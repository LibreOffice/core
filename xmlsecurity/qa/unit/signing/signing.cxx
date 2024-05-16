/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_crypto.h>
#include <config_features.h>
#include <config_gpgme.h>

#include <sal/config.h>

#if USE_CRYPTO_NSS
#include <secoid.h>
#endif

#include <test/unoapixml_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/objsh.hxx>
#include <osl/thread.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <unotools/streamwrap.hxx>

#include <documentsignaturehelper.hxx>
#include <xmlsignaturehelper.hxx>
#include <documentsignaturemanager.hxx>
#include <biginteger.hxx>
#include <certificate.hxx>
#include <xsecctl.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <officecfg/Office/Common.hxx>
#include <comphelper/configuration.hxx>
#include <svx/signaturelinehelper.hxx>
#include <sfx2/viewsh.hxx>
#include <comphelper/propertyvalue.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/scheduler.hxx>

using namespace com::sun::star;

/// Testsuite for the document signing feature.
class SigningTest : public UnoApiXmlTest
{
protected:
    uno::Reference<xml::crypto::XSEInitializer> mxSEInitializer;
    uno::Reference<xml::crypto::XXMLSecurityContext> mxSecurityContext;

public:
    SigningTest();
    virtual void setUp() override;
    virtual void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;

protected:
    uno::Reference<security::XCertificate>
    getCertificate(DocumentSignatureManager& rSignatureManager,
                   svl::crypto::SignatureMethodAlgorithm eAlgo);
#if HAVE_FEATURE_GPGVERIFY
    SfxObjectShell* assertDocument(const ::CppUnit::SourceLine aSrcLine,
                                   const OUString& rFilterName, const SignatureState nDocSign,
                                   const SignatureState nMacroSign, const OUString& sVersion);
#endif
};

SigningTest::SigningTest()
    : UnoApiXmlTest(u"/xmlsecurity/qa/unit/signing/data/"_ustr)
{
}

void SigningTest::setUp()
{
    UnoApiXmlTest::setUp();

    MacrosTest::setUpX509(m_directories, u"xmlsecurity_signing"_ustr);
    MacrosTest::setUpGpg(m_directories, u"xmlsecurity_signing"_ustr);

    // Initialize crypto after setting up the environment variables.
    mxSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    mxSecurityContext = mxSEInitializer->createSecurityContext(OUString());
#if USE_CRYPTO_NSS
#ifdef NSS_USE_ALG_IN_ANY_SIGNATURE
    // policy may disallow using SHA1 for signatures but unit test documents
    // have such existing signatures (call this after createSecurityContext!)
    NSS_SetAlgorithmPolicy(SEC_OID_SHA1, NSS_USE_ALG_IN_ANY_SIGNATURE, 0);
#endif
#endif
}

void SigningTest::tearDown()
{
    MacrosTest::tearDownGpg();

    UnoApiXmlTest::tearDown();
}

uno::Reference<security::XCertificate>
SigningTest::getCertificate(DocumentSignatureManager& rSignatureManager,
                            svl::crypto::SignatureMethodAlgorithm eAlgo)
{
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment
        = rSignatureManager.getSecurityEnvironment();
    const uno::Sequence<uno::Reference<security::XCertificate>> aCertificates
        = xSecurityEnvironment->getPersonalCertificates();

    for (const auto& xCertificate : aCertificates)
    {
        auto pCertificate = dynamic_cast<xmlsecurity::Certificate*>(xCertificate.get());
        CPPUNIT_ASSERT(pCertificate);
        if (pCertificate->getSignatureMethodAlgorithm() == eAlgo
            && IsValid(xCertificate, xSecurityEnvironment))
            return xCertificate;
    }
    return uno::Reference<security::XCertificate>();
}

CPPUNIT_TEST_FIXTURE(SigningTest, testDescription)
{
    // Create an empty document and store it to a tempfile, finally load it as a storage.
    load(u"private:factory/swriter"_ustr);

    save(u"writer8"_ustr);

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");

    // Then add a signature document.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
    if (!xCertificate.is())
        return;
    OUString aDescription(u"SigningTest::testDescription"_ustr);
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, aDescription, nSecurityId, false);

    // Read back the signature and make sure that the description survives the roundtrip.
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    CPPUNIT_ASSERT_EQUAL(aDescription, rInformations[0].ouDescription);
}

CPPUNIT_TEST_FIXTURE(SigningTest, testECDSA)
{
    // Create an empty document and store it to a tempfile, finally load it as a storage.
    load(u"private:factory/swriter"_ustr);

    save(u"writer8"_ustr);

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");

    // Then add a signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::ECDSA);
    if (!xCertificate.is())
        return;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, u""_ustr, nSecurityId, false);

    // Read back the signature and make sure that it's valid.
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    // This was SecurityOperationStatus_UNKNOWN, signing with an ECDSA key was
    // broken.
    CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                         rInformations[0].nStatus);
}

CPPUNIT_TEST_FIXTURE(SigningTest, testECDSAOOXML)
{
    // Create an empty document and store it to a tempfile, finally load it as a storage.
    load(u"private:factory/swriter"_ustr);

    save(u"MS Word 2007 XML"_ustr);

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");

    // Then add a document signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::ECDSA);
    if (!xCertificate.is())
        return;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, u""_ustr, nSecurityId,
                 /*bAdESCompliant=*/false);

    // Read back the signature and make sure that it's valid.
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    // This was SecurityOperationStatus_UNKNOWN, signing with an ECDSA key was
    // broken.
    CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                         rInformations[0].nStatus);
}

CPPUNIT_TEST_FIXTURE(SigningTest, testECDSAPDF)
{
    // Create an empty document and store it to a tempfile, finally load it as
    // a stream.
    load(u"private:factory/swriter"_ustr);

    save(u"writer_pdf_Export"_ustr);

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(
        maTempFile.GetURL(), StreamMode::READ | StreamMode::WRITE));
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
    CPPUNIT_ASSERT(xStream.is());
    aManager.setSignatureStream(xStream);

    // Then add a document signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::ECDSA);
    if (!xCertificate.is())
        return;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, u""_ustr, nSecurityId,
                 /*bAdESCompliant=*/true);

    // Read back the signature and make sure that it's valid.
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    // This was SecurityOperationStatus_UNKNOWN, signing with an ECDSA key was
    // broken.
    CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                         rInformations[0].nStatus);
}

CPPUNIT_TEST_FIXTURE(SigningTest, testOOXMLDescription)
{
    // Create an empty document and store it to a tempfile, finally load it as a storage.
    load(u"private:factory/swriter"_ustr);

    save(u"MS Word 2007 XML"_ustr);

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");

    // Then add a document signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
    if (!xCertificate.is())
        return;
    OUString aDescription(u"SigningTest::testDescription"_ustr);
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, aDescription, nSecurityId, false);

    // Read back the signature and make sure that the description survives the roundtrip.
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    CPPUNIT_ASSERT_EQUAL(aDescription, rInformations[0].ouDescription);
}

/// Test appending a new signature next to an existing one.
CPPUNIT_TEST_FIXTURE(SigningTest, testOOXMLAppend)
{
    // Copy the test document to a temporary file, as it'll be modified.
    createTempCopy(u"partial.docx");
    // Load the test document as a storage and read its single signature.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());

    // Then add a second document signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
    if (!xCertificate.is())
        return;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, OUString(), nSecurityId, false);

    // Read back the signatures and make sure that we have the expected amount.
    aManager.read(/*bUseTempStream=*/true);
    // This was 1: the original signature was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), rInformations.size());
}

/// Test removing a signature from existing ones.
CPPUNIT_TEST_FIXTURE(SigningTest, testOOXMLRemove)
{
    // Load the test document as a storage and read its signatures: purpose1 and purpose2.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    createTempCopy(u"multi.docx");
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), rInformations.size());

    // Then remove the last added signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
    if (!xCertificate.is())
        return;
    aManager.remove(0);

    // Read back the signatures and make sure that only purpose1 is left.
    aManager.read(/*bUseTempStream=*/true);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    CPPUNIT_ASSERT_EQUAL(u"purpose1"_ustr, rInformations[0].ouDescription);
}

/// Test removing all signatures from a document.
CPPUNIT_TEST_FIXTURE(SigningTest, testOOXMLRemoveAll)
{
    // Copy the test document to a temporary file, as it'll be modified.
    createTempCopy(u"partial.docx");
    // Load the test document as a storage and read its single signature.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());

    // Then remove the only signature in the document.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
    if (!xCertificate.is())
        return;
    aManager.remove(0);
    aManager.read(/*bUseTempStream=*/true);
    aManager.write(/*bXAdESCompliantIfODF=*/false);

    // Make sure that the signature count is zero and the whole signature storage is removed completely.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), rInformations.size());
    CPPUNIT_ASSERT(!xStorage->hasByName(u"_xmlsignatures"_ustr));

    // And that content types no longer contains signature types.
    uno::Reference<io::XStream> xStream
        = xStorage->openStreamElement(u"[Content_Types].xml"_ustr, embed::ElementModes::READWRITE);
    uno::Reference<io::XInputStream> xInputStream = xStream->getInputStream();
    uno::Sequence<uno::Sequence<beans::StringPair>> aContentTypeInfo
        = comphelper::OFOPXMLHelper::ReadContentTypeSequence(xInputStream, mxComponentContext);
    const uno::Sequence<beans::StringPair>& rOverrides = aContentTypeInfo[1];
    CPPUNIT_ASSERT(
        std::none_of(rOverrides.begin(), rOverrides.end(), [](const beans::StringPair& rPair) {
            return rPair.First.startsWith("/_xmlsignatures/sig");
        }));
}

