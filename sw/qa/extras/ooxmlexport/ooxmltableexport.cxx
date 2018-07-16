/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <string>
#include <bordertest.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

protected:
};

DECLARE_OOXMLEXPORT_TEST(testShapeInFloattable, "shape-in-floattable.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // No nested drawingML w:txbxContent.
        assertXPath(pXmlDoc, "//mc:Choice//w:txbxContent//w:txbxContent", 0);
        // Instead, make sure we have a separate shape and group shape:
        assertXPath(pXmlDoc, "//mc:AlternateContent//mc:Choice[@Requires='wps']", 1);
        assertXPath(pXmlDoc, "//mc:AlternateContent//mc:Choice[@Requires='wpg']", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testDropdownInCell, "dropdown-in-cell.docx")
{
    // First problem: table was missing from the document, this was 0.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // Second problem: dropdown shape wasn't anchored inside the B1 cell.
    uno::Reference<text::XTextContent> xShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xAnchor = xShape->getAnchor();
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRangeCompare> xTextRangeCompare(xCell, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextRangeCompare->compareRegionStarts(xAnchor, xCell));
}

DECLARE_OOXMLEXPORT_TEST(testTableAlignment, "table-alignment.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was LEFT_AND_WIDTH, i.e. table alignment wasn't imported correctly.
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT,
                         getProperty<sal_Int16>(xTable, "HoriOrient"));
}

DECLARE_OOXMLEXPORT_TEST(testTableStyleCellBackColor, "table-style-cell-back-color.docx")
{
    // The problem was that cell background was white, not green.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    // This was 0xffffff.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00ff00), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTableStyleBorder, "table-style-border.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // This was 0, the second cell was missing its right border.
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A2");
    CPPUNIT_ASSERT(getProperty<table::BorderLine2>(xCell, "RightBorder").LineWidth > 0);

    // This was also 0 (even after fixing the previous problem), the first cell was missing its right border, too.
    xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT(getProperty<table::BorderLine2>(xCell, "RightBorder").LineWidth > 0);
}

DECLARE_OOXMLEXPORT_TEST(testTableStyleBorderExport, "table-style-border-export.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A3");
    // Bottom border was white, so this was 0xffffff.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x8064A2),
                         getProperty<table::BorderLine2>(xCell, "BottomBorder").Color);
}

DECLARE_OOXMLEXPORT_TEST(testTextboxTable, "textbox-table.docx")
{
    // We wrote not-well-formed XML during export for this one:
    // Shape with textbox, having a table and also anchored inside a table.

    // Just make sure that we have both tables.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTableRtl, "table-rtl.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was text::WritingMode2::LR_TB, i.e. direction of the table was ignored.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>(xTable, "WritingMode"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf97090, "tdf97090.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x95B3D7),
                         getProperty<sal_Int32>(xTable->getCellByName("A1"), "BackColor"));

    uno::Reference<container::XEnumerationAccess> paraEnumAccess(xTable->getCellByName("A1"),
                                                                 uno::UNO_QUERY);
    assert(paraEnumAccess.is());
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();

    assert(paraEnum.is());
    uno::Reference<beans::XPropertySet> paragraphProperties(paraEnum->nextElement(),
                                                            uno::UNO_QUERY);
    assert(paragraphProperties.is());
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE,
                         getProperty<drawing::FillStyle>(paragraphProperties, "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xffffff),
                         getProperty<sal_Int32>(paragraphProperties, "FillColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf79639, "tdf79639.docx")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 0, floating table in header wasn't converted to a TextFrame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDrawPage->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf44986, "tdf44986.docx")
{
    // Check that the table at the second paragraph.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // Check the first row of the table, it should have two cells (one separator).
    // This was 0: the first row had no separators, so it had only one cell, which was too wide.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                           xTableRows->getByIndex(0), "TableColumnSeparators")
                                           .getLength());
}

DECLARE_OOXMLEXPORT_TEST(testTable, "table.odt")
{
    // Validation test: order of elements were wrong.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Order was: insideH, end, insideV.
    int nEnd = getXPathPosition(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblBorders", "end");
    int nInsideH
        = getXPathPosition(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblBorders", "insideH");
    int nInsideV
        = getXPathPosition(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblBorders", "insideV");
    CPPUNIT_ASSERT(nEnd < nInsideH);
    CPPUNIT_ASSERT(nInsideH < nInsideV);

    // Make sure we write qFormat for well-known style names.
    assertXPath(parseExport("word/styles.xml"), "//w:style[@w:styleId='Normal']/w:qFormat", 1);
}

struct SingleLineBorders
{
    sal_Int16 top, bottom, left, right;
    SingleLineBorders(int t = 0, int b = 0, int l = 0, int r = 0)
        : top(t)
        , bottom(b)
        , left(l)
        , right(r)
    {
    }
    sal_Int16 getBorder(int i) const
    {
        switch (i)
        {
            case 0:
                return top;
            case 1:
                return bottom;
            case 2:
                return left;
            case 3:
                return right;
            default:
                assert(false);
                return 0;
        }
    }
};

DECLARE_OOXMLEXPORT_TEST(testTableBorders, "table-borders.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);

    std::map<OUString, SingleLineBorders> cellBorders;
    cellBorders[OUString("A1")] = SingleLineBorders(106, 106, 106, 106);
    cellBorders[OUString("B1")] = SingleLineBorders(106, 0, 106, 35);
    cellBorders[OUString("C1")] = SingleLineBorders(106, 106, 35, 106);
    cellBorders[OUString("A2")] = SingleLineBorders(106, 35, 106, 0);
    cellBorders[OUString("B2")] = SingleLineBorders(0, 0, 0, 0);
    cellBorders[OUString("C2")] = SingleLineBorders(106, 106, 0, 106);
    cellBorders[OUString("A3")] = SingleLineBorders(35, 35, 106, 106);
    cellBorders[OUString("B3")] = SingleLineBorders(0, 106, 106, 106);
    cellBorders[OUString("C3")] = SingleLineBorders(106, 106, 106, 106);
    cellBorders[OUString("A4")] = SingleLineBorders(35, 106, 106, 35);
    cellBorders[OUString("B4")] = SingleLineBorders(106, 106, 35, 106);
    cellBorders[OUString("C4")] = SingleLineBorders(106, 106, 106, 106);

    const OUString borderNames[] = {
        OUString("TopBorder"),
        OUString("BottomBorder"),
        OUString("LeftBorder"),
        OUString("RightBorder"),
    };

    uno::Sequence<OUString> const cells = xTextTable->getCellNames();
    sal_Int32 nLength = cells.getLength();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(cellBorders.size()), nLength);

    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        uno::Reference<table::XCell> xCell = xTextTable->getCellByName(cells[i]);
        uno::Reference<beans::XPropertySet> xPropSet(xCell, uno::UNO_QUERY_THROW);
        const SingleLineBorders& borders = cellBorders[cells[i]];

        for (sal_Int32 j = 0; j < 4; ++j)
        {
            uno::Any aBorder = xPropSet->getPropertyValue(borderNames[j]);
            table::BorderLine aBorderLine;
            if (aBorder >>= aBorderLine)
            {
                std::stringstream message;
                message << cells[i] << "'s " << borderNames[j] << " is incorrect";
                CPPUNIT_ASSERT_EQUAL_MESSAGE(message.str(), borders.getBorder(j),
                                             aBorderLine.OuterLineWidth);
            }
        }
    }
}

/*
* doesn't work on openSUSE12.2 at least
DECLARE_OOXMLEXPORT_TEST(test1Table1Page, "1-table-1-page.docx")
{
// 2 problem for this document after export:
//   - invalid sectPr inserted at the beginning of the page
//   - font of empty cell is not preserved, leading to change in rows height
uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
xCursor->jumpToLastPage();
CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}
*/

DECLARE_OOXMLEXPORT_TEST(testTableStylerPrSz, "table-style-rPr-sz.docx")
{
    // Verify that font size inside the table is 20pt, despite the sz attribute in the table size.
    // Also check that other rPr attribute are used: italic, bold, underline
    // Office has the same behavior
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(20.f, getProperty<float>(getRun(xPara, 1), "CharHeight"));
    //    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::SINGLE, getProperty<short>(getRun(xPara, 1), "CharUnderline"));
    //    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(xPara, 1), "CharWeight"));
    //    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, getProperty<awt::FontSlant>(getRun(xPara, 1), "CharPosture"));
}

DECLARE_OOXMLEXPORT_TEST(testTableFloating, "table-floating.docx")
{
    // Both the size and the position of the table was incorrect.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // Second table was too wide: 16249, i.e. as wide as the first table.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11248), getProperty<sal_Int32>(xTables->getByIndex(1), "Width"));

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    if (xIndexAccess->getCount())
    {
        // After import, table is inside a TextFrame.
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        // This was 0, should be the opposite of (left margin + half of the border width).
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-199), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
        // Was 0 as well, should be the right margin.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xFrame, "RightMargin"));
    }
    else
    {
        // After import, table is inside a TextFrame.
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        // This was 0, should be the opposite of (left margin + half of the border width).
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-199), getProperty<sal_Int32>(xShape, "HoriOrientPosition"));
        // Was 0 as well, should be the right margin.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xShape, "RightMargin"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testCellBtlr, "cell-btlr.docx")
{
    /*
    * The problem was that the exporter didn't mirror the workaround of the
    * importer, regarding the btLr text direction: the <w:textDirection
    * w:val="btLr"/> token was completely missing in the output.
    */

    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tcPr/w:textDirection", "val",
                "btLr");
}

DECLARE_OOXMLEXPORT_TEST(testCalendar1, "calendar1.docx")
{
    // Document has a non-trivial table style, test the roundtrip of it.
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:basedOn", "val",
                "TableNormal");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:rsid", "val", "00903003");
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='Calendar1']/w:tblPr/w:tblStyleColBandSize", "val",
                "1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tcPr/w:shd", "val",
                "clear");

    // Table style lost its paragraph / run properties.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:pPr/w:spacing", "lineRule",
                "auto");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:rPr/w:lang", "eastAsia",
                "ja-JP");

    // Table style lost its conditional table formatting properties.
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:pPr/"
                "w:wordWrap",
                1);
    assertXPath(
        pXmlStyles,
        "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:rFonts",
        "hAnsiTheme", "minorHAnsi");
    assertXPath(
        pXmlStyles,
        "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:tblPr", 1);
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:tcPr/"
                "w:vAlign",
                "val", "bottom");
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='lastRow']/w:tcPr/"
                "w:tcBorders/w:tr2bl",
                "val", "nil");
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='band2Horz']/"
                "w:tcPr/w:tcBorders/w:top",
                "themeColor", "text1");

    // w:tblLook element and its attributes were missing.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "firstRow", "1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "lastRow", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "lastColumn", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "firstColumn", "1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "noHBand", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "noVBand", "1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "val", "04a0");
}

DECLARE_OOXMLEXPORT_TEST(testCalendar2, "calendar2.docx")
{
    // Problem was that CharCaseMap was style::CaseMap::NONE.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        style::CaseMap::UPPERCASE,
        getProperty<sal_Int16>(getRun(getParagraphOfText(1, xCell->getText()), 1), "CharCaseMap"));
    // Font size in the second row was 11.
    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        16.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()), 1), "CharHeight"));
    // Font size in the third row was 11 as well.
    xCell.set(xTable->getCellByName("B3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        14.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()), 1), "CharHeight"));

    // This paragraph property was missing in table style.
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:pPr/w:jc", "val",
                "center");

    // These run properties were missing
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:rPr/w:lang", "val",
                "en-US");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:rPr/w:lang", "bidi",
                "ar-SA");
    assertXPath(
        pXmlStyles,
        "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:caps",
        1);
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/"
                "w:smallCaps",
                "val", "0");
    assertXPath(
        pXmlStyles,
        "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:color",
        "themeColor", "accent1");
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/"
                "w:spacing",
                "val", "20");

    // Table borders were also missing
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='Calendar2']/w:tblPr/w:tblBorders/w:insideV",
                "themeTint", "99");
}

DECLARE_OOXMLEXPORT_TEST(testTcBorders, "testTcBorders.docx")
{
    //fdo#76635 : Table borders are not getting preserved.

    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    assertXPath(pXmlDocument,
                "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/"
                "w:bottom[1][@w:val = 'single']",
                1);
    assertXPath(pXmlDocument,
                "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/"
                "w:bottom[1][@w:sz = 4]",
                1);
    assertXPath(pXmlDocument,
                "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/"
                "w:bottom[1][@w:space = 0]",
                1);
    assertXPath(pXmlDocument,
                "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/"
                "w:bottom[1][@w:color = 808080]",
                1);
}

DECLARE_OOXMLEXPORT_TEST(testQuicktables, "quicktables.docx")
{
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;

    // These were missing in the Calendar3 table style.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:rPr/w:rFonts", "cstheme",
                "majorBidi");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:rPr/w:color", "themeTint",
                "80");
    CPPUNIT_ASSERT(getXPath(pXmlStyles,
                            "/w:styles/w:style[@w:styleId='Calendar3']/"
                            "w:tblStylePr[@w:type='firstRow']/w:rPr/w:color",
                            "themeShade")
                       .equalsIgnoreAsciiCase("BF"));

    // Calendar4.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:pPr/w:snapToGrid", "val",
                "0");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:rPr/w:bCs", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tcPr/w:shd", "themeFill",
                "accent1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tcPr/w:shd",
                "themeFillShade", "80");
    assertXPath(
        pXmlStyles,
        "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='firstCol']/w:pPr/w:ind",
        "rightChars", "0");
    assertXPath(
        pXmlStyles,
        "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='firstCol']/w:pPr/w:ind",
        "right", "144");
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='band2Horz']/"
                "w:tcPr/w:tcMar/w:bottom",
                "w", "86");

    // LightList.
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='LightList']/w:tblStylePr[@w:type='firstRow']/w:pPr/"
                "w:spacing",
                "before", "0");

    // MediumList2-Accent1.
    CPPUNIT_ASSERT(getXPath(pXmlStyles,
                            "/w:styles/w:style[@w:styleId='MediumList2-Accent1']/"
                            "w:tblStylePr[@w:type='band1Vert']/w:tcPr/w:shd",
                            "themeFillTint")
                       .equalsIgnoreAsciiCase("3F"));

    // MediumShading2-Accent5.
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='MediumShading2-Accent5']/"
                "w:tblStylePr[@w:type='firstRow']/w:tcPr/w:tcBorders/w:top",
                "color", "auto");
}

DECLARE_OOXMLEXPORT_TEST(testFdo69644, "fdo69644.docx")
{
    // The problem was that the exporter exported the table definition
    // with only 3 columns, instead of 5 columns.
    // Check that the table grid is exported with 5 columns
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol", 5);
}

DECLARE_OOXMLEXPORT_TEST(testCellGridSpan, "cell-grid-span.docx")
{
    // The problem was during export gridSpan value for 1st & 2nd cells for test document
    // used to get set wrongly to 5 and 65532 respectively which was the reason for crash during save operation
    // Verifying gridSpan element is not present in RoundTripped Document (As it's Default value is 1).
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:tcPr/w:gridSpan", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:gridSpan", 0);
}

DECLARE_OOXMLEXPORT_TEST(testSpacingLineRule, "table_lineRule.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p/w:pPr/w:spacing", "lineRule",
                "auto");
}

DECLARE_OOXMLEXPORT_TEST(testTableLineSpacing, "table_atleast.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:pPr/w:spacing", "line", "320");
}

DECLARE_OOXMLEXPORT_TEST(testTableThemePreservation, "table-theme-preservation.docx")
{
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    // check cell theme colors have been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:shd", "themeFill",
                "accent6");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:shd",
                "themeFillShade", "");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:shd",
                "themeFillTint", "33");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:shd", "themeFill",
                "accent6");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:shd",
                "themeFillShade", "");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:shd",
                "themeFillTint", "");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "themeFill",
                "accent6");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd",
                "themeFillShade", "80");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd",
                "themeFillTint", "");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "val",
                "horzStripe");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "themeColor",
                "accent3");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "themeTint",
                "33");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "color",
                "E7EEEE");

    // check table style has been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tblPr/w:tblStyle", "val",
                "Sombreadoclaro-nfasis1");
    // check table style is not overwritten by other properties
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[2]/w:tcPr/w:tcBorders/*", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[3]/w:tcPr/w:tcBorders/*", 0);
    // check that one cell attribute present in the original document has been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/*", 1);

    // Check that w:cnfStyle row, cell and paragraph property is preserved.
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle", "val", "100000000000");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle", "firstRow", "1");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle", "lastRow", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle", "firstColumn", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle", "lastColumn", "0");

    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle", "val", "001000000000");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle", "oddVBand", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle", "evenVBand", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle", "oddHBand", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle", "evenHBand", "0");

    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle", "val",
                "100000000000");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle", "firstRowFirstColumn",
                "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle", "firstRowLastColumn",
                "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle", "lastRowFirstColumn",
                "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle", "lastRowLastColumn",
                "0");
}

DECLARE_OOXMLEXPORT_TEST(testcantSplit, "2_table_doc.docx")
{
    // if Split table value is true for a table then during export do not write <w:cantSplit w:val="false"/>
    // in table row property,As default row prop is allow row to break across page.
    // writing <w:cantSplit w:val="false"/> during export was causing problem that all the cell data used to come on same page
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr/w:trPr/w:cantSplit", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr/w:trPr/w:cantSplit", "val", "true");
}

DECLARE_OOXMLEXPORT_TEST(testDontSplitTable, "tdf101589_dontSplitTable.odt")
{
    //single row tables need to prevent split by setting row to no split
    CPPUNIT_ASSERT_EQUAL(OUString("Row 1"),
                         parseDump("/root/page[2]/body/tab[1]/row[1]/cell[1]/txt[1]"));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTable, "Split"));

    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTableRows->getByIndex(0), "IsSplitAllowed"));
}

DECLARE_OOXMLEXPORT_TEST(testExtraSectionBreak, "1_page.docx")
{
    // There was a problem for some documents during export.Invalid sectPr getting added
    // because of faulty calculation of PageDesc value
    // This was the reason for increasing number of pages after RT
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testFdo71785, "fdo71785.docx")
{
    // crashtest
}

DECLARE_OOXMLEXPORT_TEST(testCrashWhileSave, "testCrashWhileSave.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/footer1.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:ftr/w:tbl/w:tr/w:tc[1]/w:p[1]/w:pPr/w:pStyle", "val")
                       .match("Normal"));
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesDeletedTableRow, "testTrackChangesDeletedTableRow.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:trPr/w:del");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesInsertedTableRow, "testTrackChangesInsertedTableRow.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:trPr/w:ins");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesDeletedTableCell, "testTrackChangesDeletedTableCell.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:tcPr/w:cellDel");
}

DECLARE_OOXMLEXPORT_TEST(testTrackChangesInsertedTableCell,
                         "testTrackChangesInsertedTableCell.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:tcPr/w:cellIns");
}

DECLARE_OOXMLEXPORT_TEST(testFdo73556, "fdo73556.docx")
{
    /*
    *  The file contains a table with 3 columns
    *  the gridcols are as follows: {1210, 1331, 1210}
    *  whereas the individual cells have {1210, 400, 1210}
    *  The table column separators were taken from the Grid, while
    *  the table width was calculated as 2820 from cells instead
    *  of 3751 from the Grid.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol", 3);
    sal_Int32 tableWidth = 0;
    tableWidth
        += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[1]", "w").toInt32();
    tableWidth
        += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[2]", "w").toInt32();
    tableWidth
        += getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[3]", "w").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3751), tableWidth);
}

DECLARE_OOXMLEXPORT_TEST(testSegFaultWhileSave, "test_segfault_while_save.docx")
{
    // fdo#74499
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(6137),
        getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol[2]", "w").toInt32());
}

DECLARE_OOXMLEXPORT_TEST(fdo69656, "Table_cell_auto_width_fdo69656.docx")
{
    // Changed the UT to check "dxa" instead of "auto"
    // For this particular issue file few cells have width type "auto"
    // LO supports VARIABLE and FIXED width type.
    // If type is VARIABLE LO calculates width as percent of PageSize
    // Else if the width is fixed it uses the width value.
    // After changes for fdo76741 the fixed width is exported as "dxa" for DOCX

    // Check for the width type of table and its cells.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblW", "type", "dxa");
}

DECLARE_OOXMLEXPORT_TEST(testFdo76741, "fdo76741.docx")
{
    // There are two issue related to table in the saved(exported) file
    // - the table alignment in saved file is "left" instead of "center"
    // - the table width type in properties is "auto" instead of "dxa"

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "//w:jc", "val", "center");
    assertXPath(pXmlDoc, "//w:tblW", "w", "10081");
    assertXPath(pXmlDoc, "//w:tblW", "type", "dxa");
}

DECLARE_OOXMLEXPORT_TEST(testFloatingTablePosition, "floating-table-position.docx")
{
    // Position of shape was wrong, because some conversion was missing.
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // This was 3295.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5964), getProperty<sal_Int32>(xShape, "HoriOrientPosition"));
    // This was 4611.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8133), getProperty<sal_Int32>(xShape, "VertOrientPosition"));
}

DECLARE_OOXMLEXPORT_TEST(testTCTagMisMatch, "TCTagMisMatch.docx")
{
    // TCTagMisMatch.docx : This document contains a empty table with borders.
    // there was a TC tag mismatch which resulted into a crash.

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl[1]/w:tr[1]/w:tc[1]", 0);
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf99090_pgbrkAfterTable, "tdf99090_pgbrkAfterTable.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        // There should be a regular page break that's in the middle of the document: right after the table.
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br", 1);
}

DECLARE_OOXMLEXPORT_TEST(testFDO76586, "fdo76586.docx")
{
    /*
    * In the test file gridCol had only one value for entire table width
    * while there are two cells in a table row.
    * So the table was not imported with the correct cell widths
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    // there is only one table in the test file
    assertXPath(pXmlDoc, "//w:tblGrid/w:gridCol[1]", "w", "1601");
    assertXPath(pXmlDoc, "//w:tblGrid/w:gridCol[2]", "w", "7843");
}

DECLARE_OOXMLEXPORT_TEST(testFloatingTable, "fdo77887.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "horzAnchor",
                "margin");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "leftFromText",
                "141");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]",
                "rightFromText", "141");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "tblpXSpec",
                "center");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "tblpY",
                "2266");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]", "vertAnchor",
                "page");

    //make sure not to write empty attributes which requires enumeration
    assertXPathNoAttribute(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblpPr[1]",
                           "tblpYSpec");
}

DECLARE_OOXMLEXPORT_TEST(testOldComplexMergeRight, "tdf90681-1.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[2]/w:tcPr/w:vMerge", "val",
                "restart");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:vMerge", "val",
                "continue");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[2]/w:tcPr/w:vMerge", "val",
                "continue");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[2]/w:tcPr/w:vMerge", "val",
                "continue");
}

DECLARE_OOXMLEXPORT_TEST(testOldComplexMergeleft, "tdf90681-2.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:vMerge", "val",
                "restart");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:vMerge", "val",
                "continue");
}

DECLARE_OOXMLEXPORT_TEST(testTablePreferredWidth, "tablePreferredWidth.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    // Problem :If the table preferred width is in percent, then after RT it changes to 0 & width type changes
    // to 'auto' instead of 'pct'.
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblW[1]", "w", "3000");
    assertXPath(pXmlDoc, "/w:document[1]/w:body[1]/w:tbl[1]/w:tblPr[1]/w:tblW[1]", "type", "pct");
}

DECLARE_OOXMLEXPORT_TEST(testFDO75431, "fdo75431.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "//w:tbl", 2);
    assertXPath(pXmlDoc, "//w:p/w:pPr/w:sectPr/w:type", "val", "nextPage");
}

DECLARE_OOXMLEXPORT_TEST(testFdo78651, "fdo78651.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;
    // ensure that there are only two tables
    assertXPath(pXmlDoc, "//w:tbl", 2);
}

DECLARE_OOXMLEXPORT_TEST(testfdo80097, "fdo80097.docx")
{
    //fdo#76635 : Table borders are not getting preserved.

    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    //Table Borders
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/w:top[@w:val "
                "= 'single']",
                1);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/w:top[@w:sz = 4]", 1);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/w:top[@w:space = 0]",
        1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/"
                "w:top[@w:color = '000000']",
                1);

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/"
                "w:bottom[@w:val = 'single']",
                1);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/w:bottom[@w:sz = 4]",
        1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/"
                "w:bottom[@w:space = 0]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/"
                "w:bottom[@w:color = '000000']",
                1);

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/"
                "w:insideH[@w:val = 'single']",
                1);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/w:insideH[@w:sz = 4]",
        1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/"
                "w:insideH[@w:space = 0]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/"
                "w:insideH[@w:color = '000000']",
                1);

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/"
                "w:insideV[@w:val = 'single']",
                1);
    assertXPath(
        pXmlDocument,
        "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/w:insideV[@w:sz = 4]",
        1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/"
                "w:insideV[@w:space = 0]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tblPr/w:tblBorders/"
                "w:insideV[@w:color = '000000']",
                1);

    //Table Cell Borders
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:top[@w:val = 'single']",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:top[@w:sz = 4]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:top[@w:space = 0]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:top[@w:color = '000000']",
                1);

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:bottom[@w:val = 'single']",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:bottom[@w:sz = 4]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:bottom[@w:space = 0]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:bottom[@w:color = '000000']",
                1);

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:insideH[@w:val = 'single']",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:insideH[@w:sz = 4]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:insideH[@w:space = 0]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:insideH[@w:color = '000000']",
                1);

    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:insideV[@w:val = 'single']",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:insideV[@w:sz = 4]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:insideV[@w:space = 0]",
                1);
    assertXPath(pXmlDocument,
                "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/"
                "w:insideV[@w:color = '000000']",
                1);
}

DECLARE_OOXMLEXPORT_TEST(testTableCellWithDirectFormatting, "fdo80800.docx")
{
    // Issue was Direct Formatting for non-first Table cells was not getting preserved.

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // Ensure that for Third Table cell Direct Formatting is preserved.
    // In file, Direct Formatting used for Third Table cell is Line Spacing="1.5 lines"
    // For Line Spacing "1.5 lines" w:line equals 360
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[3]/w:p/w:pPr/w:spacing", "line",
                "360");
}

DECLARE_OOXMLEXPORT_TEST(testFdo80800b_tableStyle, "fdo80800b_tableStyle.docx")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Cell1 1.5lines"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell A1 1.5 line spacing", sal_Int16(150),
                                 getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);

    xCell.set(xTable->getCellByName("B1"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Cell2 Implicit (Single)"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 single line spacing", sal_Int16(100),
                                 getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);

    xCell.set(xTable->getCellByName("C1"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Cell3 Implicit (Single)"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("C1 paragraph1 single line spacing", sal_Int16(100),
                                 getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("C1 paragraph3 line spacing", sal_Int16(212),
                                 getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);
}

DECLARE_OOXMLEXPORT_TEST(testTdf117297_tableStyle, "tdf117297_tableStyle.docx")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xPara->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Green text, default size (9), 1.5 spaced"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph1 green font", sal_Int32(0x70AD47),
                                 getProperty<sal_Int32>(getRun(xPara, 1), "CharColor"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph1 1.5 line spacing", sal_Int16(150),
                                 getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xText.set(xPara->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("TableGrid color (blue), TableGrid size (9), double spacing"),
                         xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph3 blue font", sal_Int32(0x00B0F0),
                                 getProperty<sal_Int32>(getRun(xPara, 1), "CharColor"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B1 Paragraph3 double spacing", sal_Int16(200),
                                 getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height);
}

DECLARE_OOXMLEXPORT_TEST(testTablepprShape, "tblppr-shape.docx")
{
    // Unhandled exception due to unexpected w:tblpPr resulted in not importing page size, either.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    // This was 2000, page width wasn't large enough.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2540),
                         getProperty<sal_Int32>(xPageStyle, "LeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testOO47778_1, "ooo47778-3.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathContent(pXmlDoc, "(//w:t)[3]", "c");
}

DECLARE_OOXMLEXPORT_TEST(testOO47778_2, "ooo47778-4.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathContent(pXmlDoc, "(//w:t)[4]", "c");
}

DECLARE_OOXMLEXPORT_TEST(testOO67471, "ooo67471-2.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathContent(pXmlDoc, "(//w:t)[2]", "B");
}

DECLARE_OOXMLEXPORT_TEST(testOO72950, "ooo72950-1.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//w:tbl", 1);
}

//There are two tables to export in this doc the second of which is inside a
//frame anchored to first cell of the first table. They must not be
//considered the same table
DECLARE_OOXMLEXPORT_TEST(fdo60957, "fdo60957-2.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//w:tbl", 2);
}

//This has more cells than msword supports, we must balance the
//number of cell start and ends
DECLARE_OOXMLEXPORT_TEST(testOO106020, "ooo106020-1.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "//w:tbl", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTableFloatingMargins, "table-floating-margins.docx")
{
    // In case the table had custom left cell margin, the horizontal position was still incorrect (too small, -199).
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    sal_Int32 nHoriOrientPosition = getProperty<sal_Int32>(xFrame, "HoriOrientPosition");
    CPPUNIT_ASSERT(nHoriOrientPosition < sal_Int32(-495));
    // These were 0 as well, due to lack of import.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(xFrame, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xFrame, "BottomMargin"));

    // Paragraph bottom margin wasn't 0 in the A1 cell of the floating table.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/w:pict/v:rect/"
                "v:textbox/w:txbxContent/w:tbl/w:tr[1]/w:tc[1]/w:p/w:pPr/w:spacing",
                "after", "0");
}

DECLARE_OOXMLEXPORT_TEST(testAllGapsWord, "all_gaps_word.docx")
{
    BorderTest borderTest;
    BorderTest::testTheBorders(mxComponent, false);
}

DECLARE_OOXMLEXPORT_TEST(testFdo74357, "fdo74357.docx")
{
    // Floating table wasn't converted to a textframe.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(),
                                                      uno::UNO_QUERY);
    // This was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());

    // Bottom margin of the first paragraph was too large, causing a layout problem.
    // This was 494.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(86),
                         getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testN780853, "n780853.docx")
{
    /*
    * The problem was that the table was not imported.
    *
    * xray ThisComponent.TextTables.Count 'was 0
    */
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    //tdf#102619 - I would have expected this to be "Standard", but MSO 2013/2010/2003 all give FollowStyle==Date
    uno::Reference<beans::XPropertySet> properties(getStyles("ParagraphStyles")->getByName("Date"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Date"), getProperty<OUString>(properties, "FollowStyle"));
}

DECLARE_OOXMLEXPORT_TEST(testN779941, "n779941.docx")
{
    /*
    * Make sure top/bottom margins of tables are set to 0 (problem was: bottom margin set to 0.35cm)
    */
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(0), uno::UNO_QUERY);
    {
        uno::Any aValue = xTableProperties->getPropertyValue("TopMargin");
        sal_Int32 nTopMargin;
        aValue >>= nTopMargin;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nTopMargin);
    }
    {
        uno::Any aValue = xTableProperties->getPropertyValue("BottomMargin");
        sal_Int32 nBottomMargin;
        aValue >>= nBottomMargin;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nBottomMargin);
    }
}

DECLARE_OOXMLEXPORT_TEST(testN785767, "n785767.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // Check the A1 and B1 cells, the width of both of them was the default value (10000 / 9, as there were 9 cells in the row).
    CPPUNIT_ASSERT_MESSAGE("A1 must not have default width",
                           sal_Int16(10000 / 9)
                               != getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                      xTableRows->getByIndex(0), "TableColumnSeparators")[0]
                                      .Position);
    CPPUNIT_ASSERT_MESSAGE("B1 must not have default width",
                           sal_Int16(10000 / 9)
                               != getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                      xTableRows->getByIndex(1), "TableColumnSeparators")[0]
                                      .Position);
}

DECLARE_OOXMLEXPORT_TEST(testFineTableDash, "tableborder-finedash.docx")
{
    // The problem was that finely dashed borders on tables were unsupported
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(0), uno::UNO_QUERY);
    table::TableBorder2 aBorder;
    xTableProperties->getPropertyValue("TableBorder2") >>= aBorder;
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::FINE_DASHED, aBorder.RightLine.LineStyle);
}

DECLARE_OOXMLEXPORT_TEST(testN793262, "n793262.docx")
{
    uno::Reference<container::XEnumerationAccess> xHeaderText
        = getProperty<uno::Reference<container::XEnumerationAccess>>(
            getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    uno::Reference<container::XEnumeration> xHeaderParagraphs(xHeaderText->createEnumeration());
    xHeaderParagraphs->nextElement();
    // Font size of the last empty paragraph in the header was ignored, this was 11.
    CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xHeaderParagraphs->nextElement(), "CharHeight"));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell margins as direct formatting were ignored, this was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(76),
                         getProperty<sal_Int32>(xTable->getCellByName("A1"), "TopBorderDistance"));
}

DECLARE_OOXMLEXPORT_TEST(testN779642, "n779642.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);

    // First problem: check that we have 2 tables, nesting caused the
    // creation of outer one to fail
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong number of imported tables", sal_Int32(2),
                                 xTables->getCount());

    // Second problem: check that the outer table is in a frame, at the bottom of the page
    uno::Reference<text::XTextTable> xTextTable(
        xTextTablesSupplier->getTextTables()->getByName("Table2"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xAnchor(xTextTable->getAnchor(), uno::UNO_QUERY);
    uno::Any aFrame = xAnchor->getPropertyValue("TextFrame");
    uno::Reference<beans::XPropertySet> xFrame;
    aFrame >>= xFrame;
    sal_Int16 nValue;
    xFrame->getPropertyValue("VertOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical orientation", text::VertOrientation::BOTTOM,
                                 nValue);
    xFrame->getPropertyValue("VertOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical orientation relation",
                                 text::RelOrientation::PAGE_PRINT_AREA, nValue);
}

DECLARE_OOXMLEXPORT_TEST(testTbLrHeight, "tblr-height.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // btLr text direction was imported as MIN, it should be FIX to avoid incorrectly large height in case of too much content.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX,
                         getProperty<sal_Int16>(xTableRows->getByIndex(0), "SizeType"));
}

DECLARE_OOXMLEXPORT_TEST(testBnc865381, "bnc865381.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // Second row has a vertically merged cell, make sure size type is not FIX in that case (otherwise B2 is not readable).
    CPPUNIT_ASSERT(text::SizeType::FIX
                   != getProperty<sal_Int16>(xTableRows->getByIndex(1), "SizeType"));
    // Explicit size of 41 mm100 was set, so the vertical text in A2 was not readable.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTableRows->getByIndex(1), "Height"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo53985, "fdo53985.docx")
{
    // Unhandled exception prevented import of the rest of the document.

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTables->getCount()); // Only 4 tables were imported.
}

DECLARE_OOXMLEXPORT_TEST(testFdo59273, "fdo59273.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Was 115596 (i.e. 10 times wider than necessary), as w:tblW was missing and the importer didn't set it.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12961), getProperty<sal_Int32>(xTextTable, "Width"));

    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // Was 9997, so the 4th column had ~zero width
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7498), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                              xTableRows->getByIndex(0), "TableColumnSeparators")[2]
                                              .Position);
}

DECLARE_OOXMLEXPORT_TEST(testConditionalstylesTablelook, "conditionalstyles-tbllook.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Background was -1.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x7F7F7F),
                         getProperty<sal_Int32>(xTable->getCellByName("A1"), "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testN816593, "n816593.docx")
{
    // Two consecutive <w:tbl> without any paragraph in between, but with different tblpPr. In this
    // case we need to have 2 different tables instead of 1
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTableAutoColumnFixedSize, "table-auto-column-fixed-size.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Width was not recognized during import when table size was 'auto'
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(3996)),
                         getProperty<sal_Int32>(xTextTable, "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testTableAutoColumnFixedSize2, "table-auto-column-fixed-size2.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was 17907, i.e. the sum of the width of the 3 cells (10152 twips each), which is too wide.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16891), getProperty<sal_Int32>(xTextTable, "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66474, "fdo66474.docx")
{
    // The table width was too small, so the text in the second cell was unreadable: this was 1397.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10492), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testTableAutoNested, "table-auto-nested.docx")
{
    // This was 176, when compat option is not enabled, the auto paragraph bottom margin value was incorrect.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(494),
                         getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // This was 115596, i.e. the width of the outer table was too large.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(23051), getProperty<sal_Int32>(xTables->getByIndex(1), "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testTableStyleParprop, "table-style-parprop.docx")
{
    // The problem was that w:spacing's w:after=0 (a paragraph property) wasn't imported from table style.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 353, the document default, i.e. paragraph property from table style had no effect.
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTablePagebreak, "table-pagebreak.docx")
{
    // Page break inside table: should be ignored (was style::BreakType_PAGE_BEFORE before).
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(getParagraphOrTable(2), "BreakType"));

    // This one is outside the table: should not be ignored.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE,
                         getProperty<style::BreakType>(getParagraph(3), "BreakType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo68607, "fdo68607.docx")
{
    // Bugdoc was 8 pages in Word, 1 in Writer due to pointlessly wrapping the
    // table in a frame. Exact layout may depend on fonts available, etc. --
    // but at least make sure that our table spans over multiple pages now.
    CPPUNIT_ASSERT(getPages() > 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf109063, "tdf109063.docx")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1, near-page-width table was imported as a TextFrame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf79272_strictDxa, "tdf79272_strictDxa.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4318), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf109306, "tdf109306.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // Both types of relative width specification (pct): simple integers (in fiftieths of percent)
    // and floats with "%" unit specification must be treated correctly
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTables->getByIndex(0), "IsWidthRelative"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(9),
                         getProperty<sal_Int16>(xTables->getByIndex(0), "RelativeWidth"));

    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTables->getByIndex(1), "IsWidthRelative"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(80),
                         getProperty<sal_Int16>(xTables->getByIndex(1), "RelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf79329, "tdf79329.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // This was 1: only the inner, not the outer table was created.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xTables->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf103976, "tdf103976.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 0, table style inheritance went wrong and w:afterLines had priority over w:after.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(convertTwipToMm100(60)),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaBottomMargin"));
}

#if HAVE_MORE_FONTS
DECLARE_OOXMLEXPORT_TEST(testTdf107889, "tdf107889.docx")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1, multi-page table was imported as a floating one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}
#endif

DECLARE_OOXMLEXPORT_TEST(testTdf109184, "tdf109184.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Before table background color was white, should be transparent (auto).
    uno::Reference<text::XTextRange> xCell1(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), getProperty<sal_Int32>(xCell1, "BackColor"));

    // Cell with auto color but with 15% fill, shouldn't be transparent.
    uno::Reference<text::XTextRange> xCell2(xTable->getCellByName("B1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xd8d8d8),
                         getProperty<sal_Int32>(xCell2, "BackColor"));

    // Cell with color defined (red).
    uno::Reference<text::XTextRange> xCell3(xTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xff0000),
                         getProperty<sal_Int32>(xCell3, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65090, "fdo65090.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // The first row had two cells, instead of a single horizontally merged one.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                           xTableRows->getByIndex(0), "TableColumnSeparators")
                                           .getLength());
}

DECLARE_OOXMLEXPORT_TEST(testFdo73389, "fdo73389.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // This was 9340, i.e. the width of the inner table was too large.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2842), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo69656, "Table_cell_auto_width_fdo69656.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8154), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testFloatingTablesAnchor, "floating-tables-anchor.docx")
{
    // Problem was one of the two text frames was anchored to the other text frame
    // Both frames should be anchored to the paragraph with the text "Anchor point"
    uno::Reference<text::XTextContent> xTextContent(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());

    xTextContent.set(getShape(2), uno::UNO_QUERY);
    xRange.set(xTextContent->getAnchor(), uno::UNO_QUERY);
    xText.set(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());
}

DECLARE_OOXMLEXPORT_TEST(testLargeTwips, "large-twips.docx")
{
    // cp#1000043: MSO seems to ignore large twips values, we didn't, which resulted in different
    // layout of broken documents (text not visible in this specific document).
    OUString width = parseDump("/root/page/body/tab/row[1]/cell[1]/txt/infos/bounds", "width");
    CPPUNIT_ASSERT(width.toInt32() > 0);
}

DECLARE_OOXMLEXPORT_TEST(testNegativeCellMarginTwips, "negative-cell-margin-twips.docx")
{
    // Slightly related to cp#1000043, the twips value was negative, which wrapped around somewhere,
    // while MSO seems to ignore that as well.
    OUString width = parseDump("/root/page/body/tab/row[1]/cell[1]/txt/infos/bounds", "width");
    CPPUNIT_ASSERT(width.toInt32() > 0);
}

DECLARE_OOXMLEXPORT_TEST(testFdo38414, "fdo38414.docx")
{
    // The cells in the last (4th) column were merged properly and so the result didn't have the same height.
    // (Since w:gridBefore is worked around by faking another cell in the row, so column count is thus 5
    // instead of 4, therefore compare height of cells 4 and 5 rather than 3 and 4.)
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableColumns> xTableColumns(xTextTable->getColumns(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTableColumns->getCount());
    OUString height3 = parseDump("/root/page/body/tab/row[1]/cell[4]/infos/bounds", "height");
    OUString height4 = parseDump("/root/page/body/tab/row[1]/cell[5]/infos/bounds", "height");
    CPPUNIT_ASSERT_EQUAL(height3, height4);
}

DECLARE_OOXMLEXPORT_TEST(testGridBefore, "gridbefore.docx")
{
    // w:gridBefore is faked by inserting two cells without border (because Writer can't do non-rectangular tables).
    // So check the first cell in the first row is in fact 3rd and that it's more to the right than the second
    // cell on the second row.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableColumns> xTableColumns(xTextTable->getColumns(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTableColumns->getCount());
    OUString textA3 = parseDump("/root/page/body/tab/row[1]/cell[3]/txt/text()");
    OUString leftA3 = parseDump("/root/page/body/tab/row[1]/cell[3]/infos/bounds", "left");
    OUString leftB2 = parseDump("/root/page/body/tab/row[2]/cell[2]/infos/bounds", "left");
    CPPUNIT_ASSERT_EQUAL(OUString("A3"), textA3);
    CPPUNIT_ASSERT(leftA3.toInt32() > leftB2.toInt32());
}

DECLARE_OOXMLEXPORT_TEST(testTableBtlrCenter, "table-btlr-center.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell vertical alignment was NONE, should be CENTER.
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER,
                         getProperty<sal_Int16>(xTable->getCellByName("A2"), "VertOrient"));
}

DECLARE_OOXMLEXPORT_TEST(testHidemark, "hidemark.docx")
{
    // Problem was that <w:hideMark> cell property was ignored.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // Height should be minimal
    CPPUNIT_ASSERT_EQUAL(convertTwipToMm100(MINLAY),
                         getProperty<sal_Int64>(xTableRows->getByIndex(1), "Height"));
    // Size type was MIN, should be FIX to avoid considering the end of paragraph marker.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX,
                         getProperty<sal_Int16>(xTableRows->getByIndex(1), "SizeType"));

    //tdf#104876: Width was not recognized during import when table size was 'auto'
    CPPUNIT_ASSERT_MESSAGE("table size is less than 7000?",
                           sal_Int32(7000) > getProperty<sal_Int32>(xTextTable, "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testHidemarkb, "tdf99616_hidemarkb.docx")
{
    // Problem was that the smallest possible height was forced, not the min specified size.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // Height should be .5cm
    CPPUNIT_ASSERT_EQUAL(sal_Int64(501),
                         getProperty<sal_Int64>(xTableRows->getByIndex(1), "Height"));
    // Size type was MIN, should be FIX to avoid considering the end of paragraph marker.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX,
                         getProperty<sal_Int16>(xTableRows->getByIndex(1), "SizeType"));
}

DECLARE_OOXMLEXPORT_TEST(testTcwRounding, "tcw-rounding.docx")
{
    // Width of the A1 cell in twips was 3200, due to a rounding error.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(3201),
        parseDump("/root/page/body/tab/row/cell[1]/infos/bounds", "width").toInt32());
}

DECLARE_OOXMLEXPORT_TEST(testTdf8255, "tdf8255.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xTextDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1: a full-page-wide multi-page floating table was imported as a TextFrame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf87460, "tdf87460.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(xTextDocument, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes = xEndnotesSupplier->getEndnotes();
    // This was 0: endnote was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xEndnotes->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf86374, "tdf86374.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // btLr text direction was imported as FIX, it should be MIN to have enough space for the additionally entered paragraphs.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::MIN,
                         getProperty<sal_Int16>(xTableRows->getByIndex(0), "SizeType"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf99140, "tdf99140.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xTextDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1: a multi-page floating table was imported as a TextFrame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(1), uno::UNO_QUERY);
    // This was text::HoriOrientation::NONE, the second table was too wide due to this.
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::LEFT_AND_WIDTH,
                         getProperty<sal_Int16>(xTableProperties, "HoriOrient"));
}

DECLARE_OOXMLEXPORT_TEST(testTableCellMargin, "table-cell-margin.docx")
{
    sal_Int32 const cellLeftMarginFromOffice[] = { 250, 100, 0, 0 };

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    for (int i = 0; i < 4; i++)
    {
        uno::Reference<text::XTextTable> xTable1(xTables->getByIndex(i), uno::UNO_QUERY);

        // Verify left margin of 1st cell :
        //  * Office left margins are measured relative to the right of the border
        //  * LO left spacing is measured from the center of the border
        uno::Reference<table::XCell> xCell = xTable1->getCellByName("A1");
        uno::Reference<beans::XPropertySet> xPropSet(xCell, uno::UNO_QUERY_THROW);
        sal_Int32 aLeftMargin = -1;
        xPropSet->getPropertyValue("LeftBorderDistance") >>= aLeftMargin;
        uno::Any aLeftBorder = xPropSet->getPropertyValue("LeftBorder");
        table::BorderLine2 aLeftBorderLine;
        aLeftBorder >>= aLeftBorderLine;
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "Incorrect left spacing computed from docx cell margin", cellLeftMarginFromOffice[i],
            aLeftMargin - 0.5 * aLeftBorderLine.LineWidth, 1);
        // The 'a' in the fourth table should not be partly hidden by the border
        if (i == 3)
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Incorrect cell padding",
                                                 0.5 * aLeftBorderLine.LineWidth, aLeftMargin, 1);
    }
}

// tdf#106742 for DOCX with compatibility level <= 14 (MS Word up to and incl. ver.2010), we should use cell margins when calculating table left border position
DECLARE_OOXMLEXPORT_TEST(testTablePosition14, "table-position-14.docx")
{
    sal_Int32 const aXCoordsFromOffice[] = { 2500, -1000, 0, 0 };

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    for (int i = 0; i < 4; i++)
    {
        uno::Reference<text::XTextTable> xTable1(xTables->getByIndex(i), uno::UNO_QUERY);

        // Verify X coord
        uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(),
                                                       uno::UNO_QUERY);
        xCtrl->select(uno::makeAny(xTable1));
        uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xCtrl,
                                                                              uno::UNO_QUERY);
        uno::Reference<text::XTextViewCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                                      uno::UNO_QUERY);
        awt::Point pos = xCursor->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Incorrect X coord computed from docx",
                                             aXCoordsFromOffice[i], pos.X, 1);
    }
}

// tdf#106742 for DOCX with compatibility level > 14 (MS Word since ver.2013), we should NOT use cell margins when calculating table left border position
DECLARE_OOXMLEXPORT_TEST(testTablePosition15, "table-position-15.docx")
{
    sal_Int32 const aXCoordsFromOffice[] = { 2751, -899, 1, 106 };

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    for (int i = 0; i < 4; i++)
    {
        uno::Reference<text::XTextTable> xTable1(xTables->getByIndex(i), uno::UNO_QUERY);

        // Verify X coord
        uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(),
                                                       uno::UNO_QUERY);
        xCtrl->select(uno::makeAny(xTable1));
        uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xCtrl,
                                                                              uno::UNO_QUERY);
        uno::Reference<text::XTextViewCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                                      uno::UNO_QUERY);
        awt::Point pos = xCursor->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Incorrect X coord computed from docx",
                                             aXCoordsFromOffice[i], pos.X, 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf116801, "tdf116801.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // This raised a lang::IndexOutOfBoundsException, table was missing from
    // the import result.
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("D1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("D1"), xCell->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
