/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/outputdevice.hxx>
#include <unotools/bootstrap.hxx>
#include <vcl/test/GraphicsRenderTests.hxx>
#include <tools/stream.hxx>

#define SHOULD_ASSERT                                                                              \
    (aOutDevTest.getRenderBackendName() != "qt5" && aOutDevTest.getRenderBackendName() != "qt5svp" \
     && aOutDevTest.getRenderBackendName() != "gtk3svp"                                            \
     && aOutDevTest.getRenderBackendName() != "aqua"                                               \
     && aOutDevTest.getRenderBackendName() != "gen"                                                \
     && aOutDevTest.getRenderBackendName() != "genpsp"                                             \
     && aOutDevTest.getRenderBackendName() != "win")

void GraphicsRenderTests::updateResult(vcl::test::TestResult const result, OString atestname)
{
    switch (result)
    {
        case vcl::test::TestResult::Passed:
            m_aPassed.push_back(atestname);
            return;
        case vcl::test::TestResult::PassedWithQuirks:
            m_aQuirky.push_back(atestname);
            return;
        case vcl::test::TestResult::Failed:
            m_aFailed.push_back(atestname);
            return;
    }
}

void GraphicsRenderTests::testDrawRectWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    m_aCurGraphicsBackend = aOutDevTest.getRenderBackendName();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithRectangle");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithRectangle");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectWithRectangle"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPixel");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPixel");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectWithPixel"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithLine");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithLine");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectWithLine"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPolygon");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPolygon");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectWithPolygon"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPolyLine");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPolyLine");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectWithPolyLine"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPolyLineB2D");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPolyLineB2D");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectWithPolyLineB2D"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPolyPolygon");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPolyPolygon");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectWithPolyPolygon"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPolyPolygonB2D");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPolyPolygonB2D");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectWithPolyPolygonB2D"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectAAWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithRectangle");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithRectangle");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectAAWithRectangle"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectAAWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPixel");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPixel");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectAAWithPixel"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectAAWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithLine");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithLine");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectAAWithLine"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPolygon");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPolygon");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectAAWithPolygon"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPolyLine");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPolyLine");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectAAWithPolyLine"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPolyLineB2D");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPolyLineB2D");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectAAWithPolyLineB2D"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPolyPolygon");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPolyPolygon");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectAAWithPolyPolygon"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPolyPolygonB2D");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPolyPolygonB2D");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawRectAAWithPolyPolygonB2D"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawFilledRectWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawFilledRectWithRectangle");
        m_aSkipped.push_back("testDrawFilledRectWithRectangleWithAA");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    updateResult(eResult, "testDrawFilledRectWithRectangle");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawFilledRectWithRectangle"] = aBitmap;
    }
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    updateResult(eResult, "testDrawFilledRectWithRectangleWithAA");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawFilledRectWithRectangleWithAA"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawFilledRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawFilledRectWithPolygon");
        m_aSkipped.push_back("testDrawFilledRectWithPolygonWithAA");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    updateResult(eResult, "testDrawFilledRectWithPolygon");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawFilledRectWithPolygon"] = aBitmap;
    }
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    updateResult(eResult, "testDrawFilledRectWithPolygonWithAA");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawFilledRectWithPolygonWithAA"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawFilledRectWithPolyPolygon");
        m_aSkipped.push_back("testDrawFilledRectWithPolyPolygonWithAA");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    updateResult(eResult, "testDrawFilledRectWithPolyPolygon");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawFilledRectWithPolyPolygon"] = aBitmap;
    }
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    updateResult(eResult, "testDrawFilledRectWithPolyPolygonWithAA");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawFilledRectWithPolyPolygonWithAA"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawFilledRectWithPolyPolygon2D");
        m_aSkipped.push_back("testDrawFilledRectWithPolyPolygon2DWithAA");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    updateResult(eResult, "testDrawFilledRectWithPolyPolygon2D");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawFilledRectWithPolyPolygon2D"] = aBitmap;
    }
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    updateResult(eResult, "testDrawFilledRectWithPolyPolygon2DWithAA");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawFilledRectWithPolyPolygon2DWithAA"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawDiamondWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawDiamondWithPolygon");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    updateResult(eResult, "testDrawDiamondWithPolygon");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawDiamondWithPolygon"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawDiamondWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawDiamondWithLine");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    updateResult(eResult, "testDrawDiamondWithLine");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawDiamondWithLine"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawDiamondWithPolyline()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawDiamondWithPolyline");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    updateResult(eResult, "testDrawDiamondWithPolyline");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawDiamondWithPolyline"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawDiamondWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawDiamondWithPolylineB2D");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    updateResult(eResult, "testDrawDiamondWithPolylineB2D");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawDiamondWithPolylineB2D"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawInvertWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_NONE();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawInvertWithRectangle");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertRectangle(aBitmap);
    updateResult(eResult, "testDrawInvertWithRectangle");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawInvertWithRectangle"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawInvertN50WithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_N50();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawInvertN50WithRectangle");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertN50Rectangle(aBitmap);
    updateResult(eResult, "testDrawInvertN50WithRectangle");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawInvertN50WithRectangle"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawInvertTrackFrameWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_TrackFrame();
    if (!(SHOULD_ASSERT && aOutDevTest.getRenderBackendName() != "svp"))
    {
        m_aSkipped.push_back("testDrawInvertTrackFrameWithRectangle");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertTrackFrameRectangle(aBitmap);
    updateResult(eResult, "testDrawInvertTrackFrameWithRectangle");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawInvertTrackFrameWithRectangle"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawBezierWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupBezier();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawBezierWithPolylineB2D");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    updateResult(eResult, "testDrawBezierWithPolylineB2D");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawBezierWithPolylineB2D"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawBezierAAWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAABezier();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawBezierAAWithPolylineB2D");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    updateResult(eResult, "testDrawBezierAAWithPolylineB2D");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawBezierAAWithPolylineB2D"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawBitmap");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    updateResult(eResult, "testDrawBitmap");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawBitmap"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawTransformedBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawTransformedBitmap");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    updateResult(eResult, "testDrawTransformedBitmap");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawTransformedBitmap"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawBitmapExWithAlpha()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawBitmapExWithAlpha");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkBitmapExWithAlpha(aBitmap);
    updateResult(eResult, "testDrawBitmapExWithAlpha");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawBitmapExWithAlpha"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawMask()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawMask();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawMask");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkMask(aBitmap);
    updateResult(eResult, "testDrawMask");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawMask"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawBlend()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawBlend");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkBlend(aBitmapEx);
    updateResult(eResult, "testDrawBlend");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawBlend"] = aBitmapEx.GetBitmap();
    }
}

