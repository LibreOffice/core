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

#include <limits>

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>

#include "sax/tools/converter.hxx"
#include "comphelper/sequenceasvector.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::util;
using sax::Converter;


namespace {

class ConverterTest
    : public ::CppUnit::TestFixture
{
public:
    virtual void setUp();
    virtual void tearDown();

    void testDuration();
    void testDateTime();
    void testDouble();
    void testMeasure();
    void testBool();
    void testPercent();
    void testColor();
    void testNumber();
    void testBase64();

    CPPUNIT_TEST_SUITE(ConverterTest);
    CPPUNIT_TEST(testDuration);
    CPPUNIT_TEST(testDateTime);
    CPPUNIT_TEST(testDouble);
    CPPUNIT_TEST(testMeasure);
    CPPUNIT_TEST(testBool);
    CPPUNIT_TEST(testPercent);
    CPPUNIT_TEST(testColor);
    CPPUNIT_TEST(testNumber);
    CPPUNIT_TEST(testBase64);
    CPPUNIT_TEST_SUITE_END();

private:
};

void ConverterTest::setUp()
{
}

void ConverterTest::tearDown()
{
}

static bool eqDuration(util::Duration a, util::Duration b) {
    return a.Years == b.Years && a.Months == b.Months && a.Days == b.Days
        && a.Hours == b.Hours && a.Minutes == b.Minutes
        && a.Seconds == b.Seconds
        && a.MilliSeconds == b.MilliSeconds
        && a.Negative == b.Negative;
}

static void doTest(util::Duration const & rid, char const*const pis,
        char const*const i_pos = 0)
{
    char const*const pos((i_pos) ? i_pos : pis);
    util::Duration od;
    ::rtl::OUString is(::rtl::OUString::createFromAscii(pis));
    bool bSuccess = Converter::convertDuration(od, is);
    OSL_TRACE("%d %dY %dM %dD %dH %dM %dS %dm",
        od.Negative, od.Years, od.Months, od.Days,
        od.Hours, od.Minutes, od.Seconds, od.MilliSeconds);
    CPPUNIT_ASSERT(bSuccess);
    CPPUNIT_ASSERT(eqDuration(rid, od));
    ::rtl::OUStringBuffer buf;
    Converter::convertDuration(buf, od);
    OSL_TRACE("%s",
        ::rtl::OUStringToOString(buf.getStr(), RTL_TEXTENCODING_UTF8).getStr());
    CPPUNIT_ASSERT(buf.makeStringAndClear().equalsAscii(pos));
}

static void doTestDurationF(char const*const pis)
{
    util::Duration od;
    bool bSuccess = Converter::convertDuration(od,
            ::rtl::OUString::createFromAscii(pis));
    OSL_TRACE("%d %dY %dM %dD %dH %dM %dS %dH",
        od.Negative, od.Years, od.Months, od.Days,
        od.Hours, od.Minutes, od.Seconds, od.MilliSeconds);
    CPPUNIT_ASSERT(!bSuccess);
}

void ConverterTest::testDuration()
{
    OSL_TRACE("\nSAX CONVERTER TEST BEGIN");
    doTest( util::Duration(false, 1, 0, 0, 0, 0, 0, 0), "P1Y" );
    doTest( util::Duration(false, 0, 42, 0, 0, 0, 0, 0), "P42M" );
    doTest( util::Duration(false, 0, 0, 111, 0, 0, 0, 0), "P111D" );
    doTest( util::Duration(false, 0, 0, 0, 52, 0, 0, 0), "PT52H" );
    doTest( util::Duration(false, 0, 0, 0, 0, 717, 0, 0), "PT717M" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 121, 0), "PT121S" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 0, 190), "PT0.19S" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 0, 90), "PT0.09S" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 0, 9), "PT0.009S" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 9, 999),
            "PT9.999999999999999999999999999999S", "PT9.999S" );
    doTest( util::Duration(true , 0, 0, 9999, 0, 0, 0, 0), "-P9999D" );
    doTest( util::Duration(true , 7, 6, 5, 4, 3, 2, 10),
            "-P7Y6M5DT4H3M2.01S" );
    doTest( util::Duration(false, 0, 6, 0, 0, 3, 0, 0), "P6MT3M" );
    doTest( util::Duration(false, 0, 0, 0, 0, 0, 0, 0), "P0D" );
    doTestDurationF("1Y1M");        // invalid: no ^P
    doTestDurationF("P-1Y1M");      // invalid: - after P
    doTestDurationF("P1M1Y");       // invalid: Y after M
    doTestDurationF("PT1Y");        // invalid: Y after T
    doTestDurationF("P1Y1M1M");     // invalid: M twice, no T
    doTestDurationF("P1YT1MT1M");   // invalid: T twice
    doTestDurationF("P1YT");        // invalid: T but no H,M,S
    doTestDurationF("P99999999999Y");   // cannot parse so many Ys
    doTestDurationF("PT.1S");       // invalid: no 0 preceding .
    doTestDurationF("PT5M.134S");   // invalid: no 0 preceding .
    doTestDurationF("PT1.S");       // invalid: no digit following .
    OSL_TRACE("\nSAX CONVERTER TEST END");
}


