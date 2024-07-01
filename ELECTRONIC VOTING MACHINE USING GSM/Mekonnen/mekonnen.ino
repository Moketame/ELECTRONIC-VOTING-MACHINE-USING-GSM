#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

// Define pin numbers for the push buttons and buzzer
#define ENROLL_BUTTON_PIN A0      // Button to initiate enrollment
#define MATCH_BUTTON_PIN A4       // Button to initiate fingerprint matching
#define DELETE_BUTTON_PIN A1      // Button to initiate fingerprint deletion
#define UP_BUTTON_PIN A2          // Button to move up among participant IDs
#define DOWN_BUTTON_PIN A3        // Button to move down among participant IDs
#define AKP_BUTTON_PIN A8 // Button for candidate   AKP
#define CHP_BUTTON_PIN A9 // Button for candidate   CHP
#define MHP_BUTTON_PIN A10 // Button for candidate   MHP
#define SEND_RESULTS_BUTTON_PIN A11 // Button for sending results
#define BUZZER_PIN A5              // Buzzer pin
#define YOUR_GSM_TX_PIN 5
#define YOUR_GSM_RX_PIN 4
//#define YOUR_SIM_PIN "1234"
// Define candidate vote counts
int AKPVotes = 0;
int CHPVotes = 0;
int MHPVotes = 0;
int Winner=0;
SoftwareSerial mySerial(2, 3); // SoftwareSerial for boards without hardware serial
#define mySerial Serial1 // Hardware serial for boards like Leonardo/M0
SoftwareSerial gsmSerial(YOUR_GSM_TX_PIN, YOUR_GSM_RX_PIN); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id = 1;
const uint8_t max_id = 30; // Maximum participant ID
bool voted[max_id]; // Array to keep track of whether each ID has voted
LiquidCrystal lcd(13, 12, 11, 10, 9, 8); // Initialize the LCD

void setup() {
  Serial.begin(9600);
  lcd.begin(20, 4); // Initialize the LCD with 16 columns and 2 rows
gsmSerial.begin(9600); // Adjust baud rate as per your GSM module's specifications
delay(1000); // Delay for stability
  pinMode(ENROLL_BUTTON_PIN, INPUT_PULLUP);       // Set button pin as input with pull-up resistor
  pinMode(MATCH_BUTTON_PIN, INPUT_PULLUP);        // Set button pin as input with pull-up resistor
  pinMode(DELETE_BUTTON_PIN, INPUT_PULLUP);       // Set button pin as input with pull-up resistor
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);           // Set button pin as input with pull-up resistor
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);         // Set button pin as input with pull-up resistor
  pinMode(AKP_BUTTON_PIN, INPUT_PULLUP);  // Set button pin as input with pull-up resistor
  pinMode(CHP_BUTTON_PIN, INPUT_PULLUP);  // Set button pin as input with pull-up resistor
  pinMode(MHP_BUTTON_PIN, INPUT_PULLUP);  // Set button pin as input with pull-up resistor
  pinMode(SEND_RESULTS_BUTTON_PIN, INPUT_PULLUP); // Set button pin as input with pull-up resistor
  pinMode(BUZZER_PIN, OUTPUT);                   // Set buzzer pin as output
for (int i = 0; i < max_id; i++) {
    voted[i] = false;
  }
  while (!Serial); // Wait for Serial Monitor to open
  lcd.setCursor(0, 0);
    lcd.print("GSM BASED ELECTRONIC");
   lcd.setCursor(0, 1);
    lcd.print("VOTING MACHINE");
     lcd.setCursor(0, 2);
    lcd.print("USING ARDUINO!");
    lcd.setCursor(0, 3);
    lcd.print("DID BY MEKONNEN");
    delay(5000);
    lcd.clear();
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint Sensor");
    lcd.setCursor(0, 1);
    lcd.print("Detected!");
    lcd.setCursor(0, 2);
    lcd.print("Please Press Enroll");
    lcd.setCursor(0, 3);
    lcd.print("Button!");
    delay(2000);
   // lcd.clear();
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    lcd.print("Fingerprint Sensor");
    lcd.setCursor(0, 1);
    lcd.print("Not Found!");
    while (1); // Hang if sensor not found
  }
}

void loop() {
  if (digitalRead(ENROLL_BUTTON_PIN) == LOW) { // Check if enroll button is pressed
    Serial.println("Enroll button pressed! Ready to enroll a fingerprint.");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Enroll Fingerprint");
    lcd.setCursor(0, 2);
    lcd.print("Press Fingerprint");
  lcd.setCursor(0, 0);
    lcd.print("Enter ID (1-30):");
    id = readnumber();
    if (id == 0 || id > max_id) {
      Serial.println("Invalid ID!");
      lcd.clear();
      lcd.print("Invalid ID!");
      delay(2000);
      return; // Invalid ID
    }
    Serial.print("Enrolling ID #");
    Serial.println(id);
    lcd.setCursor(0, 1);
    lcd.print("Enrolling ID #:");
    while (!getFingerprintEnroll()); // Wait for fingerprint enrollment to complete
    buzz(); // Activate the buzzer
  }
  if (digitalRead(MATCH_BUTTON_PIN) == LOW) { // Check if match button is pressed
    Serial.println("Match button pressed! Ready to match a fingerprint.");
    lcd.clear();
    lcd.print("Match Fingerprint");
    lcd.setCursor(0, 1);
    lcd.print("Place Finger");
    while (!getFingerprintMatch()); // Wait for fingerprint matching to complete
    buzz(); // Activate the buzzer
  }
  if (digitalRead(DELETE_BUTTON_PIN) == LOW) { // Check if delete button is pressed
    Serial.println("Delete button pressed! Ready to delete a fingerprint.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Delete Fingerprint");
    lcd.setCursor(0, 1);
    lcd.print("Enter ID (1-30):");
    id = readnumber();
    if (id == 0 || id > max_id) {
      Serial.println("Invalid ID!");
    lcd.setCursor(0, 2);
      lcd.print("Invalid ID!");
      return; // Invalid ID
    }
    Serial.print("Deleting ID #");
    Serial.println(id);
    lcd.setCursor(0, 3);
    lcd.print("Deleting ID #");
    while (!getFingerprintDelete()); // Wait for fingerprint deletion to complete
    buzz(); // Activate the buzzer
    // Permanently delete the fingerprint ID from EEPROM
    EEPROM.write(id, 0);
  }
  if (digitalRead(UP_BUTTON_PIN) == LOW) { // Check if up button is pressed
    id = (id == max_id) ? 1 : id + 1;
    Serial.print("ID moved up to #");
    Serial.println(id);
    lcd.clear();
    lcd.print("ID Moved Up to #");
    lcd.print(id);
    delay(2000); // Debouncing delay
    buzz(); // Activate the buzzer
  }
  if (digitalRead(DOWN_BUTTON_PIN) == LOW) { // Check if down button is pressed
    id = (id == 1) ? max_id : id - 1;
    Serial.print("ID moved down to #");
    Serial.println(id);
    lcd.clear();
    lcd.print("ID Moved Down to #");
    lcd.print(id);
    delay(2000); // Debouncing delay
    buzz(); // Activate the buzzer
  }
  // Check if AKP button is pressed and verify fingerprint
if (digitalRead(AKP_BUTTON_PIN) == LOW) {
  // Check if the voter's fingerprint matches before allowing the vote
  if (verifyFingerprint()) {
    if (!voted[id - 1]) {
      AKPVotes++;
      Serial.println("AKP voted!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("AKP Voted!");
      delay(2000); // Debouncing delay
      buzz();      // Activate the buzzer
      voted[id - 1] = true; // Mark ID as voted
    } else {
      // Buzzer sound for trying to vote again
      buzz();
      delay(500);
      buzz();
    }
  } else {
    Serial.println("Fingerprint not recognized!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint Not");
    lcd.setCursor(0, 1);
    lcd.print("Recognized!");
    delay(2000); // Display the message for 2 seconds
    lcd.clear();
  }
}

// Implement similar logic for CHP and MHP buttons
// Check if CHP button is pressed and verify fingerprint
if (digitalRead(CHP_BUTTON_PIN) == LOW) {
  if (verifyFingerprint()) {
    if (!voted[id - 1]) {
      CHPVotes++;
      Serial.println("CHP voted!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CHP Voted!");
      delay(2000); // Debouncing delay
      buzz();      // Activate the buzzer
      voted[id - 1] = true; // Mark ID as voted
    } else {
      buzz();
      delay(500);
      buzz();
    }
  } else {
    Serial.println("Fingerprint not recognized!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint Not");
    lcd.setCursor(0, 1);
    lcd.print("Recognized!");
    delay(2000); // Display the message for 2 seconds
    lcd.clear();
  }
}

// Check if MHP button is pressed and verify fingerprint
if (digitalRead(MHP_BUTTON_PIN) == LOW) {
  if (verifyFingerprint()) {
    if (!voted[id - 1]) {
      MHPVotes++;
      Serial.println("MHP voted!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MHP Voted!");
      delay(2000); // Debouncing delay
      buzz();      // Activate the buzzer
      voted[id - 1] = true; // Mark ID as voted
    } else {
      buzz();
      delay(500);
      buzz();
    }
  } else {
    Serial.println("Fingerprint not recognized!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint Not");
    lcd.setCursor(0, 1);
    lcd.print("Recognized!");
    delay(2000); // Display the message for 2 seconds
    lcd.clear();
  }
}

  // Check if Send Results button is pressed
    if (digitalRead(SEND_RESULTS_BUTTON_PIN) == LOW) {
    Serial.println("Sending results");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sending Results");
    String message = "AKP: " + String(AKPVotes) + " votes\n";
    message += "CHP: " + String(CHPVotes) + " votes\n";
    message += "MHP: " + String(MHPVotes) + " votes\n";
    message += "Winner: ";
    String winner;
    if (AKPVotes > CHPVotes && AKPVotes > MHPVotes) {
      winner = "AKP";
    } else if (CHPVotes > AKPVotes && CHPVotes > MHPVotes) {
      winner = "CHP";
    } else if (MHPVotes > AKPVotes && MHPVotes > CHPVotes) {
      winner = "MHP";
    } else {
      winner = "No winner";
    }
    message += winner;
    // Send message via GSM module
    sendSMS("+905013360467", message);
    // Display results on LCD
    //lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("AKP: ");
    lcd.print(AKPVotes);
    lcd.setCursor(7, 1);
    lcd.print("CHP: ");
    lcd.print(CHPVotes);
    lcd.setCursor(14, 1);
    lcd.print("MHP: ");
    lcd.print(MHPVotes);
    lcd.setCursor(0, 2);
    lcd.print("Winner: ");
    lcd.print(winner);
    delay(2000);
  }
}
void sendSMS(String phoneNumber, String message) {
  gsmSerial.println("AT"); // Send AT command to check if module is responding
  delay(100);
  if (gsmSerial.find("OK")) {
  gsmSerial.println("AT+CMGF=1"); // Set the GSM module to text mode
  delay(100);
  gsmSerial.println("AT+CMGS=\"+905013360467\""); // Set the recipient phone number
  delay(1000);
  gsmSerial.println(message); // Set SMS content
  delay(100);
  gsmSerial.println((char)26); // Send the ASCII equivalent of Ctrl+Z (end of message)
  delay(100);
}
}
void buzz() {
  digitalWrite(BUZZER_PIN, HIGH); // Turn the buzzer on
  delay(100); // Buzz duration
  digitalWrite(BUZZER_PIN, LOW); // Turn the buzzer off
}
uint8_t readnumber(void) {
  uint8_t num = 0;
  while (1) {
    while (!Serial.available());
    char c = Serial.read();
    if (isdigit(c)) {
      num = num * 10 + c - '0';
    }
    if (c == '\n') break;
  }
  return num;
}
bool getFingerprintEnroll() {
  int p = -1;
  Serial.println("Waiting for valid finger to enroll");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for Finger");
  lcd.setCursor(0, 1);
  lcd.print("to Enroll...");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
  lcd.setCursor(0, 2);
  lcd.print("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }
  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");

      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return false;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return false;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return false;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return false;
    default:
      Serial.println("Unknown error");
      return false;
  }

  Serial.println("Remove finger");
 // lcd.clear();
  lcd.setCursor(0, 3);
  lcd.print("Remove Finger...");
  delay(2000);

  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  Serial.print("ID "); 
  Serial.println(id);

  p = -1;
  Serial.println("Place same finger again");
  lcd.clear();
 lcd.setCursor(0, 0);
  lcd.print("Place Same Finger");
  lcd.setCursor(0, 1);
  lcd.print("Again...");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return false;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return false;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return false;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return false;
    default:
      Serial.println("Unknown error");
      return false;
  }

  // OK converted!
  Serial.print("Creating model for #"); 
   Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    lcd.setCursor(0, 3);
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return false;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return false;
  } else {
    Serial.print("Unknown error: 0x");
     Serial.println(p, HEX);
    return false;
  }
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    lcd.clear();
    lcd.setCursor(0, 2);
    lcd.print("Stored!");
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint");
    lcd.setCursor(0, 1);
    lcd.print("Enrolled!");
   delay(2000);
  // lcd.clear();
    return true;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return false;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    return false;
  }
}
bool getFingerprintMatch() {
  int p = -1;
  Serial.println("Waiting for valid finger");
 lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for Finger");
  lcd.setCursor(0, 1);
  lcd.print("to Matching");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return false;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return false;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return false;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return false;
    default:
      Serial.println("Unknown error");
      return false;
  }
  // OK converted!
  Serial.print("Searching for match in ID #");
  Serial.println(id);
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a match!");
    lcd.clear();
    lcd.print("Fingerprint Match");
    lcd.setCursor(0, 1);
    lcd.print("Found!");
    delay(2000);
    //lcd.clear();
    return true;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return false;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("No match found");
    lcd.clear();
    lcd.print("No Match Found!");
    delay(2000);
    lcd.clear();
    return false;
  } else {
    Serial.print("Unknown error: 0x");
     Serial.println(p, HEX);
    return false;
  }
}
bool getFingerprintDelete() {
  int p = -1;
  Serial.println("Waiting for valid finger...");
 lcd.clear();
  lcd.print("Waiting for Finger");
  lcd.setCursor(0, 1);
  lcd.print("to Delete...");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return false;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return false;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return false;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return false;
    default:
      Serial.println("Unknown error");
      return false;
  }
  // OK converted!
  p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
    lcd.clear();
    lcd.print("Fingerprint");
    lcd.setCursor(0, 1);
    lcd.print("Deleted!");
    //delay(2000);
   // lcd.clear();
    return true;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return false;
  } else {
    Serial.print("Unknown error: 0x");
     Serial.println(p, HEX);
    return false;
  }
}
bool verifyFingerprint() {
  int p = -1;
  Serial.println("Place your finger");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Your Finger");
  lcd.setCursor(0, 1);
  lcd.print("on the Sensor");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return false;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        return false;
      default:
        Serial.println("Unknown error");
        return false;
    }
  }
  
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return false;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return false;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return false;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return false;
    default:
      Serial.println("Unknown error");
      return false;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint recognized!");
    return true;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return false;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Fingerprint not recognized");
    return false;
  } else {
    Serial.print("Unknown error: 0x");
    Serial.println(p, HEX);
    return false;
  }
}
