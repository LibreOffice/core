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
#include <type_traits>

#include <config_features.h>
#include <config_fonts.h>
#include <osl/process.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <comphelper/scopeguard.hxx>
#include <comphelper/processfactory.hxx>
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

#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/pdfread.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests the PDF export filter.
class PdfExportTest : public UnoApiTest
{
protected:
    utl::MediaDescriptor aMediaDescriptor;

public:
    PdfExportTest()
        : UnoApiTest("/vcl/qa/cppunit/pdfexport/data/")
    {
    }

    void saveAsPDF(std::u16string_view rFile);
    void load(std::u16string_view rFile, vcl::filter::PDFDocument& rDocument,
              bool bUseTaggedPDF = true);
};

void PdfExportTest::saveAsPDF(std::u16string_view rFile)
{
    // Import the bugdoc and export as PDF.
    loadFromURL(rFile);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
}

void PdfExportTest::load(std::u16string_view rFile, vcl::filter::PDFDocument& rDocument,
                         bool bUseTaggedPDF)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "UseTaggedPDF", uno::Any(bUseTaggedPDF) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(rFile);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(rDocument.Read(aStream));
}

/// Tests that a pdf image is roundtripped back to PDF as a vector format.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf106059)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    // Explicitly enable the usage of the reference XObject markup.
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "UseReferenceXObject", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"tdf106059.odt");

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // Assert that the XObject in the page resources dictionary is a reference XObject.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    // The page has one image.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pReferenceXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pReferenceXObject);
    // The image is a reference XObject.
    // This dictionary key was missing, so the XObject wasn't a reference one.
    CPPUNIT_ASSERT(pReferenceXObject->Lookup("Ref"));
}

/// Tests export of PDF images without reference XObjects.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf106693)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf106693.odt", aDocument);

    // Assert that the XObject in the page resources dictionary is a form XObject.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    // The page has one image.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);
    // The image is a form XObject.
    auto pSubtype = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject->Lookup("Subtype"));
    CPPUNIT_ASSERT(pSubtype);
    CPPUNIT_ASSERT_EQUAL(OString("Form"), pSubtype->GetValue());
    // This failed: UseReferenceXObject was ignored and Ref was always created.
    CPPUNIT_ASSERT(!pXObject->Lookup("Ref"));

    // Assert that the form object refers to an inner form object, not a
    // bitmap.
    auto pInnerResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"));
    CPPUNIT_ASSERT(pInnerResources);
    auto pInnerXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
        pInnerResources->LookupElement("XObject"));
    CPPUNIT_ASSERT(pInnerXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pInnerXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pInnerXObject
        = pInnerXObjects->LookupObject(pInnerXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pInnerXObject);
    auto pInnerSubtype
        = dynamic_cast<vcl::filter::PDFNameElement*>(pInnerXObject->Lookup("Subtype"));
    CPPUNIT_ASSERT(pInnerSubtype);
    // This failed: this was Image (bitmap), not Form (vector).
    CPPUNIT_ASSERT_EQUAL(OString("Form"), pInnerSubtype->GetValue());
}

/// Tests that text highlight from Impress is not lost.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf105461)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf105461.odp");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Make sure there is a filled rectangle inside.
    int nPageObjectCount = pPdfPage->getObjectCount();
    int nYellowPathCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPdfPageObject = pPdfPage->getObject(i);
        if (pPdfPageObject->getType() != vcl::pdf::PDFPageObjectType::Path)
            continue;

        if (pPdfPageObject->getFillColor() == COL_YELLOW)
            ++nYellowPathCount;
    }

    // This was 0, the page contained no yellow paths.
    CPPUNIT_ASSERT_EQUAL(1, nYellowPathCount);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf107868)
{
// No need to run it on Windows, since it would use GDI printing, and not trigger PDF export
// which is the intent of the test.
// FIXME: Why does this fail on macOS?
#if !defined MACOSX && !defined _WIN32

    // Import the bugdoc and print to PDF.
    loadFromURL(u"tdf107868.odt");
    uno::Reference<view::XPrintable> xPrintable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPrintable.is());
    uno::Sequence<beans::PropertyValue> aOptions(comphelper::InitPropertySequence(
        { { "FileName", uno::Any(maTempFile.GetURL()) }, { "Wait", uno::Any(true) } }));
    xPrintable->print(aOptions);

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    if (!pPdfDocument)
        // Printing to PDF failed in a non-interesting way, e.g. CUPS is not
        // running, there is no printer defined, etc.
        return;

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Make sure there is no filled rectangle inside.
    int nPageObjectCount = pPdfPage->getObjectCount();
    int nWhitePathCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPdfPageObject = pPdfPage->getObject(i);
        if (pPdfPageObject->getType() != vcl::pdf::PDFPageObjectType::Path)
            continue;

        if (pPdfPageObject->getFillColor() == COL_WHITE)
            ++nWhitePathCount;
    }

    // This was 4, the page contained 4 white paths at problematic positions.
    CPPUNIT_ASSERT_EQUAL(0, nWhitePathCount);
#endif
}

/// Tests that embedded video from Impress is not exported as a linked one.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf105093)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf105093.odp", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // Get page annotations.
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(
        OString("Annot"),
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"))->GetValue());

    // Get the Action -> Rendition -> MediaClip -> FileSpec.
    auto pAction = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"));
    CPPUNIT_ASSERT(pAction);
    auto pRendition = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAction->LookupElement("R"));
    CPPUNIT_ASSERT(pRendition);
    auto pMediaClip
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pRendition->LookupElement("C"));
    CPPUNIT_ASSERT(pMediaClip);
    auto pFileSpec
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pMediaClip->LookupElement("D"));
    CPPUNIT_ASSERT(pFileSpec);
    // Make sure the filespec refers to an embedded file.
    // This key was missing, the embedded video was handled as a linked one.
    CPPUNIT_ASSERT(pFileSpec->LookupElement("EF"));
}

/// Tests export of non-PDF images.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf106206)
{
    // Import the bugdoc and export as PDF.
    vcl::filter::PDFDocument aDocument;
    load(u"tdf106206.odt", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // The page has a stream.
    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents");
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

    // Make sure there is an image reference there.
    OString aImage("/Im");
    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* pEnd = pStart + aUncompressed.GetSize();
    auto it = std::search(pStart, pEnd, aImage.getStr(), aImage.getStr() + aImage.getLength());
    CPPUNIT_ASSERT(it != pEnd);

    // And also that it's not an invalid one.
    OString aInvalidImage("/Im0");
    it = std::search(pStart, pEnd, aInvalidImage.getStr(),
                     aInvalidImage.getStr() + aInvalidImage.getLength());
    // This failed, object #0 was referenced.
    CPPUNIT_ASSERT(bool(it == pEnd));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf127217)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf127217.odt");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // The page has one annotation.
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnot = pPdfPage->getAnnotation(0);

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(!pAnnot->hasKey("DA"));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf109143)
{
    // Import the bugdoc and export as PDF.
    vcl::filter::PDFDocument aDocument;
    load(u"tdf109143.odt", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // Get access to the only image on the only page.
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Make sure it's re-compressed.
    auto pLength = dynamic_cast<vcl::filter::PDFNumberElement*>(pXObject->Lookup("Length"));
    CPPUNIT_ASSERT(pLength);
    int nLength = pLength->GetValue();
    // This failed: cropped TIFF-in-JPEG wasn't re-compressed, so crop was
    // lost. Size was 59416, now is 11827.
    CPPUNIT_ASSERT(nLength < 50000);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf106972)
{
    // Import the bugdoc and export as PDF.
    vcl::filter::PDFDocument aDocument;
    load(u"tdf106972.odt", aDocument);

    // Get access to the only form object on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Get access to the only image inside the form object.
    auto pFormResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"));
    CPPUNIT_ASSERT(pFormResources);
    auto pImages = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
        pFormResources->LookupElement("XObject"));
    CPPUNIT_ASSERT(pImages);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pImages->GetItems().size());
    vcl::filter::PDFObjectElement* pImage
        = pImages->LookupObject(pImages->GetItems().begin()->first);
    CPPUNIT_ASSERT(pImage);

    // Assert resources of the image.
    auto pImageResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pImage->Lookup("Resources"));
    CPPUNIT_ASSERT(pImageResources);
    // This failed: the PDF image had no Font resource.
    CPPUNIT_ASSERT(pImageResources->LookupElement("Font"));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf106972Pdf17)
{
    // Import the bugdoc and export as PDF.
    vcl::filter::PDFDocument aDocument;
    load(u"tdf106972-pdf17.odt", aDocument);

    // Get access to the only image on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Assert that we now attempt to preserve the original PDF data, even if
    // the original input was PDF >= 1.4.
    CPPUNIT_ASSERT(pXObject->Lookup("Resources"));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testSofthyphenPos)
{
// No need to run it on Windows, since it would use GDI printing, and not trigger PDF export
// which is the intent of the test.
// FIXME: Why does this fail on macOS?
#if !defined MACOSX && !defined _WIN32

    // Import the bugdoc and print to PDF.
    loadFromURL(u"softhyphen_pdf.odt");
    uno::Reference<view::XPrintable> xPrintable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPrintable.is());
    uno::Sequence<beans::PropertyValue> aOptions(comphelper::InitPropertySequence(
        { { "FileName", uno::Any(maTempFile.GetURL()) }, { "Wait", uno::Any(true) } }));
    xPrintable->print(aOptions);

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    if (!pPdfDocument)
        // Printing to PDF failed in a non-interesting way, e.g. CUPS is not
        // running, there is no printer defined, etc.
        return;

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // tdf#96892 incorrect fractional part of font size caused soft-hyphen to
    // be positioned inside preceding text (incorrect = 11.1, correct = 11.05)

    // there are 3 texts currently, for line 1, soft-hyphen, line 2
    bool haveText(false);

    int nPageObjectCount = pPdfPage->getObjectCount();
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPdfPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFPageObjectType::Text, pPdfPageObject->getType());
        haveText = true;
        double const size = pPdfPageObject->getFontSize();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.05, size, 1E-06);
    }

    CPPUNIT_ASSERT(haveText);
#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf107013)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf107013.odt", aDocument);

    // Get access to the only image on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    // This failed, the reference to the image was created, but not the image.
    CPPUNIT_ASSERT(pXObject);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf107018)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf107018.odt", aDocument);

    // Get access to the only image on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Get access to the form object inside the image.
    auto pXObjectResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"));
    CPPUNIT_ASSERT(pXObjectResources);
    auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
        pXObjectResources->LookupElement("XObject"));
    CPPUNIT_ASSERT(pXObjectForms);
    vcl::filter::PDFObjectElement* pForm
        = pXObjectForms->LookupObject(pXObjectForms->GetItems().begin()->first);
    CPPUNIT_ASSERT(pForm);

    // Get access to Resources -> Font -> F1 of the form.
    auto pFormResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pForm->Lookup("Resources"));
    CPPUNIT_ASSERT(pFormResources);
    auto pFonts
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pFormResources->LookupElement("Font"));
    CPPUNIT_ASSERT(pFonts);
    auto pF1Ref = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFonts->LookupElement("F1"));
    CPPUNIT_ASSERT(pF1Ref);
    vcl::filter::PDFObjectElement* pF1 = pF1Ref->LookupObject();
    CPPUNIT_ASSERT(pF1);

    // Check that Foo -> Bar of the font is of type Pages.
    auto pFontFoo = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pF1->Lookup("Foo"));
    CPPUNIT_ASSERT(pFontFoo);
    auto pBar = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFontFoo->LookupElement("Bar"));
    CPPUNIT_ASSERT(pBar);
    vcl::filter::PDFObjectElement* pObject = pBar->LookupObject();
    CPPUNIT_ASSERT(pObject);
    auto pName = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
    CPPUNIT_ASSERT(pName);
    // This was "XObject", reference in a nested dictionary wasn't updated when
    // copying the page stream of a PDF image.
    CPPUNIT_ASSERT_EQUAL(OString("Pages"), pName->GetValue());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf148706)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf148706.odt");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // The page has one annotation.
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnot = pPdfPage->getAnnotation(0);

    CPPUNIT_ASSERT(pAnnot->hasKey("V"));
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFObjectType::String, pAnnot->getValueType("V"));
    OUString aV = pAnnot->getString("V");

    // Without the fix in place, this test would have failed with
    // - Expected: 1821.84
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("1821.84"), aV);

    CPPUNIT_ASSERT(pAnnot->hasKey("DV"));
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFObjectType::String, pAnnot->getValueType("DV"));
    OUString aDV = pAnnot->getString("DV");

    CPPUNIT_ASSERT_EQUAL(OUString("1821.84"), aDV);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf107089)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf107089.odt", aDocument);

    // Get access to the only image on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Get access to the form object inside the image.
    auto pXObjectResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"));
    CPPUNIT_ASSERT(pXObjectResources);
    auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
        pXObjectResources->LookupElement("XObject"));
    CPPUNIT_ASSERT(pXObjectForms);
    vcl::filter::PDFObjectElement* pForm
        = pXObjectForms->LookupObject(pXObjectForms->GetItems().begin()->first);
    CPPUNIT_ASSERT(pForm);

    // Make sure 'Hello' is part of the form object's stream.
    vcl::filter::PDFStreamElement* pStream = pForm->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream aObjectStream;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    pStream->GetMemory().Seek(0);
    aZCodec.Decompress(pStream->GetMemory(), aObjectStream);
    CPPUNIT_ASSERT(aZCodec.EndCompression());
    aObjectStream.Seek(0);
    OString aHello("Hello");
    auto pStart = static_cast<const char*>(aObjectStream.GetData());
    const char* pEnd = pStart + aObjectStream.GetSize();
    auto it = std::search(pStart, pEnd, aHello.getStr(), aHello.getStr() + aHello.getLength());
    // This failed, 'Hello' was part only a mixed compressed/uncompressed stream, i.e. garbage.
    CPPUNIT_ASSERT(it != pEnd);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf99680)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf99680.odt", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // The page 1 has a stream.
    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents");
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

    // tdf#130150 See infos in task - short: tdf#99680 was not the
    // correct fix, so empty clip regions are valid - allow again in tests
    //      Make sure there are no empty clipping regions.
    //      OString aEmptyRegion("0 0 m h W* n");
    //      auto it = std::search(pStart, pEnd, aEmptyRegion.getStr(), aEmptyRegion.getStr() + aEmptyRegion.getLength());
    //      CPPUNIT_ASSERT_EQUAL_MESSAGE("Empty clipping region detected!", it, pEnd);

    // Count save graphic state (q) and restore (Q) operators
    // and ensure their amount is equal
    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* pEnd = pStart + aUncompressed.GetSize();
    size_t nSaveCount = std::count(pStart, pEnd, 'q');
    size_t nRestoreCount = std::count(pStart, pEnd, 'Q');
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Save/restore graphic state operators count mismatch!", nSaveCount,
                                 nRestoreCount);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf99680_2)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf99680-2.odt", aDocument);

    // For each document page
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aPages.size());
    for (size_t nPageNr = 0; nPageNr < aPages.size(); nPageNr++)
    {
        // Get page contents and stream.
        vcl::filter::PDFObjectElement* pContents = aPages[nPageNr]->LookupObject("Contents");
        CPPUNIT_ASSERT(pContents);
        vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
        CPPUNIT_ASSERT(pStream);
        SvMemoryStream& rObjectStream = pStream->GetMemory();

        // Uncompress the stream.
        SvMemoryStream aUncompressed;
        ZCodec aZCodec;
        aZCodec.BeginCompression();
        rObjectStream.Seek(0);
        aZCodec.Decompress(rObjectStream, aUncompressed);
        CPPUNIT_ASSERT(aZCodec.EndCompression());

        // tdf#130150 See infos in task - short: tdf#99680 was not the
        // correct fix, so empty clip regions are valid - allow again in tests
        //      Make sure there are no empty clipping regions.
        //      OString aEmptyRegion("0 0 m h W* n");
        //      auto it = std::search(pStart, pEnd, aEmptyRegion.getStr(), aEmptyRegion.getStr() + aEmptyRegion.getLength());
        //      CPPUNIT_ASSERT_EQUAL_MESSAGE("Empty clipping region detected!", it, pEnd);

        // Count save graphic state (q) and restore (Q) operators
        // and ensure their amount is equal
        auto pStart = static_cast<const char*>(aUncompressed.GetData());
        const char* pEnd = pStart + aUncompressed.GetSize();
        size_t nSaveCount = std::count(pStart, pEnd, 'q');
        size_t nRestoreCount = std::count(pStart, pEnd, 'Q');
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Save/restore graphic state operators count mismatch!",
                                     nSaveCount, nRestoreCount);
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf108963)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf108963.odp");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Test page size (28x15.75 cm, was 1/100th mm off, tdf#112690)
    // bad: MediaBox[0 0 793.672440944882 446.428346456693]
    // good: MediaBox[0 0 793.700787401575 446.456692913386]
    const double aWidth = pPdfPage->getWidth();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(793.7, aWidth, 0.01);
    const double aHeight = pPdfPage->getHeight();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(446.46, aHeight, 0.01);

    // Make sure there is a filled rectangle inside.
    int nPageObjectCount = pPdfPage->getObjectCount();
    int nYellowPathCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPdfPageObject = pPdfPage->getObject(i);
        if (pPdfPageObject->getType() != vcl::pdf::PDFPageObjectType::Path)
            continue;

        if (pPdfPageObject->getFillColor() == COL_YELLOW)
        {
            ++nYellowPathCount;
            // The path described a yellow rectangle, but it was not rotated.
            int nSegments = pPdfPageObject->getPathSegmentCount();
            CPPUNIT_ASSERT_EQUAL(5, nSegments);
            std::unique_ptr<vcl::pdf::PDFiumPathSegment> pSegment
                = pPdfPageObject->getPathSegment(0);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Moveto, pSegment->getType());
            basegfx::B2DPoint aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(245.367, aPoint.getX(), 0.0005);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(244.232, aPoint.getY(), 0.0005);
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(1);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(275.074, aPoint.getX(), 0.0005);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(267.590, aPoint.getY(), 0.0005);
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(2);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(287.490, aPoint.getX(), 0.0005);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(251.801, aPoint.getY(), 0.0005);
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(3);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(257.811, aPoint.getX(), 0.0005);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(228.443, aPoint.getY(), 0.0005);
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(4);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(245.367, aPoint.getX(), 0.0005);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(244.232, aPoint.getY(), 0.0005);
            CPPUNIT_ASSERT(pSegment->isClosed());
        }
    }

    CPPUNIT_ASSERT_EQUAL(1, nYellowPathCount);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testAlternativeText)
{
    aMediaDescriptor["FilterName"] <<= OUString("impress_pdf_Export");

    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "UseTaggedPDF", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"alternativeText.fodp");

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"));
            if (pS && pS->GetValue() == "Figure")
            {
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"This is the text alternative - This is the description"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                        *dynamic_cast<vcl::filter::PDFHexStringElement*>(pObject->Lookup("Alt"))));
            }
        }
    }

    // tdf#67866 check that Catalog contains Lang
    auto* pCatalog = aDocument.GetCatalog();
    CPPUNIT_ASSERT(pCatalog);
    auto* pCatalogDictionary = pCatalog->GetDictionary();
    CPPUNIT_ASSERT(pCatalogDictionary);
    auto pLang = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
        pCatalogDictionary->LookupElement("Lang"));
    CPPUNIT_ASSERT(pLang);
    CPPUNIT_ASSERT_EQUAL(OString("en-US"), pLang->GetValue());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf105972)
{
    vcl::filter::PDFDocument aDocument;
    // Loading fails with tagged PDF enabled
    load(u"tdf105972.fodt", aDocument, false);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pAnnots->GetElements().size());

    sal_uInt32 nTextFieldCount = 0;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("FT"));
        if (pType && pType->GetValue() == "Tx")
        {
            ++nTextFieldCount;

            auto pT = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pObject->Lookup("T"));
            CPPUNIT_ASSERT(pT);
            auto pAA = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject->Lookup("AA"));
            CPPUNIT_ASSERT(pAA);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pAA->GetItems().size());
            auto pF = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAA->LookupElement("F"));
            CPPUNIT_ASSERT(pF);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pF->GetItems().size());

            if (nTextFieldCount == 1)
            {
                CPPUNIT_ASSERT_EQUAL(OString("CurrencyField"), pT->GetValue());

                auto pJS
                    = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pF->LookupElement("JS"));
                CPPUNIT_ASSERT_EQUAL(
                    OString("AFNumber_Format\\(4, 0, 0, 0, \"\\\\u20ac\",true\\);"),
                    pJS->GetValue());
            }
            else if (nTextFieldCount == 2)
            {
                CPPUNIT_ASSERT_EQUAL(OString("TimeField"), pT->GetValue());

                auto pJS
                    = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pF->LookupElement("JS"));
                CPPUNIT_ASSERT_EQUAL(OString("AFTime_FormatEx\\(\"h:MM:sstt\"\\);"),
                                     pJS->GetValue());
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL(OString("DateField"), pT->GetValue());

                auto pJS
                    = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pF->LookupElement("JS"));
                CPPUNIT_ASSERT_EQUAL(OString("AFDate_FormatEx\\(\"yy-mm-dd\"\\);"),
                                     pJS->GetValue());
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf148442)
{
    vcl::filter::PDFDocument aDocument;
    // Loading fails with tagged PDF enabled
    load(u"tdf148442.odt", aDocument, false);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pAnnots->GetElements().size());

    sal_uInt32 nBtnCount = 0;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("FT"));
        if (pType && pType->GetValue() == "Btn")
        {
            ++nBtnCount;
            auto pT = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pObject->Lookup("T"));
            CPPUNIT_ASSERT(pT);
            auto pAS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("AS"));
            CPPUNIT_ASSERT(pAS);

            auto pAP = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject->Lookup("AP"));
            CPPUNIT_ASSERT(pAP);
            auto pN = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAP->LookupElement("N"));
            CPPUNIT_ASSERT(pN);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pN->GetItems().size());

            if (nBtnCount == 1)
            {
                CPPUNIT_ASSERT_EQUAL(OString("Checkbox1"), pT->GetValue());
                CPPUNIT_ASSERT_EQUAL(OString("Yes"), pAS->GetValue());
                CPPUNIT_ASSERT(!pN->GetItems().count("ref"));
                CPPUNIT_ASSERT(pN->GetItems().count("Yes"));
                CPPUNIT_ASSERT(pN->GetItems().count("Off"));
            }
            else if (nBtnCount == 2)
            {
                CPPUNIT_ASSERT_EQUAL(OString("Checkbox2"), pT->GetValue());
                CPPUNIT_ASSERT_EQUAL(OString("Yes"), pAS->GetValue());

                // Without the fix in place, this test would have failed here
                CPPUNIT_ASSERT(pN->GetItems().count("ref"));
                CPPUNIT_ASSERT(!pN->GetItems().count("Yes"));
                CPPUNIT_ASSERT(pN->GetItems().count("Off"));
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL(OString("Checkbox3"), pT->GetValue());
                CPPUNIT_ASSERT_EQUAL(OString("Off"), pAS->GetValue());
                CPPUNIT_ASSERT(pN->GetItems().count("ref"));
                CPPUNIT_ASSERT(!pN->GetItems().count("Yes"));

                // tdf#143612: Without the fix in place, this test would have failed here
                CPPUNIT_ASSERT(!pN->GetItems().count("Off"));
                CPPUNIT_ASSERT(pN->GetItems().count("refOff"));
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf118244_radioButtonGroup)
{
    vcl::filter::PDFDocument aDocument;
    // Loading fails with tagged PDF enabled
    load(u"tdf118244_radioButtonGroup.odt", aDocument, false);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // There are eight radio buttons.
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of radio buttons", static_cast<size_t>(8),
                                 pAnnots->GetElements().size());

    sal_uInt32 nRadioGroups = 0;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("FT"));
        if (pType && pType->GetValue() == "Btn")
        {
            auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject->Lookup("Kids"));
            if (pKids)
            {
                size_t expectedSize = 2;
                ++nRadioGroups;
                if (nRadioGroups == 3)
                    expectedSize = 3;
                CPPUNIT_ASSERT_EQUAL(expectedSize, pKids->GetElements().size());
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of radio groups", sal_uInt32(3), nRadioGroups);
}

