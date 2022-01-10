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

#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>

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
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("p"),
                       BAD_CAST("http://schemas.openxmlformats.org/presentationml/2006/main"));
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
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "word/document.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:ext", "cx", "5328360");
    // Without the accompanying fix in place, this test would have failed, the <a:chExt> element was
    // not written.
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:chExt", "cx", "5328360");
    assertXPath(pXmlDoc, "//wps:spPr/a:xfrm/a:ext", "cx", "5328360");
}

CPPUNIT_TEST_FIXTURE(Test, testCameraRevolutionGrabBag)
{
    // Given a PPTX file that contains camera revolution (rotation around z axis) applied shapes
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "camera-rotation-revolution-nonwps.pptx";

    // When saving that document:
    loadAndSave(aURL, "Impress Office Open XML");

    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Then make sure the revolution is exported without a problem:
    // First shape textbox:
    assertXPath(pXmlDoc, "//p:sp[1]/p:spPr/a:scene3d/a:camera/a:rot", "rev", "5400000");

    // Second shape rectangle:
    assertXPath(pXmlDoc, "//p:sp[2]/p:spPr/a:scene3d/a:camera/a:rot", "rev", "18300000");

    // Make sure Shape3DProperties don't leak under txBody
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // - In <>, XPath '//p:sp[1]/p:txBody/a:bodyPr/a:scene3d/a:camera/a:rot' number of nodes is incorrect
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:bodyPr/a:scene3d/a:camera/a:rot", 0);
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:bodyPr/a:scene3d/a:camera/a:rot", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146690_endParagraphRunPropertiesNewLinesTextSize)
{
    // Given PPTX file contains a shape with a textbody populated with new lines
    // and the text size isn't the default size.
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "endParaRPr-newline-textsize.pptx";

    // When saving that document:
    loadAndSave(aURL, "Impress Office Open XML");

    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Then make sure the endParaRPr has the correct values exported for 'sz'
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 500
    // - Actual  : 1800
    // i.e. the endParaRPr 'size' wasn't exported correctly
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p[1]/a:endParaRPr", "sz", "500");
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p[2]/a:endParaRPr", "sz", "500");
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p[3]/a:endParaRPr", "sz", "500");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
