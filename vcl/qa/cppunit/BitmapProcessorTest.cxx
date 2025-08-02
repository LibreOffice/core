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

#include <vcl/bitmap.hxx>
#include <vcl/alpha.hxx>

#include <bitmap/BitmapDisabledImageFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>

namespace
{
class BitmapProcessorTest : public CppUnit::TestFixture
{
    void testDisabledImage();

    CPPUNIT_TEST_SUITE(BitmapProcessorTest);
    CPPUNIT_TEST(testDisabledImage);
    CPPUNIT_TEST_SUITE_END();
};

void BitmapProcessorTest::testDisabledImage()
{
    {
        Bitmap aBitmap(Size(3, 3), vcl::PixelFormat::N24_BPP);
        {
            BitmapScopedWriteAccess pWriteAccess(aBitmap);
            pWriteAccess->Erase(Color(ColorTransparency, 0x00, 0x00, 0xFF, 0x00));
        }
        BitmapDisabledImageFilter aDisabledImageFilter;
        BitmapEx aDisabledBitmapEx(aDisabledImageFilter.execute(aBitmap));
        Bitmap aDisabledBitmap(aDisabledBitmapEx.GetBitmap());
        {
            BitmapScopedReadAccess pReadAccess(aDisabledBitmap);
            Color aColor(pReadAccess->GetPixel(0, 0));
            CPPUNIT_ASSERT_EQUAL(Color(0x00C5C5C5), aColor);
        }
    }

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
        BitmapDisabledImageFilter aDisabledImageFilter;
        Bitmap aDisabledBitmap(aDisabledImageFilter.execute(Bitmap(aBitmapEx)));
        {
            BitmapScopedReadAccess pReadAccess(aDisabledBitmap);
            Color aColor(pReadAccess->GetPixel(0, 0));
            CPPUNIT_ASSERT_EQUAL(Color(ColorAlpha, 0xAA, 0xC5, 0xC5, 0xC5), aColor);
        }
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapProcessorTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
