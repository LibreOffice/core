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
class EPUBExportTest : public test::BootstrapFixture,
                       public unotest::MacrosTest,
                       public XmlTestTools
{
protected:
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;
    utl::TempFile maTempFile;
    xmlDocPtr mpXmlDoc = nullptr;
    uno::Reference<packages::zip::XZipFileAccess2> mxZipFile;
    OUString maFilterOptions;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    void createDoc(const OUString& rFile, const uno::Sequence<beans::PropertyValue>& rFilterData);
    /// Returns an XML representation of the stream named rName in the exported package.
    xmlDocPtr parseExport(const OUString& rName);
    /// Parses a CSS representation of the stream named rName and returns it.
    std::map<OUString, std::vector<OUString>> parseCss(const OUString& rName);
    /// Looks up a key of a class in rCss.
    static OUString getCss(std::map<OUString, std::vector<OUString>>& rCss, const OUString& rClass,
                           const OUString& rKey);
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

void EPUBExportTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dc"), BAD_CAST("http://purl.org/dc/elements/1.1/"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("opf"), BAD_CAST("http://www.idpf.org/2007/opf"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xhtml"), BAD_CAST("http://www.w3.org/1999/xhtml"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("svg"), BAD_CAST("http://www.w3.org/2000/svg"));
}

void EPUBExportTest::createDoc(const OUString& rFile,
                               const uno::Sequence<beans::PropertyValue>& rFilterData)
{
    // Import the bugdoc and export as EPUB.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + rFile;
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    maTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("EPUB");
    if (maFilterOptions.isEmpty())
        aMediaDescriptor["FilterData"] <<= rFilterData;
    else
        aMediaDescriptor["FilterOptions"] <<= maFilterOptions;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mxZipFile
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, maTempFile.GetURL());
}

xmlDocPtr EPUBExportTest::parseExport(const OUString& rName)
{
    uno::Reference<io::XInputStream> xInputStream(mxZipFile->getByName(rName), uno::UNO_QUERY);
    std::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    return parseXmlStream(pStream.get());
}

std::map<OUString, std::vector<OUString>> EPUBExportTest::parseCss(const OUString& rName)
{
    std::map<OUString, std::vector<OUString>> aRet;

    uno::Reference<io::XInputStream> xInputStream(mxZipFile->getByName(rName), uno::UNO_QUERY);
    std::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));

    // Minimal CSS handler till orcus is up to our needs.
    OString aLine;
    OUString aRuleName;
    while (!pStream->eof())
    {
        pStream->ReadLine(aLine);
        if (aLine.endsWith("{"))
            // '.name {' -> 'name'
            aRuleName = OUString::fromUtf8(aLine.copy(1, aLine.getLength() - 3));
        else if (aLine.endsWith(";"))
            aRet[aRuleName].push_back(OUString::fromUtf8(aLine));
    }

    return aRet;
}

OUString EPUBExportTest::getCss(std::map<OUString, std::vector<OUString>>& rCss,
                                const OUString& rClass, const OUString& rKey)
{
    OUString aRet;

    auto it = rCss.find(rClass);
    CPPUNIT_ASSERT(it != rCss.end());

    for (const auto& rKeyValue : it->second)
    {
        OUString aKeyValue = rKeyValue.trim();
        std::vector<OUString> aTokens = comphelper::string::split(aKeyValue, ':');
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aTokens.size());
        if (aTokens[0].trim() == rKey)
        {
            aRet = aTokens[1].trim();
            if (aRet.endsWith(";"))
                // Ignore trailing semicolon.
                aRet = aRet.copy(0, aRet.getLength() - 1);
            break;
        }
    }

    return aRet;
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testOutlineLevel)
{
    createDoc("outline-level.fodt", {});

    // Make sure that the output is split into two.
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/sections/section0001.xhtml"));
    // This failed, output was a single section.
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/sections/section0002.xhtml"));
    CPPUNIT_ASSERT(!mxZipFile->hasByName("OEBPS/sections/section0003.xhtml"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testMimetype)
{
    createDoc("hello.fodt", {});

    // Check that the mime type is written uncompressed at the expected location.
    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemoryStream;
    aMemoryStream.WriteStream(aFileStream);
    OString aExpected("application/epub+zip");
    CPPUNIT_ASSERT(aMemoryStream.GetSize() > static_cast<sal_uInt64>(aExpected.getLength()) + 38);

    OString aActual(static_cast<const char*>(aMemoryStream.GetData()) + 38, aExpected.getLength());
    // This failed: actual data was some garbage, not the uncompressed mime type.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    mpXmlDoc = parseExport("OEBPS/content.opf");
    // Default is EPUB3.
    assertXPath(mpXmlDoc, "/opf:package", "version", "3.0");

    // This was just "libepubgen/x.y.z", i.e. the LO version was missing.
    OUString aGenerator
        = getXPath(mpXmlDoc, "/opf:package/opf:metadata/opf:meta[@name='generator']", "content");
    CPPUNIT_ASSERT(aGenerator.startsWith(utl::DocInfoHelper::GetGeneratorString()));

    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponentContext->getServiceManager(),
                                                    uno::UNO_QUERY);
    const OUString aServiceName("com.sun.star.comp.Writer.EPUBExportFilter");
    uno::Reference<document::XFilter> xFilter(xMSF->createInstance(aServiceName), uno::UNO_QUERY);
    // Should result in no errors.
    xFilter->cancel();
    // We got back what we expected.
    uno::Reference<lang::XServiceInfo> xServiceInfo(xFilter, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(aServiceName, xServiceInfo->getImplementationName());
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.document.ExportFilter"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testEPUB2)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { // Explicitly request EPUB2.
          { "EPUBVersion", uno::makeAny(static_cast<sal_Int32>(20)) } }));
    createDoc("hello.fodt", aFilterData);

    mpXmlDoc = parseExport("OEBPS/content.opf");
    // This was 3.0, EPUBVersion filter option was ignored and we always emitted EPUB3.
    assertXPath(mpXmlDoc, "/opf:package", "version", "2.0");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testEPUBFixedLayout)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { // Explicitly request fixed layout.
          { "EPUBLayoutMethod",
            uno::makeAny(static_cast<sal_Int32>(libepubgen::EPUB_LAYOUT_METHOD_FIXED)) } }));
    createDoc("hello.fodt", aFilterData);

    mpXmlDoc = parseExport("OEBPS/content.opf");
    // This was missing, EPUBLayoutMethod filter option was ignored and we always emitted reflowable layout.
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/opf:meta[@property='rendition:layout']",
                       "pre-paginated");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testEPUBFixedLayoutOption)
{
    // Explicitly request fixed layout, this time via FilterOptions.
    maFilterOptions = "layout=fixed";
    createDoc("hello.fodt", {});

    // This failed, fixed layout was only working via the FilterData map.
    mpXmlDoc = parseExport("OEBPS/content.opf");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/opf:meta[@property='rendition:layout']",
                       "pre-paginated");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testEPUBFixedLayoutImplicitBreak)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { // Explicitly request fixed layout.
          { "EPUBLayoutMethod",
            uno::makeAny(static_cast<sal_Int32>(libepubgen::EPUB_LAYOUT_METHOD_FIXED)) } }));
    createDoc("fxl-2page.fodt", aFilterData);

    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/sections/section0001.xhtml"));
    // This was missing, implicit page break (as calculated by the layout) was lost on export.
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/sections/section0002.xhtml"));
    CPPUNIT_ASSERT(!mxZipFile->hasByName("OEBPS/sections/section0003.xhtml"));

    // Make sure that fixed layout has chapter names in the navigation
    // document.
    mpXmlDoc = parseExport("OEBPS/toc.xhtml");
    // This was 'Page 1' instead.
    assertXPathContent(mpXmlDoc, "//xhtml:li[1]/xhtml:a", "First chapter");
    assertXPathContent(mpXmlDoc, "//xhtml:li[2]/xhtml:a", "Second chapter");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testPageBreakSplit)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { // Explicitly request split on page break (instead of on heading).
          { "EPUBSplitMethod",
            uno::makeAny(static_cast<sal_Int32>(libepubgen::EPUB_SPLIT_METHOD_PAGE_BREAK)) } }));
    createDoc("2pages.fodt", aFilterData);

    // Make sure that the output is split into two.
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/sections/section0001.xhtml"));
    // This failed, output was a single section.
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/sections/section0002.xhtml"));
    CPPUNIT_ASSERT(!mxZipFile->hasByName("OEBPS/sections/section0003.xhtml"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testSpanAutostyle)
{
    createDoc("span-autostyle.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[1]", "class", "span0");
    // This failed, it was still span1, i.e. the bold and the italic formatting
    // did not differ.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]", "class", "span1");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[3]", "class", "span2");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testParaAutostyleCharProps)
{
    createDoc("para-autostyle-char-props.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This failed, para-level char props were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p[1]/xhtml:span", "class", "span0");
    assertXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span", "class", "span1");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testMeta)
{
    createDoc("meta.fodt", {});

    mpXmlDoc = parseExport("OEBPS/content.opf");
    // This was "Unknown Author", <meta:initial-creator> was not handled.
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:creator", "A U Thor");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:title", "Title");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:language", "hu");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/opf:meta[@property='dcterms:modified']",
                       "2017-09-27T09:51:19Z");

    // Make sure that cover image next to the source document is picked up.
    assertXPath(mpXmlDoc, "/opf:package/opf:manifest/opf:item[@href='images/image0001.png']",
                "properties", "cover-image");
    assertXPath(mpXmlDoc, "/opf:package/opf:manifest/opf:item[@href='images/image0001.png']",
                "media-type", "image/png");
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/images/image0001.png"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testMetaXMP)
{
    createDoc("meta-xmp.fodt", {});
    mpXmlDoc = parseExport("OEBPS/content.opf");

    // These were the libepubgen default values, metadata from a matching .xmp file was not picked up.
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:identifier",
                       "deadbeef-e394-4cd6-9b83-7172794612e5");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:title", "unknown title from xmp");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:creator", "unknown author from xmp");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:language", "nl");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/opf:meta[@property='dcterms:modified']",
                       "2016-11-20T17:16:07Z");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testMetaAPI)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "RVNGIdentifier", uno::makeAny(OUString("deadc0de-e394-4cd6-9b83-7172794612e5")) },
          { "RVNGTitle", uno::makeAny(OUString("unknown title from api")) },
          { "RVNGInitialCreator", uno::makeAny(OUString("unknown author from api")) },
          { "RVNGLanguage", uno::makeAny(OUString("hu")) },
          { "RVNGDate", uno::makeAny(OUString("2015-11-20T17:16:07Z")) } }));
    createDoc("meta-xmp.fodt", aFilterData);
    mpXmlDoc = parseExport("OEBPS/content.opf");

    // These were values from XMP (deadbeef, etc.), not from API.
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:identifier",
                       "deadc0de-e394-4cd6-9b83-7172794612e5");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:title", "unknown title from api");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:creator", "unknown author from api");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:language", "hu");
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/opf:meta[@property='dcterms:modified']",
                       "2015-11-20T17:16:07Z");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testCoverImage)
{
    OUString aCoverURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "meta.cover-image.png";
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "RVNGCoverImage", uno::makeAny(aCoverURL) } }));
    createDoc("hello.fodt", aFilterData);
    mpXmlDoc = parseExport("OEBPS/content.opf");

    // Make sure that the explicitly set cover image is used.
    // This failed, as the image was not part of the package.
    assertXPath(mpXmlDoc, "/opf:package/opf:manifest/opf:item[@href='images/image0001.png']",
                "properties", "cover-image");
    assertXPath(mpXmlDoc, "/opf:package/opf:manifest/opf:item[@href='images/image0001.png']",
                "media-type", "image/png");
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/images/image0001.png"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testParaNamedstyle)
{
    createDoc("para-namedstyle.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPath(mpXmlDoc, "//xhtml:p[1]", "class", "para0");
    // This failed, paragraph properties from style were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p[2]", "class", "para1");

    // Test character properties from named paragraph style.
    assertXPath(mpXmlDoc, "//xhtml:p[1]/xhtml:span", "class", "span0");
    // This failed, character properties from paragraph style were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span", "class", "span1");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testCharNamedstyle)
{
    createDoc("char-namedstyle.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");

    // Test character properties from named text style.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[1]", "class", "span0");
    // This failed, character properties from text style were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]", "class", "span1");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testNamedStyleInheritance)
{
    createDoc("named-style-inheritance.fodt", {});

    // Find the CSS rule for the blue text.
    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");
    OUString aBlue = getXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span[2]", "class");

    CPPUNIT_ASSERT_EQUAL(OUString("#0000ff"), EPUBExportTest::getCss(aCssDoc, aBlue, "color"));
    // This failed, the span only had the properties from its style, but not
    // from the style's parent(s).
    CPPUNIT_ASSERT_EQUAL(OUString("'Liberation Mono'"),
                         EPUBExportTest::getCss(aCssDoc, aBlue, "font-family"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testNestedSpan)
{
    createDoc("nested-span.fodt", {});

    // Check textural content of nested span.
    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");
    // This crashed, span had no content.
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:span[2]", "red");

    // Check formatting of nested span.
    OUString aRed = getXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]", "class");
    // This failed, direct formatting on top of named style was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("#ff0000"), EPUBExportTest::getCss(aCssDoc, aRed, "color"));
    CPPUNIT_ASSERT_EQUAL(OUString("'Liberation Mono'"),
                         EPUBExportTest::getCss(aCssDoc, aRed, "font-family"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testLineBreak)
{
    createDoc("line-break.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This was 0, line break was not handled.
    assertXPath(mpXmlDoc, "//xhtml:p[1]/xhtml:span/xhtml:br", 1);
    // This was 0, line break inside span was not handled.
    assertXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span/xhtml:br", 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testEscape)
{
    createDoc("escape.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This was lost.
    assertXPathContent(mpXmlDoc, "//xhtml:p[1]/xhtml:span[1]", OUString::fromUtf8("\xc2\xa0"));
    // Make sure escaping happens only once.
    assertXPathContent(mpXmlDoc, "//xhtml:p[1]/xhtml:span[2]", "a&b");
    // This was also lost.
    assertXPathContent(
        mpXmlDoc, "//xhtml:p[1]/xhtml:span[3]",
        OUString::fromUtf8("\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2"
                           "\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0 "));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testParaCharProps)
{
    createDoc("para-char-props.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");
    // Check formatting of the middle span.
    OUString aMiddle = getXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]", "class");
    CPPUNIT_ASSERT_EQUAL(OUString("italic"),
                         EPUBExportTest::getCss(aCssDoc, aMiddle, "font-style"));
    // Direct para formatting was lost, only direct char formatting was
    // written, so this failed.
    CPPUNIT_ASSERT_EQUAL(OUString("bold"), EPUBExportTest::getCss(aCssDoc, aMiddle, "font-weight"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testSection)
{
    createDoc("section.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This was "After.", i.e. in-section content was ignored.
    assertXPathContent(mpXmlDoc, "//xhtml:p[2]/xhtml:span", "In section.");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testList)
{
    createDoc("list.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This was "C", i.e. in-list content was ignored.
    assertXPathContent(mpXmlDoc, "//xhtml:p[2]/xhtml:span", "B");
    // Test nested list content.
    assertXPathContent(mpXmlDoc, "//xhtml:p[6]/xhtml:span", "F");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testImage)
{
    createDoc("image.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:img", 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testImageBorder)
{
    createDoc("image-border.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");

    OUString aClass = getXPath(mpXmlDoc, "//xhtml:img", "class");
    // This failed, image had no border.
    CPPUNIT_ASSERT_EQUAL(OUString("0.99pt dashed #ed1c24"),
                         EPUBExportTest::getCss(aCssDoc, aClass, "border"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testImageNospan)
{
    createDoc("image-nospan.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // Image outside a span was lost.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:img", 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTable)
{
    createDoc("table.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr/xhtml:td", 4);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTableRowSpan)
{
    createDoc("table-row-span.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This failed, row span wasn't exported.
    assertXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[1]", "rowspan", "2");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTableCellBorder)
{
    createDoc("table-cell-border.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");

    OUString aClass
        = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[1]", "class");
    // This failed, cell border wasn't exported.
    CPPUNIT_ASSERT_EQUAL(OUString("0.05pt solid #000000"),
                         EPUBExportTest::getCss(aCssDoc, aClass, "border-left"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTableCellWidth)
{
    createDoc("table-cell-width.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");
    OUString aClass1
        = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[1]", "class");
    OUString aClass2
        = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[2]", "class");
    OUString aClass3
        = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[3]", "class");
    // These failed, all widths were 0.
    CPPUNIT_ASSERT_GREATER(EPUBExportTest::getCss(aCssDoc, aClass2, "width").toDouble(),
                           EPUBExportTest::getCss(aCssDoc, aClass1, "width").toDouble());
    CPPUNIT_ASSERT_GREATER(EPUBExportTest::getCss(aCssDoc, aClass3, "width").toDouble(),
                           EPUBExportTest::getCss(aCssDoc, aClass1, "width").toDouble());
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTableRowHeight)
{
    createDoc("table-row-height.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");
    OUString aClass1 = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]", "class");
    OUString aClass2 = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[2]", "class");
    // These failed, both heights were 0.
    CPPUNIT_ASSERT_GREATER(EPUBExportTest::getCss(aCssDoc, aClass2, "height").toDouble(),
                           EPUBExportTest::getCss(aCssDoc, aClass1, "height").toDouble());
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testLink)
{
    createDoc("link.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span", "https://libreoffice.org/");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a", "href", "https://libreoffice.org/");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testLinkInvalid)
{
    createDoc("link-invalid.odt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This was 1, invalid relative link was not filtered out.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a", 0);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testLinkCharFormat)
{
    createDoc("link-charformat.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // <span> was lost, link text having a char format was missing.
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span", "https://libreoffice.org/");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a", "href", "https://libreoffice.org/");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testLinkNamedCharFormat)
{
    // Character properties from named character style on hyperlink was lost.
    createDoc("link-namedcharformat.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");
    // This failed, there was no span inside the hyperlink.
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span", "http://libreoffice.org");
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a", "href", "http://libreoffice.org/");

    OUString aClass = getXPath(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span", "class");
    CPPUNIT_ASSERT_EQUAL(OUString("#ff0000"), EPUBExportTest::getCss(aCssDoc, aClass, "color"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTableWidth)
{
    createDoc("table-width.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");

    OUString aClass = getXPath(mpXmlDoc, "//xhtml:table", "class");
    // This failed, relative total width of table was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("50%"), EPUBExportTest::getCss(aCssDoc, aClass, "width"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTextBox)
{
    createDoc("text-box.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");

    // This failed, image with caption was lost.
    assertXPath(mpXmlDoc, "//xhtml:img", "class", "frame1");
    // Expected spans:
    // 1) break after the image
    // 2) "Illustration "
    // 3) The sequence field, this was missing (was ": foo" instead).
    assertXPathContent(mpXmlDoc, "//xhtml:div/xhtml:p/xhtml:span[3]", "1");

    OUString aClass = getXPath(mpXmlDoc, "//xhtml:div/xhtml:p/xhtml:span[3]", "class");
    // This failed, the 3rd span was not italic.
    CPPUNIT_ASSERT_EQUAL(OUString("italic"), EPUBExportTest::getCss(aCssDoc, aClass, "font-style"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testFontEmbedding)
{
#if !defined(MACOSX)
    createDoc("font-embedding.fodt", {});

    // Make sure that the params of defineEmbeddedFont() are all handled.
    // librevenge:name
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");
    // 'SketchFlow Print' or ''SketchFlow Print1'
    CPPUNIT_ASSERT(EPUBExportTest::getCss(aCssDoc, "font-face", "font-family")
                       .startsWith("'SketchFlow Print"));
    // librevenge:mime-type
    mpXmlDoc = parseExport("OEBPS/content.opf");
    assertXPath(mpXmlDoc, "/opf:package/opf:manifest/opf:item[@href='fonts/font0001.otf']",
                "media-type", "application/vnd.ms-opentype");
    // office:binary-data
    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/fonts/font0001.otf"));
    // librevenge:font-style
    CPPUNIT_ASSERT_EQUAL(OUString("normal"),
                         EPUBExportTest::getCss(aCssDoc, "font-face", "font-style"));
    // librevenge:font-weight
    CPPUNIT_ASSERT_EQUAL(OUString("normal"),
                         EPUBExportTest::getCss(aCssDoc, "font-face", "font-weight"));
#endif
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testImageLink)
{
    createDoc("image-link.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // This failed, image was missing.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:img", 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testFootnote)
{
    createDoc("footnote.fodt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // These were missing, footnote was lost.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p/xhtml:sup/xhtml:a", "type", "noteref");
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:aside", "type", "footnote");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testPopup)
{
    createDoc("popup.odt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // Test image popup anchor.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p[1]/xhtml:a", "type", "noteref");
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p[1]/xhtml:a/xhtml:img", 1);
    // Test image popup content.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:aside[1]", "type", "footnote");
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:aside[1]/xhtml:img", 1);

    // Test text popup anchor.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p[2]/xhtml:span/xhtml:a", "type", "noteref");
    assertXPathContent(mpXmlDoc, "//xhtml:body/xhtml:p[2]/xhtml:span/xhtml:a", "link");
    // Test text popup content.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:aside[2]", "type", "footnote");
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:aside[2]/xhtml:img", 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testPopupMedia)
{
    // This is the same as testPopup(), but the links point to images in the
    // default media directory, not in the document directory.
    createDoc("popup-media.odt", {});

    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // Test image popup anchor. This failed, number of XPath nodes was 0.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p[1]/xhtml:a", "type", "noteref");
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p[1]/xhtml:a/xhtml:img", 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testPopupAPI)
{
    // Make sure that the popup works with data from a media directory.
    OUString aMediaDir = m_directories.getURLFromSrc(DATA_DIRECTORY) + "popup";
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "RVNGMediaDir", uno::makeAny(aMediaDir) } }));
    createDoc("popup-api.odt", aFilterData);

    // We have a non-empty anchor image.
    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    OUString aAnchor = getXPath(mpXmlDoc, "//xhtml:body/xhtml:p[1]/xhtml:a/xhtml:img", "src");
    CPPUNIT_ASSERT(!aAnchor.isEmpty());
    // We have a non-empty popup image.
    OUString aData = getXPath(mpXmlDoc, "//xhtml:body/xhtml:aside[1]/xhtml:img", "src");
    CPPUNIT_ASSERT(!aData.isEmpty());
    // The anchor is different from the popup image.
    CPPUNIT_ASSERT(aAnchor != aData);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testPageSize)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "EPUBLayoutMethod",
            uno::makeAny(static_cast<sal_Int32>(libepubgen::EPUB_LAYOUT_METHOD_FIXED)) } }));
    createDoc("hello.fodt", aFilterData);

    // This failed, viewport was empty, so page size was lost.
    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    // 21,59cm x 27.94cm (letter).
    assertXPath(mpXmlDoc, "/xhtml:html/xhtml:head/xhtml:meta[@name='viewport']", "content",
                "width=816, height=1056");

    xmlFreeDoc(mpXmlDoc);
    mpXmlDoc = parseExport("OEBPS/images/image0001.svg");
    // This was 288mm, logic->logic conversion input was a pixel value.
    assertXPath(mpXmlDoc, "/svg:svg", "width", "216mm");
    assertXPath(mpXmlDoc, "/svg:svg", "height", "279mm");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testSVG)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "EPUBLayoutMethod",
            uno::makeAny(static_cast<sal_Int32>(libepubgen::EPUB_LAYOUT_METHOD_FIXED)) } }));
    createDoc("hello.fodt", aFilterData);

    CPPUNIT_ASSERT(mxZipFile->hasByName("OEBPS/images/image0001.svg"));
    uno::Reference<io::XInputStream> xInputStream(
        mxZipFile->getByName("OEBPS/images/image0001.svg"), uno::UNO_QUERY);
    std::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));

    SvMemoryStream aMemoryStream;
    aMemoryStream.WriteStream(*pStream);
    OString aExpected("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n<svg");
    CPPUNIT_ASSERT(aMemoryStream.GetSize() > static_cast<sal_uInt64>(aExpected.getLength()));

    // This failed, there was a '<!DOCTYPE' line between the xml and the svg
    // one, causing a validation error.
    OString aActual(static_cast<const char*>(aMemoryStream.GetData()), aExpected.getLength());
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    // This failed, we used the xlink attribute namespace, but we did not
    // define its URL.
    mpXmlDoc = parseExport("OEBPS/images/image0001.svg");
    assertXPathNSDef(mpXmlDoc, "/svg:svg", "xlink", "http://www.w3.org/1999/xlink");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTdf115623SingleWritingMode)
{
    // Simple page that has single writing mode should work.
    createDoc("tdf115623-single-writing-mode.odt", {});
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");
    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    OUString aClass = getXPath(mpXmlDoc, "//xhtml:body", "class");
    CPPUNIT_ASSERT_EQUAL(OUString("vertical-rl"),
                         EPUBExportTest::getCss(aCssDoc, aClass, "writing-mode"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTdf115623SplitByChapter)
{
    createDoc("tdf115623-split-by-chapter.odt", {});
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");
    {
        mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
        OUString aClass = getXPath(mpXmlDoc, "//xhtml:body", "class");
        CPPUNIT_ASSERT_EQUAL(OUString("vertical-rl"),
                             EPUBExportTest::getCss(aCssDoc, aClass, "writing-mode"));
        xmlFreeDoc(mpXmlDoc);
        mpXmlDoc = nullptr;
    }
    // Splitted HTML should keep the same writing-mode.
    {
        mpXmlDoc = parseExport("OEBPS/sections/section0002.xhtml");
        OUString aClass = getXPath(mpXmlDoc, "//xhtml:body", "class");
        CPPUNIT_ASSERT_EQUAL(OUString("vertical-rl"),
                             EPUBExportTest::getCss(aCssDoc, aClass, "writing-mode"));
    }
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTdf115623ManyPageSpans)
{
    createDoc("tdf115623-many-pagespans.odt", {});
    std::map<OUString, std::vector<OUString>> aCssDoc = parseCss("OEBPS/styles/stylesheet.css");
    // Two pages should have different writing modes.
    {
        mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
        OUString aClass = getXPath(mpXmlDoc, "//xhtml:body", "class");
        CPPUNIT_ASSERT_EQUAL(OUString("vertical-rl"),
                             EPUBExportTest::getCss(aCssDoc, aClass, "writing-mode"));
        xmlFreeDoc(mpXmlDoc);
        mpXmlDoc = nullptr;
    }
    {
        mpXmlDoc = parseExport("OEBPS/sections/section0002.xhtml");
        OUString aClass = getXPath(mpXmlDoc, "//xhtml:body", "class");
        CPPUNIT_ASSERT_EQUAL(OUString("horizontal-tb"),
                             EPUBExportTest::getCss(aCssDoc, aClass, "writing-mode"));
    }
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testSimpleRuby)
{
    createDoc("simple-ruby.odt", {});
    mpXmlDoc = parseExport("OEBPS/sections/section0001.xhtml");
    assertXPathContent(mpXmlDoc, "//xhtml:body/xhtml:p/xhtml:ruby/xhtml:span", "base text");
    assertXPathContent(mpXmlDoc, "//xhtml:body/xhtml:p/xhtml:ruby/xhtml:rt", "ruby text");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
