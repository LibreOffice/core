/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlictxt.hxx"

#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

XMLImportContext::XMLImportContext(XMLImport &rImport)
    : mrImport(rImport)
{
}

XMLImportContext *XMLImportContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    return mrImport.CreateContext(rName, xAttribs);
}

void XMLImportContext::startDocument()
{
}

void XMLImportContext::endDocument()
{
}

void XMLImportContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
}

void XMLImportContext::endElement(const OUString &/*rName*/)
{
}

void XMLImportContext::characters(const OUString &/*rChars*/)
{
}

void XMLImportContext::ignorableWhitespace(const OUString &/*rWhitespaces*/)
{
}

void XMLImportContext::processingInstruction(const OUString &/*rTarget*/, const OUString &/*rData*/)
{
}

void XMLImportContext::setDocumentLocator(const css::uno::Reference<css::xml::sax::XLocator> &/*xLocator*/)
{
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
