# MSDOS Zortech-C environment customization.

# Standard C-language command names and flags
CC         *:= ztc		# C   compiler 
CPP	   *:=			# C-preprocessor
CFLAGS     *=			# C   compiler flags
"C++"      *:=			# C++ Compiler
"C++FLAGS" *=			# C++ Compiler flags

AS         *:= masm		# Assembler and flags
ASFLAGS    *= 
LD         *= blink		# Loader and flags
LDFLAGS    *=
LDLIBS     *=			# Default libraries
AR         *:= ????		# archiver
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
