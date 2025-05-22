// SFML VERSION 3.0

#include <SFML/Graphics.hpp>

#include <iostream>
#include <time.h>
#include <optional>
#include <cstdlib>
#include <stdlib.h>
#include <vector>
#include <list>
#include <algorithm>
using namespace std;

// Constantes pour définir les dimensions du jeu
const int TILE_SIZE = 30;                       // Taille d'une tuile en pixels
const int GRID_SIZE = 25;                       // Nombre de tuiles dans la grille (25x25)
const int WINDOW_SIZE = GRID_SIZE * TILE_SIZE;  // Dimension totale de la zone de jeu
const int MARGIN = 50;                          // Marge autour de la zone de jeu

// Couleurs utilisées dans le jeu
const sf::Color FOOD_COLOR = sf::Color(255, 99, 71, 255);        // Tomate pour la nourriture
const sf::Color SNAKE_COLOR = sf::Color(50, 205, 50, 255);       // Vert lime pour le serpent
const sf::Color BACKGROUND_COLOR = sf::Color(47, 79, 79, 255);   // Gris ardoise foncé
const sf::Color OBSTACLES_COLOR = sf::Color(105, 105, 105, 255); // Gris foncé pour les obstacles


// Variables globales pour le score et l'état du jeu
int currentScore = 0;          // Score actuel du joueur
vector<int> highScores;        // Liste des meilleurs scores

bool gameStarted = false;      // Indique si le jeu a commencé
bool isGameOver = false;       // Indique si le jeu est terminé

/*
  Vérifie si un élément est présent dans une liste
  parametre "element" L'élément à rechercher
  parametre "targetList" La liste dans laquelle chercher
  retourne true si l'élément est dans la liste, false sinon
*/
bool isElementInList(vector<int>& element, list<vector<int>>& targetList) {
    list<vector<int>>::iterator it;
    for (it = targetList.begin(); it != targetList.end(); ++it)
    {
        if (element[0] == (*it)[0] && element[1] == (*it)[1])
        {
            return true;
        }
    }
    return false;
}

/*
  Fonction de comparaison utilisée pour trier les scores par ordre décroissant
  parametre "a" Premier score
  parametre "b" Deuxième score
  retourne true si a > b
*/
bool compareDescending(int a, int b) {
    return a > b;
}

/*
  Classe représentant le serpent
*/
class Snake {
public:
    list<vector<int>> body = { {6,10},{5,10},{4,10} };  // Corps du serpent (liste de coordonnées [x,y])
    vector<int> moveDirection = { 1,0 };                // Direction du mouvement (initialement vers la droite)
    bool shouldGrow = false;                            // Indique si le serpent doit grandir au prochain mouvement
    sf::Texture snakeTexture;                           // Texture pour l'affichage du serpent
    bool snakeTextureLoaded = false;                    // Indique si la texture a été chargée
    /*
      Constructeur du serpent
    */
    Snake() {
        if (!snakeTexture.loadFromFile("./Sprites/snake/corps.png")) {
            snakeTextureLoaded = false;
        }
        else {
            snakeTextureLoaded = true;
        }
    }

    /*
      Dessine le serpent sur la fenêtre
      parametre "window" La fenêtre où dessiner
    */
    void Draw(sf::RenderWindow& window) {
        list<vector<int>>::iterator it;
        for (it = body.begin(); it != body.end(); ++it)
        {
            int x = (*it)[0];
            int y = (*it)[1];
            sf::RectangleShape segment(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            if (snakeTextureLoaded) {
                segment.setTexture(&snakeTexture);
            } 
            else {
                segment.setFillColor(SNAKE_COLOR);
            }
            segment.setPosition(sf::Vector2f(MARGIN + x * TILE_SIZE, MARGIN + y * TILE_SIZE));
            window.draw(segment);
        }
    }

    /*
      Ajuste la position pour permettre au serpent de traverser les bords de l'écran
      parametre "position" La position à ajuster
    */
    void WrapPosition(vector<int>& position) {
        if (position[0] < 0) { position[0] = GRID_SIZE - 1; }
        else if (position[0] >= GRID_SIZE) { position[0] = 0; }

        if (position[1] < 0) { position[1] = GRID_SIZE - 1; }
        else if (position[1] >= GRID_SIZE) { position[1] = 0; }
    }

    /*
      Affiche le score actuel sur l'écran
      parametre "window" La fenêtre où afficher le score
    */
    void DisplayScore(sf::RenderWindow& window) {
        sf::Font font;
        if (!font.openFromFile("./Font/Heavitas.ttf")) {
            std::cout << "Error loading font!" << std::endl;
        }

        sf::Text scoreText(font);

        scoreText.setString("Score : " + to_string(currentScore));
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::Black);
        scoreText.setPosition({ TILE_SIZE * 21, 5 });

        window.draw(scoreText);
    }

