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

#include <algorithm>
#include <cassert>
#include <limits>

#include <cstring>
#include <wchar.h>
#include <sal/log.hxx>
#include <rtl/character.hxx>

/*
inline void rtl_str_ImplCopy( IMPL_RTL_STRCODE* pDest,
                              const IMPL_RTL_STRCODE* pSrc,
                              sal_Int32 nCount )
{
    while ( nCount > 0 )
    {
        *pDest = *pSrc;
        pDest++;
        pSrc++;
        nCount--;
    }
}
*/

static inline void rtl_str_ImplCopy( IMPL_RTL_STRCODE* _pDest,
                                     const IMPL_RTL_STRCODE* _pSrc,
                                     sal_Int32 _nCount )
{
    // take advantage of builtin optimisations
    memcpy( _pDest, _pSrc, _nCount * sizeof(IMPL_RTL_STRCODE));
}

/* ======================================================================= */
/* C-String functions which could be used without the String-Class         */
/* ======================================================================= */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( getLength )( const IMPL_RTL_STRCODE* pStr )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
#if !IMPL_RTL_IS_USTRING
    // take advantage of builtin optimisations
    return strlen( pStr);
#else
    if (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        return wcslen(reinterpret_cast<wchar_t const *>(pStr));
    }
    else
    {
        const IMPL_RTL_STRCODE* pTempStr = pStr;
        while( *pTempStr )
            pTempStr++;
        return pTempStr-pStr;
    }
#endif
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compare )( const IMPL_RTL_STRCODE* pStr1,
                                                const IMPL_RTL_STRCODE* pStr2 )
    SAL_THROW_EXTERN_C()
{
    assert(pStr1);
    assert(pStr2);
#if !IMPL_RTL_IS_USTRING
    // take advantage of builtin optimisations
    return strcmp( pStr1, pStr2);
#else
    if (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        return wcscmp(reinterpret_cast<wchar_t const *>(pStr1), reinterpret_cast<wchar_t const *>(pStr2));
    }
    else
    {
        sal_Int32 nRet;
        while ( ((nRet = ((sal_Int32)(IMPL_RTL_USTRCODE(*pStr1)))-
                         ((sal_Int32)(IMPL_RTL_USTRCODE(*pStr2)))) == 0) &&
                *pStr2 )
        {
            pStr1++;
            pStr2++;
        }

        return nRet;
    }
#endif
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compare_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                           sal_Int32 nStr1Len,
                                                           const IMPL_RTL_STRCODE* pStr2,
                                                           sal_Int32 nStr2Len )
    SAL_THROW_EXTERN_C()
{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
#if !IMPL_RTL_IS_USTRING
    // take advantage of builtin optimisations
    sal_Int32 nMin = std::min(nStr1Len, nStr2Len);
    sal_Int32 nRet = strncmp(pStr1, pStr2, nMin);
    return nRet == 0 ? nStr1Len - nStr2Len : nRet;
#else
    if (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        sal_Int32 nMin = std::min(nStr1Len, nStr2Len);
        sal_Int32 nRet = wcsncmp(reinterpret_cast<wchar_t const *>(pStr1), reinterpret_cast<wchar_t const *>(pStr2), nMin);
        return nRet == 0 ? nStr1Len - nStr2Len : nRet;
    }
    else
    {
        sal_Int32 nRet = nStr1Len - nStr2Len;
        int nCount = (nRet <= 0) ? nStr1Len : nStr2Len;

        --pStr1;
        --pStr2;
        while( (--nCount >= 0) && (*++pStr1 == *++pStr2) ) ;

        if( nCount >= 0 )
            nRet = ((sal_Int32)(IMPL_RTL_USTRCODE( *pStr1 )))
                 - ((sal_Int32)(IMPL_RTL_USTRCODE( *pStr2 )));

        return nRet;
    }
#endif
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( shortenedCompare_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                                    sal_Int32 nStr1Len,
                                                                    const IMPL_RTL_STRCODE* pStr2,
                                                                    sal_Int32 nStr2Len,
                                                                    sal_Int32 nShortenedLength )
    SAL_THROW_EXTERN_C()
{
    assert(nStr1Len >= 0);
    assert(nStr2Len >= 0);
    assert(nShortenedLength >= 0);
#if !IMPL_RTL_IS_USTRING
    // take advantage of builtin optimisations
    sal_Int32 nMin = std::min(std::min(nStr1Len, nStr2Len), nShortenedLength);
    sal_Int32 nRet = strncmp(pStr1, pStr2, nMin);
    if (nRet == 0 && nShortenedLength > std::min(nStr1Len, nStr2Len))
        return nStr1Len - nStr2Len;
    return nRet;
#else
    if (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        sal_Int32 nMin = std::min(std::min(nStr1Len, nStr2Len), nShortenedLength);
        sal_Int32 nRet = wcsncmp(reinterpret_cast<wchar_t const *>(pStr1), reinterpret_cast<wchar_t const *>(pStr2), nMin);
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
            nRet = ((sal_Int32)(IMPL_RTL_USTRCODE( *pStr1 )))-
                   ((sal_Int32)(IMPL_RTL_USTRCODE( *pStr2 )));
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
#endif
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( reverseCompare_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                                  sal_Int32 nStr1Len,
                                                                  const IMPL_RTL_STRCODE* pStr2,
                                                                  sal_Int32 nStr2Len )
    SAL_THROW_EXTERN_C()
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
        nRet = ((sal_Int32)(IMPL_RTL_USTRCODE( *pStr1Run )))-
               ((sal_Int32)(IMPL_RTL_USTRCODE( *pStr2Run )));
        if ( nRet )
            return nRet;
    }

    return nStr1Len - nStr2Len;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compareIgnoreAsciiCase )( const IMPL_RTL_STRCODE* pStr1,
                                                               const IMPL_RTL_STRCODE* pStr2 )
    SAL_THROW_EXTERN_C()
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

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compareIgnoreAsciiCase_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                                          sal_Int32 nStr1Len,
                                                                          const IMPL_RTL_STRCODE* pStr2,
                                                                          sal_Int32 nStr2Len )
    SAL_THROW_EXTERN_C()
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

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( shortenedCompareIgnoreAsciiCase_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                                                   sal_Int32 nStr1Len,
                                                                                   const IMPL_RTL_STRCODE* pStr2,
                                                                                   sal_Int32 nStr2Len,
                                                                                   sal_Int32 nShortenedLength )
    SAL_THROW_EXTERN_C()
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

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( hashCode )( const IMPL_RTL_STRCODE* pStr )
    SAL_THROW_EXTERN_C()
{
    return IMPL_RTL_STRNAME( hashCode_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ) );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( hashCode_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                            sal_Int32 nLen )
    SAL_THROW_EXTERN_C()
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

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( indexOfChar )( const IMPL_RTL_STRCODE* pStr,
                                                    IMPL_RTL_STRCODE c )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
