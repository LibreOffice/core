/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers ooox/source/export/ fixes.
class Test : public UnoApiXmlTest
{
public:
    Test()
        : UnoApiXmlTest("/oox/qa/unit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testPolylineConnectorPosition)
{
    // Given a document with a group shape and therein a polyline and a connector.
    loadFromURL(u"tdf141786_PolylineConnectorInGroup.odt");
    // When saving that to DOCX:
    save("Office Open XML Text");

    // Then make sure polyline and connector have the correct position.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

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
    loadFromURL(u"tdf141786_RotatedShapeInGroup.odt");

    // FIXME: validation error in OOXML export: Errors: 3
    skipValidation();

    // When saving that to DOCX:
    save("Office Open XML Text");

    // Then make sure the rotated child shape has the correct position.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

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
    loadFromURL(u"dml-groupshape-polygon.docx");

    // FIXME: validation error in OOXML export: Errors: 9
    skipValidation();

    // When saving that to DOCX:
    save("Office Open XML Text");

    // Then make sure that the group shape, the group shape's child size and the child shape's size
    // match:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:ext", "cx", "5328360");
    // Without the accompanying fix in place, this test would have failed, the <a:chExt> element was
    // not written.
    assertXPath(pXmlDoc, "//wpg:grpSpPr/a:xfrm/a:chExt", "cx", "5328360");
    assertXPath(pXmlDoc, "//wps:spPr/a:xfrm/a:ext", "cx", "5328360");
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapeArrowExport)
{
    // Given a document with a few different kinds of arrow shapes in it:
    loadFromURL(u"tdf142602_CustomShapeArrows.odt");

    // FIXME: validation error in OOXML export: Errors: 11
    skipValidation();

    // When saving that to DOCX:
    save("Office Open XML Text");

    // Then the shapes should retain their correct control values.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

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
    loadFromURL(u"camera-rotation-revolution-nonwps.pptx");

    // When saving that document:
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
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
    loadFromURL(u"refer-to-theme.pptx");

    // When saving that document:
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
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

CPPUNIT_TEST_FIXTURE(Test, testThemeColor_ShapeFill)
{
    // Given an ODP file that contains references to a theme for shape fill:
    loadFromURL(u"ReferenceShapeFill.fodp");

    // When saving that document:
    save("Impress Office Open XML");

    // Then make sure the shape fill color is a scheme color.
    // Note that this was already working from PPTX files via grab-bags,
    //so this test intentionally uses an ODP file as input.
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:sp[1]/p:spPr/a:solidFill/a:schemeClr", "val", "accent6");
    assertXPath(pXmlDoc, "//p:sp[1]/p:spPr/a:solidFill/a:schemeClr/a:lumMod", "val", "75000");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146690_endParagraphRunPropertiesNewLinesTextSize)
{
    // Given a PPTX file that contains references to a theme:
    loadFromURL(u"endParaRPr-newline-textsize.pptx");

    // When saving that document:
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
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
    loadFromURL(u"tdf147978_endsubpath.odp");

    // When saving that document:
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
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
    loadFromURL(u"tdf147978_enhancedPath_commandA.odp");

    // When saving that document:
    save("Impress Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
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
    loadFromURL(u"tdf147978_enhancedPath_commandT.odp");

    // Export to pptx had only exported the command M and has used a wrong path size
    save("Impress Office Open XML");

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
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
    loadFromURL(u"tdf147978_enhancedPath_commandXY.odp");

    // Export to pptx had dropped commands X and Y.
    save("Impress Office Open XML");

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
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
    loadFromURL(u"tdf147978_enhancedPath_commandHIJK.odp");

    // Export to pptx had dropped commands X and Y.
    save("Impress Office Open XML");

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
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
    loadFromURL(u"tdf147978_enhancedPath_subpath.pptx");

    // Export to pptx had dropped the subpaths.
    save("Impress Office Open XML");

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
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
    loadFromURL(u"tdf100391_TextAreaRect.odp");
    // When saving to PPTX the textarea rect was set to default instead of using the actual area
    save("Impress Office Open XML");

    // Verify the markup. Without fix the values were l="l", t="t", r="r", b="b"
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
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
    loadFromURL(u"tdf109169_OctagonBevel.odt");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    // Export to docx had not written a:fill or a:stroke attributes at all.
    save("Office Open XML Text");

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
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
    loadFromURL(u"FaultyPathStart.odp");

    save("Impress Office Open XML");

    // Verify the markup:
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
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
    loadFromURL(u"tdf148784_StretchXY.odp");
    save("Impress Office Open XML");

    // Verify the markup.
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");

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
    loadFromURL(u"tdf148784_StretchCommandQ.odp");
    save("Impress Office Open XML");

    // Verify the markup.
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");

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
    loadFromURL(u"tdf148784_StretchCommandVW.odp");
    save("Impress Office Open XML");

    // Verify the markup.
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");

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

CPPUNIT_TEST_FIXTURE(Test, testTdf149551VertPadding)
{
    // The document has shape[1] with attribute vert="vert270" and shape[2] with vert="vert". The text
    // has paddings lIns="720000"=2cm, tIns="360000"=1cm, rIns="0" and bIns="0".
    // After load and save the paddings were rotated and a 90deg text rotation was added.
    loadFromURL(u"tdf149551_vert_and_padding.pptx");
    save("Impress Office Open XML");

    // Verify the markup. The values must be the same as in the original file.
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    for (sal_Int32 i = 1; i <= 2; i++)
    {
        OString sElement = "//p:spTree/p:sp[" + OString::number(i) + "]/p:txBody/a:bodyPr";
        assertXPath(pXmlDoc, sElement, "lIns", "720000");
        assertXPath(pXmlDoc, sElement, "tIns", "360000");
        assertXPath(pXmlDoc, sElement, "rIns", "0");
        assertXPath(pXmlDoc, sElement, "bIns", "0");
        assertXPathNoAttribute(pXmlDoc, sElement, "rot");
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149538upright)
{
    // The document has a shape with attribute upright="1" in the bodyPr element. On import it is
    // emulated by rotating the text area rectangle. On export there should be an upright="1"
    // attribute but no 'rot' attribute. Without the fix the 'rot' attribute with values from
    // the emulation was written out.
    loadFromURL(u"tdf149538_upright.pptx");
    save("Impress Office Open XML");

    // Verify the markup. The values must be the same as in the original file.
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    assertXPath(pXmlDoc, "//p:spTree/p:sp/p:txBody/a:bodyPr", "upright", "1");
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp/p:txBody/a:bodyPr", "rot");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf151008VertAnchor)
{
    loadFromURL(u"tdf151008_eaVertAnchor.pptx");
    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    // The order of the shapes in the file is by name "Right", "Center", "Left", "RightMiddle",
    // "CenterMiddle" and "LeftMiddle". I access the shapes here by index, because the XPath is
    // easier then.
    // As of Sep 2022 LibreOffice does not write the default anchorCtr="0"
    // Right
    assertXPath(pXmlDoc, "//p:spTree/p:sp[1]/p:txBody/a:bodyPr", "anchor", "t");
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp[1]/p:txBody/a:bodyPr", "anchorCtr");
    // Center
    assertXPath(pXmlDoc, "//p:spTree/p:sp[2]/p:txBody/a:bodyPr", "anchor", "ctr");
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp[2]/p:txBody/a:bodyPr", "anchorCtr");
    // Left
    assertXPath(pXmlDoc, "//p:spTree/p:sp[3]/p:txBody/a:bodyPr", "anchor", "b");
    assertXPathNoAttribute(pXmlDoc, "//p:spTree/p:sp[3]/p:txBody/a:bodyPr", "anchorCtr");
    // RightMiddle
    assertXPath(pXmlDoc, "//p:spTree/p:sp[4]/p:txBody/a:bodyPr", "anchor", "t");
    assertXPath(pXmlDoc, "//p:spTree/p:sp[4]/p:txBody/a:bodyPr", "anchorCtr", "1");
    // CenterMiddle
    assertXPath(pXmlDoc, "//p:spTree/p:sp[5]/p:txBody/a:bodyPr", "anchor", "ctr");
    assertXPath(pXmlDoc, "//p:spTree/p:sp[5]/p:txBody/a:bodyPr", "anchorCtr", "1");
    // LeftMiddle
    assertXPath(pXmlDoc, "//p:spTree/p:sp[6]/p:txBody/a:bodyPr", "anchor", "b");
    assertXPath(pXmlDoc, "//p:spTree/p:sp[6]/p:txBody/a:bodyPr", "anchorCtr", "1");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkBitmapFill)
{
    // The document has a Fontwork shape with bitmap fill.
    loadFromURL(u"tdf128568_FontworkBitmapFill.odt");

    // FIXME: validation error in OOXML export: Errors: 1
    // Attribute ID is not allowed in element v:shape
    skipValidation();

    // Saving that to DOCX:
    save("Office Open XML Text");

    // Make sure it is exported to VML and has no txbxContent but a textpath element. Without fix it
    // was exported as DML 'abc transform', but that is not able to use bitmap fill in Word.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
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
    assertXPath(pXmlDoc, "//v:shape/v:fill", "type", "frame");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkFontProperties)
{
    // The document has five Fontwork shapes. They have bitmap fill and thus are exported to VML.
    // They differ in font properties e.g. font weight and character spacing.
    loadFromURL(u"tdf128568_FontworkFontProperties.odt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Make sure the style attribute of the textpath element has the needed items. Without fix only
    // font-family and font-size were written.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
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
    loadFromURL(u"tdf153296_VML_export_SlantUp.odt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup. I could identify the following two problems to hinder Word from
    // accepting the markup. There might exist more problems.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Make sure a <v:path> element exists and has an o:connecttype attribute
    assertXPath(pXmlDoc, "//v:shapetype/v:path", 1);
    assertXPath(pXmlDoc, "//v:shapetype/v:path[@o:connecttype]", 1);

    // Make sure the handle position is written without reference to a formula
    OUString sPosition = getXPath(pXmlDoc, "//v:h", "position");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), sPosition.indexOf("@"));
    CPPUNIT_ASSERT_EQUAL(OUString("topLeft,#0"), sPosition);
}

CPPUNIT_TEST_FIXTURE(Test, testVMLFontworkArchUp)
{
    // The document has a Fontwork shape type 'textArchUp' (172). When exporting to docx, the shape
    // was not exported as VML Fontwork but as a rectangle.
    loadFromURL(u"tdf153296_VML_export_ArchUpCurve.odt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Make sure there is no <v:rect> element
    assertXPath(pXmlDoc, "//v:rect", 0);
    // ..., but a <v:shapetype> element with <v:textpath> subelement
    assertXPath(pXmlDoc, "//v:shapetype/v:textpath", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testVMLAdjustmentExport)
{
    // The document has a Fontwork shape type 'textCirclePour' (150). When exporting to docx, the
    // adjustment values were not exported at all.
    loadFromURL(u"tdf153246_VML_export_Fontwork_Adjustment.odt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

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
    loadFromURL(u"tdf51195_Fontwork_DirectColor.odt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    static constexpr OStringLiteral sElement = "//w:txbxContent/w:p/w:r/w:rPr/";

    // Make sure the fallback in <w:color> has correct value.
    assertXPath(pXmlDoc, sElement + "w:color", "val", "2E8B57");

    // ... and <w14:textOutline> exists and has correct values.
    assertXPath(pXmlDoc, sElement + "w14:textOutline", 1);
    assertXPath(pXmlDoc, sElement + "w14:textOutline/w14:solidFill/w14:srgbClr", "val", "ff7f50");
    assertXPath(pXmlDoc, sElement + "w14:textOutline/w14:solidFill/w14:srgbClr/w14:alpha", "val",
                "20000");
    assertXPath(pXmlDoc, sElement + "w14:textOutline/w14:round", 1);

    // ... and w14:textFill exists and has correct values.
    assertXPath(pXmlDoc, sElement + "w14:textFill", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:srgbClr", "val", "2e8b57");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:srgbClr/w14:alpha", "val",
                "60000");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkThemeColor)
{
    // The document has a Fontwork shape with fill theme color 'lt2' and 50% darker, and a Fontwork
    // shape with theme color 'dk2' and 40% lighter. When exporting to docx, fill color was not
    // exported as theme color at all, and it was replaced with character color.
    loadFromURL(u"tdf51195_Fontwork_ThemeColor.fodt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // shape with 'darker'
    OString sElement = "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/";

    // Make sure the fallback in <w:color> has correct values
    assertXPath(pXmlDoc, sElement + "w:color", "val", "948A54");
    assertXPath(pXmlDoc, sElement + "w:color", "themeColor", "light2");
    assertXPath(pXmlDoc, sElement + "w:color", "themeShade", "80");

    // ... and w14:textFill exists and has correct values.
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:schemeClr", "val", "bg2");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:schemeClr/w14:lumMod", "val",
                "50000");

    // shape with 'lighter'
    sElement = "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/"
               "a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/";

    // Make sure the fallback in <w:color> has correct values
    assertXPath(pXmlDoc, sElement + "w:color", "val", "558ED5");
    assertXPath(pXmlDoc, sElement + "w:color", "themeColor", "dark2");
    assertXPath(pXmlDoc, sElement + "w:color", "themeTint", "99");

    // ... and w14:textFill exists and has correct values.
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:schemeClr", "val", "tx2");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:schemeClr/w14:lumMod", "val",
                "60000");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:solidFill/w14:schemeClr/w14:lumOff", "val",
                "40000");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkDistance)
{
    // LO ignores 'Spacing to Borders' of the 'Text Attributes' for Fontwork shapes. Word interprets
    // them. Make sure we force them to zero on export to docx, otherwise Word might wrap the text.
    loadFromURL(u"tdf51195_Fontwork_Distance.odt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPathAttrs(pXmlDoc,
                     "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                     "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:bodyPr",
                     { { "lIns", "0" }, { "rIns", "0" }, { "tIns", "0" }, { "bIns", "0" } });
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkLinGradientRGBColor)
{
    // The document has a Fontwork shape with UI settings: linear gradient fill with angle 330deg,
    // start color #ffff00 (Yellow) with 'Intensity' 80%, end color #4682B4 (Steel Blue), Transition
    // Start 25% and solid transparency 30%.
    // Without fix the gradient was not exported at all.
    loadFromURL(u"tdf51195_Fontwork_linearGradient.fodt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/";

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    // 330deg gradient rotation = 120deg color transition direction
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:lin", "ang", "7200000");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:lin", "scaled", "0");

    // Make sure the color stops have correct position and color
    // The 'intensity' property in the UI has a different algorithm than the 'lumMod' attribute in
    // OOXML. Therefore it cannot be exported as 'lumMod' but need to be incorporated into the color.
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", "0");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:srgbClr", "val", "cccc00");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:srgbClr/w14:alpha", "val", "30000");

    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", "25000");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:srgbClr", "val", "cccc00");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:srgbClr/w14:alpha", "val", "30000");

    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", "100000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr", "val", "4682b4");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr/w14:alpha", "val", "30000");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkAxialGradientTransparency)
{
    // The document has a Fontwork shape with UI settings: solid fill theme color Accent3 25% darker,
    // Transparency gradient Type Axial with Angle 160deg, Transition start 40%,
    // Start value 5%, End value 90%
    // Without fix the gradient was not exported at all.
    loadFromURL(u"tdf51195_Fontwork_axialGradient.fodt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/";

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    // 160deg gradient rotation = 290deg (360deg-160deg+90deg) color transition direction
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:lin", "ang", "17400000");
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:lin", "scaled", "0");

    // Make sure the color stops have correct position and color
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    // gradient is in transparency, color is always the same.
    for (char ch = '1'; ch <= '3'; ++ch)
    {
        assertXPath(pXmlDoc, sElement + "w14:gs[" + OStringChar(ch) + "]/w14:schemeClr", "val",
                    "accent3");
        assertXPath(pXmlDoc, sElement + "w14:gs[" + OStringChar(ch) + "]/w14:schemeClr/w14:lumMod",
                    "val", "75000");
    }
    // transparency values are not exactly like in UI because converting through rgb-color.
    // border 40% in UI means 20% on each side.
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", "20000");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:schemeClr/w14:alpha", "val", "89800");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", "50000");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:schemeClr/w14:alpha", "val", "4710");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", "80000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:schemeClr/w14:alpha", "val", "89800");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkRadialGradient)
{
    // The document has a Fontwork shape with UI settings: gradient fill, Type radial,
    // From Color #40E0D0, To Color #FF0000, Center x|y 75%|20%, no transparency
    // Transition start 10%
    // Without fix the gradient was not exported at all.
    loadFromURL(u"tdf51195_Fontwork_radialGradient.fodt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/";

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path", "path", "circle");
    assertXPathAttrs(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path/w14:fillToRect",
                     { { "l", "75000" }, { "t", "20000" }, { "r", "25000" }, { "b", "80000" } });

    // Make sure the color stops have correct position and color
    // The first stop is duplicated to force Word to render the gradient same as LO.
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", "0");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", "0");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", "90000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr", "val", "40e0d0");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkEllipticalGradient)
{
    // The document has a Fontwork shape with UI settings: solid fill, Color #00008B (deep blue),
    // transparency gradient type Ellipsoid, Center x|y 50%|50%, Transition Start 50%,
    // Start 70%, End 0%.
    // Without fix the gradient was not exported at all.
    loadFromURL(u"tdf51195_Fontwork_ellipticalGradient.fodt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/";

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path", "path", "circle");
    assertXPathAttrs(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path/w14:fillToRect",
                     { { "l", "50000" }, { "t", "50000" }, { "r", "50000" }, { "b", "50000" } });

    // Make sure the color stops have correct position and color
    // transparency values are not exactly like in UI because converting through rgb-color.
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", "0");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:srgbClr", "val", "00008b");
    // stop is duplicated to force Word to same rendering as LO does.
    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", "0");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:srgbClr", "val", "00008b");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", "50000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr", "val", "00008b");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr/w14:alpha", "val", "69800");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkSquareGradient)
{
    // The document has a Fontwork shape with UI settings: gradient fill Type "Square (Quadratic)",
    // From Color #4963ef 40%, To Color #ffff6e 90%, Center x|y 100%|50%,
    // no transparency
    // Without fix the gradient was not exported at all.
    loadFromURL(u"tdf51195_Fontwork_squareGradient.fodt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/";

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path", "path", "rect");
    assertXPathAttrs(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path/w14:fillToRect",
                     { { "l", "100000" }, { "t", "50000" }, { "r", "0" }, { "b", "50000" } });

    // Make sure the color stops have correct position and color
    // The 'intensity' property in the UI has a different algorithm than the 'lumMod' attribute in
    // OOXML. Therefore it cannot be exported as 'lumMod' but need to be incorporated into the color.
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", "0");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:srgbClr", "val", "e6e663");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", "0");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:srgbClr", "val", "e6e663");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", "100000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:srgbClr", "val", "1d4860");
}

CPPUNIT_TEST_FIXTURE(Test, testFontworkRectGradient)
{
    // The document has a Fontwork shape with UI settings: solid color theme Accent 4 60% lighter,
    // transparency gradient Type "Rectangular", Center x|y 50%|50%, Transition start 10%,
    // Start value 70%, End value 5%.
    // Without fix the gradient was not exported at all.
    loadFromURL(u"tdf51195_Fontwork_rectGradient.fodt");

    // FIXME: tdf#153183 validation error in OOXML export: Errors: 1
    // Attribute 'ID' is not allowed to appear in element 'v:shape'.
    skipValidation();

    // Save to DOCX:
    save("Office Open XML Text");

    // Examine the saved markup.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // path to shape text run properties
    OString sElement = "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                       "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/"
                       "w:rPr/";

    // Make sure w14:textFill and w14:gradFill elements exist with child elements
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst", 1);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:gsLst/w14:gs", 3);
    assertXPath(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path", "path", "rect");
    assertXPathAttrs(pXmlDoc, sElement + "w14:textFill/w14:gradFill/w14:path/w14:fillToRect",
                     { { "l", "50000" }, { "t", "50000" }, { "r", "50000" }, { "b", "50000" } });

    // Make sure the color stops have correct position and color
    // transparency values are not exactly like in UI because converting through rgb-color.
    sElement += "w14:textFill/w14:gradFill/w14:gsLst/";
    assertXPath(pXmlDoc, sElement + "w14:gs[1]", "pos", "0");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:schemeClr", "val", "accent4");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:schemeClr/w14:lumMod", "val", "40000");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:schemeClr/w14:lumOff", "val", "60000");
    assertXPath(pXmlDoc, sElement + "w14:gs[1]/w14:schemeClr/w14:alpha", "val", "4710");
    // The first stop is duplicated to force Word to render the gradient same as LO.
    assertXPath(pXmlDoc, sElement + "w14:gs[2]", "pos", "0");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:schemeClr", "val", "accent4");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:schemeClr/w14:lumMod", "val", "40000");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:schemeClr/w14:lumOff", "val", "60000");
    assertXPath(pXmlDoc, sElement + "w14:gs[2]/w14:schemeClr/w14:alpha", "val", "4710");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]", "pos", "90000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:schemeClr", "val", "accent4");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:schemeClr/w14:lumMod", "val", "40000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:schemeClr/w14:lumOff", "val", "60000");
    assertXPath(pXmlDoc, sElement + "w14:gs[3]/w14:schemeClr/w14:alpha", "val", "69800");
}

