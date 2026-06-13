/*****************************************
 * 
 * By submitting this file, I affirm that
 * I am the author of all modifications to
 * the provided code.
 *
 * The following is a list of those students
 * I had worked together in preparing this project:
 *    - Kingsman Quek (20654179)
 *    - 
 *    - 
 *    - 
 *****************************************/

#ifndef WEIGHTED_GRAPH_H
#define WEIGHTED_GRAPH_H

#ifndef nullptr
#define nullptr 0
#endif

#include <iostream>
#include <limits>
#include <cstdlib>
#include "Exception.h"

#if defined(_WIN32) || defined(__MINGW32__)
inline void srand48(long seed) { std::srand(seed); }
inline double drand48() { return static_cast<double>(std::rand()) / RAND_MAX; }
inline long lrand48() { return std::rand(); }
#endif


class Weighted_graph {
	private:
		int vertex_capacity;     // Stores the maximum capacity of vertices (N)
		int num_edges;           // Tracks the total number of unique edges in the graph
		int *vertex_degrees;     // Dynamic array holding the degree of each individual vertex
		double *matrix;          // Flat dynamic 1D array representing the adjacency matrix
		int cached_source;       // Tracks the last vertex used as a source for Dijkstra
		double *cached_distances;// Dynamically allocated array to store shortest paths from cached_source

		static const double INF;

	public:
		Weighted_graph( int = 50 );
		~Weighted_graph();
		Weighted_graph( const Weighted_graph & );
		Weighted_graph &operator=( const Weighted_graph & );

		int degree( int ) const;
		int edge_count() const;
		double adjacent( int, int ) const;
		double distance( int, int );

		void insert( int, int, double );

	// Friends

	friend std::ostream &operator<<( std::ostream &, Weighted_graph const & );
};

const double Weighted_graph::INF = std::numeric_limits<double>::infinity();

/*
 * Function: Constructor
 * Description: Initializes a new Weighted_graph with a specific vertex capacity.
 * Parameters: n - the maximum number of vertices the graph can hold.
 * Assumptions: None.
 * Error Handling: If n is 0 or negative, it defaults n to 1 to prevent invalid graphs.
 */
Weighted_graph::Weighted_graph( int n ) {
	if ( n <= 0 ) {
		n = 1;
	}

	vertex_capacity = n;
	num_edges = 0;
	cached_source = -1;

	// Allocate memory for tracking vertex degrees
	vertex_degrees = new int[vertex_capacity];
	for ( int i = 0; i < vertex_capacity; ++i ) {
		vertex_degrees[i] = 0;
	}

	// Allocate memory for the flat Adjacency Matrix
	matrix = new double[vertex_capacity * vertex_capacity];
	for ( int i = 0; i < vertex_capacity * vertex_capacity; ++i ) {
		matrix[i] = INF; // Initialize all paths to infinity (disconnected)
	}

	// Allocate memory for cached Dijkstra distances
	cached_distances = new double[vertex_capacity];
	for ( int i = 0; i < vertex_capacity; ++i ) {
		cached_distances[i] = INF;
	}
}

/*
 * Function: Destructor
 * Description: Frees all dynamically allocated memory for the graph's arrays.
 * Parameters: None.
 * Assumptions: Assumes arrays were successfully allocated.
 * Error Handling: None.
 */
Weighted_graph::~Weighted_graph() {
	delete [] vertex_degrees;
	delete [] matrix;
	delete [] cached_distances;
}

/*
 * Function: degree
 * Description: Returns the number of edges connected to a specific vertex.
 * Parameters: u - the index of the vertex to check.
 * Assumptions: Assumes the vertex index exists within the graph.
 * Error Handling: Throws illegal_argument if u is out of bounds (less than 0 or >= capacity).
 */
int Weighted_graph::degree( int u ) const {
	if ( u < 0 || u >= vertex_capacity ) {
		throw illegal_argument();
	}
	return vertex_degrees[u];
}

/*
 * Function: edge_count
 * Description: Returns the total number of edges currently in the graph.
 * Parameters: None.
 * Assumptions: None.
 * Error Handling: None.
 */
int Weighted_graph::edge_count() const {
	return num_edges;
}

/*
 * Function: adjacent
 * Description: Returns the weight of the edge between two vertices.
 * Parameters: u - the first vertex, v - the second vertex.
 * Assumptions: Assumes both vertices exist within the graph.
 * Error Handling: Throws illegal_argument if u or v are out of bounds.
 */
double Weighted_graph::adjacent( int u, int v ) const {
	if ( u < 0 || u >= vertex_capacity || v < 0 || v >= vertex_capacity ) {
		throw illegal_argument();
	}

	// Distance from a vertex to itself is always 0.0
	if ( u == v ) {
		return 0.0;
	}

	return matrix[u * vertex_capacity + v];
}

/*
 * Function: insert
 * Description: Adds or updates an undirected edge between two vertices with a specific weight.
 * Parameters: u - first vertex, v - second vertex, w - the weight of the edge.
 * Assumptions: Assumes weights are positive and self-loops are not allowed.
 * Error Handling: Throws illegal_argument if vertices are out of bounds, u == v, or weight is invalid.
 */
