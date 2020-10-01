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
#include <osl/endian.h>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <tools/zcodec.hxx>
#include <fpdf_edit.h>
#include <fpdf_text.h>
#include <fpdf_doc.h>
#include <fpdf_annot.h>
#include <fpdfview.h>
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
          << " A:" << static_cast<int>(rColor.GetTransparency());
    return rStrm;
}

namespace
{

/// Tests the PDF export filter.
class PdfExportTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    uno::Reference<lang::XComponent> mxComponent;
    utl::TempFile maTempFile;
    SvMemoryStream maMemory;
    // Export the document as PDF, then parse it with PDFium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> exportAndParse(const OUString& rURL, const utl::MediaDescriptor& rDescriptor);
    std::shared_ptr<vcl::pdf::PDFium> mpPDFium;

public:
    PdfExportTest();
    virtual void setUp() override;
    virtual void tearDown() override;
    void saveAsPDF(const OUString& rFile);
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
    void testTdf109143();
    void testTdf105954();
    void testTdf128630();
    void testTdf106702();
    void testTdf113143();
    void testTdf115262();
    void testTdf121962();
    void testTdf115967();
    void testTdf121615();
    void testTocLink();
    void testPdfImageResourceInlineXObjectRef();
    void testReduceSmallImage();
    void testReduceImage();
    void testLinkWrongPage();
    void testLargePage();
    void testVersion15();
    void testDefaultVersion();
    void testMultiPagePDF();
    void testFormFontName();


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
    CPPUNIT_TEST(testTdf115117_1);
    CPPUNIT_TEST(testTdf115117_1a);
    CPPUNIT_TEST(testTdf115117_2);
    CPPUNIT_TEST(testTdf115117_2a);
    CPPUNIT_TEST(testTdf66597_1);
    CPPUNIT_TEST(testTdf66597_2);
    CPPUNIT_TEST(testTdf66597_3);
    CPPUNIT_TEST(testTdf109143);
    CPPUNIT_TEST(testTdf105954);
    CPPUNIT_TEST(testTdf128630);
    CPPUNIT_TEST(testTdf106702);
    CPPUNIT_TEST(testTdf113143);
    CPPUNIT_TEST(testTdf115262);
    CPPUNIT_TEST(testTdf121962);
    CPPUNIT_TEST(testTdf115967);
    CPPUNIT_TEST(testTdf121615);
    CPPUNIT_TEST(testTocLink);
    CPPUNIT_TEST(testPdfImageResourceInlineXObjectRef);
    CPPUNIT_TEST(testReduceSmallImage);
    CPPUNIT_TEST(testReduceImage);
    CPPUNIT_TEST(testLinkWrongPage);
    CPPUNIT_TEST(testLargePage);
    CPPUNIT_TEST(testVersion15);
    CPPUNIT_TEST(testDefaultVersion);
    CPPUNIT_TEST(testMultiPagePDF);
    CPPUNIT_TEST(testFormFontName);
    CPPUNIT_TEST_SUITE_END();
};

PdfExportTest::PdfExportTest()
{
    maTempFile.EnableKillingFile();
}

std::unique_ptr<vcl::pdf::PDFiumDocument> PdfExportTest::exportAndParse(const OUString& rURL, const utl::MediaDescriptor& rDescriptor)
{
    // Import the bugdoc and export as PDF.
    mxComponent = loadFromDesktop(rURL);
    CPPUNIT_ASSERT(mxComponent.is());

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

char const DATA_DIRECTORY[] = "/vcl/qa/cppunit/pdfexport/data/";

void PdfExportTest::saveAsPDF(const OUString& rFile)
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
    saveAsPDF(rFile);

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
        if (pPdfPageObject->getType() != FPDF_PAGEOBJ_PATH)
            continue;

        if (pPdfPageObject->getFillColor() == COL_YELLOW)
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
        if (pPdfPageObject->getType() != FPDF_PAGEOBJ_PATH)
            continue;

        if (pPdfPageObject->getFillColor() == COL_WHITE)
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
    CPPUNIT_ASSERT(pLength);
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
    // No need to run it on Windows, since it would use GDI printing, and not trigger PDF export
    // which is the intent of the test.
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
        CPPUNIT_ASSERT_EQUAL(FPDF_PAGEOBJ_TEXT, pPdfPageObject->getType());
        haveText = true;
        double const size = pPdfPageObject->getFontSize();
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
        if (pPdfPageObject->getType() != FPDF_PAGEOBJ_PATH)
            continue;

        if (pPdfPageObject->getFillColor() == COL_YELLOW)
        {
            ++nYellowPathCount;
            // The path described a yellow rectangle, but it was not rotated.
            int nSegments = pPdfPageObject->getPathSegmentCount();
            CPPUNIT_ASSERT_EQUAL(5, nSegments);
            std::unique_ptr<vcl::pdf::PDFiumPathSegment> pSegment = pPdfPageObject->getPathSegment(0);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_MOVETO, pSegment->getType());
            basegfx::B2DPoint aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_EQUAL(245395, static_cast<int>(round(aPoint.getX() * 1000)));
            CPPUNIT_ASSERT_EQUAL(244261, static_cast<int>(round(aPoint.getY() * 1000)));
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(1);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_EQUAL(275102, static_cast<int>(round(aPoint.getX() * 1000)));
            CPPUNIT_ASSERT_EQUAL(267618, static_cast<int>(round(aPoint.getY() * 1000)));
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(2);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_EQUAL(287518, static_cast<int>(round(aPoint.getX() * 1000)));
            CPPUNIT_ASSERT_EQUAL(251829, static_cast<int>(round(aPoint.getY() * 1000)));
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(3);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_EQUAL(257839, static_cast<int>(round(aPoint.getX() * 1000)));
            CPPUNIT_ASSERT_EQUAL(228472, static_cast<int>(round(aPoint.getY() * 1000)));
            CPPUNIT_ASSERT(!pSegment->isClosed());

            pSegment = pPdfPageObject->getPathSegment(4);
            CPPUNIT_ASSERT_EQUAL(FPDF_SEGMENT_LINETO, pSegment->getType());
            aPoint = pSegment->getPoint();
            CPPUNIT_ASSERT_EQUAL(245395, static_cast<int>(round(aPoint.getX() * 1000)));
            CPPUNIT_ASSERT_EQUAL(244261, static_cast<int>(round(aPoint.getY() * 1000)));
            CPPUNIT_ASSERT(pSegment->isClosed());
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

// This requires Carlito font, if it is missing the test will most likely
// fail.
void PdfExportTest::testTdf115117_1()
{
#if HAVE_MORE_FONTS
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
#endif
}

// This requires DejaVu Sans font, if it is missing the test will most likely
// fail.
void PdfExportTest::testTdf115117_2()
{
#if HAVE_MORE_FONTS
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
#endif
}

void PdfExportTest::testTdf115117_1a()
{
#if HAVE_MORE_FONTS
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

void PdfExportTest::testTdf115117_2a()
{
#if HAVE_MORE_FONTS
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

void PdfExportTest::testTdf66597_1()
{
#if HAVE_MORE_FONTS
    // This requires Amiri font, if it is missing the test will fail.
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
#if HAVE_MORE_FONTS
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
#if HAVE_MORE_FONTS
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
#endif
}

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
    FPDF_IMAGEOBJ_METADATA aMeta;
    CPPUNIT_ASSERT(FPDFImageObj_GetImageMetadata(pPageObject->getPointer(), pPdfPage->getPointer(), &aMeta));
    // This was 2000, i.e. the 'reduce to 300 DPI' request was ignored.
    // This is now around 238 (228 on macOS).
    CPPUNIT_ASSERT_LESS(static_cast<unsigned int>(250), aMeta.width);
}

void PdfExportTest::testTdf128630()
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
        if (pPageObject->getType() != FPDF_PAGEOBJ_IMAGE)
            continue;

        FPDF_BITMAP pBitmap = FPDFImageObj_GetBitmap(pPageObject->getPointer());
        CPPUNIT_ASSERT(pBitmap);
        int nWidth = FPDFBitmap_GetWidth(pBitmap);
        int nHeight = FPDFBitmap_GetHeight(pBitmap);
        FPDFBitmap_Destroy(pBitmap);
        // Without the accompanying fix in place, this test would have failed with:
        // assertion failed
        // - Expression: nWidth != nHeight
        // i.e. the bitmap lost its custom aspect ratio during export.
        CPPUNIT_ASSERT(nWidth != nHeight);
    }
}

