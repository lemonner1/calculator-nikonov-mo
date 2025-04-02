#include "main.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int flag_float = 0;

typedef union {
   long int_value;
   double float_value;
} Number;

typedef struct {
   Number data[STACK_SIZE];
   int top;
} Stack;

void initStack(Stack* stack) { stack->top = -1; }
int isStackEmpty(Stack* stack) { return stack->top == -1; }
int isStackFull(Stack* stack) { return stack->top == STACK_SIZE - 1; }

void push(Stack* stack, Number item) {
   if (isStackFull(stack)) {
      fprintf(stderr, "Stack overflow!\n");
      exit(5);
   }
   stack->data[++stack->top] = item;
}

Number pop(Stack* stack) {
   if (isStackEmpty(stack)) {
      fprintf(stderr, "Stack underflow!\n");
      exit(5);
   }
   return stack->data[stack->top--];
}

Number peek(Stack* stack) {
   if (isStackEmpty(stack)) {
      fprintf(stderr, "Stack is empty!\n");
      exit(5);
   }
   return stack->data[stack->top];
}

void validateNumber(Number num) {
   double value = flag_float ? num.float_value : num.int_value;
   if (value > 2000000000 || value < -2000000000) {
      fprintf(stderr, "Number out of range!\n");
      exit(4);
   }
}

int getPriority(char op) {
   return (op == '+' || op == '-') ? 1 : (op == '*' || op == '/') ? 2 : 0;
}

Number compute(Number a, Number b, char op) {
   Number res = { 0 };
   if (flag_float) {
      switch (op) {
      case '+': res.float_value = a.float_value + b.float_value; break;
      case '-': res.float_value = a.float_value - b.float_value; break;
      case '*': res.float_value = a.float_value * b.float_value; break;
      case '/':
         if (b.float_value > -1e-4 && b.float_value < 1e-4) {
            fprintf(stderr, "Division by zero!\n");
            exit(1);
         }
         res.float_value = a.float_value / b.float_value;
         break;
      }
   }
   else {
      switch (op) {
      case '+': res.int_value = a.int_value + b.int_value; break;
      case '-': res.int_value = a.int_value - b.int_value; break;
      case '*': res.int_value = a.int_value * b.int_value; break;
      case '/':
         if (b.int_value == 0) {
            fprintf(stderr, "Division by zero!\n");
            exit(1);
         }
         res.int_value = a.int_value / b.int_value;
         break;
      }
   }
   validateNumber(res);
   return res;
}

Number evaluateExpression(char* input) {
   Stack values, ops;
   initStack(&values);
   initStack(&ops);

   for (int i = 0; input[i] != '\0'; i++) {
      if (isspace(input[i])) continue;

      if (isdigit(input[i])) {
         Number num = { 0 };
         char* endptr;
         if (flag_float)
            num.float_value = strtod(&input[i], &endptr);
         else
            num.int_value = strtol(&input[i], &endptr, 10);
         i = endptr - input - 1;
         validateNumber(num);
         push(&values, num);
      }
      else if (input[i] == '(') {
         Number op = { .int_value = '(' };
         push(&ops, op);
      }
      else if (input[i] == ')') {
         while (!isStackEmpty(&ops) && peek(&ops).int_value != '(') {
            Number b = pop(&values);
            Number a = pop(&values);
            push(&values, compute(a, b, (char)pop(&ops).int_value));
         }
         pop(&ops);
      }
      else {
         while (!isStackEmpty(&ops) && getPriority((char)peek(&ops).int_value) >= getPriority(input[i])) {
            Number b = pop(&values);
            Number a = pop(&values);
            push(&values, compute(a, b, (char)pop(&ops).int_value));
         }
         Number op = { .int_value = input[i] };
         push(&ops, op);
      }
   }

   while (!isStackEmpty(&ops)) {
      Number b = pop(&values);
      Number a = pop(&values);
      push(&values, compute(a, b, (char)pop(&ops).int_value));
   }
   return pop(&values);
}

int validateInput(char* input) {
   int len = strlen(input), balance = 0, last_was_op = 1;
   for (int i = 0; i < len; i++) {
      if (isspace(input[i])) continue;
      if (isdigit(input[i])) {
         if (!last_was_op) return 2;
         while (i < len && isdigit(input[i])) i++;
         i--;
         last_was_op = 0;
      }
      else if (strchr("+-*/", input[i])) {
         if (last_was_op) return 2;
         last_was_op = 1;
      }
      else if (input[i] == '(') {
         if (!last_was_op) return 2;
         balance++;
         last_was_op = 1;
      }
      else if (input[i] == ')') {
         if (last_was_op || balance == 0) return 2;
         balance--;
         last_was_op = 0;
      }
      else return 2;
   }
   return (last_was_op || balance) ? 2 : 1;
}

#ifndef GTEST
int main(int argc, char** argv) {
   for (int i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "--float")) flag_float = 1;
   }

   char input[1024];
   if (!fgets(input, sizeof(input), stdin)) {
      fprintf(stderr, "Error reading input!\n");
      return 2;
   }

   if (validateInput(input) != 1) {
      fprintf(stderr, "Invalid input!\n");
      return 2;
   }

   Number result = evaluateExpression(input);
   printf(flag_float ? "%.4lf\n" : "%ld\n", flag_float ? result.float_value : result.int_value);
   return 0;
}
#endif