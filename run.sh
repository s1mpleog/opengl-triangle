#!/bin/bash
set -e # Exit on error

clang -o main main.c -lm -lglfw -lGLEW -lGL
./main
