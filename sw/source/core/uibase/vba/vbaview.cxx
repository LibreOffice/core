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
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
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

static const sal_Int32 DEFAULT_BODY_DISTANCE = 500;

SwVbaView::SwVbaView( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext,
    const uno::Reference< frame::XModel >& rModel ) throw ( uno::RuntimeException ) :
    SwVbaView_BASE( rParent, rContext ), mxModel( rModel )
{
    uno::Reference< frame::XController > xController = mxModel->getCurrentController();

    uno::Reference< text::XTextViewCursorSupplier > xTextViewCursorSupp( xController, uno::UNO_QUERY_THROW );
    mxViewCursor = xTextViewCursorSupp->getViewCursor();

    uno::Reference< view::XViewSettingsSupplier > xViewSettingSupp( xController, uno::UNO_QUERY_THROW );
    mxViewSettings.set( xViewSettingSupp->getViewSettings(), uno::UNO_QUERY_THROW );
}

SwVbaView::~SwVbaView()
{
}

::sal_Int32 SAL_CALL
SwVbaView::getSeekView() throw (css::uno::RuntimeException, std::exception)
{
    // FIXME: if the view cursor is in table, field, section and frame
    // handle if the cursor is in table
    uno::Reference< text::XText > xCurrentText = mxViewCursor->getText();
    uno::Reference< beans::XPropertySet > xCursorProps( mxViewCursor, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextContent > xTextContent;
    while( xCursorProps->getPropertyValue("TextTable") >>= xTextContent )
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
        if( xServiceInfo->supportsService("com.sun.star.text.Endnote") )
            return word::WdSeekView::wdSeekEndnotes;
        else
            return word::WdSeekView::wdSeekFootnotes;
    }

    return word::WdSeekView::wdSeekMainDocument;
}

void SAL_CALL
SwVbaView::setSeekView( ::sal_Int32 _seekview ) throw (css::uno::RuntimeException, std::exception)
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
            mxViewCursor->gotoRange( getHFTextRange( _seekview ), sal_False );
            break;
        }
        case word::WdSeekView::wdSeekFootnotes:
        {
            uno::Reference< text::XFootnotesSupplier > xFootnotesSupp( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< container::XIndexAccess > xFootnotes( xFootnotesSupp->getFootnotes(), uno::UNO_QUERY_THROW );
            if( xFootnotes->getCount() > 0 )
            {
                uno::Reference< text::XText > xText( xFootnotes->getByIndex(0), uno::UNO_QUERY_THROW );
                mxViewCursor->gotoRange( xText->getStart(), sal_False );
            }
            else
            {
                DebugHelper::exception( SbERR_NO_ACTIVE_OBJECT, OUString() );
            }
            break;
        }
        case word::WdSeekView::wdSeekEndnotes:
        {
            uno::Reference< text::XEndnotesSupplier > xEndnotesSupp( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< container::XIndexAccess > xEndnotes( xEndnotesSupp->getEndnotes(), uno::UNO_QUERY_THROW );
            if( xEndnotes->getCount() > 0 )
            {
                uno::Reference< text::XText > xText( xEndnotes->getByIndex(0), uno::UNO_QUERY_THROW );
                mxViewCursor->gotoRange( xText->getStart(), sal_False );
            }
            else
            {
                DebugHelper::exception( SbERR_NO_ACTIVE_OBJECT, OUString() );
            }
            break;
        }
        case word::WdSeekView::wdSeekMainDocument:
        {
            uno::Reference< text::XTextDocument > xTextDocument( mxModel, uno::UNO_QUERY_THROW );
            uno::Reference< text::XText > xText = xTextDocument->getText();
            mxViewCursor->gotoRange( word::getFirstObjectPosition( xText ), sal_False );
            break;
        }
    }
}

::sal_Int32 SAL_CALL
SwVbaView::getSplitSpecial() throw (css::uno::RuntimeException, std::exception)
{
    return word::WdSpecialPane::wdPaneNone;
}

