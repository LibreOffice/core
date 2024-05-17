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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include "XMLTableHeaderFooterContext.hxx"
#include <xmloff/xmltoken.hxx>
#include <comphelper/extract.hxx>
#include <sal/log.hxx>

#include <unonames.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace xmloff::token;


XMLTableHeaderFooterContext::XMLTableHeaderFooterContext( SvXMLImport& rImport, sal_Int32 /*nElement*/,
                       const uno::Reference<
                            xml::sax::XFastAttributeList > & xAttrList,
                       const Reference < XPropertySet > & rPageStylePropSet,
                       bool bFooter, bool bLeft, bool bFirst ) :
    SvXMLImportContext( rImport ),
    xPropSet( rPageStylePropSet ),
    bContainsLeft(false),
    bContainsRight(false),
    bContainsCenter(false)
{
    OUString sOn( bFooter ? SC_UNO_PAGE_FTRON : SC_UNO_PAGE_HDRON );
    OUString sShareFirstContent( bFooter ? SC_UNO_PAGE_FIRSTFTRSHARED : SC_UNO_PAGE_FIRSTHDRSHARED );
    bool bDisplay( true );
    for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
    {
        if( aIter.getToken() == XML_ELEMENT(STYLE, XML_DISPLAY) )
            bDisplay = IsXMLToken(aIter, XML_TRUE);
        else
            XMLOFF_WARN_UNKNOWN("sc", aIter);
    }
    bool bOn(::cppu::any2bool(xPropSet->getPropertyValue( sOn )));
    if( bLeft || bFirst )
    {
        OUString sShareContent( bFooter ? SC_UNO_PAGE_FTRSHARED : SC_UNO_PAGE_HDRSHARED );
        const OUString sShare = bLeft ? sShareContent : sShareFirstContent;
        if( bOn && bDisplay )
        {
            if( ::cppu::any2bool(xPropSet->getPropertyValue( sShare )) )
                // Don't share headers any longer
                xPropSet->setPropertyValue( sShare, uno::Any(false) );
        }
        else
        {
            if( !::cppu::any2bool(xPropSet->getPropertyValue( sShare )) )
                // share headers
                xPropSet->setPropertyValue( sShare, uno::Any(true) );
        }
    }
    else
    {
        if ( bOn != bDisplay )
            xPropSet->setPropertyValue( sOn, uno::Any(bDisplay) );
    }
    if (bLeft)
    {
        sCont = bFooter ? SC_UNO_PAGE_LEFTFTRCONT : SC_UNO_PAGE_LEFTHDRCONT;
    }
    else if (bFirst)
    {
        sCont = bFooter ? SC_UNO_PAGE_FIRSTFTRCONT : SC_UNO_PAGE_FIRSTHDRCONT;
        xPropSet->setPropertyValue( sShareFirstContent, uno::Any(!bDisplay) );
    }
    else
    {
        sCont = bFooter ? SC_UNO_PAGE_RIGHTFTRCON : SC_UNO_PAGE_RIGHTHDRCON;
    }
    xPropSet->getPropertyValue( sCont ) >>= xHeaderFooterContent;
}

XMLTableHeaderFooterContext::~XMLTableHeaderFooterContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTableHeaderFooterContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if (xHeaderFooterContent.is())
    {
        uno::Reference < text::XText > xText;
        switch (nElement)
        {
            case XML_ELEMENT(STYLE, XML_REGION_LEFT):
                xText.set(xHeaderFooterContent->getLeftText());
                bContainsLeft = true;
                break;
            case XML_ELEMENT(STYLE, XML_REGION_CENTER):
                xText.set(xHeaderFooterContent->getCenterText());
                bContainsCenter = true;
                break;
            case XML_ELEMENT(STYLE, XML_REGION_RIGHT):
                xText.set(xHeaderFooterContent->getRightText());
                bContainsRight = true;
                break;
            default: break;
        }
        if (xText.is())
        {
            xText->setString(u""_ustr);
            uno::Reference < text::XTextCursor > xTempTextCursor(xText->createTextCursor());
            return new XMLHeaderFooterRegionContext( GetImport(), xTempTextCursor);
        }
    }

    if ( nElement == XML_ELEMENT(TEXT, XML_P) )
    {
        if (!xTextCursor.is())
        {
            if( xHeaderFooterContent.is() )
            {
                uno::Reference < text::XText > xText(xHeaderFooterContent->getCenterText());
                xText->setString(u""_ustr);
                xTextCursor.set(xText->createTextCursor());
                xOldTextCursor.set(GetImport().GetTextImport()->GetCursor());
                GetImport().GetTextImport()->SetCursor( xTextCursor );
                bContainsCenter = true;
            }
        }
        return
            GetImport().GetTextImport()->CreateTextChildContext(GetImport(),
                                                                    nElement,
                                                                    xAttrList);
    }

    XMLOFF_WARN_UNKNOWN_ELEMENT("sc", nElement);
    return nullptr;
}

void XMLTableHeaderFooterContext::endFastElement(sal_Int32 )
{
    if( GetImport().GetTextImport()->GetCursor().is() )
    {
        //GetImport().GetTextImport()->GetCursor()->gotoEnd(sal_False);
        if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, true ) )
        {
            GetImport().GetTextImport()->GetText()->insertString(
                GetImport().GetTextImport()->GetCursorAsRange(), u""_ustr,
                true );
        }
        GetImport().GetTextImport()->ResetCursor();
    }
    if (xOldTextCursor.is())
        GetImport().GetTextImport()->SetCursor(xOldTextCursor);
    if (xHeaderFooterContent.is())
    {
        if (!bContainsLeft)
            xHeaderFooterContent->getLeftText()->setString(u""_ustr);
        if (!bContainsCenter)
            xHeaderFooterContent->getCenterText()->setString(u""_ustr);
        if (!bContainsRight)
            xHeaderFooterContent->getRightText()->setString(u""_ustr);

        xPropSet->setPropertyValue( sCont, uno::Any(xHeaderFooterContent) );
    }
}


XMLHeaderFooterRegionContext::XMLHeaderFooterRegionContext( SvXMLImport& rImport,
                       uno::Reference< text::XTextCursor >& xCursor ) :
    SvXMLImportContext( rImport ),
    xTextCursor ( xCursor )
{
    xOldTextCursor.set(GetImport().GetTextImport()->GetCursor());
    GetImport().GetTextImport()->SetCursor( xTextCursor );
}

XMLHeaderFooterRegionContext::~XMLHeaderFooterRegionContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLHeaderFooterRegionContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if (nElement == XML_ELEMENT(TEXT, XML_P))
    {
        return GetImport().GetTextImport()->CreateTextChildContext(GetImport(),
                                                                    nElement,
                                                                    xAttrList);
    }
    XMLOFF_WARN_UNKNOWN_ELEMENT("sc", nElement);
    return pContext;
}

void XMLHeaderFooterRegionContext::endFastElement(sal_Int32 )
{
    if( GetImport().GetTextImport()->GetCursor().is() )
    {
        //GetImport().GetTextImport()->GetCursor()->gotoEnd(sal_False);
        if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, true ) )
        {
            GetImport().GetTextImport()->GetText()->insertString(
                GetImport().GetTextImport()->GetCursorAsRange(), u""_ustr,
                true );
        }
        GetImport().GetTextImport()->ResetCursor();
    }
    if (xOldTextCursor.is())
        GetImport().GetTextImport()->SetCursor(xOldTextCursor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
