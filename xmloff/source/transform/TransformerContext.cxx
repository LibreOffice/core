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

#ifndef _XMLOFF_TRANSFOERMERBASE_HXX
#include "TransformerBase.hxx"
#endif

#ifndef _XMLOFF_TRANSFOERMERCONTEXT_HXX
#include "TransformerContext.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

TYPEINIT0( XMLTransformerContext );

sal_Bool XMLTransformerContext::HasQName( sal_uInt16 nPrefix,
                       ::xmloff::token::XMLTokenEnum eToken ) const
{
    OUString aLocalName;
    return GetTransformer().GetNamespaceMap().GetKeyByAttrName( m_aQName,
                                              &aLocalName ) == nPrefix &&
           ::xmloff::token::IsXMLToken( aLocalName, eToken );
}

sal_Bool XMLTransformerContext::HasNamespace( sal_uInt16 nPrefix ) const
{
    return GetTransformer().GetNamespaceMap().GetKeyByAttrName( m_aQName ) == nPrefix;
}

XMLTransformerContext::XMLTransformerContext( XMLTransformerBase& rImp,
                                                const OUString& rQName ) :
    m_rTransformer( rImp ),
    m_aQName( rQName ),
    m_pRewindMap( 0 )
{
}

XMLTransformerContext::~XMLTransformerContext()
{
}

XMLTransformerContext *XMLTransformerContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLocalName,
                                            const OUString& rQName,
                                            const Reference< XAttributeList >& )
{
    return m_rTransformer.CreateContext( nPrefix, rLocalName, rQName );
}

void XMLTransformerContext::StartElement( const Reference< XAttributeList >& rAttrList )
{
    m_rTransformer.GetDocHandler()->startElement( m_aQName, rAttrList );
}

void XMLTransformerContext::EndElement()
{
    m_rTransformer.GetDocHandler()->endElement( m_aQName );
}

void XMLTransformerContext::Characters( const OUString& rChars )
{
    m_rTransformer.GetDocHandler()->characters( rChars );
}

sal_Bool XMLTransformerContext::IsPersistent() const
{
    return sal_False;
}

void XMLTransformerContext::Export()
{
    OSL_ENSURE( sal_False, "context is not persistent" );
}

void XMLTransformerContext::ExportContent()
{
    OSL_ENSURE( sal_False, "context is not persistent" );
}
