/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <transliteration_Ignore.hxx>
#include <unicode/uchar.h>
#include <unicode/translit.h>

namespace com { namespace sun { namespace star { namespace i18n {

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
    throw(css::uno::RuntimeException,
        css::i18n::MultipleCharsOutputException, std::exception)
{
    if (!m_transliterator)
        throw css::uno::RuntimeException();

    UnicodeString aChar(nInChar);
    m_transliterator->transliterate(aChar);

    if (aChar.isEmpty())
        return 0xffff; // Skip this character.

    if (aChar.length() > 1)
        return nInChar; // Don't know what to do here, return the otiginal.

    return aChar[0];
}

OUString SAL_CALL
ignoreDiacritics_CTL::folding(const OUString& rInStr, sal_Int32 nStartPos,
    sal_Int32 nCount, css::uno::Sequence<sal_Int32>& rOffset)
    throw(css::uno::RuntimeException, std::exception)
{
    if (!m_transliterator)
        throw css::uno::RuntimeException();

    OUString aOutStr;

    sal_Int32 nPosition = nStartPos;
    sal_Int32 nOffset = 0;
    if (useOffset)
        rOffset.realloc(nCount);

    while (nPosition < nStartPos + nCount)
    {
        sal_Int32 nIndex = nPosition;
        UChar32 nChar = rInStr.iterateCodePoints(&nIndex);
        UnicodeString aChar(nChar);
        m_transliterator->transliterate(aChar);

        if (useOffset && nOffset + aChar.length() > rOffset.getLength())
            rOffset.realloc(rOffset.getLength() + aChar.length());

        for (int32_t i = 0; i < aChar.length(); i++)
        {
            aOutStr += OUStringLiteral1(aChar[i]);
            if (useOffset)
                rOffset[nOffset++] = nPosition;
        }

        nPosition = nIndex;
    }

    if (useOffset)
        rOffset.realloc(aOutStr.getLength());

    return aOutStr;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
