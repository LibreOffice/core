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
#include <bitmapwriteaccess.hxx>
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
    void testDrawTransformedBitmapEx();
    void testDrawTransformedBitmapExFlip();
    void testRTL();
    void testRTLGuard();

    CPPUNIT_TEST_SUITE(VclOutdevTest);
    CPPUNIT_TEST(testVirtualDevice);
    CPPUNIT_TEST(testUseAfterDispose);
    CPPUNIT_TEST(testPrinterBackgroundColor);
    CPPUNIT_TEST(testWindowBackgroundColor);
    CPPUNIT_TEST(testGetReadableFontColorPrinter);
    CPPUNIT_TEST(testGetReadableFontColorWindow);
    CPPUNIT_TEST(testDrawTransformedBitmapEx);
    CPPUNIT_TEST(testDrawTransformedBitmapExFlip);
    CPPUNIT_TEST(testRTL);
    CPPUNIT_TEST(testRTLGuard);
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
    CPPUNIT_ASSERT_EQUAL(pWindow->GetReadableFontColor(COL_WHITE, COL_BLACK), COL_WHITE);
    CPPUNIT_ASSERT_EQUAL(pWindow->GetReadableFontColor(COL_WHITE, COL_WHITE), COL_BLACK);
    CPPUNIT_ASSERT_EQUAL(pWindow->GetReadableFontColor(COL_BLACK, COL_BLACK), COL_WHITE);
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

void VclOutdevTest::testDrawTransformedBitmapEx()
{
    // Create a virtual device, and connect a metafile to it.
    // Also create a 16x16 bitmap.
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    Bitmap aBitmap(Size(16, 16), 24);
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
    Bitmap aBitmap(Size(16, 16), 24);
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
    vcl::RenderContext& rRenderContext = *pWindow;
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

CPPUNIT_TEST_SUITE_REGISTRATION(VclOutdevTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
