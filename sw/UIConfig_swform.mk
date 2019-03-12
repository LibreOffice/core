# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/swform))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/swform,\
	sw/uiconfig/swform/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_popupmenufiles,modules/swform,\
	sw/uiconfig/swform/popupmenu/anchor \
	sw/uiconfig/swform/popupmenu/annotation \
	sw/uiconfig/swform/popupmenu/draw \
	sw/uiconfig/swform/popupmenu/drawtext \
	sw/uiconfig/swform/popupmenu/form \
	sw/uiconfig/swform/popupmenu/formrichtext \
	sw/uiconfig/swform/popupmenu/frame \
	sw/uiconfig/swform/popupmenu/graphic \
	sw/uiconfig/swform/popupmenu/insertfield \
	sw/uiconfig/swform/popupmenu/media \
	sw/uiconfig/swform/popupmenu/oleobject \
	sw/uiconfig/swform/popupmenu/preview \
	sw/uiconfig/swform/popupmenu/table \
	sw/uiconfig/swform/popupmenu/text \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/swform,\
	sw/uiconfig/swform/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/swform,\
	sw/uiconfig/swform/toolbar/alignmentbar \
	sw/uiconfig/swform/toolbar/arrowshapes \
	sw/uiconfig/swform/toolbar/basicshapes \
	sw/uiconfig/swform/toolbar/bezierobjectbar \
	sw/uiconfig/swform/toolbar/calloutshapes \
	sw/uiconfig/swform/toolbar/colorbar \
	sw/uiconfig/swform/toolbar/drawbar \
	sw/uiconfig/swform/toolbar/drawingobjectbar \
	sw/uiconfig/swform/toolbar/drawtextobjectbar \
	sw/uiconfig/swform/toolbar/extrusionobjectbar \
	sw/uiconfig/swform/toolbar/flowchartshapes \
	sw/uiconfig/swform/toolbar/fontworkobjectbar \
	sw/uiconfig/swform/toolbar/fontworkshapetype \
	sw/uiconfig/swform/toolbar/formcontrols \
	sw/uiconfig/swform/toolbar/formdesign \
	sw/uiconfig/swform/toolbar/formsfilterbar \
	sw/uiconfig/swform/toolbar/formsnavigationbar \
	sw/uiconfig/swform/toolbar/formtextobjectbar \
	sw/uiconfig/swform/toolbar/frameobjectbar \
	sw/uiconfig/swform/toolbar/fullscreenbar \
	sw/uiconfig/swform/toolbar/graffilterbar \
	sw/uiconfig/swform/toolbar/graphicobjectbar \
	sw/uiconfig/swform/toolbar/insertbar \
	sw/uiconfig/swform/toolbar/insertobjectbar \
	sw/uiconfig/swform/toolbar/mailmerge \
	sw/uiconfig/swform/toolbar/mediaobjectbar \
	sw/uiconfig/swform/toolbar/numobjectbar \
	sw/uiconfig/swform/toolbar/oleobjectbar \
	sw/uiconfig/swform/toolbar/optimizetablebar \
	sw/uiconfig/swform/toolbar/previewobjectbar \
	sw/uiconfig/swform/toolbar/standardbar \
	sw/uiconfig/swform/toolbar/starshapes \
	sw/uiconfig/swform/toolbar/symbolshapes \
	sw/uiconfig/swform/toolbar/tableobjectbar \
	sw/uiconfig/swform/toolbar/textobjectbar \
	sw/uiconfig/swform/toolbar/toolbar \
	sw/uiconfig/swform/toolbar/viewerbar \
))

# vim: set noet sw=4 ts=4:
