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
#include <utility>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <sal/log.hxx>

#include <com/sun/star/container/XNameContainer.hpp>

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::xmloff::token::XML_STYLE_NAME;


XMLIndexSimpleEntryContext::XMLIndexSimpleEntryContext(
    SvXMLImport& rImport,
    OUString aEntry,
    XMLIndexTemplateContext& rTemplate )
:   SvXMLImportContext(rImport)
,   m_rEntryType(std::move(aEntry))
,   m_bCharStyleNameOK(false)
,   m_rTemplateContext(rTemplate)
,   m_nValues(1)
{
}

XMLIndexSimpleEntryContext::~XMLIndexSimpleEntryContext()
{
}

void XMLIndexSimpleEntryContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // we know only one attribute: style-name
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if(aIter.getToken() == XML_ELEMENT(TEXT, XML_STYLE_NAME))
        {
            m_sCharStyleName = aIter.toString();
            OUString sDisplayStyleName = GetImport().GetStyleDisplayName(
                XmlStyleFamily::TEXT_TEXT, m_sCharStyleName );
            // #142494#: Check if style exists
            const Reference < css::container::XNameContainer > & rStyles =
                GetImport().GetTextImport()->GetTextStyles();
            if( rStyles.is() && rStyles->hasByName( sDisplayStyleName ) )
                m_bCharStyleNameOK = true;
            else
                m_bCharStyleNameOK = false;
        }
        else
            XMLOFF_INFO_UNKNOWN("xmloff", aIter);
    }

    // if we have a style name, set it!
    if (m_bCharStyleNameOK)
    {
        m_nValues++;
    }

}

void XMLIndexSimpleEntryContext::endFastElement(sal_Int32 )
{
    Sequence<PropertyValue> aValues(m_nValues);

    FillPropertyValues(aValues);
    m_rTemplateContext.addTemplateEntry(aValues);
}

void XMLIndexSimpleEntryContext::FillPropertyValues(
    css::uno::Sequence<css::beans::PropertyValue> & rValues)
{
    // due to the limited number of subclasses, we fill the values
    // directly into the slots. Subclasses will have to know they can
    // only use slot so-and-so.

    Any aAny;
    auto pValues = rValues.getArray();

    // token type
    pValues[0].Name = "TokenType";
    pValues[0].Value <<= m_rEntryType;

    // char style
    if (m_bCharStyleNameOK)
    {
        pValues[1].Name = "CharacterStyleName";
        aAny <<= GetImport().GetStyleDisplayName(
                                    XmlStyleFamily::TEXT_TEXT,
                                    m_sCharStyleName );
        pValues[1].Value = aAny;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
