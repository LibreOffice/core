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
#include <docsh.hxx>
#include "wordvbahelper.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <unotxdoc.hxx>
#include <doc.hxx>
#include <view.hxx>
#include <viewsh.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

#define FIRST_PAGE 1;

namespace ooo
{
namespace vba
{
namespace word
{

SwDocShell* getDocShell( const uno::Reference< frame::XModel>& xModel )
{
    uno::Reference< lang::XUnoTunnel > xTunnel( xModel, uno::UNO_QUERY_THROW );
    SwXTextDocument* pXDoc = reinterpret_cast< SwXTextDocument * >( sal::static_int_cast< sal_IntPtr >(xTunnel->getSomething(SwXTextDocument::getUnoTunnelId())));
    return pXDoc ? pXDoc->GetDocShell() : 0;
}

SwView* getView( const uno::Reference< frame::XModel>& xModel )
{
    SwDocShell* pDocShell = getDocShell( xModel );
    return pDocShell? pDocShell->GetView() : 0;
}

uno::Reference< text::XTextViewCursor > getXTextViewCursor( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    uno::Reference< frame::XController > xController = xModel->getCurrentController();
    uno::Reference< text::XTextViewCursorSupplier > xTextViewCursorSupp( xController, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextViewCursor > xTextViewCursor = xTextViewCursorSupp->getViewCursor();
    return xTextViewCursor;
}

uno::Reference< style::XStyle > getCurrentPageStyle( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xCursorProps( getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
    return getCurrentPageStyle( xModel, xCursorProps );
}

uno::Reference< style::XStyle > getCurrentPageStyle( const uno::Reference< frame::XModel >& xModel, const uno::Reference< beans::XPropertySet >& xProps ) throw (uno::RuntimeException)
{
    rtl::OUString aPageStyleName;
    xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageStyleName"))) >>= aPageStyleName;
    uno::Reference< style::XStyleFamiliesSupplier > xSytleFamSupp( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xSytleFamNames( xSytleFamSupp->getStyleFamilies(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xPageStyles( xSytleFamNames->getByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PageStyles") ) ), uno::UNO_QUERY_THROW );
    uno::Reference< style::XStyle > xStyle( xPageStyles->getByName( aPageStyleName ), uno::UNO_QUERY_THROW );

    return xStyle;
}

sal_Int32 getPageCount( const uno::Reference< frame::XModel>& xModel ) throw (uno::RuntimeException)
{
    SwDocShell* pDocShell = getDocShell( xModel );
    ViewShell* pViewSh = pDocShell ? pDocShell->GetDoc()->GetCurrentViewShell() : 0;
    return pViewSh ? pViewSh->GetPageCount() : 0;
}

uno::Reference< style::XStyle > getDefaultParagraphStyle( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    uno::Reference< style::XStyleFamiliesSupplier > xSytleFamSupp( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xSytleFamNames( xSytleFamSupp->getStyleFamilies(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xParaStyles( xSytleFamNames->getByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ParagraphStyles") ) ), uno::UNO_QUERY_THROW );
    uno::Reference< style::XStyle > xStyle( xParaStyles->getByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Standard") ) ), uno::UNO_QUERY_THROW );

    return xStyle;
}

uno::Reference< text::XTextRange > getFirstObjectPosition( const uno::Reference< text::XText >& xText ) throw (uno::RuntimeException)
{
    // if the first object is table, get the position of first cell
    uno::Reference< text::XTextRange > xTextRange;
    uno::Reference< container::XEnumerationAccess > xParaAccess( xText, uno::UNO_QUERY_THROW );
    uno::Reference< container::XEnumeration> xParaEnum = xParaAccess->createEnumeration();
    if( xParaEnum->hasMoreElements() )
    {
        uno::Reference< lang::XServiceInfo > xServiceInfo( xParaEnum->nextElement(), uno::UNO_QUERY_THROW );
        if( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextTable") ) ) )
        {
            uno::Reference< table::XCellRange > xCellRange( xServiceInfo, uno::UNO_QUERY_THROW );
            uno::Reference< text::XText> xFirstCellText( xCellRange->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
            xTextRange = xFirstCellText->getStart();
        }
    }
    if( !xTextRange.is() )
        xTextRange = xText->getStart();
    return xTextRange;
}

uno::Reference< text::XText > getCurrentXText( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    uno::Reference< text::XTextRange > xTextRange;
    uno::Reference< text::XTextContent > xTextContent( xModel->getCurrentSelection(), uno::UNO_QUERY );
    if( !xTextContent.is() )
    {
        uno::Reference< container::XIndexAccess > xIndexAccess( xModel->getCurrentSelection(), uno::UNO_QUERY );
        if( xIndexAccess.is() )
        {
            xTextContent.set( xIndexAccess->getByIndex(0), uno::UNO_QUERY );
        }
    }

    if( xTextContent.is() )
        xTextRange.set( xTextContent->getAnchor(), uno::UNO_QUERY );

    if( !xTextRange.is() )
        xTextRange.set( getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );

    uno::Reference< text::XText > xText;
    try
    {
        xText.set( xTextRange->getText(), uno::UNO_QUERY );
    }
    catch( uno::RuntimeException& )
    {
        //catch exception "no text selection"
    }
    uno::Reference< beans::XPropertySet > xVCProps( xTextRange, uno::UNO_QUERY_THROW );
    while( xVCProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TextTable") ) ) >>= xTextContent )
    {
        xText = xTextContent->getAnchor()->getText();
        xVCProps.set( xText->createTextCursor(), uno::UNO_QUERY_THROW );
    }

    if( !xText.is() )
        throw  uno::RuntimeException( rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "no text selection" ) ), uno::Reference< uno::XInterface >() );

    return xText;
}

sal_Bool gotoSelectedObjectAnchor( const uno::Reference< frame::XModel>& xModel ) throw (uno::RuntimeException)
{
    sal_Bool isObjectSelected = sal_False;
    uno::Reference< text::XTextContent > xTextContent( xModel->getCurrentSelection(), uno::UNO_QUERY );
    if( xTextContent.is() )
    {
        uno::Reference< text::XTextRange > xTextRange( xTextContent->getAnchor(), uno::UNO_QUERY_THROW );
        uno::Reference< view::XSelectionSupplier > xSelectSupp( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
        xSelectSupp->select( uno::makeAny( xTextRange ) );
        isObjectSelected = sal_True;
    }
    return isObjectSelected;
}

} // word
} //
} //

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
