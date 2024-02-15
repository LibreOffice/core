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
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
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
#include <vcl/wmf.hxx>

#include <impgraph.hxx>
#include <graphic/GraphicFormatDetector.hxx>

#if USE_TLS_NSS
#include <nss.h>
#endif

using namespace css;

class GraphicTest : public test::BootstrapFixture
{
public:
    ~GraphicTest();
};

GraphicTest::~GraphicTest()
{
#if USE_TLS_NSS
    NSS_Shutdown();
#endif
}

namespace
{
BitmapEx createBitmap(bool alpha = false)
{
    Bitmap aBitmap(Size(120, 100), vcl::PixelFormat::N24_BPP);
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
    rGraphicFilter.ExportGraphic(aBitmapEx, u"none", rStream, nFilterFormat, &aFilterData);

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

std::vector<unsigned char> calculateHash(SvStream* pStream)
{
    comphelper::Hash aHashEngine(comphelper::HashType::SHA1);
    const sal_uInt32 nSize(pStream->remainingSize());
    std::vector<sal_uInt8> aData(nSize);
    aHashEngine.update(aData.data(), nSize);
    return aHashEngine.finalize();
}

bool checkBitmap(Graphic& rGraphic)
{
    bool bResult = true;

    Bitmap aBitmap(rGraphic.GetBitmapEx().GetBitmap());
    {
        BitmapScopedReadAccess pReadAccess(aBitmap);
        for (tools::Long y = 0; y < rGraphic.GetSizePixel().Height(); y++)
        {
            for (tools::Long x = 0; x < rGraphic.GetSizePixel().Width(); x++)
            {
                if (pReadAccess->HasPalette())
                {
                    sal_uInt32 nIndex = pReadAccess->GetPixelIndex(y, x);
                    Color aColor = pReadAccess->GetPaletteColor(nIndex);
                    bResult &= (aColor == COL_LIGHTRED);
                }
                else
                {
                    Color aColor = pReadAccess->GetPixel(y, x);
                    bResult &= (aColor == COL_LIGHTRED);
                }
            }
        }
    }

    return bResult;
}

constexpr OUString DATA_DIRECTORY = u"/vcl/qa/cppunit/data/"_ustr;
constexpr OUString PDFEXPORT_DATA_DIRECTORY = u"/vcl/qa/cppunit/pdfexport/data/"_ustr;

Graphic loadGraphic(std::u16string_view const& rFilename)
{
    test::Directories aDirectories;
    OUString aFilename = aDirectories.getURLFromSrc(DATA_DIRECTORY) + rFilename;
    SvFileStream aFileStream(aFilename, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();

    Graphic aGraphic;
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, rGraphicFilter.ImportGraphic(aGraphic, u"", aFileStream,
                                                                    GRFILTER_FORMAT_DONTKNOW));
    return aGraphic;
}

Graphic importUnloadedGraphic(std::u16string_view const& rFilename)
{
    test::Directories aDirectories;
    OUString aFilename = aDirectories.getURLFromSrc(DATA_DIRECTORY) + rFilename;
    SvFileStream aFileStream(aFilename, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    return rGraphicFilter.ImportUnloadedGraphic(aFileStream);
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

} //namespace

CPPUNIT_TEST_FIXTURE(GraphicTest, testUnloadedGraphic)
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

CPPUNIT_TEST_FIXTURE(GraphicTest, testUnloadedGraphicLoadingPng)
{
    Graphic aGraphic = makeUnloadedGraphic(u"png");

    // check available
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0);
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(true, checkBitmap(aGraphic));
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testUnloadedGraphicLoadingGif)
{
    Graphic aGraphic = makeUnloadedGraphic(u"gif");

    // check available
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0);
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(true, checkBitmap(aGraphic));
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testUnloadedGraphicLoadingJpg)
{
    Graphic aGraphic = makeUnloadedGraphic(u"jpg");

    // check available
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0);
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(false, checkBitmap(aGraphic));
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testUnloadedGraphicLoadingTif)
{
    Graphic aGraphic = makeUnloadedGraphic(u"tif");

    // check available
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0);
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(true, checkBitmap(aGraphic));
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testUnloadedGraphicLoadingWebp)
{
    Graphic aGraphic = makeUnloadedGraphic(u"webp");

    // check available
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > 0);
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(true, checkBitmap(aGraphic));
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testUnloadedGraphicWmf)
{
    // Create some in-memory WMF data, set its own preferred size to 99x99.
    BitmapEx aBitmapEx = createBitmap();
    SvMemoryStream aStream;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilterFormat = rGraphicFilter.GetExportFormatNumberForShortName(u"wmf");
    Graphic aGraphic(aBitmapEx);
    aGraphic.SetPrefSize(Size(99, 99));
    aGraphic.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
    rGraphicFilter.ExportGraphic(aGraphic, u"none", aStream, nFilterFormat);
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

CPPUNIT_TEST_FIXTURE(GraphicTest, testUnloadedGraphicAlpha)
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

CPPUNIT_TEST_FIXTURE(GraphicTest, testUnloadedGraphicSizeUnit)
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

CPPUNIT_TEST_FIXTURE(GraphicTest, testWMFRoundtrip)
{
    // Load a WMF file.
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(u"vcl/qa/cppunit/data/roundtrip.wmf");
    SvFileStream aStream(aURL, StreamMode::READ);
    sal_uInt64 nExpectedSize = aStream.TellEnd();
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);

    // Save as WMF.
    utl::TempFileNamed aTempFile;
    aTempFile.EnableKillingFile();
    sal_uInt16 nFormat = rGraphicFilter.GetExportFormatNumberForShortName(u"WMF");
    SvStream& rOutStream = *aTempFile.GetStream(StreamMode::READWRITE);
    rGraphicFilter.ExportGraphic(aGraphic, u"", rOutStream, nFormat);

    // Check if we preserved the WMF data perfectly.
    sal_uInt64 nActualSize = rOutStream.TellEnd();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6475
    // - Actual  : 2826
    // i.e. we lost some of the WMF data on roundtrip.
    CPPUNIT_ASSERT_EQUAL(nExpectedSize, nActualSize);
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testWMFWithEmfPlusRoundtrip)
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
        utl::TempFileNamed aTempFile;
        SvStream& rOutStream = *aTempFile.GetStream(StreamMode::READWRITE);
        if (useConvertMetafile)
            ConvertGraphicToWMF(aGraphic, rOutStream, nullptr);
        else
        {
            sal_uInt16 nFormat = rGraphicFilter.GetExportFormatNumberForShortName(u"WMF");
            rGraphicFilter.ExportGraphic(aGraphic, u"", rOutStream, nFormat);
        }
        CPPUNIT_ASSERT_EQUAL(nExpectedSize, rOutStream.TellEnd());

        rOutStream.Seek(0);
        Graphic aNewGraphic = rGraphicFilter.ImportUnloadedGraphic(rOutStream);
        // Check that reading the WMF back preserves the EMF+ actions in it.
        CPPUNIT_ASSERT_GREATER(0, getEmfPlusActionsCount(aNewGraphic));
        // EmfReader::ReadEnhWMF() drops non-EMF+ drawing actions if EMF+ is found.
        CPPUNIT_ASSERT_EQUAL(0, getPolygonActionsCount(aNewGraphic));

        // With EMF+ disabled there should be no EMF+ actions.
        auto aVectorGraphicData = std::make_shared<VectorGraphicData>(
            aNewGraphic.GetGfxLink().getDataContainer(), VectorGraphicDataType::Wmf);
        aVectorGraphicData->setEnableEMFPlus(false);
        Graphic aNoEmfPlusGraphic(aVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(0, getEmfPlusActionsCount(aNoEmfPlusGraphic));
        CPPUNIT_ASSERT_GREATER(0, getPolygonActionsCount(aNoEmfPlusGraphic));
    }
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testEmfToWmfConversion)
{
    // Load EMF data.
    GraphicFilter aGraphicFilter;
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "to-wmf.emf";
    SvFileStream aStream(aURL, StreamMode::READ);
    Graphic aGraphic;
    // This similar to an application/x-openoffice-wmf mime type in manifest.xml in the ODF case.
    sal_uInt16 nFormatEMF = aGraphicFilter.GetImportFormatNumberForShortName(u"EMF");
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE,
                         aGraphicFilter.ImportGraphic(aGraphic, u"", aStream, nFormatEMF));
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Emf, aGraphic.getVectorGraphicData()->getType());

    // Save as WMF.
    sal_uInt16 nFilterType = aGraphicFilter.GetExportFormatNumberForShortName(u"WMF");
    SvMemoryStream aGraphicStream;
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE,
                         aGraphicFilter.ExportGraphic(aGraphic, u"", aGraphicStream, nFilterType));
    aGraphicStream.Seek(0);
    vcl::GraphicFormatDetector aDetector(aGraphicStream, OUString());
    CPPUNIT_ASSERT(aDetector.detect());
    CPPUNIT_ASSERT(aDetector.checkWMF());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: WMF
    // - Actual  : EMF
    // i.e. EMF data was requested to be converted to WMF, but the output was still EMF.
    CPPUNIT_ASSERT_EQUAL(OUString("WMF"),
                         vcl::getImportFormatShortName(aDetector.getMetadata().mnFormat));

    // Import the WMF result and check for traces of EMF+ in it.
    Graphic aWmfGraphic;
    aGraphicStream.Seek(0);
    sal_uInt16 nFormatWMF = aGraphicFilter.GetImportFormatNumberForShortName(u"WMF");
    CPPUNIT_ASSERT_EQUAL(
        ERRCODE_NONE, aGraphicFilter.ImportGraphic(aWmfGraphic, u"", aGraphicStream, nFormatWMF));
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

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingGraphic_PNG_WithGfxLink)
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
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileStream() == nullptr);

    // Swapping out
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Byte size doesn't change when we swapped out
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());

    // Check the swap file (still shouldn't exist)
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileStream() == nullptr);

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

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingGraphic_PNG_WithoutGfxLink)
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
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileStream() == nullptr);

    // Swapping out
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Byte size doesn't change when we swapped out
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());

    // Let's check the swap file

    { // Check the swap file content
        SvStream* pStream = aGraphic.ImplGetImpGraphic()->getSwapFileStream();
        pStream->Seek(0);

        // Check size of the stream
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(36079), pStream->remainingSize());

        std::vector<unsigned char> aHash = calculateHash(pStream);
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
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileStream() == nullptr);

    // Check the bitmap
    CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphic.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(true, checkBitmap(aGraphic));
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingGraphicProperties_PNG_WithGfxLink)
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

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingGraphicProperties_PNG_WithoutGfxLink)
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

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingVectorGraphic_SVG_WithGfxLink)
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

    CPPUNIT_ASSERT_EQUAL(size_t(223), pVectorData->getBinaryDataContainer().getSize());

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
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileStream() == nullptr);

    // SWAP OUT the Graphic and make sure it's not available currently
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // We use GfxLink so no swap file in this case
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileStream() == nullptr);

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

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingVectorGraphic_SVG_WithoutGfxLink)
{
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "SimpleExample.svg";
    SvFileStream aStream(aURL, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();

    Graphic aInputGraphic = rGraphicFilter.ImportUnloadedGraphic(aStream);
    CPPUNIT_ASSERT_EQUAL(size_t(223),
                         aInputGraphic.getVectorGraphicData()->getBinaryDataContainer().getSize());

    // Create graphic
    Graphic aGraphic(aInputGraphic.getVectorGraphicData());

    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(true, bool(aGraphic.getVectorGraphicData()));
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());

    CPPUNIT_ASSERT_EQUAL(size_t(223),
                         aGraphic.getVectorGraphicData()->getBinaryDataContainer().getSize());

    CPPUNIT_ASSERT_EQUAL(false, aGraphic.IsGfxLink());

    BitmapChecksum aBitmapChecksumBeforeSwapping = aGraphic.GetBitmapEx().GetChecksum();

    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // Get the declared byte size of the graphic
    sal_uLong rByteSize = aGraphic.GetSizeBytes();
    CPPUNIT_ASSERT_EQUAL(sal_uLong(223), rByteSize);

    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileStream() == nullptr);

    // Swapping out
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Byte size doesn't change when we swapped out
    // TODO: In case we don't trigger GetBitmapEx (above) the size is 0
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());

    // Let's check the swap file
    {
        // Check the swap file content
        SvStream* pStream = aGraphic.ImplGetImpGraphic()->getSwapFileStream();
        pStream->Seek(0);

        // Check size of the stream
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(247), pStream->remainingSize());

        std::vector<unsigned char> aHash = calculateHash(pStream);
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

    size_t nVectorByteSize = aGraphic.getVectorGraphicData()->getBinaryDataContainer().getSize();
    CPPUNIT_ASSERT_EQUAL(size_t(223), nVectorByteSize);

    CPPUNIT_ASSERT_EQUAL(false, aGraphic.IsGfxLink());

    CPPUNIT_ASSERT_EQUAL(aBitmapChecksumBeforeSwapping, aGraphic.GetBitmapEx().GetChecksum());

    // File shouldn't be available anymore
    CPPUNIT_ASSERT_EQUAL(static_cast<SvStream*>(nullptr),
                         aGraphic.ImplGetImpGraphic()->getSwapFileStream());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingGraphicProperties_SVG_WithGfxLink)
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

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingGraphicProperties_SVG_WithoutGfxLink)
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
    CPPUNIT_ASSERT_EQUAL(size_t(223),
                         aInputGraphic.getVectorGraphicData()->getBinaryDataContainer().getSize());

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

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingVectorGraphic_PDF_WithGfxLink)
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

    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, aGraphic.getVectorGraphicData()->getType());
    CPPUNIT_ASSERT_EQUAL(size_t(17693),
                         aGraphic.getVectorGraphicData()->getBinaryDataContainer().getSize());
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

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingVectorGraphic_PDF_WithoutGfxLink)
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

    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, aGraphic.getVectorGraphicData()->getType());
    CPPUNIT_ASSERT_EQUAL(size_t(17693),
                         aGraphic.getVectorGraphicData()->getBinaryDataContainer().getSize());
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

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingAnimationGraphic_GIF_WithGfxLink)
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
    CPPUNIT_ASSERT_EQUAL(sal_uLong(89552), rByteSize);

    // Make sure we don't have a file
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileStream() == nullptr);

    // SWAP OUT the Graphic and make sure it's not available currently
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // We use GfxLink so no swap file in this case
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileStream() == nullptr);

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

