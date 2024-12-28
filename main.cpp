//By Shehr Bano (NUM-BSCS-23-11) - Software Engineering Assignment Task 2
#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <windows.h>

using namespace std;

class Ant;
class Room;
template<typename T> class AntFarm;
class AntMediator;
class Queen;
class Worker;
class Warrior;

// my position holder
struct Position {
    int x;
    int y;
};

// all my colors in one place
namespace Color {
    const string RED     = "\033[31m";
    const string GREEN   = "\033[32m";
    const string YELLOW  = "\033[33m";
    const string BLUE    = "\033[34m";
    const string MAGENTA = "\033[35m";
    const string CYAN    = "\033[36m";
    const string WHITE   = "\033[37m";
    const string RESET   = "\033[0m";
    const string BOLD    = "\033[1m";
}

// keeps track of ants and helps them fight
class AntMediator {
private:
    vector<Ant*> all_ants;

public:
    void add_new_ant(Ant* ant);
    void remove_dead_ant(Ant* ant);
    void do_ant_actions();
    Ant* find_enemy(Ant* my_ant);
};

// basic ant skills (for decoration pattern)
class AntAttributes {
public:
    virtual ~AntAttributes() = default;
    virtual int getAttackBonus() const = 0;
    virtual int getDefenseBonus() const = 0;
    virtual int getHarvestingBonus() const = 0;
    virtual string getDescription() const = 0;
};

// regular ant abilities
class BaseAttributes : public AntAttributes {
public:
    int getAttackBonus() const override { return 0; }
    int getDefenseBonus() const override { return 0; }
    int getHarvestingBonus() const override { return 0; }
    string getDescription() const override { return "Basic Ant"; }
};

// special ant abilities based on species
class SpeciesAttributes : public AntAttributes {
private:
    AntAttributes* base_attrs;
    string ant_species;
    int attack_power;
    int defense_power;
    int harvest_power;

public:
    SpeciesAttributes(AntAttributes* basic_attrs,
                     const string& species_name,
                     int attack, int defense, int harvesting)
        : base_attrs(basic_attrs)
        , ant_species(species_name)
        , attack_power(attack)
        , defense_power(defense)
        , harvest_power(harvesting) {}

    ~SpeciesAttributes() {
        delete base_attrs;
    }

    int getAttackBonus() const override {
        return base_attrs->getAttackBonus() + attack_power;
    }

    int getDefenseBonus() const override {
        return base_attrs->getDefenseBonus() + defense_power;
    }

    int getHarvestingBonus() const override {
        return base_attrs->getHarvestingBonus() + harvest_power;
    }

    string getDescription() const override {
        return base_attrs->getDescription() + " + " + ant_species;
    }
};

// info about different ant types
struct SpeciesInfo {
    string name;
    string description;
    int attack;
    int defense;
    int harvesting;
    string color;
};

// all my ant species, used the formula (Student_Rollno MOD 6) + 10. Since my Roll Number is 11, it gives answer 15
const vector<SpeciesInfo> SPECIES_LIST = {
    {"FireAnts",     "Aggressive warriors with burning attacks", 25, 10, 5,  Color::RED},
    {"Harvesters",   "Expert food gatherers", 5, 15, 25, Color::GREEN},
    {"Guardians",    "Defensive specialists", 10, 30, 5,  Color::BLUE},
    {"Assassins",    "Stealth hunters", 30, 5, 5,   Color::MAGENTA},
    {"Builders",     "Fast room constructors", 5, 20, 15, Color::YELLOW},
    {"Balanced",     "Jack of all trades", 15, 15, 15, Color::CYAN},
    {"Giants",       "High health and attack", 25, 25, 5,  Color::RED},
    {"Scouts",       "Fast movers and gatherers", 10, 10, 20, Color::GREEN},
    {"Royal",        "Enhanced queen abilities", 15, 20, 10, Color::MAGENTA},
    {"Swarmers",     "Strength in numbers", 12, 12, 12, Color::YELLOW},
    {"Poisoners",    "Toxic attacks", 28, 8, 5,   Color::GREEN},
    {"Defenders",    "Ultimate protection", 5, 35, 5,   Color::BLUE},
    {"Scavengers",   "Efficient resource hunters", 8, 8, 28,  Color::CYAN},
    {"Berserkers",   "Rage-fueled warriors", 35, 5, 5,   Color::RED},
    {"Engineers",    "Master builders", 5, 15, 25,  Color::YELLOW}
};

// Main colony class
template<typename AntType>
class AntFarm {
private:
    vector<AntType*> ant_list;
    vector<Room*> room_list;
    Position colony_pos;
    string colony_species;
    int food_storage;
    int colony_id;
    static int next_colony_id;

