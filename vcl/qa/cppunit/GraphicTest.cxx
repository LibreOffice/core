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
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/bitmapaccess.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>
#include <unotest/directories.hxx>
#include <comphelper/DirectoryHelper.hxx>
#include <comphelper/hash.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <impgraph.hxx>

#if USE_TLS_NSS
#include <nss.h>
#endif

using namespace css;

namespace
{
class GraphicTest : public CppUnit::TestFixture
{
public:
    ~GraphicTest();

private:
    void testUnloadedGraphic();
    void testUnloadedGraphicLoading();
    void testUnloadedGraphicWmf();
    void testUnloadedGraphicAlpha();
    void testUnloadedGraphicSizeUnit();
    void testSwapping();
    void testSwappingVectorGraphic();
    void testSwappingPageNumber();

    CPPUNIT_TEST_SUITE(GraphicTest);
    CPPUNIT_TEST(testUnloadedGraphic);
    CPPUNIT_TEST(testUnloadedGraphicLoading);
    CPPUNIT_TEST(testUnloadedGraphicWmf);
    CPPUNIT_TEST(testUnloadedGraphicAlpha);
    CPPUNIT_TEST(testUnloadedGraphicSizeUnit);
    CPPUNIT_TEST(testSwapping);
    CPPUNIT_TEST(testSwappingVectorGraphic);
    CPPUNIT_TEST(testSwappingPageNumber);
    CPPUNIT_TEST_SUITE_END();
};

GraphicTest::~GraphicTest()
{
#if USE_TLS_NSS
    NSS_Shutdown();
#endif
}

BitmapEx createBitmap(bool alpha = false)
{
    Bitmap aBitmap(Size(120, 100), 24);
    aBitmap.Erase(COL_LIGHTRED);

    aBitmap.SetPrefSize(Size(6000, 5000));
    aBitmap.SetPrefMapMode(MapMode(MapUnit::Map100thMM));

    if (alpha)
    {
        sal_uInt8 uAlphaValue = 0x80;
        AlphaMask aAlphaMask(Size(120, 100), &uAlphaValue);

        return BitmapEx(aBitmap, aAlphaMask);
    }
    else
    {
        return BitmapEx(aBitmap);
    }
}

void createBitmapAndExportForType(SvStream& rStream, OUString const& sType, bool alpha)
{
    BitmapEx aBitmapEx = createBitmap(alpha);

    uno::Sequence<beans::PropertyValue> aFilterData;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilterFormat = rGraphicFilter.GetExportFormatNumberForShortName(sType);
    rGraphicFilter.ExportGraphic(aBitmapEx, "none", rStream, nFilterFormat, &aFilterData);

    rStream.Seek(STREAM_SEEK_TO_BEGIN);
}

Graphic makeUnloadedGraphic(OUString const& sType, bool alpha = false)
{
    SvMemoryStream aStream;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    createBitmapAndExportForType(aStream, sType, alpha);
    return rGraphicFilter.ImportUnloadedGraphic(aStream);
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

std::unique_ptr<SvStream> createStream(OUString const& rSwapFileURL)
{
    std::unique_ptr<SvStream> xStream;

    try
    {
        xStream = ::utl::UcbStreamHelper::CreateStream(
            rSwapFileURL, StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE);
    }
    catch (const css::uno::Exception&)
    {
    }

    return xStream;
}

std::vector<unsigned char> calculateHash(std::unique_ptr<SvStream>& rStream)
{
    comphelper::Hash aHashEngine(comphelper::HashType::SHA1);
    const sal_uInt32 nSize(rStream->remainingSize());
    std::vector<sal_uInt8> aData(nSize);
    aHashEngine.update(aData.data(), nSize);
    return aHashEngine.finalize();
}

bool checkBitmap(Graphic& rGraphic)
{
    bool bResult = true;

    Bitmap aBitmap(rGraphic.GetBitmapEx().GetBitmap());
    {
        Bitmap::ScopedReadAccess pReadAccess(aBitmap);
        for (long y = 0; y < rGraphic.GetSizePixel().Height(); y++)
        {
            for (long x = 0; x < rGraphic.GetSizePixel().Width(); x++)
            {
                if (pReadAccess->HasPalette())
                {
                    sal_uInt32 nIndex = pReadAccess->GetPixelIndex(y, x);
                    Color aColor = pReadAccess->GetPaletteColor(nIndex);
                    bResult &= (aColor == Color(0xff, 0x00, 0x00));
                }
                else
                {
                    Color aColor = pReadAccess->GetPixel(y, x);
                    bResult &= (aColor == Color(0xff, 0x00, 0x00));
                }
            }
        }
    }

    return bResult;
}

char const DATA_DIRECTORY[] = "/vcl/qa/cppunit/data/";
char const PDFEXPORT_DATA_DIRECTORY[] = "/vcl/qa/cppunit/pdfexport/data/";

void GraphicTest::testUnloadedGraphic()
{
    // make unloaded test graphic
    Graphic aGraphic = makeUnloadedGraphic("png");
    Graphic aGraphic2 = aGraphic;

    // check available
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic2.isAvailable());

    CPPUNIT_ASSERT_EQUAL(true, aGraphic2.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic2.isAvailable());

    // check GetSizePixel doesn't load graphic
    aGraphic = makeUnloadedGraphic("png");
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(120L, aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(100L, aGraphic.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // check GetPrefSize doesn't load graphic
    CPPUNIT_ASSERT_EQUAL(6000L, aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(5000L, aGraphic.GetPrefSize().Height());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // check GetSizeBytes loads graphic
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0);
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    //check Type
    aGraphic = makeUnloadedGraphic("png");
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
}

void GraphicTest::testUnloadedGraphicLoading()
{
    const OUString aFormats[] = { "png", "gif", "jpg" };

    for (OUString const& sFormat : aFormats)
    {
        Graphic aGraphic = makeUnloadedGraphic(sFormat);

        // check available
        CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
        CPPUNIT_ASSERT_EQUAL(120L, aGraphic.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL(100L, aGraphic.GetSizePixel().Height());
        CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
        CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0);
        CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

        if (sFormat != "jpg")
            CPPUNIT_ASSERT_EQUAL(true, checkBitmap(aGraphic));
    }
}

void GraphicTest::testUnloadedGraphicWmf()
{
    // Create some in-memory WMF data, set its own preferred size to 99x99.
    BitmapEx aBitmapEx = createBitmap();
    SvMemoryStream aStream;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilterFormat = rGraphicFilter.GetExportFormatNumberForShortName("wmf");
    Graphic aGraphic(aBitmapEx);
    aGraphic.SetPrefSize(Size(99, 99));
    aGraphic.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
    rGraphicFilter.ExportGraphic(aGraphic, "none", aStream, nFilterFormat);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    // Now lazy-load this WMF data, with a custom preferred size of 42x42.
    Size aMtfSize100(42, 42);
    aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream, 0, &aMtfSize100);
    aGraphic.makeAvailable();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 42x42
    // - Actual  : 99x99
    // i.e. the custom preferred size was lost after lazy-load.
    CPPUNIT_ASSERT_EQUAL(Size(42, 42), aGraphic.GetPrefSize());
}

void GraphicTest::testUnloadedGraphicAlpha()
{
    // make unloaded test graphic with alpha
    Graphic aGraphic = makeUnloadedGraphic("png", true);

    CPPUNIT_ASSERT_EQUAL(true, aGraphic.IsAlpha());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.IsTransparent());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // make unloaded test graphic without alpha
    aGraphic = makeUnloadedGraphic("png", false);

    CPPUNIT_ASSERT_EQUAL(false, aGraphic.IsAlpha());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.IsTransparent());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
}

