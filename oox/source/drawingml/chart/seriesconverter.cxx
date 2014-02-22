/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "oox/drawingml/chart/seriesconverter.hxx"

#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>
#include <basegfx/numeric/ftools.hxx>
#include "oox/drawingml/chart/datasourceconverter.hxx"
#include "oox/drawingml/chart/seriesmodel.hxx"
#include "oox/drawingml/chart/titleconverter.hxx"
#include "oox/drawingml/chart/typegroupconverter.hxx"
#include "oox/drawingml/chart/typegroupmodel.hxx"
#include "oox/helper/containerhelper.hxx"

namespace oox {
namespace drawingml {
namespace chart {



using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::uno;



namespace {

/** nastied-up sgn function - employs some gratuity around 0 - values
   smaller than 0.33 are clamped to 0
 */
int lclSgn( double nVal )
{
    const int intVal=nVal*3;
    return intVal == 0 ? 0 : (intVal < 0 ? -1 : 1);
}

Reference< XLabeledDataSequence > lclCreateLabeledDataSequence(
        const ConverterRoot& rParent,
        DataSourceModel* pValues, const OUString& rRole,
        TextModel* pTitle = 0 )
{
    
    Reference< XDataSequence > xValueSeq;
    if( pValues )
    {
        DataSourceConverter aSourceConv( rParent, *pValues );
        xValueSeq = aSourceConv.createDataSequence( rRole );
    }

    
    Reference< XDataSequence > xTitleSeq;
    if( pTitle )
    {
        TextConverter aTextConv( rParent, *pTitle );
        xTitleSeq = aTextConv.createDataSequence( "label" );
    }

    
    Reference< XLabeledDataSequence > xLabeledSeq;
    if( xValueSeq.is() || xTitleSeq.is() )
    {
        xLabeledSeq.set( LabeledDataSequence::create(rParent.getComponentContext()), UNO_QUERY );
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
    if( bShowValue &&
        !bShowPercent && rTypeInfo.meTypeCategory == TYPECATEGORY_PIE &&
        rDataLabel.maNumberFormat.maFormatCode.indexOf('%') >= 0 )
    {
        bShowValue = false;
        bShowPercent = true;
    }
    bool bShowCateg   = !rDataLabel.mbDeleted && rDataLabel.mobShowCatName.get( false );
    bool bShowSymbol  = !rDataLabel.mbDeleted && rDataLabel.mobShowLegendKey.get( false );

    
    if( bHasAnyElement || rDataLabel.mbDeleted )
    {
        DataPointLabel aPointLabel( bShowValue, bShowPercent, bShowCateg, bShowSymbol );
        rPropSet.setProperty( PROP_Label, aPointLabel );
    }

    if( !rDataLabel.mbDeleted )
    {
        
        rFormatter.convertNumberFormat( rPropSet, rDataLabel.maNumberFormat, bShowPercent );

        
        rFormatter.convertTextFormatting( rPropSet, rDataLabel.mxTextProp, OBJECTTYPE_DATALABEL );
        rFormatter.convertTextRotation( rPropSet, rDataLabel.mxTextProp, false );

        
        if( bDataSeriesLabel || rDataLabel.moaSeparator.has() )
            rPropSet.setProperty( PROP_LabelSeparator, rDataLabel.moaSeparator.get( "; " ) );

        
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

} 



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
        const TypeGroupInfo& rTypeInfo = rTypeGroup.getTypeInfo();
        bool bIsPie = rTypeInfo.meTypeCategory == TYPECATEGORY_PIE;
        if( mrModel.mxLayout && !mrModel.mxLayout->mbAutoLayout && !bIsPie )
        {
            
            
            
            namespace csscd = ::com::sun::star::chart::DataLabelPlacement;
            const sal_Int32 aPositionsLookupTable[] =
                {
                    csscd::TOP_LEFT,    csscd::TOP,    csscd::TOP_RIGHT,
                    csscd::LEFT,        csscd::CENTER, csscd::RIGHT,
                    csscd::BOTTOM_LEFT, csscd::BOTTOM, csscd::BOTTOM_RIGHT
                };
            const double nMax=std::max(
                fabs(mrModel.mxLayout->mfX),
                fabs(mrModel.mxLayout->mfY));
            const int simplifiedX=lclSgn(mrModel.mxLayout->mfX/nMax);
            const int simplifiedY=lclSgn(mrModel.mxLayout->mfY/nMax);
            aPropSet.setProperty( PROP_LabelPlacement,
                                  aPositionsLookupTable[ simplifiedX+1 + 3*(simplifiedY+1) ] );
        }
    }
    catch( Exception& )
    {
    }
}



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

    
    for( DataLabelsModel::DataLabelVector::iterator aIt = mrModel.maPointLabels.begin(), aEnd = mrModel.maPointLabels.end(); aIt != aEnd; ++aIt )
    {
        (*aIt)->maNumberFormat.maFormatCode = mrModel.maNumberFormat.maFormatCode;
        if( !mrModel.maNumberFormat.maFormatCode.isEmpty() )
            (*aIt)->maNumberFormat.mbSourceLinked = false;

        DataLabelConverter aLabelConv( *this, **aIt );
        aLabelConv.convertFromModel( rxDataSeries, rTypeGroup );
    }
}



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
        Reference< XPropertySet > xErrorBar( createInstance( "com.sun.star.chart2.ErrorBar" ), UNO_QUERY_THROW );
        PropertySet aBarProp( xErrorBar );

        
        aBarProp.setProperty( PROP_ShowPositiveError, bShowPos );
        aBarProp.setProperty( PROP_ShowNegativeError, bShowNeg );

        
        namespace cssc = ::com::sun::star::chart;
        switch( mrModel.mnValueType )
        {
            case XML_cust:
            {
                
                aBarProp.setProperty( PROP_ErrorBarStyle, cssc::ErrorBarStyle::FROM_DATA );
                
                Reference< XDataSink > xDataSink( xErrorBar, UNO_QUERY );
                if( xDataSink.is() )
                {
                    
                    ::std::vector< Reference< XLabeledDataSequence > > aLabeledSeqVec;
                    
                    if( bShowPos )
                    {
                        Reference< XLabeledDataSequence > xValueSeq = createLabeledDataSequence( ErrorBarModel::PLUS );
                        if( xValueSeq.is() )
                            aLabeledSeqVec.push_back( xValueSeq );
                    }
                    
                    if( bShowNeg )
                    {
                        Reference< XLabeledDataSequence > xValueSeq = createLabeledDataSequence( ErrorBarModel::MINUS );
                        if( xValueSeq.is() )
                            aLabeledSeqVec.push_back( xValueSeq );
                    }
                    
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
                OSL_FAIL( "ErrorBarConverter::convertFromModel - unknown error bar type" );
                xErrorBar.clear();
        }

        
        getFormatter().convertFrameFormatting( aBarProp, mrModel.mxShapeProp, OBJECTTYPE_ERRORBAR );

        if( xErrorBar.is() )
        {
            PropertySet aSeriesProp( rxDataSeries );
            switch( mrModel.mnDirection )
            {
                case XML_x: aSeriesProp.setProperty( PROP_ErrorBarX, xErrorBar );   break;
                case XML_y: aSeriesProp.setProperty( PROP_ErrorBarY, xErrorBar );   break;
                default:    OSL_FAIL( "ErrorBarConverter::convertFromModel - invalid error bar direction" );
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "ErrorBarConverter::convertFromModel - error while creating error bars" );
    }
}



Reference< XLabeledDataSequence > ErrorBarConverter::createLabeledDataSequence( ErrorBarModel::SourceType eSourceType )
{
    OUString aRole;
    switch( eSourceType )
    {
        case ErrorBarModel::PLUS:
            switch( mrModel.mnDirection )
            {
                case XML_x: aRole = "error-bars-x-positive"; break;
                case XML_y: aRole = "error-bars-y-positive"; break;
            }
        break;
        case ErrorBarModel::MINUS:
            switch( mrModel.mnDirection )
            {
                case XML_x: aRole = "error-bars-x-negative"; break;
                case XML_y: aRole = "error-bars-y-negative"; break;
            }
        break;
    }
    OSL_ENSURE( !aRole.isEmpty(), "ErrorBarConverter::createLabeledDataSequence - invalid error bar direction" );
    return lclCreateLabeledDataSequence( *this, mrModel.maSources.get( eSourceType ).get(), aRole );
}



TrendlineLabelConverter::TrendlineLabelConverter( const ConverterRoot& rParent, TrendlineLabelModel& rModel ) :
    ConverterBase< TrendlineLabelModel >( rParent, rModel )
{
}

TrendlineLabelConverter::~TrendlineLabelConverter()
{
}

void TrendlineLabelConverter::convertFromModel( PropertySet& rPropSet )
{
    
    getFormatter().convertFormatting( rPropSet, mrModel.mxShapeProp, mrModel.mxTextProp, OBJECTTYPE_TRENDLINELABEL );
}



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
        
        OUString aServiceName;
        switch( mrModel.mnTypeId )
        {
            case XML_exp:
                aServiceName = "com.sun.star.chart2.ExponentialRegressionCurve";
            break;
            case XML_linear:
                aServiceName = "com.sun.star.chart2.LinearRegressionCurve";
            break;
            case XML_log:
                aServiceName = "com.sun.star.chart2.LogarithmicRegressionCurve";
            break;
            case XML_movingAvg:
                aServiceName = "com.sun.star.chart2.MovingAverageRegressionCurve";
            break;
            case XML_poly:
                aServiceName = "com.sun.star.chart2.PolynomialRegressionCurve";
            break;
            case XML_power:
                aServiceName = "com.sun.star.chart2.PotentialRegressionCurve";
            break;
            default:
                OSL_FAIL( "TrendlineConverter::convertFromModel - unknown trendline type" );
        }
        if( !aServiceName.isEmpty() )
        {
            Reference< XRegressionCurve > xRegCurve( createInstance( aServiceName ), UNO_QUERY_THROW );
            PropertySet aPropSet( xRegCurve );

            
            aPropSet.setProperty( PROP_CurveName, mrModel.maName );
            aPropSet.setProperty( PROP_PolynomialDegree, mrModel.mnOrder );
            aPropSet.setProperty( PROP_MovingAveragePeriod, mrModel.mnPeriod );

            
            sal_Bool hasIntercept = mrModel.mfIntercept.has();
            aPropSet.setProperty( PROP_ForceIntercept, hasIntercept);
            if (hasIntercept)
                aPropSet.setProperty( PROP_InterceptValue,  mrModel.mfIntercept.get());

            
            if (mrModel.mfForward.has())
                aPropSet.setProperty( PROP_ExtrapolateForward, mrModel.mfForward.get() );
            if (mrModel.mfBackward.has())
                aPropSet.setProperty( PROP_ExtrapolateBackward, mrModel.mfBackward.get() );

            
            getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, OBJECTTYPE_TRENDLINE );

            
            PropertySet aLabelProp( xRegCurve->getEquationProperties() );
            aLabelProp.setProperty( PROP_ShowEquation, mrModel.mbDispEquation );
            aLabelProp.setProperty( PROP_ShowCorrelationCoefficient, mrModel.mbDispRSquared );

            
            if( mrModel.mbDispEquation || mrModel.mbDispRSquared )
            {
                TrendlineLabelConverter aLabelConv( *this, mrModel.mxLabel.getOrCreate() );
                aLabelConv.convertFromModel( aLabelProp );
            }

            
            

            Reference< XRegressionCurveContainer > xRegCurveCont( rxDataSeries, UNO_QUERY_THROW );
            xRegCurveCont->addRegressionCurve( xRegCurve );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "TrendlineConverter::convertFromModel - error while creating trendline" );
    }
}



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

        
        if( mrModel.monMarkerSymbol.differsFrom( rSeries.mnMarkerSymbol ) || mrModel.monMarkerSize.differsFrom( rSeries.mnMarkerSize ) )
            rTypeGroup.convertMarker( aPropSet, mrModel.monMarkerSymbol.get( rSeries.mnMarkerSymbol ), mrModel.monMarkerSize.get( rSeries.mnMarkerSize ) );

        
        if( mrModel.monExplosion.differsFrom( rSeries.mnExplosion ) )
            rTypeGroup.convertPieExplosion( aPropSet, mrModel.monExplosion.get() );

        
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

    
    Reference< XDataSeries > xDataSeries( createInstance( "com.sun.star.chart2.DataSeries" ), UNO_QUERY );
    PropertySet aSeriesProp( xDataSeries );

    
    sal_Int32 nDataPointCount = 0;
    Reference< XDataSink > xDataSink( xDataSeries, UNO_QUERY );
    if( xDataSink.is() )
    {
        
        ::std::vector< Reference< XLabeledDataSequence > > aLabeledSeqVec;
        
        Reference< XLabeledDataSequence > xYValueSeq = createValueSequence( "values-y" );
        if( xYValueSeq.is() )
        {
            aLabeledSeqVec.push_back( xYValueSeq );
            Reference< XDataSequence > xValues = xYValueSeq->getValues();
            if( xValues.is() )
                nDataPointCount = xValues->getData().getLength();
        }
        
        if( !rTypeInfo.mbCategoryAxis )
        {
            Reference< XLabeledDataSequence > xXValueSeq = createCategorySequence( "values-x" );
            if( xXValueSeq.is() )
                aLabeledSeqVec.push_back( xXValueSeq );
            
            if( rTypeInfo.meTypeId == TYPEID_BUBBLE )
            {
                Reference< XLabeledDataSequence > xSizeValueSeq = createLabeledDataSequence( SeriesModel::POINTS, "values-size", true );
                if( xSizeValueSeq.is() )
                    aLabeledSeqVec.push_back( xSizeValueSeq );
            }
        }
        
        if( !aLabeledSeqVec.empty() )
            xDataSink->setData( ContainerHelper::vectorToSequence( aLabeledSeqVec ) );
    }

    
    for( SeriesModel::ErrorBarVector::iterator aIt = mrModel.maErrorBars.begin(), aEnd = mrModel.maErrorBars.end(); aIt != aEnd; ++aIt )
    {
        ErrorBarConverter aErrorBarConv( *this, **aIt );
        aErrorBarConv.convertFromModel( xDataSeries );
    }

    
    for( SeriesModel::TrendlineVector::iterator aIt = mrModel.maTrendlines.begin(), aEnd = mrModel.maTrendlines.end(); aIt != aEnd; ++aIt )
    {
        TrendlineConverter aTrendlineConv( *this, **aIt );
        aTrendlineConv.convertFromModel( xDataSeries );
    }

    
    rTypeGroup.convertMarker( aSeriesProp, mrModel.mnMarkerSymbol, mrModel.mnMarkerSize );
#if OOX_CHART_SMOOTHED_PER_SERIES
    
    rTypeGroup.convertLineSmooth( aSeriesProp, mrModel.mbSmooth );
#endif
    
    rTypeGroup.convertBarGeometry( aSeriesProp, mrModel.monShape.get( rTypeGroup.getModel().mnShape ) );
    
    rTypeGroup.convertPieExplosion( aSeriesProp, mrModel.mnExplosion );

    
    ObjectFormatter& rFormatter = getFormatter();
    ObjectType eObjType = rTypeGroup.getSeriesObjectType();
    if( rTypeInfo.mbPictureOptions )
        rFormatter.convertFrameFormatting( aSeriesProp, mrModel.mxShapeProp, mrModel.mxPicOptions.getOrCreate(), eObjType, mrModel.mnIndex );
    else
        rFormatter.convertFrameFormatting( aSeriesProp, mrModel.mxShapeProp, eObjType, mrModel.mnIndex );

    
    bool bIsPie = rTypeInfo.meTypeCategory == TYPECATEGORY_PIE;
    aSeriesProp.setProperty( PROP_VaryColorsByPoint, bIsPie );

    
    
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
        if( xLabels->maNumberFormat.maFormatCode.isEmpty() )
        {
            
            DataSourceModel* pValues = mrModel.maSources.get( SeriesModel::VALUES ).get();
            if( pValues )
                xLabels->maNumberFormat.maFormatCode = pValues->mxDataSeq->maFormatCode;
        }
        DataLabelsConverter aLabelsConv( *this, *xLabels );
        aLabelsConv.convertFromModel( xDataSeries, rTypeGroup );
    }

    return xDataSeries;
}



Reference< XLabeledDataSequence > SeriesConverter::createLabeledDataSequence(
        SeriesModel::SourceType eSourceType, const OUString& rRole, bool bUseTextLabel )
{
    DataSourceModel* pValues = mrModel.maSources.get( eSourceType ).get();
    TextModel* pTitle = bUseTextLabel ? mrModel.mxText.get() : 0;
    return lclCreateLabeledDataSequence( *this, pValues, rRole, pTitle );
}



} 
} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
