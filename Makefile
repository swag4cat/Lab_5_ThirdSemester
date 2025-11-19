# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Targets
TARGET = cave_generator
SRC = main.cpp

# Default target - build the program
all: $(TARGET)

# Create executable
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(SFML_FLAGS)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Generate documentation
doc:
	doxygen Doxyfile

# Clean build files
clean:
	rm -f $(TARGET)

# Clean documentation
clean-doc:
	rm -rf docs

# Clean everything
distclean: clean clean-doc

.PHONY: all run doc clean clean-doc distclean
