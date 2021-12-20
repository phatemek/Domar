#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>

#pragma warning(disable:4996)

//colors
#define RED 12
#define BLUE 3
#define GREEN 10
#define YELLOW 14
#define GRAY 8
#define PINK 13
#define WHITE 15
#define WAIT_TIME_MILI_SEC 100
//directions
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
// general
#define BOARD_SIZE 40
#define INITIAL_SNAKE_LENGTH 3
#define MINIMUM_SNAKE_LENGTH 2
#define MAX_LEN_SNAKES 30
#define NUMBER_OF_MOUSES 20
//board_characters
#define EMPTY '0'
#define MOUSE 'm'
#define PLAYER1_SNAKE_HEAD '1'
#define PLAYER2_SNAKE_HEAD '2'
#define PLAYER1_SNAKE_BODY 'a'
#define PLAYER2_SNAKE_BODY 'b'
//Bernard, Poison and golden star
#define BERNARD_CLOCK 'c' //on the board character
#define GOLDEN_STAR '*' //on the board character
#define POISON 'x' //on the board character
#define NUMBER_OF_POISONS 5
#define NUMBER_OF_GOLDEN_STARS 3
#define BERNARD_CLOCK_APPEARANCE_CHANCE_PERCENT 20
#define BERNARD_CLOCK_APPEARANCE_CHECK_PERIOD_MILI_SEC 2000
#define BERNARD_CLOCK_FROZEN_TIME_MILI_SEC 4000

CONSOLE_FONT_INFOEX former_cfi;
CONSOLE_CURSOR_INFO former_info;
COORD former_screen_size;

void reset_console() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleDisplayMode(consoleHandle, CONSOLE_WINDOWED_MODE, &former_screen_size);
    SetCurrentConsoleFontEx(consoleHandle, FALSE, &former_cfi);
    SetConsoleCursorInfo(consoleHandle, &former_info);
}

void hidecursor(HANDLE consoleHandle)
{
    GetConsoleCursorInfo(consoleHandle, &former_info);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void set_console_font_and_font_size(HANDLE consoleHandle) {
    former_cfi.cbSize = sizeof(former_cfi);
    GetCurrentConsoleFontEx(consoleHandle, FALSE, &former_cfi);
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 15;
    cfi.dwFontSize.Y = 15;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy(cfi.FaceName, L"Tahoma");
    SetCurrentConsoleFontEx(consoleHandle, FALSE, &cfi);
}

void set_full_screen_mode(HANDLE consoleHandle) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    former_screen_size.X = csbi.dwSize.X; former_screen_size.Y = csbi.dwSize.Y;
    COORD coord;
    SetConsoleDisplayMode(consoleHandle, CONSOLE_FULLSCREEN_MODE, &coord);
}

void init_screen()
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    set_full_screen_mode(consoleHandle);
    hidecursor(consoleHandle);
    set_console_font_and_font_size(consoleHandle);

}

void wait_and_get_direction(int* player1_snake_direction, int* player2_snake_direction) {
    DWORD64 start_time, check_time;
    start_time = GetTickCount64();
    check_time = start_time + WAIT_TIME_MILI_SEC; //GetTickCount returns time in miliseconds
    char key = 0;
    char player1_key_hit = 0;
    char player2_key_hit = 0;

    while (check_time > GetTickCount64()) {
        if (_kbhit()) {
            key = _getch();
            if (key == 0)
                key = _getch();
            if (key == 'w' || key == 'a' || key == 's' || key == 'd')
                player1_key_hit = key;
            if (key == 'i' || key == 'j' || key == 'k' || key == 'l')
                player2_key_hit = key;
        }
    }

    switch (player1_key_hit) {
        case 'w': if (*player1_snake_direction != DOWN) *player1_snake_direction = UP; break;
        case 'a': if (*player1_snake_direction != RIGHT) *player1_snake_direction = LEFT; break;
        case 's': if (*player1_snake_direction != UP) *player1_snake_direction = DOWN; break;
        case 'd': if (*player1_snake_direction != LEFT) *player1_snake_direction = RIGHT; break;
        default: break;
    }

    switch (player2_key_hit) {
        case 'i': if (*player2_snake_direction != DOWN) *player2_snake_direction = UP; break;
        case 'j': if (*player2_snake_direction != RIGHT) *player2_snake_direction = LEFT; break;
        case 'k': if (*player2_snake_direction != UP) *player2_snake_direction = DOWN; break;
        case 'l': if (*player2_snake_direction != LEFT) *player2_snake_direction = RIGHT; break;
        default: break;
    }
}

