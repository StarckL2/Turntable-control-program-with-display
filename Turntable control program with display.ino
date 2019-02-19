//------------------Turntable control program with display ------------//
//----------------------------By: StarckL2 ----------------------------//
//------------------Insta acc: @marcellowellingtonra ------------------// 

//---------------------------Display LCD 16x2--------------------------//
// Conections: VSS on GND / VDD on 5v / VO on central pin potentiometer//
//             RS on 2 / RW on GND / E on 3 / D4 to D7 4,5,6,7         //
//             A on 5v / K on GND                                      //
//--------------------------------MFRC522------------------------------//
// Conections: SDA on 10 / SCK on 13 / MOSI on 11 / MISO on 12         //
//             NC off / GND on GND / RST on 7 / 3.3v on 3.3v           //

//-----------------------------Libraries-------------------------------//
//MFRC522:https://github.com/StarckL2/RFID-Library-for-MFRC522-Arduino //
//LiquidCrystal:https://github.com/StarckL2/LiquidCrystal-Arduino      //
//-------------------------------Thanks !------------------------------//


//------------ Including libraries ---------------//
#include <SPI.h>   // Serial Peripheral Interface (SPI)
#include <MFRC522.h> // RFID Library for MFRC522 (SPI)
#include <Servo.h> // Servo Library
#include <LiquidCrystal.h>  // LCD display Library

//--------------- Definitions ------------------//

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
#define LED_G           A1           // Red led door
#define LED_R           A4           // Green led door
#define BUZZER          A3           // BUZZER door
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
Servo myServo;    // Variable to the servo
LiquidCrystal lcd(2,3,4,5,6,7); // Display ports
MFRC522::MIFARE_Key key;


void setup() {
  
	Serial.begin(9600);		// Initialize serial communications with the PC
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
  myServo.attach(A2);  //Servo pin
  myServo.write(0); // Servo start position
  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  noTone(BUZZER);
 	Serial.println(F("Put your card in the reader"));
  Serial.println();
  lcd.begin(16,2);    // Starts the LCD and specifies size
  lcd.clear();
  mensagemini();  // Homme screen
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF; //Prepare key - factory default = FFFFFFFFFFFFh
  
  }

void loop() {

    //Waiting for card
if ( ! mfrc522.PICC_IsNewCardPresent()) {
  return;
  }                                         

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }                                         

  Serial.println(F("Card detected"));
  
  //Show UID on serial monitor
  Serial.print("Card number");

  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0 " : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  
  // Finds the name in the card's memory
  byte block;
  byte len;
  byte buffer1[18];
  MFRC522::StatusCode status;

  //-------------Authorization test--------------//
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  
  // UID of authorized cards - Replaced
  if (content.substring(1) == "DA 8A 25 D9" || content.substring(1) == "EB 60 D6 2B" )
  {
    Serial.println("Authorized entry");
    Serial.println();
    Serial.print("Name: "); 
  
     block = 4;
     len = 18;

  // Get sector data 1, block 4 = Name
  byte sector         = 1;
  byte blockAddr      = 4;
  byte trailerBlock   = 7;
  byte status;
  byte buffer[18];
  byte size = sizeof(buffer);

  // Authentication using key A
  status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                  trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.print(mfrc522.GetStatusCodeName(status));
    return;
  }
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.print(mfrc522.GetStatusCodeName(status));
  }

  // Displays the first name data on the Serial Monitor and LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  for (byte i = 1; i < 16; i++)
  {
    Serial.print(char(buffer[i]));
    lcd.write(char(buffer[i]));
  }
  
  // Get sector data 0, block 1 = Last name
  sector         = 0;
  blockAddr      = 1;
  trailerBlock   = 3;
 
  // Authentication using key A for the last name
  status=mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                  trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.print(mfrc522.GetStatusCodeName(status));
    return;
  }
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.print(mfrc522.GetStatusCodeName(status));
  }
  
  //Displays the last name
  lcd.setCursor(0, 1);
  for (byte i = 0; i < 16; i++)
  {
    Serial.print(char(buffer[i]));
    lcd.write(char(buffer[i]));
  } 
    delay(100);
    digitalWrite(LED_G, HIGH);
    tone(BUZZER, 1000);
    delay(300);
    noTone(BUZZER);
    myServo.write(180);
    delay(1000);
    myServo.write(0);
    digitalWrite(LED_G, LOW);
  }

  // If the test fails
  else {
    Serial.println("Access denied");
    digitalWrite(LED_R, HIGH);
    tone(BUZZER, 300);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access denied");
    delay(500);
    noTone(BUZZER);
    tone(BUZZER, 300);
    digitalWrite(LED_R, LOW);
    noTone(BUZZER);

  }
  
  mfrc522.PICC_HaltA();   // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
  mensagemini();
}
void mensagemini()  {
  lcd.clear();
  lcd.print("Pass the card");  // Homme screen
  }
