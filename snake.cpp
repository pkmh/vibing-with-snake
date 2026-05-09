// Snake for the terminal
// Compile: g++ -std=c++17 -O2 snake.cpp -lncurses -o snake
// Run:     ./snake
//
// Controls:
//   Arrow keys / WASD : change direction
//   P                 : pause
//   R                 : restart after game over
//   Q                 : quit
 
#include <ncurses.h>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <thread>
 
constexpr int BOARD_W = 40;
constexpr int BOARD_H = 20;
 
enum Dir { UP, DOWN, LEFT, RIGHT };
 
struct Point {
    int x, y;
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
};
 
struct Game {
    std::deque<Point> snake;
    Point food{};
    Dir dir = RIGHT;
    Dir pendingDir = RIGHT;
    int score = 0;
    int best = 0;
    bool over = false;
    bool paused = false;
 
    Game() {
        std::srand((unsigned)std::time(nullptr));
        reset();
    }
 
    void reset() {
        snake.clear();
        int cx = BOARD_W / 2, cy = BOARD_H / 2;
        snake.push_back({cx,     cy});
        snake.push_back({cx - 1, cy});
        snake.push_back({cx - 2, cy});
        dir = RIGHT;
        pendingDir = RIGHT;
        score = 0;
        over = false;
        paused = false;
        placeFood();
    }
 
    bool occupied(Point p) const {
        for (auto& s : snake) if (s == p) return true;
        return false;
    }
 
    void placeFood() {
        // Bail out if the board is somehow full.
        if ((int)snake.size() >= BOARD_W * BOARD_H) return;
        while (true) {
            Point p{ std::rand() % BOARD_W, std::rand() % BOARD_H };
            if (!occupied(p)) { food = p; return; }
        }
    }
 
    bool opposite(Dir a, Dir b) const {
        return (a == UP && b == DOWN) || (a == DOWN && b == UP) ||
               (a == LEFT && b == RIGHT) || (a == RIGHT && b == LEFT);
    }
 
    void setDir(Dir d) {
        // Block 180-degree turns relative to the *committed* direction so
        // that double-tapping in one tick can't fold the snake into itself.
        if (!opposite(dir, d)) pendingDir = d;
    }
 
    void step() {
        if (over || paused) return;
        dir = pendingDir;
 
        Point head = snake.front();
        switch (dir) {
            case UP:    head.y -= 1; break;
            case DOWN:  head.y += 1; break;
            case LEFT:  head.x -= 1; break;
            case RIGHT: head.x += 1; break;
        }
 
        // Wall collision
        if (head.x < 0 || head.x >= BOARD_W || head.y < 0 || head.y >= BOARD_H) {
            over = true;
            if (score > best) best = score;
            return;
        }
 
        bool ate = (head == food);
 
        // Self collision: tail will move unless we ate, so the last cell is
        // about to be vacated and is safe to walk into.
        for (size_t i = 0; i < snake.size(); i++) {
            if (!ate && i == snake.size() - 1) break;
            if (snake[i] == head) {
                over = true;
                if (score > best) best = score;
                return;
            }
        }
 
        snake.push_front(head);
        if (ate) {
            score += 10;
            placeFood();
        } else {
            snake.pop_back();
        }
    }
 
    int tickMs() const {
        // Speed up gradually as the snake grows. Floor keeps it playable.
        int ms = 120 - (int)snake.size();
        return ms < 50 ? 50 : ms;
    }
};
 
static void render(const Game& g) {
    erase();
 
    int originY = 1;
    int originX = 2;
 
    // Top/bottom borders
    for (int x = 0; x < BOARD_W * 2 + 2; x++) {
        mvaddch(originY - 1,            originX - 1 + x, '-');
        mvaddch(originY + BOARD_H,      originX - 1 + x, '-');
    }
    // Side borders
    for (int y = 0; y < BOARD_H; y++) {
        mvaddch(originY + y, originX - 1,             '|');
        mvaddch(originY + y, originX + BOARD_W * 2,   '|');
    }
    // Corners
    mvaddch(originY - 1,       originX - 1,           '+');
    mvaddch(originY - 1,       originX + BOARD_W * 2, '+');
    mvaddch(originY + BOARD_H, originX - 1,           '+');
    mvaddch(originY + BOARD_H, originX + BOARD_W * 2, '+');
 
    // Food
    attron(COLOR_PAIR(2));
    mvaddstr(originY + g.food.y, originX + g.food.x * 2, "()");
    attroff(COLOR_PAIR(2));
 
    // Snake (head distinct from body)
    for (size_t i = 0; i < g.snake.size(); i++) {
        const auto& s = g.snake[i];
        attron(COLOR_PAIR(1));
        if (i == 0) {
            mvaddstr(originY + s.y, originX + s.x * 2, "@@");
        } else {
            mvaddstr(originY + s.y, originX + s.x * 2, "##");
        }
        attroff(COLOR_PAIR(1));
    }
 
    // Side panel
    int panelX = originX + BOARD_W * 2 + 4;
    mvprintw(originY + 0, panelX, "SNAKE");
    mvprintw(originY + 2, panelX, "Score:  %d", g.score);
    mvprintw(originY + 3, panelX, "Best:   %d", g.best);
    mvprintw(originY + 4, panelX, "Length: %d", (int)g.snake.size());
 
    mvprintw(originY + 6,  panelX, "Controls:");
    mvprintw(originY + 7,  panelX, " arrows / WASD");
    mvprintw(originY + 8,  panelX, " p  pause");
    mvprintw(originY + 9,  panelX, " r  restart");
    mvprintw(originY + 10, panelX, " q  quit");
 
    if (g.paused) {
        mvprintw(originY + BOARD_H / 2, originX + BOARD_W - 4, " PAUSED ");
    }
    if (g.over) {
        mvprintw(originY + BOARD_H / 2,     originX + BOARD_W - 6, " GAME OVER ");
        mvprintw(originY + BOARD_H / 2 + 1, originX + BOARD_W - 7, " press r to retry ");
    }
 
    refresh();
}
 
int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
 
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_GREEN, -1); // snake
        init_pair(2, COLOR_RED,   -1); // food
    }
 
    Game g;
    auto last = std::chrono::steady_clock::now();
 
    while (true) {
        int ch = getch();
        if (ch != ERR) {
            if (ch == 'q' || ch == 'Q') break;
            if (ch == 'p' || ch == 'P') { if (!g.over) g.paused = !g.paused; }
            else if (ch == 'r' || ch == 'R') { if (g.over) g.reset(); }
            else if (!g.paused && !g.over) {
                switch (ch) {
                    case KEY_UP:    case 'w': case 'W': g.setDir(UP);    break;
                    case KEY_DOWN:  case 's': case 'S': g.setDir(DOWN);  break;
                    case KEY_LEFT:  case 'a': case 'A': g.setDir(LEFT);  break;
                    case KEY_RIGHT: case 'd': case 'D': g.setDir(RIGHT); break;
                }
            }
        }
 
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
        if (elapsed >= g.tickMs()) {
            g.step();
            last = now;
        }
 
        render(g);
        napms(16); // ~60 FPS for input responsiveness
    }
 
    endwin();
    return 0;
}