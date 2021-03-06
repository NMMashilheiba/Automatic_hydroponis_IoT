#include <ThingSpeak.h>
#include <DHT.h>                                    // Including library for dht
#include <ESP8266WiFi.h>

long unsigned int myChannelNumber = 1081208;
const char *apiKey = "WS83ALR7IPF22DUA";            //  Enter your Write API key from ThingSpeak              
const char *ssid =  "Galaxy_M514F0A";               // replace with your wifi ssid and wpa2 key
const char *pass =  "rlpc0937";
const char *server = "api.thingspeak.com";
               
uint8_t LED = D4;
uint8_t TEMFAN = D2;  
uint8_t BUFFPUMP = D0;
uint8_t trigPin = D5; 
uint8_t echoPin = D6; 

long duration;
int distance;

#define SensorPin A0          // the pH meter Analog output is connected with the Arduino’s Analog
#define DHTTYPE DHT11 

 
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

uint8_t DHTPin = D1; 

DHT dht(DHTPin, DHTTYPE);
WiFiClient  client;
void setup(void) 
{              
      delay(100);
      pinMode(DHTPin, INPUT);     
      pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
      pinMode(echoPin, INPUT); // Sets the echoPin as an Input
      pinMode(LED, OUTPUT);
      pinMode(BUFFPUMP,OUTPUT);
      pinMode(TEMFAN,OUTPUT);  

      Serial.begin(115200);
      dht.begin();    
      Serial.println("Connecting to ");
      Serial.println(ssid);
      WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED) 
      {
            delay(500);
            Serial.print(".");
      }
      Serial.println("");
      Serial.println("WiFi connected\n");     
      ThingSpeak.begin(client);                    // Initialize ThingSpeak
}
void loop() 
{
      for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
          { 
            buf[i]=analogRead(SensorPin);
            delay(10);
          }
      for(int i=0;i<9;i++)        //sort the analog from small to large
          {
            for(int j=i+1;j<10;j++)
            {
              if(buf[i]>buf[j])
              {
                temp=buf[i];
                buf[i]=buf[j];
                buf[j]=temp;
              }
            }
           }
      avgValue=0;
      for(int i=2;i<8;i++)                      //take the average value of 6 center sample
        avgValue+=buf[i];
      float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
      phValue=3.5*phValue;                      //convert the millivolt into pH value
      
      float hum = dht.readHumidity(); 
      float tem = dht.readTemperature();               
      Serial.print("Humidity: ");
      Serial.print(hum);
      Serial.print(" %, Temp: ");
      Serial.print(tem);
      Serial.println(" Celsius");
      long duration, inches, cm;  
      pinMode(trigPin, OUTPUT);
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      pinMode(echoPin, INPUT);
      duration = pulseIn(echoPin, HIGH);
      inches = microsecondsToInches(duration);
      cm = microsecondsToCentimeters(duration); 
      Serial.print("Nutrient level: ");
      Serial.print(inches);
      Serial.print("in, ");
      Serial.print(cm);
      Serial.print("cm");
      Serial.print("\n"); 
     
      Serial.print("    pH:");  
      Serial.print(phValue,2);
      Serial.println(" ");
      
      if (inches >= 6) 
      { 
        pinMode(LED, OUTPUT);
        digitalWrite(LED, HIGH);
      }
      else 
      {
        pinMode(LED, OUTPUT);
        digitalWrite(LED, LOW);
      }


      if ( tem >= 25) 
      { 
        pinMode(TEMFAN, OUTPUT);
        digitalWrite(TEMFAN, HIGH);
      }
      else 
      {
        pinMode(TEMFAN, OUTPUT);
        digitalWrite(TEMFAN, LOW);
      }

      if (phValue < 7) 
      { 
        pinMode(BUFFPUMP, OUTPUT);
        digitalWrite(BUFFPUMP, HIGH);
        Serial.println("ACID");
      }
      else 
      {
        pinMode(BUFFPUMP, OUTPUT);
        digitalWrite(BUFFPUMP, LOW);
        Serial.println("BASIC");
      }  

       if (inches >= 6) 
      { 
        pinMode(LED, OUTPUT);
        digitalWrite(LED, HIGH);
      }
      else 
      {
        pinMode(LED, OUTPUT);
        digitalWrite(LED, LOW);
      } 
      
      unsigned short cal;
      if(inches > 15)
      {
        cal = 0;  
      }
      else
      {
        cal = 15 - inches;
      } 
      Serial.println("...Updating on ThingSpeak...");
      ThingSpeak.setField(1, tem);
      ThingSpeak.setField(2, hum);
      ThingSpeak.setField(3, cal);
      ThingSpeak.setField(4, phValue);
      int x = ThingSpeak.writeFields(myChannelNumber, apiKey);       //Update in ThingSpeak
      delay(16000);
//    int y = ThingSpeak.writeField(myChannelNumber, 2, hum, apiKey);       //Update in ThingSpeak
//    delay(15001);
//    int z = ThingSpeak.writeField(myChannelNumber, 3, cal, apiKey);       //Update in ThingSpeak
//    delay(15001);
      if (x==200 )
      {
        Serial.println("...Updated succesfully on ThingSpeak...\n");
      }
      else
      {
        Serial.println("Failed to update" + String(x));
      } 
       delay(500);    
}
long microsecondsToInches(long microseconds) 
{
   return microseconds / 74 / 2;
}
long microsecondsToCentimeters(long microseconds) 
{
   return microseconds / 29 / 2;
}
