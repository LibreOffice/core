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

#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <comphelper/scopeguard.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <rtl/character.hxx>
#include <tools/stream.hxx>
#include <tools/zcodec.hxx>
#include <vcl/graphicfilter.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <unotools/streamwrap.hxx>
#include <o3tl/string_view.hxx>

#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/pdf/PDFObjectType.hxx>
#include <vcl/pdf/PDFPageObjectType.hxx>
#include <comphelper/propertyvalue.hxx>
#include <libxml/xpathInternals.h>

#include <test/unoapi_test.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests the PDF export filter.
class PdfExportTest2 : public UnoApiTest
{
public:
    PdfExportTest2()
        : UnoApiTest(u"/vcl/qa/cppunit/pdfexport/data/"_ustr)
    {
    }

    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    xmlDocUniquePtr parseMetadata();
};

void PdfExportTest2::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("x"), BAD_CAST("adobe:ns:meta/"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("rdf"),
                       BAD_CAST("http://www.w3.org/1999/02/22-rdf-syntax-ns#"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dc"), BAD_CAST("http://purl.org/dc/elements/1.1/"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("pdfuaid"),
                       BAD_CAST("http://www.aiim.org/pdfua/ns/id/"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("pdfaid"),
                       BAD_CAST("http://www.aiim.org/pdfa/ns/id/"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("pdfaExtension"),
                       BAD_CAST("http://www.aiim.org/pdfa/ns/extension/"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("pdfaSchema"),
                       BAD_CAST("http://www.aiim.org/pdfa/ns/schema#"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("pdfaProperty"),
                       BAD_CAST("http://www.aiim.org/pdfa/ns/property#"));
}

xmlDocUniquePtr PdfExportTest2::parseMetadata()
{
    vcl::filter::PDFDocument aDocument;
    // Parse the export result.
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
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&rStream);
    CPPUNIT_ASSERT(pXmlDoc);
    return pXmlDoc;
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf160705)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf160705.odt");
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    int nTable(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;

        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
            if (pS && pS->GetValue() == "Table")
            {
                auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids);

                // In the first table, the caption element is the first child element
                // In the second table, the caption element is the last child element
                // In the third table, the caption element is the first child element
                int nId = 0;
                if (nTable == 1) // second table
                    nId = pKids->GetElements().size() - 1;
                else if (nTable == 2) // third table
                    nId = 0;

                auto pTableKids = pKids->GetElements();
                auto pRefKid = dynamic_cast<vcl::filter::PDFReferenceElement*>(pTableKids[nId]);
                CPPUNIT_ASSERT(pRefKid);
                auto pObj = pRefKid->LookupObject();
                CPPUNIT_ASSERT(pObj);
                auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObj->Lookup("Type"_ostr));
                CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1->GetValue());
                auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObj->Lookup("S"_ostr));
                CPPUNIT_ASSERT_EQUAL("TableCaption"_ostr, pS1->GetValue());

                auto pKids2 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObj->Lookup("K"_ostr));
                CPPUNIT_ASSERT(pKids2);

                // The captions of Table1, Table2 and Table3 also have two standard elements
                CPPUNIT_ASSERT_EQUAL(size_t(2), pKids2->GetElements().size());

                ++nTable;
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf159895)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "PDFUACompliance", uno::Any(true) },
        { "ExportFormFields", uno::Any(true) },
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf159895.odt");
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObj = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObj)
            continue;

        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObj->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "XObject")
        {
            auto pFilter = dynamic_cast<vcl::filter::PDFNameElement*>(pObj->Lookup("Filter"_ostr));
            CPPUNIT_ASSERT(pFilter);

            vcl::filter::PDFStreamElement* pStream = pObj->GetStream();
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

            OStringBuffer sText;
            while (true)
            {
                auto const pLine = ::std::find(pStart, pEnd, '\n');
                if (pLine == pEnd)
                    break;

                std::string_view const line(pStart, pLine - pStart);
                pStart = pLine + 1;
                if (!line.empty() && line[0] != '%')
                {
                    sText.append(line + "\n"_ostr);
                }
            }
            CPPUNIT_ASSERT_EQUAL("/Tx BMC\nEMC\n"_ostr, sText.toString());
        }
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf124272)
{
    // Import the bugdoc and export as PDF.
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf124272.odt");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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

    OString aBitmap("Q q 299.9 782.19 m\n"
                    "55.2 435.89 l 299.9 435.89 l 299.9 782.19 l h"_ostr);

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* pEnd = pStart + aUncompressed.GetSize();
    auto it = std::search(pStart, pEnd, aBitmap.getStr(), aBitmap.getStr() + aBitmap.getLength());
    CPPUNIT_ASSERT(it != pEnd);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf121615)
{
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf121615.odt");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
    Bitmap aBitmap = aGraphic.GetBitmap();
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(300), aBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N8_BPP, aBitmap.getPixelFormat());
    // tdf#121615 was caused by broken handling of data width with 8bit color,
    // so the test image has some black in the bottom-right corner, check it's there
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aBitmap.GetPixelColor(0, 0));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aBitmap.GetPixelColor(0, 299));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aBitmap.GetPixelColor(199, 0));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aBitmap.GetPixelColor(199, 299));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf141171)
{
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf141171.odt");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    int nImages(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Subtype"_ostr));
        if (pType && pType->GetValue() == "Image")
        {
            auto pBitsPerComponent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                pObject->Lookup("BitsPerComponent"_ostr));
            CPPUNIT_ASSERT(pBitsPerComponent);
            CPPUNIT_ASSERT_EQUAL(8.0, pBitsPerComponent->GetValue());

            auto pWidth
                = dynamic_cast<vcl::filter::PDFNumberElement*>(pObject->Lookup("Width"_ostr));
            CPPUNIT_ASSERT(pWidth);
            CPPUNIT_ASSERT_EQUAL(878.0, pWidth->GetValue());

            auto pHeight
                = dynamic_cast<vcl::filter::PDFNumberElement*>(pObject->Lookup("Height"_ostr));
            CPPUNIT_ASSERT(pHeight);
            CPPUNIT_ASSERT_EQUAL(127.0, pHeight->GetValue());

            auto pColorSpace
                = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("ColorSpace"_ostr));
            CPPUNIT_ASSERT(pColorSpace);
            CPPUNIT_ASSERT_EQUAL("DeviceGray"_ostr, pColorSpace->GetValue());

            // Check the first image has a SMask
            if (nImages == 0)
                CPPUNIT_ASSERT(pObject->Lookup("SMask"_ostr));
            ++nImages;
        }
    }
    // Without the fix in place, there would have been only one image
    CPPUNIT_ASSERT_EQUAL(2, nImages);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf161346)
{
    // this crashed
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"fdo47811-1_Word2013.docx");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aPages.size());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf129085)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        // if the Quality is too high (like 90) then the image will be a DIB,
        // not a JPEG! 80 works currently but set it lower to be sure...
        { "Quality", uno::Any(sal_Int32(50)) },
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf129085.docx");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
    Bitmap aBitmap = aGraphic.GetBitmap();
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
    save(TestFilter::PDF_WRITER);

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Ensure there is a link on the first page (in the ToC).
    // Without the accompanying fix in place, this test would have failed, as the page contained no
    // links.
    CPPUNIT_ASSERT(pPdfPage->hasLinks());
}

