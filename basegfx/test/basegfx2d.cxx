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

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/curve/b2dbeziertools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dtrapezoid.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/range/b1drange.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2dpolyrange.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/color/bcolortools.hxx>

#include <basegfx/tools/rectcliptools.hxx>

#include <iostream>
#include <fstream>

#include <boxclipper.hxx>

using namespace ::basegfx;

namespace basegfx2d
{
class b2dsvgdimpex : public CppUnit::TestFixture
{
private:
    OUString aPath0;
    OUString aPath1;
    OUString aPath2;
    OUString aPath3;

public:
    // initialise your test code values here.
    void setUp() override
    {
        // simple rectangle
        aPath0 = "M 10 10-10 10-10-10 10-10Z";

        // simple bezier polygon
        aPath1 = "m11430 0c-8890 3810 5715 6985 5715 6985 "
            "0 0-17145-1905-17145-1905 0 0 22860-10160 "
            "16510 6350-6350 16510-3810-11430-3810-11430z";

        // '@' as a bezier polygon
        aPath2 = "m1917 1114c-89-189-233-284-430-284-167 "
            "0-306 91-419 273-113 182-170 370-170 564 "
            "0 145 33 259 98 342 65 84 150 126 257 126 "
            "77 0 154-19 231-57 77-38 147-97 210-176 63"
            "-79 99-143 109-190 38-199 76-398 114-598z"
            "m840 1646c-133 73-312 139-537 197-225 57"
            "-440 86-644 87-483-1-866-132-1150-392-284"
            "-261-426-619-426-1076 0-292 67-560 200-803 "
            "133-243 321-433 562-569 241-136 514-204 821"
            "-204 405 0 739 125 1003 374 264 250 396 550 "
            "396 899 0 313-88 576-265 787-177 212-386 318"
            "-627 318-191 0-308-94-352-281-133 187-315 281"
            "-546 281-172 0-315-67-428-200-113-133-170-301"
            "-170-505 0-277 90-527 271-751 181-223 394"
            "-335 640-335 196 0 353 83 470 250 13-68 26"
            "-136 41-204 96 0 192 0 288 0-74 376-148 752"
            "-224 1128-21 101-31 183-31 245 0 39 9 70 26 "
            "93 17 24 39 36 67 36 145 0 279-80 400-240 121"
            "-160 182-365 182-615 0-288-107-533-322-734"
            "-215-201-487-301-816-301-395 0-715 124-960 "
            "373-245 249-368 569-368 958 0 385 119 685 "
            "357 900 237 216 557 324 958 325 189-1 389-27 "
            "600-77 211-52 378-110 503-174 27 70 54 140 81 210z";

        // first part of 'Hello World' as a line polygon
        aPath3 = "m1598 125h306v2334h-306v-1105h-1293v1105h-305v"
            "-2334h305v973h1293zm2159 1015 78-44 85 235-91 "
            "47-91 40-90 34-90 29-89 21-88 16-88 10-88 3-102"
            "-4-97-12-91-19-85-26-40-16-39-18-38-20-36-22-34"
            "-24-33-26-32-27-30-30-29-31-27-33-25-33-23-36-21"
            "-36-19-38-18-40-16-40-26-86-18-91-11-97-4-103 3"
            "-98 11-94 17-89 24-84 31-79 37-75 22-35 23-34 24"
            "-33 27-32 28-30 29-28 31-27 31-24 33-22 34-21 35"
            "-18 37-17 38-14 38-13 41-11 41-8 86-12 91-4 82 4 "
            "78 10 37 9 37 9 36 12 35 14 33 15 33 17 32 19 31 "
            "21 30 22 30 25 55 54 26 29 24 31 22 32 21 33 19 "
            "34 18 36 30 74 23 80 17 84 10 89 3 94v78h-1277l6 "
            "75 10 70 14 66 19 62 23 57 13 26 14 26 15 25 17 "
            "23 17 22 19 21 19 20 21 18 21 18 23 16 23 14 24 "
            "14 26 12 26 11 27 10 28 8 59 13 63 7 67 3 80-3 81"
            "-9 79-14 80-21 78-26 79-32zm-1049-808-12 53h963l"
            "-7-51-11-49-14-46-17-43-21-40-24-38-27-36-31-32"
            "-33-29-35-25-37-22-38-17-40-14-41-9-42-6-44-2-48 "
            "2-46 6-44 9-42 13-40 17-38 21-36 24-34 28-32 32"
            "-29 34-26 38-23 41-20 44-17 47zm1648-1293h288v"
            "2459h-288zm752-2459h288v2459h-288zm1286-1750 86-11 "
            "91-4 91 4 85 12 42 8 39 11 39 13 38 14 36 17 35 18 "
            "34 20 33 23 31 24 30 26 29 28 28 30 26 32 25 33 23 "
            "34 21 35 37 75 31 80 24 84 16 90 11 94 3 100-3 100"
            "-11 95-16 89-24 85-31 80-37 74-21 35-23 35-25 32-26 "
            "32-28 30-29 28-30 26-31 24-33 22-34 21-35 18-36 17"
            "-38 14-39 13-39 10-42 9-85 12-91 4-91-4-86-12-41-9"
            "-40-10-39-13-37-14-36-17-35-18-34-21-33-22-31-24-30"
            "-26-29-28-28-30-26-32-25-32-23-35-21-35-38-74-30-80"
            "-24-85-17-89-11-95-3-100 3-101 11-95 17-90 24-85 30"
            "-79 38-75 21-35 23-35 25-32 26-32 28-30 29-28 30-26 "
            "31-24 33-22 34-20 35-18 36-16 37-15 39-12 40-11z";
    }

    void impex()
    {
        B2DPolyPolygon  aPoly;
        OUString aExport;

        CPPUNIT_ASSERT_MESSAGE("importing simple rectangle from SVG-D",
                               tools::importFromSvgD( aPoly, aPath0, false, nullptr ));
        aExport = tools::exportToSvgD( aPoly, true, true, false );
        const char* sExportString = "m10 10h-20v-20h20z";
        CPPUNIT_ASSERT_MESSAGE("exporting rectangle to SVG-D",
                               aExport.equalsAscii(sExportString) );
        CPPUNIT_ASSERT_MESSAGE("importing simple rectangle from SVG-D (round-trip",
                               tools::importFromSvgD( aPoly, aExport, false, nullptr ));
        aExport = tools::exportToSvgD( aPoly, true, true, false );
        CPPUNIT_ASSERT_MESSAGE("exporting rectangle to SVG-D (round-trip)",
                               aExport.equalsAscii(sExportString));

        CPPUNIT_ASSERT_MESSAGE("importing simple bezier polygon from SVG-D",
                               tools::importFromSvgD( aPoly, aPath1, false, nullptr ));
        aExport = tools::exportToSvgD( aPoly, true, true, false );

        // Adaptions for B2DPolygon bezier change (see #i77162#):

        // The import/export of aPath1 does not reproduce aExport again. This is
        // correct since aPath1 contains a segment with non-used control points
        // which gets exported now correctly as 'l' and also a point (#4, index 3)
        // with C2 continuity which produces a 's' statement now.

        // The old SVGexport identified nun-used ControlVectors erraneously as bezier segments
        // because the 2nd vector at the start point was used, even when added
        // with start point was identical to end point. Exactly for that reason
        // i reworked the B2DPolygon to use prev, next control points.

        // so for correct unit test i add the new exported string here as sExportStringSimpleBezier
        // and compare to it.
        const char* sExportStringSimpleBezier =
            "m11430 0c-8890 3810 5715 6985 5715 6985"
            "l-17145-1905c0 0 22860-10160 16510 6350"
            "s-3810-11430-3810-11430z";
        CPPUNIT_ASSERT_MESSAGE("exporting bezier polygon to SVG-D", aExport.equalsAscii(sExportStringSimpleBezier));

        // Adaptions for B2DPolygon bezier change (see #i77162#):

        // a 2nd good test is that re-importing of aExport has to create the same
        // B2DPolPolygon again:
        B2DPolyPolygon aReImport;
        CPPUNIT_ASSERT_MESSAGE("importing simple bezier polygon from SVG-D", tools::importFromSvgD( aReImport, aExport, false, nullptr));
        CPPUNIT_ASSERT_MESSAGE("re-imported polygon needs to be identical", aReImport == aPoly);

        CPPUNIT_ASSERT_MESSAGE("importing '@' from SVG-D", tools::importFromSvgD( aPoly, aPath2, false, nullptr));
        aExport = tools::exportToSvgD( aPoly, true, true, false );

        // Adaptions for B2DPolygon bezier change (see #i77162#):

        // same here, the corrected export with the corrected B2DPolygon is simply more efficient,
        // so i needed to change the compare string. Also adding the re-import comparison below.
        const char* sExportString1 =
            "m1917 1114c-89-189-233-284-430-284-167 0-306 91-419 273s-170 370-17"
            "0 564c0 145 33 259 98 342 65 84 150 126 257 126q115.5 0 231-57s147-97 210-176 99-143 109-190c38-199 76-398 114"
            "-598zm840 1646c-133 73-312 139-537 197-225 57-440 86-644 87-483-1-866-132-1150-392-284-261-426-619-426-1076 0-"
            "292 67-560 200-803s321-433 562-569 514-204 821-204c405 0 739 125 1003 374 264 250 396 550 396 899 0 313-88 576"
            "-265 787q-265.5 318-627 318c-191 0-308-94-352-281-133 187-315 281-546 281-172 0-315-67-428-200s-170-301-170-50"
            "5c0-277 90-527 271-751 181-223 394-335 640-335 196 0 353 83 470 250 13-68 26-136 41-204q144 0 288 0c-74 376-14"
            "8 752-224 1128-21 101-31 183-31 245 0 39 9 70 26 93 17 24 39 36 67 36 145 0 279-80 400-240s182-365 182-615c0-2"
            "88-107-533-322-734s-487-301-816-301c-395 0-715 124-960 373s-368 569-368 958q0 577.5 357 900c237 216 557 324 95"
            "8 325 189-1 389-27 600-77 211-52 378-110 503-174q40.5 105 81 210z";
        CPPUNIT_ASSERT_MESSAGE("re-importing '@' from SVG-D", tools::importFromSvgD( aReImport, aExport, false, nullptr));
        CPPUNIT_ASSERT_MESSAGE("re-imported '@' needs to be identical", aReImport == aPoly);

        CPPUNIT_ASSERT_MESSAGE("exporting '@' to SVG-D", aExport.equalsAscii(sExportString1));
        CPPUNIT_ASSERT_MESSAGE("importing '@' from SVG-D (round-trip",
                               tools::importFromSvgD( aPoly, aExport, false, nullptr ));
        aExport = tools::exportToSvgD( aPoly, true, true, false );
        CPPUNIT_ASSERT_MESSAGE("exporting '@' to SVG-D (round-trip)",
                               aExport.equalsAscii(sExportString1));

        CPPUNIT_ASSERT_MESSAGE("importing complex polygon from SVG-D",
                               tools::importFromSvgD( aPoly, aPath3, false, nullptr ));
        aExport = tools::exportToSvgD( aPoly, true, true, false );
        const char* sExportString2 =
            "m1598 125h306v2334h-306v-1105h-1293v1105h-305v-2334h305v973h1293"
            "zm2159 1015 78-44 85 235-91 47-91 40-90 34-90 29-89 21-88 16-88 10-88 3-102-4-97"
            "-12-91-19-85-26-40-16-39-18-38-20-36-22-34-24-33-26-32-27-30-30-29-31-27-33-25-3"
            "3-23-36-21-36-19-38-18-40-16-40-26-86-18-91-11-97-4-103 3-98 11-94 17-89 24-84 3"
            "1-79 37-75 22-35 23-34 24-33 27-32 28-30 29-28 31-27 31-24 33-22 34-21 35-18 37-"
            "17 38-14 38-13 41-11 41-8 86-12 91-4 82 4 78 10 37 9 37 9 36 12 35 14 33 15 33 1"
            "7 32 19 31 21 30 22 30 25 55 54 26 29 24 31 22 32 21 33 19 34 18 36 30 74 23 80 "
            "17 84 10 89 3 94v78h-1277l6 75 10 70 14 66 19 62 23 57 13 26 14 26 15 25 17 23 1"
            "7 22 19 21 19 20 21 18 21 18 23 16 23 14 24 14 26 12 26 11 27 10 28 8 59 13 63 7"
            " 67 3 80-3 81-9 79-14 80-21 78-26 79-32zm-1049-808-12 53h963l-7-51-11-49-14-46-1"
            "7-43-21-40-24-38-27-36-31-32-33-29-35-25-37-22-38-17-40-14-41-9-42-6-44-2-48 2-4"
            "6 6-44 9-42 13-40 17-38 21-36 24-34 28-32 32-29 34-26 38-23 41-20 44-17 47zm1648"
            "-1293h288v2459h-288zm752-2459h288v2459h-288zm1286-1750 86-11 91-4 91 4 85 12 42 "
            "8 39 11 39 13 38 14 36 17 35 18 34 20 33 23 31 24 30 26 29 28 28 30 26 32 25 33 "
            "23 34 21 35 37 75 31 80 24 84 16 90 11 94 3 100-3 100-11 95-16 89-24 85-31 80-37"
            " 74-21 35-23 35-25 32-26 32-28 30-29 28-30 26-31 24-33 22-34 21-35 18-36 17-38 1"
            "4-39 13-39 10-42 9-85 12-91 4-91-4-86-12-41-9-40-10-39-13-37-14-36-17-35-18-34-2"
            "1-33-22-31-24-30-26-29-28-28-30-26-32-25-32-23-35-21-35-38-74-30-80-24-85-17-89-"
            "11-95-3-100 3-101 11-95 17-90 24-85 30-79 38-75 21-35 23-35 25-32 26-32 28-30 29"
            "-28 30-26 31-24 33-22 34-20 35-18 36-16 37-15 39-12 40-11z";
        CPPUNIT_ASSERT_MESSAGE("exporting complex polygon to SVG-D",
                               aExport.equalsAscii(sExportString2));
        CPPUNIT_ASSERT_MESSAGE("importing complex polygon from SVG-D (round-trip",
                               tools::importFromSvgD( aPoly, aExport, false, nullptr ));
        aExport = tools::exportToSvgD( aPoly, true, true, false );
        CPPUNIT_ASSERT_MESSAGE("exporting complex polygon to SVG-D (round-trip)",
                               aExport.equalsAscii(sExportString2));

        const B2DPolygon aRect(
            tools::createPolygonFromRect( B2DRange(0.0,0.0,4000.0,4000.0) ));
        aExport = tools::exportToSvgD( B2DPolyPolygon(aRect), false, false, false );

        const char* sExportStringRect = "M0 0H4000V4000H0Z";
        CPPUNIT_ASSERT_MESSAGE("exporting to rectangle svg-d string",
                               aExport.equalsAscii(sExportStringRect));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dsvgdimpex);
    CPPUNIT_TEST(impex);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dsvgdimpex

class b2drange : public CppUnit::TestFixture
{
private:
public:
    void check()
    {
        CPPUNIT_ASSERT_MESSAGE("simple range rounding from double to integer",
                               fround(B2DRange(1.2, 2.3, 3.5, 4.8)) == B2IRange(1, 2, 4, 5));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2drange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
};

class b2dpolyrange : public CppUnit::TestFixture
{
private:
public:
    void check()
    {
        B2DPolyRange aRange;
        aRange.appendElement(B2DRange(0,0,1,1),B2VectorOrientation::Positive);
        aRange.appendElement(B2DRange(2,2,3,3),B2VectorOrientation::Positive);

        CPPUNIT_ASSERT_MESSAGE("simple poly range - count",
                               aRange.count() == 2);
        CPPUNIT_ASSERT_MESSAGE("simple poly range - first element",
                               std::get<0>(aRange.getElement(0)) == B2DRange(0,0,1,1));
        CPPUNIT_ASSERT_MESSAGE("simple poly range - second element",
                               std::get<0>(aRange.getElement(1)) == B2DRange(2,2,3,3));

        // B2DPolyRange relies on correctly orientated rects
        const B2DRange aRect(0,0,1,1);
        CPPUNIT_ASSERT_MESSAGE("createPolygonFromRect - correct orientation",
                               tools::getOrientation(
                                   tools::createPolygonFromRect(aRect)) == B2VectorOrientation::Positive );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolyrange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
};

class b2dhommatrix : public CppUnit::TestFixture
{
private:
    B2DHomMatrix    maIdentity;
    B2DHomMatrix    maScale;
    B2DHomMatrix    maTranslate;
    B2DHomMatrix    maShear;
    B2DHomMatrix    maAffine;
    B2DHomMatrix    maPerspective;

public:
    // initialise your test code values here.
    void setUp() override
    {
        // setup some test matrices
        maIdentity.identity(); // force compact layout
        maIdentity.set(0,0, 1.0);
        maIdentity.set(0,1, 0.0);
        maIdentity.set(0,2, 0.0);
        maIdentity.set(1,0, 0.0);
        maIdentity.set(1,1, 1.0);
        maIdentity.set(1,2, 0.0);

        maScale.identity(); // force compact layout
        maScale.set(0,0, 2.0);
        maScale.set(1,1, 20.0);

        maTranslate.identity(); // force compact layout
        maTranslate.set(0,2, 20.0);
        maTranslate.set(1,2, 2.0);

        maShear.identity(); // force compact layout
        maShear.set(0,1, 3.0);
        maShear.set(1,0, 7.0);
        maShear.set(1,1, 22.0);

        maAffine.identity(); // force compact layout
        maAffine.set(0,0, 1.0);
        maAffine.set(0,1, 2.0);
        maAffine.set(0,2, 3.0);
        maAffine.set(1,0, 4.0);
        maAffine.set(1,1, 5.0);
        maAffine.set(1,2, 6.0);

        maPerspective.set(0,0, 1.0);
        maPerspective.set(0,1, 2.0);
        maPerspective.set(0,2, 3.0);
        maPerspective.set(1,0, 4.0);
        maPerspective.set(1,1, 5.0);
        maPerspective.set(1,2, 6.0);
        maPerspective.set(2,0, 7.0);
        maPerspective.set(2,1, 8.0);
        maPerspective.set(2,2, 9.0);
    }

