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


#include "rtl/math.h"

#include "osl/diagnose.h"
#include "rtl/alloc.h"
#include "rtl/math.hxx"
#include "rtl/strbuf.h"
#include "rtl/string.h"
#include "rtl/ustrbuf.h"
#include "rtl/ustring.h"
#include "sal/mathconf.h"
#include "sal/types.h"

#include <algorithm>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>


static int const n10Count = 16;
static double const n10s[2][n10Count] = {
    { 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8,
      1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16 },
    { 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8,
      1e-9, 1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16 }
};

// return pow(10.0,nExp) optimized for exponents in the interval [-16,16]
static double getN10Exp( int nExp )
{
    if ( nExp < 0 )
    {
        // && -nExp > 0 necessary for std::numeric_limits<int>::min()
        // because -nExp = nExp
        if ( -nExp <= n10Count && -nExp > 0 )
            return n10s[1][-nExp-1];
        else
            return pow( 10.0, static_cast<double>( nExp ) );
    }
    else if ( nExp > 0 )
    {
        if ( nExp <= n10Count )
            return n10s[0][nExp-1];
        else
            return pow( 10.0, static_cast<double>( nExp ) );
    }
    else // ( nExp == 0 )
        return 1.0;
}

/** Approximation algorithm for erf for 0 < x < 0.65. */
static void lcl_Erf0065( double x, double& fVal )
{
    static const double pn[] = {
        1.12837916709551256,
        1.35894887627277916E-1,
        4.03259488531795274E-2,
        1.20339380863079457E-3,
        6.49254556481904354E-5
    };
    static const double qn[] = {
        1.00000000000000000,
        4.53767041780002545E-1,
        8.69936222615385890E-2,
        8.49717371168693357E-3,
        3.64915280629351082E-4
    };
    double fPSum = 0.0;
    double fQSum = 0.0;
    double fXPow = 1.0;
    for ( unsigned int i = 0; i <= 4; ++i )
    {
        fPSum += pn[i]*fXPow;
        fQSum += qn[i]*fXPow;
        fXPow *= x*x;
    }
    fVal = x * fPSum / fQSum;
}

/** Approximation algorithm for erfc for 0.65 < x < 6.0. */
static void lcl_Erfc0600( double x, double& fVal )
{
    double fPSum = 0.0;
    double fQSum = 0.0;
    double fXPow = 1.0;
    const double *pn;
    const double *qn;

    if ( x < 2.2 )
    {
        static const double pn22[] = {
            9.99999992049799098E-1,
            1.33154163936765307,
            8.78115804155881782E-1,
            3.31899559578213215E-1,
            7.14193832506776067E-2,
            7.06940843763253131E-3
        };
        static const double qn22[] = {
            1.00000000000000000,
            2.45992070144245533,
            2.65383972869775752,
            1.61876655543871376,
            5.94651311286481502E-1,
            1.26579413030177940E-1,
            1.25304936549413393E-2
        };
        pn = pn22;
        qn = qn22;
    }
    else /* if ( x < 6.0 )  this is true, but the compiler does not know */
    {
        static const double pn60[] = {
            9.99921140009714409E-1,
            1.62356584489366647,
            1.26739901455873222,
            5.81528574177741135E-1,
            1.57289620742838702E-1,
            2.25716982919217555E-2
        };
        static const double qn60[] = {
            1.00000000000000000,
            2.75143870676376208,
            3.37367334657284535,
            2.38574194785344389,
            1.05074004614827206,
            2.78788439273628983E-1,
            4.00072964526861362E-2
        };
        pn = pn60;
        qn = qn60;
    }

    for ( unsigned int i = 0; i < 6; ++i )
    {
        fPSum += pn[i]*fXPow;
        fQSum += qn[i]*fXPow;
        fXPow *= x;
    }
    fQSum += qn[6]*fXPow;
    fVal = exp( -1.0*x*x )* fPSum / fQSum;
}

/** Approximation algorithm for erfc for 6.0 < x < 26.54 (but used for all
    x > 6.0). */
static void lcl_Erfc2654( double x, double& fVal )
{
    static const double pn[] = {
        5.64189583547756078E-1,
        8.80253746105525775,
        3.84683103716117320E1,
        4.77209965874436377E1,
        8.08040729052301677
    };
    static const double qn[] = {
        1.00000000000000000,
        1.61020914205869003E1,
        7.54843505665954743E1,
        1.12123870801026015E2,
        3.73997570145040850E1
    };

    double fPSum = 0.0;
    double fQSum = 0.0;
    double fXPow = 1.0;

    for ( unsigned int i = 0; i <= 4; ++i )
    {
        fPSum += pn[i]*fXPow;
        fQSum += qn[i]*fXPow;
        fXPow /= x*x;
    }
    fVal = exp(-1.0*x*x)*fPSum / (x*fQSum);
}

namespace {

double const nKorrVal[] = {
    0, 9e-1, 9e-2, 9e-3, 9e-4, 9e-5, 9e-6, 9e-7, 9e-8,
    9e-9, 9e-10, 9e-11, 9e-12, 9e-13, 9e-14, 9e-15
};

struct StringTraits
{
    typedef sal_Char Char;

    typedef rtl_String String;

    static inline void createString(rtl_String ** pString,
                                    sal_Char const * pChars, sal_Int32 nLen)
    {
        rtl_string_newFromStr_WithLength(pString, pChars, nLen);
    }

    static inline void createBuffer(rtl_String ** pBuffer,
                                    sal_Int32 * pCapacity)
    {
        rtl_string_new_WithLength(pBuffer, *pCapacity);
    }

    static inline void appendChar(rtl_String ** pBuffer, sal_Int32 * pCapacity,
                                  sal_Int32 * pOffset, sal_Char cChar)
    {
        rtl_stringbuffer_insert(pBuffer, pCapacity, *pOffset, &cChar, 1);
        ++*pOffset;
    }

    static inline void appendChars(rtl_String ** pBuffer, sal_Int32 * pCapacity,
                                   sal_Int32 * pOffset, sal_Char const * pChars,
                                   sal_Int32 nLen)
    {
        rtl_stringbuffer_insert(pBuffer, pCapacity, *pOffset, pChars, nLen);
        *pOffset += nLen;
    }

    static inline void appendAscii(rtl_String ** pBuffer, sal_Int32 * pCapacity,
                                   sal_Int32 * pOffset, sal_Char const * pStr,
                                   sal_Int32 nLen)
    {
        rtl_stringbuffer_insert(pBuffer, pCapacity, *pOffset, pStr, nLen);
        *pOffset += nLen;
    }
};

struct UStringTraits
{
    typedef sal_Unicode Char;

    typedef rtl_uString String;

    static inline void createString(rtl_uString ** pString,
                                    sal_Unicode const * pChars, sal_Int32 nLen)
    {
        rtl_uString_newFromStr_WithLength(pString, pChars, nLen);
    }

    static inline void createBuffer(rtl_uString ** pBuffer,
                                    sal_Int32 * pCapacity)
    {
        rtl_uString_new_WithLength(pBuffer, *pCapacity);
    }

    static inline void appendChar(rtl_uString ** pBuffer, sal_Int32 * pCapacity,
                                  sal_Int32 * pOffset, sal_Unicode cChar)
    {
        rtl_uStringbuffer_insert(pBuffer, pCapacity, *pOffset, &cChar, 1);
        ++*pOffset;
    }

    static inline void appendChars(rtl_uString ** pBuffer,
                                   sal_Int32 * pCapacity, sal_Int32 * pOffset,
                                   sal_Unicode const * pChars, sal_Int32 nLen)
    {
        rtl_uStringbuffer_insert(pBuffer, pCapacity, *pOffset, pChars, nLen);
        *pOffset += nLen;
    }

    static inline void appendAscii(rtl_uString ** pBuffer,
                                   sal_Int32 * pCapacity, sal_Int32 * pOffset,
                                   sal_Char const * pStr, sal_Int32 nLen)
    {
        rtl_uStringbuffer_insert_ascii(pBuffer, pCapacity, *pOffset, pStr,
                                       nLen);
        *pOffset += nLen;
    }
};


// Solaris C++ 5.2 compiler has problems when "StringT ** pResult" is
// "typename T::String ** pResult" instead:
template< typename T, typename StringT >
inline void doubleToString(StringT ** pResult,
                           sal_Int32 * pResultCapacity, sal_Int32 nResultOffset,
                           double fValue, rtl_math_StringFormat eFormat,
                           sal_Int32 nDecPlaces, typename T::Char cDecSeparator,
                           sal_Int32 const * pGroups,
                           typename T::Char cGroupSeparator,
                           bool bEraseTrailingDecZeros)
{
    static double const nRoundVal[] = {
        5.0e+0, 0.5e+0, 0.5e-1, 0.5e-2, 0.5e-3, 0.5e-4, 0.5e-5, 0.5e-6,
        0.5e-7, 0.5e-8, 0.5e-9, 0.5e-10,0.5e-11,0.5e-12,0.5e-13,0.5e-14
    };

    // sign adjustment, instead of testing for fValue<0.0 this will also fetch
    // -0.0
    bool bSign = rtl::math::isSignBitSet( fValue );
    if( bSign )
        fValue = -fValue;

    if ( rtl::math::isNan( fValue ) )
    {
        // #i112652# XMLSchema-2
        sal_Int32 nCapacity = RTL_CONSTASCII_LENGTH("NaN");
        if (pResultCapacity == 0)
        {
            pResultCapacity = &nCapacity;
            T::createBuffer(pResult, pResultCapacity);
            nResultOffset = 0;
        }
        T::appendAscii(pResult, pResultCapacity, &nResultOffset,
                       RTL_CONSTASCII_STRINGPARAM("NaN"));

        return;
    }

    bool bHuge = fValue == HUGE_VAL; // g++ 3.0.1 requires it this way...
    if ( bHuge || rtl::math::isInf( fValue ) )
    {
        // #i112652# XMLSchema-2
        sal_Int32 nCapacity = RTL_CONSTASCII_LENGTH("-INF");
        if (pResultCapacity == 0)
        {
            pResultCapacity = &nCapacity;
            T::createBuffer(pResult, pResultCapacity);
            nResultOffset = 0;
        }
        if ( bSign )
            T::appendAscii(pResult, pResultCapacity, &nResultOffset,
                           RTL_CONSTASCII_STRINGPARAM("-"));
        T::appendAscii(pResult, pResultCapacity, &nResultOffset,
                       RTL_CONSTASCII_STRINGPARAM("INF"));

        return;
    }

    // find the exponent
    int nExp = 0;
    if ( fValue > 0.0 )
    {
        nExp = static_cast< int >( floor( log10( fValue ) ) );
        fValue /= getN10Exp( nExp );
    }

    switch ( eFormat )
    {
        case rtl_math_StringFormat_Automatic :
        {   // E or F depending on exponent magnitude
            int nPrec;
            if ( nExp <= -15 || nExp >= 15 )        // #58531# was <-16, >16
            {
                nPrec = 14;
                eFormat = rtl_math_StringFormat_E;
            }
            else
            {
                if ( nExp < 14 )
                {
                    nPrec = 15 - nExp - 1;
                    eFormat = rtl_math_StringFormat_F;
                }
                else
                {
                    nPrec = 15;
                    eFormat = rtl_math_StringFormat_F;
                }
            }
            if ( nDecPlaces == rtl_math_DecimalPlaces_Max )
                nDecPlaces = nPrec;
        }
        break;
        case rtl_math_StringFormat_G :
        {   // G-Point, similar to sprintf %G
            if ( nDecPlaces == rtl_math_DecimalPlaces_DefaultSignificance )
                nDecPlaces = 6;
            if ( nExp < -4 || nExp >= nDecPlaces )
            {
                nDecPlaces = std::max< sal_Int32 >( 1, nDecPlaces - 1 );
                eFormat = rtl_math_StringFormat_E;
            }
            else
            {
                nDecPlaces = std::max< sal_Int32 >( 0, nDecPlaces - nExp - 1 );
                eFormat = rtl_math_StringFormat_F;
            }
        }
        break;
        default:
        break;
    }

    sal_Int32 nDigits = nDecPlaces + 1;

    if( eFormat == rtl_math_StringFormat_F )
        nDigits += nExp;

    // Round the number
    if( nDigits >= 0 )
    {
        if( ( fValue += nRoundVal[ nDigits > 15 ? 15 : nDigits ] ) >= 10 )
        {
            fValue = 1.0;
            nExp++;
            if( eFormat == rtl_math_StringFormat_F )
                nDigits++;
        }
    }

    static sal_Int32 const nBufMax = 256;
    typename T::Char aBuf[nBufMax];
    typename T::Char * pBuf;
    sal_Int32 nBuf = static_cast< sal_Int32 >
        ( nDigits <= 0 ? std::max< sal_Int32 >( nDecPlaces, abs(nExp) )
          : nDigits + nDecPlaces ) + 10 + (pGroups ? abs(nDigits) * 2 : 0);
    if ( nBuf > nBufMax )
    {
        pBuf = reinterpret_cast< typename T::Char * >(
            rtl_allocateMemory(nBuf * sizeof (typename T::Char)));
        OSL_ENSURE(pBuf != 0, "Out of memory");
    }
    else
        pBuf = aBuf;
    typename T::Char * p = pBuf;
    if ( bSign )
        *p++ = static_cast< typename T::Char >('-');

    bool bHasDec = false;

    int nDecPos;
    // Check for F format and number < 1
    if( eFormat == rtl_math_StringFormat_F )
    {
        if( nExp < 0 )
        {
            *p++ = static_cast< typename T::Char >('0');
            if ( nDecPlaces > 0 )
            {
                *p++ = cDecSeparator;
                bHasDec = true;
            }
            sal_Int32 i = ( nDigits <= 0 ? nDecPlaces : -nExp - 1 );
            while( (i--) > 0 )
                *p++ = static_cast< typename T::Char >('0');
            nDecPos = 0;
        }
        else
            nDecPos = nExp + 1;
    }
    else
        nDecPos = 1;

    int nGrouping = 0, nGroupSelector = 0, nGroupExceed = 0;
    if ( nDecPos > 1 && pGroups && pGroups[0] && cGroupSeparator )
    {
        while ( nGrouping + pGroups[nGroupSelector] < nDecPos )
        {
            nGrouping += pGroups[ nGroupSelector ];
            if ( pGroups[nGroupSelector+1] )
            {
                if ( nGrouping + pGroups[nGroupSelector+1] >= nDecPos )
                    break;  // while
                ++nGroupSelector;
            }
            else if ( !nGroupExceed )
                nGroupExceed = nGrouping;
        }
    }

    // print the number
    if( nDigits > 0 )
    {
        for ( int i = 0; ; i++ )
        {
            if( i < 15 )
            {
                int nDigit;
                if (nDigits-1 == 0 && i > 0 && i < 14)
                    nDigit = static_cast< int >( floor( fValue
                                                        + nKorrVal[15-i] ) );
                else
                    nDigit = static_cast< int >( fValue + 1E-15 );
                if (nDigit >= 10)
                {   // after-treatment of up-rounding to the next decade
                    sal_Int32 sLen = static_cast< long >(p-pBuf)-1;
                    if (sLen == -1)
                    {
                        p = pBuf;
                        if ( eFormat == rtl_math_StringFormat_F )
                        {
                            *p++ = static_cast< typename T::Char >('1');
                            *p++ = static_cast< typename T::Char >('0');
                        }
                        else
                        {
                            *p++ = static_cast< typename T::Char >('1');
                            *p++ = cDecSeparator;
                            *p++ = static_cast< typename T::Char >('0');
                            nExp++;
                            bHasDec = true;
                        }
                    }
                    else
                    {
                        for (sal_Int32 j = sLen; j >= 0; j--)
                        {
                            typename T::Char cS = pBuf[j];
                            if (cS != cDecSeparator)
                            {
                                if ( cS != static_cast< typename T::Char >('9'))
                                {
                                    pBuf[j] = ++cS;
                                    j = -1;                 // break loop
                                }
                                else
                                {
                                    pBuf[j]
                                        = static_cast< typename T::Char >('0');
                                    if (j == 0)
                                    {
                                        if ( eFormat == rtl_math_StringFormat_F)
                                        {   // insert '1'
                                            typename T::Char * px = p++;
                                            while ( pBuf < px )
                                            {
                                                *px = *(px-1);
                                                px--;
                                            }
                                            pBuf[0] = static_cast<
                                                typename T::Char >('1');
                                        }
                                        else
                                        {
                                            pBuf[j] = static_cast<
                                                typename T::Char >('1');
                                            nExp++;
                                        }
                                    }
                                }
                            }
                        }
                        *p++ = static_cast< typename T::Char >('0');
                    }
                    fValue = 0.0;
                }
                else
                {
                    *p++ = static_cast< typename T::Char >(
                        nDigit + static_cast< typename T::Char >('0') );
                    fValue = ( fValue - nDigit ) * 10.0;
                }
            }
            else
                *p++ = static_cast< typename T::Char >('0');
            if( !--nDigits )
                break;  // for
            if( nDecPos )
            {
                if( !--nDecPos )
                {
                    *p++ = cDecSeparator;
                    bHasDec = true;
                }
                else if ( nDecPos == nGrouping )
                {
                    *p++ = cGroupSeparator;
                    nGrouping -= pGroups[ nGroupSelector ];
                    if ( nGroupSelector && nGrouping < nGroupExceed )
                        --nGroupSelector;
                }
            }
        }
    }

    if ( !bHasDec && eFormat == rtl_math_StringFormat_F )
    {   // nDecPlaces < 0 did round the value
        while ( --nDecPos > 0 )
        {   // fill before decimal point
            if ( nDecPos == nGrouping )
            {
                *p++ = cGroupSeparator;
                nGrouping -= pGroups[ nGroupSelector ];
                if ( nGroupSelector && nGrouping < nGroupExceed )
                    --nGroupSelector;
            }
            *p++ = static_cast< typename T::Char >('0');
        }
    }

    if ( bEraseTrailingDecZeros && bHasDec && p > pBuf )
    {
        while ( *(p-1) == static_cast< typename T::Char >('0') )
            p--;
        if ( *(p-1) == cDecSeparator )
            p--;
    }

    // Print the exponent ('E', followed by '+' or '-', followed by exactly
    // three digits).  The code in rtl_[u]str_valueOf{Float|Double} relies on
    // this format.
    if( eFormat == rtl_math_StringFormat_E )
    {
        if ( p == pBuf )
            *p++ = static_cast< typename T::Char >('1');
                // maybe no nDigits if nDecPlaces < 0
        *p++ = static_cast< typename T::Char >('E');
        if( nExp < 0 )
        {
            nExp = -nExp;
            *p++ = static_cast< typename T::Char >('-');
        }
        else
            *p++ = static_cast< typename T::Char >('+');
//      if (nExp >= 100 )
        *p++ = static_cast< typename T::Char >(
            nExp / 100 + static_cast< typename T::Char >('0') );
        nExp %= 100;
        *p++ = static_cast< typename T::Char >(
            nExp / 10 + static_cast< typename T::Char >('0') );
        *p++ = static_cast< typename T::Char >(
            nExp % 10 + static_cast< typename T::Char >('0') );
    }

    if (pResultCapacity == 0)
        T::createString(pResult, pBuf, p - pBuf);
    else
        T::appendChars(pResult, pResultCapacity, &nResultOffset, pBuf,
                       p - pBuf);

    if ( pBuf != &aBuf[0] )
        rtl_freeMemory(pBuf);
}

}

void SAL_CALL rtl_math_doubleToString(rtl_String ** pResult,
                                      sal_Int32 * pResultCapacity,
                                      sal_Int32 nResultOffset, double fValue,
                                      rtl_math_StringFormat eFormat,
                                      sal_Int32 nDecPlaces,
                                      sal_Char cDecSeparator,
                                      sal_Int32 const * pGroups,
                                      sal_Char cGroupSeparator,
                                      sal_Bool bEraseTrailingDecZeros)
    SAL_THROW_EXTERN_C()
{
    doubleToString< StringTraits, StringTraits::String >(
        pResult, pResultCapacity, nResultOffset, fValue, eFormat, nDecPlaces,
        cDecSeparator, pGroups, cGroupSeparator, bEraseTrailingDecZeros);
}

void SAL_CALL rtl_math_doubleToUString(rtl_uString ** pResult,
                                       sal_Int32 * pResultCapacity,
                                       sal_Int32 nResultOffset, double fValue,
                                       rtl_math_StringFormat eFormat,
                                       sal_Int32 nDecPlaces,
                                       sal_Unicode cDecSeparator,
                                       sal_Int32 const * pGroups,
                                       sal_Unicode cGroupSeparator,
                                       sal_Bool bEraseTrailingDecZeros)
    SAL_THROW_EXTERN_C()
{
    doubleToString< UStringTraits, UStringTraits::String >(
        pResult, pResultCapacity, nResultOffset, fValue, eFormat, nDecPlaces,
        cDecSeparator, pGroups, cGroupSeparator, bEraseTrailingDecZeros);
}


namespace {

// if nExp * 10 + nAdd would result in overflow
inline bool long10Overflow( long& nExp, int nAdd )
{
    if ( nExp > (LONG_MAX/10)
         || (nExp == (LONG_MAX/10) && nAdd > (LONG_MAX%10)) )
    {
        nExp = LONG_MAX;
        return true;
    }
    return false;
}

// We are only concerned about ASCII arabic numerical digits here
template< typename CharT >
inline bool isDigit( CharT c )
{
    return 0x30 <= c && c <= 0x39;
}

template< typename CharT >
inline double stringToDouble(CharT const * pBegin, CharT const * pEnd,
                             CharT cDecSeparator, CharT cGroupSeparator,
                             rtl_math_ConversionStatus * pStatus,
                             CharT const ** pParsedEnd)
{
    double fVal = 0.0;
    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;

    CharT const * p0 = pBegin;
    while (p0 != pEnd && (*p0 == CharT(' ') || *p0 == CharT('\t')))
        ++p0;
    bool bSign;
    if (p0 != pEnd && *p0 == CharT('-'))
    {
        bSign = true;
        ++p0;
    }
    else
    {
        bSign = false;
        if (p0 != pEnd && *p0 == CharT('+'))
            ++p0;
    }
    CharT const * p = p0;
    bool bDone = false;

    // #i112652# XMLSchema-2
    if (3 >= (pEnd - p))
    {
        if ((CharT('N') == p[0]) && (CharT('a') == p[1])
            && (CharT('N') == p[2]))
        {
            p += 3;
            rtl::math::setNan( &fVal );
            bDone = true;
        }
        else if ((CharT('I') == p[0]) && (CharT('N') == p[1])
                 && (CharT('F') == p[2]))
        {
            p += 3;
            fVal = HUGE_VAL;
            eStatus = rtl_math_ConversionStatus_OutOfRange;
            bDone = true;
        }
    }

    if (!bDone) // do not recognize e.g. NaN1.23
    {
        // leading zeros and group separators may be safely ignored
        while (p != pEnd && (*p == CharT('0') || *p == cGroupSeparator))
            ++p;

        long nValExp = 0;       // carry along exponent of mantissa

        // integer part of mantissa
        for (; p != pEnd; ++p)
        {
            CharT c = *p;
            if (isDigit(c))
            {
                fVal = fVal * 10.0 + static_cast< double >( c - CharT('0') );
                ++nValExp;
            }
            else if (c != cGroupSeparator)
                break;
        }

        // fraction part of mantissa
        if (p != pEnd && *p == cDecSeparator)
        {
            ++p;
            double fFrac = 0.0;
            long nFracExp = 0;
            while (p != pEnd && *p == CharT('0'))
            {
                --nFracExp;
                ++p;
            }
            if ( nValExp == 0 )
                nValExp = nFracExp - 1; // no integer part => fraction exponent
            // one decimal digit needs ld(10) ~= 3.32 bits
            static const int nSigs = (DBL_MANT_DIG / 3) + 1;
            int nDigs = 0;
            for (; p != pEnd; ++p)
            {
                CharT c = *p;
                if (!isDigit(c))
                    break;
                if ( nDigs < nSigs )
                {   // further digits (more than nSigs) don't have any
                    // significance
                    fFrac = fFrac * 10.0 + static_cast<double>(c - CharT('0'));
                    --nFracExp;
                    ++nDigs;
                }
            }
            if ( fFrac != 0.0 )
                fVal += rtl::math::pow10Exp( fFrac, nFracExp );
            else if ( nValExp < 0 )
                nValExp = 0;    // no digit other than 0 after decimal point
        }

        if ( nValExp > 0 )
            --nValExp;  // started with offset +1 at the first mantissa digit

        // Exponent
        if (p != p0 && p != pEnd && (*p == CharT('E') || *p == CharT('e')))
        {
            ++p;
            bool bExpSign;
            if (p != pEnd && *p == CharT('-'))
            {
                bExpSign = true;
                ++p;
            }
            else
            {
                bExpSign = false;
                if (p != pEnd && *p == CharT('+'))
                    ++p;
            }
            if ( fVal == 0.0 )
            {   // no matter what follows, zero stays zero, but carry on the
                // offset
                while (p != pEnd && isDigit(*p))
                    ++p;
            }
            else
            {
                bool bOverFlow = false;
                long nExp = 0;
                for (; p != pEnd; ++p)
                {
                    CharT c = *p;
                    if (!isDigit(c))
                        break;
                    int i = c - CharT('0');
                    if ( long10Overflow( nExp, i ) )
                        bOverFlow = true;
                    else
                        nExp = nExp * 10 + i;
                }
                if ( nExp )
                {
                    if ( bExpSign )
                        nExp = -nExp;
                    long nAllExp = ( bOverFlow ? 0 : nExp + nValExp );
                    if ( nAllExp > DBL_MAX_10_EXP || (bOverFlow && !bExpSign) )
                    {   // overflow
                        fVal = HUGE_VAL;
                        eStatus = rtl_math_ConversionStatus_OutOfRange;
                    }
                    else if ((nAllExp < DBL_MIN_10_EXP) ||
                             (bOverFlow && bExpSign) )
                    {   // underflow
                        fVal = 0.0;
                        eStatus = rtl_math_ConversionStatus_OutOfRange;
                    }
                    else if ( nExp > DBL_MAX_10_EXP || nExp < DBL_MIN_10_EXP )
                    {   // compensate exponents
                        fVal = rtl::math::pow10Exp( fVal, -nValExp );
                        fVal = rtl::math::pow10Exp( fVal, nAllExp );
                    }
                    else
                        fVal = rtl::math::pow10Exp( fVal, nExp );  // normal
                }
            }
        }
        else if (p - p0 == 2 && p != pEnd && p[0] == CharT('#')
                 && p[-1] == cDecSeparator && p[-2] == CharT('1'))
        {
            if (pEnd - p >= 4 && p[1] == CharT('I') && p[2] == CharT('N')
                && p[3] == CharT('F'))
            {
                // "1.#INF", "+1.#INF", "-1.#INF"
                p += 4;
                fVal = HUGE_VAL;
                eStatus = rtl_math_ConversionStatus_OutOfRange;
                // Eat any further digits:
                while (p != pEnd && isDigit(*p))
                    ++p;
            }
            else if (pEnd - p >= 4 && p[1] == CharT('N') && p[2] == CharT('A')
                && p[3] == CharT('N'))
            {
                // "1.#NAN", "+1.#NAN", "-1.#NAN"
                p += 4;
                rtl::math::setNan( &fVal );
                if (bSign)
                {
                    union {
                        double sd;
                        sal_math_Double md;
                    } m;
                    m.sd = fVal;
                    m.md.w32_parts.msw |= 0x80000000; // create negative NaN
                    fVal = m.sd;
                    bSign = false; // don't negate again
                }
                // Eat any further digits:
                while (p != pEnd && isDigit(*p))
                    ++p;
            }
        }
    }

    // overflow also if more than DBL_MAX_10_EXP digits without decimal
    // separator, or 0. and more than DBL_MIN_10_EXP digits, ...
    bool bHuge = fVal == HUGE_VAL; // g++ 3.0.1 requires it this way...
    if ( bHuge )
        eStatus = rtl_math_ConversionStatus_OutOfRange;

    if ( bSign )
        fVal = -fVal;

    if (pStatus != 0)
        *pStatus = eStatus;
    if (pParsedEnd != 0)
        *pParsedEnd = p == p0 ? pBegin : p;

    return fVal;
}

}

double SAL_CALL rtl_math_stringToDouble(sal_Char const * pBegin,
                                        sal_Char const * pEnd,
                                        sal_Char cDecSeparator,
                                        sal_Char cGroupSeparator,
                                        rtl_math_ConversionStatus * pStatus,
                                        sal_Char const ** pParsedEnd)
    SAL_THROW_EXTERN_C()
{
    return stringToDouble(pBegin, pEnd, cDecSeparator, cGroupSeparator, pStatus,
                          pParsedEnd);
}

double SAL_CALL rtl_math_uStringToDouble(sal_Unicode const * pBegin,
                                         sal_Unicode const * pEnd,
                                         sal_Unicode cDecSeparator,
                                         sal_Unicode cGroupSeparator,
                                         rtl_math_ConversionStatus * pStatus,
                                         sal_Unicode const ** pParsedEnd)
    SAL_THROW_EXTERN_C()
{
    return stringToDouble(pBegin, pEnd, cDecSeparator, cGroupSeparator, pStatus,
                          pParsedEnd);
}

double SAL_CALL rtl_math_round(double fValue, int nDecPlaces,
                               enum rtl_math_RoundingMode eMode)
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT(nDecPlaces >= -20 && nDecPlaces <= 20);

    if ( fValue == 0.0  )
        return fValue;

    // sign adjustment
    bool bSign = rtl::math::isSignBitSet( fValue );
    if ( bSign )
        fValue = -fValue;

    double fFac = 0;
    if ( nDecPlaces != 0 )
    {
        // max 20 decimals, we don't have unlimited precision
        // #38810# and no overflow on fValue*=fFac
        if ( nDecPlaces < -20 || 20 < nDecPlaces || fValue > (DBL_MAX / 1e20) )
            return bSign ? -fValue : fValue;

        fFac = getN10Exp( nDecPlaces );
        fValue *= fFac;
    }
    //else  //! uninitialized fFac, not needed

    switch ( eMode )
    {
        case rtl_math_RoundingMode_Corrected :
        {
            int nExp;       // exponent for correction
            if ( fValue > 0.0 )
                nExp = static_cast<int>( floor( log10( fValue ) ) );
            else
                nExp = 0;
            int nIndex = 15 - nExp;
            if ( nIndex > 15 )
                nIndex = 15;
            else if ( nIndex <= 1 )
                nIndex = 0;
            fValue = floor( fValue + 0.5 + nKorrVal[nIndex] );
        }
        break;
        case rtl_math_RoundingMode_Down :
            fValue = rtl::math::approxFloor( fValue );
        break;
        case rtl_math_RoundingMode_Up :
            fValue = rtl::math::approxCeil( fValue );
        break;
        case rtl_math_RoundingMode_Floor :
            fValue = bSign ? rtl::math::approxCeil( fValue )
                : rtl::math::approxFloor( fValue );
        break;
        case rtl_math_RoundingMode_Ceiling :
            fValue = bSign ? rtl::math::approxFloor( fValue )
                : rtl::math::approxCeil( fValue );
        break;
        case rtl_math_RoundingMode_HalfDown :
        {
            double f = floor( fValue );
            fValue = ((fValue - f) <= 0.5) ? f : ceil( fValue );
        }
        break;
        case rtl_math_RoundingMode_HalfUp :
        {
            double f = floor( fValue );
            fValue = ((fValue - f) < 0.5) ? f : ceil( fValue );
        }
        break;
        case rtl_math_RoundingMode_HalfEven :
#if defined FLT_ROUNDS
/*
    Use fast version. FLT_ROUNDS may be defined to a function by some compilers!

    DBL_EPSILON is the smallest fractional number which can be represented,
    its reciprocal is therefore the smallest number that cannot have a
    fractional part. Once you add this reciprocal to `x', its fractional part
    is stripped off. Simply subtracting the reciprocal back out returns `x'
    without its fractional component.
    Simple, clever, and elegant - thanks to Ross Cottrell, the original author,
    who placed it into public domain.

    volatile: prevent compiler from being too smart
*/
            if ( FLT_ROUNDS == 1 )
            {
                volatile double x = fValue + 1.0 / DBL_EPSILON;
                fValue = x - 1.0 / DBL_EPSILON;
            }
            else
#endif // FLT_ROUNDS
            {
                double f = floor( fValue );
                if ( (fValue - f) != 0.5 )
                    fValue = floor( fValue + 0.5 );
                else
                {
                    double g = f / 2.0;
                    fValue = (g == floor( g )) ? f : (f + 1.0);
                }
            }
        break;
        default:
            OSL_ASSERT(false);
        break;
    }

    if ( nDecPlaces != 0 )
        fValue /= fFac;

    return bSign ? -fValue : fValue;
}


double SAL_CALL rtl_math_pow10Exp(double fValue, int nExp) SAL_THROW_EXTERN_C()
{
    return fValue * getN10Exp( nExp );
}


double SAL_CALL rtl_math_approxValue( double fValue ) SAL_THROW_EXTERN_C()
{
    if (fValue == 0.0 || fValue == HUGE_VAL || !::rtl::math::isFinite( fValue))
        // We don't handle these conditions.  Bail out.
        return fValue;

    double fOrigValue = fValue;

    bool bSign = ::rtl::math::isSignBitSet( fValue);
    if (bSign)
        fValue = -fValue;

    int nExp = static_cast<int>( floor( log10( fValue)));
    nExp = 14 - nExp;
    double fExpValue = getN10Exp( nExp);

    fValue *= fExpValue;
    // If the original value was near DBL_MIN we got an overflow. Restore and
    // bail out.
    if (!rtl::math::isFinite( fValue))
        return fOrigValue;
    fValue = rtl_math_round( fValue, 0, rtl_math_RoundingMode_Corrected);
    fValue /= fExpValue;
    // If the original value was near DBL_MAX we got an overflow. Restore and
    // bail out.
    if (!rtl::math::isFinite( fValue))
        return fOrigValue;

    return bSign ? -fValue : fValue;
}


double SAL_CALL rtl_math_expm1( double fValue ) SAL_THROW_EXTERN_C()
{
    double fe = exp( fValue );
    if (fe == 1.0)
        return fValue;
    if (fe-1.0 == -1.0)
        return -1.0;
    return (fe-1.0) * fValue / log(fe);
}


double SAL_CALL rtl_math_log1p( double fValue ) SAL_THROW_EXTERN_C()
{
    // Use volatile because a compiler may be too smart "optimizing" the
    // condition such that in certain cases the else path was called even if
    // (fp==1.0) was true, where the term (fp-1.0) then resulted in 0.0 and
    // hence the entire expression resulted in NaN.
    // Happened with g++ 3.4.1 and an input value of 9.87E-18
    volatile double fp = 1.0 + fValue;
    if (fp == 1.0)
        return fValue;
    else
        return log(fp) * fValue / (fp-1.0);
}


double SAL_CALL rtl_math_atanh( double fValue ) SAL_THROW_EXTERN_C()
{
   return 0.5 * rtl_math_log1p( 2.0 * fValue / (1.0-fValue) );
}


/** Parent error function (erf) that calls different algorithms based on the
    value of x.  It takes care of cases where x is negative as erf is an odd
    function i.e. erf(-x) = -erf(x).

    Kramer, W., and Blomquist, F., 2000, Algorithms with Guaranteed Error Bounds
    for the Error Function and the Complementary Error Function

    http://www.math.uni-wuppertal.de/wrswt/literatur_en.html

    @author Kohei Yoshida <kohei@openoffice.org>

    @see #i55735#
 */
double SAL_CALL rtl_math_erf( double x ) SAL_THROW_EXTERN_C()
{
    if( x == 0.0 )
        return 0.0;

    bool bNegative = false;
    if ( x < 0.0 )
    {
        x = fabs( x );
        bNegative = true;
    }

    double fErf = 1.0;
    if ( x < 1.0e-10 )
        fErf = (double) (x*1.1283791670955125738961589031215452L);
    else if ( x < 0.65 )
        lcl_Erf0065( x, fErf );
    else
        fErf = 1.0 - rtl_math_erfc( x );

    if ( bNegative )
        fErf *= -1.0;

    return fErf;
}


/** Parent complementary error function (erfc) that calls different algorithms
    based on the value of x.  It takes care of cases where x is negative as erfc
    satisfies relationship erfc(-x) = 2 - erfc(x).  See the comment for Erf(x)
    for the source publication.

    @author Kohei Yoshida <kohei@openoffice.org>

    @see #i55735#, moved from module scaddins (#i97091#)

 */
double SAL_CALL rtl_math_erfc( double x ) SAL_THROW_EXTERN_C()
{
    if ( x == 0.0 )
        return 1.0;

    bool bNegative = false;
    if ( x < 0.0 )
    {
        x = fabs( x );
        bNegative = true;
    }

    double fErfc = 0.0;
    if ( x >= 0.65 )
    {
        if ( x < 6.0 )
            lcl_Erfc0600( x, fErfc );
        else
            lcl_Erfc2654( x, fErfc );
    }
    else
        fErfc = 1.0 - rtl_math_erf( x );

    if ( bNegative )
        fErfc = 2.0 - fErfc;

    return fErfc;
}

/** improved accuracy of asinh for |x| large and for x near zero
    @see #i97605#
 */
double SAL_CALL rtl_math_asinh( double fX ) SAL_THROW_EXTERN_C()
{
    if ( fX == 0.0 )
        return 0.0;
    else
    {
        double fSign = 1.0;
        if ( fX < 0.0 )
        {
            fX = - fX;
            fSign = -1.0;
        }
        if ( fX < 0.125 )
            return fSign * rtl_math_log1p( fX + fX*fX / (1.0 + sqrt( 1.0 + fX*fX)));
        else if ( fX < 1.25e7 )
            return fSign * log( fX + sqrt( 1.0 + fX*fX));
        else
            return fSign * log( 2.0*fX);
    }
}

/** improved accuracy of acosh for x large and for x near 1
    @see #i97605#
 */
double SAL_CALL rtl_math_acosh( double fX ) SAL_THROW_EXTERN_C()
{
    volatile double fZ = fX - 1.0;
    if ( fX < 1.0 )
    {
        double fResult;
        ::rtl::math::setNan( &fResult );
        return fResult;
    }
    else if ( fX == 1.0 )
        return 0.0;
    else if ( fX < 1.1 )
        return rtl_math_log1p( fZ + sqrt( fZ*fZ + 2.0*fZ));
    else if ( fX < 1.25e7 )
        return log( fX + sqrt( fX*fX - 1.0));
    else
        return log( 2.0*fX);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
