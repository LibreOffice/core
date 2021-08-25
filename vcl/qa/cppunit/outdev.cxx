/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <vcl/print.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <bufferdevice.hxx>
#include <window.h>

#include <basegfx/matrix/b2dhommatrix.hxx>

class VclOutdevTest : public test::BootstrapFixture
{
public:
    VclOutdevTest()
        : BootstrapFixture(true, false)
    {
    }

    void testVirtualDevice();
    void testUseAfterDispose();
    void testPrinterBackgroundColor();
    void testWindowBackgroundColor();
    void testGetReadableFontColorPrinter();
    void testGetReadableFontColorWindow();
    void testDrawInvertedBitmap();
    void testDrawBlackBitmap();
    void testDrawWhiteBitmap();
    void testDrawGrayBitmap();
    void testDrawBitmap();
    void testDrawScaleBitmap();
    void testDrawScalePartBitmap();
    void testDrawTransformedBitmapEx();
    void testDrawTransformedBitmapExFlip();
    void testRTL();
    void testRTLGuard();
    void testDefaultFillColor();
    void testTransparentFillColor();
    void testFillColor();
    void testDefaultLineColor();
    void testTransparentLineColor();
    void testLineColor();
    void testFont();
    void testTransparentFont();
    void testDefaultRefPoint();
    void testRefPoint();
    void testSystemTextColor();
    void testShouldDrawWavePixelAsRect();
    void testGetWaveLineSize();

    CPPUNIT_TEST_SUITE(VclOutdevTest);
    CPPUNIT_TEST(testVirtualDevice);
    CPPUNIT_TEST(testUseAfterDispose);
    CPPUNIT_TEST(testPrinterBackgroundColor);
    CPPUNIT_TEST(testWindowBackgroundColor);
    CPPUNIT_TEST(testDrawInvertedBitmap);
    CPPUNIT_TEST(testDrawBlackBitmap);
    CPPUNIT_TEST(testDrawWhiteBitmap);
    CPPUNIT_TEST(testDrawGrayBitmap);
    CPPUNIT_TEST(testDrawBitmap);
    CPPUNIT_TEST(testDrawScaleBitmap);
    CPPUNIT_TEST(testDrawScalePartBitmap);
    CPPUNIT_TEST(testGetReadableFontColorPrinter);
    CPPUNIT_TEST(testGetReadableFontColorWindow);
    CPPUNIT_TEST(testDrawTransformedBitmapEx);
    CPPUNIT_TEST(testDrawTransformedBitmapExFlip);
    CPPUNIT_TEST(testRTL);
    CPPUNIT_TEST(testRTLGuard);
    CPPUNIT_TEST(testDefaultFillColor);
    CPPUNIT_TEST(testTransparentFillColor);
    CPPUNIT_TEST(testFillColor);
    CPPUNIT_TEST(testDefaultLineColor);
    CPPUNIT_TEST(testTransparentLineColor);
    CPPUNIT_TEST(testLineColor);
    CPPUNIT_TEST(testFont);
    CPPUNIT_TEST(testTransparentFont);
    CPPUNIT_TEST(testDefaultRefPoint);
    CPPUNIT_TEST(testRefPoint);
    CPPUNIT_TEST(testSystemTextColor);
    CPPUNIT_TEST(testShouldDrawWavePixelAsRect);
    CPPUNIT_TEST(testGetWaveLineSize);
    CPPUNIT_TEST_SUITE_END();
};

void VclOutdevTest::testGetReadableFontColorPrinter()
{
    ScopedVclPtrInstance<Printer> pPrinter;
    CPPUNIT_ASSERT_EQUAL(pPrinter->GetReadableFontColor(COL_WHITE, COL_WHITE), COL_BLACK);
}

void VclOutdevTest::testGetReadableFontColorWindow()
{
    ScopedVclPtrInstance<vcl::Window> pWindow(nullptr, WB_APP | WB_STDWORK);
    CPPUNIT_ASSERT_EQUAL(pWindow->GetOutDev()->GetReadableFontColor(COL_WHITE, COL_BLACK),
                         COL_WHITE);
    CPPUNIT_ASSERT_EQUAL(pWindow->GetOutDev()->GetReadableFontColor(COL_WHITE, COL_WHITE),
                         COL_BLACK);
    CPPUNIT_ASSERT_EQUAL(pWindow->GetOutDev()->GetReadableFontColor(COL_BLACK, COL_BLACK),
                         COL_WHITE);
}

void VclOutdevTest::testPrinterBackgroundColor()
{
    ScopedVclPtrInstance<Printer> pPrinter;
    CPPUNIT_ASSERT_EQUAL(pPrinter->GetBackgroundColor(), COL_WHITE);
}

void VclOutdevTest::testWindowBackgroundColor()
{
    ScopedVclPtrInstance<vcl::Window> pWindow(nullptr, WB_APP | WB_STDWORK);
    pWindow->SetBackground(Wallpaper(COL_WHITE));
    CPPUNIT_ASSERT_EQUAL(pWindow->GetBackgroundColor(), COL_WHITE);
}

void VclOutdevTest::testVirtualDevice()
{
// TODO: This unit test is not executed for macOS unless bitmap scaling is implemented
#ifndef MACOSX
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    pVDev->SetOutputSizePixel(Size(32, 32));
    pVDev->SetBackground(Wallpaper(COL_WHITE));

    CPPUNIT_ASSERT_EQUAL(pVDev->GetBackgroundColor(), COL_WHITE);

    pVDev->Erase();
    pVDev->DrawPixel(Point(1, 2), COL_BLUE);
    pVDev->DrawPixel(Point(31, 30), COL_RED);

    Size aSize = pVDev->GetOutputSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(32, 32), aSize);

    Bitmap aBmp = pVDev->GetBitmap(Point(), aSize);

#if 0
    OUString rFileName("/tmp/foo-unx.png");
    try {
        vcl::PNGWriter aWriter( aBmp );
        SvFileStream sOutput( rFileName, StreamMode::WRITE );
        aWriter.Write( sOutput );
        sOutput.Close();
    } catch (...) {
        SAL_WARN("vcl", "Error writing png to " << rFileName);
    }
#endif

    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(0, 0)));
#if !defined _WIN32 //TODO: various failures on Windows tinderboxes
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, pVDev->GetPixel(Point(1, 2)));
    CPPUNIT_ASSERT_EQUAL(COL_RED, pVDev->GetPixel(Point(31, 30)));
#endif
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(30, 31)));

    // Gotcha: y and x swap for BitmapReadAccess: deep joy.
    Bitmap::ScopedReadAccess pAcc(aBmp);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, static_cast<Color>(pAcc->GetPixel(0, 0)));
#if !defined _WIN32 //TODO: various failures on Windows tinderboxes
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, static_cast<Color>(pAcc->GetPixel(2, 1)));
    CPPUNIT_ASSERT_EQUAL(COL_RED, static_cast<Color>(pAcc->GetPixel(30, 31)));
#endif
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, static_cast<Color>(pAcc->GetPixel(31, 30)));

#if 0
    VclPtr<vcl::Window> pWin = VclPtr<WorkWindow>::Create( (vcl::Window *)nullptr );
    CPPUNIT_ASSERT( pWin );
    OutputDevice *pOutDev = pWin.get();
#endif
#endif
}

void VclOutdevTest::testUseAfterDispose()
{
    // Create a virtual device, enable map mode then dispose it.
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    pVDev->EnableMapMode();

    pVDev->disposeOnce();

    // Make sure that these don't crash after dispose.
    pVDev->GetInverseViewTransformation();

    pVDev->GetViewTransformation();
}

