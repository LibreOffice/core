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

#include <sal/config.h>

#include <string_view>

#include <test/bootstrapfixture.hxx>
#include <tools/stream.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <bitmap/BitmapWriteAccess.hxx>
#include <vcl/alpha.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/tempfile.hxx>

using namespace css;

class PngFilterTest : public test::BootstrapFixture
{
    // Should keep the temp files (should be false)
    static constexpr bool bKeepTemp = true;

    OUString maDataUrl;

    OUString getFullUrl(std::u16string_view sFileName)
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
    void testMsGifInPng();
    void testPngRoundtrip8BitGrey();
    void testPngRoundtrip24();
    void testPngRoundtrip24_8();
    void testPngRoundtrip32();

    CPPUNIT_TEST_SUITE(PngFilterTest);
    CPPUNIT_TEST(testPng);
    CPPUNIT_TEST(testMsGifInPng);
    CPPUNIT_TEST(testPngRoundtrip8BitGrey);
    CPPUNIT_TEST(testPngRoundtrip24);
    CPPUNIT_TEST(testPngRoundtrip24_8);
    CPPUNIT_TEST(testPngRoundtrip32);
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
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Height());

            if (pAccess->GetBitCount() == 24 || pAccess->GetBitCount() == 32)
            {
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 3));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 3));

                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(1, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(1, 2));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(2, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(2, 2));
            }
            else
            {
                CPPUNIT_ASSERT_MESSAGE("Bitmap is not 24 or 32 bit.", false);
            }
        }
    }

    OUString aFilenames[] = {
        OUString("color-rect-8bit-RGB.png"),
        OUString("color-rect-8bit-RGB-interlaced.png"),
        OUString("color-rect-4bit-pal.png"),
    };

    for (const OUString& aFileName : aFilenames)
    {
        SvFileStream aFileStream(getFullUrl(aFileName), StreamMode::READ);

        vcl::PngImageReader aPngReader(aFileStream);
        BitmapEx aBitmapEx;
        aPngReader.read(aBitmapEx);

        Bitmap aBitmap = aBitmapEx.GetBitmap();
        {
            Bitmap::ScopedReadAccess pAccess(aBitmap);
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Height());
            if (pAccess->GetBitCount() == 24 || pAccess->GetBitCount() == 32)
            {
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 3));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 3));

                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(1, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0xFF, 0x00, 0x00),
                                     pAccess->GetPixel(1, 2));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0xFF, 0x00),
                                     pAccess->GetPixel(2, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0x00, 0x00),
                                     pAccess->GetPixel(2, 2));
            }
            else
            {
                CPPUNIT_ASSERT_MESSAGE("Bitmap is not 24 or 32 bit.", false);
            }
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
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Width());
            CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAccess->Height());

            if (pAccess->GetBitCount() == 24)
            {
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 3));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(3, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x00),
                                     pAccess->GetPixel(0, 3));

                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0x00, 0x00, 0x00),
                                     pAccess->GetPixel(1, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0xFF, 0x00, 0x00),
                                     pAccess->GetPixel(1, 2));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0xFF, 0x00),
                                     pAccess->GetPixel(2, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0x00, 0x00),
                                     pAccess->GetPixel(2, 2));

                AlphaMask aAlpha = aBitmapEx.GetAlpha();
                {
                    AlphaMask::ScopedReadAccess pAlphaAccess(aAlpha);
                    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), pAlphaAccess->GetBitCount());
                    CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAlphaAccess->Width());
                    CPPUNIT_ASSERT_EQUAL(tools::Long(4), pAlphaAccess->Height());

                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x80, 0x00),
                                         pAlphaAccess->GetPixel(0, 0));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x80, 0x00),
                                         pAlphaAccess->GetPixel(3, 3));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x80, 0x00),
                                         pAlphaAccess->GetPixel(3, 0));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x80, 0x00),
                                         pAlphaAccess->GetPixel(0, 3));

                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x40, 0x00),
                                         pAlphaAccess->GetPixel(1, 1));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0xC0, 0x00),
                                         pAlphaAccess->GetPixel(1, 2));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0xC0, 0x00),
                                         pAlphaAccess->GetPixel(2, 1));
                    CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0x40, 0x00),
                                         pAlphaAccess->GetPixel(2, 2));
                }
            }
            else if (pAccess->GetBitCount() == 32)
            {
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x80),
                                     pAccess->GetPixel(0, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x80),
                                     pAccess->GetPixel(3, 3));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x80),
                                     pAccess->GetPixel(3, 0));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0xFF, 0x80),
                                     pAccess->GetPixel(0, 3));

                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0x00, 0x00, 0x40),
                                     pAccess->GetPixel(1, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0xFF, 0x00, 0xC0),
                                     pAccess->GetPixel(1, 2));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0x00, 0x00, 0xFF, 0xC0),
                                     pAccess->GetPixel(2, 1));
                CPPUNIT_ASSERT_EQUAL(BitmapColor(ColorTransparency, 0xFF, 0xFF, 0x00, 0x40),
                                     pAccess->GetPixel(2, 2));
            }
            else
            {
                CPPUNIT_ASSERT_MESSAGE("Bitmap is not 24 or 32 bit.", false);
            }
        }
    }
}

void PngFilterTest::testMsGifInPng()
{
    Graphic aGraphic;
    const OUString aURL(getFullUrl(u"ms-gif.png"));
    SvFileStream aFileStream(aURL, StreamMode::READ);
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    ErrCode aResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, aResult);
    CPPUNIT_ASSERT(aGraphic.IsGfxLink());
    // The image is technically a PNG, but it has an animated Gif as a chunk (Microsoft extension).
    CPPUNIT_ASSERT_EQUAL(GfxLinkType::NativeGif, aGraphic.GetSharedGfxLink()->GetType());
    CPPUNIT_ASSERT(aGraphic.IsAnimated());
}

void PngFilterTest::testPngRoundtrip8BitGrey()
{
    utl::TempFile aTempFile(u"testPngRoundtrip8BitGrey");
    if (!bKeepTemp)
        aTempFile.EnableKillingFile();
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::WRITE);
        Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N8_BPP, &Bitmap::GetGreyPalette(256));
        {
            BitmapScopedWriteAccess pWriteAccess(aBitmap);
            pWriteAccess->Erase(COL_BLACK);
            for (int i = 0; i < 8; ++i)
            {
                for (int j = 0; j < 8; ++j)
                {
                    pWriteAccess->SetPixel(i, j, COL_GRAY);
                }
            }
            for (int i = 8; i < 16; ++i)
            {
                for (int j = 8; j < 16; ++j)
                {
                    pWriteAccess->SetPixel(i, j, COL_LIGHTGRAY);
                }
            }
        }
        BitmapEx aBitmapEx(aBitmap);

        vcl::PngImageWriter aPngWriter(rStream);
        CPPUNIT_ASSERT_EQUAL(true, aPngWriter.write(aBitmapEx));
        aTempFile.CloseStream();
    }
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::READ);

        vcl::PngImageReader aPngReader(rStream);
        BitmapEx aBitmapEx;
        CPPUNIT_ASSERT_EQUAL(true, aPngReader.read(aBitmapEx));

        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Height());

        CPPUNIT_ASSERT_EQUAL(COL_GRAY, aBitmapEx.GetPixelColor(0, 0));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTGRAY, aBitmapEx.GetPixelColor(15, 15));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmapEx.GetPixelColor(15, 0));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmapEx.GetPixelColor(0, 15));
    }
}

