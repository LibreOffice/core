/*************************************************************************
 *
 *  $RCSfile: math.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 16:46:39 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#if defined WIN
#pragma optimize("",off)
#elif defined MSC && defined WNT
// #56399# z.B. 1e88 => 1,00000000000001E+088
#pragma optimize("g",off)
#endif // WIN, MSC, WNT

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
        sal_Int32 nCapacity = RTL_CONSTASCII_LENGTH("-1.#NAN");
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
                       RTL_CONSTASCII_STRINGPARAM("1"));
        T::appendChar(pResult, pResultCapacity, &nResultOffset, cDecSeparator);
        T::appendAscii(pResult, pResultCapacity, &nResultOffset,
                       RTL_CONSTASCII_STRINGPARAM("#NAN"));
        return;
    }

    bool bHuge = fValue == HUGE_VAL; // g++ 3.0.1 requires it this way...
    if ( bHuge || rtl::math::isInf( fValue ) )
    {
        sal_Int32 nCapacity = RTL_CONSTASCII_LENGTH("-1.#INF");
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
                       RTL_CONSTASCII_STRINGPARAM("1"));
        T::appendChar(pResult, pResultCapacity, &nResultOffset, cDecSeparator);
        T::appendAscii(pResult, pResultCapacity, &nResultOffset,
                       RTL_CONSTASCII_STRINGPARAM("#INF"));
        return;
    }

    // find the exponent
    int nExp = 0;
    if ( fValue > 0.0 )
    {
        if ( fValue < 1e-8 || fValue > 1e8 )
        {   // the shear whether it's faster or not is between 1e7 and 1e8
            nExp = static_cast< int >( floor( log10( fValue ) ) );
            fValue /= pow( 10.0, static_cast< double >( nExp ) );
        }
        else
        {   // else imagine 1E+308 ...
            while( fValue < 1.0 )
            {
                fValue *= 10.0;
                nExp--;
            }
            while( fValue >= 10.0 )
            {
                fValue /= 10.0;
                nExp++;
            }
        }
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

#if defined WIN || (defined MSC && defined WNT)
#pragma optimize("",on)
#endif // WIN, MSC, WNT

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
            nValExp = nFracExp - 1;    // no integer part => fraction exponent
        // one decimal digit needs ld(10) ~= 3.32 bits
        static const int nSigs = (DBL_MANT_DIG / 3) + 1;
        int nDigs = 0;
        for (; p != pEnd; ++p)
        {
            CharT c = *p;
            if (!isDigit(c))
                break;
            if ( nDigs < nSigs )
            {   // further digits (more than nSigs) don't have any significance
                fFrac = fFrac * 10.0 + static_cast< double >( c - CharT('0') );
                --nFracExp;
                ++nDigs;
            }
        }
        if ( fFrac != 0.0 )
            fVal += rtl::math::pow10Exp( fFrac, nFracExp );
        else if ( nValExp < 0 )
            nValExp = 0;        // no digit other than 0 after decimal point
    }

    if ( nValExp > 0 )
        --nValExp;      // started with offset +1 at the first mantissa digit

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
        {   // no matter what follows, zero stays zero, but carry on the offset
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
                else if ( nAllExp < DBL_MIN_10_EXP || (bOverFlow && bExpSign) )
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
                    fVal = rtl::math::pow10Exp( fVal, nExp );      // normal
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
                reinterpret_cast< sal_math_Double * >(&fVal)->w32_parts.msw
                    |= 0x80000000; // create negative NaN
                bSign = false; // don't negate again
            }
            // Eat any further digits:
            while (p != pEnd && isDigit(*p))
                ++p;
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
        *pParsedEnd = p;

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

    double fFac;
    if ( nDecPlaces != 0 )
    {
        // max 20 decimals, we don't have unlimited precision
        // #38810# and no overflow on fValue*=fFac
        if ( nDecPlaces < -20 || 20 < nDecPlaces || fValue > (DBL_MAX / 1e20) )
            return bSign ? -fValue : fValue;

        fFac = pow( 10.0, nDecPlaces );
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
    }

    if ( nDecPlaces != 0 )
        fValue /= fFac;

    return bSign ? -fValue : fValue;
}

double SAL_CALL rtl_math_pow10Exp(double fValue, int nExp) SAL_THROW_EXTERN_C()
{
    static int const n10Count = 16;
    static double const n10s[2][n10Count] = {
        { 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8,
          1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16 },
        { 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8,
          1e-9, 1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16 }
    };

    if ( nExp < 0 )
    {
        if ( -nExp <= n10Count )
            return fValue * n10s[1][-nExp-1];
        else
            return fValue * pow( 10.0, static_cast<double>( nExp ) );
    }
    else if ( nExp > 0 )
    {
        if ( nExp <= n10Count )
            return fValue * n10s[0][nExp-1];
        else
            return fValue * pow( 10.0, static_cast<double>( nExp ) );
    }
    else
        return fValue;
}
