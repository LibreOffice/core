# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,sfx))

$(eval $(call gb_UIConfig_add_uifiles,sfx,\
	sfx2/uiconfig/ui/alienwarndialog \
	sfx2/uiconfig/ui/bookmarkdialog \
	sfx2/uiconfig/ui/checkin \
	sfx2/uiconfig/ui/cmisinfopage \
	sfx2/uiconfig/ui/cmisline \
	sfx2/uiconfig/ui/custominfopage \
	sfx2/uiconfig/ui/descriptioninfopage \
	sfx2/uiconfig/ui/documentfontspage \
	sfx2/uiconfig/ui/documentinfopage \
	sfx2/uiconfig/ui/documentpropertiesdialog \
	sfx2/uiconfig/ui/editdurationdialog \
	sfx2/uiconfig/ui/errorfindemaildialog \
	sfx2/uiconfig/ui/floatingrecord \
	sfx2/uiconfig/ui/helpbookmarkpage \
	sfx2/uiconfig/ui/helpcontrol \
	sfx2/uiconfig/ui/helpcontentpage \
	sfx2/uiconfig/ui/helpindexpage \
	sfx2/uiconfig/ui/helpsearchpage \
	sfx2/uiconfig/ui/inputdialog \
	sfx2/uiconfig/ui/licensedialog \
	sfx2/uiconfig/ui/linkeditdialog \
	sfx2/uiconfig/ui/loadtemplatedialog \
	sfx2/uiconfig/ui/managestylepage \
	sfx2/uiconfig/ui/newstyle \
	sfx2/uiconfig/ui/optprintpage \
	sfx2/uiconfig/ui/password \
	sfx2/uiconfig/ui/printeroptionsdialog \
	sfx2/uiconfig/ui/querysavedialog \
	sfx2/uiconfig/ui/searchdialog \
	sfx2/uiconfig/ui/securityinfopage \
	sfx2/uiconfig/ui/singletabdialog \
	sfx2/uiconfig/ui/startcenter \
	sfx2/uiconfig/ui/templatedlg \
	sfx2/uiconfig/ui/versionsofdialog \
	sfx2/uiconfig/ui/versioncommentdialog \
	sfx2/uiconfig/ui/versionscmis \
))

# vim: set noet sw=4 ts=4:
