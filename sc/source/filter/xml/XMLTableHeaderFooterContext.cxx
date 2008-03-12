/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTableHeaderFooterContext.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:16:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _SC_XMLTABLEHEADERFOOTERCONTEXT_HXX_
#include "XMLTableHeaderFooterContext.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#include "unonames.hxx"

using ::rtl::OUString;
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
    bDisplay( sal_True ),
    bInsertContent( sal_True ),
    bLeft( bLft ),
    bContainsLeft(sal_False),
    bContainsRight(sal_False),
    bContainsCenter(sal_False)
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
        sal_Bool bOn(::cppu::any2bool(xPropSet->getPropertyValue( sOn )));

        if( bOn && bDisplay )
        {
            if( ::cppu::any2bool(xPropSet->getPropertyValue( sShareContent )) )
                // Don't share headers any longer
                xPropSet->setPropertyValue( sShareContent, uno::makeAny(sal_False) );
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
        sal_Bool bOn(::cppu::any2bool(xPropSet->getPropertyValue( sOn )));
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
                    xText.set(xHeaderFooterContent->getLeftText());
                    bContainsLeft = sal_True;
                }
                else if (IsXMLToken(rLocalName, XML_REGION_CENTER ))
                {
                    xText.set(xHeaderFooterContent->getCenterText());
                    bContainsCenter = sal_True;
                }
                else if (IsXMLToken(rLocalName, XML_REGION_RIGHT ))
                {
                    xText.set(xHeaderFooterContent->getRightText());
                    bContainsRight = sal_True;
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
