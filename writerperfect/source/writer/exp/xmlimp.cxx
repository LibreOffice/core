/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

XMLImport::XMLImport(librevenge::RVNGTextInterface &rGenerator)
    : mrGenerator(rGenerator)
{
}

void XMLImport::startDocument()
{
    mrGenerator.startDocument(librevenge::RVNGPropertyList());
}

void XMLImport::endDocument()
{
    mrGenerator.endDocument();
}

void XMLImport::startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "text:p")
    {
        mrGenerator.openParagraph(librevenge::RVNGPropertyList());
        mbParagraphOpened = true;
    }
}

void XMLImport::endElement(const OUString &rName)
{
    if (rName == "text:p")
    {
        mrGenerator.closeParagraph();
        mbParagraphOpened = false;
    }
}

void XMLImport::characters(const OUString &rChars)
{
    if (mbParagraphOpened)
    {
        OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
        mrGenerator.insertText(librevenge::RVNGString(sCharU8.getStr()));
    }
}

void XMLImport::ignorableWhitespace(const OUString &/*rWhitespaces*/)
{
}

void XMLImport::processingInstruction(const OUString &/*rTarget*/, const OUString &/*rData*/)
{
    SAL_WARN("writerperfect", "XMLImport::processingInstruction: implement me");
}

void XMLImport::setDocumentLocator(const css::uno::Reference<css::xml::sax::XLocator> &/*xLocator*/)
{
    SAL_WARN("writerperfect", "XMLImport::setDocumentLocator: implement me");
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
