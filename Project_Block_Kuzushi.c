#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>

#define PADDLE_WIDTH 120
#define PADDLE_HEIGHT 20
#define BALL_RADIUS 10
#define BRICK_COLUMNS 10
#define BRICK_ROWS 5
#define BRICK_WIDTH 120
#define BRICK_HEIGHT 35
#define BRICK_SPACING 10

typedef enum {
    GAME_MAIN_MENU,
    GAME_PLAYING,
    GAME_OVER,
    GAME_WIN
} game_state_e;

typedef struct Ball {
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
} Ball;

typedef struct Paddle {
    Rectangle rect;
    int lives;
} Paddle;

typedef struct Brick {
    Rectangle rect;
    bool active;
} Brick;

void ResetBall(Ball *ball, Paddle *paddle);
void ResetBricks(Brick bricks[BRICK_ROWS][BRICK_COLUMNS], int screenWidth, int level);


int main()
{
    const int SCREEN_WIDTH = 1600;
    const int SCREEN_HEIGHT = 900;
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Block Kuzushi Game");
    SetTargetFPS(60);

    Paddle paddle = {{SCREEN_WIDTH / 2 - PADDLE_WIDTH / 2, SCREEN_HEIGHT - 50, PADDLE_WIDTH, PADDLE_HEIGHT}, 3};

    Ball ball;
    ResetBall(&ball, &paddle);

    Brick bricks[BRICK_ROWS][BRICK_COLUMNS];
    int level = 1; // Početni nivo
    ResetBricks(bricks, SCREEN_WIDTH, level);

    game_state_e gameState = GAME_MAIN_MENU;
    bool nextLevel = false;

    int points = 0;
    int pointsToNextLevel = 100;

    while (!WindowShouldClose()) {
        switch (gameState) {
            case GAME_MAIN_MENU:
                if (IsKeyPressed(KEY_SPACE)) {
                    gameState = GAME_PLAYING;
                    ball.active = true;
                }
                break;

            case GAME_PLAYING:
                if (IsKeyDown(KEY_LEFT) && paddle.rect.x > 0) {
                    paddle.rect.x -= 20.0f;
                }
                if (IsKeyDown(KEY_RIGHT) && paddle.rect.x + paddle.rect.width < SCREEN_WIDTH) {
                    paddle.rect.x += 20.0f;
                }

                if (ball.active) {
                    ball.position = Vector2Add(ball.position, ball.speed);

                    if (ball.position.x <= ball.radius || ball.position.x >= SCREEN_WIDTH - ball.radius) {
                        ball.speed.x *= -1;
                    }
                    if (ball.position.y <= ball.radius) {
                        ball.speed.y *= -1;
                    }

                    if (CheckCollisionCircleRec(ball.position, ball.radius, paddle.rect)) {
                        ball.speed.y *= -1;
                        ball.position.y = paddle.rect.y - ball.radius;
                    }

                    for (int i = 0; i < BRICK_ROWS; i++) {
                        for (int j = 0; j < BRICK_COLUMNS; j++) {
                            if (bricks[i][j].active && CheckCollisionCircleRec(ball.position, ball.radius, bricks[i][j].rect)) {
                                bricks[i][j].active = false;
                                ball.speed.y *= -1;
                                points += 10;

                                if (points >= pointsToNextLevel) {
                                    nextLevel = true;
                                }
                            }
                        }
                    }

                    if (ball.position.y > SCREEN_HEIGHT) {
                        paddle.lives--;
                        if (paddle.lives <= 0) {
                            gameState = GAME_OVER;
                        } else {
                            ResetBall(&ball, &paddle);
                            ball.active = true;
                        }
                    }
                }

                bool allBricksCleared = true;
                for (int i = 0; i < BRICK_ROWS; i++) {
                    for (int j = 0; j < BRICK_COLUMNS; j++) {
                        if (bricks[i][j].active) {
                            allBricksCleared = false;
                            break;
                        }
                    }
                }
                if (allBricksCleared) {
                    gameState = GAME_WIN;
                }
                break;

            case GAME_OVER:
                if (IsKeyPressed(KEY_R)) {
                    ResetBall(&ball, &paddle);
                    ResetBricks(bricks, SCREEN_WIDTH, level);
                    gameState = GAME_MAIN_MENU;
                    paddle.lives = 3;
                    points = 0;
                    level = 1;
                    pointsToNextLevel = 200;
                } else if (IsKeyPressed(KEY_Q)) {
                    break;
                }
                break;

            case GAME_WIN:
                if (IsKeyPressed(KEY_N)) {
                    nextLevel = true;
                } else if (IsKeyPressed(KEY_Q)) {
                    break;
                }
                break;
        }

        if (nextLevel) {
            level++;
            pointsToNextLevel += 00; 
            ResetBall(&ball, &paddle);
            ResetBricks(bricks, SCREEN_WIDTH, level);
            nextLevel = false; 
            gameState = GAME_PLAYING; 
            points = 0;          
            ball.active = true;
        }        

        BeginDrawing();
        ClearBackground(BLACK);

        switch (gameState) {
            case GAME_MAIN_MENU:
                DrawText("Welcome to Block Kuzushi Game!\n\n Press SPACEBAR to play!", SCREEN_WIDTH / 2 - 400, SCREEN_HEIGHT / 2 - 50, 50, WHITE);
                break;

            case GAME_PLAYING:
                DrawRectangleRec(paddle.rect, GREEN);
                DrawCircleV(ball.position, ball.radius, WHITE);

                for (int i = 0; i < BRICK_ROWS; i++) {
                    for (int j = 0; j < BRICK_COLUMNS; j++) {
                        if (bricks[i][j].active) {
                            DrawRectangleRec(bricks[i][j].rect, RED);
                        }
                    }
                }

                DrawText(TextFormat("Lives: %d", paddle.lives), 1450, 30, 30, YELLOW);
                DrawText(TextFormat("Points: %d", points), 20, 30, 30, YELLOW);
                DrawText(TextFormat("Level: %d", level), SCREEN_WIDTH / 2 - 50, 30, 30, YELLOW);
                break;

            case GAME_OVER:
                DrawText("GAME OVER!", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2 - 50, 100, RED);
                DrawText("Press R to Restart or Q to Quit", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2 + 50, 40, WHITE);
                break;

            case GAME_WIN:
                DrawText("YOU WIN!", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 100, GREEN);
                DrawText("Press N for Next Level or Q to Quit", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2 + 50, 40, WHITE);
                break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void ResetBall(Ball *ball, Paddle *paddle) {
    ball->position = (Vector2){paddle->rect.x + paddle->rect.width / 2, paddle->rect.y - BALL_RADIUS};
    ball->speed = (Vector2){10, -10};
    ball->radius = BALL_RADIUS;
    ball->active = false;
}

void ResetBricks(Brick bricks[BRICK_ROWS][BRICK_COLUMNS], int screenWidth, int level) {
    float totalBricksWidth = BRICK_COLUMNS * BRICK_WIDTH + (BRICK_COLUMNS - 1) * BRICK_SPACING;
    float startX = (screenWidth - totalBricksWidth) / 2;

    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLUMNS; j++) {
            bricks[i][j].rect = (Rectangle){
                startX + j * (BRICK_WIDTH + BRICK_SPACING),
                100 + i * (BRICK_HEIGHT + BRICK_SPACING),
                BRICK_WIDTH,
                BRICK_HEIGHT
            };            
            bricks[i][j].active = (level % 2 == 0) ? (j % 2 == 0) : (i % 2 == 0);
        }
    }
}