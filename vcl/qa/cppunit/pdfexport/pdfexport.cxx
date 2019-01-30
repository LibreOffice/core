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
#include <type_traits>

#include <config_features.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/view/XPrintable.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <tools/zcodec.hxx>
#include <fpdf_edit.h>
#include <fpdf_text.h>
#include <fpdfview.h>

using namespace ::com::sun::star;

namespace
{

struct CloseDocument {
    void operator ()(FPDF_DOCUMENT doc) {
        if (doc != nullptr) {
            FPDF_CloseDocument(doc);
        }
    }
};

using DocumentHolder =
    std::unique_ptr<typename std::remove_pointer<FPDF_DOCUMENT>::type, CloseDocument>;

struct ClosePage {
    void operator ()(FPDF_PAGE page) {
        if (page != nullptr) {
            FPDF_ClosePage(page);
        }
    }
};

using PageHolder =
    std::unique_ptr<typename std::remove_pointer<FPDF_PAGE>::type, ClosePage>;

/// Tests the PDF export filter.
class PdfExportTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;
    utl::TempFile maTempFile;
    SvMemoryStream maMemory;
    // Export the document as PDF, then parse it with PDFium.
    DocumentHolder exportAndParse(const OUString& rURL, const utl::MediaDescriptor& rDescriptor);

public:
    PdfExportTest();
    virtual void setUp() override;
    virtual void tearDown() override;
    void topdf(const OUString& rFile);
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
    void testForcePoint71();
    void testTdf106972();
    void testTdf106972Pdf17();
    void testSofthyphenPos();
    void testTdf107013();
    void testTdf107018();
    void testTdf107089();
    void testTdf99680();
    void testTdf99680_2();
    void testTdf108963();
    void testTdf118244_radioButtonGroup();
#if HAVE_MORE_FONTS
    /// Test writing ToUnicode CMAP for LTR ligatures.
    void testTdf115117_1();
    /// Text extracting LTR text with ligatures.
    void testTdf115117_1a();
    /// Test writing ToUnicode CMAP for RTL ligatures.
    void testTdf115117_2();
    /// Test extracting RTL text with ligatures.
    void testTdf115117_2a();
    /// Test writing ToUnicode CMAP for doubly encoded glyphs.
    void testTdf66597_1();
    /// Test writing ActualText for RTL many to one glyph to Unicode mapping.
    void testTdf66597_2();
    /// Test writing ActualText for LTR many to one glyph to Unicode mapping.
    void testTdf66597_3();
#endif
    void testTdf109143();
    void testTdf105954();
    void testTdf106702();
    void testTdf113143();
    void testTdf115262();
    void testTdf121962();

    CPPUNIT_TEST_SUITE(PdfExportTest);
    CPPUNIT_TEST(testTdf106059);
    CPPUNIT_TEST(testTdf105461);
    CPPUNIT_TEST(testTdf107868);
    CPPUNIT_TEST(testTdf105093);
    CPPUNIT_TEST(testTdf106206);
    CPPUNIT_TEST(testTdf106693);
    CPPUNIT_TEST(testForcePoint71);
    CPPUNIT_TEST(testTdf106972);
    CPPUNIT_TEST(testTdf106972Pdf17);
    CPPUNIT_TEST(testSofthyphenPos);
    CPPUNIT_TEST(testTdf107013);
    CPPUNIT_TEST(testTdf107018);
    CPPUNIT_TEST(testTdf107089);
    CPPUNIT_TEST(testTdf99680);
    CPPUNIT_TEST(testTdf99680_2);
    CPPUNIT_TEST(testTdf108963);
    CPPUNIT_TEST(testTdf118244_radioButtonGroup);
#if HAVE_MORE_FONTS
    CPPUNIT_TEST(testTdf115117_1);
    CPPUNIT_TEST(testTdf115117_1a);
    CPPUNIT_TEST(testTdf115117_2);
    CPPUNIT_TEST(testTdf115117_2a);
    CPPUNIT_TEST(testTdf66597_1);
    CPPUNIT_TEST(testTdf66597_2);
    CPPUNIT_TEST(testTdf66597_3);
#endif
    CPPUNIT_TEST(testTdf109143);
    CPPUNIT_TEST(testTdf105954);
    CPPUNIT_TEST(testTdf106702);
    CPPUNIT_TEST(testTdf113143);
    CPPUNIT_TEST(testTdf115262);
    CPPUNIT_TEST(testTdf121962);
    CPPUNIT_TEST_SUITE_END();
};

