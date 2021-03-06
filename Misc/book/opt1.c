#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NODES	1024

char	*P;			// The most recent token character scanned in

				// Each AST node has these member fields
int	Type[NODES],		// The operation being performed on
	Left[NODES],		// the left, right, neither or both sub-trees
	Right[NODES],	
	Value[NODES];		// Leaf nodes have a value

// Type can be one of: + - * /, or
//	'c' for a literal constant stored in Value, or
//	'v' for a single-letter variable name store in Value

// We start at node 1 because 0 is used in Left and Right to indicate
// no child sub-node
int	Next = 1;

// Allocate a node from the list with
// the given values and return its slot number
int node(int t, int v, int l, int r) {
	if (Next >= NODES) {
		fprintf(stderr, "out of nodes\n");
		exit(EXIT_FAILURE);
	}
	Type[Next] = t;
	Value[Next] = v;
	Left[Next] = l;
	Right[Next] = r;
	return Next++;
}

// Skip whitespace on the input
void skip(void) {
	while (isspace(*P)) P++;
}

int sum(void);

// factor := 
//	  VARIABLE
//	| CONSTANT
//	| '(' sum ')'
//	| '-' factor

int factor(void) {
	int	n = 0, v;

	// Skip whitespace
	skip();

	// Parentheses: skip, parse with sum, skip ')'
	if ('(' == *P) {
		P++;
		n = sum();
		P++;
	}
	// Minus: skip, make a unary '-' node with the
	// factor() expression as the left child
	else if ('-' == *P) {
		P++;
		n = node('-', 0, factor(), 0);
	}
	// Digit: build up the decimal value, make
	// constant leaf node with this value
	else if (isdigit(*P)) {
		for (v=0; isdigit(*P); P++)
			v = v*10 + *P-'0';
		n = node('c', v, 0, 0);
	}
	// Letter: make a variable leaf node with this letter
	else if (isalpha(*P)) {
		n = node('v', *P++, 0, 0);
	}
	// Return the slot number of the tree
	return n;
}

// term :=
//	  factor
//	| factor '*' factor
//	| factor '/' factor

int term(void) {
	int	n, n2;

	// Skip whitespace, parse the next expression
	skip();
	n = factor();
	for (;;) {
		// Skip following whitespace
		skip();
		switch (*P) {
		// Next token was '*'. Skip over it. Parse
		// the next expression. Build a binary node
		// with '*' as the operator and both children
		case '*': P++;
			  n2 = factor();
			  n = node('*', 0, n, n2);
			  break;
		// Next token was '/'. Skip over it. Parse
		// the next expression. Build a binary node
		// with '/' as the operator and both children
		case '/': P++;
			  n2 = factor();
			  n = node('/', 0, n, n2);
			  break;
		// No following '*' or '/', return the factor tree
		default:  return n;
		}
	}
}

// sum :=
//	  term
//	| term '+' term
//	| term '-' term
int sum(void) {
	int	n, n2;

	// Skip whitespace, parse the next expression
	skip();
	n = term();
	for (;;) {
		// Skip following whitespace
		skip();
		switch (*P) {
		// Next token was '+'. Skip over it. Parse
		// the next expression. Build a binary node
		// with '+' as the operator and both children
		case '+': P++;
			  n2 = term();
			  n = node('+', 0, n, n2);
			  break;
		// Next token was '-'. Skip over it. Parse
		// the next expression. Build a binary node
		// with '-' as the operator and both children
		case '-': P++;
			  n2 = term();
			  n = node('-', 0, n, n2);
			  break;
		default:  return n;
		}
	}
}

// Parse an expression. Get a pointer
// to the first character and call sum()
int expr(char *s) {
	P = s;
	return sum();
}

// Recursively dump the AST rooted at n.
// Indent by k spaces.
void dump(int n, int k) {
	int	i;

	// No node, return
	if (!n) return;

	// Print k spaces
	for (i=0; i<k; i++)
		printf("  ");

	// Print out the operator, 'c' or 'v'
	putchar(Type[n]);

	// Print out a leaf's literal value or variable name
	if ('c' == Type[n])
		printf("(%d)", Value[n]);
	else if ('v' == Type[n])
		printf("(%c)", Value[n]);

	// End the line, then dump the left and right children
	putchar('\n');
	dump(Left[n], k+1);
	dump(Right[n], k+1);
}

