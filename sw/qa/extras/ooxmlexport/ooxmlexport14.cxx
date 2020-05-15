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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <IDocumentSettingAccess.hxx>

#include <editsh.hxx>
#include <frmatr.hxx>
#include <tools/lineend.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextContentAppend.hpp>

char const DATA_DIRECTORY[] = "/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLIMPORT_TEST(Tdf130907,"tdf130907.docx")
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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf78749, "tdf78749.docx")
{
    //Shape lost the background image before, now check if it still has...
    auto xShape = getShape(1);
    uno::Reference<beans::XPropertySet> xShpProps(xShape, uno::UNO_QUERY);
    OUString aPropertyVal;
    xShpProps->getPropertyValue("FillBitmapName") >>= aPropertyVal;
    CPPUNIT_ASSERT(!aPropertyVal.isEmpty());
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128207, "tdf128207.docx")
{
    //There was the charts on each other, because their horizontal and vertical position was 0!
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPathContent(p_XmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:anchor/wp:positionH/wp:posOffset", "4445");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf123873, "tdf123873.docx")
{
    //OLE Object were overlapped due to missing wrap import
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:drawing/wp:anchor/wp:wrapTopAndBottom");
}

DECLARE_OOXMLEXPORT_TEST(testTdf130814model, "tdf130814.docx")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x1F497D), getProperty<sal_Int32>(getRun(getParagraph(2), 1), "CharColor"));
    CPPUNIT_ASSERT_EQUAL(double(16), getProperty<double>(getRun(getParagraph(2), 1), "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::SINGLE, getProperty<sal_Int16>(getRun(getParagraph(2), 1), "CharUnderline"));
    CPPUNIT_ASSERT_EQUAL(OUString("Candara"), getProperty<OUString>(getRun(getParagraph(2), 1), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Arial Unicode MS"), getProperty<OUString>(getRun(getParagraph(2), 1), "CharFontNameAsian"));
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf107020, "tdf107020.docx")
{
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect", "l", "4910");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect", "t", "27183");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect", "r", "57638");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:blipFill/a:srcRect", "b", "48360");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf130814ooxml, "tdf130814.docx")
{
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:rFonts", "eastAsia", "Arial Unicode MS");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:rFonts", "ascii", "Candara");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:rFonts", "hAnsi", "Candara");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:color", "val", "1F497D");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:sz", "val", "32");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:szCs", "val", "32");
    assertXPath(
        p_XmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w:u", "val", "single");
}

DECLARE_OOXMLIMPORT_TEST(testTdf129888vml, "tdf129888vml.docx")
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

DECLARE_OOXMLIMPORT_TEST(testTdf129888dml, "tdf129888dml.docx")
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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf130120, "tdf130120.docx")
{
    //Text for exporting the allowincell attribute:
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    assertXPath(p_XmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r/mc:AlternateContent/"
        "mc:Choice/w:drawing/wp:anchor", "layoutInCell", "0");
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
    assertXPathContent(pDump, "/root/page[1]/body/txt[2]", "First line");
    const sal_Int32 nLevel1Margin = getXPath(pDump, "//page[1]/body/txt[2]/infos/prtBounds", "left").toInt32();
    assertXPathContent(pDump, "/root/page[1]/body/txt[4]", "One sublevel");
    const sal_Int32 nLevel2Margin = getXPath(pDump, "//page[1]/body/txt[4]/infos/prtBounds", "left").toInt32();
    CPPUNIT_ASSERT( nLevel1Margin < nLevel2Margin );
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
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO),
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
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO3),
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
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO4),
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
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO5),
                         aMap["NumberingType"].get<sal_uInt16>());
}

