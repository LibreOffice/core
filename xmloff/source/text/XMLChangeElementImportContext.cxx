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
#include "XMLChangeElementImportContext.hxx"
#include "XMLChangedRegionImportContext.hxx"
#include "XMLChangeInfoContext.hxx"
#include <com/sun/star/uno/Reference.h>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>



using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_P;
using ::xmloff::token::XML_CHANGE_INFO;

XMLChangeElementImportContext::XMLChangeElementImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    sal_Bool bAccContent,
    XMLChangedRegionImportContext& rParent) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        bAcceptContent(bAccContent),
        rChangedRegion(rParent)
{
}

SvXMLImportContext* XMLChangeElementImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLImportContext* pContext = NULL;

    if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken( rLocalName, XML_CHANGE_INFO) )
    {
        pContext = new XMLChangeInfoContext(GetImport(), nPrefix, rLocalName,
                                            rChangedRegion, GetLocalName());
    }
    else
    {
        // import into redline -> create XText
        rChangedRegion.UseRedlineText();

        pContext = GetImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList,
            XML_TEXT_TYPE_CHANGED_REGION);

        if (NULL == pContext)
        {
            // no text element -> use default
            pContext = SvXMLImportContext::CreateChildContext(
                nPrefix, rLocalName, xAttrList);

            // illegal element content! TODO: discard this redline!
        }
    }


    return pContext;
}

// #107848#
void XMLChangeElementImportContext::StartElement( const Reference< XAttributeList >& )
{
    if(bAcceptContent)
    {
        GetImport().GetTextImport()->SetInsideDeleteContext(sal_True);
    }
}

// #107848#
void XMLChangeElementImportContext::EndElement()
{
    if(bAcceptContent)
    {
        GetImport().GetTextImport()->SetInsideDeleteContext(sal_False);
    }
}