static bool eqDateTime(util::DateTime a, util::DateTime b) {
    return a.Year == b.Year && a.Month == b.Month && a.Day == b.Day
        && a.Hours == b.Hours && a.Minutes == b.Minutes
        && a.Seconds == b.Seconds
        && a.HundredthSeconds == b.HundredthSeconds;
}

static void doTest(util::DateTime const & rdt, char const*const pis,
        char const*const i_pos = 0)
{
    char const*const pos((i_pos) ? i_pos : pis);
    ::rtl::OUString is(::rtl::OUString::createFromAscii(pis));
    util::DateTime odt;
    bool bSuccess( Converter::convertDateTime(odt, is) );
    OSL_TRACE("Y:%d M:%d D:%d  H:%d M:%d S:%d H:%d",
        odt.Year, odt.Month, odt.Day,
        odt.Hours, odt.Minutes, odt.Seconds, odt.HundredthSeconds);
    CPPUNIT_ASSERT(bSuccess);
    CPPUNIT_ASSERT(eqDateTime(rdt, odt));
    ::rtl::OUStringBuffer buf;
    Converter::convertDateTime(buf, odt, true);
    OSL_TRACE("%s",
        ::rtl::OUStringToOString(buf.getStr(), RTL_TEXTENCODING_UTF8).getStr());
    CPPUNIT_ASSERT(buf.makeStringAndClear().equalsAscii(pos));
}

static void doTestDateTimeF(char const*const pis)
{
    util::DateTime odt;
    bool bSuccess = Converter::convertDateTime(odt,
            ::rtl::OUString::createFromAscii(pis));
    OSL_TRACE("Y:%d M:%d D:%d  H:%dH M:%d S:%d H:%d",
        odt.Year, odt.Month, odt.Day,
        odt.Hours, odt.Minutes, odt.Seconds, odt.HundredthSeconds);
    CPPUNIT_ASSERT(!bSuccess);
}

void ConverterTest::testDateTime()
{
    OSL_TRACE("\nSAX CONVERTER TEST BEGIN");
    doTest( util::DateTime(0, 0, 0, 0, 1, 1, 1), "0001-01-01T00:00:00" );
    doTest( util::DateTime(0, 0, 0, 0, 1, 1, 1),
            "0001-01-01T00:00:00Z", "0001-01-01T00:00:00" );
//    doTest( util::DateTime(0, 0, 0, 0, 1, 1, -1), "-0001-01-01T00:00:00" );
//    doTest( util::DateTime(0, 0, 0, 0, 1, 1, -1), "-0001-01-01T00:00:00Z" );
    doTest( util::DateTime(0, 0, 0, 0, 1, 1, 1),
            "0001-01-01T00:00:00-00:00", "0001-01-01T00:00:00" );
    doTest( util::DateTime(0, 0, 0, 0, 1, 1, 1),
            "0001-01-01T00:00:00+00:00", "0001-01-01T00:00:00" );
    doTest( util::DateTime(0, 0, 0, 0, 2, 1, 1)/*(0, 0, 12, 0, 2, 1, 1)*/,
            "0001-01-02T00:00:00-12:00", "0001-01-02T00:00:00" );
//            "0001-02-01T12:00:00" );
    doTest( util::DateTime(0, 0, 0, 0, 2, 1, 1)/*(0, 0, 12, 0, 1, 1, 1)*/,
            "0001-01-02T00:00:00+12:00", "0001-01-02T00:00:00" );
//            "0001-01-01T12:00:00" );
    doTest( util::DateTime(99, 59, 59, 23, 31, 12, 9999),
            "9999-12-31T23:59:59.99" );
    doTest( util::DateTime(99, 59, 59, 23, 31, 12, 9999),
            "9999-12-31T23:59:59.99Z", "9999-12-31T23:59:59.99" );
    doTest( util::DateTime(99, 59, 59, 23, 31, 12, 9999),
            "9999-12-31T23:59:59.9999999999999999999999999999999999999",
            "9999-12-31T23:59:59.99" );
    doTest( util::DateTime(99, 59, 59, 23, 31, 12, 9999),
            "9999-12-31T23:59:59.9999999999999999999999999999999999999Z",
            "9999-12-31T23:59:59.99" );
    doTest( util::DateTime(0, 0, 0, 0, 29, 2, 2000), // leap year
            "2000-02-29T00:00:00-00:00", "2000-02-29T00:00:00" );
    doTest( util::DateTime(0, 0, 0, 0, 29, 2, 1600), // leap year
            "1600-02-29T00:00:00-00:00", "1600-02-29T00:00:00" );
    doTest( util::DateTime(0, 0, 0, 24, 1, 1, 333)
                /*(0, 0, 0, 0, 2, 1, 333)*/,
            "0333-01-01T24:00:00"/*, "0333-01-02T00:00:00"*/ );
    // While W3C XMLSchema specifies a minimum of 4 year digits we are lenient
    // in what we accept.
    doTest( util::DateTime(0, 0, 0, 0, 1, 1, 1),
            "1-01-01T00:00:00", "0001-01-01T00:00:00" );
    doTestDateTimeF( "+0001-01-01T00:00:00" ); // invalid: ^+
    doTestDateTimeF( "0001-1-01T00:00:00" ); // invalid: < 2 M
    doTestDateTimeF( "0001-01-1T00:00:00" ); // invalid: < 2 D
    doTestDateTimeF( "0001-01-01T0:00:00" ); // invalid: < 2 H
    doTestDateTimeF( "0001-01-01T00:0:00" ); // invalid: < 2 M
    doTestDateTimeF( "0001-01-01T00:00:0" ); // invalid: < 2 S
    doTestDateTimeF( "0001-01-01T00:00:00." ); // invalid: .$
    doTestDateTimeF( "0001-01-01T00:00:00+1:00" ); // invalid: < 2 TZ H
    doTestDateTimeF( "0001-01-01T00:00:00+00:1" ); // invalid: < 2 TZ M
    doTestDateTimeF( "0001-13-01T00:00:00" ); // invalid: M > 12
    doTestDateTimeF( "0001-01-32T00:00:00" ); // invalid: D > 31
    doTestDateTimeF( "0001-01-01T25:00:00" ); // invalid: H > 24
    doTestDateTimeF( "0001-01-01T00:60:00" ); // invalid: H > 59
    doTestDateTimeF( "0001-01-01T00:00:60" ); // invalid: S > 59
    doTestDateTimeF( "0001-01-01T24:01:00" ); // invalid: H=24, but M != 0
    doTestDateTimeF( "0001-01-01T24:00:01" ); // invalid: H=24, but S != 0
    doTestDateTimeF( "0001-01-01T24:00:00.1" ); // invalid: H=24, but H != 0
    doTestDateTimeF( "0001-01-02T00:00:00+15:00" ); // invalid: TZ > +14:00
    doTestDateTimeF( "0001-01-02T00:00:00+14:01" ); // invalid: TZ > +14:00
    doTestDateTimeF( "0001-01-02T00:00:00-15:00" ); // invalid: TZ < -14:00
    doTestDateTimeF( "0001-01-02T00:00:00-14:01" ); // invalid: TZ < -14:00
    doTestDateTimeF( "2100-02-29T00:00:00-00:00" ); // invalid: no leap year
    doTestDateTimeF( "1900-02-29T00:00:00-00:00" ); // invalid: no leap year
    OSL_TRACE("\nSAX CONVERTER TEST END");
}

