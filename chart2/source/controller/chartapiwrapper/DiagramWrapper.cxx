/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "DiagramWrapper.hxx"
#include <servicenames_charttypes.hxx>
#include "DataSeriesPointWrapper.hxx"
#include <DataSeriesProperties.hxx>
#include "AxisWrapper.hxx"
#include "Chart2ModelContact.hxx"
#include "WallFloorWrapper.hxx"
#include "MinMaxLineWrapper.hxx"
#include "UpDownBarWrapper.hxx"
#include <Diagram.hxx>
#include <DiagramHelper.hxx>
#include <DataSourceHelper.hxx>
#include <ChartModelHelper.hxx>
#include <ChartType.hxx>
#include <DataSeries.hxx>
#include <WrappedIgnoreProperty.hxx>
#include "WrappedAxisAndGridExistenceProperties.hxx"
#include "WrappedStatisticProperties.hxx"
#include "WrappedSymbolProperties.hxx"
#include "WrappedDataCaptionProperties.hxx"
#include "WrappedSplineProperties.hxx"
#include "WrappedStockProperties.hxx"
#include "WrappedSceneProperty.hxx"
#include <ControllerLockGuard.hxx>
#include <DisposeHelper.hxx>
#include "WrappedAutomaticPositionProperties.hxx"
#include <CommonConverters.hxx>
#include <unonames.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <ChartTypeManager.hxx>
#include <ChartTypeTemplate.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/PieChartSubType.hpp>
#include <com/sun/star/chart/ChartSolidType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <UserDefinedProperties.hxx>
#include <SceneProperties.hxx>

#include <algorithm>
#include <map>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/string_view.hxx>
#include <utility>

using namespace ::com::sun::star;
using namespace ::chart::wrapper;
using namespace ::chart::DataSeriesProperties;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::com::sun::star::chart::XAxis;

namespace
{

enum
{
    PROP_DIAGRAM_ATTRIBUTED_DATA_POINTS,
    PROP_DIAGRAM_PERCENT_STACKED,
    PROP_DIAGRAM_STACKED,
    PROP_DIAGRAM_THREE_D,
    PROP_DIAGRAM_SOLIDTYPE,
    PROP_DIAGRAM_DEEP,
    PROP_DIAGRAM_VERTICAL,
    PROP_DIAGRAM_NUMBER_OF_LINES,
    PROP_DIAGRAM_STACKED_BARS_CONNECTED,
    PROP_DIAGRAM_DATAROW_SOURCE,

    PROP_DIAGRAM_GROUP_BARS_PER_AXIS,
    PROP_DIAGRAM_INCLUDE_HIDDEN_CELLS,

    PROP_DIAGRAM_SORT_BY_X_VALUES,

    PROP_DIAGRAM_STARTING_ANGLE,
    PROP_DIAGRAM_OF_PIE_TYPE,

    PROP_DIAGRAM_RIGHT_ANGLED_AXES,
    PROP_DIAGRAM_PERSPECTIVE,
    PROP_DIAGRAM_ROTATION_HORIZONTAL,
    PROP_DIAGRAM_ROTATION_VERTICAL,

    PROP_DIAGRAM_MISSING_VALUE_TREATMENT,

    PROP_DIAGRAM_HAS_X_AXIS,
    PROP_DIAGRAM_HAS_X_AXIS_DESCR,
    PROP_DIAGRAM_HAS_X_AXIS_TITLE,
    PROP_DIAGRAM_HAS_X_AXIS_GRID,
    PROP_DIAGRAM_HAS_X_AXIS_HELP_GRID,

    PROP_DIAGRAM_HAS_Y_AXIS,
    PROP_DIAGRAM_HAS_Y_AXIS_DESCR,
    PROP_DIAGRAM_HAS_Y_AXIS_TITLE,
    PROP_DIAGRAM_HAS_Y_AXIS_GRID,
    PROP_DIAGRAM_HAS_Y_AXIS_HELP_GRID,

    PROP_DIAGRAM_HAS_Z_AXIS,
    PROP_DIAGRAM_HAS_Z_AXIS_DESCR,
    PROP_DIAGRAM_HAS_Z_AXIS_TITLE,
    PROP_DIAGRAM_HAS_Z_AXIS_GRID,
    PROP_DIAGRAM_HAS_Z_AXIS_HELP_GRID,

    PROP_DIAGRAM_HAS_SECOND_X_AXIS,
    PROP_DIAGRAM_HAS_SECOND_X_AXIS_DESCR,

    PROP_DIAGRAM_HAS_SECOND_Y_AXIS,
    PROP_DIAGRAM_HAS_SECOND_Y_AXIS_DESCR,

    PROP_DIAGRAM_HAS_SECOND_X_AXIS_TITLE,
    PROP_DIAGRAM_HAS_SECOND_Y_AXIS_TITLE,

    PROP_DIAGRAM_AUTOMATIC_SIZE,
    PROP_DIAGRAM_EXTERNALDATA
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "AttributedDataPoints",
                  PROP_DIAGRAM_ATTRIBUTED_DATA_POINTS,
                  cppu::UnoType<uno::Sequence< uno::Sequence< sal_Int32 > >>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    // see com.sun.star.chart.StackableDiagram
    rOutProperties.emplace_back( "Percent",
                  PROP_DIAGRAM_PERCENT_STACKED,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "Stacked",
                  PROP_DIAGRAM_STACKED,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "Dim3D",
                  PROP_DIAGRAM_THREE_D,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // see com.sun.star.chart.Chart3DBarProperties
    rOutProperties.emplace_back( "SolidType",
                  PROP_DIAGRAM_SOLIDTYPE,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // see com.sun.star.chart.BarDiagram
    rOutProperties.emplace_back( "Deep",
                  PROP_DIAGRAM_DEEP,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "Vertical",
                  PROP_DIAGRAM_VERTICAL,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "NumberOfLines",
                  PROP_DIAGRAM_NUMBER_OF_LINES,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "StackedBarsConnected",
                  PROP_DIAGRAM_STACKED_BARS_CONNECTED,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "DataRowSource",
                  PROP_DIAGRAM_DATAROW_SOURCE,
                  cppu::UnoType<css::chart::ChartDataRowSource>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "GroupBarsPerAxis",
                  PROP_DIAGRAM_GROUP_BARS_PER_AXIS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "IncludeHiddenCells",
                  PROP_DIAGRAM_INCLUDE_HIDDEN_CELLS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    //new for XY charts
    rOutProperties.emplace_back( CHART_UNONAME_SORT_BY_XVALUES,
                  PROP_DIAGRAM_SORT_BY_X_VALUES,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    //for pie and donut charts
    rOutProperties.emplace_back( "StartingAngle",
                  PROP_DIAGRAM_STARTING_ANGLE,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "SubPieType",
                  PROP_DIAGRAM_OF_PIE_TYPE,
                  cppu::UnoType<chart2::PieChartSubType>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    //new for 3D charts
    rOutProperties.emplace_back( "RightAngledAxes",
                  PROP_DIAGRAM_RIGHT_ANGLED_AXES,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "Perspective",
                  PROP_DIAGRAM_PERSPECTIVE,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "RotationHorizontal",
                  PROP_DIAGRAM_ROTATION_HORIZONTAL,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "RotationVertical",
                  PROP_DIAGRAM_ROTATION_VERTICAL,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::MAYBEVOID );

    // XAxisXSupplier
    rOutProperties.emplace_back( "HasXAxis",
                  PROP_DIAGRAM_HAS_X_AXIS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasXAxisDescription",
                  PROP_DIAGRAM_HAS_X_AXIS_DESCR,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasXAxisTitle",
                  PROP_DIAGRAM_HAS_X_AXIS_TITLE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasXAxisGrid",
                  PROP_DIAGRAM_HAS_X_AXIS_GRID,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasXAxisHelpGrid",
                  PROP_DIAGRAM_HAS_X_AXIS_HELP_GRID,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // XAxisYSupplier
    rOutProperties.emplace_back( "HasYAxis",
                  PROP_DIAGRAM_HAS_Y_AXIS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasYAxisDescription",
                  PROP_DIAGRAM_HAS_Y_AXIS_DESCR,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasYAxisTitle",
                  PROP_DIAGRAM_HAS_Y_AXIS_TITLE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasYAxisGrid",
                  PROP_DIAGRAM_HAS_Y_AXIS_GRID,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasYAxisHelpGrid",
                  PROP_DIAGRAM_HAS_Y_AXIS_HELP_GRID,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // XAxisZSupplier
    rOutProperties.emplace_back( "HasZAxis",
                  PROP_DIAGRAM_HAS_Z_AXIS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasZAxisDescription",
                  PROP_DIAGRAM_HAS_Z_AXIS_DESCR,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasZAxisTitle",
                  PROP_DIAGRAM_HAS_Z_AXIS_TITLE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasZAxisGrid",
                  PROP_DIAGRAM_HAS_Z_AXIS_GRID,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasZAxisHelpGrid",
                  PROP_DIAGRAM_HAS_Z_AXIS_HELP_GRID,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // XTwoAxisXSupplier
    rOutProperties.emplace_back( "HasSecondaryXAxis",
                  PROP_DIAGRAM_HAS_SECOND_X_AXIS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasSecondaryXAxisDescription",
                  PROP_DIAGRAM_HAS_SECOND_X_AXIS_DESCR,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // XTwoAxisYSupplier
    rOutProperties.emplace_back( "HasSecondaryYAxis",
                  PROP_DIAGRAM_HAS_SECOND_Y_AXIS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasSecondaryYAxisDescription",
                  PROP_DIAGRAM_HAS_SECOND_Y_AXIS_DESCR,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // XSecondAxisTitleSupplier
    rOutProperties.emplace_back( "HasSecondaryXAxisTitle",
                  PROP_DIAGRAM_HAS_SECOND_X_AXIS_TITLE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "HasSecondaryYAxisTitle",
                  PROP_DIAGRAM_HAS_SECOND_Y_AXIS_TITLE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "MissingValueTreatment",
                  PROP_DIAGRAM_MISSING_VALUE_TREATMENT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "AutomaticSize",
                  PROP_DIAGRAM_AUTOMATIC_SIZE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ExternalData",
                  PROP_DIAGRAM_EXTERNALDATA,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );
}

const Sequence< Property >& StaticDiagramWrapperPropertyArray()
{
    static Sequence< Property > aPropSeq = []()
        {
            std::vector< css::beans::Property > aProperties;
            lcl_AddPropertiesToVector( aProperties );
            ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
            ::chart::FillProperties::AddPropertiesToVector( aProperties );
            ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );
            ::chart::SceneProperties::AddPropertiesToVector( aProperties );
            WrappedStatisticProperties::addProperties( aProperties );
            WrappedSymbolProperties::addProperties( aProperties );
            WrappedDataCaptionProperties::addProperties( aProperties );
            WrappedSplineProperties::addProperties( aProperties );
            WrappedStockProperties::addProperties( aProperties );
            WrappedAutomaticPositionProperties::addProperties( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropSeq;
};

bool lcl_isXYChart( const rtl::Reference< ::chart::Diagram >& xDiagram )
{
    bool bRet = false;
    rtl::Reference< ::chart::ChartType > xChartType( xDiagram->getChartTypeByIndex( 0 ) );
    if( xChartType.is() )
    {
        OUString aChartType( xChartType->getChartType() );
        if( aChartType.equalsIgnoreAsciiCase(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
            bRet = true;
    }
    return bRet;
}

sal_Int32 lcl_getNewAPIIndexForOldAPIIndex(
                sal_Int32 nOldAPIIndex
                , const rtl::Reference< ::chart::Diagram >& xDiagram )
{
    sal_Int32 nNewAPIIndex = nOldAPIIndex;

    if( lcl_isXYChart( xDiagram ) )
    {
        if( nNewAPIIndex >= 1 )
            nNewAPIIndex -= 1;
    }

    std::vector< rtl::Reference< ::chart::DataSeries > > aSeriesList =
        xDiagram->getDataSeries();
    if( nNewAPIIndex >= static_cast<sal_Int32>(aSeriesList.size()) )
        nNewAPIIndex = -1;

    return nNewAPIIndex;
}

OUString lcl_getDiagramType( std::u16string_view rTemplateServiceName )
{
    static constexpr OUString aPrefix(u"com.sun.star.chart2.template."_ustr);

    if( o3tl::starts_with(rTemplateServiceName, aPrefix) )
    {
        const std::u16string_view aName( rTemplateServiceName.substr( aPrefix.getLength()));

        // "Area" "StackedArea" "PercentStackedArea" "ThreeDArea"
        // "StackedThreeDArea" "PercentStackedThreeDArea"
        if( aName.find( u"Area" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.AreaDiagram"_ustr;

        // Handle bar-of-pie and pie-of-pie before simple pie
        // "BarOfPie"
        if( aName.find( u"BarOfPie" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.BarOfPieDiagram"_ustr;

        // "PieOfPie"
        if( aName.find( u"PieOfPie" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.PieOfPieDiagram"_ustr;

        // "Pie" "PieAllExploded" "ThreeDPie" "ThreeDPieAllExploded"
        if( aName.find( u"Pie" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.PieDiagram"_ustr;

        // "Column" "StackedColumn" "PercentStackedColumn" "ThreeDColumnDeep"
        // "ThreeDColumnFlat" "StackedThreeDColumnFlat"
        // "PercentStackedThreeDColumnFlat" "Bar" "StackedBar"
        // "PercentStackedBar" "ThreeDBarDeep" "ThreeDBarFlat"
        // "StackedThreeDBarFlat" "PercentStackedThreeDBarFlat" "ColumnWithLine"
        // "StackedColumnWithLine"
        if( aName.find( u"Column" ) != std::u16string_view::npos || aName.find( u"Bar" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.BarDiagram"_ustr;

        // "Donut" "DonutAllExploded" "ThreeDDonut" "ThreeDDonutAllExploded"
        if( aName.find( u"Donut" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.DonutDiagram"_ustr;

        // "ScatterLineSymbol" "ScatterLine" "ScatterSymbol" "ThreeDScatter"
        if( aName.find( u"Scatter" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.XYDiagram"_ustr;

        // "FilledNet" "StackedFilledNet" "PercentStackedFilledNet"
        if( aName.find( u"FilledNet" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.FilledNetDiagram"_ustr;

        // "Net" "NetSymbol" "NetLine" "StackedNet" "StackedNetSymbol"
        // "StackedNetLine" "PercentStackedNet" "PercentStackedNetSymbol"
        // "PercentStackedNetLine"
        if( aName.find( u"Net" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.NetDiagram"_ustr;

        // "StockLowHighClose" "StockOpenLowHighClose" "StockVolumeLowHighClose"
        // "StockVolumeOpenLowHighClose"
        if( aName.find( u"Stock" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.StockDiagram"_ustr;

        if( aName.find( u"Bubble" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.BubbleDiagram"_ustr;

        // Note: this must be checked after Bar, Net and Scatter

        // "Symbol" "StackedSymbol" "PercentStackedSymbol" "Line" "StackedLine"
        // "PercentStackedLine" "LineSymbol" "StackedLineSymbol"
        // "PercentStackedLineSymbol" "ThreeDLine" "StackedThreeDLine"
        // "PercentStackedThreeDLine" "ThreeDLineDeep"
        if( aName.find( u"Line" ) != std::u16string_view::npos || aName.find( u"Symbol" ) != std::u16string_view::npos )
            return u"com.sun.star.chart.LineDiagram"_ustr;

        OSL_FAIL( "unknown template" );
    }

    return OUString();
}

typedef std::map< OUString, OUString > tMakeStringStringMap;

const tMakeStringStringMap& lcl_getChartTypeNameMap()
{
    static tMakeStringStringMap g_aChartTypeNameMap{
        {"com.sun.star.chart2.LineChartType", "com.sun.star.chart.LineDiagram"},
        {"com.sun.star.chart2.AreaChartType", "com.sun.star.chart.AreaDiagram"},
        {"com.sun.star.chart2.ColumnChartType", "com.sun.star.chart.BarDiagram"},
        {"com.sun.star.chart2.PieChartType", "com.sun.star.chart.PieDiagram"},
        {"com.sun.star.chart2.DonutChartType", "com.sun.star.chart.DonutDiagram"},
        {"com.sun.star.chart2.ScatterChartType", "com.sun.star.chart.XYDiagram"},
        {"com.sun.star.chart2.FilledNetChartType", "com.sun.star.chart.FilledNetDiagram"},
        {"com.sun.star.chart2.NetChartType", "com.sun.star.chart.NetDiagram"},
        {"com.sun.star.chart2.CandleStickChartType", "com.sun.star.chart.StockDiagram"},
        {"com.sun.star.chart2.BubbleChartType", "com.sun.star.chart.BubbleDiagram"}
    };
    return g_aChartTypeNameMap;
}

OUString lcl_getOldChartTypeName( const OUString & rNewChartTypeName )
{
    OUString aOld(rNewChartTypeName);

    const tMakeStringStringMap& rMap = lcl_getChartTypeNameMap();
    tMakeStringStringMap::const_iterator aIt( rMap.find( rNewChartTypeName ));
    if( aIt != rMap.end())
    {
        aOld = aIt->second;
    }
    return aOld;
}

} // anonymous namespace

namespace chart::wrapper
{

DiagramWrapper::DiagramWrapper(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
    : m_spChart2ModelContact(std::move(spChart2ModelContact))
{
}

DiagramWrapper::~DiagramWrapper()
{}

// ____ XDiagram ____
OUString SAL_CALL DiagramWrapper::getDiagramType()
{
    OUString aRet;

    rtl::Reference< ChartModel > xChartDoc( m_spChart2ModelContact->getDocumentModel() );
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( xChartDoc.is() && xDiagram.is() )
    {
        Reference< beans::XPropertySet > xChartDocProp( static_cast<cppu::OWeakObject*>(xChartDoc.get()), uno::UNO_QUERY );
        if( xChartDocProp.is() )
        {
            uno::Reference< util::XRefreshable > xAddIn;
            if( xChartDocProp->getPropertyValue( u"AddIn"_ustr ) >>= xAddIn )
            {
                uno::Reference< lang::XServiceName > xServiceName( xAddIn, uno::UNO_QUERY );
                if( xServiceName.is())
                    return xServiceName->getServiceName();
            }
        }

        rtl::Reference< ::chart::ChartTypeManager > xChartTypeManager = xChartDoc->getTypeManager();
        Diagram::tTemplateWithServiceName aTemplateAndService =
            xDiagram->getTemplate( xChartTypeManager );

        aRet = lcl_getDiagramType( aTemplateAndService.sServiceName );
    }

    if( !aRet.isEmpty())
        return aRet;

    // none of the standard templates matched
    // use first chart type
    if (xDiagram)
    {
        rtl::Reference< ChartType > xChartType( xDiagram->getChartTypeByIndex( 0 ) );
        if( xChartType.is() )
        {
            aRet = xChartType->getChartType();
            if( !aRet.isEmpty() )
                aRet = lcl_getOldChartTypeName( aRet );
        }
    }
    if( aRet.isEmpty())
        aRet = "com.sun.star.chart.BarDiagram";

    return aRet;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getDataRowProperties( sal_Int32 nRow )
{
    if( nRow < 0 )
        throw lang::IndexOutOfBoundsException(u"DataSeries index invalid"_ustr,
                                              static_cast< ::cppu::OWeakObject * >( this ));

    sal_Int32 nNewAPIIndex = lcl_getNewAPIIndexForOldAPIIndex( nRow, m_spChart2ModelContact->getDiagram() );
    if( nNewAPIIndex < 0 )
        throw lang::IndexOutOfBoundsException(u"DataSeries index invalid"_ustr,
                                              static_cast< ::cppu::OWeakObject * >( this ));

    Reference< beans::XPropertySet > xRet( new DataSeriesPointWrapper(
          DataSeriesPointWrapper::DATA_SERIES, nNewAPIIndex, 0, m_spChart2ModelContact ) );
    return xRet;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow )
{
    if( nCol < 0 || nRow < 0 )
        throw lang::IndexOutOfBoundsException(u"DataSeries index invalid"_ustr,
                                              static_cast< ::cppu::OWeakObject * >( this ));

    sal_Int32 nNewAPIIndex = lcl_getNewAPIIndexForOldAPIIndex( nRow, m_spChart2ModelContact->getDiagram() );
    if( nNewAPIIndex < 0 )
        throw lang::IndexOutOfBoundsException(u"DataSeries index invalid"_ustr,
                                              static_cast< ::cppu::OWeakObject * >( this ));

    //todo: check borders of point index

    Reference< beans::XPropertySet > xRet( new DataSeriesPointWrapper(
          DataSeriesPointWrapper::DATA_POINT, nNewAPIIndex, nCol, m_spChart2ModelContact ) );

    return xRet;
}

// ____ XShape (base of XDiagram) ____
awt::Point SAL_CALL DiagramWrapper::getPosition()
{
    awt::Point aPosition = ToPoint( m_spChart2ModelContact->GetDiagramRectangleIncludingAxes() );
    return aPosition;
}

void SAL_CALL DiagramWrapper::setPosition( const awt::Point& aPosition )
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getDocumentModel() );
    Reference< beans::XPropertySet > xProp( getInnerPropertySet() );
    if( !xProp.is() )
        return;

    awt::Size aPageSize( m_spChart2ModelContact->GetPageSize() );

    chart2::RelativePosition aRelativePosition;
    aRelativePosition.Anchor = drawing::Alignment_TOP_LEFT;
    aRelativePosition.Primary = double(aPosition.X)/double(aPageSize.Width);
    aRelativePosition.Secondary = double(aPosition.Y)/double(aPageSize.Height);
    if( aRelativePosition.Primary < 0 || aRelativePosition.Secondary < 0 || aRelativePosition.Primary > 1 || aRelativePosition.Secondary > 1 )
    {
        OSL_FAIL("DiagramWrapper::setPosition called with a position out of range -> automatic values are taken instead" );
        uno::Any aEmpty;
        xProp->setPropertyValue( u"RelativePosition"_ustr, aEmpty );
        return;
    }
    xProp->setPropertyValue( u"RelativePosition"_ustr, uno::Any(aRelativePosition) );
    xProp->setPropertyValue( u"PosSizeExcludeAxes"_ustr, uno::Any(false) );
}

awt::Size SAL_CALL DiagramWrapper::getSize()
{
    awt::Size aSize = ToSize( m_spChart2ModelContact->GetDiagramRectangleIncludingAxes() );
    return aSize;
}

void SAL_CALL DiagramWrapper::setSize( const awt::Size& aSize )
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getDocumentModel() );
    Reference< beans::XPropertySet > xProp( getInnerPropertySet() );
    if( !xProp.is() )
        return;

    awt::Size aPageSize( m_spChart2ModelContact->GetPageSize() );

    chart2::RelativeSize aRelativeSize;
    aRelativeSize.Primary = double(aSize.Width)/double(aPageSize.Width);
    aRelativeSize.Secondary = double(aSize.Height)/double(aPageSize.Height);

    if( aRelativeSize.Primary > 1 || aRelativeSize.Secondary > 1 )
    {
        OSL_FAIL("DiagramWrapper::setSize called with sizes bigger than page -> automatic values are taken instead" );
        uno::Any aEmpty;
        xProp->setPropertyValue( u"RelativeSize"_ustr, aEmpty );
        return;
    }

    xProp->setPropertyValue( u"RelativeSize"_ustr, uno::Any(aRelativeSize) );
    xProp->setPropertyValue( u"PosSizeExcludeAxes"_ustr, uno::Any(false) );
}

// ____ XShapeDescriptor (base of XShape) ____
OUString SAL_CALL DiagramWrapper::getShapeType()
{
    return u"com.sun.star.chart.Diagram"_ustr;
}

// ____ XDiagramPositioning ____

void SAL_CALL DiagramWrapper::setAutomaticDiagramPositioning()
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getDocumentModel() );
    uno::Reference< beans::XPropertySet > xDiaProps( getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
    {
        xDiaProps->setPropertyValue( u"RelativeSize"_ustr, Any() );
        xDiaProps->setPropertyValue( u"RelativePosition"_ustr, Any() );
    }
}
sal_Bool SAL_CALL DiagramWrapper::isAutomaticDiagramPositioning(  )
{
    uno::Reference< beans::XPropertySet > xDiaProps( getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
    {
        Any aRelativeSize( xDiaProps->getPropertyValue( u"RelativeSize"_ustr ) );
        Any aRelativePosition( xDiaProps->getPropertyValue( u"RelativePosition"_ustr ) );
        if( aRelativeSize.hasValue() && aRelativePosition.hasValue() )
            return false;
    }
    return true;
}
void SAL_CALL DiagramWrapper::setDiagramPositionExcludingAxes( const awt::Rectangle& rPositionRect )
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getDocumentModel() );
    DiagramHelper::setDiagramPositioning( m_spChart2ModelContact->getDocumentModel(), rPositionRect );
    uno::Reference< beans::XPropertySet > xDiaProps( getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
        xDiaProps->setPropertyValue(u"PosSizeExcludeAxes"_ustr, uno::Any(true) );
}
sal_Bool SAL_CALL DiagramWrapper::isExcludingDiagramPositioning()
{
    uno::Reference< beans::XPropertySet > xDiaProps( getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
    {
        Any aRelativeSize( xDiaProps->getPropertyValue( u"RelativeSize"_ustr ) );
        Any aRelativePosition( xDiaProps->getPropertyValue( u"RelativePosition"_ustr ) );
        if( aRelativeSize.hasValue() && aRelativePosition.hasValue() )
        {
            bool bPosSizeExcludeAxes = false;
            xDiaProps->getPropertyValue( u"PosSizeExcludeAxes"_ustr ) >>= bPosSizeExcludeAxes;
            return bPosSizeExcludeAxes;
        }
    }
    return false;
}
awt::Rectangle SAL_CALL DiagramWrapper::calculateDiagramPositionExcludingAxes(  )
{
    return m_spChart2ModelContact->GetDiagramRectangleExcludingAxes();
}
void SAL_CALL DiagramWrapper::setDiagramPositionIncludingAxes( const awt::Rectangle& rPositionRect )
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getDocumentModel() );
    DiagramHelper::setDiagramPositioning( m_spChart2ModelContact->getDocumentModel(), rPositionRect );
    uno::Reference< beans::XPropertySet > xDiaProps( getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
        xDiaProps->setPropertyValue(u"PosSizeExcludeAxes"_ustr, uno::Any(false) );
}
awt::Rectangle SAL_CALL DiagramWrapper::calculateDiagramPositionIncludingAxes(  )
{
    return m_spChart2ModelContact->GetDiagramRectangleIncludingAxes();
}
void SAL_CALL DiagramWrapper::setDiagramPositionIncludingAxesAndAxisTitles( const awt::Rectangle& rPositionRect )
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getDocumentModel() );
    awt::Rectangle aRect( m_spChart2ModelContact->SubstractAxisTitleSizes(rPositionRect) );
    DiagramWrapper::setDiagramPositionIncludingAxes( aRect );
}
css::awt::Rectangle SAL_CALL DiagramWrapper::calculateDiagramPositionIncludingAxesAndAxisTitles(  )
{
    return m_spChart2ModelContact->GetDiagramRectangleIncludingTitle();
}

// ____ XAxisSupplier ____
Reference< XAxis > SAL_CALL DiagramWrapper::getAxis( sal_Int32 nDimensionIndex )
{
    Reference< XAxis > xAxis;
    if(!nDimensionIndex)
    {
        if( !m_xXAxis.is() )
            m_xXAxis = new AxisWrapper( AxisWrapper::X_AXIS, m_spChart2ModelContact );
        xAxis = m_xXAxis;
    }
    else if(nDimensionIndex==1)
    {
        if( !m_xYAxis.is() )
            m_xYAxis = new AxisWrapper( AxisWrapper::Y_AXIS, m_spChart2ModelContact );
        xAxis = m_xYAxis;
    }
    else if(nDimensionIndex==2)
    {
        if( !m_xZAxis.is() )
            m_xZAxis = new AxisWrapper( AxisWrapper::Z_AXIS, m_spChart2ModelContact );
        xAxis = m_xZAxis;
    }
    return xAxis;
}

Reference< XAxis > SAL_CALL DiagramWrapper::getSecondaryAxis( sal_Int32 nDimensionIndex )
{
    Reference< XAxis > xAxis;
    if(!nDimensionIndex)
    {
        if( !m_xSecondXAxis.is() )
            m_xSecondXAxis = new AxisWrapper( AxisWrapper::SECOND_X_AXIS, m_spChart2ModelContact );
        xAxis = m_xSecondXAxis;
    }
    else if(nDimensionIndex==1)
    {
        if( !m_xSecondYAxis.is() )
            m_xSecondYAxis = new AxisWrapper( AxisWrapper::SECOND_Y_AXIS, m_spChart2ModelContact );
        xAxis = m_xSecondYAxis;
    }
    return xAxis;
}

// ____ XAxisZSupplier ____
Reference< drawing::XShape > SAL_CALL DiagramWrapper::getZAxisTitle()
{
    Reference< drawing::XShape > xRet;
    Reference< XAxis > xAxis( getAxis(2) );
    if( xAxis.is() )
        xRet.set( xAxis->getAxisTitle(), uno::UNO_QUERY );
    return xRet;
}

Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getZMainGrid()
{
    Reference< beans::XPropertySet > xRet;
    Reference< XAxis > xAxis( getAxis(2) );
    if( xAxis.is() )
        xRet = xAxis->getMajorGrid();
    return xRet;
}

Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getZHelpGrid()
{
    Reference< beans::XPropertySet > xRet;
    Reference< XAxis > xAxis( getAxis(2) );
    if( xAxis.is() )
        xRet = xAxis->getMinorGrid();
    return xRet;
}

Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getZAxis()
{
    if( ! m_xZAxis.is())
        m_xZAxis = new AxisWrapper( AxisWrapper::Z_AXIS, m_spChart2ModelContact );
    return Reference< beans::XPropertySet >( m_xZAxis, uno::UNO_QUERY );
}

// ____ XTwoAxisXSupplier ____
Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getSecondaryXAxis()
{
    if( ! m_xSecondXAxis.is())
        m_xSecondXAxis = new AxisWrapper( AxisWrapper::SECOND_X_AXIS, m_spChart2ModelContact );
    return Reference< beans::XPropertySet >( m_xSecondXAxis, uno::UNO_QUERY );
}

// ____ XAxisXSupplier (base of XTwoAxisXSupplier) ____
Reference< drawing::XShape > SAL_CALL DiagramWrapper::getXAxisTitle()
{
    Reference< drawing::XShape > xRet;
    Reference< XAxis > xAxis( getAxis(0) );
    if( xAxis.is() )
        xRet.set( xAxis->getAxisTitle(), uno::UNO_QUERY );
    return xRet;
}

Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getXAxis()
{
    if( ! m_xXAxis.is())
        m_xXAxis = new AxisWrapper( AxisWrapper::X_AXIS, m_spChart2ModelContact );
    return Reference< beans::XPropertySet >( m_xXAxis, uno::UNO_QUERY );
}

Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getXMainGrid()
{
    Reference< beans::XPropertySet > xRet;
    Reference< XAxis > xAxis( getAxis(0) );
    if( xAxis.is() )
        xRet = xAxis->getMajorGrid();
    return xRet;
}

Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getXHelpGrid()
{
    Reference< beans::XPropertySet > xRet;
    Reference< XAxis > xAxis( getAxis(0) );
    if( xAxis.is() )
        xRet = xAxis->getMinorGrid();
    return xRet;
}

// ____ XTwoAxisYSupplier ____
Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getSecondaryYAxis()
{
    if( ! m_xSecondYAxis.is())
        m_xSecondYAxis = new AxisWrapper( AxisWrapper::SECOND_Y_AXIS, m_spChart2ModelContact );
    return Reference< beans::XPropertySet >( m_xSecondYAxis, uno::UNO_QUERY );
}

// ____ XAxisYSupplier (base of XTwoAxisYSupplier) ____
Reference< drawing::XShape > SAL_CALL DiagramWrapper::getYAxisTitle()
{
    Reference< drawing::XShape > xRet;
    Reference< XAxis > xAxis( getAxis(1) );
    if( xAxis.is() )
        xRet.set( xAxis->getAxisTitle(), uno::UNO_QUERY );
    return xRet;
}

Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getYAxis()
{
    if( ! m_xYAxis.is())
        m_xYAxis = new AxisWrapper( AxisWrapper::Y_AXIS, m_spChart2ModelContact );
    return Reference< beans::XPropertySet >( m_xYAxis, uno::UNO_QUERY );
}

Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getYMainGrid()
{
    Reference< beans::XPropertySet > xRet;
    Reference< XAxis > xAxis( getAxis(1) );
    if( xAxis.is() )
        xRet = xAxis->getMajorGrid();
    return xRet;
}

Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getYHelpGrid()
{
    Reference< beans::XPropertySet > xRet;
    Reference< XAxis > xAxis( getAxis(1) );
    if( xAxis.is() )
        xRet = xAxis->getMinorGrid();
    return xRet;
}

// ____ XSecondAxisTitleSupplier ____
Reference< drawing::XShape > SAL_CALL DiagramWrapper::getSecondXAxisTitle()
{
    Reference< drawing::XShape > xRet;
    Reference< XAxis > xAxis( getSecondaryAxis(0) );
    if( xAxis.is() )
        xRet.set( xAxis->getAxisTitle(), uno::UNO_QUERY );
    return xRet;
}

Reference< drawing::XShape > SAL_CALL DiagramWrapper::getSecondYAxisTitle()
{
    Reference< drawing::XShape > xRet;
    Reference< XAxis > xAxis( getSecondaryAxis(1) );
    if( xAxis.is() )
        xRet.set( xAxis->getAxisTitle(), uno::UNO_QUERY );
    return xRet;
}

// ____ XStatisticDisplay ____
Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getUpBar()
{
    if( !m_xUpBarWrapper.is() )
    {
        m_xUpBarWrapper = new UpDownBarWrapper( true, m_spChart2ModelContact );
    }
    return m_xUpBarWrapper;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getDownBar()
{
    if( !m_xDownBarWrapper.is() )
    {
        m_xDownBarWrapper = new UpDownBarWrapper( false, m_spChart2ModelContact );
    }
    return m_xDownBarWrapper;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getMinMaxLine()
{
    if( !m_xMinMaxLineWrapper.is() )
    {
        m_xMinMaxLineWrapper = new MinMaxLineWrapper( m_spChart2ModelContact );
    }
    return m_xMinMaxLineWrapper;
}

// ____ X3DDisplay ____
Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getWall()
{
    if( !m_xWall.is() )
    {
        m_xWall = new WallFloorWrapper( true, m_spChart2ModelContact );
    }
    return m_xWall;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getFloor()
{
    if( !m_xFloor.is() )
    {
        m_xFloor = new WallFloorWrapper( false, m_spChart2ModelContact );
    }
    return m_xFloor;
}

// ____ X3DDefaultSetter ____
void SAL_CALL DiagramWrapper::set3DSettingsToDefault()
{
    rtl::Reference< ::chart::Diagram > x3DDefaultSetter( m_spChart2ModelContact->getDiagram() );
    if( x3DDefaultSetter.is() )
        x3DDefaultSetter->set3DSettingsToDefault();
}

void SAL_CALL DiagramWrapper::setDefaultRotation()
{
    rtl::Reference< ::chart::Diagram > x3DDefaultSetter( m_spChart2ModelContact->getDiagram() );
    if( x3DDefaultSetter.is() )
        x3DDefaultSetter->setDefaultRotation();
}

void SAL_CALL DiagramWrapper::setDefaultIllumination()
{
    rtl::Reference< ::chart::Diagram > x3DDefaultSetter( m_spChart2ModelContact->getDiagram() );
    if( x3DDefaultSetter.is() )
        x3DDefaultSetter->setDefaultIllumination();
}

// ____ XComponent ____
void SAL_CALL DiagramWrapper::dispose()
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.disposeAndClear( g, lang::EventObject( static_cast< ::cppu::OWeakObject* >( this )));

    DisposeHelper::DisposeAndClear( m_xXAxis );
    DisposeHelper::DisposeAndClear( m_xYAxis );
    DisposeHelper::DisposeAndClear( m_xZAxis );
    DisposeHelper::DisposeAndClear( m_xSecondXAxis );
    DisposeHelper::DisposeAndClear( m_xSecondYAxis );
    DisposeHelper::DisposeAndClear( m_xWall );
    DisposeHelper::DisposeAndClear( m_xFloor );
    DisposeHelper::DisposeAndClear( m_xMinMaxLineWrapper );
    DisposeHelper::DisposeAndClear( m_xUpBarWrapper );
    DisposeHelper::DisposeAndClear( m_xDownBarWrapper );

    clearWrappedPropertySet();
}

void SAL_CALL DiagramWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.addInterface( g, xListener );
}

void SAL_CALL DiagramWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.removeInterface( g, aListener );
}

namespace {

//PROP_DIAGRAM_DATAROW_SOURCE
class WrappedDataRowSourceProperty : public WrappedProperty
{
public:
    explicit WrappedDataRowSourceProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

}

WrappedDataRowSourceProperty::WrappedDataRowSourceProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
            : WrappedProperty(u"DataRowSource"_ustr,OUString())
            , m_spChart2ModelContact(std::move( spChart2ModelContact ))
{
    m_aOuterValue = WrappedDataRowSourceProperty::getPropertyDefault( nullptr );
}

void WrappedDataRowSourceProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    css::chart::ChartDataRowSource eChartDataRowSource = css::chart::ChartDataRowSource_ROWS;
    if( ! (rOuterValue >>= eChartDataRowSource) )
    {
        sal_Int32 nNew = sal_Int32(css::chart::ChartDataRowSource_ROWS);
        if( !(rOuterValue >>= nNew) )
            throw lang::IllegalArgumentException( u"Property DataRowSource requires css::chart::ChartDataRowSource value"_ustr, nullptr, 0 );
        eChartDataRowSource = css::chart::ChartDataRowSource(nNew);
    }

    m_aOuterValue = rOuterValue;

    bool bNewUseColumns = eChartDataRowSource == css::chart::ChartDataRowSource_COLUMNS;

    OUString aRangeString;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;

    if( DataSourceHelper::detectRangeSegmentation(
            m_spChart2ModelContact->getDocumentModel(), aRangeString, aSequenceMapping, bUseColumns
            , bFirstCellAsLabel, bHasCategories ) )
    {
        if( bUseColumns != bNewUseColumns )
        {
            aSequenceMapping.realloc(0);
            DataSourceHelper::setRangeSegmentation(
                m_spChart2ModelContact->getDocumentModel(), aSequenceMapping, bNewUseColumns , bFirstCellAsLabel , bHasCategories);
        }
    }
}

Any WrappedDataRowSourceProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    OUString aRangeString;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;

    if( DataSourceHelper::detectRangeSegmentation(
            m_spChart2ModelContact->getDocumentModel(), aRangeString, aSequenceMapping, bUseColumns
            , bFirstCellAsLabel, bHasCategories ) )
    {
        css::chart::ChartDataRowSource eChartDataRowSource = css::chart::ChartDataRowSource_ROWS;
        if(bUseColumns)
            eChartDataRowSource = css::chart::ChartDataRowSource_COLUMNS;

        m_aOuterValue <<= eChartDataRowSource;
    }

    return m_aOuterValue;
}

Any WrappedDataRowSourceProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= css::chart::ChartDataRowSource_COLUMNS;
    return aRet;
}

namespace {

//PROP_DIAGRAM_STACKED
//PROP_DIAGRAM_DEEP
//PROP_DIAGRAM_PERCENT_STACKED
class WrappedStackingProperty : public WrappedProperty
{
public:
    WrappedStackingProperty(StackMode eStackMode, std::shared_ptr<Chart2ModelContact> spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

protected: //methods
    bool    detectInnerValue( StackMode& eInnerStackMode ) const;

private: //member
std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    const StackMode                         m_eStackMode;
    mutable Any                             m_aOuterValue;
};

}

WrappedStackingProperty::WrappedStackingProperty(StackMode eStackMode, std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
            : WrappedProperty(OUString(),OUString())
            , m_spChart2ModelContact(std::move( spChart2ModelContact ))
            , m_eStackMode( eStackMode )
{
    switch( m_eStackMode )
    {
    case StackMode::YStacked:
        m_aOuterName = "Stacked";
        break;
    case StackMode::YStackedPercent:
        m_aOuterName = "Percent";
        break;
    case StackMode::ZStacked:
        m_aOuterName = "Deep";
        break;
    default:
        OSL_FAIL( "unexpected stack mode" );
        break;
    }
}

bool WrappedStackingProperty::detectInnerValue( StackMode& eStackMode ) const
{
    bool bHasDetectableInnerValue = false;
    bool bIsAmbiguous = false;
    rtl::Reference<Diagram> xDiagram = m_spChart2ModelContact->getDiagram();
    eStackMode = xDiagram ? xDiagram->getStackMode( bHasDetectableInnerValue, bIsAmbiguous ) : StackMode::NONE;
    return bHasDetectableInnerValue;
}

void WrappedStackingProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( u"Stacking Properties require boolean values"_ustr, nullptr, 0 );

    StackMode eInnerStackMode;
    bool bHasDetectableInnerValue = detectInnerValue( eInnerStackMode );

    if( !bHasDetectableInnerValue )
    {
        m_aOuterValue = rOuterValue;
        return;
    }

    if( bNewValue && eInnerStackMode == m_eStackMode )
        return;
    if( !bNewValue && eInnerStackMode != m_eStackMode )
        return;

    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( xDiagram.is() )
    {
        StackMode eNewStackMode = bNewValue ? m_eStackMode : StackMode::NONE;
        xDiagram->setStackMode( eNewStackMode );
    }
}

Any WrappedStackingProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    StackMode eInnerStackMode;
    if( detectInnerValue( eInnerStackMode ) )
    {
        bool bValue = (eInnerStackMode == m_eStackMode);
        return Any(bValue);
    }
    return m_aOuterValue;
}

Any WrappedStackingProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

namespace {

//PROP_DIAGRAM_THREE_D
class WrappedDim3DProperty : public WrappedProperty
{
public:
    explicit WrappedDim3DProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

}

WrappedDim3DProperty::WrappedDim3DProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
            : WrappedProperty(u"Dim3D"_ustr,OUString())
            , m_spChart2ModelContact(std::move( spChart2ModelContact ))
{
    m_aOuterValue = WrappedDim3DProperty::getPropertyDefault( nullptr );
}

void WrappedDim3DProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNew3D = false;
    if( ! (rOuterValue >>= bNew3D) )
        throw lang::IllegalArgumentException( u"Property Dim3D requires boolean value"_ustr, nullptr, 0 );

    m_aOuterValue = rOuterValue;

    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( !xDiagram.is() )
        return;

    bool bOld3D = xDiagram->getDimension() == 3;
    if( bOld3D != bNew3D )
        xDiagram->setDimension( bNew3D ? 3 : 2 );
}

Any WrappedDim3DProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( xDiagram.is() )
    {
        bool b3D = xDiagram->getDimension() == 3;
        m_aOuterValue <<= b3D;
    }
    return m_aOuterValue;
}

Any WrappedDim3DProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

namespace {

//PROP_DIAGRAM_VERTICAL
class WrappedVerticalProperty : public WrappedProperty
{
public:
    explicit WrappedVerticalProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

}

WrappedVerticalProperty::WrappedVerticalProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
            : WrappedProperty(u"Vertical"_ustr,OUString())
            , m_spChart2ModelContact(std::move( spChart2ModelContact ))
{
    m_aOuterValue = WrappedVerticalProperty::getPropertyDefault( nullptr );
}

void WrappedVerticalProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNewVertical = false;
    if( ! (rOuterValue >>= bNewVertical) )
        throw lang::IllegalArgumentException( u"Property Vertical requires boolean value"_ustr, nullptr, 0 );

    m_aOuterValue = rOuterValue;

    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( !xDiagram.is() )
        return;

    bool bFound = false;
    bool bAmbiguous = false;
    bool bOldVertical = xDiagram->getVertical( bFound, bAmbiguous );
    if( bFound && ( bOldVertical != bNewVertical || bAmbiguous ) )
        xDiagram->setVertical( bNewVertical );
}

Any WrappedVerticalProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( xDiagram.is() )
    {
        bool bFound = false;
        bool bAmbiguous = false;
        bool bVertical = xDiagram->getVertical( bFound, bAmbiguous );
        if( bFound )
            m_aOuterValue <<= bVertical;
    }
    return m_aOuterValue;
}

Any WrappedVerticalProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

namespace {

//PROP_DIAGRAM_NUMBER_OF_LINES
class WrappedNumberOfLinesProperty : public WrappedProperty
{
public:
    explicit WrappedNumberOfLinesProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

protected: //methods
    bool    detectInnerValue( uno::Any& rInnerValue ) const;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

}

WrappedNumberOfLinesProperty::WrappedNumberOfLinesProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
            : WrappedProperty(u"NumberOfLines"_ustr,OUString())
            , m_spChart2ModelContact(std::move( spChart2ModelContact ))
            , m_aOuterValue( getPropertyDefault(nullptr) )
{
}

bool WrappedNumberOfLinesProperty::detectInnerValue( uno::Any& rInnerValue ) const
{
    sal_Int32 nNumberOfLines = 0;
    bool bHasDetectableInnerValue = false;
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    rtl::Reference< ChartModel > xChartDoc( m_spChart2ModelContact->getDocumentModel() );
    if( xDiagram.is() && xChartDoc.is() )
    {
        std::vector< rtl::Reference< DataSeries > > aSeriesVector =
            xDiagram->getDataSeries();
        if( !aSeriesVector.empty() )
        {
            rtl::Reference< ::chart::ChartTypeManager > xChartTypeManager = xChartDoc->getTypeManager();
            Diagram::tTemplateWithServiceName aTemplateAndService =
                    xDiagram->getTemplate( xChartTypeManager );
            if( aTemplateAndService.sServiceName == "com.sun.star.chart2.template.ColumnWithLine" )
            {
                try
                {
                    uno::Reference< beans::XPropertySet > xProp( static_cast<cppu::OWeakObject*>(aTemplateAndService.xChartTypeTemplate.get()), uno::UNO_QUERY );
                    xProp->getPropertyValue( m_aOuterName ) >>= nNumberOfLines;
                    bHasDetectableInnerValue = true;
                }
                catch( const uno::Exception & )
                {
                    DBG_UNHANDLED_EXCEPTION("chart2");
                }
            }
        }
    }
    if(bHasDetectableInnerValue)
        rInnerValue <<= nNumberOfLines;
    return bHasDetectableInnerValue;
}

void WrappedNumberOfLinesProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    sal_Int32 nNewValue;
    if( ! (rOuterValue >>= nNewValue) )
        throw lang::IllegalArgumentException( u"property NumberOfLines requires sal_Int32 value"_ustr, nullptr, 0 );

    m_aOuterValue = rOuterValue;

    rtl::Reference< ChartModel > xChartDoc( m_spChart2ModelContact->getDocumentModel() );
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( !xChartDoc || !xDiagram )
        return;
    sal_Int32 nDimension = xDiagram->getDimension();
    if( nDimension != 2 )
        return;

    rtl::Reference< ::chart::ChartTypeManager > xChartTypeManager = xChartDoc->getTypeManager();
    Diagram::tTemplateWithServiceName aTemplateAndService =
            xDiagram->getTemplate( xChartTypeManager );

    rtl::Reference< ChartTypeTemplate > xTemplate;
    if( aTemplateAndService.sServiceName == "com.sun.star.chart2.template.ColumnWithLine" )
    {
        if( nNewValue != 0 )
        {
            xTemplate = aTemplateAndService.xChartTypeTemplate;
            try
            {
                sal_Int32 nOldValue = 0;
                uno::Reference< beans::XPropertySet > xProp( static_cast<cppu::OWeakObject*>(xTemplate.get()), uno::UNO_QUERY );
                xProp->getPropertyValue( m_aOuterName ) >>= nOldValue;
                if( nOldValue == nNewValue )
                    return;
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
        else
        {
            xTemplate = xChartTypeManager->createTemplate(u"com.sun.star.chart2.template.Column"_ustr);
        }
    }
    else if( aTemplateAndService.sServiceName == "com.sun.star.chart2.template.Column" )
    {
        if( nNewValue == 0 )
            return;
        xTemplate = xChartTypeManager->createTemplate( u"com.sun.star.chart2.template.ColumnWithLine"_ustr );
    }

    if(!xTemplate.is())
        return;

    try
    {
        // locked controllers
        ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getDocumentModel() );
        uno::Reference< beans::XPropertySet > xProp( static_cast<cppu::OWeakObject*>(xTemplate.get()), uno::UNO_QUERY );
        xProp->setPropertyValue( u"NumberOfLines"_ustr, uno::Any(nNewValue) );
        xTemplate->changeDiagram( xDiagram );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

Any WrappedNumberOfLinesProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    Any aRet;
    if( !detectInnerValue( aRet ) )
        aRet = m_aOuterValue;
    return aRet;
}

Any WrappedNumberOfLinesProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= sal_Int32( 0 );
    return aRet;
}

namespace {

//PROP_DIAGRAM_ATTRIBUTED_DATA_POINTS
class WrappedAttributedDataPointsProperty : public WrappedProperty
{
public:
    explicit WrappedAttributedDataPointsProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

}

WrappedAttributedDataPointsProperty::WrappedAttributedDataPointsProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
            : WrappedProperty(u"AttributedDataPoints"_ustr,OUString())
            , m_spChart2ModelContact(std::move( spChart2ModelContact ))
{
    m_aOuterValue = WrappedAttributedDataPointsProperty::getPropertyDefault( nullptr );
}

void WrappedAttributedDataPointsProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    uno::Sequence< uno::Sequence< sal_Int32 > > aNewValue;
    if( ! (rOuterValue >>= aNewValue) )
        throw lang::IllegalArgumentException( u"Property AttributedDataPoints requires value of type uno::Sequence< uno::Sequence< sal_Int32 > >"_ustr, nullptr, 0 );

    m_aOuterValue = rOuterValue;

    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );

    if( !xDiagram )
        return;

    std::vector< rtl::Reference< DataSeries > > aSeriesVector =
        xDiagram->getDataSeries();
    sal_Int32 i = 0;
    for (auto const& series : aSeriesVector)
    {
        uno::Any aVal;
        if( i < aNewValue.getLength() )
            aVal <<= aNewValue[i];
        else
        {
            //set empty sequence
            uno::Sequence< sal_Int32 > aSeq;
            aVal <<= aSeq;
        }
        series->setFastPropertyValue( PROP_DATASERIES_ATTRIBUTED_DATA_POINTS, aVal ); // "AttributedDataPoints"
        ++i;
    }
}

Any WrappedAttributedDataPointsProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );

    if( xDiagram )
    {
        std::vector< rtl::Reference< DataSeries > > aSeriesVector =
            xDiagram->getDataSeries();

        uno::Sequence< uno::Sequence< sal_Int32 > > aResult( aSeriesVector.size() );
        auto aResultRange = asNonConstRange(aResult);
        sal_Int32 i = 0;
        for (auto const& series : aSeriesVector)
        {
            uno::Any aVal(
                series->getFastPropertyValue(PROP_DATASERIES_ATTRIBUTED_DATA_POINTS)); // "AttributedDataPoints"
            uno::Sequence< sal_Int32 > aSeq;
            if( aVal >>= aSeq )
                aResultRange[ i ] = aSeq;
            ++i;
        }
        m_aOuterValue <<= aResult;
    }
    return m_aOuterValue;
}

Any WrappedAttributedDataPointsProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    uno::Sequence< uno::Sequence< sal_Int32 > > aSeq;
    aRet <<= aSeq;
    return aRet;
}

namespace {

//PROP_DIAGRAM_SOLIDTYPE
class WrappedSolidTypeProperty : public WrappedProperty
{
public:
    explicit WrappedSolidTypeProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

}

WrappedSolidTypeProperty::WrappedSolidTypeProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
            : WrappedProperty( u"SolidType"_ustr, OUString() )
            , m_spChart2ModelContact(std::move( spChart2ModelContact ))
{
    m_aOuterValue = WrappedSolidTypeProperty::getPropertyDefault( nullptr );
}

void WrappedSolidTypeProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    sal_Int32 nNewSolidType = css::chart::ChartSolidType::RECTANGULAR_SOLID;
    if( ! (rOuterValue >>= nNewSolidType) )
        throw lang::IllegalArgumentException( u"Property SolidType requires integer value"_ustr, nullptr, 0 );

    m_aOuterValue = rOuterValue;

    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( !xDiagram.is() )
        return;

    bool bFound = false;
    bool bAmbiguous = false;
    sal_Int32 nOldSolidType = xDiagram->getGeometry3D( bFound, bAmbiguous );
    if( bFound && ( nOldSolidType != nNewSolidType || bAmbiguous ) )
        xDiagram->setGeometry3D( nNewSolidType );
}

Any WrappedSolidTypeProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( xDiagram.is() )
    {
        bool bFound = false;
        bool bAmbiguous = false;
        sal_Int32 nGeometry = xDiagram->getGeometry3D( bFound, bAmbiguous );
        if( bFound )
            m_aOuterValue <<= nGeometry;
    }
    return m_aOuterValue;
}

Any WrappedSolidTypeProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    return uno::Any( css::chart::ChartSolidType::RECTANGULAR_SOLID );
}

namespace {

class WrappedAutomaticSizeProperty : public WrappedProperty
{
public:
    WrappedAutomaticSizeProperty();

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;
};

}

WrappedAutomaticSizeProperty::WrappedAutomaticSizeProperty()
            : WrappedProperty( u"AutomaticSize"_ustr, OUString() )
{
}

void WrappedAutomaticSizeProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    if( !xInnerPropertySet.is() )
        return;

    bool bNewValue = true;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( u"Property AutomaticSize requires value of type boolean"_ustr, nullptr, 0 );

    try
    {
        if( bNewValue )
        {
            Any aRelativeSize( xInnerPropertySet->getPropertyValue( u"RelativeSize"_ustr ) );
            if( aRelativeSize.hasValue() )
                xInnerPropertySet->setPropertyValue( u"RelativeSize"_ustr, Any() );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

Any WrappedAutomaticSizeProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Any aRet( getPropertyDefault( Reference< beans::XPropertyState >( xInnerPropertySet, uno::UNO_QUERY ) ) );
    if( xInnerPropertySet.is() )
    {
        Any aRelativeSize( xInnerPropertySet->getPropertyValue( u"RelativeSize"_ustr ) );
        if( !aRelativeSize.hasValue() )
            aRet <<= true;
    }
    return aRet;
}

Any WrappedAutomaticSizeProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

namespace {

//PROP_DIAGRAM_INCLUDE_HIDDEN_CELLS
class WrappedIncludeHiddenCellsProperty : public WrappedProperty
{
public:
    explicit WrappedIncludeHiddenCellsProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;
    virtual Any getPropertyValue(const Reference<beans::XPropertySet>& xInnerPropertySet) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
};

}

WrappedIncludeHiddenCellsProperty::WrappedIncludeHiddenCellsProperty(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
            : WrappedProperty(u"IncludeHiddenCells"_ustr,u"IncludeHiddenCells"_ustr)
            , m_spChart2ModelContact(std::move( spChart2ModelContact ))
{
}

void WrappedIncludeHiddenCellsProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( u"Property IncludeHiddenCells requires boolean value"_ustr, nullptr, 0 );

    ChartModelHelper::setIncludeHiddenCells( bNewValue, *m_spChart2ModelContact->getDocumentModel() );
}

Any WrappedIncludeHiddenCellsProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bValue = ChartModelHelper::isIncludeHiddenCells( m_spChart2ModelContact->getDocumentModel() );
    return uno::Any(bValue);
}

// ____ XDiagramProvider ____
Reference< chart2::XDiagram > SAL_CALL DiagramWrapper::getDiagram()
{
    return m_spChart2ModelContact->getDiagram();
}

void SAL_CALL DiagramWrapper::setDiagram(
    const Reference< chart2::XDiagram >& /*xDiagram*/ )
{
    //@todo: remove this method from interface
    OSL_FAIL("DiagramWrapper::setDiagram is not implemented, should be removed and not be called" );
}

Reference< beans::XPropertySet > DiagramWrapper::getInnerPropertySet()
{
    return m_spChart2ModelContact->getDiagram();
}

const Sequence< beans::Property >& DiagramWrapper::getPropertySequence()
{
    return StaticDiagramWrapperPropertyArray();
}

std::vector< std::unique_ptr<WrappedProperty> > DiagramWrapper::createWrappedProperties()
{
    std::vector< std::unique_ptr<WrappedProperty> > aWrappedProperties;

    WrappedAxisAndGridExistenceProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );
    WrappedAxisTitleExistenceProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );
    WrappedAxisLabelExistenceProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );
    WrappedSceneProperty::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );
    WrappedIgnoreProperties::addIgnoreFillProperties( aWrappedProperties );
    WrappedIgnoreProperties::addIgnoreLineProperties( aWrappedProperties );
    WrappedStatisticProperties::addWrappedPropertiesForDiagram( aWrappedProperties, m_spChart2ModelContact );
    WrappedSymbolProperties::addWrappedPropertiesForDiagram( aWrappedProperties, m_spChart2ModelContact );
    WrappedDataCaptionProperties::addWrappedPropertiesForDiagram( aWrappedProperties, m_spChart2ModelContact );
    WrappedSplineProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );
    WrappedStockProperties::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );
    WrappedAutomaticPositionProperties::addWrappedProperties( aWrappedProperties );

    aWrappedProperties.emplace_back( new WrappedDataRowSourceProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedStackingProperty( StackMode::YStacked,m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedStackingProperty( StackMode::YStackedPercent, m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedStackingProperty( StackMode::ZStacked, m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedDim3DProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedVerticalProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedNumberOfLinesProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedAttributedDataPointsProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedProperty( u"StackedBarsConnected"_ustr, u"ConnectBars"_ustr ) );
    aWrappedProperties.emplace_back( new WrappedSolidTypeProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedAutomaticSizeProperty() );
    aWrappedProperties.emplace_back( new WrappedIncludeHiddenCellsProperty( m_spChart2ModelContact ) );

    return aWrappedProperties;
}

OUString SAL_CALL DiagramWrapper::getImplementationName()
{
    return u"com.sun.star.comp.chart.Diagram"_ustr;
}

sal_Bool SAL_CALL DiagramWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL DiagramWrapper::getSupportedServiceNames()
{
    return {
        u"com.sun.star.chart.Diagram"_ustr,
        u"com.sun.star.xml.UserDefinedAttributesSupplier"_ustr,
        u"com.sun.star.chart.StackableDiagram"_ustr,
        u"com.sun.star.chart.ChartAxisXSupplier"_ustr,
        u"com.sun.star.chart.ChartAxisYSupplier"_ustr,
        u"com.sun.star.chart.ChartAxisZSupplier"_ustr,
        u"com.sun.star.chart.ChartTwoAxisXSupplier"_ustr,
        u"com.sun.star.chart.ChartTwoAxisYSupplier"_ustr
    };
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
