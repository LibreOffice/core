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
#include "AxisWrapper.hxx"
#include <AxisHelper.hxx>
#include "Chart2ModelContact.hxx"
#include <PositionAndSizeHelper.hxx>
#include "WallFloorWrapper.hxx"
#include "MinMaxLineWrapper.hxx"
#include "UpDownBarWrapper.hxx"
#include <DiagramHelper.hxx>
#include <DataSourceHelper.hxx>
#include <ChartModelHelper.hxx>
#include <WrappedIgnoreProperty.hxx>
#include "WrappedAxisAndGridExistenceProperties.hxx"
#include "WrappedStatisticProperties.hxx"
#include "WrappedSymbolProperties.hxx"
#include "WrappedDataCaptionProperties.hxx"
#include "WrappedSplineProperties.hxx"
#include "WrappedStockProperties.hxx"
#include "WrappedSceneProperty.hxx"
#include <RelativePositionHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <ModifyListenerHelper.hxx>
#include <DisposeHelper.hxx>
#include "WrappedAutomaticPositionProperties.hxx"
#include <CommonConverters.hxx>
#include <unonames.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart/ChartSolidType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <UserDefinedProperties.hxx>
#include <SceneProperties.hxx>

#include <algorithm>
#include <map>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::chart::wrapper;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::com::sun::star::chart::XAxis;
using ::osl::MutexGuard;

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
    PROP_DIAGRAM_DATATABLEHBORDER,
    PROP_DIAGRAM_DATATABLEVBORDER,
    PROP_DIAGRAM_DATATABLEOUTLINE,
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
    rOutProperties.emplace_back( "DataTableHBorder",
                PROP_DIAGRAM_DATATABLEHBORDER,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "DataTableVBorder",
                PROP_DIAGRAM_DATATABLEVBORDER,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "DataTableOutline",
                PROP_DIAGRAM_DATATABLEOUTLINE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "ExternalData",
                  PROP_DIAGRAM_EXTERNALDATA,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );
}

struct StaticDiagramWrapperPropertyArray_Initializer
{
    Sequence< Property >* operator()()
    {
        static Sequence< Property > aPropSeq( lcl_GetPropertySequence() );
        return &aPropSeq;
    }

private:
    static uno::Sequence< Property > lcl_GetPropertySequence()
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
    }
};

struct StaticDiagramWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticDiagramWrapperPropertyArray_Initializer >
{
};

