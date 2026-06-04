#include <LedControl.h>

//Snake Structure
typedef struct Snake Snake;
struct Snake{
  int head[2];     // the (row, column) of the snake head
  int body[40][2]; //An array that contains the (row, column) coordinates
  int len;         //The length of the snake 
  int dir[2];      //A direction to move the snake along
};

//Bait Structure
typedef struct Bait Bait;
struct Bait{
  int rPos; //The row index of the bait
  int cPos; //The column index of the bait
};

//LED Dot Matrix, Joystick, LED and Button 
const int DIN =13;
const int CS =32;
const int CLK = 33;
LedControl lc = LedControl(DIN, CLK, CS,1);

const int varXPin = 12;  // Joystick X value
const int varYPin = 14;  // Joystick Y value

const int LED =15; //Snake is dead 
const int PAUSE = 26;//STOP button
const int RESET =25; //RESET button
const int LED1 = 23; //Snake ate the bait 
const int LED2 = 22; //Game is paused


byte pic[8] = {0,0,0,0,0,0,0,0};

Snake snake = {{1,5},{{0,5}, {1,5}}, 2, {1,0}};
Bait bait = {(int)random(0,8),(int)random(0,8)};


float oldTime = 0;
float timer = 0;
float updateRate = 3;

int reset_button_state;       // the current state of button
int reset_last_button_state; 

int pause_button_state;       // the current state of button
int pause_last_button_state; 

int i,j;//Counters
void setup() {
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);

  //Set Joystick Pins as INPUTs
  pinMode(varXPin, INPUT);
  pinMode(varYPin, INPUT);
  pinMode(LED, OUTPUT); // Snake is dead (RED LED)
  pinMode(LED1, OUTPUT); // Snake ate the bait (GREEN LED)
  pinMode(LED2, OUTPUT); // You paused the game (YELLOW LED)
  pinMode(RESET, INPUT_PULLUP); // Reset Button
  pinMode(PAUSE, INPUT_PULLUP); // Pause Button
}

void loop() {
  float deltaTime = calculateDeltaTime();
  timer += deltaTime;

  //Check For Inputs
  int xVal = analogRead(varXPin);
  int yVal = analogRead(varYPin);
  
  if(xVal==0 && snake.dir[1]==0){
    snake.dir[0] = 0;
    snake.dir[1] = -1;
  }else if(xVal==4095 && snake.dir[1]==0){
    snake.dir[0] = 0;
    snake.dir[1] = 1;
  }else if(yVal==0 && snake.dir[0]==0){
    snake.dir[0] = -1;
    snake.dir[1] = 0;
  }else if(yVal==4095 && snake.dir[0]==0){
    snake.dir[0] = 1;
    snake.dir[1] = 0;
  }
  
  
  if(timer > 1000/updateRate){
    timer = 0;
    Update();
  }
  
  
  Render();
  
}

float calculateDeltaTime(){
  float currentTime = millis();
  float dt = currentTime - oldTime;
  oldTime = currentTime;
  return dt;
}

void reset(){
  for(int j=0;j<8;j++){
    pic[j] = 0;
  }
}
void Update(){
  reset(); // Reset matrix
  
  int newHead[2] = {snake.head[0]+snake.dir[0], snake.head[1]+snake.dir[1]};

  //Handle Borders
  if(newHead[0]==8){
    newHead[0]=0;
  }else if(newHead[0]==-1){
    newHead[0] = 7;
  }else if(newHead[1]==8){
    newHead[1]=0;
  }else if(newHead[1]==-1){
    newHead[1]=7;
  }

  //Reset button
  reset_last_button_state = reset_button_state;  //save last button state  
  reset_button_state = digitalRead(RESET); // new button state

  if(reset_last_button_state == HIGH && reset_button_state == LOW){
      //LED blinks once
      digitalWrite(LED,HIGH);
      delay(1000);
      digitalWrite(LED,LOW);

      delay(1000);
      snake = {{1,5},{{0,5}, {1,5}}, 2, {1,0}};//Reinitialize the snake object
      bait = {(int)random(0,8),(int)random(0,8)};//Reinitialize a bait object
      return;
  }

  //Pause button
  pause_last_button_state = pause_button_state;      // //save last button state 
  pause_button_state = digitalRead(PAUSE); // new button state

  if(pause_last_button_state == HIGH && pause_button_state == LOW){
    static bool isPaused = false;
    isPaused = !isPaused; 

    while (isPaused) {
      // loop for syopping the game
      delay(100); 
        //LED blinks once
      digitalWrite(LED2,HIGH);
      // check the pause button
      pause_last_button_state = pause_button_state;
      pause_button_state = digitalRead(PAUSE);
        
      if (pause_last_button_state == HIGH && pause_button_state == LOW) {
        isPaused = !isPaused; // continiue 
        digitalWrite(LED2,LOW);
            break; // exit loop
        }
    }
    return;
  }

  //Check If The Snake hits itself
   for(j=0;j<snake.len;j++){
    if(snake.body[j][0] == newHead[0] && snake.body[j][1] == newHead[1]){
      //LED blinks once
      digitalWrite(LED,HIGH);
      delay(1000);
      digitalWrite(LED,LOW);
      
      delay(1000);
      snake = {{1,5},{{0,5}, {1,5}}, 2, {1,0}};//Reinitialize the snake object
      bait = {(int)random(0,8),(int)random(0,8)};//Reinitialize bait object
      return;
    }
  }

  //Check if The snake ate the bait
  if(newHead[0] == bait.rPos && newHead[1] ==bait.cPos){
    digitalWrite(LED1,HIGH);
    delay(250);
    digitalWrite(LED1,LOW); 
    snake.len = snake.len+1;
    bait.rPos = (int)random(0,8);
    bait.cPos = (int)random(0,8);
    
  }else{
    removeFirst();
  } 
   
  snake.body[snake.len-1][0]= newHead[0];
  snake.body[snake.len-1][1]= newHead[1];
  
  snake.head[0] = newHead[0];
  snake.head[1] = newHead[1];
  
  //Update the pic Array to Display(snake and bait)
  for(j=0;j<snake.len;j++){
    pic[snake.body[j][0]] |= 128 >> snake.body[j][1];
  }
  pic[bait.rPos] |= 128 >> bait.cPos;
  
}

void Render(){
  
   for(i=0;i<8;i++){
    lc.setRow(0,i,pic[i]);
   }
}

void removeFirst(){
  for(j=1;j<snake.len;j++){
    snake.body[j-1][0] = snake.body[j][0];
    snake.body[j-1][1] = snake.body[j][1];
  }
}