/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/outputdevice.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/vector/b2enums.hxx>

#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/print.hxx>
#include <vcl/rendercontext/RasterOp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <bufferdevice.hxx>
#include <window.h>

const size_t INITIAL_SETUP_ACTION_COUNT = 5;

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
    void testRasterOp();
    void testOutputFlag();
    void testAntialias();
    void testDrawMode();
    void testLayoutMode();
    void testDigitLanguage();
    void testStackFunctions();
    void testSystemTextColor();
    void testShouldDrawWavePixelAsRect();
    void testGetWaveLineSize();
    void testErase();
    void testDrawPixel();
    void testDrawLine();
    void testDrawRect();
    void testDrawArc();
    void testDrawEllipse();
    void testDrawPie();
    void testDrawChord();
    void testDrawCheckered();
    void testDrawBorder();
    void testDrawWaveLine();
    void testDrawPolyLine();
    void testDrawPolygon();
    void testDrawPolyPolygon();
    void testDrawGradient_drawmode();
    void testDrawGradient_rect_linear();
    void testDrawGradient_rect_axial();
    void testDrawGradient_polygon_linear();
    void testDrawGradient_polygon_axial();
    void testDrawGradient_rect_complex();

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
    CPPUNIT_TEST(testRasterOp);
    CPPUNIT_TEST(testOutputFlag);
    CPPUNIT_TEST(testAntialias);
    CPPUNIT_TEST(testDrawMode);
    CPPUNIT_TEST(testLayoutMode);
    CPPUNIT_TEST(testDigitLanguage);
    CPPUNIT_TEST(testStackFunctions);
    CPPUNIT_TEST(testSystemTextColor);
    CPPUNIT_TEST(testShouldDrawWavePixelAsRect);
    CPPUNIT_TEST(testGetWaveLineSize);
    CPPUNIT_TEST(testErase);
    CPPUNIT_TEST(testDrawPixel);
    CPPUNIT_TEST(testDrawLine);
    CPPUNIT_TEST(testDrawRect);
    CPPUNIT_TEST(testDrawArc);
    CPPUNIT_TEST(testDrawEllipse);
    CPPUNIT_TEST(testDrawPie);
    CPPUNIT_TEST(testDrawChord);
    CPPUNIT_TEST(testDrawCheckered);
    CPPUNIT_TEST(testDrawBorder);
    CPPUNIT_TEST(testDrawWaveLine);
    CPPUNIT_TEST(testDrawPolyLine);
    CPPUNIT_TEST(testDrawPolygon);
    CPPUNIT_TEST(testDrawPolyPolygon);
    CPPUNIT_TEST(testDrawGradient_drawmode);
    CPPUNIT_TEST(testDrawGradient_rect_linear);
    CPPUNIT_TEST(testDrawGradient_rect_axial);
    CPPUNIT_TEST(testDrawGradient_polygon_linear);
    CPPUNIT_TEST(testDrawGradient_polygon_axial);
    CPPUNIT_TEST(testDrawGradient_rect_complex);
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
    aBitmap.Erase(COL_RED);

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetDrawMode(DrawModeFlags::BlackBitmap);
    pVDev->DrawBitmap(Point(0, 0), Size(10, 10), Point(0, 0), Size(10, 10), aBitmap,
                      MetaActionType::BMP);

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::PUSH, pAction->GetType());
    auto pPushAction = static_cast<MetaPushAction*>(pAction);
    bool bLineFillFlag
        = ((vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR) == pPushAction->GetFlags());
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
    bool bLineFillFlag
        = ((vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR) == pPushAction->GetFlags());
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
    aBmp.Erase(COL_RED);

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

void VclOutdevTest::testRasterOp()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetRasterOp(RasterOp::Invert);

    CPPUNIT_ASSERT_EQUAL(RasterOp::Invert, pVDev->GetRasterOp());
    CPPUNIT_ASSERT(pVDev->IsLineColor());
    CPPUNIT_ASSERT(pVDev->IsFillColor());

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::RASTEROP, pAction->GetType());
    auto pRasterOpAction = static_cast<MetaRasterOpAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(RasterOp::Invert, pRasterOpAction->GetRasterOp());
}

void VclOutdevTest::testOutputFlag()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    CPPUNIT_ASSERT(pVDev->IsOutputEnabled());
    CPPUNIT_ASSERT(pVDev->IsDeviceOutputNecessary());

    pVDev->EnableOutput(false);

    CPPUNIT_ASSERT(!pVDev->IsOutputEnabled());
    CPPUNIT_ASSERT(!pVDev->IsDeviceOutputNecessary());
}

void VclOutdevTest::testAntialias()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    CPPUNIT_ASSERT_EQUAL(AntialiasingFlags::NONE, pVDev->GetAntialiasing());

    pVDev->SetAntialiasing(AntialiasingFlags::Enable);

    CPPUNIT_ASSERT_EQUAL(AntialiasingFlags::Enable, pVDev->GetAntialiasing());
}

