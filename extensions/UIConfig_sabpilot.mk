# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/sabpilot))

$(eval $(call gb_UIConfig_add_uifiles,modules/sabpilot,\
	extensions/uiconfig/sabpilot/ui/contentfieldpage \
	extensions/uiconfig/sabpilot/ui/contenttablepage \
	extensions/uiconfig/sabpilot/ui/datasourcepage \
	extensions/uiconfig/sabpilot/ui/defaultfieldselectionpage \
	extensions/uiconfig/sabpilot/ui/fieldassignpage \
	extensions/uiconfig/sabpilot/ui/fieldlinkpage \
	extensions/uiconfig/sabpilot/ui/gridfieldsselectionpage \
	extensions/uiconfig/sabpilot/ui/groupradioselectionpage \
	extensions/uiconfig/sabpilot/ui/invokeadminpage \
	extensions/uiconfig/sabpilot/ui/optiondbfieldpage \
	extensions/uiconfig/sabpilot/ui/optionsfinalpage \
	extensions/uiconfig/sabpilot/ui/optionvaluespage \
	extensions/uiconfig/sabpilot/ui/selecttablepage \
	extensions/uiconfig/sabpilot/ui/selecttypepage \
	extensions/uiconfig/sabpilot/ui/tableselectionpage \
))

# vim: set noet sw=4 ts=4:
