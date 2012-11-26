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
#include "XMLChangeInfoContext.hxx"
#include "XMLChangedRegionImportContext.hxx"
#include "XMLStringBufferImportContext.hxx"
#include <com/sun/star/uno/Reference.h>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>



using namespace ::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;


XMLChangeInfoContext::XMLChangeInfoContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    XMLChangedRegionImportContext& rPParent,
    const OUString& rChangeType)
:   SvXMLImportContext(rImport, nPrefix, rLocalName)
,   rType(rChangeType)
,   rChangedRegion(rPParent)
{
}

XMLChangeInfoContext::~XMLChangeInfoContext()
{
}

void XMLChangeInfoContext::StartElement(const Reference<XAttributeList> &)
{
    // no attributes
}

SvXMLImportContext* XMLChangeInfoContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if( XML_NAMESPACE_DC == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CREATOR ) )
            pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, sAuthorBuffer);
        else if( IsXMLToken( rLocalName, XML_DATE ) )
            pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, sDateTimeBuffer);
    }
    else if ( ( XML_NAMESPACE_TEXT == nPrefix ) &&
         IsXMLToken( rLocalName, XML_P )       )
    {
        pContext = new XMLStringBufferImportContext(GetImport(), nPrefix,
                                                   rLocalName, sCommentBuffer);
    }

    if( !pContext )
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);
    }

    return pContext;
}

void XMLChangeInfoContext::EndElement()
{
    // set values at changed region context
    rChangedRegion.SetChangeInfo(rType, sAuthorBuffer.makeStringAndClear(),
                                 sCommentBuffer.makeStringAndClear(),
                                 sDateTimeBuffer.makeStringAndClear());
}
