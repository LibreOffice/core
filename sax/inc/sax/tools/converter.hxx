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

#ifndef _SAX_CONVERTER_HXX
#define _SAX_CONVERTER_HXX

#include "sax/dllapi.h"

#include <sal/types.h>

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/util/MeasureUnit.hpp>


namespace rtl
{
class OUString;
class OUStringBuffer;
}

namespace com { namespace sun { namespace star {
    namespace util {
        struct Date;
        struct DateTime;
        struct Duration;
    }
} } }

namespace sax {

/** the Converter converts values of various types from
    their internal represantation to the textual form used in xml
    and back.

    All unit types are expressed as com::sun::star::util::MeasureUnit
*/


class SAX_DLLPUBLIC Converter
{
public:
    /** convert string to measure using optional min and max values*/
    static bool convertMeasure( sal_Int32& rValue,
                                const ::rtl::OUString& rString,
                                sal_Int16 nTargetUnit = ::com::sun::star::util::MeasureUnit::MM_100TH,
                                sal_Int32 nMin = SAL_MIN_INT32,
                                sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert measure to string */
    static void convertMeasure( ::rtl::OUStringBuffer& rBuffer,
                                sal_Int32 nMeasure,
                                sal_Int16 SourceUnit = ::com::sun::star::util::MeasureUnit::MM_100TH,
                                sal_Int16 nTargetUnit = ::com::sun::star::util::MeasureUnit::INCH  );

    /** convert string to boolean */
    static bool convertBool( bool& rBool,
                             const ::rtl::OUString& rString );

    /** convert boolean to string */
    static void convertBool( ::rtl::OUStringBuffer& rBuffer,
                             bool bValue );

    /** convert string to percent */
    static bool convertPercent( sal_Int32& rValue,
                                const ::rtl::OUString& rString );

    /** convert percent to string */
    static void convertPercent( ::rtl::OUStringBuffer& rBuffer,
                                sal_Int32 nValue );

    /** convert string to pixel measure unite */
    static bool convertMeasurePx( sal_Int32& rValue,
                                  const ::rtl::OUString& rString );

    /** convert pixel measure unit to string */
    static void convertMeasurePx( ::rtl::OUStringBuffer& rBuffer,
                                  sal_Int32 nValue );

    /** convert string to color */
    static bool convertColor( sal_Int32& rColor,
                              const ::rtl::OUString&rValue );

    /** convert color to string */
    static void convertColor( ::rtl::OUStringBuffer &rBuffer,
                              sal_Int32 nColor );

    /** convert number to string */
    static void convertNumber( ::rtl::OUStringBuffer& rBuffer,
                               sal_Int32 nNumber );

    /** convert string to number with optional min and max values */
    static bool convertNumber( sal_Int32& rValue,
                               const ::rtl::OUString& rString,
                               sal_Int32 nMin = SAL_MIN_INT32,
                               sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert double number to string (using ::rtl::math) and
        DO convert from source unit to target unit */
    static void convertDouble( ::rtl::OUStringBuffer& rBuffer,
                               double fNumber,
                               bool bWriteUnits,
                               sal_Int16 nSourceUnit,
                               sal_Int16 nTargetUnit );

    /** convert double number to string (using ::rtl::math) without unit conversion */
    static void convertDouble( ::rtl::OUStringBuffer& rBuffer, double fNumber);

    /** convert string to double number (using ::rtl::math) and DO convert from
        source unit to target unit. */
    static bool convertDouble(  double& rValue,
                                const ::rtl::OUString& rString,
                                sal_Int16 nSourceUnit,
                                sal_Int16 nTargetUnit );

    /** convert string to double number (using ::rtl::math) without unit conversion */
    static bool convertDouble(double& rValue, const ::rtl::OUString& rString);

    /** convert string to double number (using ::rtl::math) with unit conversion */
    static bool convertDouble(double& rValue, const ::rtl::OUString& rString, sal_Int16 nTargetUnit );

    /** convert double to ISO "duration" string; negative durations allowed */
    static void convertDuration(::rtl::OUStringBuffer& rBuffer,
                                const double fTime);

    /** convert util::Duration to ISO "duration" string */
    static void convertDuration(::rtl::OUStringBuffer& rBuffer,
                        const ::com::sun::star::util::Duration& rDuration);

    /** convert ISO "duration" string to double; negative durations allowed */
    static bool convertDuration(double & rfTime,
                                const ::rtl::OUString& rString);

    /** convert ISO "duration" string to util::Duration */
    static bool convertDuration(::com::sun::star::util::Duration& rDuration,
                        const ::rtl::OUString& rString);

    /** convert util::Date to ISO "date" string */
    static void convertDate( ::rtl::OUStringBuffer& rBuffer,
                    const com::sun::star::util::Date& rDate );

    /** convert util::DateTime to ISO "date" or "dateTime" string */
    static void convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const com::sun::star::util::DateTime& rDateTime,
                                   bool bAddTimeIf0AM = false );

    /** convert ISO "date" or "dateTime" string to util::DateTime */
    static bool convertDateTime( com::sun::star::util::DateTime& rDateTime,
                                 const ::rtl::OUString& rString );

    /** convert ISO "date" or "dateTime" string to util::DateTime or
        util::Date */
    static bool convertDateOrDateTime(
                    com::sun::star::util::Date & rDate,
                    com::sun::star::util::DateTime & rDateTime,
                    bool & rbDateTime,
                    const ::rtl::OUString & rString );

    /** gets the position of the first comma after npos in the string
        rStr. Commas inside '"' pairs are not matched */
    static sal_Int32 indexOfComma( const ::rtl::OUString& rStr,
                                   sal_Int32 nPos );

    /** encodes the given byte sequence into Base64 */
    static void encodeBase64(rtl::OUStringBuffer& aStrBuffer, const com::sun::star::uno::Sequence<sal_Int8>& aPass);

    // Decode a base 64 encoded string into a sequence of bytes. The first
    // version can be used for attribute values only, bacause it does not
    // return any chars left from conversion.
    // For text submitted throgh the SAX characters call, the later method
    // must be used!
    static void decodeBase64(com::sun::star::uno::Sequence<sal_Int8>& aPass, const rtl::OUString& sBuffer);

    static sal_Int32 decodeBase64SomeChars(com::sun::star::uno::Sequence<sal_Int8>& aPass, const rtl::OUString& sBuffer);

    static void clearUndefinedChars(rtl::OUString& rTarget, const rtl::OUString& rSource);

    static double GetConversionFactor(::rtl::OUStringBuffer& rUnit, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit);
    static sal_Int16 GetUnitFromString(const ::rtl::OUString& rString, sal_Int16 nDefaultUnit);

};

}

#endif  //  _SAX_CONVERTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
