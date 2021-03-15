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

/* ======================================================================= */
/* Internal C-String help functions which could be used without the        */
/* String-Class                                                            */
/* ======================================================================= */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <string_view>
#include <type_traits>

#include <cstring>
#include <wchar.h>
#include <sal/log.hxx>
#include <rtl/character.hxx>

namespace rtl::str
{
template <typename C> auto IMPL_RTL_USTRCODE(C c) { return std::make_unsigned_t<C>(c); }

template <typename IMPL_RTL_STRCODE>
void Copy( IMPL_RTL_STRCODE* _pDest,
                                     const IMPL_RTL_STRCODE* _pSrc,
                                     sal_Int32 _nCount )
{
    // take advantage of builtin optimisations
    memcpy( _pDest, _pSrc, _nCount * sizeof(IMPL_RTL_STRCODE));
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
    else if constexpr (sizeof(T) == sizeof(char))
    {
        // take advantage of builtin optimisations
        return strlen( pStr);
    }
    else if constexpr (sizeof(T) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        return wcslen(reinterpret_cast<wchar_t const *>(pStr));
    }
    else
    {
        const T* pTempStr = pStr;
        while( *pTempStr )
            pTempStr++;
        return pTempStr-pStr;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 compare                             ( const IMPL_RTL_STRCODE* pStr1,
                                                const IMPL_RTL_STRCODE* pStr2 )
{
    assert(pStr1);
    assert(pStr2);
    if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(char))
    {
        // take advantage of builtin optimisations
        return strcmp( pStr1, pStr2);
    }
    else if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        return wcscmp(reinterpret_cast<wchar_t const *>(pStr1), reinterpret_cast<wchar_t const *>(pStr2));
    }
    else
    {
        sal_Int32 nRet;
        for (;;)
        {
            nRet = static_cast<sal_Int32>(IMPL_RTL_USTRCODE(*pStr1)) -
                   static_cast<sal_Int32>(IMPL_RTL_USTRCODE(*pStr2));
            if (!(nRet == 0 && *pStr2 ))
                break;
            pStr1++;
            pStr2++;
        }

        return nRet;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 compare_WithLength                             ( const IMPL_RTL_STRCODE* pStr1,
                                                           sal_Int32 nStr1Len,
                                                           const IMPL_RTL_STRCODE* pStr2,
                                                           sal_Int32 nStr2Len )
{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
    if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(char))
    {
        // take advantage of builtin optimisations
        sal_Int32 nMin = std::min(nStr1Len, nStr2Len);
        sal_Int32 nRet = memcmp(pStr1, pStr2, nMin);
        return nRet == 0 ? nStr1Len - nStr2Len : nRet;
    }
    else if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        sal_Int32 nMin = std::min(nStr1Len, nStr2Len);
        sal_Int32 nRet = wmemcmp(reinterpret_cast<wchar_t const *>(pStr1),
                reinterpret_cast<wchar_t const *>(pStr2), nMin);
        return nRet == 0 ? nStr1Len - nStr2Len : nRet;
    }
    else
    {
        sal_Int32 nRet = nStr1Len - nStr2Len;
        int nCount = (nRet <= 0) ? nStr1Len : nStr2Len;

        while( --nCount >= 0 ) {
            if (*pStr1 != *pStr2) {
                break;
            }
            ++pStr1;
            ++pStr2;
        }

        if( nCount >= 0 )
            nRet = static_cast<sal_Int32>(IMPL_RTL_USTRCODE( *pStr1 ))
                 - static_cast<sal_Int32>(IMPL_RTL_USTRCODE( *pStr2 ));

        return nRet;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 shortenedCompare_WithLength                             ( const IMPL_RTL_STRCODE* pStr1,
                                                                    sal_Int32 nStr1Len,
                                                                    const IMPL_RTL_STRCODE* pStr2,
                                                                    sal_Int32 nStr2Len,
                                                                    sal_Int32 nShortenedLength )
{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
    assert(nShortenedLength >= 0);
    if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(char))
    {
        // take advantage of builtin optimisations
        sal_Int32 nMin = std::min(std::min(nStr1Len, nStr2Len), nShortenedLength);
        sal_Int32 nRet = memcmp(pStr1, pStr2, nMin);
        if (nRet == 0 && nShortenedLength > std::min(nStr1Len, nStr2Len))
            return nStr1Len - nStr2Len;
        return nRet;
    }
    else if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        sal_Int32 nMin = std::min(std::min(nStr1Len, nStr2Len), nShortenedLength);
        sal_Int32 nRet = wmemcmp(reinterpret_cast<wchar_t const *>(pStr1), reinterpret_cast<wchar_t const *>(pStr2), nMin);
        if (nRet == 0 && nShortenedLength > std::min(nStr1Len, nStr2Len))
            return nStr1Len - nStr2Len;
        return nRet;
    }
    else
    {
        const IMPL_RTL_STRCODE* pStr1End = pStr1 + nStr1Len;
        const IMPL_RTL_STRCODE* pStr2End = pStr2 + nStr2Len;
        sal_Int32               nRet;
        while ( (nShortenedLength > 0) &&
                (pStr1 < pStr1End) && (pStr2 < pStr2End) )
        {
            nRet = static_cast<sal_Int32>(IMPL_RTL_USTRCODE( *pStr1 ))-
                   static_cast<sal_Int32>(IMPL_RTL_USTRCODE( *pStr2 ));
            if ( nRet )
                return nRet;

            nShortenedLength--;
            pStr1++;
            pStr2++;
        }

        if ( nShortenedLength <= 0 )
            return 0;
        return nStr1Len - nStr2Len;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 reverseCompare_WithLength                             ( const IMPL_RTL_STRCODE* pStr1,
                                                                  sal_Int32 nStr1Len,
                                                                  const IMPL_RTL_STRCODE* pStr2,
                                                                  sal_Int32 nStr2Len )
{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
    const IMPL_RTL_STRCODE* pStr1Run = pStr1+nStr1Len;
    const IMPL_RTL_STRCODE* pStr2Run = pStr2+nStr2Len;
    sal_Int32               nRet;
    while ( (pStr1 < pStr1Run) && (pStr2 < pStr2Run) )
    {
        pStr1Run--;
        pStr2Run--;
        nRet = static_cast<sal_Int32>(IMPL_RTL_USTRCODE( *pStr1Run ))-
               static_cast<sal_Int32>(IMPL_RTL_USTRCODE( *pStr2Run ));
        if ( nRet )
            return nRet;
    }

    return nStr1Len - nStr2Len;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 compareIgnoreAsciiCase                             ( const IMPL_RTL_STRCODE* pStr1,
                                                               const IMPL_RTL_STRCODE* pStr2 )
{
    assert(pStr1);
    assert(pStr2);
    sal_uInt32 c1;
    do
    {
        c1 = IMPL_RTL_USTRCODE(*pStr1);
        sal_Int32 nRet = rtl::compareIgnoreAsciiCase(
            c1, IMPL_RTL_USTRCODE(*pStr2));
        if ( nRet != 0 )
            return nRet;

        pStr1++;
        pStr2++;
    }
    while (c1);

    return 0;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 compareIgnoreAsciiCase_WithLength                             ( const IMPL_RTL_STRCODE* pStr1,
                                                                          sal_Int32 nStr1Len,
                                                                          const IMPL_RTL_STRCODE* pStr2,
                                                                          sal_Int32 nStr2Len )
{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
    const IMPL_RTL_STRCODE* pStr1End = pStr1 + nStr1Len;
    const IMPL_RTL_STRCODE* pStr2End = pStr2 + nStr2Len;
    while ( (pStr1 < pStr1End) && (pStr2 < pStr2End) )
    {
        sal_Int32 nRet = rtl::compareIgnoreAsciiCase(
            IMPL_RTL_USTRCODE(*pStr1), IMPL_RTL_USTRCODE(*pStr2));
        if ( nRet != 0 )
            return nRet;

        pStr1++;
        pStr2++;
    }

    return nStr1Len - nStr2Len;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 shortenedCompareIgnoreAsciiCase_WithLength                             ( const IMPL_RTL_STRCODE* pStr1,
                                                                                   sal_Int32 nStr1Len,
                                                                                   const IMPL_RTL_STRCODE* pStr2,
                                                                                   sal_Int32 nStr2Len,
                                                                                   sal_Int32 nShortenedLength )
{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
    assert(nShortenedLength >= 0);
    const IMPL_RTL_STRCODE* pStr1End = pStr1 + nStr1Len;
    const IMPL_RTL_STRCODE* pStr2End = pStr2 + nStr2Len;
    while ( (nShortenedLength > 0) &&
            (pStr1 < pStr1End) && (pStr2 < pStr2End) )
    {
        sal_Int32 nRet = rtl::compareIgnoreAsciiCase(
            IMPL_RTL_USTRCODE(*pStr1), IMPL_RTL_USTRCODE(*pStr2));
        if ( nRet != 0 )
            return nRet;

        nShortenedLength--;
        pStr1++;
        pStr2++;
    }

    if ( nShortenedLength <= 0 )
        return 0;
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
    if constexpr (sizeof(IMPL_RTL_STRCODE) == sizeof(char))
    {
        // take advantage of builtin optimisations
        IMPL_RTL_STRCODE* p = static_cast<IMPL_RTL_STRCODE*>(std::memchr(const_cast<IMPL_RTL_STRCODE *>(pStr), c, nLen));
        return p ? p - pStr : -1;
    }
    else
    {
        const IMPL_RTL_STRCODE* pTempStr = pStr;
        while ( nLen > 0 )
        {
            if ( *pTempStr == c )
                return pTempStr-pStr;

            pTempStr++;
            nLen--;
        }

        return -1;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
sal_Int32 lastIndexOfChar_WithLength(const IMPL_RTL_STRCODE*, sal_Int32, IMPL_RTL_STRCODE);

template <typename IMPL_RTL_STRCODE>
sal_Int32 lastIndexOfChar                             ( const IMPL_RTL_STRCODE* pStr,
                                                        IMPL_RTL_STRCODE c )
{
    assert(pStr);
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
    pStr += nLen;
    while ( nLen > 0 )
    {
        nLen--;
        pStr--;

        if ( *pStr == c )
            return nLen;
    }

    return -1;
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
    /* faster search for a single character */
    if ( nSubLen < 2 )
    {
        /* an empty SubString is always not findable */
        if ( nSubLen == 1 )
        {
            IMPL_RTL_STRCODE        c = *pSubStr;
            const IMPL_RTL_STRCODE* pTempStr = pStr;
            while ( nStrLen > 0 )
            {
                if ( *pTempStr == c )
                    return pTempStr-pStr;

                pTempStr++;
                nStrLen--;
            }
        }
    }
    else
    {
        const IMPL_RTL_STRCODE* pTempStr = pStr;
        while ( nStrLen > 0 )
        {
            if ( *pTempStr == *pSubStr )
            {
                /* Compare SubString */
                if ( nSubLen <= nStrLen )
                {
                    const IMPL_RTL_STRCODE* pTempStr1 = pTempStr;
                    const IMPL_RTL_STRCODE* pTempStr2 = pSubStr;
                    sal_Int32               nTempLen = nSubLen;
                    while ( nTempLen )
                    {
                        if ( *pTempStr1 != *pTempStr2 )
                            break;

                        pTempStr1++;
                        pTempStr2++;
                        nTempLen--;
                    }

                    if ( !nTempLen )
                        return pTempStr-pStr;
                }
                else
                    break;
            }

            nStrLen--;
            pTempStr++;
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
    /* faster search for a single character */
    if ( nSubLen < 2 )
    {
        /* an empty SubString is always not findable */
        if ( nSubLen == 1 )
        {
            IMPL_RTL_STRCODE c = *pSubStr;
            pStr += nStrLen;
            while ( nStrLen > 0 )
            {
                nStrLen--;
                pStr--;

                if ( *pStr == c )
                    return nStrLen;
            }
        }
    }
    else
    {
        pStr += nStrLen;
        nStrLen -= nSubLen;
        pStr -= nSubLen;
        while ( nStrLen >= 0 )
        {
            const IMPL_RTL_STRCODE* pTempStr1 = pStr;
            const IMPL_RTL_STRCODE* pTempStr2 = pSubStr;
            sal_Int32               nTempLen = nSubLen;
            while ( nTempLen )
            {
                if ( *pTempStr1 != *pTempStr2 )
                    break;

                pTempStr1++;
                pTempStr2++;
                nTempLen--;
            }

            if ( !nTempLen )
                return nStrLen;

            nStrLen--;
            pStr--;
        }
    }

    return -1;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
void replaceChar                             ( IMPL_RTL_STRCODE* pStr,
                                               IMPL_RTL_STRCODE cOld,
                                               IMPL_RTL_STRCODE cNew )
{
    assert(pStr);
    while ( *pStr )
    {
        if ( *pStr == cOld )
            *pStr = cNew;

        pStr++;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
void replaceChar_WithLength                             ( IMPL_RTL_STRCODE* pStr,
                                                          sal_Int32 nLen,
                                                          IMPL_RTL_STRCODE cOld,
                                                          IMPL_RTL_STRCODE cNew )
{
    assert(nLen >= 0);
    while ( nLen > 0 )
    {
        if ( *pStr == cOld )
            *pStr = cNew;

        pStr++;
        nLen--;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE> void toAsciiLowerCase( IMPL_RTL_STRCODE* pStr )
{
    assert(pStr);
    while ( *pStr )
    {
        *pStr = rtl::toAsciiLowerCase(IMPL_RTL_USTRCODE(*pStr));

        pStr++;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
void toAsciiLowerCase_WithLength                             ( IMPL_RTL_STRCODE* pStr,
                                                               sal_Int32 nLen )
{
    assert(nLen >= 0);
    while ( nLen > 0 )
    {
        *pStr = rtl::toAsciiLowerCase(IMPL_RTL_USTRCODE(*pStr));

        pStr++;
        nLen--;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE> void toAsciiUpperCase( IMPL_RTL_STRCODE* pStr )
{
    assert(pStr);
    while ( *pStr )
    {
        *pStr = rtl::toAsciiUpperCase(IMPL_RTL_USTRCODE(*pStr));

        pStr++;
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRCODE>
void toAsciiUpperCase_WithLength                             ( IMPL_RTL_STRCODE* pStr,
                                                               sal_Int32 nLen )
{
    assert(nLen >= 0);
    while ( nLen > 0 )
    {
        *pStr = rtl::toAsciiUpperCase(IMPL_RTL_USTRCODE(*pStr));

        pStr++;
        nLen--;
    }
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

    while ( (nPreSpaces < nLen) && rtl_ImplIsWhitespace( IMPL_RTL_USTRCODE(*(pStr+nPreSpaces)) ) )
        nPreSpaces++;

    while ( (nIndex > nPreSpaces) && rtl_ImplIsWhitespace( IMPL_RTL_USTRCODE(*(pStr+nIndex)) ) )
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
            memmove(pStr, view.data(), nLen * sizeof(IMPL_RTL_STRCODE));
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

template <typename IMPL_RTL_STRCODE>
sal_Int32 valueOfInt32                             ( IMPL_RTL_STRCODE* pStr,
                                                     sal_Int32 n,
                                                     sal_Int16 nRadix )
{
    assert(pStr);
    assert( nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX );
    char    aBuf[RTL_STR_MAX_VALUEOFINT32];
    char*   pBuf = aBuf;
    sal_Int32   nLen = 0;
    sal_uInt32  nValue;

    /* Radix must be valid */
    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    /* is value negative */
    if ( n < 0 )
    {
        *pStr = '-';
        pStr++;
        nLen++;
        nValue = n == SAL_MIN_INT32 ? static_cast<sal_uInt32>(n) : -n;
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

template <typename IMPL_RTL_STRCODE>
sal_Int32 valueOfInt64                             ( IMPL_RTL_STRCODE* pStr,
                                                     sal_Int64 n,
                                                     sal_Int16 nRadix )
{
    assert(pStr);
    assert( nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX );
    char    aBuf[RTL_STR_MAX_VALUEOFINT64];
    char*   pBuf = aBuf;
    sal_Int32   nLen = 0;
    sal_uInt64  nValue;

    /* Radix must be valid */
    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    /* is value negative */
    if ( n < 0 )
    {
        *pStr = '-';
        pStr++;
        nLen++;
        nValue = n == SAL_MIN_INT64 ? static_cast<sal_uInt64>(n) : -n;
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

template <typename IMPL_RTL_STRCODE>
sal_Int32 valueOfUInt64                             ( IMPL_RTL_STRCODE* pStr,
                                                      sal_uInt64 n,
                                                      sal_Int16 nRadix )
{
    assert(pStr);
    assert( nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX );
    char    aBuf[RTL_STR_MAX_VALUEOFUINT64];
    char*   pBuf = aBuf;
    sal_Int32   nLen = 0;
    sal_uInt64  nValue;

    /* Radix must be valid */
    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

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
template <typename T, typename U, typename IMPL_RTL_STRCODE>
T toInt_WithLength                                              ( const IMPL_RTL_STRCODE* pStr,
                                                                  sal_Int16 nRadix,
                                                                  sal_Int32 nStrLength )
{
    static_assert(std::numeric_limits<T>::is_signed, "is signed");
    assert( nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX );
    assert( nStrLength >= 0 );
    bool    bNeg;
    sal_Int16   nDigit;
    U           n = 0;
    const IMPL_RTL_STRCODE* pEnd = pStr + nStrLength;

    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    /* Skip whitespaces */
    while ( pStr != pEnd && rtl_ImplIsWhitespace( IMPL_RTL_USTRCODE( *pStr ) ) )
        pStr++;

    if ( *pStr == '-' )
    {
        bNeg = true;
        pStr++;
    }
    else
    {
        if ( *pStr == '+' )
            pStr++;
        bNeg = false;
    }

    T nDiv;
    sal_Int16 nMod;
    if ( bNeg )
    {
        nDiv = std::numeric_limits<T>::min() / nRadix;
        nMod = std::numeric_limits<T>::min() % nRadix;
        // Cater for C++03 implementations that round the quotient down
        // instead of truncating towards zero as mandated by C++11:
        if ( nMod > 0 )
        {
            --nDiv;
            nMod -= nRadix;
        }
        nDiv = -nDiv;
        nMod = -nMod;
    }
    else
    {
        nDiv = std::numeric_limits<T>::max() / nRadix;
        nMod = std::numeric_limits<T>::max() % nRadix;
    }

    while ( pStr != pEnd )
    {
        nDigit = rtl_ImplGetDigit( IMPL_RTL_USTRCODE( *pStr ), nRadix );
        if ( nDigit < 0 )
            break;
        assert(nDiv > 0);
        if( static_cast<U>( nMod < nDigit ? nDiv-1 : nDiv ) < n )
            return 0;

        n *= nRadix;
        n += nDigit;

        pStr++;
    }

    if ( bNeg )
        return n == static_cast<U>(std::numeric_limits<T>::min())
            ? std::numeric_limits<T>::min() : -static_cast<T>(n);
    else
        return static_cast<T>(n);
}

template <typename IMPL_RTL_STRCODE>
sal_Int32 toInt32                             ( const IMPL_RTL_STRCODE* pStr,
                                                sal_Int16 nRadix )
{
    assert(pStr);
    return toInt_WithLength<sal_Int32, sal_uInt32>(pStr, nRadix, getLength(pStr));
}

template <typename IMPL_RTL_STRCODE>
sal_Int64 toInt64                             ( const IMPL_RTL_STRCODE* pStr,
                                                sal_Int16 nRadix )
{
    assert(pStr);
    return toInt_WithLength<sal_Int64, sal_uInt64>(pStr, nRadix, getLength(pStr));
}

template <typename IMPL_RTL_STRCODE>
sal_Int64 toInt64_WithLength                  ( const IMPL_RTL_STRCODE* pStr,
                                                sal_Int16 nRadix,
                                                sal_Int32 nStrLength)

{
    assert(pStr);
    return toInt_WithLength<sal_Int64, sal_uInt64>(pStr, nRadix, nStrLength);
}

/* ----------------------------------------------------------------------- */
template <typename T, typename IMPL_RTL_STRCODE> T toUInt( const IMPL_RTL_STRCODE* pStr,
                                                                      sal_Int16 nRadix )
{
    static_assert(!std::numeric_limits<T>::is_signed, "is not signed");
    assert( nRadix >= RTL_STR_MIN_RADIX && nRadix <= RTL_STR_MAX_RADIX );
    sal_Int16   nDigit;
    T           n = 0;

    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    /* Skip whitespaces */
    while ( *pStr && rtl_ImplIsWhitespace( IMPL_RTL_USTRCODE( *pStr ) ) )
        ++pStr;

    // skip optional explicit sign
    if ( *pStr == '+' )
        ++pStr;

    T nDiv = std::numeric_limits<T>::max() / nRadix;
    sal_Int16 nMod = std::numeric_limits<T>::max() % nRadix;
    while ( *pStr )
    {
        nDigit = rtl_ImplGetDigit( IMPL_RTL_USTRCODE( *pStr ), nRadix );
        if ( nDigit < 0 )
            break;
        if( ( nMod < nDigit ? nDiv-1 : nDiv ) < n )
            return 0;

        n *= nRadix;
        n += nDigit;

        ++pStr;
    }

    return n;
}

template <typename IMPL_RTL_STRCODE>
sal_uInt32 toUInt32                             ( const IMPL_RTL_STRCODE* pStr,
                                                  sal_Int16 nRadix )
{
    assert(pStr);
    return toUInt<sal_uInt32>(pStr, nRadix);
}

template <typename IMPL_RTL_STRCODE>
sal_uInt64 toUInt64                             ( const IMPL_RTL_STRCODE* pStr,
                                                  sal_Int16 nRadix )
{
    assert(pStr);
    return toUInt<sal_uInt64>(pStr, nRadix);
}

/* ======================================================================= */
/* Internal String-Class help functions                                    */
/* ======================================================================= */

template <typename STRINGDATA> struct STRCODE_DATA
{
    using type = std::remove_extent_t<decltype(STRINGDATA::buffer)>;
    using unsigned_type = std::make_unsigned_t<type>;
};
template <class STRINGDATA> using STRCODE = typename STRCODE_DATA<STRINGDATA>::type;
template <class STRINGDATA> using USTRCODE = typename STRCODE_DATA<STRINGDATA>::unsigned_type;

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

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
auto* NewCopy                                              ( IMPL_RTL_STRINGDATA** ppThis,
                                                             IMPL_RTL_STRINGDATA* pStr,
                                                             sal_Int32 nCount )
{
    assert(nCount >= 0);
    IMPL_RTL_STRINGDATA*    pData = Alloc<IMPL_RTL_STRINGDATA>( pStr->length );
    OSL_ASSERT(pData != nullptr);

    auto* pDest   = pData->buffer;
    auto* pSrc    = pStr->buffer;

    memcpy( pDest, pSrc, nCount * sizeof(*pSrc) );

    *ppThis = pData;

    RTL_LOG_STRING_NEW( pData );
    return pDest + nCount;
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

template <typename IMPL_RTL_STRINGDATA> struct EmptyStringImpl
{
    static IMPL_RTL_STRINGDATA data; // defined in respective units
};

template <typename IMPL_RTL_STRINGDATA> void new_( IMPL_RTL_STRINGDATA** ppThis )
{
    assert(ppThis);
    if ( *ppThis)
        release( *ppThis );

    *ppThis = &EmptyStringImpl<IMPL_RTL_STRINGDATA>::data;
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA> IMPL_RTL_STRINGDATA* alloc( sal_Int32 nLen )
{
    assert(nLen >= 0);
    return Alloc<IMPL_RTL_STRINGDATA>( nLen );
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
        OSL_ASSERT(*ppThis != nullptr);
        (*ppThis)->length   = 0;

        auto* pTempStr = (*ppThis)->buffer;
        memset(pTempStr, 0, nLen*sizeof(*pTempStr));
    }
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void newFromString                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                    const IMPL_RTL_STRINGDATA* pStr )
{
    assert(ppThis);
    assert(pStr);
    IMPL_RTL_STRINGDATA* pOrg;

    if ( !pStr->length )
    {
        new_( ppThis );
        return;
    }

    pOrg = *ppThis;
    *ppThis = Alloc<IMPL_RTL_STRINGDATA>( pStr->length );
    OSL_ASSERT(*ppThis != nullptr);
    Copy( (*ppThis)->buffer, pStr->buffer, pStr->length );
    RTL_LOG_STRING_NEW( *ppThis );

    /* must be done last, if pStr == *ppThis */
    if ( pOrg )
        release( pOrg );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void newFromStr                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                 const STRCODE<IMPL_RTL_STRINGDATA>* pCharStr )
{
    assert(ppThis);
    IMPL_RTL_STRINGDATA*    pOrg;
    sal_Int32               nLen;

#if OSL_DEBUG_LEVEL > 0
    //TODO: For now, only abort in non-production debug builds; once all places that rely on the
    // undocumented newFromStr behavior of treating a null pCharStr like an empty string have been
    // found and fixed, drop support for that behavior and turn this into a general assert:
    if (pCharStr == nullptr) {
        std::abort();
    }
#endif

    if ( pCharStr )
    {
        nLen = getLength( pCharStr );
    }
    else
        nLen = 0;

    if ( !nLen )
    {
        new_( ppThis );
        return;
    }

    pOrg = *ppThis;
    *ppThis = Alloc<IMPL_RTL_STRINGDATA>( nLen );
    OSL_ASSERT(*ppThis != nullptr);
    Copy( (*ppThis)->buffer, pCharStr, nLen );
    RTL_LOG_STRING_NEW( *ppThis );

    /* must be done last, if pCharStr == *ppThis */
    if ( pOrg )
        release( pOrg );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void newFromStr_WithLength                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                            const STRCODE<IMPL_RTL_STRINGDATA>* pCharStr,
                                                            sal_Int32 nLen )
{
    assert(ppThis);
    assert(pCharStr != nullptr || nLen == 0);
    assert(nLen >= 0);
    IMPL_RTL_STRINGDATA* pOrg;

    if ( nLen == 0 )
    {
        new_( ppThis );
        return;
    }

    pOrg = *ppThis;
    *ppThis = Alloc<IMPL_RTL_STRINGDATA>( nLen );
    OSL_ASSERT(*ppThis != nullptr);
    Copy( (*ppThis)->buffer, pCharStr, nLen );

    RTL_LOG_STRING_NEW( *ppThis );

    /* must be done last, if pCharStr == *ppThis */
    if ( pOrg )
        release( pOrg );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA> void assign(IMPL_RTL_STRINGDATA**, IMPL_RTL_STRINGDATA*);
template <typename IMPL_RTL_STRINGDATA>
void newFromLiteral(IMPL_RTL_STRINGDATA**, const char*, sal_Int32, sal_Int32);

template <typename IMPL_RTL_STRINGDATA>
void newFromSubString                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                       const IMPL_RTL_STRINGDATA* pFrom,
                                                       sal_Int32 beginIndex,
                                                       sal_Int32 count )
{
    assert(ppThis);
    if ( beginIndex == 0 && count == pFrom->length )
    {
        assign( ppThis, const_cast< IMPL_RTL_STRINGDATA * >( pFrom ) );
        return;
    }
    if ( count < 0 || beginIndex < 0 || beginIndex + count > pFrom->length )
    {
        assert(false); // fail fast at least in debug builds
        newFromLiteral( ppThis, "!!br0ken!!", 10, 0 );
        return;
    }

    newFromStr_WithLength( ppThis, pFrom->buffer + beginIndex, count );
}

/* ----------------------------------------------------------------------- */

// Used when creating from string literals.
template <typename IMPL_RTL_STRINGDATA>
void newFromLiteral                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                     const char* pCharStr,
                                                     sal_Int32 nLen,
                                                     sal_Int32 allocExtra )
{
    assert(ppThis);
    assert(nLen >= 0);
    assert(allocExtra >= 0);
    if ( nLen + allocExtra == 0 )
    {
        new_( ppThis );
        return;
    }

    if ( *ppThis )
        release( *ppThis );

    *ppThis = Alloc<IMPL_RTL_STRINGDATA>( nLen + allocExtra );
    assert( *ppThis != nullptr );

    (*ppThis)->length = nLen; // fix after possible allocExtra != 0
    (*ppThis)->buffer[nLen] = 0;
    auto* pBuffer = (*ppThis)->buffer;
    sal_Int32 nCount;
    for( nCount = nLen; nCount > 0; --nCount )
    {
        if constexpr (sizeof(STRCODE<IMPL_RTL_STRINGDATA>) == sizeof(sal_Unicode))
        {
            assert(static_cast<unsigned char>(*pCharStr) < 0x80); // ASCII range
        }
        SAL_WARN_IF( (static_cast<unsigned char>(*pCharStr)) == '\0', "rtl.string",
                    "rtl_uString_newFromLiteral - Found embedded \\0 character" );

        *pBuffer = *pCharStr;
        pBuffer++;
        pCharStr++;
    }

    RTL_LOG_STRING_NEW( *ppThis );
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

template <typename IMPL_RTL_STRINGDATA>
void newConcat                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                IMPL_RTL_STRINGDATA* pLeft,
                                                IMPL_RTL_STRINGDATA* pRight )
{
    assert(ppThis);
    IMPL_RTL_STRINGDATA* pOrg = *ppThis;

    /* Test for 0-Pointer - if not, change newReplaceStrAt! */
    if ( !pRight || !pRight->length )
    {
        *ppThis = pLeft;
        acquire( pLeft );
    }
    else if ( !pLeft || !pLeft->length )
    {
        *ppThis = pRight;
        acquire( pRight );
    }
    else if (pLeft->length
             > std::numeric_limits<sal_Int32>::max() - pRight->length)
    {
        *ppThis = nullptr;
    }
    else
    {
        auto* pTempStr = Alloc<IMPL_RTL_STRINGDATA>( pLeft->length + pRight->length );
        OSL_ASSERT(pTempStr != nullptr);
        *ppThis = pTempStr;
        if (*ppThis != nullptr) {
            Copy( pTempStr->buffer, pLeft->buffer, pLeft->length );
            Copy( pTempStr->buffer+pLeft->length, pRight->buffer, pRight->length );

            RTL_LOG_STRING_NEW( *ppThis );
        }
    }

    /* must be done last, if left or right == *ppThis */
    if ( pOrg )
        release( pOrg );
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
    if ( nIndex >= pStr->length )
    {
        /* newConcat test, if pNewSubStr is 0 */
        newConcat( ppThis, pStr, pNewSubStr );
        return;
    }

    /* negative index? */
    if ( nIndex < 0 )
    {
        nCount -= nIndex;
        nIndex = 0;
    }

    /* not more than the String length could be deleted */
    if ( nCount >= pStr->length-nIndex )
    {
        nCount = pStr->length-nIndex;

        /* Assign of NewSubStr? */
        if ( !nIndex && (nCount >= pStr->length) )
        {
            if ( !pNewSubStr )
                new_( ppThis );
            else
                assign( ppThis, pNewSubStr );
            return;
        }
    }

    /* Assign of Str? */
    if ( !nCount && (!pNewSubStr || !pNewSubStr->length) )
    {
        assign( ppThis, pStr );
        return;
    }

    IMPL_RTL_STRINGDATA*    pOrg = *ppThis;
    sal_Int32               nNewLen;

    /* Calculate length of the new string */
    nNewLen = pStr->length-nCount;
    if ( pNewSubStr )
        nNewLen += pNewSubStr->length;

    /* Alloc New Buffer */
    *ppThis = Alloc<IMPL_RTL_STRINGDATA>( nNewLen );
    OSL_ASSERT(*ppThis != nullptr);
    auto* pBuffer = (*ppThis)->buffer;
    if ( nIndex )
    {
        Copy( pBuffer, pStr->buffer, nIndex );
        pBuffer += nIndex;
    }
    if ( pNewSubStr && pNewSubStr->length )
    {
        Copy( pBuffer, pNewSubStr->buffer, pNewSubStr->length );
        pBuffer += pNewSubStr->length;
    }
    Copy( pBuffer, pStr->buffer+nIndex+nCount, pStr->length-nIndex-nCount );

    RTL_LOG_STRING_NEW( *ppThis );
    /* must be done last, if pStr or pNewSubStr == *ppThis */
    if ( pOrg )
        release( pOrg );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void newReplace                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                 IMPL_RTL_STRINGDATA* pStr,
                                                 STRCODE<IMPL_RTL_STRINGDATA> cOld,
                                                 STRCODE<IMPL_RTL_STRINGDATA> cNew )
{
    assert(ppThis);
    assert(pStr);
    IMPL_RTL_STRINGDATA*    pOrg        = *ppThis;
    bool                    bChanged    = false;
    sal_Int32               nLen        = pStr->length;
    const auto*             pCharStr    = pStr->buffer;

    while ( nLen > 0 )
    {
        if ( *pCharStr == cOld )
        {
            /* Copy String */
            auto* pNewCharStr = NewCopy( ppThis, pStr, pCharStr-pStr->buffer );

            /* replace/copy rest of the string */
            if ( pNewCharStr )
            {
                *pNewCharStr = cNew;
                pNewCharStr++;
                pCharStr++;
                nLen--;

                while ( nLen > 0 )
                {
                    if ( *pCharStr == cOld )
                        *pNewCharStr = cNew;
                    else
                        *pNewCharStr = *pCharStr;

                    pNewCharStr++;
                    pCharStr++;
                    nLen--;
                }
            }

            bChanged = true;
            break;
        }

        pCharStr++;
        nLen--;
    }

    if ( !bChanged )
    {
        *ppThis = pStr;
        acquire( pStr );
    }

    RTL_LOG_STRING_NEW( *ppThis );
    /* must be done last, if pStr == *ppThis */
    if ( pOrg )
        release( pOrg );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void newToAsciiLowerCase                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                          IMPL_RTL_STRINGDATA* pStr )
{
    assert(ppThis);
    assert(pStr);
    IMPL_RTL_STRINGDATA*    pOrg        = *ppThis;
    bool                    bChanged    = false;
    sal_Int32               nLen        = pStr->length;
    const auto*             pCharStr    = pStr->buffer;

    while ( nLen > 0 )
    {
        if ( rtl::isAsciiUpperCase(USTRCODE<IMPL_RTL_STRINGDATA>(*pCharStr)) )
        {
            /* Copy String */
            auto* pNewCharStr = NewCopy( ppThis, pStr, pCharStr-pStr->buffer );

            /* replace/copy rest of the string */
            if ( pNewCharStr )
            {
                *pNewCharStr = rtl::toAsciiLowerCase(USTRCODE<IMPL_RTL_STRINGDATA>(*pCharStr));
                pNewCharStr++;
                pCharStr++;
                nLen--;

                while ( nLen > 0 )
                {
                    *pNewCharStr = rtl::toAsciiLowerCase(USTRCODE<IMPL_RTL_STRINGDATA>(*pCharStr));

                    pNewCharStr++;
                    pCharStr++;
                    nLen--;
                }
            }

            bChanged = true;
            break;
        }

        pCharStr++;
        nLen--;
    }

    if ( !bChanged )
    {
        *ppThis = pStr;
        acquire( pStr );
    }

    RTL_LOG_STRING_NEW( *ppThis );
    /* must be done last, if pStr == *ppThis */
    if ( pOrg )
        release( pOrg );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void newToAsciiUpperCase                                ( IMPL_RTL_STRINGDATA** ppThis,
                                                          IMPL_RTL_STRINGDATA* pStr )
{
    assert(ppThis);
    assert(pStr);
    IMPL_RTL_STRINGDATA*    pOrg        = *ppThis;
    bool                    bChanged    = false;
    sal_Int32               nLen        = pStr->length;
    const auto*             pCharStr    = pStr->buffer;

    while ( nLen > 0 )
    {
        if ( rtl::isAsciiLowerCase(USTRCODE<IMPL_RTL_STRINGDATA>(*pCharStr)) )
        {
            /* Copy String */
            auto* pNewCharStr = NewCopy( ppThis, pStr, pCharStr-pStr->buffer );

            /* replace/copy rest of the string */
            if ( pNewCharStr )
            {
                *pNewCharStr = rtl::toAsciiUpperCase(USTRCODE<IMPL_RTL_STRINGDATA>(*pCharStr));
                pNewCharStr++;
                pCharStr++;
                nLen--;

                while ( nLen > 0 )
                {
                    *pNewCharStr = rtl::toAsciiUpperCase(USTRCODE<IMPL_RTL_STRINGDATA>(*pCharStr));

                    pNewCharStr++;
                    pCharStr++;
                    nLen--;
                }
            }

            bChanged = true;
            break;
        }

        pCharStr++;
        nLen--;
    }

    if ( !bChanged )
    {
        *ppThis = pStr;
        acquire( pStr );
    }

    RTL_LOG_STRING_NEW( *ppThis );
    /* must be done last, if pStr == *ppThis */
    if ( pOrg )
        release( pOrg );
}

/* ----------------------------------------------------------------------- */

template <typename IMPL_RTL_STRINGDATA>
void newTrim                                ( IMPL_RTL_STRINGDATA** ppThis,
                                              IMPL_RTL_STRINGDATA* pStr )
{
    assert(ppThis);
    assert(pStr);
    IMPL_RTL_STRINGDATA*    pOrg        = *ppThis;
    const auto view = trimView(pStr->buffer, pStr->length);

    if (static_cast<sal_Int32>(view.size()) == pStr->length)
    {
        *ppThis = pStr;
        acquire( pStr );
    }
    else
    {
        sal_Int32 nLen = static_cast<sal_Int32>(view.size());
        *ppThis = Alloc<IMPL_RTL_STRINGDATA>( nLen );
        assert(*ppThis);
        Copy( (*ppThis)->buffer, view.data(), nLen );
    }

    RTL_LOG_STRING_NEW( *ppThis );
    /* must be done last, if pStr == *ppThis */
    if ( pOrg )
        release( pOrg );
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
    const auto*             pCharStr        = pStr->buffer;
    sal_Int32               nLen            = pStr->length-nIndex;
    sal_Int32               nTokCount       = 0;

    // Set ppThis to an empty string and return -1 if either nToken or nIndex is
    // negative:
    if (nIndex < 0)
        nToken = -1;

    pCharStr += nIndex;
    const auto* pOrgCharStr = pCharStr;
    const auto* pCharStrStart = pCharStr;
    while ( nLen > 0 )
    {
        if ( *pCharStr == cTok )
        {
            nTokCount++;

            if ( nTokCount == nToken )
                pCharStrStart = pCharStr+1;
            else
            {
                if ( nTokCount > nToken )
                    break;
            }
        }

        pCharStr++;
        nLen--;
    }

    if ( (nToken < 0) || (nTokCount < nToken) || (pCharStr == pCharStrStart) )
    {
        new_( ppThis );
        if( (nToken < 0) || (nTokCount < nToken) )
            return -1;
        else if( nLen > 0 )
            return nIndex+(pCharStr-pOrgCharStr)+1;
        else return -1;
    }
    else
    {
        newFromStr_WithLength( ppThis, pCharStrStart, pCharStr-pCharStrStart );
        if ( nLen )
            return nIndex+(pCharStr-pOrgCharStr)+1;
        else
            return -1;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