// Inserts three cross-reference fields to a one-digit bookmark target, separated only by
// ", ", the way consecutive caption numbers are.
void lcl_insertAdjacentReferenceFields(const uno::Reference<lang::XComponent>& xComponent,
                                       float fCharHeight)
{
    uno::Reference<text::XTextDocument> xTextDocument(xComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<lang::XMultiServiceFactory> xFactory(xComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
    xCursorProps->setPropertyValue(u"CharHeight"_ustr, uno::Any(fCharHeight));

    xText->insertString(xCursor, u"1"_ustr, /*bAbsorb=*/false);
    xCursor->goLeft(1, /*bExpand=*/true);
    uno::Reference<text::XTextContent> xBookmark(
        xFactory->createInstance(u"com.sun.star.text.Bookmark"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xBookmarkName(xBookmark, uno::UNO_QUERY);
    xBookmarkName->setName(u"Target"_ustr);
    xText->insertTextContent(xCursor, xBookmark, /*bAbsorb=*/true);

    xCursor->gotoEnd(/*bExpand=*/false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK,
                                  /*bAbsorb=*/false);

    auto insertReferenceField = [&]() {
        xCursor->gotoEnd(/*bExpand=*/false);
        uno::Reference<text::XTextField> xField(
            xFactory->createInstance(u"com.sun.star.text.TextField.GetReference"_ustr),
            uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xFieldProps(xField, uno::UNO_QUERY);
        xFieldProps->setPropertyValue(u"ReferenceFieldSource"_ustr,
                                      uno::Any(sal_Int16(text::ReferenceFieldSource::BOOKMARK)));
        xFieldProps->setPropertyValue(u"ReferenceFieldPart"_ustr,
                                      uno::Any(sal_Int16(text::ReferenceFieldPart::TEXT)));
        xFieldProps->setPropertyValue(u"SourceName"_ustr, uno::Any(u"Target"_ustr));
        xField->attach(xCursor);
    };

    insertReferenceField();
    xCursor->gotoEnd(/*bExpand=*/false);
    xText->insertString(xCursor, u", "_ustr, /*bAbsorb=*/false);
    insertReferenceField();
    xCursor->gotoEnd(/*bExpand=*/false);
    xText->insertString(xCursor, u", "_ustr, /*bAbsorb=*/false);
    insertReferenceField();
}

int lcl_countAnnotations(const std::unique_ptr<vcl::pdf::PDFiumDocument>& pPdfDocument)
{
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    return pPdfPage->getAnnotationCount();
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testAdjacentReferenceFieldLinks_12)
{
    // Every one of three adjacent one-digit cross-reference fields gets its own clickable
    // area in the exported PDF, at every font size.
    loadFromURL(u"private:factory/swriter"_ustr);
    lcl_insertAdjacentReferenceFields(mxComponent, 12.0f);

    save(TestFilter::PDF_WRITER);

    OString aMessage("font size " + OString::number(12.0f));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        aMessage.getStr(), 3, lcl_countAnnotations(parsePDFExport(vcl::pdf::PDFiumLibrary::get())));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testAdjacentReferenceFieldLinks_10)
{
    // Every one of three adjacent one-digit cross-reference fields gets its own clickable
    // area in the exported PDF, at every font size.
    loadFromURL(u"private:factory/swriter"_ustr);
    lcl_insertAdjacentReferenceFields(mxComponent, 10.0f);

    save(TestFilter::PDF_WRITER);

    OString aMessage("font size " + OString::number(10.0f));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        aMessage.getStr(), 3, lcl_countAnnotations(parsePDFExport(vcl::pdf::PDFiumLibrary::get())));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testAdjacentReferenceFieldLinks_8)
{
    // Every one of three adjacent one-digit cross-reference fields gets its own clickable
    // area in the exported PDF, at every font size.
    loadFromURL(u"private:factory/swriter"_ustr);
    lcl_insertAdjacentReferenceFields(mxComponent, 8.0f);

    save(TestFilter::PDF_WRITER);

    OString aMessage("font size " + OString::number(8.0f));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        aMessage.getStr(), 3, lcl_countAnnotations(parsePDFExport(vcl::pdf::PDFiumLibrary::get())));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testAdjacentReferenceFieldLinksWideCaret)
{
    // The width of the text caret is a desktop setting, and can be set wider than a small
    // glyph. The clickable areas stay independent of it.
    AllSettings aOldSettings(Application::GetSettings());
    comphelper::ScopeGuard aSettingsGuard(
        [&aOldSettings] { Application::SetSettings(aOldSettings); });
    AllSettings aSettings(aOldSettings);
    StyleSettings aStyleSettings(aSettings.GetStyleSettings());
    // 6 pixels is 90 twips at 96 dots per inch, wider than an 8 point digit.
    aStyleSettings.SetCursorSize(6);
    aSettings.SetStyleSettings(aStyleSettings);
    Application::SetSettings(aSettings);

    loadFromURL(u"private:factory/swriter"_ustr);
    lcl_insertAdjacentReferenceFields(mxComponent, 8.0f);

    save(TestFilter::PDF_WRITER);

    CPPUNIT_ASSERT_EQUAL(3, lcl_countAnnotations(parsePDFExport(vcl::pdf::PDFiumLibrary::get())));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testReduceSmallImage)
{
    // Load the Writer document.
    loadFromFile(u"reduce-small-image.fodt");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
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
    // i.e. the image was scaled down to 300 DPI, even though it had a tiny size.
    CPPUNIT_ASSERT_EQUAL(16, nWidth);
    CPPUNIT_ASSERT_EQUAL(16, nHeight);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf114256)
{
    loadFromFile(u"tdf114256.ods");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
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
    loadFromFile(u"tdf150931.ods");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
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
    loadFromFile(u"tdf147027.ods");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
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
    loadFromFile(u"tdf135346.ods");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Without the fix in place, this test would have failed with
    // - Expected: 55
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(55, pPdfPage->getObjectCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf147164)
{
    loadFromFile(u"tdf147164.odp");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
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
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
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
    loadFromFile(u"link-wrong-page.odp");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

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
    // When exporting the 2nd and 3rd page to pdf:
    uno::Sequence<beans::PropertyValue> aFilterData = {
        comphelper::makePropertyValue(u"PageRange"_ustr, u"2-3"_ustr),
    };
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    loadFromFile(u"link-wrong-page-partial.odg");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Then make sure that we have a link on the 1st page, but not on the 2nd one:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
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
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterOptions"_ustr]
        <<= u"{\"PageRange\":{\"type\":\"string\",\"value\":\"2-\"}}"_ustr;
    loadFromFile(u"link-wrong-page-partial.odg");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Then make sure the resulting PDF has 2 pages:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. FilterOptions was ignored.
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testLargePage)
{
    // Import the bugdoc and export as PDF.
    loadFromFile(u"6m-wide.odg");
    save(TestFilter::PDF_WRITER);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

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
    loadFromURL(u"private:factory/swriter"_ustr);
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
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
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
    loadFromURL(u"private:factory/swriter"_ustr);

    // Save as PDF.
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    int nFileVersion = pPdfDocument->getFileVersion();
    CPPUNIT_ASSERT_EQUAL(17, nFileVersion);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testVersion15)
{
    // Create an empty document.
    loadFromURL(u"private:factory/swriter"_ustr);

    // Save as PDF.
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "SelectPdfVersion", uno::Any(static_cast<sal_Int32>(15)) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    int nFileVersion = pPdfDocument->getFileVersion();
    CPPUNIT_ASSERT_EQUAL(15, nFileVersion);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf160196)
{
    // Create an empty document.
    mxComponent = loadFromDesktop("private:factory/swriter");

    // Add a title, because
    // the Metadata stream as specified in ISO 32000-2:2020,
    // 14.3 in the document catalog dictionary shall contain a dc:title entry
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocumentPropertiesSupplier.is());
    uno::Reference<document::XDocumentProperties> xDocumentProperties
        = xDocumentPropertiesSupplier->getDocumentProperties();
    xDocumentProperties->setTitle(u"Title"_ustr);

    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) },
                                           { "SelectPdfVersion", uno::Any(sal_Int32(20)) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor["FilterData"] <<= aFilterData;
    // Without the fix in place, the validation would have failed
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTaggedNestedTableInRepeatedHeaderRow)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    // The outer table spans more than one page, so its header row, and the
    // nested table it carries, are laid out once per page.
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tagged-nested-table-in-repeated-header.fodt");
    skipValidation();

    // Tagging the nested table a second time aborted on the assert that each
    // frame is tagged only once.
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The counts do not depend on the page count: each body row and the nested
    // table are tagged once, the repeated headers are artifacts.
    int nTables(0);
    int nRows(0);
    int nParagraphs(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (!pType || pType->GetValue() != "StructElem")
            continue;
        auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
        if (!pS)
            continue;
        if (pS->GetValue() == "Table")
            ++nTables;
        else if (pS->GetValue() == "TR")
            ++nRows;
        else if (pS->GetValue() == "Standard")
            ++nParagraphs;
    }

    // The outer table and the one nested table.
    CPPUNIT_ASSERT_EQUAL(2, nTables);
    // One header row, four body rows, and the nested table's single row. A row
    // per page for the nested table would mean it was tagged on each repeat.
    CPPUNIT_ASSERT_EQUAL(6, nRows);
    // One paragraph in the nested table and one in each of the four body rows.
    CPPUNIT_ASSERT_EQUAL(5, nParagraphs);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testVersion20)
{
    // Create an empty document.
    mxComponent = loadFromDesktop("private:factory/swriter");

    // Save as PDF.
    uno::Sequence<beans::PropertyValue> aFilterData = comphelper::InitPropertySequence(
        { { "SelectPdfVersion", uno::Any(static_cast<sal_Int32>(20)) } });
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor["FilterData"] <<= aFilterData;
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());
    int nFileVersion = pPdfDocument->getFileVersion();
    CPPUNIT_ASSERT_EQUAL(20, nFileVersion);
}

// Check round-trip of importing and exporting the PDF with PDFium filter,
// which imports the PDF document as multiple PDFs as graphic object.
// Each page in the document has one PDF graphic object whose content is
// the corresponding page in the PDF. When such a document is exported,
// the PDF graphic gets embedded into the exported PDF document (as a
// Form XObject).
CPPUNIT_TEST_FIXTURE(PdfExportTest2, testMultiPagePDF)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    UsePdfium aGuard;

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"SimpleMultiPagePDF.pdf");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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

        SvMemoryStream aUncompressed;
        ZCodec aZCodec;
        aZCodec.BeginCompression();
        aZCodec.Decompress(rObjectStream, aUncompressed);
        CPPUNIT_ASSERT(aZCodec.EndCompression());

        // Just check that the size of the page stream is what is expected.
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(1218), aUncompressed.Tell());
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

        SvMemoryStream aUncompressed;
        ZCodec aZCodec;
        aZCodec.BeginCompression();
        aZCodec.Decompress(rObjectStream, aUncompressed);
        CPPUNIT_ASSERT(aZCodec.EndCompression());

        // Just check that the size of the page stream is what is expected.
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(3893), aUncompressed.Tell());
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

        SvMemoryStream aUncompressed;
        ZCodec aZCodec;
        aZCodec.BeginCompression();
        aZCodec.Decompress(rObjectStream, aUncompressed);
        CPPUNIT_ASSERT(aZCodec.EndCompression());

        // Just check that the size of the page stream is what is expected.
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(355), aUncompressed.Tell());
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testFormFontName)
{
    // Import the bugdoc and export as PDF.
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "ExportFormFields", uno::Any(true) },
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    loadFromFile(u"form-font-name.odt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport(vcl::pdf::PDFiumLibrary::get());

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
    // We need to enable PDFium import (and make sure to disable after the test)
    UsePdfium aGuard;

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"PDFWithImages.pdf");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf160117)
{
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf160117.ods");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
    // We need to enable PDFium import (and make sure to disable after the test)
    UsePdfium aGuard;

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"ComplexContentDictionary.pdf");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf170448)
{
    loadFromFile(u"tdf170448.odt");
    save(TestFilter::PDF_WRITER);

    xmlDocUniquePtr pXmlDoc = parseMetadata();
    // Without the fix in place, this test would have failed with
    // - Expected: ' " < > & - ‘ » - l’île (copié-collé du texte)
    // - Actual  : &apos; &quot; &lt; &gt; &amp; - ‘ » - l’île (copié-collé du texte)
    assertXPathContent(pXmlDoc, "/x:xmpmeta/rdf:RDF/rdf:Description[1]/dc:title/rdf:Alt/rdf:li",
                       u"' \" < > & - ‘ » - l’île (copié-collé du texte)");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[1]/dc:description/rdf:Alt/rdf:li",
                       u"' \" <>& - ' \" - l'île (écrit directement au clavier)");
}

// Tests that at export the PDF has the PDF/UA metadata properly set
// when we enable PDF/UA support.
CPPUNIT_TEST_FIXTURE(PdfExportTest2, testPdfUaMetadata)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"BrownFoxLazyDog.odt");
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    xmlDocUniquePtr pXmlDoc = parseMetadata();
    assertXPathContent(pXmlDoc, "/x:xmpmeta/rdf:RDF/rdf:Description[1]/pdfuaid:part", u"1");
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf138792)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf138792.odt");
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    xmlDocUniquePtr pXmlDoc = parseMetadata();
    assertXPath(pXmlDoc, "/x:xmpmeta/rdf:RDF/rdf:Description[1]/dc:date/rdf:Seq/rdf:li");
    assertXPathContent(pXmlDoc, "/x:xmpmeta/rdf:RDF/rdf:Description[1]/dc:format",
                       u"application/pdf");
    assertXPathContent(pXmlDoc, "/x:xmpmeta/rdf:RDF/rdf:Description[1]/dc:title/rdf:Alt/rdf:li",
                       u"Test document title");
    assertXPathContent(pXmlDoc, "/x:xmpmeta/rdf:RDF/rdf:Description[1]/dc:creator/rdf:Seq/rdf:li",
                       u"Gabor Kelemen LO");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[1]/dc:description/rdf:Alt/rdf:li",
                       u"Test subject");
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf153472)
{
    // Use PDF/A-1b
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "SelectPdfVersion", uno::Any(sal_Int32(1)) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromURL(u"private:factory/swriter"_ustr);
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    xmlDocUniquePtr pXmlDoc = parseMetadata();
    assertXPathContent(pXmlDoc, "/x:xmpmeta/rdf:RDF/rdf:Description[1]/pdfaid:part", u"1");
    assertXPathContent(pXmlDoc, "/x:xmpmeta/rdf:RDF/rdf:Description[1]/pdfaid:conformance", u"B");
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf139736)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) },
                                           { "SelectPdfVersion", uno::Any(sal_Int32(17)) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf139736-1.odt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
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

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf168057)
{
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) },
                                           { "SelectPdfVersion", uno::Any(sal_Int32(20)) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf168057.odt");
    // Without the fix in place, the validation would have failed
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf152231)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) },
                                           { "ExportNotesInMargin", uno::Any(true) },
                                           { "SelectPdfVersion", uno::Any(sal_Int32(17)) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf152231.fodt");
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
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

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf136805)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "SelectPdfVersion", uno::Any(static_cast<sal_Int32>(1)) },
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf136805.pdf");
    // Without the fix in place, the validation would have failed
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf157517)
{
    // Only reproduced with PDF/A-2 / PDF/A-3b and PDF/UA.
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "PDFUACompliance", uno::Any(true) },
        { "SelectPdfVersion", uno::Any(static_cast<sal_Int32>(3)) },
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf157517.odt");
    // Without the fix in place, the validation would have failed
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    xmlDocUniquePtr pXmlDoc = parseMetadata();
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:namespaceURI",
                       u"http://www.aiim.org/pdfua/ns/id/");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:prefix",
                       u"pdfuaid");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:schema",
                       u"PDF/UA identification schema");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[1]/pdfaProperty:category",
                       u"internal");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[1]/pdfaProperty:description",
                       u"PDF/UA version identifier");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[1]/pdfaProperty:name",
                       u"part");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[1]/pdfaProperty:valueType",
                       u"Integer");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[2]/pdfaProperty:category",
                       u"internal");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[2]/pdfaProperty:description",
                       u"PDF/UA amendment identifier");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[2]/pdfaProperty:name",
                       u"amd");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[2]/pdfaProperty:valueType",
                       u"Text");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[3]/pdfaProperty:category",
                       u"internal");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[3]/pdfaProperty:description",
                       u"PDF/UA corrigenda identifier");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[3]/pdfaProperty:name",
                       u"corr");
    assertXPathContent(pXmlDoc,
                       "/x:xmpmeta/rdf:RDF/rdf:Description[3]/pdfaExtension:schemas/rdf:Bag/rdf:li/"
                       "pdfaSchema:property/rdf:Seq/rdf:li[3]/pdfaProperty:valueType",
                       u"Text");
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf152218)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "PDFUACompliance", uno::Any(true) },
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf152218.odt");
    // Without the fix in place, the validation would have failed
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf152235)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "PDFUACompliance", uno::Any(true) },
          { "Watermark", uno::Any(u"kendy"_ustr) },
          // need to set a font to avoid assertions about missing "Helvetica"
          { "WatermarkFontName", uno::Any(u"Liberation Sans"_ustr) },
          { "SelectPdfVersion", uno::Any(sal_Int32(17)) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    loadFromURL(u"private:factory/swriter"_ustr);
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

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

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf167290)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "SelectPdfVersion", uno::Any(static_cast<sal_Int32>(1)) },
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf167290.odt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf149140)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"TableTH_test_LibreOfficeWriter7.3.3_HeaderRow-HeadersInTopRow.fodt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
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

