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

#include <tools/duration.hxx>
#include <tools/datetime.hxx>

namespace tools
{
class DurationTest : public CppUnit::TestFixture
{
public:
    void testDuration();

    CPPUNIT_TEST_SUITE(DurationTest);
    CPPUNIT_TEST(testDuration);
    CPPUNIT_TEST_SUITE_END();
};

void DurationTest::testDuration()
{
    {
        const Duration aD(Time(Time::EMPTY), Time(12, 0, 0));
        CPPUNIT_ASSERT_EQUAL(0.5, aD.GetInDays());
    }
    {
        const Duration aD(Time(24, 0, 0), Time(12, 0, 0));
        CPPUNIT_ASSERT_EQUAL(-0.5, aD.GetInDays());
    }
    {
        const DateTime aS(Date(23, 11, 1999), Time(6, 0, 0));
        const DateTime aE(Date(24, 11, 1999), Time(18, 0, 0));
        const Duration aD(aS, aE);
        CPPUNIT_ASSERT_EQUAL(1.5, aD.GetInDays());
        DateTime aDT1(aS);
        const DateTime aDT2 = aDT1 + aD;
        CPPUNIT_ASSERT_EQUAL(aE, aDT2);
        aDT1 += aD;
        CPPUNIT_ASSERT_EQUAL(aE, aDT1);
        aDT1 += aD;
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(26, 11, 1999), Time(6, 0, 0)), aDT1);
    }
    {
        const DateTime aS(Date(23, 11, 1999), Time(18, 0, 0));
        const DateTime aE(Date(24, 11, 1999), Time(6, 0, 0));
        const Duration aD(aS, aE);
        CPPUNIT_ASSERT_EQUAL(0.5, aD.GetInDays());
        DateTime aDT1(aS);
        const DateTime aDT2 = aDT1 + aD;
        CPPUNIT_ASSERT_EQUAL(aE, aDT2);
        aDT1 += aD;
        CPPUNIT_ASSERT_EQUAL(aE, aDT1);
        aDT1 += aD;
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(24, 11, 1999), Time(18, 0, 0)), aDT1);
    }
    {
        const DateTime aS(Date(24, 11, 1999), Time(18, 0, 0));
        const DateTime aE(Date(23, 11, 1999), Time(6, 0, 0));
        const Duration aD(aS, aE);
        CPPUNIT_ASSERT_EQUAL(-1.5, aD.GetInDays());
        DateTime aDT1(aS);
        const DateTime aDT2 = aDT1 + aD;
        CPPUNIT_ASSERT_EQUAL(aE, aDT2);
        aDT1 += aD;
        CPPUNIT_ASSERT_EQUAL(aE, aDT1);
        aDT1 += aD;
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(21, 11, 1999), Time(18, 0, 0)), aDT1);
    }
    {
        const DateTime aS(Date(24, 11, 1999), Time(6, 0, 0));
        const DateTime aE(Date(23, 11, 1999), Time(18, 0, 0));
        const Duration aD(aS, aE);
        CPPUNIT_ASSERT_EQUAL(-0.5, aD.GetInDays());
        DateTime aDT1(aS);
        const DateTime aDT2 = aDT1 + aD;
        CPPUNIT_ASSERT_EQUAL(aE, aDT2);
        aDT1 += aD;
        CPPUNIT_ASSERT_EQUAL(aE, aDT1);
        aDT1 += aD;
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(23, 11, 1999), Time(6, 0, 0)), aDT1);
    }
    {
        const Duration aD(1.5);
        CPPUNIT_ASSERT_EQUAL(1.5, aD.GetInDays());
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(24, 11, 1999), Time(18, 0, 0)),
                             DateTime(Date(23, 11, 1999), Time(6, 0, 0)) + aD);
    }
    {
        const Duration aD(-1.5);
        CPPUNIT_ASSERT_EQUAL(-1.5, aD.GetInDays());
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(23, 11, 1999), Time(6, 0, 0)),
                             DateTime(Date(24, 11, 1999), Time(18, 0, 0)) + aD);
    }
    {
        const Duration aD(-1.5);
        const Duration aN = -aD;
        CPPUNIT_ASSERT_EQUAL(1.5, aN.GetInDays());
    }
    {
        const Duration aD(1, Time(2, 3, 4, 5));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aD.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), aD.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(3), aD.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(4), aD.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(5), aD.GetTime().GetNanoSec());
    }
    {
        // 235929599 seconds == SAL_MAX_UINT16 hours + 59 minutes + 59 seconds
        const Duration aD(0, Time(0, 0, 235929599, Time::nanoSecPerSec - 1));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2730), aD.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(15), aD.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aD.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aD.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(999999999), aD.GetTime().GetNanoSec());
    }
    {
        // 235929599 seconds == SAL_MAX_UINT16 hours + 59 minutes + 59 seconds
        const Duration aD(0, 0, 0, 235929599, Time::nanoSecPerSec - 1);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2730), aD.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(15), aD.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aD.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aD.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(999999999), aD.GetTime().GetNanoSec());
    }
    {
        const Duration aD(1, 2, 3, 4, 5);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aD.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), aD.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(3), aD.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(4), aD.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(5), aD.GetTime().GetNanoSec());
    }
    {
        const Duration aD(-1, 2, 3, 4, 5);
        CPPUNIT_ASSERT(aD.IsNegative());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), aD.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), aD.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(3), aD.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(4), aD.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(5), aD.GetTime().GetNanoSec());
    }
    {
        const Duration aD(1, SAL_MAX_UINT32, SAL_MAX_UINT32, SAL_MAX_UINT32, SAL_MAX_UINT64);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(182202802), aD.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), aD.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(17), aD.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(48), aD.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(709551615), aD.GetTime().GetNanoSec());
    }
    {
        const Duration aD(-1, SAL_MAX_UINT32, SAL_MAX_UINT32, SAL_MAX_UINT32, SAL_MAX_UINT64);
        CPPUNIT_ASSERT(aD.IsNegative());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-182202802), aD.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), aD.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(17), aD.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(48), aD.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(709551615), aD.GetTime().GetNanoSec());
    }
    { // Maximum days with all max possible.
        const Duration aD(1965280846, SAL_MAX_UINT32, SAL_MAX_UINT32, SAL_MAX_UINT32,
                          SAL_MAX_UINT64);
        CPPUNIT_ASSERT_EQUAL(SAL_MAX_INT32, aD.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), aD.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(17), aD.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(48), aD.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(709551615), aD.GetTime().GetNanoSec());
    }
    { // Maximum negative days with all max possible.
        const Duration aD(-1965280847, SAL_MAX_UINT32, SAL_MAX_UINT32, SAL_MAX_UINT32,
                          SAL_MAX_UINT64);
        CPPUNIT_ASSERT_EQUAL(SAL_MIN_INT32, aD.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), aD.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(17), aD.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(48), aD.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(709551615), aD.GetTime().GetNanoSec());
    }
    { // Add()
        const DateTime aS(Date(23, 11, 1999), Time(0, 0, 0));
        const DateTime aE(Date(23, 11, 1999), Time(1, 23, 45));
        const Duration aD(aS, aE);
        Duration aV = aD;
        bool bOverflow = true;
        aV.Add(aD, bOverflow);
        CPPUNIT_ASSERT(!bOverflow);
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(23, 11, 1999), Time(2, 47, 30)),
                             DateTime(Date(23, 11, 1999), Time(0, 0, 0)) + aV);
        for (int i = 0; i < 20; ++i)
            aV.Add(aD, bOverflow);
        CPPUNIT_ASSERT(!bOverflow);
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(24, 11, 1999), Time(6, 42, 30)),
                             DateTime(Date(23, 11, 1999), Time(0, 0, 0)) + aV);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aV.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(6), aV.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(42), aV.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(30), aV.GetTime().GetSec());
        CPPUNIT_ASSERT(aV.GetTime().GetTime() > 0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.27951388888889, aV.GetInDays(), 1E-14);
        // Negative duration.
        const Duration aN(aE, aS);
        aV = aN;
        aV.Add(aN, bOverflow);
        CPPUNIT_ASSERT(!bOverflow);
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(22, 11, 1999), Time(21, 12, 30)),
                             DateTime(Date(23, 11, 1999), Time(0, 0, 0)) + aV);
        for (int i = 0; i < 20; ++i)
            aV.Add(aN, bOverflow);
        CPPUNIT_ASSERT(!bOverflow);
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(21, 11, 1999), Time(17, 17, 30)),
                             DateTime(Date(23, 11, 1999), Time(0, 0, 0)) + aV);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), aV.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(6), aV.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(42), aV.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(30), aV.GetTime().GetSec());
        CPPUNIT_ASSERT(aV.GetTime().GetTime() < 0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.27951388888889, aV.GetInDays(), 1E-14);
    }
    { // Mult()
        const DateTime aS(Date(23, 11, 1999), Time(0, 0, 0));
        const DateTime aE(Date(23, 11, 1999), Time(1, 23, 45));
        const Duration aD(aS, aE);
        bool bOverflow = true;
        Duration aV = aD.Mult(22, bOverflow);
        CPPUNIT_ASSERT(!bOverflow);
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(24, 11, 1999), Time(6, 42, 30)),
                             DateTime(Date(23, 11, 1999), Time(0, 0, 0)) + aV);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aV.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(6), aV.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(42), aV.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(30), aV.GetTime().GetSec());
        CPPUNIT_ASSERT(aV.GetTime().GetTime() > 0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.27951388888889, aV.GetInDays(), 1E-14);
        // Negative duration.
        const Duration aN(aE, aS);
        bOverflow = true;
        aV = aN.Mult(22, bOverflow);
        CPPUNIT_ASSERT(!bOverflow);
        CPPUNIT_ASSERT_EQUAL(DateTime(Date(21, 11, 1999), Time(17, 17, 30)),
                             DateTime(Date(23, 11, 1999), Time(0, 0, 0)) + aV);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), aV.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(6), aV.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(42), aV.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(30), aV.GetTime().GetSec());
        CPPUNIT_ASSERT(aV.GetTime().GetTime() < 0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.27951388888889, aV.GetInDays(), 1E-14);
    }
    { // Mult() including days.
        const Duration aD(1.5);
        bool bOverflow = true;
        Duration aV = aD.Mult(10, bOverflow);
        CPPUNIT_ASSERT(!bOverflow);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(15.0, aV.GetInDays(), 0.0);
    }
    { // Mult() including days.
        const Duration aD(-1.5);
        bool bOverflow = true;
        Duration aV = aD.Mult(10, bOverflow);
        CPPUNIT_ASSERT(!bOverflow);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-15.0, aV.GetInDays(), 0.0);
    }
    { // Mult() including days.
        const Duration aD(1.5);
        bool bOverflow = true;
        Duration aV = aD.Mult(-10, bOverflow);
        CPPUNIT_ASSERT(!bOverflow);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-15.0, aV.GetInDays(), 0.0);
    }
    { // Mult() including days.
        const Duration aD(-1.5);
        bool bOverflow = true;
        Duration aV = aD.Mult(-10, bOverflow);
        CPPUNIT_ASSERT(!bOverflow);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(15.0, aV.GetInDays(), 0.0);
    }
    { // Mult() with overflow.
        const Duration aD(SAL_MAX_INT32);
        bool bOverflow = false;
        Duration aV = aD.Mult(2, bOverflow);
        CPPUNIT_ASSERT(bOverflow);
        CPPUNIT_ASSERT_EQUAL(SAL_MAX_INT32, aV.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(23), aV.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aV.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aV.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(Time::nanoSecPerSec - 1),
                             aV.GetTime().GetNanoSec());
    }
    { // Mult() with overflow.
        const Duration aD(SAL_MIN_INT32);
        bool bOverflow = false;
        Duration aV = aD.Mult(2, bOverflow);
        CPPUNIT_ASSERT(bOverflow);
        CPPUNIT_ASSERT_EQUAL(SAL_MIN_INT32, aV.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(23), aV.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aV.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aV.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(Time::nanoSecPerSec - 1),
                             aV.GetTime().GetNanoSec());
    }
    { // Mult() with overflow.
        const Duration aD(SAL_MAX_INT32);
        bool bOverflow = false;
        Duration aV = aD.Mult(-2, bOverflow);
        CPPUNIT_ASSERT(bOverflow);
        CPPUNIT_ASSERT_EQUAL(SAL_MIN_INT32, aV.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(23), aV.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aV.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aV.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(Time::nanoSecPerSec - 1),
                             aV.GetTime().GetNanoSec());
    }
    { // Mult() with overflow.
        const Duration aD(SAL_MIN_INT32);
        bool bOverflow = false;
        Duration aV = aD.Mult(-2, bOverflow);
        CPPUNIT_ASSERT(bOverflow);
        CPPUNIT_ASSERT_EQUAL(SAL_MAX_INT32, aV.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(23), aV.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aV.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(59), aV.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(Time::nanoSecPerSec - 1),
                             aV.GetTime().GetNanoSec());
    }
    { // Inaccurate double yielding exact duration.
        const Time aS(15, 0, 0);
        const Time aE(16, 0, 0);
        const Duration aD(aE.GetTimeInDays() - aS.GetTimeInDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aD.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), aD.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), aD.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), aD.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), aD.GetTime().GetNanoSec());
    }
    { // Inaccurate double yielding exact duration, negative.
        const Time aS(15, 0, 0);
        const Time aE(16, 0, 0);
        const Duration aD(aS.GetTimeInDays() - aE.GetTimeInDays());
        CPPUNIT_ASSERT(aD.IsNegative());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aD.GetDays());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), aD.GetTime().GetHour());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), aD.GetTime().GetMin());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), aD.GetTime().GetSec());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), aD.GetTime().GetNanoSec());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(DurationTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
