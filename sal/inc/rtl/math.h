/*************************************************************************
 *
 *  $RCSfile: math.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 16:45:48 $
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

#if !defined INCLUDED_RTL_MATH_H
#define INCLUDED_RTL_MATH_H

#include "rtl/ustring.h"
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

    /** @internal
     */
    rtl_math_StringFormat_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
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

    /** @internal
     */
    rtl_math_ConversionStatus_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
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

    /** @internal
     */
    rtl_math_RoundingMode_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
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
    rtl_math_DecimalPlaces_DefaultSignificance = 0x7ffffff
};


/** Conversions analogous to sprintf() using internal rounding.

    +/-HUGE_VAL are converted to "1.#INF" and "-1.#INF", NAN values are
    converted to "1.#NAN" and "-1.#NAN", of course using cDecSeparator instead
    of '.'.

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
void SAL_CALL rtl_math_doubleToString(rtl_String ** pResult,
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

    +/-HUGE_VAL are converted to "1.#INF" and "-1.#INF", NAN values are
    converted to "1.#NAN" and "-1.#NAN", of course using cDecSeparator instead
    of '.'.

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
void SAL_CALL rtl_math_doubleToUString(rtl_uString ** pResult,
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
    rtl_math_ConversionStatus_Ok.  "+/-1.#INF" is recognized as +/-HUGE_VAL,
    pStatus is set to rtl_math_ConversionStatus_OutOfRange.  "+/-1.#NAN" is
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
    parsed, *pParsedEnd == pEnd on return.
 */
double SAL_CALL rtl_math_stringToDouble(
    sal_Char const * pBegin, sal_Char const * pEnd, sal_Char cDecSeparator,
    sal_Char cGroupSeparator, enum rtl_math_ConversionStatus * pStatus,
    sal_Char const ** pParsedEnd) SAL_THROW_EXTERN_C();

/** Conversion analogous to strtod(), convert a string representing a
    decimal number into a double value.

    Leading tabs (U+0009) and spaces (U+0020) are eaten.  Overflow returns
    +/-HUGE_VAL, underflow 0.  In both cases pStatus is set to
    rtl_math_ConversionStatus_OutOfRange, otherwise to
    rtl_math_ConversionStatus_Ok.  "+/-1.#INF" is recognized as +/-HUGE_VAL,
    pStatus is set to rtl_math_ConversionStatus_OutOfRange.  "+/-1.#NAN" is
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
    parsed, *pParsedEnd == pEnd on return.
 */
double SAL_CALL rtl_math_uStringToDouble(
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
double SAL_CALL rtl_math_round(double fValue, int nDecPlaces,
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
double SAL_CALL rtl_math_pow10Exp(double fValue, int nExp) SAL_THROW_EXTERN_C();

#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDED_RTL_MATH_H */
