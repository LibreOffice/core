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
#include "TitleHelper.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "AxisHelper.hxx"
#include "DiagramHelper.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <rtl/ustrbuf.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

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
            OSL_ENSURE( false, "Unsupported Title-Type requested" );
            break;
    }

    return xResult;
}

uno::Reference< XTitled > lcl_getTitleParent( TitleHelper::eTitleType nTitleIndex
                                              , const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< XTitled > xResult;
    uno::Reference< XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
    uno::Reference< XDiagram > xDiagram;
    if( xChartDoc.is())
        xDiagram.set( xChartDoc->getFirstDiagram());

    switch( nTitleIndex )
    {
        case TitleHelper::MAIN_TITLE:
            xResult.set( xModel, uno::UNO_QUERY );
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
            OSL_ENSURE( false, "Unsupported Title-Type requested" );
            break;
    }

    return xResult;
}

uno::Reference< XTitle > TitleHelper::getTitle( TitleHelper::eTitleType nTitleIndex
                            , const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< XTitled > xTitled( lcl_getTitleParent( nTitleIndex, xModel ) );
    if( xTitled.is())
        return xTitled->getTitleObject();
    return NULL;
}

uno::Reference< XTitle > TitleHelper::createTitle(
      TitleHelper::eTitleType eTitleType
    , const rtl::OUString& rTitleText
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
            xProps->setPropertyValue( C2U( "Show" ), uno::makeAny( sal_False ) );
            xTitled = lcl_getTitleParent( eTitleType, xModel );
        }
    }

    if(xTitled.is())
    {
        uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xModel ) );

        xTitle.set( xContext->getServiceManager()->createInstanceWithContext(
                        C2U( "com.sun.star.chart2.Title" ),
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
                            xTitleProps->setPropertyValue( C2U( "TextRotation" ), uno::makeAny( fNewAngleDegree ));
                        }
                    }
                }
                catch( uno::Exception & ex )
                {
                    ASSERT_EXCEPTION( ex );
                }
            }
        }
    }
    return xTitle;

}

rtl::OUString TitleHelper::getCompleteString( const uno::Reference< XTitle >& xTitle )
{
    rtl::OUString aRet;
    if(!xTitle.is())
        return aRet;
    uno::Sequence< uno::Reference< XFormattedString > > aStringList = xTitle->getText();
    for( sal_Int32 nN=0; nN<aStringList.getLength();nN++ )
        aRet += aStringList[nN]->getString();
    return aRet;
}

void TitleHelper::setCompleteString( const rtl::OUString& rNewText
                    , const uno::Reference< XTitle >& xTitle
                    , const uno::Reference< uno::XComponentContext > & xContext
                    , float * pDefaultCharHeight /* = 0 */ )
{
    //the format of the first old text portion will be maintained if there is any
    if(!xTitle.is())
        return;

    rtl::OUString aNewText = rNewText;

    bool bStacked = false;
    uno::Reference< beans::XPropertySet > xTitleProperties( xTitle, uno::UNO_QUERY );
    if( xTitleProperties.is() )
        xTitleProperties->getPropertyValue( C2U( "StackCharacters" ) ) >>= bStacked;

    if( bStacked )
    {
        //#i99841# remove linebreaks that were added for vertical stacking
        rtl::OUStringBuffer aUnstackedStr;
        rtl::OUStringBuffer aSource(rNewText);

        bool bBreakIgnored = false;
        sal_Int32 nLen = rNewText.getLength();
        for( sal_Int32 nPos = 0; nPos < nLen; ++nPos )
        {
            sal_Unicode aChar = aSource.charAt( nPos );
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
        uno::Reference< uno::XInterface > xI(
            xContext->getServiceManager()->createInstanceWithContext(
            C2U( "com.sun.star.chart2.FormattedString" ), xContext ) );
        uno::Reference< XFormattedString > xFormattedString( xI, uno::UNO_QUERY );

        if(xFormattedString.is())
        {
            xFormattedString->setString( aNewText );
            aNewStringList[0].set( xFormattedString );
            if( pDefaultCharHeight != 0 )
            {
                try
                {
                    uno::Reference< beans::XPropertySet > xProp( xFormattedString, uno::UNO_QUERY_THROW );

                    uno::Any aFontSize( uno::makeAny( *pDefaultCharHeight ));
                    xProp->setPropertyValue( C2U("CharHeight"), aFontSize );
                    xProp->setPropertyValue( C2U("CharHeightAsian"), aFontSize );
                    xProp->setPropertyValue( C2U("CharHeightComplex"), aFontSize );
                }
                catch( uno::Exception & ex )
                {
                    ASSERT_EXCEPTION( ex );
                }
            }
        }
    }
    xTitle->setText( aNewStringList );
}

void TitleHelper::removeTitle( TitleHelper::eTitleType nTitleIndex
                    , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xModel )
{
    uno::Reference< XTitled > xTitled( lcl_getTitleParent( nTitleIndex, xModel ) );
    if( xTitled.is())
    {
        xTitled->setTitleObject(NULL);
    }
}

bool TitleHelper::getTitleType( eTitleType& rType
                    , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XTitle >& xTitle
                    , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xModel )
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

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
