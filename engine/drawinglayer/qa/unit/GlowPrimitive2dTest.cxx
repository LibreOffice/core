/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/callgrind.hxx>

#include <vcl/virdev.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/glowprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>

using namespace drawinglayer;

namespace
{
class GlowPrimitive2dTest : public test::BootstrapFixture
{
public:
    GlowPrimitive2dTest()
        : BootstrapFixture(true, false)
    {
    }

    static primitive2d::Primitive2DContainer createFilledRectangle(const basegfx::B2DRange& rRange,
                                                                   const basegfx::BColor& rColor)
    {
        return primitive2d::Primitive2DContainer{
            rtl::Reference<primitive2d::PolyPolygonColorPrimitive2D>(
                new primitive2d::PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(rRange)), rColor))
        };
    }

    // An L shape, so that there is one concave corner at (300, 350) to look at.
    static primitive2d::Primitive2DContainer createLShape(const basegfx::BColor& rColor)
    {
        basegfx::B2DPolygon aPolygon;
        aPolygon.append(basegfx::B2DPoint(200, 200));
        aPolygon.append(basegfx::B2DPoint(400, 200));
        aPolygon.append(basegfx::B2DPoint(400, 350));
        aPolygon.append(basegfx::B2DPoint(300, 350));
        aPolygon.append(basegfx::B2DPoint(300, 500));
        aPolygon.append(basegfx::B2DPoint(200, 500));
        aPolygon.setClosed(true);

        return primitive2d::Primitive2DContainer{
            rtl::Reference<primitive2d::PolyPolygonColorPrimitive2D>(
                new primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPolygon),
                                                             rColor))
        };
    }

    // Paints the glow behind the content on a white page, the order a glowing shape is drawn in.
    static Bitmap renderGlow(const primitive2d::Primitive2DContainer& rContent,
                             const Color& rGlowColor, double fGlowRadius, sal_Int32 nWidth,
                             sal_Int32 nHeight,
                             const basegfx::B2DRange& rViewport = basegfx::B2DRange())
    {
        ScopedVclPtr<VirtualDevice> pDevice
            = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
        pDevice->SetOutputSizePixel(Size(nWidth, nHeight));
        pDevice->SetBackground(Wallpaper(COL_WHITE));
        pDevice->Erase();

        // A glow needs a pixel target, which a non-identity object to view transformation stands
        // for. A tiny translation gives one without moving the content.
        basegfx::B2DHomMatrix aViewTransform;
        aViewTransform.translate(0.001, 0.001);

        geometry::ViewInformation2D aViewInformation;
        aViewInformation.setViewTransformation(aViewTransform);

        // An empty viewport means everything is visible, which is what the other tests want.
        if (!rViewport.isEmpty())
            aViewInformation.setViewport(rViewport);

        std::unique_ptr<processor2d::BaseProcessor2D> pProcessor(
            processor2d::createProcessor2DFromOutputDevice(*pDevice, aViewInformation));

        primitive2d::Primitive2DContainer aScene(2);
        aScene[0] = new primitive2d::GlowPrimitive2D(rGlowColor, fGlowRadius,
                                                     primitive2d::Primitive2DContainer(rContent));
        aScene[1] = new primitive2d::GroupPrimitive2D(primitive2d::Primitive2DContainer(rContent));

        pProcessor->process(aScene);

        return pDevice->GetBitmap(Point(), pDevice->GetOutputSizePixel());
    }

    // How far the pixel is from the white page, which is how much halo covers it.
    static sal_uInt16 getHaloStrength(BitmapScopedReadAccess& rAccess, sal_Int32 nX, sal_Int32 nY)
    {
        return rAccess->GetColor(Point(nX, nY)).GetColorError(COL_WHITE);
    }

    static primitive2d::Primitive2DContainer createGlowingRectangle()
    {
        return createFilledRectangle(basegfx::B2DRange(200, 200, 400, 400),
                                     basegfx::BColor(0.36, 0.61, 0.84));
    }

    // Best of a handful of renders, printed in milliseconds. The best rather than the mean, since
    // a reading is only ever spoiled upwards by whatever else the machine is doing.
    void timeOneGlow(double fShapeSize, double fGlowRadius, sal_Int32 nCanvasSize)
    {
        const primitive2d::Primitive2DContainer aContent
            = createFilledRectangle(basegfx::B2DRange(200, 200, 200 + fShapeSize, 200 + fShapeSize),
                                    basegfx::BColor(0.36, 0.61, 0.84));

        // One render first, so the reading is not the one that faults the buffers in.
        renderGlow(aContent, Color(0xED, 0x7D, 0x31), fGlowRadius, nCanvasSize, nCanvasSize);

        double fBest = std::numeric_limits<double>::max();

        for (int nRun = 0; nRun < 5; ++nRun)
        {
            const auto aStart = std::chrono::steady_clock::now();
            renderGlow(aContent, Color(0xED, 0x7D, 0x31), fGlowRadius, nCanvasSize, nCanvasSize);
            const std::chrono::duration<double, std::milli> aElapsed(
                std::chrono::steady_clock::now() - aStart);
            fBest = std::min(fBest, aElapsed.count());
        }

        std::cout << "glow " << fShapeSize << " wide, radius " << fGlowRadius << ": " << fBest
                  << " ms" << std::endl;
    }
};

CPPUNIT_TEST_FIXTURE(GlowPrimitive2dTest, testHaloSurroundsShape)
{
    Bitmap aBitmap = renderGlow(createGlowingRectangle(), Color(0xED, 0x7D, 0x31), 50.0, 600, 600);
    BitmapScopedReadAccess aAccess(aBitmap);
    CPPUNIT_ASSERT(aAccess);

    // Far outside the glow the page is left alone.
    CPPUNIT_ASSERT_LESS(sal_uInt16(5), getHaloStrength(aAccess, 50, 300));

    // Strong just outside the shape, weaker but still there near the outer end of the radius.
    const sal_uInt16 nNear = getHaloStrength(aAccess, 190, 300);
    const sal_uInt16 nFar = getHaloStrength(aAccess, 170, 300);
    CPPUNIT_ASSERT_GREATER(sal_uInt16(100), nNear);
    CPPUNIT_ASSERT_GREATER(sal_uInt16(20), nFar);
    CPPUNIT_ASSERT_GREATER(nFar, nNear);

    // The shape itself keeps its own fill and is not covered by the halo.
    CPPUNIT_ASSERT_LESS(sal_uInt16(10),
                        aAccess->GetColor(Point(300, 300)).GetColorError(Color(92, 156, 215)));
}

CPPUNIT_TEST_FIXTURE(GlowPrimitive2dTest, testHaloFadesOutwards)
{
    Bitmap aBitmap = renderGlow(createGlowingRectangle(), Color(0xED, 0x7D, 0x31), 50.0, 600, 600);
    BitmapScopedReadAccess aAccess(aBitmap);
    CPPUNIT_ASSERT(aAccess);

    // Walking away from the right edge of the shape at x = 400.
    const sal_uInt16 nAt1Pixel = getHaloStrength(aAccess, 401, 300);
    const sal_uInt16 nAt10Pixels = getHaloStrength(aAccess, 410, 300);
    const sal_uInt16 nAt20Pixels = getHaloStrength(aAccess, 420, 300);
    const sal_uInt16 nAt35Pixels = getHaloStrength(aAccess, 435, 300);

    // The halo is strong right next to the shape and still carries well at ten pixels out.
    CPPUNIT_ASSERT_GREATER(sal_uInt16(200), nAt1Pixel);
    CPPUNIT_ASSERT_GREATER(sal_uInt16(100), nAt10Pixels);

    CPPUNIT_ASSERT_GREATEREQUAL(nAt10Pixels, nAt1Pixel);
    CPPUNIT_ASSERT_GREATEREQUAL(nAt20Pixels, nAt10Pixels);
    CPPUNIT_ASSERT_GREATEREQUAL(nAt35Pixels, nAt20Pixels);
}

CPPUNIT_TEST_FIXTURE(GlowPrimitive2dTest, testHaloEndsWithinRadius)
{
    Bitmap aBitmap = renderGlow(createGlowingRectangle(), Color(0xED, 0x7D, 0x31), 50.0, 600, 600);
    BitmapScopedReadAccess aAccess(aBitmap);
    CPPUNIT_ASSERT(aAccess);

    // 20 pixels out of a radius of 50 still carries halo, 60 pixels out is past the end of it.
    CPPUNIT_ASSERT_GREATER(sal_uInt16(50), getHaloStrength(aAccess, 420, 300));
    CPPUNIT_ASSERT_LESS(sal_uInt16(10), getHaloStrength(aAccess, 460, 300));
}

CPPUNIT_TEST_FIXTURE(GlowPrimitive2dTest, testConcaveCornerCollectsMoreHaloThanAStraightEdge)
{
    Bitmap aBitmap = renderGlow(createLShape(basegfx::BColor(0.36, 0.61, 0.84)),
                                Color(0xED, 0x7D, 0x31), 40.0, 600, 700);
    BitmapScopedReadAccess aAccess(aBitmap);
    CPPUNIT_ASSERT(aAccess);

    // Both points sit 15 pixels out from the shape, one on the bisector of the concave corner
    // and one off the middle of the long left edge. The corner has the shape on two sides of
    // it, so the blur gathers more there.
    const sal_uInt16 nAtCorner = getHaloStrength(aAccess, 315, 365);
    const sal_uInt16 nAtStraightEdge = getHaloStrength(aAccess, 185, 350);
    CPPUNIT_ASSERT_GREATER(sal_uInt16(50), nAtStraightEdge);
    CPPUNIT_ASSERT_GREATER(nAtStraightEdge, nAtCorner);
}

CPPUNIT_TEST_FIXTURE(GlowPrimitive2dTest, testTransparentShapeGivesWeakerHalo)
{
    primitive2d::Primitive2DContainer aHalfTransparentShape{
        rtl::Reference<primitive2d::UnifiedTransparencePrimitive2D>(
            new primitive2d::UnifiedTransparencePrimitive2D(createGlowingRectangle(), 0.5))
    };

    Bitmap aHalfTransparent
        = renderGlow(aHalfTransparentShape, Color(0xED, 0x7D, 0x31), 50.0, 600, 600);
    BitmapScopedReadAccess aHalfAccess(aHalfTransparent);
    CPPUNIT_ASSERT(aHalfAccess);

    Bitmap aOpaque = renderGlow(createGlowingRectangle(), Color(0xED, 0x7D, 0x31), 50.0, 600, 600);
    BitmapScopedReadAccess aOpaqueAccess(aOpaque);
    CPPUNIT_ASSERT(aOpaqueAccess);

    // A shape that only half covers its own outline only half lights the halo.
    CPPUNIT_ASSERT_GREATER(getHaloStrength(aHalfAccess, 410, 300),
                           getHaloStrength(aOpaqueAccess, 410, 300));
    CPPUNIT_ASSERT_GREATER(sal_uInt16(20), getHaloStrength(aHalfAccess, 410, 300));
}

CPPUNIT_TEST_FIXTURE(GlowPrimitive2dTest, testTransparentGlowColorGivesWeakerHalo)
{
    Bitmap aHalfTransparent = renderGlow(createGlowingRectangle(),
                                         Color(ColorAlpha, 128, 0xED, 0x7D, 0x31), 50.0, 600, 600);
    BitmapScopedReadAccess aHalfAccess(aHalfTransparent);
    CPPUNIT_ASSERT(aHalfAccess);

    Bitmap aOpaque = renderGlow(createGlowingRectangle(), Color(0xED, 0x7D, 0x31), 50.0, 600, 600);
    BitmapScopedReadAccess aOpaqueAccess(aOpaque);
    CPPUNIT_ASSERT(aOpaqueAccess);

    CPPUNIT_ASSERT_GREATER(getHaloStrength(aHalfAccess, 410, 300),
                           getHaloStrength(aOpaqueAccess, 410, 300));
}

CPPUNIT_TEST_FIXTURE(GlowPrimitive2dTest, testGlowTiming)
{
    // Wall clock cost of a whole glow render, at a few sizes. Turn bTimeGlowRendering on to take a
    // reading, and only on an idle machine: the same figure reads three times higher with a build
    // running alongside it.
    static bool bTimeGlowRendering(false); // loplugin:constvars:ignore

    if (!bTimeGlowRendering)
        return;

    timeOneGlow(1200.0, 100.0, 1600);
    timeOneGlow(300.0, 20.0, 900);
    timeOneGlow(200.0, 6.0, 700);
}

CPPUNIT_TEST_FIXTURE(GlowPrimitive2dTest, testSmallRadiusStillFades)
{
    // A halo only a few pixels wide still has to be a fade. The box passes that soften it round
    // their width to whole pixels, and at this radius that rounding once left nothing to soften
    // with, so the halo held full strength and stopped at half the radius it was asked for.
    Bitmap aBitmap = renderGlow(createGlowingRectangle(), Color(0xED, 0x7D, 0x31), 5.0, 600, 600);
    BitmapScopedReadAccess aAccess(aBitmap);
    CPPUNIT_ASSERT(aAccess);

    const sal_uInt16 nAtOnePixel = getHaloStrength(aAccess, 401, 300);
    const sal_uInt16 nAtTwoPixels = getHaloStrength(aAccess, 402, 300);
    const sal_uInt16 nAtThreePixels = getHaloStrength(aAccess, 403, 300);

    // Strong against the shape, and already coming down one pixel out rather than holding.
    CPPUNIT_ASSERT_GREATER(sal_uInt16(100), nAtOnePixel);
    CPPUNIT_ASSERT_LESS(nAtOnePixel, nAtTwoPixels);
    CPPUNIT_ASSERT_LESS(nAtTwoPixels, nAtThreePixels);

    // Still carrying three pixels out, past where an unsoftened halo of this radius would end.
    CPPUNIT_ASSERT_GREATER(sal_uInt16(2), nAtThreePixels);
}

CPPUNIT_TEST_FIXTURE(GlowPrimitive2dTest, testViewportLeavesTheVisibleHaloAlone)
{
    // The viewport decides whether the glow is worth drawing at all, and nothing else. Once it is
    // drawn, one bitmap covers the whole glow range, so a viewport that only shows part of the halo
    // leaves the part it does show exactly as it was.
    Bitmap aUnclipped
        = renderGlow(createGlowingRectangle(), Color(0xED, 0x7D, 0x31), 50.0, 600, 600);
    BitmapScopedReadAccess aUnclippedAccess(aUnclipped);
    CPPUNIT_ASSERT(aUnclippedAccess);

    // A viewport over the right of the shape, which the halo on the left falls outside of.
    Bitmap aClipped = renderGlow(createGlowingRectangle(), Color(0xED, 0x7D, 0x31), 50.0, 600, 600,
                                 basegfx::B2DRange(390, 200, 600, 400));
    BitmapScopedReadAccess aClippedAccess(aClipped);
    CPPUNIT_ASSERT(aClippedAccess);

    CPPUNIT_ASSERT_EQUAL(getHaloStrength(aUnclippedAccess, 410, 300),
                         getHaloStrength(aClippedAccess, 410, 300));
    CPPUNIT_ASSERT_EQUAL(getHaloStrength(aUnclippedAccess, 190, 300),
                         getHaloStrength(aClippedAccess, 190, 300));
}

CPPUNIT_TEST_FIXTURE(GlowPrimitive2dTest, testNoHaloOutsideTheViewport)
{
    // A viewport nowhere near the shape, so none of the glow can reach it.
    Bitmap aBitmap = renderGlow(createGlowingRectangle(), Color(0xED, 0x7D, 0x31), 50.0, 600, 600,
                                basegfx::B2DRange(2000, 2000, 2400, 2400));
    BitmapScopedReadAccess aAccess(aBitmap);
    CPPUNIT_ASSERT(aAccess);

    CPPUNIT_ASSERT_LESS(sal_uInt16(5), getHaloStrength(aAccess, 410, 300));

    // The shape is drawn on its own and keeps its fill whatever the viewport says.
    CPPUNIT_ASSERT_LESS(sal_uInt16(10),
                        aAccess->GetColor(Point(300, 300)).GetColorError(Color(92, 156, 215)));
}

CPPUNIT_TEST_FIXTURE(GlowPrimitive2dTest, testNoHaloWithZeroRadius)
{
    Bitmap aBitmap = renderGlow(createGlowingRectangle(), Color(0xED, 0x7D, 0x31), 0.0, 600, 600);
    BitmapScopedReadAccess aAccess(aBitmap);
    CPPUNIT_ASSERT(aAccess);

    CPPUNIT_ASSERT_LESS(sal_uInt16(5), getHaloStrength(aAccess, 190, 300));
    CPPUNIT_ASSERT_LESS(sal_uInt16(5), getHaloStrength(aAccess, 410, 300));
}

} // anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
