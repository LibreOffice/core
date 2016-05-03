/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "XMLSectionSourceImportContext.hxx"
#include "XMLSectionImportContext.hxx"
#include <com/sun/star/text/SectionFileLink.hpp>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/XPropertySet.hpp>


using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;


XMLSectionSourceImportContext::XMLSectionSourceImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rSectPropSet) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rSectionPropertySet(rSectPropSet)
{
}

XMLSectionSourceImportContext::~XMLSectionSourceImportContext()
{
}

enum XMLSectionSourceToken
{
    XML_TOK_SECTION_XLINK_HREF,
    XML_TOK_SECTION_TEXT_FILTER_NAME,
    XML_TOK_SECTION_TEXT_SECTION_NAME
};

static const SvXMLTokenMapEntry aSectionSourceTokenMap[] =
{
    { XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_SECTION_XLINK_HREF },
    { XML_NAMESPACE_TEXT, XML_FILTER_NAME, XML_TOK_SECTION_TEXT_FILTER_NAME },
    { XML_NAMESPACE_TEXT, XML_SECTION_NAME,
                                        XML_TOK_SECTION_TEXT_SECTION_NAME },
    XML_TOKEN_MAP_END
};


void XMLSectionSourceImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLTokenMap aTokenMap(aSectionSourceTokenMap);
    OUString sURL;
    OUString sFilterName;
    OUString sSectionName;

    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );

        switch (aTokenMap.Get(nPrefix, sLocalName))
        {
            case XML_TOK_SECTION_XLINK_HREF:
                sURL = xAttrList->getValueByIndex(nAttr);
                break;

            case XML_TOK_SECTION_TEXT_FILTER_NAME:
                sFilterName = xAttrList->getValueByIndex(nAttr);
                break;

            case XML_TOK_SECTION_TEXT_SECTION_NAME:
                sSectionName = xAttrList->getValueByIndex(nAttr);
                break;

            default:
                ; // ignore
                break;
        }
    }

    // we only need them once
    const OUString sFileLink("FileLink");
    const OUString sLinkRegion("LinkRegion");

    if (!sURL.isEmpty() || !sFilterName.isEmpty())
    {
        SectionFileLink aFileLink;
        aFileLink.FileURL = GetImport().GetAbsoluteReference( sURL );
        aFileLink.FilterName = sFilterName;

        rSectionPropertySet->setPropertyValue(sFileLink, Any(aFileLink));
    }

    if (!sSectionName.isEmpty())
    {
        rSectionPropertySet->setPropertyValue(sLinkRegion, Any(sSectionName));
    }
}

void XMLSectionSourceImportContext::EndElement()
{
    // this space intentionally left blank.
}

SvXMLImportContext* XMLSectionSourceImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & )
{
    // ignore -> default context
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
