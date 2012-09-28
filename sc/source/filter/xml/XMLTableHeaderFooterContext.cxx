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

#include <com/sun/star/text/XText.hpp>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include "XMLTableHeaderFooterContext.hxx"
#include <xmloff/xmltoken.hxx>
#include <comphelper/extract.hxx>

#include "unonames.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace xmloff::token;

using rtl::OUString;

TYPEINIT1( XMLTableHeaderFooterContext, SvXMLImportContext );

XMLTableHeaderFooterContext::XMLTableHeaderFooterContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                       const OUString& rLName,
                       const uno::Reference<
                            xml::sax::XAttributeList > & xAttrList,
                       const Reference < XPropertySet > & rPageStylePropSet,
                       bool bFooter, bool bLft ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xPropSet( rPageStylePropSet ),
    sOn( bFooter ? OUString(SC_UNO_PAGE_FTRON) : OUString(SC_UNO_PAGE_HDRON) ),
    sShareContent( bFooter ? OUString(SC_UNO_PAGE_FTRSHARED) : OUString(SC_UNO_PAGE_HDRSHARED) ),
    sContent( bFooter ? OUString(SC_UNO_PAGE_RIGHTFTRCON) : OUString(SC_UNO_PAGE_RIGHTHDRCON) ),
    sContentLeft( bFooter ? OUString(SC_UNO_PAGE_LEFTFTRCONT) : OUString(SC_UNO_PAGE_LEFTHDRCONT) ),
    bDisplay( true ),
    bLeft( bLft ),
    bContainsLeft(false),
    bContainsRight(false),
    bContainsCenter(false)
{
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
                xPropSet->setPropertyValue( sShareContent, uno::makeAny(sal_True) );
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

SvXMLImportContext *XMLTableHeaderFooterContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext(0);

    if ((nPrefix == XML_NAMESPACE_TEXT) &&
        IsXMLToken(rLocalName, XML_P))
    {
        if (!xTextCursor.is())
        {
            if( xHeaderFooterContent.is() )
            {
                uno::Reference < text::XText > xText(xHeaderFooterContent->getCenterText());
                xText->setString(sEmpty);
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
                    xText->setString(sEmpty);
                    //SvXMLImport aSvXMLImport( GetImport() );
                    uno::Reference < text::XTextCursor > xTempTextCursor(xText->createTextCursor());
                    pContext = new XMLHeaderFooterRegionContext( GetImport(), nPrefix, rLocalName, xAttrList, xTempTextCursor);
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
                GetImport().GetTextImport()->GetCursorAsRange(), sEmpty,
                true );
        }
        GetImport().GetTextImport()->ResetCursor();
    }
    if (xOldTextCursor.is())
        GetImport().GetTextImport()->SetCursor(xOldTextCursor);
    if (xHeaderFooterContent.is())
    {
        if (!bContainsLeft)
            xHeaderFooterContent->getLeftText()->setString(sEmpty);
        if (!bContainsCenter)
            xHeaderFooterContent->getCenterText()->setString(sEmpty);
        if (!bContainsRight)
            xHeaderFooterContent->getRightText()->setString(sEmpty);

        xPropSet->setPropertyValue( sCont, uno::makeAny(xHeaderFooterContent) );
    }
}

TYPEINIT1( XMLHeaderFooterRegionContext, SvXMLImportContext );

XMLHeaderFooterRegionContext::XMLHeaderFooterRegionContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                       const OUString& rLName,
                       const uno::Reference<
                            xml::sax::XAttributeList > & /* xAttrList */,
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

SvXMLImportContext *XMLHeaderFooterRegionContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext(0);

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
            OUString sEmpty;
            GetImport().GetTextImport()->GetText()->insertString(
                GetImport().GetTextImport()->GetCursorAsRange(), sEmpty,
                true );
        }
        GetImport().GetTextImport()->ResetCursor();
    }
    if (xOldTextCursor.is())
        GetImport().GetTextImport()->SetCursor(xOldTextCursor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
