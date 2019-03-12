# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/sglobal))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/sglobal,\
	sw/uiconfig/sglobal/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_popupmenufiles,modules/sglobal,\
	sw/uiconfig/sglobal/popupmenu/anchor \
	sw/uiconfig/sglobal/popupmenu/annotation \
	sw/uiconfig/sglobal/popupmenu/draw \
	sw/uiconfig/sglobal/popupmenu/drawtext \
	sw/uiconfig/sglobal/popupmenu/form \
	sw/uiconfig/sglobal/popupmenu/formrichtext \
	sw/uiconfig/sglobal/popupmenu/frame \
	sw/uiconfig/sglobal/popupmenu/graphic \
	sw/uiconfig/sglobal/popupmenu/insertfield \
	sw/uiconfig/sglobal/popupmenu/media \
	sw/uiconfig/sglobal/popupmenu/oleobject \
	sw/uiconfig/sglobal/popupmenu/preview \
	sw/uiconfig/sglobal/popupmenu/table \
	sw/uiconfig/sglobal/popupmenu/text \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/sglobal,\
	sw/uiconfig/sglobal/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/sglobal,\
	sw/uiconfig/sglobal/toolbar/alignmentbar \
	sw/uiconfig/sglobal/toolbar/arrowshapes \
	sw/uiconfig/sglobal/toolbar/basicshapes \
	sw/uiconfig/sglobal/toolbar/bezierobjectbar \
	sw/uiconfig/sglobal/toolbar/calloutshapes \
	sw/uiconfig/sglobal/toolbar/colorbar \
	sw/uiconfig/sglobal/toolbar/drawbar \
	sw/uiconfig/sglobal/toolbar/drawingobjectbar \
	sw/uiconfig/sglobal/toolbar/drawtextobjectbar \
	sw/uiconfig/sglobal/toolbar/extrusionobjectbar \
	sw/uiconfig/sglobal/toolbar/findbar \
	sw/uiconfig/sglobal/toolbar/flowchartshapes \
	sw/uiconfig/sglobal/toolbar/fontworkobjectbar \
	sw/uiconfig/sglobal/toolbar/fontworkshapetype \
	sw/uiconfig/sglobal/toolbar/formcontrols \
	sw/uiconfig/sglobal/toolbar/formdesign \
	sw/uiconfig/sglobal/toolbar/formsfilterbar \
	sw/uiconfig/sglobal/toolbar/formsnavigationbar \
	sw/uiconfig/sglobal/toolbar/formtextobjectbar \
	sw/uiconfig/sglobal/toolbar/frameobjectbar \
	sw/uiconfig/sglobal/toolbar/fullscreenbar \
	sw/uiconfig/sglobal/toolbar/graffilterbar \
	sw/uiconfig/sglobal/toolbar/graphicobjectbar \
	sw/uiconfig/sglobal/toolbar/insertbar \
	sw/uiconfig/sglobal/toolbar/insertobjectbar \
	sw/uiconfig/sglobal/toolbar/mediaobjectbar \
	sw/uiconfig/sglobal/toolbar/numobjectbar \
	sw/uiconfig/sglobal/toolbar/oleobjectbar \
	sw/uiconfig/sglobal/toolbar/optimizetablebar \
	sw/uiconfig/sglobal/toolbar/previewobjectbar \
	sw/uiconfig/sglobal/toolbar/standardbar \
	sw/uiconfig/sglobal/toolbar/starshapes \
	sw/uiconfig/sglobal/toolbar/symbolshapes \
	sw/uiconfig/sglobal/toolbar/tableobjectbar \
	sw/uiconfig/sglobal/toolbar/textobjectbar \
    sw/uiconfig/sglobal/toolbar/textstylebar \
	sw/uiconfig/sglobal/toolbar/toolbar \
	sw/uiconfig/sglobal/toolbar/viewerbar \
))

# vim: set noet sw=4 ts=4:
