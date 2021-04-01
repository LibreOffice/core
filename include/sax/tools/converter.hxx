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

#ifndef INCLUDED_SAX_TOOLS_CONVERTER_HXX
#define INCLUDED_SAX_TOOLS_CONVERTER_HXX

#include <sal/config.h>

#include <optional>

#include <sax/saxdllapi.h>

#include <sal/types.h>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <tools/color.hxx>
#include <unotools/saveopt.hxx>

namespace com::sun::star {
    namespace uno {
        class Any;
    }
    namespace util {
        struct Date;
        struct DateTime;
        struct Duration;
    }
}

namespace sax {

/** the Converter converts values of various types from
    their internal representation to the textual form used in xml
    and back.

    All unit types are expressed as css::util::MeasureUnit
*/


class SAX_DLLPUBLIC Converter
{
public:
    /** convert string to measure using optional min and max values*/
    static bool convertMeasure( sal_Int32& rValue,
                                std::u16string_view rString,
                                sal_Int16 nTargetUnit = css::util::MeasureUnit::MM_100TH,
                                sal_Int32 nMin = SAL_MIN_INT32,
                                sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert string to measure using optional min and max values*/
    static bool convertMeasure( sal_Int32& rValue,
                                std::string_view rString,
                                sal_Int16 nTargetUnit = css::util::MeasureUnit::MM_100TH,
                                sal_Int32 nMin = SAL_MIN_INT32,
                                sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert measure to string */
    static void convertMeasure( OUStringBuffer& rBuffer,
                                sal_Int32 nMeasure,
                                sal_Int16 SourceUnit,
                                sal_Int16 nTargetUnit  );

    /** convert string to boolean */
    static bool convertBool( bool& rBool,
                             std::u16string_view rString );

    /** convert string to boolean */
    static bool convertBool( bool& rBool,
                             std::string_view rString );

    /** convert boolean to string */
    static void convertBool( OUStringBuffer& rBuffer,
                             bool bValue );

    /** convert string to percent */
    static bool convertPercent( sal_Int32& rValue,
                                std::u16string_view rString );

    /** convert string to percent */
    static bool convertPercent( sal_Int32& rValue,
                                std::string_view rString );

    /** convert percent to string */
    static void convertPercent( OUStringBuffer& rBuffer,
                                sal_Int32 nValue );

    /** convert string to pixel measure unit */
    static bool convertMeasurePx( sal_Int32& rValue,
                                  std::u16string_view rString );

    /** convert string to pixel measure unit */
    static bool convertMeasurePx( sal_Int32& rValue,
                                  std::string_view rString );

    /** convert pixel measure unit to string */
    static void convertMeasurePx( OUStringBuffer& rBuffer,
                                  sal_Int32 nValue );

    /** convert string to rgb color */
    static bool convertColor( sal_Int32& rColor,
                              std::u16string_view rValue );
    static bool convertColor( sal_Int32& rColor,
                              std::string_view rValue );
    static bool convertColor( ::Color& rColor,
                              std::u16string_view rValue )
    {
        sal_Int32 n(rColor);
        bool b = convertColor( n, rValue );
        if (b) rColor = Color(ColorTransparency, n);
        return b;
    }
    static bool convertColor( ::Color& rColor,
                              std::string_view rValue )
    {
        sal_Int32 n(rColor);
        bool b = convertColor( n, rValue );
        if (b) rColor = Color(ColorTransparency, n);
        return b;
    }

    /** convert color to string */
    static void convertColor( OUStringBuffer &rBuffer,
                              sal_Int32 nColor );
    static void convertColor( OUStringBuffer &rBuffer,
                              ::Color nColor )
    { convertColor( rBuffer, sal_Int32(nColor) ); }

    /** convert string to number with optional min and max values */
    static bool convertNumber( sal_Int32& rValue,
                               std::u16string_view aString,
                               sal_Int32 nMin = SAL_MIN_INT32,
                               sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert string to number with optional min and max values */
    static bool convertNumber( sal_Int32& rValue,
                               std::string_view aString,
                               sal_Int32 nMin = SAL_MIN_INT32,
                               sal_Int32 nMax = SAL_MAX_INT32 );

    /** convert string to number with optional min and max values */
    static bool convertNumber64(sal_Int64& rValue,
                                std::u16string_view aString,
                                sal_Int64 nMin = SAL_MIN_INT64,
                                sal_Int64 nMax = SAL_MAX_INT64);

    /** convert string to number with optional min and max values */
    static bool convertNumber64(sal_Int64& rValue,
                                std::string_view aString,
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
                                std::u16string_view rString,
                                sal_Int16 nSourceUnit,
                                sal_Int16 nTargetUnit );

    /** convert string to double number (using ::rtl::math) without unit conversion */
    static bool convertDouble(double& rValue, std::u16string_view rString);

    /** convert string to double number (using ::rtl::math) without unit conversion */
    static bool convertDouble(double& rValue, std::string_view rString);

    /** convert number, 10th of degrees with range [0..3600] to SVG angle */
    static void convertAngle(OUStringBuffer& rBuffer, sal_Int16 nAngle,
            SvtSaveOptions::ODFSaneDefaultVersion nVersion);

    /** convert SVG angle to number, 10th of degrees with range [0..3600] */
    static bool convertAngle(sal_Int16& rAngle, std::u16string_view rString,
            bool isWrongOOo10thDegAngle);

    /** convert SVG angle to number, 10th of degrees with range [0..3600] */
    static bool convertAngle(sal_Int16& rAngle, std::string_view rString,
            bool isWrongOOo10thDegAngle);

    /** convert double to XMLSchema-2 "duration" string; negative durations allowed */
    static void convertDuration(OUStringBuffer& rBuffer,
                                const double fTime);

    /** convert util::Duration to XMLSchema-2 "duration" string */
    static void convertDuration(OUStringBuffer& rBuffer,
                        const css::util::Duration& rDuration);

    /** convert XMLSchema-2 "duration" string to double; negative durations allowed */
    static bool convertDuration(double & rfTime,
                                std::u16string_view rString);

    /** convert XMLSchema-2 "duration" string to double; negative durations allowed */
    static bool convertDuration(double & rfTime,
                                std::string_view rString);

    /** convert XMLSchema-2 "duration" string to util::Duration */
    static bool convertDuration(css::util::Duration& rDuration,
                        std::u16string_view rString);

    /** convert util::Date to XMLSchema-2 "date" string */
    static void convertDate( OUStringBuffer& rBuffer,
                    const css::util::Date& rDate,
                    sal_Int16 const* pTimeZoneOffset);

    /** convert util::DateTime to XMLSchema-2 "date" or "dateTime" string */
    static void convertDateTime( OUStringBuffer& rBuffer,
                                const css::util::DateTime& rDateTime,
                                 sal_Int16 const* pTimeZoneOffset,
                                   bool bAddTimeIf0AM = false );

    /** convert util::DateTime to XMLSchema-2 "time" or "dateTime" string */
    static void convertTimeOrDateTime(OUStringBuffer& rBuffer,
                            const css::util::DateTime& rDateTime);

    /** convert XMLSchema-2 "date" or "dateTime" string to util::DateTime */
    static bool parseDateTime( css::util::DateTime& rDateTime,
                               std::u16string_view rString );

    /** convert XMLSchema-2 "date" or "dateTime" string to util::DateTime */
    static bool parseDateTime( css::util::DateTime& rDateTime,
                               std::string_view rString );

    /** convert XMLSchema-2 "time" or "dateTime" string to util::DateTime */
    static bool parseTimeOrDateTime(css::util::DateTime& rDateTime,
                                 std::u16string_view rString);

    /** convert XMLSchema-2 "time" or "dateTime" string to util::DateTime */
    static bool parseTimeOrDateTime(css::util::DateTime& rDateTime,
                                 std::string_view rString);

    /** convert XMLSchema-2 "date" or "dateTime" string to util::DateTime or
        util::Date */
    static bool parseDateOrDateTime(
                    css::util::Date * pDate,
                    css::util::DateTime & rDateTime,
                    bool & rbDateTime,
                    std::optional<sal_Int16> * pTimeZoneOffset,
                    std::u16string_view rString );

    /** convert XMLSchema-2 "date" or "dateTime" string to util::DateTime or
        util::Date */
    static bool parseDateOrDateTime(
                    css::util::Date * pDate,
                    css::util::DateTime & rDateTime,
                    bool & rbDateTime,
                    std::optional<sal_Int16> * pTimeZoneOffset,
                    std::string_view rString );

    /** gets the position of the first comma after npos in the string
        rStr. Commas inside '"' pairs are not matched */
    static sal_Int32 indexOfComma( std::u16string_view rStr,
                                   sal_Int32 nPos );

    static double GetConversionFactor(OUStringBuffer& rUnit, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit);
    static sal_Int16 GetUnitFromString(std::u16string_view rString, sal_Int16 nDefaultUnit);
    static sal_Int16 GetUnitFromString(std::string_view rString, sal_Int16 nDefaultUnit);

    /** convert an Any to string (typesafe) */
    static bool convertAny(OUStringBuffer&          rsValue,
                           OUStringBuffer&          rsType ,
                           const css::uno::Any& rValue);

};

}

#endif // INCLUDED_SAX_TOOLS_CONVERTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
