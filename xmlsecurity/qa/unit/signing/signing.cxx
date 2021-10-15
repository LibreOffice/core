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

#include <type_traits>

#ifndef _WIN32
#include <secoid.h>
#endif

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

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
#include <comphelper/sequence.hxx>
#include <comphelper/ofopxmlhelper.hxx>

#include <documentsignaturehelper.hxx>
#include <xmlsignaturehelper.hxx>
#include <documentsignaturemanager.hxx>
#include <certificate.hxx>
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
    void testECDSA();
    void testECDSAOOXML();
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
    /// Test importing of signature line images
    void testSignatureLineImages();
#if HAVE_FEATURE_GPGVERIFY
    /// Test a typical ODF where all streams are GPG-signed.
    void testODFGoodGPG();
    /// Test a typical ODF where all streams are GPG-signed, but we don't trust the signature.
    void testODFUntrustedGoodGPG();
    /// Test a typical broken ODF signature where one stream is corrupted.
    void testODFBrokenStreamGPG();
    /// Test a typical broken ODF signature where the XML dsig hash is corrupted.
    void testODFBrokenDsigGPG();
#if HAVE_GPGCONF_SOCKETDIR
    /// Test loading an encrypted ODF document
    void testODFEncryptedGPG();
#endif
#endif
    void testPreserveMacroTemplateSignature12();
    void testDropMacroTemplateSignature();
    void testPreserveMacroTemplateSignature10();
    void testPreserveMacroSignatureODB();

    CPPUNIT_TEST_SUITE(SigningTest);
    CPPUNIT_TEST(testDescription);
    CPPUNIT_TEST(testECDSA);
    CPPUNIT_TEST(testECDSAOOXML);
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
    CPPUNIT_TEST(testSignatureLineImages);
#if HAVE_FEATURE_GPGVERIFY
    CPPUNIT_TEST(testODFGoodGPG);
    CPPUNIT_TEST(testODFUntrustedGoodGPG);
    CPPUNIT_TEST(testODFBrokenStreamGPG);
    CPPUNIT_TEST(testODFBrokenDsigGPG);
    CPPUNIT_TEST(testPreserveMacroTemplateSignature12);
#if HAVE_GPGCONF_SOCKETDIR
    CPPUNIT_TEST(testODFEncryptedGPG);
#endif
#endif
    CPPUNIT_TEST(testDropMacroTemplateSignature);
    CPPUNIT_TEST(testPreserveMacroTemplateSignature10);
    CPPUNIT_TEST(testPreserveMacroSignatureODB);
    CPPUNIT_TEST_SUITE_END();

private:
    void createDoc(const OUString& rURL);
    void createCalc(const OUString& rURL);
    uno::Reference<security::XCertificate> getCertificate(DocumentSignatureManager& rSignatureManager, svl::crypto::SignatureMethodAlgorithm eAlgo);
    SfxObjectShell* assertDocument(const ::CppUnit::SourceLine aSrcLine,
                                   const OUString& rFilterName, const SignatureState nDocSign,
                                   const SignatureState nMacroSign, const OUString& sVersion);
};

SigningTest::SigningTest()
{
}

