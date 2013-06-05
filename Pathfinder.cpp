/*
 * File: Pathfinder.cpp
 * --------------------
 * Name: Hari Iyer
 * This file implements the Pathfinder application, which allows a user
 * to play with two graph algorithms on three different maps.
 */
  
 
#include <iostream>
#include <fstream>
#include <string>
#include "console.h"
#include "graphtypes.h"
#include "gpathfinder.h"
#include "graph.h"
#include "set.h"
#include <math.h>
#include "pqueue.h"
#include <map>
#include "path.h"
#include "simpio.h"
using namespace std;
 
 
 
/* CONSTANTS */
const int WHITESPACE = 1;
const int REASONABLE_CLICK_RANGE = 6;
const int MIDDLE_EARTH_SECOND_CITY_INDEX = 15;
const int MIDDLE_EARTH_DISTANCE_INDEX = 30;
const int MIDDLE_EARTH_DISTANCE_TEXT = 2;
const int DISTANCE_FORMULA_POWER = 2;
const int NUM_PATH_ENDPOINTS = 2;
const string DEFAULT_ARC_COLOR = "Blue";
 
 
 
 
/* Function prototypes */
void runPathfinder();
void convertMapDataToInternalRepresentation(PathfinderGraph & graph);
void openAndProcessFileByLine(PathfinderGraph & graph, string mapName);
void processNodes(ifstream & infile, PathfinderGraph & graph);
void addNodeToGraph(string city, double xCoord, double yCoord, PathfinderGraph & graph);
void processArcs(ifstream & infile, PathfinderGraph & graph, string mapName);
void addArcToGraph(string pairCityOne, string pairCityTwo, double distancePairCities, PathfinderGraph & graph);
void drawAllNodesArcs(PathfinderGraph & graph);
void recolorAllNodes(PathfinderGraph & graph, string color);
void recolorAllArcs(PathfinderGraph & graph, string color);
void highlightNode(Node* node);
void highlightArc(Arc* arc);
void addBasicButtons(PathfinderGraph & graph);
void dijkstra(PathfinderGraph & graph);
bool withinCityRadius(GPoint pt, Node* node);
Node* userSelectNode(Set<Node*> & allNodes);
Path findShortestPath(Node *start, Node *finish);
double getPathCost(const Vector<Arc *> & path);
void quitAction();
void kruskal(PathfinderGraph & graph);
 
 
 
 
 
/* Main program */
 
int main() {
    runPathfinder();
    return 0;
}
 
 
/* Function: runPathfinder
 * Usage: runPathfinder();
 * --------------------------------------
 * This function calls all the core functions needed for Pathfinder.
 */
 
  
void runPathfinder() {
    PathfinderGraph graph;
    initPathfinderGraphics();
    addBasicButtons(graph);
    pathfinderEventLoop();
}
 
 
/* Function: askUserWhichMap
 * Usage: mapChoice = askUserWhichMap();
 * ---------------------------------------
 * This function asks the user which map he/she wants to use.
 * It assumes the files "Small.txt", "USA.txt", and "MiddleEarth.txt"
 * are stored in the same location.
 */
  
  
int askUserWhichMap(){
    cout<<"Which map would you like to use?"<<endl<<endl;
    cout<<"1 for a small test map!"<<endl;
    cout<<"2 for a map of the US!"<<endl;
    cout<<"3 for a map of Middle Earth!"<<endl;
    int mapChoice = getInteger("Please enter 1, 2 or 3: ");
    return mapChoice;
}
 
/* Function: convertMapDataToInternalRepresentation
 * Usage: addButton("Map", convertMapDataToInternalRepresentation, graph);
 * -----------------------------------------
 * This function is called when the user clicks on the map button.
 * It first calls askUserWhichMap to identify the file needed, and then
 * it sends that file name to openAndProcessFileByLine, which then operates
 * on that file.
 */
 
 