    // stats for colony
    int ants_killed;
    int colonies_killed;
    int total_warriors;  // Track warrior count
    int total_workers;   // Track worker count
    int days_alive;
    bool colony_alive;
    vector<string> beaten_colonies;

public:
    AntFarm(vector<Room*>& starting_rooms,
            Position pos,
            const string& species)
        : room_list()
        , colony_pos(pos)
        , colony_species(species)
        , food_storage(0)
        , colony_id(next_colony_id++)
        , ants_killed(0)
        , colonies_killed(0)
        , total_warriors(0)
        , total_workers(0)
        , days_alive(0)
        , colony_alive(true)
    {
        for(int i = 0; i < starting_rooms.size(); i++) {
            room_list.push_back(starting_rooms[i]);
        }
    }


    void setMediator(AntMediator* mediator) {
        for(int i = 0; i < ant_list.size(); i++) {
            ant_list[i]->setMediator(mediator);
        }
    }

    vector<Room*>& getRooms() { return room_list; }

    ~AntFarm() {
        for(int i = 0; i < ant_list.size(); i++) {
            delete ant_list[i];
        }
        for(int i = 0; i < room_list.size(); i++) {
            delete room_list[i];
        }
    }

    void tick() {
        if (!colony_alive) return;
        days_alive++;

        for(int i = 0; i < ant_list.size(); i++) {
            if (ant_list[i] && !ant_list[i]->isResting()) {
                ant_list[i]->performAction();
            }
        }

        useFood();
    }

   void useFood() {
        if (!colony_alive) return;

        int total_food_needed = 0;
        for(int i = 0; i < ant_list.size(); i++) {
            if (ant_list[i]) {
                total_food_needed += ant_list[i]->food_requirement;
                if (food_storage < total_food_needed) {
                    ant_list[i]->health_points -= 10;
                    if (ant_list[i]->health_points <= 0) {
                        if (ant_list[i]->game_mediator) {
                            ant_list[i]->game_mediator->remove_dead_ant(ant_list[i]);
                        }
                    }
                }
            }
        }

        food_storage -= total_food_needed;
        if (food_storage < 0) food_storage = 0;
    }

    void addFood(int amount) {
        food_storage += amount;
    }

  void mergeColony(AntFarm* beaten_colony) {
    if (!beaten_colony) return;

    // Create combined species attributes for all ants
    string combined_species = colony_species + "-" + beaten_colony->colony_species;

    // Transfer and upgrade all ants
    for(auto* ant : beaten_colony->ant_list) {
        // Update ant's attributes to include both species' bonuses
        AntAttributes* combined_attrs = new SpeciesAttributes(
            ant->ant_powers,
            combined_species,
            ant->getAttack(),
            ant->getDefense(),
            ant->ant_powers->getHarvestingBonus()
        );

        ant->ant_powers = combined_attrs;
        ant->setColony(this);
        ant_list.push_back(ant);
    }

    beaten_colony->ant_list.clear();
    colonies_killed++;
    beaten_colonies.push_back(beaten_colony->colony_species);

    // Combine food stores
    food_storage += beaten_colony->food_storage;
    beaten_colony->colony_alive = false;

    // Update colony species name to show merger
    colony_species = combined_species;
}

    void addWarrior() { total_warriors++; }
    void addWorker() { total_workers++; }

    int getId() const { return colony_id; }
    const string& getSpecies() const { return colony_species; }
    Position getPosition() const { return colony_pos; }
    int getFood() const { return food_storage; }
    bool isActive() const { return colony_alive; }

    void displayStatus() const {
        cout << Color::BOLD << "\nColony #" << colony_id << " Status Report" << Color::RESET << "\n";

        // make a line
        for(int i = 0; i < 40; i++) {
            cout << "=";
        }
        cout << "\n";

        string species_color = Color::WHITE;
        for(int i = 0; i < SPECIES_LIST.size(); i++) {
            if (SPECIES_LIST[i].name == colony_species) {
                species_color = SPECIES_LIST[i].color;
                break;
            }
        }

        cout << "Colony Type: " << species_color << colony_species << Color::RESET << "\n";
        cout << "Location: (" << colony_pos.x << ", " << colony_pos.y << ")\n";
        cout << "Food Available: " << Color::YELLOW << food_storage << Color::RESET << "\n";
        cout << "Total Ants: " << Color::CYAN << ant_list.size() << Color::RESET << "\n";
        cout << "Colony Age: " << days_alive << " days\n";
        cout << "Warriors: " << Color::RED << total_warriors << Color::RESET << "\n";
        cout << "Workers: " << Color::GREEN << total_workers << Color::RESET << "\n\n";

        cout << "Battle History:\n";
        cout << "  Ants Defeated: " << Color::RED << ants_killed << Color::RESET << "\n";
        cout << "  Colonies Conquered: " << Color::MAGENTA << colonies_killed << Color::RESET << "\n";

        if (!beaten_colonies.empty()) {
            cout << "Defeated Colony Types: ";
            for(int i = 0; i < beaten_colonies.size(); i++) {
                cout << Color::RED << beaten_colonies[i] << Color::RESET << " ";
            }
            cout << "\n";
        }

        cout << "\nRooms Status:\n";
        for(int i = 0; i < room_list.size(); i++) {
            cout << "  - " << Color::CYAN << room_list[i]->getType() << Color::RESET
                 << " [Ants: " << room_list[i]->getCurrentAntCount() << "/" << room_list[i]->getRoomCapacity()
                 << "] - " << (room_list[i]->isFinishedBuilding() ?
                    "Complete" : to_string(room_list[i]->getBuildProgress()) +
                    "/" + to_string(room_list[i]->getRequiredBuildTime()) + " days")
                 << "\n";
        }

        cout << "\nColony State: " << (colony_alive ? Color::GREEN + "Active" : Color::RED + "Defeated") << Color::RESET << "\n";

        // make another line
        for(int i = 0; i < 40; i++) {
            cout << "=";
        }
        cout << "\n";
    }
};

// main ant class - all ants come from this
class Ant {
protected:
    int health_points;
    AntMediator* game_mediator;
    AntAttributes* ant_powers;
    bool is_taking_rest;
    int food_requirement;
    template<typename T> friend class AntFarm;
    AntFarm<Ant>* my_colony;
    string ant_type; // store ant type explicitly

public:
    Ant(int starting_health, AntAttributes* powers, int food_needed, string type)
        : health_points(starting_health)
        , ant_powers(powers)
        , is_taking_rest(false)
        , food_requirement(food_needed)
        , my_colony(nullptr)
        , game_mediator(nullptr)
        , ant_type(type) {}

    virtual ~Ant() {
        delete ant_powers;
    }

    virtual void performAction() = 0;
    int getAttack() const { return ant_powers->getAttackBonus(); }
    int getDefense() const { return ant_powers->getDefenseBonus(); }
    string getType() const { return ant_type; }

    bool isResting() const { return is_taking_rest; }
    void rest() { is_taking_rest = true; }
    void wakeUp() { is_taking_rest = false; }

    template<typename T>
    void setColony(AntFarm<T>* new_colony) {
        my_colony = (AntFarm<Ant>*)new_colony;
    }

    AntFarm<Ant>* getColony() const { return my_colony; }
    void setMediator(AntMediator* m) { game_mediator = m; }
    AntMediator* getMediator() const { return game_mediator; }

    bool battle(Ant* enemy) {
    if (!enemy || enemy->health_points <= 0 || health_points <= 0) return false;

    int my_attack = ant_powers->getAttackBonus();
    int my_defense = ant_powers->getDefenseBonus();
    int their_attack = enemy->ant_powers->getAttackBonus();
    int their_defense = enemy->ant_powers->getDefenseBonus();

    int damage_dealt = max(5, my_attack - (their_defense / 2) + (rand() % 11 - 5));
    int damage_taken = max(5, their_attack - (my_defense / 2) + (rand() % 11 - 5));

    enemy->health_points -= damage_dealt;
    health_points -= damage_taken;

    // Winner gets loser's attributes
    if (enemy->health_points <= 0) {
        // Create new combined attributes
        ant_powers = new SpeciesAttributes(
            ant_powers,  // Keep existing attributes
            enemy->getColony()->getSpecies(),  // Add enemy species
            enemy->getAttack(),
            enemy->getDefense(),
            enemy->ant_powers->getHarvestingBonus()
        );

        // If enemy was a queen, take control of their colony
        if (enemy->getType() == "Queen") {
            AntFarm<Ant>* enemy_colony = enemy->getColony();
            AntFarm<Ant>* my_colony = getColony();

            if (enemy_colony && my_colony) {
                my_colony->mergeColony(enemy_colony);
            }
        }

        if (enemy->game_mediator) {
            enemy->game_mediator->remove_dead_ant(enemy);
        }
        return true;
    }
    return false;
}
};
// Queen ant implementation
class Queen : public Ant {
private:
    int egg_timer;
    int egg_cooldown;

public:
    Queen(AntAttributes* powers)
        : Ant(200, powers, 5, "Queen")
        , egg_timer(0)
        , egg_cooldown(10) {}

