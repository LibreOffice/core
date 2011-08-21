# System V R3 GCC compiler specific macro definitions...
#

# Common tool renamings
CC    *:= gcc
"C++" *:= g++
AS    *:= gas
YACC  *:= bison
LEX   *:= flex

# Common flag settings
ARFLAGS *= -rvs
