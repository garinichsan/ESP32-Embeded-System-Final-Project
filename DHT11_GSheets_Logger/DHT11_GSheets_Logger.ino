/* 
 * Garin Ichsan Nugraha - STI18050 - ITB
 * DHT11 Data Logging to Google Spread Sheets
 *  
 * Update 25 Apr 2020 
 * 
 * Adaptation from
 * Rui Santos - https://randomnerdtutorials.com
 */
 
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <Wire.h>
#include <DHT.h>

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT11     // DHT 11DHT dht(DHTPIN, DHTTYPE);

// Replace with your SSID and Password
const char* ssid = "";
const char* password = "";

// Replace with your unique IFTTT URL resource
const char* resource = "/trigger/dht_readings/with/key/bBUy3MFpC1x0Ow3cH1HdCNyGSBdos10A2qFU_9fZOjd";

// How your resource variable should look like, but with your own API KEY (that API KEY below is just an example):
//const char* resource = "/trigger/DHT11_TO_GSHEETS/with/key/bEE-kSDiWDxdHyFWUNltMG";

// Maker Webhooks IFTTT
const char* server = "maker.ifttt.com";

// Time to sleep
uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for micro seconds to seconds
// sleep for 3 seconds
uint64_t TIME_TO_SLEEP = 3;

setup() {
  Serial.begin(115200); 
  delay(2000);// initialize DHT Sensor
  dht.begin();
  
  initWifi();
  makeIFTTTRequest();
    
  #ifdef ESP32
    // enable timer deep sleep
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);    
    Serial.println("Going to sleep now");
    // start deep sleep for 3600 seconds (60 minutes)
    esp_deep_sleep_start();
  #else
    // Deep sleep mode for 3600 seconds (60 minutes)
    Serial.println("Going to sleep now");
    ESP.deepSleep(TIME_TO_SLEEP * uS_TO_S_FACTOR); 
  #endif
}

void loop() {
  // sleeping so wont get here 
}

// Establish a Wi-Fi connection with your router
void initWifi() {
  Serial.print("Connecting to: "); 
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  int timeout = 10 * 4; // 10 seconds
  
  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }
  
  Serial.println("");if(WiFi.status() != WL_CONNECTED) {
     Serial.println("Failed to connect, going back to sleep");
  }
  
  Serial.print("WiFi connected in: "); 
  Serial.print(millis());
  Serial.print(", IP address: "); 
  Serial.println(WiFi.localIP());
}

// Make an HTTP request to the IFTTT web service
void makeIFTTTRequest() {
  Serial.print("Connecting to "); 
  Serial.print(server);
  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  
  Serial.println();
  if(!!!client.connected()) {
    Serial.println("Failed to connect...");
  }
  
  Serial.print("Request resource: "); 
  Serial.println(resource);// Temperature in Celsius
  String jsonObject = String("{\"value1\":\"") + dht.readTemperature() + "\",\"value2\":\"" + dht.readHumidity() + "\"}";
                      
  // Comment the previous line and uncomment the next line to publish temperature readings in Fahrenheit                    
  /*String jsonObject = String("{\"value1\":\"") + (1.8 * bme.readTemperature() + 32) + "\",\"value2\":\"" 
                      + (bme.readPressure()/100.0F) + "\",\"value3\":\"" + bme.readHumidity() + "\"}";*/
                      
  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server); 
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);
        
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
    Serial.println("No response...");
  }
  while(client.available()){
    Serial.write(client.read());
  }
  
  Serial.println("\nclosing connection");
  client.stop(); 
}
