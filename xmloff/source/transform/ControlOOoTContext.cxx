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
#include "ControlOOoTContext.hxx"
#include "IgnoreTContext.hxx"
#include "MutableAttrList.hxx"
#include "xmloff/xmlnmspe.hxx"
#include "ActionMapTypesOOo.hxx"
#include "ElemTransformerAction.hxx"
#include "TransformerActions.hxx"
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

XMLControlOOoTransformerContext::XMLControlOOoTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLControlOOoTransformerContext::~XMLControlOOoTransformerContext()
{
}

void XMLControlOOoTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );
}

XMLTransformerContext *XMLControlOOoTransformerContext::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = 0;

    if( !m_aElemQName.getLength() )
    {
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName,
                                                    sal_False, sal_False );
        m_aElemQName = rQName;
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                ->AppendAttributeList( rAttrList );
        GetTransformer().ProcessAttrList( m_xAttrList,
                                          OOO_FORM_CONTROL_ACTIONS,
                                          sal_False );
        GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                        m_xAttrList );
    }
    else
    {
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName,
                                                    sal_True, sal_True );
    }
    return pContext;
}

void XMLControlOOoTransformerContext::EndElement()
{
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

void XMLControlOOoTransformerContext::Characters( const OUString& rChars )
{
    // ignore
    if( m_aElemQName.getLength() )
        XMLTransformerContext::Characters( rChars );
}