// the structure element type of a table cell that a row's kid refers to
OString GetCellType(vcl::filter::PDFElement* pElement)
{
    auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pElement);
    CPPUNIT_ASSERT(pRef);
    auto pCell = pRef->LookupObject();
    CPPUNIT_ASSERT(pCell);
    auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pCell->Lookup("S"_ostr));
    CPPUNIT_ASSERT(pS);
    return pS->GetValue();
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf173162)
{
    loadFromFile(u"StructureNamespaces.fodt");

    // Tagged rather than PDF/UA, because the harness validates a PDF/UA export and this document
    // still fails ISO 14289-2 8.2.5.8 (a TOCI needs Ref) and 8.2.5.14 (PDF/UA-2 wants FENote,
    // not Note). TODO: ask for PDF/UA here once those are fixed. The namespaces below depend on
    // the version alone.
    uno::Sequence aFilterData{ comphelper::makePropertyValue(u"UseTaggedPDF"_ustr, true),
                               comphelper::makePropertyValue(u"SelectPdfVersion"_ustr,
                                                             sal_Int32(20)) };
    save(TestFilter::PDF_WRITER,
         { comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData) });

    vcl::filter::PDFDocument aDocument;
    CPPUNIT_ASSERT(aDocument.Read(*maTempFile.GetStream(StreamMode::READ)));

    static constexpr OString aPDF17("http://iso.org/pdf/ssn"_ostr);
    static constexpr OString aPDF20("http://iso.org/pdf2/ssn"_ostr);

    // the namespace each element points at, by object
    std::unordered_map<vcl::filter::PDFObjectElement*, OString> aNamespaceOf;
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (!pType || pType->GetValue() != "Namespace")
            continue;
        auto pURI = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pObject->Lookup("NS"_ostr));
        CPPUNIT_ASSERT(pURI);
        aNamespaceOf[pObject] = pURI->GetValue();

        // the 1.7 namespace needs none: for it the structure tree root's RoleMap is the fallback
        auto pMap
            = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject->Lookup("RoleMapNS"_ostr));
        if (pURI->GetValue() == aPDF17)
            CPPUNIT_ASSERT_MESSAGE("the 1.7 namespace maps nothing", !pMap);
        else
            CPPUNIT_ASSERT(pMap);
    }
    // one for PDF 2.0 and one for the types it dropped
    CPPUNIT_ASSERT_EQUAL(size_t(2), aNamespaceOf.size());

    CPPUNIT_ASSERT(aDocument.GetCatalog());
    auto pRootRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
        aDocument.GetCatalog()->Lookup("StructTreeRoot"_ostr));
    CPPUNIT_ASSERT(pRootRef);
    CPPUNIT_ASSERT(pRootRef->LookupObject());
    auto pNamespaces = dynamic_cast<vcl::filter::PDFArrayElement*>(
        pRootRef->LookupObject()->Lookup("Namespaces"_ostr));
    CPPUNIT_ASSERT(pNamespaces);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pNamespaces->GetElements().size());
    for (const auto pElement : pNamespaces->GetElements())
    {
        auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pElement);
        CPPUNIT_ASSERT(pRef);
        CPPUNIT_ASSERT_MESSAGE("the root names a namespace object",
                               aNamespaceOf.contains(pRef->LookupObject()));
    }

    // The references have to be separated in the file itself: "1 0 R2 0 R" is not two
    // references, and this parser reads it as though it were, so check the bytes. veraPDF is
    // stricter - it fails to parse such a file and reports nothing, which reads as a pass.
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    pStream->Seek(0);
    const OString aFile(read_uInt8s_ToOString(*pStream, pStream->remainingSize()));
    const sal_Int32 nStart(aFile.indexOf("/Namespaces ["));
    CPPUNIT_ASSERT_GREATER(sal_Int32(-1), nStart);
    const sal_Int32 nEnd(aFile.indexOf("]", nStart));
    CPPUNIT_ASSERT_GREATER(nStart, nEnd);
    const std::string_view aArray(aFile.subView(nStart, nEnd - nStart));
    for (size_t nR = aArray.find('R'); nR != std::string_view::npos; nR = aArray.find('R', nR + 1))
    {
        const bool bSeparated(nR + 1 == aArray.size()
                              || !rtl::isAsciiDigit(static_cast<unsigned char>(aArray[nR + 1])));
        CPPUNIT_ASSERT_MESSAGE(
            OString(OString::Concat("unseparated references in ") + aArray).getStr(), bSeparated);
    }

    // the namespace each structure type landed in
    std::unordered_map<OString, OString> aNamespaceOfType;
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (!pType || pType->GetValue() != "StructElem")
            continue;
        auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
        if (!pS)
            continue;
        auto pNS = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("NS"_ostr));
        CPPUNIT_ASSERT_MESSAGE("every element in a PDF 2.0 file names its namespace", pNS);
        const OString aURI(aNamespaceOf[pNS->LookupObject()]);
        const auto aEntry(aNamespaceOfType.emplace(pS->GetValue(), aURI));
        if (!aEntry.second)
            CPPUNIT_ASSERT_EQUAL_MESSAGE("one type, two namespaces", aEntry.first->second, aURI);
    }

    // the types PDF 2.0 dropped, which ISO 14289-2 8.2.4 accepts only in the 1.7 namespace
    CPPUNIT_ASSERT_EQUAL(aPDF17, aNamespaceOfType["TOC"_ostr]);
    CPPUNIT_ASSERT_EQUAL(aPDF17, aNamespaceOfType["TOCI"_ostr]);
    CPPUNIT_ASSERT_EQUAL(aPDF17, aNamespaceOfType["Index"_ostr]);
    CPPUNIT_ASSERT_EQUAL(aPDF17, aNamespaceOfType["BlockQuote"_ostr]);
    CPPUNIT_ASSERT_EQUAL(aPDF17, aNamespaceOfType["Quote"_ostr]);
    CPPUNIT_ASSERT_EQUAL(aPDF17, aNamespaceOfType["Code"_ostr]);
    CPPUNIT_ASSERT_EQUAL(aPDF17, aNamespaceOfType["Note"_ostr]);
    CPPUNIT_ASSERT_EQUAL(aPDF17, aNamespaceOfType["BibEntry"_ostr]);
    // and the ones it kept
    CPPUNIT_ASSERT_EQUAL(aPDF20, aNamespaceOfType["Document"_ostr]);
    CPPUNIT_ASSERT_EQUAL(aPDF20, aNamespaceOfType["H1"_ostr]);
    CPPUNIT_ASSERT_EQUAL(aPDF20, aNamespaceOfType["Link"_ostr]);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf166963)
{
    // Takes about two minutes locally, most of it in the PDFDocument below, which rescans its
    // element list once per object. Disable the test if that costs CI too much.
    //
    // A table with more rows than ncMaxPDFArraySize, so its kid list has to be split into Div
    // containers, and with a repeated heading row, which is emitted as a NonStructElement: that
    // gives the Table element more children than kids, which is what the split used to mishandle.
    utl::TempFileNamed aSource(u"tdf166963", true, u".fodt");
    aSource.EnableKillingFile();
    SvStream* pSource = aSource.GetStream(StreamMode::WRITE | StreamMode::TRUNC);
    pSource->WriteOString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<office:document"
        " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
        " xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
        " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\""
        " xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
        " xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\""
        " xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\""
        " office:version=\"1.3\""
        " office:mimetype=\"application/vnd.oasis.opendocument.text\">"
        "<office:font-face-decls><style:font-face"
        " style:name=\"Liberation Serif\""
        " svg:font-family=\"&apos;Liberation Serif&apos;\"/>"
        "</office:font-face-decls>"
        "<office:styles><style:default-style style:family=\"paragraph\">"
        "<style:text-properties style:font-name=\"Liberation Serif\""
        " fo:font-size=\"6pt\" fo:language=\"en\" fo:country=\"US\"/>"
        "</style:default-style></office:styles>"
        "<office:body><office:text><table:table table:name=\"T\">"
        "<table:table-column/>"
        "<table:table-header-rows><table:table-row><table:table-cell"
        " office:value-type=\"string\"><text:p>h</text:p></table:table-cell>"
        "</table:table-row></table:table-header-rows>");
    for (int i = 0; i < 8300; ++i)
        pSource->WriteOString("<table:table-row><table:table-cell office:value-type=\"string\">"
                              "<text:p>r</text:p></table:table-cell></table:table-row>");
    pSource->WriteOString("</table:table></office:text></office:body></office:document>");
    aSource.CloseStream();

    loadFromURL(aSource.GetURL());

    uno::Sequence aFilterData{ comphelper::makePropertyValue(u"UseTaggedPDF"_ustr, true) };
    save(TestFilter::PDF_WRITER,
         { comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData) });

    vcl::filter::PDFDocument aDocument;
    CPPUNIT_ASSERT(aDocument.Read(*maTempFile.GetStream(StreamMode::READ)));

    // which element actually lists each structure element as its kid
    std::unordered_map<vcl::filter::PDFObjectElement*, vcl::filter::PDFObjectElement*> aLister;
    std::vector<vcl::filter::PDFObjectElement*> aElements;
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (!pType)
            continue;
        const bool bRoot(pType->GetValue() == "StructTreeRoot");
        if (!bRoot && pType->GetValue() != "StructElem")
            continue;
        if (!bRoot)
            aElements.push_back(pObject);
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject->Lookup("K"_ostr));
        if (!pKids)
            continue;
        for (const auto pKid : pKids->GetElements())
        {
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKid);
            if (!pRef)
                continue;
            CPPUNIT_ASSERT_MESSAGE("a structure element is the kid of two elements",
                                   aLister.emplace(pRef->LookupObject(), pObject).second);
        }
    }
    // a generated container holds exactly ncMaxPDFArraySize kids; without one the kid list
    // never overflowed and the document proves nothing
    size_t nFullContainers(0);
    for (const auto pElement : aElements)
    {
        auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pElement->Lookup("S"_ostr));
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pElement->Lookup("K"_ostr));
        if (pS && pS->GetValue() == "Div" && pKids && pKids->GetElements().size() == 8191)
            ++nFullContainers;
    }
    CPPUNIT_ASSERT_EQUAL(size_t(1), nFullContainers);

    for (const auto pElement : aElements)
    {
        auto pParent = dynamic_cast<vcl::filter::PDFReferenceElement*>(pElement->Lookup("P"_ostr));
        CPPUNIT_ASSERT(pParent);
        CPPUNIT_ASSERT_MESSAGE("a structure element that nothing lists as its kid",
                               aLister.contains(pElement));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("a structure element claims a parent that does not list it",
                                     aLister.at(pElement), pParent->LookupObject());
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf173194)
{
    loadFromFile(u"TableHeadingDerived.fodt");

    uno::Sequence aFilterData{ comphelper::makePropertyValue(u"PDFUACompliance"_ustr, true) };
    save(TestFilter::PDF_WRITER,
         { comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData) });

    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::filter::PDFDocument aDocument;
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    vcl::filter::PDFObjectElement* pTableSE(nullptr);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
        if (pType && pType->GetValue() == "StructElem" && pS && pS->GetValue() == "Table")
        {
            CPPUNIT_ASSERT(!pTableSE);
            pTableSE = pObject;
        }
    }
    CPPUNIT_ASSERT(pTableSE);

    auto pRows = dynamic_cast<vcl::filter::PDFArrayElement*>(pTableSE->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pRows);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pRows->GetElements().size());

    // the first column is Table Heading in row one and a style derived from it in row two,
    // so both rows read TH then TD
    for (const auto pRowElement : pRows->GetElements())
    {
        auto pRowRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pRowElement);
        CPPUNIT_ASSERT(pRowRef);
        auto pRow = pRowRef->LookupObject();
        CPPUNIT_ASSERT(pRow);
        auto pRowS = dynamic_cast<vcl::filter::PDFNameElement*>(pRow->Lookup("S"_ostr));
        CPPUNIT_ASSERT(pRowS);
        CPPUNIT_ASSERT_EQUAL("TR"_ostr, pRowS->GetValue());

        auto pCells = dynamic_cast<vcl::filter::PDFArrayElement*>(pRow->Lookup("K"_ostr));
        CPPUNIT_ASSERT(pCells);
        CPPUNIT_ASSERT_EQUAL(size_t(2), pCells->GetElements().size());
        CPPUNIT_ASSERT_EQUAL("TH"_ostr, GetCellType(pCells->GetElement(0)));
        CPPUNIT_ASSERT_EQUAL("TD"_ostr, GetCellType(pCells->GetElement(1)));
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf153935)
{
    // tdf#153935: PDF/A-1 + PDF/UA must still emit /Scope on TH and /Tabs on
    // pages with annotations. Without the fix the version check rejects them
    // because PDF/A-1 forces PDF version 1.4 (< 1.5).
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "PDFUACompliance", uno::Any(true) },
        { "SelectPdfVersion", uno::Any(sal_Int32(1)) }, // PDF/A-1b
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"TableTH_test_LibreOfficeWriter7.3.3_HeaderRow-HeadersInTopRow.fodt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // Each page should carry /Tabs /S (clause 7.18.3) regardless of PDF version
    // when PDF/UA is requested, since PDF/UA mandates structural tab order on
    // any page that holds annotations.
    int nPagesWithTabs(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (!pType || pType->GetValue() != "Page")
            continue;
        auto pTabs = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Tabs"_ostr));
        CPPUNIT_ASSERT_MESSAGE("Page is missing /Tabs entry under PDF/A-1 + PDF/UA",
                               pTabs != nullptr);
        CPPUNIT_ASSERT_EQUAL("S"_ostr, pTabs->GetValue());
        ++nPagesWithTabs;
    }
    CPPUNIT_ASSERT_GREATEREQUAL(1, nPagesWithTabs);

    // Each TH structure element must carry the /Scope /Column attribute under
    // the /Table owner (clause 7.5), regardless of PDF version when PDF/UA is
    // requested. The fixture has 6 TH cells.
    int nTH(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (!pType || pType->GetValue() != "StructElem")
            continue;
        auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
        if (!pS || pS->GetValue() != "TH")
            continue;
        int nTable(0);
        auto pAttrs = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject->Lookup("A"_ostr));
        CPPUNIT_ASSERT(pAttrs != nullptr);
        for (const auto& rAttrRef : pAttrs->GetElements())
        {
            auto pAttrDict = dynamic_cast<vcl::filter::PDFDictionaryElement*>(rAttrRef);
            CPPUNIT_ASSERT(pAttrDict != nullptr);
            auto pOwner
                = dynamic_cast<vcl::filter::PDFNameElement*>(pAttrDict->LookupElement("O"_ostr));
            CPPUNIT_ASSERT(pOwner != nullptr);
            if (pOwner->GetValue() == "Table")
            {
                auto pScope = dynamic_cast<vcl::filter::PDFNameElement*>(
                    pAttrDict->LookupElement("Scope"_ostr));
                CPPUNIT_ASSERT_MESSAGE("TH is missing /Scope attribute under PDF/A-1 + PDF/UA",
                                       pScope != nullptr);
                CPPUNIT_ASSERT_EQUAL("Column"_ostr, pScope->GetValue());
                ++nTable;
            }
        }
        CPPUNIT_ASSERT_EQUAL(1, nTable);
        ++nTH;
    }
    CPPUNIT_ASSERT_EQUAL(6, nTH);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testNestedSection)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"nestedsection.fodt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
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
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"SimpleTOC.fodt");
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has two pages.
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

    // The ToC heading is a sibling of <TOC>, not a child (PDF/UA disallows
    // non-TOCI children of <TOC>). Locate TOC's parent and find the entry
    // immediately preceding TOC in its children list — that should be the
    // <H1> "Table of Contents" heading.
    auto pTOCParentRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pTOC->Lookup("P"_ostr));
    CPPUNIT_ASSERT(pTOCParentRef);
    auto pTOCParent = pTOCParentRef->LookupObject();
    CPPUNIT_ASSERT(pTOCParent);
    auto pParentKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pTOCParent->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pParentKids);
    auto pParentKidsV = pParentKids->GetElements();
    size_t nTocIndex = 0;
    bool bFoundTOC = false;
    for (size_t i = 0; i < pParentKidsV.size(); ++i)
    {
        auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pParentKidsV[i]);
        if (pRef && pRef->LookupObject() == pTOC)
        {
            nTocIndex = i;
            bFoundTOC = true;
            break;
        }
    }
    CPPUNIT_ASSERT(bFoundTOC);
    CPPUNIT_ASSERT(nTocIndex > 0);
    auto pHeadingRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pParentKidsV[nTocIndex - 1]);
    CPPUNIT_ASSERT(pHeadingRef);
    auto pHeading = pHeadingRef->LookupObject();
    CPPUNIT_ASSERT(pHeading);
    auto pHeadingType = dynamic_cast<vcl::filter::PDFNameElement*>(pHeading->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pHeadingType->GetValue());
    auto pHeadingS = dynamic_cast<vcl::filter::PDFNameElement*>(pHeading->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("H1"_ostr, pHeadingS->GetValue());
    // The H1 contains the heading text directly (an MCID), with no
    // intermediate paragraph struct element.
    auto pHeadingKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pHeading->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pHeadingKids);
    auto pHeadingKidsV = pHeadingKids->GetElements();
    CPPUNIT_ASSERT(!pHeadingKidsV.empty());
    auto pHeadingMcid = dynamic_cast<vcl::filter::PDFNumberElement*>(pHeadingKidsV[0]);
    CPPUNIT_ASSERT(pHeadingMcid);

    // <TOC>'s first child must be a <TOCI> — no headings inside.
    auto pKidsT = dynamic_cast<vcl::filter::PDFArrayElement*>(pTOC->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsT);
    auto pKidsTv = pKidsT->GetElements();
    auto pRefKidT1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsTv[0]);
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
    CPPUNIT_ASSERT_EQUAL("Contents 1"_ostr, pST10->GetValue());

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
    CPPUNIT_ASSERT_EQUAL("Contents 2"_ostr, pST20->GetValue());

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

    auto pRefKidT3 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsTv[2]);
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
    CPPUNIT_ASSERT_EQUAL("Contents 3"_ostr, pST30->GetValue());

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
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"image-shape.fodt");
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
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
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"LO_Lbl_Lbody_bug_report.fodt");
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
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
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"spanlist.fodt");
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
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
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "PDFUACompliance", uno::Any(true) },
        // only happens with PDF/A-1
        { "SelectPdfVersion", uno::Any(static_cast<sal_Int32>(1)) },
    }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    loadFromFile(u"transparentshape.fodp");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // just check this does not crash or assert
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf57423)
{
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"Description PDF Export test .odt");
    skipValidation();
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
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
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    comphelper::SequenceAsHashMap aMediaDescriptor;
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;

    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"tdf154982.odt");
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
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

