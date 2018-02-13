/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/view/XPrintable.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <tools/zcodec.hxx>
#if HAVE_FEATURE_PDFIUM
#include <fpdf_edit.h>
#include <fpdfview.h>
#endif

using namespace ::com::sun::star;

namespace
{

/// Tests the PDF export filter.
class PdfExportTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;
#if HAVE_FEATURE_PDFIUM
    FPDF_PAGE mpPdfPage = nullptr;
    FPDF_DOCUMENT mpPdfDocument = nullptr;
#endif

public:
    virtual void setUp() override;
    virtual void tearDown() override;
#if HAVE_FEATURE_PDFIUM
    void load(const OUString& rFile, vcl::filter::PDFDocument& rDocument);
    /// Tests that a pdf image is roundtripped back to PDF as a vector format.
    void testTdf106059();
    /// Tests that text highlight from Impress is not lost.
    void testTdf105461();
    void testTdf107868();
    /// Tests that embedded video from Impress is not exported as a linked one.
    void testTdf105093();
    /// Tests export of non-PDF images.
    void testTdf106206();
    /// Tests export of PDF images without reference XObjects.
    void testTdf106693();
    void testTdf106972();
    void testTdf106972Pdf17();
    void testTdf107013();
    void testTdf107018();
    void testTdf107089();
    void testTdf99680();
    void testTdf99680_2();
    void testTdf108963();
#endif

    CPPUNIT_TEST_SUITE(PdfExportTest);
#if HAVE_FEATURE_PDFIUM
    CPPUNIT_TEST(testTdf106059);
    CPPUNIT_TEST(testTdf105461);
    CPPUNIT_TEST(testTdf107868);
    CPPUNIT_TEST(testTdf105093);
    CPPUNIT_TEST(testTdf106206);
    CPPUNIT_TEST(testTdf106693);
    CPPUNIT_TEST(testTdf106972);
    CPPUNIT_TEST(testTdf106972Pdf17);
    CPPUNIT_TEST(testTdf107013);
    CPPUNIT_TEST(testTdf107018);
    CPPUNIT_TEST(testTdf107089);
    CPPUNIT_TEST(testTdf99680);
    CPPUNIT_TEST(testTdf99680_2);
    CPPUNIT_TEST(testTdf108963);
#endif
    CPPUNIT_TEST_SUITE_END();
};

void PdfExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));

#if HAVE_FEATURE_PDFIUM
    FPDF_LIBRARY_CONFIG config;
    config.version = 2;
    config.m_pUserFontPaths = nullptr;
    config.m_pIsolate = nullptr;
    config.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&config);
#endif
}

void PdfExportTest::tearDown()
{
#if HAVE_FEATURE_PDFIUM
    FPDF_ClosePage(mpPdfPage);
    FPDF_CloseDocument(mpPdfDocument);
    FPDF_DestroyLibrary();
#endif

    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

#if HAVE_FEATURE_PDFIUM

char const DATA_DIRECTORY[] = "/vcl/qa/cppunit/pdfexport/data/";

void PdfExportTest::load(const OUString& rFile, vcl::filter::PDFDocument& rDocument)
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + rFile;
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    SvFileStream aStream(aTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(rDocument.Read(aStream));
}

void PdfExportTest::testTdf106059()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106059.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    // Explicitly enable the usage of the reference XObject markup.
    uno::Sequence<beans::PropertyValue> aFilterData( comphelper::InitPropertySequence({
        {"UseReferenceXObject", uno::Any(true) }
    }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(aTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // Assert that the XObject in the page resources dictionary is a reference XObject.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    // The page has one image.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pReferenceXObject = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pReferenceXObject);
    // The image is a reference XObject.
    // This dictionary key was missing, so the XObject wasn't a reference one.
    CPPUNIT_ASSERT(pReferenceXObject->Lookup("Ref"));
}

void PdfExportTest::testTdf106693()
{
    vcl::filter::PDFDocument aDocument;
    load("tdf106693.odt", aDocument);

    // Assert that the XObject in the page resources dictionary is a form XObject.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    // The page has one image.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);
    // The image is a form XObject.
    auto pSubtype = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject->Lookup("Subtype"));
    CPPUNIT_ASSERT(pSubtype);
    CPPUNIT_ASSERT_EQUAL(OString("Form"), pSubtype->GetValue());
    // This failed: UseReferenceXObject was ignored and Ref was always created.
    CPPUNIT_ASSERT(!pXObject->Lookup("Ref"));

    // Assert that the form object refers to an inner form object, not a
    // bitmap.
    auto pInnerResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"));
    CPPUNIT_ASSERT(pInnerResources);
    auto pInnerXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pInnerResources->LookupElement("XObject"));
    CPPUNIT_ASSERT(pInnerXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pInnerXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pInnerXObject = pInnerXObjects->LookupObject(pInnerXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pInnerXObject);
    auto pInnerSubtype = dynamic_cast<vcl::filter::PDFNameElement*>(pInnerXObject->Lookup("Subtype"));
    CPPUNIT_ASSERT(pInnerSubtype);
    // This failed: this was Image (bitmap), not Form (vector).
    CPPUNIT_ASSERT_EQUAL(OString("Form"), pInnerSubtype->GetValue());
}

void PdfExportTest::testTdf105461()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf105461.odp";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(aTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    mpPdfDocument = FPDF_LoadMemDocument(aMemory.GetData(), aMemory.GetSize(), /*password=*/nullptr);
    CPPUNIT_ASSERT(mpPdfDocument);

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, FPDF_GetPageCount(mpPdfDocument));
    mpPdfPage = FPDF_LoadPage(mpPdfDocument, /*page_index=*/0);
    CPPUNIT_ASSERT(mpPdfPage);

    // Make sure there is a filled rectangle inside.
    int nPageObjectCount = FPDFPage_CountObjects(mpPdfPage);
    int nYellowPathCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPdfPageObject = FPDFPage_GetObject(mpPdfPage, i);
        if (FPDFPageObj_GetType(pPdfPageObject) != FPDF_PAGEOBJ_PATH)
            continue;

        unsigned int nRed = 0, nGreen = 0, nBlue = 0, nAlpha = 0;
        FPDFPath_GetFillColor(pPdfPageObject, &nRed, &nGreen, &nBlue, &nAlpha);
        if (RGB_COLORDATA(nRed, nGreen, nBlue) == COL_YELLOW)
            ++nYellowPathCount;
    }

    // This was 0, the page contained no yellow paths.
    CPPUNIT_ASSERT_EQUAL(1, nYellowPathCount);
}

void PdfExportTest::testTdf107868()
{
    // FIXME: Why does this fail on macOS?
    // FIXME: Why does this fail when building 64bit?
#if !defined MACOSX && !defined _WIN64
    // Import the bugdoc and print to PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf107868.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Reference<view::XPrintable> xPrintable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPrintable.is());
    uno::Sequence<beans::PropertyValue> aOptions(comphelper::InitPropertySequence(
    {
        {"FileName", uno::makeAny(aTempFile.GetURL())},
        {"Wait", uno::makeAny(true)}
    }));
    xPrintable->print(aOptions);

    // Parse the export result with pdfium.
    SvFileStream aFile(aTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    mpPdfDocument = FPDF_LoadMemDocument(aMemory.GetData(), aMemory.GetSize(), /*password=*/nullptr);
    if (!mpPdfDocument)
        // Printing to PDF failed in a non-interesting way, e.g. CUPS is not
        // running, there is no printer defined, etc.
        return;

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, FPDF_GetPageCount(mpPdfDocument));
    mpPdfPage = FPDF_LoadPage(mpPdfDocument, /*page_index=*/0);
    CPPUNIT_ASSERT(mpPdfPage);

    // Make sure there is no filled rectangle inside.
    int nPageObjectCount = FPDFPage_CountObjects(mpPdfPage);
    int nWhitePathCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPdfPageObject = FPDFPage_GetObject(mpPdfPage, i);
        if (FPDFPageObj_GetType(pPdfPageObject) != FPDF_PAGEOBJ_PATH)
            continue;

        unsigned int nRed = 0, nGreen = 0, nBlue = 0, nAlpha = 0;
        FPDFPath_GetFillColor(pPdfPageObject, &nRed, &nGreen, &nBlue, &nAlpha);
        if (RGB_COLORDATA(nRed, nGreen, nBlue) == COL_WHITE)
            ++nWhitePathCount;
    }

    // This was 4, the page contained 4 white paths at problematic positions.
    CPPUNIT_ASSERT_EQUAL(0, nWhitePathCount);
#endif
}

void PdfExportTest::testTdf105093()
{
    vcl::filter::PDFDocument aDocument;
    load("tdf105093.odp", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // Get page annotations.
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnnots->GetElements().size());
    auto pAnnotReference = dynamic_cast<vcl::filter::PDFReferenceElement*>(pAnnots->GetElements()[0]);
    CPPUNIT_ASSERT(pAnnotReference);
    vcl::filter::PDFObjectElement* pAnnot = pAnnotReference->LookupObject();
    CPPUNIT_ASSERT(pAnnot);
    CPPUNIT_ASSERT_EQUAL(OString("Annot"), static_cast<vcl::filter::PDFNameElement*>(pAnnot->Lookup("Type"))->GetValue());

    // Get the Action -> Rendition -> MediaClip -> FileSpec.
    auto pAction = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAnnot->Lookup("A"));
    CPPUNIT_ASSERT(pAction);
    auto pRendition = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pAction->LookupElement("R"));
    CPPUNIT_ASSERT(pRendition);
    auto pMediaClip = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pRendition->LookupElement("C"));
    CPPUNIT_ASSERT(pMediaClip);
    auto pFileSpec = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pMediaClip->LookupElement("D"));
    CPPUNIT_ASSERT(pFileSpec);
    // Make sure the filespec refers to an embedded file.
    // This key was missing, the embedded video was handled as a linked one.
    CPPUNIT_ASSERT(pFileSpec->LookupElement("EF"));
}

void PdfExportTest::testTdf106206()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106206.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(aTempFile.GetURL(), StreamMode::READ);
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
    it = std::search(pStart, pEnd, aInvalidImage.getStr(), aInvalidImage.getStr() + aInvalidImage.getLength());
    // This failed, object #0 was referenced.
    CPPUNIT_ASSERT(bool(it == pEnd));
}

void PdfExportTest::testTdf106972()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106972.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(aTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // Get access to the only form object on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Get access to the only image inside the form object.
    auto pFormResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"));
    CPPUNIT_ASSERT(pFormResources);
    auto pImages = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pFormResources->LookupElement("XObject"));
    CPPUNIT_ASSERT(pImages);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pImages->GetItems().size());
    vcl::filter::PDFObjectElement* pImage = pImages->LookupObject(pImages->GetItems().begin()->first);
    CPPUNIT_ASSERT(pImage);

    // Assert resources of the image.
    auto pImageResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pImage->Lookup("Resources"));
    CPPUNIT_ASSERT(pImageResources);
    // This failed: the PDF image had no Font resource.
    CPPUNIT_ASSERT(pImageResources->LookupElement("Font"));
}

void PdfExportTest::testTdf106972Pdf17()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106972-pdf17.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result.
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(aTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // Get access to the only image on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Assert that we now attempt to preserve the original PDF data, even if
    // the original input was PDF >= 1.4.
    CPPUNIT_ASSERT(pXObject->Lookup("Resources"));
}

void PdfExportTest::testTdf107013()
{
    vcl::filter::PDFDocument aDocument;
    load("tdf107013.odt", aDocument);

    // Get access to the only image on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    // This failed, the reference to the image was created, but not the image.
    CPPUNIT_ASSERT(pXObject);
}

void PdfExportTest::testTdf107018()
{
    vcl::filter::PDFDocument aDocument;
    load("tdf107018.odt", aDocument);

    // Get access to the only image on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Get access to the form object inside the image.
    auto pXObjectResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"));
    CPPUNIT_ASSERT(pXObjectResources);
    auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObjectResources->LookupElement("XObject"));
    CPPUNIT_ASSERT(pXObjectForms);
    vcl::filter::PDFObjectElement* pForm = pXObjectForms->LookupObject(pXObjectForms->GetItems().begin()->first);
    CPPUNIT_ASSERT(pForm);

    // Get access to Resources -> Font -> F1 of the form.
    auto pFormResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pForm->Lookup("Resources"));
    CPPUNIT_ASSERT(pFormResources);
    auto pFonts = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pFormResources->LookupElement("Font"));
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

void PdfExportTest::testTdf107089()
{
    vcl::filter::PDFDocument aDocument;
    load("tdf107089.odt", aDocument);

    // Get access to the only image on the only page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());
    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);
    auto pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Get access to the form object inside the image.
    auto pXObjectResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject->Lookup("Resources"));
    CPPUNIT_ASSERT(pXObjectResources);
    auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObjectResources->LookupElement("XObject"));
    CPPUNIT_ASSERT(pXObjectForms);
    vcl::filter::PDFObjectElement* pForm = pXObjectForms->LookupObject(pXObjectForms->GetItems().begin()->first);
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

void PdfExportTest::testTdf99680()
{
    vcl::filter::PDFDocument aDocument;
    load("tdf99680.odt", aDocument);

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

    // Make sure there are no empty clipping regions.
    OString aEmptyRegion("0 0 m h W* n");
    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* pEnd = pStart + aUncompressed.GetSize();
    auto it = std::search(pStart, pEnd, aEmptyRegion.getStr(), aEmptyRegion.getStr() + aEmptyRegion.getLength());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Empty clipping region detected!", it, pEnd);

    // Count save graphic state (q) and restore (Q) operators
    // and ensure their amount is equal
    size_t nSaveCount = std::count(pStart, pEnd, 'q');
    size_t nRestoreCount = std::count(pStart, pEnd, 'Q');
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Save/restore graphic state operators count mismatch!", nSaveCount, nRestoreCount);
}

void PdfExportTest::testTdf99680_2()
{
    vcl::filter::PDFDocument aDocument;
    load("tdf99680-2.odt", aDocument);

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

        // Make sure there are no empty clipping regions.
        OString aEmptyRegion("0 0 m h W* n");
        auto pStart = static_cast<const char*>(aUncompressed.GetData());
        const char* pEnd = pStart + aUncompressed.GetSize();
        auto it = std::search(pStart, pEnd, aEmptyRegion.getStr(), aEmptyRegion.getStr() + aEmptyRegion.getLength());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Empty clipping region detected!", it, pEnd);

        // Count save graphic state (q) and restore (Q) operators
        // and ensure their amount is equal
        size_t nSaveCount = std::count(pStart, pEnd, 'q');
        size_t nRestoreCount = std::count(pStart, pEnd, 'Q');
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Save/restore graphic state operators count mismatch!", nSaveCount, nRestoreCount);
    }
}

void PdfExportTest::testTdf108963()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf108963.odp";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(aTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    mpPdfDocument = FPDF_LoadMemDocument(aMemory.GetData(), aMemory.GetSize(), /*password=*/nullptr);
    CPPUNIT_ASSERT(mpPdfDocument);

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, FPDF_GetPageCount(mpPdfDocument));
    mpPdfPage = FPDF_LoadPage(mpPdfDocument, /*page_index=*/0);
    CPPUNIT_ASSERT(mpPdfPage);

    // Make sure there is a filled rectangle inside.
    int nPageObjectCount = FPDFPage_CountObjects(mpPdfPage);
    int nYellowPathCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPdfPageObject = FPDFPage_GetObject(mpPdfPage, i);
        if (FPDFPageObj_GetType(pPdfPageObject) != FPDF_PAGEOBJ_PATH)
            continue;

        unsigned int nRed = 0, nGreen = 0, nBlue = 0, nAlpha = 0;
        FPDFPath_GetFillColor(pPdfPageObject, &nRed, &nGreen, &nBlue, &nAlpha);
        if (RGB_COLORDATA(nRed, nGreen, nBlue) == COL_YELLOW)
        {
            ++nYellowPathCount;
            // The path described a yellow rectangle, but it was not rotated.
            int nSegments = FPDFPath_CountSegments(pPdfPageObject);
            CPPUNIT_ASSERT_EQUAL(5, nSegments);
            FPDF_PATHSEGMENT pSegment = FPDFPath_GetPathSegment(pPdfPageObject, 0);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_MOVETO, FPDFPathSegment_GetType(pSegment));
            float fX = 0;
            float fY = 0;
            FPDFPathSegment_GetPoint(pSegment, &fX, &fY);
            CPPUNIT_ASSERT_EQUAL(245395, static_cast<int>(round(fX * 1000)));
            CPPUNIT_ASSERT_EQUAL(244233, static_cast<int>(round(fY * 1000)));
            CPPUNIT_ASSERT(!FPDFPathSegment_GetClose(pSegment));

            pSegment = FPDFPath_GetPathSegment(pPdfPageObject, 1);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(pSegment));
            FPDFPathSegment_GetPoint(pSegment, &fX, &fY);
            CPPUNIT_ASSERT_EQUAL(275102, static_cast<int>(round(fX * 1000)));
            CPPUNIT_ASSERT_EQUAL(267591, static_cast<int>(round(fY * 1000)));
            CPPUNIT_ASSERT(!FPDFPathSegment_GetClose(pSegment));

            pSegment = FPDFPath_GetPathSegment(pPdfPageObject, 2);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(pSegment));
            FPDFPathSegment_GetPoint(pSegment, &fX, &fY);
            CPPUNIT_ASSERT_EQUAL(287518, static_cast<int>(round(fX * 1000)));
            CPPUNIT_ASSERT_EQUAL(251802, static_cast<int>(round(fY * 1000)));
            CPPUNIT_ASSERT(!FPDFPathSegment_GetClose(pSegment));

            pSegment = FPDFPath_GetPathSegment(pPdfPageObject, 3);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(pSegment));
            FPDFPathSegment_GetPoint(pSegment, &fX, &fY);
            CPPUNIT_ASSERT_EQUAL(257839, static_cast<int>(round(fX * 1000)));
            CPPUNIT_ASSERT_EQUAL(228444, static_cast<int>(round(fY * 1000)));
            CPPUNIT_ASSERT(!FPDFPathSegment_GetClose(pSegment));

            pSegment = FPDFPath_GetPathSegment(pPdfPageObject, 4);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(pSegment));
            FPDFPathSegment_GetPoint(pSegment, &fX, &fY);
            CPPUNIT_ASSERT_EQUAL(245395, static_cast<int>(round(fX * 1000)));
            CPPUNIT_ASSERT_EQUAL(244233, static_cast<int>(round(fY * 1000)));
            CPPUNIT_ASSERT(FPDFPathSegment_GetClose(pSegment));
        }
    }

    CPPUNIT_ASSERT_EQUAL(1, nYellowPathCount);
}
#endif

CPPUNIT_TEST_SUITE_REGISTRATION(PdfExportTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
