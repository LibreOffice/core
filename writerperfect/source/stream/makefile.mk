PRJ=..$/..

PRJNAME=writerperfect
TARGET=stream
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+= $(LIBWPD_CFLAGS)
.ELSE
# broken but ... necessary, internal include shafted ...
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwpd
.ENDIF

SLOFILES= $(SLO)$/WPXSvStream.obj

.INCLUDE :  target.mk
