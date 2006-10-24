PRJ=..$/..
PRJNAME=writerfilter
TARGET=filter
GEN_HID=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION

# --- Files -------------------------------------

SLOFILES=           $(SLO)$/WriterFilter.obj \
                    $(SLO)$/ImportFilter.obj 


# --- Targets ----------------------------------

.INCLUDE : target.mk



