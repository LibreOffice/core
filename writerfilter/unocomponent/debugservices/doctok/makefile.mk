#
#  Copyright 2005 Sun Microsystems, Inc.
#
PRJ=..$/..$/..
PRJNAME=writerfilter
TARGET=debugservices_doctok
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES= \
    $(SLO)$/DocTokTestService.obj \
    $(SLO)$/DocTokAnalyzeService.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
