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

#ifndef INCLUDED_RTL_CHARACTER_HXX
#define INCLUDED_RTL_CHARACTER_HXX

#include "sal/config.h"
#include "sal/types.h"
#include "sal/log.hxx"

#include <assert.h>

namespace rtl
{
/** Check for ASCII character.

    @param nUtf32  A Unicode scalar value (represented as a UTF-32 code unit).

    @return  True if nChar is a ASCII character (0x00--0x7F).

    @since LibreOffice 4.1
 */
inline bool isAscii(sal_uInt32 nUtf32)
{
    return nUtf32 <= 0x7F;
}

/** Check for ASCII lower case character.

    @param nUtf32  A Unicode scalar value (represented as a UTF-32 code unit).

    @return  True if nChar is a US-ASCII lower case alphabetic character
    (ASCII 'a'--'z').

    @since LibreOffice 4.1
 */
inline bool isAsciiLowerCase(sal_uInt32 nUtf32)
{
    return nUtf32 >= 'a' && nUtf32 <= 'z';
}

/** Check for US-ASCII upper case character.

    @param nUtf32  A Unicode scalar value (represented as a UTF-32 code unit).

    @return  True if nChar is a US-ASCII upper case alphabetic character
    (US-ASCII 'A'--'Z').

    @since LibreOffice 4.1
 */
inline bool isAsciiUpperCase(sal_uInt32 nUtf32)
{
    return nUtf32 >= 'A' && nUtf32 <= 'Z';
}

/** Check for ASCII alphanumeric character.

    @param nUtf32  A Unicode scalar value (represented as a UTF-32 code unit).

    @return  True if nUtf32 is a US-ASCII alphanumeric character
    (ASCII '0'--'9', 'A'--'Z' or 'a'--'z').

    @since LibreOffice 4.1
 */
inline bool isAsciiAlpha(sal_uInt32 nUtf32)
{
    return isAsciiLowerCase(nUtf32) || isAsciiUpperCase(nUtf32);
}

/** Check for ASCII digit character.

    @param nUtf32  A Unicode scalar value (represented as a UTF-32 code unit).

    @return  True if nChar is a ASCII (decimal) digit character
    (ASCII '0'--'9').

    @since LibreOffice 4.1
 */
inline bool isAsciiDigit(sal_uInt32 nUtf32)
{
    return nUtf32 >= '0' && nUtf32 <= '9';
}

/** Check for US-ASCII alphanumeric character.

    @param nUtf32  A Unicode scalar value (represented as a UTF-32 code unit).

    @return  True if nChar is a US-ASCII alphanumeric character (US-ASCII
    '0'--'9', 'A'--'Z' or 'a'--'z').

    @since LibreOffice 4.1
 */
inline bool isAsciiAlphanumeric(sal_uInt32 nUtf32)
{
    return isAsciiDigit(nUtf32) || isAsciiAlpha(nUtf32);
}

/** Check for US-ASCII canonic hexadecimal digit character.

    @param nUtf32  A Unicode scalar value (represented as a UTF-32 code unit).

    @return  True if nChar is a US-ASCII canonic (i.e., upper case)
    hexadecimal digit character (US-ASCII '0'--'9' or 'A'--'F').

    @since LibreOffice 4.1
 */
inline bool isAsciiCanonicHexDigit(sal_uInt32 nUtf32)
{
    return isAsciiDigit(nUtf32) || (nUtf32 >= 'A' && nUtf32 <= 'F');
}

/** Check for US-ASCII hexadecimal digit character.

    @param nUtf32  A Unicode scalar value (represented as a UTF-32 code unit).

    @return  True if nChar is a US-ASCII hexadecimal digit character (US-
    ASCII '0'--'9', 'A'--'F', 'a'--'f').

    @since LibreOffice 4.1
 */
inline bool isAsciiHexDigit(sal_uInt32 nUtf32)
{
    return isAsciiCanonicHexDigit(nUtf32) || (nUtf32 >= 'a' && nUtf32 <= 'f');
}

/** Compare two US-ASCII characters.

    @param nChar1 A Unicode scalar value (represented as a UTF-32 code unit).
    @param nChar2 A unicode scalar value (represented as a UTF-32 code unit).

    @return
        0 if both strings are equal
        < 0 - if this string is less than the string argument
        > 0 - if this string is greater than the string argument

    @since LibreOffice 4.2
 */
inline sal_Int32 compareAsciiIgnoreCase(sal_uInt32 nChar1, sal_uInt32 nChar2)
{
    assert(isAscii(nChar1) && isAscii(nChar2));
    if ( isAsciiUpperCase(nChar1) )
        nChar1 += 32;
    if ( isAsciiUpperCase(nChar2) )
        nChar2 += 32;
    return nChar1 - nChar2;
}


}//rtl namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
