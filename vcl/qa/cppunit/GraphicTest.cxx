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
#include <test/bootstrapfixture.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/BitmapReadAccess.hxx>
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

#include <impgraph.hxx>
#include <graphic/GraphicFormatDetector.hxx>

#if USE_TLS_NSS
#include <nss.h>
#endif

using namespace css;

namespace
{
class GraphicTest : public test::BootstrapFixture
{
public:
    ~GraphicTest();

private:
    void testUnloadedGraphic();
    void testUnloadedGraphicLoading();
    void testUnloadedGraphicWmf();
    void testUnloadedGraphicAlpha();
    void testUnloadedGraphicSizeUnit();

    void testWMFRoundtrip();
    void testEmfToWmfConversion();

    void testSwappingGraphic_PNG_WithGfxLink();
    void testSwappingGraphic_PNG_WithoutGfxLink();
    void testSwappingGraphicProperties_PNG_WithGfxLink();
    void testSwappingGraphicProperties_PNG_WithoutGfxLink();

    void testSwappingVectorGraphic_SVG_WithGfxLink();
    void testSwappingVectorGraphic_SVG_WithoutGfxLink();
    void testSwappingGraphicProperties_SVG_WithGfxLink();
    void testSwappingGraphicProperties_SVG_WithoutGfxLink();

    void testSwappingVectorGraphic_PDF_WithGfxLink();
    void testSwappingVectorGraphic_PDF_WithoutGfxLink();

    void testSwappingAnimationGraphic_GIF_WithGfxLink();
    void testSwappingAnimationGraphic_GIF_WithoutGfxLink();

    CPPUNIT_TEST_SUITE(GraphicTest);
    CPPUNIT_TEST(testUnloadedGraphic);
    CPPUNIT_TEST(testUnloadedGraphicLoading);
    CPPUNIT_TEST(testUnloadedGraphicWmf);
    CPPUNIT_TEST(testUnloadedGraphicAlpha);
    CPPUNIT_TEST(testUnloadedGraphicSizeUnit);
    CPPUNIT_TEST(testWMFRoundtrip);
    CPPUNIT_TEST(testEmfToWmfConversion);

    CPPUNIT_TEST(testSwappingGraphic_PNG_WithGfxLink);
    CPPUNIT_TEST(testSwappingGraphic_PNG_WithoutGfxLink);
    CPPUNIT_TEST(testSwappingGraphicProperties_PNG_WithGfxLink);
    CPPUNIT_TEST(testSwappingGraphicProperties_PNG_WithoutGfxLink);

    CPPUNIT_TEST(testSwappingVectorGraphic_SVG_WithGfxLink);
    CPPUNIT_TEST(testSwappingVectorGraphic_SVG_WithoutGfxLink);
    CPPUNIT_TEST(testSwappingGraphicProperties_SVG_WithGfxLink);
    CPPUNIT_TEST(testSwappingGraphicProperties_SVG_WithoutGfxLink);

    CPPUNIT_TEST(testSwappingVectorGraphic_PDF_WithGfxLink);
    CPPUNIT_TEST(testSwappingVectorGraphic_PDF_WithoutGfxLink);

