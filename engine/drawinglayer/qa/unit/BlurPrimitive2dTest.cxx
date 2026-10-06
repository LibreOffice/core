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

#include <vcl/virdev.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/softedgeprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <cmath>

using namespace drawinglayer;

namespace
{
// The radius of the blur in pixels, and the right edge of the black square it is applied to.
constexpr sal_Int32 nBlurRadius = 30;
constexpr sal_Int32 nSquareRightEdge = 400;

// Gray levels the blur may differ from a true Gaussian by. It comes within a few of them.
constexpr double fTolerance = 6.0;

class BlurPrimitive2dTest : public test::BootstrapFixture
{
public:
    BlurPrimitive2dTest()
        : BootstrapFixture(true, false)
    {
    }

    static primitive2d::Primitive2DContainer createBlackSquare()
    {
        const basegfx::B2DRange aRange(200, 200, nSquareRightEdge, nSquareRightEdge);
        return primitive2d::Primitive2DContainer{
            rtl::Reference<primitive2d::PolyPolygonColorPrimitive2D>(
                new primitive2d::PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(aRange)),
                    basegfx::BColor(0, 0, 0)))
        };
    }

    static Bitmap render(const primitive2d::Primitive2DReference& rPrimitive)
    {
        ScopedVclPtr<VirtualDevice> pDevice
            = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
        pDevice->SetOutputSizePixel(Size(600, 600));
        pDevice->SetBackground(Wallpaper(COL_WHITE));
        pDevice->Erase();

        // A blur needs a pixel target, which a non-identity object to view transformation stands
        // for. A tiny translation gives one without moving the content.
        basegfx::B2DHomMatrix aViewTransform;
        aViewTransform.translate(0.001, 0.001);
        geometry::ViewInformation2D aViewInformation;
        aViewInformation.setViewTransformation(aViewTransform);

        std::unique_ptr<processor2d::BaseProcessor2D> pProcessor(
            processor2d::createProcessor2DFromOutputDevice(*pDevice, aViewInformation));
        pProcessor->process(primitive2d::Primitive2DContainer{ rPrimitive });

        return pDevice->GetBitmap(Point(), pDevice->GetOutputSizePixel());
    }

    // How much of the black covers the pixel on the white page, as a gray level.
    static double getCoverage(BitmapScopedReadAccess& rAccess, sal_Int32 nX)
    {
        return 255.0 - rAccess->GetColor(Point(nX, 300)).GetRed();
    }

    // A straight edge at fEdge blurred by a Gaussian of a third of the radius, at the center of
    // pixel nX, as a gray level.
    static double getGaussianEdge(sal_Int32 nX, double fEdge)
    {
        const double fSigma = nBlurRadius / 3.0;
        return 255.0 * 0.5 * std::erfc((nX + 0.5 - fEdge) / (fSigma * std::sqrt(2.0)));
    }
};

CPPUNIT_TEST_FIXTURE(BlurPrimitive2dTest, testShadowBlurIsAGaussianOfAThirdOfTheRadius)
{
    // The shadow lies right under the square, so its blurred edge is the square's own edge.
    Bitmap aBitmap = render(new primitive2d::ShadowPrimitive2D(
        basegfx::B2DHomMatrix(), basegfx::BColor(0, 0, 0), nBlurRadius, createBlackSquare()));
    BitmapScopedReadAccess aAccess(aBitmap);
    CPPUNIT_ASSERT(aAccess);

    for (sal_Int32 nX = nSquareRightEdge - 20; nX <= nSquareRightEdge + 20; nX += 10)
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(getGaussianEdge(nX, nSquareRightEdge),
                                     getCoverage(aAccess, nX), fTolerance);
    }

    // The blur has faded out at its radius.
    CPPUNIT_ASSERT_LESS(2.0, getCoverage(aAccess, nSquareRightEdge + nBlurRadius));
}

CPPUNIT_TEST_FIXTURE(BlurPrimitive2dTest, testScaledShadowRangeHoldsTheWholeBlur)
{
    // The shadow at half size spans 100 to 200, and the blur adds the full radius around it.
    basegfx::B2DHomMatrix aShadowTransform;
    aShadowTransform.scale(0.5, 0.5);
    const rtl::Reference<primitive2d::ShadowPrimitive2D> pShadow(new primitive2d::ShadowPrimitive2D(
        aShadowTransform, basegfx::BColor(0, 0, 0), nBlurRadius, createBlackSquare()));

    const basegfx::B2DRange aRange(pShadow->getB2DRange(geometry::ViewInformation2D()));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0 - nBlurRadius, aRange.getMinX(), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0 - nBlurRadius, aRange.getMinY(), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0 + nBlurRadius, aRange.getMaxX(), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0 + nBlurRadius, aRange.getMaxY(), 1e-9);
}

CPPUNIT_TEST_FIXTURE(BlurPrimitive2dTest, testSoftEdgeFadesOneRadiusInsideTheShape)
{
    // The shape is pulled in by the radius and blurred back out, so the fade is centered a radius
    // inside the edge.
    Bitmap aBitmap = render(new primitive2d::SoftEdgePrimitive2D(nBlurRadius, createBlackSquare()));
    BitmapScopedReadAccess aAccess(aBitmap);
    CPPUNIT_ASSERT(aAccess);

    // The fade has the same Gaussian as the shadow.
    const sal_Int32 nFadeCenter = nSquareRightEdge - nBlurRadius;
    for (sal_Int32 nX = nFadeCenter - 20; nX <= nFadeCenter + 20; nX += 10)
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(getGaussianEdge(nX, nFadeCenter), getCoverage(aAccess, nX),
                                     fTolerance);
    }

    // Nothing of the shape shows past its own edge.
    CPPUNIT_ASSERT_LESS(2.0, getCoverage(aAccess, nSquareRightEdge + 1));
}

} // anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
