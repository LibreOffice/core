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
#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLHelper.hxx>
#endif
#include <vcl/BitmapMonochromeFilter.hxx>

#include <BitmapSymmetryCheck.hxx>
#include <bitmapwriteaccess.hxx>

namespace
{
class BitmapTest : public CppUnit::TestFixture
{
    void testCreation();
    void testEmpty();
    void testMonochrome();
    void testN4Greyscale();
    void testN8Greyscale();
    void testConvert();
    void testScale();
    void testCRC();
    void testGreyPalette();

    CPPUNIT_TEST_SUITE(BitmapTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testMonochrome);
    CPPUNIT_TEST(testConvert);
    CPPUNIT_TEST(testN4Greyscale);
    CPPUNIT_TEST(testN8Greyscale);
    CPPUNIT_TEST(testScale);
    CPPUNIT_TEST(testCRC);
    CPPUNIT_TEST(testGreyPalette);
    CPPUNIT_TEST_SUITE_END();
};

void BitmapTest::testCreation()
{
    {
        Bitmap aBmp;
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<long>(0), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<long>(0), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Not empty", aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(0),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", static_cast<sal_uLong>(1),
                                     aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(0),
                                     aBmp.GetSizeBytes());
    }

    {
        Bitmap aBmp(Size(10, 10), 1);
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<long>(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<long>(10), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Empty bitmap", !aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(1),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", static_cast<sal_uLong>(2),
                                     aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(12),
                                     aBmp.GetSizeBytes());
    }

    {
        Bitmap aBmp(Size(10, 10), 4);
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<long>(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<long>(10), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Empty bitmap", !aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(4),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", static_cast<sal_uLong>(16),
                                     aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(50),
                                     aBmp.GetSizeBytes());
    }

    {
        Bitmap aBmp(Size(10, 10), 8);
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<long>(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<long>(10), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Empty bitmap", !aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(8),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", static_cast<sal_uLong>(256),
                                     aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(100),
                                     aBmp.GetSizeBytes());
    }

    {
        Bitmap aBmp(Size(10, 10), 16);
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<long>(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<long>(10), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Empty bitmap", !aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(24),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", static_cast<sal_uLong>(16777216),
                                     aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(300),
                                     aBmp.GetSizeBytes());
    }

    {
        Bitmap aBmp(Size(10, 10), 24);
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<long>(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<long>(10), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Empty bitmap", !aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(24),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", static_cast<sal_uLong>(16777216),
                                     aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(300),
                                     aBmp.GetSizeBytes());
    }

    {
        Bitmap aBmp(Size(10, 10), 32);
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<long>(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<long>(10), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Empty bitmap", !aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(24),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", static_cast<sal_uLong>(16777216),
                                     aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(300),
                                     aBmp.GetSizeBytes());
    }
}

void BitmapTest::testEmpty()
{
    Bitmap aBitmap(Size(10, 10), 8);
    aBitmap.Erase(COL_LIGHTGRAYBLUE);

    CPPUNIT_ASSERT(!aBitmap.IsEmpty());

    aBitmap.SetEmpty();
    CPPUNIT_ASSERT(aBitmap.IsEmpty());
}

const Bitmap createTestBitmap()
{
    Bitmap aBmp(Size(4, 4), 24);
    BitmapWriteAccess aBmpAccess(aBmp);

    // row 1
    aBmpAccess.SetPixel(0, 0, BitmapColor(COL_BLACK));
    aBmpAccess.SetPixel(0, 1, BitmapColor(COL_BLUE));
    aBmpAccess.SetPixel(0, 2, BitmapColor(COL_GREEN));
    aBmpAccess.SetPixel(0, 3, BitmapColor(COL_CYAN));

    // row 2
    aBmpAccess.SetPixel(1, 0, BitmapColor(COL_RED));
    aBmpAccess.SetPixel(1, 1, BitmapColor(COL_MAGENTA));
    aBmpAccess.SetPixel(1, 2, BitmapColor(COL_BROWN));
    aBmpAccess.SetPixel(1, 3, BitmapColor(COL_GRAY));

    // row 3
    aBmpAccess.SetPixel(2, 0, BitmapColor(COL_LIGHTGRAY));
    aBmpAccess.SetPixel(2, 1, BitmapColor(COL_LIGHTBLUE));
    aBmpAccess.SetPixel(2, 2, BitmapColor(COL_LIGHTGREEN));
    aBmpAccess.SetPixel(2, 3, BitmapColor(COL_LIGHTCYAN));

    // row 4
    aBmpAccess.SetPixel(3, 0, BitmapColor(COL_LIGHTRED));
    aBmpAccess.SetPixel(3, 1, BitmapColor(COL_LIGHTMAGENTA));
    aBmpAccess.SetPixel(3, 2, BitmapColor(COL_YELLOW));
    aBmpAccess.SetPixel(3, 3, BitmapColor(COL_WHITE));

    return aBmp;
}

void BitmapTest::testMonochrome()
{
    Bitmap aBmp = createTestBitmap();

    BitmapEx aBmpEx(aBmp);
    BitmapFilter::Filter(aBmpEx, BitmapMonochromeFilter(63));
    aBmp = aBmpEx.GetBitmap();
    BitmapReadAccess aBmpReadAccess(aBmp);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Black pixel wrong monochrome value", BitmapColor(COL_BLACK),
                                 aBmpReadAccess.GetColor(0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue pixel wrong monochrome value", BitmapColor(COL_BLACK),
                                 aBmpReadAccess.GetColor(0, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Green pixel wrong monochrome value", BitmapColor(COL_WHITE),
                                 aBmpReadAccess.GetColor(0, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cyan pixel wrong monochrome value", BitmapColor(COL_WHITE),
                                 aBmpReadAccess.GetColor(0, 3));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Red pixel wrong monochrome value", BitmapColor(COL_BLACK),
                                 aBmpReadAccess.GetColor(1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Magenta pixel wrong monochrome value", BitmapColor(COL_BLACK),
                                 aBmpReadAccess.GetColor(1, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Brown pixel wrong monochrome value", BitmapColor(COL_WHITE),
                                 aBmpReadAccess.GetColor(1, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Gray pixel wrong monochrome value", BitmapColor(COL_WHITE),
                                 aBmpReadAccess.GetColor(1, 3));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light gray pixel wrong monochrome value", BitmapColor(COL_WHITE),
                                 aBmpReadAccess.GetColor(2, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light blue pixel wrong monochrome value", BitmapColor(COL_BLACK),
                                 aBmpReadAccess.GetColor(2, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light green pixel wrong monochrome value", BitmapColor(COL_WHITE),
                                 aBmpReadAccess.GetColor(2, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light cyan pixel wrong monochrome value", BitmapColor(COL_WHITE),
                                 aBmpReadAccess.GetColor(2, 3));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light red pixel wrong monochrome value", BitmapColor(COL_WHITE),
                                 aBmpReadAccess.GetColor(3, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light magenta pixel wrong monochrome value",
                                 BitmapColor(COL_WHITE), aBmpReadAccess.GetColor(3, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Yellow pixel wrong monochrome value", BitmapColor(COL_WHITE),
                                 aBmpReadAccess.GetColor(3, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("White pixel wrong monochrome value", BitmapColor(COL_WHITE),
                                 aBmpReadAccess.GetColor(3, 3));
}

void BitmapTest::testN4Greyscale()
{
    Bitmap aBmp = createTestBitmap();
    BitmapPalette aGreyscalePalette = Bitmap::GetGreyPalette(16);

    aBmp.Convert(BmpConversion::N4BitGreys);
    BitmapReadAccess aBmpReadAccess(aBmp);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Black pixel wrong 8-bit greyscale value", aGreyscalePalette[0],
                                 aBmpReadAccess.GetColor(0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue pixel wrong 8-bit greyscale value", aGreyscalePalette[0],
                                 aBmpReadAccess.GetColor(0, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Green pixel wrong 8-bit greyscale value", aGreyscalePalette[4],
                                 aBmpReadAccess.GetColor(0, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cyan pixel wrong 8-bit greyscale value", aGreyscalePalette[5],
                                 aBmpReadAccess.GetColor(0, 3));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Red pixel wrong 8-bit greyscale value", aGreyscalePalette[2],
                                 aBmpReadAccess.GetColor(1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Magenta pixel wrong 8-bit greyscale value", aGreyscalePalette[3],
                                 aBmpReadAccess.GetColor(1, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Brown pixel wrong 8-bit greyscale value", aGreyscalePalette[7],
                                 aBmpReadAccess.GetColor(1, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Gray pixel wrong 8-bit greyscale value", aGreyscalePalette[8],
                                 aBmpReadAccess.GetColor(1, 3));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light gray pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[12], aBmpReadAccess.GetColor(2, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light blue pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[1], aBmpReadAccess.GetColor(2, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light green pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[9], aBmpReadAccess.GetColor(2, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light cyan pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[11], aBmpReadAccess.GetColor(2, 3));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light red pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[4], aBmpReadAccess.GetColor(3, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light magenta pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[6], aBmpReadAccess.GetColor(3, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Yellow pixel wrong 8-bit greyscale value", aGreyscalePalette[14],
                                 aBmpReadAccess.GetColor(3, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("White pixel wrong 8-bit greyscale value", aGreyscalePalette[15],
                                 aBmpReadAccess.GetColor(3, 3));
}

void BitmapTest::testN8Greyscale()
{
    Bitmap aBmp = createTestBitmap();
    BitmapPalette aGreyscalePalette = Bitmap::GetGreyPalette(256);

    aBmp.Convert(BmpConversion::N8BitGreys);
    BitmapReadAccess aBmpReadAccess(aBmp);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Black pixel wrong 8-bit greyscale value", aGreyscalePalette[0],
                                 aBmpReadAccess.GetColor(0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Blue pixel wrong 8-bit greyscale value", aGreyscalePalette[14],
                                 aBmpReadAccess.GetColor(0, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Green pixel wrong 8-bit greyscale value", aGreyscalePalette[75],
                                 aBmpReadAccess.GetColor(0, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cyan pixel wrong 8-bit greyscale value", aGreyscalePalette[89],
                                 aBmpReadAccess.GetColor(0, 3));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Red pixel wrong 8-bit greyscale value", aGreyscalePalette[38],
                                 aBmpReadAccess.GetColor(1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Magenta pixel wrong 8-bit greyscale value", aGreyscalePalette[52],
                                 aBmpReadAccess.GetColor(1, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Brown pixel wrong 8-bit greyscale value", aGreyscalePalette[114],
                                 aBmpReadAccess.GetColor(1, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Gray pixel wrong 8-bit greyscale value", aGreyscalePalette[128],
                                 aBmpReadAccess.GetColor(1, 3));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light gray pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[192], aBmpReadAccess.GetColor(2, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light blue pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[27], aBmpReadAccess.GetColor(2, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light green pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[150], aBmpReadAccess.GetColor(2, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light cyan pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[178], aBmpReadAccess.GetColor(2, 3));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light red pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[76], aBmpReadAccess.GetColor(3, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Light magenta pixel wrong 8-bit greyscale value",
                                 aGreyscalePalette[104], aBmpReadAccess.GetColor(3, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Yellow pixel wrong 8-bit greyscale value", aGreyscalePalette[227],
                                 aBmpReadAccess.GetColor(3, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("White pixel wrong 8-bit greyscale value", aGreyscalePalette[255],
                                 aBmpReadAccess.GetColor(3, 3));
}

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
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(10), pReadAccess->GetScanlineSize());
#else
#if HAVE_FEATURE_OPENGL
        if (!OpenGLHelper::isVCLOpenGLEnabled())
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(12), pReadAccess->GetScanlineSize());
#endif
#endif
        CPPUNIT_ASSERT(pReadAccess->HasPalette());
        const BitmapColor& rColor = pReadAccess->GetPaletteColor(pReadAccess->GetPixelIndex(1, 1));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(rColor.GetRed()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(rColor.GetGreen()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(255), sal_Int32(rColor.GetBlue()));
    }

    aBitmap.Convert(BmpConversion::N24Bit);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(24), aBitmap.GetBitCount());
    {
        Bitmap::ScopedReadAccess pReadAccess(aBitmap);
        // 24 bit Bitmap on SVP backend can now use 24bit RGB everywhere.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(24), pReadAccess->GetBitCount());

#if defined LINUX || defined FREEBSD
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(32), pReadAccess->GetScanlineSize());
#else
#if defined(_WIN32)
        if (!OpenGLHelper::isVCLOpenGLEnabled())
        {
            // GDI Scanlines padded to DWORD multiples, it seems
            CPPUNIT_ASSERT_EQUAL(sal_uInt32(32), pReadAccess->GetScanlineSize());
        }
        else
#endif
        {
            CPPUNIT_ASSERT_EQUAL(sal_uInt32(30), pReadAccess->GetScanlineSize());
        }
#endif

        CPPUNIT_ASSERT(!pReadAccess->HasPalette());
        Color aColor = pReadAccess->GetPixel(0, 0).GetColor();
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
        BitmapScopedWriteAccess aWriteAccess(aBitmap24Bit);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(COL_BLACK);
        aWriteAccess->DrawRect(tools::Rectangle(1, 1, 8, 8));
        aWriteAccess->DrawRect(tools::Rectangle(3, 3, 6, 6));
    }

    BitmapSymmetryCheck aBitmapSymmetryCheck;

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(10), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(10), aBitmap24Bit.GetSizePixel().Height());

    // Check symmetry of the bitmap
    CPPUNIT_ASSERT(BitmapSymmetryCheck::check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream("~/scale_before.png", StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream);
    }

    aBitmap24Bit.Scale(2, 2, BmpScaleFlag::Fast);

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(20), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(20), aBitmap24Bit.GetSizePixel().Height());

    // After scaling the bitmap should still be symmetrical. This check guarantees that
    // scaling doesn't misalign the bitmap.
    CPPUNIT_ASSERT(BitmapSymmetryCheck::check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream("~/scale_after.png", StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream);
    }
}

typedef std::unordered_map<sal_uInt64, const char*> CRCHash;

void checkAndInsert(CRCHash& rHash, sal_uInt64 nCRC, const char* pLocation)
{
    auto it = rHash.find(nCRC);
    if (it != rHash.end())
    {
        OStringBuffer aBuf("CRC collision between ");
        aBuf.append(pLocation);
        aBuf.append(" and ");
        aBuf.append(it->second);
        aBuf.append(" hash is 0x");
        aBuf.append(static_cast<sal_Int64>(nCRC), 16);
        CPPUNIT_FAIL(aBuf.toString().getStr());
    }
    rHash[nCRC] = pLocation;
}

void checkAndInsert(CRCHash& rHash, Bitmap const& rBmp, const char* pLocation)
{
    checkAndInsert(rHash, rBmp.GetChecksum(), pLocation);
}

Bitmap getAsBitmap(VclPtr<OutputDevice> const& pOut)
{
    return pOut->GetBitmap(Point(), pOut->GetOutputSizePixel());
}

void BitmapTest::testCRC()
{
    CRCHash aCRCs;

    Bitmap aBitmap(Size(1023, 759), 24, nullptr);
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
    aVDev->DrawCheckered(Point(), aVDev->GetOutputSizePixel(), 1, Color(0, 0, 1));
    Bitmap aChecker = getAsBitmap(aVDev);
    checkAndInsert(aCRCs, aChecker, "checkerboard");
    aChecker.Invert();
    checkAndInsert(aCRCs, aChecker, "inverted checkerboard");
}

void BitmapTest::testGreyPalette()
{
    {
        BitmapPalette aPalette = Bitmap::GetGreyPalette(2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong number of palette entries", static_cast<sal_uInt16>(2),
                                     aPalette.GetEntryCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 1 wrong", BitmapColor(0, 0, 0), aPalette[0]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 2 wrong", BitmapColor(255, 255, 255), aPalette[1]);
    }

    {
        BitmapPalette aPalette = Bitmap::GetGreyPalette(4);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong number of palette entries", static_cast<sal_uInt16>(4),
                                     aPalette.GetEntryCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 1 wrong", BitmapColor(0, 0, 0), aPalette[0]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 2 wrong", BitmapColor(85, 85, 85), aPalette[1]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 3 wrong", BitmapColor(170, 170, 170), aPalette[2]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 4 wrong", BitmapColor(255, 255, 255), aPalette[3]);
    }

    {
        BitmapPalette aPalette = Bitmap::GetGreyPalette(16);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong number of palette entries", static_cast<sal_uInt16>(16),
                                     aPalette.GetEntryCount());
        // this is a *reall* specific number of greys, incremented in units of 17 so may
        // as well test them all...
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 1 wrong", BitmapColor(0, 0, 0), aPalette[0]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 2 wrong", BitmapColor(17, 17, 17), aPalette[1]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 3 wrong", BitmapColor(34, 34, 34), aPalette[2]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 4 wrong", BitmapColor(51, 51, 51), aPalette[3]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 5 wrong", BitmapColor(68, 68, 68), aPalette[4]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 6 wrong", BitmapColor(85, 85, 85), aPalette[5]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 7 wrong", BitmapColor(102, 102, 102), aPalette[6]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 8 wrong", BitmapColor(119, 119, 119), aPalette[7]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 9 wrong", BitmapColor(136, 136, 136), aPalette[8]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 10 wrong", BitmapColor(153, 153, 153), aPalette[9]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 11 wrong", BitmapColor(170, 170, 170), aPalette[10]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 12 wrong", BitmapColor(187, 187, 187), aPalette[11]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 13 wrong", BitmapColor(204, 204, 204), aPalette[12]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 14 wrong", BitmapColor(221, 221, 221), aPalette[13]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 15 wrong", BitmapColor(238, 238, 238), aPalette[14]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 16 wrong", BitmapColor(255, 255, 255), aPalette[15]);
    }

    {
        BitmapPalette aPalette = Bitmap::GetGreyPalette(256);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong number of palette entries",
                                     static_cast<sal_uInt16>(256), aPalette.GetEntryCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 1 wrong", BitmapColor(0, 0, 0), aPalette[0]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 127 wrong", BitmapColor(127, 127, 127), aPalette[127]);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Entry 255 wrong", BitmapColor(255, 255, 255), aPalette[255]);
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
