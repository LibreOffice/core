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
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <vbahelper/vbahelper.hxx>
#include <unotxdoc.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <view.hxx>
#include <viewsh.hxx>
#include <comphelper/servicehelper.hxx>
#include <unostyle.hxx>
#include <unotbl.hxx>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

namespace ooo::vba::word
{

SwView* getView( const rtl::Reference< SwXTextDocument>& xModel )
{
    SwDocShell* pDocShell = xModel->GetDocShell( );
    return pDocShell? pDocShell->GetView() : nullptr;
}

uno::Reference< text::XTextViewCursor > getXTextViewCursor( const rtl::Reference< SwXTextDocument >& xModel )
{
    uno::Reference< frame::XController > xController = xModel->getCurrentController();
    uno::Reference< text::XTextViewCursorSupplier > xTextViewCursorSupp( xController, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextViewCursor > xTextViewCursor = xTextViewCursorSupp->getViewCursor();
    return xTextViewCursor;
}

rtl::Reference< SwXBaseStyle > getCurrentPageStyle( const rtl::Reference< SwXTextDocument >& xModel )
{
    uno::Reference< beans::XPropertySet > xCursorProps( getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );
    return getCurrentPageStyle( xModel, xCursorProps );
}

rtl::Reference< SwXBaseStyle > getCurrentPageStyle( const rtl::Reference< SwXTextDocument >& xModel, const uno::Reference< beans::XPropertySet >& xProps )
{
    OUString aPageStyleName;
    xProps->getPropertyValue(u"PageStyleName"_ustr) >>= aPageStyleName;
    rtl::Reference< SwXStyleFamilies > xStyleFamNames( xModel->getSwStyleFamilies() );
    rtl::Reference< SwXStyleFamily > xPageStyles( xStyleFamNames->GetPageStyles() );
    return xPageStyles->getStyleByName( aPageStyleName );
}

sal_Int32 getPageCount( const rtl::Reference<SwXTextDocument>& xModel )
{
    SwDocShell* pDocShell = xModel->GetDocShell();
    SwViewShell* pViewSh = pDocShell ? pDocShell->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell() : nullptr;
    return pViewSh ? pViewSh->GetPageCount() : 0;
}

rtl::Reference< SwXBaseStyle > getDefaultParagraphStyle( const rtl::Reference< SwXTextDocument >& xModel )
{
    rtl::Reference< SwXStyleFamilies > xStyleFamNames( xModel->getSwStyleFamilies() );
    rtl::Reference< SwXStyleFamily > xParaStyles( xStyleFamNames->GetParagraphStyles() );
    rtl::Reference< SwXBaseStyle > xStyle( xParaStyles->getStyleByName(u"Standard"_ustr) );

    return xStyle;
}

uno::Reference< text::XTextRange > getFirstObjectPosition( const uno::Reference< text::XText >& xText )
{
    // if the first object is table, get the position of first cell
    uno::Reference< text::XTextRange > xTextRange;
    uno::Reference< container::XEnumerationAccess > xParaAccess( xText, uno::UNO_QUERY_THROW );
    uno::Reference< container::XEnumeration> xParaEnum = xParaAccess->createEnumeration();
    if( xParaEnum->hasMoreElements() )
    {
        uno::Reference< lang::XServiceInfo > xServiceInfo( xParaEnum->nextElement(), uno::UNO_QUERY_THROW );
        if( xServiceInfo->supportsService(u"com.sun.star.text.TextTable"_ustr) )
        {
            rtl::Reference< SwXTextTable > xCellRange = dynamic_cast<SwXTextTable*>(xServiceInfo.get());
            rtl::Reference< SwXCell> xFirstCellText( xCellRange->getSwCellByPosition(0, 0) );
            xTextRange = xFirstCellText->getStart();
        }
    }
    if( !xTextRange.is() )
        xTextRange = xText->getStart();
    return xTextRange;
}

uno::Reference< text::XText > getCurrentXText( const rtl::Reference< SwXTextDocument >& xModel )
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
        xTextRange = xTextContent->getAnchor();

    if( !xTextRange.is() )
        xTextRange.set( getXTextViewCursor( xModel ), uno::UNO_QUERY_THROW );

    uno::Reference< text::XText > xText;
    try
    {
        xText = xTextRange->getText();
    }
    catch (const uno::RuntimeException&)
    {
        //catch exception "no text selection"
    }
    uno::Reference< beans::XPropertySet > xVCProps( xTextRange, uno::UNO_QUERY_THROW );
    while( xVCProps->getPropertyValue(u"TextTable"_ustr) >>= xTextContent )
    {
        xText = xTextContent->getAnchor()->getText();
        xVCProps.set( xText->createTextCursor(), uno::UNO_QUERY_THROW );
    }

    if( !xText.is() )
        throw  uno::RuntimeException(u"no text selection"_ustr );

    return xText;
}

bool gotoSelectedObjectAnchor( const rtl::Reference<SwXTextDocument>& xModel )
{
    bool isObjectSelected = false;
    uno::Reference< text::XTextContent > xTextContent( xModel->getCurrentSelection(), uno::UNO_QUERY );
    if( xTextContent.is() )
    {
        uno::Reference< text::XTextRange > xTextRange( xTextContent->getAnchor(), uno::UNO_SET_THROW );
        uno::Reference< view::XSelectionSupplier > xSelectSupp( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
        xSelectSupp->select( uno::Any( xTextRange ) );
        isObjectSelected = true;
    }
    return isObjectSelected;
}

rtl::Reference< SwXTextDocument > getCurrentWordDoc( const uno::Reference< uno::XComponentContext >& xContext )
{
    try
    {
        return dynamic_cast<SwXTextDocument*>(getCurrentDoc( u"ThisWordDoc"_ustr ).get());
    }
    catch (const uno::Exception&)
    {
        try
        {
            return getThisWordDoc( xContext );
        }
        catch (const uno::Exception&)
        {
        }
    }
    return {};
}

rtl::Reference< SwXTextDocument > getThisWordDoc( const uno::Reference< uno::XComponentContext >& xContext )
{
    return dynamic_cast<SwXTextDocument*>(::ooo::vba::getCurrentDocCtx( u"WordDocumentContext"_ustr , xContext ).get());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
