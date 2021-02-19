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
	vcl/uiconfig/ui/aboutbox \
	vcl/uiconfig/ui/combobox \
	vcl/uiconfig/ui/cupspassworddialog \
	vcl/uiconfig/ui/editmenu \
	vcl/uiconfig/ui/errornocontentdialog \
	vcl/uiconfig/ui/errornoprinterdialog \
	vcl/uiconfig/ui/interimdockparent \
	vcl/uiconfig/ui/interimtearableparent \
	vcl/uiconfig/ui/menutogglebutton \
	vcl/uiconfig/ui/printdialog \
	vcl/uiconfig/ui/printerdevicepage \
	vcl/uiconfig/ui/printerpaperpage \
	vcl/uiconfig/ui/printerpropertiesdialog \
	vcl/uiconfig/ui/printprogressdialog \
	vcl/uiconfig/ui/querydialog \
	vcl/uiconfig/ui/screenshotparent \
	vcl/uiconfig/ui/wizard \
))

$(eval $(call gb_UIConfig_add_a11yerrors_uifiles,vcl,\
	vcl/qa/cppunit/builder/demo \
))

# vim: set noet sw=4 ts=4:
