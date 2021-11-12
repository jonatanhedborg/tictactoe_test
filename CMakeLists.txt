cmake_minimum_required(VERSION 3.17)
project(tictactoe)

option(BUILD_SHARED_LIBS "Build shared libs" OFF) #optional
set(XXHASH_BUILD_ENABLE_INLINE_API OFF) #optional
set(XXHASH_BUILD_XXHSUM OFF) #optional
add_subdirectory(external/xxHash/cmake_unofficial/ external/xxHash/build/ EXCLUDE_FROM_ALL)

set(CMAKE_CXX_STANDARD 17)

add_executable(tictactoe main.cpp)
target_link_libraries(tictactoe PRIVATE xxHash::xxhash)



