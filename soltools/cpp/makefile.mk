#*************************************************************************
#*
#*    Makefile fuer Cpp    NP 23.12.1998
#*
#*************************************************************************

PRJ=..

PRJNAME=soltools
TARGET=cpp
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/_cpp.obj		\
    $(OBJ)$/_eval.obj 	\
    $(OBJ)$/_getopt.obj	\
    $(OBJ)$/_include.obj \
    $(OBJ)$/_lex.obj 	\
    $(OBJ)$/_macro.obj 	\
    $(OBJ)$/_mcrvalid.obj \
    $(OBJ)$/_nlist.obj 	\
    $(OBJ)$/_tokens.obj 	\
    $(OBJ)$/_unix.obj

APP1TARGET	=	$(TARGET)
APP1STACK	=	1000000
APP1OBJS	=   $(OBJ)$/_cpp.obj
APP1STDLIBS	= 	$(STDLIBCUIMT)
APP1LIBS	=	$(LB)$/$(TARGET).lib
APP1DEPN	=   $(LB)$/$(TARGET).lib

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



