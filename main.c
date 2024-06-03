#define _CRT_SECURE_NO_WARNINGS

#ifndef _DEBUG

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#endif

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define CELL_SIZE 32
#define CELL_COUNT 25

#define MAX_LENGTH CELL_COUNT*CELL_COUNT


typedef struct {
	Vector2 buffer[32 * 32];
	int front;
	int back;
}cqueue;

static inline Vector2 front(cqueue* snek) {
	return snek->buffer[snek->front];
}

static inline void insert_f(cqueue *const   dq, Vector2 pos) {
	dq->front++;
	if (dq->front == MAX_LENGTH) {
		dq->front = 0;
	}

	dq->buffer[dq->front] = pos;
}

static inline void insert_b(cqueue* const   snek, Vector2 pos) {
	snek->back--;

	if (snek->back < 0) {
		snek->back = MAX_LENGTH - 1;
	}

	snek->buffer[snek->back] = pos;

}

static inline Vector2 pop_b(cqueue *const   dq) {
	Vector2 temp = dq->buffer[dq->back];
	dq->back++;

	if (dq->back == MAX_LENGTH) {
		dq->back = 0;
	}

	return temp;
}

// Snake

typedef enum{
	SNEK_UP,
	SNEK_DOWN,
	SNEK_LEFT,
	SNEK_RIGHT
}Direction;

static inline void init_snek(cqueue* const   snek, Vector2 pos) {
	snek->buffer[0] = pos;
}

static inline bool check_collision(cqueue* const snek, const Vector2 pos) {

	if (snek->buffer[snek->front].x == pos.x && snek->buffer[snek->front].y == pos.y) {
		return true;
	}
	return false;
}

static bool check_bounds_collision(cqueue *const   snek) {
	if (snek->buffer[snek->front].x > CELL_COUNT - 1 || snek->buffer[snek->front].x < 0) {
		return true;
	}
	else if (snek->buffer[snek->front].y > CELL_COUNT - 1 || snek->buffer[snek->front].y < 0) {
		return true;
	}
	return false;
}

static bool check_segment_collision(cqueue *const snek) {
	if (snek->front >= snek->back) {
		for (int i = snek->back; i < snek->front; i++) {
			if (check_collision(snek, snek->buffer[i]) == true) {
				
				return true;
			}
		}
	}
	else {
		for (int i = 0; i < snek->front; i++) {
			if (check_collision(snek, snek->buffer[i]) == true) {
				
				return true;
			}
		}
		for (int i = snek->back; i < MAX_LENGTH; i++) {
			if (check_collision(snek, snek->buffer[i]) == true) {
				return true;
			}
		}
	}
	return false;
}

static Vector2 move_snek(cqueue* const snek, Direction dir, Vector2 fruit_location) {

	Vector2 segment = front(snek);

	switch (dir) {
	case SNEK_UP:
		segment.y -= 1;
		break;
	case SNEK_DOWN:
		segment.y += 1;
		break;
	case SNEK_LEFT:
		segment.x -= 1;
		break;
	case SNEK_RIGHT:
		segment.x += 1;
		break;
	}

	insert_f(snek, segment);
	return pop_b(snek);
}

static inline void draw_segment(Vector2 pos) {
	DrawRectangle(pos.x * CELL_SIZE, pos.y * CELL_SIZE, CELL_SIZE, CELL_SIZE, WHITE);
}

void draw_snake(cqueue *const   snek) {
	
	if (snek->front >= snek->back) {
		for (int i = snek->back; i <= snek->front; i++) {
			draw_segment(snek->buffer[i]);
		}
	}
	else {
		for (int i = 0; i <= snek->front; i++) {
			draw_segment(snek->buffer[i]);
		}
		for (int i = snek->back; i < MAX_LENGTH; i++) {
			draw_segment(snek->buffer[i]);
		}
	}

}

// Fruit

void draw_fruit(Texture2D* fruit, Vector2 grid_pos) {
	DrawTexture(*fruit, grid_pos.x * CELL_SIZE, grid_pos.y * CELL_SIZE, WHITE);
}


Vector2 get_random_pos() {
	Vector2 pos = { 0 };
	pos.x = GetRandomValue(0, CELL_COUNT - 1);
	pos.y = GetRandomValue(0, CELL_COUNT - 1);
	return pos;
}

// Game Context
typedef struct {

	cqueue snek;
	Vector2 prev;

	Direction current_dir;
	Vector2 current_food_loc;

	int score;

	bool game_over;
	bool pause_game;
	bool is_food_on_screen;


	Texture2D fruit;

	Color bg;

}game_ctx;

void init_ctx(game_ctx* const ctx) {
	ctx->snek = (cqueue){ 0 };
	init_snek(&ctx->snek, (Vector2) {12, 12});
	ctx->prev = (Vector2){ 0 };

	ctx->current_dir = SNEK_DOWN;
	ctx->current_food_loc = (Vector2){ 0 };
	ctx->is_food_on_screen = false;

	ctx->score = 0;
	ctx->game_over = false;
	ctx->pause_game = false;

	ctx->fruit = LoadTexture("fruit.png");

	ctx->bg = (Color){ 103, 114, 169, 255 };

}

int main() {

	InitWindow(CELL_SIZE * CELL_COUNT, CELL_SIZE * CELL_COUNT, "SNEK");
	SetTargetFPS(20);

	game_ctx ctx = { 0 };
	init_ctx(&ctx);

	while (!WindowShouldClose()) {
		
		// Input
		if (!ctx.pause_game) {
			if (IsKeyPressed(KEY_W) && ctx.current_dir != SNEK_DOWN) {
				ctx.current_dir = SNEK_UP;
			}
			if (IsKeyPressed(KEY_S) && ctx.current_dir != SNEK_UP) {
				ctx.current_dir = SNEK_DOWN;
			}
			if (IsKeyPressed(KEY_A) && ctx.current_dir != SNEK_RIGHT) {
				ctx.current_dir = SNEK_LEFT;
			}
			if (IsKeyPressed(KEY_D) && ctx.current_dir != SNEK_LEFT) {
				ctx.current_dir = SNEK_RIGHT;
			}

			// Movement & Collision
			ctx.prev = move_snek(&ctx.snek, ctx.current_dir, ctx.current_food_loc);
			if (check_collision(&ctx.snek, ctx.current_food_loc)) {
				ctx.score++;
				insert_b(&ctx.snek, ctx.prev);
				ctx.is_food_on_screen = false;
			}

			if (check_segment_collision(&ctx.snek) || check_bounds_collision(&ctx.snek)) {
				ctx.game_over = true;
			}
		}
		
		
		// Rendering
		BeginDrawing();
		ClearBackground(ctx.bg);

		if (ctx.game_over == true) {
			ctx.pause_game = true;

			DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, 0.7f));
			DrawRectangle(0, GetScreenHeight()/2 - 80, GetScreenWidth(), 80, BLACK);
			DrawText(TextFormat("SCORE: %d", ctx.score), GetScreenWidth()/2 - 50, GetScreenHeight()/2 - 40, 20, WHITE);
			bool again = GuiButton((Rectangle) { GetScreenWidth() / 2 - 53, GetScreenHeight() / 2 + 20, 100, 40 }, "Again");

			if (again) {
				init_ctx(&ctx);
				ctx.game_over = false;
				ctx.pause_game = false;
			}

		}
		else {
			// Spawn new fruit
			if (!ctx.is_food_on_screen) {
				ctx.is_food_on_screen = true;
				ctx.current_food_loc = get_random_pos();
			}

			DrawText(TextFormat("SCORE:%d", ctx.score), 0, 0, 20, BLACK);
			DrawFPS(100, 0);

			draw_snake(&ctx.snek);
			draw_fruit(&ctx.fruit, ctx.current_food_loc);
		}

		EndDrawing();
	}

	UnloadTexture(ctx.fruit);
	CloseWindow();

	return 0;
}