    void performAction() override {
        egg_timer++;
        if (egg_timer >= egg_cooldown && my_colony) {
            egg_timer = 0;
        }
    }
};

// Worker ant implementation
class Worker : public Ant {
public:
    Worker(AntAttributes* powers)
        : Ant(100, powers, 2, "Worker") {}

    void performAction() override {
        if (my_colony) {
            int food_collected = 5 + ant_powers->getHarvestingBonus();
        }
    }
};

// Warrior ant implementation
class Warrior : public Ant {
public:
    Warrior(AntAttributes* powers)
        : Ant(150, powers, 3, "Warrior") {}

    void performAction() override {
        if (my_colony) {
            if (health_points < 50) {
                // Return to colony for food
            } else {
                // Look for enemies
            }
        }
    }
};

void AntMediator::add_new_ant(Ant* ant) {
    all_ants.push_back(ant);
}

// shuffle ants for random actions
void AntMediator::remove_dead_ant(Ant* ant) {
    for(auto it = all_ants.begin(); it != all_ants.end(); ++it) {
        if(*it == ant) {
            all_ants.erase(it);
            break;
        }
    }
}

// make each ant do something
void AntMediator::do_ant_actions() {
    vector<Ant*> active_ants = all_ants;
    for(auto it = active_ants.begin(); it != active_ants.end(); ++it) {
        if(*it && !(*it)->isResting()) {
            (*it)->performAction();
        }
    }
}

// find enemy ant to fight
Ant* AntMediator::find_enemy(Ant* my_ant) {
    for(auto ant : all_ants) {
        if(ant && ant != my_ant && ant->getColony() != my_ant->getColony()) {
            return ant;
        }
    }
    return nullptr;
}

// Base class for making different ant types
class AntFactory {
public:
    virtual ~AntFactory() = default;
    virtual Ant* createAnt(AntAttributes* powers) = 0;
};

class DroneFactory : public AntFactory {
public:
    Worker* createAnt(AntAttributes* powers) override {
        return new Worker(powers);
    }
};

class WarriorFactory : public AntFactory {
public:
    Warrior* createAnt(AntAttributes* powers) override {
        return new Warrior(powers);
    }
};

class QueenFactory : public AntFactory {
public:
    Queen* createAnt(AntAttributes* powers) override {
        return new Queen(powers);
    }
};


// Room in the colony
class Room {
protected:
    int max_ants;
    int build_time_done;
    int build_time_needed;
    vector<Ant*> ants_inside;
    AntFactory* ant_maker;

public:
    Room(int max_capacity, int build_ticks, AntFactory* factory)
        : max_ants(max_capacity)
        , build_time_done(0)
        , build_time_needed(build_ticks)
        , ant_maker(factory) {}

    virtual ~Room() {
        delete ant_maker;
    }

    virtual string getType() const = 0;

    Ant* createAnt(AntAttributes* powers) {
        if (ant_maker && canFitMoreAnts()) {
            Ant* new_ant = ant_maker->createAnt(powers);
            addAntToRoom(new_ant);
            if (AntMediator* mediator = new_ant->getMediator()) {
                mediator->add_new_ant(new_ant);
            }
            return new_ant;
        }
        return nullptr;
    }

    bool isFinishedBuilding() const {
        return build_time_done >= build_time_needed;
    }

    void continueBuildingRoom(int builder_count) {
        build_time_done += builder_count;
    }

    bool canFitMoreAnts() const {
        return ants_inside.size() < max_ants;
    }

    void addAntToRoom(Ant* ant) {
        if (canFitMoreAnts()) {
            ants_inside.push_back(ant);
        }
    }

    void removeAntFromRoom(Ant* ant) {
        for(int i = 0; i < ants_inside.size(); i++) {
            if(ants_inside[i] == ant) {
                ants_inside.erase(ants_inside.begin() + i);
                break;
            }
        }
    }

    int getBuildProgress() const { return build_time_done; }
    int getRequiredBuildTime() const { return build_time_needed; }
    int getRoomCapacity() const { return max_ants; }
    int getCurrentAntCount() const { return ants_inside.size(); }
};

// Different room types
class RestingRoom : public Room {
public:
    RestingRoom() : Room(5, 100, new DroneFactory()) {}
    string getType() const override { return "Resting Room"; }
};

class SpawningRoom : public Room {
public:
    SpawningRoom() : Room(3, 150, new WarriorFactory()) {}
    string getType() const override { return "Spawning Room"; }
};

// Builds the ant colonies
class AntFarmBuilder {
private:
    vector<Room*> room_list;
    Position colony_pos;
    string colony_species;

public:
    ~AntFarmBuilder() {
        for(int i = 0; i < room_list.size(); i++) {
            delete room_list[i];
        }
    }

