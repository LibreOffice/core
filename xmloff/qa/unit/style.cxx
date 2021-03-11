/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <comphelper/propertysequence.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;

constexpr OUStringLiteral DATA_DIRECTORY = u"/xmloff/qa/unit/data/";

/// Covers xmloff/source/style/ fixes.
class XmloffStyleTest : public test::BootstrapFixture,
                        public unotest::MacrosTest,
                        public XmlTestTools
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    void load(std::u16string_view rURL);
};

void XmloffStyleTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void XmloffStyleTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void XmloffStyleTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("office"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:office:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("style"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:style:1.0"));
}

void XmloffStyleTest::load(std::u16string_view rFileName)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + rFileName;
    mxComponent = loadFromDesktop(aURL);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testFillImageBase64)
{
    // Load a flat ODG that has base64-encoded bitmap as a fill style.
    load(u"fill-image-base64.fodg");
    uno::Reference<lang::XMultiServiceFactory> xFactory(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xBitmaps(
        xFactory->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed, as the base64 stream was
    // not considered when parsing the fill-image style.
    CPPUNIT_ASSERT(xBitmaps->hasByName("libreoffice_0"));
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testFontSorting)
{
    // Given an empty document with default fonts (Liberation Sans, Lucida Sans, etc):
    getComponent() = loadFromDesktop("private:factory/swriter");

    // When saving that document to ODT:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::makeAny(OUString("writer8")) },
    });
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);

    // Then make sure <style:font-face> elements are sorted (by style:name="..."):
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, aTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("content.xml"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    xmlXPathObjectPtr pXPath
        = getXPathNode(pXmlDoc, "/office:document-content/office:font-face-decls/style:font-face");
    xmlNodeSetPtr pXmlNodes = pXPath->nodesetval;
    int nNodeCount = xmlXPathNodeSetGetLength(pXmlNodes);
    std::vector<OString> aXMLNames;
    std::set<OString> aSortedNames;
    for (int i = 0; i < nNodeCount; ++i)
    {
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[i];
        xmlChar* pName = xmlGetProp(pXmlNode, BAD_CAST("name"));
        OString aName(reinterpret_cast<char const*>(pName));
        aXMLNames.push_back(aName);
        aSortedNames.insert(aName);
        xmlFree(pName);
    }
    size_t nIndex = 0;
    for (const auto& rName : aSortedNames)
    {
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: Liberation Sans
        // - Actual  : Lucida Sans1
        // i.e. the output was not lexicographically sorted, "u" was before "i".
        CPPUNIT_ASSERT_EQUAL(rName, aXMLNames[nIndex]);
        ++nIndex;
    }
    xmlXPathFreeObject(pXPath);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
