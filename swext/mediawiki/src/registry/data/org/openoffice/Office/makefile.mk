PRJ=..$/..$/..$/..$/..$/..$/..
PRJNAME=swext
CUSTID=mediawiki
TARGET=mediawiki_registry_data_office
PACKAGE=org.openoffice.Office

.INCLUDE : settings.mk
.INCLUDE : ${PRJ}$/makefile.pmk

LOCALIZEDFILES= Addons.xcu OptionsDialog.xcu

MODULEFILES=

XCUFILES = Addons.xcu OptionsDialog.xcu ProtocolHandler.xcu

.INCLUDE : target.mk

