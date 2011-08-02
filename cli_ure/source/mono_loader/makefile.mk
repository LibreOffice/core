PRJ=..$/..

PRJNAME=cli_ure
TARGET=mono_loader

VISIBILITY_HIDDEN=TRUE
NO_BSYMBOLIC=	TRUE
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)
COMPRDB=$(SOLARBINDIR)$/types.rdb

.IF "$(ENABLE_MONO)" != "YES"
dummy:
     @echo "Mono binding disabled - skipping ..."
.ELSE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
CFLAGS+=$(MONO_CFLAGS)
DLLPRE =

# ------------------------------------------------------------------

#.INCLUDE :  ..$/cppumaker.mk

SLOFILES= \
		$(SLO)$/service.obj \
		$(SLO)$/mono_loader.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX).uno
SHL1IMPLIB=     i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
                                                                              
SHL1STDLIBS= \
                $(CPPUHELPERLIB) \
                $(CPPULIB) \
                $(SALLIB)

SHL1STDLIBS+=$(MONO_LIBS)
                                                                              
SHL1DEPN=
SHL1LIBS=$(SLB)$/$(TARGET).lib

.ENDIF
# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
