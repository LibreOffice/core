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
#include "RenameElemTContext.hxx"
#include "MutableAttrList.hxx"
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#include <xmloff/nmspmap.hxx>

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

XMLRenameElemTransformerContext::XMLRenameElemTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( nPrefix,
                            ::xmloff::token::GetXMLToken( eToken ) ) )
{
}

XMLRenameElemTransformerContext::XMLRenameElemTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
        sal_uInt16 nAPrefix,
        ::xmloff::token::XMLTokenEnum eAToken,
        ::xmloff::token::XMLTokenEnum eVToken ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( nPrefix,
                            ::xmloff::token::GetXMLToken( eToken ) ) ),
    m_aAttrQName( rImp.GetNamespaceMap().GetQNameByKey( nAPrefix,
                                    ::xmloff::token::GetXMLToken( eAToken ) ) ),
    m_aAttrValue( ::xmloff::token::GetXMLToken( eVToken ) )
{
}

XMLRenameElemTransformerContext::~XMLRenameElemTransformerContext()
{
}

void XMLRenameElemTransformerContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    Reference< XAttributeList > xAttrList( rAttrList );
    if( m_aAttrQName.getLength() )
    {
        XMLMutableAttributeList *pMutableAttrList =
            new XMLMutableAttributeList( xAttrList );
        xAttrList = pMutableAttrList;
        pMutableAttrList->AddAttribute( m_aAttrQName, m_aAttrValue );
    }
    GetTransformer().GetDocHandler()->startElement( m_aElemQName, xAttrList );
}

void XMLRenameElemTransformerContext::EndElement()
{
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}
