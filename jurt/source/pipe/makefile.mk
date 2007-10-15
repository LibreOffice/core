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

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building jurt because Java is disabled"
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/com_sun_star_lib_connections_pipe_PipeConnection.obj

SHL1TARGET=$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1STDLIBS=$(SALLIB)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=URELIB

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(TARGET).dxp
DEF1DES=jurtpipe

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

