/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <tools/date.hxx>

namespace tools
{
class DateTest : public CppUnit::TestFixture
{
public:
    void testDate();
    void testLeapYear();
    void testGetDaysInYear();
    void testValidGregorianDate();
    void testValidDate();
    void testNormalize();
    void testGetDayOfWeek();
    void testGetDaysInMonth();
    void testIsBetween();
    void testIsEndOfMonth();

    CPPUNIT_TEST_SUITE(DateTest);
    CPPUNIT_TEST(testDate);
    CPPUNIT_TEST(testLeapYear);
    CPPUNIT_TEST(testGetDaysInYear);
    CPPUNIT_TEST(testValidGregorianDate);
    CPPUNIT_TEST(testValidDate);
    CPPUNIT_TEST(testNormalize);
    CPPUNIT_TEST(testGetDayOfWeek);
    CPPUNIT_TEST(testGetDaysInMonth);
    CPPUNIT_TEST(testIsBetween);
    CPPUNIT_TEST(testIsEndOfMonth);
    CPPUNIT_TEST_SUITE_END();
};

void DateTest::testDate()
{
    const Date aCE(1, 1, 1); // first day CE
    const Date aBCE(31, 12, -1); // last day BCE
    const Date aMin(1, 1, -32768); // minimum date
    const Date aMax(31, 12, 32767); // maximum date
    Date aDate(Date::EMPTY);
    const sal_Int32 kMinDays = -11968265;
    const sal_Int32 kMaxDays = 11967900;

    // Last day BCE to first day CE is 1 day difference.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aCE - aBCE);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), aBCE - aCE);
    aDate = aBCE;
    aDate.AddDays(1);
    CPPUNIT_ASSERT_EQUAL(aCE.GetDate(), aDate.GetDate());
    aDate = aCE;
    aDate.AddDays(-1);
    CPPUNIT_ASSERT_EQUAL(aBCE.GetDate(), aDate.GetDate());

    // The entire BCE and CE ranges cover that many days. Day 0 is -0001-12-31
    CPPUNIT_ASSERT_EQUAL(kMaxDays, aMax - aBCE);
    CPPUNIT_ASSERT_EQUAL(kMinDays, aMin - aBCE);

    // Truncate at limits, not under-/overflow or wrap.
    aDate = aMin;
    aDate.AddDays(-1);
    CPPUNIT_ASSERT_EQUAL(aMin.GetDate(), aDate.GetDate());
    aDate = aMax;
    aDate.AddDays(1);
    CPPUNIT_ASSERT_EQUAL(aMax.GetDate(), aDate.GetDate());
    aDate = aBCE;
    aDate.AddDays(kMinDays - 10);
    CPPUNIT_ASSERT_EQUAL(aMin.GetDate(), aDate.GetDate());
    aDate = aBCE;
    aDate.AddDays(kMaxDays + 10);
    CPPUNIT_ASSERT_EQUAL(aMax.GetDate(), aDate.GetDate());
    aDate = aMax;
    aDate.SetDay(32);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL(aMax.GetDate(), aDate.GetDate());
    CPPUNIT_ASSERT(!aDate.IsEmpty());

    // 0001-00-x normalized to -0001-12-x
    aDate.SetYear(1);
    aDate.SetMonth(0);
    aDate.SetDay(22);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL(Date(22, 12, -1).GetDate(), aDate.GetDate());

    sal_uInt32 nExpected = 11222;
    CPPUNIT_ASSERT_EQUAL(nExpected, aDate.GetDateUnsigned());
    // 1999-02-32 normalized to 1999-03-04
    aDate.SetYear(1999);
    aDate.SetMonth(2);
    aDate.SetDay(32);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL(Date(4, 3, 1999).GetDate(), aDate.GetDate());

    // Empty date is not normalized and stays empty date.
    aDate = Date(Date::EMPTY);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL(Date(Date::EMPTY).GetDate(), aDate.GetDate());
    CPPUNIT_ASSERT(!aDate.IsValidDate()); // GetDate() also shall have no normalizing side effect

    // 0000-01-00 normalized to -0001-12-31
    // SetYear(0) asserts, use empty date to force.
    aDate = Date(Date::EMPTY);
    aDate.SetMonth(1);
    aDate.SetDay(0);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL(Date(31, 12, -1).GetDate(), aDate.GetDate());

    // 1999-00-00 normalized to 1998-12-31 (not 1998-11-30, or otherwise
    // also 0001-00-00 should be -0001-11-30 which it should not, should it?)
    aDate.SetYear(1999);
    aDate.SetMonth(0);
    aDate.SetDay(0);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL(Date(31, 12, 1998).GetDate(), aDate.GetDate());

    // 0001-00-00 normalized to -0001-12-31
    aDate.SetYear(1);
    aDate.SetMonth(0);
    aDate.SetDay(0);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL(Date(31, 12, -1).GetDate(), aDate.GetDate());
}

void DateTest::testLeapYear()
{
    {
        Date aDate(1, 1, 2000);
        CPPUNIT_ASSERT(aDate.IsLeapYear());
    }

    {
        Date aDate(1, 1, 1900);
        CPPUNIT_ASSERT(!aDate.IsLeapYear());
    }

    {
        Date aDate(1, 1, 1999);
        CPPUNIT_ASSERT(!aDate.IsLeapYear());
    }

    {
        Date aDate(1, 1, 2004);
        CPPUNIT_ASSERT(aDate.IsLeapYear());
    }

    {
        Date aDate(1, 1, 400);
        CPPUNIT_ASSERT(aDate.IsLeapYear());
    }

    {
        // Year -1 is a leap year.
        Date aDate(28, 2, -1);
        aDate.AddDays(1);
        CPPUNIT_ASSERT(aDate.IsLeapYear());
        CPPUNIT_ASSERT_EQUAL(Date(29, 2, -1).GetDate(), aDate.GetDate());
    }

    {
        Date aDate(1, 3, -1);
        aDate.AddDays(-1);
        CPPUNIT_ASSERT(aDate.IsLeapYear());
        CPPUNIT_ASSERT_EQUAL(Date(29, 2, -1).GetDate(), aDate.GetDate());
    }

    {
        // Year -5 is a leap year.
        Date aDate(28, 2, -5);
        aDate.AddDays(1);
        CPPUNIT_ASSERT(aDate.IsLeapYear());
        CPPUNIT_ASSERT_EQUAL(Date(29, 2, -5).GetDate(), aDate.GetDate());
    }

    {
        Date aDate(1, 3, -5);
        aDate.AddDays(-1);
        CPPUNIT_ASSERT(aDate.IsLeapYear());
        CPPUNIT_ASSERT_EQUAL(Date(29, 2, -5).GetDate(), aDate.GetDate());
    }
}

void DateTest::testGetDaysInYear()
{
    {
        Date aDate(1, 1, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(366), aDate.GetDaysInYear());
    }

    {
        Date aDate(1, 1, 1900);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(365), aDate.GetDaysInYear());
    }