    void equal()
    {
        B2DHomMatrix    aIdentity;
        B2DHomMatrix    aScale;
        B2DHomMatrix    aTranslate;
        B2DHomMatrix    aShear;
        B2DHomMatrix    aAffine;
        B2DHomMatrix    aPerspective;

        // setup some test matrices
        aIdentity.identity(); // force compact layout
        aIdentity.set(0,0, 1.0);
        aIdentity.set(0,1, 0.0);
        aIdentity.set(0,2, 0.0);
        aIdentity.set(1,0, 0.0);
        aIdentity.set(1,1, 1.0);
        aIdentity.set(1,2, 0.0);

        aScale.identity(); // force compact layout
        aScale.set(0,0, 2.0);
        aScale.set(1,1, 20.0);

        aTranslate.identity(); // force compact layout
        aTranslate.set(0,2, 20.0);
        aTranslate.set(1,2, 2.0);

        aShear.identity(); // force compact layout
        aShear.set(0,1, 3.0);
        aShear.set(1,0, 7.0);
        aShear.set(1,1, 22.0);

        aAffine.identity(); // force compact layout
        aAffine.set(0,0, 1.0);
        aAffine.set(0,1, 2.0);
        aAffine.set(0,2, 3.0);
        aAffine.set(1,0, 4.0);
        aAffine.set(1,1, 5.0);
        aAffine.set(1,2, 6.0);

        aPerspective.set(0,0, 1.0);
        aPerspective.set(0,1, 2.0);
        aPerspective.set(0,2, 3.0);
        aPerspective.set(1,0, 4.0);
        aPerspective.set(1,1, 5.0);
        aPerspective.set(1,2, 6.0);
        aPerspective.set(2,0, 7.0);
        aPerspective.set(2,1, 8.0);
        aPerspective.set(2,2, 9.0);

        CPPUNIT_ASSERT_MESSAGE("operator==: identity matrix", aIdentity == maIdentity);
        CPPUNIT_ASSERT_MESSAGE("operator==: scale matrix", aScale == maScale);
        CPPUNIT_ASSERT_MESSAGE("operator==: translate matrix", aTranslate == maTranslate);
        CPPUNIT_ASSERT_MESSAGE("operator==: shear matrix", aShear == maShear);
        CPPUNIT_ASSERT_MESSAGE("operator==: affine matrix", aAffine == maAffine);
        CPPUNIT_ASSERT_MESSAGE("operator==: perspective matrix", aPerspective == maPerspective);
    }

    void identity()
    {
        B2DHomMatrix ident;

        CPPUNIT_ASSERT_MESSAGE("identity", maIdentity == ident);
    }

    void scale()
    {
        B2DHomMatrix mat;
        mat.scale(2.0,20.0);
        CPPUNIT_ASSERT_MESSAGE("scale", maScale == mat);
    }

    void rotate()
    {
        B2DHomMatrix mat;
        mat.rotate(90*F_PI180);
        CPPUNIT_ASSERT_MESSAGE("rotate pi/2 yields exact matrix",
                               mat.get(0,0) == 0.0 &&
                               mat.get(0,1) == -1.0 &&
                               mat.get(0,2) == 0.0 &&
                               mat.get(1,0) == 1.0 &&
                               mat.get(1,1) == 0.0 &&
                               mat.get(1,2) == 0.0);
        mat.rotate(90*F_PI180);
        CPPUNIT_ASSERT_MESSAGE("rotate pi yields exact matrix",
                               mat.get(0,0) == -1.0 &&
                               mat.get(0,1) == 0.0 &&
                               mat.get(0,2) == 0.0 &&
                               mat.get(1,0) == 0.0 &&
                               mat.get(1,1) == -1.0 &&
                               mat.get(1,2) == 0.0);
        mat.rotate(90*F_PI180);
        CPPUNIT_ASSERT_MESSAGE("rotate 3/2 pi yields exact matrix",
                               mat.get(0,0) == 0.0 &&
                               mat.get(0,1) == 1.0 &&
                               mat.get(0,2) == 0.0 &&
                               mat.get(1,0) == -1.0 &&
                               mat.get(1,1) == 0.0 &&
                               mat.get(1,2) == 0.0);
        mat.rotate(90*F_PI180);
        CPPUNIT_ASSERT_MESSAGE("rotate 2 pi yields exact matrix",
                               mat.get(0,0) == 1.0 &&
                               mat.get(0,1) == 0.0 &&
                               mat.get(0,2) == 0.0 &&
                               mat.get(1,0) == 0.0 &&
                               mat.get(1,1) == 1.0 &&
                               mat.get(1,2) == 0.0);
    }

    void translate()
    {
        B2DHomMatrix mat;
        mat.translate(20.0,2.0);
        CPPUNIT_ASSERT_MESSAGE("translate", maTranslate == mat);
    }

    void shear()
    {
        B2DHomMatrix mat;
        mat.shearX(3.0);
        mat.shearY(7.0);
        CPPUNIT_ASSERT_MESSAGE("translate", maShear == mat);
    }

    void multiply()
    {
        B2DHomMatrix affineAffineProd;

        affineAffineProd.set(0,0, 9);
        affineAffineProd.set(0,1, 12);
        affineAffineProd.set(0,2, 18);
        affineAffineProd.set(1,0, 24);
        affineAffineProd.set(1,1, 33);
        affineAffineProd.set(1,2, 48);

        B2DHomMatrix affinePerspectiveProd;

        affinePerspectiveProd.set(0,0, 30);
        affinePerspectiveProd.set(0,1, 36);
        affinePerspectiveProd.set(0,2, 42);
        affinePerspectiveProd.set(1,0, 66);
        affinePerspectiveProd.set(1,1, 81);
        affinePerspectiveProd.set(1,2, 96);
        affinePerspectiveProd.set(2,0, 7);
        affinePerspectiveProd.set(2,1, 8);
        affinePerspectiveProd.set(2,2, 9);

        B2DHomMatrix perspectiveAffineProd;

        perspectiveAffineProd.set(0,0, 9);
        perspectiveAffineProd.set(0,1, 12);
        perspectiveAffineProd.set(0,2, 18);
        perspectiveAffineProd.set(1,0, 24);
        perspectiveAffineProd.set(1,1, 33);
        perspectiveAffineProd.set(1,2, 48);
        perspectiveAffineProd.set(2,0, 39);
        perspectiveAffineProd.set(2,1, 54);
        perspectiveAffineProd.set(2,2, 78);

        B2DHomMatrix perspectivePerspectiveProd;

        perspectivePerspectiveProd.set(0,0, 30);
        perspectivePerspectiveProd.set(0,1, 36);
        perspectivePerspectiveProd.set(0,2, 42);
        perspectivePerspectiveProd.set(1,0, 66);
        perspectivePerspectiveProd.set(1,1, 81);
        perspectivePerspectiveProd.set(1,2, 96);
        perspectivePerspectiveProd.set(2,0, 102);
        perspectivePerspectiveProd.set(2,1, 126);
        perspectivePerspectiveProd.set(2,2, 150);

        B2DHomMatrix temp;

        temp = maAffine;
        temp*=maAffine;
        CPPUNIT_ASSERT_MESSAGE("multiply: both compact", temp == affineAffineProd);

        temp = maPerspective;
        temp*=maAffine;
        CPPUNIT_ASSERT_MESSAGE("multiply: first compact", temp == affinePerspectiveProd);

        temp = maAffine;
        temp*=maPerspective;
        CPPUNIT_ASSERT_MESSAGE("multiply: second compact", temp == perspectiveAffineProd);

        temp = maPerspective;
        temp*=maPerspective;
        CPPUNIT_ASSERT_MESSAGE("multiply: none compact", temp == perspectivePerspectiveProd);
    }

    void impFillMatrix(B2DHomMatrix& rSource, double fScaleX, double fScaleY, double fShearX, double fRotate)
    {
        // fill rSource with a linear combination of scale, shear and rotate
        rSource.identity();
        rSource.scale(fScaleX, fScaleY);
        rSource.shearX(fShearX);
        rSource.rotate(fRotate);
    }

    bool impDecomposeComposeTest(double fScaleX, double fScaleY, double fShearX, double fRotate)
    {
        // linear combine matrix with given values
        B2DHomMatrix aSource;
        impFillMatrix(aSource, fScaleX, fScaleY, fShearX, fRotate);

        // decompose that matrix
        B2DTuple aDScale;
        B2DTuple aDTrans;
        double fDRot;
        double fDShX;
        bool bWorked = aSource.decompose(aDScale, aDTrans, fDRot, fDShX);

        // linear combine another matrix with decomposition results
        B2DHomMatrix aRecombined;
        impFillMatrix(aRecombined, aDScale.getX(), aDScale.getY(), fDShX, fDRot);

        // if decomposition worked, matrices need to be the same
        return bWorked && aSource == aRecombined;
    }

    void decompose()
    {
        // test matrix decompositions. Each matrix decomposed and rebuilt
        // using the decompose result should be the same as before. Test
        // with all ranges of values. Translations are not tested since these
        // are just the two rightmost values and uncritical
        static double fSX(10.0);
        static double fSY(12.0);
        static double fR(45.0 * F_PI180);
        static double fS(15.0 * F_PI180);

        // check all possible scaling combinations
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A1", impDecomposeComposeTest(fSX, fSY, 0.0, 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A2", impDecomposeComposeTest(-fSX, fSY, 0.0, 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A3", impDecomposeComposeTest(fSX, -fSY, 0.0, 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test A4", impDecomposeComposeTest(-fSX, -fSY, 0.0, 0.0));

        // check all possible scaling combinations with positive rotation
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B1", impDecomposeComposeTest(fSX, fSY, 0.0, fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B2", impDecomposeComposeTest(-fSX, fSY, 0.0, fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B3", impDecomposeComposeTest(fSX, -fSY, 0.0, fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test B4", impDecomposeComposeTest(-fSX, -fSY, 0.0, fR));

        // check all possible scaling combinations with negative rotation
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C1", impDecomposeComposeTest(fSX, fSY, 0.0, -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C2", impDecomposeComposeTest(-fSX, fSY, 0.0, -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C3", impDecomposeComposeTest(fSX, -fSY, 0.0, -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test C4", impDecomposeComposeTest(-fSX, -fSY, 0.0, -fR));

        // check all possible scaling combinations with positive shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D1", impDecomposeComposeTest(fSX, fSY, tan(fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D2", impDecomposeComposeTest(-fSX, fSY, tan(fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D3", impDecomposeComposeTest(fSX, -fSY, tan(fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test D4", impDecomposeComposeTest(-fSX, -fSY, tan(fS), 0.0));

        // check all possible scaling combinations with negative shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E1", impDecomposeComposeTest(fSX, fSY, tan(-fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E2", impDecomposeComposeTest(-fSX, fSY, tan(-fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E3", impDecomposeComposeTest(fSX, -fSY, tan(-fS), 0.0));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test E4", impDecomposeComposeTest(-fSX, -fSY, tan(-fS), 0.0));

        // check all possible scaling combinations with positive rotate and positive shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F1", impDecomposeComposeTest(fSX, fSY, tan(fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F2", impDecomposeComposeTest(-fSX, fSY, tan(fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F3", impDecomposeComposeTest(fSX, -fSY, tan(fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test F4", impDecomposeComposeTest(-fSX, -fSY, tan(fS), fR));

        // check all possible scaling combinations with negative rotate and positive shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G1", impDecomposeComposeTest(fSX, fSY, tan(fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G2", impDecomposeComposeTest(-fSX, fSY, tan(fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G3", impDecomposeComposeTest(fSX, -fSY, tan(fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test G4", impDecomposeComposeTest(-fSX, -fSY, tan(fS), -fR));

        // check all possible scaling combinations with positive rotate and negative shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H1", impDecomposeComposeTest(fSX, fSY, tan(-fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H2", impDecomposeComposeTest(-fSX, fSY, tan(-fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H3", impDecomposeComposeTest(fSX, -fSY, tan(-fS), fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test H4", impDecomposeComposeTest(-fSX, -fSY, tan(-fS), fR));

        // check all possible scaling combinations with negative rotate and negative shear
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I1", impDecomposeComposeTest(fSX, fSY, tan(-fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I2", impDecomposeComposeTest(-fSX, fSY, tan(-fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I3", impDecomposeComposeTest(fSX, -fSY, tan(-fS), -fR));
        CPPUNIT_ASSERT_MESSAGE("decompose: error test I4", impDecomposeComposeTest(-fSX, -fSY, tan(-fS), -fR));

        // cover special case of 180 degree rotation
        B2DHomMatrix aTest=tools::createScaleShearXRotateTranslateB2DHomMatrix(
            6425,3938,
            0,
            180*F_PI180,
            10482,4921);
        // decompose that matrix
        B2DTuple aDScale;
        B2DTuple aDTrans;
        double fDRot;
        double fDShX;
        aTest.decompose(aDScale, aDTrans, fDRot, fDShX);
        CPPUNIT_ASSERT_MESSAGE("decompose: error test J1", aDScale.getX() == 6425 && aDScale.getY() == 3938);
        CPPUNIT_ASSERT_MESSAGE("decompose: error test J1", aDTrans.getX() == 10482 && aDTrans.getY() == 4921);
        CPPUNIT_ASSERT_MESSAGE("decompose: error test J1", fDRot == 180*F_PI180);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dhommatrix);
    CPPUNIT_TEST(equal);
    CPPUNIT_TEST(identity);
    CPPUNIT_TEST(scale);
    CPPUNIT_TEST(translate);
    CPPUNIT_TEST(rotate);
    CPPUNIT_TEST(shear);
    CPPUNIT_TEST(multiply);
    CPPUNIT_TEST(decompose);
    CPPUNIT_TEST_SUITE_END();

}; // class b2dhommatrix

class b2dpoint : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    // this is only demonstration code
    void EmptyMethod()
    {
          // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpoint);
    CPPUNIT_TEST(EmptyMethod);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dpoint

class b2dpolygon : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    void testBasics()
    {
        B2DPolygon aPoly;

        aPoly.appendBezierSegment(B2DPoint(1,1),B2DPoint(2,2),B2DPoint(3,3));

        CPPUNIT_ASSERT_MESSAGE("#1 first polygon point wrong",
                               aPoly.getB2DPoint(0) == B2DPoint(3,3));
        CPPUNIT_ASSERT_MESSAGE("#1 first control point wrong",
                               aPoly.getPrevControlPoint(0) == B2DPoint(2,2));
        CPPUNIT_ASSERT_MESSAGE("#1 second control point wrong",
                               aPoly.getNextControlPoint(0) == B2DPoint(3,3));
        CPPUNIT_ASSERT_MESSAGE("next control point not used",
                               !aPoly.isNextControlPointUsed(0));

        aPoly.setNextControlPoint(0,B2DPoint(4,4));
        CPPUNIT_ASSERT_MESSAGE("#1.1 second control point wrong",
                               aPoly.getNextControlPoint(0) == B2DPoint(4,4));
        CPPUNIT_ASSERT_MESSAGE("next control point used",
                               aPoly.isNextControlPointUsed(0));
        CPPUNIT_ASSERT_MESSAGE("areControlPointsUsed() wrong",
                               aPoly.areControlPointsUsed());
        CPPUNIT_ASSERT_MESSAGE("getContinuityInPoint() wrong",
                               aPoly.getContinuityInPoint(0) == B2VectorContinuity::C2);

        aPoly.resetControlPoints();
        CPPUNIT_ASSERT_MESSAGE("resetControlPoints() did not clear",
                               aPoly.getB2DPoint(0) == B2DPoint(3,3));
        CPPUNIT_ASSERT_MESSAGE("resetControlPoints() did not clear",
                               aPoly.getPrevControlPoint(0) == B2DPoint(3,3));
        CPPUNIT_ASSERT_MESSAGE("resetControlPoints() did not clear",
                               aPoly.getNextControlPoint(0) == B2DPoint(3,3));
        CPPUNIT_ASSERT_MESSAGE("areControlPointsUsed() wrong #2",
                               !aPoly.areControlPointsUsed());

        aPoly.clear();
        aPoly.append(B2DPoint(0,0));
        aPoly.appendBezierSegment(B2DPoint(1,1),B2DPoint(2,2),B2DPoint(3,3));

        CPPUNIT_ASSERT_MESSAGE("#2 first polygon point wrong",
                               aPoly.getB2DPoint(0) == B2DPoint(0,0));
        CPPUNIT_ASSERT_MESSAGE("#2 first control point wrong",
                               aPoly.getPrevControlPoint(0) == B2DPoint(0,0));
        CPPUNIT_ASSERT_MESSAGE("#2 second control point wrong",
                               aPoly.getNextControlPoint(0) == B2DPoint(1,1));
        CPPUNIT_ASSERT_MESSAGE("#2 third control point wrong",
                               aPoly.getPrevControlPoint(1) == B2DPoint(2,2));
        CPPUNIT_ASSERT_MESSAGE("#2 fourth control point wrong",
                               aPoly.getNextControlPoint(1) == B2DPoint(3,3));
        CPPUNIT_ASSERT_MESSAGE("#2 second polygon point wrong",
                               aPoly.getB2DPoint(1) == B2DPoint(3,3));
    }
    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolygon);
    CPPUNIT_TEST(testBasics);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dpolygon

class b2dpolygontools : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    // this is only demonstration code
    void testIsRectangle()
    {
        B2DPolygon aRect1(
            tools::createPolygonFromRect(
                B2DRange(0,0,1,1) ) );

        B2DPolygon aRect2 {
            {0, 0},
            {1, 0},
            {1, 0.5},
            {1, 1},
            {0, 1}
        };
        aRect2.setClosed(true);

        B2DPolygon aNonRect1 {
            {0, 0},
            {1, 0},
            {0.5, 1},
            {0.5, 0}
        };
        aNonRect1.setClosed(true);

        B2DPolygon aNonRect2 {
            {0, 0},
            {1, 1},
            {1, 0},
            {0, 1}
        };
        aNonRect2.setClosed(true);

        B2DPolygon aNonRect3 {
            {0, 0},
            {1, 0},
            {1, 1}
        };
        aNonRect3.setClosed(true);

        B2DPolygon aNonRect4 {
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1}
        };

        B2DPolygon aNonRect5 {
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1}
        };
        aNonRect5.setControlPoints(1, B2DPoint(1,0), B2DPoint(-11,0));
        aNonRect5.setClosed(true);

