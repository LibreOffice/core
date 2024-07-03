/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>
#include <config_gpgme.h>

#include <sal/config.h>

#include <cstdlib>

#ifndef _WIN32
#include <secoid.h>
#endif

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/BrokenPackageRequest.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/objsh.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <unotools/streamwrap.hxx>

#include <documentsignaturehelper.hxx>
#include <xmlsignaturehelper.hxx>
#include <documentsignaturemanager.hxx>
#include <biginteger.hxx>
#include <certificate.hxx>
#include <xsecctl.hxx>
#include <ucbhelper/interceptedinteraction.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <officecfg/Office/Common.hxx>
#include <comphelper/configuration.hxx>

using namespace com::sun::star;

namespace
{
char const DATA_DIRECTORY[] = "/xmlsecurity/qa/unit/signing/data/";
}

/// Testsuite for the document signing feature.
class SigningTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
protected:
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;
    uno::Reference<xml::crypto::XSEInitializer> mxSEInitializer;
    uno::Reference<xml::crypto::XXMLSecurityContext> mxSecurityContext;

#if HAVE_GPGCONF_SOCKETDIR
    OString m_gpgconfCommandPrefix;
#endif

public:
    SigningTest();
    virtual void setUp() override;
    virtual void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;

protected:
    void createDoc(const OUString& rURL);
    void createCalc(const OUString& rURL);
    uno::Reference<security::XCertificate>
    getCertificate(DocumentSignatureManager& rSignatureManager,
                   svl::crypto::SignatureMethodAlgorithm eAlgo);
    SfxObjectShell* assertDocument(const ::CppUnit::SourceLine aSrcLine,
                                   const OUString& rFilterName, const SignatureState nDocSign,
                                   const SignatureState nMacroSign, const OUString& sVersion);
};

SigningTest::SigningTest() {}

void SigningTest::setUp()
{
    test::BootstrapFixture::setUp();

    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aTargetDir
        = m_directories.getURLFromWorkdir("CppunitTest/xmlsecurity_signing.test.user");

    // Set up NSS database in workdir/CppunitTest/
    osl::File::copy(aSourceDir + "cert9.db", aTargetDir + "/cert9.db");
    osl::File::copy(aSourceDir + "key4.db", aTargetDir + "/key4.db");
    osl::File::copy(aSourceDir + "pkcs11.txt", aTargetDir + "/pkcs11.txt");

    // Make gpg use our own defined setup & keys
    osl::File::copy(aSourceDir + "pubring.gpg", aTargetDir + "/pubring.gpg");
    osl::File::copy(aSourceDir + "random_seed", aTargetDir + "/random_seed");
    osl::File::copy(aSourceDir + "secring.gpg", aTargetDir + "/secring.gpg");
    osl::File::copy(aSourceDir + "trustdb.gpg", aTargetDir + "/trustdb.gpg");

    OUString aTargetPath;
    osl::FileBase::getSystemPathFromFileURL(aTargetDir, aTargetPath);

#ifdef _WIN32
    // CryptoAPI test certificates
    osl::File::copy(aSourceDir + "test.p7b", aTargetDir + "/test.p7b");
    OUString caVar("LIBO_TEST_CRYPTOAPI_PKCS7");
    osl_setEnvironment(caVar.pData, aTargetPath.pData);
#else
    OUString mozCertVar("MOZILLA_CERTIFICATE_FOLDER");
    osl_setEnvironment(mozCertVar.pData, aTargetPath.pData);
#endif
    OUString gpgHomeVar("GNUPGHOME");
    osl_setEnvironment(gpgHomeVar.pData, aTargetPath.pData);

#if HAVE_GPGCONF_SOCKETDIR
    auto const ldPath = std::getenv("LIBO_LD_PATH");
    m_gpgconfCommandPrefix
        = ldPath == nullptr ? OString() : OStringLiteral("LD_LIBRARY_PATH=") + ldPath + " ";
    OString path;
    bool ok = aTargetPath.convertToString(&path, osl_getThreadTextEncoding(),
                                          RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                                              | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR);
    // if conversion fails, at least provide a best-effort conversion in the message here, for
    // context
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(aTargetPath, RTL_TEXTENCODING_UTF8).getStr(), ok);
    m_gpgconfCommandPrefix += "GNUPGHOME=" + path + " " GPGME_GPGCONF;
    // HAVE_GPGCONF_SOCKETDIR is only defined in configure.ac for Linux for now, so (a) std::system
    // behavior will conform to POSIX (and the relevant env var to set is named LD_LIBRARY_PATH), and
    // (b) gpgconf --create-socketdir should return zero:
    OString cmd = m_gpgconfCommandPrefix + " --create-socketdir";
    int res = std::system(cmd.getStr());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(cmd.getStr(), 0, res);
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

#if HAVE_GPGCONF_SOCKETDIR
    // HAVE_GPGCONF_SOCKETDIR is only defined in configure.ac for Linux for now, so (a) std::system
    // behavior will conform to POSIX, and (b) gpgconf --remove-socketdir should return zero:
    OString cmd = m_gpgconfCommandPrefix + " --remove-socketdir";
    int res = std::system(cmd.getStr());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(cmd.getStr(), 0, res);
#endif

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
        mxComponent
            = loadFromDesktop("private:factory/swriter", "com.sun.star.sheet.SpreadsheetDocument");
    else
        mxComponent = loadFromDesktop(rURL, "com.sun.star.sheet.SpreadsheetDocument");
}

uno::Reference<security::XCertificate>
SigningTest::getCertificate(DocumentSignatureManager& rSignatureManager,
                            svl::crypto::SignatureMethodAlgorithm eAlgo)
{
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment
        = rSignatureManager.getSecurityEnvironment();
    uno::Sequence<uno::Reference<security::XCertificate>> aCertificates
        = xSecurityEnvironment->getPersonalCertificates();

    for (const auto& xCertificate : aCertificates)
    {
        auto pCertificate = dynamic_cast<xmlsecurity::Certificate*>(xCertificate.get());
        CPPUNIT_ASSERT(pCertificate);
        if (pCertificate->getSignatureMethodAlgorithm() == eAlgo)
            return xCertificate;
    }
    return uno::Reference<security::XCertificate>();
}

CPPUNIT_TEST_FIXTURE(SigningTest, testDescription)
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
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, "1.2");

    // Then add a signature document.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
    if (!xCertificate.is())
        return;
    OUString aDescription("SigningTest::testDescription");
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
    createDoc("");

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer8");
    xStorable->storeAsURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, "1.2");

    // Then add a signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::ECDSA);
    if (!xCertificate.is())
        return;
    OUString aDescription;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, aDescription, nSecurityId, false);

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
    createDoc("");

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("MS Word 2007 XML");
    xStorable->storeAsURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, "1.2");

    // Then add a document signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::ECDSA);
    if (!xCertificate.is())
        return;
    OUString aDescription;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, aDescription, nSecurityId,
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
    createDoc("");

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer_pdf_Export");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(
        aTempFile.GetURL(), StreamMode::READ | StreamMode::WRITE));
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
    CPPUNIT_ASSERT(xStream.is());
    aManager.setSignatureStream(xStream);

    // Then add a document signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::ECDSA);
    if (!xCertificate.is())
        return;
    OUString aDescription;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, aDescription, nSecurityId,
                 /*bAdESCompliant=*/true);

    // Read back the signature and make sure that it's valid.
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    // This was SecurityOperationStatus_UNKNOWN, signing with an ECDSA key was
    // broken.
    CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                         rInformations[0].nStatus);
}

CPPUNIT_TEST_FIXTURE(SigningTest, testOOXMLDescription)
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
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, "1.2");

    // Then add a document signature.
    uno::Reference<security::XCertificate> xCertificate
        = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
    if (!xCertificate.is())
        return;
    OUString aDescription("SigningTest::testDescription");
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
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    OUString aURL = aTempFile.GetURL();
    CPPUNIT_ASSERT_EQUAL(
        osl::File::RC::E_None,
        osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY) + "partial.docx", aURL));
    // Load the test document as a storage and read its single signature.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL,
                                                                embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, "1.2");
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
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    OUString aURL = aTempFile.GetURL();
    CPPUNIT_ASSERT_EQUAL(
        osl::File::RC::E_None,
        osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY) + "multi.docx", aURL));
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL,
                                                                embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, "1.2");
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
    CPPUNIT_ASSERT_EQUAL(OUString("purpose1"), rInformations[0].ouDescription);
}

/// Test removing all signatures from a document.
CPPUNIT_TEST_FIXTURE(SigningTest, testOOXMLRemoveAll)
{
    // Copy the test document to a temporary file, as it'll be modified.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    OUString aURL = aTempFile.GetURL();
    CPPUNIT_ASSERT_EQUAL(
        osl::File::RC::E_None,
        osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY) + "partial.docx", aURL));
    // Load the test document as a storage and read its single signature.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL,
                                                                embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, "1.2");
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
    uno::Reference<container::XNameAccess> xNameAccess(xStorage, uno::UNO_QUERY);
    CPPUNIT_ASSERT(!xNameAccess->hasByName("_xmlsignatures"));

    // And that content types no longer contains signature types.
    uno::Reference<io::XStream> xStream(
        xStorage->openStreamElement("[Content_Types].xml", embed::ElementModes::READWRITE),
        uno::UNO_QUERY);
    uno::Reference<io::XInputStream> xInputStream = xStream->getInputStream();
    uno::Sequence<uno::Sequence<beans::StringPair>> aContentTypeInfo
        = comphelper::OFOPXMLHelper::ReadContentTypeSequence(xInputStream, mxComponentContext);
    uno::Sequence<beans::StringPair>& rOverrides = aContentTypeInfo[1];
    CPPUNIT_ASSERT(
        std::none_of(rOverrides.begin(), rOverrides.end(), [](const beans::StringPair& rPair) {
            return rPair.First.startsWith("/_xmlsignatures/sig");
        }));
}

/// Test a typical ODF where all streams are signed.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFGood)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "good.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // We expect NOTVALIDATED in case the root CA is not imported on the system, and OK otherwise, so accept both.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
}

/// Test a typical broken ODF signature where one stream is corrupted.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFBroken)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "bad.odt");
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
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "no.odt");
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
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "02_doc_signed_by_trusted_person_manipulated.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    // was: 66666666
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20210126), infos[0].SignatureDate);
    // was: 0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18183742), infos[0].SignatureTime);
}

CPPUNIT_TEST_FIXTURE(SigningTest, testODFX509CertificateChain)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "signed_with_x509certificate_chain.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    // check that the signing certificate was picked, not one of the 2 CA ones
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::VALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(infos[0].Signer.is());
    CPPUNIT_ASSERT_EQUAL(
        OUString("CN=Xmlsecurity RSA Test example Alice,O=Xmlsecurity RSA Test,ST=England,C=UK"),
        // CryptoAPI puts a space after comma, NSS does not...
        infos[0].Signer->getSubjectName().replaceAll(", ", ","));
}

CPPUNIT_TEST_FIXTURE(SigningTest, testODFDoubleX509Data)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "02_doc_signed_by_attacker_manipulated.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

CPPUNIT_TEST_FIXTURE(SigningTest, testODFTripleX509Data)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "02_doc_signed_by_attacker_manipulated_triple.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    // here, libxmlsec will pick the 1st X509Data but signing key is the 2nd
    CPPUNIT_ASSERT_EQUAL_MESSAGE((OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
                                 SignatureState::BROKEN, nActual);
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

CPPUNIT_TEST_FIXTURE(SigningTest, testODFMacroDoubleX509Data)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY)
              + "02_doc_macros_signed_by_attacker_manipulated.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    SignatureState nActual = pObjectShell->GetScriptingSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(true));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