CPPUNIT_TEST_FIXTURE(Test, testThemeColorTransparency)
{
    // The document has first a Fontwork shape with solid fill theme color with transparency and
    // outline transparency and second a textbox with character transparency.
    // Without fix the transparency was not written to file.
    loadFromURL(u"tdf139618_ThemeColorTransparency.pptx");

    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");

    // Make sure a:alpha is written for line color and for fill color.
    // Make sure fill color is a schemeClr.
    OString sElement = "/p:sld/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p/a:r/a:rPr/";
    assertXPath(pXmlDoc, sElement + "a:ln/a:solidFill/a:srgbClr/a:alpha", "val", "25000");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr", "val", "accent1");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr/a:lumMod", "val", "60000");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr/a:lumOff", "val", "40000");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr/a:alpha", "val", "35000");

    // Make sure a:alpha is written for characters and fill color is a schemeClr.
    sElement = "/p:sld/p:cSld/p:spTree/p:sp[2]/p:txBody/a:p/a:r/a:rPr/";
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr", "val", "accent4");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr/a:lumMod", "val", "75000");
    assertXPath(pXmlDoc, sElement + "a:solidFill/a:schemeClr/a:alpha", "val", "20000");
}

CPPUNIT_TEST_FIXTURE(Test, testThemeFontTypeface)
{
    // Saving the document had produced a file which PowerPoint wants to repair. The mandatory
    // attribute 'typeface' was missing in the <a:ea> elements.

    loadFromURL(u"tdf155412_typeface.pptx");

    save("Impress Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/theme/theme1.xml");

    // Make sure typeface is written.
    OString sElement = "/a:theme/a:themeElements/a:fontScheme/";
    assertXPath(pXmlDoc, sElement + "a:majorFont/a:ea", "typeface", "");
    assertXPath(pXmlDoc, sElement + "a:minorFont/a:ea", "typeface", "");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
