/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <unotest/macros_test.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;

namespace
{

char const DATA_DIRECTORY[] = "/writerperfect/qa/unit/data/writer/epubexport/";

/// Tests the EPUB export filter.
class EPUBExportTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;
    utl::TempFile maTempFile;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr &pXmlXpathCtx) override;
    void createDoc(const OUString &rFile, const uno::Sequence<beans::PropertyValue> &rFilterData);
    void testOutlineLevel();
    void testMimetype();
    void testEPUB2();

    CPPUNIT_TEST_SUITE(EPUBExportTest);
    CPPUNIT_TEST(testOutlineLevel);
    CPPUNIT_TEST(testMimetype);
    CPPUNIT_TEST(testEPUB2);
    CPPUNIT_TEST_SUITE_END();
};

void EPUBExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void EPUBExportTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void EPUBExportTest::registerNamespaces(xmlXPathContextPtr &pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("opf"), BAD_CAST("http://www.idpf.org/2007/opf"));
}

void EPUBExportTest::createDoc(const OUString &rFile, const uno::Sequence<beans::PropertyValue> &rFilterData)
{
    // Import the bugdoc and export as EPUB.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + rFile;
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    maTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("EPUB");
    aMediaDescriptor["FilterData"] <<= rFilterData;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
}

void EPUBExportTest::testOutlineLevel()
{
    createDoc("outline-level.fodt", {});

    // Make sure that the output is split into two.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, maTempFile.GetURL());
    CPPUNIT_ASSERT(xNameAccess->hasByName("OEBPS/sections/section0001.xhtml"));
    // This failed, output was a single section.
    CPPUNIT_ASSERT(xNameAccess->hasByName("OEBPS/sections/section0002.xhtml"));
    CPPUNIT_ASSERT(!xNameAccess->hasByName("OEBPS/sections/section0003.xhtml"));
}

void EPUBExportTest::testMimetype()
{
    createDoc("hello.fodt", {});

    // Check that the mime type is written uncompressed at the expected location.
    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemoryStream;
    aMemoryStream.WriteStream(aFileStream);
    OString aExpected("application/epub+zip");
    CPPUNIT_ASSERT(aMemoryStream.GetSize() > static_cast<sal_uInt64>(38 + aExpected.getLength()));

    OString aActual(static_cast<const char *>(aMemoryStream.GetBuffer()) + 38, aExpected.getLength());
    // This failed: actual data was some garbage, not the uncompressed mime type.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

void EPUBExportTest::testEPUB2()
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
    {
        // Explicitly request EPUB2.
        {"EPUBVersion", uno::makeAny(static_cast<sal_Int32>(20))}
    }));
    createDoc("hello.fodt", aFilterData);

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, maTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("OEBPS/content.opf"), uno::UNO_QUERY);
    std::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocPtr pXmlDoc = parseXmlStream(pStream.get());
    // This was 3.0, EPUBVersion filter option was ignored and we always emitted EPUB3.
    assertXPath(pXmlDoc, "/opf:package", "version", "2.0");
    xmlFreeDoc(pXmlDoc);
}

CPPUNIT_TEST_SUITE_REGISTRATION(EPUBExportTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
