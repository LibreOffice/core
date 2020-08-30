/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <XMLChartStyleContext.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlimppr.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/diagnose_ex.h>

#include "XMLChartPropertyContext.hxx"

using namespace com::sun::star;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_DATA_STYLE_NAME;
using ::xmloff::token::XML_PERCENTAGE_DATA_STYLE_NAME;
using ::xmloff::token::XML_TEXT_PROPERTIES;
using ::xmloff::token::XML_PARAGRAPH_PROPERTIES;
using ::xmloff::token::XML_GRAPHIC_PROPERTIES;
using ::xmloff::token::XML_CHART_PROPERTIES;


void XMLChartStyleContext::SetAttribute(
    sal_uInt16 nPrefixKey,
    const OUString& rLocalName,
    const OUString& rValue )
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

XMLChartStyleContext::XMLChartStyleContext(
    SvXMLImport& rImport,
    SvXMLStylesContext& rStyles, XmlStyleFamily nFamily ) :
        XMLShapeStyleContext( rImport, rStyles, nFamily ),
        mrStyles( rStyles )
{}

XMLChartStyleContext::~XMLChartStyleContext()
{}

namespace
{

void lcl_NumberFormatStyleToProperty( const OUString& rStyleName, const OUString& rPropertyName,
        const SvXMLStylesContext& rStylesContext,
        const uno::Reference< beans::XPropertySet >& rPropSet )
{
    if( !rStyleName.isEmpty())
    {
        const SvXMLNumFormatContext* pStyle = static_cast<const SvXMLNumFormatContext *>(rStylesContext.FindStyleChildContext(
            XmlStyleFamily::DATA_STYLE, rStyleName, true ));
        if( pStyle )
        {
            sal_Int32 nNumberFormat = const_cast<SvXMLNumFormatContext*>(pStyle)->GetKey();
            rPropSet->setPropertyValue( rPropertyName, uno::Any(nNumberFormat) );
        }
    }
}

}

void XMLChartStyleContext::FillPropertySet(
    const uno::Reference< beans::XPropertySet > & rPropSet )
{
    try
    {
        XMLShapeStyleContext::FillPropertySet( rPropSet );
    }
    catch( beans::UnknownPropertyException&  )
    {
        TOOLS_WARN_EXCEPTION( "xmloff", "unknown property exception -> shape style not completely imported for chart style" );
    }

    lcl_NumberFormatStyleToProperty( msDataStyleName, "NumberFormat", mrStyles, rPropSet );
    lcl_NumberFormatStyleToProperty( msPercentageDataStyleName, "PercentageNumberFormat", mrStyles, rPropSet );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLChartStyleContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;

    if( IsTokenInNamespace(nElement, XML_NAMESPACE_STYLE) ||
        IsTokenInNamespace(nElement, XML_NAMESPACE_LO_EXT) )
    {
        sal_Int32 nLocalName = nElement & TOKEN_MASK;
        sal_uInt32 nFamily = 0;
        if( nLocalName == XML_TEXT_PROPERTIES )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( nLocalName == XML_PARAGRAPH_PROPERTIES )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( nLocalName == XML_GRAPHIC_PROPERTIES )
            nFamily = XML_TYPE_PROP_GRAPHIC;
        else if( nLocalName == XML_CHART_PROPERTIES )
            nFamily = XML_TYPE_PROP_CHART;
        if( nFamily )
        {
            rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
                GetStyles()->GetImportPropertyMapper( GetFamily() );
            if( xImpPrMap.is() )
                return new XMLChartPropertyContext(
                    GetImport(), nElement, xAttrList, nFamily,
                    GetProperties(), xImpPrMap );
        }
    }

    return XMLShapeStyleContext::createFastChildContext( nElement, xAttrList );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
