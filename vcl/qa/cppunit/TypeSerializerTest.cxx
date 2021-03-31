/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <config_oox.h>
#include <config_features.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <unotest/directories.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <comphelper/hash.hxx>
#include <tools/vcompat.hxx>
#include <comphelper/fileformat.h>
#include <tools/fract.hxx>

#include <vcl/TypeSerializer.hxx>

#if USE_TLS_NSS
#include <nss.h>
#endif

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/vcl/qa/cppunit/data/";

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
public:
    ~TypeSerializerTest();

private:
    void testGradient();
    void testGraphic_Vector();
    void testGraphic_Bitmap_NoGfxLink();
    void testGraphic_Animation();
    void testGraphic_GDIMetaFile();
    void testMapMode();

    CPPUNIT_TEST_SUITE(TypeSerializerTest);
    CPPUNIT_TEST(testGradient);
    CPPUNIT_TEST(testGraphic_Vector);
    CPPUNIT_TEST(testGraphic_Bitmap_NoGfxLink);
    CPPUNIT_TEST(testGraphic_Animation);
    CPPUNIT_TEST(testGraphic_GDIMetaFile);
    CPPUNIT_TEST(testMapMode);
    CPPUNIT_TEST_SUITE_END();
};

TypeSerializerTest::~TypeSerializerTest()
{
#if USE_TLS_NSS
    NSS_Shutdown();
#endif
}

void TypeSerializerTest::testGradient()
{
    Gradient aGradient(GradientStyle::Radial, Color(0xFF, 0x00, 0x00), Color(0x00, 0xFF, 0x00));
    aGradient.SetAngle(900_deg10);
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
    CPPUNIT_ASSERT_EQUAL(sal_Int16(900), aReadGradient.GetAngle().get());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), aReadGradient.GetBorder());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(11), aReadGradient.GetOfsX());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(12), aReadGradient.GetOfsY());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(21), aReadGradient.GetStartIntensity());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(22), aReadGradient.GetEndIntensity());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), aReadGradient.GetSteps());
}

void TypeSerializerTest::testGraphic_Vector()
{
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "SimpleExample.svg";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();
    BitmapChecksum aChecksum = aGraphic.getVectorGraphicData()->GetChecksum();

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

        // Read it back
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        Graphic aNewGraphic;
        {
            TypeSerializer aSerializer(aMemoryStream);
            aSerializer.readGraphic(aNewGraphic);
        }
        CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aNewGraphic.GetType());
        CPPUNIT_ASSERT_EQUAL(aChecksum, aNewGraphic.getVectorGraphicData()->GetChecksum());
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

        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        sal_uInt32 nType;
        aMemoryStream.ReadUInt32(nType);
        CPPUNIT_ASSERT_EQUAL(createMagic('s', 'v', 'g', '0'), nType);

        // Read it back
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        Graphic aNewGraphic;
        {
            TypeSerializer aSerializer(aMemoryStream);
            aSerializer.readGraphic(aNewGraphic);
        }
        CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aNewGraphic.GetType());
        CPPUNIT_ASSERT_EQUAL(aChecksum, aNewGraphic.getVectorGraphicData()->GetChecksum());
    }
}

void TypeSerializerTest::testGraphic_Bitmap_NoGfxLink()
{
    Bitmap aBitmap(Size(10, 10), vcl::PixelFormat::N24_BPP);
    aBitmap.Erase(COL_LIGHTGRAYBLUE);
    BitmapEx aBitmapEx(aBitmap);
    Graphic aGraphic(aBitmapEx);

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

        // Read it back
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        Graphic aNewGraphic;
        {
            TypeSerializer aSerializer(aMemoryStream);
            aSerializer.readGraphic(aNewGraphic);
        }
        CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aNewGraphic.GetType());
        CPPUNIT_ASSERT_EQUAL(aBitmapEx.GetChecksum(), aNewGraphic.GetBitmapExRef().GetChecksum());
    }
}

void TypeSerializerTest::testGraphic_Animation()
{
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "123_Numbers.gif";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    aGraphic.makeAvailable();
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.IsAnimated());

    // Test TypeSerializer
    {
        SvMemoryStream aMemoryStream;
        {
            TypeSerializer aSerializer(aMemoryStream);
            aSerializer.writeGraphic(aGraphic);
        }
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(15167), aMemoryStream.remainingSize());
        std::vector<unsigned char> aHash = calculateHash(aMemoryStream);
        CPPUNIT_ASSERT_EQUAL(std::string("69d0f80832a0aebcbda7ad43ecadf85e99fc1057"),
                             toHexString(aHash));

        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        sal_uInt16 nType;
        aMemoryStream.ReadUInt16(nType);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0x4D42), nType);

        // Read it back
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        Graphic aNewGraphic;
        {
            TypeSerializer aSerializer(aMemoryStream);
            aSerializer.readGraphic(aNewGraphic);
        }
        CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aNewGraphic.GetType());
        CPPUNIT_ASSERT_EQUAL(true, aNewGraphic.IsAnimated());
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

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(1582), aMemoryStream.remainingSize());
        std::vector<unsigned char> aHash = calculateHash(aMemoryStream);
        CPPUNIT_ASSERT_EQUAL(std::string("da3b9600340fa80a895f2107357e4ab65a9292eb"),
                             toHexString(aHash));

        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        sal_uInt32 nType;
        aMemoryStream.ReadUInt32(nType);
        CPPUNIT_ASSERT_EQUAL(COMPAT_FORMAT('N', 'A', 'T', '5'), nType);

        // Read it back
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        Graphic aNewGraphic;
        {
            TypeSerializer aSerializer(aMemoryStream);
            aSerializer.readGraphic(aNewGraphic);
        }
        CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aNewGraphic.GetType());
        CPPUNIT_ASSERT_EQUAL(true, aNewGraphic.IsAnimated());
    }
}

void TypeSerializerTest::testGraphic_GDIMetaFile()
{
    GDIMetaFile aGDIMetaFile;
    {
        ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
        pVirtualDev->SetConnectMetaFile(&aGDIMetaFile);
        Size aVDSize(10, 10);
        pVirtualDev->SetOutputSizePixel(aVDSize);
        pVirtualDev->SetBackground(Wallpaper(COL_LIGHTRED));
        pVirtualDev->Erase();
        pVirtualDev->DrawPixel(Point(4, 4));
    }
    Graphic aGraphic(aGDIMetaFile);
    CPPUNIT_ASSERT_EQUAL(GraphicType::GdiMetafile, aGraphic.GetType());

    // Test TypeSerializer
    {
        SvMemoryStream aMemoryStream;
        {
            TypeSerializer aSerializer(aMemoryStream);
            aSerializer.writeGraphic(aGraphic);
        }
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);

        CPPUNIT_ASSERT_EQUAL(sal_uInt64(229), aMemoryStream.remainingSize());
        std::vector<unsigned char> aHash = calculateHash(aMemoryStream);
        CPPUNIT_ASSERT_EQUAL(std::string("144c518e5149d61ab4bc34643df820372405d61d"),
                             toHexString(aHash));

        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        char aIdCharArray[7] = { 0, 0, 0, 0, 0, 0, 0 };
        aMemoryStream.ReadBytes(aIdCharArray, 6);
        OString sID(aIdCharArray);
        CPPUNIT_ASSERT_EQUAL(OString("VCLMTF"), sID);

        // Read it back
        aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
        Graphic aNewGraphic;
        {
            TypeSerializer aSerializer(aMemoryStream);
            aSerializer.readGraphic(aNewGraphic);
        }
        CPPUNIT_ASSERT_EQUAL(GraphicType::GdiMetafile, aNewGraphic.GetType());
    }
}

void TypeSerializerTest::testMapMode()
{
    { // "simple" case - only map unit is set, IsSimple = true
        MapMode aMapMode(MapUnit::Map100thMM);

        SvMemoryStream aStream;
        TypeSerializer aSerializer(aStream);
        aSerializer.writeMapMode(aMapMode);
        aStream.Seek(STREAM_SEEK_TO_BEGIN);

        MapMode aReadMapMode;
        aSerializer.readMapMode(aReadMapMode);
        CPPUNIT_ASSERT_EQUAL(MapUnit::Map100thMM, aReadMapMode.GetMapUnit());
        CPPUNIT_ASSERT_EQUAL(true, aReadMapMode.IsSimple());
    }
    { // "complex" case - map unit, origin and scale are set, IsSimple = false
        MapMode aMapMode(MapUnit::MapTwip, Point(5, 10), Fraction(1, 2), Fraction(2, 3));

        SvMemoryStream aStream;
        TypeSerializer aSerializer(aStream);
        aSerializer.writeMapMode(aMapMode);
        aStream.Seek(STREAM_SEEK_TO_BEGIN);

        MapMode aReadMapMode;
        aSerializer.readMapMode(aReadMapMode);
        CPPUNIT_ASSERT_EQUAL(MapUnit::MapTwip, aReadMapMode.GetMapUnit());
        CPPUNIT_ASSERT_EQUAL(tools::Long(5), aReadMapMode.GetOrigin().X());
        CPPUNIT_ASSERT_EQUAL(tools::Long(10), aReadMapMode.GetOrigin().Y());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aReadMapMode.GetScaleX().GetNumerator());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aReadMapMode.GetScaleX().GetDenominator());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aReadMapMode.GetScaleY().GetNumerator());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aReadMapMode.GetScaleY().GetDenominator());
        CPPUNIT_ASSERT_EQUAL(false, aReadMapMode.IsSimple());
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(TypeSerializerTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
