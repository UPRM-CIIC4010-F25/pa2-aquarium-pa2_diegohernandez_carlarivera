#include "Aquarium.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

string AquariumCreatureTypeToString(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return "BiggerFish";
        case AquariumCreatureType::NPCreature:
            return "BaseFish";
        default:
            return "UknownFish";
    }
}

// PlayerCreature Implementation
PlayerCreature::PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 10.0f, 1, sprite) {}


void PlayerCreature::setDirection(float dx, float dy) {
    m_dx = dx;
    m_dy = dy;
    normalize();
}

void PlayerCreature::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    this->bounce();
}

void PlayerCreature::reduceDamageDebounce() {
    if (m_damage_debounce > 0) {
        m_damage_debounce -= 1;
        m_x += m_dx * m_speed;
        m_y += m_dy * m_speed;
        this->bounce();
    }
}

void PlayerCreature::update() {
    if (m_growBoostFrames > 0) {
        --m_growBoostFrames;
        if (m_growBoostFrames == 0) {
            m_collisionRadius /= 1.6f;
        }
    }        
    this->reduceDamageDebounce();
    this->move();
}


void PlayerCreature::draw() const {
    
    ofLogVerbose() << "PlayerCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    if (this->m_damage_debounce > 0) {
        ofSetColor(ofColor::red); // Flash red if in damage debounce
    } else if (m_growBoostFrames > 0) {
        ofSetColor(255, 230, 120);
    } else {
        ofSetColor(ofColor::white);
    }

    if (m_sprite) m_sprite->draw(m_x, m_y);
    ofSetColor(ofColor::white);

}

void PlayerCreature::changeSpeed(int speed) {
    m_speed = speed;
}

void PlayerCreature::loseLife(int debounce) {
    if (m_damage_debounce <= 0) {
        if (m_lives > 0) this->m_lives -= 1;
        m_damage_debounce = debounce; // Set debounce frames
        ofLogNotice() << "Player lost a life! Lives remaining: " << m_lives << std::endl;
    }
    // If in debounce period, do nothing
    if (m_damage_debounce > 0) {
        ofLogVerbose() << "Player is in damage debounce period. Frames left: " << m_damage_debounce << std::endl;
    }
}

void PlayerCreature::applyGrowBoost(float scaleFactor, int frames) {
    m_growBoostFrames = frames;
    m_collisionRadius *= scaleFactor;

}
// NPCreature Implementation
NPCreature::NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 30, 1, sprite) {
    m_dx = (rand() % 3 - 1); // -1, 0, or 1
    m_dy = (rand() % 3 - 1); // -1, 0, or 1
    normalize();

    m_creatureType = AquariumCreatureType::NPCreature;
}

void NPCreature::move() {
    // Simple AI movement logic (random direction)
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }
    bounce();
}

void NPCreature::draw() const {
    ofLogVerbose() << "NPCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    ofSetColor(ofColor::white);
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
}


BiggerFish::BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();

    setCollisionRadius(60); // Bigger fish have a larger collision radius
    m_value = 5; // Bigger fish have a higher value
    m_creatureType = AquariumCreatureType::BiggerFish;
}

void BiggerFish::move() {
    // Bigger fish might move slower or have different logic
    m_x += m_dx * (m_speed * 0.5); // Moves at half speed
    m_y += m_dy * (m_speed * 0.5);
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }

    bounce();
}

void BiggerFish::draw() const {
    ofLogVerbose() << "BiggerFish at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    this->m_sprite->draw(this->m_x, this->m_y);
}

class PufferFish : public NPCreature {
public:
   PufferFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
   : NPCreature(x, y, speed, sprite) { m_value = 2; }
   void move() override {
    r += 0.04f * dir;
    if (r > 1.35f) dir = -1.0f;
    if (r < 0.75f) dir = 1.0f;
    m_x += m_dx * m_speed * 0.8f;
    m_y += m_dy * m_speed * 0.8f;
    if (m_dx < 0) m_sprite->setFlipped(true); else m_sprite->setFlipped(false);
    bounce();

   }
   private:
        float r = 1.0f, dir = 1.0f;
};

class Jellyfish : public NPCreature {
public:
    Jellyfish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) { m_value = 1; }
void move() override {
    t += 0.05f;
    m_x += std::sin(t) * 1.2f;
    m_y += std::cos(t) * 0.9f + m_dy * m_speed * 0.2f;
    bounce();
}
private:
    float t = 0.0f;
};

