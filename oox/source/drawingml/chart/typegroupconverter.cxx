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
 * $Revision: 1.4 $
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
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/chart/seriesconverter.hxx"
#include "oox/drawingml/chart/typegroupmodel.hxx"
#include "properties.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::chart2::XChartType;
using ::com::sun::star::chart2::XChartTypeContainer;
using ::com::sun::star::chart2::XCoordinateSystem;
using ::com::sun::star::chart2::XDataSeries;
using ::com::sun::star::chart2::XDataSeriesContainer;
using ::com::sun::star::chart2::XDiagram;
using ::com::sun::star::chart2::data::XDataSink;
using ::com::sun::star::chart2::data::XLabeledDataSequence;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

namespace {

// chart type service names
const sal_Char SERVICE_CHART2_AREA[]      = "com.sun.star.chart2.AreaChartType";
const sal_Char SERVICE_CHART2_CANDLE[]    = "com.sun.star.chart2.CandleStickChartType";
const sal_Char SERVICE_CHART2_COLUMN[]    = "com.sun.star.chart2.ColumnChartType";
const sal_Char SERVICE_CHART2_LINE[]      = "com.sun.star.chart2.LineChartType";
const sal_Char SERVICE_CHART2_NET[]       = "com.sun.star.chart2.NetChartType";
const sal_Char SERVICE_CHART2_FILLEDNET[] = "com.sun.star.chart2.FilledNetChartType";
const sal_Char SERVICE_CHART2_PIE[]       = "com.sun.star.chart2.PieChartType";
const sal_Char SERVICE_CHART2_SCATTER[]   = "com.sun.star.chart2.ScatterChartType";
const sal_Char SERVICE_CHART2_BUBBLE[]    = "com.sun.star.chart2.BubbleChartType";
const sal_Char SERVICE_CHART2_SURFACE[]   = "com.sun.star.chart2.ColumnChartType";    // Todo

namespace csscd = ::com::sun::star::chart::DataLabelPlacement;

static const TypeGroupInfo spTypeInfos[] =
{
    // type-id          type-category         service                   varied-point-color   default label pos     comb2d supp3d polar  area2d 1stvis xcateg swap   stack  revers betw   picopt
    { TYPEID_BAR,       TYPECATEGORY_BAR,     SERVICE_CHART2_COLUMN,    VARPOINTMODE_SINGLE, csscd::OUTSIDE,       true,  true,  false, true,  false, true,  false, true,  false, true,  true  },
    { TYPEID_HORBAR,    TYPECATEGORY_BAR,     SERVICE_CHART2_COLUMN,    VARPOINTMODE_SINGLE, csscd::OUTSIDE,       false, true,  false, true,  false, true,  true,  true,  false, true,  true  },
    { TYPEID_LINE,      TYPECATEGORY_LINE,    SERVICE_CHART2_LINE,      VARPOINTMODE_SINGLE, csscd::RIGHT,         true,  true,  false, false, false, true,  false, true,  false, true,  false },
    { TYPEID_AREA,      TYPECATEGORY_LINE,    SERVICE_CHART2_AREA,      VARPOINTMODE_NONE,   csscd::CENTER,        true,  true,  false, true,  false, true,  false, true,  true,  false, false },
    { TYPEID_STOCK,     TYPECATEGORY_LINE,    SERVICE_CHART2_CANDLE,    VARPOINTMODE_NONE,   csscd::RIGHT,         true,  false, false, false, false, true,  false, true,  false, true,  false },
    { TYPEID_RADARLINE, TYPECATEGORY_RADAR,   SERVICE_CHART2_NET,       VARPOINTMODE_SINGLE, csscd::TOP,           false, false, true,  false, false, true,  false, false, false, false, false },
    { TYPEID_RADARAREA, TYPECATEGORY_RADAR,   SERVICE_CHART2_FILLEDNET, VARPOINTMODE_NONE,   csscd::TOP,           false, false, true,  true,  false, true,  false, false, true,  false, false },
    { TYPEID_PIE,       TYPECATEGORY_PIE,     SERVICE_CHART2_PIE,       VARPOINTMODE_MULTI,  csscd::AVOID_OVERLAP, false, true,  true,  true,  true,  true,  false, false, false, false, false },
    { TYPEID_DOUGHNUT,  TYPECATEGORY_PIE,     SERVICE_CHART2_PIE,       VARPOINTMODE_MULTI,  csscd::AVOID_OVERLAP, false, true,  true,  true,  false, true,  false, false, false, false, false },
    { TYPEID_OFPIE,     TYPECATEGORY_PIE,     SERVICE_CHART2_PIE,       VARPOINTMODE_MULTI,  csscd::AVOID_OVERLAP, false, true,  true,  true,  true,  true,  false, false, false, false, false },
    { TYPEID_SCATTER,   TYPECATEGORY_SCATTER, SERVICE_CHART2_SCATTER,   VARPOINTMODE_SINGLE, csscd::RIGHT,         true,  true,  false, false, false, false, false, false, false, false, false },
    { TYPEID_BUBBLE,    TYPECATEGORY_SCATTER, SERVICE_CHART2_BUBBLE,    VARPOINTMODE_SINGLE, csscd::RIGHT,         false, false, false, true,  false, false, false, false, false, false, false },
    { TYPEID_SURFACE,   TYPECATEGORY_SURFACE, SERVICE_CHART2_SURFACE,   VARPOINTMODE_NONE,   csscd::RIGHT,         false, true,  false, true,  false, true,  false, false, false, false, false }
};

static const TypeGroupInfo saUnknownTypeInfo =
    { TYPEID_UNKNOWN,   TYPECATEGORY_BAR,     SERVICE_CHART2_COLUMN,  VARPOINTMODE_SINGLE, csscd::OUTSIDE,       true,  true,  false, true,  false, true,  false, true,  false, true,  true  };

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

UpDownBarsConverter::UpDownBarsConverter( const ConverterRoot& rParent, UpDownBarsModel& rModel ) :
    ConverterBase< UpDownBarsModel >( rParent, rModel )
{
}

UpDownBarsConverter::~UpDownBarsConverter()
{
}

void UpDownBarsConverter::convertFromModel( const Reference< XChartType >& rxChartType )
{
    PropertySet aTypeProp( rxChartType );

    // upbar format
    Reference< XPropertySet > xWhitePropSet;
    if( aTypeProp.getProperty( xWhitePropSet, PROP_WhiteDay ) )
    {
        PropertySet aPropSet( xWhitePropSet );
        getFormatter().convertFrameFormatting( aPropSet, mrModel.mxUpBars, OBJECTTYPE_UPBAR );
    }

    // downbar format
    Reference< XPropertySet > xBlackPropSet;
    if( aTypeProp.getProperty( xBlackPropSet, PROP_BlackDay ) )
    {
        PropertySet aPropSet( xBlackPropSet );
        getFormatter().convertFrameFormatting( aPropSet, mrModel.mxDownBars, OBJECTTYPE_DOWNBAR );
    }
}

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
        case C_TOKEN( surfaceChart ):   ENSURE_AXESCOUNT( 2, 3 ); eTypeId = TYPEID_SURFACE;   mb3dChart = true;   break;    // 3D bar chart from all surface charts
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
        case TYPEID_SURFACE:
            // create a deep 3D bar chart from surface charts
            mrModel.mnGrouping = XML_standard;
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
    return mb3dChart && (maTypeInfo.meTypeCategory != TYPECATEGORY_PIE);
}

bool TypeGroupConverter::isDeep3dChart() const
{
    return isWall3dChart() && (mrModel.mnGrouping == XML_standard);
}

bool TypeGroupConverter::isSeriesFrameFormat() const
{
    return mb3dChart || maTypeInfo.mbSeriesIsFrame2d;
}

ObjectType TypeGroupConverter::getSeriesObjectType() const
{
    return mb3dChart ? OBJECTTYPE_FILLEDSERIES3D :
        (maTypeInfo.mbSeriesIsFrame2d ? OBJECTTYPE_FILLEDSERIES2D : OBJECTTYPE_LINEARSERIES2D);
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
        aPropSet.setProperty( PROP_SwapXAndYAxis, true );
    }

    return xCoordSystem;
}

Reference< XLabeledDataSequence > TypeGroupConverter::createCategorySequence()
{
    Reference< XLabeledDataSequence > xLabeledSeq;
    /*  Find first existing category sequence. The bahaviour of Excel 2007 is
        different to Excel 2003, which always used the category sequence of the
        first series, even if it was empty. */
    for( TypeGroupModel::SeriesVector::iterator aIt = mrModel.maSeries.begin(), aEnd = mrModel.maSeries.end(); !xLabeledSeq.is() && (aIt != aEnd); ++aIt )
    {
        if( (*aIt)->maSources.has( SeriesModel::CATEGORIES ) )
        {
            SeriesConverter aSeriesConv( *this, **aIt );
            xLabeledSeq = aSeriesConv.createCategorySequence( CREATE_OUSTRING( "categories" ) );
        }
    }
    return xLabeledSeq;
}

void TypeGroupConverter::convertFromModel( const Reference< XDiagram >& rxDiagram,
        const Reference< XCoordinateSystem >& rxCoordSystem,
        sal_Int32 nAxesSetIdx, bool bSupportsVaryColorsByPoint )
{
    try
    {
        // create the chart type object
        OUString aService = OUString::createFromAscii( maTypeInfo.mpcServiceName );
        Reference< XChartType > xChartType( createInstance( aService ), UNO_QUERY_THROW );

        // additional properties
        PropertySet aDiaProp( rxDiagram );
        PropertySet aTypeProp( xChartType );
        switch( maTypeInfo.meTypeCategory )
        {
            case TYPECATEGORY_BAR:
            {
                Sequence< sal_Int32 > aInt32Seq( 2 );
                aInt32Seq[ 0 ] = aInt32Seq[ 1 ] = mrModel.mnOverlap;
                aTypeProp.setProperty( PROP_OverlapSequence, aInt32Seq );
                aInt32Seq[ 0 ] = aInt32Seq[ 1 ] = mrModel.mnGapWidth;
                aTypeProp.setProperty( PROP_GapwidthSequence, aInt32Seq );
            }
            break;
            case TYPECATEGORY_PIE:
            {
                aTypeProp.setProperty( PROP_UseRings, maTypeInfo.meTypeId == TYPEID_DOUGHNUT );
                /*  #i85166# starting angle of first pie slice. 3D pie charts
                    use Y rotation setting in view3D element. Of-pie charts do
                    not support pie rotation. */
                if( !is3dChart() && (maTypeInfo.meTypeId != TYPEID_OFPIE) )
                    convertPieRotation( aDiaProp, mrModel.mnFirstAngle );
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

        // decide whether to use varying colors for each data point
        bool bVaryColorsByPoint = bSupportsVaryColorsByPoint && mrModel.mbVaryColors;
        switch( maTypeInfo.meVarPointMode )
        {
            case VARPOINTMODE_NONE:     bVaryColorsByPoint = false;                             break;
            case VARPOINTMODE_SINGLE:   bVaryColorsByPoint &= (mrModel.maSeries.size() == 1);   break;
            case VARPOINTMODE_MULTI:                                                            break;
        }

        /*  Stock chart needs special processing. Create one 'big' series with
            data sequences of different roles. */
        if( maTypeInfo.meTypeId == TYPEID_STOCK )
        {
            // create the data series object
            Reference< XDataSeries > xDataSeries( createInstance( CREATE_OUSTRING( "com.sun.star.chart2.DataSeries" ) ), UNO_QUERY );
            Reference< XDataSink > xDataSink( xDataSeries, UNO_QUERY );
            if( xDataSink.is() )
            {
                // create a list of data sequences from all series
                ::std::vector< Reference< XLabeledDataSequence > > aLabeledSeqVec;
                OSL_ENSURE( aSeries.size() >= 3, "TypeGroupConverter::convertFromModel - too few stock chart series" );
                int nRoleIdx = (aSeries.size() == 3) ? 1 : 0;
                for( SeriesConvVector::iterator aIt = aSeries.begin(), aEnd = aSeries.end(); (nRoleIdx < 4) && (aIt != aEnd); ++nRoleIdx, ++aIt )
                {
                    // create a data sequence with a specific role
                    OUString aRole;
                    switch( nRoleIdx )
                    {
                        case 0: aRole = CREATE_OUSTRING( "values-first" );  break;
                        case 1: aRole = CREATE_OUSTRING( "values-max" );    break;
                        case 2: aRole = CREATE_OUSTRING( "values-min" );    break;
                        case 3: aRole = CREATE_OUSTRING( "values-last" );   break;
                    }
                    Reference< XLabeledDataSequence > xDataSeq = (*aIt)->createValueSequence( aRole );
                    if( xDataSeq.is() )
                        aLabeledSeqVec.push_back( xDataSeq );
                }

                // attach labeled data sequences to series and insert series into chart type
                xDataSink->setData( ContainerHelper::vectorToSequence( aLabeledSeqVec ) );

                // formatting of high/low lines
                aTypeProp.setProperty( PROP_ShowHighLow, true );
                PropertySet aSeriesProp( xDataSeries );
                if( mrModel.mxHiLowLines.is() )
                    getFormatter().convertFrameFormatting( aSeriesProp, mrModel.mxHiLowLines, OBJECTTYPE_HILOLINE );
                else
                    // hi/low-lines cannot be switched off via "ShowHighLow" property (?)
                    aSeriesProp.setProperty( PROP_LineStyle, ::com::sun::star::drawing::LineStyle_NONE );

                // formatting of up/down bars
                bool bUpDownBars = mrModel.mxUpDownBars.is();
                aTypeProp.setProperty( PROP_Japanese, bUpDownBars );
                aTypeProp.setProperty( PROP_ShowFirst, bUpDownBars );
                if( bUpDownBars )
                {
                    UpDownBarsConverter aUpDownConv( *this, *mrModel.mxUpDownBars );
                    aUpDownConv.convertFromModel( xChartType );
                }

                // insert the series into the chart type object
                insertDataSeries( xChartType, xDataSeries, nAxesSetIdx );
            }
        }
        else
        {
            for( SeriesConvVector::iterator aIt = aSeries.begin(), aEnd = aSeries.end(); aIt != aEnd; ++aIt )
            {
                SeriesConverter& rSeriesConv = **aIt;
                Reference< XDataSeries > xDataSeries = rSeriesConv.createDataSeries( *this, bVaryColorsByPoint );
                insertDataSeries( xChartType, xDataSeries, nAxesSetIdx );

                /*  Excel does not use the value of the c:smooth element of the
                    chart type to set a default line smoothing for the data
                    series. Line smoothing is always controlled by the c:smooth
                    element of the respective data series. If the element in the
                    data series is missing, line smoothing is off, regardless of
                    the c:smooth element of the chart type. */
#if !OOX_CHART_SMOOTHED_PER_SERIES
                if( rSeriesConv.getModel().mbSmooth )
                    convertLineSmooth( aTypeProp, true );
#endif
            }
        }

        // add chart type object to coordinate system
        Reference< XChartTypeContainer > xChartTypeCont( rxCoordSystem, UNO_QUERY_THROW );
        xChartTypeCont->addChartType( xChartType );

        // set existence of bar connector lines at diagram (only in stacked 2D bar charts)
        if( mrModel.mxSerLines.is() && !mb3dChart && (maTypeInfo.meTypeCategory == TYPECATEGORY_BAR) && (isStacked() || isPercent()) )
            aDiaProp.setProperty( PROP_ConnectBars, true );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "TypeGroupConverter::convertFromModel - cannot add chart type" );
    }
}

void TypeGroupConverter::convertMarker( PropertySet& rPropSet, sal_Int32 nOoxSymbol, sal_Int32 nOoxSize ) const
{
    if( !isSeriesFrameFormat() )
    {
        namespace cssc = ::com::sun::star::chart2;

        // symbol style
        cssc::Symbol aSymbol;
        aSymbol.Style = cssc::SymbolStyle_STANDARD;
        switch( nOoxSymbol )
        {
            case XML_auto:      aSymbol.Style = cssc::SymbolStyle_AUTO; break;
            case XML_none:      aSymbol.Style = cssc::SymbolStyle_NONE; break;
            case XML_square:    aSymbol.StandardSymbol = 0;             break;  // square
            case XML_diamond:   aSymbol.StandardSymbol = 1;             break;  // diamond
            case XML_triangle:  aSymbol.StandardSymbol = 3;             break;  // arrow up
            case XML_x:         aSymbol.StandardSymbol = 6;             break;  // bow tie
            case XML_star:      aSymbol.StandardSymbol = 7;             break;  // sand glass
            case XML_dot:       aSymbol.StandardSymbol = 4;             break;  // arrow right
            case XML_dash:      aSymbol.StandardSymbol = 2;             break;  // arrow down
            case XML_circle:    aSymbol.StandardSymbol = 4;             break;  // arrow right
            case XML_plus:      aSymbol.StandardSymbol = 5;             break;  // arrow left
        }

        // symbol size (points in OOXML, 1/100 mm in Chart2)
        sal_Int32 nSize = static_cast< sal_Int32 >( nOoxSize * (2540.0 / 72.0) + 0.5 );
        aSymbol.Size.Width = aSymbol.Size.Height = nSize;

        // set the property
        rPropSet.setProperty( PROP_Symbol, aSymbol );
    }
}

void TypeGroupConverter::convertLineSmooth( PropertySet& rPropSet, bool bOoxSmooth ) const
{
    if( !isSeriesFrameFormat() && (maTypeInfo.meTypeCategory != TYPECATEGORY_RADAR) )
    {
        namespace cssc = ::com::sun::star::chart2;
        cssc::CurveStyle eCurveStyle = bOoxSmooth ? cssc::CurveStyle_CUBIC_SPLINES : cssc::CurveStyle_LINES;
        rPropSet.setProperty( PROP_CurveStyle, eCurveStyle );
    }
}

void TypeGroupConverter::convertBarGeometry( PropertySet& rPropSet, sal_Int32 nOoxShape ) const
{
    if( mb3dChart && (maTypeInfo.meTypeCategory == TYPECATEGORY_BAR) )
    {
        namespace cssc = ::com::sun::star::chart2;

        sal_Int32 nGeom3d = cssc::DataPointGeometry3D::CUBOID;
        switch( nOoxShape )
        {
            case XML_box:           nGeom3d = cssc::DataPointGeometry3D::CUBOID;    break;
            case XML_cone:          nGeom3d = cssc::DataPointGeometry3D::CONE;      break;
            case XML_coneToMax:     nGeom3d = cssc::DataPointGeometry3D::CONE;      break;
            case XML_cylinder:      nGeom3d = cssc::DataPointGeometry3D::CYLINDER;  break;
            case XML_pyramid:       nGeom3d = cssc::DataPointGeometry3D::PYRAMID;   break;
            case XML_pyramidToMax:  nGeom3d = cssc::DataPointGeometry3D::PYRAMID;   break;
            default:                OSL_ENSURE( false, "TypeGroupConverter::convertBarGeometry - unknown 3D bar shape type" );
        }
        rPropSet.setProperty( PROP_Geometry3D, nGeom3d );
    }
}

void TypeGroupConverter::convertPieRotation( PropertySet& rPropSet, sal_Int32 nOoxAngle ) const
{
    if( maTypeInfo.meTypeCategory == TYPECATEGORY_PIE )
    {
        // map OOXML [0,360] clockwise (0deg top) to Chart2 counterclockwise (0deg left)
        sal_Int32 nAngle = (450 - nOoxAngle) % 360;
        rPropSet.setProperty( PROP_StartingAngle, nAngle );
    }
}

void TypeGroupConverter::convertPieExplosion( PropertySet& rPropSet, sal_Int32 nOoxExplosion ) const
{
    if( maTypeInfo.meTypeCategory == TYPECATEGORY_PIE )
    {
        // pie explosion restricted to 100% in Chart2, set as double in range [0,1]
        double fOffset = getLimitedValue< double >( nOoxExplosion / 100.0, 0.0, 1.0 );
        rPropSet.setProperty( PROP_Offset, fOffset );
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
        aSeriesProp.setProperty( PROP_StackingDirection, eStacking );

        // additional series properties
        aSeriesProp.setProperty( PROP_AttachedAxisIndex, nAxesSetIdx );

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