void VclOutdevTest::testDrawInvertedBitmap()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetRasterOp(RasterOp::Invert);
    pVDev->DrawBitmap(Point(0, 0), Size(10, 10), Point(0, 0), Size(10, 10), aBitmap,
                      MetaActionType::BMP);

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::RASTEROP, pAction->GetType());
    auto pRasterOpAction = static_cast<MetaRasterOpAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(RasterOp::Invert, pRasterOpAction->GetRasterOp());

    pAction = aMtf.GetAction(1);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::RECT, pAction->GetType());
    auto pRectAction = static_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(0, 0), Size(10, 10)), pRectAction->GetRect());
}

void VclOutdevTest::testDrawBlackBitmap()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(COL_RED);
    }

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetDrawMode(DrawModeFlags::BlackBitmap);
    pVDev->DrawBitmap(Point(0, 0), Size(10, 10), Point(0, 0), Size(10, 10), aBitmap,
                      MetaActionType::BMP);

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::PUSH, pAction->GetType());
    auto pPushAction = static_cast<MetaPushAction*>(pAction);
    bool bLineFillFlag = ((PushFlags::LINECOLOR | PushFlags::FILLCOLOR) == pPushAction->GetFlags());
    CPPUNIT_ASSERT_MESSAGE("Push flags not LINECOLOR | FILLCOLOR", bLineFillFlag);

    pAction = aMtf.GetAction(1);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::LINECOLOR, pAction->GetType());
    auto pLineColorAction = static_cast<MetaLineColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pLineColorAction->GetColor());

    pAction = aMtf.GetAction(2);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::FILLCOLOR, pAction->GetType());
    auto pFillColorAction = static_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pFillColorAction->GetColor());

    pAction = aMtf.GetAction(3);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::RECT, pAction->GetType());
    auto pRectAction = static_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(0, 0), Size(10, 10)), pRectAction->GetRect());

    pAction = aMtf.GetAction(4);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::POP, pAction->GetType());

    // test to see if the color is black
    Bitmap aBlackBmp(pVDev->GetBitmap(Point(0, 0), Size(10, 10)));
    Bitmap::ScopedReadAccess pReadAccess(aBlackBmp);
    const BitmapColor& rColor = pReadAccess->GetColor(0, 0);
    CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_BLACK), rColor);
}

void VclOutdevTest::testDrawWhiteBitmap()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetDrawMode(DrawModeFlags::WhiteBitmap);
    pVDev->DrawBitmap(Point(0, 0), Size(10, 10), Point(0, 0), Size(10, 10), aBitmap,
                      MetaActionType::BMP);

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::PUSH, pAction->GetType());
    auto pPushAction = static_cast<MetaPushAction*>(pAction);
    bool bLineFillFlag = ((PushFlags::LINECOLOR | PushFlags::FILLCOLOR) == pPushAction->GetFlags());
    CPPUNIT_ASSERT_MESSAGE("Push flags not LINECOLOR | FILLCOLOR", bLineFillFlag);

    pAction = aMtf.GetAction(1);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::LINECOLOR, pAction->GetType());
    auto pLineColorAction = static_cast<MetaLineColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pLineColorAction->GetColor());

    pAction = aMtf.GetAction(2);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::FILLCOLOR, pAction->GetType());
    auto pFillColorAction = static_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pFillColorAction->GetColor());

    pAction = aMtf.GetAction(3);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::RECT, pAction->GetType());
    auto pRectAction = static_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(0, 0), Size(10, 10)), pRectAction->GetRect());

    pAction = aMtf.GetAction(4);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::POP, pAction->GetType());

    // test to see if the color is white
    Bitmap aWhiteBmp(pVDev->GetBitmap(Point(0, 0), Size(10, 10)));
    Bitmap::ScopedReadAccess pReadAccess(aWhiteBmp);
    const BitmapColor& rColor = pReadAccess->GetColor(0, 0);
    CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_WHITE), rColor);
}

void VclOutdevTest::testDrawBitmap()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->DrawBitmap(Point(0, 0), Size(10, 10), Point(0, 0), Size(10, 10), aBitmap,
                      MetaActionType::BMP);

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::BMP, pAction->GetType());
    auto pBmpAction = static_cast<MetaBmpAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(Size(16, 16), pBmpAction->GetBitmap().GetSizePixel());
    CPPUNIT_ASSERT_EQUAL(Point(0, 0), pBmpAction->GetPoint());
}

void VclOutdevTest::testDrawScaleBitmap()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->DrawBitmap(Point(5, 5), Size(10, 10), Point(0, 0), Size(10, 10), aBitmap,
                      MetaActionType::BMPSCALE);

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::BMPSCALE, pAction->GetType());
    auto pBmpScaleAction = static_cast<MetaBmpScaleAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(Size(16, 16), pBmpScaleAction->GetBitmap().GetSizePixel());
    CPPUNIT_ASSERT_EQUAL(Point(5, 5), pBmpScaleAction->GetPoint());
    CPPUNIT_ASSERT_EQUAL(Size(10, 10), pBmpScaleAction->GetSize());
}

void VclOutdevTest::testDrawScalePartBitmap()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->DrawBitmap(Point(0, 0), Size(10, 10), Point(5, 5), Size(10, 10), aBitmap,
                      MetaActionType::BMPSCALEPART);

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::BMPSCALEPART, pAction->GetType());
    auto pBmpScalePartAction = static_cast<MetaBmpScalePartAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(Size(16, 16), pBmpScalePartAction->GetBitmap().GetSizePixel());
    CPPUNIT_ASSERT_EQUAL(Point(5, 5), pBmpScalePartAction->GetSrcPoint());
    CPPUNIT_ASSERT_EQUAL(Size(10, 10), pBmpScalePartAction->GetSrcSize());
    CPPUNIT_ASSERT_EQUAL(Point(0, 0), pBmpScalePartAction->GetDestPoint());
    CPPUNIT_ASSERT_EQUAL(Size(10, 10), pBmpScalePartAction->GetDestSize());
}

void VclOutdevTest::testDrawGrayBitmap()
{
    // draw a red 1x1 bitmap
    Bitmap aBmp(Size(1, 1), vcl::PixelFormat::N24_BPP);
    BitmapScopedWriteAccess pWriteAccess(aBmp);
    pWriteAccess->Erase(COL_RED);

    // check to ensure that the bitmap is red
    {
        Bitmap::ScopedReadAccess pReadAccess(aBmp);
        const BitmapColor& rColor = pReadAccess->GetColor(0, 0);
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_RED), rColor);
    }

    ScopedVclPtrInstance<VirtualDevice> pVDev;

    pVDev->SetDrawMode(DrawModeFlags::GrayBitmap);
    pVDev->DrawBitmap(Point(0, 0), Size(1, 1), Point(0, 0), Size(1, 1), aBmp, MetaActionType::BMP);

    // should be a grey
    Bitmap aVDevBmp(pVDev->GetBitmap(Point(), Size(1, 1)));
    {
        Bitmap::ScopedReadAccess pReadAccess(aVDevBmp);
        const BitmapColor& rColor = pReadAccess->GetColor(0, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x26), sal_Int32(rColor.GetRed()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x26), sal_Int32(rColor.GetGreen()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x26), sal_Int32(rColor.GetBlue()));
    }
}

void VclOutdevTest::testDrawTransformedBitmapEx()
{
    // Create a virtual device, and connect a metafile to it.
    // Also create a 16x16 bitmap.
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);
    {
        // Fill the top left quarter with black.
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(COL_WHITE);
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                pWriteAccess->SetPixel(j, i, COL_BLACK);
            }
        }
    }
    BitmapEx aBitmapEx(aBitmap);
    basegfx::B2DHomMatrix aMatrix;
    aMatrix.scale(8, 8);
    // Rotate 90 degrees clockwise, so the black part goes to the top right.
    aMatrix.rotate(M_PI / 2);
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    // Draw the rotated bitmap on the vdev.
    pVDev->DrawTransformedBitmapEx(aMatrix, aBitmapEx);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aMtf.GetActionSize());
    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::BMPEXSCALE, pAction->GetType());
    auto pBitmapAction = static_cast<MetaBmpExScaleAction*>(pAction);
    const BitmapEx& rBitmapEx = pBitmapAction->GetBitmapEx();
    Size aTransformedSize = rBitmapEx.GetSizePixel();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 16x16
    // - Actual  : 8x8
    // I.e. the bitmap before scaling was already scaled down, just because it was rotated.
    CPPUNIT_ASSERT_EQUAL(Size(16, 16), aTransformedSize);

    aBitmap = rBitmapEx.GetBitmap();
    Bitmap::ScopedReadAccess pAccess(aBitmap);
    for (int i = 0; i < 16; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            BitmapColor aColor = pAccess->GetPixel(j, i);
            Color aExpected = i >= 8 && j < 8 ? COL_BLACK : COL_WHITE;
            std::stringstream ss;
            ss << "Color is expected to be ";
            ss << ((aExpected == COL_WHITE) ? "white" : "black");
            ss << ", is " << aColor.AsRGBHexString();
            ss << " (row " << j << ", col " << i << ")";
            // Without the accompanying fix in place, this test would have failed with:
            // - Expected: c[00000000]
            // - Actual  : c[ffffff00]
            // - Color is expected to be black, is ffffff (row 0, col 8)
            // i.e. the top right quarter of the image was not fully black, there was a white first
            // row.
            CPPUNIT_ASSERT_EQUAL_MESSAGE(ss.str(), aExpected, Color(aColor));
        }
    }
}

void VclOutdevTest::testDrawTransformedBitmapExFlip()
{
    // Create a virtual device, and connect a metafile to it.
    // Also create a 16x16 bitmap.
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);
    {
        // Fill the top left quarter with black.
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(COL_WHITE);
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                pWriteAccess->SetPixel(j, i, COL_BLACK);
            }
        }
    }
    BitmapEx aBitmapEx(aBitmap);
    basegfx::B2DHomMatrix aMatrix;
    // Negative y scale: bitmap should be upside down, so the black part goes to the bottom left.
    aMatrix.scale(8, -8);
    // Rotate 90 degrees clockwise, so the black part goes back to the top left.
    aMatrix.rotate(M_PI / 2);
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    // Draw the scaled and rotated bitmap on the vdev.
    pVDev->DrawTransformedBitmapEx(aMatrix, aBitmapEx);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aMtf.GetActionSize());
    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::BMPEXSCALE, pAction->GetType());
    auto pBitmapAction = static_cast<MetaBmpExScaleAction*>(pAction);
    const BitmapEx& rBitmapEx = pBitmapAction->GetBitmapEx();

    aBitmap = rBitmapEx.GetBitmap();
    Bitmap::ScopedReadAccess pAccess(aBitmap);
    int nX = 8 * 0.25;
    int nY = 8 * 0.25;
    BitmapColor aColor = pAccess->GetPixel(nY, nX);
    std::stringstream ss;
    ss << "Color is expected to be black, is " << aColor.AsRGBHexString();
    ss << " (row " << nY << ", col " << nX << ")";
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: c[00000000]
    // - Actual  : c[ffffff00]
    // - Color is expected to be black, is ffffff (row 2, col 2)
    // i.e. the top left quarter of the image was not black, due to a missing flip.
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ss.str(), COL_BLACK, Color(aColor));
}

void VclOutdevTest::testRTL()
{
    ScopedVclPtrInstance<vcl::Window> pWindow(nullptr, WB_APP | WB_STDWORK);
    pWindow->EnableRTL();
    vcl::RenderContext& rRenderContext = *pWindow->GetOutDev();
    vcl::BufferDevice pBuffer(pWindow, rRenderContext);

    // Without the accompanying fix in place, this test would have failed, because the RTL status
    // from pWindow was not propagated to pBuffer.
    CPPUNIT_ASSERT(pBuffer->IsRTLEnabled());
}

void VclOutdevTest::testRTLGuard()
{
    ScopedVclPtrInstance<vcl::Window> pWindow(nullptr, WB_APP | WB_STDWORK);
    pWindow->EnableRTL();
    pWindow->RequestDoubleBuffering(true);
    ImplFrameData* pFrameData = pWindow->ImplGetWindowImpl()->mpFrameData;
    vcl::PaintBufferGuard aGuard(pFrameData, pWindow);
    // Without the accompanying fix in place, this test would have failed, because the RTL status
    // from pWindow was not propagated to aGuard.
    CPPUNIT_ASSERT(aGuard.GetRenderContext()->IsRTLEnabled());
}

void VclOutdevTest::testDefaultFillColor()
{
    // Create a virtual device, and connect a metafile to it.
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    CPPUNIT_ASSERT(pVDev->IsFillColor());
    CPPUNIT_ASSERT_EQUAL(Color(0xFF, 0xFF, 0xFF), pVDev->GetFillColor());

    pVDev->SetFillColor();
    CPPUNIT_ASSERT(!pVDev->IsFillColor());
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, pVDev->GetFillColor());
    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::FILLCOLOR, pAction->GetType());
    auto pFillAction = static_cast<MetaFillColorAction*>(pAction);
    const Color& rColor = pFillAction->GetColor();
    CPPUNIT_ASSERT_EQUAL(Color(), rColor);
}

void VclOutdevTest::testTransparentFillColor()
{
    // Create a virtual device, and connect a metafile to it.
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    CPPUNIT_ASSERT(pVDev->IsFillColor());
    CPPUNIT_ASSERT_EQUAL(Color(0xFF, 0xFF, 0xFF), pVDev->GetFillColor());

    pVDev->SetFillColor(COL_TRANSPARENT);
    CPPUNIT_ASSERT(!pVDev->IsFillColor());
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, pVDev->GetFillColor());
    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::FILLCOLOR, pAction->GetType());
    auto pFillAction = static_cast<MetaFillColorAction*>(pAction);
    const Color& rColor = pFillAction->GetColor();
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, rColor);
}

void VclOutdevTest::testFillColor()
{
    // Create a virtual device, and connect a metafile to it.
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    CPPUNIT_ASSERT(pVDev->IsFillColor());
    CPPUNIT_ASSERT_EQUAL(Color(0xFF, 0xFF, 0xFF), pVDev->GetFillColor());

    pVDev->SetFillColor(COL_RED);
    CPPUNIT_ASSERT(pVDev->IsFillColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, pVDev->GetFillColor());
    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::FILLCOLOR, pAction->GetType());
    auto pFillAction = static_cast<MetaFillColorAction*>(pAction);
    const Color& rColor = pFillAction->GetColor();
    CPPUNIT_ASSERT_EQUAL(COL_RED, rColor);
}

void VclOutdevTest::testDefaultLineColor()
{
    // Create a virtual device, and connect a metafile to it.
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    CPPUNIT_ASSERT(pVDev->IsLineColor());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pVDev->GetLineColor());

    pVDev->SetLineColor();
    CPPUNIT_ASSERT(!pVDev->IsLineColor());
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, pVDev->GetLineColor());
    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::LINECOLOR, pAction->GetType());
    auto pLineAction = static_cast<MetaLineColorAction*>(pAction);
    const Color& rColor = pLineAction->GetColor();
    CPPUNIT_ASSERT_EQUAL(Color(), rColor);
}

void VclOutdevTest::testTransparentLineColor()
{
    // Create a virtual device, and connect a metafile to it.
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    CPPUNIT_ASSERT(pVDev->IsLineColor());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pVDev->GetLineColor());

    pVDev->SetLineColor(COL_TRANSPARENT);
    CPPUNIT_ASSERT(!pVDev->IsLineColor());
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, pVDev->GetLineColor());
    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::LINECOLOR, pAction->GetType());
    auto pLineAction = static_cast<MetaLineColorAction*>(pAction);
    const Color& rColor = pLineAction->GetColor();
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, rColor);
}

void VclOutdevTest::testLineColor()
{
    // Create a virtual device, and connect a metafile to it.
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    CPPUNIT_ASSERT(pVDev->IsLineColor());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pVDev->GetLineColor());

    pVDev->SetLineColor(COL_RED);
    CPPUNIT_ASSERT(pVDev->IsLineColor());
    CPPUNIT_ASSERT_EQUAL(COL_RED, pVDev->GetLineColor());
    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::LINECOLOR, pAction->GetType());
    auto pLineAction = static_cast<MetaLineColorAction*>(pAction);
    const Color& rColor = pLineAction->GetColor();
    CPPUNIT_ASSERT_EQUAL(COL_RED, rColor);
}

void VclOutdevTest::testFont()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    // Use Dejavu fonts, they are shipped with LO, so they should be ~always available.
    // Use Sans variant for simpler glyph shapes (no serifs).
    vcl::Font font("DejaVu Sans", "Book", Size(0, 36));
    font.SetColor(COL_BLACK);
    font.SetFillColor(COL_RED);

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetFont(font);
    bool bSameFont(font == pVDev->GetFont());
    CPPUNIT_ASSERT_MESSAGE("Font is not the same", bSameFont);

    // four actions:
    // 1. Font action
    // 2. Text alignment action
    // 3. Text fill color action
    // 4. As not COL_TRANSPARENT (means use system font color), font color action
    size_t nActionsExpected = 4;
    CPPUNIT_ASSERT_EQUAL(nActionsExpected, aMtf.GetActionSize());

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::FONT, pAction->GetType());
    auto pFontAction = static_cast<MetaFontAction*>(pAction);
    bool bSameMetaFont = (font == pFontAction->GetFont());
    CPPUNIT_ASSERT_MESSAGE("Metafile font is not the same", bSameMetaFont);

    pAction = aMtf.GetAction(1);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::TEXTALIGN, pAction->GetType());
    auto pTextAlignAction = static_cast<MetaTextAlignAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(font.GetAlignment(), pTextAlignAction->GetTextAlign());

    pAction = aMtf.GetAction(2);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::TEXTFILLCOLOR, pAction->GetType());
    auto pTextFillColorAction = static_cast<MetaTextFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(COL_RED, pTextFillColorAction->GetColor());

    pAction = aMtf.GetAction(3);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::TEXTCOLOR, pAction->GetType());
    auto pTextColorAction = static_cast<MetaTextColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pTextColorAction->GetColor());
}

void VclOutdevTest::testTransparentFont()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    // Use Dejavu fonts, they are shipped with LO, so they should be ~always available.
    // Use Sans variant for simpler glyph shapes (no serifs).
    vcl::Font font("DejaVu Sans", "Book", Size(0, 36));
    font.SetColor(COL_TRANSPARENT);

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetFont(font);

    // three actions as it sets the colour to the default system color (and doesn't add a text color action):
    // 1. Font action
    // 2. Text alignment action
    // 3. Text fill color action
    size_t nActionsExpected = 3;
    CPPUNIT_ASSERT_EQUAL(nActionsExpected, aMtf.GetActionSize());
}

void VclOutdevTest::testDefaultRefPoint()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetRefPoint();

    CPPUNIT_ASSERT(!pVDev->IsRefPoint());
    CPPUNIT_ASSERT_EQUAL(Point(), pVDev->GetRefPoint());

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::REFPOINT, pAction->GetType());
    auto pRefPointAction = static_cast<MetaRefPointAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(Point(), pRefPointAction->GetRefPoint());
}

void VclOutdevTest::testRefPoint()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetRefPoint(Point(10, 20));

    CPPUNIT_ASSERT(pVDev->IsRefPoint());
    CPPUNIT_ASSERT_EQUAL(Point(10, 20), pVDev->GetRefPoint());

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::REFPOINT, pAction->GetType());
    auto pRefPointAction = static_cast<MetaRefPointAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(Point(10, 20), pRefPointAction->GetRefPoint());
}

void VclOutdevTest::testSystemTextColor()
{
    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;

        pVDev->SetSystemTextColor(SystemTextColorFlags::NONE, true);
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, pVDev->GetTextColor());
        pVDev->SetSystemTextColor(SystemTextColorFlags::Mono, false);
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, pVDev->GetTextColor());
    }

    {
        ScopedVclPtrInstance<Printer> pPrinter;
        pPrinter->SetSystemTextColor(SystemTextColorFlags::NONE, true);
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, pPrinter->GetTextColor());
    }
}

namespace
{
class WaveLineTester : public OutputDevice
{
public:
    WaveLineTester()
        : OutputDevice(OUTDEV_VIRDEV)
    {
    }

    bool AcquireGraphics() const { return true; }
    void ReleaseGraphics(bool) {}
    bool UsePolyPolygonForComplexGradient() { return false; }

    bool testShouldDrawWavePixelAsRect(tools::Long nLineWidth)
    {
        return shouldDrawWavePixelAsRect(nLineWidth);
    }

    Size testGetWaveLineSize(tools::Long nLineWidth) { return GetWaveLineSize(nLineWidth); }
};

class WaveLineTesterPrinter : public Printer
{
public:
    WaveLineTesterPrinter() {}

    bool AcquireGraphics() const { return true; }
    void ReleaseGraphics(bool) {}
    bool UsePolyPolygonForComplexGradient() { return false; }

    Size testGetWaveLineSize(tools::Long nLineWidth) { return GetWaveLineSize(nLineWidth); }
};
}

void VclOutdevTest::testShouldDrawWavePixelAsRect()
{
    ScopedVclPtrInstance<WaveLineTester> pTestOutDev;

    CPPUNIT_ASSERT(!pTestOutDev->testShouldDrawWavePixelAsRect(0));
    CPPUNIT_ASSERT(!pTestOutDev->testShouldDrawWavePixelAsRect(1));

    CPPUNIT_ASSERT(pTestOutDev->testShouldDrawWavePixelAsRect(10));
}

void VclOutdevTest::testGetWaveLineSize()
{
    {
        ScopedVclPtrInstance<WaveLineTester> pTestOutDev;

        pTestOutDev->SetDPIX(96);
        pTestOutDev->SetDPIY(96);

        CPPUNIT_ASSERT_EQUAL(Size(1, 1), pTestOutDev->testGetWaveLineSize(0));
        CPPUNIT_ASSERT_EQUAL(Size(1, 1), pTestOutDev->testGetWaveLineSize(1));

        CPPUNIT_ASSERT_EQUAL(Size(10, 10), pTestOutDev->testGetWaveLineSize(10));
    }

    {
        ScopedVclPtrInstance<WaveLineTesterPrinter> pTestOutDev;

        pTestOutDev->SetDPIX(96);
        pTestOutDev->SetDPIY(96);

        CPPUNIT_ASSERT_EQUAL(Size(0, 0), pTestOutDev->testGetWaveLineSize(0));
        CPPUNIT_ASSERT_EQUAL(Size(1, 1), pTestOutDev->testGetWaveLineSize(1));

        CPPUNIT_ASSERT_EQUAL(Size(10, 10), pTestOutDev->testGetWaveLineSize(10));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclOutdevTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
