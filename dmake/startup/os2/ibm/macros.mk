# OS/2 1.3 and 2.1 specific customization.

# Standard C-language command names and flags
CC         *:= icc		# C   compiler 
CPP	   *:=			# C-preprocessor
CFLAGS     *=			# C   compiler flags
"C++"      *:=			# C++ Compiler
"C++FLAGS" *=			# C++ Compiler flags

AS         *:= masm		# Assembler and flags
ASFLAGS    *= 
LD         *= link386		# Loader and flags
LDFLAGS    *=
LDLIBS     *=			# Default libraries
AR         *:= lib		# archiver
ARFLAGS    *= ????

# Definition of Print command for this system.
PRINT      *= print

# Language and Parser generation Tools and their flags
YACC	   *:= yacc		# standard yacc
YFLAGS     *=
LEX	   *:= lex		# standard lex
LFLAGS     *=

# Other Compilers, Tools and their flags
PC	   *:= ???		# pascal compiler
RC	   *:= ???		# ratfor compiler
FC	   *:= ???		# fortran compiler