void doTestDouble(char const*const pis, double const rd,
        sal_Int16 const nSourceUnit, sal_Int16 const nTargetUnit)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    double od;
    bool bSuccess(Converter::convertDouble(od, is, nSourceUnit, nTargetUnit));
    OSL_TRACE("%f", od);
    CPPUNIT_ASSERT(bSuccess);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(rd, od, 0.00000001);
    ::rtl::OUStringBuffer buf;
    Converter::convertDouble(buf, od, true, nTargetUnit, nSourceUnit);
    OSL_TRACE("%s",
        ::rtl::OUStringToOString(buf.getStr(), RTL_TEXTENCODING_UTF8).getStr());
    CPPUNIT_ASSERT_EQUAL(is, buf.makeStringAndClear());
}

void ConverterTest::testDouble()
{
    doTestDouble("42", 42.0, MeasureUnit::TWIP, MeasureUnit::TWIP);
    doTestDouble("42", 42.0, MeasureUnit::POINT, MeasureUnit::POINT);
    doTestDouble("42", 42.0, MeasureUnit::MM_100TH, MeasureUnit::MM_100TH);
    doTestDouble("42", 42.0, MeasureUnit::MM_10TH, MeasureUnit::MM_10TH);
    doTestDouble("42", 42.0, MeasureUnit::MM, MeasureUnit::MM); // identity don't seem to add unit?
    doTestDouble("42", 42.0, MeasureUnit::CM, MeasureUnit::CM);
    doTestDouble("42", 42.0, MeasureUnit::INCH, MeasureUnit::INCH);
    doTestDouble("2pt", 40.0, MeasureUnit::POINT, MeasureUnit::TWIP);
    doTestDouble("20pc", 1, MeasureUnit::TWIP, MeasureUnit::POINT);
    doTestDouble("4", 2.26771653543307, MeasureUnit::MM_100TH, MeasureUnit::TWIP);
    doTestDouble("4", 22.6771653543307, MeasureUnit::MM_10TH, MeasureUnit::TWIP);
    doTestDouble("4mm", 226.771653543307, MeasureUnit::MM, MeasureUnit::TWIP);
    doTestDouble("4cm", 2267.71653543307, MeasureUnit::CM, MeasureUnit::TWIP);
    doTestDouble("4in", 5760.0, MeasureUnit::INCH, MeasureUnit::TWIP);
    doTestDouble("1440pc", 1.0, MeasureUnit::TWIP, MeasureUnit::INCH);
    doTestDouble("567pc", 1.000125, MeasureUnit::TWIP, MeasureUnit::CM);
    doTestDouble("56.7pc", 1.000125, MeasureUnit::TWIP, MeasureUnit::MM);
    doTestDouble("5.67pc", 1.000125, MeasureUnit::TWIP, MeasureUnit::MM_10TH);
    doTestDouble("0.567pc", 1.000125, MeasureUnit::TWIP, MeasureUnit::MM_100TH);
    doTestDouble("42pt", 1.4816666666666, MeasureUnit::POINT, MeasureUnit::CM);
    doTestDouble("42pt", 14.816666666666, MeasureUnit::POINT, MeasureUnit::MM);
    doTestDouble("42pt", 148.16666666666, MeasureUnit::POINT, MeasureUnit::MM_10TH);
    doTestDouble("42pt", 1481.6666666666, MeasureUnit::POINT, MeasureUnit::MM_100TH);
    doTestDouble("72pt", 1.0, MeasureUnit::POINT, MeasureUnit::INCH);
    doTestDouble("3.5in", 8.89, MeasureUnit::INCH, MeasureUnit::CM);
    doTestDouble("3.5in", 88.9, MeasureUnit::INCH, MeasureUnit::MM);
    doTestDouble("3.5in", 889.0, MeasureUnit::INCH, MeasureUnit::MM_10TH);
    doTestDouble("3.5in", 8890.0, MeasureUnit::INCH, MeasureUnit::MM_100TH);
    doTestDouble("2in", 144, MeasureUnit::INCH, MeasureUnit::POINT);
    doTestDouble("5.08cm", 2.0, MeasureUnit::CM, MeasureUnit::INCH);
    doTestDouble("3.5cm", 3500.0, MeasureUnit::CM, MeasureUnit::MM_100TH);
    doTestDouble("3.5cm", 350.0, MeasureUnit::CM, MeasureUnit::MM_10TH);
    doTestDouble("3.5cm", 35.0, MeasureUnit::CM, MeasureUnit::MM);
    doTestDouble("10cm", 283.464566929134, MeasureUnit::CM, MeasureUnit::POINT);
    doTestDouble("0.5cm", 283.464566929134, MeasureUnit::CM, MeasureUnit::TWIP);
    doTestDouble("10mm", 28.3464566929134, MeasureUnit::MM, MeasureUnit::POINT);
    doTestDouble("0.5mm", 28.3464566929134, MeasureUnit::MM, MeasureUnit::TWIP);
    doTestDouble("10", 2.83464566929134, MeasureUnit::MM_10TH, MeasureUnit::POINT);
    doTestDouble("0.5", 2.83464566929134, MeasureUnit::MM_10TH, MeasureUnit::TWIP);
    doTestDouble("10", 0.283464566929134, MeasureUnit::MM_100TH, MeasureUnit::POINT);
    doTestDouble("0.5", 0.283464566929134, MeasureUnit::MM_100TH, MeasureUnit::TWIP);
    doTestDouble("10mm", 1.0, MeasureUnit::MM, MeasureUnit::CM);
    doTestDouble("10mm", 100.0, MeasureUnit::MM, MeasureUnit::MM_10TH);
    doTestDouble("20mm", 2000.0, MeasureUnit::MM, MeasureUnit::MM_100TH);
    doTestDouble("300", 30.0, MeasureUnit::MM_10TH, MeasureUnit::MM);
    doTestDouble("400", 4.0, MeasureUnit::MM_100TH, MeasureUnit::MM);
    doTestDouble("600", 6000.0, MeasureUnit::MM_10TH, MeasureUnit::MM_100TH);
    doTestDouble("700", 70.0, MeasureUnit::MM_100TH, MeasureUnit::MM_10TH);
}