    /*
      Déplace le serpent dans la direction actuelle
    */
    void Move() {
        if (gameStarted)
        {
            vector<int> headPosition = { body.front()[0] + moveDirection[0], body.front()[1] + moveDirection[1] };

            WrapPosition(headPosition);

            body.push_front(headPosition);

            if (shouldGrow == true)
            {
                shouldGrow = false;
            }
            else {
                body.pop_back();
            }
        }
    }

    /*
      Réinitialise le serpent à sa position et taille initiales
    */
    void Reset() {
        body = { {6,10},{5,10},{4,10} };
        moveDirection = { 1,0 };
    }
};

/*
  Classe représentant la nourriture pour le serpent
*/

class Fruit {
public:
    vector<int> position;      // Position de la nourriture [x,y]
    sf::Texture fruitTexture;  // Texture pour l'affichage de la nourriture
    bool fruitTextureLoaded = false;  // Indique si la texture a été chargée

    /*
      Constructeur de la nourriture
      parametre "snakeBody" Corps du serpent pour éviter de placer la nourriture sur le serpent
    */
    Fruit(list<vector<int>>& snakeBody) {
        if (!fruitTexture.loadFromFile("./sprites/fruites/Strawberry.png")) {
            fruitTextureLoaded = false;
        }
        else {
            fruitTextureLoaded = true;
        }
        position = GenerateRandomPosition(snakeBody);
    }

    /*
      Dessine la nourriture sur la fenêtre
      parametre "window" La fenêtre où dessiner
    */
    void Draw(sf::RenderWindow& window) {
        sf::RectangleShape fruitShape{ sf::Vector2f(TILE_SIZE, TILE_SIZE) };
        if (fruitTextureLoaded) {
            fruitShape.setTexture(&fruitTexture);
        }
        else {
            fruitShape.setFillColor(FOOD_COLOR);
        }
        fruitShape.setPosition(sf::Vector2f(MARGIN + position[0] * TILE_SIZE, MARGIN + position[1] * TILE_SIZE));
        window.draw(fruitShape);
    }

    /*
      Génère une position aléatoire sur la grille
      retourne Coordonnées [x,y] d'une position aléatoire
    */
    vector<int> GetRandomTile() {
        int x = rand() % GRID_SIZE;
        int y = rand() % GRID_SIZE;
        return { x, y };
    }

    /*
      Génère une position aléatoire qui n'est pas sur le corps du serpent
      parametre "snakeBody" Corps du serpent à éviter
      retourne Position aléatoire valide [x,y]
    */
    vector<int> GenerateRandomPosition(list<vector<int>>& snakeBody) {
        vector<int> newPosition = GetRandomTile();
        while (isElementInList(newPosition, snakeBody))
        {
            newPosition = GetRandomTile();
        }
        return newPosition;
    }
};

/*
  Classe représentant les obstacles dans le jeu
*/
class Obstacles {
public:
    vector<int> position1;                 // Position du premier obstacle [x,y]
    vector<int> position2;                 // Position du deuxième obstacle [x,y]
    sf::Texture obstacleTexture;           // Texture pour l'affichage des obstacles
    bool obstacleTextureLoaded = false;   // Indique si la texture a été chargée

    /*
      Constructeur des obstacles
      parametre "snakeBody" Corps du serpent pour éviter de placer des obstacles sur le serpent
      parametre "fruitPosition" Position de la nourriture pour éviter de placer des obstacles sur la nourriture
    */
    Obstacles(list<vector<int>>& snakeBody, vector<int> fruitPosition) {
        if (!obstacleTexture.loadFromFile("./sprites/obstacle/obstacle.png")) {
            obstacleTextureLoaded = false;
        }
        else {
            obstacleTextureLoaded = true;
        }
        position1 = GenerateRandomPosition(snakeBody, fruitPosition);
        position2 = GenerateRandomPosition(snakeBody, fruitPosition);
    }

    /*
      Dessine les obstacles sur la fenêtre
      parametre "window" La fenêtre où dessiner
    */
    void Draw(sf::RenderWindow& window) {
        sf::RectangleShape obstacleShape{ sf::Vector2f(TILE_SIZE, TILE_SIZE) };
        if (obstacleTextureLoaded) {
            obstacleShape.setTexture(&obstacleTexture);
        }
        else {
            obstacleShape.setFillColor(OBSTACLES_COLOR);
        }
        obstacleShape.setPosition(sf::Vector2f(MARGIN + position1[0] * TILE_SIZE, MARGIN + position1[1] * TILE_SIZE));
        window.draw(obstacleShape);

        obstacleShape.setTexture(&obstacleTexture);
        obstacleShape.setPosition(sf::Vector2f(MARGIN + position2[0] * TILE_SIZE, MARGIN + position2[1] * TILE_SIZE));
        window.draw(obstacleShape);
    }

    /*
      Génère une position aléatoire sur la grille
      retourne Coordonnées [x,y] d'une position aléatoire
    */
    vector<int> GetRandomTile() {
        int x = rand() % GRID_SIZE;
        int y = rand() % GRID_SIZE;
        return { x, y };
    }

    /*
      Génère une position aléatoire qui n'est pas sur le corps du serpent ni sur la nourriture
      parametre "snakeBody" Corps du serpent à éviter
      parametre "fruitPosition" Position de la nourriture à éviter
      retourne Position aléatoire valide [x,y]
    */
    vector<int> GenerateRandomPosition(list<vector<int>> snakeBody, vector<int> fruitPosition) {
        vector<int> newPosition = GetRandomTile();
        while (isElementInList(newPosition, snakeBody) || (newPosition[0] == fruitPosition[0] && newPosition[1] == fruitPosition[1]))
        {
            newPosition = GetRandomTile();
        }
        return newPosition;
    }
};

/*
  Classe principale du jeu qui gère toutes les interactions
*/
class Game {
public:
    Snake snake = Snake();                           // Instance du serpent
    Fruit fruit = Fruit(snake.body);                 // Instance de la nourriture
    Obstacles obstacles = Obstacles(snake.body, fruit.position);  // Instance des obstacles

    /*
      Dessine tous les éléments du jeu
      parametre "window" La fenêtre où dessiner
    */
    void Draw(sf::RenderWindow& window) {
        fruit.Draw(window);
        obstacles.Draw(window);
        snake.Draw(window);
    }

    /*
      Vérifie si le serpent a mangé la nourriture
    */
    void CheckFoodCollision() {
        if (snake.body.front()[0] == fruit.position[0] && snake.body.front()[1] == fruit.position[1])
        {
            fruit.position = fruit.GenerateRandomPosition(snake.body);
            obstacles.position1 = obstacles.GenerateRandomPosition(snake.body, fruit.position);
            obstacles.position2 = obstacles.GenerateRandomPosition(snake.body, fruit.position);
            snake.shouldGrow = true;
            currentScore++;
        }
    }

    /*
      Met à jour la liste des meilleurs scores
      parametre "scoreList" Liste des scores à mettre à jour
      parametre "newScore" Nouveau score à ajouter
    */
    void UpdateHighScores(vector<int>& scoreList, int newScore) {
        if (find(scoreList.begin(), scoreList.end(), newScore) == scoreList.end()) {
            scoreList.push_back(newScore);

            sort(scoreList.begin(), scoreList.end(), compareDescending);
            if (scoreList.size() > 5) {
                scoreList.pop_back();
            }
        }
    }