void PdfExportTest::testTdf106702()
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
        if (pPageObject->getType() != FPDF_PAGEOBJ_IMAGE)
            continue;

        float fLeft = 0, fBottom = 0, fRight = 0, fTop = 0;
        FPDFPageObj_GetBounds(pPageObject->getPointer(), &fLeft, &fBottom, &fRight, &fTop);
        nExpected = fTop;
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
        if (pPageObject->getType() != FPDF_PAGEOBJ_IMAGE)
            continue;

        float fLeft = 0, fBottom = 0, fRight = 0, fTop = 0;
        FPDFPageObj_GetBounds(pPageObject->getPointer(), &fLeft, &fBottom, &fRight, &fTop);
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
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());

    // First has the original (larger) image.
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    int nLarger = 0;
    int nPageObjectCount = pPdfPage->getObjectCount();
    for (int i = 0; i < nPageObjectCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(i);
        if (pPageObject->getType() != FPDF_PAGEOBJ_IMAGE)
            continue;

        float fLeft = 0, fBottom = 0, fRight = 0, fTop = 0;
        FPDFPageObj_GetBounds(pPageObject->getPointer(), &fLeft, &fBottom, &fRight, &fTop);
        nLarger = fRight - fLeft;
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
        if (pPageObject->getType() != FPDF_PAGEOBJ_IMAGE)
            continue;

        float fLeft = 0, fBottom = 0, fRight = 0, fTop = 0;
        FPDFPageObj_GetBounds(pPageObject->getPointer(), &fLeft, &fBottom, &fRight, &fTop);
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
    saveAsPDF("forcepoint71.key");
}

void PdfExportTest::testTdf115262()
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
        float fLeft = 0, fBottom = 0, fRight = 0, fTop = 0;
        FPDFPageObj_GetBounds(pPageObject->getPointer(), &fLeft, &fBottom, &fRight, &fTop);

        if (pPageObject->getType() == FPDF_PAGEOBJ_IMAGE)
        {
            nFirstImageTop = fTop;
        }
        else if (pPageObject->getType() == FPDF_PAGEOBJ_TEXT)
        {
            unsigned long nTextSize = FPDFTextObj_GetText(pPageObject->getPointer(), pTextPage->getPointer(), nullptr, 0);
            std::vector<sal_Unicode> aText(nTextSize);
            FPDFTextObj_GetText(pPageObject->getPointer(), pTextPage->getPointer(), aText.data(), nTextSize);
#if defined OSL_BIGENDIAN
            // The data returned by FPDFTextObj_GetText is documented to always be UTF-16LE:
            for (auto & j: aText) {
                j = OSL_SWAPWORD(j);
            }
#endif
            OUString sText(aText.data(), nTextSize / 2 - 1);
            if (sText == "400")
                nRowTop = fTop;
        }
    }
    // Make sure that the top of the "400" is below the top of the image (in
    // bottom-right-corner-based PDF coordinates).
    // This was: expected less than 144, actual is 199.
    CPPUNIT_ASSERT_LESS(nFirstImageTop, nRowTop);
}

void PdfExportTest::testTdf121962()
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
        if (pPageObject->getType() != FPDF_PAGEOBJ_TEXT)
            continue;
        unsigned long nTextSize = FPDFTextObj_GetText(pPageObject->getPointer(), pTextPage->getPointer(), nullptr, 0);
        std::vector<sal_Unicode> aText(nTextSize);
        FPDFTextObj_GetText(pPageObject->getPointer(), pTextPage->getPointer(), aText.data(), nTextSize);
#if defined OSL_BIGENDIAN
        // The data returned by FPDFTextObj_GetText is documented to always be UTF-16LE:
        for (auto & j: aText) {
            j = OSL_SWAPWORD(j);
        }
#endif
        OUString sText(aText.data(), nTextSize / 2 - 1);
        CPPUNIT_ASSERT(sText != "** Expression is faulty **");
    }
}

void PdfExportTest::testTdf115967()
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
        if (pPageObject->getType() != FPDF_PAGEOBJ_TEXT)
            continue;
        unsigned long nTextSize = FPDFTextObj_GetText(pPageObject->getPointer(), pTextPage->getPointer(), nullptr, 2);
        std::vector<sal_Unicode> aText(nTextSize);
        FPDFTextObj_GetText(pPageObject->getPointer(), pTextPage->getPointer(), aText.data(), nTextSize);
#if defined OSL_BIGENDIAN
        // The data returned by FPDFTextObj_GetText is documented to always be UTF-16LE:
        for (auto & j: aText) {
            j = OSL_SWAPWORD(j);
        }
#endif
        OUString sChar(aText.data(), nTextSize / 2 - 1);
        sText += sChar.trim();
    }
    CPPUNIT_ASSERT_EQUAL(OUString("m=750abc"), sText);
}

void PdfExportTest::testTdf121615()
{
    vcl::filter::PDFDocument aDocument;
    load("tdf121615.odt", aDocument);

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
    vcl::filter::PDFStreamElement* pStream = pXObject->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();

    // Load the embedded image.
    rObjectStream.Seek( 0 );
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    sal_uInt16 format;
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, OUString( "import" ), rObjectStream,
        GRFILTER_FORMAT_DONTKNOW, &format);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

    // The image should be grayscale 8bit JPEG.
    sal_uInt16 jpegFormat = rFilter.GetImportFormatNumberForShortName( JPG_SHORTNAME );
    CPPUNIT_ASSERT( jpegFormat != GRFILTER_FORMAT_NOTFOUND );
    CPPUNIT_ASSERT_EQUAL( jpegFormat, format );
    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL( 200L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL( 300L, aBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL( 8, int(aBitmap.GetBitCount()));
    // tdf#121615 was caused by broken handling of data width with 8bit color,
    // so the test image has some black in the bottomright corner, check it's there
    CPPUNIT_ASSERT_EQUAL( COL_WHITE, aBitmap.GetPixelColor( 0, 0 ));
    CPPUNIT_ASSERT_EQUAL( COL_WHITE, aBitmap.GetPixelColor( 0, 299 ));
    CPPUNIT_ASSERT_EQUAL( COL_WHITE, aBitmap.GetPixelColor( 199, 0 ));
    CPPUNIT_ASSERT_EQUAL( COL_BLACK, aBitmap.GetPixelColor( 199, 299 ));
}

void PdfExportTest::testTocLink()
{
    // Load the Writer document.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "toc-link.fodt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

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
    int nStartPos = 0;
    FPDF_LINK pLinkAnnot = nullptr;
    // Without the accompanying fix in place, this test would have failed, as FPDFLink_Enumerate()
    // returned false, as the page contained no links.
    CPPUNIT_ASSERT(FPDFLink_Enumerate(pPdfPage->getPointer(), &nStartPos, &pLinkAnnot));
}

void PdfExportTest::testReduceSmallImage()
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
    CPPUNIT_ASSERT_EQUAL(FPDF_PAGEOBJ_IMAGE, pPageObject->getType());

    // Make sure we don't scale down a tiny bitmap.
    FPDF_BITMAP pBitmap = FPDFImageObj_GetBitmap(pPageObject->getPointer());
    CPPUNIT_ASSERT(pBitmap);
    int nWidth = FPDFBitmap_GetWidth(pBitmap);
    int nHeight = FPDFBitmap_GetHeight(pBitmap);
    FPDFBitmap_Destroy(pBitmap);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 16
    // - Actual  : 6
    // i.e. the image was scaled down to 300 DPI, even if it had tiny size.
    CPPUNIT_ASSERT_EQUAL(16, nWidth);
    CPPUNIT_ASSERT_EQUAL(16, nHeight);
}

