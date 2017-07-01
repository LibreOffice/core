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

    CPPUNIT_TEST_SUITE(DateTest);
    CPPUNIT_TEST(testDate);
    CPPUNIT_TEST_SUITE_END();
};

void DateTest::testDate()
{
    const Date aCE(1,1,1);          // first day CE
    const Date aBCE(31,12,-1);      // last day BCE
    const Date aMin(1,1,-32768);    // minimum date
    const Date aMax(31,12,32767);   // maximum date
    Date aDate(Date::EMPTY);
    const sal_Int32 kMinDays = -11968265;
    const sal_Int32 kMaxDays =  11967900;

    // Last day BCE to first day CE is 1 day difference.
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(1), aCE - aBCE);
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(-1), aBCE - aCE);
    aDate = aBCE;
    CPPUNIT_ASSERT_EQUAL( aCE.GetDate(), (aDate += 1).GetDate());
    aDate = aCE;
    CPPUNIT_ASSERT_EQUAL( aBCE.GetDate(), (aDate -= 1).GetDate());

    // The entire BCE and CE ranges cover that many days. Day 0 is -0001-12-31
    CPPUNIT_ASSERT_EQUAL( kMaxDays, aMax - aBCE);
    CPPUNIT_ASSERT_EQUAL( kMinDays, aMin - aBCE);

    // Truncate at limits, not under-/overflow or wrap.
    aDate = aMin;
    CPPUNIT_ASSERT_EQUAL( aMin.GetDate(), (aDate -= 1).GetDate());
    aDate = aMax;
    CPPUNIT_ASSERT_EQUAL( aMax.GetDate(), (aDate += 1).GetDate());
    aDate = aBCE;
    CPPUNIT_ASSERT_EQUAL( aMin.GetDate(), (aDate += (kMinDays-10)).GetDate());
    aDate = aBCE;
    CPPUNIT_ASSERT_EQUAL( aMax.GetDate(), (aDate += (kMaxDays+10)).GetDate());
    aDate = aMax;
    aDate.SetDay(32);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL( aMax.GetDate(), aDate.GetDate());

    // Empty date is not a valid date.
    aDate = Date( Date::EMPTY );
    CPPUNIT_ASSERT( !aDate.IsValidDate());

    // 0001-00-x normalized to -0001-12-x
    aDate.SetYear(1);
    aDate.SetMonth(0);
    aDate.SetDay(22);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL( Date(22,12,-1).GetDate(), aDate.GetDate());
    // 1999-02-32 normalized to 1999-03-04
    aDate.SetYear(1999);
    aDate.SetMonth(2);
    aDate.SetDay(32);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL( Date(4,3,1999).GetDate(), aDate.GetDate());

    // Empty date is not normalized and stays empty date.
    aDate = Date( Date::EMPTY );
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL( Date(Date::EMPTY).GetDate(), aDate.GetDate());
    CPPUNIT_ASSERT( !aDate.IsValidDate());  // GetDate() also shall have no normalizing side effect

    // 0000-01-00 normalized to -0001-12-31
    // SetYear(0) asserts, use empty date to force.
    aDate = Date( Date::EMPTY );
    aDate.SetMonth(1);
    aDate.SetDay(0);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL( Date(31,12,-1).GetDate(), aDate.GetDate());

    // 1999-00-00 normalized to 1998-12-31 (not 1998-11-30, or otherwise
    // also 0001-00-00 should be -0001-11-30 which it should not, should it?)
    aDate.SetYear(1999);
    aDate.SetMonth(0);
    aDate.SetDay(0);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL( Date(31,12,1998).GetDate(), aDate.GetDate());

    // 0001-00-00 normalized to -0001-12-31
    aDate.SetYear(1);
    aDate.SetMonth(0);
    aDate.SetDay(0);
    aDate.Normalize();
    CPPUNIT_ASSERT_EQUAL( Date(31,12,-1).GetDate(), aDate.GetDate());

    // Year -1 is a leap year.
    aDate = Date(28,2,-1);
    CPPUNIT_ASSERT_EQUAL( Date(29,2,-1).GetDate(), (aDate += 1).GetDate());
    aDate = Date(1,3,-1);
    CPPUNIT_ASSERT_EQUAL( Date(29,2,-1).GetDate(), (aDate -= 1).GetDate());
    // Year -5 is a leap year.
    aDate = Date(28,2,-5);
    CPPUNIT_ASSERT_EQUAL( Date(29,2,-5).GetDate(), (aDate += 1).GetDate());
    aDate = Date(1,3,-5);
    CPPUNIT_ASSERT_EQUAL( Date(29,2,-5).GetDate(), (aDate -= 1).GetDate());
}

CPPUNIT_TEST_SUITE_REGISTRATION(DateTest);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
