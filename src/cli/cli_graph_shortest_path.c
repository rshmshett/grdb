/*
 * Single Source Shortest Path Implementation
 * in Graph Database using Dijkstra's Algorithm
 *
 * Rashmi Shetty
 * Reference : https://rosettacode.org/wiki/Dijkstra's_algorithm
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "tuple.h"
#include "graph.h"
#include <limits.h>
#define SIZE 20

typedef struct {
    int vertex; 
    int weight;
} newEdge_t;
        
typedef struct { 
    newEdge_t **edges;
    int edges_len;
    int edges_size;
    int dist;
    int prev;
    int visited;
} newVertex_t;
 
typedef struct {
    newVertex_t **vertices;
    int vertices_len;
    int vertices_size;
} newGraph_t;
    
typedef struct {
    int *data;
    int *prio;
    int *index;
    int len;
    int size; 
} heap_t;

//Adding vertices for shortest path calculations
void add_vertex (newGraph_t *g, int i) {
    if (g->vertices_size < i+1) {
        int size = g->vertices_size * 2 > i ? g->vertices_size * 2 : i + 4;
        g->vertices = realloc(g->vertices, size * sizeof (newVertex_t *));
        for (int j = g->vertices_size; j < size; j++)
            g->vertices[j] = NULL;
        g->vertices_size = size;
    }
    if (!g->vertices[i]) {
        g->vertices[i] = calloc(1, sizeof (newVertex_t));
        g->vertices_len++;
    }
}
 
//Adding edge for shortest path calculation
void add_edge (newGraph_t *g, int a, int b, int w) {
    add_vertex(g, a);
    add_vertex(g, b);
    newVertex_t *v = g->vertices[a];
    if (v->edges_len >= v->edges_size) {
        v->edges_size = v->edges_size ? v->edges_size * 2 : 4;
        v->edges = realloc(v->edges, v->edges_size * sizeof (newEdge_t *));
    }
    newEdge_t *e = calloc(1, sizeof (newEdge_t));
    e->vertex = b;
    e->weight = w;
    v->edges[v->edges_len++] = e;
}
 
//Function to create heap
heap_t *create_heap (int n) {
    heap_t *h = calloc(1, sizeof (heap_t));
    h->data = calloc(n + 1, sizeof (int));
    h->prio = calloc(n + 1, sizeof (int));
    h->index = calloc(n, sizeof (int));
    return h;
}
 
//Function to push on heap data structure the distances from source node
void push_heap (heap_t *h, int v, int p) {
    int i = h->index[v] == 0 ? ++h->len : h->index[v];
    int j = i / 2;
    while (i > 1) {
        if (h->prio[j] < p)
            break;
        h->data[i] = h->data[j];
        h->prio[i] = h->prio[j];
        h->index[h->data[i]] = i;
        i = j;
        j = j / 2;
    }
    h->data[i] = v;
    h->prio[i] = p;
    h->index[v] = i;
}
 

//Finding the minimum distance from heap
int min (heap_t *h, int i, int j, int k) {
    int m = i;
    if (j <= h->len && h->prio[j] < h->prio[m])
        m = j;
    if (k <= h->len && h->prio[k] < h->prio[m])
        m = k;
    return m;
}
 
//Retrieving the minimum distance entry from heap
int pop_heap (heap_t *h) {
    int v = h->data[1];
    int i = 1;
    while (1) {
        int j = min(h, h->len, 2 * i, 2 * i + 1);
        if (j == h->len)
            break;
        h->data[i] = h->data[j];
        h->prio[i] = h->prio[j];
        h->index[h->data[i]] = i;
        i = j;
    }
    h->data[i] = h->data[h->len];
    h->prio[i] = h->prio[h->len];
    h->index[h->data[i]] = i;
    h->len--;
    return v;
}
 
//Shortest Path calculations
void dijkstra (newGraph_t *g, int a, int b) {
    int i, j;
    for (i = 1; i <= g->vertices_len; i++) {
        newVertex_t *v = g->vertices[i];
        v->dist = INT_MAX;
        v->prev = 0;
        v->visited = 0;
    }
    newVertex_t *v = g->vertices[a];
    v->dist = 0;
    heap_t *h = create_heap(g->vertices_len);
    push_heap(h, a, v->dist);
    while (h->len) {
        i = pop_heap(h);
        if (i == b)
            break;
        v = g->vertices[i];
        v->visited = 1;
        for (j = 0; j < v->edges_len; j++) {
            newEdge_t *e = v->edges[j];
            newVertex_t *u = g->vertices[e->vertex];
            if (!u->visited && v->dist + e->weight <= u->dist) {
                u->prev = i;
                u->dist = v->dist + e->weight;
                push_heap(h, e->vertex, u->dist);
            }
        }
    }
}
 
void print_path (newGraph_t *g, int i) {
	int n, j;
        newVertex_t *v, *u;
        v = g->vertices[i];
        if (v->dist == INT_MAX) {
        	printf("no path\n");
	        return;
    	}
        for (n = 1, u = v; u->dist; u = g->vertices[u->prev], n++);
        int *path = (int *)malloc(sizeof(int)*n);
        path[n - 1] = i;
        for (j = 0, u = v; u->dist; u = g->vertices[u->prev], j++) 
        	path[n - j - 2] = u->prev;
    
        printf("\nShortest dist to destination: %d\nShortest Path: ", v->dist);
        for(j = 0; j<n; j++) 
		printf("%d  ", path[j]);
	printf("\n");
}

int get_weight(tuple_t t, enum_list_t el)
{
	attribute_t attr;
        int offset, weight;
        for (attr = t->s->attrlist; attr != NULL; attr = attr->next) {
                offset = tuple_get_offset(t, attr->name);
                if (offset >= 0) {
			weight = tuple_get_int(t->buf + offset);
			return weight;
		}
	}
	return 0;
}

void cli_graph_shortest_path(char *cmdline, int *pos)
{

	newGraph_t *g = calloc(1, sizeof (newGraph_t));

	vertex_t v, w;
	edge_t edge;
	char s[BUFSIZE];
	int node1[SIZE], node2[SIZE];
	int i, j;
	int count=0;
	int source, target;
	int weight[SIZE];

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
	if (strlen(s) == 0) {
		printf("Missing vertex id\n");
		return;
	}
	i = atoi(s);

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
	if (strlen(s) == 0) {
		printf("Missing vertex id\n");
		return;
	}
	j = atoi(s);

	v = graph_find_vertex_by_id(current, i);
	w = graph_find_vertex_by_id(current, j);

	if (v == NULL || w == NULL) {
		printf("Vertices do not exist in the current graph\n");
		return;
	}
	
	source = v->id;
	target = w->id;

	printf("\nSource vertex: %d, Destination vertex: %d\n", source, target);


        for (edge = current->e; edge->next != NULL; edge=edge->next, ++count) {
                printf("\nEdge found between: %llu and %llu", edge->id1, edge->id2);       
                node1[count] = edge->id1;
                node2[count] = edge->id2;
		weight[count] = get_weight(edge->tuple, current->el);
		printf("\nEdge weight = %d", weight[count]);
	}

		printf("\nEdge found between: %llu and %llu", edge->id1, edge->id2);
		node1[count] = edge->id1;
		node2[count] = edge->id2;
		weight[count] = get_weight(edge->tuple, current->el);
		 printf("\nEdge weight = %d\n", weight[count]);

	for(i=0; i<=count; i++) 
		add_edge(g, node1[i], node2[i], weight[i]);

			
    dijkstra(g, source, target);
    print_path(g, target);	

}
