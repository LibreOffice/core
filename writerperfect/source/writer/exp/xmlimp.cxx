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

XMLImport::XMLImport()
{
}

void XMLImport::startDocument()
{
}

void XMLImport::endDocument()
{
}

void XMLImport::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
}

void XMLImport::endElement(const OUString &/*rName*/)
{
}

void XMLImport::characters(const OUString &/*rChars*/)
{
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
