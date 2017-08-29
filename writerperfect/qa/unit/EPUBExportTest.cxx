/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libepubgen/libepubgen.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <unotest/macros_test.hxx>
#include <unotools/docinfohelper.hxx>
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
    xmlDocPtr mpXmlDoc = nullptr;
    uno::Reference<packages::zip::XZipFileAccess2> mxZipFile;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr &pXmlXpathCtx) override;
    void createDoc(const OUString &rFile, const uno::Sequence<beans::PropertyValue> &rFilterData);
    /// Returns an XML representation of the stream named rName in the exported package.
    xmlDocPtr parseExport(const OUString &rName);
    void testOutlineLevel();
    void testMimetype();
    void testEPUB2();
    void testPageBreakSplit();
    void testSpanAutostyle();
    void testParaAutostyleCharProps();

    CPPUNIT_TEST_SUITE(EPUBExportTest);
    CPPUNIT_TEST(testOutlineLevel);
    CPPUNIT_TEST(testMimetype);
    CPPUNIT_TEST(testEPUB2);
    CPPUNIT_TEST(testPageBreakSplit);
    CPPUNIT_TEST(testSpanAutostyle);
    CPPUNIT_TEST(testParaAutostyleCharProps);
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

    if (mpXmlDoc)
    {
        xmlFreeDoc(mpXmlDoc);
        mpXmlDoc = nullptr;
    }

    test::BootstrapFixture::tearDown();
}

void EPUBExportTest::registerNamespaces(xmlXPathContextPtr &pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("opf"), BAD_CAST("http://www.idpf.org/2007/opf"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xhtml"), BAD_CAST("http://www.w3.org/1999/xhtml"));
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
    mxZipFile = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, maTempFile.GetURL());
}

xmlDocPtr EPUBExportTest::parseExport(const OUString &rName)
{
    uno::Reference<io::XInputStream> xInputStream(mxZipFile->getByName(rName), uno::UNO_QUERY);
    std::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    return parseXmlStream(pStream.get());
}

void EPUBExportTest::testOutlineLevel()
{
    createDoc("outline-level.fodt", {});

    // Make sure that the output is split into two.
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/sections/section0001.xhtml"));
    // This failed, output was a single section.
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/sections/section0002.xhtml"));
    CPPUNIT_ASSERT(!mxZipFile->hasByName("OEBPS/sections/section0003.xhtml"));
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

    mpXmlDoc = parseExport("OEBPS/content.opf");
    // Default is EPUB3.
    assertXPath(mpXmlDoc, "/opf:package", "version", "3.0");

    // This was just "libepubgen/x.y.z", i.e. the LO version was missing.
    OUString aGenerator = getXPath(mpXmlDoc, "/opf:package/opf:metadata/opf:meta[@name='generator']", "content");
    CPPUNIT_ASSERT(aGenerator.startsWith(utl::DocInfoHelper::GetGeneratorString()));
}

void EPUBExportTest::testEPUB2()
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
    {
        // Explicitly request EPUB2.
        {"EPUBVersion", uno::makeAny(static_cast<sal_Int32>(20))}
    }));
    createDoc("hello.fodt", aFilterData);

    mpXmlDoc = parseExport("OEBPS/content.opf");
    // This was 3.0, EPUBVersion filter option was ignored and we always emitted EPUB3.
    assertXPath(mpXmlDoc, "/opf:package", "version", "2.0");
}

void EPUBExportTest::testPageBreakSplit()
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
    {
        // Explicitly request split on page break (instead of on heading).
        {"EPUBSplitMethod", uno::makeAny(static_cast<sal_Int32>(libepubgen::EPUB_SPLIT_METHOD_PAGE_BREAK))}
    }));
    createDoc("2pages.fodt", aFilterData);

    // Make sure that the output is split into two.
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/sections/section0001.xhtml"));
    // This failed, output was a single section.
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/sections/section0002.xhtml"));
    CPPUNIT_ASSERT(!mxZipFile->hasByName("OEBPS/sections/section0003.xhtml"));
}

void EPUBExportTest::testSpanAutostyle()
{
    createDoc("span-autostyle.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[1]", "class", "span0");
    // This failed, it was still span1, i.e. the bold and the italic formatting
    // did not differ.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]", "class", "span1");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[3]", "class", "span2");
}

void EPUBExportTest::testParaAutostyleCharProps()
{
    createDoc("para-autostyle-char-props.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This failed, para-level char props were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p[1]/xhtml:span", "class", "span0");
    assertXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span", "class", "span1");
}

CPPUNIT_TEST_SUITE_REGISTRATION(EPUBExportTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
