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

#ifndef INCLUDED_RTL_MATH_H
#define INCLUDED_RTL_MATH_H

#include "sal/config.h"

#include "rtl/ustring.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Formatting modes for rtl_math_doubleToString and rtl_math_doubleToUString
    and rtl_math_doubleToUStringBuffer.
 */
enum rtl_math_StringFormat
{
    /** Like sprintf() %E.
     */
    rtl_math_StringFormat_E,

    /** Like sprintf() %f.
     */
    rtl_math_StringFormat_F,

    /** Like sprintf() %G, 'F' or 'E' format is used depending on which one is
        more compact.
    */
    rtl_math_StringFormat_G,

    /** Automatic, 'F' or 'E' format is used depending on the numeric value to
        be formatted.
     */
    rtl_math_StringFormat_Automatic,

    /** Same 'E', but with only 1 minimum digits in exponent.
        @since LibreOffice 5.0
     */
    rtl_math_StringFormat_E1,

    /** Same 'E', but with only 2 minimum digits in exponent.
        @since LibreOffice 5.0
     */
    rtl_math_StringFormat_E2,

    /** Same 'G', but with only 1 minimum digits in exponent.
        @since LibreOffice 5.0
     */
    rtl_math_StringFormat_G1,

    /** Same 'G', but with only 2 minimum digits in exponent.
        @since LibreOffice 5.0
     */
    rtl_math_StringFormat_G2,

    /** @cond INTERNAL */
    rtl_math_StringFormat_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
    /** @endcond */
};

/** Status for rtl_math_stringToDouble and rtl_math_uStringToDouble.
 */
enum rtl_math_ConversionStatus
{
    /** Conversion was successful.
     */
    rtl_math_ConversionStatus_Ok,

    /** Conversion caused overflow or underflow.
     */
    rtl_math_ConversionStatus_OutOfRange,

    /** @cond INTERNAL */
    rtl_math_ConversionStatus_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
    /** @endcond */
};

/** Rounding modes for rtl_math_round.
 */
enum rtl_math_RoundingMode
{
    /** Like HalfUp, but corrects roundoff errors, preferred.
     */
    rtl_math_RoundingMode_Corrected,

    /** Floor of absolute value, signed return (commercial).
     */
    rtl_math_RoundingMode_Down,

    /** Ceil of absolute value, signed return (commercial).
     */
    rtl_math_RoundingMode_Up,

    /** Floor of signed value.
     */
    rtl_math_RoundingMode_Floor,

    /** Ceil of signed value.
     */
    rtl_math_RoundingMode_Ceiling,

    /** Frac <= 0.5 ? floor of abs : ceil of abs, signed return.
     */
    rtl_math_RoundingMode_HalfDown,

    /** Frac < 0.5 ? floor of abs : ceil of abs, signed return (mathematical).
     */
    rtl_math_RoundingMode_HalfUp,

    /** IEEE rounding mode (statistical).
     */
    rtl_math_RoundingMode_HalfEven,

    /** @cond INTERNAL */
    rtl_math_RoundingMode_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
    /** @endcond */
};

/** Special decimal places constants for rtl_math_doubleToString and
    rtl_math_doubleToUString and rtl_math_doubleToUStringBuffer.
 */
enum rtl_math_DecimalPlaces
{
    /** Value to be used with rtl_math_StringFormat_Automatic.
     */
    rtl_math_DecimalPlaces_Max = 0x7ffffff,

    /** Value to be used with rtl_math_StringFormat_G.
        In fact the same value as rtl_math_DecimalPlaces_Max, just an alias for
        better understanding.
     */
    rtl_math_DecimalPlaces_DefaultSignificance = rtl_math_DecimalPlaces_Max
};


