# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/StartModule))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/StartModule,\
	framework/uiconfig/startmodule/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/StartModule,\
	framework/uiconfig/startmodule/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/StartModule,\
	framework/uiconfig/startmodule/toolbar/standardbar \
))

$(eval $(call gb_UIConfig_add_uifiles,modules/StartModule,\
	framework/uiconfig/startmodule/ui/startcenter \
))

# vim: set noet sw=4 ts=4:
