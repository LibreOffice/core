#*************************************************************************
#*
#*	  $Workfile:   makefile.  $
#*
#*	  Ersterstellung	TH 01.04.97
#*	  Letzte Aenderung	$Author: hr $ $Date: 2000-09-18 17:05:49 $
#*	  $Revision: 1.1.1.1 $
#*
#*	  $Logfile:   T:/sv2/win/source/app/makefile.__v  $
#*
#*	  Copyright (c) 1990 - 2000, STAR DIVISION
#*
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=vcl
TARGET=salapp

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

# --- Files --------------------------------------------------------

OBJFILES=	$(OBJ)$/salmain.obj

SLOFILES=	$(SLO)$/salshl.obj		\
            $(SLO)$/saldata.obj 	\
            $(SLO)$/salinst.obj 	\
            $(SLO)$/saltimer.obj	\
            $(SLO)$/salsound.obj	\
            $(SLO)$/salinfo.obj 	\
            $(SLO)$/salsys.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
