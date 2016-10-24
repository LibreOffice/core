/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <test/bootstrapfixture.hxx>

#include <pdfio/pdfdocument.hxx>

using namespace com::sun::star;

namespace
{
const char* DATA_DIRECTORY = "/xmlsecurity/qa/unit/pdfsigning/data/";
}

/// Testsuite for the PDF signing feature.
class PDFSigningTest : public test::BootstrapFixture
{
    uno::Reference<uno::XComponentContext> mxComponentContext;

public:
    PDFSigningTest();
    void setUp() override;

    /// Test adding a new signature to a previously unsigned file.
    void testPDFAdd();

    CPPUNIT_TEST_SUITE(PDFSigningTest);
    CPPUNIT_TEST(testPDFAdd);
    CPPUNIT_TEST_SUITE_END();
};

PDFSigningTest::PDFSigningTest()
{
}

void PDFSigningTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));

#ifndef _WIN32
    // Set up cert8.db and key3.db in workdir/CppunitTest/
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aTargetDir = m_directories.getURLFromWorkdir(
                              "/CppunitTest/xmlsecurity_signing.test.user/");
    osl::File::copy(aSourceDir + "cert8.db", aTargetDir + "cert8.db");
    osl::File::copy(aSourceDir + "key3.db", aTargetDir + "key3.db");
    OUString aTargetPath;
    osl::FileBase::getSystemPathFromFileURL(aTargetDir, aTargetPath);
    setenv("MOZILLA_CERTIFICATE_FOLDER", aTargetPath.toUtf8().getStr(), 1);
#endif
}

void PDFSigningTest::testPDFAdd()
{
#ifndef _WIN32
    // Make sure that no.pdf has no signatures.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    xmlsecurity::pdfio::PDFDocument aDocument;
    {
        OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
        OUString aInURL = aSourceDir + "no.pdf";
        SvFileStream aStream(aInURL, StreamMode::READ);
        CPPUNIT_ASSERT(aDocument.Read(aStream));
        std::vector<xmlsecurity::pdfio::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
        CPPUNIT_ASSERT(aSignatures.empty());
    }

    // Sign it and write out the result as add.pdf.
    OUString aTargetDir = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_signing.test.user/");
    OUString aOutURL = aTargetDir + "add.pdf";
    {
        uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = xSecurityContext->getSecurityEnvironment();
        uno::Sequence<uno::Reference<security::XCertificate>> aCertificates = xSecurityEnvironment->getPersonalCertificates();
        if (!aCertificates.hasElements())
        {
            // NSS failed to parse it's own profile.
            return;
        }
        CPPUNIT_ASSERT(aDocument.Sign(aCertificates[0], "test"));
        SvFileStream aOutStream(aOutURL, StreamMode::WRITE | StreamMode::TRUNC);
        CPPUNIT_ASSERT(aDocument.Write(aOutStream));
    }

    // Read back the signed pdf and make sure that it has one valid signature.
    {
        SvFileStream aStream(aOutURL, StreamMode::READ);
        xmlsecurity::pdfio::PDFDocument aVerifyDocument;
        CPPUNIT_ASSERT(aVerifyDocument.Read(aStream));
        std::vector<xmlsecurity::pdfio::PDFObjectElement*> aSignatures = aVerifyDocument.GetSignatureWidgets();
        // This was 0 when PDFDocument::Sign() silently returned success, without doing anything.
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aSignatures.size());
        SignatureInformation aInfo(0);
        CPPUNIT_ASSERT(xmlsecurity::pdfio::PDFDocument::ValidateSignature(aStream, aSignatures[0], aInfo));
    }
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(PDFSigningTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
