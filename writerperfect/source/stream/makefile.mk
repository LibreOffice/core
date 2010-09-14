PRJ=..$/..

PRJNAME=writerperfect
TARGET=stream
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(LIBWPD_CFLAGS)
.ENDIF

.IF "$(SYSTEM_LIBWPS)" == "YES"
INCPRE+=$(LIBWPS_CFLAGS)
.ELSE
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwps
.ENDIF

# broken but ... necessary, internal include shafted ...
INCPRE+= -I..

SLOFILES= $(SLO)$/WPXSvStream.obj

.INCLUDE :  target.mk