        CPPUNIT_ASSERT_MESSAGE("checking rectangle-ness of rectangle 1",
                               tools::isRectangle( aRect1 ));
        CPPUNIT_ASSERT_MESSAGE("checking rectangle-ness of rectangle 2",
                               tools::isRectangle( aRect2 ));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 1",
                               !tools::isRectangle( aNonRect1 ));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 2",
                               !tools::isRectangle( aNonRect2 ));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 3",
                               !tools::isRectangle( aNonRect3 ));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 4",
                               !tools::isRectangle( aNonRect4 ));
        CPPUNIT_ASSERT_MESSAGE("checking non-rectangle-ness of polygon 5",
                               !tools::isRectangle( aNonRect5 ));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolygontools);
    CPPUNIT_TEST(testIsRectangle);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dpolygontools

class b2dpolypolygon : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    void testTrapezoidHelper()
    {
        B2DPolygon aPolygon;
        // provoke the PointBlockAllocator to exercise the freeIfLast path
        for(int i = 0; i < 16 * 10; i++)
        {
            B2DPoint aPoint(getRandomOrdinal(1000), getRandomOrdinal(1000));
            aPolygon.append(aPoint);
        }
        // scatter some duplicate points in to stress things more.
        for(int i = 0; i < 16 * 10; i++)
        {
            aPolygon.insert(getRandomOrdinal(aPolygon.count() - 1),
                            aPolygon.getB2DPoint(getRandomOrdinal(aPolygon.count() - 1)));
        }
        B2DPolygon aPolygonOffset;
        // duplicate the polygon and offset it slightly.
        for(size_t i = 0; i < aPolygon.count(); i++)
        {
            B2DPoint aPoint(aPolygon.getB2DPoint(i));
            aPoint += B2DPoint(0.5-getRandomOrdinal(1),0.5-getRandomOrdinal(1));
        }
        B2DPolyPolygon aPolyPolygon;
        aPolyPolygon.append(aPolygon);
        aPolyPolygon.append(aPolygonOffset);
        B2DTrapezoidVector aVector;
        basegfx::tools::trapezoidSubdivide(aVector, aPolyPolygon);
        CPPUNIT_ASSERT_MESSAGE("more than zero sub-divided trapezoids",
                               aVector.size() > 0);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dpolypolygon);
    CPPUNIT_TEST(testTrapezoidHelper);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dpolypolygon

class b1Xrange : public CppUnit::TestFixture
{
public:
    template<class Type> void implCheck()
    {
        // test interval axioms
        // (http://en.wikipedia.org/wiki/Interval_%28mathematics%29)
        Type aRange;
        CPPUNIT_ASSERT_MESSAGE("default ctor - empty range", aRange.isEmpty());
        CPPUNIT_ASSERT_MESSAGE("center - get cop-out value since range is empty", aRange.getCenter()==0);

        // degenerate interval
        aRange.expand(1);
        CPPUNIT_ASSERT_MESSAGE("degenerate range - still, not empty!", !aRange.isEmpty());
        CPPUNIT_ASSERT_MESSAGE("degenerate range - size of 0", aRange.getRange() == 0);
        CPPUNIT_ASSERT_MESSAGE("same value as degenerate range - is inside range", aRange.isInside(1));
        CPPUNIT_ASSERT_MESSAGE("center - must be the single range value", aRange.getCenter()==1);

        // proper interval
        aRange.expand(2);
        CPPUNIT_ASSERT_MESSAGE("proper range - size of 1", aRange.getRange() == 1);
        CPPUNIT_ASSERT_MESSAGE("smaller value of range - is inside *closed* range", aRange.isInside(1));
        CPPUNIT_ASSERT_MESSAGE("larger value of range - is inside *closed* range", aRange.isInside(2));

        // center for proper interval that works for ints, too
        aRange.expand(3);
        CPPUNIT_ASSERT_MESSAGE("center - must be half of the range", aRange.getCenter()==2);

        // check overlap
        Type aRange2(0,1);
        CPPUNIT_ASSERT_MESSAGE("range overlapping *includes* upper bound", aRange.overlaps(aRange2));
        CPPUNIT_ASSERT_MESSAGE("range overlapping *includes* upper bound, but only barely", !aRange.overlapsMore(aRange2));

        Type aRange3(0,2);
        CPPUNIT_ASSERT_MESSAGE("range overlapping is fully overlapping now", aRange.overlapsMore(aRange3));

        // check intersect
        Type aRange4(3,4);
        aRange.intersect(aRange4);
        CPPUNIT_ASSERT_MESSAGE("range intersection is yielding empty range!", !aRange.isEmpty());

        Type aRange5(5,6);
        aRange.intersect(aRange5);
        CPPUNIT_ASSERT_MESSAGE("range intersection is yielding nonempty range!", aRange.isEmpty());

        // just so that this compiles -
        Type aRange6( aRange );
        (void)aRange6;
    }