    AntFarmBuilder& addRestingRoom() {
        room_list.push_back(new RestingRoom());
        return *this;
    }

    AntFarmBuilder& addSpawningRoom() {
        room_list.push_back(new SpawningRoom());
        return *this;
    }

    AntFarmBuilder& setPosition(int x, int y) {
        colony_pos.x = x;
        colony_pos.y = y;
        return *this;
    }

    AntFarmBuilder& setSpecies(const string& species) {
        colony_species = species;
        return *this;
    }

    template<typename AntType>
    AntFarm<AntType>* build() {
       AntFarm<AntType>* new_farm = new AntFarm<AntType>(room_list, colony_pos, colony_species);
        room_list.clear();
        return new_farm;
    }
};

// Main game controller
class Meadow {
private:
    static Meadow* single_instance;
    int max_colonies;
    AntFarm<Ant>** colony_list;  // Array of colony pointers
    int colony_count;
    QueenFactory* queen_maker;

    struct SpeciesAttribute {
        string name;
        int attack;
        int defense;
        int harvesting;
    };
    vector<SpeciesAttribute> species_powers;
    AntMediator game_mediator;

    Meadow() : max_colonies(100), colony_count(0), queen_maker(new QueenFactory()) {
        colony_list = new AntFarm<Ant>*[max_colonies];
        for(int i = 0; i < max_colonies; i++) {
            colony_list[i] = nullptr;
        }
        setupSpecies();
    }

    const SpeciesAttribute* findSpeciesAttributes(const string& species) {
        for(int i = 0; i < species_powers.size(); i++) {
            if (species_powers[i].name == species) {
                return &species_powers[i];
            }
        }
        return nullptr;
    }

    void setupSpecies() {
        for(int i = 0; i < SPECIES_LIST.size(); i++) {
            species_powers.push_back({
                SPECIES_LIST[i].name,
                SPECIES_LIST[i].attack,
                SPECIES_LIST[i].defense,
                SPECIES_LIST[i].harvesting
            });
        }
    }

public:
    static Meadow* getInstance() {
        if (!single_instance) {
            single_instance = new Meadow();
        }
        return single_instance;
    }

    ~Meadow() {
        delete queen_maker;
        for(int i = 0; i < max_colonies; i++) {
            delete colony_list[i];
        }
        delete[] colony_list;
    }

    int spawnColony(int x, int y, const string& species) {
        // Check if species exists
        bool found_species = false;
        for(int i = 0; i < species_powers.size(); i++) {
            if (species_powers[i].name == species) {
                found_species = true;
                break;
            }
        }

        if (!found_species) {
            cout << "Error: Species not found!\n";
            return -1;
        }

        AntFarmBuilder builder;
        Position pos = {x, y};
        AntFarm<Ant>* new_colony = builder
            .setPosition(x, y)
            .setSpecies(species)
            .addRestingRoom()
            .addSpawningRoom()
            .build<Ant>();

        new_colony->setMediator(&game_mediator);

        int id = new_colony->getId();
        colony_list[colony_count++] = new_colony;
        return id;
    }

    void tick(int count = 1) {
        for(int i = 0; i < count; i++) {
            game_mediator.do_ant_actions();

            for(int j = 0; j < colony_count; j++) {
                if(colony_list[j] && colony_list[j]->isActive()) {
                    colony_list[j]->tick();
                }
            }

            if(isGameOver()) {
                showWinner();
                break;
            }
        }
    }

    bool isGameOver() {
        int active_colonies = 0;
        for(int i = 0; i < colony_count; i++) {
            if(colony_list[i] && colony_list[i]->isActive()) {
                active_colonies++;
            }
        }
        return active_colonies <= 1;
    }

    void showWinner() {
        cout << Color::BOLD << "\n GAME FINISHED! \n" << Color::RESET;
        for(int i = 0; i < colony_count; i++) {
            if(colony_list[i] && colony_list[i]->isActive()) {
                cout << Color::GREEN << "Winner is Colony #" << colony_list[i]->getId()
                     << " (" << colony_list[i]->getSpecies() << ")\n" << Color::RESET;
                colony_list[i]->displayStatus();
                break;
            }
        }
    }

