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
SwVbaView::getSeekView() throw (css::uno::RuntimeException)
{
    // FIXME: if the view cursor is in table, field, section and frame
    // handle if the cursor is in table
    uno::Reference< text::XText > xCurrentText = mxViewCursor->getText();
    uno::Reference< beans::XPropertySet > xCursorProps( mxViewCursor, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextContent > xTextContent;
    while( xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextTable") ) ) >>= xTextContent )
    {
        xCurrentText = xTextContent->getAnchor()->getText();
        xCursorProps.set( xCurrentText->createTextCursor(), uno::UNO_QUERY_THROW );
    }
    uno::Reference< lang::XServiceInfo > xServiceInfo( xCurrentText, uno::UNO_QUERY_THROW );
    rtl::OUString aImplName = xServiceInfo->getImplementationName();
    if( aImplName.equalsAscii("SwXBodyText") )
    {
        return word::WdSeekView::wdSeekMainDocument;
    }
    else if( aImplName.equalsAscii("SwXHeadFootText") )
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
    else if( aImplName.equalsAscii("SwXFootnote") )
    {
        if( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Endnote") ) ) )
            return word::WdSeekView::wdSeekEndnotes;
        else
            return word::WdSeekView::wdSeekFootnotes;
    }

    return word::WdSeekView::wdSeekMainDocument;
}

void SAL_CALL
SwVbaView::setSeekView( ::sal_Int32 _seekview ) throw (css::uno::RuntimeException)
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
                DebugHelper::exception( SbERR_NO_ACTIVE_OBJECT, rtl::OUString() );
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
                DebugHelper::exception( SbERR_NO_ACTIVE_OBJECT, rtl::OUString() );
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
SwVbaView::getSplitSpecial() throw (css::uno::RuntimeException)
{
    return word::WdSpecialPane::wdPaneNone;
}

void SAL_CALL
SwVbaView::setSplitSpecial( ::sal_Int32/* _splitspecial */) throw (css::uno::RuntimeException)
{
    // not support in Writer
}

::sal_Bool SAL_CALL
SwVbaView::getTableGridLines() throw (css::uno::RuntimeException)
{
    sal_Bool bShowTableGridLine = sal_False;
    mxViewSettings->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ShowTableBoundaries"))) >>= bShowTableGridLine;
    return bShowTableGridLine;
}

void SAL_CALL
SwVbaView::setTableGridLines( ::sal_Bool _tablegridlines ) throw (css::uno::RuntimeException)
{
    mxViewSettings->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ShowTableBoundaries")), uno::makeAny( _tablegridlines ) );
}

::sal_Int32 SAL_CALL
SwVbaView::getType() throw (css::uno::RuntimeException)
{
    // FIXME: handle wdPrintPreview type
    sal_Bool bOnlineLayout = sal_False;
    mxViewSettings->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ShowOnlineLayout"))) >>= bOnlineLayout;
    return bOnlineLayout ? word::WdViewType::wdWebView : word::WdViewType::wdPrintView;
}

void SAL_CALL
SwVbaView::setType( ::sal_Int32 _type ) throw (css::uno::RuntimeException)
{
    // FIXME: handle wdPrintPreview type
    switch( _type )
    {
        case word::WdViewType::wdPrintView:
        case word::WdViewType::wdNormalView:
        {
            mxViewSettings->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ShowOnlineLayout")), uno::makeAny( sal_False ) );
            break;
        }
        case word::WdViewType::wdWebView:
        {
            mxViewSettings->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ShowOnlineLayout")), uno::makeAny( sal_True ) );
            break;
        }
        case word::WdViewType::wdPrintPreview:
        {
            PrintPreviewHelper( uno::Any(),word::getView( mxModel ) );
            break;
        }
        default:
            DebugHelper::exception( SbERR_NOT_IMPLEMENTED, rtl::OUString() );

    }
}

uno::Reference< text::XTextRange > SwVbaView::getHFTextRange( sal_Int32 nType ) throw (uno::RuntimeException)
{
    mxModel->lockControllers();

    rtl::OUString aPropIsOn;
    rtl::OUString aPropIsShared;
    rtl::OUString aPropBodyDistance;
    rtl::OUString aPropText;

    switch( nType )
    {
        case word::WdSeekView::wdSeekCurrentPageFooter:
        case word::WdSeekView::wdSeekFirstPageFooter:
        case word::WdSeekView::wdSeekPrimaryFooter:
        case word::WdSeekView::wdSeekEvenPagesFooter:
        {
            aPropIsOn = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsOn") );
            aPropIsShared = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterIsShared") );
            aPropBodyDistance = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterBodyDistance") );
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FooterText") );
            break;
        }
        case word::WdSeekView::wdSeekCurrentPageHeader:
        case word::WdSeekView::wdSeekFirstPageHeader:
        case word::WdSeekView::wdSeekPrimaryHeader:
        case word::WdSeekView::wdSeekEvenPagesHeader:
        {
            aPropIsOn = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn") );
            aPropIsShared = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderIsShared") );
            aPropBodyDistance = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderBodyDistance") );
            aPropText = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HeaderText") );
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
            rtl::OUString aPageStyleName;
            xCursorProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageStyleName"))) >>= aPageStyleName;
            if( aPageStyleName.equalsAscii("First Page") )
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
                    DebugHelper::exception( SbERR_BAD_ACTION, rtl::OUString() );
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
        rtl::OUString aTempPropText = aPropText;
        if( nType == word::WdSeekView::wdSeekEvenPagesFooter
            || nType == word::WdSeekView::wdSeekEvenPagesHeader )
        {
            aTempPropText += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Left") );
        }
        else
        {
            aTempPropText += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Right") );
        }
        xText.set( xPageProps->getPropertyValue( aTempPropText), uno::UNO_QUERY_THROW );
    }
    else
    {
        if( nType == word::WdSeekView::wdSeekEvenPagesFooter
            || nType == word::WdSeekView::wdSeekEvenPagesHeader )
        {
            DebugHelper::exception( SbERR_BAD_ACTION, rtl::OUString() );
        }
        xText.set( xPageProps->getPropertyValue( aPropText ), uno::UNO_QUERY_THROW );
    }

    mxModel->unlockControllers();
    if( !xText.is() )
    {
        DebugHelper::exception( SbERR_INTERNAL_ERROR, rtl::OUString() );
    }
    uno::Reference< text::XTextRange > xTextRange = word::getFirstObjectPosition( xText );
    return xTextRange;
}

rtl::OUString&
SwVbaView::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaView") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaView::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.View" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
