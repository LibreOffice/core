/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextField.hpp>

using namespace ::com::sun::star;
using namespace ::cpo;

namespace
{
/// Covers ooox/source/export/ fixes.
class Test : public UnoApiXmlTest
{
public:
    Test()
        : UnoApiXmlTest(u"/oox/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testPolylineConnectorPosition)
{
    // Given a document with a group shape and therein a polyline and a connector.
    loadFromFile(u"tdf141786_PolylineConnectorInGroup.odt");
    // When saving that to DOCX:
    save(TestFilter::DOCX);

    // Then make sure polyline and connector have the correct position.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // For child elements of groups in Writer the position has to be adapted to be relative
    // to group instead of being relative to anchor. That was missing for polyline and
    // connector.
    // Polyline: Without fix it would have failed with expected: 0, actual: 1800360
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[1]/wps:spPr/a:xfrm/a:off", "x", u"0");
    // ... failed with expected: 509400, actual: 1229400
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[1]/wps:spPr/a:xfrm/a:off", "y", u"509400");

    // Connector: Without fix it would have failed with expected: 763200, actual: 2563560
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[3]/wps:spPr/a:xfrm/a:off", "x", u"763200");
    // ... failed with expected: 0, actual: 720000
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[3]/wps:spPr/a:xfrm/a:off", "y", u"0");
    // Polyline and connector were shifted 1800360EMU right, 720000EMU down.
}

CPPUNIT_TEST_FIXTURE(Test, testRotatedShapePosition)
{
    // Given a document with a group shape and therein a rotated custom shape.
    loadFromFile(u"tdf141786_RotatedShapeInGroup.odt");

    // When saving that to DOCX:
    save(TestFilter::DOCX);

    // Then make sure the rotated child shape has the correct position.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // For a group itself and for shapes outside of groups, the position calculation is done in
    // DocxSdrExport. For child elements of groups it has to be done in
    // DrawingML::WriteShapeTransformation(), but was missing.
    // Without fix it would have failed with expected: 469440, actual: 92160
    // The shape was about 1cm shifted up and partly outside its group.
    assertXPath(pXmlDoc, "//wpg:wgp/wps:wsp[1]/wps:spPr/a:xfrm/a:off", "y", u"469440");
}

CPPUNIT_TEST_FIXTURE(Test, testInsertCheckboxContentControlOdt)
{
    loadFromFile(u"tdf141786_RotatedShapeInGroup.odt");

    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertCheckboxContentControl"_ustr, {});

    save(TestFilter::DOCX);
}

CPPUNIT_TEST_FIXTURE(Test, testInsertCheckboxContentControlDocx)
{
    {
        loadFromFile(u"dml-groupshape-polygon.docx");

        // Without TrackChanges, inserting the Checkbox works just fine
        // when exporting to docx.
        dispatchCommand(mxComponent, u".uno:InsertCheckboxContentControl"_ustr, {});

        save(TestFilter::DOCX);
    }

    {
        loadFromFile(u"dml-groupshape-polygon.docx");

        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XText> xText = xTextDocument->getText();
        uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();

        // With TrackChanges, the Checkbox causes an assertion in the sax serializer,
        // in void sax_fastparser::FastSaxSerializer::endFastElement(sal_Int32).
        // Element == maMarkStack.top()->m_DebugStartedElements.back()
        // sax/source/tools/fastserializer.cxx#402
        dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});

        xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);

        dispatchCommand(mxComponent, u".uno:InsertCheckboxContentControl"_ustr, {});

        // Loading should not show the "corrupted" dialog, which would assert.
        saveAndReload(TestFilter::DOCX);

        // Now that we loaded it successfully, delete the controls,
        // still with change-tracking enabled, and save.
        dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
        dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

        // Loading should not show the "corrupted" dialog, which would assert.
        saveAndReload(TestFilter::DOCX);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDmlGroupshapePolygon)
{
    // Given a document with a group shape, containing a single polygon child shape:
    loadFromFile(u"dml-groupshape-polygon.docx");

    // When saving that to DOCX:
    save(TestFilter::DOCX);

    // Then make sure that the group shape, the group shape's child size and the child shape's size
    // match:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:ext", "cx", u"5328360");
    // Without the accompanying fix in place, this test would have failed, the <a:chExt> element was
    // not written.
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:chExt", "cx", u"5328360");
    assertXPath(pXmlDoc, "//wps:spPr/a:xfrm/a:ext", "cx", u"5328360");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeArrowExport)
{
    // Given a document with a few different kinds of arrow shapes in it:
    loadFromFile(u"tdf142602_CustomShapeArrows.odt");

    // When saving that to DOCX:
    save(TestFilter::DOCX);

    // Then the shapes should retain their correct control values.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Without the fix the output OOXML would have no <a:prstGeom> tags in it.

    // Right arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", u"rightArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", u"val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", u"val 46321");

    // Left arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", u"leftArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", u"val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", u"val 52939");

    // Down arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", u"downArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", u"val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", u"val 59399");

    // Up arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", u"upArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", u"val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[4]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", u"val 63885");

    // Left-right arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[5]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", u"leftRightArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[5]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", u"val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[5]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", u"val 53522");

    // Up-down arrow
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[6]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", u"upDownArrow");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[6]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", u"val 50000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[6]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", u"val 62743");

    // Right arrow callout
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", u"rightArrowCallout");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", u"val 25002");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", u"val 25000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[3]",
                "fmla", u"val 25052");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[7]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[4]",
                "fmla", u"val 66667");

    // Left arrow callout
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", u"leftArrowCallout");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", u"val 25002");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", u"val 25000");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[3]",
                "fmla", u"val 25057");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[8]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[4]",
                "fmla", u"val 66673");

    // Down arrow callout
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[9]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", u"downArrowCallout");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[9]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", u"val 29415");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[9]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", u"val 29413");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[9]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[3]",
                "fmla", u"val 16667");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[9]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[4]",
                "fmla", u"val 66667");

    // Up arrow callout
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[10]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom",
                "prst", u"upArrowCallout");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[10]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[1]",
                "fmla", u"val 31033");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[10]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[2]",
                "fmla", u"val 31030");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[10]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[3]",
                "fmla", u"val 16667");
    assertXPath(pXmlDoc,
                "//w:r/mc:AlternateContent[10]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
                "a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd[4]",
                "fmla", u"val 66660");
}

CPPUNIT_TEST_FIXTURE(Test, testHyperlinkTooltip)
{
    // Given a presentation whose hyperlink carries an accessible name:
    loadFromFile(u"hyperlink-name.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the name goes out as the tooltip
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:hlinkClick", "tooltip", u"Collabora Office home page");

    // and it survives reading that file back as the title; the tooltip used to land in
    // Representation, which TextRun overwrites with the link's visible text
    loadFromURL(maTempFile.GetURL());
    auto xDoc = mxComponent.queryThrow<drawing::XDrawPagesSupplier>();
    auto xPage = xDoc->getDrawPages()->getByIndex(0).queryThrow<drawing::XDrawPage>();
    auto xShape = xPage->getByIndex(0).queryThrow<container::XEnumerationAccess>();
    auto xParagraphs = xShape->createEnumeration();
    auto xParagraph = xParagraphs->nextElement().queryThrow<container::XEnumerationAccess>();
    auto xRuns = xParagraph->createEnumeration();
    auto xRun = xRuns->nextElement().queryThrow<beans::XPropertySet>();
    auto xField = xRun->getPropertyValue(u"TextField"_ustr).queryThrow<text::XTextField>();
    OUString aTitle;
    xField.queryThrow<beans::XPropertySet>()->getPropertyValue(u"Title"_ustr) >>= aTitle;
    CPPUNIT_ASSERT_EQUAL(u"Collabora Office home page"_ustr, aTitle);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf173709HyperlinkTitleTooltip)
{
    // Given a presentation whose hyperlink has both a name and a title:
    loadFromFile(u"tdf173709-hyperlink-title.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the title goes out as the tooltip:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Title tip
    // - Actual  : Name tip
    assertXPath(pXmlDoc, "//a:hlinkClick", "tooltip", u"Title tip");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf173709HyperlinkTitleTooltipXLSX)
{
    // Given a spreadsheet with a text box whose hyperlink has a title:
    loadFromFile(u"tdf173709-hyperlink-title.fods");

    // When saving it to XLSX:
    save(TestFilter::XLSX);

    // Then the title goes out as the tooltip:
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:hlinkClick", "tooltip", u"Title tip");

    // And it is read back as the title:
    loadFromURL(maTempFile.GetURL());
    auto xDoc = mxComponent.queryThrow<drawing::XDrawPagesSupplier>();
    auto xPage = xDoc->getDrawPages()->getByIndex(0).queryThrow<drawing::XDrawPage>();
    auto xShape = xPage->getByIndex(0).queryThrow<container::XEnumerationAccess>();
    auto xParagraphs = xShape->createEnumeration();
    auto xParagraph = xParagraphs->nextElement().queryThrow<container::XEnumerationAccess>();
    auto xRuns = xParagraph->createEnumeration();
    auto xRun = xRuns->nextElement().queryThrow<beans::XPropertySet>();
    auto xField = xRun->getPropertyValue(u"TextField"_ustr).queryThrow<text::XTextField>();
    OUString aTitle;
    xField.queryThrow<beans::XPropertySet>()->getPropertyValue(u"Title"_ustr) >>= aTitle;
    // Without the accompanying fix in place, this test would have failed, the tooltip was read
    // back as the name.
    CPPUNIT_ASSERT_EQUAL(u"Title tip"_ustr, aTitle);
}

CPPUNIT_TEST_FIXTURE(Test, testHyperlinkNoTooltip)
{
    // Given a presentation whose hyperlink has no name:
    loadFromFile(u"hyperlink-no-name.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then no attribute is written, rather than an empty one
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPathNoAttribute(pXmlDoc, "//a:hlinkClick", "tooltip");
}

CPPUNIT_TEST_FIXTURE(Test, testCameraRevolutionGrabBag)
{
    // Given a PPTX file that contains camera revolution (rotation around z axis) applied shapes
    loadFromFile(u"camera-rotation-revolution-nonwps.pptx");

    // When saving that document:
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Then make sure the revolution is exported without a problem:
    // First shape textbox:
    assertXPath(pXmlDoc, "//p:sp[1]/p:spPr/a:scene3d/a:camera/a:rot", "rev", u"5400000");

    // Second shape rectangle:
    assertXPath(pXmlDoc, "//p:sp[2]/p:spPr/a:scene3d/a:camera/a:rot", "rev", u"18300000");

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
    loadFromFile(u"refer-to-theme.pptx");

    // When saving that document:
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Then make sure the shape text color is a scheme color:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr' number of nodes is incorrect
    // i.e. the <a:schemeClr> element was not written.
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr", "val",
                u"accent1");
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumMod", 0);
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumOff", 0);

    // Second shape: lighter color:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//p:sp[2]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr' number of nodes is incorrect
    // i.e. the effects case did not write scheme colors.
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr", "val",
                u"accent1");
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumMod", "val",
                u"40000");
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumOff", "val",
                u"60000");

    // Third shape, darker color:
    assertXPath(pXmlDoc, "//p:sp[3]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr", "val",
                u"accent1");
    assertXPath(pXmlDoc, "//p:sp[3]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumMod", "val",
                u"75000");
    assertXPath(pXmlDoc, "//p:sp[3]/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:schemeClr/a:lumOff", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testThemeColor_ShapeFill)
{
    // Given an ODP file that contains references to a theme for shape fill:
    loadFromFile(u"ReferenceShapeFill.fodp");

    // When saving that document:
    save(TestFilter::PPTX);

    // Then make sure the shape fill color is a scheme color.
    // Note that this was already working from PPTX files via grab-bags,
    //so this test intentionally uses an ODP file as input.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//p:sp[1]/p:spPr/a:solidFill/a:schemeClr", "val", u"accent6");
    assertXPath(pXmlDoc, "//p:sp[1]/p:spPr/a:solidFill/a:schemeClr/a:lumMod", "val", u"75000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146690_endParagraphRunPropertiesNewLinesTextSize)
{
    // Given a PPTX file that contains references to a theme:
    loadFromFile(u"endParaRPr-newline-textsize.pptx");

    // When saving that document:
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Make sure the text size is exported correctly:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 500
    // - Actual  : 1800
    // i.e. the endParaRPr 'size' wasn't exported correctly
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p[1]/a:endParaRPr", "sz", u"500");
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p[2]/a:endParaRPr", "sz", u"500");
    assertXPath(pXmlDoc, "//p:sp[1]/p:txBody/a:p[3]/a:endParaRPr", "sz", u"500");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_endsubpath)
{
    // Given an odp file that contains a non-primitive custom shape with command N
    loadFromFile(u"tdf147978_endsubpath.odp");

    // When saving that document:
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Then make sure the pathLst has two child elements,
    // Without the accompanying fix in place, only one element a:path was exported.
    assertXPathChildren(pXmlDoc, "//a:pathLst", 2);
    // and make sure first path with no stroke, second with no fill
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "stroke", u"0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]", "fill", u"none");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_commandA)
{
    // Given an odp file that contains a non-primitive custom shape with command N
    loadFromFile(u"tdf147978_enhancedPath_commandA.odp");

    // When saving that document:
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Then make sure the path has a child element arcTo. Prior to the fix that part of the curve was
    // not exported at all. In odp it is a command A. Such does not exist in OOXML and is therefore
    // exported as a:lnTo followed by a:arcTo
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo", 2);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", 1);
    // And assert its attribute values
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "wR", u"7200");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "hR", u"5400");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "stAng", u"7719588");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "swAng", u"-5799266");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_commandT)
{
    // The odp file contains a non-primitive custom shape with commands MTZ
    loadFromFile(u"tdf147978_enhancedPath_commandT.odp");

    // Export to pptx had only exported the command M and has used a wrong path size
    save(TestFilter::PPTX);

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // File has draw:viewBox="0 0 216 216"
    assertXPath(pXmlDoc, "//a:pathLst/a:path", "w", u"216");
    assertXPath(pXmlDoc, "//a:pathLst/a:path", "h", u"216");
    // Command T is exported as lnTo followed by arcTo.
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo", 1);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo", 1);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", 1);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:close", 1);
    // And assert its values
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo/a:pt", "x", u"108");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo/a:pt", "y", u"162");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo/a:pt", "x", u"138");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo/a:pt", "y", u"110");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "wR", u"108");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "hR", u"54");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "stAng", u"18000000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "swAng", u"18000000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_commandXY)
{
    // The odp file contains a non-primitive custom shapes with commands XY
    loadFromFile(u"tdf147978_enhancedPath_commandXY.odp");

    // Export to pptx had dropped commands X and Y.
    save(TestFilter::PPTX);

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // File has draw:viewBox="0 0 10 10"
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "w", u"10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "h", u"10");
    // Shape has M 0 5 Y 5 0 10 5 5 10 F Y 0 5 N M 10 10 X 0 0
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:moveTo/a:pt", "x", u"0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:moveTo/a:pt", "y", u"5");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[1]", "wR", u"5");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[1]", "hR", u"5");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[1]", "stAng", u"10800000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[1]", "swAng", u"5400000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[2]", "stAng", u"16200000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[2]", "swAng", u"5400000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[3]", "stAng", u"0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[3]", "swAng", u"5400000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[4]", "stAng", u"0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]/a:arcTo[4]", "swAng", u"-5400000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:moveTo/a:pt", "x", u"10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:moveTo/a:pt", "y", u"10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "wR", u"10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "hR", u"10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "stAng", u"5400000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "swAng", u"5400000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_commandHIJK)
{
    // The odp file contains a non-primitive custom shapes with commands H,I,J,K
    loadFromFile(u"tdf147978_enhancedPath_commandHIJK.odp");

    // Export to pptx had dropped commands X and Y.
    save(TestFilter::PPTX);

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // File has draw:viewBox="0 0 80 80"
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "w", u"80");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "h", u"80");
    // File uses from back to front J (lighten), I (lightenLess), normal fill, K (darkenLess),
    // H (darken). New feature, old versions did not export these at all.
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "fill", u"lighten");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]", "fill", u"lightenLess");
    assertXPathNoAttribute(pXmlDoc, "//a:pathLst/a:path[3]", "fill");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[4]", "fill", u"darkenLess");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[5]", "fill", u"darken");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978_subpath)
{
    // The odp file contains a non-primitive custom shapes with commands H,I,J,K
    loadFromFile(u"tdf147978_enhancedPath_subpath.pptx");

    // Export to pptx had dropped the subpaths.
    save(TestFilter::PPTX);

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // File should have four subpaths with increasing path size
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "w", u"10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "h", u"10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]", "w", u"20");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]", "h", u"20");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[3]", "w", u"40");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[3]", "h", u"40");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[4]", "w", u"80");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[4]", "h", u"80");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf100391TextAreaRect)
{
    // The document has a custom shape of type "non-primitive" to trigger the custGeom export
    loadFromFile(u"tdf100391_TextAreaRect.odp");
    // When saving to PPTX the textarea rect was set to default instead of using the actual area
    save(TestFilter::PPTX);

    // Verify the markup. Without fix the values were l="l", t="t", r="r", b="b"
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:rect", "l", u"textAreaLeft");
    assertXPath(pXmlDoc, "//a:custGeom/a:rect", "t", u"textAreaTop");
    assertXPath(pXmlDoc, "//a:custGeom/a:rect", "r", u"textAreaRight");
    assertXPath(pXmlDoc, "//a:custGeom/a:rect", "b", u"textAreaBottom");
    // The values are calculated in guides, for example
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[1]", "name", u"textAreaLeft");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[1]", "fmla", u"*/ 1440000 w 2880000");
    // The test reflects the state of Apr 2022. It needs to be adapted when export of handles and
    // guides is implemented.
}

