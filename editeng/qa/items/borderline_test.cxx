/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ insert your name / company etc. here eg. Jim Bob <jim@bob.org> ]
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Jim Bob <jim@bob.org>
 *                 Ted <ted@bear.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <editeng/borderline.hxx>

#define TEST_WIDTH long( 40 )

#define THINTHICKSG_IN_WIDTH long( 15 )
#define THINTHICKSG_OUT_WIDTH long( 40 )
#define THINTHICKSG_DIST_WIDTH long( 15 )

#define THINTHICKLG_IN_WIDTH long( 15 )
#define THINTHICKLG_OUT_WIDTH long( 30 )
#define THINTHICKLG_DIST_WIDTH long( 40 )

namespace {

class BorderLineTest : public CppUnit::TestFixture
{
    public:
        void testGuessWidthDouble();
        void testGuessWidthNoMatch();
        void testGuessWidthThinthickSmallgap();
        void testGuessWidthThinthickLargegap();

    CPPUNIT_TEST_SUITE(BorderLineTest);
    CPPUNIT_TEST(testGuessWidthDouble);
    CPPUNIT_TEST(testGuessWidthNoMatch);
    CPPUNIT_TEST(testGuessWidthThinthickSmallgap);
    CPPUNIT_TEST(testGuessWidthThinthickLargegap);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BorderLineTest);

void BorderLineTest::testGuessWidthDouble()
{
    // Normal double case
    SvxBorderLine line;
    line.SetLinesWidths( DOUBLE, TEST_WIDTH, TEST_WIDTH, TEST_WIDTH );
    CPPUNIT_ASSERT_EQUAL( DOUBLE, line.GetStyle() );
    CPPUNIT_ASSERT_EQUAL( TEST_WIDTH, line.GetWidth() );
}

void BorderLineTest::testGuessWidthNoMatch()
{
    SvxBorderLine line;
    line.SetLinesWidths( DOUBLE,
            1, 2, 3 );
    CPPUNIT_ASSERT_EQUAL( DOUBLE, line.GetStyle() );
    CPPUNIT_ASSERT_EQUAL( long( 0 ), line.GetWidth() );
}

void BorderLineTest::testGuessWidthThinthickSmallgap()
{
    SvxBorderLine line;
    line.SetLinesWidths( DOUBLE,
            THINTHICKSG_IN_WIDTH,
            THINTHICKSG_OUT_WIDTH,
            THINTHICKSG_DIST_WIDTH );
    CPPUNIT_ASSERT_EQUAL( THINTHICK_SMALLGAP, line.GetStyle() );
    CPPUNIT_ASSERT_EQUAL( TEST_WIDTH, line.GetWidth() );
}

void BorderLineTest::testGuessWidthThinthickLargegap()
{
    SvxBorderLine line;
    line.SetLinesWidths( DOUBLE,
            THINTHICKLG_IN_WIDTH,
            THINTHICKLG_OUT_WIDTH,
            THINTHICKLG_DIST_WIDTH );
    CPPUNIT_ASSERT_EQUAL( THINTHICK_LARGEGAP, line.GetStyle() );
    CPPUNIT_ASSERT_EQUAL( TEST_WIDTH, line.GetWidth() );
}

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
