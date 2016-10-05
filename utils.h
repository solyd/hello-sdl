//
// Created by sol on 10/2/16.
//

#ifndef HELLO_WORLD_UTILS_H
#define HELLO_WORLD_UTILS_H


#include <memory>
#include <stdexcept>
#include <SDL_rect.h>


#define LOG_INFO std::cout << "[INFO] "
#define LOG_WARN std::cout << "[WARN] "

#define ThrowIfNull(x) \
do { \
  if ((x) == nullptr) { \
    throw std::logic_error("null assertion failed for: " #x); \
  } \
} while(false)


// TODO this doens't belong in the general "utils" file
// ----------------------------------------------------
#define SDLThrowIfNull(op) \
do { \
  if ((op) == NULL) { \
    throw std::logic_error(SDL_GetError()); \
  } \
} while (false)

#define SDLThrowOnErr(op) \
do { \
  if ((op) != 0) { \
    throw std::logic_error(SDL_GetError()); \
  } \
} while (false)
// ----------------------------------------------------

namespace utils {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&& ... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class NonCopyable {
protected:
		NonCopyable() = default;
		~NonCopyable() = default;

		NonCopyable(const NonCopyable& other) = delete;
		NonCopyable& operator=(const NonCopyable& other) = delete;
};

struct Color {
		const uint8_t r;
		const uint8_t g;
		const uint8_t b;
		const uint8_t alpha;

		Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t alpha = 0xFF);

		static Color BLACK;
		static Color WHITE;
};

#define EXPAND_COLOR(c) (c).r, (c).g, (c).b

struct Point2D {
		const int x;
		const int y;

		Point2D(int x, int y) : x(x), y(y) {}
};

#define EXPAND_POINT2D(p) (p).x, (p).y

struct Line {
		const Point2D point1;
		const Point2D point2;

		Line(const Point2D& point1, const Point2D point2);
};

struct Rectangle {
		const Point2D topLeft;
		const int width;
		const int height;

		Rectangle(const Point2D& topLeft, const int width, const int height);
};

struct Square : public Rectangle {
		Square(const Point2D& topLeft, const int sideLength);
};

}

namespace SDL {

SDL_Rect convert(const utils::Rectangle& rect);
SDL_Color convert(const utils::Color& color);

}

#endif //HELLO_WORLD_UTILS_H
