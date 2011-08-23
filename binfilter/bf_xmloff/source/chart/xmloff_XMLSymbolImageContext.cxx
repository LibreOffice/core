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
#include "XMLSymbolImageContext.hxx"

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLBASE64IMPORTCONTEXT_HXX
#include "XMLBase64ImportContext.hxx"
#endif

namespace binfilter {

TYPEINIT1( XMLSymbolImageContext, XMLElementPropertyContext );

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
    { XML_NAMESPACE_XLINK,  ::binfilter::xmloff::token::XML_HREF,     XML_TOK_SYMBOL_IMAGE_HREF    },
    { XML_NAMESPACE_XLINK,  ::binfilter::xmloff::token::XML_TYPE,     XML_TOK_SYMBOL_IMAGE_TYPE    },
    { XML_NAMESPACE_XLINK,  ::binfilter::xmloff::token::XML_ACTUATE,  XML_TOK_SYMBOL_IMAGE_ACTUATE },
    { XML_NAMESPACE_XLINK,  ::binfilter::xmloff::token::XML_SHOW,     XML_TOK_SYMBOL_IMAGE_SHOW },
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
}//end of namespace binfilter