    void giveResources(int colony_id, const string& resource_type, int amount) {
        AntFarm<Ant>* target_colony = nullptr;
        for(int i = 0; i < colony_count; i++) {
            if(colony_list[i] && colony_list[i]->getId() == colony_id) {
                target_colony = colony_list[i];
                break;
            }
        }

        if (!target_colony) {
            cout << Color::RED << "Colony not found!\n" << Color::RESET;
            return;
        }

        if (resource_type == "food") {
            target_colony->addFood(amount);
            cout << Color::GREEN << "Added " << amount << " food to colony " << colony_id << "\n" << Color::RESET;
        }
        else if (resource_type == "warrior") {
            for(int i = 0; i < target_colony->getRooms().size(); i++) {
                Room* room = target_colony->getRooms()[i];
                if (room->getType() == "Spawning Room") {
                    for(int j = 0; j < amount; j++) {
                        AntAttributes* powers = new SpeciesAttributes(
                            new BaseAttributes(),
                            target_colony->getSpecies(),
                            15, 15, 5
                        );
                        room->createAnt(powers);
                        target_colony->addWarrior();  // Update warrior count
                    }
                    cout << Color::GREEN << "Made " << amount << " warriors in colony " << colony_id << "\n" << Color::RESET;
                    return;
                }
            }
        }
        else if (resource_type == "drone" || resource_type == "worker") {
            for(int i = 0; i < target_colony->getRooms().size(); i++) {
                Room* room = target_colony->getRooms()[i];
                if (room->getType() == "Resting Room") {
                    for(int j = 0; j < amount; j++) {
                        AntAttributes* powers = new SpeciesAttributes(
                            new BaseAttributes(),
                            target_colony->getSpecies(),
                            5, 5, 15
                        );
                        room->createAnt(powers);
                        target_colony->addWorker();  // Update worker count
                    }
                    cout << Color::GREEN << "Made " << amount << " workers in colony " << colony_id << "\n" << Color::RESET;
                    return;
                }
            }
        }
    }

    void displayColonySummary(int colony_id) {
        AntFarm<Ant>* target_colony = nullptr;
        for(int i = 0; i < colony_count; i++) {
            if(colony_list[i] && colony_list[i]->getId() == colony_id) {
                target_colony = colony_list[i];
                break;
            }
        }

        if (target_colony) {
            target_colony->displayStatus();
        } else {
            cout << Color::RED << "Colony not found!\n" << Color::RESET;
        }
    }

    void displayAllColonies() {
        cout << Color::BOLD << "\nActive Colonies List\n" << Color::RESET;
        for(int i = 0; i < 50; i++) cout << "=";
        cout << "\n";

        for(int i = 0; i < colony_count; i++) {
            if(colony_list[i] && colony_list[i]->isActive()) {
                Position pos = colony_list[i]->getPosition();
                cout << Color::CYAN << "Colony #" << colony_list[i]->getId() << Color::RESET
                     << " (" << colony_list[i]->getSpecies() << ") at "
                     << "(" << pos.x << "," << pos.y << ")\n";
            }
            }
        for(int i = 0; i < 50; i++) cout << "=";
        cout << "\n";
    }

private:
    // looks for colony matching the id number
    AntFarm<Ant>* findColony(int id) {
        for(int i = 0; i < colony_count; i++) {
            if(colony_list[i] && colony_list[i]->getId() == id) {
                return colony_list[i];
            }
        }
        return NULL;
    }
};

// start id counter from 1
Meadow* Meadow::single_instance = nullptr;

template<typename AntType>
int AntFarm<AntType>::next_colony_id = 1;

// waits for key press before clearing screen
void pauseBeforeClear() {
    cout << "                                                 Press Enter to continue...";
    cin.get();
}

// clears the screen
void clearScreen() {
    system("cls");
}

// shows cool title screen at start
void displayTitle() {
    Sleep(1000);
    cout << Color::CYAN << R"(




                    _          __    __   _________   ________      _          ________    __       __
                   / \        /  \  |  \ |         \ /        \    / \        /        \  /  \     /  \
                  / $$        | $$\ | $$  \ $$$$$$$$ | $$$$$$$$   / $$        | $$$$$$$$  | $$\   /  $$
                 / $$ $$      | $$$\| $$    | $$     | $$___     / $$ $$      | $$___ $$  | $$$\ /  $$$
                / $$___$$     | $$$$\ $$    | $$     | $$   \   / $$___$$     | $$   \$$  | $$$$$ $$$$$
               / $$    \$$    | $$\$$ $$    | $$     | $$$$$$  / $$    \$$    | $$$$$$$$  | $$  $$$  $$
              / $$$$$$$$$$$   | $$|\$$$$    | $$     | $$     / $$$$$$$$$$$   | $$ \$$    | $$       $$
             / $$         $$  | $$| \$$$    | $$     | $$    / $$         $$  | $$  \$$   | $$       $$
             \$$           $$ \ $$/  \$$     \$$      \$$    \$$           $$  \$$   \$$   \$$       $$




                                          ______________________________________
                                         | ____________________________________ |
                                         ||                                    ||
                                         ||   By Shehr Bano (NUM-BSCS-23-11)   ||
                                         ||____________________________________||
                                         |______________________________________|




)" << Color::RESET << "\n";
}

