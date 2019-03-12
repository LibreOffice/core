# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/swreport))

$(eval $(call gb_UIConfig_add_menubarfiles,modules/swreport,\
	sw/uiconfig/swreport/menubar/menubar \
))

$(eval $(call gb_UIConfig_add_popupmenufiles,modules/swreport,\
	sw/uiconfig/swreport/popupmenu/anchor \
	sw/uiconfig/swreport/popupmenu/annotation \
	sw/uiconfig/swreport/popupmenu/draw \
	sw/uiconfig/swreport/popupmenu/drawtext \
	sw/uiconfig/swreport/popupmenu/form \
	sw/uiconfig/swreport/popupmenu/formrichtext \
	sw/uiconfig/swreport/popupmenu/frame \
	sw/uiconfig/swreport/popupmenu/graphic \
	sw/uiconfig/swreport/popupmenu/insertfield \
	sw/uiconfig/swreport/popupmenu/media \
	sw/uiconfig/swreport/popupmenu/oleobject \
	sw/uiconfig/swreport/popupmenu/preview \
	sw/uiconfig/swreport/popupmenu/table \
	sw/uiconfig/swreport/popupmenu/text \
))

$(eval $(call gb_UIConfig_add_statusbarfiles,modules/swreport,\
	sw/uiconfig/swreport/statusbar/statusbar \
))

$(eval $(call gb_UIConfig_add_toolbarfiles,modules/swreport,\
	sw/uiconfig/swreport/toolbar/alignmentbar \
	sw/uiconfig/swreport/toolbar/arrowshapes \
	sw/uiconfig/swreport/toolbar/basicshapes \
	sw/uiconfig/swreport/toolbar/bezierobjectbar \
	sw/uiconfig/swreport/toolbar/calloutshapes \
	sw/uiconfig/swreport/toolbar/colorbar \
	sw/uiconfig/swreport/toolbar/drawbar \
	sw/uiconfig/swreport/toolbar/drawingobjectbar \
	sw/uiconfig/swreport/toolbar/drawtextobjectbar \
	sw/uiconfig/swreport/toolbar/extrusionobjectbar \
	sw/uiconfig/swreport/toolbar/flowchartshapes \
	sw/uiconfig/swreport/toolbar/fontworkobjectbar \
	sw/uiconfig/swreport/toolbar/fontworkshapetype \
	sw/uiconfig/swreport/toolbar/formcontrols \
	sw/uiconfig/swreport/toolbar/formdesign \
	sw/uiconfig/swreport/toolbar/formsfilterbar \
	sw/uiconfig/swreport/toolbar/formsnavigationbar \
	sw/uiconfig/swreport/toolbar/formtextobjectbar \
	sw/uiconfig/swreport/toolbar/frameobjectbar \
	sw/uiconfig/swreport/toolbar/fullscreenbar \
	sw/uiconfig/swreport/toolbar/graffilterbar \
	sw/uiconfig/swreport/toolbar/graphicobjectbar \
	sw/uiconfig/swreport/toolbar/insertbar \
	sw/uiconfig/swreport/toolbar/insertobjectbar \
	sw/uiconfig/swreport/toolbar/mailmerge \
	sw/uiconfig/swreport/toolbar/mediaobjectbar \
	sw/uiconfig/swreport/toolbar/numobjectbar \
	sw/uiconfig/swreport/toolbar/oleobjectbar \
	sw/uiconfig/swreport/toolbar/optimizetablebar \
	sw/uiconfig/swreport/toolbar/previewobjectbar \
	sw/uiconfig/swreport/toolbar/standardbar \
	sw/uiconfig/swreport/toolbar/starshapes \
	sw/uiconfig/swreport/toolbar/symbolshapes \
	sw/uiconfig/swreport/toolbar/tableobjectbar \
	sw/uiconfig/swreport/toolbar/textobjectbar \
	sw/uiconfig/swreport/toolbar/toolbar \
	sw/uiconfig/swreport/toolbar/viewerbar \
))

# vim: set noet sw=4 ts=4:
