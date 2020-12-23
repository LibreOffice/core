/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <config_features.h>

#include <string_view>

#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>

#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <tools/datetime.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/filter/pdfdocument.hxx>

#include <documentsignaturemanager.hxx>
#include <pdfsignaturehelper.hxx>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace com::sun::star;

namespace
{
OUStringLiteral const DATA_DIRECTORY = u"/xmlsecurity/qa/unit/pdfsigning/data/";
}

/// Testsuite for the PDF signing feature.
class PDFSigningTest : public test::BootstrapFixture, public unotest::MacrosTest
{
protected:
    /**
     * Sign rInURL once and save the result as rOutURL, asserting that rInURL
     * had nOriginalSignatureCount signatures.
     */
    bool sign(const OUString& rInURL, const OUString& rOutURL, size_t nOriginalSignatureCount);
    /**
     * Read a pdf and make sure that it has the expected number of valid
     * signatures.
     */
    std::vector<SignatureInformation> verify(const OUString& rURL, size_t nCount);

public:
    PDFSigningTest();
    void setUp() override;
    void tearDown() override;
};

PDFSigningTest::PDFSigningTest() {}

void PDFSigningTest::setUp()
{
    test::BootstrapFixture::setUp();
    MacrosTest::setUpNssGpg(m_directories, "xmlsecurity_pdfsigning");
}

void PDFSigningTest::tearDown()
{
    MacrosTest::tearDownNssGpg();
    test::BootstrapFixture::tearDown();
}

std::vector<SignatureInformation> PDFSigningTest::verify(const OUString& rURL, size_t nCount)
{
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer
        = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext
        = xSEInitializer->createSecurityContext(OUString());
    std::vector<SignatureInformation> aRet;

    SvFileStream aStream(rURL, StreamMode::READ);
    PDFSignatureHelper aHelper;
    CPPUNIT_ASSERT(aHelper.ReadAndVerifySignatureSvStream(aStream));

#if HAVE_FEATURE_PDFIUM
    CPPUNIT_ASSERT_EQUAL(nCount, aHelper.GetSignatureInformations().size());
#else
    (void)nCount;
#endif
    for (size_t i = 0; i < aHelper.GetSignatureInformations().size(); ++i)
    {
        const SignatureInformation& rInfo = aHelper.GetSignatureInformations()[i];
        aRet.push_back(rInfo);
    }

    return aRet;
}

bool PDFSigningTest::sign(const OUString& rInURL, const OUString& rOutURL,
                          size_t nOriginalSignatureCount)
{
    // Make sure that input has nOriginalSignatureCount signatures.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer
        = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext
        = xSEInitializer->createSecurityContext(OUString());
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
        uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment
            = xSecurityContext->getSecurityEnvironment();
        uno::Sequence<uno::Reference<security::XCertificate>> aCertificates
            = xSecurityEnvironment->getPersonalCertificates();
        DateTime now(DateTime::SYSTEM);
        for (auto& cert : aCertificates)
        {
            css::util::DateTime aNotValidAfter = cert->getNotValidAfter();
            css::util::DateTime aNotValidBefore = cert->getNotValidBefore();

            // Only try certificates that are already active and not expired
            if ((now > aNotValidAfter) || (now < aNotValidBefore))
            {
                SAL_WARN("xmlsecurity.qa",
                         "Skipping a certificate that is not yet valid or already not valid");
            }
            else
            {
                bool bSignResult = aDocument.Sign(cert, "test", /*bAdES=*/true);
#ifdef _WIN32
                if (!bSignResult)
                {
                    DWORD dwErr = GetLastError();
                    if (HRESULT_FROM_WIN32(dwErr) == CRYPT_E_NO_KEY_PROPERTY)
                    {
                        SAL_WARN("xmlsecurity.qa", "Skipping a certificate without a private key");
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
        verify(rOutURL, nOriginalSignatureCount + 1);

    // May return false if NSS failed to parse its own profile or Windows has no valid certificates installed.
    return bSignSuccessful;
}

#if HAVE_FEATURE_PDFIUM
/// Test adding a new signature to a previously unsigned file.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testPDFAdd)
{
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aInURL = aSourceDir + "no.pdf";
    OUString aTargetDir
        = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
    OUString aOutURL = aTargetDir + "add.pdf";
    bool bHadCertificates = sign(aInURL, aOutURL, 0);

    if (bHadCertificates)
    {
        // Assert that the SubFilter is not adbe.pkcs7.detached in the bAdES case.
        std::vector<SignatureInformation> aInfos = verify(aOutURL, 1);
        CPPUNIT_ASSERT(aInfos[0].bHasSigningCertificate);
        // Make sure the timestamp is correct.
        DateTime aDateTime(DateTime::SYSTEM);
        // This was 0 (on Windows), as neither the /M key nor the PKCS#7 blob contained a timestamp.
        CPPUNIT_ASSERT_EQUAL(aDateTime.GetYear(), aInfos[0].stDateTime.Year);
        // Assert that the digest algorithm is not SHA-1 in the bAdES case.
        CPPUNIT_ASSERT_EQUAL(xml::crypto::DigestID::SHA256, aInfos[0].nDigestID);
    }
}
#endif

/// Test signing a previously unsigned file twice.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testPDFAdd2)
{
    // Sign.
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aInURL = aSourceDir + "no.pdf";
    OUString aTargetDir
        = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
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

#if HAVE_FEATURE_PDFIUM
/// Test removing a signature from a previously signed file.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testPDFRemove)
{
    // Make sure that good.pdf has 1 valid signature.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer
        = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext
        = xSEInitializer->createSecurityContext(OUString());
    vcl::filter::PDFDocument aDocument;
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aInURL = aSourceDir + "good.pdf";
    {
        SvFileStream aStream(aInURL, StreamMode::READ);
        PDFSignatureHelper aHelper;
        aHelper.ReadAndVerifySignatureSvStream(aStream);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aHelper.GetSignatureInformations().size());
    }

    // Remove the signature and write out the result as remove.pdf.
    OUString aTargetDir
        = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
    OUString aOutURL = aTargetDir + "remove.pdf";
    osl::File::copy(aInURL, aOutURL);
    {
        uno::Reference<io::XInputStream> xInputStream;
        std::unique_ptr<SvStream> pStream
            = utl::UcbStreamHelper::CreateStream(aOutURL, StreamMode::READWRITE);
        uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
        xInputStream.set(xStream, uno::UNO_QUERY);
        PDFSignatureHelper::RemoveSignature(xInputStream, 0);
    }

    // Read back the pdf and make sure that it no longer has signatures.
    // This failed when PDFDocument::RemoveSignature() silently returned
    // success, without doing anything.
    verify(aOutURL, 0);
}

/// Test removing all signatures from a previously multi-signed file.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testPDFRemoveAll)
{
    // Make sure that good2.pdf has 2 valid signatures.  Unlike in
    // testPDFRemove(), here intentionally test DocumentSignatureManager and
    // PDFSignatureHelper code as well.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer
        = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext
        = xSEInitializer->createSecurityContext(OUString());

    // Copy the test document to a temporary file, as it'll be modified.
    OUString aTargetDir
        = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
    OUString aOutURL = aTargetDir + "remove-all.pdf";
    CPPUNIT_ASSERT_EQUAL(
        osl::File::RC::E_None,
        osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY) + "2good.pdf", aOutURL));
    // Load the test document as a storage and read its two signatures.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    std::unique_ptr<SvStream> pStream
        = utl::UcbStreamHelper::CreateStream(aOutURL, StreamMode::READ | StreamMode::WRITE);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
    aManager.setSignatureStream(xStream);
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
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

#endif

CPPUNIT_TEST_FIXTURE(PDFSigningTest, testTdf107782)
{
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer
        = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext
        = xSEInitializer->createSecurityContext(OUString());

    // Load the test document as a storage and read its signatures.
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf107782.pdf";
    std::unique_ptr<SvStream> pStream
        = utl::UcbStreamHelper::CreateStream(aURL, StreamMode::READ | StreamMode::WRITE);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
    aManager.setSignatureStream(xStream);
    aManager.read(/*bUseTempStream=*/false);
    CPPUNIT_ASSERT(aManager.hasPDFSignatureHelper());

    // This failed with an std::bad_alloc exception on Windows.
    aManager.getPDFSignatureHelper().GetDocumentSignatureInformations(
        aManager.getSecurityEnvironment());
}

#if HAVE_FEATURE_PDFIUM
/// Test a PDF 1.4 document, signed by Adobe.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testPDF14Adobe)
{
    // Two signatures, first is SHA1, the second is SHA256.
    // This was 0, as we failed to find the Annots key's value when it was a
    // reference-to-array, not an array.
    std::vector<SignatureInformation> aInfos
        = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "pdf14adobe.pdf", 2);
    // This was 0, out-of-PKCS#7 signature date wasn't read.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2016), aInfos[1].stDateTime.Year);
}
#endif

/// Test a PDF 1.6 document, signed by Adobe.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testPDF16Adobe)
{
    // Contains a cross-reference stream, object streams and a compressed
    // stream with a predictor. And a valid signature.
    // Found signatures was 0, as parsing failed due to lack of support for
    // these features.
    verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "pdf16adobe.pdf", 1);
}

/// Test adding a signature to a PDF 1.6 document.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testPDF16Add)
{
    // Contains PDF 1.6 features, make sure we can add a signature using that
    // markup correctly.
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aInURL = aSourceDir + "pdf16adobe.pdf";
    OUString aTargetDir
        = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
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

/// Test a PDF 1.4 document, signed by LO on Windows.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testPDF14LOWin)
{
    // mscrypto used SEC_OID_PKCS1_SHA1_WITH_RSA_ENCRYPTION as a digest
    // algorithm when it meant SEC_OID_SHA1, make sure we tolerate that on all
    // platforms.
    // This failed, as NSS HASH_Create() didn't handle the sign algorithm.
    verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "pdf14lowin.pdf", 1);
}

#if HAVE_FEATURE_PDFIUM
/// Test a PAdES document, signed by LO on Linux.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testPDFPAdESGood)
{
    std::vector<SignatureInformation> aInfos
        = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "good-pades.pdf", 1);
    CPPUNIT_ASSERT(aInfos[0].bHasSigningCertificate);
}

/// Test a valid signature that does not cover the whole file.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testPartial)
{
    std::vector<SignatureInformation> aInfos
        = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "partial.pdf", 1);
    CPPUNIT_ASSERT(!aInfos.empty());
    SignatureInformation& rInformation = aInfos[0];
    CPPUNIT_ASSERT(rInformation.bPartialDocumentSignature);
}

CPPUNIT_TEST_FIXTURE(PDFSigningTest, testPartialInBetween)
{
    std::vector<SignatureInformation> aInfos
        = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "partial-in-between.pdf", 2);
    CPPUNIT_ASSERT(!aInfos.empty());
    SignatureInformation& rInformation = aInfos[0];
    // Without the accompanying fix in place, this test would have failed, as unsigned incremental
    // update between two signatures were not detected.
    CPPUNIT_ASSERT(rInformation.bPartialDocumentSignature);
}

CPPUNIT_TEST_FIXTURE(PDFSigningTest, testBadCertP1)
{
    std::vector<SignatureInformation> aInfos
        = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "bad-cert-p1.pdf", 1);
    CPPUNIT_ASSERT(!aInfos.empty());
    SignatureInformation& rInformation = aInfos[0];
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0 (SecurityOperationStatus_UNKNOWN)
    // - Actual  : 1 (SecurityOperationStatus_OPERATION_SUCCEEDED)
    // i.e. annotation after a P1 signature was not considered as a bad modification.
    CPPUNIT_ASSERT_EQUAL(xml::crypto::SecurityOperationStatus::SecurityOperationStatus_UNKNOWN,
                         rInformation.nStatus);
}

CPPUNIT_TEST_FIXTURE(PDFSigningTest, testBadCertP3Stamp)
{
    std::vector<SignatureInformation> aInfos
        = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + "bad-cert-p3-stamp.pdf", 1);
    CPPUNIT_ASSERT(!aInfos.empty());
    SignatureInformation& rInformation = aInfos[0];

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0 (SecurityOperationStatus_UNKNOWN)
    // - Actual  : 1 (SecurityOperationStatus_OPERATION_SUCCEEDED)
    // i.e. adding a stamp annotation was not considered as a bad modification.
    CPPUNIT_ASSERT_EQUAL(xml::crypto::SecurityOperationStatus::SecurityOperationStatus_UNKNOWN,
                         rInformation.nStatus);
}

/// Test writing a PAdES signature.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testSigningCertificateAttribute)
{
    // Create a new signature.
    OUString aSourceDir = m_directories.getURLFromSrc(DATA_DIRECTORY);
    OUString aInURL = aSourceDir + "no.pdf";
    OUString aTargetDir
        = m_directories.getURLFromWorkdir("/CppunitTest/xmlsecurity_pdfsigning.test.user/");
    OUString aOutURL = aTargetDir + "signing-certificate-attribute.pdf";
    bool bHadCertificates = sign(aInURL, aOutURL, 0);
    if (!bHadCertificates)
        return;

    // Verify it.
    std::vector<SignatureInformation> aInfos = verify(aOutURL, 1);
    CPPUNIT_ASSERT(!aInfos.empty());
    SignatureInformation& rInformation = aInfos[0];
    // Assert that it has a signed signingCertificateV2 attribute.
    CPPUNIT_ASSERT(rInformation.bHasSigningCertificate);
}

/// Test that we accept files which are supposed to be good.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testGood)
{
    const std::initializer_list<std::u16string_view> aNames = {
        // We failed to determine if this is good or bad.
        u"good-non-detached.pdf",
        // Boolean value for dictionary key caused read error.
        u"dict-bool.pdf",
    };

    for (const auto& rName : aNames)
    {
        std::vector<SignatureInformation> aInfos
            = verify(m_directories.getURLFromSrc(DATA_DIRECTORY) + rName, 1);
        CPPUNIT_ASSERT(!aInfos.empty());
        SignatureInformation& rInformation = aInfos[0];
        CPPUNIT_ASSERT_EQUAL(int(xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED),
                             static_cast<int>(rInformation.nStatus));
    }
}
#endif

/// Test that we don't crash / loop while tokenizing these files.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testTokenize)
{
    const std::initializer_list<std::u16string_view> aNames = {
        // We looped on this broken input.
        u"no-eof.pdf",
        // ']' in a name token was mishandled.
        u"name-bracket.pdf",
        // %%EOF at the end wasn't followed by a newline.
        u"noeol.pdf",
        // File that's intentionally smaller than 1024 bytes.
        u"small.pdf",
        u"tdf107149.pdf",
        // Nested parentheses were not handled.
        u"tdf114460.pdf",
        // Valgrind was unhappy about this.
        u"forcepoint16.pdf",
    };

    for (const auto& rName : aNames)
    {
        SvFileStream aStream(m_directories.getURLFromSrc(DATA_DIRECTORY) + rName, StreamMode::READ);
        vcl::filter::PDFDocument aDocument;
        // Just make sure the tokenizer finishes without an error, don't look at the signature.
        CPPUNIT_ASSERT(aDocument.Read(aStream));

        if (OUString(rName) == "tdf107149.pdf")
            // This failed, page list was empty.
            CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aDocument.GetPages().size());
    }
}
#if HAVE_FEATURE_PDFIUM

/// Test handling of unknown SubFilter values.
CPPUNIT_TEST_FIXTURE(PDFSigningTest, testUnknownSubFilter)
{
    // Tokenize the bugdoc.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer
        = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext
        = xSEInitializer->createSecurityContext(OUString());
    std::unique_ptr<SvStream> pStream = utl::UcbStreamHelper::CreateStream(
        m_directories.getURLFromSrc(DATA_DIRECTORY) + "cr-comment.pdf", StreamMode::STD_READ);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    aManager.setSignatureStream(xStream);
    aManager.read(/*bUseTempStream=*/false);

    // Make sure we find both signatures, even if the second has unknown SubFilter.
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), rInformations.size());
}

CPPUNIT_TEST_FIXTURE(PDFSigningTest, testGoodCustomMagic)
{
    // Tokenize the bugdoc.
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer
        = xml::crypto::SEInitializer::create(mxComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext
        = xSEInitializer->createSecurityContext(OUString());
    std::unique_ptr<SvStream> pStream = utl::UcbStreamHelper::CreateStream(
        m_directories.getURLFromSrc(DATA_DIRECTORY) + "good-custom-magic.pdf",
        StreamMode::STD_READ);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
    DocumentSignatureManager aManager(mxComponentContext, DocumentSignatureMode::Content);
    aManager.setSignatureStream(xStream);
    aManager.read(/*bUseTempStream=*/false);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1 (SecurityOperationStatus_OPERATION_SUCCEEDED)
    // - Actual  : 0 (SecurityOperationStatus_UNKNOWN)
    // i.e. no signatures were found due to a custom non-comment magic after the header.
    std::vector<SignatureInformation>& rInformations = aManager.getCurrentSignatureInformations();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());
}
#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
