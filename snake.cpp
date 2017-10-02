#include <stdio.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <deque>

#include  <random>
#include  <iterator>


template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

#define MAX_COST 9001
#define BOARD_X 20
#define BOARD_Y 20

char board[BOARD_X][BOARD_Y];


struct position {
    int x;
    int y;

};

bool operator==(const position& l, const position& r) {
        return l.x == r.x && l.y == r.y;
}
bool operator<(const position& l, const position& r) {
     return (l.x<r.x || (l.x==r.x && l.y<r.y));
}

struct snakeFrag {
    position pos;
    char bodyInd;
};

struct appleData {
    position pos;
    int distanceToMe;
    int distanceToEnemy;
    int id;
    double priority;
};

void input();
void fillLists(std::deque<snakeFrag>& mySnake, std::deque<snakeFrag>& enemySnake, std::vector<position>& apples, position& myHead, position& enemyHead);
int distanceToPoint(position head, position apple);
int heuristCost(position point1, position point2);
std::vector<position> aStar(position startPoint, position endPoint, bool heu = true);
position lowestScore(std::map<position, int> score, std::vector<position> openSet);
std::vector<position> getValidNeighbors(position current);
std::vector<position> getPath(std::map<position, position> cameFrom, position current);
std::vector<int> prioritizeApplesIDs(std::vector<appleData> applesData);
double priority(position center, double dist, std::vector<appleData> meHDistToApples, int appleID);
bool appPrioritySort(appleData f, appleData s);
int applesInRangeToMe(std::vector<appleData> apples, int distance = 20);
position endGameTarget();

int main() {
    std::deque<snakeFrag> mySnake;
    std::deque<snakeFrag> enemySnake;
    std::vector<position> apples;

    position myHead;
    position enemyHead;

    input();
    fillLists(mySnake, enemySnake, apples, myHead, enemyHead);

    std::vector <appleData> applesData;

    int id = 0;
    appleData temp;
    for (auto& app: apples) {
        temp.pos                = app;
        temp.id                 = id;
        temp.distanceToMe       = distanceToPoint(myHead, app);
        temp.distanceToEnemy    = distanceToPoint(enemyHead, app);
        applesData.push_back(temp);
        ++id;
    }

    std::vector<int> targetAppleID;
    targetAppleID = prioritizeApplesIDs(applesData);
    std::vector<position> path;
    if (mySnake.size() > 13) {
        path = aStar(myHead, endGameTarget(), false);
    } else {
        path = aStar(myHead, applesData[targetAppleID[0]].pos);
    }
    position next;
    std::vector<position> neighbors = getValidNeighbors(myHead);

    if(path.size()>1) {
        next = path[path.size()-2];
    } else if (neighbors.empty()) {
        puts("Shit\n");
        return 0;
    } else {
        next = *select_randomly(neighbors.begin(), neighbors.end());
    }

    if(next.x > myHead.x) {
        puts("Down\n");
    } else if (next.x < myHead.x){
        puts("Up\n");
    } else if(next.y > myHead.y) {
        puts("Right\n");
    } else if (next.y < myHead.y){
        puts("Left\n");
    }

    return 0;
}

void input() {
    for (auto i = 0u ; i < 20 ; ++i) {
        scanf("%s19", board[i]);
    }
}

void fillLists(std::deque<snakeFrag>& mySnake, std::deque<snakeFrag>& enemySnake, std::vector<position>& apples, position& myHead, position& enemyHead) {
    position temp;
    snakeFrag tSnake;
    for (auto i = 0u ; i < 20 ; ++i) {
        for (auto k = 0u ; k < 20; ++k) {
            temp.x = i;
            temp.y = k;

            if(board[i][k] == '@') {
                apples.push_back(temp);
            } else if (board[i][k] >= 'A' && board[i][k] <= 'Z') {
                tSnake.pos = temp;
                tSnake.bodyInd = board[i][k];
                mySnake.push_back(tSnake);
                if (board[i][k] == 'A')
                    myHead = temp;
            } else if (board[i][k] >= 'a' && board[i][k] <= 'z') {
                tSnake.pos = temp;
                tSnake.bodyInd = board[i][k];
                enemySnake.push_back(tSnake);
                if (board[i][k] == 'a')
                    enemyHead = temp;
            }
        }
    }

}

int heuristCost(position point1, position point2) {
    int x = pow(abs(point1.x - point2.x),2);
    int y = pow(abs(point1.y - point2.y),2);
    int mod;

    if (board[point1.x][point1.y] == '@') {
        mod = -2;
    }
    if (point1.x == 0 || point1.x == 19) {
        mod += 1;
    }
    if (point1.y == 0 || point1.y == 19) {
        mod += 1;
    }
    return (x + y + mod);
}

int distanceToPoint(position point1, position point2) {
    int x = abs(point1.x - point2.x);
    int y = abs(point1.y - point2.y);

    return (x + y);
}

std::vector<position> aStar(position startPoint, position endPoint, bool heu) {
    std::vector<position> closedSet;
    std::vector<position> openSet;
    openSet.push_back(startPoint);

    std::map<position, position> cameFrom;

    std::map<position, int> gScore;
    std::map<position, int> fScore;

    position temp;
    int tentative_gScore;
    std::vector<position> neighbors;
    for (auto i = 0u ; i < 20 ; ++i) {
        for (auto k = 0u ; k < 20 ; ++k) {
            temp.x = i;
            temp.y = k;

            gScore[temp] = MAX_COST;
            fScore[temp] = MAX_COST;
        }
    }
    gScore[startPoint] = 0;
    fScore[startPoint] = heuristCost(startPoint, endPoint);
    position current;
    while (openSet.size()) {
        neighbors.clear();
        current = lowestScore(fScore, openSet);

        if (current == endPoint) {
            return getPath(cameFrom, current);
        }
        openSet.erase(std::remove(openSet.begin(), openSet.end(), current), openSet.end());
        closedSet.push_back(current);
        neighbors = getValidNeighbors(current);
        for (auto n: neighbors) {
            if(std::find(closedSet.begin(), closedSet.end(), n) != closedSet.end()) {
                continue;
            }
            if(std::find(openSet.begin(), openSet.end(), n) == openSet.end()) {
                openSet.push_back(n);
            }
            tentative_gScore = gScore[current] + distanceToPoint(current, n);
            if (tentative_gScore >= gScore[n]) {
                continue;
            }

            cameFrom[n] = current;
            gScore[n]   = tentative_gScore;
            if (heu) {
                fScore[n]   = gScore[n] + heuristCost(n, endPoint);
            } else {
                fScore[n] = gScore[n];
            }
        }
    }
    std::vector<position> e;
    return e;
}

position lowestScore(std::map<position, int> score, std::vector<position> openSet) {
    position lowest;
    int lScore = MAX_COST;

    position temp;
    for (auto& n: openSet) {
        if (score[n] < lScore) {
            lScore = score[n];
            lowest = n;
        }
    }

    return lowest;
}

std::vector<position> getValidNeighbors(position current) {
    std::vector<position> neighbors;
    position temp;
    if (current.x+1 < 20 && (board[current.x+1][current.y] == '.' || board[current.x+1][current.y] == '@' )) {
        temp.x = (current.x + 1);
        temp.y = current.y;
        neighbors.push_back(temp);
    }
    if (current.x-1 >= 0 && (board[current.x-1][current.y] == '.' || board[current.x-1][current.y] == '@' )) {
        temp.x = (current.x - 1);
        temp.y = current.y;
        neighbors.push_back(temp);
    }

    if (current.y+1 < 20 && (board[current.x][current.y+1] == '.' || board[current.x][current.y+1] == '@' )) {
        temp.x = current.x;
        temp.y = (current.y + 1);
        neighbors.push_back(temp);
    }
    if (current.y-1 >= 0 && (board[current.x][current.y-1] == '.' || board[current.x][current.y-1] == '@' )) {
        temp.x = current.x;
        temp.y = (current.y - 1);
        neighbors.push_back(temp);
    }

    return neighbors;
}

std::vector<position> getPath(std::map<position, position> cameFrom, position current) {
    std::vector<position> keys;
    std::vector<position> path;
    path.push_back(current);

    for (auto& c: cameFrom) {
        keys.push_back(c.first);
    }

    while (std::find(keys.begin(), keys.end(), current) != keys.end()) {
        current = cameFrom[current];
        path.push_back(current);
    }

    return path;
}

std::vector<int> prioritizeApplesIDs(std::vector<appleData> applesData) {
    std::vector<int> IDs;
    std::vector<appleData> apples;

    position appleCenter;
    appleCenter.x = 0;
    appleCenter.y = 0;

    std::vector<double> distances;

    double tempDist;
    for (auto i = 0u ; i < applesData.size() ; ++i) {
        tempDist = 0;
        for (auto j = 0u ; j < applesData.size() ; ++j) {
            if (j == i)
                continue;
            tempDist += distanceToPoint(applesData[i].pos, applesData[j].pos)/(applesData.size()*1.0);
        }
        distances.push_back(tempDist);
    }

    for (auto& a: applesData) {
        appleCenter.x += a.pos.x;
        appleCenter.y += a.pos.y;
    }

    appleCenter.x /= applesData.size();
    appleCenter.y /= applesData.size();

    for (auto i = 0u ; i < applesData.size() ; ++i) {
        applesData[i].priority = priority(appleCenter, distances[i], applesData, i);
        apples.push_back(applesData[i]);
    }

    std::sort(apples.begin(), apples.end(), appPrioritySort);

    for (auto i = 0u ; i < apples.size() ; ++i) {
        IDs.push_back(apples[i].id);
    }
    return IDs;
}


double priority(position center, double dist, std::vector<appleData> applesData, int appleID) {
    double priority;
    int rate;
    int distToCenter;
    int cornerRate = 0;

    if ((applesData[appleID].pos.x == 0 || applesData[appleID].pos.x == 19) &&
        (applesData[appleID].pos.y == 0 || applesData[appleID].pos.y == 19)) {
        cornerRate = MAX_COST;
    }

    distToCenter = distanceToPoint(center, applesData[appleID].pos);

    if (applesData[appleID].distanceToEnemy <= 2 &&
        applesData[appleID].distanceToMe > applesData[appleID].distanceToEnemy) {
        rate = MAX_COST;
    } else if (applesData[appleID].distanceToEnemy > 16) {
        rate = -2;
    } else if (applesData[appleID].distanceToEnemy > 8) {
        rate = -6;
    } else if (applesData[appleID].distanceToEnemy > 4) {
        rate = -12;
    } else {
        rate = -20;
    }

    if (applesInRangeToMe(applesData) > applesData.size()/4 && applesData.size() > 4) {
        rate = 6;
    }


    priority = (applesData[appleID].distanceToMe+1)*(distToCenter*(0.65)+dist) + rate + cornerRate;

    return priority;
}

bool appPrioritySort(appleData f, appleData s) {
    return (f.priority < s.priority);
}

int applesInRangeToMe(std::vector<appleData> apples, int distance) {
    int counter = 0;

    for(auto& u: apples) {
        if (u.distanceToMe < distance) {
            ++counter;
        }
    }
    return counter;
}

position endGameTarget() {
    position targetPos;

    if(board[10][0] == '.' || board[10][0] == '@') {
        targetPos.x = 10;
        targetPos.y = 0;
    } else if (board[10][19] == '.' || board[10][19] == '@') {
        targetPos.x = 10;
        targetPos.y = 19;
    } else {
        targetPos.x = 19;
        targetPos.y = 10;
    }

    return targetPos;
}
