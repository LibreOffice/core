PRJ=..$/..

PRJNAME=writerperfect
TARGET=wpgimp
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(LIBWPD_CFLAGS)
.ELSE
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwpd
.ENDIF

.IF "$(SYSTEM_LIBWPG)" == "YES"
INCPRE+=$(LIBWPG_CFLAGS)
.ELSE
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwpg
.ENDIF

# broken but ... necessary, internal include shafted ...
INCPRE+= -I..

SLOFILES= \
    $(SLO)$/WPGImportFilter.obj \
    $(SLO)$/wpgimport_genericfilter.obj

.INCLUDE :  target.mk
