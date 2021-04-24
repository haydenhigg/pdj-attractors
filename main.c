#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct {
	double a, b, c, d;
} AttractorParams;

typedef struct {
   double x, y;
} Point;

typedef struct {
	uint8_t r, g, b;
} Color;

AttractorParams newParams(double a, double b, double c, double d) {
	AttractorParams ret;

	ret.a = a;
	ret.b = b;
	ret.c = c;
	ret.d = d;

	return ret;
}

void generate(int iters, Point attractor[iters], AttractorParams params) {
	attractor[0].x = 0;
	attractor[0].y = 0;

	Point p = attractor[0];
	
	for (int n = 1; n < iters; n++) {
		attractor[n].x = sin(params.a * p.y) - cos(params.b * p.x); // x_n+1 = sin(a * y_n) - cos(b * x_n)
		attractor[n].y = sin(params.c * p.x) - cos(params.d * p.y); // y_n+1 = sin(c * x_n) - cos(d * y_n)

		p = attractor[n];
	}
}

void makeHistogram(int n, Point attractor[n], int w, int h, long histogram[h][w]) {
	//-- find top left and bottom right --//
	Point min = {.x = INT32_MAX, .y = INT32_MAX};
	Point max = {.x = INT32_MIN, .y = INT32_MIN};

	for (int i = 0; i < n; i++) {
		if (attractor[i].x < min.x)
			min.x = attractor[i].x;
		else if (attractor[i].x > max.x)
			max.x = attractor[i].x;

		if (attractor[i].y < min.y)
			min.y = attractor[i].y;
		else if (attractor[i].y > max.y)
			max.y = attractor[i].y;
	}

	//-- initialize all histogram counts to 0 --//
	for (int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
			histogram[i][j] = 0;

	//-- so that the math is easier when scaling --//
	double widthScaling = w / (max.x - min.x);
	double heightScaling = h / (max.y - min.y);

	double offsetX = min.x * widthScaling;
	double offsetY = min.y * heightScaling;

	//-- re-adjusts all points in `attractor` to  --//
	//-- fill a box with width `w` and height `h` --//
	//-- and writes the values in that box to the --//
	//-- matrix `histogram`						  --//
	int x, y;

	for (int i = 0; i < n; i++) {
		x = attractor[i].x * widthScaling - offsetX;
		y = attractor[i].y * heightScaling - offsetY;

		histogram[y][x]++;
	}
}

Color getHue(double density) {
	Color ret;

	ret.r = fmax(255 - density * 2, 0);
	ret.g = fmax(255 - density * 8, 0);
	ret.b = fmax(255 - density * 5, 0);

	return ret;
}

void makeColorMap(int w, int h, long histogram[h][w], uint8_t colorMap[h][w][3]) {
	long min = __LONG_MAX__;
	long max = -__LONG_MAX__ - 1;

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (histogram[i][j] < min && histogram[i][j] > 0)
				min = histogram[i][j];
			else if (histogram[i][j] > max)
				max = histogram[i][j];
		}
	}

	double scaling = 255. / (max - min);
	double offset = min * scaling;

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (histogram[i][j] == 0) { // blank
				colorMap[i][j][0] = 255;
				colorMap[i][j][1] = 255;
				colorMap[i][j][2] = 255;
			} else {					// passed through at least once
				Color hue = getHue(histogram[i][j] * scaling - offset);

				colorMap[i][j][0] = hue.r;
				colorMap[i][j][1] = hue.g;
				colorMap[i][j][2] = hue.b;
			}
		}
	}
}

int writeToFile(int w, int h, uint8_t colorMap[h][w][3], const char *fileName) {
	FILE *file = fopen(fileName, "wb");
	if (!file) return 1;

	fprintf(file, "P6 %d %d %d\n", w, h, 255);

	for (int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
			fwrite(colorMap[i][j], 1, 3, file);

	fclose(file);

	return 0;
}

const int ITERS = 120000000;
const int WIDTH = 1400;
const int HEIGHT = 1400;

static Point attractor[ITERS];
static long histogram[HEIGHT][WIDTH];
static uint8_t colorMap[HEIGHT][WIDTH][3];

int main(void) {
	// 2.01, 2.53, 1.61, -0.33
	// -2, -2, -1.2, 2
	
	generate(ITERS, attractor, newParams(-2, -2, -1.2, 2));
	makeHistogram(ITERS, attractor, WIDTH, HEIGHT, histogram);
	makeColorMap(WIDTH, HEIGHT, histogram, colorMap);

	return writeToFile(WIDTH, HEIGHT, colorMap, "test.ppm");
}
