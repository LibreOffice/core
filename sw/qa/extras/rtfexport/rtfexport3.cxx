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
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>

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

DECLARE_RTFEXPORT_TEST(testTdf104035, "tdf104035.rtf")
{
    auto aTabStops = getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), "ParaTabStops");
    CPPUNIT_ASSERT(aTabStops.hasElements());
    // This was 3330 twips instead, as tabs were assumed to be relative.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(450)), aTabStops[0].Position);
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
    uno::Reference<text::XTextRangeCompare> xTextRangeCompare(xCell, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextRangeCompare->compareRegionStarts(xAnchorCell, xCell));
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
