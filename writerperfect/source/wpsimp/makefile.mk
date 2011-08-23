PRJ=..$/..

PRJNAME=writerperfect
TARGET=wpsimp
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(LIBWPD_CFLAGS)
.ELSE
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwpd
.ENDIF

.IF "$(SYSTEM_LIBWPS)" == "YES"
INCPRE+=$(LIBWPS_CFLAGS)
.ELSE
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwps
.ENDIF

# broken but ... necessary, internal include shafted ...
INCPRE+= -I..

SLOFILES= \
    $(SLO)$/MSWorksCollector.obj  \
    $(SLO)$/MSWorksImportFilter.obj  \
    $(SLO)$/msworks_genericfilter.obj

.INCLUDE :  target.mk
