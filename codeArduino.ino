#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define BROKER_IP    "mqtt.eclipse.org" //Server
#define DEV_NAME     "dev"    //name device mqtt
#define MQTT_USER    "ok"     //User mqtt
#define MQTT_PW      "123456"   //Password mqtt
const char ssid[] = "TIM-26108969";  // Name Wifi
const char pass[] = "hTGi0xe0uBm11GjGsjnpV5Cn";  //Password Wifi

#include <MQTT.h>

#ifdef ARDUINO_SAMD_MKRWIFI1010
#include <WiFiNINA.h>
#elif ARDUINO_SAMD_MKR1000
#include <WiFi101.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#else
#error unknown board
#endif

#include "Ultrasonic.h"

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
u8g2_uint_t offset;      
u8g2_uint_t width;
     
const char *text = "U8g2 "; 

Ultrasonic ultrasonic(0);
// the PIN
int led_pin = 5;
int button_pin = 6;

WiFiClient net;
MQTTClient client;


void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) { //connection to Wifi
    Serial.print(".");
    delay(10000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect(DEV_NAME, MQTT_USER, MQTT_PW)) {   //connection to mqtt
    Serial.print(".");
    delay(10000);
  }
  Serial.println("\nconnected!");
  client.subscribe("/Hi"); //Topic mqtt(/Hi)
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void setup()
{
    Serial.begin(9600);
    pinMode(led_pin, OUTPUT);
    pinMode(button_pin, INPUT);

    u8g2.begin();
    u8g2.setFont(u8g2_font_inb30_mr);  
    width = u8g2.getUTF8Width(text);    
  
    u8g2.setFontMode(0);  

    WiFi.begin(ssid, pass);

    client.begin(BROKER_IP, 1883, net);
    client.onMessage(messageReceived);
    connect();
}

void loop() {

    client.loop();
    if (!client.connected()) {
      connect();
    }
    
  
    long RangeInCentimeters;
    RangeInCentimeters = ultrasonic.MeasureInCentimeters(); 
    Serial.print("Distance: ");
    Serial.print(RangeInCentimeters);//0~400cm
    Serial.println(" cm");
    delay(1000);

    u8g2_uint_t x;


     u8g2.firstPage();
    do {
    
      
      x = offset;
      u8g2.setFont(u8g2_font_inb30_mr); 
      u8g2.setFont(u8g2_font_inb16_mr);   
      u8g2.setCursor(0, 58);
     
      u8g2.print(RangeInCentimeters);          
  
    } while ( u8g2.nextPage() );
    
    offset-=1;              
    if ( (u8g2_uint_t)offset < (u8g2_uint_t)-width )  
      offset = 0;             
      
    delay(10);  
    
    int delay_value = RangeInCentimeters/1.5;

    if(RangeInCentimeters >= 150){
        digitalWrite(led_pin, LOW);
    }
    else if(RangeInCentimeters < 150 && RangeInCentimeters >= 10){
        digitalWrite(led_pin, HIGH);
        delay(delay_value);
        digitalWrite(led_pin, LOW);
        delay(delay_value);
        client.publish("/Hi", " Distancing not respected");
        delay(5000);
    }
    else{
      digitalWrite(led_pin, HIGH);
      client.publish("/Hi", " Distancing not respected");
        delay(5000);
    }
}
