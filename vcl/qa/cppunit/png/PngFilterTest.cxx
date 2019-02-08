/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <test/bootstrapfixture.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/alpha.hxx>

using namespace css;

class PngFilterTest : public test::BootstrapFixture
{
    OUString maDataUrl;

    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }

public:
    PngFilterTest()
        : BootstrapFixture(true, false)
        , maDataUrl("/vcl/qa/cppunit/png/data/")
    {
    }

    void testPng();

    CPPUNIT_TEST_SUITE(PngFilterTest);
    CPPUNIT_TEST(testPng);
    CPPUNIT_TEST_SUITE_END();
};

void PngFilterTest::testPng()
{
    for (const OUString& aFileName : { OUString("rect-1bit-pal.png") })
    {
        SvFileStream aFileStream(getFullUrl(aFileName), StreamMode::READ);

        vcl::PngImageReader aPngReader(aFileStream);
        BitmapEx aBitmapEx;
        aPngReader.read(aBitmapEx);

        Bitmap aBitmap = aBitmapEx.GetBitmap();
        {
            Bitmap::ScopedReadAccess pAccess(aBitmap);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(24), pAccess->GetBitCount());
            CPPUNIT_ASSERT_EQUAL(4L, pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(4L, pAccess->Height());

            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(0, 0));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(3, 3));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(3, 0));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(0, 3));

            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0x00, 0x00), pAccess->GetPixel(1, 1));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0x00, 0x00), pAccess->GetPixel(1, 2));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0x00, 0x00), pAccess->GetPixel(2, 1));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0x00, 0x00), pAccess->GetPixel(2, 2));
        }
    }
    for (const OUString& aFileName :
         { OUString("color-rect-8bit-RGB.png"), OUString("color-rect-4bit-pal.png") })
    {
        SvFileStream aFileStream(getFullUrl(aFileName), StreamMode::READ);

        vcl::PngImageReader aPngReader(aFileStream);
        BitmapEx aBitmapEx;
        aPngReader.read(aBitmapEx);

        Bitmap aBitmap = aBitmapEx.GetBitmap();
        {
            Bitmap::ScopedReadAccess pAccess(aBitmap);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(24), pAccess->GetBitCount());
            CPPUNIT_ASSERT_EQUAL(4L, pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(4L, pAccess->Height());

            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(0, 0));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(3, 3));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(3, 0));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(0, 3));

            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0x00, 0x00, 0x00), pAccess->GetPixel(1, 1));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0xFF, 0x00, 0x00), pAccess->GetPixel(1, 2));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0xFF, 0x00), pAccess->GetPixel(2, 1));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0x00, 0x00), pAccess->GetPixel(2, 2));
        }
    }
    for (const OUString& aFileName : { OUString("alpha-rect-8bit-RGBA.png") })
    {
        SvFileStream aFileStream(getFullUrl(aFileName), StreamMode::READ);

        vcl::PngImageReader aPngReader(aFileStream);
        BitmapEx aBitmapEx;
        aPngReader.read(aBitmapEx);

        Bitmap aBitmap = aBitmapEx.GetBitmap();
        {
            Bitmap::ScopedReadAccess pAccess(aBitmap);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(24), pAccess->GetBitCount());
            CPPUNIT_ASSERT_EQUAL(4L, pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(4L, pAccess->Height());

            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(0, 0));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(3, 3));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(3, 0));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0xFF, 0x00), pAccess->GetPixel(0, 3));

            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0x00, 0x00, 0x00), pAccess->GetPixel(1, 1));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0xFF, 0x00, 0x00), pAccess->GetPixel(1, 2));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0xFF, 0x00), pAccess->GetPixel(2, 1));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0xFF, 0xFF, 0x00, 0x00), pAccess->GetPixel(2, 2));
        }
        AlphaMask aAlpha = aBitmapEx.GetAlpha();
        {
            AlphaMask::ScopedReadAccess pAccess(aAlpha);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), pAccess->GetBitCount());
            CPPUNIT_ASSERT_EQUAL(4L, pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(4L, pAccess->Height());

            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0x80, 0x00), pAccess->GetPixel(0, 0));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0x80, 0x00), pAccess->GetPixel(3, 3));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0x80, 0x00), pAccess->GetPixel(3, 0));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0x80, 0x00), pAccess->GetPixel(0, 3));

            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0x40, 0x00), pAccess->GetPixel(1, 1));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0xC0, 0x00), pAccess->GetPixel(1, 2));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0xC0, 0x00), pAccess->GetPixel(2, 1));
            CPPUNIT_ASSERT_EQUAL(BitmapColor(0x00, 0x00, 0x40, 0x00), pAccess->GetPixel(2, 2));
        }
    }
    // CPPUNIT_ASSERT(false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(PngFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
