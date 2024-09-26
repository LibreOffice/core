/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/storagehelper.hxx>
#include <sfx2/lokhelper.hxx>

#include <documentsignaturemanager.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers xmlsecurity/source/xmlsec/ fixes.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/xmlsecurity/qa/xmlsec/data/")
    {
    }

    void setUp() override
    {
        UnoApiTest::setUp();
        MacrosTest::setUpX509(m_directories, "xmlsecurity_xmlsec");
    }
};

OString ReadToString(const OUString& rUrl)
{
    SvFileStream aStream(rUrl, StreamMode::READ);
    return read_uInt8s_ToOString(aStream, aStream.remainingSize());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertPrivateKey)
{
    // Given a view that has CA/cert/key data data associated:
    uno::Reference<xml::crypto::XSEInitializer> mxSEInitializer
        = xml::crypto::SEInitializer::create(getComponentContext());
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext
        = mxSEInitializer->createSecurityContext(OUString());
    load("private:factory/swriter");
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
        = xSecurityContext->getSecurityEnvironment();
    // Get the certificate flags, the certificate chooser dialog does this:
    xSecurityEnvironment->getCertificateCharacters(xCertificate);
    sal_Int32 nSecurityId;
    CPPUNIT_ASSERT(aManager.add(xCertificate, xSecurityContext, OUString(), nSecurityId, false));

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
