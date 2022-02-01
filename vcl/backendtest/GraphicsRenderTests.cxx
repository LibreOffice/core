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
#include <svl/svlresid.hxx>
#include <svl/svl.hrc>
#include <unotools/bootstrap.hxx>
#include <vcl/test/GraphicsRenderTests.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>

#include <svdata.hxx>
#include <salinst.hxx>
#include <strings.hrc>
#include <test/GraphicsRenderTests.hxx>

#include <unordered_map>

#define SHOULD_ASSERT                                                                              \
    (aOutDevTest.getRenderBackendName() != "qt5" && aOutDevTest.getRenderBackendName() != "qt5svp" \
     && aOutDevTest.getRenderBackendName() != "gtk3svp"                                            \
     && aOutDevTest.getRenderBackendName() != "aqua"                                               \
     && aOutDevTest.getRenderBackendName() != "gen"                                                \
     && aOutDevTest.getRenderBackendName() != "genpsp"                                             \
     && aOutDevTest.getRenderBackendName() != "win")

namespace vcl::test
{
static OUString activeGraphicsRenderTestName;
void setActiveGraphicsRenderTest(const OUString& name) { activeGraphicsRenderTestName = name; }
const OUString& activeGraphicsRenderTest() { return activeGraphicsRenderTestName; }
} // namespace vcl::test

OUString VclTestResult::getStatus(bool bLocalize)
{ // tdf#145919 localize for UI but not in the log file
    if (bLocalize)
    {
        if (m_aTestStatus == "PASSED")
        {
            return SvlResId(GRTSTR_PASSED);
        }
        else if (m_aTestStatus == "QUIRKY")
        {
            return SvlResId(GRTSTR_QUIRKY);
        }
        else if (m_aTestStatus == "FAILED")
        {
            return SvlResId(GRTSTR_FAILED);
        }
        else
        {
            return SvlResId(GRTSTR_SKIPPED);
        }
    }
    else
        return m_aTestStatus;
}

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

// Some tests need special handling in drawing code (for example, not smoothscaling
// when handling HiDPI bitmaps). Temporarily set up the test name to get such special
// handling as well.
class GraphicsTestZone
{
public:
    GraphicsTestZone(std::u16string_view name)
    {
        vcl::test::setActiveGraphicsRenderTest(OUString::Concat("GraphicsRenderTest__") + name);
    }
    ~GraphicsTestZone() { vcl::test::setActiveGraphicsRenderTest(""); }
};
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
    OUString aTestName = "testDrawRectWithRectangle";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    m_aCurGraphicsBackend = aOutDevTest.getRenderBackendName();
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
    OUString aTestName = "testDrawRectWithPixel";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
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
    OUString aTestName = "testDrawRectWithLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
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
    OUString aTestName = "testDrawRectWithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
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
    OUString aTestName = "testDrawRectWithPolyLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
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
    OUString aTestName = "testDrawRectWithPolyLineB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
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
    OUString aTestName = "testDrawRectWithPolyPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
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
    OUString aTestName = "testDrawRectWithPolyPolygonB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
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
    OUString aTestName = "testDrawRectAAWithRectangle";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
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
    OUString aTestName = "testDrawRectAAWithPixel";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
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
    OUString aTestName = "testDrawRectAAWithLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
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
    OUString aTestName = "testDrawRectAAWithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
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
    OUString aTestName = "testDrawRectAAWithPolyLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
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
    OUString aTestName = "testDrawRectAAWithPolyLineB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
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
    OUString aTestName = "testDrawRectAAWithPolyPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
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
    OUString aTestName = "testDrawRectAAWithPolyPolygonB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
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
    OUString aTestName = "testDrawFilledRectWithRectangle";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
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
    OUString aTestName = "testDrawFilledRectWithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
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
    OUString aTestName = "testDrawFilledRectWithPolyPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
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
    OUString aTestName = "testDrawFilledRectWithPolyPolygon2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
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
    OUString aTestName = "testDrawDiamondWithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
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
    OUString aTestName = "testDrawDiamondWithLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
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
    OUString aTestName = "testDrawDiamondWithPolyline";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
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
    OUString aTestName = "testDrawDiamondWithPolylineB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
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
    OUString aTestName = "testDrawInvertWithRectangle";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_NONE();
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
    OUString aTestName = "testDrawInvertN50WithRectangle";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_N50();
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
    OUString aTestName = "testDrawInvertTrackFrameWithRectangle";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_TrackFrame();
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
    OUString aTestName = "testDrawDropShapeWithPolyline";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDropShape();
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
    OUString aTestName = "testDrawDropShapeAAWithPolyline";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAADropShape();
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
    OUString aTestName = "testDrawBezierWithPolylineB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupBezier();
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
    OUString aTestName = "testDrawBezierAAWithPolylineB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAABezier();
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
    OUString aTestName = "testDrawDropShapeWithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDropShape();
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
    OUString aTestName = "testDrawDropShapeAAWithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAADropShape();
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
    OUString aTestName = "testDrawBitmap24bpp";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N24_BPP);
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
    OUString aTestName = "testDrawTransformedBitmap24bpp";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N24_BPP);
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
    OUString aTestName = "testComplexDrawTransformedBitmap24bpp";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupComplexDrawTransformedBitmap(vcl::PixelFormat::N24_BPP);
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
    OUString aTestName = "testDrawBitmapExWithAlpha24bpp";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha(vcl::PixelFormat::N24_BPP);
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
    OUString aTestName = "testDrawMask24bpp";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawMask(vcl::PixelFormat::N24_BPP);
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
    OUString aTestName = "testDrawBlend24bpp";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend(vcl::PixelFormat::N24_BPP);
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
    OUString aTestName = "testDrawXor";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupXOR();
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
    OUString aTestName = "testClipRectangle";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipRectangle();
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
    OUString aTestName = "testClipPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolygon();
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
    OUString aTestName = "testClipPolyPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolyPolygon();
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
    OUString aTestName = "testClipB2DPolyPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipB2DPolyPolygon();
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
    OUString aTestName = "testDrawOutDev";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDev();
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
    OUString aTestName = "testDrawOutDevScaledClipped";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDevScaledClipped();
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
    OUString aTestName = "testDrawOutDevSelf";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDevSelf();
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
    OUString aTestName = "testDashedLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDashedLine();
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
    OUString aTestName = "testLinearGradient";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradient();
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
    OUString aTestName = "testLinearGradientAngled";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientAngled();
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
    OUString aTestName = "testLinearGradientBorder";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientBorder();
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
    OUString aTestName = "testLinearGradientIntensity";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientIntensity();
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
    OUString aTestName = "testLinearGradientSteps";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientSteps();
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
    OUString aTestName = "testAxialGradient";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAxialGradient();
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
    OUString aTestName = "testRadialGradient";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradient();
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
    OUString aTestName = "testRadialGradientOfs";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradientOfs();
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
    OUString aTestName = "testLineJoinBevel";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinBevel();
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
    OUString aTestName = "testLineJoinRound";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinRound();
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
    OUString aTestName = "testLineJoinMiter";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinMiter();
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
    OUString aTestName = "testLineJoinNone";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinNone();
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
    OUString aTestName = "testLineCapRound";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapRound();
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
    OUString aTestName = "testLineCapSquare";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapSquare();
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
    OUString aTestName = "testLineCapButt";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapButt();
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
    OUString aTestName = "testHalfEllipseWithPolyLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
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
    OUString aTestName = "testHalfEllipseAAWithPolyLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
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
    OUString aTestName = "testHalfEllipseWithPolyLineB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
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
    OUString aTestName = "testHalfEllipseAAWithPolyLineB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
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
    OUString aTestName = "testHalfEllipseWithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
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
    OUString aTestName = "testClosedBezierWithPolyline";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClosedBezier();
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
    OUString aTestName = "testClosedBezierWithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClosedBezier();
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
    OUString aTestName = "testHalfEllipseAAWithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
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
    OUString aTestName = "testFilledAsymmetricalDropShape";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledAsymmetricalDropShape();
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
    OUString aTestName = "testTextDrawing";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestText aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupTextBitmap();
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
    OUString aTestName = "testEvenOddRuleInIntersectingRectsWithPolyPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupIntersectingRectangles();
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
    OUString aTestName = "testEvenOddRuleInIntersectingRectsWithPolyPolygonB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupIntersectingRectangles();
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
    OUString aTestName = "testDrawBitmap8bppGreyScale";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N8_BPP, true);
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
    OUString aTestName = "testDrawTransformedBitmap8bppGreyScale";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N8_BPP, true);
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
    OUString aTestName = "testDrawBitmap32bpp";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N32_BPP);
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
    OUString aTestName = "testDrawTransformedBitmap32bpp";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N32_BPP);
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
    OUString aTestName = "testDrawBitmapExWithAlpha32bpp";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha(vcl::PixelFormat::N32_BPP);
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
    OUString aTestName = "testDrawMask32bpp";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawMask(vcl::PixelFormat::N32_BPP);
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
    OUString aTestName = "testDrawBlend32bpp";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend(vcl::PixelFormat::N32_BPP);
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
    OUString aTestName = "testDrawRectangleOnSize1028WithRect";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    m_aCurGraphicsBackend = aOutDevTest.getRenderBackendName();
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
    OUString aTestName = "testDrawRectangleOnSize4096WithRect";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    m_aCurGraphicsBackend = aOutDevTest.getRenderBackendName();
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
    OUString aTestName = "testDrawRectangleOnSize1028WithPixel";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
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
    OUString aTestName = "testDrawRectangleOnSize4096WithPixel";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
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
    OUString aTestName = "testDrawRectangleOnSize1028WithLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
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
    OUString aTestName = "testDrawRectangleOnSize4096WithLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
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
    OUString aTestName = "testDrawRectangleOnSize1028WithPolyLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
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
    OUString aTestName = "testDrawRectangleOnSize4096WithPolyLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
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
    OUString aTestName = "testDrawRectangleOnSize1028WithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
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
    OUString aTestName = "testDrawRectangleOnSize4096WithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
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
    OUString aTestName = "testDrawRectangleOnSize1028WithPolyLineB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
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
    OUString aTestName = "testDrawRectangleOnSize4096WithPolyLineB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
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
    OUString aTestName = "testDrawRectangleOnSize1028WithPolyPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
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
    OUString aTestName = "testDrawRectangleOnSize4096WithPolyPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
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
    OUString aTestName = "testDrawRectangleOnSize1028WithPolyPolygonB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
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
    OUString aTestName = "testDrawRectangleOnSize4096WithPolygonPolygonB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
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
    OUString aTestName = "testDrawOpenPolygonWithPolyLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
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
    OUString aTestName = "testDrawOpenPolygonWithPolyLineB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
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
    OUString aTestName = "testDrawOpenPolygonWithPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
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
    OUString aTestName = "testDrawOpenPolygonWithPolyPolygon";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
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
    OUString aTestName = "testDrawOpenPolygonWithPolyPolygonB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
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
    OUString aTestName = "testDrawOpenBezierWithPolyLine";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenBezier();
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
    OUString aTestName = "testDrawOpenBezierWithPolyLineB2D";
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenBezier();
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
    testDrawBitmap24bpp();
    testDrawTransformedBitmap24bpp();
    testComplexDrawTransformedBitmap24bpp();
    testDrawBitmapExWithAlpha24bpp();
    testDrawMask24bpp();
    testDrawBlend24bpp();
    testDrawXor();
    testClipRectangle();
    testClipPolygon();
    testClipPolyPolygon();
    testClipB2DPolyPolygon();
    testDrawOutDev();
    testDrawOutDevScaledClipped();
    testDrawOutDevSelf();
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
    testTextDrawing();
    testEvenOddRuleInIntersectingRectsWithPolyPolygon();
    testEvenOddRuleInIntersectingRectsWithPolyPolygonB2D();
    testDrawBitmap8bppGreyScale();
    testDrawTransformedBitmap8bppGreyScale();
    testDrawBitmap32bpp();
    testDrawTransformedBitmap32bpp();
    testDrawBitmapExWithAlpha32bpp();
    testDrawMask32bpp();
    testDrawBlend32bpp();
    testDrawRectangleOnSize1028WithRect();
    testDrawRectangleOnSize4096WithRect();
    testDrawRectangleOnSize1028WithPixel();
    testDrawRectangleOnSize4096WithPixel();
    testDrawRectangleOnSize1028WithLine();
    testDrawRectangleOnSize4096WithLine();
    testDrawRectangleOnSize1028WithPolyLine();
    testDrawRectangleOnSize4096WithPolyLine();
    testDrawRectangleOnSize1028WithPolygon();
    testDrawRectangleOnSize4096WithPolygon();
    testDrawRectangleOnSize1028WithPolyLineB2D();
    testDrawRectangleOnSize4096WithPolyLineB2D();
    testDrawRectangleOnSize1028WithPolyPolygon();
    testDrawRectangleOnSize4096WithPolyPolygon();
    testDrawRectangleOnSize1028WithPolyPolygonB2D();
    testDrawRectangleOnSize4096WithPolygonPolygonB2D();
    testDrawOpenPolygonWithPolyLine();
    testDrawOpenPolygonWithPolyLineB2D();
    testDrawOpenPolygonWithPolygon();
    testDrawOpenPolygonWithPolyPolygon();
    testDrawOpenPolygonWithPolyPolygonB2D();
    testDrawOpenBezierWithPolyLine();
    testDrawOpenBezierWithPolyLineB2D();
}

void GraphicsRenderTests::appendTestResult(OUString aTestName, OUString aTestStatus,
                                           Bitmap aTestBitmap)
{
    m_aTestResult.push_back(VclTestResult(aTestName, aTestStatus, aTestBitmap));
}

std::vector<VclTestResult>& GraphicsRenderTests::getTestResults() { return m_aTestResult; }

OUString GraphicsRenderTests::getResultString(bool bLocalize)
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
    // tdf#145919 localize for UI but not in the log file
    OUString resultString;
    if (bLocalize)
    {
        resultString
            = VclResId(STR_GBU).replaceFirst("%1", m_aCurGraphicsBackend) + "\n"
              + VclResId(STR_PASSED).replaceFirst("%1", OUString::number(testResults[0])) + "\n"
              + VclResId(STR_QUIRKY).replaceFirst("%1", OUString::number(testResults[1])) + "\n"
              + VclResId(STR_FAILED).replaceFirst("%1", OUString::number(testResults[2])) + "\n"
              + VclResId(STR_SKIPPED).replaceFirst("%1", OUString::number(testResults[3])) + "\n";
    }
    else
    {
        resultString = "Graphics Backend used: " + m_aCurGraphicsBackend
                       + "\nPassed Tests: " + OUString::number(testResults[0])
                       + "\nQuirky Tests: " + OUString::number(testResults[1])
                       + "\nFailed Tests: " + OUString::number(testResults[2])
                       + "\nSkipped Tests: " + OUString::number(testResults[3]) + "\n";
    }
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
