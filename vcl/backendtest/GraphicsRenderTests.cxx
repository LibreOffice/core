/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
    ~GraphicsTestZone() { vcl::test::setActiveGraphicsRenderTest(u""_ustr); }
};
}

OUString GraphicsRenderTests::returnTestStatus(vcl::test::TestResult const result)
{
    switch (result)
    {
        case vcl::test::TestResult::Passed:
            return u"PASSED"_ustr;
        case vcl::test::TestResult::PassedWithQuirks:
            return u"QUIRKY"_ustr;
        case vcl::test::TestResult::Failed:
            return u"FAILED"_ustr;
    }
    return u"SKIPPED"_ustr;
}

void GraphicsRenderTests::testDrawRectWithRectangle()
{
    OUString aTestName = u"testDrawRectWithRectangle"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    m_aCurGraphicsBackend = aOutDevTest.getRenderBackendName();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectWithPixel"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectWithLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectWithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectWithPolyLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectWithPolyLineB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectWithPolyPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectWithPolyPolygonB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectAAWithRectangle"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectAAWithPixel"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectAAWithLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectAAWithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectAAWithPolyLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectAAWithPolyLineB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectAAWithPolyPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectAAWithPolyPolygonB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawFilledRectWithRectangle"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawFilledRectWithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawFilledRectWithPolyPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawFilledRectWithPolyPolygon2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawDiamondWithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawDiamondWithLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawDiamondWithPolyline"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawDiamondWithPolylineB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawInvertWithRectangle"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_NONE();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawInvertN50WithRectangle"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_N50();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawInvertTrackFrameWithRectangle"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_TrackFrame();
    if (!(SHOULD_ASSERT && aOutDevTest.getRenderBackendName() != "svp"))
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawDropShapeWithPolyline"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDropShape();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawDropShapeAAWithPolyline"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAADropShape();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawBezierWithPolylineB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupBezier();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawBezierAAWithPolylineB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAABezier();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawDropShapeWithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDropShape();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawDropShapeAAWithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAADropShape();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawBitmap24bpp"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N24_BPP);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawTransformedBitmap24bpp"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N24_BPP);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testComplexDrawTransformedBitmap24bpp"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupComplexDrawTransformedBitmap(vcl::PixelFormat::N24_BPP);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawBitmapExWithAlpha24bpp"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha(vcl::PixelFormat::N24_BPP);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawMask24bpp"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawMask(vcl::PixelFormat::N24_BPP);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawBlend24bpp"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend(vcl::PixelFormat::N24_BPP);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawXor"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupXOR();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testClipRectangle"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipRectangle();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testClipPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolygon();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testClipPolyPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolyPolygon();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testClipB2DPolyPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipB2DPolyPolygon();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawOutDev"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDev();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawOutDevScaledClipped"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDevScaledClipped();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawOutDevSelf"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDevSelf();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDashedLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDashedLine();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testLinearGradient"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradient();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testLinearGradientAngled"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientAngled();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testLinearGradientBorder"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientBorder();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testLinearGradientIntensity"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientIntensity();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testLinearGradientSteps"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientSteps();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testAxialGradient"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAxialGradient();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testRadialGradient"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradient();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testRadialGradientOfs"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradientOfs();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
        return;
    }
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradientOfs(aBitmap);
    appendTestResult(aTestName, returnTestStatus(eResult),
                     (m_aStoreResultantBitmap ? aBitmap : Bitmap()));
}

void GraphicsRenderTests::testLineJoinBevel()
{
    OUString aTestName = u"testLineJoinBevel"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinBevel();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testLineJoinRound"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinRound();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testLineJoinMiter"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinMiter();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testLineJoinNone"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinNone();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testLineCapRound"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapRound();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testLineCapSquare"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapSquare();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testLineCapButt"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapButt();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testHalfEllipseWithPolyLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testHalfEllipseAAWithPolyLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testHalfEllipseWithPolyLineB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testHalfEllipseAAWithPolyLineB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testHalfEllipseWithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testClosedBezierWithPolyline"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClosedBezier();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testClosedBezierWithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClosedBezier();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testHalfEllipseAAWithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupHalfEllipse(true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testFilledAsymmetricalDropShape"_ustr;
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
    OUString aTestName = u"testTextDrawing"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestText aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupTextBitmap();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testEvenOddRuleInIntersectingRectsWithPolyPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupIntersectingRectangles();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testEvenOddRuleInIntersectingRectsWithPolyPolygonB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupIntersectingRectangles();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawBitmap8bppGreyScale"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N8_BPP, true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawTransformedBitmap8bppGreyScale"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N8_BPP, true);
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawBitmap32bpp"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap(vcl::PixelFormat::N32_BPP);
    if (!SHOULD_ASSERT || !is32bppSupported())
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawTransformedBitmap32bpp"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap(vcl::PixelFormat::N32_BPP);
    if (!SHOULD_ASSERT || !is32bppSupported())
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawBitmapExWithAlpha32bpp"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha(vcl::PixelFormat::N32_BPP);
    if (!SHOULD_ASSERT || !is32bppSupported())
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawMask32bpp"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawMask(vcl::PixelFormat::N32_BPP);
    if (!SHOULD_ASSERT || !is32bppSupported())
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawBlend32bpp"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend(vcl::PixelFormat::N32_BPP);
    if (!SHOULD_ASSERT || !is32bppSupported())
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize1028WithRect"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    m_aCurGraphicsBackend = aOutDevTest.getRenderBackendName();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize4096WithRect"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    m_aCurGraphicsBackend = aOutDevTest.getRenderBackendName();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize1028WithPixel"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize4096WithPixel"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize1028WithLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize4096WithLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize1028WithPolyLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize4096WithPolyLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize1028WithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize4096WithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize1028WithPolyLineB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize4096WithPolyLineB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize1028WithPolyPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize4096WithPolyPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize1028WithPolyPolygonB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize1028();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawRectangleOnSize4096WithPolygonPolygonB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangleOnSize4096();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawOpenPolygonWithPolyLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawOpenPolygonWithPolyLineB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawOpenPolygonWithPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawOpenPolygonWithPolyPolygon"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawOpenPolygonWithPolyPolygonB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenPolygon();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawOpenBezierWithPolyLine"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenBezier();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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
    OUString aTestName = u"testDrawOpenBezierWithPolyLineB2D"_ustr;
    GraphicsTestZone zone(aTestName);
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupOpenBezier();
    if (!SHOULD_ASSERT)
    {
        appendTestResult(aTestName, u"SKIPPED"_ustr);
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

void GraphicsRenderTests::appendTestResult(const OUString& rTestName, const OUString& rTestStatus,
                                           const Bitmap& rTestBitmap)
{
    m_aTestResult.push_back(VclTestResult(rTestName, rTestStatus, rTestBitmap));
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
    if (static_cast<int>(aTests[u"FAILED"_ustr].size()) > 0)
    {
        for (const class OUString& tests : aTests[u"FAILED"_ustr])
        {
            writeResult += tests + "\n";
        }
    }
    else
    {
        writeResult += "No test has been failed.\n";
    }
    writeResult += "\n---Name of the tests that were Quirky---\n";
    if (static_cast<int>(aTests[u"QUIRKY"_ustr].size()) > 0)
    {
        for (const class OUString& tests : aTests[u"QUIRKY"_ustr])
        {
            writeResult += tests + "\n";
        }
    }
    else
    {
        writeResult += "No test was Quirky.\n";
    }
    writeResult += "\n---Name of the tests that were Skipped---\n";
    if (static_cast<int>(aTests[u"SKIPPED"_ustr].size()) > 0)
    {
        for (const class OUString& tests : aTests[u"SKIPPED"_ustr])
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
