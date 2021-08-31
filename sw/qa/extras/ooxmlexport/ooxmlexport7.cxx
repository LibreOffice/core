/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <config_features.h>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/processfactory.hxx>

#include <pagedesc.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

    // We import OOXML's EMUs into integral mm100 internal representation, then export back into
    // EMUs. This results in inaccuracies.
    void assertXPathHasApproxEMU(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath,
                                 const OString& rAttribute, sal_Int64 nAttributeVal)
    {
        OUString val = getXPath(pXmlDoc, rXPath, rAttribute);
        // Use precision of 1/2 of 100th of mm, which is 180 EMU
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            OString("XPath: " + rXPath + "\nAttribute: " + rAttribute).getStr(), nAttributeVal,
            val.toInt64(), 180);
    }
};

CPPUNIT_TEST_FIXTURE(Test,  testChildNodesOfCubicBezierTo)
{
    loadAndSave("FDO74774.docx");
    /* Number of children required by cubicBexTo is 3 of type "pt".
       While exporting, sometimes the child nodes are less than 3.
       The test case ensures that there are 3 child nodes of type "pt"
       for cubicBexTo
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath( pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent[1]/mc:Choice/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/wpg:wgp[1]/wps:wsp[3]/wps:spPr[1]/a:custGeom[1]/a:pathLst[1]/a:path[1]/a:cubicBezTo[2]/a:pt[3]");
}

CPPUNIT_TEST_FIXTURE(Test, testMSwordHang)
{
    loadAndSave("test_msword_hang.docx");
    // fdo#74771:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r[2]/w:drawing/wp:inline", "distT", "0");
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeThemeFont, "groupshape-theme-font.docx")
{
    // Font was specified using a theme reference, which wasn't handled.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY_THROW)->getText();
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText),1);
    // This was Calibri.
    CPPUNIT_ASSERT_EQUAL(OUString("Cambria"), getProperty<OUString>(xRun, "CharFontName"));
}

CPPUNIT_TEST_FIXTURE(Test, testAnchorIdForWP14AndW14)
{
    loadAndSave("AnchorId.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/mc:AlternateContent/mc:Choice/w:drawing/wp:inline", "anchorId", "78735EFD");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "anchorId", "78735EFD");
}

DECLARE_OOXMLEXPORT_TEST(testDkVert, "dkvert.docx")
{
    // <a:pattFill prst="dkVert"> was exported as ltVert.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was 50.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(25), getProperty<drawing::Hatch>(xShape, "FillHatch").Distance);
}

CPPUNIT_TEST_FIXTURE(Test, testTextWatermark)
{
    loadAndSave("textWatermark.docx");
    //The problem was that the watermark ID was not preserved,
    //and Word uses the object ID to identify if it is a watermark.
    //It has to have the 'PowerPlusWaterMarkObject' string in it
    xmlDocUniquePtr pXmlHeader2 = parseExport("word/header2.xml");

    assertXPath(pXmlHeader2, "/w:hdr[1]/w:p[1]/w:r[1]/w:pict[1]/v:shape[1]","id","PowerPlusWaterMarkObject93701316");

    //The second problem was that Word uses also "o:spid"
    const OUString& sSpid = getXPath(pXmlHeader2, "/w:hdr[1]/w:p[1]/w:r[1]/w:pict[1]/v:shape[1]","spid");
    CPPUNIT_ASSERT(!sSpid.isEmpty());
}

CPPUNIT_TEST_FIXTURE(Test, testPictureWatermark)
{
    loadAndSave("pictureWatermark.docx");
    //The problem was that the watermark ID was not preserved,
    //and Word uses the object ID to identify if it is a watermark.
    //It has to have the 'WordPictureWaterMarkObject' string in it

    xmlDocUniquePtr pXmlHeader2 = parseExport("word/header2.xml");

    // Check the watermark ID
    assertXPath(pXmlHeader2, "/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Fallback[1]/w:pict[1]/v:shape[1]","id","WordPictureWatermark11962361");
}


CPPUNIT_TEST_FIXTURE(Test, testFdo76249)
{
    loadAndSave("fdo76249.docx");
    /*
     * The Locked Canvas is imported correctly, but while exporting
     * the drawing element is exported inside a textbox. However the drawing has to exported
     * as a Locked Canvas inside a text-box for the RT file to work in MS Word, as drawing elements
     * are not allowed inside the textboxes.
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Fallback[1]/w:pict[1]/v:rect[1]/v:textbox[1]/w:txbxContent[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/lc:lockedCanvas[1]",1);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo76979)
{
    loadAndSave("fdo76979.docx");
    // The problem was that black was exported as "auto" fill color, resulting in well-formed, but invalid XML.
    xmlDocUniquePtr pXmlDoc = parseExport("word/header2.xml");
    // This was "auto", not "FFFFFF".
    assertXPath(pXmlDoc, "//wps:spPr/a:solidFill/a:srgbClr", "val", "FFFFFF");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104539)
{
    loadAndSave("tdf104539.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "dir", "13500000");
}

DECLARE_OOXMLEXPORT_TEST(testTdf57155, "tdf57155.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    // Without the fix in place, the image in the header of page 2 wouldn't exist
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

DECLARE_OOXMLEXPORT_TEST(testTdf129582, "tdf129582.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    // Without the fix in place, the image in the footer of page 2 wouldn't exist
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(Test, testShapeEffectPreservation)
{
    loadAndSave("shape-effect-preservation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // first shape with outer shadow, rgb color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "algn", "tl");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "blurRad", "50760"); // because convertEMUtoHmm rounds fractions into nearest integer 50800 will be 50760
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "dir", "2700000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "dist", "37674");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "rotWithShape", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:srgbClr",
            "val", "000000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:srgbClr/a:alpha",
            "val", "40000");

    // second shape with outer shadow, scheme color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "algn", "tl");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "blurRad", "114480"); // because convertEMUtoHmm rounds fractions into nearest integer 114300 will be 114480
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "dir", "2700000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "dist", "203137");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
            "rotWithShape", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr",
            "val", "accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr/a:lumMod",
            "val", "40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr/a:lumOff",
            "val", "60000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr/a:alpha",
            "val", "40000");

    // third shape with inner shadow, rgb color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
            "blurRad", "63500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
            "dir", "16200000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
            "dist", "50800");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr",
            "val", "ffff00");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr/a:alpha",
            "val", "50000");

    // 4th shape with soft edge
    assertXPathHasApproxEMU(
        pXmlDoc,
        "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:softEdge",
        "rad", 127000); // actually, it returns 127080
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:softEdge/*",
            0 ); // should not be present

    // 5th shape with glow effect, scheme color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:srgbClr",
            "val", "eb2722");

    // 6th shape with reflection
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "blurRad", "6350");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "stA", "50000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "endA", "300");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "endPos", "55500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection/*",
            0 ); // should not be present

    // 7th shape with several effects: glow, inner shadow and reflection
    assertXPathHasApproxEMU(pXmlDoc,
                            "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow",
                            "rad", 63500); // actually, it returns 63360
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:srgbClr",
            "val", "eb2722");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
            "blurRad", "63500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw",
            "dir", "2700000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr",
            "val", "000000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr/a:alpha",
            "val", "50000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "blurRad", "6350");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection",
            "stA", "52000");

}

CPPUNIT_TEST_FIXTURE(Test, testShape3DEffectPreservation)
{
    loadAndSave("shape-3d-effect-preservation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // first shape: extrusion and shift on z, rotated camera with zoom, rotated light rig
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera",
            "prst", "perspectiveRelaxedModerately");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera",
            "zoom", "150000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
            "lat", "19490639");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
            "lon", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
            "rev", "12900001");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
            "rig", "threePt");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
            "dir", "t");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
            "lat", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
            "lon", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
            "rev", "4800000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "extrusionH", "63500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "z", "488950");

    // second shape: extrusion with theme color, no camera or light rotation, metal material
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera",
            "prst", "isometricLeftDown");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot",
            0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
            "rig", "threePt");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig",
            "dir", "t");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot",
            0);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "extrusionH", "25400");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "prstMaterial", "metal");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/a:schemeClr",
            "val", "accent5");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/a:schemeClr/a:lumMod",
            "val", "40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/a:schemeClr/a:lumOff",
            "val", "60000");

    // third shape: colored contour and top and bottom bevel, plastic material
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "contourW", "50800");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "prstMaterial", "plastic");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT",
            "w", "139700");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT",
            "h", "88900");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT",
            "prst", "cross");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelB",
            "h", "88900");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelB",
            "prst", "relaxedInset");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:contourClr/a:srgbClr",
            "val", "3333ff");

    // fourth shape: wireframe
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d",
            "prstMaterial", "legacyWireframe");
}

CPPUNIT_TEST_FIXTURE(Test, testPictureEffectPreservation)
{
    loadAndSave("picture-effects-preservation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // first picture: glow effect with theme color and transformations, 3d rotation and extrusion
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow",
            "rad", "228600");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow/a:srgbClr",
            "val", "267de6");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow/a:srgbClr/a:alpha",
            "val", "40000");


    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:scene3d/a:camera",
            "prst", "isometricRightUp");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:scene3d/a:lightRig",
            "rig", "threePt");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:sp3d",
            "extrusionH", "76200");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:sp3d/a:extrusionClr/a:srgbClr",
            "val", "92d050");

    // second picture: shadow and reflection effects
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw",
            "dir", "8100000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw/a:srgbClr",
            "val", "000000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw/a:srgbClr/a:alpha",
            "val", "40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:reflection",
            "dir", "5400000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:reflection/*",
            0 ); // should not be present

    // third picture: soft edge effect
    assertXPathHasApproxEMU(
        pXmlDoc,
        "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:softEdge",
        "rad", 63500); // actually, it returns 63360
}

CPPUNIT_TEST_FIXTURE(Test, testPictureArtisticEffectPreservation)
{
    loadAndSave("picture-artistic-effects-preservation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    xmlDocUniquePtr pRelsDoc = parseExport("word/_rels/document.xml.rels");

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(
            comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());

    // 1st picture: marker effect
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticMarker",
            "trans", "14000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticMarker",
            "size", "80");

    OUString sEmbedId1 = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
            "embed");
    OUString sXmlPath = "/rels:Relationships/rels:Relationship[@Id='" + sEmbedId1 + "']";
    OUString sFile = getXPath(pRelsDoc, OUStringToOString( sXmlPath, RTL_TEXTENCODING_UTF8 ), "Target");
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/" + sFile)));

    // 2nd picture: pencil grayscale
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilGrayscale",
            "trans", "15000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilGrayscale",
            "pencilSize", "66");

    OUString sEmbedId2 = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
            "embed");
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId2);

    // 3rd picture: pencil sketch
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilSketch",
            "trans", "7000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilSketch",
            "pressure", "17");

    OUString sEmbedId3 = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
            "embed");
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId3);

    // 4th picture: light screen
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticLightScreen",
            "trans", "13000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticLightScreen",
            "gridSize", "1");

    OUString sEmbedId4 = getXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
            "embed");
    sXmlPath = "/rels:Relationships/rels:Relationship[@Id='" + sEmbedId4 + "']";
    sFile = getXPath(pRelsDoc, OUStringToOString( sXmlPath, RTL_TEXTENCODING_UTF8 ), "Target");
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/" + sFile)));

    // 5th picture: watercolor sponge
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticWatercolorSponge",
            "brushSize", "4");

    OUString sEmbedId5 = getXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
            "embed");
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId5);

    // 6th picture: photocopy (no attributes)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPhotocopy", 1);

    OUString sEmbedId6 = getXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer",
            "embed");
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId6);

    // no redundant wdp files saved
    CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName("word/media/hdphoto3.wdp")));
}

CPPUNIT_TEST_FIXTURE(Test, fdo77719)
{
    loadAndSave("fdo77719.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:custGeom[1]", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testNestedAlternateContent)
{
    loadAndSave("nestedAlternateContent.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // We check alternateContent  could not contains alternateContent (i.e. nested alternateContent)
    assertXPath(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wpg:wgp[1]/wps:wsp[2]/wps:txbx[1]/w:txbxContent[1]/w:p[1]/w:r[2]/mc:AlternateContent[1]",0);
}

#if 0
// Currently LibreOffice exports custom geometry for this hexagon, not preset shape.
// When LibreOffice can export preset shapes with correct modifiers, then this test can be re-enabled.

CPPUNIT_TEST_FIXTURE(Test, test76317)
{
    loadAndSave("test76317.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]", "prst", "hexagon");
}

#endif

CPPUNIT_TEST_FIXTURE(Test, fdo76591)
{
    loadAndSave("fdo76591.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[3]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]", "relativeHeight", "3");
}

CPPUNIT_TEST_FIXTURE(Test, test76317_2K10)
{
    loadAndSave("test76317_2K10.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]/a:avLst[1]/a:gd[1]", "name", "adj");
}

CPPUNIT_TEST_FIXTURE(Test, testFDO77122)
{
    loadAndSave("LinkedTextBoxes.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    //ensure that the text box links are preserved.
    assertXPath(pXmlDoc, "//wps:txbx[1]", "id", "1");
    assertXPath(pXmlDoc, "//wps:linkedTxbx[1]", "id", "1");
}

CPPUNIT_TEST_FIXTURE(Test, test76734_2K7)
{
    loadAndSave("test76734_2K7.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[3]/mc:AlternateContent[1]/mc:Choice[1]", "Requires", "wps");
}

CPPUNIT_TEST_FIXTURE(Test, test77219)
{
    loadAndSave("test77219.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[6]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]", "behindDoc", "1");
}

DECLARE_OOXMLEXPORT_TEST(testTdf77219_backgroundShape, "tdf77219_backgroundShape.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape is in front of the paragraph", false, getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf77219_foregroundShape, "tdf77219_foregroundShape.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape is in front of the paragraph", true, getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf108973_backgroundTextbox, "tdf108973_backgroundTextbox.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox is in front of the paragraph", false, getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf108973_foregroundTextbox, "tdf108973_foregroundTextbox.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox is in front of the paragraph", true, getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testPresetShape, "preset-shape.docx")
{
    // Document contains a flowChartMultidocument preset shape, our date for that shape wasn't correct.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aCustomShapeGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    comphelper::SequenceAsHashMap aPath(aCustomShapeGeometry["Path"]);
    uno::Sequence<awt::Size>      aSubViewSize((aPath["SubViewSize"]).get<uno::Sequence<awt::Size> >() );

    // This was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21600), aSubViewSize[0].Height);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo76101)
{
    loadAndSave("fdo76101.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/w:styles/w:style");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT(4091 >= xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtAndShapeOverlapping)
{
    loadAndSave("ShapeOverlappingWithSdt.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/mc:AlternateContent");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt[1]/w:sdtContent[1]/w:r[1]/w:t[1]");
}

CPPUNIT_TEST_FIXTURE(Test, testLockedCanvas)
{
    loadAndSave("fdo78658.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Checking for lockedCanvas tag
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/lc:lockedCanvas", 1);
}

CPPUNIT_TEST_FIXTURE(Test, fdo78474)
{
    loadAndSave("fdo78474.docx");
    xmlDocUniquePtr pXmlDoc1 = parseExport("word/document.xml");
    //docx file after RT is getting corrupted.
    assertXPath(pXmlDoc1, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:txbx[1]/w:txbxContent[1]/w:p[1]/w:r[1]/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/pic:pic[1]/pic:blipFill[1]/a:blip[1]", "embed", "rId2");

    xmlDocUniquePtr pXmlDoc2 = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDoc2,"/rels:Relationships/rels:Relationship[2]","Id","rId2");
}

CPPUNIT_TEST_FIXTURE(Test, testAbsolutePositionOffsetValue)
{
    loadAndSave("fdo78432.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    xmlXPathObjectPtr pXmlObjs[6];
    pXmlObjs[0] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]");
    pXmlObjs[1] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]");

    pXmlObjs[2] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[2]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]");
    pXmlObjs[3] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[2]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]");

    pXmlObjs[4] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[3]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]");
    pXmlObjs[5] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[3]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]");

    for(sal_Int32 index = 0; index<6; ++index)
    {
        CPPUNIT_ASSERT(pXmlObjs[index]->nodesetval != nullptr);
        xmlNodePtr pXmlNode = pXmlObjs[index]->nodesetval->nodeTab[0];
        OUString contents = OUString::createFromAscii(reinterpret_cast<const char*>((pXmlNode->children[0]).content));
        CPPUNIT_ASSERT( contents.toInt64() <= SAL_MAX_INT32 );
        xmlXPathFreeObject(pXmlObjs[index]);
    }
}

DECLARE_OOXMLEXPORT_TEST(testRubyHyperlink, "rubyhyperlink.fodt")
{
    // test that export doesn't assert with overlapping ruby / hyperlink attr
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78300)
{
    loadAndSave("fdo78300.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing[1]/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[1]/w:r[1]/w:drawing[1]",
                0);
}

CPPUNIT_TEST_FIXTURE(Test, testWordArtWithinDraingtool)
{
    loadAndSave("testWordArtWithinDraingtool.docx");
/*   * Within a file, there is a 2007 wordArt enclosed in a drawing tool
     * LO was exporting it as below:
     * Sample XML as in Original file:
     * <p> <r> <ac> <drawing> <txbx> <txbxContent> ..  <pict> </pict> </txbxContent></txbx> </drawing> </ac> </r> </p>
     *  After RT :
     * <p> <r> <ac> <drawing> <txbx> <txbxContent> ..  <drawing> <txbx> <txbxContent> ..  </txbxContent></txbx> </drawing> .. </txbxContent></txbx> </drawing> </ac> </r> </p>
     *  Expected : as there is nesting of a 2007 Word Art within a drawing tool, then can be separated in two different runs.
     * */

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent",1);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:p/w:r/w:pict/v:shape",1);
    // Make sure that the shape inside a shape is exported as VML-only, no embedded mc:AlternateContent before w:pict.
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict",1);
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78663)
{
    loadAndSave("fdo78663.docx");
/*     * A 2007 word art tool is enclosed in a 2010 drawing toolWithin a file,
     * Originally the file has the following xml tag hierarchy.
     *
     * <p> <r> <ac> <drawing> <txbx> <txbxContent> <pict><shapetype> <shape> ...</shape></shapetype> </pict> </txbxContent></txbx> </drawing> </ac> </r> </p>
     *  After RT :
     * <p> <r> <ac> <drawing> <txbx> <txbxContent> <pict><shapetype> <shape> <textbox><txbxContent> ... </txbxContent></textbox></shape></shapetype> </pict> </txbxContent></txbx> </drawing> </ac> </r> </p>
     * MSO doesn't allow nesting of txbxContent tags.
     * As the text of the wordart tool is written in the tag <v:textpath string="Welcome to... "History is fun and informative"/>
     * We shouldn't repeat it again in <shapetype><shape> <textbox><txbxContent>
     * */

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict/v:shape/v:path",1);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:p/w:r/w:pict/v:shape/v:path",1);
    // Make sure that the shape inside a shape is exported as VML-only, no embedded mc:AlternateContent before w:pict.
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict",1);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo78957)
{
    loadAndSave("fdo78957.docx");
    xmlDocUniquePtr pXmlHeader = parseExport("word/header2.xml");

    const sal_Int64 IntMax = SAL_MAX_INT32;
    sal_Int64 cx = 0, cy = 0;
    cx = getXPath(pXmlHeader,"/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:extent[1]","cx").toInt64();
    cy = getXPath(pXmlHeader,"/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:extent[1]","cy").toInt64();
    //  Here we check the values of extent width & height
    CPPUNIT_ASSERT(cx <= IntMax );
    CPPUNIT_ASSERT(cy >= 0 );
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79256)
{
    loadAndSave("fdo79256.docx");
    /* corruption issue also solved by fixing tdf#108064:
     * since that LO keeps MSO preset dash styles during OOXML export
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln/a:prstDash", "val", "lgDash");
}

CPPUNIT_TEST_FIXTURE(Test, testDashedLine_CustDash1000thOfPercent)
{
    loadAndSave("dashed_line_custdash_1000th_of_percent.docx");
    /* Make sure that preset line is exported correctly as "1000th of a percent".
     * This test-file has a CUSTOM dash-line that is defined as '1000th of a percent'.
     * This should be imported by LO as-is, and exported back with the same values.
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]", "d" , "800000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]", "sp", "300000");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]", "d" , "100000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]", "sp", "300000");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]", "d" , "100000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]", "sp", "300000");
}

CPPUNIT_TEST_FIXTURE(Test, testDashedLine_CustDashPercentage)
{
    loadAndSave("dashed_line_custdash_percentage.docx");
    /* Make sure that preset line is exported correctly as "1000th of a percent".
     * This test-file has a CUSTOM dash-line that is defined as percentages.
     * This should be imported by LO as '1000th of a percent', and exported back
     * as '1000th of a percent'.
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]", "d" , "800000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]", "sp", "300000");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]", "d" , "100000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]", "sp", "300000");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]", "d" , "100000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]", "sp", "300000");
}

CPPUNIT_TEST_FIXTURE(Test, testCommentInitials)
{
    loadAndSave("comment_initials.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/comments.xml");

    assertXPath(pXmlDoc,"/w:comments/w:comment[1]", "initials", "initials");
}

DECLARE_OOXMLEXPORT_TEST(testTextboxRoundedCorners, "textbox-rounded-corners.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    comphelper::SequenceAsHashMap aCustomShapeGeometry(getProperty< uno::Sequence<beans::PropertyValue> >(xShape, "CustomShapeGeometry"));

    // Test that the shape is a rounded rectangle.
    CPPUNIT_ASSERT_EQUAL(OUString("ooxml-roundRect"), aCustomShapeGeometry["Type"].get<OUString>());

    // The shape text should start with a table, with "a" in its A1 cell.
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1, xText), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xCell->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79591)
{
    loadAndSave("fdo79591.docx");
    /* Values set for docPr name and shape ID attributes
     * in RT file were not valid as per UTF-8 encoding format
     * and hence was showing RT document as corrupt with error
     * message "invalid character"
     */
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:docPr", "name", "_x0000_t0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:shape", "ID", "_x0000_t0");
}

