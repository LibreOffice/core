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

#include <memory>
#include <VSeriesPlotter.hxx>
#include <AbstractShapeFactory.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include <svl/zformat.hxx>

#include <CommonConverters.hxx>
#include <ViewDefines.hxx>
#include <ObjectIdentifier.hxx>
#include <StatisticsHelper.hxx>
#include <PlottingPositionHelper.hxx>
#include <LabelPositionHelper.hxx>
#include <ChartTypeHelper.hxx>
#include <Clipping.hxx>
#include <servicenames_charttypes.hxx>
#include <NumberFormatterWrapper.hxx>
#include <DataSeriesHelper.hxx>
#include <RegressionCurveHelper.hxx>
#include <VLegendSymbolFactory.hxx>
#include <FormattedStringHelper.hxx>
#include <ResId.hxx>
#include <strings.hrc>
#include <RelativePositionHelper.hxx>
#include <DateHelper.hxx>
#include <DiagramHelper.hxx>
#include <defines.hxx>

//only for creation: @todo remove if all plotter are uno components and instantiated via servicefactory
#include "BarChart.hxx"
#include "PieChart.hxx"
#include "AreaChart.hxx"
#include "CandleStickChart.hxx"
#include "BubbleChart.hxx"
#include "NetChart.hxx"
#include <unonames.hxx>
#include <SpecialCharacters.hxx>

#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <editeng/unoprnms.hxx>
#include <tools/color.hxx>
#include <o3tl/make_unique.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/util/XCloneable.hpp>

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>

#include <svx/unoshape.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <functional>
#include <map>
#include <unordered_map>


