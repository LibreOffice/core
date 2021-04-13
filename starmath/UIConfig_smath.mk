# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/smath))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/smath,\
	starmath/uiconfig/smath/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_popupmenufiles,modules/smath,\
	starmath/uiconfig/smath/popupmenu/edit \
	starmath/uiconfig/smath/popupmenu/view \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/smath,\
	starmath/uiconfig/smath/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/smath,\
	starmath/uiconfig/smath/toolbar/toolbar \
	starmath/uiconfig/smath/toolbar/standardbar \
	starmath/uiconfig/smath/toolbar/fullscreenbar \
))

$(eval $(call gb_UIConfig_add_uifiles,modules/smath,\
	starmath/uiconfig/smath/ui/alignmentdialog \
	starmath/uiconfig/smath/ui/catalogdialog \
	starmath/uiconfig/smath/ui/dockingelements \
	starmath/uiconfig/smath/ui/editwindow \
	starmath/uiconfig/smath/ui/fontdialog \
	starmath/uiconfig/smath/ui/fontsizedialog \
	starmath/uiconfig/smath/ui/fonttypedialog \
	starmath/uiconfig/smath/ui/printeroptions \
	starmath/uiconfig/smath/ui/savedefaultsdialog \
	starmath/uiconfig/smath/ui/smathsettings \
	starmath/uiconfig/smath/ui/spacingdialog \
	starmath/uiconfig/smath/ui/symdefinedialog \
))

# vim: set noet sw=4 ts=4:
