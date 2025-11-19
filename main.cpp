/**
 * @file main.cpp
 * @brief Cave Generator using Cellular Automata
 * @details Implementation of cave generation algorithm for Lab 5
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <string>

/**
 * @class CaveGenerator
 * @brief Cellular automata for cave generation
 *
 * Implements the cave generation algorithm using cellular automata rules.
 * Cells can be alive (true) or dead (false) based on neighbor counts.
 */

class CaveGenerator {
private:
    int width, height;
    std::vector<std::vector<bool>> cave;
    double birthChance;
    int birthLimit;
    int deathLimit;

    /**
     * @brief Count alive neighbors around a cell
     * @param x X coordinate of the cell
     * @param y Y coordinate of the cell
     * @return Number of alive neighbors (0-8)
     */

    int countAliveNeighbors(int x, int y) {
        int count = 0;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) continue;

                int neighborX = x + i;
                int neighborY = y + j;

                if (neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height) {
                    if (cave[neighborX][neighborY]) {
                        count++;
                    }
                }
            }
        }
        return count;
    }

public:

    /**
     * @brief Constructor for CaveGenerator
     * @param w Width of the cave
     * @param h Height of the cave
     * @param chance Birth chance probability (0.0-1.0)
     * @param birth Birth limit for new cells
     * @param death Death limit for existing cells
     */

    CaveGenerator(int w, int h, double chance, int birth, int death)
    : width(w), height(h), birthChance(chance), birthLimit(birth), deathLimit(death) {
        cave.resize(width, std::vector<bool>(height, false));
        initializeCave();
    }

    /**
     * @brief Initialize cave with random cells based on birth chance
     */

    void initializeCave() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                cave[x][y] = (dis(gen) < birthChance);
            }
        }
    }

    /**
     * @brief Perform one iteration of cellular automata
     *
     * Applies the rules:
     * - Alive cells die if neighbors < deathLimit
     * - Dead cells become alive if neighbors > birthLimit
     */

    void simulateStep() {
        std::vector<std::vector<bool>> newCave = cave;

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                int aliveNeighbors = countAliveNeighbors(x, y);

                if (cave[x][y]) {
                    if (aliveNeighbors < deathLimit) {
                        newCave[x][y] = false;
                    }
                } else {
                    if (aliveNeighbors > birthLimit) {
                        newCave[x][y] = true;
                    }
                }
            }
        }

        cave = newCave;
    }

    const std::vector<std::vector<bool>>& getCave() const {
        return cave;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    double getBirthChance() const { return birthChance; }
    int getBirthLimit() const { return birthLimit; }
    int getDeathLimit() const { return deathLimit; }

    int getAliveCount() const {
        int count = 0;
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                if (cave[x][y]) count++;
            }
        }
        return count;
    }
};

/**
 * @class GraphicsManager
 * @brief Handles SFML graphics and user interface
 *
 * Manages the rendering window, user input, and visualization
 * of the cave generation process.
 */

class GraphicsManager {
private:
    sf::RenderWindow window;
    sf::Font font;
    bool fontLoaded;
    sf::Text infoText;
    int iteration;
    int cellSize;
    CaveGenerator& caveGen;

public:

    /**
     * @brief Constructor for GraphicsManager
     * @param generator Reference to the CaveGenerator instance
     */

    GraphicsManager(CaveGenerator& generator)
    : window(),
    font(),
    fontLoaded(false),
    infoText(),
    iteration(0),
    cellSize(0),
    caveGen(generator) {

        window.create(sf::VideoMode(1000, 700), "Cave Generator");

        // Try to load font
        const char* fontPaths[] = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
            "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
            nullptr
        };

        for (int i = 0; fontPaths[i] != nullptr; i++) {
            if (font.loadFromFile(fontPaths[i])) {
                fontLoaded = true;
                std::cout << "Font loaded: " << fontPaths[i] << std::endl;
                break;
            }
        }

        if (!fontLoaded) {
            std::cout << "Font not loaded, using simple graphics" << std::endl;
        }

        // Setup info text
        if (fontLoaded) {
            infoText.setFont(font);
            infoText.setCharacterSize(14);
            infoText.setFillColor(sf::Color::White);
            infoText.setPosition(650, 20);
        }

        // Calculate cell size
        int caveWidth = caveGen.getWidth();
        int caveHeight = caveGen.getHeight();
        cellSize = std::min(600 / caveWidth, 500 / caveHeight);
        if (cellSize < 3) cellSize = 3;
    }

    /**
     * @brief Main graphics loop
     */

    void run() {
        while (window.isOpen()) {
            handleEvents();
            render();
        }
    }

