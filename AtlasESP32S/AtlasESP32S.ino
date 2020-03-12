/*Esta version  mide corriente y tension para mejorar el error*/
/*La calibracion esta fija*/

#include <WiFi.h>
#include <driver/adc.h>

#define ADC_SCALE 4095 //LE CAMBIE ESTO PORQUE LOS ADC SON DE 12 BITS
#define VREF 4.5 //tension maxima entregada por el sensor
#define VREFv 323 //tension maxima entregada por el sensor
#define DEFAULT_FREQUENCY 50
#define pinI ADC1_CHANNEL_6
#define pinV ADC1_CHANNEL_7
float sensitivity = 0.066; //ESTE SENSOR ES PARA HASTA 30A
float zero = 2144;

const char* ssid     = "motorola";
const char* password = "6e47935c271a";
const char* host = "api.thingspeak.com";
const char* streamId   = "/update/";
const char* privateKey = "KQG9ZA6G41Z0JSC6";


void setup() {

  Serial.begin(115200);
   delay(10);
   
  /*Parte WIFI*/

  Serial.println();
  Serial.println();
  Serial.println();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  /*for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }*/

  WiFi.begin(ssid, password);
// WiFi.begin("CIME", "1@labocime2008");

/*while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }*/

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  
}

void loop() {

  float U = getVoltageAC(DEFAULT_FREQUENCY);
  float I = getCurrentAC(DEFAULT_FREQUENCY);
  float P = U * I;
  Serial.println(String("V = ") + U + " V");
  Serial.println(String("I = ") + I + " A");
  Serial.println(String("P = ") + P + " Watts");
  
 Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // We now create a URI for the request
    
    String url = streamId;
    url += "?api_key=";
    url += privateKey;
    url += "&field1=";
    url += I;

    Serial.print("Requesting URL: ");
    Serial.println(url);

    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection"); 

     delay(60000);

}

float getCurrentAC(uint16_t frequency) {
  uint32_t period = 1000000 / frequency;
  uint32_t t_start = micros();

  uint32_t Isum = 0, measurements_count = 0;
  int32_t Inow;
  //uint16_t Inow = 0;

  while (micros() - t_start < period) {
    Inow = adc1_get_raw(pinI) - zero;
    //Serial.println(String("Inow = ") + Inow);
    Isum += Inow*Inow;
    //Serial.println(String("Isum = ") + Isum);
    measurements_count++;
  }

  float Irms = sqrt(Isum / measurements_count) / ADC_SCALE * VREF / sensitivity;
  
  //Serial.println(String("contador de mediciones = ") + measurements_count);
  return Irms;
}

float getVoltageAC(uint16_t frequency) {
  uint32_t period = 1000000 / frequency;
  uint32_t t_start = micros();

  uint32_t Vsum = 0, measurements_count = 0;
  int32_t Vnow;

  while (1)
  {
     while (micros() - t_start < period) {
    Vnow = adc1_get_raw(pinV);
    //Serial.println(String("Inow = ") + Inow);
    Vsum += Vnow*Vnow;
    //Serial.println(String("Isum = ") + Isum);
    measurements_count++;
  }

  float Vrms = sqrt(Vsum / measurements_count) / ADC_SCALE * VREFv ;
  
  Serial.println(String("tensiones = ") + Vrms);
  return Vrms; 
  }
}
