/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "../gfxtypes.hxx"
#include "../parserfragments.hxx"

using namespace svgi;

class TestParser : public CppUnit::TestFixture
{
public:
    void setUp()
    {}

    void tearDown()
    {}

    void testParseColor()
    {
        ARGBColor aTmp;

        const char* sIn="#102030  ";
        ARGBColor aOut(16, 32, 48);
        CPPUNIT_ASSERT_MESSAGE( "Consuming color #112233",
                                parseColor( sIn, aTmp ) );
        OSL_TRACE("color is: a:%f r:%f g:%f b:%f", aTmp.a, aTmp.r, aTmp.g, aTmp.b);
        CPPUNIT_ASSERT_MESSAGE( "Parsing color #112233",
                                aOut==aTmp );

        sIn="  #321";
        aOut=ARGBColor(51, 34, 17);
        CPPUNIT_ASSERT_MESSAGE( "Consuming color #321",
                                parseColor( sIn, aTmp ) );
        OSL_TRACE("color is: a:%f r:%f g:%f b:%f", aTmp.a, aTmp.r, aTmp.g, aTmp.b);
        CPPUNIT_ASSERT_MESSAGE( "Parsing color #321",
                                aOut==aTmp );

        sIn="rgb(100,200,\t 50)";
        aOut=ARGBColor(100, 200, 50);
        CPPUNIT_ASSERT_MESSAGE( "Consuming color rgb(100,200,50)",
                                parseColor( sIn, aTmp ) );
        OSL_TRACE("color is: a:%f r:%f g:%f b:%f", aTmp.a, aTmp.r, aTmp.g, aTmp.b);
        CPPUNIT_ASSERT_MESSAGE( "Parsing color rgb(100,200,50)",
                                aOut==aTmp );

        sIn="rgb(0.1, \t0.2,0.9)";
        aOut=ARGBColor(0.1, 0.2, 0.9);
        CPPUNIT_ASSERT_MESSAGE( "Consuming color rgb(0.1,0.2,0.9)",
                                parseColor( sIn, aTmp ) );
        OSL_TRACE("color is: a:%f r:%f g:%f b:%f", aTmp.a, aTmp.r, aTmp.g, aTmp.b);
        CPPUNIT_ASSERT_MESSAGE( "Parsing color rgb(0.1,0.2,0.9)",
                                aOut==aTmp );

        sIn=" burlywood ";
        aOut=ARGBColor(222,184,135);
        CPPUNIT_ASSERT_MESSAGE( "Consuming color burlywood",
                                parseColor( sIn, aTmp ) );
        OSL_TRACE("color is: a:%f r:%f g:%f b:%f", aTmp.a, aTmp.r, aTmp.g, aTmp.b);
        CPPUNIT_ASSERT_MESSAGE( "Parsing color burlywood",
                                aOut==aTmp );
    }

    void testParseOpacity()
    {
        ARGBColor aTmp;

        const char* sIn=" 0.123  ";
        ARGBColor aOut(0.123, 0.0, 0.0, 0.0);
        CPPUNIT_ASSERT_MESSAGE( "Consuming opacity 0.123",
                                parseOpacity( sIn, aTmp ) );
        OSL_TRACE("color is: a:%f r:%f g:%f b:%f", aTmp.a, aTmp.r, aTmp.g, aTmp.b);
        CPPUNIT_ASSERT_MESSAGE( "Parsing opacity 0.123",
                                aOut==aTmp );
    }

