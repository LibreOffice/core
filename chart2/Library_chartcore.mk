# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,chartcore))

$(eval $(call gb_Library_set_include,chartcore,\
    $$(INCLUDE) \
    -I$(SRCDIR)/chart2/source/model/inc \
    -I$(SRCDIR)/chart2/source/view/inc \
    -I$(SRCDIR)/chart2/source/inc \
))

# not ideal - we should use a single core define ideally
$(eval $(call gb_Library_add_defs,chartcore,\
    -DOOO_DLLIMPLEMENTATION_CHARTTOOLS \
    -DOOO_DLLIMPLEMENTATION_CHARTVIEW \
))

$(eval $(call gb_Library_use_sdk_api,chartcore))

$(eval $(call gb_Library_use_libraries,chartcore,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
	drawinglayer \
	editeng \
    fwe \
    i18nisolang1 \
    sal \
	sfx \
    svl \
    svt \
	svxcore \
    tl \
    ucbhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_set_componentfile,chartcore,chart2/source/chartcore))

# view pieces ...
$(eval $(call gb_Library_add_exception_objects,chartcore,\
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
    chart2/source/view/charttypes/PieChart \
    chart2/source/view/charttypes/Splines \
    chart2/source/view/charttypes/VSeriesPlotter \
    chart2/source/view/diagram/VDiagram \
    chart2/source/view/main/ChartItemPool \
    chart2/source/view/main/ChartView \
    chart2/source/view/main/Clipping \
    chart2/source/view/main/DataPointSymbolSupplier \
    chart2/source/view/main/DrawModelWrapper \
    chart2/source/view/main/LabelPositionHelper \
    chart2/source/view/main/Linear3DTransformation \
    chart2/source/view/main/PlotterBase \
    chart2/source/view/main/PlottingPositionHelper \
    chart2/source/view/main/PolarLabelPositionHelper \
    chart2/source/view/main/PropertyMapper \
    chart2/source/view/main/_serviceregistration_view \
    chart2/source/view/main/ShapeFactory \
    chart2/source/view/main/Stripe \
    chart2/source/view/main/VDataSeries \
    chart2/source/view/main/VLegend \
    chart2/source/view/main/VLegendSymbolFactory \
    chart2/source/view/main/VLineProperties \
    chart2/source/view/main/VPolarTransformation \
    chart2/source/view/main/VTitle \
))

# model pieces ...
$(eval $(call gb_Library_add_exception_objects,chartcore,\
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
    chart2/source/model/main/Diagram \
    chart2/source/model/main/FormattedString \
    chart2/source/model/main/GridProperties \
    chart2/source/model/main/Legend \
    chart2/source/model/main/PageBackground \
    chart2/source/model/main/PolarCoordinateSystem \
    chart2/source/model/main/_serviceregistration_model \
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
    chart2/source/model/template/LineChartType \
    chart2/source/model/template/LineChartTypeTemplate \
    chart2/source/model/template/NetChartType \
    chart2/source/model/template/NetChartTypeTemplate \
    chart2/source/model/template/PieChartType \
    chart2/source/model/template/PieChartTypeTemplate \
    chart2/source/model/template/ScatterChartType \
    chart2/source/model/template/ScatterChartTypeTemplate \
    chart2/source/model/template/_serviceregistration_charttypes \
    chart2/source/model/template/StockChartTypeTemplate \
    chart2/source/model/template/StockDataInterpreter \
    chart2/source/model/template/XYDataInterpreter \
))

# tools pieces
$(eval $(call gb_Library_add_exception_objects,chartcore,\
    chart2/source/tools/AxisHelper \
    chart2/source/tools/BaseGFXHelper \
    chart2/source/tools/CachedDataSequence \
    chart2/source/tools/CharacterProperties \
    chart2/source/tools/ChartDebugTrace \
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
    chart2/source/tools/ImplOPropertySet \
    chart2/source/tools/InternalData \
    chart2/source/tools/InternalDataProvider \
    chart2/source/tools/LabeledDataSequence \
    chart2/source/tools/LegendHelper \
    chart2/source/tools/LifeTime \
    chart2/source/tools/LinearRegressionCurveCalculator \
    chart2/source/tools/LineProperties \
    chart2/source/tools/LogarithmicRegressionCurveCalculator \
    chart2/source/tools/MeanValueRegressionCurveCalculator \
    chart2/source/tools/MediaDescriptorHelper \
    chart2/source/tools/ModifyListenerCallBack \
    chart2/source/tools/ModifyListenerHelper \
    chart2/source/tools/MutexContainer \
    chart2/source/tools/NameContainer \
    chart2/source/tools/NumberFormatterWrapper \
    chart2/source/tools/ObjectIdentifier \
    chart2/source/tools/OPropertySet \
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
    chart2/source/tools/ResourceManager \
    chart2/source/tools/Scaling \
    chart2/source/tools/SceneProperties \
    chart2/source/tools/_serviceregistration_tools \
    chart2/source/tools/StatisticsHelper \
    chart2/source/tools/ThreeDHelper \
    chart2/source/tools/TitleHelper \
    chart2/source/tools/TrueGuard \
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
