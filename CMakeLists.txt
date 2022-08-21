cmake_minimum_required(VERSION 3.5)

project(ytask LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ytask main.cpp)
target_link_libraries(ytask -pthread)
