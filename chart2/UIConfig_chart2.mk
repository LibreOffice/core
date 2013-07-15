# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/schart))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/schart,\
	chart2/uiconfig/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/schart,\
	chart2/uiconfig/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/schart,\
	chart2/uiconfig/toolbar/arrowshapes \
	chart2/uiconfig/toolbar/basicshapes \
	chart2/uiconfig/toolbar/calloutshapes \
	chart2/uiconfig/toolbar/drawbar \
	chart2/uiconfig/toolbar/flowchartshapes \
	chart2/uiconfig/toolbar/standardbar \
	chart2/uiconfig/toolbar/starshapes \
	chart2/uiconfig/toolbar/symbolshapes \
	chart2/uiconfig/toolbar/toolbar \
))

$(eval $(call gb_UIConfig_add_uifiles,modules/schart,\
	chart2/uiconfig/ui/insertaxisdlg \
	chart2/uiconfig/ui/insertgriddlg \
	chart2/uiconfig/ui/smoothlinesdlg \
	chart2/uiconfig/ui/steppedlinesdlg \
	chart2/uiconfig/ui/tp_SeriesToAxis \
))

# vim: set noet sw=4 ts=4:
