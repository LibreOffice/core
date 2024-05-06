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

#include <TitleHelper.hxx>
#include <Title.hxx>
#include <ChartModel.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <Diagram.hxx>
#include <ReferenceSizeProvider.hxx>
#include <com/sun/star/chart2/FormattedString.hpp>
#include <rtl/ustrbuf.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>

namespace chart
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

namespace {

uno::Reference< XTitled > lcl_getTitleParentFromDiagram(
      TitleHelper::eTitleType nTitleIndex
    , const rtl::Reference< Diagram >& xDiagram )
{
    uno::Reference< XTitled > xResult;

    if( nTitleIndex == TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION ||
        nTitleIndex == TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION )
    {
        bool bDummy = false;
        bool bIsVertical = xDiagram && xDiagram->getVertical( bDummy, bDummy );

        if( nTitleIndex == TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION )
            nTitleIndex = bIsVertical ? TitleHelper::X_AXIS_TITLE : TitleHelper::Y_AXIS_TITLE;
        else
            nTitleIndex = bIsVertical ? TitleHelper::Y_AXIS_TITLE : TitleHelper::X_AXIS_TITLE;
    }

    switch( nTitleIndex )
    {
        case TitleHelper::SUB_TITLE:
            if( xDiagram.is())
                xResult = xDiagram;
            break;
        case TitleHelper::X_AXIS_TITLE:
            if( xDiagram.is())
                xResult = AxisHelper::getAxis( 0, true, xDiagram );
            break;
        case TitleHelper::Y_AXIS_TITLE:
            if( xDiagram.is())
                xResult = AxisHelper::getAxis( 1, true, xDiagram );
            break;
        case TitleHelper::Z_AXIS_TITLE:
            if( xDiagram.is())
                xResult = AxisHelper::getAxis( 2, true, xDiagram );
            break;
        case TitleHelper::SECONDARY_X_AXIS_TITLE:
            if( xDiagram.is())
                xResult = AxisHelper::getAxis( 0, false, xDiagram );
            break;
        case TitleHelper::SECONDARY_Y_AXIS_TITLE:
            if( xDiagram.is())
                xResult = AxisHelper::getAxis( 1, false, xDiagram );
            break;

        case TitleHelper::MAIN_TITLE:
        default:
            OSL_FAIL( "Unsupported Title-Type requested" );
            break;
    }

    return xResult;
}

uno::Reference< XTitled > lcl_getTitleParent( TitleHelper::eTitleType nTitleIndex
                                              , const rtl::Reference< Diagram >& xDiagram )
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
                                              , const rtl::Reference<::chart::ChartModel>& xModel )
{
    if(nTitleIndex == TitleHelper::MAIN_TITLE)
    {
        return xModel;
    }

    rtl::Reference< Diagram > xDiagram;

    if( xModel.is())
        xDiagram = xModel->getFirstChartDiagram();

    return lcl_getTitleParent( nTitleIndex, xDiagram );
}

}

rtl::Reference< Title > TitleHelper::getTitle( TitleHelper::eTitleType nTitleIndex
                            , ChartModel& rModel )
{
    if(nTitleIndex == TitleHelper::MAIN_TITLE)
        return rModel.getTitleObject2();

    rtl::Reference< Diagram > xDiagram = rModel.getFirstChartDiagram();
    uno::Reference< XTitled > xTitled( lcl_getTitleParent( nTitleIndex, xDiagram ) );
    if( !xTitled )
        return nullptr;
    uno::Reference<XTitle> xTitle = xTitled->getTitleObject();
    auto pTitle = dynamic_cast<Title*>(xTitle.get());
    assert(!xTitle || pTitle);
    return pTitle;
}

rtl::Reference< Title > TitleHelper::getTitle( TitleHelper::eTitleType nTitleIndex
                            , const rtl::Reference<ChartModel>& xModel )
{
    uno::Reference< XTitled > xTitled;
    if(nTitleIndex == TitleHelper::MAIN_TITLE)
    {
        xTitled = xModel;
    }
    else
    {
        rtl::Reference< Diagram > xDiagram;
        if( xModel.is())
            xDiagram = xModel->getFirstChartDiagram();
        xTitled = lcl_getTitleParent( nTitleIndex, xDiagram );
    }
    if( !xTitled )
        return nullptr;
    uno::Reference<XTitle> xTitle = xTitled->getTitleObject();
    Title* pTitle = dynamic_cast<Title*>(xTitle.get());
    assert(!xTitle || pTitle);
    return pTitle;
}

rtl::Reference< Title > TitleHelper::createOrShowTitle(
      TitleHelper::eTitleType eTitleType
    , const OUString& rTitleText
    , const rtl::Reference<ChartModel>& xModel
    , const uno::Reference< uno::XComponentContext > & xContext )
{
    rtl::Reference< Title > xTitled( TitleHelper::getTitle( eTitleType, xModel ) );
    if( xTitled.is())
    {
        xTitled->setPropertyValue(u"Visible"_ustr,css::uno::Any(true));
        return xTitled;
    }
    else
    {
        return createTitle(eTitleType, rTitleText, xModel, xContext, nullptr/*pRefSizeProvider*/);
    }
}

rtl::Reference< Title > TitleHelper::createTitle(
      TitleHelper::eTitleType eTitleType
    , const OUString& rTitleText
    , const rtl::Reference<ChartModel>& xModel
    , const uno::Reference< uno::XComponentContext > & xContext
    , ReferenceSizeProvider * pRefSizeProvider )
{
    rtl::Reference< ::chart::Title > xTitle;
    uno::Reference< XTitled > xTitled( lcl_getTitleParent( eTitleType, xModel ) );

    if( !xTitled.is() )
    {
        rtl::Reference< Diagram > xDiagram( xModel->getFirstChartDiagram() );
        rtl::Reference< Axis > xAxis;
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
        if( xAxis.is() )
        {
            xAxis->setPropertyValue( u"Show"_ustr, uno::Any( false ) );
            xTitled = lcl_getTitleParent( eTitleType, xModel );
        }
    }

    if(xTitled.is())
    {
        rtl::Reference< Diagram > xDiagram( xModel->getFirstChartDiagram() );

        xTitle = new ::chart::Title();

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
                bool bIsVertical = xDiagram->getVertical( bDummy, bDummy );

                if( (!bIsVertical && eTitleType == TitleHelper::Y_AXIS_TITLE)
                    || (bIsVertical && eTitleType == TitleHelper::X_AXIS_TITLE)
                    || (!bIsVertical && eTitleType == TitleHelper::SECONDARY_Y_AXIS_TITLE)
                    || (bIsVertical && eTitleType == TitleHelper::SECONDARY_X_AXIS_TITLE) )
                {
                    xTitle->setPropertyValue( u"TextRotation"_ustr, uno::Any( 90.0 ));
                }
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
    }
    return xTitle;

}

OUString TitleHelper::getCompleteString( const rtl::Reference< Title >& xTitle )
{
    if(!xTitle.is())
        return OUString();
    OUStringBuffer aRet;
    const uno::Sequence< uno::Reference< XFormattedString > > aStringList = xTitle->getText();
    for( uno::Reference< XFormattedString > const & formattedStr : aStringList )
        aRet.append( formattedStr->getString() );
    return aRet.makeStringAndClear();
}

OUString TitleHelper::getUnstackedStr(const OUString& rNewText)
{
    //#i99841# remove linebreaks that were added for vertical stacking
    OUStringBuffer aUnstackedStr;
    OUStringBuffer aSource(rNewText);

    bool bBreakIgnored = false;
    sal_Int32 nLen = rNewText.getLength();
    for (sal_Int32 nPos = 0; nPos < nLen; ++nPos)
    {
        sal_Unicode aChar = aSource[nPos];
        if (aChar != '\n')
        {
            aUnstackedStr.append(aChar);
            bBreakIgnored = false;
        }
        else if (aChar == '\n' && bBreakIgnored)
            aUnstackedStr.append(aChar);
        else
            bBreakIgnored = true;
    }
    return aUnstackedStr.makeStringAndClear();
}

void TitleHelper::setFormattedString( const rtl::Reference< Title >& xTitle,
    const css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > >& aNewFormattedTitle )
{
    if (!xTitle.is() || !aNewFormattedTitle.hasElements())
        return;

    bool bStacked = false;
    xTitle->getPropertyValue(u"StackCharacters"_ustr) >>= bStacked;

    if (bStacked)
    {
        for (uno::Reference< chart2::XFormattedString >const& formattedStr : aNewFormattedTitle)
        {
            formattedStr->setString(TitleHelper::getUnstackedStr(formattedStr->getString()));
        }
    }

    xTitle->setText(aNewFormattedTitle);
}

void TitleHelper::setCompleteString( const OUString& rNewText
                    , const rtl::Reference< Title >& xTitle
                    , const uno::Reference< uno::XComponentContext > & xContext
                    , const float * pDefaultCharHeight /* = 0 */
                    , bool bDialogTitle /*= false*/ )
{
    if (!xTitle.is())
        return;

    bool bStacked = false;
    if( xTitle.is() )
        xTitle->getPropertyValue( u"StackCharacters"_ustr ) >>= bStacked;

    OUString aNewText = rNewText;
    if( bStacked )
    {
        aNewText = getUnstackedStr(rNewText);
    }

    uno::Sequence< uno::Reference< XFormattedString > > aNewStringList;
    uno::Sequence< uno::Reference< XFormattedString > > aOldStringList = xTitle->getText();
    if( aOldStringList.hasElements())
    {
        const OUString aFullString = getCompleteString(xTitle);
        if (bDialogTitle && aNewText.equals(getUnstackedStr(aFullString)))
        {
            // If the new title setted from a dialog window to a new string
            // the first old text portion will be maintained if it's a new string,
            // otherwise we use the original one.
            aNewStringList = aOldStringList;
        }
        else
        {
            // If the new title setted from a dialog to a new string the first
            // old text portion will be maintained if there was any. Also in case of ODF
            // import which still not support non-uniform formatted titles
            aNewStringList = { aOldStringList[0] };
            aNewStringList[0]->setString(aNewText);
        }
    }
    else
    {
        uno::Reference< chart2::XFormattedString2 > xFormattedString =
            chart2::FormattedString::create( xContext );

        xFormattedString->setString( aNewText );
        aNewStringList = { xFormattedString };
        if( pDefaultCharHeight != nullptr )
        {
            try
            {
                uno::Any aFontSize( *pDefaultCharHeight );
                xFormattedString->setPropertyValue( u"CharHeight"_ustr, aFontSize );
                xFormattedString->setPropertyValue( u"CharHeightAsian"_ustr, aFontSize );
                xFormattedString->setPropertyValue( u"CharHeightComplex"_ustr, aFontSize );
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
    }
    xTitle->setText( aNewStringList );
}

void TitleHelper::removeTitle( TitleHelper::eTitleType nTitleIndex
                    , const rtl::Reference<ChartModel>& xModel )
{
    uno::Reference< XTitled > xTitled( lcl_getTitleParent( nTitleIndex, xModel ) );
    if( xTitled.is())
    {
        xTitled->setTitleObject(nullptr);
    }
}

bool TitleHelper::getTitleType( eTitleType& rType
                    , const rtl::Reference< Title >& xTitle
                    , const rtl::Reference<ChartModel>& xModel )
{
    if( !xTitle.is() || !xModel.is() )
        return false;

    rtl::Reference< Title > xCurrentTitle;
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

void TitleHelper::hideTitle( TitleHelper::eTitleType nTitleIndex
        , const rtl::Reference<ChartModel>& xModel)
{
    uno::Reference< chart2::XTitle > xTitled( TitleHelper::getTitle( nTitleIndex, xModel ) );
    if( xTitled.is())
    {
        css::uno::Reference<css::beans::XPropertySet> xProps(xTitled, css::uno::UNO_QUERY_THROW);
        xProps->setPropertyValue(u"Visible"_ustr,css::uno::Any(false));
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
