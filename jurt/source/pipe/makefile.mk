PRJ=..$/..

PRJNAME=jurt
TARGET=jpipe
ENABLE_EXCEPTIONS=TRUE

#?
NO_DEFAULT_STL=TRUE
NO_BSYMBOLIC=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/com_sun_star_lib_connections_pipe_PipeConnection.obj

SHL1TARGET=$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1STDLIBS=$(SALLIB)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(TARGET).dxp
DEF1DES=jurtpipe

NO_SHL1DESCRIPTION=TRUE

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