#if !IMPL_RTL_IS_USTRING
    // take advantage of builtin optimisations
    const IMPL_RTL_STRCODE* p = strchr(pStr, c);
    return p ? p - pStr : -1;
#else
    if (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        wchar_t const * p = wcschr(reinterpret_cast<wchar_t const *>(pStr), (wchar_t)c);
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
#endif
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( indexOfChar_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                               sal_Int32 nLen,
                                                               IMPL_RTL_STRCODE c )
    SAL_THROW_EXTERN_C()
{
//    assert(nLen >= 0);
#if !IMPL_RTL_IS_USTRING
    // take advantage of builtin optimisations
    IMPL_RTL_STRCODE* p = static_cast<IMPL_RTL_STRCODE*>(std::memchr(const_cast<IMPL_RTL_STRCODE *>(pStr), c, nLen));
    return p ? p - pStr : -1;
#else
    const IMPL_RTL_STRCODE* pTempStr = pStr;
    while ( nLen > 0 )
    {
        if ( *pTempStr == c )
            return pTempStr-pStr;

        pTempStr++;
        nLen--;
    }

    return -1;
#endif
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( lastIndexOfChar )( const IMPL_RTL_STRCODE* pStr,
                                                        IMPL_RTL_STRCODE c )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
#if !IMPL_RTL_IS_USTRING
    // take advantage of builtin optimisations
    const IMPL_RTL_STRCODE* p = strrchr(pStr, c);
    return p ? p - pStr : -1;
#else
    if (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        wchar_t const * p = wcsrchr(reinterpret_cast<wchar_t const *>(pStr), (wchar_t)c);
        return p ? p - reinterpret_cast<wchar_t const *>(pStr) : -1;
    }
    else
    {
        return IMPL_RTL_STRNAME( lastIndexOfChar_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ), c );
    }
#endif
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( lastIndexOfChar_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                                   sal_Int32 nLen,
                                                                   IMPL_RTL_STRCODE c )
    SAL_THROW_EXTERN_C()
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

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( indexOfStr )( const IMPL_RTL_STRCODE* pStr,
                                                   const IMPL_RTL_STRCODE* pSubStr )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    assert(pSubStr);
#if !IMPL_RTL_IS_USTRING
    // take advantage of builtin optimisations
    const IMPL_RTL_STRCODE* p = strstr(pStr, pSubStr);
    return p ? p - pStr : -1;
#else
    if (sizeof(IMPL_RTL_STRCODE) == sizeof(wchar_t))
    {
        // take advantage of builtin optimisations
        wchar_t const * p = wcsstr(reinterpret_cast<wchar_t const *>(pStr), reinterpret_cast<wchar_t const *>(pSubStr));
        return p ? p - reinterpret_cast<wchar_t const *>(pStr) : -1;
    }
    else
    {
       return IMPL_RTL_STRNAME( indexOfStr_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ),
                                                         pSubStr, IMPL_RTL_STRNAME( getLength )( pSubStr ) );
    }
#endif
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( indexOfStr_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                              sal_Int32 nStrLen,
                                                              const  IMPL_RTL_STRCODE* pSubStr,
                                                              sal_Int32 nSubLen )
    SAL_THROW_EXTERN_C()
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

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( lastIndexOfStr )( const IMPL_RTL_STRCODE* pStr,
                                                       const IMPL_RTL_STRCODE* pSubStr )
    SAL_THROW_EXTERN_C()
{
    return IMPL_RTL_STRNAME( lastIndexOfStr_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ),
                                                          pSubStr, IMPL_RTL_STRNAME( getLength )( pSubStr ) );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( lastIndexOfStr_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                                  sal_Int32 nStrLen,
                                                                  const IMPL_RTL_STRCODE* pSubStr,
                                                                  sal_Int32 nSubLen )
    SAL_THROW_EXTERN_C()
{
//    assert(nStrLen >= 0);
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

void SAL_CALL IMPL_RTL_STRNAME( replaceChar )( IMPL_RTL_STRCODE* pStr,
                                               IMPL_RTL_STRCODE cOld,
                                               IMPL_RTL_STRCODE cNew )
    SAL_THROW_EXTERN_C()
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

void SAL_CALL IMPL_RTL_STRNAME( replaceChar_WithLength )( IMPL_RTL_STRCODE* pStr,
                                                          sal_Int32 nLen,
                                                          IMPL_RTL_STRCODE cOld,
                                                          IMPL_RTL_STRCODE cNew )
    SAL_THROW_EXTERN_C()
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

void SAL_CALL IMPL_RTL_STRNAME( toAsciiLowerCase )( IMPL_RTL_STRCODE* pStr )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    while ( *pStr )
    {
        *pStr = rtl::toAsciiLowerCase(IMPL_RTL_USTRCODE(*pStr));

        pStr++;
    }
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRNAME( toAsciiLowerCase_WithLength )( IMPL_RTL_STRCODE* pStr,
                                                               sal_Int32 nLen )
    SAL_THROW_EXTERN_C()
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

void SAL_CALL IMPL_RTL_STRNAME( toAsciiUpperCase )( IMPL_RTL_STRCODE* pStr )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    while ( *pStr )
    {
        *pStr = rtl::toAsciiUpperCase(IMPL_RTL_USTRCODE(*pStr));

        pStr++;
    }
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRNAME( toAsciiUpperCase_WithLength )( IMPL_RTL_STRCODE* pStr,
                                                               sal_Int32 nLen )
    SAL_THROW_EXTERN_C()
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

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( trim )( IMPL_RTL_STRCODE* pStr )
    SAL_THROW_EXTERN_C()
{
    return IMPL_RTL_STRNAME( trim_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ) );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( trim_WithLength )( IMPL_RTL_STRCODE* pStr, sal_Int32 nLen )
    SAL_THROW_EXTERN_C()
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

    if ( nPostSpaces )
    {
        nLen -= nPostSpaces;
        *(pStr+nLen) = 0;
    }

    if ( nPreSpaces )
    {
        IMPL_RTL_STRCODE* pNewStr = pStr+nPreSpaces;

        nLen -= nPreSpaces;
        nIndex = nLen;

        while ( nIndex )
        {
            *pStr = *pNewStr;
            pStr++;
            pNewStr++;
            nIndex--;
        }
        *pStr = 0;
    }

    return nLen;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( valueOfBoolean )( IMPL_RTL_STRCODE* pStr, sal_Bool b )
    SAL_THROW_EXTERN_C()
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

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( valueOfChar )( IMPL_RTL_STRCODE* pStr,
                                                    IMPL_RTL_STRCODE c )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    *pStr++ = c;
    *pStr = 0;
    return 1;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( valueOfInt32 )( IMPL_RTL_STRCODE* pStr,
                                                     sal_Int32 n,
                                                     sal_Int16 nRadix )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    sal_Char    aBuf[RTL_STR_MAX_VALUEOFINT32];
    sal_Char*   pBuf = aBuf;
    sal_Int32   nLen = 0;
    sal_uInt32  nValue;

    /* Radix must be valid */
    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    /* is value negativ */
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
        sal_Char nDigit = (sal_Char)(nValue % nRadix);
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

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( valueOfInt64 )( IMPL_RTL_STRCODE* pStr,
                                                     sal_Int64 n,
                                                     sal_Int16 nRadix )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    sal_Char    aBuf[RTL_STR_MAX_VALUEOFINT64];
    sal_Char*   pBuf = aBuf;
    sal_Int32   nLen = 0;
    sal_uInt64  nValue;

    /* Radix must be valid */
    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    /* is value negativ */
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
        sal_Char nDigit = (sal_Char)(nValue % nRadix);
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

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( valueOfUInt64 )( IMPL_RTL_STRCODE* pStr,
                                                      sal_uInt64 n,
                                                      sal_Int16 nRadix )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    sal_Char    aBuf[RTL_STR_MAX_VALUEOFUINT64];
    sal_Char*   pBuf = aBuf;
    sal_Int32   nLen = 0;
    sal_uInt64  nValue;

    /* Radix must be valid */
    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    nValue = n;

    /* create a recursive buffer with all values, except the last one */
    do
    {
        sal_Char nDigit = (sal_Char)(nValue % nRadix);
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

sal_Bool SAL_CALL IMPL_RTL_STRNAME( toBoolean )( const IMPL_RTL_STRCODE* pStr )
    SAL_THROW_EXTERN_C()
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
namespace {
    template<typename T, typename U> inline T IMPL_RTL_STRNAME( toInt )( const IMPL_RTL_STRCODE* pStr,
                                                                     sal_Int16 nRadix )
    {
        static_assert(std::numeric_limits<T>::is_signed, "is signed");
        bool    bNeg;
        sal_Int16   nDigit;
        U           n = 0;

        if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
            nRadix = 10;

        /* Skip whitespaces */
        while ( *pStr && rtl_ImplIsWhitespace( IMPL_RTL_USTRCODE( *pStr ) ) )
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

        while ( *pStr )
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
}

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( toInt32 )( const IMPL_RTL_STRCODE* pStr,
                                                sal_Int16 nRadix )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    return IMPL_RTL_STRNAME( toInt )<sal_Int32, sal_uInt32>(pStr, nRadix);
}

sal_Int64 SAL_CALL IMPL_RTL_STRNAME( toInt64 )( const IMPL_RTL_STRCODE* pStr,
                                                sal_Int16 nRadix )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    return IMPL_RTL_STRNAME( toInt )<sal_Int64, sal_uInt64>(pStr, nRadix);
}

/* ----------------------------------------------------------------------- */
namespace {
    template <typename T> inline T IMPL_RTL_STRNAME( toUInt )( const IMPL_RTL_STRCODE* pStr,
                                                                      sal_Int16 nRadix )
    {
        static_assert(!std::numeric_limits<T>::is_signed, "is not signed");
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
}

sal_uInt32 SAL_CALL IMPL_RTL_STRNAME( toUInt32 )( const IMPL_RTL_STRCODE* pStr,
                                                  sal_Int16 nRadix )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    return IMPL_RTL_STRNAME( toUInt )<sal_uInt32>(pStr, nRadix);
}

sal_uInt64 SAL_CALL IMPL_RTL_STRNAME( toUInt64 )( const IMPL_RTL_STRCODE* pStr,
                                                  sal_Int16 nRadix )
    SAL_THROW_EXTERN_C()
{
    assert(pStr);
    return IMPL_RTL_STRNAME( toUInt )<sal_uInt64>(pStr, nRadix);
}

/* ======================================================================= */
/* Internal String-Class help functions                                    */
/* ======================================================================= */

static IMPL_RTL_STRINGDATA* IMPL_RTL_STRINGNAME( ImplAlloc )( sal_Int32 nLen )
{
    IMPL_RTL_STRINGDATA * pData
        = (sal::static_int_cast< sal_uInt32 >(nLen)
           <= ((SAL_MAX_UINT32 - sizeof (IMPL_RTL_STRINGDATA))
               / sizeof (IMPL_RTL_STRCODE)))
        ? static_cast<IMPL_RTL_STRINGDATA *>(rtl_allocateMemory(
            sizeof (IMPL_RTL_STRINGDATA) + nLen * sizeof (IMPL_RTL_STRCODE)))
        : nullptr;
    if (pData != nullptr) {
        pData->refCount = 1;
        pData->length = nLen;
        pData->buffer[nLen] = 0;
    }
    return pData;
}

/* ----------------------------------------------------------------------- */

static IMPL_RTL_STRCODE* IMPL_RTL_STRINGNAME( ImplNewCopy )( IMPL_RTL_STRINGDATA** ppThis,
                                                             IMPL_RTL_STRINGDATA* pStr,
                                                             sal_Int32 nCount )
{
    assert(nCount >= 0);
    IMPL_RTL_STRCODE*       pDest;
    const IMPL_RTL_STRCODE* pSrc;
    IMPL_RTL_STRINGDATA*    pData = IMPL_RTL_STRINGNAME( ImplAlloc )( pStr->length );
    OSL_ASSERT(pData != nullptr);

    pDest   = pData->buffer;
    pSrc    = pStr->buffer;
    while ( nCount > 0 )
    {
        *pDest = *pSrc;
        pDest++;
        pSrc++;
        nCount--;
    }

    *ppThis = pData;

    RTL_LOG_STRING_NEW( pData );
    return pDest;
}

/* ======================================================================= */
/* String-Class functions                                                  */
/* ======================================================================= */

namespace {

void IMPL_RTL_ACQUIRE(IMPL_RTL_STRINGDATA * pThis)
{
    if (!SAL_STRING_IS_STATIC (pThis))
        osl_atomic_increment( &((pThis)->refCount) );
}

}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( acquire )( IMPL_RTL_STRINGDATA* pThis )
    SAL_THROW_EXTERN_C()
{
    IMPL_RTL_ACQUIRE( pThis );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( release )( IMPL_RTL_STRINGDATA* pThis )
    SAL_THROW_EXTERN_C()
{
    if (SAL_UNLIKELY(SAL_STRING_IS_STATIC (pThis)))
        return;

/* OString doesn't have an 'intern' */
#if IMPL_RTL_IS_USTRING
    if (SAL_STRING_IS_INTERN (pThis))
    {
        internRelease (pThis);
        return;
    }
#endif

    if ( !osl_atomic_decrement( &(pThis->refCount) ) )
    {
        RTL_LOG_STRING_DELETE( pThis );
        rtl_freeMemory( pThis );
    }
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( new )( IMPL_RTL_STRINGDATA** ppThis )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    if ( *ppThis)
        IMPL_RTL_STRINGNAME( release )( *ppThis );

    *ppThis = const_cast<IMPL_RTL_STRINGDATA*>(&IMPL_RTL_EMPTYSTRING);
}

/* ----------------------------------------------------------------------- */

IMPL_RTL_STRINGDATA* SAL_CALL IMPL_RTL_STRINGNAME( alloc )( sal_Int32 nLen )
    SAL_THROW_EXTERN_C()
{
    if ( nLen < 0 )
        return nullptr;
    else
        return IMPL_RTL_STRINGNAME( ImplAlloc )( nLen );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( new_WithLength )( IMPL_RTL_STRINGDATA** ppThis, sal_Int32 nLen )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    if ( nLen <= 0 )
        IMPL_RTL_STRINGNAME( new )( ppThis );
    else
    {
        if ( *ppThis)
            IMPL_RTL_STRINGNAME( release )( *ppThis );

        *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen );
        OSL_ASSERT(*ppThis != nullptr);
        (*ppThis)->length   = 0;

        IMPL_RTL_STRCODE* pTempStr = (*ppThis)->buffer;
        memset(pTempStr, 0, nLen*sizeof(IMPL_RTL_STRCODE));
    }
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newFromString )( IMPL_RTL_STRINGDATA** ppThis,
                                                    const IMPL_RTL_STRINGDATA* pStr )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    assert(pStr);
    IMPL_RTL_STRINGDATA* pOrg;

    if ( !pStr->length )
    {
        IMPL_RTL_STRINGNAME( new )( ppThis );
        return;
    }

    pOrg = *ppThis;
    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( pStr->length );
    OSL_ASSERT(*ppThis != nullptr);
    rtl_str_ImplCopy( (*ppThis)->buffer, pStr->buffer, pStr->length );
    RTL_LOG_STRING_NEW( *ppThis );

    /* must be done last, if pStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newFromStr )( IMPL_RTL_STRINGDATA** ppThis,
                                                 const IMPL_RTL_STRCODE* pCharStr )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    IMPL_RTL_STRCODE*       pBuffer;
    IMPL_RTL_STRINGDATA*    pOrg;
    sal_Int32               nLen;

    if ( pCharStr )
    {
        const IMPL_RTL_STRCODE* pTempStr = pCharStr;
        while( *pTempStr )
            pTempStr++;
        nLen = pTempStr-pCharStr;
    }
    else
        nLen = 0;

    if ( !nLen )
    {
        IMPL_RTL_STRINGNAME( new )( ppThis );
        return;
    }

    pOrg = *ppThis;
    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen );
    OSL_ASSERT(*ppThis != nullptr);
    pBuffer = (*ppThis)->buffer;
    do
    {
        *pBuffer = *pCharStr;
        pBuffer++;
        pCharStr++;
    }
    while ( *pCharStr );

    RTL_LOG_STRING_NEW( *ppThis );

    /* must be done last, if pCharStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newFromStr_WithLength )( IMPL_RTL_STRINGDATA** ppThis,
                                                            const IMPL_RTL_STRCODE* pCharStr,
                                                            sal_Int32 nLen )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    IMPL_RTL_STRINGDATA* pOrg;

    if ( !pCharStr || (nLen <= 0) )
    {
        IMPL_RTL_STRINGNAME( new )( ppThis );
        return;
    }

    pOrg = *ppThis;
    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen );
    OSL_ASSERT(*ppThis != nullptr);
    rtl_str_ImplCopy( (*ppThis)->buffer, pCharStr, nLen );

    RTL_LOG_STRING_NEW( *ppThis );

    /* must be done last, if pCharStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newFromSubString )( IMPL_RTL_STRINGDATA** ppThis,
                                                       const IMPL_RTL_STRINGDATA* pFrom,
                                                       sal_Int32 beginIndex,
                                                       sal_Int32 count )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    if ( beginIndex == 0 && count == pFrom->length )
    {
        IMPL_RTL_STRINGNAME( assign )( ppThis, const_cast< IMPL_RTL_STRINGDATA * >( pFrom ) );
        return;
    }
    if ( count < 0 || beginIndex < 0 || beginIndex + count > pFrom->length )
    {
        assert(false); // fail fast at least in debug builds
        IMPL_RTL_STRINGNAME( newFromLiteral )( ppThis, "!!br0ken!!", 10, 0 );
        return;
    }

    IMPL_RTL_STRINGNAME( newFromStr_WithLength )( ppThis, pFrom->buffer + beginIndex, count );
}

/* ----------------------------------------------------------------------- */

// Used when creating from string literals.
void SAL_CALL IMPL_RTL_STRINGNAME( newFromLiteral )( IMPL_RTL_STRINGDATA** ppThis,
                                                     const sal_Char* pCharStr,
                                                     sal_Int32 nLen,
                                                     sal_Int32 allocExtra )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    assert(nLen >= 0);
    assert(allocExtra >= 0);
    if ( nLen + allocExtra == 0 )
    {
        IMPL_RTL_STRINGNAME( new )( ppThis );
        return;
    }

    if ( *ppThis )
        IMPL_RTL_STRINGNAME( release )( *ppThis );

    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen + allocExtra );
    assert( *ppThis != nullptr );

    (*ppThis)->length = nLen; // fix after possible allocExtra != 0
    (*ppThis)->buffer[nLen] = 0;
    IMPL_RTL_STRCODE* pBuffer = (*ppThis)->buffer;
    sal_Int32 nCount;
    for( nCount = nLen; nCount > 0; --nCount )
    {
#if IMPL_RTL_IS_USTRING
        assert(static_cast<unsigned char>(*pCharStr) < 0x80); // ASCII range
#endif
        SAL_WARN_IF( ((unsigned char)*pCharStr) == '\0', "rtl.string",
                    "rtl_uString_newFromLiteral - Found embedded \\0 character" );

        *pBuffer = *pCharStr;
        pBuffer++;
        pCharStr++;
    }

    RTL_LOG_STRING_NEW( *ppThis );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( assign )( IMPL_RTL_STRINGDATA** ppThis,
                                             IMPL_RTL_STRINGDATA* pStr )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    /* must be done at first, if pStr == *ppThis */
    IMPL_RTL_ACQUIRE( pStr );

    if ( *ppThis )
        IMPL_RTL_STRINGNAME( release )( *ppThis );

    *ppThis = pStr;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRINGNAME( getLength )( const IMPL_RTL_STRINGDATA* pThis )
    SAL_THROW_EXTERN_C()
{
    assert(pThis);
    return pThis->length;
}

/* ----------------------------------------------------------------------- */

IMPL_RTL_STRCODE* SAL_CALL IMPL_RTL_STRINGNAME( getStr )( IMPL_RTL_STRINGDATA * pThis )
    SAL_THROW_EXTERN_C()
{
    assert(pThis);
    return pThis->buffer;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newConcat )( IMPL_RTL_STRINGDATA** ppThis,
                                                IMPL_RTL_STRINGDATA* pLeft,
                                                IMPL_RTL_STRINGDATA* pRight )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    IMPL_RTL_STRINGDATA* pOrg = *ppThis;

    /* Test for 0-Pointer - if not, change newReplaceStrAt! */
    if ( !pRight || !pRight->length )
    {
        *ppThis = pLeft;
        IMPL_RTL_ACQUIRE( pLeft );
    }
    else if ( !pLeft || !pLeft->length )
    {
        *ppThis = pRight;
        IMPL_RTL_ACQUIRE( pRight );
    }
    else
    {
        IMPL_RTL_STRINGDATA* pTempStr = IMPL_RTL_STRINGNAME( ImplAlloc )( pLeft->length + pRight->length );
        OSL_ASSERT(pTempStr != nullptr);
        rtl_str_ImplCopy( pTempStr->buffer, pLeft->buffer, pLeft->length );
        rtl_str_ImplCopy( pTempStr->buffer+pLeft->length, pRight->buffer, pRight->length );
        *ppThis = pTempStr;

        RTL_LOG_STRING_NEW( *ppThis );
    }

    /* must be done last, if left or right == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( ensureCapacity )( IMPL_RTL_STRINGDATA** ppThis,
                                                     sal_Int32 size )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    IMPL_RTL_STRINGDATA* const pOrg = *ppThis;
    if ( pOrg->refCount == 1 && pOrg->length >= size )
        return;
    assert( pOrg->length <= size ); // do not truncate
    IMPL_RTL_STRINGDATA* pTempStr = IMPL_RTL_STRINGNAME( ImplAlloc )( size );
    rtl_str_ImplCopy( pTempStr->buffer, pOrg->buffer, pOrg->length );
    // right now the length is still the same as of the original
    pTempStr->length = pOrg->length;
    pTempStr->buffer[ pOrg->length ] = '\0';
    *ppThis = pTempStr;
    RTL_LOG_STRING_NEW( *ppThis );

    IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newReplaceStrAt )( IMPL_RTL_STRINGDATA** ppThis,
                                                      IMPL_RTL_STRINGDATA* pStr,
                                                      sal_Int32 nIndex,
                                                      sal_Int32 nCount,
                                                      IMPL_RTL_STRINGDATA* pNewSubStr )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
//    assert(nCount >= 0);
    /* Append? */
    if ( nIndex >= pStr->length )
    {
        /* newConcat test, if pNewSubStr is 0 */
        IMPL_RTL_STRINGNAME( newConcat )( ppThis, pStr, pNewSubStr );
        return;
    }

    /* negativ index? */
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
                IMPL_RTL_STRINGNAME( new )( ppThis );
            else
                IMPL_RTL_STRINGNAME( assign )( ppThis, pNewSubStr );
            return;
        }
    }

    /* Assign of Str? */
    if ( !nCount && (!pNewSubStr || !pNewSubStr->length) )
    {
        IMPL_RTL_STRINGNAME( assign )( ppThis, pStr );
        return;
    }

    IMPL_RTL_STRINGDATA*    pOrg = *ppThis;
    IMPL_RTL_STRCODE*       pBuffer;
    sal_Int32               nNewLen;

    /* Calculate length of the new string */
    nNewLen = pStr->length-nCount;
    if ( pNewSubStr )
        nNewLen += pNewSubStr->length;

    /* Alloc New Buffer */
    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nNewLen );
    OSL_ASSERT(*ppThis != nullptr);
    pBuffer = (*ppThis)->buffer;
    if ( nIndex )
    {
        rtl_str_ImplCopy( pBuffer, pStr->buffer, nIndex );
        pBuffer += nIndex;
    }
    if ( pNewSubStr && pNewSubStr->length )
    {
        rtl_str_ImplCopy( pBuffer, pNewSubStr->buffer, pNewSubStr->length );
        pBuffer += pNewSubStr->length;
    }
    rtl_str_ImplCopy( pBuffer, pStr->buffer+nIndex+nCount, pStr->length-nIndex-nCount );

    RTL_LOG_STRING_NEW( *ppThis );
    /* must be done last, if pStr or pNewSubStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newReplace )( IMPL_RTL_STRINGDATA** ppThis,
                                                 IMPL_RTL_STRINGDATA* pStr,
                                                 IMPL_RTL_STRCODE cOld,
                                                 IMPL_RTL_STRCODE cNew )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    assert(pStr);
    IMPL_RTL_STRINGDATA*    pOrg        = *ppThis;
    int                     bChanged    = 0;
    sal_Int32               nLen        = pStr->length;
    const IMPL_RTL_STRCODE* pCharStr    = pStr->buffer;

    while ( nLen > 0 )
    {
        if ( *pCharStr == cOld )
        {
            /* Copy String */
            IMPL_RTL_STRCODE* pNewCharStr = IMPL_RTL_STRINGNAME( ImplNewCopy )( ppThis, pStr, pCharStr-pStr->buffer );

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

            bChanged = 1;
            break;
        }

        pCharStr++;
        nLen--;
    }

    if ( !bChanged )
    {
        *ppThis = pStr;
        IMPL_RTL_ACQUIRE( pStr );
    }

    RTL_LOG_STRING_NEW( *ppThis );
    /* must be done last, if pStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newToAsciiLowerCase )( IMPL_RTL_STRINGDATA** ppThis,
                                                          IMPL_RTL_STRINGDATA* pStr )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    assert(pStr);
    IMPL_RTL_STRINGDATA*    pOrg        = *ppThis;
    int                     bChanged    = 0;
    sal_Int32               nLen        = pStr->length;
    const IMPL_RTL_STRCODE* pCharStr    = pStr->buffer;

    while ( nLen > 0 )
    {
        if ( rtl::isAsciiUpperCase(IMPL_RTL_USTRCODE(*pCharStr)) )
        {
            /* Copy String */
            IMPL_RTL_STRCODE* pNewCharStr = IMPL_RTL_STRINGNAME( ImplNewCopy )( ppThis, pStr, pCharStr-pStr->buffer );

            /* replace/copy rest of the string */
            if ( pNewCharStr )
            {
                *pNewCharStr = rtl::toAsciiLowerCase(IMPL_RTL_USTRCODE(*pCharStr));
                pNewCharStr++;
                pCharStr++;
                nLen--;

                while ( nLen > 0 )
                {
                    *pNewCharStr = rtl::toAsciiLowerCase(IMPL_RTL_USTRCODE(*pCharStr));

                    pNewCharStr++;
                    pCharStr++;
                    nLen--;
                }
            }

            bChanged = 1;
            break;
        }

        pCharStr++;
        nLen--;
    }

    if ( !bChanged )
    {
        *ppThis = pStr;
        IMPL_RTL_ACQUIRE( pStr );
    }

    RTL_LOG_STRING_NEW( *ppThis );
    /* must be done last, if pStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newToAsciiUpperCase )( IMPL_RTL_STRINGDATA** ppThis,
                                                          IMPL_RTL_STRINGDATA* pStr )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    assert(pStr);
    IMPL_RTL_STRINGDATA*    pOrg        = *ppThis;
    int                     bChanged    = 0;
    sal_Int32               nLen        = pStr->length;
    const IMPL_RTL_STRCODE* pCharStr    = pStr->buffer;

    while ( nLen > 0 )
    {
        if ( rtl::isAsciiLowerCase(IMPL_RTL_USTRCODE(*pCharStr)) )
        {
            /* Copy String */
            IMPL_RTL_STRCODE* pNewCharStr = IMPL_RTL_STRINGNAME( ImplNewCopy )( ppThis, pStr, pCharStr-pStr->buffer );

            /* replace/copy rest of the string */
            if ( pNewCharStr )
            {
                *pNewCharStr = rtl::toAsciiUpperCase(IMPL_RTL_USTRCODE(*pCharStr));
                pNewCharStr++;
                pCharStr++;
                nLen--;

                while ( nLen > 0 )
                {
                    *pNewCharStr = rtl::toAsciiUpperCase(IMPL_RTL_USTRCODE(*pCharStr));

                    pNewCharStr++;
                    pCharStr++;
                    nLen--;
                }
            }

            bChanged = 1;
            break;
        }

        pCharStr++;
        nLen--;
    }

    if ( !bChanged )
    {
        *ppThis = pStr;
        IMPL_RTL_ACQUIRE( pStr );
    }

    RTL_LOG_STRING_NEW( *ppThis );
    /* must be done last, if pStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newTrim )( IMPL_RTL_STRINGDATA** ppThis,
                                              IMPL_RTL_STRINGDATA* pStr )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    assert(pStr);
    IMPL_RTL_STRINGDATA*    pOrg        = *ppThis;
    const IMPL_RTL_STRCODE* pCharStr    = pStr->buffer;
    sal_Int32               nPreSpaces  = 0;
    sal_Int32               nPostSpaces = 0;
    sal_Int32               nLen        = pStr->length;
    sal_Int32               nIndex      = nLen-1;

    while ( (nPreSpaces < nLen) && rtl_ImplIsWhitespace( IMPL_RTL_USTRCODE(*(pCharStr+nPreSpaces)) ) )
        nPreSpaces++;

    while ( (nIndex > nPreSpaces) && rtl_ImplIsWhitespace( IMPL_RTL_USTRCODE(*(pCharStr+nIndex)) ) )
    {
        nPostSpaces++;
        nIndex--;
    }

    if ( !nPreSpaces && !nPostSpaces )
    {
        *ppThis = pStr;
        IMPL_RTL_ACQUIRE( pStr );
    }
    else
    {
        nLen -= nPostSpaces+nPreSpaces;
        *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen );
        assert(*ppThis);
        rtl_str_ImplCopy( (*ppThis)->buffer, pStr->buffer+nPreSpaces, nLen );
    }

    RTL_LOG_STRING_NEW( *ppThis );
    /* must be done last, if pStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRINGNAME( getToken )( IMPL_RTL_STRINGDATA** ppThis,
                                                    IMPL_RTL_STRINGDATA* pStr,
                                                    sal_Int32 nToken,
                                                    IMPL_RTL_STRCODE cTok,
                                                    sal_Int32 nIndex )
    SAL_THROW_EXTERN_C()
{
    assert(ppThis);
    assert(pStr);
    const IMPL_RTL_STRCODE* pCharStr        = pStr->buffer;
    const IMPL_RTL_STRCODE* pCharStrStart;
    const IMPL_RTL_STRCODE* pOrgCharStr;
    sal_Int32               nLen            = pStr->length-nIndex;
    sal_Int32               nTokCount       = 0;

    // Set ppThis to an empty string and return -1 if either nToken or nIndex is
    // negative:
    if (nIndex < 0)
        nToken = -1;

    pCharStr += nIndex;
    pOrgCharStr = pCharStr;
    pCharStrStart = pCharStr;
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
        IMPL_RTL_STRINGNAME( new )( ppThis );
        if( (nToken < 0) || (nTokCount < nToken ) )
            return -1;
        else if( nLen > 0 )
            return nIndex+(pCharStr-pOrgCharStr)+1;
        else return -1;
    }
    else
    {
        IMPL_RTL_STRINGNAME( newFromStr_WithLength )( ppThis, pCharStrStart, pCharStr-pCharStrStart );
        if ( nLen )
            return nIndex+(pCharStr-pOrgCharStr)+1;
        else
            return -1;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