class Shark : public NPCreature {
public:
   Shark(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
   : NPCreature(x, y, speed, sprite) { m_value = 10; }
   void move() override {
    float px = ofGetMouseX(), py = ofGetMouseY();
    float vx = px - m_x, vy = py - m_y;
    float L = std::max(1.0f, std::sqrt(vx* vx + vy * vy));
    m_dx = vx / L; m_dy = vy / L;
    m_x += m_dx * (m_speed + 2);
    m_y += m_dy * (m_speed + 2);
    if (m_dx < 0) m_sprite->setFlipped(true); else m_sprite->setFlipped(false);
    bounce();

   }
};



// Power Up implementation
PowerUp::PowerUp(float x, float y, std::shared_ptr<GameSprite> sprite, PowerUpType type)
: Creature(x, y, 0, 0, 1, sprite), m_type(type) {}

void PowerUp::update() {
 // for movement like floating, etc
}

void PowerUp::move() {
    m_y += sin(ofGetElapsedTimef() * 2) * 0.5f;
}

void PowerUp::draw() const {
    if (m_sprite && !m_collected) {
        m_sprite->draw(m_x, m_y);
    }
}

void PowerUp::collect(std::shared_ptr<PlayerCreature> player) {
    if (!m_collected) {
        m_collected = true;
        player->applyGrowBoost(1.6f, 7 * 60);
        }
    }


bool PowerUp::shouldRemove() const {
    return m_collected;
}

void Aquarium::SpawnPowerUp() {
    int x = rand() % m_width;
    int y = rand() % m_height;
    auto sprite = std::make_shared<GameSprite>("fish-food.png", 48, 48);
    m_powerups.push_back(std::make_shared<PowerUp>(x, y, sprite, PowerUpType::Grow));
}


// AquariumSpriteManager
AquariumSpriteManager::AquariumSpriteManager(){
    this->m_npc_fish = std::make_shared<GameSprite>("base-fish.png", 70,70);
    this->m_big_fish = std::make_shared<GameSprite>("bigger-fish.png", 120, 120);
    this->m_puffer = std::make_shared<GameSprite>("pufferfish.png", 90, 90);
    this->m_jelly = std::make_shared<GameSprite>("jellyfish.png", 80, 100);
    this->m_shark = std::make_shared<GameSprite>("shark.png", 200, 120);
    this->m_food = std::make_shared<GameSprite>("fish-food.png", 48, 48);
}

std::shared_ptr<GameSprite> AquariumSpriteManager::GetSprite(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return std::make_shared<GameSprite>(*this->m_big_fish);
            
        case AquariumCreatureType::NPCreature:
            return std::make_shared<GameSprite>(*this->m_npc_fish);

            case AquariumCreatureType::PufferFish:
            return std::make_shared<GameSprite>(*this->m_puffer);
            
        case AquariumCreatureType::Jellyfish:
            return std::make_shared<GameSprite>(*this->m_jelly);

        case AquariumCreatureType::Shark:
            return std::make_shared<GameSprite>(*this->m_shark);
            
        case AquariumCreatureType::FishFoodPowerUp:
            return std::make_shared<GameSprite>(*this->m_food);

        default:
            return nullptr;
    }
}


// Aquarium Implementation
Aquarium::Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager)
    : m_width(width), m_height(height) {
        m_sprite_manager =  spriteManager;
    }



void Aquarium::addCreature(std::shared_ptr<Creature> creature) {
    creature->setBounds(m_width - 20, m_height - 20);
    m_creatures.push_back(creature);
}

void Aquarium::addAquariumLevel(std::shared_ptr<AquariumLevel> level){
    if(level == nullptr){return;} // guard to not add noise
    this->m_aquariumlevels.push_back(level);
}

void Aquarium::update() {
    for (auto& creature : m_creatures) {
        creature->move();
    }
    
    for (auto& p : m_powerups) p->update();
    
    m_powerups.erase(std::remove_if(m_powerups.begin(), m_powerups.end(),
    [](const std::shared_ptr<PowerUp>& p){ return p->shouldRemove(); }),
    m_powerups.end());


this->Repopulate();
}

void Aquarium::draw() const {
    for (const auto& creature : m_creatures) {
    creature->draw();
}
    for (const auto& p: m_powerups) {
         p->draw();
    }
}



