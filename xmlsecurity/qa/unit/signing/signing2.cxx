/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/propertysequence.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.hxx>

using namespace css;

namespace
{
OUStringLiteral const DATA_DIRECTORY = u"/xmlsecurity/qa/unit/signing/data/";
}

/// Testsuite for the document signing feature.
class SigningTest2 : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
protected:
    uno::Reference<lang::XComponent> mxComponent;
    uno::Reference<xml::crypto::XSEInitializer> mxSEInitializer;
    uno::Reference<xml::crypto::XXMLSecurityContext> mxSecurityContext;

public:
    SigningTest2();
    virtual void setUp() override;
    virtual void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
};

SigningTest2::SigningTest2() {}

void SigningTest2::setUp()
{
    test::BootstrapFixture::setUp();

    // Initialize crypto after setting up the environment variables.
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void SigningTest2::tearDown()
{
    if (mxComponent.is())
    {
        css::uno::Reference<css::util::XCloseable> closer(mxComponent, css::uno::UNO_QUERY);
        if (closer.is())
        {
            closer->close(true);
        }
        mxComponent->dispose();
    }

    test::BootstrapFixture::tearDown();
}

/// Test if a macro signature from a ODF Database is preserved when saving
CPPUNIT_TEST_FIXTURE(SigningTest2, testPreserveMacroSignatureODB)
{
    const OUString aURL(m_directories.getURLFromSrc(DATA_DIRECTORY) + "odb_signed_macros.odb");
    const OUString sLoadMessage = "loading failed: " + aURL;

    // load the file
    mxComponent = loadFromDesktop(aURL, "com.sun.star.sdb.OfficeDatabaseDocument");
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sLoadMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           mxComponent.is());

    // save as ODB
    utl::TempFile aTempFileSaveAsODB;
    aTempFileSaveAsODB.EnableKillingFile();
    try
    {
        uno::Reference<frame::XStorable> xDocStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> descSaveAs(comphelper::InitPropertySequence(
            { { "FilterName", uno::Any(OUString("StarOffice XML (Base)")) } }));
        xDocStorable->storeAsURL(aTempFileSaveAsODB.GetURL(), descSaveAs);
    }
    catch (...)
    {
        CPPUNIT_FAIL("Failed to save ODB file");
    }

    // Parse the resulting XML.
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(
            ZIP_STORAGE_FORMAT_STRING, aTempFileSaveAsODB.GetURL(), embed::ElementModes::READ);
    CPPUNIT_ASSERT(xStorage.is());
    uno::Reference<embed::XStorage> xMetaInf
        = xStorage->openStorageElement("META-INF", embed::ElementModes::READ);
    uno::Reference<io::XInputStream> xInputStream(
        xMetaInf->openStreamElement("macrosignatures.xml", embed::ElementModes::READ),
        uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // Make sure the signature is still there
    assertXPath(pXmlDoc, "//dsig:Signature", "Id",
                "ID_00a7002f009000bc00ce00f7004400460080002f002e00e400e0003700df00e8");
}

void SigningTest2::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("odfds"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:digitalsignature:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dsig"),
                       BAD_CAST("http://www.w3.org/2000/09/xmldsig#"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xd"), BAD_CAST("http://uri.etsi.org/01903/v1.3.2#"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
