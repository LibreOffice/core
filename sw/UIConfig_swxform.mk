# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/swxform))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/swxform,\
	sw/uiconfig/swxform/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_popupmenufiles,modules/swxform,\
	sw/uiconfig/swxform/popupmenu/anchor \
	sw/uiconfig/swxform/popupmenu/annotation \
	sw/uiconfig/swxform/popupmenu/draw \
	sw/uiconfig/swxform/popupmenu/drawtext \
	sw/uiconfig/swxform/popupmenu/form \
	sw/uiconfig/swxform/popupmenu/formrichtext \
	sw/uiconfig/swxform/popupmenu/frame \
	sw/uiconfig/swxform/popupmenu/graphic \
	sw/uiconfig/swxform/popupmenu/insertfield \
	sw/uiconfig/swxform/popupmenu/media \
	sw/uiconfig/swxform/popupmenu/oleobject \
	sw/uiconfig/swxform/popupmenu/preview \
	sw/uiconfig/swxform/popupmenu/table \
	sw/uiconfig/swxform/popupmenu/text \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/swxform,\
	sw/uiconfig/swxform/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/swxform,\
	sw/uiconfig/swxform/toolbar/alignmentbar \
	sw/uiconfig/swxform/toolbar/arrowshapes \
	sw/uiconfig/swxform/toolbar/basicshapes \
	sw/uiconfig/swxform/toolbar/bezierobjectbar \
	sw/uiconfig/swxform/toolbar/calloutshapes \
	sw/uiconfig/swxform/toolbar/colorbar \
	sw/uiconfig/swxform/toolbar/drawbar \
	sw/uiconfig/swxform/toolbar/drawingobjectbar \
	sw/uiconfig/swxform/toolbar/drawtextobjectbar \
	sw/uiconfig/swxform/toolbar/extrusionobjectbar \
	sw/uiconfig/swxform/toolbar/findbar \
	sw/uiconfig/swxform/toolbar/flowchartshapes \
	sw/uiconfig/swxform/toolbar/fontworkobjectbar \
	sw/uiconfig/swxform/toolbar/fontworkshapetype \
	sw/uiconfig/swxform/toolbar/formcontrols \
	sw/uiconfig/swxform/toolbar/formdesign \
	sw/uiconfig/swxform/toolbar/formsfilterbar \
	sw/uiconfig/swxform/toolbar/formsnavigationbar \
	sw/uiconfig/swxform/toolbar/formtextobjectbar \
	sw/uiconfig/swxform/toolbar/frameobjectbar \
	sw/uiconfig/swxform/toolbar/fullscreenbar \
	sw/uiconfig/swxform/toolbar/graffilterbar \
	sw/uiconfig/swxform/toolbar/graphicobjectbar \
	sw/uiconfig/swxform/toolbar/insertbar \
	sw/uiconfig/swxform/toolbar/insertobjectbar \
	sw/uiconfig/swxform/toolbar/mailmerge \
	sw/uiconfig/swxform/toolbar/mediaobjectbar \
	sw/uiconfig/swxform/toolbar/numobjectbar \
	sw/uiconfig/swxform/toolbar/oleobjectbar \
	sw/uiconfig/swxform/toolbar/optimizetablebar \
	sw/uiconfig/swxform/toolbar/previewobjectbar \
	sw/uiconfig/swxform/toolbar/standardbar \
	sw/uiconfig/swxform/toolbar/starshapes \
	sw/uiconfig/swxform/toolbar/symbolshapes \
	sw/uiconfig/swxform/toolbar/tableobjectbar \
	sw/uiconfig/swxform/toolbar/textobjectbar \
	sw/uiconfig/swxform/toolbar/toolbar \
	sw/uiconfig/swxform/toolbar/viewerbar \
))

# vim: set noet sw=4 ts=4:
