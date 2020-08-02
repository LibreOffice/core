/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLBase64ImportContext.hxx"

#include <comphelper/base64.hxx>

using namespace com::sun::star;

namespace writerperfect::exp
{
XMLBase64ImportContext::XMLBase64ImportContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
}

void XMLBase64ImportContext::startElement(
    const OUString& /*rName*/,
    const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
}

void XMLBase64ImportContext::endElement(const OUString& /*rName*/)
{
    m_aBinaryData.append(static_cast<const unsigned char*>(m_aStream.GetData()),
                         m_aStream.GetSize());
}

void XMLBase64ImportContext::characters(const OUString& rChars)
{
    OUString aTrimmedChars(rChars.trim());

    if (aTrimmedChars.isEmpty())
        return;

    OUString aChars;
    if (!m_aBase64CharsLeft.isEmpty())
    {
        aChars = m_aBase64CharsLeft + aTrimmedChars;
        m_aBase64CharsLeft.clear();
    }
    else
        aChars = aTrimmedChars;

    uno::Sequence<sal_Int8> aBuffer((aChars.getLength() / 4) * 3);
    const sal_Int32 nCharsDecoded = comphelper::Base64::decodeSomeChars(aBuffer, aChars);
    m_aStream.WriteBytes(aBuffer.getArray(), aBuffer.getLength());
    if (nCharsDecoded != aChars.getLength())
        m_aBase64CharsLeft = aChars.copy(nCharsDecoded);
}

const librevenge::RVNGBinaryData& XMLBase64ImportContext::getBinaryData() const
{
    return m_aBinaryData;
}

} // namespace writerperfect::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
