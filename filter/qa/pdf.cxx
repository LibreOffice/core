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
#include <tools/helpers.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers filter/source/pdf/ fixes.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/filter/qa/data/"_ustr)
    {
    }

    void setUp() override;
    void doTestCommentsInMargin(bool commentsInMarginEnabled);
};

void Test::setUp()
{
    UnoApiTest::setUp();

    MacrosTest::setUpX509(m_directories, u"filter_pdf"_ustr);
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
        comphelper::makePropertyValue(u"SignPDF"_ustr, true),
        comphelper::makePropertyValue(
            u"SignCertificateSubjectName"_ustr,
            u"CN=Xmlsecurity RSA Test example Alice,O=Xmlsecurity RSA Test,ST=England,C=UK"_ustr),
    };
    if (!GetValidCertificate(xSecurityEnvironment->getPersonalCertificates(), xSecurityEnvironment,
                             aFilterData))
    {
        return;
    }

    // Given an empty document:
    mxComponent.set(
        loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr));

    // When exporting to PDF, and referring to a certificate using a subject name:
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance(u"com.sun.star.document.PDFFilter"_ustr), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue(u"FilterName"_ustr, u"writer_pdf_Export"_ustr),
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
        comphelper::makePropertyValue(u"OutputStream"_ustr, xOutputStream),
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
    mxComponent.set(
        loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr));

    // When inserting a 267 points wide PDF image into the document:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FileName"_ustr, createFileURL(u"picture.pdf")),
    };
    dispatchCommand(mxComponent, u".uno:InsertGraphic"_ustr, aArgs);

    // Then make sure that its size is correct:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    auto xGraphic
        = xShape->getPropertyValue(u"Graphic"_ustr).get<uno::Reference<graphic::XGraphic>>();
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

void Test::doTestCommentsInMargin(bool commentsInMarginEnabled)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    loadFromFile(u"commentsInMargin.odt");
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance(u"com.sun.star.document.PDFFilter"_ustr), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aFilterData{ comphelper::makePropertyValue(
        u"ExportNotesInMargin"_ustr, commentsInMarginEnabled) };
    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue(u"FilterName"_ustr, u"writer_pdf_Export"_ustr),
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
        comphelper::makePropertyValue(u"OutputStream"_ustr, xOutputStream),
    };
    xFilter->filter(aDescriptor);

    // Make sure the number of objects is correct
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aStream.GetData(), aStream.GetSize(), OString());
    CPPUNIT_ASSERT(pPdfDocument);
    if (commentsInMarginEnabled)
    {
        // Unfortunately, the comment box is DPI dependent, and the lines there may split
        // at higher DPIs, creating additional objects on import, hence the "_GREATER"
        CPPUNIT_ASSERT_GREATER(8, pPdfDocument->openPage(0)->getObjectCount());
    }
    else
        CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->openPage(0)->getObjectCount());
}

CPPUNIT_TEST_FIXTURE(Test, testCommentsInMargin)
{
    // Test that setting/unsetting the "ExportNotesInMargin" property works correctly
    doTestCommentsInMargin(true);
    doTestCommentsInMargin(false);
}

CPPUNIT_TEST_FIXTURE(Test, testWatermarkColor)
{
    // Given an empty Writer document:
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;
    mxComponent.set(
        loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr));

    // When exporting that as PDF with a red watermark:
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance(u"com.sun.star.document.PDFFilter"_ustr), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue(u"Watermark"_ustr, u"X"_ustr),
        comphelper::makePropertyValue(u"WatermarkColor"_ustr, static_cast<sal_Int32>(0xff0000)),
    };
    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue(u"FilterName"_ustr, u"writer_pdf_Export"_ustr),
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
        comphelper::makePropertyValue(u"OutputStream"_ustr, xOutputStream),
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
    mxComponent.set(
        loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr));

    // When exporting that as PDF with a 100pt-sized watermark:
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance(u"com.sun.star.document.PDFFilter"_ustr), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    sal_Int32 nExpectedFontSize = 100;
    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue(u"Watermark"_ustr, u"X"_ustr),
        comphelper::makePropertyValue(u"WatermarkFontHeight"_ustr, nExpectedFontSize),
    };
    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue(u"FilterName"_ustr, u"writer_pdf_Export"_ustr),
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
        comphelper::makePropertyValue(u"OutputStream"_ustr, xOutputStream),
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
    mxComponent.set(
        loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr));

    // When exporting that as PDF with a serif watermark:
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance(u"com.sun.star.document.PDFFilter"_ustr), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    OUString aExpectedFontName(u"Liberation Serif"_ustr);
    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue(u"Watermark"_ustr, u"X"_ustr),
        comphelper::makePropertyValue(u"WatermarkFontName"_ustr, aExpectedFontName),
    };
    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue(u"FilterName"_ustr, u"writer_pdf_Export"_ustr),
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
        comphelper::makePropertyValue(u"OutputStream"_ustr, xOutputStream),
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

CPPUNIT_TEST_FIXTURE(Test, testWatermarkRotateAngle)
{
    // Given an empty Writer document:
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;
    mxComponent.set(
        loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr));

    // When exporting that as PDF with a rotated watermark:
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance(u"com.sun.star.document.PDFFilter"_ustr), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(mxComponent);
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    // 45.0 degrees, counter-clockwise.
    sal_Int32 nExpectedRotateAngle = 45;
    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue(u"Watermark"_ustr, u"X"_ustr),
        comphelper::makePropertyValue(u"WatermarkRotateAngle"_ustr, nExpectedRotateAngle * 10),
    };
    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue(u"FilterName"_ustr, u"writer_pdf_Export"_ustr),
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
        comphelper::makePropertyValue(u"OutputStream"_ustr, xOutputStream),
    };
    xFilter->filter(aDescriptor);

    // Then make sure that the watermark rotation angle is correct:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aStream.GetData(), aStream.GetSize(), OString());
    CPPUNIT_ASSERT(pPdfDocument);
    std::unique_ptr<vcl::pdf::PDFiumPage> pPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT_EQUAL(1, pPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPage->getObject(0);
    CPPUNIT_ASSERT_EQUAL(1, pPageObject->getFormObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pFormObject = pPageObject->getFormObject(0);
    basegfx::B2DHomMatrix aMatrix = pFormObject->getMatrix();
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate{};
    double fShearX{};
    aMatrix.decompose(aScale, aTranslate, fRotate, fShearX);
    sal_Int32 nActualRotateAngle = NormAngle360(basegfx::rad2deg<1>(fRotate));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 45
    // - Actual  : 270
    // i.e. the rotation angle was 270 for an A4 page, not the requested 45 degrees.
    CPPUNIT_ASSERT_EQUAL(nExpectedRotateAngle, nActualRotateAngle);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
