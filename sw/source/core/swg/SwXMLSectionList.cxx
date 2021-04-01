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

#include <SwXMLSectionList.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltkmap.hxx>
#include <vector>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

namespace {

class SvXMLSectionListContext : public SvXMLImportContext
{
private:
    SwXMLSectionList & GetImport() { return static_cast<SwXMLSectionList&>(SvXMLImportContext::GetImport()); }

public:
    SvXMLSectionListContext(SwXMLSectionList& rImport);

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;
};

class SwXMLParentContext : public SvXMLImportContext
{
private:
    SwXMLSectionList & GetImport() { return static_cast<SwXMLSectionList&>(SvXMLImportContext::GetImport()); }

public:
    SwXMLParentContext(SwXMLSectionList& rImport)
        : SvXMLImportContext(rImport)
    {
    }

    virtual css::uno::Reference<XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*xAttrList*/ ) override
    {
        if (Element == XML_ELEMENT(OFFICE, XML_BODY) ||
            Element == XML_ELEMENT(OFFICE_OOO, XML_BODY))
        {
            return new SvXMLSectionListContext(GetImport());
        }
        if (IsTokenInNamespace(Element, XML_NAMESPACE_TEXT) ||
            IsTokenInNamespace(Element, XML_NAMESPACE_TEXT_OOO))
        {
            auto nToken = Element & TOKEN_MASK;
            if (nToken == XML_P ||
                nToken == XML_H ||
                nToken == XML_A ||
                nToken == XML_SPAN ||
                nToken == XML_SECTION ||
                nToken == XML_INDEX_BODY ||
                nToken == XML_INDEX_TITLE ||
                nToken == XML_INSERTION ||
                nToken == XML_DELETION)
                return new SvXMLSectionListContext(GetImport());
        }
        return new SwXMLParentContext(GetImport());
    }
};

}

SwXMLSectionList::SwXMLSectionList(const css::uno::Reference< css::uno::XComponentContext >& rContext, std::vector<OUString> &rNewSectionList)
: SvXMLImport(rContext, "")
, m_rSectionList(rNewSectionList)
{
}

SwXMLSectionList::~SwXMLSectionList()
    throw()
{
}

SvXMLImportContext * SwXMLSectionList::CreateFastContext(
        sal_Int32 /*Element*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    return new SwXMLParentContext(*this);
}

SvXMLSectionListContext::SvXMLSectionListContext( SwXMLSectionList& rImport )
    : SvXMLImportContext ( rImport )
{
}

css::uno::Reference<css::xml::sax::XFastContextHandler> SvXMLSectionListContext::createFastChildContext(
        sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    if (Element == XML_ELEMENT(TEXT, XML_SECTION ) ||
        Element == XML_ELEMENT(TEXT, XML_BOOKMARK) ||
        Element == XML_ELEMENT(TEXT_OOO, XML_SECTION ) ||
        Element == XML_ELEMENT(TEXT_OOO, XML_BOOKMARK) )
    {
        OUString sName;
        for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
            if (aIter.getToken() == XML_ELEMENT(TEXT, XML_NAME) ||
                aIter.getToken() == XML_ELEMENT(TEXT_OOO, XML_NAME))
                sName = aIter.toString();
        if ( !sName.isEmpty() )
            GetImport().m_rSectionList.push_back(sName);
    }

    pContext = new SvXMLSectionListContext(GetImport());
    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