    CPPUNIT_TEST(testSwappingAnimationGraphic_GIF_WithGfxLink);
    CPPUNIT_TEST(testSwappingAnimationGraphic_GIF_WithoutGfxLink);
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

void createBitmapAndExportForType(SvStream& rStream, std::u16string_view sType, bool alpha)
{
    BitmapEx aBitmapEx = createBitmap(alpha);

    uno::Sequence<beans::PropertyValue> aFilterData;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilterFormat = rGraphicFilter.GetExportFormatNumberForShortName(sType);
    rGraphicFilter.ExportGraphic(aBitmapEx, "none", rStream, nFilterFormat, &aFilterData);

    rStream.Seek(STREAM_SEEK_TO_BEGIN);
}

Graphic makeUnloadedGraphic(std::u16string_view sType, bool alpha = false)
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
        for (tools::Long y = 0; y < rGraphic.GetSizePixel().Height(); y++)
        {
            for (tools::Long x = 0; x < rGraphic.GetSizePixel().Width(); x++)
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

OUStringLiteral const DATA_DIRECTORY = u"/vcl/qa/cppunit/data/";
OUStringLiteral const PDFEXPORT_DATA_DIRECTORY = u"/vcl/qa/cppunit/pdfexport/data/";

void GraphicTest::testUnloadedGraphic()
{
    // make unloaded test graphic
    Graphic aGraphic = makeUnloadedGraphic(u"png");
    Graphic aGraphic2 = aGraphic;

    // check available
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic2.isAvailable());

    CPPUNIT_ASSERT_EQUAL(true, aGraphic2.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic2.isAvailable());

    // check GetSizePixel doesn't load graphic
    aGraphic = makeUnloadedGraphic(u"png");
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // check GetPrefSize doesn't load graphic
    CPPUNIT_ASSERT_EQUAL(tools::Long(6000), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(5000), aGraphic.GetPrefSize().Height());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // check GetSizeBytes loads graphic
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0);
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    //check Type
    aGraphic = makeUnloadedGraphic(u"png");
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
}

void GraphicTest::testUnloadedGraphicLoading()
{
    const OUString aFormats[] = { "png", "gif", "jpg", "tif" };

    for (OUString const& sFormat : aFormats)
    {
        Graphic aGraphic = makeUnloadedGraphic(sFormat);

        // check available
        CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
        CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());
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
    sal_uInt16 nFilterFormat = rGraphicFilter.GetExportFormatNumberForShortName(u"wmf");
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
    Graphic aGraphic = makeUnloadedGraphic(u"png", true);

    CPPUNIT_ASSERT_EQUAL(true, aGraphic.IsAlpha());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.IsTransparent());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // make unloaded test graphic without alpha
    aGraphic = makeUnloadedGraphic(u"png", false);

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

    CPPUNIT_ASSERT_EQUAL(Size(42, 42), aGraphic.GetPrefSize());

    // Force it to swap in
    aGraphic.makeAvailable();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 400x363
    // - Actual  : 42x42
    // i.e. a mm100 size was used as a hint and the inch size was set for a non-matching unit.
    CPPUNIT_ASSERT_EQUAL(Size(400, 363), aGraphic.GetPrefSize());
}

void GraphicTest::testWMFRoundtrip()
{
    // Load a WMF file.
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(u"vcl/qa/cppunit/data/roundtrip.wmf");
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

void GraphicTest::testSwappingGraphic_PNG_WithGfxLink()
{
    // Prepare Graphic from a PNG image first
    Graphic aGraphic = makeUnloadedGraphic(u"png");

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());

    BitmapChecksum aChecksumBeforeSwapping = aGraphic.GetChecksum();

    CPPUNIT_ASSERT_EQUAL(true, aGraphic.IsGfxLink());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(319), aGraphic.GetGfxLink().GetDataSize());

    // We loaded the Graphic and made it available
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    // Get the declared byte size of the graphic
    sal_uLong rByteSize = aGraphic.GetSizeBytes();

    // Check the swap file (shouldn't exist)
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileURL().isEmpty());

    // Swapping out
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Byte size doesn't change when we swapped out
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());

    // Check the swap file (still shouldn't exist)
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileURL().isEmpty());

    // Let's swap in
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    CPPUNIT_ASSERT_EQUAL(aChecksumBeforeSwapping, aGraphic.GetChecksum());

    // Check the bitmap
    CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(true, checkBitmap(aGraphic));
}

void GraphicTest::testSwappingGraphic_PNG_WithoutGfxLink()
{
    // Prepare Graphic from a PNG image first

    // Make sure to construct the Graphic from BitmapEx, so that we
    // don't have the GfxLink present.
    Graphic aGraphic(makeUnloadedGraphic(u"png").GetBitmapEx());

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());

    BitmapChecksum aChecksumBeforeSwapping = aGraphic.GetChecksum();

    CPPUNIT_ASSERT_EQUAL(false, aGraphic.IsGfxLink());

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
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(36079), xStream->remainingSize());

        std::vector<unsigned char> aHash = calculateHash(xStream);
        CPPUNIT_ASSERT_EQUAL(std::string("9347511e3b80dfdfaadf91a3bdef55a8ae85552b"),
                             toHexString(aHash));
    }

    // SWAP IN
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // reset the checksum to make sure we don't get the cached value
    aGraphic.ImplGetImpGraphic()->resetChecksum();
    CPPUNIT_ASSERT_EQUAL(aChecksumBeforeSwapping, aGraphic.GetChecksum());

    // File shouldn't be available anymore
    CPPUNIT_ASSERT_EQUAL(false, comphelper::DirectoryHelper::fileExists(rSwapFileURL));

    // Check the bitmap
    CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(true, checkBitmap(aGraphic));
}

void GraphicTest::testSwappingGraphicProperties_PNG_WithGfxLink()
{
    // Prepare Graphic from a PNG image
    Graphic aGraphic = makeUnloadedGraphic(u"png");

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    // Origin URL
    aGraphic.setOriginURL("Origin URL");
    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());

    //Set PrefMapMode
    CPPUNIT_ASSERT_EQUAL(MapUnit::Map100thMM, aGraphic.GetPrefMapMode().GetMapUnit());
    aGraphic.SetPrefMapMode(MapMode(MapUnit::MapTwip));
    CPPUNIT_ASSERT_EQUAL(MapUnit::MapTwip, aGraphic.GetPrefMapMode().GetMapUnit());

    // Set the PrefSize
    CPPUNIT_ASSERT_EQUAL(tools::Long(6000), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(5000), aGraphic.GetPrefSize().Height());
    aGraphic.SetPrefSize(Size(200, 100));
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());

    // SWAP OUT
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Check properties
    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());
    CPPUNIT_ASSERT_EQUAL(MapUnit::MapTwip, aGraphic.GetPrefMapMode().GetMapUnit());
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());

    // SWAP IN
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Check properties
    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());
    CPPUNIT_ASSERT_EQUAL(MapUnit::MapTwip, aGraphic.GetPrefMapMode().GetMapUnit());
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());
}

void GraphicTest::testSwappingGraphicProperties_PNG_WithoutGfxLink()
{
    // Prepare Graphic from a PNG image
    Graphic aGraphic(makeUnloadedGraphic(u"png").GetBitmapEx());

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    // Origin URL
    aGraphic.setOriginURL("Origin URL");
    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());

    //Set PrefMapMode
    CPPUNIT_ASSERT_EQUAL(MapUnit::Map100thMM, aGraphic.GetPrefMapMode().GetMapUnit());
    aGraphic.SetPrefMapMode(MapMode(MapUnit::MapTwip));
    CPPUNIT_ASSERT_EQUAL(MapUnit::MapTwip, aGraphic.GetPrefMapMode().GetMapUnit());

    // Set the PrefSize
    CPPUNIT_ASSERT_EQUAL(tools::Long(6000), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(5000), aGraphic.GetPrefSize().Height());
    aGraphic.SetPrefSize(Size(200, 100));
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());

    // SWAP OUT
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Check properties
    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());
    CPPUNIT_ASSERT_EQUAL(MapUnit::MapTwip, aGraphic.GetPrefMapMode().GetMapUnit());
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());

    // SWAP IN
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Check properties
    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());
    CPPUNIT_ASSERT_EQUAL(MapUnit::MapTwip, aGraphic.GetPrefMapMode().GetMapUnit());
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());
}

void GraphicTest::testSwappingVectorGraphic_SVG_WithGfxLink()
{
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "SimpleExample.svg";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    // Loaded into "prepared" state

    // Check that the state is as expected
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Load the vector graphic
    auto pVectorData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT_EQUAL(true, bool(pVectorData));
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(sal_uInt32(223), pVectorData->getVectorGraphicDataArrayLength());

    CPPUNIT_ASSERT_EQUAL(true, aGraphic.IsGfxLink());
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(223), aGraphic.GetGfxLink().GetDataSize());

    // Remember checksum so we can compare after swapping back in again
    BitmapChecksum aBitmapChecksumBeforeSwapping = aGraphic.GetBitmapEx().GetChecksum();

    // Check we are not swapped out yet
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Get the declared byte size of the graphic
    sal_uLong rByteSize = aGraphic.GetSizeBytes();
    CPPUNIT_ASSERT_EQUAL(sal_uLong(223), rByteSize);

    // Make sure we don't have a file
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileURL().isEmpty());

    // SWAP OUT the Graphic and make sure it's not available currently
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // We use GfxLink so no swap file in this case
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileURL().isEmpty());

    // Byte size doesn't change when we swapped out
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());

    // SWAP IN
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Compare that the checksum of the bitmap is still the same
    CPPUNIT_ASSERT_EQUAL(aBitmapChecksumBeforeSwapping, aGraphic.GetBitmapEx().GetChecksum());

    // Byte size shouldn't change
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());
}

