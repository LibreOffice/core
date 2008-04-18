/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: typegroupconverter.cxx,v $
 *
 * $Revision: 1.2 $
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

#include "oox/drawingml/chart/typegroupconverter.hxx"
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include "oox/drawingml/chart/seriesconverter.hxx"
#include "oox/drawingml/chart/typegroupmodel.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::chart2::XChartType;
using ::com::sun::star::chart2::XChartTypeContainer;
using ::com::sun::star::chart2::XCoordinateSystem;
using ::com::sun::star::chart2::XDataSeries;
using ::com::sun::star::chart2::XDataSeriesContainer;
using ::com::sun::star::chart2::XDiagram;
using ::com::sun::star::chart2::data::XLabeledDataSequence;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

namespace {

// chart type service names
const sal_Char SERVICE_CHART2_AREA[]    = "com.sun.star.chart2.AreaChartType";
const sal_Char SERVICE_CHART2_CANDLE[]  = "com.sun.star.chart2.CandleStickChartType";
const sal_Char SERVICE_CHART2_COLUMN[]  = "com.sun.star.chart2.ColumnChartType";
const sal_Char SERVICE_CHART2_LINE[]    = "com.sun.star.chart2.LineChartType";
const sal_Char SERVICE_CHART2_NET[]     = "com.sun.star.chart2.NetChartType";
const sal_Char SERVICE_CHART2_PIE[]     = "com.sun.star.chart2.PieChartType";
const sal_Char SERVICE_CHART2_SCATTER[] = "com.sun.star.chart2.ScatterChartType";
const sal_Char SERVICE_CHART2_SURFACE[] = "com.sun.star.chart2.ColumnChartType";    // Todo

static const TypeGroupInfo spTypeInfos[] =
{
    // type-id          type-category         service                 varied-point-color   combi  supp3d 3dwall polar  area2d area3d 1stvis xcateg swap   stack  revers betw
    { TYPEID_BAR,       TYPECATEGORY_BAR,     SERVICE_CHART2_COLUMN,  VARPOINTMODE_SINGLE, true,  true,  true,  false, true,  true,  false, true,  false, true,  false, true  },
    { TYPEID_HORBAR,    TYPECATEGORY_BAR,     SERVICE_CHART2_COLUMN,  VARPOINTMODE_SINGLE, false, true,  true,  false, true,  true,  false, true,  true,  true,  false, true  },
    { TYPEID_LINE,      TYPECATEGORY_LINE,    SERVICE_CHART2_LINE,    VARPOINTMODE_SINGLE, true,  true,  true,  false, false, true,  false, true,  false, true,  false, true  },
    { TYPEID_AREA,      TYPECATEGORY_LINE,    SERVICE_CHART2_AREA,    VARPOINTMODE_NONE,   true,  true,  true,  false, true,  true,  false, true,  false, true,  true,  false },
    { TYPEID_STOCK,     TYPECATEGORY_LINE,    SERVICE_CHART2_CANDLE,  VARPOINTMODE_NONE,   true,  false, false, false, false, false, false, true,  false, true,  false, true  },
    { TYPEID_RADARLINE, TYPECATEGORY_RADAR,   SERVICE_CHART2_NET,     VARPOINTMODE_SINGLE, false, false, false, true,  false, true,  false, true,  false, false, false, false },
    { TYPEID_RADARAREA, TYPECATEGORY_RADAR,   SERVICE_CHART2_NET,     VARPOINTMODE_NONE,   false, false, false, true,  true,  true,  false, true,  false, false, false, false },
    { TYPEID_PIE,       TYPECATEGORY_PIE,     SERVICE_CHART2_PIE,     VARPOINTMODE_MULTI,  false, true,  false, true,  true,  true,  true,  true,  false, false, false, false },
    { TYPEID_DOUGHNUT,  TYPECATEGORY_PIE,     SERVICE_CHART2_PIE,     VARPOINTMODE_MULTI,  false, true,  false, true,  true,  true,  false, true,  false, false, true,  false },
    { TYPEID_OFPIE,     TYPECATEGORY_PIE,     SERVICE_CHART2_PIE,     VARPOINTMODE_MULTI,  false, true,  false, true,  true,  true,  true,  true,  false, false, false, false },
    { TYPEID_SCATTER,   TYPECATEGORY_SCATTER, SERVICE_CHART2_SCATTER, VARPOINTMODE_SINGLE, true,  true,  false, false, false, true,  false, false, false, false, false, false },
    { TYPEID_BUBBLE,    TYPECATEGORY_SCATTER, SERVICE_CHART2_SCATTER, VARPOINTMODE_SINGLE, false, true,  false, false, true,  true,  false, false, false, false, false, false },
    { TYPEID_SURFACE,   TYPECATEGORY_SURFACE, SERVICE_CHART2_SURFACE, VARPOINTMODE_NONE,   false, true,  true,  false, true,  true,  false, true,  false, false, false, false },
};

static const TypeGroupInfo saUnknownTypeInfo =
    { TYPEID_UNKNOWN,   TYPECATEGORY_BAR,     SERVICE_CHART2_COLUMN,  VARPOINTMODE_SINGLE, true,  true,  true,  false, true,  true,  false, true,  false, true,  false, true  };

const TypeGroupInfo& lclGetTypeInfoFromTypeId( TypeId eTypeId )
{
    const TypeGroupInfo* pEnd = STATIC_ARRAY_END( spTypeInfos );
    for( const TypeGroupInfo* pIt = spTypeInfos; pIt != pEnd; ++pIt )
        if( pIt->meTypeId == eTypeId )
            return *pIt;
    OSL_ENSURE( eTypeId == TYPEID_UNKNOWN, "lclGetTypeInfoFromTypeId - unexpected chart type identifier" );
    return saUnknownTypeInfo;
}

} // namespace

