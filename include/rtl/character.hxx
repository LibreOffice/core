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

/** Check for Unicode code point.

    @param code  An integer.

    @return  True if code is a Unicode code point.

    @since LibreOffice 5.2
*/
inline bool isUnicodeCodePoint(sal_uInt32 code)
{
    return code <= 0x10FFFF;
}

/** Check for ASCII character.

    @param code  A Unicode code point.

    @return  True if code is an ASCII character (0x00--0x7F).

    @since LibreOffice 4.1
 */
inline bool isAscii(sal_uInt32 code)
{
    assert(isUnicodeCodePoint(code));
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
    assert(isUnicodeCodePoint(code));
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
    assert(isUnicodeCodePoint(code));
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
    assert(isUnicodeCodePoint(code));
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
    assert(isUnicodeCodePoint(code));
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
    assert(isUnicodeCodePoint(code));
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
    assert(isUnicodeCodePoint(code));
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
    assert(isUnicodeCodePoint(code));
    return isAsciiCanonicHexDigit(code) || (code >= 'a' && code <= 'f');
}

/** Check for ASCII octal digit character.

    @param code  A Unicode code point.

    @return  True if code is an ASCII octal digit character (ASCII '0'--'7').

    @since LibreOffice 5.0
 */
inline bool isAsciiOctalDigit(sal_uInt32 code)
{
    assert(isUnicodeCodePoint(code));
    return code >= '0' && code <= '7';
}


/** Convert a character, if ASCII, to upper case.

    @param code  A Unicode code point.

    @return  code converted to ASCII upper case.

    @since LibreOffice 4.2
*/
inline sal_uInt32 toAsciiUpperCase(sal_uInt32 code)
{
    assert(isUnicodeCodePoint(code));
    return isAsciiLowerCase(code) ? code - 32 : code;
}

/** Convert a character, if ASCII, to lower case.

    @param code  A Unicode code point.

    @return  code converted to ASCII lower case.

    @since LibreOffice 4.2
*/
inline sal_uInt32 toAsciiLowerCase(sal_uInt32 code)
{
    assert(isUnicodeCodePoint(code));
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
    assert(isUnicodeCodePoint(code1));
    assert(isUnicodeCodePoint(code2));
    return static_cast<sal_Int32>(toAsciiLowerCase(code1))
        - static_cast<sal_Int32>(toAsciiLowerCase(code2));
}

/// @cond INTERNAL
namespace detail {

sal_uInt32 const surrogatesHighFirst = 0xD800;
sal_uInt32 const surrogatesHighLast = 0xDBFF;
sal_uInt32 const surrogatesLowFirst = 0xDC00;
sal_uInt32 const surrogatesLowLast = 0xDFFF;

}
/// @endcond

/** Check for high surrogate.

    @param code  A Unicode code point.

    @return  True if code is a high surrogate code point (0xD800--0xDBFF).

    @since LibreOffice 5.0
*/
inline bool isHighSurrogate(sal_uInt32 code) {
    assert(isUnicodeCodePoint(code));
    return code >= detail::surrogatesHighFirst
        && code <= detail::surrogatesHighLast;
}

/** Check for low surrogate.

    @param code  A Unicode code point.

    @return  True if code is a low surrogate code point (0xDC00--0xDFFF).

    @since LibreOffice 5.0
*/
inline bool isLowSurrogate(sal_uInt32 code) {
    assert(isUnicodeCodePoint(code));
    return code >= detail::surrogatesLowFirst
        && code <= detail::surrogatesLowLast;
}

/** Get high surrogate half of a non-BMP Unicode code point.

    @param code  A non-BMP Unicode code point.

    @return  The UTF-16 high surrogate half for the give code point.

    @since LibreOffice 5.0
 */
inline sal_Unicode getHighSurrogate(sal_uInt32 code) {
    assert(isUnicodeCodePoint(code));
    assert(code >= 0x10000);
    return static_cast<sal_Unicode>(((code - 0x10000) >> 10) | detail::surrogatesHighFirst);
}

/** Get low surrogate half of a non-BMP Unicode code point.

    @param code  A non-BMP Unicode code point.

    @return  The UTF-16 low surrogate half for the give code point.

    @since LibreOffice 5.0
 */
inline sal_Unicode getLowSurrogate(sal_uInt32 code) {
    assert(isUnicodeCodePoint(code));
    assert(code >= 0x10000);
    return static_cast<sal_Unicode>(((code - 0x10000) & 0x3FF) | detail::surrogatesLowFirst);
}

/** Combine surrogates to form a code point.

    @param high  A high surrogate code point.

    @param low  A low surrogate code point.

    @return  The code point represented by the surrogate pair.

    @since LibreOffice 5.0
*/
inline sal_uInt32 combineSurrogates(sal_uInt32 high, sal_uInt32 low) {
    assert(isHighSurrogate(high));
    assert(isLowSurrogate(low));
    return ((high - detail::surrogatesHighFirst) << 10)
        + (low - detail::surrogatesLowFirst) + 0x10000;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