void doTestStringToMeasure(sal_Int32 rValue, char const*const pis, sal_Int16 nTargetUnit, sal_Int32 nMin, sal_Int32 nMax)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    sal_Int32 nVal;
    bool bSuccess(Converter::convertMeasure(nVal, is, nTargetUnit, nMin, nMax));
    OSL_TRACE("%i", nVal);
    CPPUNIT_ASSERT(bSuccess);
    CPPUNIT_ASSERT_EQUAL(rValue, nVal);
}

void doTestMeasureToString(char const*const pis, sal_Int32 nMeasure, sal_Int16 const nSourceUnit, sal_Int16 const nTargetUnit)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    ::rtl::OUStringBuffer buf;
    Converter::convertMeasure(buf, nMeasure, nSourceUnit, nTargetUnit);
    OSL_TRACE("%s", ::rtl::OUStringToOString(buf.getStr(), RTL_TEXTENCODING_UTF8).getStr());
    CPPUNIT_ASSERT_EQUAL(is, buf.makeStringAndClear());
}

void ConverterTest::testMeasure()
{
    //check all the measure units
    doTestStringToMeasure(1000, "10mm", MeasureUnit::MM_100TH, -1, 4321);
    doTestStringToMeasure(200, "20mm", MeasureUnit::MM_10TH, 12, 4567);
    doTestStringToMeasure(300, "300", MeasureUnit::MM, 31, 555);
    doTestStringToMeasure(400, "400", MeasureUnit::CM, 10, 4321);
    doTestStringToMeasure(120, "120", MeasureUnit::INCH_1000TH, 10, 4321);
    doTestStringToMeasure(111, "111", MeasureUnit::INCH_100TH, 10, 4321);
    doTestStringToMeasure(22, "22", MeasureUnit::INCH_10TH, 10, 4321);
    doTestStringToMeasure(27, "27", MeasureUnit::INCH, 10, 4321);
    doTestStringToMeasure(52, "52", MeasureUnit::POINT, 10, 4321);
    doTestStringToMeasure(120, "120", MeasureUnit::TWIP, 10, 4321);
    doTestStringToMeasure(666, "666", MeasureUnit::M, 10, 4321);
    doTestStringToMeasure(42, "42", MeasureUnit::KM, 10, 4321);
    doTestStringToMeasure(30, "30", MeasureUnit::PICA, 10, 4321);
    doTestStringToMeasure(20, "20", MeasureUnit::FOOT, 10, 4321);
    doTestStringToMeasure(40, "40", MeasureUnit::MILE, 10, 4321);
    doTestStringToMeasure(40, "40%", MeasureUnit::PERCENT, 10, 4321);
    doTestStringToMeasure(800, "800", MeasureUnit::PIXEL, 10, 4321);
    doTestStringToMeasure(600, "600px", MeasureUnit::PIXEL, 10, 4321);
    doTestStringToMeasure(777, "777", MeasureUnit::APPFONT, 10, 4321);
    doTestStringToMeasure(80000, "80000", MeasureUnit::SYSFONT, 10, 432100);
    //strange values (negative, too large etc.)
    doTestStringToMeasure(555, "666", MeasureUnit::MM, -1000, 555);
    doTestStringToMeasure(-1000, "-1001", MeasureUnit::MM, -1000, 555);
    doTestStringToMeasure(0, "-0", MeasureUnit::MM, -1, 0);
    doTestStringToMeasure(::std::numeric_limits<sal_Int32>::max(), "1234567890mm", MeasureUnit::MM_10TH, 12, ::std::numeric_limits<sal_Int32>::max());
    doTestStringToMeasure(-300, "-300", MeasureUnit::MM, -1000, 555);
    doTestStringToMeasure(::std::numeric_limits<sal_Int32>::min(), "-999999999999999px", MeasureUnit::PIXEL, ::std::numeric_limits<sal_Int32>::min(), 555);   //really crazy numbers...

    doTestMeasureToString("6mm", 600, MeasureUnit::MM_100TH, MeasureUnit::MM);
    doTestMeasureToString("0.005cm", 000000005, MeasureUnit::MM_100TH, MeasureUnit::CM);    // zeros in the front doesn't count
    doTestMeasureToString("3mm", 30, MeasureUnit::MM_10TH, MeasureUnit::MM);
    doTestMeasureToString("6.66cm", 666, MeasureUnit::MM_10TH, MeasureUnit::CM);
    doTestMeasureToString("-157.3pt", -555, MeasureUnit::MM_10TH, MeasureUnit::POINT);
    doTestMeasureToString("174976.378in", 44444000, MeasureUnit::MM_10TH, MeasureUnit::INCH);    //let's check accuracy
    doTestMeasureToString("40%", 40, MeasureUnit::PERCENT, MeasureUnit::PERCENT);
    doTestMeasureToString("70.56mm", 4000, MeasureUnit::TWIP, MeasureUnit::MM);
    doTestMeasureToString("979.928cm", 555550, MeasureUnit::TWIP, MeasureUnit::CM);
    doTestMeasureToString("111.1pt", 2222, MeasureUnit::TWIP, MeasureUnit::POINT);
    doTestMeasureToString("385.7986in", 555550, MeasureUnit::TWIP, MeasureUnit::INCH);
}

