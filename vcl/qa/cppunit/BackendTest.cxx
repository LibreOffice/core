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

#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <bitmap/BitmapWriteAccess.hxx>

#include <svdata.hxx>
#include <salinst.hxx>
#include <salgdi.hxx>

#include <test/outputdevice.hxx>

// Run tests from visualbackendtest ('bin/run visualbackendtest').
class BackendTest : public test::BootstrapFixture
{
    // if enabled - check the result images with:
    // "xdg-open ./workdir/CppunitTest/vcl_backend_test.test.core/"
    static constexpr const bool mbExportBitmap = false;

    void exportImage(OUString const& rsFilename, BitmapEx const& rBitmapEx)
    {
        if (mbExportBitmap)
        {
            BitmapEx aBitmapEx(rBitmapEx);
            aBitmapEx.Scale(Size(128, 128), BmpScaleFlag::Fast);
            SvFileStream aStream(rsFilename, StreamMode::WRITE | StreamMode::TRUNC);
            GraphicFilter::GetGraphicFilter().compressAsPNG(aBitmapEx, aStream);
        }
    }

    void exportImage(OUString const& rsFilename, Bitmap const& rBitmap)
    {
        if (mbExportBitmap)
        {
            Bitmap aBitmap(rBitmap);
            aBitmap.Scale(Size(128, 128), BmpScaleFlag::Fast);
            SvFileStream aStream(rsFilename, StreamMode::WRITE | StreamMode::TRUNC);
            GraphicFilter::GetGraphicFilter().compressAsPNG(BitmapEx(aBitmap), aStream);
        }
    }

    void exportDevice(const OUString& filename, const VclPtr<VirtualDevice>& device)
    {
        if (mbExportBitmap)
        {
            BitmapEx aBitmapEx(device->GetBitmapEx(Point(0, 0), device->GetOutputSizePixel()));
            SvFileStream aStream(filename, StreamMode::WRITE | StreamMode::TRUNC);
            GraphicFilter::GetGraphicFilter().compressAsPNG(aBitmapEx, aStream);
        }
    }

    bool is32bppSupported() { return ImplGetSVData()->mpDefInst->supportsBitmap32(); }

public:
    BackendTest()
        : BootstrapFixture(true, false)
    {
    }

    // We need to enable tests ONE BY ONE as they fail because of backend bugs
    // it is still important to have the test defined so we know the issues
    // exist and we need to fix them. Consistent behaviour of our backends
    // is of highest priority.

    static bool assertBackendNameNotEmpty(const OUString& name)
    {
        // This ensures that all backends return a valid name.
        assert(!name.isEmpty());
        (void)name;
        return true;
    }

// Check whether tests should fail depending on which backend is used
// (not all work). If you want to disable just a specific test
// for a specific backend, use something like
// 'if(SHOULD_ASSERT && aOutDevTest.getRenderBackendName() != "skia")'.
// The macro uses opt-out rather than opt-in so that this doesn't "pass"
// silently in case a new backend is added.
#define SHOULD_ASSERT                                                                              \
    (assertBackendNameNotEmpty(aOutDevTest.getRenderBackendName())                                 \
     && aOutDevTest.getRenderBackendName() != "qt5"                                                \
     && aOutDevTest.getRenderBackendName() != "qt5svp"                                             \
     && aOutDevTest.getRenderBackendName() != "gtk3svp"                                            \
     && aOutDevTest.getRenderBackendName() != "aqua"                                               \
     && aOutDevTest.getRenderBackendName() != "gen"                                                \
     && aOutDevTest.getRenderBackendName() != "genpsp"                                             \
     && aOutDevTest.getRenderBackendName() != "win")

#ifdef MACOSX
    static OUString getRenderBackendName(OutputDevice* device)
    {
        assert(device);
        return device->GetGraphics()->getRenderBackendName();
    }
#endif

