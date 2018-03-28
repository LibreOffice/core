/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <sal/config.h>

#include <type_traits>

#ifndef _WIN32
#include <secoid.h>
#endif

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/BrokenPackageRequest.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/packages/manifest/ManifestReader.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>

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
#include <osl/file.hxx>
#include <comphelper/ofopxmlhelper.hxx>

#include <documentsignaturehelper.hxx>
#include <xmlsignaturehelper.hxx>
#include <documentsignaturemanager.hxx>
#include <biginteger.hxx>
#include <certificate.hxx>
#include <xsecctl.hxx>
#include <ucbhelper/interceptedinteraction.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <comphelper/configuration.hxx>

using namespace com::sun::star;

namespace
{
const char* const DATA_DIRECTORY = "/xmlsecurity/qa/unit/signing/data/";
}

/// Testsuite for the document signing feature.
class SigningTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;
    uno::Reference<xml::crypto::XSEInitializer> mxSEInitializer;
    uno::Reference<xml::crypto::XXMLSecurityContext> mxSecurityContext;

public:
    SigningTest();
    virtual void setUp() override;
    virtual void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;

    void testDescription();
    /// Test a typical ODF where all streams are signed.
    void testODFGood();
    /// Test a typical broken ODF signature where one stream is corrupted.
    void testODFBroken();
    /// Document has a signature stream, but no actual signatures.
    void testODFNo();
    void testODFUnsignedTimestamp();
    void testODFX509CertificateChain();
    void testODFDoubleX509Data();
    void testODFTripleX509Data();
    void testODFMacroDoubleX509Data();
    void testODFDoubleX509Certificate();
    void testDNCompatibility();
    /// Test a typical OOXML where a number of (but not all) streams are signed.
    void testOOXMLPartial();
    /// Test a typical broken OOXML signature where one stream is corrupted.
    void testOOXMLBroken();
    void testOOXMLDescription();
    /// Test appending a new signature next to an existing one.
    void testOOXMLAppend();
    /// Test removing a signature from existing ones.
    void testOOXMLRemove();
    /// Test removing all signatures from a document.
    void testOOXMLRemoveAll();
#if HAVE_FEATURE_PDFIMPORT
    /// Test a typical PDF where the signature is good.
    void testPDFGood();
    /// Test a typical PDF where the signature is bad.
    void testPDFBad();
    /// Test a maliciously manipulated signed pdf
    void testPDFHideAndReplace();
    /// Test a typical PDF which is not signed.
    void testPDFNo();
#endif
    void test96097Calc();
    void test96097Doc();
    void testXAdESNotype();
    /// Creates a XAdES signature from scratch.
    void testXAdES();
    /// Works with an existing good XAdES signature.
    void testXAdESGood();
    void testInvalidZIP();

    CPPUNIT_TEST_SUITE(SigningTest);
    CPPUNIT_TEST(testDescription);
    CPPUNIT_TEST(testODFGood);
    CPPUNIT_TEST(testODFBroken);
    CPPUNIT_TEST(testODFNo);
    CPPUNIT_TEST(testODFBroken);
    CPPUNIT_TEST(testODFUnsignedTimestamp);
    CPPUNIT_TEST(testODFX509CertificateChain);
    CPPUNIT_TEST(testODFDoubleX509Data);
    CPPUNIT_TEST(testODFTripleX509Data);
    CPPUNIT_TEST(testODFMacroDoubleX509Data);
    CPPUNIT_TEST(testODFDoubleX509Certificate);
    CPPUNIT_TEST(testDNCompatibility);
    CPPUNIT_TEST(testOOXMLPartial);
    CPPUNIT_TEST(testOOXMLBroken);
    CPPUNIT_TEST(testOOXMLDescription);
    CPPUNIT_TEST(testOOXMLAppend);
    CPPUNIT_TEST(testOOXMLRemove);
    CPPUNIT_TEST(testOOXMLRemoveAll);
#if HAVE_FEATURE_PDFIMPORT
    CPPUNIT_TEST(testPDFGood);
    CPPUNIT_TEST(testPDFBad);
    CPPUNIT_TEST(testPDFHideAndReplace);
    CPPUNIT_TEST(testPDFNo);
#endif
    CPPUNIT_TEST(test96097Calc);
    CPPUNIT_TEST(test96097Doc);
    CPPUNIT_TEST(testXAdESNotype);
    CPPUNIT_TEST(testXAdES);
    CPPUNIT_TEST(testXAdESGood);
    CPPUNIT_TEST(testInvalidZIP);
    CPPUNIT_TEST_SUITE_END();

private:
    void createDoc(const OUString& rURL);
    void createCalc(const OUString& rURL);
    uno::Reference<security::XCertificate> getCertificate(DocumentSignatureManager& rSignatureManager);
};

SigningTest::SigningTest()
{
}

void SigningTest::setUp()
{
    test::BootstrapFixture::setUp();

#ifndef _WIN32
    // Set up cert8.db in workdir/CppunitTest/
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aTargetDir = m_directories.getURLFromWorkdir(
                              "/CppunitTest/xmlsecurity_signing.test.user/");

    // Set up NSS database in workdir/CppunitTest/
    osl::File::copy(aSourceDir + "cert9.db", aTargetDir + "cert9.db");
    osl::File::copy(aSourceDir + "key4.db", aTargetDir + "key4.db");
    osl::File::copy(aSourceDir + "pkcs11.txt", aTargetDir + "pkcs11.txt");

    OUString aTargetPath;
    osl::FileBase::getSystemPathFromFileURL(aTargetDir, aTargetPath);
    setenv("MOZILLA_CERTIFICATE_FOLDER", aTargetPath.toUtf8().getStr(), 1);
#endif

#ifdef _WIN32
    // CryptoAPI test certificates
    // FIXME for some reason aSourceDir and aTargetDir are undeclared
    //osl::File::copy(aSourceDir + "test.p7b", aTargetDir + "test.p7b");
    //OUString caVar("LIBO_TEST_CRYPTOAPI_PKCS7");
    //osl_setEnvironment(caVar.pData, aTargetPath.pData);
#endif

    // Initialize crypto after setting up the environment variables.
    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
    mxSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    mxSecurityContext = mxSEInitializer->createSecurityContext(OUString());
#ifndef _WIN32
#ifdef NSS_USE_ALG_IN_ANY_SIGNATURE
    // policy may disallow using SHA1 for signatures but unit test documents
    // have such existing signatures (call this after createSecurityContext!)
    NSS_SetAlgorithmPolicy(SEC_OID_SHA1, NSS_USE_ALG_IN_ANY_SIGNATURE, 0);
#endif
#endif
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

void SigningTest::createCalc(const OUString& rURL)
{
    if (mxComponent.is())
        mxComponent->dispose();
    if (rURL.isEmpty())
        mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.sheet.SpreadsheetDocument");
    else
        mxComponent = loadFromDesktop(rURL, "com.sun.star.sheet.SpreadsheetDocument");
}

uno::Reference<security::XCertificate> SigningTest::getCertificate(DocumentSignatureManager& rSignatureManager)
{
    uno::Reference<security::XCertificate> xCertificate;

    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = rSignatureManager.getSecurityEnvironment();
    uno::Sequence<uno::Reference<security::XCertificate>> aCertificates = xSecurityEnvironment->getPersonalCertificates();
    if (!aCertificates.hasElements())
        return xCertificate;

    return aCertificates[0];
}

void SigningTest::testDescription()
{
    // Create an empty document and store it to a tempfile, finally load it as a storage.
    createDoc("");

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer8");
    xStorable->storeAsURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");

    // Then add a signature document.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager);
    if (!xCertificate.is())
        return;
    OUString aDescription("SigningTest::testDescription");
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, aDescription, nSecurityId, false);

    // Read back the signature and make sure that the description survives the roundtrip.
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    CPPUNIT_ASSERT_EQUAL(aDescription, rInformations[0].ouDescription);
}

void SigningTest::testOOXMLDescription()
{
    // Create an empty document and store it to a tempfile, finally load it as a storage.
    createDoc("");

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("MS Word 2007 XML");
    xStorable->storeAsURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");

    // Then add a document signature.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager);
    if (!xCertificate.is())
        return;
    OUString aDescription("SigningTest::testDescription");
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, aDescription, nSecurityId, false);

    // Read back the signature and make sure that the description survives the roundtrip.
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    CPPUNIT_ASSERT_EQUAL(aDescription, rInformations[0].ouDescription);
}

void SigningTest::testOOXMLAppend()
{
    // Copy the test document to a temporary file, as it'll be modified.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    OUString aURL = aTempFile.GetURL();
    CPPUNIT_ASSERT_EQUAL(osl::File::RC::E_None,
                         osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY) + "partial.docx", aURL));
    // Load the test document as a storage and read its single signature.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL, embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());

    // Then add a second document signature.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager);
    if (!xCertificate.is())
        return;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, OUString(), nSecurityId, false);

    // Read back the signatures and make sure that we have the expected amount.
    aManager.read(/*bUseTempStream=*/true);
    // This was 1: the original signature was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), rInformations.size());
}

void SigningTest::testOOXMLRemove()
{
    // Load the test document as a storage and read its signatures: purpose1 and purpose2.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    OUString aURL = aTempFile.GetURL();
    CPPUNIT_ASSERT_EQUAL(
        osl::File::RC::E_None,
        osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY) + "multi.docx", aURL));
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL, embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), rInformations.size());

    // Then remove the last added signature.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager);
    if (!xCertificate.is())
        return;
    aManager.remove(0);

    // Read back the signatures and make sure that only purpose1 is left.
    aManager.read(/*bUseTempStream=*/true);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    CPPUNIT_ASSERT_EQUAL(OUString("purpose1"), rInformations[0].ouDescription);
}

void SigningTest::testOOXMLRemoveAll()
{
    // Copy the test document to a temporary file, as it'll be modified.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    OUString aURL = aTempFile.GetURL();
    CPPUNIT_ASSERT_EQUAL(osl::File::RC::E_None,
                         osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY) + "partial.docx", aURL));
    // Load the test document as a storage and read its single signature.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL, embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());

    // Then remove the only signature in the document.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager);
    if (!xCertificate.is())
        return;
    aManager.remove(0);
    aManager.read(/*bUseTempStream=*/true);
    aManager.write(/*bXAdESCompliantIfODF=*/false);

    // Make sure that the signature count is zero and the whole signature storage is removed completely.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), rInformations.size());
    uno::Reference<container::XNameAccess> xNameAccess(xStorage, uno::UNO_QUERY);
    CPPUNIT_ASSERT(!xNameAccess->hasByName("_xmlsignatures"));

    // And that content types no longer contains signature types.
    sal_Int32 nOpenMode = embed::ElementModes::READWRITE;
    uno::Reference<io::XStream> xStream(xStorage->openStreamElement("[Content_Types].xml", nOpenMode), uno::UNO_QUERY);
    uno::Reference<io::XInputStream> xInputStream = xStream->getInputStream();
    uno::Sequence< uno::Sequence<beans::StringPair> > aContentTypeInfo = comphelper::OFOPXMLHelper::ReadContentTypeSequence(xInputStream, mxComponentContext);
    uno::Sequence<beans::StringPair>& rOverrides = aContentTypeInfo[1];
    CPPUNIT_ASSERT_EQUAL(rOverrides.end(), std::find_if(rOverrides.begin(), rOverrides.end(), [](const beans::StringPair& rPair)
    {
        return rPair.First.startsWith("/_xmlsignatures/sig");
    }));
}

void SigningTest::testODFGood()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "good.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // We expect NOTVALIDATED in case the root CA is not imported on the system, and OK otherwise, so accept both.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(
             static_cast<std::underlying_type<SignatureState>::type>(nActual))
         .getStr()),
        (nActual == SignatureState::NOTVALIDATED
         || nActual == SignatureState::OK));
}

void SigningTest::testODFBroken()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "bad.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN), static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

void SigningTest::testODFNo()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "no.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::NOSIGNATURES), static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

// document has one signed timestamp and one unsigned timestamp
void SigningTest::testODFUnsignedTimestamp()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "02_doc_signed_by_trusted_person_manipulated.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(/*o3tl::underlyingEnumValue(*/(int)nActual/*)*/).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->ImplAnalyzeSignature(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    // was: 66666666
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20210126), infos[0].SignatureDate);
    // was: 0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18183742), infos[0].SignatureTime);
}

void SigningTest::testODFX509CertificateChain()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "signed_with_x509certificate_chain.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(/*o3tl::underlyingEnumValue(*/(int)nActual/*)*/).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->ImplAnalyzeSignature(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    // check that the signing certificate was picked, not one of the 2 CA ones
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::VALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(infos[0].Signer.is());
    CPPUNIT_ASSERT_EQUAL(
        OUString("CN=Xmlsecurity RSA Test example Alice,O=Xmlsecurity RSA Test,ST=England,C=UK"),
        // CryptoAPI puts a space after comma, NSS does not...
        infos[0].Signer->getSubjectName().replaceAll(", ", ","));
}

void SigningTest::testODFDoubleX509Data()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "02_doc_signed_by_attacker_manipulated.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(/*o3tl::underlyingEnumValue(*/(int)nActual/*)*/).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->ImplAnalyzeSignature(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

void SigningTest::testODFTripleX509Data()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "02_doc_signed_by_attacker_manipulated_triple.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    // here, libxmlsec will pick the 1st X509Data but signing key is the 2nd
    CPPUNIT_ASSERT_EQUAL_MESSAGE((OString::number(/*o3tl::underlyingEnumValue(*/(int)nActual/*)*/).getStr()),
                                 SignatureState::BROKEN, nActual);
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->ImplAnalyzeSignature(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

void SigningTest::testODFMacroDoubleX509Data()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "02_doc_macros_signed_by_attacker_manipulated.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetScriptingSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(/*o3tl::underlyingEnumValue(*/(int)nActual/*)*/).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->ImplAnalyzeSignature(true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

void SigningTest::testODFDoubleX509Certificate()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "02_doc_signed_by_attacker_manipulated2.odt");
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
    CPPUNIT_ASSERT_MESSAGE((OString::number(/*o3tl::underlyingEnumValue(*/(int)nActual/*)*/).getStr()), nTemp);
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->ImplAnalyzeSignature(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

void SigningTest::testDNCompatibility()
{
    OUString const msDN("CN=\"\"\"ABC\"\".\", O=\"Enterprise \"\"ABC\"\"\"");
    OUString const nssDN("CN=\\\"ABC\\\".,O=Enterprise \\\"ABC\\\"");
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

void SigningTest::testOOXMLPartial()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "partial.docx");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // This was SignatureState::BROKEN due to missing RelationshipTransform and SHA-256 support.
    // We expect NOTVALIDATED in case the root CA is not imported on the system, and PARTIAL_OK otherwise, so accept both.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(
             static_cast<std::underlying_type<SignatureState>::type>(nActual))
         .getStr()),
        (nActual == SignatureState::NOTVALIDATED
         || nActual == SignatureState::PARTIAL_OK));
}

void SigningTest::testOOXMLBroken()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "bad.docx");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // This was SignatureState::NOTVALIDATED/PARTIAL_OK as we did not validate manifest references.
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN), static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

#if HAVE_FEATURE_PDFIMPORT

void SigningTest::testPDFGood()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "good.pdf");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // We expect NOTVALIDATED in case the root CA is not imported on the system, and OK otherwise, so accept both.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(
             static_cast<std::underlying_type<SignatureState>::type>(nActual))
         .getStr()),
        (nActual == SignatureState::NOTVALIDATED
         || nActual == SignatureState::OK));
}

void SigningTest::testPDFBad()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "bad.pdf");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN), static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

void SigningTest::testPDFHideAndReplace()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "hide-and-replace-shadow-file-signed-2.pdf");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2 (BROKEN)
    // - Actual  : 6 (NOTVALIDATED_PARTIAL_OK)
    // i.e. a non-commenting update after a signature was not marked as invalid.
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

void SigningTest::testPDFNo()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "no.pdf");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::NOSIGNATURES), static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

#endif

void SigningTest::test96097Calc()
{
    createCalc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf96097.ods");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT_MESSAGE("Failed to access document base model", pBaseModel);

    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pObjectShell);

    SignatureState nActual = pObjectShell->GetScriptingSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(
             static_cast<std::underlying_type<SignatureState>::type>(nActual))
         .getStr()),
        (nActual == SignatureState::OK
         || nActual == SignatureState::NOTVALIDATED
         || nActual == SignatureState::INVALID));


    uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDocStorable.is());

    // Save a copy
    utl::TempFile aTempFileSaveCopy;
    aTempFileSaveCopy.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> descSaveACopy(2);
    descSaveACopy[0].Name = "SaveACopy";
    descSaveACopy[0].Value <<= true;
    descSaveACopy[1].Name = "FilterName";
    descSaveACopy[1].Value <<= OUString("calc8");
    xDocStorable->storeToURL(aTempFileSaveCopy.GetURL(), descSaveACopy);

    try
    {
        // Save As
        utl::TempFile aTempFileSaveAs;
        aTempFileSaveAs.EnableKillingFile();
        uno::Sequence<beans::PropertyValue> descSaveAs(1);
        descSaveAs[0].Name = "FilterName";
        descSaveAs[0].Value <<= OUString("calc8");
        xDocStorable->storeAsURL(aTempFileSaveAs.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Fail to save as the document");
    }
}

void SigningTest::test96097Doc()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf96097.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);

    SignatureState nActual = pObjectShell->GetScriptingSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(
             static_cast<std::underlying_type<SignatureState>::type>(nActual))
         .getStr()),
        (nActual == SignatureState::OK
         || nActual == SignatureState::NOTVALIDATED
         || nActual == SignatureState::INVALID));



    uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDocStorable.is());

    // Save a copy
    utl::TempFile aTempFileSaveCopy;
    aTempFileSaveCopy.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> descSaveACopy(2);
    descSaveACopy[0].Name = "SaveACopy";
    descSaveACopy[0].Value <<= true;
    descSaveACopy[1].Name = "FilterName";
    descSaveACopy[1].Value <<= OUString("writer8");
    xDocStorable->storeToURL(aTempFileSaveCopy.GetURL(), descSaveACopy);

    try
    {
        // Save As
        utl::TempFile aTempFileSaveAs;
        aTempFileSaveAs.EnableKillingFile();
        uno::Sequence<beans::PropertyValue> descSaveAs(1);
        descSaveAs[0].Name = "FilterName";
        descSaveAs[0].Value <<= OUString("writer8");
        xDocStorable->storeAsURL(aTempFileSaveAs.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Fail to save as the document");
    }
}

void SigningTest::testXAdESNotype()
{
    // Create a working copy.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    OUString aURL = aTempFile.GetURL();
    CPPUNIT_ASSERT_EQUAL(
        osl::File::RC::E_None,
        osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY) + "notype-xades.odt", aURL));

    // Read existing signature.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");
    aManager.read(/*bUseTempStream=*/false);

    // Create a new signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager/*, svl::crypto::SignatureMethodAlgorithm::RSA*/);
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
        = xStorage->openStorageElement("META-INF", embed::ElementModes::READ);
    uno::Reference<io::XInputStream> xInputStream(
        xMetaInf->openStreamElement("documentsignatures.xml", embed::ElementModes::READ),
        uno::UNO_QUERY);
    std::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocPtr pXmlDoc = parseXmlStream(pStream.get());

    // Without the accompanying fix in place, this test would have failed with "unexpected 'Type'
    // attribute", i.e. the signature without such an attribute was not preserved correctly.
    assertXPathNoAttribute(pXmlDoc,
                           "/odfds:document-signatures/dsig:Signature[1]/dsig:SignedInfo/"
                           "dsig:Reference[@URI='#idSignedProperties']",
                           "Type");

    // New signature always has the Type attribute.
    assertXPath(pXmlDoc,
                "/odfds:document-signatures/dsig:Signature[2]/dsig:SignedInfo/"
                "dsig:Reference[@URI='#idSignedProperties']",
                "Type", "http://uri.etsi.org/01903#SignedProperties");
}

void SigningTest::testXAdES()
{
    // Create an empty document, store it to a tempfile and load it as a storage.
    createDoc(OUString());

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer8");
    xStorable->storeAsURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");

    // Create a signature.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager);
    if (!xCertificate.is())
        return;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, /*rDescription=*/OUString(), nSecurityId, /*bAdESCompliant=*/true);

    // Write to storage.
    aManager.read(/*bUseTempStream=*/true);
    aManager.write(/*bXAdESCompliantIfODF=*/true);
    uno::Reference<embed::XTransactedObject> xTransactedObject(xStorage, uno::UNO_QUERY);
    xTransactedObject->commit();

    // Parse the resulting XML.
    uno::Reference<embed::XStorage> xMetaInf = xStorage->openStorageElement("META-INF", embed::ElementModes::READ);
    uno::Reference<io::XInputStream> xInputStream(xMetaInf->openStreamElement("documentsignatures.xml", embed::ElementModes::READ), uno::UNO_QUERY);
    std::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocPtr pXmlDoc = parseXmlStream(pStream.get());

    // Assert that the digest algorithm is SHA-256 in the bAdESCompliant case, not SHA-1.
    assertXPath(pXmlDoc, "/odfds:document-signatures/dsig:Signature/dsig:SignedInfo/dsig:Reference[@URI='content.xml']/dsig:DigestMethod", "Algorithm", ALGO_XMLDSIGSHA256);

    // Assert that the digest of the signing certificate is included.
    assertXPath(pXmlDoc, "//xd:CertDigest", 1);

    // Assert that the Type attribute on the idSignedProperties reference is
    // not missing.
    assertXPath(pXmlDoc, "/odfds:document-signatures/dsig:Signature/dsig:SignedInfo/dsig:Reference[@URI='#idSignedProperties']", "Type", "http://uri.etsi.org/01903#SignedProperties");
}

void SigningTest::testXAdESGood()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "good-xades.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // We expect NOTVALIDATED in case the root CA is not imported on the system, and OK otherwise, so accept both.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(
             static_cast<std::underlying_type<SignatureState>::type>(nActual))
         .getStr()),
        (nActual == SignatureState::NOTVALIDATED
         || nActual == SignatureState::OK));
}

void SigningTest::testInvalidZIP()
{
// set RepairPackage via interaction handler, same as soffice does
// - if it's passed to load the behavior is different, oddly enough.
#if 0
    std::vector<::ucbhelper::InterceptedInteraction::InterceptedRequest> interceptions{
        { css::uno::Any(css::document::BrokenPackageRequest()),
          cppu::UnoType<css::task::XInteractionApprove>::get(), 0 },
    };
    ::rtl::Reference<ucbhelper::InterceptedInteraction> pIH(new ucbhelper::InterceptedInteraction);
    pIH->setInterceptions(std::move(interceptions));

    uno::Sequence<beans::PropertyValue> args = { comphelper::makePropertyValue(
        "InteractionHandler", uno::Reference<task::XInteractionHandler>(pIH)) };
#endif
    OUString const url(m_directories.getURLFromSrc(DATA_DIRECTORY)
                       + "signature-forgery-cdh-lfh.docx");
    mxComponent = mxDesktop->loadComponentFromURL(url, "_default", 0, {} /*args*/);
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(!pBaseModel); // old branch cannot repair DOCX
#if 0
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // the problem was that the document Zip structure is interpreted
    // misleadingly in RepairPackage case, but signature was still returned
    // as partially valid.
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
#endif
}

void SigningTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("odfds"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:digitalsignature:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dsig"), BAD_CAST("http://www.w3.org/2000/09/xmldsig#"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xd"), BAD_CAST("http://uri.etsi.org/01903/v1.3.2#"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SigningTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
