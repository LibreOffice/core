# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,chart2))

$(eval $(call gb_Library_set_include,chart2,\
    $$(INCLUDE) \
    -I$(SRCDIR)/chart2/source/controller/inc \
    -I$(SRCDIR)/chart2/source/model/inc \
    -I$(SRCDIR)/chart2/source/view/inc \
    -I$(SRCDIR)/chart2/source/inc \
    -I$(SRCDIR)/chart2/inc \
))

# not ideal - we should use a single core define ideally
$(eval $(call gb_Library_add_defs,chart2,\
    -DOOO_DLLIMPLEMENTATION_CHARTTOOLS \
    -DOOO_DLLIMPLEMENTATION_CHARTVIEW \
))

$(eval $(call gb_Library_set_precompiled_header,chart2,chart2/inc/pch/precompiled_chart2))

$(eval $(call gb_Library_use_externals,chart2,\
	boost_headers \
	libxml2 \
))


$(eval $(call gb_Library_use_sdk_api,chart2))

$(eval $(call gb_Library_use_custom_headers,chart2,\
        officecfg/registry \
))

$(eval $(call gb_Library_use_libraries,chart2,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    docmodel \
    editeng \
    fwk \
    sal \
    salhelper \
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
))

$(eval $(call gb_Library_set_componentfile,chart2,chart2/source/chart2,services))

ifneq ($(ENABLE_WASM_STRIP_ACCESSIBILITY),TRUE)
$(eval $(call gb_Library_add_exception_objects,chart2,\
    chart2/source/controller/accessibility/AccessibleBase \
    chart2/source/controller/accessibility/AccessibleChartElement \
    chart2/source/controller/accessibility/AccessibleChartShape \
    chart2/source/controller/accessibility/AccessibleChartView \
    chart2/source/controller/accessibility/AccessibleTextHelper \
    chart2/source/controller/accessibility/AccessibleViewForwarder \
    chart2/source/controller/accessibility/ChartElementFactory \
))
endif

$(eval $(call gb_Library_add_exception_objects,chart2,\
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
    chart2/source/controller/dialogs/ChartResourceGroupDlgs \
    chart2/source/controller/dialogs/ChartResourceGroups \
    chart2/source/controller/dialogs/ChartTypeDialogController \
    chart2/source/controller/dialogs/DataBrowser \
    chart2/source/controller/dialogs/dlg_ChartType \
    chart2/source/controller/dialogs/dlg_ChartType_UNO \
    chart2/source/controller/dialogs/dlg_CreationWizard \
    chart2/source/controller/dialogs/dlg_CreationWizard_UNO \
    chart2/source/controller/dialogs/dlg_DataEditor \
    chart2/source/controller/dialogs/dlg_DataSource \
    chart2/source/controller/dialogs/dlg_InsertAxis_Grid \
    chart2/source/controller/dialogs/dlg_InsertDataLabel \
    chart2/source/controller/dialogs/dlg_InsertDataTable \
    chart2/source/controller/dialogs/dlg_InsertErrorBars \
    chart2/source/controller/dialogs/dlg_InsertLegend \
    chart2/source/controller/dialogs/dlg_InsertTitle \
    chart2/source/controller/dialogs/dlg_NumberFormat \
    chart2/source/controller/dialogs/dlg_ObjectProperties \
    chart2/source/controller/dialogs/dlg_ShapeFont \
    chart2/source/controller/dialogs/dlg_ShapeParagraph \
    chart2/source/controller/dialogs/dlg_View3D \
    chart2/source/controller/dialogs/ObjectNameProvider \
    chart2/source/controller/dialogs/res_BarGeometry \
    chart2/source/controller/dialogs/res_DataLabel \
    chart2/source/controller/dialogs/res_DataTableProperties \
    chart2/source/controller/dialogs/res_ErrorBar \
    chart2/source/controller/dialogs/res_LegendPosition \
    chart2/source/controller/dialogs/res_Titles \
    chart2/source/controller/dialogs/res_Trendline \
    chart2/source/controller/dialogs/TextDirectionListBox \
    chart2/source/controller/dialogs/TitleDialogData \
    chart2/source/controller/dialogs/tp_3D_SceneAppearance \
    chart2/source/controller/dialogs/tp_3D_SceneGeometry \
    chart2/source/controller/dialogs/tp_3D_SceneIllumination \
    chart2/source/controller/dialogs/tp_AxisLabel \
    chart2/source/controller/dialogs/tp_AxisPositions \
    chart2/source/controller/dialogs/tp_ChartType \
    chart2/source/controller/dialogs/tp_DataLabel \
    chart2/source/controller/dialogs/tp_DataPointOption \
    chart2/source/controller/dialogs/tp_DataSource \
    chart2/source/controller/dialogs/tp_DataTable \
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
    chart2/source/controller/itemsetwrapper/DataTableItemConverter \
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
    chart2/source/controller/itemsetwrapper/TextLabelItemConverter \
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
    chart2/source/controller/main/ShapeController \
    chart2/source/controller/main/StatusBarCommandDispatch \
    chart2/source/controller/main/ToolbarController \
    chart2/source/controller/main/UndoActions \
    chart2/source/controller/main/UndoCommandDispatch \
    chart2/source/controller/main/UndoGuard \
    chart2/source/controller/sidebar/Chart2PanelFactory \
    chart2/source/controller/sidebar/ChartAreaPanel \
    chart2/source/controller/sidebar/ChartAxisPanel \
    chart2/source/controller/sidebar/ChartColorWrapper \
    chart2/source/controller/sidebar/ChartElementsPanel \
    chart2/source/controller/sidebar/ChartErrorBarPanel \
    chart2/source/controller/sidebar/ChartLinePanel \
    chart2/source/controller/sidebar/ChartSeriesPanel \
    chart2/source/controller/sidebar/ChartSidebarModifyListener \
    chart2/source/controller/sidebar/ChartSidebarSelectionListener \
    chart2/source/controller/sidebar/ChartTypePanel \
    chart2/source/controller/uitest/uiobject \
))

# view pieces ...
$(eval $(call gb_Library_add_exception_objects,chart2,\
    chart2/source/controller/dialogs/DataBrowserModel \
    chart2/source/controller/dialogs/DialogModel \
    chart2/source/controller/dialogs/RangeSelectionHelper \
    chart2/source/controller/dialogs/RangeSelectionListener \
    chart2/source/controller/dialogs/TimerTriggeredControllerLock \
    chart2/source/view/axes/DateHelper \
    chart2/source/view/axes/DateScaling \
    chart2/source/view/axes/MinimumAndMaximumSupplier \
    chart2/source/view/axes/ScaleAutomatism \
    chart2/source/view/axes/Tickmarks \
    chart2/source/view/axes/Tickmarks_Dates \
    chart2/source/view/axes/Tickmarks_Equidistant \
    chart2/source/view/axes/VAxisBase \
    chart2/source/view/axes/VAxisOrGridBase \
    chart2/source/view/axes/VAxisProperties \
    chart2/source/view/axes/VCartesianAxis \
    chart2/source/view/axes/VCartesianCoordinateSystem \
    chart2/source/view/axes/VCartesianGrid \
    chart2/source/view/axes/VCoordinateSystem \
    chart2/source/view/axes/VPolarAngleAxis \
    chart2/source/view/axes/VPolarAxis \
    chart2/source/view/axes/VPolarCoordinateSystem \
    chart2/source/view/axes/VPolarGrid \
    chart2/source/view/axes/VPolarRadiusAxis \
    chart2/source/view/charttypes/AreaChart \
    chart2/source/view/charttypes/BarChart \
    chart2/source/view/charttypes/BarPositionHelper \
    chart2/source/view/charttypes/BubbleChart \
    chart2/source/view/charttypes/CandleStickChart \
    chart2/source/view/charttypes/CategoryPositionHelper \
    chart2/source/view/charttypes/HistogramChart \
    chart2/source/view/charttypes/NetChart \
    chart2/source/view/charttypes/PieChart \
    chart2/source/view/charttypes/Splines \
    chart2/source/view/charttypes/VSeriesPlotter \
    chart2/source/view/diagram/VDiagram \
    chart2/source/view/main/ChartItemPool \
    chart2/source/view/main/ChartView \
    chart2/source/view/main/Clipping \
    chart2/source/view/main/DataPointSymbolSupplier \
    chart2/source/view/main/DataTableView \
    chart2/source/view/main/DrawModelWrapper \
    chart2/source/view/main/ExplicitValueProvider \
    chart2/source/view/main/LabelPositionHelper \
    chart2/source/view/main/Linear3DTransformation \
    chart2/source/view/main/PlotterBase \
    chart2/source/view/main/PlottingPositionHelper \
    chart2/source/view/main/PolarLabelPositionHelper \
    chart2/source/view/main/PropertyMapper \
    chart2/source/view/main/SeriesPlotterContainer \
    chart2/source/view/main/ShapeFactory \
    chart2/source/view/main/Stripe \
    chart2/source/view/main/VDataSeries \
    chart2/source/view/main/VLegend \
    chart2/source/view/main/VLegendSymbolFactory \
    chart2/source/view/main/VLineProperties \
    chart2/source/view/main/VPolarTransformation \
    chart2/source/view/main/VTitle \
    chart2/source/view/main/VButton \
))

# model pieces ...
$(eval $(call gb_Library_add_exception_objects,chart2,\
    chart2/source/model/filter/XMLFilter \
    chart2/source/model/main/Axis \
    chart2/source/model/main/BaseCoordinateSystem \
    chart2/source/model/main/CartesianCoordinateSystem \
    chart2/source/model/main/ChartModel \
    chart2/source/model/main/ChartModel_Persistence \
    chart2/source/model/main/DataPoint \
    chart2/source/model/main/DataPointProperties \
    chart2/source/model/main/DataSeries \
    chart2/source/model/main/DataSeriesProperties \
    chart2/source/model/main/DataTable \
    chart2/source/model/main/Diagram \
    chart2/source/model/main/FormattedString \
    chart2/source/model/main/GridProperties \
    chart2/source/model/main/Legend \
    chart2/source/model/main/PageBackground \
    chart2/source/model/main/PolarCoordinateSystem \
    chart2/source/model/main/StockBar \
    chart2/source/model/main/Title \
    chart2/source/model/main/UndoManager \
    chart2/source/model/main/Wall \
    chart2/source/model/template/AreaChartType \
    chart2/source/model/template/AreaChartTypeTemplate \
    chart2/source/model/template/BarChartType \
    chart2/source/model/template/BarChartTypeTemplate \
    chart2/source/model/template/BubbleChartType \
    chart2/source/model/template/BubbleChartTypeTemplate \
    chart2/source/model/template/BubbleDataInterpreter \
    chart2/source/model/template/CandleStickChartType \
    chart2/source/model/template/ChartType \
    chart2/source/model/template/ChartTypeManager \
    chart2/source/model/template/ChartTypeTemplate \
    chart2/source/model/template/ColumnChartType \
    chart2/source/model/template/ColumnLineChartTypeTemplate \
    chart2/source/model/template/ColumnLineDataInterpreter \
    chart2/source/model/template/DataInterpreter \
    chart2/source/model/template/FilledNetChartType \
    chart2/source/model/template/HistogramChartType \
    chart2/source/model/template/HistogramChartTypeTemplate \
    chart2/source/model/template/HistogramDataInterpreter \
    chart2/source/model/template/HistogramCalculator \
    chart2/source/model/template/LineChartType \
    chart2/source/model/template/LineChartTypeTemplate \
    chart2/source/model/template/NetChartType \
    chart2/source/model/template/NetChartTypeTemplate \
    chart2/source/model/template/PieChartType \
    chart2/source/model/template/PieChartTypeTemplate \
    chart2/source/model/template/ScatterChartType \
    chart2/source/model/template/ScatterChartTypeTemplate \
    chart2/source/model/template/StockChartTypeTemplate \
    chart2/source/model/template/StockDataInterpreter \
    chart2/source/model/template/XYDataInterpreter \
))

# tools pieces
$(eval $(call gb_Library_add_exception_objects,chart2,\
    chart2/source/tools/AxisHelper \
    chart2/source/tools/BaseGFXHelper \
    chart2/source/tools/CachedDataSequence \
    chart2/source/tools/CharacterProperties \
    chart2/source/tools/ChartModelHelper \
    chart2/source/tools/ChartTypeHelper \
    chart2/source/tools/ChartViewHelper \
    chart2/source/tools/ColorPerPointHelper \
    chart2/source/tools/CommonConverters \
    chart2/source/tools/ConfigColorScheme \
    chart2/source/tools/ControllerLockGuard \
    chart2/source/tools/DataSeriesHelper \
    chart2/source/tools/DataSource \
    chart2/source/tools/DataSourceHelper \
    chart2/source/tools/DiagramHelper \
    chart2/source/tools/ErrorBar \
    chart2/source/tools/ExplicitCategoriesProvider \
    chart2/source/tools/ExponentialRegressionCurveCalculator \
    chart2/source/tools/FillProperties \
    chart2/source/tools/FormattedStringHelper \
    chart2/source/tools/HistogramDataSequence \
    chart2/source/tools/InternalData \
    chart2/source/tools/InternalDataProvider \
    chart2/source/tools/LabeledDataSequence \
    chart2/source/tools/LegendHelper \
    chart2/source/tools/LifeTime \
    chart2/source/tools/LinearRegressionCurveCalculator \
    chart2/source/tools/LinePropertiesHelper \
    chart2/source/tools/LogarithmicRegressionCurveCalculator \
    chart2/source/tools/MeanValueRegressionCurveCalculator \
    chart2/source/tools/MediaDescriptorHelper \
    chart2/source/tools/ModifyListenerCallBack \
    chart2/source/tools/ModifyListenerHelper \
    chart2/source/tools/MovingAverageRegressionCurveCalculator \
    chart2/source/tools/NameContainer \
    chart2/source/tools/NumberFormatterWrapper \
    chart2/source/tools/ObjectIdentifier \
    chart2/source/tools/OPropertySet \
    chart2/source/tools/PolynomialRegressionCurveCalculator \
    chart2/source/tools/PopupRequest \
    chart2/source/tools/PotentialRegressionCurveCalculator \
    chart2/source/tools/PropertyHelper \
    chart2/source/tools/RangeHighlighter \
    chart2/source/tools/ReferenceSizeProvider \
    chart2/source/tools/RegressionCurveCalculator \
    chart2/source/tools/RegressionCurveHelper \
    chart2/source/tools/RegressionCurveModel \
    chart2/source/tools/RegressionEquation \
    chart2/source/tools/RelativePositionHelper \
    chart2/source/tools/RelativeSizeHelper \
    chart2/source/tools/ResId \
    chart2/source/tools/Scaling \
    chart2/source/tools/SceneProperties \
    chart2/source/tools/StatisticsHelper \
    chart2/source/tools/ThreeDHelper \
    chart2/source/tools/TitleHelper \
    chart2/source/tools/UncachedDataSequence \
    chart2/source/tools/UserDefinedProperties \
    chart2/source/tools/WeakListenerAdapter \
    chart2/source/tools/WrappedDefaultProperty \
    chart2/source/tools/WrappedDirectStateProperty \
    chart2/source/tools/WrappedIgnoreProperty \
    chart2/source/tools/WrappedProperty \
    chart2/source/tools/WrappedPropertySet \
    chart2/source/tools/XMLRangeHelper \
))

# vim: set noet sw=4 ts=4:
