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
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XTextContentAppend.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <editeng/unolingu.hxx>

#include <comphelper/sequenceashashmap.hxx>
#include <officecfg/Office/Common.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <o3tl/string_view.hxx>

using namespace com::sun::star;

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}
};

DECLARE_OOXMLEXPORT_TEST(Tdf130907, "tdf130907.docx")
{
    uno::Reference<text::XTextRange> xPara1 = getParagraph(2);
    CPPUNIT_ASSERT(xPara1.is());
    uno::Reference<beans::XPropertySet> xFormula1Props(xPara1, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFormula1Props.is());
    sal_Int16 nHOri1;
    xFormula1Props->getPropertyValue("ParaAdjust") >>= nHOri1;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The alignment of the equation is not left!",
        sal_Int16(style::ParagraphAdjust::ParagraphAdjust_LEFT), nHOri1);

    uno::Reference<text::XTextRange> xPara2 = getParagraph(3);
    CPPUNIT_ASSERT(xPara2.is());
    uno::Reference<beans::XPropertySet> xFormula2Props(xPara2, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFormula2Props.is());
    sal_Int16 nHOri2;
    xFormula2Props->getPropertyValue("ParaAdjust") >>= nHOri2;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The alignment of the equation is not center!",
        sal_Int16(style::ParagraphAdjust::ParagraphAdjust_CENTER), nHOri2);

    uno::Reference<text::XTextRange> xPara3 = getParagraph(5);
    CPPUNIT_ASSERT(xPara3.is());
    uno::Reference<beans::XPropertySet> xFormula3Props(xPara3, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFormula3Props.is());
    sal_Int16 nHOri3;
    xFormula3Props->getPropertyValue("ParaAdjust") >>= nHOri3;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The alignment of the equation is not right!",
        sal_Int16(style::ParagraphAdjust::ParagraphAdjust_RIGHT), nHOri3);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128197)
{
    createSwDoc("128197_compat14.docx");
    xmlDocUniquePtr pLayout14 = parseLayoutDump();
    sal_Int32 nHeight14 = getXPath(pLayout14, "//page[1]/body/txt[1]/infos/bounds"_ostr, "height"_ostr).toInt32();

    createSwDoc("128197_compat15.docx");
    xmlDocUniquePtr pLayout15 = parseLayoutDump();
    sal_Int32 nHeight15 = getXPath(pLayout15, "//page[1]/body/txt[1]/infos/bounds"_ostr, "height"_ostr).toInt32();

    // In compat mode=14 second line has size of the shape thus entire paragraph height is smaller
    // So nHeight14 < nHeight15
    CPPUNIT_ASSERT_LESS(nHeight15, nHeight14);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135595_HFtableWrap)
{
    loadAndReload("tdf135595_HFtableWrap.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nRowHeight = getXPath(pXmlDoc, "//page[1]/header/tab/row/infos/bounds"_ostr, "height"_ostr).toInt32();
    // tdf#77794: always force bLayoutInCell from false to true for MSO2013+
    // The fly is supposed to be inside the cell. Before, height was 998. Now it is 2839.
    CPPUNIT_ASSERT_MESSAGE("Image must be contained inside the table cell", nRowHeight > 2000);
}

DECLARE_OOXMLEXPORT_TEST(testTdf135943_shapeWithText_L0c15,
                         "tdf135943_shapeWithText_LayoutInCell0_compat15.docx")
{
    // With compat15, layoutinCell ought to be ignored/forced to true.
    // HOWEVER, currently only the shape is correctly placed, while its text is un-synced separately.
    // So to prevent this ugly mess, just leave everything together in the historical (wrong) spot.
    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nFrameLeft = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "left"_ostr).toInt32();
    sal_Int32 nFrameRight = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "right"_ostr).toInt32();
    sal_Int32 nTextLeft = getXPath(pDump, "//anchored/fly/infos/bounds"_ostr, "left"_ostr).toInt32();
    sal_Int32 nTextRight = getXPath(pDump, "//anchored/fly/infos/bounds"_ostr, "right"_ostr).toInt32();
    // The text must be inside of its frame boundaries
    CPPUNIT_ASSERT(nFrameRight >= nTextRight);
    CPPUNIT_ASSERT(nFrameLeft <= nTextLeft);
    // LayoutInCell: The text must fit inside cell A1 //cell[1]/info/bounds/right = 4703
    //CPPUNIT_ASSERT(nTextRight < 4704);

    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    bool bValue;
    xShapeProperties->getPropertyValue("IsFollowingTextFlow") >>= bValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("YOU FIXED ME? LayoutInCell ought to be true", false, bValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf135595_HFtableWrap_c12, "tdf135595_HFtableWrap_c12.docx")
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // tdf#104596: ignore wrap exception apparently does not apply if it is not "layout in table cell".
    // Should be only one page. Row height should be two lines at 722, not wrapped to three lines at 998.
    sal_Int32 nRowHeight = getXPath(pXmlDoc, "//header/tab/row/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_MESSAGE("Text must not wrap around header image", nRowHeight < 800);
}

DECLARE_OOXMLEXPORT_TEST(testTdf151704_thinColumnHeight, "tdf151704_thinColumnHeight.docx")
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nRowHeightT1 = getXPath(
        pXmlDoc, "//page[1]/body/tab[1]/row/cell/tab[1]/row[1]/infos/bounds"_ostr, "height"_ostr).toInt32();
    sal_Int32 nRowHeightT2 = getXPath(
        pXmlDoc, "//page[2]/body/tab/row[1]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Same row height in both tables", nRowHeightT1, nRowHeightT2);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123622, "tdf123622.docx")
{
    uno::Reference<beans::XPropertySet> XPropsRight(getShape(1),uno::UNO_QUERY);
    sal_Int16 nRelativePosR = 0;
    XPropsRight->getPropertyValue("HoriOrientRelation")>>=nRelativePosR;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape inside the margin", sal_Int16(4), nRelativePosR);

    uno::Reference<beans::XPropertySet> XPropsLeft(getShape(2), uno::UNO_QUERY);
    sal_Int16 nRelativePosL = 0;
    XPropsLeft->getPropertyValue("HoriOrientRelation") >>= nRelativePosL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape inside the margin", sal_Int16(3), nRelativePosL);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf78749)
{
    loadAndReload("tdf78749.docx");
    //Shape lost the background image before, now check if it still has...
    auto xShape = getShape(1);
    uno::Reference<beans::XPropertySet> xShpProps(xShape, uno::UNO_QUERY);
    OUString aPropertyVal;
    xShpProps->getPropertyValue("FillBitmapName") >>= aPropertyVal;
    CPPUNIT_ASSERT(!aPropertyVal.isEmpty());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128207)
{
    loadAndSave("tdf128207.docx");
    //There was the charts on each other, because their horizontal and vertical position was 0!
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPathContent(p_XmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:anchor/wp:positionH/wp:posOffset"_ostr, "4445");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123873)
{
    loadAndSave("tdf123873.docx");
    //OLE Object were overlapped due to missing wrap import
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:drawing/wp:anchor/wp:wrapTopAndBottom"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, Tdf133065)
{
    loadAndSave("tdf133065.odt");
    CPPUNIT_ASSERT_EQUAL(7, getShapes());
    CPPUNIT_ASSERT_EQUAL(3, getPages());
    auto pxmldoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pxmldoc);
    OUString aVal;

    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[3]/w:r[2]/w:object/v:shape/w10:wrap"_ostr, "type"_ostr);
    CPPUNIT_ASSERT(aVal.indexOf("square") > -1);
    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[3]/w:r[2]/w:object/v:shape/w10:wrap"_ostr, "side"_ostr);
    CPPUNIT_ASSERT(aVal.indexOf("left") > -1);

    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[8]/w:r[2]/w:object/v:shape/w10:wrap"_ostr, "type"_ostr);
    CPPUNIT_ASSERT(aVal.indexOf("square") > -1);
    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[8]/w:r[2]/w:object/v:shape/w10:wrap"_ostr, "side"_ostr);
    CPPUNIT_ASSERT(aVal.indexOf("right") > -1);

    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[12]/w:r[2]/w:object/v:shape/w10:wrap"_ostr, "type"_ostr);
    CPPUNIT_ASSERT(aVal.indexOf("square") > -1);
    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[12]/w:r[2]/w:object/v:shape/w10:wrap"_ostr, "side"_ostr);
    CPPUNIT_ASSERT(aVal.indexOf("largest") > -1);

    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[20]/w:r[2]/w:object/v:shape/w10:wrap"_ostr, "type"_ostr);
    CPPUNIT_ASSERT(aVal.indexOf("topAndBottom") > -1);

    aVal = getXPath(pxmldoc, "/w:document/w:body/w:p[24]/w:r[2]/w:object/v:shape/w10:wrap"_ostr, "type"_ostr);
    CPPUNIT_ASSERT(aVal.indexOf("square") > -1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf130814model, "tdf130814.docx")
{
    CPPUNIT_ASSERT_EQUAL(Color(0x1F497D), getProperty<Color>(getRun(getParagraph(2), 1), "CharColor"));
    CPPUNIT_ASSERT_EQUAL(double(16), getProperty<double>(getRun(getParagraph(2), 1), "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::SINGLE, getProperty<sal_Int16>(getRun(getParagraph(2), 1), "CharUnderline"));
    CPPUNIT_ASSERT_EQUAL(OUString("Candara"), getProperty<OUString>(getRun(getParagraph(2), 1), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Arial Unicode MS"), getProperty<OUString>(getRun(getParagraph(2), 1), "CharFontNameAsian"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107020)
{
    loadAndSave("tdf107020.docx");
    if (!IsDefaultDPI())
        return;
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect"_ostr, "l"_ostr, "4910");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect"_ostr, "t"_ostr, "27183");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect"_ostr, "r"_ostr, "57638");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect"_ostr, "b"_ostr, "48360");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130814ooxml)
{
    loadAndSave("tdf130814.docx");
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:rFonts"_ostr, "eastAsia"_ostr, "Arial Unicode MS");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:rFonts"_ostr, "ascii"_ostr, "Candara");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:rFonts"_ostr, "hAnsi"_ostr, "Candara");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:color"_ostr, "val"_ostr, "1F497D");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:sz"_ostr, "val"_ostr, "32");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:szCs"_ostr, "val"_ostr, "32");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:u"_ostr, "val"_ostr, "single");
}

DECLARE_OOXMLEXPORT_TEST(testTdf129888vml, "tdf129888vml.docx")
{
    //the line shape has anchor in the first cell however it has to
    //be positioned to an another cell. To reach this we must handle
    //the o:allowincell attribute of the shape, and its position has
    //to be calculated from the page frame instead of the table:

    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    bool bValue;
    xShapeProperties->getPropertyValue("IsFollowingTextFlow") >>= bValue;
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
    xShapeProperties->getPropertyValue("IsFollowingTextFlow") >>= bValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf129888dml The shape has bad place!",
        false, bValue);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130120)
{
    loadAndSave("tdf130120.docx");
    // Text for exporting the allowincell attribute:
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    assertXPath(p_XmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r/mc:AlternateContent/"
        "mc:Choice/w:drawing/wp:anchor"_ostr, "layoutInCell"_ostr, "0");
}

CPPUNIT_TEST_FIXTURE(Test, Tdf133030)
{
    loadAndSave("tdf133030.docx");
    auto pExport = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pExport);

    assertXPath(pExport, "/w:document/w:body/w:p[3]/m:oMathPara/m:oMathParaPr/m:jc"_ostr, "val"_ostr, "center");
    assertXPath(pExport, "/w:document/w:body/w:p[5]/m:oMathPara/m:oMathParaPr/m:jc"_ostr, "val"_ostr, "left");
    assertXPath(pExport, "/w:document/w:body/w:p[7]/m:oMathPara/m:oMathParaPr/m:jc"_ostr, "val"_ostr, "right");
}

DECLARE_OOXMLEXPORT_TEST(testTdf87569v, "tdf87569_vml.docx")
{
    //the original tdf87569 sample has vml shapes...
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    bool bValue;
    xShapeProperties->getPropertyValue("IsFollowingTextFlow") >>= bValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tdf87569_vml: The Shape is not in the table!",
                                 true, bValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf133000_numStyleFormatting, "tdf133000_numStyleFormatting.docx")
{
    // Paragraph style's LeftMargin should not override numbering's Left Margin
    xmlDocUniquePtr pDump = parseLayoutDump();
    assertXPathContent(pDump, "/root/page[1]/body/txt[2]"_ostr, "First line");
    const sal_Int32 nLevel1Margin = getXPath(pDump, "//page[1]/body/txt[2]/infos/prtBounds"_ostr, "left"_ostr).toInt32();
    assertXPathContent(pDump, "/root/page[1]/body/txt[4]"_ostr, "One sublevel");
    const sal_Int32 nLevel2Margin = getXPath(pDump, "//page[1]/body/txt[4]/infos/prtBounds"_ostr, "left"_ostr).toInt32();
    CPPUNIT_ASSERT( nLevel1Margin < nLevel2Margin );
}

DECLARE_OOXMLEXPORT_TEST(testTdf134260, "tdf134260.docx")
{
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1270

    auto xNum1Levels
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");

    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        comphelper::SequenceAsHashMap(xNum1Levels->getByIndex(0))["ListtabStopPosition"]
            .get<sal_Int32>());

    auto xNum2Levels
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(2), "NumberingRules");

    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        comphelper::SequenceAsHashMap(xNum2Levels->getByIndex(0))["ListtabStopPosition"]
            .get<sal_Int32>());

    auto xNum3Levels
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(3), "NumberingRules");

    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        comphelper::SequenceAsHashMap(xNum3Levels->getByIndex(0))["ListtabStopPosition"]
            .get<sal_Int32>());
}

