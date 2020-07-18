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

#include <rtl/math.h>

#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <rtl/alloc.h>
#include <rtl/character.hxx>
#include <rtl/math.hxx>
#include <rtl/strbuf.h>
#include <rtl/string.h>
#include <rtl/ustrbuf.h>
#include <rtl/ustring.h>
#include <sal/mathconf.h>
#include <sal/types.h>

#include <algorithm>
#include <cassert>
#include <float.h>
#include <limits>
#include <limits.h>
#include <math.h>
#include <memory>
#include <stdlib.h>

#include <dtoa.h>

int const n10Count = 16;
double const n10s[2][n10Count] = {
    { 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8,
      1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16 },
    { 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8,
      1e-9, 1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16 }
};

// return pow(10.0,nExp) optimized for exponents in the interval [-16,16]
static double getN10Exp(int nExp)
{
    if (nExp < 0)
    {
        // && -nExp > 0 necessary for std::numeric_limits<int>::min()
        // because -nExp = nExp
        if (-nExp <= n10Count && -nExp > 0)
            return n10s[1][-nExp-1];
        return pow(10.0, static_cast<double>(nExp));
    }
    if (nExp > 0)
    {
        if (nExp <= n10Count)
            return n10s[0][nExp-1];

        return pow(10.0, static_cast<double>(nExp));
    }
    return 1.0;
}

namespace {

double const nCorrVal[] = {
    0, 9e-1, 9e-2, 9e-3, 9e-4, 9e-5, 9e-6, 9e-7, 9e-8,
    9e-9, 9e-10, 9e-11, 9e-12, 9e-13, 9e-14, 9e-15
};

struct StringTraits
{
    typedef char Char;

    typedef rtl_String String;

    static void createString(rtl_String ** pString,
                                    char const * pChars, sal_Int32 nLen)
    {
        rtl_string_newFromStr_WithLength(pString, pChars, nLen);
    }

    static void createBuffer(rtl_String ** pBuffer,
                                    const sal_Int32 * pCapacity)
    {
        rtl_string_new_WithLength(pBuffer, *pCapacity);
    }

    static void appendChars(rtl_String ** pBuffer, sal_Int32 * pCapacity,
                                   sal_Int32 * pOffset, char const * pChars,
                                   sal_Int32 nLen)
    {
        assert(pChars);
        rtl_stringbuffer_insert(pBuffer, pCapacity, *pOffset, pChars, nLen);
        *pOffset += nLen;
    }

    static void appendAscii(rtl_String ** pBuffer, sal_Int32 * pCapacity,
                                   sal_Int32 * pOffset, char const * pStr,
                                   sal_Int32 nLen)
    {
        assert(pStr);
        rtl_stringbuffer_insert(pBuffer, pCapacity, *pOffset, pStr, nLen);
        *pOffset += nLen;
    }
};

struct UStringTraits
{
    typedef sal_Unicode Char;

    typedef rtl_uString String;

    static void createString(rtl_uString ** pString,
                                    sal_Unicode const * pChars, sal_Int32 nLen)
    {
        rtl_uString_newFromStr_WithLength(pString, pChars, nLen);
    }

    static void createBuffer(rtl_uString ** pBuffer,
                                    const sal_Int32 * pCapacity)
    {
        rtl_uString_new_WithLength(pBuffer, *pCapacity);
    }

    static void appendChars(rtl_uString ** pBuffer,
                                   sal_Int32 * pCapacity, sal_Int32 * pOffset,
                                   sal_Unicode const * pChars, sal_Int32 nLen)
    {
        assert(pChars);
        rtl_uStringbuffer_insert(pBuffer, pCapacity, *pOffset, pChars, nLen);
        *pOffset += nLen;
    }

    static void appendAscii(rtl_uString ** pBuffer,
                                   sal_Int32 * pCapacity, sal_Int32 * pOffset,
                                   char const * pStr, sal_Int32 nLen)
    {
        rtl_uStringbuffer_insert_ascii(pBuffer, pCapacity, *pOffset, pStr,
                                       nLen);
        *pOffset += nLen;
    }
};

/** If value (passed as absolute value) is an integer representable as double,
    which we handle explicitly at some places.
 */
bool isRepresentableInteger(double fAbsValue)
{
    assert(fAbsValue >= 0.0);
    const sal_Int64 kMaxInt = (static_cast< sal_Int64 >(1) << 53) - 1;
    if (fAbsValue <= static_cast< double >(kMaxInt))
    {
        sal_Int64 nInt = static_cast< sal_Int64 >(fAbsValue);
        // Check the integer range again because double comparison may yield
        // true within the precision range.
        // XXX loplugin:fpcomparison complains about floating-point comparison
        // for static_cast<double>(nInt) == fAbsValue, though we actually want
        // this here.
        if (nInt > kMaxInt)
            return false;
        double fInt = static_cast< double >(nInt);
        return !(fInt < fAbsValue) && !(fInt > fAbsValue);
    }
    return false;
}

// Returns 1-based index of least significant bit in a number, or zero if number is zero
int findFirstSetBit(unsigned n)
{
#if defined _WIN32
    unsigned long pos;
    unsigned char bNonZero = _BitScanForward(&pos, n);
    return (bNonZero == 0) ? 0 : pos + 1;
#else
    return __builtin_ffs(n);
#endif
}

/** Returns number of binary bits for fractional part of the number
    Expects a proper non-negative double value, not +-INF, not NAN
 */
int getBitsInFracPart(double fAbsValue)
{
    assert(std::isfinite(fAbsValue) && fAbsValue >= 0.0);
    if (fAbsValue == 0.0)
        return 0;
    auto pValParts = reinterpret_cast< const sal_math_Double * >(&fAbsValue);
    int nExponent = pValParts->inf_parts.exponent - 1023;
    if (nExponent >= 52)
        return 0; // All bits in fraction are in integer part of the number
    int nLeastSignificant = findFirstSetBit(pValParts->inf_parts.fraction_lo);
    if (nLeastSignificant == 0)
    {
        nLeastSignificant = findFirstSetBit(pValParts->inf_parts.fraction_hi);
        if (nLeastSignificant == 0)
            nLeastSignificant = 53; // the implied leading 1 is the least significant
        else
            nLeastSignificant += 32;
    }
    int nFracSignificant = 53 - nLeastSignificant;
    int nBitsInFracPart = nFracSignificant - nExponent;

    return std::max(nBitsInFracPart, 0);
}

template< typename T >
void doubleToString(typename T::String ** pResult,
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
    bool bSign = std::signbit(fValue);

    if (bSign)
        fValue = -fValue;

    if (std::isnan(fValue))
    {
        // #i112652# XMLSchema-2
        sal_Int32 nCapacity = RTL_CONSTASCII_LENGTH("NaN");
        if (!pResultCapacity)
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
    if (bHuge || std::isinf(fValue))
    {
        // #i112652# XMLSchema-2
        sal_Int32 nCapacity = RTL_CONSTASCII_LENGTH("-INF");
        if (!pResultCapacity)
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

    // Use integer representation for integer values that fit into the
    // mantissa (1.((2^53)-1)) with a precision of 1 for highest accuracy.
    const sal_Int64 kMaxInt = (static_cast< sal_Int64 >(1) << 53) - 1;
    if ((eFormat == rtl_math_StringFormat_Automatic ||
         eFormat == rtl_math_StringFormat_F) && fValue <= static_cast< double >(kMaxInt))
    {
        sal_Int64 nInt = static_cast< sal_Int64 >(fValue);
        // Check the integer range again because double comparison may yield
        // true within the precision range.
        if (nInt <= kMaxInt && static_cast< double >(nInt) == fValue)
        {
            if (nDecPlaces == rtl_math_DecimalPlaces_Max)
                nDecPlaces = 0;
            else
                nDecPlaces = ::std::max< sal_Int32 >(::std::min<sal_Int32>(nDecPlaces, 15), -15);

            if (bEraseTrailingDecZeros && nDecPlaces > 0)
                nDecPlaces = 0;

            // Round before decimal position.
            if (nDecPlaces < 0)
            {
                sal_Int64 nRounding = static_cast< sal_Int64 >(getN10Exp(-nDecPlaces - 1));
                sal_Int64 nTemp = nInt / nRounding;
                int nDigit = nTemp % 10;
                nTemp /= 10;

                if (nDigit >= 5)
                    ++nTemp;

                nTemp *= 10;
                nTemp *= nRounding;
                nInt = nTemp;
                nDecPlaces = 0;
            }

            // Max 1 sign, 16 integer digits, 15 group separators, 1 decimal
            // separator, 15 decimals digits.
            typename T::Char aBuf[64];
            typename T::Char * pBuf = aBuf;
            typename T::Char * p = pBuf;

            // Backward fill.
            size_t nGrouping = 0;
            sal_Int32 nGroupDigits = 0;
            do
            {
                typename T::Char nDigit = nInt % 10;
                nInt /= 10;
                *p++ = nDigit + '0';
                if (pGroups && pGroups[nGrouping] == ++nGroupDigits && nInt > 0 && cGroupSeparator)
                {
                    *p++ = cGroupSeparator;
                    if (pGroups[nGrouping+1])
                        ++nGrouping;
                    nGroupDigits = 0;
                }
            }
            while (nInt > 0);
            if (bSign)
                *p++ = '-';

            // Reverse buffer content.
            sal_Int32 n = (p - pBuf) / 2;
            for (sal_Int32 i=0; i < n; ++i)
            {
                ::std::swap( pBuf[i], p[-i-1]);
            }

            // Append decimals.
            if (nDecPlaces > 0)
            {
                *p++ = cDecSeparator;
                while (nDecPlaces--)
                    *p++ = '0';
            }

            if (!pResultCapacity)
                T::createString(pResult, pBuf, p - pBuf);
            else
                T::appendChars(pResult, pResultCapacity, &nResultOffset, pBuf, p - pBuf);

            return;
        }
    }

    // find the exponent
    int nExp = 0;
    if ( fValue > 0.0 )
    {
        // Cap nExp at a small value beyond which "fValue /= N10Exp" would lose precision (or N10Exp
        // might even be zero); that will produce output with the decimal point in a non-normalized
        // position, but the current quality of output for such small values is probably abysmal,
        // anyway:
        nExp = std::max(
            static_cast< int >(floor(log10(fValue))), std::numeric_limits<double>::min_exponent10);
        double const N10Exp = getN10Exp(nExp);
        assert(N10Exp != 0);
        fValue /= N10Exp;
    }

    switch (eFormat)
    {
        case rtl_math_StringFormat_Automatic:
        {   // E or F depending on exponent magnitude
            int nPrec;
            if (nExp <= -15 || nExp >= 15)  // was <-16, >16 in ancient versions, which leads to inaccuracies
            {
                nPrec = 14;
                eFormat = rtl_math_StringFormat_E;
            }
            else
            {
                if (nExp < 14)
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

            if (nDecPlaces == rtl_math_DecimalPlaces_Max)
                nDecPlaces = nPrec;
        }
        break;

        case rtl_math_StringFormat_G :
        case rtl_math_StringFormat_G1 :
        case rtl_math_StringFormat_G2 :
        {   // G-Point, similar to sprintf %G
            if (nDecPlaces == rtl_math_DecimalPlaces_DefaultSignificance)
                nDecPlaces = 6;

            if (nExp < -4 || nExp >= nDecPlaces)
            {
                nDecPlaces = std::max< sal_Int32 >(1, nDecPlaces - 1);

                if (eFormat == rtl_math_StringFormat_G)
                    eFormat = rtl_math_StringFormat_E;
                else if (eFormat == rtl_math_StringFormat_G2)
                    eFormat = rtl_math_StringFormat_E2;
                else if (eFormat == rtl_math_StringFormat_G1)
                    eFormat = rtl_math_StringFormat_E1;
            }
            else
            {
                nDecPlaces = std::max< sal_Int32 >(0, nDecPlaces - nExp - 1);
                eFormat = rtl_math_StringFormat_F;
            }
        }
        break;
        default:
        break;
    }

    sal_Int32 nDigits = nDecPlaces + 1;

    if (eFormat == rtl_math_StringFormat_F)
        nDigits += nExp;

    // Round the number
    if(nDigits >= 0)
    {
        fValue += nRoundVal[std::min<sal_Int32>(nDigits, 15)];
        if (fValue >= 10)
        {
            fValue = 1.0;
            nExp++;

            if (eFormat == rtl_math_StringFormat_F)
                nDigits++;
        }
    }

    static sal_Int32 const nBufMax = 256;
    typename T::Char aBuf[nBufMax];
    typename T::Char * pBuf;
    sal_Int32 nBuf = static_cast< sal_Int32 >
        (nDigits <= 0 ? std::max< sal_Int32 >(nDecPlaces, abs(nExp))
          : nDigits + nDecPlaces ) + 10 + (pGroups ? abs(nDigits) * 2 : 0);

    if (nBuf > nBufMax)
    {
        pBuf = static_cast< typename T::Char * >(
            malloc(nBuf * sizeof (typename T::Char)));
        OSL_ENSURE(pBuf, "Out of memory");
    }
    else
    {
        pBuf = aBuf;
    }

    typename T::Char * p = pBuf;
    if ( bSign )
        *p++ = static_cast< typename T::Char >('-');

    bool bHasDec = false;

    int nDecPos;
    // Check for F format and number < 1
    if(eFormat == rtl_math_StringFormat_F)
    {
        if(nExp < 0)
        {
            *p++ = static_cast< typename T::Char >('0');
            if (nDecPlaces > 0)
            {
                *p++ = cDecSeparator;
                bHasDec = true;
            }

            sal_Int32 i = (nDigits <= 0 ? nDecPlaces : -nExp - 1);

            while((i--) > 0)
            {
                *p++ = static_cast< typename T::Char >('0');
            }

            nDecPos = 0;
        }
        else
        {
            nDecPos = nExp + 1;
        }
    }
    else
    {
        nDecPos = 1;
    }

    int nGrouping = 0, nGroupSelector = 0, nGroupExceed = 0;
    if (nDecPos > 1 && pGroups && pGroups[0] && cGroupSeparator)
    {
        while (nGrouping + pGroups[nGroupSelector] < nDecPos)
        {
            nGrouping += pGroups[nGroupSelector];
            if (pGroups[nGroupSelector+1])
            {
                if (nGrouping + pGroups[nGroupSelector+1] >= nDecPos)
                    break;  // while

                ++nGroupSelector;
            }
            else if (!nGroupExceed)
            {
                nGroupExceed = nGrouping;
            }
        }
    }

    // print the number
    if (nDigits > 0)
    {
        for (int i = 0; ; i++)
        {
            if (i < 15)     // was 16 in ancient versions, which leads to inaccuracies
            {
                int nDigit;
                if (nDigits-1 == 0 && i > 0 && i < 14)
                    nDigit = static_cast< int >(floor( fValue + nCorrVal[15-i]));
                else
                    nDigit = static_cast< int >(fValue + 1E-15);

                if (nDigit >= 10)
                {   // after-treatment of up-rounding to the next decade
                    sal_Int32 sLen = static_cast< long >(p-pBuf)-1;
                    if (sLen == -1 || (sLen == 0 && bSign))
                    {
                        // Assert that no one changed the logic we rely on.
                        assert(!bSign || *pBuf == static_cast< typename T::Char >('-'));
                        p = pBuf;
                        if (bSign)
                            ++p;
                        if (eFormat == rtl_math_StringFormat_F)
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
                            if (j == 0 && bSign)
                            {
                                // Do not touch leading minus sign put earlier.
                                assert(cS == static_cast< typename T::Char >('-'));
                                break;  // for, this is the last character backwards.
                            }
                            if (cS != cDecSeparator)
                            {
                                if (cS != static_cast< typename T::Char >('9'))
                                {
                                    pBuf[j] = ++cS;
                                    j = -1;                 // break loop
                                }
                                else
                                {
                                    pBuf[j] = static_cast< typename T::Char >('0');
                                    if (j == 0 || (j == 1 && bSign))
                                    {
                                        if (eFormat == rtl_math_StringFormat_F)
                                        {   // insert '1'
                                            typename T::Char * px = p++;
                                            while (pBuf < px)
                                            {
                                                *px = *(px-1);
                                                px--;
                                            }

                                            pBuf[0] = static_cast< typename T::Char >('1');
                                        }
                                        else
                                        {
                                            pBuf[j] = static_cast< typename T::Char >('1');
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
                    fValue = (fValue - nDigit) * 10.0;
                }
            }
            else
            {
                *p++ = static_cast< typename T::Char >('0');
            }

            if (!--nDigits)
                break;  // for

            if (nDecPos)
            {
                if(!--nDecPos)
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
    {   // nDecPlaces < 0 did round the value
        while (--nDecPos > 0)
        {   // fill before decimal point
            if (nDecPos == nGrouping)
            {
                *p++ = cGroupSeparator;
                nGrouping -= pGroups[nGroupSelector];

                if (nGroupSelector && nGrouping < nGroupExceed)
                    --nGroupSelector;
            }

            *p++ = static_cast< typename T::Char >('0');
        }
    }

    if (bEraseTrailingDecZeros && bHasDec && p > pBuf)
    {
        while (*(p-1) == static_cast< typename T::Char >('0'))
        {
            p--;
        }

        if (*(p-1) == cDecSeparator)
            p--;
    }

    // Print the exponent ('E', followed by '+' or '-', followed by exactly
    // three digits for rtl_math_StringFormat_E).  The code in
    // rtl_[u]str_valueOf{Float|Double} relies on this format.
    if (eFormat == rtl_math_StringFormat_E || eFormat == rtl_math_StringFormat_E2 || eFormat == rtl_math_StringFormat_E1)
    {
        if (p == pBuf)
            *p++ = static_cast< typename T::Char >('1');
                // maybe no nDigits if nDecPlaces < 0

        *p++ = static_cast< typename T::Char >('E');
        if(nExp < 0)
        {
            nExp = -nExp;
            *p++ = static_cast< typename T::Char >('-');
        }
        else
        {
            *p++ = static_cast< typename T::Char >('+');
        }

        if (eFormat == rtl_math_StringFormat_E || nExp >= 100)
            *p++ = static_cast< typename T::Char >(
                nExp / 100 + static_cast< typename T::Char >('0') );

        nExp %= 100;

        if (eFormat == rtl_math_StringFormat_E || eFormat == rtl_math_StringFormat_E2 || nExp >= 10)
            *p++ = static_cast< typename T::Char >(
                nExp / 10 + static_cast< typename T::Char >('0') );

        *p++ = static_cast< typename T::Char >(
            nExp % 10 + static_cast< typename T::Char >('0') );
    }

    if (!pResultCapacity)
        T::createString(pResult, pBuf, p - pBuf);
    else
        T::appendChars(pResult, pResultCapacity, &nResultOffset, pBuf, p - pBuf);

    if (pBuf != &aBuf[0])
        free(pBuf);
}

}

void SAL_CALL rtl_math_doubleToString(rtl_String ** pResult,
                                      sal_Int32 * pResultCapacity,
                                      sal_Int32 nResultOffset, double fValue,
                                      rtl_math_StringFormat eFormat,
                                      sal_Int32 nDecPlaces,
                                      char cDecSeparator,
                                      sal_Int32 const * pGroups,
                                      char cGroupSeparator,
                                      sal_Bool bEraseTrailingDecZeros)
    SAL_THROW_EXTERN_C()
{
    doubleToString< StringTraits >(
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
    doubleToString< UStringTraits >(
        pResult, pResultCapacity, nResultOffset, fValue, eFormat, nDecPlaces,
        cDecSeparator, pGroups, cGroupSeparator, bEraseTrailingDecZeros);
}

namespace {

template< typename CharT >
double stringToDouble(CharT const * pBegin, CharT const * pEnd,
                             CharT cDecSeparator, CharT cGroupSeparator,
                             rtl_math_ConversionStatus * pStatus,
                             CharT const ** pParsedEnd)
{
    double fVal = 0.0;
    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;

    CharT const * p0 = pBegin;
    while (p0 != pEnd && (*p0 == CharT(' ') || *p0 == CharT('\t')))
    {
        ++p0;
    }

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
    if ((pEnd - p) >= 3)
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
        std::unique_ptr<char[]> bufInHeap;
        std::unique_ptr<const CharT * []> bufInHeapMap;
        constexpr int bufOnStackSize = 256;
        char bufOnStack[bufOnStackSize];
        const CharT* bufOnStackMap[bufOnStackSize];
        char* buf = bufOnStack;
        const CharT** bufmap = bufOnStackMap;
        int bufpos = 0;
        const size_t bufsize = pEnd - p + (bSign ? 2 : 1);
        if (bufsize > bufOnStackSize)
        {
            bufInHeap = std::make_unique<char[]>(bufsize);
            bufInHeapMap = std::make_unique<const CharT*[]>(bufsize);
            buf = bufInHeap.get();
            bufmap = bufInHeapMap.get();
        }

        if (bSign)
        {
            buf[0] = '-';
            bufmap[0] = p; // yes, this may be the same pointer as for the next mapping
            bufpos = 1;
        }
        // Put first zero to buffer for strings like "-0"
        if (p != pEnd && *p == CharT('0'))
        {
            buf[bufpos] = '0';
            bufmap[bufpos] = p;
            ++bufpos;
            ++p;
        }
        // Leading zeros and group separators between digits may be safely
        // ignored. p0 < p implies that there was a leading 0 already,
        // consecutive group separators may not happen as *(p+1) is checked for
        // digit.
        while (p != pEnd && (*p == CharT('0') || (*p == cGroupSeparator
                        && p0 < p && p+1 < pEnd && rtl::isAsciiDigit(*(p+1)))))
        {
            ++p;
        }

        // integer part of mantissa
        for (; p != pEnd; ++p)
        {
            CharT c = *p;
            if (rtl::isAsciiDigit(c))
            {
                buf[bufpos] = static_cast<char>(c);
                bufmap[bufpos] = p;
                ++bufpos;
            }
            else if (c != cGroupSeparator)
            {
                break;
            }
            else if (p == p0 || (p+1 == pEnd) || !rtl::isAsciiDigit(*(p+1)))
            {
                // A leading or trailing (not followed by a digit) group
                // separator character is not a group separator.
                break;
            }
        }

        // fraction part of mantissa
        if (p != pEnd && *p == cDecSeparator)
        {
            buf[bufpos] = '.';
            bufmap[bufpos] = p;
            ++bufpos;
            ++p;

            for (; p != pEnd; ++p)
            {
                CharT c = *p;
                if (!rtl::isAsciiDigit(c))
                {
                    break;
                }
                buf[bufpos] = static_cast<char>(c);
                bufmap[bufpos] = p;
                ++bufpos;
            }
        }

        // Exponent
        if (p != p0 && p != pEnd && (*p == CharT('E') || *p == CharT('e')))
        {
            buf[bufpos] = 'E';
            bufmap[bufpos] = p;
            ++bufpos;
            ++p;
            if (p != pEnd && *p == CharT('-'))
            {
                buf[bufpos] = '-';
                bufmap[bufpos] = p;
                ++bufpos;
                ++p;
            }
            else if (p != pEnd && *p == CharT('+'))
                ++p;

            for (; p != pEnd; ++p)
            {
                CharT c = *p;
                if (!rtl::isAsciiDigit(c))
                    break;

                buf[bufpos] = static_cast<char>(c);
                bufmap[bufpos] = p;
                ++bufpos;
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
                while (p != pEnd && rtl::isAsciiDigit(*p))
                    ++p;
                bDone = true;
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
                while (p != pEnd && rtl::isAsciiDigit(*p))
                {
                    ++p;
                }
                bDone = true;
            }
        }

        if (!bDone)
        {
            buf[bufpos] = '\0';
            bufmap[bufpos] = p;
            char* pCharParseEnd;
            errno = 0;
            fVal = strtod_nolocale(buf, &pCharParseEnd);
            if (errno == ERANGE)
                eStatus = rtl_math_ConversionStatus_OutOfRange;
            p = bufmap[pCharParseEnd - buf];
            bSign = false;
        }
    }

    // overflow also if more than DBL_MAX_10_EXP digits without decimal
    // separator, or 0. and more than DBL_MIN_10_EXP digits, ...
    bool bHuge = fVal == HUGE_VAL; // g++ 3.0.1 requires it this way...
    if (bHuge)
        eStatus = rtl_math_ConversionStatus_OutOfRange;

    if (bSign)
        fVal = -fVal;

    if (pStatus)
        *pStatus = eStatus;

    if (pParsedEnd)
        *pParsedEnd = p == p0 ? pBegin : p;

    return fVal;
}

}

double SAL_CALL rtl_math_stringToDouble(char const * pBegin,
                                        char const * pEnd,
                                        char cDecSeparator,
                                        char cGroupSeparator,
                                        rtl_math_ConversionStatus * pStatus,
                                        char const ** pParsedEnd)
    SAL_THROW_EXTERN_C()
{
    return stringToDouble(
        reinterpret_cast<unsigned char const *>(pBegin),
        reinterpret_cast<unsigned char const *>(pEnd),
        static_cast<unsigned char>(cDecSeparator),
        static_cast<unsigned char>(cGroupSeparator), pStatus,
        reinterpret_cast<unsigned char const **>(pParsedEnd));
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

    if (fValue == 0.0)
        return fValue;

    if ( nDecPlaces == 0 && eMode == rtl_math_RoundingMode_Corrected )
        return std::round( fValue );

    // sign adjustment
    bool bSign = std::signbit( fValue );
    if (bSign)
        fValue = -fValue;

    double fFac = 0;
    if (nDecPlaces != 0)
    {
        // max 20 decimals, we don't have unlimited precision
        // #38810# and no overflow on fValue*=fFac
        if (nDecPlaces < -20 || 20 < nDecPlaces || fValue > (DBL_MAX / 1e20))
            return bSign ? -fValue : fValue;

        fFac = getN10Exp(nDecPlaces);
        fValue *= fFac;
    }

    switch ( eMode )
    {
        case rtl_math_RoundingMode_Corrected :
        {
            int nExp;       // exponent for correction
            if ( fValue > 0.0 )
                nExp = static_cast<int>( floor( log10( fValue ) ) );
            else
                nExp = 0;

            int nIndex;

            if (nExp < 0)
                nIndex = 15;
            else if (nExp >= 14)
                nIndex = 0;
            else
                nIndex = 15 - nExp;

            fValue = floor(fValue + 0.5 + nCorrVal[nIndex]);
        }
        break;
        case rtl_math_RoundingMode_Down:
            fValue = rtl::math::approxFloor(fValue);
        break;
        case rtl_math_RoundingMode_Up:
            fValue = rtl::math::approxCeil(fValue);
        break;
        case rtl_math_RoundingMode_Floor:
            fValue = bSign ? rtl::math::approxCeil(fValue)
                : rtl::math::approxFloor( fValue );
        break;
        case rtl_math_RoundingMode_Ceiling:
            fValue = bSign ? rtl::math::approxFloor(fValue)
                : rtl::math::approxCeil(fValue);
        break;
        case rtl_math_RoundingMode_HalfDown :
        {
            double f = floor(fValue);
            fValue = ((fValue - f) <= 0.5) ? f : ceil(fValue);
        }
        break;
        case rtl_math_RoundingMode_HalfUp:
        {
            double f = floor(fValue);
            fValue = ((fValue - f) < 0.5) ? f : ceil(fValue);
        }
        break;
        case rtl_math_RoundingMode_HalfEven:
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
            if (FLT_ROUNDS == 1)
            {
                volatile double x = fValue + 1.0 / DBL_EPSILON;
                fValue = x - 1.0 / DBL_EPSILON;
            }
            else
#endif // FLT_ROUNDS
            {
                double f = floor(fValue);
                if ((fValue - f) != 0.5)
                {
                    fValue = floor( fValue + 0.5 );
                }
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

    if (nDecPlaces != 0)
        fValue /= fFac;

    return bSign ? -fValue : fValue;
}

double SAL_CALL rtl_math_pow10Exp(double fValue, int nExp) SAL_THROW_EXTERN_C()
{
    return fValue * getN10Exp(nExp);
}

double SAL_CALL rtl_math_approxValue( double fValue ) SAL_THROW_EXTERN_C()
{
    const double fBigInt = 2199023255552.0; // 2^41 -> only 11 bits left for fractional part, fine as decimal
    if (fValue == 0.0 || fValue == HUGE_VAL || !std::isfinite( fValue) || fValue > fBigInt)
    {
        // We don't handle these conditions.  Bail out.
        return fValue;
    }

    double fOrigValue = fValue;

    bool bSign = std::signbit(fValue);
    if (bSign)
        fValue = -fValue;

    // If the value is either integer representable as double,
    // or only has small number of bits in fraction part, then we need not do any approximation
    if (isRepresentableInteger(fValue) || getBitsInFracPart(fValue) <= 11)
        return fOrigValue;

    int nExp = static_cast< int >(floor(log10(fValue)));
    nExp = 14 - nExp;
    double fExpValue = getN10Exp(nExp);

    fValue *= fExpValue;
    // If the original value was near DBL_MIN we got an overflow. Restore and
    // bail out.
    if (!std::isfinite(fValue))
        return fOrigValue;

    fValue = rtl_math_round(fValue, 0, rtl_math_RoundingMode_Corrected);
    fValue /= fExpValue;

    // If the original value was near DBL_MAX we got an overflow. Restore and
    // bail out.
    if (!std::isfinite(fValue))
        return fOrigValue;

    return bSign ? -fValue : fValue;
}

bool SAL_CALL rtl_math_approxEqual(double a, double b) SAL_THROW_EXTERN_C()
{
    static const double e48 = 1.0 / (16777216.0 * 16777216.0);
    static const double e44 = e48 * 16.0;

    if (a == b)
        return true;

    if (a == 0.0 || b == 0.0)
        return false;

    const double d = fabs(a - b);
    if (!std::isfinite(d))
        return false;   // Nan or Inf involved

    a = fabs(a);
    if (d > (a * e44))
        return false;
    b = fabs(b);
    if (d > (b * e44))
        return false;

    if (isRepresentableInteger(d) && isRepresentableInteger(a) && isRepresentableInteger(b))
        return false;   // special case for representable integers.

    return (d < a * e48 && d < b * e48);
}

double SAL_CALL rtl_math_expm1(double fValue) SAL_THROW_EXTERN_C()
{
    return expm1(fValue);
}

double SAL_CALL rtl_math_log1p(double fValue) SAL_THROW_EXTERN_C()
{
#ifdef __APPLE__
    if (fValue == -0.0)
        return fValue; // macOS 10.8 libc returns 0.0 for -0.0
#endif

    return log1p(fValue);
}

double SAL_CALL rtl_math_atanh(double fValue) SAL_THROW_EXTERN_C()
#if defined __clang__
    __attribute__((no_sanitize("float-divide-by-zero"))) // atahn(1) -> inf
#endif
{
   return 0.5 * rtl_math_log1p(2.0 * fValue / (1.0-fValue));
}

/** Parent error function (erf) */
double SAL_CALL rtl_math_erf(double x) SAL_THROW_EXTERN_C()
{
    return erf(x);
}

/** Parent complementary error function (erfc) */
double SAL_CALL rtl_math_erfc(double x) SAL_THROW_EXTERN_C()
{
    return erfc(x);
}

/** improved accuracy of asinh for |x| large and for x near zero
    @see #i97605#
 */
double SAL_CALL rtl_math_asinh(double fX) SAL_THROW_EXTERN_C()
{
    if ( fX == 0.0 )
        return 0.0;

    double fSign = 1.0;
    if ( fX < 0.0 )
    {
        fX = - fX;
        fSign = -1.0;
    }

    if ( fX < 0.125 )
        return fSign * rtl_math_log1p( fX + fX*fX / (1.0 + sqrt( 1.0 + fX*fX)));

    if ( fX < 1.25e7 )
        return fSign * log( fX + sqrt( 1.0 + fX*fX));

    return fSign * log( 2.0*fX);
}

/** improved accuracy of acosh for x large and for x near 1
    @see #i97605#
 */
double SAL_CALL rtl_math_acosh(double fX) SAL_THROW_EXTERN_C()
{
    volatile double fZ = fX - 1.0;
    if (fX < 1.0)
    {
        double fResult;
        ::rtl::math::setNan( &fResult );
        return fResult;
    }
    if ( fX == 1.0 )
        return 0.0;

    if ( fX < 1.1 )
        return rtl_math_log1p( fZ + sqrt( fZ*fZ + 2.0*fZ));

    if ( fX < 1.25e7 )
        return log( fX + sqrt( fX*fX - 1.0));

    return log( 2.0*fX);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
