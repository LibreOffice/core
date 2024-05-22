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
#include "vbaview.hxx"
#include <utility>
#include <vbahelper/vbahelper.hxx>
#include <basic/sberrors.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <ooo/vba/word/WdSpecialPane.hpp>
#include <ooo/vba/word/WdViewType.hpp>
#include <ooo/vba/word/WdSeekView.hpp>

#include "wordvbahelper.hxx"
#include "vbaheaderfooterhelper.hxx"
#include <view.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

const sal_Int32 DEFAULT_BODY_DISTANCE = 500;

SwVbaView::SwVbaView( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext,
    uno::Reference< frame::XModel > xModel ) :
    SwVbaView_BASE( rParent, rContext ), mxModel(std::move( xModel ))
{
    uno::Reference< frame::XController > xController = mxModel->getCurrentController();

    uno::Reference< text::XTextViewCursorSupplier > xTextViewCursorSupp( xController, uno::UNO_QUERY_THROW );
    mxViewCursor = xTextViewCursorSupp->getViewCursor();

    uno::Reference< view::XViewSettingsSupplier > xViewSettingSupp( xController, uno::UNO_QUERY_THROW );
    mxViewSettings.set( xViewSettingSupp->getViewSettings(), uno::UNO_SET_THROW );
}

SwVbaView::~SwVbaView()
{
}

sal_Bool SwVbaView::getShowAll()
{
    bool bShowFormattingMarks = false;
    mxViewSettings->getPropertyValue(u"ShowNonprintingCharacters"_ustr) >>= bShowFormattingMarks;
    return bShowFormattingMarks;
}

void SwVbaView::setShowAll(sal_Bool bSet)
{
    mxViewSettings->setPropertyValue(u"ShowNonprintingCharacters"_ustr, uno::Any(bSet));
}

::sal_Int32 SAL_CALL
SwVbaView::getSeekView()
{
    // FIXME: if the view cursor is in table, field, section and frame
    // handle if the cursor is in table
    uno::Reference< text::XText > xCurrentText = mxViewCursor->getText();
    uno::Reference< beans::XPropertySet > xCursorProps( mxViewCursor, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextContent > xTextContent;
    while( xCursorProps->getPropertyValue(u"TextTable"_ustr) >>= xTextContent )
    {
        xCurrentText = xTextContent->getAnchor()->getText();
        xCursorProps.set( xCurrentText->createTextCursor(), uno::UNO_QUERY_THROW );
    }
    uno::Reference< lang::XServiceInfo > xServiceInfo( xCurrentText, uno::UNO_QUERY_THROW );
    OUString aImplName = xServiceInfo->getImplementationName();
    if ( aImplName == "SwXBodyText" )
    {
        return word::WdSeekView::wdSeekMainDocument;
    }
    else if ( aImplName == "SwXHeadFootText" )
    {
        if( HeaderFooterHelper::isHeader( mxModel ) )
        {
            if( HeaderFooterHelper::isFirstPageHeader( mxModel ) )
                return word::WdSeekView::wdSeekFirstPageHeader;
            else if( HeaderFooterHelper::isEvenPagesHeader( mxModel ) )
                return word::WdSeekView::wdSeekEvenPagesHeader;
            else
                return word::WdSeekView::wdSeekPrimaryHeader;
        }
        else
        {
            if( HeaderFooterHelper::isFirstPageFooter( mxModel ) )
                return word::WdSeekView::wdSeekFirstPageFooter;
            else if( HeaderFooterHelper::isEvenPagesFooter( mxModel ) )
                return word::WdSeekView::wdSeekEvenPagesFooter;
            else
                return word::WdSeekView::wdSeekPrimaryFooter;
        }
    }
    else if ( aImplName == "SwXFootnote" )
    {
        if( xServiceInfo->supportsService(u"com.sun.star.text.Endnote"_ustr) )
            return word::WdSeekView::wdSeekEndnotes;
        else
            return word::WdSeekView::wdSeekFootnotes;
    }

    return word::WdSeekView::wdSeekMainDocument;
}

void SAL_CALL
SwVbaView::setSeekView( ::sal_Int32 _seekview )
{
    // FIXME: save the current cursor position, if the cursor is in the main
    // document, so we can jump back to this position, if the macro sets
    // the ViewMode back to wdSeekMainDocument

    word::gotoSelectedObjectAnchor( mxModel );
    switch( _seekview )
    {
        case word::WdSeekView::wdSeekFirstPageFooter:
        case word::WdSeekView::wdSeekFirstPageHeader:
        case word::WdSeekView::wdSeekCurrentPageFooter:
        case word::WdSeekView::wdSeekCurrentPageHeader:
        case word::WdSeekView::wdSeekPrimaryFooter:
        case word::WdSeekView::wdSeekPrimaryHeader:
        case word::WdSeekView::wdSeekEvenPagesFooter:
        case word::WdSeekView::wdSeekEvenPagesHeader:
        {
            // need to test
            mxViewCursor->gotoRange( getHFTextRange( _seekview ), false );
            break;
        }
        case word::WdSeekView::wdSeekFootnotes:
        {
            uno::Reference< text::XFootnotesSupplier > xFootnotesSupp( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< container::XIndexAccess > xFootnotes( xFootnotesSupp->getFootnotes(), uno::UNO_SET_THROW );
            if( xFootnotes->getCount() > 0 )
            {
                uno::Reference< text::XText > xText( xFootnotes->getByIndex(0), uno::UNO_QUERY_THROW );
                mxViewCursor->gotoRange( xText->getStart(), false );
            }
            else
            {
                DebugHelper::runtimeexception( ERRCODE_BASIC_NO_ACTIVE_OBJECT );
            }
            break;
        }
        case word::WdSeekView::wdSeekEndnotes:
        {
            uno::Reference< text::XEndnotesSupplier > xEndnotesSupp( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< container::XIndexAccess > xEndnotes( xEndnotesSupp->getEndnotes(), uno::UNO_SET_THROW );
            if( xEndnotes->getCount() > 0 )
            {
                uno::Reference< text::XText > xText( xEndnotes->getByIndex(0), uno::UNO_QUERY_THROW );
                mxViewCursor->gotoRange( xText->getStart(), false );
            }
            else
            {
                DebugHelper::runtimeexception( ERRCODE_BASIC_NO_ACTIVE_OBJECT );
            }
            break;
        }
        case word::WdSeekView::wdSeekMainDocument:
        {
            uno::Reference< text::XTextDocument > xTextDocument( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< text::XText > xText = xTextDocument->getText();
            mxViewCursor->gotoRange( word::getFirstObjectPosition( xText ), false );
            break;
        }
    }
}

::sal_Int32 SAL_CALL
SwVbaView::getSplitSpecial()
{
    return word::WdSpecialPane::wdPaneNone;
}

void SAL_CALL
SwVbaView::setSplitSpecial( ::sal_Int32/* _splitspecial */)
{
    // not support in Writer
}

sal_Bool SAL_CALL
SwVbaView::getTableGridLines()
{
    bool bShowTableGridLine = false;
    mxViewSettings->getPropertyValue(u"ShowTableBoundaries"_ustr) >>= bShowTableGridLine;
    return bShowTableGridLine;
}

void SAL_CALL
SwVbaView::setTableGridLines( sal_Bool _tablegridlines )
{
    mxViewSettings->setPropertyValue(u"ShowTableBoundaries"_ustr, uno::Any( _tablegridlines ) );
}

::sal_Int32 SAL_CALL
SwVbaView::getType()
{
    // FIXME: handle wdPrintPreview type
    bool bOnlineLayout = false;
    mxViewSettings->getPropertyValue(u"ShowOnlineLayout"_ustr) >>= bOnlineLayout;
    return bOnlineLayout ? word::WdViewType::wdWebView : word::WdViewType::wdPrintView;
}

void SAL_CALL
SwVbaView::setType( ::sal_Int32 _type )
{
    // FIXME: handle wdPrintPreview type
    switch( _type )
    {
        case word::WdViewType::wdPrintView:
        case word::WdViewType::wdNormalView:
        {
            mxViewSettings->setPropertyValue(u"ShowOnlineLayout"_ustr, uno::Any( false ) );
            break;
        }
        case word::WdViewType::wdWebView:
        {
            mxViewSettings->setPropertyValue(u"ShowOnlineLayout"_ustr, uno::Any( true ) );
            break;
        }
        case word::WdViewType::wdPrintPreview:
        {
            PrintPreviewHelper( uno::Any(),word::getView( mxModel ) );
            break;
        }
        default:
            DebugHelper::runtimeexception( ERRCODE_BASIC_NOT_IMPLEMENTED );

    }
}

uno::Reference< text::XTextRange > SwVbaView::getHFTextRange( sal_Int32 nType )
{
    mxModel->lockControllers();

    OUString aPropIsOn;
    OUString aPropIsShared;
    OUString aPropBodyDistance;
    OUString aPropText;

    switch( nType )
    {
        case word::WdSeekView::wdSeekCurrentPageFooter:
        case word::WdSeekView::wdSeekFirstPageFooter:
        case word::WdSeekView::wdSeekPrimaryFooter:
        case word::WdSeekView::wdSeekEvenPagesFooter:
        {
            aPropIsOn = "FooterIsOn";
            aPropIsShared = "FooterIsShared";
            aPropBodyDistance = "FooterBodyDistance";
            aPropText = "FooterText";
            break;
        }
        case word::WdSeekView::wdSeekCurrentPageHeader:
        case word::WdSeekView::wdSeekFirstPageHeader:
        case word::WdSeekView::wdSeekPrimaryHeader:
        case word::WdSeekView::wdSeekEvenPagesHeader:
        {
            aPropIsOn = "HeaderIsOn";
            aPropIsShared = "HeaderIsShared";
            aPropBodyDistance = "HeaderBodyDistance";
            aPropText = "HeaderText";
            break;
        }
    }

    uno::Reference< text::XPageCursor > xPageCursor( mxViewCursor, uno::UNO_QUERY_THROW );

    if( nType == word::WdSeekView::wdSeekFirstPageFooter
        || nType == word::WdSeekView::wdSeekFirstPageHeader )
    {
        xPageCursor->jumpToFirstPage();
    }

    uno::Reference< style::XStyle > xStyle;
    uno::Reference< text::XText > xText;
    switch( nType )
    {
        case word::WdSeekView::wdSeekPrimaryFooter:
        case word::WdSeekView::wdSeekPrimaryHeader:
        case word::WdSeekView::wdSeekEvenPagesFooter:
        case word::WdSeekView::wdSeekEvenPagesHeader:
        {
            // The primary header is the first header of the section.
            // If the header is not shared between odd and even pages
            // the odd page's header is the primary header. If the
            // first page's header is different from the rest of the
            // document, it is NOT the primary header ( the next primary
            // header would be on page 3 )
            // The even pages' header is only available if the header is
            // not shared and the current style is applied to a page with
            // an even page number
            uno::Reference< beans::XPropertySet > xCursorProps( mxViewCursor, uno::UNO_QUERY_THROW );
            OUString aPageStyleName;
            xCursorProps->getPropertyValue(u"PageStyleName"_ustr) >>= aPageStyleName;
            if ( aPageStyleName == "First Page" )
            {
                // go to the beginning of where the next style is used
                bool hasNextPage = false;
                xStyle = word::getCurrentPageStyle( mxModel );
                do
                {
                    hasNextPage = xPageCursor->jumpToNextPage();
                }
                while( hasNextPage && ( xStyle == word::getCurrentPageStyle( mxModel ) ) );

                if( !hasNextPage )
                    DebugHelper::basicexception( ERRCODE_BASIC_BAD_ACTION, {} );
            }
            break;
        }
        default:
        {
            break;
        }
    }

    xStyle = word::getCurrentPageStyle( mxModel );
    uno::Reference< beans::XPropertySet > xPageProps( xStyle, uno::UNO_QUERY_THROW );
    bool isOn = false;
    xPageProps->getPropertyValue( aPropIsOn ) >>= isOn;
    bool isShared =  false;
    xPageProps->getPropertyValue( aPropIsShared ) >>= isShared;
    if( !isOn )
    {
        xPageProps->setPropertyValue( aPropIsOn, uno::Any( true ) );
        xPageProps->setPropertyValue( aPropBodyDistance, uno::Any( DEFAULT_BODY_DISTANCE ) );
    }
    if( !isShared )
    {
        OUString aTempPropText = aPropText;
        if( nType == word::WdSeekView::wdSeekEvenPagesFooter
            || nType == word::WdSeekView::wdSeekEvenPagesHeader )
        {
            aTempPropText += "Left";
        }
        else
        {
            aTempPropText += "Right";
        }
        xText.set( xPageProps->getPropertyValue( aTempPropText), uno::UNO_QUERY_THROW );
    }
    else
    {
        if( nType == word::WdSeekView::wdSeekEvenPagesFooter
            || nType == word::WdSeekView::wdSeekEvenPagesHeader )
        {
            DebugHelper::basicexception( ERRCODE_BASIC_BAD_ACTION, {} );
        }
        xText.set( xPageProps->getPropertyValue( aPropText ), uno::UNO_QUERY_THROW );
    }

    mxModel->unlockControllers();
    if( !xText.is() )
    {
        DebugHelper::basicexception( ERRCODE_BASIC_INTERNAL_ERROR, {} );
    }
    uno::Reference< text::XTextRange > xTextRange = word::getFirstObjectPosition( xText );
    return xTextRange;
}

OUString
SwVbaView::getServiceImplName()
{
    return u"SwVbaView"_ustr;
}

uno::Sequence< OUString >
SwVbaView::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.View"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
