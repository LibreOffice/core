PRJ=..$/..

PRJNAME=writerperfect
TARGET=wpdimp
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(LIBWPD_CFLAGS) -I..
.ELSE
# broken but ... necessary, internal include shafted ...
INCPRE+=-I..
.ENDIF

SLOFILES= \
    $(SLO)$/WordPerfectCollector.obj  \
    $(SLO)$/WordPerfectImportFilter.obj  \
    $(SLO)$/wpft_genericfilter.obj		

.INCLUDE :  target.mk
