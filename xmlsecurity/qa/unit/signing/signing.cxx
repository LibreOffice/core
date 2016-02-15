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

#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/security/SerialNumberAdapter.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/packages/manifest/ManifestReader.hpp>

#include <comphelper/processfactory.hxx>
#include <sax/tools/converter.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/storagehelper.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/objsh.hxx>

#include <xmlsecurity/documentsignaturehelper.hxx>
#include <xmlsecurity/xmlsignaturehelper.hxx>
#include <documentsignaturemanager.hxx>

using namespace com::sun::star;

namespace
{
const char* DATA_DIRECTORY = "/xmlsecurity/qa/unit/signing/data/";
}

/// Testsuite for the document signing feature.
class SigningTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;

public:
    SigningTest();
    virtual void setUp() override;
    virtual void tearDown() override;

    void testDescription();
    /// Test a typical OOXML where a number of (but not all) streams are signed.
    void testOOXMLPartial();
    /// Test a typical broken OOXML signature where one stream is corrupted.
    void testOOXMLBroken();
    void testOOXMLDescription();

    CPPUNIT_TEST_SUITE(SigningTest);
    CPPUNIT_TEST(testDescription);
    CPPUNIT_TEST(testOOXMLPartial);
    CPPUNIT_TEST(testOOXMLBroken);
    CPPUNIT_TEST(testOOXMLDescription);
    CPPUNIT_TEST_SUITE_END();

private:
    void createDoc(const OUString& rURL = OUString());
    uno::Reference<security::XCertificate> getCertificate(XMLSignatureHelper& rSignatureHelper);
};

SigningTest::SigningTest()
{
}

void SigningTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void SigningTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void SigningTest::createDoc(const OUString& rURL)
{
    if (mxComponent.is())
        mxComponent->dispose();
    if (rURL.isEmpty())
        mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    else
        mxComponent = loadFromDesktop(rURL, "com.sun.star.text.TextDocument");
}

uno::Reference<security::XCertificate> SigningTest::getCertificate(XMLSignatureHelper& rSignatureHelper)
{
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = rSignatureHelper.GetSecurityEnvironment();
    OUString aCertificate;
    {
        SvFileStream aStream(getURLFromSrc(DATA_DIRECTORY) + "certificate.crt", StreamMode::READ);
        OString aLine;
        bool bMore = aStream.ReadLine(aLine);
        while (bMore)
        {
            aCertificate += OUString::fromUtf8(aLine);
            aCertificate += "\n";
            bMore = aStream.ReadLine(aLine);
        }
    }
    return xSecurityEnvironment->createCertificateFromAscii(aCertificate);
}

void SigningTest::testDescription()
{
    // Create an empty document and store it to a tempfile, finally load it as a storage.
    createDoc();

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer8");
    xStorable->storeAsURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    DocumentSignatureManager aManager(mxComponentContext, SignatureModeDocumentContent);
    CPPUNIT_ASSERT(aManager.maSignatureHelper.Init());
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");

    // Then add a signature document.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager.maSignatureHelper);
    CPPUNIT_ASSERT(xCertificate.is());
    OUString aDescription("SigningTest::testDescription");
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, aDescription, nSecurityId);

    // Read back the signature and make sure that the description survives the roundtrip.
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rInformations.size());
    CPPUNIT_ASSERT_EQUAL(aDescription, rInformations[0].ouDescription);
}

void SigningTest::testOOXMLDescription()
{
    // Create an empty document and store it to a tempfile, finally load it as a storage.
    createDoc();

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("MS Word 2007 XML");
    xStorable->storeAsURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    DocumentSignatureManager aManager(mxComponentContext, SignatureModeDocumentContent);
    CPPUNIT_ASSERT(aManager.maSignatureHelper.Init());
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");

    // Then add a signature document.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager.maSignatureHelper);
    CPPUNIT_ASSERT(xCertificate.is());
    OUString aDescription("SigningTest::testDescription");
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, aDescription, nSecurityId);

    // Read back the signature and make sure that the description survives the roundtrip.
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rInformations.size());
    CPPUNIT_ASSERT_EQUAL(aDescription, rInformations[0].ouDescription);
}

void SigningTest::testOOXMLPartial()
{
    createDoc(getURLFromSrc(DATA_DIRECTORY) + "partial.docx");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // This was SignatureState::BROKEN due to missing RelationshipTransform and SHA-256 support.
    // We expect NOTVALIDATED in case the root CA is not imported on the system, and PARTIAL_OK otherwise, so accept both.
    int nActual = static_cast<int>(pObjectShell->GetDocumentSignatureState());
    CPPUNIT_ASSERT(nActual == static_cast<int>(SignatureState::NOTVALIDATED) || nActual == static_cast<int>(SignatureState::PARTIAL_OK));
}

void SigningTest::testOOXMLBroken()
{
    createDoc(getURLFromSrc(DATA_DIRECTORY) + "bad.docx");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // This was SignatureState::NOTVALIDATED/PARTIAL_OK as we did not validate manifest references.
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN), static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SigningTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