DECLARE_ODFEXPORT_TEST(testArabicZeroNumbering, "arabic-zero-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 64
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero3Numbering, "arabic-zero3-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 65
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO3.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO3),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero4Numbering, "arabic-zero4-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 66
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO4.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO4),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero5Numbering, "arabic-zero5-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 67
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO5.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO5),
                         aMap["NumberingType"].get<sal_uInt16>());
}

CPPUNIT_TEST_FIXTURE(Test, testArabicZeroNumberingFootnote)
{
    // Create a document, set footnote numbering type to ARABIC_ZERO.
    createSwDoc();
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::ARABIC_ZERO;
    xFootnoteSettings->setPropertyValue("NumberingType", uno::Any(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    saveAndReload("Office Open XML Text");

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt' number of nodes is incorrect
    // because the exporter had no idea what markup to use for ARABIC_ZERO.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt"_ostr, "val"_ostr, "decimalZero");
}

CPPUNIT_TEST_FIXTURE(Test, testChicagoNumberingFootnote)
{
    // Create a document, set footnote numbering type to SYMBOL_CHICAGO.
    createSwDoc();
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::SYMBOL_CHICAGO;
    xFootnoteSettings->setPropertyValue("NumberingType", uno::Any(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    saveAndReload("Office Open XML Text");

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt' number of nodes is incorrect
    // because the exporter had no idea what markup to use for SYMBOL_CHICAGO.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt"_ostr, "val"_ostr, "chicago");
}

CPPUNIT_TEST_FIXTURE(Test, testListNotCountedIndent)
{
    loadAndReload("list_notcounted_indent.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:numPr/w:numId"_ostr, "val"_ostr, "0");
    // wrong: 720
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind"_ostr, "start"_ostr, "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind"_ostr, "end"_ostr, "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind"_ostr, "hanging"_ostr, "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:numPr/w:numId"_ostr, "val"_ostr, "2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:ind"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:numPr/w:numId"_ostr, "val"_ostr, "0");
    // wrong: 180
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind"_ostr, "start"_ostr, "720");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind"_ostr, "hanging"_ostr, "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:numPr/w:numId"_ostr, "val"_ostr, "0");
    // wrong: 720
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind"_ostr, "start"_ostr, "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind"_ostr, "end"_ostr, "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind"_ostr, "hanging"_ostr, "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:numPr/w:numId"_ostr, "val"_ostr, "0");
    // wrong: 720
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind"_ostr, "start"_ostr, "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind"_ostr, "end"_ostr, "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind"_ostr, "hanging"_ostr, "0");
}

DECLARE_OOXMLEXPORT_TEST(testTdf87569d, "tdf87569_drawingml.docx")
{
    //if the original tdf87569 sample is upgraded it will have drawingml shapes...
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    bool bValue;
    xShapeProperties->getPropertyValue("IsFollowingTextFlow") >>= bValue;
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
            getStyles("CharacterStyles")->getByName("WollMuxRoemischeZiffer"),
            uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Bold", awt::FontWeight::BOLD, getProperty<float>(xStyle, "CharWeight"));
    }

    // check paragraph properties
    {
        uno::Reference<beans::XPropertySet> xStyle(
            getStyles("ParagraphStyles")->getByName("WollMuxVerfuegungspunkt"),
            uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Bold", awt::FontWeight::BOLD, getProperty<float>(xStyle, "CharWeight"));
    }

    // check inline text properties
    {
        if (isExported())
        {
            xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
            assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:rPr/w:b"_ostr);
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121045)
{
    loadAndSave("tdf121045.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:sz"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:szCs"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[1]/w:fldChar"_ostr, "fldCharType"_ostr, "begin");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:instrText"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[3]/w:fldChar"_ostr, "fldCharType"_ostr, "separate");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[5]/w:fldChar"_ostr, "fldCharType"_ostr, "end");
    // form control keeps its direct formatted font size
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:rPr/w:sz"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:rPr/w:szCs"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[3]/w:rPr/w:sz"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[3]/w:rPr/w:szCs"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[4]/w:rPr/w:sz"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[4]/w:rPr/w:szCs"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[5]/w:rPr/w:sz"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[5]/w:rPr/w:szCs"_ostr, "val"_ostr, "20");
}

DECLARE_OOXMLEXPORT_TEST(testTdf78352, "tdf78352.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Ensure that width of first tab is close to zero (previous value was ~1000 twips)
    int nWidth = parseDump("/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabLeft']"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(150, nWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf81567)
{
    loadAndReload("tdf81567.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    int nFrameWidth = parseDump("/root/page/body/txt/anchored/fly/infos/bounds"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(2371, nFrameWidth);

    int nFrameHeight = parseDump("/root/page/body/txt/anchored/fly/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(3520, nFrameHeight);

    int nFrameTop = parseDump("/root/page/body/txt/anchored/fly/infos/bounds"_ostr, "top"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(1518, nFrameTop);

    int nImageWidth = parseDump("/root/page/body/txt/anchored/fly/txt/anchored/fly/infos/bounds"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(2370, nImageWidth);

    int nImageHeight = parseDump("/root/page/body/txt/anchored/fly/txt/anchored/fly/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(1605, nImageHeight);

    // Check the image is at the top of the frame
    // Without the fix in place, this test would have failed with:
    // - Expected: 1638
    // - Actual  : 2236
    int nImageTop = parseDump("/root/page/body/txt/anchored/fly/txt/anchored/fly/infos/bounds"_ostr, "top"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(1638, nImageTop);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf92472)
{
    loadAndSave("tdf92472.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:sz"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:szCs"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:fldChar"_ostr, "fldCharType"_ostr, "begin");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:fldChar"_ostr, "fldCharType"_ostr, "separate");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[5]/w:fldChar"_ostr, "fldCharType"_ostr, "end");
    // form control keeps its direct formatted font size
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr/w:sz"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr/w:szCs"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w:sz"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w:szCs"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[4]/w:rPr/w:sz"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[4]/w:rPr/w:szCs"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[5]/w:rPr/w:sz"_ostr, "val"_ostr, "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[5]/w:rPr/w:szCs"_ostr, "val"_ostr, "20");
}

CPPUNIT_TEST_FIXTURE(Test, Tdf133035)
{
    loadAndSave("tdf133035.docx");
    auto pxml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pxml);
    OUString aXmlVal = getXPath(pxml, "/w:document/w:body/w:p[1]/w:r[1]/w:object/v:shape"_ostr, "style"_ostr);
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
                             xTableRows->getByIndex(0), "TableColumnSeparators")[0]
                             .Position,
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(1), "TableColumnSeparators")[2]
                             .Position);
}

DECLARE_OOXMLEXPORT_TEST(testTdf108350_noFontdefaults, "tdf108350_noFontdefaults.docx")
{
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference< beans::XPropertySet > xStyleProps(paragraphStyles->getByName("NoParent"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Times New Roman"), getProperty<OUString>(xStyleProps, "CharFontName"));
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
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int16>(xShape, "VertOrientRelation"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129522_removeShadowStyle)
{
    loadAndReload("tdf129522_removeShadowStyle.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference< beans::XPropertySet > xStyleProps(paragraphStyles->getByName("Shadow"), uno::UNO_QUERY_THROW);
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xStyleProps, "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    // Shadows were inherited regardless of whether the style disabled them.
    xStyleProps.set(paragraphStyles->getByName("Shadow-removed"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

    uno::Reference< container::XNameAccess > characterStyles = getStyles("CharacterStyles");
    xStyleProps.set(characterStyles->getByName("CharShadow"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "CharShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    xStyleProps.set(characterStyles->getByName("CharShadow-removed"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "CharShadowFormat");
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
        if (xServiceInfo->supportsService("com.sun.star.text.TextField.SetExpression"))
        {
            OUString aValue;
            xPropertySet->getPropertyValue("Content") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("demo"), aValue);
        }
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf83309, "tdf83309.docx")
{
    // Important: bug case had 4 pages
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // First paragraph does not have tab before
    // (same applies to all paragraphs in doc, but lets assume they are
    // behave same way)
    OUString sNodeType = parseDump("(/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*)[1]"_ostr, "type"_ostr);
    CPPUNIT_ASSERT_EQUAL(OUString("PortionType::Text"), sNodeType);

    // tdf148380: creation-date field in header.xml was unsupported on export
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("8/31/14 10:26 AM"), xField->getPresentation(false));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121661)
{
    loadAndSave("tdf121661.docx");
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    assertXPath(pXmlSettings, "/w:settings/w:hyphenationZone"_ostr, "val"_ostr, "851");

    // tdf#149421
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Standard"), uno::UNO_QUERY);
    // This was false
    CPPUNIT_ASSERT_GREATER( static_cast<sal_Int16>(0), getProperty<sal_Int16>(xStyle, "ParaHyphenationZone"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf149421, "tdf121661.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Standard"), uno::UNO_QUERY);
    // This was false
    CPPUNIT_ASSERT_GREATER( static_cast<sal_Int16>(0), getProperty<sal_Int16>(xStyle, "ParaHyphenationZone"));

    if (!isExported())
    {
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int16>(851), getProperty<sal_Int16>(xStyle, "ParaHyphenationZone"));
        // modify hyphenation zone (note: only hyphenation zone set in Standard paragraph style
        // is exported, according to the document-level hyphenation settings of OOXML)
        xStyle->setPropertyValue("ParaHyphenationZone", uno::Any(static_cast<sal_Int16>(2000)));
    }
    else
    {
        // check the export of the modified hyphenation zone
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int16>(2000), getProperty<sal_Int16>(xStyle, "ParaHyphenationZone"));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121658)
{
    loadAndSave("tdf121658.docx");
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    assertXPath(pXmlSettings, "/w:settings/w:doNotHyphenateCaps"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTableStyleConfNested)
{
    loadAndSave("table-style-conf-nested.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed, as the custom table cell
    // border properties were lost, so the outer A2 cell started to have borders, not present in the
    // doc model.
    assertXPath(pXmlDoc, "//w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:tcBorders/w:top"_ostr, "val"_ostr, "nil");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133771)
{
    // Create the doc model.
    createSwDoc("tdf133771.odt", /*pPassword*/ "test");

    CPPUNIT_ASSERT_EQUAL(OUString("Password Protected"), getParagraph(1)->getString());

    // Without the fix in place, this test would have failed with
    // "An uncaught exception of type com.sun.star.io.IOException"
    // exporting to docx
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathContent(pXmlDoc, "//w:body/w:p/w:r/w:t"_ostr, "Password Protected");
}

CPPUNIT_TEST_FIXTURE(Test, testZeroLineSpacing)
{
    // Create the doc model.
    createSwDoc();
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    style::LineSpacing aSpacing;
    aSpacing.Mode = style::LineSpacingMode::MINIMUM;
    aSpacing.Height = 0;
    xParagraph->setPropertyValue("ParaLineSpacing", uno::Any(aSpacing));

    // Export to docx.
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: atLeast
    // - Actual  : auto
    // i.e. the minimal linespacing was lost on export.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing"_ostr, "lineRule"_ostr, "atLeast");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing"_ostr, "line"_ostr, "0");
}

CPPUNIT_TEST_FIXTURE(Test, testSemiTransparentText)
{
    // Create an in-memory empty document.
    createSwDoc();

    // Set text to half-transparent and type a character.
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph.is());
    sal_Int16 nTransparence = 75;
    xParagraph->setPropertyValue("CharTransparence", uno::Any(nTransparence));
    uno::Reference<text::XTextRange> xTextRange(xParagraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextRange.is());
    xTextRange->setString("x");

    // Export to docx.
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    double fValue = getXPath(
            pXmlDoc,
            "/w:document/w:body/w:p/w:r/w:rPr/w14:textFill/w14:solidFill/w14:srgbClr/w14:alpha"_ostr,
            "val"_ostr)
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
        xFactory->createInstance("com.sun.star.text.TextField.User"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMaster(
        xFactory->createInstance("com.sun.star.text.FieldMaster.User"), uno::UNO_QUERY);
    xMaster->setPropertyValue("Name", uno::Any(OUString("foo")));
    xField->attachTextFieldMaster(xMaster);
    xField->getTextFieldMaster()->setPropertyValue("Content", uno::Any(OUString("bar")));
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xField, /*bAbsorb=*/false);

    // Export to docx.
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed, the user field was
    // exported as <w:t>User Field foo = bar</w:t>.
    assertXPathContent(pXmlDoc, "//w:p/w:r[2]/w:instrText"_ostr, " DOCVARIABLE foo ");
    assertXPathContent(pXmlDoc, "//w:p/w:r[4]/w:t"_ostr, "bar");

    // Make sure that not only the variables, but also their values are written.
    pXmlDoc = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//w:docVars/w:docVar"_ostr, "name"_ostr, "foo");
    assertXPath(pXmlDoc, "//w:docVars/w:docVar"_ostr, "val"_ostr, "bar");
}

CPPUNIT_TEST_FIXTURE(Test, testHighlightEdit_numbering)
{
    // Create the doc model.
    createSwDoc("tdf135774_numberingCRProps.docx");

    // This only affects when saving as w:highlight - which is not the default since 7.0.
    bool bWasExportToShade = !officecfg::Office::Common::Filter::Microsoft::Export::CharBackgroundToHighlighting::get();
    auto batch = comphelper::ConfigurationChanges::create();
    officecfg::Office::Common::Filter::Microsoft::Export::CharBackgroundToHighlighting::set(true, batch);
    batch->commit();

    //Simulate a user editing the char background color of the paragraph 2 marker (CR)
    uno::Reference<beans::XPropertySet> properties(getParagraph(2), uno::UNO_QUERY);
    uno::Sequence<beans::NamedValue> aListAutoFormat;
    CPPUNIT_ASSERT(properties->getPropertyValue("ListAutoFormat") >>= aListAutoFormat);
    comphelper::SequenceAsHashMap aMap(properties->getPropertyValue("ListAutoFormat"));
    // change the background color to RES_CHRATR_BACKGROUND.
    aMap["CharBackColor"] <<= static_cast<sal_Int32>(0xff00ff);
    // Two attributes can affect character background. Highlight has priority, and is only there for MS compatibility,
    // so clear any potential highlight set earlier, or override any coming via a style.
    aMap["CharHighlight"] <<= static_cast<sal_Int32>(COL_TRANSPARENT);

    uno::Sequence<beans::PropertyValue> aGrabBag;
    aMap["CharInteropGrabBag"] >>= aGrabBag;
    for (beans::PropertyValue& rProp : asNonConstRange(aGrabBag))
    {
        // The shading is no longer defined from import, so clear that flag.
        // BackColor 0xff00ff will now attempt to export as highlight, since we set that in officecfg.
        if (rProp.Name == "CharShadingMarker")
            rProp.Value <<= false;
    }
    aMap["CharInteropGrabBag"] <<= aGrabBag;

    aMap >> aListAutoFormat;
    properties->setPropertyValue("ListAutoFormat", uno::Any(aListAutoFormat));

    // Export to docx.
    save("Office Open XML Text");

    // Paragraph 2 should have only one w:highlight written per w:rPr. Without the fix, there were two.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "//w:body/w:p[2]/w:pPr/w:rPr/w:highlight"_ostr, "val"_ostr, "none");
    // Visually, the "none" highlight means the bullet point should not have a character background.

    if (bWasExportToShade)
    {
        officecfg::Office::Common::Filter::Microsoft::Export::CharBackgroundToHighlighting::set(false, batch);
        batch->commit();
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132766)
{
    loadAndSave("tdf132766.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/numbering.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Ensure that for list=1 and level=0 we wrote correct bullet char and correct font
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:lvlText"_ostr,
                "val"_ostr, u"\uF0B7"_ustr);
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts"_ostr,
                "ascii"_ostr, "Symbol");
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts"_ostr,
                "hAnsi"_ostr, "Symbol");
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts"_ostr,
                "cs"_ostr, "Symbol");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128245)
{
    loadAndSave("tdf128245.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/numbering.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//w:num[@w:numId='1']/w:abstractNumId"_ostr, "val"_ostr, "1");
    assertXPath(pXmlDoc, "//w:num[@w:numId='2']/w:abstractNumId"_ostr, "val"_ostr, "2");
    assertXPath(pXmlDoc, "//w:num[@w:numId='3']/w:abstractNumId"_ostr, "val"_ostr, "3");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:abstractNumId"_ostr, "val"_ostr, "1");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:lvlOverride[@w:ilvl='0']"_ostr, "ilvl"_ostr, "0");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:lvlOverride/w:startOverride"_ostr, "val"_ostr, "1");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:lvlOverride[@w:ilvl='1']"_ostr, 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf124367, "tdf124367.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    // it was 2761 at the first import, and 2760 at the second import, due to incorrect rounding
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2762),
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(2), "TableColumnSeparators")[0]
                             .Position);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95189)
{
    loadAndReload("tdf95189.docx");
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("3"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(7), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128820)
{
    loadAndSave("tdf128820.fodt");
    // Import of exported DOCX failed because of wrong namespace used for wsp element
    // Now test the exported XML, in case we stop failing opening invalid files
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    // The parent wpg:wgp element has three children: wpg:cNvGrpSpPr, wpg:grpSpPr, and wpg:wsp
    // (if we start legitimately exporting additional children, this needs to be adjusted to check
    // all those, to make sure we don't export wrong elements).
    assertXPathChildren(pXml,
                        "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                        "wp:inline/a:graphic/a:graphicData/wpg:wgp"_ostr,
                        3);
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wpg:cNvGrpSpPr"_ostr);
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wpg:grpSpPr"_ostr);
    // This one was pic:wsp instead of wps:wsp
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128889)
{
    loadAndSave("tdf128889.fodt");
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    // There was an w:r (with w:br) as an invalid child of first paragraph's w:pPr
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:pPr/w:r"_ostr, 0);
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:r"_ostr, 2);
    // Check that the break is in proper - last - position
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:r[2]/w:br"_ostr, "type"_ostr, "page");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132754)
{
    loadAndReload("tdf132754.docx");
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("0.0.0."), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("0.0.1."), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("0.0.2."), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf129353, "tdf129353.docx")
{
    CPPUNIT_ASSERT_EQUAL(8, getParagraphs());
    getParagraph(1, "(Verne, 1870)");
    getParagraph(2, "Bibliography");
    getParagraph(4, "Christie, A. (1922). The Secret Adversary. ");
    CPPUNIT_ASSERT_EQUAL(OUString(), getParagraph(8)->getString());

    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    uno::Reference<text::XDocumentIndex> xIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xIndex->getAnchor();
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
    xTextCursor->gotoRange(xTextRange->getStart(), false);
    xTextCursor->gotoRange(xTextRange->getEnd(), true);
    OUString aIndexString(convertLineEnd(xTextCursor->getString(), LineEnd::LINEEND_LF));

    // Check that all the pre-rendered entries are correct, including trailing spaces
    CPPUNIT_ASSERT_EQUAL(OUString("\n" // starting with an empty paragraph
                                  "Christie, A. (1922). The Secret Adversary. \n"
                                  "\n"
                                  "Verne, J. G. (1870). Twenty Thousand Leagues Under the Sea. \n"
                                  ""), // ending with an empty paragraph
                         aIndexString);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf77796)
{
    loadAndSave("tdf77796.docx");
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    // cell paddings from table style
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:start"_ostr, "w"_ostr, "5");
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:top"_ostr, "w"_ostr, "240");
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:bottom"_ostr, "w"_ostr, "480");
    // not modified
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:end"_ostr, "w"_ostr, "108");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128290)
{
    loadAndSave("tdf128290.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblLayout"_ostr, "type"_ostr, "fixed");

    // ensure unnecessary suppressLineNumbers entry is not created.
    xmlDocUniquePtr pStylesXml = parseExport("word/styles.xml");
    assertXPath(pStylesXml, "//w:style[@w:styleId='Normal']/w:pPr/w:suppressLineNumbers"_ostr, 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf120394, "tdf120394.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("1.2.1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf133605, "tdf133605.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("1.1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(3), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("(a)"), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf133605_2, "tdf133605_2.docx")
{
    // About the same document as tdf133605.docx, but number definition has level definitions in random order
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("1.1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(3), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("(a)"), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123757)
{
    loadAndSave("tdf123757.docx");
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:document/w:body/w:tbl"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141172)
{
    loadAndSave("tdf141172.docx");
    xmlDocUniquePtr pXml = parseExport("word/endnotes.xml");
    CPPUNIT_ASSERT(pXml);
    // This was 1 (lost table during copying endnote content)
    assertXPath(pXml, "/w:endnotes/w:endnote/w:tbl"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141548)
{
    loadAndSave("tdf141548.docx");
    xmlDocUniquePtr pXml = parseExport("word/endnotes.xml");
    CPPUNIT_ASSERT(pXml);
    // This was 0 (lost text content of the run with endnoteRef)
    assertXPath(pXml, "/w:endnotes/w:endnote[4]/w:p/w:r[2]/w:t"_ostr, 2);
    assertXPathContent(pXml, "/w:endnotes/w:endnote[4]/w:p/w:r[2]/w:t[1]"_ostr, "another endnote");
    assertXPathContent(pXml, "/w:endnotes/w:endnote[4]/w:p/w:r[2]/w:t[2]"_ostr, "new line");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143399)
{
    loadAndSave("tdf143399.docx");
    xmlDocUniquePtr pXml = parseExport("word/footnotes.xml");
    CPPUNIT_ASSERT(pXml);
    // These were 0 (lost text content of documents both with footnotes and endnotes)
    assertXPath(pXml, "/w:footnotes/w:footnote[3]/w:p/w:r[3]/w:t"_ostr, 1);
    assertXPathContent(pXml, "/w:footnotes/w:footnote[3]/w:p/w:r[3]/w:t"_ostr, "Footnotes_graphic2");
    assertXPath(pXml, "/w:footnotes/w:footnote[4]/w:p/w:r[3]/w:t"_ostr, 1);
    assertXPathContent(pXml, "/w:footnotes/w:footnote[4]/w:p/w:r[3]/w:t"_ostr, "Footnotes_graphic");

    xmlDocUniquePtr pXml2 = parseExport("word/endnotes.xml");
    CPPUNIT_ASSERT(pXml);
    // This was 0 (lost text content of the run with endnoteRef)
    assertXPath(pXml2, "/w:endnotes/w:endnote[3]/w:p/w:r[3]/w:t"_ostr, 1);
    assertXPathContent(pXml2, "/w:endnotes/w:endnote[3]/w:p/w:r[3]/w:t[1]"_ostr, "Endnotes");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143583)
{
    loadAndSave("tdf143583_emptyParaAtEndOfFootnote.docx");
    xmlDocUniquePtr pXml = parseExport("word/footnotes.xml");
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:footnotes/w:footnote[3]/w:p"_ostr, 2);
    // This was 1
    assertXPath(pXml, "/w:footnotes/w:footnote[4]/w:p"_ostr, 2);
    // This was 2
    assertXPath(pXml, "/w:footnotes/w:footnote[5]/w:p"_ostr, 3);
    // This was 2
    assertXPath(pXml, "/w:footnotes/w:footnote[6]/w:p"_ostr, 3);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152203)
{
    loadAndSave("tdf152203.docx");
    xmlDocUniquePtr pXml = parseExport("word/footnotes.xml");
    CPPUNIT_ASSERT(pXml);

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xLastFootnote(xFootnotes->getByIndex(5), uno::UNO_QUERY);
    // This was "Footnote for pg5" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg 6"), xLastFootnote->getString().trim() );

    uno::Reference<text::XTextRange> xLastButOne(xFootnotes->getByIndex(4), uno::UNO_QUERY);
    // This was "Footnote for pg 6" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg5"), xLastButOne->getString().trim() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152506)
{
    loadAndSave("tdf152506.docx");
    xmlDocUniquePtr pXml = parseExport("word/footnotes.xml");
    CPPUNIT_ASSERT(pXml);

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xLastFootnote(xFootnotes->getByIndex(1), uno::UNO_QUERY);
    // This was "Footnote for pg5" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg 6"), xLastFootnote->getString().trim() );

    uno::Reference<text::XTextRange> xLastButOne(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This was "Footnote for pg 6" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg5"), xLastButOne->getString().trim() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153255)
{
    loadAndSave("tdf153255.docx");
    xmlDocUniquePtr pXml = parseExport("word/footnotes.xml");
    CPPUNIT_ASSERT(pXml);

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xLastFootnote(xFootnotes->getByIndex(5), uno::UNO_QUERY);
    // This was "Footnote for pg2" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg 6"), xLastFootnote->getString().trim() );

    uno::Reference<text::XTextRange> xLastButOne(xFootnotes->getByIndex(4), uno::UNO_QUERY);
    // This was "Footnote for pg 6" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg5"), xLastButOne->getString().trim() );

    // check all the remaining footnotes

    uno::Reference<text::XTextRange> xFootnote1(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This was "Footnote for pg3" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg1"), xFootnote1->getString().trim() );

    uno::Reference<text::XTextRange> xFootnote2(xFootnotes->getByIndex(1), uno::UNO_QUERY);
    // This was "Footnote for pg5" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg2"), xFootnote2->getString().trim() );

    uno::Reference<text::XTextRange> xFootnote3(xFootnotes->getByIndex(2), uno::UNO_QUERY);
    // This was "Footnote for pg4." (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg3"), xFootnote3->getString().trim() );

    uno::Reference<text::XTextRange> xFootnote4(xFootnotes->getByIndex(3), uno::UNO_QUERY);
    // This was "Footnote for pg1" (replaced footnotes)
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg4."), xFootnote4->getString().trim() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153804)
{
    loadAndSave("tdf153804.docx");
    xmlDocUniquePtr pXml = parseExport("word/footnotes.xml");
    CPPUNIT_ASSERT(pXml);

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xLastFootnote(xFootnotes->getByIndex(1), uno::UNO_QUERY);
    // This was empty
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg 6"), xLastFootnote->getString().trim() );

    uno::Reference<text::XTextRange> xLastButOne(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This was empty
    CPPUNIT_ASSERT_EQUAL( OUString("Footnote for pg5"), xLastButOne->getString().trim() );
}

// skip test for macOS (missing fonts?)
#if !defined(MACOSX)
DECLARE_OOXMLEXPORT_TEST(testTdf146346, "tdf146346.docx")
{
    // This was 2 (by bad docDefault vertical margins around tables in footnotes)
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // only first page has table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // check first page: all tables on the first page
    assertXPath(pXmlDoc, "/root/page[1]//anchored/fly"_ostr, 8);
    assertXPath(pXmlDoc, "/root/page[1]//anchored/fly/tab"_ostr, 8);

    // No second page.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. unwanted lower margin in the floating table's anchor paragraph in the footnote created a
    // second page.
    assertXPath(pXmlDoc, "/root/page[2]"_ostr, 0);
}
#endif

DECLARE_OOXMLEXPORT_TEST(testTdf130088, "tdf130088.docx")
{
    // This was 2 (justification without shrinking resulted more lines)
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // check compatibility option in ODT export/import, too
    saveAndReload("writer8");

    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf158436, "tdf158436.docx")
{
    // This resulted freezing
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf159032, "tdf124795-5.docx")
{
    // This resulted crashing
    CPPUNIT_ASSERT_EQUAL(57, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf160518, "tdf160518_useWord2013TrackBottomHyphenation.docx")
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    // TODO: fix export too
    if (isExported())
        return;
    // This was 2 (without shifting last hyphenated line of the page)
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf160518_compatible, "tdf160518_allowHyphenationAtTrackBottom.docx")
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    // TODO: fix export too
    if (isExported())
        return;
    // This is still 2
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf160518_ODT, "tdf160518_useWord2013TrackBottomHyphenation.docx")
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    // TODO: fix export too
    if (isExported())
        return;
    // This was 2 (without shifting last hyphenated line of the page)
    CPPUNIT_ASSERT_EQUAL(3, getPages());

    // check compatibility option in ODT export/import, too
    saveAndReload("writer8");

    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf160518_ODT_compatible, "tdf160518_allowHyphenationAtTrackBottom.docx")
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    // TODO: fix export too
    if (isExported())
        return;
    // This is still 2
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // check compatibility option in ODT export/import, too
    saveAndReload("writer8");

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testHyphenationAuto)
{
    loadAndReload("hyphenation.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Explicitly set hyphenation=auto on document level
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlSettings);
    assertXPath(pXmlSettings, "/w:settings/w:autoHyphenation"_ostr, "val"_ostr, "true");

    // Second paragraph has explicitly enabled hyphenation
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:document/w:body/w:p[2]/w:pPr/w:suppressAutoHyphens"_ostr, "val"_ostr, "false");

    // Default paragraph style explicitly disables hyphens
    xmlDocUniquePtr pXmlStyles = parseExport("word/styles.xml");
    CPPUNIT_ASSERT(pXmlStyles);
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:pPrDefault/w:pPr/w:suppressAutoHyphens"_ostr, "val"_ostr, "true");
}

CPPUNIT_TEST_FIXTURE(Test, testStrikeoutGroupShapeText)
{
    loadAndSave("tdf131776_StrikeoutGroupShapeText.docx");
    // tdf#131776: Check if strikeout is used in shape group texts
    xmlDocUniquePtr pXml = parseExport("word/document.xml");

    // double strike (dstrike)
    //   no "val" attribute
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike"_ostr);
    assertXPathNoAttribute(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike"_ostr, "val"_ostr);
    //   "val" attribute is true, this is used in this test file. However, LO is going to export this element without the "val" attribute
    //   because if the element appears, but without an attribute its "val" true by default.
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike"_ostr);
    assertXPathNoAttribute(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike"_ostr, "val"_ostr);
    //   "val" attribute is false (this was missing, resulting the regression)
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[3]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike"_ostr, "val"_ostr, "false");

    // simple strike (strike)
        //   no "val" attribute
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[4]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike"_ostr);
    assertXPathNoAttribute(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[4]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike"_ostr, "val"_ostr);
    //   "val" attribute is true, this is used in this test file. However, LO is going to export this element without the "val" attribute
    //   because if the element appears, but without an attribute its "val" true by default.
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[5]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike"_ostr);
    assertXPathNoAttribute(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[5]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike"_ostr, "val"_ostr);
    //   "val" attribute is false
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[6]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike"_ostr, "val"_ostr, "false");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131539)
{
    loadAndSave("tdf131539.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    //The positions of OLE objects were not exported, check if now it is exported correctly
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    OUString aXmlVal = getXPath(p_XmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:object/v:shape"_ostr, "style"_ostr);
    // This data was missing
    CPPUNIT_ASSERT(aXmlVal.indexOf("margin-left:139.95")>-1);
}

CPPUNIT_TEST_FIXTURE(Test, testLineWidthRounding)
{
    loadAndSave("tdf126363_LineWidthRounding.docx");
    // tdf#126363: check if line with stays the same after export
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    // this was 57240 (it differs from the original 57150, losing the preset line width)
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln"_ostr, "w"_ostr, "57150");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108505)
{
    loadAndReload("tdf108505.docx");
    uno::Reference<text::XTextRange> xParagraph = getParagraph(3);
    uno::Reference<text::XTextRange> xText
        = getRun(xParagraph, 1, "Wrong font when alone on the line");

    // Without the fix in place this would have become Times New Roman
    CPPUNIT_ASSERT_EQUAL(
        OUString("Trebuchet MS"),
        getProperty<OUString>(xText, "CharFontName"));
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeAnchorHeightFromTopMarginHasHeader)
{
    loadAndReload("tdf123324_testRelativeAnchorHeightFromTopMarginHasHeader.docx");
    // tdf#123324 The height was set relative to page print area top,
    // but this was handled relative to page height.
    // Note: page print area top = margin + header height.
    // In this case the header exists.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "height"_ostr, "2551");
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeAnchorHeightFromTopMarginNoHeader)
{
    loadAndReload("tdf123324_testRelativeAnchorHeightFromTopMarginNoHeader.docx");
    // tdf#123324 The height was set relative from top margin, but this was handled relative from page height.
    // Note: the MSO Word margin = LO margin + LO header height.
    // In this case the header does not exist, so MSO Word margin and LO Writer margin are the same.

    // tdf#123324 The height was set relative to page print area top,
    // but this was handled relative to page height.
    // Note: page print area top = margin + header height.
    // In this case the header does not exist, so OpenDocument and OOXML margins are the same.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "height"_ostr, "2551");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf64531)
{
    loadAndReload("tdf64531.docx");
    xmlDocUniquePtr pXmlDoc= parseExport("word/document.xml");
    OString sPathToTabs= "/w:document/w:body/w:sdt/w:sdtContent/w:p[2]/w:pPr/w:tabs/"_ostr;
    assertXPath(pXmlDoc, sPathToTabs+"w:tab[1]", "pos"_ostr,"720");
    assertXPath(pXmlDoc, sPathToTabs+"w:tab[2]", "pos"_ostr,"12950");
}
/* temporarily disabled to get further test results
   The import now uses the dml shape, not the VML fallback.
DECLARE_OOXMLEXPORT_TEST(testVmlShapeTextWordWrap, "tdf97618_testVmlShapeTextWordWrap.docx")
{
    // tdf#97618 The text wrapping of a shape was not handled in a canvas.
    // TODO: fix export too
    if (isExported())
        return;
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    if (!pXmlDoc)
        return;
    // The bound rect of shape will be wider if wrap does not work (the wrong value is 3167).
    assertXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "width", "2500");
}
*/

DECLARE_OOXMLEXPORT_TEST(testVmlLineShapeMirroredX, "tdf97517_testVmlLineShapeMirroredX.docx")
{
    // tdf#97517 The "flip:x" was not handled for VML line shapes.
    if (!isExported())
        return;
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    OUString sStyle = getXPath(pXmlDoc,
        "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:line"_ostr,
        "style"_ostr);
    CPPUNIT_ASSERT(sStyle.indexOf("flip:x") > 0);
}

DECLARE_OOXMLEXPORT_TEST(testVmlLineShapeMirroredY, "tdf137678_testVmlLineShapeMirroredY.docx")
{
    // tdf#137678 The "flip:y" was not handled for VML line shapes.
    if (!isExported())
        return;
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    OUString sStyle = getXPath(pXmlDoc,
        "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:line"_ostr,
        "style"_ostr);
    CPPUNIT_ASSERT(sStyle.indexOf("flip:y") > 0);
}

CPPUNIT_TEST_FIXTURE(Test, testVmlLineShapeRotated)
{
    loadAndSave("tdf137765_testVmlLineShapeRotated.docx");
    // tdf#137765 The "rotation" (in style attribute) was not handled correctly for VML line shapes.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // it was 1.55pt,279.5pt
    assertXPath(pXmlDoc,
        "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:line"_ostr,
        "from"_ostr,
        "-9pt,296.75pt");
    // it was 25.5pt,317.8pt
    assertXPath(pXmlDoc,
        "/w:document/w:body/w:p[3]/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:line"_ostr,
        "to"_ostr,
        "36.05pt,300.55pt");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
