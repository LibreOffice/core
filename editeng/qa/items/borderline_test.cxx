/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <editeng/borderline.hxx>

using namespace ::com::sun::star::table::BorderLineStyle;

#define TEST_WIDTH long( 40 )

#define THINTHICKSG_IN_WIDTH long( 15 )
#define THINTHICKSG_OUT_WIDTH long( 40 )
#define THINTHICKSG_DIST_WIDTH long( 15 )

#define THINTHICKLG_IN_WIDTH long( 15 )
#define THINTHICKLG_OUT_WIDTH long( 30 )
#define THINTHICKLG_DIST_WIDTH long( 40 )

using namespace editeng;

CPPUNIT_NS_BEGIN

template<> struct assertion_traits<SvxBorderStyle>
{
    static bool equal( const SvxBorderStyle& x, const SvxBorderStyle& y )
    {
        return x == y;
    }

    static std::string toString( const SvxBorderStyle& x )
    {
        OStringStream ost;
        ost << static_cast<unsigned int>(x);
        return ost.str();
    }
};

CPPUNIT_NS_END

namespace {

class BorderLineTest : public CppUnit::TestFixture
{
    public:
        void testGuessWidthDouble();
        void testGuessWidthNoMatch();
        void testGuessWidthThinthickSmallgap();
        void testGuessWidthThinthickLargegap();
        void testGuessWidthNostyleDouble();
        void testGuessWidthNostyleSingle();

    CPPUNIT_TEST_SUITE(BorderLineTest);
    CPPUNIT_TEST(testGuessWidthDouble);
    CPPUNIT_TEST(testGuessWidthNoMatch);
    CPPUNIT_TEST(testGuessWidthThinthickSmallgap);
    CPPUNIT_TEST(testGuessWidthThinthickLargegap);
    CPPUNIT_TEST(testGuessWidthNostyleDouble);
    CPPUNIT_TEST(testGuessWidthNostyleSingle);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BorderLineTest);

void BorderLineTest::testGuessWidthDouble()
{
    // Normal double case
    SvxBorderLine line;
    line.GuessLinesWidths( DOUBLE, TEST_WIDTH, TEST_WIDTH, TEST_WIDTH );
    CPPUNIT_ASSERT_EQUAL( DOUBLE, line.GetBorderLineStyle() );
    CPPUNIT_ASSERT_EQUAL( TEST_WIDTH, static_cast<long>(line.GetOutWidth()) );
    CPPUNIT_ASSERT_EQUAL( TEST_WIDTH, static_cast<long>(line.GetInWidth()) );
    CPPUNIT_ASSERT_EQUAL( TEST_WIDTH, static_cast<long>(line.GetDistance()) );
    CPPUNIT_ASSERT_EQUAL( 3*TEST_WIDTH, line.GetWidth() );
}

void BorderLineTest::testGuessWidthNoMatch()
{
    SvxBorderLine line;
    line.GuessLinesWidths( DOUBLE,
            TEST_WIDTH + 1, TEST_WIDTH + 2, TEST_WIDTH + 3 );
    CPPUNIT_ASSERT_EQUAL( DOUBLE, line.GetBorderLineStyle() );
    CPPUNIT_ASSERT_EQUAL( TEST_WIDTH+1, static_cast<long>(line.GetOutWidth()) );
    CPPUNIT_ASSERT_EQUAL( TEST_WIDTH+2, static_cast<long>(line.GetInWidth()) );
    CPPUNIT_ASSERT_EQUAL( TEST_WIDTH+3, static_cast<long>(line.GetDistance()));
    CPPUNIT_ASSERT_EQUAL( long( (3 * TEST_WIDTH) + 6 ), line.GetWidth() );
}

void BorderLineTest::testGuessWidthThinthickSmallgap()
{
    SvxBorderLine line;
    line.GuessLinesWidths( DOUBLE,
            THINTHICKSG_OUT_WIDTH,
            THINTHICKSG_IN_WIDTH,
            THINTHICKSG_DIST_WIDTH );
    CPPUNIT_ASSERT_EQUAL( THINTHICK_SMALLGAP, line.GetBorderLineStyle() );
    CPPUNIT_ASSERT_EQUAL( THINTHICKSG_OUT_WIDTH,
            static_cast<long>(line.GetOutWidth()) );
    CPPUNIT_ASSERT_EQUAL( THINTHICKSG_IN_WIDTH,
            static_cast<long>(line.GetInWidth()) );
    CPPUNIT_ASSERT_EQUAL( THINTHICKSG_DIST_WIDTH,
            static_cast<long>(line.GetDistance()) );
    CPPUNIT_ASSERT_EQUAL( THINTHICKSG_OUT_WIDTH + THINTHICKSG_IN_WIDTH
            + THINTHICKSG_DIST_WIDTH, line.GetWidth() );
}

void BorderLineTest::testGuessWidthThinthickLargegap()
{
    SvxBorderLine line;
    line.GuessLinesWidths( DOUBLE,
            THINTHICKLG_OUT_WIDTH,
            THINTHICKLG_IN_WIDTH,
            THINTHICKLG_DIST_WIDTH );
    CPPUNIT_ASSERT_EQUAL( THINTHICK_LARGEGAP, line.GetBorderLineStyle() );
    CPPUNIT_ASSERT_EQUAL( THINTHICKLG_OUT_WIDTH,
            static_cast<long>(line.GetOutWidth()) );
    CPPUNIT_ASSERT_EQUAL( THINTHICKLG_IN_WIDTH,
            static_cast<long>(line.GetInWidth()) );
    CPPUNIT_ASSERT_EQUAL( THINTHICKLG_DIST_WIDTH,
            static_cast<long>(line.GetDistance()) );
    CPPUNIT_ASSERT_EQUAL( THINTHICKLG_OUT_WIDTH + THINTHICKLG_IN_WIDTH
            + THINTHICKLG_DIST_WIDTH, line.GetWidth() );
}

void BorderLineTest::testGuessWidthNostyleDouble()
{
    SvxBorderLine line;
    line.GuessLinesWidths( css::table::BorderLineStyle::NONE,
            THINTHICKLG_OUT_WIDTH,
            THINTHICKLG_IN_WIDTH,
            THINTHICKLG_DIST_WIDTH );
    CPPUNIT_ASSERT_EQUAL( THINTHICK_LARGEGAP, line.GetBorderLineStyle() );
    CPPUNIT_ASSERT_EQUAL( THINTHICKLG_OUT_WIDTH,
            static_cast<long>(line.GetOutWidth()) );
    CPPUNIT_ASSERT_EQUAL( THINTHICKLG_IN_WIDTH,
            static_cast<long>(line.GetInWidth()) );
    CPPUNIT_ASSERT_EQUAL( THINTHICKLG_DIST_WIDTH,
            static_cast<long>(line.GetDistance()) );
    CPPUNIT_ASSERT_EQUAL( THINTHICKLG_OUT_WIDTH + THINTHICKLG_IN_WIDTH
            + THINTHICKLG_DIST_WIDTH, line.GetWidth() );
}

void BorderLineTest::testGuessWidthNostyleSingle()
{
    SvxBorderLine line;
    line.GuessLinesWidths( css::table::BorderLineStyle::NONE, TEST_WIDTH );
    CPPUNIT_ASSERT_EQUAL( SOLID, line.GetBorderLineStyle() );
    CPPUNIT_ASSERT_EQUAL( TEST_WIDTH, line.GetWidth() );
}

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
