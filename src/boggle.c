#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../include/dictionary.h"
#include "../include/board_generator.h"
#include "../include/word_checker.h"
#include "../include/scoreboard.h"

#define MAX_LINE 100

/*
 * set up board,
 * set up dictionary,
 */
void incrementTotalScore(int *userScore, char *word);

void freeAndResetBoard(RolledDice** gameBoard, PresetDice* inputArrayOfDice)
{
    for (int i = 0; i < 4; i++) {
        free(gameBoard[i]);
    }
    rollDice(gameBoard, inputArrayOfDice);
}

char *convertToUpper(char **upper)
{
    char *upperDeref = *upper;

    for(int i = 0; upperDeref[i]; i++){
        upperDeref[i] = toupper(upperDeref[i]);
    }
    return upperDeref;
}

char *convertToUpper2(char (*upper)[])
{
    char *upperDeref = *upper;

    for(int i = 0; upperDeref[i]; i++){
        upperDeref[i] = toupper(upperDeref[i]);
    }
    return upperDeref;
}

void incrementTotalScore(int *userScore, char *word)
{
    int wordLen = strlen(word);
    fprintf(stdout, "word_len: %d\n",wordLen );
    if(wordLen == 3 || wordLen == 4){
        *userScore+=1;
    }else if(wordLen == 5){
        *userScore+=2;
    }else if(wordLen == 6){
        *userScore+=3;
    }else if(wordLen == 7){
        *userScore+=5;
    }else if(wordLen >= 8){
        *userScore+=11;
    }
}


