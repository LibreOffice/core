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
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"

#ifndef _XMLOFF_TRANSFOERMERBASE_HXX
#include "TransformerBase.hxx"
#endif
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


