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
#include <vcl/bmpacc.hxx>

#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>

#include "BitmapSymmetryCheck.hxx"

namespace
{

class BitmapTest : public CppUnit::TestFixture
{
    void testScale();

    CPPUNIT_TEST_SUITE(BitmapTest);
    CPPUNIT_TEST(testScale);
    CPPUNIT_TEST_SUITE_END();
};

void BitmapTest::testScale()
{
    bool bExportBitmap(false);

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
    CPPUNIT_ASSERT(aBitmapSymmetryCheck.check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream(OUString("~/scale_before.png"), StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream, 9);
    }

    aBitmap24Bit.Scale(2, 2, BMP_SCALE_FAST);

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(20), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(20), aBitmap24Bit.GetSizePixel().Height());

    // After scaling the bitmap should still be symmetrical. This check guarantees that
    // scaling doesn't misalign the bitmap.
    CPPUNIT_ASSERT(aBitmapSymmetryCheck.check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream(OUString("~/scale_after.png"), StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream, 9);
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
