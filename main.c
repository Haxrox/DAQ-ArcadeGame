/*
 * Author: Bell Chen
 * Lab Section: L1X
 * Date: 12/18/2020 10:10:10 AM
 *
 */
#define _CRT_SECURE_NO_WARNINGS

/* Headers */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <DAQlib.h>

/* Simulator */
#define SIMULATOR 3

/* Symbolic Constants */
#define TRUE 1
#define FALSE 0
#define ON 1
#define OFF 0

/* States */
#define INITIALIZE 0
#define PLAY 1
#define LOSE 2

/* Constants */
#define ONE_SECOND 1000
/* Slider */
#define SLIDER 0
#define SLIDER_POSITIONS 5
/* Bricks */
#define NUM_BRICKS 5
/* Blinking */
#define RED_LED 4
#define NUM_BLINKS 6
#define START_SECONDS 3
/* Update health delay */
#define UPDATE_DELAY 1000
#define MAX_UPDATE 750
#define UPDATE_MULTIPLIER 5
/* Amount of health to add */
#define HEALTH_MULTIPLIER 50
#define MAX_HEALTH 10

/* function prototypes */
/* 
 * Purpose: Handles the game functions
 */
void playGame(void);
/*
 * Purpose: Update brick's HP and returns whether all bricks have health or not
 * 
 * Param: hp - Array of all brick HP's
 * Param: brick - Brick to update
 * Param: multiplier - How much HP to add to the brick
 * 
 * Assumptions: brick < NUM_BRICKS
 */
int updateHp(int hp[], int brick, int multiplier);
/*
 * Purpose: Damages the brick and increments the score
 *
 * Param: hp - Array of all brick HP's
 * Param: brick - Brick to damage
 * Param: *score - Pointer to the player's current score
 *
 * Assumptions: brick < NUM_BRICKS
 */
void damageBrick(int hp[], int brick, int* score);
/*
 * Purpose: Resets all bricks HP by setting each value in the array to 0
 * 
 * Param: hp - Array of all brick HP's
 * 
 * Assumptions: hp is initialized correctly
 */
void resetHp(int hp[]);
/*
 * Purpose: Generates a random number
 * 
 * Param: lowerBound - Lowerbound of the random number generation
 * Param: upperBound - Upperbound of the random number generation
 * 
 */
int randomNumber(int lowerBound, int upperBound);
/*
 * Purpose: Returns the minimum between two values
 * 
 * Param: val1 - Value wanting to clamp
 * Param: max - Maximum value
 * 
 */
int minimum(int val1, int max);

/* Main */
int main(void) {

	// Initialize the DAQ module
	if (setupDAQ(SIMULATOR)) {
		playGame();
	} else {
		printf("ERROR: Cannot initialize DAQ\n");
	}

	system("PAUSE"); /* pause to view output */
	return 0;
}

/* Functions */
void playGame(void) {
	int state = INITIALIZE;
	int highScore = 0;
	int currentScore = 0;

	int hp[NUM_BRICKS];
	int updateBrick = 0;
	resetHp(hp);

	int startGame = millis() + ONE_SECOND;
	int startCounter = 0;

	int blinkLED = 0;
	int blinkCounter = 0;

	const static double sliderPositions[SLIDER_POSITIONS] = { 1.0, 2.4, 3.75, 5, 10};

	while (continueSuperLoop()) {
		int currentTime = millis();
		switch (state) {
		case INITIALIZE:
			if (currentTime > startGame) {
				if (startCounter >= 3) {
					printf("Begin!\n");
					state = PLAY;
					startCounter = 0;
				} else {
					printf("Starting game in %d seconds ...\n", START_SECONDS - startCounter);
					startGame = currentTime + ONE_SECOND;
					startCounter++;
				}
			}
			break;
		case PLAY:
			// Handle updating
			if (currentTime > updateBrick) {
				if (updateHp(hp, randomNumber(0, NUM_BRICKS - 1), randomNumber(1, minimum(currentScore / HEALTH_MULTIPLIER, MAX_HEALTH)))) {
					state = LOSE;
				} else {
					updateBrick = currentTime + (UPDATE_DELAY - minimum(currentScore * UPDATE_MULTIPLIER, MAX_UPDATE));
				}
			}
			// Handle damaging
			int damaged = FALSE;
			double position = analogRead(SLIDER);
			for (int index = 0; index < SLIDER_POSITIONS && !damaged; index++) {
				if (position < sliderPositions[index]) {
					damageBrick(hp, index, &currentScore);
					damaged = TRUE;
				}
			}
			break;
		case LOSE:
			if (blinkCounter == 0) {
				resetHp(hp);
				printf("You lost! You got a score of %d\n", currentScore);
				if (currentScore > highScore) {
					printf("Congratulations, you beat your highscore!\n");
					highScore = currentScore;
				}
				printf("Current highscore: %d\n", highScore);
			}

			if (currentTime > blinkLED) {
				digitalWrite(RED_LED, blinkCounter % 2);
				blinkCounter++;
				if (blinkCounter % NUM_BLINKS == 0) {
					state = INITIALIZE;
					currentScore = 0;
					blinkCounter = 0;
					startGame = currentTime + ONE_SECOND;
					srand((unsigned)time(NULL));
				} else {
					blinkLED = currentTime + ONE_SECOND / 2;
				}
			}
			break;
		}
	}
}

int updateHp(int hp[], int brick, int multiplier) {
	hp[brick] += multiplier;
	digitalWrite(brick, ON);

	int full = TRUE;
	for (int index = 0; index < NUM_BRICKS; index++) {
		if (hp[index] <= 0) {
			digitalWrite(index, OFF);
			full = FALSE;
		} else {
			digitalWrite(index, ON);
		}
	}

	return full;
}

void resetHp(int hp[]) {
	for (int index = 0; index < NUM_BRICKS; index++) {
		hp[index] = 0;
		digitalWrite(index, OFF);
	}
}

void damageBrick(int hp[], int brick, int* score) {
	if (hp[brick] > 0) {
		hp[brick]--;
		digitalWrite(brick, OFF);
		if (hp[brick] > 0) {
			digitalWrite(brick, ON);
		}
		*score = *score + 1;
	}
}

int randomNumber(int lowerBound, int upperBound) {
	if (upperBound < lowerBound) {
		return lowerBound;
	} else {
		return rand() % (upperBound - lowerBound + 1) + lowerBound;
	}
}

int minimum(int val1, int max) {
	if (val1 > max) {
		return max;
	} else {
		return val1;
	}
}