/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlimp.hxx"

#include "xmlfmt.hxx"
#include "xmlictxt.hxx"
#include "xmlmetai.hxx"
#include "xmltext.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

/// Handler for <office:body>.
class XMLBodyContext : public XMLImportContext
{
public:
    XMLBodyContext(XMLImport &rImport);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/) override;
};

XMLBodyContext::XMLBodyContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLBodyContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "office:text")
        return new XMLBodyContentContext(mrImport);
    return nullptr;
}

/// Handler for <office:document>.
class XMLOfficeDocContext : public XMLImportContext
{
public:
    XMLOfficeDocContext(XMLImport &rImport);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/) override;
};

XMLOfficeDocContext::XMLOfficeDocContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLOfficeDocContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "office:body")
        return new XMLBodyContext(mrImport);
    else if (rName == "office:meta")
        return new XMLMetaDocumentContext(mrImport);
    else if (rName == "office:automatic-styles")
        return new XMLStylesContext(mrImport, /*bAutomatic=*/true);
    else if (rName == "office:styles")
        return new XMLStylesContext(mrImport, /*bAutomatic=*/false);
    else if (rName == "office:font-face-decls")
        return new XMLFontFaceDeclsContext(mrImport);
    return nullptr;
}

XMLImport::XMLImport(librevenge::RVNGTextInterface &rGenerator)
    : mrGenerator(rGenerator)
{
}

rtl::Reference<XMLImportContext> XMLImport::CreateContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "office:document")
        return new XMLOfficeDocContext(*this);
    return nullptr;
}

librevenge::RVNGTextInterface &XMLImport::GetGenerator() const
{
    return mrGenerator;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticTextStyles()
{
    return maAutomaticTextStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticParagraphStyles()
{
    return maAutomaticParagraphStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticCellStyles()
{
    return maAutomaticCellStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticColumnStyles()
{
    return maAutomaticColumnStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticRowStyles()
{
    return maAutomaticRowStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticTableStyles()
{
    return maAutomaticTableStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticGraphicStyles()
{
    return maAutomaticGraphicStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetTextStyles()
{
    return maTextStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetParagraphStyles()
{
    return maParagraphStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetCellStyles()
{
    return maCellStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetColumnStyles()
{
    return maColumnStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetRowStyles()
{
    return maRowStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetTableStyles()
{
    return maTableStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetGraphicStyles()
{
    return maGraphicStyles;
}

void XMLImport::startDocument()
{
    mrGenerator.startDocument(librevenge::RVNGPropertyList());
}

void XMLImport::endDocument()
{
    mrGenerator.endDocument();
}

void XMLImport::startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    rtl::Reference<XMLImportContext> xContext;
    if (!maContexts.empty())
    {
        if (maContexts.top().is())
            xContext = maContexts.top()->CreateChildContext(rName, xAttribs);
    }
    else
        xContext = CreateContext(rName, xAttribs);

    if (xContext.is())
        xContext->startElement(rName, xAttribs);

    maContexts.push(xContext);
}

void XMLImport::endElement(const OUString &rName)
{
    if (maContexts.empty())
        return;

    if (maContexts.top().is())
        maContexts.top()->endElement(rName);

    maContexts.pop();
}

void XMLImport::characters(const OUString &rChars)
{
    if (maContexts.top().is())
        maContexts.top()->characters(rChars);
}

void XMLImport::ignorableWhitespace(const OUString &/*rWhitespaces*/)
{
}

void XMLImport::processingInstruction(const OUString &/*rTarget*/, const OUString &/*rData*/)
{
}

void XMLImport::setDocumentLocator(const css::uno::Reference<css::xml::sax::XLocator> &/*xLocator*/)
{
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