void draw_point(char point_content) {
    switch (point_content) {
        case PLAYER1_SNAKE_HEAD: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED); printf("@"); break;
        case PLAYER2_SNAKE_HEAD: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLUE);  printf("@"); break;
        case PLAYER1_SNAKE_BODY: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED);  printf("o"); break;
        case PLAYER2_SNAKE_BODY: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BLUE);  printf("o"); break;
        case MOUSE: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GRAY); printf("m"); break;
        case GOLDEN_STAR: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW); printf("*"); break;
        case POISON: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN); printf("x"); break;
        case BERNARD_CLOCK: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), PINK); printf("c"); break;
        default: printf(" ");
    }
}

void draw_horizonatal_walls() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
    for (int i = 0; i < BOARD_SIZE + 2; ++i)
        printf("-");
    printf("\n");
}

void draw_board(char board_content[BOARD_SIZE][BOARD_SIZE]) {
    system("cls");
    draw_horizonatal_walls();
    for (int i = 0; i < BOARD_SIZE; i++) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
        printf("|"); // vertical wall
        for (int j = 0; j < BOARD_SIZE; j++)
            draw_point(board_content[i][j]);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
        printf("|\n"); // vertical wall
    }
    draw_horizonatal_walls();
}
void move_snake_body(int snake_length,int snake_cordinates[MAX_LEN_SNAKES][2]){
    for(int i = snake_length - 1; i > 0; i--){
        snake_cordinates[i][0] = snake_cordinates[i - 1][0];
        snake_cordinates[i][1] = snake_cordinates[i - 1][1];
    }
}

// prototypes
void init_screen();
void reset_console();
void wait_and_get_direction(int* player1_snake_direction, int* player2_snake_direction);
void draw_board(char board_content[BOARD_SIZE][BOARD_SIZE]);

