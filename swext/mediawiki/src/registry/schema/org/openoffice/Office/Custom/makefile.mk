PRJ=..$/..$/..$/..$/..$/..$/..$/..
PRJNAME=swext
CUSTID=mediawiki
TARGET=mediawiki_registry_schema_custom
PACKAGE=org.openoffice.Office.Custom

.INCLUDE : settings.mk
.INCLUDE : ${PRJ}$/makefile.pmk

.IF "$(ENABLE_MEDIAWIKI)" == "YES"
LOCALIZEDFILES=

MODULEFILES=

XCSFILES = WikiExtension.xcs
.ENDIF

.INCLUDE : target.mk

