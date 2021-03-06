/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <librevenge/librevenge.h>

#include "xmlictxt.hxx"

namespace writerperfect::exp
{
/// Handler for <office:meta>.
class XMLMetaDocumentContext : public XMLImportContext
{
public:
    XMLMetaDocumentContext(XMLImport& rImport);

    librevenge::RVNGPropertyList& GetPropertyList() { return m_aPropertyList; }

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    void SAL_CALL endElement(const OUString& rName) override;

private:
    librevenge::RVNGPropertyList m_aPropertyList;
};

/// Parses an XMP file.
class XMPParser : public cppu::WeakImplHelper<css::xml::sax::XDocumentHandler>
{
public:
    explicit XMPParser(librevenge::RVNGPropertyList& rMetaData);
    ~XMPParser() override;

    // XDocumentHandler
    void SAL_CALL startDocument() override;

    void SAL_CALL endDocument() override;

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    void SAL_CALL endElement(const OUString& rName) override;

    void SAL_CALL characters(const OUString& rChars) override;

    void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces) override;

    void SAL_CALL processingInstruction(const OUString& aTarget, const OUString& aData) override;

    void SAL_CALL
    setDocumentLocator(const css::uno::Reference<css::xml::sax::XLocator>& xLocator) override;

private:
    librevenge::RVNGPropertyList& mrMetaData;
    bool m_bInIdentifier = false;
    OUString m_aIdentifier;
    bool m_bInTitle = false;
    bool m_bInTitleItem = false;
    OUString m_aTitle;
    bool m_bInCreator = false;
    bool m_bInCreatorItem = false;
    OUString m_aCreator;
    bool m_bInLanguage = false;
    bool m_bInLanguageItem = false;
    OUString m_aLanguage;
    bool m_bInDate = false;
    bool m_bInDateItem = false;
    OUString m_aDate;
};

} // namespace writerperfect::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