    void check()
    {
        implCheck<B1DRange>();
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b1Xrange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
}; // class b1Xrange

class b2Xrange : public CppUnit::TestFixture
{
public:
    template<class Type> void implCheck()
    {
        // cohen sutherland clipping
        Type aRange(0,0,10,10);

        CPPUNIT_ASSERT_MESSAGE("(0,0) is outside range!",
                               tools::getCohenSutherlandClipFlags(B2IPoint(0,0),aRange) == 0);
        CPPUNIT_ASSERT_MESSAGE("(-1,-1) is inside range!",
                               tools::getCohenSutherlandClipFlags(B2IPoint(-1,-1),aRange) ==
                               (tools::RectClipFlags::LEFT|tools::RectClipFlags::TOP));
        CPPUNIT_ASSERT_MESSAGE("(10,10) is outside range!",
                               tools::getCohenSutherlandClipFlags(B2IPoint(10,10),aRange) == 0);
        CPPUNIT_ASSERT_MESSAGE("(11,11) is inside range!",
                               tools::getCohenSutherlandClipFlags(B2IPoint(11,11),aRange) ==
                               (tools::RectClipFlags::RIGHT|tools::RectClipFlags::BOTTOM));

        // just so that this compiles -
        Type aRange1( aRange );
        (void)aRange1;
    }

