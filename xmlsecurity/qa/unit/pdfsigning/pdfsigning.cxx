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
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <documentsignaturemanager.hxx>
#include <pdfio/pdfdocument.hxx>

using namespace com::sun::star;

#if !defined _WIN32
namespace
{
const char* DATA_DIRECTORY = "/xmlsecurity/qa/unit/pdfsigning/data/";
}
#endif

/// Testsuite for the PDF signing feature.
class PDFSigningTest : public test::BootstrapFixture
{
    uno::Reference<uno::XComponentContext> mxComponentContext;

    /**
     * Sign rInURL once and save the result as rOutURL, asserting that rInURL
     * had nOriginalSignatureCount signatures.
     */
    void sign(const OUString& rInURL, const OUString& rOutURL, size_t nOriginalSignatureCount);
public:
    PDFSigningTest();
    void setUp() override;

    /// Test adding a new signature to a previously unsigned file.
    void testPDFAdd();
    /// Test signing a previously unsigned file twice.
    void testPDFAdd2();
    /// Test removing a signature from a previously signed file.
    void testPDFRemove();
    /// Test removing all signatures from a previously multi-signed file.
    void testPDFRemoveAll();

    CPPUNIT_TEST_SUITE(PDFSigningTest);
    CPPUNIT_TEST(testPDFAdd);
    CPPUNIT_TEST(testPDFAdd2);
    CPPUNIT_TEST(testPDFRemove);
    CPPUNIT_TEST(testPDFRemoveAll);
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

void PDFSigningTest::sign(const OUString& rInURL, const OUString& rOutURL, size_t nOriginalSignatureCount)
{
    // Make sure that input has nOriginalSignatureCount signatures.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    xmlsecurity::pdfio::PDFDocument aDocument;
    {
        SvFileStream aStream(rInURL, StreamMode::READ);
        CPPUNIT_ASSERT(aDocument.Read(aStream));
        std::vector<xmlsecurity::pdfio::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
        CPPUNIT_ASSERT_EQUAL(nOriginalSignatureCount, aSignatures.size());
    }

    // Sign it and write out the result.
    {
        uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = xSecurityContext->getSecurityEnvironment();
        uno::Sequence<uno::Reference<security::XCertificate>> aCertificates = xSecurityEnvironment->getPersonalCertificates();
        if (!aCertificates.hasElements())
        {
            // NSS failed to parse it's own profile.
            return;
        }
        CPPUNIT_ASSERT(aDocument.Sign(aCertificates[0], "test"));
        SvFileStream aOutStream(rOutURL, StreamMode::WRITE | StreamMode::TRUNC);
        CPPUNIT_ASSERT(aDocument.Write(aOutStream));
    }

    // Read back the signed pdf and make sure that it has one valid signature.
    {
        SvFileStream aStream(rOutURL, StreamMode::READ);
        xmlsecurity::pdfio::PDFDocument aVerifyDocument;
        CPPUNIT_ASSERT(aVerifyDocument.Read(aStream));
        std::vector<xmlsecurity::pdfio::PDFObjectElement*> aSignatures = aVerifyDocument.GetSignatureWidgets();
        // This was nOriginalSignatureCount when PDFDocument::Sign() silently returned success, without doing anything.
        CPPUNIT_ASSERT_EQUAL(nOriginalSignatureCount + 1, aSignatures.size());
        for (size_t i = 0; i < aSignatures.size(); ++i)
        {
            SignatureInformation aInfo(i);
            bool bLast = i == aSignatures.size() - 1;
            CPPUNIT_ASSERT(xmlsecurity::pdfio::PDFDocument::ValidateSignature(aStream, aSignatures[i], aInfo, bLast));
        }
    }
}

void PDFSigningTest::testPDFAdd()
{
#ifndef _WIN32
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aInURL = aSourceDir + "no.pdf";
    OUString aTargetDir = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_signing.test.user/");
    OUString aOutURL = aTargetDir + "add.pdf";
    sign(aInURL, aOutURL, 0);
#endif
}

void PDFSigningTest::testPDFAdd2()
{
#ifndef _WIN32
    // Sign.
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aInURL = aSourceDir + "no.pdf";
    OUString aTargetDir = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_signing.test.user/");
    OUString aOutURL = aTargetDir + "add.pdf";
    sign(aInURL, aOutURL, 0);

    // Sign again.
    aInURL = aTargetDir + "add.pdf";
    aOutURL = aTargetDir + "add2.pdf";
    // This failed with "second range end is not the end of the file" for the
    // first signature.
    sign(aInURL, aOutURL, 1);
#endif
}

void PDFSigningTest::testPDFRemove()
{
#ifndef _WIN32
    // Make sure that good.pdf has 1 valid signature.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    xmlsecurity::pdfio::PDFDocument aDocument;
    {
        OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
        OUString aInURL = aSourceDir + "good.pdf";
        SvFileStream aStream(aInURL, StreamMode::READ);
        CPPUNIT_ASSERT(aDocument.Read(aStream));
        std::vector<xmlsecurity::pdfio::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aSignatures.size());
        SignatureInformation aInfo(0);
        CPPUNIT_ASSERT(xmlsecurity::pdfio::PDFDocument::ValidateSignature(aStream, aSignatures[0], aInfo, /*bLast=*/true));
    }

