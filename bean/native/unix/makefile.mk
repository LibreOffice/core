PRJ=..$/..

PRJNAME=beans
TARGET=officebean
ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(SOLAR_JAVA)"=="" || "$(OS)"=="MACOSX"
nojava:
    @echo "Not building odk/source/OOSupport because Java has been disabled"
.ENDIF
.IF "$(JDK)"=="gcj"
all:
        @echo This dir cannot be build with gcj because of jawt.h
.ENDIF
.IF "$(OS)"=="MACOSX"
dummy:
    @echo "Nothing to build for OS $(OS)"
.ENDIF

SLOFILES = \
    $(SLO)$/com_sun_star_comp_beans_LocalOfficeWindow.obj \
    $(SLO)$/com_sun_star_beans_LocalOfficeWindow.obj

SHL1TARGET=$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1STDLIBS=-ljawt

.IF "$(OS)" == "LINUX"
.IF "$(CPUNAME)" == "S390"
SHL1STDLIBS+=-lzip -lawt
.ENDIF
SHL1STDLIBS+=-lstdc++
.ENDIF

NO_SHL1DESCRIPTION=TRUE

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
