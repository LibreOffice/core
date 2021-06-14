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
#include <unotools/tempfile.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/wmf.hxx>

#include <impgraph.hxx>
#include <graphic/GraphicFormatDetector.hxx>

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
    void testWMFRoundtrip();
    void testWMFWithEmfPlusRoundtrip();
    void testEmfToWmfConversion();

    CPPUNIT_TEST_SUITE(GraphicTest);
    CPPUNIT_TEST(testUnloadedGraphic);
    CPPUNIT_TEST(testUnloadedGraphicLoading);
    CPPUNIT_TEST(testUnloadedGraphicWmf);
    CPPUNIT_TEST(testUnloadedGraphicAlpha);
    CPPUNIT_TEST(testUnloadedGraphicSizeUnit);
    CPPUNIT_TEST(testSwapping);
    CPPUNIT_TEST(testSwappingVectorGraphic);
    CPPUNIT_TEST(testWMFRoundtrip);
    CPPUNIT_TEST(testWMFWithEmfPlusRoundtrip);
    CPPUNIT_TEST(testEmfToWmfConversion);
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

int getEmfPlusActionsCount(const Graphic& graphic)
{
    const GDIMetaFile& metafile = graphic.GetGDIMetaFile();
    int emfPlusCount = 0;
    for (size_t i = 0; i < metafile.GetActionSize(); ++i)
    {
        MetaAction* action = metafile.GetAction(i);
        if (action->GetType() == MetaActionType::COMMENT)
        {
            const MetaCommentAction* commentAction = static_cast<const MetaCommentAction*>(action);
            if (commentAction->GetComment() == "EMF_PLUS")
                ++emfPlusCount;
        }
    }
    return emfPlusCount;
}

int getPolygonActionsCount(const Graphic& graphic)
{
    const GDIMetaFile& metafile = graphic.GetGDIMetaFile();
    int polygonCount = 0;
    for (size_t i = 0; i < metafile.GetActionSize(); ++i)
    {
        MetaAction* action = metafile.GetAction(i);
        if (action->GetType() == MetaActionType::POLYGON)
            ++polygonCount;
    }
    return polygonCount;
}

void GraphicTest::testWMFWithEmfPlusRoundtrip()
{
    // Load a WMF file.
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(u"vcl/qa/cppunit/data/wmf-embedded-emfplus.wmf");
    SvFileStream aStream(aURL, StreamMode::READ);
    sal_uInt64 nExpectedSize = aStream.TellEnd();
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);

    CPPUNIT_ASSERT_GREATER(0, getEmfPlusActionsCount(aGraphic));
    CPPUNIT_ASSERT_EQUAL(0, getPolygonActionsCount(aGraphic));

    for (bool useConvertMetafile : { false, true })
    {
        // Save as WMF.
        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        SvStream& rOutStream = *aTempFile.GetStream(StreamMode::READWRITE);
        if (useConvertMetafile)
            ConvertGraphicToWMF(aGraphic, rOutStream, nullptr);
        else
        {
            sal_uInt16 nFormat = rGraphicFilter.GetExportFormatNumberForShortName(u"WMF");
            rGraphicFilter.ExportGraphic(aGraphic, OUString(), rOutStream, nFormat);
        }
        CPPUNIT_ASSERT_EQUAL(nExpectedSize, rOutStream.TellEnd());

        rOutStream.Seek(0);
        Graphic aNewGraphic = rGraphicFilter.ImportUnloadedGraphic(rOutStream);
        // Check that reading the WMF back preserves the EMF+ actions in it.
        CPPUNIT_ASSERT_GREATER(0, getEmfPlusActionsCount(aNewGraphic));
        // EmfReader::ReadEnhWMF() drops non-EMF+ drawing actions if EMF+ is found.
        CPPUNIT_ASSERT_EQUAL(0, getPolygonActionsCount(aNewGraphic));

        // With EMF+ disabled there should be no EMF+ actions.
        auto& rDataContainer = aNewGraphic.GetGfxLink().getDataContainer();
        auto aVectorGraphicData
            = std::make_shared<VectorGraphicData>(rDataContainer, VectorGraphicDataType::Wmf);
        aVectorGraphicData->setEnableEMFPlus(false);
        Graphic aNoEmfPlusGraphic(aVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(0, getEmfPlusActionsCount(aNoEmfPlusGraphic));
        CPPUNIT_ASSERT_GREATER(0, getPolygonActionsCount(aNoEmfPlusGraphic));
    }
}

void GraphicTest::testEmfToWmfConversion()
{
    // Load EMF data.
    GraphicFilter aGraphicFilter;
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "to-wmf.emf";
    SvFileStream aStream(aURL, StreamMode::READ);
    Graphic aGraphic;
    // This similar to an application/x-openoffice-wmf mime type in manifest.xml in the ODF case.
    sal_uInt16 nFormat = aGraphicFilter.GetImportFormatNumberForShortName(u"WMF");
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE,
                         aGraphicFilter.ImportGraphic(aGraphic, OUString(), aStream, nFormat));
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Wmf,
                         aGraphic.getVectorGraphicData()->getVectorGraphicDataType());

    // Save as WMF.
    sal_uInt16 nFilterType = aGraphicFilter.GetExportFormatNumberForShortName(u"WMF");
    SvMemoryStream aGraphicStream;
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, aGraphicFilter.ExportGraphic(aGraphic, OUString(),
                                                                    aGraphicStream, nFilterType));
    aGraphicStream.Seek(0);
    vcl::GraphicFormatDetector aDetector(aGraphicStream, OUString());
    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkWMForEMF());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: WMF
    // - Actual  : EMF
    // i.e. EMF data was requested to be converted to WMF, but the output was still EMF.
    CPPUNIT_ASSERT_EQUAL(OUString("WMF"), aDetector.msDetectedFormat);

    // Import the WMF result and check for traces of EMF+ in it.
    Graphic aWmfGraphic;
    aGraphicStream.Seek(0);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, aGraphicFilter.ImportGraphic(aWmfGraphic, OUString(),
                                                                    aGraphicStream, nFormat));
    int nCommentCount = 0;
    for (size_t i = 0; i < aWmfGraphic.GetGDIMetaFile().GetActionSize(); ++i)
    {
        MetaAction* pAction = aWmfGraphic.GetGDIMetaFile().GetAction(i);
        if (pAction->GetType() == MetaActionType::COMMENT)
        {
            auto pComment = static_cast<MetaCommentAction*>(pAction);
            if (pComment->GetComment().startsWith("EMF_PLUS"))
            {
                ++nCommentCount;
            }
        }
    }
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less or equal than: 4
    // - Actual  : 8
    // i.e. even more EMF+ comments were left in the WMF output. The ideal would be to get this down
    // to 0, though.
    CPPUNIT_ASSERT_LESSEQUAL(4, nCommentCount);
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
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(445), xStream->remainingSize());

        std::vector<unsigned char> aHash = calculateHash(xStream);
        CPPUNIT_ASSERT_EQUAL(std::string("304f17d9c56e79b95f6c337dab88709d4f9b61f0"),
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
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(349), xStream->remainingSize());

        std::vector<unsigned char> aHash = calculateHash(xStream);
        CPPUNIT_ASSERT_EQUAL(std::string("88b4c1c359e3cf7be005fbb46c93ffa6de9dcf4a"),
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

void GraphicTest::testWMFRoundtrip()
{
    // Load a WMF file.
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc("vcl/qa/cppunit/data/roundtrip.wmf");
    SvFileStream aStream(aURL, StreamMode::READ);
    sal_uInt64 nExpectedSize = aStream.TellEnd();
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);

    // Save as WMF.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    sal_uInt16 nFormat = rGraphicFilter.GetExportFormatNumberForShortName(u"WMF");
    SvStream& rOutStream = *aTempFile.GetStream(StreamMode::READWRITE);
    rGraphicFilter.ExportGraphic(aGraphic, OUString(), rOutStream, nFormat);

    // Check if we preserved the WMF data perfectly.
    sal_uInt64 nActualSize = rOutStream.TellEnd();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6475
    // - Actual  : 2826
    // i.e. we lost some of the WMF data on roundtrip.
    CPPUNIT_ASSERT_EQUAL(nExpectedSize, nActualSize);
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
