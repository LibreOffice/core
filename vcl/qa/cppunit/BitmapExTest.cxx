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

#include <vcl/bitmapex.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

namespace
{
class BitmapExTest : public CppUnit::TestFixture
{
    void testGetPixelColor24_8();
    void testGetPixelColor32();
    void testTransformBitmapEx();
    void testAlphaBlendWith();
    void testCreateMask();
    void testCombineMaskOr();

    CPPUNIT_TEST_SUITE(BitmapExTest);
    CPPUNIT_TEST(testGetPixelColor24_8);
    CPPUNIT_TEST(testGetPixelColor32);
    CPPUNIT_TEST(testTransformBitmapEx);
    CPPUNIT_TEST(testAlphaBlendWith);
    CPPUNIT_TEST(testCreateMask);
    CPPUNIT_TEST(testCombineMaskOr);
    CPPUNIT_TEST_SUITE_END();
};

void BitmapExTest::testGetPixelColor24_8()
{
    Bitmap aBitmap(Size(3, 3), vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(Color(ColorTransparency, 0x00, 0x00, 0xFF, 0x00));
    }
    AlphaMask aMask(Size(3, 3));
    {
        BitmapScopedWriteAccess pWriteAccess(aMask);
        pWriteAccess->Erase(Color(ColorTransparency, 0x00, 0xAA, 0xAA, 0xAA));
    }

    BitmapEx aBitmapEx(aBitmap, aMask);

    CPPUNIT_ASSERT_EQUAL(Color(ColorAlpha, 0xAA, 0x00, 0xFF, 0x00), aBitmapEx.GetPixelColor(0, 0));
}

void BitmapExTest::testGetPixelColor32()
{
    // Check backend capabilities and return from the test successfully
    // if the backend doesn't support 32-bit bitmap
    if (!ImplGetSVData()->mpDefInst->supportsBitmap32())
        return;

    Bitmap aBitmap(Size(3, 3), vcl::PixelFormat::N32_BPP);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(Color(ColorTransparency, 0xAA, 0x00, 0xFF, 0x00));
    }

    BitmapEx aBitmapEx(aBitmap);

    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xAA, 0x00, 0xFF, 0x00),
                         aBitmapEx.GetPixelColor(0, 0));
}

void BitmapExTest::testTransformBitmapEx()
{
    Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(COL_WHITE);
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                pWriteAccess->SetPixel(i, j, COL_BLACK);
            }
        }
    }
    BitmapEx aBitmapEx(aBitmap);

    basegfx::B2DHomMatrix aMatrix;
    aMatrix.rotate(M_PI / 2);
    BitmapEx aTransformed = aBitmapEx.TransformBitmapEx(16, 16, aMatrix);
    aBitmap = aTransformed.GetBitmap();
    BitmapScopedReadAccess pAccess(aBitmap);
    for (int i = 0; i < 16; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            BitmapColor aColor = pAccess->GetPixel(i, j);
            std::stringstream ss;
            ss << "Color is expected to be white or black, is '" << aColor.AsRGBHexString() << "'";
            // Without the accompanying fix in place, this test would have failed with:
            // - Expression: aColor == COL_WHITE || aColor == COL_BLACK
            // - Color is expected to be white or black, is 'bfbfbf'
            // i.e. smoothing introduced noise for a simple 90 deg rotation.
            CPPUNIT_ASSERT_MESSAGE(ss.str(), aColor == COL_WHITE || aColor == COL_BLACK);
        }
    }
}

void BitmapExTest::testAlphaBlendWith()
{
    AlphaMask alpha(Size(1, 1));
    AlphaMask bitmap(Size(1, 1));

    // Just test a handful of combinations to make sure the algorithm doesn't
    // change (as happened when I did some 32-bit alpha changes)

    // Note that Erase() takes a transparency value, but we get alpha values in GetPixelIndex.

    alpha.Erase(64);
    bitmap.Erase(64);
    alpha.BlendWith(bitmap);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(255 - 112),
                         BitmapScopedReadAccess(alpha)->GetPixelIndex(0, 0));

    alpha.Erase(12);
    bitmap.Erase(64);
    alpha.BlendWith(bitmap);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(255 - 73),
                         BitmapScopedReadAccess(alpha)->GetPixelIndex(0, 0));

    alpha.Erase(12);
    bitmap.Erase(12);
    alpha.BlendWith(bitmap);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(255 - 24),
                         BitmapScopedReadAccess(alpha)->GetPixelIndex(0, 0));

    alpha.Erase(127);
    bitmap.Erase(13);
    alpha.BlendWith(bitmap);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(255 - 134),
                         BitmapScopedReadAccess(alpha)->GetPixelIndex(0, 0));

    alpha.Erase(255);
    bitmap.Erase(255);
    alpha.BlendWith(bitmap);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(255 - 255),
                         BitmapScopedReadAccess(alpha)->GetPixelIndex(0, 0));

    alpha.Erase(0);
    bitmap.Erase(255);
    alpha.BlendWith(bitmap);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(255 - 255),
                         BitmapScopedReadAccess(alpha)->GetPixelIndex(0, 0));

    alpha.Erase(255);
    bitmap.Erase(0);
    alpha.BlendWith(bitmap);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(255 - 255),
                         BitmapScopedReadAccess(alpha)->GetPixelIndex(0, 0));

    alpha.Erase(0);
    bitmap.Erase(0);
    alpha.BlendWith(bitmap);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(255 - 0),
                         BitmapScopedReadAccess(alpha)->GetPixelIndex(0, 0));
}

void BitmapExTest::testCreateMask()
{
    Bitmap aBitmap(Size(3, 3), vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(COL_WHITE);
        for (int i = 0; i < 3; ++i)
            pWriteAccess->SetPixel(i, i, COL_RED);
    }
    aBitmap = aBitmap.CreateMask(COL_RED, 1);
    BitmapScopedReadAccess pAccess(aBitmap);
    // the output is a greyscale palette bitmap
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xff), pAccess->GetPixelIndex(0, 0));
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pAccess->GetPixelIndex(0, 1));
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pAccess->GetPixelIndex(0, 2));
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pAccess->GetPixelIndex(1, 0));
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xff), pAccess->GetPixelIndex(1, 1));
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pAccess->GetPixelIndex(1, 2));
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pAccess->GetPixelIndex(2, 0));
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pAccess->GetPixelIndex(2, 1));
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xff), pAccess->GetPixelIndex(2, 2));
}

void BitmapExTest::testCombineMaskOr()
{
    Bitmap aBitmap(Size(3, 3), vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(COL_WHITE);
        for (int i = 0; i < 3; ++i)
            pWriteAccess->SetPixel(1, i, COL_RED);
    }
    AlphaMask aAlphaBitmap(Size(3, 3));
    {
        BitmapScopedWriteAccess pWriteAccess(aAlphaBitmap);
        pWriteAccess->Erase(COL_WHITE);
        for (int i = 1; i < 3; ++i)
        {
            pWriteAccess->SetPixel(i, 0, COL_BLACK);
            pWriteAccess->SetPixel(i, 1, COL_GRAY);
            pWriteAccess->SetPixel(i, 0, Color(0xef, 0xef, 0xef));
        }
    }

    {
        AlphaMask aMask = aBitmap.CreateAlphaMask(COL_RED, 1);
        BitmapScopedReadAccess pAccess(aMask);
        // the output is a greyscale palette bitmap
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xff), pAccess->GetPixelIndex(0, 0));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xff), pAccess->GetPixelIndex(0, 1));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xff), pAccess->GetPixelIndex(0, 2));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pAccess->GetPixelIndex(1, 0));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pAccess->GetPixelIndex(1, 1));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x00), pAccess->GetPixelIndex(1, 2));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xff), pAccess->GetPixelIndex(2, 0));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xff), pAccess->GetPixelIndex(2, 1));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xff), pAccess->GetPixelIndex(2, 2));
    }

    BitmapEx aBitmapEx(aBitmap, aAlphaBitmap);
    aBitmapEx.CombineMaskOr(COL_RED, 1);

    CPPUNIT_ASSERT_EQUAL(Color(ColorAlpha, 0xff, 0xff, 0xff, 0xff), aBitmapEx.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(Color(ColorAlpha, 0x00, 0x80, 0x00, 0x00), aBitmapEx.GetPixelColor(0, 1));
    CPPUNIT_ASSERT_EQUAL(Color(ColorAlpha, 0x00, 0xff, 0xff, 0xff), aBitmapEx.GetPixelColor(0, 2));
    CPPUNIT_ASSERT_EQUAL(Color(ColorAlpha, 0xff, 0xff, 0xff, 0xff), aBitmapEx.GetPixelColor(1, 0));
    CPPUNIT_ASSERT_EQUAL(Color(ColorAlpha, 0x00, 0x80, 0x00, 0x00), aBitmapEx.GetPixelColor(1, 1));
    CPPUNIT_ASSERT_EQUAL(Color(ColorAlpha, 0x00, 0xff, 0xff, 0xff), aBitmapEx.GetPixelColor(1, 2));
    CPPUNIT_ASSERT_EQUAL(Color(ColorAlpha, 0xff, 0xff, 0xff, 0xff), aBitmapEx.GetPixelColor(2, 0));
    CPPUNIT_ASSERT_EQUAL(Color(ColorAlpha, 0x00, 0x80, 0x00, 0x00), aBitmapEx.GetPixelColor(2, 1));
    CPPUNIT_ASSERT_EQUAL(Color(ColorAlpha, 0xff, 0xff, 0xff, 0xff), aBitmapEx.GetPixelColor(2, 2));
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapExTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
