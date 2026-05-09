# vibing-with-snake
[May 9th 2026] I'm a reluctant vibe-coder, and much of my work is too sensitive to use AI for. But, I want to make sure I keep on top of the tech landscape as these tools evolve so I'm experimenting with vibe-coding in some of my free time. Most of this README file & the C++ code is all thanks to Claude!!

The goal This time? To play Snake in your CLI!!

All I prompted Claude with was "Hey Claude! Can you write another C++ program for me? I want to play Snake in my CLI" This took Claude significantly longer than Tetris, but it was still very fast. I think thats understandable too, considering the popularity of Tetris as a game and as a programming task

## set-up
You'll need ncurses installed (`sudo apt install libncurses-dev` on Debian/Ubuntu, `brew install ncurses` on macOS — usually preinstalled).

## compile and run
```
g++ -std=c++17 -O2 snake.cpp -lncurses -o snake
./snake
```

## controls
- Arrow keys / WASD to change direction
- P to pause
- R to restart after game over
- Q to quit

## notes
the snake speeds up gradually as it grows (from 120ms per tick down to a 50ms floor), 180-degree turns are blocked so you can't accidentally fold the snake into itself, and there's a session best score tracked across restarts. Eating food gives you 10 points and grows the snake by one segment.
