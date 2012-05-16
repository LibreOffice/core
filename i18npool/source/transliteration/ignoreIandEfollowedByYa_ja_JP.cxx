/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <utility>
#include <comphelper/string.hxx>
#include <i18nutil/oneToOneMapping.hxx>
#define TRANSLITERATION_IandEfollowedByYa_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

using ::rtl::OUString;

namespace com { namespace sun { namespace star { namespace i18n {

OneToOneMappingTable_t IandE[] = {
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
  throw(RuntimeException)
{
    // Create a string buffer which can hold nCount + 1 characters.
    // The reference count is 1 now.
    rtl_uString * newStr = comphelper::string::rtl_uString_alloc(nCount);
    sal_Unicode * dst = newStr->buffer;
    const sal_Unicode * src = inStr.getStr() + startPos;

    sal_Int32 *p = 0;
    sal_Int32 position = 0;
    if (useOffset) {
        // Allocate nCount length to offset argument.
        offset.realloc( nCount );
        p = offset.getArray();
        position = startPos;
    }

    //
    sal_Unicode previousChar = *src ++;
    sal_Unicode currentChar;

    // One to one mapping
    oneToOneMapping aTable(IandE, sizeof(IandE));

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

    *dst = (sal_Unicode) 0;

    newStr->length = sal_Int32(dst - newStr->buffer);
    if (useOffset)
        offset.realloc(newStr->length);
    return OUString(newStr, SAL_NO_ACQUIRE); // take ownership
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
