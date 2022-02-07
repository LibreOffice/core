/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/Desktop.hpp>
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
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();
    MacrosTest::setUpNssGpg(m_directories, "filter_pdf");

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

constexpr OUStringLiteral DATA_DIRECTORY = u"/filter/qa/data/";

CPPUNIT_TEST_FIXTURE(Test, testSignCertificateSubjectName)
{
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer
        = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext
        = xSEInitializer->createSecurityContext(OUString());
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment
        = xSecurityContext->getSecurityEnvironment();
    if (!xSecurityEnvironment->getPersonalCertificates().hasElements())
    {
        return;
    }

    // Given an empty document:
    getComponent().set(
        loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"));

    // When exporting to PDF, and referring to a certificate using a subject name:
    uno::Reference<css::lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();
    uno::Reference<document::XFilter> xFilter(
        xFactory->createInstance("com.sun.star.document.PDFFilter"), uno::UNO_QUERY);
    uno::Reference<document::XExporter> xExporter(xFilter, uno::UNO_QUERY);
    xExporter->setSourceDocument(getComponent());
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("SignPDF", true),
        comphelper::makePropertyValue(
            "SignCertificateSubjectName",
            OUString(
                "CN=Xmlsecurity RSA Test example Alice,O=Xmlsecurity RSA Test,ST=England,C=UK")),
    };
    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("FilterName", OUString("writer_pdf_Export")),
        comphelper::makePropertyValue("FilterData", aFilterData),
        comphelper::makePropertyValue("OutputStream", xOutputStream),
    };
    xFilter->filter(aDescriptor);

    // Then make sure the resulting PDF has a signature:
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aStream.GetData(), aStream.GetSize());
    // Without the accompanying fix in place, this test would have failed, as signing was enabled
    // without configuring a certificate, so the whole export failed.
    CPPUNIT_ASSERT(pPdfDocument);
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getSignatureCount());
}

CPPUNIT_TEST_FIXTURE(Test, testPdfDecompositionSize)
{
    // Given an empty Writer document:
    getComponent().set(
        loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument"));

    // When inserting a 267 points wide PDF image into the document:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FileName",
                                      m_directories.getURLFromSrc(DATA_DIRECTORY) + "picture.pdf"),
    };
    dispatchCommand(getComponent(), ".uno:InsertGraphic", aArgs);

    // Then make sure that its size is correct:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    auto xGraphic = xShape->getPropertyValue("Graphic").get<uno::Reference<graphic::XGraphic>>();
    CPPUNIT_ASSERT(xGraphic.is());
    Graphic aGraphic(xGraphic);
    basegfx::B2DRange aRange = aGraphic.getVectorGraphicData()->getRange();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 9419
    // - Actual  : 34176
    // i.e. the width was too large, it used all width of the body frame.
    // 9419 mm100 is 267 points from the file.
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(9419), aRange.getWidth());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