void GraphicTest::testSwappingVectorGraphic_SVG_WithoutGfxLink()
{
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "SimpleExample.svg";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();

    Graphic aInputGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(223),
                         aInputGraphic.getVectorGraphicData()->getVectorGraphicDataArrayLength());

    // Create graphic
    Graphic aGraphic(aInputGraphic.getVectorGraphicData());

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, bool(aGraphic.getVectorGraphicData()));
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(sal_uInt32(223),
                         aGraphic.getVectorGraphicData()->getVectorGraphicDataArrayLength());

    CPPUNIT_ASSERT_EQUAL(false, aGraphic.IsGfxLink());

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
    CPPUNIT_ASSERT_EQUAL(false, rSwapFileURL.isEmpty());
    CPPUNIT_ASSERT_EQUAL(true, comphelper::DirectoryHelper::fileExists(rSwapFileURL));

    {
        // Check the swap file content
        std::unique_ptr<SvStream> xStream = createStream(rSwapFileURL);
        CPPUNIT_ASSERT_EQUAL(true, bool(xStream));

        // Check size of the stream
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(247), xStream->remainingSize());

        std::vector<unsigned char> aHash = calculateHash(xStream);
        CPPUNIT_ASSERT_EQUAL(std::string("666820973fd95e6cd9e7bc5f1c53732acbc99326"),
                             toHexString(aHash));
    }

    // Let's swap in
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    // Check the Graphic
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, bool(aGraphic.getVectorGraphicData()));

    sal_uInt32 nVectorByteSize = aGraphic.getVectorGraphicData()->getVectorGraphicDataArrayLength();
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(223), nVectorByteSize);

    CPPUNIT_ASSERT_EQUAL(false, aGraphic.IsGfxLink());

    CPPUNIT_ASSERT_EQUAL(aBitmapChecksumBeforeSwapping, aGraphic.GetBitmapEx().GetChecksum());

    // File shouldn't be available anymore
    CPPUNIT_ASSERT_EQUAL(false, comphelper::DirectoryHelper::fileExists(rSwapFileURL));
}

void GraphicTest::testSwappingGraphicProperties_SVG_WithGfxLink()
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // We check that Graphic properties like MapMode, PrefSize are properly
    // restored through a swap cycle

    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "SimpleExample.svg";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    // Loaded into "prepared" state

    // Load the vector graphic
    auto pVectorData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT_EQUAL(true, bool(pVectorData));
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    // Origin URL
    aGraphic.setOriginURL("Origin URL");
    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());

    // Check size in pixels
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Height());

    // Set and check the PrefSize
    CPPUNIT_ASSERT_EQUAL(tools::Long(1349), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1349), aGraphic.GetPrefSize().Height());
    aGraphic.SetPrefSize(Size(200, 100));
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());

    // SWAP OUT the Graphic and make sure it's not available currently
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Check properties
    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Height());

    // SWAP IN
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Check properties
    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Height());
}

void GraphicTest::testSwappingGraphicProperties_SVG_WithoutGfxLink()
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "SimpleExample.svg";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();

    Graphic aInputGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(223),
                         aInputGraphic.getVectorGraphicData()->getVectorGraphicDataArrayLength());

    // Create graphic
    Graphic aGraphic(aInputGraphic.getVectorGraphicData());

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, bool(aGraphic.getVectorGraphicData()));
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.IsGfxLink());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Origin URL
    aGraphic.setOriginURL("Origin URL");
    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());

    // Check size in pixels
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Height());

    // Set and check the PrefSize
    CPPUNIT_ASSERT_EQUAL(tools::Long(1349), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1349), aGraphic.GetPrefSize().Height());
    aGraphic.SetPrefSize(Size(200, 100));
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());

    // SWAP OUT the Graphic and make sure it's not available currently
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());

    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Height());

    // SWAP IN
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    CPPUNIT_ASSERT_EQUAL(OUString("Origin URL"), aGraphic.getOriginURL());

    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aGraphic.GetPrefSize().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetPrefSize().Height());
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(51), aGraphic.GetSizePixel().Height());
}