/// Test a typical ODF where all streams are signed.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFGood)
{
    loadFromFile(u"good.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // We expect NOTVALIDATED in case the root CA is not imported on the system, and OK otherwise, so accept both.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::to_underlying(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
}

/// Test a typical broken ODF signature where one stream is corrupted.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFBroken)
{
    loadFromFile(u"bad.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

// Document has a signature stream, but no actual signatures.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFNo)
{
    loadFromFile(u"no.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::NOSIGNATURES),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

// document has one signed timestamp and one unsigned timestamp
CPPUNIT_TEST_FIXTURE(SigningTest, testODFUnsignedTimestamp)
{
    loadFromFile(u"02_doc_signed_by_trusted_person_manipulated.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::to_underlying(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    // was: 66666666
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20210126), infos[0].SignatureDate);
    // was: 0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18183742), infos[0].SignatureTime);
}

// FIXME: For some unknown reason, this test fails on tml's Mac unless it is the only or the first
// test that is run in this CppunitTest program. When using our patched bundled cppunit library (as
// we obviously always do on macOS), the CPPUNIT_TEST_FIXTUREs are run in lexicographical order so
// use a name for this test that makes it the first one to run.

CPPUNIT_TEST_FIXTURE(SigningTest, aaa_testODFX509CertificateChain)
{
    loadFromFile(u"signed_with_x509certificate_chain.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::to_underlying(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    // check that the signing certificate was picked, not one of the 2 CA ones
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::VALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(infos[0].Signer.is());
    CPPUNIT_ASSERT_EQUAL(
        u"CN=Xmlsecurity RSA Test example Alice,O=Xmlsecurity RSA Test,ST=England,C=UK"_ustr,
        // CryptoAPI puts a space after comma, NSS does not...
        infos[0].Signer->getSubjectName().replaceAll(", ", ","));
}

CPPUNIT_TEST_FIXTURE(SigningTest, testODFDoubleX509Data)
{
    loadFromFile(u"02_doc_signed_by_attacker_manipulated.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::to_underlying(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    // the signature in this manipulated document is technically valid but we can't tell who signed
    // it, so make sure no misleading info is shown to the user
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

CPPUNIT_TEST_FIXTURE(SigningTest, testODFTripleX509Data)
{
    loadFromFile(u"02_doc_signed_by_attacker_manipulated_triple.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    // here, libxmlsec will pick the 1st X509Data but signing key is the 2nd
    CPPUNIT_ASSERT_EQUAL_MESSAGE((OString::number(o3tl::to_underlying(nActual)).getStr()),
                                 SignatureState::BROKEN, nActual);
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    // the signature in this manipulated document is technically valid but we can't tell who signed
    // it, so make sure no misleading info is shown to the user
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

CPPUNIT_TEST_FIXTURE(SigningTest, testODFMacroDoubleX509Data)
{
    loadFromFile(u"02_doc_macros_signed_by_attacker_manipulated.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetScriptingSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::to_underlying(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    // the signature in this manipulated document is technically valid but we can't tell who signed
    // it, so make sure no misleading info is shown to the user
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

CPPUNIT_TEST_FIXTURE(SigningTest, testODFDoubleX509Certificate)
{
    loadFromFile(u"02_doc_signed_by_attacker_manipulated2.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    bool const nTemp((nActual == SignatureState::NOTVALIDATED
                      || nActual == SignatureState::OK
#if defined(_WIN32)
                      // oddly BCryptVerifySignature returns STATUS_INVALID_SIGNATURE
                      // while the same succeeds with NSS _SGN_VerifyPKCS1DigestInfo
                      || nActual == SignatureState::BROKEN
#endif
                      ));
    CPPUNIT_ASSERT_MESSAGE((OString::number(o3tl::to_underlying(nActual)).getStr()), nTemp);
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    // the signature in this manipulated document is technically valid but we can't tell who signed
    // it, so make sure no misleading info is shown to the user
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

CPPUNIT_TEST_FIXTURE(SigningTest, testDNCompatibility)
{
    OUString constexpr msDN(u"CN=\"\"\"ABC\"\".\", O=\"Enterprise \"\"ABC\"\"\""_ustr);
    OUString constexpr nssDN(u"CN=\\\"ABC\\\".,O=Enterprise \\\"ABC\\\""_ustr);
    // this is just the status quo, possibly either NSS or CryptoAPI might change
    CPPUNIT_ASSERT(!xmlsecurity::EqualDistinguishedNames(msDN, nssDN, xmlsecurity::NOCOMPAT));
    CPPUNIT_ASSERT(!xmlsecurity::EqualDistinguishedNames(nssDN, msDN, xmlsecurity::NOCOMPAT));
    // with compat flag it should work, with the string one 2nd and the native one 1st
#ifdef _WIN32
    CPPUNIT_ASSERT(xmlsecurity::EqualDistinguishedNames(msDN, nssDN, xmlsecurity::COMPAT_2ND));
#else
    CPPUNIT_ASSERT(xmlsecurity::EqualDistinguishedNames(nssDN, msDN, xmlsecurity::COMPAT_2ND));
#endif
}

/// Test a typical OOXML where a number of (but not all) streams are signed.
CPPUNIT_TEST_FIXTURE(SigningTest, testOOXMLPartial)
{
    loadFromFile(u"partial.docx");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // This was SignatureState::BROKEN due to missing RelationshipTransform and SHA-256 support.
    // We expect NOTVALIDATED_PARTIAL_OK in case the root CA is not imported on the system, and PARTIAL_OK otherwise, so accept both.
    // But reject NOTVALIDATED, hiding incompleteness is not OK.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE((OString::number(o3tl::to_underlying(nActual)).getStr()),
                           (nActual == SignatureState::NOTVALIDATED_PARTIAL_OK
                            || nActual == SignatureState::PARTIAL_OK));
}

/// Test a typical broken OOXML signature where one stream is corrupted.
CPPUNIT_TEST_FIXTURE(SigningTest, testOOXMLBroken)
{
    loadFromFile(u"bad.docx");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // This was SignatureState::NOTVALIDATED/PARTIAL_OK as we did not validate manifest references.
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

#if HAVE_FEATURE_PDFIMPORT

/// Test a typical PDF where the signature is good.
CPPUNIT_TEST_FIXTURE(SigningTest, testPDFGood)
{
    loadFromFile(u"good.pdf");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // We expect NOTVALIDATED in case the root CA is not imported on the system, and OK otherwise, so accept both.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }

    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::to_underlying(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
}

/// Test a typical PDF where the signature is bad.
CPPUNIT_TEST_FIXTURE(SigningTest, testPDFBad)
{
    loadFromFile(u"bad.pdf");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

CPPUNIT_TEST_FIXTURE(SigningTest, testPDFHideAndReplace)
{
    loadFromFile(u"hide-and-replace-shadow-file-signed-2.pdf");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2 (BROKEN)
    // - Actual  : 6 (NOTVALIDATED_PARTIAL_OK)
    // i.e. a non-commenting update after a signature was not marked as invalid.
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

/// Test a typical PDF which is not signed.
CPPUNIT_TEST_FIXTURE(SigningTest, testPDFNo)
{
    loadFromFile(u"no.pdf");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::NOSIGNATURES),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

#endif

CPPUNIT_TEST_FIXTURE(SigningTest, testPDFAddVisibleSignature)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }

    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;
    // Given: copy the test document to a temporary file, as it'll be modified.
    createTempCopy(u"add-visible-signature.pdf");

    // Open it.
    uno::Sequence<beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue(u"ReadOnly"_ustr, true) };
    mxComponent
        = loadFromDesktop(maTempFile.GetURL(), u"com.sun.star.drawing.DrawingDocument"_ustr, aArgs);
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);

    // Add a signature line.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.GraphicObjectShape"_ustr), uno::UNO_QUERY);
    xShape->setPosition(awt::Point(1000, 15000));
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xSupplier->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    xDrawPage->add(xShape);

    // Select it and assign a certificate.
    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(pBaseModel->getCurrentController(),
                                                                uno::UNO_QUERY);
    xSelectionSupplier->select(uno::Any(xShape));
    uno::Sequence<uno::Reference<security::XCertificate>> aCertificates
        = mxSecurityContext->getSecurityEnvironment()->getPersonalCertificates();
    if (!aCertificates.hasElements())
    {
        return;
    }
    SfxViewShell* pCurrent = SfxViewShell::Current();
    CPPUNIT_ASSERT(pCurrent);
    SdrView* pView = pCurrent->GetDrawView();
    svx::SignatureLineHelper::setShapeCertificate(pView, aCertificates[0]);

    // the document is modified now, but Sign function can't show SaveAs dialog
    // in unit test, so just clear the modified
    pObjectShell->SetModified(false);

    // When: do the actual signing.
    pObjectShell->SignDocumentContentUsingCertificate(aCertificates[0]);

    // Then: count the # of shapes on the signature widget/annotation.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnot = pPdfPage->getAnnotation(/*nIndex=*/0);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 0
    // i.e. the signature was there, but it was empty / not visible.
    CPPUNIT_ASSERT_EQUAL(4, pAnnot->getObjectCount());
}

CPPUNIT_TEST_FIXTURE(SigningTest, test96097Calc)
{
    loadFromFile(u"tdf96097.ods");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT_MESSAGE("Failed to access document base model", pBaseModel);

    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pObjectShell);

    SignatureState nActual = pObjectShell->GetScriptingSignatureState();
    CPPUNIT_ASSERT_MESSAGE((OString::number(o3tl::to_underlying(nActual)).getStr()),
                           (nActual == SignatureState::OK || nActual == SignatureState::NOTVALIDATED
                            || nActual == SignatureState::INVALID));

    uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY_THROW);

    // Save a copy
    uno::Sequence<beans::PropertyValue> descSaveACopy(comphelper::InitPropertySequence(
        { { "SaveACopy", uno::Any(true) }, { "FilterName", uno::Any(u"calc8"_ustr) } }));
    xDocStorable->storeToURL(maTempFile.GetURL(), descSaveACopy);

    // FIXME: Error: element "document-signatures" is missing "version" attribute
    skipValidation();

    // Save As
    save(u"calc8"_ustr);
}

CPPUNIT_TEST_FIXTURE(SigningTest, test96097Doc)
{
    loadFromFile(u"tdf96097.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);

    SignatureState nActual = pObjectShell->GetScriptingSignatureState();
    CPPUNIT_ASSERT_MESSAGE((OString::number(o3tl::to_underlying(nActual)).getStr()),
                           (nActual == SignatureState::OK || nActual == SignatureState::NOTVALIDATED
                            || nActual == SignatureState::INVALID));

    uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY_THROW);

    // Save a copy
    uno::Sequence<beans::PropertyValue> descSaveACopy(comphelper::InitPropertySequence(
        { { "SaveACopy", uno::Any(true) }, { "FilterName", uno::Any(u"writer8"_ustr) } }));
    xDocStorable->storeToURL(maTempFile.GetURL(), descSaveACopy);

    // FIXME: Error: element "document-signatures" is missing "version" attribute
    skipValidation();

    // Save As
    save(u"writer8"_ustr);
}

CPPUNIT_TEST_FIXTURE(SigningTest, testXAdESNotype)
{
    // Create a working copy.
    createTempCopy(u"notype-xades.odt");

    // Read existing signature.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");
    aManager.read(/*bUseTempStream=*/false);

    // Create a new signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
    if (!xCertificate.is())
        return;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, /*rDescription=*/OUString(), nSecurityId,
                 /*bAdESCompliant=*/true);

    // Write to storage.
    aManager.read(/*bUseTempStream=*/true);
    aManager.write(/*bXAdESCompliantIfODF=*/true);
    uno::Reference<embed::XTransactedObject> xTransactedObject(xStorage, uno::UNO_QUERY);
    xTransactedObject->commit();

    // Parse the resulting XML.
    uno::Reference<embed::XStorage> xMetaInf
        = xStorage->openStorageElement(u"META-INF"_ustr, embed::ElementModes::READ);
    uno::Reference<io::XInputStream> xInputStream(
        xMetaInf->openStreamElement(u"documentsignatures.xml"_ustr, embed::ElementModes::READ),
        uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // Without the accompanying fix in place, this test would have failed with "unexpected 'Type'
    // attribute", i.e. the signature without such an attribute was not preserved correctly.
    assertXPathNoAttribute(pXmlDoc,
                           "/odfds:document-signatures/dsig:Signature[1]/dsig:SignedInfo/"
                           "dsig:Reference[starts-with(@URI, '#idSignedProperties')]"_ostr,
                           "Type"_ostr);

    // New signature always has the Type attribute.
    assertXPath(pXmlDoc,
                "/odfds:document-signatures/dsig:Signature[2]/dsig:SignedInfo/"
                "dsig:Reference[starts-with(@URI, '#idSignedProperties')]"_ostr,
                "Type"_ostr, u"http://uri.etsi.org/01903#SignedProperties"_ustr);
}

/// Creates a XAdES signature from scratch.
CPPUNIT_TEST_FIXTURE(SigningTest, testXAdES)
{
    // Create an empty document, store it to a tempfile and load it as a storage.
    load(u"private:factory/swriter"_ustr);

    save(u"writer8"_ustr);

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");

    // Create a signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
    if (!xCertificate.is())
        return;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, /*rDescription=*/OUString(), nSecurityId,
                 /*bAdESCompliant=*/true);

    // Write to storage.
    aManager.read(/*bUseTempStream=*/true);
    aManager.write(/*bXAdESCompliantIfODF=*/true);
    uno::Reference<embed::XTransactedObject> xTransactedObject(xStorage, uno::UNO_QUERY);
    xTransactedObject->commit();

    // Parse the resulting XML.
    uno::Reference<embed::XStorage> xMetaInf
        = xStorage->openStorageElement(u"META-INF"_ustr, embed::ElementModes::READ);
    uno::Reference<io::XInputStream> xInputStream(
        xMetaInf->openStreamElement(u"documentsignatures.xml"_ustr, embed::ElementModes::READ),
        uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // Assert that the digest algorithm is SHA-256 in the bAdESCompliant case, not SHA-1.
    assertXPath(pXmlDoc,
                "/odfds:document-signatures/dsig:Signature/dsig:SignedInfo/"
                "dsig:Reference[@URI='content.xml']/dsig:DigestMethod"_ostr,
                "Algorithm"_ostr, ALGO_XMLDSIGSHA256);

    // Assert that the digest of the signing certificate is included.
    assertXPath(pXmlDoc, "//xd:CertDigest"_ostr, 1);

    // Assert that the Type attribute is set on all URI's that start with #idSignedProperties
    assertXPath(pXmlDoc, "//dsig:Reference[starts-with(@URI, '#idSignedProperties')]"_ostr,
                "Type"_ostr, u"http://uri.etsi.org/01903#SignedProperties"_ustr);
}

CPPUNIT_TEST_FIXTURE(SigningTest, testSigningMultipleTimes_ODT)
{
    load(u"private:factory/swriter"_ustr);

    save(u"writer8"_ustr);
    {
        DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
        CPPUNIT_ASSERT(aManager.init());
        uno::Reference<embed::XStorage> xStorage
            = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
                ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
        CPPUNIT_ASSERT(xStorage.is());
        aManager.setStore(xStorage);
        aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");

        // Create a signature.
        uno::Reference<security::XCertificate> xCertificate
            = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);

        if (!xCertificate.is())
            return;
        sal_Int32 nSecurityId;
        aManager.add(xCertificate, mxSecurityContext, /*rDescription=*/OUString(), nSecurityId,
                     /*bAdESCompliant=*/true);

        // Read back the signature and make sure that it's valid.
        aManager.read(/*bUseTempStream=*/true);
        {
            std::vector<SignatureInformation>& rInformations
                = aManager.getCurrentSignatureInformations();
            CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
            CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                                 rInformations[0].nStatus);
        }

        aManager.add(xCertificate, mxSecurityContext, /*rDescription=*/OUString(), nSecurityId,
                     /*bAdESCompliant=*/true);
        aManager.read(/*bUseTempStream=*/true);
        {
            std::vector<SignatureInformation>& rInformations
                = aManager.getCurrentSignatureInformations();
            CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), rInformations.size());
            CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                                 rInformations[1].nStatus);
        }

        aManager.add(xCertificate, mxSecurityContext, /*rDescription=*/OUString(), nSecurityId,
                     /*bAdESCompliant=*/true);
        aManager.read(/*bUseTempStream=*/true);
        {
            std::vector<SignatureInformation>& rInformations
                = aManager.getCurrentSignatureInformations();
            CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), rInformations.size());
            CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                                 rInformations[2].nStatus);
        }

        aManager.write(/*bXAdESCompliantIfODF=*/true);
        uno::Reference<embed::XTransactedObject> xTransactedObject(xStorage, uno::UNO_QUERY);
        xTransactedObject->commit();
    }

    Scheduler::ProcessEventsToIdle();

    load(maTempFile.GetURL());

    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(SignatureState::OK, pObjectShell->GetDocumentSignatureState());
}

CPPUNIT_TEST_FIXTURE(SigningTest, testSigningMultipleTimes_OOXML)
{
    load(u"private:factory/swriter"_ustr);

    save(u"MS Word 2007 XML"_ustr);
    {
        DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
        CPPUNIT_ASSERT(aManager.init());
        uno::Reference<embed::XStorage> xStorage
            = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
                ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
        CPPUNIT_ASSERT(xStorage.is());
        aManager.setStore(xStorage);
        aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");

        // Create a signature.
        uno::Reference<security::XCertificate> xCertificate
            = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::ECDSA);
        if (!xCertificate.is())
            return;

        sal_Int32 nSecurityId;
        aManager.add(xCertificate, mxSecurityContext, u""_ustr, nSecurityId,
                     /*bAdESCompliant=*/false);
        aManager.read(/*bUseTempStream=*/true);
        {
            std::vector<SignatureInformation>& rInformations
                = aManager.getCurrentSignatureInformations();
            CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
            CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                                 rInformations[0].nStatus);
        }

        aManager.add(xCertificate, mxSecurityContext, u""_ustr, nSecurityId,
                     /*bAdESCompliant=*/false);
        aManager.read(/*bUseTempStream=*/true);
        {
            std::vector<SignatureInformation>& rInformations
                = aManager.getCurrentSignatureInformations();
            CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), rInformations.size());
            CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                                 rInformations[1].nStatus);
        }

        aManager.add(xCertificate, mxSecurityContext, u""_ustr, nSecurityId,
                     /*bAdESCompliant=*/false);
        aManager.read(/*bUseTempStream=*/true);
        {
            std::vector<SignatureInformation>& rInformations
                = aManager.getCurrentSignatureInformations();
            CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), rInformations.size());
            CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                                 rInformations[2].nStatus);
        }

        aManager.write(/*bXAdESCompliantIfODF=*/true);
        uno::Reference<embed::XTransactedObject> xTransactedObject(xStorage, uno::UNO_QUERY);
        xTransactedObject->commit();
    }

    Scheduler::ProcessEventsToIdle();

    load(maTempFile.GetURL());

    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(SignatureState::PARTIAL_OK, pObjectShell->GetDocumentSignatureState());
}