/** Conversions analogous to sprintf() using internal rounding.

    +/-HUGE_VAL are converted to "INF" and "-INF", NAN values are
    converted to "NaN".

    @param pResult
    Returns the resulting byte string.  Must itself not be null, and must point
    to either null or a valid string.

    @param pResultCapacity
    If null, pResult is considered to point to immutable strings, and a new
    string will be allocated in pResult.
    If non-null, it points to the current capacity of pResult, which is
    considered to point to a string buffer (pResult must not itself be null in
    this case, and must point to a string that has room for the given capacity).
    The string representation of the given double value is inserted into pResult
    at position nResultOffset.  If pResult's current capacity is too small, a
    new string buffer will be allocated in pResult as necessary, and
    pResultCapacity will contain the new capacity on return.

    @param nResultOffset
    If pResult is used as a string buffer (i.e., pResultCapacity is non-null),
    nResultOffset specifies the insertion offset within the buffer.  Ignored
    otherwise.

    @param fValue
    The value to convert.

    @param eFormat
    The format to use, one of rtl_math_StringFormat.

    @param nDecPlaces
    The number of decimals to be generated.  Effectively fValue is rounded at
    this position, specifying nDecPlaces <= 0 accordingly rounds the value
    before the decimal point and fills with zeros.
    If eFormat == rtl_math_StringFormat_Automatic and nDecPlaces ==
    rtl_math_DecimalPlaces_Max, the highest number of significant decimals
    possible is generated.
    If eFormat == rtl_math_StringFormat_G, nDecPlaces specifies the number of
    significant digits instead.  If nDecPlaces ==
    rtl_math_DecimalPlaces_DefaultSignificance, the default number (currently 6
    as implemented by most libraries) of significant digits is generated.
    According to the ANSI C90 standard the E style will be used only if the
    exponent resulting from the conversion is less than -4 or greater than or
    equal to the precision.  However, as opposed to the ANSI standard, trailing
    zeros are not necessarily removed from the fractional portion of the result
    unless bEraseTrailingDecZeros == true was specified.

    @param cDecSeparator
    The decimal separator.

    @param pGroups
    Either null (no grouping is used), or a null-terminated list of group
    lengths.  Each group length must be strictly positive.  If the number of
    digits in a conversion exceeds the specified range, the last (highest) group
    length is repeated as needed.  Values are applied from right to left, for a
    grouping of 1,00,00,000 you'd have to specify pGroups={3,2,0}.

    @param cGroupSeparator
    The group separator.  Ignored if pGroups is null.

    @param bEraseTrailingDecZeros
    Trailing zeros in decimal places are erased.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_math_doubleToString(rtl_String ** pResult,
                                      sal_Int32 * pResultCapacity,
                                      sal_Int32 nResultOffset, double fValue,
                                      enum rtl_math_StringFormat eFormat,
                                      sal_Int32 nDecPlaces,
                                      sal_Char cDecSeparator,
                                      sal_Int32 const * pGroups,
                                      sal_Char cGroupSeparator,
                                      sal_Bool bEraseTrailingDecZeros)
    SAL_THROW_EXTERN_C();

/** Conversions analogous to sprintf() using internal rounding.

    +/-HUGE_VAL are converted to "INF" and "-INF", NAN values are
    converted to "NaN".

    @param pResult
    Returns the resulting Unicode string.  Must itself not be null, and must
    point to either null or a valid string.

    @param pResultCapacity
    If null, pResult is considered to point to immutable strings, and a new
    string will be allocated in pResult.
    If non-null, it points to the current capacity of pResult, which is
    considered to point to a string buffer (pResult must not itself be null in
    this case, and must point to a string that has room for the given capacity).
    The string representation of the given double value is inserted into pResult
    at position nResultOffset.  If pResult's current capacity is too small, a
    new string buffer will be allocated in pResult as necessary, and
    pResultCapacity will contain the new capacity on return.

    @param nResultOffset
    If pResult is used as a string buffer (i.e., pResultCapacity is non-null),
    nResultOffset specifies the insertion offset within the buffer.  Ignored
    otherwise.

    @param fValue
    The value to convert.

    @param eFormat
    The format to use, one of rtl_math_StringFormat.

    @param nDecPlaces
    The number of decimals to be generated.  Effectively fValue is rounded at
    this position, specifying nDecPlaces <= 0 accordingly rounds the value
    before the decimal point and fills with zeros.
    If eFormat == rtl_math_StringFormat_Automatic and nDecPlaces ==
    rtl_math_DecimalPlaces_Max, the highest number of significant decimals
    possible is generated.
    If eFormat == rtl_math_StringFormat_G, nDecPlaces specifies the number of
    significant digits instead.  If nDecPlaces ==
    rtl_math_DecimalPlaces_DefaultSignificance, the default number (currently 6
    as implemented by most libraries) of significant digits is generated.
    According to the ANSI C90 standard the E style will be used only if the
    exponent resulting from the conversion is less than -4 or greater than or
    equal to the precision.  However, as opposed to the ANSI standard, trailing
    zeros are not necessarily removed from the fractional portion of the result
    unless bEraseTrailingDecZeros == true was specified.

    @param cDecSeparator
    The decimal separator.

    @param pGroups
    Either null (no grouping is used), or a null-terminated list of group
    lengths.  Each group length must be strictly positive.  If the number of
    digits in a conversion exceeds the specified range, the last (highest) group
    length is repeated as needed.  Values are applied from right to left, for a
    grouping of 1,00,00,000 you'd have to specify pGroups={3,2,0}.

    @param cGroupSeparator
    The group separator.  Ignored if pGroups is null.

    @param bEraseTrailingDecZeros
    Trailing zeros in decimal places are erased.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_math_doubleToUString(rtl_uString ** pResult,
                                       sal_Int32 * pResultCapacity,
                                       sal_Int32 nResultOffset, double fValue,
                                       enum rtl_math_StringFormat eFormat,
                                       sal_Int32 nDecPlaces,
                                       sal_Unicode cDecSeparator,
                                       sal_Int32 const * pGroups,
                                       sal_Unicode cGroupSeparator,
                                       sal_Bool bEraseTrailingDecZeros)
    SAL_THROW_EXTERN_C();

/** Conversion analogous to strtod(), convert a string representing a
    decimal number into a double value.

    Leading tabs (0x09) and spaces (0x20) are eaten.  Overflow returns
    +/-HUGE_VAL, underflow 0.  In both cases pStatus is set to
    rtl_math_ConversionStatus_OutOfRange, otherwise to
    rtl_math_ConversionStatus_Ok.  "INF", "-INF" and "+/-1.#INF" are
    recognized as +/-HUGE_VAL, pStatus is set to
    rtl_math_ConversionStatus_OutOfRange.  "NaN" and "+/-1.#NAN" are
    recognized and the value is set to +/-NAN, pStatus is set to
    rtl_math_ConversionStatus_Ok.

    @param pBegin
    Points to the start of the byte string to convert.  Must not be null.

    @param pEnd
    Points one past the end of the byte string to convert.  The condition
    pEnd >= pBegin must hold.

    @param cDecSeparator
    The decimal separator.

    @param cGroupSeparator
    The group (aka thousands) separator.

    @param pStatus
    If non-null, returns the status of the conversion.

    @param pParsedEnd
    If non-null, returns one past the position of the last character parsed
    away.  Thus if [pBegin..pEnd) only contains the numerical string to be
    parsed, *pParsedEnd == pEnd on return.  If no numerical (sub-)string is
    found, *pParsedEnd == pBegin on return, even if there was leading
    whitespace.
 */
