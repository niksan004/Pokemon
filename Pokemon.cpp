#include <iostream>
#include <fstream>
#pragma warning(disable:4996)


const unsigned short MAX_SIZE_POKEMON_NAME = 50;
const unsigned short MAX_SIZE_FILE_NAME = 100;


typedef char PokemonName[MAX_SIZE_POKEMON_NAME];


enum class PokemonType
{
    NORMAL, 
    FIRE, 
    WATER, 
    GRASS, 
    ELECTRIC, 
    GHOST, 
    FLYING,
};


struct Pokemon
{
    PokemonType type = PokemonType::NORMAL;
    unsigned short power = 10;
    PokemonName name = "";
};

struct PokemonHandler
{
    char fileName[MAX_SIZE_FILE_NAME] = "";
};


Pokemon inputPokemonFromConsole()
{
    Pokemon pokemon;
    char tempName[MAX_SIZE_POKEMON_NAME + 2];

    std::cout << "Input pokemon name: ";
    std::cin >> tempName;
    // check whether name is too long
    if (strlen(tempName) > 50) { return {}; }
    strcpy(pokemon.name, tempName);

    std::cout << "Input pokemon type: " << std::endl
              << "0, 1, 2, 3, 4, 5, 6, 7 respectively for" << std::endl 
              << "NORMAL, FIRE, WATER, GRASS, ELECTRIC, GHOST, FLYING" << std::endl;
    int type = 0;
    std::cin >> type;
    // check whether type is in [0, 7]
    if (type < 0 || type > 7) { return {}; }
    pokemon.type = (PokemonType)type;

    std::cout << "Input polemon power: ";
    std::cin >> pokemon.power;
    // check whether power is in [10, 1000]
    if (pokemon.power < 10 || pokemon.power > 1000) { return {}; }

    return pokemon;
}

Pokemon createPokemonFromBinaryStream(std::istream& is)
{
    // assuming stream is opened correctly
    // data in binary stream: name(length 50), type(int), power(short)

    Pokemon pokemon;
    is.read(pokemon.name, MAX_SIZE_POKEMON_NAME);
    
    int type = 0;
    is.read((char*)&type, sizeof(type));
    pokemon.type = (PokemonType)type;

    is.read((char*)&pokemon.power, sizeof(pokemon.power));

    return pokemon;
}

void savePokemonInBinaryStream(std::ostream& os, const Pokemon& pokemon)
{
    // assuming stream is opened correctly
    // data in binary stream will be: name(length 50), type(int), power(short)

    os.write((const char*)&pokemon.name, MAX_SIZE_POKEMON_NAME);

    os.write((const char*)&pokemon.type, sizeof(pokemon.type));

    os.write((const char*)&pokemon.power, sizeof(pokemon.power));
}

int size(const PokemonHandler& ph)
{
    std::ifstream ifs(ph.fileName);

    if (!ifs.is_open()) { return 0; }

    ifs.seekg(0, std::ios::end);
    int size = ifs.tellg();

    ifs.close();

    return size / sizeof(Pokemon);
}

Pokemon at(const PokemonHandler& ph, int i)
{
    int sizeOfColl = size(ph);

    // check index
    if (i < 0 || i >= sizeOfColl) { return {}; }

    std::ifstream ifs(ph.fileName, std::ios::binary);

    // check stream
    if (!ifs.is_open()) { return {}; }

    ifs.seekg(i * sizeof(Pokemon));
    Pokemon pokemon = createPokemonFromBinaryStream(ifs);

    ifs.close();

    return pokemon;
}

void swap(const PokemonHandler& ph, int i, int j)
{
    int sizeOfColl = size(ph);

    // check indices
    if (i < 0 || i >= sizeOfColl || j < 0 || j >= sizeOfColl) { return; }

    std::fstream fs(ph.fileName, std::ios::binary |
                                  std::ios::in |
                                  std::ios::out);

    // check stream
    if (!fs.is_open()) { return; }

    // swap pokemons
    Pokemon pokI = at(ph, i);
    Pokemon pokJ = at(ph, j);

    fs.seekg(i * sizeof(Pokemon));
    savePokemonInBinaryStream(fs, pokJ);

    fs.seekg(j * sizeof(Pokemon));
    savePokemonInBinaryStream(fs, pokI);

    fs.close();
}

void selectionSortPokemons(const PokemonHandler& ph)
{
    int sizeOfCollection = size(ph);

    for (int i = 0; i < sizeOfCollection; i++)
    {
        Pokemon highestPow = at(ph, i);
        int highestPowIdx = i;

        for (int j = i + 1; j < sizeOfCollection; j++)
        {
            Pokemon currJ = at(ph, j);
            if (currJ.power > highestPow.power)
            { 
                highestPow = currJ;
                highestPowIdx = j;
            }
        }

        Pokemon currI = at(ph, i);
        if (highestPow.power > currI.power) { swap(ph, i, highestPowIdx); }
    }
}

PokemonHandler newPokemonHandler(const char* fileName)
{
    PokemonHandler ph;
    strcpy(ph.fileName, fileName);
    selectionSortPokemons(ph);
    return ph;
}