namespace chart {

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

VDataSeriesGroup::CachedYValues::CachedYValues()
        : m_bValuesDirty(true)
        , m_fMinimumY(0.0)
        , m_fMaximumY(0.0)
{
}

VDataSeriesGroup::VDataSeriesGroup( VDataSeries* pSeries )
        : m_aSeriesVector(1,pSeries)
        , m_bMaxPointCountDirty(true)
        , m_nMaxPointCount(0)
        , m_aListOfCachedYValues()
{
}

VDataSeriesGroup::~VDataSeriesGroup()
{
}

void VDataSeriesGroup::deleteSeries()
{
    //delete all data series help objects:
    for (VDataSeries* pSeries : m_aSeriesVector)
    {
        delete pSeries;
    }
    m_aSeriesVector.clear();
}

void VDataSeriesGroup::addSeries( VDataSeries* pSeries )
{
    m_aSeriesVector.push_back(pSeries);
    m_bMaxPointCountDirty=true;
}

sal_Int32 VDataSeriesGroup::getSeriesCount() const
{
    return m_aSeriesVector.size();
}

VSeriesPlotter::VSeriesPlotter( const uno::Reference<XChartType>& xChartTypeModel
                               , sal_Int32 nDimensionCount, bool bCategoryXAxis )
        : PlotterBase( nDimensionCount )
        , m_pMainPosHelper( nullptr )
        , m_xChartTypeModel(xChartTypeModel)
        , m_xChartTypeModelProps( uno::Reference< beans::XPropertySet >::query( xChartTypeModel ))
        , m_aZSlots()
        , m_bCategoryXAxis(bCategoryXAxis)
        , m_nTimeResolution(css::chart::TimeUnit::DAY)
        , m_aNullDate(30,12,1899)
        , m_xColorScheme()
        , m_pExplicitCategoriesProvider(nullptr)
        , m_bPointsWereSkipped(false)
{
    SAL_WARN_IF(!m_xChartTypeModel.is(),"chart2","no XChartType available in view, fallback to default values may be wrong");
}

VSeriesPlotter::~VSeriesPlotter()
{
    //delete all data series help objects:
    for (std::vector<VDataSeriesGroup>  & rGroupVector : m_aZSlots)
    {
        for (VDataSeriesGroup & rGroup : rGroupVector)
        {
            rGroup.deleteSeries();
        }
        rGroupVector.clear();
    }
    m_aZSlots.clear();

    tSecondaryPosHelperMap::iterator aPosIt = m_aSecondaryPosHelperMap.begin();
    while( aPosIt != m_aSecondaryPosHelperMap.end() )
    {
        PlottingPositionHelper* pPosHelper = aPosIt->second;
        delete pPosHelper;

        ++aPosIt;
    }
    m_aSecondaryPosHelperMap.clear();

    m_aSecondaryValueScales.clear();
}

void VSeriesPlotter::addSeries( VDataSeries* pSeries, sal_Int32 zSlot, sal_Int32 xSlot, sal_Int32 ySlot )
{
    //take ownership of pSeries

    OSL_PRECOND( pSeries, "series to add is NULL" );
    if(!pSeries)
        return;

    if(m_bCategoryXAxis)
    {
        if( m_pExplicitCategoriesProvider && m_pExplicitCategoriesProvider->isDateAxis() )
            pSeries->setXValues( m_pExplicitCategoriesProvider->getOriginalCategories() );
        else
            pSeries->setCategoryXAxis();
    }
    else
    {
        if( m_pExplicitCategoriesProvider )
            pSeries->setXValuesIfNone( m_pExplicitCategoriesProvider->getOriginalCategories() );
    }

    if(zSlot<0 || zSlot>=static_cast<sal_Int32>(m_aZSlots.size()))
    {
        //new z slot
        std::vector< VDataSeriesGroup > aZSlot;
        aZSlot.emplace_back(pSeries );
        m_aZSlots.push_back( aZSlot );
    }
    else
    {
        //existing zslot
        std::vector< VDataSeriesGroup >& rXSlots = m_aZSlots[zSlot];

        if(xSlot<0 || xSlot>=static_cast<sal_Int32>(rXSlots.size()))
        {
            //append the series to already existing x series
            rXSlots.emplace_back(pSeries );
        }
        else
        {
            //x slot is already occupied
            //y slot decides what to do:

            VDataSeriesGroup& rYSlots = rXSlots[xSlot];
            sal_Int32 nYSlotCount = rYSlots.getSeriesCount();

            if( ySlot < -1 )
            {
                //move all existing series in the xSlot to next slot
                //@todo
                OSL_FAIL( "Not implemented yet");
            }
            else if( ySlot == -1 || ySlot >= nYSlotCount)
            {
                //append the series to already existing y series
                rYSlots.addSeries(pSeries);
            }
            else
            {
                //y slot is already occupied
                //insert at given y and x position

                //@todo
                OSL_FAIL( "Not implemented yet");
            }
        }
    }
}

drawing::Direction3D VSeriesPlotter::getPreferredDiagramAspectRatio() const
{
    drawing::Direction3D aRet(1.0,1.0,1.0);
    if (!m_pPosHelper)
        return aRet;

    drawing::Direction3D aScale( m_pPosHelper->getScaledLogicWidth() );
    aRet.DirectionZ = aScale.DirectionZ*0.2;
    if(aRet.DirectionZ>1.0)
        aRet.DirectionZ=1.0;
    if(aRet.DirectionZ>10)
        aRet.DirectionZ=10;
    return aRet;
}

void VSeriesPlotter::releaseShapes()
{
    for (std::vector<VDataSeriesGroup> const & rGroupVector :  m_aZSlots)
    {
        for (VDataSeriesGroup const & rGroup : rGroupVector)
        {
            //iterate through all series in this x slot
            for (VDataSeries* pSeries : rGroup.m_aSeriesVector)
            {
                pSeries->releaseShapes();
            }
        }
    }
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getSeriesGroupShape( VDataSeries* pDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTarget )
{
    uno::Reference< drawing::XShapes > xShapes( pDataSeries->m_xGroupShape );
    if( !xShapes.is() )
    {
        //create a group shape for this series and add to logic target:
        xShapes = createGroupShape( xTarget,pDataSeries->getCID() );
        pDataSeries->m_xGroupShape = xShapes;
    }
    return xShapes;
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getSeriesGroupShapeFrontChild( VDataSeries* pDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTarget )
{
    uno::Reference< drawing::XShapes > xShapes( pDataSeries->m_xFrontSubGroupShape );
    if(!xShapes.is())
    {
        //ensure that the series group shape is already created
        uno::Reference< drawing::XShapes > xSeriesShapes( getSeriesGroupShape( pDataSeries, xTarget ) );
        //ensure that the back child is created first
        getSeriesGroupShapeBackChild( pDataSeries, xTarget );
        //use series group shape as parent for the new created front group shape
        xShapes = createGroupShape( xSeriesShapes );
        pDataSeries->m_xFrontSubGroupShape = xShapes;
    }
    return xShapes;
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getSeriesGroupShapeBackChild( VDataSeries* pDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTarget )
{
    uno::Reference< drawing::XShapes > xShapes( pDataSeries->m_xBackSubGroupShape );
    if(!xShapes.is())
    {
        //ensure that the series group shape is already created
        uno::Reference< drawing::XShapes > xSeriesShapes( getSeriesGroupShape( pDataSeries, xTarget ) );
        //use series group shape as parent for the new created back group shape
        xShapes = createGroupShape( xSeriesShapes );
        pDataSeries->m_xBackSubGroupShape = xShapes;
    }
    return xShapes;
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getLabelsGroupShape( VDataSeries& rDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTextTarget )
{
    //xTextTarget needs to be a 2D shape container always!

    uno::Reference< drawing::XShapes > xShapes( rDataSeries.m_xLabelsGroupShape );
    if(!xShapes.is())
    {
        //create a 2D group shape for texts of this series and add to text target:
        xShapes = m_pShapeFactory->createGroup2D( xTextTarget, rDataSeries.getLabelsCID() );
        rDataSeries.m_xLabelsGroupShape = xShapes;
    }
    return xShapes;
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getErrorBarsGroupShape( VDataSeries& rDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTarget
                                        , bool bYError )
{
    uno::Reference< css::drawing::XShapes > &rShapeGroup =
            bYError ? rDataSeries.m_xErrorYBarsGroupShape : rDataSeries.m_xErrorXBarsGroupShape;

    uno::Reference< drawing::XShapes > xShapes( rShapeGroup );
    if(!xShapes.is())
    {
        //create a group shape for this series and add to logic target:
        xShapes = createGroupShape( xTarget,rDataSeries.getErrorBarsCID(bYError) );
        rShapeGroup = xShapes;
    }
    return xShapes;

}

OUString VSeriesPlotter::getLabelTextForValue( VDataSeries const & rDataSeries
                , sal_Int32 nPointIndex
                , double fValue
                , bool bAsPercentage )
{
    OUString aNumber;

    if( m_apNumberFormatterWrapper.get())
    {
        sal_Int32 nNumberFormatKey = 0;
        if( rDataSeries.hasExplicitNumberFormat(nPointIndex,bAsPercentage) )
            nNumberFormatKey = rDataSeries.getExplicitNumberFormat(nPointIndex,bAsPercentage);
        else if( bAsPercentage )
        {
            sal_Int32 nPercentFormat = DiagramHelper::getPercentNumberFormat( m_apNumberFormatterWrapper->getNumberFormatsSupplier() );
            if( nPercentFormat != -1 )
                nNumberFormatKey = nPercentFormat;
        }
        else
        {
            if( rDataSeries.shouldLabelNumberFormatKeyBeDetectedFromYAxis() && m_aAxesNumberFormats.hasFormat(1,rDataSeries.getAttachedAxisIndex()) ) //y-axis
                nNumberFormatKey = m_aAxesNumberFormats.getFormat(1,rDataSeries.getAttachedAxisIndex());
            else
                nNumberFormatKey = rDataSeries.detectNumberFormatKey( nPointIndex );
        }
        if(nNumberFormatKey<0)
            nNumberFormatKey=0;

        sal_Int32 nLabelCol = 0;
        bool bColChanged;
        aNumber = m_apNumberFormatterWrapper->getFormattedString(
                nNumberFormatKey, fValue, nLabelCol, bColChanged );
        //@todo: change color of label if bColChanged is true
    }
    else
    {
        const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();
        const OUString& aNumDecimalSep = rLocaleDataWrapper.getNumDecimalSep();
        assert(aNumDecimalSep.getLength() > 0);
        sal_Unicode cDecSeparator = aNumDecimalSep[0];
        aNumber = ::rtl::math::doubleToUString( fValue, rtl_math_StringFormat_G /*rtl_math_StringFormat*/
            , 3/*DecPlaces*/ , cDecSeparator );
    }
    return aNumber;
}

uno::Reference< drawing::XShape > VSeriesPlotter::createDataLabel( const uno::Reference< drawing::XShapes >& xTarget
                    , VDataSeries& rDataSeries
                    , sal_Int32 nPointIndex
                    , double fValue
                    , double fSumValue
                    , const awt::Point& rScreenPosition2D
                    , LabelAlignment eAlignment
                    , sal_Int32 nOffset
                    , sal_Int32 nTextWidth )
{
    uno::Reference< drawing::XShape > xTextShape;
    Sequence<uno::Reference<XDataPointCustomLabelField>> aCustomLabels;

    try
    {
        const uno::Reference< css::beans::XPropertySet >& xPropertySet(
            rDataSeries.getPropertiesOfPoint( nPointIndex ) );
        if( xPropertySet.is() )
        {
            uno::Any aAny = xPropertySet->getPropertyValue( CHART_UNONAME_CUSTOM_LABEL_FIELDS );
            if( aAny.hasValue() )
            {
                aAny >>= aCustomLabels;
            }
        }

        awt::Point aScreenPosition2D(rScreenPosition2D);
        if(eAlignment==LABEL_ALIGN_LEFT)
            aScreenPosition2D.X -= nOffset;
        else if(eAlignment==LABEL_ALIGN_RIGHT)
            aScreenPosition2D.X += nOffset;
        else if(eAlignment==LABEL_ALIGN_TOP)
            aScreenPosition2D.Y -= nOffset;
        else if(eAlignment==LABEL_ALIGN_BOTTOM)
            aScreenPosition2D.Y += nOffset;

        uno::Reference< drawing::XShapes > xTarget_ =
            m_pShapeFactory->createGroup2D(
                getLabelsGroupShape(rDataSeries, xTarget),
                ObjectIdentifier::createPointCID( rDataSeries.getLabelCID_Stub(), nPointIndex));

        //check whether the label needs to be created and how:
        DataPointLabel* pLabel = rDataSeries.getDataPointLabelIfLabel( nPointIndex );

        if( !pLabel )
            return xTextShape;

        //prepare legend symbol

        // get the font size for the label through the "CharHeight" property
        // attached to the passed data series entry.
        // (By tracing font height values it results that for pie chart the
        // font size is not the same for all labels, but here no font size
        // modification occurs).
        float fViewFontSize( 10.0 );
        {
            uno::Reference< beans::XPropertySet > xProps( rDataSeries.getPropertiesOfPoint( nPointIndex ) );
            if( xProps.is() )
                xProps->getPropertyValue( "CharHeight") >>= fViewFontSize;
            // pt -> 1/100th mm
            fViewFontSize *= (2540.0f / 72.0f);
        }

        // the font height is used for computing the size of an optional legend
        // symbol to be prepended to the text label.
        Reference< drawing::XShape > xSymbol;
        if(pLabel->ShowLegendSymbol)
        {
            sal_Int32 nSymbolHeight = static_cast< sal_Int32 >( fViewFontSize * 0.6  );
            awt::Size aCurrentRatio = getPreferredLegendKeyAspectRatio();
            sal_Int32 nSymbolWidth = aCurrentRatio.Width;
            if( aCurrentRatio.Height > 0 )
            {
                nSymbolWidth = nSymbolHeight* aCurrentRatio.Width/aCurrentRatio.Height;
            }
            awt::Size aMaxSymbolExtent( nSymbolWidth, nSymbolHeight );

            if( rDataSeries.isVaryColorsByPoint() )
                xSymbol.set( VSeriesPlotter::createLegendSymbolForPoint( aMaxSymbolExtent, rDataSeries, nPointIndex, xTarget_, m_xShapeFactory ) );
            else
                xSymbol.set( VSeriesPlotter::createLegendSymbolForSeries( aMaxSymbolExtent, rDataSeries, xTarget_, m_xShapeFactory ) );
        }

        //prepare text
        bool bTextWrap = false;
        OUString aSeparator(" ");
        double fRotationDegrees = 0.0;
        try
        {
            uno::Reference< beans::XPropertySet > xPointProps( rDataSeries.getPropertiesOfPoint( nPointIndex ) );
            if(xPointProps.is())
            {
                xPointProps->getPropertyValue( "TextWordWrap" ) >>= bTextWrap;
                xPointProps->getPropertyValue( "LabelSeparator" ) >>= aSeparator;
                // Extract the optional text rotation through the
                // "TextRotation" property attached to the passed data point.
                xPointProps->getPropertyValue( "TextRotation" ) >>= fRotationDegrees;
            }
        }
        catch( const uno::Exception& e )
        {
            SAL_WARN("chart2", "Exception caught. " << e );
        }

        sal_Int32 nLineCountForSymbolsize = 0;
        sal_uInt32 nTextListLength = 3;
        sal_uInt32 nCustomLabelsCount = aCustomLabels.getLength();
        bool bUseCustomLabel = false;
        Sequence< OUString > aTextList( nTextListLength );

        bUseCustomLabel = nCustomLabelsCount > 0;
        if( bUseCustomLabel )
        {
            nTextListLength = ( nCustomLabelsCount > 3 ) ? nCustomLabelsCount : 3;
            aSeparator = "";
            aTextList = Sequence< OUString >( nTextListLength );
            for( sal_uInt32 i = 0; i < nCustomLabelsCount; ++i )
            {
                switch( aCustomLabels[i]->getFieldType() )
                {
                    case DataPointCustomLabelFieldType_VALUE:
                    {
                        aTextList[i] = getLabelTextForValue( rDataSeries, nPointIndex, fValue, false );
                        break;
                    }
                    case DataPointCustomLabelFieldType_CATEGORYNAME:
                    {
                        aTextList[i] = getCategoryName( nPointIndex );
                        break;
                    }
                    case DataPointCustomLabelFieldType_SERIESNAME:
                    {
                        OUString aRole;
                        if ( m_xChartTypeModel.is() )
                            aRole = m_xChartTypeModel->getRoleOfSequenceForSeriesLabel();
                        uno::Reference< XDataSeries > xSeries( rDataSeries.getModel() );
                        aTextList[i] = DataSeriesHelper::getDataSeriesLabel( xSeries, aRole );
                        break;
                    }
                    case DataPointCustomLabelFieldType_CELLREF:
                    {
                        // TODO: for now doesn't show placeholder
                        aTextList[i] = OUString();
                        break;
                    }
                    case DataPointCustomLabelFieldType_TEXT:
                    {
                        aTextList[i] = aCustomLabels[i]->getString();
                        break;
                    }
                    case DataPointCustomLabelFieldType_NEWLINE:
                    {
                        aTextList[i] = "\n";
                        break;
                    }
                    default:
                    break;
                }
                aCustomLabels[i]->setString( aTextList[i] );
            }
        }
        else
        {
            if( pLabel->ShowCategoryName )
            {
                aTextList[0] = getCategoryName( nPointIndex );
            }

            if( pLabel->ShowNumber )
            {
                aTextList[1] = getLabelTextForValue(rDataSeries, nPointIndex, fValue, false);
            }

            if( pLabel->ShowNumberInPercent )
            {
                if(fSumValue==0.0)
                    fSumValue=1.0;
                fValue /= fSumValue;
                if( fValue < 0 )
                    fValue*=-1.0;

                aTextList[2] = getLabelTextForValue(rDataSeries, nPointIndex, fValue, true);
            }
        }

        for( sal_Int32 nN = 0; nN < aTextList.getLength(); ++nN )
        {
            if( !aTextList[nN].isEmpty() )
            {
                ++nLineCountForSymbolsize;
            }
        }

        //prepare properties for multipropertyset-interface of shape
        tNameSequence* pPropNames;
        tAnySequence* pPropValues;
        if( !rDataSeries.getTextLabelMultiPropertyLists( nPointIndex, pPropNames, pPropValues ) )
            return xTextShape;

        // set text alignment for the text shape to be created.
        LabelPositionHelper::changeTextAdjustment( *pPropValues, *pPropNames, eAlignment );

        // check if data series entry percent value and absolute value have to
        // be appended to the text label, and what should be the separator
        // character (comma, space, new line). In case it is a new line we get
        // a multi-line label.
        bool bMultiLineLabel = ( aSeparator == "\n" );

        if( bUseCustomLabel )
        {
            Sequence< uno::Reference< XFormattedString > > aFormattedLabels( aCustomLabels.getLength() );
            for( int i = 0; i < aFormattedLabels.getLength(); i++ )
            {
                uno::Reference< XFormattedString > xString( aCustomLabels[i], uno::UNO_QUERY );
                aFormattedLabels[i] = xString;
            }

            // center the text
            sal_uInt32 nProperties = pPropNames->getLength();
            pPropNames->realloc( nProperties + 1 );
            pPropValues->realloc( nProperties + 1 );
            (*pPropNames)[ nProperties ] = UNO_NAME_EDIT_PARA_ADJUST;
            (*pPropValues)[ nProperties ] <<= style::ParagraphAdjust_CENTER;

            // create text shape
            xTextShape = AbstractShapeFactory::getOrCreateShapeFactory( m_xShapeFactory )->
                createText( xTarget_, aFormattedLabels, *pPropNames, *pPropValues,
                    AbstractShapeFactory::makeTransformation( aScreenPosition2D ) );
        }
        else if( bMultiLineLabel )
        {
            // prepare properties for each paragraph
            // we want to have the value and percent value centered respect
            // with the category name
            Sequence< tNameSequence > aParaPropNames(3);
            aParaPropNames[1].realloc(1);
            aParaPropNames[1][0] = "ParaAdjust";
            aParaPropNames[2].realloc(1);
            aParaPropNames[2][0] = "ParaAdjust";

            Sequence< tAnySequence > aParaPropValues(3);
            aParaPropValues[1].realloc(1);
            aParaPropValues[1][0] <<= style::ParagraphAdjust_CENTER;
            aParaPropValues[2].realloc(1);
            aParaPropValues[2][0] <<= style::ParagraphAdjust_CENTER;

            //create text shape
            xTextShape = AbstractShapeFactory::getOrCreateShapeFactory(m_xShapeFactory)->
                createText( xTarget_, aTextList, aParaPropNames, aParaPropValues
                            , *pPropNames, *pPropValues, AbstractShapeFactory::makeTransformation( aScreenPosition2D ) );
        }
        else
        {
            // join text list elements
            OUStringBuffer aText;
            for( sal_uInt32 nN = 0; nN < nTextListLength; ++nN)
            {
                if( !aTextList[nN].isEmpty() )
                {
                    if( !aText.isEmpty() )
                    {
                        aText.append(aSeparator);
                    }
                    aText.append( aTextList[nN] );
                }
            }

            //create text shape
            xTextShape = AbstractShapeFactory::getOrCreateShapeFactory(m_xShapeFactory)->
                createText( xTarget_, aText.makeStringAndClear()
                            , *pPropNames, *pPropValues, AbstractShapeFactory::makeTransformation( aScreenPosition2D ) );
        }

        if( !xTextShape.is() )
            return xTextShape;

        // we need to use a default value for the maximum width property ?
        if( nTextWidth == 0 && bTextWrap )
        {
            sal_Int32 nMinSize =
                    (m_aPageReferenceSize.Height < m_aPageReferenceSize.Width)
                        ? m_aPageReferenceSize.Height
                        : m_aPageReferenceSize.Width;
            nTextWidth = nMinSize / 3;
        }

        // in case text must be wrapped set the maximum width property
        // for the text shape
        if( nTextWidth != 0 && bTextWrap )
        {
            uno::Reference< beans::XPropertySet > xProp( xTextShape, uno::UNO_QUERY );
            if( xProp.is() )
            {
                // compute the height of a line of text
                if( !bMultiLineLabel || nLineCountForSymbolsize <= 0 )
                {
                    nLineCountForSymbolsize = 1;
                }
                awt::Size aTextSize = xTextShape->getSize();
                sal_Int32 aTextLineHeight =  aTextSize.Height / nLineCountForSymbolsize;

                // set maximum text width
                uno::Any aTextMaximumFrameWidth( nTextWidth );
                xProp->setPropertyValue( "TextMaximumFrameWidth", aTextMaximumFrameWidth );

                // compute the total lines of text
                aTextSize = xTextShape->getSize();
                nLineCountForSymbolsize = aTextSize.Height / aTextLineHeight;
            }
        }

        // in case text is rotated, the transformation property of the text
        // shape is modified.
        const awt::Point aUnrotatedTextPos( xTextShape->getPosition() );
        if( fRotationDegrees != 0.0 )
        {
            const double fDegreesPi( fRotationDegrees * ( F_PI / -180.0 ) );
            uno::Reference< beans::XPropertySet > xProp( xTextShape, uno::UNO_QUERY );
            if( xProp.is() )
                xProp->setPropertyValue( "Transformation", AbstractShapeFactory::makeTransformation( aScreenPosition2D, fDegreesPi ) );
            LabelPositionHelper::correctPositionForRotation( xTextShape, eAlignment, fRotationDegrees, true /*bRotateAroundCenter*/ );
        }

        // in case legend symbol has to be displayed, text shape position is
        // slightly changed.
        if( xSymbol.is() )
        {
            const awt::Point aOldTextPos( xTextShape->getPosition() );
            awt::Point aNewTextPos( aOldTextPos );

            awt::Point aSymbolPosition( aUnrotatedTextPos );
            awt::Size aSymbolSize( xSymbol->getSize() );
            awt::Size aTextSize = xTextShape->getSize();

            sal_Int32 nXDiff = aSymbolSize.Width + static_cast< sal_Int32 >( std::max( 100.0, fViewFontSize * 0.22 ) );//minimum 1mm
            if( !bMultiLineLabel || nLineCountForSymbolsize <= 0 )
                nLineCountForSymbolsize = 1;
            aSymbolPosition.Y += ((aTextSize.Height/nLineCountForSymbolsize)/4);

            if(eAlignment==LABEL_ALIGN_LEFT
                || eAlignment==LABEL_ALIGN_LEFT_TOP
                || eAlignment==LABEL_ALIGN_LEFT_BOTTOM)
            {
                aSymbolPosition.X -= nXDiff;
            }
            else if(eAlignment==LABEL_ALIGN_RIGHT
                || eAlignment==LABEL_ALIGN_RIGHT_TOP
                || eAlignment==LABEL_ALIGN_RIGHT_BOTTOM )
            {
                aNewTextPos.X += nXDiff;
            }
            else if(eAlignment==LABEL_ALIGN_TOP
                || eAlignment==LABEL_ALIGN_BOTTOM
                || eAlignment==LABEL_ALIGN_CENTER )
            {
                aSymbolPosition.X -= nXDiff/2;
                aNewTextPos.X += nXDiff/2;
            }

            xSymbol->setPosition( aSymbolPosition );
            xTextShape->setPosition( aNewTextPos );
        }
    }
    catch( const uno::Exception& e )
    {
        SAL_WARN("chart2", "Exception caught. " << e );
    }

    return xTextShape;
}

namespace
{
double lcl_getErrorBarLogicLength(
    const uno::Sequence< double > & rData,
    const uno::Reference< beans::XPropertySet >& xProp,
    sal_Int32 nErrorBarStyle,
    sal_Int32 nIndex,
    bool bPositive,
    bool bYError )
{
    double fResult;
    ::rtl::math::setNan( & fResult );
    try
    {
        switch( nErrorBarStyle )
        {
            case css::chart::ErrorBarStyle::NONE:
                break;
            case css::chart::ErrorBarStyle::VARIANCE:
                fResult = StatisticsHelper::getVariance( rData );
                break;
            case css::chart::ErrorBarStyle::STANDARD_DEVIATION:
                fResult = StatisticsHelper::getStandardDeviation( rData );
                break;
            case css::chart::ErrorBarStyle::RELATIVE:
            {
                double fPercent = 0;
                if( xProp->getPropertyValue( bPositive
                                             ? OUString("PositiveError")
                                             : OUString("NegativeError") ) >>= fPercent )
                {
                    if( nIndex >=0 && nIndex < rData.getLength() &&
                        ! ::rtl::math::isNan( rData[nIndex] ) &&
                        ! ::rtl::math::isNan( fPercent ))
                    {
                        fResult = rData[nIndex] * fPercent / 100.0;
                    }
                }
            }
            break;
            case css::chart::ErrorBarStyle::ABSOLUTE:
                xProp->getPropertyValue( bPositive
                                         ? OUString("PositiveError")
                                         : OUString("NegativeError") ) >>= fResult;
                break;
            case css::chart::ErrorBarStyle::ERROR_MARGIN:
            {
                // todo: check if this is really what's called error-margin
                double fPercent = 0;
                if( xProp->getPropertyValue( bPositive
                                             ? OUString("PositiveError")
                                             : OUString("NegativeError") ) >>= fPercent )
                {
                    double fMaxValue;
                    ::rtl::math::setInf(&fMaxValue, true);
                    const double* pValues = rData.getConstArray();
                    for(sal_Int32 i=0; i<rData.getLength(); ++i, ++pValues)
                    {
                        if(fMaxValue<*pValues)
                            fMaxValue=*pValues;
                    }
                    if( ::rtl::math::isFinite( fMaxValue ) &&
                        ::rtl::math::isFinite( fPercent ))
                    {
                        fResult = fMaxValue * fPercent / 100.0;
                    }
                }
            }
            break;
            case css::chart::ErrorBarStyle::STANDARD_ERROR:
                fResult = StatisticsHelper::getStandardError( rData );
                break;
            case css::chart::ErrorBarStyle::FROM_DATA:
            {
                uno::Reference< chart2::data::XDataSource > xErrorBarData( xProp, uno::UNO_QUERY );
                if( xErrorBarData.is())
                    fResult = StatisticsHelper::getErrorFromDataSource(
                        xErrorBarData, nIndex, bPositive, bYError);
            }
            break;
        }
    }
    catch( const uno::Exception & e )
    {
        SAL_WARN("chart2", "Exception caught. " << e );
    }

    return fResult;
}

void lcl_AddErrorBottomLine( const drawing::Position3D& rPosition, ::basegfx::B2DVector aMainDirection
                , drawing::PolyPolygonShape3D& rPoly, sal_Int32 nSequenceIndex )
{
    double fFixedWidth = 200.0;

    aMainDirection.normalize();
    ::basegfx::B2DVector aOrthoDirection(-aMainDirection.getY(),aMainDirection.getX());
    aOrthoDirection.normalize();

    ::basegfx::B2DVector aAnchor( rPosition.PositionX, rPosition.PositionY );
    ::basegfx::B2DVector aStart = aAnchor + aOrthoDirection*fFixedWidth/2.0;
    ::basegfx::B2DVector aEnd = aAnchor - aOrthoDirection*fFixedWidth/2.0;

    AddPointToPoly( rPoly, drawing::Position3D( aStart.getX(), aStart.getY(), rPosition.PositionZ), nSequenceIndex );
    AddPointToPoly( rPoly, drawing::Position3D( aEnd.getX(), aEnd.getY(), rPosition.PositionZ), nSequenceIndex );
}

::basegfx::B2DVector lcl_getErrorBarMainDirection(
              const drawing::Position3D& rStart
            , const drawing::Position3D& rBottomEnd
            , PlottingPositionHelper const * pPosHelper
            , const drawing::Position3D& rUnscaledLogicPosition
            , bool bYError )
{
    ::basegfx::B2DVector aMainDirection = ::basegfx::B2DVector( rStart.PositionX - rBottomEnd.PositionX
                                              , rStart.PositionY - rBottomEnd.PositionY );
    if( !aMainDirection.getLength() )
    {
        //get logic clip values:
        double MinX = pPosHelper->getLogicMinX();
        double MinY = pPosHelper->getLogicMinY();
        double MaxX = pPosHelper->getLogicMaxX();
        double MaxY = pPosHelper->getLogicMaxY();
        double fZ = pPosHelper->getLogicMinZ();

        if( bYError )
        {
            //main direction has constant x value
            MinX = rUnscaledLogicPosition.PositionX;
            MaxX = rUnscaledLogicPosition.PositionX;
        }
        else
        {
            //main direction has constant y value
            MinY = rUnscaledLogicPosition.PositionY;
            MaxY = rUnscaledLogicPosition.PositionY;
        }

        drawing::Position3D aStart = pPosHelper->transformLogicToScene( MinX, MinY, fZ, false );
        drawing::Position3D aEnd = pPosHelper->transformLogicToScene( MaxX, MaxY, fZ, false );

        aMainDirection = ::basegfx::B2DVector( aStart.PositionX - aEnd.PositionX
                                              , aStart.PositionY - aEnd.PositionY );
    }
    if( !aMainDirection.getLength() )
    {
        //@todo
    }
    return aMainDirection;
}

drawing::Position3D lcl_transformMixedToScene( PlottingPositionHelper const * pPosHelper
    , double fX /*scaled*/, double fY /*unscaled*/, double fZ /*unscaled*/ )
{
    if(!pPosHelper)
        return drawing::Position3D(0,0,0);
    pPosHelper->doLogicScaling( nullptr,&fY,&fZ );
    pPosHelper->clipScaledLogicValues( &fX,&fY,&fZ );
    return pPosHelper->transformScaledLogicToScene( fX, fY, fZ, false );
}

} // anonymous namespace

void VSeriesPlotter::createErrorBar(
      const uno::Reference< drawing::XShapes >& xTarget
    , const drawing::Position3D& rUnscaledLogicPosition
    , const uno::Reference< beans::XPropertySet > & xErrorBarProperties
    , const VDataSeries& rVDataSeries
    , sal_Int32 nIndex
    , bool bYError /* = true */
    , const double* pfScaledLogicX
    )
{
    if( !ChartTypeHelper::isSupportingStatisticProperties( m_xChartTypeModel, m_nDimension ) )
        return;

    if( ! xErrorBarProperties.is())
        return;

    try
    {
        bool bShowPositive = false;
        bool bShowNegative = false;
        sal_Int32 nErrorBarStyle = css::chart::ErrorBarStyle::VARIANCE;

        xErrorBarProperties->getPropertyValue( "ShowPositiveError") >>= bShowPositive;
        xErrorBarProperties->getPropertyValue( "ShowNegativeError") >>= bShowNegative;
        xErrorBarProperties->getPropertyValue( "ErrorBarStyle") >>= nErrorBarStyle;

        if(!bShowPositive && !bShowNegative)
            return;

        if(nErrorBarStyle==css::chart::ErrorBarStyle::NONE)
            return;

        if (!m_pPosHelper)
            return;

        drawing::Position3D aUnscaledLogicPosition(rUnscaledLogicPosition);
        if(nErrorBarStyle==css::chart::ErrorBarStyle::STANDARD_DEVIATION)
        {
            if (bYError)
                aUnscaledLogicPosition.PositionY = rVDataSeries.getYMeanValue();
            else
                aUnscaledLogicPosition.PositionX = rVDataSeries.getXMeanValue();
        }

        bool bCreateNegativeBorder = false;//make a vertical line at the negative end of the error bar
        bool bCreatePositiveBorder = false;//make a vertical line at the positive end of the error bar
        drawing::Position3D aMiddle(aUnscaledLogicPosition);
        const double fX = aUnscaledLogicPosition.PositionX;
        const double fY = aUnscaledLogicPosition.PositionY;
        const double fZ = aUnscaledLogicPosition.PositionZ;
        double fScaledX = fX;
        if( pfScaledLogicX )
            fScaledX = *pfScaledLogicX;
        else
            m_pPosHelper->doLogicScaling( &fScaledX, nullptr, nullptr );

        aMiddle = lcl_transformMixedToScene( m_pPosHelper, fScaledX, fY, fZ );

        drawing::Position3D aNegative(aMiddle);
        drawing::Position3D aPositive(aMiddle);

        uno::Sequence< double > aData( bYError ? rVDataSeries.getAllY() : rVDataSeries.getAllX() );

        if( bShowPositive )
        {
            double fLength = lcl_getErrorBarLogicLength( aData, xErrorBarProperties, nErrorBarStyle, nIndex, true, bYError );
            if( ::rtl::math::isFinite( fLength ) )
            {
                double fLocalX = fX;
                double fLocalY = fY;
                if( bYError )
                {
                    fLocalY+=fLength;
                    aPositive = lcl_transformMixedToScene( m_pPosHelper, fScaledX, fLocalY, fZ );
                }
                else
                {
                    fLocalX+=fLength;
                    aPositive = m_pPosHelper->transformLogicToScene( fLocalX, fLocalY, fZ, true );
                }
                bCreatePositiveBorder = m_pPosHelper->isLogicVisible(fLocalX, fLocalY, fZ);
            }
            else
                bShowPositive = false;
        }

        if( bShowNegative )
        {
            double fLength = lcl_getErrorBarLogicLength( aData, xErrorBarProperties, nErrorBarStyle, nIndex, false, bYError );
            if( ::rtl::math::isFinite( fLength ) )
            {
                double fLocalX = fX;
                double fLocalY = fY;
                if( bYError )
                {
                    fLocalY-=fLength;
                    aNegative = lcl_transformMixedToScene( m_pPosHelper, fScaledX, fLocalY, fZ );
                }
                else
                {
                    fLocalX-=fLength;
                    aNegative = m_pPosHelper->transformLogicToScene( fLocalX, fLocalY, fZ, true );
                }
                bCreateNegativeBorder = m_pPosHelper->isLogicVisible( fLocalX, fLocalY, fZ);
            }
            else
                bShowNegative = false;
        }

        if(!bShowPositive && !bShowNegative)
            return;

        drawing::PolyPolygonShape3D aPoly;

        sal_Int32 nSequenceIndex=0;
        if( bShowNegative )
            AddPointToPoly( aPoly, aNegative, nSequenceIndex );
        AddPointToPoly( aPoly, aMiddle, nSequenceIndex );
        if( bShowPositive )
            AddPointToPoly( aPoly, aPositive, nSequenceIndex );

        if( bShowNegative && bCreateNegativeBorder )
        {
            ::basegfx::B2DVector aMainDirection = lcl_getErrorBarMainDirection( aMiddle, aNegative, m_pPosHelper, aUnscaledLogicPosition, bYError );
            nSequenceIndex++;
            lcl_AddErrorBottomLine( aNegative, aMainDirection, aPoly, nSequenceIndex );
        }
        if( bShowPositive && bCreatePositiveBorder )
        {
            ::basegfx::B2DVector aMainDirection = lcl_getErrorBarMainDirection( aMiddle, aPositive, m_pPosHelper, aUnscaledLogicPosition, bYError );
            nSequenceIndex++;
            lcl_AddErrorBottomLine( aPositive, aMainDirection, aPoly, nSequenceIndex );
        }

        uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D( xTarget, PolyToPointSequence( aPoly) );
        setMappedProperties( xShape, xErrorBarProperties, PropertyMapper::getPropertyNameMapForLineProperties() );
    }
    catch( const uno::Exception & e )
    {
        SAL_WARN("chart2", "Exception caught. " << e );
    }

}

void VSeriesPlotter::createErrorBar_X( const drawing::Position3D& rUnscaledLogicPosition
                            , VDataSeries& rVDataSeries, sal_Int32 nPointIndex
                            , const uno::Reference< drawing::XShapes >& xTarget )
{
    if(m_nDimension!=2)
        return;
    // error bars
    uno::Reference< beans::XPropertySet > xErrorBarProp(rVDataSeries.getXErrorBarProperties(nPointIndex));
    if( xErrorBarProp.is())
    {
        uno::Reference< drawing::XShapes > xErrorBarsGroup_Shapes(
            getErrorBarsGroupShape(rVDataSeries, xTarget, false) );

        createErrorBar( xErrorBarsGroup_Shapes
            , rUnscaledLogicPosition, xErrorBarProp
            , rVDataSeries, nPointIndex
            , false /* bYError */
            , nullptr );
    }
}

void VSeriesPlotter::createErrorBar_Y( const drawing::Position3D& rUnscaledLogicPosition
                            , VDataSeries& rVDataSeries, sal_Int32 nPointIndex
                            , const uno::Reference< drawing::XShapes >& xTarget
                            , double const * pfScaledLogicX )
{
    if(m_nDimension!=2)
        return;
    // error bars
    uno::Reference< beans::XPropertySet > xErrorBarProp(rVDataSeries.getYErrorBarProperties(nPointIndex));
    if( xErrorBarProp.is())
    {
        uno::Reference< drawing::XShapes > xErrorBarsGroup_Shapes(
            getErrorBarsGroupShape(rVDataSeries, xTarget, true) );

        createErrorBar( xErrorBarsGroup_Shapes
            , rUnscaledLogicPosition, xErrorBarProp
            , rVDataSeries, nPointIndex
            , true /* bYError */
            , pfScaledLogicX );
    }
}

void VSeriesPlotter::createRegressionCurvesShapes( VDataSeries const & rVDataSeries,
                            const uno::Reference< drawing::XShapes >& xTarget,
                            const uno::Reference< drawing::XShapes >& xEquationTarget,
                            bool bMaySkipPoints )
{
    if(m_nDimension!=2)
        return;
    uno::Reference< XRegressionCurveContainer > xContainer( rVDataSeries.getModel(), uno::UNO_QUERY );
    if(!xContainer.is())
        return;

    if (!m_pPosHelper)
        return;

    uno::Sequence< uno::Reference< XRegressionCurve > > aCurveList = xContainer->getRegressionCurves();

    for(sal_Int32 nN=0; nN<aCurveList.getLength(); nN++)
    {
        uno::Reference< XRegressionCurveCalculator > xCalculator( aCurveList[nN]->getCalculator() );
        if( !xCalculator.is())
            continue;

        uno::Reference< beans::XPropertySet > xProperties( aCurveList[nN], uno::UNO_QUERY );

        bool bAverageLine = RegressionCurveHelper::isMeanValueLine( aCurveList[nN] );

        sal_Int32 aDegree = 2;
        sal_Int32 aPeriod = 2;
        double aExtrapolateForward = 0.0;
        double aExtrapolateBackward = 0.0;
        bool bForceIntercept = false;
        double aInterceptValue = 0.0;

        if ( xProperties.is() && !bAverageLine )
        {
            xProperties->getPropertyValue( "PolynomialDegree") >>= aDegree;
            xProperties->getPropertyValue( "MovingAveragePeriod") >>= aPeriod;
            xProperties->getPropertyValue( "ExtrapolateForward") >>= aExtrapolateForward;
            xProperties->getPropertyValue( "ExtrapolateBackward") >>= aExtrapolateBackward;
            xProperties->getPropertyValue( "ForceIntercept") >>= bForceIntercept;
            if (bForceIntercept)
                xProperties->getPropertyValue( "InterceptValue") >>= aInterceptValue;
        }

        double fChartMinX = m_pPosHelper->getLogicMinX();
        double fChartMaxX = m_pPosHelper->getLogicMaxX();

        double fMinX = fChartMinX;
        double fMaxX = fChartMaxX;

        double fPointScale = 1.0;

        if( !bAverageLine )
        {
            rVDataSeries.getMinMaxXValue(fMinX, fMaxX);
            fMaxX += aExtrapolateForward;
            fMinX -= aExtrapolateBackward;

            fPointScale = (fMaxX - fMinX) / (fChartMaxX - fChartMinX);
        }

        xCalculator->setRegressionProperties(aDegree, bForceIntercept, aInterceptValue, aPeriod);
        xCalculator->recalculateRegression( rVDataSeries.getAllX(), rVDataSeries.getAllY() );
        sal_Int32 nPointCount = 100 * fPointScale;

        if ( nPointCount < 2 )
            nPointCount = 2;

        std::vector< ExplicitScaleData > aScales( m_pPosHelper->getScales());
        uno::Reference< chart2::XScaling > xScalingX;
        uno::Reference< chart2::XScaling > xScalingY;
        if( aScales.size() >= 2 )
        {
            xScalingX.set( aScales[0].Scaling );
            xScalingY.set( aScales[1].Scaling );
        }

        uno::Sequence< geometry::RealPoint2D > aCalculatedPoints(
            xCalculator->getCurveValues(
                            fMinX, fMaxX, nPointCount,
                            xScalingX, xScalingY, bMaySkipPoints ));

        nPointCount = aCalculatedPoints.getLength();

        drawing::PolyPolygonShape3D aRegressionPoly;
        aRegressionPoly.SequenceX.realloc(1);
        aRegressionPoly.SequenceY.realloc(1);
        aRegressionPoly.SequenceZ.realloc(1);
        aRegressionPoly.SequenceX[0].realloc(nPointCount);
        aRegressionPoly.SequenceY[0].realloc(nPointCount);
        aRegressionPoly.SequenceZ[0].realloc(nPointCount);

        sal_Int32 nRealPointCount = 0;

        for(sal_Int32 nP = 0; nP < aCalculatedPoints.getLength(); ++nP)
        {
            double fLogicX = aCalculatedPoints[nP].X;
            double fLogicY = aCalculatedPoints[nP].Y;
            double fLogicZ = 0.0; //dummy

            // fdo#51656: don't scale mean value lines
            if(!bAverageLine)
                m_pPosHelper->doLogicScaling( &fLogicX, &fLogicY, &fLogicZ );

            if(!rtl::math::isNan(fLogicX) && !rtl::math::isInf(fLogicX) &&
               !rtl::math::isNan(fLogicY) && !rtl::math::isInf(fLogicY) &&
               !rtl::math::isNan(fLogicZ) && !rtl::math::isInf(fLogicZ) )
            {
                aRegressionPoly.SequenceX[0][nRealPointCount] = fLogicX;
                aRegressionPoly.SequenceY[0][nRealPointCount] = fLogicY;
                nRealPointCount++;
            }
        }
        aRegressionPoly.SequenceX[0].realloc(nRealPointCount);
        aRegressionPoly.SequenceY[0].realloc(nRealPointCount);
        aRegressionPoly.SequenceZ[0].realloc(nRealPointCount);

        drawing::PolyPolygonShape3D aClippedPoly;
        Clipping::clipPolygonAtRectangle( aRegressionPoly, m_pPosHelper->getScaledLogicClipDoubleRect(), aClippedPoly );
        aRegressionPoly = aClippedPoly;
        m_pPosHelper->transformScaledLogicToScene( aRegressionPoly );

        awt::Point aDefaultPos;
        if( aRegressionPoly.SequenceX.getLength() && aRegressionPoly.SequenceX[0].getLength() )
        {
            VLineProperties aVLineProperties;
            aVLineProperties.initFromPropertySet( xProperties );

            //create an extra group shape for each curve for selection handling
            uno::Reference< drawing::XShapes > xRegressionGroupShapes =
                createGroupShape( xTarget, rVDataSeries.getDataCurveCID( nN, bAverageLine ) );
            uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
                xRegressionGroupShapes, PolyToPointSequence( aRegressionPoly ), &aVLineProperties );
            AbstractShapeFactory::setShapeName( xShape, "MarkHandles" );
            aDefaultPos = xShape->getPosition();
        }

        // curve equation and correlation coefficient
        uno::Reference< beans::XPropertySet > xEquationProperties( aCurveList[nN]->getEquationProperties());
        if( xEquationProperties.is())
        {
            createRegressionCurveEquationShapes(
                rVDataSeries.getDataCurveEquationCID( nN ),
                xEquationProperties, xEquationTarget, xCalculator,
                aDefaultPos );
        }
    }
}

sal_Int32 lcl_getOUStringMaxLineLength ( OUStringBuffer const & aString )
{
    const sal_Int32 nStringLength = aString.getLength();
    sal_Int32 nMaxLineLength = 0;

    for ( sal_Int32 i=0; i<nStringLength; i++ )
    {
        sal_Int32 indexSep = aString.indexOf( "\n", i );
        if ( indexSep < 0 )
            indexSep = nStringLength;
        sal_Int32 nLineLength = indexSep - i;
        if ( nLineLength > nMaxLineLength )
            nMaxLineLength = nLineLength;
        i = indexSep;
    }

    return nMaxLineLength;
}

void VSeriesPlotter::createRegressionCurveEquationShapes(
    const OUString & rEquationCID,
    const uno::Reference< beans::XPropertySet > & xEquationProperties,
    const uno::Reference< drawing::XShapes >& xEquationTarget,
    const uno::Reference< chart2::XRegressionCurveCalculator > & xRegressionCurveCalculator,
    awt::Point aDefaultPos )
{
    OSL_ASSERT( xEquationProperties.is());
    if( !xEquationProperties.is())
        return;

    bool bShowEquation = false;
    bool bShowCorrCoeff = false;
    if(( xEquationProperties->getPropertyValue( "ShowEquation") >>= bShowEquation ) &&
       ( xEquationProperties->getPropertyValue( "ShowCorrelationCoefficient") >>= bShowCorrCoeff ))
    {
        if( ! (bShowEquation || bShowCorrCoeff))
            return;

        OUStringBuffer aFormula;
        sal_Int32 nNumberFormatKey = 0;
        sal_Int32 nFormulaWidth = 0;
        xEquationProperties->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormatKey;
        bool bResizeEquation = true;
        sal_Int32 nMaxIteration = 2;
        if ( bShowEquation )
        {
            OUString aXName, aYName;
            if ( !(xEquationProperties->getPropertyValue( "XName" ) >>= aXName) )
                aXName = OUString( "x" );
            if ( !(xEquationProperties->getPropertyValue( "YName" ) >>= aYName) )
                aYName = OUString( "f(x)" );
            xRegressionCurveCalculator->setXYNames( aXName, aYName );
        }

        for ( sal_Int32 nCountIteration = 0; bResizeEquation && nCountIteration < nMaxIteration ; nCountIteration++ )
        {
            bResizeEquation = false;
            if( bShowEquation )
            {
                if( m_apNumberFormatterWrapper.get())
                {   // iteration 0: default representation (no wrap)
                    // iteration 1: expected width (nFormulaWidth) is calculated
                    aFormula = xRegressionCurveCalculator->getFormattedRepresentation(
                        m_apNumberFormatterWrapper->getNumberFormatsSupplier(),
                        nNumberFormatKey, nFormulaWidth );
                    nFormulaWidth = lcl_getOUStringMaxLineLength( aFormula );
                }
                else
                {
                    aFormula = xRegressionCurveCalculator->getRepresentation();
                }

                if( bShowCorrCoeff )
                {
                    aFormula.append( "\n" );
                }
            }
            if( bShowCorrCoeff )
            {
                aFormula.append( "R" + OUStringLiteral1( aSuperscriptFigures[2] ) + " = " );
                double fR( xRegressionCurveCalculator->getCorrelationCoefficient());
                if( m_apNumberFormatterWrapper.get())
                {
                    sal_Int32 nLabelCol = 0;
                    bool bColChanged;
                    aFormula.append(
                        m_apNumberFormatterWrapper->getFormattedString(
                            nNumberFormatKey, fR*fR, nLabelCol, bColChanged ));
                    //@todo: change color of label if bColChanged is true
                }
                else
                {
                    const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();
                    const OUString& aNumDecimalSep = rLocaleDataWrapper.getNumDecimalSep();
                    sal_Unicode aDecimalSep = aNumDecimalSep[0];
                    aFormula.append( ::rtl::math::doubleToUString(
                                        fR*fR, rtl_math_StringFormat_G, 4, aDecimalSep, true ));
                }
            }

            awt::Point aScreenPosition2D;
            chart2::RelativePosition aRelativePosition;
            if( xEquationProperties->getPropertyValue( "RelativePosition") >>= aRelativePosition )
            {
                //@todo decide whether x is primary or secondary
                double fX = aRelativePosition.Primary*m_aPageReferenceSize.Width;
                double fY = aRelativePosition.Secondary*m_aPageReferenceSize.Height;
                aScreenPosition2D.X = static_cast< sal_Int32 >( ::rtl::math::round( fX ));
                aScreenPosition2D.Y = static_cast< sal_Int32 >( ::rtl::math::round( fY ));
            }
            else
                aScreenPosition2D = aDefaultPos;

            if( !aFormula.isEmpty())
            {
                // set fill and line properties on creation
                tNameSequence aNames;
                tAnySequence  aValues;
                PropertyMapper::getPreparedTextShapePropertyLists( xEquationProperties, aNames, aValues );

                uno::Reference< drawing::XShape > xTextShape = m_pShapeFactory->createText(
                    xEquationTarget, aFormula.makeStringAndClear(),
                    aNames, aValues, AbstractShapeFactory::makeTransformation( aScreenPosition2D ));

                OSL_ASSERT( xTextShape.is());
                if( xTextShape.is())
                {
                    AbstractShapeFactory::setShapeName( xTextShape, rEquationCID );
                    awt::Size aSize( xTextShape->getSize() );
                    awt::Point aPos( RelativePositionHelper::getUpperLeftCornerOfAnchoredObject(
                        aScreenPosition2D, aSize, aRelativePosition.Anchor ) );
                    //ensure that the equation is fully placed within the page (if possible)
                    if( (aPos.X + aSize.Width) > m_aPageReferenceSize.Width )
                        aPos.X = m_aPageReferenceSize.Width - aSize.Width;
                    if( aPos.X < 0 )
                    {
                        aPos.X = 0;
                        if ( nFormulaWidth > 0 )
                        {
                            bResizeEquation = true;
                            if ( nCountIteration < nMaxIteration-1 )
                                xEquationTarget->remove( xTextShape );  // remove equation
                            nFormulaWidth *= m_aPageReferenceSize.Width / static_cast< double >(aSize.Width);
                            nFormulaWidth -= nCountIteration;
                            if ( nFormulaWidth < 0 )
                                nFormulaWidth = 0;
                        }
                    }
                    if( (aPos.Y + aSize.Height) > m_aPageReferenceSize.Height )
                        aPos.Y = m_aPageReferenceSize.Height - aSize.Height;
                    if( aPos.Y < 0 )
                        aPos.Y = 0;
                    if ( !bResizeEquation || nCountIteration == nMaxIteration-1 )
                        xTextShape->setPosition(aPos);  // if equation was not removed
                }
            }
        }
    }
}

void VSeriesPlotter::setMappedProperties(
          const uno::Reference< drawing::XShape >& xTargetShape
        , const uno::Reference< beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap
        , tPropertyNameValueMap const * pOverwriteMap )
{
    uno::Reference< beans::XPropertySet > xTargetProp( xTargetShape, uno::UNO_QUERY );
    PropertyMapper::setMappedProperties(xTargetProp,xSource,rMap,pOverwriteMap);
}

void VSeriesPlotter::setTimeResolutionOnXAxis( long TimeResolution, const Date& rNullDate )
{
    m_nTimeResolution = TimeResolution;
    m_aNullDate = rNullDate;
}

// MinimumAndMaximumSupplier
long VSeriesPlotter::calculateTimeResolutionOnXAxis()
{
    long nRet = css::chart::TimeUnit::YEAR;
    if( m_pExplicitCategoriesProvider )
    {
        const std::vector< double >&  rDateCategories = m_pExplicitCategoriesProvider->getDateCategories();
        std::vector< double >::const_iterator aIt = rDateCategories.begin(), aEnd = rDateCategories.end();
        Date aNullDate(30,12,1899);
        if( m_apNumberFormatterWrapper.get() )
            aNullDate = m_apNumberFormatterWrapper->getNullDate();
        if( aIt!=aEnd )
        {
            Date aPrevious(aNullDate); aPrevious.AddDays(rtl::math::approxFloor(*aIt));
            ++aIt;
            for(;aIt!=aEnd;++aIt)
            {
                Date aCurrent(aNullDate); aCurrent.AddDays(rtl::math::approxFloor(*aIt));
                if( nRet == css::chart::TimeUnit::YEAR )
                {
                    if( DateHelper::IsInSameYear( aPrevious, aCurrent ) )
                        nRet = css::chart::TimeUnit::MONTH;
                }
                if( nRet == css::chart::TimeUnit::MONTH )
                {
                    if( DateHelper::IsInSameMonth( aPrevious, aCurrent ) )
                        nRet = css::chart::TimeUnit::DAY;
                }
                if( nRet == css::chart::TimeUnit::DAY )
                    break;
                aPrevious=aCurrent;
            }
        }
    }
    return nRet;
}
double VSeriesPlotter::getMinimumX()
{
    double fMinimum, fMaximum;
    getMinimumAndMaximiumX( fMinimum, fMaximum );
    return fMinimum;
}
double VSeriesPlotter::getMaximumX()
{
    double fMinimum, fMaximum;
    getMinimumAndMaximiumX( fMinimum, fMaximum );
    return fMaximum;
}

double VSeriesPlotter::getMinimumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex )
{
    if( !m_bCategoryXAxis || ( m_pExplicitCategoriesProvider && m_pExplicitCategoriesProvider->isDateAxis() ) )
    {
        double fMinY, fMaxY;
        getMinimumAndMaximiumYInContinuousXRange( fMinY, fMaxY, fMinimumX, fMaximumX, nAxisIndex );
        return fMinY;
    }

    double fMinimum, fMaximum;
    ::rtl::math::setInf(&fMinimum, false);
    ::rtl::math::setInf(&fMaximum, true);
    for(std::vector<VDataSeriesGroup> & rXSlots : m_aZSlots)
    {
        for(VDataSeriesGroup & rXSlot : rXSlots)
        {
            double fLocalMinimum, fLocalMaximum;
            rXSlot.calculateYMinAndMaxForCategoryRange(
                                static_cast<sal_Int32>(fMinimumX-1.0) //first category (index 0) matches with real number 1.0
                                , static_cast<sal_Int32>(fMaximumX-1.0) //first category (index 0) matches with real number 1.0
                                , isSeparateStackingForDifferentSigns( 1 )
                                , fLocalMinimum, fLocalMaximum, nAxisIndex );
            if(fMaximum<fLocalMaximum)
                fMaximum=fLocalMaximum;
            if(fMinimum>fLocalMinimum)
                fMinimum=fLocalMinimum;
        }
    }
    if(::rtl::math::isInf(fMinimum))
        ::rtl::math::setNan(&fMinimum);
    return fMinimum;
}

double VSeriesPlotter::getMaximumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex )
{
    if( !m_bCategoryXAxis || ( m_pExplicitCategoriesProvider && m_pExplicitCategoriesProvider->isDateAxis() ) )
    {
        double fMinY, fMaxY;
        getMinimumAndMaximiumYInContinuousXRange( fMinY, fMaxY, fMinimumX, fMaximumX, nAxisIndex );
        return fMaxY;
    }

    double fMinimum, fMaximum;
    ::rtl::math::setInf(&fMinimum, false);
    ::rtl::math::setInf(&fMaximum, true);
    for( std::vector< VDataSeriesGroup > & rXSlots : m_aZSlots)
    {
        for(VDataSeriesGroup & rXSlot : rXSlots)
        {
            double fLocalMinimum, fLocalMaximum;
            rXSlot.calculateYMinAndMaxForCategoryRange(
                                static_cast<sal_Int32>(fMinimumX-1.0) //first category (index 0) matches with real number 1.0
                                , static_cast<sal_Int32>(fMaximumX-1.0) //first category (index 0) matches with real number 1.0
                                , isSeparateStackingForDifferentSigns( 1 )
                                , fLocalMinimum, fLocalMaximum, nAxisIndex );
            if(fMaximum<fLocalMaximum)
                fMaximum=fLocalMaximum;
            if(fMinimum>fLocalMinimum)
                fMinimum=fLocalMinimum;
        }
    }
    if(::rtl::math::isInf(fMaximum))
        ::rtl::math::setNan(&fMaximum);
    return fMaximum;
}

double VSeriesPlotter::getMinimumZ()
{
    //this is the default for all charts without a meaningful z axis
    return 1.0;
}
double VSeriesPlotter::getMaximumZ()
{
    if( m_nDimension!=3 || !m_aZSlots.size() )
        return getMinimumZ()+1;
    return m_aZSlots.size();
}

namespace
{
    bool lcl_isValueAxis( sal_Int32 nDimensionIndex, bool bCategoryXAxis )
    {
        // default implementation: true for Y axes, and for value X axis
        if( nDimensionIndex == 0 )
            return !bCategoryXAxis;
        return nDimensionIndex == 1;
    }
}

bool VSeriesPlotter::isExpandBorderToIncrementRhythm( sal_Int32 nDimensionIndex )
{
    return lcl_isValueAxis( nDimensionIndex, m_bCategoryXAxis );
}

bool VSeriesPlotter::isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex )
{
    // do not expand axes in 3D charts
    return (m_nDimension < 3) && lcl_isValueAxis( nDimensionIndex, m_bCategoryXAxis );
}

bool VSeriesPlotter::isExpandWideValuesToZero( sal_Int32 nDimensionIndex )
{
    // default implementation: only for Y axis
    return nDimensionIndex == 1;
}

bool VSeriesPlotter::isExpandNarrowValuesTowardZero( sal_Int32 nDimensionIndex )
{
    // default implementation: only for Y axis
    return nDimensionIndex == 1;
}

bool VSeriesPlotter::isSeparateStackingForDifferentSigns( sal_Int32 nDimensionIndex )
{
    // default implementation: only for Y axis
    return nDimensionIndex == 1;
}

void VSeriesPlotter::getMinimumAndMaximiumX( double& rfMinimum, double& rfMaximum ) const
{
    ::rtl::math::setInf(&rfMinimum, false);
    ::rtl::math::setInf(&rfMaximum, true);

    std::vector< std::vector< VDataSeriesGroup > >::const_iterator       aZSlotIter = m_aZSlots.begin();
    const std::vector< std::vector< VDataSeriesGroup > >::const_iterator  aZSlotEnd = m_aZSlots.end();
    for( ; aZSlotIter != aZSlotEnd; ++aZSlotIter )
    {
        std::vector< VDataSeriesGroup >::const_iterator      aXSlotIter = aZSlotIter->begin();
        const std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = aZSlotIter->end();
        for( ; aXSlotIter != aXSlotEnd; ++aXSlotIter )
        {
            double fLocalMinimum, fLocalMaximum;
            aXSlotIter->getMinimumAndMaximiumX( fLocalMinimum, fLocalMaximum );
            if( !::rtl::math::isNan(fLocalMinimum) && fLocalMinimum< rfMinimum )
                rfMinimum = fLocalMinimum;
            if( !::rtl::math::isNan(fLocalMaximum) && fLocalMaximum> rfMaximum )
                rfMaximum = fLocalMaximum;
        }
    }
    if(::rtl::math::isInf(rfMinimum))
        ::rtl::math::setNan(&rfMinimum);
    if(::rtl::math::isInf(rfMaximum))
        ::rtl::math::setNan(&rfMaximum);
}

void VSeriesPlotter::getMinimumAndMaximiumYInContinuousXRange( double& rfMinY, double& rfMaxY, double fMinX, double fMaxX, sal_Int32 nAxisIndex ) const
{
    ::rtl::math::setInf(&rfMinY, false);
    ::rtl::math::setInf(&rfMaxY, true);

    std::vector< std::vector< VDataSeriesGroup > >::const_iterator       aZSlotIter = m_aZSlots.begin();
    const std::vector< std::vector< VDataSeriesGroup > >::const_iterator  aZSlotEnd = m_aZSlots.end();
    for( ; aZSlotIter != aZSlotEnd; ++aZSlotIter )
    {
        std::vector< VDataSeriesGroup >::const_iterator      aXSlotIter = aZSlotIter->begin();
        const std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = aZSlotIter->end();
        for( ; aXSlotIter != aXSlotEnd; ++aXSlotIter )
        {
            double fLocalMinimum, fLocalMaximum;
            aXSlotIter->getMinimumAndMaximiumYInContinuousXRange( fLocalMinimum, fLocalMaximum, fMinX, fMaxX, nAxisIndex );
            if( !::rtl::math::isNan(fLocalMinimum) && fLocalMinimum< rfMinY )
                rfMinY = fLocalMinimum;
            if( !::rtl::math::isNan(fLocalMaximum) && fLocalMaximum> rfMaxY )
                rfMaxY = fLocalMaximum;
        }
    }
    if(::rtl::math::isInf(rfMinY))
        ::rtl::math::setNan(&rfMinY);
    if(::rtl::math::isInf(rfMaxY))
        ::rtl::math::setNan(&rfMaxY);
}

sal_Int32 VSeriesPlotter::getPointCount() const
{
    sal_Int32 nRet = 0;

    std::vector< std::vector< VDataSeriesGroup > >::const_iterator       aZSlotIter = m_aZSlots.begin();
    const std::vector< std::vector< VDataSeriesGroup > >::const_iterator aZSlotEnd = m_aZSlots.end();

    for( ; aZSlotIter != aZSlotEnd; ++aZSlotIter )
    {
        std::vector< VDataSeriesGroup >::const_iterator       aXSlotIter = aZSlotIter->begin();
        const std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = aZSlotIter->end();

        for( ; aXSlotIter != aXSlotEnd; ++aXSlotIter )
        {
            sal_Int32 nPointCount = aXSlotIter->getPointCount();
            if( nPointCount>nRet )
                nRet = nPointCount;
        }
    }
    return nRet;
}

void VSeriesPlotter::setNumberFormatsSupplier(
    const uno::Reference< util::XNumberFormatsSupplier > & xNumFmtSupplier )
{
    m_apNumberFormatterWrapper.reset( new NumberFormatterWrapper( xNumFmtSupplier ));
}

void VSeriesPlotter::setColorScheme( const uno::Reference< XColorScheme >& xColorScheme )
{
    m_xColorScheme = xColorScheme;
}

void VSeriesPlotter::setExplicitCategoriesProvider( ExplicitCategoriesProvider* pExplicitCategoriesProvider )
{
    m_pExplicitCategoriesProvider = pExplicitCategoriesProvider;
}

sal_Int32 VDataSeriesGroup::getPointCount() const
{
    if(!m_bMaxPointCountDirty)
        return m_nMaxPointCount;

    sal_Int32 nRet = 0;

    for (VDataSeries* pSeries : m_aSeriesVector)
    {
        sal_Int32 nPointCount = pSeries->getTotalPointCount();
        if( nPointCount>nRet )
            nRet = nPointCount;
    }
    m_nMaxPointCount=nRet;
    m_aListOfCachedYValues.clear();
    m_aListOfCachedYValues.resize(m_nMaxPointCount);
    m_bMaxPointCountDirty=false;
    return nRet;
}

sal_Int32 VDataSeriesGroup::getAttachedAxisIndexForFirstSeries() const
{
    sal_Int32 nRet = 0;

    std::vector<VDataSeries*>::const_iterator aSeriesIter = m_aSeriesVector.begin();

    if (aSeriesIter != m_aSeriesVector.end())
        nRet = (*aSeriesIter)->getAttachedAxisIndex();

    return nRet;
}

void VDataSeriesGroup::getMinimumAndMaximiumX( double& rfMinimum, double& rfMaximum ) const
{

    ::rtl::math::setInf(&rfMinimum, false);
    ::rtl::math::setInf(&rfMaximum, true);

    for (VDataSeries* pSeries : m_aSeriesVector)
    {
        sal_Int32 nPointCount = pSeries->getTotalPointCount();
        for(sal_Int32 nN=0;nN<nPointCount;nN++)
        {
            double fX = pSeries->getXValue( nN );
            if( ::rtl::math::isNan(fX) )
                continue;
            if(rfMaximum<fX)
                rfMaximum=fX;
            if(rfMinimum>fX)
                rfMinimum=fX;
        }
    }
    if(::rtl::math::isInf(rfMinimum))
        ::rtl::math::setNan(&rfMinimum);
    if(::rtl::math::isInf(rfMaximum))
        ::rtl::math::setNan(&rfMaximum);
}

namespace {

/**
 * Keep track of minimum and maximum Y values for one or more data series.
 * When multiple data series exist, that indicates that the data series are
 * stacked.
 *
 * <p>For each X value, we calculate separate Y value ranges for each data
 * series in the first pass.  In the second pass, we calculate the minimum Y
 * value by taking the absolute minimum value of all data series, whereas
 * the maximum Y value is the sum of all the series maximum Y values.</p>
 *
 * <p>Once that's done for all X values, the final min / max Y values get
 * calculated by taking the absolute min / max Y values across all the X
 * values.</p>
 */
class PerXMinMaxCalculator
{
    typedef std::pair<double, double> MinMaxType;
    typedef std::map<size_t, MinMaxType> SeriesMinMaxType;
    typedef std::map<double, std::unique_ptr<SeriesMinMaxType>> GroupMinMaxType;
    typedef std::unordered_map<double, MinMaxType> TotalStoreType;
    GroupMinMaxType m_SeriesGroup;
    size_t mnCurSeries;

public:
    PerXMinMaxCalculator() : mnCurSeries(0) {}

    void nextSeries() { ++mnCurSeries; }

    void setValue(double fX, double fY)
    {
        SeriesMinMaxType* pStore = getByXValue(fX); // get storage for given X value.
        if (!pStore)
            // This shouldn't happen!
            return;

        SeriesMinMaxType::iterator it = pStore->lower_bound(mnCurSeries);
        if (it != pStore->end() && !pStore->key_comp()(mnCurSeries, it->first))
        {
            MinMaxType& r = it->second;
            // A min-max pair already exists for this series.  Update it.
            if (fY < r.first)
                r.first = fY;
            if (r.second < fY)
                r.second = fY;
        }
        else
        {
            // No existing pair. Insert a new one.
            pStore->insert(
                it, SeriesMinMaxType::value_type(
                    mnCurSeries, MinMaxType(fY,fY)));
        }
    }

    void getTotalRange(double& rfMin, double& rfMax) const
    {
        rtl::math::setNan(&rfMin);
        rtl::math::setNan(&rfMax);

        TotalStoreType aStore;
        getTotalStore(aStore);

        if (aStore.empty())
            return;

        TotalStoreType::const_iterator it = aStore.begin(), itEnd = aStore.end();
        rfMin = it->second.first;
        rfMax = it->second.second;
        for (++it; it != itEnd; ++it)
        {
            if (rfMin > it->second.first)
                rfMin = it->second.first;
            if (rfMax < it->second.second)
                rfMax = it->second.second;
        }
    }

private:
    /**
     * Parse all data and reduce them into a set of global Y value ranges per
     * X value.
     */
    void getTotalStore(TotalStoreType& rStore) const
    {
        TotalStoreType aStore;
        for (auto const& it : m_SeriesGroup)
        {
            double fX = it.first;

            const SeriesMinMaxType& rSeries = *it.second;
            SeriesMinMaxType::const_iterator itSeries = rSeries.begin(), itSeriesEnd = rSeries.end();
            for (; itSeries != itSeriesEnd; ++itSeries)
            {
                double fYMin = itSeries->second.first, fYMax = itSeries->second.second;
                TotalStoreType::iterator itr = aStore.find(fX);
                if (itr == aStore.end())
                    // New min-max pair for give X value.
                    aStore.emplace(fX, std::pair<double,double>(fYMin,fYMax));
                else
                {
                    MinMaxType& r = itr->second;
                    if (fYMin < r.first)
                        r.first = fYMin; // min y-value

                    r.second += fYMax; // accumulative max y-value.
                }
            }
        }
        rStore.swap(aStore);
    }

    SeriesMinMaxType* getByXValue(double fX)
    {
        GroupMinMaxType::iterator it = m_SeriesGroup.find(fX);
        if (it == m_SeriesGroup.end())
        {
            std::pair<GroupMinMaxType::iterator,bool> r =
                m_SeriesGroup.insert(std::make_pair(fX, o3tl::make_unique<SeriesMinMaxType>()));

            if (!r.second)
                // insertion failed.
                return nullptr;

            it = r.first;
        }

        return it->second.get();
    }
};

}

void VDataSeriesGroup::getMinimumAndMaximiumYInContinuousXRange(
    double& rfMinY, double& rfMaxY, double fMinX, double fMaxX, sal_Int32 nAxisIndex ) const
{
    ::rtl::math::setNan(&rfMinY);
    ::rtl::math::setNan(&rfMaxY);

    if (m_aSeriesVector.empty())
        // No data series.  Bail out.
        return;

    PerXMinMaxCalculator aRangeCalc;
    for (const VDataSeries* pSeries : m_aSeriesVector)
    {
        if (!pSeries)
            continue;

        for (sal_Int32 i = 0, n = pSeries->getTotalPointCount(); i < n; ++i)
        {
            if (nAxisIndex != pSeries->getAttachedAxisIndex())
                continue;

            double fX = pSeries->getXValue(i);
            if (rtl::math::isNan(fX))
                continue;

            if (fX < fMinX || fX > fMaxX)
                // Outside specified X range.  Skip it.
                continue;

            double fY = pSeries->getYValue(i);
            if (::rtl::math::isNan(fY))
                continue;

            aRangeCalc.setValue(fX, fY);
        }
        aRangeCalc.nextSeries();
    }

    aRangeCalc.getTotalRange(rfMinY, rfMaxY);
}

void VDataSeriesGroup::calculateYMinAndMaxForCategory( sal_Int32 nCategoryIndex
        , bool bSeparateStackingForDifferentSigns
        , double& rfMinimumY, double& rfMaximumY, sal_Int32 nAxisIndex )
{
    assert(nCategoryIndex >= 0);
    assert(nCategoryIndex < getPointCount());

    ::rtl::math::setInf(&rfMinimumY, false);
    ::rtl::math::setInf(&rfMaximumY, true);

    if(m_aSeriesVector.empty())
        return;

    CachedYValues aCachedYValues = m_aListOfCachedYValues[nCategoryIndex][nAxisIndex];
    if( !aCachedYValues.m_bValuesDirty )
    {
        //return cached values
        rfMinimumY = aCachedYValues.m_fMinimumY;
        rfMaximumY = aCachedYValues.m_fMaximumY;
        return;
    }

    double fTotalSum, fPositiveSum, fNegativeSum, fFirstPositiveY, fFirstNegativeY;
    ::rtl::math::setNan( &fTotalSum );
    ::rtl::math::setNan( &fPositiveSum );
    ::rtl::math::setNan( &fNegativeSum );
    ::rtl::math::setNan( &fFirstPositiveY );
    ::rtl::math::setNan( &fFirstNegativeY );

    if( bSeparateStackingForDifferentSigns )
    {
        for (const VDataSeries* pSeries: m_aSeriesVector)
        {
            if( nAxisIndex != pSeries->getAttachedAxisIndex() )
                continue;

            double fValueMinY = pSeries->getMinimumofAllDifferentYValues( nCategoryIndex );
            double fValueMaxY = pSeries->getMaximumofAllDifferentYValues( nCategoryIndex );

            if( fValueMaxY >= 0 )
            {
                if( ::rtl::math::isNan( fPositiveSum ) )
                    fPositiveSum = fFirstPositiveY = fValueMaxY;
                else
                    fPositiveSum += fValueMaxY;
            }
            if( fValueMinY < 0 )
            {
                if(::rtl::math::isNan( fNegativeSum ))
                    fNegativeSum = fFirstNegativeY = fValueMinY;
                else
                    fNegativeSum += fValueMinY;
            }
        }
        rfMinimumY = ::rtl::math::isNan( fNegativeSum ) ? fFirstPositiveY : fNegativeSum;
        rfMaximumY = ::rtl::math::isNan( fPositiveSum ) ? fFirstNegativeY : fPositiveSum;
    }
    else
    {
        for (const VDataSeries* pSeries: m_aSeriesVector)
        {
            if( nAxisIndex != pSeries->getAttachedAxisIndex() )
                continue;

            double fValueMinY = pSeries->getMinimumofAllDifferentYValues( nCategoryIndex );
            double fValueMaxY = pSeries->getMaximumofAllDifferentYValues( nCategoryIndex );

            if( ::rtl::math::isNan( fTotalSum ) )
            {
                rfMinimumY = fValueMinY;
                rfMaximumY = fTotalSum = fValueMaxY;
            }
            else
            {
                fTotalSum += fValueMaxY;
                if( rfMinimumY > fTotalSum )
                    rfMinimumY = fTotalSum;
                if( rfMaximumY < fTotalSum )
                    rfMaximumY = fTotalSum;
            }
        }
    }

    aCachedYValues.m_fMinimumY = rfMinimumY;
    aCachedYValues.m_fMaximumY = rfMaximumY;
    aCachedYValues.m_bValuesDirty = false;
    m_aListOfCachedYValues[nCategoryIndex][nAxisIndex]=aCachedYValues;
}

void VDataSeriesGroup::calculateYMinAndMaxForCategoryRange(
        sal_Int32 nStartCategoryIndex, sal_Int32 nEndCategoryIndex
        , bool bSeparateStackingForDifferentSigns
        , double& rfMinimumY, double& rfMaximumY, sal_Int32 nAxisIndex )
{
    //@todo maybe cache these values
    ::rtl::math::setInf(&rfMinimumY, false);
    ::rtl::math::setInf(&rfMaximumY, true);

    //iterate through the given categories
    if(nStartCategoryIndex<0)
        nStartCategoryIndex=0;
    const sal_Int32 nPointCount = getPointCount();//necessary to create m_aListOfCachedYValues
    if(nPointCount <= 0)
        return;
    if (nEndCategoryIndex >= nPointCount)
        nEndCategoryIndex = nPointCount - 1;
    if(nEndCategoryIndex<0)
        nEndCategoryIndex=0;
    for( sal_Int32 nCatIndex = nStartCategoryIndex; nCatIndex <= nEndCategoryIndex; nCatIndex++ )
    {
        double fMinimumY; ::rtl::math::setNan(&fMinimumY);
        double fMaximumY; ::rtl::math::setNan(&fMaximumY);

        calculateYMinAndMaxForCategory( nCatIndex
            , bSeparateStackingForDifferentSigns, fMinimumY, fMaximumY, nAxisIndex );

        if(rfMinimumY > fMinimumY)
            rfMinimumY = fMinimumY;
        if(rfMaximumY < fMaximumY)
            rfMaximumY = fMaximumY;
    }
}

double VSeriesPlotter::getTransformedDepth() const
{
    double MinZ = m_pMainPosHelper->getLogicMinZ();
    double MaxZ = m_pMainPosHelper->getLogicMaxZ();
    m_pMainPosHelper->doLogicScaling( nullptr, nullptr, &MinZ );
    m_pMainPosHelper->doLogicScaling( nullptr, nullptr, &MaxZ );
    return FIXED_SIZE_FOR_3D_CHART_VOLUME/(MaxZ-MinZ);
}

void VSeriesPlotter::addSecondaryValueScale( const ExplicitScaleData& rScale, sal_Int32 nAxisIndex )
{
    if( nAxisIndex<1 )
        return;

    m_aSecondaryValueScales[nAxisIndex]=rScale;
}

PlottingPositionHelper& VSeriesPlotter::getPlottingPositionHelper( sal_Int32 nAxisIndex ) const
{
    PlottingPositionHelper* pRet = nullptr;
    if(nAxisIndex>0)
    {
        tSecondaryPosHelperMap::const_iterator aPosIt = m_aSecondaryPosHelperMap.find( nAxisIndex );
        if( aPosIt != m_aSecondaryPosHelperMap.end() )
        {
            pRet = aPosIt->second;
        }
        else if (m_pPosHelper)
        {
            tSecondaryValueScales::const_iterator aScaleIt = m_aSecondaryValueScales.find( nAxisIndex );
            if( aScaleIt != m_aSecondaryValueScales.end() )
            {
                pRet = m_pPosHelper->createSecondaryPosHelper( aScaleIt->second );
                m_aSecondaryPosHelperMap[nAxisIndex] = pRet;
            }
        }
    }
    if( !pRet )
        pRet = m_pMainPosHelper;
    if(pRet)
        pRet->setTimeResolution( m_nTimeResolution, m_aNullDate );
    return *pRet;
}

void VSeriesPlotter::rearrangeLabelToAvoidOverlapIfRequested( const awt::Size& /*rPageSize*/ )
{
}

VDataSeries* VSeriesPlotter::getFirstSeries() const
{
    for (std::vector<VDataSeriesGroup> const & rGroup : m_aZSlots)
    {
        std::vector<VDataSeriesGroup>::const_iterator       aXSlotIter = rGroup.begin();
        const std::vector<VDataSeriesGroup>::const_iterator aXSlotEnd  = rGroup.end();

        if (aXSlotIter != aXSlotEnd)
        {
            VDataSeriesGroup aSeriesGroup(*aXSlotIter);
            if (aSeriesGroup.m_aSeriesVector.size())
            {
                VDataSeries* pSeries = aSeriesGroup.m_aSeriesVector[0];
                if (pSeries)
                    return pSeries;
            }
        }
    }
    return nullptr;
}

OUString VSeriesPlotter::getCategoryName( sal_Int32 nPointIndex ) const
{
    if (m_pExplicitCategoriesProvider)
    {
        Sequence< OUString > aCategories(m_pExplicitCategoriesProvider->getSimpleCategories());
        if (nPointIndex >= 0 && nPointIndex < aCategories.getLength())
        {
            return aCategories[nPointIndex];
        }
    }
    return OUString();
}

uno::Sequence< OUString > VSeriesPlotter::getSeriesNames() const
{
    std::vector<OUString> aRetVector;

    OUString aRole;
    if( m_xChartTypeModel.is() )
        aRole = m_xChartTypeModel->getRoleOfSequenceForSeriesLabel();

    for (std::vector<VDataSeriesGroup> const & rGroup : m_aZSlots)
    {
        std::vector<VDataSeriesGroup>::const_iterator       aXSlotIter = rGroup.begin();
        const std::vector<VDataSeriesGroup>::const_iterator aXSlotEnd  = rGroup.end();

        if (aXSlotIter != aXSlotEnd)
        {
            VDataSeriesGroup aSeriesGroup(*aXSlotIter);
            if (aSeriesGroup.m_aSeriesVector.size())
            {
                VDataSeries* pSeries = aSeriesGroup.m_aSeriesVector[0];
                uno::Reference< XDataSeries > xSeries( pSeries ? pSeries->getModel() : nullptr );
                if( xSeries.is() )
                {
                    OUString aSeriesName( DataSeriesHelper::getDataSeriesLabel( xSeries, aRole ) );
                    aRetVector.push_back( aSeriesName );
                }
            }
        }
    }
    return comphelper::containerToSequence( aRetVector );
}

void VSeriesPlotter::setPageReferenceSize( const css::awt::Size & rPageRefSize )
{
    m_aPageReferenceSize = rPageRefSize;

    // set reference size also at all data series

    std::vector<VDataSeriesGroup> aSeriesGroups(FlattenVector(m_aZSlots));
    for (VDataSeriesGroup const & rGroup : aSeriesGroups)
    {
        for (VDataSeries* pSeries : rGroup.m_aSeriesVector)
        {
            pSeries->setPageReferenceSize(m_aPageReferenceSize);
        }
    }
}

//better performance for big data
void VSeriesPlotter::setCoordinateSystemResolution( const Sequence< sal_Int32 >& rCoordinateSystemResolution )
{
    m_aCoordinateSystemResolution = rCoordinateSystemResolution;
}

bool VSeriesPlotter::WantToPlotInFrontOfAxisLine()
{
    return ChartTypeHelper::isSeriesInFrontOfAxisLine( m_xChartTypeModel );
}

bool VSeriesPlotter::shouldSnapRectToUsedArea()
{
    return m_nDimension != 3;
}

std::vector< ViewLegendEntry > VSeriesPlotter::createLegendEntries(
              const awt::Size& rEntryKeyAspectRatio
            , css::chart::ChartLegendExpansion eLegendExpansion
            , const Reference< beans::XPropertySet >& xTextProperties
            , const Reference< drawing::XShapes >& xTarget
            , const Reference< lang::XMultiServiceFactory >& xShapeFactory
            , const Reference< uno::XComponentContext >& xContext
            )
{
    std::vector< ViewLegendEntry > aResult;

    if( xTarget.is() )
    {
        //iterate through all series
        bool bBreak = false;
        bool bFirstSeries = true;


        for (std::vector<VDataSeriesGroup> const & rGroupVector : m_aZSlots)
        {
            for (VDataSeriesGroup const & rGroup : rGroupVector)
            {
                for (VDataSeries* pSeries : rGroup.m_aSeriesVector)
                {
                    if (!pSeries)
                        continue;

                    std::vector<ViewLegendEntry> aSeriesEntries(
                            createLegendEntriesForSeries(
                                        rEntryKeyAspectRatio, *pSeries, xTextProperties,
                                        xTarget, xShapeFactory, xContext));

                    //add series entries to the result now

                    // use only the first series if VaryColorsByPoint is set for the first series
                    if (bFirstSeries && pSeries->isVaryColorsByPoint())
                        bBreak = true;
                    bFirstSeries = false;

                    // add entries reverse if chart is stacked in y-direction and the legend is not wide.
                    // If the legend is wide and we have a stacked bar-chart the normal order
                    // is the correct one
                    bool bReverse = false;
                    if( eLegendExpansion != css::chart::ChartLegendExpansion_WIDE )
                    {
                        StackingDirection eStackingDirection( pSeries->getStackingDirection() );
                        bReverse = ( eStackingDirection == StackingDirection_Y_STACKING );

                        //todo: respect direction of axis in future
                    }

                    if (bReverse)
                        aResult.insert( aResult.begin(), aSeriesEntries.begin(), aSeriesEntries.end() );
                    else
                        aResult.insert( aResult.end(), aSeriesEntries.begin(), aSeriesEntries.end() );
                }
                if (bBreak)
                    return aResult;
            }
        }
    }

    return aResult;
}

std::vector<VDataSeries*> VSeriesPlotter::getAllSeries()
{
    std::vector<VDataSeries*> aAllSeries;
    for (std::vector<VDataSeriesGroup> const & rXSlot : m_aZSlots)
    {
        for(VDataSeriesGroup const & rGroup : rXSlot)
        {
            std::vector<VDataSeries*> aSeriesList = rGroup.m_aSeriesVector;
            aAllSeries.insert(aAllSeries.end(), aSeriesList.begin(), aSeriesList.end());
        }
    }
    return aAllSeries;
}

namespace
{
bool lcl_HasVisibleLine( const uno::Reference< beans::XPropertySet >& xProps, bool& rbHasDashedLine )
{
    bool bHasVisibleLine = false;
    rbHasDashedLine = false;
    drawing::LineStyle aLineStyle = drawing::LineStyle_NONE;
    if( xProps.is() && ( xProps->getPropertyValue( "LineStyle") >>= aLineStyle ) )
    {
        if( aLineStyle != drawing::LineStyle_NONE )
            bHasVisibleLine = true;
        if( aLineStyle == drawing::LineStyle_DASH )
            rbHasDashedLine = true;
    }
    return bHasVisibleLine;
}

bool lcl_HasRegressionCurves( const VDataSeries& rSeries, bool& rbHasDashedLine )
{
    bool bHasRegressionCurves = false;
    Reference< XRegressionCurveContainer > xRegrCont( rSeries.getModel(), uno::UNO_QUERY );
    if( xRegrCont.is())
    {
        Sequence< Reference< XRegressionCurve > > aCurves( xRegrCont->getRegressionCurves() );
        sal_Int32 i = 0, nCount = aCurves.getLength();
        for( i=0; i<nCount; ++i )
        {
            if( aCurves[i].is() )
            {
                bHasRegressionCurves = true;
                lcl_HasVisibleLine( uno::Reference< beans::XPropertySet >( aCurves[i], uno::UNO_QUERY ), rbHasDashedLine );
            }
        }
    }
    return bHasRegressionCurves;
}
}
LegendSymbolStyle VSeriesPlotter::getLegendSymbolStyle()
{
    return LegendSymbolStyle_BOX;
}

awt::Size VSeriesPlotter::getPreferredLegendKeyAspectRatio()
{
    awt::Size aRet(1000,1000);
    if( m_nDimension==3 )
        return aRet;

    bool bSeriesAllowsLines = (getLegendSymbolStyle() == LegendSymbolStyle_LINE);
    bool bHasLines = false;
    bool bHasDashedLines = false;
    //iterate through all series
    for (VDataSeries* pSeries :  getAllSeries())
    {
        if( bSeriesAllowsLines )
        {
            bool bCurrentDashed = false;
            if( lcl_HasVisibleLine( pSeries->getPropertiesOfSeries(), bCurrentDashed ) )
            {
                bHasLines = true;
                if( bCurrentDashed )
                {
                    bHasDashedLines = true;
                    break;
                }
            }
        }
        bool bRegressionHasDashedLines=false;
        if( lcl_HasRegressionCurves( *pSeries, bRegressionHasDashedLines ) )
        {
            bHasLines = true;
            if( bRegressionHasDashedLines )
            {
                bHasDashedLines = true;
                break;
            }
        }
    }
    if( bHasLines )
    {
        if( bHasDashedLines )
            aRet = awt::Size(1600,-1);
        else
            aRet = awt::Size(800,-1);
    }
    return aRet;
}

uno::Any VSeriesPlotter::getExplicitSymbol( const VDataSeries& /*rSeries*/, sal_Int32 /*nPointIndex*/ )
{
    return uno::Any();
}

Reference< drawing::XShape > VSeriesPlotter::createLegendSymbolForSeries(
                  const awt::Size& rEntryKeyAspectRatio
                , const VDataSeries& rSeries
                , const Reference< drawing::XShapes >& xTarget
                , const Reference< lang::XMultiServiceFactory >& xShapeFactory )
{

    LegendSymbolStyle eLegendSymbolStyle = getLegendSymbolStyle();
    uno::Any aExplicitSymbol( getExplicitSymbol( rSeries, -1 ) );

    VLegendSymbolFactory::PropertyType ePropType =
        VLegendSymbolFactory::PropertyType::FilledSeries;

    // todo: maybe the property-style does not solely depend on the
    // legend-symbol type
    switch( eLegendSymbolStyle )
    {
        case LegendSymbolStyle_LINE:
            ePropType = VLegendSymbolFactory::PropertyType::LineSeries;
            break;
        default:
            break;
    };
    Reference< drawing::XShape > xShape( VLegendSymbolFactory::createSymbol( rEntryKeyAspectRatio,
        xTarget, eLegendSymbolStyle, xShapeFactory
            , rSeries.getPropertiesOfSeries(), ePropType, aExplicitSymbol ));

    return xShape;
}

Reference< drawing::XShape > VSeriesPlotter::createLegendSymbolForPoint(
                  const awt::Size& rEntryKeyAspectRatio
                , const VDataSeries& rSeries
                , sal_Int32 nPointIndex
                , const Reference< drawing::XShapes >& xTarget
                , const Reference< lang::XMultiServiceFactory >& xShapeFactory )
{

    LegendSymbolStyle eLegendSymbolStyle = getLegendSymbolStyle();
    uno::Any aExplicitSymbol( getExplicitSymbol(rSeries,nPointIndex) );

    VLegendSymbolFactory::PropertyType ePropType =
        VLegendSymbolFactory::PropertyType::FilledSeries;

    // todo: maybe the property-style does not solely depend on the
    // legend-symbol type
    switch( eLegendSymbolStyle )
    {
        case LegendSymbolStyle_LINE:
            ePropType = VLegendSymbolFactory::PropertyType::LineSeries;
            break;
        default:
            break;
    };

    // the default properties for the data point are the data series properties.
    // If a data point has own attributes overwrite them
    Reference< beans::XPropertySet > xSeriesProps( rSeries.getPropertiesOfSeries() );
    Reference< beans::XPropertySet > xPointSet( xSeriesProps );
    if( rSeries.isAttributedDataPoint( nPointIndex ) )
        xPointSet.set( rSeries.getPropertiesOfPoint( nPointIndex ));

    // if a data point has no own color use a color fom the diagram's color scheme
    if( ! rSeries.hasPointOwnColor( nPointIndex ))
    {
        Reference< util::XCloneable > xCloneable( xPointSet,uno::UNO_QUERY );
        if( xCloneable.is() && m_xColorScheme.is() )
        {
            xPointSet.set( xCloneable->createClone(), uno::UNO_QUERY );
            Reference< container::XChild > xChild( xPointSet, uno::UNO_QUERY );
            if( xChild.is())
                xChild->setParent( xSeriesProps );

            OSL_ASSERT( xPointSet.is());
            xPointSet->setPropertyValue(
                "Color", uno::Any( m_xColorScheme->getColorByIndex( nPointIndex )));
        }
    }

    Reference< drawing::XShape > xShape( VLegendSymbolFactory::createSymbol( rEntryKeyAspectRatio,
        xTarget, eLegendSymbolStyle, xShapeFactory, xPointSet, ePropType, aExplicitSymbol ));

    return xShape;
}

std::vector< ViewLegendEntry > VSeriesPlotter::createLegendEntriesForSeries(
              const awt::Size& rEntryKeyAspectRatio
            , const VDataSeries& rSeries
            , const Reference< beans::XPropertySet >& xTextProperties
            , const Reference< drawing::XShapes >& xTarget
            , const Reference< lang::XMultiServiceFactory >& xShapeFactory
            , const Reference< uno::XComponentContext >& xContext
            )
{
    std::vector< ViewLegendEntry > aResult;

    if( ! ( xShapeFactory.is() && xTarget.is() && xContext.is() ) )
        return aResult;

    try
    {
        ViewLegendEntry aEntry;
        OUString aLabelText;
        bool bVaryColorsByPoint = rSeries.isVaryColorsByPoint();
        if( bVaryColorsByPoint )
        {
            Sequence< OUString > aCategoryNames;
            if( m_pExplicitCategoriesProvider )
                aCategoryNames = m_pExplicitCategoriesProvider->getSimpleCategories();

            for( sal_Int32 nIdx=0; nIdx<aCategoryNames.getLength(); ++nIdx )
            {
                // symbol
                uno::Reference< drawing::XShapes > xSymbolGroup( AbstractShapeFactory::getOrCreateShapeFactory(xShapeFactory)->createGroup2D( xTarget ));

                // create the symbol
                Reference< drawing::XShape > xShape( createLegendSymbolForPoint( rEntryKeyAspectRatio,
                    rSeries, nIdx, xSymbolGroup, xShapeFactory ) );

                // set CID to symbol for selection
                if( xShape.is() )
                {
                    aEntry.aSymbol.set( xSymbolGroup, uno::UNO_QUERY );

                    OUString aChildParticle( ObjectIdentifier::createChildParticleWithIndex( OBJECTTYPE_DATA_POINT, nIdx ) );
                    aChildParticle = ObjectIdentifier::addChildParticle( aChildParticle, ObjectIdentifier::createChildParticleWithIndex( OBJECTTYPE_LEGEND_ENTRY, 0 ) );
                    OUString aCID = ObjectIdentifier::createClassifiedIdentifierForParticles( rSeries.getSeriesParticle(), aChildParticle );
                    AbstractShapeFactory::setShapeName( xShape, aCID );
                }

                // label
                aLabelText = aCategoryNames[nIdx];
                if( xShape.is() || !aLabelText.isEmpty() )
                {
                    aEntry.aLabel = FormattedStringHelper::createFormattedStringSequence( xContext, aLabelText, xTextProperties );
                    aResult.push_back(aEntry);
                }
            }
        }
        else
        {
            // symbol
            uno::Reference< drawing::XShapes > xSymbolGroup( AbstractShapeFactory::getOrCreateShapeFactory(xShapeFactory)->createGroup2D( xTarget ));

            // create the symbol
            Reference< drawing::XShape > xShape( createLegendSymbolForSeries(
                rEntryKeyAspectRatio, rSeries, xSymbolGroup, xShapeFactory ) );

            // set CID to symbol for selection
            if( xShape.is())
            {
                aEntry.aSymbol.set( xSymbolGroup, uno::UNO_QUERY );

                OUString aChildParticle( ObjectIdentifier::createChildParticleWithIndex( OBJECTTYPE_LEGEND_ENTRY, 0 ) );
                OUString aCID = ObjectIdentifier::createClassifiedIdentifierForParticles( rSeries.getSeriesParticle(), aChildParticle );
                AbstractShapeFactory::setShapeName( xShape, aCID );
            }

            // label
            aLabelText = DataSeriesHelper::getDataSeriesLabel( rSeries.getModel(), m_xChartTypeModel.is() ? m_xChartTypeModel->getRoleOfSequenceForSeriesLabel() : "values-y");
            aEntry.aLabel = FormattedStringHelper::createFormattedStringSequence( xContext, aLabelText, xTextProperties );

            aResult.push_back(aEntry);
        }

        // don't show legend entry of regression curve & friends if this type of chart
        // doesn't support statistics #i63016#, fdo#37197
        if (!ChartTypeHelper::isSupportingStatisticProperties( m_xChartTypeModel, m_nDimension ))
            return aResult;

        Reference< XRegressionCurveContainer > xRegrCont( rSeries.getModel(), uno::UNO_QUERY );
        if( xRegrCont.is())
        {
            Sequence< Reference< XRegressionCurve > > aCurves( xRegrCont->getRegressionCurves());
            sal_Int32 i = 0, nCount = aCurves.getLength();
            for( i=0; i<nCount; ++i )
            {
                if( aCurves[i].is() )
                {
                    //label
                    OUString aResStr( RegressionCurveHelper::getUINameForRegressionCurve( aCurves[i] ) );
                    replaceParamterInString( aResStr, "%SERIESNAME", aLabelText );
                    aEntry.aLabel = FormattedStringHelper::createFormattedStringSequence( xContext, aResStr, xTextProperties );

                    // symbol
                    uno::Reference< drawing::XShapes > xSymbolGroup( AbstractShapeFactory::getOrCreateShapeFactory(xShapeFactory)->createGroup2D( xTarget ));

                    // create the symbol
                    Reference< drawing::XShape > xShape( VLegendSymbolFactory::createSymbol( rEntryKeyAspectRatio,
                        xSymbolGroup, LegendSymbolStyle_LINE, xShapeFactory,
                        Reference< beans::XPropertySet >( aCurves[i], uno::UNO_QUERY ),
                        VLegendSymbolFactory::PropertyType::Line, uno::Any() ));

                    // set CID to symbol for selection
                    if( xShape.is())
                    {
                        aEntry.aSymbol.set( xSymbolGroup, uno::UNO_QUERY );

                        bool bAverageLine = RegressionCurveHelper::isMeanValueLine( aCurves[i] );
                        ObjectType eObjectType = bAverageLine ? OBJECTTYPE_DATA_AVERAGE_LINE : OBJECTTYPE_DATA_CURVE;
                        OUString aChildParticle( ObjectIdentifier::createChildParticleWithIndex( eObjectType, i ) );
                        aChildParticle = ObjectIdentifier::addChildParticle( aChildParticle, ObjectIdentifier::createChildParticleWithIndex( OBJECTTYPE_LEGEND_ENTRY, 0 ) );
                        OUString aCID = ObjectIdentifier::createClassifiedIdentifierForParticles( rSeries.getSeriesParticle(), aChildParticle );
                        AbstractShapeFactory::setShapeName( xShape, aCID );
                    }

                    aResult.push_back(aEntry);
                }
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        SAL_WARN("chart2", "Exception caught. " << ex );
    }
    return aResult;
}

VSeriesPlotter* VSeriesPlotter::createSeriesPlotter(
    const uno::Reference<XChartType>& xChartTypeModel
    , sal_Int32 nDimensionCount
    , bool bExcludingPositioning )
{
    if (!xChartTypeModel.is())
        return nullptr;

    OUString aChartType = xChartTypeModel->getChartType();

    VSeriesPlotter* pRet=nullptr;
    if( aChartType.equalsIgnoreAsciiCase( CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ) )
        pRet = new BarChart(xChartTypeModel,nDimensionCount);
    else if( aChartType.equalsIgnoreAsciiCase( CHART2_SERVICE_NAME_CHARTTYPE_BAR ) )
        pRet = new BarChart(xChartTypeModel,nDimensionCount);
    else if( aChartType.equalsIgnoreAsciiCase( CHART2_SERVICE_NAME_CHARTTYPE_AREA ) )
        pRet = new AreaChart(xChartTypeModel,nDimensionCount,true);
    else if( aChartType.equalsIgnoreAsciiCase( CHART2_SERVICE_NAME_CHARTTYPE_LINE ) )
        pRet = new AreaChart(xChartTypeModel,nDimensionCount,true,true);
    else if( aChartType.equalsIgnoreAsciiCase(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
        pRet = new AreaChart(xChartTypeModel,nDimensionCount,false,true);
    else if( aChartType.equalsIgnoreAsciiCase(CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE) )
        pRet = new BubbleChart(xChartTypeModel,nDimensionCount);
    else if( aChartType.equalsIgnoreAsciiCase(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
        pRet = new PieChart(xChartTypeModel,nDimensionCount, bExcludingPositioning );
    else if( aChartType.equalsIgnoreAsciiCase(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
        pRet = new NetChart(xChartTypeModel,nDimensionCount,true,new PolarPlottingPositionHelper());
    else if( aChartType.equalsIgnoreAsciiCase(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
        pRet = new NetChart(xChartTypeModel,nDimensionCount,false,new PolarPlottingPositionHelper());
    else if( aChartType.equalsIgnoreAsciiCase(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
        pRet = new CandleStickChart(xChartTypeModel,nDimensionCount);
    else
        pRet = new AreaChart(xChartTypeModel,nDimensionCount,false,true);
    return pRet;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
