cmake_minimum_required(VERSION 2.8)

SET( PLUGIN_NAME Volume_Import )

SET( PLUGIN_ROOT_DIR ${SCHNApps_ROOT_DIR}/Plugins/volume_import )

INCLUDE_DIRECTORIES(
	${PLUGIN_ROOT_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}
)

file(
	GLOB_RECURSE
	PLUGIN_FILES
	${PLUGIN_ROOT_DIR}/src/*.cpp
	${PLUGIN_ROOT_DIR}/include/*.h
	${PLUGIN_ROOT_DIR}/include/*.hpp
)

file(
	GLOB_RECURSE
	PLUGIN_UI_FILES
	${PLUGIN_ROOT_DIR}/forms/*.ui
)

SET(
	PLUGIN_QOBJECT_FILES
	${PLUGIN_ROOT_DIR}/include/volume_import.h
)

include( ${SCHNApps_ROOT_DIR}/Plugins/plugins_cmake.txt )
