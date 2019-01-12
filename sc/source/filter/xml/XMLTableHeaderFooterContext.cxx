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
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlimp.hxx>
#include "XMLTableHeaderFooterContext.hxx"
#include <xmloff/xmltoken.hxx>
#include <comphelper/extract.hxx>

#include <unonames.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace xmloff::token;


XMLTableHeaderFooterContext::XMLTableHeaderFooterContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                       const OUString& rLName,
                       const uno::Reference<
                            xml::sax::XAttributeList > & xAttrList,
                       const Reference < XPropertySet > & rPageStylePropSet,
                       bool bFooter, bool bLeft ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xPropSet( rPageStylePropSet ),
    sOn( bFooter ? OUString(SC_UNO_PAGE_FTRON) : OUString(SC_UNO_PAGE_HDRON) ),
    bContainsLeft(false),
    bContainsRight(false),
    bContainsCenter(false)
{
    OUString sContent( bFooter ? OUString(SC_UNO_PAGE_RIGHTFTRCON) : OUString(SC_UNO_PAGE_RIGHTHDRCON) );
    OUString sContentLeft( bFooter ? OUString(SC_UNO_PAGE_LEFTFTRCONT) : OUString(SC_UNO_PAGE_LEFTHDRCONT) );
    OUString sShareContent( bFooter ? OUString(SC_UNO_PAGE_FTRSHARED) : OUString(SC_UNO_PAGE_HDRSHARED) );
    bool bDisplay( true );
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& rAttrName(xAttrList->getNameByIndex( i ));
        OUString aLName;
        sal_uInt16 nPrefix(GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLName ));
        const OUString& rValue(xAttrList->getValueByIndex( i ));

        // TODO: use a map here
        if( XML_NAMESPACE_STYLE == nPrefix )
        {
            if( IsXMLToken(aLName, XML_DISPLAY ) )
                bDisplay = IsXMLToken(rValue, XML_TRUE);
        }
    }
    if( bLeft )
    {
        bool bOn(::cppu::any2bool(xPropSet->getPropertyValue( sOn )));

        if( bOn && bDisplay )
        {
            if( ::cppu::any2bool(xPropSet->getPropertyValue( sShareContent )) )
                // Don't share headers any longer
                xPropSet->setPropertyValue( sShareContent, uno::makeAny(false) );
        }
        else
        {
            if( !::cppu::any2bool(xPropSet->getPropertyValue( sShareContent )) )
                // share headers
                xPropSet->setPropertyValue( sShareContent, uno::makeAny(true) );
        }
    }
    else
    {
        bool bOn(::cppu::any2bool(xPropSet->getPropertyValue( sOn )));
        if ( bOn != bDisplay )
            xPropSet->setPropertyValue( sOn, uno::makeAny(bDisplay) );
    }
    if (bLeft)
        sCont = sContentLeft;
    else
        sCont = sContent;
    xPropSet->getPropertyValue( sCont ) >>= xHeaderFooterContent;
}

XMLTableHeaderFooterContext::~XMLTableHeaderFooterContext()
{
}

SvXMLImportContextRef XMLTableHeaderFooterContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if ((nPrefix == XML_NAMESPACE_TEXT) &&
        IsXMLToken(rLocalName, XML_P))
    {
        if (!xTextCursor.is())
        {
            if( xHeaderFooterContent.is() )
            {
                uno::Reference < text::XText > xText(xHeaderFooterContent->getCenterText());
                xText->setString("");
                xTextCursor.set(xText->createTextCursor());
                xOldTextCursor.set(GetImport().GetTextImport()->GetCursor());
                GetImport().GetTextImport()->SetCursor( xTextCursor );
                bContainsCenter = true;
            }
        }
        pContext =
            GetImport().GetTextImport()->CreateTextChildContext(GetImport(),
                                                                    nPrefix,
                                                                    rLocalName,
                                                                    xAttrList);
    }
    else
    {
        if (nPrefix == XML_NAMESPACE_STYLE)
        {
            if (xHeaderFooterContent.is())
            {
                uno::Reference < text::XText > xText;
                if (IsXMLToken(rLocalName, XML_REGION_LEFT ))
                {
                    xText.set(xHeaderFooterContent->getLeftText());
                    bContainsLeft = true;
                }
                else if (IsXMLToken(rLocalName, XML_REGION_CENTER ))
                {
                    xText.set(xHeaderFooterContent->getCenterText());
                    bContainsCenter = true;
                }
                else if (IsXMLToken(rLocalName, XML_REGION_RIGHT ))
                {
                    xText.set(xHeaderFooterContent->getRightText());
                    bContainsRight = true;
                }
                if (xText.is())
                {
                    xText->setString("");
                    //SvXMLImport aSvXMLImport( GetImport() );
                    uno::Reference < text::XTextCursor > xTempTextCursor(xText->createTextCursor());
                    pContext = new XMLHeaderFooterRegionContext( GetImport(), nPrefix, rLocalName, xTempTextCursor);
                }
            }
        }
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLTableHeaderFooterContext::EndElement()
{
    if( GetImport().GetTextImport()->GetCursor().is() )
    {
        //GetImport().GetTextImport()->GetCursor()->gotoEnd(sal_False);
        if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, true ) )
        {
            GetImport().GetTextImport()->GetText()->insertString(
                GetImport().GetTextImport()->GetCursorAsRange(), "",
                true );
        }
        GetImport().GetTextImport()->ResetCursor();
    }
    if (xOldTextCursor.is())
        GetImport().GetTextImport()->SetCursor(xOldTextCursor);
    if (xHeaderFooterContent.is())
    {
        if (!bContainsLeft)
            xHeaderFooterContent->getLeftText()->setString("");
        if (!bContainsCenter)
            xHeaderFooterContent->getCenterText()->setString("");
        if (!bContainsRight)
            xHeaderFooterContent->getRightText()->setString("");

        xPropSet->setPropertyValue( sCont, uno::makeAny(xHeaderFooterContent) );
    }
}


XMLHeaderFooterRegionContext::XMLHeaderFooterRegionContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                       const OUString& rLName,
                       uno::Reference< text::XTextCursor >& xCursor ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xTextCursor ( xCursor )
{
    xOldTextCursor.set(GetImport().GetTextImport()->GetCursor());
    GetImport().GetTextImport()->SetCursor( xTextCursor );
}

XMLHeaderFooterRegionContext::~XMLHeaderFooterRegionContext()
{
}

SvXMLImportContextRef XMLHeaderFooterRegionContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if ((nPrefix == XML_NAMESPACE_TEXT) &&
        IsXMLToken(rLocalName, XML_P))
    {
        pContext =
            GetImport().GetTextImport()->CreateTextChildContext(GetImport(),
                                                                    nPrefix,
                                                                    rLocalName,
                                                                    xAttrList);
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLHeaderFooterRegionContext::EndElement()
{
    if( GetImport().GetTextImport()->GetCursor().is() )
    {
        //GetImport().GetTextImport()->GetCursor()->gotoEnd(sal_False);
        if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, true ) )
        {
            GetImport().GetTextImport()->GetText()->insertString(
                GetImport().GetTextImport()->GetCursorAsRange(), "",
                true );
        }
        GetImport().GetTextImport()->ResetCursor();
    }
    if (xOldTextCursor.is())
        GetImport().GetTextImport()->SetCursor(xOldTextCursor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
