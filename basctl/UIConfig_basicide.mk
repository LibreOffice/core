# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/BasicIDE))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/BasicIDE,\
	basctl/uiconfig/basicide/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/BasicIDE,\
	basctl/uiconfig/basicide/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/BasicIDE,\
	basctl/uiconfig/basicide/toolbar/dialogbar \
	basctl/uiconfig/basicide/toolbar/findbar \
	basctl/uiconfig/basicide/toolbar/fullscreenbar \
	basctl/uiconfig/basicide/toolbar/insertcontrolsbar \
	basctl/uiconfig/basicide/toolbar/formcontrolsbar \
	basctl/uiconfig/basicide/toolbar/macrobar \
	basctl/uiconfig/basicide/toolbar/standardbar \
	basctl/uiconfig/basicide/toolbar/translationbar \
))

$(eval $(call gb_UIConfig_add_uifiles,modules/BasicIDE,\
	basctl/uiconfig/basicide/ui/basicmacrodialog \
))

# vim: set noet sw=4 ts=4:
