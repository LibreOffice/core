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

#include <drawingml/chart/seriesconverter.hxx>

#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelFieldType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>
#include <com/sun/star/chart2/XFormattedString2.hpp>
#include <com/sun/star/chart2/FormattedString.hpp>
#include <osl/diagnose.h>
#include <basegfx/numeric/ftools.hxx>
#include <drawingml/chart/datasourceconverter.hxx>
#include <drawingml/chart/seriesmodel.hxx>
#include <drawingml/chart/titleconverter.hxx>
#include <drawingml/chart/typegroupconverter.hxx>
#include <drawingml/chart/typegroupmodel.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <drawingml/lineproperties.hxx>
#include <drawingml/textparagraph.hxx>
#include <drawingml/textrun.hxx>
#include <drawingml/textfield.hxx>
#include <drawingml/textbody.hxx>

namespace oox {
namespace drawingml {
namespace chart {

using namespace com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::uno;

namespace {

/** Function to get vertical position of label from chart height factor.
    Value can be negative, prefer top placement.
 */
int lclGetPositionY( double nVal )
{
    if( nVal <= 0.1 )
        return -1;
    else if( nVal <= 0.6 )
        return 0;
    else
        return 1;
}

/** Function to get horizontal position of label from chart width factor.
    Value can be negative, prefer center placement.
*/
int lclGetPositionX( double nVal )
{
    if( nVal <= -0.2 )
        return -1;
    else if( nVal <= 0.2 )
        return 0;
    else
        return 1;
}

Reference< XLabeledDataSequence > lclCreateLabeledDataSequence(
        const ConverterRoot& rParent,
        DataSourceModel* pValues, const OUString& rRole,
        TextModel* pTitle = nullptr )
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
        xTitleSeq = aTextConv.createDataSequence( "label" );
    }

    // create the labeled data sequence, if values or title are present
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
                                const DataLabelModelBase& rDataLabel, const TypeGroupConverter& rTypeGroup,
                                bool bDataSeriesLabel, bool bMSO2007Doc, const PropertySet* pSeriesPropSet )
{
    const TypeGroupInfo& rTypeInfo = rTypeGroup.getTypeInfo();

    /*  Excel 2007 does not change the series setting for a single data point,
        if none of some specific elements occur. But only one existing element
        in a data point will reset most other of these elements from the series
        (e.g.: series has <c:showVal>, data point has <c:showCatName>, this
        will reset <c:showVal> for this point, unless <c:showVal> is repeated
        in the data point). The elements <c:layout>, <c:numberFormat>,
        <c:spPr>, <c:tx>, and <c:txPr> are not affected at all. */
    bool bHasAnyElement = true;
    if (bMSO2007Doc)
    {
        bHasAnyElement = rDataLabel.moaSeparator.has() || rDataLabel.monLabelPos.has() ||
            rDataLabel.mobShowCatName.has() || rDataLabel.mobShowLegendKey.has() ||
            rDataLabel.mobShowPercent.has() || rDataLabel.mobShowSerName.has() ||
            rDataLabel.mobShowVal.has();
    }

    bool bShowValue   = !rDataLabel.mbDeleted && rDataLabel.mobShowVal.get( !bMSO2007Doc );
    bool bShowPercent = !rDataLabel.mbDeleted && rDataLabel.mobShowPercent.get( !bMSO2007Doc ) && (rTypeInfo.meTypeCategory == TYPECATEGORY_PIE);
    if( bShowValue &&
        !bShowPercent && rTypeInfo.meTypeCategory == TYPECATEGORY_PIE &&
        rDataLabel.maNumberFormat.maFormatCode.indexOf('%') >= 0 )
    {
        bShowValue = false;
        bShowPercent = true;
    }
    bool bShowCateg   = !rDataLabel.mbDeleted && rDataLabel.mobShowCatName.get( !bMSO2007Doc );
    bool bShowSymbol  = !rDataLabel.mbDeleted && rDataLabel.mobShowLegendKey.get( !bMSO2007Doc );

    // type of attached label
    if( bHasAnyElement || rDataLabel.mbDeleted )
    {
        DataPointLabel aPointLabel( bShowValue, bShowPercent, bShowCateg, bShowSymbol );
        rPropSet.setProperty( PROP_Label, aPointLabel );
    }

    if( !rDataLabel.mbDeleted )
    {
        // data label number format (percentage format wins over value format)
        rFormatter.convertNumberFormat( rPropSet, rDataLabel.maNumberFormat, false, bShowPercent );

        // data label text formatting (frame formatting not supported by Chart2)
        rFormatter.convertTextFormatting( rPropSet, rDataLabel.mxTextProp, OBJECTTYPE_DATALABEL );
        ObjectFormatter::convertTextRotation( rPropSet, rDataLabel.mxTextProp, false );
        ObjectFormatter::convertTextWrap( rPropSet, rDataLabel.mxTextProp );


        // data label separator (do not overwrite series separator, if no explicit point separator is present)
        if( bDataSeriesLabel || rDataLabel.moaSeparator.has() )
            rPropSet.setProperty( PROP_LabelSeparator, rDataLabel.moaSeparator.get( "; " ) );

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

            sal_Int32 nGlobalPlacement = 0;
            if ( !bDataSeriesLabel && nPlacement == rTypeInfo.mnDefLabelPos && pSeriesPropSet &&
                 pSeriesPropSet->getProperty( nGlobalPlacement, PROP_LabelPlacement ) )
                nPlacement = nGlobalPlacement;

            rPropSet.setProperty( PROP_LabelPlacement, nPlacement );
        }
    }
}

void importBorderProperties( PropertySet& rPropSet, Shape& rShape, const GraphicHelper& rGraphicHelper )
{
    LineProperties& rLP = rShape.getLineProperties();
    // no fill has the same effect as no border so skip it
    if (rLP.maLineFill.moFillType.get() == XML_noFill)
        return;

    if (rLP.moLineWidth.has())
    {
        sal_Int32 nWidth = convertEmuToHmm(rLP.moLineWidth.get());
        rPropSet.setProperty(PROP_LabelBorderWidth, uno::makeAny(nWidth));
        rPropSet.setProperty(PROP_LabelBorderStyle, uno::makeAny(drawing::LineStyle_SOLID));
    }
    const Color& aColor = rLP.maLineFill.maFillColor;
    sal_Int32 nColor = aColor.getColor(rGraphicHelper);
    rPropSet.setProperty(PROP_LabelBorderColor, uno::makeAny(nColor));
}

DataPointCustomLabelFieldType lcl_ConvertFieldNameToFieldEnum( const OUString& rField )
{
    if (rField == "VALUE")
        return DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_VALUE;
    else if (rField == "SERIESNAME")
        return DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_SERIESNAME;
    else if (rField == "CATEGORYNAME")
        return DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CATEGORYNAME;
    else if (rField == "CELLREF")
        return DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CELLREF;
    else
        return DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT;
}

} // namespace

