# Define Atari TOS specific macros.
#

# Process environment-specific refinements, if any.
.IF $(OSENVIRONMENT)
   .INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OSENVIRONMENT)$/macros.mk
.ENDIF

# Set default to GCC Compiler.
CPP   *:= /gnu/lib/cpp
CC    *:= gcc
"C++" *:= g++
AS    *:= gas
YACC  *:= bison
LEX   *:= flex
RM    *:= /bin/rm

# Common flag settings
ARFLAGS *= -rvs

# Other appropriate macro settings.
A            *:= .olb
SHELLFLAGS   *:=
GROUPSUFFIX  *:= .bat
