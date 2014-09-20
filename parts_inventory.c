#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXNAME 12
#define MAXPARTS 10
#define MAXDIGITS 4

#define INTERNAL 1
#define EXTERNAL 0

static int debug = 0;
FILE* debug_file;

struct entry {
   int ct;
   struct node* nd;
};

struct internal_node {
   int entries;
   struct entry parts[MAXPARTS];
};

struct node {
   char name[MAXNAME];
   int internal;
   union {
      struct internal_node nd_i;
      float cost;
   } *data;
};

struct node* setup_tree(char* namestring);

float calculate_cost(struct node* node, int ct) {
   if (node->internal) { /* if internal node */
      float sum = 0.0;
      int i;

      for (i = 0; i < node->data->nd_i.entries; i++)
         sum += calculate_cost(node->data->nd_i.parts[i].nd, node->data->nd_i.parts[i].ct);

      if (debug) 
         fprintf(debug_file, "\n[DEBUG] calculate_cost -- %s: returning %f\n\n", node->name, ct * sum);

      return (ct * sum);
   }

   else {
      if (debug)
         fprintf(debug_file, "\n[DEBUG] calculate_cost -- %s: returning %f\n\n", node->name, ct * node->data->cost);
      return (ct * node->data->cost);
   }
}

int is_internal(char* line_buffer) {
   if ((strchr(line_buffer, '*')) != NULL)
      return 1;
   return 0;
}

int get_entries(char* line, struct entry* parts) {
   int index = 0;

   char name_buffer[MAXNAME], num_buffer[MAXDIGITS];

   int i = 0;
   int entries = 0;

   /* my hacky parsing solution */
   while (1) {
      while (isspace(line[index])) 
         index++; // skip whitespace
 
      i = 0;     
      while (isdigit(line[index]))
         num_buffer[i++] = line[index++]; // copy number
      num_buffer[i] = '\0';
      
      while (line[index] != '*') {
         if (line[index] == '\0')
            return entries;
         index++; // skip to '*' character
      }

      index++; // skip over '*' character
      
      while (isspace(line[index]))
         index++; //skip whitespace
      
      i = 0;
      while (isalpha(line[index]))
         name_buffer[i++] = line[index++]; // copy name
      name_buffer[i] = '\0';

      if ((strlen(num_buffer) == 0) || (strlen(name_buffer) == 0))
         return entries;

      if (debug) {
         fprintf(debug_file, "\n[DEBUG] num_buffer contains string %s\n", num_buffer);
         fprintf(debug_file, "[DEBUG] name_buffer contains string %s\n\n", name_buffer);
      }

      parts->ct = atoi(num_buffer); // store in data structure
      parts->nd = setup_tree(name_buffer);

      entries++;
      parts++;

      while(line[index] != '+') {
         if (line[index] == '\0')
            return entries;
         index++;
      }
      index++; // skip over '+' character
   }
   return entries;
}

struct node* setup_tree(char* namestring) {
   char line_buffer[80];

   struct node* treenode = malloc (sizeof(struct node));
   strncpy (treenode->name, namestring, strlen(namestring));

   printf("what is %s?\n", namestring);
   
   fgets(line_buffer, 80, stdin);
   *(strchr(line_buffer, '\n')) = '\0';

   if (is_internal(line_buffer)) {
      treenode->internal = 1;

      treenode->data = malloc(sizeof(struct internal_node));

      treenode->data->nd_i.entries = get_entries(line_buffer, &(treenode->data->nd_i.parts));
   }

   else {
      float cost;

      treenode->data = malloc(sizeof(int));
      treenode->internal = 0;

      sscanf(line_buffer, "%f", &cost);

      if (debug) {
         fprintf(debug_file, "\n[DEBUG] line_buffer contains string \'%s\'\n", line_buffer);      
         fprintf(debug_file, "[DEBUG] cost contains value %f\n\n", cost);
      }

      treenode->data->cost = cost;
   }

   return treenode;
}     

void print_tree(struct node* root)
{
   int i;

   if (root->internal) {
      fprintf(debug_file, "\nname: %s, address: %p,\n", root->name, root->name);
      fprintf(debug_file, "internal: %d, &internal: %p\n", root->internal, &root->internal);
      
      fprintf(debug_file, "entries: %d &entries: %p\n", root->data->nd_i.entries, &root->data->nd_i.entries); 
      for (i = 0; i < root->data->nd_i.entries; i++)
         fprintf(debug_file, "entry #: %d\nct: %d, &ct: %p\nnd: %p, &nd: %p\n", i+1, root->data->nd_i.parts[i].ct, &root->data->nd_i.parts[i].ct, root->data->nd_i.parts[i].nd, &root->data->nd_i.parts[i].nd);
      for (i = 0; i < root->data->nd_i.entries; i++)
         print_tree(root->data->nd_i.parts[i].nd);
   }

   else {
      fprintf(debug_file, "\nname: %s, %p, &name: %p\n", root->name, root->name, &root->name);
      fprintf(debug_file, "internal: %d, &internal: %p\n", root->internal, &root->internal);
      fprintf(debug_file, "cost: %f, &cost: %p\n", root->data->cost, &root->data->cost);
   }
}

void free_tree(struct node* root)
{
   if (root->internal) {
      int i;
      for (i = 0; i < root->data->nd_i.entries; i++)
         free_tree(root->data->nd_i.parts[i].nd);   
   }

   if (debug)
      fprintf(debug_file, "freeing %s, %p, data @ %p\n", root->name, root, root->data);
   free(root->data);
   free(root);
}

int main(int argc, char** argv)
{
   if (argc > 2)
      if (strcmp(argv[1], "-d") == 0) {
         debug = 1;
         debug_file = fopen(argv[2], "w");
      }

   char namestring[MAXNAME];

   printf("what will you define?\n");

   fgets(namestring, MAXNAME, stdin);
   *(strchr(namestring, '\n')) = '\0';

   struct node* root = setup_tree(namestring);

   if (debug)
      print_tree(root);

   printf("total cost for %s is %.2f\n", namestring, calculate_cost(root, 1));

   free_tree(root);

   if (debug)
      fclose(debug_file);

   return 0;
}
