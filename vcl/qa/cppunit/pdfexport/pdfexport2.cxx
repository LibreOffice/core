/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <memory>
#include <string_view>

#include <config_fonts.h>
#include <osl/process.h>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <comphelper/scopeguard.hxx>
#include <comphelper/propertysequence.hxx>
#include <test/unoapi_test.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <tools/zcodec.hxx>
#include <tools/XmlWalker.hxx>
#include <vcl/graphicfilter.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <unotools/streamwrap.hxx>
#include <rtl/math.hxx>
#include <o3tl/string_view.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <printdata.hxx>
#include <unotxdoc.hxx>
#include <doc.hxx>
#include <docsh.hxx>

#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/pdfread.hxx>
#include <comphelper/propertyvalue.hxx>
#include <cmath>

using namespace ::com::sun::star;

namespace
{
/// Tests the PDF export filter.
class PdfExportTest2 : public UnoApiTest
{
protected:
    utl::MediaDescriptor aMediaDescriptor;

public:
    PdfExportTest2()
        : UnoApiTest(u"/vcl/qa/cppunit/pdfexport/data/"_ustr)
    {
    }

    void saveAsPDF(std::u16string_view rFile);
    void load(std::u16string_view rFile, vcl::filter::PDFDocument& rDocument,
              bool bUseTaggedPDF = true);
};

void PdfExportTest2::saveAsPDF(std::u16string_view rFile)
{
    // Import the bugdoc and export as PDF.
    loadFromFile(rFile);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
}

void PdfExportTest2::load(std::u16string_view rFile, vcl::filter::PDFDocument& rDocument,
                          bool bUseTaggedPDF)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "UseTaggedPDF", uno::Any(bUseTaggedPDF) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(rFile);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(rDocument.Read(aStream));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf124272)
{
    // Import the bugdoc and export as PDF.
    vcl::filter::PDFDocument aDocument;
    load(u"tdf124272.odt", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    // The page has a stream.
    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    OString aBitmap("Q q 299.899 782.189 m\n"
                    "55.2 435.889 l 299.899 435.889 l 299.899 782.189 l\n"
                    "h"_ostr);

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* pEnd = pStart + aUncompressed.GetSize();
    auto it = std::search(pStart, pEnd, aBitmap.getStr(), aBitmap.getStr() + aBitmap.getLength());
    CPPUNIT_ASSERT(it != pEnd);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf121615)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf121615.odt", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    // Get access to the only image on the only page.
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);
    vcl::filter::PDFStreamElement* pStream = pXObject->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();

    // Load the embedded image.
    rObjectStream.Seek(0);
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    sal_uInt16 format;
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, u"import", rObjectStream,
                                            GRFILTER_FORMAT_DONTKNOW, &format);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

    // The image should be grayscale 8bit JPEG.
    sal_uInt16 jpegFormat = rFilter.GetImportFormatNumberForShortName(JPG_SHORTNAME);
    CPPUNIT_ASSERT(jpegFormat != GRFILTER_FORMAT_NOTFOUND);
    CPPUNIT_ASSERT_EQUAL(jpegFormat, format);
    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(300), aBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N8_BPP, aBitmap.getPixelFormat());
    // tdf#121615 was caused by broken handling of data width with 8bit color,
    // so the test image has some black in the bottomright corner, check it's there
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aBitmap.GetPixelColor(0, 299));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aBitmap.GetPixelColor(199, 0));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmap.GetPixelColor(199, 299));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf141171)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf141171.odt", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    // Get access to the only image on the only page.
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);
    vcl::filter::PDFStreamElement* pStream = pXObject->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();

    // Load the embedded image.
    rObjectStream.Seek(0);
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    sal_uInt16 format;
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, u"import", rObjectStream,
                                            GRFILTER_FORMAT_DONTKNOW, &format);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

    // The image should be grayscale 8bit JPEG.
    sal_uInt16 jpegFormat = rFilter.GetImportFormatNumberForShortName(JPG_SHORTNAME);
    CPPUNIT_ASSERT(jpegFormat != GRFILTER_FORMAT_NOTFOUND);
    CPPUNIT_ASSERT_EQUAL(jpegFormat, format);
    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    Size aSize = aBitmap.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(tools::Long(878), aSize.Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(127), aSize.Height());
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N8_BPP, aBitmap.getPixelFormat());

    for (tools::Long nX = 0; nX < aSize.Width(); ++nX)
    {
        for (tools::Long nY = 0; nY < aSize.Height(); ++nY)
        {
            // Check all pixels in the image are white
            // Without the fix in place, this test would have failed with
            // - Expected: Color: R:255 G:255 B:255 A:0
            // - Actual  : Color: R:0 G:0 B:0 A:0
            CPPUNIT_ASSERT_EQUAL(COL_WHITE, aBitmap.GetPixelColor(nX, nY));
        }
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf129085)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf129085.docx", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    // Get access to the only image on the only page.
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);
    vcl::filter::PDFStreamElement* pStream = pXObject->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();

    // Load the embedded image.
    rObjectStream.Seek(0);
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    sal_uInt16 format;
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, u"import", rObjectStream,
                                            GRFILTER_FORMAT_DONTKNOW, &format);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

    sal_uInt16 jpegFormat = rFilter.GetImportFormatNumberForShortName(JPG_SHORTNAME);
    CPPUNIT_ASSERT(jpegFormat != GRFILTER_FORMAT_NOTFOUND);
    CPPUNIT_ASSERT_EQUAL(jpegFormat, format);
    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(tools::Long(884), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(925), aBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N24_BPP, aBitmap.getPixelFormat());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTocLink)
{
    // Load the Writer document.
    loadFromFile(u"toc-link.fodt");

    // Update the ToC.
    uno::Reference<text::XDocumentIndexesSupplier> xDocumentIndexesSupplier(mxComponent,
                                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocumentIndexesSupplier.is());

    uno::Reference<util::XRefreshable> xToc(
        xDocumentIndexesSupplier->getDocumentIndexes()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xToc.is());

    xToc->refresh();

    // Save as PDF.
    save(u"writer_pdf_Export"_ustr);

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Ensure there is a link on the first page (in the ToC).
    // Without the accompanying fix in place, this test would have failed, as the page contained no
    // links.
    CPPUNIT_ASSERT(pPdfPage->hasLinks());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testReduceSmallImage)
{
    // Load the Writer document.
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    saveAsPDF(u"reduce-small-image.fodt");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Image, pPageObject->getType());

    // Make sure we don't scale down a tiny bitmap.
    std::unique_ptr<vcl::pdf::PDFiumBitmap> pBitmap = pPageObject->getImageBitmap();
    CPPUNIT_ASSERT(pBitmap);
    int nWidth = pBitmap->getWidth();
    int nHeight = pBitmap->getHeight();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 16
    // - Actual  : 6
    // i.e. the image was scaled down to 300 DPI, even if it had tiny size.
    CPPUNIT_ASSERT_EQUAL(16, nWidth);
    CPPUNIT_ASSERT_EQUAL(16, nHeight);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf114256)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"calc_pdf_Export"_ustr;
    saveAsPDF(u"tdf114256.ods");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Without the fix in place, this test would have failed with
    // - Expected: 13
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(13, pPdfPage->getObjectCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf150931)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"calc_pdf_Export"_ustr;
    saveAsPDF(u"tdf150931.ods");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    int nPageObjectCount = pPdfPage->getObjectCount();
    // Without the fix in place, this test would have failed with
    // - Expected: 15
    // - Actual  : 16
    CPPUNIT_ASSERT_EQUAL(16, nPageObjectCount);

    int nYellowPathCount = 0;
    int nBlackPathCount = 0;
    int nGrayPathCount = 0;
    int nRedPathCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPdfPageObject = pPdfPage->getObject(i);
        if (pPdfPageObject->getType() != vcl::pdf::PDFPageObjectType::Path)
            continue;

        int nSegments = pPdfPageObject->getPathSegmentCount();
        CPPUNIT_ASSERT_EQUAL(5, nSegments);

        if (pPdfPageObject->getFillColor() == COL_YELLOW)
            ++nYellowPathCount;
        else if (pPdfPageObject->getFillColor() == COL_BLACK)
            ++nBlackPathCount;
        else if (pPdfPageObject->getFillColor() == COL_GRAY)
            ++nGrayPathCount;
        else if (pPdfPageObject->getFillColor() == COL_LIGHTRED)
            ++nRedPathCount;
    }

    CPPUNIT_ASSERT_EQUAL(3, nYellowPathCount);
    CPPUNIT_ASSERT_EQUAL(3, nRedPathCount);
    CPPUNIT_ASSERT_EQUAL(3, nGrayPathCount);
    CPPUNIT_ASSERT_EQUAL(3, nBlackPathCount);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf147027)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // Load the Calc document.
    aMediaDescriptor[u"FilterName"_ustr] <<= u"calc_pdf_Export"_ustr;
    saveAsPDF(u"tdf147027.ods");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Without the fix in place, this test would have failed with
    // - Expected: 778
    // - Actual  : 40
    CPPUNIT_ASSERT_EQUAL(778, pPdfPage->getObjectCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf135346)
{
    // Load the Calc document.
    aMediaDescriptor[u"FilterName"_ustr] <<= u"calc_pdf_Export"_ustr;
    saveAsPDF(u"tdf135346.ods");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Without the fix in place, this test would have failed with
    // - Expected: 56
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(56, pPdfPage->getObjectCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf147164)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"impress_pdf_Export"_ustr;
    saveAsPDF(u"tdf147164.odp");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/1);
    CPPUNIT_ASSERT(pPdfPage);

    // Without the fix in place, this test would have failed with
    // - Expected: 22
    // - Actual  : 16
    CPPUNIT_ASSERT_EQUAL(22, pPdfPage->getObjectCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testReduceImage)
{
    // Load the Writer document.
    loadFromFile(u"reduce-image.fodt");

    // Save as PDF.
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance(u"com.sun.star.document.PDFFilter"_ustr), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);

    SvFileStream aOutputStream(maTempFile.GetURL(), StreamMode::WRITE);
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aOutputStream));

    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "ReduceImageResolution", uno::Any(false) } }));

    // This is intentionally in an "unlucky" order, output stream comes before filter data.
    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(u"writer_pdf_Export"_ustr) },
        { "OutputStream", uno::Any(xOutputStream) },
        { "FilterData", uno::Any(aFilterData) },
    }));
    xFilter->filter(aDescriptor);
    aOutputStream.Close();

    // Parse the PDF: get the image.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Image, pPageObject->getType());

    // Make sure we don't scale down a bitmap.
    std::unique_ptr<vcl::pdf::PDFiumBitmap> pBitmap = pPageObject->getImageBitmap();
    CPPUNIT_ASSERT(pBitmap);
    int nWidth = pBitmap->getWidth();
    int nHeight = pBitmap->getHeight();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 160
    // - Actual  : 6
    // i.e. the image was scaled down even with ReduceImageResolution=false.
    CPPUNIT_ASSERT_EQUAL(160, nWidth);
    CPPUNIT_ASSERT_EQUAL(160, nHeight);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testLinkWrongPage)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor[u"FilterName"_ustr] <<= u"impress_pdf_Export"_ustr;
    saveAsPDF(u"link-wrong-page.odp");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has 2 pages.
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());

    // First page should have 1 link (2nd slide, 1st was hidden).
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Without the accompanying fix in place, this test would have failed, as the link of the first
    // page went to the second page due to the hidden first slide.
    CPPUNIT_ASSERT(pPdfPage->hasLinks());

    // Second page should have no links (3rd slide).
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage2 = pPdfDocument->openPage(/*nIndex=*/1);
    CPPUNIT_ASSERT(pPdfPage2);
    CPPUNIT_ASSERT(!pPdfPage2->hasLinks());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testLinkWrongPagePartial)
{
    // Given a Draw document with 3 pages, a link on the 2nd page:
    // When exporting that the 2nd and 3rd page to pdf:
    uno::Sequence<beans::PropertyValue> aFilterData = {
        comphelper::makePropertyValue(u"PageRange"_ustr, u"2-3"_ustr),
    };
    aMediaDescriptor[u"FilterName"_ustr] <<= u"draw_pdf_Export"_ustr;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"link-wrong-page-partial.odg");

    // Then make sure the we have a link on the 1st page, but not on the 2nd one:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    // Without the accompanying fix in place, this test would have failed, as the link was on the
    // 2nd page instead.
    CPPUNIT_ASSERT(pPdfPage->hasLinks());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage2 = pPdfDocument->openPage(/*nIndex=*/1);
    CPPUNIT_ASSERT(pPdfPage2);
    CPPUNIT_ASSERT(!pPdfPage2->hasLinks());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testPageRange)
{
    // Given a document with 3 pages:
    // When exporting that document to PDF, skipping the first page:
    aMediaDescriptor[u"FilterName"_ustr] <<= u"draw_pdf_Export"_ustr;
    aMediaDescriptor[u"FilterOptions"_ustr]
        <<= u"{\"PageRange\":{\"type\":\"string\",\"value\":\"2-\"}}"_ustr;
    saveAsPDF(u"link-wrong-page-partial.odg");

    // Then make sure the resulting PDF has 2 pages:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. FilterOptions was ignored.
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testLargePage)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor[u"FilterName"_ustr] <<= u"draw_pdf_Export"_ustr;
    saveAsPDF(u"6m-wide.odg");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has 1 page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    // Check the value (not the unit) of the page size.
    basegfx::B2DSize aSize = pPdfDocument->getPageSize(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 8503.94
    // - Actual  : 17007.875
    // i.e. the value for 600 cm was larger than the 14 400 limit set in the spec.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8503.94, aSize.getWidth(), 0.01);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testPdfImageResourceInlineXObjectRef)
{
    // Create an empty document.
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    // Insert the PDF image.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphicObject(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    OUString aURL = createFileURL(u"pdf-image-resource-inline-xobject-ref.pdf");
    xGraphicObject->setPropertyValue(u"GraphicURL"_ustr, uno::Any(aURL));
    uno::Reference<drawing::XShape> xShape(xGraphicObject, uno::UNO_QUERY);
    xShape->setSize(awt::Size(1000, 1000));
    uno::Reference<text::XTextContent> xTextContent(xGraphicObject, uno::UNO_QUERY);
    xText->insertTextContent(xCursor->getStart(), xTextContent, /*bAbsorb=*/false);

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Make sure that the page -> form -> form has a child image.
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pPageObject->getType());
    // 2: white background and the actual object.
    CPPUNIT_ASSERT_EQUAL(2, pPageObject->getFormObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pFormObject = pPageObject->getFormObject(1);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pFormObject->getType());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the sub-form was missing its image.
    CPPUNIT_ASSERT_EQUAL(1, pFormObject->getFormObjectCount());

    // Check if the inner form object (original page object in the pdf image) has the correct
    // rotation.
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pInnerFormObject = pFormObject->getFormObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pInnerFormObject->getType());
    CPPUNIT_ASSERT_EQUAL(1, pInnerFormObject->getFormObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pImage = pInnerFormObject->getFormObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Image, pImage->getType());
    basegfx::B2DHomMatrix aMat = pInnerFormObject->getMatrix();
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate = 0;
    double fShearX = 0;
    aMat.decompose(aScale, aTranslate, fRotate, fShearX);
    int nRotateDeg = basegfx::rad2deg(fRotate);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -90
    // - Actual  : 0
    // i.e. rotation was lost on pdf export.
    CPPUNIT_ASSERT_EQUAL(-90, nRotateDeg);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testDefaultVersion)
{
    // Create an empty document.
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    int nFileVersion = pPdfDocument->getFileVersion();
    CPPUNIT_ASSERT_EQUAL(17, nFileVersion);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testVersion15)
{
    // Create an empty document.
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "SelectPdfVersion", uno::Any(static_cast<sal_Int32>(15)) } }));
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    int nFileVersion = pPdfDocument->getFileVersion();
    CPPUNIT_ASSERT_EQUAL(15, nFileVersion);
}

