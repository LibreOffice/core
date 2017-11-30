/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLMETAI_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLMETAI_HXX

#include <librevenge/librevenge.h>

#include "xmlictxt.hxx"

namespace writerperfect
{
namespace exp
{

/// Handler for <office:meta>.
class XMLMetaDocumentContext : public XMLImportContext
{
public:
    XMLMetaDocumentContext(XMLImport &rImport);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    void SAL_CALL endElement(const OUString &rName) override;

    librevenge::RVNGPropertyList m_aPropertyList;
};

/// Parses an XMP file.
class XMPParser: public cppu::WeakImplHelper
    <
    css::xml::sax::XDocumentHandler
    >
{
public:
    explicit XMPParser();
    virtual ~XMPParser() override;

    // XDocumentHandler
    virtual void SAL_CALL startDocument() override;

    virtual void SAL_CALL endDocument() override;

    virtual void SAL_CALL startElement(const OUString &aName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    virtual void SAL_CALL endElement(const OUString &aName) override;

    virtual void SAL_CALL characters(const OUString &aChars) override;

    virtual void SAL_CALL ignorableWhitespace(const OUString &aWhitespaces) override;

    virtual void SAL_CALL processingInstruction(const OUString &aTarget, const OUString &aData) override;

    virtual void SAL_CALL setDocumentLocator(const css::uno::Reference<css::xml::sax::XLocator> &xLocator) override;

    OUString m_aIdentifier;
    OUString m_aTitle;
    OUString m_aCreator;
    OUString m_aLanguage;
    OUString m_aDate;

private:
    bool m_bInIdentifier = false;
    bool m_bInTitle = false;
    bool m_bInTitleItem = false;
    bool m_bInCreator = false;
    bool m_bInCreatorItem = false;
    bool m_bInLanguage = false;
    bool m_bInLanguageItem = false;
    bool m_bInDate = false;
    bool m_bInDateItem = false;
};

} // namespace exp
} // namespace writerperfect

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
