/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLFootnoteImportContext.hxx"

#include "xmlimp.hxx"
#include "xmltext.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{
/// Handler for <text:note-citation>.
class XMLTextNoteCitationContext : public XMLImportContext
{
public:
    XMLTextNoteCitationContext(XMLImport& rImport, librevenge::RVNGPropertyList& rProperties);

    void SAL_CALL characters(const OUString& rCharacters) override;
    void SAL_CALL endElement(const OUString& rName) override;

private:
    librevenge::RVNGPropertyList& m_rProperties;
    OUString m_aCharacters;
};

XMLTextNoteCitationContext::XMLTextNoteCitationContext(XMLImport& rImport,
                                                       librevenge::RVNGPropertyList& rProperties)
    : XMLImportContext(rImport)
    , m_rProperties(rProperties)
{
}

void XMLTextNoteCitationContext::endElement(const OUString& /*rName*/)
{
    m_rProperties.insert("librevenge:number", m_aCharacters.toUtf8().getStr());
}

void XMLTextNoteCitationContext::characters(const OUString& rCharacters)
{
    m_aCharacters += rCharacters;
}

/// Handler for <text:note-body>.
class XMLFootnoteBodyImportContext : public XMLImportContext
{
public:
    XMLFootnoteBodyImportContext(XMLImport& rImport,
                                 const librevenge::RVNGPropertyList& rProperties);

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
    void SAL_CALL endElement(const OUString& rName) override;

private:
    const librevenge::RVNGPropertyList& m_rProperties;
};

XMLFootnoteBodyImportContext::XMLFootnoteBodyImportContext(
    XMLImport& rImport, const librevenge::RVNGPropertyList& rProperties)
    : XMLImportContext(rImport)
    , m_rProperties(rProperties)
{
}

rtl::Reference<XMLImportContext> XMLFootnoteBodyImportContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    return CreateTextChildContext(mrImport, rName);
}

void XMLFootnoteBodyImportContext::startElement(
    const OUString& /*rName*/,
    const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    mrImport.GetGenerator().openFootnote(m_rProperties);
}

void XMLFootnoteBodyImportContext::endElement(const OUString& /*rName*/)
{
    mrImport.GetGenerator().closeFootnote();
}

XMLFootnoteImportContext::XMLFootnoteImportContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLFootnoteImportContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "text:note-citation")
        return new XMLTextNoteCitationContext(mrImport, m_aProperties);
    if (rName == "text:note-body")
        return new XMLFootnoteBodyImportContext(mrImport, m_aProperties);
    SAL_WARN("writerperfect", "XMLFootnoteImportContext::CreateChildContext: unhandled " << rName);
    return nullptr;
}

void XMLFootnoteImportContext::startElement(
    const OUString& /*rName*/,
    const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
}
} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
