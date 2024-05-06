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

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>

using namespace ::com::sun::star;

class CanvasTest : public test::BootstrapFixture
{
    VclPtr<VirtualDevice> mVclDevice;
    uno::Reference<rendering::XCanvas> mCanvas;
    uno::Reference<rendering::XGraphicDevice> mDevice;
    rendering::ViewState mViewState;
    rendering::RenderState mRenderState;
    uno::Sequence<double> mColorBlack;
    uno::Sequence<double> mColorBlue;

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
        mColorBlue = vcl::unotools::colorToStdColorSpaceSequence(COL_BLUE);
        // Geometry init
        geometry::AffineMatrix2D aUnit(1, 0, 0, 0, 1, 0);
        mViewState.AffineTransform = aUnit;
        mRenderState.AffineTransform = aUnit;
        mRenderState.DeviceColor = mColorBlack;
        mRenderState.CompositeOperation = rendering::CompositeOperation::OVER;
    }

    virtual void tearDown() override
    {
        mVclDevice.disposeAndClear();
        mCanvas = uno::Reference<rendering::XCanvas>();
        mDevice = uno::Reference<rendering::XGraphicDevice>();
        BootstrapFixture::tearDown();
    }

    void setupCanvas(const Size& size, Color backgroundColor = COL_WHITE, bool alpha = false)
    {
        mVclDevice = alpha ? VclPtr<VirtualDevice>::Create(DeviceFormat::WITH_ALPHA)
                           : VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
        mVclDevice->SetOutputSizePixel(size);
        mVclDevice->SetBackground(Wallpaper(backgroundColor));
        mVclDevice->Erase();
        mCanvas = mVclDevice->GetCanvas();
        CPPUNIT_ASSERT(mCanvas.is());
        mDevice
            = uno::Reference<rendering::XGraphicDevice>(mCanvas->getDevice(), uno::UNO_SET_THROW);
        CPPUNIT_ASSERT(mDevice.is());
    }

    void testDrawLine()
    {
        setupCanvas(Size(10, 10));
        mCanvas->drawLine(geometry::RealPoint2D(1, 1), geometry::RealPoint2D(9, 1), mViewState,
                          mRenderState);
        exportDevice(u"test-draw-line.png"_ustr, mVclDevice);
        Bitmap bitmap = mVclDevice->GetBitmap(Point(), mVclDevice->GetOutputSizePixel());
        BitmapScopedReadAccess access(bitmap);
        // Canvas uses AA, which blurs the line, and it cannot be turned off,
        // so do not check the end points.
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_WHITE), access->GetPixel(0, 0));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_BLACK), access->GetPixel(1, 2));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_BLACK), access->GetPixel(1, 8));
    }

    // Draw a dashed line scaled, make sure the dashing is scaled properly.
    void testTdf134053()
    {
        setupCanvas(Size(1000, 100));
        // Scale everything up by 10 (2 in render state, 5 in view state).
        mRenderState.AffineTransform = geometry::AffineMatrix2D(2, 0, 0, 0, 2, 0);
        mViewState.AffineTransform = geometry::AffineMatrix2D(5, 0, 0, 0, 5, 0);

        uno::Sequence<uno::Sequence<geometry::RealPoint2D>> polygonPoints{ { { 10, 5 },
                                                                             { 88, 5 } } };
        uno::Reference<rendering::XLinePolyPolygon2D> polygon
            = mDevice->createCompatibleLinePolyPolygon(polygonPoints);
        polygon->setClosed(0, false);

        mRenderState.DeviceColor = mColorBlue;
        rendering::StrokeAttributes strokeAttributes;
        strokeAttributes.StrokeWidth = 2.0;
        strokeAttributes.MiterLimit = 2.0; // ?
        strokeAttributes.StartCapType = rendering::PathCapType::ROUND;
        strokeAttributes.EndCapType = rendering::PathCapType::ROUND;
        strokeAttributes.JoinType = rendering::PathJoinType::MITER;
        strokeAttributes.DashArray = { 10, 5, 0.1, 5 };

        mCanvas->strokePolyPolygon(polygon, mViewState, mRenderState, strokeAttributes);

        exportDevice(u"test-tdf134053.png"_ustr, mVclDevice);
        Bitmap bitmap = mVclDevice->GetBitmap(Point(), mVclDevice->GetOutputSizePixel());
        BitmapScopedReadAccess access(bitmap);
        struct Check
        {
            tools::Long start;
            tools::Long end;
            Color color;
        };
        // There should be a long dash at X 100-200, a dot at 250, long one at 300-400, a dot at 450, etc.
        // until a dot at 850. Add -5/+5 to account for round caps.
        const Check checks[] = { { 0, 85, COL_WHITE }, // empty start
                                 // dash, space, dot, space
                                 { 95, 205, COL_BLUE },
                                 { 215, 235, COL_WHITE },
                                 { 245, 255, COL_BLUE },
                                 { 265, 285, COL_WHITE },
                                 { 295, 405, COL_BLUE },
                                 { 415, 435, COL_WHITE },
                                 { 445, 455, COL_BLUE },
                                 { 465, 485, COL_WHITE },
                                 { 495, 605, COL_BLUE },
                                 { 615, 635, COL_WHITE },
                                 { 645, 655, COL_BLUE },
                                 { 665, 685, COL_WHITE },
                                 { 695, 805, COL_BLUE },
                                 { 815, 835, COL_WHITE },
                                 { 845, 855, COL_BLUE },
                                 { 865, 999, COL_WHITE } }; // empty end
        for (const Check& check : checks)
        {
            for (tools::Long x = check.start; x <= check.end; ++x)
            {
                if (access->GetColor(50, x) != check.color)
                {
                    std::ostringstream str;
                    str << "X: " << x;
                    CPPUNIT_ASSERT_EQUAL_MESSAGE(str.str(), BitmapColor(check.color),
                                                 access->GetColor(50, x));
                }
            }
        }
    }

    CPPUNIT_TEST_SUITE(CanvasTest);
    CPPUNIT_TEST(testDrawLine);
    CPPUNIT_TEST(testTdf134053);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CanvasTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
