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
#include "XMLChartStyleContext.hxx"

#include "xmlnmspe.hxx"
#include "xmlnumfi.hxx"
#include "families.hxx"

#include "XMLChartPropertyContext.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using ::binfilter::xmloff::token::IsXMLToken;
using ::binfilter::xmloff::token::XML_DATA_STYLE_NAME;


TYPEINIT1( XMLChartStyleContext, XMLPropStyleContext );

// protected

void XMLChartStyleContext::SetAttribute(
    sal_uInt16 nPrefixKey,
    const ::rtl::OUString& rLocalName,
    const ::rtl::OUString& rValue )
{
    if( IsXMLToken( rLocalName, XML_DATA_STYLE_NAME ) )
    {
        msDataStyleName = rValue;
    }
    else
    {
        XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}

//public

// CTOR
XMLChartStyleContext::XMLChartStyleContext(
    SvXMLImport& rImport, sal_uInt16 nPrfx,
    const ::rtl::OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList,
    SvXMLStylesContext& rStyles, sal_uInt16 nFamily ) :

        XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily ),
        mrStyles( rStyles )
{}

// DTOR
XMLChartStyleContext::~XMLChartStyleContext()
{}

void XMLChartStyleContext::FillPropertySet(
    const uno::Reference< beans::XPropertySet > & rPropSet )
{
    XMLPropStyleContext::FillPropertySet( rPropSet );
    if( msDataStyleName.getLength())
    {
        SvXMLNumFormatContext* pStyle = (SvXMLNumFormatContext *)mrStyles.FindStyleChildContext(
            XML_STYLE_FAMILY_DATA_STYLE, msDataStyleName, sal_True );
        if( pStyle )
        {
            uno::Any aNumberFormat;
            sal_Int32 nNumberFormat = pStyle->GetKey();
            aNumberFormat <<= nNumberFormat;
            rPropSet->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NumberFormat" )),
                                        aNumberFormat );
        }
    }
}

SvXMLImportContext *XMLChartStyleContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken( rLocalName, ::binfilter::xmloff::token::XML_PROPERTIES ) )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
            pContext = new XMLChartPropertyContext(
                GetImport(), nPrefix, rLocalName, xAttrList,
                GetProperties(), xImpPrMap );
    }

    if( !pContext )
        pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
