/*************************************************************************
 *
 *  $RCSfile: math.hxx,v $
 *
 *  $Revision: 1.3 $
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

#if !defined INCLUDED_RTL_MATH_HXX
#define INCLUDED_RTL_MATH_HXX

#include "rtl/math.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "sal/mathconf.h"
#include "sal/types.h"

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
    rtl_math_doubleToString(&aResult.pData, 0, 0, fValue, eFormat, nDecPlaces,
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
    rtl_math_doubleToString(&aResult.pData, 0, 0, fValue, eFormat, nDecPlaces,
                            cDecSeparator, 0, 0, bEraseTrailingDecZeros);
    return aResult;
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
    rtl_math_doubleToUString(&aResult.pData, 0, 0, fValue, eFormat, nDecPlaces,
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
    rtl_math_doubleToUString(&aResult.pData, 0, 0, fValue, eFormat, nDecPlaces,
                             cDecSeparator, 0, 0, bEraseTrailingDecZeros);
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
                              eFormat, nDecPlaces, cDecSeparator, 0, 0,
                              bEraseTrailingDecZeros);
}

/** A wrapper around rtl_math_stringToDouble.
 */
inline double stringToDouble(rtl::OString const & rString,
                             sal_Char cDecSeparator, sal_Char cGroupSeparator,
                             rtl_math_ConversionStatus * pStatus,
                             sal_Int32 * pParsedEnd)
{
    sal_Char const * pBegin = rString.getStr();
    sal_Char const * pEnd;
    double fResult = rtl_math_stringToDouble(pBegin,
                                             pBegin + rString.getLength(),
                                             cDecSeparator, cGroupSeparator,
                                             pStatus, &pEnd);
    if (pParsedEnd != 0)
        *pParsedEnd = pEnd - pBegin;
    return fResult;
}

/** A wrapper around rtl_math_uStringToDouble.
 */
inline double stringToDouble(rtl::OUString const & rString,
                             sal_Unicode cDecSeparator,
                             sal_Unicode cGroupSeparator,
                             rtl_math_ConversionStatus * pStatus,
                             sal_Int32 * pParsedEnd)
{
    sal_Unicode const * pBegin = rString.getStr();
    sal_Unicode const * pEnd;
    double fResult = rtl_math_uStringToDouble(pBegin,
                                              pBegin + rString.getLength(),
                                              cDecSeparator, cGroupSeparator,
                                              pStatus, &pEnd);
    if (pParsedEnd != 0)
        *pParsedEnd = pEnd - pBegin;
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

/** Test equality of two values with an accuracy of the magnitude of the
    given values scaled by 2^-48 (4 bits roundoff stripped).

    @ATTENTION
    ApproxEqual( value!=0.0, 0.0 ) _never_ yields true.
 */
inline bool approxEqual(double a, double b)
{
    if ( a == b )
        return true;
    double x = a - b;
    return (x < 0.0 ? -x : x)
        < ((a < 0.0 ? -a : a) * (1.0 / (16777216.0 * 16777216.0)));
}

/** Add two values.

    If signs differ and the absolute values are equal according to ApproxEqual()
    the method returns 0.0 instead of calculating the sum.

    If you wanted to sum up multiple values it would be convenient not to call
    ApproxAdd() for each value but instead remember the first value not equal to
    0.0, add all other values using normal + operator, and with the result and
    the remembered value call ApproxAdd().
 */
inline double approxAdd(double a, double b)
{
    if ( ((a < 0.0 && b > 0.0) || (b < 0.0 && a > 0.0))
         && approxEqual( a, -b ) )
        return 0.0;
    return a + b;
}

/** Substract two values (a-b).

    If signs are identical and the values are equal according to ApproxEqual()
    the method returns 0.0 instead of calculating the substraction.
 */
inline double approxSub(double a, double b)
{
    if ( ((a < 0.0 && b < 0.0) || (a > 0.0 && b > 0.0)) && approxEqual( a, b ) )
        return 0.0;
    return a - b;
}

/** floor() method taking ApproxEqual() into account.

    Use for expected integer values being calculated by double functions.

    @ATTENTION
    The threshhold value is 3.55271e-015
 */
inline double approxFloor(double a)
{
    double b = floor( a );
    if ( approxEqual( a - 1.0, b ) )
        return b + 1.0;
    return b;
}

/** ceil() method taking ApproxEqual() into account.

    Use for expected integer values being calculated by double functions.

    @ATTENTION
    The threshhold value is 3.55271e-015
 */
inline double approxCeil(double a)
{
    double b = ceil( a );
    if ( approxEqual( a + 1.0, b ) )
        return b - 1.0;
    return b;
}

/** Tests whether a value is neither INF nor NAN.
 */
inline bool isFinite(double d)
{
    return SAL_MATH_FINITE(d) != 0;
}

/** If a value represents +INF or -INF.

    The sign bit may be queried with isSignBitSet().

    If isFinite(d)==false and isInf(d)==false then NAN.
 */
inline bool isInf(double d)
{
    // exponent==0x7ff fraction==0
    return (SAL_MATH_FINITE(d) == 0) &&
        (reinterpret_cast< sal_math_Double * >(&d)->inf_parts.fraction_hi == 0)
        && (reinterpret_cast< sal_math_Double * >(&d)->inf_parts.fraction_lo
            == 0);
}

/** Test on any QNAN or SNAN.
 */
inline bool isNan(double d)
{
    // exponent==0x7ff fraction!=0
    return (SAL_MATH_FINITE(d) == 0) && (
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
    reinterpret_cast< sal_math_Double * >(pd)->w32_parts.msw
        = bNegative ? 0xFFF00000 : 0x7FF00000;
    reinterpret_cast< sal_math_Double * >(pd)->w32_parts.lsw = 0;
}

/** Set a QNAN.
 */
inline void setNan(double * pd)
{
    reinterpret_cast< sal_math_Double * >(pd)->w32_parts.msw = 0x7FFFFFFF;
    reinterpret_cast< sal_math_Double * >(pd)->w32_parts.lsw = 0xFFFFFFFF;
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