DataLabelConverter::DataLabelConverter( const ConverterRoot& rParent, DataLabelModel& rModel ) :
    ConverterBase< DataLabelModel >( rParent, rModel )
{
}

DataLabelConverter::~DataLabelConverter()
{
}

void DataLabelConverter::convertFromModel( const Reference< XDataSeries >& rxDataSeries, const TypeGroupConverter& rTypeGroup,
                                           const PropertySet& rSeriesPropSet )
{
    if (!rxDataSeries.is())
        return;

    try
    {
        bool bMSO2007Doc = getFilter().isMSO2007Document();
        PropertySet aPropSet( rxDataSeries->getDataPointByIndex( mrModel.mnIndex ) );
        lclConvertLabelFormatting( aPropSet, getFormatter(), mrModel, rTypeGroup, false, bMSO2007Doc, &rSeriesPropSet );
        const TypeGroupInfo& rTypeInfo = rTypeGroup.getTypeInfo();
        bool bIsPie = rTypeInfo.meTypeCategory == TYPECATEGORY_PIE;
        if( mrModel.mxLayout && !mrModel.mxLayout->mbAutoLayout && !bIsPie )
        {
            // bnc#694340 - nasty hack - chart2 cannot individually
            // place data labels, let's try to find a useful
            // compromise instead
            namespace csscd = ::com::sun::star::chart::DataLabelPlacement;
            const sal_Int32 aPositionsLookupTable[] =
                {
                    csscd::TOP_LEFT,    csscd::TOP,    csscd::TOP_RIGHT,
                    csscd::LEFT,        csscd::CENTER, csscd::RIGHT,
                    csscd::BOTTOM_LEFT, csscd::BOTTOM, csscd::BOTTOM_RIGHT
                };
            const int simplifiedX = lclGetPositionX(mrModel.mxLayout->mfX);
            const int simplifiedY = lclGetPositionY(mrModel.mxLayout->mfY);
            aPropSet.setProperty( PROP_LabelPlacement,
                                  aPositionsLookupTable[ simplifiedX+1 + 3*(simplifiedY+1) ] );
        }

        if (mrModel.mxShapeProp)
            importBorderProperties(aPropSet, *mrModel.mxShapeProp, getFilter().getGraphicHelper());

        if( mrModel.mxText && mrModel.mxText->mxTextBody && mrModel.mxText->mxTextBody->getParagraphs().size() )
        {
            css::uno::Reference< XComponentContext > xContext = getComponentContext();
            uno::Sequence< css::uno::Reference< XDataPointCustomLabelField > > aSequence;

            auto& rParagraphs = mrModel.mxText->mxTextBody->getParagraphs();

            int nSequenceSize = 0;
            for( auto& pParagraph : rParagraphs )
                nSequenceSize += pParagraph->getRuns().size();

            int nParagraphs = rParagraphs.size();
            if( nParagraphs > 1 )
                nSequenceSize += nParagraphs - 1;

            aSequence.realloc( nSequenceSize );

            int nPos = 0;
            for( auto& pParagraph : rParagraphs )
            {
                for( auto& pRun : pParagraph->getRuns() )
                {
                    css::uno::Reference< XDataPointCustomLabelField > xCustomLabel = DataPointCustomLabelField::create( xContext );

                    // Store properties
                    oox::PropertySet aPropertySet( xCustomLabel );
                    pRun->getTextCharacterProperties().pushToPropSet( aPropertySet, getFilter() );

                    TextField* pField = nullptr;
                    if( ( pField = dynamic_cast< TextField* >( pRun.get() ) ) )
                    {
                        xCustomLabel->setString( pField->getText() );
                        xCustomLabel->setFieldType( lcl_ConvertFieldNameToFieldEnum( pField->getType() ) );
                        xCustomLabel->setGuid( pField->getUuid() );
                    }
                    else if( pRun.get() )
                    {
                        xCustomLabel->setString( pRun->getText() );
                        xCustomLabel->setFieldType( DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT );
                    }
                    aSequence[ nPos++ ] = xCustomLabel;
                }

                if( nParagraphs > 1 && nPos < nSequenceSize )
                {
                    css::uno::Reference< XDataPointCustomLabelField > xCustomLabel = DataPointCustomLabelField::create( xContext );
                    xCustomLabel->setFieldType( DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_NEWLINE );
                    xCustomLabel->setString("\n");
                    aSequence[ nPos++ ] = xCustomLabel;
                }
            }

            aPropSet.setProperty( PROP_CustomLabelFields, makeAny( aSequence ) );
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
    PropertySet aPropSet( rxDataSeries );
    if( !mrModel.mbDeleted )
    {
        bool bMSO2007Doc = getFilter().isMSO2007Document();
        lclConvertLabelFormatting( aPropSet, getFormatter(), mrModel, rTypeGroup, true, bMSO2007Doc, nullptr );

        if (mrModel.mxShapeProp)
            // Import baseline border properties for these data labels.
            importBorderProperties(aPropSet, *mrModel.mxShapeProp, getFilter().getGraphicHelper());
    }

    // data point label settings
    for( DataLabelsModel::DataLabelVector::iterator aIt = mrModel.maPointLabels.begin(), aEnd = mrModel.maPointLabels.end(); aIt != aEnd; ++aIt )
    {
        if ((*aIt)->maNumberFormat.maFormatCode.isEmpty())
            (*aIt)->maNumberFormat = mrModel.maNumberFormat;

        DataLabelConverter aLabelConv( *this, **aIt );
        aLabelConv.convertFromModel( rxDataSeries, rTypeGroup, aPropSet );
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
                OSL_FAIL( "ErrorBarConverter::convertFromModel - unknown error bar type" );
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
    // formatting
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
        // trend line type
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

            // Name
            aPropSet.setProperty( PROP_CurveName, mrModel.maName );
            aPropSet.setProperty( PROP_PolynomialDegree, mrModel.mnOrder );
            aPropSet.setProperty( PROP_MovingAveragePeriod, mrModel.mnPeriod );

            // Intercept
            bool hasIntercept = mrModel.mfIntercept.has();
            aPropSet.setProperty( PROP_ForceIntercept, hasIntercept);
            if (hasIntercept)
                aPropSet.setProperty( PROP_InterceptValue,  mrModel.mfIntercept.get());

            // Extrapolation
            if (mrModel.mfForward.has())
                aPropSet.setProperty( PROP_ExtrapolateForward, mrModel.mfForward.get() );
            if (mrModel.mfBackward.has())
                aPropSet.setProperty( PROP_ExtrapolateBackward, mrModel.mfBackward.get() );

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
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    try
    {
        PropertySet aPropSet( rxDataSeries->getDataPointByIndex( mrModel.mnIndex ) );

        // data point marker
        if( mrModel.monMarkerSymbol.differsFrom( rSeries.mnMarkerSymbol ) || mrModel.monMarkerSize.differsFrom( rSeries.mnMarkerSize ) )
            rTypeGroup.convertMarker( aPropSet, mrModel.monMarkerSymbol.get( rSeries.mnMarkerSymbol ),
                    mrModel.monMarkerSize.get( rSeries.mnMarkerSize ), mrModel.mxMarkerProp );

        // data point pie explosion
        if( mrModel.monExplosion.differsFrom( rSeries.mnExplosion ) )
            rTypeGroup.convertPieExplosion( aPropSet, mrModel.monExplosion.get() );

        // point formatting
        if( mrModel.mxShapeProp.is() )
        {
            if( rTypeGroup.getTypeInfo().mbPictureOptions )
                getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, mrModel.mxPicOptions.getOrCreate(bMSO2007Doc), rTypeGroup.getSeriesObjectType(), rSeries.mnIndex );
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
    return createLabeledDataSequence(SeriesModel::CATEGORIES, rRole, false);
}

Reference< XLabeledDataSequence > SeriesConverter::createValueSequence( const OUString& rRole )
{
    return createLabeledDataSequence( SeriesModel::VALUES, rRole, true );
}

Reference< XDataSeries > SeriesConverter::createDataSeries( const TypeGroupConverter& rTypeGroup, bool bVaryColorsByPoint )
{
    const TypeGroupInfo& rTypeInfo = rTypeGroup.getTypeInfo();

    // create the data series object
    Reference< XDataSeries > xDataSeries( createInstance( "com.sun.star.chart2.DataSeries" ), UNO_QUERY );
    PropertySet aSeriesProp( xDataSeries );

    // attach data and title sequences to series
    sal_Int32 nDataPointCount = 0;
    Reference< XDataSink > xDataSink( xDataSeries, UNO_QUERY );
    if( xDataSink.is() )
    {
        // create vector of all value sequences
        ::std::vector< Reference< XLabeledDataSequence > > aLabeledSeqVec;
        // add Y values
        Reference< XLabeledDataSequence > xYValueSeq = createValueSequence( "values-y" );
        if( xYValueSeq.is() )
        {
            aLabeledSeqVec.push_back( xYValueSeq );
            Reference< XDataSequence > xValues = xYValueSeq->getValues();
            if( xValues.is() )
                nDataPointCount = xValues->getData().getLength();

            if (!nDataPointCount)
                // No values present.  Don't create a data series.
                return Reference<XDataSeries>();
        }
        // add X values of scatter and bubble charts
        if( !rTypeInfo.mbCategoryAxis )
        {
            Reference< XLabeledDataSequence > xXValueSeq = createCategorySequence( "values-x" );
            if( xXValueSeq.is() )
                aLabeledSeqVec.push_back( xXValueSeq );
            // add size values of bubble charts
            if( rTypeInfo.meTypeId == TYPEID_BUBBLE )
            {
                Reference< XLabeledDataSequence > xSizeValueSeq = createLabeledDataSequence( SeriesModel::POINTS, "values-size", true );
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
    rTypeGroup.convertMarker( aSeriesProp, mrModel.mnMarkerSymbol, mrModel.mnMarkerSize, mrModel.mxMarkerProp );
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
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( rTypeInfo.mbPictureOptions )
        rFormatter.convertFrameFormatting( aSeriesProp, mrModel.mxShapeProp, mrModel.mxPicOptions.getOrCreate(bMSO2007Doc), eObjType, mrModel.mnIndex );
    else
        rFormatter.convertFrameFormatting( aSeriesProp, mrModel.mxShapeProp, eObjType, mrModel.mnIndex );

    // set the (unused) property default value used by the Chart2 templates (true for pie/doughnut charts)
    bool bIsPie = rTypeInfo.meTypeCategory == TYPECATEGORY_PIE;
    aSeriesProp.setProperty( PROP_VaryColorsByPoint, bVaryColorsByPoint );

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
        if( xLabels->maNumberFormat.maFormatCode.isEmpty() )
        {
            // Use number format code from Value series
            DataSourceModel* pValues = mrModel.maSources.get( SeriesModel::VALUES ).get();
            if( pValues )
                xLabels->maNumberFormat.maFormatCode = pValues->mxDataSeq->maFormatCode;
        }
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
    TextModel* pTitle = bUseTextLabel ? mrModel.mxText.get() : nullptr;
    return lclCreateLabeledDataSequence( *this, pValues, rRole, pTitle );
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