PdfExportTest::PdfExportTest()
{
    maTempFile.EnableKillingFile();
}

DocumentHolder PdfExportTest::exportAndParse(const OUString& rURL, const utl::MediaDescriptor& rDescriptor)
{
    // Import the bugdoc and export as PDF.
    mxComponent = loadFromDesktop(rURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), rDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    maMemory.WriteStream(aFile);
    DocumentHolder pPdfDocument(
        FPDF_LoadMemDocument(maMemory.GetData(), maMemory.GetSize(), /*password=*/nullptr));
    CPPUNIT_ASSERT(pPdfDocument.get());
    return pPdfDocument;
}

void PdfExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));

    FPDF_LIBRARY_CONFIG config;
    config.version = 2;
    config.m_pUserFontPaths = nullptr;
    config.m_pIsolate = nullptr;
    config.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&config);
}

void PdfExportTest::tearDown()
{
    FPDF_DestroyLibrary();

    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

char const DATA_DIRECTORY[] = "/vcl/qa/cppunit/pdfexport/data/";

void PdfExportTest::topdf(const OUString& rFile)
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + rFile;
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
}

void PdfExportTest::load(const OUString& rFile, vcl::filter::PDFDocument& rDocument)
{
    topdf(rFile);

    // Parse the export result.
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(rDocument.Read(aStream));
}

void PdfExportTest::testTdf106059()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106059.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    // Explicitly enable the usage of the reference XObject markup.
    uno::Sequence<beans::PropertyValue> aFilterData( comphelper::InitPropertySequence({
        {"UseReferenceXObject", uno::Any(true) }
    }));
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
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    DocumentHolder pPdfDocument(FPDF_LoadMemDocument(aMemory.GetData(), aMemory.GetSize(), /*password=*/nullptr));
    CPPUNIT_ASSERT(pPdfDocument.get());

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, FPDF_GetPageCount(pPdfDocument.get()));
    PageHolder pPdfPage(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/0));
    CPPUNIT_ASSERT(pPdfPage.get());

    // Make sure there is a filled rectangle inside.
    int nPageObjectCount = FPDFPage_CountObjects(pPdfPage.get());
    int nYellowPathCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPdfPageObject = FPDFPage_GetObject(pPdfPage.get(), i);
        if (FPDFPageObj_GetType(pPdfPageObject) != FPDF_PAGEOBJ_PATH)
            continue;

        unsigned int nRed = 0, nGreen = 0, nBlue = 0, nAlpha = 0;
        FPDFPath_GetFillColor(pPdfPageObject, &nRed, &nGreen, &nBlue, &nAlpha);
        if (Color(nRed, nGreen, nBlue) == COL_YELLOW)
            ++nYellowPathCount;
    }

    // This was 0, the page contained no yellow paths.
    CPPUNIT_ASSERT_EQUAL(1, nYellowPathCount);
}

void PdfExportTest::testTdf107868()
{
    // No need to run it on Windows, since it would use GDI printing, and not trigger PDF export
    // which is the intent of the test.
    // FIXME: Why does this fail on macOS?
#if !defined MACOSX && !defined _WIN32
    // Import the bugdoc and print to PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf107868.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XPrintable> xPrintable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPrintable.is());
    uno::Sequence<beans::PropertyValue> aOptions(comphelper::InitPropertySequence(
    {
        {"FileName", uno::makeAny(maTempFile.GetURL())},
        {"Wait", uno::makeAny(true)}
    }));
    xPrintable->print(aOptions);

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    DocumentHolder pPdfDocument(FPDF_LoadMemDocument(aMemory.GetData(), aMemory.GetSize(), /*password=*/nullptr));
    if (!pPdfDocument.get())
        // Printing to PDF failed in a non-interesting way, e.g. CUPS is not
        // running, there is no printer defined, etc.
        return;

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, FPDF_GetPageCount(pPdfDocument.get()));
    PageHolder pPdfPage(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/0));
    CPPUNIT_ASSERT(pPdfPage.get());

    // Make sure there is no filled rectangle inside.
    int nPageObjectCount = FPDFPage_CountObjects(pPdfPage.get());
    int nWhitePathCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPdfPageObject = FPDFPage_GetObject(pPdfPage.get(), i);
        if (FPDFPageObj_GetType(pPdfPageObject) != FPDF_PAGEOBJ_PATH)
            continue;

        unsigned int nRed = 0, nGreen = 0, nBlue = 0, nAlpha = 0;
        FPDFPath_GetFillColor(pPdfPageObject, &nRed, &nGreen, &nBlue, &nAlpha);
        if (Color(nRed, nGreen, nBlue) == COL_WHITE)
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
    it = std::search(pStart, pEnd, aInvalidImage.getStr(), aInvalidImage.getStr() + aInvalidImage.getLength());
    // This failed, object #0 was referenced.
    CPPUNIT_ASSERT(bool(it == pEnd));
}

void PdfExportTest::testTdf109143()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf109143.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

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
    auto pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Make sure it's re-compressed.
    auto pLength = dynamic_cast<vcl::filter::PDFNumberElement*>(pXObject->Lookup("Length"));
    int nLength = pLength->GetValue();
    // This failed: cropped TIFF-in-JPEG wasn't re-compressed, so crop was
    // lost. Size was 59416, now is 11827.
    CPPUNIT_ASSERT(nLength < 50000);
}

void PdfExportTest::testTdf106972()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106972.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

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
    auto pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pXObjects->GetItems().size());
    vcl::filter::PDFObjectElement* pXObject = pXObjects->LookupObject(pXObjects->GetItems().begin()->first);
    CPPUNIT_ASSERT(pXObject);

    // Assert that we now attempt to preserve the original PDF data, even if
    // the original input was PDF >= 1.4.
    CPPUNIT_ASSERT(pXObject->Lookup("Resources"));
}

void PdfExportTest::testSofthyphenPos()
{
    // No need to run it on Windows, since it would use GDI printing, and not
    // trigger PDF export which is the intent of the test.
    // FIXME: Why does this fail on macOS?
#if !defined MACOSX && !defined _WIN32
    // Import the bugdoc and print to PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "softhyphen_pdf.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XPrintable> xPrintable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPrintable.is());
    uno::Sequence<beans::PropertyValue> aOptions(comphelper::InitPropertySequence(
    {
        {"FileName", uno::makeAny(maTempFile.GetURL())},
        {"Wait", uno::makeAny(true)}
    }));
    xPrintable->print(aOptions);

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    DocumentHolder pPdfDocument(FPDF_LoadMemDocument(aMemory.GetData(), aMemory.GetSize(), /*password=*/nullptr));
    if (!pPdfDocument.get())
    {
        // Printing to PDF failed in a non-interesting way, e.g. CUPS is not
        // running, there is no printer defined, etc.
        return;
    }

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, FPDF_GetPageCount(pPdfDocument.get()));
    PageHolder pPdfPage(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/0));
    CPPUNIT_ASSERT(pPdfPage.get());

    // tdf#96892 incorrect fractional part of font size caused soft-hyphen to
    // be positioned inside preceding text (incorrect = 11.1, correct = 11.05)

    // there are 3 texts currently, for line 1, soft-hyphen, line 2
    bool haveText(false);

    int nPageObjectCount = FPDFPage_CountObjects(pPdfPage.get());
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPdfPageObject = FPDFPage_GetObject(pPdfPage.get(), i);
        CPPUNIT_ASSERT_EQUAL(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(pPdfPageObject));
        haveText = true;
        double const size(FPDFTextObj_GetFontSize(pPdfPageObject));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(11.05, size, 1E-06);
    }

    CPPUNIT_ASSERT(haveText);
#endif
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
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    DocumentHolder pPdfDocument(FPDF_LoadMemDocument(aMemory.GetData(), aMemory.GetSize(), /*password=*/nullptr));
    CPPUNIT_ASSERT(pPdfDocument.get());

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, FPDF_GetPageCount(pPdfDocument.get()));
    PageHolder pPdfPage(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/0));
    CPPUNIT_ASSERT(pPdfPage.get());

    // FIXME: strangely this fails on some Win systems after a pdfium update, expected: 793.7; actual: 793
#if !defined _WIN32
    // Test page size (28x15.75 cm, was 1/100th mm off, tdf#112690)
    // bad: MediaBox[0 0 793.672440944882 446.428346456693]
    // good: MediaBox[0 0 793.700787401575 446.456692913386]
    const double aWidth = FPDF_GetPageWidth(pPdfPage.get());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(793.7, aWidth, 0.01);
    const double aHeight = FPDF_GetPageHeight(pPdfPage.get());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(446.46, aHeight, 0.01);

    // Make sure there is a filled rectangle inside.
    int nPageObjectCount = FPDFPage_CountObjects(pPdfPage.get());
    int nYellowPathCount = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPdfPageObject = FPDFPage_GetObject(pPdfPage.get(), i);
        if (FPDFPageObj_GetType(pPdfPageObject) != FPDF_PAGEOBJ_PATH)
            continue;

        unsigned int nRed = 0, nGreen = 0, nBlue = 0, nAlpha = 0;
        FPDFPath_GetFillColor(pPdfPageObject, &nRed, &nGreen, &nBlue, &nAlpha);
        if (Color(nRed, nGreen, nBlue) == COL_YELLOW)
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
            CPPUNIT_ASSERT_EQUAL(244261, static_cast<int>(round(fY * 1000)));
            CPPUNIT_ASSERT(!FPDFPathSegment_GetClose(pSegment));

            pSegment = FPDFPath_GetPathSegment(pPdfPageObject, 1);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(pSegment));
            FPDFPathSegment_GetPoint(pSegment, &fX, &fY);
            CPPUNIT_ASSERT_EQUAL(275102, static_cast<int>(round(fX * 1000)));
            CPPUNIT_ASSERT_EQUAL(267618, static_cast<int>(round(fY * 1000)));
            CPPUNIT_ASSERT(!FPDFPathSegment_GetClose(pSegment));

            pSegment = FPDFPath_GetPathSegment(pPdfPageObject, 2);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(pSegment));
            FPDFPathSegment_GetPoint(pSegment, &fX, &fY);
            CPPUNIT_ASSERT_EQUAL(287518, static_cast<int>(round(fX * 1000)));
            CPPUNIT_ASSERT_EQUAL(251829, static_cast<int>(round(fY * 1000)));
            CPPUNIT_ASSERT(!FPDFPathSegment_GetClose(pSegment));

            pSegment = FPDFPath_GetPathSegment(pPdfPageObject, 3);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(pSegment));
            FPDFPathSegment_GetPoint(pSegment, &fX, &fY);
            CPPUNIT_ASSERT_EQUAL(257839, static_cast<int>(round(fX * 1000)));
            CPPUNIT_ASSERT_EQUAL(228472, static_cast<int>(round(fY * 1000)));
            CPPUNIT_ASSERT(!FPDFPathSegment_GetClose(pSegment));

            pSegment = FPDFPath_GetPathSegment(pPdfPageObject, 4);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(pSegment));
            FPDFPathSegment_GetPoint(pSegment, &fX, &fY);
            CPPUNIT_ASSERT_EQUAL(245395, static_cast<int>(round(fX * 1000)));
            CPPUNIT_ASSERT_EQUAL(244261, static_cast<int>(round(fY * 1000)));
            CPPUNIT_ASSERT(FPDFPathSegment_GetClose(pSegment));
        }
    }

    CPPUNIT_ASSERT_EQUAL(1, nYellowPathCount);
#endif
}

void PdfExportTest::testTdf118244_radioButtonGroup()
{
    vcl::filter::PDFDocument aDocument;
    load("tdf118244_radioButtonGroup.odt", aDocument);

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    // There are eight radio buttons.
    auto pAnnots = dynamic_cast<vcl::filter::PDFArrayElement*>(aPages[0]->Lookup("Annots"));
    CPPUNIT_ASSERT(pAnnots);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of radio buttons",static_cast<size_t>(8), pAnnots->GetElements().size());

    sal_uInt32 nRadioGroups = 0;
    for ( const auto& aElement : aDocument.GetElements() )
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if ( !pObject )
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("FT"));
        if ( pType && pType->GetValue() == "Btn" )
        {
            auto pKids = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject->Lookup("Kids"));
            if ( pKids )
            {
                size_t expectedSize = 2;
                ++nRadioGroups;
                if ( nRadioGroups == 3 )
                    expectedSize = 3;
                CPPUNIT_ASSERT_EQUAL(expectedSize, pKids->GetElements().size());
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of radio groups", sal_uInt32(3), nRadioGroups);
}

#if HAVE_MORE_FONTS
// This requires Carlito font, if it is missing the test will most likely
// fail.
void PdfExportTest::testTdf115117_1()
{
    vcl::filter::PDFDocument aDocument;
    load("tdf115117-1.odt", aDocument);

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
            auto pToUnicodeRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("ToUnicode"));
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
}

// This requires DejaVu Sans font, if it is missing the test will most likely
// fail.
void PdfExportTest::testTdf115117_2()
{
    // See the comments in testTdf115117_1() for explanation.

    vcl::filter::PDFDocument aDocument;
    load("tdf115117-2.odt", aDocument);

    vcl::filter::PDFObjectElement* pToUnicode = nullptr;

    for (const auto& aElement : aDocument.GetElements())
    {
        auto pObject = dynamic_cast<vcl::filter::PDFObjectElement*>(aElement.get());
        if (!pObject)
            continue;
        auto pType = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("Type"));
        if (pType && pType->GetValue() == "Font")
        {
            auto pToUnicodeRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("ToUnicode"));
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
}

void PdfExportTest::testTdf115117_1a()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf115117-1.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    DocumentHolder pPdfDocument(FPDF_LoadMemDocument(aMemory.GetData(), aMemory.GetSize(), /*password=*/nullptr));
    CPPUNIT_ASSERT(pPdfDocument.get());

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, FPDF_GetPageCount(pPdfDocument.get()));
    PageHolder pPdfPage(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/0));
    CPPUNIT_ASSERT(pPdfPage.get());

    auto pPdfTextPage = FPDFText_LoadPage(pPdfPage.get());
    CPPUNIT_ASSERT(pPdfTextPage);

    // Extract the text from the page. This pdfium API is a bit higher level
    // than we want and might apply heuristic that give false positive, but it
    // is a good approximation in addition to the check in testTdf115117_1().
    int nChars = FPDFText_CountChars(pPdfTextPage);
    CPPUNIT_ASSERT_EQUAL(44, nChars);

    OUString aExpectedText = "ti ti test ti\r\nti test fi fl ffi ffl test fi";
    std::vector<sal_uInt32> aChars(nChars);
    for (int i = 0; i < nChars; i++)
        aChars[i] = FPDFText_GetUnicode(pPdfTextPage, i);
    OUString aActualText(aChars.data(), aChars.size());
    CPPUNIT_ASSERT_EQUAL(aExpectedText, aActualText);
}

void PdfExportTest::testTdf115117_2a()
{
    // See the comments in testTdf115117_1a() for explanation.

    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf115117-2.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    DocumentHolder pPdfDocument(FPDF_LoadMemDocument(aMemory.GetData(), aMemory.GetSize(), /*password=*/nullptr));
    CPPUNIT_ASSERT(pPdfDocument.get());

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, FPDF_GetPageCount(pPdfDocument.get()));
    PageHolder pPdfPage(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/0));
    CPPUNIT_ASSERT(pPdfPage.get());

    auto pPdfTextPage = FPDFText_LoadPage(pPdfPage.get());
    CPPUNIT_ASSERT(pPdfTextPage);

    int nChars = FPDFText_CountChars(pPdfTextPage);
    CPPUNIT_ASSERT_EQUAL(13, nChars);

    OUString aExpectedText = u"\u0627\u0644 \u0628\u0627\u0644 \u0648\u0642\u0641 \u0627\u0644";
    std::vector<sal_uInt32> aChars(nChars);
    for (int i = 0; i < nChars; i++)
        aChars[i] = FPDFText_GetUnicode(pPdfTextPage, i);
    OUString aActualText(aChars.data(), aChars.size());
    CPPUNIT_ASSERT_EQUAL(aExpectedText, aActualText);
}