SAL_DLLPUBLIC double SAL_CALL rtl_math_stringToDouble(
    sal_Char const * pBegin, sal_Char const * pEnd, sal_Char cDecSeparator,
    sal_Char cGroupSeparator, enum rtl_math_ConversionStatus * pStatus,
    sal_Char const ** pParsedEnd) SAL_THROW_EXTERN_C();

/** Conversion analogous to strtod(), convert a string representing a
    decimal number into a double value.

    Leading tabs (U+0009) and spaces (U+0020) are eaten.  Overflow returns
    +/-HUGE_VAL, underflow 0.  In both cases pStatus is set to
    rtl_math_ConversionStatus_OutOfRange, otherwise to
    rtl_math_ConversionStatus_Ok.  "INF", "-INF" and "+/-1.#INF" are
    recognized as +/-HUGE_VAL, pStatus is set to
    rtl_math_ConversionStatus_OutOfRange.  "NaN" and "+/-1.#NAN" are
    recognized and the value is set to +/-NAN, pStatus is set to
    rtl_math_ConversionStatus_Ok.

    @param pBegin
    Points to the start of the Unicode string to convert.  Must not be null.

    @param pEnd
    Points one past the end of the Unicode string to convert.  The condition
    pEnd >= pBegin must hold.

    @param cDecSeparator
    The decimal separator.

    @param cGroupSeparator
    The group (aka thousands) separator.

    @param pStatus
    If non-null, returns the status of the conversion.

    @param pParsedEnd
    If non-null, returns one past the position of the last character parsed
    away.  Thus if [pBegin..pEnd) only contains the numerical string to be
    parsed, *pParsedEnd == pEnd on return.  If no numerical (sub-)string is
    found, *pParsedEnd == pBegin on return, even if there was leading
    whitespace.
 */
