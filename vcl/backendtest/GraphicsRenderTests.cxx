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

#include <svdata.hxx>
#include <salinst.hxx>

#include <unordered_map>

#define SHOULD_ASSERT                                                                              \
    (aOutDevTest.getRenderBackendName() != "qt5" && aOutDevTest.getRenderBackendName() != "qt5svp" \
     && aOutDevTest.getRenderBackendName() != "gtk3svp"                                            \
     && aOutDevTest.getRenderBackendName() != "aqua"                                               \
     && aOutDevTest.getRenderBackendName() != "gen"                                                \
     && aOutDevTest.getRenderBackendName() != "genpsp"                                             \
     && aOutDevTest.getRenderBackendName() != "win")

namespace
{
void exportBitmapExToImage(OUString const& rImageName, const BitmapEx& rBitmapEx)
{
    BitmapEx aBitmapEx(rBitmapEx);
    aBitmapEx.Scale(Size(500, 500), BmpScaleFlag::Fast);
    SvFileStream aStream(rImageName, StreamMode::WRITE | StreamMode::TRUNC);
    GraphicFilter::GetGraphicFilter().compressAsPNG(aBitmapEx, aStream);
}
bool is32bppSupported() { return ImplGetSVData()->mpDefInst->supportsBitmap32(); }
}

OUString GraphicsRenderTests::returnTestStatus(vcl::test::TestResult const result)
{
    switch (result)
    {
        case vcl::test::TestResult::Passed:
            return "PASSED";
        case vcl::test::TestResult::PassedWithQuirks:
            return "QUIRKY";
        case vcl::test::TestResult::Failed:
            return "FAILED";
    }
    return "SKIPPED";
}

GraphicsRenderTests::GraphicsRenderTests()
    : m_aStoreResultantBitmap(false)
    , m_xExecutionFlag(true)
{
    m_aTestVector = std::vector<std::function<void()>>{
        [this]() { testDrawRectWithRectangle(); },
        [this]() { testDrawRectWithPixel(); },
        [this]() { testDrawRectWithLine(); },
        [this]() { testDrawRectWithPolygon(); },
        [this]() { testDrawRectWithPolyLine(); },
        [this]() { testDrawRectWithPolyLineB2D(); },
        [this]() { testDrawRectWithPolyPolygon(); },
        [this]() { testDrawRectWithPolyPolygonB2D(); },
        [this]() { testDrawRectAAWithRectangle(); },
        [this]() { testDrawRectAAWithPixel(); },
        [this]() { testDrawRectAAWithLine(); },
        [this]() { testDrawRectAAWithPolygon(); },
        [this]() { testDrawRectAAWithPolyLine(); },
        [this]() { testDrawRectAAWithPolyLineB2D(); },
        [this]() { testDrawRectAAWithPolyPolygon(); },
        [this]() { testDrawRectAAWithPolyPolygonB2D(); },
        [this]() { testDrawFilledRectWithRectangle(); },
        [this]() { testDrawFilledRectWithRectangleWithAA(); },
        [this]() { testDrawFilledRectWithPolygon(); },
        [this]() { testDrawFilledRectWithPolygonWithAA(); },
        [this]() { testDrawFilledRectWithPolyPolygon(); },
        [this]() { testDrawFilledRectWithPolyPolygonWithAA(); },
        [this]() { testDrawFilledRectWithPolyPolygon2D(); },
        [this]() { testDrawFilledRectWithPolyPolygon2DWithAA(); },
        [this]() { testDrawDiamondWithPolygon(); },
        [this]() { testDrawDiamondWithLine(); },
        [this]() { testDrawDiamondWithPolyline(); },
        [this]() { testDrawDiamondWithPolylineB2D(); },
        [this]() { testDrawInvertWithRectangle(); },
        [this]() { testDrawInvertN50WithRectangle(); },
        [this]() { testDrawInvertTrackFrameWithRectangle(); },
        [this]() { testDrawBezierWithPolylineB2D(); },
        [this]() { testDrawBezierAAWithPolylineB2D(); },
        [this]() { testDrawBitmap24bpp(); },
        [this]() { testDrawTransformedBitmap24bpp(); },
        [this]() { testDrawBitmapExWithAlpha24bpp(); },
        [this]() { testDrawMask24bpp(); },
        [this]() { testDrawBlend24bpp(); },
        [this]() { testDrawXor(); },
        [this]() { testClipRectangle(); },
        [this]() { testClipPolygon(); },
        [this]() { testClipPolyPolygon(); },
        [this]() { testClipB2DPolyPolygon(); },
        [this]() { testDrawOutDev(); },
        [this]() { testDashedLine(); },
        [this]() { testLinearGradient(); },
        [this]() { testLinearGradientAngled(); },
        [this]() { testLinearGradientBorder(); },
        [this]() { testLinearGradientIntensity(); },
        [this]() { testLinearGradientSteps(); },
        [this]() { testAxialGradient(); },
        [this]() { testRadialGradient(); },
        [this]() { testRadialGradientOfs(); },
        [this]() { testLineJoinBevel(); },
        [this]() { testLineJoinRound(); },
        [this]() { testLineJoinMiter(); },
        [this]() { testLineJoinNone(); },
        [this]() { testLineCapRound(); },
        [this]() { testLineCapSquare(); },
        [this]() { testLineCapButt(); },
        [this]() { testDrawDropShapeWithPolyline(); },
        [this]() { testDrawDropShapeAAWithPolyline(); },
        [this]() { testDrawDropShapeWithPolygon(); },
        [this]() { testDrawDropShapeAAWithPolygon(); },
        [this]() { testHalfEllipseWithPolyLine(); },
        [this]() { testHalfEllipseAAWithPolyLine(); },
        [this]() { testHalfEllipseWithPolyLineB2D(); },
        [this]() { testHalfEllipseAAWithPolyLineB2D(); },
        [this]() { testHalfEllipseWithPolygon(); },
        [this]() { testHalfEllipseAAWithPolygon(); },
        [this]() { testClosedBezierWithPolyline(); },
        [this]() { testClosedBezierWithPolygon(); },
        [this]() { testFilledAsymmetricalDropShape(); },
        [this]() { testTextDrawing(); },
        [this]() { testEvenOddRuleInIntersectingRectsWithPolyPolygon(); },
        [this]() { testEvenOddRuleInIntersectingRectsWithPolyPolygonB2D(); },
        [this]() { testDrawBitmap8bppGreyScale(); },
        [this]() { testDrawTransformedBitmap8bppGreyScale(); },
        [this]() { testDrawBitmap32bpp(); },
        [this]() { testDrawTransformedBitmap32bpp(); },
        [this]() { testDrawBitmapExWithAlpha32bpp(); },
        [this]() { testDrawMask32bpp(); },
        [this]() { testDrawBlend32bpp(); },
        [this]() { testDrawRectangleOnSize1028WithRect(); },
        [this]() { testDrawRectangleOnSize4096WithRect(); },
        [this]() { testDrawRectangleOnSize1028WithPixel(); },
        [this]() { testDrawRectangleOnSize4096WithPixel(); },
        [this]() { testDrawRectangleOnSize1028WithLine(); },
        [this]() { testDrawRectangleOnSize4096WithLine(); },
        [this]() { testDrawRectangleOnSize1028WithPolyLine(); },
        [this]() { testDrawRectangleOnSize4096WithPolyLine(); },
        [this]() { testDrawRectangleOnSize1028WithPolygon(); },
        [this]() { testDrawRectangleOnSize4096WithPolygon(); },
        [this]() { testDrawRectangleOnSize1028WithPolyLineB2D(); },
        [this]() { testDrawRectangleOnSize4096WithPolyLineB2D(); },
        [this]() { testDrawRectangleOnSize1028WithPolyPolygon(); },
        [this]() { testDrawRectangleOnSize4096WithPolyPolygon(); },
        [this]() { testDrawRectangleOnSize1028WithPolyPolygonB2D(); },
        [this]() { testDrawRectangleOnSize4096WithPolygonPolygonB2D(); },
        [this]() { testDrawOpenPolygonWithPolyLine(); },
        [this]() { testDrawOpenPolygonWithPolyLineB2D(); },
        [this]() { testDrawOpenPolygonWithPolygon(); },
        [this]() { testDrawOpenPolygonWithPolyPolygon(); },
        [this]() { testDrawOpenPolygonWithPolyPolygonB2D(); },
        [this]() { testDrawOpenBezierWithPolyLine(); },
        [this]() { testDrawOpenBezierWithPolyLineB2D(); }
    };
}

void GraphicsRenderTests::testDrawRectWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    m_aCurGraphicsBackend = aOutDevTest.getRenderBackendName();
    OUString aTestName = "testDrawRectWithRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString aTestName = "testDrawRectWithPixel";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString aTestName = "testDrawRectWithLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString aTestName = "testDrawRectWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString aTestName = "testDrawRectWithPolyLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString aTestName = "testDrawRectWithPolyLineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString aTestName = "testDrawRectWithPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString aTestName = "testDrawRectWithPolyPolygonB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString aTestName = "testDrawRectAAWithRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString aTestName = "testDrawRectAAWithPixel";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString aTestName = "testDrawRectAAWithLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString aTestName = "testDrawRectAAWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString aTestName = "testDrawRectAAWithPolyLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString aTestName = "testDrawRectAAWithPolyLineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString aTestName = "testDrawRectAAWithPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString aTestName = "testDrawRectAAWithPolyPolygonB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawFilledRectWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    OUString aTestName = "testDrawFilledRectWithRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawFilledRectWithRectangleWithAA()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(true);
    OUString aTestName = "testDrawFilledRectWithRectangleWithAA";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawFilledRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    OUString aTestName = "testDrawFilledRectWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawFilledRectWithPolygonWithAA()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(true);
    OUString aTestName = "testDrawFilledRectWithPolygonWithAA";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    OUString aTestName = "testDrawFilledRectWithPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygonWithAA()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(true);
    OUString aTestName = "testDrawFilledRectWithPolyPolygonWithAA";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    OUString aTestName = "testDrawFilledRectWithPolyPolygon2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
    aTestName += "WithAA";
    aBitmap = aOutDevTest.setupFilledRectangle(true);
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon2DWithAA()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(true);
    OUString aTestName = "testDrawFilledRectWithPolyPolygon2DWithAA";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDiamondWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    OUString aTestName = "testDrawDiamondWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDiamondWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    OUString aTestName = "testDrawDiamondWithLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDiamondWithPolyline()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    OUString aTestName = "testDrawDiamondWithPolyline";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDiamondWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    OUString aTestName = "testDrawDiamondWithPolylineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawInvertWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_NONE();
    OUString aTestName = "testDrawInvertWithRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertRectangle(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawInvertN50WithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_N50();
    OUString aTestName = "testDrawInvertN50WithRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertN50Rectangle(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawInvertTrackFrameWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_TrackFrame();
    OUString aTestName = "testDrawInvertTrackFrameWithRectangle";
    if (!(SHOULD_ASSERT && aOutDevTest.getRenderBackendName() != "svp"))
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertTrackFrameRectangle(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDropShapeWithPolyline()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDropShape();
    OUString aTestName = "testDrawDropShapeWithPolyline";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDropShapeAAWithPolyline()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAADropShape();
    OUString aTestName = "testDrawDropShapeAAWithPolyline";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap, true);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBezierWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupBezier();
    OUString aTestName = "testDrawBezierWithPolylineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBezierAAWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAABezier();
    OUString aTestName = "testDrawBezierAAWithPolylineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDropShapeWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDropShape();
    OUString aTestName = "testDrawDropShapeWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDropShapeAAWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAADropShape();
    OUString aTestName = "testDrawDropShapeAAWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap, true);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBitmap24bpp()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N24_BPP);
    OUString aTestName = "testDrawBitmap24bpp";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawTransformedBitmap24bpp()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N24_BPP);
    OUString aTestName = "testDrawTransformedBitmap24bpp";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testComplexDrawTransformedBitmap24bpp()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupComplexDrawTransformedBitmap(vcl::PixelFormat::N24_BPP);
    OUString aTestName = "testComplexDrawTransformedBitmap24bpp";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkComplexTransformedBitmap(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBitmapExWithAlpha24bpp()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha(vcl::PixelFormat::N24_BPP);
    OUString aTestName = "testDrawBitmapExWithAlpha24bpp";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkBitmapExWithAlpha(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawMask24bpp()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawMask(vcl::PixelFormat::N24_BPP);
    OUString aTestName = "testDrawMask24bpp";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkMask(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBlend24bpp()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend(vcl::PixelFormat::N24_BPP);
    OUString aTestName = "testDrawBlend24bpp";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkBlend(aBitmapEx);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmapEx.GetBitmap() : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawXor()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupXOR();
    OUString aTestName = "testDrawXor";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestAnotherOutDev::checkXOR(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testClipRectangle()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipRectangle();
    OUString aTestName = "testClipRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testClipPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolygon();
    OUString aTestName = "testClipPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testClipPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolyPolygon();
    OUString aTestName = "testClipPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testClipB2DPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipB2DPolyPolygon();
    OUString aTestName = "testClipB2DPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawOutDev()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDev();
    OUString aTestName = "testDrawOutDev";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDev(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawOutDevScaledClipped()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDevScaledClipped();
    OUString aTestName = "testDrawOutDevScaledClipped";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDevScaledClipped(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawOutDevSelf()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDevSelf();
    OUString aTestName = "testDrawOutDevSelf";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDevSelf(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDashedLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDashedLine();
    OUString aTestName = "testDashedLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkDashedLine(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLinearGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradient();
    OUString aTestName = "testLinearGradient";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradient(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLinearGradientAngled()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientAngled();
    OUString aTestName = "testLinearGradientAngled";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientAngled(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLinearGradientBorder()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientBorder();
    OUString aTestName = "testLinearGradientBorder";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientBorder(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLinearGradientIntensity()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientIntensity();
    OUString aTestName = "testLinearGradientIntensity";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientIntensity(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLinearGradientSteps()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientSteps();
    OUString aTestName = "testLinearGradientSteps";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientSteps(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testAxialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAxialGradient();
    OUString aTestName = "testAxialGradient";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkAxialGradient(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testRadialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradient();
    OUString aTestName = "testRadialGradient";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradient(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testRadialGradientOfs()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradientOfs();
    OUString aTestName = "testRadialGradientOfs";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradientOfs(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
}

void GraphicsRenderTests::testLineJoinBevel()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinBevel();
    OUString aTestName = "testLineJoinBevel";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinBevel(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineJoinRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinRound();
    OUString aTestName = "testLineJoinRound";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinRound(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineJoinMiter()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinMiter();
    OUString aTestName = "testLineJoinMiter";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinMiter(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineJoinNone()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinNone();
    OUString aTestName = "testLineJoinNone";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinNone(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineCapRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapRound();
    OUString aTestName = "testLineCapRound";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapRound(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineCapSquare()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapSquare();
    OUString aTestName = "testLineCapSquare";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapSquare(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineCapButt()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapButt();
    OUString aTestName = "testLineCapButt";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapButt(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testHalfEllipseWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
    OUString aTestName = "testHalfEllipseWithPolyLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkHalfEllipse(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testHalfEllipseAAWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
    OUString aTestName = "testHalfEllipseAAWithPolyLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestLine::checkHalfEllipse(aBitmap, true);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testHalfEllipseWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
    OUString aTestName = "testHalfEllipseWithPolyLineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkHalfEllipse(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testHalfEllipseAAWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
    OUString aTestName = "testHalfEllipseAAWithPolyLineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestLine::checkHalfEllipse(aBitmap, true);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testHalfEllipseWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
    OUString aTestName = "testHalfEllipseWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkHalfEllipse(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testClosedBezierWithPolyline()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClosedBezier();
    OUString aTestName = "testClosedBezierWithPolyline";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkClosedBezier(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testClosedBezierWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClosedBezier();
    OUString aTestName = "testClosedBezierWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkClosedBezier(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testHalfEllipseAAWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
    OUString aTestName = "testHalfEllipseAAWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestLine::checkHalfEllipse(aBitmap, true);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testFilledAsymmetricalDropShape()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledAsymmetricalDropShape();
    OUString aTestName = "testFilledAsymmetricalDropShape";
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestLine::checkFilledAsymmetricalDropShape(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testTextDrawing()
{
    vcl::test::OutputDeviceTestText aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupTextBitmap();
    OUString aTestName = "testTextDrawing";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkTextLocation(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testEvenOddRuleInIntersectingRectsWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupIntersectingRectangles();
    OUString aTestName = "testEvenOddRuleInIntersectingRectsWithPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestLine::checkEvenOddRuleInIntersectingRecs(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testEvenOddRuleInIntersectingRectsWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupIntersectingRectangles();
    OUString aTestName = "testEvenOddRuleInIntersectingRectsWithPolyPolygonB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestLine::checkEvenOddRuleInIntersectingRecs(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBitmap8bppGreyScale()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N8_BPP, true);
    OUString aTestName = "testDrawBitmap8bppGreyScale";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap8bppGreyScale(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawTransformedBitmap8bppGreyScale()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N8_BPP, true);
    OUString aTestName = "testDrawTransformedBitmap8bppGreyScale";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap8bppGreyScale(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBitmap32bpp()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N32_BPP);
    OUString aTestName = "testDrawBitmap32bpp";
    if (!SHOULD_ASSERT || !is32bppSupported())
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawTransformedBitmap32bpp()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N32_BPP);
    OUString aTestName = "testDrawTransformedBitmap32bpp";
    if (!SHOULD_ASSERT || !is32bppSupported())
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBitmapExWithAlpha32bpp()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha(vcl::PixelFormat::N32_BPP);
    OUString aTestName = "testDrawBitmapExWithAlpha32bpp";
    if (!SHOULD_ASSERT || !is32bppSupported())
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkBitmapExWithAlpha(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawMask32bpp()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawMask(vcl::PixelFormat::N32_BPP);
    OUString aTestName = "testDrawMask32bpp";
    if (!SHOULD_ASSERT || !is32bppSupported())
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkMask(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBlend32bpp()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend(vcl::PixelFormat::N32_BPP);
    OUString aTestName = "testDrawBlend32bpp";
    if (!SHOULD_ASSERT || !is32bppSupported())
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkBlend(aBitmapEx);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmapEx.GetBitmap() : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize1028WithRect()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    m_aCurGraphicsBackend = aOutDevTest.getRenderBackendName();
    OUString aTestName = "testDrawRectangleOnSize1028WithRect";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize4096WithRect()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    m_aCurGraphicsBackend = aOutDevTest.getRenderBackendName();
    OUString aTestName = "testDrawRectangleOnSize4096WithRect";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize1028WithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    OUString aTestName = "testDrawRectangleOnSize1028WithPixel";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize4096WithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    OUString aTestName = "testDrawRectangleOnSize4096WithPixel";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize1028WithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    OUString aTestName = "testDrawRectangleOnSize1028WithLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize4096WithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    OUString aTestName = "testDrawRectangleOnSize4096WithLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize1028WithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    OUString aTestName = "testDrawRectangleOnSize1028WithPolyLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize4096WithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    OUString aTestName = "testDrawRectangleOnSize4096WithPolyLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize1028WithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    OUString aTestName = "testDrawRectangleOnSize1028WithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize4096WithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    OUString aTestName = "testDrawRectangleOnSize4096WithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize1028WithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    OUString aTestName = "testDrawRectangleOnSize1028WithPolyLineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize4096WithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    OUString aTestName = "testDrawRectangleOnSize4096WithPolyLineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize1028WithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    OUString aTestName = "testDrawRectangleOnSize1028WithPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize4096WithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    OUString aTestName = "testDrawRectangleOnSize4096WithPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize1028WithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    OUString aTestName = "testDrawRectangleOnSize1028WithPolyPolygonB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectangleOnSize4096WithPolygonPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    OUString aTestName = "testDrawRectangleOnSize4096WithPolygonPolygonB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangles(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawOpenPolygonWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
    OUString aTestName = "testDrawOpenPolygonWithPolyLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkOpenPolygon(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawOpenPolygonWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
    OUString aTestName = "testDrawOpenPolygonWithPolyLineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkOpenPolygon(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawOpenPolygonWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
    OUString aTestName = "testDrawOpenPolygonWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkOpenPolygon(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawOpenPolygonWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
    OUString aTestName = "testDrawOpenPolygonWithPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkOpenPolygon(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawOpenPolygonWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
    OUString aTestName = "testDrawOpenPolygonWithPolyPolygonB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkOpenPolygon(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawOpenBezierWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenBezier();
    OUString aTestName = "testDrawOpenBezierWithPolyLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkOpenBezier(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawOpenBezierWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenBezier();
    OUString aTestName = "testDrawOpenBezierWithPolyLineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkOpenBezier(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + aTestName + ".png", aBitmapEx);
    }
}

int GraphicsRenderTests::getNumberOfTests() { return int(m_aTestVector.size()); }

void GraphicsRenderTests::runALLTests()
{
    for (std::function<void()>& rTestFunction : m_aTestVector)
    {
        if (!m_xExecutionFlag)
            return;

        rTestFunction();
    }
}

void GraphicsRenderTests::appendTestResult(OUString aTestName, OUString aTestStatus,
                                           Bitmap aTestBitmap)
{
    m_aTestResult.push_back(VclTestResult(aTestName, aTestStatus, aTestBitmap));
}

std::vector<VclTestResult>& GraphicsRenderTests::getTestResults() { return m_aTestResult; }

OUString GraphicsRenderTests::getResultString()
{
    std::vector<int> testResults(4);
    for (const VclTestResult& test : m_aTestResult)
    {
        if (test.getStatus() == "PASSED")
        {
            testResults[0]++;
        }
        else if (test.getStatus() == "QUIRKY")
        {
            testResults[1]++;
        }
        else if (test.getStatus() == "FAILED")
        {
            testResults[2]++;
        }
        else
        {
            testResults[3]++;
        }
    }
    OUString resultString = "Graphics Backend used: " + m_aCurGraphicsBackend
                            + "\nPassed Tests : " + OUString::number(testResults[0])
                            + "\nQuirky Tests : " + OUString::number(testResults[1])
                            + "\nFailed Tests : " + OUString::number(testResults[2])
                            + "\nSkipped Tests : " + OUString::number(testResults[3]) + "\n";
    return resultString;
}

void GraphicsRenderTests::run(bool storeResultBitmap)
{
    m_aStoreResultantBitmap = storeResultBitmap;
    ::utl::Bootstrap::locateUserInstallation(m_aUserInstallPath);
    if (storeResultBitmap)
    {
        m_aUserInstallPath += "/user/GraphicTestResults/";
    }
    else
    {
        m_aUserInstallPath += "/user/";
    }
    runALLTests();
    //Storing the test's results in the main user installation directory.
    SvFileStream logFile(m_aUserInstallPath + "GraphicsRenderTests.log",
                         StreamMode::WRITE | StreamMode::TRUNC);
    std::unordered_map<OUString, std::vector<OUString>> aTests;
    for (VclTestResult& tests : m_aTestResult)
    {
        aTests[tests.getStatus()].push_back(tests.getName());
    }
    OUString writeResult = getResultString() + "\n---Name of the tests that failed---\n";
    if (static_cast<int>(aTests["FAILED"].size()) > 0)
    {
        for (const class OUString& tests : aTests["FAILED"])
        {
            writeResult += tests + "\n";
        }
    }
    else
    {
        writeResult += "No test has been failed.\n";
    }
    writeResult += "\n---Name of the tests that were Quirky---\n";
    if (static_cast<int>(aTests["QUIRKY"].size()) > 0)
    {
        for (const class OUString& tests : aTests["QUIRKY"])
        {
            writeResult += tests + "\n";
        }
    }
    else
    {
        writeResult += "No test was Quirky.\n";
    }
    writeResult += "\n---Name of the tests that were Skipped---\n";
    if (static_cast<int>(aTests["SKIPPED"].size()) > 0)
    {
        for (const class OUString& tests : aTests["SKIPPED"])
        {
            writeResult += tests + "\n";
        }
    }
    else
    {
        writeResult += "No test was Skipped.";
    }
    logFile.WriteOString(OUStringToOString(writeResult, RTL_TEXTENCODING_UTF8));
}
