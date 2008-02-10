PRJ=..$/..$/..$/..$/..$/..$/..$/..
PRJNAME=swext
CUSTID=mediawiki
TARGET=mediawiki_registry_data_Custom
PACKAGE=org.openoffice.Office.Custom

.INCLUDE : settings.mk
.INCLUDE : ${PRJ}$/makefile.pmk

ABSXCSROOT=$(PWD)$/$(MISC)$/$(CUSTID)
XCSROOT=$(MISC)$/$(CUSTID)

.IF "$(ENABLE_MEDIAWIKI)" == "YES"
LOCALIZEDFILES= WikiExtension.xcu

MODULEFILES=

XCUFILES = WikiExtension.xcu
.ENDIF

.INCLUDE : target.mk
