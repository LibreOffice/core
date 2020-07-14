# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/BasicIDE))

$(eval $(call gb_UIConfig_add_popupmenufiles,modules/BasicIDE,\
	basctl/uiconfig/basicide/popupmenu/dialog \
	basctl/uiconfig/basicide/popupmenu/tabbar \
))

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
	basctl/uiconfig/basicide/ui/breakpointmenus \
	basctl/uiconfig/basicide/ui/codecomplete \
	basctl/uiconfig/basicide/ui/combobox \
	basctl/uiconfig/basicide/ui/defaultlanguage \
	basctl/uiconfig/basicide/ui/deletelangdialog \
	basctl/uiconfig/basicide/ui/dialogpage \
	basctl/uiconfig/basicide/ui/dockingorganizer \
	basctl/uiconfig/basicide/ui/dockingstack \
	basctl/uiconfig/basicide/ui/dockingwatch \
	basctl/uiconfig/basicide/ui/exportdialog \
	basctl/uiconfig/basicide/ui/gotolinedialog \
	basctl/uiconfig/basicide/ui/importlibdialog \
	basctl/uiconfig/basicide/ui/libpage \
	basctl/uiconfig/basicide/ui/managebreakpoints \
	basctl/uiconfig/basicide/ui/managelanguages \
	basctl/uiconfig/basicide/ui/modulepage \
	basctl/uiconfig/basicide/ui/newlibdialog \
	basctl/uiconfig/basicide/ui/organizedialog \
	basctl/uiconfig/basicide/ui/sortmenu \
))

# vim: set noet sw=4 ts=4:
