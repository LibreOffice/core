PRJ=..$/..

PRJNAME=writerperfect
TARGET=filter
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(LIBWPD_CFLAGS) -I..
.ELSE
# broken but ... necessary, internal include shafted ...
INCPRE+=-I..
.ENDIF

SLOFILES= \
    $(SLO)$/DocumentElement.obj		\
    $(SLO)$/FontStyle.obj			\
    $(SLO)$/ListStyle.obj			\
    $(SLO)$/DocumentHandler.obj		\
    $(SLO)$/PageSpan.obj			\
    $(SLO)$/SectionStyle.obj		\
    $(SLO)$/TableStyle.obj			\
    $(SLO)$/TextRunStyle.obj 		\
    $(SLO)$/DocumentCollector.obj

.INCLUDE :  target.mk
