/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <calendar_hijri.hxx>
#include <com/sun/star/i18n/XOrdinalSuffix.hpp>
#include <unotest/bootstrapfixturebase.hxx>

using namespace com::sun::star;

class TestCalendar : public test::BootstrapFixtureBase
{
private:
    uno::Reference<i18n::XOrdinalSuffix> m_xOrdinal;

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void testGetHijri();
    void testToGregorian();
    void testGetGregorianDay();

    CPPUNIT_TEST_SUITE(TestCalendar);
    CPPUNIT_TEST(testGetHijri);
    CPPUNIT_TEST(testToGregorian);
    CPPUNIT_TEST(testGetGregorianDay);
    CPPUNIT_TEST_SUITE_END();
};

void TestCalendar::setUp()
{
    BootstrapFixtureBase::setUp();
    m_xOrdinal.set(m_xSFactory->createInstance("com.sun.star.i18n.OrdinalSuffix"),
                   uno::UNO_QUERY_THROW);
}

void TestCalendar::tearDown()
{
    m_xOrdinal.clear();
    BootstrapFixtureBase::tearDown();
}

void TestCalendar::testGetHijri()
{
    sal_Int32 day, month, year;
    i18npool::Calendar_hijri::getHijri(&day, &month, &year);
    CPPUNIT_ASSERT(day == 1);
    CPPUNIT_ASSERT(month == 1);
    CPPUNIT_ASSERT(year == 1);
}

void TestCalendar::testToGregorian()
{
    sal_Int32 day, month, year;
    i18npool::Calendar_hijri::ToGregorian(&day, &month, &year);
    CPPUNIT_ASSERT(day == 1);
    CPPUNIT_ASSERT(month == 1);
    CPPUNIT_ASSERT(year == 1);
}

void TestCalendar::testGetGregorianDay()
{
    sal_Int32 lJulianDay = 1, day, month, year;
    i18npool::Calendar_hijri::getGregorianDay(lJulianDay, &day, &month, &year);
    CPPUNIT_ASSERT(day == 1);
    CPPUNIT_ASSERT(month == 1);
    CPPUNIT_ASSERT(year == 1);
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestCalendar);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