void convertMapDataToInternalRepresentation(PathfinderGraph & graph) {
    graph.clear();
    int mapChoice = askUserWhichMap();
    Vector<string> allMaps;
    allMaps.add("Small.txt");
    allMaps.add("USA.txt");
    allMaps.add("MiddleEarth.txt");
    string mapName = allMaps[mapChoice-1];
    openAndProcessFileByLine(graph, mapName);
}
 
 
 
/* Function: openAndProcessFileByLine
 * Usage: openAndProcessFileByLine(graph, mapName);
 * --------------------------------------------
 * This function goes through the text file and draws the relevant map,
 * and then it calls processNodes and processArcs to store the info in
 * the relevant data structures. Nodes and arcs are drawn by calling
 * the function drawAllNodesArcs.
 */
 
void openAndProcessFileByLine(PathfinderGraph & graph, string mapName) {
    ifstream infile;
    string line;
    infile.open(mapName.c_str());
    getline(infile, line);
    drawPathfinderMap(line);
    processNodes(infile, graph);
    processArcs(infile, graph, mapName);
    drawAllNodesArcs(graph);
    infile.close();
}
 
 
 
/* Function: processNodes
 * Usage: processNodes(infile, graph);
 * --------------------------------------------
 * This function goes through each line of the file, extracts the
 * city name and location info, and then calls addNodeToGraph to store
 * that node info in the graph.
 */
 
 
void processNodes(ifstream & infile, PathfinderGraph & graph) {
    string line;
    getline(infile, line);
    while (true) {
        getline(infile, line);
        if (line=="ARCS") return;
 
        int firstSpace = line.find(" ");
        int secondSpace = line.find(" ", firstSpace+WHITESPACE);
        string city = line.substr(0, firstSpace);
         
        double xCoord = strtod(line.substr(firstSpace+WHITESPACE, secondSpace-(firstSpace+WHITESPACE)).c_str(), NULL);
        double yCoord = strtod(line.substr(secondSpace+WHITESPACE, (line.size()-1) - (secondSpace)).c_str(), NULL);
        addNodeToGraph(city, xCoord, yCoord, graph);
    }
}
 
 
/* Function: addNodeToGraph
 * Usage: addNodeToGraph(city, xCoord, yCoord, graph);
 * -------------------------------------------------
 * This function takes the info extracted by processNodes and
 * adds it to the Node* struct, which is then stored in the graph.
 */
 
 
void addNodeToGraph(string city, double xCoord, double yCoord, PathfinderGraph & graph) {
    Node* currentNode = new Node;
    currentNode->name = city;
    currentNode->loc = GPoint(xCoord,yCoord);
    graph.addNode(currentNode);
}
 
 
/* Function: processMiddleEarthArcs
 * Usage: processMiddleEarthArcs(line, graph);
 * ----------------------------------------------
 * This function takes the line info and extracts the relevant information.
 * The formatting for the arc descriptions in the file MiddleEarth.txt is 
 * different from that in Small.txt and USA.txt, so it needs to be processed
 * separately. It then calls addArcToGraph to add the info the graph.
 */
 
void processMiddleEarthArcs(string line, PathfinderGraph & graph) {
     
    int firstSpace = line.find(" ");
    int secondSpace = line.find(" ", MIDDLE_EARTH_SECOND_CITY_INDEX);
    string pairCityOne = line.substr(0, firstSpace);
    string pairCityTwo = line.substr(MIDDLE_EARTH_SECOND_CITY_INDEX, secondSpace-MIDDLE_EARTH_SECOND_CITY_INDEX);
    double distancePairCities = strtod((line.substr(MIDDLE_EARTH_DISTANCE_INDEX, MIDDLE_EARTH_DISTANCE_TEXT)).c_str(), NULL);
    addArcToGraph(pairCityOne, pairCityTwo, distancePairCities, graph);         
 
}
 
/* Function: processUSArcs
 * Usage: processUSArcs(line, graph);
 * ----------------------------------------------
 * This function extracts arc information from lines in the Small.txt
 * and USA.txt files. It then calls addArcToGraph to add the info to 
 * arcs in the graph.
 *
 */
 
void processUSArcs(string line, PathfinderGraph & graph) {
     
    int firstSpace = line.find(" ");
    int secondSpace = line.find(" ", firstSpace+WHITESPACE);
    string pairCityOne = line.substr(0, firstSpace);
    string pairCityTwo = line.substr(firstSpace+1, secondSpace-(firstSpace+WHITESPACE));
    double distancePairCities = strtod(line.substr(secondSpace+WHITESPACE, (line.size()-1)).c_str(), NULL);
    addArcToGraph(pairCityOne, pairCityTwo, distancePairCities, graph);         
 
     
}
 
 
/* Function: processArcs
 * Usage: processArcs(infile, graph, mapName);
 * -------------------------------------------
 * This function is called by openAndProcessFileByLine to process
 * all the arc information in the files. The function checks to see
 * which file the info needs to be extracted from, and separates
 * the case for MiddleEarth and USA/Small, because the formatting is
 * different.
 */
 
 
void processArcs(ifstream & infile, PathfinderGraph & graph, string mapName) {
    string line;
    while (true) {
        getline(infile, line);
        if (infile.fail()) break;
        if (mapName=="MiddleEarth.txt") {
            processMiddleEarthArcs(line, graph);
        }
         
        else {
            processUSArcs(line, graph);
        }
    }   
}
 
 
/* Function: addArcToGraph
 * Usage: addArcToGraph(pairCityOne, pairCityTwo, distancePairCities, graph);
 * -----------------------------------------
 * This function takes the extracted info about the arcs and adds them to
 * an Arc* struct, which is then added to the graph.
 */
 
 
void addArcToGraph(string pairCityOne, string pairCityTwo, double distancePairCities, PathfinderGraph & graph) {
    for (int i=0; i<NUM_PATH_ENDPOINTS; i++) {
        Arc* currentArc = new Arc;
        currentArc->cost = distancePairCities;
        if (i==0) {
            currentArc->start = graph.getNode(pairCityOne);
            currentArc->finish = graph.getNode(pairCityTwo);
        }
        if (i==1) {
            currentArc->start = graph.getNode(pairCityTwo);
            currentArc->finish = graph.getNode(pairCityOne);
        }
        graph.addArc(currentArc);
    }
}
 
 
 
/* Function: drawAllNodesArcs
 * Usage: drawAllNodesArcs(graph);
 * ------------------------------------------
 * This function draws all the nodes and arcs in the graph.
 */
 
 
void drawAllNodesArcs(PathfinderGraph & graph) {
    recolorAllNodes(graph,NODE_COLOR);
    recolorAllArcs(graph, DEFAULT_ARC_COLOR);
    repaintPathfinderDisplay();
}
 
 
/* Function: highlightNode
 * Usage: highlightNode(node);
 * -----------------------------------------
 * This function makes it easy to highlight a node so we don't need
 * to remember the name of the function.
 */
 
void highlightNode(Node* node) {
    drawPathfinderNode(node->loc, HIGHLIGHT_COLOR, node->name);
}
 
 
/* Function: highlightArc
 * Usage: highlightArc(arc);
 * -------------------------------------------
 * This function makes it easy to highlight an arc.
 */
 
 
void highlightArc(Arc* arc) {
    drawPathfinderArc((arc->start)->loc, (arc->finish)->loc, HIGHLIGHT_COLOR);
}
 
 
/* Function: recolorAllNodes
 * Usage: recolorAllNodes(graph, color);
 * ---------------------------------------------
 * This function makes it easy to color all the nodes at once.
 */
 
void recolorAllNodes(PathfinderGraph & graph, string color) {
    Set<Node*> allNodes = graph.getNodeSet();
    foreach (Node* node in allNodes) {
        drawPathfinderNode(node->loc, color, node->name);
    }
    repaintPathfinderDisplay();
}
 
 
/* Function: recolorAllArcs
 * Usage: recolorAllArcs(graph, color);
 * ---------------------------------------------
 * This function makes it easy to color all the arcs at once.
 */
 