// These functions draw the AST using
// pic and groff. Invoke with -d.
void draw2(int n, int w, int d) {
	if (!n) return;
	if ('c' == Type[n])
		printf("N%d: box width 0.3i height 0.3i \"%d\"\n",
			n, Value[n]);
	else if ('v' == Type[n])
		printf("N%d: box width 0.3i height 0.3i \"%c\"\n",
			n, Value[n]);
	else
		printf("N%d: circle radius 0.15i \"%c\"\n", n, Type[n]);
	if (Left[n]) {
		printf("move to N%d\n", n);
		if (Right[n]) {
			printf("move down 0.5i left %d.%di\n", w/1000, w%1000);
			draw2(Left[n], w/d, d+1);
			printf("arrow from left of N%d to top of N%d\n",
				n, Left[n]);
		}
		else {
			printf("move down 0.5i\n");
			draw2(Left[n], w/d, d+1);
			printf("arrow from bottom of N%d to top of N%d\n",
				n, Left[n]);
		}
	}
	if (Right[n]) {
		printf("move to N%d\n", n);
		if (Left[n]) {
			printf("move down 0.5i right %d.%di\n", w/1000, w%1000);
			draw2(Right[n], w/d, d+1);
			printf("arrow from right of N%d to top of N%d\n",
				n, Right[n]);
		}
		else {
			printf("move down 0.5i\n");
			draw2(Right[n], w/d, d+1);
			printf("arrow from bottom of N%d to top of N%d\n",
				n, Right[n]);
		}
	}
}

// These functions draw the AST using
// pic and groff. Invoke with -d.
void draw(int n) {
	printf(".ft C\n.ps 12\n.PS\n");
	draw2(n, 1800, 24);
	printf(".PE\n");
}

// Perform constant expression folding
// on the tree rooted at n, returning
// the root of a new tree.
int fold(int n) {
	int	cl, cr, vl, vr;

	// Empty tree, nothing to do
	if (!n) return 0;

	// Fold both child sub-trees
	Left[n] = fold(Left[n]);
	Right[n] = fold(Right[n]);

	// Determine if the left child and/or
	// the right child is a literal constant
	cl = Left[n] && 'c' == Type[Left[n]];
	cr = Right[n] && 'c' == Type[Right[n]];

	// No right child, left child is a literal
	// constant and the operation is unary minus
	if (cl && 0 == Right[n]) {
		if ('-' == Type[n])
			// Replace with a negative literal value
			return node('c', -Value[Left[n]], 0, 0);
	}

	// Get the values from left and right child. Set to zero
	// if no child
	vl = Left[n]? Value[Left[n]]: 0;
	vr = Right[n]? Value[Right[n]]: 0;

	// Both children are literal constants
	if (cl && cr) {

		// So we can do the node's operation here
		// and save assembly instructions. Only
		// do a division if the divisor is not zero.
		switch (Type[n]) {
		case '+': return node('c', vl+vr, 0, 0);
		case '-': return node('c', vl-vr, 0, 0);
		case '*': return node('c', vl*vr, 0, 0);
		case '/': if (vr) return node('c', vl/vr, 0, 0);
		}
	}

	// 0 + right is just right.
	if (cl && 0 == vl && '+' == Type[n]) {
		return Right[n];
	}

	// 1 * right is just right.
	if (cl && 1 == vl && '*' == Type[n]) {
		return Right[n];
	}

	// left + or - 0 is just left
	if (cr && 0 == vr && ('+' == Type[n] || '-' == Type[n])) {
		return Left[n];
	}

	// left * or / 1 is just left
	if (cr && 1 == vr && ('*' == Type[n] || '/' == Type[n])) {
		return Left[n];
	}

	// Either child is 0 and multiply, result is just 0
	if ((cr && 0 == vr || cl && 0 == vl) && '*' == Type[n]) {
		return node('c', 0, 0, 0);
	}
	return n;
}

// Return true if n is a leaf node
int leaf(int n) {
	return n && ('c' == Type[n] || 'v' == Type[n]);
}

int rewrite(int n) {
	int	t;

	if (!n) return 0;
	Left[n] = rewrite(Left[n]);
	Right[n] = rewrite(Right[n]);
	if ('+' == Type[n] || '*' == Type[n]) {
		if (leaf(Left[n]) && !leaf(Right[n])) {
			t = Left[n];
			Left[n] = Right[n];
			Right[n] = t;
		}
	}
	if (	'+' == Type[n] &&
		Right[n] &&
		'-' == Type[Right[n]] &&
		!Right[Right[n]]
	) {
		return node('-', 0, Left[n], Left[Right[n]]);
	}
	if (	'-' == Type[n] &&
		Left[n] &&
		Right[n] &&
		'c' == Type[Left[n]] &&
		0 == Value[Left[n]]
	) {
		return node('-', 0, Right[n], 0);
	}
	if (	'*' == Type[n] &&
		leaf(Left[n]) &&
		Right[n] &&
		Type[Right[n]] == 'c' &&
		Value[Right[n]] == 2
	) {
		return node('+', 0, Left[n], Left[n]);
	}
	return n;
}

