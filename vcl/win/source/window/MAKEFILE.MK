#*************************************************************************
#*
#*	  $Workfile:   makefile.  $
#*
#*	  Ersterstellung	TH 01.04.97
#*	  Letzte Aenderung	$Author: hr $ $Date: 2000-09-18 17:05:50 $
#*	  $Revision: 1.1.1.1 $
#*
#*	  $Logfile:   T:/sv2/win/source/window/makefile.__v  $
#*
#*	  Copyright (c) 1990 - 1998, STAR DIVISION
#*
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=vcl
TARGET=salwin

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

# --- Files --------------------------------------------------------

SLOFILES=	$(SLO)$/salframe.obj	\
            $(SLO)$/salobj.obj

.IF "$(UPDATER)"=="YES"
OBJFILES=	$(OBJ)$/salframe.obj	\
            $(OBJ)$/salobj.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