// Check round-trip of importing and exporting the PDF with PDFium filter,
// which imports the PDF document as multiple PDFs as graphic object.
// Each page in the document has one PDF graphic object which content is
// the corresponding page in the PDF. When such a document is exported,
// the PDF graphic gets embedded into the exported PDF document (as a
// Form XObject).
CPPUNIT_TEST_FIXTURE(PdfExportTest2, testMultiPagePDF)
{
// setenv only works on unix based systems
#ifndef _WIN32
    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        setenv("LO_IMPORT_USE_PDFIUM", "1", false);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            unsetenv("LO_IMPORT_USE_PDFIUM");
    });

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    load(u"SimpleMultiPagePDF.pdf", aDocument);

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(3), aPages.size());

    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);

    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);

    CPPUNIT_ASSERT_EQUAL(size_t(3),
                         pXObjects->GetItems().size()); // 3 PDFs as Form XObjects

    std::vector<OString> rIDs;
    for (auto const& rPair : pXObjects->GetItems())
    {
        rIDs.push_back(rPair.first);
    }

    // Let's check the embedded PDF pages - just make sure the size differs,
    // which should indicate we don't have 3 times the same page.

    { // embedded PDF page 1
        vcl::filter::PDFObjectElement* pXObject1 = pXObjects->LookupObject(rIDs[0]);
        CPPUNIT_ASSERT(pXObject1);
        CPPUNIT_ASSERT_EQUAL("Im21"_ostr, rIDs[0]);

        auto pSubtype1
            = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject1->Lookup("Subtype"_ostr));
        CPPUNIT_ASSERT(pSubtype1);
        CPPUNIT_ASSERT_EQUAL("Form"_ostr, pSubtype1->GetValue());

        auto pXObjectResources
            = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject1->Lookup("Resources"_ostr));
        CPPUNIT_ASSERT(pXObjectResources);
        auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
            pXObjectResources->LookupElement("XObject"_ostr));
        CPPUNIT_ASSERT(pXObjectForms);
        vcl::filter::PDFObjectElement* pForm
            = pXObjectForms->LookupObject(pXObjectForms->GetItems().begin()->first);
        CPPUNIT_ASSERT(pForm);

        vcl::filter::PDFStreamElement* pStream = pForm->GetStream();
        CPPUNIT_ASSERT(pStream);
        SvMemoryStream& rObjectStream = pStream->GetMemory();
        rObjectStream.Seek(STREAM_SEEK_TO_BEGIN);

        // Just check that the size of the page stream is what is expected.
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(230), rObjectStream.remainingSize());
    }

    { // embedded PDF page 2
        vcl::filter::PDFObjectElement* pXObject2 = pXObjects->LookupObject(rIDs[1]);
        CPPUNIT_ASSERT(pXObject2);
        CPPUNIT_ASSERT_EQUAL("Im27"_ostr, rIDs[1]);

        auto pSubtype2
            = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject2->Lookup("Subtype"_ostr));
        CPPUNIT_ASSERT(pSubtype2);
        CPPUNIT_ASSERT_EQUAL("Form"_ostr, pSubtype2->GetValue());

        auto pXObjectResources
            = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject2->Lookup("Resources"_ostr));
        CPPUNIT_ASSERT(pXObjectResources);
        auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
            pXObjectResources->LookupElement("XObject"_ostr));
        CPPUNIT_ASSERT(pXObjectForms);
        vcl::filter::PDFObjectElement* pForm
            = pXObjectForms->LookupObject(pXObjectForms->GetItems().begin()->first);
        CPPUNIT_ASSERT(pForm);

        vcl::filter::PDFStreamElement* pStream = pForm->GetStream();
        CPPUNIT_ASSERT(pStream);
        SvMemoryStream& rObjectStream = pStream->GetMemory();
        rObjectStream.Seek(STREAM_SEEK_TO_BEGIN);

        // Just check that the size of the page stream is what is expected
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(309), rObjectStream.remainingSize());
    }

    { // embedded PDF page 3
        vcl::filter::PDFObjectElement* pXObject3 = pXObjects->LookupObject(rIDs[2]);
        CPPUNIT_ASSERT(pXObject3);
        CPPUNIT_ASSERT_EQUAL("Im5"_ostr, rIDs[2]);

        auto pSubtype3
            = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject3->Lookup("Subtype"_ostr));
        CPPUNIT_ASSERT(pSubtype3);
        CPPUNIT_ASSERT_EQUAL("Form"_ostr, pSubtype3->GetValue());

        auto pXObjectResources
            = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject3->Lookup("Resources"_ostr));
        CPPUNIT_ASSERT(pXObjectResources);
        auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
            pXObjectResources->LookupElement("XObject"_ostr));
        CPPUNIT_ASSERT(pXObjectForms);
        vcl::filter::PDFObjectElement* pForm
            = pXObjectForms->LookupObject(pXObjectForms->GetItems().begin()->first);
        CPPUNIT_ASSERT(pForm);

        vcl::filter::PDFStreamElement* pStream = pForm->GetStream();
        CPPUNIT_ASSERT(pStream);
        SvMemoryStream& rObjectStream = pStream->GetMemory();
        rObjectStream.Seek(STREAM_SEEK_TO_BEGIN);

        // Just check that the size of the page stream is what is expected
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(193), rObjectStream.remainingSize());
    }
#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testFormFontName)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    saveAsPDF(u"form-font-name.odt");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // The page has one annotation.
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnot = pPdfPage->getAnnotation(0);

    // Examine the default appearance.
    CPPUNIT_ASSERT(pAnnot->hasKey("DA"_ostr));
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFObjectType::String, pAnnot->getValueType("DA"_ostr));
    OUString aDA = pAnnot->getString("DA"_ostr);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0 0 0 rg /TiRo 12 Tf
    // - Actual  : 0 0 0 rg /F2 12 Tf
    // i.e. Liberation Serif was exposed as a form font as-is, without picking the closest built-in
    // font.
    CPPUNIT_ASSERT_EQUAL(u"0 0 0 rg /TiRo 12 Tf"_ustr, aDA);
}

// Check we don't have duplicated objects when we reexport the PDF multiple
// times or the size will exponentially increase over time.
CPPUNIT_TEST_FIXTURE(PdfExportTest2, testReexportPDF)
{
// setenv only works on unix based systems
#ifndef _WIN32
    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        setenv("LO_IMPORT_USE_PDFIUM", "1", false);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            unsetenv("LO_IMPORT_USE_PDFIUM");
    });

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    load(u"PDFWithImages.pdf", aDocument);

    // Assert that the XObject in the page resources dictionary is a reference XObject.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();

    // The document has 2 pages.
    CPPUNIT_ASSERT_EQUAL(size_t(2), aPages.size());

    // PAGE 1
    {
        vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
        CPPUNIT_ASSERT(pResources);

        auto pXObjects
            = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
        CPPUNIT_ASSERT(pXObjects);

        std::vector<OString> rIDs;
        for (auto const& rPair : pXObjects->GetItems())
            rIDs.push_back(rPair.first);

        CPPUNIT_ASSERT_EQUAL(size_t(2), rIDs.size());

        std::vector<int> aBitmapRefs1;
        std::vector<int> aBitmapRefs2;

        {
            // FORM object 1
            OString aID = rIDs[0];
            CPPUNIT_ASSERT_EQUAL("Im14"_ostr, aID);
            vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(aID);
            CPPUNIT_ASSERT(pXObject);

            auto pSubtype
                = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject->Lookup("Subtype"_ostr));
            CPPUNIT_ASSERT(pSubtype);
            CPPUNIT_ASSERT_EQUAL("Form"_ostr, pSubtype->GetValue());

            auto pInnerResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pXObject->Lookup("Resources"_ostr));
            CPPUNIT_ASSERT(pInnerResources);
            auto pInnerXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerResources->LookupElement("XObject"_ostr));
            CPPUNIT_ASSERT(pInnerXObjects);
            CPPUNIT_ASSERT_EQUAL(size_t(1), pInnerXObjects->GetItems().size());
            OString aInnerObjectID = pInnerXObjects->GetItems().begin()->first;
            CPPUNIT_ASSERT_EQUAL("Im15"_ostr, aInnerObjectID);

            vcl::filter::PDFObjectElement* pInnerXObject
                = pInnerXObjects->LookupObject(aInnerObjectID);
            CPPUNIT_ASSERT(pInnerXObject);

            auto pInnerSubtype
                = dynamic_cast<vcl::filter::PDFNameElement*>(pInnerXObject->Lookup("Subtype"_ostr));
            CPPUNIT_ASSERT(pInnerSubtype);
            CPPUNIT_ASSERT_EQUAL("Form"_ostr, pInnerSubtype->GetValue());

            auto pInnerInnerResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerXObject->Lookup("Resources"_ostr));
            CPPUNIT_ASSERT(pInnerInnerResources);
            auto pInnerInnerXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerInnerResources->LookupElement("XObject"_ostr));
            CPPUNIT_ASSERT(pInnerInnerXObjects);
            CPPUNIT_ASSERT_EQUAL(size_t(2), pInnerInnerXObjects->GetItems().size());

            std::vector<OString> aBitmapIDs1;
            for (auto const& rPair : pInnerInnerXObjects->GetItems())
                aBitmapIDs1.push_back(rPair.first);

            {
                CPPUNIT_ASSERT_EQUAL("Im11"_ostr, aBitmapIDs1[0]);
                auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pInnerInnerXObjects->LookupElement(aBitmapIDs1[0]));
                CPPUNIT_ASSERT(pRef);
                aBitmapRefs1.push_back(pRef->GetObjectValue());
                CPPUNIT_ASSERT_EQUAL(0, pRef->GetGenerationValue());

                vcl::filter::PDFObjectElement* pBitmap
                    = pInnerInnerXObjects->LookupObject(aBitmapIDs1[0]);
                CPPUNIT_ASSERT(pBitmap);
                auto pBitmapSubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pBitmap->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT(pBitmapSubtype);
                CPPUNIT_ASSERT_EQUAL("Image"_ostr, pBitmapSubtype->GetValue());
            }
            {
                CPPUNIT_ASSERT_EQUAL("Im5"_ostr, aBitmapIDs1[1]);
                auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pInnerInnerXObjects->LookupElement(aBitmapIDs1[1]));
                CPPUNIT_ASSERT(pRef);
                aBitmapRefs1.push_back(pRef->GetObjectValue());
                CPPUNIT_ASSERT_EQUAL(0, pRef->GetGenerationValue());

                vcl::filter::PDFObjectElement* pBitmap
                    = pInnerInnerXObjects->LookupObject(aBitmapIDs1[1]);
                CPPUNIT_ASSERT(pBitmap);
                auto pBitmapSubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pBitmap->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT(pBitmapSubtype);
                CPPUNIT_ASSERT_EQUAL("Image"_ostr, pBitmapSubtype->GetValue());
            }
        }

        {
            // FORM object 2
            OString aID = rIDs[1];
            CPPUNIT_ASSERT_EQUAL("Im5"_ostr, aID);
            vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(aID);
            CPPUNIT_ASSERT(pXObject);

            auto pSubtype
                = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject->Lookup("Subtype"_ostr));
            CPPUNIT_ASSERT(pSubtype);
            CPPUNIT_ASSERT_EQUAL("Form"_ostr, pSubtype->GetValue());

            auto pInnerResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pXObject->Lookup("Resources"_ostr));
            CPPUNIT_ASSERT(pInnerResources);
            auto pInnerXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerResources->LookupElement("XObject"_ostr));
            CPPUNIT_ASSERT(pInnerXObjects);
            CPPUNIT_ASSERT_EQUAL(size_t(1), pInnerXObjects->GetItems().size());
            OString aInnerObjectID = pInnerXObjects->GetItems().begin()->first;
            CPPUNIT_ASSERT_EQUAL("Im6"_ostr, aInnerObjectID);

            vcl::filter::PDFObjectElement* pInnerXObject
                = pInnerXObjects->LookupObject(aInnerObjectID);
            CPPUNIT_ASSERT(pInnerXObject);

            auto pInnerSubtype
                = dynamic_cast<vcl::filter::PDFNameElement*>(pInnerXObject->Lookup("Subtype"_ostr));
            CPPUNIT_ASSERT(pInnerSubtype);
            CPPUNIT_ASSERT_EQUAL("Form"_ostr, pInnerSubtype->GetValue());

            auto pInnerInnerResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerXObject->Lookup("Resources"_ostr));
            CPPUNIT_ASSERT(pInnerInnerResources);
            auto pInnerInnerXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerInnerResources->LookupElement("XObject"_ostr));
            CPPUNIT_ASSERT(pInnerInnerXObjects);
            CPPUNIT_ASSERT_EQUAL(size_t(2), pInnerInnerXObjects->GetItems().size());

            std::vector<OString> aBitmapIDs2;
            for (auto const& rPair : pInnerInnerXObjects->GetItems())
                aBitmapIDs2.push_back(rPair.first);

            {
                CPPUNIT_ASSERT_EQUAL("Im11"_ostr, aBitmapIDs2[0]);
                auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pInnerInnerXObjects->LookupElement(aBitmapIDs2[0]));
                CPPUNIT_ASSERT(pRef);
                aBitmapRefs2.push_back(pRef->GetObjectValue());
                CPPUNIT_ASSERT_EQUAL(0, pRef->GetGenerationValue());

                vcl::filter::PDFObjectElement* pBitmap
                    = pInnerInnerXObjects->LookupObject(aBitmapIDs2[0]);
                CPPUNIT_ASSERT(pBitmap);
                auto pBitmapSubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pBitmap->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT(pBitmapSubtype);
                CPPUNIT_ASSERT_EQUAL("Image"_ostr, pBitmapSubtype->GetValue());
            }
            {
                CPPUNIT_ASSERT_EQUAL("Im5"_ostr, aBitmapIDs2[1]);
                auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pInnerInnerXObjects->LookupElement(aBitmapIDs2[1]));
                CPPUNIT_ASSERT(pRef);
                aBitmapRefs2.push_back(pRef->GetObjectValue());
                CPPUNIT_ASSERT_EQUAL(0, pRef->GetGenerationValue());

                vcl::filter::PDFObjectElement* pBitmap
                    = pInnerInnerXObjects->LookupObject(aBitmapIDs2[1]);
                CPPUNIT_ASSERT(pBitmap);
                auto pBitmapSubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pBitmap->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT(pBitmapSubtype);
                CPPUNIT_ASSERT_EQUAL("Image"_ostr, pBitmapSubtype->GetValue());
            }
        }
        // Ref should point to the same bitmap
        CPPUNIT_ASSERT_EQUAL(aBitmapRefs1[0], aBitmapRefs2[0]);
        CPPUNIT_ASSERT_EQUAL(aBitmapRefs1[1], aBitmapRefs2[1]);
    }

#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf160117)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf160117.ods", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    int nCount = 0;
    bool bFound1 = false;
    bool bFound2 = false;
    bool bFound3 = false;
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "FontDescriptor")
        {
            auto pFontName
                = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("FontName"_ostr));
            CPPUNIT_ASSERT(pFontName);
            if ("CAAAAA+LiberationSans-Bold"_ostr == pFontName->GetValue())
                bFound1 = true;
            else if ("DAAAAA+LiberationSans-Italic"_ostr == pFontName->GetValue())
                bFound2 = true;
            else if ("BAAAAA+LiberationSans"_ostr == pFontName->GetValue())
                bFound3 = true;
            ++nCount;
        }
    }
    // Without the fix in place, this test would have failed with
    // - Expected: 3
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(3, nCount);
    CPPUNIT_ASSERT(bFound1);
    CPPUNIT_ASSERT(bFound2);
    CPPUNIT_ASSERT(bFound3);
}

// Check we correctly copy more complex resources (Fonts describing
// glyphs in recursive arrays) to the target PDF
CPPUNIT_TEST_FIXTURE(PdfExportTest2, testReexportDocumentWithComplexResources)
{
// setenv only works on unix based systems
#ifndef _WIN32
    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        setenv("LO_IMPORT_USE_PDFIUM", "1", false);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            unsetenv("LO_IMPORT_USE_PDFIUM");
    });

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    load(u"ComplexContentDictionary.pdf", aDocument);

    // Assert that the XObject in the page resources dictionary is a reference XObject.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    // Go directly to the Font object (24 0) (number could change if we change how PDF export works)
    auto pFont = aDocument.LookupObject(24);
    CPPUNIT_ASSERT(pFont);

    // Check it is the Font object (Type = Font)
    auto pName = dynamic_cast<vcl::filter::PDFNameElement*>(
        pFont->GetDictionary()->LookupElement("Type"_ostr));
    CPPUNIT_ASSERT(pName);
    CPPUNIT_ASSERT_EQUAL("Font"_ostr, pName->GetValue());

    // Check BaseFont is what we expect
    auto pBaseFont = dynamic_cast<vcl::filter::PDFNameElement*>(
        pFont->GetDictionary()->LookupElement("BaseFont"_ostr));
    CPPUNIT_ASSERT(pBaseFont);
    CPPUNIT_ASSERT_EQUAL("HOTOMR+Calibri,Italic"_ostr, pBaseFont->GetValue());

    // Check and get the W array
    auto pWArray = dynamic_cast<vcl::filter::PDFArrayElement*>(
        pFont->GetDictionary()->LookupElement("W"_ostr));
    CPPUNIT_ASSERT(pWArray);
    CPPUNIT_ASSERT_EQUAL(size_t(26), pWArray->GetElements().size());

    // Check the content of W array
    // ObjectCopier didn't copy this array correctly and the document
    // had glyphs at the wrong places
    {
        // first 2 elements
        auto pNumberAtIndex0 = dynamic_cast<vcl::filter::PDFNumberElement*>(pWArray->GetElement(0));
        CPPUNIT_ASSERT(pNumberAtIndex0);
        CPPUNIT_ASSERT_EQUAL(3.0, pNumberAtIndex0->GetValue());

        auto pArrayAtIndex1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pWArray->GetElement(1));
        CPPUNIT_ASSERT(pArrayAtIndex1);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pArrayAtIndex1->GetElements().size());

        {
            auto pNumber
                = dynamic_cast<vcl::filter::PDFNumberElement*>(pArrayAtIndex1->GetElement(0));
            CPPUNIT_ASSERT(pNumber);
            CPPUNIT_ASSERT_EQUAL(226.0, pNumber->GetValue());
        }

        // last 2 elements
        auto pNumberAtIndex24
            = dynamic_cast<vcl::filter::PDFNumberElement*>(pWArray->GetElement(24));
        CPPUNIT_ASSERT(pNumberAtIndex24);
        CPPUNIT_ASSERT_EQUAL(894.0, pNumberAtIndex24->GetValue());

        auto pArrayAtIndex25 = dynamic_cast<vcl::filter::PDFArrayElement*>(pWArray->GetElement(25));
        CPPUNIT_ASSERT(pArrayAtIndex25);
        CPPUNIT_ASSERT_EQUAL(size_t(2), pArrayAtIndex25->GetElements().size());

        {
            auto pNumber1
                = dynamic_cast<vcl::filter::PDFNumberElement*>(pArrayAtIndex25->GetElement(0));
            CPPUNIT_ASSERT(pNumber1);
            CPPUNIT_ASSERT_EQUAL(303.0, pNumber1->GetValue());

            auto pNumber2
                = dynamic_cast<vcl::filter::PDFNumberElement*>(pArrayAtIndex25->GetElement(1));
            CPPUNIT_ASSERT(pNumber2);
            CPPUNIT_ASSERT_EQUAL(303.0, pNumber2->GetValue());
        }
    }
#endif
}

// Tests that at export the PDF has the PDF/UA metadata properly set
// when we enable PDF/UA support.
CPPUNIT_TEST_FIXTURE(PdfExportTest2, testPdfUaMetadata)
{
    // Import a basic document (document doesn't really matter)
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"BrownFoxLazyDog.odt");

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    auto* pCatalog = aDocument.GetCatalog();
    CPPUNIT_ASSERT(pCatalog);
    auto* pCatalogDictionary = pCatalog->GetDictionary();
    CPPUNIT_ASSERT(pCatalogDictionary);
    auto* pMetadataObject = pCatalogDictionary->LookupObject("Metadata"_ostr);
    CPPUNIT_ASSERT(pMetadataObject);
    auto* pMetadataDictionary = pMetadataObject->GetDictionary();
    auto* pType = dynamic_cast<vcl::filter::PDFNameElement*>(
        pMetadataDictionary->LookupElement("Type"_ostr));
    CPPUNIT_ASSERT(pType);
    CPPUNIT_ASSERT_EQUAL("Metadata"_ostr, pType->GetValue());

    auto* pStreamObject = pMetadataObject->GetStream();
    CPPUNIT_ASSERT(pStreamObject);
    auto& rStream = pStreamObject->GetMemory();
    rStream.Seek(0);

    // Search for the PDF/UA marker in the metadata

    tools::XmlWalker aWalker;
    CPPUNIT_ASSERT(aWalker.open(&rStream));
    CPPUNIT_ASSERT_EQUAL(std::string_view("xmpmeta"), aWalker.name());

    bool bPdfUaMarkerFound = false;
    OString aPdfUaPart;

    aWalker.children();
    while (aWalker.isValid())
    {
        if (aWalker.name() == "RDF"
            && aWalker.namespaceHref() == "http://www.w3.org/1999/02/22-rdf-syntax-ns#")
        {
            aWalker.children();
            while (aWalker.isValid())
            {
                if (aWalker.name() == "Description"
                    && aWalker.namespaceHref() == "http://www.w3.org/1999/02/22-rdf-syntax-ns#")
                {
                    aWalker.children();
                    while (aWalker.isValid())
                    {
                        if (aWalker.name() == "part"
                            && aWalker.namespaceHref() == "http://www.aiim.org/pdfua/ns/id/")
                        {
                            aPdfUaPart = aWalker.content();
                            bPdfUaMarkerFound = true;
                        }
                        aWalker.next();
                    }
                    aWalker.parent();
                }
                aWalker.next();
            }
            aWalker.parent();
        }
        aWalker.next();
    }
    aWalker.parent();

    CPPUNIT_ASSERT(bPdfUaMarkerFound);
    CPPUNIT_ASSERT_EQUAL("1"_ostr, aPdfUaPart);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf139736)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) },
                                           { "SelectPdfVersion", uno::Any(sal_Int32(17)) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"tdf139736-1.odt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();

    enum
    {
        Default,
        Artifact,
        ArtifactProps1,
        ArtifactProps2,
        Tagged
    } state
        = Default;

    int nLine(0);
    int nTagged(0);
    int nArtifacts(0);
    while (true)
    {
        ++nLine;
        auto const pLine = ::std::find(pStart, pEnd, '\n');
        if (pLine == pEnd)
        {
            break;
        }
        std::string_view const line(pStart, pLine - pStart);
        pStart = pLine + 1;
        if (!line.empty() && line[0] != '%')
        {
            ::std::cerr << nLine << ": " << line << "\n";
            if (line == "/Artifact BMC")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Artifact;
                ++nArtifacts;
            }
            else if (o3tl::starts_with(line, "/Artifact <<"))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                // check header/footer properties
                CPPUNIT_ASSERT_EQUAL(std::string_view("/Type/Pagination"), line.substr(12));
                state = ArtifactProps1;
                ++nArtifacts;
            }
            else if (state == ArtifactProps1)
            {
                CPPUNIT_ASSERT_EQUAL(std::string_view("/Subtype/Header"), line);
                state = ArtifactProps2;
            }
            else if (state == ArtifactProps2 && line == ">> BDC")
            {
                state = Artifact;
            }
            else if (line == "/Standard<</MCID 0>>BDC")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Tagged;
                ++nTagged;
            }
            else if (line == "EMC")
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected end", state != Default);
                state = Default;
            }
            else if (nLine > 1) // first line is expected "0.1 w"
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected content outside MCS", state != Default);
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unclosed MCS", Default, state);
    CPPUNIT_ASSERT_EQUAL(1, nTagged); // text in body
    // 1 image and 1 frame and 1 header text; arbitrary number of aux stuff like borders
    CPPUNIT_ASSERT_GREATEREQUAL(3, nArtifacts);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf152231)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) },
                                           { "ExportNotesInMargin", uno::Any(true) },
                                           { "SelectPdfVersion", uno::Any(sal_Int32(17)) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"tdf152231.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();

    enum
    {
        Default,
        Artifact,
        Tagged
    } state
        = Default;

    int nLine(0);
    int nTagged(0);
    int nArtifacts(0);
    while (true)
    {
        ++nLine;
        auto const pLine = ::std::find(pStart, pEnd, '\n');
        if (pLine == pEnd)
        {
            break;
        }
        std::string_view const line(pStart, pLine - pStart);
        pStart = pLine + 1;
        if (!line.empty() && line[0] != '%')
        {
            ::std::cerr << nLine << ": " << line << "\n";
            if (line == "/Artifact BMC")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Artifact;
                ++nArtifacts;
            }
            else if (o3tl::starts_with(line, "/Standard<</MCID "))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Tagged;
                ++nTagged;
            }
            else if (line == "EMC")
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected end", state != Default);
                state = Default;
            }
            else if (nLine > 1) // first line is expected "0.1 w"
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected content outside MCS", state != Default);
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unclosed MCS", Default, state);
    CPPUNIT_ASSERT_GREATEREQUAL(12, nTagged); // text in body
    // 1 annotation
    CPPUNIT_ASSERT_GREATEREQUAL(1, nArtifacts);

    int nPara(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
            if (pS && pS->GetValue() == "Standard")
            {
                ++nPara;
                auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids);
                // one problem was that some StructElem were missing kids
                CPPUNIT_ASSERT(!pKids->GetElements().empty());
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(12, nPara);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf152235)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "PDFUACompliance", uno::Any(true) },
          { "Watermark", uno::Any(u"kendy"_ustr) },
          // need to set a font to avoid assertions about missing "Helvetica"
          { "WatermarkFontName", uno::Any(u"Liberation Sans"_ustr) },
          { "SelectPdfVersion", uno::Any(sal_Int32(17)) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();

    enum
    {
        Default,
        Artifact,
        Tagged
    } state
        = Default;

    int nLine(0);
    int nTagged(0);
    int nArtifacts(0);
    while (true)
    {
        ++nLine;
        auto const pLine = ::std::find(pStart, pEnd, '\n');
        if (pLine == pEnd)
        {
            break;
        }
        std::string_view const line(pStart, pLine - pStart);
        pStart = pLine + 1;
        if (!line.empty() && line[0] != '%')
        {
            ::std::cerr << nLine << ": " << line << "\n";
            if (o3tl::starts_with(line, "/Artifact "))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Artifact;
                ++nArtifacts;
            }
            else if (o3tl::starts_with(line, "/Standard<</MCID "))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Tagged;
                ++nTagged;
            }
            else if (line == "EMC")
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected end", state != Default);
                state = Default;
            }
            else if (nLine > 1) // first line is expected "0.1 w"
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected content outside MCS", state != Default);
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unclosed MCS", Default, state);
    CPPUNIT_ASSERT_GREATEREQUAL(0, nTagged); // text in body
    CPPUNIT_ASSERT_GREATEREQUAL(2, nArtifacts); // 1 watermark + 1 other thing
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf149140)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"TableTH_test_LibreOfficeWriter7.3.3_HeaderRow-HeadersInTopRow.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    int nTH(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
            if (pS && pS->GetValue() == "TH")
            {
                int nTable(0);
                auto pAttrs
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject->Lookup("A"_ostr));
                CPPUNIT_ASSERT(pAttrs != nullptr);
                for (const auto& rAttrRef : pAttrs->GetElements())
                {
                    auto pAttrDict = dynamic_cast<vcl::filter::PDFDictionaryElement*>(rAttrRef);
                    CPPUNIT_ASSERT(pAttrDict != nullptr);
                    auto pOwner = dynamic_cast<vcl::filter::PDFNameElement*>(
                        pAttrDict->LookupElement("O"_ostr));
                    CPPUNIT_ASSERT(pOwner != nullptr);
                    if (pOwner->GetValue() == "Table")
                    {
                        auto pScope = dynamic_cast<vcl::filter::PDFNameElement*>(
                            pAttrDict->LookupElement("Scope"_ostr));
                        CPPUNIT_ASSERT(pScope != nullptr);
                        CPPUNIT_ASSERT_EQUAL("Column"_ostr, pScope->GetValue());
                        ++nTable;
                    }
                }
                CPPUNIT_ASSERT_EQUAL(1, nTable);
                ++nTH;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(6, nTH);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testNestedSection)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"nestedsection.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // the assert needs 2 follows to reproduce => 3 pages
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(3), aPages.size());

    int nDoc(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"_ostr));
            if (pS1 && pS1->GetValue() == "Document")
            {
                auto pKids1
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids1);
                // assume there are no MCID ref at this level
                auto pKids1v = pKids1->GetElements();
                auto pRefKid10 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[0]);
                CPPUNIT_ASSERT(pRefKid10);
                auto pObject10 = pRefKid10->LookupObject();
                CPPUNIT_ASSERT(pObject10);
                auto pType10
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType10->GetValue());
                auto pS10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Sect"_ostr, pS10->GetValue());

                auto pKids10
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids10);
                // assume there are no MCID ref at this level
                auto pKids10v = pKids10->GetElements();

                auto pRefKid100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids10v[0]);
                CPPUNIT_ASSERT(pRefKid100);
                auto pObject100 = pRefKid100->LookupObject();
                CPPUNIT_ASSERT(pObject100);
                auto pType100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject100->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType100->GetValue());
                auto pS100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject100->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS100->GetValue());

                auto pRefKid101 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids10v[1]);
                CPPUNIT_ASSERT(pRefKid101);
                auto pObject101 = pRefKid101->LookupObject();
                CPPUNIT_ASSERT(pObject101);
                auto pType101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject101->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType101->GetValue());
                auto pS101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject101->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS101->GetValue());

                auto pRefKid102 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids10v[2]);
                CPPUNIT_ASSERT(pRefKid102);
                auto pObject102 = pRefKid102->LookupObject();
                CPPUNIT_ASSERT(pObject102);
                auto pType102
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType102->GetValue());
                auto pS102
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Sect"_ostr, pS102->GetValue());

                auto pKids102
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject102->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids102);
                // assume there are no MCID ref at this level
                auto pKids102v = pKids102->GetElements();

                auto pRefKid1020 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids102v[0]);
                CPPUNIT_ASSERT(pRefKid1020);
                auto pObject1020 = pRefKid1020->LookupObject();
                CPPUNIT_ASSERT(pObject1020);
                auto pType1020
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1020->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1020->GetValue());
                auto pS1020
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1020->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS1020->GetValue());

                CPPUNIT_ASSERT_EQUAL(size_t(1), pKids102v.size());

                auto pRefKid103 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids10v[3]);
                CPPUNIT_ASSERT(pRefKid103);
                auto pObject103 = pRefKid103->LookupObject();
                CPPUNIT_ASSERT(pObject103);
                auto pType103
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject103->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType103->GetValue());
                auto pS103
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject103->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS103->GetValue());

                auto pRefKid104 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids10v[4]);
                CPPUNIT_ASSERT(pRefKid104);
                auto pObject104 = pRefKid104->LookupObject();
                CPPUNIT_ASSERT(pObject104);
                auto pType104
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject104->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType104->GetValue());
                auto pS104
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject104->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS104->GetValue());

                CPPUNIT_ASSERT_EQUAL(size_t(5), pKids10v.size());

                auto pRefKid11 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[1]);
                CPPUNIT_ASSERT(pRefKid11);
                auto pObject11 = pRefKid11->LookupObject();
                CPPUNIT_ASSERT(pObject11);
                auto pType11
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType11->GetValue());
                auto pS11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS11->GetValue());

                CPPUNIT_ASSERT_EQUAL(size_t(2), pKids1v.size());
                ++nDoc;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nDoc);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf157817)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"SimpleTOC.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aPages.size());

    vcl::filter::PDFObjectElement* pTOC(nullptr);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"_ostr));
            if (pS1 && pS1->GetValue() == "TOC")
            {
                pTOC = pObject1;
            }
        }
    }
    CPPUNIT_ASSERT(pTOC);

    auto pKidsT = dynamic_cast<vcl::filter::PDFArrayElement*>(pTOC->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsT);
    // assume there are no MCID ref at this level
    auto pKidsTv = pKidsT->GetElements();
    auto pRefKidT0 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsTv[0]);
    CPPUNIT_ASSERT(pRefKidT0);
    auto pObjectT0 = pRefKidT0->LookupObject();
    CPPUNIT_ASSERT(pObjectT0);
    auto pTypeT0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT0->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeT0->GetValue());
    auto pST0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT0->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Caption"_ostr, pST0->GetValue());

    auto pKidsT0 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT0->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsT0);
    auto pKidsT0v = pKidsT0->GetElements();
    auto pRefKidT00 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT0v[0]);
    CPPUNIT_ASSERT(pRefKidT00);
    auto pObjectT00 = pRefKidT00->LookupObject();
    CPPUNIT_ASSERT(pObjectT00);
    auto pTypeT00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT00->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeT00->GetValue());
    auto pST00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT00->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Contents#20Heading"_ostr, pST00->GetValue());

    auto pRefKidT1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsTv[1]);
    CPPUNIT_ASSERT(pRefKidT1);
    auto pObjectT1 = pRefKidT1->LookupObject();
    CPPUNIT_ASSERT(pObjectT1);
    auto pTypeT1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT1->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeT1->GetValue());
    auto pST1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT1->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("TOCI"_ostr, pST1->GetValue());

    auto pKidsT1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT1->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsT1);
    auto pKidsT1v = pKidsT1->GetElements();

    auto pRefKidT10 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT1v[0]);
    CPPUNIT_ASSERT(pRefKidT10);
    auto pObjectT10 = pRefKidT10->LookupObject();
    CPPUNIT_ASSERT(pObjectT10);
    auto pTypeT10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT10->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeT10->GetValue());
    auto pST10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT10->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Contents#201"_ostr, pST10->GetValue());

    auto pKidsT10 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT10->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsT10);
    auto pKidsT10v = pKidsT10->GetElements();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pKidsT10v.size());

    // there is one and only one Link
    auto pRefKidT100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT10v[0]);
    CPPUNIT_ASSERT(pRefKidT100);
    auto pObjectT100 = pRefKidT100->LookupObject();
    CPPUNIT_ASSERT(pObjectT100);
    auto pTypeT100 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT100->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeT100->GetValue());
    auto pST100 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT100->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pST100->GetValue());

    auto pRefKidT2 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsTv[1]);
    CPPUNIT_ASSERT(pRefKidT2);
    auto pObjectT2 = pRefKidT2->LookupObject();
    CPPUNIT_ASSERT(pObjectT2);
    auto pTypeT2 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT2->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeT2->GetValue());
    auto pST2 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT2->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("TOCI"_ostr, pST2->GetValue());

    auto pKidsT2 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT2->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsT2);
    auto pKidsT2v = pKidsT2->GetElements();

    auto pRefKidT20 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT2v[0]);
    CPPUNIT_ASSERT(pRefKidT20);
    auto pObjectT20 = pRefKidT20->LookupObject();
    CPPUNIT_ASSERT(pObjectT20);
    auto pTypeT20 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT20->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeT20->GetValue());
    auto pST20 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT20->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Contents#201"_ostr, pST20->GetValue());

    auto pKidsT20 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT20->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsT20);
    auto pKidsT20v = pKidsT20->GetElements();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pKidsT20v.size());

    // there is one and only one Link
    auto pRefKidT200 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT20v[0]);
    CPPUNIT_ASSERT(pRefKidT200);
    auto pObjectT200 = pRefKidT200->LookupObject();
    CPPUNIT_ASSERT(pObjectT200);
    auto pTypeT200 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT200->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeT200->GetValue());
    auto pST200 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT200->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pST200->GetValue());

    auto pRefKidT3 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsTv[1]);
    CPPUNIT_ASSERT(pRefKidT3);
    auto pObjectT3 = pRefKidT3->LookupObject();
    CPPUNIT_ASSERT(pObjectT3);
    auto pTypeT3 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT3->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeT3->GetValue());
    auto pST3 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT3->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("TOCI"_ostr, pST3->GetValue());

    auto pKidsT3 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT3->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsT3);
    auto pKidsT3v = pKidsT3->GetElements();

    auto pRefKidT30 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT3v[0]);
    CPPUNIT_ASSERT(pRefKidT30);
    auto pObjectT30 = pRefKidT30->LookupObject();
    CPPUNIT_ASSERT(pObjectT30);
    auto pTypeT30 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT30->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeT30->GetValue());
    auto pST30 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT30->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Contents#201"_ostr, pST30->GetValue());

    auto pKidsT30 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT30->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsT30);
    auto pKidsT30v = pKidsT30->GetElements();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pKidsT30v.size());

    // there is one and only one Link
    auto pRefKidT300 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT30v[0]);
    CPPUNIT_ASSERT(pRefKidT300);
    auto pObjectT300 = pRefKidT300->LookupObject();
    CPPUNIT_ASSERT(pObjectT300);
    auto pTypeT300 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT300->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeT300->GetValue());
    auto pST300 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT300->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pST300->GetValue());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf135638)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"image-shape.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    int nFigure(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
            if (pS && pS->GetValue() == "Figure")
            {
                auto pAttrDict
                    = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject->Lookup("A"_ostr));
                CPPUNIT_ASSERT(pAttrDict != nullptr);
                auto pOwner = dynamic_cast<vcl::filter::PDFNameElement*>(
                    pAttrDict->LookupElement("O"_ostr));
                CPPUNIT_ASSERT(pOwner != nullptr);
                CPPUNIT_ASSERT_EQUAL("Layout"_ostr, pOwner->GetValue());
                auto pBBox = dynamic_cast<vcl::filter::PDFArrayElement*>(
                    pAttrDict->LookupElement("BBox"_ostr));
                CPPUNIT_ASSERT(pBBox != nullptr);
                if (nFigure == 0)
                {
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(
                        139.5,
                        dynamic_cast<vcl::filter::PDFNumberElement*>(pBBox->GetElements()[0])
                            ->GetValue(),
                        0.01);
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(
                        480.3,
                        dynamic_cast<vcl::filter::PDFNumberElement*>(pBBox->GetElements()[1])
                            ->GetValue(),
                        0.01);
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(
                        472.5,
                        dynamic_cast<vcl::filter::PDFNumberElement*>(pBBox->GetElements()[2])
                            ->GetValue(),
                        0.01);
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(
                        735.3,
                        dynamic_cast<vcl::filter::PDFNumberElement*>(pBBox->GetElements()[3])
                            ->GetValue(),
                        0.01);
                }
                else
                {
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(
                        178.45,
                        dynamic_cast<vcl::filter::PDFNumberElement*>(pBBox->GetElements()[0])
                            ->GetValue(),
                        0.01);
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(
                        318.65,
                        dynamic_cast<vcl::filter::PDFNumberElement*>(pBBox->GetElements()[1])
                            ->GetValue(),
                        0.01);
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(
                        326.35,
                        dynamic_cast<vcl::filter::PDFNumberElement*>(pBBox->GetElements()[2])
                            ->GetValue(),
                        0.01);
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(
                        382.55,
                        dynamic_cast<vcl::filter::PDFNumberElement*>(pBBox->GetElements()[3])
                            ->GetValue(),
                        0.01);
                }
                ++nFigure;
            }
        }
    }
    // the first one is a Writer image, 2nd one SdrRectObj
    CPPUNIT_ASSERT_EQUAL(2, nFigure);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf157703)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"LO_Lbl_Lbody_bug_report.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    vcl::filter::PDFObjectElement* pDocument(nullptr);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"_ostr));
            if (pS1 && pS1->GetValue() == "Document")
            {
                pDocument = pObject1;
            }
        }
    }
    CPPUNIT_ASSERT(pDocument);

    auto pKidsD = dynamic_cast<vcl::filter::PDFArrayElement*>(pDocument->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsD);
    // assume there are no MCID ref at this level
    auto pKidsDv = pKidsD->GetElements();
    auto pRefKidD0 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsDv[0]);
    CPPUNIT_ASSERT(pRefKidD0);
    auto pObjectD0 = pRefKidD0->LookupObject();
    CPPUNIT_ASSERT(pObjectD0);
    auto pTypeD0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD0->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD0->GetValue());
    auto pSD0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD0->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("H1"_ostr, pSD0->GetValue());

    auto pKidsD0 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD0->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsD0);
    auto pKidsD0v = pKidsD0->GetElements();
    auto pRefKidD00 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD0v[0]);
    // MCID for label
    CPPUNIT_ASSERT(!pRefKidD00);

    // MCID for text
    auto pRefKidD01 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD0v[1]);
    CPPUNIT_ASSERT(!pRefKidD01);

    auto pRefKidD1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsDv[1]);
    CPPUNIT_ASSERT(pRefKidD1);
    auto pObjectD1 = pRefKidD1->LookupObject();
    CPPUNIT_ASSERT(pObjectD1);
    auto pTypeD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD1->GetValue());
    auto pSD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("H2"_ostr, pSD1->GetValue());

    auto pKidsD1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD1->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsD1);
    auto pKidsD1v = pKidsD1->GetElements();

    // MCID for text
    auto pRefKidD11 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD1v[0]);
    CPPUNIT_ASSERT(!pRefKidD11);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testSpans)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"spanlist.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has two pages.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aPages.size());

    int nDoc(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"_ostr));
            if (pS1 && pS1->GetValue() == "Document")
            {
                auto pKids1
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids1);
                // assume there are no MCID ref at this level
                auto vKids1 = pKids1->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids1.size());
                auto pRefKid10 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1[0]);
                CPPUNIT_ASSERT(pRefKid10);
                auto pObject10 = pRefKid10->LookupObject();
                CPPUNIT_ASSERT(pObject10);
                auto pType10
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType10->GetValue());
                auto pS10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("L"_ostr, pS10->GetValue());

                auto pKids10
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids10);
                // assume there are no MCID ref at this level
                auto vKids10 = pKids10->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(4), vKids10.size());

                auto pRefKid100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10[0]);
                CPPUNIT_ASSERT(pRefKid100);
                auto pObject100 = pRefKid100->LookupObject();
                CPPUNIT_ASSERT(pObject100);
                auto pType100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject100->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType100->GetValue());
                auto pS100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject100->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("LI"_ostr, pS100->GetValue());

                auto pKids100
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject100->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids100);
                // assume there are no MCID ref at this level
                auto vKids100 = pKids100->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids100.size());

                auto pRefKid1000 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids100[0]);
                CPPUNIT_ASSERT(pRefKid1000);
                auto pObject1000 = pRefKid1000->LookupObject();
                CPPUNIT_ASSERT(pObject1000);
                auto pType1000
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1000->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1000->GetValue());
                auto pS1000
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1000->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Lbl"_ostr, pS1000->GetValue());

                auto pRefKid1001 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids100[1]);
                CPPUNIT_ASSERT(pRefKid1001);
                auto pObject1001 = pRefKid1001->LookupObject();
                CPPUNIT_ASSERT(pObject1001);
                auto pType1001
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1001->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1001->GetValue());
                auto pS1001
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1001->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("LBody"_ostr, pS1001->GetValue());
                auto pKids1001
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1001->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids1001);
                // assume there are no MCID ref at this level
                auto vKids1001 = pKids1001->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids1001.size());

                auto pRefKid10010 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1001[0]);
                CPPUNIT_ASSERT(pRefKid10010);
                auto pObject10010 = pRefKid10010->LookupObject();
                CPPUNIT_ASSERT(pObject10010);
                auto pType10010
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10010->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType10010->GetValue());
                auto pS10010
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10010->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS10010->GetValue());
                auto pKids10010
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10010->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids10010);
                // assume there are no MCID ref at this level
                auto vKids10010 = pKids10010->GetElements();
                // only one span
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids10010.size());

                auto pRefKid100100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10010[0]);
                CPPUNIT_ASSERT(pRefKid100100);
                auto pObject100100 = pRefKid100100->LookupObject();
                CPPUNIT_ASSERT(pObject100100);
                auto pType100100 = dynamic_cast<vcl::filter::PDFNameElement*>(
                    pObject100100->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType100100->GetValue());
                auto pS100100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject100100->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Span"_ostr, pS100100->GetValue());
                // this span exists because of lang
                auto pLang100100 = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pObject100100->Lookup("Lang"_ostr));
                CPPUNIT_ASSERT_EQUAL("en-GB"_ostr, pLang100100->GetValue());

                auto pRefKid101 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10[1]);
                CPPUNIT_ASSERT(pRefKid101);
                auto pObject101 = pRefKid101->LookupObject();
                CPPUNIT_ASSERT(pObject101);
                auto pType101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject101->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType101->GetValue());
                auto pS101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject101->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("LI"_ostr, pS101->GetValue());

                auto pKids101
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject101->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids101);
                // assume there are no MCID ref at this level
                auto vKids101 = pKids101->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids101.size());

                auto pRefKid1010 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids101[0]);
                CPPUNIT_ASSERT(pRefKid1010);
                auto pObject1010 = pRefKid1010->LookupObject();
                CPPUNIT_ASSERT(pObject1010);
                auto pType1010
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1010->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1010->GetValue());
                auto pS1010
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1010->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Lbl"_ostr, pS1010->GetValue());

                auto pRefKid1011 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids101[1]);
                CPPUNIT_ASSERT(pRefKid1011);
                auto pObject1011 = pRefKid1011->LookupObject();
                CPPUNIT_ASSERT(pObject1011);
                auto pType1011
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1011->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1011->GetValue());
                auto pS1011
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1011->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("LBody"_ostr, pS1011->GetValue());

                auto pKids1011
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1011->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids1011);
                // assume there are no MCID ref at this level
                auto vKids1011 = pKids1011->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids1011.size());

                auto pRefKid10110 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1011[0]);
                CPPUNIT_ASSERT(pRefKid10110);
                auto pObject10110 = pRefKid10110->LookupObject();
                CPPUNIT_ASSERT(pObject10110);
                auto pType10110
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10110->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType10110->GetValue());
                auto pS10110
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10110->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS10110->GetValue());
                auto pKids10110
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10110->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids10110);
                auto vKids10110 = pKids10110->GetElements();
                // only MCIDs, no span
                for (size_t i = 0; i < vKids10110.size(); ++i)
                {
                    auto pKid = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10110[i]);
                    CPPUNIT_ASSERT(!pKid);
                }

                auto pRefKid102 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10[2]);
                CPPUNIT_ASSERT(pRefKid102);
                auto pObject102 = pRefKid102->LookupObject();
                CPPUNIT_ASSERT(pObject102);
                auto pType102
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType102->GetValue());
                auto pS102
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("LI"_ostr, pS102->GetValue());

                auto pKids102
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject102->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids102);
                // assume there are no MCID ref at this level
                auto vKids102 = pKids102->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids102.size());

                auto pRefKid1020 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids102[0]);
                CPPUNIT_ASSERT(pRefKid1020);
                auto pObject1020 = pRefKid1020->LookupObject();
                CPPUNIT_ASSERT(pObject1020);
                auto pType1020
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1020->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1020->GetValue());
                auto pS1020
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1020->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Lbl"_ostr, pS1020->GetValue());

                auto pRefKid1021 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids102[1]);
                CPPUNIT_ASSERT(pRefKid1021);
                auto pObject1021 = pRefKid1021->LookupObject();
                CPPUNIT_ASSERT(pObject1021);
                auto pType1021
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1021->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1021->GetValue());
                auto pS1021
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1021->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("LBody"_ostr, pS1021->GetValue());

                auto pKids1021
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1021->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids1021);
                // assume there are no MCID ref at this level
                auto vKids1021 = pKids1021->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids1021.size());

                auto pRefKid10210 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1021[0]);
                CPPUNIT_ASSERT(pRefKid10210);
                auto pObject10210 = pRefKid10210->LookupObject();
                CPPUNIT_ASSERT(pObject10210);
                auto pType10210
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10210->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType10210->GetValue());
                auto pS10210
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10210->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS10210->GetValue());
                auto pKids10210
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10210->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids10210);
                // assume there are no MCID ref at this level
                auto vKids10210 = pKids10210->GetElements();
                // 2 span and a hyperlink
                CPPUNIT_ASSERT_EQUAL(size_t(3), vKids10210.size());

                auto pRefKid102100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10210[0]);
                CPPUNIT_ASSERT(pRefKid102100);
                auto pObject102100 = pRefKid102100->LookupObject();
                CPPUNIT_ASSERT(pObject102100);
                auto pType102100 = dynamic_cast<vcl::filter::PDFNameElement*>(
                    pObject102100->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType102100->GetValue());
                auto pS102100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102100->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Span"_ostr, pS102100->GetValue());
                auto pKids102100
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject102100->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids102100);
                auto vKids102100 = pKids102100->GetElements();
                for (size_t i = 0; i < vKids102100.size(); ++i)
                {
                    auto pKid = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids102100[i]);
                    CPPUNIT_ASSERT(!pKid);
                }

                auto pRefKid102101 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10210[1]);
                CPPUNIT_ASSERT(pRefKid102101);
                auto pObject102101 = pRefKid102101->LookupObject();
                CPPUNIT_ASSERT(pObject102101);
                auto pType102101 = dynamic_cast<vcl::filter::PDFNameElement*>(
                    pObject102101->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType102101->GetValue());
                auto pS102101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102101->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pS102101->GetValue());
                auto pKids102101
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject102101->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids102101);
                auto vKids102101 = pKids102101->GetElements();
                int nRef(0);
                for (size_t i = 0; i < vKids102101.size(); ++i)
                {
                    auto pKid = dynamic_cast<vcl::filter::PDFDictionaryElement*>(vKids102101[i]);
                    if (pKid)
                    {
                        ++nRef; // annotation
                    }
                }
                CPPUNIT_ASSERT_EQUAL(1, nRef);

                auto pRefKid102102 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10210[2]);
                CPPUNIT_ASSERT(pRefKid102102);
                auto pObject102102 = pRefKid102102->LookupObject();
                CPPUNIT_ASSERT(pObject102102);
                auto pType102102 = dynamic_cast<vcl::filter::PDFNameElement*>(
                    pObject102102->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType102102->GetValue());
                auto pS102102
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102102->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Span"_ostr, pS102102->GetValue());
                auto pKids102102
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject102102->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids102102);
                auto vKids102102 = pKids102102->GetElements();
                // there is a footnote
                int nFtn(0);
                for (size_t i = 0; i < vKids102102.size(); ++i)
                {
                    auto pKid = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids102102[i]);
                    if (pKid)
                    {
                        auto pObject = pKid->LookupObject();
                        CPPUNIT_ASSERT(pObject);
                        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(
                            pObject->Lookup("Type"_ostr));
                        CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType->GetValue());
                        auto pS
                            = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
                        CPPUNIT_ASSERT_EQUAL("Link"_ostr, pS->GetValue());
                        ++nFtn;
                    }
                }
                CPPUNIT_ASSERT_EQUAL(1, nFtn);

                auto pRefKid103 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10[3]);
                CPPUNIT_ASSERT(pRefKid103);
                auto pObject103 = pRefKid103->LookupObject();
                CPPUNIT_ASSERT(pObject103);
                auto pType103
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject103->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType103->GetValue());
                auto pS103
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject103->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("LI"_ostr, pS103->GetValue());

                auto pKids103
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject103->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids103);
                // assume there are no MCID ref at this level
                auto vKids103 = pKids103->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids103.size());

                auto pRefKid1030 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids103[0]);
                CPPUNIT_ASSERT(pRefKid1030);
                auto pObject1030 = pRefKid1030->LookupObject();
                CPPUNIT_ASSERT(pObject1030);
                auto pType1030
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1030->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1030->GetValue());
                auto pS1030
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1030->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Lbl"_ostr, pS1030->GetValue());

                auto pRefKid1031 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids103[1]);
                CPPUNIT_ASSERT(pRefKid1031);
                auto pObject1031 = pRefKid1031->LookupObject();
                CPPUNIT_ASSERT(pObject1031);
                auto pType1031
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1031->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1031->GetValue());
                auto pS1031
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1031->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("LBody"_ostr, pS1031->GetValue());

                auto pKids1031
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1031->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids1031);
                // assume there are no MCID ref at this level
                auto vKids1031 = pKids1031->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids1031.size());

                auto pRefKid10310 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1031[0]);
                CPPUNIT_ASSERT(pRefKid10310);
                auto pObject10310 = pRefKid10310->LookupObject();
                CPPUNIT_ASSERT(pObject10310);
                auto pType10310
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10310->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType10310->GetValue());
                auto pS10310
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10310->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS10310->GetValue());
                auto pKids10310
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10310->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids10310);
                // assume there are no MCID ref at this level
                auto vKids10310 = pKids10310->GetElements();
                // only one span, following a MCID for some strike-out gap
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids10310.size());

                auto pRefKid103100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10310[0]);
                CPPUNIT_ASSERT(!pRefKid103100);

                auto pRefKid103101 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10310[1]);
                CPPUNIT_ASSERT(pRefKid103101);
                auto pObject103101 = pRefKid103101->LookupObject();
                CPPUNIT_ASSERT(pObject103101);
                auto pType103101 = dynamic_cast<vcl::filter::PDFNameElement*>(
                    pObject103101->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType103101->GetValue());
                auto pS103101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject103101->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Span"_ostr, pS103101->GetValue());
                auto pDictA103101 = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                    pObject103101->Lookup("A"_ostr));
                CPPUNIT_ASSERT(pDictA103101 != nullptr);
                CPPUNIT_ASSERT_EQUAL("Layout"_ostr, dynamic_cast<vcl::filter::PDFNameElement*>(
                                                        pDictA103101->LookupElement("O"_ostr))
                                                        ->GetValue());
                CPPUNIT_ASSERT_EQUAL("LineThrough"_ostr,
                                     dynamic_cast<vcl::filter::PDFNameElement*>(
                                         pDictA103101->LookupElement("TextDecorationType"_ostr))
                                         ->GetValue());

                // now the footnote container - following the list
                auto pRefKid11 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1[1]);
                CPPUNIT_ASSERT(pRefKid11);
                auto pObject11 = pRefKid11->LookupObject();
                CPPUNIT_ASSERT(pObject11);
                auto pType11
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType11->GetValue());
                auto pS11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Div"_ostr, pS11->GetValue());

                auto pKids11
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject11->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids11);
                // assume there are no MCID ref at this level
                auto vKids11 = pKids11->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids11.size());

                auto pRefKid110 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids11[0]);
                CPPUNIT_ASSERT(pRefKid110);
                auto pObject110 = pRefKid110->LookupObject();
                CPPUNIT_ASSERT(pObject110);
                auto pType110
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject110->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType110->GetValue());
                auto pS110
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject110->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Note"_ostr, pS110->GetValue());

                auto pKids110
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject110->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids110);
                // assume there are no MCID ref at this level
                auto vKids110 = pKids110->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids110.size());

                auto pRefKid1100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids110[0]);
                CPPUNIT_ASSERT(pRefKid1100);
                auto pObject1100 = pRefKid1100->LookupObject();
                CPPUNIT_ASSERT(pObject1100);
                auto pType1100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1100->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1100->GetValue());
                auto pS1100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1100->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Lbl"_ostr, pS1100->GetValue());

                auto pKids1100
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1100->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids1100);
                // assume there are no MCID ref at this level
                auto vKids1100 = pKids1100->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids1100.size());

                auto pRefKid11000 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1100[0]);
                CPPUNIT_ASSERT(pRefKid11000);
                auto pObject11000 = pRefKid11000->LookupObject();
                CPPUNIT_ASSERT(pObject11000);
                auto pType11000
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11000->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType11000->GetValue());
                auto pS11000
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11000->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pS11000->GetValue());

                auto pRefKid1101 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids110[1]);
                CPPUNIT_ASSERT(pRefKid1101);
                auto pObject1101 = pRefKid1101->LookupObject();
                CPPUNIT_ASSERT(pObject1101);
                auto pType1101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1101->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1101->GetValue());
                auto pS1101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1101->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Footnote"_ostr, pS1101->GetValue());

                ++nDoc;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nDoc);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf157182)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "PDFUACompliance", uno::Any(true) },
        // only happens with PDF/A-1
        { "SelectPdfVersion", uno::Any(static_cast<sal_Int32>(1)) },
    }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    saveAsPDF(u"transparentshape.fodp");

    // just check this does not crash or assert
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf57423)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"Description PDF Export test .odt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    int nFigure(0);
    int nFormula(0);
    int nDiv(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
            if (pS && pS->GetValue() == "Figure")
            {
                switch (nFigure)
                {
                    case 2:
                        CPPUNIT_ASSERT_EQUAL(u"QR Code - Tells how to get to Mosegaard"_ustr,
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"_ostr))));
                        break;
                    case 0:
                        CPPUNIT_ASSERT_EQUAL(u"Title: Arrows - Description:  Explains the "
                                             u"different arrow appearances"_ustr,
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"_ostr))));
                        break;
                    case 1:
                        CPPUNIT_ASSERT_EQUAL(
                            u"My blue triangle - Does not need further description"_ustr,
                            ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                    pObject->Lookup("Alt"_ostr))));
                        break;
                }
                ++nFigure;
            }
            if (pS && pS->GetValue() == "Formula")
            {
                CPPUNIT_ASSERT_EQUAL(
                    u"Equation 1 - Now we give the full description of eq 1 here"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                        *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                            pObject->Lookup("Alt"_ostr))));
                ++nFormula;
            }
            if (pS && pS->GetValue() == "Div")
            {
                switch (nDiv)
                {
                    case 0:
                        CPPUNIT_ASSERT_EQUAL(u"This frame has a description"_ustr,
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"_ostr))));
                        break;
                    case 1:
                        // no properties set on this
                        CPPUNIT_ASSERT(!pObject->Lookup("Alt"_ostr));
                        break;
                    case 2:
                        CPPUNIT_ASSERT_EQUAL(u"My textbox - Has a light background"_ustr,
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"_ostr))));
                        break;
                    case 3:
                        CPPUNIT_ASSERT_EQUAL(u"Hey!  There is no alternate text for Frame "
                                             u"// but maybe not needed?"_ustr,
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"_ostr))));
                        break;
                }
                ++nDiv;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(3, nFigure);
    CPPUNIT_ASSERT_EQUAL(1, nFormula);
    CPPUNIT_ASSERT_EQUAL(4, nDiv);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf154982)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"tdf154982.odt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    int nFigure(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
            if (pS && pS->GetValue() == "Figure")
            {
                switch (nFigure)
                {
                    case 0:
                        CPPUNIT_ASSERT_EQUAL(u"Here comes the signature - Please sign here"_ustr,
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"_ostr))));
                        break;
                    case 1:
                        CPPUNIT_ASSERT_EQUAL(u"Home"_ustr,
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"_ostr))));
                        break;
                }

                // the problem was that the figures in the hell layer were not
                // below their anchor paragraphs in the structure tree
                auto pParentRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("P"_ostr));
                CPPUNIT_ASSERT(pParentRef);
                auto pParent(pParentRef->LookupObject());
                CPPUNIT_ASSERT(pParent);
                auto pParentType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pParentType->GetValue());
                auto pParentS
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pParentS->GetValue());

                auto pPParentRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pParent->Lookup("P"_ostr));
                CPPUNIT_ASSERT(pPParentRef);
                auto pPParent(pPParentRef->LookupObject());
                CPPUNIT_ASSERT(pPParent);
                auto pPParentType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pPParent->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pPParentType->GetValue());
                auto pPParentS
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pPParent->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Document"_ostr, pPParentS->GetValue());
                ++nFigure;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(2, nFigure);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf157397)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"PDF_export_with_formcontrol.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    vcl::filter::PDFObjectElement* pDocument(nullptr);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"_ostr));
            if (pS1 && pS1->GetValue() == "Document")
            {
                pDocument = pObject1;
            }
        }
    }
    CPPUNIT_ASSERT(pDocument);

    auto pKids1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pDocument->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKids1);
    // assume there are no MCID ref at this level
    auto pKids1v = pKids1->GetElements();
    auto pRefKid12 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[2]);
    CPPUNIT_ASSERT(pRefKid12);
    auto pObject12 = pRefKid12->LookupObject();
    CPPUNIT_ASSERT(pObject12);
    auto pType12 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject12->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType12->GetValue());
    auto pS12 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject12->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Text#20body"_ostr, pS12->GetValue());

    auto pKids12 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject12->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKids12);
    // assume there are no MCID ref at this level
    auto pKids12v = pKids12->GetElements();
    auto pRefKid120 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids12v[0]);
    CPPUNIT_ASSERT(pRefKid120);
    auto pObject120 = pRefKid120->LookupObject();
    CPPUNIT_ASSERT(pObject120);
    auto pType120 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject120->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType120->GetValue());
    auto pS120 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject120->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pS120->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject120->Lookup("K"_ostr));
        int nMCID(0);
        int nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pObjR)
            {
                ++nRef;
                auto pOType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
                auto pAnnotRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pObjR->LookupElement("Obj"_ostr));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Annot"_ostr, pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAContents = dynamic_cast<vcl::filter::PDFHexStringElement*>(
                    pAnnot->Lookup("Contents"_ostr));
                CPPUNIT_ASSERT_EQUAL(
                    u"https://klexikon.zum.de/wiki/Kläranlage"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAA
                    = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
                CPPUNIT_ASSERT(pAA);
                auto pAAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Action"_ostr, pAAType->GetValue());
                auto pAAS
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("URI"_ostr, pAAS->GetValue());
                auto pAAURI = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pAA->LookupElement("URI"_ostr));
                CPPUNIT_ASSERT_EQUAL("https://klexikon.zum.de/wiki/Kl%C3%A4ranlage"_ostr,
                                     pAAURI->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(1, nMCID);
        CPPUNIT_ASSERT_EQUAL(1, nRef);
    }

    auto pRefKid13 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[3]);
    CPPUNIT_ASSERT(pRefKid13);
    auto pObject13 = pRefKid13->LookupObject();
    CPPUNIT_ASSERT(pObject13);
    auto pType13 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject13->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType13->GetValue());
    auto pS13 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject13->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Text#20body"_ostr, pS13->GetValue());

    auto pKids13 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject13->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKids13);
    // assume there are no MCID ref at this level
    auto pKids13v = pKids13->GetElements();
    auto pRefKid130 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids13v[0]);
    CPPUNIT_ASSERT(pRefKid130);
    auto pObject130 = pRefKid130->LookupObject();
    CPPUNIT_ASSERT(pObject130);
    auto pType130 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject130->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType130->GetValue());
    auto pS130 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject130->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pS130->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject130->Lookup("K"_ostr));
        int nMCID(0);
        int nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pObjR)
            {
                ++nRef;
                auto pOType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
                auto pAnnotRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pObjR->LookupElement("Obj"_ostr));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Annot"_ostr, pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAContents = dynamic_cast<vcl::filter::PDFHexStringElement*>(
                    pAnnot->Lookup("Contents"_ostr));
                CPPUNIT_ASSERT_EQUAL(
                    u"https://de.wikipedia.org/wiki/Kläranlage#Mechanische_Vorreinigung"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAA
                    = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
                CPPUNIT_ASSERT(pAA);
                auto pAAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Action"_ostr, pAAType->GetValue());
                auto pAAS
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("URI"_ostr, pAAS->GetValue());
                auto pAAURI = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pAA->LookupElement("URI"_ostr));
                CPPUNIT_ASSERT_EQUAL(
                    "https://de.wikipedia.org/wiki/Kl%C3%A4ranlage#Mechanische_Vorreinigung"_ostr,
                    pAAURI->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(1, nMCID);
        CPPUNIT_ASSERT_EQUAL(1, nRef);
    }

    auto pRefKid14 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[4]);
    CPPUNIT_ASSERT(pRefKid14);
    auto pObject14 = pRefKid14->LookupObject();
    CPPUNIT_ASSERT(pObject14);
    auto pType14 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject14->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType14->GetValue());
    auto pS14 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject14->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Text#20body"_ostr, pS14->GetValue());

    auto pKids14 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject14->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKids14);
    // assume there are no MCID ref at this level
    auto pKids14v = pKids14->GetElements();
    auto pRefKid140 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids14v[0]);
    CPPUNIT_ASSERT(pRefKid140);
    auto pObject140 = pRefKid140->LookupObject();
    CPPUNIT_ASSERT(pObject140);
    auto pType140 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject140->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType140->GetValue());
    auto pS140 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject140->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pS140->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject140->Lookup("K"_ostr));
        int nMCID(0);
        int nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pObjR)
            {
                ++nRef;
                auto pOType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
                auto pAnnotRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pObjR->LookupElement("Obj"_ostr));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Annot"_ostr, pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAContents = dynamic_cast<vcl::filter::PDFHexStringElement*>(
                    pAnnot->Lookup("Contents"_ostr));
                CPPUNIT_ASSERT_EQUAL(
                    u"https://vr-easy.com/tour/usr/220113-virtuellerschulausflug/#pano=24"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                CPPUNIT_ASSERT_EQUAL("Link"_ostr, pASubtype->GetValue());
                auto pAA
                    = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
                CPPUNIT_ASSERT(pAA);
                auto pAAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Action"_ostr, pAAType->GetValue());
                auto pAAS
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("URI"_ostr, pAAS->GetValue());
                auto pAAURI = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pAA->LookupElement("URI"_ostr));
                CPPUNIT_ASSERT_EQUAL(
                    "https://vr-easy.com/tour/usr/220113-virtuellerschulausflug/#pano=24"_ostr,
                    pAAURI->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(1, nMCID);
        CPPUNIT_ASSERT_EQUAL(1, nRef);
    }

    auto pRefKid16 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[6]);
    CPPUNIT_ASSERT(pRefKid16);
    auto pObject16 = pRefKid16->LookupObject();
    CPPUNIT_ASSERT(pObject16);
    auto pType16 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject16->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType16->GetValue());
    auto pS16 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject16->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Text#20body"_ostr, pS16->GetValue());

    auto pKids16 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject16->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKids16);
    // assume there are no MCID ref at this level
    auto pKids16v = pKids16->GetElements();
    auto pRefKid160 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids16v[0]);
    CPPUNIT_ASSERT(pRefKid160);
    auto pObject160 = pRefKid160->LookupObject();
    CPPUNIT_ASSERT(pObject160);
    auto pType160 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject160->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType160->GetValue());
    auto pS160 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject160->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Form"_ostr, pS160->GetValue());
    auto pA160Dict = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject160->Lookup("A"_ostr));
    CPPUNIT_ASSERT(pA160Dict);
    auto pA160O = dynamic_cast<vcl::filter::PDFNameElement*>(pA160Dict->LookupElement("O"_ostr));
    CPPUNIT_ASSERT(pA160O);
    CPPUNIT_ASSERT_EQUAL("PrintField"_ostr, pA160O->GetValue());
    auto pA160Role
        = dynamic_cast<vcl::filter::PDFNameElement*>(pA160Dict->LookupElement("Role"_ostr));
    CPPUNIT_ASSERT(pA160Role);
    CPPUNIT_ASSERT_EQUAL("tv"_ostr, pA160Role->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject160->Lookup("K"_ostr));
        int nMCID(0);
        int nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pObjR)
            {
                ++nRef;
                auto pOType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
                auto pAnnotRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pObjR->LookupElement("Obj"_ostr));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("Annot"_ostr, pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr));
                CPPUNIT_ASSERT_EQUAL("Widget"_ostr, pASubtype->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(1, nMCID);
        CPPUNIT_ASSERT_EQUAL(1, nRef);
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf135192)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"tdf135192-1.fodp");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    int nTable(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"_ostr));
            if (pS1 && pS1->GetValue() == "Table")
            {
                int nTR(0);
                auto pKids1
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids1);
                // there can be additional children, such as MCID ref
                for (auto pKid1 : pKids1->GetElements())
                {
                    auto pRefKid1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKid1);
                    if (pRefKid1)
                    {
                        auto pObject2 = pRefKid1->LookupObject();
                        if (pObject2)
                        {
                            auto pType2 = dynamic_cast<vcl::filter::PDFNameElement*>(
                                pObject2->Lookup("Type"_ostr));
                            if (pType2 && pType2->GetValue() == "StructElem")
                            {
                                auto pS2 = dynamic_cast<vcl::filter::PDFNameElement*>(
                                    pObject2->Lookup("S"_ostr));
                                if (pS2 && pS2->GetValue() == "TR")
                                {
                                    int nTD(0);
                                    auto pKids2 = dynamic_cast<vcl::filter::PDFArrayElement*>(
                                        pObject2->Lookup("K"_ostr));
                                    CPPUNIT_ASSERT(pKids2);
                                    for (auto pKid2 : pKids2->GetElements())
                                    {
                                        auto pRefKid2
                                            = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                                                pKid2);
                                        if (pRefKid2)
                                        {
                                            auto pObject3 = pRefKid2->LookupObject();
                                            if (pObject3)
                                            {
                                                auto pType3
                                                    = dynamic_cast<vcl::filter::PDFNameElement*>(
                                                        pObject3->Lookup("Type"_ostr));
                                                if (pType3 && pType3->GetValue() == "StructElem")
                                                {
                                                    auto pS3 = dynamic_cast<
                                                        vcl::filter::PDFNameElement*>(
                                                        pObject3->Lookup("S"_ostr));
                                                    if (nTR == 0 && pS3 && pS3->GetValue() == "TH")
                                                    {
                                                        int nOTable(0);
                                                        auto pAttrs = dynamic_cast<
                                                            vcl::filter::PDFArrayElement*>(
                                                            pObject3->Lookup("A"_ostr));
                                                        CPPUNIT_ASSERT(pAttrs != nullptr);
                                                        for (const auto& rAttrRef :
                                                             pAttrs->GetElements())
                                                        {
                                                            auto pAttrDict = dynamic_cast<
                                                                vcl::filter::PDFDictionaryElement*>(
                                                                rAttrRef);
                                                            CPPUNIT_ASSERT(pAttrDict != nullptr);
                                                            auto pOwner = dynamic_cast<
                                                                vcl::filter::PDFNameElement*>(
                                                                pAttrDict->LookupElement("O"_ostr));
                                                            CPPUNIT_ASSERT(pOwner != nullptr);
                                                            if (pOwner->GetValue() == "Table")
                                                            {
                                                                auto pScope = dynamic_cast<
                                                                    vcl::filter::PDFNameElement*>(
                                                                    pAttrDict->LookupElement(
                                                                        "Scope"_ostr));
                                                                CPPUNIT_ASSERT(pScope != nullptr);
                                                                CPPUNIT_ASSERT_EQUAL(
                                                                    "Column"_ostr,
                                                                    pScope->GetValue());
                                                                ++nOTable;
                                                            }
                                                        }
                                                        CPPUNIT_ASSERT_EQUAL(1, nOTable);
                                                        ++nTD;
                                                    }
                                                    else if (nTR != 0 && pS3
                                                             && pS3->GetValue() == "TD")
                                                    {
                                                        ++nTD;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    CPPUNIT_ASSERT_EQUAL(3, nTD);
                                    ++nTR;
                                }
                            }
                        }
                    }
                }
                CPPUNIT_ASSERT_EQUAL(2, nTR);
                ++nTable;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nTable);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf154955)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"grouped-shape.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();

    enum
    {
        Default,
        Artifact,
        Tagged
    } state
        = Default;

    int nLine(0);
    int nTagged(0);
    int nArtifacts(0);
    while (true)
    {
        ++nLine;
        auto const pLine = ::std::find(pStart, pEnd, '\n');
        if (pLine == pEnd)
        {
            break;
        }
        std::string_view const line(pStart, pLine - pStart);
        pStart = pLine + 1;
        if (!line.empty() && line[0] != '%')
        {
            ::std::cerr << nLine << ": " << line << "\n";
            if (o3tl::starts_with(line, "/Artifact "))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Artifact;
                ++nArtifacts;
            }
            else if (o3tl::starts_with(line, "/Figure<</MCID "))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Tagged;
                ++nTagged;
            }
            else if (line == "EMC")
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected end", state != Default);
                state = Default;
            }
            else if (nLine > 1) // first line is expected "0.1 w"
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected content outside MCS", state != Default);
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unclosed MCS", Default, state);
    CPPUNIT_ASSERT_EQUAL(2, nTagged);
    CPPUNIT_ASSERT_GREATEREQUAL(1, nArtifacts);

    int nFigure(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
            if (pS && pS->GetValue() == "Figure")
            {
                switch (nFigure)
                {
                    case 0:
                        CPPUNIT_ASSERT_EQUAL(u"Two rectangles - Grouped"_ustr,
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"_ostr))));
                        break;
                    case 1:
                        CPPUNIT_ASSERT_EQUAL(u"these ones are green"_ustr,
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"_ostr))));
                        break;
                }

                auto pParentRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("P"_ostr));
                CPPUNIT_ASSERT(pParentRef);
                auto pParent(pParentRef->LookupObject());
                CPPUNIT_ASSERT(pParent);
                auto pParentType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pParentType->GetValue());
                auto pParentS
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pParentS->GetValue());

                ++nFigure;
            }
        }
    }
    // the problem was that there were 4 shapes (the sub-shapes of the 2 groups)
    CPPUNIT_ASSERT_EQUAL(2, nFigure);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf155190)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    saveAsPDF(u"tdf155190.odt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    int nDiv(0);
    int nFigure(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));

        auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"_ostr));
        // start with the text box
        if (pType1 && pType1->GetValue() == "StructElem" && pS1 && pS1->GetValue() == "Div")
        {
            ++nDiv;
            auto pKids1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1->Lookup("K"_ostr));
            CPPUNIT_ASSERT(pKids1);
            for (auto pKid1 : pKids1->GetElements())
            {
                auto pRefKid1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKid1);
                if (pRefKid1)
                {
                    auto pObject2 = pRefKid1->LookupObject();
                    CPPUNIT_ASSERT(pObject2);
                    auto pType2
                        = dynamic_cast<vcl::filter::PDFNameElement*>(pObject2->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType2);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType2->GetValue());
                    auto pS2
                        = dynamic_cast<vcl::filter::PDFNameElement*>(pObject2->Lookup("S"_ostr));
                    CPPUNIT_ASSERT_EQUAL("FigureCaption"_ostr, pS2->GetValue());
                    auto pKids2
                        = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject2->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids2);
                    // there are additional children, MCID ref
                    for (auto pKid2 : pKids2->GetElements())
                    {
                        auto pRefKid2 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKid2);
                        if (pRefKid2)
                        {
                            auto pObject3 = pRefKid2->LookupObject();
                            CPPUNIT_ASSERT(pObject3);
                            auto pType3 = dynamic_cast<vcl::filter::PDFNameElement*>(
                                pObject3->Lookup("Type"_ostr));
                            if (pType3 && pType3->GetValue() == "StructElem")
                            {
                                auto pS3 = dynamic_cast<vcl::filter::PDFNameElement*>(
                                    pObject3->Lookup("S"_ostr));
                                CPPUNIT_ASSERT_EQUAL("Figure"_ostr, pS3->GetValue());
                                auto pAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                    pObject3->Lookup("Alt"_ostr));
                                CPPUNIT_ASSERT_EQUAL(
                                    u"Picture of apples"_ustr,
                                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAlt));
                                auto pKids3 = dynamic_cast<vcl::filter::PDFArrayElement*>(
                                    pObject3->Lookup("K"_ostr));
                                CPPUNIT_ASSERT(pKids3);
                                // the problem was that this didn't reference an MCID
                                CPPUNIT_ASSERT(!pKids3->GetElements().empty());
                                ++nFigure;
                            }
                        }
                    }
                }
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nDiv);
    CPPUNIT_ASSERT_EQUAL(1, nFigure);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testMediaShapeAnnot)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    saveAsPDF(u"vid.odt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);

    // There should be one annotation
    CPPUNIT_ASSERT_EQUAL(size_t(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    // check /Annot - produced by sw
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(
        "Annot"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        "Screen"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr))->GetValue());

    auto pA = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
    CPPUNIT_ASSERT(pA);
    auto pR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pA->LookupElement("R"_ostr));
    CPPUNIT_ASSERT(pR);
    auto pC = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pR->LookupElement("C"_ostr));
    CPPUNIT_ASSERT(pC);
    auto pCT = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pC->LookupElement("CT"_ostr));
    CPPUNIT_ASSERT_EQUAL("video/webm"_ostr, pCT->GetValue());
    auto pD = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pC->LookupElement("D"_ostr));
    CPPUNIT_ASSERT(pD);
    auto pDesc = dynamic_cast<vcl::filter::PDFHexStringElement*>(pD->LookupElement("Desc"_ostr));
    CPPUNIT_ASSERT(pDesc);
    CPPUNIT_ASSERT_EQUAL(u"alternativloser text\nand some description"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pDesc));
    auto pAlt = dynamic_cast<vcl::filter::PDFArrayElement*>(pC->LookupElement("Alt"_ostr));
    CPPUNIT_ASSERT(pAlt);
    auto pLang = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pAlt->GetElement(0));
    CPPUNIT_ASSERT_EQUAL(""_ostr, pLang->GetValue());
    auto pAltText = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAlt->GetElement(1));
    CPPUNIT_ASSERT_EQUAL(u"alternativloser text\nand some description"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAltText));

    auto pStructParent
        = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"_ostr));
    CPPUNIT_ASSERT(pStructParent);

    vcl::filter::PDFReferenceElement* pStructElemRef(nullptr);

    // check ParentTree to find StructElem
    int nRoots(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructTreeRoot")
        {
            ++nRoots;
            auto pParentTree = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                pObject1->Lookup("ParentTree"_ostr));
            CPPUNIT_ASSERT(pParentTree);
            auto pNumTree = pParentTree->LookupObject();
            CPPUNIT_ASSERT(pNumTree);
            auto pNums = dynamic_cast<vcl::filter::PDFArrayElement*>(pNumTree->Lookup("Nums"_ostr));
            CPPUNIT_ASSERT(pNums);
            int nFound(0);
            for (size_t i = 0; i < pNums->GetElements().size(); i += 2)
            {
                auto pI = dynamic_cast<vcl::filter::PDFNumberElement*>(pNums->GetElement(i));
                if (pI->GetValue() == pStructParent->GetValue())
                {
                    ++nFound;
                    CPPUNIT_ASSERT(i < pNums->GetElements().size() - 1);
                    pStructElemRef
                        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pNums->GetElement(i + 1));
                    CPPUNIT_ASSERT(pStructElemRef);
                }
            }
            CPPUNIT_ASSERT_EQUAL(1, nFound);
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nRoots);

    // check /StructElem - produced by drawinglayer
    CPPUNIT_ASSERT(pStructElemRef);
    auto pStructElem(pStructElemRef->LookupObject());
    CPPUNIT_ASSERT(pStructElem);

    auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType->GetValue());
    auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Annot"_ostr, pS->GetValue());
    auto pSEAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(pStructElem->Lookup("Alt"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"alternativloser text - and some description"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pSEAlt));
    auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pStructElem->Lookup("K"_ostr));
    int nMCID(0);
    int nRef(0);
    for (size_t i = 0; i < pKids->GetElements().size(); ++i)
    {
        auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
        auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
        if (pNum)
        {
            ++nMCID;
        }
        if (pObjR)
        {
            ++nRef;
            auto pOType
                = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
            CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
            auto pAnnotRef
                = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->LookupElement("Obj"_ostr));
            CPPUNIT_ASSERT_EQUAL(pAnnot, pAnnotRef->LookupObject());
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nMCID);
    CPPUNIT_ASSERT_EQUAL(1, nRef);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testFlyFrameHyperlinkAnnot)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    saveAsPDF(u"image-hyperlink-alttext.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);

    // There should be one annotation
    CPPUNIT_ASSERT_EQUAL(size_t(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    // check /Annot - produced by sw
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(
        "Annot"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        "Link"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr))->GetValue());

    auto pContents
        = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Image2"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pContents));

    auto pStructParent
        = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"_ostr));
    CPPUNIT_ASSERT(pStructParent);

    vcl::filter::PDFReferenceElement* pStructElemRef(nullptr);

    // check ParentTree to find StructElem
    int nRoots(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructTreeRoot")
        {
            ++nRoots;
            auto pParentTree = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                pObject1->Lookup("ParentTree"_ostr));
            CPPUNIT_ASSERT(pParentTree);
            auto pNumTree = pParentTree->LookupObject();
            CPPUNIT_ASSERT(pNumTree);
            auto pNums = dynamic_cast<vcl::filter::PDFArrayElement*>(pNumTree->Lookup("Nums"_ostr));
            CPPUNIT_ASSERT(pNums);
            int nFound(0);
            for (size_t i = 0; i < pNums->GetElements().size(); i += 2)
            {
                auto pI = dynamic_cast<vcl::filter::PDFNumberElement*>(pNums->GetElement(i));
                if (pI->GetValue() == pStructParent->GetValue())
                {
                    ++nFound;
                    CPPUNIT_ASSERT(i < pNums->GetElements().size() - 1);
                    pStructElemRef
                        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pNums->GetElement(i + 1));
                    CPPUNIT_ASSERT(pStructElemRef);
                }
            }
            CPPUNIT_ASSERT_EQUAL(1, nFound);
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nRoots);

    // check /StructElem - produced by sw painting code
    CPPUNIT_ASSERT(pStructElemRef);
    auto pStructElem(pStructElemRef->LookupObject());
    CPPUNIT_ASSERT(pStructElem);

    auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType->GetValue());
    auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pS->GetValue());
    auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pStructElem->Lookup("K"_ostr));
    int nMCID(0);
    int nRef(0);
    for (size_t i = 0; i < pKids->GetElements().size(); ++i)
    {
        auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
        auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
        if (pNum)
        {
            ++nMCID;
        }
        if (pObjR)
        {
            ++nRef;
            auto pOType
                = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
            CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
            auto pAnnotRef
                = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->LookupElement("Obj"_ostr));
            CPPUNIT_ASSERT_EQUAL(pAnnot, pAnnotRef->LookupObject());
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nMCID);
    CPPUNIT_ASSERT_EQUAL(1, nRef);

    // the Link is inside a Figure
    auto pParentRef
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pStructElem->Lookup("P"_ostr));
    CPPUNIT_ASSERT(pParentRef);
    auto pParent(pParentRef->LookupObject());
    CPPUNIT_ASSERT(pParent);
    auto pParentType = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pParentType->GetValue());
    auto pParentS = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Figure"_ostr, pParentS->GetValue());
    auto pAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(pParent->Lookup("Alt"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Ship drawing - Very cute"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAlt));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testFormControlAnnot)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    saveAsPDF(u"formcontrol.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);

    // There should be one annotation
    CPPUNIT_ASSERT_EQUAL(size_t(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    // check /Annot
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(
        "Annot"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        "Widget"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr))->GetValue());
    auto pT = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pAnnot->Lookup("T"_ostr));
    CPPUNIT_ASSERT(pT);
    CPPUNIT_ASSERT_EQUAL("Check Box 1"_ostr, pT->GetValue());
    auto pTU = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("TU"_ostr));
    CPPUNIT_ASSERT(pTU);
    CPPUNIT_ASSERT_EQUAL(u"helpful text"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pTU));

    auto pStructParent
        = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"_ostr));
    CPPUNIT_ASSERT(pStructParent);

    vcl::filter::PDFReferenceElement* pStructElemRef(nullptr);

    // check ParentTree to find StructElem
    int nRoots(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
        if (pType1 && pType1->GetValue() == "StructTreeRoot")
        {
            ++nRoots;
            auto pParentTree = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                pObject1->Lookup("ParentTree"_ostr));
            CPPUNIT_ASSERT(pParentTree);
            auto pNumTree = pParentTree->LookupObject();
            CPPUNIT_ASSERT(pNumTree);
            auto pNums = dynamic_cast<vcl::filter::PDFArrayElement*>(pNumTree->Lookup("Nums"_ostr));
            CPPUNIT_ASSERT(pNums);
            int nFound(0);
            for (size_t i = 0; i < pNums->GetElements().size(); i += 2)
            {
                auto pI = dynamic_cast<vcl::filter::PDFNumberElement*>(pNums->GetElement(i));
                if (pI->GetValue() == pStructParent->GetValue())
                {
                    ++nFound;
                    CPPUNIT_ASSERT(i < pNums->GetElements().size() - 1);
                    pStructElemRef
                        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pNums->GetElement(i + 1));
                    CPPUNIT_ASSERT(pStructElemRef);
                }
            }
            CPPUNIT_ASSERT_EQUAL(1, nFound);
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nRoots);

    // check /StructElem
    CPPUNIT_ASSERT(pStructElemRef);
    auto pStructElem(pStructElemRef->LookupObject());
    CPPUNIT_ASSERT(pStructElem);

    auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType->GetValue());
    auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Form"_ostr, pS->GetValue());
    auto pAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(pStructElem->Lookup("Alt"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"textuelle alternative - a box to check"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAlt));
    auto pA = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pStructElem->Lookup("A"_ostr));
    CPPUNIT_ASSERT(pA);
    auto pO = dynamic_cast<vcl::filter::PDFNameElement*>(pA->LookupElement("O"_ostr));
    CPPUNIT_ASSERT(pO);
    CPPUNIT_ASSERT_EQUAL("PrintField"_ostr, pO->GetValue());
    auto pRole = dynamic_cast<vcl::filter::PDFNameElement*>(pA->LookupElement("Role"_ostr));
    CPPUNIT_ASSERT(pRole);
    CPPUNIT_ASSERT_EQUAL("cb"_ostr, pRole->GetValue());
    auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pStructElem->Lookup("K"_ostr));
    int nMCID(0);
    int nRef(0);
    for (size_t i = 0; i < pKids->GetElements().size(); ++i)
    {
        auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
        auto pObjR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pKids->GetElement(i));
        if (pNum)
        {
            ++nMCID;
        }
        if (pObjR)
        {
            ++nRef;
            auto pOType
                = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->LookupElement("Type"_ostr));
            CPPUNIT_ASSERT_EQUAL("OBJR"_ostr, pOType->GetValue());
            auto pAnnotRef
                = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->LookupElement("Obj"_ostr));
            CPPUNIT_ASSERT_EQUAL(pAnnot, pAnnotRef->LookupObject());
        }
    }
    CPPUNIT_ASSERT_EQUAL(1, nMCID);
    CPPUNIT_ASSERT_EQUAL(1, nRef);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf142129)
{
    loadFromFile(u"master.odm");

    // update linked section
    dispatchCommand(mxComponent, u".uno:UpdateAllLinks"_ustr, {});

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable Outlines export
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "ExportBookmarks", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    auto* pCatalog = aDocument.GetCatalog();
    CPPUNIT_ASSERT(pCatalog);
    auto* pCatalogDictionary = pCatalog->GetDictionary();
    CPPUNIT_ASSERT(pCatalogDictionary);
    auto* pOutlinesObject = pCatalogDictionary->LookupObject("Outlines"_ostr);
    CPPUNIT_ASSERT(pOutlinesObject);
    auto* pOutlinesDictionary = pOutlinesObject->GetDictionary();
#if 0
    // Type isn't actually written currently
    auto* pType
        = dynamic_cast<vcl::filter::PDFNameElement*>(pOutlinesDictionary->LookupElement("Type"));
    CPPUNIT_ASSERT(pType);
    CPPUNIT_ASSERT_EQUAL(OString("Outlines"), pType->GetValue());
#endif

    auto* pFirst = dynamic_cast<vcl::filter::PDFReferenceElement*>(
        pOutlinesDictionary->LookupElement("First"_ostr));
    CPPUNIT_ASSERT(pFirst);
    auto* pFirstD = pFirst->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirstD);
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirstD->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(u"Preface"_ustr, ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                              *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                  pFirstD->LookupElement("Title"_ostr))));

    auto* pFirst1
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFirstD->LookupElement("First"_ostr));
    CPPUNIT_ASSERT(pFirst1);
    auto* pFirst1D = pFirst1->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirst1D);
    // here is a hidden section with headings "Copyright" etc.; check that
    // there are no outline entries for it
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirst1D->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(u"Who is this book for?"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                             *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                 pFirst1D->LookupElement("Title"_ostr))));

    auto* pFirst2
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFirst1D->LookupElement("Next"_ostr));
    auto* pFirst2D = pFirst2->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirst2D);
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirst2D->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(u"What\u2019s in this book?"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                             *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                 pFirst2D->LookupElement("Title"_ostr))));

    auto* pFirst3
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFirst2D->LookupElement("Next"_ostr));
    auto* pFirst3D = pFirst3->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirst3D);
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirst3D->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(u"Minimum requirements for using LibreOffice"_ustr,
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                             *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                 pFirst3D->LookupElement("Title"_ostr))));

    CPPUNIT_ASSERT_EQUAL(static_cast<vcl::filter::PDFElement*>(nullptr),
                         pFirst3D->LookupElement("Next"_ostr));
    CPPUNIT_ASSERT_EQUAL(static_cast<vcl::filter::PDFElement*>(nullptr),
                         pFirstD->LookupElement("Next"_ostr));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testPdfImageRotate180)
{
    // Create an empty document.
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    // Insert the PDF image.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphicObject(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    OUString aURL = createFileURL(u"pdf-image-rotate-180.pdf");
    xGraphicObject->setPropertyValue(u"GraphicURL"_ustr, uno::Any(aURL));
    uno::Reference<drawing::XShape> xShape(xGraphicObject, uno::UNO_QUERY);
    xShape->setSize(awt::Size(1000, 1000));
    uno::Reference<text::XTextContent> xTextContent(xGraphicObject, uno::UNO_QUERY);
    xText->insertTextContent(xCursor->getStart(), xTextContent, /*bAbsorb=*/false);

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Make sure that the page -> form -> form has a child image.
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pPageObject->getType());
    // 2: white background and the actual object.
    CPPUNIT_ASSERT_EQUAL(2, pPageObject->getFormObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pFormObject = pPageObject->getFormObject(1);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pFormObject->getType());
    CPPUNIT_ASSERT_EQUAL(1, pFormObject->getFormObjectCount());

    // Check if the inner form object (original page object in the pdf image) has the correct
    // rotation.
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pInnerFormObject = pFormObject->getFormObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pInnerFormObject->getType());
    CPPUNIT_ASSERT_EQUAL(1, pInnerFormObject->getFormObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pImage = pInnerFormObject->getFormObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Image, pImage->getType());
    basegfx::B2DHomMatrix aMat = pInnerFormObject->getMatrix();
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate = 0;
    double fShearX = 0;
    aMat.decompose(aScale, aTranslate, fRotate, fShearX);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -1
    // - Actual  : 1
    // i.e. the 180 degrees rotation didn't happen (via a combination of horizontal + vertical
    // flip).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0, aScale.getX(), 0.01);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf144222)
{
// Assume Windows has the font for U+4E2D
#ifdef _WIN32
    aMediaDescriptor["FilterName"] <<= OUString("calc_pdf_Export");
    saveAsPDF(u"tdf144222.ods");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    int nPageObjectCount = pPdfPage->getObjectCount();
    const OUString sChar = u"\u4E2D"_ustr;
    basegfx::B2DRectangle aRect1, aRect2;
    int nCount = 0;

    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPdfPageObject = pPdfPage->getObject(i);
        if (pPdfPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            ++nCount;
            OUString sText = pPdfPageObject->getText(pTextPage);
            if (sText == sChar)
                aRect1 = pPdfPageObject->getBounds();
            else
                aRect2 = pPdfPageObject->getBounds();
        }
    }

    CPPUNIT_ASSERT_EQUAL(2, nCount);
    CPPUNIT_ASSERT(!aRect1.isEmpty());
    CPPUNIT_ASSERT(!aRect2.isEmpty());
    CPPUNIT_ASSERT(!aRect1.overlaps(aRect2));
#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf145873)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor[u"FilterName"_ustr] <<= u"impress_pdf_Export"_ustr;
    saveAsPDF(u"tdf145873.pptx");

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    int nPageObjectCount = pPdfPage->getObjectCount();

    // tdf#145873: Without the fix #1 in place, this test would have failed with
    // - Expected: 66
    // - Actual  : 3
    CPPUNIT_ASSERT_EQUAL(66, nPageObjectCount);

    auto pObject = pPdfPage->getObject(4);
    CPPUNIT_ASSERT_MESSAGE("no object", pObject != nullptr);

    // tdf#145873: Without the fix #2 in place, this test would have failed with
    // - Expected: 13.40
    // - Actual  : 3.57...
    // - Delta   : 0.1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.40, pObject->getBounds().getWidth(), 0.1);
    // - Expected: 13.79
    // - Actual  : 3.74...
    // - Delta   : 0.1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.79, pObject->getBounds().getHeight(), 0.1);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testPdfImageHyperlink)
{
    // Given a Draw file, containing a PDF image, which has a hyperlink in it:
    aMediaDescriptor[u"FilterName"_ustr] <<= u"draw_pdf_Export"_ustr;

    // When saving to PDF:
    saveAsPDF(u"pdf-image-hyperlink.odg");

    // Then make sure that link is preserved:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    // Without the accompanying fix in place, this test would have failed, the hyperlink of the PDF
    // image was lost.
    CPPUNIT_ASSERT(pPdfPage->hasLinks());

    // Also test the precision of the form XObject.
    // Given a full-page form XObject, page height is 27.94 cm (792 points):
    // When writing the reciprocal of the object height to PDF:
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pFormObject;
    for (int i = 0; i < pPdfPage->getObjectCount(); ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pObject = pPdfPage->getObject(i);
        if (pObject->getType() == vcl::pdf::PDFPageObjectType::Form)
        {
            pFormObject = std::move(pObject);
            break;
        }
    }
    CPPUNIT_ASSERT(pFormObject);
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pInnerFormObject;
    for (int i = 0; i < pFormObject->getFormObjectCount(); ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pObject = pFormObject->getFormObject(i);
        if (pObject->getType() == vcl::pdf::PDFPageObjectType::Form)
        {
            pInnerFormObject = std::move(pObject);
            break;
        }
    }
    CPPUNIT_ASSERT(pInnerFormObject);
    // Then make sure that enough digits are used, so the point size is unchanged:
    basegfx::B2DHomMatrix aMatrix = pInnerFormObject->getMatrix();
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate{};
    double fShearX{};
    aMatrix.decompose(aScale, aTranslate, fRotate, fShearX);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0.0012626264
    // - Actual  : 0.00126
    // i.e. the rounded reciprocal was 794 points, not the original 792.
    CPPUNIT_ASSERT_EQUAL(0.0012626264, rtl::math::round(aScale.getY(), 10));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testURIs)
{
    struct
    {
        OUString in;
        OString out;
        bool relativeFsys;
    } URIs[] = { {
                     u"http://example.com/"_ustr,
                     "http://example.com/"_ostr,
                     true,
                 },
                 {
                     u"file://localfile.odt/"_ustr,
                     "file://localfile.odt/"_ostr,
                     true,
                 },
                 {
                     // tdf 143216
                     u"http://username:password@example.com"_ustr,
                     "http://username:password@example.com"_ostr,
                     true,
                 },
                 {
                     u"git://git.example.org/project/example"_ustr,
                     "git://git.example.org/project/example"_ostr,
                     true,
                 },
                 {
                     // The odt/pdf gets substituted due to 'ConvertOOoTargetToPDFTarget'
                     u"filebypath.odt"_ustr,
                     "filebypath.pdf"_ostr,
                     true,
                 },
                 {
                     // The odt/pdf gets substituted due to 'ConvertOOoTargetToPDFTarget'
                     // but this time with ExportLinksRelativeFsys off the path is added
                     u"filebypath.odt"_ustr,
                     OUStringToOString(utl::GetTempNameBaseDirectory(), RTL_TEXTENCODING_UTF8)
                         + "filebypath.pdf",
                     false,
                 },
                 {
                     // This also gets made relative due to 'ExportLinksRelativeFsys'
                     utl::GetTempNameBaseDirectory() + "fileintempdir.odt",
                     "fileintempdir.pdf"_ostr,
                     true,
                 } };

    // Create an empty document.
    // Note: The test harness gets very upset if we try and create multiple
    // documents, or recreate it; so reuse one instance for all the links
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"Test pdf"_ustr, /*bAbsorb=*/false);

    // Set the name so it can do relative name replacement
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    xModel->attachResource(maTempFile.GetURL(), xModel->getArgs());

    for (unsigned int i = 0; i < (sizeof(URIs) / sizeof(URIs[0])); i++)
    {
        // Test the filename rewriting
        uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
            { "ExportLinksRelativeFsys", uno::Any(URIs[i].relativeFsys) },
            { "ConvertOOoTargetToPDFTarget", uno::Any(true) },
        }));
        aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

        // Add a link (based on testNestedHyperlink in rtfexport3)
        xCursor->gotoStart(/*bExpand=*/false);
        xCursor->gotoEnd(/*bExpand=*/true);
        uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
        xCursorProps->setPropertyValue(u"HyperLinkURL"_ustr, uno::Any(URIs[i].in));

        // Save as PDF.
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
        xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

        // Use the filter rather than the pdfium route, as per the tdf105093 test, it's
        // easier to parse the annotations
        vcl::filter::PDFDocument aDocument;

        // Parse the export result.
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
        CPPUNIT_ASSERT(aDocument.Read(aStream));

        // The document has one page.
        std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
        CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());
        auto pAnnots
            = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
        CPPUNIT_ASSERT(pAnnots);

        // There should be one annotation
        CPPUNIT_ASSERT_EQUAL(size_t(1), pAnnots->GetElements().size());
        auto pAnnotReference
            = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
        CPPUNIT_ASSERT(pAnnotReference);
        vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
        CPPUNIT_ASSERT(pAnnot);
        // We're expecting something like /Type /Annot /A << /Type /Action /S /URI /URI (path)
        CPPUNIT_ASSERT_EQUAL(
            "Annot"_ostr,
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());
        CPPUNIT_ASSERT_EQUAL(
            "Link"_ostr,
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr))->GetValue());
        auto pAction = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
        CPPUNIT_ASSERT(pAction);
        auto pURIElem = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
            pAction->LookupElement("URI"_ostr));
        CPPUNIT_ASSERT(pURIElem);
        // Check it matches
        CPPUNIT_ASSERT_EQUAL(URIs[i].out, pURIElem->GetValue());
        // tdf#148934 check a11y
        CPPUNIT_ASSERT_EQUAL(u"Test pdf"_ustr, ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                   *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                       pAnnot->Lookup("Contents"_ostr))));
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testPdfImageAnnots)
{
    // Given a document with a PDF image that has 2 comments (popup, text) and a hyperlink:
    aMediaDescriptor[u"FilterName"_ustr] <<= u"draw_pdf_Export"_ustr;

    // When saving to PDF:
    saveAsPDF(u"pdf-image-annots.odg");

    // Then make sure only the hyperlink is kept, since Draw itself has its own comments:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 3
    // i.e. not only the hyperlink but also the 2 comments were exported, leading to duplication.
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getAnnotationCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testPdfImageEncryption)
{
    // Given an empty document, with an inserted PDF image:
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphicObject(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    OUString aURL = createFileURL(u"rectangles.pdf");
    xGraphicObject->setPropertyValue(u"GraphicURL"_ustr, uno::Any(aURL));
    uno::Reference<drawing::XShape> xShape(xGraphicObject, uno::UNO_QUERY);
    xShape->setSize(awt::Size(1000, 1000));
    uno::Reference<text::XTextContent> xTextContent(xGraphicObject, uno::UNO_QUERY);
    xText->insertTextContent(xCursor->getStart(), xTextContent, /*bAbsorb=*/false);

    // When saving as encrypted PDF:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    uno::Sequence<beans::PropertyValue> aFilterData = {
        comphelper::makePropertyValue(u"EncryptFile"_ustr, true),
        comphelper::makePropertyValue(u"DocumentOpenPassword"_ustr, u"secret"_ustr),
    };
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Then make sure that the image is not lost:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport("secret"_ostr);
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Form, pPageObject->getType());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0
    // i.e. instead of the white background and the actual form child, the image was lost due to
    // missing encryption.
    CPPUNIT_ASSERT_EQUAL(2, pPageObject->getFormObjectCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testBitmapScaledown)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // Given a document with an upscaled and rotated barcode bitmap in it:
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // When saving as PDF:
    saveAsPDF(u"bitmap-scaledown.odt");

    // Then verify that the bitmap is not downscaled:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    int nPageObjectCount = pPdfPage->getObjectCount();
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Image)
            continue;

        std::unique_ptr<vcl::pdf::PDFiumBitmap> pBitmap = pPageObject->getImageBitmap();
        CPPUNIT_ASSERT(pBitmap);
        // In-file sizes: good is 2631x380, bad is 1565x14.
        int nWidth = pBitmap->getWidth();
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 2616
        // - Actual  : 1565
        // i.e. the bitmap in the pdf result was small enough to be blurry.
        CPPUNIT_ASSERT_EQUAL(2616, nWidth);
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf139627)
{
#if HAVE_MORE_FONTS
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    saveAsPDF(u"justified-arabic-kashida.odt");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // 7 objects, 3 text, others are path
    int nPageObjectCount = pPdfPage->getObjectCount();
    CPPUNIT_ASSERT_EQUAL(7, nPageObjectCount);

    // 3 text objects
    OUString sText[3];

    /* With "Noto Sans Arabic" font, these are the X ranges on Linux:
        0: ( 61.75 - 415.94)
        1: (479.70 - 422.40)
        2: (209.40 - 453.2)
    */
    basegfx::B2DRectangle aRect[3];

    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject;

    int nTextObjectCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        pPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT_MESSAGE("no object", pPageObject != nullptr);
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            sText[nTextObjectCount] = pPageObject->getText(pTextPage);
            aRect[nTextObjectCount] = pPageObject->getBounds();
            ++nTextObjectCount;
        }
    }
    CPPUNIT_ASSERT_EQUAL(3, nTextObjectCount);

    // Text: جِـرم (which means "mass" in Persian)
    // Rendered as (left to right): "reh + mim" - "kasreh" - "jeh + tatweel"
    int rehmim = 0, kasreh = 1, jehtatweel = 2;

    CPPUNIT_ASSERT_EQUAL(u"رم"_ustr, sText[rehmim].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, sText[kasreh].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, sText[jehtatweel].trim());

    // "Kasreh" should be within "jeh" character
    CPPUNIT_ASSERT_GREATER(aRect[jehtatweel].getMinX(), aRect[kasreh].getMinX());
    CPPUNIT_ASSERT_LESS(aRect[jehtatweel].getMaxX(), aRect[kasreh].getMaxX());

    // "Tatweel" should cover "jeh" and "reh"+"mim" to avoid gap
    // Checking right gap
    //CPPUNIT_ASSERT_GREATER(aRect[jehtatweel].getMinX(), aRect[tatweel].getMaxX());
    // Checking left gap
    // Kashida fails to reach to rehmim before the series of patches starting
    // with 3901e029bd39575f700e69a73818565d62226a23. The visible symptom is
    // a gap in the left of Kashida.
    CPPUNIT_ASSERT_LESS(aRect[rehmim].getMaxX(), aRect[jehtatweel].getMinX());

    // Overlappings of Kashida and surrounding characters is ~9% of the width
    // of the "jeh" character, while using "Noto Arabic Sans" font in this
    // specific example.
    // We set the hard limit of 10% here.
    CPPUNIT_ASSERT_LESS(0.1, fabs(aRect[rehmim].getMaxX() - aRect[jehtatweel].getMinX())
                                 / aRect[jehtatweel].getWidth());
#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testRexportRefToKids)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        osl_setEnvironment(u"LO_IMPORT_USE_PDFIUM"_ustr.pData, u"1"_ustr.pData);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            osl_clearEnvironment(u"LO_IMPORT_USE_PDFIUM"_ustr.pData);
    });

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    load(u"ref-to-kids.pdf", aDocument);

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(5), aPages.size());

    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);

    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);

    // Without the fix LookupObject for all /Im's will fail.
    for (auto const& rPair : pXObjects->GetItems())
    {
        if (rPair.first.startsWith("Im"))
            CPPUNIT_ASSERT(pXObjects->LookupObject(rPair.first));
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testRexportFilterSingletonArray)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        osl_setEnvironment(u"LO_IMPORT_USE_PDFIUM"_ustr.pData, u"1"_ustr.pData);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            osl_clearEnvironment(u"LO_IMPORT_USE_PDFIUM"_ustr.pData);
    });

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    // Loading fails with tagged PDF enabled
    load(u"ref-to-kids.pdf", aDocument, false);

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(5), aPages.size());

    // Directly go to the inner XObject Im5 that contains the rectangle drawings.
    auto pInnerIm = aDocument.LookupObject(5);
    CPPUNIT_ASSERT(pInnerIm);

    auto pFilter = dynamic_cast<vcl::filter::PDFNameElement*>(pInnerIm->Lookup("Filter"_ostr));
    CPPUNIT_ASSERT(pFilter);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Filter must be FlateDecode", "FlateDecode"_ostr,
                                 pFilter->GetValue());

    vcl::filter::PDFStreamElement* pStream = pInnerIm->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    // Without the fix, the stream is doubly compressed,
    // hence one decompression will not yield the "re" expressions.
    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* pEnd = pStart + aUncompressed.GetSize();
    OString aImage = "100 0 30 50 re B*\n70 67 50 30 re B*\n"_ostr;
    auto it = std::search(pStart, pEnd, aImage.getStr(), aImage.getStr() + aImage.getLength());
    CPPUNIT_ASSERT(it != pEnd);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testRexportMediaBoxOrigin)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        osl_setEnvironment(u"LO_IMPORT_USE_PDFIUM"_ustr.pData, u"1"_ustr.pData);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            osl_clearEnvironment(u"LO_IMPORT_USE_PDFIUM"_ustr.pData);
    });

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    load(u"ref-to-kids.pdf", aDocument);

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(5), aPages.size());

    // Directly go to the inner XObject Im12 that contains the rectangle drawings in page 2.
    auto pInnerIm = aDocument.LookupObject(12);
    CPPUNIT_ASSERT(pInnerIm);

    constexpr sal_Int32 aOrigin[2] = { -800, -600 };
    sal_Int32 aSize[2] = { 0, 0 };

    auto pBBox = dynamic_cast<vcl::filter::PDFArrayElement*>(pInnerIm->Lookup("BBox"_ostr));
    CPPUNIT_ASSERT(pBBox);
    const auto& rElements2 = pBBox->GetElements();
    CPPUNIT_ASSERT_EQUAL(size_t(4), rElements2.size());
    for (sal_Int32 nIdx = 0; nIdx < 4; ++nIdx)
    {
        const auto* pNumElement = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements2[nIdx]);
        CPPUNIT_ASSERT(pNumElement);
        if (nIdx < 2)
            CPPUNIT_ASSERT_EQUAL(aOrigin[nIdx], static_cast<sal_Int32>(pNumElement->GetValue()));
        else
            aSize[nIdx - 2] = static_cast<sal_Int32>(pNumElement->GetValue()) - aOrigin[nIdx - 2];
    }

    auto pMatrix = dynamic_cast<vcl::filter::PDFArrayElement*>(pInnerIm->Lookup("Matrix"_ostr));
    CPPUNIT_ASSERT(pMatrix);
    const auto& rElements = pMatrix->GetElements();
    CPPUNIT_ASSERT_EQUAL(size_t(6), rElements.size());
    sal_Int32 aMatTranslate[6]
        = { // Rotation by $\theta$ $cos(\theta), sin(\theta), -sin(\theta), cos(\theta)$
            0, -1, 1, 0,
            // Translate x,y
            -aOrigin[1] - aSize[1] / vcl::PDF_INSERT_MAGIC_SCALE_FACTOR / 2
                + aSize[0] / vcl::PDF_INSERT_MAGIC_SCALE_FACTOR / 2,
            aOrigin[0] + aSize[0] / vcl::PDF_INSERT_MAGIC_SCALE_FACTOR / 2
                + aSize[1] / vcl::PDF_INSERT_MAGIC_SCALE_FACTOR / 2
          };

    for (sal_Int32 nIdx = 0; nIdx < 6; ++nIdx)
    {
        const auto* pNumElement = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[nIdx]);
        CPPUNIT_ASSERT(pNumElement);
        CPPUNIT_ASSERT_EQUAL(aMatTranslate[nIdx], static_cast<sal_Int32>(pNumElement->GetValue()));
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testRexportResourceItemReference)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        osl_setEnvironment(u"LO_IMPORT_USE_PDFIUM"_ustr.pData, u"1"_ustr.pData);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            osl_clearEnvironment(u"LO_IMPORT_USE_PDFIUM"_ustr.pData);
    });

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    load(u"ref-to-kids.pdf", aDocument);

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(5), aPages.size());

    // Directly go to the inner XObject Im12 that has reference to Font in page 2.
    auto pInnerIm = aDocument.LookupObject(12);
    CPPUNIT_ASSERT(pInnerIm);

    auto pResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pInnerIm->Lookup("Resources"_ostr));
    CPPUNIT_ASSERT(pResources);
    auto pFontsReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pResources->LookupElement("Font"_ostr));
    CPPUNIT_ASSERT(pFontsReference);

    auto pFontsObject = pFontsReference->LookupObject();
    CPPUNIT_ASSERT(pFontsObject);

    auto pFontDict
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pFontsObject->Lookup("FF132"_ostr));
    CPPUNIT_ASSERT(pFontDict);

    auto pFontDescriptor = pFontDict->LookupObject("FontDescriptor"_ostr);
    CPPUNIT_ASSERT(pFontDescriptor);

    auto pFontWidths = pFontDict->LookupObject("Widths"_ostr);
    CPPUNIT_ASSERT(pFontWidths);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf152246)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    saveAsPDF(u"content-control-rtl.docx");

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    // Position array
    constexpr double aPos[5][4] = { { 56.699, 707.701, 131.401, 721.499 },
                                    { 198.499, 707.701, 273.201, 721.499 },
                                    { 303.349, 680.101, 378.051, 693.899 },
                                    { 480.599, 680.101, 555.301, 693.899 },
                                    { 56.699, 652.501, 131.401, 666.299 } };

    // Get page annotations.
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(size_t(5), pAnnots->GetElements().size());
    for (sal_Int32 i = 0; i < 5; ++i)
    {
        auto pAnnotReference
            = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[i]);
        CPPUNIT_ASSERT(pAnnotReference);
        vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
        CPPUNIT_ASSERT(pAnnot);
        CPPUNIT_ASSERT_EQUAL(
            "Annot"_ostr,
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());
        CPPUNIT_ASSERT_EQUAL(
            "Widget"_ostr,
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr))->GetValue());

        auto pRect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
        CPPUNIT_ASSERT(pRect);
        const auto& rElements = pRect->GetElements();
        CPPUNIT_ASSERT_EQUAL(size_t(4), rElements.size());
        for (sal_Int32 nIdx = 0; nIdx < 4; ++nIdx)
        {
            const auto* pNumElement = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[nIdx]);
            CPPUNIT_ASSERT(pNumElement);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(aPos[i][nIdx], pNumElement->GetValue(), 1e-6);
        }
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf155161)
{
// TODO: We seem to get a fallback font on Windows
#ifndef _WIN32
    vcl::filter::PDFDocument aDocument;
    load(u"tdf155161.odt", aDocument);

    // Check that all fonts in the document are Type 3 fonts
    int nFonts = 0;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "Font")
        {
            auto pSubtype
                = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Subtype"_ostr));
            CPPUNIT_ASSERT(pSubtype);
            CPPUNIT_ASSERT_EQUAL("Type3"_ostr, pSubtype->GetValue());

            auto pName = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Name"_ostr));
            CPPUNIT_ASSERT(pName);
            CPPUNIT_ASSERT_EQUAL("Cantarell-Regular"_ostr, pName->GetValue());

            nFonts++;
        }
    }