void insert(const PokemonHandler& ph, const Pokemon& pokemon)
{
    int sizeOfCollection = size(ph);

    if (sizeOfCollection == 0)
    {
        std::ofstream ofs(ph.fileName, std::ios::binary | std::ios::app);

        // check file
        if (!ofs.is_open()) { return; }

        savePokemonInBinaryStream(ofs, pokemon);

        ofs.close();

        return;
    }

    if (sizeOfCollection == 1)
    {
        Pokemon curr = at(ph, 0);
        if (curr.power >= pokemon.power) 
        { 
            std::ofstream ofs(ph.fileName, std::ios::binary | std::ios::in | std::ios::ate);

            // check file
            if (!ofs.is_open()) { return; }

            savePokemonInBinaryStream(ofs, pokemon);

            ofs.close();

            return;
        }
        else
        {
            Pokemon toDisplace = at(ph, 0);

            std::ofstream ofs(ph.fileName, std::ios::binary | std::ios::in);

            // check file
            if (!ofs.is_open()) { return; }

            ofs.seekp(std::ios::beg);
            savePokemonInBinaryStream(ofs, pokemon);
            savePokemonInBinaryStream(ofs, toDisplace);

            ofs.close();

            return;
        }
    }
    
    if (sizeOfCollection > 1)
    {
        Pokemon first = at(ph, 0);
        if (pokemon.power >= first.power)
        {
            std::fstream fs(ph.fileName, std::ios::in | std::ios::out | std::ios::binary);

            // check file
            if (!fs.is_open()) { return; }

            char* pokemonsToDisplace = new char[sizeof(Pokemon) * (sizeOfCollection)];
            fs.seekg(std::ios::beg);
            fs.read(pokemonsToDisplace, sizeof(Pokemon) * (sizeOfCollection));

            fs.seekp(std::ios::beg);
            savePokemonInBinaryStream(fs, pokemon);

            fs.write(pokemonsToDisplace, sizeof(Pokemon) * (sizeOfCollection));

            fs.close();

            return;
        }

        Pokemon last = at(ph, sizeOfCollection - 1);
        if (pokemon.power <= last.power)
        {
            std::ofstream ofs(ph.fileName, std::ios::binary | std::ios::app);

            // check file
            if (!ofs.is_open()) { return; }

            savePokemonInBinaryStream(ofs, pokemon); 
            
            ofs.close();

            return;
        }

        for (int i = 1; i < sizeOfCollection; i++)
        {
            Pokemon below = at(ph, i - 1);
            Pokemon curr = at(ph, i);

            if (below.power >= pokemon.power && curr.power <= pokemon.power)
            {
                std::fstream fs(ph.fileName, std::ios::in | std::ios::out | std::ios::binary);

                // check file
                if (!fs.is_open()) { return; }

                char* pokemonsToDisplace = new char[sizeof(Pokemon) * (sizeOfCollection - i)];
                fs.seekg(i * sizeof(Pokemon));
                fs.read(pokemonsToDisplace, sizeof(Pokemon) * (sizeOfCollection - i));

                fs.seekp(i * sizeof(Pokemon));
                savePokemonInBinaryStream(fs, pokemon);

                fs.write(pokemonsToDisplace, sizeof(Pokemon) * (sizeOfCollection - i));

                fs.close();

                return;
            }
        }
    }
}

void insertShorter(const PokemonHandler& ph, const Pokemon& pokemon)
{
    std::ofstream ofs(ph.fileName, std::ios::binary | std::ios::app);

    // check file
    if (!ofs.is_open()) { return; }

    savePokemonInBinaryStream(ofs, pokemon);

    ofs.close();

    selectionSortPokemons(ph);
}

void textify(const PokemonHandler& ph, const char* fileName)
{
    int sizeOfCollection = size(ph);

    std::ofstream ofs(fileName);

    // check file
    if (!ofs.is_open()) { return; }

    for (int i = 0; i < sizeOfCollection; i++)
    {
        Pokemon pokemon = at(ph, i);

        ofs << pokemon.name << " " << (int)pokemon.type << " " << pokemon.power << '\n';
    }

    ofs.close();
}

void untextify(const PokemonHandler& ph, const char* fileName)
{
    std::ifstream ifs(fileName);

    // check file
    if (!ifs.is_open()) { return; }

    while (true)
    {
        Pokemon pokemon;
        int type = 0;

        ifs >> pokemon.name;

        ifs >> type;

        pokemon.type = (PokemonType)type;

        ifs >> pokemon.power;

        insert(ph, pokemon);

        if (ifs.eof()) { break; }
    }

    ifs.close();

    return;
}

void printFile(const PokemonHandler& ph)
{
    int sizeOfCollection = size(ph);

    for (int i = 0; i < sizeOfCollection; i++)
    {
        Pokemon curr = at(ph, i);
        std::cout << curr.name << " - " << (int)curr.type << " - " << curr.power << std::endl;
    }
}


int main()
{
    // test pokemon handler
    PokemonHandler ph = newPokemonHandler("test");
    std::cout << "Input 3 pokemon:" << std::endl;
    for (int i = 0; i < 3; i++)
    {
        Pokemon p = inputPokemonFromConsole();
        insert(ph, p);
    }

    // test textify
    textify(ph, "textify.txt");
    
    // test untextify
    PokemonHandler phUntextify = newPokemonHandler("untextified");
    untextify(phUntextify, "textified.txt");
    std::cout << "Result from untextifying: " << std::endl;
    printFile(phUntextify);
}
