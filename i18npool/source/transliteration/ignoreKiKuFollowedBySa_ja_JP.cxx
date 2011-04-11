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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_KiKuFollowedBySa_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

using ::rtl::OUString;

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL
ignoreKiKuFollowedBySa_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    // Create a string buffer which can hold nCount + 1 characters.
    // The reference count is 0 now.
    rtl_uString * newStr = x_rtl_uString_new_WithLength( nCount ); // defined in x_rtl_ustring.h
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
    return OUString( newStr ); // defined in rtl/usrting. The reference count is increased from 0 to 1.
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
