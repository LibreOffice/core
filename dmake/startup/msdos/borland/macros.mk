# MSDOS Borland-C environment customization.

.IF $(OSENVIRONMENT)
   .INCLUDE .IGNORE .NOINFER : $(INCFILENAME:d)$(OSENVIRONMENT)$/macros.mk
.ENDIF

# Standard C-language command names and flags
CPP	   *:=			# C-preprocessor
CFLAGS     *=			# C   compiler flags
"C++"      *:=			# C++ Compiler
"C++FLAGS" *=			# C++ Compiler flags

AS         *:= tasm		# Assembler and flags
ASFLAGS    *= 
LD         *= tlink		# Loader and flags
LDFLAGS    *=
LDLIBS     *=			# Default libraries
AR         *:= tlib		# archiver
ARFLAGS    *= ????

# Definition of Print command for this system.
PRINT      *= print

# Language and Parser generation Tools and their flags
YACC	   *:= yacc		# standard yacc
YFLAGS     *=
LEX	   *:= lex		# standard lex
LFLAGS     *=

# Other Compilers, Tools and their flags
PC	   *:= tpc		# pascal compiler
RC	   *:= ???		# ratfor compiler
FC	   *:= ???		# fortran compiler