void PdfExportTest::testReduceImage()
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
    CPPUNIT_ASSERT_EQUAL(FPDF_PAGEOBJ_IMAGE, pPageObject->getType());

    // Make sure we don't scale down a bitmap.
    FPDF_BITMAP pBitmap = FPDFImageObj_GetBitmap(pPageObject->getPointer());
    CPPUNIT_ASSERT(pBitmap);
    int nWidth = FPDFBitmap_GetWidth(pBitmap);
    int nHeight = FPDFBitmap_GetHeight(pBitmap);
    FPDFBitmap_Destroy(pBitmap);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 160
    // - Actual  : 6
    // i.e. the image was scaled down even with ReduceImageResolution=false.
    CPPUNIT_ASSERT_EQUAL(160, nWidth);
    CPPUNIT_ASSERT_EQUAL(160, nHeight);
}

bool HasLinksOnPage(std::unique_ptr<vcl::pdf::PDFiumPage>& pPdfPage)
{
    int nStartPos = 0;
    FPDF_LINK pLinkAnnot = nullptr;
    return FPDFLink_Enumerate(pPdfPage->getPointer(), &nStartPos, &pLinkAnnot);
}

void PdfExportTest::testLinkWrongPage()
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
    CPPUNIT_ASSERT(HasLinksOnPage(pPdfPage));

    // Second page should have no links (3rd slide).
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage2 = pPdfDocument->openPage(/*nIndex=*/1);
    CPPUNIT_ASSERT(pPdfPage2);
    CPPUNIT_ASSERT(!HasLinksOnPage(pPdfPage2));
}

void PdfExportTest::testLargePage()
{
    // Import the bugdoc and export as PDF.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "6m-wide.odg";
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("draw_pdf_Export");
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = exportAndParse(aURL, aMediaDescriptor);

    // The document has 1 page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    // Check the value (not the unit) of the page size.
    FS_SIZEF aSize;
    FPDF_GetPageSizeByIndexF(pPdfDocument->getPointer(), 0, &aSize);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 8503.94
    // - Actual  : 17007.875
    // i.e. the value for 600 cm was larger than the 14 400 limit set in the spec.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8503.94, static_cast<double>(aSize.width), 0.01);
}