void SigningTest::setUp()
{
    test::BootstrapFixture::setUp();

    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aTargetDir = m_directories.getURLFromWorkdir(
                              "/CppunitTest/xmlsecurity_signing.test.user/");

    // Set up NSS database in workdir/CppunitTest/
    osl::File::copy(aSourceDir + "cert9.db", aTargetDir + "cert9.db");
    osl::File::copy(aSourceDir + "key4.db", aTargetDir + "key4.db");
    osl::File::copy(aSourceDir + "pkcs11.txt", aTargetDir + "pkcs11.txt");

    // Make gpg use our own defined setup & keys
    osl::File::copy(aSourceDir + "pubring.gpg", aTargetDir + "pubring.gpg");
    osl::File::copy(aSourceDir + "random_seed", aTargetDir + "random_seed");
    osl::File::copy(aSourceDir + "secring.gpg", aTargetDir + "secring.gpg");
    osl::File::copy(aSourceDir + "trustdb.gpg", aTargetDir + "trustdb.gpg");

    OUString aTargetPath;
    osl::FileBase::getSystemPathFromFileURL(aTargetDir, aTargetPath);

#ifdef _WIN32
    // CryptoAPI test certificates
    osl::File::copy(aSourceDir + "test.p7b", aTargetDir + "test.p7b");
    OUString caVar("LIBO_TEST_CRYPTOAPI_PKCS7");
    osl_setEnvironment(caVar.pData, aTargetPath.pData);
#else
    OUString mozCertVar("MOZILLA_CERTIFICATE_FOLDER");
    osl_setEnvironment(mozCertVar.pData, aTargetPath.pData);
#endif
    OUString gpgHomeVar("GNUPGHOME");
    osl_setEnvironment(gpgHomeVar.pData, aTargetPath.pData);

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

uno::Reference<security::XCertificate> SigningTest::getCertificate(DocumentSignatureManager& rSignatureManager, svl::crypto::SignatureMethodAlgorithm eAlgo)
{
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = rSignatureManager.getSecurityEnvironment();
    uno::Sequence<uno::Reference<security::XCertificate>> aCertificates = xSecurityEnvironment->getPersonalCertificates();

    for (const auto& xCertificate : aCertificates)
    {
        auto pCertificate = dynamic_cast<xmlsecurity::Certificate*>(xCertificate.get());
        CPPUNIT_ASSERT(pCertificate);
        if (pCertificate->getSignatureMethodAlgorithm() == eAlgo)
            return xCertificate;
    }
    return uno::Reference<security::XCertificate>();
}

void SigningTest::testPreserveMacroSignatureODB()
{
    const OUString aURL(m_directories.getURLFromSrc(DATA_DIRECTORY) + "odb_signed_macros.odb");
    const OUString sLoadMessage = "loading failed: " + aURL;

    // load the file
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop(aURL, "com.sun.star.sdb.OfficeDatabaseDocument");
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // save as ODB
    utl::TempFile aTempFileSaveAsODB;
    aTempFileSaveAsODB.EnableKillingFile();
    try
    {
        uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> descSaveAs(comphelper::InitPropertySequence(
            { { "FilterName", uno::Any(OUString("StarOffice XML (Base)")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAsODB.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Failed to save ODB file");
    }

    // Parse the resulting XML.
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, aTempFileSaveAsODB.GetURL(), embed::ElementModes::READ);
    CPPUNIT_ASSERT(xStorage.is());
    uno::Reference<embed::XStorage> xMetaInf
        = xStorage->openStorageElement("META-INF", embed::ElementModes::READ);
    uno::Reference<io::XInputStream> xInputStream(
        xMetaInf->openStreamElement("macrosignatures.xml", embed::ElementModes::READ),
        uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocPtr pXmlDoc = parseXmlStream(pStream.get());

    // Make sure the signature is still there
    assertXPath(pXmlDoc, "//dsig:Signature", "Id",
                "ID_00a7002f009000bc00ce00f7004400460080002f002e00e400e0003700df00e8");
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
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
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

void SigningTest::testECDSA()
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

    // Then add a signature.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::ECDSA);
    if (!xCertificate.is())
        return;
    OUString aDescription;
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, mxSecurityContext, aDescription, nSecurityId, false);

    // Read back the signature and make sure that it's valid.
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    // This was SecurityOperationStatus_UNKNOWN, signing with an ECDSA key was
    // broken.
    CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED, rInformations[0].nStatus);
}

void SigningTest::testECDSAOOXML()
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
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");

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
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    // This was SecurityOperationStatus_UNKNOWN, signing with an ECDSA key was
    // broken.
    CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                         rInformations[0].nStatus);
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
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
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
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
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
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
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
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
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
    uno::Reference<io::XStream> xStream(xStorage->openStreamElement("[Content_Types].xml", embed::ElementModes::READWRITE), uno::UNO_QUERY);
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

void SigningTest::testOOXMLPartial()
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
    CPPUNIT_ASSERT_MESSAGE(
        (OString::number(
             static_cast<std::underlying_type<SignatureState>::type>(nActual))
         .getStr()),
        (nActual == SignatureState::NOTVALIDATED_PARTIAL_OK
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
    uno::Sequence<beans::PropertyValue> descSaveACopy(comphelper::InitPropertySequence(
    {
        { "SaveACopy", uno::Any(true) },
        { "FilterName", uno::Any(OUString("calc8")) }
    }));
    xDocStorable->storeToURL(aTempFileSaveCopy.GetURL(), descSaveACopy);

    try
    {
        // Save As
        utl::TempFile aTempFileSaveAs;
        aTempFileSaveAs.EnableKillingFile();
        uno::Sequence<beans::PropertyValue> descSaveAs(comphelper::InitPropertySequence(
        {
            { "FilterName", uno::Any(OUString("calc8")) }
        }));
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
    uno::Sequence<beans::PropertyValue> descSaveACopy(comphelper::InitPropertySequence(
    {
        { "SaveACopy", uno::Any(true) },
        { "FilterName", uno::Any(OUString("writer8")) }
    }));
    xDocStorable->storeToURL(aTempFileSaveCopy.GetURL(), descSaveACopy);

    try
    {
        // Save As
        utl::TempFile aTempFileSaveAs;
        aTempFileSaveAs.EnableKillingFile();
        uno::Sequence<beans::PropertyValue> descSaveAs(comphelper::InitPropertySequence(
        {
            { "FilterName", uno::Any(OUString("writer8")) }
        }));
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
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager, svl::crypto::SignatureMethodAlgorithm::RSA);
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

void SigningTest::testSignatureLineImages()
{
    // Given: A document (docx) with a signature line and a valid signature
    uno::Reference< security::XDocumentDigitalSignatures > xSignatures(
        security::DocumentDigitalSignatures::createWithVersion(
            comphelper::getProcessComponentContext(), "1.2"));

    uno::Reference<embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
                ZIP_STORAGE_FORMAT_STRING, m_directories.getURLFromSrc(DATA_DIRECTORY) + "signatureline.docx",
                embed::ElementModes::READ);
    CPPUNIT_ASSERT(xStorage.is());

    uno::Sequence< security::DocumentSignatureInformation > xSignatureInfo =
        xSignatures->verifyScriptingContentSignatures(xStorage, uno::Reference< io::XInputStream >());

    // The signature should have a valid signature, and signature line with two valid images
    CPPUNIT_ASSERT(xSignatureInfo[0].SignatureIsValid);
    CPPUNIT_ASSERT_EQUAL(OUString("{DEE0514B-13E8-4674-A831-46E3CDB18BB4}"), xSignatureInfo[0].SignatureLineId);
    CPPUNIT_ASSERT(xSignatureInfo[0].ValidSignatureLineImage.is());
    CPPUNIT_ASSERT(xSignatureInfo[0].InvalidSignatureLineImage.is());
}

#if HAVE_FEATURE_GPGVERIFY
void SigningTest::testODFGoodGPG()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "goodGPG.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    // Our local gpg config fully trusts the signing cert, so in
    // contrast to the X509 test we can fail on NOTVALIDATED here
    SignatureState nActual = pObjectShell->GetDocumentSignatureState();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        (OString::number(
             static_cast<std::underlying_type<SignatureState>::type>(nActual))
         .getStr()),
        nActual, SignatureState::OK);
}

void SigningTest::testODFUntrustedGoodGPG()
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        (OString::number(
             static_cast<std::underlying_type<SignatureState>::type>(nActual))
         .getStr()),
        nActual, SignatureState::NOTVALIDATED);
}

void SigningTest::testODFBrokenStreamGPG()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "badStreamGPG.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN), static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

void SigningTest::testODFBrokenDsigGPG()
{
    createDoc(m_directories.getURLFromSrc(DATA_DIRECTORY) + "badDsigGPG.odt");
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT(pObjectShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SignatureState::BROKEN), static_cast<int>(pObjectShell->GetDocumentSignatureState()));
}

#if HAVE_GPGCONF_SOCKETDIR

void SigningTest::testODFEncryptedGPG()
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
void SigningTest::testPreserveMacroTemplateSignature12()
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
    mxComponent->dispose();
    mxComponent = mxDesktop->loadComponentFromURL(aURL, "_default", 0,
                                                  uno::Sequence<beans::PropertyValue>(0));
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
void SigningTest::testDropMacroTemplateSignature()
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
    mxComponent->dispose();
    mxComponent = mxDesktop->loadComponentFromURL(aURL, "_default", 0,
                                                  uno::Sequence<beans::PropertyValue>(0));
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
void SigningTest::testPreserveMacroTemplateSignature10()
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
    mxComponent->dispose();
    mxComponent = mxDesktop->loadComponentFromURL(aURL, "_default", 0,
                                                  uno::Sequence<beans::PropertyValue>(0));
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

void SigningTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("odfds"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:digitalsignature:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dsig"), BAD_CAST("http://www.w3.org/2000/09/xmldsig#"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xd"), BAD_CAST("http://uri.etsi.org/01903/v1.3.2#"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SigningTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
