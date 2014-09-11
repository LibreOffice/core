# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,vcl))

$(eval $(call gb_UIConfig_add_uifiles,vcl,\
	vcl/uiconfig/ui/cupspassworddialog \
	vcl/uiconfig/ui/errornocontentdialog \
	vcl/uiconfig/ui/errornoprinterdialog \
	vcl/uiconfig/ui/notebookbar \
	vcl/uiconfig/ui/printdialog \
	vcl/uiconfig/ui/printerdevicepage \
	vcl/uiconfig/ui/printerpaperpage \
	vcl/uiconfig/ui/printerpropertiesdialog \
	vcl/uiconfig/ui/printprogressdialog \
	vcl/uiconfig/ui/querydialog \
))

# vim: set noet sw=4 ts=4:
