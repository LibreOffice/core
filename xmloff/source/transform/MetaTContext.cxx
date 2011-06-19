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
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"

#include "TransformerBase.hxx"
#include "MutableAttrList.hxx"
#include "MetaTContext.hxx"

using ::rtl::OUString;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

XMLTokenEnum aMetaTokens[] =
{
    XML_GENERATOR,
    XML_TITLE,
    XML_DESCRIPTION,
    XML_SUBJECT,
    XML_INITIAL_CREATOR,
    XML_CREATION_DATE,
    XML_CREATOR,
    XML_DATE,
    XML_PRINTED_BY,
    XML_PRINT_DATE,
    XML_KEYWORD,
    XML_LANGUAGE,
    XML_EDITING_CYCLES,
    XML_EDITING_DURATION,
    XML_HYPERLINK_BEHAVIOUR,
    XML_AUTO_RELOAD,
    XML_TEMPLATE,
    XML_USER_DEFINED,
    XML_DOCUMENT_STATISTIC,
    XML_TOKEN_END
};

TYPEINIT1( XMLMetaTransformerContext, XMLTransformerContext );

XMLMetaTransformerContext::XMLMetaTransformerContext( XMLTransformerBase& rImp,
                                                const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLMetaTransformerContext::~XMLMetaTransformerContext()
{
}

XMLTransformerContext *XMLMetaTransformerContext::CreateChildContext(
            sal_uInt16 /*nPrefix*/,
            const OUString& rLocalName,
            const OUString& rQName,
            const Reference< XAttributeList >& )
{
    XMLPersTextContentTContext *pContext =
        new XMLPersTextContentTContext( GetTransformer(), rQName );
    XMLMetaContexts_Impl::value_type aVal( rLocalName, pContext );
    m_aContexts.insert( aVal );

    return pContext;
}

void XMLMetaTransformerContext::EndElement()
{
    // export everything in the correct order
    OUString aKeywordsQName;
    XMLTokenEnum *pToken = aMetaTokens;
    while( *pToken != XML_TOKEN_END )
    {
        const OUString& rToken = GetXMLToken( *pToken );
        XMLMetaContexts_Impl::const_iterator aIter =
            m_aContexts.find( rToken );
        if( aIter != m_aContexts.end() )
        {
            if( XML_KEYWORD == *pToken )
            {
                aKeywordsQName =
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_META, GetXMLToken(XML_KEYWORDS ) );

                Reference< XAttributeList > xAttrList =
                    new XMLMutableAttributeList;
                GetTransformer().GetDocHandler()->startElement( aKeywordsQName,
                                                            xAttrList );
            }

            // All elements may occur multiple times
            XMLMetaContexts_Impl::const_iterator aEndIter =
                m_aContexts.upper_bound( rToken );
            while( aIter != aEndIter )
            {
                (*aIter).second->Export();
                ++aIter;
            }

            if( XML_KEYWORD == *pToken )
                GetTransformer().GetDocHandler()->endElement( aKeywordsQName );
        }
        pToken++;
    }

    GetTransformer().GetDocHandler()->endElement( GetQName() );
}

void XMLMetaTransformerContext::Characters( const OUString& )
{
    // ignore them
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
