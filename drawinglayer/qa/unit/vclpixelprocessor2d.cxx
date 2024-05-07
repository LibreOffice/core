/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/bootstrapfixture.hxx>

#include <vcl/virdev.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/utils/gradienttools.hxx>

using namespace drawinglayer;

class VclPixelProcessor2DTest : public test::BootstrapFixture
{
    // if enabled - check the result images with:
    // "xdg-open ./workdir/CppunitTest/drawinglayer_processors.test.core/"
    static constexpr const bool mbExportBitmap = false;

    void exportDevice(const OUString& filename, const VclPtr<VirtualDevice>& device)
    {
        if (mbExportBitmap)
        {
            BitmapEx aBitmapEx(device->GetBitmapEx(Point(0, 0), device->GetOutputSizePixel()));
            SvFileStream aStream(filename, StreamMode::WRITE | StreamMode::TRUNC);
            GraphicFilter::GetGraphicFilter().compressAsPNG(aBitmapEx, aStream);
        }
    }

public:
    VclPixelProcessor2DTest()
        : BootstrapFixture(true, false)
    {
    }

    // Test that drawing only a part of a gradient draws the proper part of it.
    void testTdf139000()
    {
        ScopedVclPtr<VirtualDevice> device
            = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
        device->SetOutputSizePixel(Size(100, 200));
        device->SetBackground(Wallpaper(COL_RED));
        device->Erase();

        drawinglayer::geometry::ViewInformation2D view;
        std::unique_ptr<processor2d::BaseProcessor2D> processor(
            processor2d::createProcessor2DFromOutputDevice(*device, view));
        CPPUNIT_ASSERT(processor);

        // I stumbled over this when hunting another problem, but I have to correct
        // this: This test does test something that is not supported. It seems to be
        // based on the *misunderstanding* that in the version of the constructor of
        // FillGradientPrimitive2D (and similar others) with two ranges the 2nd
        // B2DRange parameter 'OutputRange' is a 'clipping' parameter. This is *not*
        // the case --- it is in fact the *contrary*, it is there to *extend* the
        // usual definition/paintRange of a gradient:
        // It was originally needed to correctly display TextFrames (TF) in Writer: If you
        // have a TF in SW filled with a gradient and that TF has sub-frames, it inherits
        // the gradient fill. Since you can freely move those sub-TFs even outside the
        // parent TF there has to be a way to not only paint gradients in their definition
        // range (classical, all DrawObjects do that), but extended from that. This is
        // needed e.g. for linear gradients, but - dependent of e.g. the center settings -
        // also for all other ones, all can have geometry 'outside' the DefinitionRange.
        // This is now also used in various other locations which is proof that this is
        // useful and needed. It is possible to see that basic history/reason for this
        // parameter by following the git history and why and under which circumstances
        // that parameter was originally added. Other hints are: It is *not* named
        // 'ClipRange'. Using a B2DRange to define a ClipRange topology would be bad due
        // to not being transformable, a PolyPolygon would be used in that case. Using as
        // clipping mechanism would offer a 2nd principle to add clipping for primitives
        // besides MaskPrimitive2D - always bad style in a sub-system. A quick look
        // on it's usages gives hints, too.
        // This means that when defining an outputRange that resides completely *inside*
        // the definitionRange *no change* at all is done by definition since this does
        // not *extend* the target area of the gradient paint region at all. If an
        // implementation does clip and limit output to 'outputRange' that should do no
        // harm, but is not the expected/reliable way to paint primitives clipped.
        // That's why all DrawObjects with gradient fill (and other fills do the same)
        // embed the fill that is defined for a range (usually the BoundRange of a
        // PolyPolygon) in a MaskPrimitive2D defined by the outline PolyPolygon of the
        // shape. Nothing speaks against renderers detecting that combination and do
        // something optimized if they want to, especially SDPRs, but this is not
        // required. The standard embedded clipping of the implementations of the
        // MaskPrimitive2D do the right thing.
        // This test intends to paint the lower part of a gradient, so define the
        // gradient for the full target range and embed it to a MaskPrimitive2D
        // defining the lower part of that area to do that.

        basegfx::B2DRange definitionRange(0, 0, 100, 200);
        basegfx::B2DRange outputRange(0, 100, 100, 200); // Paint only lower half of the gradient.

        const primitive2d::Primitive2DContainer primitives{
            rtl::Reference<primitive2d::MaskPrimitive2D>(new primitive2d::MaskPrimitive2D(
                basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(outputRange)),
                primitive2d::Primitive2DContainer{
                    rtl::Reference<primitive2d::FillGradientPrimitive2D>(
                        new primitive2d::FillGradientPrimitive2D(
                            definitionRange, attribute::FillGradientAttribute(
                                                 css::awt::GradientStyle_LINEAR, 0, 0, 0, 0,
                                                 basegfx::BColorStops(COL_WHITE.getBColor(),
                                                                      COL_BLACK.getBColor())))) }))
        };
        processor->process(primitives);

        exportDevice(u"test-tdf139000.png"_ustr, device);
        Bitmap bitmap = device->GetBitmap(Point(), device->GetOutputSizePixel());
        BitmapScopedReadAccess access(bitmap);
        // The upper half should keep its red background color.
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_RED), access->GetColor(Point(0, 99)));
        // First line of the gradient should not be the start color, but something halfway.
        CPPUNIT_ASSERT_LESS(static_cast<sal_uInt16>(16),
                            access->GetColor(Point(0, 100)).GetColorError(COL_GRAY));
        // Last line of the gradient should be the end color, or close.
        CPPUNIT_ASSERT_LESS(static_cast<sal_uInt16>(16),
                            access->GetColor(Point(0, 199)).GetColorError(COL_BLACK));
    }

    CPPUNIT_TEST_SUITE(VclPixelProcessor2DTest);
    CPPUNIT_TEST(testTdf139000);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VclPixelProcessor2DTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
