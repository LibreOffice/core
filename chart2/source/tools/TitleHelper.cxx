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

#include "TitleHelper.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "AxisHelper.hxx"
#include "DiagramHelper.hxx"
#include <com/sun/star/chart2/FormattedString.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <rtl/ustrbuf.hxx>

namespace chart
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

namespace {

uno::Reference< XTitled > lcl_getTitleParentFromDiagram(
      TitleHelper::eTitleType nTitleIndex
    , const uno::Reference< XDiagram >& xDiagram )
{
    uno::Reference< XTitled > xResult;

    if( nTitleIndex == TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION ||
        nTitleIndex == TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION )
    {
        bool bDummy = false;
        bool bIsVertical = DiagramHelper::getVertical( xDiagram, bDummy, bDummy );

        if( nTitleIndex == TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION )
            nTitleIndex = bIsVertical ? TitleHelper::X_AXIS_TITLE : TitleHelper::Y_AXIS_TITLE;
        else
            nTitleIndex = bIsVertical ? TitleHelper::Y_AXIS_TITLE : TitleHelper::X_AXIS_TITLE;
    }

    switch( nTitleIndex )
    {
        case TitleHelper::SUB_TITLE:
            if( xDiagram.is())
                xResult.set( xDiagram, uno::UNO_QUERY );
            break;
        case TitleHelper::X_AXIS_TITLE:
            if( xDiagram.is())
                xResult.set( AxisHelper::getAxis( 0, true, xDiagram ), uno::UNO_QUERY );
            break;
        case TitleHelper::Y_AXIS_TITLE:
            if( xDiagram.is())
                xResult.set( AxisHelper::getAxis( 1, true, xDiagram ), uno::UNO_QUERY );
            break;
        case TitleHelper::Z_AXIS_TITLE:
            if( xDiagram.is())
                xResult.set( AxisHelper::getAxis( 2, true, xDiagram ), uno::UNO_QUERY );
            break;
        case TitleHelper::SECONDARY_X_AXIS_TITLE:
            if( xDiagram.is())
                xResult.set( AxisHelper::getAxis( 0, false, xDiagram ), uno::UNO_QUERY );
            break;
        case TitleHelper::SECONDARY_Y_AXIS_TITLE:
            if( xDiagram.is())
                xResult.set( AxisHelper::getAxis( 1, false, xDiagram ), uno::UNO_QUERY );
            break;

        case TitleHelper::MAIN_TITLE:
        default:
            OSL_FAIL( "Unsupported Title-Type requested" );
            break;
    }

    return xResult;
}

uno::Reference< XTitled > lcl_getTitleParent( TitleHelper::eTitleType nTitleIndex
                                              , const uno::Reference< XDiagram >& xDiagram )
{
    uno::Reference< XTitled > xResult;
    switch( nTitleIndex )
    {
        case TitleHelper::MAIN_TITLE:
            SAL_WARN("chart2", "should not be reached");
            break;
        case TitleHelper::SUB_TITLE:
        case TitleHelper::X_AXIS_TITLE:
        case TitleHelper::Y_AXIS_TITLE:
        case TitleHelper::Z_AXIS_TITLE:
        case TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION:
        case TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION:
        case TitleHelper::SECONDARY_X_AXIS_TITLE:
        case TitleHelper::SECONDARY_Y_AXIS_TITLE:
            xResult.set( lcl_getTitleParentFromDiagram( nTitleIndex, xDiagram ));
            break;
        default:
            OSL_FAIL( "Unsupported Title-Type requested" );
            break;
    }

    return xResult;
}

uno::Reference< XTitled > lcl_getTitleParent( TitleHelper::eTitleType nTitleIndex
                                              , const uno::Reference< frame::XModel >& xModel )
{
    if(nTitleIndex == TitleHelper::MAIN_TITLE)
    {
        uno::Reference< XTitled > xTitled( xModel, uno::UNO_QUERY );
        return xTitled;
    }

    uno::Reference< XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
    uno::Reference< XDiagram > xDiagram;

    if( xChartDoc.is())
        xDiagram.set( xChartDoc->getFirstDiagram());

    return lcl_getTitleParent( nTitleIndex, xDiagram );
}

}

uno::Reference< XTitle > TitleHelper::getTitle( TitleHelper::eTitleType nTitleIndex
                            , ChartModel& rModel )
{
    if(nTitleIndex == TitleHelper::MAIN_TITLE)
        return rModel.getTitleObject();

    uno::Reference< XDiagram > xDiagram( rModel.getFirstDiagram(), uno::UNO_QUERY );
    uno::Reference< XTitled > xTitled( lcl_getTitleParent( nTitleIndex, xDiagram ) );
    if( xTitled.is())
        return xTitled->getTitleObject();
    return nullptr;
}

uno::Reference< XTitle > TitleHelper::getTitle( TitleHelper::eTitleType nTitleIndex
                            , const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< XTitled > xTitled;
    if(nTitleIndex == TitleHelper::MAIN_TITLE)
    {
        xTitled.set( xModel, uno::UNO_QUERY );
    }
    else
    {
        uno::Reference< XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
        uno::Reference< XDiagram > xDiagram;
        if( xChartDoc.is())
            xDiagram.set( xChartDoc->getFirstDiagram());
        xTitled = lcl_getTitleParent( nTitleIndex, xDiagram );
    }
    if( xTitled.is())
        return xTitled->getTitleObject();
    return nullptr;
}

uno::Reference< XTitle > TitleHelper::createOrShowTitle(
      TitleHelper::eTitleType eTitleType
    , const OUString& rTitleText
    , const uno::Reference< frame::XModel >& xModel
    , const uno::Reference< uno::XComponentContext > & xContext
    , ReferenceSizeProvider * pRefSizeProvider )
{
    uno::Reference< chart2::XTitle > xTitled( TitleHelper::getTitle( eTitleType, xModel ) );
    if( xTitled.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xProps(xTitled, css::uno::UNO_QUERY_THROW);
        xProps->setPropertyValue("Visible",css::uno::Any(true));
        return xTitled;
    }
    else
    {
        return createTitle(eTitleType, rTitleText, xModel, xContext, pRefSizeProvider);
    }
}

uno::Reference< XTitle > TitleHelper::createTitle(
      TitleHelper::eTitleType eTitleType
    , const OUString& rTitleText
    , const uno::Reference< frame::XModel >& xModel
    , const uno::Reference< uno::XComponentContext > & xContext
    , ReferenceSizeProvider * pRefSizeProvider )
{
    uno::Reference< XTitle > xTitle;
    uno::Reference< XTitled > xTitled( lcl_getTitleParent( eTitleType, xModel ) );

    if( !xTitled.is() )
    {
        uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xModel ) );
        uno::Reference< chart2::XAxis > xAxis;
        switch( eTitleType )
        {
            case TitleHelper::SECONDARY_X_AXIS_TITLE:
                xAxis = AxisHelper::createAxis( 0, false, xDiagram, xContext );
                break;
            case TitleHelper::SECONDARY_Y_AXIS_TITLE:
                xAxis = AxisHelper::createAxis( 1, false, xDiagram, xContext );
               break;
            default:
               break;
        }
        uno::Reference< beans::XPropertySet > xProps( xAxis, uno::UNO_QUERY );
        if( xProps.is() )
        {
            xProps->setPropertyValue( "Show", uno::Any( false ) );
            xTitled = lcl_getTitleParent( eTitleType, xModel );
        }
    }

    if(xTitled.is())
    {
        uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xModel ) );

        xTitle.set( xContext->getServiceManager()->createInstanceWithContext(
                        "com.sun.star.chart2.Title",
                        xContext ), uno::UNO_QUERY );

        if(xTitle.is())
        {
            // default char height (main: 13.0 == default)
            float fDefaultCharHeightSub = 11.0;
            float fDefaultCharHeightAxis = 9.0;
            switch( eTitleType )
            {
                case TitleHelper::SUB_TITLE:
                    TitleHelper::setCompleteString(
                        rTitleText, xTitle, xContext, & fDefaultCharHeightSub );
                    break;
                case TitleHelper::X_AXIS_TITLE:
                case TitleHelper::Y_AXIS_TITLE:
                case TitleHelper::Z_AXIS_TITLE:
                case TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION:
                case TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION:
                case TitleHelper::SECONDARY_X_AXIS_TITLE:
                case TitleHelper::SECONDARY_Y_AXIS_TITLE:
                    TitleHelper::setCompleteString(
                        rTitleText, xTitle, xContext, & fDefaultCharHeightAxis );
                    break;
                default:
                    TitleHelper::setCompleteString( rTitleText, xTitle, xContext );
                    break;
            }

            // set/clear autoscale
            if( pRefSizeProvider )
                pRefSizeProvider->setValuesAtTitle( xTitle );

            xTitled->setTitleObject( xTitle );

            //default rotation 90 degree for y axis title in normal coordinatesystems or for x axis title for swapped coordinatesystems
            if( eTitleType == TitleHelper::X_AXIS_TITLE ||
                eTitleType == TitleHelper::Y_AXIS_TITLE ||
                eTitleType == TitleHelper::SECONDARY_X_AXIS_TITLE ||
                eTitleType == TitleHelper::SECONDARY_Y_AXIS_TITLE )

            {
                try
                {
                    bool bDummy = false;
                    bool bIsVertical = DiagramHelper::getVertical( xDiagram, bDummy, bDummy );

                    Reference< beans::XPropertySet > xTitleProps( xTitle, uno::UNO_QUERY );
                    if( xTitleProps.is() )
                    {
                        if( (!bIsVertical && eTitleType == TitleHelper::Y_AXIS_TITLE)
                            || (bIsVertical && eTitleType == TitleHelper::X_AXIS_TITLE)
                            || (!bIsVertical && eTitleType == TitleHelper::SECONDARY_Y_AXIS_TITLE)
                            || (bIsVertical && eTitleType == TitleHelper::SECONDARY_X_AXIS_TITLE) )
                        {
                            double fNewAngleDegree = 90.0;
                            xTitleProps->setPropertyValue( "TextRotation", uno::Any( fNewAngleDegree ));
                        }
                    }
                }
                catch( const uno::Exception & ex )
                {
                    ASSERT_EXCEPTION( ex );
                }
            }
        }
    }
    return xTitle;

}

OUString TitleHelper::getCompleteString( const uno::Reference< XTitle >& xTitle )
{
    OUString aRet;
    if(!xTitle.is())
        return aRet;
    uno::Sequence< uno::Reference< XFormattedString > > aStringList = xTitle->getText();
    for( sal_Int32 nN=0; nN<aStringList.getLength();nN++ )
        aRet += aStringList[nN]->getString();
    return aRet;
}

void TitleHelper::setCompleteString( const OUString& rNewText
                    , const uno::Reference< XTitle >& xTitle
                    , const uno::Reference< uno::XComponentContext > & xContext
                    , float * pDefaultCharHeight /* = 0 */ )
{
    //the format of the first old text portion will be maintained if there is any
    if(!xTitle.is())
        return;

    OUString aNewText = rNewText;

    bool bStacked = false;
    uno::Reference< beans::XPropertySet > xTitleProperties( xTitle, uno::UNO_QUERY );
    if( xTitleProperties.is() )
        xTitleProperties->getPropertyValue( "StackCharacters" ) >>= bStacked;

    if( bStacked )
    {
        //#i99841# remove linebreaks that were added for vertical stacking
        OUStringBuffer aUnstackedStr;
        OUStringBuffer aSource(rNewText);

        bool bBreakIgnored = false;
        sal_Int32 nLen = rNewText.getLength();
        for( sal_Int32 nPos = 0; nPos < nLen; ++nPos )
        {
            sal_Unicode aChar = aSource[nPos];
            if( aChar != '\n' )
            {
                aUnstackedStr.append( aChar );
                bBreakIgnored = false;
            }
            else if( aChar == '\n' && bBreakIgnored )
                aUnstackedStr.append( aChar );
            else
                bBreakIgnored = true;
        }
        aNewText = aUnstackedStr.makeStringAndClear();
    }

    uno::Sequence< uno::Reference< XFormattedString > > aNewStringList(1);

    uno::Sequence< uno::Reference< XFormattedString > >  aOldStringList = xTitle->getText();
    if( aOldStringList.getLength() )
    {
        aNewStringList[0].set( aOldStringList[0] );
        aNewStringList[0]->setString( aNewText );
    }
    else
    {
        uno::Reference< chart2::XFormattedString2 > xFormattedString =
            chart2::FormattedString::create( xContext );

        xFormattedString->setString( aNewText );
        aNewStringList[0].set( xFormattedString );
        if( pDefaultCharHeight != nullptr )
        {
            try
            {
                uno::Any aFontSize( *pDefaultCharHeight );
                xFormattedString->setPropertyValue( "CharHeight", aFontSize );
                xFormattedString->setPropertyValue( "CharHeightAsian", aFontSize );
                xFormattedString->setPropertyValue( "CharHeightComplex", aFontSize );
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
    xTitle->setText( aNewStringList );
}

void TitleHelper::removeTitle( TitleHelper::eTitleType nTitleIndex
                    , const css::uno::Reference< css::frame::XModel >& xModel )
{
    uno::Reference< XTitled > xTitled( lcl_getTitleParent( nTitleIndex, xModel ) );
    if( xTitled.is())
    {
        xTitled->setTitleObject(nullptr);
    }
}

void TitleHelper::hideTitle( TitleHelper::eTitleType nTitleIndex
                    , const css::uno::Reference< css::frame::XModel >& xModel )
{
    uno::Reference< chart2::XTitle > xTitled( TitleHelper::getTitle( nTitleIndex, xModel ) );
    if( xTitled.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xProps(xTitled, css::uno::UNO_QUERY_THROW);
        xProps->setPropertyValue("Visible",css::uno::Any(false));
    }
}

bool TitleHelper::getTitleType( eTitleType& rType
                    , const css::uno::Reference< css::chart2::XTitle >& xTitle
                    , ChartModel& rModel )
{
    if( !xTitle.is() )
        return false;

    Reference< chart2::XTitle > xCurrentTitle;
    for( sal_Int32 nTitleType = TITLE_BEGIN; nTitleType < NORMAL_TITLE_END; nTitleType++ )
    {
        xCurrentTitle = TitleHelper::getTitle( static_cast<eTitleType>(nTitleType), rModel );
        if( xCurrentTitle == xTitle )
        {
            rType = static_cast<eTitleType>(nTitleType);
            return true;
        }
    }

    return false;
}

bool TitleHelper::getTitleType( eTitleType& rType
                    , const css::uno::Reference< css::chart2::XTitle >& xTitle
                    , const css::uno::Reference< css::frame::XModel >& xModel )
{
    if( !xTitle.is() || !xModel.is() )
        return false;

    Reference< chart2::XTitle > xCurrentTitle;
    for( sal_Int32 nTitleType = TITLE_BEGIN; nTitleType < NORMAL_TITLE_END; nTitleType++ )
    {
        xCurrentTitle = TitleHelper::getTitle( static_cast<eTitleType>(nTitleType), xModel );
        if( xCurrentTitle == xTitle )
        {
            rType = static_cast<eTitleType>(nTitleType);
            return true;
        }
    }

    return false;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