    /*
      Affiche les meilleurs scores sur l'écran de fin de partie
      parametre "window" La fenêtre où afficher les scores
      parametre "scoreList" Liste des meilleurs scores
    */
    void DisplayTopScores(sf::RenderWindow& window, vector<int>& scoreList) {
        sf::Font font;
        if (!font.openFromFile("./Font/Heavitas.ttf")) {
            std::cout << "Error loading font!" << std::endl;
        }
        if (isGameOver) {
            sf::RectangleShape gameOverScreen(sf::Vector2f(WINDOW_SIZE, WINDOW_SIZE));
            gameOverScreen.setFillColor(sf::Color(0, 0, 0, 150));
            gameOverScreen.setPosition(sf::Vector2f(MARGIN, MARGIN));
            window.draw(gameOverScreen);

            sf::Text gameOverText(font);
            gameOverText.setString("Game Over");
            gameOverText.setCharacterSize(48);
            gameOverText.setFillColor(sf::Color::White);
            gameOverText.setPosition({ WINDOW_SIZE / 2 + MARGIN, WINDOW_SIZE / 2 + MARGIN });
            window.draw(gameOverText);

            sf::Text restartText(font);
            restartText.setString("Press SPACE to restart");
            restartText.setCharacterSize(24);
            restartText.setFillColor(sf::Color::White);
            restartText.setPosition({ WINDOW_SIZE / 2 + MARGIN, WINDOW_SIZE / 2 + MARGIN + 100 });
            window.draw(restartText);

            sf::Text highScoreTitle(font);
            highScoreTitle.setString("Top Scores:");
            highScoreTitle.setCharacterSize(24);
            highScoreTitle.setFillColor(sf::Color::White);
            highScoreTitle.setPosition({ WINDOW_SIZE / 2 + MARGIN - 100, MARGIN + 50 });
            window.draw(highScoreTitle);

            for (int i = 0; i < scoreList.size() && i < 5; i++) {
                sf::Text scoreText(font);
                scoreText.setString(to_string(scoreList[i]));
                scoreText.setCharacterSize(24);
                scoreText.setFillColor(sf::Color::White);
                scoreText.setPosition(sf::Vector2f(WINDOW_SIZE / 2 + MARGIN - 50, MARGIN + 100 + i * 30));
                window.draw(scoreText);
            }
        }
    }

    /*
      Marque le jeu comme terminé et enregistre le score
      parametre "window" La fenêtre du jeu
    */
    void SetGameOver(sf::RenderWindow& window) {
        if (!isGameOver) {
            isGameOver = true;
            UpdateHighScores(highScores, currentScore);
        }
    }

