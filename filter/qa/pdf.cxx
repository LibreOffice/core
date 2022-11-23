/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <comphelper/propertyvalue.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers filter/source/pdf/ fixes.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/filter/qa/data/")
    {
    }

    void setUp() override;
    void tearDown() override;
};

void Test::setUp()
{
    UnoApiTest::setUp();

    MacrosTest::setUpNssGpg(m_directories, "filter_pdf");
}

void Test::tearDown()
{
    MacrosTest::tearDownNssGpg();

    UnoApiTest::tearDown();
}

CPPUNIT_TEST_FIXTURE(Test, testSignCertificateSubjectName)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer
        = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext
        = xSEInitializer->createSecurityContext(OUString());
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment
        = xSecurityContext->getSecurityEnvironment();
    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("SignPDF", true),
        comphelper::makePropertyValue(
            "SignCertificateSubjectName",
            OUString(
                "CN=Xmlsecurity RSA Test example Alice,O=Xmlsecurity RSA Test,ST=England,C=UK")),
    };
    if (!GetValidCertificate(xSecurityEnvironment->getPersonalCertificates(), xSecurityEnvironment,
                             aFilterData))
    {
        return;
    }

    // Given an empty document:
    mxComponent.set(loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"));

    // When exporting to PDF, and referring to a certificate using a subject name:
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance("com.sun.star.document.PDFFilter"), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("FilterName", OUString("writer_pdf_Export")),
        comphelper::makePropertyValue("FilterData", aFilterData),
        comphelper::makePropertyValue("OutputStream", xOutputStream),
    };
    xFilter->filter(aDescriptor);

    // Then make sure the resulting PDF has a signature:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aStream.GetData(), aStream.GetSize(), OString());
    // Without the accompanying fix in place, this test would have failed, as signing was enabled
    // without configuring a certificate, so the whole export failed.
    CPPUNIT_ASSERT(pPdfDocument);
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getSignatureCount());
}

CPPUNIT_TEST_FIXTURE(Test, testPdfDecompositionSize)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Given an empty Writer document:
    mxComponent.set(loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"));

    // When inserting a 267 points wide PDF image into the document:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FileName", createFileURL(u"picture.pdf")),
    };
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);

    // Then make sure that its size is correct:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    auto xGraphic = xShape->getPropertyValue("Graphic").get<uno::Reference<graphic::XGraphic>>();
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    basegfx::B2DRange aRange = aGraphic.getVectorGraphicData()->getRange();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 9437
    // - Actual  : 34176
    // i.e. the width was too large, it used all width of the body frame.
    // 9437 mm100 is 267.507 points from the file.
#if defined MACOSX
    // TODO the bitmap size is larger (75486) on macOS, but that should not affect the logic size.
    (void)aRange;
#else
    // Unfortunately, this test is DPI-dependent.
    // Use some allowance (~1/2 pt) to let it pass on non-default DPI.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9437, aRange.getWidth(), 20.0);
#endif
}

CPPUNIT_TEST_FIXTURE(Test, testWatermarkColor)
{
    // Given an empty Writer document:
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;
    mxComponent.set(loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"));

    // When exporting that as PDF with a red watermark:
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance("com.sun.star.document.PDFFilter"), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("Watermark", OUString("X")),
        comphelper::makePropertyValue("WatermarkColor", static_cast<sal_Int32>(0xff0000)),
    };
    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("FilterName", OUString("writer_pdf_Export")),
        comphelper::makePropertyValue("FilterData", aFilterData),
        comphelper::makePropertyValue("OutputStream", xOutputStream),
    };
    xFilter->filter(aDescriptor);

    // Then make sure that the watermark color is correct:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aStream.GetData(), aStream.GetSize(), OString());
    CPPUNIT_ASSERT(pPdfDocument);
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT_EQUAL(1, pPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(0);
    CPPUNIT_ASSERT_EQUAL(1, pPageObject->getFormObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pFormObject = pPageObject->getFormObject(0);
    Color aFillColor = pFormObject->getFillColor();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: rgba[ff0000ff]
    // - Actual  : rgba[00ff00ff]
    // i.e. the color was the (default) green, not red.
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xff0000), aFillColor);
}

CPPUNIT_TEST_FIXTURE(Test, testWatermarkFontHeight)
{
    // Given an empty Writer document:
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;
    mxComponent.set(loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"));

    // When exporting that as PDF with a 100pt-sized watermark:
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance("com.sun.star.document.PDFFilter"), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    sal_Int32 nExpectedFontSize = 100;
    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("Watermark", OUString("X")),
        comphelper::makePropertyValue("WatermarkFontHeight", nExpectedFontSize),
    };
    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("FilterName", OUString("writer_pdf_Export")),
        comphelper::makePropertyValue("FilterData", aFilterData),
        comphelper::makePropertyValue("OutputStream", xOutputStream),
    };
    xFilter->filter(aDescriptor);

    // Then make sure that the watermark font size is correct:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aStream.GetData(), aStream.GetSize(), OString());
    CPPUNIT_ASSERT(pPdfDocument);
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT_EQUAL(1, pPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(0);
    CPPUNIT_ASSERT_EQUAL(1, pPageObject->getFormObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pFormObject = pPageObject->getFormObject(0);
    sal_Int32 nFontSize = pFormObject->getFontSize();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 100
    // - Actual  : 594
    // i.e. the font size was automatic, could not specify an explicit size.
    CPPUNIT_ASSERT_EQUAL(nExpectedFontSize, nFontSize);
}

CPPUNIT_TEST_FIXTURE(Test, testWatermarkFontName)
{
    // Given an empty Writer document:
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;
    mxComponent.set(loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"));

    // When exporting that as PDF with a serif watermark:
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance("com.sun.star.document.PDFFilter"), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    OUString aExpectedFontName("Liberation Serif");
    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("Watermark", OUString("X")),
        comphelper::makePropertyValue("WatermarkFontName", aExpectedFontName),
    };
    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("FilterName", OUString("writer_pdf_Export")),
        comphelper::makePropertyValue("FilterData", aFilterData),
        comphelper::makePropertyValue("OutputStream", xOutputStream),
    };
    xFilter->filter(aDescriptor);

    // Then make sure that the watermark font name is correct:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aStream.GetData(), aStream.GetSize(), OString());
    CPPUNIT_ASSERT(pPdfDocument);
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT_EQUAL(1, pPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(0);
    CPPUNIT_ASSERT_EQUAL(1, pPageObject->getFormObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pFormObject = pPageObject->getFormObject(0);
    OUString aFontName = pFormObject->getFontName();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Liberation Serif
    // - Actual  : Helvetica
    // i.e. the font name was sans, could not specify an explicit name.
    CPPUNIT_ASSERT_EQUAL(aExpectedFontName, aFontName);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