    void check()
    {
        implCheck<B2DRange>();
        implCheck<B2IRange>();
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2Xrange);
    CPPUNIT_TEST(check);
    CPPUNIT_TEST_SUITE_END();
}; // class b2Xrange

class b2ibox : public CppUnit::TestFixture
{
public:
    void TestBox()
    {
        // cohen sutherland clipping
        B2IBox aBox(0,0,10,10);

        CPPUNIT_ASSERT_MESSAGE("(0,0) is outside range!",
                               tools::getCohenSutherlandClipFlags(B2IPoint(0,0),aBox) == 0);
        CPPUNIT_ASSERT_MESSAGE("(-1,-1) is inside range!",
                               tools::getCohenSutherlandClipFlags(B2IPoint(-1,-1),aBox) ==
                               (tools::RectClipFlags::LEFT|tools::RectClipFlags::TOP));
        CPPUNIT_ASSERT_MESSAGE("(9,9) is outside range!",
                               tools::getCohenSutherlandClipFlags(B2IPoint(9,9),aBox) == 0);
        CPPUNIT_ASSERT_MESSAGE("(10,10) is inside range!",
                               tools::getCohenSutherlandClipFlags(B2IPoint(10,10),aBox) ==
                               (tools::RectClipFlags::RIGHT|tools::RectClipFlags::BOTTOM));

        // just so that this compiles -
        B2IBox aBox1( aBox );
        (void)aBox1;
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.
    CPPUNIT_TEST_SUITE(b2ibox);
    CPPUNIT_TEST(TestBox);
    CPPUNIT_TEST_SUITE_END();
}; // class b2ibox

class b2dtuple : public CppUnit::TestFixture
{
public:
    // insert your test code here.
    // this is only demonstration code
    void EmptyMethod()
    {
          // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b2dtuple);
    CPPUNIT_TEST(EmptyMethod);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dtuple

class bcolor : public CppUnit::TestFixture
{
    BColor maWhite;
    BColor maBlack;
    BColor maRed;
    BColor maGreen;
    BColor maBlue;
    BColor maYellow;
    BColor maMagenta;
    BColor maCyan;

public:
    bcolor() :
        maWhite(1,1,1),
        maBlack(0,0,0),
        maRed(1,0,0),
        maGreen(0,1,0),
        maBlue(0,0,1),
        maYellow(1,1,0),
        maMagenta(1,0,1),
        maCyan(0,1,1)
    {}

    // insert your test code here.
    void hslTest()
    {
        CPPUNIT_ASSERT_MESSAGE("white",
                               tools::rgb2hsl(maWhite) == BColor(0,0,1));
        CPPUNIT_ASSERT_MESSAGE("black",
                               tools::rgb2hsl(maBlack) == BColor(0,0,0));
        CPPUNIT_ASSERT_MESSAGE("red",
                               tools::rgb2hsl(maRed) == BColor(0,1,0.5));
        CPPUNIT_ASSERT_MESSAGE("green",
                               tools::rgb2hsl(maGreen) == BColor(120,1,0.5));
        CPPUNIT_ASSERT_MESSAGE("blue",
                               tools::rgb2hsl(maBlue) == BColor(240,1,0.5));
        CPPUNIT_ASSERT_MESSAGE("yellow",
                               tools::rgb2hsl(maYellow) == BColor(60,1,0.5));
        CPPUNIT_ASSERT_MESSAGE("magenta",
                               tools::rgb2hsl(maMagenta) == BColor(300,1,0.5));
        CPPUNIT_ASSERT_MESSAGE("cyan",
                               tools::rgb2hsl(maCyan) == BColor(180,1,0.5));
        CPPUNIT_ASSERT_MESSAGE("third hue case",
                               tools::rgb2hsl(BColor(0,0.5,1)) == BColor(210,1,0.5));

        CPPUNIT_ASSERT_MESSAGE("roundtrip white",
                               tools::hsl2rgb(tools::rgb2hsl(maWhite)) == maWhite);
        CPPUNIT_ASSERT_MESSAGE("roundtrip black",
                               tools::hsl2rgb(tools::rgb2hsl(maBlack)) == maBlack);
        CPPUNIT_ASSERT_MESSAGE("roundtrip red",
                               tools::hsl2rgb(tools::rgb2hsl(maRed)) == maRed);
        CPPUNIT_ASSERT_MESSAGE("roundtrip green",
                               tools::hsl2rgb(tools::rgb2hsl(maGreen)) == maGreen);
        CPPUNIT_ASSERT_MESSAGE("roundtrip blue",
                               tools::hsl2rgb(tools::rgb2hsl(maBlue)) == maBlue);
        CPPUNIT_ASSERT_MESSAGE("roundtrip yellow",
                               tools::hsl2rgb(tools::rgb2hsl(maYellow)) == maYellow);
        CPPUNIT_ASSERT_MESSAGE("roundtrip magenta",
                               tools::hsl2rgb(tools::rgb2hsl(maMagenta)) == maMagenta);
        CPPUNIT_ASSERT_MESSAGE("roundtrip cyan",
                               tools::hsl2rgb(tools::rgb2hsl(maCyan)) == maCyan);

        CPPUNIT_ASSERT_MESSAGE("grey10",
                               tools::rgb2hsl(maWhite*.1) == BColor(0,0,.1));
        CPPUNIT_ASSERT_MESSAGE("grey90",
                               tools::rgb2hsl(maWhite*.9) == BColor(0,0,.9));
        CPPUNIT_ASSERT_MESSAGE("red/2",
                               tools::rgb2hsl(maRed*.5) == BColor(0,1,0.25));
        CPPUNIT_ASSERT_MESSAGE("green/2",
                               tools::rgb2hsl(maGreen*.5) == BColor(120,1,0.25));
        CPPUNIT_ASSERT_MESSAGE("blue/2",
                               tools::rgb2hsl(maBlue*.5) == BColor(240,1,0.25));
        CPPUNIT_ASSERT_MESSAGE("yellow/2",
                               tools::rgb2hsl(maYellow*.5) == BColor(60,1,0.25));
        CPPUNIT_ASSERT_MESSAGE("magenta/2",
                               tools::rgb2hsl(maMagenta*.5) == BColor(300,1,0.25));
        CPPUNIT_ASSERT_MESSAGE("cyan/2",
                               tools::rgb2hsl(maCyan*.5) == BColor(180,1,0.25));

        CPPUNIT_ASSERT_MESSAGE("pastel",
                               tools::rgb2hsl(BColor(.75,.25,.25)) == BColor(0,.5,.5));
    }

    // insert your test code here.
    void hsvTest()
    {
        CPPUNIT_ASSERT_MESSAGE("white",
                               tools::rgb2hsv(maWhite) == BColor(0,0,1));
        CPPUNIT_ASSERT_MESSAGE("black",
                               tools::rgb2hsv(maBlack) == BColor(0,0,0));
        CPPUNIT_ASSERT_MESSAGE("red",
                               tools::rgb2hsv(maRed) == BColor(0,1,1));
        CPPUNIT_ASSERT_MESSAGE("green",
                               tools::rgb2hsv(maGreen) == BColor(120,1,1));
        CPPUNIT_ASSERT_MESSAGE("blue",
                               tools::rgb2hsv(maBlue) == BColor(240,1,1));
        CPPUNIT_ASSERT_MESSAGE("yellow",
                               tools::rgb2hsv(maYellow) == BColor(60,1,1));
        CPPUNIT_ASSERT_MESSAGE("magenta",
                               tools::rgb2hsv(maMagenta) == BColor(300,1,1));
        CPPUNIT_ASSERT_MESSAGE("cyan",
                               tools::rgb2hsv(maCyan) == BColor(180,1,1));

        CPPUNIT_ASSERT_MESSAGE("roundtrip white",
                               tools::hsv2rgb(tools::rgb2hsv(maWhite)) == maWhite);
        CPPUNIT_ASSERT_MESSAGE("roundtrip black",
                               tools::hsv2rgb(tools::rgb2hsv(maBlack)) == maBlack);
        CPPUNIT_ASSERT_MESSAGE("roundtrip red",
                               tools::hsv2rgb(tools::rgb2hsv(maRed)) == maRed);
        CPPUNIT_ASSERT_MESSAGE("roundtrip green",
                               tools::hsv2rgb(tools::rgb2hsv(maGreen)) == maGreen);
        CPPUNIT_ASSERT_MESSAGE("roundtrip blue",
                               tools::hsv2rgb(tools::rgb2hsv(maBlue)) == maBlue);
        CPPUNIT_ASSERT_MESSAGE("roundtrip yellow",
                               tools::hsv2rgb(tools::rgb2hsv(maYellow)) == maYellow);
        CPPUNIT_ASSERT_MESSAGE("roundtrip magenta",
                               tools::hsv2rgb(tools::rgb2hsv(maMagenta)) == maMagenta);
        CPPUNIT_ASSERT_MESSAGE("roundtrip cyan",
                               tools::hsv2rgb(tools::rgb2hsv(maCyan)) == maCyan);

        CPPUNIT_ASSERT_MESSAGE("grey10",
                               tools::rgb2hsv(maWhite*.1) == BColor(0,0,.1));
        CPPUNIT_ASSERT_MESSAGE("grey90",
                               tools::rgb2hsv(maWhite*.9) == BColor(0,0,.9));
        CPPUNIT_ASSERT_MESSAGE("red/2",
                               tools::rgb2hsv(maRed*.5) == BColor(0,1,0.5));
        CPPUNIT_ASSERT_MESSAGE("green/2",
                               tools::rgb2hsv(maGreen*.5) == BColor(120,1,0.5));
        CPPUNIT_ASSERT_MESSAGE("blue/2",
                               tools::rgb2hsv(maBlue*.5) == BColor(240,1,0.5));
        CPPUNIT_ASSERT_MESSAGE("yellow/2",
                               tools::rgb2hsv(maYellow*.5) == BColor(60,1,0.5));
        CPPUNIT_ASSERT_MESSAGE("magenta/2",
                               tools::rgb2hsv(maMagenta*.5) == BColor(300,1,0.5));
        CPPUNIT_ASSERT_MESSAGE("cyan/2",
                               tools::rgb2hsv(maCyan*.5) == BColor(180,1,0.5));

        CPPUNIT_ASSERT_MESSAGE("pastel",
                               tools::rgb2hsv(BColor(.5,.25,.25)) == BColor(0,.5,.5));
    }

    void ciexyzTest()
    {
        tools::rgb2ciexyz(maWhite);
        tools::rgb2ciexyz(maBlack);
        tools::rgb2ciexyz(maRed);
        tools::rgb2ciexyz(maGreen);
        tools::rgb2ciexyz(maBlue);
        tools::rgb2ciexyz(maYellow);
        tools::rgb2ciexyz(maMagenta);
        tools::rgb2ciexyz(maCyan);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(bcolor);
    CPPUNIT_TEST(hslTest);
    CPPUNIT_TEST(hsvTest);
    CPPUNIT_TEST(ciexyzTest);
    CPPUNIT_TEST_SUITE_END();
}; // class b2dvector

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b2dsvgdimpex);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b2drange);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b2dpolyrange);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b2dhommatrix);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b2dpoint);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b2dpolygon);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b2dpolygontools);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b2dpolypolygon);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b1Xrange);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b2Xrange);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b2ibox);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::b2dtuple);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::bcolor);
} // namespace basegfx2d

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