    void testDrawRectWithRectangle()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
        exportImage("01-01_rectangle_test-rectangle.png", aBitmap);

        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPixel()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPixel aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
        exportImage("01-02_rectangle_test-pixel.png", aBitmap);

        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithLine()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
        exportImage("01-03_rectangle_test-line.png", aBitmap);

        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPolygon()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
        exportImage("01-04_rectangle_test-polygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPolyLine()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
        exportImage("01-05_rectangle_test-polyline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPolyLineB2D()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
        exportImage("01-06_rectangle_test-polyline_b2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPolyPolygon()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
        exportImage("01-07_rectangle_test-polypolygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPolyPolygonB2D()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
        exportImage("01-08_rectangle_test-polypolygon_b2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithRectangle()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-01_rectangle_AA_test-rectangle.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPixel()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPixel aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-02_rectangle_AA_test-pixel.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithLine()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-03_rectangle_AA_test-line.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPolygon()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-04_rectangle_AA_test-polygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPolyLine()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-05_rectangle_AA_test-polyline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPolyLineB2D()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-06_rectangle_AA_test-polyline_b2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPolyPolygon()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-07_rectangle_AA_test-polypolygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPolyPolygonB2D()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-08_rectangle_AA_test-polypolygon_b2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawFilledRectWithRectangle()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
        exportImage("03-01_filled_rectangle_test-rectangle_noline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
        aBitmap = aOutDevTest.setupFilledRectangle(true);
        eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
        exportImage("03-01_filled_rectangle_test-rectangle_line.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawFilledRectWithPolygon()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
        exportImage("03-02_filled_rectangle_test-polygon_noline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
        aBitmap = aOutDevTest.setupFilledRectangle(true);
        eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
        exportImage("03-02_filled_rectangle_test-polygon_line.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawFilledRectWithPolyPolygon()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
        exportImage("03-03_filled_rectangle_test-polypolygon_noline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
        aBitmap = aOutDevTest.setupFilledRectangle(true);
        eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
        exportImage("03-03_filled_rectangle_test-polypolygon_line.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawFilledRectWithPolyPolygon2D()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
        exportImage("03-04_filled_rectangle_test-polypolygon_b2d_noline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
        aBitmap = aOutDevTest.setupFilledRectangle(true);
        eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
        exportImage("03-04_filled_rectangle_test-polypolygon_b2d_line.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDiamondWithPolygon()
    {
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDiamond();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
        exportImage("04-01_diamond_test-polygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDiamondWithLine()
    {
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDiamond();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
        exportImage("04-02_diamond_test-line.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDiamondWithPolyline()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDiamond();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
        exportImage("04-03_diamond_test-polyline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDiamondWithPolylineB2D()
    {
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDiamond();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
        exportImage("04-04_diamond_test-polyline_b2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawInvertWithRectangle()
    {
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupInvert_NONE();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkInvertRectangle(aBitmap);
        exportImage("05-01_invert_test-rectangle.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawInvertN50WithRectangle()
    {
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupInvert_N50();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkInvertN50Rectangle(aBitmap);
        exportImage("05-02_invert_N50_test-rectangle.png", aBitmap);
        if (SHOULD_ASSERT && aOutDevTest.getRenderBackendName() != "svp")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawInvertTrackFrameWithRectangle()
    {
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupInvert_TrackFrame();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkInvertTrackFrameRectangle(aBitmap);
        exportImage("05-03_invert_TrackFrame_test-rectangle.png", aBitmap);
        if (SHOULD_ASSERT && aOutDevTest.getRenderBackendName() != "svp")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBezierWithPolylineB2D()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupBezier();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
        exportImage("06-01_bezier_test-polyline_b2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBezierAAWithPolylineB2D()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupAABezier();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
        exportImage("07-01_bezier_AA_test-polyline_b2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBitmap24bpp()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N24_BPP);
        exportImage("08-01_bitmap_test_24bpp.png", aBitmap);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawTransformedBitmap24bpp()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N24_BPP);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
        exportImage("08-02_transformed_bitmap_test_24bpp.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testComplexDrawTransformedBitmap24bpp()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupComplexDrawTransformedBitmap(vcl::PixelFormat::N24_BPP);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkComplexTransformedBitmap(aBitmap);
        exportImage("08-03_transformed_bitmap_test_24bpp.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBitmapExWithAlpha24bpp()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha(vcl::PixelFormat::N24_BPP);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkBitmapExWithAlpha(aBitmap);
        exportImage("08-04_bitmapex_with_alpha_test_24bpp.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawMask24bpp()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawMask(vcl::PixelFormat::N24_BPP);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkMask(aBitmap);
        exportImage("08-05_mask_test_24bpp.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBlend24bpp()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend(vcl::PixelFormat::N24_BPP);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkBlend(aBitmapEx);
        exportImage("08-06_blend_test_24bpp.png", aBitmapEx);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBitmap32bpp()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N32_BPP);
        exportImage("09-01_bitmap_test_32bpp.png", aBitmap);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
        if (SHOULD_ASSERT && is32bppSupported())
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawTransformedBitmap32bpp()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N32_BPP);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
        exportImage("09-02_transformed_bitmap_test_32bpp.png", aBitmap);
        if (SHOULD_ASSERT && is32bppSupported())
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBitmapExWithAlpha32bpp()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha(vcl::PixelFormat::N32_BPP);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkBitmapExWithAlpha(aBitmap);
        exportImage("09-03_bitmapex_with_alpha_test_32bpp.png", aBitmap);
        if (SHOULD_ASSERT && is32bppSupported())
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawMask32bpp()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawMask(vcl::PixelFormat::N32_BPP);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkMask(aBitmap);
        exportImage("09-04_mask_test_32bpp.png", aBitmap);
        if (SHOULD_ASSERT && is32bppSupported())
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBlend32bpp()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend(vcl::PixelFormat::N32_BPP);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkBlend(aBitmapEx);
        exportImage("09-05_blend_test_32bpp.png", aBitmapEx);
        if (SHOULD_ASSERT && is32bppSupported())
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBitmap8bppGreyScale()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N8_BPP, true);
        exportImage("010-01_bitmap_test_8bpp_greyscale.png", aBitmap);
        auto eResult
            = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap8bppGreyScale(aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawTransformedBitmap8bppGreyScale()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N8_BPP, true);
        auto eResult
            = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap8bppGreyScale(aBitmap);
        exportImage("010-02_transformed_bitmap_test_8bpp_greyscale.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawXor()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupXOR();
        auto eResult = vcl::test::OutputDeviceTestAnotherOutDev::checkXOR(aBitmap);
        exportImage("08-06_xor_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawTransformedBitmapExAlpha()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        ScopedVclPtrInstance<VirtualDevice> device;
#ifdef MACOSX
        // TODO: This unit test is not executed for macOS unless bitmap scaling is implemented
        if (getRenderBackendName(device) == "aqua")
            return;
#endif
        device->SetOutputSizePixel(Size(16, 16));
        device->SetBackground(Wallpaper(COL_WHITE));
        device->Erase();
        Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);
        {
            // Fill the top left quarter with black.
            BitmapScopedWriteAccess pWriteAccess(aBitmap);
            pWriteAccess->Erase(COL_WHITE);
            for (int i = 0; i < 8; ++i)
                for (int j = 0; j < 8; ++j)
                    pWriteAccess->SetPixel(j, i, COL_BLACK);
        }
        BitmapEx aBitmapEx(aBitmap);
        basegfx::B2DHomMatrix aMatrix;
        // Draw with no transformation, only alpha change.
        aMatrix.scale(16, 16);
        device->DrawTransformedBitmapEx(aMatrix, aBitmapEx, 0.5);
        BitmapEx result = device->GetBitmapEx(Point(0, 0), Size(16, 16));
        CPPUNIT_ASSERT_EQUAL(Color(0x80, 0x80, 0x80), result.GetPixelColor(0, 0));
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, result.GetPixelColor(15, 15));
        // Draw rotated and move to the bottom-left corner.
        device->Erase();
        aMatrix.identity();
        aMatrix.scale(16, 16);
        aMatrix.rotate(M_PI / 2);
        aMatrix.translate(8, 8);
        device->DrawTransformedBitmapEx(aMatrix, aBitmapEx, 0.5);
        result = device->GetBitmap(Point(0, 0), Size(16, 16));
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, result.GetPixelColor(0, 0));
        CPPUNIT_ASSERT_EQUAL(Color(0x80, 0x80, 0x80), result.GetPixelColor(0, 15));
    }

    void testClipRectangle()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestClip aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupClipRectangle();
        auto eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
        exportImage("09-01_clip_rectangle_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testClipPolygon()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestClip aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupClipPolygon();
        auto eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
        exportImage("09-02_clip_polygon_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testClipPolyPolygon()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestClip aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupClipPolyPolygon();
        auto eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
        exportImage("09-03_clip_polypolygon_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testClipB2DPolyPolygon()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestClip aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupClipB2DPolyPolygon();
        auto eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
        exportImage("09-04_clip_b2dpolypolygon_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawOutDev()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawOutDev();
        auto eResult = vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDev(aBitmap);
        exportImage("10-01_draw_out_dev_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawOutDevScaledClipped()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawOutDevScaledClipped();
        auto eResult
            = vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDevScaledClipped(aBitmap);
        exportImage("10-02_draw_out_dev_scaled_clipped_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawOutDevSelf()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawOutDevSelf();
        auto eResult = vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDevSelf(aBitmap);
        exportImage("10-03_draw_out_dev_self_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDashedLine()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDashedLine();
        auto eResult = vcl::test::OutputDeviceTestLine::checkDashedLine(aBitmap);
        exportImage("11-01_dashed_line_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testErase()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        {
            // Create normal virtual device (no alpha).
            ScopedVclPtr<VirtualDevice> device
                = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
            device->SetOutputSizePixel(Size(10, 10));
            // Erase with white, check it's white.
            device->SetBackground(Wallpaper(COL_WHITE));
            device->Erase();
            exportDevice("12-01_erase.png", device);
            CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(0, 0)));
            CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(9, 9)));
            CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(5, 5)));
            // Erase with black, check it's black.
            device->SetBackground(Wallpaper(COL_BLACK));
            device->Erase();
            exportDevice("12-02_erase.png", device);
            CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(0, 0)));
            CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(9, 9)));
            CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(5, 5)));
            // Erase with cyan, check it's cyan.
            device->SetBackground(Wallpaper(COL_CYAN));
            device->Erase();
            exportDevice("12-03_erase.png", device);
            CPPUNIT_ASSERT_EQUAL(COL_CYAN, device->GetPixel(Point(0, 0)));
            CPPUNIT_ASSERT_EQUAL(COL_CYAN, device->GetPixel(Point(9, 9)));
            CPPUNIT_ASSERT_EQUAL(COL_CYAN, device->GetPixel(Point(5, 5)));
        }
        {
            // Create virtual device with alpha.
            ScopedVclPtr<VirtualDevice> device
                = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
            device->SetOutputSizePixel(Size(10, 10));
            // Erase with white, check it's white.
            device->SetBackground(Wallpaper(COL_WHITE));
            device->Erase();
            exportDevice("12-04_erase.png", device);
            CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(0, 0)));
            CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(9, 9)));
            CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(5, 5)));
            // Erase with black, check it's black.
            device->SetBackground(Wallpaper(COL_BLACK));
            device->Erase();
            exportDevice("12-05_erase.png", device);
            CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(0, 0)));
            CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(9, 9)));
            CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(5, 5)));
            // Erase with cyan, check it's cyan.
            device->SetBackground(Wallpaper(COL_CYAN));
            device->Erase();
            exportDevice("12-06_erase.png", device);
            CPPUNIT_ASSERT_EQUAL(COL_CYAN, device->GetPixel(Point(0, 0)));
            CPPUNIT_ASSERT_EQUAL(COL_CYAN, device->GetPixel(Point(9, 9)));
            CPPUNIT_ASSERT_EQUAL(COL_CYAN, device->GetPixel(Point(5, 5)));
            // Erase with transparent, check it's transparent.
            device->SetBackground(Wallpaper(COL_TRANSPARENT));
            device->Erase();
            exportDevice("12-07_erase.png", device);
            CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), device->GetPixel(Point(0, 0)).GetAlpha());
            CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), device->GetPixel(Point(9, 9)).GetAlpha());
            CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), device->GetPixel(Point(5, 5)).GetAlpha());
        }
    }

    void testLinearGradient()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestGradient aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLinearGradient();
        auto eResult = vcl::test::OutputDeviceTestGradient::checkLinearGradient(aBitmap);
        exportImage("13-01_linear_gradient_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testLinearGradientAngled()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestGradient aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLinearGradientAngled();
        auto eResult = vcl::test::OutputDeviceTestGradient::checkLinearGradientAngled(aBitmap);
        exportImage("13-02_linear_gradient_angled_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testLinearGradientBorder()
    {
        vcl::test::OutputDeviceTestGradient aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLinearGradientBorder();
        auto eResult = vcl::test::OutputDeviceTestGradient::checkLinearGradientBorder(aBitmap);
        exportImage("13-03_linear_gradient_border_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testLinearGradientIntensity()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestGradient aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLinearGradientIntensity();
        auto eResult = vcl::test::OutputDeviceTestGradient::checkLinearGradientIntensity(aBitmap);
        exportImage("13-04_linear_gradient_intensity_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testLinearGradientSteps()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestGradient aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLinearGradientSteps();
        auto eResult = vcl::test::OutputDeviceTestGradient::checkLinearGradientSteps(aBitmap);
        exportImage("13-05_linear_gradient_steps_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testAxialGradient()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestGradient aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupAxialGradient();
        auto eResult = vcl::test::OutputDeviceTestGradient::checkAxialGradient(aBitmap);
        exportImage("13-06_axial_gradient_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testRadialGradient()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        vcl::test::OutputDeviceTestGradient aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRadialGradient();
        auto eResult = vcl::test::OutputDeviceTestGradient::checkRadialGradient(aBitmap);
        exportImage("13-07_radial_gradient_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testRadialGradientOfs()
    {
        vcl::test::OutputDeviceTestGradient aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRadialGradientOfs();
        auto eResult = vcl::test::OutputDeviceTestGradient::checkRadialGradientOfs(aBitmap);
        exportImage("13-08_radial_gradient_ofs_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testLineJoinBevel()
    {
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLineJoinBevel();
        auto eResult = vcl::test::OutputDeviceTestLine::checkLineJoinBevel(aBitmap);
        exportImage("14-01_line_join_bevel_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testLineJoinRound()
    {
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLineJoinRound();
        auto eResult = vcl::test::OutputDeviceTestLine::checkLineJoinRound(aBitmap);
        exportImage("14-02_line_join_round_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testLineJoinMiter()
    {
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLineJoinMiter();
        auto eResult = vcl::test::OutputDeviceTestLine::checkLineJoinMiter(aBitmap);
        exportImage("14-03_line_join_miter_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testLineJoinNone()
    {
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLineJoinNone();
        auto eResult = vcl::test::OutputDeviceTestLine::checkLineJoinNone(aBitmap);
        exportImage("14-04_line_join_none_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testLineCapRound()
    {
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLineCapRound();
        auto eResult = vcl::test::OutputDeviceTestLine::checkLineCapRound(aBitmap);
        exportImage("14-05_line_cap_round_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testLineCapSquare()
    {
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLineCapSquare();
        auto eResult = vcl::test::OutputDeviceTestLine::checkLineCapSquare(aBitmap);
        exportImage("14-06_line_cap_square_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testLineCapButt()
    {
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupLineCapButt();
        auto eResult = vcl::test::OutputDeviceTestLine::checkLineCapButt(aBitmap);
        exportImage("14-07_line_cap_butt_test.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDropShapeWithPolyline()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDropShape();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap);
        exportImage("15-01_drop_shape_test-polyline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDropShapeAAWithPolyline()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupAADropShape();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap, true);
        exportImage("15-02_drop_shape_AA_test-polyline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDropShapeWithPolygon()
    {
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDropShape();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap);
        exportImage("16-01_drop_shape_test-polygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDropShapeAAWithPolygon()
    {
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupAADropShape();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap, true);
        exportImage("16-02_drop_shape_AA_test-polygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawHaflEllipseWithPolyLine()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkHalfEllipse(aBitmap);
        exportImage("17-01_half_ellipse_test-polyline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawHaflEllipseAAWithPolyLine()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkHalfEllipse(aBitmap, true);
        exportImage("17-02_half_ellipse_AA_test-polyline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawHaflEllipseWithPolyLineB2D()
    {
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkHalfEllipse(aBitmap);
        exportImage("17-03_half_ellipse_test-polylineb2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawHaflEllipseAAWithPolyLineB2D()
    {
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkHalfEllipse(aBitmap, true);
        exportImage("17-03_half_ellipse_AA_test-polylineb2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawHaflEllipseWithPolygon()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkHalfEllipse(aBitmap);
        exportImage("17-04_half_ellipse_test-polygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawHaflEllipseAAWithPolygon()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkHalfEllipse(aBitmap, true);
        exportImage("17-05_half_ellipse_AA_test-polygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testClosedBezierWithPolyline()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupClosedBezier();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkClosedBezier(aBitmap);
        exportImage("18-01_closed_bezier-polyline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testClosedBezierWithPolygon()
    {
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupClosedBezier();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkClosedBezier(aBitmap);
        exportImage("18-02_closed_bezier-polygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testFilledAsymmetricalDropShape()
    {
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupFilledAsymmetricalDropShape();
        vcl::test::TestResult eResult
            = vcl::test::OutputDeviceTestLine::checkFilledAsymmetricalDropShape(aBitmap);
        exportImage("19-01_filled_drop_shape-polygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    // Test SalGraphics::blendBitmap() and blendAlphaBitmap() calls.
    void testDrawBlendExtended()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        // Create virtual device with alpha.
        ScopedVclPtr<VirtualDevice> device
            = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
#ifdef MACOSX
        // TODO: This unit test is not executed for macOS unless bitmap scaling is implemented
        if (getRenderBackendName(device) == "aqua")
            return;
#endif
        device->SetOutputSizePixel(Size(10, 10));
        device->SetBackground(Wallpaper(COL_WHITE));
        device->Erase();
        Bitmap bitmap(Size(5, 5), vcl::PixelFormat::N24_BPP);
        bitmap.Erase(COL_BLUE);
        // No alpha, this will actually call SalGraphics::DrawBitmap(), but still check
        // the alpha of the device is handled correctly.
        device->DrawBitmapEx(Point(2, 2), BitmapEx(bitmap));
        exportDevice("blend_extended_01.png", device);
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(2, 2)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(6, 6)));
        // Check pixels outside of the bitmap aren't affected.
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(1, 1)));
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(7, 7)));

        device->Erase();
        AlphaMask alpha(Size(5, 5));
        alpha.Erase(0); // opaque
        device->DrawBitmapEx(Point(2, 2), BitmapEx(bitmap, alpha));
        exportDevice("blend_extended_02.png", device);
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(2, 2)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(6, 6)));

        device->Erase();
        alpha.Erase(255); // transparent
        device->DrawBitmapEx(Point(2, 2), BitmapEx(bitmap, alpha));
        exportDevice("blend_extended_03.png", device);
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(2, 2)));
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(6, 6)));

        // Skia optimizes bitmaps that have just been Erase()-ed, so explicitly
        // set some pixels in the alpha to avoid this and have an actual bitmap
        // as the alpha mask.
        device->Erase();
        alpha.Erase(255); // transparent
        BitmapWriteAccess* alphaWrite = alpha.AcquireAlphaWriteAccess();
        alphaWrite->SetPixelIndex(0, 0, 255); // opaque
        alpha.ReleaseAccess(alphaWrite);
        device->DrawBitmapEx(Point(2, 2), BitmapEx(bitmap, alpha));
        exportDevice("blend_extended_04.png", device);
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(2, 2)));
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(6, 6)));
    }

    void testDrawAlphaBitmapMirrored()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        // Normal virtual device.
        ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
#ifdef MACOSX
        // TODO: This unit test is not executed for macOS unless bitmap scaling is implemented
        if (getRenderBackendName(device) == "aqua")
            return;
#endif
        // Virtual device with alpha.
        ScopedVclPtr<VirtualDevice> alphaDevice
            = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
        device->SetOutputSizePixel(Size(20, 20));
        device->SetBackground(Wallpaper(COL_BLACK));
        device->Erase();
        alphaDevice->SetOutputSizePixel(Size(20, 20));
        alphaDevice->SetBackground(Wallpaper(COL_BLACK));
        alphaDevice->Erase();
        Bitmap bitmap(Size(4, 4), vcl::PixelFormat::N24_BPP);
        AlphaMask alpha(Size(4, 4));
        bitmap.Erase(COL_LIGHTBLUE);
        {
            BitmapScopedWriteAccess writeAccess(bitmap);
            writeAccess->SetPixel(3, 3, COL_LIGHTRED);
        }
        // alpha 127 will make COL_LIGHTRED -> COL_RED and the same for blue
        alpha.Erase(127);
        // Normal device.
        device->DrawBitmapEx(Point(5, 5), Size(-4, -4), BitmapEx(bitmap));
        device->DrawBitmapEx(Point(15, 15), Size(4, 4), BitmapEx(bitmap));
        exportDevice("draw_alpha_bitmap_mirrored_01.png", device);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, device->GetPixel(Point(18, 18)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, device->GetPixel(Point(17, 18)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, device->GetPixel(Point(2, 2)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, device->GetPixel(Point(3, 2)));
        device->Erase();
        device->DrawBitmapEx(Point(5, 5), Size(-4, -4), BitmapEx(bitmap, alpha));
        device->DrawBitmapEx(Point(15, 15), Size(4, 4), BitmapEx(bitmap, alpha));
        exportDevice("draw_alpha_bitmap_mirrored_02.png", device);
        CPPUNIT_ASSERT_EQUAL(COL_RED, device->GetPixel(Point(18, 18)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(17, 18)));
        CPPUNIT_ASSERT_EQUAL(COL_RED, device->GetPixel(Point(2, 2)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(3, 2)));
        device->Erase();
        // Now with alpha device.
        alphaDevice->DrawBitmapEx(Point(5, 5), Size(-4, -4), BitmapEx(bitmap));
        alphaDevice->DrawBitmapEx(Point(15, 15), Size(4, 4), BitmapEx(bitmap));
        exportDevice("draw_alpha_bitmap_mirrored_03.png", alphaDevice);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, alphaDevice->GetPixel(Point(18, 18)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, alphaDevice->GetPixel(Point(17, 18)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, alphaDevice->GetPixel(Point(2, 2)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, alphaDevice->GetPixel(Point(3, 2)));
        alphaDevice->Erase();
        alphaDevice->DrawBitmapEx(Point(5, 5), Size(-4, -4), BitmapEx(bitmap, alpha));
        alphaDevice->DrawBitmapEx(Point(15, 15), Size(4, 4), BitmapEx(bitmap, alpha));
        exportDevice("draw_alpha_bitmap_mirrored_04.png", alphaDevice);
        CPPUNIT_ASSERT_EQUAL(COL_RED, alphaDevice->GetPixel(Point(18, 18)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, alphaDevice->GetPixel(Point(17, 18)));
        CPPUNIT_ASSERT_EQUAL(COL_RED, alphaDevice->GetPixel(Point(2, 2)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, alphaDevice->GetPixel(Point(3, 2)));
        alphaDevice->Erase();
    }

    void testDrawingText()
    {
        vcl::test::OutputDeviceTestText aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupTextBitmap();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkTextLocation(aBitmap);
        exportImage("17-01_test_text_Drawing.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testEvenOddRuleInIntersectionRectangles()
    {
        vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupIntersectingRectangles();
        auto eResult
            = vcl::test::OutputDeviceTestCommon::checkEvenOddRuleInIntersectingRecs(aBitmap);
        exportImage("18-01_test_Even-Odd-rule_intersecting_Recs.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawOpenPolygonWithPolyLine()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkOpenPolygon(aBitmap);
        exportImage("19-01_open_polygon-polyline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawOpenPolygonWithPolyLineB2D()
    {
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkOpenPolygon(aBitmap);
        exportImage("19-02_open_polygon-polyline_b2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawOpenPolygonWithPolygon()
    {
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkOpenPolygon(aBitmap);
        exportImage("19-03_open_polygon-polygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawOpenPolygonWithPolyPolygon()
    {
        vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkOpenPolygon(aBitmap);
        exportImage("19-04_open_polygon-polypolygon.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawOpenPolygonWithPolyPolygonB2D()
    {
        vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkOpenPolygon(aBitmap);
        exportImage("19-04_open_polygon-polypolygon_b2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawOpenBezierWithPolyLine()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupOpenBezier();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkOpenBezier(aBitmap);
        exportImage("19-01_open_bezier-polyline.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawOpenBezierWithPolyLineB2D()
    {
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupOpenBezier();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkOpenBezier(aBitmap);
        exportImage("19-01_open_bezier-polyline_b2d.png", aBitmap);
        if (SHOULD_ASSERT)
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testTdf124848()
    {
        ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
#ifdef MACOSX
        // TODO: This unit test is not executed for macOS unless bitmap scaling is implemented
        if (getRenderBackendName(device) == "aqua")
            return;
#endif
        device->SetOutputSizePixel(Size(100, 100));
        device->SetBackground(Wallpaper(COL_WHITE));
        device->Erase();
        device->SetAntialiasing(AntialiasingFlags::Enable);
        device->SetLineColor(COL_BLACK);
        basegfx::B2DHomMatrix matrix;
        // DrawPolyLine() would apply the whole matrix to the line width, making it negative
        // in case of a larger rotation.
        matrix.rotate(M_PI); //180 degrees
        matrix.translate(100, 100);
        CPPUNIT_ASSERT(device->DrawPolyLineDirect(matrix,
                                                  basegfx::B2DPolygon{ { 50, 50 }, { 50, 100 } },
                                                  100, 0, nullptr, basegfx::B2DLineJoin::Miter));
        exportDevice("tdf124848-1.png", device);
        // 100px wide line should fill the entire width of the upper half
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(2, 2)));

        // Also check hairline.
        device->Erase();
        CPPUNIT_ASSERT(device->DrawPolyLineDirect(matrix,
                                                  basegfx::B2DPolygon{ { 50, 50 }, { 50, 100 } }, 0,
                                                  0, nullptr, basegfx::B2DLineJoin::Miter));
        exportDevice("tdf124848-2.png", device);
        // 1px wide
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(50, 20)));
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(49, 20)));
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(51, 20)));
    }

    void testTdf136171()
    {
        if (getDefaultDeviceBitCount() < 24)
            return;
        // Create virtual device with alpha.
        ScopedVclPtr<VirtualDevice> device
            = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
        device->SetOutputSizePixel(Size(10, 10));
        device->SetBackground(Wallpaper(COL_WHITE));
        device->Erase();
        Bitmap bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP);
        bitmap.Erase(COL_BLUE);
        basegfx::B2DHomMatrix matrix;
        matrix.scale(bitmap.GetSizePixel().Width(),
                     bitmap.GetSizePixel().Height()); // draw as 10x10
        // Draw a blue bitmap to the device. The bug was that there was no alpha, but OutputDevice::DrawTransformBitmapExDirect()
        // supplied a fully opaque alpha done with Erase() on the alpha bitmap, and Skia backend didn't handle such alpha correctly.
        device->DrawTransformedBitmapEx(matrix, BitmapEx(bitmap));
        exportDevice("tdf136171.png", device);
        // The whole virtual device content now should be blue.
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(0, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(9, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(0, 9)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(9, 9)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device->GetPixel(Point(4, 4)));
    }

    void testTdf145811()
    {
        // VCL may call copyArea()/copyBits() of backends even with coordinates partially
        // outside of the device, so try various copying like that.
        ScopedVclPtr<VirtualDevice> device1 = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
        device1->SetOutputSizePixel(Size(100, 100));
        device1->SetBackground(Wallpaper(COL_YELLOW));
        device1->Erase();
        device1->SetLineColor(COL_BLUE);
        device1->DrawPixel(Point(0, 0), COL_BLUE);
        device1->DrawPixel(Point(99, 99), COL_BLUE);
#ifdef MACOSX
        // TODO: This unit test is not executed for macOS unless bitmap scaling is implemented
        if (getRenderBackendName(device1) == "aqua")
            return;
#endif

        // Plain 1:1 copy device1->device2.
        ScopedVclPtr<VirtualDevice> device2 = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
        device2->SetOutputSizePixel(Size(100, 100));
        device2->DrawOutDev(Point(0, 0), Size(100, 100), Point(0, 0), Size(100, 100), *device1);
        exportDevice("tdf145811-1.png", device2);
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(0, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(1, 1)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(98, 98)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(99, 99)));

        // For the rest call directly SalGraphics, because OutputDevice does range checking,
        // but other code may call copyArea()/copyBits() of SalGraphics directly without range checking.
        SalGraphics* graphics1 = device1->GetGraphics();
        SalGraphics* graphics2 = device2->GetGraphics();

        device2->DrawOutDev(Point(0, 0), Size(100, 100), Point(0, 0), Size(100, 100), *device1);
        // Copy device1->device2 offset by 10,10.
        graphics2->CopyBits(SalTwoRect(0, 0, 100, 100, 10, 10, 100, 100), *graphics1, *device2,
                            *device1);
        exportDevice("tdf145811-2.png", device2);
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(0, 0))); // unmodified
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(9, 9)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(10, 10)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(11, 11)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(99, 99)));

        device2->DrawOutDev(Point(0, 0), Size(100, 100), Point(0, 0), Size(100, 100), *device1);
        // Copy area of device2 offset by 10,10.
        graphics2->CopyArea(10, 10, 0, 0, 100, 100, *device1);
        exportDevice("tdf145811-3.png", device2);
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(0, 0))); // unmodified
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(9, 9)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(10, 10)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(11, 11)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(99, 99)));

        device2->DrawOutDev(Point(0, 0), Size(100, 100), Point(0, 0), Size(100, 100), *device1);
        // Copy device1->device2 offset by -20,-20.
        graphics2->CopyBits(SalTwoRect(0, 0, 100, 100, -20, -20, 100, 100), *graphics1, *device2,
                            *device1);
        exportDevice("tdf145811-4.png", device2);
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(0, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(78, 78)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(79, 79)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(80, 80)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(99, 99))); // unmodified

        device2->DrawOutDev(Point(0, 0), Size(100, 100), Point(0, 0), Size(100, 100), *device1);
        // Copy area of device2 offset by -20,-20.
        graphics2->CopyArea(-20, -20, 0, 0, 100, 100, *device1);
        exportDevice("tdf145811-5.png", device2);
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(0, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(78, 78)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(79, 79)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(80, 80)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(99, 99))); // unmodified

        device2->DrawOutDev(Point(0, 0), Size(100, 100), Point(0, 0), Size(100, 100), *device1);
        // Copy device1->device2 offset by -10,-10 starting from -20,-20 at 150x150 size
        // (i.e. outside in all directions).
        graphics2->CopyBits(SalTwoRect(-20, -20, 150, 150, -30, -30, 150, 150), *graphics1,
                            *device2, *device1);
        exportDevice("tdf145811-6.png", device2);
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(0, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(88, 88)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(89, 89)));
        // (90,90) and further originate from outside and may be garbage.

        device2->DrawOutDev(Point(0, 0), Size(100, 100), Point(0, 0), Size(100, 100), *device1);
        // Copy area of device2 offset by -10,-10 starting from -20,-20 at 150x150 size
        // (i.e. outside in all directions).
        graphics2->CopyArea(-30, -30, -20, -20, 150, 150, *device1);
        exportDevice("tdf145811-7.png", device2);
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(0, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, device2->GetPixel(Point(88, 88)));
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, device2->GetPixel(Point(89, 89)));
        // (90,90) and further originate from outside and may be garbage.
    }

    CPPUNIT_TEST_SUITE(BackendTest);
    CPPUNIT_TEST(testDrawRectWithRectangle);
    CPPUNIT_TEST(testDrawRectWithPixel);
    CPPUNIT_TEST(testDrawRectWithLine);
    CPPUNIT_TEST(testDrawRectWithPolygon);
    CPPUNIT_TEST(testDrawRectWithPolyLine);
    CPPUNIT_TEST(testDrawRectWithPolyLineB2D);
    CPPUNIT_TEST(testDrawRectWithPolyPolygon);
    CPPUNIT_TEST(testDrawRectWithPolyPolygonB2D);

    CPPUNIT_TEST(testDrawRectAAWithRectangle);
    CPPUNIT_TEST(testDrawRectAAWithPixel);
    CPPUNIT_TEST(testDrawRectAAWithLine);
    CPPUNIT_TEST(testDrawRectAAWithPolygon);
    CPPUNIT_TEST(testDrawRectAAWithPolyLine);
    CPPUNIT_TEST(testDrawRectAAWithPolyLineB2D);
    CPPUNIT_TEST(testDrawRectAAWithPolyPolygon);
    CPPUNIT_TEST(testDrawRectAAWithPolyPolygonB2D);

    CPPUNIT_TEST(testDrawFilledRectWithRectangle);
    CPPUNIT_TEST(testDrawFilledRectWithPolygon);
    CPPUNIT_TEST(testDrawFilledRectWithPolyPolygon);
    CPPUNIT_TEST(testDrawFilledRectWithPolyPolygon2D);

    CPPUNIT_TEST(testDrawDiamondWithPolygon);
    CPPUNIT_TEST(testDrawDiamondWithLine);
    CPPUNIT_TEST(testDrawDiamondWithPolyline);
    CPPUNIT_TEST(testDrawDiamondWithPolylineB2D);

    CPPUNIT_TEST(testDrawInvertWithRectangle);
    CPPUNIT_TEST(testDrawInvertN50WithRectangle);
    CPPUNIT_TEST(testDrawInvertTrackFrameWithRectangle);

    CPPUNIT_TEST(testDrawBezierWithPolylineB2D);
    CPPUNIT_TEST(testDrawBezierAAWithPolylineB2D);

    CPPUNIT_TEST(testDrawDropShapeWithPolyline);
    CPPUNIT_TEST(testDrawDropShapeAAWithPolyline);

    CPPUNIT_TEST(testDrawDropShapeWithPolygon);
    CPPUNIT_TEST(testDrawDropShapeAAWithPolygon);

    CPPUNIT_TEST(testDrawHaflEllipseWithPolyLine);
    CPPUNIT_TEST(testDrawHaflEllipseAAWithPolyLine);
    CPPUNIT_TEST(testDrawHaflEllipseWithPolyLineB2D);
    CPPUNIT_TEST(testDrawHaflEllipseAAWithPolyLineB2D);
    CPPUNIT_TEST(testDrawHaflEllipseWithPolygon);
    CPPUNIT_TEST(testDrawHaflEllipseAAWithPolygon);

    CPPUNIT_TEST(testClosedBezierWithPolyline);
    CPPUNIT_TEST(testClosedBezierWithPolygon);

    CPPUNIT_TEST(testFilledAsymmetricalDropShape);

    CPPUNIT_TEST(testDrawBitmap24bpp);
    CPPUNIT_TEST(testDrawTransformedBitmap24bpp);
    CPPUNIT_TEST(testComplexDrawTransformedBitmap24bpp);
    CPPUNIT_TEST(testDrawBitmapExWithAlpha24bpp);
    CPPUNIT_TEST(testDrawMask24bpp);
    CPPUNIT_TEST(testDrawBlend24bpp);

    CPPUNIT_TEST(testDrawXor);

    CPPUNIT_TEST(testDrawBitmap32bpp);
    CPPUNIT_TEST(testDrawTransformedBitmap32bpp);
    CPPUNIT_TEST(testDrawBitmapExWithAlpha32bpp);
    CPPUNIT_TEST(testDrawMask32bpp);
    CPPUNIT_TEST(testDrawBlend32bpp);

    CPPUNIT_TEST(testDrawTransformedBitmap8bppGreyScale);
    CPPUNIT_TEST(testDrawBitmap8bppGreyScale);

    CPPUNIT_TEST(testDrawTransformedBitmapExAlpha);

    CPPUNIT_TEST(testClipRectangle);
    CPPUNIT_TEST(testClipPolygon);
    CPPUNIT_TEST(testClipPolyPolygon);
    CPPUNIT_TEST(testClipB2DPolyPolygon);

    CPPUNIT_TEST(testDrawOutDev);
    CPPUNIT_TEST(testDrawOutDevScaledClipped);
    CPPUNIT_TEST(testDrawOutDevSelf);

    CPPUNIT_TEST(testDashedLine);

    CPPUNIT_TEST(testErase);

    CPPUNIT_TEST(testLinearGradient);
    CPPUNIT_TEST(testLinearGradientAngled);
    CPPUNIT_TEST(testLinearGradientBorder);
    CPPUNIT_TEST(testLinearGradientIntensity);
    CPPUNIT_TEST(testLinearGradientSteps);
    CPPUNIT_TEST(testAxialGradient);
    CPPUNIT_TEST(testRadialGradient);
    CPPUNIT_TEST(testRadialGradientOfs);

    CPPUNIT_TEST(testLineCapRound);
    CPPUNIT_TEST(testLineCapSquare);
    CPPUNIT_TEST(testLineCapButt);
    CPPUNIT_TEST(testLineJoinBevel);
    CPPUNIT_TEST(testLineJoinRound);
    CPPUNIT_TEST(testLineJoinMiter);
    CPPUNIT_TEST(testLineJoinNone);

    CPPUNIT_TEST(testDrawBlendExtended);
    CPPUNIT_TEST(testDrawAlphaBitmapMirrored);

    CPPUNIT_TEST(testDrawingText);
    CPPUNIT_TEST(testEvenOddRuleInIntersectionRectangles);

    CPPUNIT_TEST(testDrawOpenPolygonWithPolyLine);
    CPPUNIT_TEST(testDrawOpenPolygonWithPolyLineB2D);
    CPPUNIT_TEST(testDrawOpenPolygonWithPolygon);
    CPPUNIT_TEST(testDrawOpenPolygonWithPolyPolygon);
    CPPUNIT_TEST(testDrawOpenPolygonWithPolyPolygonB2D);

    CPPUNIT_TEST(testDrawOpenBezierWithPolyLine);
    CPPUNIT_TEST(testDrawOpenBezierWithPolyLineB2D);

    CPPUNIT_TEST(testTdf124848);
    CPPUNIT_TEST(testTdf136171);
    CPPUNIT_TEST(testTdf145811);

    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BackendTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