CPPUNIT_TEST_FIXTURE(GraphicTest, testSwappingAnimationGraphic_GIF_WithoutGfxLink)
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
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->getSwapFileStream() == nullptr);

    // SWAP OUT
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());

    // Byte size doesn't change when we swapped out
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());

    // Let's check the swap file
    {
        // Check the swap file content
        SvStream* pStream = aGraphic.ImplGetImpGraphic()->getSwapFileStream();
        pStream->Seek(0);

        // Check size of the stream
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(15139), pStream->remainingSize());

        std::vector<unsigned char> aHash = calculateHash(pStream);
        CPPUNIT_ASSERT_EQUAL(std::string("ecae5354edd9cf98553eb3153e44181f56d35338"),
                             toHexString(aHash));
    }

    // SWAP IN
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(true, aGraphic.isAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphic.ImplGetImpGraphic()->isSwappedOut());

    // File shouldn't be available anymore
    CPPUNIT_ASSERT_EQUAL(static_cast<SvStream*>(nullptr),
                         aGraphic.ImplGetImpGraphic()->getSwapFileStream());

    // Check the bitmap
    CPPUNIT_ASSERT_EQUAL(tools::Long(124), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(146), aGraphic.GetSizePixel().Height());

    // Byte size is still the same
    CPPUNIT_ASSERT_EQUAL(rByteSize, aGraphic.GetSizeBytes());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testLoadMET)
{
    Graphic aGraphic = loadGraphic(u"TypeDetectionExample.met");
    CPPUNIT_ASSERT_EQUAL(GraphicType::GdiMetafile, aGraphic.GetType());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testLoadBMP)
{
    Graphic aGraphic = loadGraphic(u"TypeDetectionExample.bmp");
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Height());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testLoadPSD)
{
    Graphic aGraphic = loadGraphic(u"TypeDetectionExample.psd");
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Height());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testLoadTGA)
{
    Graphic aGraphic = loadGraphic(u"TypeDetectionExample.tga");
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Height());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testLoadXBM)
{
    Graphic aGraphic = loadGraphic(u"TypeDetectionExample.xbm");
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Height());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testLoadXPM)
{
    Graphic aGraphic = loadGraphic(u"TypeDetectionExample.xpm");
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Height());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testLoadPCX)
{
    Graphic aGraphic = loadGraphic(u"TypeDetectionExample.pcx");
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Height());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testLoadEPS)
{
    Graphic aGraphic = loadGraphic(u"TypeDetectionExample.eps");
    CPPUNIT_ASSERT_EQUAL(GraphicType::GdiMetafile, aGraphic.GetType());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testLoadWEBP)
{
    Graphic aGraphic = loadGraphic(u"TypeDetectionExample.webp");
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aGraphic.GetSizePixel().Height());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testLoadSVGZ)
{
    Graphic aGraphic = loadGraphic(u"TypeDetectionExample.svgz");
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    const auto[scalingX, scalingY] = getDPIScaling();
    CPPUNIT_ASSERT_EQUAL(tools::Long(100 * scalingX), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100 * scalingY), aGraphic.GetSizePixel().Height());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testTdf156016)
{
    // Without the fix in place, this test would have failed with
    // - Expected: 0x0(Error Area:Io Class:NONE Code:0)
    // - Actual  : 0x8203(Error Area:Vcl Class:General Code:3)
    Graphic aGraphic = loadGraphic(u"tdf156016.svg");
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    const auto[scalingX, scalingY] = getDPIScaling();
    CPPUNIT_ASSERT_EQUAL(tools::Long(100 * scalingX), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100 * scalingY), aGraphic.GetSizePixel().Height());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testTdf149545)
{
    // Without the fix in place, this test would have failed with
    // - Expected: 0x0(Error Area:Io Class:NONE Code:0)
    // - Actual  : 0x8203(Error Area:Vcl Class:General Code:3)
    Graphic aGraphic = loadGraphic(u"tdf149545.svg");
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    const auto[scalingX, scalingY] = getDPIScaling();
    CPPUNIT_ASSERT_EQUAL(tools::Long(100 * scalingX), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100 * scalingY), aGraphic.GetSizePixel().Height());
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testAvailableThreaded)
{
    Graphic jpgGraphic1 = importUnloadedGraphic(u"TypeDetectionExample.jpg");
    Graphic jpgGraphic2 = importUnloadedGraphic(u"Exif1.jpg");
    Graphic pngGraphic1 = importUnloadedGraphic(u"TypeDetectionExample.png");
    Graphic pngGraphic2 = importUnloadedGraphic(u"testBasicMorphology.png");
    std::vector<Graphic*> graphics = { &jpgGraphic1, &jpgGraphic2, &pngGraphic1, &pngGraphic2 };
    std::vector<Size> sizes;
    for (auto& graphic : graphics)
    {
        CPPUNIT_ASSERT(!graphic->isAvailable());
        sizes.push_back(graphic->GetSizePixel());
    }
    GraphicFilter& graphicFilter = GraphicFilter::GetGraphicFilter();
    graphicFilter.MakeGraphicsAvailableThreaded(graphics);
    int i = 0;
    for (auto& graphic : graphics)
    {
        CPPUNIT_ASSERT(graphic->isAvailable());
        CPPUNIT_ASSERT_EQUAL(sizes[i], graphic->GetSizePixel());
        ++i;
    }
}