    /*
      Redémarre le jeu lorsque la touche espace est pressée à la fin d'une partie
    */
    void RestartGame() {
        if (isGameOver && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
        {
            snake.Reset();
            fruit.position = fruit.GenerateRandomPosition(snake.body);
            obstacles.position1 = obstacles.GenerateRandomPosition(snake.body, fruit.position);
            obstacles.position2 = obstacles.GenerateRandomPosition(snake.body, fruit.position);
            gameStarted = false;
            currentScore = 0;
            isGameOver = false;
        }
    }

    /*
      Vérifie si le serpent est entré en collision avec lui-même
      parametre "window" La fenêtre du jeu
    */
    void CheckSelfCollision(sf::RenderWindow& window) {
        if (!isGameOver) {
            list<vector<int>> bodyWithoutHead = snake.body;
            bodyWithoutHead.pop_front();
            if (isElementInList(snake.body.front(), bodyWithoutHead)) {
                SetGameOver(window);
            }
        }
    }
};

/*
  Fonction principale du programme
  retourne Code de sortie
*/
int main()
{
    srand(time(NULL));  // Initialisation du générateur de nombres aléatoires

    cout << "***** Game started *****" << endl;

    sf::Clock gameClock;                          // Horloge pour mesurer le temps
    sf::Time moveInterval = sf::seconds(0.2f);    // Intervalle entre les mouvements du serpent
    sf::Time timeSinceLastMove;                   // Temps écoulé depuis le dernier mouvement

    // Création de la fenêtre du jeu
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(2 * MARGIN + WINDOW_SIZE, 2 * MARGIN + WINDOW_SIZE)), "Snake Game");
    window.setFramerateLimit(60);  // Limite la cadence à 60 FPS

    // Chargement et configuration de la texture d'arrière-plan
    sf::Texture backgroundTexture;
    bool backgroundTextureLoaded = backgroundTexture.loadFromFile("./sprites/background/Grass.png");
    if (backgroundTextureLoaded) {
        backgroundTexture.setRepeated(true);
    }

    // Création du fond avec la texture 
    sf::RectangleShape background(sf::Vector2f(WINDOW_SIZE, WINDOW_SIZE));
    if (backgroundTextureLoaded) {
        background.setTexture(&backgroundTexture);
        background.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(WINDOW_SIZE, WINDOW_SIZE)));
    }
    else {
        background.setFillColor(BACKGROUND_COLOR);
    }
    background.setPosition(sf::Vector2f(MARGIN, MARGIN));


    // Création de l'instance du jeu
    Game game = Game();

    // Boucle principale du jeu
    while (window.isOpen())
    {
        // Gestion des événements
        while (const optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Effacement de l'écran et dessin du fond
        window.clear(sf::Color(BACKGROUND_COLOR));
        window.draw(background);

        // Dessin de la bordure du terrain de jeu
        sf::RectangleShape borderOutline(sf::Vector2f(WINDOW_SIZE, WINDOW_SIZE));
        borderOutline.setFillColor(sf::Color::Transparent);
        borderOutline.setPosition(sf::Vector2f(MARGIN, MARGIN));
        borderOutline.setOutlineThickness(5);
        borderOutline.setOutlineColor(sf::Color(34, 34, 34, 255));
        window.draw(borderOutline);

        // Dessin des éléments du jeu
        game.Draw(window);

        // Chargement de la police et affichage du titre
        sf::Font font;
        if (!font.openFromFile("./Font/Heavitas.ttf")) {
            std::cout << "Error loading font!" << std::endl;
        }

        sf::Text titleText(font);
        titleText.setString("Snake Game");
        titleText.setCharacterSize(24);
        titleText.setFillColor(sf::Color::Black);
        titleText.setPosition({ 5, 5 });
        window.draw(titleText);

        // Affichage du score actuel
        game.snake.DisplayScore(window);

        // Affichage des meilleurs scores si le jeu est terminé
        game.DisplayTopScores(window, highScores);

        // Mise à jour de la position du serpent à intervalle régulier
        timeSinceLastMove += gameClock.restart();
        if (timeSinceLastMove >= moveInterval) {
            if (!isGameOver) {
                game.snake.Move();
                game.CheckFoodCollision();
                game.CheckSelfCollision(window);
            }
            timeSinceLastMove -= moveInterval;
        }

        // Gestion du redémarrage du jeu
        game.RestartGame();

        // Gestion des entrées clavier pour diriger le serpent
        if (!isGameOver)
        {
            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) && game.snake.moveDirection[0] != 1) {
                game.snake.moveDirection = { -1, 0 };
                gameStarted = true;
            }

            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) && game.snake.moveDirection[0] != -1) {
                game.snake.moveDirection = { 1, 0 };
                gameStarted = true;
            }
            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) && game.snake.moveDirection[1] != 1) {
                game.snake.moveDirection = { 0, -1 };
                gameStarted = true;
            }
            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) && game.snake.moveDirection[1] != -1) {
                game.snake.moveDirection = { 0, 1 };
                gameStarted = true;
            }
        }

        // Vérification de la collision avec les obstacles
        if (!isGameOver &&
            (game.snake.body.front() == game.obstacles.position1 ||
                game.snake.body.front() == game.obstacles.position2)) {
            game.SetGameOver(window);
        }

        // Affichage de tous les éléments dessinés
        window.display();
    }
}