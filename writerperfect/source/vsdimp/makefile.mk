PRJ=..$/..

PRJNAME=writerperfect
TARGET=vsdimp
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(LIBWPD_CFLAGS)
.ENDIF

.IF "$(SYSTEM_LIBWPG)" == "YES"
INCPRE+=$(LIBWPG_CFLAGS)
.ENDIF

.IF "$(SYSTEM_LIBVISIO)" == "YES"
INCPRE+=$(LIBVISIO_CFLAGS)
.ENDIF

# broken but ... necessary, internal include shafted ...
INCPRE+= -I..

SLOFILES= \
    $(SLO)$/VisioImportFilter.obj \
    $(SLO)$/visioimport_genericfilter.obj

.INCLUDE :  target.mk
