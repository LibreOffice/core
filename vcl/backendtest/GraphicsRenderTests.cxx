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
void exportBitmapExToImage(OUString const& rImageName, BitmapEx& rBitmapEx)
{
    BitmapEx aBitmapEx(rBitmapEx);
    aBitmapEx.Scale(Size(500, 500), BmpScaleFlag::Fast);
    SvFileStream aStream(rImageName, StreamMode::WRITE | StreamMode::TRUNC);
    GraphicFilter::GetGraphicFilter().compressAsPNG(aBitmapEx, aStream);
}
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
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
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
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
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
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
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
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
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
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
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
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
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
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
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
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
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
    aTestName += "WithAA";
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
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
    aTestName += "WithAA";
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
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
    aTestName += "WithAA";
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
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
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
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

void GraphicsRenderTests::testDrawBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap();
    OUString aTestName = "testDrawBitmap";
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

void GraphicsRenderTests::testDrawTransformedBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap();
    OUString aTestName = "testDrawTransformedBitmap";
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

void GraphicsRenderTests::testDrawBitmapExWithAlpha()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha();
    OUString aTestName = "testDrawBitmapExWithAlpha";
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

void GraphicsRenderTests::testDrawMask()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawMask();
    OUString aTestName = "testDrawMask";
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

void GraphicsRenderTests::testDrawBlend()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend();
    OUString aTestName = "testDrawBlend";
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
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, "SKIPPED");
        return;
    }
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
    testDrawDropShapeWithPolyline();
    testDrawDropShapeAAWithPolyline();
    testDrawDropShapeWithPolygon();
    testDrawDropShapeAAWithPolygon();
    testHalfEllipseWithPolyLine();
    testHalfEllipseAAWithPolyLine();
    testHalfEllipseWithPolyLineB2D();
    testHalfEllipseAAWithPolyLineB2D();
    testHalfEllipseWithPolygon();
    testHalfEllipseAAWithPolygon();
    testClosedBezierWithPolyline();
    testClosedBezierWithPolygon();
    testFilledAsymmetricalDropShape();
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
    for (VclTestResult& test : m_aTestResult)
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
        aTests[tests.getStatus()].push_back(tests.getTestName());
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
