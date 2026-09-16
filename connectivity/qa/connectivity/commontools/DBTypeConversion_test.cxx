/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
 * Unit tests for DBTypeConversion string-to-date/time/datetime conversions.
 *
 * These tests cover the fix for bug 153057: filtering on DATE, TIME and
 * TIMESTAMP columns using a string value in the Firebird prepared statement.
 * The fix added handling in OPreparedStatement::setString() for
 * DataType::DATE, DataType::TIME and DataType::TIMESTAMP, relying on
 * DBTypeConversion::toDate(), toTime() and toDateTime().
 */

#include <test/bootstrapfixture.hxx>

#include <connectivity/dbconversion.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>

using namespace ::dbtools;
using namespace ::com::sun::star::util;

namespace connectivity::commontools
{
class DBTypeConversionTest : public test::BootstrapFixture
{
public:
    DBTypeConversionTest()
        : test::BootstrapFixture(false, false)
    {
    }

    // toDate tests
    void test_toDate_basic();
    void test_toDate_leading_zeros();
    void test_toDate_year_only();

    // toTime tests
    void test_toTime_basic();
    void test_toTime_with_seconds();
    void test_toTime_with_nanoseconds();

    // toDateTime tests
    void test_toDateTime_basic();
    void test_toDateTime_date_only();
    void test_toDateTime_with_nanoseconds();

    // toTimeString / toDateTimeString tests
    void test_toTimeString_no_fraction_when_zero_nanoseconds();
    void test_toTimeString_keeps_full_precision();
    void test_toDateTimeString_no_fraction_when_zero_nanoseconds();

    // fractional-seconds rounding for SQL literal generation
    void test_roundTimeFraction_rounds_to_4_digits();
    void test_roundTimeFraction_carries_into_seconds();
    void test_roundTimeFraction_wraps_at_midnight();
    void test_roundDateTimeFraction_carries_into_next_day();
    void test_roundDateTimeFraction_rolls_over_month_end();
    void test_roundTimeFraction_noop_when_no_cap_requested();

    CPPUNIT_TEST_SUITE(DBTypeConversionTest);

    CPPUNIT_TEST(test_toDate_basic);
    CPPUNIT_TEST(test_toDate_leading_zeros);
    CPPUNIT_TEST(test_toDate_year_only);

    CPPUNIT_TEST(test_toTime_basic);
    CPPUNIT_TEST(test_toTime_with_seconds);
    CPPUNIT_TEST(test_toTime_with_nanoseconds);

    CPPUNIT_TEST(test_toDateTime_basic);
    CPPUNIT_TEST(test_toDateTime_date_only);
    CPPUNIT_TEST(test_toDateTime_with_nanoseconds);

    CPPUNIT_TEST(test_toTimeString_no_fraction_when_zero_nanoseconds);
    CPPUNIT_TEST(test_toTimeString_keeps_full_precision);
    CPPUNIT_TEST(test_toDateTimeString_no_fraction_when_zero_nanoseconds);

    CPPUNIT_TEST(test_roundTimeFraction_rounds_to_4_digits);
    CPPUNIT_TEST(test_roundTimeFraction_carries_into_seconds);
    CPPUNIT_TEST(test_roundTimeFraction_wraps_at_midnight);
    CPPUNIT_TEST(test_roundDateTimeFraction_carries_into_next_day);
    CPPUNIT_TEST(test_roundDateTimeFraction_rolls_over_month_end);
    CPPUNIT_TEST(test_roundTimeFraction_noop_when_no_cap_requested);

    CPPUNIT_TEST_SUITE_END();
};

// ----- toDate ---------------------------------------------------------------

void DBTypeConversionTest::test_toDate_basic()
{
    // SQL date format: YYYY-MM-DD
    Date aDate = DBTypeConversion::toDate(u"2024-07-15");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Year", sal_Int16(2024), aDate.Year);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Month", sal_uInt16(7), aDate.Month);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Day", sal_uInt16(15), aDate.Day);
}

void DBTypeConversionTest::test_toDate_leading_zeros()
{
    // Months and days with leading zeros must parse correctly
    Date aDate = DBTypeConversion::toDate(u"2000-01-01");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Year", sal_Int16(2000), aDate.Year);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Month", sal_uInt16(1), aDate.Month);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Day", sal_uInt16(1), aDate.Day);
}

void DBTypeConversionTest::test_toDate_year_only()
{
    // Only the year component present; month and day default to 0
    Date aDate = DBTypeConversion::toDate(u"1999");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Year", sal_Int16(1999), aDate.Year);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Month", sal_uInt16(0), aDate.Month);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Day", sal_uInt16(0), aDate.Day);
}

// ----- toTime ---------------------------------------------------------------

void DBTypeConversionTest::test_toTime_basic()
{
    // ISO 8601 time: HH:MM
    Time aTime = DBTypeConversion::toTime(u"14:30");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hours", sal_uInt16(14), aTime.Hours);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minutes", sal_uInt16(30), aTime.Minutes);
}

void DBTypeConversionTest::test_toTime_with_seconds()
{
    // ISO 8601 time: HH:MM:SS
    Time aTime = DBTypeConversion::toTime(u"08:05:59");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hours", sal_uInt16(8), aTime.Hours);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minutes", sal_uInt16(5), aTime.Minutes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Seconds", sal_uInt16(59), aTime.Seconds);
}

void DBTypeConversionTest::test_toTime_with_nanoseconds()
{
    // ISO 8601 time with fractional seconds: HH:MM:SS.nnnnnnnnn
    Time aTime = DBTypeConversion::toTime(u"10:20:30.123456789");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hours", sal_uInt16(10), aTime.Hours);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minutes", sal_uInt16(20), aTime.Minutes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Seconds", sal_uInt16(30), aTime.Seconds);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("NanoSeconds", sal_uInt32(123456789), aTime.NanoSeconds);
}

// ----- toDateTime -----------------------------------------------------------

void DBTypeConversionTest::test_toDateTime_basic()
{
    // SQL timestamp format: YYYY-MM-DD HH:MM:SS
    DateTime aDT = DBTypeConversion::toDateTime(u"2024-03-21 11:45:00"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Year", sal_Int16(2024), aDT.Year);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Month", sal_uInt16(3), aDT.Month);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Day", sal_uInt16(21), aDT.Day);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hours", sal_uInt16(11), aDT.Hours);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minutes", sal_uInt16(45), aDT.Minutes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Seconds", sal_uInt16(0), aDT.Seconds);
}

void DBTypeConversionTest::test_toDateTime_date_only()
{
    // No time part: time fields default to 0
    DateTime aDT = DBTypeConversion::toDateTime(u"2023-12-31"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Year", sal_Int16(2023), aDT.Year);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Month", sal_uInt16(12), aDT.Month);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Day", sal_uInt16(31), aDT.Day);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hours", sal_uInt16(0), aDT.Hours);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minutes", sal_uInt16(0), aDT.Minutes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Seconds", sal_uInt16(0), aDT.Seconds);
}

void DBTypeConversionTest::test_toDateTime_with_nanoseconds()
{
    // Fractional seconds in the timestamp string
    DateTime aDT = DBTypeConversion::toDateTime(u"2026-01-15 09:30:45.500000000"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Year", sal_Int16(2026), aDT.Year);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Month", sal_uInt16(1), aDT.Month);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Day", sal_uInt16(15), aDT.Day);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hours", sal_uInt16(9), aDT.Hours);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minutes", sal_uInt16(30), aDT.Minutes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Seconds", sal_uInt16(45), aDT.Seconds);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("NanoSeconds", sal_uInt32(500000000), aDT.NanoSeconds);
}

// ----- toTimeString / toDateTimeString ---------------------------------------

void DBTypeConversionTest::test_toTimeString_no_fraction_when_zero_nanoseconds()
{
    // tdf#153057 (reopened): TIME values must not get trailing ".000000000"
    css::util::Time aTime;
    aTime.Hours = 0;
    aTime.Minutes = 3;
    aTime.Seconds = 0;
    aTime.NanoSeconds = 0;
    CPPUNIT_ASSERT_EQUAL(u"00:03:00"_ustr, DBTypeConversion::toTimeString(aTime));
}

void DBTypeConversionTest::test_toTimeString_keeps_full_precision()
{
    css::util::Time aTime;
    aTime.Hours = 0;
    aTime.Minutes = 3;
    aTime.Seconds = 0;
    aTime.NanoSeconds = 123456789;
    CPPUNIT_ASSERT_EQUAL(u"00:03:00.123456789"_ustr, DBTypeConversion::toTimeString(aTime));
}

