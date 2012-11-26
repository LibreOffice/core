/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "XMLSymbolImageContext.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmltkmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>
#include <com/sun/star/io/XOutputStream.hpp>

using namespace ::com::sun::star;

enum SvXMLTokenMapAttrs
{
    XML_TOK_SYMBOL_IMAGE_HREF,
    XML_TOK_SYMBOL_IMAGE_TYPE,
    XML_TOK_SYMBOL_IMAGE_ACTUATE,
    XML_TOK_SYMBOL_IMAGE_SHOW,
    XML_TOK_SYMBOL_IMAGE_END = XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aSymbolImageAttrTokenMap[] =
{
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_HREF,     XML_TOK_SYMBOL_IMAGE_HREF    },
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_TYPE,     XML_TOK_SYMBOL_IMAGE_TYPE    },
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_ACTUATE,  XML_TOK_SYMBOL_IMAGE_ACTUATE },
    { XML_NAMESPACE_XLINK,  ::xmloff::token::XML_SHOW,     XML_TOK_SYMBOL_IMAGE_SHOW },
    XML_TOKEN_MAP_END
};

XMLSymbolImageContext::XMLSymbolImageContext(
    SvXMLImport& rImport, sal_uInt16 nPrfx,
    const ::rtl::OUString& rLName,
    const XMLPropertyState& rProp,
    ::std::vector< XMLPropertyState > &rProps ) :
        XMLElementPropertyContext(
            rImport, nPrfx, rLName, rProp, rProps )
{
}

XMLSymbolImageContext::~XMLSymbolImageContext()
{}

void XMLSymbolImageContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLTokenMap aTokenMap( aSymbolImageAttrTokenMap );
    ::rtl::OUString aLocalName;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        const ::rtl::OUString& rAttrName = xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const ::rtl::OUString& rValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SYMBOL_IMAGE_HREF:
                msURL = rValue;
                break;
            case XML_TOK_SYMBOL_IMAGE_ACTUATE:
            case XML_TOK_SYMBOL_IMAGE_TYPE:
            case XML_TOK_SYMBOL_IMAGE_SHOW:
                // these values are currently not interpreted
                // it is always assumed 'actuate=onLoad', 'type=simple', 'show=embed'
                break;
        }
    }
}

SvXMLImportContext* XMLSymbolImageContext::CreateChildContext(
    sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext* pContext = NULL;
    if( xmloff::token::IsXMLToken( rLocalName,
                                   xmloff::token::XML_BINARY_DATA ) )
    {
        if( ! msURL.getLength() && ! mxBase64Stream.is() )
        {
            mxBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( mxBase64Stream.is() )
                pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                       rLocalName, xAttrList,
                                                       mxBase64Stream );
        }
    }
    if( ! pContext )
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void XMLSymbolImageContext::EndElement()
{
    ::rtl::OUString sResolvedURL;

    if( msURL.getLength() )
    {
        sResolvedURL = GetImport().ResolveGraphicObjectURL( msURL, sal_False );
    }
    else if( mxBase64Stream.is() )
    {
        sResolvedURL = GetImport().ResolveGraphicObjectURLFromBase64( mxBase64Stream );
        mxBase64Stream = 0;
    }

    if( sResolvedURL.getLength())
    {
        // aProp is a member of XMLElementPropertyContext
        aProp.maValue <<= sResolvedURL;
        SetInsert( sal_True );
    }

    XMLElementPropertyContext::EndElement();
}
