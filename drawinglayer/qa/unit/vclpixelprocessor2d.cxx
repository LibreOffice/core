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
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>

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
        ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
        device->SetOutputSizePixel(Size(100, 200));
        device->SetBackground(Wallpaper(COL_RED));
        device->Erase();

        drawinglayer::geometry::ViewInformation2D view;
        std::unique_ptr<processor2d::BaseProcessor2D> processor(
            processor2d::createProcessor2DFromOutputDevice(*device, view));
        CPPUNIT_ASSERT(processor);

        basegfx::B2DRange definitionRange(0, 0, 100, 200);
        basegfx::B2DRange outputRange(0, 100, 100, 200); // Paint only lower half of the gradient.
        attribute::FillGradientAttribute attributes(attribute::GradientStyle::Linear, 0, 0, 0, 0,
                                                    COL_WHITE.getBColor(), COL_BLACK.getBColor());
        rtl::Reference<primitive2d::FillGradientPrimitive2D> gradientPrimitive(
            new primitive2d::FillGradientPrimitive2D(outputRange, definitionRange, attributes));
        primitive2d::Primitive2DContainer primitives;
        primitives.push_back(primitive2d::Primitive2DReference(gradientPrimitive));
        processor->process(primitives);

        exportDevice("test-tdf139000.png", device);
        Bitmap bitmap = device->GetBitmap(Point(), device->GetOutputSizePixel());
        Bitmap::ScopedReadAccess access(bitmap);
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