// This requires Amiri font, if it is missing the test will fail.
void PdfExportTest::testTdf66597_1()
{
    // FIXME: Fallback font is used on Windows for some reason.
#if !defined _WIN32
    vcl::filter::PDFDocument aDocument;
    load("tdf66597-1.odt", aDocument);

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
                auto pName = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("BaseFont"));
                auto aName = pName->GetValue().copy(7); // skip the subset id
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected font name", OString("Amiri-Regular"), aName);

                auto pToUnicodeRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("ToUnicode"));
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
        std::string aData(static_cast<const char*>(aObjectStream.GetData()), aObjectStream.GetSize());
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
        std::string aData(static_cast<const char*>(aUncompressed.GetData()), aUncompressed.GetSize());

        std::string aActualText("/Span<</ActualText<");
        size_t nCount = 0;
        size_t nPos = 0;
        while ((nPos = aData.find(aActualText, nPos)) != std::string::npos)
        {
            nCount++;
            nPos += aActualText.length();
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("The should be one ActualText entry!", static_cast<size_t>(1), nCount);

        aActualText = "/Span<</ActualText<FEFF00A0>>>";
        nPos = aData.find(aActualText);
        CPPUNIT_ASSERT_MESSAGE("ActualText not found!", nPos != std::string::npos);
    }
#endif
}

// This requires Reem Kufi font, if it is missing the test will fail.
void PdfExportTest::testTdf66597_2()
{
    // FIXME: Fallback font is used on Windows for some reason.
#if !defined _WIN32
    vcl::filter::PDFDocument aDocument;
    load("tdf66597-2.odt", aDocument);

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
                auto pName = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("BaseFont"));
                auto aName = pName->GetValue().copy(7); // skip the subset id
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected font name", OString("ReemKufi-Regular"), aName);

                auto pToUnicodeRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("ToUnicode"));
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
        std::string aData(static_cast<const char*>(aObjectStream.GetData()), aObjectStream.GetSize());
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
        std::string aData(static_cast<const char*>(aUncompressed.GetData()), aUncompressed.GetSize());

        std::vector<std::string> aCodes({ "0632", "062C", "0628", "0623" });
        std::string aActualText("/Span<</ActualText<");
        size_t nCount = 0;
        size_t nPos = 0;
        while ((nPos = aData.find(aActualText, nPos)) != std::string::npos)
        {
            nCount++;
            nPos += aActualText.length();
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of ActualText entries does not match!", aCodes.size(), nCount);

        for (const auto& aCode : aCodes)
        {
            aActualText = "/Span<</ActualText<FEFF" + aCode + ">>>";
            nPos = aData.find(aActualText);
            CPPUNIT_ASSERT_MESSAGE("ActualText not found for " + aCode, nPos != std::string::npos);
        }
    }
#endif
}

// This requires Gentium Basic font, if it is missing the test will fail.
void PdfExportTest::testTdf66597_3()
{
    vcl::filter::PDFDocument aDocument;
    load("tdf66597-3.odt", aDocument);

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
                auto pName = dynamic_cast<vcl::filter::PDFNameElement*>(pObject->Lookup("BaseFont"));
                auto aName = pName->GetValue().copy(7); // skip the subset id
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected font name", OString("GentiumBasic"), aName);

                auto pToUnicodeRef = dynamic_cast<vcl::filter::PDFReferenceElement*>(pObject->Lookup("ToUnicode"));
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
        std::string aData(static_cast<const char*>(aObjectStream.GetData()), aObjectStream.GetSize());
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
        std::string aData(static_cast<const char*>(aUncompressed.GetData()), aUncompressed.GetSize());

        std::string aActualText("/Span<</ActualText<FEFF1ECB0331030B>>>");
        size_t nCount = 0;
        size_t nPos = 0;
        while ((nPos = aData.find(aActualText, nPos)) != std::string::npos)
        {
            nCount++;
            nPos += aActualText.length();
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of ActualText entries does not match!", static_cast<size_t>(4), nCount);
    }
}
#endif

void PdfExportTest::testTdf105954()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf105954.odt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

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
    DocumentHolder pPdfDocument(
        FPDF_LoadMemDocument(aMemory.GetData(), aMemory.GetSize(), /*password=*/nullptr));
    CPPUNIT_ASSERT(pPdfDocument.get());

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, FPDF_GetPageCount(pPdfDocument.get()));
    PageHolder pPdfPage(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/0));
    CPPUNIT_ASSERT(pPdfPage.get());

    // There is a single image on the page.
    int nPageObjectCount = FPDFPage_CountObjects(pPdfPage.get());
    CPPUNIT_ASSERT_EQUAL(1, nPageObjectCount);

    // Check width of the image.
    FPDF_PAGEOBJECT pPageObject = FPDFPage_GetObject(pPdfPage.get(), /*index=*/0);
    FPDF_IMAGEOBJ_METADATA aMeta;
    CPPUNIT_ASSERT(FPDFImageObj_GetImageMetadata(pPageObject, pPdfPage.get(), &aMeta));
    // This was 2000, i.e. the 'reduce to 300 DPI' request was ignored.
    // This is now around 238 (228 on macOS).
    CPPUNIT_ASSERT_LESS(static_cast<unsigned int>(250), aMeta.width);
}

void PdfExportTest::testTdf106702()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf106702.odt";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    auto pPdfDocument = exportAndParse(aURL, aMediaDescriptor);

    // The document has two pages.
    CPPUNIT_ASSERT_EQUAL(2, FPDF_GetPageCount(pPdfDocument.get()));

    // First page already has the correct image position.
    PageHolder pPdfPage(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/0));
    CPPUNIT_ASSERT(pPdfPage.get());
    int nExpected = 0;
    int nPageObjectCount = FPDFPage_CountObjects(pPdfPage.get());
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPageObject = FPDFPage_GetObject(pPdfPage.get(), i);
        if (FPDFPageObj_GetType(pPageObject) != FPDF_PAGEOBJ_IMAGE)
            continue;

        float fLeft = 0, fBottom = 0, fRight = 0, fTop = 0;
        FPDFPageObj_GetBounds(pPageObject, &fLeft, &fBottom, &fRight, &fTop);
        nExpected = fTop;
        break;
    }

    // Second page had an incorrect image position.
    pPdfPage.reset(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/1));
    CPPUNIT_ASSERT(pPdfPage.get());
    int nActual = 0;
    nPageObjectCount = FPDFPage_CountObjects(pPdfPage.get());
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPageObject = FPDFPage_GetObject(pPdfPage.get(), i);
        if (FPDFPageObj_GetType(pPageObject) != FPDF_PAGEOBJ_IMAGE)
            continue;

        float fLeft = 0, fBottom = 0, fRight = 0, fTop = 0;
        FPDFPageObj_GetBounds(pPageObject, &fLeft, &fBottom, &fRight, &fTop);
        nActual = fTop;
        break;
    }

    // This failed, vertical pos is 818 points, was 1674 (outside visible page
    // bounds).
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

void PdfExportTest::testTdf113143()
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
    CPPUNIT_ASSERT_EQUAL(2, FPDF_GetPageCount(pPdfDocument.get()));

    // First has the original (larger) image.
    PageHolder pPdfPage(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/0));
    CPPUNIT_ASSERT(pPdfPage.get());
    int nLarger = 0;
    int nPageObjectCount = FPDFPage_CountObjects(pPdfPage.get());
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPageObject = FPDFPage_GetObject(pPdfPage.get(), i);
        if (FPDFPageObj_GetType(pPageObject) != FPDF_PAGEOBJ_IMAGE)
            continue;

        float fLeft = 0, fBottom = 0, fRight = 0, fTop = 0;
        FPDFPageObj_GetBounds(pPageObject, &fLeft, &fBottom, &fRight, &fTop);
        nLarger = fRight - fLeft;
        break;
    }

    // Second page has the scaled (smaller) image.
    pPdfPage.reset(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/1));
    CPPUNIT_ASSERT(pPdfPage.get());
    int nSmaller = 0;
    nPageObjectCount = FPDFPage_CountObjects(pPdfPage.get());
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPageObject = FPDFPage_GetObject(pPdfPage.get(), i);
        if (FPDFPageObj_GetType(pPageObject) != FPDF_PAGEOBJ_IMAGE)
            continue;

        float fLeft = 0, fBottom = 0, fRight = 0, fTop = 0;
        FPDFPageObj_GetBounds(pPageObject, &fLeft, &fBottom, &fRight, &fTop);
        nSmaller = fRight - fLeft;
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

