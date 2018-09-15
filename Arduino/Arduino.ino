
/**************************************************************************
    INCLUDES
**************************************************************************/
#include <LiquidCrystal.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
/**************************************************************************
    DEFINES
**************************************************************************/
#define LED1 2
#define SAMPLE_DELAY 800
#define LCD_RS_PIN 8 
#define LCD_E_PIN 9   
#define LCD_D7_PIN 7 
#define LCD_D6_PIN 6 
#define LCD_D5_PIN 5 
#define LCD_D4_PIN 3 
#define ANALOG_INPUT_A0 0
#define ANALOG_INPUT_A1 1
#define ANALOG_INPUT_A2 2
#define ANALOG_INPUT_A3 3
#define ANALOG_INPUT_A4 4
#define ANALOG_INPUT_A5 5



/**************************************************************************
    I2C CONFIGURATION
**************************************************************************/
Adafruit_BMP280 bme; // I2C


/**************************************************************************
    LCD CONFIGURATION
**************************************************************************/
const int rs = LCD_RS_PIN;
const int en = LCD_E_PIN;
const int d4 = LCD_D4_PIN;
const int d5 = LCD_D5_PIN;
const int d6 = LCD_D6_PIN;
const int d7 = LCD_D7_PIN;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

bool ledStatus;

/**************************************************************************
    ADC CONFIGURATION
**************************************************************************/
int analogPin = ANALOG_INPUT_A0;
unsigned int readedValue = 0;
unsigned int savedValue = 0;
int lightPresent = 0;
int sensorReading;



/**************************************************************************
    ETHERNET CONFIGURATION
**************************************************************************/
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xCD
};
IPAddress ip(192, 168, 120, 50);
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
String readString;
char c;


/**************************************************************************
    SETUP
**************************************************************************/
void setup() {


  
/**************************************************************************
    LCD SETUP
**************************************************************************/
  lcd.begin(16, 2);
  lcd.setCursor(3, 0);


  
/**************************************************************************
    I2C SETUP
**************************************************************************/
  if (!bme.begin())
  {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }


/**************************************************************************
    SERIAL SETUP
**************************************************************************/
  Serial.begin(9600);
  while (!Serial){} // wait for serial port to connect. Needed for native USB port only
  lcd.clear();


/**************************************************************************
    ETHERNET SETUP
**************************************************************************/
  Ethernet.begin(mac, ip);
  server.begin();


  
/**************************************************************************
    LED SETUP AND TEST BY BLINKING
**************************************************************************/
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, HIGH);
  
  for (int x = 0 ; x < 5 ; x++)
  {
    LedON();
    delay(200);
    LedOFF();
    delay(200);    
  }


/**************************************************************************
    END OF SETUP
**************************************************************************/
  lcd.clear();
  lcd.print("Server IP : ");
  lcd.setCursor(0, 1);
  lcd.println(Ethernet.localIP());
  delay(2000);
  lcd.clear();
}



/**************************************************************************
    LOOP
**************************************************************************/
void loop()
{
    sensorReading = analogRead(analogPin);
    lcd.clear();
    lcd.print("LDR: ");
    lcd.print(analogRead(analogPin));
    lcd.setCursor(0, 1);
    lcd.print("TEMP: ");
    lcd.print(bme.readTemperature());
    lcd.print("'C");

    
    // listen for incoming clients
    EthernetClient client = server.available();
    /* START CLINET*/
    
    if (client) 
    {
        Serial.println("new client");
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;
      
        while (client.connected()) 
        {
            lcd.setCursor(0, 1);
            lcd.write("Connected");
            lcd.write("     ");
            
            if (client.available())
            {
                c = client.read();
                readString += c;
                //Serial.write(c);
                // if you've gotten to the end of the line (received a newline
                // character) and the line is blank, the http request has ended,
                // so you can send a reply
                
                if ((readString.indexOf("?buttonOn") > 0) & !(ledStatus)) 
                {
                    LedON();
                }
                if ((readString.indexOf("?buttonOff") > 0) & (ledStatus)) 
                {
                    LedOFF();
                }
                
                if (readString.indexOf("?savevalue") > 0) 
                {
                    savedValue = sensorReading;
                }

                    lightPresent = sensorReading ; //MAX = 1024 MIN = 0
                    lightPresent = lightPresent / 10.24 ;

                    
                if (c == '\n' && currentLineIsBlank)
                {

                    /**************************************************************************
                        send a standard http response header
                    **************************************************************************/
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");  // the connection will be closed after completion of the response
                    //client.println("Refresh: 1");  // refresh the page automatically every 1 sec
                    client.println();
                    client.println("<!DOCTYPE HTML>");
                    client.println("<html>");
                    /**************************************************************************
                        HTML Page
                    **************************************************************************/
                    
                    client.println("<a href=/?buttonOn >Turn On Led</a>");
                    client.println("<a href=/?buttonOff >Turn Off Led</a>");
                    client.println("<a href=/?savevalue >Save Value</a>");

                    client.println("<br /><br />");
                    client.print(" LDR Saved Value = ");
                    client.print(savedValue);
                    client.println("<br />");

                    client.print("LDR Value From ADC A");
                    client.print(analogPin);
                    client.print(" = ");
                    client.print(sensorReading);
                    client.print("   ");
                    client.print(lightPresent);
                    client.print("%");
                    client.println("<br />");
                    
                    client.print("BMP Sensor Temperature  = ");
                    client.print(bme.readTemperature());
                    client.print(" *C");
                    client.println("<br />");
                    
                    client.print("BMP Sensor Pressure     = ");
                    client.print(bme.readPressure());
                    client.print(" Pa");
                    client.println("<br />");
                    
                    client.print("BMP Sensor Approx altitude = ");
                    client.print(bme.readAltitude(1013.25)); // this should be adjusted to your local forcase
                    client.print(" m");
                    client.println("<br />");
                    
                    client.println("</html>");
                    break;
                }
                if (c == '\n')
                {
                    // you're starting a new line
                    currentLineIsBlank = true;
                } 
                else if (c != '\r')
                {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }

        }
        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        client.stop();
        Serial.println("client disconnected");
        lcd.setCursor(0, 1);
        lcd.write("Disconnected   ");
        readString = " ";
        c = ' ';
    }
    //Serial.println();
    delay(1000);
}

void LedON() {
    digitalWrite(LED1, LOW);
    ledStatus = true; 
}

void LedOFF() {
    digitalWrite(LED1, HIGH);
    ledStatus = false;
}
