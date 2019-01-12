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

#include "XMLIndexTitleTemplateContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>


using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_STYLE_NAME;


XMLIndexTitleTemplateContext::XMLIndexTitleTemplateContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet,
    sal_uInt16 nPrfx,
    const OUString& rLocalName)
:   SvXMLImportContext(rImport, nPrfx, rLocalName)
,   bStyleNameOK(false)
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
            const Reference < css::container::XNameContainer >&
                rStyles = GetImport().GetTextImport()->GetParaStyles();
            bStyleNameOK = rStyles.is() && rStyles->hasByName( sDisplayStyleName );
        }
    }
}

void XMLIndexTitleTemplateContext::EndElement()
{
    Any aAny;

    aAny <<= sContent.makeStringAndClear();
    rTOCPropertySet->setPropertyValue("Title", aAny);

    if (bStyleNameOK)
    {
        aAny <<= GetImport().GetStyleDisplayName(
                                XML_STYLE_FAMILY_TEXT_PARAGRAPH,
                                sStyleName );
        rTOCPropertySet->setPropertyValue("ParaStyleHeading", aAny);
    }
}

void XMLIndexTitleTemplateContext::Characters(
    const OUString& sString)
{
    sContent.append(sString);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
