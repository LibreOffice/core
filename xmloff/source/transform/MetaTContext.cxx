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

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>

#include "TransformerBase.hxx"
#include "MutableAttrList.hxx"
#include "MetaTContext.hxx"

using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

XMLTokenEnum const aMetaTokens[] =
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

XMLMetaTransformerContext::XMLMetaTransformerContext( XMLTransformerBase& rImp,
                                                const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLMetaTransformerContext::~XMLMetaTransformerContext()
{
}

rtl::Reference<XMLTransformerContext> XMLMetaTransformerContext::CreateChildContext(
            sal_uInt16 /*nPrefix*/,
            const OUString& rLocalName,
            const OUString& rQName,
            const Reference< XAttributeList >& )
{
    rtl::Reference<XMLPersTextContentTContext> pContext(
        new XMLPersTextContentTContext( GetTransformer(), rQName ));
    XMLMetaContexts_Impl::value_type aVal( rLocalName, pContext );
    m_aContexts.insert( aVal );

    return pContext;
}

void XMLMetaTransformerContext::EndElement()
{
    // export everything in the correct order
    OUString aKeywordsQName;
    XMLTokenEnum const *pToken = aMetaTokens;
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