/// Test writing ToUnicode CMAP for LTR ligatures.
// This requires Carlito font, if it is missing the test will most likely
// fail.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf115117_1)
{
#if HAVE_MORE_FONTS
    vcl::filter::PDFDocument aDocument;
    load(u"tdf115117-1.odt", aDocument);

    vcl::filter::PDFObjectElement* pToUnicode = nullptr;

    // Get access to ToUnicode of the first font
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
        if (pType && pType->GetValue() == "Font")
        {
            auto pToUnicodeRef
                = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("ToUnicode"));
            CPPUNIT_ASSERT(pToUnicodeRef);
            pToUnicode = pToUnicodeRef->LookupObject();
            break;
        }
    }

    CPPUNIT_ASSERT(pToUnicode);
    auto pStream = pToUnicode->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream aObjectStream;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    pStream->GetMemory().Seek(0);
    aZCodec.Decompress(pStream->GetMemory(), aObjectStream);
    CPPUNIT_ASSERT(aZCodec.EndCompression());
    aObjectStream.Seek(0);
    // The first values, <01> <02> etc., are glyph ids, they might change order
    // if we changed how font subsets are created.
    // The second values, <00740069> etc., are Unicode code points in hex,
    // <00740069> is U+0074 and U+0069 i.e. "ti" which is a ligature in
    // Carlito/Calibri. This test is failing if any of the second values
    // changed which means we are not detecting ligatures and writing CMAP
    // entries for them correctly. If glyph order in the subset changes then
    // the order here will changes and the PDF has to be carefully inspected to
    // ensure that the new values are correct before updating the string below.
    OString aCmap("9 beginbfchar\n"
                  "<01> <00740069>\n"
                  "<02> <0020>\n"
                  "<03> <0074>\n"
                  "<04> <0065>\n"
                  "<05> <0073>\n"
                  "<06> <00660069>\n"
                  "<07> <0066006C>\n"
                  "<08> <006600660069>\n"
                  "<09> <00660066006C>\n"
                  "endbfchar");
    auto pStart = static_cast<const char*>(aObjectStream.GetData());
    const char* pEnd = pStart + aObjectStream.GetSize();
    auto it = std::search(pStart, pEnd, aCmap.getStr(), aCmap.getStr() + aCmap.getLength());
    CPPUNIT_ASSERT(it != pEnd);
#endif
}

/// Test writing ToUnicode CMAP for RTL ligatures.
// This requires DejaVu Sans font, if it is missing the test will most likely
// fail.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf115117_2)
{
#if HAVE_MORE_FONTS
    // See the comments in testTdf115117_1() for explanation.

    vcl::filter::PDFDocument aDocument;
    load(u"tdf115117-2.odt", aDocument);

    vcl::filter::PDFObjectElement* pToUnicode = nullptr;

    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
        if (pType && pType->GetValue() == "Font")
        {
            auto pToUnicodeRef
                = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("ToUnicode"));
            CPPUNIT_ASSERT(pToUnicodeRef);
            pToUnicode = pToUnicodeRef->LookupObject();
            break;
        }
    }

    CPPUNIT_ASSERT(pToUnicode);
    auto pStream = pToUnicode->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream aObjectStream;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    pStream->GetMemory().Seek(0);
    aZCodec.Decompress(pStream->GetMemory(), aObjectStream);
    CPPUNIT_ASSERT(aZCodec.EndCompression());
    aObjectStream.Seek(0);
    OString aCmap("7 beginbfchar\n"
                  "<01> <06440627>\n"
                  "<02> <0020>\n"
                  "<03> <0641>\n"
                  "<04> <0642>\n"
                  "<05> <0648>\n"
                  "<06> <06440627>\n"
                  "<07> <0628>\n"
                  "endbfchar");
    auto pStart = static_cast<const char*>(aObjectStream.GetData());
    const char* pEnd = pStart + aObjectStream.GetSize();
    auto it = std::search(pStart, pEnd, aCmap.getStr(), aCmap.getStr() + aCmap.getLength());
    CPPUNIT_ASSERT(it != pEnd);
#endif
}

/// Text extracting LTR text with ligatures.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf115117_1a)
{
#if HAVE_MORE_FONTS
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf115117-1.odt");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pPdfTextPage);

    // Extract the text from the page. This pdfium API is a bit higher level
    // than we want and might apply heuristic that give false positive, but it
    // is a good approximation in addition to the check in testTdf115117_1().
    int nChars = pPdfTextPage->countChars();
    CPPUNIT_ASSERT_EQUAL(44, nChars);

    std::vector<sal_uInt32> aChars(nChars);
    for (int i = 0; i < nChars; i++)
        aChars[i] = pPdfTextPage->getUnicode(i);
    OUString aActualText(aChars.data(), aChars.size());
    CPPUNIT_ASSERT_EQUAL(OUString("ti ti test ti\r\nti test fi fl ffi ffl test fi"), aActualText);
#endif
}

/// Test extracting RTL text with ligatures.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf115117_2a)
{
#if HAVE_MORE_FONTS
    // See the comments in testTdf115117_1a() for explanation.

    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf115117-2.odt");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pPdfTextPage);

    int nChars = pPdfTextPage->countChars();
    CPPUNIT_ASSERT_EQUAL(13, nChars);

    std::vector<sal_uInt32> aChars(nChars);
    for (int i = 0; i < nChars; i++)
        aChars[i] = pPdfTextPage->getUnicode(i);
    OUString aActualText(aChars.data(), aChars.size());
    CPPUNIT_ASSERT_EQUAL(
        OUString(u"\u0627\u0644 \u0628\u0627\u0644 \u0648\u0642\u0641 \u0627\u0644"), aActualText);
#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf154549)
{
// FIXME: On Windows, the number of chars is 4 instead of 3
#ifndef _WIN32
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf154549.odt");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pPdfTextPage);

    int nChars = pPdfTextPage->countChars();

    CPPUNIT_ASSERT_EQUAL(3, nChars);

    std::vector<sal_uInt32> aChars(nChars);
    for (int i = 0; i < nChars; i++)
        aChars[i] = pPdfTextPage->getUnicode(i);
    OUString aActualText(aChars.data(), aChars.size());

    // Without the fix in place, this test would have failed with
    // - Expected: ִبي
    // - Actual  : بִي
    CPPUNIT_ASSERT_EQUAL(OUString(u"\u05B4\u0628\u064A"), aActualText);
#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf150846)
{
    // Without the fix in place, this test would have failed with
    // An uncaught exception of type com.sun.star.io.IOException
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf150846.txt");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pPdfTextPage);

    int nChars = pPdfTextPage->countChars();

    CPPUNIT_ASSERT_EQUAL(5, nChars);

    std::vector<sal_uInt32> aChars(nChars);
    for (int i = 0; i < nChars; i++)
        aChars[i] = pPdfTextPage->getUnicode(i);
    OUString aActualText(aChars.data(), aChars.size());
    CPPUNIT_ASSERT_EQUAL(OUString(u"hello"), aActualText);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf103492)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf103492.odt");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has two page.
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage1 = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage1);

    std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage1 = pPdfPage1->getTextPage();
    CPPUNIT_ASSERT(pPdfTextPage1);

    int nChars1 = pPdfTextPage1->countChars();

    // Without the fix in place, this test would have failed with
    // - Expected: 15
    // - Actual  : 18
    CPPUNIT_ASSERT_EQUAL(15, nChars1);

    std::vector<sal_uInt32> aChars1(nChars1);
    for (int i = 0; i < nChars1; i++)
        aChars1[i] = pPdfTextPage1->getUnicode(i);
    OUString aActualText1(aChars1.data(), aChars1.size());
    CPPUNIT_ASSERT_EQUAL(OUString(u"يوسف My name is"), aActualText1);

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage2 = pPdfDocument->openPage(/*nIndex=*/1);
    CPPUNIT_ASSERT(pPdfPage2);

    std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage2 = pPdfPage2->getTextPage();
    CPPUNIT_ASSERT(pPdfTextPage2);

    int nChars2 = pPdfTextPage2->countChars();

    CPPUNIT_ASSERT_EQUAL(15, nChars2);

    std::vector<sal_uInt32> aChars2(nChars2);
    for (int i = 0; i < nChars2; i++)
        aChars2[i] = pPdfTextPage2->getUnicode(i);
    OUString aActualText2(aChars2.data(), aChars2.size());
    CPPUNIT_ASSERT_EQUAL(OUString(u"My name is يوسف"), aActualText2);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf145274)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf145274.docx");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    auto pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    int nPageObjectCount = pPage->getObjectCount();

    // Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : 4
    CPPUNIT_ASSERT_EQUAL(6, nPageObjectCount);

    auto pTextPage = pPage->getTextPage();

    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Text)
            continue;

        CPPUNIT_ASSERT_EQUAL(11.0, pPageObject->getFontSize());
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFTextRenderMode::Fill, pPageObject->getTextRenderMode());
        CPPUNIT_ASSERT_EQUAL(COL_RED, pPageObject->getFillColor());
    }
}

/// Test writing ToUnicode CMAP for doubly encoded glyphs.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf66597_1)
{
#if HAVE_MORE_FONTS
    // This requires Amiri font, if it is missing the test will fail.
    vcl::filter::PDFDocument aDocument;
    load(u"tdf66597-1.odt", aDocument);

    {
        // Get access to ToUnicode of the first font
        vcl::filter::PDFObjectElement* pToUnicode = nullptr;
        for (const auto& aElement : aDocument.GetElements())
        {
            auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
            if (!pObject)
                continue;
            auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
            if (pType && pType->GetValue() == "Font")
            {
                auto pName
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("BaseFont"));
                auto aName = pName->GetValue().copy(7); // skip the subset id
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected font name", OString("Amiri-Regular"),
                                             aName);

                auto pToUnicodeRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("ToUnicode"));
                CPPUNIT_ASSERT(pToUnicodeRef);
                pToUnicode = pToUnicodeRef->LookupObject();
                break;
            }
        }

        CPPUNIT_ASSERT(pToUnicode);
        auto pStream = pToUnicode->GetStream();
        CPPUNIT_ASSERT(pStream);
        SvMemoryStream aObjectStream;
        ZCodec aZCodec;
        aZCodec.BeginCompression();
        pStream->GetMemory().Seek(0);
        aZCodec.Decompress(pStream->GetMemory(), aObjectStream);
        CPPUNIT_ASSERT(aZCodec.EndCompression());
        aObjectStream.Seek(0);
        // The <01> is glyph id, <2044> is code point.
        // The document has two characters <2044><2215><2044>, but the font
        // reuses the same glyph for U+2044 and U+2215 so we should have a single
        // CMAP entry for the U+2044, and U+2215 will be handled with ActualText
        // (tested below).
        std::string aCmap("1 beginbfchar\n"
                          "<01> <2044>\n"
                          "endbfchar");
        std::string aData(static_cast<const char*>(aObjectStream.GetData()),
                          aObjectStream.GetSize());
        auto nPos = aData.find(aCmap);
        CPPUNIT_ASSERT(nPos != std::string::npos);
    }

    {
        auto aPages = aDocument.GetPages();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
        // Get page contents and stream.
        auto pContents = aPages[0]->LookupObject("Contents");
        CPPUNIT_ASSERT(pContents);
        auto pStream = pContents->GetStream();
        CPPUNIT_ASSERT(pStream);
        auto& rObjectStream = pStream->GetMemory();

        // Uncompress the stream.
        SvMemoryStream aUncompressed;
        ZCodec aZCodec;
        aZCodec.BeginCompression();
        rObjectStream.Seek(0);
        aZCodec.Decompress(rObjectStream, aUncompressed);
        CPPUNIT_ASSERT(aZCodec.EndCompression());

        // Make sure the expected ActualText is present.
        std::string aData(static_cast<const char*>(aUncompressed.GetData()),
                          aUncompressed.GetSize());

        std::string aActualText("/Span<</ActualText<");
        size_t nCount = 0;
        size_t nPos = 0;
        while ((nPos = aData.find(aActualText, nPos)) != std::string::npos)
        {
            nCount++;
            nPos += aActualText.length();
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("The should be one ActualText entry!", static_cast<size_t>(1),
                                     nCount);

        aActualText = "/Span<</ActualText<FEFF2215>>>";
        nPos = aData.find(aActualText);
        CPPUNIT_ASSERT_MESSAGE("ActualText not found!", nPos != std::string::npos);
    }
#endif
}

/// Test writing ActualText for RTL many to one glyph to Unicode mapping.
// This requires Reem Kufi font, if it is missing the test will fail.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf66597_2)
{
#if HAVE_MORE_FONTS
    vcl::filter::PDFDocument aDocument;
    load(u"tdf66597-2.odt", aDocument);

    {
        // Get access to ToUnicode of the first font
        vcl::filter::PDFObjectElement* pToUnicode = nullptr;
        for (const auto& aElement : aDocument.GetElements())
        {
            auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
            if (!pObject)
                continue;
            auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
            if (pType && pType->GetValue() == "Font")
            {
                auto pName
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("BaseFont"));
                auto aName = pName->GetValue().copy(7); // skip the subset id
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected font name", OString("ReemKufi-Regular"),
                                             aName);

                auto pToUnicodeRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("ToUnicode"));
                CPPUNIT_ASSERT(pToUnicodeRef);
                pToUnicode = pToUnicodeRef->LookupObject();
                break;
            }
        }

        CPPUNIT_ASSERT(pToUnicode);
        auto pStream = pToUnicode->GetStream();
        CPPUNIT_ASSERT(pStream);
        SvMemoryStream aObjectStream;
        ZCodec aZCodec;
        aZCodec.BeginCompression();
        pStream->GetMemory().Seek(0);
        aZCodec.Decompress(pStream->GetMemory(), aObjectStream);
        CPPUNIT_ASSERT(aZCodec.EndCompression());
        aObjectStream.Seek(0);
        std::string aCmap("8 beginbfchar\n"
                          "<02> <0632>\n"
                          "<03> <0020>\n"
                          "<04> <0648>\n"
                          "<05> <0647>\n"
                          "<06> <062F>\n"
                          "<08> <062C>\n"
                          "<0A> <0628>\n"
                          "<0C> <0623>\n"
                          "endbfchar");
        std::string aData(static_cast<const char*>(aObjectStream.GetData()),
                          aObjectStream.GetSize());
        auto nPos = aData.find(aCmap);
        CPPUNIT_ASSERT(nPos != std::string::npos);
    }

    {
        auto aPages = aDocument.GetPages();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
        // Get page contents and stream.
        auto pContents = aPages[0]->LookupObject("Contents");
        CPPUNIT_ASSERT(pContents);
        auto pStream = pContents->GetStream();
        CPPUNIT_ASSERT(pStream);
        auto& rObjectStream = pStream->GetMemory();

        // Uncompress the stream.
        SvMemoryStream aUncompressed;
        ZCodec aZCodec;
        aZCodec.BeginCompression();
        rObjectStream.Seek(0);
        aZCodec.Decompress(rObjectStream, aUncompressed);
        CPPUNIT_ASSERT(aZCodec.EndCompression());

        // Make sure the expected ActualText is present.
        std::string aData(static_cast<const char*>(aUncompressed.GetData()),
                          aUncompressed.GetSize());

        std::vector<std::string> aCodes({ "0632", "062C", "0628", "0623" });
        std::string aActualText("/Span<</ActualText<");
        size_t nCount = 0;
        size_t nPos = 0;
        while ((nPos = aData.find(aActualText, nPos)) != std::string::npos)
        {
            nCount++;
            nPos += aActualText.length();
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of ActualText entries does not match!", aCodes.size(),
                                     nCount);

        for (const auto& aCode : aCodes)
        {
            aActualText = "/Span<</ActualText<FEFF" + aCode + ">>>";
            nPos = aData.find(aActualText);
            CPPUNIT_ASSERT_MESSAGE("ActualText not found for " + aCode, nPos != std::string::npos);
        }
    }
#endif
}

/// Test writing ActualText for LTR many to one glyph to Unicode mapping.
// This requires Gentium Basic font, if it is missing the test will fail.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf66597_3)
{
#if HAVE_MORE_FONTS
    vcl::filter::PDFDocument aDocument;
    load(u"tdf66597-3.odt", aDocument);

    {
        // Get access to ToUnicode of the first font
        vcl::filter::PDFObjectElement* pToUnicode = nullptr;
        for (const auto& aElement : aDocument.GetElements())
        {
            auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
            if (!pObject)
                continue;
            auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
            if (pType && pType->GetValue() == "Font")
            {
                auto pName
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("BaseFont"));
                auto aName = pName->GetValue().copy(7); // skip the subset id
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected font name", OString("GentiumBasic"),
                                             aName);

                auto pToUnicodeRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("ToUnicode"));
                CPPUNIT_ASSERT(pToUnicodeRef);
                pToUnicode = pToUnicodeRef->LookupObject();
                break;
            }
        }

        CPPUNIT_ASSERT(pToUnicode);
        auto pStream = pToUnicode->GetStream();
        CPPUNIT_ASSERT(pStream);
        SvMemoryStream aObjectStream;
        ZCodec aZCodec;
        aZCodec.BeginCompression();
        pStream->GetMemory().Seek(0);
        aZCodec.Decompress(pStream->GetMemory(), aObjectStream);
        CPPUNIT_ASSERT(aZCodec.EndCompression());
        aObjectStream.Seek(0);
        std::string aCmap("2 beginbfchar\n"
                          "<01> <1ECB0331030B>\n"
                          "<05> <0020>\n"
                          "endbfchar");
        std::string aData(static_cast<const char*>(aObjectStream.GetData()),
                          aObjectStream.GetSize());
        auto nPos = aData.find(aCmap);
        CPPUNIT_ASSERT(nPos != std::string::npos);
    }

    {
        auto aPages = aDocument.GetPages();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
        // Get page contents and stream.
        auto pContents = aPages[0]->LookupObject("Contents");
        CPPUNIT_ASSERT(pContents);
        auto pStream = pContents->GetStream();
        CPPUNIT_ASSERT(pStream);
        auto& rObjectStream = pStream->GetMemory();

        // Uncompress the stream.
        SvMemoryStream aUncompressed;
        ZCodec aZCodec;
        aZCodec.BeginCompression();
        rObjectStream.Seek(0);
        aZCodec.Decompress(rObjectStream, aUncompressed);
        CPPUNIT_ASSERT(aZCodec.EndCompression());

        // Make sure the expected ActualText is present.
        std::string aData(static_cast<const char*>(aUncompressed.GetData()),
                          aUncompressed.GetSize());

        std::string aActualText("/Span<</ActualText<FEFF1ECB0331030B>>>");
        size_t nCount = 0;
        size_t nPos = 0;
        while ((nPos = aData.find(aActualText, nPos)) != std::string::npos)
        {
            nCount++;
            nPos += aActualText.length();
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of ActualText entries does not match!",
                                     static_cast<size_t>(4), nCount);
    }
#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf105954)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "ReduceImageResolution", uno::Any(true) },
          { "MaxImageResolution", uno::Any(static_cast<sal_Int32>(300)) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"tdf105954.odt");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // There is a single image on the page.
    int nPageObjectCount = pPdfPage->getObjectCount();
    CPPUNIT_ASSERT_EQUAL(1, nPageObjectCount);

    // Check width of the image.
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(/*index=*/0);
    Size aMeta = pPageObject->getImageSize(*pPdfPage);
    // This was 2000, i.e. the 'reduce to 300 DPI' request was ignored.
    // This is now around 238 (228 on macOS).
    CPPUNIT_ASSERT_LESS(static_cast<tools::Long>(250), aMeta.getWidth());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf157679)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("impress_pdf_Export");
    saveAsPDF(u"tdf157679.pptx");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Without the fix in place, this test would have failed with
    // - Expected: 3
    // - Actual  : 5
    CPPUNIT_ASSERT_EQUAL(3, pPdfPage->getObjectCount());

    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    int nPageObjectCount = pPdfPage->getObjectCount();
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        // Check there are not Text objects
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);
        CPPUNIT_ASSERT(pPageObject->getType() != vcl::pdf::PDFPageObjectType::Text);
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf128445)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("impress_pdf_Export");
    saveAsPDF(u"tdf128445.odp");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Without the fix in place, this test would have failed with
    // - Expected: 7
    // - Actual  : 6
    CPPUNIT_ASSERT_EQUAL(7, pPdfPage->getObjectCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf128630)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("impress_pdf_Export");
    saveAsPDF(u"tdf128630.odp");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Assert the size of the only bitmap on the page.
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
        int nWidth = pBitmap->getWidth();
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 466
        // - Actual  : 289
        // i.e. the rotated + scaled arrow was more thin than it should be.
        CPPUNIT_ASSERT_DOUBLES_EQUAL(466, nWidth, 1);
        int nHeight = pBitmap->getHeight();
        CPPUNIT_ASSERT_EQUAL(nWidth, nHeight);
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf106702)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf106702.odt");

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has two pages.
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());

    // First page already has the correct image position.
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    int nExpected = 0;
    int nPageObjectCount = pPdfPage->getObjectCount();
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Image)
            continue;

        // Top, but upside down.
        nExpected = pPageObject->getBounds().getMaxY();
        break;
    }

    // Second page had an incorrect image position.
    pPdfPage = pPdfDocument->openPage(/*nIndex=*/1);
    CPPUNIT_ASSERT(pPdfPage);
    int nActual = 0;
    nPageObjectCount = pPdfPage->getObjectCount();
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Image)
            continue;

        // Top, but upside down.
        nActual = pPageObject->getBounds().getMaxY();
        break;
    }

    // This failed, vertical pos is 818 points, was 1674 (outside visible page
    // bounds).
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf113143)
{
    aMediaDescriptor["FilterName"] <<= OUString("impress_pdf_Export");
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "ExportNotesPages", uno::Any(true) },
        // ReduceImageResolution is on by default and that hides the bug we
        // want to test.
        { "ReduceImageResolution", uno::Any(false) },
        // Set a custom PDF version.
        { "SelectPdfVersion", uno::Any(static_cast<sal_Int32>(16)) },
    }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"tdf113143.odp");

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // The document has two pages.
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());

    // First has the original (larger) image.
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    int nLarger = 0;
    int nPageObjectCount = pPdfPage->getObjectCount();
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Image)
            continue;

        nLarger = pPageObject->getBounds().getWidth();
        break;
    }

    // Second page has the scaled (smaller) image.
    pPdfPage = pPdfDocument->openPage(/*nIndex=*/1);
    CPPUNIT_ASSERT(pPdfPage);
    int nSmaller = 0;
    nPageObjectCount = pPdfPage->getObjectCount();
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Image)
            continue;

        nSmaller = pPageObject->getBounds().getWidth();
        break;
    }

    // This failed, both were 319, now nSmaller is 169.
    CPPUNIT_ASSERT_LESS(nLarger, nSmaller);

    // The following check used to fail in the past, header was "%PDF-1.5":
    maMemory.Seek(0);
    OString aExpectedHeader("%PDF-1.6");
    OString aHeader(read_uInt8s_ToOString(maMemory, aExpectedHeader.getLength()));
    CPPUNIT_ASSERT_EQUAL(aExpectedHeader, aHeader);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testForcePoint71)
{
    // I just care it doesn't crash
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"forcepoint71.key");
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testForcePoint80)
{
    // printing asserted in SwCellFrame::FindStartEndOfRowSpanCell
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"forcepoint80-1.rtf");
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testForcePoint3)
{
    // printing asserted in SwFrame::GetNextSctLeaf()
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"flowframe_null_ptr_deref.sample");
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf84283)
{
    // Without the fix in place, this test would have crashed
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf84283.doc");
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf115262)
{
    aMediaDescriptor["FilterName"] <<= OUString("calc_pdf_Export");
    saveAsPDF(u"tdf115262.ods");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(8, pPdfDocument->getPageCount());

    // Get the 6th page.
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/5);
    CPPUNIT_ASSERT(pPdfPage);

    // Look up the position of the first image and the 400th row.
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();
    int nPageObjectCount = pPdfPage->getObjectCount();
    int nFirstImageTop = 0;
    int nRowTop = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);
        // Top, but upside down.
        float fTop = pPageObject->getBounds().getMaxY();

        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Image)
        {
            nFirstImageTop = fTop;
        }
        else if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Text)
        {
            OUString sText = pPageObject->getText(pTextPage);
            if (sText == "400")
                nRowTop = fTop;
        }
    }
    // Make sure that the top of the "400" is below the top of the image (in
    // bottom-right-corner-based PDF coordinates).
    // This was: expected less than 144, actual is 199.
    CPPUNIT_ASSERT_LESS(nFirstImageTop, nRowTop);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf121962)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf121962.odt");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Get the first page
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();

    // Make sure the table sum is displayed as "0", not faulty expression.
    int nPageObjectCount = pPdfPage->getObjectCount();
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Text)
            continue;
        OUString sText = pPageObject->getText(pTextPage);
        CPPUNIT_ASSERT(sText != "** Expression is faulty **");
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf139065)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf139065.odt");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Without the fix in place, this test would have failed with
    // - Expected: 15
    // - Actual  : 6
    CPPUNIT_ASSERT_EQUAL(15, pPdfPage->getObjectCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf157816)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"tdf157816.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pDocument(nullptr);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"));
            if (pS1 && pS1->GetValue() == "Document")
            {
                pDocument = pObject1;
            }
        }
    }
    CPPUNIT_ASSERT(pDocument);

    auto pKidsD = dynamic_cast<vcl::filter::PDFArrayElement*>(pDocument->Lookup("K"));
    CPPUNIT_ASSERT(pKidsD);
    // assume there are no MCID ref at this level
    auto pKidsDv = pKidsD->GetElements();
    auto pRefKidD2 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsDv[2]);
    CPPUNIT_ASSERT(pRefKidD2);
    auto pObjectD2 = pRefKidD2->LookupObject();
    CPPUNIT_ASSERT(pObjectD2);
    auto pTypeD2 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD2->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD2->GetValue());
    auto pSD2 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD2->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Text#20body"), pSD2->GetValue());

    auto pKidsD2 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD2->Lookup("K"));
    CPPUNIT_ASSERT(pKidsD2);
    auto pKidsD2v = pKidsD2->GetElements();
    auto pRefKidD20 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD2v[0]);
    // MCID for text
    CPPUNIT_ASSERT(!pRefKidD20);
    auto pRefKidD21 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD2v[1]);
    // MCID for text
    CPPUNIT_ASSERT(!pRefKidD21);

    auto pRefKidD22 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD2v[2]);
    CPPUNIT_ASSERT(pRefKidD22);
    auto pObjectD22 = pRefKidD22->LookupObject();
    CPPUNIT_ASSERT(pObjectD22);
    auto pTypeD22 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD22->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD22->GetValue());
    auto pSD22 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD22->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD22->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD22->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"Error: Reference source not found"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(95.143, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(674.589, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                // this changed to the end of the text, not the start of the fly
                CPPUNIT_ASSERT_DOUBLES_EQUAL(187.207, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(688.389, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD23 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD2v[3]);
    CPPUNIT_ASSERT(pRefKidD23);
    auto pObjectD23 = pRefKidD23->LookupObject();
    CPPUNIT_ASSERT(pObjectD23);
    auto pTypeD23 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD23->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD23->GetValue());
    auto pSD23 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD23->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD23->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD23->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"Error: Reference source not found"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.693, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(660.789, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(146.157, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(674.589, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD24 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD2v[4]);
    CPPUNIT_ASSERT(pRefKidD24);
    auto pObjectD24 = pRefKidD24->LookupObject();
    CPPUNIT_ASSERT(pObjectD24);
    auto pTypeD24 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD24->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD24->GetValue());
    auto pSD24 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD24->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD24->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD24->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"Error: Reference source not found"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(146.093, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(660.789, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(179.457, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(674.589, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD25 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD2v[5]);
    CPPUNIT_ASSERT(pRefKidD25);
    auto pObjectD25 = pRefKidD25->LookupObject();
    CPPUNIT_ASSERT(pObjectD25);
    auto pTypeD25 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD25->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD25->GetValue());
    auto pSD25 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD25->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD25->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD25->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"Error: Reference source not found"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.693, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(646.989, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                // master had 174.757?
                CPPUNIT_ASSERT_DOUBLES_EQUAL(174.807, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(660.789, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD26 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD2v[6]);
    CPPUNIT_ASSERT(pRefKidD26);
    auto pObjectD26 = pRefKidD26->LookupObject();
    CPPUNIT_ASSERT(pObjectD26);
    auto pTypeD26 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD26->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD26->GetValue());
    auto pSD26 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD26->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD26->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD26->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"Error: Reference source not found"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.693, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(633.189, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(86.807, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(646.989, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD27 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD2v[7]);
    // MCID for text
    CPPUNIT_ASSERT(!pRefKidD27);

    // the problem was that in addition to the 5 links with SE there were 3 more
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), pAnnots->GetElements().size());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf157816Link)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"LinkWithFly.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pDocument(nullptr);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"));
            if (pS1 && pS1->GetValue() == "Document")
            {
                pDocument = pObject1;
            }
        }
    }
    CPPUNIT_ASSERT(pDocument);

    auto pKidsD = dynamic_cast<vcl::filter::PDFArrayElement*>(pDocument->Lookup("K"));
    CPPUNIT_ASSERT(pKidsD);
    // assume there are no MCID ref at this level
    auto pKidsDv = pKidsD->GetElements();
    auto pRefKidD0 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsDv[0]);
    CPPUNIT_ASSERT(pRefKidD0);
    auto pObjectD0 = pRefKidD0->LookupObject();
    CPPUNIT_ASSERT(pObjectD0);
    auto pTypeD0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD0->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD0->GetValue());
    auto pSD0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD0->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Standard"), pSD0->GetValue());

    auto pKidsD0 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD0->Lookup("K"));
    CPPUNIT_ASSERT(pKidsD0);
    auto pKidsD0v = pKidsD0->GetElements();

    auto pRefKidD00 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD0v[0]);
    CPPUNIT_ASSERT(pRefKidD00);
    auto pObjectD00 = pRefKidD00->LookupObject();
    CPPUNIT_ASSERT(pObjectD00);
    auto pTypeD00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD00->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD00->GetValue());
    auto pSD00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD00->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD00->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD00->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"https://www.mozilla.org/en-US/firefox/119.0/releasenotes/"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.693, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(771.389, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                // this changed to the end of the text, not the start of the fly
                CPPUNIT_ASSERT_DOUBLES_EQUAL(191.707, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(785.189, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD01 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD0v[1]);
    CPPUNIT_ASSERT(pRefKidD01);
    auto pObjectD01 = pRefKidD01->LookupObject();
    CPPUNIT_ASSERT(pObjectD01);
    auto pTypeD01 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD01->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD01->GetValue());
    auto pSD01 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD01->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD01->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD01->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"https://www.mozilla.org/en-US/firefox/119.0/releasenotes/"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(387.843, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(771.389, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                // this changed to the end of the text, not the start of the fly
                CPPUNIT_ASSERT_DOUBLES_EQUAL(534.407, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(785.189, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD02 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD0v[2]);
    CPPUNIT_ASSERT(pRefKidD02);
    auto pObjectD02 = pRefKidD02->LookupObject();
    CPPUNIT_ASSERT(pObjectD02);
    auto pTypeD02 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD02->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD02->GetValue());
    auto pSD02 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD02->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Figure"), pSD02->GetValue());

    auto pRefKidD1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsDv[1]);
    CPPUNIT_ASSERT(pRefKidD1);
    auto pObjectD1 = pRefKidD1->LookupObject();
    CPPUNIT_ASSERT(pObjectD1);
    auto pTypeD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD1->GetValue());
    auto pSD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Standard"), pSD1->GetValue());

    auto pKidsD1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD1->Lookup("K"));
    CPPUNIT_ASSERT(pKidsD1);
    auto pKidsD1v = pKidsD1->GetElements();

    auto pRefKidD10 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD1v[0]);
    CPPUNIT_ASSERT(pRefKidD10);
    auto pObjectD10 = pRefKidD10->LookupObject();
    CPPUNIT_ASSERT(pObjectD10);
    auto pTypeD10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD10->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD10->GetValue());
    auto pSD10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD10->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD10->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD10->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"https://www.mozilla.org/en-US/firefox/118.0/releasenotes/"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.693, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(757.589, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                // this changed to the end of the text, not the start of the fly
                CPPUNIT_ASSERT_DOUBLES_EQUAL(191.707, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(771.389, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD11 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD1v[1]);
    CPPUNIT_ASSERT(pRefKidD11);
    auto pObjectD11 = pRefKidD11->LookupObject();
    CPPUNIT_ASSERT(pObjectD11);
    auto pTypeD11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD11->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD11->GetValue());
    auto pSD11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD11->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD11->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD11->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"https://www.mozilla.org/en-US/firefox/118.0/releasenotes/"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(387.843, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(757.589, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                // this changed to the end of the text, not the start of the fly
                CPPUNIT_ASSERT_DOUBLES_EQUAL(534.407, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(771.389, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    // the problem was that in addition to the 4 links with SE there was 1 more
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pAnnots->GetElements().size());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf142133)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf142133.docx", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);

    // There should be one annotation
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    // We're expecting something like /Type /Annot /A << /Type /Action /S /URI /URI (path)
    CPPUNIT_ASSERT_EQUAL(
        OString("Annot"),
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        OString("Link"),
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"))->GetValue());
    auto pAction = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"));
    CPPUNIT_ASSERT(pAction);
    auto pURIElem
        = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pAction->LookupElement("URI"));
    CPPUNIT_ASSERT(pURIElem);
    // Check it matches
    CPPUNIT_ASSERT_EQUAL(OString("https://google.com/"), pURIElem->GetValue());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf142806)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"LinkPages.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), aPages.size());

    vcl::filter::PDFObjectElement* pDocument(nullptr);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"));
            if (pS1 && pS1->GetValue() == "Document")
            {
                pDocument = pObject1;
            }
        }
    }
    CPPUNIT_ASSERT(pDocument);

    auto pKidsD = dynamic_cast<vcl::filter::PDFArrayElement*>(pDocument->Lookup("K"));
    CPPUNIT_ASSERT(pKidsD);
    // assume there are no MCID ref at this level
    auto pKidsDv = pKidsD->GetElements();
    auto pRefKidD0 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsDv[0]);
    CPPUNIT_ASSERT(pRefKidD0);
    auto pObjectD0 = pRefKidD0->LookupObject();
    CPPUNIT_ASSERT(pObjectD0);
    auto pTypeD0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD0->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD0->GetValue());
    auto pSD0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD0->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("H1"), pSD0->GetValue());

    auto pKidsD0 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD0->Lookup("K"));
    CPPUNIT_ASSERT(pKidsD0);
    auto pKidsD0v = pKidsD0->GetElements();

    auto pRefKidD00 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD0v[0]);
    CPPUNIT_ASSERT(pRefKidD00);
    auto pObjectD00 = pRefKidD00->LookupObject();
    CPPUNIT_ASSERT(pObjectD00);
    auto pTypeD00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD00->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD00->GetValue());
    auto pSD00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD00->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD00->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD00->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"foo foo foo foo"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.693, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(240.455, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(241.007, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(350.855, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD01 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD0v[1]);
    CPPUNIT_ASSERT(pRefKidD01);
    auto pObjectD01 = pRefKidD01->LookupObject();
    CPPUNIT_ASSERT(pObjectD01);
    auto pTypeD01 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD01->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD01->GetValue());
    auto pSD01 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD01->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD01->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD01->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"foo foo foo foo"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.643, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(130.055, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(241.007, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(240.455, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD02 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD0v[2]);
    CPPUNIT_ASSERT(pRefKidD02);
    auto pObjectD02 = pRefKidD02->LookupObject();
    CPPUNIT_ASSERT(pObjectD02);
    auto pTypeD02 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD02->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD02->GetValue());
    auto pSD02 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD02->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD02->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD02->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"foo foo foo foo"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.643, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(252.455, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(241.007, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(362.855, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD03 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD0v[3]);
    CPPUNIT_ASSERT(pRefKidD03);
    auto pObjectD03 = pRefKidD03->LookupObject();
    CPPUNIT_ASSERT(pObjectD03);
    auto pTypeD03 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD03->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD03->GetValue());
    auto pSD03 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD03->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD03->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD03->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"foo foo foo foo"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.643, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(142.055, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(206.007, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(252.455, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }
    auto pRefKidD1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsDv[1]);
    CPPUNIT_ASSERT(pRefKidD1);
    auto pObjectD1 = pRefKidD1->LookupObject();
    CPPUNIT_ASSERT(pObjectD1);
    auto pTypeD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD1->GetValue());
    auto pSD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Text#20body"), pSD1->GetValue());

    auto pKidsD1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD1->Lookup("K"));
    CPPUNIT_ASSERT(pKidsD1);
    auto pKidsD1v = pKidsD1->GetElements();

    auto pRefKidD10 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD1v[0]);
    CPPUNIT_ASSERT(pRefKidD10);
    auto pObjectD10 = pRefKidD10->LookupObject();
    CPPUNIT_ASSERT(pObjectD10);
    auto pTypeD10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD10->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD10->GetValue());
    auto pSD10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD10->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD10->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD10->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"foo foo foo foo"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.693, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(252.455, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(241.007, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(362.855, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD11 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD1v[1]);
    CPPUNIT_ASSERT(pRefKidD11);
    auto pObjectD11 = pRefKidD11->LookupObject();
    CPPUNIT_ASSERT(pObjectD11);
    auto pTypeD11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD11->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD11->GetValue());
    auto pSD11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD11->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD11->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD11->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"foo foo foo foo"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.643, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(140.005, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(241.007, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(252.455, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD12 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD1v[2]);
    CPPUNIT_ASSERT(pRefKidD12);
    auto pObjectD12 = pRefKidD12->LookupObject();
    CPPUNIT_ASSERT(pObjectD12);
    auto pTypeD12 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD12->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD12->GetValue());
    auto pSD12 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD12->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD12->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD12->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"foo foo foo foo"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.643, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(252.455, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(241.007, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(362.855, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKidD13 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD1v[3]);
    CPPUNIT_ASSERT(pRefKidD13);
    auto pObjectD13 = pRefKidD13->LookupObject();
    CPPUNIT_ASSERT(pObjectD13);
    auto pTypeD13 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD13->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD13->GetValue());
    auto pSD13 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD13->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pSD13->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD13->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"foo foo foo foo"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent
                    = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
                CPPUNIT_ASSERT(pARect);
                const auto& rElements = pARect->GetElements();
                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
                const auto* pNumL = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[0]);
                CPPUNIT_ASSERT(pNumL);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(56.643, pNumL->GetValue(), 1e-3);
                const auto* pNumT = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[1]);
                CPPUNIT_ASSERT(pNumT);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(140.005, pNumT->GetValue(), 1e-3);
                const auto* pNumR = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[2]);
                CPPUNIT_ASSERT(pNumR);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(184.707, pNumR->GetValue(), 1e-3);
                const auto* pNumB = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[3]);
                CPPUNIT_ASSERT(pNumB);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(252.455, pNumB->GetValue(), 1e-3);
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    // the problem was that the links in follow frames were all missing
    auto pAnnots0 = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pAnnots0->GetElements().size());
    auto pAnnots1 = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[1]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots1);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pAnnots1->GetElements().size());
    auto pAnnots2 = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[2]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots2);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pAnnots2->GetElements().size());
    auto pAnnots3 = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[3]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots3);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pAnnots3->GetElements().size());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf115967)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf115967.odt");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Get the first page
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();

    // Make sure the elements inside a formula in a RTL document are exported
    // LTR ( m=750abc ) and not RTL ( m=057cba )
    int nPageObjectCount = pPdfPage->getObjectCount();
    OUString sText;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Text)
            continue;
        OUString sChar = pPageObject->getText(pTextPage);
        sText += o3tl::trim(sChar);
    }
    CPPUNIT_ASSERT_EQUAL(OUString("m=750abc"), sText);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf124272)
{
    // Import the bugdoc and export as PDF.
    vcl::filter::PDFDocument aDocument;
    load(u"tdf124272.odt", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // The page has a stream.
    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents");
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
                    "h");

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* pEnd = pStart + aUncompressed.GetSize();
    auto it = std::search(pStart, pEnd, aBitmap.getStr(), aBitmap.getStr() + aBitmap.getLength());
    CPPUNIT_ASSERT(it != pEnd);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf121615)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf121615.odt", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // Get access to the only image on the only page.
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf141171)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf141171.odt", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // Get access to the only image on the only page.
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf129085)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf129085.docx", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // Get access to the only image on the only page.
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTocLink)
{
    // Load the Writer document.
    loadFromURL(u"toc-link.fodt");

    // Update the ToC.
    uno::Reference<text::XDocumentIndexesSupplier> xDocumentIndexesSupplier(mxComponent,
                                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocumentIndexesSupplier.is());

    uno::Reference<util::XRefreshable> xToc(
        xDocumentIndexesSupplier->getDocumentIndexes()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xToc.is());

    xToc->refresh();

    // Save as PDF.
    save("writer_pdf_Export");

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Ensure there is a link on the first page (in the ToC).
    // Without the accompanying fix in place, this test would have failed, as the page contained no
    // links.
    CPPUNIT_ASSERT(pPdfPage->hasLinks());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testReduceSmallImage)
{
    // Load the Writer document.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf114256)
{
    aMediaDescriptor["FilterName"] <<= OUString("calc_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf150931)
{
    aMediaDescriptor["FilterName"] <<= OUString("calc_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf147027)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // Load the Calc document.
    aMediaDescriptor["FilterName"] <<= OUString("calc_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf135346)
{
    // Load the Calc document.
    aMediaDescriptor["FilterName"] <<= OUString("calc_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf147164)
{
    aMediaDescriptor["FilterName"] <<= OUString("impress_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testReduceImage)
{
    // Load the Writer document.
    loadFromURL(u"reduce-image.fodt");

    // Save as PDF.
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance("com.sun.star.document.PDFFilter"), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);

    SvFileStream aOutputStream(maTempFile.GetURL(), StreamMode::WRITE);
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aOutputStream));

    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "ReduceImageResolution", uno::Any(false) } }));

    // This is intentionally in an "unlucky" order, output stream comes before filter data.
    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer_pdf_Export")) },
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testLinkWrongPage)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("impress_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testLinkWrongPagePartial)
{
    // Given a Draw document with 3 pages, a link on the 2nd page:
    // When exporting that the 2nd and 3rd page to pdf:
    uno::Sequence<beans::PropertyValue> aFilterData = {
        comphelper::makePropertyValue("PageRange", OUString("2-3")),
    };
    aMediaDescriptor["FilterName"] <<= OUString("draw_pdf_Export");
    aMediaDescriptor["FilterData"] <<= aFilterData;
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testPageRange)
{
    // Given a document with 3 pages:
    // When exporting that document to PDF, skipping the first page:
    aMediaDescriptor["FilterName"] <<= OUString("draw_pdf_Export");
    aMediaDescriptor["FilterOptions"]
        <<= OUString("{\"PageRange\":{\"type\":\"string\",\"value\":\"2-\"}}");
    saveAsPDF(u"link-wrong-page-partial.odg");

    // Then make sure the resulting PDF has 2 pages:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. FilterOptions was ignored.
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testLargePage)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("draw_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testPdfImageResourceInlineXObjectRef)
{
    // Create an empty document.
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    // Insert the PDF image.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphicObject(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    OUString aURL = createFileURL(u"pdf-image-resource-inline-xobject-ref.pdf");
    xGraphicObject->setPropertyValue("GraphicURL", uno::Any(aURL));
    uno::Reference<drawing::XShape> xShape(xGraphicObject, uno::UNO_QUERY);
    xShape->setSize(awt::Size(1000, 1000));
    uno::Reference<text::XTextContent> xTextContent(xGraphicObject, uno::UNO_QUERY);
    xText->insertTextContent(xCursor->getStart(), xTextContent, /*bAbsorb=*/false);

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testDefaultVersion)
{
    // Create an empty document.
    mxComponent = loadFromDesktop("private:factory/swriter");

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    int nFileVersion = pPdfDocument->getFileVersion();
    CPPUNIT_ASSERT_EQUAL(17, nFileVersion);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testVersion15)
{
    // Create an empty document.
    mxComponent = loadFromDesktop("private:factory/swriter");

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "SelectPdfVersion", uno::Any(static_cast<sal_Int32>(15)) } }));
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    aMediaDescriptor["FilterData"] <<= aFilterData;
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
CPPUNIT_TEST_FIXTURE(PdfExportTest, testMultiPagePDF)
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
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aPages.size());

    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);

    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3),
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
        CPPUNIT_ASSERT_EQUAL(OString("Im21"), rIDs[0]);

        auto pSubtype1 = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject1->Lookup("Subtype"));
        CPPUNIT_ASSERT(pSubtype1);
        CPPUNIT_ASSERT_EQUAL(OString("Form"), pSubtype1->GetValue());

        auto pXObjectResources
            = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject1->Lookup("Resources"));
        CPPUNIT_ASSERT(pXObjectResources);
        auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
            pXObjectResources->LookupElement("XObject"));
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
        CPPUNIT_ASSERT_EQUAL(OString("Im27"), rIDs[1]);

        auto pSubtype2 = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject2->Lookup("Subtype"));
        CPPUNIT_ASSERT(pSubtype2);
        CPPUNIT_ASSERT_EQUAL(OString("Form"), pSubtype2->GetValue());

        auto pXObjectResources
            = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject2->Lookup("Resources"));
        CPPUNIT_ASSERT(pXObjectResources);
        auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
            pXObjectResources->LookupElement("XObject"));
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
        CPPUNIT_ASSERT_EQUAL(OString("Im5"), rIDs[2]);

        auto pSubtype3 = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject3->Lookup("Subtype"));
        CPPUNIT_ASSERT(pSubtype3);
        CPPUNIT_ASSERT_EQUAL(OString("Form"), pSubtype3->GetValue());

        auto pXObjectResources
            = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject3->Lookup("Resources"));
        CPPUNIT_ASSERT(pXObjectResources);
        auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
            pXObjectResources->LookupElement("XObject"));
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testFormFontName)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
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
    CPPUNIT_ASSERT(pAnnot->hasKey("DA"));
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFObjectType::String, pAnnot->getValueType("DA"));
    OUString aDA = pAnnot->getString("DA");

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0 0 0 rg /TiRo 12 Tf
    // - Actual  : 0 0 0 rg /F2 12 Tf
    // i.e. Liberation Serif was exposed as a form font as-is, without picking the closest built-in
    // font.
    CPPUNIT_ASSERT_EQUAL(OUString("0 0 0 rg /TiRo 12 Tf"), aDA);
}

// Check we don't have duplicated objects when we reexport the PDF multiple
// times or the size will exponentially increase over time.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testReexportPDF)
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
        vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
        CPPUNIT_ASSERT(pResources);

        auto pXObjects
            = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
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
            CPPUNIT_ASSERT_EQUAL(OString("Im14"), aID);
            vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(aID);
            CPPUNIT_ASSERT(pXObject);

            auto pSubtype = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject->Lookup("Subtype"));
            CPPUNIT_ASSERT(pSubtype);
            CPPUNIT_ASSERT_EQUAL(OString("Form"), pSubtype->GetValue());

            auto pInnerResources
                = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"));
            CPPUNIT_ASSERT(pInnerResources);
            auto pInnerXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerResources->LookupElement("XObject"));
            CPPUNIT_ASSERT(pInnerXObjects);
            CPPUNIT_ASSERT_EQUAL(size_t(1), pInnerXObjects->GetItems().size());
            OString aInnerObjectID = pInnerXObjects->GetItems().begin()->first;
            CPPUNIT_ASSERT_EQUAL(OString("Im15"), aInnerObjectID);

            vcl::filter::PDFObjectElement* pInnerXObject
                = pInnerXObjects->LookupObject(aInnerObjectID);
            CPPUNIT_ASSERT(pInnerXObject);

            auto pInnerSubtype
                = dynamic_cast<vcl::filter::PDFNameElement*>(pInnerXObject->Lookup("Subtype"));
            CPPUNIT_ASSERT(pInnerSubtype);
            CPPUNIT_ASSERT_EQUAL(OString("Form"), pInnerSubtype->GetValue());

            auto pInnerInnerResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerXObject->Lookup("Resources"));
            CPPUNIT_ASSERT(pInnerInnerResources);
            auto pInnerInnerXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerInnerResources->LookupElement("XObject"));
            CPPUNIT_ASSERT(pInnerInnerXObjects);
            CPPUNIT_ASSERT_EQUAL(size_t(2), pInnerInnerXObjects->GetItems().size());

            std::vector<OString> aBitmapIDs1;
            for (auto const& rPair : pInnerInnerXObjects->GetItems())
                aBitmapIDs1.push_back(rPair.first);

            {
                CPPUNIT_ASSERT_EQUAL(OString("Im11"), aBitmapIDs1[0]);
                auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pInnerInnerXObjects->LookupElement(aBitmapIDs1[0]));
                CPPUNIT_ASSERT(pRef);
                aBitmapRefs1.push_back(pRef->GetObjectValue());
                CPPUNIT_ASSERT_EQUAL(0, pRef->GetGenerationValue());

                vcl::filter::PDFObjectElement* pBitmap
                    = pInnerInnerXObjects->LookupObject(aBitmapIDs1[0]);
                CPPUNIT_ASSERT(pBitmap);
                auto pBitmapSubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pBitmap->Lookup("Subtype"));
                CPPUNIT_ASSERT(pBitmapSubtype);
                CPPUNIT_ASSERT_EQUAL(OString("Image"), pBitmapSubtype->GetValue());
            }
            {
                CPPUNIT_ASSERT_EQUAL(OString("Im5"), aBitmapIDs1[1]);
                auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pInnerInnerXObjects->LookupElement(aBitmapIDs1[1]));
                CPPUNIT_ASSERT(pRef);
                aBitmapRefs1.push_back(pRef->GetObjectValue());
                CPPUNIT_ASSERT_EQUAL(0, pRef->GetGenerationValue());

                vcl::filter::PDFObjectElement* pBitmap
                    = pInnerInnerXObjects->LookupObject(aBitmapIDs1[1]);
                CPPUNIT_ASSERT(pBitmap);
                auto pBitmapSubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pBitmap->Lookup("Subtype"));
                CPPUNIT_ASSERT(pBitmapSubtype);
                CPPUNIT_ASSERT_EQUAL(OString("Image"), pBitmapSubtype->GetValue());
            }
        }

        {
            // FORM object 2
            OString aID = rIDs[1];
            CPPUNIT_ASSERT_EQUAL(OString("Im5"), aID);
            vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(aID);
            CPPUNIT_ASSERT(pXObject);

            auto pSubtype = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject->Lookup("Subtype"));
            CPPUNIT_ASSERT(pSubtype);
            CPPUNIT_ASSERT_EQUAL(OString("Form"), pSubtype->GetValue());

            auto pInnerResources
                = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"));
            CPPUNIT_ASSERT(pInnerResources);
            auto pInnerXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerResources->LookupElement("XObject"));
            CPPUNIT_ASSERT(pInnerXObjects);
            CPPUNIT_ASSERT_EQUAL(size_t(1), pInnerXObjects->GetItems().size());
            OString aInnerObjectID = pInnerXObjects->GetItems().begin()->first;
            CPPUNIT_ASSERT_EQUAL(OString("Im6"), aInnerObjectID);

            vcl::filter::PDFObjectElement* pInnerXObject
                = pInnerXObjects->LookupObject(aInnerObjectID);
            CPPUNIT_ASSERT(pInnerXObject);

            auto pInnerSubtype
                = dynamic_cast<vcl::filter::PDFNameElement*>(pInnerXObject->Lookup("Subtype"));
            CPPUNIT_ASSERT(pInnerSubtype);
            CPPUNIT_ASSERT_EQUAL(OString("Form"), pInnerSubtype->GetValue());

            auto pInnerInnerResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerXObject->Lookup("Resources"));
            CPPUNIT_ASSERT(pInnerInnerResources);
            auto pInnerInnerXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
                pInnerInnerResources->LookupElement("XObject"));
            CPPUNIT_ASSERT(pInnerInnerXObjects);
            CPPUNIT_ASSERT_EQUAL(size_t(2), pInnerInnerXObjects->GetItems().size());

            std::vector<OString> aBitmapIDs2;
            for (auto const& rPair : pInnerInnerXObjects->GetItems())
                aBitmapIDs2.push_back(rPair.first);

            {
                CPPUNIT_ASSERT_EQUAL(OString("Im11"), aBitmapIDs2[0]);
                auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pInnerInnerXObjects->LookupElement(aBitmapIDs2[0]));
                CPPUNIT_ASSERT(pRef);
                aBitmapRefs2.push_back(pRef->GetObjectValue());
                CPPUNIT_ASSERT_EQUAL(0, pRef->GetGenerationValue());

                vcl::filter::PDFObjectElement* pBitmap
                    = pInnerInnerXObjects->LookupObject(aBitmapIDs2[0]);
                CPPUNIT_ASSERT(pBitmap);
                auto pBitmapSubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pBitmap->Lookup("Subtype"));
                CPPUNIT_ASSERT(pBitmapSubtype);
                CPPUNIT_ASSERT_EQUAL(OString("Image"), pBitmapSubtype->GetValue());
            }
            {
                CPPUNIT_ASSERT_EQUAL(OString("Im5"), aBitmapIDs2[1]);
                auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pInnerInnerXObjects->LookupElement(aBitmapIDs2[1]));
                CPPUNIT_ASSERT(pRef);
                aBitmapRefs2.push_back(pRef->GetObjectValue());
                CPPUNIT_ASSERT_EQUAL(0, pRef->GetGenerationValue());

                vcl::filter::PDFObjectElement* pBitmap
                    = pInnerInnerXObjects->LookupObject(aBitmapIDs2[1]);
                CPPUNIT_ASSERT(pBitmap);
                auto pBitmapSubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pBitmap->Lookup("Subtype"));
                CPPUNIT_ASSERT(pBitmapSubtype);
                CPPUNIT_ASSERT_EQUAL(OString("Image"), pBitmapSubtype->GetValue());
            }
        }
        // Ref should point to the same bitmap
        CPPUNIT_ASSERT_EQUAL(aBitmapRefs1[0], aBitmapRefs2[0]);
        CPPUNIT_ASSERT_EQUAL(aBitmapRefs1[1], aBitmapRefs2[1]);
    }

#endif
}

