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
#include "XMLStringBufferImportContext.hxx"
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"


using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_P;


XMLStringBufferImportContext::XMLStringBufferImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& sLocalName,
    OUStringBuffer& rBuffer) :
    SvXMLImportContext(rImport, nPrefix, sLocalName),
    rTextBuffer(rBuffer)
{
}

XMLStringBufferImportContext::~XMLStringBufferImportContext()
{
}

SvXMLImportContext *XMLStringBufferImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> &)
{
    return new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, rTextBuffer);
}

void XMLStringBufferImportContext::Characters(
    const OUString& rChars )
{
    rTextBuffer.append(rChars);
}

void XMLStringBufferImportContext::EndElement()
{
    // add return for paragraph elements
    if ( (XML_NAMESPACE_TEXT == GetPrefix()) &&
         (IsXMLToken(GetLocalName(), XML_P))    )
    {
        rTextBuffer.append(sal_Unicode(0x0a));
    }
}

