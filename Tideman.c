#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);

int formsCycle(int start, int loser);
void heap_assembler(int i, int end, int heap[], pair duo[], int direction); // Puts a heap element in correct position.
void heap_sort(int end, int heap[], pair duo[]);                            // Uses heap_assembler to reverse sort an array.


int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();

    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    int current = 0;
    while (current < candidate_count)
    {
        if (!strcmp(candidates[current], name))
        {
            ranks[rank] = current;
            return true;
        }
        current++;
    }
    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    for (int winner = 0;  winner < candidate_count - 1; winner ++)
    {
        for (int loser = winner + 1; loser < candidate_count; loser++)
        {
            preferences[ranks[winner]][ranks[loser]]++;
        }
    }

    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{

    for (int current = 0;  current < candidate_count - 1; current ++)
    {
        for (int next = current + 1; next <= candidate_count; next++)
        {
            // If current candidate wins over next, add pair and update pair_count.
            if (preferences[current][next] > preferences[next][current])
            {
                pairs[pair_count].winner = current;
                pairs[pair_count].loser = next;
                pair_count++;
            }

            // If next candidate wins over current, add pair and update pair_count.
            if (preferences[next][current] > preferences[current][next])
            {
                pairs[pair_count].winner = next;
                pairs[pair_count].loser = current;
                pair_count++;
            }
            // If there is a stalemate, no pair will be added.
        }
    }
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    // Makes an array of current pairs' victory strenght.
    int advantage[pair_count];

    for (int p = 0; p < pair_count; p++)
    {
        // Sets advantage according to preferences.
        advantage[p] = preferences[pairs[p].winner][pairs[p].loser] - preferences[pairs[p].loser][pairs[p].winner];
        //printf("%s over %s :%d \n", candidates[pairs[p].winner], candidates[pairs[p].loser], advantage[p]); // DEBUG - prints all pairs with identification.
    }
    heap_sort(pair_count, advantage, pairs);
    return;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    // Loops over all pairs. If no cycle is formed, lock the pair.
    for (int p = 0; p < pair_count; p++)
    {
        formsCycle(pairs[p].winner, pairs[p].loser) ? 0 : (locked[pairs[p].winner][pairs[p].loser] = true);
    }
    return;
}

// Print the winner of the election
void print_winner(void)
{
    bool check = false;
    int elected = 0;

    for (int loser = 0; loser < candidate_count; loser++)
    {
        check = false;
        for (int winner = 0; winner < candidate_count; winner++)
        {
            check = (check || locked[winner][loser]);
        }
        if (!check) // If "loser" has not lost against anyone
        {
            elected = loser;
            break;
        }
    }

    printf("%s\n", candidates[elected]);
    return;
}

void heap_sort(int end, int heap[], pair duo[])
{

    // Recursion maketh the heap.
    for (int c = end; c > 0; c--)
    {
        heap_assembler(c, end, heap, duo, 0);
    }

    // Sorts the heap.
    for (int c = end; c > 0; c--)
    {
        // Swaps first and last.
        int swapper = heap[end - 1];
        pair p_swapper = duo[end - 1];

        heap[end - 1] = heap[0];
        heap[0] = swapper;

        duo[end - 1] = duo[0];
        duo[0] = p_swapper;

        // Reduces heap size and puts the swapped element in correct position.
        end--;
        heap_assembler(1, end, heap, duo, 1);

    }
}

void heap_assembler(int i, int end, int heap[], pair duo[], int direction)
{

// This functions receives:
// a) target element of heap (int) | b) heap size (int) | c) an array | d) direction flag – 0 for down-top or positive int for top-down
// It will swap given element to correct position, regarding a min-heap. Works in-place.

//Starts with recursion check to avoid chance of segmentation fault during variables declarations.

    // Root reached, ends recursive call.
    if (direction == 0 && i == 1)
    {
        return;
    }

    // Last leaf reached, ends recursive call (if reverse flag is set)
    if (direction > 0 && i * 2 > end)
    {
        return;
    }

    int parent;
    direction == 0 ? (parent = i / 2) : (parent = i);

    int child;
    direction == 0 ? (child = i) : (child = i * 2);

    int sibling = 0;
    int swapper = heap[parent - 1];
    pair p_swapper = duo[parent - 1];

    // Checks if child has no siblings.
    if (!(child == end && child % 2 == 0))
    {
        // If the sibling is smaller, takes child's place.
        child % 2 !=  0 ? (sibling = child - 1) : (sibling = child + 1);
        heap[child - 1] > heap[sibling - 1] ? child = sibling : 0;
    }

    // If parent is bigger, swaps with child.
    if (heap[parent - 1] > heap[child - 1])
    {
        heap[parent - 1] = heap[child - 1];
        heap[child - 1] = swapper;

        duo[parent - 1] = duo[child - 1];
        duo[child - 1] = p_swapper;
    }

    // Recursive call until root / deepest level if reverse flag is set.
    direction == 0 ? heap_assembler(i / 2, end, heap, duo, 0) : heap_assembler(child, end, heap, duo, 1);
}

int formsCycle(int start, int loser)
// This function checks if the graph matrix plus given pair forms a cycle. Returns 1 or 0.
{
    // Checks if a cycle is formed.
    if (start == loser)
    {
        return 1;
    }

    for (int index = candidate_count - 1; index >= 0; index--)
    {
        // If the loser candidates wins over another, builds a chain through recursion to inspect for cycles.
        if (locked[loser][index])
        {
            if (formsCycle(start, index))
            {
                return 1;
            }
        }
    }
    return 0;
}