/// Works with an existing good XAdES signature.
CPPUNIT_TEST_FIXTURE(SigningTest, testXAdESGood)
{
    loadFromFile(u"good-xades.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // We expect NOTVALIDATED in case the root CA is not imported on the system, and OK otherwise, so accept both.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::to_underlying(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
}

/// Test importing of signature line
CPPUNIT_TEST_FIXTURE(SigningTest, testSignatureLineOOXML)
{
    // Given: A document (docx) with a signature line and a valid signature
    uno::Reference<security::XDocumentDigitalSignatures> xSignatures(
        security::DocumentDigitalSignatures::createDefault(
            comphelper::getProcessComponentContext()));

    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, createFileURL(u"signatureline.docx"),
            embed::ElementModes::READ);
    CPPUNIT_ASSERT(xStorage.is());

    uno::Sequence<security::DocumentSignatureInformation> xSignatureInfo
        = xSignatures->verifyScriptingContentSignatures(xStorage,
                                                        uno::Reference<io::XInputStream>());

    CPPUNIT_ASSERT(xSignatureInfo.getLength());

    // The signature should have a valid signature, and signature line with two valid images
    CPPUNIT_ASSERT(xSignatureInfo[0].SignatureIsValid);
    CPPUNIT_ASSERT_EQUAL(u"{DEE0514B-13E8-4674-A831-46E3CDB18BB4}"_ustr,
                         xSignatureInfo[0].SignatureLineId);
    CPPUNIT_ASSERT(xSignatureInfo[0].ValidSignatureLineImage.is());
    CPPUNIT_ASSERT(xSignatureInfo[0].InvalidSignatureLineImage.is());
}

