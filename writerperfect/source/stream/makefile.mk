PRJ=..$/..

PRJNAME=stream
TARGET=stream
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

# broken but ... necessary, internal include shafted ...
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwpd

SLOFILES= $(SLO)$/WPXSvStream.obj

.INCLUDE :  target.mk