CPPUNIT_TEST_FIXTURE(GraphicTest, testColorChangeToTransparent)
{
    Graphic aGraphic = importUnloadedGraphic(u"testColorChange-red-linear-gradient.png");

    auto xGraphic = aGraphic.GetXGraphic();
    uno::Reference<graphic::XGraphicTransformer> xGraphicTransformer{ xGraphic, uno::UNO_QUERY };
    ::Color nColorFrom{ ColorTransparency, 0x00, 0xFF, 0x00, 0x00 };
    ::Color nColorTo{ ColorTransparency, 0xFF, 0xFF, 0x00, 0x00 };
    sal_uInt8 nTolerance{ 15 };

    auto xGraphicAfter = xGraphicTransformer->colorChange(
        xGraphic, static_cast<sal_Int32>(nColorFrom), nTolerance, static_cast<sal_Int32>(nColorTo),
        static_cast<sal_Int8>(nColorTo.GetAlpha()));

    Graphic aGraphicAfter{ xGraphicAfter };
    const BitmapEx& rBitmapAfter = aGraphicAfter.GetBitmapExRef();
    const BitmapEx& rBitmapBefore = aGraphic.GetBitmapExRef();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: rgba[ff000000]
    // - Actual  : rgba[f00000ff]
    // i.e. the color change to transparent didn't apply correctly
    CPPUNIT_ASSERT_EQUAL(nColorTo, rBitmapAfter.GetPixelColor(386, 140));

    // Test if color stayed same on 410,140
    // colorChange with nTolerance 15 shouldn't change this pixel.
    CPPUNIT_ASSERT_EQUAL(rBitmapBefore.GetPixelColor(410, 140),
                         rBitmapAfter.GetPixelColor(410, 140));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