// shows all commands for user
void displayHelp() {
    cout << Color::BOLD << "\nAvailable Commands:\n" << Color::RESET;
    cout << Color::YELLOW << "1. spawn X Y Species" << Color::RESET << " - Create new colony at (X,Y) of specified species\n";
    cout << Color::YELLOW << "2. give ID Resource Amount" << Color::RESET << " - Give resources to colony (food/warrior/drone)\n";
    cout << Color::YELLOW << "3. tick [N]" << Color::RESET << " - Advance simulation by N ticks (default 1)\n";
    cout << Color::YELLOW << "4. summary ID" << Color::RESET << " - Display colony status\n";
    cout << Color::YELLOW << "5. list" << Color::RESET << " - Show all active colonies\n";
    cout << Color::YELLOW << "6. species" << Color::RESET << " - List available species\n";
    cout << Color::YELLOW << "7. help" << Color::RESET << " - Show this help menu\n";
    cout << Color::YELLOW << "8. quit" << Color::RESET << " - Exit simulation\n";
}

// show types of ants available
void displaySpecies() {
    cout << Color::BOLD << "\nAvailable Species:\n" << Color::RESET;
    for(int i = 0; i < 80; i++) cout << "=";
    cout << "\n";

    for(int i = 0; i < SPECIES_LIST.size(); i++) {
        cout << SPECIES_LIST[i].color << left << setw(15) << SPECIES_LIST[i].name << Color::RESET
             << " | " << SPECIES_LIST[i].description << "\n"
             << "Attack: " << SPECIES_LIST[i].attack
             << " | Defense: " << SPECIES_LIST[i].defense
             << " | Harvesting: " << SPECIES_LIST[i].harvesting << "\n";
        for(int j = 0; j < 80; j++) cout << "-";
        cout << "\n";
    }
}

