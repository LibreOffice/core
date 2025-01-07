/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_crypto.h>

#if USE_CRYPTO_NSS
#include <secoid.h>
#include <nss.h>
#endif

#include <test/unoapixml_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/objsh.hxx>
#include <comphelper/documentconstants.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/saveopt.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/storagehelper.hxx>

using namespace css;

/// Testsuite for the document signing feature.
class SigningTest2 : public UnoApiXmlTest
{
protected:
    uno::Reference<xml::crypto::XSEInitializer> mxSEInitializer;
    uno::Reference<xml::crypto::XXMLSecurityContext> mxSecurityContext;

public:
    SigningTest2();
    virtual void setUp() override;
    virtual void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
};

SigningTest2::SigningTest2()
    : UnoApiXmlTest(u"/xmlsecurity/qa/unit/signing/data/"_ustr)
{
}

void SigningTest2::setUp()
{
    UnoApiXmlTest::setUp();

    MacrosTest::setUpX509(m_directories, u"xmlsecurity_signing2"_ustr);
    MacrosTest::setUpGpg(m_directories, std::u16string_view(u"xmlsecurity_signing2"));

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

void SigningTest2::tearDown()
{
    MacrosTest::tearDownGpg();

    UnoApiXmlTest::tearDown();
}

/// Test if a macro signature from a ODF Database is preserved when saving
CPPUNIT_TEST_FIXTURE(SigningTest2, testPreserveMacroSignatureODB)
{
    loadFromFile(u"odb_signed_macros.odb");

    // save as ODB
    save(u"StarOffice XML (Base)"_ustr);

    // Parse the resulting XML.
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, maTempFile.GetURL(), embed::ElementModes::READ);
    CPPUNIT_ASSERT(xStorage.is());
    uno::Reference<embed::XStorage> xMetaInf
        = xStorage->openStorageElement(u"META-INF"_ustr, embed::ElementModes::READ);
    uno::Reference<io::XInputStream> xInputStream(
        xMetaInf->openStreamElement(u"macrosignatures.xml"_ustr, embed::ElementModes::READ),
        uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // Make sure the signature is still there
    assertXPath(pXmlDoc, "//dsig:Signature", "Id",
                u"ID_00a7002f009000bc00ce00f7004400460080002f002e00e400e0003700df00e8");
}

CPPUNIT_TEST_FIXTURE(SigningTest2, testPasswordPreserveMacroSignatureODF13)
{
    // load ODF 1.3 encrypted document
    loadFromFile(u"encrypted_scriptsig_odf13.odt", "password");
    {
        uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"secret"_ustr, xTextDoc->getText()->getString());
        // test macro signature
        SfxBaseModel* pBaseModel(dynamic_cast<SfxBaseModel*>(mxComponent.get()));
        CPPUNIT_ASSERT(pBaseModel);
        SfxObjectShell* pObjectShell(pBaseModel->GetObjectShell());
        uno::Reference<beans::XPropertySet> xPropSet(pObjectShell->GetStorage(),
                                                     uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(ODFVER_013_TEXT,
                             xPropSet->getPropertyValue(u"Version"_ustr).get<OUString>());
        CPPUNIT_ASSERT_EQUAL(SignatureState::OK, pObjectShell->GetScriptingSignatureState());
    }

    {
        // test the old, standard ODF 1.2/1.3/1.4 encryption
        Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });
        SetODFDefaultVersion(SvtSaveOptions::ODFVER_013);

        saveAndReload(u"writer8"_ustr, "password");

        xmlDocUniquePtr pXmlDoc = parseExport(u"META-INF/manifest.xml"_ustr);
        assertXPath(pXmlDoc, "/manifest:manifest", "version", u"1.3");
        assertXPath(pXmlDoc, "/manifest:manifest/manifest:file-entry[@manifest:size != '0']", 8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/"
                    "manifest:encryption-data[@manifest:checksum-type and @manifest:checksum]",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:algorithm[@manifest:algorithm-name='http://www.w3.org/2001/04/"
                    "xmlenc#aes256-cbc']",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:algorithm[string-length(@manifest:initialisation-vector) = 24]",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:start-key-generation[@manifest:start-key-generation-name='http://"
                    "www.w3.org/2000/09/xmldsig#sha256' and @manifest:key-size='32']",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[@manifest:key-derivation-name='PBKDF2' and "
                    "@manifest:key-size='32']",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[@manifest:iteration-count='100000']",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[string-length(@manifest:salt) = 24]",
                    8);
        // test reimport
        uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"secret"_ustr, xTextDoc->getText()->getString());
        // test macro signature - this didn't actually work!
        // using Zip Storage means the encrypted streams are signed, so
        // after encrypting again the signature didn't match and was dropped
        //        assertDocument(CPPUNIT_SOURCELINE(), "writer8", SignatureState::NOSIGNATURES,
        //                       SignatureState::OK, ODFVER_014_TEXT);
    }

    {
        // store it with new wholesome ODF extended encryption - reload
        saveAndReload(u"writer8"_ustr, "password");

        // test wholesome ODF extended encryption
        xmlDocUniquePtr pXmlDoc = parseExport(u"META-INF/manifest.xml"_ustr);
        assertXPath(pXmlDoc, "/manifest:manifest", "version", u"1.4");
        assertXPath(pXmlDoc, "/manifest:manifest/manifest:file-entry", 1);
        assertXPath(pXmlDoc, "/manifest:manifest/manifest:file-entry", "full-path",
                    u"encrypted-package");
        assertXPath(pXmlDoc, "/manifest:manifest/manifest:file-entry[@manifest:size != '0']", 1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/"
                    "manifest:encryption-data[@manifest:checksum-type or @manifest:checksum]",
                    0);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:algorithm[@manifest:algorithm-name='http://www.w3.org/2009/"
                    "xmlenc11#aes256-gcm']",
                    1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:algorithm[string-length(@manifest:initialisation-vector) = 16]",
                    1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:start-key-generation[@manifest:start-key-generation-name='http://"
                    "www.w3.org/2001/04/xmlenc#sha256' and @manifest:key-size='32']",
                    1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[@manifest:key-derivation-name='urn:org:"
                    "documentfoundation:names:experimental:office:manifest:argon2id' and "
                    "@manifest:key-size='32']",
                    1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[@manifest:iteration-count]",
                    0);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[string-length(@manifest:salt) = 24]",
                    1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[@loext:argon2-iterations='3' and "
                    "@loext:argon2-memory='65536' and @loext:argon2-lanes='4']",
                    1);
        // test reimport
        uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"secret"_ustr, xTextDoc->getText()->getString());
    }
}

CPPUNIT_TEST_FIXTURE(SigningTest2, testPasswordPreserveMacroSignatureODFWholesomeLO242)
{
    // load wholesome ODF (extended) encrypted document
    loadFromFile(u"encrypted_scriptsig_lo242.odt", "password");
    {
        uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"secret"_ustr, xTextDoc->getText()->getString());
        // test macro signature
        SfxBaseModel* pBaseModel(dynamic_cast<SfxBaseModel*>(mxComponent.get()));
        CPPUNIT_ASSERT(pBaseModel);
        SfxObjectShell* pObjectShell(pBaseModel->GetObjectShell());
        uno::Reference<beans::XPropertySet> xPropSet(pObjectShell->GetStorage(),
                                                     uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(ODFVER_013_TEXT,
                             xPropSet->getPropertyValue(u"Version"_ustr).get<OUString>());
        CPPUNIT_ASSERT_EQUAL(SignatureState::OK, pObjectShell->GetScriptingSignatureState());
    }

    {
        // store it with new wholesome ODF extended encryption - reload
        saveAndReload(u"writer8"_ustr, "password");

        // test wholesome ODF extended encryption
        xmlDocUniquePtr pXmlDoc = parseExport(u"META-INF/manifest.xml"_ustr);
        assertXPath(pXmlDoc, "/manifest:manifest", "version", u"1.4");
        assertXPath(pXmlDoc, "/manifest:manifest/manifest:file-entry", 1);
        assertXPath(pXmlDoc, "/manifest:manifest/manifest:file-entry", "full-path",
                    u"encrypted-package");
        assertXPath(pXmlDoc, "/manifest:manifest/manifest:file-entry[@manifest:size != '0']", 1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/"
                    "manifest:encryption-data[@manifest:checksum-type or @manifest:checksum]",
                    0);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:algorithm[@manifest:algorithm-name='http://www.w3.org/2009/"
                    "xmlenc11#aes256-gcm']",
                    1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:algorithm[string-length(@manifest:initialisation-vector) = 16]",
                    1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:start-key-generation[@manifest:start-key-generation-name='http://"
                    "www.w3.org/2001/04/xmlenc#sha256' and @manifest:key-size='32']",
                    1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[@manifest:key-derivation-name='urn:org:"
                    "documentfoundation:names:experimental:office:manifest:argon2id' and "
                    "@manifest:key-size='32']",
                    1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[@manifest:iteration-count]",
                    0);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[string-length(@manifest:salt) = 24]",
                    1);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[@loext:argon2-iterations='3' and "
                    "@loext:argon2-memory='65536' and @loext:argon2-lanes='4']",
                    1);
        // test reimport
        uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"secret"_ustr, xTextDoc->getText()->getString());
        // test macro signature - this should work now
        SfxBaseModel* pBaseModel(dynamic_cast<SfxBaseModel*>(mxComponent.get()));
        CPPUNIT_ASSERT(pBaseModel);
        SfxObjectShell* pObjectShell(pBaseModel->GetObjectShell());
        uno::Reference<beans::XPropertySet> xPropSet(pObjectShell->GetStorage(),
                                                     uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(ODFVER_014_TEXT,
                             xPropSet->getPropertyValue(u"Version"_ustr).get<OUString>());
        CPPUNIT_ASSERT_EQUAL(SignatureState::OK, pObjectShell->GetScriptingSignatureState());
    }

    {
        // test the old, standard ODF 1.2/1.3/1.4 encryption
        Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });
        SetODFDefaultVersion(SvtSaveOptions::ODFVER_013);

        saveAndReload(u"writer8"_ustr, "password");

        xmlDocUniquePtr pXmlDoc = parseExport(u"META-INF/manifest.xml"_ustr);
        assertXPath(pXmlDoc, "/manifest:manifest", "version", u"1.3");
        assertXPath(pXmlDoc, "/manifest:manifest/manifest:file-entry[@manifest:size != '0']", 8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/"
                    "manifest:encryption-data[@manifest:checksum-type and @manifest:checksum]",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:algorithm[@manifest:algorithm-name='http://www.w3.org/2001/04/"
                    "xmlenc#aes256-cbc']",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:algorithm[string-length(@manifest:initialisation-vector) = 24]",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:start-key-generation[@manifest:start-key-generation-name='http://"
                    "www.w3.org/2000/09/xmldsig#sha256' and @manifest:key-size='32']",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[@manifest:key-derivation-name='PBKDF2' and "
                    "@manifest:key-size='32']",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[@manifest:iteration-count='100000']",
                    8);
        assertXPath(pXmlDoc,
                    "/manifest:manifest/manifest:file-entry/manifest:encryption-data/"
                    "manifest:key-derivation[string-length(@manifest:salt) = 24]",
                    8);
        // test reimport
        uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"secret"_ustr, xTextDoc->getText()->getString());
        // test macro signature - this didn't actually work!
        // using Zip Storage means the encrypted streams are signed, so
        // after encrypting again the signature didn't match and was dropped
        //        assertDocument(CPPUNIT_SOURCELINE(), "writer8", SignatureState::NOSIGNATURES,
        //                       SignatureState::OK, ODFVER_014_TEXT);
    }
}

void SigningTest2::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("odfds"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:digitalsignature:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dsig"),
                       BAD_CAST("http://www.w3.org/2000/09/xmldsig#"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xd"), BAD_CAST("http://uri.etsi.org/01903/v1.3.2#"));

    // manifest.xml
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("manifest"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("loext"),
        BAD_CAST("urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