SAL_DLLPUBLIC double SAL_CALL rtl_math_uStringToDouble(
    sal_Unicode const * pBegin, sal_Unicode const * pEnd,
    sal_Unicode cDecSeparator, sal_Unicode cGroupSeparator,
    enum rtl_math_ConversionStatus * pStatus, sal_Unicode const ** pParsedEnd)
    SAL_THROW_EXTERN_C();

/** Rounds a double value.

    @param fValue
    Specifies the value to be rounded.

    @param nDecPlaces
    Specifies the decimal place where rounding occurs.  Must be in the range
    -20 to +20, inclusive.  Negative if rounding occurs before the decimal
    point.

    @param eMode
    Specifies the rounding mode.
 */
SAL_DLLPUBLIC double SAL_CALL rtl_math_round(double fValue, int nDecPlaces,
                               enum rtl_math_RoundingMode eMode)
    SAL_THROW_EXTERN_C();

/** Scales fVal to a power of 10 without calling pow() or div() for nExp values
    between -16 and +16, providing a faster method.

    @param fValue
    The value to be raised.

    @param nExp
    The exponent.

    @return
    fVal * pow(10.0, nExp)
 */
SAL_DLLPUBLIC double SAL_CALL rtl_math_pow10Exp(double fValue, int nExp) SAL_THROW_EXTERN_C();

/** Rounds value to 15 significant decimal digits.

    @param fValue
    The value to be rounded.
  */
SAL_DLLPUBLIC double SAL_CALL rtl_math_approxValue(double fValue) SAL_THROW_EXTERN_C();

/** Test equality of two values with an accuracy of the magnitude of the
    given values scaled by 2^-48 (4 bits roundoff stripped).

    @attention
    approxEqual( value!=0.0, 0.0 ) _never_ yields true.

    @since LibreOffice 5.3
 */
SAL_DLLPUBLIC bool SAL_CALL rtl_math_approxEqual(double a, double b) SAL_THROW_EXTERN_C();

/** Returns more accurate e^x-1 for x near 0 than calculating directly.

    expm1 is part of the C99 standard, but not provided by some compilers.

    @param fValue
    The value x in the term e^x-1.
  */
SAL_DLLPUBLIC double SAL_CALL rtl_math_expm1(double fValue) SAL_THROW_EXTERN_C();

/** Returns more accurate log(1+x) for x near 0 than calculating directly.

    log1p is part of the C99 standard, but not provided by some compilers.

    @param fValue
    The value x in the term log(1+x).
  */
SAL_DLLPUBLIC double SAL_CALL rtl_math_log1p(double fValue) SAL_THROW_EXTERN_C();

/** Returns more accurate atanh(x) for x near 0 than calculating
    0.5*log((1+x)/(1-x)).

    atanh is part of the C99 standard, but not provided by some compilers.

    @param fValue
    The value x in the term atanh(x).
  */
SAL_DLLPUBLIC double SAL_CALL rtl_math_atanh(double fValue) SAL_THROW_EXTERN_C();

/** Returns values of the Errorfunction erf.

    erf is part of the C99 standard, but not provided by some compilers.

    @param fValue
    The value x in the term erf(x).
  */
SAL_DLLPUBLIC double SAL_CALL rtl_math_erf(double fValue) SAL_THROW_EXTERN_C();

/** Returns values of the complement Errorfunction erfc.

    erfc is part of the C99 standard, but not provided by some compilers.

    @param fValue
    The value x in the term erfc(x).
  */
SAL_DLLPUBLIC double SAL_CALL rtl_math_erfc(double fValue) SAL_THROW_EXTERN_C();

/** Returns values of the inverse hyperbolic sine.

    asinh is part of the C99 standard, but not provided by some compilers.

    @param fValue
    The value x in the term asinh(x).
  */
SAL_DLLPUBLIC double SAL_CALL rtl_math_asinh(double fValue) SAL_THROW_EXTERN_C();

/** Returns values of the inverse hyperbolic cosine.

    acosh is part of the C99 standard, but not provided by some compilers.

    @param fValue
    The value x in the term acosh(x).
  */
SAL_DLLPUBLIC double SAL_CALL rtl_math_acosh(double fValue) SAL_THROW_EXTERN_C();

#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDED_RTL_MATH_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
