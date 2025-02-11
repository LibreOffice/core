/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _WIN32
#include <secoid.h>
#endif

#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <test/bootstrapfixture.hxx>
#include <tools/datetime.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <documentsignaturemanager.hxx>
#include <xmlsecurity/pdfio/pdfdocument.hxx>

#ifdef _WIN32
 #define WIN32_LEAN_AND_MEAN
 #include <windows.h>
#endif

using namespace com::sun::star;

namespace
{
const char* const DATA_DIRECTORY = "/xmlsecurity/qa/unit/pdfsigning/data/";
}

/// Testsuite for the PDF signing feature.
class PDFSigningTest : public test::BootstrapFixture
{
    uno::Reference<uno::XComponentContext> mxComponentContext;

    /**
     * Sign rInURL once and save the result as rOutURL, asserting that rInURL
     * had nOriginalSignatureCount signatures.
     */
    bool sign(const OUString& rInURL, const OUString& rOutURL, size_t nOriginalSignatureCount);
    /**
     * Read a pdf and make sure that it has the expected number of valid
     * signatures.
     */
    std::vector<SignatureInformation> verify(const OUString& rURL, size_t nCount, const OString& rExpectedSubFilter);

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
    /// Test a PDF 1.4 document, signed by Adobe.
    void testPDF14Adobe();
    /// Test a PDF 1.6 document, signed by Adobe.
    void testPDF16Adobe();
    /// Test adding a signature to a PDF 1.6 document.
    void testPDF16Add();
    /// Test a PDF 1.4 document, signed by LO on Windows.
    void testPDF14LOWin();
    /// Test a PAdES document, signed by LO on Linux.
    void testPDFPAdESGood();
    /// Test a valid signature that does not cover the whole file.
    void testPartial();
    void testBadCertP1();
    void testBadCertP3Stamp();
    void testPartialInBetween();
    /// Test writing a PAdES signature.
    void testSigningCertificateAttribute();
    /// Test that we accept files which are supposed to be good.
    void testGood();
    /// Test that we don't crash / loop while tokenizing these files.
    void testTokenize();
    /// Test handling of unknown SubFilter values.
    void testUnknownSubFilter();
    void testTdf107782();

    CPPUNIT_TEST_SUITE(PDFSigningTest);
    CPPUNIT_TEST(testPDFAdd);
    CPPUNIT_TEST(testPDFAdd2);
    CPPUNIT_TEST(testPDFRemove);
    CPPUNIT_TEST(testPDFRemoveAll);
    CPPUNIT_TEST(testPDF14Adobe);
    CPPUNIT_TEST(testPDF16Adobe);
    CPPUNIT_TEST(testPDF16Add);
    CPPUNIT_TEST(testPDF14LOWin);
    CPPUNIT_TEST(testPDFPAdESGood);
    CPPUNIT_TEST(testPartial);
    CPPUNIT_TEST(testBadCertP1);
    CPPUNIT_TEST(testBadCertP3Stamp);
    CPPUNIT_TEST(testPartialInBetween);
    CPPUNIT_TEST(testSigningCertificateAttribute);
    CPPUNIT_TEST(testGood);
    CPPUNIT_TEST(testTokenize);
    CPPUNIT_TEST(testUnknownSubFilter);
    CPPUNIT_TEST(testTdf107782);
    CPPUNIT_TEST_SUITE_END();
};

PDFSigningTest::PDFSigningTest()
{
}

void PDFSigningTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));

    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aTargetDir = m_directories.getURLFromWorkdir(
                              "/CppunitTest/xmlsecurity_pdfsigning.test.user/");
    OUString aTargetPath;
    osl::FileBase::getSystemPathFromFileURL(aTargetDir, aTargetPath);

#ifdef _WIN32
    // CryptoAPI test certificates
    osl::File::copy(aSourceDir + "test.p7b", aTargetDir + "test.p7b");
    OUString caVar("LIBO_TEST_CRYPTOAPI_PKCS7");
    osl_setEnvironment(caVar.pData, aTargetPath.pData);
#else
    // Set up NSS database in workdir/CppunitTest/
    osl::File::copy(aSourceDir + "cert9.db", aTargetDir + "cert9.db");
    osl::File::copy(aSourceDir + "key4.db", aTargetDir + "key4.db");
    osl::File::copy(aSourceDir + "pkcs11.txt", aTargetDir + "pkcs11.txt");
    setenv("MOZILLA_CERTIFICATE_FOLDER", aTargetPath.toUtf8().getStr(), 1);
#endif

    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer
        = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext
        = xSEInitializer->createSecurityContext(OUString());
#ifndef _WIN32
#ifdef NSS_USE_ALG_IN_ANY_SIGNATURE
    // policy may disallow using SHA1 for signatures but unit test documents
    // have such existing signatures (call this after createSecurityContext!)
    NSS_SetAlgorithmPolicy(SEC_OID_SHA1, NSS_USE_ALG_IN_ANY_SIGNATURE, 0);
#endif
#endif
}

std::vector<SignatureInformation> PDFSigningTest::verify(const OUString& rURL, size_t nCount, const OString& rExpectedSubFilter)
{
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    std::vector<SignatureInformation> aRet;

    SvFileStream aStream(rURL, StreamMode::READ);
    vcl::filter::PDFDocument aVerifyDocument;
    CPPUNIT_ASSERT(aVerifyDocument.Read(aStream));
    std::vector<vcl::filter::PDFObjectElement*> aSignatures = aVerifyDocument.GetSignatureWidgets();
    CPPUNIT_ASSERT_EQUAL(nCount, aSignatures.size());
    for (size_t i = 0; i < aSignatures.size(); ++i)
    {
        SignatureInformation aInfo(i);
        int nMDPPerm = aVerifyDocument.GetMDPPerm();
        xmlsecurity::pdfio::ValidateSignature(aStream, aSignatures[i], aInfo, aVerifyDocument,
                                              nMDPPerm);
        aRet.push_back(aInfo);

        if (!rExpectedSubFilter.isEmpty())
        {
            vcl::filter::PDFObjectElement* pValue = aSignatures[i]->LookupObject("V");
            CPPUNIT_ASSERT(pValue);
            auto pSubFilter = dynamic_cast<vcl::filter::PDFNameElement*>(pValue->Lookup("SubFilter"));
            CPPUNIT_ASSERT(pSubFilter);
            CPPUNIT_ASSERT_EQUAL(rExpectedSubFilter, pSubFilter->GetValue());
        }
    }

    return aRet;
}

bool PDFSigningTest::sign(const OUString& rInURL, const OUString& rOutURL, size_t nOriginalSignatureCount)
{
    // Make sure that input has nOriginalSignatureCount signatures.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    vcl::filter::PDFDocument aDocument;
    {
        SvFileStream aStream(rInURL, StreamMode::READ);
        CPPUNIT_ASSERT(aDocument.Read(aStream));
        std::vector<vcl::filter::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
        CPPUNIT_ASSERT_EQUAL(nOriginalSignatureCount, aSignatures.size());
    }

    bool bSignSuccessful = false;
    // Sign it and write out the result.
    {
        uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = xSecurityContext->getSecurityEnvironment();
        uno::Sequence<uno::Reference<security::XCertificate>> aCertificates = xSecurityEnvironment->getPersonalCertificates();
        DateTime now(DateTime::SYSTEM);
        for (auto& cert : aCertificates)
        {
            css::util::DateTime aNotValidAfter = cert->getNotValidAfter();
            css::util::DateTime aNotValidBefore = cert->getNotValidBefore();

            // Only try certificates that are already active and not expired
            if ((now > aNotValidAfter) || (now < aNotValidBefore))
            {
                SAL_WARN("xmlsecurity.pdfio.test", "Skipping a certificate that is not yet valid or already not valid");
            }
            else
            {
                bool bSignResult = aDocument.Sign(cert, "test", /*bAdES=*/true);
#ifdef _WIN32
                if (!bSignResult)
                {
                    DWORD dwErr = GetLastError();
                    if (dwErr == CRYPT_E_NO_KEY_PROPERTY)
                    {
                        SAL_WARN("xmlsecurity.pdfio.test", "Skipping a certificate without a private key");
                        continue; // The certificate does not have a private key - not a valid certificate
                    }
                }
#endif
                CPPUNIT_ASSERT(bSignResult);
                SvFileStream aOutStream(rOutURL, StreamMode::WRITE | StreamMode::TRUNC);
                CPPUNIT_ASSERT(aDocument.Write(aOutStream));
                bSignSuccessful = true;
                break;
            }
        }
    }

    // This was nOriginalSignatureCount when PDFDocument::Sign() silently returned success, without doing anything.
    if (bSignSuccessful)
        verify(rOutURL, nOriginalSignatureCount + 1, /*rExpectedSubFilter=*/OString());

    // May return false if NSS failed to parse it's own profile or Windows has no valid certificates installed.
    return bSignSuccessful;
}

void PDFSigningTest::testPDFAdd()
{
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aInURL = aSourceDir + "no.pdf";
    OUString aTargetDir = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
    OUString aOutURL = aTargetDir + "add.pdf";
    bool bHadCertificates = sign(aInURL, aOutURL, 0);

    if (bHadCertificates)
    {
        // Assert that the SubFilter is not adbe.pkcs7.detached in the bAdES case.
        std::vector<SignatureInformation> aInfos = verify(aOutURL, 1, "ETSI.CAdES.detached");
        // Make sure the timestamp is correct.
        DateTime aDateTime(DateTime::SYSTEM);
        // This was 0 (on Windows), as neither the /M key nor the PKCS#7 blob contained a timestamp.
        CPPUNIT_ASSERT_EQUAL(aDateTime.GetYear(), aInfos[0].stDateTime.Year);
        // Assert that the digest algorithm is not SHA-1 in the bAdES case.
        CPPUNIT_ASSERT_EQUAL(xml::crypto::DigestID::SHA256, aInfos[0].nDigestID);
    }
}

void PDFSigningTest::testPDFAdd2()
{
    // Sign.
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aInURL = aSourceDir + "no.pdf";
    OUString aTargetDir = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
    OUString aOutURL = aTargetDir + "add.pdf";
    bool bHadCertificates = sign(aInURL, aOutURL, 0);

    // Sign again.
    aInURL = aTargetDir + "add.pdf";
    aOutURL = aTargetDir + "add2.pdf";
    // This failed with "second range end is not the end of the file" for the
    // first signature.
    if (bHadCertificates)
        sign(aInURL, aOutURL, 1);
}

void PDFSigningTest::testPDFRemove()
{
    // Make sure that good.pdf has 1 valid signature.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    vcl::filter::PDFDocument aDocument;
    {
        OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
        OUString aInURL = aSourceDir + "good.pdf";
        SvFileStream aStream(aInURL, StreamMode::READ);
        CPPUNIT_ASSERT(aDocument.Read(aStream));
        std::vector<vcl::filter::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aSignatures.size());
        SignatureInformation aInfo(0);
        int nMDPPerm = aDocument.GetMDPPerm();
        CPPUNIT_ASSERT(xmlsecurity::pdfio::ValidateSignature(aStream, aSignatures[0], aInfo,
                                                             aDocument, nMDPPerm));
    }

    // Remove the signature and write out the result as remove.pdf.
    OUString aTargetDir = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
    OUString aOutURL = aTargetDir + "remove.pdf";
    {
        CPPUNIT_ASSERT(aDocument.RemoveSignature(0));
        SvFileStream aOutStream(aOutURL, StreamMode::WRITE | StreamMode::TRUNC);
        CPPUNIT_ASSERT(aDocument.Write(aOutStream));
    }

    // Read back the pdf and make sure that it no longer has signatures.
    // This failed when PDFDocument::RemoveSignature() silently returned
    // success, without doing anything.
    verify(aOutURL, 0, /*rExpectedSubFilter=*/OString());
}

void PDFSigningTest::testPDFRemoveAll()
{
    // Make sure that good2.pdf has 2 valid signatures.  Unlike in
    // testPDFRemove(), here intentionally test DocumentSignatureManager and
    // PDFSignatureHelper code as well.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());

    // Copy the test document to a temporary file, as it'll be modified.
    OUString aTargetDir = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
    OUString aOutURL = aTargetDir + "remove-all.pdf";
    CPPUNIT_ASSERT_EQUAL(osl::File::RC::E_None, osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY) + "2good.pdf", aOutURL));
    // Load the test document as a storage and read its two signatures.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
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
}

void PDFSigningTest::testTdf107782()
{
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());

    // Load the test document as a storage and read its signatures.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf107782.pdf";
    SvStream* pStream = utl::UcbStreamHelper::CreateStream(aURL, StreamMode::READ | StreamMode::WRITE);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
    aManager.mxSignatureStream = xStream;
    aManager.read(/*bUseTempStream=*/false);
    CPPUNIT_ASSERT(aManager.mpPDFSignatureHelper);

    // This failed with an std::bad_alloc exception on Windows.
    aManager.mpPDFSignatureHelper->GetDocumentSignatureInformations(aManager.getSecurityEnvironment());
}

void PDFSigningTest::testPDF14Adobe()
{
    // Two signatures, first is SHA1, the second is SHA256.
    // This was 0, as we failed to find the Annots key's value when it was a
    // reference-to-array, not an array.
    std::vector<SignatureInformation> aInfos = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "pdf14adobe.pdf", 2, /*rExpectedSubFilter=*/OString());
    // This was 0, out-of-PKCS#7 signature date wasn't read.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2016), aInfos[1].stDateTime.Year);
}

void PDFSigningTest::testPDF16Adobe()
{
    // Contains a cross-reference stream, object streams and a compressed
    // stream with a predictor. And a valid signature.
    // Found signatures was 0, as parsing failed due to lack of support for
    // these features.
    verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "pdf16adobe.pdf", 1, /*rExpectedSubFilter=*/OString());
}

void PDFSigningTest::testPDF16Add()
{
    // Contains PDF 1.6 features, make sure we can add a signature using that
    // markup correctly.
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aInURL = aSourceDir + "pdf16adobe.pdf";
    OUString aTargetDir = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
    OUString aOutURL = aTargetDir + "add.pdf";
    // This failed: verification broke as incorrect xref stream was written as
    // part of the new signature.
    bool bHadCertificates = sign(aInURL, aOutURL, 1);

    // Sign again.
    aInURL = aTargetDir + "add.pdf";
    aOutURL = aTargetDir + "add2.pdf";
    // This failed as non-compressed AcroForm wasn't handled.
    if (bHadCertificates)
        sign(aInURL, aOutURL, 2);
}

void PDFSigningTest::testPDF14LOWin()
{
    // mscrypto used SEC_OID_PKCS1_SHA1_WITH_RSA_ENCRYPTION as a digest
    // algorithm when it meant SEC_OID_SHA1, make sure we tolerate that on all
    // platforms.
    // This failed, as NSS HASH_Create() didn't handle the sign algorithm.
    verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "pdf14lowin.pdf", 1, /*rExpectedSubFilter=*/OString());
}

void PDFSigningTest::testPDFPAdESGood()
{
    verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "good-pades.pdf", 1, "ETSI.CAdES.detached");
}

void PDFSigningTest::testPartial()
{
    std::vector<SignatureInformation> aInfos = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "partial.pdf", 1, /*rExpectedSubFilter=*/OString());
    CPPUNIT_ASSERT(!aInfos.empty());
    SignatureInformation& rInformation = aInfos[0];
    CPPUNIT_ASSERT(rInformation.bPartialDocumentSignature);
}

void PDFSigningTest::testBadCertP1()
{
    std::vector<SignatureInformation> aInfos
        = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "bad-cert-p1.pdf", 1,
                 /*rExpectedSubFilter=*/OString());
    CPPUNIT_ASSERT(!aInfos.empty());
#if HAVE_FEATURE_PDFIUM
    SignatureInformation& rInformation = aInfos[0];
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0 (SecurityOperationStatus_UNKNOWN)
    // - Actual  : 1 (SecurityOperationStatus_OPERATION_SUCCEEDED)
    // i.e. annotation after a P1 signature was not considered as a bad modification.
    CPPUNIT_ASSERT_EQUAL(xml::crypto::SecurityOperationStatus::SecurityOperationStatus_UNKNOWN,
                         rInformation.nStatus);
#endif
}

void PDFSigningTest::testBadCertP3Stamp()
{
    std::vector<SignatureInformation> aInfos
        = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "bad-cert-p3-stamp.pdf", 1,
                 /*rExpectedSubFilter=*/OString());
    CPPUNIT_ASSERT(!aInfos.empty());
#if HAVE_FEATURE_PDFIUM
    SignatureInformation& rInformation = aInfos[0];

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0 (SecurityOperationStatus_UNKNOWN)
    // - Actual  : 1 (SecurityOperationStatus_OPERATION_SUCCEEDED)
    // i.e. adding a stamp annotation was not considered as a bad modification.
    CPPUNIT_ASSERT_EQUAL(xml::crypto::SecurityOperationStatus::SecurityOperationStatus_UNKNOWN,
                         rInformation.nStatus);
#endif
}

/// Test writing a PAdES signature.
void PDFSigningTest::testSigningCertificateAttribute()
{
    // Create a new signature.
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aInURL = aSourceDir + "no.pdf";
    OUString aTargetDir = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
    OUString aOutURL = aTargetDir + "signing-certificate-attribute.pdf";
    bool bHadCertificates = sign(aInURL, aOutURL, 0);
    if (!bHadCertificates)
        return;

#if HAVE_FEATURE_PDFIUM
    // Verify it.
    std::vector<SignatureInformation> aInfos = verify(aOutURL, 1, "ETSI.CAdES.detached");
    CPPUNIT_ASSERT(!aInfos.empty());
    SignatureInformation& rInformation = aInfos[0];
    // Assert that it has a signed signingCertificateV2 attribute.
    CPPUNIT_ASSERT(rInformation.bHasSigningCertificate);
#endif
}

void PDFSigningTest::testGood()
{
    const std::initializer_list<OUStringLiteral> aNames =
    {
        // We failed to determine if this is good or bad.
        "good-non-detached.pdf",
        // Boolean value for dictionary key caused read error.
        "dict-bool.pdf",
    };

#if HAVE_FEATURE_PDFIUM
    for (const auto& rName : aNames)
    {
        std::vector<SignatureInformation> aInfos = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + rName, 1, /*rExpectedSubFilter=*/OString());
        CPPUNIT_ASSERT(!aInfos.empty());
        SignatureInformation& rInformation = aInfos[0];
        CPPUNIT_ASSERT_EQUAL((int)xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED, (int)rInformation.nStatus);
    }
#endif
}

void PDFSigningTest::testTokenize()
{
    const std::initializer_list<OUStringLiteral> aNames =
    {
        // We looped on this broken input.
        "no-eof.pdf",
        // ']' in a name token was mishandled.
        "name-bracket.pdf",
        // %%EOF at the end wasn't followed by a newline.
        "noeol.pdf",
        // File that's intentionally smaller than 1024 bytes.
        "small.pdf",
        "tdf107149.pdf",
        // Nested parentheses were not handled.
        "tdf114460.pdf",
        // Valgrind was unhappy about this.
        "forcepoint16.pdf",
    };

    for (const auto& rName : aNames)
    {
        SvFileStream aStream(m_directories.getURLFromSrc(DATA_DIRECTORY) + rName, StreamMode::READ);
        vcl::filter::PDFDocument aDocument;
        // Just make sure the tokenizer finishes without an error, don't look at the signature.
        CPPUNIT_ASSERT(aDocument.Read(aStream));

        OUString aNoPages("tdf107149.pdf");
        if (aNoPages == rName)
            // This failed, page list was empty.
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aDocument.GetPages().size());
    }
}

void PDFSigningTest::testUnknownSubFilter()
{
    // Tokenize the bugdoc.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    SvStream* pStream = utl::UcbStreamHelper::CreateStream(m_directories.getURLFromSrc(DATA_DIRECTORY) + "cr-comment.pdf", StreamMode::READ | StreamMode::WRITE);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    aManager.mxSignatureStream = xStream;
    aManager.read(/*bUseTempStream=*/false);

    // Make sure we find both signatures, even if the second has unknown SubFilter.
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), rInformations.size());
}

void PDFSigningTest::testPartialInBetween()
{
    std::vector<SignatureInformation> aInfos
        = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "partial-in-between.pdf", 2,
                 /*rExpectedSubFilter=*/OString());
    CPPUNIT_ASSERT(!aInfos.empty());
    SignatureInformation& rInformation = aInfos[0];
    // Without the accompanying fix in place, this test would have failed, as unsigned incremental
    // update between two signatures were not detected.
    CPPUNIT_ASSERT(rInformation.bPartialDocumentSignature);
}

CPPUNIT_TEST_SUITE_REGISTRATION(PDFSigningTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
