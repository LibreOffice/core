/*************************************************************************
 *
 *  $RCSfile: prstylei.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-18 11:18:29 $
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


#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLPROPERTYSETCONTEXT_HXX
#include "xmlprcon.hxx"
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_PRSTYLEI_HXX
#include "prstylei.hxx"
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;


void XMLPropStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    if( XML_NAMESPACE_STYLE == nPrefixKey &&
        rLocalName.compareToAscii( sXML_family ) == 0 )
    {
        SetFamily( ((SvXMLStylesContext *)&xStyles)->GetFamily( rValue ) );
    }
    else
    {
        SvXMLStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}

TYPEINIT1( XMLPropStyleContext, SvXMLStyleContext );

XMLPropStyleContext::XMLPropStyleContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList ),
    xStyles( &rStyles ),
    sIsPhysical( RTL_CONSTASCII_USTRINGPARAM( "IsPhysical" ) ),
    sFollowStyle( RTL_CONSTASCII_USTRINGPARAM( "FollowStyle" ) )
{
}

XMLPropStyleContext::~XMLPropStyleContext()
{
}

SvXMLImportContext *XMLPropStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        rLocalName.compareToAscii( sXML_properties ) == 0 )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            ((SvXMLStylesContext *)&xStyles)->GetImportPropertyMapper(
                                                        GetFamily() );
        if( xImpPrMap.is() )
            pContext = new SvXMLPropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    aProperties,
                                                    xImpPrMap );
    }

    if( !pContext )
        pContext = SvXMLStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void XMLPropStyleContext::FillPropertySet(
            const Reference< XPropertySet > & rPropSet )
{
    UniReference < SvXMLImportPropertyMapper > xImpPrMap =
        ((SvXMLStylesContext *)&xStyles)->GetImportPropertyMapper(
                                                                GetFamily() );
    DBG_ASSERT( xImpPrMap.is(), "There is the import prop mapper" );
    if( xImpPrMap.is() )
        xImpPrMap->getPropertySetMapper()->FillPropertySet( aProperties,
                                                           rPropSet );
}

Reference < XStyle > XMLPropStyleContext::Create()
{
    Reference < XStyle > xNewStyle;

    OUString sServiceName(
        ((SvXMLStylesContext *)&xStyles)->GetServiceName( GetFamily() ) );
    if( sServiceName.getLength() )
    {
        Reference< XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                    UNO_QUERY );
        if( xFactory.is() )
        {
            Reference < XInterface > xIfc =
                xFactory->createInstance( sServiceName );
            if( xIfc.is() )
                xNewStyle = Reference < XStyle >( xIfc, UNO_QUERY );
        }
    }

    return xNewStyle;
}

void XMLPropStyleContext::CreateAndInsert( sal_Bool bOverwrite )
{
    const OUString& rName = GetName();
    if( 0 == rName.getLength() )
        return;

    Reference < XNameContainer > xFamilies =
            ((SvXMLStylesContext *)&xStyles)->GetStylesContainer( GetFamily() );
    if( !xFamilies.is() )
        return;

    sal_Bool bNew = sal_False;
    if( xFamilies->hasByName( rName ) )
    {
        Any aAny = xFamilies->getByName( rName );
        aAny >>= xStyle;
    }
    else
    {
        xStyle = Create();
        if( !xStyle.is() )
            return;

        Any aAny;
        aAny <<= xStyle;
        xFamilies->insertByName( rName, aAny );
        bNew = sal_True;
    }

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();
    if( !bNew && xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        Any aAny = xPropSet->getPropertyValue( sIsPhysical );
        bNew = !*(sal_Bool *)aAny.getValue();
    }
    SetNew( bNew );

    if( bOverwrite || bNew )
    {
        Reference< XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );

        UniReference < XMLPropertySetMapper > xPrMap;
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            ((SvXMLStylesContext *)&xStyles)->GetImportPropertyMapper(
                                                                GetFamily() );
        DBG_ASSERT( xImpPrMap.is(), "There is the import prop mapper" );
        if( xImpPrMap.is() )
            xPrMap = xImpPrMap->getPropertySetMapper();
        if( xPrMap.is() )
        {
            sal_Int32 nCount = xPrMap->GetEntryCount();
            for( sal_Int32 i = 0; i < nCount; i++ )
            {
                const OUString& rName = xPrMap->GetEntryAPIName( i );
                if( xPropSetInfo->hasPropertyByName( rName ) &&
                    PropertyState_DIRECT_VALUE ==
                        xPropState->getPropertyState( rName ) )
                {
                    xPropState->setPropertyToDefault( rName );
                }
            }
        }

        FillPropertySet( xPropSet );
    }
    else
    {
        SetValid( sal_False );
    }
}

void XMLPropStyleContext::Finish( sal_Bool bOverwrite )
{
    if( xStyle.is() && (IsNew() || bOverwrite) )
    {
        // The families cintaner must exist
        Reference < XNameContainer > xFamilies =
            ((SvXMLStylesContext *)&xStyles)->GetStylesContainer( GetFamily() );
        DBG_ASSERT( xFamilies.is(), "Families lost" );
        if( !xFamilies.is() )
            return;

        // connect parent
        OUString sParent( GetParent() );
        if( sParent.getLength() && !xFamilies->hasByName( sParent ) )
            sParent = OUString();

        if( sParent != xStyle->getParentStyle() )
            xStyle->setParentStyle( sParent );

        // connect follow
        OUString sFollow( GetFollow() );
        if( !sFollow.getLength() || !xFamilies->hasByName( sFollow ) )
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


