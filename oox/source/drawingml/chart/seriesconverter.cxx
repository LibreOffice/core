/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: seriesconverter.cxx,v $
 *
 * $Revision: 1.4.6.2 $
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

#include "oox/drawingml/chart/seriesconverter.hxx"
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include "oox/drawingml/chart/datasourceconverter.hxx"
#include "oox/drawingml/chart/seriesmodel.hxx"
#include "oox/drawingml/chart/titleconverter.hxx"
#include "oox/drawingml/chart/typegroupconverter.hxx"
#include "oox/drawingml/chart/typegroupmodel.hxx"
#include "properties.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::chart2::DataPointLabel;
using ::com::sun::star::chart2::XDataSeries;
using ::com::sun::star::chart2::XRegressionCurve;
using ::com::sun::star::chart2::XRegressionCurveContainer;
using ::com::sun::star::chart2::data::XDataSequence;
using ::com::sun::star::chart2::data::XDataSink;
using ::com::sun::star::chart2::data::XLabeledDataSequence;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

namespace {

Reference< XLabeledDataSequence > lclCreateLabeledDataSequence(
        const ConverterRoot& rParent,
        DataSourceModel* pValues, const OUString& rRole,
        TextModel* pTitle = 0 )
{
    // create data sequence for values
    Reference< XDataSequence > xValueSeq;
    if( pValues )
    {
        DataSourceConverter aSourceConv( rParent, *pValues );
        xValueSeq = aSourceConv.createDataSequence( rRole );
    }

    // create data sequence for title
    Reference< XDataSequence > xTitleSeq;
    if( pTitle )
    {
        TextConverter aTextConv( rParent, *pTitle );
        xTitleSeq = aTextConv.createDataSequence( CREATE_OUSTRING( "label" ) );
    }

    // create the labeled data sequence, if values or title are present
    Reference< XLabeledDataSequence > xLabeledSeq;
    if( xValueSeq.is() || xTitleSeq.is() )
    {
        xLabeledSeq.set( rParent.createInstance( CREATE_OUSTRING( "com.sun.star.chart2.data.LabeledDataSequence" ) ), UNO_QUERY );
        if( xLabeledSeq.is() )
        {
            xLabeledSeq->setValues( xValueSeq );
            xLabeledSeq->setLabel( xTitleSeq );
        }
    }
    return xLabeledSeq;
}

void lclConvertLabelFormatting( PropertySet& rPropSet, ObjectFormatter& rFormatter,
        const DataLabelModelBase& rDataLabel, const TypeGroupConverter& rTypeGroup, bool bDataSeriesLabel )
{
    const TypeGroupInfo& rTypeInfo = rTypeGroup.getTypeInfo();

    /*  Excel 2007 does not change the series setting for a single data point,
        if none of some specific elements occur. But only one existing element
        in a data point will reset most other of these elements from the series
        (e.g.: series has <c:showVal>, data point has <c:showCatName>, this
        will reset <c:showVal> for this point, unless <c:showVal> is repeated
        in the data point). The elements <c:layout>, <c:numberFormat>,
        <c:spPr>, <c:tx>, and <c:txPr> are not affected at all. */
    bool bHasAnyElement =
        rDataLabel.moaSeparator.has() || rDataLabel.monLabelPos.has() ||
        rDataLabel.mobShowCatName.has() || rDataLabel.mobShowLegendKey.has() ||
        rDataLabel.mobShowPercent.has() || rDataLabel.mobShowSerName.has() ||
        rDataLabel.mobShowVal.has();

    bool bShowValue   = !rDataLabel.mbDeleted && rDataLabel.mobShowVal.get( false );
    bool bShowPercent = !rDataLabel.mbDeleted && rDataLabel.mobShowPercent.get( false ) && (rTypeInfo.meTypeCategory == TYPECATEGORY_PIE);
    bool bShowCateg   = !rDataLabel.mbDeleted && rDataLabel.mobShowCatName.get( false );
    bool bShowSymbol  = !rDataLabel.mbDeleted && rDataLabel.mobShowLegendKey.get( false );

    // type of attached label
    if( bHasAnyElement || rDataLabel.mbDeleted )
    {
        DataPointLabel aPointLabel( bShowValue, bShowPercent, bShowCateg, bShowSymbol );
        rPropSet.setProperty( PROP_Label, aPointLabel );
    }

    if( !rDataLabel.mbDeleted )
    {
        // data label number format (percentage format wins over value format)
        rFormatter.convertNumberFormat( rPropSet, rDataLabel.maNumberFormat, bShowPercent );

        // data label text formatting (frame formatting not supported by Chart2)
        rFormatter.convertTextFormatting( rPropSet, rDataLabel.mxTextProp, OBJECTTYPE_DATALABEL );
        rFormatter.convertTextRotation( rPropSet, rDataLabel.mxTextProp, false );

        // data label separator (do not overwrite series separator, if no explicit point separator is present)
        if( bDataSeriesLabel || rDataLabel.moaSeparator.has() )
            rPropSet.setProperty( PROP_LabelSeparator, rDataLabel.moaSeparator.get( CREATE_OUSTRING( "; " ) ) );

        // data label placement (do not overwrite series placement, if no explicit point placement is present)
        if( bDataSeriesLabel || rDataLabel.monLabelPos.has() )
        {
            namespace csscd = ::com::sun::star::chart::DataLabelPlacement;
            sal_Int32 nPlacement = rTypeInfo.mnDefLabelPos;
            switch( rDataLabel.monLabelPos.get( XML_TOKEN_INVALID ) )
            {
                case XML_outEnd:    nPlacement = csscd::OUTSIDE;        break;
                case XML_inEnd:     nPlacement = csscd::INSIDE;         break;
                case XML_ctr:       nPlacement = csscd::CENTER;         break;
                case XML_inBase:    nPlacement = csscd::NEAR_ORIGIN;    break;
                case XML_t:         nPlacement = csscd::TOP;            break;
                case XML_b:         nPlacement = csscd::BOTTOM;         break;
                case XML_l:         nPlacement = csscd::LEFT;           break;
                case XML_r:         nPlacement = csscd::RIGHT;          break;
                case XML_bestFit:   nPlacement = csscd::AVOID_OVERLAP;  break;
            }
            rPropSet.setProperty( PROP_LabelPlacement, nPlacement );
        }
    }
}

} // namespace

// ============================================================================

DataLabelConverter::DataLabelConverter( const ConverterRoot& rParent, DataLabelModel& rModel ) :
    ConverterBase< DataLabelModel >( rParent, rModel )
{
}

DataLabelConverter::~DataLabelConverter()
{
}

void DataLabelConverter::convertFromModel( const Reference< XDataSeries >& rxDataSeries, const TypeGroupConverter& rTypeGroup )
{
    if( rxDataSeries.is() ) try
    {
        PropertySet aPropSet( rxDataSeries->getDataPointByIndex( mrModel.mnIndex ) );
        lclConvertLabelFormatting( aPropSet, getFormatter(), mrModel, rTypeGroup, false );
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

DataLabelsConverter::DataLabelsConverter( const ConverterRoot& rParent, DataLabelsModel& rModel ) :
    ConverterBase< DataLabelsModel >( rParent, rModel )
{
}

DataLabelsConverter::~DataLabelsConverter()
{
}

void DataLabelsConverter::convertFromModel( const Reference< XDataSeries >& rxDataSeries, const TypeGroupConverter& rTypeGroup )
{
    if( !mrModel.mbDeleted )
    {
        PropertySet aPropSet( rxDataSeries );
        lclConvertLabelFormatting( aPropSet, getFormatter(), mrModel, rTypeGroup, true );
    }

    // data point label settings
    for( DataLabelsModel::DataLabelVector::iterator aIt = mrModel.maPointLabels.begin(), aEnd = mrModel.maPointLabels.end(); aIt != aEnd; ++aIt )
    {
        DataLabelConverter aLabelConv( *this, **aIt );
        aLabelConv.convertFromModel( rxDataSeries, rTypeGroup );
    }
}

// ============================================================================

ErrorBarConverter::ErrorBarConverter( const ConverterRoot& rParent, ErrorBarModel& rModel ) :
    ConverterBase< ErrorBarModel >( rParent, rModel )
{
}

ErrorBarConverter::~ErrorBarConverter()
{
}

void ErrorBarConverter::convertFromModel( const Reference< XDataSeries >& rxDataSeries )
{
    bool bShowPos = (mrModel.mnTypeId == XML_plus) || (mrModel.mnTypeId == XML_both);
    bool bShowNeg = (mrModel.mnTypeId == XML_minus) || (mrModel.mnTypeId == XML_both);
    if( bShowPos || bShowNeg ) try
    {
        Reference< XPropertySet > xErrorBar( createInstance( CREATE_OUSTRING( "com.sun.star.chart2.ErrorBar" ) ), UNO_QUERY_THROW );
        PropertySet aBarProp( xErrorBar );

        // plus/minus bars
        aBarProp.setProperty( PROP_ShowPositiveError, bShowPos );
        aBarProp.setProperty( PROP_ShowNegativeError, bShowNeg );

        // type of displayed error
        namespace cssc = ::com::sun::star::chart;
        switch( mrModel.mnValueType )
        {
            case XML_cust:
            {
                // #i87806# manual error bars
                aBarProp.setProperty( PROP_ErrorBarStyle, cssc::ErrorBarStyle::FROM_DATA );
                // attach data sequences to erorr bar
                Reference< XDataSink > xDataSink( xErrorBar, UNO_QUERY );
                if( xDataSink.is() )
                {
                    // create vector of all value sequences
                    ::std::vector< Reference< XLabeledDataSequence > > aLabeledSeqVec;
                    // add positive values
                    if( bShowPos )
                    {
                        Reference< XLabeledDataSequence > xValueSeq = createLabeledDataSequence( ErrorBarModel::PLUS );
                        if( xValueSeq.is() )
                            aLabeledSeqVec.push_back( xValueSeq );
                    }
                    // add negative values
                    if( bShowNeg )
                    {
                        Reference< XLabeledDataSequence > xValueSeq = createLabeledDataSequence( ErrorBarModel::MINUS );
                        if( xValueSeq.is() )
                            aLabeledSeqVec.push_back( xValueSeq );
                    }
                    // attach labeled data sequences to series
                    if( aLabeledSeqVec.empty() )
                        xErrorBar.clear();
                    else
                        xDataSink->setData( ContainerHelper::vectorToSequence( aLabeledSeqVec ) );
                }
            }
            break;
            case XML_fixedVal:
                aBarProp.setProperty( PROP_ErrorBarStyle, cssc::ErrorBarStyle::ABSOLUTE );
                aBarProp.setProperty( PROP_PositiveError, mrModel.mfValue );
                aBarProp.setProperty( PROP_NegativeError, mrModel.mfValue );
            break;
            case XML_percentage:
                aBarProp.setProperty( PROP_ErrorBarStyle, cssc::ErrorBarStyle::RELATIVE );
                aBarProp.setProperty( PROP_PositiveError, mrModel.mfValue );
                aBarProp.setProperty( PROP_NegativeError, mrModel.mfValue );
            break;
            case XML_stdDev:
                aBarProp.setProperty( PROP_ErrorBarStyle, cssc::ErrorBarStyle::STANDARD_DEVIATION );
                aBarProp.setProperty( PROP_Weight, mrModel.mfValue );
            break;
            case XML_stdErr:
                aBarProp.setProperty( PROP_ErrorBarStyle, cssc::ErrorBarStyle::STANDARD_ERROR );
            break;
            default:
                OSL_ENSURE( false, "ErrorBarConverter::convertFromModel - unknown error bar type" );
                xErrorBar.clear();
        }

        // error bar formatting
        getFormatter().convertFrameFormatting( aBarProp, mrModel.mxShapeProp, OBJECTTYPE_ERRORBAR );

        if( xErrorBar.is() )
        {
            PropertySet aSeriesProp( rxDataSeries );
            switch( mrModel.mnDirection )
            {
                case XML_x: aSeriesProp.setProperty( PROP_ErrorBarX, xErrorBar );   break;
                case XML_y: aSeriesProp.setProperty( PROP_ErrorBarY, xErrorBar );   break;
                default:    OSL_ENSURE( false, "ErrorBarConverter::convertFromModel - invalid error bar direction" );
            }
        }
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "ErrorBarConverter::convertFromModel - error while creating error bars" );
    }
}

// private --------------------------------------------------------------------

Reference< XLabeledDataSequence > ErrorBarConverter::createLabeledDataSequence( ErrorBarModel::SourceType eSourceType )
{
    OUString aRole;
    switch( eSourceType )
    {
        case ErrorBarModel::PLUS:
            switch( mrModel.mnDirection )
            {
                case XML_x: aRole = CREATE_OUSTRING( "error-bars-x-positive" ); break;
                case XML_y: aRole = CREATE_OUSTRING( "error-bars-y-positive" ); break;
            }
        break;
        case ErrorBarModel::MINUS:
            switch( mrModel.mnDirection )
            {
                case XML_x: aRole = CREATE_OUSTRING( "error-bars-x-negative" ); break;
                case XML_y: aRole = CREATE_OUSTRING( "error-bars-y-negative" ); break;
            }
        break;
    }
    OSL_ENSURE( aRole.getLength() > 0, "ErrorBarConverter::createLabeledDataSequence - invalid error bar direction" );
    return lclCreateLabeledDataSequence( *this, mrModel.maSources.get( eSourceType ).get(), aRole );
}

// ============================================================================

TrendlineLabelConverter::TrendlineLabelConverter( const ConverterRoot& rParent, TrendlineLabelModel& rModel ) :
    ConverterBase< TrendlineLabelModel >( rParent, rModel )
{
}

TrendlineLabelConverter::~TrendlineLabelConverter()
{
}

void TrendlineLabelConverter::convertFromModel( PropertySet& rPropSet )
{
    // formatting
    getFormatter().convertFormatting( rPropSet, mrModel.mxShapeProp, mrModel.mxTextProp, OBJECTTYPE_TRENDLINELABEL );
}

// ============================================================================

TrendlineConverter::TrendlineConverter( const ConverterRoot& rParent, TrendlineModel& rModel ) :
    ConverterBase< TrendlineModel >( rParent, rModel )
{
}

TrendlineConverter::~TrendlineConverter()
{
}

void TrendlineConverter::convertFromModel( const Reference< XDataSeries >& rxDataSeries )
{
    try
    {
        // trend line type
        OUString aServiceName;
        switch( mrModel.mnTypeId )
        {
            case XML_exp:       aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.ExponentialRegressionCurve" ); break;
            case XML_linear:    aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.LinearRegressionCurve" );      break;
            case XML_log:       aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.LogarithmicRegressionCurve" ); break;
            case XML_movingAvg: /* #i66819# moving average trendlines not supported */                              break;
            case XML_poly:      /* #i20819# polynomial trendlines not supported */                                  break;
            case XML_power:     aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.PotentialRegressionCurve" );   break;
            default:            OSL_ENSURE( false, "TrendlineConverter::convertFromModel - unknown trendline type" );
        }
        if( aServiceName.getLength() > 0 )
        {
            Reference< XRegressionCurve > xRegCurve( createInstance( aServiceName ), UNO_QUERY_THROW );
            PropertySet aPropSet( xRegCurve );

            // trendline formatting
            getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, OBJECTTYPE_TRENDLINE );

            // #i83100# show equation and correlation coefficient
            PropertySet aLabelProp( xRegCurve->getEquationProperties() );
            aLabelProp.setProperty( PROP_ShowEquation, mrModel.mbDispEquation );
            aLabelProp.setProperty( PROP_ShowCorrelationCoefficient, mrModel.mbDispRSquared );

            // #i83100# formatting of the equation text box
            if( mrModel.mbDispEquation || mrModel.mbDispRSquared )
            {
                TrendlineLabelConverter aLabelConv( *this, mrModel.mxLabel.getOrCreate() );
                aLabelConv.convertFromModel( aLabelProp );
            }

            // unsupported: #i5085# manual trendline size
            // unsupported: #i34093# manual crossing point

            Reference< XRegressionCurveContainer > xRegCurveCont( rxDataSeries, UNO_QUERY_THROW );
            xRegCurveCont->addRegressionCurve( xRegCurve );
        }
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "TrendlineConverter::convertFromModel - error while creating trendline" );
    }
}

// ============================================================================

DataPointConverter::DataPointConverter( const ConverterRoot& rParent, DataPointModel& rModel ) :
    ConverterBase< DataPointModel >( rParent, rModel )
{
}

DataPointConverter::~DataPointConverter()
{
}

void DataPointConverter::convertFromModel( const Reference< XDataSeries >& rxDataSeries,
        const TypeGroupConverter& rTypeGroup, const SeriesModel& rSeries )
{
    try
    {
        PropertySet aPropSet( rxDataSeries->getDataPointByIndex( mrModel.mnIndex ) );

        // data point marker
        if( mrModel.monMarkerSymbol.differsFrom( rSeries.mnMarkerSymbol ) || mrModel.monMarkerSize.differsFrom( rSeries.mnMarkerSize ) )
            rTypeGroup.convertMarker( aPropSet, mrModel.monMarkerSymbol.get( rSeries.mnMarkerSymbol ), mrModel.monMarkerSize.get( rSeries.mnMarkerSize ) );

        // data point pie explosion
        if( mrModel.monExplosion.differsFrom( rSeries.mnExplosion ) )
            rTypeGroup.convertPieExplosion( aPropSet, mrModel.monExplosion.get() );

        // point formatting
        if( mrModel.mxShapeProp.is() )
        {
            if( rTypeGroup.getTypeInfo().mbPictureOptions )
                getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, mrModel.mxPicOptions.getOrCreate(), rTypeGroup.getSeriesObjectType(), rSeries.mnIndex );
            else
                getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, rTypeGroup.getSeriesObjectType(), rSeries.mnIndex );
        }
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

SeriesConverter::SeriesConverter( const ConverterRoot& rParent, SeriesModel& rModel ) :
    ConverterBase< SeriesModel >( rParent, rModel )
{
}

SeriesConverter::~SeriesConverter()
{
}

Reference< XLabeledDataSequence > SeriesConverter::createCategorySequence( const OUString& rRole )
{
    return createLabeledDataSequence( SeriesModel::CATEGORIES, rRole, false );
}

Reference< XLabeledDataSequence > SeriesConverter::createValueSequence( const OUString& rRole )
{
    return createLabeledDataSequence( SeriesModel::VALUES, rRole, true );
}

Reference< XDataSeries > SeriesConverter::createDataSeries( const TypeGroupConverter& rTypeGroup, bool bVaryColorsByPoint )
{
    const TypeGroupInfo& rTypeInfo = rTypeGroup.getTypeInfo();

    // create the data series object
    Reference< XDataSeries > xDataSeries( createInstance( CREATE_OUSTRING( "com.sun.star.chart2.DataSeries" ) ), UNO_QUERY );
    PropertySet aSeriesProp( xDataSeries );

    // attach data and title sequences to series
    sal_Int32 nDataPointCount = 0;
    Reference< XDataSink > xDataSink( xDataSeries, UNO_QUERY );
    if( xDataSink.is() )
    {
        // create vector of all value sequences
        ::std::vector< Reference< XLabeledDataSequence > > aLabeledSeqVec;
        // add Y values
        Reference< XLabeledDataSequence > xYValueSeq = createValueSequence( CREATE_OUSTRING( "values-y" ) );
        if( xYValueSeq.is() )
        {
            aLabeledSeqVec.push_back( xYValueSeq );
            Reference< XDataSequence > xValues = xYValueSeq->getValues();
            if( xValues.is() )
                nDataPointCount = xValues->getData().getLength();
        }
        // add X values of scatter and bubble charts
        if( !rTypeInfo.mbCategoryAxis )
        {
            Reference< XLabeledDataSequence > xXValueSeq = createCategorySequence( CREATE_OUSTRING( "values-x" ) );
            if( xXValueSeq.is() )
                aLabeledSeqVec.push_back( xXValueSeq );
            // add size values of bubble charts
            if( rTypeInfo.meTypeId == TYPEID_BUBBLE )
            {
                Reference< XLabeledDataSequence > xSizeValueSeq = createLabeledDataSequence( SeriesModel::POINTS, CREATE_OUSTRING( "values-size" ), true );
                if( xSizeValueSeq.is() )
                    aLabeledSeqVec.push_back( xSizeValueSeq );
            }
        }
        // attach labeled data sequences to series
        if( !aLabeledSeqVec.empty() )
            xDataSink->setData( ContainerHelper::vectorToSequence( aLabeledSeqVec ) );
    }

    // error bars
    for( SeriesModel::ErrorBarVector::iterator aIt = mrModel.maErrorBars.begin(), aEnd = mrModel.maErrorBars.end(); aIt != aEnd; ++aIt )
    {
        ErrorBarConverter aErrorBarConv( *this, **aIt );
        aErrorBarConv.convertFromModel( xDataSeries );
    }

    // trendlines
    for( SeriesModel::TrendlineVector::iterator aIt = mrModel.maTrendlines.begin(), aEnd = mrModel.maTrendlines.end(); aIt != aEnd; ++aIt )
    {
        TrendlineConverter aTrendlineConv( *this, **aIt );
        aTrendlineConv.convertFromModel( xDataSeries );
    }

    // data point markers
    rTypeGroup.convertMarker( aSeriesProp, mrModel.mnMarkerSymbol, mrModel.mnMarkerSize );
#if OOX_CHART_SMOOTHED_PER_SERIES
    // #i66858# smoothed series lines
    rTypeGroup.convertLineSmooth( aSeriesProp, mrModel.mbSmooth );
#endif
    // 3D bar style (not possible to set at chart type -> set at all series)
    rTypeGroup.convertBarGeometry( aSeriesProp, mrModel.monShape.get( rTypeGroup.getModel().mnShape ) );
    // pie explosion (restricted to [0%,100%] in Chart2)
    rTypeGroup.convertPieExplosion( aSeriesProp, mrModel.mnExplosion );

    // series formatting
    ObjectFormatter& rFormatter = getFormatter();
    ObjectType eObjType = rTypeGroup.getSeriesObjectType();
    if( rTypeInfo.mbPictureOptions )
        rFormatter.convertFrameFormatting( aSeriesProp, mrModel.mxShapeProp, mrModel.mxPicOptions.getOrCreate(), eObjType, mrModel.mnIndex );
    else
        rFormatter.convertFrameFormatting( aSeriesProp, mrModel.mxShapeProp, eObjType, mrModel.mnIndex );

    // set the (unused) property default value used by the Chart2 templates (true for pie/doughnut charts)
    bool bIsPie = rTypeInfo.meTypeCategory == TYPECATEGORY_PIE;
    aSeriesProp.setProperty( PROP_VaryColorsByPoint, bIsPie );

    // own area formatting for every data point (TODO: varying line color not supported)
    // #i91271# always set area formatting for every point in pie/doughnut charts to override their automatic point formatting
    if( bIsPie || (bVaryColorsByPoint && rTypeGroup.isSeriesFrameFormat() && ObjectFormatter::isAutomaticFill( mrModel.mxShapeProp )) )
    {
        /*  Set the series point number as color cycle size at the object
            formatter to get correct start-shade/end-tint. TODO: in doughnut
            charts, the sizes of the series may vary, need to use the maximum
            point count of all series. */
        sal_Int32 nOldMax = rFormatter.getMaxSeriesIndex();
        if( bVaryColorsByPoint )
            rFormatter.setMaxSeriesIndex( nDataPointCount - 1 );
        for( sal_Int32 nIndex = 0; nIndex < nDataPointCount; ++nIndex )
        {
            try
            {
                PropertySet aPointProp( xDataSeries->getDataPointByIndex( nIndex ) );
                rFormatter.convertAutomaticFill( aPointProp, eObjType, bVaryColorsByPoint ? nIndex : mrModel.mnIndex );
            }
            catch( Exception& )
            {
            }
        }
        rFormatter.setMaxSeriesIndex( nOldMax );
    }

    // data point settings
    for( SeriesModel::DataPointVector::iterator aIt = mrModel.maPoints.begin(), aEnd = mrModel.maPoints.end(); aIt != aEnd; ++aIt )
    {
        DataPointConverter aPointConv( *this, **aIt );
        aPointConv.convertFromModel( xDataSeries, rTypeGroup, mrModel );
    }

    /*  Series data label settings. If and only if the series does not contain
        a c:dLbls element, then the c:dLbls element of the parent chart type is
        used (data label settings of the parent chart type are *not* merged
        into own existing data label settings). */
    ModelRef< DataLabelsModel > xLabels = mrModel.mxLabels.is() ? mrModel.mxLabels : rTypeGroup.getModel().mxLabels;
    if( xLabels.is() )
    {
        DataLabelsConverter aLabelsConv( *this, *xLabels );
        aLabelsConv.convertFromModel( xDataSeries, rTypeGroup );
    }

    return xDataSeries;
}

// private --------------------------------------------------------------------

Reference< XLabeledDataSequence > SeriesConverter::createLabeledDataSequence(
        SeriesModel::SourceType eSourceType, const OUString& rRole, bool bUseTextLabel )
{
    DataSourceModel* pValues = mrModel.maSources.get( eSourceType ).get();
    TextModel* pTitle = bUseTextLabel ? mrModel.mxText.get() : 0;
    return lclCreateLabeledDataSequence( *this, pValues, rRole, pTitle );
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

