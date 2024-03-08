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

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/view/XPrintable.hpp>

#include <comphelper/propertysequence.hxx>
#include <test/unoapi_test.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <tools/zcodec.hxx>
#include <o3tl/string_view.hxx>

#include <vcl/filter/PDFiumLibrary.hxx>

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
    loadFromFile(rFile);
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
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    // The page has one image.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pReferenceXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pReferenceXObject);
    // The image is a reference XObject.
    // This dictionary key was missing, so the XObject wasn't a reference one.
    CPPUNIT_ASSERT(pReferenceXObject->Lookup("Ref"_ostr));
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
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    // The page has one image.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);
    // The image is a form XObject.
    auto pSubtype = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject->Lookup("Subtype"_ostr));
    CPPUNIT_ASSERT(pSubtype);
    CPPUNIT_ASSERT_EQUAL("Form"_ostr, pSubtype->GetValue());
    // This failed: UseReferenceXObject was ignored and Ref was always created.
    CPPUNIT_ASSERT(!pXObject->Lookup("Ref"_ostr));

    // Assert that the form object refers to an inner form object, not a
    // bitmap.
    auto pInnerResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"_ostr));
    CPPUNIT_ASSERT(pInnerResources);
    auto pInnerXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
        pInnerResources->LookupElement("XObject"_ostr));
    CPPUNIT_ASSERT(pInnerXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pInnerXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pInnerXObject
        = pInnerXObjects->LookupObject(pInnerXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pInnerXObject);
    auto pInnerSubtype
        = dynamic_cast<vcl::filter::PDFNameElement*>(pInnerXObject->Lookup("Subtype"_ostr));
    CPPUNIT_ASSERT(pInnerSubtype);
    // This failed: this was Image (bitmap), not Form (vector).
    CPPUNIT_ASSERT_EQUAL("Form"_ostr, pInnerSubtype->GetValue());
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
    loadFromFile(u"tdf107868.odt");
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
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnnots->GetElements().size());
    auto pAnnotReference
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(
        "Annot"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());

    // Get the Action -> Rendition -> MediaClip -> FileSpec.
    auto pAction = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
    CPPUNIT_ASSERT(pAction);
    auto pRendition
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAction->LookupElement("R"_ostr));
    CPPUNIT_ASSERT(pRendition);
    auto pMediaClip
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pRendition->LookupElement("C"_ostr));
    CPPUNIT_ASSERT(pMediaClip);
    auto pFileSpec
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pMediaClip->LookupElement("D"_ostr));
    CPPUNIT_ASSERT(pFileSpec);
    // Make sure the filespec refers to an embedded file.
    // This key was missing, the embedded video was handled as a linked one.
    CPPUNIT_ASSERT(pFileSpec->LookupElement("EF"_ostr));
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

    // Make sure there is an image reference there.
    OString aImage("/Im"_ostr);
    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* pEnd = pStart + aUncompressed.GetSize();
    auto it = std::search(pStart, pEnd, aImage.getStr(), aImage.getStr() + aImage.getLength());
    CPPUNIT_ASSERT(it != pEnd);

    // And also that it's not an invalid one.
    OString aInvalidImage("/Im0"_ostr);
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
    CPPUNIT_ASSERT(!pAnnot->hasKey("DA"_ostr));
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
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Make sure it's re-compressed.
    auto pLength = dynamic_cast<vcl::filter::PDFNumberElement*>(pXObject->Lookup("Length"_ostr));
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
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Get access to the only image inside the form object.
    auto pFormResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"_ostr));
    CPPUNIT_ASSERT(pFormResources);
    auto pImages = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
        pFormResources->LookupElement("XObject"_ostr));
    CPPUNIT_ASSERT(pImages);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pImages->GetItems().size());
    vcl::filter::PDFObjectElement* pImage
        = pImages->LookupObject(pImages->GetItems().begin()->first);
    CPPUNIT_ASSERT(pImage);

    // Assert resources of the image.
    auto pImageResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pImage->Lookup("Resources"_ostr));
    CPPUNIT_ASSERT(pImageResources);
    // This failed: the PDF image had no Font resource.
    CPPUNIT_ASSERT(pImageResources->LookupElement("Font"_ostr));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf106972Pdf17)
{
    // Import the bugdoc and export as PDF.
    vcl::filter::PDFDocument aDocument;
    load(u"tdf106972-pdf17.odt", aDocument);

    // Get access to the only image on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Assert that we now attempt to preserve the original PDF data, even if
    // the original input was PDF >= 1.4.
    CPPUNIT_ASSERT(pXObject->Lookup("Resources"_ostr));
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testSofthyphenPos)
{
// No need to run it on Windows, since it would use GDI printing, and not trigger PDF export
// which is the intent of the test.
// FIXME: Why does this fail on macOS?
#if !defined MACOSX && !defined _WIN32

    // Import the bugdoc and print to PDF.
    loadFromFile(u"softhyphen_pdf.odt");
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
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
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
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Get access to the form object inside the image.
    auto pXObjectResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"_ostr));
    CPPUNIT_ASSERT(pXObjectResources);
    auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
        pXObjectResources->LookupElement("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjectForms);
    vcl::filter::PDFObjectElement* pForm
        = pXObjectForms->LookupObject(pXObjectForms->GetItems().begin()->first);
    CPPUNIT_ASSERT(pForm);

    // Get access to Resources -> Font -> F1 of the form.
    auto pFormResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pForm->Lookup("Resources"_ostr));
    CPPUNIT_ASSERT(pFormResources);
    auto pFonts = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
        pFormResources->LookupElement("Font"_ostr));
    CPPUNIT_ASSERT(pFonts);
    auto pF1Ref = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFonts->LookupElement("F1"_ostr));
    CPPUNIT_ASSERT(pF1Ref);
    vcl::filter::PDFObjectElement* pF1 = pF1Ref->LookupObject();
    CPPUNIT_ASSERT(pF1);

    // Check that Foo -> Bar of the font is of type Pages.
    auto pFontFoo = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pF1->Lookup("Foo"_ostr));
    CPPUNIT_ASSERT(pFontFoo);
    auto pBar
        = dynamic_cast<vcl::filter::PDFReferenceElement*>(pFontFoo->LookupElement("Bar"_ostr));
    CPPUNIT_ASSERT(pBar);
    vcl::filter::PDFObjectElement* pObject = pBar->LookupObject();
    CPPUNIT_ASSERT(pObject);
    auto pName = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
    CPPUNIT_ASSERT(pName);
    // This was "XObject", reference in a nested dictionary wasn't updated when
    // copying the page stream of a PDF image.
    CPPUNIT_ASSERT_EQUAL("Pages"_ostr, pName->GetValue());
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

    CPPUNIT_ASSERT(pAnnot->hasKey("V"_ostr));
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFObjectType::String, pAnnot->getValueType("V"_ostr));
    OUString aV = pAnnot->getString("V"_ostr);

    // Without the fix in place, this test would have failed with
    // - Expected: 1821.84
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("1821.84"), aV);

    CPPUNIT_ASSERT(pAnnot->hasKey("DV"_ostr));
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFObjectType::String, pAnnot->getValueType("DV"_ostr));
    OUString aDV = pAnnot->getString("DV"_ostr);

    CPPUNIT_ASSERT_EQUAL(OUString("1821.84"), aDV);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf107089)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf107089.odt", aDocument);

    // Get access to the only image on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources"_ostr);
    CPPUNIT_ASSERT(pResources);
    auto pXObjects
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"_ostr));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject
        = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Get access to the form object inside the image.
    auto pXObjectResources
        = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"_ostr));
    CPPUNIT_ASSERT(pXObjectResources);
    auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(
        pXObjectResources->LookupElement("XObject"_ostr));
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
    OString aHello("Hello"_ostr);
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
        vcl::filter::PDFObjectElement* pContents = aPages[nPageNr]->LookupObject("Contents"_ostr);
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
            CPPUNIT_ASSERT_DOUBLES_EQUAL(245, aPoint.getX(), 0.999);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(244, aPoint.getY(), 0.999);
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(1);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(275, aPoint.getX(), 0.999);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(267, aPoint.getY(), 0.999);
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(2);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(287, aPoint.getX(), 0.999);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(251, aPoint.getY(), 0.999);
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(3);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(257, aPoint.getX(), 0.999);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(228, aPoint.getY(), 0.999);
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(4);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(245, aPoint.getX(), 0.999);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(244, aPoint.getY(), 0.999);
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
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "StructElem")
        {
            auto pS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("S"_ostr));
            if (pS && pS->GetValue() == "Figure")
            {
                CPPUNIT_ASSERT_EQUAL(u"This is the text alternative - This is the description"_ustr,
                                     ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(
                                         *dynamic_cast<vcl::filter::PDFHexStringElement*>(
                                             pObject->Lookup("Alt"_ostr))));
            }
        }
    }

    // tdf#67866 check that Catalog contains Lang
    auto* pCatalog = aDocument.GetCatalog();
    CPPUNIT_ASSERT(pCatalog);
    auto* pCatalogDictionary = pCatalog->GetDictionary();
    CPPUNIT_ASSERT(pCatalogDictionary);
    auto pLang = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
        pCatalogDictionary->LookupElement("Lang"_ostr));
    CPPUNIT_ASSERT(pLang);
    CPPUNIT_ASSERT_EQUAL("en-US"_ostr, pLang->GetValue());
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf105972)
{
    vcl::filter::PDFDocument aDocument;
    // Loading fails with tagged PDF enabled
    load(u"tdf105972.fodt", aDocument, false);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pAnnots->GetElements().size());

    sal_uInt32 nTextFieldCount = 0;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("FT"_ostr));
        if (pType && pType->GetValue() == "Tx")
        {
            ++nTextFieldCount;

            auto pT
                = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pObject->Lookup("T"_ostr));
            CPPUNIT_ASSERT(pT);
            auto pAA = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject->Lookup("AA"_ostr));
            CPPUNIT_ASSERT(pAA);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pAA->GetItems().size());
            auto pF
                = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAA->LookupElement("F"_ostr));
            CPPUNIT_ASSERT(pF);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pF->GetItems().size());

            if (nTextFieldCount == 1)
            {
                CPPUNIT_ASSERT_EQUAL("CurrencyField"_ostr, pT->GetValue());

                auto pJS = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pF->LookupElement("JS"_ostr));
                CPPUNIT_ASSERT_EQUAL("AFNumber_Format\\(4, 0, 0, 0, \"\\\\u20ac\",true\\);"_ostr,
                                     pJS->GetValue());
            }
            else if (nTextFieldCount == 2)
            {
                CPPUNIT_ASSERT_EQUAL("TimeField"_ostr, pT->GetValue());

                auto pJS = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pF->LookupElement("JS"_ostr));
                CPPUNIT_ASSERT_EQUAL("AFTime_FormatEx\\(\"h:MM:sstt\"\\);"_ostr, pJS->GetValue());
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL("DateField"_ostr, pT->GetValue());

                auto pJS = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(
                    pF->LookupElement("JS"_ostr));
                CPPUNIT_ASSERT_EQUAL("AFDate_FormatEx\\(\"yy-mm-dd\"\\);"_ostr, pJS->GetValue());
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

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pAnnots->GetElements().size());

    sal_uInt32 nBtnCount = 0;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("FT"_ostr));
        if (pType && pType->GetValue() == "Btn")
        {
            ++nBtnCount;
            auto pT
                = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pObject->Lookup("T"_ostr));
            CPPUNIT_ASSERT(pT);
            auto pAS = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("AS"_ostr));
            CPPUNIT_ASSERT(pAS);

            auto pAP = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pObject->Lookup("AP"_ostr));
            CPPUNIT_ASSERT(pAP);
            auto pN
                = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAP->LookupElement("N"_ostr));
            CPPUNIT_ASSERT(pN);
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pN->GetItems().size());

            if (nBtnCount == 1)
            {
                CPPUNIT_ASSERT_EQUAL("Checkbox1"_ostr, pT->GetValue());
                CPPUNIT_ASSERT_EQUAL("Yes"_ostr, pAS->GetValue());
                CPPUNIT_ASSERT(!pN->GetItems().count("ref"_ostr));
                CPPUNIT_ASSERT(pN->GetItems().count("Yes"_ostr));
                CPPUNIT_ASSERT(pN->GetItems().count("Off"_ostr));
            }
            else if (nBtnCount == 2)
            {
                CPPUNIT_ASSERT_EQUAL("Checkbox2"_ostr, pT->GetValue());
                CPPUNIT_ASSERT_EQUAL("Yes"_ostr, pAS->GetValue());

                // Without the fix in place, this test would have failed here
                CPPUNIT_ASSERT(pN->GetItems().count("ref"_ostr));
                CPPUNIT_ASSERT(!pN->GetItems().count("Yes"_ostr));
                CPPUNIT_ASSERT(pN->GetItems().count("Off"_ostr));
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL("Checkbox3"_ostr, pT->GetValue());
                CPPUNIT_ASSERT_EQUAL("Off"_ostr, pAS->GetValue());
                CPPUNIT_ASSERT(pN->GetItems().count("ref"_ostr));
                CPPUNIT_ASSERT(!pN->GetItems().count("Yes"_ostr));

                // tdf#143612: Without the fix in place, this test would have failed here
                CPPUNIT_ASSERT(!pN->GetItems().count("Off"_ostr));
                CPPUNIT_ASSERT(pN->GetItems().count("refOff"_ostr));
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
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of radio buttons", static_cast<size_t>(8),
                                 pAnnots->GetElements().size());

    sal_uInt32 nRadioGroups = 0;
    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("FT"_ostr));
        if (pType && pType->GetValue() == "Btn")
        {
            auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject->Lookup("Kids"_ostr));
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
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "Font")
        {
            auto pToUnicodeRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                pObject->Lookup("ToUnicode"_ostr));
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
                  "endbfchar"_ostr);
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
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
        if (pType && pType->GetValue() == "Font")
        {
            auto pToUnicodeRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                pObject->Lookup("ToUnicode"_ostr));
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
                  "endbfchar"_ostr);
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
    CPPUNIT_ASSERT_EQUAL(u"\u0627\u0644 \u0628\u0627\u0644 \u0648\u0642\u0641 \u0627\u0644"_ustr,
                         aActualText);
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
    CPPUNIT_ASSERT_EQUAL(u"\u05B4\u0628\u064A"_ustr, aActualText);
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
    CPPUNIT_ASSERT_EQUAL(u"hello"_ustr, aActualText);
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
    CPPUNIT_ASSERT_EQUAL(u"يوسف My name is"_ustr, aActualText1);

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
    CPPUNIT_ASSERT_EQUAL(u"My name is يوسف"_ustr, aActualText2);
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf156685)
{
    // Import the bugdoc and export as PDF.
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    saveAsPDF(u"tdf156685.docx");

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    auto pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPage);

    int nPageObjectCount = pPage->getObjectCount();

    CPPUNIT_ASSERT_EQUAL(9, nPageObjectCount);

    auto pTextPage = pPage->getTextPage();

    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(i);
        if (pPageObject->getType() != vcl::pdf::PDFPageObjectType::Text)
            continue;

        CPPUNIT_ASSERT_EQUAL(11.0, pPageObject->getFontSize());
        CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFTextRenderMode::Fill, pPageObject->getTextRenderMode());

        // Without the fix in place, this test would have failed with
        // - Expected: rgba[000000ff]
        // - Actual  : rgba[ffffffff]
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, pPageObject->getFillColor());
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
            auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
            if (pType && pType->GetValue() == "Font")
            {
                auto pName
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("BaseFont"_ostr));
                auto aName = pName->GetValue().copy(7); // skip the subset id
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected font name", "Amiri-Regular"_ostr, aName);

                auto pToUnicodeRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pObject->Lookup("ToUnicode"_ostr));
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
        auto pContents = aPages[0]->LookupObject("Contents"_ostr);
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
            auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
            if (pType && pType->GetValue() == "Font")
            {
                auto pName
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("BaseFont"_ostr));
                auto aName = pName->GetValue().copy(7); // skip the subset id
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected font name", "ReemKufi-Regular"_ostr,
                                             aName);

                auto pToUnicodeRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pObject->Lookup("ToUnicode"_ostr));
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
        auto pContents = aPages[0]->LookupObject("Contents"_ostr);
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
            auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"_ostr));
            if (pType && pType->GetValue() == "Font")
            {
                auto pName
                    = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("BaseFont"_ostr));
                auto aName = pName->GetValue().copy(7); // skip the subset id
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected font name", "GentiumBasic"_ostr, aName);

                auto pToUnicodeRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(
                    pObject->Lookup("ToUnicode"_ostr));
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
        auto pContents = aPages[0]->LookupObject("Contents"_ostr);
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
    OString aExpectedHeader("%PDF-1.6"_ostr);
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
    auto pRefKidD2 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsDv[2]);
    CPPUNIT_ASSERT(pRefKidD2);
    auto pObjectD2 = pRefKidD2->LookupObject();
    CPPUNIT_ASSERT(pObjectD2);
    auto pTypeD2 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD2->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD2->GetValue());
    auto pSD2 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD2->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Text#20body"_ostr, pSD2->GetValue());

    auto pKidsD2 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD2->Lookup("K"_ostr));
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
    auto pTypeD22 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD22->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD22->GetValue());
    auto pSD22 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD22->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD22->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD22->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"Error: Reference source not found"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD23 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD23->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD23->GetValue());
    auto pSD23 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD23->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD23->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD23->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"Error: Reference source not found"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD24 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD24->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD24->GetValue());
    auto pSD24 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD24->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD24->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD24->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"Error: Reference source not found"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD25 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD25->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD25->GetValue());
    auto pSD25 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD25->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD25->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD25->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"Error: Reference source not found"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
                CPPUNIT_ASSERT_DOUBLES_EQUAL(174.757, pNumR->GetValue(), 1e-3);
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
    auto pTypeD26 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD26->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD26->GetValue());
    auto pSD26 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD26->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD26->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD26->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"Error: Reference source not found"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
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
    CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pSD0->GetValue());

    auto pKidsD0 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD0->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsD0);
    auto pKidsD0v = pKidsD0->GetElements();

    auto pRefKidD00 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD0v[0]);
    CPPUNIT_ASSERT(pRefKidD00);
    auto pObjectD00 = pRefKidD00->LookupObject();
    CPPUNIT_ASSERT(pObjectD00);
    auto pTypeD00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD00->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD00->GetValue());
    auto pSD00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD00->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD00->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD00->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"https://www.mozilla.org/en-US/firefox/119.0/releasenotes/"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD01 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD01->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD01->GetValue());
    auto pSD01 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD01->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD01->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD01->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"https://www.mozilla.org/en-US/firefox/119.0/releasenotes/"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD02 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD02->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD02->GetValue());
    auto pSD02 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD02->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Figure"_ostr, pSD02->GetValue());

    auto pRefKidD1 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsDv[1]);
    CPPUNIT_ASSERT(pRefKidD1);
    auto pObjectD1 = pRefKidD1->LookupObject();
    CPPUNIT_ASSERT(pObjectD1);
    auto pTypeD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD1->GetValue());
    auto pSD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pSD1->GetValue());

    auto pKidsD1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD1->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsD1);
    auto pKidsD1v = pKidsD1->GetElements();

    auto pRefKidD10 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD1v[0]);
    CPPUNIT_ASSERT(pRefKidD10);
    auto pObjectD10 = pRefKidD10->LookupObject();
    CPPUNIT_ASSERT(pObjectD10);
    auto pTypeD10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD10->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD10->GetValue());
    auto pSD10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD10->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD10->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD10->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"https://www.mozilla.org/en-US/firefox/118.0/releasenotes/"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD11->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD11->GetValue());
    auto pSD11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD11->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD11->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD11->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"https://www.mozilla.org/en-US/firefox/118.0/releasenotes/"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
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

    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
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
        "Annot"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"_ostr))->GetValue());
    CPPUNIT_ASSERT_EQUAL(
        "Link"_ostr,
        static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Subtype"_ostr))->GetValue());
    auto pAction = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"_ostr));
    CPPUNIT_ASSERT(pAction);
    auto pURIElem
        = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pAction->LookupElement("URI"_ostr));
    CPPUNIT_ASSERT(pURIElem);
    // Check it matches
    CPPUNIT_ASSERT_EQUAL("https://google.com/"_ostr, pURIElem->GetValue());
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
    CPPUNIT_ASSERT(pRefKidD00);
    auto pObjectD00 = pRefKidD00->LookupObject();
    CPPUNIT_ASSERT(pObjectD00);
    auto pTypeD00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD00->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD00->GetValue());
    auto pSD00 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD00->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD00->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD00->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"foo foo foo foo"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD01 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD01->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD01->GetValue());
    auto pSD01 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD01->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD01->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD01->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"foo foo foo foo"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD02 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD02->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD02->GetValue());
    auto pSD02 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD02->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD02->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD02->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"foo foo foo foo"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD03 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD03->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD03->GetValue());
    auto pSD03 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD03->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD03->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD03->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"foo foo foo foo"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD1->GetValue());
    auto pSD1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD1->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Text#20body"_ostr, pSD1->GetValue());

    auto pKidsD1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD1->Lookup("K"_ostr));
    CPPUNIT_ASSERT(pKidsD1);
    auto pKidsD1v = pKidsD1->GetElements();

    auto pRefKidD10 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKidsD1v[0]);
    CPPUNIT_ASSERT(pRefKidD10);
    auto pObjectD10 = pRefKidD10->LookupObject();
    CPPUNIT_ASSERT(pObjectD10);
    auto pTypeD10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD10->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD10->GetValue());
    auto pSD10 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD10->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD10->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD10->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"foo foo foo foo"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD11->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD11->GetValue());
    auto pSD11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD11->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD11->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD11->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"foo foo foo foo"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD12 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD12->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD12->GetValue());
    auto pSD12 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD12->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD12->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD12->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"foo foo foo foo"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pTypeD13 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD13->Lookup("Type"_ostr));
    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pTypeD13->GetValue());
    auto pSD13 = dynamic_cast<vcl::filter::PDFNameElement*>(pObjectD13->Lookup("S"_ostr));
    CPPUNIT_ASSERT_EQUAL("Link"_ostr, pSD13->GetValue());
    {
        auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObjectD13->Lookup("K"_ostr));
        auto nMCID(0);
        auto nRef(0);
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
                    u"foo foo foo foo"_ustr,
                    ::vcl::filter::PDFDocument::DecodeHexStringUTF16BE(*pAContents));
                auto pStructParent = dynamic_cast<vcl::filter::PDFNumberElement*>(
                    pAnnot->Lookup("StructParent"_ostr));
                CPPUNIT_ASSERT(pStructParent); // every link must have it!
                auto pARect
                    = dynamic_cast<vcl::filter::PDFArrayElement*>(pAnnot->Lookup("Rect"_ostr));
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
    auto pAnnots0 = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pAnnots0->GetElements().size());
    auto pAnnots1 = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[1]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots1);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pAnnots1->GetElements().size());
    auto pAnnots2 = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[2]->Lookup("Annots"_ostr));
    CPPUNIT_ASSERT(pAnnots2);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pAnnots2->GetElements().size());
    auto pAnnots3 = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[3]->Lookup("Annots"_ostr));
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

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