void DBTypeConversionTest::test_roundTimeFraction_rounds_to_4_digits()
{
    // this is the rounding the Firebird is using
    css::util::Time aTime;
    aTime.Hours = 0;
    aTime.Minutes = 3;
    aTime.Seconds = 0;
    aTime.NanoSeconds = 123456789;
    CPPUNIT_ASSERT(!DBTypeConversion::roundTimeFraction(aTime, 4));
    // 123456789ns rounds to the nearest 1/10000s (123500000ns = .1235).
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(123500000), aTime.NanoSeconds);
    // toTimeString() trims trailing zeros
    CPPUNIT_ASSERT_EQUAL(u"00:03:00.1235"_ustr, DBTypeConversion::toTimeString(aTime));
}

void DBTypeConversionTest::test_roundTimeFraction_carries_into_seconds()
{
    css::util::Time aTime;
    aTime.Hours = 0;
    aTime.Minutes = 3;
    aTime.Seconds = 59;
    aTime.NanoSeconds = 999960000; // rounds up to a full extra second
    CPPUNIT_ASSERT(!DBTypeConversion::roundTimeFraction(aTime, 4));
    CPPUNIT_ASSERT_EQUAL(u"00:04:00"_ustr, DBTypeConversion::toTimeString(aTime));
}

void DBTypeConversionTest::test_roundTimeFraction_wraps_at_midnight()
{
    css::util::Time aTime;
    aTime.Hours = 23;
    aTime.Minutes = 59;
    aTime.Seconds = 59;
    aTime.NanoSeconds = 999950000;
    CPPUNIT_ASSERT(DBTypeConversion::roundTimeFraction(aTime, 4));
    CPPUNIT_ASSERT_EQUAL(u"00:00:00"_ustr, DBTypeConversion::toTimeString(aTime));
}

void DBTypeConversionTest::test_roundDateTimeFraction_carries_into_next_day()
{
    css::util::DateTime aDT;
    aDT.Year = 2024;
    aDT.Month = 7;
    aDT.Day = 15;
    aDT.Hours = 23;
    aDT.Minutes = 59;
    aDT.Seconds = 59;
    aDT.NanoSeconds = 999950000;
    DBTypeConversion::roundDateTimeFraction(aDT, 4);
    CPPUNIT_ASSERT_EQUAL(u"2024-07-16 00:00:00"_ustr, DBTypeConversion::toDateTimeString(aDT));
}

void DBTypeConversionTest::test_roundDateTimeFraction_rolls_over_month_end()
{
    css::util::DateTime aDT;
    aDT.Year = 2024;
    aDT.Month = 12;
    aDT.Day = 31;
    aDT.Hours = 23;
    aDT.Minutes = 59;
    aDT.Seconds = 59;
    aDT.NanoSeconds = 999950000;
    DBTypeConversion::roundDateTimeFraction(aDT, 4);
    CPPUNIT_ASSERT_EQUAL(u"2025-01-01 00:00:00"_ustr, DBTypeConversion::toDateTimeString(aDT));
}

void DBTypeConversionTest::test_roundTimeFraction_noop_when_no_cap_requested()
{
    css::util::Time aTime;
    aTime.Hours = 0;
    aTime.Minutes = 3;
    aTime.Seconds = 0;
    aTime.NanoSeconds = 123456789;
    CPPUNIT_ASSERT(!DBTypeConversion::roundTimeFraction(aTime, 9));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(123456789), aTime.NanoSeconds);
}

void DBTypeConversionTest::test_toDateTimeString_no_fraction_when_zero_nanoseconds()
{
    css::util::DateTime aDT;
    aDT.Year = 2024;
    aDT.Month = 7;
    aDT.Day = 15;
    aDT.Hours = 14;
    aDT.Minutes = 30;
    aDT.Seconds = 0;
    aDT.NanoSeconds = 0;
    CPPUNIT_ASSERT_EQUAL(u"2024-07-15 14:30:00"_ustr, DBTypeConversion::toDateTimeString(aDT));
}

CPPUNIT_TEST_SUITE_REGISTRATION(DBTypeConversionTest);

} // namespace connectivity::commontools

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