void GraphicsRenderTests::testDrawXor()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupXOR();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawXor");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestAnotherOutDev::checkXOR(aBitmap);
    updateResult(eResult, "testDrawXor");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawXor"] = aBitmap;
    }
}

void GraphicsRenderTests::testClipRectangle()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipRectangle();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testClipRectangle");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    updateResult(eResult, "testClipRectangle");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testClipRectangle"] = aBitmap;
    }
}

void GraphicsRenderTests::testClipPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolygon();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testClipPolygon");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    updateResult(eResult, "testClipPolygon");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testClipPolygon"] = aBitmap;
    }
}

void GraphicsRenderTests::testClipPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolyPolygon();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testClipPolyPolygon");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    updateResult(eResult, "testClipPolyPolygon");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testClipPolyPolygon"] = aBitmap;
    }
}

void GraphicsRenderTests::testClipB2DPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipB2DPolyPolygon();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testClipB2DPolyPolygon");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    updateResult(eResult, "testClipB2DPolyPolygon");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testClipB2DPolyPolygon"] = aBitmap;
    }
}

void GraphicsRenderTests::testDrawOutDev()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDev();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawOutDev");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDev(aBitmap);
    updateResult(eResult, "testDrawOutDev");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDrawOutDev"] = aBitmap;
    }
}

