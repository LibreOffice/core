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
    XmlTestTools::registerOOXMLNamespaces(pXmlXpathCtx);
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

CPPUNIT_TEST_FIXTURE(Test, testPolylineConnectorPosition)
{
    // Given a document with a group shape and therein a polyline and a connector.
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf141786_PolylineConnectorInGroup.odt";
    // When saving that to DOCX:
    loadAndSave(aURL, "Office Open XML Text");

    // Then make sure polyline and connector have the correct position.
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "word/document.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // For child elements of groups in Writer the position has to be adapted to be relative
    // to group instead of being relative to anchor. That was missing for polyline and
    // connector.
    // Polyline: Without fix it would have failed with expected: 0, actual: 1800360
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[1]/wps:spPr/a:xfrm/a:off", "x", "0");
    // ... failed with expected: 509400, actual: 1229400
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[1]/wps:spPr/a:xfrm/a:off", "y", "509400");

    // Connector: Without fix it would have failed with expected: 763200, actual: 2563560
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[3]/wps:spPr/a:xfrm/a:off", "x", "763200");
    // ... failed with expected: 0, actual: 720000
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[3]/wps:spPr/a:xfrm/a:off", "y", "0");
    // Polyline and connector were shifted 1800360EMU right, 720000EMU down.
}

CPPUNIT_TEST_FIXTURE(Test, testRotatedShapePosition)
{
    // Given a document with a group shape and therein a rotated custom shape.
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf141786_RotatedShapeInGroup.odt";
    // When saving that to DOCX:
    loadAndSave(aURL, "Office Open XML Text");

    // Then make sure the rotated child shape has the correct position.
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "word/document.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // For a group itself and for shapes outside of groups, the position calculation is done in
    // DocxSdrExport. For child elements of groups it has to be done in
    // DrawingML::WriteShapeTransformation(), but was missing.
    // Without fix it would have failed with expected: 469440, actual: 92160
    // The shape was about 1cm shifted up and partly outside its group.
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[1]/wps:spPr/a:xfrm/a:off", "y", "469440");
}

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

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeArrowExport)
{
    // Given a document with a few different kinds of arrow shapes in it:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf142602_CustomShapeArrows.odt";
    // When saving that to DOCX:
    loadAndSave(aURL, "Office Open XML Text");

    // Then the shapes should retain their correct control values.
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "word/document.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // Without the fix the output OOXML would have no <a:prstGeom> tags in it.

    // Right arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "rightArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", "val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", "val 46321");

    // Left arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "leftArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", "val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", "val 52939");

    // Down arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "downArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", "val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", "val 59399");

    // Up arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "upArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", "val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", "val 63885");

    // Left-right arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[5]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "leftRightArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[5]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", "val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[5]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", "val 53522");

    // Up-down arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[6]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "upDownArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[6]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", "val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[6]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", "val 62743");

    // Right arrow callout
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "rightArrowCallout");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", "val 25002");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", "val 25000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[3]",
                "fmla", "val 25052");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[4]",
                "fmla", "val 66667");

    // Left arrow callout
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "leftArrowCallout");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", "val 25002");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", "val 25000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[3]",
                "fmla", "val 25057");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[4]",
                "fmla", "val 66673");

    // Down arrow callout
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[9]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "downArrowCallout");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[9]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", "val 29415");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[9]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", "val 29413");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[9]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[3]",
                "fmla", "val 16667");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[9]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[4]",
                "fmla", "val 66667");

    // Up arrow callout
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[10]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", "upArrowCallout");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[10]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", "val 31033");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[10]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", "val 31030");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[10]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[3]",
                "fmla", "val 16667");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[10]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[4]",
                "fmla", "val 66660");
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

CPPUNIT_TEST_FIXTURE(Test, testReferToTheme)
{
    // Given a PPTX file that contains references to a theme:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "refer-to-theme.pptx";

    // When saving that document:
    loadAndSave(aURL, "Impress Office Open XML");

    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Then make sure the shape text color is a scheme color:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr' number of nodes is incorrect
    // i.e. the <a:schemeClr> element was not written.
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr", "val",
                "accent1");
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumMod", 0);
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumOff", 0);

    // Second shape: lighter color:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//p:sp[2]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr' number of nodes is incorrect
    // i.e. the effects case did not write scheme colors.
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr", "val",
                "accent1");
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumMod", "val",
                "40000");
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumOff", "val",
                "60000");

    // Third shape, darker color:
    assertXPath(pXmlDoc, "//p:sp[3]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr", "val",
                "accent1");
    assertXPath(pXmlDoc, "//p:sp[3]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumMod", "val",
                "75000");
    assertXPath(pXmlDoc, "//p:sp[3]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumOff", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testReferToThemeShapeFill)
{
    // Given an ODP file that contains references to a theme for shape fill:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "refer-to-theme-shape-fill.odp";

    // When saving that document:
    loadAndSave(aURL, "Impress Office Open XML");

    // Then make sure the shape fill color is a scheme color:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the <a:schemeClr> element was not written. Note that this was already working from PPTX
    // files via grab-bags, so this test intentionally uses an ODP file as input.
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    assertXPath(pXmlDoc, "//p:sp[1]/p:spPr/a:solidFill/a:schemeClr", "val", "accent1");
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//p:sp[1]/p:spPr/a:solidFill/a:schemeClr/a:lumMod' number of nodes is incorrect
    // i.e. the effects of the themed color were lost.
    assertXPath(pXmlDoc, "//p:sp[1]/p:spPr/a:solidFill/a:schemeClr/a:lumMod", "val", "40000");
    assertXPath(pXmlDoc, "//p:sp[1]/p:spPr/a:solidFill/a:schemeClr/a:lumOff", "val", "60000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146690_endParagraphRunPropertiesNewLinesTextSize)
{
    // Given a PPTX file that contains references to a theme:
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "endParaRPr-newline-textsize.pptx";

    // When saving that document:
    loadAndSave(aURL, "Impress Office Open XML");

    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Make sure the text size is exported correctly:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 500
    // - Actual  : 1800
    // i.e. the endParaRPr 'size' wasn't exported correctly
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p[1]/a:endParaRPr", "sz", "500");
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p[2]/a:endParaRPr", "sz", "500");
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p[3]/a:endParaRPr", "sz", "500");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_endsubpath)
{
    // Given an odp file that contains a non-primitive custom shape with command N
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf147978_endsubpath.odp";

    // When saving that document:
    loadAndSave(aURL, "Impress Office Open XML");

    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Then make sure the pathLst has two child elements,
    // Without the accompanying fix in place, only one element a:path was exported.
    assertXPathChildren(pXmlDoc, "//a:pathLst", 2);
    // and make sure first path with no stroke, second with no fill
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "stroke", "0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]", "fill", "none");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_commandA)
{
    // Given an odp file that contains a non-primitive custom shape with command N
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf147978_enhancedPath_commandA.odp";

    // When saving that document:
    loadAndSave(aURL, "Impress Office Open XML");

    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Then make sure the path has a child element arcTo. Prior to the fix that part of the curve was
    // not exported at all. In odp it is a command A. Such does not exist in OOXML and is therefore
    // exported as a:lnTo followed by a:arcTo
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo", 2);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", 1);
    // And assert its attribute values
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "wR", "7200");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "hR", "5400");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "stAng", "7719588");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "swAng", "-5799266");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_commandT)
{
    // The odp file contains a non-primitive custom shape with commands MTZ
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf147978_enhancedPath_commandT.odp";

    // Export to pptx had only exported the command M and has used a wrong path size
    loadAndSave(aURL, "Impress Office Open XML");

    // Verify the markup:
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // File has draw:viewBox="0 0 216 216"
    assertXPath(pXmlDoc, "//a:pathLst/a:path", "w", "216");
    assertXPath(pXmlDoc, "//a:pathLst/a:path", "h", "216");
    // Command T is exported as lnTo followed by arcTo.
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo", 1);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo", 1);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", 1);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:close", 1);
    // And assert its values
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo/a:pt", "x", "108");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo/a:pt", "y", "162");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo/a:pt", "x", "138");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo/a:pt", "y", "110");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "wR", "108");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "hR", "54");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "stAng", "18000000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "swAng", "18000000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_commandXY)
{
    // The odp file contains a non-primitive custom shapes with commands XY
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf147978_enhancedPath_commandXY.odp";

    // Export to pptx had dropped commands X and Y.
    loadAndSave(aURL, "Impress Office Open XML");

    // Verify the markup:
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // File has draw:viewBox="0 0 10 10"
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "w", "10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "h", "10");
    // Shape has M 0 5 Y 5 0 10 5 5 10 F Y 0 5 N M 10 10 X 0 0
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:moveTo/a:pt", "x", "0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:moveTo/a:pt", "y", "5");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[1]", "wR", "5");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[1]", "hR", "5");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[1]", "stAng", "10800000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[1]", "swAng", "5400000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[2]", "stAng", "16200000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[2]", "swAng", "5400000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[3]", "stAng", "0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[3]", "swAng", "5400000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[4]", "stAng", "0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[4]", "swAng", "-5400000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:moveTo/a:pt", "x", "10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:moveTo/a:pt", "y", "10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "wR", "10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "hR", "10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "stAng", "5400000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "swAng", "5400000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_commandHIJK)
{
    // The odp file contains a non-primitive custom shapes with commands H,I,J,K
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf147978_enhancedPath_commandHIJK.odp";

    // Export to pptx had dropped commands X and Y.
    loadAndSave(aURL, "Impress Office Open XML");

    // Verify the markup:
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // File has draw:viewBox="0 0 80 80"
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "w", "80");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "h", "80");
    // File uses from back to front J (lighten), I (lightenLess), normal fill, K (darkenLess),
    // H (darken). New feature, old versions did not export these at all.
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "fill", "lighten");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]", "fill", "lightenLess");
    assertXPathNoAttribute(pXmlDoc, "//a:pathLst/a:path[3]", "fill");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[4]", "fill", "darkenLess");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[5]", "fill", "darken");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_subpath)
{
    // The odp file contains a non-primitive custom shapes with commands H,I,J,K
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf147978_enhancedPath_subpath.pptx";

    // Export to pptx had dropped the subpaths.
    loadAndSave(aURL, "Impress Office Open XML");

    // Verify the markup:
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // File should have four subpaths with increasing path size
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "w", "10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "h", "10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]", "w", "20");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]", "h", "20");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[3]", "w", "40");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[3]", "h", "40");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[4]", "w", "80");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[4]", "h", "80");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf100391TextAreaRect)
{
    // The document has a custom shape of type "non-primitive" to trigger the custGeom export
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf100391_TextAreaRect.odp";
    // When saving to PPTX the textarea rect was set to default instead of using the actual area
    loadAndSave(aURL, "Impress Office Open XML");

    // Verify the markup. Without fix the values were l="l", t="t", r="r", b="b"
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    assertXPath(pXmlDoc, "//a:custGeom/a:rect", "l", "textAreaLeft");
    assertXPath(pXmlDoc, "//a:custGeom/a:rect", "t", "textAreaTop");
    assertXPath(pXmlDoc, "//a:custGeom/a:rect", "r", "textAreaRight");
    assertXPath(pXmlDoc, "//a:custGeom/a:rect", "b", "textAreaBottom");
    // The values are calculated in guides, for example
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[1]", "name", "textAreaLeft");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[1]", "fmla", "*/ 1440000 w 2880000");
    // The test reflects the state of Apr 2022. It needs to be adapted when export of handles and
    // guides is implemented.
}

CPPUNIT_TEST_FIXTURE(Test, testTdf109169_OctagonBevel)
{
    // The odp file contains an "Octagon Bevel" shape. Such has shading not in commands H,I,J,K
    // but shading is generated in ctor of EnhancedCustomShape2d from the Type value.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf109169_OctagonBevel.odt";

    // Export to docx had not written a:fill or a:stroke attributes at all.
    loadAndSave(aURL, "Office Open XML Text");

    // Verify the markup:
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "word/document.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // File should have six subpaths, one with stroke and five with fill
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "stroke", "0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]", "fill", "darkenLess");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[3]", "fill", "darken");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[4]", "fill", "darken");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[5]", "fill", "lightenLess");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[6]", "fill", "lighten");
}

CPPUNIT_TEST_FIXTURE(Test, testFaultyPathCommandsAWT)
{
    // The odp file contains shapes whose path starts with command A, W, T or L. That is a faulty
    // path. LO is tolerant and renders it so that is makes a moveTo to the start point of the arc or
    // the end of the line respectively. Export to OOXML does the same now and writes a moveTo
    // instead of the normally used lnTo. If a lnTo is written, MS Office shows nothing of the shape.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "FaultyPathStart.odp";

    loadAndSave(aURL, "Impress Office Open XML");

    // Verify the markup:
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // First child of a:path should be a moveTo in all four shapes.
    assertXPath(pXmlDoc, "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:moveTo");
    assertXPath(pXmlDoc, "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/a:moveTo");
    assertXPath(pXmlDoc, "//p:spTree/p:sp[3]/p:spPr/a:custGeom/a:pathLst/a:path/a:moveTo");
    assertXPath(pXmlDoc, "//p:spTree/p:sp[4]/p:spPr/a:custGeom/a:pathLst/a:path/a:moveTo");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148784StretchXY)
{
    // The document has a custom shapes of type "non-primitive" to trigger the custGeom export.
    // They use formulas with 'right' and 'bottom'.
    // When saving to PPTX the attributes stretchpoint-x and stretchpoint-y were not considered. The
    // line at right and bottom edge were positioned inside as if the shape had a square size.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf148784_StretchXY.odp";
    loadAndSave(aURL, "Impress Office Open XML");

    // Verify the markup.
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // x-position of last segment should be same as path width. It was 21600 without fix.
    sal_Int32 nWidth
        = getXPathContent(pXmlDoc, "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/@w")
              .toInt32();
    sal_Int32 nPosX
        = getXPathContent(
              pXmlDoc, "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:moveTo[4]/a:pt/@x")
              .toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("StretchX", nWidth, nPosX);

    // y-position of last segment should be same as path height. It was 21600 without fix.
    sal_Int32 nHeight
        = getXPathContent(pXmlDoc, "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/@h")
              .toInt32();
    sal_Int32 nPosY
        = getXPathContent(
              pXmlDoc, "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/a:moveTo[4]/a:pt/@y")
              .toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("StretchY", nHeight, nPosY);

    // The test reflects the state of Apr 2022. It needs to be adapted when export of handles and
    // guides is implemented.
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148784StretchCommandQ)
{
    // The document has a custom shapes of type "non-primitive" to trigger the custGeom export.
    // They use formulas with 'right' and 'bottom'.
    // When saving to PPTX the attributes stretchpoint-x and stretchpoint-y were not considered.
    // That results in wrong arcs on the right or bottom side of the shape.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf148784_StretchCommandQ.odp";
    loadAndSave(aURL, "Impress Office Open XML");

    // Verify the markup.
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // x-position of second quadBezTo control should be same as path width. It was 21600 without fix.
    sal_Int32 nWidth
        = getXPathContent(pXmlDoc, "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/@w")
              .toInt32();
    sal_Int32 nPosX
        = getXPathContent(
              pXmlDoc,
              "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:quadBezTo[2]/a:pt/@x")
              .toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("StretchX", nWidth, nPosX);

    // y-position of third quadBezTo control should be same as path height. It was 21600 without fix.
    sal_Int32 nHeight
        = getXPathContent(pXmlDoc, "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/@h")
              .toInt32();
    sal_Int32 nPosY
        = getXPathContent(
              pXmlDoc,
              "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/a:quadBezTo[3]/a:pt/@y")
              .toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("StretchY", nHeight, nPosY);

    // The test reflects the state of Apr 2022. It needs to be adapted when export of handles and
    // guides is implemented.
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148784StretchCommandVW)
{
    // The document has a custom shapes of type "non-primitive" to trigger the custGeom export.
    // It should not need adaption when export of handles and guides is implemented because it
    // has only fixed values in the path.
    // When saving to PPTX the attributes stretchpoint-x and stretchpoint-y were not considered.
    // That results in circles instead of ellipses.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf148784_StretchCommandVW.odp";
    loadAndSave(aURL, "Impress Office Open XML");

    // Verify the markup.
    std::unique_ptr<SvStream> pStream = parseExportStream(getTempFile(), "ppt/slides/slide1.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());

    // wR of first ArcTo in first shape should be same as path width/2. It was 10800 without fix.
    sal_Int32 nHalfWidth
        = getXPathContent(pXmlDoc, "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/@w")
              .toInt32()
          / 2;
    sal_Int32 nWR
        = getXPathContent(pXmlDoc,
                          "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:arcTo[1]/@wR")
              .toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("StretchX", nHalfWidth, nWR);

    // hR of first ArcTo in second shape should be same as path height /2. It was 10800 without fix.
    sal_Int32 nHalfHeight
        = getXPathContent(pXmlDoc, "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/@h")
              .toInt32()
          / 2;
    sal_Int32 nHR
        = getXPathContent(pXmlDoc,
                          "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/a:arcTo[1]/@hR")
              .toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("StretchY", nHalfHeight, nHR);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
