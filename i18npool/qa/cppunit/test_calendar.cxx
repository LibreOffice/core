/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <calendar_hijri.hxx>
#include <unotest/bootstrapfixturebase.hxx>

using namespace com::sun::star;

class TestCalendar : public test::BootstrapFixtureBase
{
public:
    void testHijriGregorian();
    void testGetGregorianJulianDay();

    CPPUNIT_TEST_SUITE(TestCalendar);
    CPPUNIT_TEST(testHijriGregorian);
    CPPUNIT_TEST(testGetGregorianJulianDay);
    CPPUNIT_TEST_SUITE_END();
};

void TestCalendar::testHijriGregorian()
{
    // 21-7-1443 (Hijri) == 22-2-2022 (Gregorian)
    sal_Int32 day = 22, month = 2, year = 2022;
    i18npool::Calendar_hijri::getHijri(&day, &month, &year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21), day);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), month);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1443), year);

    i18npool::Calendar_hijri::ToGregorian(&day, &month, &year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(22), day);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), month);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2022), year);

    // 1-1-1 (Hijri) == 15-7-622 (Gregorian)
    // NOTE: The calculated date is 15-7-622, as it was with the
    // previous version of i18npool::Calendar_hijri::ToGregorian()
    // but in some articles, 15-7-622 is considered the equivalent date
    // https://en.wikipedia.org/wiki/622
    // This article states that 15-7-622 is correct:
    // "On the Origins of the Hijrī Calendar: A Multi-Faceted Perspective
    // Based on the Covenants of the Prophet and Specific Date Verification"
    // https://www.mdpi.com/2077-1444/12/1/42/htm
    day = 15;
    month = 7;
    year = 622;
    i18npool::Calendar_hijri::getHijri(&day, &month, &year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), day);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), month);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), year);

    i18npool::Calendar_hijri::ToGregorian(&day, &month, &year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15), day);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), month);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(622), year);

    // 1-1-100 (Hijri) == 2-8-718 (Gregorian)
    // https://habibur.com/hijri/100/
    day = 2;
    month = 8;
    year = 718;
    i18npool::Calendar_hijri::getHijri(&day, &month, &year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), day);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), month);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), year);

    i18npool::Calendar_hijri::ToGregorian(&day, &month, &year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), day);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), month);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(718), year);

    // 1-1-1000 (Hijri) == 19-10-1591 (Gregorian)
    // NOTE: The calculated date is 18-10-1591, but there is inconsistency
    // with this website, as it states it should be 19-10-1591
    // https://habibur.com/hijri/1000/
    day = 18;
    month = 10;
    year = 1591;
    i18npool::Calendar_hijri::getHijri(&day, &month, &year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), day);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), month);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), year);

    i18npool::Calendar_hijri::ToGregorian(&day, &month, &year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18), day);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), month);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1591), year);

    // 1-1-2000 (Hijri) == 7-1-2562 (Gregorian)
    // NOTE: The calculated date is 7-1-2562, but there is inconsistency
    // with this website, as it states it should be 8-1-2562
    // https://habibur.com/hijri/2000/
    day = 7;
    month = 1;
    year = 2562;
    i18npool::Calendar_hijri::getHijri(&day, &month, &year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), day);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), month);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), year);

    i18npool::Calendar_hijri::ToGregorian(&day, &month, &year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), day);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), month);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2562), year);
}

void TestCalendar::testGetGregorianJulianDay()
{
    // Julian day for 22-2-2022 (Gregorian) == 2459633
    // https://core2.gsfc.nasa.gov/time/julian.html
    sal_Int32 lJulianDay, day = 22, month = 2, year = 2022;
    lJulianDay = i18npool::Calendar_hijri::getJulianDay(day, month, year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2459633), lJulianDay);

    i18npool::Calendar_hijri::getGregorianDay(lJulianDay, &day, &month, &year);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(22), day);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), month);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2022), year);
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestCalendar);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
