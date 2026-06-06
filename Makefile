# Build: simplesmente rode  ->  mingw32-make
# Requer MSYS2 MinGW64 com freeglut instalado:
#   pacman -S mingw-w64-x86_64-freeglut

CXX      = g++
CXXFLAGS = -O2 -Wall
LDLIBS   = -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32

iso-tilemap.exe: src/main.cpp
	$(CXX) $(CXXFLAGS) src/main.cpp -o iso-tilemap.exe $(LDLIBS)

clean:
	del /Q iso-tilemap.exe