CPPUNIT_TEST_FIXTURE(SigningTest, testSignatureLineODF)
{
    loadFromFile(u"signatureline.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);

    uno::Sequence<security::DocumentSignatureInformation> xSignatureInfo
        = pObjectShell->GetDocumentSignatureInformation(false);

    CPPUNIT_ASSERT(xSignatureInfo.getLength());

    CPPUNIT_ASSERT(xSignatureInfo[0].SignatureIsValid);
    CPPUNIT_ASSERT_EQUAL(u"{41CF56EE-331B-4125-97D8-2F5669DD3AAC}"_ustr,
                         xSignatureInfo[0].SignatureLineId);
    CPPUNIT_ASSERT(xSignatureInfo[0].ValidSignatureLineImage.is());
    CPPUNIT_ASSERT(xSignatureInfo[0].InvalidSignatureLineImage.is());
}

#if HAVE_FEATURE_GPGVERIFY
/// Test a typical ODF where all streams are GPG-signed.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFGoodGPG)
{
    loadFromFile(u"goodGPG.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // Our local gpg config fully trusts the signing cert, so in
    // contrast to the X509 test we can fail on NOTVALIDATED here
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_EQUAL_MESSAGE((OString::number(o3tl::to_underlying(nActual)).getStr()),
                                 SignatureState::OK, nActual);
}

/// Test a typical ODF where all streams are GPG-signed, but we don't trust the signature.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFUntrustedGoodGPG)
{
    loadFromFile(u"untrustedGoodGPG.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // Our local gpg config does _not_ trust the signing cert, so in
    // contrast to the X509 test we can fail everything but
    // NOTVALIDATED here
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_EQUAL_MESSAGE((OString::number(o3tl::to_underlying(nActual)).getStr()),
                                 SignatureState::NOTVALIDATED, nActual);
}

/// Test a typical broken ODF signature where one stream is corrupted.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFBrokenStreamGPG)
{
    loadFromFile(u"badStreamGPG.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

/// Test a typical broken ODF signature where the XML dsig hash is corrupted.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFBrokenDsigGPG)
{
    loadFromFile(u"badDsigGPG.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

#if HAVE_GPGCONF_SOCKETDIR

/// Test loading an encrypted ODF document
CPPUNIT_TEST_FIXTURE(SigningTest, testODFEncryptedGPG)
{
    // ODF1.2 + loext flavour
    loadFromFile(u"encryptedGPG.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);

    // ODF1.3 flavour
    loadFromFile(u"encryptedGPG_odf13.odt");
    pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);

    // export and import again
    saveAndReload(u"writer8"_ustr);

    pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
}

#endif

SfxObjectShell* SigningTest::assertDocument(const ::CppUnit::SourceLine aSrcLine,
                                            const OUString& rFilterName,
                                            const SignatureState nDocSign,
                                            const SignatureState nMacroSign,
                                            const OUString& sVersion)
{
    std::string sPos = aSrcLine.fileName() + ":" + OString::number(aSrcLine.lineNumber()).getStr();

    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT_MESSAGE(sPos, pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT_MESSAGE(sPos, pObjectShell);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(sPos, rFilterName,
                                 pObjectShell->GetMedium()->GetFilter()->GetFilterName());
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sPos, nDocSign, nActual);
    nActual = pObjectShell->GetScriptingSignatureState();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sPos, nMacroSign, nActual);

    OUString aODFVersion;
    uno::Reference<beans::XPropertySet> xPropSet(pObjectShell->GetStorage(), uno::UNO_QUERY_THROW);
    xPropSet->getPropertyValue(u"Version"_ustr) >>= aODFVersion;
    CPPUNIT_ASSERT_EQUAL(sVersion, aODFVersion);

    return pObjectShell;
}

/// Test if a macro signature from a OTT 1.2 template is preserved for ODT 1.2
CPPUNIT_TEST_FIXTURE(SigningTest, testPreserveMacroTemplateSignature12_ODF)
{
    const OUString aFormats[] = { u"writer8"_ustr, u"writer8_template"_ustr };

    for (OUString const& sFormat : aFormats)
    {
        const OUString aURL(createFileURL(u"tdf42316_odt12.ott"));
        const OUString sLoadMessage = "loading failed: " + aURL;

        // load the template as-is to validate signatures
        mxComponent = loadFromDesktop(
            aURL, OUString(),
            comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));

        // we are a template, and have a valid document and macro signature
        assertDocument(CPPUNIT_SOURCELINE(), u"writer8_template"_ustr, SignatureState::OK,
                       SignatureState::OK, ODFVER_012_TEXT);

        // create new document from template
        load(aURL);
        CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                               mxComponent.is());

        // we are somehow a template (?), and have just a valid macro signature
        assertDocument(CPPUNIT_SOURCELINE(), u"writer8_template"_ustr, SignatureState::NOSIGNATURES,
                       SignatureState::OK, ODFVER_012_TEXT);

        // FIXME: Error: element "document-signatures" is missing "version" attribute
        skipValidation();

        if (sFormat == "writer8")
            // save as new ODT document
            saveAndReload(sFormat);
        else
        {
            // save as new OTT template
            save(u"writer8_template"_ustr);

            // load the saved OTT template as-is to validate signatures
            mxComponent->dispose();
            mxComponent = loadFromDesktop(
                maTempFile.GetURL(), OUString(),
                comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));
        }

        // the loaded document is a OTT/ODT with a macro signature
        assertDocument(CPPUNIT_SOURCELINE(), sFormat, SignatureState::NOSIGNATURES,
                       SignatureState::OK, ODFVER_013_TEXT);

        // save as new OTT template
        save(u"writer8_template"_ustr);

        // load the template as-is to validate signatures
        mxComponent->dispose();
        mxComponent = loadFromDesktop(
            maTempFile.GetURL(), OUString(),
            comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));

        // the loaded document is a OTT with a valid macro signature
        assertDocument(CPPUNIT_SOURCELINE(), u"writer8_template"_ustr, SignatureState::NOSIGNATURES,
                       SignatureState::OK, ODFVER_013_TEXT);

        mxComponent->dispose();
        mxComponent.clear();
    }
}

/// Test if a macro signature from an OTT 1.0 is dropped for ODT 1.2
CPPUNIT_TEST_FIXTURE(SigningTest, testDropMacroTemplateSignature)
{
    const OUString aURL(createFileURL(u"tdf42316.ott"));
    const OUString sLoadMessage = "loading failed: " + aURL;

    // load the template as-is to validate signatures
    mxComponent = loadFromDesktop(
        aURL, OUString(), comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));

    // we are a template, and have a non-invalid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), u"writer8_template"_ustr, SignatureState::NOSIGNATURES,
                   SignatureState::NOTVALIDATED, OUString());

    // create new document from template
    load(aURL);
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // we are somehow a template (?), and have just a valid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), u"writer8_template"_ustr, SignatureState::NOSIGNATURES,
                   SignatureState::NOTVALIDATED, OUString());

    // save as new ODT document
    saveAndReload(u"writer8"_ustr);

    // the loaded document is a 1.2 ODT without any signatures
    assertDocument(CPPUNIT_SOURCELINE(), u"writer8"_ustr, SignatureState::NOSIGNATURES,
                   SignatureState::NOSIGNATURES, ODFVER_013_TEXT);

    // load the template as-is to validate signatures
    mxComponent->dispose();
    mxComponent = loadFromDesktop(
        aURL, OUString(), comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));

    // we are a template, and have a non-invalid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), u"writer8_template"_ustr, SignatureState::NOSIGNATURES,
                   SignatureState::NOTVALIDATED, OUString());

    // save as new OTT template
    save(u"writer8_template"_ustr);

    // load the template as-is to validate signatures
    mxComponent->dispose();
    mxComponent
        = loadFromDesktop(maTempFile.GetURL(), OUString(),
                          comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));

    // the loaded document is a 1.2 OTT without any signatures
    assertDocument(CPPUNIT_SOURCELINE(), u"writer8_template"_ustr, SignatureState::NOSIGNATURES,
                   SignatureState::NOSIGNATURES, ODFVER_013_TEXT);
}

/// Test if a macro signature from a OTT 1.0 template is preserved for ODT 1.0
CPPUNIT_TEST_FIXTURE(SigningTest, testPreserveMacroTemplateSignature10)
{
    // set ODF version 1.0 / 1.1 as default
    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        return pBatch->commit();
    });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Save::ODF::DefaultVersion::set(2, pBatch);
    pBatch->commit();

    const OUString aFormats[] = { u"writer8"_ustr, u"writer8_template"_ustr };

    for (OUString const& sFormat : aFormats)
    {
        const OUString aURL(createFileURL(u"tdf42316.ott"));
        const OUString sLoadMessage = "loading failed: " + aURL;

        // load the template as-is to validate signatures
        mxComponent = loadFromDesktop(
            aURL, OUString(),
            comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));

        // we are a template, and have a non-invalid macro signature
        assertDocument(CPPUNIT_SOURCELINE(), u"writer8_template"_ustr, SignatureState::NOSIGNATURES,
                       SignatureState::NOTVALIDATED, OUString());

        // create new document from template
        load(aURL);
        CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                               mxComponent.is());

        // we are somehow a template (?), and have just a valid macro signature
        assertDocument(CPPUNIT_SOURCELINE(), u"writer8_template"_ustr, SignatureState::NOSIGNATURES,
                       SignatureState::NOTVALIDATED, OUString());

        // FIXME: Error: element "manifest:manifest" is missing "version" attribute
        skipValidation();

        if (sFormat == "writer8")
            // save as new ODT document
            saveAndReload(sFormat);
        else
        {
            // save as new OTT template
            save(u"writer8_template"_ustr);

            // load the saved OTT template as-is to validate signatures
            mxComponent->dispose();
            mxComponent = loadFromDesktop(
                maTempFile.GetURL(), OUString(),
                comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));
        }

        assertDocument(CPPUNIT_SOURCELINE(), sFormat, SignatureState::NOSIGNATURES,
                       SignatureState::NOTVALIDATED, OUString());

        save(u"writer8_template"_ustr);

        // load the template as-is to validate signatures
        mxComponent->dispose();
        mxComponent = loadFromDesktop(
            maTempFile.GetURL(), OUString(),
            comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));

        // the loaded document is a OTT with a non-invalid macro signature
        assertDocument(CPPUNIT_SOURCELINE(), u"writer8_template"_ustr, SignatureState::NOSIGNATURES,
                       SignatureState::NOTVALIDATED, OUString());

        mxComponent->dispose();
        mxComponent.clear();
    }
}

#endif

void SigningTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("odfds"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:digitalsignature:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dsig"),
                       BAD_CAST("http://www.w3.org/2000/09/xmldsig#"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xd"), BAD_CAST("http://uri.etsi.org/01903/v1.3.2#"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