    // Remove the signature and write out the result as remove.pdf.
    OUString aTargetDir = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_signing.test.user/");
    OUString aOutURL = aTargetDir + "remove.pdf";
    {
        CPPUNIT_ASSERT(aDocument.RemoveSignature(0));
        SvFileStream aOutStream(aOutURL, StreamMode::WRITE | StreamMode::TRUNC);
        CPPUNIT_ASSERT(aDocument.Write(aOutStream));
    }

    // Read back the pdf and make sure that it no longer has signatures.
    {
        SvFileStream aStream(aOutURL, StreamMode::READ);
        xmlsecurity::pdfio::PDFDocument aVerifyDocument;
        CPPUNIT_ASSERT(aVerifyDocument.Read(aStream));
        std::vector<xmlsecurity::pdfio::PDFObjectElement*> aSignatures = aVerifyDocument.GetSignatureWidgets();
        // This failed when PDFDocument::RemoveSignature() silently returned success, without doing anything.
        CPPUNIT_ASSERT(aSignatures.empty());
    }
#endif
}

void PDFSigningTest::testPDFRemoveAll()
{
#ifndef _WIN32
    // Make sure that good2.pdf has 2 valid signatures.  Unlike in
    // testPDFRemove(), here intentionally test DocumentSignatureManager and
    // PDFSignatureHelper code as well.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());

    // Copy the test document to a temporary file, as it'll be modified.
    OUString aTargetDir = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_signing.test.user/");
    OUString aOutURL = aTargetDir + "remove-all.pdf";
    CPPUNIT_ASSERT_EQUAL(osl::File::RC::E_None, osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY) + "2good.pdf", aOutURL));
    // Load the test document as a storage and read its two signatures.
    DocumentSignatureManager aManager(mxComponentContext, SignatureModeDocumentContent);
    SvStream* pStream = utl::UcbStreamHelper::CreateStream(aOutURL, StreamMode::READ | StreamMode::WRITE);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
    aManager.mxSignatureStream = xStream;
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    // This was 1 when NSS_CMSSignerInfo_GetSigningCertificate() failed, which
    // means that we only used the locally imported certificates for
    // verification, not the ones provided in the PDF signature data.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), rInformations.size());

    // Request removal of the first signature, should imply removal of the
    // second chained signature as well.
    aManager.remove(0);
    // This was 2, Manager didn't write anything to disk when removal succeeded
    // (instead of doing that when removal failed).
    // Then this was 1, when the chained signature wasn't removed.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), rInformations.size());
#endif
}
CPPUNIT_TEST_SUITE_REGISTRATION(PDFSigningTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
