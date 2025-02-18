/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <svl/itemset.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <tools/json_writer.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/propertyvalue.hxx>
#include <vcl/scheduler.hxx>

using namespace com::sun::star;

/// Covers sfx2/source/view/ fixes.
class Sfx2ViewTest : public UnoApiTest
{
public:
    Sfx2ViewTest()
        : UnoApiTest("/sfx2/qa/cppunit/data/")
    {
    }

    void setUp() override
    {
        UnoApiTest::setUp();
        MacrosTest::setUpX509(m_directories, "sfx2_view");
    }
};

CPPUNIT_TEST_FIXTURE(Sfx2ViewTest, testReloadPage)
{
    // Load a document, which has 2 pages.
    loadFromFile(u"reload-page.odg");

    // Reload, and request to start on page 2.
    SfxViewFrame* pFrame = SfxViewFrame::Current();
    SfxAllItemSet aSet(SfxGetpApp()->GetPool());
    aSet.Put(SfxInt32Item(SID_PAGE_NUMBER, 1));
    SfxRequest aReq(SID_RELOAD, SfxCallMode::SLOT, aSet);
    pFrame->ExecReload_Impl(aReq);
    uno::Reference<frame::XModel> xModel = SfxObjectShell::Current()->GetBaseModel();
    mxComponent = xModel;

    // Check the current page after reload.
    uno::Reference<drawing::XDrawView> xController(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPage(xController->getCurrentPage(), uno::UNO_QUERY);
    sal_Int32 nPage{};
    xPage->getPropertyValue("Number") >>= nPage;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the document was opened on page 1, not page 2, SID_PAGE_NUMBER was ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), nPage);
}

CPPUNIT_TEST_FIXTURE(Sfx2ViewTest, testLokHelperExtractCertificates)
{
    std::string signatureCa = R"(-----BEGIN CERTIFICATE-----
foo
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
bar
-----END CERTIFICATE-----)";

    std::vector<std::string> aRet = SfxLokHelper::extractCertificates(signatureCa);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aRet.size());
    CPPUNIT_ASSERT_EQUAL(std::string("\nfoo\n"), aRet[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("\nbar\n"), aRet[1]);
}

#ifdef UNX
CPPUNIT_TEST_FIXTURE(Sfx2ViewTest, testLokHelperAddCertifices)
{
    // Given a loaded and signed document, CA is not trusted by default:
    loadFromFile(u"signed.odt");
    auto pBaseModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    CPPUNIT_ASSERT_EQUAL(SignatureState::NOTVALIDATED, pObjectShell->GetDocumentSignatureState());

    // When trusting the CA:
    OUString aCaUrl = createFileURL(u"ca.pem");
    SvFileStream aCaStream(aCaUrl, StreamMode::READ);
    std::string aCa;
    aCa = read_uInt8s_ToOString(aCaStream, aCaStream.remainingSize());
    std::vector<std::string> aCerts = SfxLokHelper::extractCertificates(aCa);
    SfxLokHelper::addCertificates(aCerts);

    // Then make sure the signature state is updated:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1 (OK)
    // - Actual  : 4 (SignatureState::NOTVALIDATED)
    // i.e. the signature status for an opened document was not updated when trusting a CA.
    CPPUNIT_ASSERT_EQUAL(SignatureState::OK, pObjectShell->GetDocumentSignatureState());
}

CPPUNIT_TEST_FIXTURE(Sfx2ViewTest, testLokHelperCommandValuesSignature)
{
    // Given an unsigned PDF file:
    loadFromFile(u"unsigned.pdf");

    // When extracting hashes:
    tools::JsonWriter aWriter;
    SfxLokHelper::getCommandValues(aWriter, ".uno:Signature");
    OString aJson = aWriter.finishAndGetAsOString();

    // Then make sure that we get a signature time and a hash:
    CPPUNIT_ASSERT(SfxLokHelper::supportsCommand(u"Signature"));
    std::stringstream aStream{ std::string(aJson) };
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    auto it = aTree.find("commandName");
    CPPUNIT_ASSERT(it != aTree.not_found());
    CPPUNIT_ASSERT_EQUAL(std::string(".uno:Signature"), it->second.get_value<std::string>());
    it = aTree.find("commandValues");
    CPPUNIT_ASSERT(it != aTree.not_found());
    aTree = it->second;
    // Non-zero timestamp:
    it = aTree.find("signatureTime");
    CPPUNIT_ASSERT(it != aTree.not_found());
    auto nSignatureTime = it->second.get_value<sal_Int64>();
    CPPUNIT_ASSERT(nSignatureTime != 0);
    // Base64 encoded hash, that has the SHA-256 length:
    it = aTree.find("digest");
    CPPUNIT_ASSERT(it != aTree.not_found());
    auto aDigest = OUString::fromUtf8(it->second.get_value<std::string>());
    uno::Sequence<sal_Int8> aBytes;
    comphelper::Base64::decode(aBytes, aDigest);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(32), aBytes.getLength());
}

namespace
{
OUString GetSignatureHash()
{
    tools::JsonWriter aWriter;
    // Provide the current time, so the system timer is not contacted:
    SfxLokHelper::getCommandValues(aWriter, ".uno:Signature?signatureTime=1731329053152");
    OString aJson = aWriter.finishAndGetAsOString();
    std::stringstream aStream{ std::string(aJson) };
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    auto it = aTree.find("commandValues");
    CPPUNIT_ASSERT(it != aTree.not_found());
    aTree = it->second;
    it = aTree.find("digest");
    CPPUNIT_ASSERT(it != aTree.not_found());
    return OUString::fromUtf8(it->second.get_value<std::string>());
}
}

CPPUNIT_TEST_FIXTURE(Sfx2ViewTest, testLokHelperCommandValuesSignatureHash)
{
    // Given an unsigned PDF file:
    loadFromFile(u"unsigned.pdf");

    // When extracting hashes, two times:
    OUString aHash1 = GetSignatureHash();
    OUString aHash2 = GetSignatureHash();

    // Then make sure that we get the same hash, since the same system time is provided:
    // In case the test was slow enough that there was 1ms system time difference between the two
    // calls, then this failed.
    CPPUNIT_ASSERT_EQUAL(aHash1, aHash2);
}

CPPUNIT_TEST_FIXTURE(Sfx2ViewTest, testSignatureSerialize)
{
    // Given an unsigned PDF file:
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;
    createTempCopy(u"unsigned.pdf");
    load(maTempFile.GetURL());

    // When signing by serializing an externally provided signature based on an earlier extracted
    // timestamp & document hash:
    OUString aSigUrl = createFileURL(u"signature.pkcs7");
    SvFileStream aSigStream(aSigUrl, StreamMode::READ);
    auto aSigValue
        = OUString::fromUtf8(read_uInt8s_ToOString(aSigStream, aSigStream.remainingSize()));
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"SignatureTime"_ustr, u"1643201995722"_ustr),
        comphelper::makePropertyValue(u"SignatureValue"_ustr, aSigValue),
    };
    dispatchCommand(mxComponent, u".uno:Signature"_ustr, aArgs);

    // Then make sure the document has a signature:
    SvMemoryStream aStream;
    aStream.WriteStream(*maTempFile.GetStream(StreamMode::READ));
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aStream.GetData(), aStream.GetSize(), OString());
    CPPUNIT_ASSERT(pPdfDocument);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. no signature was added, since we tried to sign interactively instead of based on
    // provided parameters.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getSignatureCount());
}

CPPUNIT_TEST_FIXTURE(Sfx2ViewTest, testScheduler)
{
    // Given an empty document:
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    // When asking for the state of the scheduler:
    int nRet = Scheduler::GetMostUrgentTaskPriority();

    // Then make sure we get a priority:
    // This returns TaskPriority::HIGH_IDLE, but just make sure we get a positive priority.
    CPPUNIT_ASSERT(nRet != -1);
}
#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
