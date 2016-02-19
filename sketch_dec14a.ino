

#include <SPI.h>
#include <WiFi.h>
#include <PubNub.h>
#include <aJSON.h>
#include <Temboo.h>
#include <WiFiClient.h>
#include <TM1637.h>
#include <DHT.h>



WiFiClient client;

#define CLK 36 /* 4-digital display clock pin */
#define DIO 35 /* 4-digital display data pin */
#define BLINK_LED RED_LED /* blink led */
#define TEMP_HUMI_PIN 25 /* pin of temperature&humidity sensor */


TM1637 tm1637(CLK, DIO); /* 4-digital display object */
DHT dht(TEMP_HUMI_PIN, DHT22); /* temperature&humidity sensor object */
int8_t t_bits[2] = {0}; /* array to store the single bits of the temperature */
int8_t h_bits[2] = {0}; /* array to store the single bits of the humidity */ 


#define TEMBOO_ACCOUNT "omidomidi"  // Your Temboo account name 
#define TEMBOO_APP_KEY_NAME "myFirstApp"  // Your Temboo app name
#define TEMBOO_APP_KEY "327cd9569736489c909d56c0e8720d3c"  // Your Temboo app key

static char ssid[] = "ukyedu";    // your network SSID (name)
//static char pass[] = "your_wifi_pass”; // your network password
//static int keyIndex = 0;               // your network key Index number (needed only for WEP)

const static char pubkey[] = "pub-c-4677d5b4-54f3-4358-bcc0-14de3dcf8023";
const static char subkey[] = "sub-c-8ba0e87e-93ab-11e5-b829-02ee2ddab7fe";
const static char channel[] = "my_channel";

#define NUM_CHANNELS 4  // How many analog channels do you want to read?
const static uint8_t analog_pins[] = {23, 24};    // which pins are you reading?



#define BUZZER_PIN     39
int length = 15; /* the number of notes */
char notes[] = "ccggaagffeeddc ";
int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
int tempo = 300;




void setup()
{
  
       tm1637.init(); /* initialize 4-digital display */
       tm1637.set(BRIGHT_TYPICAL); /* set the brightness */
       tm1637.point(POINT_ON); /* light the clock point ":" */
      
       dht.begin(); /* initialize temperature humidity sensor */
      
       pinMode(RED_LED, OUTPUT); /* declare the red_led pin as an OUTPUT */

        pinMode(BUZZER_PIN, OUTPUT);
  	Serial.begin(9600);

        Serial.println("Start WiFi");
        WiFi.begin(ssid);
        while(WiFi.localIP() == INADDR_NONE) {
          Serial.print(".");
          delay(300);
        }
	Serial.println("WiFi set up");

	PubNub.begin(pubkey, subkey);
	Serial.println("PubNub set up");
        delay(5000);
}

void loop()
{
         int _temperature = dht.readTemperature(); /* read the temperature value from the
         sensor */
         int _humidity = dht.readHumidity(); /* read the humidity value from the sensor */
        
         memset(t_bits, 0, 2); /* reset array when we use it */
         memset(h_bits, 0, 2);
        
         /* 4-digital-display [0,1] is used to display temperature */
         t_bits[0] = _temperature % 10;
         _temperature /= 10;
         t_bits[1] = _temperature % 10;
        
         /* 4-digital-display [2,3] is used to display humidity */
         h_bits[0] = _humidity % 10;
         _humidity /= 10;
          h_bits[1] = _humidity % 10;

         /* show it */
         tm1637.display(1, t_bits[0]);
         tm1637.display(0, t_bits[1]);
        
         tm1637.display(3, h_bits[0]);
         tm1637.display(2, h_bits[1]);



  
        // create JSON objects
        aJsonObject *msg, *analogReadings;
        msg = aJson.createObject();
        aJson.addItemToObject(msg, "analogReadings", analogReadings = aJson.createObject());
        
        // get latest sensor values then add to JSON message
        for (int i = 0; i < NUM_CHANNELS; i++) {
          String analogChannel = String(analog_pins[i]);
          char charBuf[analogChannel.length()+1];
          analogChannel.toCharArray(charBuf, analogChannel.length()+1);
          int analogValues = analogRead(analog_pins[i]);
          aJson.addNumberToObject(analogReadings, charBuf, analogValues);
        }

          
        // convert JSON object into char array, then delete JSON object
        char *json_String = aJson.print(msg);
        aJson.deleteItem(msg);
        
        // publish JSON formatted char array to PubNub
	Serial.print("publishing a message: ");
	Serial.println(json_String);
        PubNub.publish(channel, json_String);
        free(json_String);
        
        
        int calls = 0;
        int maxCalls = 10;
        int sensorValue = analogRead(analog_pins[0]);
        if (sensorValue < 1300) {
          if (calls < maxCalls) {
            Serial.println("\nTriggered! Calling CaptureTextToSpeechPrompt Choreo...");
            runCaptureTextToSpeechPrompt(sensorValue);
            calls++;
          } else {
            Serial.println("\nTriggered! Skipping to save Temboo calls. Adjust maxCalls as required.");
          }
        }
        
	delay(500);
}

