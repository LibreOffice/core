/*************************************************************************
 *
 *  $RCSfile: strtmpl.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: th $ $Date: 2001-03-16 16:38:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* ======================================================================= */
/* Internal C-String help functions which could be used without the        */
/* String-Class                                                            */
/* ======================================================================= */

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
{
    const IMPL_RTL_STRCODE* pTempStr = pStr;
    while( *pTempStr )
        pTempStr++;
    return pTempStr-pStr;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compare )( const IMPL_RTL_STRCODE* pStr1,
                                                const IMPL_RTL_STRCODE* pStr2 )
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
{
    const IMPL_RTL_STRCODE* pStr1End = pStr1 + nStr1Len;
    const IMPL_RTL_STRCODE* pStr2End = pStr2 + nStr2Len;
    sal_Int32 nRet = 0;
    while( (pStr1 < pStr1End) &&
           (pStr2 < pStr2End) &&
            ((nRet = ((sal_Int32)(IMPL_RTL_USTRCODE( *pStr1 )))-
                     ((sal_Int32)(IMPL_RTL_USTRCODE( *pStr2 )))) == 0) )
    {
        pStr1++;
        pStr2++;
    }

    if ( nRet )
        return nRet;
    return nStr1Len - nStr2Len;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( hashCode )( const IMPL_RTL_STRCODE* pStr )
{
    return IMPL_RTL_STRNAME( hashCode_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ) );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( hashCode_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                            sal_Int32 nLen )
{
    sal_Int32 h = nLen;

    if ( nLen < 16 )
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
{
    return IMPL_RTL_STRNAME( lastIndexOfChar_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ), c );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( lastIndexOfChar_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                                   sal_Int32 nLen,
                                                                   IMPL_RTL_STRCODE c )
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
{
    return IMPL_RTL_STRNAME( indexOfStr_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ),
                                                      pSubStr, IMPL_RTL_STRNAME( getLength )( pSubStr ) );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( indexOfStr_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                              sal_Int32 nStrLen,
                                                              const  IMPL_RTL_STRCODE* pSubStr,
                                                              sal_Int32 nSubLen )
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
{
    return IMPL_RTL_STRNAME( lastIndexOfStr_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ),
                                                          pSubStr, IMPL_RTL_STRNAME( getLength )( pSubStr ) );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( lastIndexOfStr_WithLength )( const IMPL_RTL_STRCODE* pStr,
                                                                  sal_Int32 nStrLen,
                                                                  const IMPL_RTL_STRCODE* pSubStr,
                                                                  sal_Int32 nSubLen )
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
        while ( nStrLen >= nSubLen )
        {
            nStrLen--;
            pStr--;

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
            }
        }
    }

    return -1;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRNAME( replaceChar )( IMPL_RTL_STRCODE* pStr,
                                               IMPL_RTL_STRCODE cOld,
                                               IMPL_RTL_STRCODE cNew )
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

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( valueOfBoolean )( IMPL_RTL_STRCODE* pStr, sal_Bool b )
{
    if ( b )
    {
        *pStr = 'T';
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
        *pStr = 'F';
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
{
    *pStr = c;
    *pStr = 0;
    return 1;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( valueOfInt32 )( IMPL_RTL_STRCODE* pStr,
                                                     sal_Int32 n,
                                                     sal_Int16 nRadix )
{
    const sal_Char*     pDigits = rtl_ImplGetDigits();
    IMPL_RTL_STRCODE    aBuf[RTL_STR_MAX_VALUEOFINT32];
    IMPL_RTL_STRCODE*   pBuf = aBuf;
    sal_Int32           nLen = 1;

    /* Radix must be valid */
    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    /* is value negativ */
    if ( n < 0 )
    {
        *pStr = '-';
        pStr++;
        nLen++;
        n = -n;
    }

    /* create a recursive buffer with all values, except the last one */
    while ( n > nRadix )
    {
        *pBuf = pDigits[n % nRadix];
        pBuf++;
        n /= nRadix;
    }

    /* add last digit and the terminating NULL-character */
    *pStr = pDigits[n];
    pStr++;

    /* copy the values in the right direction into the destination buffer */
    while ( pBuf != aBuf )
    {
        pBuf--;
        *pStr = *pBuf;
        pStr++;
        nLen++;
    }
    *pStr = 0;

    return nLen;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( valueOfInt64 )( IMPL_RTL_STRCODE* pStr,
                                                     sal_Int64 n,
                                                     sal_Int16 nRadix )
{
    const sal_Char*     pDigits = rtl_ImplGetDigits();
    IMPL_RTL_STRCODE    aBuf[RTL_STR_MAX_VALUEOFINT64];
    IMPL_RTL_STRCODE*   pBuf = aBuf;
    sal_Int32           nLen = 1;

    /* Radix must be valid */
    if ( (nRadix < RTL_STR_MIN_RADIX) || (nRadix > RTL_STR_MAX_RADIX) )
        nRadix = 10;

    /* is value negativ */
    if ( n < 0 )
    {
        *pStr = '-';
        pStr++;
        nLen++;
        n = -n;
    }

    /* create a recursive buffer with all values, except the last one */
    while ( n > nRadix )
    {
        *pBuf = pDigits[n % nRadix];
        pBuf++;
        n /= nRadix;
    }

    /* add last digit and the terminating NULL-character */
    *pStr = pDigits[n];
    pStr++;

    /* copy the values in the right direction into the destination buffer */
    while ( pBuf != aBuf )
    {
        pBuf--;
        *pStr = *pBuf;
        pStr++;
        nLen++;
    }
    *pStr = 0;

    return nLen;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( valueOfFloat )( IMPL_RTL_STRCODE* pStr,
                                                     float f )
{
    sal_Char    aBuf[RTL_STR_MAX_VALUEOFDOUBLE];
    sal_Char*   pBuf = aBuf;
    sal_Int32   nLen;
    sal_Int32   nTempLen;

    nLen = rtl_ImplFloatToString( pBuf, f );

    /* Copy Buffer */
    nTempLen = nLen;
    while ( nTempLen )
    {
        *pStr = *pBuf;
        pStr++;
        pBuf++;
        nTempLen--;

    }
    *pStr = 0;

    return nLen;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( valueOfDouble )( IMPL_RTL_STRCODE* pStr,
                                                      double d )
{
    sal_Char    aBuf[RTL_STR_MAX_VALUEOFDOUBLE];
    sal_Char*   pBuf = aBuf;
    sal_Int32   nLen;
    sal_Int32   nTempLen;

    nLen = rtl_ImplDoubleToString( pBuf, d );

    /* Copy Buffer */
    nTempLen = nLen;
    while ( nTempLen )
    {
        *pStr = *pBuf;
        pStr++;
        pBuf++;
        nTempLen--;

    }
    *pStr = 0;

    return nLen;
}

/* ----------------------------------------------------------------------- */

sal_Bool SAL_CALL IMPL_RTL_STRNAME( toBoolean )( const IMPL_RTL_STRCODE* pStr )
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

#define INT32_MIN_VALUE     ((sal_Int32)0x80000000);
#define INT32_MAX_VALUE     ((sal_Int32)0x7fffffff);

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( toInt32 )( const IMPL_RTL_STRCODE* pStr,
                                                sal_Int16 nRadix )
{
    sal_Bool    bNeg;
    sal_Int32   nLimit;
    sal_Int32   nMultMin;
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
        nLimit = INT32_MIN_VALUE;
        pStr++;
    }
    else
    {
        bNeg = sal_False;
        nLimit = -INT32_MAX_VALUE;
    }
    nMultMin = nLimit / nRadix;

    while ( *pStr )
    {
        nDigit = rtl_ImplGetDigit( IMPL_RTL_USTRCODE( *pStr ), nRadix );
        if ( nDigit < 0 )
            break;

        if ( n < nMultMin )
            break;

        n *= nRadix;
        if ( n < (nLimit+nDigit) )
            break;
        n -= nDigit;

        pStr++;
    }

    if ( bNeg )
        return n;
    else
        return -n;
}

/* ----------------------------------------------------------------------- */

#ifndef WNT
#define INT64_MIN_VALUE     0x8000000000000000LL;
#define INT64_MAX_VALUE     0x7fffffffffffffffLL;
#else
#define INT64_MIN_VALUE     0x8000000000000000L;
#define INT64_MAX_VALUE     0x7fffffffffffffffL;
#endif

sal_Int64 SAL_CALL IMPL_RTL_STRNAME( toInt64 )( const IMPL_RTL_STRCODE* pStr,
                                                sal_Int16 nRadix )
{
    sal_Bool    bNeg;
    sal_Int64   nLimit;
    sal_Int64   nMultMin;
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
        nLimit = INT64_MIN_VALUE;
        pStr++;
    }
    else
    {
        bNeg = sal_False;
        nLimit = -INT64_MAX_VALUE;
    }
    nMultMin = nLimit / nRadix;

    while ( *pStr )
    {
        nDigit = rtl_ImplGetDigit( IMPL_RTL_USTRCODE( *pStr ), nRadix );
        if ( nDigit < 0 )
            break;

        if ( n < nMultMin )
            break;

        n *= nRadix;
        if ( n < (nLimit+nDigit) )
            break;
        n -= nDigit;

        pStr++;
    }

    if ( bNeg )
        return n;
    else
        return -n;
}

/* ----------------------------------------------------------------------- */

static double IMPL_RTL_STRNAME( ImplStringToDouble )( const IMPL_RTL_STRCODE* pStr )
{
    sal_Bool    bExp        = sal_False;
    sal_Bool    bNeg        = sal_False;
    sal_Bool    bNegExp     = sal_False;
    sal_Bool    bDec        = sal_False;
    int         nExp        = 0;
    double      fRet        = 0.0;
    double      fDiv        = 0.1;

    /* Skip whitespaces and leading zeros */
    while ( *pStr &&
            rtl_ImplIsWhitespace( IMPL_RTL_USTRCODE( *pStr ) ) &&
            (*pStr == 0) )
        pStr++;

    while ( *pStr )
    {
        /* Digit */
        if ( (*pStr >= '0') && (*pStr <= '9') )
        {
            if ( bExp )
                nExp = nExp*10 + (*pStr-'0');
            else if ( !bDec )
                fRet = 10.0 * fRet + (double)(*pStr-'0');
            else
            {
                fRet += fDiv * (double)(*pStr-'0');
                fDiv /= 10.0;
            }
        }
        /* Decimal separator */
        else if ( *pStr == '.' )
        {
            if ( bExp || bDec )
                break;
            bDec = sal_True;
        }
        else if ( *pStr == '-' )
        {
            if ( bExp )
            {
                if ( nExp != 0 )
                    break;
                bNegExp = !bNegExp;
            }
            else
            {
                if ( fRet != 0.0 )
                    break;
                bNeg = !bNeg;
            }
        }
        else if ( *pStr == '+' )
        {
            if ( bExp && (nExp != 0) )
                break;
            if ( fRet != 0.0 )
                break;
        }
        /* Exponent separator */
        else if ( (*pStr == 'e') || (*pStr == 'E') )
        {
            if ( bExp )
                break;
            bExp = sal_True;
        }
        else
            break;

        pStr++;
    }

    /* Infinity || NaN not handled !!! ??? */

    if ( bExp )
    {
        if ( bNegExp )
            nExp = -nExp;
        fRet *= rtl_ImplCalcPow10( nExp );
    }

    if ( bNeg )
        return -fRet;
    else
        return fRet;
}

/* ----------------------------------------------------------------------- */

float SAL_CALL IMPL_RTL_STRNAME( toFloat )( const IMPL_RTL_STRCODE* pStr )
{
    double d = IMPL_RTL_STRNAME( ImplStringToDouble )( pStr );
    return (float)d;
}

/* ----------------------------------------------------------------------- */

double SAL_CALL IMPL_RTL_STRNAME( toDouble )( const IMPL_RTL_STRCODE* pStr )
{
    return IMPL_RTL_STRNAME( ImplStringToDouble )( pStr );
}

/* ======================================================================= */
/* Internal String-Class help functions                                    */
/* ======================================================================= */

static IMPL_RTL_STRINGDATA* IMPL_RTL_STRINGNAME( ImplAlloc )( sal_Int32 nLen )
{
    IMPL_RTL_STRINGDATA* pData = (IMPL_RTL_STRINGDATA*)rtl_allocateMemory( sizeof( IMPL_RTL_STRINGDATA ) + (nLen*sizeof( IMPL_RTL_STRCODE )) );
    pData->refCount     = 1;
    pData->length       = nLen;
    pData->buffer[nLen] = 0;
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
    return pDest;
}

/* ======================================================================= */
/* String-Class functions                                                  */
/* ======================================================================= */

#define IMPL_RTL_AQUIRE( pThis )                                        \
{                                                                       \
    osl_incrementInterlockedCount( &((pThis)->refCount) );              \
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( acquire )( IMPL_RTL_STRINGDATA* pThis )
{
    IMPL_RTL_AQUIRE( pThis );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( release )( IMPL_RTL_STRINGDATA* pThis )
{
    if ( pThis->refCount == 1 )
    {
        OSL_ENSURE( pThis != &IMPL_RTL_EMPTYSTRING, "static empty string: refCount < 1" );
        rtl_freeMemory( pThis );
    }
    else if ( !osl_decrementInterlockedCount( &(pThis->refCount) ) )
    {
        OSL_ENSURE( IMPL_RTL_EMPTYSTRING.refCount >= 1, "static empty string: refCount < 1" );
        rtl_freeMemory( pThis );
    }
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( new )( IMPL_RTL_STRINGDATA** ppThis )
{
    if ( *ppThis)
        IMPL_RTL_STRINGNAME( release )( *ppThis );

    *ppThis = &IMPL_RTL_EMPTYSTRING;
    IMPL_RTL_AQUIRE( *ppThis );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( new_WithLength )( IMPL_RTL_STRINGDATA** ppThis, sal_Int32 nLen )
{
    if ( nLen <= 0 )
        IMPL_RTL_STRINGNAME( new )( ppThis );
    else
    {
        if ( *ppThis)
            IMPL_RTL_STRINGNAME( release )( *ppThis );

        *ppThis = (IMPL_RTL_STRINGDATA*)rtl_allocateMemory( sizeof( IMPL_RTL_STRINGDATA ) + (nLen*sizeof( IMPL_RTL_STRCODE )) );
        (*ppThis)->refCount = 1;
        (*ppThis)->length   = 0;

        {
        IMPL_RTL_STRCODE* pTempStr = (*ppThis)->buffer;
        while ( nLen >= 0 )
        {
            *pTempStr = 0;
            pTempStr++;
            nLen--;
        }
        }
    }
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newFromString )( IMPL_RTL_STRINGDATA** ppThis,
                                                    const IMPL_RTL_STRINGDATA* pStr )
{
    IMPL_RTL_STRINGDATA* pOrg;

    if ( !pStr->length )
    {
        IMPL_RTL_STRINGNAME( new )( ppThis );
        return;
    }

    pOrg = *ppThis;
    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( pStr->length );
    rtl_str_ImplCopy( (*ppThis)->buffer, pStr->buffer, pStr->length );

    /* must be done at least, if pStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newFromStr )( IMPL_RTL_STRINGDATA** ppThis,
                                                 const IMPL_RTL_STRCODE* pCharStr )
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
    pBuffer = (*ppThis)->buffer;
    do
    {
        *pBuffer = *pCharStr;
        pBuffer++;
        pCharStr++;
    }
    while ( *pCharStr );

    /* must be done at least, if pCharStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newFromStr_WithLength )( IMPL_RTL_STRINGDATA** ppThis,
                                                            const IMPL_RTL_STRCODE* pCharStr,
                                                            sal_Int32 nLen )
{
    IMPL_RTL_STRINGDATA* pOrg;

    if ( !pCharStr || (nLen <= 0) )
    {
        IMPL_RTL_STRINGNAME( new )( ppThis );
        return;
    }

    pOrg = *ppThis;
    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nLen );
    rtl_str_ImplCopy( (*ppThis)->buffer, pCharStr, nLen );

    /* must be done at least, if pCharStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( assign )( IMPL_RTL_STRINGDATA** ppThis,
                                             IMPL_RTL_STRINGDATA* pStr )
{
    /* must be done at first, if pStr == *ppThis */
    IMPL_RTL_AQUIRE( pStr );

    if ( *ppThis )
        IMPL_RTL_STRINGNAME( release )( *ppThis );

    *ppThis = pStr;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRINGNAME( getLength )( const IMPL_RTL_STRINGDATA* pThis )
{
    return pThis->length;
}

/* ----------------------------------------------------------------------- */

IMPL_RTL_STRCODE* SAL_CALL IMPL_RTL_STRINGNAME( getStr )( IMPL_RTL_STRINGDATA* pThis )
{
    return pThis->buffer;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newConcat )( IMPL_RTL_STRINGDATA** ppThis,
                                                IMPL_RTL_STRINGDATA* pLeft,
                                                IMPL_RTL_STRINGDATA* pRight )
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
        rtl_str_ImplCopy( pTempStr->buffer, pLeft->buffer, pLeft->length );
        rtl_str_ImplCopy( pTempStr->buffer+pLeft->length, pRight->buffer, pRight->length );
        *ppThis = pTempStr;
    }

    /* must be done at least, if left or right == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newReplaceStrAt )( IMPL_RTL_STRINGDATA** ppThis,
                                                      IMPL_RTL_STRINGDATA* pStr,
                                                      sal_Int32 nIndex,
                                                      sal_Int32 nCount,
                                                      IMPL_RTL_STRINGDATA* pNewSubStr )
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

    {
    IMPL_RTL_STRINGDATA*    pOrg = *ppThis;
    IMPL_RTL_STRCODE*       pBuffer;
    sal_Int32               nNewLen;

    /* Calculate length of the new string */
    nNewLen = pStr->length-nCount;
    if ( pNewSubStr )
        nNewLen += pNewSubStr->length;

    /* Alloc New Buffer */
    *ppThis = IMPL_RTL_STRINGNAME( ImplAlloc )( nNewLen );
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

    /* must be done at least, if pStr or pNewSubStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
    }
}

/* ======================================================================= */
/* ======================================================================= */

#if 0

/* This file is included from string.c and ustring.c and shares the
   template code between both implementations */

/* ======================================================================= */
/* C-String functions which could be used without the String-Class         */
/* ======================================================================= */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( getLengthMax )( const IMPL_RTL_STRCODE* pStr,
                                                     sal_Int32 nMaxCount )
{
    sal_Int32 nMaxLen = 0;
    while ( (nMaxLen < nMaxCount) && *pStr )
    {
        pStr++;
        nMaxLen++;
    }
    return nMaxLen;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compareMax )( const IMPL_RTL_STRCODE* pStr1,
                                                   const IMPL_RTL_STRCODE* pStr2,
                                                   sal_Int32 nCount )
{
    sal_Int32 nRet = 0;
    while ( nCount &&
            ((nRet = ((sal_Int32)(IMPL_RTL_USTRCODE( *pStr1 )))-
                     ((sal_Int32)(IMPL_RTL_USTRCODE( *pStr2 )))) == 0) &&
            *pStr2 )
    {
        pStr1++;
        pStr2++;
        nCount--;
    }

    return nRet;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compareIgnoreAsciiCase )( const IMPL_RTL_STRCODE* pStr1,
                                                               const IMPL_RTL_STRCODE* pStr2 )
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
            break;

        pStr1++;
        pStr2++;
    }
    while ( c2 );

    return nRet;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compareIgnoreAsciiCaseMax )( const IMPL_RTL_STRCODE* pStr1,
                                                                  const IMPL_RTL_STRCODE* pStr2,
                                                                  sal_Int32 nCount )
{
    sal_Int32   nRet = 0;
    sal_Int32   c1;
    sal_Int32   c2;
    do
    {
        if ( !nCount )
            break;

        /* If character between 'A' and 'Z', than convert it to lowercase */
        c1 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr1 );
        c2 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr2 );
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = c1-c2;
        if ( nRet != 0 )
            break;

        pStr1++;
        pStr2++;
        nCount--;
    }
    while ( c2 );

    return nRet;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( compareIgnoreAsciiCase_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                                          sal_Int32 nStr1Len,
                                                                          const IMPL_RTL_STRCODE* pStr2,
                                                                          sal_Int32 nStr2Len )
{
    const IMPL_RTL_STRCODE* pStr1End = pStr1 + nStr1Len;
    const IMPL_RTL_STRCODE* pStr2End = pStr2 + nStr2Len;
    sal_Int32   nRet = 0;
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
            break;

        pStr1++;
        pStr2++;
    }

    if ( nRet )
        return nRet;
    return nStr1Len - nStr2Len;
}

/* ----------------------------------------------------------------------- */

sal_Bool SAL_CALL IMPL_RTL_STRNAME( equals )( const IMPL_RTL_STRCODE* pStr1,
                                              const IMPL_RTL_STRCODE* pStr2 )
{
    return IMPL_RTL_STRNAME( compare )( pStr1, pStr2 ) == 0;
}

/* ----------------------------------------------------------------------- */

sal_Bool SAL_CALL IMPL_RTL_STRNAME( equals_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                         sal_Int32 nStr1Len,
                                                         const IMPL_RTL_STRCODE* pStr2,
                                                         sal_Int32 nStr2Len )
{
    if ( nStr1Len != nStr2Len )
        return sal_False;

    return IMPL_RTL_STRNAME( compare_WithLength )( pStr1, nStr1Len, pStr2, nStr2Len ) == 0;
}

/* ----------------------------------------------------------------------- */

sal_Bool SAL_CALL IMPL_RTL_STRNAME( equalsIgnoreAsciiCase )( const IMPL_RTL_STRCODE* pStr1,
                                                             const IMPL_RTL_STRCODE* pStr2 )
{
    return IMPL_RTL_STRNAME( compareIgnoreAsciiCase )( pStr1, pStr2 ) == 0;
}

/* ----------------------------------------------------------------------- */

sal_Bool SAL_CALL IMPL_RTL_STRNAME( equalsIgnoreAsciiCase_WithLength )( const IMPL_RTL_STRCODE* pStr1,
                                                                        sal_Int32 nStr1Len,
                                                                        const IMPL_RTL_STRCODE* pStr2,
                                                                        sal_Int32 nStr2Len )
{
    if ( nStr1Len != nStr2Len )
        return sal_False;

    return IMPL_RTL_STRNAME( compareIgnoreAsciiCase_WithLength )( pStr1, nStr1Len, pStr2, nStr2Len ) == 0;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( indexOfCharMax )( const IMPL_RTL_STRCODE* pStr,
                                                       IMPL_RTL_STRCODE c,
                                                       sal_Int32 nCount )
{
    const IMPL_RTL_STRCODE* pTempStr = pStr;
    while ( nCount && *pTempStr )
    {
        if ( *pTempStr == c )
            return pTempStr-pStr;

        pTempStr++;
        nCount--;
    }

    return -1;
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( lastIndexOfCharMax )( const IMPL_RTL_STRCODE* pStr,
                                                           IMPL_RTL_STRCODE c,
                                                           sal_Int32 nCount )
{
    return IMPL_RTL_STRNAME( lastIndexOfChar_WithLength )( pStr, IMPL_RTL_STRNAME( getLengthMax )( pStr, nCount ), c );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( indexOfStrMax )( const IMPL_RTL_STRCODE* pStr,
                                                      const IMPL_RTL_STRCODE* pSubStr,
                                                      sal_Int32 nMaxLen )
{
    return IMPL_RTL_STRNAME( indexOfStr_WithLength )( pStr, IMPL_RTL_STRNAME( getLengthMax )( pStr, nMaxLen ),
                                                      pSubStr, IMPL_RTL_STRNAME( getLengthMax )( pSubStr, nMaxLen ) );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( lastIndexOfStrMax )( const IMPL_RTL_STRCODE* pStr,
                                                          const IMPL_RTL_STRCODE* pSubStr,
                                                          sal_Int32 nMaxLen )
{
    return IMPL_RTL_STRNAME( lastIndexOfStr_WithLength )( pStr, IMPL_RTL_STRNAME( getLengthMax )( pStr, nMaxLen ),
                                                          pSubStr, IMPL_RTL_STRNAME( getLengthMax )( pSubStr, nMaxLen ) );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRNAME( toAsciiLowerCase )( IMPL_RTL_STRCODE* pStr )
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
{
    return IMPL_RTL_STRNAME( trim_WithLength )( pStr, IMPL_RTL_STRNAME( getLength )( pStr ) );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRNAME( trim_WithLength )( IMPL_RTL_STRCODE* pStr, sal_Int32 nLen )
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

/* ======================================================================= */
/* String-Class functions                                                  */
/* ======================================================================= */

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newReplace )( IMPL_RTL_STRINGDATA** ppThis,
                                                 IMPL_RTL_STRINGDATA* pStr,
                                                 IMPL_RTL_STRCODE cOld,
                                                 IMPL_RTL_STRCODE cNew )
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

    /* must be done at least, if pStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newToAsciiLowerCase )( IMPL_RTL_STRINGDATA** ppThis,
                                                          IMPL_RTL_STRINGDATA* pStr )
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

    /* must be done at least, if pStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newToAsciiUpperCase )( IMPL_RTL_STRINGDATA** ppThis,
                                                          IMPL_RTL_STRINGDATA* pStr )
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

    /* must be done at least, if pStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( newTrim )( IMPL_RTL_STRINGDATA** ppThis,
                                              IMPL_RTL_STRINGDATA* pStr )
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
        if ( *ppThis )
            rtl_str_ImplCopy( (*ppThis)->buffer, pStr->buffer+nPreSpaces, nLen );
    }

    /* must be done at least, if pStr == *ppThis */
    if ( pOrg )
        IMPL_RTL_STRINGNAME( release )( pOrg );
}

/* ----------------------------------------------------------------------- */

sal_Int32 SAL_CALL IMPL_RTL_STRINGNAME( getTokenCount )( const IMPL_RTL_STRINGDATA* pThis,
                                                         IMPL_RTL_STRCODE cTok )
{
    sal_Int32               nTokCount   = 1;
    sal_Int32               nLen        = pThis->length;
    const IMPL_RTL_STRCODE* pCharStr    = pThis->buffer;
    while ( nLen > 0 )
    {
        if ( *pCharStr == cTok )
            nTokCount++;
        pCharStr++;
        nLen--;
    }

    return nTokCount;
}

/* ----------------------------------------------------------------------- */

void SAL_CALL IMPL_RTL_STRINGNAME( getToken )( IMPL_RTL_STRINGDATA** ppThis,
                                               IMPL_RTL_STRINGDATA* pStr,
                                               sal_Int32 nToken,
                                               IMPL_RTL_STRCODE cTok )
{
    IMPL_RTL_STRINGDATA*    pOrg            = *ppThis;
    const IMPL_RTL_STRCODE* pCharStr        = pStr->buffer;
    const IMPL_RTL_STRCODE* pCharStrStart   = pCharStr;
    sal_Int32               nTokCount       = 0;
    sal_Int32               nFirstChar      = 0;
    sal_Int32               nLen            = pStr->length;

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
        IMPL_RTL_STRINGNAME( new )( ppThis );
    else
        IMPL_RTL_STRINGNAME( newFromStr_WithLength )( ppThis, pCharStrStart, pCharStr-pCharStrStart );
}

#endif
