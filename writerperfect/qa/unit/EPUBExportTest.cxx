/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/unoapixml_test.hxx>

#include <string_view>

#include <libepubgen/libepubgen.h>

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <o3tl/safeint.hxx>
#include <unotools/docinfohelper.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <o3tl/string_view.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests the EPUB export filter.
class EPUBExportTest : public UnoApiXmlTest
{
protected:
    uno::Reference<packages::zip::XZipFileAccess2> mxZipFile;
    OUString maFilterOptions;

public:
    EPUBExportTest()
        : UnoApiXmlTest(u"/writerperfect/qa/unit/data/writer/epubexport/"_ustr)
    {
    }

    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    void createDoc(std::u16string_view rFile,
                   const uno::Sequence<beans::PropertyValue>& rFilterData);
    /// Parses a CSS representation of the stream named rName and returns it.
    std::map<OUString, std::vector<OUString>> parseCss(const OUString& rName);
    /// Looks up a key of a class in rCss.
    static OUString getCss(std::map<OUString, std::vector<OUString>>& rCss, const OUString& rClass,
                           std::u16string_view rKey);
};

void EPUBExportTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dc"), BAD_CAST("http://purl.org/dc/elements/1.1/"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("opf"), BAD_CAST("http://www.idpf.org/2007/opf"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xhtml"), BAD_CAST("http://www.w3.org/1999/xhtml"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("svg"), BAD_CAST("http://www.w3.org/2000/svg"));
}

void EPUBExportTest::createDoc(std::u16string_view rFile,
                               const uno::Sequence<beans::PropertyValue>& rFilterData)
{
    // Import the bugdoc and export as EPUB.
    loadFromFile(rFile);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor[u"FilterName"_ustr] <<= u"EPUB"_ustr;
    if (maFilterOptions.isEmpty())
        aMediaDescriptor[u"FilterData"_ustr] <<= rFilterData;
    else
        aMediaDescriptor[u"FilterOptions"_ustr] <<= maFilterOptions;
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mxZipFile
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, maTempFile.GetURL());
}

std::map<OUString, std::vector<OUString>> EPUBExportTest::parseCss(const OUString& rName)
{
    std::map<OUString, std::vector<OUString>> aRet;

    uno::Reference<io::XInputStream> xInputStream(mxZipFile->getByName(rName), uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));

    // Minimal CSS handler till orcus is up to our needs.
    OString aLine;
    OUString aRuleName;
    while (!pStream->eof())
    {
        pStream->ReadLine(aLine);
        if (aLine.endsWith("{"))
            // '.name {' -> 'name'
            aRuleName = OUString::fromUtf8(aLine.subView(1, aLine.getLength() - 3));
        else if (aLine.endsWith(";"))
            aRet[aRuleName].push_back(OUString::fromUtf8(aLine));
    }

    return aRet;
}

OUString EPUBExportTest::getCss(std::map<OUString, std::vector<OUString>>& rCss,
                                const OUString& rClass, std::u16string_view rKey)
{
    OUString aRet;

    auto it = rCss.find(rClass);
    CPPUNIT_ASSERT(it != rCss.end());

    for (const auto& rKeyValue : it->second)
    {
        OUString aKeyValue = rKeyValue.trim();
        std::vector<OUString> aTokens = comphelper::string::split(aKeyValue, ':');
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aTokens.size());
        if (o3tl::trim(aTokens[0]) == rKey)
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
    createDoc(u"outline-level.fodt", {});

    // Make sure that the output is split into two.
    CPPUNIT_ASSERT(mxZipFile->hasByName(u"OEBPS/sections/section0001.xhtml"_ustr));
    // This failed, output was a single section.
    CPPUNIT_ASSERT(mxZipFile->hasByName(u"OEBPS/sections/section0002.xhtml"_ustr));
    CPPUNIT_ASSERT(!mxZipFile->hasByName(u"OEBPS/sections/section0003.xhtml"_ustr));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testMimetype)
{
    createDoc(u"hello.fodt", {});

    // Check that the mime type is written uncompressed at the expected location.
    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemoryStream;
    aMemoryStream.WriteStream(aFileStream);
    OString aExpected("application/epub+zip"_ostr);
    CPPUNIT_ASSERT(aMemoryStream.GetSize() > static_cast<sal_uInt64>(aExpected.getLength()) + 38);

    OString aActual(static_cast<const char*>(aMemoryStream.GetData()) + 38, aExpected.getLength());
    // This failed: actual data was some garbage, not the uncompressed mime type.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/content.opf"_ustr);
    // Default is EPUB3.
    assertXPath(mpXmlDoc, "/opf:package"_ostr, "version"_ostr, u"3.0"_ustr);

    // This was just "libepubgen/x.y.z", i.e. the LO version was missing.
    OUString aGenerator = getXPath(
        mpXmlDoc, "/opf:package/opf:metadata/opf:meta[@name='generator']"_ostr, "content"_ostr);
    CPPUNIT_ASSERT(aGenerator.startsWith(utl::DocInfoHelper::GetGeneratorString()));

    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponentContext->getServiceManager(),
                                                    uno::UNO_QUERY);
    constexpr OUString aServiceName(u"com.sun.star.comp.Writer.EPUBExportFilter"_ustr);
    uno::Reference<document::XFilter> xFilter(xMSF->createInstance(aServiceName), uno::UNO_QUERY);
    // Should result in no errors.
    xFilter->cancel();
    // We got back what we expected.
    uno::Reference<lang::XServiceInfo> xServiceInfo(xFilter, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(aServiceName, xServiceInfo->getImplementationName());
    CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.document.ExportFilter"_ustr));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testEPUB2)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { // Explicitly request EPUB2.
          { "EPUBVersion", uno::Any(static_cast<sal_Int32>(20)) } }));
    createDoc(u"hello.fodt", aFilterData);

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/content.opf"_ustr);
    // This was 3.0, EPUBVersion filter option was ignored and we always emitted EPUB3.
    assertXPath(mpXmlDoc, "/opf:package"_ostr, "version"_ostr, u"2.0"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testEPUBFixedLayout)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { // Explicitly request fixed layout.
          { "EPUBLayoutMethod",
            uno::Any(static_cast<sal_Int32>(libepubgen::EPUB_LAYOUT_METHOD_FIXED)) } }));
    createDoc(u"hello.fodt", aFilterData);

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/content.opf"_ustr);
    // This was missing, EPUBLayoutMethod filter option was ignored and we always emitted reflowable layout.
    assertXPathContent(mpXmlDoc,
                       "/opf:package/opf:metadata/opf:meta[@property='rendition:layout']"_ostr,
                       u"pre-paginated"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testEPUBFixedLayoutOption)
{
    // Explicitly request fixed layout, this time via FilterOptions.
    maFilterOptions = "layout=fixed";
    createDoc(u"hello.fodt", {});

    // This failed, fixed layout was only working via the FilterData map.
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/content.opf"_ustr);
    assertXPathContent(mpXmlDoc,
                       "/opf:package/opf:metadata/opf:meta[@property='rendition:layout']"_ostr,
                       u"pre-paginated"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testEPUBFixedLayoutImplicitBreak)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { // Explicitly request fixed layout.
          { "EPUBLayoutMethod",
            uno::Any(static_cast<sal_Int32>(libepubgen::EPUB_LAYOUT_METHOD_FIXED)) } }));
    createDoc(u"fxl-2page.fodt", aFilterData);

    CPPUNIT_ASSERT(mxZipFile->hasByName(u"OEBPS/sections/section0001.xhtml"_ustr));
    // This was missing, implicit page break (as calculated by the layout) was lost on export.
    CPPUNIT_ASSERT(mxZipFile->hasByName(u"OEBPS/sections/section0002.xhtml"_ustr));
    CPPUNIT_ASSERT(!mxZipFile->hasByName(u"OEBPS/sections/section0003.xhtml"_ustr));

    // Make sure that fixed layout has chapter names in the navigation
    // document.
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/toc.xhtml"_ustr);
    // This was 'Page 1' instead.
    assertXPathContent(mpXmlDoc, "//xhtml:li[1]/xhtml:a"_ostr, u"First chapter"_ustr);
    assertXPathContent(mpXmlDoc, "//xhtml:li[2]/xhtml:a"_ostr, u"Second chapter"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testPageBreakSplit)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { // Explicitly request split on page break (instead of on heading).
          { "EPUBSplitMethod",
            uno::Any(static_cast<sal_Int32>(libepubgen::EPUB_SPLIT_METHOD_PAGE_BREAK)) } }));
    createDoc(u"2pages.fodt", aFilterData);

    // Make sure that the output is split into two.
    CPPUNIT_ASSERT(mxZipFile->hasByName(u"OEBPS/sections/section0001.xhtml"_ustr));
    // This failed, output was a single section.
    CPPUNIT_ASSERT(mxZipFile->hasByName(u"OEBPS/sections/section0002.xhtml"_ustr));
    CPPUNIT_ASSERT(!mxZipFile->hasByName(u"OEBPS/sections/section0003.xhtml"_ustr));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testSpanAutostyle)
{
    createDoc(u"span-autostyle.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[1]"_ostr, "class"_ostr, u"span0"_ustr);
    // This failed, it was still span1, i.e. the bold and the italic formatting
    // did not differ.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]"_ostr, "class"_ostr, u"span1"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[3]"_ostr, "class"_ostr, u"span2"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testParaAutostyleCharProps)
{
    createDoc(u"para-autostyle-char-props.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // This failed, para-level char props were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p[1]/xhtml:span"_ostr, "class"_ostr, u"span0"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span"_ostr, "class"_ostr, u"span1"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testMeta)
{
    createDoc(u"meta.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/content.opf"_ustr);
    // This was "Unknown Author", <meta:initial-creator> was not handled.
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:creator"_ostr, u"A U Thor"_ustr);
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:title"_ostr, u"Title"_ustr);
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:language"_ostr, u"hu"_ustr);
    assertXPathContent(mpXmlDoc,
                       "/opf:package/opf:metadata/opf:meta[@property='dcterms:modified']"_ostr,
                       u"2017-09-27T09:51:19Z"_ustr);

    // Make sure that cover image next to the source document is picked up.
    assertXPath(mpXmlDoc, "/opf:package/opf:manifest/opf:item[@href='images/image0001.png']"_ostr,
                "properties"_ostr, u"cover-image"_ustr);
    assertXPath(mpXmlDoc, "/opf:package/opf:manifest/opf:item[@href='images/image0001.png']"_ostr,
                "media-type"_ostr, u"image/png"_ustr);
    CPPUNIT_ASSERT(mxZipFile->hasByName(u"OEBPS/images/image0001.png"_ustr));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testMetaXMP)
{
    createDoc(u"meta-xmp.fodt", {});
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/content.opf"_ustr);

    // These were the libepubgen default values, metadata from a matching .xmp file was not picked up.
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:identifier"_ostr,
                       u"deadbeef-e394-4cd6-9b83-7172794612e5"_ustr);
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:title"_ostr,
                       u"unknown title from xmp"_ustr);
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:creator"_ostr,
                       u"unknown author from xmp"_ustr);
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:language"_ostr, u"nl"_ustr);
    assertXPathContent(mpXmlDoc,
                       "/opf:package/opf:metadata/opf:meta[@property='dcterms:modified']"_ostr,
                       u"2016-11-20T17:16:07Z"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testMetaAPI)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "RVNGIdentifier", uno::Any(u"deadc0de-e394-4cd6-9b83-7172794612e5"_ustr) },
          { "RVNGTitle", uno::Any(u"unknown title from api"_ustr) },
          { "RVNGInitialCreator", uno::Any(u"unknown author from api"_ustr) },
          { "RVNGLanguage", uno::Any(u"hu"_ustr) },
          { "RVNGDate", uno::Any(u"2015-11-20T17:16:07Z"_ustr) } }));
    createDoc(u"meta-xmp.fodt", aFilterData);
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/content.opf"_ustr);

    // These were values from XMP (deadbeef, etc.), not from API.
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:identifier"_ostr,
                       u"deadc0de-e394-4cd6-9b83-7172794612e5"_ustr);
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:title"_ostr,
                       u"unknown title from api"_ustr);
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:creator"_ostr,
                       u"unknown author from api"_ustr);
    assertXPathContent(mpXmlDoc, "/opf:package/opf:metadata/dc:language"_ostr, u"hu"_ustr);
    assertXPathContent(mpXmlDoc,
                       "/opf:package/opf:metadata/opf:meta[@property='dcterms:modified']"_ostr,
                       u"2015-11-20T17:16:07Z"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testCoverImage)
{
    OUString aCoverURL = createFileURL(u"meta.cover-image.png");
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "RVNGCoverImage", uno::Any(aCoverURL) } }));
    createDoc(u"hello.fodt", aFilterData);
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/content.opf"_ustr);

    // Make sure that the explicitly set cover image is used.
    // This failed, as the image was not part of the package.
    assertXPath(mpXmlDoc, "/opf:package/opf:manifest/opf:item[@href='images/image0001.png']"_ostr,
                "properties"_ostr, u"cover-image"_ustr);
    assertXPath(mpXmlDoc, "/opf:package/opf:manifest/opf:item[@href='images/image0001.png']"_ostr,
                "media-type"_ostr, u"image/png"_ustr);
    CPPUNIT_ASSERT(mxZipFile->hasByName(u"OEBPS/images/image0001.png"_ustr));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testParaNamedstyle)
{
    createDoc(u"para-namedstyle.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:p[1]"_ostr, "class"_ostr, u"para0"_ustr);
    // This failed, paragraph properties from style were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p[2]"_ostr, "class"_ostr, u"para1"_ustr);

    // Test character properties from named paragraph style.
    assertXPath(mpXmlDoc, "//xhtml:p[1]/xhtml:span"_ostr, "class"_ostr, u"span0"_ustr);
    // This failed, character properties from paragraph style were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span"_ostr, "class"_ostr, u"span1"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testCharNamedstyle)
{
    createDoc(u"char-namedstyle.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);

    // Test character properties from named text style.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[1]"_ostr, "class"_ostr, u"span0"_ustr);
    // This failed, character properties from text style were not exported.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]"_ostr, "class"_ostr, u"span1"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testNamedStyleInheritance)
{
    createDoc(u"named-style-inheritance.fodt", {});

    // Find the CSS rule for the blue text.
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);
    OUString aBlue = getXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span[2]"_ostr, "class"_ostr);

    CPPUNIT_ASSERT_EQUAL(u"#0000ff"_ustr, EPUBExportTest::getCss(aCssDoc, aBlue, u"color"));
    // This failed, the span only had the properties from its style, but not
    // from the style's parent(s).
    CPPUNIT_ASSERT_EQUAL(u"'Liberation Mono'"_ustr,
                         EPUBExportTest::getCss(aCssDoc, aBlue, u"font-family"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testNestedSpan)
{
    createDoc(u"nested-span.fodt", {});

    // Check textural content of nested span.
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);
    // This crashed, span had no content.
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:span[2]"_ostr, u"red"_ustr);

    // Check formatting of nested span.
    OUString aRed = getXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]"_ostr, "class"_ostr);
    // This failed, direct formatting on top of named style was lost.
    CPPUNIT_ASSERT_EQUAL(u"#ff0000"_ustr, EPUBExportTest::getCss(aCssDoc, aRed, u"color"));
    CPPUNIT_ASSERT_EQUAL(u"'Liberation Mono'"_ustr,
                         EPUBExportTest::getCss(aCssDoc, aRed, u"font-family"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testLineBreak)
{
    createDoc(u"line-break.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // This was 0, line break was not handled.
    assertXPath(mpXmlDoc, "//xhtml:p[1]/xhtml:span/xhtml:br"_ostr, 1);
    // This was 0, line break inside span was not handled.
    assertXPath(mpXmlDoc, "//xhtml:p[2]/xhtml:span/xhtml:br"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testEscape)
{
    createDoc(u"escape.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // This was lost.
    assertXPathContent(mpXmlDoc, "//xhtml:p[1]/xhtml:span[1]"_ostr, OUString::fromUtf8("\xc2\xa0"));
    // Make sure escaping happens only once.
    assertXPathContent(mpXmlDoc, "//xhtml:p[1]/xhtml:span[2]"_ostr, u"a&b"_ustr);
    // This was also lost.
    assertXPathContent(
        mpXmlDoc, "//xhtml:p[1]/xhtml:span[3]"_ostr,
        OUString::fromUtf8("\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2"
                           "\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0 "));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testParaCharProps)
{
    createDoc(u"para-char-props.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);
    // Check formatting of the middle span.
    OUString aMiddle = getXPath(mpXmlDoc, "//xhtml:p/xhtml:span[2]"_ostr, "class"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"italic"_ustr, EPUBExportTest::getCss(aCssDoc, aMiddle, u"font-style"));
    // Direct para formatting was lost, only direct char formatting was
    // written, so this failed.
    CPPUNIT_ASSERT_EQUAL(u"bold"_ustr, EPUBExportTest::getCss(aCssDoc, aMiddle, u"font-weight"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testSection)
{
    createDoc(u"section.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // This was "After.", i.e. in-section content was ignored.
    assertXPathContent(mpXmlDoc, "//xhtml:p[2]/xhtml:span"_ostr, u"In section."_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testList)
{
    createDoc(u"list.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // This was "C", i.e. in-list content was ignored.
    assertXPathContent(mpXmlDoc, "//xhtml:p[2]/xhtml:span"_ostr, u"B"_ustr);
    // Test nested list content.
    assertXPathContent(mpXmlDoc, "//xhtml:p[6]/xhtml:span"_ostr, u"F"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testImage)
{
    createDoc(u"image.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:img"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testImageBorder)
{
    createDoc(u"image-border.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);

    OUString aClass = getXPath(mpXmlDoc, "//xhtml:img"_ostr, "class"_ostr);
    // This failed, image had no border.
    CPPUNIT_ASSERT_EQUAL(u"0.99pt dashed #ed1c24"_ustr,
                         EPUBExportTest::getCss(aCssDoc, aClass, u"border"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testImageNospan)
{
    createDoc(u"image-nospan.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // Image outside a span was lost.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:img"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTable)
{
    createDoc(u"table.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr/xhtml:td"_ostr, 4);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTableRowSpan)
{
    createDoc(u"table-row-span.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // This failed, row span wasn't exported.
    assertXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[1]"_ostr, "rowspan"_ostr,
                u"2"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTableCellBorder)
{
    createDoc(u"table-cell-border.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);

    OUString aClass = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[1]"_ostr,
                               "class"_ostr);
    // This failed, cell border wasn't exported.
    CPPUNIT_ASSERT_EQUAL(u"0.05pt solid #000000"_ustr,
                         EPUBExportTest::getCss(aCssDoc, aClass, u"border-left"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTableCellWidth)
{
    createDoc(u"table-cell-width.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);
    OUString aClass1 = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[1]"_ostr,
                                "class"_ostr);
    OUString aClass2 = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[2]"_ostr,
                                "class"_ostr);
    OUString aClass3 = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]/xhtml:td[3]"_ostr,
                                "class"_ostr);
    // These failed, all widths were 0.
    CPPUNIT_ASSERT_GREATER(EPUBExportTest::getCss(aCssDoc, aClass2, u"width").toDouble(),
                           EPUBExportTest::getCss(aCssDoc, aClass1, u"width").toDouble());
    CPPUNIT_ASSERT_GREATER(EPUBExportTest::getCss(aCssDoc, aClass3, u"width").toDouble(),
                           EPUBExportTest::getCss(aCssDoc, aClass1, u"width").toDouble());
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTableRowHeight)
{
    createDoc(u"table-row-height.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);
    OUString aClass1
        = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[1]"_ostr, "class"_ostr);
    OUString aClass2
        = getXPath(mpXmlDoc, "//xhtml:table/xhtml:tbody/xhtml:tr[2]"_ostr, "class"_ostr);
    // These failed, both heights were 0.
    CPPUNIT_ASSERT_GREATER(EPUBExportTest::getCss(aCssDoc, aClass2, u"height").toDouble(),
                           EPUBExportTest::getCss(aCssDoc, aClass1, u"height").toDouble());
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testLink)
{
    createDoc(u"link.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span"_ostr,
                       u"https://libreoffice.org/"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a"_ostr, "href"_ostr, u"https://libreoffice.org/"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testLinkInvalid)
{
    createDoc(u"link-invalid.odt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // This was 1, invalid relative link was not filtered out.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testLinkCharFormat)
{
    createDoc(u"link-charformat.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // <span> was lost, link text having a char format was missing.
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span"_ostr,
                       u"https://libreoffice.org/"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a"_ostr, "href"_ostr, u"https://libreoffice.org/"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testLinkNamedCharFormat)
{
    // Character properties from named character style on hyperlink was lost.
    createDoc(u"link-namedcharformat.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);
    // This failed, there was no span inside the hyperlink.
    assertXPathContent(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span"_ostr,
                       u"http://libreoffice.org"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a"_ostr, "href"_ostr, u"http://libreoffice.org/"_ustr);

    OUString aClass = getXPath(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:span"_ostr, "class"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"#ff0000"_ustr, EPUBExportTest::getCss(aCssDoc, aClass, u"color"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTableWidth)
{
    createDoc(u"table-width.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);

    OUString aClass = getXPath(mpXmlDoc, "//xhtml:table"_ostr, "class"_ostr);
    // This failed, relative total width of table was lost.
    CPPUNIT_ASSERT_EQUAL(u"50%"_ustr, EPUBExportTest::getCss(aCssDoc, aClass, u"width"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTextBox)
{
    createDoc(u"text-box.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);

    // This failed, image with caption was lost.
    assertXPath(mpXmlDoc, "//xhtml:img"_ostr, "class"_ostr, u"frame1"_ustr);
    // Expected spans:
    // 1) break after the image
    // 2) "Illustration "
    // 3) The sequence field, this was missing (was ": foo" instead).
    assertXPathContent(mpXmlDoc, "//xhtml:div/xhtml:p/xhtml:span[3]"_ostr, u"1"_ustr);

    OUString aClass = getXPath(mpXmlDoc, "//xhtml:div/xhtml:p/xhtml:span[3]"_ostr, "class"_ostr);
    // This failed, the 3rd span was not italic.
    CPPUNIT_ASSERT_EQUAL(u"italic"_ustr, EPUBExportTest::getCss(aCssDoc, aClass, u"font-style"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testFontEmbedding)
{
#if !defined(MACOSX)
    createDoc(u"font-embedding.fodt", {});

    // Make sure that the params of defineEmbeddedFont() are all handled.
    // librevenge:name
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);
    // 'SketchFlow Print' or ''SketchFlow Print1'
    CPPUNIT_ASSERT(EPUBExportTest::getCss(aCssDoc, u"font-face"_ustr, u"font-family")
                       .startsWith("'SketchFlow Print"));
    // librevenge:mime-type
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/content.opf"_ustr);
    assertXPath(mpXmlDoc, "/opf:package/opf:manifest/opf:item[@href='fonts/font0001.otf']"_ostr,
                "media-type"_ostr, u"application/vnd.ms-opentype"_ustr);
    // office:binary-data
    CPPUNIT_ASSERT(mxZipFile->hasByName(u"OEBPS/fonts/font0001.otf"_ustr));
    // librevenge:font-style
    CPPUNIT_ASSERT_EQUAL(u"normal"_ustr,
                         EPUBExportTest::getCss(aCssDoc, u"font-face"_ustr, u"font-style"));
    // librevenge:font-weight
    CPPUNIT_ASSERT_EQUAL(u"normal"_ustr,
                         EPUBExportTest::getCss(aCssDoc, u"font-face"_ustr, u"font-weight"));
#endif
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testImageLink)
{
    createDoc(u"image-link.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // This failed, image was missing.
    assertXPath(mpXmlDoc, "//xhtml:p/xhtml:a/xhtml:img"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testFootnote)
{
    createDoc(u"footnote.fodt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // These were missing, footnote was lost.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p/xhtml:sup/xhtml:a"_ostr, "type"_ostr,
                u"noteref"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:aside"_ostr, "type"_ostr, u"footnote"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testPopup)
{
    createDoc(u"popup.odt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // Test image popup anchor.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p[1]/xhtml:a"_ostr, "type"_ostr, u"noteref"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p[1]/xhtml:a/xhtml:img"_ostr, 1);
    // Test image popup content.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:aside[1]"_ostr, "type"_ostr, u"footnote"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:aside[1]/xhtml:img"_ostr, 1);

    // Test text popup anchor.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p[2]/xhtml:span/xhtml:a"_ostr, "type"_ostr,
                u"noteref"_ustr);
    assertXPathContent(mpXmlDoc, "//xhtml:body/xhtml:p[2]/xhtml:span/xhtml:a"_ostr, u"link"_ustr);
    // Test text popup content.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:aside[2]"_ostr, "type"_ostr, u"footnote"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:aside[2]/xhtml:img"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testPopupMedia)
{
    // This is the same as testPopup(), but the links point to images in the
    // default media directory, not in the document directory.
    createDoc(u"popup-media.odt", {});

    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // Test image popup anchor. This failed, number of XPath nodes was 0.
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p[1]/xhtml:a"_ostr, "type"_ostr, u"noteref"_ustr);
    assertXPath(mpXmlDoc, "//xhtml:body/xhtml:p[1]/xhtml:a/xhtml:img"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testPopupAPI)
{
    // Make sure that the popup works with data from a media directory.
    OUString aMediaDir = createFileURL(u"popup");
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "RVNGMediaDir", uno::Any(aMediaDir) } }));
    createDoc(u"popup-api.odt", aFilterData);

    // We have a non-empty anchor image.
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    OUString aAnchor
        = getXPath(mpXmlDoc, "//xhtml:body/xhtml:p[1]/xhtml:a/xhtml:img"_ostr, "src"_ostr);
    CPPUNIT_ASSERT(!aAnchor.isEmpty());
    // We have a non-empty popup image.
    OUString aData = getXPath(mpXmlDoc, "//xhtml:body/xhtml:aside[1]/xhtml:img"_ostr, "src"_ostr);
    CPPUNIT_ASSERT(!aData.isEmpty());
    // The anchor is different from the popup image.
    CPPUNIT_ASSERT(aAnchor != aData);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testPageSize)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "EPUBLayoutMethod",
            uno::Any(static_cast<sal_Int32>(libepubgen::EPUB_LAYOUT_METHOD_FIXED)) } }));
    createDoc(u"hello.fodt", aFilterData);

    // This failed, viewport was empty, so page size was lost.
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    // 21,59cm x 27.94cm (letter).
    assertXPath(mpXmlDoc, "/xhtml:html/xhtml:head/xhtml:meta[@name='viewport']"_ostr,
                "content"_ostr, u"width=816, height=1056"_ustr);

    mpXmlDoc = parseExport(u"OEBPS/images/image0001.svg"_ustr);
    // This was 288mm, logic->logic conversion input was a pixel value.
    assertXPath(mpXmlDoc, "/svg:svg"_ostr, "width"_ostr, u"216mm"_ustr);
    assertXPath(mpXmlDoc, "/svg:svg"_ostr, "height"_ostr, u"279mm"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testSVG)
{
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "EPUBLayoutMethod",
            uno::Any(static_cast<sal_Int32>(libepubgen::EPUB_LAYOUT_METHOD_FIXED)) } }));
    createDoc(u"hello.fodt", aFilterData);

    CPPUNIT_ASSERT(mxZipFile->hasByName(u"OEBPS/images/image0001.svg"_ustr));
    uno::Reference<io::XInputStream> xInputStream(
        mxZipFile->getByName(u"OEBPS/images/image0001.svg"_ustr), uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));

    SvMemoryStream aMemoryStream;
    aMemoryStream.WriteStream(*pStream);
    OString aExpected("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n<svg"_ostr);
    CPPUNIT_ASSERT(aMemoryStream.GetSize() > o3tl::make_unsigned(aExpected.getLength()));

    // This failed, there was a '<!DOCTYPE' line between the xml and the svg
    // one, causing a validation error.
    OString aActual(static_cast<const char*>(aMemoryStream.GetData()), aExpected.getLength());
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    // This failed, we used the xlink attribute namespace, but we did not
    // define its URL.
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/images/image0001.svg"_ustr);
    assertXPathNSDef(mpXmlDoc, "/svg:svg"_ostr, u"xlink", u"http://www.w3.org/1999/xlink");
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTdf115623SingleWritingMode)
{
    // Simple page that has single writing mode should work.
    createDoc(u"tdf115623-single-writing-mode.odt", {});
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    OUString aClass = getXPath(mpXmlDoc, "//xhtml:body"_ostr, "class"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"vertical-rl"_ustr,
                         EPUBExportTest::getCss(aCssDoc, aClass, u"writing-mode"));
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTdf115623SplitByChapter)
{
    createDoc(u"tdf115623-split-by-chapter.odt", {});
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);
    {
        xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
        OUString aClass = getXPath(mpXmlDoc, "//xhtml:body"_ostr, "class"_ostr);
        CPPUNIT_ASSERT_EQUAL(u"vertical-rl"_ustr,
                             EPUBExportTest::getCss(aCssDoc, aClass, u"writing-mode"));
    }
    // Split HTML should keep the same writing-mode.
    {
        xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0002.xhtml"_ustr);
        OUString aClass = getXPath(mpXmlDoc, "//xhtml:body"_ostr, "class"_ostr);
        CPPUNIT_ASSERT_EQUAL(u"vertical-rl"_ustr,
                             EPUBExportTest::getCss(aCssDoc, aClass, u"writing-mode"));
    }
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testTdf115623ManyPageSpans)
{
    createDoc(u"tdf115623-many-pagespans.odt", {});
    std::map<OUString, std::vector<OUString>> aCssDoc
        = parseCss(u"OEBPS/styles/stylesheet.css"_ustr);
    // Two pages should have different writing modes.
    {
        xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
        OUString aClass = getXPath(mpXmlDoc, "//xhtml:body"_ostr, "class"_ostr);
        CPPUNIT_ASSERT_EQUAL(u"vertical-rl"_ustr,
                             EPUBExportTest::getCss(aCssDoc, aClass, u"writing-mode"));
    }
    {
        xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0002.xhtml"_ustr);
        OUString aClass = getXPath(mpXmlDoc, "//xhtml:body"_ostr, "class"_ostr);
        CPPUNIT_ASSERT_EQUAL(u"horizontal-tb"_ustr,
                             EPUBExportTest::getCss(aCssDoc, aClass, u"writing-mode"));
    }
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testSimpleRuby)
{
    createDoc(u"simple-ruby.odt", {});
    xmlDocUniquePtr mpXmlDoc = parseExport(u"OEBPS/sections/section0001.xhtml"_ustr);
    assertXPathContent(mpXmlDoc, "//xhtml:body/xhtml:p/xhtml:ruby/xhtml:span"_ostr,
                       u"base text"_ustr);
    assertXPathContent(mpXmlDoc, "//xhtml:body/xhtml:p/xhtml:ruby/xhtml:rt"_ostr,
                       u"ruby text"_ustr);
}

CPPUNIT_TEST_FIXTURE(EPUBExportTest, testAbi11105)
{
    // This crashed because the paragraph style "P5" which had a master-page-name
    // appeared in a table cell messed up page spans.
    createDoc(u"abi11105.abw", {});
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
