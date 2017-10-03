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

#include <i18nutil/oneToOneMapping.hxx>

#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace i18npool {

i18nutil::OneToOneMappingTable_t const IandE[] = {
    { 0x30A3, 0x0000 },  // KATAKANA LETTER SMALL I
    { 0x30A4, 0x0000 },  // KATAKANA LETTER I
    { 0x30A7, 0x0000 },  // KATAKANA LETTER SMALL E
    { 0x30A8, 0x0000 },  // KATAKANA LETTER E
    { 0x30AD, 0x0000 },  // KATAKANA LETTER KI
    { 0x30AE, 0x0000 },  // KATAKANA LETTER GI
    { 0x30B1, 0x0000 },  // KATAKANA LETTER KE
    { 0x30B2, 0x0000 },  // KATAKANA LETTER GE
    { 0x30B7, 0x0000 },  // KATAKANA LETTER SI
    { 0x30B8, 0x0000 },  // KATAKANA LETTER ZI
    { 0x30BB, 0x0000 },  // KATAKANA LETTER SE
    { 0x30BC, 0x0000 },  // KATAKANA LETTER ZE
    { 0x30C1, 0x0000 },  // KATAKANA LETTER TI
    { 0x30C2, 0x0000 },  // KATAKANA LETTER DI
    { 0x30C6, 0x0000 },  // KATAKANA LETTER TE
    { 0x30C7, 0x0000 },  // KATAKANA LETTER DE
    { 0x30CB, 0x0000 },  // KATAKANA LETTER NI
    { 0x30CD, 0x0000 },  // KATAKANA LETTER NE
    { 0x30D2, 0x0000 },  // KATAKANA LETTER HI
    { 0x30D3, 0x0000 },  // KATAKANA LETTER BI
    { 0x30D4, 0x0000 },  // KATAKANA LETTER PI
    { 0x30D8, 0x0000 },  // KATAKANA LETTER HE
    { 0x30D9, 0x0000 },  // KATAKANA LETTER BE
    { 0x30DA, 0x0000 },  // KATAKANA LETTER PE
    { 0x30DF, 0x0000 },  // KATAKANA LETTER MI
    { 0x30E1, 0x0000 },  // KATAKANA LETTER ME
    { 0x30EA, 0x0000 },  // KATAKANA LETTER RI
    { 0x30EC, 0x0000 },  // KATAKANA LETTER RE
    { 0x30F0, 0x0000 },  // KATAKANA LETTER WI
    { 0x30F1, 0x0000 },  // KATAKANA LETTER WE
    { 0x30F6, 0x0000 },  // KATAKANA LETTER SMALL KE
    { 0x30F8, 0x0000 },  // KATAKANA LETTER VI
    { 0x30F9, 0x0000 }   // KATAKANA LETTER VE
};


OUString SAL_CALL
ignoreIandEfollowedByYa_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
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

    // One to one mapping
    i18nutil::oneToOneMapping aTable(IandE, sizeof(IandE));

    // Translation
    while (-- nCount > 0) {
        currentChar = *src ++;

        // the character listed in above table + YA --> the character + A
        if (currentChar == 0x30E3 ||   // KATAKANA LETTER SMALL YA
                currentChar == 0x30E4) {   // KATAKANA LETTER YA
            if (aTable[ previousChar ] != previousChar) {
                if (useOffset) {
                    *p ++ = position++;
                    *p ++ = position++;
                }
                *dst ++ = previousChar;
                *dst ++ = 0x30A2;          // KATAKANA LETTER A
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

    *dst = u'\0';

    newStr->length = sal_Int32(dst - newStr->buffer);
    if (useOffset)
        offset.realloc(newStr->length);
    return OUString(newStr, SAL_NO_ACQUIRE); // take ownership
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
