# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/sdraw))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/sdraw,\
	sd/uiconfig/sdraw/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/sdraw,\
	sd/uiconfig/sdraw/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/sdraw,\
	sd/uiconfig/sdraw/toolbar/3dobjectsbar \
	sd/uiconfig/sdraw/toolbar/alignmentbar \
	sd/uiconfig/sdraw/toolbar/arrowsbar \
	sd/uiconfig/sdraw/toolbar/arrowshapes \
	sd/uiconfig/sdraw/toolbar/basicshapes \
	sd/uiconfig/sdraw/toolbar/bezierobjectbar \
	sd/uiconfig/sdraw/toolbar/calloutshapes \
	sd/uiconfig/sdraw/toolbar/choosemodebar \
	sd/uiconfig/sdraw/toolbar/colorbar \
	sd/uiconfig/sdraw/toolbar/commentsbar \
	sd/uiconfig/sdraw/toolbar/connectorsbar \
	sd/uiconfig/sdraw/toolbar/drawingobjectbar \
	sd/uiconfig/sdraw/toolbar/ellipsesbar \
	sd/uiconfig/sdraw/toolbar/extrusionobjectbar \
	sd/uiconfig/sdraw/toolbar/findbar \
	sd/uiconfig/sdraw/toolbar/flowchartshapes \
	sd/uiconfig/sdraw/toolbar/fontworkobjectbar \
	sd/uiconfig/sdraw/toolbar/fontworkshapetype \
	sd/uiconfig/sdraw/toolbar/formcontrols \
	sd/uiconfig/sdraw/toolbar/formdesign \
	sd/uiconfig/sdraw/toolbar/formsfilterbar \
	sd/uiconfig/sdraw/toolbar/formsnavigationbar \
	sd/uiconfig/sdraw/toolbar/formtextobjectbar \
	sd/uiconfig/sdraw/toolbar/fullscreenbar \
	sd/uiconfig/sdraw/toolbar/gluepointsobjectbar \
	sd/uiconfig/sdraw/toolbar/graffilterbar \
	sd/uiconfig/sdraw/toolbar/graphicobjectbar \
	sd/uiconfig/sdraw/toolbar/insertbar \
	sd/uiconfig/sdraw/toolbar/linesbar \
	sd/uiconfig/sdraw/toolbar/masterviewtoolbar \
	sd/uiconfig/sdraw/toolbar/mediaobjectbar \
	sd/uiconfig/sdraw/toolbar/moreformcontrols \
	sd/uiconfig/sdraw/toolbar/optimizetablebar \
	sd/uiconfig/sdraw/toolbar/optionsbar \
	sd/uiconfig/sdraw/toolbar/positionbar \
	sd/uiconfig/sdraw/toolbar/rectanglesbar \
	sd/uiconfig/sdraw/toolbar/standardbar \
	sd/uiconfig/sdraw/toolbar/starshapes \
	sd/uiconfig/sdraw/toolbar/symbolshapes \
	sd/uiconfig/sdraw/toolbar/tableobjectbar \
	sd/uiconfig/sdraw/toolbar/textbar \
	sd/uiconfig/sdraw/toolbar/textobjectbar \
	sd/uiconfig/sdraw/toolbar/toolbar \
	sd/uiconfig/sdraw/toolbar/viewerbar \
	sd/uiconfig/sdraw/toolbar/zoombar \
))

$(eval $(call gb_UIConfig_add_uifiles,modules/sdraw,\
	sd/uiconfig/sdraw/ui/dlgsnap \
	sd/uiconfig/sdraw/ui/copydlg \
        sd/uiconfig/sdraw/ui/drawparadialog \
	sd/uiconfig/sdraw/ui/insertlayer \
	sd/uiconfig/sdraw/ui/paranumberingtab \
	sd/uiconfig/sdraw/ui/printeroptions \
))

# vim: set noet sw=4 ts=4:
