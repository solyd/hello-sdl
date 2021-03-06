#include <SDL.h>

#include <string>
#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <memory>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <sstream>
#include <vector>
#include <random>

#include "utils.h"
#include "Window.h"
#include "Renderer.h"
#include "Texture.h"
#include "TtfFont.h"
#include "tetris/Grid.h"
#include "tetris/Shapes.h"

//Screen dimension constants
const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;
const char* ASSETS_DIR = "/home/solyd/.tmp/assets/";

using std::logic_error;
using std::unique_ptr;
using std::shared_ptr;

using namespace utils;

int main(int argc, char* args[]) {
	SDL_Init(SDL_INIT_VIDEO);
	auto sdlQuit = utils::make_guard([]() {SDL_Quit();});

	if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") != SDL_TRUE) {
		LOG_WARN << "Linear texture filtering is not enabled";
	}

	int imgFlags      = IMG_INIT_PNG;
	int imgInitResult = IMG_Init(imgFlags);
	if (imgInitResult == 0 || imgInitResult != imgFlags) {
		throw logic_error(IMG_GetError());
	}
	auto imgQuit = utils::make_guard([]() {IMG_Quit();});

	if (TTF_Init() == -1) {
		throw logic_error(TTF_GetError());
	}
	auto ttfQuit = utils::make_guard([]() {TTF_Quit();});

	try {
		SDL::Window               window("sdl test", SCREEN_WIDTH, SCREEN_HEIGHT);
		shared_ptr<SDL::Renderer> renderer(window.create_renderer());
		SDL::TtfFont              defaultFont("/home/solyd/.tmp/assets/default.ttf", 16);

		unique_ptr<SDL::Surface> dot = SDL::Surface::create_from_img("/home/solyd/.tmp/assets/dot.bmp");
		dot->set_color_key(Color::WHITE);
		unique_ptr<SDL::Texture> dotTexture = dot->create_texture(*renderer);

		while (true) {
			SDL_Event e;
			while (SDL_PollEvent(&e) != 0) {
				switch (e.type) {
					case SDL_QUIT:
						return 0;
				}
			}

			renderer->set_drawcolor(Color::WHITE);
			renderer->clear();
			renderer->render_copy(*dotTexture, { 0, 0 });

			renderer->present();
		}
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}


	return 0;
}