void PngFilterTest::testPngRoundtrip24()
{
    utl::TempFile aTempFile(u"testPngRoundtrip24");
    if (!bKeepTemp)
        aTempFile.EnableKillingFile();
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::WRITE);
        Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);
        {
            BitmapScopedWriteAccess pWriteAccess(aBitmap);
            pWriteAccess->Erase(COL_BLACK);
            for (int i = 0; i < 8; ++i)
            {
                for (int j = 0; j < 8; ++j)
                {
                    pWriteAccess->SetPixel(i, j, COL_LIGHTRED);
                }
            }
            for (int i = 8; i < 16; ++i)
            {
                for (int j = 8; j < 16; ++j)
                {
                    pWriteAccess->SetPixel(i, j, COL_LIGHTBLUE);
                }
            }
        }
        BitmapEx aBitmapEx(aBitmap);

        vcl::PngImageWriter aPngWriter(rStream);
        CPPUNIT_ASSERT_EQUAL(true, aPngWriter.write(aBitmapEx));
    }
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::READ);
        rStream.Seek(0);

        vcl::PngImageReader aPngReader(rStream);
        BitmapEx aBitmapEx;
        CPPUNIT_ASSERT_EQUAL(true, aPngReader.read(aBitmapEx));

        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Height());

        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aBitmapEx.GetPixelColor(0, 0));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aBitmapEx.GetPixelColor(15, 15));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmapEx.GetPixelColor(15, 0));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmapEx.GetPixelColor(0, 15));
    }
}

void PngFilterTest::testPngRoundtrip24_8()
{
    utl::TempFile aTempFile(u"testPngRoundtrip24_8");
    if (!bKeepTemp)
        aTempFile.EnableKillingFile();
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::WRITE);
        Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);
        AlphaMask aAlpha(Size(16, 16));
        {
            BitmapScopedWriteAccess pWriteAccessBitmap(aBitmap);
            AlphaScopedWriteAccess pWriteAccessAlpha(aAlpha);
            pWriteAccessAlpha->Erase(Color(ColorTransparency, 0x00, 0xAA, 0xAA, 0xAA));
            pWriteAccessBitmap->Erase(COL_BLACK);
            for (int i = 0; i < 8; ++i)
            {
                for (int j = 0; j < 8; ++j)
                {
                    pWriteAccessBitmap->SetPixel(i, j, COL_LIGHTRED);
                    pWriteAccessAlpha->SetPixel(i, j,
                                                Color(ColorTransparency, 0x00, 0xBB, 0xBB, 0xBB));
                }
            }
            for (int i = 8; i < 16; ++i)
            {
                for (int j = 8; j < 16; ++j)
                {
                    pWriteAccessBitmap->SetPixel(i, j, COL_LIGHTBLUE);
                    pWriteAccessAlpha->SetPixel(i, j,
                                                Color(ColorTransparency, 0x00, 0xCC, 0xCC, 0xCC));
                }
            }
        }
        BitmapEx aBitmapEx(aBitmap, aAlpha);
        vcl::PngImageWriter aPngWriter(rStream);
        CPPUNIT_ASSERT_EQUAL(true, aPngWriter.write(aBitmapEx));
    }
    {
        SvStream& rStream = *aTempFile.GetStream(StreamMode::READ);
        rStream.Seek(0);

        vcl::PngImageReader aPngReader(rStream);
        BitmapEx aBitmapEx;
        CPPUNIT_ASSERT_EQUAL(true, aPngReader.read(aBitmapEx));

        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL(16L, aBitmapEx.GetSizePixel().Height());

        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xBB, 0xFF, 0x00, 0x00),
                             aBitmapEx.GetPixelColor(0, 0));
        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xCC, 0x00, 0x00, 0xFF),
                             aBitmapEx.GetPixelColor(15, 15));
        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xAA, 0x00, 0x00, 0x00),
                             aBitmapEx.GetPixelColor(15, 0));
        CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xAA, 0x00, 0x00, 0x00),
                             aBitmapEx.GetPixelColor(0, 15));
    }
}

void PngFilterTest::testPngRoundtrip32() {}

CPPUNIT_TEST_SUITE_REGISTRATION(PngFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
