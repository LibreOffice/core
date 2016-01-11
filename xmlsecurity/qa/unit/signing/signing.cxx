/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

#include <comphelper/processfactory.hxx>
#include <sax/tools/converter.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/storagehelper.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>

#include <xmlsecurity/documentsignaturehelper.hxx>
#include <xmlsecurity/xmlsignaturehelper.hxx>

#if !defined(MACOSX) && !defined(WNT)

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

    CPPUNIT_TEST_SUITE(SigningTest);
    CPPUNIT_TEST(testDescription);
    CPPUNIT_TEST_SUITE_END();

private:
    void createDoc();
    uno::Reference<security::XCertificate> getCertificate(XMLSignatureHelper& rSignatureHelper);
    void sign(utl::TempFile& rTempFile, XMLSignatureHelper& rSignatureHelper, const uno::Reference<io::XOutputStream>& xOutputStream);
    std::vector<SignatureInformation> verify(XMLSignatureHelper& rSignatureHelper, const uno::Reference<io::XInputStream>& xInputStream);
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

void SigningTest::createDoc()
{
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
}

uno::Reference<security::XCertificate> SigningTest::getCertificate(XMLSignatureHelper& rSignatureHelper)
{
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = rSignatureHelper.GetSecurityEnvironment();
    OUString aCertificate;
    {
        SvFileStream aStream(getURLFromSrc(DATA_DIRECTORY) + "certificate.crt", StreamMode::READ);
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

void SigningTest::sign(utl::TempFile& rTempFile, XMLSignatureHelper& rSignatureHelper, const uno::Reference<io::XOutputStream>& xOutputStream)
{
    CPPUNIT_ASSERT(rSignatureHelper.Init());

    SvStream* pStream = utl::UcbStreamHelper::CreateStream(rTempFile.GetURL(), StreamMode::READ);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
    uno::Reference<embed::XStorage> xStorage = comphelper::OStorageHelper::GetStorageOfFormatFromStream(ZIP_STORAGE_FORMAT_STRING, xStream);
    CPPUNIT_ASSERT(xStorage.is());

    rSignatureHelper.SetStorage(xStorage, "1.2");
    uno::Reference<security::XCertificate> xCertificate = getCertificate(rSignatureHelper);
    CPPUNIT_ASSERT(xCertificate.is());

    uno::Reference<security::XSerialNumberAdapter> xSerialNumberAdapter = security::SerialNumberAdapter::create(mxComponentContext);
    OUString aCertSerial = xSerialNumberAdapter->toString(xCertificate->getSerialNumber());
    CPPUNIT_ASSERT(!aCertSerial.isEmpty());

    rSignatureHelper.StartMission();
    sal_Int32 nSecurityId = rSignatureHelper.GetNewSecurityId();
    OUStringBuffer aStrBuffer;
    sax::Converter::encodeBase64(aStrBuffer, xCertificate->getEncoded());
    rSignatureHelper.SetX509Certificate(nSecurityId, xCertificate->getIssuerName(), aCertSerial, aStrBuffer.makeStringAndClear());
    DocumentSignatureMode eSignatureMode = SignatureModeDocumentContent;
    std::vector<OUString> aElements = DocumentSignatureHelper::CreateElementList(xStorage, eSignatureMode, OOo3_2Document);
    for (size_t i = 0; i < aElements.size(); ++i)
    {
        bool bBinaryMode = aElements[i].startsWith("Thumbnails/");
        rSignatureHelper.AddForSigning(nSecurityId, aElements[i], aElements[i], bBinaryMode);
    }
    rSignatureHelper.SetDateTime(nSecurityId, Date(Date::SYSTEM), tools::Time(tools::Time::SYSTEM));
    rSignatureHelper.SetDescription(nSecurityId, "SigningTest::sign");

    uno::Reference<xml::sax::XWriter> xWriter = rSignatureHelper.CreateDocumentHandlerWithHeader(xOutputStream);
    uno::Reference<xml::sax::XDocumentHandler> xDocumentHandler(xWriter, uno::UNO_QUERY);
    CPPUNIT_ASSERT(rSignatureHelper.CreateAndWriteSignature(xDocumentHandler));
    XMLSignatureHelper::CloseDocumentHandler(xDocumentHandler);

    rSignatureHelper.EndMission();
}

std::vector<SignatureInformation> SigningTest::verify(XMLSignatureHelper& rSignatureHelper, const uno::Reference<io::XInputStream>& xInputStream)
{
    rSignatureHelper.StartMission();
    rSignatureHelper.ReadAndVerifySignature(xInputStream);
    rSignatureHelper.EndMission();
    return rSignatureHelper.GetSignatureInformations();
}

void SigningTest::testDescription()
{
    // Create an empty document and store it to a tempfile.
    createDoc();
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer8");
    xStorable->storeAsURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Then sign the document, and store the signature it a second tempfile, so we don't write to the original storage when we read it as well.
    XMLSignatureHelper aSignatureHelper(mxComponentContext);
    uno::Reference<io::XStream> xSignatureStream(io::TempFile::create(mxComponentContext), uno::UNO_QUERY);
    uno::Reference<io::XOutputStream> xOutputStream(xSignatureStream, uno::UNO_QUERY);
    sign(aTempFile, aSignatureHelper, xOutputStream);

    // Go back to the start of the signature.
    uno::Reference<io::XSeekable> xSeekable(xSignatureStream, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSeekable->getPosition() != 0);
    xSeekable->seek(0);

    // Read back the signature and make sure that the description survives the roundtrip.
    uno::Reference<io::XInputStream> xInputStream(xSignatureStream, uno::UNO_QUERY);
    std::vector<SignatureInformation> aSignatureInformations = verify(aSignatureHelper, xInputStream);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aSignatureInformations.size());
    CPPUNIT_ASSERT_EQUAL(OUString("SigningTest::sign"), aSignatureInformations[0].ouDescription);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SigningTest);

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
