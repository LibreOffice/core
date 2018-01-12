/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#include <memory>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <unotest/macros_test.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>


using namespace ::com::sun::star;


namespace {

class MiscTest
    : public test::BootstrapFixture
    , public unotest::MacrosTest
    , public XmlTestTools
{
public:
    virtual void setUp() override;
    void testODFCustomMetadata();
    void testNoThumbnail();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override
    {
        // ODF
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("office"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:office:1.0"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("meta"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:meta:1.0"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dc"), BAD_CAST("http://purl.org/dc/elements/1.1/"));
        // used in testCustomMetadata
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("foo"), BAD_CAST("http://foo.net"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("baz"), BAD_CAST("http://baz.net"));
    }

    CPPUNIT_TEST_SUITE(MiscTest);
    CPPUNIT_TEST(testODFCustomMetadata);
    CPPUNIT_TEST(testNoThumbnail);
    CPPUNIT_TEST_SUITE_END();

private:
};

void MiscTest::setUp()
{
    m_xContext = comphelper::getProcessComponentContext();
    mxDesktop.set(frame::Desktop::create(m_xContext));
    SfxApplication::GetOrCreate();
}

void MiscTest::testODFCustomMetadata()
{
    uno::Reference<document::XDocumentProperties> const xProps(
        ::com::sun::star::document::DocumentProperties::create(m_xContext));

    OUString const url(m_directories.getURLFromSrc("/sfx2/qa/complex/sfx2/testdocuments/CUSTOM.odt"));
    xProps->loadFromMedium(url, uno::Sequence<beans::PropertyValue>());
    CPPUNIT_ASSERT_EQUAL(OUString(""), xProps->getAuthor());
    uno::Sequence<beans::PropertyValue> mimeArgs({
        beans::PropertyValue("MediaType", -1, uno::Any(OUString("application/vnd.oasis.opendocument.text")), beans::PropertyState_DIRECT_VALUE)
        });
    utl::TempFile aTempFile;
    xProps->storeToMedium(aTempFile.GetURL(), mimeArgs);

    // check that custom metadata is preserved
    uno::Reference<packages::zip::XZipFileAccess2> const xZip(
        packages::zip::ZipFileAccess::createWithURL(m_xContext, aTempFile.GetURL()));
    uno::Reference<io::XInputStream> const xInputStream(xZip->getByName("meta.xml"), uno::UNO_QUERY);
    std::shared_ptr<SvStream> const pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocPtr pXmlDoc = parseXmlStream(pStream.get());
    assertXPathContent(pXmlDoc, "/office:document-meta/office:meta/bork", "bork");
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar", 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar/baz:foo", 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar/baz:foo[@baz:bar='foo']");
    assertXPathContent(pXmlDoc, "/office:document-meta/office:meta/foo:bar/foo:baz", "bar");

    aTempFile.EnableKillingFile();
}

void MiscTest::testNoThumbnail()
{
    // Load a document.
    const OUString aURL(m_directories.getURLFromSrc("/sfx2/qa/cppunit/misc/hello.odt"));
    uno::Reference<lang::XComponent> xComponent
        = loadFromDesktop(aURL, "com.sun.star.text.TextDocument");
    CPPUNIT_ASSERT(xComponent.is());

    // Save it with the NoThumbnail option and assert that it has no thumbnail.
    uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xStorable.is());
    utl::TempFile aTempFile;
    uno::Sequence<beans::PropertyValue> aProperties(
        comphelper::InitPropertySequence({ { "NoThumbnail", uno::makeAny(true) } }));
    xStorable->storeToURL(aTempFile.GetURL(), aProperties);
    uno::Reference<packages::zip::XZipFileAccess2> xZipFile
        = packages::zip::ZipFileAccess::createWithURL(m_xContext, aTempFile.GetURL());
    CPPUNIT_ASSERT(!xZipFile->hasByName("Thumbnails/thumbnail.png"));

    xComponent->dispose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(MiscTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
