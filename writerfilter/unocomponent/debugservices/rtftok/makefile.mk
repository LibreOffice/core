#
#  Copyright 2005 Sun Microsystems, Inc.
#
PRJ=..$/..$/..
PRJNAME=writerfilter
TARGET=debugservices_rtftok
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=\
    $(SLO)$/ScannerTestService.obj \
    $(SLO)$/XMLScanner.obj 
#$(SLO)$/TestService.obj $(SLO)$/ResolverTestService.obj $(SLO)$/XmlParserTestService.obj $(SLO)$/RngService.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