void Aquarium::removeCreature(std::shared_ptr<Creature> creature) {
    auto it = std::find(m_creatures.begin(), m_creatures.end(), creature);
    if (it != m_creatures.end()) {
        ofLogVerbose() << "removing creature " << endl;
        int selectLvl = this->currentLevel % this->m_aquariumlevels.size();
        auto npcCreature = std::static_pointer_cast<NPCreature>(creature);
        this->m_aquariumlevels.at(selectLvl)->ConsumePopulation(npcCreature->GetType(), npcCreature->getValue());
        m_creatures.erase(it);
    }
}

void Aquarium::clearCreatures() {
    m_creatures.clear();
}

std::shared_ptr<Creature> Aquarium::getCreatureAt(int index) {
    if (index < 0 || size_t(index) >= m_creatures.size()) {
        return nullptr;
    }
    return m_creatures[index];
}



void Aquarium::SpawnCreature(AquariumCreatureType type) {
    int x = rand() % this->getWidth();
    int y = rand() % this->getHeight();
    int speed = 1 + rand() % 25; // Speed between 1 and 25

    switch (type) {
        case AquariumCreatureType::NPCreature:
            this->addCreature(std::make_shared<NPCreature>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::NPCreature)));
            break;
        case AquariumCreatureType::BiggerFish:
            this->addCreature(std::make_shared<BiggerFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::BiggerFish)));
            break;
        case AquariumCreatureType::PufferFish:
        {
            int x = rand() % this->getWidth();
            int y = rand() % this->getHeight();
            int speed = 2;
            this->addCreature(std::make_shared<PufferFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::PufferFish)));
            break;
        }
        case AquariumCreatureType::Jellyfish:
        {
            int x = rand() % this->getWidth();
            int y = rand() % this->getHeight();
            int speed = 1;
            this->addCreature(std::make_shared<Jellyfish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::Jellyfish)));
            break;
        }
        case AquariumCreatureType::Shark:
        {
            int x = rand() % this->getWidth();
            int y = rand() % this->getHeight();
            int speed = 4;
            this->addCreature(std::make_shared<Shark>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::Shark)));
            break;
        }
        case AquariumCreatureType::FishFoodPowerUp:
        {
            this->SpawnPowerUp();
            break;
        }
        default:
            ofLogError() << "Unknown creature type to spawn!";
            break;
    }

}


// repopulation will be called from the levl class
// it will compose into aquarium so eating eats frm the pool of NPCs in the lvl class
// once lvl criteria met, we move to new lvl through inner signal asking for new lvl
// which will mean incrementing the buffer and pointing to a new lvl index
void Aquarium::Repopulate() {
    ofLogVerbose("entering phase repopulation");
    // lets make the levels circular
    int selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
    ofLogVerbose() << "the current index: " << selectedLevelIdx << endl;
    std::shared_ptr<AquariumLevel> level = this->m_aquariumlevels.at(selectedLevelIdx);


    if(level->isCompleted()){
        level->levelReset();
        this->currentLevel += 1;
        selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
        ofLogNotice()<<"new level reached : " << selectedLevelIdx << std::endl;
        level = this->m_aquariumlevels.at(selectedLevelIdx);
        this->clearCreatures();
    }

    
    // now lets find how many to respawn if needed 
    std::vector<AquariumCreatureType> toRespawn = level->Repopulate();
    ofLogVerbose() << "amount to repopulate : " << toRespawn.size() << endl;
    if(toRespawn.size() <= 0 ){return;} // there is nothing for me to do here
    for(AquariumCreatureType newCreatureType : toRespawn){
        this->SpawnCreature(newCreatureType);
    }
}


// Aquarium collision detection
std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium, std::shared_ptr<PlayerCreature> player) {
    if (!aquarium || !player) return nullptr;
    
    for (int i = 0; i < aquarium->getCreatureCount(); ++i) {
        std::shared_ptr<Creature> npc = aquarium->getCreatureAt(i);
        if (npc && checkCollision(player, npc)) {
            return std::make_shared<GameEvent>(GameEventType::COLLISION, player, npc);
        }
    }
    return nullptr;
};

//  Imlementation of the AquariumScene

