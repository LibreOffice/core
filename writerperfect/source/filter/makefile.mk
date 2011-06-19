PRJ=..$/..

PRJNAME=writerperfect
TARGET=filter
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(LIBWPD_CFLAGS)
.ENDIF

.IF "$(SYSTEM_LIBWPG)" == "YES"
INCPRE+=$(LIBWPG_CFLAGS)
.ENDIF

.IF "$(SYSTEM_LIBWPS)" == "YES"
INCPRE+=$(LIBWPS_CFLAGS)
.ENDIF

# broken but ... necessary, internal include shafted ...
INCPRE+= -I..

SLOFILES= \
    $(SLO)$/DocumentElement.obj \
    $(SLO)$/DocumentHandler.obj \
    $(SLO)$/FontStyle.obj \
    $(SLO)$/InternalHandler.obj \
    $(SLO)$/ListStyle.obj \
    $(SLO)$/OdgGenerator.obj \
    $(SLO)$/OdtGenerator.obj \
    $(SLO)$/PageSpan.obj \
    $(SLO)$/SectionStyle.obj \
    $(SLO)$/TableStyle.obj \
    $(SLO)$/TextRunStyle.obj

.INCLUDE :  target.mk