int main() {
    char board_content[BOARD_SIZE][BOARD_SIZE];
    char player1_snake[BOARD_SIZE][BOARD_SIZE];
    char player2_snake[BOARD_SIZE][BOARD_SIZE];
    char mouses[BOARD_SIZE][BOARD_SIZE];
    int player1_snake_cordinates[MAX_LEN_SNAKES][2];
    int player2_snake_cordinates[MAX_LEN_SNAKES][2];
    int player1_snake_direction, player2_snake_direction;
    int snake1_length = INITIAL_SNAKE_LENGTH;
    int snake2_length = INITIAL_SNAKE_LENGTH;
    int result = 0;

    for(int i = 0; i < BOARD_SIZE; i++) { // empty mouses
        for (int j = 0; j < BOARD_SIZE; j++) {
            mouses[i][j] = EMPTY;
        }
    }

    for(int i = 0; i < MAX_LEN_SNAKES; i++){ // put -1 in every snake cordinate
        player1_snake_cordinates[i][0] = -1;
        player1_snake_cordinates[i][1] = -1;
        player2_snake_cordinates[i][0] = -1;
        player2_snake_cordinates[i][1] = -1;
    }

    for(int i = 1; i < INITIAL_SNAKE_LENGTH ; i++){ // initial snake body cordinates
        player2_snake_cordinates[i][0] = INITIAL_SNAKE_LENGTH - i - 1;
        player2_snake_cordinates[i][1] = BOARD_SIZE - 1;
        player1_snake_cordinates[i][0] = BOARD_SIZE - (INITIAL_SNAKE_LENGTH - i);
        player1_snake_cordinates[i][1] = 0;
    }
    player2_snake_cordinates[0][0] = INITIAL_SNAKE_LENGTH - 1; // snake head cordinates
    player2_snake_cordinates[0][1] = BOARD_SIZE - 1;
    player1_snake_cordinates[0][0] = BOARD_SIZE - INITIAL_SNAKE_LENGTH;
    player1_snake_cordinates[0][1] = 0;

    init_screen();
    player2_snake_direction = DOWN;
    player1_snake_direction = UP;

    while (TRUE) {
        for(int i = 0; i < BOARD_SIZE; i++) { // empty board content and snakes
            for (int j = 0; j < BOARD_SIZE; j++) {
                board_content[i][j] = EMPTY;
                player1_snake[i][j] = EMPTY;
                player2_snake[i][j] = EMPTY;
            }
        }

        player1_snake[player1_snake_cordinates[0][0]][player1_snake_cordinates[0][1]] = PLAYER1_SNAKE_HEAD; // snake heads
        player2_snake[player2_snake_cordinates[0][0]][player2_snake_cordinates[0][1]] = PLAYER2_SNAKE_HEAD;
        for(int i = 1; player1_snake_cordinates[i][0] != -1 ; i++){ // snake bodies
            player1_snake[player1_snake_cordinates[i][0]][player1_snake_cordinates[i][1]] = PLAYER1_SNAKE_BODY;
        }
        for(int i = 1; player2_snake_cordinates[i][0] != -1 ; i++){
            player2_snake[player2_snake_cordinates[i][0]][player2_snake_cordinates[i][1]] = PLAYER2_SNAKE_BODY;
        }

        for(int i = 0; i < BOARD_SIZE; i++){ // merge snakes and mouses and board content
            for(int j = 0; j < BOARD_SIZE; j++){
                if(player1_snake[i][j] != EMPTY){
                    board_content[i][j] = player1_snake[i][j];
                }
                else if(player2_snake[i][j] != EMPTY){
                    board_content[i][j] = player2_snake[i][j];
                }
                else if(mouses[i][j] != EMPTY){
                    board_content[i][j] = mouses[i][j];
                }
            }
        }

        int count_mouses = 0; // count mouses
        for(int i = 0; i < BOARD_SIZE; i++){
            for(int j = 0; j < BOARD_SIZE; j++){
                if(mouses[i][j] == MOUSE){
                    count_mouses++;
                }
            }
        }
        srand(time(0));
        while(count_mouses < NUMBER_OF_MOUSES){ // draw 20 mouses
            int i = rand() % BOARD_SIZE;
            int j = rand() % BOARD_SIZE;
            if(board_content[i][j] == EMPTY){
                mouses[i][j] = MOUSE;
                count_mouses++;
            }
        }
        for(int i = 0; i < BOARD_SIZE; i++){ // add new mouses to board content
            for(int j = 0; j < BOARD_SIZE; j++){
                if(mouses[i][j] != EMPTY){
                    if(board_content[i][j] == EMPTY) {
                        board_content[i][j] = mouses[i][j];
                    }
                }
            }
        }

        draw_board(board_content); // draw board
        wait_and_get_direction(&player1_snake_direction, &player2_snake_direction);
        int snake1_head_i = player1_snake_cordinates[0][0]; // snake1 eats mouses
        int snake1_head_j = player1_snake_cordinates[0][1];
        if(mouses[snake1_head_i][snake1_head_j] == MOUSE && snake1_length < MAX_LEN_SNAKES - 1){
            mouses[snake1_head_i][snake1_head_j] = EMPTY;
            player1_snake_cordinates[snake1_length][0] = player1_snake_cordinates[snake1_length - 1][0];
            player1_snake_cordinates[snake1_length][1] = player1_snake_cordinates[snake1_length - 1][1];
            snake1_length++;
        }
        int snake2_head_i = player2_snake_cordinates[0][0]; // snake2 eats mouses
        int snake2_head_j = player2_snake_cordinates[0][1];
        if(mouses[snake2_head_i][snake2_head_j] == MOUSE && snake2_length < MAX_LEN_SNAKES - 1) {
            mouses[snake2_head_i][snake2_head_j] = EMPTY;
            player2_snake_cordinates[snake2_length][0] = player2_snake_cordinates[snake2_length - 1][0];
            player2_snake_cordinates[snake2_length][1] = player2_snake_cordinates[snake2_length - 1][1];
            snake2_length++;
        }

        switch(player1_snake_direction) { // snake1 movement
            case DOWN:
                if (player1_snake_cordinates[0][0] + 1 == BOARD_SIZE) {
                    move_snake_body(snake1_length, player1_snake_cordinates);
                    player1_snake_cordinates[0][0] = 0;
                }
                else {
                    move_snake_body(snake1_length, player1_snake_cordinates);
                    player1_snake_cordinates[0][0]++;
                }
                break;
            case UP:
                if(player1_snake_cordinates[0][0] - 1 == -1){
                    move_snake_body(snake1_length, player1_snake_cordinates);
                    player1_snake_cordinates[0][0] = BOARD_SIZE - 1;
                }
                move_snake_body(snake1_length, player1_snake_cordinates);
                player1_snake_cordinates[0][0]--;
                break;
            case RIGHT:
                if(player1_snake_cordinates[0][1] + 1 == BOARD_SIZE){
                    move_snake_body(snake1_length, player1_snake_cordinates);
                    player1_snake_cordinates[0][1] = 0;
                }
                move_snake_body(snake1_length, player1_snake_cordinates);
                player1_snake_cordinates[0][1]++;
                break;
            case LEFT:
                if(player1_snake_cordinates[0][1] + 1 == -1){
                    move_snake_body(snake1_length, player1_snake_cordinates);
                    player1_snake_cordinates[0][1] = BOARD_SIZE - 1;
                }
                move_snake_body(snake1_length, player1_snake_cordinates);
                player1_snake_cordinates[0][1]--;
                break;
        }

        switch(player2_snake_direction) { // snake2 movement
            case DOWN:
                if(player2_snake_cordinates[0][0] + 1 == BOARD_SIZE){
                    move_snake_body(snake2_length, player2_snake_cordinates);
                    player2_snake_cordinates[0][0] = 0;
                }
                move_snake_body(snake2_length, player2_snake_cordinates);
                player2_snake_cordinates[0][0]++;
                break;
            case UP:
                if(player2_snake_cordinates[0][0] - 1 == -1){
                    move_snake_body(snake2_length, player2_snake_cordinates);
                    player2_snake_cordinates[0][0] = BOARD_SIZE - 1;
                }
                move_snake_body(snake2_length, player2_snake_cordinates);
                player2_snake_cordinates[0][0]--;
                break;
            case RIGHT:
                if(player2_snake_cordinates[0][1] + 1 == BOARD_SIZE){
                    move_snake_body(snake2_length, player2_snake_cordinates);
                    player2_snake_cordinates[0][1] = 0;
                }
                move_snake_body(snake2_length, player2_snake_cordinates);
                player2_snake_cordinates[0][1]++;
                break;
            case LEFT:
                if(player2_snake_cordinates[0][1] - 1 == -1){
                    move_snake_body(snake2_length, player2_snake_cordinates);
                    player2_snake_cordinates[0][1] = BOARD_SIZE - 1;
                }
                move_snake_body(snake2_length, player2_snake_cordinates);
                player2_snake_cordinates[0][1]--;
                break;
        }
        for(int i = 1; i < snake1_length; i++){ // ending conditions
            if(player1_snake_cordinates[i][0] == player1_snake_cordinates[0][0] && player1_snake_cordinates[i][1] == player1_snake_cordinates[0][1]){
                result = 2;
                break;
            }
            else if(player2_snake_cordinates[i][0] == player2_snake_cordinates[0][0] && player2_snake_cordinates [i][1] == player2_snake_cordinates[0][1]){
                result = 1;
                break;
            }
            else if(player1_snake_cordinates[0][0] == player2_snake_cordinates[i][0] && player1_snake_cordinates[0][1] == player2_snake_cordinates[i][1]){
                result = 2;
                break;
            }
            else if(player2_snake_cordinates[0][0] == player1_snake_cordinates[i][0] && player2_snake_cordinates[0][1] == player1_snake_cordinates[i][1]){
                result = 1;
                break;
            }
        }
        if(player1_snake_cordinates[0][0] == player2_snake_cordinates[0][0] && player1_snake_cordinates[0][1] == player2_snake_cordinates[0][1]){
            if(snake1_length > snake2_length){
                result = 1;
            }
            else if(snake1_length  < snake2_length){
                result = 2;
            }
            else{
                result = 3;
            }
        }
        if(result){
            reset_console();
            break;
        }
    }
    switch (result) {
        case 1:
            printf("player 1 won this round!");
            break;
        case 2:
            printf("player 2 won this round!");
            break;
        case 3:
            printf("Draw.");
            break;
    }
    return 0;
}