CPPUNIT_TEST_FIXTURE(PdfExportTest2, cool16122BadPantoneElement)
{
    vcl::filter::PDFDocument aDocument;
    loadFromFile(u"cool16122.odt");
    save(TestFilter::PDF_WRITER);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    // Previously this would have failed here with "PDFDocument::Tokenize: unexpected 'C' keyword at byte position 280"
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // But just to be sure, check that the PANTONE element round-trips properly
    bool bFound = false;
    for (const auto& rDocElement : aDocument.GetElements())
    {
        if (auto pObject = dynamic_cast<vcl::filter::PDFNameElement*>(rDocElement.get()))
            if (pObject->GetValue() == "PANTONE 293 C")
            {
                bFound = true;
                break;
            }
    }
    CPPUNIT_ASSERT(bFound);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest2, testTdf168462)
{
    uno::Sequence aFilterData{ comphelper::makePropertyValue(u"UseTaggedPDF"_ustr, true) };
    loadFromFile(u"master-background-artifact.fodp");
    save(TestFilter::PDF_WRITER,
         { comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData) });

    vcl::filter::PDFDocument aDocument;
    CPPUNIT_ASSERT(aDocument.Read(*maTempFile.GetStream(StreamMode::READ)));
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());
    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    // an operator shares its line with its operands, and sometimes with another operator
    auto isPaintingOperator = [](const std::string_view line) {
        for (size_t nPos = 0; nPos != std::string_view::npos;)
        {
            const size_t nStart = line.find_first_not_of(' ', nPos);
            if (nStart == std::string_view::npos)
                break;
            nPos = line.find(' ', nStart);
            const std::string_view aToken(
                line.substr(nStart, nPos == std::string_view::npos ? nPos : nPos - nStart));
            for (const auto op :
                 { "f", "f*", "F", "S", "s", "B", "B*", "b", "b*", "Do", "TJ", "Tj" })
            {
                if (aToken == op)
                    return true;
            }
        }
        return false;
    };

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();
    int nOpenMarks(0);
    int nArtifacts(0);
    OStringBuffer aUntagged;
    // ISO 14289-1 7.1: content is either tagged or marked as an artifact
    while (pStart != pEnd)
    {
        const auto pLineEnd = std::find(pStart, pEnd, '\n');
        const std::string_view line(pStart, pLineEnd - pStart);
        pStart = pLineEnd == pEnd ? pEnd : pLineEnd + 1;

        if (line == "EMC")
            --nOpenMarks;
        else if (o3tl::ends_with(line, "BMC") || o3tl::ends_with(line, "BDC"))
        {
            ++nOpenMarks;
            if (o3tl::starts_with(line, "/Artifact"))
                ++nArtifacts;
        }
        else if (nOpenMarks == 0 && isPaintingOperator(line))
            aUntagged.append(OString::Concat(line) + " ");
    }

    // without the fix the background was painted outside every marked-content section
    CPPUNIT_ASSERT_MESSAGE(aUntagged.toString().getStr(), aUntagged.isEmpty());
    // the page decoration, and the background the slide takes from its master page
    CPPUNIT_ASSERT_EQUAL(2, nArtifacts);
    CPPUNIT_ASSERT_EQUAL(0, nOpenMarks);
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
