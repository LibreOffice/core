/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <type_traits>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/security/SerialNumberAdapter.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/packages/manifest/ManifestReader.hpp>

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

using namespace com::sun::star;

namespace
{
const char* DATA_DIRECTORY = "/xmlsecurity/qa/unit/signing/data/";
}

/// Testsuite for the document signing feature.
class SigningTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;

public:
    SigningTest();
    virtual void setUp() override;
    virtual void tearDown() override;

    void testDescription();
    /// Test a typical ODF where all streams are signed.
    void testODFGood();
    /// Test a typical broken ODF signature where one stream is corrupted.
    void testODFBroken();
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
    void test96097Calc();
    void test96097Doc();

    CPPUNIT_TEST_SUITE(SigningTest);
    CPPUNIT_TEST(testDescription);
    CPPUNIT_TEST(testODFGood);
    CPPUNIT_TEST(testODFBroken);
    CPPUNIT_TEST(testODFBroken);
    CPPUNIT_TEST(testOOXMLPartial);
    CPPUNIT_TEST(testOOXMLBroken);
    CPPUNIT_TEST(testOOXMLDescription);
    CPPUNIT_TEST(testOOXMLAppend);
    CPPUNIT_TEST(testOOXMLRemove);
    CPPUNIT_TEST(testOOXMLRemoveAll);
    CPPUNIT_TEST(test96097Calc);
    CPPUNIT_TEST(test96097Doc);
    CPPUNIT_TEST_SUITE_END();

private:
    void createDoc(const OUString& rURL);
    void createCalc(const OUString& rURL);
    uno::Reference<security::XCertificate> getCertificate(XMLSignatureHelper& rSignatureHelper);
};

SigningTest::SigningTest()
{
}

void SigningTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
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

uno::Reference<security::XCertificate> SigningTest::getCertificate(XMLSignatureHelper& rSignatureHelper)
{
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = rSignatureHelper.GetSecurityEnvironment();
    OUString aCertificate;
    {
        SvFileStream aStream(m_directories.getURLFromSrc(DATA_DIRECTORY) + "certificate.crt", StreamMode::READ);
        OString aLine;
        bool bMore = aStream.ReadLine(aLine);
        while (bMore)
        {
            aCertificate += OUString::fromUtf8(aLine);
            aCertificate += "\n";
            bMore = aStream.ReadLine(aLine);
        }
    }
    return xSecurityEnvironment->createCertificateFromAscii(aCertificate);
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

    DocumentSignatureManager aManager(mxComponentContext, SignatureModeDocumentContent);
    CPPUNIT_ASSERT(aManager.maSignatureHelper.Init());
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");

    // Then add a signature document.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager.maSignatureHelper);
    CPPUNIT_ASSERT(xCertificate.is());
    OUString aDescription("SigningTest::testDescription");
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, aDescription, nSecurityId);

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

    DocumentSignatureManager aManager(mxComponentContext, SignatureModeDocumentContent);
    CPPUNIT_ASSERT(aManager.maSignatureHelper.Init());
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aTempFile.GetURL(), embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");

    // Then add a document signature.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager.maSignatureHelper);
    CPPUNIT_ASSERT(xCertificate.is());
    OUString aDescription("SigningTest::testDescription");
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, aDescription, nSecurityId);

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
    DocumentSignatureManager aManager(mxComponentContext, SignatureModeDocumentContent);
    CPPUNIT_ASSERT(aManager.maSignatureHelper.Init());
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL, embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());

    // Then add a second document signature.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager.maSignatureHelper);
    CPPUNIT_ASSERT(xCertificate.is());
    sal_Int32 nSecurityId;
    aManager.add(xCertificate, OUString(), nSecurityId);

    // Read back the signatures and make sure that we have the expected amount.
    aManager.read(/*bUseTempStream=*/true);
    // This was 1: the original signature was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), rInformations.size());
}

void SigningTest::testOOXMLRemove()
{
    // Load the test document as a storage and read its signatures: purpose1 and purpose2.
    DocumentSignatureManager aManager(mxComponentContext, SignatureModeDocumentContent);
    CPPUNIT_ASSERT(aManager.maSignatureHelper.Init());
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "multi.docx";
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL, embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), rInformations.size());

    // Then remove the last added signature.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager.maSignatureHelper);
    CPPUNIT_ASSERT(xCertificate.is());
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
    DocumentSignatureManager aManager(mxComponentContext, SignatureModeDocumentContent);
    CPPUNIT_ASSERT(aManager.maSignatureHelper.Init());
    uno::Reference <embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL, embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());
    aManager.mxStore = xStorage;
    aManager.maSignatureHelper.SetStorage(xStorage, "1.2");
    aManager.read(/*bUseTempStream=*/false);
    std::vector<SignatureInformation>& rInformations = aManager.maCurrentSignatureInformations;
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), rInformations.size());

    // Then remove the only signature in the document.
    uno::Reference<security::XCertificate> xCertificate = getCertificate(aManager.maSignatureHelper);
    CPPUNIT_ASSERT(xCertificate.is());
    aManager.remove(0);
    aManager.read(/*bUseTempStream=*/true);
    aManager.write();

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
    descSaveACopy[0].Value <<= uno::makeAny(true);
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
    descSaveACopy[0].Value <<= uno::makeAny(true);
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

CPPUNIT_TEST_SUITE_REGISTRATION(SigningTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
