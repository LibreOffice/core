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

#include "XMLSymbolImageContext.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

using namespace css;
using namespace xmloff::token;

namespace {

enum SvXMLTokenMapAttrs
{
    XML_TOK_SYMBOL_IMAGE_HREF,
    XML_TOK_SYMBOL_IMAGE_TYPE,
    XML_TOK_SYMBOL_IMAGE_ACTUATE,
    XML_TOK_SYMBOL_IMAGE_SHOW,
};

}

const SvXMLTokenMapEntry aSymbolImageAttrTokenMap[] =
{
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_HREF,     XML_TOK_SYMBOL_IMAGE_HREF    },
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_TYPE,     XML_TOK_SYMBOL_IMAGE_TYPE    },
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_ACTUATE,  XML_TOK_SYMBOL_IMAGE_ACTUATE },
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_SHOW,     XML_TOK_SYMBOL_IMAGE_SHOW },
    XML_TOKEN_MAP_END
};

XMLSymbolImageContext::XMLSymbolImageContext(
    SvXMLImport& rImport, sal_Int32 nElement,
    const XMLPropertyState& rProp,
    ::std::vector< XMLPropertyState > &rProps ) :
        XMLElementPropertyContext(
            rImport, nElement, rProp, rProps )
{
}

XMLSymbolImageContext::~XMLSymbolImageContext()
{}

void XMLSymbolImageContext::startFastElement(
    sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        const OUString sValue = aIter.toString();

        switch( aIter.getToken() )
        {
            case XML_ELEMENT(XLINK, XML_HREF):
                msURL = sValue;
                break;
            case XML_ELEMENT(XLINK, XML_ACTUATE):
            case XML_ELEMENT(XLINK, XML_TYPE):
            case XML_ELEMENT(XLINK, XML_SHOW):
                // these values are currently not interpreted
                // it is always assumed 'actuate=onLoad', 'type=simple', 'show=embed'
                break;
            default:
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }
}

SvXMLImportContextRef XMLSymbolImageContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext* pContext = nullptr;
    if( xmloff::token::IsXMLToken( rLocalName,
                                   xmloff::token::XML_BINARY_DATA ) )
    {
        if( msURL.isEmpty() && ! mxBase64Stream.is() )
        {
            mxBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( mxBase64Stream.is() )
                pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                       rLocalName, xAttrList,
                                                       mxBase64Stream );
        }
    }

    return pContext;
}

void XMLSymbolImageContext::endFastElement(sal_Int32 nElement)
{
    uno::Reference<graphic::XGraphic> xGraphic;

    if (!msURL.isEmpty())
    {
        xGraphic = GetImport().loadGraphicByURL(msURL);
    }
    else if (mxBase64Stream.is())
    {
        xGraphic = GetImport().loadGraphicFromBase64(mxBase64Stream);
        mxBase64Stream = nullptr;
    }

    if (xGraphic.is())
    {
        // aProp is a member of XMLElementPropertyContext
        aProp.maValue <<= xGraphic;
        SetInsert( true );
    }

    XMLElementPropertyContext::endFastElement(nElement);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
