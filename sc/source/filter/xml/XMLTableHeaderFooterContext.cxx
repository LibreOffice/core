/*************************************************************************
 *
 *  $RCSfile: XMLTableHeaderFooterContext.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-28 08:19:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _SC_XMLTABLEHEADERFOOTERCONTEXT_HXX_
#include "XMLTableHeaderFooterContext.hxx"
#endif

#include "unonames.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
//using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
//using namespace ::com::sun::star::container;
//using namespace ::com::sun::star::lang;
//using namespace ::com::sun::star::text;


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
    bDisplay( sal_True )
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
            if( aLName.compareToAscii( sXML_display ) == 0 )
                if ( rValue.compareToAscii( sXML_false ) == 0 )
                    bDisplay = sal_False;
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

    if (nPrefix == XML_NAMESPACE_TEXT &&
        rLocalName.compareToAscii(sXML_p) == 0)
    {
        if (!xTextCursor.is())
        {
            if( xHeaderFooterContent.is() )
            {
                uno::Reference < text::XText > xText = xHeaderFooterContent->getCenterText();
                rtl::OUString sEmpty;
                xText->setString(sEmpty);
                xTextCursor = xText->createTextCursor();
                xOldTextCursor = GetImport().GetTextImport()->GetCursor();
                GetImport().GetTextImport()->SetCursor( xTextCursor );
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
                if (rLocalName.compareToAscii( sXML_region_left ) == 0)
                    xText = xHeaderFooterContent->getLeftText();
                else if (rLocalName.compareToAscii( sXML_region_center ) == 0)
                    xText = xHeaderFooterContent->getCenterText();
                else if (rLocalName.compareToAscii( sXML_region_right ) == 0)
                    xText = xHeaderFooterContent->getRightText();
                if (xText.is())
                {
                    rtl::OUString sEmpty;
                    xText->setString(sEmpty);
                    pContext = new XMLHeaderFooterRegionContext( GetImport(), nPrefix, rLocalName, xAttrList, xText->createTextCursor() );
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
            OUString sEmpty;
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

    if (nPrefix == XML_NAMESPACE_TEXT &&
        rLocalName.compareToAscii(sXML_p) == 0)
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
