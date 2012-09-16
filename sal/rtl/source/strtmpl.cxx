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

/* ======================================================================= */
/* Internal C-String help functions which could be used without the        */
/* String-Class                                                            */
/* ======================================================================= */

#include <string.h>
#include <sal/log.hxx>

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

#define rtl_str_ImplCopy( _pDest, _pSrc, _nCount )                  \
{                                                                   \
    IMPL_RTL_STRCODE*       __mm_pDest      = _pDest;               \
    const IMPL_RTL_STRCODE* __mm_pSrc       = _pSrc;                \
    sal_Int32               __mm_nCount     = _nCount;              \
    while ( __mm_nCount > 0 )                                       \
    {                                                               \
        *__mm_pDest = *__mm_pSrc;                                   \
        __mm_pDest++;                                               \
        __mm_pSrc++;                                                \
        __mm_nCount--;                                              \
    }                                                               \
}

/* ======================================================================= */
/* C-String functions which could be used without the String-Class         */
/* ======================================================================= */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( getLength )( const IMPL_RTL_STRCODE* pStr )
    SAL_THROW_EXTERN_C()
{
    const IMPL_RTL_STRCODE* pTempStr = pStr;
    while( *pTempStr )
        pTempStr++;
    return pTempStr-pStr;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compare )( const IMPL_RTL_STRCODE* pStr1,
                                                const IMPL_RTL_STRCODE* pStr2 )
    SAL_THROW_EXTERN_C()
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

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compare_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                           sal_Int32 nStr1Len,
                                                           const IMPL_RTL_STRCODE* pStr2,
                                                           sal_Int32 nStr2Len )
    SAL_THROW_EXTERN_C()
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

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( shortenedCompare_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                                    sal_Int32 nStr1Len,
                                                                    const IMPL_RTL_STRCODE* pStr2,
                                                                    sal_Int32 nStr2Len,
                                                                    sal_Int32 nShortenedLength )
    SAL_THROW_EXTERN_C()
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

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( reverseCompare_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                                  sal_Int32 nStr1Len,
                                                                  const IMPL_RTL_STRCODE* pStr2,
                                                                  sal_Int32 nStr2Len )
    SAL_THROW_EXTERN_C()
{
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
    sal_Int32   nRet;
    sal_Int32   c1;
    sal_Int32   c2;
    do
    {
        /* If character between 'A' and 'Z', than convert it to lowercase */
        c1 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr1 );
        c2 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr2 );
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = c1-c2;
        if ( nRet != 0 )
            return nRet;

        pStr1++;
        pStr2++;
    }
    while ( c2 );

    return 0;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compareIgnoreAsciiCase_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                                          sal_Int32 nStr1Len,
                                                                          const IMPL_RTL_STRCODE* pStr2,
                                                                          sal_Int32 nStr2Len )
    SAL_THROW_EXTERN_C()
{
    const IMPL_RTL_STRCODE* pStr1End = pStr1 + nStr1Len;
    const IMPL_RTL_STRCODE* pStr2End = pStr2 + nStr2Len;
    sal_Int32   nRet;
    sal_Int32   c1;
    sal_Int32   c2;
    while ( (pStr1 < pStr1End) && (pStr2 < pStr2End) )
    {
        /* If character between 'A' and 'Z', than convert it to lowercase */
        c1 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr1 );
        c2 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr2 );
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = c1-c2;
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
    const IMPL_RTL_STRCODE* pStr1End = pStr1 + nStr1Len;
    const IMPL_RTL_STRCODE* pStr2End = pStr2 + nStr2Len;
    sal_Int32               nRet;
    sal_Int32               c1;
    sal_Int32               c2;
    while ( (nShortenedLength > 0) &&
            (pStr1 < pStr1End) && (pStr2 < pStr2End) )
    {
        /* If character between 'A' and 'Z', than convert it to lowercase */
        c1 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr1 );
        c2 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr2 );
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = c1-c2;
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
    sal_Int32 h = nLen;

    if ( nLen < 256 )
    {
        while ( nLen > 0 )
        {
            h = (h*37) + IMPL_RTL_USTRCODE( *pStr );
            pStr++;
            nLen--;
        }
    }
    else
    {
        sal_Int32               nSkip;
        const IMPL_RTL_STRCODE* pEndStr = pStr+nLen-5;

        /* only sample some characters */
        /* the first 3, some characters between, and the last 5 */
        h = (h*39) + IMPL_RTL_USTRCODE( *pStr );
        pStr++;
        h = (h*39) + IMPL_RTL_USTRCODE( *pStr );
        pStr++;
        h = (h*39) + IMPL_RTL_USTRCODE( *pStr );
        pStr++;

        if ( nLen < 32 )
            nSkip = nLen / 4;
        else
            nSkip = nLen / 8;
        nLen -= 8;
        while ( nLen > 0 )
        {
            h = (h*39) + IMPL_RTL_USTRCODE( *pStr );
            pStr += nSkip;
            nLen -= nSkip;
        }

        h = (h*39) + IMPL_RTL_USTRCODE( *pEndStr );
        pEndStr++;
        h = (h*39) + IMPL_RTL_USTRCODE( *pEndStr );
        pEndStr++;
        h = (h*39) + IMPL_RTL_USTRCODE( *pEndStr );
        pEndStr++;
        h = (h*39) + IMPL_RTL_USTRCODE( *pEndStr );
        pEndStr++;
        h = (h*39) + IMPL_RTL_USTRCODE( *pEndStr );
    }

    return h;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( indexOfChar )( const IMPL_RTL_STRCODE* pStr,
                                                    IMPL_RTL_STRCODE c )
    SAL_THROW_EXTERN_C()
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

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( indexOfChar_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                               sal_Int32 nLen,
                                                               IMPL_RTL_STRCODE c )
    SAL_THROW_EXTERN_C()
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

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( lastIndexOfChar )( const IMPL_RTL_STRCODE* pStr,
                                                        IMPL_RTL_STRCODE c )
    SAL_THROW_EXTERN_C()
{
    return IMPL_RTL_STRNAME( lastIndexOfChar_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ), c );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( lastIndexOfChar_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                                   sal_Int32 nLen,
                                                                   IMPL_RTL_STRCODE c )
    SAL_THROW_EXTERN_C()
{
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
    return IMPL_RTL_STRNAME( indexOfStr_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ),
                                                      pSubStr, IMPL_RTL_STRNAME( getLength )( pSubStr ) );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( indexOfStr_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                              sal_Int32 nStrLen,
                                                              const  IMPL_RTL_STRCODE* pSubStr,
                                                              sal_Int32 nSubLen )
    SAL_THROW_EXTERN_C()
{
    /* faster search for a single character */
    if ( nSubLen < 2 )
    {
        /* an empty SubString is always not foundable */
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
    /* faster search for a single character */
    if ( nSubLen < 2 )
    {
        /* an empty SubString is always not foundable */
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
    while ( *pStr )
    {
        /* Between A-Z (65-90), than to lowercase (+32) */
        if ( (*pStr >= 65) && (*pStr <= 90) )
            *pStr += 32;

        pStr++;
    }
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRNAME( toAsciiLowerCase_WithLength )( IMPL_RTL_STRCODE* pStr,
                                                               sal_Int32 nLen )
    SAL_THROW_EXTERN_C()
{
    while ( nLen > 0 )
    {
        /* Between A-Z (65-90), than to lowercase (+32) */
        if ( (*pStr >= 65) && (*pStr <= 90) )
            *pStr += 32;

        pStr++;
        nLen--;
    }
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRNAME( toAsciiUpperCase )( IMPL_RTL_STRCODE* pStr )
    SAL_THROW_EXTERN_C()
{
    while ( *pStr )
    {
        /* Between a-z (97-122), than to uppercase (-32) */
        if ( (*pStr >= 97) && (*pStr <= 122) )
            *pStr -= 32;

        pStr++;
    }
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRNAME( toAsciiUpperCase_WithLength )( IMPL_RTL_STRCODE* pStr,
                                                               sal_Int32 nLen )
    SAL_THROW_EXTERN_C()
{
    while ( nLen > 0 )
    {
        /* Between a-z (97-122), than to uppercase (-32) */
        if ( (*pStr >= 97) && (*pStr <= 122) )
            *pStr -= 32;

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
        nValue = -n; /* FIXME this code is not portable for n == -2147483648
                        (smallest negative value for sal_Int32) */
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
        nValue = -n; /* FIXME this code is not portable for
                        n == -9223372036854775808 (smallest negative value for
                        sal_Int64) */
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

sal_Bool SAL_CALL IMPL_RTL_STRNAME( toBoolean )( const IMPL_RTL_STRCODE* pStr )
    SAL_THROW_EXTERN_C()
{
    if ( *pStr == '1' )
        return sal_True;

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
                    return sal_True;
            }
        }
    }

    return sal_False;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( toInt32 )( const IMPL_RTL_STRCODE* pStr,
                                                sal_Int16 nRadix )
    SAL_THROW_EXTERN_C()
{
    sal_Bool    bNeg;
    sal_Int16   nDigit;
    sal_Int32   n = 0;

    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    /* Skip whitespaces */
    while ( *pStr && rtl_ImplIsWhitespace( IMPL_RTL_USTRCODE( *pStr ) ) )
        pStr++;

    if ( *pStr == '-' )
    {
        bNeg = sal_True;
        pStr++;
    }
    else
    {
        if ( *pStr == '+' )
            pStr++;
        bNeg = sal_False;
    }

    while ( *pStr )
    {
        nDigit = rtl_ImplGetDigit( IMPL_RTL_USTRCODE( *pStr ), nRadix );
        if ( nDigit < 0 )
            break;

        n *= nRadix;
        n += nDigit;

        pStr++;
    }

    if ( bNeg )
        return -n;
    else
        return n;
}

/* ----------------------------------------------------------------------- */

sal_Int64 SAL_CALL IMPL_RTL_STRNAME( toInt64 )( const IMPL_RTL_STRCODE* pStr,
                                                sal_Int16 nRadix )
    SAL_THROW_EXTERN_C()
{
    sal_Bool    bNeg;
    sal_Int16   nDigit;
    sal_Int64   n = 0;

    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    /* Skip whitespaces */
    while ( *pStr && rtl_ImplIsWhitespace( IMPL_RTL_USTRCODE( *pStr ) ) )
        pStr++;

    if ( *pStr == '-' )
    {
        bNeg = sal_True;
        pStr++;
    }
    else
    {
        if ( *pStr == '+' )
            pStr++;
        bNeg = sal_False;
    }

    while ( *pStr )
    {
        nDigit = rtl_ImplGetDigit( IMPL_RTL_USTRCODE( *pStr ), nRadix );
        if ( nDigit < 0 )
            break;

        n *= nRadix;
        n += nDigit;

        pStr++;
    }

    if ( bNeg )
        return -n;
    else
        return n;
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
        ? (IMPL_RTL_STRINGDATA *) rtl_allocateMemory(
            sizeof (IMPL_RTL_STRINGDATA) + nLen * sizeof (IMPL_RTL_STRCODE))
        : NULL;
    if (pData != NULL) {
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
    IMPL_RTL_STRCODE*       pDest;
    const IMPL_RTL_STRCODE* pSrc;
    IMPL_RTL_STRINGDATA*    pData = IMPL_RTL_STRINGNAME( ImplAlloc )( pStr->length );
    OSL_ASSERT(pData != NULL);

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

#define IMPL_RTL_AQUIRE( pThis )                                \
{                                                               \
    if (!SAL_STRING_IS_STATIC (pThis))                          \
        osl_atomic_increment( &((pThis)->refCount) );  \
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( acquire )( IMPL_RTL_STRINGDATA* pThis )
    SAL_THROW_EXTERN_C()
{
    IMPL_RTL_AQUIRE( pThis );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( release )( IMPL_RTL_STRINGDATA* pThis )
    SAL_THROW_EXTERN_C()
{
    if (SAL_STRING_IS_STATIC (pThis))
        return;

/* OString doesn't have an 'intern' */
#ifdef IMPL_RTL_INTERN
    if (SAL_STRING_IS_INTERN (pThis))
    {
        internRelease (pThis);
        return;
    }
#endif

    if ( pThis->refCount == 1 ||
         !osl_atomic_decrement( &(pThis->refCount) ) )
    {
        RTL_LOG_STRING_DELETE( pThis );
        rtl_freeMemory( pThis );
    }
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( new )( IMPL_RTL_STRINGDATA** ppThis )
    SAL_THROW_EXTERN_C()
{
    if ( *ppThis)
        IMPL_RTL_STRINGNAME( release )( *ppThis );

    *ppThis = (IMPL_RTL_STRINGDATA*) (&IMPL_RTL_EMPTYSTRING);
    IMPL_RTL_AQUIRE( *ppThis );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( new_WithLength )( IMPL_RTL_STRINGDATA** ppThis, sal_Int32 nLen )
    SAL_THROW_EXTERN_C()
{
    if ( nLen <= 0 )
        IMPL_RTL_STRINGNAME( new )( ppThis );
    else
    {
        if ( *ppThis)
            IMPL_RTL_STRINGNAME( release )( *ppThis );

        *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen );
        OSL_ASSERT(*ppThis != NULL);
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
    IMPL_RTL_STRINGDATA* pOrg;

    if ( !pStr->length )
    {
        IMPL_RTL_STRINGNAME( new )( ppThis );
        return;
    }

    pOrg = *ppThis;
    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( pStr->length );
    OSL_ASSERT(*ppThis != NULL);
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
    OSL_ASSERT(*ppThis != NULL);
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
    IMPL_RTL_STRINGDATA* pOrg;

    if ( !pCharStr || (nLen <= 0) )
    {
        IMPL_RTL_STRINGNAME( new )( ppThis );
        return;
    }

    pOrg = *ppThis;
    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen );
    OSL_ASSERT(*ppThis != NULL);
    rtl_str_ImplCopy( (*ppThis)->buffer, pCharStr, nLen );

    RTL_LOG_STRING_NEW( *ppThis );

    /* must be done last, if pCharStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

// Used when creating from string literals.
void SAL_CALL IMPL_RTL_STRINGNAME( newFromLiteral )( IMPL_RTL_STRINGDATA** ppThis,
                                                     const sal_Char* pCharStr,
                                                     sal_Int32 nLen,
                                                     sal_Int32 allocExtra )
    SAL_THROW_EXTERN_C()
{
    if ( !nLen )
    {
        IMPL_RTL_STRINGNAME( new )( ppThis );
        return;
    }

    if ( *ppThis )
        IMPL_RTL_STRINGNAME( release )( *ppThis );

    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen + allocExtra );
    assert( *ppThis != NULL );
    if ( (*ppThis) )
    {
        (*ppThis)->length = nLen; // fix after possible allocExtra != 0
        (*ppThis)->buffer[nLen] = 0;
        IMPL_RTL_STRCODE* pBuffer = (*ppThis)->buffer;
        sal_Int32 nCount;
        for( nCount = nLen; nCount > 0; --nCount )
        {
            /* Check ASCII range */
            SAL_WARN_IF( ((unsigned char)*pCharStr) > 127, "rtl.string",
                        "rtl_uString_newFromLiteral - Found char > 127" );
            SAL_WARN_IF( ((unsigned char)*pCharStr) == '\0', "rtl.string",
                        "rtl_uString_newFromLiteral - Found embedded \\0 character" );

            *pBuffer = *pCharStr;
            pBuffer++;
            pCharStr++;
        }
    }

    RTL_LOG_STRING_NEW( *ppThis );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( assign )( IMPL_RTL_STRINGDATA** ppThis,
                                             IMPL_RTL_STRINGDATA* pStr )
    SAL_THROW_EXTERN_C()
{
    /* must be done at first, if pStr == *ppThis */
    IMPL_RTL_AQUIRE( pStr );

    if ( *ppThis )
        IMPL_RTL_STRINGNAME( release )( *ppThis );

    *ppThis = pStr;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRINGNAME( getLength )( const IMPL_RTL_STRINGDATA* pThis )
    SAL_THROW_EXTERN_C()
{
    return pThis->length;
}

/* ----------------------------------------------------------------------- */

IMPL_RTL_STRCODE* SAL_CALL IMPL_RTL_STRINGNAME( getStr )( IMPL_RTL_STRINGDATA * pThis )
    SAL_THROW_EXTERN_C()
{
    return pThis->buffer;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newConcat )( IMPL_RTL_STRINGDATA** ppThis,
                                                IMPL_RTL_STRINGDATA* pLeft,
                                                IMPL_RTL_STRINGDATA* pRight )
    SAL_THROW_EXTERN_C()
{
    IMPL_RTL_STRINGDATA* pOrg = *ppThis;

    /* Test for 0-Pointer - if not, change newReplaceStrAt! */
    if ( !pRight || !pRight->length )
    {
        *ppThis = pLeft;
        IMPL_RTL_AQUIRE( pLeft );
    }
    else if ( !pLeft || !pLeft->length )
    {
        *ppThis = pRight;
        IMPL_RTL_AQUIRE( pRight );
    }
    else
    {
        IMPL_RTL_STRINGDATA* pTempStr = IMPL_RTL_STRINGNAME( ImplAlloc )( pLeft->length + pRight->length );
        OSL_ASSERT(pTempStr != NULL);
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

void SAL_CALL IMPL_RTL_STRINGNAME( newReplaceStrAt )( IMPL_RTL_STRINGDATA** ppThis,
                                                      IMPL_RTL_STRINGDATA* pStr,
                                                      sal_Int32 nIndex,
                                                      sal_Int32 nCount,
                                                      IMPL_RTL_STRINGDATA* pNewSubStr )
    SAL_THROW_EXTERN_C()
{
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
    OSL_ASSERT(*ppThis != NULL);
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
        IMPL_RTL_AQUIRE( pStr );
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
    IMPL_RTL_STRINGDATA*    pOrg        = *ppThis;
    int                     bChanged    = 0;
    sal_Int32               nLen        = pStr->length;
    const IMPL_RTL_STRCODE* pCharStr    = pStr->buffer;

    while ( nLen > 0 )
    {
        /* Between A-Z (65-90), than to lowercase (+32) */
        if ( (*pCharStr >= 65) && (*pCharStr <= 90) )
        {
            /* Copy String */
            IMPL_RTL_STRCODE* pNewCharStr = IMPL_RTL_STRINGNAME( ImplNewCopy )( ppThis, pStr, pCharStr-pStr->buffer );

            /* replace/copy rest of the string */
            if ( pNewCharStr )
            {
                /* to lowercase (+32) */
                *pNewCharStr = *pCharStr+32;
                pNewCharStr++;
                pCharStr++;
                nLen--;

                while ( nLen > 0 )
                {
                    /* Between A-Z (65-90), than to lowercase (+32) */
                    if ( (*pCharStr >= 65) && (*pCharStr <= 90) )
                        *pNewCharStr = *pCharStr+32;
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
        IMPL_RTL_AQUIRE( pStr );
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
    IMPL_RTL_STRINGDATA*    pOrg        = *ppThis;
    int                     bChanged    = 0;
    sal_Int32               nLen        = pStr->length;
    const IMPL_RTL_STRCODE* pCharStr    = pStr->buffer;

    while ( nLen > 0 )
    {
        /* Between a-z (97-122), than to uppercase (-32) */
        if ( (*pCharStr >= 97) && (*pCharStr <= 122) )
        {
            /* Copy String */
            IMPL_RTL_STRCODE* pNewCharStr = IMPL_RTL_STRINGNAME( ImplNewCopy )( ppThis, pStr, pCharStr-pStr->buffer );

            /* replace/copy rest of the string */
            if ( pNewCharStr )
            {
                /* to uppercase (-32) */
                *pNewCharStr = *pCharStr-32;
                pNewCharStr++;
                pCharStr++;
                nLen--;

                while ( nLen > 0 )
                {
                    /* Between a-z (97-122), than to uppercase (-32) */
                    if ( (*pCharStr >= 97) && (*pCharStr <= 122) )
                        *pNewCharStr = *pCharStr-32;
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
        IMPL_RTL_AQUIRE( pStr );
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
        IMPL_RTL_AQUIRE( pStr );
    }
    else
    {
        nLen -= nPostSpaces+nPreSpaces;
        *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen );
        OSL_ASSERT(*ppThis != NULL);
        if ( *ppThis )
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