DECLARE_OOXMLEXPORT_TEST(testBnc884615, "bnc884615.docx")
{
    // The problem was that the shape in the header wasn't in the background.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(1), "Opaque"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo80894)
{
    loadAndSave("TextFrameRotation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Rotation value was not roundtripped for textframe.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:xfrm",
    "rot","16200000");

    // w:enforcement defaults to off if not explicitly specified, so DocProtect forms should not be enabled.
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No protected sections", sal_Int32(0), xSections->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testfdo80895)
{
    loadAndSave("fdo80895.docx");
    // DML shapes in header and footer were not getting rendered in LO and the same were not preserved after RT.
    // In actual there was a shape but because of fetching wrong theme for header.xml or footer.xml
    // resultant shape was with <a:noFill/> prop in <wps:spPr> hence was not visible.
    // Checking there is a shape in header without <a:noFill/> element.

    xmlDocUniquePtr pXmlDoc = parseExport("word/header2.xml");
    assertXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:noFill",0);
    assertXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:noFill",0);

    // Check for fallback (required for MSO-2007)
    assertXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect", "fillcolor", "#4f81bd");
    assertXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:fill", "type", "solid");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf118242)
{
    loadAndSave("tdf118242.odt");
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
    "/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[2]/w:pPr/w:pStyle", "val", "HeaderRight");

    // w:sectPr is not exported
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
    "/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[2]/w:pPr/w:sectPr", 0);

    // and drawing is no longer in the document
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r[3]", 0);

    //but it's in the header
    pXmlDocument = parseExport("word/header1.xml");

    assertXPath(pXmlDocument, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData"
            "/wps:wsp/wps:txbx/w:txbxContent/w:p[1]/w:r/w:drawing", 1);

}

CPPUNIT_TEST_FIXTURE(Test, testWrapTightThrough)
{
    loadAndSave("wrap-tight-through.docx");
    // These were wrapSquare without a wrap polygon before.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // The first shape should be wrapThrough with a wrap polygon (was wrapSquare).
    assertXPath(pXmlDoc, "//w:drawing/wp:anchor[1]/wp:wrapThrough/wp:wrapPolygon/wp:start", "x", "-1104");
    // The second shape should be wrapTight with a wrap polygon (was wrapSquare).
    assertXPath(pXmlDoc, "//w:drawing/wp:anchor[1]/wp:wrapTight/wp:wrapPolygon/wp:start", "y", "792");
}

DECLARE_OOXMLEXPORT_TEST(testPictureWrapPolygon, "picture-wrap-polygon.docx")
{
    // The problem was that the wrap polygon was ignored during export.
    drawing::PointSequenceSequence aSeqSeq = getProperty<drawing::PointSequenceSequence>(getShape(1), "ContourPolyPolygon");
    // This was 0: the polygon list was empty.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aSeqSeq.getLength());

    drawing::PointSequence aSeq = aSeqSeq[0];
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aSeq.getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testPictureColormodeGrayscale)
{
    loadAndSave("picture_colormode_grayscale.docx");
    // The problem was that the grayscale was not exported
    xmlDocUniquePtr pXmlDoc = parseExport ("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:grayscl", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testPictureColormodeBlackWhite)
{
    loadAndSave("picture_colormode_black_white.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport ("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:biLevel", "thresh", "50000");
}

CPPUNIT_TEST_FIXTURE(Test, testPictureColormodeWatermark)
{
    loadAndSave("picture_colormode_watermark.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport ("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:lum", "bright", "70000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:lum", "contrast", "-70000");
}


CPPUNIT_TEST_FIXTURE(Test, testExportShadow)
{
    loadAndSave("bnc637947.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The problem was that shadows of shapes from non-OOXML origin were not exported to DrawingML
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw", "dist" , "109865");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw", "dir" , "634411");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw/a:srgbClr", "val" , "000000");
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw/a:srgbClr/a:alpha", "val" , "38000");
}

CPPUNIT_TEST_FIXTURE(Test, testExportAdjustmentValue)
{
    loadAndSave("tdf91429.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd", "fmla", "val 50000");
}

DECLARE_OOXMLEXPORT_TEST(testTextVerticalAdjustment, "tdf36117_verticalAdjustment.docx")
{
    //Preserve the page vertical alignment setting for .docx
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    SwPageDesc* pDesc = &pDoc->GetPageDesc( 0 );
    drawing::TextVerticalAdjust nVA = pDesc->GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_CENTER, nVA );

    pDesc = &pDoc->GetPageDesc( 1 );
    nVA = pDesc->GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_TOP, nVA );

    pDesc = &pDoc->GetPageDesc( 2 );
    nVA = pDesc->GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_BOTTOM, nVA );

    pDesc = &pDoc->GetPageDesc( 3 );
    nVA = pDesc->GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_BLOCK, nVA );
}

