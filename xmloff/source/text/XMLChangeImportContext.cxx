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
#include "XMLChangeImportContext.hxx"
#include <com/sun/star/text/XTextRange.hpp>
#include <tools/debug.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_CHANGE_ID;

XMLChangeImportContext::XMLChangeImportContext(
    SvXMLImport& rImport,
    sal_Int16 nPrefix,
    const OUString& rLocalName,
    sal_Bool bStart,
    sal_Bool bEnd,
    sal_Bool bOutsideOfParagraph) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        bIsStart(bStart),
        bIsEnd(bEnd),
        bIsOutsideOfParagraph(bOutsideOfParagraph)
{
    DBG_ASSERT(bStart || bEnd, "Must be either start, end, or both!");
}

XMLChangeImportContext::~XMLChangeImportContext()
{
}

void XMLChangeImportContext::StartElement(
    const Reference<XAttributeList>& xAttrList)
{
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             IsXMLToken( sLocalName, XML_CHANGE_ID ) )
        {
            // Id found! Now call RedlineImportHelper

            // prepare parameters
            UniReference<XMLTextImportHelper> rHelper =
                GetImport().GetTextImport();
            OUString sID = xAttrList->getValueByIndex(nAttr);

            // call for bStart and bEnd (may both be true)
            if (bIsStart)
                rHelper->RedlineSetCursor(sID,sal_True,bIsOutsideOfParagraph);
            if (bIsEnd)
                rHelper->RedlineSetCursor(sID,sal_False,bIsOutsideOfParagraph);

            // outside of paragraph and still open? set open redline ID
            if (bIsOutsideOfParagraph)
            {
                rHelper->SetOpenRedlineId(sID);
            }
        }
        // else: ignore
    }
}