    {
        Date aDate(1, 1, 1999);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(365), aDate.GetDaysInYear());
    }

    {
        Date aDate(1, 1, 2004);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(366), aDate.GetDaysInYear());
    }

    {
        Date aDate(1, 1, 400);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(366), aDate.GetDaysInYear());
    }
}

void DateTest::testValidGregorianDate()
{
    {
        Date aDate(1, 0, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(1, 13, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(1, 1, 1581);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(1, 9, 1582);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(1, 10, 1582);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(32, 1, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(29, 2, 2000);
        CPPUNIT_ASSERT(aDate.IsValidAndGregorian());
    }

    {
        Date aDate(29, 2, 2001);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(32, 3, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(31, 4, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(32, 5, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(31, 6, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(32, 7, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(32, 8, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(31, 9, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(32, 10, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(31, 11, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }

    {
        Date aDate(32, 12, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidAndGregorian());
    }
}

void DateTest::testValidDate()
{
    {
        // Empty date is not a valid date.
        Date aDate(Date::EMPTY);
        CPPUNIT_ASSERT(aDate.IsEmpty());
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(32, 1, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(29, 2, 2000);
        CPPUNIT_ASSERT(aDate.IsValidDate());
    }

    {
        Date aDate(29, 2, 2001);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(32, 3, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(31, 4, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(32, 5, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(31, 6, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(32, 7, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(32, 8, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(31, 9, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(32, 10, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(31, 11, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }

    {
        Date aDate(32, 12, 2000);
        CPPUNIT_ASSERT(!aDate.IsValidDate());
    }
}

void DateTest::testNormalize()
{
    {
        Date aDate(32, 2, 1999);
        aDate.Normalize();
        Date aExpectedDate(4, 3, 1999);
        CPPUNIT_ASSERT_EQUAL(aExpectedDate, aDate);
    }

    {
        Date aDate(1, 13, 1999);
        aDate.Normalize();
        Date aExpectedDate(1, 1, 2000);
        CPPUNIT_ASSERT_EQUAL(aExpectedDate, aDate);
    }

    {
        Date aDate(42, 13, 1999);
        aDate.Normalize();
        Date aExpectedDate(11, 2, 2000);
        CPPUNIT_ASSERT_EQUAL(aExpectedDate, aDate);
    }

    {
        Date aDate(1, 0, 1);
        aDate.Normalize();
        Date aExpectedDate(1, 12, -1);
        CPPUNIT_ASSERT_EQUAL(aExpectedDate, aDate);
    }
}

void DateTest::testGetDayOfWeek()
{
    {
        DayOfWeek eExpectedDay = DayOfWeek::MONDAY;
        Date aDate(30, 4, 2018);
        CPPUNIT_ASSERT_EQUAL(eExpectedDay, aDate.GetDayOfWeek());
    }

    {
        DayOfWeek eExpectedDay = DayOfWeek::TUESDAY;
        Date aDate(1, 5, 2018);
        CPPUNIT_ASSERT_EQUAL(eExpectedDay, aDate.GetDayOfWeek());
    }

    {
        DayOfWeek eExpectedDay = DayOfWeek::WEDNESDAY;
        Date aDate(2, 5, 2018);
        CPPUNIT_ASSERT_EQUAL(eExpectedDay, aDate.GetDayOfWeek());
    }

    {
        DayOfWeek eExpectedDay = DayOfWeek::THURSDAY;
        Date aDate(3, 5, 2018);
        CPPUNIT_ASSERT_EQUAL(eExpectedDay, aDate.GetDayOfWeek());
    }

    {
        DayOfWeek eExpectedDay = DayOfWeek::FRIDAY;
        Date aDate(4, 5, 2018);
        CPPUNIT_ASSERT_EQUAL(eExpectedDay, aDate.GetDayOfWeek());
    }

    {
        DayOfWeek eExpectedDay = DayOfWeek::SATURDAY;
        Date aDate(5, 5, 2018);
        CPPUNIT_ASSERT_EQUAL(eExpectedDay, aDate.GetDayOfWeek());
    }

    {
        DayOfWeek eExpectedDay = DayOfWeek::SUNDAY;
        Date aDate(6, 5, 2018);
        CPPUNIT_ASSERT_EQUAL(eExpectedDay, aDate.GetDayOfWeek());
    }
}

void DateTest::testGetDaysInMonth()
{
    {
        Date aDate(1, 1, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(31), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 2, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(29), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 2, 1999);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(28), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 3, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(31), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 4, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 5, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(31), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 6, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 7, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(31), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 8, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(31), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 9, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 10, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(31), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 11, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), aDate.GetDaysInMonth());
    }

    {
        Date aDate(1, 12, 2000);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(31), aDate.GetDaysInMonth());
    }
}

void DateTest::testIsBetween()
{
    Date aDate(6, 4, 2018);
    CPPUNIT_ASSERT(aDate.IsBetween(Date(1, 1, 2018), Date(1, 12, 2018)));
}

void DateTest::testIsEndOfMonth()
{
    {
        Date aDate(31, 12, 2000);
        CPPUNIT_ASSERT(aDate.IsEndOfMonth());
    }

    {
        Date aDate(30, 12, 2000);
        CPPUNIT_ASSERT(!aDate.IsEndOfMonth());
    }

    {
        Date aDate(29, 2, 2000);
        CPPUNIT_ASSERT(aDate.IsEndOfMonth());
    }

    {
        Date aDate(28, 2, 2000);
        CPPUNIT_ASSERT(!aDate.IsEndOfMonth());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(DateTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