int main() {
    srand(time(0));
    clearScreen();
    displayTitle();
    pauseBeforeClear();
    clearScreen();

cout << Color::BOLD << " ================== GAME INSTRUCTIONS ==================\n\n" << Color::RESET;

cout << Color::CYAN << " 1. Colony Setup:" << Color::RESET << "\n";
cout << "    - Create at least 2 colonies using 'spawn' command\n";
cout << "    - Each species has unique bonuses (check 'species' command)\n";
cout << "    - Position colonies strategically on the map\n\n";

cout << Color::GREEN << " 2. Resource Management:" << Color::RESET << "\n";
cout << "    - Provide food to colonies using 'give ID food amount'\n";
cout << "    - Create workers for harvesting: 'give ID drone amount'\n";
cout << "    - Train warriors for battles: 'give ID warrior amount'\n\n";

cout << Color::YELLOW << " 3. Colony Development:" << Color::RESET << "\n";
cout << "    - Rooms require worker-ticks to build\n";
cout << "    - More workers = faster building\n";
cout << "    - Limited resting capacity per room\n\n";

cout << Color::RED << " 4. Combat System:" << Color::RESET << "\n";
cout << "    - Warriors battle automatically during ticks\n";
cout << "    - Winners gain attributes of defeated ants\n";
cout << "    - Killing enemy queen lets you control their colony\n\n";

cout << Color::MAGENTA << " 5. Victory Conditions:" << Color::RESET << "\n";
cout << "    - Last surviving colony wins\n";
cout << "    - Keep your queen alive!\n\n";

cout << " Type 'help' for commands or 'species' to see available ant types.\n";
cout << " Additional commands:\n";
cout << " - 'build ID roomtype' to start room construction\n";
cout << " - 'rest ID antcount' to send ants to rest\n";

    Meadow* meadow = Meadow::getInstance();
    char cmd_buffer[100];
    bool game_started = false;
    int colony_num = 0;

    while(1) {
        cout << Color::BOLD << "\nEnter command > " << Color::RESET;
        cin.getline(cmd_buffer, 100);

        // get first word from command
        char first_word[20];
        int i = 0;
        while(cmd_buffer[i] != ' ' && cmd_buffer[i] != '\0') {
            first_word[i] = cmd_buffer[i];
            i++;
        }
        first_word[i] = '\0';

        if(strcmp(first_word, "spawn") == 0) {
            int x = 0, y = 0;
            char species_name[30];

            // get x position
            i++;
            int j = 0;
            char num[10];
            while(cmd_buffer[i] != ' ' && cmd_buffer[i] != '\0') {
                num[j] = cmd_buffer[i];
                i++;
                j++;
            }
            num[j] = '\0';
            x = strtol(num, NULL, 10);

            // get y position
            i++;
            j = 0;
            while(cmd_buffer[i] != ' ' && cmd_buffer[i] != '\0') {
                num[j] = cmd_buffer[i];
                i++;
                j++;
            }
            num[j] = '\0';
            y = strtol(num, NULL, 10);

            // get species name
            i++;
            j = 0;
            while(cmd_buffer[i] != '\0') {
                species_name[j] = cmd_buffer[i];
                i++;
                j++;
            }
            species_name[j] = '\0';

            if(strlen(species_name) > 0) {
                int colony_id = meadow->spawnColony(x, y, species_name);
                colony_num++;
                cout << Color::GREEN << "Created colony " << colony_id
                     << " of species " << species_name << " at (" << x << "," << y << ")\n"
                     << Color::RESET;

                if(colony_num >= 2 && !game_started) {
                    cout << Color::YELLOW << "\nYou now have enough colonies to start the simulation!\n"
                         << "Use 'tick' command to begin.\n" << Color::RESET;
                    game_started = true;
                }
            }
        }
        else if(strcmp(first_word, "give") == 0) {
            int id = 0, amount = 0;
            char resource[20];
            i++;
            int j = 0;
            char num[10];
            while(cmd_buffer[i] != ' ' && cmd_buffer[i] != '\0') {
                num[j] = cmd_buffer[i];
                i++;
                j++;
            }
            num[j] = '\0';
            id = strtol(num, NULL, 10);
            i++;
            j = 0;
            while(cmd_buffer[i] != ' ' && cmd_buffer[i] != '\0') {
                resource[j] = cmd_buffer[i];
                i++;
                j++;
            }
            resource[j] = '\0';
            i++;
            j = 0;
            while(cmd_buffer[i] != '\0') {
                num[j] = cmd_buffer[i];
                i++;
                j++;
            }
            num[j] = '\0';
            amount = strtol(num, NULL, 10);

            meadow->giveResources(id, resource, amount);
        }
        else if(strcmp(first_word, "tick") == 0) {
            if(colony_num < 2) {
                cout << Color::RED << "Need at least 2 colonies to start simulation!\n" << Color::RESET;
                continue;
            }

            int ticks = 1;
            if(strlen(cmd_buffer) > 5) {
                i++;
                int j = 0;
                char num[10];
                while(cmd_buffer[i] != '\0') {
                    num[j] = cmd_buffer[i];
                    i++;
                    j++;
                }
                num[j] = '\0';
                ticks = strtol(num, NULL, 10);
            }
            meadow->tick(ticks);
        }
        else if(strcmp(first_word, "summary") == 0) {
            int id = 0;
            if(strlen(cmd_buffer) > 8) {
                i++;
                int j = 0;
                char num[10];
                while(cmd_buffer[i] != '\0') {
                    num[j] = cmd_buffer[i];
                    i++;
                    j++;
                }
                num[j] = '\0';
                id = strtol(num, NULL, 10);
                meadow->displayColonySummary(id);
            }
        }
                else if(strcmp(first_word, "build") == 0) {
            int id = 0;
            char room_type[20];
            i++;
            int j = 0;
            char num[10];
            while(cmd_buffer[i] != ' ' && cmd_buffer[i] != '\0') {
                num[j] = cmd_buffer[i];
                i++;
                j++;
            }
            num[j] = '\0';
            id = strtol(num, NULL, 10);
            i++;
            j = 0;
            while(cmd_buffer[i] != '\0') {
                room_type[j] = cmd_buffer[i];
                i++;
                j++;
            }
            room_type[j] = '\0';

            cout << Color::GREEN << "Starting construction in colony " << id << "\n" << Color::RESET;
        }
        else if(strcmp(first_word, "rest") == 0) {
            cout << Color::GREEN << "Sending ants to rest \n" << Color::RESET;
        }
        else if(strcmp(first_word, "list") == 0) {
            meadow->displayAllColonies();
        }
        else if(strcmp(first_word, "species") == 0) {
            displaySpecies();
        }
        else if(strcmp(first_word, "help") == 0) {
            displayHelp();
        }
        else if(strcmp(first_word, "quit") == 0) {
            cout << Color::GREEN << "Thanks for playing!\n" << Color::RESET;
            break;
        }
        else {
            cout << Color::RED << "Unknown command. Type 'help' for available commands.\n" << Color::RESET;
        }
    }

    return 0;
}