CPPUNIT_TEST_FIXTURE(SigningTest, testODFDoubleX509Certificate)
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
    CPPUNIT_ASSERT_MESSAGE((OString::number(o3tl::underlyingEnumValue(nActual)).getStr()), nTemp);
    uno::Sequence<security::DocumentSignatureInformation> const infos(
        pObjectShell->GetDocumentSignatureInformation(false));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), infos.getLength());
    CPPUNIT_ASSERT_EQUAL(security::CertificateValidity::INVALID, infos[0].CertificateStatus);
    CPPUNIT_ASSERT(!infos[0].Signer.is());
}

CPPUNIT_TEST_FIXTURE(SigningTest, testDNCompatibility)
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

/// Test a typical OOXML where a number of (but not all) streams are signed.
CPPUNIT_TEST_FIXTURE(SigningTest, testOOXMLPartial)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "partial.docx");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // This was SignatureState::BROKEN due to missing RelationshipTransform and SHA-256 support.
    // We expect NOTVALIDATED_PARTIAL_OK in case the root CA is not imported on the system, and PARTIAL_OK otherwise, so accept both.
    // But reject NOTVALIDATED, hiding incompleteness is not OK.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE((OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
                           (nActual == SignatureState::NOTVALIDATED_PARTIAL_OK
                            || nActual == SignatureState::PARTIAL_OK));
}

/// Test a typical broken OOXML signature where one stream is corrupted.
CPPUNIT_TEST_FIXTURE(SigningTest, testOOXMLBroken)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "bad.docx");
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
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "good.pdf");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // We expect NOTVALIDATED in case the root CA is not imported on the system, and OK otherwise, so accept both.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
}

/// Test a typical PDF where the signature is bad.
CPPUNIT_TEST_FIXTURE(SigningTest, testPDFBad)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "bad.pdf");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

CPPUNIT_TEST_FIXTURE(SigningTest, testPDFHideAndReplace)
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

/// Test a typical PDF which is not signed.
CPPUNIT_TEST_FIXTURE(SigningTest, testPDFNo)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "no.pdf");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::NOSIGNATURES),
                         static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

#endif

CPPUNIT_TEST_FIXTURE(SigningTest, test96097Calc)
{
    createCalc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf96097.ods");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT_MESSAGE("Failed to access document base model", pBaseModel);

    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pObjectShell);

    SignatureState nActual = pObjectShell->GetScriptingSignatureState();
    CPPUNIT_ASSERT_MESSAGE((OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
                           (nActual == SignatureState::OK || nActual == SignatureState::NOTVALIDATED
                            || nActual == SignatureState::INVALID));

    uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY_THROW);

    // Save a copy
    utl::TempFile aTempFileSaveCopy;
    aTempFileSaveCopy.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> descSaveACopy(comphelper::InitPropertySequence(
        { { "SaveACopy", uno::Any(true) }, { "FilterName", uno::Any(OUString("calc8")) } }));
    xDocStorable->storeToURL(aTempFileSaveCopy.GetURL(), descSaveACopy);

    try
    {
        // Save As
        utl::TempFile aTempFileSaveAs;
        aTempFileSaveAs.EnableKillingFile();
        uno::Sequence<beans::PropertyValue> descSaveAs(
            comphelper::InitPropertySequence({ { "FilterName", uno::Any(OUString("calc8")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAs.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Fail to save as the document");
    }
}

CPPUNIT_TEST_FIXTURE(SigningTest, test96097Doc)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf96097.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);

    SignatureState nActual = pObjectShell->GetScriptingSignatureState();
    CPPUNIT_ASSERT_MESSAGE((OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
                           (nActual == SignatureState::OK || nActual == SignatureState::NOTVALIDATED
                            || nActual == SignatureState::INVALID));

    uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY_THROW);

    // Save a copy
    utl::TempFile aTempFileSaveCopy;
    aTempFileSaveCopy.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> descSaveACopy(comphelper::InitPropertySequence(
        { { "SaveACopy", uno::Any(true) }, { "FilterName", uno::Any(OUString("writer8")) } }));
    xDocStorable->storeToURL(aTempFileSaveCopy.GetURL(), descSaveACopy);

    try
    {
        // Save As
        utl::TempFile aTempFileSaveAs;
        aTempFileSaveAs.EnableKillingFile();
        uno::Sequence<beans::PropertyValue> descSaveAs(
            comphelper::InitPropertySequence({ { "FilterName", uno::Any(OUString("writer8")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAs.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Fail to save as the document");
    }
}

CPPUNIT_TEST_FIXTURE(SigningTest, testXAdESNotype)
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
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, "1.2");
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

/// Creates a XAdES signature from scratch.
CPPUNIT_TEST_FIXTURE(SigningTest, testXAdES)
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
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, "1.2");

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
        = xStorage->openStorageElement("META-INF", embed::ElementModes::READ);
    uno::Reference<io::XInputStream> xInputStream(
        xMetaInf->openStreamElement("documentsignatures.xml", embed::ElementModes::READ),
        uno::UNO_QUERY);
    std::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocPtr pXmlDoc = parseXmlStream(pStream.get());

    // Assert that the digest algorithm is SHA-256 in the bAdESCompliant case, not SHA-1.
    assertXPath(pXmlDoc,
                "/odfds:document-signatures/dsig:Signature/dsig:SignedInfo/"
                "dsig:Reference[@URI='content.xml']/dsig:DigestMethod",
                "Algorithm", ALGO_XMLDSIGSHA256);

    // Assert that the digest of the signing certificate is included.
    assertXPath(pXmlDoc, "//xd:CertDigest", 1);

    // Assert that the Type attribute on the idSignedProperties reference is
    // not missing.
    assertXPath(pXmlDoc,
                "/odfds:document-signatures/dsig:Signature/dsig:SignedInfo/"
                "dsig:Reference[@URI='#idSignedProperties']",
                "Type", "http://uri.etsi.org/01903#SignedProperties");
}

/// Works with an existing good XAdES signature.
CPPUNIT_TEST_FIXTURE(SigningTest, testXAdESGood)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "good-xades.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // We expect NOTVALIDATED in case the root CA is not imported on the system, and OK otherwise, so accept both.
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
        (nActual == SignatureState::NOTVALIDATED || nActual == SignatureState::OK));
}

/// Test importing of signature line
CPPUNIT_TEST_FIXTURE(SigningTest, testSignatureLineOOXML)
{
    // Given: A document (docx) with a signature line and a valid signature
    uno::Reference<security::XDocumentDigitalSignatures> xSignatures(
        security::DocumentDigitalSignatures::createWithVersion(
            comphelper::getProcessComponentContext(), "1.2"));

    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING,
            m_directories.getURLFromSrc(DATA_DIRECTORY) + "signatureline.docx",
            embed::ElementModes::READ);
    CPPUNIT_ASSERT(xStorage.is());

    uno::Sequence<security::DocumentSignatureInformation> xSignatureInfo
        = xSignatures->verifyScriptingContentSignatures(xStorage,
                                                        uno::Reference<io::XInputStream>());

    // The signature should have a valid signature, and signature line with two valid images
    CPPUNIT_ASSERT(xSignatureInfo[0].SignatureIsValid);
    CPPUNIT_ASSERT_EQUAL(OUString("{DEE0514B-13E8-4674-A831-46E3CDB18BB4}"),
                         xSignatureInfo[0].SignatureLineId);
    CPPUNIT_ASSERT(xSignatureInfo[0].ValidSignatureLineImage.is());
    CPPUNIT_ASSERT(xSignatureInfo[0].InvalidSignatureLineImage.is());
}

CPPUNIT_TEST_FIXTURE(SigningTest, testSignatureLineODF)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "signatureline.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);

    uno::Sequence<security::DocumentSignatureInformation> xSignatureInfo
        = pObjectShell->GetDocumentSignatureInformation(false);

    CPPUNIT_ASSERT(xSignatureInfo[0].SignatureIsValid);
    CPPUNIT_ASSERT_EQUAL(OUString("{41CF56EE-331B-4125-97D8-2F5669DD3AAC}"),
                         xSignatureInfo[0].SignatureLineId);
    CPPUNIT_ASSERT(xSignatureInfo[0].ValidSignatureLineImage.is());
    CPPUNIT_ASSERT(xSignatureInfo[0].InvalidSignatureLineImage.is());
}

#if HAVE_FEATURE_GPGVERIFY
/// Test a typical ODF where all streams are GPG-signed.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFGoodGPG)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "goodGPG.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // Our local gpg config fully trusts the signing cert, so in
    // contrast to the X509 test we can fail on NOTVALIDATED here
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_EQUAL_MESSAGE((OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
                                 SignatureState::OK, nActual);
}

/// Test a typical ODF where all streams are GPG-signed, but we don't trust the signature.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFUntrustedGoodGPG)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "untrustedGoodGPG.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // Our local gpg config does _not_ trust the signing cert, so in
    // contrast to the X509 test we can fail everything but
    // NOTVALIDATED here
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_EQUAL_MESSAGE((OString::number(o3tl::underlyingEnumValue(nActual)).getStr()),
                                 SignatureState::NOTVALIDATED, nActual);
}

CPPUNIT_TEST_FIXTURE(SigningTest, testInvalidZIP)
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

/// Test a typical broken ODF signature where one stream is corrupted.
CPPUNIT_TEST_FIXTURE(SigningTest, testODFBrokenStreamGPG)
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "badStreamGPG.odt");
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
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "badDsigGPG.odt");
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
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "encryptedGPG.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);

    // ODF1.3 flavour
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "encryptedGPG_odf13.odt");
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
    xPropSet->getPropertyValue("Version") >>= aODFVersion;
    CPPUNIT_ASSERT_EQUAL(sVersion, aODFVersion);

    return pObjectShell;
}

/// Test if a macro signature from a OTT 1.2 template is preserved for ODT 1.2
CPPUNIT_TEST_FIXTURE(SigningTest, testPreserveMacroTemplateSignature12_ODF)
{
    const OUString aURL(m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf42316_odt12.ott");
    const OUString sLoadMessage = "loading failed: " + aURL;

    // load the template as-is to validate signatures
    mxComponent = loadFromDesktop(
        aURL, OUString(), comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // we are a template, and have a valid document and macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::OK, SignatureState::OK,
                   ODFVER_012_TEXT);

    // create new document from template
    // we can't use createDoc / MacrosTest::loadFromDesktop, because ALWAYS_EXECUTE_NO_WARN
    // won't verify the signature for templates, so the resulting document won't be able to
    // preserve the templates signature.
    mxComponent->dispose();
    mxComponent = mxDesktop->loadComponentFromURL(
        aURL, "_default", 0,
        comphelper::InitPropertySequence(
            { { "MacroExecutionMode",
                uno::Any(document::MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // we are somehow a template (?), and have just a valid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::NOSIGNATURES,
                   SignatureState::OK, ODFVER_012_TEXT);

    // save as new ODT document
    utl::TempFile aTempFileSaveAsODT;
    aTempFileSaveAsODT.EnableKillingFile();
    try
    {
        uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> descSaveAs(
            comphelper::InitPropertySequence({ { "FilterName", uno::Any(OUString("writer8")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAsODT.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Failed to save ODT document");
    }

    // save as new OTT template
    utl::TempFile aTempFileSaveAsOTT;
    aTempFileSaveAsOTT.EnableKillingFile();
    try
    {
        uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> descSaveAs(comphelper::InitPropertySequence(
            { { "FilterName", uno::Any(OUString("writer8_template")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAsOTT.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Failed to save OTT template");
    }

    // load the saved OTT template as-is to validate signatures
    mxComponent->dispose();
    mxComponent
        = loadFromDesktop(aTempFileSaveAsOTT.GetURL(), OUString(),
                          comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // the loaded document is a OTT with a valid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::NOSIGNATURES,
                   SignatureState::OK, ODFVER_012_TEXT);

    // load saved ODT document
    createDoc(aTempFileSaveAsODT.GetURL());

    // the loaded document is a ODT with a macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8", SignatureState::NOSIGNATURES,
                   SignatureState::OK, ODFVER_012_TEXT);

    // save as new OTT template
    utl::TempFile aTempFileSaveAsODT_OTT;
    aTempFileSaveAsODT_OTT.EnableKillingFile();
    try
    {
        uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> descSaveAs(comphelper::InitPropertySequence(
            { { "FilterName", uno::Any(OUString("writer8_template")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAsODT_OTT.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Failed to save OTT template");
    }

    // load the template as-is to validate signatures
    mxComponent->dispose();
    mxComponent
        = loadFromDesktop(aTempFileSaveAsODT_OTT.GetURL(), OUString(),
                          comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // the loaded document is a OTT with a valid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::NOSIGNATURES,
                   SignatureState::OK, ODFVER_012_TEXT);
}

/// Test if a macro signature from an OTT 1.0 is dropped for ODT 1.2
CPPUNIT_TEST_FIXTURE(SigningTest, testDropMacroTemplateSignature)
{
    const OUString aURL(m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf42316.ott");
    const OUString sLoadMessage = "loading failed: " + aURL;

    // load the template as-is to validate signatures
    mxComponent = loadFromDesktop(
        aURL, OUString(), comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // we are a template, and have a non-invalid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::NOSIGNATURES,
                   SignatureState::NOTVALIDATED, OUString());

    // create new document from template
    // we can't use createDoc / MacrosTest::loadFromDesktop, because ALWAYS_EXECUTE_NO_WARN
    // won't verify the signature for templates, so the resulting document won't be able to
    // preserve the templates signature.
    mxComponent->dispose();
    mxComponent = mxDesktop->loadComponentFromURL(
        aURL, "_default", 0,
        comphelper::InitPropertySequence(
            { { "MacroExecutionMode",
                uno::Any(document::MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // we are somehow a template (?), and have just a valid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::NOSIGNATURES,
                   SignatureState::NOTVALIDATED, OUString());

    // save as new ODT document
    utl::TempFile aTempFileSaveAs;
    aTempFileSaveAs.EnableKillingFile();
    try
    {
        uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> descSaveAs(
            comphelper::InitPropertySequence({ { "FilterName", uno::Any(OUString("writer8")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAs.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Failed to save ODT document");
    }

    // load saved document
    createDoc(aTempFileSaveAs.GetURL());

    // the loaded document is a 1.2 ODT without any signatures
    assertDocument(CPPUNIT_SOURCELINE(), "writer8", SignatureState::NOSIGNATURES,
                   SignatureState::NOSIGNATURES, ODFVER_012_TEXT);

    // load the template as-is to validate signatures
    mxComponent->dispose();
    mxComponent = loadFromDesktop(
        aURL, OUString(), comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // we are a template, and have a non-invalid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::NOSIGNATURES,
                   SignatureState::NOTVALIDATED, OUString());

    // save as new OTT template
    utl::TempFile aTempFileSaveAsOTT;
    aTempFileSaveAsOTT.EnableKillingFile();
    try
    {
        uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> descSaveAs(comphelper::InitPropertySequence(
            { { "FilterName", uno::Any(OUString("writer8_template")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAsOTT.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Failed to save OTT template");
    }

    // load the template as-is to validate signatures
    mxComponent->dispose();
    mxComponent
        = loadFromDesktop(aTempFileSaveAsOTT.GetURL(), OUString(),
                          comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // the loaded document is a 1.2 OTT without any signatures
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::NOSIGNATURES,
                   SignatureState::NOSIGNATURES, ODFVER_012_TEXT);
}

class Resetter
{
private:
    std::function<void()> m_Func;

public:
    Resetter(std::function<void()> const& rFunc)
        : m_Func(rFunc)
    {
    }
    ~Resetter()
    {
        try
        {
            m_Func();
        }
        catch (...) // has to be reliable
        {
            fprintf(stderr, "resetter failed with exception\n");
            abort();
        }
    }
};

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

    const OUString aURL(m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf42316.ott");
    const OUString sLoadMessage = "loading failed: " + aURL;

    // load the template as-is to validate signatures
    mxComponent = loadFromDesktop(
        aURL, OUString(), comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // we are a template, and have a non-invalid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::NOSIGNATURES,
                   SignatureState::NOTVALIDATED, OUString());

    // create new document from template
    // we can't use createDoc / MacrosTest::loadFromDesktop, because ALWAYS_EXECUTE_NO_WARN
    // won't verify the signature for templates, so the resulting document won't be able to
    // preserve the templates signature.
    mxComponent->dispose();
    mxComponent = mxDesktop->loadComponentFromURL(
        aURL, "_default", 0,
        comphelper::InitPropertySequence(
            { { "MacroExecutionMode",
                uno::Any(document::MacroExecMode::FROM_LIST_AND_SIGNED_NO_WARN) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // we are somehow a template (?), and have just a valid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::NOSIGNATURES,
                   SignatureState::NOTVALIDATED, OUString());

    // save as new ODT document
    utl::TempFile aTempFileSaveAsODT;
    aTempFileSaveAsODT.EnableKillingFile();
    try
    {
        uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> descSaveAs(
            comphelper::InitPropertySequence({ { "FilterName", uno::Any(OUString("writer8")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAsODT.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Failed to save ODT document");
    }

    // save as new OTT template
    utl::TempFile aTempFileSaveAsOTT;
    aTempFileSaveAsOTT.EnableKillingFile();
    try
    {
        uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> descSaveAs(comphelper::InitPropertySequence(
            { { "FilterName", uno::Any(OUString("writer8_template")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAsOTT.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Failed to save OTT template");
    }

    // load the saved OTT template as-is to validate signatures
    mxComponent->dispose();
    mxComponent
        = loadFromDesktop(aTempFileSaveAsOTT.GetURL(), OUString(),
                          comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // the loaded document is a OTT with a non-invalid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::NOSIGNATURES,
                   SignatureState::NOTVALIDATED, OUString());

    // load saved ODT document
    createDoc(aTempFileSaveAsODT.GetURL());

    // the loaded document is a ODT with a non-invalid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8", SignatureState::NOSIGNATURES,
                   SignatureState::NOTVALIDATED, OUString());

    // save as new OTT template
    utl::TempFile aTempFileSaveAsODT_OTT;
    aTempFileSaveAsODT_OTT.EnableKillingFile();
    try
    {
        uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> descSaveAs(comphelper::InitPropertySequence(
            { { "FilterName", uno::Any(OUString("writer8_template")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAsODT_OTT.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Failed to save OTT template");
    }

    // load the template as-is to validate signatures
    mxComponent->dispose();
    mxComponent
        = loadFromDesktop(aTempFileSaveAsODT_OTT.GetURL(), OUString(),
                          comphelper::InitPropertySequence({ { "AsTemplate", uno::Any(false) } }));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // the loaded document is a OTT with a non-invalid macro signature
    assertDocument(CPPUNIT_SOURCELINE(), "writer8_template", SignatureState::NOSIGNATURES,
                   SignatureState::NOTVALIDATED, OUString());
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
