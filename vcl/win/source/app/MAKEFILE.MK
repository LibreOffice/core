#*************************************************************************
#*
#*    $Workfile:   makefile.  $
#*
#*    Ersterstellung    TH 01.04.97
#*    Letzte Aenderung  $Author: th $ $Date: 2001-07-25 11:10:43 $
#*    $Revision: 1.2 $
#*
#*    $Logfile:   T:/sv2/win/source/app/makefile.__v  $
#*
#*    Copyright (c) 1990 - 2000, STAR DIVISION
#*
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=vcl
TARGET=salapp

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

OBJFILES=   $(OBJ)$/salmain.obj

SLOFILES=   $(SLO)$/salshl.obj      \
            $(SLO)$/saldata.obj     \
            $(SLO)$/salinst.obj     \
            $(SLO)$/saltimer.obj    \
            $(SLO)$/salsound.obj    \
            $(SLO)$/salinfo.obj

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
