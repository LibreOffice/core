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

#ifndef _SAX_CONVERTER_HXX
#define _SAX_CONVERTER_HXX

#include "sax/saxdllapi.h"

#include <boost/optional/optional.hpp>

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/util/MeasureUnit.hpp>


namespace com { namespace sun { namespace star {
    namespace uno {
        class Any;
    }
    namespace util {
        struct Date;
        struct DateTime;
        struct DateWithTimezone;
        struct DateTimeWithTimezone;
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
                                const OUString& rString,
                                sal_Int16 nTargetUnit = ::com::sun::star::util::MeasureUnit::MM_100TH,
                                sal_Int32 nMin = SAL_MIN_INT32,
                                sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert measure to string */
    static void convertMeasure( OUStringBuffer& rBuffer,
                                sal_Int32 nMeasure,
                                sal_Int16 SourceUnit = ::com::sun::star::util::MeasureUnit::MM_100TH,
                                sal_Int16 nTargetUnit = ::com::sun::star::util::MeasureUnit::INCH  );

    /** convert string to boolean */
    static bool convertBool( bool& rBool,
                             const OUString& rString );

    /** convert boolean to string */
    static void convertBool( OUStringBuffer& rBuffer,
                             bool bValue );

    /** convert string to percent */
    static bool convertPercent( sal_Int32& rValue,
                                const OUString& rString );

    /** convert percent to string */
    static void convertPercent( OUStringBuffer& rBuffer,
                                sal_Int32 nValue );

    /** convert string to pixel measure unite */
    static bool convertMeasurePx( sal_Int32& rValue,
                                  const OUString& rString );

    /** convert pixel measure unit to string */
    static void convertMeasurePx( OUStringBuffer& rBuffer,
                                  sal_Int32 nValue );

    /** convert string to rgb color */
    static bool convertColor( sal_Int32& rColor,
                              const OUString&rValue );

    /** convert color to string */
    static void convertColor( OUStringBuffer &rBuffer,
                              sal_Int32 nColor );

    /** convert number to string */
    static void convertNumber( OUStringBuffer& rBuffer,
                               sal_Int32 nNumber );

    /** convert string to number with optional min and max values */
    static bool convertNumber( sal_Int32& rValue,
                               const OUString& rString,
                               sal_Int32 nMin = SAL_MIN_INT32,
                               sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert string to number with optional min and max values */
    static bool convertNumber64(sal_Int64& rValue,
                                const OUString& rString,
                                sal_Int64 nMin = SAL_MIN_INT64,
                                sal_Int64 nMax = SAL_MAX_INT64);

    /** convert double number to string (using ::rtl::math) and
        DO convert from source unit to target unit */
    static void convertDouble( OUStringBuffer& rBuffer,
                               double fNumber,
                               bool bWriteUnits,
                               sal_Int16 nSourceUnit,
                               sal_Int16 nTargetUnit );

    /** convert double number to string (using ::rtl::math) without unit conversion */
    static void convertDouble( OUStringBuffer& rBuffer, double fNumber);

    /** convert string to double number (using ::rtl::math) and DO convert from
        source unit to target unit. */
    static bool convertDouble(  double& rValue,
                                const OUString& rString,
                                sal_Int16 nSourceUnit,
                                sal_Int16 nTargetUnit );

    /** convert string to double number (using ::rtl::math) without unit conversion */
    static bool convertDouble(double& rValue, const OUString& rString);

    /** convert double to ISO "duration" string; negative durations allowed */
    static void convertDuration(OUStringBuffer& rBuffer,
                                const double fTime);

    /** convert util::Duration to ISO "duration" string */
    static void convertDuration(OUStringBuffer& rBuffer,
                        const ::com::sun::star::util::Duration& rDuration);

    /** convert ISO "duration" string to double; negative durations allowed */
    static bool convertDuration(double & rfTime,
                                const OUString& rString);

    /** convert ISO "duration" string to util::Duration */
    static bool convertDuration(::com::sun::star::util::Duration& rDuration,
                        const OUString& rString);

    /** convert util::Date to ISO "date" string */
    static void convertDate( OUStringBuffer& rBuffer,
                    const com::sun::star::util::Date& rDate,
                    sal_Int16 const* pTimeZoneOffset);

    /** convert util::DateTime to ISO "date" or "dateTime" string */
    static void convertDateTime( OUStringBuffer& rBuffer,
                                const com::sun::star::util::DateTime& rDateTime,
                                 sal_Int16 const* pTimeZoneOffset,
                                   bool bAddTimeIf0AM = false );

    /** convert ISO "date" or "dateTime" string to util::DateTime */
    static bool parseDateTime( com::sun::star::util::DateTime& rDateTime,
                                 boost::optional<sal_Int16> * pTimeZoneOffset,
                                 const OUString& rString );

    /** convert ISO "date" or "dateTime" string to util::DateTime or
        util::Date */
    static bool parseDateOrDateTime(
                    com::sun::star::util::Date * pDate,
                    com::sun::star::util::DateTime & rDateTime,
                    bool & rbDateTime,
                    boost::optional<sal_Int16> * pTimeZoneOffset,
                    const OUString & rString );

    /** gets the position of the first comma after npos in the string
        rStr. Commas inside '"' pairs are not matched */
    static sal_Int32 indexOfComma( const OUString& rStr,
                                   sal_Int32 nPos );

    /** encodes the given byte sequence into Base64 */
    static void encodeBase64(OUStringBuffer& aStrBuffer, const com::sun::star::uno::Sequence<sal_Int8>& aPass);

    // Decode a base 64 encoded string into a sequence of bytes. The first
    // version can be used for attribute values only, bacause it does not
    // return any chars left from conversion.
    // For text submitted throgh the SAX characters call, the later method
    // must be used!
    static void decodeBase64(com::sun::star::uno::Sequence<sal_Int8>& aPass, const OUString& sBuffer);

    static sal_Int32 decodeBase64SomeChars(com::sun::star::uno::Sequence<sal_Int8>& aPass, const OUString& sBuffer);

    static double GetConversionFactor(OUStringBuffer& rUnit, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit);
    static sal_Int16 GetUnitFromString(const OUString& rString, sal_Int16 nDefaultUnit);

    /** convert an Any to string (typesafe) */
    static bool convertAny(OUStringBuffer&          rsValue,
                           OUStringBuffer&          rsType ,
                           const ::com::sun::star::uno::Any& rValue);

};

}

#endif  //  _SAX_CONVERTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
