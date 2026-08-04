/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <comphelper/attributelist.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/token/namespaces.hxx>

#include <map>

class NamespaceAdjustmentHandler : public cppu::WeakImplHelper<css::xml::sax::XDocumentHandler>
{
    const std::map<OUString, sal_Int32> m_aNamespaces{
        { "xmlns:r", OOX_NS(officeRel) },
        { "xmlns:w", OOX_NS(doc) },
    };

    const oox::core::XmlFilterBase& m_rFilter;
    css::uno::Reference<css::xml::sax::XDocumentHandler> m_xNext;

public:
    NamespaceAdjustmentHandler(
        oox::core::XmlFilterBase& rFilter, css::uno::Reference<css::xml::sax::XDocumentHandler>
            xNext)
        : m_rFilter(rFilter), m_xNext(std::move(xNext))
    {
    }

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override
    {
        rtl::Reference<comphelper::AttributeList> pList = new comphelper::AttributeList;

        const sal_Int16 nLen = xAttribs.is() ? xAttribs->getLength() : 0;
        for (sal_Int16 i = 0; i < nLen; ++i)
        {
            const OUString aName = xAttribs->getNameByIndex(i);
            auto it = m_aNamespaces.find(aName);
            if (it != m_aNamespaces.end())
            {
                pList->AddAttribute(aName, m_rFilter.getNamespaceURL(it->second));
            }
            else
            {
                pList->AddAttribute(aName, xAttribs->getValueByIndex(i));
            }
        }

        m_xNext->startElement(rName, pList);
    }

    void SAL_CALL startDocument() override { m_xNext->startDocument(); }
    void SAL_CALL endDocument() override { m_xNext->endDocument(); }
    void SAL_CALL endElement(const OUString& rName) override { m_xNext->endElement(rName); }
    void SAL_CALL characters(const OUString& rChars) override { m_xNext->characters(rChars); }
    void SAL_CALL ignorableWhitespace(const OUString& rWs) override
    {
        m_xNext->ignorableWhitespace(rWs);
    }
    void SAL_CALL processingInstruction(const OUString& rTarget, const OUString& rData) override
    {
        m_xNext->processingInstruction(rTarget, rData);
    }
    void SAL_CALL
    setDocumentLocator(const css::uno::Reference<css::xml::sax::XLocator>& xLocator) override
    {
        m_xNext->setDocumentLocator(xLocator);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