    void testParseTransform()
    {
        basegfx::B2DHomMatrix aOut;

        const char* sIn=" none  ";
        basegfx::B2DHomMatrix aTmp;
        CPPUNIT_ASSERT_MESSAGE( "Consuming transformation none",
                                parseTransform( sIn, aTmp ) );
        OSL_TRACE("transformation is: m00:%f m01:%f m02:%f m10:%f m11:%f m12:%f",
                  aTmp.get(0,0), aTmp.get(0,1), aTmp.get(0,2), aTmp.get(1,0), aTmp.get(1,1), aTmp.get(1,2) );
        CPPUNIT_ASSERT_MESSAGE( "Parsing transformation none",
                                aOut==aTmp );

        sIn=" scale( 10 )  ";
        aOut.identity();
        aOut.scale(10.0,10.0);
        CPPUNIT_ASSERT_MESSAGE( "Consuming transformation scale(10)",
                                parseTransform( sIn, aTmp ) );
        OSL_TRACE("transformation is: m00:%f m01:%f m02:%f m10:%f m11:%f m12:%f",
                  aTmp.get(0,0), aTmp.get(0,1), aTmp.get(0,2), aTmp.get(1,0), aTmp.get(1,1), aTmp.get(1,2) );
        CPPUNIT_ASSERT_MESSAGE( "Parsing transformation scale(10)",
                                aOut==aTmp );

        sIn=" scale( 10 20.12 )  ";
        aOut.identity();
        aOut.scale(10.0,20.12);
        CPPUNIT_ASSERT_MESSAGE( "Consuming transformation scale(10 20.12)",
                                parseTransform( sIn, aTmp ) );
        OSL_TRACE("transformation is: m00:%f m01:%f m02:%f m10:%f m11:%f m12:%f",
                  aTmp.get(0,0), aTmp.get(0,1), aTmp.get(0,2), aTmp.get(1,0), aTmp.get(1,1), aTmp.get(1,2) );
        CPPUNIT_ASSERT_MESSAGE( "Parsing transformation scale(10 20.12)",
                                aOut==aTmp );

        sIn="matrix( 1,2 3,4,5 6 )";
        aOut.identity();
        aOut.set(0,0,1.0); aOut.set(1,0,2.0); aOut.set(0,1,3.0); aOut.set(1,1,4.0); aOut.set(0,2,5.0); aOut.set(1,2,6.0);
        CPPUNIT_ASSERT_MESSAGE( "Consuming transformation matrix(1,2,3,4,5,6)",
                                parseTransform( sIn, aTmp ) );
        OSL_TRACE("transformation is: m00:%f m01:%f m02:%f m10:%f m11:%f m12:%f",
                  aTmp.get(0,0), aTmp.get(0,1), aTmp.get(0,2), aTmp.get(1,0), aTmp.get(1,1), aTmp.get(1,2) );
        CPPUNIT_ASSERT_MESSAGE( "Parsing transformation matrix(1,2,3,4,5,6)",
                                aOut==aTmp );

        sIn="matrix( 1 0 0 1 -10 -10 ) translate(10) scale(10), rotate(90)";
        aOut.identity();
        aOut.set(0,0,0.0); aOut.set(1,0,10.0); aOut.set(0,1,-10.0); aOut.set(1,1,0.0); aOut.set(0,2,0.0); aOut.set(1,2,0.0);
        CPPUNIT_ASSERT_MESSAGE( "Consuming transformation matrix(1,2,3,4,5,6)",
                                parseTransform( sIn, aTmp ) );
        OSL_TRACE("transformation is: m00:%f m01:%f m02:%f m10:%f m11:%f m12:%f",
                  aTmp.get(0,0), aTmp.get(0,1), aTmp.get(0,2), aTmp.get(1,0), aTmp.get(1,1), aTmp.get(1,2) );
        CPPUNIT_ASSERT_MESSAGE( "Parsing transformation matrix(1,2,3,4,5,6)",
                                aOut==aTmp );

        sIn="skewX(45)";
        aOut.identity();
        aOut.set(0,0,1.0); aOut.set(1,0,1.0); aOut.set(0,1,0.0); aOut.set(1,1,1.0); aOut.set(0,2,0.0); aOut.set(1,2,0.0);
        CPPUNIT_ASSERT_MESSAGE( "Consuming transformation skewX(45)",
                                parseTransform( sIn, aTmp ) );
        OSL_TRACE("transformation is: m00:%f m01:%f m02:%f m10:%f m11:%f m12:%f",
                  aTmp.get(0,0), aTmp.get(0,1), aTmp.get(0,2), aTmp.get(1,0), aTmp.get(1,1), aTmp.get(1,2) );
        CPPUNIT_ASSERT_MESSAGE( "Parsing transformation skewX(45)",
                                aOut==aTmp );

        sIn="skewY(45)";
        aOut.identity();
        aOut.set(0,0,1.0); aOut.set(1,0,0.0); aOut.set(0,1,1.0); aOut.set(1,1,1.0); aOut.set(0,2,0.0); aOut.set(1,2,0.0);
        CPPUNIT_ASSERT_MESSAGE( "Consuming transformation skewY(45)",
                                parseTransform( sIn, aTmp ) );
        OSL_TRACE("transformation is: m00:%f m01:%f m02:%f m10:%f m11:%f m12:%f",
                  aTmp.get(0,0), aTmp.get(0,1), aTmp.get(0,2), aTmp.get(1,0), aTmp.get(1,1), aTmp.get(1,2) );
        CPPUNIT_ASSERT_MESSAGE( "Parsing transformation skewY(45)",
                                aOut==aTmp );
    }

    void testParseViewBox()
    {
        basegfx::B2DRange aTmp;

        const char* sIn=" 10 20, 30.5,5  ";
        basegfx::B2DRange aOut(10,20,40.5,25);
        CPPUNIT_ASSERT_MESSAGE( "Consuming 10,20,30.5,5",
                                parseViewBox( sIn, aTmp ) );
        OSL_TRACE("viewbox is: x1:%f y1:%f x2:%f y2:%f", aTmp.getMinX(), aTmp.getMinY(), aTmp.getMaxX(), aTmp.getMaxY());
        CPPUNIT_ASSERT_MESSAGE( "Parsing 10,20,30.5,5",
                                aOut==aTmp );
    }

    void testParseDashArray()
    {
        std::vector<double> aTmp;

        const char* sIn=" 10,20, -10.00  ";
        std::vector<double> aOut; aOut.push_back(10.0); aOut.push_back(20.0); aOut.push_back(-10.0);
        CPPUNIT_ASSERT_MESSAGE( "Consuming 10,20,-10.00",
                                parseDashArray( sIn, aTmp ) );
        OSL_TRACE("dash array is: len %d, %f %f %f", aTmp.size(), aTmp[0], aTmp[1], aTmp[2] );
        CPPUNIT_ASSERT_MESSAGE( "Parsing 10,20,-10.00",
                                aOut==aTmp );
    }

    CPPUNIT_TEST_SUITE(TestParser);
    CPPUNIT_TEST(testParseColor);
    CPPUNIT_TEST(testParseOpacity);
    CPPUNIT_TEST(testParseTransform);
    CPPUNIT_TEST(testParseViewBox);
    CPPUNIT_TEST(testParseDashArray);
    // TODO: CPPUNIT_TEST(testParseXlinkHref);
    CPPUNIT_TEST_SUITE_END();
};

// -----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestParser, "test svg parser fragments");

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
