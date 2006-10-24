#
#  Copyright 2005 Sun Microsystems, Inc.
#

PRJ=..
PRJNAME=writerfilter
TARGET=odiapi
TARGET2=writerfilter
#LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
#USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION

# --- Files --------------------------------------------------------

#SLOFILES=$(SLO)$/PropertiesImpl.obj	

LIB1TARGET=$(SLB)$/godiapi.lib
LIB1FILES=	\
    $(SLB)$/xxml.lib	\
    $(SLB)$/sl.lib	\
    $(SLB)$/props.lib	\
    $(SLB)$/qname.lib#\
#	$(SLB)$/core.lib


SHL1TARGET=$(TARGET)
SHL1LIBS=$(SLB)$/godiapi.lib
SHL1STDLIBS=$(SALLIB) ${LIBXML2LIB} $(CPPULIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(UCBHELPERLIB)
SHL1IMPLIB=i$(SHL1TARGET)
SHL1USE_EXPORTS=ordinal
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=godiapi

LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2FILES=  \
    $(SLB)$/doctok.lib \
    $(SLB)$/dmapper.lib \
    $(SLB)$/filter.lib

SHL2LIBS=$(SLB)$/$(TARGET2).lib


SHL2TARGET=$(TARGET2)$(UPD)$(DLLPOSTFIX)
SHL2STDLIBS=\
    $(I18NISOLANGLIB) \
    $(CPPUHELPERLIB)    \
    $(COMPHELPERLIB)    \
    $(CPPULIB)          \
    $(SALLIB)


SHL2DEPN=
SHL2IMPLIB= i$(SHL2TARGET)
#SHL2LIBS=   $(SLB)$/$(TARGET2).lib
SHL2DEF=    $(MISC)$/$(SHL2TARGET).def
SHL2VERSIONMAP=exports.map

DEF2NAME=$(SHL2TARGET)


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


