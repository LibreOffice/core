# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/scalc))

ifeq ($(ENABLE_TELEPATHY),TRUE)
$(eval $(call gb_UIConfig_add_generated_menubarfiles,modules/scalc,\
	CustomTarget/sc/uiconfig/scalc/menubar/menubar \
))
else
$(eval $(call gb_UIConfig_add_menubarfiles,modules/scalc,\
	sc/uiconfig/scalc/menubar/menubar \
))
endif

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/scalc,\
	sc/uiconfig/scalc/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/scalc,\
	sc/uiconfig/scalc/toolbar/alignmentbar \
	sc/uiconfig/scalc/toolbar/arrowshapes \
	sc/uiconfig/scalc/toolbar/basicshapes \
	sc/uiconfig/scalc/toolbar/calloutshapes \
	sc/uiconfig/scalc/toolbar/colorbar \
	sc/uiconfig/scalc/toolbar/drawbar \
	sc/uiconfig/scalc/toolbar/drawobjectbar \
	sc/uiconfig/scalc/toolbar/extrusionobjectbar \
	sc/uiconfig/scalc/toolbar/findbar \
	sc/uiconfig/scalc/toolbar/flowchartshapes \
	sc/uiconfig/scalc/toolbar/fontworkobjectbar \
	sc/uiconfig/scalc/toolbar/fontworkshapetype \
	sc/uiconfig/scalc/toolbar/formatobjectbar \
	sc/uiconfig/scalc/toolbar/formcontrols \
	sc/uiconfig/scalc/toolbar/formdesign \
	sc/uiconfig/scalc/toolbar/formsfilterbar \
	sc/uiconfig/scalc/toolbar/formsnavigationbar \
	sc/uiconfig/scalc/toolbar/formtextobjectbar \
	sc/uiconfig/scalc/toolbar/fullscreenbar \
	sc/uiconfig/scalc/toolbar/graffilterbar \
	sc/uiconfig/scalc/toolbar/graphicobjectbar \
	sc/uiconfig/scalc/toolbar/insertbar \
	sc/uiconfig/scalc/toolbar/insertcellsbar \
	sc/uiconfig/scalc/toolbar/mediaobjectbar \
	sc/uiconfig/scalc/toolbar/moreformcontrols \
	sc/uiconfig/scalc/toolbar/previewbar \
	sc/uiconfig/scalc/toolbar/standardbar \
	sc/uiconfig/scalc/toolbar/starshapes \
	sc/uiconfig/scalc/toolbar/symbolshapes \
	sc/uiconfig/scalc/toolbar/textobjectbar \
	sc/uiconfig/scalc/toolbar/toolbar \
	sc/uiconfig/scalc/toolbar/viewerbar \
))

$(eval $(call gb_UIConfig_add_uifiles,modules/scalc,\
	sc/uiconfig/scalc/ui/cellprotectionpage \
	sc/uiconfig/scalc/ui/definename \
	sc/uiconfig/scalc/ui/deletecells \
	sc/uiconfig/scalc/ui/deletecontents \
	sc/uiconfig/scalc/ui/formatcellsdialog \
	sc/uiconfig/scalc/ui/insertname \
	sc/uiconfig/scalc/ui/insertsheet \
	sc/uiconfig/scalc/ui/managenamesdialog \
	sc/uiconfig/scalc/ui/printeroptions \
	sc/uiconfig/scalc/ui/sheetprintpage \
	sc/uiconfig/scalc/ui/selectrange \
	sc/uiconfig/scalc/ui/sortcriteriapage \
	sc/uiconfig/scalc/ui/sortkey \
	sc/uiconfig/scalc/ui/sortoptionspage \
	sc/uiconfig/scalc/ui/textimportoptions \
	sc/uiconfig/scalc/ui/textimportcsv \
))

# vim: set noet sw=4 ts=4:
