/*************************************************************************
 *
 *  $RCSfile: XMLTextMasterPageContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-18 11:18:30 $
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

#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PAGESTYLELAYOUT_HPP_
#include <com/sun/star/style/PageStyleLayout.hpp>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_TEXTMASTERPAGECONTEXT_HXX_
#include "XMLTextMasterPageContext.hxx"
#endif
#ifndef _XMLOFF_TEXTHEADERFOOTERCONTEXT_HXX_
#include "XMLTextHeaderFooterContext.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//using namespace ::com::sun::star::text;

Reference < XStyle > XMLTextMasterPageContext::Create()
{
    Reference < XStyle > xNewStyle;

    Reference< XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                    UNO_QUERY );
    if( xFactory.is() )
    {
        Reference < XInterface > xIfc =
            xFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.style.PageStyle")) );
        if( xIfc.is() )
            xNewStyle = Reference < XStyle >( xIfc, UNO_QUERY );
    }

    return xNewStyle;
}
TYPEINIT1( XMLTextMasterPageContext, SvXMLStyleContext );

XMLTextMasterPageContext::XMLTextMasterPageContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        sal_Bool bOverwrite ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList ),
    sIsPhysical( RTL_CONSTASCII_USTRINGPARAM( "IsPhysical" ) ),
    sFollowStyle( RTL_CONSTASCII_USTRINGPARAM( "FollowStyle" ) ),
    sPageStyleLayout( RTL_CONSTASCII_USTRINGPARAM( "PageStyleLayout" ) ),
    bInsertHeader( sal_False ),
    bInsertFooter( sal_False ),
    bInsertHeaderLeft( sal_False ),
    bInsertFooterLeft( sal_False ),
    bHeaderInserted( sal_False ),
    bFooterInserted( sal_False ),
    bHeaderLeftInserted( sal_False ),
    bFooterLeftInserted( sal_False )
{
    SetFamily( XML_STYLE_FAMILY_MASTER_PAGE );

    OUString sName;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );
        if( XML_NAMESPACE_STYLE == nPrefix )
        {
            if( aLocalName.equalsAsciiL( sXML_name, sizeof( sXML_name )-1 ) )
            {
                sName = xAttrList->getValueByIndex( i );
            }
            else if( aLocalName.equalsAsciiL( sXML_next_style_name,
                        sizeof( sXML_next_style_name )-1 ) )
            {
                sFollow = xAttrList->getValueByIndex( i );
            }
        }
    }

    if( 0 == sName.getLength() )
        return;

    Reference < XNameContainer > xPageStyles =
            GetImport().GetTextImport()->GetPageStyles();
    if( !xPageStyles.is() )
        return;

    Any aAny;
    sal_Bool bNew = sal_False;
    if( xPageStyles->hasByName( sName ) )
    {
        aAny = xPageStyles->getByName( sName );
        aAny >>= xStyle;
    }
    else
    {
        xStyle = Create();
        if( !xStyle.is() )
            return;

        aAny <<= xStyle;
        xPageStyles->insertByName( sName, aAny );
        bNew = sal_True;
    }

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();
    if( !bNew && xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        aAny = xPropSet->getPropertyValue( sIsPhysical );
        bNew = !*(sal_Bool *)aAny.getValue();
    }
    SetNew( bNew );

    if( bOverwrite || bNew )
    {
        // TODO: Look for page master and insert its attributes here!

        bInsertHeader = bInsertFooter = sal_True;
        bInsertHeaderLeft = bInsertFooterLeft = sal_True;
    }
}

XMLTextMasterPageContext::~XMLTextMasterPageContext()
{
}

SvXMLImportContext *XMLTextMasterPageContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextMasterPageElemTokenMap();

    sal_Bool bInsert = sal_False, bFooter = sal_False, bLeft = sal_False;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_TEXT_MP_HEADER:
        if( bInsertHeader && !bHeaderInserted )
        {
            bInsert = sal_True;
            bHeaderInserted = sal_True;
        }
        break;
    case XML_TOK_TEXT_MP_FOOTER:
        if( bInsertFooter && !bFooterInserted )
        {
            bInsert = bFooter = sal_True;
            bFooterInserted = sal_True;
        }
        break;
    case XML_TOK_TEXT_MP_HEADER_LEFT:
        if( bInsertHeaderLeft && bHeaderInserted && !bHeaderLeftInserted )
            bInsert = bLeft = sal_True;
        break;
    case XML_TOK_TEXT_MP_FOOTER_LEFT:
        if( bInsertFooterLeft && bFooterInserted && !bFooterLeftInserted )
            bInsert = bFooter = bLeft = sal_True;
        break;
    }

    if( bInsert && xStyle.is() )
    {
        Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
        pContext = new XMLTextHeaderFooterContext( GetImport(),
                                                    nPrefix, rLocalName,
                                                    xAttrList,
                                                    xPropSet,
                                                    bFooter, bLeft );
    }
    else
    {
        pContext = SvXMLStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );
    }

    return pContext;
}

void XMLTextMasterPageContext::Finish( sal_Bool bOverwrite )
{
    if( xStyle.is() && (IsNew() || bOverwrite) )
    {
        Reference < XNameContainer > xPageStyles =
            GetImport().GetTextImport()->GetPageStyles();
        if( !xPageStyles.is() )
            return;

        if( !sFollow.getLength() || !xPageStyles->hasByName( sFollow ) )
            sFollow = xStyle->getName();

        Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
        Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        if( xPropSetInfo->hasPropertyByName( sFollowStyle ) )
        {
            Any aAny = xPropSet->getPropertyValue( sFollowStyle );
            OUString sCurrFollow;
            aAny >>= sCurrFollow;
            if( sCurrFollow != sFollow )
            {
                aAny <<= sFollow;
                xPropSet->setPropertyValue( sFollowStyle, aAny );
            }
        }
    }
}
