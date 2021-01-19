# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/sweb))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/sweb,\
	sw/uiconfig/sweb/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_popupmenufiles,modules/sweb,\
	sw/uiconfig/sweb/popupmenu/anchor \
	sw/uiconfig/sweb/popupmenu/annotation \
	sw/uiconfig/sweb/popupmenu/form \
	sw/uiconfig/sweb/popupmenu/formrichtext \
	sw/uiconfig/sweb/popupmenu/frame \
	sw/uiconfig/sweb/popupmenu/graphic \
	sw/uiconfig/sweb/popupmenu/insertfield \
	sw/uiconfig/sweb/popupmenu/oleobject \
	sw/uiconfig/sweb/popupmenu/preview \
	sw/uiconfig/sweb/popupmenu/source \
	sw/uiconfig/sweb/popupmenu/table \
	sw/uiconfig/sweb/popupmenu/text \
	sw/uiconfig/swriter/popupmenu/showtrackedchanges \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/sweb,\
	sw/uiconfig/sweb/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/sweb,\
	sw/uiconfig/sweb/toolbar/arrowshapes \
	sw/uiconfig/sweb/toolbar/basicshapes \
	sw/uiconfig/sweb/toolbar/bezierobjectbar \
	sw/uiconfig/sweb/toolbar/calloutshapes \
	sw/uiconfig/sweb/toolbar/colorbar \
	sw/uiconfig/sweb/toolbar/drawingobjectbar \
	sw/uiconfig/sweb/toolbar/drawtextobjectbar \
	sw/uiconfig/sweb/toolbar/extrusionobjectbar \
	sw/uiconfig/sweb/toolbar/findbar \
	sw/uiconfig/sweb/toolbar/flowchartshapes \
	sw/uiconfig/sweb/toolbar/fontworkobjectbar \
	sw/uiconfig/sweb/toolbar/fontworkshapetype \
	sw/uiconfig/sweb/toolbar/formcontrols \
	sw/uiconfig/sweb/toolbar/formdesign \
	sw/uiconfig/sweb/toolbar/formsfilterbar \
	sw/uiconfig/sweb/toolbar/formsnavigationbar \
	sw/uiconfig/sweb/toolbar/formtextobjectbar \
	sw/uiconfig/sweb/toolbar/frameobjectbar \
	sw/uiconfig/sweb/toolbar/fullscreenbar \
	sw/uiconfig/sweb/toolbar/graffilterbar \
	sw/uiconfig/sweb/toolbar/graphicobjectbar \
	sw/uiconfig/sweb/toolbar/insertbar \
	sw/uiconfig/sweb/toolbar/mediaobjectbar \
	sw/uiconfig/sweb/toolbar/numobjectbar \
	sw/uiconfig/sweb/toolbar/oleobjectbar \
	sw/uiconfig/sweb/toolbar/optimizetablebar \
	sw/uiconfig/sweb/toolbar/previewobjectbar \
	sw/uiconfig/sweb/toolbar/standardbar \
	sw/uiconfig/sweb/toolbar/starshapes \
	sw/uiconfig/sweb/toolbar/symbolshapes \
	sw/uiconfig/sweb/toolbar/tableobjectbar \
	sw/uiconfig/sweb/toolbar/textobjectbar \
	sw/uiconfig/sweb/toolbar/toolbar \
	sw/uiconfig/sweb/toolbar/viewerbar \
))

# vim: set noet sw=4 ts=4:
