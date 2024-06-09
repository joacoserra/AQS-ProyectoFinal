// Librerias que vamos a usar en nuestro programa
#include <Arduino.h>
#include <MQ7.h>
#include <SPI.h>
//#include <TFT_eSPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <time.h>
#include <ESP32Time.h>

//Imagenes
#include "Logo.h"
#include "monoxide.h"

// Los handlers para las funciones que corren en los nucleos
TaskHandle_t Task1; // Handler para la de los sensores
TaskHandle_t Task2; // Handler para la pantalla, fecha y hora

// Credenciales para conectar la placa a internet
const char* ssid     = "Fibertel WiFi867 2.4GHz";
const char* password = "0043559168";

// NTP Server donde se obtiene la hora mundial
// NTP (Network Time Protocol)
struct tm timeinfo;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; // GMT -3 Argentina
const int   daylightOffset_sec = 3600;

#define A_PIN 33 //GPIO33 ADC1_CH5
#define VOLTAGE 5 
#define TFT_GREY 0x5AEB // New colour
#define DHTPIN 32 // GPIO32 ADC1_CH4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
MQ7 mq7(A_PIN, VOLTAGE);
//TFT_eSPI tft = TFT_eSPI();  // Invoke library
ESP32Time rtc;

// put function declarations here:
void Task1code( void * pvParameters );
void Task2code( void * pvParameters );

void setup() {
  Serial.begin(115200);
  dht.begin();
  //tft.init();
  //tft.setRotation(4);  
  //tft.setSwapBytes(true); 
  mq7.calibrate();		// calculates R0
  //dht.begin();
  delay(2000);
  //tft.fillScreen(TFT_BLACK);
  //tft.pushImage(15,35,220,169,Logo);
  //delay(8000);
  //tft.fillScreen(TFT_BLACK);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  // Inicializar y obtener la hora de internet
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  //Aca lo que estamos haciendo es cargar el RTC interno con la hora y fecha que obtenemos de internet
  rtc.setTime(
    timeinfo.tm_sec,
    timeinfo.tm_min,
    timeinfo.tm_hour,
    timeinfo.tm_mday,
    timeinfo.tm_mon +1,
    timeinfo.tm_year + 1900
  );

  // Creacion de tareas para los nucleos
  xTaskCreatePinnedToCore(
             Task1code, /* Task function. */
             "Task1",   /* name of task. */
             10000,     /* Stack size of task */
             NULL,      /* parameter of the task */
             1,         /* priority of the task */
             &Task1,    /* Task handle to keep track of created task */
             0);        /* pin task to core 0 */
  xTaskCreatePinnedToCore(
             Task2code,  /* Task function. */
             "Task2",    /* name of task. */
             10000,      /* Stack size of task */
             NULL,       /* parameter of the task */
             1,          /* priority of the task */
             &Task2,     /* Task handle to keep track of created task */
             1);         /* pin task to core 1 */
}

void loop() {
  // Esto no se deberia usar ya que correremos nuestro programa en los nucleos
}

/////// Aca van nuestras funciones:
// Esta primera funcion lo que deberia hacer es tomar las mediciones de los sensores
void Task1code( void * pvParameters ){
  for (;;)
  {
    float ppm= mq7.readPpm();
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    Serial.print("Monoxido: ");
    Serial.println(ppm);
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println("%");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(F("°C "));
    delay(1500);
  }
}
// Es funcion deberia ser la que actualiza la pantalla cada cierto tiempo y muestra la hora
void Task2code( void * pvParameters ){
  for(;;){
    Serial.println(rtc.getDateTime());
    delay(1000);
  }
}