bool lcl_isXYChart( const Reference< chart2::XDiagram >& rDiagram )
{
    bool bRet = false;
    Reference< chart2::XChartType > xChartType( ::chart::DiagramHelper::getChartTypeByIndex( rDiagram, 0 ) );
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
                , const Reference< chart2::XDiagram >& xDiagram )
{
    sal_Int32 nNewAPIIndex = nOldAPIIndex;

    if( lcl_isXYChart( xDiagram ) )
    {
        if( nNewAPIIndex >= 1 )
            nNewAPIIndex -= 1;
    }

    std::vector< uno::Reference< chart2::XDataSeries > > aSeriesList(
        ::chart::DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
    if( nNewAPIIndex >= static_cast<sal_Int32>(aSeriesList.size()) )
        nNewAPIIndex = -1;

    return nNewAPIIndex;
}

OUString lcl_getDiagramType( const OUString & rTemplateServiceName )
{
    const OUString aPrefix("com.sun.star.chart2.template.");

    if( rTemplateServiceName.match( aPrefix ))
    {
        const OUString aName( rTemplateServiceName.copy( aPrefix.getLength()));

        // "Area" "StackedArea" "PercentStackedArea" "ThreeDArea"
        // "StackedThreeDArea" "PercentStackedThreeDArea"
        if( aName.indexOf( "Area" ) != -1 )
            return OUString("com.sun.star.chart.AreaDiagram");

        // "Pie" "PieAllExploded" "ThreeDPie" "ThreeDPieAllExploded"
        if( aName.indexOf( "Pie" ) != -1 )
            return OUString("com.sun.star.chart.PieDiagram");

        // "Column" "StackedColumn" "PercentStackedColumn" "ThreeDColumnDeep"
        // "ThreeDColumnFlat" "StackedThreeDColumnFlat"
        // "PercentStackedThreeDColumnFlat" "Bar" "StackedBar"
        // "PercentStackedBar" "ThreeDBarDeep" "ThreeDBarFlat"
        // "StackedThreeDBarFlat" "PercentStackedThreeDBarFlat" "ColumnWithLine"
        // "StackedColumnWithLine"
        if( aName.indexOf( "Column" ) != -1 || aName.indexOf( "Bar" ) != -1 )
            return OUString( "com.sun.star.chart.BarDiagram" );

        // "Donut" "DonutAllExploded" "ThreeDDonut" "ThreeDDonutAllExploded"
        if( aName.indexOf( "Donut" ) != -1 )
            return OUString( "com.sun.star.chart.DonutDiagram" );

        // "ScatterLineSymbol" "ScatterLine" "ScatterSymbol" "ThreeDScatter"
        if( aName.indexOf( "Scatter" ) != -1 )
            return OUString( "com.sun.star.chart.XYDiagram" );

        // "FilledNet" "StackedFilledNet" "PercentStackedFilledNet"
        if( aName.indexOf( "FilledNet" ) != -1 )
            return OUString( "com.sun.star.chart.FilledNetDiagram" );

        // "Net" "NetSymbol" "NetLine" "StackedNet" "StackedNetSymbol"
        // "StackedNetLine" "PercentStackedNet" "PercentStackedNetSymbol"
        // "PercentStackedNetLine"
        if( aName.indexOf( "Net" ) != -1 )
            return OUString( "com.sun.star.chart.NetDiagram" );

        // "StockLowHighClose" "StockOpenLowHighClose" "StockVolumeLowHighClose"
        // "StockVolumeOpenLowHighClose"
        if( aName.indexOf( "Stock" ) != -1 )
            return OUString( "com.sun.star.chart.StockDiagram" );

        if( aName.indexOf( "Bubble" ) != -1 )
            return OUString( "com.sun.star.chart.BubbleDiagram" );

        // Note: this must be checked after Bar, Net and Scatter

        // "Symbol" "StackedSymbol" "PercentStackedSymbol" "Line" "StackedLine"
        // "PercentStackedLine" "LineSymbol" "StackedLineSymbol"
        // "PercentStackedLineSymbol" "ThreeDLine" "StackedThreeDLine"
        // "PercentStackedThreeDLine" "ThreeDLineDeep"
        if( aName.indexOf( "Line" ) != -1 || aName.indexOf( "Symbol" ) != -1 )
            return OUString( "com.sun.star.chart.LineDiagram" );

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

namespace chart
{
namespace wrapper
{

DiagramWrapper::DiagramWrapper(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
    : m_spChart2ModelContact(spChart2ModelContact)
    , m_aEventListenerContainer(m_aMutex)
{
}

DiagramWrapper::~DiagramWrapper()
{}

// ____ XDiagram ____
OUString SAL_CALL DiagramWrapper::getDiagramType()
{
    OUString aRet;

    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xChartDoc.is() && xDiagram.is() )
    {
        Reference< beans::XPropertySet > xChartDocProp( xChartDoc, uno::UNO_QUERY );
        if( xChartDocProp.is() )
        {
            uno::Reference< util::XRefreshable > xAddIn;
            if( xChartDocProp->getPropertyValue( "AddIn" ) >>= xAddIn )
            {
                uno::Reference< lang::XServiceName > xServiceName( xAddIn, uno::UNO_QUERY );
                if( xServiceName.is())
                    return xServiceName->getServiceName();
            }
        }

        Reference< lang::XMultiServiceFactory > xChartTypeManager( xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
        DiagramHelper::tTemplateWithServiceName aTemplateAndService =
            DiagramHelper::getTemplateForDiagram( xDiagram, xChartTypeManager );

        aRet = lcl_getDiagramType( aTemplateAndService.second );
    }

    if( aRet.isEmpty())
    {
        // none of the standard templates matched
        // use first chart type
        Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
        if( xChartType.is() )
        {
            aRet = xChartType->getChartType();
            if( !aRet.isEmpty() )
                aRet = lcl_getOldChartTypeName( aRet );
        }
        if( aRet.isEmpty())
            aRet = "com.sun.star.chart.BarDiagram";
    }

    return aRet;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getDataRowProperties( sal_Int32 nRow )
{
    if( nRow < 0 )
        throw lang::IndexOutOfBoundsException("DataSeries index invalid",
                                              static_cast< ::cppu::OWeakObject * >( this ));

    sal_Int32 nNewAPIIndex = lcl_getNewAPIIndexForOldAPIIndex( nRow, m_spChart2ModelContact->getChart2Diagram() );
    if( nNewAPIIndex < 0 )
        throw lang::IndexOutOfBoundsException("DataSeries index invalid",
                                              static_cast< ::cppu::OWeakObject * >( this ));

    Reference< beans::XPropertySet > xRet( new DataSeriesPointWrapper(
          DataSeriesPointWrapper::DATA_SERIES, nNewAPIIndex, 0, m_spChart2ModelContact ) );
    return xRet;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow )
{
    if( nCol < 0 || nRow < 0 )
        throw lang::IndexOutOfBoundsException("DataSeries index invalid",
                                              static_cast< ::cppu::OWeakObject * >( this ));

    sal_Int32 nNewAPIIndex = lcl_getNewAPIIndexForOldAPIIndex( nRow, m_spChart2ModelContact->getChart2Diagram() );
    if( nNewAPIIndex < 0 )
        throw lang::IndexOutOfBoundsException("DataSeries index invalid",
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
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    Reference< beans::XPropertySet > xProp( getInnerPropertySet() );
    if( xProp.is() )
    {
        awt::Size aPageSize( m_spChart2ModelContact->GetPageSize() );

        chart2::RelativePosition aRelativePosition;
        aRelativePosition.Anchor = drawing::Alignment_TOP_LEFT;
        aRelativePosition.Primary = double(aPosition.X)/double(aPageSize.Width);
        aRelativePosition.Secondary = double(aPosition.Y)/double(aPageSize.Height);
        if( aRelativePosition.Primary < 0 || aRelativePosition.Secondary < 0 || aRelativePosition.Primary > 1 || aRelativePosition.Secondary > 1 )
        {
            OSL_FAIL("DiagramWrapper::setPosition called with a position out of range -> automatic values are taken instead" );
            uno::Any aEmpty;
            xProp->setPropertyValue( "RelativePosition", aEmpty );
            return;
        }
        xProp->setPropertyValue( "RelativePosition", uno::Any(aRelativePosition) );
        xProp->setPropertyValue( "PosSizeExcludeAxes", uno::Any(false) );
    }
}

awt::Size SAL_CALL DiagramWrapper::getSize()
{
    awt::Size aSize = ToSize( m_spChart2ModelContact->GetDiagramRectangleIncludingAxes() );
    return aSize;
}

void SAL_CALL DiagramWrapper::setSize( const awt::Size& aSize )
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    Reference< beans::XPropertySet > xProp( getInnerPropertySet() );
    if( xProp.is() )
    {
        awt::Size aPageSize( m_spChart2ModelContact->GetPageSize() );

        chart2::RelativeSize aRelativeSize;
        aRelativeSize.Primary = double(aSize.Width)/double(aPageSize.Width);
        aRelativeSize.Secondary = double(aSize.Height)/double(aPageSize.Height);

        if( aRelativeSize.Primary > 1 || aRelativeSize.Secondary > 1 )
        {
            OSL_FAIL("DiagramWrapper::setSize called with sizes bigger than page -> automatic values are taken instead" );
            uno::Any aEmpty;
            xProp->setPropertyValue( "RelativeSize", aEmpty );
            return;
        }

        xProp->setPropertyValue( "RelativeSize", uno::Any(aRelativeSize) );
        xProp->setPropertyValue( "PosSizeExcludeAxes", uno::Any(false) );
    }
}

// ____ XShapeDescriptor (base of XShape) ____
OUString SAL_CALL DiagramWrapper::getShapeType()
{
    return OUString( "com.sun.star.chart.Diagram" );
}

// ____ XDiagramPositioning ____

void SAL_CALL DiagramWrapper::setAutomaticDiagramPositioning()
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    uno::Reference< beans::XPropertySet > xDiaProps( getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
    {
        xDiaProps->setPropertyValue( "RelativeSize", Any() );
        xDiaProps->setPropertyValue( "RelativePosition", Any() );
    }
}
sal_Bool SAL_CALL DiagramWrapper::isAutomaticDiagramPositioning(  )
{
    uno::Reference< beans::XPropertySet > xDiaProps( getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
    {
        Any aRelativeSize( xDiaProps->getPropertyValue( "RelativeSize" ) );
        Any aRelativePosition( xDiaProps->getPropertyValue( "RelativePosition" ) );
        if( aRelativeSize.hasValue() && aRelativePosition.hasValue() )
            return false;
    }
    return true;
}
void SAL_CALL DiagramWrapper::setDiagramPositionExcludingAxes( const awt::Rectangle& rPositionRect )
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    DiagramHelper::setDiagramPositioning( m_spChart2ModelContact->getChartModel(), rPositionRect );
    uno::Reference< beans::XPropertySet > xDiaProps( getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
        xDiaProps->setPropertyValue("PosSizeExcludeAxes", uno::Any(true) );
}
sal_Bool SAL_CALL DiagramWrapper::isExcludingDiagramPositioning()
{
    uno::Reference< beans::XPropertySet > xDiaProps( getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
    {
        Any aRelativeSize( xDiaProps->getPropertyValue( "RelativeSize" ) );
        Any aRelativePosition( xDiaProps->getPropertyValue( "RelativePosition" ) );
        if( aRelativeSize.hasValue() && aRelativePosition.hasValue() )
        {
            bool bPosSizeExcludeAxes = false;
            xDiaProps->getPropertyValue( "PosSizeExcludeAxes" ) >>= bPosSizeExcludeAxes;
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
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    DiagramHelper::setDiagramPositioning( m_spChart2ModelContact->getChartModel(), rPositionRect );
    uno::Reference< beans::XPropertySet > xDiaProps( getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
        xDiaProps->setPropertyValue("PosSizeExcludeAxes", uno::Any(false) );
}
awt::Rectangle SAL_CALL DiagramWrapper::calculateDiagramPositionIncludingAxes(  )
{
    return m_spChart2ModelContact->GetDiagramRectangleIncludingAxes();
}
void SAL_CALL DiagramWrapper::setDiagramPositionIncludingAxesAndAxisTitles( const awt::Rectangle& rPositionRect )
{
    ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
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
    Reference< X3DDefaultSetter > x3DDefaultSetter( m_spChart2ModelContact->getChart2Diagram(), uno::UNO_QUERY );
    if( x3DDefaultSetter.is() )
        x3DDefaultSetter->set3DSettingsToDefault();
}

void SAL_CALL DiagramWrapper::setDefaultRotation()
{
    Reference< X3DDefaultSetter > x3DDefaultSetter( m_spChart2ModelContact->getChart2Diagram(), uno::UNO_QUERY );
    if( x3DDefaultSetter.is() )
        x3DDefaultSetter->setDefaultRotation();
}

void SAL_CALL DiagramWrapper::setDefaultIllumination()
{
    Reference< X3DDefaultSetter > x3DDefaultSetter( m_spChart2ModelContact->getChart2Diagram(), uno::UNO_QUERY );
    if( x3DDefaultSetter.is() )
        x3DDefaultSetter->setDefaultIllumination();
}

// ____ XComponent ____
void SAL_CALL DiagramWrapper::dispose()
{
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( static_cast< ::cppu::OWeakObject* >( this )));

    MutexGuard aGuard( GetMutex());

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
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL DiagramWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    m_aEventListenerContainer.removeInterface( aListener );
}

//PROP_DIAGRAM_DATAROW_SOURCE
class WrappedDataRowSourceProperty : public WrappedProperty
{
public:
    explicit WrappedDataRowSourceProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedDataRowSourceProperty::WrappedDataRowSourceProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty("DataRowSource",OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
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
            throw lang::IllegalArgumentException( "Property DataRowSource requires css::chart::ChartDataRowSource value", nullptr, 0 );
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
            m_spChart2ModelContact->getChartModel(), aRangeString, aSequenceMapping, bUseColumns
            , bFirstCellAsLabel, bHasCategories ) )
    {
        if( bUseColumns != bNewUseColumns )
        {
            aSequenceMapping.realloc(0);
            DataSourceHelper::setRangeSegmentation(
                m_spChart2ModelContact->getChartModel(), aSequenceMapping, bNewUseColumns , bFirstCellAsLabel , bHasCategories);
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
            m_spChart2ModelContact->getChartModel(), aRangeString, aSequenceMapping, bUseColumns
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

//PROP_DIAGRAM_STACKED
//PROP_DIAGRAM_DEEP
//PROP_DIAGRAM_PERCENT_STACKED
class WrappedStackingProperty : public WrappedProperty
{
public:
    WrappedStackingProperty(StackMode eStackMode, const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

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

WrappedStackingProperty::WrappedStackingProperty(StackMode eStackMode, const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty(OUString(),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_eStackMode( eStackMode )
            , m_aOuterValue()
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
    eStackMode = DiagramHelper::getStackMode( m_spChart2ModelContact->getChart2Diagram()
        , bHasDetectableInnerValue, bIsAmbiguous );
    return bHasDetectableInnerValue;
}

void WrappedStackingProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( "Stacking Properties require boolean values", nullptr, 0 );

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

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xDiagram.is() )
    {
        StackMode eNewStackMode = bNewValue ? m_eStackMode : StackMode::NONE;
        DiagramHelper::setStackMode( xDiagram, eNewStackMode );
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

//PROP_DIAGRAM_THREE_D
class WrappedDim3DProperty : public WrappedProperty
{
public:
    explicit WrappedDim3DProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedDim3DProperty::WrappedDim3DProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty("Dim3D",OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
{
    m_aOuterValue = WrappedDim3DProperty::getPropertyDefault( nullptr );
}

void WrappedDim3DProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNew3D = false;
    if( ! (rOuterValue >>= bNew3D) )
        throw lang::IllegalArgumentException( "Property Dim3D requires boolean value", nullptr, 0 );

    m_aOuterValue = rOuterValue;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( !xDiagram.is() )
        return;

    bool bOld3D = DiagramHelper::getDimension( xDiagram ) == 3;
    if( bOld3D != bNew3D )
        DiagramHelper::setDimension( xDiagram, bNew3D ? 3 : 2 );
}

Any WrappedDim3DProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xDiagram.is() )
    {
        bool b3D = DiagramHelper::getDimension( xDiagram ) == 3;
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

//PROP_DIAGRAM_VERTICAL
class WrappedVerticalProperty : public WrappedProperty
{
public:
    explicit WrappedVerticalProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedVerticalProperty::WrappedVerticalProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty("Vertical",OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
{
    m_aOuterValue = WrappedVerticalProperty::getPropertyDefault( nullptr );
}

void WrappedVerticalProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNewVertical = false;
    if( ! (rOuterValue >>= bNewVertical) )
        throw lang::IllegalArgumentException( "Property Vertical requires boolean value", nullptr, 0 );

    m_aOuterValue = rOuterValue;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( !xDiagram.is() )
        return;

    bool bFound = false;
    bool bAmbiguous = false;
    bool bOldVertical = DiagramHelper::getVertical( xDiagram, bFound, bAmbiguous );
    if( bFound && ( bOldVertical != bNewVertical || bAmbiguous ) )
        DiagramHelper::setVertical( xDiagram, bNewVertical );
}

Any WrappedVerticalProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xDiagram.is() )
    {
        bool bFound = false;
        bool bAmbiguous = false;
        bool bVertical = DiagramHelper::getVertical( xDiagram, bFound, bAmbiguous );
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

//PROP_DIAGRAM_NUMBER_OF_LINES
class WrappedNumberOfLinesProperty : public WrappedProperty
{
public:
    explicit WrappedNumberOfLinesProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

protected: //methods
    bool    detectInnerValue( uno::Any& rInnerValue ) const;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedNumberOfLinesProperty::WrappedNumberOfLinesProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty("NumberOfLines",OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue( getPropertyDefault(nullptr) )
{
}

bool WrappedNumberOfLinesProperty::detectInnerValue( uno::Any& rInnerValue ) const
{
    sal_Int32 nNumberOfLines = 0;
    bool bHasDetectableInnerValue = false;
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    uno::Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    if( xDiagram.is() && xChartDoc.is() )
    {
        std::vector< uno::Reference< chart2::XDataSeries > > aSeriesVector(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
        if( aSeriesVector.size() > 0 )
        {
            Reference< lang::XMultiServiceFactory > xFact( xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
            DiagramHelper::tTemplateWithServiceName aTemplateAndService =
                    DiagramHelper::getTemplateForDiagram( xDiagram, xFact );
            if( aTemplateAndService.second == "com.sun.star.chart2.template.ColumnWithLine" )
            {
                try
                {
                    uno::Reference< beans::XPropertySet > xProp( aTemplateAndService.first, uno::UNO_QUERY );
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
        throw lang::IllegalArgumentException( "property NumberOfLines requires sal_Int32 value", nullptr, 0 );

    m_aOuterValue = rOuterValue;

    uno::Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    sal_Int32 nDimension = ::chart::DiagramHelper::getDimension( xDiagram );
    if( xChartDoc.is() && xDiagram.is() && nDimension == 2 )
    {
        Reference< lang::XMultiServiceFactory > xFact( xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
        DiagramHelper::tTemplateWithServiceName aTemplateAndService =
                DiagramHelper::getTemplateForDiagram( xDiagram, xFact );

        uno::Reference< chart2::XChartTypeTemplate > xTemplate(nullptr);
        if( aTemplateAndService.second == "com.sun.star.chart2.template.ColumnWithLine" )
        {
            if( nNewValue != 0 )
            {
                xTemplate.set( aTemplateAndService.first );
                try
                {
                    sal_Int32 nOldValue = 0;
                    uno::Reference< beans::XPropertySet > xProp( xTemplate, uno::UNO_QUERY );
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
                xTemplate.set( xFact->createInstance("com.sun.star.chart2.template.Column"), uno::UNO_QUERY );
            }
        }
        else if( aTemplateAndService.second == "com.sun.star.chart2.template.Column" )
        {
            if( nNewValue == 0 )
                return;
            xTemplate.set( xFact->createInstance( "com.sun.star.chart2.template.ColumnWithLine" ), uno::UNO_QUERY );
        }

        if(xTemplate.is())
        {
            try
            {
                // locked controllers
                ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
                uno::Reference< beans::XPropertySet > xProp( xTemplate, uno::UNO_QUERY );
                xProp->setPropertyValue( "NumberOfLines", uno::Any(nNewValue) );
                xTemplate->changeDiagram( xDiagram );
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
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

//PROP_DIAGRAM_ATTRIBUTED_DATA_POINTS
class WrappedAttributedDataPointsProperty : public WrappedProperty
{
public:
    explicit WrappedAttributedDataPointsProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedAttributedDataPointsProperty::WrappedAttributedDataPointsProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty("AttributedDataPoints",OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
{
    m_aOuterValue = WrappedAttributedDataPointsProperty::getPropertyDefault( nullptr );
}

void WrappedAttributedDataPointsProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    uno::Sequence< uno::Sequence< sal_Int32 > > aNewValue;
    if( ! (rOuterValue >>= aNewValue) )
        throw lang::IllegalArgumentException( "Property AttributedDataPoints requires value of type uno::Sequence< uno::Sequence< sal_Int32 > >", nullptr, 0 );

    m_aOuterValue = rOuterValue;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY );

    if( xDiagram.is() && xDiaProp.is())
    {
        std::vector< Reference< chart2::XDataSeries > > aSeriesVector(
            ::chart::DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
        sal_Int32 i = 0;
        for (auto const& series : aSeriesVector)
        {
            Reference< beans::XPropertySet > xProp(series, uno::UNO_QUERY);
            if( xProp.is())
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
                xProp->setPropertyValue( "AttributedDataPoints", aVal );
            }
            ++i;
        }
    }
}

Any WrappedAttributedDataPointsProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    Reference< chart2::XDiagram >    xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY );

    if( xDiagram.is() && xDiaProp.is())
    {
        std::vector< Reference< chart2::XDataSeries > > aSeriesVector(
            ::chart::DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );

        uno::Sequence< uno::Sequence< sal_Int32 > > aResult( aSeriesVector.size() );

        sal_Int32 i = 0;
        for (auto const& series : aSeriesVector)
        {
            Reference< beans::XPropertySet > xProp(series, uno::UNO_QUERY);
            if( xProp.is())
            {
                uno::Any aVal(
                    xProp->getPropertyValue("AttributedDataPoints"));
                uno::Sequence< sal_Int32 > aSeq;
                if( aVal >>= aSeq )
                    aResult[ i ] = aSeq;
            }
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

//PROP_DIAGRAM_SOLIDTYPE
class WrappedSolidTypeProperty : public WrappedProperty
{
public:
    explicit WrappedSolidTypeProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedSolidTypeProperty::WrappedSolidTypeProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty( "SolidType", OUString() )
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
{
    m_aOuterValue = WrappedSolidTypeProperty::getPropertyDefault( nullptr );
}

void WrappedSolidTypeProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    sal_Int32 nNewSolidType = css::chart::ChartSolidType::RECTANGULAR_SOLID;
    if( ! (rOuterValue >>= nNewSolidType) )
        throw lang::IllegalArgumentException( "Property SolidType requires integer value", nullptr, 0 );

    m_aOuterValue = rOuterValue;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( !xDiagram.is() )
        return;

    bool bFound = false;
    bool bAmbiguous = false;
    sal_Int32 nOldSolidType = DiagramHelper::getGeometry3D( xDiagram, bFound, bAmbiguous );
    if( bFound && ( nOldSolidType != nNewSolidType || bAmbiguous ) )
        DiagramHelper::setGeometry3D( xDiagram, nNewSolidType );
}

Any WrappedSolidTypeProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xDiagram.is() )
    {
        bool bFound = false;
        bool bAmbiguous = false;
        sal_Int32 nGeometry = DiagramHelper::getGeometry3D( xDiagram, bFound, bAmbiguous );
        if( bFound )
            m_aOuterValue <<= nGeometry;
    }
    return m_aOuterValue;
}

Any WrappedSolidTypeProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    return uno::Any( css::chart::ChartSolidType::RECTANGULAR_SOLID );
}

class WrappedAutomaticSizeProperty : public WrappedProperty
{
public:
    WrappedAutomaticSizeProperty();

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;
};

WrappedAutomaticSizeProperty::WrappedAutomaticSizeProperty()
            : WrappedProperty( "AutomaticSize", OUString() )
{
}

void WrappedAutomaticSizeProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    if( xInnerPropertySet.is() )
    {
        bool bNewValue = true;
        if( ! (rOuterValue >>= bNewValue) )
            throw lang::IllegalArgumentException( "Property AutomaticSize requires value of type boolean", nullptr, 0 );

        try
        {
            if( bNewValue )
            {
                Any aRelativeSize( xInnerPropertySet->getPropertyValue( "RelativeSize" ) );
                if( aRelativeSize.hasValue() )
                    xInnerPropertySet->setPropertyValue( "RelativeSize", Any() );
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
}

Any WrappedAutomaticSizeProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    Any aRet( getPropertyDefault( Reference< beans::XPropertyState >( xInnerPropertySet, uno::UNO_QUERY ) ) );
    if( xInnerPropertySet.is() )
    {
        Any aRelativeSize( xInnerPropertySet->getPropertyValue( "RelativeSize" ) );
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

//PROP_DIAGRAM_INCLUDE_HIDDEN_CELLS
class WrappedIncludeHiddenCellsProperty : public WrappedProperty
{
public:
    explicit WrappedIncludeHiddenCellsProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;
    virtual Any getPropertyValue(const Reference<beans::XPropertySet>& xInnerPropertySet) const override;

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
};

WrappedIncludeHiddenCellsProperty::WrappedIncludeHiddenCellsProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
            : WrappedProperty("IncludeHiddenCells","IncludeHiddenCells")
            , m_spChart2ModelContact( spChart2ModelContact )
{
}

void WrappedIncludeHiddenCellsProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( "Property IncludeHiddenCells requires boolean value", nullptr, 0 );

    ChartModelHelper::setIncludeHiddenCells( bNewValue, *m_spChart2ModelContact->getModel() );
}

Any WrappedIncludeHiddenCellsProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bValue = ChartModelHelper::isIncludeHiddenCells( m_spChart2ModelContact->getChartModel() );
    return uno::Any(bValue);
}

// ____ XDiagramProvider ____
Reference< chart2::XDiagram > SAL_CALL DiagramWrapper::getDiagram()
{
    return m_spChart2ModelContact->getChart2Diagram();
}

void SAL_CALL DiagramWrapper::setDiagram(
    const Reference< chart2::XDiagram >& /*xDiagram*/ )
{
    //@todo: remove this method from interface
    OSL_FAIL("DiagramWrapper::setDiagram is not implemented, should be removed and not be called" );
}

Reference< beans::XPropertySet > DiagramWrapper::getInnerPropertySet()
{
    return Reference< beans::XPropertySet >( m_spChart2ModelContact->getChart2Diagram(), uno::UNO_QUERY );
}

const Sequence< beans::Property >& DiagramWrapper::getPropertySequence()
{
    return *StaticDiagramWrapperPropertyArray::get();
}

const std::vector< std::unique_ptr<WrappedProperty> > DiagramWrapper::createWrappedProperties()
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
    aWrappedProperties.emplace_back( new WrappedProperty( "StackedBarsConnected", "ConnectBars" ) );
    aWrappedProperties.emplace_back( new WrappedSolidTypeProperty( m_spChart2ModelContact ) );
    aWrappedProperties.emplace_back( new WrappedAutomaticSizeProperty() );
    aWrappedProperties.emplace_back( new WrappedIncludeHiddenCellsProperty( m_spChart2ModelContact ) );

    return aWrappedProperties;
}

OUString SAL_CALL DiagramWrapper::getImplementationName()
{
    return OUString("com.sun.star.comp.chart.Diagram");
}

sal_Bool SAL_CALL DiagramWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL DiagramWrapper::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart.Diagram",
        "com.sun.star.xml.UserDefinedAttributesSupplier",
        "com.sun.star.chart.StackableDiagram",
        "com.sun.star.chart.ChartAxisXSupplier",
        "com.sun.star.chart.ChartAxisYSupplier",
        "com.sun.star.chart.ChartAxisZSupplier",
        "com.sun.star.chart.ChartTwoAxisXSupplier",
        "com.sun.star.chart.ChartTwoAxisYSupplier"
    };
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
