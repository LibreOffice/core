/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/bootstrapfixture.hxx>

#include <algorithm>

#include <vcl/BitmapReadAccess.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drange.hxx>

using namespace drawinglayer;

class CairoPixelProcessor2DTest : public test::BootstrapFixture
{
public:
    CairoPixelProcessor2DTest()
        : BootstrapFixture(true, false)
    {
    }

    // Mask polygons whose object-space coordinates sit far from the origin
    // (e.g. Calc cells thousands of rows down the sheet) used to collapse
    // inside cairo's 24.8 fixed-point pipeline. The resulting cairo_clip
    // was empty, so the mask's children rendered nothing - the original
    // symptom was invisible descender glyphs in Calc. The fix extends
    // processMaskPrimitive2D's coordinate-limit check to also trigger the
    // view-space fallback when the mask polygon's object-space range is
    // far enough from the origin. The test drives a mask polygon at
    // Y ~ 1.5e7 with a contrasting child fill positioned underneath via
    // a TransformPrimitive2D, and verifies that the child is visible after
    // rendering.
    void testFarDownMaskClipsCorrectly()
    {
        constexpr double Y_OFFSET = 15'000'000.0;

        // Map object Y around Y_OFFSET down to view Y around 0. Object Y
        // is well above the 2^17 threshold the workaround uses, but view
        // Y stays in [0, 100] for the visible 100x100 surface.
        basegfx::B2DHomMatrix aViewTrans;
        aViewTrans.translate(0.0, -Y_OFFSET);
        geometry::ViewInformation2D aView;
        aView.setViewTransformation(aViewTrans);

        std::unique_ptr<processor2d::BaseProcessor2D> pProcessor(
            processor2d::createPixelProcessor2DFromScratch(aView, 100, 100, /*bUseRGBA=*/true));

        if (!pProcessor)
        {
            // No system-dependent primitive renderer available in this
            // build (e.g. DISABLE_SYSTEM_DEPENDENT_PRIMITIVE_RENDERER set).
            // Nothing to exercise.
            return;
        }

        // Mask polygon at object Y around Y_OFFSET. Transformed by the
        // view it covers view (10, 0) - (90, 100), well inside the
        // surface.
        basegfx::B2DRange aMaskObjectRange(10.0, Y_OFFSET, 90.0, Y_OFFSET + 100.0);
        basegfx::B2DPolyPolygon aMaskPolygon(
            basegfx::utils::createPolygonFromRect(aMaskObjectRange));

        // Place the red child polygon at small object coordinates and add
        // Y_OFFSET back via a TransformPrimitive2D so that its effective
        // object-to-view transform is identity. This keeps the child's
        // coordinates small at the cairo API boundary, so the test
        // isolates the mask path: only the mask polygon has coordinates
        // that would overflow cairo's 24.8 representation.
        basegfx::B2DHomMatrix aShiftDown;
        aShiftDown.translate(0.0, Y_OFFSET);
        primitive2d::Primitive2DContainer aRedFill{
            rtl::Reference<primitive2d::PolyPolygonColorPrimitive2D>(
                new primitive2d::PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(
                        basegfx::B2DRange(10.0, 0.0, 90.0, 100.0))),
                    COL_LIGHTRED.getBColor()))
        };

        primitive2d::Primitive2DContainer aMaskChildren{
            rtl::Reference<primitive2d::TransformPrimitive2D>(
                new primitive2d::TransformPrimitive2D(aShiftDown, std::move(aRedFill)))
        };

        primitive2d::Primitive2DContainer aPrimitives{ rtl::Reference<primitive2d::MaskPrimitive2D>(
            new primitive2d::MaskPrimitive2D(aMaskPolygon, std::move(aMaskChildren))) };

        pProcessor->process(aPrimitives);

        Bitmap aBitmap(processor2d::extractBitmapFromBaseProcessor2D(pProcessor));
        CPPUNIT_ASSERT(!aBitmap.IsEmpty());

        BitmapScopedReadAccess aAccess(aBitmap);
        CPPUNIT_ASSERT(aAccess);

        // Inside the mask region (view 10..90, 0..100) the child fill
        // should be visible. Without the fix the mask clip collapses
        // to empty and the surface stays at the initial transparent
        // state, so the red channel would be 0.
        const BitmapColor aCenter(aAccess->GetColor(Point(50, 50)));
        CPPUNIT_ASSERT_MESSAGE("mask far down: center pixel red channel high",
                               aCenter.GetRed() > 200);
        CPPUNIT_ASSERT_MESSAGE("mask far down: center pixel green channel low",
                               aCenter.GetGreen() < 50);
        CPPUNIT_ASSERT_MESSAGE("mask far down: center pixel blue channel low",
                               aCenter.GetBlue() < 50);

        // Outside the mask region the surface stays at the initial
        // transparent state regardless of whether the fix is active.
        // Verify that the mask actually clipped (the child polygon
        // alone would have filled x=10..90 with red, so x=5 is a
        // strict outside-the-mask point).
        const BitmapColor aOutside(aAccess->GetColor(Point(5, 50)));
        CPPUNIT_ASSERT_MESSAGE("outside mask: pixel must remain unpainted", aOutside.GetRed() < 50);
    }

    // Y coordinate at which to render the far-down shape tests. This is well
    // above the 2^17 threshold the coordinate-limit workaround uses, so it
    // would overflow cairo's 24.8 fixed-point pipeline if handed to cairo
    // directly.
    static constexpr double SHAPE_LARGE_COORD = 15'000'000.0;

    // Build a 100x100 RGBA processor whose view transform maps object Y around
    // SHAPE_LARGE_COORD down to view Y around 0. Also disable anti-aliasing to
    // avoid confusing results. Returns nullptr when no
    // system-dependent primitive renderer is available in this build.
    static std::unique_ptr<processor2d::BaseProcessor2D> createFarDownProcessor()
    {
        basegfx::B2DHomMatrix aViewTrans;
        aViewTrans.translate(0.0, -SHAPE_LARGE_COORD);
        geometry::ViewInformation2D aView;
        aView.setViewTransformation(aViewTrans);
        aView.setUseAntiAliasing(false);
        return processor2d::createPixelProcessor2DFromScratch(aView, 100, 100, /*bUseRGBA=*/true);
    }

    // A shape inserted far down/right on a Calc sheet carries large absolute
    // coordinates in its leaf primitives, which can be above the limits of
    // the default cairo 24.8 fixed-point representation. Render a shape at
    // large Y value (in object space) and verify the expected pixels are
    // painted.

    void testFarDownShapeFillPaints()
    {
        std::unique_ptr<processor2d::BaseProcessor2D> pProcessor(createFarDownProcessor());
        if (!pProcessor)
            return;

        // Filled rectangle at object Y around SHAPE_LARGE_COORD. The view
        // transform maps it to view (10, 0) - (90, 100), inside the surface.
        primitive2d::Primitive2DContainer aPrimitives{
            rtl::Reference<primitive2d::PolyPolygonColorPrimitive2D>(
                new primitive2d::PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(basegfx::B2DRange(
                        10.0, SHAPE_LARGE_COORD, 90.0, SHAPE_LARGE_COORD + 100.0))),
                    COL_LIGHTRED.getBColor()))
        };

        pProcessor->process(aPrimitives);

        Bitmap aBitmap(processor2d::extractBitmapFromBaseProcessor2D(pProcessor));
        CPPUNIT_ASSERT(!aBitmap.IsEmpty());

        BitmapScopedReadAccess aAccess(aBitmap);
        CPPUNIT_ASSERT(aAccess);

        // Inside the shape the fill must be visible.
        const BitmapColor aCenter(aAccess->GetColor(Point(50, 50)));
        CPPUNIT_ASSERT_MESSAGE("far down fill: center pixel red channel high",
                               aCenter.GetRed() > 200);
        CPPUNIT_ASSERT_MESSAGE("far down fill: center pixel green channel low",
                               aCenter.GetGreen() < 50);
        CPPUNIT_ASSERT_MESSAGE("far down fill: center pixel blue channel low",
                               aCenter.GetBlue() < 50);

        // Outside the shape (x < 10) the surface should stay unpainted.
        const BitmapColor aOutside(aAccess->GetColor(Point(5, 50)));
        CPPUNIT_ASSERT_MESSAGE("far down fill: outside pixel must remain unpainted",
                               aOutside.GetRed() < 50);
    }

    void testFarDownShapeStrokePaints()
    {
        std::unique_ptr<processor2d::BaseProcessor2D> pProcessor(createFarDownProcessor());
        if (!pProcessor)
            return;

        // Thick vertical line at object X = 50, object Y around
        // SHAPE_LARGE_COORD.
        // The view transform maps it to view X = 50, Y 10..90. With width 21
        // the stroke covers view X ~ 40..60, so the center pixel is solid.
        basegfx::B2DPolygon aLine;
        aLine.append(basegfx::B2DPoint(50.0, SHAPE_LARGE_COORD + 10.0));
        aLine.append(basegfx::B2DPoint(50.0, SHAPE_LARGE_COORD + 90.0));

        const attribute::LineAttribute aLineAttribute(COL_LIGHTBLUE.getBColor(), 21.0);
        primitive2d::Primitive2DContainer aPrimitives{
            rtl::Reference<primitive2d::PolygonStrokePrimitive2D>(
                new primitive2d::PolygonStrokePrimitive2D(aLine, aLineAttribute))
        };

        pProcessor->process(aPrimitives);

        Bitmap aBitmap(processor2d::extractBitmapFromBaseProcessor2D(pProcessor));
        CPPUNIT_ASSERT(!aBitmap.IsEmpty());

        BitmapScopedReadAccess aAccess(aBitmap);
        CPPUNIT_ASSERT(aAccess);

        // On the stroke the line color must be visible.
        const BitmapColor aCenter(aAccess->GetColor(Point(50, 50)));
        CPPUNIT_ASSERT_MESSAGE("far down stroke: center pixel blue channel high",
                               aCenter.GetBlue() > 200);
        CPPUNIT_ASSERT_MESSAGE("far down stroke: center pixel red channel low",
                               aCenter.GetRed() < 50);
        CPPUNIT_ASSERT_MESSAGE("far down stroke: center pixel green channel low",
                               aCenter.GetGreen() < 50);

        // Well away from the line (x = 10) the surface stays unpainted.
        const BitmapColor aOutside(aAccess->GetColor(Point(10, 50)));
        CPPUNIT_ASSERT_MESSAGE("far down stroke: off-line pixel must remain unpainted",
                               aOutside.GetBlue() < 50);
    }

    void testFarDownShapeHairlinePaints()
    {
        std::unique_ptr<processor2d::BaseProcessor2D> pProcessor(createFarDownProcessor());
        if (!pProcessor)
            return;

        // Hairline vertical line at object X = 50, object Y around
        // SHAPE_LARGE_COORD, mapping to view X = 50, Y 10..90.
        basegfx::B2DPolygon aLine;
        aLine.append(basegfx::B2DPoint(50.0, SHAPE_LARGE_COORD + 10.0));
        aLine.append(basegfx::B2DPoint(50.0, SHAPE_LARGE_COORD + 90.0));

        primitive2d::Primitive2DContainer aPrimitives{
            rtl::Reference<primitive2d::PolygonHairlinePrimitive2D>(
                new primitive2d::PolygonHairlinePrimitive2D(aLine, COL_LIGHTBLUE.getBColor()))
        };

        pProcessor->process(aPrimitives);

        Bitmap aBitmap(processor2d::extractBitmapFromBaseProcessor2D(pProcessor));
        CPPUNIT_ASSERT(!aBitmap.IsEmpty());

        BitmapScopedReadAccess aAccess(aBitmap);
        CPPUNIT_ASSERT(aAccess);

        // The hairline is 1px wide; its exact column may be 49 or 50 depending
        // on rounding, so scan a small window. It should be blue.
        sal_uInt8 nMaxBlue(0);
        for (tools::Long nX = 48; nX <= 51; ++nX)
            nMaxBlue = std::max(nMaxBlue, aAccess->GetColor(Point(nX, 50)).GetBlue());
        CPPUNIT_ASSERT_MESSAGE("far down hairline: a pixel on the line must be painted blue",
                               nMaxBlue > 200);

        // Well away from the line (x = 10) the surface stays unpainted.
        const BitmapColor aOutside(aAccess->GetColor(Point(10, 50)));
        CPPUNIT_ASSERT_MESSAGE("far down hairline: off-line pixel must remain unpainted",
                               aOutside.GetBlue() < 50);
    }

    CPPUNIT_TEST_SUITE(CairoPixelProcessor2DTest);
    CPPUNIT_TEST(testFarDownMaskClipsCorrectly);
    CPPUNIT_TEST(testFarDownShapeFillPaints);
    CPPUNIT_TEST(testFarDownShapeStrokePaints);
    CPPUNIT_TEST(testFarDownShapeHairlinePaints);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CairoPixelProcessor2DTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
