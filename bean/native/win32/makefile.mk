PRJ=..$/..

PRJNAME=odk
TARGET=officebean
ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
NO_BSYMBOLIC=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/com_sun_star_comp_beans_LocalOfficeWindow.obj \
    $(SLO)$/com_sun_star_beans_LocalOfficeWindow.obj

SHL1TARGET=$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
#SHL1IMPLIB=	i$(TARGET)
SHL1STDLIBS=$(AWTLIB) $(SALLIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	$(TARGET).dxp
DEF1DES=officebean

NO_SHL1DESCRIPTION=TRUE

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

