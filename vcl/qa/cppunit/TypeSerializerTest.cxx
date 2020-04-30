/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <unotest/directories.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <comphelper/hash.hxx>
#include <tools/vcompat.hxx>
#include <comphelper/fileformat.h>

#include <TypeSerializer.hxx>

namespace
{
constexpr char DATA_DIRECTORY[] = "/vcl/qa/cppunit/data/";

std::vector<unsigned char> calculateHash(SvStream& rStream)
{
    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    comphelper::Hash aHashEngine(comphelper::HashType::SHA1);
    const sal_uInt32 nSize(rStream.remainingSize());
    std::vector<sal_uInt8> aData(nSize);
    aHashEngine.update(aData.data(), nSize);
    return aHashEngine.finalize();
}

std::string toHexString(const std::vector<unsigned char>& a)
{
    std::stringstream aStrm;
    for (auto& i : a)
    {
        aStrm << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(i);
    }

    return aStrm.str();
}

class TypeSerializerTest : public CppUnit::TestFixture
{
    void testGradient();
    void testGraphic();
    void testGraphic_Bitmap_NoGfxLink();

    CPPUNIT_TEST_SUITE(TypeSerializerTest);
    CPPUNIT_TEST(testGradient);
    CPPUNIT_TEST(testGraphic);
    CPPUNIT_TEST(testGraphic_Bitmap_NoGfxLink);
    CPPUNIT_TEST_SUITE_END();
};

void TypeSerializerTest::testGradient()
{
    Gradient aGradient(GradientStyle::Radial, Color(0xFF, 0x00, 0x00), Color(0x00, 0xFF, 0x00));
    aGradient.SetAngle(900);
    aGradient.SetBorder(5);
    aGradient.SetOfsX(11);
    aGradient.SetOfsY(12);
    aGradient.SetStartIntensity(21);
    aGradient.SetEndIntensity(22);
    aGradient.SetSteps(30);

    SvMemoryStream aStream;
    TypeSerializer aSerializer(aStream);
    aSerializer.writeGradient(aGradient);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    Gradient aReadGradient;
    aSerializer.readGradient(aReadGradient);
    CPPUNIT_ASSERT_EQUAL(GradientStyle::Radial, aReadGradient.GetStyle());
    CPPUNIT_ASSERT_EQUAL(Color(0xFF, 0x00, 0x00), aReadGradient.GetStartColor());
    CPPUNIT_ASSERT_EQUAL(Color(0x00, 0xFF, 0x00), aReadGradient.GetEndColor());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(900), aReadGradient.GetAngle());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), aReadGradient.GetBorder());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(11), aReadGradient.GetOfsX());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(12), aReadGradient.GetOfsY());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(21), aReadGradient.GetStartIntensity());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(22), aReadGradient.GetEndIntensity());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), aReadGradient.GetSteps());
}

void TypeSerializerTest::testGraphic()
{
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "SimpleExample.svg";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();

    // Test WriteGraphic - Native Format 5
    {
        SvMemoryStream aMemoryStream;
        aMemoryStream.SetVersion(SOFFICE_FILEFORMAT_50);
        aMemoryStream.SetCompressMode(SvStreamCompressFlags::NATIVE);
        WriteGraphic(aMemoryStream, aGraphic);
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(290), aMemoryStream.remainingSize());
        std::vector<unsigned char> aHash = calculateHash(aMemoryStream);
        CPPUNIT_ASSERT_EQUAL(std::string("ee55ab6faa73b61b68bc3d5628d95f0d3c528e2a"),
                             toHexString(aHash));

        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        sal_uInt32 nType;
        aMemoryStream.ReadUInt32(nType);
        CPPUNIT_ASSERT_EQUAL(COMPAT_FORMAT('N', 'A', 'T', '5'), nType);
    }

    // Test WriteGraphic - Normal
    {
        SvMemoryStream aMemoryStream;
        WriteGraphic(aMemoryStream, aGraphic);
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(233), aMemoryStream.remainingSize());
        std::vector<unsigned char> aHash = calculateHash(aMemoryStream);
        CPPUNIT_ASSERT_EQUAL(std::string("c2bed2099ce617f1cc035701de5186f0d43e3064"),
                             toHexString(aHash));
    }

    // Test TypeSerializer - Native Format 5
    {
        SvMemoryStream aMemoryStream;
        aMemoryStream.SetVersion(SOFFICE_FILEFORMAT_50);
        aMemoryStream.SetCompressMode(SvStreamCompressFlags::NATIVE);
        {
            TypeSerializer aSerializer(aMemoryStream);
            aSerializer.writeGraphic(aGraphic);
        }
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(290), aMemoryStream.remainingSize());
        std::vector<unsigned char> aHash = calculateHash(aMemoryStream);
        CPPUNIT_ASSERT_EQUAL(std::string("ee55ab6faa73b61b68bc3d5628d95f0d3c528e2a"),
                             toHexString(aHash));

        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        sal_uInt32 nType;
        aMemoryStream.ReadUInt32(nType);
        CPPUNIT_ASSERT_EQUAL(COMPAT_FORMAT('N', 'A', 'T', '5'), nType);
    }

    // Test TypeSerializer - Normal
    {
        SvMemoryStream aMemoryStream;
        {
            TypeSerializer aSerializer(aMemoryStream);
            aSerializer.writeGraphic(aGraphic);
        }
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(233), aMemoryStream.remainingSize());
        std::vector<unsigned char> aHash = calculateHash(aMemoryStream);
        CPPUNIT_ASSERT_EQUAL(std::string("c2bed2099ce617f1cc035701de5186f0d43e3064"),
                             toHexString(aHash));
    }
}

void TypeSerializerTest::testGraphic_Bitmap_NoGfxLink()
{
    Bitmap aBitmap(Size(10, 10), 24);
    aBitmap.Erase(COL_LIGHTGRAYBLUE);
    BitmapEx aBitmapEx(aBitmap);
    Graphic aGraphic(aBitmapEx);

    // Test WriteGraphic
    {
        SvMemoryStream aMemoryStream;
        WriteGraphic(aMemoryStream, aGraphic);
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(383), aMemoryStream.remainingSize());
        std::vector<unsigned char> aHash = calculateHash(aMemoryStream);
        CPPUNIT_ASSERT_EQUAL(std::string("da831418499146d51bf245fadf60b9111faa76c2"),
                             toHexString(aHash));

        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        sal_uInt16 nType;
        aMemoryStream.ReadUInt16(nType);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x4D42), nType); // Magic written with WriteDIBBitmapEx
    }

    // Test TypeSerializer
    {
        SvMemoryStream aMemoryStream;
        {
            TypeSerializer aSerializer(aMemoryStream);
            aSerializer.writeGraphic(aGraphic);
        }
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(383), aMemoryStream.remainingSize());
        std::vector<unsigned char> aHash = calculateHash(aMemoryStream);
        CPPUNIT_ASSERT_EQUAL(std::string("da831418499146d51bf245fadf60b9111faa76c2"),
                             toHexString(aHash));

        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        sal_uInt16 nType;
        aMemoryStream.ReadUInt16(nType);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x4D42), nType); // Magic written with WriteDIBBitmapEx
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(TypeSerializerTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
