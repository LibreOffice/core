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

#include <test/outputdevice.hxx>

class BackendTest : public test::BootstrapFixture
{
    // if enabled - check the result images with:
    // "xdg-open ./workdir/CppunitTest/vcl_backend_test.test.core/"
    static constexpr const bool mbExportBitmap = false;

    void exportImage(OUString const& rsFilename, Bitmap const& rBitmap)
    {
        if (mbExportBitmap)
        {
            Bitmap aBitmap(rBitmap);
            aBitmap.Scale(Size(128, 128), BmpScaleFlag::Fast);
            SvFileStream aStream(rsFilename, StreamMode::WRITE | StreamMode::TRUNC);
            GraphicFilter::GetGraphicFilter().compressAsPNG(aBitmap, aStream);
        }
    }

public:
    BackendTest()
        : BootstrapFixture(true, false)
    {
    }

    // We need to enable tests ONE BY ONE as they fail because of backend bugs
    // it is still important to have the test defined so we know the issues
    // exist and we need to fix them. Consistent behaviour of our backends
    // is of highest priority.

    void testDrawRectWithRectangle()
    {
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
        exportImage("01-01_rectangle_test-rectangle.png", aBitmap);

        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPixel()
    {
        vcl::test::OutputDeviceTestPixel aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
        exportImage("01-02_rectangle_test-pixel.png", aBitmap);

        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithLine()
    {
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
        exportImage("01-03_rectangle_test-line.png", aBitmap);

        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPolygon()
    {
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
        exportImage("01-04_rectangle_test-polygon.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPolyLine()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
        exportImage("01-05_rectangle_test-polyline.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPolyLineB2D()
    {
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
        exportImage("01-06_rectangle_test-polyline_b2d.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPolyPolygon()
    {
        vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
        exportImage("01-07_rectangle_test-polypolygon.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectWithPolyPolygonB2D()
    {
        vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(false);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
        exportImage("01-08_rectangle_test-polypolygon_b2d.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithRectangle()
    {
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-01_rectangle_AA_test-rectangle.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPixel()
    {
        vcl::test::OutputDeviceTestPixel aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-02_rectangle_AA_test-pixel.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithLine()
    {
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-03_rectangle_AA_test-line.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPolygon()
    {
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-04_rectangle_AA_test-polygon.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPolyLine()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-05_rectangle_AA_test-polyline.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPolyLineB2D()
    {
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-06_rectangle_AA_test-polyline_b2d.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPolyPolygon()
    {
        vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-07_rectangle_AA_test-polypolygon.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawRectAAWithPolyPolygonB2D()
    {
        vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupRectangle(true);
        auto eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
        exportImage("02-08_rectangle_AA_test-polypolygon_b2d.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawFilledRectWithRectangle()
    {
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupFilledRectangle();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap);
        exportImage("03-01_filled_rectangle_test-rectangle.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawFilledRectWithPolygon()
    {
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupFilledRectangle();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap);
        exportImage("03-02_filled_rectangle_test-polygon.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawFilledRectWithPolyPolygon()
    {
        vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupFilledRectangle();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap);
        exportImage("03-03_filled_rectangle_test-polypolygon.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawFilledRectWithPolyPolygon2D()
    {
        vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupFilledRectangle();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap);
        exportImage("03-04_filled_rectangle_test-polypolygon_b2d.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDiamondWithPolygon()
    {
        vcl::test::OutputDeviceTestPolygon aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDiamond();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
        exportImage("04-01_diamond_test-polygon.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDiamondWithLine()
    {
        vcl::test::OutputDeviceTestLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDiamond();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
        exportImage("04-02_diamond_test-line.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDiamondWithPolyline()
    {
        vcl::test::OutputDeviceTestPolyLine aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDiamond();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
        exportImage("04-03_diamond_test-polyline.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawDiamondWithPolylineB2D()
    {
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDiamond();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
        exportImage("04-04_diamond_test-polyline_b2d.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawInvertWithRectangle()
    {
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupInvert_NONE();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkInvertRectangle(aBitmap);
        exportImage("05-01_invert_test-rectangle.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawInvertN50WithRectangle()
    {
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupInvert_N50();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkInvertN50Rectangle(aBitmap);
        exportImage("05-02_invert_N50_test-rectangle.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawInvertTrackFrameWithRectangle()
    {
        vcl::test::OutputDeviceTestRect aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupInvert_TrackFrame();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkInvertTrackFrameRectangle(aBitmap);
        exportImage("05-03_invert_TrackFrame_test-rectangle.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBezierWithPolylineB2D()
    {
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupBezier();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
        exportImage("06-01_bezier_test-polyline_b2d.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBezierAAWithPolylineB2D()
    {
        vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupAABezier();
        auto eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
        exportImage("07-01_bezier_AA_test-polyline_b2d.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBitmap()
    {
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawBitmap();
        exportImage("08-01_bitmap_test.png", aBitmap);
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawTransformedBitmap()
    {
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap();
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
        exportImage("08-02_transformed_bitmap_test.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawBitmapExWithAlpha()
    {
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha();
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkBitmapExWithAlpha(aBitmap);
        exportImage("08-03_bitmapex_with_alpha_test.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    void testDrawMask()
    {
        vcl::test::OutputDeviceTestBitmap aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupDrawMask();
        auto eResult = vcl::test::OutputDeviceTestBitmap::checkMask(aBitmap);
        exportImage("08-04_mask_test.png", aBitmap);
        if (aOutDevTest.getRenderBackendName() == "skia")
            CPPUNIT_ASSERT(eResult != vcl::test::TestResult::Failed);
    }

    CPPUNIT_TEST_SUITE(BackendTest);
    CPPUNIT_TEST(testDrawRectWithRectangle);
    //    CPPUNIT_TEST(testDrawRectWithPixel); TODO SKIA
    CPPUNIT_TEST(testDrawRectWithLine);
    CPPUNIT_TEST(testDrawRectWithPolygon);
    CPPUNIT_TEST(testDrawRectWithPolyLine);
    CPPUNIT_TEST(testDrawRectWithPolyLineB2D);
    CPPUNIT_TEST(testDrawRectWithPolyPolygon);
    CPPUNIT_TEST(testDrawRectWithPolyPolygonB2D);

    CPPUNIT_TEST(testDrawRectAAWithRectangle);
    //    CPPUNIT_TEST(testDrawRectAAWithPixel); TODO SKIA
    CPPUNIT_TEST(testDrawRectAAWithLine);
    CPPUNIT_TEST(testDrawRectAAWithPolygon);
    CPPUNIT_TEST(testDrawRectAAWithPolyLine);
    CPPUNIT_TEST(testDrawRectAAWithPolyLineB2D);
    CPPUNIT_TEST(testDrawRectAAWithPolyPolygon);
    CPPUNIT_TEST(testDrawRectAAWithPolyPolygonB2D);

    CPPUNIT_TEST(testDrawFilledRectWithRectangle);
    //    CPPUNIT_TEST(testDrawFilledRectWithPolygon); TODO SKIA
    //    CPPUNIT_TEST(testDrawFilledRectWithPolyPolygon); TODO SKIA
    //    CPPUNIT_TEST(testDrawFilledRectWithPolyPolygon2D); TODO SKIA

    //    CPPUNIT_TEST(testDrawDiamondWithPolygon); TODO SKIA
    //    CPPUNIT_TEST(testDrawDiamondWithLine); TODO SKIA
    CPPUNIT_TEST(testDrawDiamondWithPolyline);
    CPPUNIT_TEST(testDrawDiamondWithPolylineB2D);

    CPPUNIT_TEST(testDrawInvertWithRectangle);
    CPPUNIT_TEST(testDrawInvertN50WithRectangle);
    //    CPPUNIT_TEST(testDrawInvertTrackFrameWithRectangle); TODO SKIA

    CPPUNIT_TEST(testDrawBezierWithPolylineB2D);
    CPPUNIT_TEST(testDrawBezierAAWithPolylineB2D);

    CPPUNIT_TEST(testDrawBitmap);
    CPPUNIT_TEST(testDrawTransformedBitmap);
    CPPUNIT_TEST(testDrawBitmapExWithAlpha);
    CPPUNIT_TEST(testDrawMask);

    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BackendTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
