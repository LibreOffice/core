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

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

namespace basegfx
{

class SvgPathImportExport : public CppUnit::TestFixture
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

    void testImportExport()
    {
        B2DPolyPolygon  aPoly;
        OUString aExport;

        CPPUNIT_ASSERT_MESSAGE("importing simple rectangle from SVG-D",
                               utils::importFromSvgD( aPoly, aPath0, false, nullptr ));
        aExport = utils::exportToSvgD( aPoly, true, true, false );
        const char sExportString[] = "m10 10h-20v-20h20z";
        CPPUNIT_ASSERT_EQUAL_MESSAGE("exporting rectangle to SVG-D",
                               OUString(sExportString), aExport );
        CPPUNIT_ASSERT_MESSAGE("importing simple rectangle from SVG-D (round-trip",
                               utils::importFromSvgD( aPoly, aExport, false, nullptr ));
        aExport = utils::exportToSvgD( aPoly, true, true, false );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("exporting rectangle to SVG-D (round-trip)",
                               OUString(sExportString), aExport);

        CPPUNIT_ASSERT_MESSAGE("importing simple bezier polygon from SVG-D",
                               utils::importFromSvgD( aPoly, aPath1, false, nullptr ));
        aExport = utils::exportToSvgD( aPoly, true, true, false );

        // Adaptations for B2DPolygon bezier change (see #i77162#):

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
        const char sExportStringSimpleBezier[] =
            "m11430 0c-8890 3810 5715 6985 5715 6985"
            "l-17145-1905c0 0 22860-10160 16510 6350"
            "s-3810-11430-3810-11430z";
        CPPUNIT_ASSERT_EQUAL_MESSAGE("exporting bezier polygon to SVG-D", OUString(sExportStringSimpleBezier), aExport);

        // Adaptations for B2DPolygon bezier change (see #i77162#):

        // a 2nd good test is that re-importing of aExport has to create the same
        // B2DPolPolygon again:
        B2DPolyPolygon aReImport;
        CPPUNIT_ASSERT_MESSAGE("importing simple bezier polygon from SVG-D", utils::importFromSvgD( aReImport, aExport, false, nullptr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("re-imported polygon needs to be identical", aPoly, aReImport);

        CPPUNIT_ASSERT_MESSAGE("importing '@' from SVG-D", utils::importFromSvgD( aPoly, aPath2, false, nullptr));
        aExport = utils::exportToSvgD( aPoly, true, true, false );

        // Adaptations for B2DPolygon bezier change (see #i77162#):

        // same here, the corrected export with the corrected B2DPolygon is simply more efficient,
        // so i needed to change the compare string. Also adding the re-import comparison below.
        const char sExportString1[] =
            "m1917 1114c-89-189-233-284-430-284-167 0-306 91-419 273s-170 370-17"
            "0 564c0 145 33 259 98 342 65 84 150 126 257 126q115.5 0 231-57s147-97 210-176 99-143 109-190c38-199 76-398 114"
            "-598zm840 1646c-133 73-312 139-537 197-225 57-440 86-644 87-483-1-866-132-1150-392-284-261-426-619-426-1076 0-"
            "292 67-560 200-803s321-433 562-569 514-204 821-204c405 0 739 125 1003 374 264 250 396 550 396 899 0 313-88 576"
            "-265 787q-265.5 318-627 318c-191 0-308-94-352-281-133 187-315 281-546 281-172 0-315-67-428-200s-170-301-170-50"
            "5c0-277 90-527 271-751 181-223 394-335 640-335 196 0 353 83 470 250 13-68 26-136 41-204q144 0 288 0c-74 376-14"
            "8 752-224 1128-21 101-31 183-31 245 0 39 9 70 26 93 17 24 39 36 67 36 145 0 279-80 400-240s182-365 182-615c0-2"
            "88-107-533-322-734s-487-301-816-301c-395 0-715 124-960 373s-368 569-368 958q0 577.5 357 900c237 216 557 324 95"
            "8 325 189-1 389-27 600-77 211-52 378-110 503-174q40.5 105 81 210z";
        CPPUNIT_ASSERT_MESSAGE("re-importing '@' from SVG-D", utils::importFromSvgD( aReImport, aExport, false, nullptr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("re-imported '@' needs to be identical", aPoly, aReImport);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("exporting '@' to SVG-D", OUString(sExportString1), aExport);
        CPPUNIT_ASSERT_MESSAGE("importing '@' from SVG-D (round-trip",
                               utils::importFromSvgD( aPoly, aExport, false, nullptr ));
        aExport = utils::exportToSvgD( aPoly, true, true, false );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("exporting '@' to SVG-D (round-trip)",
                               OUString(sExportString1), aExport);

        CPPUNIT_ASSERT_MESSAGE("importing complex polygon from SVG-D",
                               utils::importFromSvgD( aPoly, aPath3, false, nullptr ));
        aExport = utils::exportToSvgD( aPoly, true, true, false );
        const char sExportString2[] =
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
        CPPUNIT_ASSERT_EQUAL_MESSAGE("exporting complex polygon to SVG-D",
                               OUString(sExportString2), aExport);
        CPPUNIT_ASSERT_MESSAGE("importing complex polygon from SVG-D (round-trip",
                               utils::importFromSvgD( aPoly, aExport, false, nullptr ));
        aExport = utils::exportToSvgD( aPoly, true, true, false );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("exporting complex polygon to SVG-D (round-trip)",
                               OUString(sExportString2), aExport);

        const B2DPolygon aRect(
            utils::createPolygonFromRect( B2DRange(0.0,0.0,4000.0,4000.0) ));
        aExport = utils::exportToSvgD( B2DPolyPolygon(aRect), false, false, false );

        const char sExportStringRect[] = "M0 0H4000V4000H0Z";
        CPPUNIT_ASSERT_EQUAL_MESSAGE("exporting to rectangle svg-d string",
                               OUString(sExportStringRect), aExport);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(SvgPathImportExport);
    CPPUNIT_TEST(testImportExport);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx::SvgPathImportExport);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