int tetris_main(int argc, char* args[]) {
	SDL_Init(SDL_INIT_VIDEO);
	auto sdlQuit = utils::make_guard([]() {SDL_Quit();});

	if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") != SDL_TRUE) {
		LOG_WARN << "Linear texture filtering is not enabled";
	}


	int imgFlags      = IMG_INIT_PNG;
	int imgInitResult = IMG_Init(imgFlags);
	if (imgInitResult == 0 || imgInitResult != imgFlags) {
		throw logic_error(IMG_GetError());
	}
	auto imgQuit = utils::make_guard([]() {IMG_Quit();});

	if (TTF_Init() == -1) {
		throw logic_error(TTF_GetError());
	}
	auto ttfQuit = utils::make_guard([]() {TTF_Quit();});

	try {
		SDL::Window               window("sdl test", SCREEN_WIDTH, SCREEN_HEIGHT);
		shared_ptr<SDL::Renderer> renderer(window.create_renderer());
		SDL::TtfFont              defaultFont("/home/solyd/.tmp/assets/default.ttf", 16);

		const Uint32 ticksPerFrame = 1000 / 60;

		// for FPS calculation
		uint32_t nframes         = 0;
		Uint32   lastFpsPollTime = 0;
		Uint32   sampleFpsDelta  = 100;

		std::unique_ptr<SDL::Texture> fpsTexture = nullptr;
		// --------------------------------

		Tetris::Grid                   grid(SCREEN_WIDTH, SCREEN_HEIGHT);
		std::unique_ptr<Tetris::Shape> shape(grid.spawn_shape());

		// don't allow too many presses per second, as it makes the kb seem too sensitive
//		Uint32 minMsBetweenKeyPresses = 120;
//		Uint32 lastKeyPressTime       = 0;

		Uint32 moveShapeEvery         = 500;
		Uint32 moveFallingShapeEvery  = 50;
		Uint32 lastShapeShiftDownTime = 0;
		Uint32 score                  = 0;

		std::unique_ptr<SDL::Texture> scoreTextTexture         =
																					defaultFont.render_solid("SCORE:", Color::BLACK)
																										 ->create_texture(*renderer);
		// The "SCORE:" will be to the right of the grid outline box
		int                           scoreTextTextureOffset_x = 30;
		int                           scoreTextTextureOffset_y = 10;

		Point2D scoreTextTexturePos(grid.outline().pos.x +
																grid.outline().width + scoreTextTextureOffset_x,
																grid.outline().pos.y + scoreTextTextureOffset_y);

		std::unique_ptr<SDL::Texture> scoreCountTexture       =
																					defaultFont.render_solid("0", Color::BLACK)
																										 ->create_texture(*renderer);
		// How much below the "SCORE:" text will be the integer score displayed
		int                           scoreCountTextureOffset = 20;

		Point2D scoreCountTexturePos(scoreTextTexturePos.x,
																 scoreTextTexturePos.y + scoreCountTextureOffset);

		while (true) {
			Uint32 frameStartTime = SDL_GetTicks();

			SDL_Event e;
			while (SDL_PollEvent(&e) != 0) {
				switch (e.type) {
					case SDL_KEYDOWN: {
						if (e.key.repeat == false) {
							switch (e.key.keysym.scancode) {
								case SDL_SCANCODE_DOWN:
									shape->rotate_clockwise();
									break;
								case SDL_SCANCODE_UP:
									shape->rotate_counterclockwise();
									break;
								case SDL_SCANCODE_RIGHT:
									shape->shift_right();
									break;
								case SDL_SCANCODE_LEFT:
									shape->shift_left();
									break;
								case SDL_SCANCODE_SPACE:
									shape->mark_falling();
									break;
							}
						}
					}
						break;

					case SDL_QUIT:
						return 0;
				}
			}

			if (( frameStartTime - lastShapeShiftDownTime >= moveShapeEvery ) ||
					( frameStartTime - lastShapeShiftDownTime >= moveFallingShapeEvery &&
						shape->is_falling())) {
				lastShapeShiftDownTime = frameStartTime;

				// Cannot move down, means we hit a dead cell or the bottom of the grid
				if (shape->shift_down() == false) {
					int rowsKilled = shape->suicide();
					if (rowsKilled > 0) {
						score += ( 1 << rowsKilled );
					}

					// TODO generalize somehow to convert int to texture/string
					std::stringstream scoreText;
					scoreText << score;
					scoreCountTexture =
									defaultFont.render_solid(scoreText.str(), Color::BLACK)
														 ->create_texture(*renderer);

					shape = grid.spawn_shape();
				}
			}

			renderer->set_drawcolor(Color::WHITE);
			renderer->clear();

			renderer->set_drawcolor(Color::BLACK);
			grid.render(*renderer);
			shape->render(*renderer);


			renderer->render_copy(*scoreTextTexture, scoreTextTexturePos);
			renderer->render_copy(*scoreCountTexture, scoreCountTexturePos);

			// -----------------------------------------
			Uint32 frameEndTime = SDL_GetTicks();

			uint32_t timeSinceFpsPoll = frameEndTime - lastFpsPollTime;
			if (timeSinceFpsPoll >= sampleFpsDelta) {
				float fps = (( nframes * 1.f ) / ( timeSinceFpsPoll / 1000.f ));

				std::stringstream fpsText;
				fpsText.precision(3);
				fpsText << fps << " FPS";

				fpsTexture = defaultFont
								.render_solid(fpsText.str(), Color::BLACK)
								->create_texture(*renderer);

				lastFpsPollTime = frameEndTime;
				nframes         = 0;
			}

			if (fpsTexture) {
				renderer->render_copy(*fpsTexture, { 0, 0 });
			}

			// ----------------------------------------------

			int32_t sleepDelta = ticksPerFrame - ( frameEndTime - frameStartTime );
			if (sleepDelta > 0) {
				SDL_Delay(sleepDelta);
			}

			renderer->present();
			++nframes;
		}
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}


	return 0;
}