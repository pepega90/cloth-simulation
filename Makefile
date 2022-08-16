all: compile run
compile:
	g++ main.cpp -o main -I ./deps/include/ -L ./deps/lib/ -lraylib -lGL -lm -lpthread -ldl -lrt
run:
	./main
web:
	emcc -std=c++17 -o index.html main.cpp -Os -Wall -I ./include/ -L ./lib/ -lraylib -s USE_GLFW=3 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH -DPLATFORM_WEB
