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

#include "XMLIndexSimpleEntryContext.hxx"
#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>

#include <com/sun/star/container/XNameContainer.hpp>

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_STYLE_NAME;


XMLIndexSimpleEntryContext::XMLIndexSimpleEntryContext(
    SvXMLImport& rImport,
    const OUString& rEntry,
    XMLIndexTemplateContext& rTemplate,
    sal_uInt16 nPrfx,
    const OUString& rLocalName )
:   SvXMLImportContext(rImport, nPrfx, rLocalName)
,   rEntryType(rEntry)
,   bCharStyleNameOK(false)
,   rTemplateContext(rTemplate)
,   nValues(1)
{
}

XMLIndexSimpleEntryContext::~XMLIndexSimpleEntryContext()
{
}

void XMLIndexSimpleEntryContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // we know only one attribute: style-name
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             IsXMLToken(sLocalName, XML_STYLE_NAME) )
        {
            sCharStyleName = xAttrList->getValueByIndex(nAttr);
            OUString sDisplayStyleName = GetImport().GetStyleDisplayName(
                XML_STYLE_FAMILY_TEXT_TEXT, sCharStyleName );
            // #142494#: Check if style exists
            const Reference < css::container::XNameContainer > & rStyles =
                GetImport().GetTextImport()->GetTextStyles();
            if( rStyles.is() && rStyles->hasByName( sDisplayStyleName ) )
                bCharStyleNameOK = true;
            else
                bCharStyleNameOK = false;
        }
    }

    // if we have a style name, set it!
    if (bCharStyleNameOK)
    {
        nValues++;
    }

}

void XMLIndexSimpleEntryContext::EndElement()
{
    Sequence<PropertyValue> aValues(nValues);

    FillPropertyValues(aValues);
    rTemplateContext.addTemplateEntry(aValues);
}

void XMLIndexSimpleEntryContext::FillPropertyValues(
    css::uno::Sequence<css::beans::PropertyValue> & rValues)
{
    // due to the limited number of subclasses, we fill the values
    // directly into the slots. Subclasses will have to know they can
    // only use slot so-and-so.

    Any aAny;

    // token type
    rValues[0].Name = "TokenType";
    rValues[0].Value <<= rEntryType;

    // char style
    if (bCharStyleNameOK)
    {
        rValues[1].Name = "CharacterStyleName";
        aAny <<= GetImport().GetStyleDisplayName(
                                    XML_STYLE_FAMILY_TEXT_TEXT,
                                    sCharStyleName );
        rValues[1].Value = aAny;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
