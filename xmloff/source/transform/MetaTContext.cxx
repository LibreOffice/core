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

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>

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
                                                sal_Int32 rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLMetaTransformerContext::~XMLMetaTransformerContext()
{
}

rtl::Reference<XMLTransformerContext> XMLMetaTransformerContext::createFastChildContext(
            sal_Int32 nElement,
            const Reference< XFastAttributeList >& )
{
    rtl::Reference<XMLPersTextContentTContext> pContext(
        new XMLPersTextContentTContext( GetTransformer(), nElement ));
    XMLMetaContexts_Impl::value_type aVal( static_cast<XMLTokenEnum>(nElement & TOKEN_MASK), pContext );
    m_aContexts.insert( aVal );

    return pContext.get();
}

void XMLMetaTransformerContext::endFastElement(sal_Int32 )
{
    // export everything in the correct order
    XMLTokenEnum const *pToken = aMetaTokens;
    while( *pToken != XML_TOKEN_END )
    {
        XMLMetaContexts_Impl::const_iterator aIter =
            m_aContexts.find( *pToken );
        if( aIter != m_aContexts.end() )
        {
            if( XML_KEYWORD == *pToken )
            {
                Reference< XFastAttributeList > xAttrList =
                    new XMLMutableAttributeList;
                GetTransformer().GetDocHandler()->startFastElement( XML_ELEMENT(META, XML_KEYWORDS),
                                                            xAttrList );
            }

            // All elements may occur multiple times
            XMLMetaContexts_Impl::const_iterator aEndIter =
                m_aContexts.upper_bound( *pToken );
            while( aIter != aEndIter )
            {
                (*aIter).second->Export();
                ++aIter;
            }

            if( XML_KEYWORD == *pToken )
                GetTransformer().GetDocHandler()->endFastElement( XML_ELEMENT(META, XML_KEYWORDS) );
        }
        pToken++;
    }

    GetTransformer().GetDocHandler()->endFastElement( GetQName() );
}

void XMLMetaTransformerContext::Characters( const OUString& )
{
    // ignore them
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