void runCaptureTextToSpeechPrompt(int sensorValue) {
  TembooChoreo CaptureTextToSpeechPromptChoreo(client);

  // Set Temboo account credentials
  CaptureTextToSpeechPromptChoreo.setAccountName(TEMBOO_ACCOUNT);
  CaptureTextToSpeechPromptChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  CaptureTextToSpeechPromptChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set Choreo inputs
  String TextValue = "Hello, the light is too low! please press one to sound the alarm";
  CaptureTextToSpeechPromptChoreo.addInput("Text", TextValue);
  String APIKeyValue = "70be2648";
  CaptureTextToSpeechPromptChoreo.addInput("APIKey", APIKeyValue);
  String ToValue = "18594893147";
  CaptureTextToSpeechPromptChoreo.addInput("To", ToValue);
  String APISecretValue = "e6633616";
  CaptureTextToSpeechPromptChoreo.addInput("APISecret", APISecretValue);
  String ByeTextValue = "thank you! goodbye";
  CaptureTextToSpeechPromptChoreo.addInput("ByeText", ByeTextValue);

  // Identify the Choreo to run
  CaptureTextToSpeechPromptChoreo.setChoreo("/Library/Nexmo/Voice/CaptureTextToSpeechPrompt");

  // Run the Choreo
  unsigned int returnCode = CaptureTextToSpeechPromptChoreo.run();

  // A return code of zero means everything worked
  if (returnCode == 0) {
    while (CaptureTextToSpeechPromptChoreo.available()) {
      String name = CaptureTextToSpeechPromptChoreo.readStringUntil('\x1F');
      name.trim();

      if (name == "CallbackData") {
        if (CaptureTextToSpeechPromptChoreo.findUntil("1", "\x1E")) {
          while(1){
           for(int i = 0; i < length; i++) {
             if(notes[i] == ' ') {
             delay(beats[i] * tempo);
             } else {
             playNote(notes[i], beats[i] * tempo);
             }
             delay(tempo / 2); /* delay between notes */
             }
          }
          CaptureTextToSpeechPromptChoreo.find("\x1E");
        }
      }
      else {
        CaptureTextToSpeechPromptChoreo.find("\x1E");
      }
    }
  }

  CaptureTextToSpeechPromptChoreo.close();
}

/* play tone */
void playTone(int tone, int duration) {
 for (long i = 0; i < duration * 1000L; i += tone * 2) {
 digitalWrite(BUZZER_PIN, HIGH);
 delayMicroseconds(tone);
 digitalWrite(BUZZER_PIN, LOW);
 delayMicroseconds(tone);
 }
}
void playNote(char note, int duration) {
 char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
 int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };

 // play the tone corresponding to the note name
 for (int i = 0; i < 8; i++) {
 if (names[i] == note) {
 playTone(tones[i], duration);
 }
 }
}