CPPUNIT_TEST_FIXTURE(Test, testArabicZeroNumberingFootnote)
{
    // Create a document, set footnote numbering type to ARABIC_ZERO.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::ARABIC_ZERO;
    xFootnoteSettings->setPropertyValue("NumberingType", uno::makeAny(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    reload("Office Open XML Text", "");

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt' number of nodes is incorrect
    // because the exporter had no idea what markup to use for ARABIC_ZERO.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt", "val", "decimalZero");
}

CPPUNIT_TEST_FIXTURE(Test, testChicagoNumberingFootnote)
{
    // Create a document, set footnote numbering type to SYMBOL_CHICAGO.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::SYMBOL_CHICAGO;
    xFootnoteSettings->setPropertyValue("NumberingType", uno::makeAny(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    reload("Office Open XML Text", "");

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt' number of nodes is incorrect
    // because the exporter had no idea what markup to use for SYMBOL_CHICAGO.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt", "val", "chicago");
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

DECLARE_OOXMLEXPORT_TEST(testTdf130610, "tdf130610_bold_in_2_styles.ott")
{
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
        xmlDocUniquePtr pXmlDoc =parseExport("word/document.xml");
        if (pXmlDoc)
        {
            assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:rPr/w:b");
        }
    }
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf121045, "tdf121045.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:sz", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:szCs", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[1]/w:fldChar", "fldCharType", "begin");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:instrText", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[3]/w:fldChar", "fldCharType", "separate");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[5]/w:fldChar", "fldCharType", "end");
    // form control keeps its direct formatted font size
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:rPr/w:sz", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:rPr/w:szCs", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[3]/w:rPr/w:sz", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[3]/w:rPr/w:szCs", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[4]/w:rPr/w:sz", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[4]/w:rPr/w:szCs", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[5]/w:rPr/w:sz", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[5]/w:rPr/w:szCs", "val", "20");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf92472, "tdf92472.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:sz", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:szCs", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:fldChar", "fldCharType", "begin");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:instrText", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:fldChar", "fldCharType", "separate");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[5]/w:fldChar", "fldCharType", "end");
    // form control keeps its direct formatted font size
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr/w:sz", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr/w:szCs", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w:sz", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w:szCs", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[4]/w:rPr/w:sz", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[4]/w:rPr/w:szCs", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[5]/w:rPr/w:sz", "val", "20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[5]/w:rPr/w:szCs", "val", "20");
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

DECLARE_OOXMLEXPORT_TEST(testTdf123116_oversizedRowSplit, "tdf123116_oversizedRowSplit.odt")
{
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

DECLARE_OOXMLEXPORT_TEST(testTdf129522_removeShadowStyle, "tdf129522_removeShadowStyle.odt")
{
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
    CPPUNIT_ASSERT(xNameAccess->getCount() < 4);
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038, "tdf125038.docx")
{
    OUString aActual = getParagraph(1)->getString();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: phone:...
    // - Actual  : result1result2phone:...
    // i.e. the result if the inner MERGEFIELD fields ended up in the body text.
    CPPUNIT_ASSERT_EQUAL(OUString("phone: \t1234567890"), aActual);
}

DECLARE_OOXMLIMPORT_TEST(testTdf124986, "tdf124986.docx")
{
    // Load a document with SET fields, where the SET fields contain leading/trailing quotation marks and spaces.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        OUString aValue;
        if (xServiceInfo->supportsService("com.sun.star.text.TextField.SetExpression"))
        {
            xPropertySet->getPropertyValue("Content") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("demo"), aValue);
        }
    }
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038b, "tdf125038b.docx")
{
    // Load a document with an IF field, where the IF field command contains a paragraph break.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphAccess->createEnumeration();
    CPPUNIT_ASSERT(xParagraphs->hasMoreElements());
    uno::Reference<text::XTextRange> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: phone: 1234
    // - Actual  :
    // i.e. the first paragraph was empty and the second paragraph had the content.
    CPPUNIT_ASSERT_EQUAL(OUString("phone: 1234"), xParagraph->getString());
    CPPUNIT_ASSERT(xParagraphs->hasMoreElements());
    xParagraphs->nextElement();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: !xParagraphs->hasMoreElements()
    // i.e. the document had 3 paragraphs, while only 2 was expected.
    CPPUNIT_ASSERT(!xParagraphs->hasMoreElements());
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038c, "tdf125038c.docx")
{
    OUString aActual = getParagraph(1)->getString();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: email: test@test.test
    // - Actual  : email:
    // I.e. the result of the MERGEFIELD field inside an IF field was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("email: test@test.test"), aActual);
}

DECLARE_OOXMLEXPORT_TEST(testTdf121661, "tdf121661.docx")
{
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    if (!pXmlSettings)
        return;
    assertXPath(pXmlSettings, "/w:settings/w:hyphenationZone", "val", "851");
}

