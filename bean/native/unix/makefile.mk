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
.IF "$(OS)"=="MACOSX"
dummy:
    @echo "Nothing to build for OS $(OS)"
.ENDIF

SLOFILES = \
    $(SLO)$/com_sun_star_comp_beans_LocalOfficeWindow.obj \
    $(SLO)$/com_sun_star_beans_LocalOfficeWindow.obj

SHL1TARGET=$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SOLARLIB+=$(JAVALIB)
SHL1STDLIBS=$(AWTLIB)
SHL1NOCHECK=TRUE

.IF "$(OS)" == "LINUX"
SHL1STDLIBS+=-lstdc++
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
