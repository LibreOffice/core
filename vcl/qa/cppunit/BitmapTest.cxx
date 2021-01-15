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
#include <vcl/virdev.hxx>

#include <rtl/strbuf.hxx>
#include <config_features.h>
#include <vcl/skia/SkiaHelper.hxx>
#include <vcl/BitmapMonochromeFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>

#include <svdata.hxx>
#include <salinst.hxx>
#include <bitmap/Octree.hxx>

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
    void testCRC();
    void testGreyPalette();
    void testCustom8BitPalette();
    void testErase();
    void testBitmap32();
    void testOctree();
    void testEmptyAccess();
    void testDitherSize();

    CPPUNIT_TEST_SUITE(BitmapTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testMonochrome);
    CPPUNIT_TEST(testConvert);
    CPPUNIT_TEST(testN4Greyscale);
    CPPUNIT_TEST(testN8Greyscale);
    CPPUNIT_TEST(testCRC);
    CPPUNIT_TEST(testGreyPalette);
    CPPUNIT_TEST(testCustom8BitPalette);
    CPPUNIT_TEST(testErase);
    CPPUNIT_TEST(testBitmap32);
    CPPUNIT_TEST(testOctree);
    CPPUNIT_TEST(testEmptyAccess);
    CPPUNIT_TEST(testDitherSize);
    CPPUNIT_TEST_SUITE_END();
};

void assertColorsAreSimilar(int maxDifference, const std::string& message,
                            const BitmapColor& expected, const BitmapColor& actual)
{
    // Check that the two colors match or are reasonably similar.
    if (expected == actual)
        return;
    if (abs(expected.GetRed() - actual.GetRed()) <= maxDifference
        && abs(expected.GetGreen() - actual.GetGreen()) <= maxDifference
        && abs(expected.GetBlue() - actual.GetBlue()) <= maxDifference
        && abs(expected.GetAlpha() - actual.GetAlpha()) <= maxDifference)
    {
        return;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expected, actual);
}

void BitmapTest::testCreation()
{
    {
        Bitmap aBmp;
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<tools::Long>(0), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<tools::Long>(0), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Not empty", aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(0),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", sal_Int64(1), aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(0),
                                     aBmp.GetSizeBytes());
    }

    {
        Bitmap aBmp(Size(10, 10), 1);
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<tools::Long>(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<tools::Long>(10), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Empty bitmap", !aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(1),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", sal_Int64(2), aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(12),
                                     aBmp.GetSizeBytes());
    }

    {
        Bitmap aBmp(Size(10, 10), 4);
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<tools::Long>(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<tools::Long>(10), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Empty bitmap", !aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(4),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", sal_Int64(16), aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(50),
                                     aBmp.GetSizeBytes());
    }

    {
        Bitmap aBmp(Size(10, 10), 8);
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<tools::Long>(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<tools::Long>(10), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Empty bitmap", !aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(8),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", sal_Int64(256), aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(100),
                                     aBmp.GetSizeBytes());
    }

    {
        Bitmap aBmp(Size(10, 10), 24);
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<tools::Long>(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<tools::Long>(10), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Empty bitmap", !aBmp.IsEmpty());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", static_cast<sal_uInt16>(24),
                                     aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", sal_Int64(16777216),
                                     aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", static_cast<sal_uLong>(300),
                                     aBmp.GetSizeBytes());
    }

    // Check backend capabilities and return from the test successfully
    // if the backend doesn't support 32-bit bitmap
    auto pBackendCapabilities = ImplGetSVData()->mpDefInst->GetBackendCapabilities();
    if (pBackendCapabilities->mbSupportsBitmap32)
    {
        Bitmap aBmp(Size(10, 10), 32);
        Size aSize = aBmp.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong width", static_cast<tools::Long>(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong height", static_cast<tools::Long>(10), aSize.Height());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong pref size", Size(), aBmp.GetPrefSize());
        CPPUNIT_ASSERT_MESSAGE("Empty bitmap", !aBmp.IsEmpty());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bit count", sal_uInt16(32), aBmp.GetBitCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong color count", sal_Int64(4294967296ull),
                                     aBmp.GetColorCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong byte size", sal_uLong(400), aBmp.GetSizeBytes());
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

Bitmap createTestBitmap()
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

    assertColorsAreSimilar(1, "Black pixel wrong 8-bit greyscale value", aGreyscalePalette[0],
                           aBmpReadAccess.GetColor(0, 0));
    assertColorsAreSimilar(1, "Blue pixel wrong 8-bit greyscale value", aGreyscalePalette[14],
                           aBmpReadAccess.GetColor(0, 1));
    assertColorsAreSimilar(1, "Green pixel wrong 8-bit greyscale value", aGreyscalePalette[75],
                           aBmpReadAccess.GetColor(0, 2));
    assertColorsAreSimilar(1, "Cyan pixel wrong 8-bit greyscale value", aGreyscalePalette[89],
                           aBmpReadAccess.GetColor(0, 3));
    assertColorsAreSimilar(1, "Red pixel wrong 8-bit greyscale value", aGreyscalePalette[38],
                           aBmpReadAccess.GetColor(1, 0));
    assertColorsAreSimilar(1, "Magenta pixel wrong 8-bit greyscale value", aGreyscalePalette[52],
                           aBmpReadAccess.GetColor(1, 1));
    assertColorsAreSimilar(1, "Brown pixel wrong 8-bit greyscale value", aGreyscalePalette[114],
                           aBmpReadAccess.GetColor(1, 2));
    assertColorsAreSimilar(1, "Gray pixel wrong 8-bit greyscale value", aGreyscalePalette[128],
                           aBmpReadAccess.GetColor(1, 3));
    assertColorsAreSimilar(1, "Light gray pixel wrong 8-bit greyscale value",
                           aGreyscalePalette[192], aBmpReadAccess.GetColor(2, 0));
    assertColorsAreSimilar(1, "Light blue pixel wrong 8-bit greyscale value", aGreyscalePalette[27],
                           aBmpReadAccess.GetColor(2, 1));
    assertColorsAreSimilar(1, "Light green pixel wrong 8-bit greyscale value",
                           aGreyscalePalette[150], aBmpReadAccess.GetColor(2, 2));
    assertColorsAreSimilar(1, "Light cyan pixel wrong 8-bit greyscale value",
                           aGreyscalePalette[178], aBmpReadAccess.GetColor(2, 3));
    assertColorsAreSimilar(1, "Light red pixel wrong 8-bit greyscale value", aGreyscalePalette[76],
                           aBmpReadAccess.GetColor(3, 0));
    assertColorsAreSimilar(1, "Light magenta pixel wrong 8-bit greyscale value",
                           aGreyscalePalette[104], aBmpReadAccess.GetColor(3, 1));
    assertColorsAreSimilar(1, "Yellow pixel wrong 8-bit greyscale value", aGreyscalePalette[227],
                           aBmpReadAccess.GetColor(3, 2));
    assertColorsAreSimilar(1, "White pixel wrong 8-bit greyscale value", aGreyscalePalette[255],
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
        if (!SkiaHelper::isVCLSkiaEnabled())
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(12), pReadAccess->GetScanlineSize());
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

        if (SkiaHelper::isVCLSkiaEnabled()) // aligned to 4 bytes
            CPPUNIT_ASSERT_EQUAL(sal_uInt32(32), pReadAccess->GetScanlineSize());
        else
#if defined LINUX || defined FREEBSD
        {
            CPPUNIT_ASSERT_EQUAL(sal_uInt32(32), pReadAccess->GetScanlineSize());
        }
#elif defined(_WIN32)
        {
            // GDI Scanlines padded to DWORD multiples, it seems
            CPPUNIT_ASSERT_EQUAL(sal_uInt32(32), pReadAccess->GetScanlineSize());
        }
#else
        {
            CPPUNIT_ASSERT_EQUAL(sal_uInt32(30), pReadAccess->GetScanlineSize());
        }
#endif

        CPPUNIT_ASSERT(!pReadAccess->HasPalette());
        Color aColor = pReadAccess->GetPixel(0, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(aColor.GetRed()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(aColor.GetGreen()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(255), sal_Int32(aColor.GetBlue()));
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
        // this is a *real* specific number of greys, incremented in units of 17 so may
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

void BitmapTest::testCustom8BitPalette()
{
    BitmapPalette aCustomPalette;
    aCustomPalette.SetEntryCount(256);
    for (sal_uInt16 i = 0; i < 256; i++)
    {
        aCustomPalette[i] = BitmapColor(sal_uInt8(i), sal_uInt8(0xCC), sal_uInt8(0x22));
    }
    Bitmap aBitmap(Size(3, 2), 8, &aCustomPalette);

    {
        BitmapScopedWriteAccess pAccess(aBitmap);
        pAccess->SetPixelIndex(0, 0, 0);
        pAccess->SetPixelIndex(0, 1, 1);
        pAccess->SetPixelIndex(0, 2, 2);

        pAccess->SetPixelIndex(1, 0, 253);
        pAccess->SetPixelIndex(1, 1, 254);
        pAccess->SetPixelIndex(1, 2, 255);
    }

    {
        Bitmap::ScopedReadAccess pAccess(aBitmap);
        CPPUNIT_ASSERT_EQUAL(0, int(pAccess->GetPixelIndex(0, 0)));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0xCC, 0x22), pAccess->GetColor(0, 0));

        CPPUNIT_ASSERT_EQUAL(1, int(pAccess->GetPixelIndex(0, 1)));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0x01, 0xCC, 0x22), pAccess->GetColor(0, 1));

        CPPUNIT_ASSERT_EQUAL(2, int(pAccess->GetPixelIndex(0, 2)));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0x02, 0xCC, 0x22), pAccess->GetColor(0, 2));

        CPPUNIT_ASSERT_EQUAL(253, int(pAccess->GetPixelIndex(1, 0)));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFD, 0xCC, 0x22), pAccess->GetColor(1, 0));

        CPPUNIT_ASSERT_EQUAL(254, int(pAccess->GetPixelIndex(1, 1)));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFE, 0xCC, 0x22), pAccess->GetColor(1, 1));

        CPPUNIT_ASSERT_EQUAL(255, int(pAccess->GetPixelIndex(1, 2)));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xCC, 0x22), pAccess->GetColor(1, 2));
    }
}

void BitmapTest::testErase()
{
    Bitmap aBitmap(Size(3, 3), 24);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(Color(0x11, 0x22, 0x33));
    }
    {
        Bitmap::ScopedReadAccess pReadAccess(aBitmap);
        BitmapColor aColor(pReadAccess->GetPixel(0, 0));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x11, 0x22, 0x33, 0x00), aColor);
    }
}

void BitmapTest::testBitmap32()
{
    // Check backend capabilities and return from the test successfully
    // if the backend doesn't support 32-bit bitmap
    auto pBackendCapabilities = ImplGetSVData()->mpDefInst->GetBackendCapabilities();
    if (!pBackendCapabilities->mbSupportsBitmap32)
        return;

    Bitmap aBitmap(Size(3, 3), 32);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(Color(ColorTransparency, 0xFF, 0x11, 0x22, 0x33));
        pWriteAccess->SetPixel(1, 1, BitmapColor(ColorTransparency, 0x44, 0xFF, 0xBB, 0x00));
        pWriteAccess->SetPixel(2, 2, BitmapColor(ColorTransparency, 0x99, 0x77, 0x66, 0x55));
    }
    {
        Bitmap::ScopedReadAccess pReadAccess(aBitmap);
        BitmapColor aColor = pReadAccess->GetPixel(0, 0);
        CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x00, 0xFF), aColor);

        aColor = pReadAccess->GetPixel(1, 1);
        CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x44, 0xFF, 0xBB, 0x00), aColor);

        aColor = pReadAccess->GetPixel(2, 2);
        CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x99, 0x77, 0x66, 0x55), aColor);
    }
}

void BitmapTest::testOctree()
{
    Size aSize(1000, 100);
    Bitmap aBitmap(aSize, 24);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        for (tools::Long y = 0; y < aSize.Height(); ++y)
        {
            for (tools::Long x = 0; x < aSize.Width(); ++x)
            {
                double fPercent = double(x) / double(aSize.Width());
                pWriteAccess->SetPixel(y, x,
                                       BitmapColor(255.0 * fPercent, 64.0 + (128.0 * fPercent),
                                                   255.0 - 255.0 * fPercent));
            }
        }
    }

    {
        // Reduce to 1 color
        Bitmap::ScopedReadAccess pAccess(aBitmap);
        Octree aOctree(*pAccess, 1);
        auto aBitmapPalette = aOctree.GetPalette();
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), aBitmapPalette.GetEntryCount());
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0x7e, 0x7f, 0x7f), aBitmapPalette[0]);
    }

    {
        // Reduce to 4 color
        Bitmap::ScopedReadAccess pAccess(aBitmap);
        Octree aOctree(*pAccess, 4);
        auto aBitmapPalette = aOctree.GetPalette();
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), aBitmapPalette.GetEntryCount());
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0x7f, 0x7f, 0x7f), aBitmapPalette[0]);
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0x3e, 0x5f, 0xbf), aBitmapPalette[1]);
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0x7f, 0x80, 0x7f), aBitmapPalette[2]);
        CPPUNIT_ASSERT_EQUAL(BitmapColor(0xbe, 0x9f, 0x3f), aBitmapPalette[3]);
    }

    {
        // Reduce to 256 color
        Bitmap::ScopedReadAccess pAccess(aBitmap);
        Octree aOctree(*pAccess, 256);
        auto aBitmapPalette = aOctree.GetPalette();
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(74), aBitmapPalette.GetEntryCount());
    }
}

void BitmapTest::testEmptyAccess()
{
    Bitmap empty;
    BitmapInfoAccess access(empty);
    CPPUNIT_ASSERT_EQUAL(tools::Long(0), access.Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(0), access.Height());
}

void BitmapTest::testDitherSize()
{
    // no need to do anything for a 1x1 pixel bitmap
    {
        Bitmap aBitmap(Size(1, 1), 24);
        CPPUNIT_ASSERT(aBitmap.Dither());
    }

    // cannot dither a bitmap with a width of 2 or 3 pixels
    {
        Bitmap aBitmap(Size(2, 4), 24);
        CPPUNIT_ASSERT(!aBitmap.Dither());
    }

    {
        Bitmap aBitmap(Size(3, 4), 24);
        CPPUNIT_ASSERT(!aBitmap.Dither());
    }

    // cannot dither a bitmap with a height of 2 pixels
    {
        Bitmap aBitmap(Size(4, 2), 24);
        CPPUNIT_ASSERT(!aBitmap.Dither());
    }

    // only dither bitmaps with a width > 3 pixels and height > 2 pixels
    {
        Bitmap aBitmap(Size(4, 3), 24);
        CPPUNIT_ASSERT(aBitmap.Dither());
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
