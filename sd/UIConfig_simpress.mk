# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/simpress))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/simpress,\
	sd/uiconfig/simpress/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/simpress,\
	sd/uiconfig/simpress/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/simpress,\
	sd/uiconfig/simpress/toolbar/3dobjectsbar \
	sd/uiconfig/simpress/toolbar/alignmentbar \
	sd/uiconfig/simpress/toolbar/arrowsbar \
	sd/uiconfig/simpress/toolbar/arrowshapes \
	sd/uiconfig/simpress/toolbar/basicshapes \
	sd/uiconfig/simpress/toolbar/bezierobjectbar \
	sd/uiconfig/simpress/toolbar/calloutshapes \
	sd/uiconfig/simpress/toolbar/choosemodebar \
	sd/uiconfig/simpress/toolbar/colorbar \
	sd/uiconfig/simpress/toolbar/commentsbar \
	sd/uiconfig/simpress/toolbar/commontaskbar \
	sd/uiconfig/simpress/toolbar/connectorsbar \
	sd/uiconfig/simpress/toolbar/drawingobjectbar \
	sd/uiconfig/simpress/toolbar/ellipsesbar \
	sd/uiconfig/simpress/toolbar/extrusionobjectbar \
	sd/uiconfig/simpress/toolbar/findbar \
	sd/uiconfig/simpress/toolbar/flowchartshapes \
	sd/uiconfig/simpress/toolbar/fontworkobjectbar \
	sd/uiconfig/simpress/toolbar/fontworkshapetype \
	sd/uiconfig/simpress/toolbar/formcontrols \
	sd/uiconfig/simpress/toolbar/formdesign \
	sd/uiconfig/simpress/toolbar/formsfilterbar \
	sd/uiconfig/simpress/toolbar/formsnavigationbar \
	sd/uiconfig/simpress/toolbar/formtextobjectbar \
	sd/uiconfig/simpress/toolbar/fullscreenbar \
	sd/uiconfig/simpress/toolbar/gluepointsobjectbar \
	sd/uiconfig/simpress/toolbar/graffilterbar \
	sd/uiconfig/simpress/toolbar/graphicobjectbar \
	sd/uiconfig/simpress/toolbar/insertbar \
	sd/uiconfig/simpress/toolbar/linesbar \
	sd/uiconfig/simpress/toolbar/masterviewtoolbar \
	sd/uiconfig/simpress/toolbar/mediaobjectbar \
	sd/uiconfig/simpress/toolbar/moreformcontrols \
	sd/uiconfig/simpress/toolbar/optimizetablebar \
	sd/uiconfig/simpress/toolbar/optionsbar \
	sd/uiconfig/simpress/toolbar/outlinetoolbar \
	sd/uiconfig/simpress/toolbar/positionbar \
	sd/uiconfig/simpress/toolbar/rectanglesbar \
	sd/uiconfig/simpress/toolbar/slideviewobjectbar \
	sd/uiconfig/simpress/toolbar/slideviewtoolbar \
	sd/uiconfig/simpress/toolbar/standardbar \
	sd/uiconfig/simpress/toolbar/starshapes \
	sd/uiconfig/simpress/toolbar/symbolshapes \
	sd/uiconfig/simpress/toolbar/tableobjectbar \
	sd/uiconfig/simpress/toolbar/textbar \
	sd/uiconfig/simpress/toolbar/textobjectbar \
	sd/uiconfig/simpress/toolbar/toolbar \
	sd/uiconfig/simpress/toolbar/viewerbar \
	sd/uiconfig/simpress/toolbar/zoombar \
))

$(eval $(call gb_UIConfig_add_uifiles,modules/simpress,\
	sd/uiconfig/simpress/ui/customanimationcreatedialog \
	sd/uiconfig/simpress/ui/customanimationcreatetab \
	sd/uiconfig/simpress/ui/customanimationspanel \
	sd/uiconfig/simpress/ui/customanimationproperties \
	sd/uiconfig/simpress/ui/customanimationeffecttab \
	sd/uiconfig/simpress/ui/customanimationtimingtab \
	sd/uiconfig/simpress/ui/customanimationtexttab \
	sd/uiconfig/simpress/ui/customslideshows \
	sd/uiconfig/simpress/ui/definecustomslideshow \
	sd/uiconfig/simpress/ui/masterlayoutdlg \
	sd/uiconfig/simpress/ui/optimpressgeneralpage \
	sd/uiconfig/simpress/ui/photoalbum \
	sd/uiconfig/simpress/ui/presentationdialog \
	sd/uiconfig/simpress/ui/printeroptions \
	sd/uiconfig/simpress/ui/prntopts \
	sd/uiconfig/simpress/ui/sdviewpage \
	sd/uiconfig/simpress/ui/templatedialog \
))

# vim: set noet sw=4 ts=4:
