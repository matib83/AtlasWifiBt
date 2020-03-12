/*
  Este programa calcula el consumo electrico con el sensor AC712 de 5A
  y los procesa con el ESP8266 (sin funciones de WIFI)
*/

#include <driver/adc.h>

#define ADC_SCALE 4095 //LE CAMBIE ESTO PORQUE LOS ADC SON DE 12 BITS
#define VREF 4.5 //tension maxima entregada por el sensor
#define DEFAULT_FREQUENCY 50
#define pin ADC1_CHANNEL_6
float sensitivity = 0.066; //ESTE SENSOR ES PARA HASTA 30A
float zero;

void setup() {
  Serial.begin(9600);
  //calibrate() saca el promedio de la medicion con I = 0
  //este programa debe iniciar sin carga para realizar correctamente esto
  Serial.flush();
  Serial.println("Calibrando... Asegúrese de que no fluya corriente a través del sensor en este momento");
  
  zero = calibrate();
  Serial.println("Hecho!");
  Serial.println(String("Cero de este sensor = ") + zero);
 
 while(1){
  zero = calibrate();
  Serial.println("Hecho!");
  Serial.println(String("Cero de este sensor = ") + zero);
  delay(2000);
 }
  
}

void loop() {
  
  float U = 220;
  float I = getCurrentAC(DEFAULT_FREQUENCY);
  float P = U * I;

  Serial.println(String("I = ") + I + " A");
  Serial.println(String("P = ") + P + " Watts");

  delay(1000);

 
}

float calibrate() {
  uint32_t acc = 0, acc1=0;
  for (int i = 0; i < 60; i++) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(pin , ADC_ATTEN_DB_0);
    acc += adc1_get_raw(pin);
    delay(500);
  }
  zero = acc / 60;
  return zero;
}

float getCurrentAC(uint16_t frequency) {
  uint32_t period = 1000000 / frequency;
  uint32_t t_start = micros();

  uint32_t Isum = 0, measurements_count = 0;
  int32_t Inow;
  //uint16_t Inow = 0;

  while (micros() - t_start < period) {
    Inow = adc1_get_raw(pin) - zero;
    //Serial.println(String("Inow = ") + Inow);
    Isum += Inow*Inow;
    //Serial.println(String("Isum = ") + Isum);
    measurements_count++;
  }

  float Irms = sqrt(Isum / measurements_count) / ADC_SCALE * VREF / sensitivity;
  
  //Serial.println(String("contador de mediciones = ") + measurements_count);
  return Irms;
}
