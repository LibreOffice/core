#*************************************************************************
#*
#*	  $Workfile:   makefile  $
#*
#*	  Ersterstellung	JSC 24.06.99
#*	  Letzte Aenderung	$Author: kr $ $Date: 2000-11-10 09:50:30 $
#*	  $Revision: 1.1 $
#*
#*	  $Logfile:$
#*
#*	  Copyright (c) 1990-1994, STAR DIVISION
#*
#*************************************************************************

PRJ=..$/..

PRJNAME=remotebridges
TARGET=dynamicloader
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

CPPUMAKERFLAGS += -C
UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb 

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc

# adding to inludepath
INCPRE+=$(UNOUCROUT)


SLOFILES= \
    $(SLO)$/dynamicloader.obj 

SHL1TARGET= $(TARGET)

SHL1STDLIBS=\
        $(CPPUHELPERLIB) 	\
        $(CPPULIB)			\
        $(SALLIB)

SHL1IMPLIB=	i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib 
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