CPPUNIT_TEST_FIXTURE(Test, testTdf109169_OctagonBevel)
{
    // The odp file contains an "Octagon Bevel" shape. Such has shading not in commands H,I,J,K
    // but shading is generated in ctor of EnhancedCustomShape2d from the Type value.
    loadFromFile(u"tdf109169_OctagonBevel.odt");

    // Export to docx had not written a:fill or a:stroke attributes at all.
    save(TestFilter::DOCX);

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // File should have six subpaths, one with stroke and five with fill
    assertXPath(pXmlDoc, "//a:pathLst/a:path[1]", "stroke", u"0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]", "fill", u"darkenLess");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[3]", "fill", u"darken");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[4]", "fill", u"darken");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[5]", "fill", u"lightenLess");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[6]", "fill", u"lighten");
}

CPPUNIT_TEST_FIXTURE(Test, testFaultyPathCommandsAWT)
{
    // The odp file contains shapes whose path starts with command A, W, T or L. That is a faulty
    // path. LO is tolerant and renders it so that is makes a moveTo to the start point of the arc or
    // the end of the line respectively. Export to OOXML does the same now and writes a moveTo
    // instead of the normally used lnTo. If a lnTo is written, MS Office shows nothing of the shape.
    loadFromFile(u"FaultyPathStart.odp");

    save(TestFilter::PPTX);

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
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
    loadFromFile(u"tdf148784_StretchXY.odp");
    save(TestFilter::PPTX);

    // Verify the markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // The path is in the extent of the shape; the x-position of the last segment is the far edge
    // of the shape, w.
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path", "w");
    assertXPath(pXmlDoc, "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:moveTo[4]/a:pt",
                "x", u"w");

    // the y-position of the last segment is the far edge of the shape, h
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path", "h");
    assertXPath(pXmlDoc, "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/a:moveTo[4]/a:pt",
                "y", u"h");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148784StretchCommandQ)
{
    // The document has a custom shapes of type "non-primitive" to trigger the custGeom export.
    // They use formulas with 'right' and 'bottom'.
    // When saving to PPTX the attributes stretchpoint-x and stretchpoint-y were not considered.
    // That results in wrong arcs on the right or bottom side of the shape.
    loadFromFile(u"tdf148784_StretchCommandQ.odp");
    save(TestFilter::PPTX);

    // Verify the markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // The path is in the extent of the shape; the x-position of the second quadBezTo control
    // is the far edge of the shape, w.
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path", "w");
    assertXPath(pXmlDoc,
                "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:quadBezTo[2]/a:pt[1]", "x",
                u"w");

    // the y-position of the third quadBezTo control is the far edge of the shape, h
    assertXPath(pXmlDoc,
                "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/a:quadBezTo[3]/a:pt[1]", "y",
                u"h");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148784StretchCommandVW)
{
    // The document has a custom shapes of type "non-primitive" to trigger the custGeom export.
    // It should not need adaption when export of handles and guides is implemented because it
    // has only fixed values in the path.
    // When saving to PPTX the attributes stretchpoint-x and stretchpoint-y were not considered.
    // That results in circles instead of ellipses.
    loadFromFile(u"tdf148784_StretchCommandVW.odp");
    save(TestFilter::PPTX);

    // Verify the markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // The path is in the extent of the shape; wR of the first arcTo of the first shape is a
    // guide: half of the box of the arc, whose far edge is w, in the extent.
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path", "w");
    OUString aWR = getXPath(
        pXmlDoc, "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:pathLst/a:path/a:arcTo[1]", "wR");
    CPPUNIT_ASSERT(aWR.startsWith("f"));
    OUString aFormula = getXPath(
        pXmlDoc,
        OString("//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:gdLst/a:gd[@name='" + aWR.toUtf8() + "']"),
        "fmla");
    CPPUNIT_ASSERT(aFormula.startsWith("*/ f"));
    CPPUNIT_ASSERT(aFormula.endsWith(" 1 2"));
    assertXPath(pXmlDoc, "//p:spTree/p:sp[1]/p:spPr/a:custGeom/a:gdLst/a:gd[@fmla='*/ w 21600 ss']",
                1);

    // hR of the first arcTo of the second shape the same way with h
    OUString aHR = getXPath(
        pXmlDoc, "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:pathLst/a:path/a:arcTo[1]", "hR");
    CPPUNIT_ASSERT(aHR.startsWith("f"));
    aFormula = getXPath(
        pXmlDoc,
        OString("//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:gdLst/a:gd[@name='" + aHR.toUtf8() + "']"),
        "fmla");
    CPPUNIT_ASSERT(aFormula.startsWith("*/ f"));
    CPPUNIT_ASSERT(aFormula.endsWith(" 1 2"));
    assertXPath(pXmlDoc, "//p:spTree/p:sp[2]/p:spPr/a:custGeom/a:gdLst/a:gd[@fmla='*/ h 21600 ss']",
                1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149551VertPadding)
{
    // The document has shape[1] with attribute vert="vert270" and shape[2] with vert="vert". The text
    // has paddings lIns="720000"=2cm, tIns="360000"=1cm, rIns="0" and bIns="0".
    // After load and save the paddings were rotated and a 90deg text rotation was added.
    loadFromFile(u"tdf149551_vert_and_padding.pptx");
    save(TestFilter::PPTX);

    // Verify the markup. The values must be the same as in the original file.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    for (sal_Int32 i = 1; i <= 2; i++)
    {
        OString sElement = "//p:spTree/p:sp[" + OString::number(i) + "]/p:txBody/a:bodyPr";
        assertXPath(pXmlDoc, sElement, "lIns", u"720000");
        assertXPath(pXmlDoc, sElement, "tIns", u"360000");
        assertXPath(pXmlDoc, sElement, "rIns", u"0");
        assertXPath(pXmlDoc, sElement, "bIns", u"0");
        assertXPathNoAttribute(pXmlDoc, sElement, "rot");
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149538upright)
{
    // The document has a shape with attribute upright="1" in the bodyPr element. On import it is
    // emulated by rotating the text area rectangle. On export there should be an upright="1"
    // attribute but no 'rot' attribute. Without the fix the 'rot' attribute with values from
    // the emulation was written out.
    loadFromFile(u"tdf149538_upright.pptx");
    save(TestFilter::PPTX);

    // Verify the markup. The values must be the same as in the original file.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//p:spTree/p:sp/p:txBody/a:bodyPr", "upright", u"1");
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp/p:txBody/a:bodyPr", "rot");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf151008VertAnchor)
{
    loadFromFile(u"tdf151008_eaVertAnchor.pptx");
    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // The order of the shapes in the file is by name "Right", "Center", "Left", "RightMiddle",
    // "CenterMiddle" and "LeftMiddle". I access the shapes here by index, because the XPath is
    // easier then.
    // As of Sep 2022 LibreOffice does not write the default anchorCtr="0"
    // Right
    assertXPath(pXmlDoc, "//p:spTree/p:sp[1]/p:txBody/a:bodyPr", "anchor", u"t");
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp[1]/p:txBody/a:bodyPr", "anchorCtr");
    // Center
    assertXPath(pXmlDoc, "//p:spTree/p:sp[2]/p:txBody/a:bodyPr", "anchor", u"ctr");
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp[2]/p:txBody/a:bodyPr", "anchorCtr");
    // Left
    assertXPath(pXmlDoc, "//p:spTree/p:sp[3]/p:txBody/a:bodyPr", "anchor", u"b");
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp[3]/p:txBody/a:bodyPr", "anchorCtr");
    // RightMiddle
    assertXPath(pXmlDoc, "//p:spTree/p:sp[4]/p:txBody/a:bodyPr", "anchor", u"t");
    assertXPath(pXmlDoc, "//p:spTree/p:sp[4]/p:txBody/a:bodyPr", "anchorCtr", u"1");
    // CenterMiddle
    assertXPath(pXmlDoc, "//p:spTree/p:sp[5]/p:txBody/a:bodyPr", "anchor", u"ctr");
    assertXPath(pXmlDoc, "//p:spTree/p:sp[5]/p:txBody/a:bodyPr", "anchorCtr", u"1");
    // LeftMiddle
    assertXPath(pXmlDoc, "//p:spTree/p:sp[6]/p:txBody/a:bodyPr", "anchor", u"b");
    assertXPath(pXmlDoc, "//p:spTree/p:sp[6]/p:txBody/a:bodyPr", "anchorCtr", u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkBitmapFill)
{
    // The document has a Fontwork shape with bitmap fill.
    loadFromFile(u"tdf128568_FontworkBitmapFill.odt");

    // Saving that to DOCX:
    save(TestFilter::DOCX);

    // Make sure it is exported to VML and has no txbxContent but a textpath element. Without fix it
    // was exported as DML 'abc transform', but that is not able to use bitmap fill in Word.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//mc:alternateContent", 0);
    assertXPath(pXmlDoc, "//v:shape/v:textbox/v:txbxContent", 0);
    assertXPath(pXmlDoc, "//v:shape/v:textpath", 1);

    // Without fix the bitmap was referenced by v:imagedata element. But that produces a picture
    // in Word not a WordArt shape. Instead a v:fill has to be used.
    assertXPath(pXmlDoc, "//v:shape/v:imagedata", 0);
    assertXPath(pXmlDoc, "//v:shape/v:fill", 1);
    assertXPath(pXmlDoc, "//v:shape/v:fill[@r:id]", 1);

    // The fill is set to 'stretched' in LO, that is type="frame" in VML. That was not implemented
    // in VML export.
    assertXPath(pXmlDoc, "//v:shape/v:fill", "type", u"frame");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkFontProperties)
{
    // The document has five Fontwork shapes. They have bitmap fill and thus are exported to VML.
    // They differ in font properties e.g. font weight and character spacing.
    loadFromFile(u"tdf128568_FontworkFontProperties.odt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Make sure the style attribute of the textpath element has the needed items. Without fix only
    // font-family and font-size were written.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OUString sStyle;
    // bold
    sStyle = getXPath(pXmlDoc, "(//v:shape)[1]/v:textpath", "style");
    CPPUNIT_ASSERT(sStyle.indexOf("font-weight:bold") > -1);
    // italic
    sStyle = getXPath(pXmlDoc, "(//v:shape)[2]/v:textpath", "style");
    CPPUNIT_ASSERT(sStyle.indexOf("font-style:italic") > -1);
    // character spacing 'very loose', 150 * 655, see escherex.cxx
    sStyle = getXPath(pXmlDoc, "(//v:shape)[3]/v:textpath", "style");
    CPPUNIT_ASSERT(sStyle.indexOf("v-text-spacing:98250f") > -1);
    // character spacing 'tight', 90 * 655, see escherex.cxx
    sStyle = getXPath(pXmlDoc, "(//v:shape)[4]/v:textpath", "style");
    CPPUNIT_ASSERT(sStyle.indexOf("v-text-spacing:58950f") > -1);
    // same letter heights
    sStyle = getXPath(pXmlDoc, "(//v:shape)[5]/v:textpath", "style");
    CPPUNIT_ASSERT(sStyle.indexOf("v-same-letter-heights:t") > -1);
}

CPPUNIT_TEST_FIXTURE(Test, testVMLFontworkSlantUp)
{
    // The document has a Fontwork shape type 'textSlantUp' (172). When exporting to docx, Word does
    // not recognize its markup as preset WordArt, because the used markup differs from what Word
    // expects for this type of shape. As a result Word saves the shape as having custom geometry
    // and such is not understand by LibreOffice.
    loadFromFile(u"tdf153296_VML_export_SlantUp.odt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup. I could identify the following two problems to hinder Word from
    // accepting the markup. There might exist more problems.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Make sure a <v:path> element exists and has an o:connecttype attribute
    assertXPath(pXmlDoc, "//v:shapetype/v:path", 1);
    assertXPath(pXmlDoc, "//v:shapetype/v:path[@o:connecttype]", 1);

    // Make sure the handle position is written without reference to a formula
    OUString sPosition = getXPath(pXmlDoc, "//v:h", "position");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), sPosition.indexOf("@"));
    CPPUNIT_ASSERT_EQUAL(u"topLeft,#0"_ustr, sPosition);
}

CPPUNIT_TEST_FIXTURE(Test, testVMLFontworkArchUp)
{
    // The document has a Fontwork shape type 'textArchUp' (172). When exporting to docx, the shape
    // was not exported as VML Fontwork but as a rectangle.
    loadFromFile(u"tdf153296_VML_export_ArchUpCurve.odt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Make sure there is no <v:rect> element
    assertXPath(pXmlDoc, "//v:rect", 0);
    // ..., but a <v:shapetype> element with <v:textpath> subelement
    assertXPath(pXmlDoc, "//v:shapetype/v:textpath", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testVMLAdjustmentExport)
{
    // The document has a Fontwork shape type 'textCirclePour' (150). When exporting to docx, the
    // adjustment values were not exported at all.
    loadFromFile(u"tdf153246_VML_export_Fontwork_Adjustment.odt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Make sure an "adj" attribute exists
    assertXPath(pXmlDoc, "//v:shape[@adj]", 1);
    // ... and has the correct values
    OUString sAdjustments = getXPath(pXmlDoc, "//v:shape", "adj");
    sal_Int32 nTokenStart = 0;
    OUString sAngle = sAdjustments.getToken(0, ',', nTokenStart);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(-7341733), sAngle.toInt32(), 2);
    OUString sRadius = sAdjustments.copy(nTokenStart);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(5296), sRadius.toInt32(), 2);
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkDirectColor)
{
    // The document has a Fontwork shape with fill #2e8b57 plus 60% transparency and line color
    // #ff7f50 with 20% transparency. When exporting to docx, line color was not exported at all
    // and fill color was replaced with character color.
    loadFromFile(u"tdf51195_Fontwork_DirectColor.odt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    static constexpr OString sElement = "//w:txbxContent/w:p/w:r/w:rPr/"_ostr;

    // Make sure the fallback in <w:color> has correct value.
    assertXPath(pXmlDoc, sElement + "w:color", "val", u"2E8B57");

    // ... and <w14:textOutline> exists and has correct values.
    assertXPath(pXmlDoc, sElement + "w14:textOutline", 1);
    assertXPath(pXmlDoc, sElement + "w14:textOutline/w14:solidFill/w14:srgbClr", "val", u"ff7f50");
    assertXPath(pXmlDoc, sElement + "w14:textOutline/w14:solidFill/w14:srgbClr/w14:alpha", "val",
                u"20000");
    assertXPath(pXmlDoc, sElement + "w14:textOutline/w14:round", 1);

    // ... and w14:textFill exists and has correct values.
    assertXPath(pXmlDoc, sElement + "w14:textFill", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:srgbClr", "val", u"2e8b57");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:srgbClr/w14:alpha", "val",
                u"60000");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkThemeColor)
{
    // The document has a Fontwork shape with fill theme color 'lt2' and 50% darker, and a Fontwork
    // shape with theme color 'dk2' and 40% lighter. When exporting to docx, fill color was not
    // exported as theme color at all, and it was replaced with character color.
    loadFromFile(u"tdf51195_Fontwork_ThemeColor.fodt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // shape with 'darker'
    OString sElement = "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/"_ostr;

    // Make sure the fallback in <w:color> has correct values
    assertXPath(pXmlDoc, sElement + "w:color", "val", u"948A54");
    assertXPath(pXmlDoc, sElement + "w:color", "themeColor", u"light2");
    assertXPath(pXmlDoc, sElement + "w:color", "themeShade", u"80");

    // ... and w14:textFill exists and has correct values.
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:schemeClr", "val", u"bg2");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:schemeClr/w14:lumMod", "val",
                u"50000");

    // shape with 'lighter'
    sElement = "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/"
               "a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/"_ostr;

    // Make sure the fallback in <w:color> has correct values
    assertXPath(pXmlDoc, sElement + "w:color", "val", u"558ED5");
    assertXPath(pXmlDoc, sElement + "w:color", "themeColor", u"dark2");
    assertXPath(pXmlDoc, sElement + "w:color", "themeTint", u"99");

    // ... and w14:textFill exists and has correct values.
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:schemeClr", "val", u"tx2");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:schemeClr/w14:lumMod", "val",
                u"60000");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:schemeClr/w14:lumOff", "val",
                u"40000");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkPlainTextIsDML)
{
    // Given plain-text, slant-up and deflate-inflate-deflate Fontwork shapes in the body, and a
    // plain-text Fontwork named as a watermark in the header:
    loadFromFile(u"fontwork-dml-plain-slant-deflate.fodt");

    // When saving to DOCX:
    save(TestFilter::DOCX);

    // Then the three shapes of the body are WordArt text boxes with a VML fallback, under their
    // names, and the watermark stays a VML shape
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OString sBodyPr = "//w:body//mc:AlternateContent/mc:Choice//wps:wsp/wps:bodyPr/a:prstTxWarp"_ostr;
    assertXPath(pXmlDoc, sBodyPr, 3);
    assertXPath(pXmlDoc, sBodyPr + "[@prst='textPlain']", 1);
    assertXPath(pXmlDoc, sBodyPr + "[@prst='textSlantUp']", 1);
    assertXPath(pXmlDoc, sBodyPr + "[@prst='textDeflateInflateDeflate']", 1);
    assertXPath(pXmlDoc, "//w:body//mc:AlternateContent/mc:Fallback//v:shape/v:textpath", 3);
    assertXPath(pXmlDoc, "//w:body//mc:AlternateContent/mc:Choice//wp:docPr[@name='Plain']", 1);
    xmlDocUniquePtr pXmlHeader = parseExport(u"word/header1.xml"_ustr);
    assertXPath(pXmlHeader, "//mc:AlternateContent", 0);
    assertXPath(pXmlHeader, "//w:pict/v:shape/v:textpath", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkLegacyTextTypes)
{
    // Given two Fontwork shapes of the text types of the binary formats: a wave (mso-spt28, with a
    // third modifier its preset does not have) and an arch (mso-spt31):
    loadFromFile(u"fontwork-legacy-ids.fodt");

    // When saving to DOCX:
    save(TestFilter::DOCX);

    // Then they are written with the presets the engine draws them with, with the two adjustments
    // of the wave and the one of the arch
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OString sWarp = "//mc:Choice//wps:wsp/wps:bodyPr/a:prstTxWarp"_ostr;
    assertXPath(pXmlDoc, sWarp + "[@prst='textWave1']/a:avLst/a:gd", 2);
    assertXPath(pXmlDoc, sWarp + "[@prst='textWave1']/a:avLst/a:gd[1]", "fmla", u"val 18750");
    assertXPath(pXmlDoc, sWarp + "[@prst='textWave1']/a:avLst/a:gd[2]", "fmla", u"val -7500");
    assertXPath(pXmlDoc, sWarp + "[@prst='textArchUp']/a:avLst/a:gd", 1);
    assertXPath(pXmlDoc, sWarp + "[@prst='textArchUp']/a:avLst/a:gd", "fmla", u"val 0");

    // and reading the file back gives the WordArt types of the same geometry
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPageSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPage());
    uno::Reference<beans::XPropertySet> xWave(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    comphelper::SequenceAsHashMap aGeometry(xWave->getPropertyValue(u"CustomShapeGeometry"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"fontwork-wave"_ustr, aGeometry[u"Type"_ustr].get<OUString>());
    uno::Reference<beans::XPropertySet> xArch(xPage->getByIndex(1), uno::UNO_QUERY_THROW);
    aGeometry = comphelper::SequenceAsHashMap(xArch->getPropertyValue(u"CustomShapeGeometry"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"fontwork-arch-up-curve"_ustr, aGeometry[u"Type"_ustr].get<OUString>());
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkAlignment)
{
    // Given five plain-text Fontwork shapes aligned by the shape: left, centre, right, justified,
    // and justified with the text fitted to the shape (stretch):
    loadFromFile(u"fontwork-alignment.fodp");

    // When saving to PPTX:
    save(TestFilter::PPTX);

    // Then every paragraph is aligned as the shape is
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//p:sp[1]//a:p/a:pPr[@algn]", 0);
    assertXPath(pXmlDoc, "//p:sp[2]//a:p/a:pPr", "algn", u"ctr");
    assertXPath(pXmlDoc, "//p:sp[3]//a:p/a:pPr", "algn", u"r");
    assertXPath(pXmlDoc, "//p:sp[4]//a:p/a:pPr", "algn", u"just");
    assertXPath(pXmlDoc, "//p:sp[5]//a:p/a:pPr", "algn", u"dist");

    // and reading the file back gives the shapes their alignment and the stretch its fit
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    drawing::TextHorizontalAdjust eAdjust = drawing::TextHorizontalAdjust_CENTER;
    uno::Reference<beans::XPropertySet> xRight(xPage->getByIndex(2), uno::UNO_QUERY_THROW);
    xRight->getPropertyValue(u"TextHorizontalAdjust"_ustr) >>= eAdjust;
    CPPUNIT_ASSERT_EQUAL(drawing::TextHorizontalAdjust_RIGHT, eAdjust);
    uno::Reference<beans::XPropertySet> xJustify(xPage->getByIndex(3), uno::UNO_QUERY_THROW);
    xJustify->getPropertyValue(u"TextHorizontalAdjust"_ustr) >>= eAdjust;
    CPPUNIT_ASSERT_EQUAL(drawing::TextHorizontalAdjust_BLOCK, eAdjust);
    uno::Reference<beans::XPropertySet> xStretch(xPage->getByIndex(4), uno::UNO_QUERY_THROW);
    xStretch->getPropertyValue(u"TextHorizontalAdjust"_ustr) >>= eAdjust;
    CPPUNIT_ASSERT_EQUAL(drawing::TextHorizontalAdjust_BLOCK, eAdjust);
    drawing::TextFitToSizeType eFit = drawing::TextFitToSizeType_NONE;
    xStretch->getPropertyValue(u"TextFitToSize"_ustr) >>= eFit;
    CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_ALLLINES, eFit);
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkAlignmentWriter)
{
    // Given the same five Fontwork shapes in a Writer document:
    loadFromFile(u"fontwork-alignment.fodt");

    // When saving to DOCX:
    save(TestFilter::DOCX);

    // Then the paragraphs of the WordArt text boxes are aligned as the shapes are
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OString sPara = "//mc:Choice//wps:wsp/wps:txbx/w:txbxContent/w:p/w:pPr/w:jc"_ostr;
    assertXPath(pXmlDoc, "(//mc:Choice//wps:wsp)[2]//w:txbxContent/w:p/w:pPr/w:jc", "val", u"center");
    assertXPath(pXmlDoc, "(//mc:Choice//wps:wsp)[3]//w:txbxContent/w:p/w:pPr/w:jc", "val", u"end");
    assertXPath(pXmlDoc, "(//mc:Choice//wps:wsp)[4]//w:txbxContent/w:p/w:pPr/w:jc", "val", u"both");
    assertXPath(pXmlDoc, "(//mc:Choice//wps:wsp)[5]//w:txbxContent/w:p/w:pPr/w:jc", "val",
                u"distribute");
    assertXPath(pXmlDoc, "(//mc:Choice//wps:wsp)[1]//w:txbxContent/w:p/w:pPr/w:jc", "val", u"start");
    assertXPath(pXmlDoc, sPara, 5);

    // and reading the file back gives the shapes their alignment and the stretch its fit
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPageSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPage());
    drawing::TextHorizontalAdjust eAdjust = drawing::TextHorizontalAdjust_CENTER;
    uno::Reference<beans::XPropertySet> xJustify(xPage->getByIndex(3), uno::UNO_QUERY_THROW);
    xJustify->getPropertyValue(u"TextHorizontalAdjust"_ustr) >>= eAdjust;
    CPPUNIT_ASSERT_EQUAL(drawing::TextHorizontalAdjust_BLOCK, eAdjust);
    uno::Reference<beans::XPropertySet> xStretch(xPage->getByIndex(4), uno::UNO_QUERY_THROW);
    drawing::TextFitToSizeType eFit = drawing::TextFitToSizeType_NONE;
    xStretch->getPropertyValue(u"TextFitToSize"_ustr) >>= eFit;
    CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_ALLLINES, eFit);
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkShapeAttributes)
{
    // Given a Fontwork whose shape draws its text in Impact 24pt bold italic with letter spacing
    // and 150% character width, while the span of its text says Liberation Sans 12pt regular:
    loadFromFile(u"fontwork-shape-attributes.fodp");

    // When saving to PPTX:
    save(TestFilter::PPTX);

    // Then the run is written with the attributes the shape draws
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//p:sp/p:txBody/a:p/a:r/a:rPr", "sz", u"2400");
    assertXPath(pXmlDoc, "//p:sp/p:txBody/a:p/a:r/a:rPr", "b", u"1");
    assertXPath(pXmlDoc, "//p:sp/p:txBody/a:p/a:r/a:rPr", "i", u"1");
    assertXPath(pXmlDoc, "//p:sp/p:txBody/a:p/a:r/a:rPr", "spc", u"283");
    assertXPath(pXmlDoc, "//p:sp/p:txBody/a:p/a:r/a:rPr/a:latin", "typeface", u"Impact");

    // and reading the file back gives the shape those attributes
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Impact"_ustr,
                         xShape->getPropertyValue(u"CharFontName"_ustr).get<OUString>());
    CPPUNIT_ASSERT_EQUAL(24.0f, xShape->getPropertyValue(u"CharHeight"_ustr).get<float>());
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         xShape->getPropertyValue(u"CharKerning"_ustr).get<sal_Int16>());
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkShapeAttributesWriter)
{
    // Given the same Fontwork in a Writer document, with the states of the attributes of its two
    // spans as they are:
    loadFromFile(u"fontwork-shape-attributes.fodt");
    beans::PropertyState eFirstFontState = beans::PropertyState_DIRECT_VALUE;
    beans::PropertyState eSecondWidthState = beans::PropertyState_DIRECT_VALUE;
    {
        uno::Reference<drawing::XDrawPageSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xShape(xDoc->getDrawPage()->getByIndex(0),
                                                   uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumerationAccess> xParas(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumerationAccess> xRuns(
            xParas->createEnumeration()->nextElement(), uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumeration> xRunEnum = xRuns->createEnumeration();
        uno::Reference<beans::XPropertyState> xFirst(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertyState> xSecond(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
        eFirstFontState = xFirst->getPropertyState(u"CharFontName"_ustr);
        eSecondWidthState = xSecond->getPropertyState(u"CharScaleWidth"_ustr);
        // the first span's font is its own, the second span's width is inherited
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, eFirstFontState);
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE, eSecondWidthState);
    }

    // When saving to DOCX:
    save(TestFilter::DOCX);

    // Then the runs of the WordArt text box are written with the attributes the shape draws, the
    // character width among them; the two spans of the text, bold and regular, are one run
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OString sRun = "//mc:Choice//wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/"_ostr;
    assertXPath(pXmlDoc, "//mc:Choice//wps:wsp/wps:txbx/w:txbxContent/w:p/w:r", 1);
    assertXPath(pXmlDoc, sRun + "w:rFonts", "ascii", u"Impact");
    assertXPath(pXmlDoc, sRun + "w:sz", "val", u"48");
    assertXPath(pXmlDoc, sRun + "w:b", 1);
    assertXPath(pXmlDoc, sRun + "w:i", 1);
    assertXPath(pXmlDoc, sRun + "w:w", "val", u"150");

    // and the document's own text keeps its two spans after the save: the second is regular
    {
        uno::Reference<drawing::XDrawPageSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xShape(xDoc->getDrawPage()->getByIndex(0),
                                                   uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumerationAccess> xParas(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumerationAccess> xRuns(
            xParas->createEnumeration()->nextElement(), uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumeration> xRunEnum = xRuns->createEnumeration();
        uno::Reference<beans::XPropertySet> xFirst(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xRunEnum->hasMoreElements());
        uno::Reference<beans::XPropertySet> xSecond(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             xFirst->getPropertyValue(u"CharWeight"_ustr).get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                             xSecond->getPropertyValue(u"CharWeight"_ustr).get<float>());
        // and the attributes of the spans are what they were, the inherited width included
        uno::Reference<beans::XPropertyState> xFirstState(xFirst, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(eFirstFontState, xFirstState->getPropertyState(u"CharFontName"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Liberation Sans"_ustr,
                             xFirst->getPropertyValue(u"CharFontName"_ustr).get<OUString>());
        uno::Reference<beans::XPropertyState> xSecondState(xSecond, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(eSecondWidthState,
                             xSecondState->getPropertyState(u"CharScaleWidth"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                             xSecond->getPropertyValue(u"CharScaleWidth"_ustr).get<sal_Int16>());
    }

    // and reading the file back gives the shape those attributes
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPageSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPage());
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Impact"_ustr,
                         xShape->getPropertyValue(u"CharFontName"_ustr).get<OUString>());
    CPPUNIT_ASSERT_EQUAL(24.0f, xShape->getPropertyValue(u"CharHeight"_ustr).get<float>());
    CPPUNIT_ASSERT_EQUAL(sal_Int16(150),
                         xShape->getPropertyValue(u"CharScaleWidth"_ustr).get<sal_Int16>());
}

// The colour of the portion with the given text in the first paragraph of a shape's text.
sal_Int32 lcl_PortionColor(const uno::Reference<beans::XPropertySet>& xShape,
                           std::u16string_view rText)
{
    uno::Reference<container::XEnumerationAccess> xParas(xShape, uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumerationAccess> xRuns(xParas->createEnumeration()->nextElement(),
                                                        uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumeration> xRunEnum = xRuns->createEnumeration();
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
        if (xRun->getString() != rText)
            continue;
        uno::Reference<beans::XPropertySet> xProps(xRun, uno::UNO_QUERY_THROW);
        sal_Int32 nColor = 0;
        xProps->getPropertyValue(u"CharColor"_ustr) >>= nColor;
        return nColor;
    }
    CPPUNIT_FAIL("portion not found");
    return 0;
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkRunColours)
{
    // Given a red, stroked Fontwork whose text has a red span, a span without a colour and a
    // blue span, as the import of a WordArt coloured run by run leaves it:
    loadFromFile(u"fontwork-run-colours.fodp");

    // When saving to PPTX:
    save(TestFilter::PPTX);

    // Then the red and the uncoloured words take the fill of the shape and "Blue" keeps its blue,
    // with the outline of the shape on every run (the runs are what the engine's text gives: the
    // uncoloured word may share a run with a red one)
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    OString sRun = "//p:sp/p:txBody/a:p/a:r"_ostr;
    assertXPath(pXmlDoc, sRun + "[starts-with(a:t,'Red')]/a:rPr/a:solidFill/a:srgbClr", "val",
                u"FF0000");
    assertXPath(pXmlDoc, sRun + "[contains(a:t,'Green')]/a:rPr/a:solidFill/a:srgbClr", "val",
                u"FF0000");
    assertXPath(pXmlDoc, sRun + "[a:t='Blue']/a:rPr/a:solidFill/a:srgbClr", "val", u"0000FF");
    assertXPath(pXmlDoc, sRun + "[a:t='Blue']/a:rPr/a:ln/a:solidFill/a:srgbClr", "val", u"000000");

    // and reading the file back gives a red shape with a blue third portion
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000),
                         xShape->getPropertyValue(u"FillColor"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0000ff), lcl_PortionColor(xShape, u"Blue"));
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkRunColoursWriter)
{
    // Given the same Fontwork in a Writer document:
    loadFromFile(u"fontwork-run-colours.fodt");

    // When saving to DOCX:
    save(TestFilter::DOCX);

    // Then the first two runs take the fill of the shape and the third keeps its blue, as its
    // colour and without the text fill of the shape, with the outline of the shape on every run
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OString sRuns = "//mc:Choice//wps:wsp/wps:txbx/w:txbxContent/w:p/w:r"_ostr;
    assertXPath(pXmlDoc, sRuns + "[starts-with(w:t,'Red')]/w:rPr/w14:textFill/w14:solidFill/w14:srgbClr",
                "val", u"ff0000");
    assertXPath(pXmlDoc, sRuns + "[contains(w:t,'Green')]/w:rPr/w14:textFill/w14:solidFill/w14:srgbClr",
                "val", u"ff0000");
    assertXPath(pXmlDoc, sRuns + "[w:t='Blue']/w:rPr/w:color", "val", u"0000FF");
    assertXPath(pXmlDoc, sRuns + "[w:t='Blue']/w:rPr/w14:textFill", 0);
    assertXPath(pXmlDoc, sRuns + "[w:t='Blue']/w:rPr/w14:textOutline", 1);
    assertXPath(pXmlDoc, sRuns + "[starts-with(w:t,'Red')]/w:rPr/w14:textOutline", 1);

    // and reading the file back gives a red shape with a blue third portion
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPageSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPage());
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000),
                         xShape->getPropertyValue(u"FillColor"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0000ff), lcl_PortionColor(xShape, u"Blue"));
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkGlowShadowRoundTrip)
{
    // Given a Fontwork with a glow of 3 mm and a shadow of 2 mm:
    loadFromFile(u"fontwork-glow-shadow.fodp");

    // When saving to PPTX, where the effects go on the runs of the WordArt:
    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//p:sp/p:txBody/a:p/a:r/a:rPr/a:effectLst/a:glow", "rad", u"108000");
    assertXPath(pXmlDoc, "//p:sp/p:txBody/a:p/a:r/a:rPr/a:effectLst/a:outerShdw", 1);

    // Then reading the file back gives the shape its glow and shadow
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(300),
                         xShape->getPropertyValue(u"GlowEffectRadius"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00ff00),
                         xShape->getPropertyValue(u"GlowEffectColor"_ustr).get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(true, xShape->getPropertyValue(u"Shadow"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(200),
                         xShape->getPropertyValue(u"ShadowXDistance"_ustr).get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkArchScaleXWriter)
{
    // Given two arch Fontworks anchored at the bottom, one with the text scaled to the path, one
    // stretched to the shape:
    loadFromFile(u"fontwork-arch-scalex.fodt");

    // When saving to DOCX:
    save(TestFilter::DOCX);

    // Then only the stretched one is a WordArt of the legacy kind, and both anchor at the bottom
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "(//mc:Choice//wps:wsp/wps:bodyPr)[1]", "fromWordArt", u"1");
    assertXPathNoAttribute(pXmlDoc, "(//mc:Choice//wps:wsp/wps:bodyPr)[2]", "fromWordArt");
    assertXPath(pXmlDoc, "(//mc:Choice//wps:wsp/wps:bodyPr)[1]", "anchor", u"b");

    // and reading the file back gives each its scaling and the anchor
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPageSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPage());
    for (sal_Int32 i = 0; i < 2; ++i)
    {
        // the first arch keeps its text scaled to the path, the second stretched to the shape
        const bool bScaleX = i == 1;
        uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(i), uno::UNO_QUERY_THROW);
        comphelper::SequenceAsHashMap aGeometry(xShape->getPropertyValue(u"CustomShapeGeometry"_ustr));
        comphelper::SequenceAsHashMap aTextPath(aGeometry[u"TextPath"_ustr]);
        CPPUNIT_ASSERT_EQUAL(bScaleX, aTextPath[u"ScaleX"_ustr].get<bool>());
        drawing::TextVerticalAdjust eAdjust = drawing::TextVerticalAdjust_TOP;
        xShape->getPropertyValue(u"TextVerticalAdjust"_ustr) >>= eAdjust;
        CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM, eAdjust);
    }
}

/// Asserts the extrusion the fontwork-3d fixtures carry: 1 cm deep, metal, turned by 20 and -30.
void assertExtrusionOfShape(const uno::Reference<beans::XPropertySet>& xShape)
{
    comphelper::SequenceAsHashMap aGeometry(xShape->getPropertyValue(u"CustomShapeGeometry"_ustr));
    comphelper::SequenceAsHashMap aExtrusion(aGeometry[u"Extrusion"_ustr]);
    CPPUNIT_ASSERT_EQUAL(true, aExtrusion[u"Extrusion"_ustr].get<bool>());
    CPPUNIT_ASSERT_EQUAL(true, aExtrusion[u"Metal"_ustr].get<bool>());
    drawing::EnhancedCustomShapeParameterPair aAngle;
    aExtrusion[u"RotateAngle"_ustr] >>= aAngle;
    double fX = 0;
    double fY = 0;
    aAngle.First.Value >>= fX;
    aAngle.Second.Value >>= fY;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, fX, 0.5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-30.0, fY, 0.5);
    drawing::EnhancedCustomShapeParameterPair aDepth;
    aExtrusion[u"Depth"_ustr] >>= aDepth;
    double fDepth = 0;
    aDepth.First.Value >>= fDepth;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.0, fDepth, 1.0);
}

CPPUNIT_TEST_FIXTURE(Test, testExtrusionToScene3d)
{
    // Given an extruded Fontwork and an extruded rectangle: depth 1 cm (three tenths of it in front
    // of the rectangle), the direction bottom right of the extrusion toolbar (skew 135, origin
    // 0.5 0.5), light from the top right, metal, turned by 20 degrees around x and -30 around y,
    // extrusion colour blue:
    loadFromFile(u"fontwork-3d.fodp");

    // When saving to PPTX:
    save(TestFilter::PPTX);

    // Then the text of the Fontwork and the rectangle get a camera, a light rig and an extrusion
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    OString sText = "//p:sp[1]/p:txBody/a:bodyPr/"_ostr;
    assertXPath(pXmlDoc, sText + "a:scene3d/a:camera", "prst", u"legacyObliqueBottomRight");
    assertXPath(pXmlDoc, sText + "a:scene3d/a:camera/a:rot", "lat", u"20400000");
    assertXPath(pXmlDoc, sText + "a:scene3d/a:camera/a:rot", "lon", u"19800000");
    assertXPath(pXmlDoc, sText + "a:scene3d/a:lightRig", "rig", u"legacyFlat3");
    assertXPath(pXmlDoc, sText + "a:scene3d/a:lightRig", "dir", u"tr");
    assertXPath(pXmlDoc, sText + "a:sp3d", "extrusionH", u"360000");
    assertXPath(pXmlDoc, sText + "a:sp3d", "prstMaterial", u"metal");
    assertXPath(pXmlDoc, sText + "a:sp3d/a:extrusionClr/a:srgbClr", "val", u"0000FF");
    OString sShape = "//p:sp[2]/p:spPr/"_ostr;
    assertXPath(pXmlDoc, sShape + "a:scene3d/a:camera", "prst", u"legacyObliqueBottomRight");
    assertXPath(pXmlDoc, sShape + "a:sp3d", "extrusionH", u"360000");
    assertXPath(pXmlDoc, sShape + "a:sp3d", "z", u"108000");

    // and reading the file back gives the rectangle and the Fontwork their extrusion
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xRectangle(xPage->getByIndex(1), uno::UNO_QUERY_THROW);
    assertExtrusionOfShape(xRectangle);
    comphelper::SequenceAsHashMap aRectangleGeometry(
        xRectangle->getPropertyValue(u"CustomShapeGeometry"_ustr));
    comphelper::SequenceAsHashMap aRectangleExtrusion(aRectangleGeometry[u"Extrusion"_ustr]);
    drawing::EnhancedCustomShapeParameterPair aDepth;
    aRectangleExtrusion[u"Depth"_ustr] >>= aDepth;
    double fFraction = 0;
    aDepth.Second.Value >>= fFraction;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3, fFraction, 0.01);
    uno::Reference<beans::XPropertySet> xFontwork(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    assertExtrusionOfShape(xFontwork);
    comphelper::SequenceAsHashMap aGeometry(
        xFontwork->getPropertyValue(u"CustomShapeGeometry"_ustr));
    comphelper::SequenceAsHashMap aTextPath(aGeometry[u"TextPath"_ustr]);
    CPPUNIT_ASSERT_EQUAL(true, aTextPath[u"TextPath"_ustr].get<bool>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0000FF),
                         xFontwork->getPropertyValue(u"FillColor2"_ustr).get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(Test, testExtrusionToScene3dWriter)
{
    // Given the same extruded Fontwork and rectangle in a Writer document, and a Fontwork turned
    // by 30 degrees:
    loadFromFile(u"fontwork-3d.fodt");

    // When saving to DOCX:
    save(TestFilter::DOCX);

    // Then the WordArt text box gets its 3D in the body properties
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OString sBody = "//mc:Choice//wp:anchor[wp:docPr/@name='Fontwork3D']//wps:wsp/wps:bodyPr/"_ostr;
    assertXPath(pXmlDoc, sBody + "a:scene3d/a:camera", "prst", u"legacyObliqueBottomRight");
    assertXPath(pXmlDoc, sBody + "a:scene3d/a:camera/a:rot", "lat", u"20400000");
    assertXPath(pXmlDoc, sBody + "a:scene3d/a:camera/a:rot", "lon", u"19800000");
    assertXPath(pXmlDoc, sBody + "a:scene3d/a:lightRig", "dir", u"tr");
    assertXPath(pXmlDoc, sBody + "a:sp3d", "extrusionH", u"360000");
    assertXPath(pXmlDoc, sBody + "a:sp3d", "prstMaterial", u"metal");
    assertXPath(pXmlDoc, sBody + "a:sp3d/a:extrusionClr/a:srgbClr", "val", u"0000FF");
    OString sTurned = "//mc:Choice//wp:anchor[wp:docPr/@name='Rotated3D']//wps:wsp/"_ostr;
    assertXPath(pXmlDoc, sTurned + "wps:spPr/a:xfrm", "rot", u"19800000");
    assertXPath(pXmlDoc, sTurned + "wps:bodyPr/a:scene3d/a:camera", "prst",
                u"legacyObliqueBottomRight");

    // and reading the file back gives the Fontwork its extrusion, and the turned one its turn
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPageSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage = xDoc->getDrawPage();
    uno::Reference<beans::XPropertySet> xFontwork;
    uno::Reference<beans::XPropertySet> xTurned;
    for (sal_Int32 i = 0; i < xPage->getCount(); ++i)
    {
        uno::Reference<container::XNamed> xNamed(xPage->getByIndex(i), uno::UNO_QUERY_THROW);
        if (xNamed->getName() == "Fontwork3D")
            xFontwork.set(xPage->getByIndex(i), uno::UNO_QUERY_THROW);
        else if (xNamed->getName() == "Rotated3D")
            xTurned.set(xPage->getByIndex(i), uno::UNO_QUERY_THROW);
    }
    CPPUNIT_ASSERT(xFontwork.is());
    assertExtrusionOfShape(xFontwork);
    CPPUNIT_ASSERT(xTurned.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3000),
                         xTurned->getPropertyValue(u"RotateAngle"_ustr).get<sal_Int32>());
    comphelper::SequenceAsHashMap aTurnedGeometry(
        xTurned->getPropertyValue(u"CustomShapeGeometry"_ustr));
    comphelper::SequenceAsHashMap aTurnedExtrusion(aTurnedGeometry[u"Extrusion"_ustr]);
    CPPUNIT_ASSERT_EQUAL(true, aTurnedExtrusion[u"Extrusion"_ustr].get<bool>());
    comphelper::SequenceAsHashMap aGeometry(
        xFontwork->getPropertyValue(u"CustomShapeGeometry"_ustr));
    comphelper::SequenceAsHashMap aTextPath(aGeometry[u"TextPath"_ustr]);
    CPPUNIT_ASSERT_EQUAL(true, aTextPath[u"TextPath"_ustr].get<bool>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0000FF),
                         xFontwork->getPropertyValue(u"FillColor2"_ustr).get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkDistance)
{
    // LO ignores 'Spacing to Borders' of the 'Text Attributes' for Fontwork shapes. Word interprets
    // them. Make sure we force them to zero on export to docx, otherwise Word might wrap the text.
    loadFromFile(u"tdf51195_Fontwork_Distance.odt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathAttrs(pXmlDoc,
                     "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                     "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:bodyPr",
                     { { "lIns", u"0" }, { "rIns", u"0" }, { "tIns", u"0" }, { "bIns", u"0" } });
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkLinGradientRGBColor)
{
    // The document has a Fontwork shape with UI settings: linear gradient fill with angle 330deg,
    // start color #ffff00 (Yellow) with 'Intensity' 80%, end color #4682B4 (Steel Blue), Transition
    // Start 25% and solid transparency 30%.
    // Without fix the gradient was not exported at all.
    loadFromFile(u"tdf51195_Fontwork_linearGradient.fodt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/"_ostr;

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    // 330deg gradient rotation = 120deg color transition direction
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:lin", "ang", u"7200000");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:lin", "scaled", u"0");

    // Make sure the color stops have correct position and color
    // The 'intensity' property in the UI has a different algorithm than the 'lumMod' attribute in
    // OOXML. Therefore it cannot be exported as 'lumMod' but need to be incorporated into the color.
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", u"0");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:srgbClr", "val", u"cccc00");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:srgbClr/w14:alpha", "val", u"30000");

    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", u"25000");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:srgbClr", "val", u"cccc00");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:srgbClr/w14:alpha", "val", u"30000");

    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", u"100000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr", "val", u"4682b4");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr/w14:alpha", "val", u"30000");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkAxialGradientTransparency)
{
    // The document has a Fontwork shape with UI settings: solid fill theme color Accent3 25% darker,
    // Transparency gradient Type Axial with Angle 160deg, Transition start 40%,
    // Start value 5%, End value 90%
    // Without fix the gradient was not exported at all.
    loadFromFile(u"tdf51195_Fontwork_axialGradient.fodt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/"_ostr;

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    // 160deg gradient rotation = 290deg (360deg-160deg+90deg) color transition direction
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:lin", "ang", u"17400000");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:lin", "scaled", u"0");

    // Make sure the color stops have correct position and color
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    // gradient is in transparency, color is always the same.
    for (char ch = '1'; ch <= '3'; ++ch)
    {
        assertXPath(pXmlDoc, sElement + "w14:gs[" + OStringChar(ch) + "]/w14:schemeClr", "val",
                    u"accent3");
        assertXPath(pXmlDoc, sElement + "w14:gs[" + OStringChar(ch) + "]/w14:schemeClr/w14:lumMod",
                    "val", u"75000");
    }
    // transparency values are not exactly like in UI because converting through rgb-color.
    // border 40% in UI means 20% on each side.
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", u"20000");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:schemeClr/w14:alpha", "val", u"89800");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", u"50000");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:schemeClr/w14:alpha", "val", u"4710");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", u"80000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:schemeClr/w14:alpha", "val", u"89800");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkRadialGradient)
{
    // The document has a Fontwork shape with UI settings: gradient fill, Type radial,
    // From Color #40E0D0, To Color #FF0000, Center x|y 75%|20%, no transparency
    // Transition start 10%
    // Without fix the gradient was not exported at all.
    loadFromFile(u"tdf51195_Fontwork_radialGradient.fodt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/"_ostr;

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path", "path", u"circle");
    assertXPathAttrs(
        pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path/w14:fillToRect",
        { { "l", u"75000" }, { "t", u"20000" }, { "r", u"25000" }, { "b", u"80000" } });

    // Make sure the color stops have correct position and color
    // The first stop is duplicated to force Word to render the gradient same as LO.
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", u"0");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:srgbClr", "val", u"ff0000");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", u"0");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:srgbClr", "val", u"ff0000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", u"90000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr", "val", u"40e0d0");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkEllipticalGradient)
{
    // The document has a Fontwork shape with UI settings: solid fill, Color #00008B (deep blue),
    // transparency gradient type Ellipsoid, Center x|y 50%|50%, Transition Start 50%,
    // Start 70%, End 0%.
    // Without fix the gradient was not exported at all.
    loadFromFile(u"tdf51195_Fontwork_ellipticalGradient.fodt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/"_ostr;

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path", "path", u"circle");
    assertXPathAttrs(
        pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path/w14:fillToRect",
        { { "l", u"50000" }, { "t", u"50000" }, { "r", u"50000" }, { "b", u"50000" } });

    // Make sure the color stops have correct position and color
    // transparency values are not exactly like in UI because converting through rgb-color.
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", u"0");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:srgbClr", "val", u"00008b");
    // stop is duplicated to force Word to same rendering as LO does.
    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", u"0");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:srgbClr", "val", u"00008b");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", u"50000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr", "val", u"00008b");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr/w14:alpha", "val", u"69800");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkSquareGradient)
{
    // The document has a Fontwork shape with UI settings: gradient fill Type "Square (Quadratic)",
    // From Color #4963ef 40%, To Color #ffff6e 90%, Center x|y 100%|50%,
    // no transparency
    // Without fix the gradient was not exported at all.
    loadFromFile(u"tdf51195_Fontwork_squareGradient.fodt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/"_ostr;

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path", "path", u"rect");
    assertXPathAttrs(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path/w14:fillToRect",
                     { { "l", u"100000" }, { "t", u"50000" }, { "r", u"0" }, { "b", u"50000" } });

    // Make sure the color stops have correct position and color
    // The 'intensity' property in the UI has a different algorithm than the 'lumMod' attribute in
    // OOXML. Therefore it cannot be exported as 'lumMod' but need to be incorporated into the color.
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", u"0");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:srgbClr", "val", u"e6e663");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", u"0");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:srgbClr", "val", u"e6e663");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", u"100000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr", "val", u"1d4860");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkRectGradient)
{
    // The document has a Fontwork shape with UI settings: solid color theme Accent 4 60% lighter,
    // transparency gradient Type "Rectangular", Center x|y 50%|50%, Transition start 10%,
    // Start value 70%, End value 5%.
    // Without fix the gradient was not exported at all.
    loadFromFile(u"tdf51195_Fontwork_rectGradient.fodt");

    // Save to DOCX:
    save(TestFilter::DOCX);

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/"_ostr;

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path", "path", u"rect");
    assertXPathAttrs(
        pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path/w14:fillToRect",
        { { "l", u"50000" }, { "t", u"50000" }, { "r", u"50000" }, { "b", u"50000" } });

    // Make sure the color stops have correct position and color
    // transparency values are not exactly like in UI because converting through rgb-color.
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", u"0");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:schemeClr", "val", u"accent4");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:schemeClr/w14:lumMod", "val", u"40000");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:schemeClr/w14:lumOff", "val", u"60000");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:schemeClr/w14:alpha", "val", u"4710");
    // The first stop is duplicated to force Word to render the gradient same as LO.
    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", u"0");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:schemeClr", "val", u"accent4");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:schemeClr/w14:lumMod", "val", u"40000");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:schemeClr/w14:lumOff", "val", u"60000");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:schemeClr/w14:alpha", "val", u"4710");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", u"90000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:schemeClr", "val", u"accent4");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:schemeClr/w14:lumMod", "val", u"40000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:schemeClr/w14:lumOff", "val", u"60000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:schemeClr/w14:alpha", "val", u"69800");
}