#if HAVE_MORE_FONTS
DECLARE_OOXMLEXPORT_TEST(testTDF87348, "tdf87348_linkedTextboxes.docx")
{
    int followCount=0;
    int precedeCount=0;
    if( !parseDump("/root/page/body/txt/anchored/fly[1]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[1]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[2]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[2]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[3]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[3]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[4]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[4]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[5]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[5]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[6]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[6]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[7]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[7]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[8]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[8]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[9]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[9]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[10]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[10]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[11]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[11]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[12]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[12]/txt","precede").isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[13]/txt","follow").isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[13]/txt","precede").isEmpty() )
        precedeCount++;
    //there should be 4 chains/13 linked textboxes (set of 5, set of 3, set of 3, set of 2)
    //that means 9 NEXT links and 9 PREV links.
    //however, the current implementation adds leftover shapes, so can't go on exact numbers
    //  (unknown number of flys, unknown order of leftovers)
    CPPUNIT_ASSERT ( (followCount >= 6) );
    CPPUNIT_ASSERT ( (precedeCount >= 6) );
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testTDF93675)
{
    loadAndSave("no-numlevel-but-indented.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "//w:ind", "start", "1418");
}



CPPUNIT_TEST_FIXTURE(Test, testFlipAndRotateCustomShape)
{
    loadAndSave("flip_and_rotate.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // there should be no flipH and flipV attributes in this case
    assertXPathNoAttribute(pXmlDoc, "//a:xfrm", "flipH");
    assertXPathNoAttribute(pXmlDoc, "//a:xfrm", "flipV");
    // check rotation angle
    assertXPath(pXmlDoc, "//a:xfrm", "rot", "13500000");
    // check the first few coordinates of the polygon
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        2351, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt", "x").toInt32(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        3171, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt", "y").toInt32(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1695, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[2]/a:pt", "x").toInt32(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        3171, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[2]/a:pt", "y").toInt32(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1695, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[3]/a:pt", "x").toInt32(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1701, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[3]/a:pt", "y").toInt32(), 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf92335)
{
    loadAndSave("tdf92335.docx");
    // Don't export redundant ListLabel character styles
    xmlDocUniquePtr pXmlStyles = parseExport("word/styles.xml");

    assertXPath(pXmlStyles, "//w:style[@w:styleId='ListLabel1']", 0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