void VclOutdevTest::testDrawMode()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    CPPUNIT_ASSERT_EQUAL(DrawModeFlags::Default, pVDev->GetDrawMode());

    pVDev->SetDrawMode(DrawModeFlags::BlackLine);

    CPPUNIT_ASSERT_EQUAL(DrawModeFlags::BlackLine, pVDev->GetDrawMode());
}

void VclOutdevTest::testLayoutMode()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    CPPUNIT_ASSERT_EQUAL(vcl::text::ComplexTextLayoutFlags::Default, pVDev->GetLayoutMode());

    pVDev->SetLayoutMode(vcl::text::ComplexTextLayoutFlags::BiDiRtl);

    CPPUNIT_ASSERT_EQUAL(vcl::text::ComplexTextLayoutFlags::BiDiRtl, pVDev->GetLayoutMode());

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::LAYOUTMODE, pAction->GetType());
    auto pLayoutModeAction = static_cast<MetaLayoutModeAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(vcl::text::ComplexTextLayoutFlags::BiDiRtl,
                         pLayoutModeAction->GetLayoutMode());
}

void VclOutdevTest::testDigitLanguage()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;

    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    CPPUNIT_ASSERT_EQUAL(LANGUAGE_SYSTEM, pVDev->GetDigitLanguage());

    pVDev->SetDigitLanguage(LANGUAGE_GERMAN);

    CPPUNIT_ASSERT_EQUAL(LANGUAGE_GERMAN, pVDev->GetDigitLanguage());

    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL(MetaActionType::TEXTLANGUAGE, pAction->GetType());
    auto pTextLanguageAction = static_cast<MetaTextLanguageAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL(LANGUAGE_GERMAN, pTextLanguageAction->GetTextLanguage());
}

void VclOutdevTest::testStackFunctions()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->Push();
    MetaAction* pAction = aMtf.GetAction(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Push action", MetaActionType::PUSH, pAction->GetType());

    pVDev->SetLineColor(COL_RED);
    pVDev->SetFillColor(COL_GREEN);
    pVDev->SetTextColor(COL_BROWN);
    pVDev->SetTextFillColor(COL_BLUE);
    pVDev->SetTextLineColor(COL_MAGENTA);
    pVDev->SetOverlineColor(COL_YELLOW);
    pVDev->SetTextAlign(TextAlign::ALIGN_TOP);
    pVDev->SetLayoutMode(vcl::text::ComplexTextLayoutFlags::BiDiRtl);
    pVDev->SetDigitLanguage(LANGUAGE_FRENCH);
    pVDev->SetRasterOp(RasterOp::N0);
    pVDev->SetMapMode(MapMode(MapUnit::MapTwip));
    pVDev->SetRefPoint(Point(10, 10));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Text color", COL_BROWN, pVDev->GetTextColor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Text fill color", COL_BLUE, pVDev->GetTextFillColor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Text line color", COL_MAGENTA, pVDev->GetTextLineColor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Text overline color", COL_YELLOW, pVDev->GetOverlineColor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Layout mode", vcl::text::ComplexTextLayoutFlags::BiDiRtl,
                                 pVDev->GetLayoutMode());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Language", LANGUAGE_FRENCH, pVDev->GetDigitLanguage());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Raster operation", RasterOp::N0, pVDev->GetRasterOp());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Map mode", MapMode(MapUnit::MapTwip), pVDev->GetMapMode());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Ref point", Point(10, 10), pVDev->GetRefPoint());

    pVDev->Pop();
    pAction = aMtf.GetAction(13);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Pop action", MetaActionType::POP, pAction->GetType());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default line color", COL_BLACK, pVDev->GetLineColor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default fill color", COL_WHITE, pVDev->GetFillColor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default text color", COL_BLACK, pVDev->GetTextColor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default text fill color", Color(ColorTransparency, 0xFFFFFFFF),
                                 pVDev->GetTextFillColor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default text line color", Color(ColorTransparency, 0xFFFFFFFF),
                                 pVDev->GetTextLineColor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default overline color", Color(ColorTransparency, 0xFFFFFFFF),
                                 pVDev->GetOverlineColor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default layout mode", vcl::text::ComplexTextLayoutFlags::Default,
                                 pVDev->GetLayoutMode());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default language", LANGUAGE_SYSTEM, pVDev->GetDigitLanguage());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default raster operation", RasterOp::OverPaint,
                                 pVDev->GetRasterOp());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default map mode", MapMode(MapUnit::MapPixel),
                                 pVDev->GetMapMode());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default ref point", Point(0, 0), pVDev->GetRefPoint());
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

void VclOutdevTest::testErase()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    // this actually triggers Erase()
    pVDev->SetOutputSizePixel(Size(10, 10));
    pVDev->Erase();

    MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a line color action (start)", MetaActionType::LINECOLOR,
                                 pAction->GetType());

    pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action (start)", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a rect action", MetaActionType::RECT, pAction->GetType());

    pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a line color action (end)", MetaActionType::LINECOLOR,
                                 pAction->GetType());

    pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action (end)", MetaActionType::FILLCOLOR,
                                 pAction->GetType());
}

