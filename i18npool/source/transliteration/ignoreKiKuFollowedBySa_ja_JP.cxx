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

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL
ignoreKiKuFollowedBySa_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException, std::exception)
{
    // Create a string buffer which can hold nCount + 1 characters.
    // The reference count is 1 now.
    rtl_uString * newStr = rtl_uString_alloc(nCount);
    sal_Unicode * dst = newStr->buffer;
    const sal_Unicode * src = inStr.getStr() + startPos;

    sal_Int32 *p = nullptr;
    sal_Int32 position = 0;
    if (useOffset) {
        // Allocate nCount length to offset argument.
        offset.realloc( nCount );
        p = offset.getArray();
        position = startPos;
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
                if (useOffset) {
                    *p ++ = position++;
                    *p ++ = position++;
                }
                *dst ++ = 0x30AD;          // KATAKANA LETTER KI
                *dst ++ = currentChar;
                previousChar = *src ++;
                nCount --;
                continue;
            }
        }

        if (useOffset)
            *p ++ = position++;
        *dst ++ = previousChar;
        previousChar = currentChar;
    }

    if (nCount == 0) {
        if (useOffset)
            *p = position;
        *dst ++ = previousChar;
    }

    *dst = (sal_Unicode) 0;

    newStr->length = sal_Int32(dst - newStr->buffer);
    if (useOffset)
        offset.realloc(newStr->length);
    return OUString(newStr, SAL_NO_ACQUIRE); // take ownership
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
