PRJ=..$/..$/..$/..$/..$/..$/..$/..
PRJNAME=swext
CUSTID=mediawiki
TARGET=mediawiki_registry_data_Custom
PACKAGE=org.openoffice.Office.Custom

.INCLUDE : settings.mk
.INCLUDE : ${PRJ}$/makefile.pmk
ABSXCSROOT=$(PWD)$/$(MISC)$/$(CUSTID)
XCSROOT=$(MISC)$/$(CUSTID)

LOCALIZEDFILES= WikiExtension.xcu

MODULEFILES=

XCUFILES = WikiExtension.xcu

.INCLUDE : target.mk

