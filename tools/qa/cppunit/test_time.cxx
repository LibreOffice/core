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

#include <tools/time.hxx>

namespace tools
{
class TimeTest : public CppUnit::TestFixture
{
public:
    void testTime();
    void testClockValues();

    CPPUNIT_TEST_SUITE(TimeTest);
    CPPUNIT_TEST(testTime);
    CPPUNIT_TEST(testClockValues);
    CPPUNIT_TEST_SUITE_END();
};

void TimeTest::testTime()
{
    Time aOrigTime(1, 56, 10);
    auto nMS = aOrigTime.GetMSFromTime();

    Time aNewTime(0);
    aNewTime.MakeTimeFromMS(nMS);

    CPPUNIT_ASSERT(bool(aOrigTime == aNewTime));
}

void TimeTest::testClockValues()
{
    double fTime;
    sal_uInt16 nHour, nMinute, nSecond, nMs;

    fTime = 0.0;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(0), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(0), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(0), nSecond);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(0), nMs);

    fTime = 1.0;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(0), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(0), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(0), nSecond);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(0), nMs);

    fTime = -1.0;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(0), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(0), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(0), nSecond);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(0), nMs);

    fTime = 1.5;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(12), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(0), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(0), nSecond);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(0), nMs);

    fTime = -1.5;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(12), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(0), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(0), nSecond);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(0), nMs);

    fTime = 0.75;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(18), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(0), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(0), nSecond);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(0), nMs, 0.0);

    fTime = 0.0208333333333333;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(0), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(30), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(0), nSecond);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(0), nMs);

    fTime = 0.0000115740625;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(0), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(0), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(1), nSecond);
    // Expect this to be a rounded 0.999999
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(0), nMs);

    fTime = 0.524268391203704;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(12), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(34), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(56), nSecond);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(789), nMs);

    fTime = -0.000001;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(23), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(59), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(59), nSecond);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(914), nMs);

    fTime = -0.000001;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(23), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(59), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(59), nSecond);
    // Expect this to be rounded.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(910), nMs);

    fTime = -0.00000000001;
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(24), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(0), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(0), nSecond);
    // Expect this to be a rounded 0.999999
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(0), nMs);

    fTime = -1e-24; // value insignificant for time
    Time::GetClock(fTime, nHour, nMinute, nSecond, nMs, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour value.", sal_uInt16(24), nHour);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute value.", sal_uInt16(0), nMinute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second value.", sal_uInt16(0), nSecond);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Millisecond value.", sal_uInt16(0), nMs);
}

CPPUNIT_TEST_SUITE_REGISTRATION(TimeTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