void Weighted_graph::insert( int u, int v, double w ) {
	// Enforce strict constraints: No out-of-bound indexes, no self-loops, and valid weights
	if ( u < 0 || u >= vertex_capacity || v < 0 || v >= vertex_capacity || u == v || w <= 0.0 || w == INF ) {
		throw illegal_argument();
	}

	// Invalidate shortest path cache since the graph structure has changed
	cached_source = -1;

	int idx1 = u * vertex_capacity + v;
	int idx2 = v * vertex_capacity + u;

	// If this edge doesn't already exist, increment total edges and both vertex degrees
	if ( matrix[idx1] == INF ) {
		num_edges++;
		vertex_degrees[u]++;
		vertex_degrees[v]++;
	}

	// Update weights symmetrically for an undirected graph structure
	matrix[idx1] = w;
	matrix[idx2] = w;
}

/*
 * Function: distance
 * Description: Computes the shortest path distance from vertex u to v using Dijkstra's Algorithm.
 * Parameters: u - the starting source vertex, v - the destination target vertex.
 * Assumptions: Assumes all edge weights are positive numbers.
 * Error Handling: Throws illegal_argument if u or v are out of bounds.
 */
double Weighted_graph::distance( int u, int v ) {
	if ( u < 0 || u >= vertex_capacity || v < 0 || v >= vertex_capacity ) {
		throw illegal_argument();
	}

	if ( u == v ) {
		return 0.0;
	}

	// Return cached result if we already ran Dijkstra from the same source vertex
	if ( u == cached_source ) {
		return cached_distances[v];
	}

	// Allocate temporary visited tracking array on the heap
	bool *visited = new bool[vertex_capacity];

	for ( int i = 0; i < vertex_capacity; ++i ) {
		cached_distances[i] = INF;
		visited[i] = false;
	}

	cached_distances[u] = 0.0; // Base distance to source node is 0

	for ( int count = 0; count < vertex_capacity; ++count ) {
		double min_dist = INF;
		int min_index = -1;

		// Identify the unvisited vertex with the absolute smallest tentative distance
		for ( int i = 0; i < vertex_capacity; ++i ) {
			if ( !visited[i] && cached_distances[i] < min_dist ) {
				min_dist = cached_distances[i];
				min_index = i;
			}
		}

		// If min_index is -1, all remaining unvisited nodes are completely unreachable from source
		if ( min_index == -1 ) {
			break;
		}

		visited[min_index] = true;

		// Relax neighbors of the chosen node using the flat 1D matrix
		int base_idx = min_index * vertex_capacity;
		for ( int neighbor = 0; neighbor < vertex_capacity; ++neighbor ) {
			double edge_weight = matrix[base_idx + neighbor];
			if ( !visited[neighbor] && edge_weight != INF ) {
				double new_dist = cached_distances[min_index] + edge_weight;
				if ( new_dist < cached_distances[neighbor] ) {
					cached_distances[neighbor] = new_dist;
				}
			}
		}
	}

	// Cache the source vertex to optimize subsequent queries
	cached_source = u;

	delete [] visited;

	return cached_distances[v];
}

/*
 * Function: Copy Constructor
 * Description: Creates a deep copy of an existing Weighted_graph to prevent memory sharing.
 * Parameters: other - a constant reference to the graph being copied.
 * Assumptions: Assumes the 'other' graph is fully initialized and valid.
 * Error Handling: None.
 */
Weighted_graph::Weighted_graph( const Weighted_graph &other ) {
	vertex_capacity = other.vertex_capacity;
	num_edges = other.num_edges;
	cached_source = other.cached_source;

	vertex_degrees = new int[vertex_capacity];
	for ( int i = 0; i < vertex_capacity; ++i ) {
		vertex_degrees[i] = other.vertex_degrees[i];
	}

	matrix = new double[vertex_capacity * vertex_capacity];
	for ( int i = 0; i < vertex_capacity * vertex_capacity; ++i ) {
		matrix[i] = other.matrix[i];
	}

	cached_distances = new double[vertex_capacity];
	for ( int i = 0; i < vertex_capacity; ++i ) {
		cached_distances[i] = other.cached_distances[i];
	}
}

/*
 * Function: Overloaded Assignment Operator
 * Description: Safely assigns the values of one graph to another, managing memory cleanly.
 * Parameters: other - a constant reference to the graph being assigned from.
 * Assumptions: Assumes the 'other' graph is valid.
 * Error Handling: Checks for self-assignment to prevent deleting its own memory prematurely.
 */
Weighted_graph &Weighted_graph::operator=( const Weighted_graph &other ) {
	// Prevent self-assignment (e.g., graph1 = graph1;)
	if ( this == &other ) {
		return *this;
	}

	// Clean up old memory to prevent leaks
	delete [] vertex_degrees;
	delete [] matrix;
	delete [] cached_distances;

	// Copy data over
	vertex_capacity = other.vertex_capacity;
	num_edges = other.num_edges;
	cached_source = other.cached_source;

	vertex_degrees = new int[vertex_capacity];
	for ( int i = 0; i < vertex_capacity; ++i ) {
		vertex_degrees[i] = other.vertex_degrees[i];
	}

	matrix = new double[vertex_capacity * vertex_capacity];
	for ( int i = 0; i < vertex_capacity * vertex_capacity; ++i ) {
		matrix[i] = other.matrix[i];
	}

	cached_distances = new double[vertex_capacity];
	for ( int i = 0; i < vertex_capacity; ++i ) {
		cached_distances[i] = other.cached_distances[i];
	}

	return *this;
}

// You can modify this function however you want:  it will not be tested
std::ostream &operator<<( std::ostream &out, Weighted_graph const &graph ) {
	return out;
}

#endif
