/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <tools/time.hxx>

namespace tools
{
class TimeTest : public CppUnit::TestFixture
{
public:
    void testZeroTime()
    {
        Time aTime(Time::TimeInitEmpty::EMPTY);
        CPPUNIT_ASSERT(!aTime.GetTime());
    }

    void testTime()
    {
        {
            Time aTime(23, 59, 59, 999999999);
            sal_uInt16 nExpectedHour = 23;
            CPPUNIT_ASSERT_EQUAL(nExpectedHour, aTime.GetHour());
            sal_uInt16 nExpectedMinute = 59;
            CPPUNIT_ASSERT_EQUAL(nExpectedMinute, aTime.GetMin());
            sal_uInt16 nExpectedSecond = 59;
            CPPUNIT_ASSERT_EQUAL(nExpectedSecond, aTime.GetSec());
            sal_Int32 nExpectedMillisecond = 86399999;
            CPPUNIT_ASSERT_EQUAL(nExpectedMillisecond, aTime.GetMSFromTime());
            sal_uInt32 nExpectedNanoSecond = 999999999;
            CPPUNIT_ASSERT_EQUAL(nExpectedNanoSecond, aTime.GetNanoSec());
        }

        {
            Time aTime(23, 59, 59);
            sal_uInt16 nExpectedHour = 23;
            CPPUNIT_ASSERT_EQUAL(nExpectedHour, aTime.GetHour());
            sal_uInt16 nExpectedMinute = 59;
            CPPUNIT_ASSERT_EQUAL(nExpectedMinute, aTime.GetMin());
            sal_uInt16 nExpectedSecond = 59;
            CPPUNIT_ASSERT_EQUAL(nExpectedSecond, aTime.GetSec());
            sal_Int32 nExpectedMillisecond = 86399000;
            CPPUNIT_ASSERT_EQUAL(nExpectedMillisecond, aTime.GetMSFromTime());
            sal_uInt32 nExpectedNanoSecond = 0;
            CPPUNIT_ASSERT_EQUAL(nExpectedNanoSecond, aTime.GetNanoSec());
        }

        {
            Time aTime(23, 59);
            sal_uInt16 nExpectedHour = 23;
            CPPUNIT_ASSERT_EQUAL(nExpectedHour, aTime.GetHour());
            sal_uInt16 nExpectedMinute = 59;
            CPPUNIT_ASSERT_EQUAL(nExpectedMinute, aTime.GetMin());
            sal_uInt16 nExpectedSecond = 0;
            CPPUNIT_ASSERT_EQUAL(nExpectedSecond, aTime.GetSec());
            sal_Int32 nExpectedMillisecond = 86340000;
            CPPUNIT_ASSERT_EQUAL(nExpectedMillisecond, aTime.GetMSFromTime());
            sal_uInt32 nExpectedNanoSecond = 0;
            CPPUNIT_ASSERT_EQUAL(nExpectedNanoSecond, aTime.GetNanoSec());
        }
    }

    void testMakeTimeFromMS()
    {
        Time aExpectedTime(23, 59, 59, 999000000);
        sal_Int32 nExpectedMilliseconds = 86399999;

        Time aTime(Time::TimeInitEmpty::EMPTY);
        aTime.MakeTimeFromMS(nExpectedMilliseconds);

        CPPUNIT_ASSERT_EQUAL(aExpectedTime, aTime);
    }

    void testMakeTimeFromNS()
    {
        {
            Time aExpectedTime(3, 17, 59, 999999999);
            long nExpectedNanoseconds = 11879999999999;

            Time aTime(Time::TimeInitEmpty::EMPTY);
            aTime.MakeTimeFromNS(nExpectedNanoseconds);

            CPPUNIT_ASSERT_EQUAL(aExpectedTime, aTime);
        }

        {
            Time aOrigTime(1, 56, 10);
            auto nMS = aOrigTime.GetMSFromTime();

            Time aNewTime(0);
            aNewTime.MakeTimeFromMS(nMS);

            CPPUNIT_ASSERT(bool(aOrigTime == aNewTime));
        }
    }

    void testGetTimeInDays()
    {
        Time aTime(12, 0);
        double fExpectedDays = 0.5;

        CPPUNIT_ASSERT_EQUAL(fExpectedDays, aTime.GetTimeInDays());
    }

    void testIsEqualIgnoreNS()
    {
        Time aTime(23, 59, 59, 999999999);
        CPPUNIT_ASSERT(aTime.IsEqualIgnoreNanoSec(Time(23, 59, 59)));
    }

    CPPUNIT_TEST_SUITE(TimeTest);
    CPPUNIT_TEST(testTime);
    CPPUNIT_TEST(testZeroTime);
    CPPUNIT_TEST(testMakeTimeFromMS);
    CPPUNIT_TEST(testMakeTimeFromNS);
    CPPUNIT_TEST(testGetTimeInDays);
    CPPUNIT_TEST(testIsEqualIgnoreNS);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TimeTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