void GraphicsRenderTests::testDashedLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDashedLine();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDashedLine");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkDashedLine(aBitmap);
    updateResult(eResult, "testDashedLine");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testDashedLine"] = aBitmap;
    }
}

void GraphicsRenderTests::testLinearGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradient();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLinearGradient");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradient(aBitmap);
    updateResult(eResult, "testLinearGradient");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLinearGradient"] = aBitmap;
    }
}

void GraphicsRenderTests::testLinearGradientAngled()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientAngled();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLinearGradientAngled");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientAngled(aBitmap);
    updateResult(eResult, "testLinearGradientAngled");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLinearGradientAngled"] = aBitmap;
    }
}

void GraphicsRenderTests::testLinearGradientBorder()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientBorder();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLinearGradientBorder");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientBorder(aBitmap);
    updateResult(eResult, "testLinearGradientBorder");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLinearGradientBorder"] = aBitmap;
    }
}

void GraphicsRenderTests::testLinearGradientIntensity()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientIntensity();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLinearGradientIntensity");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientIntensity(aBitmap);
    updateResult(eResult, "testLinearGradientIntensity");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLinearGradientIntensity"] = aBitmap;
    }
}

void GraphicsRenderTests::testLinearGradientSteps()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientSteps();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLinearGradientSteps");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientSteps(aBitmap);
    updateResult(eResult, "testLinearGradientSteps");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLinearGradientSteps"] = aBitmap;
    }
}

void GraphicsRenderTests::testAxialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAxialGradient();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testAxialGradient");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkAxialGradient(aBitmap);
    updateResult(eResult, "testAxialGradient");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testAxialGradient"] = aBitmap;
    }
}

void GraphicsRenderTests::testRadialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradient();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testRadialGradient");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradient(aBitmap);
    updateResult(eResult, "testRadialGradient");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testRadialGradient"] = aBitmap;
    }
}

void GraphicsRenderTests::testRadialGradientOfs()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradientOfs();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testRadialGradientOfs");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradientOfs(aBitmap);
    updateResult(eResult, "testRadialGradientOfs");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testRadialGradientOfs"] = aBitmap;
    }
}

void GraphicsRenderTests::testLineJoinBevel()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinBevel();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineJoinBevel");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinBevel(aBitmap);
    updateResult(eResult, "testLineJoinBevel");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLineJoinBevel"] = aBitmap;
    }
}

void GraphicsRenderTests::testLineJoinRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinRound();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineJoinRound");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinRound(aBitmap);
    updateResult(eResult, "testLineJoinRound");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLineJoinRound"] = aBitmap;
    }
}

void GraphicsRenderTests::testLineJoinMiter()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinMiter();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineJoinMiter");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinMiter(aBitmap);
    updateResult(eResult, "testLineJoinMiter");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLineJoinMiter"] = aBitmap;
    }
}

void GraphicsRenderTests::testLineJoinNone()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinNone();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineJoinNone");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinNone(aBitmap);
    updateResult(eResult, "testLineJoinNone");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLineJoinNone"] = aBitmap;
    }
}

void GraphicsRenderTests::testLineCapRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapRound();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineCapRound");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapRound(aBitmap);
    updateResult(eResult, "testLineCapRound");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLineCapRound"] = aBitmap;
    }
}

void GraphicsRenderTests::testLineCapSquare()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapSquare();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineCapSquare");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapSquare(aBitmap);
    updateResult(eResult, "testLineCapSquare");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLineCapSquare"] = aBitmap;
    }
}

void GraphicsRenderTests::testLineCapButt()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapButt();
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineCapButt");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapButt(aBitmap);
    updateResult(eResult, "testLineCapButt");
    if (m_aStoreResultantBitmap)
    {
        m_aResultantBitmap["testLineCapButt"] = aBitmap;
    }
}

void GraphicsRenderTests::runALLTests()
{
    testDrawRectWithRectangle();
    testDrawRectWithPixel();
    testDrawRectWithLine();
    testDrawRectWithPolygon();
    testDrawRectWithPolyLine();
    testDrawRectWithPolyLineB2D();
    testDrawRectWithPolyPolygon();
    testDrawRectWithPolyPolygonB2D();
    testDrawRectAAWithRectangle();
    testDrawRectAAWithPixel();
    testDrawRectAAWithLine();
    testDrawRectAAWithPolygon();
    testDrawRectAAWithPolyLine();
    testDrawRectAAWithPolyLineB2D();
    testDrawRectAAWithPolyPolygon();
    testDrawRectAAWithPolyPolygonB2D();
    testDrawFilledRectWithRectangle();
    testDrawFilledRectWithPolygon();
    testDrawFilledRectWithPolyPolygon();
    testDrawFilledRectWithPolyPolygon2D();
    testDrawDiamondWithPolygon();
    testDrawDiamondWithLine();
    testDrawDiamondWithPolyline();
    testDrawDiamondWithPolylineB2D();
    testDrawInvertWithRectangle();
    testDrawInvertN50WithRectangle();
    testDrawInvertTrackFrameWithRectangle();
    testDrawBezierWithPolylineB2D();
    testDrawBezierAAWithPolylineB2D();
    testDrawBitmap();
    testDrawTransformedBitmap();
    testDrawBitmapExWithAlpha();
    testDrawMask();
    testDrawBlend();
    testDrawXor();
    testClipRectangle();
    testClipPolygon();
    testClipPolyPolygon();
    testClipB2DPolyPolygon();
    testDrawOutDev();
    testDashedLine();
    testLinearGradient();
    testLinearGradientAngled();
    testLinearGradientBorder();
    testLinearGradientIntensity();
    testLinearGradientSteps();
    testAxialGradient();
    testRadialGradient();
    testRadialGradientOfs();
    testLineJoinBevel();
    testLineJoinRound();
    testLineJoinMiter();
    testLineJoinNone();
    testLineCapRound();
    testLineCapSquare();
    testLineCapButt();
}

void GraphicsRenderTests::run(bool storeResultBitmap)
{
    m_aStoreResultantBitmap = storeResultBitmap;
    runALLTests();
    //Storing the test's results in the main user installation directory.
    OUString aUserInstallPath;
    ::utl::Bootstrap::locateUserInstallation(aUserInstallPath);
    SvFileStream logFile(aUserInstallPath + "/user/GraphicsRenderTests.log",
                         StreamMode::WRITE | StreamMode::TRUNC);
    OUString atemp = "Graphic Backend used: " + m_aCurGraphicsBackend;
    logFile.WriteLine(OUStringToOString(atemp, RTL_TEXTENCODING_UTF8));
    logFile.WriteLine("Passed tests : " + std::to_string(m_aPassed.size()));
    logFile.WriteLine("Quirky tests : " + std::to_string(m_aQuirky.size()));
    logFile.WriteLine("Failed tests : " + std::to_string(m_aFailed.size()));
    logFile.WriteLine("Skipped tests : " + std::to_string(m_aSkipped.size()));
    logFile.WriteLine("\n---Name of the tests that failed---");
    if (m_aFailed.size() > 0)
    {
        for (const class OString& tests : m_aFailed)
        {
            logFile.WriteLine(tests);
        }
    }
    else
    {
        logFile.WriteLine("No test has been failed.");
    }
    logFile.WriteLine("\n---Name of the tests that were Quirky---");
    if (m_aQuirky.size() > 0)
    {
        for (const class OString& tests : m_aQuirky)
        {
            logFile.WriteLine(tests);
        }
    }
    else
    {
        logFile.WriteLine("No test was Quirky.");
    }
    logFile.WriteLine("\n---Name of the tests that were Skipped---");
    if (m_aSkipped.size() > 0)
    {
        for (const class OString& tests : m_aSkipped)
        {
            logFile.WriteLine(tests);
        }
    }
    else
    {
        logFile.WriteLine("No test was Skipped.");
    }
}
