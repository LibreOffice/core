# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,chartcontroller))

$(eval $(call gb_Library_set_include,chartcontroller,\
    $$(INCLUDE) \
    -I$(SRCDIR)/chart2/source/controller/inc \
    -I$(SRCDIR)/chart2/source/inc \
))

$(eval $(call gb_Library_set_precompiled_header,chartcontroller,$(SRCDIR)/chart2/inc/pch/precompiled_chartcontroller))

$(eval $(call gb_Library_use_external,chartcontroller,boost_headers))

$(eval $(call gb_Library_use_sdk_api,chartcontroller))

$(eval $(call gb_Library_use_libraries,chartcontroller,\
    basegfx \
    chartcore \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    sal \
	i18nlangtag \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    svx \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,chartcontroller,chart2/source/controller/chartcontroller))

$(eval $(call gb_Library_add_exception_objects,chartcontroller,\
    chart2/source/controller/accessibility/AccessibleBase \
    chart2/source/controller/accessibility/AccessibleChartElement \
    chart2/source/controller/accessibility/AccessibleChartShape \
    chart2/source/controller/accessibility/AccessibleChartView \
    chart2/source/controller/accessibility/AccessibleTextHelper \
    chart2/source/controller/accessibility/AccessibleViewForwarder \
    chart2/source/controller/accessibility/ChartElementFactory \
    chart2/source/controller/chartapiwrapper/AreaWrapper \
    chart2/source/controller/chartapiwrapper/AxisWrapper \
    chart2/source/controller/chartapiwrapper/Chart2ModelContact \
    chart2/source/controller/chartapiwrapper/ChartDataWrapper \
    chart2/source/controller/chartapiwrapper/ChartDocumentWrapper \
    chart2/source/controller/chartapiwrapper/DataSeriesPointWrapper \
    chart2/source/controller/chartapiwrapper/DiagramWrapper \
    chart2/source/controller/chartapiwrapper/GridWrapper \
    chart2/source/controller/chartapiwrapper/LegendWrapper \
    chart2/source/controller/chartapiwrapper/MinMaxLineWrapper \
    chart2/source/controller/chartapiwrapper/TitleWrapper \
    chart2/source/controller/chartapiwrapper/UpDownBarWrapper \
    chart2/source/controller/chartapiwrapper/WallFloorWrapper \
    chart2/source/controller/chartapiwrapper/WrappedAddInProperty \
    chart2/source/controller/chartapiwrapper/WrappedAutomaticPositionProperties \
    chart2/source/controller/chartapiwrapper/WrappedAxisAndGridExistenceProperties \
    chart2/source/controller/chartapiwrapper/WrappedCharacterHeightProperty \
    chart2/source/controller/chartapiwrapper/WrappedDataCaptionProperties \
    chart2/source/controller/chartapiwrapper/WrappedGapwidthProperty \
    chart2/source/controller/chartapiwrapper/WrappedNumberFormatProperty \
    chart2/source/controller/chartapiwrapper/WrappedScaleProperty \
    chart2/source/controller/chartapiwrapper/WrappedScaleTextProperties \
    chart2/source/controller/chartapiwrapper/WrappedSceneProperty \
    chart2/source/controller/chartapiwrapper/WrappedSeriesAreaOrLineProperty \
    chart2/source/controller/chartapiwrapper/WrappedSplineProperties \
    chart2/source/controller/chartapiwrapper/WrappedStatisticProperties \
    chart2/source/controller/chartapiwrapper/WrappedStockProperties \
    chart2/source/controller/chartapiwrapper/WrappedSymbolProperties \
    chart2/source/controller/chartapiwrapper/WrappedTextRotationProperty \
    chart2/source/controller/dialogs/ChangingResource \
    chart2/source/controller/dialogs/ChartTypeDialogController \
    chart2/source/controller/dialogs/DataBrowser \
    chart2/source/controller/dialogs/DataBrowserModel \
    chart2/source/controller/dialogs/DialogModel \
    chart2/source/controller/dialogs/dlg_ChartType \
    chart2/source/controller/dialogs/dlg_ChartType_UNO \
    chart2/source/controller/dialogs/dlg_CreationWizard \
    chart2/source/controller/dialogs/dlg_CreationWizard_UNO \
    chart2/source/controller/dialogs/dlg_DataEditor \
    chart2/source/controller/dialogs/dlg_DataSource \
    chart2/source/controller/dialogs/dlg_InsertAxis_Grid \
    chart2/source/controller/dialogs/dlg_InsertDataLabel \
    chart2/source/controller/dialogs/dlg_InsertErrorBars \
    chart2/source/controller/dialogs/dlg_InsertLegend \
    chart2/source/controller/dialogs/dlg_InsertTitle \
    chart2/source/controller/dialogs/dlg_NumberFormat \
    chart2/source/controller/dialogs/dlg_ObjectProperties \
    chart2/source/controller/dialogs/dlg_ShapeFont \
    chart2/source/controller/dialogs/dlg_ShapeParagraph \
    chart2/source/controller/dialogs/dlg_View3D \
    chart2/source/controller/dialogs/ObjectNameProvider \
    chart2/source/controller/dialogs/RangeEdit \
    chart2/source/controller/dialogs/RangeSelectionButton \
    chart2/source/controller/dialogs/RangeSelectionHelper \
    chart2/source/controller/dialogs/RangeSelectionListener \
    chart2/source/controller/dialogs/res_BarGeometry \
    chart2/source/controller/dialogs/res_DataLabel \
    chart2/source/controller/dialogs/res_ErrorBar \
    chart2/source/controller/dialogs/res_LegendPosition \
    chart2/source/controller/dialogs/res_TextSeparator \
    chart2/source/controller/dialogs/res_Titles \
    chart2/source/controller/dialogs/res_Trendline \
    chart2/source/controller/dialogs/TextDirectionListBox \
    chart2/source/controller/dialogs/TimerTriggeredControllerLock \
    chart2/source/controller/dialogs/TitleDialogData \
    chart2/source/controller/dialogs/tp_3D_SceneAppearance \
    chart2/source/controller/dialogs/tp_3D_SceneGeometry \
    chart2/source/controller/dialogs/tp_3D_SceneIllumination \
    chart2/source/controller/dialogs/tp_AxisLabel \
    chart2/source/controller/dialogs/tp_AxisPositions \
    chart2/source/controller/dialogs/tp_ChartType \
    chart2/source/controller/dialogs/tp_DataLabel \
    chart2/source/controller/dialogs/tp_DataSourceControls \
    chart2/source/controller/dialogs/tp_DataSource \
    chart2/source/controller/dialogs/tp_ErrorBars \
    chart2/source/controller/dialogs/tp_LegendPosition \
    chart2/source/controller/dialogs/tp_PointGeometry \
    chart2/source/controller/dialogs/tp_PolarOptions \
    chart2/source/controller/dialogs/tp_RangeChooser \
    chart2/source/controller/dialogs/tp_Scale \
    chart2/source/controller/dialogs/tp_SeriesToAxis \
    chart2/source/controller/dialogs/tp_TitleRotation \
    chart2/source/controller/dialogs/tp_Trendline \
    chart2/source/controller/dialogs/tp_Wizard_TitlesAndObjects \
    chart2/source/controller/drawinglayer/DrawViewWrapper \
    chart2/source/controller/drawinglayer/ViewElementListProvider \
    chart2/source/controller/itemsetwrapper/AxisItemConverter \
    chart2/source/controller/itemsetwrapper/CharacterPropertyItemConverter \
    chart2/source/controller/itemsetwrapper/DataPointItemConverter \
    chart2/source/controller/itemsetwrapper/ErrorBarItemConverter \
    chart2/source/controller/itemsetwrapper/GraphicPropertyItemConverter \
    chart2/source/controller/itemsetwrapper/ItemConverter \
    chart2/source/controller/itemsetwrapper/LegendItemConverter \
    chart2/source/controller/itemsetwrapper/MultipleChartConverters \
    chart2/source/controller/itemsetwrapper/MultipleItemConverter \
    chart2/source/controller/itemsetwrapper/RegressionCurveItemConverter \
    chart2/source/controller/itemsetwrapper/RegressionEquationItemConverter \
    chart2/source/controller/itemsetwrapper/SeriesOptionsItemConverter \
    chart2/source/controller/itemsetwrapper/StatisticsItemConverter \
    chart2/source/controller/itemsetwrapper/TitleItemConverter \
    chart2/source/controller/main/ChartController \
    chart2/source/controller/main/ChartController_EditData \
    chart2/source/controller/main/ChartController_Insert \
    chart2/source/controller/main/ChartController_Position \
    chart2/source/controller/main/ChartController_Properties \
    chart2/source/controller/main/ChartController_TextEdit \
    chart2/source/controller/main/ChartController_Tools \
    chart2/source/controller/main/ChartController_Window \
    chart2/source/controller/main/ChartDropTargetHelper \
    chart2/source/controller/main/ChartFrameloader \
    chart2/source/controller/main/ChartModelClone \
    chart2/source/controller/main/ChartTransferable \
    chart2/source/controller/main/ChartWindow \
    chart2/source/controller/main/CommandDispatchContainer \
    chart2/source/controller/main/CommandDispatch \
    chart2/source/controller/main/ConfigurationAccess \
    chart2/source/controller/main/ControllerCommandDispatch \
    chart2/source/controller/main/DragMethod_Base \
    chart2/source/controller/main/DragMethod_PieSegment \
    chart2/source/controller/main/DragMethod_RotateDiagram \
    chart2/source/controller/main/DrawCommandDispatch \
    chart2/source/controller/main/ElementSelector \
    chart2/source/controller/main/FeatureCommandDispatchBase \
    chart2/source/controller/main/ObjectHierarchy \
    chart2/source/controller/main/PositionAndSizeHelper \
    chart2/source/controller/main/SelectionHelper \
    chart2/source/controller/main/_serviceregistration_controller \
    chart2/source/controller/main/ShapeController \
    chart2/source/controller/main/ShapeToolbarController \
    chart2/source/controller/main/StatusBarCommandDispatch \
    chart2/source/controller/main/UndoActions \
    chart2/source/controller/main/UndoCommandDispatch \
    chart2/source/controller/main/UndoGuard \
))

# Runtime dependency for unit-tests
$(eval $(call gb_Library_use_restarget,chartcontroller,chartcontroller))

# vim: set noet sw=4 ts=4:
