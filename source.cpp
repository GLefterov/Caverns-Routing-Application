#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <cmath>
#include <unordered_map>

struct Cavern {
    int x, y;
};

double euclideanDistance(const Cavern& a, const Cavern& b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

struct FibNode {
    int key;
    double value;
    FibNode* left;
    FibNode* right;
    FibNode* child;
    FibNode* parent;
    bool marked;
    int degree;

    FibNode(int k, double v) : key(k), value(v), left(this), right(this), child(nullptr), parent(nullptr), marked(false), degree(0) {}
};

class FibonacciHeap {
private:
    FibNode* min;
    int nH;

    void consolidate() {
        std::vector<FibNode*> A(std::ceil(std::log(nH) / std::log(2)) + 1, nullptr);
        std::vector<FibNode*> roots;
        FibNode* curr = min;
        if (curr) {
            do {
                roots.push_back(curr);
                curr = curr->right;
            } while (curr != min);
        }

        for (FibNode* w : roots) {
            FibNode* x = w;
            int d = x->degree;
            while (A[d] != nullptr) {
                FibNode* y = A[d]; // Another node with the same degree as x
                if (x->value > y->value) std::swap(x, y);
                if (y == min) min = x;
                if (y->right == x) min = x; // Make sure min points to something outside the list we are about to modify
                link(y, x);
                A[d] = nullptr;
                d++;
            }
            A[d] = x;
        }

        min = nullptr;
        for (FibNode* x : A) {
            if (x) {
                if (!min) {
                    min = x;
                    min->left = min->right = min;
                } else {
                    x->left = min;
                    x->right = min->right;
                    min->right = x;
                    x->right->left = x;
                    if (x->value < min->value) {
                        min = x;
                    }
                }
            }
        }
    }

    void link(FibNode* y, FibNode* x) {
        // Remove y from the root list of the heap
        y->left->right = y->right;
        y->right->left = y->left;

        // Make y a child of x
        y->parent = x;
        if (x->child == nullptr) {
            x->child = y;
            y->right = y;
            y->left = y;
        } else {
            y->left = x->child;
            y->right = x->child->right;
            x->child->right = y;
            y->right->left = y;
        }

        x->degree++;
        y->marked = false;
    }

    void cut(FibNode* x, FibNode* y) {
        // Cuts x from y and add x to the root list.
    }

    void cascadingCut(FibNode* y) {
        // Performs cascading cut if necessary.
    }

public:
    FibonacciHeap() : min(nullptr), nH(0) {}

    FibNode* insert(int key, double value) {
        FibNode* newNode = new FibNode(key, value);
        min = merge(min, newNode);
        nH++;
        return newNode;
    }

    FibNode* merge(FibNode* a, FibNode* b) {
        if (!a) return b;
        if (!b) return a;
        if (a->value > b->value) std::swap(a, b);
        FibNode* temp = a->right;
        a->right = b;
        b->left = a;
        temp->left = b;
        b->right = temp;
        return a;
    }

    FibNode* getMin() {
        return min;
    }

    void decreaseKey(FibNode* x, double new_val) {
        if (x->value < new_val) return; // new value should be smaller
        x->value = new_val;
        FibNode* y = x->parent;
        if (y && x->value < y->value) {
            cut(x, y);
            cascadingCut(y);
        }
        if (x->value < min->value) {
            min = x;
        }
    }

    FibNode* removeMin() {
        FibNode* oldMin = min;
        if (min) {
            // Attach child nodes to root list
            // ...

            // Remove min from root list
            // ...

            if (min == min->right) {
                min = nullptr;
            } else {
                min = min->right;
                consolidate();
            }
            nH--;
        }
        return oldMin;
    }
};

// ... Rest of the Cavern and Dijkstra's implementation ...

std::vector<int> findPath(const std::vector<Cavern>& caverns, const std::vector<std::vector<int>>& matrix, int start, int end) {
    // Create a vector to store distances and initialize them to the maximum possible value.
    std::vector<double> distances(caverns.size(), std::numeric_limits<double>::max());
    // Create a vector to store predecessor indices to reconstruct the shortest path later.
    std::vector<int> predecessors(caverns.size(), -1);
    // Create a vector to keep track of visited caverns.
    std::vector<bool> visited(caverns.size(), false);

    // Initialize Fibonacci heap
    FibonacciHeap fh;

    // This unordered_map will store the mapping from cavern index to FibNode pointer.
    std::unordered_map<int, FibNode*> nodes;

    // Insert all nodes into the heap
    for (size_t i = 0; i < caverns.size(); ++i) {
        if (i == start) {
            nodes[i] = fh.insert(i, 0.0);
            distances[i] = 0.0;
        } else {
            nodes[i] = fh.insert(i, std::numeric_limits<double>::max());
        }
    }

    // Dijkstra's algorithm
    while (fh.getMin() != nullptr) {
        // Get the node with the minimum distance
        FibNode* minNode = fh.removeMin();
        int u = minNode->key;
        visited[u] = true;

        // Update distances for all adjacent vertices
        for (size_t v = 0; v < caverns.size(); ++v) {
            // Update distance if there's an edge, the vertex hasn't been visited, and found a shorter path
            if (matrix[u][v] != 0 && !visited[v]) {
                double distanceThroughU = distances[u] + euclideanDistance(caverns[u], caverns[v]);
                if (distanceThroughU < distances[v]) {
                    distances[v] = distanceThroughU;
                    fh.decreaseKey(nodes[v], distanceThroughU);
                    predecessors[v] = u;
                }
            }
        }
    }

    // Reconstruct the shortest path from start to end
    std::vector<int> path;
    for (int at = end; at != -1; at = predecessors[at]) {
        path.push_back(at);
    }
    std::reverse(path.begin(), path.end());

    // Return the shortest path from start to end
    return path;
}

int main(int argc, char* argv[]) {
    // Assuming you have a function to read caverns and matrix from file or standard input
    std::vector<Cavern> caverns = /* function to read caverns */;
    std::vector<std::vector<int>> matrix = /* function to read adjacency matrix */;

    int startCavern = /* index of start cavern */;
    int endCavern = /* index of end cavern */;

    std::vector<int> path = findPath(caverns, matrix, startCavern, endCavern);

    // Output the shortest path
    for (int cavernIndex : path) {
        std::cout << cavernIndex;
        if (cavernIndex != path.back()) {
            std::cout << " -> ";
        }
    }
    std::cout << std::endl;

    return 0;
}
