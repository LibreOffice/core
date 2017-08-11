/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "txtparai.hxx"

#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

XMLParaContext::XMLParaContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

XMLImportContext *XMLParaContext::CreateChildContext(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    return nullptr;
}

void XMLParaContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    mrImport.GetGenerator().openParagraph(librevenge::RVNGPropertyList());
}

void XMLParaContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeParagraph();
}

void XMLParaContext::characters(const OUString &rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrImport.GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
