The extension build in this test uses an update information which is obtained
through a http get request. That is the URL does not reference an update 
information file. Instead it invokes code on a webserver which returns the 
update information. The URL used in this example is:

http://update.services.openoffice.org/ProductUpdateService/check.Update?product=extension&amp;extensionid=org.openoffice.extensions.testarea.desktop.updateinfo&amp;refresh=true

The updateinfo.oxt in this directory has the version 1.0 and in the sub-directory "update is the version 2 of this extension. Version 1.0 is also available here
/extensions/www/testarea/desktop/updateinfocreation/updateinfo.oxt
and version 2.0 here
/extensions/www/testarea/desktop/updateinfocreation/update/updateinfo.oxt

Therefore they can be accessed through

http://extensions.openoffice.org/testarea/desktop/updateinfocreation/updateinfo.oxt
and 
http://extensions.openoffice.org/testarea/desktop/updateinfocreation/update/updateinfo.oxt

The latter location (version 2.0) will also be referenced by the update information
which are returned by the webserver.

The build sub-directory contains the code of the extension (version 2.0) and can 
be build by calling dmake in this directory. The makefile uses the special macros:

EXTUPDATEINFO_NAME=org.openoffice.extensions.testarea.desktop.updateinfo.update.xml
EXTUPDATEINFO_SOURCE=description.xml
EXTUPDATEINFO_URLS = http://extensions.openoffice.org/testarea/desktop/updateinfocreation/update/updateinfo.oxt

This causes the generation of the update information file. This file could be 
directly references by the URL in the <update-information> of the description.xml. 
See also the Wiki entry at:
http://wiki.openoffice.org/wiki/Creating_update_information_for_extensions
This generated update information file can then be used by the webserver, when it 
sends back the requested update information. The update information file will be 
generated in the misc diretory of the output directory.

The update information file needs to be copied into common.pro/pus.mxyz directory. 
The project mwsfinish will process the files in the pus directory.
