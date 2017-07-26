#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TYPE_INT   'i'
#define TYPE_FLOAT 'f'
#define TYPE_STR   's'

typedef struct avl {
    unsigned long key;
    void *object;
    struct avl *left;
    struct avl *right;
    unsigned long height;
    char type;
} avl;


#define max(a, b) ((a) > (b)) ? (a) : (b)

avl *
new_node(void *object, unsigned long key, char type)
{
    avl *node = (avl *)malloc(sizeof(avl));
    node->key = key;
    node->object = object;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;  // new node is always first at leaf
    node->type = type;
    return node;
}

long
height(avl *node)
{
    if (node == NULL) return 0;
    return node->height;
}

int
balnce(avl *node)
{
    return height(node->left) - height(node->right);
}

// rotate right subtree rooted with y
avl *
right_rotate(avl *y)
{
    avl *x = y->left;
    avl *t2 = x->right;

    // rotate right
    x->right = y;
    y->left = t2;

    // update heights
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

// rotate left subtree rooted with x
avl *
left_rotate(avl *x)
{
    avl *y = x->right;
    avl *t2 = y->left;

    // rotate left
    y->left = x;
    x->right = t2;

    // update heights
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

// insert
avl *
avl_insert(avl *node, void *object, unsigned long key, char type)
{
    // normal BST insert
    if (node == NULL)
        return new_node(object, key, type);
    if (key < node->key)
        node->left  = avl_insert(node->left, object, key, type);
    else
        node->right = avl_insert(node->right, object, key, type);

    // update height of parent
    node->height = max(height(node->left), height(node->right)) + 1;

    // get balance and update if needed
    int balance = height(node->left) - height(node->right);

    // left-left Case
    if (balance > 1 && key < node->left->key)
        return right_rotate(node);

    // right-right Case
    if (balance < -1 && key > node->right->key)
        return left_rotate(node);

    // left-right
    if (balance > 1 && key > node->left->key) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    // right-left
    if (balance < -1 && key < node->right->key) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

avl *min_value_node(avl *node) {
    avl *current = node;
    while (current->left != NULL)
        current = current->left;
    return current;
}

avl *
avl_delete_node(avl *root, unsigned long key)
{
    if (root == NULL)
        return root;

    // if the key to be deleted is smaller than root's key,
    // it exists in left subtree
    if (key < root->key)
        root->left = avl_delete_node(root->left, key);

    // or, if the key to be deleted is greater than root's key,
    // it exists in right subtree
    else if (key > root->key)
        root->right = avl_delete_node(root->right, key);

    // or, if key is same as root's key, delete this node
    else {
        // node with only one or no child
        if( (root->left == NULL) || (root->right == NULL) ) {
            avl *temp = root->left ? root->left : root->right;
            // no child
            if (temp == NULL) {
                temp = root;
                root = NULL;
            }
            // one child
            else
                *root = *temp; // copy contents of non-empty child
            free(temp->object);
            free(temp);
        }
        else {
            // node with two children: Get the inorder successor (smallest
            // in the right subtree)
            avl* temp = min_value_node(root->right);
            // copy the inorder successor's data to this node
            root->key = temp->key;
            // delete the inorder successor
            root->right = avl_delete_node(root->right, temp->key);
        }
    }

    if (root == NULL)
      return root;

    // update height of current node
    root->height = max(height(root->left), height(root->right)) + 1;

    // handle four cases if it becomes unbalanced
    int balance = balnce(root);
    if (balance > 1) {
        // left-left
        if (balnce(root->left) >= 0)
            return right_rotate(root);

        // left-right
        else {
            root->left = left_rotate(root->left);
            return right_rotate(root);
        }
    } else if (balance < -1) {
        // left-left
        if (balnce(root->right) <= 0)
            return left_rotate(root);

        // right-left
        else {
            root->right = right_rotate(root->right);
            return left_rotate(root);
        }
    }

    return root;
}

avl *search(avl *root, unsigned long key) {
    if (root == NULL || root->key == key)
       return root;

    if (root->key < key)
       return search(root->right, key);

    return search(root->left, key);
}

#define HASH_KEY_OPTION 1
#define INSERT_OPTION   2
#define SEARCH_OPTION   3
#define REMOVE_OPTION   4
#define INORDER_OPTION  5

#define MAX_STR 20

unsigned long
hash(unsigned char *bytes, size_t len)
{
    unsigned long hash;

    hash = 5381;
    while (len--)
        hash = ((hash << 5) + hash) + *bytes++; // hash * 33 + c
    return hash;
}

void *
input_object(char *type)
{
    void *object;
    int *i;
    double *d;
    char *str;
    char s[MAX_STR];

    type_menu:
    printf("  Object type:\n");
    printf("  %c - Integer\n", TYPE_INT);
    printf("  %c - Float\n", TYPE_FLOAT);
    printf("  %c - String [20 chars]\n", TYPE_STR);
    printf("  : ");
    scanf("%s", s);
    *type = *s;
    switch (*s) {
        case TYPE_INT:
            i = (int *)malloc(sizeof(int));
            printf("  : ");
            scanf("%d", i);
            object = (void *)i;
            break;
        case TYPE_FLOAT:
            d = (double *)malloc(sizeof(float));
            printf("  : ");
            scanf("%lf", d);
            object = (void *)d;
            break;
        case TYPE_STR:
            str = (char *)malloc(sizeof(char) * MAX_STR);
            printf("  : ");
            scanf("%s", str);
            object = (void *)str;
            break;
        default:
            goto type_menu;
            break;
    }
    return object;
}

unsigned long
input_key()
{
    int i;
    double d;
    char c;
    char s[MAX_STR];
    unsigned long hashed_key;

    type_menu:
    printf("  Key type:\n");
    printf("  %c - Integer\n", TYPE_INT);
    printf("  %c - Float\n", TYPE_FLOAT);
    printf("  %c - String [20 chars]\n", TYPE_STR);
    printf("  : ");
    scanf("%s", s);
    switch (*s) {
        case TYPE_INT:
            printf("  : ");
            scanf("%d", &i);
            hashed_key = hash((unsigned char *)&i, sizeof(i));
            break;
        case TYPE_FLOAT:
            printf("  : ");
            scanf("%lf", &d);
            hashed_key = hash((unsigned char *)&d, sizeof(d));
            break;
        case TYPE_STR:
            printf("  : ");
            scanf("%s", s);
            hashed_key = hash((unsigned char *)&s, strlen(s));
            break;
        default:
            goto type_menu;
            break;
    }
    return hashed_key;
}

void print_node(avl *node) {
    if (node == NULL) return;
    else if (node->type == TYPE_INT) printf("Object = %d\n", *(int *)node->object);
    else if (node->type == TYPE_FLOAT) printf("Object = %lf\n", *(double *)node->object);
    else if (node->type == TYPE_STR) printf("Object = %s\n", (char *)node->object);
}

void print_inorder(avl *node) {
    if (node == NULL) return;
    print_inorder(node->left);
    printf("%ld\n", node->key);
    print_inorder(node->right);
}

int
main()
{
    int option;
    unsigned long hashed_key;
    void *object;
    avl *node;
    avl *snode;
    char choosen_type;

    node = NULL;
    printf("C Dictionary using AVL Tree.\n\n");
    menu:
    printf("%d - Hash key\n", HASH_KEY_OPTION);
    printf("%d - Insert\n", INSERT_OPTION);
    printf("%d - Search\n", SEARCH_OPTION);
    printf("%d - Remove\n", REMOVE_OPTION);
    printf("%d - Inorder\n", INORDER_OPTION);
    printf(": ");
    scanf("%d", &option);
    if (option == HASH_KEY_OPTION) {
        hashed_key = input_key();
        printf("  Hashed key: %ld\n", hashed_key);
    } else if (option == INSERT_OPTION) {
        object = input_object(&choosen_type);
        hashed_key = input_key();
        node = avl_insert(node, object, hashed_key, choosen_type);
    } else if (option == SEARCH_OPTION) {
        hashed_key = input_key();
        snode = search(node, hashed_key);
        if (snode == NULL) printf("Object does not exist.\n");
        else print_node(snode);
    } else if (option == REMOVE_OPTION) {
        hashed_key = input_key();
        node = avl_delete_node(node, hashed_key);
    } else if (option == INORDER_OPTION)
        print_inorder(node);
    printf("\n");
    goto menu;
    return 0;
}
