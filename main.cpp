// Project Identifier: A8A3A33EF075ACEF9B08F5B9845569ECCB423725
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <iostream>
#include <limits>
#include <set>
#include <stack>
#include <tuple>
#include <vector>

#include <getopt.h>

using namespace std;

class Solver {
private:
    struct location {
        uint32_t row = 0;
        uint32_t col = 0;
        char color = '\0';

        bool operator==(const location& other) const {
            return color == other.color && row == other.row && col == other.col;
        }

        bool operator<(const location& other) const {
            return tie(color, row, col) < tie(other.color, other.row, other.col);
        }
    };

    vector<vector<vector<char>>> backtrace;   // 3D vector for backtrace
    vector<vector<char>> map;                 // 2D vector for map
    deque<location> sc;

    uint32_t start_row;
    uint32_t start_col;
    uint32_t num_colors;
    uint32_t target_row;
    uint32_t target_col;
    char target_color = '\0';

    char container = '\0';     // to store either stack or queue (s or q)
    char output_type = '\0';   // to store either map or list (m or l)

public:
    // Read and process command line arguments.
    void get_options(int argc, char** argv);

    // Read in the CSV music file through stdin.
    void read_data();

    bool is_valid_character(char& ch, uint32_t& num_colors);

    void puzzle_solver();

    bool is_button(uint32_t& num_colors, char& current_loc, char& current_state);

    bool is_valid_location(location& loc, char& current_color);

    int color_to_room(char color);

    void output();

    void color_map(char colorChar);
};


int main(int argc, char** argv) {
    ios_base::sync_with_stdio(false);

    Solver puzzle;
    puzzle.get_options(argc, argv);
    puzzle.read_data();
    puzzle.puzzle_solver();
    puzzle.output();

    exit(0);
}


void Solver::get_options(int argc, char** argv) {
    int option_index = 0, option = 0;

    // Don't display getopt error messages about options
    opterr = false;

    struct option longOpts[] = {
        {"output", required_argument, nullptr,  'o'},
        {  "help",       no_argument, nullptr,  'h'},
        { "stack",       no_argument, nullptr,  's'},
        { "queue",       no_argument, nullptr,  'q'},
        { nullptr,                 0, nullptr, '\0'}
    };

    while ((option = getopt_long(argc, argv, "o:hsq", longOpts, &option_index)) != -1) {
        switch (option) {
        case 'o':
            if (strcmp(optarg, "list") == 0) {
                output_type = 'l';
            } else if (strcmp(optarg, "map") == 0) {
                output_type = 'm';
            } else {
                cerr << "Invalid output type provided!\n";
                exit(1);
            }
            break;
        case 'h':
            cout << "Helpful message..\n";
            exit(0);
        case 's':
            if (container == 's' || container == 'q') {
                cerr << "Multiple container modes specified!\n";
                exit(1);
            }
            container = 's';
            break;
        case 'q':
            if (container == 's' || container == 'q') {
                cerr << "Multiple container modes specified!\n";
                exit(1);
            }
            container = 'q';
            break;
        default:
            cerr << "Invalid option!\n";
            exit(1);
        }
    }
    if (output_type == '\0') {
        output_type = 'm';   // if output flag is not provided
    }
    if (container != 'q' && container != 's') {
        cerr << "No container mode specified!\n";
        exit(1);
    }
}

// Read data into the program through stdin.
void Solver::read_data() {
    uint32_t height;
    uint32_t width;
    uint32_t current_row = 0;
    string line;
    char ch;
    bool target = false;
    bool start = false;

    cin >> num_colors >> height >> width;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (num_colors > 26 || width < 1 || height < 1 || (width == 1 && height == 1)) {
        cerr << "Invalid map parameters!\n";
        exit(1);
    }

    backtrace.resize(num_colors + 1);
    for (uint32_t level = 0; level < num_colors + 1; ++level) {
        backtrace[level].resize(height);
        for (uint32_t row = 0; row < height; ++row) {
            backtrace[level][row].resize(width, '\0');
        }
    }

    map.resize(height, vector<char>(width, '\0'));

    while (getline(cin, line) && current_row < height) {
        if (line.size() >= 2 && line[0] == '/' && line[1] == '/') {
            continue;
        }

        for (uint32_t col = 0; col < width && col < line.size(); ++col) {
            ch = line[col];

            if (!is_valid_character(ch, num_colors)) {
                cerr << "Invalid character in map: " << ch << "\n";
                exit(1);
            }

            if (ch == '@') {
                if (!start) {
                    start_col = col;
                    start_row = current_row;
                    start = true;
                } else {
                    cerr << "Multiple '@' characters\n";
                    exit(1);
                }
            }

            if (ch == '?') {
                if (!target) {
                    target = true;
                } else {
                    cerr << "Multiple '?' characters\n";
                    exit(1);
                }
            }

            map[current_row][col] = ch;
        }
        current_row++;
    }
}

