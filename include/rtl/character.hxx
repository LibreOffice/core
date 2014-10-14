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

#include <sal/config.h>

#include <cassert>

#include <sal/types.h>

namespace rtl
{

/** Check for ASCII character.

    @param code  A Unicode code point.

    @return  True if code is an ASCII character (0x00--0x7F).

    @since LibreOffice 4.1
 */
inline bool isAscii(sal_uInt32 code)
{
    assert(code <= 0x10FFFF);
    return code <= 0x7F;
}

/** Check for ASCII lower case character.

    @param code  A Unicode code point.

    @return  True if code is an ASCII lower case alphabetic character (ASCII
    'a'--'z').

    @since LibreOffice 4.1
 */
inline bool isAsciiLowerCase(sal_uInt32 code)
{
    assert(code <= 0x10FFFF);
    return code >= 'a' && code <= 'z';
}

/** Check for ASCII upper case character.

    @param code  A Unicode code point.

    @return  True if code is an ASCII upper case alphabetic character (ASCII
    'A'--'Z').

    @since LibreOffice 4.1
 */
inline bool isAsciiUpperCase(sal_uInt32 code)
{
    assert(code <= 0x10FFFF);
    return code >= 'A' && code <= 'Z';
}

/** Check for ASCII alphabetic character.

    @param code  A Unicode code point.

    @return  True if code is an ASCII alphabetic character (ASCII 'A'--'Z' or
    'a'--'z').

    @since LibreOffice 4.1
 */
inline bool isAsciiAlpha(sal_uInt32 code)
{
    assert(code <= 0x10FFFF);
    return isAsciiLowerCase(code) || isAsciiUpperCase(code);
}

/** Check for ASCII digit character.

    @param code  A Unicode code point.

    @return  True if code is an ASCII (decimal) digit character (ASCII
    '0'--'9').

    @since LibreOffice 4.1
 */
inline bool isAsciiDigit(sal_uInt32 code)
{
    assert(code <= 0x10FFFF);
    return code >= '0' && code <= '9';
}

/** Check for ASCII alphanumeric character.

    @param code  A Unicode code point.

    @return  True if code is an ASCII alphanumeric character (ASCII '0'--'9',
    'A'--'Z', or 'a'--'z').

    @since LibreOffice 4.1
 */
inline bool isAsciiAlphanumeric(sal_uInt32 code)
{
    assert(code <= 0x10FFFF);
    return isAsciiDigit(code) || isAsciiAlpha(code);
}

/** Check for ASCII canonic hexadecimal digit character.

    @param code  A Unicode code point.

    @return  True if code is an ASCII canonic (i.e., upper case) hexadecimal
    digit character (ASCII '0'--'9' or 'A'--'F').

    @since LibreOffice 4.1
 */
inline bool isAsciiCanonicHexDigit(sal_uInt32 code)
{
    assert(code <= 0x10FFFF);
    return isAsciiDigit(code) || (code >= 'A' && code <= 'F');
}

/** Check for ASCII hexadecimal digit character.

    @param code  A Unicode code point.

    @return  True if code is an ASCII hexadecimal digit character (ASCII
    '0'--'9', 'A'--'F', or 'a'--'f').

    @since LibreOffice 4.1
 */
inline bool isAsciiHexDigit(sal_uInt32 code)
{
    assert(code <= 0x10FFFF);
    return isAsciiCanonicHexDigit(code) || (code >= 'a' && code <= 'f');
}

/** Check for ASCII octal digit character.

    @param code  A Unicode code point.

    @return  True if code is an ASCII (octal) digit character (ASCII
    '0'--'9').

    @since LibreOffice 4.4
 */
inline bool isAsciiOctalDigit(sal_uInt32 code)
{
    assert(code <= 0x10FFFF);
    return code >= '0' && code < '8';
}

/** Convert a character, if ASCII, to upper case.

    @param code  A Unicode code point.

    @return  code converted to ASCII upper case.

    @since LibreOffice 4.2
*/
inline sal_uInt32 toAsciiUpperCase(sal_uInt32 code)
{
    assert(code <= 0x10FFFF);
    return isAsciiLowerCase(code) ? code - 32 : code;
}

/** Convert a character, if ASCII, to lower case.

    @param code  A Unicode code point.

    @return  code converted to ASCII lower case.

    @since LibreOffice 4.2
*/
inline sal_uInt32 toAsciiLowerCase(sal_uInt32 code)
{
    assert(code <= 0x10FFFF);
    return isAsciiUpperCase(code) ? code + 32 : code;
}

/** Compare two characters ignoring ASCII case.

    @param code1  A Unicode code point.

    @param code2  A unicode code point.

    @return  0 if both code points are equal,
             < 0 if code1 is less than code2,
             > 0 if code1 is greater than code2.

    @since LibreOffice 4.2
 */
inline sal_Int32 compareIgnoreAsciiCase(sal_uInt32 code1, sal_uInt32 code2)
{
    assert(code1 <= 0x10FFFF);
    assert(code2 <= 0x10FFFF);
    return static_cast<sal_Int32>(toAsciiLowerCase(code1))
        - static_cast<sal_Int32>(toAsciiLowerCase(code2));
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
