/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers ooox/source/export/ fixes.
class Test : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
private:
    uno::Reference<lang::XComponent> mxComponent;
    utl::TempFile maTempFile;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    utl::TempFile& getTempFile() { return maTempFile; }
    void loadAndSave(const OUString& rURL, const OUString& rFilterName);
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void Test::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w"),
                       BAD_CAST("http://schemas.openxmlformats.org/wordprocessingml/2006/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("v"), BAD_CAST("urn:schemas-microsoft-com:vml"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("mc"),
                       BAD_CAST("http://schemas.openxmlformats.org/markup-compatibility/2006"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("wps"),
        BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingShape"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("wpg"),
        BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingGroup"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("wp"),
        BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("wp14"),
        BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("a"),
                       BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("pic"),
                       BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/picture"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("rels"),
                       BAD_CAST("http://schemas.openxmlformats.org/package/2006/relationships"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w14"),
                       BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordml"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("m"),
                       BAD_CAST("http://schemas.openxmlformats.org/officeDocument/2006/math"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("ContentType"),
                       BAD_CAST("http://schemas.openxmlformats.org/package/2006/content-types"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("lc"),
                       BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/lockedCanvas"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("cp"),
        BAD_CAST("http://schemas.openxmlformats.org/package/2006/metadata/core-properties"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("extended-properties"),
        BAD_CAST("http://schemas.openxmlformats.org/officeDocument/2006/extended-properties"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("custom-properties"),
        BAD_CAST("http://schemas.openxmlformats.org/officeDocument/2006/custom-properties"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("vt"),
        BAD_CAST("http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dcterms"), BAD_CAST("http://purl.org/dc/terms/"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("a14"),
                       BAD_CAST("http://schemas.microsoft.com/office/drawing/2010/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("c"),
                       BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/chart"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("o"),
                       BAD_CAST("urn:schemas-microsoft-com:office:office"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w10"),
                       BAD_CAST("urn:schemas-microsoft-com:office:word"));
}

void Test::loadAndSave(const OUString& rURL, const OUString& rFilterName)
{
    mxComponent = loadFromDesktop(rURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= rFilterName;
    maTempFile.EnableKillingFile();
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mxComponent->dispose();
    mxComponent.clear();
    // too many DOCX validation errors right now
    if (rFilterName != "Office Open XML Text")
    {
        validate(maTempFile.GetFileName(), test::OOXML);
    }
}

constexpr OUStringLiteral DATA_DIRECTORY = u"/oox/qa/unit/data/";

CPPUNIT_TEST_FIXTURE(Test, testDmlGroupshapePolygon)
{
    // Given a document with a group shape, containing a single polygon child shape:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "dml-groupshape-polygon.docx";

    // When saving that to DOCX:
    loadAndSave(aURL, "Office Open XML Text");

    // Then make sure that the group shape, the group shape's child size and the child shape's size
    // match:
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, getTempFile().GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("word/document.xml"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:ext", "cx", "5328360");
    // Without the accompanying fix in place, this test would have failed, the <a:chExt> element was
    // not written.
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:chExt", "cx", "5328360");
    assertXPath(pXmlDoc, "//wps:spPr/a:xfrm/a:ext", "cx", "5328360");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