void AquariumGameScene::Update(){

    std::shared_ptr<GameEvent> event;

    this->m_player->update();
    static AwaitFrames powerupTimer(10 * 60);
    if (powerupTimer.tick()) {
        m_aquarium->SpawnPowerUp();
    }

    for (auto& pu : m_aquarium->m_powerups){
        pu->update();
        if (!pu->shouldRemove() && checkCollision(m_player, pu)) {
            pu->collect(m_player);
        }
    }
    m_aquarium->m_powerups.erase(
        std::remove_if(
            m_aquarium->m_powerups.begin(), 
            m_aquarium->m_powerups.end(),
                [](const std::shared_ptr<PowerUp>& p){return p->shouldRemove(); }
            ),
                m_aquarium->m_powerups.end()
    );
    
    if (this->updateControl.tick()) {
        event = DetectAquariumCollisions(this->m_aquarium, this->m_player);
        if (event != nullptr && event->isCollisionEvent()) {
            ofLogVerbose() << "Collision detected between player and NPC!" << std::endl;
            if(event->creatureB != nullptr){
                event->print();
                // update and collect powerups
                if(this->m_player->getPower() < event->creatureB->getValue()){
                    ofLogNotice() << "Player is too weak to eat the creature!" << std::endl;
                    this->m_player->loseLife(3*60); // 3 frames debounce, 3 seconds at 60fps
                    this->m_player->setDirection(-this->m_player->getDx(), -this->m_player->getDy());
                    float pushback = 5.0f;
                    this->m_player->setX(this->m_player->getX() + this->m_player->getDx() * pushback);
                    this->m_player->setY(this->m_player->getY() + this->m_player->getDy() * pushback);
                    if(this->m_player->getLives() <= 0){
                        this->m_lastEvent = std::make_shared<GameEvent>(GameEventType::GAME_OVER, this->m_player, nullptr);
                        return;
                    }
                }
                else{
                    this->m_aquarium->removeCreature(event->creatureB);
                    this->m_player->addToScore(1, event->creatureB->getValue());
                    if (this->m_player->getScore() % 25 == 0){
                        this->m_player->increasePower(1);
                        ofLogNotice() << "Player power increased to " << this->m_player->getPower() << "!" << std::endl;
                    }
                    
                }

            } else {
                ofLogError() << "Error: creatureB is null in collision event." << std::endl;
            }
        }
        this->m_aquarium->update();
    }

}

void AquariumGameScene::Draw() {
    this->m_player->draw();
    this->m_aquarium->draw();
    this->paintAquariumHUD();

}


void AquariumGameScene::paintAquariumHUD(){
    float panelWidth = ofGetWindowWidth() - 150;
    ofDrawBitmapString("Score: " + std::to_string(this->m_player->getScore()), panelWidth, 20);
    ofDrawBitmapString("Power: " + std::to_string(this->m_player->getPower()), panelWidth, 30);
    ofDrawBitmapString("Lives: " + std::to_string(this->m_player->getLives()), panelWidth, 40);
    for (int i = 0; i < this->m_player->getLives(); ++i) {
        ofSetColor(ofColor::red);
        ofDrawCircle(panelWidth + i * 20, 50, 5);
    }
    if (this->m_player->getGrowBoostFrames() > 0) {
        ofDrawBitmapString("Grow: " + std::to_string(this->m_player->getGrowBoostFrames() / 60) + "s", ofGetWindowWidth()- 150, 60);
    }
    ofSetColor(ofColor::white); // Reset color to white for other drawings
}

void AquariumLevel::populationReset(){
    for(auto node: this->m_levelPopulation){
        node->currentPopulation = 0; // need to reset the population to ensure they are made a new in the next level
    }
}

void AquariumLevel::ConsumePopulation(AquariumCreatureType creatureType, int power){
    for(std::shared_ptr<AquariumLevelPopulationNode> node: this->m_levelPopulation){
        ofLogVerbose() << "consuming from this level creatures" << endl;
        if(node->creatureType == creatureType){
            ofLogVerbose() << "-cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            if(node->currentPopulation == 0){
                return;
            } 
            node->currentPopulation -= 1;
            ofLogVerbose() << "+cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            this->m_level_score += power;
            return;
        }
    }
}

bool AquariumLevel::isCompleted(){
    return this->m_level_score >= this->m_targetScore;
}
 std::vector<AquariumCreatureType> AquariumLevel::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate; 
    for (std::shared_ptr<AquariumLevelPopulationNode> node : m_levelPopulation) {
         int delta = node->population - node->currentPopulation;
        if (delta > 0)   {
            for (int i = 0; i < delta; ++i) {
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;
        }     
  }
  return toRepopulate;
}