// ============================================================================

TypeGroupConverter::TypeGroupConverter( const ConverterRoot& rParent, TypeGroupModel& rModel ) :
    ConverterBase< TypeGroupModel >( rParent, rModel ),
    mb3dChart( false )
{
    TypeId eTypeId = TYPEID_UNKNOWN;
    switch( mrModel.mnTypeId )
    {
#define ENSURE_AXESCOUNT( min, max ) OSL_ENSURE( (min <= (int)mrModel.maAxisIds.size()) && ((int)mrModel.maAxisIds.size() <= max), "TypeGroupConverter::TypeGroupConverter - invalid axes count" )
        case C_TOKEN( area3DChart ):    ENSURE_AXESCOUNT( 2, 3 ); eTypeId = TYPEID_AREA;      mb3dChart = true;   break;
        case C_TOKEN( areaChart ):      ENSURE_AXESCOUNT( 2, 2 ); eTypeId = TYPEID_AREA;      mb3dChart = false;  break;
        case C_TOKEN( bar3DChart ):     ENSURE_AXESCOUNT( 2, 3 ); eTypeId = TYPEID_BAR;       mb3dChart = true;   break;
        case C_TOKEN( barChart ):       ENSURE_AXESCOUNT( 2, 2 ); eTypeId = TYPEID_BAR;       mb3dChart = false;  break;
        case C_TOKEN( bubbleChart ):    ENSURE_AXESCOUNT( 2, 2 ); eTypeId = TYPEID_BUBBLE;    mb3dChart = false;  break;
        case C_TOKEN( doughnutChart ):  ENSURE_AXESCOUNT( 0, 0 ); eTypeId = TYPEID_DOUGHNUT;  mb3dChart = false;  break;
        case C_TOKEN( line3DChart ):    ENSURE_AXESCOUNT( 3, 3 ); eTypeId = TYPEID_LINE;      mb3dChart = true;   break;
        case C_TOKEN( lineChart ):      ENSURE_AXESCOUNT( 2, 2 ); eTypeId = TYPEID_LINE;      mb3dChart = false;  break;
        case C_TOKEN( ofPieChart ):     ENSURE_AXESCOUNT( 0, 0 ); eTypeId = TYPEID_OFPIE;     mb3dChart = false;  break;
        case C_TOKEN( pie3DChart ):     ENSURE_AXESCOUNT( 0, 0 ); eTypeId = TYPEID_PIE;       mb3dChart = true;   break;
        case C_TOKEN( pieChart ):       ENSURE_AXESCOUNT( 0, 0 ); eTypeId = TYPEID_PIE;       mb3dChart = false;  break;
        case C_TOKEN( radarChart ):     ENSURE_AXESCOUNT( 2, 2 ); eTypeId = TYPEID_RADARLINE; mb3dChart = false;  break;
        case C_TOKEN( scatterChart ):   ENSURE_AXESCOUNT( 2, 2 ); eTypeId = TYPEID_SCATTER;   mb3dChart = false;  break;
        case C_TOKEN( stockChart ):     ENSURE_AXESCOUNT( 2, 2 ); eTypeId = TYPEID_STOCK;     mb3dChart = false;  break;
        case C_TOKEN( surface3DChart ): ENSURE_AXESCOUNT( 3, 3 ); eTypeId = TYPEID_SURFACE;   mb3dChart = true;   break;
        case C_TOKEN( surfaceChart ):   ENSURE_AXESCOUNT( 2, 3 ); eTypeId = TYPEID_SURFACE;   mb3dChart = false;  break;
        default:    OSL_ENSURE( false, "TypeGroupConverter::TypeGroupConverter - unknown chart type" );
#undef ENSURE_AXESCOUNT
    }

    // special handling for some chart types
    switch( eTypeId )
    {
        case TYPEID_BAR:
            if( mrModel.mnBarDir == XML_bar )
                eTypeId = TYPEID_HORBAR;
        break;
        case TYPEID_RADARLINE:
            if( mrModel.mnRadarStyle == XML_filled )
                eTypeId = TYPEID_RADARAREA;
        break;
        default:;
    }

    // set the chart type info struct for the current chart type
    maTypeInfo = lclGetTypeInfoFromTypeId( eTypeId );
}

TypeGroupConverter::~TypeGroupConverter()
{
}

bool TypeGroupConverter::isStacked() const
{
    return maTypeInfo.mbSupportsStacking && (mrModel.mnGrouping == XML_stacked);
}

bool TypeGroupConverter::isPercent() const
{
    return maTypeInfo.mbSupportsStacking && (mrModel.mnGrouping == XML_percentStacked);
}

bool TypeGroupConverter::is3dChart() const
{
    return mb3dChart;
}

bool TypeGroupConverter::isWall3dChart() const
{
    return mb3dChart && maTypeInfo.mbWalls3d;
}

bool TypeGroupConverter::isDeep3dChart() const
{
    return isWall3dChart() && (mrModel.mnGrouping == XML_standard);
}

bool TypeGroupConverter::isSeriesFrameFormat() const
{
    return mb3dChart ? maTypeInfo.mbSeriesIsFrame3d : maTypeInfo.mbSeriesIsFrame2d;
}

bool TypeGroupConverter::isReverseSeries() const
{
    return maTypeInfo.mbReverseSeries && !mb3dChart && !isStacked() && !isPercent();
}

OUString TypeGroupConverter::getSingleSeriesTitle() const
{
    OUString aSeriesTitle;
    if( !mrModel.maSeries.empty() && (maTypeInfo.mbSingleSeriesVis || (mrModel.maSeries.size() == 1)) )
        if( const TextModel* pText = mrModel.maSeries.front()->mxText.get() )
            if( const DataSequenceModel* pDataSeq = pText->mxDataSeq.get() )
                if( !pDataSeq->maData.empty() )
                    pDataSeq->maData.begin()->second >>= aSeriesTitle;
    return aSeriesTitle;
}

Reference< XCoordinateSystem > TypeGroupConverter::createCoordinateSystem()
{
    // find service name for coordinate system
    OUString aServiceName;
    if( maTypeInfo.mbPolarCoordSystem )
    {
        if( mb3dChart )
            aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.PolarCoordinateSystem3d" );
        else
            aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.PolarCoordinateSystem2d" );
    }
    else
    {
        if( mb3dChart )
            aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.CartesianCoordinateSystem3d" );
        else
            aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.CartesianCoordinateSystem2d" );
    }

    // create the coordinate system object
    Reference< XCoordinateSystem > xCoordSystem( createInstance( aServiceName ), UNO_QUERY );

    // swap X and Y axis
    if( maTypeInfo.mbSwappedAxesSet )
    {
        PropertySet aPropSet( xCoordSystem );
        aPropSet.setProperty( CREATE_OUSTRING( "SwapXAndYAxis" ), true );
    }

    return xCoordSystem;
}

Reference< XLabeledDataSequence > TypeGroupConverter::createCategorySequence()
{
    Reference< XLabeledDataSequence > xLabeledSeq;
    // create category sequence from first visible series
    if( !mrModel.maSeries.empty() )
    {
        SeriesConverter aSeriesConv( *this, isReverseSeries() ? *mrModel.maSeries.back() : *mrModel.maSeries.front() );
        xLabeledSeq = aSeriesConv.createCategorySequence( CREATE_OUSTRING( "categories" ) );
    }
    return xLabeledSeq;
}

void TypeGroupConverter::convertModelToDocument( const Reference< XDiagram >& rxDiagram,
        const Reference< XCoordinateSystem >& rxCoordSystem, sal_Int32 nAxesSetIdx )
{
    try
    {
        // create the chart type object
        OUString aService = OUString::createFromAscii( maTypeInfo.mpcServiceName );
        Reference< XChartType > xChartType( createInstance( aService ), UNO_QUERY_THROW );

        // additional properties
        PropertySet aTypeProp( xChartType );
        switch( maTypeInfo.meTypeCategory )
        {
            case TYPECATEGORY_BAR:
            {
                Sequence< sal_Int32 > aInt32Seq( 2 );
                aInt32Seq[ 0 ] = aInt32Seq[ 1 ] = mrModel.mnOverlap;
                aTypeProp.setProperty( CREATE_OUSTRING( "OverlapSequence" ), aInt32Seq );
                aInt32Seq[ 0 ] = aInt32Seq[ 1 ] = mrModel.mnGapWidth;
                aTypeProp.setProperty( CREATE_OUSTRING( "GapwidthSequence" ), aInt32Seq );
            }
            break;
            case TYPECATEGORY_PIE:
            {
                aTypeProp.setProperty( CREATE_OUSTRING( "UseRings" ), maTypeInfo.meTypeId == TYPEID_DOUGHNUT );
                // #i85166# starting angle of first pie slice
                PropertySet aDiagramProp( rxDiagram );
                sal_Int32 nAngle = (450 - getLimitedValue< sal_Int32, sal_Int32 >( mrModel.mnFirstAngle, 0, 360 )) % 360;
                aDiagramProp.setProperty( CREATE_OUSTRING( "StartingAngle" ), nAngle );
            }
            break;
            default:;
        }

        // create converter objects for all series models
        typedef RefVector< SeriesConverter > SeriesConvVector;
        SeriesConvVector aSeries;
        for( TypeGroupModel::SeriesVector::iterator aIt = mrModel.maSeries.begin(), aEnd = mrModel.maSeries.end(); aIt != aEnd; ++aIt )
            aSeries.push_back( SeriesConvVector::value_type( new SeriesConverter( *this, **aIt ) ) );

        // reverse series order for some unstacked 2D chart types
        if( isReverseSeries() )
            ::std::reverse( aSeries.begin(), aSeries.end() );

        /*  Stock chart needs special processing. Create one 'big' series with
            data sequences of different roles. */
        if( maTypeInfo.meTypeId == TYPEID_STOCK )
        {
        }
        else
        {
            for( SeriesConvVector::iterator aIt = aSeries.begin(), aEnd = aSeries.end(); aIt != aEnd; ++aIt )
            {
                SeriesConverter& rSeriesConv = **aIt;
                Reference< XDataSeries > xDataSeries = rSeriesConv.createDataSeries( *this );
                insertDataSeries( xChartType, xDataSeries, nAxesSetIdx );

                /*  The ECMA spec states that the <c:smooth> element can be a
                    child of the <c:lineChart>, but Excel 2007 ignores it. Line
                    smoothing is always set with the <c:smooth> element in the
                    single data series. If one line is smoothed, set the entire
                    chart type to smoothed mode (#i66858# single smoothed
                    series not supported). */
                if( rSeriesConv.getModel().mobSmooth.get( false ) )
                    aTypeProp.setProperty( CREATE_OUSTRING( "CurveStyle" ), ::com::sun::star::chart2::CurveStyle_CUBIC_SPLINES );
            }
        }

        // add chart type object to coordinate system
        Reference< XChartTypeContainer > xChartTypeCont( rxCoordSystem, UNO_QUERY_THROW );
        xChartTypeCont->addChartType( xChartType );

        // set existence of bar connector lines at diagram (only in stacked 2D bar charts)
        if( mrModel.mxSerLines.is() && !mb3dChart && (maTypeInfo.meTypeCategory == TYPECATEGORY_BAR) && (isStacked() || isPercent()) )
        {
            PropertySet aDiaProp( rxDiagram );
            aDiaProp.setProperty( CREATE_OUSTRING( "ConnectBars" ), true );
        }
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "TypeGroupConverter::convertModelToDocument - cannot add chart type" );
    }
}

// private --------------------------------------------------------------------

void TypeGroupConverter::insertDataSeries( const Reference< XChartType >& rxChartType, const Reference< XDataSeries >& rxSeries, sal_Int32 nAxesSetIdx )
{
    if( rxSeries.is() )
    {
        PropertySet aSeriesProp( rxSeries );

        // series stacking mode
        namespace cssc = ::com::sun::star::chart2;
        cssc::StackingDirection eStacking = cssc::StackingDirection_NO_STACKING;
        // stacked overrides deep-3d
        if( isStacked() || isPercent() )
            eStacking = cssc::StackingDirection_Y_STACKING;
        else if( isDeep3dChart() )
            eStacking = cssc::StackingDirection_Z_STACKING;
        aSeriesProp.setProperty( CREATE_OUSTRING( "StackingDirection" ), eStacking );

        // additional series properties
        aSeriesProp.setProperty( CREATE_OUSTRING( "AttachedAxisIndex" ), nAxesSetIdx );

        // insert series into container
        try
        {
            Reference< XDataSeriesContainer > xSeriesCont( rxChartType, UNO_QUERY_THROW );
            xSeriesCont->addDataSeries( rxSeries );
        }
        catch( Exception& )
        {
            OSL_ENSURE( false, "TypeGroupConverter::insertDataSeries - cannot add data series" );
        }
    }
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

