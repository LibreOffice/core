PRJ=..$/..

PRJNAME=writerperfect
TARGET=stream
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(LIBWPD_CFLAGS)
.ENDIF

# broken but ... necessary, internal include shafted ...
INCPRE+= -I..

SLOFILES= $(SLO)$/WPXSvStream.obj

.INCLUDE :  target.mk
