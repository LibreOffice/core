/*************************************************************************
 *
 *  $RCSfile: prstylei.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:24:59 $
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
#ifndef __SGI_STL_SET
#include <set>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
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
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSTATES_HPP_
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
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
#ifndef _XMLOFF_XMLERROR_HXX
#include "xmlerror.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::xmloff::token;


void XMLPropStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    if( XML_NAMESPACE_STYLE == nPrefixKey && IsXMLToken( rLocalName, XML_FAMILY ) )
    {
        DBG_ASSERT( GetFamily() == ((SvXMLStylesContext *)&xStyles)->GetFamily( rValue ), "unexpected style family" );
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
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily,
        sal_Bool bDefault ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, nFamily, bDefault ),
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

    sal_uInt32 nFamily = 0;
    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_GRAPHIC_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_GRAPHIC;
        else if( IsXMLToken( rLocalName, XML_DRAWING_PAGE_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_DRAWING_PAGE;
        else if( IsXMLToken( rLocalName, XML_TEXT_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( IsXMLToken( rLocalName, XML_PARAGRAPH_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( IsXMLToken( rLocalName, XML_RUBY_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_RUBY;
        else if( IsXMLToken( rLocalName, XML_SECTION_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_SECTION;
        else if( IsXMLToken( rLocalName, XML_TABLE_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE;
        else if( IsXMLToken( rLocalName, XML_TABLE_COLUMN_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE_COLUMN;
        else if( IsXMLToken( rLocalName, XML_TABLE_ROW_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE_ROW;
        else if( IsXMLToken( rLocalName, XML_TABLE_CELL_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE_CELL;
        else if( IsXMLToken( rLocalName, XML_CHART_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_CHART;
    }
    if( nFamily )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            ((SvXMLStylesContext *)&xStyles)->GetImportPropertyMapper(
                                                        GetFamily() );
        if( xImpPrMap.is() )
            pContext = new SvXMLPropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    nFamily,
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
        xImpPrMap->FillPropertySet( aProperties, rPropSet );
}

void XMLPropStyleContext::SetDefaults()
{
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

typedef ::std::set < OUString, ::comphelper::UStringLess > PropertyNameSet;

void XMLPropStyleContext::CreateAndInsert( sal_Bool bOverwrite )
{
    const OUString& rName = GetDisplayName();
    if( 0 == rName.getLength() || IsDefaultStyle() )
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
    if( rName != GetName() )
        GetImport().AddStyleDisplayName( GetFamily(), GetName(), rName );

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
            Reference < XMultiPropertyStates > xMultiStates( xPropSet,
                                                             UNO_QUERY );
            if( xMultiStates.is() )
            {
                xMultiStates->setAllPropertiesToDefault();
            }
            else
            {
                PropertyNameSet aNameSet;
                sal_Int32 nCount = xPrMap->GetEntryCount();
                sal_Int32 i;
                for( i = 0; i < nCount; i++ )
                {
                    const OUString& rName = xPrMap->GetEntryAPIName( i );
                    if( xPropSetInfo->hasPropertyByName( rName ) )
                        aNameSet.insert( rName );
                }

                nCount = aNameSet.size();
                Sequence < OUString > aNames( nCount );
                OUString *pNames = aNames.getArray();
                PropertyNameSet::iterator aIter = aNameSet.begin();
                while( aIter != aNameSet.end() )
                    *pNames++ = *aIter++;

                Sequence < PropertyState > aStates(
                    xPropState->getPropertyStates( aNames ) );
                const PropertyState *pStates = aStates.getConstArray();
                pNames = aNames.getArray();

                for( i = 0; i < nCount; i++ )
                {
                    if( PropertyState_DIRECT_VALUE == *pStates++ )
                        xPropState->setPropertyToDefault( pNames[i] );
                }
            }
        }

        if (xStyle.is())
            xStyle->setParentStyle(OUString());

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
        if( sParent.getLength() )
            sParent = GetImport().GetStyleDisplayName( GetFamily(), sParent );
        if( sParent.getLength() && !xFamilies->hasByName( sParent ) )
            sParent = OUString();

        if( sParent != xStyle->getParentStyle() )
        {
            // this may except if setting the parent style forms a
            // circle in the style depencies; especially if the parent
            // style is the same as the current style
            try
            {
                xStyle->setParentStyle( sParent );
            }
            catch( uno::Exception e )
            {
                // according to the API definition, I would expect a
                // container::NoSuchElementException. But it throws an
                // uno::RuntimeException instead. I catch
                // uno::Exception in order to process both of them.

                // We can't set the parent style. For a proper
                // Error-Message, we should pass in the name of the
                // style, as well as the desired parent style.
                Sequence<OUString> aSequence(2);

                // getName() throws no non-Runtime exception:
                aSequence[0] = xStyle->getName();
                aSequence[1] = sParent;

                GetImport().SetError(
                    XMLERROR_FLAG_ERROR | XMLERROR_PARENT_STYLE_NOT_ALLOWED,
                    aSequence, e.Message, NULL );
            }
        }

        // connect follow
        OUString sFollow( GetFollow() );
        if( sFollow.getLength() )
            sFollow = GetImport().GetStyleDisplayName( GetFamily(), sFollow );
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


