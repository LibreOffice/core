/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <unordered_map>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/virdev.hxx>

#include <rtl/strbuf.hxx>
#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include "BitmapSymmetryCheck.hxx"

namespace
{

class BitmapTest : public CppUnit::TestFixture
{
    void testConvert();
    void testScale();
    void testCRC();

    CPPUNIT_TEST_SUITE(BitmapTest);
    CPPUNIT_TEST(testConvert);
    CPPUNIT_TEST(testScale);
    CPPUNIT_TEST(testCRC);
    CPPUNIT_TEST_SUITE_END();
};

void BitmapTest::testConvert()
{
    Bitmap aBitmap(Size(10, 10), 8);

    aBitmap.Erase(COL_LIGHTGRAYBLUE);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(8), aBitmap.GetBitCount());
    {
        Bitmap::ScopedReadAccess pReadAccess(aBitmap);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(8), pReadAccess->GetBitCount());
#if defined MACOSX || defined IOS
        //it would be nice to find and change the stride for quartz to be the same as everyone else
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(10), pReadAccess->GetScanlineSize());
#else
        if (!OpenGLHelper::isVCLOpenGLEnabled())
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(12), pReadAccess->GetScanlineSize());
#endif
        CPPUNIT_ASSERT(pReadAccess->HasPalette());
        const BitmapColor& rColor = pReadAccess->GetPaletteColor(pReadAccess->GetPixelIndex(1, 1));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(rColor.GetRed()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(rColor.GetGreen()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(255), sal_Int32(rColor.GetBlue()));
    }

    aBitmap.Convert(BMP_CONVERSION_24BIT);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(24), aBitmap.GetBitCount());
    {
        Bitmap::ScopedReadAccess pReadAccess(aBitmap);
#if defined LINUX
        // 24 bit Bitmap on SVP backend uses 32bit BGRA format
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(32), pReadAccess->GetBitCount());
        CPPUNIT_ASSERT_EQUAL(sal_uLong(40), pReadAccess->GetScanlineSize());
#else
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(24), pReadAccess->GetBitCount());
#if defined WNT
        if (!OpenGLHelper::isVCLOpenGLEnabled())
        {
            // GDI Scanlines padded to DWORD multiples, it seems
            CPPUNIT_ASSERT_EQUAL(sal_uLong(32), pReadAccess->GetScanlineSize());
        }
        else
#endif
        {
            CPPUNIT_ASSERT_EQUAL(sal_uLong(30), pReadAccess->GetScanlineSize());
        }
#endif
        CPPUNIT_ASSERT(!pReadAccess->HasPalette());
        Color aColor = pReadAccess->GetPixel(0, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(aColor.GetRed()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(aColor.GetGreen()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(255), sal_Int32(aColor.GetBlue()));
    }
}

void BitmapTest::testScale()
{
    const bool bExportBitmap(false);

    Bitmap aBitmap24Bit(Size(10, 10), 24);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(24), aBitmap24Bit.GetBitCount());

    {
        Bitmap::ScopedWriteAccess aWriteAccess(aBitmap24Bit);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(COL_BLACK);
        aWriteAccess->DrawRect(Rectangle(1, 1, 8, 8));
        aWriteAccess->DrawRect(Rectangle(3, 3, 6, 6));
    }

    BitmapSymmetryCheck aBitmapSymmetryCheck;

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(10), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(10), aBitmap24Bit.GetSizePixel().Height());

    // Check symmetry of the bitmap
    CPPUNIT_ASSERT(BitmapSymmetryCheck::check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream(OUString("~/scale_before.png"), StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream, 9);
    }

    aBitmap24Bit.Scale(2, 2, BmpScaleFlag::Fast);

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(20), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(20), aBitmap24Bit.GetSizePixel().Height());

    // After scaling the bitmap should still be symmetrical. This check guarantees that
    // scaling doesn't misalign the bitmap.
    CPPUNIT_ASSERT(BitmapSymmetryCheck::check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream(OUString("~/scale_after.png"), StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream, 9);
    }
}

typedef std::unordered_map<sal_uInt64, const char *> CRCHash;

void checkAndInsert(CRCHash &rHash, sal_uInt64 nCRC, const char *pLocation)
{
    auto it = rHash.find(nCRC);
    if (it != rHash.end()) {
        OStringBuffer aBuf("CRC collision between ");
        aBuf.append(pLocation);
        aBuf.append(" and ");
        aBuf.append(it->second);
        aBuf.append(" hash is 0x");
        aBuf.append((sal_Int64)nCRC, 16);
        CPPUNIT_FAIL(aBuf.toString().getStr());
    }
    rHash[nCRC] = pLocation;
}

void checkAndInsert(CRCHash &rHash, Bitmap rBmp, const char *pLocation)
{
    checkAndInsert(rHash, rBmp.GetChecksum(), pLocation);
}

Bitmap getAsBitmap(VclPtr<OutputDevice> pOut)
{
    return pOut->GetBitmap(Point(), pOut->GetOutputSizePixel());
}

void BitmapTest::testCRC()
{
    CRCHash aCRCs;

    Bitmap aBitmap(Size(1023,759), 24, nullptr);
    aBitmap.Erase(COL_BLACK);
    checkAndInsert(aCRCs, aBitmap, "black bitmap");
    aBitmap.Invert();
    checkAndInsert(aCRCs, aBitmap, "white bitmap");

    ScopedVclPtrInstance<VirtualDevice> aVDev;
    aVDev->SetBackground(Wallpaper(COL_WHITE));
    aVDev->SetOutputSizePixel(Size(1023, 759));

#if 0 // disabled for now - oddly breaks on OS/X - but why ?
    Bitmap aWhiteCheck = getAsBitmap(aVDev);
    CPPUNIT_ASSERT(aCRCs.find(aWhiteCheck.GetChecksum()) != aCRCs.end());
#endif

    // a 1x1 black & white checkerboard
    aVDev->DrawCheckered(Point(), aVDev->GetOutputSizePixel(), 1, 1);
    Bitmap aChecker = getAsBitmap(aVDev);
    checkAndInsert(aCRCs, aChecker, "checkerboard");
    aChecker.Invert();
    checkAndInsert(aCRCs, aChecker, "inverted checkerboard");
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
