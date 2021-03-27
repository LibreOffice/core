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
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <tools/zcodec.hxx>
#include <tools/XmlWalker.hxx>
#include <vcl/graphicfilter.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <unotools/streamwrap.hxx>

#include <vcl/filter/PDFiumLibrary.hxx>

using namespace ::com::sun::star;

static std::ostream& operator<<(std::ostream& rStrm, const Color& rColor)
{
    rStrm << "Color: R:" << static_cast<int>(rColor.GetRed())
          << " G:" << static_cast<int>(rColor.GetGreen())
          << " B:" << static_cast<int>(rColor.GetBlue())
          << " A:" << static_cast<int>(255 - rColor.GetAlpha());
    return rStrm;
}

namespace
{
/// Tests the PDF export filter.
class PdfExportTest : public test::BootstrapFixture, public unotest::MacrosTest
{
protected:
    uno::Reference<lang::XComponent> mxComponent;
    utl::TempFile maTempFile;
    SvMemoryStream maMemory;
    // Export the document as PDF, then parse it with PDFium.
    std::unique_ptr<vcl::pdf::PDFiumDocument>
    exportAndParse(const OUString& rURL, const utl::MediaDescriptor& rDescriptor);
    std::shared_ptr<vcl::pdf::PDFium> mpPDFium;

public:
    PdfExportTest();
    virtual void setUp() override;
    virtual void tearDown() override;
    void saveAsPDF(std::u16string_view rFile);
    void load(std::u16string_view rFile, vcl::filter::PDFDocument& rDocument);
};

PdfExportTest::PdfExportTest() { maTempFile.EnableKillingFile(); }

std::unique_ptr<vcl::pdf::PDFiumDocument>
PdfExportTest::exportAndParse(const OUString& rURL, const utl::MediaDescriptor& rDescriptor)
{
    // Import the bugdoc and export as PDF.
    mxComponent = loadFromDesktop(rURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), rDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    maMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(maMemory.GetData(), maMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);
    return pPdfDocument;
}

void PdfExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));

    mpPDFium = vcl::pdf::PDFiumLibrary::get();
}

void PdfExportTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

constexpr OUStringLiteral DATA_DIRECTORY = u"/vcl/qa/cppunit/pdfexport/data/";

void PdfExportTest::saveAsPDF(std::u16string_view rFile)
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + rFile;
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
}

void PdfExportTest::load(std::u16string_view rFile, vcl::filter::PDFDocument& rDocument)
{
    saveAsPDF(rFile);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(rDocument.Read(aStream));
}

/// Tests that a pdf image is roundtripped back to PDF as a vector format.
CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf106059)
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106059.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    // Explicitly enable the usage of the reference XObject markup.
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "UseReferenceXObject", uno::Any(true) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf105461.odp";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf107868.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XPrintable> xPrintable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPrintable.is());
    uno::Sequence<beans::PropertyValue> aOptions(comphelper::InitPropertySequence(
        { { "FileName", uno::makeAny(maTempFile.GetURL()) }, { "Wait", uno::makeAny(true) } }));
    xPrintable->print(aOptions);

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106206.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf127217.odt";
    mxComponent = loadFromDesktop(aURL);

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf109143.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106972.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106972-pdf17.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "softhyphen_pdf.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XPrintable> xPrintable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPrintable.is());
    uno::Sequence<beans::PropertyValue> aOptions(comphelper::InitPropertySequence(
        { { "FileName", uno::makeAny(maTempFile.GetURL()) }, { "Wait", uno::makeAny(true) } }));
    xPrintable->print(aOptions);

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    if (aFile.bad() || !aMemory.GetSize())
    {
        // Printing to PDF failed in a non-interesting way, e.g. CUPS is not
        // running, there is no printer defined, etc.
        return;
    }
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf108963.odp";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // FIXME: strangely this fails on some Win systems after a pdfium update, expected: 793.7; actual: 793
#if !defined _WIN32
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
            CPPUNIT_ASSERT_EQUAL(245395, static_cast<int>(round(aPoint.getX() * 1000)));
            CPPUNIT_ASSERT_EQUAL(244261, static_cast<int>(round(aPoint.getY() * 1000)));
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(1);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_EQUAL(275102, static_cast<int>(round(aPoint.getX() * 1000)));
            CPPUNIT_ASSERT_EQUAL(267618, static_cast<int>(round(aPoint.getY() * 1000)));
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(2);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_EQUAL(287518, static_cast<int>(round(aPoint.getX() * 1000)));
            CPPUNIT_ASSERT_EQUAL(251829, static_cast<int>(round(aPoint.getY() * 1000)));
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(3);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_EQUAL(257839, static_cast<int>(round(aPoint.getX() * 1000)));
            CPPUNIT_ASSERT_EQUAL(228472, static_cast<int>(round(aPoint.getY() * 1000)));
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(4);
            CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFSegmentType::Lineto, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_EQUAL(245395, static_cast<int>(round(aPoint.getX() * 1000)));
            CPPUNIT_ASSERT_EQUAL(244261, static_cast<int>(round(aPoint.getY() * 1000)));
            CPPUNIT_ASSERT(pSegment->isClosed());
        }
    }

    CPPUNIT_ASSERT_EQUAL(1, nYellowPathCount);
