/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <transliteration_Ignore.hxx>

#include <numeric>

using namespace com::sun::star::uno;

namespace i18npool {

OUString
ignoreKiKuFollowedBySa_ja_JP::foldingImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >* pOffset )
{
    // Create a string buffer which can hold nCount + 1 characters.
    // The reference count is 1 now.
    rtl_uString * newStr = rtl_uString_alloc(nCount);
    sal_Unicode * dst = newStr->buffer;
    const sal_Unicode * src = inStr.getStr() + startPos;

    if (pOffset) {
        // Allocate nCount length to offset argument.
        pOffset->realloc( nCount );
        auto [begin, end] = asNonConstRange(*pOffset);
        std::iota(begin, end, startPos);
    }


    sal_Unicode previousChar = *src ++;
    sal_Unicode currentChar;

    // Translation
    while (-- nCount > 0) {
        currentChar = *src ++;

        // KU + Sa-So --> KI + Sa-So
        if (previousChar == 0x30AF ) { // KATAKANA LETTER KU
            if (0x30B5 <= currentChar && // KATAKANA LETTER SA
                    currentChar <= 0x30BE) { // KATAKANA LETTER ZO
                *dst ++ = 0x30AD;          // KATAKANA LETTER KI
                *dst ++ = currentChar;
                previousChar = *src ++;
                nCount --;
                continue;
            }
        }

        *dst ++ = previousChar;
        previousChar = currentChar;
    }

    if (nCount == 0) {
        *dst ++ = previousChar;
    }

    *dst = u'\0';

    newStr->length = sal_Int32(dst - newStr->buffer);
    if (pOffset)
        pOffset->realloc(newStr->length);
    return OUString(newStr, SAL_NO_ACQUIRE); // take ownership
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