void recolorAllArcs(PathfinderGraph & graph, string color) {
    Set<Arc*> allArcs = graph.getArcSet();
    foreach (Arc* arc in allArcs) {
        drawPathfinderArc((arc->start)->loc, (arc->finish)->loc, color);
    }
    repaintPathfinderDisplay();
}
 
 
/* Function: addBasicButtons
 * Usage: addBasicButtons(graph);
 * ----------------------------------------------
 * This function adds the standard set of buttons to the display.
 */
 
void addBasicButtons(PathfinderGraph & graph){
    addButton("Quit", quitAction);
    addButton("Map", convertMapDataToInternalRepresentation, graph);
    addButton("Dijkstra", dijkstra, graph);
    addButton("Kruskal", kruskal, graph);
}
 
 
/* Function: quitAction
 * Usage: addButton("Quit", quitAction);
 * ------------------------------------------------
 * This function is called when the user clicks the Quit button,
 * causing the program to terminate and close the display.
 */
 
 
void quitAction() {
    exit(0);
}
 
 
 
 
/* Function: userSelectNode
 * Usage: Node* startNode = userSelectNode(allNodes);
 * --------------------------------------------------
 * This function waits for the user to click on the graph, and calls
 * withinCityRadius to check if the user clicked on a city. It doesn't
 * terminate till the user clicks on a valid city. It's called by
 * the implementation of Dijkstra.
 */
 
 
Node* userSelectNode(Set<Node*> & allNodes) {
 
    bool validEntry = false;
    while (!validEntry) {
        GPoint pt = getMouseClick();
        foreach (Node* node in allNodes) {
            if (withinCityRadius(pt, node)) {
                highlightNode(node);
                return node;
            }
        }
        cout<< "Please click on a city." <<endl; 
    }   
}
 
 
/* Function: withinCityRadius
 * Usage: if (withinCityRadius(pt, node))
 * ---------------------------------------------------
 * This function uses the standard distance formula:
 *   distance = sqrt((x1-x2)^2 + (y2-y1)^2)
 * to see if th user clicked reasonably close to a city,
 * as defined by the REASONABLE_CLICK_RANGE constant.
 * The constant is set to 6, and the node radius is 4,
 * so it seems fair.
 */
 
 
bool withinCityRadius(GPoint pt, Node* node) {
     
    double nodeXCoord = node->loc.getX();
    double nodeYCoord = node->loc.getY();
    double ptXcoord = pt.getX();
    double ptYcoord = pt.getY();
    double distance = sqrt(pow((nodeXCoord - ptXcoord), DISTANCE_FORMULA_POWER) + pow((nodeYCoord - ptYcoord), DISTANCE_FORMULA_POWER));
     
    if (distance<REASONABLE_CLICK_RANGE) {
        return true;
    }
    return false;
     
}
 
 
/* Function: dijkstra
 * Usage: dijkstra(graph);
 * ---------------------------------------------
 * This function uses Dijkstra's shortest path algorithm
 * to highlight the shortest path between two cities the
 * user selects. It starts off graying out all paths, then
 * asks the user to select two cities. These cities are then
 * sent to findShortestPath, which returns a vector of arcs
 * that form the shortest path. Those arcs are then
 * highlighted for display.
 *
 */
 
 
void dijkstra(PathfinderGraph & graph) {
     
    if (graph.isEmpty()) {
        cout<<"Please select a map!"<<endl;
        return;
    }
    Set<Node*> allNodes = graph.getNodeSet();
    recolorAllArcs(graph, DIM_COLOR);
    recolorAllNodes(graph, NODE_COLOR);
         
    Node* startNode = userSelectNode(allNodes);
    Node* endNode = userSelectNode(allNodes);
 
    Path path = findShortestPath(startNode, endNode);
    Vector<Arc*> allArcs = path.allArcs();
     
    foreach (Arc* arc in allArcs) {
        highlightArc(arc);
    }
 
     
}
 
 
/* Function: findShortestPath
 * Usage: Vector<Arc *> path = findShortestPath(start, finish);
 * ------------------------------------------------------------
 * Finds the shortest path between the nodes start and finish 
 * using * Dijkstra's algorithm, which keeps track of the shortest 
 * paths in * a priority queue. The function returns a vector of arcs, 
 * which is * empty if start and finish are the same node or if no path 
 * exists.
 * 
 * Mostly copied from Course Reader with slight modifications,
 * so kept the length of the given code the same for clarity.
 */
 
Path findShortestPath(Node *start, Node *finish) {
    Path path;
    PriorityQueue < Path > mainQueue;
    Map<string, double> fixed;
    while (start != finish) {
        if (!fixed.containsKey(start->name)) {
            fixed.put(start->name, path.totalCost());
            foreach (Arc *arc in start->arcs) {
                if (!fixed.containsKey(arc->finish->name)) {
                    path.add(arc);
                    mainQueue.enqueue(path, path.totalCost());
                    path.removeAt(path.size() - 1);
                }
            }
        }
        if (mainQueue.isEmpty()) {
            path.clear();
            return path;
        }
        path = mainQueue.dequeue();
        start = path.getArc(path.size() - 1)->finish;
    }
    return path;
}
 
 
 
 
 
 
 
/* ---------------------------- CODE RELATED TO KRUSKAL'S ALGORITHM ----------------------- */
 
 
  
 
/* Function: oneCityInBucketButNotOther
 * Usage: if (oneCityInBucketButNotOther(i, visitedLocations, startName, finishName))
 * -----------------------------------------------
 * As the cities on the endpoints of the arc are added in sets stored in a vector,
 * this function checks to see if those cities have been previously added
 * but stored in separate sets.
 */
  
 bool oneCityInBucketButNotOther(int i, Vector<Set<string> > & visitedLocations, string startName, string finishName) {
  
     if ((visitedLocations[i].contains(startName) &&
     !visitedLocations[i].contains(finishName)) ||
     (!visitedLocations[i].contains(startName) &&
     visitedLocations[i].contains(finishName))) {
         return true;
     }
     else {
         return false;
     }
 }
  
  
/* Function: addNewCitySet
 * Usage: addNewCitySet(startName, finishName, visitedLocations);
 * -------------------------------------------------
 * If a pair of cities has not been added to the path,
 * this function creates a new set, adds the names of the cities
 * to that set, and then adds the set to the path.
 */
 
 void addNewCitySet(string startName, string finishName, Vector<Set<string> > & visitedLocations){
  
    Set<string> newCitySet;
    newCitySet.add(startName);
    newCitySet.add(finishName);
    visitedLocations.add(newCitySet);
 }
  
  
 
/* Function: checkEachBucketOfCities
 * Usage: if (checkEachBucketOfCities(currentArc, visitedLocations, numBucketsEndpoints, startName, finishName, path))
 * -----------------------------------------------------
 * This function goes through each set of cities in the vector
 * and checks two cases:
 *
 *      1) Both cities are in the same set. This means that
 *          they're both on the same path, and nothing needs
 *          to be done.
 *
 *      2) They are in different sets/buckets. In this case,
 *          the two buckets are merged, the arc is added
 *          to the path, and the second bucket is deleted.
 *          This is the same as cities in different paths
 *          now being joined together on the same path.
 */
  
bool checkEachBucketOfCities(Arc* currentArc, Vector<Set<string> > & visitedLocations, Vector<int> & numBucketsEndpoints, string startName, string finishName, Path & path) {
 
    for (int i=0; i<visitedLocations.size(); i++) {
        if (visitedLocations[i].contains(startName) &&
            visitedLocations[i].contains(finishName)) {
            return true;
        }
        if (oneCityInBucketButNotOther(i, visitedLocations, startName, finishName)){
            numBucketsEndpoints.add(i);
             
            if (numBucketsEndpoints.size()==2) {
                visitedLocations[numBucketsEndpoints[0]] += visitedLocations[numBucketsEndpoints[1]];
                visitedLocations.removeAt(numBucketsEndpoints[1]);
                path.add(currentArc);
                return true;
            }
        }
    }
    return false;
}
 
/* Function: atLeastOneNewCity
 * Usage: if (atLeastOneNewCity(currentArc, visitedLocations, numBucketsEndpoints, startName, finishName, path))
 * ---------------------------------------
 * In the case where one of the two cities in the new arc
 * is already in the path, the second city is added to
 * the same set. In the case where neither are part of the
 * path, we call addNewCitySet to add the new path to the graph.
 */
  
 bool atLeastOneNewCity(Arc* currentArc,Vector<Set<string> > & visitedLocations, Vector<int> & numBucketsEndpoints, string startName, string finishName,Path & path) {
  
     if (numBucketsEndpoints.size()==1) {
         visitedLocations[numBucketsEndpoints[0]].add(startName);
         visitedLocations[numBucketsEndpoints[0]].add(finishName);
         path.add(currentArc);
         return true;
     }
     if (numBucketsEndpoints.isEmpty()) {
         addNewCitySet(startName, finishName, visitedLocations);
         path.add(currentArc);
         return true;
     }          
     return false;
 }
  
  
/* Function: findBucketWithCities
 * Usage: findBucketWithCities(currentArc, visitedLocations, startName, finishName, path);
 * -------------------------------------------
 * This function identifies the location of the set in the vector
 * of Set<Arc*> that contains the cities in the arc being considered.
 * This information is then stored in a vector. This is key, because it
 * has two advantages: First, the vector size shows how many of the cities
 * are new - not currently part of the path. Second, the elements themselves
 * are references to locations, which allow direct modification of the sets
 * at a later time as needed.
 */
 
 
 void findBucketWithCities(Arc* currentArc, Vector<Set<string> > & visitedLocations, string startName, string finishName, Path & path) {
     Vector<int> numBucketsEndpoints;
     if (checkEachBucketOfCities(currentArc, visitedLocations, numBucketsEndpoints, startName, finishName, path)){
         return;
     }
     if (atLeastOneNewCity(currentArc, visitedLocations, numBucketsEndpoints, startName, finishName, path)) {
        return;
     }
 }
  
 
/* Function: processInOrderOfCost
 * Usage: processInOrderOfCost(mainQueue, path);
 * --------------------------------------------
 * This function dequeues the priority queue that stores
 * all the Arc*, and since it's a priority queue, it dequeues
 * them in order of increasing cost. After extracting the names
 * of the cities it connects, it calls findBucketWithCities to
 * figure out if the arc is a new path, creates a cycle, or adds
 * to an existing path.
 */
  
 void processInOrderOfCost(PriorityQueue<Arc*> & mainQueue, Path & path){
     Vector<Set<string> > visitedLocations;
     while (!mainQueue.isEmpty()) {
         Arc* currentArc = mainQueue.dequeue();
         string startName = currentArc->start->name;
         string finishName = currentArc->finish->name;
         findBucketWithCities(currentArc, visitedLocations, startName, finishName, path);
     }
  }
  
 /* Function: kruskal
  * Usage: addButton("Kruskal", kruskal, graph);
  * ------------------------------------------
  * This is the function called when the user clicks the Kruskal
  * button, and it implements Kruskal's algorithm for finding 
  * minimum spanning trees (MST). Once the MST is found, the function
  * iterates through the arcs that make up the MST and highlights them.
  */
 
  
 void kruskal(PathfinderGraph & graph) {
     if (graph.isEmpty()) {
         cout<<"Please select a map!"<<endl;
         return;
     }
     Path path;
     Set<Arc*> allArcs = graph.getArcSet();
     recolorAllArcs(graph, DIM_COLOR);
     PriorityQueue<Arc*> mainQueue;
     foreach (Arc* arc in allArcs) {
         mainQueue.enqueue(arc, arc->cost);
     }
     processInOrderOfCost(mainQueue, path);
     Vector<Arc*> pathArcs = path.allArcs();
     foreach (Arc* arc in pathArcs) {
         highlightArc(arc);
     }
 }
 
 
