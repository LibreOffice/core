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

#ifndef INCLUDED_RTL_MATH_HXX
#define INCLUDED_RTL_MATH_HXX

#include "rtl/math.h"
#include "rtl/strbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "sal/mathconf.h"
#include "sal/types.h"

#include <cstddef>
#include <math.h>

namespace rtl {

namespace math {

/** A wrapper around rtl_math_doubleToString.
 */
inline rtl::OString doubleToString(double fValue, rtl_math_StringFormat eFormat,
                                   sal_Int32 nDecPlaces,
                                   sal_Char cDecSeparator,
                                   sal_Int32 const * pGroups,
                                   sal_Char cGroupSeparator,
                                   bool bEraseTrailingDecZeros = false)
{
    rtl::OString aResult;
    rtl_math_doubleToString(&aResult.pData, NULL, 0, fValue, eFormat, nDecPlaces,
                            cDecSeparator, pGroups, cGroupSeparator,
                            bEraseTrailingDecZeros);
    return aResult;
}

/** A wrapper around rtl_math_doubleToString, with no grouping.
 */
inline rtl::OString doubleToString(double fValue, rtl_math_StringFormat eFormat,
                                   sal_Int32 nDecPlaces,
                                   sal_Char cDecSeparator,
                                   bool bEraseTrailingDecZeros = false)
{
    rtl::OString aResult;
    rtl_math_doubleToString(&aResult.pData, NULL, 0, fValue, eFormat, nDecPlaces,
                            cDecSeparator, NULL, 0, bEraseTrailingDecZeros);
    return aResult;
}

/** A wrapper around rtl_math_doubleToString that appends to an
    rtl::OStringBuffer.

    @since LibreOffice 5.4
*/
inline void doubleToStringBuffer(
    rtl::OStringBuffer& rBuffer, double fValue, rtl_math_StringFormat eFormat,
    sal_Int32 nDecPlaces, sal_Char cDecSeparator, sal_Int32 const * pGroups,
    sal_Char cGroupSeparator, bool bEraseTrailingDecZeros = false)
{
    rtl_String ** pData;
    sal_Int32 * pCapacity;
    rBuffer.accessInternals(&pData, &pCapacity);
    rtl_math_doubleToString(
        pData, pCapacity, rBuffer.getLength(), fValue, eFormat, nDecPlaces,
        cDecSeparator, pGroups, cGroupSeparator, bEraseTrailingDecZeros);
}

/** A wrapper around rtl_math_doubleToString that appends to an
    rtl::OStringBuffer, with no grouping.

    @since LibreOffice 5.4
*/
inline void doubleToStringBuffer(
    rtl::OStringBuffer& rBuffer, double fValue, rtl_math_StringFormat eFormat,
    sal_Int32 nDecPlaces, sal_Char cDecSeparator,
    bool bEraseTrailingDecZeros = false)
{
    rtl_String ** pData;
    sal_Int32 * pCapacity;
    rBuffer.accessInternals(&pData, &pCapacity);
    rtl_math_doubleToString(
        pData, pCapacity, rBuffer.getLength(), fValue, eFormat, nDecPlaces,
        cDecSeparator, NULL, 0, bEraseTrailingDecZeros);
}

/** A wrapper around rtl_math_doubleToUString.
 */
inline rtl::OUString doubleToUString(double fValue,
                                     rtl_math_StringFormat eFormat,
                                     sal_Int32 nDecPlaces,
                                     sal_Unicode cDecSeparator,
                                     sal_Int32 const * pGroups,
                                     sal_Unicode cGroupSeparator,
                                     bool bEraseTrailingDecZeros = false)
{
    rtl::OUString aResult;
    rtl_math_doubleToUString(&aResult.pData, NULL, 0, fValue, eFormat, nDecPlaces,
                             cDecSeparator, pGroups, cGroupSeparator,
                             bEraseTrailingDecZeros);
    return aResult;
}

/** A wrapper around rtl_math_doubleToUString, with no grouping.
 */
inline rtl::OUString doubleToUString(double fValue,
                                     rtl_math_StringFormat eFormat,
                                     sal_Int32 nDecPlaces,
                                     sal_Unicode cDecSeparator,
                                     bool bEraseTrailingDecZeros = false)
{
    rtl::OUString aResult;
    rtl_math_doubleToUString(&aResult.pData, NULL, 0, fValue, eFormat, nDecPlaces,
                             cDecSeparator, NULL, 0, bEraseTrailingDecZeros);
    return aResult;
}

/** A wrapper around rtl_math_doubleToUString that appends to an
    rtl::OUStringBuffer.
 */
inline void doubleToUStringBuffer( rtl::OUStringBuffer& rBuffer, double fValue,
                                   rtl_math_StringFormat eFormat,
                                   sal_Int32 nDecPlaces,
                                   sal_Unicode cDecSeparator,
                                   sal_Int32 const * pGroups,
                                   sal_Unicode cGroupSeparator,
                                   bool bEraseTrailingDecZeros = false)
{
    rtl_uString ** pData;
    sal_Int32 * pCapacity;
    rBuffer.accessInternals( &pData, &pCapacity );
    rtl_math_doubleToUString( pData, pCapacity, rBuffer.getLength(), fValue,
                              eFormat, nDecPlaces, cDecSeparator, pGroups,
                              cGroupSeparator, bEraseTrailingDecZeros);
}

/** A wrapper around rtl_math_doubleToUString that appends to an
    rtl::OUStringBuffer, with no grouping.
 */
inline void doubleToUStringBuffer( rtl::OUStringBuffer& rBuffer, double fValue,
                                   rtl_math_StringFormat eFormat,
                                   sal_Int32 nDecPlaces,
                                   sal_Unicode cDecSeparator,
                                   bool bEraseTrailingDecZeros = false)
{
    rtl_uString ** pData;
    sal_Int32 * pCapacity;
    rBuffer.accessInternals( &pData, &pCapacity );
    rtl_math_doubleToUString( pData, pCapacity, rBuffer.getLength(), fValue,
                              eFormat, nDecPlaces, cDecSeparator, NULL, 0,
                              bEraseTrailingDecZeros);
}

/** A wrapper around rtl_math_stringToDouble.
 */
inline double stringToDouble(rtl::OString const & rString,
                             sal_Char cDecSeparator, sal_Char cGroupSeparator,
                             rtl_math_ConversionStatus * pStatus = NULL,
                             sal_Int32 * pParsedEnd = NULL)
{
    sal_Char const * pBegin = rString.getStr();
    sal_Char const * pEnd;
    double fResult = rtl_math_stringToDouble(pBegin,
                                             pBegin + rString.getLength(),
                                             cDecSeparator, cGroupSeparator,
                                             pStatus, &pEnd);
    if (pParsedEnd != NULL)
        *pParsedEnd = static_cast<sal_Int32>(pEnd - pBegin);
    return fResult;
}

/** A wrapper around rtl_math_uStringToDouble.
 */
inline double stringToDouble(rtl::OUString const & rString,
                             sal_Unicode cDecSeparator,
                             sal_Unicode cGroupSeparator,
                             rtl_math_ConversionStatus * pStatus = NULL,
                             sal_Int32 * pParsedEnd = NULL)
{
    sal_Unicode const * pBegin = rString.getStr();
    sal_Unicode const * pEnd;
    double fResult = rtl_math_uStringToDouble(pBegin,
                                              pBegin + rString.getLength(),
                                              cDecSeparator, cGroupSeparator,
                                              pStatus, &pEnd);
    if (pParsedEnd != NULL)
        *pParsedEnd = static_cast<sal_Int32>(pEnd - pBegin);
    return fResult;
}

/** A wrapper around rtl_math_round.
 */
inline double round(
    double fValue, int nDecPlaces = 0,
    rtl_math_RoundingMode eMode = rtl_math_RoundingMode_Corrected)
{
    return rtl_math_round(fValue, nDecPlaces, eMode);
}

/** A wrapper around rtl_math_pow10Exp.
 */
inline double pow10Exp(double fValue, int nExp)
{
    return rtl_math_pow10Exp(fValue, nExp);
}

/** A wrapper around rtl_math_approxValue.
 */
inline double approxValue(double fValue)
{
    return rtl_math_approxValue(fValue);
}

/** A wrapper around rtl_math_expm1.
 */
inline double expm1(double fValue)
{
    return rtl_math_expm1(fValue);
}

/** A wrapper around rtl_math_log1p.
 */
inline double log1p(double fValue)
{
    return rtl_math_log1p(fValue);
}

/** A wrapper around rtl_math_atanh.
 */
inline double atanh(double fValue)
{
    return rtl_math_atanh(fValue);
}

/** A wrapper around rtl_math_erf.
 */
inline double erf(double fValue)
{
    return rtl_math_erf(fValue);
}

/** A wrapper around rtl_math_erfc.
 */
inline double erfc(double fValue)
{
    return rtl_math_erfc(fValue);
}

/** A wrapper around rtl_math_asinh.
 */
inline double asinh(double fValue)
{
    return rtl_math_asinh(fValue);
}

/** A wrapper around rtl_math_acosh.
 */
inline double acosh(double fValue)
{
    return rtl_math_acosh(fValue);
}

/** A wrapper around rtl_math_approxEqual.
 */
inline bool approxEqual(double a, double b)
{
    return rtl_math_approxEqual( a, b );
}

/** Test equality of two values with an accuracy defined by nPrec

    @attention
    approxEqual( value!=0.0, 0.0 ) _never_ yields true.
 */
inline bool approxEqual(double a, double b, sal_Int16 nPrec)
{
    if ( a == b )
        return true;
    double x = a - b;
    return (x < 0.0 ? -x : x)
        < ((a < 0.0 ? -a : a) * (1.0 / (pow(2.0, nPrec))));
}

/** Add two values.

    If signs differ and the absolute values are equal according to approxEqual()
    the method returns 0.0 instead of calculating the sum.

    If you wanted to sum up multiple values it would be convenient not to call
    approxAdd() for each value but instead remember the first value not equal to
    0.0, add all other values using normal + operator, and with the result and
    the remembered value call approxAdd().
 */
inline double approxAdd(double a, double b)
{
    if ( ((a < 0.0 && b > 0.0) || (b < 0.0 && a > 0.0))
         && approxEqual( a, -b ) )
        return 0.0;
    return a + b;
}

/** Subtract two values (a-b).

    If signs are identical and the values are equal according to approxEqual()
    the method returns 0.0 instead of calculating the subtraction.
 */
inline double approxSub(double a, double b)
{
    if ( ((a < 0.0 && b < 0.0) || (a > 0.0 && b > 0.0)) && approxEqual( a, b ) )
        return 0.0;
    return a - b;
}

/** floor() method taking approxValue() into account.

    Use for expected integer values being calculated by double functions.
 */
inline double approxFloor(double a)
{
    return floor( approxValue( a ));
}

/** ceil() method taking approxValue() into account.

    Use for expected integer values being calculated by double functions.
 */
inline double approxCeil(double a)
{
    return ceil( approxValue( a ));
}

/** Tests whether a value is neither INF nor NAN.
 */
inline bool isFinite(double d)
{
    return SAL_MATH_FINITE(d);
}

/** If a value represents +INF or -INF.

    The sign bit may be queried with isSignBitSet().

    If isFinite(d)==false and isInf(d)==false then NAN.
 */
inline bool isInf(double d)
{
    // exponent==0x7ff fraction==0
    return !SAL_MATH_FINITE(d) &&
        (reinterpret_cast< sal_math_Double * >(&d)->inf_parts.fraction_hi == 0)
        && (reinterpret_cast< sal_math_Double * >(&d)->inf_parts.fraction_lo
            == 0);
}

/** Test on any QNAN or SNAN.
 */
inline bool isNan(double d)
{
    // exponent==0x7ff fraction!=0
    return !SAL_MATH_FINITE(d) && (
        (reinterpret_cast< sal_math_Double * >(&d)->inf_parts.fraction_hi != 0)
        || (reinterpret_cast< sal_math_Double * >(&d)->inf_parts.fraction_lo
            != 0) );
}

/** If the sign bit is set.
 */
inline bool isSignBitSet(double d)
{
    return reinterpret_cast< sal_math_Double * >(&d)->inf_parts.sign != 0;
}

/** Set to +INF if bNegative==false or -INF if bNegative==true.
 */
inline void setInf(double * pd, bool bNegative)
{
    union
    {
        double sd;
        sal_math_Double md;
    };
    md.w32_parts.msw = bNegative ? 0xFFF00000 : 0x7FF00000;
    md.w32_parts.lsw = 0;
    *pd = sd;
}

/** Set a QNAN.
 */
inline void setNan(double * pd)
{
    union
    {
        double sd;
        sal_math_Double md;
    };
    md.w32_parts.msw = 0x7FFFFFFF;
    md.w32_parts.lsw = 0xFFFFFFFF;
    *pd = sd;
}

/** If a value is a valid argument for sin(), cos(), tan().

    IEEE 754 specifies that absolute values up to 2^64 (=1.844e19) for the
    radian must be supported by trigonometric functions.  Unfortunately, at
    least on x86 architectures, the FPU doesn't generate an error pattern for
    values >2^64 but produces erroneous results instead and sets only the
    "invalid operation" (IM) flag in the status word :-(  Thus the application
    has to handle it itself.
 */
inline bool isValidArcArg(double d)
{
    return fabs(d)
        <= (static_cast< double >(static_cast< unsigned long >(0x80000000))
            * static_cast< double >(static_cast< unsigned long >(0x80000000))
            * 2);
}

/** Safe sin(), returns NAN if not valid.
 */
inline double sin(double d)
{
    if ( isValidArcArg( d ) )
        return ::sin( d );
    setNan( &d );
    return d;
}

/** Safe cos(), returns NAN if not valid.
 */
inline double cos(double d)
{
    if ( isValidArcArg( d ) )
        return ::cos( d );
    setNan( &d );
    return d;
}

/** Safe tan(), returns NAN if not valid.
 */
inline double tan(double d)
{
    if ( isValidArcArg( d ) )
        return ::tan( d );
    setNan( &d );
    return d;
}

}

}

#endif // INCLUDED_RTL_MATH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
