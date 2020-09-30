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

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/propertyvalue.hxx>
#include <osl/file.hxx>
#include <unotools/tempfile.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <svx/svdview.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/signaturelinehelper.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace ::com::sun::star;

namespace
{
char const DATA_DIRECTORY[] = "/vcl/qa/cppunit/filter/ipdf/data/";
}

/// Covers vcl/source/filter/ipdf/ fixes.
class VclFilterIpdfTest : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;
    uno::Reference<xml::crypto::XSEInitializer> mxSEInitializer;
    uno::Reference<xml::crypto::XXMLSecurityContext> mxSecurityContext;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    uno::Reference<xml::crypto::XXMLSecurityContext>& getSecurityContext()
    {
        return mxSecurityContext;
    }
};

void VclFilterIpdfTest::setUp()
{
    test::BootstrapFixture::setUp();
    MacrosTest::setUpNssGpg(m_directories, "vcl_filter_ipdf");

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
    mxSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    mxSecurityContext = mxSEInitializer->createSecurityContext(OUString());
}

void VclFilterIpdfTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    MacrosTest::tearDownNssGpg();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_FIXTURE(VclFilterIpdfTest, testPDFAddVisibleSignatureLastPage)
{
    // Given: copy the test document to a temporary file, as it'll be modified.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    OUString aSourceURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "add-visible-signature-last-page.pdf";
    OUString aURL = aTempFile.GetURL();
    osl::File::RC eRet = osl::File::copy(aSourceURL, aURL);
    CPPUNIT_ASSERT_EQUAL(osl::File::RC::E_None, eRet);

    // Open it.
    uno::Sequence<beans::PropertyValue> aArgs = { comphelper::makePropertyValue("ReadOnly", true) };
    getComponent() = loadFromDesktop(aURL, "com.sun.star.drawing.DrawingDocument", aArgs);
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(getComponent().get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);

    // Add a signature line to the 2nd page.
    uno::Reference<lang::XMultiServiceFactory> xFactory(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"), uno::UNO_QUERY);
    xShape->setPosition(awt::Point(1000, 15000));
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XDrawPagesSupplier> xSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xSupplier->getDrawPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xDrawPages->getCount());

    uno::Reference<frame::XModel> xModel(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawView> xController(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(1), uno::UNO_QUERY);
    xController->setCurrentPage(xDrawPage);
    xDrawPage->add(xShape);

    // Select it and assign a certificate.
    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(pBaseModel->getCurrentController(),
                                                                uno::UNO_QUERY);
    xSelectionSupplier->select(uno::makeAny(xShape));
    uno::Sequence<uno::Reference<security::XCertificate>> aCertificates
        = getSecurityContext()->getSecurityEnvironment()->getPersonalCertificates();
    if (!aCertificates.hasElements())
    {
        return;
    }
    SdrView* pView = SfxViewShell::Current()->GetDrawView();
    svx::SignatureLineHelper::setShapeCertificate(pView, aCertificates[0]);

    // When: do the actual signing.
    pObjectShell->SignDocumentContentUsingCertificate(aCertificates[0]);

    // Then: count the # of shapes on the signature widget/annotation.
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    SvFileStream aFile(aTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    // Last page.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the signature was there, but it was on the first page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnot = pPdfPage->getAnnotation(0);
    CPPUNIT_ASSERT_EQUAL(4, pAnnot->getObjectCount());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