void PdfExportTest::testPdfImageResourceInlineXObjectRef()
{
    // Create an empty document.
    mxComponent = loadFromDesktop("private:factory/swriter");
    CPPUNIT_ASSERT(mxComponent.is());
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
    CPPUNIT_ASSERT_EQUAL(FPDF_PAGEOBJ_FORM, pPageObject->getType());
    // 2: white background and the actual object.
    CPPUNIT_ASSERT_EQUAL(2, FPDFFormObj_CountObjects(pPageObject->getPointer()));
    FPDF_PAGEOBJECT pFormObject = FPDFFormObj_GetObject(pPageObject->getPointer(), 1);
    CPPUNIT_ASSERT_EQUAL(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(pFormObject));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the sub-form was missing its image.
    CPPUNIT_ASSERT_EQUAL(1, FPDFFormObj_CountObjects(pFormObject));

    // Check if the inner form object (original page object in the pdf image) has the correct
    // rotation.
    FPDF_PAGEOBJECT pInnerFormObject = FPDFFormObj_GetObject(pFormObject, 0);
    CPPUNIT_ASSERT_EQUAL(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(pInnerFormObject));
    CPPUNIT_ASSERT_EQUAL(1, FPDFFormObj_CountObjects(pInnerFormObject));
    FPDF_PAGEOBJECT pImage = FPDFFormObj_GetObject(pInnerFormObject, 0);
    CPPUNIT_ASSERT_EQUAL(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(pImage));
    FS_MATRIX aMatrix;
    FPDFFormObj_GetMatrix(pInnerFormObject, &aMatrix);
    basegfx::B2DHomMatrix aMat{ aMatrix.a, aMatrix.c, aMatrix.e, aMatrix.b, aMatrix.d, aMatrix.f };
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

void PdfExportTest::testDefaultVersion()
{
    // Create an empty document.
    mxComponent = loadFromDesktop("private:factory/swriter");
    CPPUNIT_ASSERT(mxComponent.is());

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
    int nFileVersion = 0;
    FPDF_GetFileVersion(pPdfDocument->getPointer(), &nFileVersion);
    CPPUNIT_ASSERT_EQUAL(16, nFileVersion);
}

void PdfExportTest::testVersion15()
{
    // Create an empty document.
    mxComponent = loadFromDesktop("private:factory/swriter");
    CPPUNIT_ASSERT(mxComponent.is());

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
    int nFileVersion = 0;
    FPDF_GetFileVersion(pPdfDocument->getPointer(), &nFileVersion);
    CPPUNIT_ASSERT_EQUAL(15, nFileVersion);
}

// Check round-trip of importing and exporting the PDF with PDFium filter,
// which imports the PDF document as multiple PDFs as graphic object.
// Each page in the document has one PDF graphic object which content is
// the corresponding page in the PDF. When such a document is exported,
// the PDF graphic gets embedded into the exported PDF document (as a
// Form XObject).
void PdfExportTest::testMultiPagePDF()
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
    CPPUNIT_ASSERT(mxComponent.is());

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

    auto pXObjects = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pResources->Lookup("XObject"));
    CPPUNIT_ASSERT(pXObjects);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pXObjects->GetItems().size()); // 3 PDFs as Form XObjects

    std::vector<OString> rIDs;
    for (auto const & rPair : pXObjects->GetItems()) {
        rIDs.push_back(rPair.first);
    }

    // Let's check the embedded PDF pages - just make sure the size differs,
    // which should indicate we don't have 3 times the same page.

    {   // embedded PDF page 1
        vcl::filter::PDFObjectElement* pXObject1 = pXObjects->LookupObject(rIDs[0]);
        CPPUNIT_ASSERT(pXObject1);
        CPPUNIT_ASSERT_EQUAL(OString("Im19"), rIDs[0]);

        auto pSubtype1 = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject1->Lookup("Subtype"));
        CPPUNIT_ASSERT(pSubtype1);
        CPPUNIT_ASSERT_EQUAL(OString("Form"), pSubtype1->GetValue());

        auto pXObjectResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject1->Lookup("Resources"));
        CPPUNIT_ASSERT(pXObjectResources);
        auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObjectResources->LookupElement("XObject"));
        CPPUNIT_ASSERT(pXObjectForms);
        vcl::filter::PDFObjectElement* pForm = pXObjectForms->LookupObject(pXObjectForms->GetItems().begin()->first);
        CPPUNIT_ASSERT(pForm);

        vcl::filter::PDFStreamElement* pStream = pForm->GetStream();
        CPPUNIT_ASSERT(pStream);
        SvMemoryStream& rObjectStream = pStream->GetMemory();
        rObjectStream.Seek(STREAM_SEEK_TO_BEGIN);

        // Just check that the size of the page stream is what is expected.
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(230), rObjectStream.remainingSize());
    }

    {   // embedded PDF page 2
        vcl::filter::PDFObjectElement* pXObject2 = pXObjects->LookupObject(rIDs[1]);
        CPPUNIT_ASSERT(pXObject2);
        CPPUNIT_ASSERT_EQUAL(OString("Im34"), rIDs[1]);

        auto pSubtype2 = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject2->Lookup("Subtype"));
        CPPUNIT_ASSERT(pSubtype2);
        CPPUNIT_ASSERT_EQUAL(OString("Form"), pSubtype2->GetValue());

        auto pXObjectResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject2->Lookup("Resources"));
        CPPUNIT_ASSERT(pXObjectResources);
        auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObjectResources->LookupElement("XObject"));
        CPPUNIT_ASSERT(pXObjectForms);
        vcl::filter::PDFObjectElement* pForm = pXObjectForms->LookupObject(pXObjectForms->GetItems().begin()->first);
        CPPUNIT_ASSERT(pForm);

        vcl::filter::PDFStreamElement* pStream = pForm->GetStream();
        CPPUNIT_ASSERT(pStream);
        SvMemoryStream& rObjectStream = pStream->GetMemory();
        rObjectStream.Seek(STREAM_SEEK_TO_BEGIN);

        // Just check that the size of the page stream is what is expected
        CPPUNIT_ASSERT_EQUAL(sal_uInt64(309), rObjectStream.remainingSize());
    }

    {   // embedded PDF page 3
        vcl::filter::PDFObjectElement* pXObject3 = pXObjects->LookupObject(rIDs[2]);
        CPPUNIT_ASSERT(pXObject3);
        CPPUNIT_ASSERT_EQUAL(OString("Im4"), rIDs[2]);

        auto pSubtype3 = dynamic_cast<vcl::filter::PDFNameElement*>(pXObject3->Lookup("Subtype"));
        CPPUNIT_ASSERT(pSubtype3);
        CPPUNIT_ASSERT_EQUAL(OString("Form"), pSubtype3->GetValue());

        auto pXObjectResources = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObject3->Lookup("Resources"));
        CPPUNIT_ASSERT(pXObjectResources);
        auto pXObjectForms = dynamic_cast<vcl::filter::PDFDictionaryElement*>(pXObjectResources->LookupElement("XObject"));
        CPPUNIT_ASSERT(pXObjectForms);
        vcl::filter::PDFObjectElement* pForm = pXObjectForms->LookupObject(pXObjectForms->GetItems().begin()->first);
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

