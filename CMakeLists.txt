# CMake entry point
cmake_minimum_required (VERSION 2.6)
project (OpenGL-BezierCurves)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenGL REQUIRED)

# Compile external dependencies
add_subdirectory (external)

include_directories(
	external/AntTweakBar-1.16/include/
	external/glfw-3.1.2/include/GLFW/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
	.
)

set(
	ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

add_executable(
	BezierCurves
	BezierCurves.cpp
	common/shader.hpp
	common/shader.cpp
	common/tessshader.hpp
	common/tessshader.cpp
)

target_link_libraries(
	BezierCurves
	${ALL_LIBS}
	ANTTWEAKBAR_116_OGLCORE_GLFW
)

add_custom_command(
	TARGET BezierCurves POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/BezierCurves${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}"
)
