# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/dbreport))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/dbreport,\
	reportdesign/uiconfig/dbreport/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/dbreport,\
	reportdesign/uiconfig/dbreport/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/dbreport,\
	reportdesign/uiconfig/dbreport/toolbar/alignmentbar \
	reportdesign/uiconfig/dbreport/toolbar/arrowshapes \
	reportdesign/uiconfig/dbreport/toolbar/basicshapes \
	reportdesign/uiconfig/dbreport/toolbar/calloutshapes \
	reportdesign/uiconfig/dbreport/toolbar/drawbar \
	reportdesign/uiconfig/dbreport/toolbar/flowchartshapes \
	reportdesign/uiconfig/dbreport/toolbar/Formatting \
	reportdesign/uiconfig/dbreport/toolbar/reportcontrols \
	reportdesign/uiconfig/dbreport/toolbar/resizebar \
	reportdesign/uiconfig/dbreport/toolbar/sectionalignmentbar \
	reportdesign/uiconfig/dbreport/toolbar/sectionshrinkbar \
	reportdesign/uiconfig/dbreport/toolbar/starshapes \
	reportdesign/uiconfig/dbreport/toolbar/symbolshapes \
	reportdesign/uiconfig/dbreport/toolbar/toolbar \
))

$(eval $(call gb_UIConfig_add_uifiles,modules/dbreport,\
	reportdesign/uiconfig/dbreport/ui/pagenumberdialog \
))
# vim: set noet sw=4 ts=4:
