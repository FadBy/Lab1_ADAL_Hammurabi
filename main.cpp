#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <cmath>
#include <iomanip>
#include <limits>
#include <cctype>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

const char* SAVE_FILE = "egypt_save.txt";

struct GameState {
    int round = 1;
    long long population = 100;
    long long acres = 1000;
    long long bushels = 2800;
    double totalStarvedPercent = 0.0;

    long long lastStarved = 0;
    long long lastArrived = 0;
    bool lastPlague = false;
    long long lastHarvest = 0;
    int lastYield = 0;
    long long lastRats = 0;
};

long long randRange(long long low, long long high) {
    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<long long> distribution(low, high);
    return distribution(generator);
}

void saveGame(const GameState& state) {
    std::ofstream out(SAVE_FILE);
    if (!out) {
        std::cout << "Не удалось сохранить прогресс.\n";
        return;
    }
    out << state.round << '\n'
        << state.population << '\n'
        << state.acres << '\n'
        << state.bushels << '\n'
        << std::setprecision(15) << state.totalStarvedPercent << '\n'
        << state.lastStarved << '\n'
        << state.lastArrived << '\n'
        << state.lastPlague << '\n'
        << state.lastHarvest << '\n'
        << state.lastYield << '\n'
        << state.lastRats << '\n';
}

bool loadGame(GameState& state) {
    std::ifstream in(SAVE_FILE);
    if (!in) {
        return false;
    }

    GameState loaded;
    bool ok = static_cast<bool>(
        in >> loaded.round
           >> loaded.population
           >> loaded.acres
           >> loaded.bushels
           >> loaded.totalStarvedPercent
           >> loaded.lastStarved
           >> loaded.lastArrived
           >> loaded.lastPlague
           >> loaded.lastHarvest
           >> loaded.lastYield
           >> loaded.lastRats);

    if (!ok) {
        return false;
    }
    if (loaded.round < 1 || loaded.round > 10 ||
        loaded.population < 0 || loaded.acres < 0 || loaded.bushels < 0 ||
        loaded.totalStarvedPercent < 0.0) {
        return false;
    }

    state = loaded;
    return true;
}

void deleteSave() {
    std::remove(SAVE_FILE);
}

bool askYesNo(const std::string& prompt) {
    std::string answer;
    while (true) {
        std::cout << prompt;
        if (!(std::cin >> answer)) {
            return false;
        }
        if (!answer.empty()) {
            char first = static_cast<char>(std::tolower(static_cast<unsigned char>(answer[0])));
            if (first == 'y') {
                return true;
            }
            if (first == 'n') {
                return false;
            }
        }
        std::cout << "Пожалуйста, ответьте 'y' (да) или 'n' (нет).\n";
    }
}

long long readPositiveInteger(const std::string& prompt) {
    long long value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            if (value >= 0) {
                return value;
            }
        }
        if (std::cin.eof()) {
            std::cout << "\nВвод завершён. Игра окончена.\n";
            std::exit(0);
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Пожалуйста, введите целое неотрицательное число.\n";
    }
}

void printError(const std::string& error, const GameState& state) {
    std::cout << error << "\n";
    std::cout << "О, повелитель, пощади нас! У нас только " << state.population
              << " человек, " << state.bushels << " бушелей пшеницы и "
              << state.acres << " акров земли!\n";
}

void printReport(const GameState& state, long long price) {
    std::cout << "\nМой повелитель, соизволь поведать тебе\n";
    std::cout << "  в году " << state.round << " твоего высочайшего правления\n";
    if (state.lastStarved > 0) {
        std::cout << "  " << state.lastStarved << " человек умерли с голоду;\n";
    }
    if (state.lastArrived > 0) {
        std::cout << "  " << state.lastArrived << " человек прибыли в наш великий город;\n";
    }
    if (state.lastPlague) {
        std::cout << "  Чума уничтожила половину населения;\n";
    }
    std::cout << "  Население города сейчас составляет " << state.population << " человек;\n";
    if (state.round > 1) {
        std::cout << "  Мы собрали " << state.lastHarvest << " бушелей пшеницы, по "
                  << state.lastYield << " бушеля с акра;\n";
        std::cout << "  Крысы истребили " << state.lastRats << " бушелей пшеницы, оставив "
                  << state.bushels << " бушеля в амбарах;\n";
    }
    std::cout << "  Город сейчас занимает " << state.acres << " акров;\n";
    std::cout << "  1 акр земли стоит сейчас " << price << " бушель.\n\n";
}

