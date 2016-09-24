# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/dbapp))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/dbapp,\
	dbaccess/uiconfig/dbapp/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/dbapp,\
	dbaccess/uiconfig/dbapp/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/dbapp,\
	dbaccess/uiconfig/dbapp/toolbar/formobjectbar \
	dbaccess/uiconfig/dbapp/toolbar/queryobjectbar \
	dbaccess/uiconfig/dbapp/toolbar/reportobjectbar \
	dbaccess/uiconfig/dbapp/toolbar/tableobjectbar \
	dbaccess/uiconfig/dbapp/toolbar/toolbar \
))

# vim: set noet sw=4 ts=4:
