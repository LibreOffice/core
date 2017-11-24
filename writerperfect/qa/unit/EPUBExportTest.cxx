/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libepubgen/libepubgen.h>

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
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
    /// Asserts that rCssDoc has a key named rKey and one of its rules is rValue.
    void assertCss(const std::map< OString, std::vector<OString> > &rCssDoc, const OString &rKey, const OString &rValue);
    void createDoc(const OUString &rFile, const uno::Sequence<beans::PropertyValue> &rFilterData);
    /// Returns an XML representation of the stream named rName in the exported package.
    xmlDocPtr parseExport(const OUString &rName);
    /// Loads a CSS representation of the stream named rName in the exported package into rTree.
    void parseCssExport(const OUString &rName, std::map< OString, std::vector<OString> > &rTree);
    /// Loads a CSS style string into a map.
    static void parseCssStyle(const OUString &rStyle, std::map<OUString, OUString> &rCss);
    void testOutlineLevel();
    void testMimetype();
    void testEPUB2();
    void testPageBreakSplit();
    void testSpanAutostyle();
    void testParaAutostyleCharProps();
    void testMeta();
    void testParaNamedstyle();
    void testCharNamedstyle();
    void testNamedStyleInheritance();
    void testNestedSpan();
    void testLineBreak();
    void testEscape();
    void testParaCharProps();
    void testSection();
    void testList();
    void testImage();
    void testTable();
    void testTableRowSpan();
    void testTableCellBorder();
    void testTableCellWidth();
    void testTableRowHeight();
    void testLink();
    void testLinkCharFormat();
    void testLinkNamedCharFormat();

    CPPUNIT_TEST_SUITE(EPUBExportTest);
    CPPUNIT_TEST(testOutlineLevel);
    CPPUNIT_TEST(testMimetype);
    CPPUNIT_TEST(testEPUB2);
    CPPUNIT_TEST(testPageBreakSplit);
    CPPUNIT_TEST(testSpanAutostyle);
    CPPUNIT_TEST(testParaAutostyleCharProps);
    CPPUNIT_TEST(testMeta);
    CPPUNIT_TEST(testParaNamedstyle);
    CPPUNIT_TEST(testCharNamedstyle);
    CPPUNIT_TEST(testNamedStyleInheritance);
    CPPUNIT_TEST(testNestedSpan);
    CPPUNIT_TEST(testLineBreak);
    CPPUNIT_TEST(testEscape);
    CPPUNIT_TEST(testParaCharProps);
    CPPUNIT_TEST(testSection);
    CPPUNIT_TEST(testList);
    CPPUNIT_TEST(testImage);
    CPPUNIT_TEST(testTable);
    CPPUNIT_TEST(testTableRowSpan);
    CPPUNIT_TEST(testTableCellBorder);
    CPPUNIT_TEST(testTableCellWidth);
    CPPUNIT_TEST(testTableRowHeight);
    CPPUNIT_TEST(testLink);
    CPPUNIT_TEST(testLinkCharFormat);
    CPPUNIT_TEST(testLinkNamedCharFormat);
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
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dc"), BAD_CAST("http://purl.org/dc/elements/1.1/"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("opf"), BAD_CAST("http://www.idpf.org/2007/opf"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xhtml"), BAD_CAST("http://www.w3.org/1999/xhtml"));
}

void EPUBExportTest::assertCss(const std::map< OString, std::vector<OString> > &rCssDoc, const OString &rKey, const OString &rValue)
{
    auto it = rCssDoc.find(rKey);
    CPPUNIT_ASSERT(it != rCssDoc.end());

    const std::vector<OString> &rRule = it->second;
    CPPUNIT_ASSERT_MESSAGE(OString("In '" + rKey + "', rule '" + rValue + "' is not found.").getStr(),
                           std::find(rRule.begin(), rRule.end(), rValue) != rRule.end());
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

void EPUBExportTest::parseCssExport(const OUString &rName, std::map< OString, std::vector<OString> > &rTree)
{
    uno::Reference<io::XInputStream> xInputStream(mxZipFile->getByName(rName), uno::UNO_QUERY);
    std::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));

    // Minimal CSS handler till orcus is up to our needs.
    OString aLine;
    OString aRuleName;
    while (!pStream->eof())
    {
        pStream->ReadLine(aLine);
        if (aLine.endsWith("{"))
            // '.name {' -> 'name'
            aRuleName = aLine.copy(1, aLine.getLength() - 3);
        else if (aLine.endsWith(";"))
            rTree[aRuleName].push_back(aLine);
    }
}

void EPUBExportTest::parseCssStyle(const OUString &rStyle, std::map<OUString, OUString> &rCss)
{
    for (const auto &rKeyValue : comphelper::string::split(rStyle, ';'))
    {
        OUString aKeyValue = rKeyValue.trim();
        std::vector<OUString> aTokens = comphelper::string::split(aKeyValue, ':');
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aTokens.size());
        rCss[aTokens[0].trim()] = aTokens[1].trim();
    }
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

    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponentContext->getServiceManager(), uno::UNO_QUERY);
    const OUString aServiceName("com.sun.star.comp.Writer.EPUBExportFilter");
    uno::Reference<document::XFilter> xFilter(xMSF->createInstance(aServiceName), uno::UNO_QUERY);
    // Should result in no errors.
    xFilter->cancel();
    // We got back what we expected.
    uno::Reference<lang::XServiceInfo> xServiceInfo(xFilter, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(aServiceName, xServiceInfo->getImplementationName());
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.document.ExportFilter"));
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
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
    {
        // Explicitly request in-CSS styles.
        {"EPUBStylesMethod", uno::makeAny(static_cast<sal_Int32>(0))}
    }));
    createDoc("span-autostyle.fodt", aFilterData);

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[1]", "class", "span0");
    // This failed, it was still span1, i.e. the bold and the italic formatting
    // did not differ.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]", "class", "span1");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[3]", "class", "span2");
}

void EPUBExportTest::testParaAutostyleCharProps()
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
    {
        // Explicitly request in-CSS styles.
        {"EPUBStylesMethod", uno::makeAny(static_cast<sal_Int32>(0))}
    }));
    createDoc("para-autostyle-char-props.fodt", aFilterData);

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This failed, para-level char props were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p[1]/xhtml:span", "class", "span0");
    assertXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span", "class", "span1");
}

void EPUBExportTest::testMeta()
{
    createDoc("meta.fodt", {});

    mpXmlDoc = parseExport("OEBPS/content.opf");
    // This was "Unknown Author", <meta:initial-creator> was not handled.
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:creator", "A U Thor");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:title", "Title");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:language", "hu");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/opf:meta[@property='dcterms:modified']", "2017-09-27T09:51:19Z");
}

void EPUBExportTest::testParaNamedstyle()
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
    {
        // Explicitly request in-CSS styles.
        {"EPUBStylesMethod", uno::makeAny(static_cast<sal_Int32>(0))}
    }));
    createDoc("para-namedstyle.fodt", aFilterData);

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPath(mpXmlDoc, "//xhtml:p[1]", "class", "para0");
    // This failed, paragraph properties from style were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p[2]", "class", "para1");

    // Test character properties from named paragraph style.
    assertXPath(mpXmlDoc, "//xhtml:p[1]/xhtml:span", "class", "span0");
    // This failed, character properties from paragraph style were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span", "class", "span1");
}

void EPUBExportTest::testCharNamedstyle()
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
    {
        // Explicitly request in-CSS styles.
        {"EPUBStylesMethod", uno::makeAny(static_cast<sal_Int32>(0))}
    }));
    createDoc("char-namedstyle.fodt", aFilterData);

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");

    // Test character properties from named text style.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[1]", "class", "span0");
    // This failed, character properties from text style were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]", "class", "span1");
}

void EPUBExportTest::testNamedStyleInheritance()
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
    {
        // Explicitly request in-CSS styles.
        {"EPUBStylesMethod", uno::makeAny(static_cast<sal_Int32>(0))}
    }));
    createDoc("named-style-inheritance.fodt", aFilterData);

    // Find the CSS rule for the blue text.
    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    OString aBlue = getXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span[2]", "class").toUtf8();

    std::map< OString, std::vector<OString> > aCssDoc;
    parseCssExport("OEBPS/styles/stylesheet.css", aCssDoc);
    assertCss(aCssDoc, aBlue, "  color: #0000ff;");
    // This failed, the span only had the properties from its style, but not
    // from the style's parent(s).
    assertCss(aCssDoc, aBlue, "  font-family: 'Liberation Mono';");
}

void EPUBExportTest::testNestedSpan()
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
    {
        // Explicitly request in-CSS styles.
        {"EPUBStylesMethod", uno::makeAny(static_cast<sal_Int32>(0))}
    }));
    createDoc("nested-span.fodt", aFilterData);

    // Check textural content of nested span.
    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This crashed, span had no content.
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:span[2]", "red");

    // Check formatting of nested span.
    OString aRed = getXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]", "class").toUtf8();
    std::map< OString, std::vector<OString> > aCssDoc;
    parseCssExport("OEBPS/styles/stylesheet.css", aCssDoc);
    // This failed, direct formatting on top of named style was lost.
    assertCss(aCssDoc, aRed, "  color: #ff0000;");
    assertCss(aCssDoc, aRed, "  font-family: 'Liberation Mono';");
}

void EPUBExportTest::testLineBreak()
{
    createDoc("line-break.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This was 0, line break was not handled.
    assertXPath(mpXmlDoc, "//xhtml:p[1]/xhtml:span/xhtml:br", 1);
    // This was 0, line break inside span was not handled.
    assertXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span/xhtml:br", 1);
}

void EPUBExportTest::testEscape()
{
    createDoc("escape.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This was lost.
    assertXPathContent(mpXmlDoc, "//xhtml:p[1]/xhtml:span[1]", OUString::fromUtf8("\xc2\xa0"));
    // Make sure escaping happens only once.
    assertXPathContent(mpXmlDoc, "//xhtml:p[1]/xhtml:span[2]", "a&b");
    // This was also lost.
    assertXPathContent(mpXmlDoc, "//xhtml:p[1]/xhtml:span[3]", OUString::fromUtf8("\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0 "));
}

void EPUBExportTest::testParaCharProps()
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
    {
        // Explicitly request in-CSS styles.
        {"EPUBStylesMethod", uno::makeAny(static_cast<sal_Int32>(0))}
    }));
    createDoc("para-char-props.fodt", aFilterData);

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // Check formatting of the middle span.
    OString aMiddle = getXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]", "class").toUtf8();
    std::map< OString, std::vector<OString> > aCssDoc;
    parseCssExport("OEBPS/styles/stylesheet.css", aCssDoc);
    assertCss(aCssDoc, aMiddle, "  font-style: italic;");
    // Direct para formatting was lost, only direct char formatting was
    // written, so this failed.
    assertCss(aCssDoc, aMiddle, "  font-weight: bold;");
}

void EPUBExportTest::testSection()
{
    createDoc("section.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This was "After.", i.e. in-section content was ignored.
    assertXPathContent(mpXmlDoc, "//xhtml:p[2]/xhtml:span", "In section.");
}

void EPUBExportTest::testList()
{
    createDoc("list.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This was "C", i.e. in-list content was ignored.
    assertXPathContent(mpXmlDoc, "//xhtml:p[2]/xhtml:span", "B");
    // Test nested list content.
    assertXPathContent(mpXmlDoc, "//xhtml:p[6]/xhtml:span", "F");
}

void EPUBExportTest::testImage()
{
    createDoc("image.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:img", 1);
}

void EPUBExportTest::testTable()
{
    createDoc("table.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr/xhtml:td", 4);
}

void EPUBExportTest::testTableRowSpan()
{
    createDoc("table-row-span.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This failed, row span wasn't exported.
    assertXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[1]", "rowspan", "2");
}

void EPUBExportTest::testTableCellBorder()
{
    createDoc("table-cell-border.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    OUString aStyle = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[1]", "style");
    std::map<OUString, OUString> aCss;
    parseCssStyle(aStyle, aCss);
    // This failed, cell border wasn't exported.
    CPPUNIT_ASSERT_EQUAL(OUString("0.05pt solid #000000"), aCss["border-left"]);
}

namespace
{
double getCellWidth(const OUString &rStyle)
{
    std::map<OUString, OUString> aCss;
    EPUBExportTest::parseCssStyle(rStyle, aCss);
    return aCss["width"].toDouble();
}

double getRowHeight(const OUString &rStyle)
{
    std::map<OUString, OUString> aCss;
    EPUBExportTest::parseCssStyle(rStyle, aCss);
    return aCss["height"].toDouble();
}
}

void EPUBExportTest::testTableCellWidth()
{
    createDoc("table-cell-width.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    OUString aStyle1 = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[1]", "style");
    OUString aStyle2 = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[2]", "style");
    OUString aStyle3 = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[3]", "style");
    // These failed, all widths were 0.
    CPPUNIT_ASSERT_GREATER(getCellWidth(aStyle2), getCellWidth(aStyle1));
    CPPUNIT_ASSERT_GREATER(getCellWidth(aStyle3), getCellWidth(aStyle1));
}

void EPUBExportTest::testTableRowHeight()
{
    createDoc("table-row-height.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    OUString aStyle1 = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]", "style");
    OUString aStyle2 = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[2]", "style");
    // These failed, both heights were 0.
    CPPUNIT_ASSERT_GREATER(getRowHeight(aStyle2), getRowHeight(aStyle1));
}

void EPUBExportTest::testLink()
{
    createDoc("link.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span", "https://libreoffice.org/");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a", "href", "https://libreoffice.org/");
}

void EPUBExportTest::testLinkCharFormat()
{
    createDoc("link-charformat.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // <span> was lost, link text having a char format was missing.
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span", "https://libreoffice.org/");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a", "href", "https://libreoffice.org/");
}

void EPUBExportTest::testLinkNamedCharFormat()
{
    // Character properties from named character style on hyperlink was lost.
    createDoc("link-namedcharformat.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This failed, there was no span inside the hyperlink.
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span", "http://libreoffice.org");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a", "href", "http://libreoffice.org/");

    OUString aStyle = getXPath(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span", "style");
    std::map<OUString, OUString> aCss;
    parseCssStyle(aStyle, aCss);
    CPPUNIT_ASSERT_EQUAL(OUString("#ff0000"), aCss["color"]);
}

CPPUNIT_TEST_SUITE_REGISTRATION(EPUBExportTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