CPPUNIT_TEST_FIXTURE(Test, testThemeColorTransparency)
{
    // The document has first a Fontwork shape with solid fill theme color with transparency and
    // outline transparency and second a textbox with character transparency.
    // Without fix the transparency was not written to file.
    loadFromFile(u"tdf139618_ThemeColorTransparency.pptx");

    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // Make sure a:alpha is written for line color and for fill color.
    // Make sure fill color is a schemeClr.
    OString sElement = "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p/a:r/a:rPr/"_ostr;
    assertXPath(pXmlDoc, sElement + "a:ln/a:solidFill/a:srgbClr/a:alpha", "val", u"25000");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr", "val", u"accent1");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr/a:lumMod", "val", u"60000");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr/a:lumOff", "val", u"40000");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr/a:alpha", "val", u"35000");

    // Make sure a:alpha is written for characters and fill color is a schemeClr.
    sElement = "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:r/a:rPr/"_ostr;
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr", "val", u"accent4");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr/a:lumMod", "val", u"75000");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr/a:alpha", "val", u"20000");
}

CPPUNIT_TEST_FIXTURE(Test, testThemeFontTypeface)
{
    // Saving the document had produced a file which PowerPoint wants to repair. The mandatory
    // attribute 'typeface' was missing in the <a:ea> elements.

    loadFromFile(u"tdf155412_typeface.pptx");

    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/theme/theme1.xml"_ustr);

    // Make sure typeface is written.
    OString sElement = "/a:theme/a:themeElements/a:fontScheme/"_ostr;
    assertXPath(pXmlDoc, sElement + "a:majorFont/a:ea", "typeface", u"");
    assertXPath(pXmlDoc, sElement + "a:minorFont/a:ea", "typeface", u"");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf157289CircularArrowExport)
{
    // The document has a custom shape of type "circular-arrow". Such uses a B command where
    // the ellipse bounding box is defined by bottom-right vertex first and then top-left vertex.
    // When saving to PPTX this had resulted in negative radii for the ellipse.
    loadFromFile(u"tdf157289_circularArrow_export.fodp");
    save(TestFilter::PPTX);

    // Verify the markup. The arc now takes its radii from guides built from the bounding box
    // of the B command; they are the half of the absolute width, so positive either way, and
    // the adjustment that sizes them is declared.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    OUString aWR = getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo[1]", "wR");
    CPPUNIT_ASSERT(aWR.startsWith("f"));
    CPPUNIT_ASSERT_EQUAL(aWR, getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo[1]", "hR"));
    OUString aHalf
        = getXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='" + aWR.toUtf8() + "']", "fmla");
    CPPUNIT_ASSERT(aHalf.startsWith("*/ "));
    CPPUNIT_ASSERT(aHalf.endsWith(" 1 2"));
    OUString aAbs = aHalf.copy(3, aHalf.getLength() - 7);
    OUString aAbsFormula
        = getXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='" + aAbs.toUtf8() + "']", "fmla");
    CPPUNIT_ASSERT(aAbsFormula.startsWith("abs "));
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", 3);

    // The two angle adjustments are stored in 60000ths of a degree although one of them is
    // also an equation of its own ($0) and the other appears in a sum (($1+45)*pi/180): the
    // -45 degree start stays signed, and both polar handles are written
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd[@name='adj1']", "fmla", u"val -2700000");
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd[@name='adj2']", "fmla", u"val 3600000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", 2);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar[1]", "gdRefAng", u"adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar[2]", "gdRefAng", u"adj2");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar[2]", "gdRefR", u"adj3");
    // the sum ($1+45) in 60000ths: the bare adjustment plus 45 degrees
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='+- adj2 2700000 0']", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134401_ExportAutoGrowToTextWordWrap)
{
    // pptx doesn't have autoGrowWidth and autoGrowHeight, but it does have TextWordWrap
    // which is similar. If autoGrowWidth and autoGrowHeight are set in the document,
    // then they are exported to pptx as TextWordWrap = "none".
    loadFromFile(u"tdf134401_ExportAutoGrowToTextWordWrap.odp");
    save(TestFilter::PPTX);

    // There are 2 shapes in the test file.
    // The 1. shape is without autoGrowWidth/Height.
    // The 2. shape is with autoGrowWidth/Height.
    // Check if wrap="none" is exported for shape 2, but no wrap is exported for shape 1.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPathNoAttribute(pXmlDoc, "//p:sp[1]/p:txBody/a:bodyPr", "wrap");
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:bodyPr", "wrap", u"none");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf163803_ImageFill)
{
    loadFromFile(u"tdf163803_image_with_fill.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Check that the fill color is saved:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '//p:pic/p:spPr/a:solidFill' number of nodes is incorrect
    // i.e. the <a:solidFill> element was not written.
    assertXPath(pXmlDoc, "//p:pic/p:spPr/a:solidFill");
    assertXPath(pXmlDoc, "//p:pic/p:spPr/a:solidFill/a:srgbClr", "val", u"000000");
}

CPPUNIT_TEST_FIXTURE(Test, testPPTXExportOutlinerListStyle)
{
    // Given a slide with an outliner shape and a matching master slide with its own outliner shape:
    loadFromFile(u"outliner-list-style.odp");

    // When saving to PPTX:
    save(TestFilter::PPTX);

    // Then make sure that the list style of the outliner shape on the master page is written:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:lstStyle/a:lvl1pPr", 1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//p:sp[2]/p:txBody/a:lstStyle/a:lvl2pPr' number of nodes is incorrect
    // i.e. only the first list level was written, the UI couldn't format a new 2nd level paragraph
    // correctly.
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:lstStyle/a:lvl2pPr", 1);
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:lstStyle/a:lvl3pPr", 1);
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:lstStyle/a:lvl4pPr", 1);
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:lstStyle/a:lvl5pPr", 1);
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:lstStyle/a:lvl6pPr", 1);
    assertXPath(pXmlDoc, "//p:sp[2]/p:txBody/a:lstStyle/a:lvl7pPr", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testFullySeeThroughFillKeepsItsColour)
{
    // Given a document whose footer holds a shape with a white fill that nothing shows of:
    loadFromFile(u"tdf81100.docx");

    // When saving to DOCX:
    save(TestFilter::DOCX);

    // Then make sure the colour is still there, with nothing of it showing:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/footer2.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//wps:spPr/a:solidFill/a:srgbClr' number of nodes is incorrect
    // i.e. the fill was written as no fill at all, which threw the colour away, so a reader that
    // made the fill solid again arrived at a colour the document never held.
    assertXPath(pXmlDoc, "//wps:spPr/a:solidFill/a:srgbClr", "val", u"FFFFFF");
    assertXPath(pXmlDoc, "//wps:spPr/a:solidFill/a:srgbClr/a:alpha", "val", u"0");
}

CPPUNIT_TEST_FIXTURE(Test, testUnderlineFillThemeColor)
{
    loadFromFile(u"underline-fill-colors.pptx");

    save(TestFilter::PPTX);

    // Make sure a theme color is written as a scheme color, with its transformation
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    auto getRunProperties = [](int nParagraph) -> OString {
        return "//p:sp[1]/p:txBody/a:p[" + OString::number(nParagraph) + "]/a:r/a:rPr";
    };
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//p:sp[1]/p:txBody/a:p[2]/a:r/a:rPr/a:uFill/a:solidFill/a:schemeClr'
    //   number of nodes is incorrect
    assertXPath(pXmlDoc, getRunProperties(2) + "/a:uFill/a:solidFill/a:schemeClr", "val",
                u"accent2");
    assertXPath(pXmlDoc, getRunProperties(3) + "/a:uFill/a:solidFill/a:schemeClr", "val",
                u"accent1");
    assertXPath(pXmlDoc, getRunProperties(3) + "/a:uFill/a:solidFill/a:schemeClr/a:lumMod", "val",
                u"75000");

    // The other color types have no theme color, so they become their RGB value
    assertXPath(pXmlDoc, getRunProperties(1) + "/a:uFill/a:solidFill/a:srgbClr", "val", u"FF0000");
    assertXPath(pXmlDoc, getRunProperties(4) + "/a:uFill/a:solidFill/a:srgbClr", "val", u"008000");
    assertXPath(pXmlDoc, getRunProperties(5) + "/a:uFill/a:solidFill/a:srgbClr", "val", u"00FFFF");
    assertXPath(pXmlDoc, getRunProperties(6) + "/a:uFill/a:solidFill/a:srgbClr", "val", u"0000FF");
}
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeEquationGuides)
{
    // Given a shape drawn by two equations of one adjustment value:
    loadFromFile(u"custGeom_xyHandle.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the equations become guides, the adjustment is declared as the fraction of the width
    // the handle moves it along (5400 of 21600 = 25000 of 100000), and the path points name the
    // guides instead of baking their current value in
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "name", u"adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 25000");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f0']", "fmla", u"*/ adj1 21600 100000");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f1']", "fmla", u"+- 21600 0 f0");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f2']", "fmla", u"*/ f0 1 2");
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path", "w", u"21600");
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path", "h", u"21600");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo/a:pt", "x", u"f0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo/a:pt", "y", u"0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[1]/a:pt", "x", u"21600");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[1]/a:pt", "y", u"f2");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[2]/a:pt", "x", u"f1");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[2]/a:pt", "y", u"21600");
    // and the handle moves the adjustment along x within its range, at the shape's position
    // of the handle, given through the w and h of the shape
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefX", u"adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "minX", u"0");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "maxX", u"50000");
    assertXPathNoAttribute(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefY");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY/a:pos", "x", u"f3");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY/a:pos", "y", u"f4");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f3']", "fmla", u"*/ f0 w 21600");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f4']", "fmla", u"*/ 0 h 21600");

    // and reading the file back gives a shape that still has its adjustment value and equations
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    comphelper::SequenceAsHashMap aGeometry(xShape->getPropertyValue(u"CustomShapeGeometry"_ustr));
    uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustments;
    aGeometry[u"AdjustmentValues"_ustr] >>= aAdjustments;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aAdjustments.getLength());
    sal_Int32 nValue = 0;
    aAdjustments[0].Value >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(25000), nValue);
    uno::Sequence<beans::PropertyValues> aHandles;
    aGeometry[u"Handles"_ustr] >>= aHandles;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aHandles.getLength());
    uno::Sequence<OUString> aEquations;
    aGeometry[u"Equations"_ustr] >>= aEquations;
    CPPUNIT_ASSERT(aEquations.getLength() >= 2);
    // and the path points are bound to the adjustment and the equations again, not numbers
    comphelper::SequenceAsHashMap aPath(aGeometry[u"Path"_ustr]);
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
    aPath[u"Coordinates"_ustr] >>= aCoordinates;
    CPPUNIT_ASSERT(aCoordinates.getLength() >= 3);
    CPPUNIT_ASSERT_EQUAL(drawing::EnhancedCustomShapeParameterType::EQUATION,
                         aCoordinates[0].First.Type);
    CPPUNIT_ASSERT_EQUAL(drawing::EnhancedCustomShapeParameterType::EQUATION,
                         aCoordinates[1].Second.Type);
    CPPUNIT_ASSERT_EQUAL(drawing::EnhancedCustomShapeParameterType::EQUATION,
                         aCoordinates[2].First.Type);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeEquationGuidesStretch)
{
    // Given the same shape with a stretch point, whose path the export has to scale itself:
    loadFromFile(u"custGeom_stretch.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the path is in the extent of the shape: x through the shorter side (ss), y through
    // h, the far edge of x is w, and the adjustment with its handle is kept
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPathNoAttribute(pXmlDoc, "//a:pathLst/a:path", "w");
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 25000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    OUString aX = getXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo/a:pt", "x");
    OUString aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aX.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.endsWith(" ss 21600"));
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[1]/a:pt", "x", u"w");
    OUString aY = getXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[1]/a:pt", "y");
    aFormula = getXPath(pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aY.toUtf8() + "']"),
                        "fmla");
    CPPUNIT_ASSERT(aFormula.endsWith(" h 21600"));
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeEquationGuidesConstants)
{
    // Given equations with fractional constants and a constant trigonometric factor:
    loadFromFile(u"custGeom_constants.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then a fraction becomes a "*/" with whole numbers, and a constant angle is written in
    // 60000ths of a degree
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 33333");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f0']", "fmla", u"*/ adj1 21600 100000");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f1']", "fmla", u"*/ f0 5 10");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f2']", "fmla", u"*/ f0 10 36");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f3']", "fmla", u"+- 21600 0 f1");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f4']", "fmla", u"+- f1 0 10800");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f5']", "fmla", u"cos f4 18900000");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f6']", "fmla", u"+- f5 10800 0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo/a:pt", "x", u"f1");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[1]/a:pt", "y", u"f2");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[2]/a:pt", "x", u"f3");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[3]/a:pt", "y", u"f6");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "maxX", u"100000");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeEquationGuidesTrig)
{
    // Given a clock hand whose angle is an adjustment in degrees, used through the pi/180 idiom:
    loadFromFile(u"custGeom_trig.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the angle is stored in 60000ths of a degree, as the polar handle writes it back, the
    // sine and cosine take it directly, and the handle sits at the end of the hand
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 2700000");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f0']", "fmla", u"cos 9000 adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f1']", "fmla", u"+- 10800 f0 0");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f2']", "fmla", u"sin 9000 adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f3']", "fmla", u"+- 10800 f2 0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:lnTo/a:pt", "x", u"f1");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:lnTo/a:pt", "y", u"f3");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "gdRefAng", u"adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "minAng", u"0");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "maxAng", u"21599999");
    assertXPathNoAttribute(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "gdRefR");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f4']", "fmla", u"cos 10800 adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f5']", "fmla", u"sin 10800 adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f6']", "fmla", u"+- 10800 f4 0");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f7']", "fmla", u"+- 10800 f5 0");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f8']", "fmla", u"*/ f6 w 21600");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f9']", "fmla", u"*/ f7 h 21600");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar/a:pos", "x", u"f8");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar/a:pos", "y", u"f9");

    // and reading it back gives a handle that moves the angle again
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    comphelper::SequenceAsHashMap aGeometry(xShape->getPropertyValue(u"CustomShapeGeometry"_ustr));
    uno::Sequence<beans::PropertyValues> aHandles;
    aGeometry[u"Handles"_ustr] >>= aHandles;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aHandles.getLength());
    comphelper::SequenceAsHashMap aHandle(aHandles[0]);
    sal_Int32 nRefAngle = -1;
    aHandle[u"RefAngle"_ustr] >>= nRefAngle;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRefAngle);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapePolarHandleWithRadius)
{
    // Given a polar handle that moves both the angle and the radius, with a radius range:
    loadFromFile(u"custGeom_polarRadius.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the angle is in 60000ths, the radius a fraction of the smaller side, the range in
    // the units of the radius, and the position is centre + radius * (cos, sin)
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd[@name='adj1']", "fmla", u"val 1800000");
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd[@name='adj2']", "fmla", u"val 33333");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "gdRefR", u"adj2");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "minR", u"0");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "maxR", u"50000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "gdRefAng", u"adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f0']", "fmla", u"*/ adj2 21600 100000");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f1']", "fmla", u"cos f0 adj1");
    // the position reuses the guides of the path: centre + radius * cos is the x of the hand
    OUString aPosX = getXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar/a:pos", "x");
    CPPUNIT_ASSERT(aPosX.startsWith("f"));
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='" + aPosX.toUtf8() + "']", "fmla",
                u"*/ f2 w 21600");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f2']", "fmla", u"+- 10800 f1 0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:lnTo/a:pt", "x", u"f2");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeSwitchedHandle)
{
    // Given a shape taller than wide whose switched handle therefore moves along y:
    loadFromFile(u"custGeom_switchedHandle.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the adjustment is a fraction of the height, the handle binds it to y with the x range
    // turned into the y range, and the position is the swapped one
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 25000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    assertXPathNoAttribute(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefX");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefY", u"adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "minY", u"0");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "maxY", u"50000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY/a:pos", "x", u"f2");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY/a:pos", "y", u"f3");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f2']", "fmla", u"*/ 0 w 21600");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f3']", "fmla", u"*/ f0 h 21600");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeArcHandleFollowsEllipse)
{
    // Given a shape whose adjustment sizes an ellipse drawn by the U command:
    loadFromFile(u"custGeom_arcHandle.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the ellipse takes its radii from the adjustment through guides, its start point is
    // the point of the ellipse at the start angle, a full turn is the swing, and the handle
    // that sizes it is written
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 23148");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f0']", "fmla", u"*/ adj1 21600 100000");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f1']", "fmla", u"*/ f0 1 2");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "wR", u"f0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "hR", u"f1");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "stAng", u"0");
    OUString aSwing = getXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "swAng");
    CPPUNIT_ASSERT(aSwing.startsWith("f"));
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f4']", "fmla", u"cat2 f0 f3 f2");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f5']", "fmla", u"sat2 f1 f3 f2");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:moveTo/a:pt", "x", u"f6");
    assertXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:moveTo/a:pt", "y", u"f7");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefX", u"adj1");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeArcByAngles)
{
    // Given a pie drawn by the G command from a polar handle's radius and angle:
    loadFromFile(u"custGeom_arcAngleTo.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the arc takes its radius from the radius adjustment, its start angle is the angle
    // adjustment itself (both are stored in the units arcTo wants), the swing is the constant
    // in 60000ths, and the polar handle is written with its radius range
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd[@name='adj1']", "fmla", u"val 3600000");
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd[@name='adj2']", "fmla", u"val 33333");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo/a:pt", "x", u"f2");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo/a:pt", "y", u"f4");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "wR", u"f0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "hR", u"f0");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "stAng", u"adj1");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "swAng", u"5400000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "gdRefR", u"adj2");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "minR", u"4630");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "maxR", u"50000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "gdRefAng", u"adj1");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeArcByPoints)
{
    // Given a wedge drawn by the B command whose end points follow an xy handle:
    loadFromFile(u"custGeom_arcByPoints.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the radii and the centre come from the bounding box, the angles from at2 of the
    // rays through the two points, the swing is turned counter-clockwise, and the handle stays
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "wR", u"f4");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "hR", u"f4");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f4']", "fmla", u"*/ f3 1 2");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f5']", "fmla", u"+/ 0 21600 2");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "stAng", u"f8");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f8']", "fmla", u"at2 f6 f7");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f11']", "fmla", u"at2 f9 f10");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "swAng", u"f20");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f18']", "fmla", u"+- f11 0 f8");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f19']", "fmla", u"+- f18 0 21600000");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f20']", "fmla", u"?: f18 f19 f18");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo[2]/a:pt", "x", u"f16");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo[2]/a:pt", "y", u"f17");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeFreeHandleIsUnlimited)
{
    // Given a handle that ODF does not limit on either axis:
    loadFromFile(u"custGeom_freeHandle.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the handle moves both adjustments and its ranges are the extreme values the presets
    // use for an unlimited handle, not an invented box
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd[1]", "fmla", u"val 25000");
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd[2]", "fmla", u"val 12500");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefX", u"adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "minX", u"-2147483647");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "maxX", u"2147483647");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefY", u"adj2");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "minY", u"-2147483647");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "maxY", u"2147483647");

    // and reading it back leaves the handle free to move
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    comphelper::SequenceAsHashMap aGeometry(xShape->getPropertyValue(u"CustomShapeGeometry"_ustr));
    uno::Sequence<beans::PropertyValues> aHandles;
    aGeometry[u"Handles"_ustr] >>= aHandles;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aHandles.getLength());
    comphelper::SequenceAsHashMap aHandle(aHandles[0]);
    drawing::EnhancedCustomShapeParameter aRange;
    aHandle[u"RangeXMinimum"_ustr] >>= aRange;
    sal_Int32 nValue = 0;
    aRange.Value >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-2147483647), nValue);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeGluePointsFollowAdjustments)
{
    // Given a parametric shape whose glue points are placed by an equation, by the adjustment
    // itself and by numbers:
    loadFromFile(u"custGeom_gluePoints.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the connection sites are guides built from the same tokens as the path, converted
    // to the shape's extent, so a connector attached in Office follows the adjustment
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:cxnLst/a:cxn", 3);
    assertXPath(pXmlDoc, "//a:custGeom/a:cxnLst/a:cxn[1]/a:pos", "x", u"GluePoint1X");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='GluePoint1X']", "fmla",
                u"*/ f1 w 21600");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='GluePoint1Y']", "fmla",
                u"*/ 10800 h 21600");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='GluePoint2X']", "fmla",
                u"*/ f0 w 21600");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='GluePoint3Y']", "fmla",
                u"*/ 21600 h 21600");
    // the handle is still there
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    // the equation nothing refers to, third after the guide of the adjustment, is not written
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f2']", "fmla", u"*/ f0 1 2");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f3']", 0);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd", 11);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeWithoutViewBox)
{
    // Given a parametric shape whose geometry has no svg:viewBox, which the engine renders in
    // 0 0 21600 21600, with a path that stays above 14400 in y:
    loadFromFile(u"custGeom_noViewBox.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the path takes the size the shape is rendered in, not the extent of its points, and
    // the shape keeps its adjustment and handle
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path", "w", u"21600");
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path", "h", u"21600");
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 25000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[2]/a:pt", "y", u"14400");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeSinusWave)
{
    // Given a sinus wave drawn from equations in radians, with pi, trigonometric functions of
    // computed angles and an angle step used as a plain value, and two handles:
    loadFromFile(u"custGeom_sinusWave.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the shape keeps its four adjustments and both handles, the angles are converted to
    // 60000ths of a degree for sin and cos
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", 4);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 2);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY[1]", "gdRefX", u"adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY[1]", "minX", u"-100000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY[1]", "maxX", u"200000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY[2]", "gdRefX", u"adj3");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[starts-with(@fmla, 'sin 1 ')]", 13);
    // pi as the closest fraction, every angle in radians scaled by 10800000/pi
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ 103993 1 33102']", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[contains(@fmla, ' 34377468 10')]", 13);
    // the first point is on the curve: y = 10800*(1-sin(...)) in path space, a guide
    OUString aY = getXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo/a:pt", "y");
    CPPUNIT_ASSERT(aY.startsWith("f"));

    // and reading it back gives the shape its handles again
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    comphelper::SequenceAsHashMap aGeometry(xShape->getPropertyValue(u"CustomShapeGeometry"_ustr));
    uno::Sequence<beans::PropertyValues> aHandles;
    aGeometry[u"Handles"_ustr] >>= aHandles;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aHandles.getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeHandleRangeOfWidth)
{
    // Given a shape whose handle ranges are given by the box (width) and by an equation:
    loadFromFile(u"custGeom_rangeWidth.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the ranges are written in adjustment units, the box as the full range
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefX", u"adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "minX", u"48000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "maxX", u"100000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefY", u"adj2");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "minY", u"48000");
    OUString aMaxY = getXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "maxY");
    CPPUNIT_ASSERT(aMaxY.startsWith("f"));
    // the equation (0.5*height = 50) scaled into the units of the adjustment
    OUString aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aMaxY.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.startsWith("*/ f"));
    CPPUNIT_ASSERT(aFormula.endsWith(" 100000 100"));
}

CPPUNIT_TEST_FIXTURE(Test, testRoundRectangleTallerThanWide)
{
    // Given a round-rectangle that is taller than wide, whose switched handle has an x range
    // only:
    loadFromFile(u"custGeom_roundRectTall.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then it is the roundRect preset with its corner as the adjustment
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:prstGeom", "prst", u"roundRect");
    assertXPath(pXmlDoc, "//a:prstGeom/a:avLst/a:gd", "fmla", u"val 16667");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeForwardReference)
{
    // Given a brace whose equations refer to a later one (?f2 = ?f4 - $0, with f4 = $1):
    loadFromFile(u"custGeom_forwardReference.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the later equation is emitted first, so that every guide is defined before its use,
    // and both handles are written
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", 2);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 2);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY[1]", "gdRefY", u"adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY[2]", "gdRefY", u"adj2");
    // the guide of f2 names the guide of f4, which precedes it
    OUString aF2 = getXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[1]/a:pt", "y");
    OUString aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aF2.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.startsWith("+- "));
    OUString aF4 = aFormula.getToken(1, ' ');
    CPPUNIT_ASSERT(aF4.startsWith("f"));
    sal_Int32 nF2
        = getXPathContent(pXmlDoc, OString("count(//a:custGeom/a:gdLst/a:gd[@name='" + aF2.toUtf8()
                                           + "']/preceding-sibling::a:gd)"))
              .toInt32();
    sal_Int32 nF4
        = getXPathContent(pXmlDoc, OString("count(//a:custGeom/a:gdLst/a:gd[@name='" + aF4.toUtf8()
                                           + "']/preceding-sibling::a:gd)"))
              .toInt32();
    CPPUNIT_ASSERT(nF4 < nF2);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeQuadrants)
{
    // Given a rounded rectangle drawn with elliptical quadrants (X and Y commands) whose
    // corners an adjustment sizes:
    loadFromFile(u"custGeom_quadrant.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then each quadrant is an arc whose radii are guides built from the current point and
    // the target, so the handle is not frozen
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", 4);
    OUString aWR = getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo[1]", "wR");
    CPPUNIT_ASSERT(aWR.startsWith("f"));
    OUString aRadius = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aWR.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aRadius.startsWith("abs f"));
    // the radius is the distance from the current point, which the adjustment places
    OUString aDistance = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aRadius.copy(4).toUtf8() + "']"),
        "fmla");
    CPPUNIT_ASSERT(aDistance.startsWith("+- "));
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ adj1 21600 100000']", 1);
    // the direction follows the two points: the first quadrant starts along x at 270 degrees
    // (the target is below the current point) and turns counter-clockwise
    OUString aStart = getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo[1]", "stAng");
    OUString aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aStart.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.startsWith("?: f"));
    CPPUNIT_ASSERT(aFormula.endsWith(" 5400000 16200000"));
    OUString aSwing = getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo[1]", "swAng");
    aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aSwing.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.endsWith(" 5400000 -5400000"));
    // its condition is the product of the two differences
    OUString aProduct = getXPath(
        pXmlDoc,
        OString("//a:custGeom/a:gdLst/a:gd[@name='" + aFormula.getToken(1, ' ').toUtf8() + "']"),
        "fmla");
    CPPUNIT_ASSERT(aProduct.startsWith("*/ f"));
    CPPUNIT_ASSERT(aProduct.endsWith(" 1"));
    // the second quadrant (a Y command) starts along y: at 0 degrees when the target is to the
    // left, else at 180, and turns clockwise when the product is negative
    aStart = getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo[2]", "stAng");
    aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aStart.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.endsWith(" 0 10800000"));
    aSwing = getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo[2]", "swAng");
    aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aSwing.toUtf8() + "']"), "fmla");
    OUString aNegated = getXPath(
        pXmlDoc,
        OString("//a:custGeom/a:gdLst/a:gd[@name='" + aFormula.getToken(1, ' ').toUtf8() + "']"),
        "fmla");
    CPPUNIT_ASSERT(aNegated.endsWith(" -1 1"));
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 16667");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefX", u"adj1");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeStretched)
{
    // Given a rounded rectangle with stretch points on both axes, twice as wide as tall:
    loadFromFile(u"custGeom_stretched.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the path is in the extent of the shape, scaled by the shorter side on both axes, the
    // far edges are w and h, right in the equations is w brought back into the view box, and
    // the corner handle is kept
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPathNoAttribute(pXmlDoc, "//a:pathLst/a:path", "w");
    assertXPathNoAttribute(pXmlDoc, "//a:pathLst/a:path", "h");
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 16667");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ w 21600 ss']", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ h 21600 ss']", 1);
    // right-$0 and bottom-$0 are two guides, each from its own edge
    OUString aRight = getXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ w 21600 ss']", "name");
    OUString aBottom
        = getXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ h 21600 ss']", "name");
    OUString aCorner
        = getXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ adj1 21600 100000']", "name");
    assertXPath(pXmlDoc,
                OString("//a:custGeom/a:gdLst/a:gd[@fmla='+- " + aRight.toUtf8() + " 0 "
                        + aCorner.toUtf8() + "']"),
                1);
    assertXPath(pXmlDoc,
                OString("//a:custGeom/a:gdLst/a:gd[@fmla='+- " + aBottom.toUtf8() + " 0 "
                        + aCorner.toUtf8() + "']"),
                1);
    OUString aX = getXPath(pXmlDoc, "//a:pathLst/a:path/a:moveTo/a:pt", "x");
    OUString aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aX.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.endsWith(" ss 21600"));
    // "L 21600 ?f8" is the third lnTo of the path
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[3]/a:pt", "x", u"w");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", 4);
    OUString aWR = getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo[1]", "wR");
    aFormula = getXPath(pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aWR.toUtf8() + "']"),
                        "fmla");
    CPPUNIT_ASSERT(aFormula.endsWith(" ss 21600"));
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeStretchedArc)
{
    // Given a shape with stretch points, twice as tall as wide, with an arc by angles (a G
    // command) whose radii an adjustment sets, and a glue point on the far edge:
    loadFromFile(u"custGeom_stretchedArc.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the radii of the arc are in the extent of the shape, its angles are angles of the
    // extent, the handle is kept, and the glue point on the far edge is w
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPathNoAttribute(pXmlDoc, "//a:pathLst/a:path", "h");
    OUString aWR = getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "wR");
    OUString aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aWR.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.endsWith(" ss 21600"));
    OUString aHR = getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "hR");
    aFormula = getXPath(pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aHR.toUtf8() + "']"),
                        "fmla");
    CPPUNIT_ASSERT(aFormula.endsWith(" ss 21600"));
    // the angles are transformed with the scales of the two axes, both ss here
    OUString aStart = getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "stAng");
    aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aStart.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.startsWith("at2 f"));
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='cos ss 10800000']", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='sin ss 10800000']", 1);
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "swAng").startsWith("f"));
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefY", u"adj1");
    OUString aRight = getXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ w 21600 ss']", "name");
    OUString aBottom
        = getXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ h 21600 ss']", "name");
    OUString aGlueX = "*/ " + aRight + " ss 21600";
    OUString aGlueY = "*/ " + aBottom + " ss 21600";
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='GluePoint1X']", "fmla", aGlueX);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='GluePoint2Y']", "fmla", aGlueY);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeStretchedTallArcs)
{
    // Given a shape stretched on x only and twice as tall as wide, so that x is scaled by w and
    // y by h, with an arc by angles from 30 to 150 degrees and an arc through two points:
    loadFromFile(u"custGeom_stretchedTallArcs.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the angle of the arc by angles is transformed with the two scales (at2 of its cosine
    // by the x scale and its sine by the y scale), and the arc through two points takes its
    // angles from the points in the extent
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    OUString aStart = getXPath(pXmlDoc, "//a:pathLst/a:path[2]/a:arcTo", "stAng");
    OUString aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aStart.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.startsWith("at2 f"));
    // the legs of the angle: its cosine by the extent of x (ss, the view box being square), its
    // sine by the extent of y (h)
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='cos ss 1800000']", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='sin h 1800000']", 1);
    OUString aWR = getXPath(pXmlDoc, "//a:pathLst/a:path[3]/a:arcTo", "wR");
    aFormula = getXPath(pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aWR.toUtf8() + "']"),
                        "fmla");
    CPPUNIT_ASSERT(aFormula.startsWith("*/ f"));
    OUString aStart3 = getXPath(pXmlDoc, "//a:pathLst/a:path[3]/a:arcTo", "stAng");
    aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aStart3.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.startsWith("at2 f"));
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeStretchedWideBox)
{
    // Given a shape with a view box twice as wide as tall, stretched on x only, with an arc by
    // angles and a full ellipse:
    loadFromFile(u"custGeom_stretchedWideBox.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the legs of the angle transform are the extent of y and the extent of x brought to
    // the view box of y (ss by 10800 over 21600), and the full ellipse keeps its turn
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ ss 10800 21600']", 1);
    OUString aLegX
        = getXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ ss 10800 21600']", "name");
    OUString aCos = "cos " + aLegX + " 1800000";
    assertXPath(pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@fmla='" + aCos.toUtf8() + "']"), 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='sin h 1800000']", 1);
    OUString aSwing = getXPath(pXmlDoc, "//a:pathLst/a:path[3]/a:arcTo", "swAng");
    OUString aFormula = getXPath(
        pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aSwing.toUtf8() + "']"), "fmla");
    CPPUNIT_ASSERT(aFormula.startsWith("?: f"));
    CPPUNIT_ASSERT(aFormula.endsWith(" 21600000"));
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeAtanAndLogwidth)
{
    // Given a shape whose equations use atan2, atan, logwidth and logheight:
    loadFromFile(u"custGeom_atanLogwidth.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the angles come from at2 scaled to radians, the sizes from w and h in 1/100 mm, and
    // the handle is kept
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    OUString aScaled
        = getXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ adj1 21600 100000']", "name");
    assertXPath(pXmlDoc,
                OString("//a:custGeom/a:gdLst/a:gd[@fmla='at2 10800 " + aScaled.toUtf8() + "']"),
                1);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[starts-with(@fmla, 'at2 1 ')]", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[contains(@fmla, ' 10 34377468')]", 2);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ w 1 360']", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ h 1 360']", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeLogwidthInWriter)
{
    // Given a text document whose shape uses logwidth, which Writer measures in twips:
    loadFromFile(u"custGeom_logwidthWriter.fodt");

    // When saving it to DOCX:
    save(TestFilter::DOCX);

    // Then logwidth is w in twips, 635 EMU each
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ w 1 635']", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@fmla='*/ w 1 360']", 0);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeBrokenEquation)
{
    // Given a shape with an equation that does not parse (*sqrt(2)/2, as some files have it),
    // which the engine evaluates as 0, used as a coordinate:
    loadFromFile(u"custGeom_brokenEquation.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then that coordinate is 0 and the rest of the shape keeps its guides and handle
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[3]/a:pt", "x", u"0");
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 25000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeFromOOXMLKeepsOnlyUsedGuides)
{
    // Given a shape that came from a pptx whose points are numbers and whose guides serve a
    // connection site, with one guide nothing refers to:
    loadFromFile(u"custGeom_ooxmlConnectionSites.pptx");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the connection site repeats its guides and nothing else is written
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:cxnLst/a:cxn", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:cxnLst/a:cxn/a:pos", "x", u"GluePoint1X");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='GluePoint1X']", "fmla",
                u"*/ 21600 w 43200");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='GluePoint1Y']", "fmla",
                u"*/ 0 h 43200");
    // the text rectangle's guides stay, no equation guide is written
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[starts-with(@name, 'f')]", 0);
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[starts-with(@name, 'GluePoint')]", 2);
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt", "x", u"21600");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeAngleUsedAsCoordinateKeepsHandle)
{
    // Given a clock hand whose angle adjustment also places a mark as a plain coordinate:
    loadFromFile(u"custGeom_angleAsCoordinate.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the adjustment is stored in 60000ths of a degree for its polar handle, and the
    // mark gets the degrees back through a guide
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 2700000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "gdRefAng", u"adj1");
    OUString aMark = getXPath(pXmlDoc, "//a:pathLst/a:path[3]/a:moveTo/a:pt", "x");
    assertXPath(pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aMark.toUtf8() + "']"),
                "fmla", u"*/ adj1 180 10800000");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeFromOOXMLKeepsHandle)
{
    // Given a shape that came from a pptx with its own geometry, an adjustment in Office's
    // units, guides over w and a handle bound to the adjustment:
    loadFromFile(u"custGeom_ooxmlHandle.pptx");

    // When saving it to PPTX again:
    save(TestFilter::PPTX);

    // Then the adjustment value is written as it came, its guides reach the file with the
    // shape's size standing in for logwidth, the path point names its guide, and the handle
    // names the adjustment at the position the engine has for it
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 20139");
    // pin 0 adj 100000, as the importer spells it: if(0-$0, 0, if(100000-$0, $0, 100000))
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f0']", "fmla", u"+- 0 0 adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f1']", "fmla", u"+- 100000 0 adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f2']", "fmla", u"?: f1 adj1 100000");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f3']", "fmla", u"?: f0 0 f2");
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path", "w", u"43200");
    OUString aCorner = getXPath(pXmlDoc, "//a:pathLst/a:path/a:lnTo[1]/a:pt", "x");
    CPPUNIT_ASSERT(aCorner.startsWith("f"));
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "gdRefX", u"adj1");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "maxX", u"100000");
    // the file had no minX, and gets none back
    assertXPathNoAttribute(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", "minX");
    // the position is the guide the file had (*/ w a 100000), with w the built-in again, and
    // the corner is that position brought into the path's space (*/ xPos 43200 w)
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f4']", "fmla", u"*/ w f3 100000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY/a:pos", "x", u"f4");
    assertXPath(pXmlDoc, "//a:custGeom/a:gdLst/a:gd[@name='f5']", "fmla", u"*/ f4 43200 w");
    CPPUNIT_ASSERT_EQUAL(u"f5"_ustr, aCorner);

    // and reading it back gives the handle bound to the adjustment again
    loadFromURL(maTempFile.GetURL());
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    comphelper::SequenceAsHashMap aGeometry(xShape->getPropertyValue(u"CustomShapeGeometry"_ustr));
    uno::Sequence<beans::PropertyValues> aHandles;
    aGeometry[u"Handles"_ustr] >>= aHandles;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aHandles.getLength());
    comphelper::SequenceAsHashMap aHandle(aHandles[0]);
    sal_Int32 nRefX = -1;
    aHandle[u"RefX"_ustr] >>= nRefX;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRefX);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeFromOOXMLDraggedHandle)
{
    // Given a shape that came from a pptx whose handle was moved in the engine, which leaves a
    // fraction of Office's units in the adjustment value:
    loadFromFile(u"custGeom_ooxmlHandle.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    comphelper::SequenceAsHashMap aGeometry(xShape->getPropertyValue(u"CustomShapeGeometry"_ustr));
    uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustments;
    aGeometry[u"AdjustmentValues"_ustr] >>= aAdjustments;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aAdjustments.getLength());
    aAdjustments.getArray()[0].Value <<= 30000.4;
    aGeometry[u"AdjustmentValues"_ustr] <<= aAdjustments;
    xShape->setPropertyValue(u"CustomShapeGeometry"_ustr,
                             uno::Any(aGeometry.getAsConstPropertyValueList()));

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the value is rounded to Office's whole units and the handle is kept
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 30000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahXY", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeAliasUsedAsCoordinateKeepsHandle)
{
    // Given a clock hand whose angle adjustment has an alias equation (?f0 = $0) that places a
    // mark as a plain coordinate:
    loadFromFile(u"custGeom_aliasAsCoordinate.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the adjustment is stored in 60000ths of a degree for its polar handle, and the
    // mark gets the degrees back through the alias
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 2700000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "gdRefAng", u"adj1");
    OUString aMark = getXPath(pXmlDoc, "//a:pathLst/a:path[3]/a:moveTo/a:pt", "x");
    assertXPath(pXmlDoc, OString("//a:custGeom/a:gdLst/a:gd[@name='" + aMark.toUtf8() + "']"),
                "fmla", u"*/ adj1 180 10800000");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeArcAngleByAlias)
{
    // Given a pie whose arc takes its start angle from an alias equation of the angle:
    loadFromFile(u"custGeom_arcByAlias.fodp");

    // When saving it to PPTX:
    save(TestFilter::PPTX);

    // Then the alias is the bare adjustment in 60000ths, the arc uses it as it is, and the polar
    // handle is written
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:avLst/a:gd", "fmla", u"val 3600000");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "stAng", u"adj1");
    assertXPath(pXmlDoc, "//a:pathLst/a:path/a:arcTo", "swAng", u"5400000");
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", 1);
    assertXPath(pXmlDoc, "//a:custGeom/a:ahLst/a:ahPolar", "gdRefAng", u"adj1");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
