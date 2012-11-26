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
#include "IgnoreTContext.hxx"
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

XMLIgnoreTransformerContext::XMLIgnoreTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_Bool bIgnoreChars,
        sal_Bool bIgnoreElems ) :
    XMLTransformerContext( rImp, rQName ),
    m_bIgnoreCharacters( bIgnoreChars ),
    m_bIgnoreElements( bIgnoreElems ),
    m_bRecursiveUse( sal_False )
{
}

XMLIgnoreTransformerContext::XMLIgnoreTransformerContext(
        XMLTransformerBase& rTransformer,
        const ::rtl::OUString& rQName,
        sal_Bool bAllowCharactersRecursive ) :
    XMLTransformerContext( rTransformer, rQName ),
    m_bIgnoreCharacters( sal_False ),
    m_bIgnoreElements( sal_False ),
    m_bAllowCharactersRecursive( bAllowCharactersRecursive ),
    m_bRecursiveUse( sal_True )
{
}

XMLIgnoreTransformerContext::~XMLIgnoreTransformerContext()
{
}

XMLTransformerContext *XMLIgnoreTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& xAttrList )
{
    XMLTransformerContext *pContext = 0;
    if( m_bIgnoreElements )
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName, sal_True,
                                                    sal_True );
    else if (m_bRecursiveUse)
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName, m_bAllowCharactersRecursive );
    else
        pContext = XMLTransformerContext::CreateChildContext(
                        nPrefix, rLocalName, rQName, xAttrList );

    return pContext;
}

void XMLIgnoreTransformerContext::StartElement( const Reference< XAttributeList >& )
{
    // ignore
}

void XMLIgnoreTransformerContext::EndElement()
{
    // ignore
}

void XMLIgnoreTransformerContext::Characters( const OUString& rChars )
{
    if( !m_bIgnoreCharacters )
        GetTransformer().GetDocHandler()->characters( rChars );
    else if ( m_bRecursiveUse && m_bAllowCharactersRecursive )
        GetTransformer().GetDocHandler()->characters( rChars );
}


