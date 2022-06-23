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

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <limits>
#include <new>
#include <string_view>
#include <type_traits>
#include <utility>

#include "strimp.hxx"

#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <rtl/character.hxx>
#include <rtl/math.h>
#include <rtl/string.h>
#include <rtl/ustring.h>

#include <dragonbox/dragonbox.h>

void internRelease(rtl_uString*);

namespace rtl::str
{
template <typename C> auto IMPL_RTL_USTRCODE(C c) { return std::make_unsigned_t<C>(c); }

// Wrappers around null-terminated/known-length strings, that allow to generalize algorithms
// without overhead (e.g., without need to get length of null-terminated strings).

template <typename C> struct null_terminated
{
    C* p;
    null_terminated(C* pStr)
        : p(pStr)
    {
        assert(pStr);
    }
    auto begin() const { return p; }
    struct EndDetector
    {
        friend bool operator==(EndDetector, C* iter) { return *iter == 0; }
        friend bool operator==(C* iter, EndDetector) { return *iter == 0; }
        friend bool operator!=(EndDetector, C* iter) { return *iter != 0; }
        friend bool operator!=(C* iter, EndDetector) { return *iter != 0; }
    };
    static auto end() { return EndDetector{}; }
};

template <typename C> struct with_length
{
    C* p;
    sal_Int32 len;
    with_length(C* pStr, sal_Int32 nLength)
        : p(pStr)
        , len(nLength)
    {
        assert(len >= 0);
    }
    auto begin() const { return p; }
    auto end() const { return p + len; }
};

struct ToAsciiLower
{
    template <typename C> static bool Applicable(C c)
    {
        return rtl::isAsciiUpperCase(IMPL_RTL_USTRCODE(c));
    }
    template <typename C> static C Replace(C c)
    {
        return rtl::toAsciiLowerCase(IMPL_RTL_USTRCODE(c));
    }
} constexpr toAsciiLower;

struct ToAsciiUpper
{
    template <typename C> static bool Applicable(C c)
    {
        return rtl::isAsciiLowerCase(IMPL_RTL_USTRCODE(c));
    }
    template <typename C> static C Replace(C c)
    {
        return rtl::toAsciiUpperCase(IMPL_RTL_USTRCODE(c));
    }
} constexpr toAsciiUpper;

template <typename C> struct FromTo
{
    C from;
    C to;
    FromTo(C cFrom, C cTo) : from(cFrom), to(cTo) {}
    C Replace(C c) const { return c == from ? to : c; }
};

template <typename C> void Copy(C* _pDest, const C* _pSrc, sal_Int32 _nCount)
{
    // take advantage of builtin optimisations
    std::copy(_pSrc, _pSrc + _nCount, _pDest);
}

template <typename C> void CopyBackward(C* _pDest, const C* _pSrc, sal_Int32 _nCount)
{
    // take advantage of builtin optimisations
    std::copy_backward(_pSrc, _pSrc + _nCount, _pDest + _nCount);
}

inline void Copy(sal_Unicode* _pDest, const char* _pSrc, sal_Int32 _nCount)
{
    std::transform(_pSrc, _pSrc + _nCount, _pDest,
                   [](char c)
                   {
                       assert(rtl::isAscii(static_cast<unsigned char>(c)));
                       SAL_WARN_IF(c == '\0', "rtl.string", "Found embedded \\0 ASCII character");
                       return static_cast<unsigned char>(c);
                   });
}

inline sal_Int16 implGetDigit(sal_Unicode ch, sal_Int16 nRadix)
{
    sal_Int16 n = -1;
    if ((ch >= '0') && (ch <= '9'))
        n = ch - '0';
    else if ((ch >= 'a') && (ch <= 'z'))
        n = ch - 'a' + 10;
    else if ((ch >= 'A') && (ch <= 'Z'))
        n = ch - 'A' + 10;
    return (n < nRadix) ? n : -1;
}

inline bool implIsWhitespace(sal_Unicode c)
{
    /* Space or Control character? */
    if ((c <= 32) && c)
        return true;

    /* Only in the General Punctuation area Space or Control characters are included? */
    if ((c < 0x2000) || (c > 0x206F))
        return false;

    if (((c >= 0x2000) && (c <= 0x200B)) ||    /* All Spaces           */
        (c == 0x2028) ||                       /* LINE SEPARATOR       */
        (c == 0x2029))                         /* PARAGRAPH SEPARATOR  */
        return true;

    return false;
}

/* ======================================================================= */
/* C-String functions which could be used without the String-Class         */
/* ======================================================================= */

template <typename T> sal_Int32 getLength( const T* pStr )
{
    assert(pStr);
    if constexpr (std::is_class_v<T>)
    {
        return pStr->length;
    }
    else
    {
        // take advantage of builtin optimisations
        return std::char_traits<T>::length(pStr);
    }
}

/* ----------------------------------------------------------------------- */

template <typename C> void warnIfCharAndNotAscii(C c)
{
    if constexpr (sizeof(c) == sizeof(char))
        SAL_WARN_IF(!rtl::isAscii(static_cast<unsigned char>(c)), "rtl.string",
                    "Found non-ASCII char");
}

template <typename C1, typename C2> void warnIfOneIsCharAndNotAscii(C1 c1, C2 c2)
{
    if constexpr (sizeof(c1) != sizeof(c2))
    {
        warnIfCharAndNotAscii(c1);
        warnIfCharAndNotAscii(c2);
    }
}

struct CompareNormal
{
    template <typename C1, typename C2> static sal_Int32 compare(C1 c1, C2 c2)
    {
        warnIfOneIsCharAndNotAscii(c1, c2);
        return static_cast<sal_Int32>(IMPL_RTL_USTRCODE(c1))
               - static_cast<sal_Int32>(IMPL_RTL_USTRCODE(c2));
    }
};

struct CompareIgnoreAsciiCase
{
    template <typename C1, typename C2> static sal_Int32 compare(C1 c1, C2 c2)
    {
        warnIfOneIsCharAndNotAscii(c1, c2);
        return rtl::compareIgnoreAsciiCase(IMPL_RTL_USTRCODE(c1), IMPL_RTL_USTRCODE(c2));
    }
};

/* ----------------------------------------------------------------------- */

struct NoShortening
{
    constexpr bool operator>=(int) { return true; } // for assert
    constexpr bool operator==(int) { return false; } // for loop break check
    constexpr void operator--() {} // for decrement in loop
} constexpr noShortening;

template <class S1, class S2, class Compare, typename Shorten_t>
sal_Int32 compare(S1 s1, S2 s2, Compare, Shorten_t shortenedLength)
{
    static_assert(std::is_same_v<Shorten_t, NoShortening> || std::is_same_v<Shorten_t, sal_Int32>);
    assert(shortenedLength >= 0);
    auto pStr1 = s1.begin();
    const auto end1 = s1.end();
    auto pStr2 = s2.begin();
    const auto end2 = s2.end();
    for (;;)
    {
        if (shortenedLength == 0)
            return 0;
        if (pStr2 == end2)
            return pStr1 == end1 ? 0 : 1;
        if (pStr1 == end1)
            return -1;
        if (const sal_Int32 nRet = Compare::compare(*pStr1, *pStr2))
            return nRet;
        --shortenedLength;
        ++pStr1;
        ++pStr2;
    }
}

// take advantage of builtin optimisations
template <typename C, std::enable_if_t<sizeof(C) == sizeof(wchar_t), int> = 0>
sal_Int32 compare(null_terminated<C> s1, null_terminated<C> s2, CompareNormal, NoShortening)
{
    return wcscmp(reinterpret_cast<wchar_t const*>(s1.p), reinterpret_cast<wchar_t const*>(s2.p));
}
template <typename C, std::enable_if_t<sizeof(C) == sizeof(char), int> = 0>
sal_Int32 compare(null_terminated<C> s1, null_terminated<C> s2, CompareNormal, NoShortening)
{
    return strcmp(reinterpret_cast<char const*>(s1.p), reinterpret_cast<char const*>(s2.p));
}
template <typename C>
sal_Int32 compare(with_length<C> s1, with_length<C> s2, CompareNormal, NoShortening)
{
    std::basic_string_view sv1(s1.p, s1.len);
    return sv1.compare(std::basic_string_view(s2.p, s2.len));
}
template <typename C1, typename C2, class Compare>
sal_Int32 compare(with_length<C1> s1, with_length<C2> s2, Compare cf, sal_Int32 nShortenedLength)
{
    assert(nShortenedLength >= 0);
    s1.len = std::min(s1.len, nShortenedLength);
    s2.len = std::min(s2.len, nShortenedLength);
    return compare(s1, s2, cf, noShortening);
}

/* ----------------------------------------------------------------------- */

template <typename C1, typename C2, class Compare>
sal_Int32 reverseCompare_WithLengths(const C1* pStr1, sal_Int32 nStr1Len,
                                     const C2* pStr2, sal_Int32 nStr2Len, Compare)
{
    assert(pStr1 || nStr1Len == 0);
    assert(nStr1Len >= 0);
    assert(pStr2 || nStr2Len == 0);
    assert(nStr2Len >= 0);
    const C1* pStr1Run = pStr1+nStr1Len;
    const C2* pStr2Run = pStr2+nStr2Len;
    while ((pStr1 < pStr1Run) && (pStr2 < pStr2Run))
    {
        pStr1Run--;
        pStr2Run--;
        if (const sal_Int32 nRet = Compare::compare(*pStr1Run, *pStr2Run))
            return nRet;
    }

    return nStr1Len - nStr2Len;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 hashCode_WithLength(const IMPL_RTL_STRCODE*, sal_Int32);

template <typename IMPL_RTL_STRCODE> sal_Int32 hashCode( const IMPL_RTL_STRCODE* pStr )
{
    return hashCode_WithLength( pStr, getLength( pStr ) );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 hashCode_WithLength                             ( const IMPL_RTL_STRCODE* pStr,
                                                            sal_Int32 nLen )
{
    assert(nLen >= 0);
    sal_uInt32 h = static_cast<sal_uInt32>(nLen);
    while ( nLen > 0 )
    {
        h = (h*37U) + IMPL_RTL_USTRCODE( *pStr );
        pStr++;
        nLen--;
    }
    return static_cast<sal_Int32>(h);
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 indexOfChar                             ( const IMPL_RTL_STRCODE* pStr,
                                                    IMPL_RTL_STRCODE c )
{
    assert(pStr);
    if (!c)
        return -1; // Unifies behavior of strchr/wcschr and unoptimized algorithm wrt '\0'

    if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(char))
    {
        // take advantage of builtin optimisations
        const IMPL_RTL_STRCODE* p = strchr(pStr, c);
        return p ? p - pStr : -1;
    }
    else if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        wchar_t const * p = wcschr(reinterpret_cast<wchar_t const *>(pStr), static_cast<wchar_t>(c));
        return p ? p - reinterpret_cast<wchar_t const *>(pStr) : -1;
    }
    else
    {
        const IMPL_RTL_STRCODE* pTempStr = pStr;
        while ( *pTempStr )
        {
            if ( *pTempStr == c )
                return pTempStr-pStr;

            pTempStr++;
        }

        return -1;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 indexOfChar_WithLength                             ( const IMPL_RTL_STRCODE* pStr,
                                                               sal_Int32 nLen,
                                                               IMPL_RTL_STRCODE c )
{
//    assert(nLen >= 0);
    if (nLen <= 0)
        return -1;
    // take advantage of builtin optimisations
    using my_string_view = std::basic_string_view<IMPL_RTL_STRCODE>;
    my_string_view v(pStr, nLen);
    typename my_string_view::size_type idx = v.find(c);
    return idx == my_string_view::npos ? -1 : idx;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 lastIndexOfChar_WithLength(const IMPL_RTL_STRCODE*, sal_Int32, IMPL_RTL_STRCODE);

template <typename IMPL_RTL_STRCODE>
sal_Int32 lastIndexOfChar                             ( const IMPL_RTL_STRCODE* pStr,
                                                        IMPL_RTL_STRCODE c )
{
    assert(pStr);
    if (!c)
        return -1; // Unifies behavior of strrchr/wcsrchr and lastIndexOfChar_WithLength wrt '\0'

    if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(char))
    {
        // take advantage of builtin optimisations
        const IMPL_RTL_STRCODE* p = strrchr(pStr, c);
        return p ? p - pStr : -1;
    }
    else if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        wchar_t const * p = wcsrchr(reinterpret_cast<wchar_t const *>(pStr), static_cast<wchar_t>(c));
        return p ? p - reinterpret_cast<wchar_t const *>(pStr) : -1;
    }
    else
    {
        return lastIndexOfChar_WithLength( pStr, getLength( pStr ), c );
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 lastIndexOfChar_WithLength                             ( const IMPL_RTL_STRCODE* pStr,
                                                                   sal_Int32 nLen,
                                                                   IMPL_RTL_STRCODE c )
{
    assert(nLen >= 0);
    // take advantage of builtin optimisations
    using my_string_view = std::basic_string_view<IMPL_RTL_STRCODE>;
    my_string_view v(pStr, nLen);
    typename my_string_view::size_type idx = v.rfind(c);
    return idx == my_string_view::npos ? -1 : idx;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 indexOfStr_WithLength(const IMPL_RTL_STRCODE*, sal_Int32, const IMPL_RTL_STRCODE*,
                                sal_Int32);

template <typename IMPL_RTL_STRCODE>
sal_Int32 indexOfStr                             ( const IMPL_RTL_STRCODE* pStr,
                                                   const IMPL_RTL_STRCODE* pSubStr )
{
    assert(pStr);
    assert(pSubStr);
    /* an empty SubString is always not findable */
    if (*pSubStr == 0)
        return -1;
    if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(char))
    {
        // take advantage of builtin optimisations
        const IMPL_RTL_STRCODE* p = strstr(pStr, pSubStr);
        return p ? p - pStr : -1;
    }
    else if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        wchar_t const * p = wcsstr(reinterpret_cast<wchar_t const *>(pStr), reinterpret_cast<wchar_t const *>(pSubStr));
        return p ? p - reinterpret_cast<wchar_t const *>(pStr) : -1;
    }
    else
    {
        return indexOfStr_WithLength( pStr, getLength( pStr ),
                                                         pSubStr, getLength( pSubStr ) );
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 indexOfStr_WithLength                             ( const IMPL_RTL_STRCODE* pStr,
                                                              sal_Int32 nStrLen,
                                                              const  IMPL_RTL_STRCODE* pSubStr,
                                                              sal_Int32 nSubLen )
{
    assert(nStrLen >= 0);
    assert(nSubLen >= 0);
    /* an empty SubString is always not findable */
    if ( nSubLen == 0 )
        return -1;
    // take advantage of builtin optimisations
    using my_string_view = std::basic_string_view<IMPL_RTL_STRCODE>;
    my_string_view v(pStr, nStrLen);
    auto idx = nSubLen == 1 ? v.find(*pSubStr) : v.find(pSubStr, 0, nSubLen);
    return idx == my_string_view::npos ? -1 : idx;
}

inline sal_Int32 indexOfStr_WithLength(const sal_Unicode* pStr, sal_Int32 nStrLen,
                                       const char* pSubStr, sal_Int32 nSubLen)
{
    assert(nStrLen >= 0);
    assert(nSubLen >= 0);
    if (nSubLen > 0 && nSubLen <= nStrLen)
    {
        sal_Unicode const* end = pStr + nStrLen;
        sal_Unicode const* cursor = pStr;

        while (cursor < end)
        {
            cursor = std::char_traits<sal_Unicode>::find(cursor, end - cursor, *pSubStr);
            if (!cursor || (end - cursor < nSubLen))
            {
                /* no enough left to actually have a match */
                break;
            }
            /* now it is worth trying a full match */
            if (nSubLen == 1 || rtl_ustr_asciil_reverseEquals_WithLength(cursor, pSubStr, nSubLen))
            {
                return cursor - pStr;
            }
            cursor += 1;
        }
    }
    return -1;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 lastIndexOfStr_WithLength(const IMPL_RTL_STRCODE*, sal_Int32, const IMPL_RTL_STRCODE*,
                                    sal_Int32);

template <typename IMPL_RTL_STRCODE>
sal_Int32 lastIndexOfStr                             ( const IMPL_RTL_STRCODE* pStr,
                                                       const IMPL_RTL_STRCODE* pSubStr )
{
    return lastIndexOfStr_WithLength( pStr, getLength( pStr ),
                                                          pSubStr, getLength( pSubStr ) );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 lastIndexOfStr_WithLength                             ( const IMPL_RTL_STRCODE* pStr,
                                                                  sal_Int32 nStrLen,
                                                                  const IMPL_RTL_STRCODE* pSubStr,
                                                                  sal_Int32 nSubLen )
{
    assert(nStrLen >= 0);
    assert(nSubLen >= 0);
    /* an empty SubString is always not findable */
    if ( nSubLen == 0 )
        return -1;
    // take advantage of builtin optimisations
    using my_string_view = std::basic_string_view<IMPL_RTL_STRCODE>;
    my_string_view v(pStr, nStrLen);
    my_string_view needle(pSubStr, nSubLen);
    typename my_string_view::size_type idx = v.rfind(needle);
    return idx == my_string_view::npos ? -1 : idx;
}

/* ----------------------------------------------------------------------- */

template <class S, class Replacer> void replaceChars(S str, Replacer replacer)
{
    for (auto& rChar : str)
        rChar = replacer.Replace(rChar);
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE> sal_Int32 trim_WithLength(IMPL_RTL_STRCODE*, sal_Int32);

template <typename IMPL_RTL_STRCODE> sal_Int32 trim( IMPL_RTL_STRCODE* pStr )
{
    return trim_WithLength( pStr, getLength( pStr ) );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
std::basic_string_view<IMPL_RTL_STRCODE> trimView( IMPL_RTL_STRCODE* pStr, sal_Int32 nLen )
{
    assert(nLen >= 0);
    sal_Int32 nPreSpaces    = 0;
    sal_Int32 nPostSpaces   = 0;
    sal_Int32 nIndex        = nLen-1;

    while ( (nPreSpaces < nLen) && implIsWhitespace( IMPL_RTL_USTRCODE(*(pStr+nPreSpaces)) ) )
        nPreSpaces++;

    while ( (nIndex > nPreSpaces) && implIsWhitespace( IMPL_RTL_USTRCODE(*(pStr+nIndex)) ) )
    {
        nPostSpaces++;
        nIndex--;
    }

    return { pStr + nPreSpaces, static_cast<size_t>(nLen - nPostSpaces - nPreSpaces) };
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 trim_WithLength( IMPL_RTL_STRCODE* pStr, sal_Int32 nLen )
{
    const auto view = trimView(pStr, nLen);

    if (static_cast<sal_Int32>(view.size()) != nLen)
    {
        nLen = static_cast<sal_Int32>(view.size());
        if (view.data() != pStr)
            Copy(pStr, view.data(), nLen);
        *(pStr+nLen) = 0;
    }

    return nLen;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE> sal_Int32 valueOfBoolean( IMPL_RTL_STRCODE* pStr, sal_Bool b )
{
    assert(pStr);
    if ( b )
    {
        *pStr = 't';
        pStr++;
        *pStr = 'r';
        pStr++;
        *pStr = 'u';
        pStr++;
        *pStr = 'e';
        pStr++;
        *pStr = 0;
        return 4;
    }
    else
    {
        *pStr = 'f';
        pStr++;
        *pStr = 'a';
        pStr++;
        *pStr = 'l';
        pStr++;
        *pStr = 's';
        pStr++;
        *pStr = 'e';
        pStr++;
        *pStr = 0;
        return 5;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 valueOfChar                             ( IMPL_RTL_STRCODE* pStr,
                                                    IMPL_RTL_STRCODE c )
{
    assert(pStr);
    *pStr++ = c;
    *pStr = 0;
    return 1;
}

/* ----------------------------------------------------------------------- */

template <sal_Int32 maxLen, typename IMPL_RTL_STRCODE, typename T>
sal_Int32 valueOfInt                               ( IMPL_RTL_STRCODE* pStr,
                                                     T n,
                                                     sal_Int16 nRadix )
{
    assert(pStr);
    assert( nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX );
    char    aBuf[maxLen];
    char*   pBuf = aBuf;
    sal_Int32   nLen = 0;
    using uT = std::make_unsigned_t<T>;
    uT nValue;

    /* Radix must be valid */
    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    if constexpr (std::is_signed_v<T>)
    {
        /* is value negative */
        if ( n < 0 )
        {
            *pStr = '-';
            pStr++;
            nLen++;
            nValue = n == std::numeric_limits<T>::min() ? static_cast<uT>(n) : -n;
        }
        else
            nValue = n;
    }
    else
        nValue = n;

    /* create a recursive buffer with all values, except the last one */
    do
    {
        char nDigit = static_cast<char>(nValue % nRadix);
        nValue /= nRadix;
        if ( nDigit > 9 )
            *pBuf = (nDigit-10) + 'a';
        else
            *pBuf = (nDigit + '0' );
        pBuf++;
    }
    while ( nValue > 0 );

    /* copy the values in the right direction into the destination buffer */
    do
    {
        pBuf--;
        *pStr = *pBuf;
        pStr++;
        nLen++;
    }
    while ( pBuf != aBuf );
    *pStr = 0;

    return nLen;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE> sal_Bool toBoolean( const IMPL_RTL_STRCODE* pStr )
{
    assert(pStr);
    if ( *pStr == '1' )
        return true;

    if ( (*pStr == 'T') || (*pStr == 't') )
    {
        pStr++;
        if ( (*pStr == 'R') || (*pStr == 'r') )
        {
            pStr++;
            if ( (*pStr == 'U') || (*pStr == 'u') )
            {
                pStr++;
                if ( (*pStr == 'E') || (*pStr == 'e') )
                    return true;
            }
        }
    }

    return false;
}

/* ----------------------------------------------------------------------- */

template <typename T, class Iter> inline bool HandleSignChar(Iter& iter)
{
    if constexpr (std::numeric_limits<T>::is_signed)
    {
        if (*iter == '-')
        {
            ++iter;
            return true;
        }
    }
    if (*iter == '+')
        ++iter;
    return false;
}

template <typename T> std::pair<T, sal_Int16> DivMod(sal_Int16 nRadix, [[maybe_unused]] bool bNeg)
{
    if constexpr (std::numeric_limits<T>::is_signed)
        if (bNeg)
            return { -(std::numeric_limits<T>::min() / nRadix),
                     -(std::numeric_limits<T>::min() % nRadix) };
    return { std::numeric_limits<T>::max() / nRadix, std::numeric_limits<T>::max() % nRadix };
}

template <typename T, class S> T toInt(S str, sal_Int16 nRadix)
{
    assert( nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX );

    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    auto pStr = str.begin();
    const auto end = str.end();

    /* Skip whitespaces */
    while (pStr != end && implIsWhitespace(IMPL_RTL_USTRCODE(*pStr)))
        pStr++;
    if (pStr == end)
        return 0;

    const bool bNeg = HandleSignChar<T>(pStr);
    const auto& [nDiv, nMod] = DivMod<T>(nRadix, bNeg);
    assert(nDiv > 0);

    std::make_unsigned_t<T> n = 0;
    while (pStr != end)
    {
        sal_Int16 nDigit = implGetDigit(IMPL_RTL_USTRCODE(*pStr), nRadix);
        if ( nDigit < 0 )
            break;
        if (static_cast<std::make_unsigned_t<T>>(nMod < nDigit ? nDiv - 1 : nDiv) < n)
            return 0;

        n *= nRadix;
        n += nDigit;

        pStr++;
    }

    if constexpr (std::numeric_limits<T>::is_signed)
        if (bNeg)
            return n == static_cast<std::make_unsigned_t<T>>(std::numeric_limits<T>::min())
                       ? std::numeric_limits<T>::min()
                       : -static_cast<T>(n);
    return static_cast<T>(n);
}

/* ======================================================================= */
/* Internal String-Class help functions                                    */
/* ======================================================================= */

template <class STRINGDATA> using STRCODE = std::remove_extent_t<decltype(STRINGDATA::buffer)>;
template <typename C> struct STRINGDATA_;
template <> struct STRINGDATA_<char>
{
    using T = rtl_String;
};
template <> struct STRINGDATA_<sal_Unicode>
{
    using T = rtl_uString;
};
template <typename C> using STRINGDATA = typename STRINGDATA_<C>::T;

template <typename IMPL_RTL_STRINGDATA> IMPL_RTL_STRINGDATA* Alloc( sal_Int32 nLen )
{
    IMPL_RTL_STRINGDATA * pData
        = (sal::static_int_cast< sal_uInt32 >(nLen)
           <= ((SAL_MAX_UINT32 - sizeof (IMPL_RTL_STRINGDATA))
               / sizeof (STRCODE<IMPL_RTL_STRINGDATA>)))
        ? static_cast<IMPL_RTL_STRINGDATA *>(rtl_allocateString(
            sizeof (IMPL_RTL_STRINGDATA) + nLen * sizeof (STRCODE<IMPL_RTL_STRINGDATA>)))
        : nullptr;
    if (pData != nullptr) {
        pData->refCount = 1;
        pData->length = nLen;
        pData->buffer[nLen] = 0;
    }
    return pData;
}

/* ======================================================================= */
/* String-Class functions                                                  */
/* ======================================================================= */

template <typename IMPL_RTL_STRINGDATA> void acquire(IMPL_RTL_STRINGDATA * pThis)
{
    if (!SAL_STRING_IS_STATIC (pThis))
        osl_atomic_increment( &((pThis)->refCount) );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA> void release( IMPL_RTL_STRINGDATA* pThis )
{
    if (SAL_UNLIKELY(SAL_STRING_IS_STATIC (pThis)))
        return;

    /* OString doesn't have an 'intern' */
    if constexpr (sizeof(STRCODE<IMPL_RTL_STRINGDATA>) == sizeof(sal_Unicode))
    {
        if (SAL_STRING_IS_INTERN (pThis))
        {
            internRelease (pThis);
            return;
        }
    }

    if ( !osl_atomic_decrement( &(pThis->refCount) ) )
    {
        RTL_LOG_STRING_DELETE( pThis );
        rtl_freeString( pThis );
    }
}

/* ----------------------------------------------------------------------- */

/* static data to be referenced by all empty strings
 * the refCount is predefined to 1 and must never become 0 !
 */
template <typename IMPL_RTL_STRINGDATA> struct EmptyStringImpl
{
    static IMPL_RTL_STRINGDATA data;
};

template <>
inline rtl_uString EmptyStringImpl<rtl_uString>::data = {
    sal_Int32(SAL_STRING_INTERN_FLAG | SAL_STRING_STATIC_FLAG | 1), /* sal_Int32   refCount;  */
    0,                                                              /* sal_Int32   length;    */
    { 0 }                                                           /* sal_Unicode buffer[1]; */
};

template <>
inline rtl_String EmptyStringImpl<rtl_String>::data = {
    SAL_STRING_STATIC_FLAG | 1, /* sal_Int32 refCount;  */
    0,                          /* sal_Int32 length;    */
    { 0 }                       /* char      buffer[1]; */
};

template <typename IMPL_RTL_STRINGDATA> void new_( IMPL_RTL_STRINGDATA** ppThis )
{
    assert(ppThis);
    if ( *ppThis)
        release( *ppThis );

    *ppThis = &EmptyStringImpl<IMPL_RTL_STRINGDATA>::data;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void new_WithLength( IMPL_RTL_STRINGDATA** ppThis, sal_Int32 nLen )
{
    assert(ppThis);
    assert(nLen >= 0);
    if ( nLen <= 0 )
        new_( ppThis );
    else
    {
        if ( *ppThis)
            release( *ppThis );

        *ppThis = Alloc<IMPL_RTL_STRINGDATA>( nLen );
        assert(*ppThis != nullptr);
        (*ppThis)->length   = 0;
        (*ppThis)->buffer[0] = 0;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA, typename C>
void newFromStr_WithLength(IMPL_RTL_STRINGDATA** ppThis, const C* pCharStr, sal_Int32 nLen,
                           sal_Int32 allocExtra = 0)
{
    assert(ppThis);
    assert(nLen >= 0);
    assert(pCharStr || nLen == 0);
    assert(allocExtra >= 0);

    if (nLen + allocExtra == 0)
        return new_(ppThis);

    IMPL_RTL_STRINGDATA* pOrg = *ppThis;
    *ppThis = Alloc<IMPL_RTL_STRINGDATA>(nLen + allocExtra);
    assert(*ppThis != nullptr);
    if (nLen > 0)
        Copy((*ppThis)->buffer, pCharStr, nLen);
    if (allocExtra > 0)
    {
        (*ppThis)->length = nLen;
        (*ppThis)->buffer[nLen] = 0;
    }

    RTL_LOG_STRING_NEW(*ppThis);

    /* must be done last, if pCharStr belongs to *ppThis */
    if (pOrg)
        release(pOrg);
}

template <typename IMPL_RTL_STRINGDATA>
void newFromString                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                    const IMPL_RTL_STRINGDATA* pStr )
{
    assert(pStr);

    newFromStr_WithLength(ppThis, pStr->buffer, pStr->length);
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void newFromStr                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                 const STRCODE<IMPL_RTL_STRINGDATA>* pCharStr )
{
#if OSL_DEBUG_LEVEL > 0
    //TODO: For now, only abort in non-production debug builds; once all places that rely on the
    // undocumented newFromStr behavior of treating a null pCharStr like an empty string have been
    // found and fixed, drop support for that behavior and turn this into a general assert:
    if (pCharStr == nullptr) {
        std::abort();
    }
#endif

    newFromStr_WithLength(ppThis, pCharStr, pCharStr ? getLength(pCharStr) : 0);
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA> void assign(IMPL_RTL_STRINGDATA**, IMPL_RTL_STRINGDATA*);

template <typename IMPL_RTL_STRINGDATA>
void newFromSubString                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                       const IMPL_RTL_STRINGDATA* pFrom,
                                                       sal_Int32 beginIndex,
                                                       sal_Int32 count )
{
    assert(ppThis);
    if ( beginIndex == 0 && count == pFrom->length )
        return assign(ppThis, const_cast<IMPL_RTL_STRINGDATA*>(pFrom));
    if ( count < 0 || beginIndex < 0 || beginIndex + count > pFrom->length )
    {
        assert(false); // fail fast at least in debug builds
        return newFromStr_WithLength(ppThis, "!!br0ken!!", 10);
    }

    newFromStr_WithLength( ppThis, pFrom->buffer + beginIndex, count );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void assign                                ( IMPL_RTL_STRINGDATA** ppThis,
                                             IMPL_RTL_STRINGDATA* pStr )
{
    assert(ppThis);
    /* must be done at first, if pStr == *ppThis */
    acquire( pStr );

    if ( *ppThis )
        release( *ppThis );

    *ppThis = pStr;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA> auto* getStr( IMPL_RTL_STRINGDATA* pThis )
{
    assert(pThis);
    return pThis->buffer;
}

/* ----------------------------------------------------------------------- */

enum ThrowPolicy { NoThrow, Throw };

template <ThrowPolicy throwPolicy, typename IMPL_RTL_STRINGDATA, typename C1, typename C2>
void newConcat(IMPL_RTL_STRINGDATA** ppThis, const C1* pLeft, sal_Int32 nLeftLength,
               const C2* pRight, sal_Int32 nRightLength)
{
    assert(ppThis);
    assert(nLeftLength >= 0);
    assert(pLeft || nLeftLength == 0);
    assert(nRightLength >= 0);
    assert(pRight || nRightLength == 0);
    IMPL_RTL_STRINGDATA* pOrg = *ppThis;

    if (nLeftLength > std::numeric_limits<sal_Int32>::max() - nRightLength)
    {
        if constexpr (throwPolicy == NoThrow)
            *ppThis = nullptr;
        else
        {
#if !defined(__COVERITY__)
            throw std::length_error("newConcat");
#else
            //coverity doesn't report std::bad_alloc as an unhandled exception when
            //potentially thrown from destructors but does report std::length_error
            throw std::bad_alloc();
#endif
        }
    }
    else
    {
        auto* pTempStr = Alloc<IMPL_RTL_STRINGDATA>(nLeftLength + nRightLength);
        OSL_ASSERT(pTempStr != nullptr);
        *ppThis = pTempStr;
        if (*ppThis != nullptr) {
            if (nLeftLength)
                Copy( pTempStr->buffer, pLeft, nLeftLength );
            if (nRightLength)
                Copy( pTempStr->buffer+nLeftLength, pRight, nRightLength );

            RTL_LOG_STRING_NEW( *ppThis );
        }
    }

    /* must be done last, if left or right == *ppThis */
    if ( pOrg )
        release( pOrg );
}

template<typename IMPL_RTL_STRINGDATA, typename IMPL_RTL_STRCODE>
void newConcat(IMPL_RTL_STRINGDATA** ppThis, IMPL_RTL_STRINGDATA* pLeft,
               const IMPL_RTL_STRCODE* pRight, sal_Int32 nRightLength)
{
    assert(pLeft != nullptr);
    if (nRightLength == 0)
        assign(ppThis, pLeft);
    else
        newConcat<Throw>(ppThis, pLeft->buffer, pLeft->length, pRight, nRightLength);
}

template <typename IMPL_RTL_STRINGDATA>
void newConcat                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                IMPL_RTL_STRINGDATA* pLeft,
                                                IMPL_RTL_STRINGDATA* pRight )
{
    /* Test for 0-Pointer - if not, change newReplaceStrAt! */
    if ( !pRight || !pRight->length )
    {
        assert(pLeft != nullptr);
        assign(ppThis, pLeft);
    }
    else if ( !pLeft || !pLeft->length )
        assign(ppThis, pRight);
    else
        newConcat<NoThrow>(ppThis, pLeft->buffer, pLeft->length, pRight->buffer, pRight->length);
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void ensureCapacity                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                     sal_Int32 size )
{
    assert(ppThis);
    IMPL_RTL_STRINGDATA* const pOrg = *ppThis;
    if ( pOrg->refCount == 1 && pOrg->length >= size )
        return;
    assert( pOrg->length <= size ); // do not truncate
    auto* pTempStr = Alloc<IMPL_RTL_STRINGDATA>( size );
    Copy( pTempStr->buffer, pOrg->buffer, pOrg->length );
    // right now the length is still the same as of the original
    pTempStr->length = pOrg->length;
    pTempStr->buffer[ pOrg->length ] = '\0';
    *ppThis = pTempStr;
    RTL_LOG_STRING_NEW( *ppThis );

    release( pOrg );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA, typename IMPL_RTL_STRCODE>
void newReplaceStrAt                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                      IMPL_RTL_STRINGDATA* pStr,
                                                      sal_Int32 nIndex,
                                                      sal_Int32 nCount,
                                                      const IMPL_RTL_STRCODE* pNewSubStr,
                                                      sal_Int32 nNewSubStrLen )
{
    assert(ppThis);
    assert(nIndex >= 0 && nIndex <= pStr->length);
    assert(nCount >= 0);
    assert(nCount <= pStr->length - nIndex);
    assert(pNewSubStr != nullptr || nNewSubStrLen == 0);
    assert(nNewSubStrLen >= 0);
    /* Append? */
    if ( nIndex >= pStr->length )
        return newConcat(ppThis, pStr, pNewSubStr, nNewSubStrLen);

    /* not more than the String length could be deleted */
    if ( nCount >= pStr->length-nIndex )
    {
        /* Assign of NewSubStr? */
        if (nIndex == 0)
            return newFromStr_WithLength( ppThis, pNewSubStr, nNewSubStrLen );

        nCount = pStr->length - nIndex;
    }

    /* Assign of Str? */
    if ( !nCount && !nNewSubStrLen )
        return assign(ppThis, pStr);

    IMPL_RTL_STRINGDATA*    pOrg = *ppThis;

    /* Alloc New Buffer */
    *ppThis = Alloc<IMPL_RTL_STRINGDATA>(pStr->length - nCount + nNewSubStrLen);
    assert(*ppThis != nullptr);
    auto* pBuffer = (*ppThis)->buffer;
    if ( nIndex )
    {
        Copy( pBuffer, pStr->buffer, nIndex );
        pBuffer += nIndex;
    }
    if ( nNewSubStrLen )
    {
        Copy( pBuffer, pNewSubStr, nNewSubStrLen );
        pBuffer += nNewSubStrLen;
    }
    Copy( pBuffer, pStr->buffer+nIndex+nCount, pStr->length-nIndex-nCount );

    RTL_LOG_STRING_NEW( *ppThis );
    /* must be done last, if pStr or pNewSubStr == *ppThis */
    if ( pOrg )
        release( pOrg );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void newReplaceStrAt                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                      IMPL_RTL_STRINGDATA* pStr,
                                                      sal_Int32 nIndex,
                                                      sal_Int32 nCount,
                                                      IMPL_RTL_STRINGDATA* pNewSubStr )
{
    assert(ppThis);
    assert(nIndex >= 0 && nIndex <= pStr->length);
    assert(nCount >= 0);
    assert(nCount <= pStr->length - nIndex);
    /* Append? */
    if (nIndex >= pStr->length)
    {
        /* newConcat test, if pNewSubStr is 0 */
        newConcat(ppThis, pStr, pNewSubStr);
        return;
    }

    /* not more than the String length could be deleted */
    if (nCount >= pStr->length-nIndex)
    {
        /* Assign of NewSubStr? */
        if (nIndex == 0)
        {
            if (!pNewSubStr)
                return new_(ppThis);
            else
                return assign(ppThis, pNewSubStr);
        }
        nCount = pStr->length - nIndex;
    }

    /* Assign of Str? */
    if (!nCount && (!pNewSubStr || !pNewSubStr->length))
    {
        assign(ppThis, pStr);
        return;
    }

    const auto* pNewSubStrBuf = pNewSubStr ? pNewSubStr->buffer : nullptr;
    const sal_Int32 nNewSubStrLength = pNewSubStr ? pNewSubStr->length : 0;
    newReplaceStrAt(ppThis, pStr, nIndex, nCount, pNewSubStrBuf, nNewSubStrLength);
}

/* ----------------------------------------------------------------------- */

template <class Traits, typename IMPL_RTL_STRINGDATA>
void newReplaceChars(IMPL_RTL_STRINGDATA** ppThis, IMPL_RTL_STRINGDATA* pStr)
{
    assert(ppThis);
    assert(pStr);

    const auto pEnd = pStr->buffer + pStr->length;
    auto pCharStr = std::find_if(pStr->buffer, pEnd, [](auto c) { return Traits::Applicable(c); });
    if (pCharStr != pEnd)
    {
        IMPL_RTL_STRINGDATA* pOrg = *ppThis;
        *ppThis = Alloc<IMPL_RTL_STRINGDATA>(pStr->length);
        assert(*ppThis != nullptr);
        auto* pNewCharStr = (*ppThis)->buffer;
        /* Copy String */
        const sal_Int32 nCount = pCharStr - pStr->buffer;
        Copy(pNewCharStr, pStr->buffer, nCount);
        pNewCharStr += nCount;
        /* replace/copy rest of the string */
        do
        {
            *pNewCharStr = Traits::Replace(*pCharStr);
            pNewCharStr++;
            pCharStr++;
        } while (pCharStr != pEnd);

        RTL_LOG_STRING_NEW(*ppThis);
        /* must be done last, if pStr == *ppThis */
        if (pOrg)
            release(pOrg);
    }
    else
        assign(ppThis, pStr);
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void newTrim                                ( IMPL_RTL_STRINGDATA** ppThis,
                                              IMPL_RTL_STRINGDATA* pStr )
{
    assert(pStr);
    const auto view = trimView(pStr->buffer, pStr->length);

    if (static_cast<sal_Int32>(view.size()) == pStr->length)
        assign(ppThis, pStr);
    else
        newFromStr_WithLength(ppThis, view.data(), view.size());
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
sal_Int32 getToken                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                    IMPL_RTL_STRINGDATA* pStr,
                                                    sal_Int32 nToken,
                                                    STRCODE<IMPL_RTL_STRINGDATA> cTok,
                                                    sal_Int32 nIndex )
{
    assert(ppThis);
    assert(pStr);
    assert(nIndex <= pStr->length);

    // Set ppThis to an empty string and return -1 if either nToken or nIndex is
    // negative:
    if (nIndex >= 0 && nToken >= 0)
    {
        const auto* pOrgCharStr = pStr->buffer;
        const auto* pCharStr = pOrgCharStr + nIndex;
        sal_Int32 nLen = pStr->length - nIndex;
        sal_Int32 nTokCount = 0;
        const auto* pCharStrStart = pCharStr;
        while (nLen > 0)
        {
            if (*pCharStr == cTok)
            {
                nTokCount++;

                if (nTokCount > nToken)
                    break;
                if (nTokCount == nToken)
                    pCharStrStart = pCharStr + 1;
            }

            pCharStr++;
            nLen--;
        }
        if (nTokCount >= nToken)
        {
            newFromStr_WithLength(ppThis, pCharStrStart, pCharStr - pCharStrStart);
            if (nLen > 0)
                return pCharStr - pOrgCharStr + 1;
            else
                return -1;
        }
    }

    new_(ppThis);
    return -1;
}

/* ======================================================================= */
/* String buffer help functions                                            */
/* ======================================================================= */

template <class IMPL_RTL_STRINGDATA>
void stringbuffer_newFromStr_WithLength(IMPL_RTL_STRINGDATA** ppThis,
                                        const STRCODE<IMPL_RTL_STRINGDATA>* pStr, sal_Int32 count)
{
    assert(ppThis);
    assert(count >= 0);
    if (!pStr)
        count = 0; // Because old code didn't care about count when !pStr

    newFromStr_WithLength(ppThis, pStr, count, 16);
}

template <class IMPL_RTL_STRINGDATA>
sal_Int32 stringbuffer_newFromStringBuffer(IMPL_RTL_STRINGDATA** ppThis, sal_Int32 capacity,
                                           IMPL_RTL_STRINGDATA* pStr)
{
    assert(capacity >= 0);
    assert(pStr);

    if (capacity < pStr->length)
        capacity = pStr->length;

    newFromStr_WithLength(ppThis, pStr->buffer, pStr->length, capacity - pStr->length);
    return capacity;
}

template <class IMPL_RTL_STRINGDATA>
void stringbuffer_ensureCapacity(IMPL_RTL_STRINGDATA** ppThis, sal_Int32* capacity,
                                 sal_Int32 minimumCapacity)
{
    assert(ppThis);
    assert(capacity && *capacity >= 0);
    // assert(minimumCapacity >= 0); // It was commented out in rtl_stringbuffer_ensureCapacity
    if (minimumCapacity <= *capacity)
        return;

    const auto nLength = (*ppThis)->length;
    *capacity = (nLength + 1) * 2;
    if (minimumCapacity > *capacity)
        *capacity = minimumCapacity;

    newFromStr_WithLength(ppThis, (*ppThis)->buffer, nLength, *capacity - nLength);
}

template <class IMPL_RTL_STRINGDATA, typename C>
void stringbuffer_insert(IMPL_RTL_STRINGDATA** ppThis, sal_Int32* capacity, sal_Int32 offset,
                         const C* pStr, sal_Int32 len)
{
    assert(ppThis);
    assert(capacity && *capacity >= 0);
    assert(offset >= 0 && offset <= (*ppThis)->length);
    assert(len >= 0);
    if (len == 0)
        return;
    if (len > std::numeric_limits<sal_Int32>::max() - (*ppThis)->length) {
        throw std::bad_alloc();
    }

    stringbuffer_ensureCapacity(ppThis, capacity, (*ppThis)->length + len);

    sal_Int32 nOldLen = (*ppThis)->length;
    auto* pBuf = (*ppThis)->buffer;

    /* copy the tail */
    const sal_Int32 n = nOldLen - offset;
    if (n > 0)
        CopyBackward(pBuf + offset + len, pBuf + offset, n);

    /* insert the new characters */
    if (pStr != nullptr)
        Copy(pBuf + offset, pStr, len);

    (*ppThis)->length = nOldLen + len;
    pBuf[nOldLen + len] = 0;
}

template <class IMPL_RTL_STRINGDATA>
void stringbuffer_remove(IMPL_RTL_STRINGDATA** ppThis, sal_Int32 start, sal_Int32 len)
{
    assert(ppThis);
    assert(start >= 0 && start <= (*ppThis)->length);
    assert(len >= 0);

    if (len > (*ppThis)->length - start)
        len = (*ppThis)->length - start;

    //remove nothing
    if (!len)
        return;

    auto* pBuf = (*ppThis)->buffer;
    const sal_Int32 nTailLen = (*ppThis)->length - (start + len);

    if (nTailLen)
    {
        /* move the tail */
        Copy(pBuf + start, pBuf + start + len, nTailLen);
    }

    (*ppThis)->length -= len;
    pBuf[(*ppThis)->length] = 0;
}

template <class S, typename CharTypeFrom, typename CharTypeTo>
void newReplaceAllFromIndex(S** s, S* s1, CharTypeFrom const* from, sal_Int32 fromLength,
                            CharTypeTo const* to, sal_Int32 toLength, sal_Int32 fromIndex)
{
    assert(s != nullptr);
    assert(s1 != nullptr);
    assert(fromLength >= 0);
    assert(from != nullptr || fromLength == 0);
    assert(toLength >= 0);
    assert(to != nullptr || toLength == 0);
    assert(fromIndex >= 0 && fromIndex <= s1->length);
    sal_Int32 i = indexOfStr_WithLength(s1->buffer + fromIndex, s1->length - fromIndex,
                                        from, fromLength);
    if (i >= 0)
    {
        if (s1->length - fromLength > SAL_MAX_INT32 - toLength)
            std::abort();
        i += fromIndex;
        sal_Int32 nCapacity = s1->length + (toLength - fromLength);
        if (fromLength < toLength)
        {
            // Pre-allocate up to 16 replacements more
            const sal_Int32 nMaxMoreFinds = (s1->length - i - fromLength) / fromLength;
            const sal_Int32 nIncrease = toLength - fromLength;
            const sal_Int32 nMoreReplacements = std::min(
                { nMaxMoreFinds, (SAL_MAX_INT32 - nCapacity) / nIncrease, sal_Int32(16) });
            nCapacity += nMoreReplacements * nIncrease;
        }
        const auto pOld = *s;
        *s = Alloc<S>(nCapacity);
        (*s)->length = 0;
        fromIndex = 0;
        do
        {
            stringbuffer_insert(s, &nCapacity, (*s)->length, s1->buffer + fromIndex, i);
            stringbuffer_insert(s, &nCapacity, (*s)->length, to, toLength);
            fromIndex += i + fromLength;
            i = indexOfStr_WithLength(s1->buffer + fromIndex, s1->length - fromIndex,
                                      from, fromLength);
        } while (i >= 0);
        // the rest
        stringbuffer_insert(s, &nCapacity, (*s)->length,
                            s1->buffer + fromIndex, s1->length - fromIndex);
        if (pOld)
            release(pOld); // Must be last in case *s == s1
    }
    else
        assign(s, s1);

    RTL_LOG_STRING_NEW(*s);
}

template <typename IMPL_RTL_STRINGDATA>
void newReplace(IMPL_RTL_STRINGDATA** ppThis, IMPL_RTL_STRINGDATA* pStr,
                STRCODE<IMPL_RTL_STRINGDATA> cOld, STRCODE<IMPL_RTL_STRINGDATA> cNew)
{
    return newReplaceAllFromIndex(ppThis, pStr, &cOld, 1, &cNew, 1, 0);
}

template <class S, typename CharTypeFrom, typename CharTypeTo>
void newReplaceFirst(S** s, S* s1, CharTypeFrom const* from, sal_Int32 fromLength,
                     CharTypeTo const* to, sal_Int32 toLength, sal_Int32& fromIndex)
{
    assert(s != nullptr);
    assert(s1 != nullptr);
    assert(fromLength >= 0);
    assert(from != nullptr || fromLength == 0);
    assert(toLength >= 0);
    assert(to != nullptr || toLength == 0);
    assert(fromIndex >= 0 && fromIndex <= s1->length);
    sal_Int32 i = indexOfStr_WithLength(s1->buffer + fromIndex, s1->length - fromIndex,
                                        from, fromLength);
    if (i >= 0)
    {
        if (s1->length - fromLength > SAL_MAX_INT32 - toLength)
            std::abort();
        i += fromIndex;
        newReplaceStrAt(s, s1, i, fromLength, to, toLength);
    }
    else
        assign(s, s1);

    fromIndex = i;
}

// doubleToString implementation

static inline constexpr sal_uInt64 eX[] = { 10ull,
                                            100ull,
                                            1000ull,
                                            10000ull,
                                            100000ull,
                                            1000000ull,
                                            10000000ull,
                                            100000000ull,
                                            1000000000ull,
                                            10000000000ull,
                                            100000000000ull,
                                            1000000000000ull,
                                            10000000000000ull,
                                            100000000000000ull,
                                            1000000000000000ull,
                                            10000000000000000ull,
                                            100000000000000000ull,
                                            1000000000000000000ull,
                                            10000000000000000000ull };

template <typename S>
void doubleToString(S** pResult, sal_Int32* pResultCapacity, sal_Int32 nResultOffset, double fValue,
                    rtl_math_StringFormat eFormat, sal_Int32 nDecPlaces, STRCODE<S> cDecSeparator,
                    sal_Int32 const* pGroups, STRCODE<S> cGroupSeparator,
                    bool bEraseTrailingDecZeros)
{
    auto decimalDigits = [](sal_uInt64 n) {
        return std::distance(std::begin(eX), std::upper_bound(std::begin(eX), std::end(eX), n)) + 1;
    };

    auto roundToPow10 = [](sal_uInt64 n, int e) {
        assert(e > 0 && o3tl::make_unsigned(e) <= std::size(eX));
        const sal_uInt64 d = eX[e - 1];
        return (n + d / 2) / d * d;
    };

    auto append = [](S** s, sal_Int32* pCapacity, sal_Int32& rOffset, auto sv)
    {
        stringbuffer_insert(s, pCapacity, rOffset, sv.data(), sv.size());
        rOffset += sv.size();
    };

    if (std::isnan(fValue))
    {
        // #i112652# XMLSchema-2
        constexpr std::string_view nan{ "NaN" };
        if (!pResultCapacity)
            return newFromStr_WithLength(pResult, nan.data(), nan.size());
        else
            return append(pResult, pResultCapacity, nResultOffset, nan);
    }

    // sign adjustment, instead of testing for fValue<0.0 this will also fetch -0.0
    bool bSign = std::signbit(fValue);

    if (std::isinf(fValue))
    {
        // #i112652# XMLSchema-2
        std::string_view inf = bSign ? std::string_view("-INF") : std::string_view("INF");
        if (!pResultCapacity)
            return newFromStr_WithLength(pResult, inf.data(), inf.size());
        else
            return append(pResult, pResultCapacity, nResultOffset, inf);
    }

    if (bSign)
        fValue = -fValue;

    decltype(jkj::dragonbox::to_decimal(fValue, jkj::dragonbox::policy::sign::ignore,
                                        jkj::dragonbox::policy::trailing_zero::ignore)) aParts{};
    if (fValue) // to_decimal is documented to only handle non-zero finite numbers
        aParts = jkj::dragonbox::to_decimal(fValue, jkj::dragonbox::policy::sign::ignore,
                                            jkj::dragonbox::policy::trailing_zero::ignore);

    int nOrigDigits = decimalDigits(aParts.significand);
    int nExp = nOrigDigits + aParts.exponent - 1;
    int nRoundDigits = 15;

    // Unfortunately the old rounding below writes 1.79769313486232e+308 for
    // DBL_MAX and 4 subsequent nextafter(...,0).
    static const double fB1 = std::nextafter(std::numeric_limits<double>::max(), 0);
    static const double fB2 = std::nextafter(fB1, 0);
    static const double fB3 = std::nextafter(fB2, 0);
    static const double fB4 = std::nextafter(fB3, 0);
    if ((fValue >= fB4) && eFormat != rtl_math_StringFormat_F)
    {
        // 1.7976931348623157e+308 instead of rounded 1.79769313486232e+308
        // that can't be converted back as out of range. For rounded values if
        // they exceed range they should not be written to exchange strings or
        // file formats.

        eFormat = rtl_math_StringFormat_E;
        nDecPlaces = std::clamp<sal_Int32>(nDecPlaces, 0, 16);
        nRoundDigits = 17;
    }

    // Use integer representation for integer values that fit into the
    // mantissa (1.((2^53)-1)) with a precision of 1 for highest accuracy.
    if ((eFormat == rtl_math_StringFormat_Automatic || eFormat == rtl_math_StringFormat_F)
        && aParts.exponent >= 0 && fValue < 0x1p53)
    {
        eFormat = rtl_math_StringFormat_F;
        if (nDecPlaces == rtl_math_DecimalPlaces_Max)
            nDecPlaces = 0;
        else
            nDecPlaces = std::clamp<sal_Int32>(nDecPlaces, -15, 15);

        if (bEraseTrailingDecZeros && nDecPlaces > 0)
            nDecPlaces = 0;

        nRoundDigits = nOrigDigits; // no rounding
    }

    switch (eFormat)
    {
        case rtl_math_StringFormat_Automatic:
            // E or F depending on exponent magnitude
            if (nExp <= -15 || nExp >= 15)
            {
                if (nDecPlaces == rtl_math_DecimalPlaces_Max)
                    nDecPlaces = 14;
                eFormat = rtl_math_StringFormat_E;
            }
            else
            {
                if (nDecPlaces == rtl_math_DecimalPlaces_Max)
                    nDecPlaces = (nExp < 14) ? 15 - nExp - 1 : 15;
                eFormat = rtl_math_StringFormat_F;
            }
            break;

        case rtl_math_StringFormat_G:
        case rtl_math_StringFormat_G1:
        case rtl_math_StringFormat_G2:
            // G-Point, similar to sprintf %G
            if (nDecPlaces == rtl_math_DecimalPlaces_DefaultSignificance)
                nDecPlaces = 6;

            if (nExp < -4 || nExp >= nDecPlaces)
            {
                nDecPlaces = std::max<sal_Int32>(1, nDecPlaces - 1);

                if (eFormat == rtl_math_StringFormat_G)
                    eFormat = rtl_math_StringFormat_E;
                else if (eFormat == rtl_math_StringFormat_G2)
                    eFormat = rtl_math_StringFormat_E2;
                else if (eFormat == rtl_math_StringFormat_G1)
                    eFormat = rtl_math_StringFormat_E1;
            }
            else
            {
                nDecPlaces = std::max<sal_Int32>(0, nDecPlaces - nExp - 1);
                eFormat = rtl_math_StringFormat_F;
            }
            break;

        default:
            break;
    }

    // Too large values for nDecPlaces make no sense; it might also be
    // rtl_math_DecimalPlaces_Max was passed with rtl_math_StringFormat_F or
    // others, but we don't want to allocate/deallocate 2GB just to fill it
    // with trailing '0' characters..
    nDecPlaces = std::clamp<sal_Int32>(nDecPlaces, -20, 20);

    sal_Int32 nDigits = nDecPlaces + 1;

    if (eFormat == rtl_math_StringFormat_F)
        nDigits += nExp;

    // Round the number
    nRoundDigits = std::min<int>(nDigits, nRoundDigits);
    if (nDigits >= 0 && nOrigDigits > nRoundDigits)
    {
        aParts.significand = roundToPow10(aParts.significand, nOrigDigits - nRoundDigits);
        assert(aParts.significand <= eX[nOrigDigits - 1]);
        if (aParts.significand == eX[nOrigDigits - 1]) // up-rounding to the next decade
        {
            nOrigDigits++;
            nExp++;

            if (eFormat == rtl_math_StringFormat_F)
                nDigits++;
        }
    }

    sal_Int32 nBuf
        = (nDigits <= 0 ? std::max<sal_Int32>(nDecPlaces, std::abs(nExp)) : nDigits + nDecPlaces)
          + 10 + (pGroups ? std::abs(nDigits) * 2 : 0);
    // max(nDigits) = max(nDecPlaces) + 1 + max(nExp) + 1 = 20 + 1 + 308 + 1 = 330
    // max(nBuf) = max(nDigits) + max(nDecPlaces) + 10 + max(nDigits) * 2 = 330 * 3 + 20 + 10 = 1020
    assert(nBuf <= 1024);
    STRCODE<S>* const pBuf = static_cast<STRCODE<S>*>(alloca(nBuf * sizeof(STRCODE<S>)));
    STRCODE<S>* p = pBuf;
    if (bSign)
        *p++ = '-';

    bool bHasDec = false;

    int nDecPos;
    // Check for F format and number < 1
    if (eFormat == rtl_math_StringFormat_F)
    {
        if (nExp < 0)
        {
            *p++ = '0';
            if (nDecPlaces > 0)
            {
                *p++ = cDecSeparator;
                bHasDec = true;
            }

            sal_Int32 i = (nDigits <= 0 ? nDecPlaces : -nExp - 1);

            while ((i--) > 0)
                *p++ = '0';

            nDecPos = 0;
        }
        else
            nDecPos = nExp + 1;
    }
    else
        nDecPos = 1;

    int nGrouping = 0, nGroupSelector = 0, nGroupExceed = 0;
    if (nDecPos > 1 && pGroups && pGroups[0] && cGroupSeparator)
    {
        while (nGrouping + pGroups[nGroupSelector] < nDecPos)
        {
            nGrouping += pGroups[nGroupSelector];
            if (pGroups[nGroupSelector + 1])
            {
                if (nGrouping + pGroups[nGroupSelector + 1] >= nDecPos)
                    break; // while

                ++nGroupSelector;
            }
            else if (!nGroupExceed)
                nGroupExceed = nGrouping;
        }
    }

    // print the number
    if (nDigits > 0)
    {
        for (int nCurExp = nOrigDigits - 1;;)
        {
            int nDigit;
            if (aParts.significand > 0 && nCurExp > 0)
            {
                --nCurExp;
                nDigit = aParts.significand / eX[nCurExp];
                aParts.significand %= eX[nCurExp];
            }
            else
            {
                nDigit = aParts.significand;
                aParts.significand = 0;
            }
            assert(nDigit >= 0 && nDigit < 10);
            *p++ = nDigit + '0';

            if (!--nDigits)
                break; // for

            if (nDecPos)
            {
                if (!--nDecPos)
                {
                    *p++ = cDecSeparator;
                    bHasDec = true;
                }
                else if (nDecPos == nGrouping)
                {
                    *p++ = cGroupSeparator;
                    nGrouping -= pGroups[nGroupSelector];

                    if (nGroupSelector && nGrouping < nGroupExceed)
                        --nGroupSelector;
                }
            }
        }
    }

    if (!bHasDec && eFormat == rtl_math_StringFormat_F)
    { // nDecPlaces < 0 did round the value
        while (--nDecPos > 0)
        { // fill before decimal point
            if (nDecPos == nGrouping)
            {
                *p++ = cGroupSeparator;
                nGrouping -= pGroups[nGroupSelector];

                if (nGroupSelector && nGrouping < nGroupExceed)
                    --nGroupSelector;
            }

            *p++ = '0';
        }
    }

    if (bEraseTrailingDecZeros && bHasDec)
    {
        while (*(p - 1) == '0')
            p--;

        if (*(p - 1) == cDecSeparator)
            p--;
    }

    // Print the exponent ('E', followed by '+' or '-', followed by exactly
    // three digits for rtl_math_StringFormat_E). The code in
    // rtl_[u]str_valueOf{Float|Double} relies on this format.
    if (eFormat == rtl_math_StringFormat_E || eFormat == rtl_math_StringFormat_E2
        || eFormat == rtl_math_StringFormat_E1)
    {
        if (p == pBuf)
            *p++ = '1';
        // maybe no nDigits if nDecPlaces < 0

        *p++ = 'E';
        if (nExp < 0)
        {
            nExp = -nExp;
            *p++ = '-';
        }
        else
            *p++ = '+';

        if (eFormat == rtl_math_StringFormat_E || nExp >= 100)
            *p++ = nExp / 100 + '0';

        nExp %= 100;

        if (eFormat == rtl_math_StringFormat_E || eFormat == rtl_math_StringFormat_E2 || nExp >= 10)
            *p++ = nExp / 10 + '0';

        *p++ = nExp % 10 + '0';
    }

    if (!pResultCapacity)
        newFromStr_WithLength(pResult, pBuf, p - pBuf);
    else
        append(pResult, pResultCapacity, nResultOffset, std::basic_string_view(pBuf, p - pBuf));
}

template <sal_Int32 maxLen, typename C, typename T> sal_Int32 SAL_CALL valueOfFP(C* pStr, T f)
{
    assert(pStr);
    STRINGDATA<C>* pResult = nullptr;
    doubleToString(&pResult, nullptr, 0, f, rtl_math_StringFormat_G,
                   maxLen - std::size("-x.E-xxx") + 1, '.', nullptr, 0, true);
    const sal_Int32 nLen = pResult->length;
    assert(nLen < maxLen);
    Copy(pStr, pResult->buffer, nLen + 1);
    release(pResult);
    return nLen;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