void doTestStringToBool(bool bBool, char const*const pis)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    bool bTemp;
    bool bSuccess(Converter::convertBool(bTemp, is));
    OSL_TRACE("%s", bTemp);
    CPPUNIT_ASSERT(bSuccess);
    CPPUNIT_ASSERT_EQUAL(bBool, bTemp);

}

void doTestBoolToString(char const*const pis, bool bValue )
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    ::rtl::OUStringBuffer buf;
    Converter::convertBool(buf, bValue);
    OSL_TRACE("%s", ::rtl::OUStringToOString(buf.getStr(), RTL_TEXTENCODING_UTF8).getStr());
    CPPUNIT_ASSERT_EQUAL(is, buf.makeStringAndClear());
}

void ConverterTest::testBool()
{
    doTestStringToBool(true, "true");
    doTestStringToBool(false, "false");
    doTestBoolToString("true", true);
    doTestBoolToString("false", false);
}

void doTestStringToPercent(sal_Int32 nValue, char const*const pis)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    sal_Int32 nTemp;
    bool bSuccess(Converter::convertPercent(nTemp, is));
    OSL_TRACE("%i", nTemp);
    CPPUNIT_ASSERT(bSuccess);
    CPPUNIT_ASSERT_EQUAL(nValue, nTemp);
}