private:
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    caveGen.simulateStep();
                    iteration++;
                } else if (event.key.code == sf::Keyboard::R) {
                    // Restart with new cave
                    caveGen.initializeCave();
                    iteration = 0;
                } else if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
        }
    }

    void render() {
        window.clear(sf::Color(20, 20, 20));

        // Draw cave (left side)
        drawCave();

        // Draw info panel (right side)
        drawInfoPanel();

        window.display();
    }

    void drawCave() {
        const auto& cave = caveGen.getCave();
        int caveWidth = caveGen.getWidth();
        int caveHeight = caveGen.getHeight();

        // Cave on the left
        int startX = 20;
        int startY = 20;

        // Border around cave
        sf::RectangleShape border(sf::Vector2f(caveWidth * cellSize + 4, caveHeight * cellSize + 4));
        border.setPosition(startX - 2, startY - 2);
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineColor(sf::Color::White);
        border.setOutlineThickness(2);
        window.draw(border);

        // Cave title
        if (fontLoaded) {
            sf::Text caveTitle;
            caveTitle.setFont(font);
            caveTitle.setString("Cave Map");
            caveTitle.setCharacterSize(18);
            caveTitle.setFillColor(sf::Color::White);
            caveTitle.setPosition(startX, startY - 25);
            window.draw(caveTitle);
        }

        // Cave cells
        for (int x = 0; x < caveWidth; x++) {
            for (int y = 0; y < caveHeight; y++) {
                sf::RectangleShape cell(sf::Vector2f(cellSize - 1, cellSize - 1));
                cell.setPosition(startX + x * cellSize, startY + y * cellSize);

                if (cave[x][y]) {
                    cell.setFillColor(sf::Color::White);
                } else {
                    cell.setFillColor(sf::Color::Black);
                }

                window.draw(cell);
            }
        }
    }

    void drawInfoPanel() {
        int panelX = 650;
        int panelY = 20;

        // Info panel background
        sf::RectangleShape panel(sf::Vector2f(320, 450));
        panel.setPosition(panelX - 10, panelY - 10);
        panel.setFillColor(sf::Color(40, 40, 40));
        panel.setOutlineColor(sf::Color::White);
        panel.setOutlineThickness(1);
        window.draw(panel);

        if (fontLoaded) {
            // Title
            sf::Text title;
            title.setFont(font);
            title.setString("CAVE INFORMATION");
            title.setCharacterSize(18);
            title.setFillColor(sf::Color::Yellow);
            title.setPosition(panelX, panelY);
            window.draw(title);

            // Information
            std::string info =
            "Iteration: " + std::to_string(iteration) + "\n\n" +
            "Size: " + std::to_string(caveGen.getWidth()) + " x " +
            std::to_string(caveGen.getHeight()) + "\n" +
            "Alive cells: " + std::to_string(caveGen.getAliveCount()) + "\n" +
            "Birth chance: " + std::to_string(static_cast<int>(caveGen.getBirthChance() * 100)) + "%\n" +
            "Birth limit: " + std::to_string(caveGen.getBirthLimit()) + "\n" +
            "Death limit: " + std::to_string(caveGen.getDeathLimit()) + "\n\n" +
            "CONTROLS:\n" +
            "SPACE - Next iteration\n" +
            "R - New random cave\n" +
            "ESC - Exit";

            infoText.setString(info);
            infoText.setPosition(panelX, panelY + 40);
            window.draw(infoText);
        } else {
            drawInfoWithoutText(panelX, panelY);
        }
    }

    void drawInfoWithoutText(int x, int y) {
        sf::RectangleShape iterationBox(sf::Vector2f(200, 20));
        iterationBox.setPosition(x, y + 40);
        iterationBox.setFillColor(sf::Color::Blue);
        window.draw(iterationBox);
    }
};

/**
 * @brief Main function
 * @return Exit status
 */

int main() {
    int width, height;
    double birthChance;
    int birthLimit, deathLimit;

    std::cout << "=== CAVE GENERATOR ===" << std::endl;
    std::cout << "Enter cave width: ";
    std::cin >> width;
    std::cout << "Enter cave height: ";
    std::cin >> height;
    std::cout << "Enter birth chance (0.0-1.0): ";
    std::cin >> birthChance;
    std::cout << "Enter birth limit: ";
    std::cin >> birthLimit;
    std::cout << "Enter death limit: ";
    std::cin >> deathLimit;

    CaveGenerator caveGen(width, height, birthChance, birthLimit, deathLimit);

    std::cout << "Starting graphics interface..." << std::endl;
    std::cout << "Controls: SPACE - next iteration, R - new cave, ESC - exit" << std::endl;

    GraphicsManager graphics(caveGen);
    graphics.run();

    return 0;
}
