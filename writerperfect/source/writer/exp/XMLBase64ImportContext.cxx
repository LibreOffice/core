/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLBase64ImportContext.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

XMLBase64ImportContext::XMLBase64ImportContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

void XMLBase64ImportContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
}

void XMLBase64ImportContext::endElement(const OUString &/*rName*/)
{
}

void XMLBase64ImportContext::characters(const OUString &rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    m_aBinaryData.appendBase64Data(librevenge::RVNGString(sCharU8.getStr()));
}

const librevenge::RVNGBinaryData &XMLBase64ImportContext::getBinaryData() const
{
    return m_aBinaryData;
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