void printFinalReport(const GameState& state) {
    double starvedAverage = state.totalStarvedPercent / 10.0;
    double landPerCitizen = state.population > 0
        ? static_cast<double>(state.acres) / static_cast<double>(state.population)
        : 0.0;

    std::cout << "\n=== Итоги правления ===\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Среднегодовой процент умерших от голода: " << starvedAverage << "%\n";
    std::cout << "Акров земли на одного жителя: " << landPerCitizen << "\n\n";

    if (starvedAverage > 33.0 && landPerCitizen < 7.0) {
        std::cout << "Из-за вашей некомпетентности в управлении народ устроил бунт "
                     "и изгнал вас из города. Теперь вы вынуждены влачить жалкое "
                     "существование в изгнании.\n";
    } else if (starvedAverage > 10.0 && landPerCitizen < 9.0) {
        std::cout << "Вы правили железной рукой, подобно Нерону и Ивану Грозному. "
                     "Народ вздохнул с облегчением, и никто больше не желает видеть "
                     "вас правителем.\n";
    } else if (starvedAverage > 3.0 && landPerCitizen < 10.0) {
        std::cout << "Вы справились вполне неплохо, у вас, конечно, есть "
                     "недоброжелатели, но многие хотели бы увидеть вас во главе "
                     "города снова.\n";
    } else {
        std::cout << "Фантастика! Карл Великий, Дизраэли и Джефферсон вместе "
                     "не справились бы лучше.\n";
    }
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    GameState state;
    if (loadGame(state)) {
        std::cout << "Найден сохранённый прогресс (год " << state.round << ").\n";
        if (!askYesNo("Продолжить предыдущую игру? (y/n): ")) {
            state = GameState();
        }
    }

    while (state.round <= 10) {
        std::cout << "\n=== Год " << state.round << " правления ===\n";
        if (state.round > 1 && askYesNo("Прервать игру и сохранить прогресс? (y/n): ")) {
            saveGame(state);
            std::cout << "Прогресс сохранён. До встречи, повелитель.\n";
            return 0;
        }

        long long price = randRange(17, 26);
        printReport(state, price);

        long long buy = 0;
        long long sell = 0;
        long long food = 0;
        long long sow = 0;

        while (true) {
            std::string error;

            buy = readPositiveInteger("Сколько акров земли повелеваешь купить? ");
            if (buy > 0 && buy > state.bushels / price) {
                error = "У нас недостаточно пшеницы, чтобы оплатить покупку земли.";
                printError(error, state);
                continue;
            }
            sell = readPositiveInteger("Сколько акров земли повелеваешь продать? ");
            long long acresAfter = state.acres + buy - sell;
            if (acresAfter < 0) {
                error = "У нас нет столько земли, чтобы продать.";
                printError(error, state);
                continue;
            } 
            food = readPositiveInteger("Сколько бушелей пшеницы повелеваешь съесть? ");
            sow = readPositiveInteger("Сколько акров земли повелеваешь засеять? ");
            
            long long moneyAfter = state.bushels - buy * price + sell * price;
            long long seedCost = sow / 2;
            if (food + seedCost > moneyAfter) {
                error = "У нас недостаточно пшеницы на еду и семена.";
            } else if (sow > acresAfter) {
                error = "Нельзя засеять больше земли, чем у нас есть.";
            } else if (sow > state.population * 10) {
                error = "У нас слишком мало людей, чтобы обработать столько земли.";
            } else {
                break;
            }

            printError(error, state);
        }

        state.bushels = state.bushels - buy * price + sell * price;
        state.acres = state.acres + buy - sell;
        long long seedCost = sow / 2;
        state.bushels -= seedCost;

        long long yield = randRange(1, 6);
        long long harvest = yield * sow;
        state.bushels += harvest;

        state.bushels -= food;
        state.bushels += food % 20;

        long long stockForNewcomers = state.bushels;
        long long maxRats = static_cast<long long>(0.07 * state.bushels);
        long long rats = randRange(0, maxRats);
        state.bushels -= rats;

        long long starved = 0;
        if (food / 20 < state.population) {
            starved = state.population - food / 20;
        }

        double starvedPercent = 100.0 * starved / state.population;

        
        state.population -= starved;
        state.totalStarvedPercent += starvedPercent;

        state.lastStarved = starved;
        state.lastHarvest = harvest;
        state.lastYield = yield;
        state.lastRats = rats;

        if (starvedPercent > 45.0) {
            std::cout << "\nБолее 45% населения умерли от голода! Народ сверг вас. "
                         "Игра окончена.\n";
            deleteSave();
            return 0;
        }
        if (state.population <= 0) {
            std::cout << "\nВсё население вымерло. Игра окончена.\n";
            deleteSave();
            return 0;
        }

        double newcomersBase = static_cast<double>(starved / 2)
            + static_cast<double>(5 - yield) * static_cast<double>(stockForNewcomers) / 600.0
            + 1.0;
        long long arrived = static_cast<long long>(std::floor(newcomersBase));
        if (arrived < 0) {
            arrived = 0;
        }
        if (arrived > 50) {
            arrived = 50;
        }
        state.population += arrived;
        state.lastArrived = arrived;

        bool plague = randRange(1, 100) <= 15;
        if (plague) {
            state.population /= 2;
        }
        state.lastPlague = plague;

        if (state.population <= 0) {
            std::cout << "\nВсё население вымерло. Игра окончена.\n";
            deleteSave();
            return 0;
        }

        ++state.round;
    }

    printFinalReport(state);
    deleteSave();
    return 0;
}
