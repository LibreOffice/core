PRJ=..$/..$/..$/..$/..$/..$/..$/..
PRJNAME=swext
CUSTID=mediawiki
TARGET=mediawiki_registry_schema_custom
PACKAGE=org.openoffice.Office.Custom

.INCLUDE : settings.mk
.INCLUDE : ${PRJ}$/makefile.pmk

LOCALIZEDFILES=

MODULEFILES=

XCSFILES = WikiExtension.xcs

.INCLUDE : target.mk

