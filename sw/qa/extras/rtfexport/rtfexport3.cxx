/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format")
    {
    }
};

DECLARE_RTFEXPORT_TEST(testTdf108949, "tdf108949_footnoteCharFormat.odt")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Paragraph Numbering style", OUString(),
                                 getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);

    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;
    // This was green (0x00A800), the character property of the footnote character, not the footnote text
    CPPUNIT_ASSERT_MESSAGE(
        "Footnote Text color",
        sal_Int32(0x000000) >= getProperty<sal_Int32>(
                                   getRun(getParagraphOfText(1, xFootnoteText), 1), "CharColor"));
}

DECLARE_RTFEXPORT_TEST(testTdf108949_footnote, "tdf108949_footnote.rtf")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Paragraph Numbering style", OUString(),
                                 getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);

    uno::Reference<text::XFootnote> xFootnote;
    xFootnotes->getByIndex(0) >>= xFootnote;
    // The color of the footnote anchor was black (0x000000)
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote Character color", sal_Int32(0xFF0000),
                                 getProperty<sal_Int32>(xFootnote->getAnchor(), "CharColor"));
}

DECLARE_RTFEXPORT_TEST(testTdf116436_tableBackground, "tdf116436_tableBackground.odt")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    if (mbExported)
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xF8DF7C), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell.set(xTable->getCellByName("A6"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x81D41A), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell.set(xTable->getCellByName("B6"));
    if (mbExported)
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFBCC), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_RTFEXPORT_TEST(testTdf122589_firstSection, "tdf122589_firstSection.odt")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText
        = getProperty<uno::Reference<text::XTextRange>>(xPageStyle, "HeaderText");
    CPPUNIT_ASSERT_EQUAL(OUString("My header"), xHeaderText->getString());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of paragraphs", 2, getParagraphs());
}

DECLARE_RTFEXPORT_TEST(testTdf104035, "tdf104035.rtf")
{
    auto aTabStops = getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), "ParaTabStops");
    CPPUNIT_ASSERT(aTabStops.hasElements());
    // This was 3330 twips instead, as tabs were assumed to be relative.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(450)), aTabStops[0].Position);
}

DECLARE_RTFEXPORT_TEST(testGraphicObjectFliph, "graphic-object-fliph.rtf")
{
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), "HoriMirroredOnEvenPages"));
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), "HoriMirroredOnOddPages"));
}

DECLARE_RTFEXPORT_TEST(testTdf114333, "tdf114333.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Check the distance from left
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8502), getProperty<sal_Int32>(xTable, "LeftMargin"));
    // This was 17000 = 8502 + 8498 on import, 15240 on export and following import
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8498), getProperty<sal_Int32>(xTable, "Width"));
}

DECLARE_RTFEXPORT_TEST(testTdf115180, "tdf115180.docx")
{
    // On export to RTF, column separator positions were written without taking base width
    // into account and then arrived huge, ~64000, which resulted in wrong table and cell widths

    sal_Int32 rowWidth = parseDump("/root/page/body/tab/row/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row width", sal_Int32(9360), rowWidth);
    sal_Int32 cell1Width
        = parseDump("/root/page/body/tab/row/cell[1]/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT_MESSAGE("First cell width", cell1Width >= 9140 && cell1Width <= 9142);
    sal_Int32 cell2Width
        = parseDump("/root/page/body/tab/row/cell[2]/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT_MESSAGE("Second cell width", cell2Width >= 218 && cell2Width <= 220);
}

DECLARE_RTFEXPORT_TEST(testTdf116841, "tdf116841.rtf")
{
    // This was 0, left margin was ignored as we assumed the default is already
    // fine for us.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

DECLARE_RTFEXPORT_TEST(testTdf117268, "tdf117268.rtf")
{
    // Here we check that we correctly mimic Word's treatment of erroneous \itap0 inside tables.
    // Previously, the first table was import as text, and second top-level one only imported
    // last row with nested table (first row was also imported as text).
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY_THROW);

    // First (simple) table
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Text 1"), xCell->getString());

    // Nested table
    xTable.set(xTables->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    xCell.set(xTable->getCellByName("A1"), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Text 3"), xCell->getString());
    uno::Reference<beans::XPropertySet> xNestedAnchor(xTable->getAnchor(), uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextRange> xAnchorCell(xNestedAnchor->getPropertyValue("Cell"),
                                                 uno::UNO_QUERY_THROW);

    // Outer table
    xTable.set(xTables->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
    xCell.set(xTable->getCellByName("A1"), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Text 2"), xCell->getString());
    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(xCell, xAnchorCell);
}

DECLARE_RTFEXPORT_TEST(testTdf117505, "tdf117505.odt")
{
    uno::Reference<container::XNameAccess> xPageStyles(getStyles("PageStyles"));
    uno::Reference<beans::XPropertySet> xFirstPage(xPageStyles->getByName("First Page"),
                                                   uno::UNO_QUERY);
    // This was 499, small header height resulted in visible whitespace from
    // remaining top margin -> header content moved down.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1499),
                         getProperty<sal_Int32>(xFirstPage, "HeaderHeight"));
}

DECLARE_RTFEXPORT_TEST(testTdf112520, "tdf112520.docx")
{
    if (!mbExported)
        return;

    // Assert that the white shape is on top of the yellow one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xffff00),
                         getProperty<sal_Int32>(getShape(2), "FillColor"));
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(2), "AnchorType"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xffffff),
                         getProperty<sal_Int32>(getShape(3), "FillColor"));
    // Without the accompanying fix in place, this test would have failed with
    // 'expected: 4, actual: 2'.
    // This means the draw page was 0/at-char/white, 1/at-char/yellow, 2/at-page/white,
    // instead of the good 0/at-page/white, 1/at-char/yellow, 2/at-char/white.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(3), "AnchorType"));
}

DECLARE_RTFEXPORT_TEST(testTdf121623, "tdf121623.rtf")
{
    // This was 2, multicolumn section was ignored at the table.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFEXPORT_TEST(testTdf66543, "tdf66543.rtf")
{
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 2; Actual  : 3' after import (off-by-one), then with
    // 'Expected: 2; Actual  : 0' (export not implemented).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLineNumberStartValue"));
}

DECLARE_RTFEXPORT_TEST(testUlw, "ulw.rtf")
{
    // Test underlying in individual words mode.
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), "CharWordMode"));
}

DECLARE_RTFEXPORT_TEST(testTdf122455, "tdf122455.rtf")
{
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 16; Actual  : 32', the font size from a list definition
    // leaked into the first run's character properties.
    CPPUNIT_ASSERT_EQUAL(16.0, getProperty<double>(getRun(getParagraph(1), 1), "CharHeight"));
}

DECLARE_RTFEXPORT_TEST(testTabs, "tabs.rtf")
{
    // Test tab alignment in decimal mode.
    auto aTabStops = getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), "ParaTabStops");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aTabStops.getLength());
    const style::TabStop& rTabStop = aTabStops[0];
    CPPUNIT_ASSERT_EQUAL(style::TabAlign_DECIMAL, rTabStop.Alignment);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
