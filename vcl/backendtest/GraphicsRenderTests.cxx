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
#include <vcl/skia/SkiaHelper.hxx>
#include <vcl/test/GraphicsRenderTests.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>
#include <unotools/bootstrap.hxx>
#include <tools/stream.hxx>

#include <sstream>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/task/OfficeRestartManager.hpp>

#include <comphelper/configuration.hxx>
#include <bitmap/BitmapWriteAccess.hxx>

#define SHOULD_ASSERT                                                                              \
    (aOutDevTest.getRenderBackendName() != "qt5" && aOutDevTest.getRenderBackendName() != "qt5svp" \
     && aOutDevTest.getRenderBackendName() != "gtk3svp"                                            \
     && aOutDevTest.getRenderBackendName() != "aqua"                                               \
     && aOutDevTest.getRenderBackendName() != "gen"                                                \
     && aOutDevTest.getRenderBackendName() != "genpsp"                                             \
     && aOutDevTest.getRenderBackendName() != "win")

GraphicsRenderTests::GraphicsRenderTests()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    //TODO: Why is this empty?
    OUString aBackendName = aOutDevTest.getRenderBackendName();
    OString aTemp = OUStringToOString(aBackendName, RTL_TEXTENCODING_UTF8);
    m_aCurGraphicsBackend = string(aTemp.getStr(), aTemp.getLength());
}

void GraphicsRenderTests::updateResult(vcl::test::TestResult const result, string atestname)
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
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithRectangle");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithRectangle");
}

void GraphicsRenderTests::testDrawRectWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPixel");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPixel");
}

void GraphicsRenderTests::testDrawRectWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithLine");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithLine");
}

void GraphicsRenderTests::testDrawRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPolygon");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPolygon");
}

void GraphicsRenderTests::testDrawRectWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPolyLine");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPolyLine");
}

void GraphicsRenderTests::testDrawRectWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPolyLineB2D");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPolyLineB2D");
}

void GraphicsRenderTests::testDrawRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPolyPolygon");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPolyPolygon");
}

void GraphicsRenderTests::testDrawRectWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectWithPolyPolygonB2D");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    updateResult(eResult, "testDrawRectWithPolyPolygonB2D");
}

void GraphicsRenderTests::testDrawRectAAWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithRectangle");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithRectangle");
}

void GraphicsRenderTests::testDrawRectAAWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPixel");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPixel");
}

void GraphicsRenderTests::testDrawRectAAWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithLine");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithLine");
}

void GraphicsRenderTests::testDrawRectAAWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPolygon");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPolygon");
}

void GraphicsRenderTests::testDrawRectAAWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPolyLine");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPolyLine");
}

void GraphicsRenderTests::testDrawRectAAWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPolyLineB2D");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPolyLineB2D");
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPolyPolygon");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPolyPolygon");
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawRectAAWithPolyPolygonB2D");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    updateResult(eResult, "testDrawRectAAWithPolyPolygonB2D");
}

void GraphicsRenderTests::testDrawFilledRectWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawFilledRectWithRectangle");
        m_aSkipped.push_back("testDrawFilledRectWithRectangleWithAA");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    updateResult(eResult, "testDrawFilledRectWithRectangle");
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    updateResult(eResult, "testDrawFilledRectWithRectangleWithAA");
}

void GraphicsRenderTests::testDrawFilledRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawFilledRectWithPolygon");
        m_aSkipped.push_back("testDrawFilledRectWithPolygonWithAA");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    updateResult(eResult, "testDrawFilledRectWithPolygon");
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    updateResult(eResult, "testDrawFilledRectWithPolygonWithAA");
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawFilledRectWithPolyPolygon");
        m_aSkipped.push_back("testDrawFilledRectWithPolyPolygonWithAA");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    updateResult(eResult, "testDrawFilledRectWithPolyPolygon");
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    updateResult(eResult, "testDrawFilledRectWithPolyPolygonWithAA");
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawFilledRectWithPolyPolygon2D");
        m_aSkipped.push_back("testDrawFilledRectWithPolyPolygon2DWithAA");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    updateResult(eResult, "testDrawFilledRectWithPolyPolygon2D");
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    updateResult(eResult, "testDrawFilledRectWithPolyPolygon2DWithAA");
}

void GraphicsRenderTests::testDrawDiamondWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawDiamondWithPolygon");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    updateResult(eResult, "testDrawDiamondWithPolygon");
}

void GraphicsRenderTests::testDrawDiamondWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawDiamondWithLine");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    updateResult(eResult, "testDrawDiamondWithLine");
}

void GraphicsRenderTests::testDrawDiamondWithPolyline()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawDiamondWithPolyline");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    updateResult(eResult, "testDrawDiamondWithPolyline");
}

void GraphicsRenderTests::testDrawDiamondWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawDiamondWithPolylineB2D");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    updateResult(eResult, "testDrawDiamondWithPolylineB2D");
}

void GraphicsRenderTests::testDrawInvertWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawInvertWithRectangle");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupInvert_NONE();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertRectangle(aBitmap);
    updateResult(eResult, "testDrawInvertWithRectangle");
}

void GraphicsRenderTests::testDrawInvertN50WithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawInvertN50WithRectangle");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupInvert_N50();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertN50Rectangle(aBitmap);
    updateResult(eResult, "testDrawInvertN50WithRectangle");
}

void GraphicsRenderTests::testDrawInvertTrackFrameWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    if (!(SHOULD_ASSERT && aOutDevTest.getRenderBackendName() != "svp"))
    {
        m_aSkipped.push_back("testDrawInvertTrackFrameWithRectangle");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupInvert_TrackFrame();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertTrackFrameRectangle(aBitmap);
    updateResult(eResult, "testDrawInvertTrackFrameWithRectangle");
}

void GraphicsRenderTests::testDrawBezierWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawBezierWithPolylineB2D");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupBezier();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    updateResult(eResult, "testDrawBezierWithPolylineB2D");
}

void GraphicsRenderTests::testDrawBezierAAWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawBezierAAWithPolylineB2D");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupAABezier();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    updateResult(eResult, "testDrawBezierAAWithPolylineB2D");
}

void GraphicsRenderTests::testDrawBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawBitmap");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    updateResult(eResult, "testDrawBitmap");
}

void GraphicsRenderTests::testDrawTransformedBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawTransformedBitmap");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    updateResult(eResult, "testDrawTransformedBitmap");
}

void GraphicsRenderTests::testDrawBitmapExWithAlpha()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawBitmapExWithAlpha");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkBitmapExWithAlpha(aBitmap);
    updateResult(eResult, "testDrawBitmapExWithAlpha");
}

void GraphicsRenderTests::testDrawMask()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawMask");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupDrawMask();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkMask(aBitmap);
    updateResult(eResult, "testDrawMask");
}

void GraphicsRenderTests::testDrawBlend()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawBlend");
        return;
    }
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkBlend(aBitmapEx);
    updateResult(eResult, "testDrawBlend");
}

void GraphicsRenderTests::testDrawXor()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawXor");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupXOR();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestAnotherOutDev::checkXOR(aBitmap);
    updateResult(eResult, "testDrawXor");
}

void GraphicsRenderTests::testClipRectangle()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testClipRectangle");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupClipRectangle();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    updateResult(eResult, "testClipRectangle");
}

void GraphicsRenderTests::testClipPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testClipPolygon");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupClipPolygon();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    updateResult(eResult, "testClipPolygon");
}

void GraphicsRenderTests::testClipPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testClipPolyPolygon");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupClipPolyPolygon();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    updateResult(eResult, "testClipPolyPolygon");
}

void GraphicsRenderTests::testClipB2DPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testClipB2DPolyPolygon");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupClipB2DPolyPolygon();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    updateResult(eResult, "testClipB2DPolyPolygon");
}

void GraphicsRenderTests::testDrawOutDev()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDrawOutDev");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupDrawOutDev();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDev(aBitmap);
    updateResult(eResult, "testDrawOutDev");
}

void GraphicsRenderTests::testDashedLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testDashedLine");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupDashedLine();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkDashedLine(aBitmap);
    updateResult(eResult, "testDashedLine");
}

void GraphicsRenderTests::testLinearGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLinearGradient");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLinearGradient();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradient(aBitmap);
    updateResult(eResult, "testLinearGradient");
}

void GraphicsRenderTests::testLinearGradientAngled()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLinearGradientAngled");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLinearGradientAngled();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientAngled(aBitmap);
    updateResult(eResult, "testLinearGradientAngled");
}

void GraphicsRenderTests::testLinearGradientBorder()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLinearGradientBorder");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLinearGradientBorder();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientBorder(aBitmap);
    updateResult(eResult, "testLinearGradientBorder");
}

void GraphicsRenderTests::testLinearGradientIntensity()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLinearGradientIntensity");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLinearGradientIntensity();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientIntensity(aBitmap);
    updateResult(eResult, "testLinearGradientIntensity");
}

void GraphicsRenderTests::testLinearGradientSteps()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLinearGradientSteps");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLinearGradientSteps();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientSteps(aBitmap);
    updateResult(eResult, "testLinearGradientSteps");
}

void GraphicsRenderTests::testAxialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testAxialGradient");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupAxialGradient();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkAxialGradient(aBitmap);
    updateResult(eResult, "testAxialGradient");
}

void GraphicsRenderTests::testRadialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testRadialGradient");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRadialGradient();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradient(aBitmap);
    updateResult(eResult, "testRadialGradient");
}

void GraphicsRenderTests::testRadialGradientOfs()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testRadialGradientOfs");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupRadialGradientOfs();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradientOfs(aBitmap);
    updateResult(eResult, "testRadialGradientOfs");
}

void GraphicsRenderTests::testLineJoinBevel()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineJoinBevel");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLineJoinBevel();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinBevel(aBitmap);
    updateResult(eResult, "testLineJoinBevel");
}

void GraphicsRenderTests::testLineJoinRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineJoinRound");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLineJoinRound();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinRound(aBitmap);
    updateResult(eResult, "testLineJoinRound");
}

void GraphicsRenderTests::testLineJoinMiter()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineJoinMiter");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLineJoinMiter();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinMiter(aBitmap);
    updateResult(eResult, "testLineJoinMiter");
}

void GraphicsRenderTests::testLineJoinNone()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineJoinNone");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLineJoinNone();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinNone(aBitmap);
    updateResult(eResult, "testLineJoinNone");
}

void GraphicsRenderTests::testLineCapRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineCapRound");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLineCapRound();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapRound(aBitmap);
    updateResult(eResult, "testLineCapRound");
}

void GraphicsRenderTests::testLineCapSquare()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineCapSquare");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLineCapSquare();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapSquare(aBitmap);
    updateResult(eResult, "testLineCapSquare");
}

void GraphicsRenderTests::testLineCapButt()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    if (!SHOULD_ASSERT)
    {
        m_aSkipped.push_back("testLineCapButt");
        return;
    }
    Bitmap aBitmap = aOutDevTest.setupLineCapButt();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapButt(aBitmap);
    updateResult(eResult, "testLineCapButt");
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

void GraphicsRenderTests::run()
{
    runALLTests();
    //Storing the test's results in the main user installation directory.
    OUString aUserInstallPath;
    ::utl::Bootstrap::locateUserInstallation(aUserInstallPath);
    SvFileStream logFile(aUserInstallPath + "/user/GraphicsRenderTests.log",
                         StreamMode::WRITE | StreamMode::TRUNC);
    ostringstream ofilestream;
    ofilestream << "Current Graphics Backend:" << m_aCurGraphicsBackend << "\n"
                << "Passed tests : " << to_string(m_aPassed.size()) << "\n"
                << "Quirky tests : " << to_string(m_aQuirky.size()) << "\n"
                << "Failed tests : " << to_string(m_aFailed.size()) << "\n"
                << "Skipped tests : " << to_string(m_aSkipped.size()) << "\n";
    logFile.WriteCharPtr(ofilestream.str().c_str());
    /*
    if (SkiaHelper::isVCLSkiaEnabled())
    {
        //TODO: What if both modes show the same results?
        if (m_aPassed.size() <= (m_aQuirky.size() + m_aFailed.size()))
        {
            shared_ptr<comphelper::ConfigurationChanges> batch(
                comphelper::ConfigurationChanges::create());
            //Falling to Raster mode if results were more erroneous & quirky.
            officecfg::Office::Common::VCL::ForceSkiaRaster::set(true, batch);
            batch->commit();
            //Restarting in order for the changes to take place.
            auto xRestartManager
                = css::task::OfficeRestartManager::get(comphelper::getProcessComponentContext());
            xRestartManager->requestRestart(css::uno::Reference<css::task::XInteractionHandler>());
        }
    } */
}