#ifdef MACOSX
    // There must be two fonts
    CPPUNIT_ASSERT_EQUAL(2, nFonts);
#else
    // But it seems that embedded variable fonts don’t register all supported
    // styles on Linux, so the bold and regular text use the same regular font.
    CPPUNIT_ASSERT(nFonts);
#endif
#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf48707_1)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    saveAsPDF(u"tdf48707-1.fodt");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    auto pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    int nPageObjectCount = pPage->getObjectCount();

    CPPUNIT_ASSERT_EQUAL(6, nPageObjectCount);

    auto pTextPage = pPage->getTextPage();

    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(i);
        // The text and path objects (underline and overline) should all be red.
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, pPageObject->getFillColor());
        else
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, pPageObject->getStrokeColor());
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf48707_2)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    saveAsPDF(u"tdf48707-2.fodt");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    auto pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    int nPageObjectCount = pPage->getObjectCount();

    CPPUNIT_ASSERT_EQUAL(13, nPageObjectCount);

    auto pTextPage = pPage->getTextPage();

    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Path)
            continue;

        // The table-like paths should be red, underline and overline should be black.
        if (i >= 8)
            CPPUNIT_ASSERT_EQUAL(COL_BLACK, pPageObject->getStrokeColor());
        else
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, pPageObject->getStrokeColor());
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf156528)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    saveAsPDF(u"wide_page1.fodt");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has two pages
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());

    // 1st page (5100 mm width x 210 mm high, UserUnit = 2)
    auto pPdfPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(5100.0 / 2, o3tl::Length::mm, o3tl::Length::pt),
                                 pPdfPage->getWidth(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(210.0 / 2, o3tl::Length::mm, o3tl::Length::pt),
                                 pPdfPage->getHeight(), 1);

    // 1 object (rectangle 5060 mm width x 170 mm high, UserUnit = 2)
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    auto pRect = pPdfPage->getObject(0);
    CPPUNIT_ASSERT(pRect);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Path, pRect->getType());
    auto bounds = pRect->getBounds();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(5060.0 / 2, o3tl::Length::mm, o3tl::Length::pt),
                                 bounds.getWidth(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(170.0 / 2, o3tl::Length::mm, o3tl::Length::pt),
                                 bounds.getHeight(), 1);

    // 2nd page (210 mm width x 297 mm high, UserUnit = 1)
    pPdfPage = pPdfDocument->openPage(1);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(210.0, o3tl::Length::mm, o3tl::Length::pt),
                                 pPdfPage->getWidth(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(297.0, o3tl::Length::mm, o3tl::Length::pt),
                                 pPdfPage->getHeight(), 1);

    // 1 object (rectangle 170 mm width x 257 mm high, UserUnit = 1)
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    pRect = pPdfPage->getObject(0);
    CPPUNIT_ASSERT(pRect);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Path, pRect->getType());
    bounds = pRect->getBounds();
    // Without the fix, this would fail with
    // - Expected: 481.889763779528
    // - Actual  : 241.925001144409
    // - Delta   : 1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(170.0, o3tl::Length::mm, o3tl::Length::pt),
                                 bounds.getWidth(), 1);
    //
    // - Expected: 728.503937007874
    // - Actual  : 365.25
    // - Delta   : 1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(257.0, o3tl::Length::mm, o3tl::Length::pt),
                                 bounds.getHeight(), 1);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf113866)
{
    loadFromFile(u"tdf113866.odt");

    // Set -- Printer Settings->Options->Print text in Black -- to true
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    IDocumentDeviceAccess& rDocAccess = pDoc->getIDocumentDeviceAccess();
    SwPrintData aDocPrintData = rDocAccess.getPrintData();
    aDocPrintData.SetPrintBlackFont(true);
    rDocAccess.setPrintData(aDocPrintData);

    // Export to pdf
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // Non-NULL pPdfDocument means pdfium is available.
    if (pPdfDocument != nullptr)
    {
        std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(0);
        CPPUNIT_ASSERT(pPdfPage);

        int nPageObjectCount = pPdfPage->getObjectCount();
        for (int i = 0; i < nPageObjectCount; ++i)
        {
            std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);

            if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
                // Without the bug fix in place the test will fail with
                // - Expected: rgba[008000ff]
                // - Actual  : rgba[000000ff]
                // With the bug fixed, the green text in the test doc will stay green,
                // when exported to pdf, while Print Text in Black is true
                CPPUNIT_ASSERT_EQUAL(COL_GREEN, pPageObject->getFillColor());
        }
    }
}

