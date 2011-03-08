/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "DiagramWrapper.hxx"
#include "macros.hxx"
#include "servicenames_charttypes.hxx"
#include "TitleWrapper.hxx"
#include "DataSeriesPointWrapper.hxx"
#include "AxisWrapper.hxx"
#include "AxisHelper.hxx"
#include "Chart2ModelContact.hxx"
#include "PositionAndSizeHelper.hxx"
#include "TitleHelper.hxx"
#include "GridWrapper.hxx"
#include "WallFloorWrapper.hxx"
#include "MinMaxLineWrapper.hxx"
#include "UpDownBarWrapper.hxx"
#include "DiagramHelper.hxx"
#include "DataSourceHelper.hxx"
#include "ChartModelHelper.hxx"
#include "WrappedIgnoreProperty.hxx"
#include "WrappedAxisAndGridExistenceProperties.hxx"
#include "WrappedStatisticProperties.hxx"
#include "WrappedSymbolProperties.hxx"
#include "WrappedDataCaptionProperties.hxx"
#include "WrappedSplineProperties.hxx"
#include "WrappedStockProperties.hxx"
#include "WrappedSceneProperty.hxx"
#include "RelativePositionHelper.hxx"
#include "ContainerHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "ModifyListenerHelper.hxx"
#include "DisposeHelper.hxx"
#include <comphelper/InlineContainer.hxx>
#include "WrappedAutomaticPositionProperties.hxx"
#include "CommonConverters.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart/ChartSolidType.hpp>

#include "LineProperties.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "SceneProperties.hxx"

#include <map>
#include <algorithm>
#include <rtl/ustrbuf.hxx>
// header for define DBG_ERROR
#include <tools/debug.hxx>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/util/XRefreshable.hpp>

using namespace ::com::sun::star;
using namespace ::chart::wrapper;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::rtl::OUString;

namespace
{
static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Diagram" ));

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