void SAL_CALL
SwVbaView::setSplitSpecial( ::sal_Int32/* _splitspecial */) throw (css::uno::RuntimeException, std::exception)
{
    // not support in Writer
}

::sal_Bool SAL_CALL
SwVbaView::getTableGridLines() throw (css::uno::RuntimeException, std::exception)
{
    sal_Bool bShowTableGridLine = sal_False;
    mxViewSettings->getPropertyValue("ShowTableBoundaries") >>= bShowTableGridLine;
    return bShowTableGridLine;
}

void SAL_CALL
SwVbaView::setTableGridLines( ::sal_Bool _tablegridlines ) throw (css::uno::RuntimeException, std::exception)
{
    mxViewSettings->setPropertyValue("ShowTableBoundaries", uno::makeAny( _tablegridlines ) );
}

::sal_Int32 SAL_CALL
SwVbaView::getType() throw (css::uno::RuntimeException, std::exception)
{
    // FIXME: handle wdPrintPreview type
    sal_Bool bOnlineLayout = sal_False;
    mxViewSettings->getPropertyValue("ShowOnlineLayout") >>= bOnlineLayout;
    return bOnlineLayout ? word::WdViewType::wdWebView : word::WdViewType::wdPrintView;
}

void SAL_CALL
SwVbaView::setType( ::sal_Int32 _type ) throw (css::uno::RuntimeException, std::exception)
{
    // FIXME: handle wdPrintPreview type
    switch( _type )
    {
        case word::WdViewType::wdPrintView:
        case word::WdViewType::wdNormalView:
        {
            mxViewSettings->setPropertyValue("ShowOnlineLayout", uno::makeAny( sal_False ) );
            break;
        }
        case word::WdViewType::wdWebView:
        {
            mxViewSettings->setPropertyValue("ShowOnlineLayout", uno::makeAny( sal_True ) );
            break;
        }
        case word::WdViewType::wdPrintPreview:
        {
            PrintPreviewHelper( uno::Any(),word::getView( mxModel ) );
            break;
        }
        default:
            DebugHelper::exception( SbERR_NOT_IMPLEMENTED, OUString() );

    }
}

uno::Reference< text::XTextRange > SwVbaView::getHFTextRange( sal_Int32 nType ) throw (uno::RuntimeException)
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
            xCursorProps->getPropertyValue("PageStyleName") >>= aPageStyleName;
            if ( aPageStyleName == "First Page" )
            {
                // go to the beginning of where the next style is used
                sal_Bool hasNextPage = sal_False;
                xStyle = word::getCurrentPageStyle( mxModel );
                do
                {
                    hasNextPage = xPageCursor->jumpToNextPage();
                }
                while( hasNextPage && ( xStyle == word::getCurrentPageStyle( mxModel ) ) );

                if( !hasNextPage )
                    DebugHelper::exception( SbERR_BAD_ACTION, OUString() );
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
    sal_Bool isOn = sal_False;
    xPageProps->getPropertyValue( aPropIsOn ) >>= isOn;
    sal_Bool isShared =  sal_False;
    xPageProps->getPropertyValue( aPropIsShared ) >>= isShared;
    if( !isOn )
    {
        xPageProps->setPropertyValue( aPropIsOn, uno::makeAny( sal_True ) );
        xPageProps->setPropertyValue( aPropBodyDistance, uno::makeAny( DEFAULT_BODY_DISTANCE ) );
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
            DebugHelper::exception( SbERR_BAD_ACTION, OUString() );
        }
        xText.set( xPageProps->getPropertyValue( aPropText ), uno::UNO_QUERY_THROW );
    }

    mxModel->unlockControllers();
    if( !xText.is() )
    {
        DebugHelper::exception( SbERR_INTERNAL_ERROR, OUString() );
    }
    uno::Reference< text::XTextRange > xTextRange = word::getFirstObjectPosition( xText );
    return xTextRange;
}

OUString
SwVbaView::getServiceImplName()
{
    return OUString("SwVbaView");
}

uno::Sequence< OUString >
SwVbaView::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.View";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
