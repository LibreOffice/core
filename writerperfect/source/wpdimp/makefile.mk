PRJ=..$/..

PRJNAME=writerperfect
TARGET=wpdimp
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
    $(SLO)$/WordPerfectCollector.obj  \
    $(SLO)$/WordPerfectImportFilter.obj  \
    $(SLO)$/wpft_genericfilter.obj		

.INCLUDE :  target.mk
