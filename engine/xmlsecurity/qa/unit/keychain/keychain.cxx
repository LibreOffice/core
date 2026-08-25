/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#ifdef MACOSX

#include <config_crypto.h>

#if USE_CRYPTO_NSS
#include <secoid.h>
#include <nss.h>
#endif

#include <test/unoapixml_test.hxx>

#include <premac.h>
#include <Security/Security.h>
#include <postmac.h>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/security/CertificateCharacters.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/storagehelper.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

#include <documentsignaturemanager.hxx>
#include <svl/cryptosign.hxx>
#include <svl/sigstruct.hxx>

using namespace css;

constexpr std::u16string_view TEST_SIGNER_SUBJECT = u"Keychain Test Signer";

/// Testsuite for signing with macOS Keychain-backed keys.
class KeychainSigningTest : public UnoApiXmlTest
{
protected:
    uno::Reference<xml::crypto::XSEInitializer> mxSEInitializer;
    uno::Reference<xml::crypto::XXMLSecurityContext> mxSecurityContext;

public:
    KeychainSigningTest()
        : UnoApiXmlTest(u"/xmlsecurity/qa/unit/keychain/data/"_ustr)
    {
    }

    virtual void setUp() override
    {
        UnoApiXmlTest::setUp();

        MacrosTest::setUpX509(m_directories, u"xmlsecurity_keychain"_ustr);
        setUpKeychain();

        // Initialize crypto after setting up the environment variables.
        mxSEInitializer = xml::crypto::SEInitializer::create(m_xContext);
        mxSecurityContext = mxSEInitializer->createSecurityContext(OUString());
#if USE_CRYPTO_NSS
#ifdef NSS_USE_ALG_IN_SIGNATURE
        // policy may disallow using SHA1 for signatures but the non-XAdES test
        // creates such a signature (call this after createSecurityContext!)
        NSS_SetAlgorithmPolicy(SEC_OID_SHA1, NSS_USE_ALG_IN_SIGNATURE, 0);
        // the minimum is 2048 in Fedora 40
        NSS_OptionSet(NSS_RSA_MIN_KEY_SIZE, 1024);
#endif
#endif
    }

    /// Creates a temporary keychain, imports the test identity, and points
    /// COKIT_TEST_KEYCHAIN at it, making the Keychain lookups hermetic.
    void setUpKeychain()
    {
        // The keychain (like the NSS database) must only be set up once per process.
        static bool bDone = false;
        if (bDone)
            return;
        bDone = true;

        OUString aTargetDir
            = m_directories.getURLFromWorkdir(u"CppunitTest/xmlsecurity_keychain.test.user");
        OUString aKeychainURL = aTargetDir + "/test.keychain";
        osl::File::remove(aKeychainURL);
        OUString aKeychainPath;
        CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None,
                             osl::FileBase::getSystemPathFromFileURL(aKeychainURL, aKeychainPath));
        OString aKeychainPathUtf8 = OUStringToOString(aKeychainPath, RTL_TEXTENCODING_UTF8);

        SAL_WNODEPRECATED_DECLARATIONS_PUSH // SecKeychainCreate etc. (macOS 10.10)
            SecKeychainRef pKeychain
            = nullptr;
        constexpr char pPassword[] = "test";
        CPPUNIT_ASSERT_EQUAL(OSStatus(errSecSuccess),
                             SecKeychainCreate(aKeychainPathUtf8.getStr(), strlen(pPassword),
                                               pPassword, false, nullptr, &pKeychain));

        // Read the PKCS#12 file with the test identity.
        OUString aP12URL = m_directories.getURLFromSrc(u"/test/signing-keys/keychain-test.p12");
        SvFileStream aP12Stream(aP12URL, StreamMode::READ);
        std::vector<char> aP12Bytes(aP12Stream.remainingSize());
        aP12Stream.ReadBytes(aP12Bytes.data(), aP12Bytes.size());
        CPPUNIT_ASSERT(!aP12Bytes.empty());
        CFDataRef pP12Data
            = CFDataCreate(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(aP12Bytes.data()),
                           aP12Bytes.size());

        // Access object trusting (only) the current process, so signing doesn't prompt.
        SecAccessRef pAccess = nullptr;
        CPPUNIT_ASSERT_EQUAL(OSStatus(errSecSuccess),
                             SecAccessCreate(CFSTR("Collabora Office test"), nullptr, &pAccess));

        const void* pKeys[]
            = { kSecImportExportPassphrase, kSecImportExportKeychain, kSecImportExportAccess };
        const void* pValues[] = { CFSTR("test"), pKeychain, pAccess };
        CFDictionaryRef pOptions
            = CFDictionaryCreate(kCFAllocatorDefault, pKeys, pValues, 3,
                                 &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

        CFArrayRef pItems = nullptr;
        OSStatus nStatus = SecPKCS12Import(pP12Data, pOptions, &pItems);
        SAL_WNODEPRECATED_DECLARATIONS_POP

        if (pItems)
            CFRelease(pItems);
        CFRelease(pOptions);
        CFRelease(pAccess);
        CFRelease(pP12Data);
        CFRelease(pKeychain);
        CPPUNIT_ASSERT_EQUAL(OSStatus(errSecSuccess), nStatus);

        OUString aVar(u"COKIT_TEST_KEYCHAIN"_ustr);
        osl_setEnvironment(aVar.pData, aKeychainPath.pData);
    }

    /// Returns the test certificate provided by the temporary keychain.
    uno::Reference<security::XCertificate>
    getKeychainCertificate(DocumentSignatureManager& rSignatureManager)
    {
        uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment
            = rSignatureManager.getSecurityEnvironment();
        const cpo::uno::Sequence<uno::Reference<security::XCertificate>> aCertificates
            = xSecurityEnvironment->getPersonalCertificates();
        for (const auto& xCertificate : aCertificates)
        {
            if (xCertificate->getSubjectName().indexOf(TEST_SIGNER_SUBJECT) != -1)
                return xCertificate;
        }
        return {};
    }
};

CPPUNIT_TEST_FIXTURE(KeychainSigningTest, testKeychainCertificateListed)
{
    // Given a security environment on top of the test keychain:
    DocumentSignatureManager aManager(m_xContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());

    // The keychain identity has to show up in the personal certificates, marked as having a
    // private key (which lives in the keychain, not in the NSS database).
    uno::Reference<security::XCertificate> xCertificate = getKeychainCertificate(aManager);
    CPPUNIT_ASSERT(xCertificate.is());
    sal_Int32 nCharacters
        = aManager.getSecurityEnvironment()->getCertificateCharacters(xCertificate);
    CPPUNIT_ASSERT(nCharacters & security::CertificateCharacters::HAS_PRIVATE_KEY);
}

CPPUNIT_TEST_FIXTURE(KeychainSigningTest, testODFSignWithKeychain)
{
    // Given an empty ODF document:
    loadFromURL(u"private:factory/swriter"_ustr);
    save(TestFilter::ODT);

    DocumentSignatureManager aManager(m_xContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");

    // When signing with the Keychain identity (XAdES, so SHA-256):
    uno::Reference<security::XCertificate> xCertificate = getKeychainCertificate(aManager);
    CPPUNIT_ASSERT(xCertificate.is());
    sal_Int32 nSecurityId;
    svl::crypto::SigningContext aSigningContext;
    aSigningContext.m_xCertificate = xCertificate;
    aManager.add(aSigningContext, mxSecurityContext, u""_ustr, nSecurityId,
                 /*bAdESCompliant=*/true);

    // Then the signature has to read back as cryptographically valid:
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                         rInformations[0].nStatus);
}

CPPUNIT_TEST_FIXTURE(KeychainSigningTest, testOOXMLSignWithKeychain)
{
    // Given an empty OOXML document:
    loadFromURL(u"private:factory/swriter"_ustr);
    save(TestFilter::DOCX_2007);

    DocumentSignatureManager aManager(m_xContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");

    // When signing with the Keychain identity (non-XAdES, so RSA-SHA1):
    uno::Reference<security::XCertificate> xCertificate = getKeychainCertificate(aManager);
    CPPUNIT_ASSERT(xCertificate.is());
    sal_Int32 nSecurityId;
    svl::crypto::SigningContext aSigningContext;
    aSigningContext.m_xCertificate = xCertificate;
    aManager.add(aSigningContext, mxSecurityContext, u""_ustr, nSecurityId,
                 /*bAdESCompliant=*/false);

    // Then the signature has to read back as cryptographically valid:
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                         rInformations[0].nStatus);
}

CPPUNIT_TEST_FIXTURE(KeychainSigningTest, testPDFSignWithKeychain)
{
    // Given an empty PDF document:
    loadFromURL(u"private:factory/swriter"_ustr);
    save(TestFilter::PDF_WRITER);

    DocumentSignatureManager aManager(m_xContext, DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(
        maTempFile.GetURL(), StreamMode::READ | StreamMode::WRITE));
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
    CPPUNIT_ASSERT(xStream.is());
    aManager.setSignatureStream(xStream);

    // When signing with the Keychain identity (the CMS is produced by the Security framework):
    uno::Reference<security::XCertificate> xCertificate = getKeychainCertificate(aManager);
    CPPUNIT_ASSERT(xCertificate.is());
    sal_Int32 nSecurityId;
    svl::crypto::SigningContext aSigningContext;
    aSigningContext.m_xCertificate = xCertificate;
    aManager.add(aSigningContext, mxSecurityContext, u""_ustr, nSecurityId,
                 /*bAdESCompliant=*/true);

    // Then the signature has to read back as cryptographically valid (verified with NSS):
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    CPPUNIT_ASSERT_EQUAL(css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED,
                         rInformations[0].nStatus);
}

#endif // MACOSX

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
