#*************************************************************************
#*
#*	  $Workfile:   makefile.  $
#*
#*	  Ersterstellung	TH 01.04.97
#*	  Letzte Aenderung	$Author: ssa $ $Date: 2002-11-20 12:36:25 $
#*	  $Revision: 1.2 $
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

# --- #105371#
CFLAGS += -DWINVER=0x0400

# --- Files --------------------------------------------------------

SLOFILES=	$(SLO)$/salframe.obj	\
            $(SLO)$/salobj.obj

.IF "$(UPDATER)"=="YES"
OBJFILES=	$(OBJ)$/salframe.obj	\
            $(OBJ)$/salobj.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