// Form controls coordinates scrambled when exporting to pdf with unchecked form creation in Writer
CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf159817)
{
    aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_pdf_Export"_ustr;

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "ExportFormFields", uno::Any(false) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    saveAsPDF(u"tdf159817.fodt");

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    // So I extracted these values by using SAL_WARN(... << ...getMinimum()), but it appears
    // that the C++ stream operators do not output double values with sufficient resolution for me
    // to recreate those values in code, sigh, so resort to rounding things.
    auto roundPoint = [&pPdfPage](int i) {
        auto p = pPdfPage->getObject(i)->getBounds().getMinimum();
        return basegfx::B2DPoint(std::floor(p.getX() * 10) / 10.0,
                                 std::floor(p.getY() * 10) / 10.0);
    };
    // before the fix these co-ordinates would have been way further down the page
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(8.6, 677.3), roundPoint(13));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(9.3, 677.9), roundPoint(14));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(9.8, 678.5), roundPoint(15));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(85.0, 677.3), roundPoint(16));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(85.6, 677.9), roundPoint(17));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(170.1, 677.3), roundPoint(18));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(170.6, 677.9), roundPoint(19));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(0.0, 654.0), roundPoint(20));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(0.6, 654.6), roundPoint(21));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.3, 655.5), roundPoint(22));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.3, 655.5), roundPoint(23));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.2, 655.5), roundPoint(24));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.2, 655.5), roundPoint(25));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.4, 655.5), roundPoint(26));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.4, 655.5), roundPoint(27));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.1, 655.5), roundPoint(28));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.1, 655.5), roundPoint(29));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.5, 655.5), roundPoint(30));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.5, 655.5), roundPoint(31));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.0, 655.5), roundPoint(32));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1.0, 655.5), roundPoint(33));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(28.3, 641.4), roundPoint(34));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(28.3, 623.7), roundPoint(35));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(28.3, 623.8), roundPoint(36));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(138.6, 623.7), roundPoint(37));
}