void VclOutdevTest::testDrawPixel()
{
    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        // triggers an Erase()
        pVDev->SetOutputSizePixel(Size(10, 10));
        pVDev->SetLineColor(COL_RED);
        pVDev->DrawPixel(Point(0, 0), COL_GREEN);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Color not green", COL_GREEN, pVDev->GetPixel(Point(0, 0)));

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a pixel action", MetaActionType::PIXEL,
                                     pAction->GetType());
        MetaPixelAction* pPixelAction = dynamic_cast<MetaPixelAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Pixel action has incorrect position", Point(0, 0),
                                     pPixelAction->GetPoint());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Pixel action is wrong color", COL_GREEN,
                                     pPixelAction->GetColor());
    }

    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        pVDev->SetOutputSizePixel(Size(1, 1));
        pVDev->SetLineColor(COL_RED);
        pVDev->DrawPixel(Point(0, 0));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Color not red", COL_RED, pVDev->GetPixel(Point(0, 0)));

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a point action", MetaActionType::POINT,
                                     pAction->GetType());
        MetaPointAction* pPointAction = dynamic_cast<MetaPointAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Pixel action has incorrect position", Point(0, 0),
                                     pPointAction->GetPoint());
    }
}

void VclOutdevTest::testDrawLine()
{
    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        pVDev->SetOutputSizePixel(Size(10, 100));
        pVDev->DrawLine(Point(0, 0), Point(0, 50));

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a line action", MetaActionType::LINE, pAction->GetType());
        MetaLineAction* pLineAction = dynamic_cast<MetaLineAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line start has incorrect position", Point(0, 0),
                                     pLineAction->GetStartPoint());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line start has incorrect position", Point(0, 50),
                                     pLineAction->GetEndPoint());
    }

    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        LineInfo aLineInfo(LineStyle::Dash, 10);
        aLineInfo.SetDashCount(5);
        aLineInfo.SetDashLen(10);
        aLineInfo.SetDotCount(3);
        aLineInfo.SetDotLen(13);
        aLineInfo.SetDistance(8);
        aLineInfo.SetLineJoin(basegfx::B2DLineJoin::Bevel);
        aLineInfo.SetLineCap(css::drawing::LineCap_BUTT);

        pVDev->SetOutputSizePixel(Size(100, 100));
        pVDev->DrawLine(Point(0, 0), Point(0, 50), aLineInfo);

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a line action", MetaActionType::LINE, pAction->GetType());
        MetaLineAction* pLineAction = dynamic_cast<MetaLineAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line start has incorrect position", Point(0, 0),
                                     pLineAction->GetStartPoint());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line start has incorrect position", Point(0, 50),
                                     pLineAction->GetEndPoint());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dash count wrong", static_cast<sal_uInt16>(5),
                                     pLineAction->GetLineInfo().GetDashCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dash len wrong", static_cast<double>(10),
                                     pLineAction->GetLineInfo().GetDashLen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dot count wrong", static_cast<sal_uInt16>(3),
                                     pLineAction->GetLineInfo().GetDotCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dot len wrong", static_cast<double>(13),
                                     pLineAction->GetLineInfo().GetDotLen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Distance wrong", static_cast<double>(8),
                                     pLineAction->GetLineInfo().GetDistance());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line join", basegfx::B2DLineJoin::Bevel,
                                     pLineAction->GetLineInfo().GetLineJoin());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line cap", css::drawing::LineCap_BUTT,
                                     pLineAction->GetLineInfo().GetLineCap());
    }
}

void VclOutdevTest::testDrawRect()
{
    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        pVDev->SetOutputSizePixel(Size(100, 100));
        pVDev->DrawRect(tools::Rectangle(Point(0, 0), Size(50, 60)));

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a rect action", MetaActionType::RECT, pAction->GetType());
        MetaRectAction* pRectAction = dynamic_cast<MetaRectAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Rectangle wrong", tools::Rectangle(Point(0, 0), Size(50, 60)),
                                     pRectAction->GetRect());
    }

    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        pVDev->SetOutputSizePixel(Size(100, 100));
        pVDev->DrawRect(tools::Rectangle(Point(0, 0), Size(50, 60)), 5, 10);

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a rect action", MetaActionType::ROUNDRECT,
                                     pAction->GetType());
        MetaRoundRectAction* pRectAction = dynamic_cast<MetaRoundRectAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Rectangle wrong", tools::Rectangle(Point(0, 0), Size(50, 60)),
                                     pRectAction->GetRect());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Horizontal round rect wrong", static_cast<sal_uInt32>(5),
                                     pRectAction->GetHorzRound());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Vertical round rect wrong", static_cast<sal_uInt32>(10),
                                     pRectAction->GetVertRound());
    }
}

void VclOutdevTest::testDrawEllipse()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetOutputSizePixel(Size(100, 100));
    pVDev->DrawEllipse(tools::Rectangle(Point(0, 0), Size(50, 60)));

    MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a ellipse action", MetaActionType::ELLIPSE,
                                 pAction->GetType());
    MetaEllipseAction* pEllipseAction = dynamic_cast<MetaEllipseAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Ellipse rect wrong", tools::Rectangle(Point(0, 0), Size(50, 60)),
                                 pEllipseAction->GetRect());
}

void VclOutdevTest::testDrawPie()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    tools::Rectangle aRect(Point(0, 0), Size(50, 60));

    pVDev->SetOutputSizePixel(Size(100, 100));
    pVDev->DrawPie(aRect, aRect.TopRight(), aRect.TopCenter());

    MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a pie action", MetaActionType::PIE, pAction->GetType());
    MetaPieAction* pPieAction = dynamic_cast<MetaPieAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Pie rect wrong", aRect, pPieAction->GetRect());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Pie start point wrong", aRect.TopRight(),
                                 pPieAction->GetStartPoint());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Pie end point wrong", aRect.TopCenter(),
                                 pPieAction->GetEndPoint());
}

void VclOutdevTest::testDrawChord()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    tools::Rectangle aRect(Point(21, 22), Size(4, 4));
    pVDev->SetOutputSizePixel(Size(100, 100));
    pVDev->DrawChord(aRect, Point(30, 31), Point(32, 33));

    MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a chord action", MetaActionType::CHORD, pAction->GetType());
    MetaChordAction* pChordAction = dynamic_cast<MetaChordAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Chord rect wrong", aRect, pChordAction->GetRect());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Chord start point wrong", Point(30, 31),
                                 pChordAction->GetStartPoint());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Chord end point wrong", Point(32, 33),
                                 pChordAction->GetEndPoint());
}

void VclOutdevTest::testDrawArc()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    tools::Rectangle aRect(Point(1, 2), Size(4, 4));

    pVDev->SetOutputSizePixel(Size(100, 100));
    pVDev->DrawArc(aRect, Point(10, 11), Point(12, 13));

    MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a arc action", MetaActionType::ARC, pAction->GetType());
    MetaArcAction* pArcAction = dynamic_cast<MetaArcAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Arc rect wrong", aRect, pArcAction->GetRect());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Arc start point wrong", Point(10, 11),
                                 pArcAction->GetStartPoint());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Arc end point wrong", Point(12, 13), pArcAction->GetEndPoint());
}

void VclOutdevTest::testDrawCheckered()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetOutputSizePixel(Size(100, 100));
    pVDev->DrawCheckered(Point(0, 0), Size(100, 100), 20, COL_BLACK, COL_WHITE);

    size_t nIndex = INITIAL_SETUP_ACTION_COUNT;

    MetaAction* pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not push action", MetaActionType::PUSH, pAction->GetType());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a line color", MetaActionType::LINECOLOR, pAction->GetType());

    // Row 1
    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 1, rect 1",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    MetaFillColorAction* pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 1, rect 1", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not first rect, row 1", MetaActionType::RECT, pAction->GetType());
    MetaRectAction* pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 1, row 1 not correct",
                                 tools::Rectangle(Point(0, 0), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 1, rect 2",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 1, rect 1", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not second rect, row 1", MetaActionType::RECT,
                                 pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 2, row 1 not correct",
                                 tools::Rectangle(Point(0, 20), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 1, rect 3",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 1, rect 1", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not third rect, row 1", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 3, row 1 not correct",
                                 tools::Rectangle(Point(0, 40), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 1, rect 4",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 1, rect 4", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not fourth rect, row 1", MetaActionType::RECT,
                                 pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 4, row 1 not correct",
                                 tools::Rectangle(Point(0, 60), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 1, rect 5",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 1, rect 5", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not fifth rect, row 1", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 5, row 1 not correct",
                                 tools::Rectangle(Point(0, 80), Size(21, 21)),
                                 pRectAction->GetRect());

    // Row 2
    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 2, rect 1",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 2, rect 1", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 2, first rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 1, row 2 not correct",
                                 tools::Rectangle(Point(20, 0), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 2, rect 2",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 2, rect 1", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 2, second rect", MetaActionType::RECT,
                                 pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 2, row 2 not correct",
                                 tools::Rectangle(Point(20, 20), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 2, rect 3",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 2, rect 1", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 2, third rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 3, row 2 not correct",
                                 tools::Rectangle(Point(20, 40), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 2, rect 4",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 2, rect 4", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 2, fourth rect", MetaActionType::RECT,
                                 pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 4, row 2 not correct",
                                 tools::Rectangle(Point(20, 60), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 2, rect 5",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 2, rect 5", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 2, fifth rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 5, row 2 not correct",
                                 tools::Rectangle(Point(20, 80), Size(21, 21)),
                                 pRectAction->GetRect());

    // Row 3
    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 3, rect 1",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 3, rect 1", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 3, first rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 1, row 3 not correct",
                                 tools::Rectangle(Point(40, 0), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 3, rect 2",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 3, rect 1", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 3, second rect", MetaActionType::RECT,
                                 pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 2, row 3 not correct",
                                 tools::Rectangle(Point(40, 20), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 3, rect 3",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 3, rect 1", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 3, third rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 3, row 3 not correct",
                                 tools::Rectangle(Point(40, 40), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 3, rect 4",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 3, rect 4", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 3, fourth rect", MetaActionType::RECT,
                                 pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 4, row 3 not correct",
                                 tools::Rectangle(Point(40, 60), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 3, rect 5",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 3, rect 5", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 3, fifth rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 5, row 3 not correct",
                                 tools::Rectangle(Point(40, 80), Size(21, 21)),
                                 pRectAction->GetRect());

    // Row 4
    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 4, rect 1",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 4, rect 1", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 4, first rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 1, row 4 not correct",
                                 tools::Rectangle(Point(60, 0), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 4, rect 2",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 4, rect 1", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 4, second rect", MetaActionType::RECT,
                                 pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 2, row 2 not correct",
                                 tools::Rectangle(Point(60, 20), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 4, rect 3",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 4, rect 1", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 4, third rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 3, row 4 not correct",
                                 tools::Rectangle(Point(60, 40), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 4, rect 4",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 4, rect 4", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 4, fourth rect", MetaActionType::RECT,
                                 pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 4, row 4 not correct",
                                 tools::Rectangle(Point(60, 60), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 4, rect 5",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 4, rect 5", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 4, fifth rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 5, row 4 not correct",
                                 tools::Rectangle(Point(60, 80), Size(21, 21)),
                                 pRectAction->GetRect());

    // Row 5
    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 5, rect 1",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 5, rect 1", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 5, first rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 1, row 5 not correct",
                                 tools::Rectangle(Point(80, 0), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 5, rect 2",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 5, rect 1", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 2, second rect", MetaActionType::RECT,
                                 pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 2, row 2 not correct",
                                 tools::Rectangle(Point(80, 20), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 2, rect 3",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 2, rect 1", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 2, third rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 3, row 2 not correct",
                                 tools::Rectangle(Point(80, 40), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 2, rect 4",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 2, rect 4", COL_BLACK,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 2, fourth rect", MetaActionType::RECT,
                                 pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 4, row 2 not correct",
                                 tools::Rectangle(Point(80, 60), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action for row 2, rect 5",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pFillColorAction = dynamic_cast<MetaFillColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fill color wrong for row 2, rect 5", COL_WHITE,
                                 pFillColorAction->GetColor());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not row 2, fifth rect", MetaActionType::RECT, pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rect 5, row 2 not correct",
                                 tools::Rectangle(Point(80, 80), Size(21, 21)),
                                 pRectAction->GetRect());

    nIndex++;
    pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not pop", MetaActionType::POP, pAction->GetType());
}

void VclOutdevTest::testDrawBorder()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetOutputSizePixel(Size(100, 100));
    pVDev->DrawBorder(tools::Rectangle(Point(0, 0), Size(50, 60)));

    MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a line color action (light gray)", MetaActionType::LINECOLOR,
                                 pAction->GetType());
    MetaLineColorAction* pLineColorAction = dynamic_cast<MetaLineColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not light gray", COL_LIGHTGRAY, pLineColorAction->GetColor());

    pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a rect action (light gray border)", MetaActionType::RECT,
                                 pAction->GetType());
    MetaRectAction* pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rectangle wrong", tools::Rectangle(Point(1, 1), Size(49, 59)),
                                 pRectAction->GetRect());

    pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a line color action (gray)", MetaActionType::LINECOLOR,
                                 pAction->GetType());
    pLineColorAction = dynamic_cast<MetaLineColorAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not gray", COL_GRAY, pLineColorAction->GetColor());

    pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a rect action (gray border)", MetaActionType::RECT,
                                 pAction->GetType());
    pRectAction = dynamic_cast<MetaRectAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rectangle wrong", tools::Rectangle(Point(0, 0), Size(49, 59)),
                                 pRectAction->GetRect());
}

void VclOutdevTest::testDrawWaveLine()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetOutputSizePixel(Size(100, 100));
    pVDev->DrawWaveLine(Point(0, 0), Point(50, 0));

    MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a bitmap action", MetaActionType::BMPEXSCALEPART,
                                 pAction->GetType());
}

void VclOutdevTest::testDrawPolyLine()
{
    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        pVDev->SetOutputSizePixel(Size(100, 100));
        tools::Polygon aPolygon(vcl::test::OutputDeviceTestCommon::createClosedBezierLoop(
            tools::Rectangle(Point(10, 10), Size(80, 8))));

        pVDev->DrawPolyLine(aPolygon);

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYLINE,
                                     pAction->GetType());
        MetaPolyLineAction* pPolyLineAction = dynamic_cast<MetaPolyLineAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Polygon in polyline action is wrong", aPolygon,
                                     pPolyLineAction->GetPolygon());
    }

    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        pVDev->SetOutputSizePixel(Size(100, 100));

        tools::Polygon aPolygon(vcl::test::OutputDeviceTestCommon::createClosedBezierLoop(
            tools::Rectangle(Point(10, 10), Size(80, 8))));

        LineInfo aLineInfo(LineStyle::Dash, 10);
        aLineInfo.SetDashCount(5);
        aLineInfo.SetDashLen(10);
        aLineInfo.SetDotCount(3);
        aLineInfo.SetDotLen(13);
        aLineInfo.SetDistance(8);
        aLineInfo.SetLineJoin(basegfx::B2DLineJoin::Bevel);
        aLineInfo.SetLineCap(css::drawing::LineCap_BUTT);

        pVDev->DrawPolyLine(aPolygon, aLineInfo);

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYLINE,
                                     pAction->GetType());
        MetaPolyLineAction* pPolyLineAction = dynamic_cast<MetaPolyLineAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Polygon in polyline action is wrong", aPolygon,
                                     pPolyLineAction->GetPolygon());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dash count wrong", static_cast<sal_uInt16>(5),
                                     pPolyLineAction->GetLineInfo().GetDashCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dash len wrong", static_cast<double>(10),
                                     pPolyLineAction->GetLineInfo().GetDashLen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dot count wrong", static_cast<sal_uInt16>(3),
                                     pPolyLineAction->GetLineInfo().GetDotCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dot len wrong", static_cast<double>(13),
                                     pPolyLineAction->GetLineInfo().GetDotLen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Distance wrong", static_cast<double>(8),
                                     pPolyLineAction->GetLineInfo().GetDistance());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line join", basegfx::B2DLineJoin::Bevel,
                                     pPolyLineAction->GetLineInfo().GetLineJoin());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line cap", css::drawing::LineCap_BUTT,
                                     pPolyLineAction->GetLineInfo().GetLineCap());
    }

    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        pVDev->SetOutputSizePixel(Size(100, 100));

        basegfx::B2DPolygon aPolygon(vcl::test::OutputDeviceTestCommon::createClosedBezierLoop(
                                         tools::Rectangle(Point(10, 10), Size(80, 8)))
                                         .getB2DPolygon());

        LineInfo aLineInfo(LineStyle::Dash, 10);
        aLineInfo.SetDashCount(5);
        aLineInfo.SetDashLen(10);
        aLineInfo.SetDotCount(3);
        aLineInfo.SetDotLen(13);
        aLineInfo.SetDistance(8);
        aLineInfo.SetLineJoin(basegfx::B2DLineJoin::Bevel);
        aLineInfo.SetLineCap(css::drawing::LineCap_BUTT);

        pVDev->DrawPolyLine(aPolygon, 3, basegfx::B2DLineJoin::Bevel, css::drawing::LineCap_BUTT,
                            basegfx::deg2rad(15.0));

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYLINE,
                                     pAction->GetType());
        MetaPolyLineAction* pPolyLineAction = dynamic_cast<MetaPolyLineAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Polygon in polyline action is wrong", aPolygon,
                                     pPolyLineAction->GetPolygon().getB2DPolygon());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Width wrong", static_cast<double>(3),
                                     pPolyLineAction->GetLineInfo().GetWidth());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dash count wrong", static_cast<sal_uInt16>(0),
                                     pPolyLineAction->GetLineInfo().GetDashCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dash len wrong", static_cast<double>(0),
                                     pPolyLineAction->GetLineInfo().GetDashLen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dot count wrong", static_cast<sal_uInt16>(0),
                                     pPolyLineAction->GetLineInfo().GetDotCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Dot len wrong", static_cast<double>(0),
                                     pPolyLineAction->GetLineInfo().GetDotLen());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Distance wrong", static_cast<double>(0),
                                     pPolyLineAction->GetLineInfo().GetDistance());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line join", basegfx::B2DLineJoin::Bevel,
                                     pPolyLineAction->GetLineInfo().GetLineJoin());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line cap", css::drawing::LineCap_BUTT,
                                     pPolyLineAction->GetLineInfo().GetLineCap());
    }
}

void VclOutdevTest::testDrawPolygon()
{
    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        pVDev->SetOutputSizePixel(Size(100, 100));
        tools::Polygon aPolygon(vcl::test::OutputDeviceTestCommon::createClosedBezierLoop(
            tools::Rectangle(Point(10, 10), Size(80, 8))));

        pVDev->DrawPolygon(aPolygon);

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                     pAction->GetType());
        MetaPolygonAction* pPolygonAction = dynamic_cast<MetaPolygonAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Polygon in polygon action is wrong", aPolygon,
                                     pPolygonAction->GetPolygon());
    }

    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        pVDev->SetOutputSizePixel(Size(100, 100));
        tools::Polygon aPolygon(vcl::test::OutputDeviceTestCommon::createClosedBezierLoop(
            tools::Rectangle(Point(10, 10), Size(80, 8))));

        pVDev->DrawPolygon(aPolygon);

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polygon action", MetaActionType::POLYGON,
                                     pAction->GetType());
        MetaPolygonAction* pPolygonAction = dynamic_cast<MetaPolygonAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Polygon in polygon action is wrong", aPolygon,
                                     pPolygonAction->GetPolygon());
    }
}

static tools::PolyPolygon createPolyPolygon()
{
    tools::Polygon aPolygon(4);

    aPolygon.SetPoint(Point(1, 8), 0);
    aPolygon.SetPoint(Point(2, 7), 1);
    aPolygon.SetPoint(Point(3, 6), 2);
    aPolygon.SetPoint(Point(4, 5), 3);

    tools::PolyPolygon aPolyPolygon(aPolygon);
    aPolyPolygon.Optimize(PolyOptimizeFlags::CLOSE);

    return aPolyPolygon;
}

void VclOutdevTest::testDrawPolyPolygon()
{
    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        pVDev->SetOutputSizePixel(Size(100, 100));

        tools::PolyPolygon aPolyPolygon = createPolyPolygon();

        pVDev->DrawPolyPolygon(aPolyPolygon);

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polypolygon action", MetaActionType::POLYPOLYGON,
                                     pAction->GetType());

        MetaPolyPolygonAction* pPolyPolygonAction = dynamic_cast<MetaPolyPolygonAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not the same polypolygon in polypolygon action", aPolyPolygon,
                                     pPolyPolygonAction->GetPolyPolygon());
    }

    {
        ScopedVclPtrInstance<VirtualDevice> pVDev;
        GDIMetaFile aMtf;
        aMtf.Record(pVDev.get());

        pVDev->SetOutputSizePixel(Size(100, 100));

        tools::PolyPolygon aPolyPolygon = createPolyPolygon();

        basegfx::B2DPolyPolygon aB2DPolyPolygon(aPolyPolygon.getB2DPolyPolygon());

        pVDev->DrawPolyPolygon(aB2DPolyPolygon);

        MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polypolygon action", MetaActionType::POLYPOLYGON,
                                     pAction->GetType());

        /* these should match, but the equality operator does not work on PolyPolygon for some reason

        MetaPolyPolygonAction* pPolyPolygonAction = dynamic_cast<MetaPolyPolygonAction*>(pAction);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not the same polypolygon in polypolygon action", aPolyPolygon,
                                     pPolyPolygonAction->GetPolyPolygon());
        */
    }
}

static size_t ClipGradientTest(GDIMetaFile& rMtf, size_t nIndex)
{
    MetaAction* pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a comment action", MetaActionType::COMMENT,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a gradientex action", MetaActionType::GRADIENTEX,
                                 pAction->GetType());

    // clip gradient
    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a push action", MetaActionType::PUSH, pAction->GetType());
    MetaPushAction* pPushAction = dynamic_cast<MetaPushAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not using XOR push flags", vcl::PushFlags::RASTEROP,
                                 pPushAction->GetFlags());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a rasterop action", MetaActionType::RASTEROP,
                                 pAction->GetType());
    MetaRasterOpAction* pRasterOpAction = dynamic_cast<MetaRasterOpAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not an XOR rasterop", RasterOp::Xor,
                                 pRasterOpAction->GetRasterOp());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a gradient action", MetaActionType::GRADIENT,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a rasterop action", MetaActionType::RASTEROP,
                                 pAction->GetType());
    pRasterOpAction = dynamic_cast<MetaRasterOpAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not an N0 rasterop", RasterOp::N0,
                                 pRasterOpAction->GetRasterOp());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polypolygon action", MetaActionType::POLYPOLYGON,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a rasterop action", MetaActionType::RASTEROP,
                                 pAction->GetType());
    pRasterOpAction = dynamic_cast<MetaRasterOpAction*>(pAction);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not an XOR rasterop", RasterOp::Xor,
                                 pRasterOpAction->GetRasterOp());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a gradient action", MetaActionType::GRADIENT,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a pop action", MetaActionType::POP, pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a comment action", MetaActionType::COMMENT,
                                 pAction->GetType());

    return nIndex;
}

void VclOutdevTest::testDrawGradient_drawmode()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    pVDev->SetOutputSizePixel(Size(100, 100));
    pVDev->SetDrawMode(DrawModeFlags::BlackGradient);

    tools::Rectangle aRect(Point(10, 10), Size(40, 40));
    pVDev->DrawGradient(aRect, Gradient());
    MetaAction* pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a push action (drawmode is black gradient)",
                                 MetaActionType::PUSH, pAction->GetType());
    MetaPushAction* pPushAction = dynamic_cast<MetaPushAction*>(pAction);
    vcl::PushFlags eFlags = vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Push flags wrong (drawmode is black gradient)", eFlags,
                                 pPushAction->GetFlags());

    pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a line color action (drawmode is black gradient)",
                                 MetaActionType::LINECOLOR, pAction->GetType());
    pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action (drawmode is black gradient)",
                                 MetaActionType::FILLCOLOR, pAction->GetType());
    pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polypolygon action (drawmode is black gradient)",
                                 MetaActionType::POLYPOLYGON, pAction->GetType());
    pAction = aMtf.GetAction(INITIAL_SETUP_ACTION_COUNT + 4);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a pop action (drawmode is black gradient)",
                                 MetaActionType::POP, pAction->GetType());
}

void VclOutdevTest::testDrawGradient_rect_linear()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    tools::Rectangle aRect(Point(10, 10), Size(40, 40));
    pVDev->SetOutputSizePixel(Size(100, 100));

    Gradient aGradient(GradientStyle::Linear, COL_RED, COL_WHITE);
    aGradient.SetBorder(100);

    pVDev->DrawGradient(aRect, aGradient);

    size_t nIndex = INITIAL_SETUP_ACTION_COUNT;

    MetaAction* pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a gradient action (rectangle area)", MetaActionType::GRADIENT,
                                 pAction->GetType());

    pAction = aMtf.GetAction(nIndex + 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a linear gradient action", MetaActionType::LINEARGRADIENT,
                                 pAction->GetType());
}

void VclOutdevTest::testDrawGradient_rect_axial()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    tools::Rectangle aRect(Point(10, 10), Size(40, 40));
    pVDev->SetOutputSizePixel(Size(100, 100));

    Gradient aGradient(GradientStyle::Axial, COL_RED, COL_WHITE);
    aGradient.SetBorder(100);

    pVDev->DrawGradient(aRect, aGradient);

    size_t nIndex = INITIAL_SETUP_ACTION_COUNT;

    MetaAction* pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a gradient action (rectangle area)", MetaActionType::GRADIENT,
                                 pAction->GetType());

    pAction = aMtf.GetAction(nIndex + 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a linear gradient action", MetaActionType::LINEARGRADIENT,
                                 pAction->GetType());
}

void VclOutdevTest::testDrawGradient_polygon_linear()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    tools::PolyPolygon aPolyPolygon = createPolyPolygon();

    pVDev->SetOutputSizePixel(Size(100, 100));

    Gradient aGradient(GradientStyle::Linear, COL_RED, COL_WHITE);
    aGradient.SetBorder(100);

    pVDev->DrawGradient(aPolyPolygon, aGradient);

    size_t nIndex = ClipGradientTest(aMtf, INITIAL_SETUP_ACTION_COUNT);

    MetaAction* pAction = aMtf.GetAction(nIndex + 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a linear gradient action", MetaActionType::LINEARGRADIENT,
                                 pAction->GetType());
}

void VclOutdevTest::testDrawGradient_polygon_axial()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    tools::PolyPolygon aPolyPolygon = createPolyPolygon();

    pVDev->SetOutputSizePixel(Size(100, 100));

    Gradient aGradient(GradientStyle::Axial, COL_RED, COL_WHITE);
    aGradient.SetBorder(100);

    pVDev->DrawGradient(aPolyPolygon, aGradient);

    size_t nIndex = ClipGradientTest(aMtf, INITIAL_SETUP_ACTION_COUNT);

    MetaAction* pAction = aMtf.GetAction(nIndex + 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a linear gradient action", MetaActionType::LINEARGRADIENT,
                                 pAction->GetType());
}

static size_t TestComplexStripes(GDIMetaFile& rMtf, size_t nTimes, size_t nIndex)
{
    nIndex++;
    MetaAction* pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    for (size_t i = 1; i < nTimes; i++)
    {
        nIndex++;
        pAction = rMtf.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polypolygon action", MetaActionType::POLYPOLYGON,
                                     pAction->GetType());

        nIndex++;
        pAction = rMtf.GetAction(nIndex);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                     pAction->GetType());
    }

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a fill color action", MetaActionType::FILLCOLOR,
                                 pAction->GetType());

    nIndex++;
    pAction = rMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a polypolygon action", MetaActionType::POLYGON,
                                 pAction->GetType());

    return nIndex;
}

void VclOutdevTest::testDrawGradient_rect_complex()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    GDIMetaFile aMtf;
    aMtf.Record(pVDev.get());

    tools::Rectangle aRect(Point(10, 10), Size(40, 40));
    pVDev->SetOutputSizePixel(Size(1000, 1000));

    Gradient aGradient(GradientStyle::Square, COL_RED, COL_WHITE);
    aGradient.SetBorder(10);
    pVDev->DrawGradient(aRect, aGradient);

    size_t nIndex = INITIAL_SETUP_ACTION_COUNT;

    MetaAction* pAction = aMtf.GetAction(nIndex);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not a gradient action (rectangle area)", MetaActionType::GRADIENT,
                                 pAction->GetType());

    nIndex = TestComplexStripes(aMtf, 40, nIndex);
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclOutdevTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