void PdfExportTest::testForcePoint71()
{
    // I just care it doesn't crash
    topdf("forcepoint71.key");
}

void PdfExportTest::testTdf115262()
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf115262.ods";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("calc_pdf_Export");
    auto pPdfDocument = exportAndParse(aURL, aMediaDescriptor);
    CPPUNIT_ASSERT_EQUAL(8, FPDF_GetPageCount(pPdfDocument.get()));

    // Get the 6th page.
    PageHolder pPdfPage(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/5));
    CPPUNIT_ASSERT(pPdfPage.get());

    // Look up the position of the first image and the 400th row.
    FPDF_TEXTPAGE pTextPage = FPDFText_LoadPage(pPdfPage.get());
    int nPageObjectCount = FPDFPage_CountObjects(pPdfPage.get());
    int nFirstImageTop = 0;
    int nRowTop = 0;
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPageObject = FPDFPage_GetObject(pPdfPage.get(), i);
        float fLeft = 0, fBottom = 0, fRight = 0, fTop = 0;
        FPDFPageObj_GetBounds(pPageObject, &fLeft, &fBottom, &fRight, &fTop);

        if (FPDFPageObj_GetType(pPageObject) == FPDF_PAGEOBJ_IMAGE)
        {
            nFirstImageTop = fTop;
        }
        else if (FPDFPageObj_GetType(pPageObject) == FPDF_PAGEOBJ_TEXT)
        {
            unsigned long nTextSize = FPDFTextObj_GetText(pPageObject, pTextPage, nullptr, 0);
            std::vector<sal_Unicode> aText(nTextSize);
            FPDFTextObj_GetText(pPageObject, pTextPage, aText.data(), nTextSize);
            OUString sText(aText.data(), nTextSize / 2 - 1);
            if (sText == "400")
                nRowTop = fTop;
        }
    }
    // Make sure that the top of the "400" is below the top of the image (in
    // bottom-right-corner-based PDF coordinates).
    // This was: expected less than 144, actual is 199.
    CPPUNIT_ASSERT_LESS(nFirstImageTop, nRowTop);
    FPDFText_ClosePage(pTextPage);
}

void PdfExportTest::testTdf121962()
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf121962.odt";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    auto pPdfDocument = exportAndParse(aURL, aMediaDescriptor);
    CPPUNIT_ASSERT_EQUAL(1, FPDF_GetPageCount(pPdfDocument.get()));

    // Get the first page
    PageHolder pPdfPage(FPDF_LoadPage(pPdfDocument.get(), /*page_index=*/0));
    CPPUNIT_ASSERT(pPdfPage.get());
    FPDF_TEXTPAGE pTextPage = FPDFText_LoadPage(pPdfPage.get());

    // Make sure the table sum is displayed as "0", not faulty expression.
    int nPageObjectCount = FPDFPage_CountObjects(pPdfPage.get());
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        FPDF_PAGEOBJECT pPageObject = FPDFPage_GetObject(pPdfPage.get(), i);
        if (FPDFPageObj_GetType(pPageObject) != FPDF_PAGEOBJ_TEXT)
            continue;
        unsigned long nTextSize = FPDFTextObj_GetText(pPageObject, pTextPage, nullptr, 0);
        std::vector<sal_Unicode> aText(nTextSize);
        FPDFTextObj_GetText(pPageObject, pTextPage, aText.data(), nTextSize);
        OUString sText(aText.data(), nTextSize / 2 - 1);
        CPPUNIT_ASSERT(sText != "** Expression is faulty **");
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(PdfExportTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