// Return true if trees n1 and n2 are equal.
// This means that the Type and Value of this
// node are the same, the left trees are the
// same and the right trees are the same.
int equal(int n1, int n2) {
	// Return true if both n1 and n2 are empty
	if (!n1) return 0 == n2;
	if (!n2) return 0 == n1;

	// False if the Types or Values differ
	if (Type[n1] != Type[n2]) return 0;
	if (Value[n1] != Value[n2]) return 0;

	// Recursively check the left & right sub-trees
	return equal(Left[n1], Left[n2]) &&
		equal(Right[n1], Right[n2]);
}

// Find tree x in larger tree n. Return true if it's
// found, false otherwise.
int find(int x, int n) {
	// No match if n is empty
	if (!n) return 0;

	// Return if x and n are equal, but not
	// the same tree
	if (x != n && equal(x, n)) return 1;

	// Didn't find an exact match, so try finding x
	// in the left or right sub-tree
	return find(x, Left[n]) || find(x, Right[n]);
}

// Return the number of nodes in the tree rooted at n
int size(int n) {
	if (!n) return 0;
	return 1 + size(Left[n]) + size(Right[n]);
}

// Sub will be the biggest common sub-tree found in t
// K will be the size of the biggest common sub-tree found so far
int	Sub, K;

// Traverse t looking for sub-tree n. If not found, try the
// children in n.
void trav2(int t, int n) {
	int	k;

	// Nothing to search for, return
	if (!n) return;

	// If the trees are not identical, there is more than
	// 2 nodes in n, we can find tree n in tree t, and
	// n's size is bigger than the biggest common sub-tree
	// found so far: record n's size and existence
	if (t != n && (k = size(n)) > 2 && find(n, t) && k > K) {
		K = k;
		Sub = n;
	}

	// Didn't find n in t, so try both of n's children
	trav2(t, Left[n]);
	trav2(t, Right[n]);
}

// Given a tree n, find and record the largest common
// sub-tree in N in the Sub variable
int maxduptree(int n) {
	// Start with no known common sub-tree, size 0
	Sub = 0;
	K = 0;

	// Traverse n looking for n or n's children
	trav2(n, n);
	return Sub;
}

// Replace sub-tree x everywhere in n where it occurs
int replace(int x, int n) {
	// Tree n is empty, return
	if (!n) return 0;

	// If both trees are equal, replace the n tree
	// with a variable '@' that represents the
	// pre-calculated value of x. Note that equal()
	// won't be true when x and n are the same tree,
	// so there will be one part of the n tree where
	// x still exists.
	if (equal(x, n)) return node('v', '@', 0, 0);

	// x and n are not equal, so recursively
	// search and replace x in n's two children
	return node(Type[n], Value[n],
			replace(x, Left[n]),
			replace(x, Right[n]));
}

// Replace the biggest common sub-tree in n
int cse(int n) {
	int	csub, t;

	// Find the slot number of the biggest common sub-tree
	csub = maxduptree(n);

	// If we found it
	if (csub) {
		// Recursively replace csub in the n tree
		n = replace(csub, n);

		// Create a variable node which gets
		// assigned the common sub-tree
		t = node('v', '@', 0, 0);
		csub = node('=', 0, t, csub);

		// Finally, glue this assignment tree to the
		// n tree where the common sub-tree has been
		// replaced
		n = node(',', 0, csub, n);
	}
	return n;
}

// Parse the expression in n.
// Perform expression folding with fold().
// Perform strength reduction with rewrite().
// Perform common sub-expression elimination with cse().
// Dump or draw the final AST tree.
void comp(char *s, int d) {
	int	n;

	n = expr(s);
	n = fold(n);
	n = rewrite(n);
	n = cse(n);
	d? draw(n): dump(n, 0);
}

// Turn on drawing if -d is on the command
// line. Then parse the last argument.
int main(int argc, char **argv) {
	int	d = 0;

	if (argc > 1 && !strcmp(argv[1], "-d")) {
		d = 1;
		argc--;
		argv++;
	}
	comp(argc>1? argv[1]: "", d);
	return EXIT_SUCCESS;
}