DECLARE_OOXMLEXPORT_TEST(testTdf121658, "tdf121658.docx")
{
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    if (!pXmlSettings)
        return;
    assertXPath(pXmlSettings, "/w:settings/w:doNotHyphenateCaps");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testTableStyleConfNested)
{
    // Create the doc model.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "table-style-conf-nested.docx";
    loadURL(aURL, nullptr);

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    validate(maTempFile.GetFileName(), test::OOXML);
    mbExported = true;
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed, as the custom table cell
    // border properties were lost, so the outer A2 cell started to have borders, not present in the
    // doc model.
    assertXPath(pXmlDoc, "//w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:tcBorders/w:top", "val", "nil");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testZeroLineSpacing)
{
    // Create the doc model.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    style::LineSpacing aSpacing;
    aSpacing.Mode = style::LineSpacingMode::MINIMUM;
    aSpacing.Height = 0;
    xParagraph->setPropertyValue("ParaLineSpacing", uno::makeAny(aSpacing));

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mbExported = true;
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: atLeast
    // - Actual  : auto
    // i.e. the minimal linespacing was lost on export.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing", "lineRule", "atLeast");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing", "line", "0");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testSemiTransparentText)
{
    // Create an in-memory empty document.
    loadURL("private:factory/swriter", nullptr);

    // Set text to half-transparent and type a character.
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph.is());
    sal_Int16 nTransparence = 75;
    xParagraph->setPropertyValue("CharTransparence", uno::makeAny(nTransparence));
    uno::Reference<text::XTextRange> xTextRange(xParagraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextRange.is());
    xTextRange->setString("x");

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mbExported = true;
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    OString aXPath
        = "/w:document/w:body/w:p/w:r/w:rPr/w14:textFill/w14:solidFill/w14:srgbClr/w14:alpha";
    double fValue = getXPath(pXmlDoc, aXPath, "val").toDouble();
    sal_Int16 nActual = basegfx::fround(fValue / oox::drawingml::PER_PERCENT);

    // Without the accompanying fix in place, this test would have failed, as the w14:textFill
    // element was missing.
    CPPUNIT_ASSERT_EQUAL(nTransparence, nActual);
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testUserField)
{
    // Create an in-memory empty document with a user field.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XDependentTextField> xField(
        xFactory->createInstance("com.sun.star.text.TextField.User"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMaster(
        xFactory->createInstance("com.sun.star.text.FieldMaster.User"), uno::UNO_QUERY);
    xMaster->setPropertyValue("Name", uno::makeAny(OUString("foo")));
    xField->attachTextFieldMaster(xMaster);
    xField->getTextFieldMaster()->setPropertyValue("Content", uno::makeAny(OUString("bar")));
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xField, /*bAbsorb=*/false);

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    validate(maTempFile.GetFileName(), test::OOXML);
    mbExported = true;
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed, the user field was
    // exported as <w:t>User Field foo = bar</w:t>.
    assertXPathContent(pXmlDoc, "//w:p/w:r[2]/w:instrText", " DOCVARIABLE foo ");
    assertXPathContent(pXmlDoc, "//w:p/w:r[4]/w:t", "bar");

    // Make sure that not only the variables, but also their values are written.
    pXmlDoc = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//w:docVars/w:docVar", "name", "foo");
    assertXPath(pXmlDoc, "//w:docVars/w:docVar", "val", "bar");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf132766, "tdf132766.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/numbering.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Ensure that for list=1 and level=0 we wrote correct bullet char and correct font
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:lvlText",
                "val", u"\uF0B7");
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts",
                "ascii", "Symbol");
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts",
                "hAnsi", "Symbol");
    assertXPath(pXmlDoc, "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:rFonts",
                "cs", "Symbol");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128245, "tdf128245.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/numbering.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//w:num[@w:numId='1']/w:abstractNumId", "val", "1");
    assertXPath(pXmlDoc, "//w:num[@w:numId='2']/w:abstractNumId", "val", "2");
    assertXPath(pXmlDoc, "//w:num[@w:numId='3']/w:abstractNumId", "val", "3");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:abstractNumId", "val", "1");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:lvlOverride[@w:ilvl='0']", "ilvl", "0");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:lvlOverride/w:startOverride", "val", "1");
    assertXPath(pXmlDoc, "//w:num[@w:numId='4']/w:lvlOverride[@w:ilvl='1']", 0);
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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf95189, "tdf95189.docx")
{
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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128820, "tdf128820.fodt")
{
    // Import of exported DOCX failed because of wrong namespace used for wsp element
    // Now test the exported XML, in case we stop failing opening invalid files
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    // The parent wpg:wgp element has three children: wpg:cNvGrpSpPr, wpg:grpSpPr, and wpg:wsp
    // (if we start legitimately exporting additional children, this needs to be adjusted to check
    // all those, to make sure we don't export wrong elements).
    assertXPathChildren(pXml,
                        "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/"
                        "wp:inline/a:graphic/a:graphicData/wpg:wgp",
                        3);
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wpg:cNvGrpSpPr");
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wpg:grpSpPr");
    // This one was pic:wsp instead of wps:wsp
    assertXPath(pXml,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/"
                "a:graphic/a:graphicData/wpg:wgp/wps:wsp");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128889, "tdf128889.fodt")
{
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    // There was an w:r (with w:br) as an invalid child of first paragraph's w:pPr
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:pPr/w:r", 0);
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:r", 2);
    // Check that the break is in proper - last - position
    assertXPath(pXml, "/w:document/w:body/w:p[1]/w:r[2]/w:br", "type", "page");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf132754, "tdf132754.docx")
{
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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf77796, "tdf77796.docx")
{
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    // cell paddings from table style
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:start", "w", "5");
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:top", "w", "240");
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:bottom", "w", "480");
    // not modified
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblCellMar/w:end", "w", "108");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf128290, "tdf128290.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:document/w:body/w:tbl/w:tblPr/w:tblLayout", "type", "fixed");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf123757, "tdf123757.docx")
{
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:document/w:body/w:tbl", 2);
}

DECLARE_OOXMLEXPORT_TEST(testContSectBreakHeaderFooter, "cont-sect-break-header-footer.docx")
{
    // Load a document with a continuous section break on page 2.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 1"),
                         parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer, section 1"),
                         parseDump("/root/page[1]/footer/txt/text()"));
    // Make sure the header stays like this; if we naively just update the page style name of the
    // first para on page 2, then this would be 'Header, section 2', which is incorrect.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 2"),
                         parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer, section 2"),
                         parseDump("/root/page[2]/footer/txt/text()"));
    // This is inherited from page 2.
    CPPUNIT_ASSERT_EQUAL(OUString("Header, section 2"),
                         parseDump("/root/page[3]/header/txt/text()"));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - xpath should match exactly 1 node
    // i.e. the footer had no text (inherited from page 2), while the correct behavior is to provide
    // the own footer text.
    CPPUNIT_ASSERT_EQUAL(OUString("Footer, section 3"),
                         parseDump("/root/page[3]/footer/txt/text()"));

    // Without the export fix in place, the import-export-import test would have failed with:
    // - Expected: Header, section 2
    // - Actual  : First page header, section 2
    // i.e. both the header and the footer on page 3 was wrong.

    // Additional problem: top margin on page 3 was wrong.
    if (mbExported)
    {
        xmlDocUniquePtr pXml = parseExport("word/document.xml");
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 2200
        // - Actual  : 2574
        // i.e. the top margin on page 3 was too large and now matches the value from the input
        // document.
        assertXPath(pXml, "/w:document/w:body/w:sectPr/w:pgMar", "top", "2200");
    }
}

