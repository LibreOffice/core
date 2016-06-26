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
#include <vcl/filter/PngReader.hxx>
#include <vcl/bitmapaccess.hxx>

using namespace css;

class PngFilterTest : public test::BootstrapFixture
{
    OUString maDataUrl;

    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }

public:
    PngFilterTest() :
        BootstrapFixture(true, false),
        maDataUrl("/vcl/qa/cppunit/png/data/")
    {}

    void testPng();

    CPPUNIT_TEST_SUITE(PngFilterTest);
    CPPUNIT_TEST(testPng);
    CPPUNIT_TEST_SUITE_END();
};

void PngFilterTest::testPng()
{
    for (const OUString& aFileName: { OUString("rect-1bit-pal.png") })
    {
        SvFileStream aFileStream(getFullUrl(aFileName), StreamMode::READ);

        Graphic aGraphic;
        vcl::PngReader aPngReader(aFileStream);
        aPngReader.Read(aGraphic);

        Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
        {
            Bitmap::ScopedReadAccess pAccess(aBitmap);
            CPPUNIT_ASSERT_EQUAL(32, int(pAccess->GetBitCount()));
            CPPUNIT_ASSERT_EQUAL(4l, pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(4l, pAccess->Height());

            CPPUNIT_ASSERT_EQUAL(ColorData(0x00FFFFFF), Color(pAccess->GetPixel(0,0)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x00FFFFFF), Color(pAccess->GetPixel(3,3)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x00FFFFFF), Color(pAccess->GetPixel(3,0)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x00FFFFFF), Color(pAccess->GetPixel(0,3)).GetColor());

            CPPUNIT_ASSERT_EQUAL(ColorData(0x00000000), Color(pAccess->GetPixel(1,1)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x00000000), Color(pAccess->GetPixel(1,2)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x00000000), Color(pAccess->GetPixel(2,1)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x00000000), Color(pAccess->GetPixel(2,2)).GetColor());
        }
    }
    for (const OUString& aFileName: { OUString("color-rect-8bit-RGB.png"), OUString("color-rect-4bit-pal.png") })
    {
        SvFileStream aFileStream(getFullUrl(aFileName), StreamMode::READ);

        Graphic aGraphic;
        vcl::PngReader aPngReader(aFileStream);
        aPngReader.Read(aGraphic);

        Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
        {
            Bitmap::ScopedReadAccess pAccess(aBitmap);
            CPPUNIT_ASSERT_EQUAL(32, int(pAccess->GetBitCount()));
            CPPUNIT_ASSERT_EQUAL(4l, pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(4l, pAccess->Height());

            CPPUNIT_ASSERT_EQUAL(ColorData(0x00FFFFFF), Color(pAccess->GetPixel(0,0)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x00FFFFFF), Color(pAccess->GetPixel(3,3)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x00FFFFFF), Color(pAccess->GetPixel(3,0)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x00FFFFFF), Color(pAccess->GetPixel(0,3)).GetColor());

            CPPUNIT_ASSERT_EQUAL(ColorData(0x00FF0000), Color(pAccess->GetPixel(1,1)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x0000FF00), Color(pAccess->GetPixel(1,2)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x000000FF), Color(pAccess->GetPixel(2,1)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x00FFFF00), Color(pAccess->GetPixel(2,2)).GetColor());
        }
    }
    for (const OUString& aFileName: { OUString("alpha-rect-8bit-RGBA.png") })
    {
        SvFileStream aFileStream(getFullUrl(aFileName), StreamMode::READ);

        Graphic aGraphic;
        vcl::PngReader aPngReader(aFileStream);
        aPngReader.Read(aGraphic);

        Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
        {
            Bitmap::ScopedReadAccess pAccess(aBitmap);
            CPPUNIT_ASSERT_EQUAL(32, int(pAccess->GetBitCount()));
            CPPUNIT_ASSERT_EQUAL(4l, pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(4l, pAccess->Height());

            printf("%08X\n", Color(pAccess->GetPixel(1,1)).GetColor());

            CPPUNIT_ASSERT_EQUAL(ColorData(0x80FFFFFF), Color(pAccess->GetPixel(0,0)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x80FFFFFF), Color(pAccess->GetPixel(3,3)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x80FFFFFF), Color(pAccess->GetPixel(3,0)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x80FFFFFF), Color(pAccess->GetPixel(0,3)).GetColor());

            CPPUNIT_ASSERT_EQUAL(ColorData(0x40FF0000), Color(pAccess->GetPixel(1,1)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0xC000FF00), Color(pAccess->GetPixel(1,2)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0xC00000FF), Color(pAccess->GetPixel(2,1)).GetColor());
            CPPUNIT_ASSERT_EQUAL(ColorData(0x40FFFF00), Color(pAccess->GetPixel(2,2)).GetColor());
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(PngFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