    PROP_DIAGRAM_AUTOMATIC_SIZE
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "AttributedDataPoints" ),
                  PROP_DIAGRAM_ATTRIBUTED_DATA_POINTS,
                  ::getCppuType( reinterpret_cast< const uno::Sequence< uno::Sequence< sal_Int32 > > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // see com.sun.star.chart.StackableDiagram
    rOutProperties.push_back(
        Property( C2U( "Percent" ),
                  PROP_DIAGRAM_PERCENT_STACKED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Stacked" ),
                  PROP_DIAGRAM_STACKED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Dim3D" ),
                  PROP_DIAGRAM_THREE_D,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // see com.sun.star.chart.Chart3DBarProperties
    rOutProperties.push_back(
        Property( C2U( "SolidType" ),
                  PROP_DIAGRAM_SOLIDTYPE,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // see com.sun.star.chart.BarDiagram
    rOutProperties.push_back(
        Property( C2U( "Deep" ),
                  PROP_DIAGRAM_DEEP,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Vertical" ),
                  PROP_DIAGRAM_VERTICAL,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "NumberOfLines" ),
                  PROP_DIAGRAM_NUMBER_OF_LINES,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "StackedBarsConnected" ),
                  PROP_DIAGRAM_STACKED_BARS_CONNECTED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "DataRowSource" ),
                  PROP_DIAGRAM_DATAROW_SOURCE,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartDataRowSource * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "GroupBarsPerAxis" ),
                  PROP_DIAGRAM_GROUP_BARS_PER_AXIS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "IncludeHiddenCells" ),
                  PROP_DIAGRAM_INCLUDE_HIDDEN_CELLS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    //new for XY charts
    rOutProperties.push_back(
        Property( C2U( "SortByXValues" ),
                  PROP_DIAGRAM_SORT_BY_X_VALUES,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    //for pie and donut charts
    rOutProperties.push_back(
        Property( C2U( "StartingAngle" ),
                  PROP_DIAGRAM_STARTING_ANGLE,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0) ),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    //new for 3D charts
    rOutProperties.push_back(
        Property( C2U("RightAngledAxes"),
                  PROP_DIAGRAM_RIGHT_ANGLED_AXES,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U("Perspective"),
                  PROP_DIAGRAM_PERSPECTIVE,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U("RotationHorizontal"),
                  PROP_DIAGRAM_ROTATION_HORIZONTAL,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U("RotationVertical"),
                  PROP_DIAGRAM_ROTATION_VERTICAL,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::MAYBEVOID ));

    // XAxisXSupplier
    rOutProperties.push_back(
        Property( C2U( "HasXAxis" ),
                  PROP_DIAGRAM_HAS_X_AXIS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasXAxisDescription" ),
                  PROP_DIAGRAM_HAS_X_AXIS_DESCR,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasXAxisTitle" ),
                  PROP_DIAGRAM_HAS_X_AXIS_TITLE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasXAxisGrid" ),
                  PROP_DIAGRAM_HAS_X_AXIS_GRID,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasXAxisHelpGrid" ),
                  PROP_DIAGRAM_HAS_X_AXIS_HELP_GRID,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // XAxisYSupplier
    rOutProperties.push_back(
        Property( C2U( "HasYAxis" ),
                  PROP_DIAGRAM_HAS_Y_AXIS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasYAxisDescription" ),
                  PROP_DIAGRAM_HAS_Y_AXIS_DESCR,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasYAxisTitle" ),
                  PROP_DIAGRAM_HAS_Y_AXIS_TITLE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasYAxisGrid" ),
                  PROP_DIAGRAM_HAS_Y_AXIS_GRID,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasYAxisHelpGrid" ),
                  PROP_DIAGRAM_HAS_Y_AXIS_HELP_GRID,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // XAxisZSupplier
    rOutProperties.push_back(
        Property( C2U( "HasZAxis" ),
                  PROP_DIAGRAM_HAS_Z_AXIS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasZAxisDescription" ),
                  PROP_DIAGRAM_HAS_Z_AXIS_DESCR,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasZAxisTitle" ),
                  PROP_DIAGRAM_HAS_Z_AXIS_TITLE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasZAxisGrid" ),
                  PROP_DIAGRAM_HAS_Z_AXIS_GRID,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasZAxisHelpGrid" ),
                  PROP_DIAGRAM_HAS_Z_AXIS_HELP_GRID,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // XTwoAxisXSupplier
    rOutProperties.push_back(
        Property( C2U( "HasSecondaryXAxis" ),
                  PROP_DIAGRAM_HAS_SECOND_X_AXIS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasSecondaryXAxisDescription" ),
                  PROP_DIAGRAM_HAS_SECOND_X_AXIS_DESCR,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // XTwoAxisYSupplier
    rOutProperties.push_back(
        Property( C2U( "HasSecondaryYAxis" ),
                  PROP_DIAGRAM_HAS_SECOND_Y_AXIS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasSecondaryYAxisDescription" ),
                  PROP_DIAGRAM_HAS_SECOND_Y_AXIS_DESCR,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // XSecondAxisTitleSupplier
    rOutProperties.push_back(
        Property( C2U( "HasSecondaryXAxisTitle" ),
                  PROP_DIAGRAM_HAS_SECOND_X_AXIS_TITLE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "HasSecondaryYAxisTitle" ),
                  PROP_DIAGRAM_HAS_SECOND_Y_AXIS_TITLE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "MissingValueTreatment" ),
                  PROP_DIAGRAM_MISSING_VALUE_TREATMENT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "AutomaticSize" ),
                  PROP_DIAGRAM_AUTOMATIC_SIZE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

const uno::Sequence< Property > & lcl_GetPropertySequence()
{
    static uno::Sequence< Property > aPropSeq;

    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );
        ::chart::SceneProperties::AddPropertiesToVector( aProperties );
        WrappedStatisticProperties::addProperties( aProperties );
        WrappedSymbolProperties::addProperties( aProperties );
        WrappedDataCaptionProperties::addProperties( aProperties );
        WrappedSplineProperties::addProperties( aProperties );
        WrappedStockProperties::addProperties( aProperties );
        WrappedAutomaticPositionProperties::addProperties( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}

bool lcl_isXYChart( const Reference< chart2::XDiagram > xDiagram )
{
    bool bRet = false;
    Reference< chart2::XChartType > xChartType( ::chart::DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
    if( xChartType.is() )
    {
        rtl::OUString aChartType( xChartType->getChartType() );
        if( aChartType.equalsIgnoreAsciiCase(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
            bRet = true;
    }
    return bRet;
}

sal_Int32 lcl_getNewAPIIndexForOldAPIIndex(
                sal_Int32 nOldAPIIndex
                , Reference< chart2::XDiagram > xDiagram )
{
    sal_Int32 nNewAPIIndex = nOldAPIIndex;

    if( lcl_isXYChart( xDiagram ) )
    {
        if( nNewAPIIndex >= 1 )
            nNewAPIIndex -= 1;
    }

    ::std::vector< uno::Reference< chart2::XDataSeries > > aSeriesList(
        ::chart::DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
    if( nNewAPIIndex >= static_cast<sal_Int32>(aSeriesList.size()) )
        nNewAPIIndex = -1;

    return nNewAPIIndex;
}

typedef ::std::map< OUString, OUString > tChartTypeMap;

OUString lcl_getDiagramType( const OUString & rTemplateServiceName )
{
    const OUString aPrefix( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart2.template."));

    if( rTemplateServiceName.match( aPrefix ))
    {
        const OUString aName( rTemplateServiceName.copy( aPrefix.getLength()));

        // "Area" "StackedArea" "PercentStackedArea" "ThreeDArea"
        // "StackedThreeDArea" "PercentStackedThreeDArea"
        if( aName.indexOf( C2U("Area") ) != -1 )
            return C2U( "com.sun.star.chart.AreaDiagram" );

        // "Pie" "PieAllExploded" "ThreeDPie" "ThreeDPieAllExploded"
        if( aName.indexOf( C2U("Pie") ) != -1 )
            return C2U( "com.sun.star.chart.PieDiagram" );

        // "Column" "StackedColumn" "PercentStackedColumn" "ThreeDColumnDeep"
        // "ThreeDColumnFlat" "StackedThreeDColumnFlat"
        // "PercentStackedThreeDColumnFlat" "Bar" "StackedBar"
        // "PercentStackedBar" "ThreeDBarDeep" "ThreeDBarFlat"
        // "StackedThreeDBarFlat" "PercentStackedThreeDBarFlat" "ColumnWithLine"
        // "StackedColumnWithLine"
        if( aName.indexOf( C2U("Column") ) != -1 || aName.indexOf( C2U("Bar") ) != -1 )
            return C2U( "com.sun.star.chart.BarDiagram" );

        // "Donut" "DonutAllExploded" "ThreeDDonut" "ThreeDDonutAllExploded"
        if( aName.indexOf( C2U("Donut") ) != -1 )
            return C2U( "com.sun.star.chart.DonutDiagram" );

        // "ScatterLineSymbol" "ScatterLine" "ScatterSymbol" "ThreeDScatter"
        if( aName.indexOf( C2U("Scatter") ) != -1 )
            return C2U( "com.sun.star.chart.XYDiagram" );

        // "FilledNet" "StackedFilledNet" "PercentStackedFilledNet"
        if( aName.indexOf( C2U("FilledNet") ) != -1 )
            return C2U( "com.sun.star.chart.FilledNetDiagram" );

        // "Net" "NetSymbol" "NetLine" "StackedNet" "StackedNetSymbol"
        // "StackedNetLine" "PercentStackedNet" "PercentStackedNetSymbol"
        // "PercentStackedNetLine"
        if( aName.indexOf( C2U("Net") ) != -1 )
            return C2U( "com.sun.star.chart.NetDiagram" );

        // "StockLowHighClose" "StockOpenLowHighClose" "StockVolumeLowHighClose"
        // "StockVolumeOpenLowHighClose"
        if( aName.indexOf( C2U("Stock") ) != -1 )
            return C2U( "com.sun.star.chart.StockDiagram" );

        if( aName.indexOf( C2U("Bubble") ) != -1 )
            return C2U( "com.sun.star.chart.BubbleDiagram" );

        // Note: this must be checked after Bar, Net and Scatter

        // "Symbol" "StackedSymbol" "PercentStackedSymbol" "Line" "StackedLine"
        // "PercentStackedLine" "LineSymbol" "StackedLineSymbol"
        // "PercentStackedLineSymbol" "ThreeDLine" "StackedThreeDLine"
        // "PercentStackedThreeDLine" "ThreeDLineDeep"
        if( aName.indexOf( C2U("Line") ) != -1 || aName.indexOf( C2U("Symbol") ) != -1 )
            return C2U( "com.sun.star.chart.LineDiagram" );

        OSL_ENSURE( false, "unknown template" );
    }

    return OUString();
}

typedef ::comphelper::MakeMap< ::rtl::OUString, ::rtl::OUString > tMakeStringStringMap;

const tMakeStringStringMap& lcl_getChartTypeNameMap()
{
    static tMakeStringStringMap g_aChartTypeNameMap =
        tMakeStringStringMap
        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.LineChartType" ) )
        , rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.LineDiagram" ) ) )

        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.AreaChartType" ) )
        , rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.AreaDiagram" ) ) )

        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.ColumnChartType" ) )
        , rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.BarDiagram" ) ) )

        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.PieChartType" ) )
        , rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.PieDiagram" ) ) )

        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.DonutChartType" ) )
        , rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.DonutDiagram" ) ) )

        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.ScatterChartType" ) )
        , rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.XYDiagram" ) ) )

        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.FilledNetChartType" ) )
        , rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.FilledNetDiagram" ) ) )

        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.NetChartType" ) )
        , rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.NetDiagram" ) ) )

        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.CandleStickChartType" ) )
        , rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.StockDiagram" ) ) )

        ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.BubbleChartType" ) )
        , rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart.BubbleDiagram" ) ) )

        ;
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

// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

DiagramWrapper::DiagramWrapper(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact ) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex )
{
}

DiagramWrapper::~DiagramWrapper()
{}

// ____ XDiagram ____
OUString SAL_CALL DiagramWrapper::getDiagramType()
    throw (uno::RuntimeException)
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
            if( xChartDocProp->getPropertyValue( C2U( "AddIn" ) ) >>= xAddIn )
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

    if( !aRet.getLength())
    {
        // none of the standard templates matched
        // use first chart type
        Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
        if( xChartType.is() )
        {
            aRet = xChartType->getChartType();
            if( aRet.getLength() )
                aRet = lcl_getOldChartTypeName( aRet );
        }
        if( !aRet.getLength())
            aRet = C2U( "com.sun.star.chart.BarDiagram" );
    }

    return aRet;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getDataRowProperties( sal_Int32 nRow )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    if( nRow < 0 )
        throw lang::IndexOutOfBoundsException(
            C2U( "DataSeries index invalid" ), static_cast< ::cppu::OWeakObject * >( this ));

    Reference< chart2::XDataSeries > xSeries;

    sal_Int32 nNewAPIIndex = lcl_getNewAPIIndexForOldAPIIndex( nRow, m_spChart2ModelContact->getChart2Diagram() );
    if( nNewAPIIndex < 0 )
        throw lang::IndexOutOfBoundsException(
            C2U( "DataSeries index invalid" ), static_cast< ::cppu::OWeakObject * >( this ));

    Reference< beans::XPropertySet > xRet( new DataSeriesPointWrapper(
          DataSeriesPointWrapper::DATA_SERIES, nNewAPIIndex, 0, m_spChart2ModelContact ) );
    return xRet;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    if( nCol < 0 || nRow < 0 )
        throw lang::IndexOutOfBoundsException(
            C2U( "DataSeries index invalid" ), static_cast< ::cppu::OWeakObject * >( this ));

    Reference< chart2::XDataSeries > xSeries;

    sal_Int32 nNewAPIIndex = lcl_getNewAPIIndexForOldAPIIndex( nRow, m_spChart2ModelContact->getChart2Diagram() );
    if( nNewAPIIndex < 0 )
        throw lang::IndexOutOfBoundsException(
            C2U( "DataSeries index invalid" ), static_cast< ::cppu::OWeakObject * >( this ));

    //todo: check borders of point index

    Reference< beans::XPropertySet > xRet( new DataSeriesPointWrapper(
          DataSeriesPointWrapper::DATA_POINT, nNewAPIIndex, nCol, m_spChart2ModelContact ) );

    return xRet;
}

// ____ XShape (base of XDiagram) ____
awt::Point SAL_CALL DiagramWrapper::getPosition()
    throw (uno::RuntimeException)
{
    awt::Point aPosition = ToPoint( m_spChart2ModelContact->GetDiagramRectangleIncludingAxes() );
    return aPosition;
}

void SAL_CALL DiagramWrapper::setPosition( const awt::Point& aPosition )
    throw (uno::RuntimeException)
{
    ControllerLockGuard aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    Reference< beans::XPropertySet > xProp( this->getInnerPropertySet() );
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
            xProp->setPropertyValue( C2U( "RelativePosition" ), aEmpty );
            return;
        }
        xProp->setPropertyValue( C2U( "RelativePosition" ), uno::makeAny(aRelativePosition) );
        xProp->setPropertyValue( C2U( "PosSizeExcludeAxes" ), uno::makeAny(false) );
    }
}

awt::Size SAL_CALL DiagramWrapper::getSize()
    throw (uno::RuntimeException)
{
    awt::Size aSize = ToSize( m_spChart2ModelContact->GetDiagramRectangleIncludingAxes() );
    return aSize;
}

void SAL_CALL DiagramWrapper::setSize( const awt::Size& aSize )
    throw (beans::PropertyVetoException,
           uno::RuntimeException)
{
    ControllerLockGuard aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    Reference< beans::XPropertySet > xProp( this->getInnerPropertySet() );
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
            xProp->setPropertyValue( C2U( "RelativeSize" ), aEmpty );
            return;
        }

        xProp->setPropertyValue( C2U( "RelativeSize" ), uno::makeAny(aRelativeSize) );
        xProp->setPropertyValue( C2U( "PosSizeExcludeAxes" ), uno::makeAny(false) );
    }
}

// ____ XShapeDescriptor (base of XShape) ____
OUString SAL_CALL DiagramWrapper::getShapeType()
    throw (uno::RuntimeException)
{
    return C2U( "com.sun.star.chart.Diagram" );
}

// ____ XDiagramPositioning ____

void SAL_CALL DiagramWrapper::setAutomaticDiagramPositioning() throw (uno::RuntimeException)
{
    ControllerLockGuard aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    uno::Reference< beans::XPropertySet > xDiaProps( this->getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
    {
        xDiaProps->setPropertyValue( C2U( "RelativeSize" ), Any() );
        xDiaProps->setPropertyValue( C2U( "RelativePosition" ), Any() );
    }
}
::sal_Bool SAL_CALL DiagramWrapper::isAutomaticDiagramPositioning(  ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xDiaProps( this->getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
    {
        Any aRelativeSize( xDiaProps->getPropertyValue( C2U( "RelativeSize" ) ) );
        Any aRelativePosition( xDiaProps->getPropertyValue( C2U( "RelativePosition" ) ) );
        if( aRelativeSize.hasValue() && aRelativePosition.hasValue() )
            return false;
    }
    return true;
}
void SAL_CALL DiagramWrapper::setDiagramPositionExcludingAxes( const awt::Rectangle& rPositionRect ) throw (uno::RuntimeException)
{
    ControllerLockGuard aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    DiagramHelper::setDiagramPositioning( m_spChart2ModelContact->getChartModel(), rPositionRect );
    uno::Reference< beans::XPropertySet > xDiaProps( this->getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
        xDiaProps->setPropertyValue(C2U("PosSizeExcludeAxes"), uno::makeAny(true) );
}
::sal_Bool SAL_CALL DiagramWrapper::isExcludingDiagramPositioning() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xDiaProps( this->getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
    {
        Any aRelativeSize( xDiaProps->getPropertyValue( C2U( "RelativeSize" ) ) );
        Any aRelativePosition( xDiaProps->getPropertyValue( C2U( "RelativePosition" ) ) );
        if( aRelativeSize.hasValue() && aRelativePosition.hasValue() )
        {
            sal_Bool bPosSizeExcludeAxes = false;
            xDiaProps->getPropertyValue( C2U( "PosSizeExcludeAxes" ) ) >>= bPosSizeExcludeAxes;
            return bPosSizeExcludeAxes;
        }
    }
    return false;
}
awt::Rectangle SAL_CALL DiagramWrapper::calculateDiagramPositionExcludingAxes(  ) throw (uno::RuntimeException)
{
    return m_spChart2ModelContact->GetDiagramRectangleExcludingAxes();
}
void SAL_CALL DiagramWrapper::setDiagramPositionIncludingAxes( const awt::Rectangle& rPositionRect ) throw (uno::RuntimeException)
{
    ControllerLockGuard aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    DiagramHelper::setDiagramPositioning( m_spChart2ModelContact->getChartModel(), rPositionRect );
    uno::Reference< beans::XPropertySet > xDiaProps( this->getDiagram(), uno::UNO_QUERY );
    if( xDiaProps.is() )
        xDiaProps->setPropertyValue(C2U("PosSizeExcludeAxes"), uno::makeAny(false) );
}
awt::Rectangle SAL_CALL DiagramWrapper::calculateDiagramPositionIncludingAxes(  ) throw (uno::RuntimeException)
{
    return m_spChart2ModelContact->GetDiagramRectangleIncludingAxes();
}
void SAL_CALL DiagramWrapper::setDiagramPositionIncludingAxesAndAxisTitles( const awt::Rectangle& rPositionRect ) throw (uno::RuntimeException)
{
    ControllerLockGuard aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
    awt::Rectangle aRect( m_spChart2ModelContact->SubstractAxisTitleSizes(rPositionRect) );
    DiagramWrapper::setDiagramPositionIncludingAxes( aRect );
}
::com::sun::star::awt::Rectangle SAL_CALL DiagramWrapper::calculateDiagramPositionIncludingAxesAndAxisTitles(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_spChart2ModelContact->GetDiagramRectangleIncludingTitle();
}

// ____ XAxisZSupplier ____
Reference<
    drawing::XShape > SAL_CALL DiagramWrapper::getZAxisTitle()
    throw (uno::RuntimeException)
{
    if( !m_xZAxisTitle.is() )
    {
        m_xZAxisTitle = new TitleWrapper( TitleHelper::Z_AXIS_TITLE, m_spChart2ModelContact );
    }
    return m_xZAxisTitle;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getZMainGrid()
    throw (uno::RuntimeException)
{
    if( ! m_xZMainGrid.is())
    {
        m_xZMainGrid = new GridWrapper( GridWrapper::Z_MAIN_GRID, m_spChart2ModelContact );
    }
    return m_xZMainGrid;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getZHelpGrid()
    throw (uno::RuntimeException)
{
    if( !m_xZHelpGrid.is() )
    {
        m_xZHelpGrid = new GridWrapper( GridWrapper::Z_SUB_GRID, m_spChart2ModelContact );
    }
    return m_xZHelpGrid;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getZAxis()
    throw (uno::RuntimeException)
{
    if( ! m_xZAxis.is())
    {
        m_xZAxis = new AxisWrapper( AxisWrapper::Z_AXIS, m_spChart2ModelContact );
    }
    return m_xZAxis;
}


// ____ XTwoAxisXSupplier ____
Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getSecondaryXAxis()
    throw (uno::RuntimeException)
{
    if( ! m_xSecondXAxis.is())
    {
        m_xSecondXAxis = new AxisWrapper( AxisWrapper::SECOND_X_AXIS, m_spChart2ModelContact );
    }
    return m_xSecondXAxis;
}


// ____ XAxisXSupplier (base of XTwoAxisXSupplier) ____
Reference<
    drawing::XShape > SAL_CALL DiagramWrapper::getXAxisTitle()
    throw (uno::RuntimeException)
{

    if( !m_xXAxisTitle.is() )
    {
        m_xXAxisTitle = new TitleWrapper( TitleHelper::X_AXIS_TITLE, m_spChart2ModelContact );
    }
    return m_xXAxisTitle;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getXAxis()
    throw (uno::RuntimeException)
{
    if( ! m_xXAxis.is())
    {
        m_xXAxis = new AxisWrapper( AxisWrapper::X_AXIS, m_spChart2ModelContact );
    }

    return m_xXAxis;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getXMainGrid()
    throw (uno::RuntimeException)
{
    if( ! m_xXMainGrid.is())
    {
        m_xXMainGrid = new GridWrapper( GridWrapper::X_MAIN_GRID, m_spChart2ModelContact );
    }

    return m_xXMainGrid;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getXHelpGrid()
    throw (uno::RuntimeException)
{
    if( ! m_xXHelpGrid.is())
    {
        m_xXHelpGrid = new GridWrapper( GridWrapper::X_SUB_GRID, m_spChart2ModelContact );
    }
    return m_xXHelpGrid;
}


// ____ XTwoAxisYSupplier ____
Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getSecondaryYAxis()
    throw (uno::RuntimeException)
{
    if( ! m_xSecondYAxis.is())
    {
        m_xSecondYAxis = new AxisWrapper( AxisWrapper::SECOND_Y_AXIS, m_spChart2ModelContact );
    }
    return m_xSecondYAxis;
}


// ____ XAxisYSupplier (base of XTwoAxisYSupplier) ____
Reference<
    drawing::XShape > SAL_CALL DiagramWrapper::getYAxisTitle()
    throw (uno::RuntimeException)
{
    if( !m_xYAxisTitle.is() )
    {
        m_xYAxisTitle = new TitleWrapper( TitleHelper::Y_AXIS_TITLE, m_spChart2ModelContact );
    }
    return m_xYAxisTitle;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getYAxis()
    throw (uno::RuntimeException)
{
    if( ! m_xYAxis.is())
    {
        m_xYAxis = new AxisWrapper( AxisWrapper::Y_AXIS, m_spChart2ModelContact );
    }
    return m_xYAxis;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getYHelpGrid()
    throw (uno::RuntimeException)
{
    if( ! m_xYHelpGrid.is())
    {
        m_xYHelpGrid = new GridWrapper( GridWrapper::Y_SUB_GRID, m_spChart2ModelContact );
    }
    return m_xYHelpGrid;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getYMainGrid()
    throw (uno::RuntimeException)
{
    if( ! m_xYMainGrid.is())
    {
        m_xYMainGrid = new GridWrapper( GridWrapper::Y_MAIN_GRID, m_spChart2ModelContact );
    }
    return m_xYMainGrid;
}

// ____ XSecondAxisTitleSupplier ____
Reference<
    drawing::XShape > SAL_CALL DiagramWrapper::getSecondXAxisTitle()
    throw (uno::RuntimeException)
{
    if( !m_xSecondXAxisTitle.is() )
    {
        m_xSecondXAxisTitle = new TitleWrapper( TitleHelper::SECONDARY_X_AXIS_TITLE, m_spChart2ModelContact );
    }
    return m_xSecondXAxisTitle;
}

Reference<
    drawing::XShape > SAL_CALL DiagramWrapper::getSecondYAxisTitle()
    throw (uno::RuntimeException)
{
    if( !m_xSecondYAxisTitle.is() )
    {
        m_xSecondYAxisTitle = new TitleWrapper( TitleHelper::SECONDARY_Y_AXIS_TITLE, m_spChart2ModelContact );
    }
    return m_xSecondYAxisTitle;
}

// ____ XStatisticDisplay ____
Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getUpBar()
    throw (uno::RuntimeException)
{
    if( !m_xUpBarWrapper.is() )
    {
        m_xUpBarWrapper = new UpDownBarWrapper( true, m_spChart2ModelContact );
    }
    return m_xUpBarWrapper;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getDownBar()
    throw (uno::RuntimeException)
{
    if( !m_xDownBarWrapper.is() )
    {
        m_xDownBarWrapper = new UpDownBarWrapper( false, m_spChart2ModelContact );
    }
    return m_xDownBarWrapper;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getMinMaxLine()
    throw (uno::RuntimeException)
{
    if( !m_xMinMaxLineWrapper.is() )
    {
        m_xMinMaxLineWrapper = new MinMaxLineWrapper( m_spChart2ModelContact );
    }
    return m_xMinMaxLineWrapper;
}

// ____ X3DDisplay ____
Reference< beans::XPropertySet > SAL_CALL DiagramWrapper::getWall()
    throw (uno::RuntimeException)
{
    if( !m_xWall.is() )
    {
        m_xWall = new WallFloorWrapper( true, m_spChart2ModelContact );
    }
    return m_xWall;
}

Reference<
    beans::XPropertySet > SAL_CALL DiagramWrapper::getFloor()
    throw (uno::RuntimeException)
{
    if( !m_xFloor.is() )
    {
        m_xFloor = new WallFloorWrapper( false, m_spChart2ModelContact );
    }
    return m_xFloor;
}

// ____ X3DDefaultSetter ____
void SAL_CALL DiagramWrapper::set3DSettingsToDefault()
    throw (uno::RuntimeException)
{
    Reference< X3DDefaultSetter > x3DDefaultSetter( m_spChart2ModelContact->getChart2Diagram(), uno::UNO_QUERY );
    if( x3DDefaultSetter.is() )
        x3DDefaultSetter->set3DSettingsToDefault();
}

void SAL_CALL DiagramWrapper::setDefaultRotation()
    throw (uno::RuntimeException)
{
    Reference< X3DDefaultSetter > x3DDefaultSetter( m_spChart2ModelContact->getChart2Diagram(), uno::UNO_QUERY );
    if( x3DDefaultSetter.is() )
        x3DDefaultSetter->setDefaultRotation();
}

void SAL_CALL DiagramWrapper::setDefaultIllumination()
    throw (uno::RuntimeException)
{
    Reference< X3DDefaultSetter > x3DDefaultSetter( m_spChart2ModelContact->getChart2Diagram(), uno::UNO_QUERY );
    if( x3DDefaultSetter.is() )
        x3DDefaultSetter->setDefaultIllumination();
}

// ____ XComponent ____
void SAL_CALL DiagramWrapper::dispose()
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( static_cast< ::cppu::OWeakObject* >( this )));

    MutexGuard aGuard( GetMutex());

    DisposeHelper::DisposeAndClear( m_xXAxisTitle );
    DisposeHelper::DisposeAndClear( m_xYAxisTitle );
    DisposeHelper::DisposeAndClear( m_xZAxisTitle );
    DisposeHelper::DisposeAndClear( m_xSecondXAxisTitle );
    DisposeHelper::DisposeAndClear( m_xSecondYAxisTitle );
    DisposeHelper::DisposeAndClear( m_xXAxis );
    DisposeHelper::DisposeAndClear( m_xYAxis );
    DisposeHelper::DisposeAndClear( m_xZAxis );
    DisposeHelper::DisposeAndClear( m_xSecondXAxis );
    DisposeHelper::DisposeAndClear( m_xSecondYAxis );
    DisposeHelper::DisposeAndClear( m_xXMainGrid );
    DisposeHelper::DisposeAndClear( m_xYMainGrid );
    DisposeHelper::DisposeAndClear( m_xZMainGrid );
    DisposeHelper::DisposeAndClear( m_xXHelpGrid );
    DisposeHelper::DisposeAndClear( m_xYHelpGrid );
    DisposeHelper::DisposeAndClear( m_xZHelpGrid );
    DisposeHelper::DisposeAndClear( m_xWall );
    DisposeHelper::DisposeAndClear( m_xFloor );
    DisposeHelper::DisposeAndClear( m_xMinMaxLineWrapper );
    DisposeHelper::DisposeAndClear( m_xUpBarWrapper );
    DisposeHelper::DisposeAndClear( m_xDownBarWrapper );

    clearWrappedPropertySet();
}

void SAL_CALL DiagramWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL DiagramWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

//-----------------------------------------------------------------------------------------------------------------

//PROP_DIAGRAM_DATAROW_SOURCE
class WrappedDataRowSourceProperty : public WrappedProperty
{
public:
    WrappedDataRowSourceProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedDataRowSourceProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedDataRowSourceProperty::WrappedDataRowSourceProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(C2U("DataRowSource"),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
{
    m_aOuterValue = WrappedDataRowSourceProperty::getPropertyDefault( 0 );
}

WrappedDataRowSourceProperty::~WrappedDataRowSourceProperty()
{
}

void WrappedDataRowSourceProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::com::sun::star::chart::ChartDataRowSource eChartDataRowSource = ::com::sun::star::chart::ChartDataRowSource_ROWS;
    if( ! (rOuterValue >>= eChartDataRowSource) )
    {
        sal_Int32 nNew = ::com::sun::star::chart::ChartDataRowSource_ROWS;
        if( !(rOuterValue >>= nNew) )
            throw lang::IllegalArgumentException( C2U("Property DataRowSource requires ::com::sun::star::chart::ChartDataRowSource value"), 0, 0 );
        else
            eChartDataRowSource = ::com::sun::star::chart::ChartDataRowSource(nNew);
    }

    m_aOuterValue = rOuterValue;

    bool bNewUseColumns = eChartDataRowSource == ::com::sun::star::chart::ChartDataRowSource_COLUMNS;

    ::rtl::OUString aRangeString;
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
                m_spChart2ModelContact->getChartModel(), aSequenceMapping, bNewUseColumns , bHasCategories, bFirstCellAsLabel );
        }
    }
}

Any WrappedDataRowSourceProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::rtl::OUString aRangeString;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;

    if( DataSourceHelper::detectRangeSegmentation(
            m_spChart2ModelContact->getChartModel(), aRangeString, aSequenceMapping, bUseColumns
            , bFirstCellAsLabel, bHasCategories ) )
    {
        ::com::sun::star::chart::ChartDataRowSource eChartDataRowSource = ::com::sun::star::chart::ChartDataRowSource_ROWS;
        if(bUseColumns)
            eChartDataRowSource = ::com::sun::star::chart::ChartDataRowSource_COLUMNS;

        m_aOuterValue <<= eChartDataRowSource;
    }

    return m_aOuterValue;
}

Any WrappedDataRowSourceProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= ::com::sun::star::chart::ChartDataRowSource_COLUMNS;
    return aRet;
}

//-----------------------------------------------------------------------------------------------------------------

//PROP_DIAGRAM_STACKED
//PROP_DIAGRAM_DEEP
//PROP_DIAGRAM_PERCENT_STACKED
class WrappedStackingProperty : public WrappedProperty
{
public:
    WrappedStackingProperty( StackMode eStackMode, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedStackingProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

protected: //methods
    bool    detectInnerValue( StackMode& eInnerStackMode ) const;

private: //member
::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    const StackMode                         m_eStackMode;
    mutable Any                             m_aOuterValue;
};

WrappedStackingProperty::WrappedStackingProperty( StackMode eStackMode, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(OUString(),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_eStackMode( eStackMode )
            , m_aOuterValue()
{
    switch( m_eStackMode )
    {
    case StackMode_Y_STACKED:
        m_aOuterName = C2U( "Stacked" );
        break;
    case StackMode_Y_STACKED_PERCENT:
        m_aOuterName = C2U( "Percent" );
        break;
    case StackMode_Z_STACKED:
        m_aOuterName = C2U( "Deep" );
        break;
    default:
        OSL_ENSURE( false, "unexpected stack mode" );
        break;
    }
}

WrappedStackingProperty::~WrappedStackingProperty()
{
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
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( C2U("Stacking Properties require boolean values"), 0, 0 );

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
        StackMode eNewStackMode = bNewValue ? m_eStackMode : StackMode_NONE;
        DiagramHelper::setStackMode( xDiagram, eNewStackMode );
    }
}

Any WrappedStackingProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    StackMode eInnerStackMode;
    if( detectInnerValue( eInnerStackMode ) )
    {
        sal_Bool bValue = (eInnerStackMode == m_eStackMode);
        Any aAny;
        aAny <<= bValue;
        return aAny;
    }
    return m_aOuterValue;
}

Any WrappedStackingProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= sal_Bool( sal_False );
    return aRet;
}

//-----------------------------------------------------------------------------------------------------------------

//PROP_DIAGRAM_THREE_D
class WrappedDim3DProperty : public WrappedProperty
{
public:
    WrappedDim3DProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedDim3DProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedDim3DProperty::WrappedDim3DProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(C2U("Dim3D"),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
{
    m_aOuterValue = WrappedDim3DProperty::getPropertyDefault( 0 );
}

WrappedDim3DProperty::~WrappedDim3DProperty()
{
}

void WrappedDim3DProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Bool bNew3D = false;
    if( ! (rOuterValue >>= bNew3D) )
        throw lang::IllegalArgumentException( C2U("Property Dim3D requires boolean value"), 0, 0 );

    m_aOuterValue = rOuterValue;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( !xDiagram.is() )
        return;

    sal_Bool bOld3D = DiagramHelper::getDimension( xDiagram ) == 3;
    if( bOld3D != bNew3D )
        DiagramHelper::setDimension( xDiagram, bNew3D ? 3 : 2 );
}

Any WrappedDim3DProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xDiagram.is() )
    {
        sal_Bool b3D = DiagramHelper::getDimension( xDiagram ) == 3;
        m_aOuterValue <<= b3D;
    }
    return m_aOuterValue;
}

Any WrappedDim3DProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= sal_Bool( sal_False );
    return aRet;
}

//-----------------------------------------------------------------------------------------------------------------

//PROP_DIAGRAM_VERTICAL
class WrappedVerticalProperty : public WrappedProperty
{
public:
    WrappedVerticalProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedVerticalProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedVerticalProperty::WrappedVerticalProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(C2U("Vertical"),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
{
    m_aOuterValue = WrappedVerticalProperty::getPropertyDefault( 0 );
}

WrappedVerticalProperty::~WrappedVerticalProperty()
{
}

void WrappedVerticalProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Bool bNewVertical = false;
    if( ! (rOuterValue >>= bNewVertical) )
        throw lang::IllegalArgumentException( C2U("Property Vertical requires boolean value"), 0, 0 );

    m_aOuterValue = rOuterValue;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( !xDiagram.is() )
        return;

    bool bFound = false;
    bool bAmbiguous = false;
    sal_Bool bOldVertical = DiagramHelper::getVertical( xDiagram, bFound, bAmbiguous );
    if( bFound && ( bOldVertical != bNewVertical || bAmbiguous ) )
        DiagramHelper::setVertical( xDiagram, bNewVertical );
}

Any WrappedVerticalProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    bool bFound = false;
    bool bAmbiguous = false;
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xDiagram.is() )
    {
        sal_Bool bVertical = DiagramHelper::getVertical( xDiagram, bFound, bAmbiguous );
        if( bFound )
            m_aOuterValue <<= bVertical;
    }
    return m_aOuterValue;
}

Any WrappedVerticalProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= sal_Bool( sal_False );
    return aRet;
}

//-----------------------------------------------------------------------------------------------------------------

//PROP_DIAGRAM_NUMBER_OF_LINES
class WrappedNumberOfLinesProperty : public WrappedProperty
{
public:
    WrappedNumberOfLinesProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedNumberOfLinesProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

protected: //methods
    bool    detectInnerValue( uno::Any& rInnerValue ) const;

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedNumberOfLinesProperty::WrappedNumberOfLinesProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(C2U("NumberOfLines"),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue( this->getPropertyDefault(0) )
{
}

WrappedNumberOfLinesProperty::~WrappedNumberOfLinesProperty()
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
        ::std::vector< uno::Reference< chart2::XDataSeries > > aSeriesVector(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
        if( aSeriesVector.size() > 0 )
        {
            Reference< lang::XMultiServiceFactory > xFact( xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
            DiagramHelper::tTemplateWithServiceName aTemplateAndService =
                    DiagramHelper::getTemplateForDiagram( xDiagram, xFact );
            if( aTemplateAndService.second.equals( C2U( "com.sun.star.chart2.template.ColumnWithLine" ) ) )
            {
                try
                {
                    uno::Reference< beans::XPropertySet > xProp( aTemplateAndService.first, uno::UNO_QUERY );
                    xProp->getPropertyValue( m_aOuterName ) >>= nNumberOfLines;
                    bHasDetectableInnerValue = true;
                }
                catch( uno::Exception & ex )
                {
                    ASSERT_EXCEPTION( ex );
                }
            }
        }
    }
    if(bHasDetectableInnerValue)
        rInnerValue = uno::makeAny(nNumberOfLines);
    return bHasDetectableInnerValue;
}

void WrappedNumberOfLinesProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Int32 nNewValue;
    if( ! (rOuterValue >>= nNewValue) )
        throw lang::IllegalArgumentException( C2U("property NumberOfLines requires sal_Int32 value"), 0, 0 );

    m_aOuterValue = rOuterValue;

    uno::Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    sal_Int32 nDimension = ::chart::DiagramHelper::getDimension( xDiagram );
    if( xChartDoc.is() && xDiagram.is() && nDimension == 2 )
    {
        Reference< lang::XMultiServiceFactory > xFact( xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
        DiagramHelper::tTemplateWithServiceName aTemplateAndService =
                DiagramHelper::getTemplateForDiagram( xDiagram, xFact );

        uno::Reference< chart2::XChartTypeTemplate > xTemplate(0);
        if( aTemplateAndService.second.equals( C2U( "com.sun.star.chart2.template.ColumnWithLine" ) ) )
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
                catch( uno::Exception & ex )
                {
                    ASSERT_EXCEPTION( ex );
                }
            }
            else
            {
                xTemplate.set( xFact->createInstance( C2U( "com.sun.star.chart2.template.Column" ) ), uno::UNO_QUERY );
            }
        }
        else if( aTemplateAndService.second.equals( C2U( "com.sun.star.chart2.template.Column" ) ) )
        {
            if( nNewValue == 0 )
                return;
            xTemplate.set( xFact->createInstance( C2U( "com.sun.star.chart2.template.ColumnWithLine" ) ), uno::UNO_QUERY );
        }

        if(xTemplate.is())
        {
            try
            {
                // locked controllers
                ControllerLockGuard aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
                uno::Reference< beans::XPropertySet > xProp( xTemplate, uno::UNO_QUERY );
                xProp->setPropertyValue( C2U( "NumberOfLines" ), uno::makeAny(nNewValue) );
                xTemplate->changeDiagram( xDiagram );
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
}

Any WrappedNumberOfLinesProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    if( !detectInnerValue( aRet ) )
        aRet = m_aOuterValue;
    return aRet;
}

Any WrappedNumberOfLinesProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= sal_Int32( 0 );
    return aRet;
}

//-----------------------------------------------------------------------------------------------------------------

//PROP_DIAGRAM_ATTRIBUTED_DATA_POINTS
class WrappedAttributedDataPointsProperty : public WrappedProperty
{
public:
    WrappedAttributedDataPointsProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedAttributedDataPointsProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedAttributedDataPointsProperty::WrappedAttributedDataPointsProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(C2U("AttributedDataPoints"),OUString())
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
{
    m_aOuterValue = WrappedAttributedDataPointsProperty::getPropertyDefault( 0 );
}

WrappedAttributedDataPointsProperty::~WrappedAttributedDataPointsProperty()
{
}

void WrappedAttributedDataPointsProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Sequence< uno::Sequence< sal_Int32 > > aNewValue;
    if( ! (rOuterValue >>= aNewValue) )
        throw lang::IllegalArgumentException( C2U("Property AttributedDataPoints requires value of type uno::Sequence< uno::Sequence< sal_Int32 > >"), 0, 0 );

    m_aOuterValue = rOuterValue;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY );

    if( xDiagram.is() && xDiaProp.is())
    {
        ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector(
            ::chart::DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );

        uno::Sequence< uno::Sequence< sal_Int32 > > aResult( aSeriesVector.size() );

        ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aIt =
                aSeriesVector.begin();
        sal_Int32 i = 0;
        for( ; aIt != aSeriesVector.end(); ++aIt, ++i )
        {
            Reference< beans::XPropertySet > xProp( *aIt, uno::UNO_QUERY );
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
                xProp->setPropertyValue( C2U( "AttributedDataPoints" ), aVal );
            }
        }
    }
}

Any WrappedAttributedDataPointsProperty::getPropertyValue( const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< chart2::XDiagram >    xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY );

    if( xDiagram.is() && xDiaProp.is())
    {
        ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector(
            ::chart::DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );

        uno::Sequence< uno::Sequence< sal_Int32 > > aResult( aSeriesVector.size() );

        ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aIt =
                aSeriesVector.begin();
        sal_Int32 i = 0;
        for( ; aIt != aSeriesVector.end(); ++aIt, ++i )
        {
            Reference< beans::XPropertySet > xProp( *aIt, uno::UNO_QUERY );
            if( xProp.is())
            {
                uno::Any aVal(
                    xProp->getPropertyValue( C2U( "AttributedDataPoints" )));
                uno::Sequence< sal_Int32 > aSeq;
                if( aVal >>= aSeq )
                    aResult[ i ] = aSeq;
            }
        }
        m_aOuterValue <<= aResult;
    }
    return m_aOuterValue;
}

Any WrappedAttributedDataPointsProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    uno::Sequence< uno::Sequence< sal_Int32 > > aSeq;
    aRet <<= aSeq;
    return aRet;
}

//-----------------------------------------------------------------------------------------------------------------

//PROP_DIAGRAM_SOLIDTYPE
class WrappedSolidTypeProperty : public WrappedProperty
{
public:
    WrappedSolidTypeProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedSolidTypeProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable Any                                 m_aOuterValue;
};

WrappedSolidTypeProperty::WrappedSolidTypeProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty( C2U( "SolidType" ), OUString() )
            , m_spChart2ModelContact( spChart2ModelContact )
            , m_aOuterValue()
{
    m_aOuterValue = WrappedSolidTypeProperty::getPropertyDefault( 0 );
}

WrappedSolidTypeProperty::~WrappedSolidTypeProperty()
{
}

void WrappedSolidTypeProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Int32 nNewSolidType = ::com::sun::star::chart::ChartSolidType::RECTANGULAR_SOLID;
    if( ! (rOuterValue >>= nNewSolidType) )
        throw lang::IllegalArgumentException( C2U("Property SolidType requires integer value"), 0, 0 );

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
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    bool bFound = false;
    bool bAmbiguous = false;
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xDiagram.is() )
    {
        sal_Int32 nGeometry = DiagramHelper::getGeometry3D( xDiagram, bFound, bAmbiguous );
        if( bFound )
            m_aOuterValue <<= nGeometry;
    }
    return m_aOuterValue;
}

Any WrappedSolidTypeProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( ::com::sun::star::chart::ChartSolidType::RECTANGULAR_SOLID );
}

//-----------------------------------------------------------------------------------------------------------------

class WrappedAutomaticSizeProperty : public WrappedProperty
{
public:
    WrappedAutomaticSizeProperty();
    virtual ~WrappedAutomaticSizeProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

WrappedAutomaticSizeProperty::WrappedAutomaticSizeProperty()
            : WrappedProperty( C2U( "AutomaticSize" ), OUString() )
{
}

WrappedAutomaticSizeProperty::~WrappedAutomaticSizeProperty()
{
}

void WrappedAutomaticSizeProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( xInnerPropertySet.is() )
    {
        bool bNewValue = true;
        if( ! (rOuterValue >>= bNewValue) )
            throw lang::IllegalArgumentException( C2U("Property AutomaticSize requires value of type boolean"), 0, 0 );

        try
        {
            if( bNewValue )
            {
                Any aRelativeSize( xInnerPropertySet->getPropertyValue( C2U( "RelativeSize" ) ) );
                if( aRelativeSize.hasValue() )
                    xInnerPropertySet->setPropertyValue( C2U( "RelativeSize" ), Any() );
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

Any WrappedAutomaticSizeProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet( getPropertyDefault( Reference< beans::XPropertyState >( xInnerPropertySet, uno::UNO_QUERY ) ) );
    if( xInnerPropertySet.is() )
    {
        Any aRelativeSize( xInnerPropertySet->getPropertyValue( C2U( "RelativeSize" ) ) );
        if( !aRelativeSize.hasValue() )
            aRet <<= true;
    }
    return aRet;
}

Any WrappedAutomaticSizeProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    aRet <<= false;
    return aRet;
}

//-----------------------------------------------------------------------------------------------------------------

//PROP_DIAGRAM_INCLUDE_HIDDEN_CELLS
class WrappedIncludeHiddenCellsProperty : public WrappedProperty
{
public:
    WrappedIncludeHiddenCellsProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedIncludeHiddenCellsProperty();

    virtual void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
};

WrappedIncludeHiddenCellsProperty::WrappedIncludeHiddenCellsProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
            : WrappedProperty(C2U("IncludeHiddenCells"),C2U("IncludeHiddenCells"))
            , m_spChart2ModelContact( spChart2ModelContact )
{
}

WrappedIncludeHiddenCellsProperty::~WrappedIncludeHiddenCellsProperty()
{
}

void WrappedIncludeHiddenCellsProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( C2U("Property Dim3D requires boolean value"), 0, 0 );

    ChartModelHelper::setIncludeHiddenCells( bNewValue, m_spChart2ModelContact->getChartModel() );
}

//-----------------------------------------------------------------------------------------------------------------

// ____ XDiagramProvider ____
Reference< chart2::XDiagram > SAL_CALL DiagramWrapper::getDiagram()
    throw (uno::RuntimeException)
{
    return m_spChart2ModelContact->getChart2Diagram();
}

void SAL_CALL DiagramWrapper::setDiagram(
    const Reference< chart2::XDiagram >& /*xDiagram*/ )
    throw (uno::RuntimeException)
{
    //@todo: remove this method from interface
    OSL_FAIL("DiagramWrapper::setDiagram is not implemented, should be removed and not be called" );
}

// ================================================================================

Reference< beans::XPropertySet > DiagramWrapper::getInnerPropertySet()
{
    return Reference< beans::XPropertySet >( m_spChart2ModelContact->getChart2Diagram(), uno::UNO_QUERY );
}

const Sequence< beans::Property >& DiagramWrapper::getPropertySequence()
{
    return lcl_GetPropertySequence();
}

const std::vector< WrappedProperty* > DiagramWrapper::createWrappedProperties()
{
    ::std::vector< ::chart::WrappedProperty* > aWrappedProperties;

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

    aWrappedProperties.push_back( new WrappedDataRowSourceProperty( m_spChart2ModelContact ) );
    aWrappedProperties.push_back( new WrappedStackingProperty( StackMode_Y_STACKED,m_spChart2ModelContact ) );
    aWrappedProperties.push_back( new WrappedStackingProperty( StackMode_Y_STACKED_PERCENT, m_spChart2ModelContact ) );
    aWrappedProperties.push_back( new WrappedStackingProperty( StackMode_Z_STACKED, m_spChart2ModelContact ) );
    aWrappedProperties.push_back( new WrappedDim3DProperty( m_spChart2ModelContact ) );
    aWrappedProperties.push_back( new WrappedVerticalProperty( m_spChart2ModelContact ) );
    aWrappedProperties.push_back( new WrappedNumberOfLinesProperty( m_spChart2ModelContact ) );
    aWrappedProperties.push_back( new WrappedAttributedDataPointsProperty( m_spChart2ModelContact ) );
    aWrappedProperties.push_back( new WrappedProperty( C2U( "StackedBarsConnected" ), C2U( "ConnectBars" ) ) );
    aWrappedProperties.push_back( new WrappedSolidTypeProperty( m_spChart2ModelContact ) );
    aWrappedProperties.push_back( new WrappedAutomaticSizeProperty() );
    aWrappedProperties.push_back( new WrappedIncludeHiddenCellsProperty( m_spChart2ModelContact ) );

    return aWrappedProperties;
}

// ================================================================================

uno::Sequence< OUString > DiagramWrapper::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 8 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.Diagram" );
    aServices[ 1 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 2 ] = C2U( "com.sun.star.chart.StackableDiagram" );
    aServices[ 3 ] = C2U( "com.sun.star.chart.ChartAxisXSupplier" );
    aServices[ 4 ] = C2U( "com.sun.star.chart.ChartAxisYSupplier" );
    aServices[ 5 ] = C2U( "com.sun.star.chart.ChartAxisZSupplier" );
    aServices[ 6 ] = C2U( "com.sun.star.chart.ChartTwoAxisXSupplier" );
    aServices[ 7 ] = C2U( "com.sun.star.chart.ChartTwoAxisYSupplier" );

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( DiagramWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