#endif
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf118244_radioButtonGroup)
{
    vcl::filter::PDFDocument aDocument;
    load(u"tdf118244_radioButtonGroup.odt", aDocument);

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf115117-1.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf115117-2.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);

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
        // The <01> is glyph id, <0020> is code point.
        // The document has three characters <space><nbspace><space>, but the font
        // reuses the same glyph for space and nbspace so we should have a single
        // CMAP entry for the space, and nbspace will be handled with ActualText
        // (tested above).
        std::string aCmap("1 beginbfchar\n"
                          "<01> <0020>\n"
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

        aActualText = "/Span<</ActualText<FEFF00A0>>>";
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
                          "<09> <0628>\n"
                          "<0B> <0623>\n"
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf105954.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "ReduceImageResolution", uno::Any(true) },
          { "MaxImageResolution", uno::Any(static_cast<sal_Int32>(300)) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);

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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf128630)
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf128630.odp";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("impress_pdf_Export");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = exportAndParse(aURL, aMediaDescriptor);

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Assert the aspect ratio of the only bitmap on the page.
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
        int nHeight = pBitmap->getHeight();
        // Without the accompanying fix in place, this test would have failed with:
        // assertion failed
        // - Expression: nWidth != nHeight
        // i.e. the bitmap lost its custom aspect ratio during export.
        CPPUNIT_ASSERT(nWidth != nHeight);
    }
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf106702)
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106702.odt";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    auto pPdfDocument = exportAndParse(aURL, aMediaDescriptor);

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf113143.odp";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("impress_pdf_Export");
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        { "ExportNotesPages", uno::Any(true) },
        // ReduceImageResolution is on by default and that hides the bug we
        // want to test.
        { "ReduceImageResolution", uno::Any(false) },
        // Set a custom PDF version.
        { "SelectPdfVersion", uno::makeAny(static_cast<sal_Int32>(16)) },
    }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    auto pPdfDocument = exportAndParse(aURL, aMediaDescriptor);

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
    saveAsPDF(u"forcepoint71.key");
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf115262)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf115262.ods";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("calc_pdf_Export");
    auto pPdfDocument = exportAndParse(aURL, aMediaDescriptor);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf121962.odt";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    auto pPdfDocument = exportAndParse(aURL, aMediaDescriptor);
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf115967)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf115967.odt";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    auto pPdfDocument = exportAndParse(aURL, aMediaDescriptor);
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
        sText += sChar.trim();
    }
    CPPUNIT_ASSERT_EQUAL(OUString("m=750abc"), sText);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testTdf124272)
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf124272.odt";
    mxComponent = loadFromDesktop(aURL);

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, OUString("import"), rObjectStream,
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
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, OUString("import"), rObjectStream,
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
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, OUString("import"), rObjectStream,
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "toc-link.fodt";
    mxComponent = loadFromDesktop(aURL);

    // Update the ToC.
    uno::Reference<text::XDocumentIndexesSupplier> xDocumentIndexesSupplier(mxComponent,
                                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDocumentIndexesSupplier.is());

    uno::Reference<util::XRefreshable> xToc(
        xDocumentIndexesSupplier->getDocumentIndexes()->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xToc.is());

    xToc->refresh();

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    maMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(maMemory.GetData(), maMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "reduce-small-image.fodt";
    mxComponent = loadFromDesktop(aURL);

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the PDF: get the image.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    maMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(maMemory.GetData(), maMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);
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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testReduceImage)
{
    // Load the Writer document.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "reduce-image.fodt";
    mxComponent = loadFromDesktop(aURL);

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
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    maMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(maMemory.GetData(), maMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "link-wrong-page.odp";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("impress_pdf_Export");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = exportAndParse(aURL, aMediaDescriptor);

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

CPPUNIT_TEST_FIXTURE(PdfExportTest, testLargePage)
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "6m-wide.odg";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("draw_pdf_Export");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = exportAndParse(aURL, aMediaDescriptor);

    // The document has 1 page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    // Check the value (not the unit) of the page size.
    basegfx::B2DSize aSize = pPdfDocument->getPageSize(0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 8503.94
    // - Actual  : 17007.875
    // i.e. the value for 600 cm was larger than the 14 400 limit set in the spec.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8503.94, aSize.getX(), 0.01);
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
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "pdf-image-resource-inline-xobject-ref.pdf";
    xGraphicObject->setPropertyValue("GraphicURL", uno::makeAny(aURL));
    uno::Reference<drawing::XShape> xShape(xGraphicObject, uno::UNO_QUERY);
    xShape->setSize(awt::Size(1000, 1000));
    uno::Reference<text::XTextContent> xTextContent(xGraphicObject, uno::UNO_QUERY);
    xText->insertTextContent(xCursor->getStart(), xTextContent, /*bAbsorb=*/false);

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    maMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(maMemory.GetData(), maMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);
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
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    maMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(maMemory.GetData(), maMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);
    int nFileVersion = pPdfDocument->getFileVersion();
    CPPUNIT_ASSERT_EQUAL(16, nFileVersion);
}

CPPUNIT_TEST_FIXTURE(PdfExportTest, testVersion15)
{
    // Create an empty document.
    mxComponent = loadFromDesktop("private:factory/swriter");

    // Save as PDF.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "SelectPdfVersion", uno::makeAny(static_cast<sal_Int32>(15)) } }));
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    aMediaDescriptor["FilterData"] <<= aFilterData;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    maMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(maMemory.GetData(), maMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "SimpleMultiPagePDF.pdf";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
        CPPUNIT_ASSERT_EQUAL(OString("Im19"), rIDs[0]);

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
        CPPUNIT_ASSERT_EQUAL(OString("Im24"), rIDs[1]);

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
        CPPUNIT_ASSERT_EQUAL(OString("Im4"), rIDs[2]);

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "form-font-name.odt";
    mxComponent = loadFromDesktop(aURL);

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "PDFWithImages.pdf";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

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
            CPPUNIT_ASSERT_EQUAL(OString("Im12"), aID);
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
            CPPUNIT_ASSERT_EQUAL(OString("Im13"), aInnerObjectID);

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
            CPPUNIT_ASSERT_EQUAL(OString("Im4"), aID);
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
            CPPUNIT_ASSERT_EQUAL(OString("Im5"), aInnerObjectID);

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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "ComplexContentDictionary.pdf";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // Assert that the XObject in the page resources dictionary is a reference XObject.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    // Go directly to the Font object (24 0) (number could change if we change how PDF export works)
    auto pFont = aDocument.LookupObject(23);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "BrownFoxLazyDog.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");

    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
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

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
