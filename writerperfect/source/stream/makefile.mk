PRJ=..$/..

PRJNAME=writerperfect
TARGET=stream
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(DISABLE_LIBWPD)" == ""

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(LIBWPD_CFLAGS)
.ENDIF

# broken but ... necessary, internal include shafted ...
INCPRE+= -I..

SLOFILES= $(SLO)$/WPXSvStream.obj

.INCLUDE :  target.mk

.ELSE
all:
    @echo "libwpd disabled"
.ENDIF