// Tests that kerning is correctly applied across color changes
CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf61444)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf61444.odt");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Get the first page
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex*/ 0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    // 4 text objects should be present
    int nPageObjectCount = pPdfPage->getObjectCount();
    CPPUNIT_ASSERT_EQUAL(4, nPageObjectCount);

    OUString sText[4];
    basegfx::B2DRectangle aRect[4];

    int nTextObjectCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        auto pPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT_MESSAGE("no object", pPageObject != nullptr);
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            sText[nTextObjectCount] = pPageObject->getText(pTextPage);
            aRect[nTextObjectCount] = pPageObject->getBounds();
            ++nTextObjectCount;
        }
    }

    CPPUNIT_ASSERT_EQUAL(4, nTextObjectCount);

    CPPUNIT_ASSERT_EQUAL(u"Wait"_ustr, sText[0].trim());
    CPPUNIT_ASSERT_EQUAL(u"W"_ustr, sText[1].trim());
    CPPUNIT_ASSERT_EQUAL(u"ai"_ustr, sText[2].trim());
    CPPUNIT_ASSERT_EQUAL(u"t"_ustr, sText[3].trim());

    // Both lines should have the same kerning, so should end at approximately the same X coordinate
    auto solid_extent = aRect[0].getMaxX();
    auto color_extent = aRect[3].getMaxX();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(solid_extent, color_extent, /*delta*/ 0.15);
}

// tdf#124116 - Tests that track-changes inside a grapheme cluster does not break positioning
CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf124116TrackUntrack)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf124116-hebrew-track-untrack.odt");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Get the first page
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex*/ 0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    int nPageObjectCount = pPdfPage->getObjectCount();
    CPPUNIT_ASSERT_EQUAL(15, nPageObjectCount);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;

    int nTextObjectCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        auto pPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT_MESSAGE("no object", pPageObject != nullptr);
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            aText.push_back(pPageObject->getText(pTextPage));
            aRect.push_back(pPageObject->getBounds());
            ++nTextObjectCount;
        }
    }

    // The underlying document has 4 lines:
    // - שמחַ plain
    // - שמחַ tracked
    // - שמחַ with patah tracked
    // - שמחַ with everything except patah tracked
    // ---
    // However, due to the way text items are inserted for Hebrew, there will be 10:
    // - het with an improperly spaced patah, then שמ for the first 2 lines
    // - as above, followed by a blank for the next 2 representing the actual diacritic
    // ---
    // This test will likely need to be rewritten if tdf#158329 is fixed.
    CPPUNIT_ASSERT_EQUAL(10, nTextObjectCount);

    // All that matters for this test is that the patah is positioned well under the het
    auto het_x0 = aRect.at(4).getMinX();
    auto patah_x0 = aRect.at(6).getMinX();
    CPPUNIT_ASSERT_GREATER(10.0, patah_x0 - het_x0);

    auto het_x1 = aRect.at(7).getMinX();
    auto patah_x1 = aRect.at(9).getMinX();
    CPPUNIT_ASSERT_GREATER(10.0, patah_x1 - het_x1);
}

// tdf#134226 - Tests that shaping is not broken by invisible spans
CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf134226)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf134226-shadda-in-hidden-span.fodt");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Get the first page
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex*/ 0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    int nPageObjectCount = pPdfPage->getObjectCount();
    CPPUNIT_ASSERT_EQUAL(8, nPageObjectCount);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;

    int nTextObjectCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        auto pPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT_MESSAGE("no object", pPageObject != nullptr);
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            aText.push_back(pPageObject->getText(pTextPage));
            aRect.push_back(pPageObject->getBounds());
            ++nTextObjectCount;
        }
    }

    CPPUNIT_ASSERT_EQUAL(8, nTextObjectCount);

    CPPUNIT_ASSERT_EQUAL(u"ة"_ustr, aText[0].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[1].trim());
    CPPUNIT_ASSERT_EQUAL(u"\u0651ق"_ustr, aText[2].trim());
    CPPUNIT_ASSERT_EQUAL(u"ش"_ustr, aText[3].trim());
    CPPUNIT_ASSERT_EQUAL(u"\u0651ق"_ustr, aText[4].trim());
    CPPUNIT_ASSERT_EQUAL(u"ش"_ustr, aText[5].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[6].trim());
    CPPUNIT_ASSERT_EQUAL(u"ة"_ustr, aText[7].trim());

    // Verify that the corresponding text segments are positioned roughly equally
    auto fnEqualPos
        = [](const basegfx::B2DRectangle& stExpected, const basegfx::B2DRectangle& stFound) {
              CPPUNIT_ASSERT_DOUBLES_EQUAL(stExpected.getMinX(), stFound.getMinX(), /*delta*/ 0.15);
              CPPUNIT_ASSERT_DOUBLES_EQUAL(stExpected.getMaxX(), stFound.getMaxX(), /*delta*/ 0.15);
          };

    fnEqualPos(aRect[0], aRect[7]);
    fnEqualPos(aRect[1], aRect[6]);
    fnEqualPos(aRect[2], aRect[4]);
    fnEqualPos(aRect[3], aRect[5]);
}

// tdf#71956 - Tests that glyphs can be individually styled
CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf71956)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf71956-styled-diacritics.fodt");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Get the first page
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex*/ 0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pTextPage);

    int nPageObjectCount = pPdfPage->getObjectCount();
    CPPUNIT_ASSERT_EQUAL(12, nPageObjectCount);

    std::vector<OUString> aText;
    std::vector<basegfx::B2DRectangle> aRect;

    int nTextObjectCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        auto pPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT_MESSAGE("no object", pPageObject != nullptr);
        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            aText.push_back(pPageObject->getText(pTextPage));
            aRect.push_back(pPageObject->getBounds());
            ++nTextObjectCount;
        }
    }

    CPPUNIT_ASSERT_EQUAL(12, nTextObjectCount);

    CPPUNIT_ASSERT_EQUAL(u"ه"_ustr, aText[0].trim());
    CPPUNIT_ASSERT_EQUAL(u"\u064e\u0651\u0670ل"_ustr, aText[1].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[2].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[3].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[4].trim());
    CPPUNIT_ASSERT_EQUAL(u"ل"_ustr, aText[5].trim());
    CPPUNIT_ASSERT_EQUAL(u"ل"_ustr, aText[6].trim());
    CPPUNIT_ASSERT_EQUAL(u"\u064e\u0651\u0670ل"_ustr, aText[7].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[8].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[9].trim());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aText[10].trim());
    CPPUNIT_ASSERT_EQUAL(u"ه"_ustr, aText[11].trim());

    // Verify that the corresponding text segments are positioned roughly equally
    auto fnEqualPos
        = [](const basegfx::B2DRectangle& stExpected, const basegfx::B2DRectangle& stFound) {
              CPPUNIT_ASSERT_DOUBLES_EQUAL(stExpected.getMinX(), stFound.getMinX(), /*delta*/ 0.15);
              CPPUNIT_ASSERT_DOUBLES_EQUAL(stExpected.getMaxX(), stFound.getMaxX(), /*delta*/ 0.15);
          };

    fnEqualPos(aRect[0], aRect[11]);
    fnEqualPos(aRect[1], aRect[10]);
    fnEqualPos(aRect[2], aRect[8]);
    fnEqualPos(aRect[3], aRect[9]);
    fnEqualPos(aRect[4], aRect[7]);
    fnEqualPos(aRect[5], aRect[6]);
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