void GraphicTest::testUnloadedGraphicSizeUnit()
{
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "inch-size.emf";
    Size aMtfSize100(42, 42);
    SvFileStream aStream(aURL, StreamMode::READ);
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream, 0, &aMtfSize100);
    aGraphic.makeAvailable();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 400x363
    // - Actual  : 42x42
    // i.e. a mm100 size was used as a hint and the inch size was set for a non-matching unit.
    CPPUNIT_ASSERT_EQUAL(Size(400, 363), aGraphic.GetPrefSize());
}

void GraphicTest::testSwapping()
{
    // Prepare Graphic from a PNG image first
    Graphic aGraphic = makeUnloadedGraphic("png");

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(120L, aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(100L, aGraphic.GetSizePixel().Height());

    BitmapChecksum aChecksumBeforeSwapping = aGraphic.GetChecksum();

    CPPUNIT_ASSERT_EQUAL(sal_uInt32(319), aGraphic.GetGfxLink().GetDataSize());

    // We loaded the Graphic and made it available
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    // Get the declared byte size of the graphic
    sal_uLong rByteSize = aGraphic.GetSizeBytes();
    OUString rSwapFileURL = aGraphic.ImplGetImpGraphic()->getSwapFileURL();
    CPPUNIT_ASSERT_EQUAL(true, rSwapFileURL.isEmpty());

    // Swapping out
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Byte size doesn't change when we swapped out
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());

    // Let's check the swap file
    rSwapFileURL = aGraphic.ImplGetImpGraphic()->getSwapFileURL();
    CPPUNIT_ASSERT_EQUAL(true, comphelper::DirectoryHelper::fileExists(rSwapFileURL));

    { // Check the swap file content
        std::unique_ptr<SvStream> xStream = createStream(rSwapFileURL);
        CPPUNIT_ASSERT_EQUAL(true, bool(xStream));

        // Check size of the stream
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(449), xStream->remainingSize());

        std::vector<unsigned char> aHash = calculateHash(xStream);
        CPPUNIT_ASSERT_EQUAL(std::string("878281e583487b29ae09078e8040c01791c7649a"),
                             toHexString(aHash));
    }

    // Let's swap in
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    CPPUNIT_ASSERT_EQUAL(aChecksumBeforeSwapping, aGraphic.GetChecksum());

    // File shouldn't be available anymore
    CPPUNIT_ASSERT_EQUAL(false, comphelper::DirectoryHelper::fileExists(rSwapFileURL));

    // Check the bitmap
    CPPUNIT_ASSERT_EQUAL(120L, aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(100L, aGraphic.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(true, checkBitmap(aGraphic));
    CPPUNIT_ASSERT_EQUAL(true, checkBitmap(aGraphic));
}

void GraphicTest::testSwappingVectorGraphic()
{
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "SimpleExample.svg";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Load the vector graphic
    CPPUNIT_ASSERT_EQUAL(true, bool(aGraphic.getVectorGraphicData()));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(223),
                         aGraphic.getVectorGraphicData()->getVectorGraphicDataArrayLength());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(223), aGraphic.GetGfxLink().GetDataSize());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    BitmapChecksum aBitmapChecksumBeforeSwapping = aGraphic.GetBitmapEx().GetChecksum();

    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Get the declared byte size of the graphic
    sal_uLong rByteSize = aGraphic.GetSizeBytes();
    CPPUNIT_ASSERT_EQUAL(sal_uLong(223), rByteSize);
    OUString rSwapFileURL = aGraphic.ImplGetImpGraphic()->getSwapFileURL();
    CPPUNIT_ASSERT_EQUAL(true, rSwapFileURL.isEmpty());

    // Swapping out
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Byte size doesn't change when we swapped out
    // TODO: In case we don't trigger GetBitmapEx (above) the size is 0
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());

    // Let's check the swap file
    rSwapFileURL = aGraphic.ImplGetImpGraphic()->getSwapFileURL();
    CPPUNIT_ASSERT_EQUAL(true, comphelper::DirectoryHelper::fileExists(rSwapFileURL));

    { // Check the swap file content
        std::unique_ptr<SvStream> xStream = createStream(rSwapFileURL);
        CPPUNIT_ASSERT_EQUAL(true, bool(xStream));

        // Check size of the stream
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(353), xStream->remainingSize());

        std::vector<unsigned char> aHash = calculateHash(xStream);
        CPPUNIT_ASSERT_EQUAL(std::string("6ae83fc9c06ca253ada0b156d6e4700a4a028c34"),
                             toHexString(aHash));
    }

    // Let's swap in
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    CPPUNIT_ASSERT_EQUAL(aBitmapChecksumBeforeSwapping, aGraphic.GetBitmapEx().GetChecksum());

    // File shouldn't be available anymore
    CPPUNIT_ASSERT_EQUAL(false, comphelper::DirectoryHelper::fileExists(rSwapFileURL));
}

void GraphicTest::testSwappingPageNumber()
{
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(PDFEXPORT_DATA_DIRECTORY) + "SimpleMultiPagePDF.pdf";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Load the vector graphic
    CPPUNIT_ASSERT_EQUAL(true, bool(aGraphic.getVectorGraphicData()));
    // Set the page index
    aGraphic.getVectorGraphicData()->setPageIndex(1);

    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                         aGraphic.getVectorGraphicData()->getVectorGraphicDataType());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(17693),
                         aGraphic.getVectorGraphicData()->getVectorGraphicDataArrayLength());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aGraphic.getVectorGraphicData()->getPageIndex());

    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Swapping out
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Let's swap in
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aGraphic.getVectorGraphicData()->getPageIndex());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
