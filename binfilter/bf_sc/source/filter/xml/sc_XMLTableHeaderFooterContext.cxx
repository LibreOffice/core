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

#ifndef _XMLOFF_NMSPMAP_HXX
#include <bf_xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif
#ifndef _SC_XMLTABLEHEADERFOOTERCONTEXT_HXX_
#include "XMLTableHeaderFooterContext.hxx"
#endif

#include "unonames.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace xmloff::token;


TYPEINIT1( XMLTableHeaderFooterContext, SvXMLImportContext );

XMLTableHeaderFooterContext::XMLTableHeaderFooterContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                       const OUString& rLName,
                       const uno::Reference<
                            xml::sax::XAttributeList > & xAttrList,
                        const Reference < XPropertySet > & rPageStylePropSet,
                       sal_Bool bFooter, sal_Bool bLft ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xPropSet( rPageStylePropSet ),
    sOn( OUString::createFromAscii( bFooter ? SC_UNO_PAGE_FTRON : SC_UNO_PAGE_HDRON ) ),
    sShareContent( OUString::createFromAscii( bFooter ? SC_UNO_PAGE_FTRSHARED : SC_UNO_PAGE_HDRSHARED ) ),
    sContent( OUString::createFromAscii( bFooter ? SC_UNO_PAGE_RIGHTFTRCON : SC_UNO_PAGE_RIGHTHDRCON ) ),
    sContentLeft( OUString::createFromAscii( bFooter ? SC_UNO_PAGE_LEFTFTRCONT : SC_UNO_PAGE_LEFTHDRCONT ) ),
    bInsertContent( sal_True ),
    bLeft( bLft ),
    bDisplay( sal_True ),
    bContainsLeft(sal_False),
    bContainsRight(sal_False),
    bContainsCenter(sal_False)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        // TODO: use a map here
        if( XML_NAMESPACE_STYLE == nPrfx )
        {
            if( IsXMLToken(aLName, XML_DISPLAY ) )
                bDisplay = IsXMLToken(rValue, XML_TRUE);
        }
    }
    if( bLeft )
    {
        Any aAny;

        aAny = xPropSet->getPropertyValue( sOn );
        sal_Bool bOn = *(sal_Bool *)aAny.getValue();

        if( bOn && bDisplay )
        {
            aAny = xPropSet->getPropertyValue( sShareContent );
            sal_Bool bShared = *(sal_Bool *)aAny.getValue();
            if( bShared )
            {
                // Don't share headers any longer
                bShared = sal_False;
                aAny.setValue( &bShared, ::getBooleanCppuType() );
                xPropSet->setPropertyValue( sShareContent, aAny );
            }
        }
        else
        {
            aAny = xPropSet->getPropertyValue( sShareContent );
            sal_Bool bShared = *(sal_Bool *)aAny.getValue();
            if( !bShared )
            {
                // share headers
                bShared = sal_True;
                aAny.setValue( &bShared, ::getBooleanCppuType() );
                xPropSet->setPropertyValue( sShareContent, aAny );
            }
        }
    }
    else
    {
        Any aAny;

        aAny = xPropSet->getPropertyValue( sOn );
        sal_Bool bOn = *(sal_Bool *)aAny.getValue();
        if ( bOn != bDisplay )
        {
            sal_Bool bTempDisplay(bDisplay);
            aAny.setValue( &bTempDisplay, ::getBooleanCppuType() );
            bDisplay = bTempDisplay;
            xPropSet->setPropertyValue( sOn, aAny );
        }
    }
    if (bLeft)
        sCont = sContentLeft;
    else
        sCont = sContent;
    Any aAny;
    aAny = xPropSet->getPropertyValue( sCont );
    aAny >>= xHeaderFooterContent;
}

XMLTableHeaderFooterContext::~XMLTableHeaderFooterContext()
{
}

SvXMLImportContext *XMLTableHeaderFooterContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TEXT) &&
        IsXMLToken(rLocalName, XML_P))
    {
        if (!xTextCursor.is())
        {
            if( xHeaderFooterContent.is() )
            {
                uno::Reference < text::XText > xText = xHeaderFooterContent->getCenterText();
                xText->setString(sEmpty);
                xTextCursor = xText->createTextCursor();
                xOldTextCursor = GetImport().GetTextImport()->GetCursor();
                GetImport().GetTextImport()->SetCursor( xTextCursor );
                bContainsCenter = sal_True;
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
                    xText = xHeaderFooterContent->getLeftText();
                    bContainsLeft = sal_True;
                }
                else if (IsXMLToken(rLocalName, XML_REGION_CENTER ))
                {
                    xText = xHeaderFooterContent->getCenterText();
                    bContainsCenter = sal_True;
                }
                else if (IsXMLToken(rLocalName, XML_REGION_RIGHT ))
                {
                    xText = xHeaderFooterContent->getRightText();
                    bContainsRight = sal_True;
                }
                if (xText.is())
                {
                    xText->setString(sEmpty);
                    //SvXMLImport aSvXMLImport( GetImport() );
                    uno::Reference<text::XTextCursor> xTextCursor( xText->createTextCursor() );
                    pContext = new XMLHeaderFooterRegionContext( GetImport(), nPrefix, rLocalName, xAttrList, xTextCursor);
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
        if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, sal_True ) )
        {
            GetImport().GetTextImport()->GetText()->insertString(
                GetImport().GetTextImport()->GetCursorAsRange(), sEmpty,
                sal_True );
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

        Any aAny;
        aAny <<= xHeaderFooterContent;
        xPropSet->setPropertyValue( sCont, aAny );
    }
}

TYPEINIT1( XMLHeaderFooterRegionContext, SvXMLImportContext );

XMLHeaderFooterRegionContext::XMLHeaderFooterRegionContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                       const OUString& rLName,
                       const uno::Reference<
                            xml::sax::XAttributeList > & xAttrList,
                       uno::Reference< text::XTextCursor >& xCursor ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xTextCursor ( xCursor )
{
    xOldTextCursor = GetImport().GetTextImport()->GetCursor();
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
    SvXMLImportContext *pContext = 0;

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
        if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, sal_True ) )
        {
            OUString sEmpty;
            GetImport().GetTextImport()->GetText()->insertString(
                GetImport().GetTextImport()->GetCursorAsRange(), sEmpty,
                sal_True );
        }
        GetImport().GetTextImport()->ResetCursor();
    }
    if (xOldTextCursor.is())
        GetImport().GetTextImport()->SetCursor(xOldTextCursor);
}
}
