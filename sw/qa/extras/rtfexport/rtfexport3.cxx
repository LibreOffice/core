/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/TabStop.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <tools/UnitConversion.hxx>
#include <comphelper/propertyvalue.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <fmtpdsc.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentSettingAccess.hxx>
#include <itabenum.hxx>
#include <frmmgr.hxx>
#include <formatflysplit.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>

using namespace css;

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/rtfexport/data/"_ustr, u"Rich Text Format"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf100961_fixedDateTime)
{
    auto verify = [this]() {
        // This should be a fixed date/time field, not the current time.
        getParagraph(1, u"05.01.19 04:06:08"_ustr);

        uno::Reference<text::XTextFieldsSupplier> xTFS(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xFields(xTFS->getTextFields()->createEnumeration());
        CPPUNIT_ASSERT_MESSAGE("constant time",
                               getProperty<bool>(xFields->nextElement(), u"IsFixed"_ustr));
    };
    createSwDoc("tdf100961_fixedDateTime.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108949)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Paragraph Numbering style", OUString(),
            getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));

        uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();

        uno::Reference<text::XText> xFootnoteText;
        xFootnotes->getByIndex(0) >>= xFootnoteText;
        // This was green (0x00A800), the character property of the footnote character, not the footnote text
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Footnote Text color", COL_AUTO,
            getProperty<Color>(getRun(getParagraphOfText(1, xFootnoteText), 1), u"CharColor"_ustr));
    };
    createSwDoc("tdf108949_footnoteCharFormat.odt");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141964_numId0)
{
    auto verify = [this]() {
        // Unit test added where numId is zero - which is only possible in RTF.
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3, u"Geschichte"_ustr),
                                                  uno::UNO_QUERY);
        // Pre-emptive test: ensure that paragraph 3 remains numbered and numId0 doesn't mean no numbering.
        CPPUNIT_ASSERT(!getProperty<OUString>(xPara, u"NumberingStyleName"_ustr).isEmpty());
    };
    createSwDoc("tdf141964_numId0.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108949_footnote)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Paragraph Numbering style", OUString(),
            getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));

        uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();

        uno::Reference<text::XFootnote> xFootnote;
        xFootnotes->getByIndex(0) >>= xFootnote;
        // The color of the footnote anchor was black (0x000000)
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote Character color", COL_LIGHTRED,
                                     getProperty<Color>(xFootnote->getAnchor(), u"CharColor"_ustr));
    };
    createSwDoc("tdf108949_footnote.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130817)
{
    auto verify = [this]() {
        uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xEndnotes = xEndnotesSupplier->getEndnotes();

        uno::Reference<text::XFootnote> xEndnote1;
        xEndnotes->getByIndex(0) >>= xEndnote1;
        uno::Reference<text::XText> xEndnoteText1;
        xEndnotes->getByIndex(0) >>= xEndnoteText1;
        CPPUNIT_ASSERT_EQUAL(u"Titolo 1"_ustr, xEndnoteText1->getString().trim());
        CPPUNIT_ASSERT_EQUAL(u"$"_ustr, xEndnote1->getAnchor()->getString());

        uno::Reference<text::XFootnote> xEndnote2;
        xEndnotes->getByIndex(1) >>= xEndnote2;
        uno::Reference<text::XText> xEndnoteText2;
        xEndnotes->getByIndex(1) >>= xEndnoteText2;
        CPPUNIT_ASSERT_EQUAL(u"Titolo 2"_ustr, xEndnoteText2->getString().trim());
        CPPUNIT_ASSERT_EQUAL(u"$"_ustr, xEndnote1->getAnchor()->getString());
    };
    createSwDoc("tdf130817.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf154129_transparentFrame)
{
    auto verify = [this]() {
        // Without the fix, this was zero, and the text frame with "Visible" just looks white.
        CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                             getProperty<sal_Int16>(getShape(1), u"FillTransparence"_ustr));
    };
    createSwDoc("tdf154129_transparentFrame.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf137683_charHighlightNone)
{
    auto verify = [this]() {
        uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY_THROW);
        // This test was failing with a brown charHighlight of 8421376 (0x808000), instead of COL_TRANSPARENT (0xFFFFFFFF)
        CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xRun, u"CharHighlight"_ustr));
    };
    createSwDoc("tdf137683_charHighlightNone.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116436_tableBackground)
{
    auto verify = [this](bool bIsExport = false) {
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A1"_ustr);
        if (bIsExport)
            CPPUNIT_ASSERT_EQUAL(Color(0xF8DF7C), getProperty<Color>(xCell, u"BackColor"_ustr));
        xCell.set(xTable->getCellByName(u"A6"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0x81D41A), getProperty<Color>(xCell, u"BackColor"_ustr));
        xCell.set(xTable->getCellByName(u"B6"_ustr));
        if (bIsExport)
            CPPUNIT_ASSERT_EQUAL(Color(0xFFFBCC), getProperty<Color>(xCell, u"BackColor"_ustr));
    };
    createSwDoc("tdf116436_tableBackground.odt");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf164945)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY_THROW);
        uno::Reference<text::XText> xA1(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY_THROW);
        auto borderA1(
            getProperty<table::BorderLine2>(getParagraphOfText(1, xA1), u"BottomBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), borderA1.InnerLineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(18), borderA1.OuterLineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), borderA1.LineDistance);
        uno::Reference<text::XText> xB1(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY_THROW);
        auto borderB1(
            getProperty<table::BorderLine2>(getParagraphOfText(1, xB1), u"BottomBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), borderB1.InnerLineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(18), borderB1.OuterLineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), borderB1.LineDistance);
    };
    createSwDoc("noparaborder.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf122589_firstSection)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        uno::Reference<beans::XPropertySet> xPageStyle(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xHeaderText
            = getProperty<uno::Reference<text::XTextRange>>(xPageStyle, u"HeaderText"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"My header"_ustr, xHeaderText->getString());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("# of paragraphs", 2, getParagraphs());
    };
    createSwDoc("tdf122589_firstSection.odt");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104035)
{
    auto verify = [this]() {
        auto aTabStops
            = getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), u"ParaTabStops"_ustr);
        CPPUNIT_ASSERT(aTabStops.hasElements());
        // This was 3330 twips instead, as tabs were assumed to be relative.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(450)),
                             aTabStops[0].Position);
    };
    createSwDoc("tdf104035.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testGraphicObjectFliph)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT(getProperty<bool>(getShape(1), u"HoriMirroredOnEvenPages"_ustr));
        CPPUNIT_ASSERT(getProperty<bool>(getShape(1), u"HoriMirroredOnOddPages"_ustr));
    };
    createSwDoc("graphic-object-fliph.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf114333)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        // Check the distance from left
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8502), getProperty<sal_Int32>(xTable, u"LeftMargin"_ustr));
        // This was 17000 = 8502 + 8498 on import, 15240 on export and following import
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8498), getProperty<sal_Int32>(xTable, u"Width"_ustr));
    };
    createSwDoc("tdf114333.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115180)
{
    auto verify = [this]() {
        // On export to RTF, column separator positions were written without taking base width
        // into account and then arrived huge, ~64000, which resulted in wrong table and cell widths

        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        sal_Int32 rowWidth
            = getXPath(pXmlDoc, "/root/page/body/tab/row/infos/bounds", "width").toInt32();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Row width", sal_Int32(9360), rowWidth);
        sal_Int32 cell1Width
            = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/infos/bounds", "width").toInt32();
        CPPUNIT_ASSERT_MESSAGE("First cell width", cell1Width >= 9140);
        CPPUNIT_ASSERT_MESSAGE("First cell width", cell1Width <= 9142);
        sal_Int32 cell2Width
            = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/infos/bounds", "width").toInt32();
        CPPUNIT_ASSERT_MESSAGE("Second cell width", cell2Width >= 218);
        CPPUNIT_ASSERT_MESSAGE("Second cell width", cell2Width <= 220);
    };
    createSwDoc("tdf115180.docx");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116841)
{
    auto verify = [this]() {
        // This was 0, left margin was ignored as we assumed the default is already
        // fine for us.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270),
                             getProperty<sal_Int32>(getParagraph(1), u"ParaLeftMargin"_ustr));
    };
    createSwDoc("tdf116841.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf117268)
{
    auto verify = [this]() {
        // Here we check that we correctly mimic Word's treatment of erroneous \itap0 inside tables.
        // Previously, the first table was import as text, and second top-level one only imported
        // last row with nested table (first row was also imported as text).
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent,
                                                                  uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY_THROW);

        // First (simple) table
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr),
                                               uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"Text 1"_ustr, xCell->getString());

        // Nested table
        xTable.set(xTables->getByIndex(1), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
        xCell.set(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"Text 3"_ustr, xCell->getString());
        uno::Reference<beans::XPropertySet> xNestedAnchor(xTable->getAnchor(),
                                                          uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> xAnchorCell(xNestedAnchor->getPropertyValue(u"Cell"_ustr),
                                                     uno::UNO_QUERY_THROW);

        // Outer table
        xTable.set(xTables->getByIndex(2), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumns()->getCount());
        xCell.set(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"Text 2"_ustr, xCell->getString());
        xCell.set(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(xCell, xAnchorCell);
    };
    createSwDoc("tdf117268.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf117505)
{
    createSwDoc("tdf117505.odt");
    {
        CPPUNIT_ASSERT_EQUAL(1, getShapes());
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        uno::Reference<container::XNameAccess> xPageStyles(getStyles(u"PageStyles"_ustr));
        uno::Reference<beans::XPropertySet> xFirstPage(xPageStyles->getByName(u"First Page"_ustr),
                                                       uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1499),
                             getProperty<sal_Int32>(xFirstPage, u"HeaderHeight"_ustr));
    }
    // When saving to rtf:
    saveAndReload(u"Rich Text Format"_ustr);

    {
        CPPUNIT_ASSERT_EQUAL(1, getShapes());
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        uno::Reference<container::XNameAccess> xPageStyles(getStyles(u"PageStyles"_ustr));
        uno::Reference<beans::XPropertySet> xFirstPage(xPageStyles->getByName(u"Standard"_ustr),
                                                       uno::UNO_QUERY);
        // This was 499, small header height resulted in visible whitespace from
        // remaining top margin -> header content moved down.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1499),
                             getProperty<sal_Int32>(xFirstPage, u"HeaderHeight"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf112520)
{
    createSwDoc("tdf112520.docx");
    saveAndReload(mpFilter);

    // Assert that the white shape is on top of the yellow one.
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, getProperty<Color>(getShape(2), u"FillColor"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(2), u"AnchorType"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<Color>(getShape(3), u"FillColor"_ustr));
    // Without the accompanying fix in place, this test would have failed with
    // 'expected: 4, actual: 2'.
    // This means the draw page was 0/at-char/white, 1/at-char/yellow, 2/at-page/white,
    // instead of the good 0/at-page/white, 1/at-char/yellow, 2/at-char/white.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(3), u"AnchorType"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testNestedHyperlink)
{
    // Given a hyperlink contains a footnote which contains a hyperlink:
    {
        createSwDoc();
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextContent> xFootnote(
            xFactory->createInstance(u"com.sun.star.text.Footnote"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XText> xText = xTextDocument->getText();
        uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
        xText->insertString(xCursor, u"a"_ustr, /*bAbsorb=*/false);
        xText->insertTextContent(xCursor, xFootnote, /*bAbsorb=*/false);
        xText->insertString(xCursor, u"b"_ustr, /*bAbsorb=*/false);
        xCursor->gotoStart(/*bExpand=*/false);
        xCursor->gotoEnd(/*bExpand=*/true);
        uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
        xCursorProps->setPropertyValue(u"HyperLinkURL"_ustr, uno::Any(u"http://body.com/"_ustr));
        uno::Reference<text::XText> xFootnoteText(xFootnote, uno::UNO_QUERY);
        xCursor = xFootnoteText->createTextCursor();
        xFootnoteText->insertString(xCursor, u"x"_ustr, /*bAbsorb=*/false);
        xCursor->gotoStart(/*bExpand=*/false);
        xCursor->gotoEnd(/*bExpand=*/true);
        xCursorProps.set(xCursor, uno::UNO_QUERY);
        xCursorProps->setPropertyValue(u"HyperLinkURL"_ustr,
                                       uno::Any(u"http://footnote.com/"_ustr));
    }

    // When exporting to RTF:
    // Without the accompanying fix in place, this test would have failed with:
    // assertion failed
    // - Expression: xComponent.is()
    // i.e. the RTF output was not well-formed, loading failed.
    saveAndReload(u"Rich Text Format"_ustr);

    // Then make sure both hyperlinks are have the correct URLs.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xPortion = getRun(xParagraph, 1);
    CPPUNIT_ASSERT_EQUAL(u"http://body.com/"_ustr,
                         getProperty<OUString>(xPortion, u"HyperLinkURL"_ustr));
    auto xFootnote
        = getProperty<uno::Reference<text::XText>>(getRun(xParagraph, 2), u"Footnote"_ustr);
    uno::Reference<text::XTextRange> xFootnotePortion = getRun(getParagraphOfText(1, xFootnote), 1);
    CPPUNIT_ASSERT_EQUAL(u"http://footnote.com/"_ustr,
                         getProperty<OUString>(xFootnotePortion, u"HyperLinkURL"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121623)
{
    auto verify = [this]() {
        // This was 2, multicolumn section was ignored at the table.
        CPPUNIT_ASSERT_EQUAL(1, getPages());
    };
    createSwDoc("tdf121623.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf66543)
{
    auto verify = [this]() {
        // Without the accompanying fix in place, this test would have failed with
        // 'Expected: 2; Actual  : 3' after import (off-by-one), then with
        // 'Expected: 2; Actual  : 0' (export not implemented).
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(
                                               getParagraph(1), u"ParaLineNumberStartValue"_ustr));
    };
    createSwDoc("tdf66543.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testUlw)
{
    auto verify = [this]() {
        // Test underlying in individual words mode.
        CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), u"CharWordMode"_ustr));
    };
    createSwDoc("ulw.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf122455)
{
    auto verify = [this]() {
        // Without the accompanying fix in place, this test would have failed with
        // 'Expected: 16; Actual  : 32', the font size from a list definition
        // leaked into the first run's character properties.
        CPPUNIT_ASSERT_EQUAL(16.0,
                             getProperty<double>(getRun(getParagraph(1), 1), u"CharHeight"_ustr));
    };
    createSwDoc("tdf122455.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125719_case_1)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                             getProperty<float>(getRun(getParagraph(1), 1), u"CharWeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                             getProperty<float>(getRun(getParagraph(3), 1), u"CharWeight"_ustr));
    };
    createSwDoc("tdf125719_case_1.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125719_case_2)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             getProperty<float>(getRun(getParagraph(1), 1), u"CharWeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             getProperty<float>(getRun(getParagraph(3), 1), u"CharWeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                             getProperty<float>(getRun(getParagraph(5), 1), u"CharWeight"_ustr));
    };
    createSwDoc("tdf125719_case_2.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTabs)
{
    auto verify = [this]() {
        // Test tab alignment in decimal mode.
        auto aTabStops
            = getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), u"ParaTabStops"_ustr);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aTabStops.getLength());
        const style::TabStop& rTabStop = aTabStops[0];
        CPPUNIT_ASSERT_EQUAL(style::TabAlign_DECIMAL, rTabStop.Alignment);
    };
    createSwDoc("tabs.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123703)
{
    auto verify = [this]() {
#if !defined(MACOSX)
        // This was 1, because of normal space character width in consecutive spaces
        CPPUNIT_ASSERT_EQUAL(2, getPages());
#else
        // still 1 here
        CPPUNIT_ASSERT_EQUAL(1, getPages());
#endif
    };
    createSwDoc("tdf123703.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123703_stshfdbch)
{
    auto verify = [this]() {
        // still 1 here
        CPPUNIT_ASSERT_EQUAL(1, getPages());
    };
    createSwDoc("tdf123703_stshfdbch.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123703_compatible)
{
    auto verify = [this]() {
#if !defined(MACOSX)
        // in the case of compatibility font id 31505
        CPPUNIT_ASSERT_EQUAL(2, getPages());
#else
        // still 1 here
        CPPUNIT_ASSERT_EQUAL(1, getPages());
#endif
    };
    createSwDoc("tdf123703_compatible.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128428_monospaced)
{
    auto verify = [this]() {
        // still 1 here
        CPPUNIT_ASSERT_EQUAL(1, getPages());
    };
    createSwDoc("tdf128428_monospaced.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128428_compatible_monospaced)
{
    auto verify = [this]() {
        // still 1 here
        CPPUNIT_ASSERT_EQUAL(1, getPages());
    };
    createSwDoc("tdf128428_compatible_monospaced.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128428_dntblnsbdb)
{
    auto verify = [this]() {
        // still 1 here
        CPPUNIT_ASSERT_EQUAL(1, getPages());
    };
    createSwDoc("tdf128428_dntblnsbdb.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testDontBreakWrappedTables)
{
    // Given a document with no DO_NOT_BREAK_WRAPPED_TABLES compat mode enabled:
    createSwDoc();
    {
        SwDoc* pDoc = getSwDoc();
        IDocumentSettingAccess& rIDSA = pDoc->getIDocumentSettingAccess();
        rIDSA.set(DocumentSettingId::DO_NOT_BREAK_WRAPPED_TABLES, true);
    }

    // When saving to rtf:
    saveAndReload(u"Rich Text Format"_ustr);

    // Then make sure \nobrkwrptbl is not written:
    SwDoc* pDoc = getSwDoc();
    IDocumentSettingAccess& rIDSA = pDoc->getIDocumentSettingAccess();
    bool bDontBreakWrappedTables = rIDSA.get(DocumentSettingId::DO_NOT_BREAK_WRAPPED_TABLES);
    // Without the accompanying fix in place, this test would have failed, the compat flag was not
    // set.
    CPPUNIT_ASSERT(bDontBreakWrappedTables);
}

CPPUNIT_TEST_FIXTURE(Test, testRtlGutter)
{
    auto verify = [this]() {
        uno::Reference<beans::XPropertySet> xStandard(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(getProperty<bool>(xStandard, u"RtlGutter"_ustr));
    };

    // Given a document with RTL gutter, when loading it:
    createSwDoc("rtl-gutter.rtf");
    // Then make sure the section's gutter is still RTL:
    // Without the accompanying fix in place, this test would have failed as \rtlgutter was missing.
    verify();
    saveAndReload(u"Rich Text Format"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testNegativePageBorder)
{
    {
        // Given a document with a top margin and a border which has more spacing than the margin on
        // its 2nd page:
        createSwDoc();
        SwDocShell* pDocShell = getSwDocShell();
        SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
        pWrtShell->Insert(u"first"_ustr);
        pWrtShell->SplitNode();
        pWrtShell->Insert(u"second"_ustr);
        SwPageDesc* pDesc = pWrtShell->FindPageDescByName(u"Left Page"_ustr, true);
        SwPaM aPaM(*pWrtShell->GetCursor()->GetPoint());
        SwFormatPageDesc aFormatPageDesc(pDesc);
        pDocShell->GetDoc()->getIDocumentContentOperations().InsertPoolItem(aPaM, aFormatPageDesc);
        uno::Reference<beans::XPropertySet> xPageStyle(
            getStyles(u"PageStyles"_ustr)->getByName(u"Left Page"_ustr), uno::UNO_QUERY);
        xPageStyle->setPropertyValue(u"TopMargin"_ustr, uno::Any(static_cast<sal_Int32>(501)));
        table::BorderLine2 aBorder;
        aBorder.LineWidth = 159;
        aBorder.OuterLineWidth = 159;
        xPageStyle->setPropertyValue(u"TopBorder"_ustr, uno::Any(aBorder));
        sal_Int32 nTopBorderDistance = -646;
        xPageStyle->setPropertyValue(u"TopBorderDistance"_ustr, uno::Any(nTopBorderDistance));
    }

    // When saving that document to RTF:
    saveAndReload(u"Rich Text Format"_ustr);

    // Then make sure that the border distance is negative, so the first line of body text appears
    // on top of the page border:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    OUString aPageStyle = pWrtShell->GetCurPageStyle();
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(aPageStyle), uno::UNO_QUERY);
    auto nTopMargin = xPageStyle->getPropertyValue(u"TopMargin"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(501), nTopMargin);
    auto aTopBorder = xPageStyle->getPropertyValue(u"TopBorder"_ustr).get<table::BorderLine2>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(159), aTopBorder.LineWidth);
    auto nTopBorderDistance
        = xPageStyle->getPropertyValue(u"TopBorderDistance"_ustr).get<sal_Int32>();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -646
    // - Actual  : 0
    // i.e. the border negative distance was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-646), nTopBorderDistance);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127806)
{
    createSwDoc("tdf127806.rtf");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.GroupShape"_ustr, getShape(1)->getShapeType());
    auto xImage = getShape(2);
    CPPUNIT_ASSERT_EQUAL(u"FrameShape"_ustr, xImage->getShapeType());
    awt::Size aSize(xImage->getSize());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(600), aSize.Height);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(635), aSize.Width);

    saveAndReload(u"Rich Text Format"_ustr);
    CPPUNIT_ASSERT_EQUAL(1, getShapes()); // FIXME: We lost one shape on export, that's sucks

    xImage = getShape(1);
    CPPUNIT_ASSERT_EQUAL(u"FrameShape"_ustr, xImage->getShapeType());

    aSize = xImage->getSize();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(600), aSize.Height);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(635), aSize.Width);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148578)
{
    auto verify = [this]() {
        // \trgaph567 should affect only table cell margins (~1cm),
        // but do not shift table, since \trleft is not provided
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);

        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                             getProperty<sal_Int32>(xTable, u"LeftMargin"_ustr));

        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1000),
                             getProperty<sal_Int32>(xCell, u"LeftBorderDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1000),
                             getProperty<sal_Int32>(xCell, u"RightBorderDistance"_ustr));

        xCell.set(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1000),
                             getProperty<sal_Int32>(xCell, u"LeftBorderDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1000),
                             getProperty<sal_Int32>(xCell, u"RightBorderDistance"_ustr));
    };
    createSwDoc("tdf148578.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testInvalidParagraphStyle)
{
    auto verify = [this]() {
        // Given test has character style #30, but referred as paragraph style #30
        // This was causing exception in finishParagraph(), so numbering and other
        // properties were not applied. Ensure numbering is still here
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::ARABIC, numFormat);
    };
    createSwDoc("invalidParagraphStyle.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152784_1)
{
    auto verify = [this]() {
        // Ensure that paragraph having style with numbering does not have numbering
        // since it is not explicitly defined in paragraph properties
        uno::Reference<beans::XPropertySet> xPara(
            getParagraph(1, u"Here should be no numbering!"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(getProperty<OUString>(xPara, u"NumberingStyleName"_ustr).isEmpty());
    };
    createSwDoc("tdf152784_1.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableExport)
{
    // Given a document with a floating table:
    createSwDoc();
    // Insert a table:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"Rows"_ustr, static_cast<sal_Int32>(1)),
        comphelper::makePropertyValue(u"Columns"_ustr, static_cast<sal_Int32>(1)),
    };
    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);
    // Select it:
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    // Wrap in a fly:
    aArgs = {
        comphelper::makePropertyValue(u"AnchorType"_ustr, static_cast<sal_uInt16>(0)),
    };
    dispatchCommand(mxComponent, u".uno:InsertFrame"_ustr, aArgs);
    // Mark it as a floating table:
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(
        xTextFramesSupplier->getTextFrames()->getByName(u"Frame1"_ustr), uno::UNO_QUERY);
    xFrame->setPropertyValue(u"IsSplitAllowed"_ustr, uno::Any(true));
    // Originally 10, 30 & 40 twips.
    xFrame->setPropertyValue(u"VertOrientPosition"_ustr, uno::Any(static_cast<sal_Int32>(18)));
    xFrame->setPropertyValue(u"LeftMargin"_ustr, uno::Any(static_cast<sal_Int32>(53)));
    xFrame->setPropertyValue(u"RightMargin"_ustr, uno::Any(static_cast<sal_Int32>(71)));

    // When saving to RTF:
    saveAndReload(u"Rich Text Format"_ustr);

    // Then make sure the floating table is there & has the expected properties:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    xFrame.set(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    bool bIsSplitAllowed{};
    xFrame->getPropertyValue(u"IsSplitAllowed"_ustr) >>= bIsSplitAllowed;
    // Without the accompanying fix in place, this test would have failed, the table was not
    // multi-page.
    CPPUNIT_ASSERT(bIsSplitAllowed);
    sal_Int16 nVertOrientRelation{};
    xFrame->getPropertyValue(u"VertOrientRelation"_ustr) >>= nVertOrientRelation;
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, nVertOrientRelation);
    sal_Int16 nHoriOrientRelation{};
    xFrame->getPropertyValue(u"HoriOrientRelation"_ustr) >>= nHoriOrientRelation;
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, nHoriOrientRelation);
    sal_Int32 nVertOrientPosition{};
    xFrame->getPropertyValue(u"VertOrientPosition"_ustr) >>= nVertOrientPosition;
    sal_Int32 nExpected = 18;
    CPPUNIT_ASSERT_EQUAL(nExpected, nVertOrientPosition);
    sal_Int32 nLeftMargin{};
    xFrame->getPropertyValue(u"LeftMargin"_ustr) >>= nLeftMargin;
    nExpected = 53;
    CPPUNIT_ASSERT_EQUAL(nExpected, nLeftMargin);
    sal_Int32 nRightMargin{};
    xFrame->getPropertyValue(u"RightMargin"_ustr) >>= nRightMargin;
    nExpected = 71;
    CPPUNIT_ASSERT_EQUAL(nExpected, nRightMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableOverlapNeverRTFExport)
{
    // Given a document with a floating table, overlap is not allowed:
    {
        createSwDoc();
        SwDoc* pDoc = getSwDoc();
        SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
        pWrtShell->Insert2(u"before table"_ustr);
        // Insert a table:
        SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
        pWrtShell->InsertTable(aTableOptions, /*nRows=*/1, /*nCols=*/1);
        pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
        // Select table:
        pWrtShell->SelAll();
        // Wrap the table in a text frame:
        SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
        pWrtShell->StartAllAction();
        aMgr.InsertFlyFrame(RndStdIds::FLY_AT_PARA, aMgr.GetPos(), aMgr.GetSize());
        pWrtShell->EndAllAction();
        // Allow the text frame to split:
        pWrtShell->StartAllAction();
        sw::FrameFormats<sw::SpzFrameFormat*>* pFlys = pDoc->GetSpzFrameFormats();
        sw::SpzFrameFormat* pFly = (*pFlys)[0];
        SwAttrSet aSet(pFly->GetAttrSet());
        aSet.Put(SwFormatFlySplit(true));
        // Don't allow overlap:
        SwFormatWrapInfluenceOnObjPos aInfluence;
        aInfluence.SetAllowOverlap(false);
        aSet.Put(aInfluence);
        pDoc->SetAttr(aSet, *pFly);
        pWrtShell->EndAllAction();
    }

    // When saving to RTF:
    saveAndReload(u"Rich Text Format"_ustr);

    // Then make sure that the overlap=never markup is written:
    SwDoc* pDoc = getSwDoc();
    sw::FrameFormats<sw::SpzFrameFormat*>* pFlys = pDoc->GetSpzFrameFormats();
    sw::SpzFrameFormat* pFly = (*pFlys)[0];
    // Without the accompanying fix in place, this test would have failed, i.e. \tabsnoovrlp was not
    // written.
    CPPUNIT_ASSERT(!pFly->GetAttrSet().GetWrapInfluenceOnObjPos().GetAllowOverlap());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158409)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1, u"DocTitle"_ustr);
        CPPUNIT_ASSERT_EQUAL(8.0, getProperty<double>(xRun, u"CharHeight"_ustr));

        xRun = getRun(getParagraph(2), 1, u"DocTitle"_ustr);
        CPPUNIT_ASSERT_EQUAL(8.0, getProperty<double>(xRun, u"CharHeight"_ustr));
    };
    createSwDoc("tdf158409.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testLegalNumbering)
{
    auto verify = [this]() {
        // Second level's numbering should use Arabic numbers for first level reference
        auto xPara = getParagraph(1);
        CPPUNIT_ASSERT_EQUAL(u"CH I"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        xPara = getParagraph(2);
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: Sect 1.01
        // - Actual  : Sect I.01
        // i.e. \levellegal was ignored on import/export.
        CPPUNIT_ASSERT_EQUAL(u"Sect 1.01"_ustr,
                             getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        xPara = getParagraph(3);
        CPPUNIT_ASSERT_EQUAL(u"CH II"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        xPara = getParagraph(4);
        CPPUNIT_ASSERT_EQUAL(u"Sect 2.01"_ustr,
                             getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    };

    createSwDoc("listWithLgl.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
