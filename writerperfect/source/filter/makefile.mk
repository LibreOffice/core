PRJ=..$/..

PRJNAME=filter
TARGET=filter
ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

# broken but ... necessary, internal include shafted ...
INCPRE+=$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc$/libwpd -I..

SLOFILES= \
    $(SLO)$/DocumentElement.obj		\
    $(SLO)$/FontMap.obj			\
    $(SLO)$/FontStyle.obj			\
    $(SLO)$/ListStyle.obj			\
    $(SLO)$/PageSpan.obj			\
    $(SLO)$/SectionStyle.obj		\
    $(SLO)$/TableStyle.obj			\
    $(SLO)$/TextRunStyle.obj 		\
    $(SLO)$/WordPerfectCollector.obj 	\
    $(SLO)$/WordPerfectImportFilter.obj 	\
    $(SLO)$/genericfilter.obj		

.INCLUDE :  target.mk
