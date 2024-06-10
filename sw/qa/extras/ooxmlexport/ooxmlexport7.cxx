/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <basegfx/utils/gradienttools.hxx>
#include <config_fonts.h>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/processfactory.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}

protected:
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
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath( pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent[1]/mc:Choice/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/wpg:wgp[1]/wps:wsp[3]/wps:spPr[1]/a:custGeom[1]/a:pathLst[1]/a:path[1]/a:cubicBezTo[2]/a:pt[3]"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testMSwordHang)
{
    loadAndSave("test_msword_hang.docx");
    // fdo#74771:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r[2]/w:drawing/wp:inline"_ostr, "distT"_ostr, u"0"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeThemeFont, "groupshape-theme-font.docx")
{
    // Font was specified using a theme reference, which wasn't handled.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY_THROW)->getText();
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText),1);
    // This was Calibri.
    CPPUNIT_ASSERT_EQUAL(u"Cambria"_ustr, getProperty<OUString>(xRun, u"CharFontName"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testAnchorIdForWP14AndW14)
{
    loadAndSave("AnchorId.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/mc:AlternateContent/mc:Choice/w:drawing/wp:inline"_ostr, "anchorId"_ostr, u"78735EFD"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/mc:AlternateContent/mc:Fallback/w:pict/v:rect"_ostr, "anchorId"_ostr, u"78735EFD"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testDkVert, "dkvert.docx")
{
    // <a:pattFill prst="dkVert"> was exported as ltVert.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was 50.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(25), getProperty<drawing::Hatch>(xShape, u"FillHatch"_ustr).Distance);
}

CPPUNIT_TEST_FIXTURE(Test, testTextWatermark)
{
    loadAndSave("textWatermark.docx");
    //The problem was that the watermark ID was not preserved,
    //and Word uses the object ID to identify if it is a watermark.
    //It has to have the 'PowerPlusWaterMarkObject' string in it
    xmlDocUniquePtr pXmlHeader2 = parseExport(u"word/header2.xml"_ustr);

    assertXPath(pXmlHeader2, "/w:hdr[1]/w:p[1]/w:r[1]/w:pict[1]/v:shape[1]"_ostr,"id"_ostr,u"PowerPlusWaterMarkObject93701316"_ustr);

    //The second problem was that Word uses also "o:spid"
    const OUString& sSpid = getXPath(pXmlHeader2, "/w:hdr[1]/w:p[1]/w:r[1]/w:pict[1]/v:shape[1]"_ostr,"spid"_ostr);
    CPPUNIT_ASSERT(!sSpid.isEmpty());
}

CPPUNIT_TEST_FIXTURE(Test, testPictureWatermark)
{
    loadAndSave("pictureWatermark.docx");
    //The problem was that the watermark ID was not preserved,
    //and Word uses the object ID to identify if it is a watermark.
    //It has to have the 'WordPictureWaterMarkObject' string in it

    xmlDocUniquePtr pXmlHeader2 = parseExport(u"word/header2.xml"_ustr);

    // Check the watermark ID
    assertXPath(pXmlHeader2, "/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Fallback[1]/w:pict[1]/v:shape[1]"_ostr,"id"_ostr,u"WordPictureWatermark11962361"_ustr);
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
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Fallback[1]/w:pict[1]/v:rect[1]/v:textbox[1]/w:txbxContent[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/lc:lockedCanvas[1]"_ostr,1);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo76979)
{
    loadAndSave("fdo76979.docx");
    // The problem was that black was exported as "auto" fill color, resulting in well-formed, but invalid XML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/header2.xml"_ustr);
    // This was "auto", not "FFFFFF".
    assertXPath(pXmlDoc, "//wps:spPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"FFFFFF"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104539)
{
    loadAndSave("tdf104539.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
            "dir"_ostr, u"13500000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testShapeEffectPreservation)
{
    loadAndSave("shape-effect-preservation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // first shape with outer shadow, rgb color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
            "algn"_ostr, u"tl"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
            "blurRad"_ostr, u"50760"_ustr); // because convertEMUtoHmm rounds fractions into nearest integer 50800 will be 50760
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
            "dir"_ostr, u"2700000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
            "dist"_ostr, u"37674"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
            "rotWithShape"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:srgbClr"_ostr,
            "val"_ostr, u"000000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:srgbClr/a:alpha"_ostr,
            "val"_ostr, u"40000"_ustr);

    // second shape with outer shadow, scheme color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
            "algn"_ostr, u"tl"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
            "blurRad"_ostr, u"114480"_ustr); // because convertEMUtoHmm rounds fractions into nearest integer 114300 will be 114480
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
            "dir"_ostr, u"2700000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
            "dist"_ostr, u"203137"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
            "rotWithShape"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr"_ostr,
            "val"_ostr, u"accent1"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr/a:lumMod"_ostr,
            "val"_ostr, u"40000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr/a:lumOff"_ostr,
            "val"_ostr, u"60000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw/a:schemeClr/a:alpha"_ostr,
            "val"_ostr, u"40000"_ustr);

    // third shape with inner shadow, rgb color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw"_ostr,
            "blurRad"_ostr, u"63500"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw"_ostr,
            "dir"_ostr, u"16200000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw"_ostr,
            "dist"_ostr, u"50800"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr"_ostr,
            "val"_ostr, u"ffff00"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr/a:alpha"_ostr,
            "val"_ostr, u"50000"_ustr);

    // 4th shape with soft edge
    assertXPathHasApproxEMU(
        pXmlDoc,
        "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:softEdge"_ostr,
        "rad"_ostr, 127000); // actually, it returns 127080
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:softEdge/*"_ostr,
            0 ); // should not be present

    // 5th shape with glow effect, scheme color
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:srgbClr"_ostr,
            "val"_ostr, u"eb2722"_ustr);

    // 6th shape with reflection
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection"_ostr,
            "blurRad"_ostr, u"6350"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection"_ostr,
            "stA"_ostr, u"50000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection"_ostr,
            "endA"_ostr, u"300"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection"_ostr,
            "endPos"_ostr, u"55500"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection/*"_ostr,
            0 ); // should not be present

    // 7th shape with several effects: glow, inner shadow and reflection
    assertXPathHasApproxEMU(pXmlDoc,
                            "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow"_ostr,
                            "rad"_ostr, 63500); // actually, it returns 63360
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:glow/a:srgbClr"_ostr,
            "val"_ostr, u"eb2722"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw"_ostr,
            "blurRad"_ostr, u"63500"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw"_ostr,
            "dir"_ostr, u"2700000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr"_ostr,
            "val"_ostr, u"000000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:innerShdw/a:srgbClr/a:alpha"_ostr,
            "val"_ostr, u"50000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection"_ostr,
            "blurRad"_ostr, u"6350"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:reflection"_ostr,
            "stA"_ostr, u"52000"_ustr);

}

CPPUNIT_TEST_FIXTURE(Test, testShape3DEffectPreservation)
{
    loadAndSave("shape-3d-effect-preservation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // first shape: extrusion and shift on z, rotated camera with zoom, rotated light rig
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera"_ostr,
            "prst"_ostr, u"perspectiveRelaxedModerately"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera"_ostr,
            "zoom"_ostr, u"150000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot"_ostr,
            "lat"_ostr, u"19490639"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot"_ostr,
            "lon"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot"_ostr,
            "rev"_ostr, u"12900001"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig"_ostr,
            "rig"_ostr, u"threePt"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig"_ostr,
            "dir"_ostr, u"t"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot"_ostr,
            "lat"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot"_ostr,
            "lon"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot"_ostr,
            "rev"_ostr, u"4800000"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d"_ostr,
            "extrusionH"_ostr, u"63500"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d"_ostr,
            "z"_ostr, u"488950"_ustr);

    // second shape: extrusion with theme color, no camera or light rotation, metal material
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera"_ostr,
            "prst"_ostr, u"isometricLeftDown"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:camera/a:rot"_ostr,
            0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig"_ostr,
            "rig"_ostr, u"threePt"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig"_ostr,
            "dir"_ostr, u"t"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:scene3d/a:lightRig/a:rot"_ostr,
            0);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d"_ostr,
            "extrusionH"_ostr, u"25400"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d"_ostr,
            "prstMaterial"_ostr, u"metal"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/a:schemeClr"_ostr,
            "val"_ostr, u"accent5"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/a:schemeClr/a:lumMod"_ostr,
            "val"_ostr, u"40000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:extrusionClr/a:schemeClr/a:lumOff"_ostr,
            "val"_ostr, u"60000"_ustr);

    // third shape: colored contour and top and bottom bevel, plastic material
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d"_ostr,
            "contourW"_ostr, u"50800"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d"_ostr,
            "prstMaterial"_ostr, u"plastic"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT"_ostr,
            "w"_ostr, u"139700"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT"_ostr,
            "h"_ostr, u"88900"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelT"_ostr,
            "prst"_ostr, u"cross"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelB"_ostr,
            "h"_ostr, u"88900"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:bevelB"_ostr,
            "prst"_ostr, u"relaxedInset"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d/a:contourClr/a:srgbClr"_ostr,
            "val"_ostr, u"3333ff"_ustr);

    // fourth shape: wireframe
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:sp3d"_ostr,
            "prstMaterial"_ostr, u"legacyWireframe"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testPictureEffectPreservation)
{
    loadAndSave("picture-effects-preservation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // first picture: glow effect with theme color and transformations, 3d rotation and extrusion
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow"_ostr,
            "rad"_ostr, u"228600"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow/a:srgbClr"_ostr,
            "val"_ostr, u"267de6"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:glow/a:srgbClr/a:alpha"_ostr,
            "val"_ostr, u"40000"_ustr);


    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:scene3d/a:camera"_ostr,
            "prst"_ostr, u"isometricRightUp"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:scene3d/a:lightRig"_ostr,
            "rig"_ostr, u"threePt"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:sp3d"_ostr,
            "extrusionH"_ostr, u"76200"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:sp3d/a:extrusionClr/a:srgbClr"_ostr,
            "val"_ostr, u"92d050"_ustr);

    // second picture: shadow and reflection effects
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw"_ostr,
            "dir"_ostr, u"8100000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw/a:srgbClr"_ostr,
            "val"_ostr, u"000000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:outerShdw/a:srgbClr/a:alpha"_ostr,
            "val"_ostr, u"40000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:reflection"_ostr,
            "dir"_ostr, u"5400000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:reflection/*"_ostr,
            0 ); // should not be present

    // third picture: soft edge effect
    assertXPathHasApproxEMU(
        pXmlDoc,
        "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
        "wp:anchor/a:graphic/a:graphicData/pic:pic/pic:spPr/a:effectLst/a:softEdge"_ostr,
        "rad"_ostr, 63500); // actually, it returns 63360
}

CPPUNIT_TEST_FIXTURE(Test, testPictureArtisticEffectPreservation)
{
    loadAndSave("picture-artistic-effects-preservation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    xmlDocUniquePtr pRelsDoc = parseExport(u"word/_rels/document.xml.rels"_ustr);

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(
            comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());

    // 1st picture: marker effect
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticMarker"_ostr,
            "trans"_ostr, u"14000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticMarker"_ostr,
            "size"_ostr, u"80"_ustr);

    OUString sEmbedId1 = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer"_ostr,
            "embed"_ostr);
    OUString sXmlPath = "/rels:Relationships/rels:Relationship[@Id='" + sEmbedId1 + "']";
    OUString sFile = getXPath(pRelsDoc, OUStringToOString( sXmlPath, RTL_TEXTENCODING_UTF8 ), "Target"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/" + sFile)));

    // 2nd picture: pencil grayscale
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilGrayscale"_ostr,
            "trans"_ostr, u"15000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilGrayscale"_ostr,
            "pencilSize"_ostr, u"66"_ustr);

    OUString sEmbedId2 = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer"_ostr,
            "embed"_ostr);
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId2);

    // 3rd picture: pencil sketch
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilSketch"_ostr,
            "trans"_ostr, u"7000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPencilSketch"_ostr,
            "pressure"_ostr, u"17"_ustr);

    OUString sEmbedId3 = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer"_ostr,
            "embed"_ostr);
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId3);

    // 4th picture: light screen
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticLightScreen"_ostr,
            "trans"_ostr, u"13000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticLightScreen"_ostr,
            "gridSize"_ostr, u"1"_ustr);

    OUString sEmbedId4 = getXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer"_ostr,
            "embed"_ostr);
    sXmlPath = "/rels:Relationships/rels:Relationship[@Id='" + sEmbedId4 + "']";
    sFile = getXPath(pRelsDoc, OUStringToOString( sXmlPath, RTL_TEXTENCODING_UTF8 ), "Target"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/" + sFile)));

    // 5th picture: watercolor sponge
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticWatercolorSponge"_ostr,
            "brushSize"_ostr, u"4"_ustr);

    OUString sEmbedId5 = getXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer"_ostr,
            "embed"_ostr);
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId5);

    // 6th picture: photocopy (no attributes)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/"
            "a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer/a14:imgEffect/"
            "a14:artisticPhotocopy"_ostr, 1);

    OUString sEmbedId6 = getXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
            "wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:extLst/a:ext/a14:imgProps/a14:imgLayer"_ostr,
            "embed"_ostr);
    CPPUNIT_ASSERT_EQUAL(sEmbedId1, sEmbedId6);

    // no redundant wdp files saved
    CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName(u"word/media/hdphoto3.wdp"_ustr)));
}

CPPUNIT_TEST_FIXTURE(Test, fdo77719)
{
    loadAndSave("fdo77719.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:custGeom[1]"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testNestedAlternateContent)
{
    loadAndSave("nestedAlternateContent.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // We check alternateContent  could not contains alternateContent (i.e. nested alternateContent)
    assertXPath(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wpg:wgp[1]/wps:wsp[2]/wps:txbx[1]/w:txbxContent[1]/w:p[1]/w:r[2]/mc:AlternateContent[1]"_ostr,0);
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
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[3]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]"_ostr, "relativeHeight"_ostr, u"3"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, test76317_2K10)
{
    loadAndSave("test76317_2K10.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:prstGeom[1]/a:avLst[1]/a:gd[1]"_ostr, "name"_ostr, u"adj"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFDO77122)
{
    loadAndSave("LinkedTextBoxes.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    //ensure that the text box links are preserved.
    assertXPath(pXmlDoc, "//wps:txbx[1]"_ostr, "id"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "//wps:linkedTxbx[1]"_ostr, "id"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, test76734_2K7)
{
    loadAndSave("test76734_2K7.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[1]/w:r[3]/mc:AlternateContent[1]/mc:Choice[1]"_ostr, "Requires"_ostr, u"wps"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, test77219)
{
    loadAndSave("test77219.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:p[6]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]"_ostr, "behindDoc"_ostr, u"1"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf77219_backgroundShape, "tdf77219_backgroundShape.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape is in front of the paragraph", false, getProperty<bool>(getShape(1), u"Opaque"_ustr));

    // tdf#126533: gradient is purple foreground to white background (top-right to bottom-left)
    uno::Reference<beans::XPropertySet> xRectangle(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xRectangle, u"FillStyle"_ustr));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xRectangle, u"FillGradient"_ustr);

    basegfx::BColorStops aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
    CPPUNIT_ASSERT_EQUAL(Color(0x5f497a), Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(aColorStops[1].getStopColor()));
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
    // without the fix, this was 1350 (visually the colors were reversed)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3150), aGradient.Angle);
}

DECLARE_OOXMLEXPORT_TEST(testTdf126533_axialAngle, "tdf126533_axialAngle.docx")
{
    // axial gradient is purple foreground/lime background in the middle (top-left to bottom-right)
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xPageStyle, u"FillStyle"_ustr));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xPageStyle, u"FillGradient"_ustr);

    // without the fix, this was 1350 (visually the colors were reversed)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2250), aGradient.Angle);
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_LIGHTGREEN), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_LIGHTMAGENTA), aGradient.EndColor);
}

DECLARE_OOXMLEXPORT_TEST(testTdf126533_axialAngle2, "tdf126533_axialAngle2.docx")
{
    // axial gradient is purple foreground/lime background in the middle (top-right to bottom-left)
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xPageStyle, u"FillStyle"_ustr));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xPageStyle, u"FillGradient"_ustr);

    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_LIGHTGREEN), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_LIGHTMAGENTA), aGradient.EndColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1350), aGradient.Angle);
}

DECLARE_OOXMLEXPORT_TEST(testTdf77219_foregroundShape, "tdf77219_foregroundShape.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape is in front of the paragraph", true, getProperty<bool>(getShape(1), u"Opaque"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf108973_backgroundTextbox, "tdf108973_backgroundTextbox.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox is in front of the paragraph", false, getProperty<bool>(getShape(1), u"Opaque"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf108973_foregroundTextbox, "tdf108973_foregroundTextbox.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox is in front of the paragraph", true, getProperty<bool>(getShape(1), u"Opaque"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testPresetShape, "preset-shape.docx")
{
    // Document contains a flowChartMultidocument preset shape, our date for that shape wasn't correct.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aCustomShapeGeometry(xPropertySet->getPropertyValue(u"CustomShapeGeometry"_ustr));
    comphelper::SequenceAsHashMap aPath(aCustomShapeGeometry[u"Path"_ustr]);
    uno::Sequence<awt::Size>      aSubViewSize((aPath[u"SubViewSize"_ustr]).get<uno::Sequence<awt::Size> >() );

    // This was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21600), aSubViewSize[0].Height);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo76101)
{
    loadAndSave("fdo76101.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/w:styles/w:style"_ostr);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT(4091 >= xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtAndShapeOverlapping)
{
    loadAndSave("ShapeOverlappingWithSdt.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/mc:AlternateContent"_ostr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt[1]/w:sdtContent[1]/w:r[1]"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testLockedCanvas)
{
    loadAndSave("fdo78658.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Checking for lockedCanvas tag
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/lc:lockedCanvas"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, fdo78474)
{
    loadAndSave("fdo78474.docx");
    xmlDocUniquePtr pXmlDoc1 = parseExport(u"word/document.xml"_ustr);
    //docx file after RT is getting corrupted.
    assertXPath(pXmlDoc1, "/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:txbx[1]/w:txbxContent[1]/w:p[1]/w:r[1]/w:drawing[1]/wp:inline[1]/a:graphic[1]/a:graphicData[1]/pic:pic[1]/pic:blipFill[1]/a:blip[1]"_ostr, "embed"_ostr, u"rId2"_ustr);

    xmlDocUniquePtr pXmlDoc2 = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(pXmlDoc2,"/rels:Relationships/rels:Relationship[2]"_ostr,"Id"_ostr,u"rId2"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testAbsolutePositionOffsetValue)
{
    loadAndSave("fdo78432.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    xmlXPathObjectPtr pXmlObjs[6];
    pXmlObjs[0] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]"_ostr);
    pXmlObjs[1] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]"_ostr);

    pXmlObjs[2] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[2]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]"_ostr);
    pXmlObjs[3] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[2]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]"_ostr);

    pXmlObjs[4] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[3]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionH[1]/wp:posOffset[1]"_ostr);
    pXmlObjs[5] = getXPathNode(pXmlDoc,"/w:document[1]/w:body[1]/w:p[1]/w:r[1]/mc:AlternateContent[3]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:positionV[1]/wp:posOffset[1]"_ostr);

    for(sal_Int32 index = 0; index<6; ++index)
    {
        CPPUNIT_ASSERT(pXmlObjs[index]->nodesetval != nullptr);
        xmlNodePtr pXmlNode = pXmlObjs[index]->nodesetval->nodeTab[0];
        OUString contents = OUString::createFromAscii(reinterpret_cast<const char*>((pXmlNode->children[0]).content));
        CPPUNIT_ASSERT( contents.toInt64() <= SAL_MAX_INT32 );
        xmlXPathFreeObject(pXmlObjs[index]);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testRubyHyperlink)
{
    loadAndReload("rubyhyperlink.fodt");
    // test that export doesn't assert with overlapping ruby / hyperlink attr
}

CPPUNIT_TEST_FIXTURE(Test, testfdo78300)
{
    loadAndSave("fdo78300.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing[1]/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[1]/w:r[1]/w:drawing[1]"_ostr,
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

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent"_ostr,1);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:p/w:r/w:pict/v:shape"_ostr,1);
    // Make sure that the shape inside a shape is exported as VML-only, no embedded mc:AlternateContent before w:pict.
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict"_ostr,1);
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

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict/v:shape/v:path"_ostr,1);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:p/w:r/w:pict/v:shape/v:path"_ostr,1);
    // Make sure that the shape inside a shape is exported as VML-only, no embedded mc:AlternateContent before w:pict.
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p/w:r/w:pict"_ostr,1);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo78957)
{
    loadAndSave("fdo78957.docx");
    xmlDocUniquePtr pXmlHeader = parseExport(u"word/header2.xml"_ustr);

    const sal_Int64 IntMax = SAL_MAX_INT32;
    sal_Int64 cx = 0, cy = 0;
    cx = getXPath(pXmlHeader,"/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:extent[1]"_ostr,"cx"_ostr).toInt64();
    cy = getXPath(pXmlHeader,"/w:hdr[1]/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:extent[1]"_ostr,"cy"_ostr).toInt64();
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
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln/a:prstDash"_ostr, "val"_ostr, u"lgDash"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDashedLine_CustDash1000thOfPercent)
{
    loadAndSave("dashed_line_custdash_1000th_of_percent.docx");
    /* Make sure that preset line is exported correctly as "1000th of a percent".
     * This test-file has a CUSTOM dash-line that is defined as '1000th of a percent'.
     * This should be imported by LO as-is, and exported back with the same values.
     */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]"_ostr, "d"_ostr , u"800000"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]"_ostr, "sp"_ostr, u"300000"_ustr);

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]"_ostr, "d"_ostr , u"100000"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]"_ostr, "sp"_ostr, u"300000"_ustr);

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]"_ostr, "d"_ostr , u"100000"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]"_ostr, "sp"_ostr, u"300000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDashedLine_CustDashPercentage)
{
    loadAndSave("dashed_line_custdash_percentage.docx");
    /* Make sure that preset line is exported correctly as "1000th of a percent".
     * This test-file has a CUSTOM dash-line that is defined as percentages.
     * This should be imported by LO as '1000th of a percent', and exported back
     * as '1000th of a percent'.
     */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]"_ostr, "d"_ostr , u"800000"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[1]"_ostr, "sp"_ostr, u"300000"_ustr);

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]"_ostr, "d"_ostr , u"100000"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[2]"_ostr, "sp"_ostr, u"300000"_ustr);

    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]"_ostr, "d"_ostr , u"100000"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/a:graphic[1]/a:graphicData[1]/wps:wsp[1]/wps:spPr[1]/a:ln[1]/a:custDash[1]/a:ds[3]"_ostr, "sp"_ostr, u"300000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testCommentInitials)
{
    loadAndSave("comment_initials.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/comments.xml"_ustr);

    assertXPath(pXmlDoc,"/w:comments/w:comment[1]"_ostr, "initials"_ostr, u"initials"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTextboxRoundedCorners, "textbox-rounded-corners.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    comphelper::SequenceAsHashMap aCustomShapeGeometry(getProperty< uno::Sequence<beans::PropertyValue> >(xShape, u"CustomShapeGeometry"_ustr));

    // Test that the shape is a rounded rectangle.
    CPPUNIT_ASSERT_EQUAL(u"ooxml-roundRect"_ustr, aCustomShapeGeometry[u"Type"_ustr].get<OUString>());

    // The shape text should start with a table, with "a" in its A1 cell.
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1, xText), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, xCell->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testfdo79591)
{
    loadAndSave("fdo79591.docx");
    /* Values set for docPr name and shape ID attributes
     * in RT file were not valid as per UTF-8 encoding format
     * and hence was showing RT document as corrupt with error
     * message "invalid character"
     */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/wp:docPr"_ostr, "name"_ostr, u"_x0000_t0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:shape"_ostr, "ID"_ostr, u"_x0000_t0"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testBnc884615, "bnc884615.docx")
{
    // The problem was that the shape in the header wasn't in the background.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(1), u"Opaque"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo80894)
{
    loadAndSave("TextFrameRotation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Rotation value was not roundtripped for textframe.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:xfrm"_ostr,
    "rot"_ostr,u"16200000"_ustr);

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

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/header2.xml"_ustr);
    assertXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:noFill"_ostr,0);
    assertXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:noFill"_ostr,0);

    // Check for fallback (required for MSO-2007)
    assertXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect"_ostr, "fillcolor"_ostr, u"#4f81bd"_ustr);
    assertXPath(pXmlDoc, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:fill"_ostr, "type"_ostr, u"solid"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf118242)
{
    loadAndSave("tdf118242.odt");
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
    "/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[2]/w:pPr/w:pStyle"_ostr, "val"_ostr, u"HeaderRight"_ustr);

    // w:sectPr is not exported
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r[2]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
    "/a:graphic/a:graphicData/wps:wsp/wps:txbx/w:txbxContent/w:p[2]/w:pPr/w:sectPr"_ostr, 0);

    // and drawing is no longer in the document
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r[3]"_ostr, 0);

    //but it's in the header
    pXmlDocument = parseExport(u"word/header1.xml"_ustr);

    assertXPath(pXmlDocument, "/w:hdr/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData"
            "/wps:wsp/wps:txbx/w:txbxContent/w:p[1]/w:r/w:drawing"_ostr, 1);

}

CPPUNIT_TEST_FIXTURE(Test, testWrapTightThrough)
{
    loadAndSave("wrap-tight-through.docx");
    // These were wrapSquare without a wrap polygon before.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // The first shape should be wrapThrough with a wrap polygon (was wrapSquare).
    assertXPath(pXmlDoc, "//w:drawing/wp:anchor[1]/wp:wrapThrough/wp:wrapPolygon/wp:start"_ostr, "x"_ostr, u"-1104"_ustr);
    // The second shape should be wrapTight with a wrap polygon (was wrapSquare).
    assertXPath(pXmlDoc, "//w:drawing/wp:anchor[1]/wp:wrapTight/wp:wrapPolygon/wp:start"_ostr, "y"_ostr, u"792"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testPictureWrapPolygon, "picture-wrap-polygon.docx")
{
    // The problem was that the wrap polygon was ignored during export.
    drawing::PointSequenceSequence aSeqSeq = getProperty<drawing::PointSequenceSequence>(getShape(1), u"ContourPolyPolygon"_ustr);
    // This was 0: the polygon list was empty.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aSeqSeq.getLength());

    drawing::PointSequence aSeq = aSeqSeq[0];
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aSeq.getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testPictureColormodeGrayscale)
{
    loadAndSave("picture_colormode_grayscale.docx");
    // The problem was that the grayscale was not exported
    xmlDocUniquePtr pXmlDoc = parseExport (u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:grayscl"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testPictureColormodeBlackWhite)
{
    loadAndSave("picture_colormode_black_white.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport (u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:biLevel"_ostr, "thresh"_ostr, u"50000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testPictureColormodeWatermark)
{
    loadAndSave("picture_colormode_watermark.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport (u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:lum"_ostr, "bright"_ostr, u"70000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:anchor/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:blip/a:lum"_ostr, "contrast"_ostr, u"-70000"_ustr);
}


CPPUNIT_TEST_FIXTURE(Test, testExportShadow)
{
    loadAndSave("bnc637947.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The problem was that shadows of shapes from non-OOXML origin were not exported to DrawingML
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw"_ostr, "dist"_ostr , u"109865"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw"_ostr, "dir"_ostr , u"634411"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw/a:srgbClr"_ostr, "val"_ostr , u"000000"_ustr);
    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/wps:wsp[1]/wps:spPr/a:effectLst/a:outerShdw/a:srgbClr/a:alpha"_ostr, "val"_ostr , u"38000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testExportAdjustmentValue)
{
    loadAndSave("tdf91429.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc,"/w:document/w:body/w:p/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom/a:avLst/a:gd"_ostr, "fmla"_ostr, u"val 50000"_ustr);
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
    if( !parseDump("/root/page/body/txt/anchored/fly[1]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[1]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[2]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[2]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[3]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[3]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[4]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[4]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[5]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[5]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[6]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[6]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[7]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[7]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[8]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[8]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[9]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[9]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[10]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[10]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[11]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[11]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[12]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[12]/txt"_ostr,"precede"_ostr).isEmpty() )
        precedeCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[13]/txt"_ostr,"follow"_ostr).isEmpty() )
        followCount++;
    if( !parseDump("/root/page/body/txt/anchored/fly[13]/txt"_ostr,"precede"_ostr).isEmpty() )
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
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:ind"_ostr, "start"_ostr, u"1418"_ustr);
}



CPPUNIT_TEST_FIXTURE(Test, testFlipAndRotateCustomShape)
{
    loadAndSave("flip_and_rotate.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // there should be no flipH
    assertXPathNoAttribute(pXmlDoc, "//a:xfrm"_ostr, "flipH"_ostr);
    // flipV should be there
    assertXPath(pXmlDoc, "//a:xfrm"_ostr, "flipV"_ostr, u"1"_ustr);
    // check rotation angle
    assertXPath(pXmlDoc, "//a:xfrm"_ostr, "rot"_ostr, u"8100000"_ustr);
    // point values depend on path size, values as of March 2022
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path"_ostr, "w"_ostr, u"21600"_ustr);
    assertXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path"_ostr, "h"_ostr, u"21600"_ustr);
    // check the first few coordinates of the polygon
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        0, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:moveTo/a:pt"_ostr, "x"_ostr).toInt32(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        15831, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:moveTo/a:pt"_ostr, "y"_ostr).toInt32(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        6098, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt"_ostr, "x"_ostr).toInt32(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        10062, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[1]/a:pt"_ostr, "y"_ostr).toInt32(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        13284, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[4]/a:pt"_ostr, "x"_ostr).toInt32(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        6098, getXPath(pXmlDoc, "//a:custGeom/a:pathLst/a:path/a:lnTo[4]/a:pt"_ostr, "y"_ostr).toInt32(), 1);
    // check path is closed
    assertXPath(pXmlDoc, "//a:close"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf92335)
{
    loadAndSave("tdf92335.docx");
    // Don't export redundant ListLabel character styles
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);

    assertXPath(pXmlStyles, "//w:style[@w:styleId='ListLabel1']"_ostr, 0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
