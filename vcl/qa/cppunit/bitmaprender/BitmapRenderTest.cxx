/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <test/bootstrapfixture.hxx>

#include <vcl/virdev.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/svapp.hxx>

#include <tools/stream.hxx>

#include <vcl/graphicfilter.hxx>
#include <vcl/filter/PngImageReader.hxx>

#include <svdata.hxx>
#include <salinst.hxx>

constexpr OUStringLiteral gaDataUrl = u"/vcl/qa/cppunit/bitmaprender/data/";

class BitmapRenderTest : public test::BootstrapFixture
{
    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(gaDataUrl) + sFileName;
    }

public:
    BitmapRenderTest()
        : BootstrapFixture(true, false)
    {
    }

    void testTdf104141();
    void testTdf113918();
    void testDrawAlphaBitmapEx();
    void testAlphaVirtualDevice();
    void testTdf116888();

    CPPUNIT_TEST_SUITE(BitmapRenderTest);
    CPPUNIT_TEST(testTdf104141);
    CPPUNIT_TEST(testTdf113918);
    CPPUNIT_TEST(testDrawAlphaBitmapEx);
    CPPUNIT_TEST(testAlphaVirtualDevice);
    CPPUNIT_TEST(testTdf116888);

    CPPUNIT_TEST_SUITE_END();
};

void BitmapRenderTest::testTdf104141()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    pVDev->SetOutputSizePixel(Size(400, 400));
    pVDev->SetBackground(Wallpaper(COL_GREEN));
    pVDev->Erase();

    // Load animated GIF and draw it on green background
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    const OUString aURL(getFullUrl(u"tdf104141.gif"));
    SvFileStream aFileStream(aURL, StreamMode::READ);
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    pVDev->DrawBitmapEx(Point(20, 20), aBitmap);

    // Check drawing results: ensure that it contains transparent
    // (greenish) pixels
    const Color aColor = pVDev->GetPixel(Point(21, 21));
    CPPUNIT_ASSERT(aColor.GetGreen() > 10 * aColor.GetRed());
    CPPUNIT_ASSERT(aColor.GetGreen() > 10 * aColor.GetBlue());
}

void BitmapRenderTest::testTdf113918()
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    pVDev->SetOutputSizePixel(Size(2480, 3508));
    pVDev->SetBackground(Wallpaper(COL_GREEN));
    pVDev->Erase();

    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    const OUString aURL(getFullUrl(u"tdf113918.png"));
    SvFileStream aFileStream(aURL, StreamMode::READ);
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    pVDev->DrawBitmapEx(Point(0, 0), aBitmap);

    // Ensure that image is drawn with white background color from palette
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(21, 21)));

    // Ensure that image is drawn with gray text color from palette
    const Color aColor = pVDev->GetPixel(Point(1298, 1368));
    CPPUNIT_ASSERT_EQUAL(aColor.GetGreen(), aColor.GetRed());
    CPPUNIT_ASSERT_EQUAL(aColor.GetGreen(), aColor.GetBlue());
    CPPUNIT_ASSERT(aColor.GetGreen() > 100);
}

#if defined(_WIN32) || defined(IOS)

namespace
{
int deltaColor(BitmapColor aColor1, BitmapColor aColor2)
{
    int deltaR = std::abs(aColor1.GetRed() - aColor2.GetRed());
    int deltaG = std::abs(aColor1.GetGreen() - aColor2.GetGreen());
    int deltaB = std::abs(aColor1.GetBlue() - aColor2.GetBlue());

    return std::max(std::max(deltaR, deltaG), deltaB);
}
}

#endif

void BitmapRenderTest::testDrawAlphaBitmapEx()
{
// TODO: This unit test is not executed for macOS unless bitmap scaling is implemented
#ifndef MACOSX
    if (isWindowsRDP())
        return;
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    pVDev->SetOutputSizePixel(Size(8, 8));
    pVDev->SetBackground(Wallpaper(COL_WHITE));
    pVDev->Erase();

    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(0, 0)));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(1, 1)));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pVDev->GetPixel(Point(2, 2)));

    SvFileStream aFileStream(getFullUrl(u"ImageRGBA.png"), StreamMode::READ);

    vcl::PngImageReader aPngReader(aFileStream);
    BitmapEx aBitmapEx;
    aPngReader.read(aBitmapEx);

    // Check backend capabilities, if the backend support 32-bit bitmap
    auto pBackendCapabilities = ImplGetSVData()->mpDefInst->GetBackendCapabilities();
    if (pBackendCapabilities->mbSupportsBitmap32)
    {
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(32), aBitmapEx.GetBitmap().GetBitCount());
    }
    else
    {
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(24), aBitmapEx.GetBitmap().GetBitCount());
        CPPUNIT_ASSERT_EQUAL(true, aBitmapEx.IsAlpha());
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), aBitmapEx.GetAlpha().GetBitCount());
    }

    // Check the bitmap has pixels we expect
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xFF, 0x00, 0x00, 0x00),
                         aBitmapEx.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0x00, 0xFF, 0xFF, 0x00),
                         aBitmapEx.GetPixelColor(1, 1));
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0x7F, 0x00, 0xFF, 0x00),
                         aBitmapEx.GetPixelColor(2, 2));

    pVDev->DrawBitmapEx(Point(), aBitmapEx);

    CPPUNIT_ASSERT_EQUAL(Color(0xFF, 0xFF, 0xFF), pVDev->GetPixel(Point(0, 0)));
    CPPUNIT_ASSERT_EQUAL(Color(0xFF, 0xFF, 0x00), pVDev->GetPixel(Point(1, 1)));

#if defined(_WIN32) || defined(MACOSX) || defined(IOS)
    // sometimes on Windows we get rounding error in blending so let's ignore this on Windows for now.
    CPPUNIT_ASSERT_LESS(2, deltaColor(Color(0x7F, 0xFF, 0x7F), pVDev->GetPixel(Point(2, 2))));
#else
    CPPUNIT_ASSERT_EQUAL(Color(0x7F, 0xFF, 0x7F), pVDev->GetPixel(Point(2, 2)));
#endif
#endif
}

void BitmapRenderTest::testAlphaVirtualDevice()
{
// TODO: This unit test is not executed for macOS unless bitmap scaling is implemented
#ifndef MACOSX
    // Create an alpha virtual device
    ScopedVclPtr<VirtualDevice> pAlphaVirtualDevice(VclPtr<VirtualDevice>::Create(
        *Application::GetDefaultDevice(), DeviceFormat::DEFAULT, DeviceFormat::DEFAULT));

    // Set it up
    pAlphaVirtualDevice->SetOutputSizePixel(Size(4, 4));
    pAlphaVirtualDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pAlphaVirtualDevice->Erase();

    // Get a BitmapEx from the VirDev -> Colors should have alpha
    BitmapEx aBitmap = pAlphaVirtualDevice->GetBitmapEx(Point(), Size(4, 4));
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), aBitmap.GetSizePixel().Height());
    Color aColor = aBitmap.GetPixelColor(1, 1);
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, aColor);

    // Draw an opaque pixel to the VirDev
    pAlphaVirtualDevice->DrawPixel(Point(1, 1), Color(0x0022ff55));

    aColor = pAlphaVirtualDevice->GetPixel(Point(1, 1));
    // Read back the opaque pixel
#if defined _WIN32
    CPPUNIT_ASSERT_LESS(6, deltaColor(Color(0x0022ff55), aColor));
#else
    CPPUNIT_ASSERT_EQUAL(Color(0x0022ff55), aColor);
#endif

    // Read back the BitmapEx and check the opaque pixel
    aBitmap = pAlphaVirtualDevice->GetBitmapEx(Point(), Size(4, 4));
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), aBitmap.GetSizePixel().Height());

    aColor = aBitmap.GetPixelColor(1, 1);
#if defined _WIN32
    CPPUNIT_ASSERT_LESS(6, deltaColor(Color(0x0022ff55), aColor));
#else
    CPPUNIT_ASSERT_EQUAL(Color(0x0022ff55), aColor);
#endif

    // Draw an semi-transparent pixel
    pAlphaVirtualDevice->DrawPixel(Point(0, 0), Color(ColorTransparency, 0x44, 0x22, 0xff, 0x55));

    aColor = pAlphaVirtualDevice->GetPixel(Point(0, 0));
    // Read back the semi-transparent pixel
#if defined _WIN32
    CPPUNIT_ASSERT_LESS(6, deltaColor(Color(ColorTransparency, 0x4422FF55), aColor));
#else
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0x4422FF55), aColor);
#endif

    // Read back the BitmapEx and check the semi-transparent pixel
    aBitmap = pAlphaVirtualDevice->GetBitmapEx(Point(), Size(4, 4));
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), aBitmap.GetSizePixel().Height());

    aColor = aBitmap.GetPixelColor(0, 0);
#if defined _WIN32
    CPPUNIT_ASSERT_LESS(6, deltaColor(Color(ColorTransparency, 0x4422FF55), aColor));
#else
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0x4422FF55), aColor);
#endif
#endif
}

void BitmapRenderTest::testTdf116888()
{
    // The image is a 8bit image with a non-grayscale palette. In OpenGL mode
    // pdf export of the image was broken, because OpenGLSalBitmap::ReadTexture()
    // didn't handle 8bit non-grayscale and moreover OpenGLSalBitmap::AcquireBuffer()
    // didn't properly release mpUserBuffer after ReadTexture() failure.
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    const OUString aURL(getFullUrl(u"tdf116888.gif"));
    SvFileStream aFileStream(aURL, StreamMode::READ);
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
    CPPUNIT_ASSERT(!aBitmap.IsEmpty());
    aBitmap.Scale(0.8, 0.8); // This scaling discards mpUserData,
    Bitmap::ScopedReadAccess pAccess(aBitmap); // forcing ReadTexture() here.
    // Check that there is mpUserBuffer content.
    CPPUNIT_ASSERT(pAccess);
    const ScanlineFormat eFormat = pAccess->GetScanlineFormat();
    CPPUNIT_ASSERT_EQUAL(ScanlineFormat::N8BitPal, eFormat);
    CPPUNIT_ASSERT(!aBitmap.HasGreyPaletteAny());
    // HACK: Some rendering backends change white to #FEFEFE while scaling for some reason.
    // That is pretty much white too in practice, so adjust for that.
    BitmapColor white(COL_WHITE);
    if (pAccess->GetColor(0, 0) == Color(0xfe, 0xfe, 0xfe))
        white = Color(0xfe, 0xfe, 0xfe);
    // Check that the image contents are also valid.
    CPPUNIT_ASSERT_EQUAL(white, pAccess->GetColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(white, pAccess->GetColor(0, pAccess->Width() - 1));
    CPPUNIT_ASSERT_EQUAL(white, pAccess->GetColor(pAccess->Height() - 1, 0));
    CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_BLACK),
                         pAccess->GetColor(pAccess->Height() - 1, pAccess->Width() - 1));
}

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapRenderTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