DECLARE_OOXMLEXPORT_TEST(testHyphenationAuto, "hyphenation.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Explicitly set hyphenation=auto on document level
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlSettings);
    assertXPath(pXmlSettings, "/w:settings/w:autoHyphenation", "val", "true");

    // Second paragraph has explicitly enabled hyphenation
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "/w:document/w:body/w:p[2]/w:pPr/w:suppressAutoHyphens", "val", "false");

    // Default paragraph style explicitly disables hyphens
    xmlDocUniquePtr pXmlStyles = parseExport("word/styles.xml");
    CPPUNIT_ASSERT(pXmlStyles);
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:pPrDefault/w:pPr/w:suppressAutoHyphens", "val", "true");
}

DECLARE_OOXMLEXPORT_TEST(testStrikeoutGroupShapeText, "tdf131776_StrikeoutGroupShapeText.docx")
{
    // tdf#131776: Check if strikeout is used in shape group texts
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    if (!pXml)
        return;

    // double strike (dstrike)
    //   no "val" attribute
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike");
    assertXPathNoAttribute(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike", "val");
    //   "val" attribute is true, this is used in this test file. However, LO is going to export this element without the "val" attribute
    //   because if the element appears, but without an attribute its "val" true by default.
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike");
    assertXPathNoAttribute(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike", "val");
    //   "val" attribute is false (this was missing, resulting the regression)
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[3]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:dstrike", "val", "false");

    // simple strike (strike)
        //   no "val" attribute
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[4]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike");
    assertXPathNoAttribute(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[4]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike", "val");
    //   "val" attribute is true, this is used in this test file. However, LO is going to export this element without the "val" attribute
    //   because if the element appears, but without an attribute its "val" true by default.
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[5]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike");
    assertXPathNoAttribute(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[5]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike", "val");
    //   "val" attribute is false
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp/"
        "wps:wsp[6]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:strike", "val", "false");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf131539, "tdf131539.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    //The positions of OLE objects were not exported, check if now it is exported correctly
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    OUString aXmlVal = getXPath(p_XmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:object/v:shape", "style");
    // This data was missing
    CPPUNIT_ASSERT(aXmlVal.indexOf("margin-left:139.95")>-1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