// Check we correctly copy more complex resources (Fonts describing
// glyphs in recursive arrays) to the target PDF
CPPUNIT_TEST_FIXTURE(PdfExportTest, testReexportDocumentWithComplexResources)
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
    auto pName
        = dynamic_cast<vcl::filter::PDFNameElement*>(pFont->GetDictionary()->LookupElement("Type"));
    CPPUNIT_ASSERT(pName);
    CPPUNIT_ASSERT_EQUAL(OString("Font"), pName->GetValue());

    // Check BaseFont is what we expect
    auto pBaseFont = dynamic_cast<vcl::filter::PDFNameElement*>(
        pFont->GetDictionary()->LookupElement("BaseFont"));
    CPPUNIT_ASSERT(pBaseFont);
    CPPUNIT_ASSERT_EQUAL(OString("HOTOMR+Calibri,Italic"), pBaseFont->GetValue());

    // Check and get the W array
    auto pWArray
        = dynamic_cast<vcl::filter::PDFArrayElement*>(pFont->GetDictionary()->LookupElement("W"));
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
CPPUNIT_TEST_FIXTURE(PdfExportTest, testPdfUaMetadata)
{
    // Import a basic document (document doesn't really matter)
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"BrownFoxLazyDog.odt");

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    auto* pCatalog = aDocument.GetCatalog();
    CPPUNIT_ASSERT(pCatalog);
    auto* pCatalogDictionary = pCatalog->GetDictionary();
    CPPUNIT_ASSERT(pCatalogDictionary);
    auto* pMetadataObject = pCatalogDictionary->LookupObject("Metadata");
    CPPUNIT_ASSERT(pMetadataObject);
    auto* pMetadataDictionary = pMetadataObject->GetDictionary();
    auto* pType
        = dynamic_cast<vcl::filter::PDFNameElement*>(pMetadataDictionary->LookupElement("Type"));
    CPPUNIT_ASSERT(pType);
    CPPUNIT_ASSERT_EQUAL(OString("Metadata"), pType->GetValue());

    auto* pStreamObject = pMetadataObject->GetStream();
    CPPUNIT_ASSERT(pStreamObject);
    auto& rStream = pStreamObject->GetMemory();
    rStream.Seek(0);

    // Search for the PDF/UA marker in the metadata

    tools::XmlWalker aWalker;
    CPPUNIT_ASSERT(aWalker.open(&rStream));
    CPPUNIT_ASSERT_EQUAL(OString("xmpmeta"), aWalker.name());

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
    CPPUNIT_ASSERT_EQUAL(OString("1"), aPdfUaPart);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf139736)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) },
                                           { "SelectPdfVersion", uno::Any(sal_Int32(17)) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"tdf139736-1.odt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents");
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

    auto nLine(0);
    auto nTagged(0);
    auto nArtifacts(0);
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
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nTagged)>(1), nTagged); // text in body
    // 1 image and 1 frame and 1 header text; arbitrary number of aux stuff like borders
    CPPUNIT_ASSERT(nArtifacts >= 3);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf152231)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) },
                                           { "ExportNotesInMargin", uno::Any(true) },
                                           { "SelectPdfVersion", uno::Any(sal_Int32(17)) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"tdf152231.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents");
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

    auto nLine(0);
    auto nTagged(0);
    auto nArtifacts(0);
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
    CPPUNIT_ASSERT(nTagged >= 12); // text in body
    // 1 annotation
    CPPUNIT_ASSERT(nArtifacts >= 1);

    auto nPara(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"));
            if (pS && pS->GetValue() == "Standard")
            {
                ++nPara;
                auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject->Lookup("K"));
                CPPUNIT_ASSERT(pKids);
                // one problem was that that some StructElem were missing kids
                CPPUNIT_ASSERT(!pKids->GetElements().empty());
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nPara)>(12), nPara);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf152235)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "PDFUACompliance", uno::Any(true) },
          { "Watermark", uno::Any(OUString("kendy")) },
          // need to set a font to avoid assertions about missing "Helvetica"
          { "WatermarkFontName", uno::Any(OUString("Liberation Sans")) },
          { "SelectPdfVersion", uno::Any(sal_Int32(17)) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents");
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

    auto nLine(0);
    auto nTagged(0);
    auto nArtifacts(0);
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
    CPPUNIT_ASSERT(nTagged >= 0); // text in body
    CPPUNIT_ASSERT(nArtifacts >= 2); // 1 watermark + 1 other thing
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf149140)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"TableTH_test_LibreOfficeWriter7.3.3_HeaderRow-HeadersInTopRow.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    int nTH(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"));
            if (pS && pS->GetValue() == "TH")
            {
                int nTable(0);
                auto pAttrs = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject->Lookup("A"));
                CPPUNIT_ASSERT(pAttrs != nullptr);
                for (const auto& rAttrRef : pAttrs->GetElements())
                {
                    auto pAttrDict = dynamic_cast<vcl::filter::PDFDictionaryElement*>(rAttrRef);
                    CPPUNIT_ASSERT(pAttrDict != nullptr);
                    auto pOwner
                        = dynamic_cast<vcl::filter::PDFNameElement*>(pAttrDict->LookupElement("O"));
                    CPPUNIT_ASSERT(pOwner != nullptr);
                    if (pOwner->GetValue() == "Table")
                    {
                        auto pScope = dynamic_cast<vcl::filter::PDFNameElement*>(
                            pAttrDict->LookupElement("Scope"));
                        CPPUNIT_ASSERT(pScope != nullptr);
                        CPPUNIT_ASSERT_EQUAL(OString("Column"), pScope->GetValue());
                        ++nTable;
                    }
                }
                CPPUNIT_ASSERT_EQUAL(int(1), nTable);
                ++nTH;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(int(6), nTH);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testNestedSection)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"nestedsection.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // the assert needs 2 follows to reproduce => 3 pages
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aPages.size());

    auto nDoc(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"));
            if (pS1 && pS1->GetValue() == "Document")
            {
                auto pKids1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1->Lookup("K"));
                CPPUNIT_ASSERT(pKids1);
                // assume there are no MCID ref at this level
                auto pKids1v = pKids1->GetElements();
                auto pRefKid10 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[0]);
                CPPUNIT_ASSERT(pRefKid10);
                auto pObject10 = pRefKid10->LookupObject();
                CPPUNIT_ASSERT(pObject10);
                auto pType10
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType10->GetValue());
                auto pS10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Sect"), pS10->GetValue());

                auto pKids10 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10->Lookup("K"));
                CPPUNIT_ASSERT(pKids10);
                // assume there are no MCID ref at this level
                auto pKids10v = pKids10->GetElements();

                auto pRefKid100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids10v[0]);
                CPPUNIT_ASSERT(pRefKid100);
                auto pObject100 = pRefKid100->LookupObject();
                CPPUNIT_ASSERT(pObject100);
                auto pType100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject100->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType100->GetValue());
                auto pS100 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject100->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pS100->GetValue());

                auto pRefKid101 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids10v[1]);
                CPPUNIT_ASSERT(pRefKid101);
                auto pObject101 = pRefKid101->LookupObject();
                CPPUNIT_ASSERT(pObject101);
                auto pType101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject101->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType101->GetValue());
                auto pS101 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject101->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pS101->GetValue());

                auto pRefKid102 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids10v[2]);
                CPPUNIT_ASSERT(pRefKid102);
                auto pObject102 = pRefKid102->LookupObject();
                CPPUNIT_ASSERT(pObject102);
                auto pType102
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType102->GetValue());
                auto pS102 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Sect"), pS102->GetValue());

                auto pKids102
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject102->Lookup("K"));
                CPPUNIT_ASSERT(pKids102);
                // assume there are no MCID ref at this level
                auto pKids102v = pKids102->GetElements();

                auto pRefKid1020 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids102v[0]);
                CPPUNIT_ASSERT(pRefKid1020);
                auto pObject1020 = pRefKid1020->LookupObject();
                CPPUNIT_ASSERT(pObject1020);
                auto pType1020
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1020->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType1020->GetValue());
                auto pS1020 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1020->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pS1020->GetValue());

                CPPUNIT_ASSERT_EQUAL(size_t(1), pKids102v.size());

                auto pRefKid103 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids10v[3]);
                CPPUNIT_ASSERT(pRefKid103);
                auto pObject103 = pRefKid103->LookupObject();
                CPPUNIT_ASSERT(pObject103);
                auto pType103
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject103->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType103->GetValue());
                auto pS103 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject103->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pS103->GetValue());

                auto pRefKid104 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids10v[4]);
                CPPUNIT_ASSERT(pRefKid104);
                auto pObject104 = pRefKid104->LookupObject();
                CPPUNIT_ASSERT(pObject104);
                auto pType104
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject104->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType104->GetValue());
                auto pS104 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject104->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pS104->GetValue());

                CPPUNIT_ASSERT_EQUAL(size_t(5), pKids10v.size());

                auto pRefKid11 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[1]);
                CPPUNIT_ASSERT(pRefKid11);
                auto pObject11 = pRefKid11->LookupObject();
                CPPUNIT_ASSERT(pObject11);
                auto pType11
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType11->GetValue());
                auto pS11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pS11->GetValue());

                CPPUNIT_ASSERT_EQUAL(size_t(2), pKids1v.size());
                ++nDoc;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nDoc)>(1), nDoc);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf157817)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"SimpleTOC.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aPages.size());

    vcl::filter::PDFObjectElement* pTOC(nullptr);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"));
            if (pS1 && pS1->GetValue() == "TOC")
            {
                pTOC = pObject1;
            }
        }
    }
    CPPUNIT_ASSERT(pTOC);

    auto pKidsT = dynamic_cast<vcl::filter::PDFArrayElement*>(pTOC->Lookup("K"));
    CPPUNIT_ASSERT(pKidsT);
    // assume there are no MCID ref at this level
    auto pKidsTv = pKidsT->GetElements();
    auto pRefKidT0 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsTv[0]);
    CPPUNIT_ASSERT(pRefKidT0);
    auto pObjectT0 = pRefKidT0->LookupObject();
    CPPUNIT_ASSERT(pObjectT0);
    auto pTypeT0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT0->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeT0->GetValue());
    auto pST0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT0->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Caption"), pST0->GetValue());

    auto pKidsT0 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT0->Lookup("K"));
    CPPUNIT_ASSERT(pKidsT0);
    auto pKidsT0v = pKidsT0->GetElements();
    auto pRefKidT00 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT0v[0]);
    CPPUNIT_ASSERT(pRefKidT00);
    auto pObjectT00 = pRefKidT00->LookupObject();
    CPPUNIT_ASSERT(pObjectT00);
    auto pTypeT00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT00->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeT00->GetValue());
    auto pST00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT00->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Contents#20Heading"), pST00->GetValue());

    auto pRefKidT1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsTv[1]);
    CPPUNIT_ASSERT(pRefKidT1);
    auto pObjectT1 = pRefKidT1->LookupObject();
    CPPUNIT_ASSERT(pObjectT1);
    auto pTypeT1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT1->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeT1->GetValue());
    auto pST1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT1->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("TOCI"), pST1->GetValue());

    auto pKidsT1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT1->Lookup("K"));
    CPPUNIT_ASSERT(pKidsT1);
    auto pKidsT1v = pKidsT1->GetElements();

    auto pRefKidT10 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT1v[0]);
    CPPUNIT_ASSERT(pRefKidT10);
    auto pObjectT10 = pRefKidT10->LookupObject();
    CPPUNIT_ASSERT(pObjectT10);
    auto pTypeT10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT10->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeT10->GetValue());
    auto pST10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT10->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Contents#201"), pST10->GetValue());

    auto pKidsT10 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT10->Lookup("K"));
    CPPUNIT_ASSERT(pKidsT10);
    auto pKidsT10v = pKidsT10->GetElements();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pKidsT10v.size());

    // there is one and only one Link
    auto pRefKidT100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT10v[0]);
    CPPUNIT_ASSERT(pRefKidT100);
    auto pObjectT100 = pRefKidT100->LookupObject();
    CPPUNIT_ASSERT(pObjectT100);
    auto pTypeT100 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT100->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeT100->GetValue());
    auto pST100 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT100->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pST100->GetValue());

    auto pRefKidT2 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsTv[1]);
    CPPUNIT_ASSERT(pRefKidT2);
    auto pObjectT2 = pRefKidT2->LookupObject();
    CPPUNIT_ASSERT(pObjectT2);
    auto pTypeT2 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT2->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeT2->GetValue());
    auto pST2 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT2->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("TOCI"), pST2->GetValue());

    auto pKidsT2 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT2->Lookup("K"));
    CPPUNIT_ASSERT(pKidsT2);
    auto pKidsT2v = pKidsT2->GetElements();

    auto pRefKidT20 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT2v[0]);
    CPPUNIT_ASSERT(pRefKidT20);
    auto pObjectT20 = pRefKidT20->LookupObject();
    CPPUNIT_ASSERT(pObjectT20);
    auto pTypeT20 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT20->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeT20->GetValue());
    auto pST20 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT20->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Contents#201"), pST20->GetValue());

    auto pKidsT20 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT20->Lookup("K"));
    CPPUNIT_ASSERT(pKidsT20);
    auto pKidsT20v = pKidsT20->GetElements();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pKidsT20v.size());

    // there is one and only one Link
    auto pRefKidT200 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT20v[0]);
    CPPUNIT_ASSERT(pRefKidT200);
    auto pObjectT200 = pRefKidT200->LookupObject();
    CPPUNIT_ASSERT(pObjectT200);
    auto pTypeT200 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT200->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeT200->GetValue());
    auto pST200 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT200->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pST200->GetValue());

    auto pRefKidT3 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsTv[1]);
    CPPUNIT_ASSERT(pRefKidT3);
    auto pObjectT3 = pRefKidT3->LookupObject();
    CPPUNIT_ASSERT(pObjectT3);
    auto pTypeT3 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT3->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeT3->GetValue());
    auto pST3 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT3->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("TOCI"), pST3->GetValue());

    auto pKidsT3 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT3->Lookup("K"));
    CPPUNIT_ASSERT(pKidsT3);
    auto pKidsT3v = pKidsT3->GetElements();

    auto pRefKidT30 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT3v[0]);
    CPPUNIT_ASSERT(pRefKidT30);
    auto pObjectT30 = pRefKidT30->LookupObject();
    CPPUNIT_ASSERT(pObjectT30);
    auto pTypeT30 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT30->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeT30->GetValue());
    auto pST30 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT30->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Contents#201"), pST30->GetValue());

    auto pKidsT30 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectT30->Lookup("K"));
    CPPUNIT_ASSERT(pKidsT30);
    auto pKidsT30v = pKidsT30->GetElements();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pKidsT30v.size());

    // there is one and only one Link
    auto pRefKidT300 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsT30v[0]);
    CPPUNIT_ASSERT(pRefKidT300);
    auto pObjectT300 = pRefKidT300->LookupObject();
    CPPUNIT_ASSERT(pObjectT300);
    auto pTypeT300 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT300->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeT300->GetValue());
    auto pST300 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectT300->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pST300->GetValue());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf135638)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"image-shape.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    int nFigure(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"));
            if (pS && pS->GetValue() == "Figure")
            {
                auto pAttrDict
                    = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject->Lookup("A"));
                CPPUNIT_ASSERT(pAttrDict != nullptr);
                auto pOwner
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAttrDict->LookupElement("O"));
                CPPUNIT_ASSERT(pOwner != nullptr);
                CPPUNIT_ASSERT_EQUAL(OString("Layout"), pOwner->GetValue());
                auto pBBox
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAttrDict->LookupElement("BBox"));
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
    CPPUNIT_ASSERT_EQUAL(int(2), nFigure);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf157703)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"LO_Lbl_Lbody_bug_report.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pDocument(nullptr);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"));
            if (pS1 && pS1->GetValue() == "Document")
            {
                pDocument = pObject1;
            }
        }
    }
    CPPUNIT_ASSERT(pDocument);

    auto pKidsD = dynamic_cast<vcl::filter::PDFArrayElement*>(pDocument->Lookup("K"));
    CPPUNIT_ASSERT(pKidsD);
    // assume there are no MCID ref at this level
    auto pKidsDv = pKidsD->GetElements();
    auto pRefKidD0 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsDv[0]);
    CPPUNIT_ASSERT(pRefKidD0);
    auto pObjectD0 = pRefKidD0->LookupObject();
    CPPUNIT_ASSERT(pObjectD0);
    auto pTypeD0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD0->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD0->GetValue());
    auto pSD0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD0->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("H1"), pSD0->GetValue());

    auto pKidsD0 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD0->Lookup("K"));
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
    auto pTypeD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pTypeD1->GetValue());
    auto pSD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("H2"), pSD1->GetValue());

    auto pKidsD1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD1->Lookup("K"));
    CPPUNIT_ASSERT(pKidsD1);
    auto pKidsD1v = pKidsD1->GetElements();

    // MCID for text
    auto pRefKidD11 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD1v[0]);
    CPPUNIT_ASSERT(!pRefKidD11);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testSpans)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"spanlist.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aPages.size());

    auto nDoc(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"));
            if (pS1 && pS1->GetValue() == "Document")
            {
                auto pKids1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1->Lookup("K"));
                CPPUNIT_ASSERT(pKids1);
                // assume there are no MCID ref at this level
                auto vKids1 = pKids1->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids1.size());
                auto pRefKid10 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1[0]);
                CPPUNIT_ASSERT(pRefKid10);
                auto pObject10 = pRefKid10->LookupObject();
                CPPUNIT_ASSERT(pObject10);
                auto pType10
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType10->GetValue());
                auto pS10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("L"), pS10->GetValue());

                auto pKids10 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10->Lookup("K"));
                CPPUNIT_ASSERT(pKids10);
                // assume there are no MCID ref at this level
                auto vKids10 = pKids10->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(4), vKids10.size());

                auto pRefKid100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10[0]);
                CPPUNIT_ASSERT(pRefKid100);
                auto pObject100 = pRefKid100->LookupObject();
                CPPUNIT_ASSERT(pObject100);
                auto pType100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject100->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType100->GetValue());
                auto pS100 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject100->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("LI"), pS100->GetValue());

                auto pKids100
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject100->Lookup("K"));
                CPPUNIT_ASSERT(pKids100);
                // assume there are no MCID ref at this level
                auto vKids100 = pKids100->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids100.size());

                auto pRefKid1000 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids100[0]);
                CPPUNIT_ASSERT(pRefKid1000);
                auto pObject1000 = pRefKid1000->LookupObject();
                CPPUNIT_ASSERT(pObject1000);
                auto pType1000
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1000->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType1000->GetValue());
                auto pS1000 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1000->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Lbl"), pS1000->GetValue());

                auto pRefKid1001 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids100[1]);
                CPPUNIT_ASSERT(pRefKid1001);
                auto pObject1001 = pRefKid1001->LookupObject();
                CPPUNIT_ASSERT(pObject1001);
                auto pType1001
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1001->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType1001->GetValue());
                auto pS1001 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1001->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("LBody"), pS1001->GetValue());
                auto pKids1001
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1001->Lookup("K"));
                CPPUNIT_ASSERT(pKids1001);
                // assume there are no MCID ref at this level
                auto vKids1001 = pKids1001->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids1001.size());

                auto pRefKid10010 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1001[0]);
                CPPUNIT_ASSERT(pRefKid10010);
                auto pObject10010 = pRefKid10010->LookupObject();
                CPPUNIT_ASSERT(pObject10010);
                auto pType10010
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10010->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType10010->GetValue());
                auto pS10010
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10010->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pS10010->GetValue());
                auto pKids10010
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10010->Lookup("K"));
                CPPUNIT_ASSERT(pKids10010);
                // assume there are no MCID ref at this level
                auto vKids10010 = pKids10010->GetElements();
                // only one span
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids10010.size());

                auto pRefKid100100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10010[0]);
                CPPUNIT_ASSERT(pRefKid100100);
                auto pObject100100 = pRefKid100100->LookupObject();
                CPPUNIT_ASSERT(pObject100100);
                auto pType100100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject100100->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType100100->GetValue());
                auto pS100100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject100100->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Span"), pS100100->GetValue());
                // this span exists because of lang
                auto pLang100100 = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pObject100100->Lookup("Lang"));
                CPPUNIT_ASSERT_EQUAL(OString("en-GB"), pLang100100->GetValue());

                auto pRefKid101 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10[1]);
                CPPUNIT_ASSERT(pRefKid101);
                auto pObject101 = pRefKid101->LookupObject();
                CPPUNIT_ASSERT(pObject101);
                auto pType101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject101->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType101->GetValue());
                auto pS101 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject101->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("LI"), pS101->GetValue());

                auto pKids101
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject101->Lookup("K"));
                CPPUNIT_ASSERT(pKids101);
                // assume there are no MCID ref at this level
                auto vKids101 = pKids101->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids101.size());

                auto pRefKid1010 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids101[0]);
                CPPUNIT_ASSERT(pRefKid1010);
                auto pObject1010 = pRefKid1010->LookupObject();
                CPPUNIT_ASSERT(pObject1010);
                auto pType1010
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1010->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType1010->GetValue());
                auto pS1010 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1010->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Lbl"), pS1010->GetValue());

                auto pRefKid1011 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids101[1]);
                CPPUNIT_ASSERT(pRefKid1011);
                auto pObject1011 = pRefKid1011->LookupObject();
                CPPUNIT_ASSERT(pObject1011);
                auto pType1011
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1011->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType1011->GetValue());
                auto pS1011 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1011->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("LBody"), pS1011->GetValue());

                auto pKids1011
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1011->Lookup("K"));
                CPPUNIT_ASSERT(pKids1011);
                // assume there are no MCID ref at this level
                auto vKids1011 = pKids1011->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids1011.size());

                auto pRefKid10110 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1011[0]);
                CPPUNIT_ASSERT(pRefKid10110);
                auto pObject10110 = pRefKid10110->LookupObject();
                CPPUNIT_ASSERT(pObject10110);
                auto pType10110
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10110->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType10110->GetValue());
                auto pS10110
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10110->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pS10110->GetValue());
                auto pKids10110
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10110->Lookup("K"));
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
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType102->GetValue());
                auto pS102 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("LI"), pS102->GetValue());

                auto pKids102
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject102->Lookup("K"));
                CPPUNIT_ASSERT(pKids102);
                // assume there are no MCID ref at this level
                auto vKids102 = pKids102->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids102.size());

                auto pRefKid1020 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids102[0]);
                CPPUNIT_ASSERT(pRefKid1020);
                auto pObject1020 = pRefKid1020->LookupObject();
                CPPUNIT_ASSERT(pObject1020);
                auto pType1020
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1020->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType1020->GetValue());
                auto pS1020 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1020->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Lbl"), pS1020->GetValue());

                auto pRefKid1021 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids102[1]);
                CPPUNIT_ASSERT(pRefKid1021);
                auto pObject1021 = pRefKid1021->LookupObject();
                CPPUNIT_ASSERT(pObject1021);
                auto pType1021
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1021->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType1021->GetValue());
                auto pS1021 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1021->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("LBody"), pS1021->GetValue());

                auto pKids1021
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1021->Lookup("K"));
                CPPUNIT_ASSERT(pKids1021);
                // assume there are no MCID ref at this level
                auto vKids1021 = pKids1021->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids1021.size());

                auto pRefKid10210 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1021[0]);
                CPPUNIT_ASSERT(pRefKid10210);
                auto pObject10210 = pRefKid10210->LookupObject();
                CPPUNIT_ASSERT(pObject10210);
                auto pType10210
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10210->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType10210->GetValue());
                auto pS10210
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10210->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pS10210->GetValue());
                auto pKids10210
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10210->Lookup("K"));
                CPPUNIT_ASSERT(pKids10210);
                // assume there are no MCID ref at this level
                auto vKids10210 = pKids10210->GetElements();
                // 2 span and a hyperlink
                CPPUNIT_ASSERT_EQUAL(size_t(3), vKids10210.size());

                auto pRefKid102100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10210[0]);
                CPPUNIT_ASSERT(pRefKid102100);
                auto pObject102100 = pRefKid102100->LookupObject();
                CPPUNIT_ASSERT(pObject102100);
                auto pType102100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102100->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType102100->GetValue());
                auto pS102100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102100->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Span"), pS102100->GetValue());
                auto pKids102100
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject102100->Lookup("K"));
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
                auto pType102101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102101->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType102101->GetValue());
                auto pS102101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102101->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pS102101->GetValue());
                auto pKids102101
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject102101->Lookup("K"));
                CPPUNIT_ASSERT(pKids102101);
                auto vKids102101 = pKids102101->GetElements();
                auto nRef(0);
                for (size_t i = 0; i < vKids102101.size(); ++i)
                {
                    auto pKid = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids102101[i]);
                    if (pKid)
                    {
                        ++nRef; // annotation
                    }
                }
                CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);

                auto pRefKid102102 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10210[2]);
                CPPUNIT_ASSERT(pRefKid102102);
                auto pObject102102 = pRefKid102102->LookupObject();
                CPPUNIT_ASSERT(pObject102102);
                auto pType102102
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102102->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType102102->GetValue());
                auto pS102102
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject102102->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Span"), pS102102->GetValue());
                auto pKids102102
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject102102->Lookup("K"));
                CPPUNIT_ASSERT(pKids102102);
                auto vKids102102 = pKids102102->GetElements();
                // there is a footnote
                auto nFtn(0);
                for (size_t i = 0; i < vKids102102.size(); ++i)
                {
                    auto pKid = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids102102[i]);
                    if (pKid)
                    {
                        auto pObject = pKid->LookupObject();
                        CPPUNIT_ASSERT(pObject);
                        auto pType
                            = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
                        CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType->GetValue());
                        auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"));
                        CPPUNIT_ASSERT_EQUAL(OString("Link"), pS->GetValue());
                        ++nFtn;
                    }
                }
                CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nFtn)>(1), nFtn);

                auto pRefKid103 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids10[3]);
                CPPUNIT_ASSERT(pRefKid103);
                auto pObject103 = pRefKid103->LookupObject();
                CPPUNIT_ASSERT(pObject103);
                auto pType103
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject103->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType103->GetValue());
                auto pS103 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject103->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("LI"), pS103->GetValue());

                auto pKids103
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject103->Lookup("K"));
                CPPUNIT_ASSERT(pKids103);
                // assume there are no MCID ref at this level
                auto vKids103 = pKids103->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids103.size());

                auto pRefKid1030 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids103[0]);
                CPPUNIT_ASSERT(pRefKid1030);
                auto pObject1030 = pRefKid1030->LookupObject();
                CPPUNIT_ASSERT(pObject1030);
                auto pType1030
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1030->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType1030->GetValue());
                auto pS1030 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1030->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Lbl"), pS1030->GetValue());

                auto pRefKid1031 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids103[1]);
                CPPUNIT_ASSERT(pRefKid1031);
                auto pObject1031 = pRefKid1031->LookupObject();
                CPPUNIT_ASSERT(pObject1031);
                auto pType1031
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1031->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType1031->GetValue());
                auto pS1031 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1031->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("LBody"), pS1031->GetValue());

                auto pKids1031
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1031->Lookup("K"));
                CPPUNIT_ASSERT(pKids1031);
                // assume there are no MCID ref at this level
                auto vKids1031 = pKids1031->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids1031.size());

                auto pRefKid10310 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1031[0]);
                CPPUNIT_ASSERT(pRefKid10310);
                auto pObject10310 = pRefKid10310->LookupObject();
                CPPUNIT_ASSERT(pObject10310);
                auto pType10310
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10310->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType10310->GetValue());
                auto pS10310
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject10310->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pS10310->GetValue());
                auto pKids10310
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject10310->Lookup("K"));
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
                auto pType103101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject103101->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType103101->GetValue());
                auto pS103101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject103101->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Span"), pS103101->GetValue());
                auto pDictA103101
                    = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject103101->Lookup("A"));
                CPPUNIT_ASSERT(pDictA103101 != nullptr);
                CPPUNIT_ASSERT_EQUAL(OString("Layout"), dynamic_cast<vcl::filter::PDFNameElement*>(
                                                            pDictA103101->LookupElement("O"))
                                                            ->GetValue());
                CPPUNIT_ASSERT_EQUAL(OString("LineThrough"),
                                     dynamic_cast<vcl::filter::PDFNameElement*>(
                                         pDictA103101->LookupElement("TextDecorationType"))
                                         ->GetValue());

                // now the footnote container - following the list
                auto pRefKid11 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1[1]);
                CPPUNIT_ASSERT(pRefKid11);
                auto pObject11 = pRefKid11->LookupObject();
                CPPUNIT_ASSERT(pObject11);
                auto pType11
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType11->GetValue());
                auto pS11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Div"), pS11->GetValue());

                auto pKids11 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject11->Lookup("K"));
                CPPUNIT_ASSERT(pKids11);
                // assume there are no MCID ref at this level
                auto vKids11 = pKids11->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids11.size());

                auto pRefKid110 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids11[0]);
                CPPUNIT_ASSERT(pRefKid110);
                auto pObject110 = pRefKid110->LookupObject();
                CPPUNIT_ASSERT(pObject110);
                auto pType110
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject110->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType110->GetValue());
                auto pS110 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject110->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Note"), pS110->GetValue());

                auto pKids110
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject110->Lookup("K"));
                CPPUNIT_ASSERT(pKids110);
                // assume there are no MCID ref at this level
                auto vKids110 = pKids110->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(2), vKids110.size());

                auto pRefKid1100 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids110[0]);
                CPPUNIT_ASSERT(pRefKid1100);
                auto pObject1100 = pRefKid1100->LookupObject();
                CPPUNIT_ASSERT(pObject1100);
                auto pType1100
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1100->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType1100->GetValue());
                auto pS1100 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1100->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Lbl"), pS1100->GetValue());

                auto pKids1100
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1100->Lookup("K"));
                CPPUNIT_ASSERT(pKids1100);
                // assume there are no MCID ref at this level
                auto vKids1100 = pKids1100->GetElements();
                CPPUNIT_ASSERT_EQUAL(size_t(1), vKids1100.size());

                auto pRefKid11000 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids1100[0]);
                CPPUNIT_ASSERT(pRefKid11000);
                auto pObject11000 = pRefKid11000->LookupObject();
                CPPUNIT_ASSERT(pObject11000);
                auto pType11000
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11000->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType11000->GetValue());
                auto pS11000
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11000->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pS11000->GetValue());

                auto pRefKid1101 = dynamic_cast<vcl::filter::PDFReferenceElement*>(vKids110[1]);
                CPPUNIT_ASSERT(pRefKid1101);
                auto pObject1101 = pRefKid1101->LookupObject();
                CPPUNIT_ASSERT(pObject1101);
                auto pType1101
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1101->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType1101->GetValue());
                auto pS1101 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1101->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Footnote"), pS1101->GetValue());

                ++nDoc;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nDoc)>(1), nDoc);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf157182)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "PDFUACompliance", uno::Any(true) },
        // only happens with PDF/A-1
        { "SelectPdfVersion", uno::Any(static_cast<sal_Int32>(1)) },
    }));
    aMediaDescriptor["FilterData"] <<= aFilterData;

    saveAsPDF(u"transparentshape.fodp");

    // just check this does not crash or assert
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf57423)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"Description PDF Export test .odt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    int nFigure(0);
    int nFormula(0);
    int nDiv(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"));
            if (pS && pS->GetValue() == "Figure")
            {
                switch (nFigure)
                {
                    case 2:
                        CPPUNIT_ASSERT_EQUAL(OUString(u"QR Code - Tells how to get to Mosegaard"),
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"))));
                        break;
                    case 0:
                        CPPUNIT_ASSERT_EQUAL(OUString(u"Title: Arrows - Description:  Explains the "
                                                      u"different arrow appearances"),
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"))));
                        break;
                    case 1:
                        CPPUNIT_ASSERT_EQUAL(
                            OUString(u"My blue triangle - Does not need further description"),
                            ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                    pObject->Lookup("Alt"))));
                        break;
                }
                ++nFigure;
            }
            if (pS && pS->GetValue() == "Formula")
            {
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"Equation 1 - Now we give the full description of eq 1 here"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                        *dynamic_cast<vcl::filter::PDFHexStringElement*>(pObject->Lookup("Alt"))));
                ++nFormula;
            }
            if (pS && pS->GetValue() == "Div")
            {
                switch (nDiv)
                {
                    case 0:
                        CPPUNIT_ASSERT_EQUAL(OUString(u"This frame has a description"),
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"))));
                        break;
                    case 1:
                        // no properties set on this
                        CPPUNIT_ASSERT(!pObject->Lookup("Alt"));
                        break;
                    case 2:
                        CPPUNIT_ASSERT_EQUAL(OUString(u"My textbox - Has a light background"),
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"))));
                        break;
                    case 3:
                        CPPUNIT_ASSERT_EQUAL(OUString(u"Hey!  There is no alternate text for Frame "
                                                      u"// but maybe not needed?"),
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"))));
                        break;
                }
                ++nDiv;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(int(3), nFigure);
    CPPUNIT_ASSERT_EQUAL(int(1), nFormula);
    CPPUNIT_ASSERT_EQUAL(int(4), nDiv);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf154982)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"tdf154982.odt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    int nFigure(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"));
            if (pS && pS->GetValue() == "Figure")
            {
                switch (nFigure)
                {
                    case 0:
                        CPPUNIT_ASSERT_EQUAL(
                            OUString(u"Here comes the signature - Please sign here"),
                            ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                    pObject->Lookup("Alt"))));
                        break;
                    case 1:
                        CPPUNIT_ASSERT_EQUAL(OUString(u"Home"),
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"))));
                        break;
                }

                // the problem was that the figures in the hell layer were not
                // below their anchor paragraphs in the structure tree
                auto pParentRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("P"));
                CPPUNIT_ASSERT(pParentRef);
                auto pParent(pParentRef->LookupObject());
                CPPUNIT_ASSERT(pParent);
                auto pParentType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pParentType->GetValue());
                auto pParentS = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pParentS->GetValue());

                auto pPParentRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pParent->Lookup("P"));
                CPPUNIT_ASSERT(pPParentRef);
                auto pPParent(pPParentRef->LookupObject());
                CPPUNIT_ASSERT(pPParent);
                auto pPParentType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pPParent->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pPParentType->GetValue());
                auto pPParentS = dynamic_cast<vcl::filter::PDFNameElement*>(pPParent->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Document"), pPParentS->GetValue());
                ++nFigure;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(int(2), nFigure);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf157397)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"PDF_export_with_formcontrol.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pDocument(nullptr);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"));
            if (pS1 && pS1->GetValue() == "Document")
            {
                pDocument = pObject1;
            }
        }
    }
    CPPUNIT_ASSERT(pDocument);

    auto pKids1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pDocument->Lookup("K"));
    CPPUNIT_ASSERT(pKids1);
    // assume there are no MCID ref at this level
    auto pKids1v = pKids1->GetElements();
    auto pRefKid12 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[2]);
    CPPUNIT_ASSERT(pRefKid12);
    auto pObject12 = pRefKid12->LookupObject();
    CPPUNIT_ASSERT(pObject12);
    auto pType12 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject12->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType12->GetValue());
    auto pS12 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject12->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Text#20body"), pS12->GetValue());

    auto pKids12 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject12->Lookup("K"));
    CPPUNIT_ASSERT(pKids12);
    // assume there are no MCID ref at this level
    auto pKids12v = pKids12->GetElements();
    auto pRefKid120 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids12v[0]);
    CPPUNIT_ASSERT(pRefKid120);
    auto pObject120 = pRefKid120->LookupObject();
    CPPUNIT_ASSERT(pObject120);
    auto pType120 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject120->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType120->GetValue());
    auto pS120 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject120->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pS120->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject120->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"https://klexikon.zum.de/wiki/Kläranlage"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAA = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"));
                CPPUNIT_ASSERT(pAA);
                auto pAAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Action"), pAAType->GetValue());
                auto pAAS = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("S"));
                CPPUNIT_ASSERT_EQUAL(OString("URI"), pAAS->GetValue());
                auto pAAURI = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pAA->LookupElement("URI"));
                CPPUNIT_ASSERT_EQUAL(OString("https://klexikon.zum.de/wiki/Kl%C3%A4ranlage"),
                                     pAAURI->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKid13 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[3]);
    CPPUNIT_ASSERT(pRefKid13);
    auto pObject13 = pRefKid13->LookupObject();
    CPPUNIT_ASSERT(pObject13);
    auto pType13 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject13->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType13->GetValue());
    auto pS13 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject13->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Text#20body"), pS13->GetValue());

    auto pKids13 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject13->Lookup("K"));
    CPPUNIT_ASSERT(pKids13);
    // assume there are no MCID ref at this level
    auto pKids13v = pKids13->GetElements();
    auto pRefKid130 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids13v[0]);
    CPPUNIT_ASSERT(pRefKid130);
    auto pObject130 = pRefKid130->LookupObject();
    CPPUNIT_ASSERT(pObject130);
    auto pType130 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject130->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType130->GetValue());
    auto pS130 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject130->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pS130->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject130->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(u"https://de.wikipedia.org/wiki/Kläranlage#Mechanische_Vorreinigung"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAA = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"));
                CPPUNIT_ASSERT(pAA);
                auto pAAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Action"), pAAType->GetValue());
                auto pAAS = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("S"));
                CPPUNIT_ASSERT_EQUAL(OString("URI"), pAAS->GetValue());
                auto pAAURI = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pAA->LookupElement("URI"));
                CPPUNIT_ASSERT_EQUAL(
                    OString(
                        "https://de.wikipedia.org/wiki/Kl%C3%A4ranlage#Mechanische_Vorreinigung"),
                    pAAURI->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKid14 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[4]);
    CPPUNIT_ASSERT(pRefKid14);
    auto pObject14 = pRefKid14->LookupObject();
    CPPUNIT_ASSERT(pObject14);
    auto pType14 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject14->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType14->GetValue());
    auto pS14 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject14->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Text#20body"), pS14->GetValue());

    auto pKids14 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject14->Lookup("K"));
    CPPUNIT_ASSERT(pKids14);
    // assume there are no MCID ref at this level
    auto pKids14v = pKids14->GetElements();
    auto pRefKid140 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids14v[0]);
    CPPUNIT_ASSERT(pRefKid140);
    auto pObject140 = pRefKid140->LookupObject();
    CPPUNIT_ASSERT(pObject140);
    auto pType140 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject140->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType140->GetValue());
    auto pS140 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject140->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pS140->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject140->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAContents
                    = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
                CPPUNIT_ASSERT_EQUAL(
                    OUString(
                        u"https://vr-easy.com/tour/usr/220113-virtuellerschulausflug/#pano=24"),
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                CPPUNIT_ASSERT_EQUAL(OString("Link"), pASubtype->GetValue());
                auto pAA = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"));
                CPPUNIT_ASSERT(pAA);
                auto pAAType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Action"), pAAType->GetValue());
                auto pAAS = dynamic_cast<vcl::filter::PDFNameElement*>(pAA->LookupElement("S"));
                CPPUNIT_ASSERT_EQUAL(OString("URI"), pAAS->GetValue());
                auto pAAURI = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pAA->LookupElement("URI"));
                CPPUNIT_ASSERT_EQUAL(
                    OString("https://vr-easy.com/tour/usr/220113-virtuellerschulausflug/#pano=24"),
                    pAAURI->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }

    auto pRefKid16 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1v[6]);
    CPPUNIT_ASSERT(pRefKid16);
    auto pObject16 = pRefKid16->LookupObject();
    CPPUNIT_ASSERT(pObject16);
    auto pType16 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject16->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType16->GetValue());
    auto pS16 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject16->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Text#20body"), pS16->GetValue());

    auto pKids16 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject16->Lookup("K"));
    CPPUNIT_ASSERT(pKids16);
    // assume there are no MCID ref at this level
    auto pKids16v = pKids16->GetElements();
    auto pRefKid160 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids16v[0]);
    CPPUNIT_ASSERT(pRefKid160);
    auto pObject160 = pRefKid160->LookupObject();
    CPPUNIT_ASSERT(pObject160);
    auto pType160 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject160->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType160->GetValue());
    auto pS160 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject160->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Form"), pS160->GetValue());
    auto pA160Dict = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject160->Lookup("A"));
    CPPUNIT_ASSERT(pA160Dict);
    auto pA160O = dynamic_cast<vcl::filter::PDFNameElement*>(pA160Dict->LookupElement("O"));
    CPPUNIT_ASSERT(pA160O);
    CPPUNIT_ASSERT_EQUAL(OString("PrintField"), pA160O->GetValue());
    auto pA160Role = dynamic_cast<vcl::filter::PDFNameElement*>(pA160Dict->LookupElement("Role"));
    CPPUNIT_ASSERT(pA160Role);
    CPPUNIT_ASSERT_EQUAL(OString("tv"), pA160Role->GetValue());

    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject160->Lookup("K"));
        auto nMCID(0);
        auto nRef(0);
        for (size_t i = 0; i < pKids->GetElements().size(); ++i)
        {
            auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
            auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
            if (pNum)
            {
                ++nMCID;
            }
            if (pRef)
            {
                ++nRef;
                auto pObjR = pRef->LookupObject();
                auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
                auto pAnnotRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
                auto pAnnot = pAnnotRef->LookupObject();
                auto pAType = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("Annot"), pAType->GetValue());
                auto pASubtype
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"));
                CPPUNIT_ASSERT_EQUAL(OString("Widget"), pASubtype->GetValue());
            }
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
        CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf135192)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"tdf135192-1.fodp");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    int nTable(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructElem")
        {
            auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"));
            if (pS1 && pS1->GetValue() == "Table")
            {
                int nTR(0);
                auto pKids1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1->Lookup("K"));
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
                                pObject2->Lookup("Type"));
                            if (pType2 && pType2->GetValue() == "StructElem")
                            {
                                auto pS2 = dynamic_cast<vcl::filter::PDFNameElement*>(
                                    pObject2->Lookup("S"));
                                if (pS2 && pS2->GetValue() == "TR")
                                {
                                    int nTD(0);
                                    auto pKids2 = dynamic_cast<vcl::filter::PDFArrayElement*>(
                                        pObject2->Lookup("K"));
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
                                                        pObject3->Lookup("Type"));
                                                if (pType3 && pType3->GetValue() == "StructElem")
                                                {
                                                    auto pS3 = dynamic_cast<
                                                        vcl::filter::PDFNameElement*>(
                                                        pObject3->Lookup("S"));
                                                    if (nTR == 0 && pS3 && pS3->GetValue() == "TH")
                                                    {
                                                        int nOTable(0);
                                                        auto pAttrs = dynamic_cast<
                                                            vcl::filter::PDFArrayElement*>(
                                                            pObject3->Lookup("A"));
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
                                                                pAttrDict->LookupElement("O"));
                                                            CPPUNIT_ASSERT(pOwner != nullptr);
                                                            if (pOwner->GetValue() == "Table")
                                                            {
                                                                auto pScope = dynamic_cast<
                                                                    vcl::filter::PDFNameElement*>(
                                                                    pAttrDict->LookupElement(
                                                                        "Scope"));
                                                                CPPUNIT_ASSERT(pScope != nullptr);
                                                                CPPUNIT_ASSERT_EQUAL(
                                                                    OString("Column"),
                                                                    pScope->GetValue());
                                                                ++nOTable;
                                                            }
                                                        }
                                                        CPPUNIT_ASSERT_EQUAL(int(1), nOTable);
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
                                    CPPUNIT_ASSERT_EQUAL(int(3), nTD);
                                    ++nTR;
                                }
                            }
                        }
                    }
                }
                CPPUNIT_ASSERT_EQUAL(int(2), nTR);
                ++nTable;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(int(1), nTable);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf154955)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    saveAsPDF(u"grouped-shape.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents");
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

    auto nLine(0);
    auto nTagged(0);
    auto nArtifacts(0);
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
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nTagged)>(2), nTagged);
    CPPUNIT_ASSERT(nArtifacts >= 1);

    int nFigure(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"));
            if (pS && pS->GetValue() == "Figure")
            {
                switch (nFigure)
                {
                    case 0:
                        CPPUNIT_ASSERT_EQUAL(OUString(u"Two rectangles - Grouped"),
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"))));
                        break;
                    case 1:
                        CPPUNIT_ASSERT_EQUAL(OUString(u"these ones are green"),
                                             ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                 *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                     pObject->Lookup("Alt"))));
                        break;
                }

                auto pParentRef
                    = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("P"));
                CPPUNIT_ASSERT(pParentRef);
                auto pParent(pParentRef->LookupObject());
                CPPUNIT_ASSERT(pParent);
                auto pParentType
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("Type"));
                CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pParentType->GetValue());
                auto pParentS = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("S"));
                CPPUNIT_ASSERT_EQUAL(OString("Standard"), pParentS->GetValue());

                ++nFigure;
            }
        }
    }
    // the problem was that there were 4 shapes (the sub-shapes of the 2 groups)
    CPPUNIT_ASSERT_EQUAL(int(2), nFigure);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf155190)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;

    saveAsPDF(u"tdf155190.odt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    auto nDiv(0);
    auto nFigure(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));

        auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"));
        // start with the text box
        if (pType1 && pType1->GetValue() == "StructElem" && pS1 && pS1->GetValue() == "Div")
        {
            ++nDiv;
            auto pKids1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1->Lookup("K"));
            CPPUNIT_ASSERT(pKids1);
            for (auto pKid1 : pKids1->GetElements())
            {
                auto pRefKid1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKid1);
                if (pRefKid1)
                {
                    auto pObject2 = pRefKid1->LookupObject();
                    CPPUNIT_ASSERT(pObject2);
                    auto pType2
                        = dynamic_cast<vcl::filter::PDFNameElement*>(pObject2->Lookup("Type"));
                    CPPUNIT_ASSERT(pType2);
                    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType2->GetValue());
                    auto pS2 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject2->Lookup("S"));
                    CPPUNIT_ASSERT_EQUAL(OString("FigureCaption"), pS2->GetValue());
                    auto pKids2
                        = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject2->Lookup("K"));
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
                                pObject3->Lookup("Type"));
                            if (pType3 && pType3->GetValue() == "StructElem")
                            {
                                auto pS3 = dynamic_cast<vcl::filter::PDFNameElement*>(
                                    pObject3->Lookup("S"));
                                CPPUNIT_ASSERT_EQUAL(OString("Figure"), pS3->GetValue());
                                auto pAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                    pObject3->Lookup("Alt"));
                                CPPUNIT_ASSERT_EQUAL(
                                    OUString("Picture of apples"),
                                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAlt));
                                auto pKids3 = dynamic_cast<vcl::filter::PDFArrayElement*>(
                                    pObject3->Lookup("K"));
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
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nDiv)>(1), nDiv);
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nDiv)>(1), nFigure);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testMediaShapeAnnot)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;

    saveAsPDF(u"vid.odt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);

    // There should be one annotation
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    // check /Annot - produced by sw
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(
        OString("Annot"),
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        OString("Screen"),
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"))->GetValue());

    auto pA = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"));
    CPPUNIT_ASSERT(pA);
    auto pR = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pA->LookupElement("R"));
    CPPUNIT_ASSERT(pR);
    auto pC = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pR->LookupElement("C"));
    CPPUNIT_ASSERT(pC);
    auto pCT = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pC->LookupElement("CT"));
    CPPUNIT_ASSERT_EQUAL(OString("video/webm"), pCT->GetValue());
    auto pD = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pC->LookupElement("D"));
    CPPUNIT_ASSERT(pD);
    auto pDesc = dynamic_cast<vcl::filter::PDFHexStringElement*>(pD->LookupElement("Desc"));
    CPPUNIT_ASSERT(pDesc);
    CPPUNIT_ASSERT_EQUAL(OUString("alternativloser text\nand some description"),
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pDesc));
    auto pAlt = dynamic_cast<vcl::filter::PDFArrayElement*>(pC->LookupElement("Alt"));
    CPPUNIT_ASSERT(pAlt);
    auto pLang = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pAlt->GetElement(0));
    CPPUNIT_ASSERT_EQUAL(OString(""), pLang->GetValue());
    auto pAltText = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAlt->GetElement(1));
    CPPUNIT_ASSERT_EQUAL(OUString("alternativloser text\nand some description"),
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAltText));

    auto pStructParent
        = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
    CPPUNIT_ASSERT(pStructParent);

    vcl::filter::PDFReferenceElement* pStructElemRef(nullptr);

    // check ParentTree to find StructElem
    auto nRoots(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructTreeRoot")
        {
            ++nRoots;
            auto pParentTree
                = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject1->Lookup("ParentTree"));
            CPPUNIT_ASSERT(pParentTree);
            auto pNumTree = pParentTree->LookupObject();
            CPPUNIT_ASSERT(pNumTree);
            auto pNums = dynamic_cast<vcl::filter::PDFArrayElement*>(pNumTree->Lookup("Nums"));
            CPPUNIT_ASSERT(pNums);
            auto nFound(0);
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
            CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nFound)>(1), nFound);
        }
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRoots)>(1), nRoots);

    // check /StructElem - produced by drawinglayer
    CPPUNIT_ASSERT(pStructElemRef);
    auto pStructElem(pStructElemRef->LookupObject());
    CPPUNIT_ASSERT(pStructElem);

    auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType->GetValue());
    auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Annot"), pS->GetValue());
    auto pSEAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(pStructElem->Lookup("Alt"));
    CPPUNIT_ASSERT_EQUAL(OUString("alternativloser text - and some description"),
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pSEAlt));
    auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pStructElem->Lookup("K"));
    auto nMCID(0);
    auto nRef(0);
    for (size_t i = 0; i < pKids->GetElements().size(); ++i)
    {
        auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
        auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
        if (pNum)
        {
            ++nMCID;
        }
        if (pRef)
        {
            ++nRef;
            auto pObjR = pRef->LookupObject();
            auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
            CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
            auto pAnnotRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
            CPPUNIT_ASSERT_EQUAL(pAnnot, pAnnotRef->LookupObject());
        }
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testFlyFrameHyperlinkAnnot)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;

    saveAsPDF(u"image-hyperlink-alttext.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);

    // There should be one annotation
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    // check /Annot - produced by sw
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(
        OString("Annot"),
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        OString("Link"),
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"))->GetValue());

    auto pContents = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"));
    CPPUNIT_ASSERT_EQUAL(OUString("Image2"),
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pContents));

    auto pStructParent
        = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
    CPPUNIT_ASSERT(pStructParent);

    vcl::filter::PDFReferenceElement* pStructElemRef(nullptr);

    // check ParentTree to find StructElem
    auto nRoots(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructTreeRoot")
        {
            ++nRoots;
            auto pParentTree
                = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject1->Lookup("ParentTree"));
            CPPUNIT_ASSERT(pParentTree);
            auto pNumTree = pParentTree->LookupObject();
            CPPUNIT_ASSERT(pNumTree);
            auto pNums = dynamic_cast<vcl::filter::PDFArrayElement*>(pNumTree->Lookup("Nums"));
            CPPUNIT_ASSERT(pNums);
            auto nFound(0);
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
            CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nFound)>(1), nFound);
        }
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRoots)>(1), nRoots);

    // check /StructElem - produced by sw painting code
    CPPUNIT_ASSERT(pStructElemRef);
    auto pStructElem(pStructElemRef->LookupObject());
    CPPUNIT_ASSERT(pStructElem);

    auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType->GetValue());
    auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Link"), pS->GetValue());
    auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pStructElem->Lookup("K"));
    auto nMCID(0);
    auto nRef(0);
    for (size_t i = 0; i < pKids->GetElements().size(); ++i)
    {
        auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
        auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
        if (pNum)
        {
            ++nMCID;
        }
        if (pRef)
        {
            ++nRef;
            auto pObjR = pRef->LookupObject();
            auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
            CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
            auto pAnnotRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
            CPPUNIT_ASSERT_EQUAL(pAnnot, pAnnotRef->LookupObject());
        }
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);

    // the Link is inside a Figure
    auto pParentRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pStructElem->Lookup("P"));
    CPPUNIT_ASSERT(pParentRef);
    auto pParent(pParentRef->LookupObject());
    CPPUNIT_ASSERT(pParent);
    auto pParentType = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pParentType->GetValue());
    auto pParentS = dynamic_cast<vcl::filter::PDFNameElement*>(pParent->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Figure"), pParentS->GetValue());
    auto pAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(pParent->Lookup("Alt"));
    CPPUNIT_ASSERT_EQUAL(OUString("Ship drawing - Very cute"),
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAlt));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testFormControlAnnot)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;

    saveAsPDF(u"formcontrol.fodt");

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);

    // There should be one annotation
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    // check /Annot
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(
        OString("Annot"),
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        OString("Widget"),
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"))->GetValue());
    auto pT = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pAnnot->Lookup("T"));
    CPPUNIT_ASSERT(pT);
    CPPUNIT_ASSERT_EQUAL(OString("Check Box 1"), pT->GetValue());
    auto pTU = dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("TU"));
    CPPUNIT_ASSERT(pTU);
    CPPUNIT_ASSERT_EQUAL(OUString("helpful text"),
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pTU));

    auto pStructParent
        = dynamic_cast<vcl::filter::PDFNumberElement*>(pAnnot->Lookup("StructParent"));
    CPPUNIT_ASSERT(pStructParent);

    vcl::filter::PDFReferenceElement* pStructElemRef(nullptr);

    // check ParentTree to find StructElem
    auto nRoots(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject1 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject1)
            continue;
        auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"));
        if (pType1 && pType1->GetValue() == "StructTreeRoot")
        {
            ++nRoots;
            auto pParentTree
                = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject1->Lookup("ParentTree"));
            CPPUNIT_ASSERT(pParentTree);
            auto pNumTree = pParentTree->LookupObject();
            CPPUNIT_ASSERT(pNumTree);
            auto pNums = dynamic_cast<vcl::filter::PDFArrayElement*>(pNumTree->Lookup("Nums"));
            CPPUNIT_ASSERT(pNums);
            auto nFound(0);
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
            CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nFound)>(1), nFound);
        }
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRoots)>(1), nRoots);

    // check /StructElem
    CPPUNIT_ASSERT(pStructElemRef);
    auto pStructElem(pStructElemRef->LookupObject());
    CPPUNIT_ASSERT(pStructElem);

    auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("Type"));
    CPPUNIT_ASSERT_EQUAL(OString("StructElem"), pType->GetValue());
    auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pStructElem->Lookup("S"));
    CPPUNIT_ASSERT_EQUAL(OString("Form"), pS->GetValue());
    auto pAlt = dynamic_cast<vcl::filter::PDFHexStringElement*>(pStructElem->Lookup("Alt"));
    CPPUNIT_ASSERT_EQUAL(OUString("textuelle alternative - a box to check"),
                         ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAlt));
    auto pA = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pStructElem->Lookup("A"));
    CPPUNIT_ASSERT(pA);
    auto pO = dynamic_cast<vcl::filter::PDFNameElement*>(pA->LookupElement("O"));
    CPPUNIT_ASSERT(pO);
    CPPUNIT_ASSERT_EQUAL(OString("PrintField"), pO->GetValue());
    auto pRole = dynamic_cast<vcl::filter::PDFNameElement*>(pA->LookupElement("Role"));
    CPPUNIT_ASSERT(pRole);
    CPPUNIT_ASSERT_EQUAL(OString("cb"), pRole->GetValue());
    auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pStructElem->Lookup("K"));
    auto nMCID(0);
    auto nRef(0);
    for (size_t i = 0; i < pKids->GetElements().size(); ++i)
    {
        auto pNum = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids->GetElement(i));
        auto pRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids->GetElement(i));
        if (pNum)
        {
            ++nMCID;
        }
        if (pRef)
        {
            ++nRef;
            auto pObjR = pRef->LookupObject();
            auto pOType = dynamic_cast<vcl::filter::PDFNameElement*>(pObjR->Lookup("Type"));
            CPPUNIT_ASSERT_EQUAL(OString("OBJR"), pOType->GetValue());
            auto pAnnotRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObjR->Lookup("Obj"));
            CPPUNIT_ASSERT_EQUAL(pAnnot, pAnnotRef->LookupObject());
        }
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nMCID)>(1), nMCID);
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nRef)>(1), nRef);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf142129)
{
    loadFromURL(u"master.odm");

    // update linked section
    dispatchCommand(mxComponent, ".uno:UpdateAllLinks", {});

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable Outlines export
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "ExportBookmarks", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    auto* pCatalog = aDocument.GetCatalog();
    CPPUNIT_ASSERT(pCatalog);
    auto* pCatalogDictionary = pCatalog->GetDictionary();
    CPPUNIT_ASSERT(pCatalogDictionary);
    auto* pOutlinesObject = pCatalogDictionary->LookupObject("Outlines");
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
        pOutlinesDictionary->LookupElement("First"));
    CPPUNIT_ASSERT(pFirst);
    auto* pFirstD = pFirst->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirstD);
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirstD->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(OUString(u"Preface"), ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                                   *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                                       pFirstD->LookupElement("Title"))));

    auto* pFirst1
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFirstD->LookupElement("First"));
    CPPUNIT_ASSERT(pFirst1);
    auto* pFirst1D = pFirst1->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirst1D);
    // here is a hidden section with headings "Copyright" etc.; check that
    // there are no outline entries for it
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirst1D->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        OUString(u"Who is this book for?"),
        ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
            *dynamic_cast<vcl::filter::PDFHexStringElement*>(pFirst1D->LookupElement("Title"))));

    auto* pFirst2
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFirst1D->LookupElement("Next"));
    auto* pFirst2D = pFirst2->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirst2D);
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirst2D->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        OUString(u"What\u2019s in this book?"),
        ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
            *dynamic_cast<vcl::filter::PDFHexStringElement*>(pFirst2D->LookupElement("Title"))));

    auto* pFirst3
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFirst2D->LookupElement("Next"));
    auto* pFirst3D = pFirst3->LookupObject()->GetDictionary();
    CPPUNIT_ASSERT(pFirst3D);
    //CPPUNIT_ASSERT_EQUAL(OString("Outlines"), dynamic_cast<vcl::filter::PDFNameElement*>(pFirst3D->LookupElement("Type"))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        OUString(u"Minimum requirements for using LibreOffice"),
        ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
            *dynamic_cast<vcl::filter::PDFHexStringElement*>(pFirst3D->LookupElement("Title"))));

    CPPUNIT_ASSERT_EQUAL(static_cast<vcl::filter::PDFElement*>(nullptr),
                         pFirst3D->LookupElement("Next"));
    CPPUNIT_ASSERT_EQUAL(static_cast<vcl::filter::PDFElement*>(nullptr),
                         pFirstD->LookupElement("Next"));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testPdfImageRotate180)
{
    // Create an empty document.
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    // Insert the PDF image.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphicObject(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    OUString aURL = createFileURL(u"pdf-image-rotate-180.pdf");
    xGraphicObject->setPropertyValue("GraphicURL", uno::Any(aURL));
    uno::Reference<drawing::XShape> xShape(xGraphicObject, uno::UNO_QUERY);
    xShape->setSize(awt::Size(1000, 1000));
    uno::Reference<text::XTextContent> xTextContent(xGraphicObject, uno::UNO_QUERY);
    xText->insertTextContent(xCursor->getStart(), xTextContent, /*bAbsorb=*/false);

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf144222)
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
    const OUString sChar = u"\u4E2D";
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf145873)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("impress_pdf_Export");
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
    // - Expected: 13.23
    // - Actual  : 3.57...
    // - Delta   : 0.1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.23, pObject->getBounds().getWidth(), 0.1);
    // - Expected: 13.49
    // - Actual  : 3.74...
    // - Delta   : 0.1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.49, pObject->getBounds().getHeight(), 0.1);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testPdfImageHyperlink)
{
    // Given a Draw file, containing a PDF image, which has a hyperlink in it:
    aMediaDescriptor["FilterName"] <<= OUString("draw_pdf_Export");

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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testURIs)
{
    struct
    {
        OUString in;
        OString out;
        bool relativeFsys;
    } URIs[] = { {
                     "http://example.com/",
                     "http://example.com/",
                     true,
                 },
                 {
                     "file://localfile.odt/",
                     "file://localfile.odt/",
                     true,
                 },
                 {
                     // tdf 143216
                     "http://username:password@example.com",
                     "http://username:password@example.com",
                     true,
                 },
                 {
                     "git://git.example.org/project/example",
                     "git://git.example.org/project/example",
                     true,
                 },
                 {
                     // The odt/pdf gets substituted due to 'ConvertOOoTargetToPDFTarget'
                     "filebypath.odt",
                     "filebypath.pdf",
                     true,
                 },
                 {
                     // The odt/pdf gets substituted due to 'ConvertOOoTargetToPDFTarget'
                     // but this time with ExportLinksRelativeFsys off the path is added
                     "filebypath.odt",
                     OUStringToOString(utl::GetTempNameBaseDirectory(), RTL_TEXTENCODING_UTF8)
                         + "filebypath.pdf",
                     false,
                 },
                 {
                     // This also gets made relative due to 'ExportLinksRelativeFsys'
                     utl::GetTempNameBaseDirectory() + "fileintempdir.odt",
                     "fileintempdir.pdf",
                     true,
                 } };

    // Create an empty document.
    // Note: The test harness gets very upset if we try and create multiple
    // documents, or recreate it; so reuse one instance for all the links
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "Test pdf", /*bAbsorb=*/false);

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
        aMediaDescriptor["FilterData"] <<= aFilterData;

        // Add a link (based on testNestedHyperlink in rtfexport3)
        xCursor->gotoStart(/*bExpand=*/false);
        xCursor->gotoEnd(/*bExpand=*/true);
        uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
        xCursorProps->setPropertyValue("HyperLinkURL", uno::Any(URIs[i].in));

        // Save as PDF.
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
        xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

        // Use the filter rather than the pdfium route, as per the tdf105093 test, it's
        // easier to parse the annotations
        vcl::filter::PDFDocument aDocument;

        // Parse the export result.
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
        CPPUNIT_ASSERT(aDocument.Read(aStream));

        // The document has one page.
        std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
        auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
        CPPUNIT_ASSERT(pAnnots);

        // There should be one annotation
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnnots->GetElements().size());
        auto pAnnotReference
            = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
        CPPUNIT_ASSERT(pAnnotReference);
        vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
        CPPUNIT_ASSERT(pAnnot);
        // We're expecting something like /Type /Annot /A << /Type /Action /S /URI /URI (path)
        CPPUNIT_ASSERT_EQUAL(
            OString("Annot"),
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"))->GetValue());
        CPPUNIT_ASSERT_EQUAL(
            OString("Link"),
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"))->GetValue());
        auto pAction = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"));
        CPPUNIT_ASSERT(pAction);
        auto pURIElem
            = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pAction->LookupElement("URI"));
        CPPUNIT_ASSERT(pURIElem);
        // Check it matches
        CPPUNIT_ASSERT_EQUAL(URIs[i].out, pURIElem->GetValue());
        // tdf#148934 check a11y
        CPPUNIT_ASSERT_EQUAL(
            OUString("Test pdf"),
            ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                *dynamic_cast<vcl::filter::PDFHexStringElement*>(pAnnot->Lookup("Contents"))));
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testPdfImageAnnots)
{
    // Given a document with a PDF image that has 2 comments (popup, text) and a hyperlink:
    aMediaDescriptor["FilterName"] <<= OUString("draw_pdf_Export");

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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testPdfImageEncryption)
{
    // Given an empty document, with an inserted PDF image:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphicObject(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    OUString aURL = createFileURL(u"rectangles.pdf");
    xGraphicObject->setPropertyValue("GraphicURL", uno::Any(aURL));
    uno::Reference<drawing::XShape> xShape(xGraphicObject, uno::UNO_QUERY);
    xShape->setSize(awt::Size(1000, 1000));
    uno::Reference<text::XTextContent> xTextContent(xGraphicObject, uno::UNO_QUERY);
    xText->insertTextContent(xCursor->getStart(), xTextContent, /*bAbsorb=*/false);

    // When saving as encrypted PDF:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    uno::Sequence<beans::PropertyValue> aFilterData = {
        comphelper::makePropertyValue("EncryptFile", true),
        comphelper::makePropertyValue("DocumentOpenPassword", OUString("secret")),
    };
    aMediaDescriptor["FilterData"] <<= aFilterData;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Then make sure that the image is not lost:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport("secret");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testBitmapScaledown)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // Given a document with an upscaled and rotated barcode bitmap in it:
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf139627)
{
#if HAVE_MORE_FONTS
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
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

    CPPUNIT_ASSERT_EQUAL(OUString(u"رم"), sText[rehmim].trim());
    CPPUNIT_ASSERT_EQUAL(OUString(u""), sText[kasreh].trim());
    CPPUNIT_ASSERT_EQUAL(OUString(u""), sText[jehtatweel].trim());

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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testRexportRefToKids)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        osl_setEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData, OUString("1").pData);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            osl_clearEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData);
    });

    // Load the PDF and save as PDF
    vcl::filter::PDFDocument aDocument;
    load(u"ref-to-kids.pdf", aDocument);

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(5), aPages.size());

    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);

    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);

    // Without the fix LookupObject for all /Im's will fail.
    for (auto const& rPair : pXObjects->GetItems())
    {
        if (rPair.first.startsWith("Im"))
            CPPUNIT_ASSERT(pXObjects->LookupObject(rPair.first));
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testRexportFilterSingletonArray)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        osl_setEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData, OUString("1").pData);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            osl_clearEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData);
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

    auto pFilter = dynamic_cast<vcl::filter::PDFNameElement*>(pInnerIm->Lookup("Filter"));
    CPPUNIT_ASSERT(pFilter);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Filter must be FlateDecode", OString("FlateDecode"),
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
    OString aImage = "100 0 30 50 re B*\n70 67 50 30 re B*\n";
    auto it = std::search(pStart, pEnd, aImage.getStr(), aImage.getStr() + aImage.getLength());
    CPPUNIT_ASSERT(it != pEnd);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testRexportMediaBoxOrigin)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        osl_setEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData, OUString("1").pData);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            osl_clearEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData);
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

    auto pBBox = dynamic_cast<vcl::filter::PDFArrayElement*>(pInnerIm->Lookup("BBox"));
    CPPUNIT_ASSERT(pBBox);
    const auto& rElements2 = pBBox->GetElements();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements2.size());
    for (sal_Int32 nIdx = 0; nIdx < 4; ++nIdx)
    {
        const auto* pNumElement = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements2[nIdx]);
        CPPUNIT_ASSERT(pNumElement);
        if (nIdx < 2)
            CPPUNIT_ASSERT_EQUAL(aOrigin[nIdx], static_cast<sal_Int32>(pNumElement->GetValue()));
        else
            aSize[nIdx - 2] = static_cast<sal_Int32>(pNumElement->GetValue()) - aOrigin[nIdx - 2];
    }

    auto pMatrix = dynamic_cast<vcl::filter::PDFArrayElement*>(pInnerIm->Lookup("Matrix"));
    CPPUNIT_ASSERT(pMatrix);
    const auto& rElements = pMatrix->GetElements();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), rElements.size());
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testRexportResourceItemReference)
{
    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        osl_setEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData, OUString("1").pData);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            osl_clearEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData);
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
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pInnerIm->Lookup("Resources"));
    CPPUNIT_ASSERT(pResources);
    auto pFontsReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pResources->LookupElement("Font"));
    CPPUNIT_ASSERT(pFontsReference);

    auto pFontsObject = pFontsReference->LookupObject();
    CPPUNIT_ASSERT(pFontsObject);

    auto pFontDict
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pFontsObject->Lookup("FF132"));
    CPPUNIT_ASSERT(pFontDict);

    auto pFontDescriptor = pFontDict->LookupObject("FontDescriptor");
    CPPUNIT_ASSERT(pFontDescriptor);

    auto pFontWidths = pFontDict->LookupObject("Widths");
    CPPUNIT_ASSERT(pFontWidths);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf152246)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"content-control-rtl.docx");

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // Position array
    constexpr double aPos[5][4] = { { 56.699, 707.701, 131.401, 721.499 },
                                    { 198.499, 707.701, 273.201, 721.499 },
                                    { 303.349, 680.101, 378.051, 693.899 },
                                    { 480.599, 680.101, 555.301, 693.899 },
                                    { 56.699, 652.501, 131.401, 666.299 } };

    // Get page annotations.
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), pAnnots->GetElements().size());
    for (sal_Int32 i = 0; i < 5; ++i)
    {
        auto pAnnotReference
            = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[i]);
        CPPUNIT_ASSERT(pAnnotReference);
        vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
        CPPUNIT_ASSERT(pAnnot);
        CPPUNIT_ASSERT_EQUAL(
            OString("Annot"),
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"))->GetValue());
        CPPUNIT_ASSERT_EQUAL(
            OString("Widget"),
            static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"))->GetValue());

        auto pRect = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"));
        CPPUNIT_ASSERT(pRect);
        const auto& rElements = pRect->GetElements();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rElements.size());
        for (sal_Int32 nIdx = 0; nIdx < 4; ++nIdx)
        {
            const auto* pNumElement = dynamic_cast<vcl::filter::PDFNumberElement*>(rElements[nIdx]);
            CPPUNIT_ASSERT(pNumElement);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(aPos[i][nIdx], pNumElement->GetValue(), 1e-6);
        }
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf48707_1)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf48707_2)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf156528)
{
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
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

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
