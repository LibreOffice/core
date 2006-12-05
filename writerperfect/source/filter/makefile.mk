PRJ=..$/..

PRJNAME=writerperfect
TARGET=filter
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(LIBWPD_CFLAGS) -I..
.ELSE
# broken but ... necessary, internal include shafted ...
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwpd -I..
.ENDIF

SLOFILES= \
    $(SLO)$/DocumentElement.obj		\
    $(SLO)$/FontStyle.obj			\
    $(SLO)$/ListStyle.obj			\
    $(SLO)$/OODocumentHandler.obj		\
    $(SLO)$/PageSpan.obj			\
    $(SLO)$/SectionStyle.obj		\
    $(SLO)$/TableStyle.obj			\
    $(SLO)$/TextRunStyle.obj 		\
    $(SLO)$/WordPerfectCollector.obj 	\
    $(SLO)$/WordPerfectImportFilter.obj 	\
    $(SLO)$/genericfilter.obj		

.INCLUDE :  target.mk
