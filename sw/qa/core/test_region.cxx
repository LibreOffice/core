/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <swregion.hxx>
#include <vcl/region.hxx>

class RegionUnittest : public CppUnit::TestFixture
{
public:
    void testCompress();
    void testInvert();

    CPPUNIT_TEST_SUITE(RegionUnittest);
    CPPUNIT_TEST(testCompress);
    CPPUNIT_TEST(testInvert);
    CPPUNIT_TEST_SUITE_END();
};

void RegionUnittest::testCompress()
{
    SwRegionRects region;

    // All inside each other, check it'll compress them to the largest one.
    region = SwRegionRects();
    region += SwRect(Point(10, 10), Size(10, 10));
    region += SwRect(Point(10, 10), Size(20, 20));
    region += SwRect(Point(10, 10), Size(100, 100));
    region += SwRect(Point(10, 10), Size(50, 50));
    region.Compress(SwRegionRects::CompressExact);
    CPPUNIT_ASSERT_EQUAL(size_t(1), region.size());
    CPPUNIT_ASSERT_EQUAL(SwRect(Point(10, 10), Size(100, 100)), region[0]);

    // Check merging of adjacent rectangles. This will merge first two groups
    // and then those two merged rects only in the next iteration.
    region = SwRegionRects();
    region += SwRect(Point(10, 10), Size(10000, 10000));
    region += SwRect(Point(10010, 10), Size(10000, 10000));
    region += SwRect(Point(10, 10010), Size(10000, 10000));
    region += SwRect(Point(10010, 10010), Size(10000, 10000));
    region.Compress(SwRegionRects::CompressExact);
    CPPUNIT_ASSERT_EQUAL(size_t(1), region.size());
    CPPUNIT_ASSERT_EQUAL(SwRect(Point(10, 10), Size(20000, 20000)), region[0]);

    // Check fuzzy compress, two almost aligned rects will be compressed to one.
    region = SwRegionRects();
    region += SwRect(Point(10, 10), Size(100, 100));
    region += SwRect(Point(110, 10), Size(100, 90));
    region.Compress(SwRegionRects::CompressExact);
    CPPUNIT_ASSERT_EQUAL(size_t(2), region.size());
    region.Compress(SwRegionRects::CompressFuzzy);
    CPPUNIT_ASSERT_EQUAL(size_t(1), region.size());
    CPPUNIT_ASSERT_EQUAL(SwRect(Point(10, 10), Size(200, 100)), region[0]);

    // Check it doesn't crash because of empty size.
    region = SwRegionRects();
    region += SwRect(Point(0, 0), Size(0, 0));
    region += SwRect(Point(10, 10), Size(0, 0));
    region += SwRect(Point(100, 100), Size(0, 0));
    region.Compress(SwRegionRects::CompressExact);
    region.Compress(SwRegionRects::CompressFuzzy);
}

void RegionUnittest::testInvert()
{
    // Check that punching holes and inverting has the same result as adding up rects.
    const SwRect fullRect(Point(100, 100), Size(1000, 1000));
    const SwRect rects[]
        = { { Point(200, 200), Size(200, 200) },
            { Point(0, 0), Size(200, 200) }, // this one is intentionally partially out of bounds
            { Point(700, 760), Size(20, 150) },
            { Point(100, 300), Size(200, 200) },
            { Point(100, 800), Size(10, 10) }, // these two partially overlap
            { Point(105, 805), Size(10, 10) } };
    SwRegionRects region1(fullRect);
    for (const SwRect& rect : rects)
        region1 -= rect;
    region1.Invert();
    region1.Compress(SwRegionRects::CompressExact);
    SwRegionRects region2;
    region2.ChangeOrigin(fullRect);
    for (const SwRect& rect : rects)
        region2 += rect;
    region2.LimitToOrigin();
    region2.Compress(SwRegionRects::CompressExact);
    // The regions should be the same area, but not necessarily the same representation.
    // SwRegionRects cannot compare those easily, but vcl::Region can.
    vcl::Region vclRegion1, vclRegion2;
    for (const SwRect& rect : region1)
        vclRegion1.Union(tools::Rectangle(rect.Pos(), rect.SSize()));
    for (const SwRect& rect : region2)
        vclRegion2.Union(tools::Rectangle(rect.Pos(), rect.SSize()));
    CPPUNIT_ASSERT_EQUAL(vclRegion1, vclRegion2);
}

CPPUNIT_TEST_SUITE_REGISTRATION(RegionUnittest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