void Solver::puzzle_solver() {
    location temp = { start_row, start_col, '^' };
    location current_state;
    uint32_t room;
    sc.push_back(temp);
    backtrace[0][start_row][start_col] = '@';

    while (!sc.empty()) {
        if (container == 's') {
            current_state = sc.back();
            sc.pop_back();
        }

        // if using queue (BFS)
        else if (container == 'q') {
            current_state = sc.front();
            sc.pop_front();
        }
        char current_loc = map[current_state.row][current_state.col];   // location on map

        if (is_button(num_colors, current_loc, current_state.color)) {   // button stuff

            room = color_to_room(current_loc);
            if (backtrace[room][current_state.row][current_state.col] == '\0') {
                backtrace[room][current_state.row][current_state.col] = current_state.color;
                temp = { current_state.row, current_state.col, current_loc };
                sc.push_back(temp);
            }
            continue;
        }

        // NESW stuff
        temp = {
            current_state.row - 1,
            current_state.col,
            current_state.color,
        };
        room = color_to_room(current_state.color);
        if (is_valid_location(temp, current_state.color)) {
            sc.push_back(temp);
            backtrace[room][current_state.row - 1][current_state.col] = 'S';
            if (map[current_state.row - 1][current_state.col] == '?') {   // found target
                target_row = current_state.row - 1;
                target_col = current_state.col;
                target_color = current_state.color;
                break;
            }
        }
        temp = { current_state.row, current_state.col + 1, current_state.color };   // east
        if (is_valid_location(temp, current_state.color)) {
            sc.push_back(temp);
            backtrace[room][current_state.row][current_state.col + 1] = 'W';
            if (map[current_state.row][current_state.col + 1] == '?') {   // found target
                target_row = current_state.row;
                target_col = current_state.col + 1;
                target_color = current_state.color;
                break;
            }
        }
        temp = { current_state.row + 1, current_state.col, current_state.color };   // south
        if (is_valid_location(temp, current_state.color)) {
            sc.push_back(temp);
            backtrace[room][current_state.row + 1][current_state.col] = 'N';
            if (map[current_state.row + 1][current_state.col] == '?') {   // found target
                target_row = current_state.row + 1;
                target_col = current_state.col;
                target_color = current_state.color;
                break;
            }
        }
        temp = { current_state.row, current_state.col - 1, current_state.color };   // west

        if (is_valid_location(temp, current_state.color)) {
            sc.push_back(temp);
            backtrace[room][current_state.row][current_state.col - 1] = 'E';
            if (map[current_state.row][current_state.col - 1] == '?') {   // found target
                target_row = current_state.row;
                target_col = current_state.col - 1;
                target_color = current_state.color;
                break;
            }
        }
    }
    if (target_color == '\0') {
        cout << "No solution.\n";
        cout << "Discovered:\n";

        for (uint32_t row = 0; row < map.size(); ++row) {
            for (uint32_t col = 0; col < map[row].size(); ++col) {
                // Check if location was discovered in any color
                bool discovered = false;
                for (uint32_t color = 0; color < num_colors + 1; ++color) {
                    if (backtrace[color][row][col] != '\0') {
                        discovered = true;
                        break;
                    }
                }

                if (discovered) {
                    cout << map[row][col];
                } else {
                    cout << '#';
                }
            }
            cout << '\n';
        }
        exit(0);
    }
}

