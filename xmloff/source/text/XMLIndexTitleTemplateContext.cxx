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
#include "XMLIndexTitleTemplateContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>


using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_STYLE_NAME;


const sal_Char sAPI_Title[] = "Title";
const sal_Char sAPI_ParaStyleHeading[] = "ParaStyleHeading";


XMLIndexTitleTemplateContext::XMLIndexTitleTemplateContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet,
    sal_uInt16 nPrfx,
    const OUString& rLocalName)
:   SvXMLImportContext(rImport, nPrfx, rLocalName)
,   sTitle(RTL_CONSTASCII_USTRINGPARAM(sAPI_Title))
,   sParaStyleHeading(RTL_CONSTASCII_USTRINGPARAM(sAPI_ParaStyleHeading))
,   bStyleNameOK(sal_False)
,   rTOCPropertySet(rPropSet)
{
}


XMLIndexTitleTemplateContext::~XMLIndexTitleTemplateContext()
{
}

void XMLIndexTitleTemplateContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // there's only one attribute: style-name
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             (IsXMLToken(sLocalName, XML_STYLE_NAME)) )
        {
            sStyleName = xAttrList->getValueByIndex(nAttr);
            OUString sDisplayStyleName = GetImport().GetStyleDisplayName(
                XML_STYLE_FAMILY_TEXT_PARAGRAPH, sStyleName );
            const Reference < ::com::sun::star::container::XNameContainer >&
                rStyles = GetImport().GetTextImport()->GetParaStyles();
            bStyleNameOK = rStyles.is() && rStyles->hasByName( sDisplayStyleName );
        }
    }
}

void XMLIndexTitleTemplateContext::EndElement()
{
    Any aAny;

    aAny <<= sContent.makeStringAndClear();
    rTOCPropertySet->setPropertyValue(sTitle, aAny);

    if (bStyleNameOK)
    {
        aAny <<= GetImport().GetStyleDisplayName(
                                XML_STYLE_FAMILY_TEXT_PARAGRAPH,
                                sStyleName );
        rTOCPropertySet->setPropertyValue(sParaStyleHeading, aAny);
    }
}

void XMLIndexTitleTemplateContext::Characters(
    const OUString& sString)
{
    sContent.append(sString);
}
