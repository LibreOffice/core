PRJ=..$/..

PRJNAME=scripting
TARGET=vbaevents
.IF "$(ENABLE_VBA)"!="YES"
dummy:
        @echo "not building vbaevents..."
.ENDIF

VISIBILITY_HIDDEN=TRUE
NO_BSYMBOLIC=	TRUE
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)
COMPRDB=$(SOLARBINDIR)$/types.rdb

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

#.INCLUDE :  ..$/cppumaker.mk

SLOFILES= \
        $(SLO)$/service.obj \
        $(SLO)$/eventhelper.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX).uno
SHL1IMPLIB=     i$(TARGET)

SHL1VERSIONMAP=$(TARGET).map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
                                                                              
SHL1STDLIBS= \
                $(CPPUHELPERLIB) \
        $(BASICLIB) \
        $(COMPHELPERLIB) \
        $(SFXLIB) \
                $(CPPULIB) \
                $(TOOLSLIB) \
                $(SALLIB)
                                                                              
SHL1DEPN=
SHL1LIBS=$(SLB)$/$(TARGET).lib

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

# ------------------------------------------------------------------

ALLTAR : \
        $(MISC)$/$(TARGET).don \
        $(SLOTARGET)

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(OUT)$/inc -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@