void Solver::output() {
    sc.clear();
    location current_location = { target_row, target_col, target_color };
    uint32_t current_color = color_to_room(current_location.color);
    char dir = backtrace[current_color][current_location.row][current_location.col];

    while (dir != '@') {
        sc.push_back(current_location);
        dir = backtrace[current_color][current_location.row][current_location.col];
        switch (dir) {
        case 'N':
            current_location.row--;
            break;
        case 'E':
            current_location.col++;
            break;
        case 'S':
            current_location.row++;
            break;
        case 'W':
            current_location.col--;
            break;
        case '@':
            break;
        default:   // it's a button or the start
            current_location.color = dir;
            current_color = color_to_room(dir);
            break;
        }
    }
    if (output_type == 'l') {   // list mode
        for (auto it = sc.rbegin(); it != sc.rend(); ++it) {
            location loc = *it;
            cout << "(" << loc.color << ", (" << loc.row << ", " << loc.col << "))\n";
        }
    } else if (output_type == 'm') {   // map mode
        location currentLocation = sc.front();
        set<char> printedColors;   // To ensure we don't print a color map more than once

        backtrace[color_to_room(currentLocation.color)][currentLocation.row][currentLocation.col] = '?';
        sc.pop_front();
        while (!sc.empty()) {
            currentLocation = sc.front();
            sc.pop_front();

            if (islower(backtrace[color_to_room(currentLocation.color)][currentLocation.row][currentLocation.col])
                || backtrace[color_to_room(currentLocation.color)][currentLocation.row][currentLocation.col] == '^') {
                backtrace[color_to_room(currentLocation.color)][currentLocation.row][currentLocation.col] = '@';

                if (sc.empty()) {   // check if sc is not empty before accessing next element
                    break;
                }
                currentLocation = sc.front();
                sc.pop_front();
                backtrace[color_to_room(currentLocation.color)][currentLocation.row][currentLocation.col] = '%';
            } else if (backtrace[color_to_room(currentLocation.color)][currentLocation.row][currentLocation.col]
                       == '@') {
                break;
            } else {
                backtrace[color_to_room(currentLocation.color)][currentLocation.row][currentLocation.col] = '+';
            }
        }

        for (char c = '^'; printedColors.size() < num_colors + 1; ++c) {
            if (c == '^' + 1)   // Jump to 'a' after '^'
                c = 'a';

            if (printedColors.find(c) != printedColors.end()) continue;

            cout << "// color " << c << "\n";
            color_map(c);
            printedColors.insert(c);
        }
    } else {
        cerr << "Wrong output type:" << output_type << "\n";
        exit(1);
    }
}

bool Solver::is_valid_character(char& ch, uint32_t& num_colors) {
    if (ch == '.' || ch == '^' || ch == '#' || ch == '@' || ch == '?') {
        return true;
    }
    if ('A' <= ch && static_cast<uint32_t>(ch) < 'A' + num_colors) {
        return true;   // valid door
    }
    if ('a' <= ch && static_cast<uint32_t>(ch) < 'a' + num_colors) {
        return true;   // valid button
    }
    cerr << "Error: Invalid character.\n";
    exit(1);
}

bool Solver::is_button(uint32_t& num_colors, char& current_loc, char& current_state) {
    // Check if current_loc is not a lowercase letter
    if (current_loc == '.' || current_loc == '@' || current_loc == '?') {
        return false;
    }

    if (current_loc >= 'A' && current_loc <= 'Z') {   // is a door so not a button
        return false;
    }

    // Check if the lowercase letter exceeds the number of colors
    if (static_cast<uint32_t>(current_loc) >= 'a' + num_colors) {
        cerr << "Error: Lowercase letter exceeds the number of colors.\n";
        exit(1);
    }

    // If current_loc matches the current_state, then this button is already pressed
    if (current_loc == current_state) {   // inactive button
        return false;
    }

    // If we reach here, it means current_loc is an active button or trap
    return true;
}

bool Solver::is_valid_location(location& loc, char& current_color) {
    // Check if state is off the edge of the map
    if (loc.row >= map.size() || loc.col >= map[0].size()) {
        return false;
    }

    // Check if state is a wall
    if (map[loc.row][loc.col] == '#') {
        return false;
    }
    // printf("I am here 2\n");

    if (current_color == '^' && isupper(map[loc.row][loc.col])) {   // cant go through door when state is '^'
        return false;
    }

    if (isupper(map[loc.row][loc.col])
        && map[loc.row][loc.col] != toupper(current_color)) {   // is a door (capital letter)
                                                                // our current color is not its lowercase
        return false;
    }
    //  check if the state has been previously discovered.
    if (backtrace[color_to_room(current_color)][loc.row][loc.col] != '\0') {
        // printf("This color_to room is not the problem\n");
        return false;
    }
    // printf("it is valid\n");
    return true;
}

int Solver::color_to_room(char color) {
    if (color == '^') {
        return 0;
    } else if (color >= 'a' && color <= 'z') {   // Assuming 'z' as the maximum possible color
        return static_cast<int>(color - 'a' + 1);
    }
    cerr << "Wrong character:" << color << "\n";
    exit(1);
}

void Solver::color_map(char colorCharDef) {
    int colorChar = color_to_room((colorCharDef));

    for (uint32_t row = 0; row < map.size(); ++row) {
        for (uint32_t col = 0; col < map[row].size(); ++col) {
            if (backtrace[colorChar][row][col] == '%' || backtrace[colorChar][row][col] == '@'
                || backtrace[colorChar][row][col] == '+') {
                cout << backtrace[colorChar][row][col];
            } else if (colorChar == 0 && map[row][col] == '^') {
                cout << '.';
            } else if (map[row][col] == colorCharDef || map[row][col] == toupper(colorCharDef)) {
                cout << ".";
            } else if (map[row][col] == '@') {
                cout << ".";
            } else {
                cout << map[row][col];
            }
        }
        cout << '\n';
    }
}
