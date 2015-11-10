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
#include <IDocumentLayoutAccess.hxx>
#include <view.hxx>
#include <viewsh.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

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
    return pXDoc ? pXDoc->GetDocShell() : nullptr;
}

SwView* getView( const uno::Reference< frame::XModel>& xModel )
{
    SwDocShell* pDocShell = getDocShell( xModel );
    return pDocShell? pDocShell->GetView() : nullptr;
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
    OUString aPageStyleName;
    xProps->getPropertyValue("PageStyleName") >>= aPageStyleName;
    uno::Reference< style::XStyleFamiliesSupplier > xSytleFamSupp( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xSytleFamNames( xSytleFamSupp->getStyleFamilies(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xPageStyles( xSytleFamNames->getByName("PageStyles"), uno::UNO_QUERY_THROW );
    uno::Reference< style::XStyle > xStyle( xPageStyles->getByName( aPageStyleName ), uno::UNO_QUERY_THROW );

    return xStyle;
}

sal_Int32 getPageCount( const uno::Reference< frame::XModel>& xModel ) throw (uno::RuntimeException)
{
    SwDocShell* pDocShell = getDocShell( xModel );
    SwViewShell* pViewSh = pDocShell ? pDocShell->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell() : nullptr;
    return pViewSh ? pViewSh->GetPageCount() : 0;
}

uno::Reference< style::XStyle > getDefaultParagraphStyle( const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    uno::Reference< style::XStyleFamiliesSupplier > xSytleFamSupp( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xSytleFamNames( xSytleFamSupp->getStyleFamilies(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xParaStyles( xSytleFamNames->getByName("ParagraphStyles"), uno::UNO_QUERY_THROW );
    uno::Reference< style::XStyle > xStyle( xParaStyles->getByName("Standard"), uno::UNO_QUERY_THROW );

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
        if( xServiceInfo->supportsService("com.sun.star.text.TextTable") )
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
    catch (const uno::RuntimeException&)
    {
        //catch exception "no text selection"
    }
    uno::Reference< beans::XPropertySet > xVCProps( xTextRange, uno::UNO_QUERY_THROW );
    while( xVCProps->getPropertyValue("TextTable") >>= xTextContent )
    {
        xText = xTextContent->getAnchor()->getText();
        xVCProps.set( xText->createTextCursor(), uno::UNO_QUERY_THROW );
    }

    if( !xText.is() )
        throw  uno::RuntimeException("no text selection" );

    return xText;
}

bool gotoSelectedObjectAnchor( const uno::Reference< frame::XModel>& xModel ) throw (uno::RuntimeException)
{
    bool isObjectSelected = false;
    uno::Reference< text::XTextContent > xTextContent( xModel->getCurrentSelection(), uno::UNO_QUERY );
    if( xTextContent.is() )
    {
        uno::Reference< text::XTextRange > xTextRange( xTextContent->getAnchor(), uno::UNO_QUERY_THROW );
        uno::Reference< view::XSelectionSupplier > xSelectSupp( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
        xSelectSupp->select( uno::makeAny( xTextRange ) );
        isObjectSelected = true;
    }
    return isObjectSelected;
}

} // word
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
