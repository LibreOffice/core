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
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelFieldType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <osl/diagnose.h>
#include <drawingml/chart/datasourceconverter.hxx>
#include <drawingml/chart/seriesmodel.hxx>
#include <drawingml/chart/titleconverter.hxx>
#include <drawingml/chart/typegroupconverter.hxx>
#include <drawingml/chart/typegroupmodel.hxx>
#include <drawingml/fillproperties.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/modelobjecthelper.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <drawingml/lineproperties.hxx>
#include <drawingml/textparagraph.hxx>
#include <drawingml/textrun.hxx>
#include <drawingml/textfield.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/hatchmap.hxx>

namespace oox::drawingml::chart {

using namespace com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::uno;

namespace {

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
        xLabeledSeq = LabeledDataSequence::create(rParent.getComponentContext());
        if( xLabeledSeq.is() )
        {
            xLabeledSeq->setValues( xValueSeq );
            xLabeledSeq->setLabel( xTitleSeq );
        }
    }
    return xLabeledSeq;
}

void convertTextProperty(PropertySet& rPropSet, ObjectFormatter& rFormatter,
        DataLabelModelBase::TextBodyRef xTextProps)
{
    rFormatter.convertTextFormatting( rPropSet, xTextProps, OBJECTTYPE_DATALABEL );
    ObjectFormatter::convertTextRotation( rPropSet, xTextProps, false );
    ObjectFormatter::convertTextWrap( rPropSet, xTextProps );
}

void lclConvertLabelFormatting( PropertySet& rPropSet, ObjectFormatter& rFormatter,
                                DataLabelModelBase& rDataLabel, const TypeGroupConverter& rTypeGroup,
                                bool bDataSeriesLabel, bool bCustomLabelField, bool bHasInternalData, bool bMSO2007Doc )
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
    bool bShowCateg   = !rDataLabel.mbDeleted && rDataLabel.mobShowCatName.get( !bMSO2007Doc );
    bool bShowSymbol  = !rDataLabel.mbDeleted && rDataLabel.mobShowLegendKey.get( !bMSO2007Doc );

    // tdf#132174, tdf#136650: the inner data table has no own cell number format.
    if( bHasInternalData && bShowValue && !bShowPercent )
        rDataLabel.maNumberFormat.mbSourceLinked = false;

    // type of attached label
    if( bHasAnyElement || rDataLabel.mbDeleted )
    {
        DataPointLabel aPointLabel( bShowValue, bShowPercent, bShowCateg, bShowSymbol, bCustomLabelField );
        rPropSet.setProperty( PROP_Label, aPointLabel );
    }

    if( rDataLabel.mbDeleted )
        return;

    // data label number format (percentage format wins over value format)
    rFormatter.convertNumberFormat( rPropSet, rDataLabel.maNumberFormat, false, bShowPercent );

    // data label text formatting (frame formatting not supported by Chart2)
    if( bDataSeriesLabel || (rDataLabel.mxTextProp.is() && !rDataLabel.mxTextProp->getParagraphs().empty()) )
        convertTextProperty(rPropSet, rFormatter, rDataLabel.mxTextProp);

    // data label separator (do not overwrite series separator, if no explicit point separator is present)
    // Set the data label separator to "new line" if the value is shown as percentage with a category name,
    // just like in MS-Office. In any other case the default separator will be a semicolon.
    if( bShowPercent && !bShowValue && ( bDataSeriesLabel || rDataLabel.moaSeparator.has() ) )
        rPropSet.setProperty( PROP_LabelSeparator, rDataLabel.moaSeparator.get( "\n" ) );
    else if( bDataSeriesLabel || rDataLabel.moaSeparator.has() )
        rPropSet.setProperty( PROP_LabelSeparator, rDataLabel.moaSeparator.get( "; " ) );

    // data label placement (do not overwrite series placement, if no explicit point placement is present)
    if( !(bDataSeriesLabel || rDataLabel.monLabelPos.has()) )
        return;

    namespace csscd = ::com::sun::star::chart::DataLabelPlacement;
    sal_Int32 nPlacement = -1;
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

    if( !bDataSeriesLabel && nPlacement == -1 )
        return;

    if( nPlacement == -1 )
        nPlacement = rTypeInfo.mnDefLabelPos;

    rPropSet.setProperty( PROP_LabelPlacement, nPlacement );
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
    ::Color nColor = aColor.getColor(rGraphicHelper);
    rPropSet.setProperty(PROP_LabelBorderColor, uno::makeAny(nColor));
}

void importFillProperties( PropertySet& rPropSet, Shape& rShape, const GraphicHelper& rGraphicHelper, ModelObjectHelper& rModelObjHelper )
{
    FillProperties& rFP = rShape.getFillProperties();

    if (rFP.moFillType.has() && rFP.moFillType.get() == XML_solidFill)
    {
        rPropSet.setProperty(PROP_LabelFillStyle, drawing::FillStyle_SOLID);

        const Color& aColor = rFP.maFillColor;
        ::Color nColor = aColor.getColor(rGraphicHelper);
        rPropSet.setProperty(PROP_LabelFillColor, uno::makeAny(nColor));
    }
    else if(rFP.moFillType.has() && rFP.moFillType.get() == XML_pattFill)
    {
        rPropSet.setProperty(PROP_LabelFillStyle, drawing::FillStyle_HATCH);
        rPropSet.setProperty(PROP_LabelFillBackground, true);

        Color aHatchColor( rFP.maPatternProps.maPattFgColor );
        drawing::Hatch aHatch = createHatch(rFP.maPatternProps.moPattPreset.get(), aHatchColor.getColor(rGraphicHelper, 0));

        OUString sHatchName = rModelObjHelper.insertFillHatch(aHatch);
        rPropSet.setProperty(PROP_LabelFillHatchName, sHatchName);

        const Color& aColor = rFP.maPatternProps.maPattBgColor;
        ::Color nColor = aColor.getColor(rGraphicHelper);
        rPropSet.setProperty(PROP_LabelFillColor, uno::makeAny(nColor));
    }

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
    else if (rField == "PERCENTAGE")
        return DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_PERCENTAGE;
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

void DataLabelConverter::convertFromModel( const Reference< XDataSeries >& rxDataSeries, const TypeGroupConverter& rTypeGroup )
{
    if (!rxDataSeries.is())
        return;

    try
    {
        bool bMSO2007Doc = getFilter().isMSO2007Document();
        bool bHasInternalData = getChartDocument()->hasInternalDataProvider();
        bool bCustomLabelField = mrModel.mxText && mrModel.mxText->mxTextBody && !mrModel.mxText->mxTextBody->getParagraphs().empty();
        PropertySet aPropSet( rxDataSeries->getDataPointByIndex( mrModel.mnIndex ) );

        lclConvertLabelFormatting( aPropSet, getFormatter(), mrModel, rTypeGroup, false, bCustomLabelField, bHasInternalData, bMSO2007Doc );

        const TypeGroupInfo& rTypeInfo = rTypeGroup.getTypeInfo();
        bool bIsPie = rTypeInfo.meTypeCategory == TYPECATEGORY_PIE;

        if( mrModel.mxLayout && !mrModel.mxLayout->mbAutoLayout && !bIsPie )
        {
            RelativePosition aPos(mrModel.mxLayout->mfX, mrModel.mxLayout->mfY, css::drawing::Alignment_TOP_LEFT);
            aPropSet.setProperty(PROP_CustomLabelPosition, aPos);
        }

        if (mrModel.mxShapeProp)
        {
            importBorderProperties(aPropSet, *mrModel.mxShapeProp, getFilter().getGraphicHelper());
            uno::Reference<lang::XMultiServiceFactory> xFactory(getChartDocument(), uno::UNO_QUERY);
            ModelObjectHelper& rHelper = getFilter().getModelObjectHelperForModel(xFactory);
            importFillProperties(aPropSet, *mrModel.mxShapeProp, getFilter().getGraphicHelper(),
                                 rHelper);
        }
        if( bCustomLabelField )
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
                    convertTextProperty( aPropertySet, getFormatter(), mrModel.mxText->mxTextBody );
                    pRun->getTextCharacterProperties().pushToPropSet( aPropertySet, getFilter() );

                    TextField* pField = nullptr;
                    if( ( pField = dynamic_cast< TextField* >( pRun.get() ) ) )
                    {
                        xCustomLabel->setString( pField->getText() );
                        xCustomLabel->setFieldType( lcl_ConvertFieldNameToFieldEnum( pField->getType() ) );
                        xCustomLabel->setGuid( pField->getUuid() );
                    }
                    else if( pRun )
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
            convertTextProperty(aPropSet, getFormatter(), mrModel.mxText->mxTextBody);
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

namespace
{
/// Inherit <c:dLbl> text props (if not set) from <c:dLbls> text props (if set).
void InheritFromDataLabelsTextProps(const DataLabelsModel& rLabels, const DataLabelModel& rLabel)
{
    // See if <c:dLbls> contains text properties to inherit.
    if (!rLabels.mxTextProp.is() || rLabels.mxTextProp->getParagraphs().empty())
    {
        return;
    }

    const std::shared_ptr<TextParagraph>& rLabelsParagraph = rLabels.mxTextProp->getParagraphs()[0];

    // See if <c:dLbl> lacks text properties.
    if (rLabel.mxTextProp.is())
    {
        return;
    }

    if (!rLabel.mxText || !rLabel.mxText->mxTextBody
        || rLabel.mxText->mxTextBody->getParagraphs().empty())
    {
        return;
    }

    const std::shared_ptr<TextParagraph>& rLabelParagraph
        = rLabel.mxText->mxTextBody->getParagraphs()[0];

    // Inherit rLabel.mxText's char props from rLabels.mxTextProp's char props.
    TextCharacterProperties aCharProps;
    aCharProps.assignUsed(rLabelsParagraph->getProperties().getTextCharacterProperties());
    aCharProps.assignUsed(rLabelParagraph->getProperties().getTextCharacterProperties());
    rLabelParagraph->getProperties().getTextCharacterProperties().assignUsed(aCharProps);
}
}

void DataLabelsConverter::convertFromModel( const Reference< XDataSeries >& rxDataSeries, const TypeGroupConverter& rTypeGroup )
{
    PropertySet aPropSet( rxDataSeries );
    if( !mrModel.mbDeleted )
    {
        bool bMSO2007Doc = getFilter().isMSO2007Document();
        bool bHasInternalData = getChartDocument()->hasInternalDataProvider();

        lclConvertLabelFormatting( aPropSet, getFormatter(), mrModel, rTypeGroup, true, false, bHasInternalData, bMSO2007Doc );

        if (mrModel.mxShapeProp)
        {
            // Import baseline border properties for these data labels.
            importBorderProperties(aPropSet, *mrModel.mxShapeProp, getFilter().getGraphicHelper());
            uno::Reference<lang::XMultiServiceFactory> xFactory(getChartDocument(), uno::UNO_QUERY);
            ModelObjectHelper& rHelper = getFilter().getModelObjectHelperForModel(xFactory);
            importFillProperties(aPropSet, *mrModel.mxShapeProp, getFilter().getGraphicHelper(),
                                 rHelper);
        }
    }
    // import leaderline of data labels
    if( !mrModel.mbShowLeaderLines )
        aPropSet.setProperty( PROP_ShowCustomLeaderLines, false );

    // data point label settings
    for (auto const& pointLabel : mrModel.maPointLabels)
    {
        if (pointLabel->maNumberFormat.maFormatCode.isEmpty())
            pointLabel->maNumberFormat = mrModel.maNumberFormat;
        InheritFromDataLabelsTextProps(mrModel, *pointLabel);

        DataLabelConverter aLabelConv(*this, *pointLabel);
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
    if( !(bShowPos || bShowNeg) )
        return;

    try
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
                // attach data sequences to error bar
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
        else if (rSeries.mxShapeProp.is())
        {
            getFormatter().convertFrameFormatting( aPropSet, rSeries.mxShapeProp, rTypeGroup.getSeriesObjectType(), rSeries.mnIndex );
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
    for (auto const& errorBar : mrModel.maErrorBars)
    {
        ErrorBarConverter aErrorBarConv(*this, *errorBar);
        aErrorBarConv.convertFromModel( xDataSeries );
    }

    // trendlines
    for (auto const& trendLine : mrModel.maTrendlines)
    {
        TrendlineConverter aTrendlineConv(*this, *trendLine);
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
    for (auto const& point : mrModel.maPoints)
    {
        DataPointConverter aPointConv(*this, *point);
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

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