void doTestPercentToString(char const*const pis, sal_Int32 nValue)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    ::rtl::OUStringBuffer buf;
    Converter::convertPercent(buf, nValue);
    OSL_TRACE("%s", ::rtl::OUStringToOString(buf.getStr(), RTL_TEXTENCODING_UTF8).getStr());
    CPPUNIT_ASSERT_EQUAL(is, buf.makeStringAndClear());
}

void ConverterTest::testPercent()
{
    doTestStringToPercent(40, "40%");
    doTestStringToPercent(30, "30");
    doTestStringToPercent(120, "120%");
    doTestStringToPercent(-40, "-40%");
    doTestStringToPercent(0, "0%");
    doTestPercentToString("12%", 12);
    doTestPercentToString("-123%", -123);
    doTestPercentToString("0%", 0);
    doTestPercentToString("1%", 00001);
}

void doTestStringToColor(sal_Int32 nValue, char const*const pis)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    sal_Int32 nTemp;
    bool bSuccess(Converter::convertColor(nTemp, is));
    OSL_TRACE("%i", nTemp);
    CPPUNIT_ASSERT(bSuccess);
    CPPUNIT_ASSERT_EQUAL(nValue, nTemp);
}

void doTestColorToString(char const*const pis, sal_Int32 nValue)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    ::rtl::OUStringBuffer buf;
    Converter::convertColor(buf, nValue);
    OSL_TRACE("%s", ::rtl::OUStringToOString(buf.getStr(), RTL_TEXTENCODING_UTF8).getStr());
    CPPUNIT_ASSERT_EQUAL(is, buf.makeStringAndClear());
}

