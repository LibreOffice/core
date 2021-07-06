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
    OUString atestName = "testDrawRectWithRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString atestName = "testDrawRectWithPixel";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString atestName = "testDrawRectWithLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString atestName = "testDrawRectWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString atestName = "testDrawRectWithPolyLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString atestName = "testDrawRectWithPolyLineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString atestName = "testDrawRectWithPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    OUString atestName = "testDrawRectWithPolyPolygonB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString atestName = "testDrawRectAAWithRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString atestName = "testDrawRectAAWithPixel";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString atestName = "testDrawRectAAWithLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString atestName = "testDrawRectAAWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString atestName = "testDrawRectAAWithPolyLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString atestName = "testDrawRectAAWithPolyLineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString atestName = "testDrawRectAAWithPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    OUString atestName = "testDrawRectAAWithPolyPolygonB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawFilledRectWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    OUString atestName = "testDrawFilledRectWithRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
    atestName += "WithAA";
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawFilledRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    OUString atestName = "testDrawFilledRectWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
    atestName += "WithAA";
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    OUString atestName = "testDrawFilledRectWithPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
    atestName += "WithAA";
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    OUString atestName = "testDrawFilledRectWithPolyPolygon2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
    atestName += "WithAA";
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDiamondWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    OUString atestName = "testDrawDiamondWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDiamondWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    OUString atestName = "testDrawDiamondWithLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDiamondWithPolyline()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    OUString atestName = "testDrawDiamondWithPolyline";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDiamondWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    OUString atestName = "testDrawDiamondWithPolylineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawInvertWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_NONE();
    OUString atestName = "testDrawInvertWithRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertRectangle(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawInvertN50WithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_N50();
    OUString atestName = "testDrawInvertN50WithRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertN50Rectangle(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawInvertTrackFrameWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_TrackFrame();
    OUString atestName = "testDrawInvertTrackFrameWithRectangle";
    if (!(SHOULD_ASSERT && aOutDevTest.getRenderBackendName() != "svp"))
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertTrackFrameRectangle(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDropShapeWithPolyline()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDropShape();
    OUString atestName = "testDrawDropShapeWithPolyline";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDropShapeAAWithPolyline()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAADropShape();
    OUString atestName = "testDrawDropShapeAAWithPolyline";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap, true);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBezierWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupBezier();
    OUString atestName = "testDrawBezierWithPolylineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBezierAAWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAABezier();
    OUString atestName = "testDrawBezierAAWithPolylineB2D";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDropShapeWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDropShape();
    OUString atestName = "testDrawDropShapeWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawDropShapeAAWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAADropShape();
    OUString atestName = "testDrawDropShapeAAWithPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkDropShape(aBitmap, true);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap();
    OUString atestName = "testDrawBitmap";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawTransformedBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap();
    OUString atestName = "testDrawTransformedBitmap";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBitmapExWithAlpha()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha();
    OUString atestName = "testDrawBitmapExWithAlpha";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkBitmapExWithAlpha(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawMask()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawMask();
    OUString atestName = "testDrawMask";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkMask(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawBlend()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend();
    OUString atestName = "testDrawBlend";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkBlend(aBitmapEx);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmapEx.GetBitmap() : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawXor()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupXOR();
    OUString atestName = "testDrawXor";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestAnotherOutDev::checkXOR(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testClipRectangle()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipRectangle();
    OUString atestName = "testClipRectangle";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testClipPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolygon();
    OUString atestName = "testClipPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testClipPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolyPolygon();
    OUString atestName = "testClipPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testClipB2DPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipB2DPolyPolygon();
    OUString atestName = "testClipB2DPolyPolygon";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDrawOutDev()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDev();
    OUString atestName = "testDrawOutDev";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDev(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testDashedLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDashedLine();
    OUString atestName = "testDashedLine";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkDashedLine(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLinearGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradient();
    OUString atestName = "testLinearGradient";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradient(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLinearGradientAngled()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientAngled();
    OUString atestName = "testLinearGradientAngled";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientAngled(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLinearGradientBorder()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientBorder();
    OUString atestName = "testLinearGradientBorder";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientBorder(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLinearGradientIntensity()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientIntensity();
    OUString atestName = "testLinearGradientIntensity";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientIntensity(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLinearGradientSteps()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientSteps();
    OUString atestName = "testLinearGradientSteps";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientSteps(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testAxialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAxialGradient();
    OUString atestName = "testAxialGradient";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkAxialGradient(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testRadialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradient();
    OUString atestName = "testRadialGradient";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradient(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testRadialGradientOfs()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradientOfs();
    OUString atestName = "testRadialGradientOfs";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradientOfs(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
}

void GraphicsRenderTests::testLineJoinBevel()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinBevel();
    OUString atestName = "testLineJoinBevel";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinBevel(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineJoinRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinRound();
    OUString atestName = "testLineJoinRound";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinRound(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineJoinMiter()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinMiter();
    OUString atestName = "testLineJoinMiter";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinMiter(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineJoinNone()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinNone();
    OUString atestName = "testLineJoinNone";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinNone(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineCapRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapRound();
    OUString atestName = "testLineCapRound";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapRound(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineCapSquare()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapSquare();
    OUString atestName = "testLineCapSquare";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapSquare(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
    }
}

void GraphicsRenderTests::testLineCapButt()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapButt();
    OUString atestName = "testLineCapButt";
    if (!SHOULD_ASSERT)
    {
        appendTestResult(atestName, "SKIPPED");
        return;
    }
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapButt(aBitmap);
    appendTestResult(atestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
    if (m_aStoreResultantBitmap)
    {
        BitmapEx aBitmapEx(aBitmap);
        exportBitmapExToImage(m_aUserInstallPath + atestName + ".png", aBitmapEx);
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
    testHalfEllipsePolyLine();
    testHalfEllipsePolygon();
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
