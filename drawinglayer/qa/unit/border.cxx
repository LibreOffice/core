/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/table/BorderLineStyle.hpp>

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <rtl/ref.hxx>

using namespace com::sun::star;

namespace
{

class DrawinglayerBorderTest : public CppUnit::TestFixture
{
public:
    void testDoubleDecompositionSolid();

    CPPUNIT_TEST_SUITE(DrawinglayerBorderTest);
    CPPUNIT_TEST(testDoubleDecompositionSolid);
    CPPUNIT_TEST_SUITE_END();
};

void DrawinglayerBorderTest::testDoubleDecompositionSolid()
{
    // Create a border line primitive that's similar to the one from the bugdoc:
    // 1.47 pixels is 0.03cm at 130% zoom and 96 DPI.
    basegfx::B2DPoint aStart(0, 20);
    basegfx::B2DPoint aEnd(100, 20);
    double fLeftWidth = 1.47;
    double fDistance = 1.47;
    double fRightWidth = 1.47;
    double fExtendLeftStart = 0;
    double fExtendLeftEnd = 0;
    double fExtendRightStart = 0;
    double fExtendRightEnd = 0;
    basegfx::BColor aColorRight;
    basegfx::BColor aColorLeft;
    basegfx::BColor aColorGap;
    bool bHasGapColor = false;
    sal_Int16 nStyle = table::BorderLineStyle::DOUBLE;
    rtl::Reference<drawinglayer::primitive2d::BorderLinePrimitive2D> aBorder(new drawinglayer::primitive2d::BorderLinePrimitive2D(aStart, aEnd, fLeftWidth, fDistance, fRightWidth, fExtendLeftStart, fExtendLeftEnd, fExtendRightStart, fExtendRightEnd, aColorRight, aColorLeft, aColorGap, bHasGapColor, nStyle));

    // Decompose it into polygons.
    drawinglayer::geometry::ViewInformation2D aView;
    drawinglayer::primitive2d::Primitive2DContainer aContainer = aBorder->get2DDecomposition(aView);

    // Make sure it results in two borders as it's a double one.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), aContainer.size());

    // Get the inside line.
    auto pInside = dynamic_cast<const drawinglayer::primitive2d::PolyPolygonColorPrimitive2D*>(aContainer[0].get());
    CPPUNIT_ASSERT(pInside);

    // Make sure the inside line's height is fLeftWidth.
    const basegfx::B2DPolyPolygon& rPolyPolygon = pInside->getB2DPolyPolygon();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(1), rPolyPolygon.count());
    const basegfx::B2DPolygon& rPolygon = rPolyPolygon.getB2DPolygon(0);
    const basegfx::B2DRange& rRange = rPolygon.getB2DRange();
    // This was 2.47, i.e. the width of the inner line was 1 unit (in the bugdoc's case: 1 pixel) wider than expected.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(fLeftWidth, rRange.getHeight(), basegfx::fTools::getSmallValue());
}

CPPUNIT_TEST_SUITE_REGISTRATION(DrawinglayerBorderTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
