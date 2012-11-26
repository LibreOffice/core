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
#include "XMLLabelSeparatorContext.hxx"

#include "SchXMLParagraphContext.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmltkmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>

using namespace ::com::sun::star;

XMLLabelSeparatorContext::XMLLabelSeparatorContext(
    SvXMLImport& rImport, sal_uInt16 nPrfx,
    const ::rtl::OUString& rLocalName,
    const XMLPropertyState& rProp,
    ::std::vector< XMLPropertyState > &rProps ) :
        XMLElementPropertyContext(
            rImport, nPrfx, rLocalName, rProp, rProps ),
            m_aSeparator()
{
}

XMLLabelSeparatorContext::~XMLLabelSeparatorContext()
{}

void XMLLabelSeparatorContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& /*xAttrList*/ )
{
}

SvXMLImportContext* XMLLabelSeparatorContext::CreateChildContext(
    sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & /*xAttrList*/ )
{
    SvXMLImportContext* pContext = NULL;
    if( xmloff::token::IsXMLToken( rLocalName, xmloff::token::XML_P ) )
    {
        pContext = new SchXMLParagraphContext( GetImport(),
                            rLocalName, m_aSeparator );
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLLabelSeparatorContext::EndElement()
{
    if( m_aSeparator.getLength() )
    {
        // aProp is a member of XMLElementPropertyContext
        aProp.maValue <<= m_aSeparator;
        SetInsert( sal_True );
    }

    XMLElementPropertyContext::EndElement();
}
