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
#include <tools/debug.hxx>


#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>

#ifndef _XMLTEXTMASTERPAGECONTEXT_HXX
#include <xmloff/XMLTextMasterPageContext.hxx>
#endif
#ifndef _XMLTEXTMASTERSTYLESCONTEXT_HXX
#include <xmloff/XMLTextMasterStylesContext.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_MASTER_PAGE;

// ------------------------------------------------------------------------

sal_Bool XMLTextMasterStylesContext::InsertStyleFamily( sal_uInt16 ) const
{
    return sal_True;
}

XMLTextMasterStylesContext::XMLTextMasterStylesContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList ) :
    SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList )
{
}

XMLTextMasterStylesContext::~XMLTextMasterStylesContext()
{
}

SvXMLStyleContext *XMLTextMasterStylesContext::CreateStyleChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken( rLocalName, XML_MASTER_PAGE ) &&
         InsertStyleFamily( XML_STYLE_FAMILY_MASTER_PAGE ) )
        pContext = new XMLTextMasterPageContext(
                        GetImport(), nPrefix, rLocalName,
                          xAttrList,
                        !GetImport().GetTextImport()->IsInsertMode() );

    // any other style will be ignored here!

    return pContext;
}

SvXMLStyleContext *XMLTextMasterStylesContext::CreateStyleStyleChildContext(
        sal_uInt16 /*nFamily*/,
        sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const Reference< XAttributeList > & /*xAttrList*/ )
{
    return 0;
}
