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
#include <vcl/canvastools.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>

#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>

using namespace ::com::sun::star;

class CanvasTest : public test::BootstrapFixture
{
    VclPtr<VirtualDevice> mVclDevice;
    uno::Reference<rendering::XCanvas> mCanvas;
    rendering::ViewState mViewState;
    rendering::RenderState mRenderState;
    uno::Sequence<double> mColorBlack;

    // if enabled - check the result images with:
    // "xdg-open ./workdir/CppunitTest/canvas_test.test.core/"
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
    CanvasTest()
        : BootstrapFixture(true, false)
    {
    }

    virtual void setUp() override
    {
        BootstrapFixture::setUp();
        mColorBlack = vcl::unotools::colorToStdColorSpaceSequence(COL_BLACK);
        // Geometry init
        geometry::AffineMatrix2D aUnit(1, 0, 0, 0, 1, 0);
        mViewState.AffineTransform = aUnit;
        mRenderState.AffineTransform = aUnit;
        mRenderState.DeviceColor = mColorBlack;
        mRenderState.CompositeOperation = rendering::CompositeOperation::OVER;
    }

    virtual void tearDown() override
    {
        mVclDevice.clear();
        mCanvas = uno::Reference<rendering::XCanvas>();
        BootstrapFixture::tearDown();
    }

    void setupCanvas(const Size& size, Color backgroundColor = COL_WHITE, bool alpha = false)
    {
        mVclDevice
            = alpha ? VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT)
                    : VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
        mVclDevice->SetOutputSizePixel(size);
        mVclDevice->SetBackground(Wallpaper(backgroundColor));
        mVclDevice->Erase();
        mCanvas = mVclDevice->GetCanvas();
        CPPUNIT_ASSERT(mCanvas.is());
    }

    void testDrawLine()
    {
        setupCanvas(Size(10, 10));
        mCanvas->drawLine(geometry::RealPoint2D(1, 1), geometry::RealPoint2D(9, 1), mViewState,
                          mRenderState);
        exportDevice("test-draw-line.png", mVclDevice);
        Bitmap bitmap = mVclDevice->GetBitmap(Point(), Size(10, 10));
        Bitmap::ScopedReadAccess access(bitmap);
        // Canvas uses AA, which blurs the line, and it cannot be turned off,
        // so do not check the end points.
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_WHITE), access->GetPixel(0, 0));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_BLACK), access->GetPixel(1, 2));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_BLACK), access->GetPixel(1, 8));
    }

    CPPUNIT_TEST_SUITE(CanvasTest);
    CPPUNIT_TEST(testDrawLine);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CanvasTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
