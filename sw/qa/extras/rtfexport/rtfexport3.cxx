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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("First cell width", sal_Int32(9142), cell1Width);
    sal_Int32 cell2Width
        = parseDump("/root/page/body/tab/row/cell[2]/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("First cell width", sal_Int32(218), cell2Width);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