void ConverterTest::testColor()
{
    doTestStringToColor(11259375, "#abcdef");
    doTestStringToColor(160, "#0000a0");
    doTestStringToColor(40960, "#00a000");
    doTestStringToColor(0, "#000000");
    doTestColorToString("#000615", 1557);
    doTestColorToString("#5bcd15", 123456789);
    doTestColorToString("#fffac7", -1337);
    doTestColorToString("#000000", 0);
}

void doTestStringToNumber(sal_Int32 nValue, char const*const pis, sal_Int32 nMin, sal_Int32 nMax)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    sal_Int32 nTemp;
    bool bSuccess(Converter::convertNumber(nTemp, is, nMin, nMax));
    OSL_TRACE("%i", nTemp);
    CPPUNIT_ASSERT(bSuccess);
    CPPUNIT_ASSERT_EQUAL(nValue, nTemp);
}

void doTestNumberToString(char const*const pis, sal_Int32 nValue)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    ::rtl::OUStringBuffer buf;
    Converter::convertNumber(buf, nValue);
    OSL_TRACE("%s", ::rtl::OUStringToOString(buf.getStr(), RTL_TEXTENCODING_UTF8).getStr());
    CPPUNIT_ASSERT_EQUAL(is, buf.makeStringAndClear());
}

void ConverterTest::testNumber()
{
    doTestStringToNumber(30, "30", 1, 40);
    doTestStringToNumber(1, "-5", 1, 300);
    doTestStringToNumber(-30, "7", -100, -30);
    doTestStringToNumber(0, "-0", 0, 1);
    doTestStringToNumber(0, "666", -0, 0);
    doTestNumberToString("333", 333);
    doTestNumberToString("-1", -1);
    doTestNumberToString("0", 0000);
    doTestNumberToString("-1", -0001);
    doTestNumberToString("0", -0);
}

void doTestEncodeBase64(char const*const pis, const uno::Sequence<sal_Int8> aPass)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    ::rtl::OUStringBuffer buf;
    Converter::encodeBase64(buf, aPass);
    OSL_TRACE("%s", ::rtl::OUStringToOString(buf.getStr(), RTL_TEXTENCODING_UTF8).getStr());
    CPPUNIT_ASSERT_EQUAL(is, buf.makeStringAndClear());
}

void doTestDecodeBase64(const uno::Sequence<sal_Int8> aPass, char const*const pis)
{
    ::rtl::OUString const is(::rtl::OUString::createFromAscii(pis));
    uno::Sequence< sal_Int8 > tempSequence;
    Converter::decodeBase64(tempSequence, is);
    OSL_TRACE("%s", ::rtl::OUStringToOString(is.getStr(), RTL_TEXTENCODING_UTF8).getStr());
    bool b = (tempSequence==aPass);
    CPPUNIT_ASSERT(b);
}

void ConverterTest::testBase64()
{
    comphelper::SequenceAsVector< sal_Int8 > tempSeq(4);
    for(sal_Int8 i = 0; i<4; ++i)
        tempSeq.push_back(i);
    uno::Sequence< sal_Int8 > tempSequence = tempSeq.getAsConstList();
    doTestEncodeBase64("AAAAAAABAgM=", tempSequence);
    doTestDecodeBase64(tempSequence, "AAAAAAABAgM=");
    tempSeq[0] = sal_Int8(5);
    tempSeq[1] = sal_Int8(2);
    tempSeq[2] = sal_Int8(3);
    tempSequence = tempSeq.getAsConstList();
    doTestEncodeBase64("BQIDAAABAgM=", tempSequence);
    doTestDecodeBase64(tempSequence, "BQIDAAABAgM=");
    tempSeq[0] = sal_Int8(sal_uInt8(200));
    tempSeq[1] = sal_Int8(31);
    tempSeq[2] = sal_Int8(77);
    tempSeq[3] = sal_Int8(111);
    tempSequence = tempSeq.getAsConstList();
    doTestEncodeBase64("yB9NbwABAgM=", tempSequence);
    doTestDecodeBase64(tempSequence, "yB9NbwABAgM=");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ConverterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