void GraphicTest::testSwappingVectorGraphic_PDF_WithGfxLink()
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

    // SWAP OUT
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // SWAP IN
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aGraphic.getVectorGraphicData()->getPageIndex());
}

void GraphicTest::testSwappingVectorGraphic_PDF_WithoutGfxLink()
{
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(PDFEXPORT_DATA_DIRECTORY) + "SimpleMultiPagePDF.pdf";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aInputGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);

    // Create graphic
    Graphic aGraphic(aInputGraphic.getVectorGraphicData());

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
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

    // SWAP OUT
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // SWAP IN
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aGraphic.getVectorGraphicData()->getPageIndex());
}

void GraphicTest::testSwappingAnimationGraphic_GIF_WithGfxLink()
{
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "123_Numbers.gif";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    // Loaded into "prepared" state

    // Check that the state is as expected
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(true, aGraphic.IsGfxLink());

    CPPUNIT_ASSERT_EQUAL(tools::Long(124), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(146), aGraphic.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1515), aGraphic.GetGfxLink().GetDataSize());

    // Remember checksum so we can compare after swapping back in again
    BitmapChecksum aBitmapChecksumBeforeSwapping = aGraphic.GetBitmapEx().GetChecksum();

    // Check we are not swapped out yet
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Get the declared byte size of the graphic
    sal_uLong rByteSize = aGraphic.GetSizeBytes();
    CPPUNIT_ASSERT_EQUAL(sal_uLong(50373), rByteSize);

    // Make sure we don't have a file
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileURL().isEmpty());

    // SWAP OUT the Graphic and make sure it's not available currently
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // We use GfxLink so no swap file in this case
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileURL().isEmpty());

    // Byte size doesn't change when we swapped out
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());

    // SWAP IN
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Compare that the checksum of the bitmap is still the same
    CPPUNIT_ASSERT_EQUAL(aBitmapChecksumBeforeSwapping, aGraphic.GetBitmapEx().GetChecksum());

    // Byte size shouldn't change
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());
}

void GraphicTest::testSwappingAnimationGraphic_GIF_WithoutGfxLink()
{
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "123_Numbers.gif";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aInputGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    Graphic aGraphic(aInputGraphic.GetAnimation());

    // Check animation graphic
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.IsAnimated());

    CPPUNIT_ASSERT_EQUAL(tools::Long(124), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(146), aGraphic.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(false, aGraphic.IsGfxLink());

    // We loaded the Graphic and made it available
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Get the declared byte size of the graphic
    sal_uLong rByteSize = aGraphic.GetSizeBytes();
    OUString rSwapFileURL = aGraphic.ImplGetImpGraphic()->getSwapFileURL();
    CPPUNIT_ASSERT_EQUAL(true, rSwapFileURL.isEmpty());

    // SWAP OUT
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Byte size doesn't change when we swapped out
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());

    // Let's check the swap file
    rSwapFileURL = aGraphic.ImplGetImpGraphic()->getSwapFileURL();
    CPPUNIT_ASSERT_EQUAL(true, comphelper::DirectoryHelper::fileExists(rSwapFileURL));

    {
        // Check the swap file content
        std::unique_ptr<SvStream> xStream = createStream(rSwapFileURL);
        CPPUNIT_ASSERT_EQUAL(true, bool(xStream));

        // Check size of the stream
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(15183), xStream->remainingSize());

        std::vector<unsigned char> aHash = calculateHash(xStream);
        CPPUNIT_ASSERT_EQUAL(std::string("deb13fdf0ffa0b58ce92fff0a6ca9e98c5d26ed9"),
                             toHexString(aHash));
    }

    // SWAP IN
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // File shouldn't be available anymore
    CPPUNIT_ASSERT_EQUAL(false, comphelper::DirectoryHelper::fileExists(rSwapFileURL));

    // Check the bitmap
    CPPUNIT_ASSERT_EQUAL(tools::Long(124), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(146), aGraphic.GetSizePixel().Height());

    // Byte size is still the same
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
