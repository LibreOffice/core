/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextContentAppend.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

using namespace com::sun::star;

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
};

DECLARE_OOXMLEXPORT_TEST(Tdf130907, "tdf130907.docx")
{
    uno::Reference<text::XTextRange> xPara1 = getParagraph(2);
    CPPUNIT_ASSERT(xPara1.is());
    uno::Reference<beans::XPropertySet> xFormula1Props(xPara1, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFormula1Props.is());
    sal_Int16 nHOri1;
    xFormula1Props->getPropertyValue(u"ParaAdjust"_ustr) >>= nHOri1;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The alignment of the equation is not left!",
        sal_Int16(style::ParagraphAdjust::ParagraphAdjust_LEFT), nHOri1);

    uno::Reference<text::XTextRange> xPara2 = getParagraph(3);
    CPPUNIT_ASSERT(xPara2.is());
    uno::Reference<beans::XPropertySet> xFormula2Props(xPara2, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFormula2Props.is());
    sal_Int16 nHOri2;
    xFormula2Props->getPropertyValue(u"ParaAdjust"_ustr) >>= nHOri2;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The alignment of the equation is not center!",
        sal_Int16(style::ParagraphAdjust::ParagraphAdjust_CENTER), nHOri2);

    uno::Reference<text::XTextRange> xPara3 = getParagraph(5);
    CPPUNIT_ASSERT(xPara3.is());
    uno::Reference<beans::XPropertySet> xFormula3Props(xPara3, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFormula3Props.is());
    sal_Int16 nHOri3;
    xFormula3Props->getPropertyValue(u"ParaAdjust"_ustr) >>= nHOri3;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The alignment of the equation is not right!",
        sal_Int16(style::ParagraphAdjust::ParagraphAdjust_RIGHT), nHOri3);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128197)
{
    createSwDoc("128197_compat14.docx");
    xmlDocUniquePtr pLayout14 = parseLayoutDump();
    sal_Int32 nHeight14 = getXPath(pLayout14, "//page[1]/body/txt[1]/infos/bounds", "height").toInt32();

    createSwDoc("128197_compat15.docx");
    xmlDocUniquePtr pLayout15 = parseLayoutDump();
    sal_Int32 nHeight15 = getXPath(pLayout15, "//page[1]/body/txt[1]/infos/bounds", "height").toInt32();

    // In compat mode=14 second line has size of the shape thus entire paragraph height is smaller
    // So nHeight14 < nHeight15
    CPPUNIT_ASSERT_LESS(nHeight15, nHeight14);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135595_HFtableWrap)
{
    loadAndReload("tdf135595_HFtableWrap.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nRowHeight = getXPath(pXmlDoc, "//page[1]/header/tab/row/infos/bounds", "height").toInt32();
    // tdf#77794: always force bLayoutInCell from false to true for MSO2013+
    // The fly is supposed to be inside the cell. Before, height was 998. Now it is 2839.
    CPPUNIT_ASSERT_MESSAGE("Image must be contained inside the table cell", nRowHeight > 2000);

    // tdf#162211: wrap-through fly can escape cell in upward direction now
    sal_Int32 nFooterImageBottom
        = getXPath(pXmlDoc,
                   "//page[1]/footer/tab/row/cell[1]/txt/anchored/fly/SwAnchoredObject/bounds",
                   "bottom")
             .toInt32();
    sal_Int32 nFooterBottom
        = getXPath(pXmlDoc, "//page[1]/footer/infos/bounds", "bottom").toInt32();
    // the image is above the bottom of the footer
    CPPUNIT_ASSERT(nFooterBottom > nFooterImageBottom); // image is higher footer
}

DECLARE_OOXMLEXPORT_TEST(testTdf135943_shapeWithText_L0c15,
                         "tdf135943_shapeWithText_LayoutInCell0_compat15.docx")
{
    // With compat15, layoutinCell ought to be ignored/forced to true.
    // HOWEVER, currently only the shape is correctly placed, while its text is un-synced separately.
    // So to prevent this ugly mess, just leave everything together in the historical (wrong) spot.
    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nFrameLeft = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds", "left").toInt32();
    sal_Int32 nFrameRight = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds", "right").toInt32();
    sal_Int32 nTextLeft = getXPath(pDump, "//anchored/fly/infos/bounds", "left").toInt32();
    sal_Int32 nTextRight = getXPath(pDump, "//anchored/fly/infos/bounds", "right").toInt32();
    // The text must be inside of its frame boundaries
    CPPUNIT_ASSERT(nFrameRight >= nTextRight);
    CPPUNIT_ASSERT(nFrameLeft <= nTextLeft);
    // LayoutInCell: The text must fit inside cell A1 //cell[1]/info/bounds/right = 4703
    //CPPUNIT_ASSERT(nTextRight < 4704);

    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    bool bValue;
    xShapeProperties->getPropertyValue(u"IsFollowingTextFlow"_ustr) >>= bValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("YOU FIXED ME? LayoutInCell ought to be true", false, bValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf135595_HFtableWrap_c12, "tdf135595_HFtableWrap_c12.docx")
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // tdf#104596: ignore wrap exception apparently does not apply if it is not "layout in table cell".
    // Should be only one page. Row height should be two lines at 722, not wrapped to three lines at 998.
    sal_Int32 nRowHeight = getXPath(pXmlDoc, "//header/tab/row/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_MESSAGE("Text must not wrap around header image", nRowHeight < 800);
}

DECLARE_OOXMLEXPORT_TEST(testTdf151704_thinColumnHeight, "tdf151704_thinColumnHeight.docx")
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nRowHeightT1 = getXPath(
        pXmlDoc, "//page[1]/body/tab[1]/row/cell/tab[1]/row[1]/infos/bounds", "height").toInt32();
    sal_Int32 nRowHeightT2 = getXPath(
        pXmlDoc, "//page[2]/body/tab/row[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Same row height in both tables", nRowHeightT1, nRowHeightT2);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123622, "tdf123622.docx")
{
    uno::Reference<beans::XPropertySet> XPropsRight(getShape(1),uno::UNO_QUERY);
    sal_Int16 nRelativePosR = 0;
    XPropsRight->getPropertyValue(u"HoriOrientRelation"_ustr)>>=nRelativePosR;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape inside the margin", sal_Int16(4), nRelativePosR);

    uno::Reference<beans::XPropertySet> XPropsLeft(getShape(2), uno::UNO_QUERY);
    sal_Int16 nRelativePosL = 0;
    XPropsLeft->getPropertyValue(u"HoriOrientRelation"_ustr) >>= nRelativePosL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape inside the margin", sal_Int16(3), nRelativePosL);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf78749)
{
    loadAndReload("tdf78749.docx");
    //Shape lost the background image before, now check if it still has...
    auto xShape = getShape(1);
    uno::Reference<beans::XPropertySet> xShpProps(xShape, uno::UNO_QUERY);
    OUString aPropertyVal;
    xShpProps->getPropertyValue(u"FillBitmapName"_ustr) >>= aPropertyVal;
    CPPUNIT_ASSERT(!aPropertyVal.isEmpty());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128207)
{
    loadAndSave("tdf128207.docx");
    //There was the charts on each other, because their horizontal and vertical position was 0!
    xmlDocUniquePtr p_XmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPathContent(p_XmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:anchor/wp:positionH/wp:posOffset", u"4445");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123873)
{
    loadAndSave("tdf123873.docx");
    //OLE Object were overlapped due to missing wrap import
    xmlDocUniquePtr p_XmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:drawing/wp:anchor/wp:wrapTopAndBottom");
}

CPPUNIT_TEST_FIXTURE(Test, Tdf133065)
{
    loadAndSave("tdf133065.odt");
    CPPUNIT_ASSERT_EQUAL(7, getShapes());
    CPPUNIT_ASSERT_EQUAL(3, getPages());
    auto pxmldoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pxmldoc);
    OUString aVal;

    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[3]/w:r[2]/w:object/v:shape/w10:wrap", "type");
    CPPUNIT_ASSERT(aVal.indexOf("square") > -1);
    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[3]/w:r[2]/w:object/v:shape/w10:wrap", "side");
    CPPUNIT_ASSERT(aVal.indexOf("left") > -1);

    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[8]/w:r[2]/w:object/v:shape/w10:wrap", "type");
    CPPUNIT_ASSERT(aVal.indexOf("square") > -1);
    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[8]/w:r[2]/w:object/v:shape/w10:wrap", "side");
    CPPUNIT_ASSERT(aVal.indexOf("right") > -1);

    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[12]/w:r[2]/w:object/v:shape/w10:wrap", "type");
    CPPUNIT_ASSERT(aVal.indexOf("square") > -1);
    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[12]/w:r[2]/w:object/v:shape/w10:wrap", "side");
    CPPUNIT_ASSERT(aVal.indexOf("largest") > -1);

    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[20]/w:r[2]/w:object/v:shape/w10:wrap", "type");
    CPPUNIT_ASSERT(aVal.indexOf("topAndBottom") > -1);

    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[24]/w:r[2]/w:object/v:shape/w10:wrap", "type");
    CPPUNIT_ASSERT(aVal.indexOf("square") > -1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf130814model, "tdf130814.docx")
{
    CPPUNIT_ASSERT_EQUAL(Color(0x1F497D), getProperty<Color>(getRun(getParagraph(2), 1), u"CharColor"_ustr));
    CPPUNIT_ASSERT_EQUAL(double(16), getProperty<double>(getRun(getParagraph(2), 1), u"CharHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::SINGLE, getProperty<sal_Int16>(getRun(getParagraph(2), 1), u"CharUnderline"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Candara"_ustr, getProperty<OUString>(getRun(getParagraph(2), 1), u"CharFontName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Arial Unicode MS"_ustr, getProperty<OUString>(getRun(getParagraph(2), 1), u"CharFontNameAsian"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107020)
{
    loadAndSave("tdf107020.docx");
    if (!IsDefaultDPI())
        return;
    xmlDocUniquePtr p_XmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect", "l", u"4910");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect", "t", u"27183");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect", "r", u"57638");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect", "b", u"48360");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130814ooxml)
{
    loadAndSave("tdf130814.docx");
    xmlDocUniquePtr p_XmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:rFonts", "eastAsia", u"Arial Unicode MS");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:rFonts", "ascii", u"Candara");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:rFonts", "hAnsi", u"Candara");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:color", "val", u"1F497D");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:sz", "val", u"32");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:szCs", "val", u"32");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:u", "val", u"single");
}

DECLARE_OOXMLEXPORT_TEST(testTdf129888vml, "tdf129888vml.docx")
{
    //the line shape has anchor in the first cell however it has to
    //be positioned to an another cell. To reach this we must handle
    //the o:allowincell attribute of the shape, and its position has
    //to be calculated from the page frame instead of the table:

    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    bool bValue;
    xShapeProperties->getPropertyValue(u"IsFollowingTextFlow"_ustr) >>= bValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf129888vml The line shape has bad place!",
                                 false, bValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf129888dml, "tdf129888dml.docx")
{
    //the shape has anchor in the first cell however it has to
    //be positioned to the right side of the page. To reach this we must handle
    //the layoutInCell attribute of the shape, and its position has
    //to be calculated from the page frame instead of the table:

    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    bool bValue;
    xShapeProperties->getPropertyValue(u"IsFollowingTextFlow"_ustr) >>= bValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf129888dml The shape has bad place!",
        false, bValue);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130120)
{
    loadAndSave("tdf130120.docx");
    // Text for exporting the allowincell attribute:
    xmlDocUniquePtr p_XmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(p_XmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r/mc:AlternateContent/"
        "mc:Choice/w:drawing/wp:anchor", "layoutInCell", u"0");
}

CPPUNIT_TEST_FIXTURE(Test, Tdf133030)
{
    loadAndSave("tdf133030.docx");
    auto pExport = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pExport);

    assertXPath(pExport, "/w:document/w:body/w:p[3]/m:oMathPara/m:oMathParaPr/m:jc", "val", u"center");
    assertXPath(pExport, "/w:document/w:body/w:p[5]/m:oMathPara/m:oMathParaPr/m:jc", "val", u"left");
    assertXPath(pExport, "/w:document/w:body/w:p[7]/m:oMathPara/m:oMathParaPr/m:jc", "val", u"right");
}

DECLARE_OOXMLEXPORT_TEST(testTdf87569v, "tdf87569_vml.docx")
{
    //the original tdf87569 sample has vml shapes...
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    bool bValue;
    xShapeProperties->getPropertyValue(u"IsFollowingTextFlow"_ustr) >>= bValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf87569_vml: The Shape is not in the table!",
                                 true, bValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf133000_numStyleFormatting, "tdf133000_numStyleFormatting.docx")
{
    // Paragraph style's LeftMargin should not override numbering's Left Margin
    xmlDocUniquePtr pDump = parseLayoutDump();
    assertXPathContent(pDump, "/root/page[1]/body/txt[2]", u"First line");
    const sal_Int32 nLevel1Margin = getXPath(pDump, "//page[1]/body/txt[2]/infos/prtBounds", "left").toInt32();
    assertXPathContent(pDump, "/root/page[1]/body/txt[4]", u"One sublevel");
    const sal_Int32 nLevel2Margin = getXPath(pDump, "//page[1]/body/txt[4]/infos/prtBounds", "left").toInt32();
    CPPUNIT_ASSERT( nLevel1Margin < nLevel2Margin );
}

DECLARE_OOXMLEXPORT_TEST(testTdf134260, "tdf134260.docx")
{
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1270

    auto xNum1Levels
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), u"NumberingRules"_ustr);

    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        comphelper::SequenceAsHashMap(xNum1Levels->getByIndex(0))[u"ListtabStopPosition"_ustr]
            .get<sal_Int32>());

    auto xNum2Levels
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(2), u"NumberingRules"_ustr);

    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        comphelper::SequenceAsHashMap(xNum2Levels->getByIndex(0))[u"ListtabStopPosition"_ustr]
            .get<sal_Int32>());

    auto xNum3Levels
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(3), u"NumberingRules"_ustr);

    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        comphelper::SequenceAsHashMap(xNum3Levels->getByIndex(0))[u"ListtabStopPosition"_ustr]
            .get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(Test, testArabicZeroNumberingFootnote)
{
    // Create a document, set footnote numbering type to ARABIC_ZERO.
    createSwDoc();
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::ARABIC_ZERO;
    xFootnoteSettings->setPropertyValue(u"NumberingType"_ustr, uno::Any(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance(u"com.sun.star.text.Footnote"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    saveAndReload(u"Office Open XML Text"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt' number of nodes is incorrect
    // because the exporter had no idea what markup to use for ARABIC_ZERO.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt", "val", u"decimalZero");
}

CPPUNIT_TEST_FIXTURE(Test, testChicagoNumberingFootnote)
{
    // Create a document, set footnote numbering type to SYMBOL_CHICAGO.
    createSwDoc();
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::SYMBOL_CHICAGO;
    xFootnoteSettings->setPropertyValue(u"NumberingType"_ustr, uno::Any(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance(u"com.sun.star.text.Footnote"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    saveAndReload(u"Office Open XML Text"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt' number of nodes is incorrect
    // because the exporter had no idea what markup to use for SYMBOL_CHICAGO.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt", "val", u"chicago");
}

CPPUNIT_TEST_FIXTURE(Test, testListNotCountedIndent)
{
    loadAndReload("list_notcounted_indent.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:numPr/w:numId", "val", u"0");
    // wrong: 720
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "end", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "hanging", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:numPr/w:numId", "val", u"2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:ind", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:numPr/w:numId", "val", u"0");
    // wrong: 180
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind", "start", u"720");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind", "hanging", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:numPr/w:numId", "val", u"0");
    // wrong: 720
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "end", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "hanging", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:numPr/w:numId", "val", u"0");
    // wrong: 720
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "end", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "hanging", u"0");
}

DECLARE_OOXMLEXPORT_TEST(testTdf87569d, "tdf87569_drawingml.docx")
{
    //if the original tdf87569 sample is upgraded it will have drawingml shapes...
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    bool bValue;
    xShapeProperties->getPropertyValue(u"IsFollowingTextFlow"_ustr) >>= bValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf87569_drawingml: The Shape is not in the table!",
                                 true, bValue);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130610)
{
    loadAndReload("tdf130610_bold_in_2_styles.ott");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // check character properties
    {
        uno::Reference<beans::XPropertySet> xStyle(
            getStyles(u"CharacterStyles"_ustr)->getByName(u"WollMuxRoemischeZiffer"_ustr),
            uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Bold", awt::FontWeight::BOLD, getProperty<float>(xStyle, u"CharWeight"_ustr));
    }

    // check paragraph properties
    {
        uno::Reference<beans::XPropertySet> xStyle(
            getStyles(u"ParagraphStyles"_ustr)->getByName(u"WollMuxVerfuegungspunkt"_ustr),
            uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Bold", awt::FontWeight::BOLD, getProperty<float>(xStyle, u"CharWeight"_ustr));
    }

    // check inline text properties
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:rPr/w:b");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121045)
{
    loadAndSave("tdf121045.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:sz", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:szCs", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[1]/w:fldChar", "fldCharType", u"begin");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:instrText", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[3]/w:fldChar", "fldCharType", u"separate");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[5]/w:fldChar", "fldCharType", u"end");
    // form control keeps its direct formatted font size
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:rPr/w:sz", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:rPr/w:szCs", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[3]/w:rPr/w:sz", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[3]/w:rPr/w:szCs", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[4]/w:rPr/w:sz", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[4]/w:rPr/w:szCs", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[5]/w:rPr/w:sz", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[5]/w:rPr/w:szCs", "val", u"20");
}

DECLARE_OOXMLEXPORT_TEST(testTdf78352, "tdf78352.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Ensure that width of first tab is close to zero (previous value was ~1000 twips)
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    int nWidth = getXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabLeft']", "width").toInt32();
    CPPUNIT_ASSERT_LESS(150, nWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf81567)
{
    loadAndReload("tdf81567.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    int nFrameWidth = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT_EQUAL(2371, nFrameWidth);

    int nFrameHeight = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(3520, nFrameHeight);

    int nFrameTop = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(1518, nFrameTop);

    int nImageWidth = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/txt/anchored/fly/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT_EQUAL(2370, nImageWidth);

    int nImageHeight = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/txt/anchored/fly/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(1605, nImageHeight);

    // Check the image is at the top of the frame
    // Without the fix in place, this test would have failed with:
    // - Expected: 1638
    // - Actual  : 2236
    int nImageTop = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/txt/anchored/fly/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(1638, nImageTop);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf92472)
{
    loadAndSave("tdf92472.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:sz", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:szCs", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:fldChar", "fldCharType", u"begin");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:fldChar", "fldCharType", u"separate");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[5]/w:fldChar", "fldCharType", u"end");
    // form control keeps its direct formatted font size
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr/w:sz", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr/w:szCs", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w:sz", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w:szCs", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[4]/w:rPr/w:sz", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[4]/w:rPr/w:szCs", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[5]/w:rPr/w:sz", "val", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[5]/w:rPr/w:szCs", "val", u"20");
}

CPPUNIT_TEST_FIXTURE(Test, Tdf133035)
{
    loadAndSave("tdf133035.docx");
    auto pxml = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pxml);
    OUString aXmlVal = getXPath(pxml, "/w:document/w:body/w:p[1]/w:r[1]/w:object/v:shape", "style");
    CPPUNIT_ASSERT(aXmlVal.indexOf("margin-left:186.6pt") > -1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf131260, "tdf131260.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTables->getCount());
}
DECLARE_OOXMLEXPORT_TEST(testTdf120315, "tdf120315.docx")
{
    // tdf#120315 cells of the second column weren't vertically merged
    // because their horizontal positions are different a little bit
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    CPPUNIT_ASSERT_EQUAL(getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr)[0]
                             .Position,
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(1), u"TableColumnSeparators"_ustr)[2]
                             .Position);
}

DECLARE_OOXMLEXPORT_TEST(testTdf108350_noFontdefaults, "tdf108350_noFontdefaults.docx")
{
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles(u"ParagraphStyles"_ustr);
    uno::Reference< beans::XPropertySet > xStyleProps(paragraphStyles->getByName(u"NoParent"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Times New Roman"_ustr, getProperty<OUString>(xStyleProps, u"CharFontName"_ustr));
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("Font size", 10.f, getProperty<float>(xStyleProps, "CharHeight"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123116_oversizedRowSplit)
{
    loadAndReload("tdf123116_oversizedRowSplit.odt");
    // Intentionally require a very non-backward-compatible, natural continuation of the table
    // instead of an ugly "page break" like MS Word does (and LO used to do).
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row splits over 4 pages", 4, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testPageContentBottom, "page-content-bottom.docx")
{
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    sal_Int16 nExpected = text::RelOrientation::PAGE_PRINT_AREA_BOTTOM;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 10 (PAGE_PRINT_AREA_BOTTOM)
    // - Actual  : 0 (FRAME)
    // i.e. the bottom-of-body relation was lost.
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129522_removeShadowStyle)
{
    loadAndReload("tdf129522_removeShadowStyle.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles(u"ParagraphStyles"_ustr);
    uno::Reference< beans::XPropertySet > xStyleProps(paragraphStyles->getByName(u"Shadow"_ustr), uno::UNO_QUERY_THROW);
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xStyleProps, u"ParaShadowFormat"_ustr);
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    // Shadows were inherited regardless of whether the style disabled them.
    xStyleProps.set(paragraphStyles->getByName(u"Shadow-removed"_ustr), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, u"ParaShadowFormat"_ustr);
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

    uno::Reference< container::XNameAccess > characterStyles = getStyles(u"CharacterStyles"_ustr);
    xStyleProps.set(characterStyles->getByName(u"CharShadow"_ustr), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, u"CharShadowFormat"_ustr);
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    xStyleProps.set(characterStyles->getByName(u"CharShadow-removed"_ustr), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, u"CharShadowFormat"_ustr);
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);
}

DECLARE_OOXMLEXPORT_TEST(testTdf130167_spilloverHeaderShape, "testTdf130167_spilloverHeader.docx")
{
    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xNameAccess(
        xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);
    // graphics from discarded headers were being added to the text body. Reduced from 5 to 2 shapes overall.
    // CPPUNIT_ASSERT(xNameAccess->getCount() <= 4); -> What about hidden headers?
    CPPUNIT_ASSERT_LESS(sal_Int32(9), xNameAccess->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf124986, "tdf124986.docx")
{
    // Load a document with SET fields, where the SET fields contain leading/trailing quotation marks and spaces.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        if (xServiceInfo->supportsService(u"com.sun.star.text.TextField.SetExpression"_ustr))
        {
            OUString aValue;
            xPropertySet->getPropertyValue(u"Content"_ustr) >>= aValue;
            CPPUNIT_ASSERT_EQUAL(u"demo"_ustr, aValue);
        }
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf83309, "tdf83309.docx")
{
    // Important: bug case had 4 pages
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // First paragraph does not have tab before
    // (same applies to all paragraphs in doc, but let's assume they are
    // behaving same way)
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    OUString sNodeType = getXPath(pXmlDoc, "(/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*)[1]", "type");
    CPPUNIT_ASSERT_EQUAL(u"PortionType::Text"_ustr, sNodeType);

    // tdf148380: creation-date field in header.xml was unsupported on export
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"8/31/14 10:26 AM"_ustr, xField->getPresentation(false));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121661)
{
    loadAndSave("tdf121661.docx");
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:hyphenationZone", "val", u"851");

    // tdf#149421
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was false
    CPPUNIT_ASSERT_GREATER( static_cast<sal_Int16>(0), getProperty<sal_Int16>(xStyle, u"ParaHyphenationZone"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149421)
{
    auto verify = [this](bool bIsExport = false) {
        uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
        // This was false
        CPPUNIT_ASSERT_GREATER( static_cast<sal_Int16>(0), getProperty<sal_Int16>(xStyle, u"ParaHyphenationZone"_ustr));

        if (!bIsExport)
        {
            CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int16>(851), getProperty<sal_Int16>(xStyle, u"ParaHyphenationZone"_ustr));
            // modify hyphenation zone (note: only hyphenation zone set in Standard paragraph style
            // is exported, according to the document-level hyphenation settings of OOXML)
            xStyle->setPropertyValue(u"ParaHyphenationZone"_ustr, uno::Any(static_cast<sal_Int16>(2000)));
        }
        else
        {
            // check the export of the modified hyphenation zone
            CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int16>(2000), getProperty<sal_Int16>(xStyle, u"ParaHyphenationZone"_ustr));
        }
    };

    createSwDoc("tdf121661.docx");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

DECLARE_OOXMLEXPORT_TEST(testTdf149421_default, "tdf146171.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was 0 (not the default OOXML hyphenationZone)
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int16>(360), getProperty<sal_Int16>(xStyle, u"ParaHyphenationZone"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf161628, "tdf132599_frames_on_right_pages_no_hyphenation.fodt")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was 360 after the second import (missing export of zero hyphenation zone)
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int16>(0), getProperty<sal_Int16>(xStyle, u"ParaHyphenationZone"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf161643)
{
    loadAndSave("fdo76163.docx");
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:consecutiveHyphenLimit", "val", u"1");

    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was false (value 0)
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int16>(1), getProperty<sal_Int16>(xStyle, u"ParaHyphenationMaxHyphens"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121658)
{
    loadAndSave("tdf121658.docx");
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:doNotHyphenateCaps");
}

CPPUNIT_TEST_FIXTURE(Test, testTableStyleConfNested)
{
    loadAndSave("table-style-conf-nested.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed, as the custom table cell
    // border properties were lost, so the outer A2 cell started to have borders, not present in the
    // doc model.
    assertXPath(pXmlDoc, "//w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:tcBorders/w:top", "val", u"nil");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133771)
{
    // Create the doc model.
    createSwDoc("tdf133771.odt", /*pPassword*/ "test");

    CPPUNIT_ASSERT_EQUAL(u"Password Protected"_ustr, getParagraph(1)->getString());

    // Without the fix in place, this test would have failed with
    // "An uncaught exception of type com.sun.star.io.IOException"
    // exporting to docx
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathContent(pXmlDoc, "//w:body/w:p/w:r/w:t", u"Password Protected");
}

CPPUNIT_TEST_FIXTURE(Test, testZeroLineSpacing)
{
    // Create the doc model.
    createSwDoc();
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    style::LineSpacing aSpacing;
    aSpacing.Mode = style::LineSpacingMode::MINIMUM;
    aSpacing.Height = 0;
    xParagraph->setPropertyValue(u"ParaLineSpacing"_ustr, uno::Any(aSpacing));

    // Export to docx.
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: atLeast
    // - Actual  : auto
    // i.e. the minimal linespacing was lost on export.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing", "lineRule", u"atLeast");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing", "line", u"0");
}

CPPUNIT_TEST_FIXTURE(Test, testSemiTransparentText)
{
    // Create an in-memory empty document.
    createSwDoc();

    // Set text to half-transparent and type a character.
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph.is());
    sal_Int16 nTransparence = 75;
    xParagraph->setPropertyValue(u"CharTransparence"_ustr, uno::Any(nTransparence));
    uno::Reference<text::XTextRange> xTextRange(xParagraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextRange.is());
    xTextRange->setString(u"x"_ustr);

    // Export to docx.
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    double fValue = getXPath(
            pXmlDoc,
            "/w:document/w:body/w:p/w:r/w:rPr/w14:textFill/w14:solidFill/w14:srgbClr/w14:alpha",
            "val")
        .toDouble();
    sal_Int16 nActual = basegfx::fround(fValue / oox::drawingml::PER_PERCENT);

    // Without the accompanying fix in place, this test would have failed, as the w14:textFill
    // element was missing.
    CPPUNIT_ASSERT_EQUAL(nTransparence, nActual);
}

DECLARE_OOXMLEXPORT_TEST(testTdf147485, "Tdf147485.docx")
{
    // Before the fix this was impossible.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf149546, "tdf149546.docx")
{
    // Before the fix this was impossible.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testUserField)
{
    // Create an in-memory empty document with a user field.
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XDependentTextField> xField(
        xFactory->createInstance(u"com.sun.star.text.TextField.User"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMaster(
        xFactory->createInstance(u"com.sun.star.text.FieldMaster.User"_ustr), uno::UNO_QUERY);
    xMaster->setPropertyValue(u"Name"_ustr, uno::Any(u"foo"_ustr));
    xField->attachTextFieldMaster(xMaster);
    xField->getTextFieldMaster()->setPropertyValue(u"Content"_ustr, uno::Any(u"bar"_ustr));
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xField, /*bAbsorb=*/false);

    // Export to docx.
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed, the user field was
    // exported as <w:t>User Field foo = bar</w:t>.
    assertXPathContent(pXmlDoc, "//w:p/w:r[2]/w:instrText", u" DOCVARIABLE foo ");
    assertXPathContent(pXmlDoc, "//w:p/w:r[4]/w:t", u"bar");

    // Make sure that not only the variables, but also their values are written.
    pXmlDoc = parseExport(u"word/settings.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//w:docVars/w:docVar", "name", u"foo");
    assertXPath(pXmlDoc, "//w:docVars/w:docVar", "val", u"bar");
}


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