void PdfExportTest::testFormFontName()
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
    CPPUNIT_ASSERT_EQUAL(FPDF_OBJECT_STRING, FPDFAnnot_GetValueType(pAnnot->getPointer(), "DA"));
    size_t nDALength = FPDFAnnot_GetStringValue(pAnnot->getPointer(), "DA", nullptr, 0);
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), nDALength % 2);
    std::vector<sal_Unicode> aDABuf(nDALength / 2);
    FPDFAnnot_GetStringValue(
        pAnnot->getPointer(), "DA", reinterpret_cast<FPDF_WCHAR *>(aDABuf.data()), nDALength);
#if defined OSL_BIGENDIAN
    // The data returned by FPDFAnnot_GetStringValue is documented to always be UTF-16LE:
    for (auto & i: aDABuf) {
        i = OSL_SWAPWORD(i);
    }
#endif
    OUString aDA(reinterpret_cast<sal_Unicode*>(aDABuf.data()));

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0 0 0 rg /TiRo 12 Tf
    // - Actual  : 0 0 0 rg /F2 12 Tf
    // i.e. Liberation Serif was exposed as a form font as-is, without picking the closest built-in
    // font.
    CPPUNIT_ASSERT_EQUAL(OUString("0 0 0 rg /TiRo 12 Tf"), aDA);
}

CPPUNIT_TEST_SUITE_REGISTRATION(PdfExportTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
