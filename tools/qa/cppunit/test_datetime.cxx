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
#include <tools/datetimeutils.hxx>

namespace tools
{
class DateTimeTest : public CppUnit::TestFixture
{
public:
    void testLeapYear();
    void testIsBetween();
    void testDateTimeToString();

    CPPUNIT_TEST_SUITE(DateTimeTest);
    CPPUNIT_TEST(testLeapYear);
    CPPUNIT_TEST(testIsBetween);
    CPPUNIT_TEST(testDateTimeToString);
    CPPUNIT_TEST_SUITE_END();
};

void DateTimeTest::testLeapYear()
{
    {
        DateTime aDateTime(Date(1, 1, 2000));
        CPPUNIT_ASSERT(aDateTime.IsLeapYear());
    }

    {
        DateTime aDateTime(Date(1, 1, 1900));
        CPPUNIT_ASSERT(!aDateTime.IsLeapYear());
    }

    {
        DateTime aDateTime(Date(1, 1, 1999));
        CPPUNIT_ASSERT(!aDateTime.IsLeapYear());
    }

    {
        DateTime aDateTime(Date(1, 1, 2004));
        CPPUNIT_ASSERT(aDateTime.IsLeapYear());
    }

    {
        DateTime aDateTime(Date(1, 1, 400));
        CPPUNIT_ASSERT(aDateTime.IsLeapYear());
    }

    {
        // Year -1 is a leap year.
        DateTime aDateTime(Date(28, 2, -1));
        aDateTime.AddDays(1);
        CPPUNIT_ASSERT(aDateTime.IsLeapYear());
        CPPUNIT_ASSERT_EQUAL(Date(29, 2, -1).GetDate(), aDateTime.GetDate());
    }

    {
        DateTime aDateTime(Date(1, 3, -1));
        aDateTime.AddDays(-1);
        CPPUNIT_ASSERT(aDateTime.IsLeapYear());
        CPPUNIT_ASSERT_EQUAL(Date(29, 2, -1).GetDate(), aDateTime.GetDate());
    }

    {
        // Year -5 is a leap year.
        DateTime aDateTime(Date(28, 2, -5));
        aDateTime.AddDays(1);
        CPPUNIT_ASSERT(aDateTime.IsLeapYear());
        CPPUNIT_ASSERT_EQUAL(Date(29, 2, -5).GetDate(), aDateTime.GetDate());
    }

    {
        DateTime aDateTime(Date(1, 3, -5));
        aDateTime.AddDays(-1);
        CPPUNIT_ASSERT(aDateTime.IsLeapYear());
        CPPUNIT_ASSERT_EQUAL(Date(29, 2, -5).GetDate(), aDateTime.GetDate());
    }
}

void DateTimeTest::testIsBetween()
{
    DateTime aDateTime(Date(6, 4, 2018));
    CPPUNIT_ASSERT(aDateTime.IsBetween(Date(1, 1, 2018), Date(1, 12, 2018)));
}

void DateTimeTest::testDateTimeToString()
{
    DateTime aDateTime(Date(12, 5, 2018), Time(13, 13, 13));
    OString aExpectedString("2018-05-12T13:13:13Z");
    CPPUNIT_ASSERT_EQUAL(aExpectedString, DateTimeToOString(aDateTime));
}

CPPUNIT_TEST_SUITE_REGISTRATION(DateTimeTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
