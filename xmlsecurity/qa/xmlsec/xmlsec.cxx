/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_crypto.h>

#if USE_CRYPTO_NSS
#include <secoid.h>
#include <nss.h>
#endif

#include <test/unoapi_test.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/storagehelper.hxx>
#include <sfx2/lokhelper.hxx>
#include <svl/cryptosign.hxx>

#include <documentsignaturemanager.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers xmlsecurity/source/xmlsec/ fixes.
class Test : public UnoApiTest
{
protected:
    uno::Reference<xml::crypto::XSEInitializer> mxSEInitializer;
    uno::Reference<xml::crypto::XXMLSecurityContext> mxSecurityContext;

public:
    Test()
        : UnoApiTest("/xmlsecurity/qa/xmlsec/data/")
    {
    }

    void setUp() override
    {
        UnoApiTest::setUp();
        MacrosTest::setUpX509(m_directories, "xmlsecurity_xmlsec");

        // Initialize crypto after setting up the environment variables.
        mxSEInitializer = xml::crypto::SEInitializer::create(m_xContext);
        mxSecurityContext = mxSEInitializer->createSecurityContext(OUString());
#if USE_CRYPTO_NSS
#ifdef NSS_USE_ALG_IN_SIGNATURE
        // policy may disallow using SHA1 for signatures but unit test documents
        // have such existing signatures (call this after createSecurityContext!)
        NSS_SetAlgorithmPolicy(SEC_OID_SHA1, NSS_USE_ALG_IN_SIGNATURE, 0);
        // the minimum is 2048 in Fedora 40
        NSS_OptionSet(NSS_RSA_MIN_KEY_SIZE, 1024);
#endif
#endif
    }
};

OString ReadToString(const OUString& rUrl)
{
    SvFileStream aStream(rUrl, StreamMode::READ);
    return read_uInt8s_ToOString(aStream, aStream.remainingSize());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertPrivateKey)
{
    // Given a view that has CA/cert/key data associated:
    loadFromURL("private:factory/swriter");
    save("writer8");
    DocumentSignatureManager aManager(getComponentContext(), DocumentSignatureMode::Content);
    CPPUNIT_ASSERT(aManager.init());
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.setStore(xStorage);
    aManager.getSignatureHelper().SetStorage(xStorage, u"1.2");
    OUString aCaPath = createFileURL(u"ca.pem");
    std::string aCa;
    aCa = ReadToString(aCaPath);
    std::vector<std::string> aCerts = SfxLokHelper::extractCertificates(aCa);
    SfxLokHelper::addCertificates(aCerts);
    OUString aCertPath = createFileURL(u"cert.pem");
    std::string aCert;
    aCert = ReadToString(aCertPath);
    OUString aKeyPath;
    aKeyPath = createFileURL(u"key.pem");
    std::string aKey;
    aKey = ReadToString(aKeyPath);
    uno::Reference<security::XCertificate> xCertificate
        = SfxLokHelper::getSigningCertificate(aCert, aKey);
    CPPUNIT_ASSERT(xCertificate.is());

    // When getting the certificate flags and signing:
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment
        = mxSecurityContext->getSecurityEnvironment();
    // Get the certificate flags, the certificate chooser dialog does this:
    xSecurityEnvironment->getCertificateCharacters(xCertificate);
    OUString aDescription;
    sal_Int32 nSecurityId;
    svl::crypto::SigningContext aSigningContext;
    aSigningContext.m_xCertificate = xCertificate;
    CPPUNIT_ASSERT(
        aManager.add(aSigningContext, mxSecurityContext, aDescription, nSecurityId, false));

    // Then make sure that signing succeeds:
    aManager.read(/*bUseTempStream=*/true);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0 (UNKNOWN)
    // - Actual  : 1 (OPERATION_SUCCEEDED)
    // i.e. the signing failed with an incorrectly imported private key.
    CPPUNIT_ASSERT_EQUAL(
        xml::crypto::SecurityOperationStatus::SecurityOperationStatus_OPERATION_SUCCEEDED,
        rInformations[0].nStatus);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
