#*************************************************************************
#*
#*    $Workfile:   makefile.  $
#*
#*    Ersterstellung    TH 01.04.97
#*    Letzte Aenderung  $Author: vg $ $Date: 2003-04-11 17:35:54 $
#*    $Revision: 1.4 $
#*
#*    $Logfile:   T:/sv2/win/source/gdi/makefile.__v  $
#*
#*    Copyright (c) 1990 - 1997, STAR DIVISION
#*
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=vcl
TARGET=salgdi

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  $(PRJ)/util/makefile.pmk

# --- #105371#
CFLAGS += -DWINVER=0x0400
# --- Files --------------------------------------------------------

SLOFILES=   $(SLO)$/salgdi.obj   \
            $(SLO)$/salgdi2.obj  \
            $(SLO)$/salgdi3.obj  \
            $(SLO)$/salvd.obj    \
            $(SLO)$/salprn.obj   \
            $(SLO)$/salbmp.obj	 \
            $(SLO)$/salogl.obj

.IF "$(ENABLE_CTL)" != ""
SLOFILES+=$(SLO)$/winlayout.obj
.ENDIF

.IF "$(GUI)" == "WNT"
SLOFILES+=$(SLO)$/wntgdi.obj
.ENDIF

EXCEPTIONSFILES=	$(SLO)$/salprn.obj

.IF "$(UPDATER)"=="YES"
OBJFILES=   $(OBJ)$/salgdi.obj   \
            $(OBJ)$/salgdi2.obj  \
            $(OBJ)$/salgdi3.obj  \
            $(OBJ)$/salvd.obj    \
            $(OBJ)$/salprn.obj   \
            $(OBJ)$/salbmp.obj	 \
            $(OBJ)$/salogl.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
