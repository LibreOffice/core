#
#  Copyright 2005 Sun Microsystems, Inc.
#
PRJ=..
PRJNAME=writerfilter
TARGET=writerfilter.uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/component.obj

SHL1TARGET=$(TARGET)

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
ODIAPILIB=-lodiapi
RTFTOKLIB=-lrtftok
DOCTOKLIB=-ldoctok
.ELIF "$(GUI)"=="WNT"
ODIAPILIB=$(LB)$/iodiapi.lib
RTFTOKLIB=$(LB)$/irtftok.lib
DOCTOKLIB=$(LB)$/idoctok.lib
.ENDIF

SHL1STDLIBS=$(SALLIB)\
    $(CPPULIB)\
    $(COMPHELPERLIB)\
    $(CPPUHELPERLIB)\
    $(UCBHELPERLIB)\
    $(ODIAPILIB) \
    $(RTFTOKLIB) \
    $(DOCTOKLIB)

SHL1LIBS=\
    $(SLB)$/debugservices_xxml.lib \
    $(SLB)$/debugservices_rtftok.lib \
    $(SLB)$/debugservices_odsl.lib \
    $(SLB)$/debugservices_rtfsl.lib \
    $(SLB)$/debugservices_doctok.lib

SHL1IMPLIB=i$(SHL1TARGET)

SHL1OBJS = $(SLO)$/component.obj

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

