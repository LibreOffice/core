/*************************************************************************
 *
 *  $RCSfile: XMLShapeStyleContext.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: cl $ $Date: 2002-01-11 12:18:09 $
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

#include <tools/debug.hxx>

#ifndef _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
#include "XMLShapeStyleContext.hxx"
#endif
#ifndef _XMLOFF_XMLSHAPEPROPERTYSETCONTEXT_HXX
#include "XMLShapePropertySetContext.hxx"
#endif

#ifndef _XMLOFF_CONTEXTID_HXX_
#include "contextid.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLNUMI_HXX
#include "xmlnumi.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmlnmspe.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLERROR_HXX
#include "xmlerror.hxx"
#endif

#include "sdpropls.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_PROPERTIES;

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( XMLShapeStyleContext, XMLPropStyleContext );

XMLShapeStyleContext::XMLShapeStyleContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    SvXMLStylesContext& rStyles,
    sal_uInt16 nFamily)
:   XMLPropStyleContext(rImport, nPrfx, rLName, xAttrList, rStyles, nFamily ),
    m_bIsNumRuleAlreadyConverted( sal_False )
{
}

XMLShapeStyleContext::~XMLShapeStyleContext()
{
}

void XMLShapeStyleContext::SetAttribute( sal_uInt16 nPrefixKey, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if ((0 == m_sControlDataStyleName.getLength()) && (::xmloff::token::GetXMLToken(::xmloff::token::XML_DATA_STYLE_NAME) == rLocalName))
    {
        m_sControlDataStyleName = rValue;
    }
    else if( (XML_NAMESPACE_STYLE == nPrefixKey) && IsXMLToken( rLocalName, ::xmloff::token::XML_LIST_STYLE_NAME ) )
    {
        m_sListStyleName = rValue;
    }
    else
    {
        XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}

SvXMLImportContext *XMLShapeStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken( rLocalName, XML_PROPERTIES ) )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
            pContext = new XMLShapePropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    GetProperties(),
                                                    xImpPrMap );
    }

    if( !pContext )
        pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void XMLShapeStyleContext::FillPropertySet( const Reference< beans::XPropertySet > & rPropSet )
{
    if( !m_bIsNumRuleAlreadyConverted )
    {
        m_bIsNumRuleAlreadyConverted = sal_True;

        // for compatibility to beta files, search for CTF_SD_NUMBERINGRULES_NAME to
        // import numbering rules from the style:properties element
        const UniReference< XMLPropertySetMapper >&rMapper = GetStyles()->GetImportPropertyMapper( GetFamily() )->getPropertySetMapper();

        ::std::vector< XMLPropertyState > &rProperties = GetProperties();
        ::std::vector< XMLPropertyState >::iterator end( rProperties.end() );
        ::std::vector< XMLPropertyState >::iterator property;

        // first, look for the old format, where we had a text:list-style-name
        // attribute in the style:properties element
        for( property = rProperties.begin(); property != end; property++ )
        {
            // find properties with context
            if( rMapper->GetEntryContextId( property->mnIndex ) == CTF_SD_NUMBERINGRULES_NAME )
                break;
        }

        // if we did not find an old list-style-name in the properties, and we need one
        // because we got a style:list-style attribute in the style-style element
        // we generate one
        if( (property == end) && ( 0 != m_sListStyleName.getLength() ) )
        {
            sal_Int32 nIndex = rMapper->FindEntryIndex( CTF_SD_NUMBERINGRULES_NAME );
            DBG_ASSERT( -1 != nIndex, "can't find numbering rules property entry, can't set numbering rule!" );

            XMLPropertyState aNewState( nIndex );
            rProperties.push_back( aNewState );
            end = rProperties.end();
            property = end - 1;
        }

        // so, if we have an old or a new list style name, we set its value to
        // a numbering rule
        if( property != end )
        {
            if( 0 == m_sListStyleName.getLength() )
            {
                property->maValue >>= m_sListStyleName;
            }

            const SvxXMLListStyleContext *pListStyle = GetImport().GetTextImport()->FindAutoListStyle( m_sListStyleName );

            DBG_ASSERT( pListStyle, "list-style not found for shape style" );
            if( pListStyle )
            {
                uno::Reference< container::XIndexReplace > xNumRule( pListStyle->CreateNumRule( GetImport().GetModel() ) );
                pListStyle->FillUnoNumRule(xNumRule, NULL /* const SvI18NMap * ??? */ );
                property->maValue <<= xNumRule;
            }
            else
            {
                property->mnIndex = -1;
            }
        }
    }

    XMLPropStyleContext::FillPropertySet(rPropSet);

    if (m_sControlDataStyleName.getLength())
    {   // we had a data-style-name attribute

        // set the formatting on the control model of the control shape
        uno::Reference< drawing::XControlShape > xControlShape(rPropSet, uno::UNO_QUERY);
        DBG_ASSERT(xControlShape.is(), "XMLShapeStyleContext::FillPropertySet: data style for a non-control shape!");
        if (xControlShape.is())
        {
            uno::Reference< beans::XPropertySet > xControlModel(xControlShape->getControl(), uno::UNO_QUERY);
            DBG_ASSERT(xControlModel.is(), "XMLShapeStyleContext::FillPropertySet: no control model for the shape!");
            if (xControlModel.is())
            {
                GetImport().GetFormImport()->applyControlNumberStyle(xControlModel, m_sControlDataStyleName);
            }
        }
    }
}

void XMLShapeStyleContext::Finish( sal_Bool bOverwrite )
{
}

