#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_TITLE "Window title"

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (450)

#define PAD_OFFSET (20)
#define PAD_WIDTH (30)
#define PAD_HEIGHT (90)

#define PAD_VELOCITY (250)

#define BALL_VELOCITY (250)
#define BALL_SIZE (10)

#define INITIAL_PAD_Y (SCREEN_HEIGHT / 2.0 - PAD_HEIGHT / 2.0)

#define SUPPORT_TRACELOG 1

typedef struct {
  int left;
  int right;
} Score;

typedef struct {
  Vector2 position;
  Vector2 direction;
  Texture2D texture;
} Ball;

typedef struct {
  int key_up;
  int key_down;
} Controller;

typedef struct {
  Vector2 position;
  Texture2D texture;
  Controller controller;
} Player;

void update_player(Player *p) {
  if (IsKeyDown(p->controller.key_down)) {
    p->position.y =
        Lerp(p->position.y, p->position.y + PAD_VELOCITY, GetFrameTime());
    TraceLog(LOG_DEBUG, "%f - %f\n", p->position.x, p->position.y);
  }

  if (IsKeyDown(p->controller.key_up)) {
    p->position.y =
        Lerp(p->position.y, p->position.y - PAD_VELOCITY, GetFrameTime());
    TraceLog(LOG_DEBUG, "%f - %f\n", p->position.x, p->position.y);
  }

  p->position.y = Clamp(p->position.y, 0, SCREEN_HEIGHT - PAD_HEIGHT);
}

void update_ball(Ball *ball) {
  float *posx = &ball->position.x;
  float *posy = &ball->position.y;

  *posx =
      Lerp(*posx, *posx + (ball->direction.x * BALL_VELOCITY), GetFrameTime());
  *posy =
      Lerp(*posy, *posy + (ball->direction.y * BALL_VELOCITY), GetFrameTime());

  if ((*posy + BALL_SIZE > SCREEN_HEIGHT) || (*posy < 0))
    ball->direction.y *= -1;
}

/* bool rec_contains(Rectangle *r1, Rectangle *r2) { */
/**/
/*   Vector2 tr, tl, br, bl; */
/**/
/*   tl = (Vector2){r2->x, r2->y}; */
/*   tr = (Vector2){r2->x + r2->width, r2->y}; */
/*   bl = (Vector2){r2->x, r2->y + r2->height}; */
/*   br = (Vector2){r2->x + r2->width, r2->y + r2->height}; */
/**/
/*   return CheckCollisionPointRec(tr, *r1) && CheckCollisionPointRec(tl, *r1)
 * && */
/*          CheckCollisionPointRec(br, *r1) && CheckCollisionPointRec(bl, *r1);
 */
/* } */

void update_game(Player *p1, Player *p2, Ball *ball) {
  update_player(p1);
  update_player(p2);
  update_ball(ball);

  int i;

  Rectangle p1_rect = {p1->position.x, p1->position.y, PAD_WIDTH, PAD_HEIGHT};
  Rectangle p2_rect = {p2->position.x, p2->position.y, PAD_WIDTH, PAD_HEIGHT};
  Rectangle ball_rect = {ball->position.x, ball->position.y, BALL_SIZE,
                         BALL_SIZE};

  Rectangle coll_rect = GetCollisionRec(p1_rect, ball_rect);
  /* if (rec_contains(&p1_rect, &coll_rect)) { */
  /*   TraceLog(LOG_INFO, "Paddle 1 containes the ball %d - %d",
   * ball->position.x, ball->position.y); */
  /*   ball->position.x += coll_rect.width + 3; */
  /* } */
  /**/
  /* if (rec_contains(&p2_rect, &coll_rect)) { */
  /*   TraceLog(LOG_INFO, "Paddle 2 containes the ball %d - %d",
   * ball->position.x, ball->position.y); */
  /*   ball->position.x -= coll_rect.width - 3; */
  /* } */

  if (CheckCollisionRecs(p1_rect, ball_rect)) {
    ball->position.x = p1_rect.x + p1_rect.width;
    ball->direction.x *= -1;
  }

  if (CheckCollisionRecs(p2_rect, ball_rect)) {
    ball->position.x = p2_rect.x - BALL_SIZE;
    ball->direction.x *= -1;
  }
}

void reset_game(Player *p1, Player *p2, Ball *ball) {
  p1->position = (Vector2){(float)PAD_OFFSET, INITIAL_PAD_Y};

  p2->position =
      (Vector2){SCREEN_WIDTH - PAD_WIDTH - 20, (float)SCREEN_HEIGHT / 2};

  int direction_x = GetRandomValue(0, 100) % 2 == 1 ? 1 : -1;
  int direction_y = GetRandomValue(0, 100) % 2 == 1 ? 1 : -1;
  TraceLog(LOG_DEBUG, "Direction x: %d Direction y: %d", direction_x,
           direction_y);
  ball->direction = (Vector2){direction_x, direction_y};
  ball->position = (Vector2){(float)SCREEN_WIDTH / 2 - (float)BALL_SIZE / 2,
                             (float)SCREEN_HEIGHT / 2 - (float)BALL_SIZE / 2};
}

int main(void) {
  SetTraceLogLevel(LOG_INFO);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(60);

  Texture2D texture = LoadTexture( ASSETS_PATH "/test.png");

  // Init game
  Player player1, player2;
  player1.texture = LoadTexture(ASSETS_PATH "/p1.png");
  player1.controller = (Controller){KEY_W, KEY_S};
  player2.texture = LoadTexture(ASSETS_PATH "/p2.png");
  player2.controller = (Controller){KEY_UP, KEY_DOWN};

  Ball ball;
  ball.texture = LoadTexture(ASSETS_PATH "/ball.png");

  Score score = {0, 0};

  reset_game(&player1, &player2, &ball);

  _Bool isGamePlaying;

  while (!WindowShouldClose()) {
    BeginDrawing();

    // Read input
    char *score_board = calloc(16, sizeof(char));

    sprintf(score_board, "%.2d - %.2d\n", score.left, score.right);
    const Vector2 text_size =
        MeasureTextEx(GetFontDefault(), score_board, 20, 1);
    DrawText(score_board, (float)SCREEN_WIDTH / 2 - text_size.x / 2,
             text_size.y / 2, 20, BLACK);

    update_game(&player1, &player2, &ball);

    // Calc score
    /* if (ball.position.x > player2.position.x + BALL_SIZE) { */
    if (ball.position.x > SCREEN_WIDTH) {
      score.left += 1;
      reset_game(&player1, &player2, &ball);
    /* } else if (ball.position.x < player1.position.x + PAD_WIDTH - BALL_SIZE) { */
    } else if (ball.position.x < 0) {
      score.right += 1;
      reset_game(&player1, &player2, &ball);
    }

    // Draw
    DrawTexture(player1.texture, player1.position.x, player1.position.y, WHITE);
    DrawTexture(player2.texture, player2.position.x, player2.position.y, WHITE);
    DrawTexture(ball.texture, ball.position.x, ball.position.y, WHITE);

    ClearBackground(RAYWHITE);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
