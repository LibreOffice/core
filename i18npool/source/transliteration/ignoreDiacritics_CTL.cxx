/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <transliteration_Ignore.hxx>
#include <unicode/translit.h>

namespace i18npool {

ignoreDiacritics_CTL::ignoreDiacritics_CTL()
{
    func = nullptr;
    table = nullptr;
    map = nullptr;
    transliterationName = "ignoreDiacritics_CTL";
    implementationName = "com.sun.star.i18n.Transliteration.ignoreDiacritics_CTL";

    UErrorCode nStatus = U_ZERO_ERROR;
    m_transliterator = icu::Transliterator::createInstance("NFD; [:M:] Remove; NFC",
            UTRANS_FORWARD, nStatus);
    if (U_FAILURE(nStatus))
        m_transliterator = nullptr;
}

sal_Unicode SAL_CALL
ignoreDiacritics_CTL::transliterateChar2Char(sal_Unicode nInChar)
{
    if (!m_transliterator)
        throw css::uno::RuntimeException();

    icu::UnicodeString aChar(nInChar);
    m_transliterator->transliterate(aChar);

    if (aChar.isEmpty())
        return 0xffff; // Skip this character.

    if (aChar.length() > 1)
        return nInChar; // Don't know what to do here, return the original.

    return aChar[0];
}

OUString
ignoreDiacritics_CTL::foldingImpl(const OUString& rInStr, sal_Int32 nStartPos,
    sal_Int32 nCount, css::uno::Sequence<sal_Int32>& rOffset, bool useOffset)
{
    if (!m_transliterator)
        throw css::uno::RuntimeException();

    if (nStartPos < 0 || nStartPos + nCount > rInStr.getLength())
        throw css::uno::RuntimeException();

    if (useOffset)
    {
        OUStringBuffer aOutBuf(nCount);

        std::vector<sal_Int32> aOffset;
        aOffset.reserve(nCount);

        sal_Int32 nPosition = nStartPos;
        while (nPosition < nStartPos + nCount)
        {
            sal_Int32 nIndex = nPosition;
            UChar32 nChar = rInStr.iterateCodePoints(&nIndex);
            icu::UnicodeString aUStr(nChar);
            m_transliterator->transliterate(aUStr);

            aOutBuf.append(reinterpret_cast<const sal_Unicode*>(aUStr.getBuffer()), aUStr.length());

            std::fill_n(std::back_inserter(aOffset), aUStr.length(), nPosition);

            nPosition = nIndex;
        }

        rOffset = comphelper::containerToSequence(aOffset);
        return aOutBuf.makeStringAndClear();
    }
    else
    {
        icu::UnicodeString aUStr(reinterpret_cast<const UChar*>(rInStr.getStr()) + nStartPos, nCount);
        m_transliterator->transliterate(aUStr);
        return OUString(reinterpret_cast<const sal_Unicode*>(aUStr.getBuffer()), aUStr.length());
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
