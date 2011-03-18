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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "XMLChartStyleContext.hxx"
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlnumfi.hxx>
#include <xmloff/families.hxx>

#include "XMLChartPropertyContext.hxx"

using namespace com::sun::star;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_DATA_STYLE_NAME;
using ::xmloff::token::XML_PERCENTAGE_DATA_STYLE_NAME;
using ::xmloff::token::XML_TEXT_PROPERTIES;
using ::xmloff::token::XML_PARAGRAPH_PROPERTIES;
using ::xmloff::token::XML_GRAPHIC_PROPERTIES;
using ::xmloff::token::XML_CHART_PROPERTIES;


TYPEINIT1( XMLChartStyleContext, XMLPropStyleContext );

// protected

void XMLChartStyleContext::SetAttribute(
    sal_uInt16 nPrefixKey,
    const ::rtl::OUString& rLocalName,
    const ::rtl::OUString& rValue )
{
    if( IsXMLToken( rLocalName, XML_DATA_STYLE_NAME ) )
    {
        msDataStyleName =rValue;
    }
    else if( IsXMLToken( rLocalName, XML_PERCENTAGE_DATA_STYLE_NAME ) )
    {
        msPercentageDataStyleName =rValue;
    }
    else
    {
        XMLShapeStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}

//public

// CTOR
XMLChartStyleContext::XMLChartStyleContext(
    SvXMLImport& rImport, sal_uInt16 nPrfx,
    const ::rtl::OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList,
    SvXMLStylesContext& rStyles, sal_uInt16 nFamily ) :

        XMLShapeStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily ),
        mrStyles( rStyles )
{}

// DTOR
XMLChartStyleContext::~XMLChartStyleContext()
{}

namespace
{

    void lcl_NumberFormatStyleToProperty( const ::rtl::OUString& rStyleName, const ::rtl::OUString& rPropertyName,
                                      const SvXMLStylesContext& rStylesContext,
                                      const uno::Reference< beans::XPropertySet >& rPropSet )
{
    if( rStyleName.getLength())
    {
        SvXMLNumFormatContext* pStyle = (SvXMLNumFormatContext *)rStylesContext.FindStyleChildContext(
            XML_STYLE_FAMILY_DATA_STYLE, rStyleName, sal_True );
        if( pStyle )
        {
            uno::Any aNumberFormat;
            sal_Int32 nNumberFormat = pStyle->GetKey();
            aNumberFormat <<= nNumberFormat;
            rPropSet->setPropertyValue( rPropertyName, aNumberFormat );
        }
    }
}

}// anonymous namespace

void XMLChartStyleContext::FillPropertySet(
    const uno::Reference< beans::XPropertySet > & rPropSet )
{
    try
    {
        XMLShapeStyleContext::FillPropertySet( rPropSet );
    }
    catch( beans::UnknownPropertyException&  )
    {
        DBG_ASSERT( false, "unknown property exception -> shape style not completly imported for chart style" );
    }

    lcl_NumberFormatStyleToProperty( msDataStyleName, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NumberFormat" )), mrStyles, rPropSet );
    lcl_NumberFormatStyleToProperty( msPercentageDataStyleName, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PercentageNumberFormat" )), mrStyles, rPropSet );
}

SvXMLImportContext *XMLChartStyleContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        sal_uInt32 nFamily = 0;
        if( IsXMLToken( rLocalName, XML_TEXT_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( IsXMLToken( rLocalName, XML_PARAGRAPH_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( IsXMLToken( rLocalName, XML_GRAPHIC_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_GRAPHIC;
        else if( IsXMLToken( rLocalName, XML_CHART_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_CHART;
        if( nFamily )
        {
            UniReference < SvXMLImportPropertyMapper > xImpPrMap =
                GetStyles()->GetImportPropertyMapper( GetFamily() );
            if( xImpPrMap.is() )
                pContext = new XMLChartPropertyContext(
                    GetImport(), nPrefix, rLocalName, xAttrList, nFamily,
                    GetProperties(), xImpPrMap );
        }
    }

    if( !pContext )
        pContext = XMLShapeStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                             xAttrList );

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