int main (int argc, char ** argv) {
    int i, points = 0, testPoints = 0, invalidSize = 0;
    char inputWord[100];
    char originalInputWord[100];

    FILE *inputFP;
    char line[MAX_LINE];
    char *fileName;

    const char *dictName = "EnglishWords.txt";
    DNode *checkEnglish;
    DNode *checkSubmitted;
    static DNode *englishDictionary[BIG_HASH_SIZE];
    static DNode *guessedWords[SMALL_HASH_SIZE];

    int currentScore = 0;
    int turnCount = 0;

    User *head = NULL;
    head = (User *) malloc(sizeof(User));

    PresetDice globalDice[16];
    RolledDice *gameBoard[4];

    FILE *outputFP;
    char readLine[MAX_LINE];

    if (!(inputFP = fopen(dictName, "r"))) {
        fprintf(stderr, "Could not open file \"%s\" for reading dictionary words\n", dictName);
        return 1;
    }

    while (fgets(line, MAX_LINE, inputFP) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';  //trim new line characters
        insert(englishDictionary, BIG_HASH_SIZE, convertToUpper2(&line));
    }
    fclose(inputFP);

    //user playing in normal mode
    if (argc == 1) {
        fprintf(stdout, "playing in normal mode\n\n");

        system("clear");

        initializePresetDice(globalDice);

        rollDice(gameBoard, globalDice);

        while (turnCount >= 0) {
            strcpy(originalInputWord, inputWord);

            convertToUpper2(&inputWord);

            User *thisUser;
            char inputName[100];

            if (strcmp(originalInputWord, "q") == 0) {
                // "q" is the input, print scoreboard and exit game
                printScoreboard(head);
                break;
            }
            // "n" is the input, adds user to/changes user in linked list and
            // resets game
            if (strcmp(originalInputWord, "n") == 0) {

                printScoreboard(head);
                fprintf(stdout, "Your current score: %d \n", currentScore);
                fprintf(stdout, "What is your name? \n");
                scanf("%s", inputName);

                if (userIsInList(head, inputName) == 0) {
                    addNode(head, inputName, currentScore);
                } else {
                    updateNodeWithName(head, inputName, currentScore);
                }

                currentScore = 0;

                strcpy(inputWord, "");

                freeAndResetBoard(gameBoard, globalDice);
                turnCount = 0;
                system("clear");
                continue;
            }
            //ALEX'S CODE ABOVE THIS
            //LOGAN'S CODE BELOW THIS
            printGameBoard(gameBoard);
            checkEnglish = lookup(englishDictionary, BIG_HASH_SIZE, inputWord);

            if (checkEnglish != NULL) {
                checkSubmitted = lookup(guessedWords, SMALL_HASH_SIZE, inputWord);

                if (checkSubmitted == NULL) {
                    if (strlen(inputWord) > 2) {
                        if (wordChecker(gameBoard, inputBord)) {
                            insert(guessedWords, SMALL_HASH_SIZE, inputWord);
                            incrementTotalScore(&currentScore, inputWord);
                            fprintf(stdout, "Correct! You current score is now: %d \n", currentScore);
                        } else {
                            fprintf(stderr, "The submitted word: \'%s\'' does not abide game rules. Try again!\n",
                                    originalInputWord);
                        }
                        //commented out due to conflicting nature of code block
                        //else {
                        //fprintf(stderr, "The submitted word: \'%s\'' must be at least 3 letters long. Try again!\n",
                        //originalInputWord);
                        //}
                    } else {
                        fprintf(stderr, "You have already submitted the word: \'%s\'' Try again!\n", originalInputWord);
                    }
                } else if (turnCount > 0) {
                    fprintf(stderr, "Incorrect word: \'%s\' is not in the English Dictionary. Try again!\n",
                            originalInputWord);
                }

                fprintf(stdout, "Submit a word you found:\n");
                scanf("%s", inputWord);
                turnCount++;
                system("clear");
            }
            for (int i = 0; i < 4; i++) {
                free(gameBoard[i]);
            }
            free_all(head);
        }
        //user is in test mode
    }else if (argc == 2) {

        fileName = argv[1];
        fprintf(stdout, "playing in test mode with file: %s\n", fileName);
        FILE *testFileFP;
        char testLine[MAX_LINE];
        char *testWords;
        char **testBoard;
        int fileLineCounter = 1;
        int i, j;
        DNode *testResult;
        int begin = 0;

        // (1) read first line which is the board
        if (!(testFileFP = fopen(fileName, "r"))) {
            fprintf(stderr, "Could not open test file \'%s\' for reading\n", fileName);
            return 1;
        } else if (!(outputFP = fopen("result.txt", "w"))) {
            fprintf(stderr, "Could not open result file \'%s\' for writing\n", "result.txt");
            return 1;
        }

        while (fgets(testLine, MAX_LINE, testFileFP)!= NULL ) {

            testLine[strcspn(testLine, "\r\n")] = '\0';  //trim new line characters
            if (fileLineCounter == 1) {
                convertToBoard(testLine, &testBoard);
                // this can be removed, it is just for testing purposes
                for (i = 0; i < 4; i++) {

                    for (j = 0; j < 4; j++) {

                        if (j != 3) {
                            fprintf(stdout, "%c \t", testBoard[i][j]);
                        } else {
                            fprintf(stdout, "%c \n", testBoard[i][j]);
                        }
                    }
                }
            }
            else if (fileLineCounter >= 2) {

                for (char *ptr = strtok(test_line, ","); ptr != NULL; ptr = strtok(NULL, ",")) {
                    checkEnglish = lookup(englishDictionary, BIG_HASH_SIZE, convertToUpper(&ptr));

                    if (checkEnglish != NULL) {
                        checkSubmitted = lookup(guessedWords, SMALL_HASH_SIZE, ptr);

                        if (checkSubmitted == NULL) {

                            if (TestWordChecker(testBoard, ptr)) {
                                insert(guessedWords, SMALL_HASH_SIZE, ptr);
                                incrementTotalScore(&testPoints, p);
                                fprintf(stdout, "Correct! You total score is now: %d \n", testPoints);
                            } else {
                                if (begin == 0) {
                                    fprintf(outputFP, "%s", ptr);
                                    begin++;
                                } else {
                                    fprintf(outputFP, ",%s", ptr);
                                }
                                fprintf(stderr,
                                        "The submitted word: \'%s\'' does not abide game rules. Try again!\n", ptr);
                            }
                        } else {
                            if (begin == 0) {
                                fprintf(outputFP, "%s", ptr);
                                begin++;
                            } else {
                                fprintf(outputFP, ",%s", ptr);
                            }
                            fprintf(stderr, "You have already submitted the word: \'%s\'' Try again!\n", ptr);
                        }
                    } else {
                        if (begin == 0) {
                            fprintf(outputFP, "%s", ptr);
                            begin++;
                        } else {
                            fprintf(outputFP, ",%s", ptr);
                        }
                        fprintf(stderr, "Incorrect word: \'%s\'' is not in the English Dictionary. Try again!\n",
                                ptr);
                    }
                }
            }
            fileLineCounter++;
        }

        fprintf(outputFP, "\n");
        fprintf(outputFP, "%d\n", testPoints);
        fclose(testFileFP);

        for (int i = 0; i < 4; i++) {
            free(testBoard[i]);
        }

        free(testBoard);
        fclose(outputFP);
    }

    freeDictionary(englishDictionary, BIG_HASH_SIZE);
    freeDictionary(guessedWords, SMALL_HASH_SIZE);

    return 0;
}

