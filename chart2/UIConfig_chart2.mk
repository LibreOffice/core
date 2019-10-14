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

$(eval $(call gb_UIConfig_add_popupmenufiles,modules/schart,\
	chart2/uiconfig/popupmenu/draw \
	chart2/uiconfig/popupmenu/drawtext \
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
	chart2/uiconfig/ui/3dviewdialog \
	chart2/uiconfig/ui/attributedialog \
	chart2/uiconfig/ui/chardialog \
	chart2/uiconfig/ui/chartdatadialog \
	chart2/uiconfig/ui/charttypedialog \
	chart2/uiconfig/ui/columnfragment \
	chart2/uiconfig/ui/datarangedialog \
	chart2/uiconfig/ui/dlg_DataLabel \
	chart2/uiconfig/ui/dlg_InsertErrorBars \
	chart2/uiconfig/ui/imagefragment \
	chart2/uiconfig/ui/insertaxisdlg \
	chart2/uiconfig/ui/insertgriddlg \
	chart2/uiconfig/ui/inserttitledlg \
	chart2/uiconfig/ui/paradialog \
	chart2/uiconfig/ui/sidebaraxis \
	chart2/uiconfig/ui/sidebarelements \
	chart2/uiconfig/ui/sidebarerrorbar \
	chart2/uiconfig/ui/sidebarseries \
	chart2/uiconfig/ui/sidebartype \
	chart2/uiconfig/ui/smoothlinesdlg \
	chart2/uiconfig/ui/steppedlinesdlg \
	chart2/uiconfig/ui/titlerotationtabpage \
	chart2/uiconfig/ui/dlg_InsertLegend \
	chart2/uiconfig/ui/tp_3D_SceneAppearance \
	chart2/uiconfig/ui/tp_3D_SceneGeometry \
	chart2/uiconfig/ui/tp_3D_SceneIllumination \
	chart2/uiconfig/ui/tp_axisLabel \
	chart2/uiconfig/ui/tp_AxisPositions \
	chart2/uiconfig/ui/tp_ChartType \
	chart2/uiconfig/ui/tp_DataLabel \
	chart2/uiconfig/ui/tp_DataSource \
	chart2/uiconfig/ui/tp_ErrorBars \
	chart2/uiconfig/ui/tp_LegendPosition \
	chart2/uiconfig/ui/tp_PolarOptions \
	chart2/uiconfig/ui/tp_RangeChooser \
	chart2/uiconfig/ui/tp_SeriesToAxis \
	chart2/uiconfig/ui/tp_Scale \
	chart2/uiconfig/ui/tp_Trendline \
	chart2/uiconfig/ui/wizelementspage \
))

# vim: set noet sw=4 ts=4:
