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
#include <bitmap/BitmapWriteAccess.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

namespace
{
class BitmapExTest : public CppUnit::TestFixture
{
    void testGetPixelColor24_8();
    void testGetPixelColor32();
    void testTransformBitmapEx();

    CPPUNIT_TEST_SUITE(BitmapExTest);
    CPPUNIT_TEST(testGetPixelColor24_8);
    CPPUNIT_TEST(testGetPixelColor32);
    CPPUNIT_TEST(testTransformBitmapEx);
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
        AlphaScopedWriteAccess pWriteAccess(aMask);
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
    Bitmap::ScopedReadAccess pAccess(aBitmap);
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

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapExTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
