/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>

#include <svx/svdpage.hxx>
#include <o3tl/string_view.hxx>

#include <ftninfo.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <sortedobjs.hxx>
#include <cntfrm.hxx>
#include <anchoredobject.hxx>
#include <tabfrm.hxx>
#include <flyfrms.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ww8export/data/"_ustr, u"MS Word 97"_ustr) {}
};

DECLARE_WW8EXPORT_TEST(testTdf99120, "tdf99120.doc")
{
    CPPUNIT_ASSERT_EQUAL(u"Section 1, odd."_ustr,  parseDump("/root/page[1]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Section 1, even."_ustr,  parseDump("/root/page[2]/header/txt/text()"_ostr));
    // This failed: the header was empty on the 3rd page, as the first page header was shown.
    CPPUNIT_ASSERT_EQUAL(u"Section 2, odd."_ustr,  parseDump("/root/page[3]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Section 2, even."_ustr,  parseDump("/root/page[4]/header/txt/text()"_ostr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf41542_borderlessPadding)
{
    loadAndReload("tdf41542_borderlessPadding.odt");
    // the page style's borderless padding should force this to 3 pages, not 1
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );
}

DECLARE_WW8EXPORT_TEST(testTdf60378_mergedBorders, "tdf60378_mergedBorders.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // The border width was zero
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(xTable->getCellByName(u"B2"_ustr), u"RightBorder"_ustr);
    CPPUNIT_ASSERT(aBorder.LineWidth > 0);
}

DECLARE_WW8EXPORT_TEST(testTdf55528_relativeTableWidth, "tdf55528_relativeTableWidth.doc")
{

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table relative width percent", sal_Int16(98), getProperty<sal_Int16>(xTable, u"RelativeWidth"_ustr));
 }

CPPUNIT_TEST_FIXTURE(Test, testTdf128700_relativeTableWidth)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

        // Since the table has been converted into a floating frame, the relative width either needed to be transferred
        // onto the frame, or else just thrown out. Otherwise it becomes relative to the size of the frame.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Floated table can't use relative width", sal_Int16(0), getProperty<sal_Int16>(xTable, u"RelativeWidth"_ustr));
    };
    // This also resulted in a layout loop when flys were allowed to split in footers.
    createSwDoc("tdf128700_relativeTableWidth.doc");
    verify();
    saveAndReload(u"MS Word 97"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116436_tableBackground)
{
    loadAndReload("tdf116436_tableBackground.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A1"_ustr);
    CPPUNIT_ASSERT_EQUAL(Color(0xF8DF7C), getProperty<Color>(xCell, u"BackColor"_ustr));
    xCell.set(xTable->getCellByName(u"A6"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0x81D41A), getProperty<Color>(xCell, u"BackColor"_ustr));
    xCell.set(xTable->getCellByName(u"B6"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0xFFFBCC), getProperty<Color>(xCell, u"BackColor"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf37153, "tdf37153_considerWrapOnObjPos.doc")
{
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH, getProperty<text::WrapTextMode>(getShape(1), u"Surround"_ustr));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::BOTTOM, getProperty<sal_Int16>(xTable->getCellByName(u"A1"_ustr), u"VertOrient"_ustr));

    //For MSO compatibility, the image should be at the top of the cell, not at the bottom - despite VertOrientation::BOTTOM
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFlyTop  = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt/anchored/fly/infos/bounds"_ostr, "top"_ostr).toInt32();
    CPPUNIT_ASSERT_MESSAGE("FlyTop should be 3820, not 6623", nFlyTop < 4000);
    sal_Int32 nTextTop  = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt[1]/infos/bounds"_ostr, "top"_ostr).toInt32();
    CPPUNIT_ASSERT_MESSAGE("TextTop should be 5388", nTextTop > 4000);
}

DECLARE_WW8EXPORT_TEST(testTdf49102_mergedCellNumbering, "tdf49102_mergedCellNumbering.doc")
{
    CPPUNIT_ASSERT_EQUAL( u"2."_ustr, parseDump("/root/page/body/tab/row[4]/cell/txt/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr, "expand"_ostr) );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf55427_footnote2endnote)
{
    loadAndReload("tdf55427_footnote2endnote.odt");
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Footnote"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Footnote style is rose color", Color(0xFF007F), getProperty< Color >(xPageStyle, u"CharColor"_ustr));
    xPageStyle.set(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Endnote"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Endnote style is cyan3 color", Color(0x2BD0D2), getProperty< Color >(xPageStyle, u"CharColor"_ustr));

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    // The footnote numbering type of ARABIC will not transfer over when those footnotes are converted to endnotes.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Footnote numbering type", SVX_NUM_ARABIC, pDoc->GetFootnoteInfo().m_aFormat.GetNumberingType() );
    // The original document has a real endnote using ROMAN_LOWER numbering, so that setting MUST remain unchanged.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Endnote numbering type", SVX_NUM_ROMAN_LOWER, pDoc->GetEndNoteInfo().m_aFormat.GetNumberingType() );

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();

    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes = xEndnotesSupplier->getEndnotes();
    uno::Reference<text::XFootnote> xEndnote;
    xEndnotes->getByIndex(0) >>= xEndnote;
    uno::Reference<text::XText> xEndnoteText;
    xEndnotes->getByIndex(0) >>= xEndnoteText;

    // ODT footnote-at-document-end's closest DOC match is an endnote, so the two imports will not exactly match by design.
    if (!isExported())
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote count", sal_Int32(5), xFootnotes->getCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote count", sal_Int32(1), xEndnotes->getCount() );

        uno::Reference<text::XFootnote> xFootnote;
        xFootnotes->getByIndex(0) >>= xFootnote;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote's number", u"1"_ustr, xFootnote->getAnchor()->getString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote's number", u"i"_ustr, xEndnote->getAnchor()->getString() );

        uno::Reference<text::XText> xFootnoteText;
        xFootnotes->getByIndex(0) >>= xFootnoteText;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote style", u"Footnote"_ustr, getProperty<OUString>(getParagraphOfText(1, xFootnoteText), u"ParaStyleName"_ustr) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote style", u"Endnote"_ustr, getProperty<OUString>(getParagraphOfText(1, xEndnoteText), u"ParaStyleName"_ustr) );
    }
    else
    {
        // These asserted items are major differences in the conversion from footnote to endnote, NOT necessary conditions for a proper functioning document.
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "At-Document-End footnotes were converted into endnotes", sal_Int32(0), xFootnotes->getCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "At-Document-End footnotes became endnotes", sal_Int32(6), xEndnotes->getCount() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "converted footnote's number", u"i"_ustr, xEndnote->getAnchor()->getString() );
        xEndnotes->getByIndex(4) >>= xEndnote;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote's new number", u"v"_ustr, xEndnote->getAnchor()->getString() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "retained footnote style", u"Footnote"_ustr, getProperty<OUString>(getParagraphOfText(1, xEndnoteText), u"ParaStyleName"_ustr) );
        xEndnotes->getByIndex(4) >>= xEndnoteText;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote style", u"Endnote"_ustr, getProperty<OUString>(getParagraphOfText(1, xEndnoteText), u"ParaStyleName"_ustr) );
    }
}

DECLARE_WW8EXPORT_TEST(testTdf107931_KERN_DocEnabled_disabledDefStyle, "testTdf107931_KERN_DocEnabled_disabledDefStyle.doc")
{
    // Paragraph 3: the default style has kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(3), 1), u"CharAutoKerning"_ustr));
    // Paragraph 4: style with kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(4), 1), u"CharAutoKerning"_ustr));
    // Paragraph 5: style with kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(5), 1), u"CharAutoKerning"_ustr));
    // Paragraph 6: directly applied character properties: kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(6), 1), u"CharAutoKerning"_ustr));
    // Paragraph 7: directly applied character properties: kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(7), 1), u"CharAutoKerning"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf107931_KERN_enabledDefStyle, "testTdf107931_KERN_enabledDefStyle.doc")
{
    // Paragraph 3: the default style has kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(3), 1), u"CharAutoKerning"_ustr));
    // Paragraph 4: style with kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(4), 1), u"CharAutoKerning"_ustr));
    // Paragraph 5: style with kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(5), 1), u"CharAutoKerning"_ustr));
    // Paragraph 6: directly applied character properties: kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(6), 1), u"CharAutoKerning"_ustr));
    // Paragraph 7: directly applied character properties: kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(7), 1), u"CharAutoKerning"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf89377, "tdf89377_tableWithBreakBeforeParaStyle.doc")
{
    // the paragraph style should set table's text-flow break-before-page
    CPPUNIT_ASSERT_EQUAL( 2, getPages() );
}

DECLARE_WW8EXPORT_TEST(testTdf96277, "tdf96277.doc")
{
    // mismatch between anchoring at paragraph and ULSpacing to Page
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

DECLARE_WW8EXPORT_TEST(testBnc863018b, "bnc863018b.doc")
{
    // The whitespace above the table should allow text to flow between the table anchor and the table.
    // Since it doesn't, don't add the whitespace.
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

DECLARE_WW8EXPORT_TEST(testTdf112517_maxSprms, "tdf112517_maxSprms.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(28), xTable->getRows()->getCount() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108448_endNote)
{
    loadAndReload("tdf108448_endNote.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes = xEndnotesSupplier->getEndnotes();
    uno::Reference<text::XText> xEndnote;
    xEndnotes->getByIndex(0) >>= xEndnote;

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of paragraphs in Endnote i", 1, getParagraphs(xEndnote) );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106062_nonHangingFootnote)
{
    loadAndReload("tdf106062_nonHangingFootnote.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This failed, tab between the footnote number and the footnote content was lost on import.
    CPPUNIT_ASSERT_MESSAGE( "Footnote starts with a tab", xTextRange->getString().startsWith("\t") );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116570_exportFootnote)
{
    loadAndReload("tdf116570_exportFootnote.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of paragraphs in first footnote", 2, getParagraphs(xFootnoteText) );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf80635_pageRightRTL)
{
    auto verify = [this]() {
        // tdf#80635 - assert horizontal position of the table.
        uno::Reference<drawing::XShape> xFly = getShape(1);
        CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xFly, u"HoriOrientRelation"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Horizontal Orientation", text::HoriOrientation::RIGHT, getProperty<sal_Int16>(xFly, u"HoriOrient"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("text probably does not wrap here", 1, getPages());
    };
    createSwDoc("tdf80635_pageRightRTL.doc");
    verify();
    saveAndReload(u"MS Word 97"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf80635_marginRTL)
{
    auto verify = [this]() {
        // tdf#80635 - assert the horizontal orientation of the table.
        uno::Reference<drawing::XShape> xFly = getShape(1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Horizontal Orientation", text::HoriOrientation::RIGHT, getProperty<sal_Int16>(xFly, u"HoriOrient"_ustr));
    };
    createSwDoc("tdf80635_marginRightRTL.doc");
    verify();
    saveAndReload(u"MS Word 97"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf80635_marginLeft)
{
    auto verify = [this]() {
        // tdf#80635 - assert horizontal position of the table.
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Table Indent", tools::Long(0), getProperty<tools::Long>(xTable, u"LeftMargin"_ustr), 100);
        uno::Reference<drawing::XShape> xFly = getShape(1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-2958), getProperty<sal_Int32>(xFly, u"HoriOrientPosition"_ustr));
    };
    createSwDoc("tdf80635_marginLeft.doc");
    verify();
    saveAndReload(u"MS Word 97"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf80635_pageLeft)
{
    auto verify = [this]() {
        // tdf#80635 - assert horizontal orient relation of the table.
        uno::Reference<drawing::XShape> xFly = getShape(1);
        CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xFly, u"HoriOrientRelation"_ustr));
        CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::NONE, getProperty<sal_Int16>(xFly, u"HoriOrient"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-189), getProperty<sal_Int32>(xFly, u"HoriOrientPosition"_ustr));
    };
    createSwDoc("tdf80635_pageLeft.doc");
    verify();
    saveAndReload(u"MS Word 97"_ustr);
    verify();
}

DECLARE_WW8EXPORT_TEST(testTdf99197_defaultLTR, "tdf99197_defaultLTR.doc")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Default Paragraph style, LTR",
        text::WritingMode2::LR_TB, getProperty<sal_Int16>(getParagraph(1), u"WritingMode"_ustr) );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "myDefaultStyle - no base style, LTR",
        text::WritingMode2::LR_TB, getProperty<sal_Int16>(getParagraph(2), u"WritingMode"_ustr) );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107773)
{
    auto verify = [this]() {
        // This failed, multi-page table was imported as a non-split frame.
        SwDoc* pDoc = getSwDoc();
        SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
        auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
        CPPUNIT_ASSERT(pPage1);
        // pPage1 has no sorted (floating) objections.
        CPPUNIT_ASSERT(pPage1->GetSortedObjs());
        const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
        auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
        CPPUNIT_ASSERT(pPage1Fly);
        auto pTab1 = dynamic_cast<SwTabFrame*>(pPage1Fly->GetLower());
        CPPUNIT_ASSERT(pTab1);
        // This failed, the split fly containing a table was exported back to DOC as shape+table,
        // which can't split.
        CPPUNIT_ASSERT(pTab1->HasFollow());

        // tdf#80635 - assert the horizontal orientation.
        const SwFormatHoriOrient& rFormatHoriOrient = pPage1Fly->GetFormat()->GetHoriOrient();
        CPPUNIT_ASSERT_EQUAL(css::text::HoriOrientation::CENTER, rFormatHoriOrient.GetHoriOrient());
    };
    createSwDoc("tdf107773.doc");
    verify();
    saveAndReload(u"MS Word 97"_ustr);
    verify();
}

DECLARE_WW8EXPORT_TEST(testTdf112074_RTLtableJustification, "tdf112074_RTLtableJustification.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Right To Left writing mode", text::WritingMode2::RL_TB, getProperty<sal_Int16>(xTable, u"WritingMode"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Horizontal Orientation", text::HoriOrientation::LEFT_AND_WIDTH, getProperty<sal_Int16>(xTable, u"HoriOrient"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Table Indent", getProperty<tools::Long>(xTable, u"LeftMargin"_ustr) > 3000);
    CPPUNIT_ASSERT_MESSAGE("Table Indent is 3750", getProperty<tools::Long>(xTable, u"LeftMargin"_ustr) < 4000 );
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_RIGHT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraphOrTable(2), u"ParaAdjust"_ustr)) );
}

DECLARE_WW8EXPORT_TEST(testTdf98620_rtlJustify, "tdf98620_rtlJustify.doc")
{
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_RIGHT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), u"ParaAdjust"_ustr)) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Right To Left style", text::WritingMode2::RL_TB, getProperty<sal_Int16>(getParagraph(1), u"WritingMode"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf121110_absJustify, "tdf121110_absJustify.doc")
{
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_RIGHT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), u"ParaAdjust"_ustr)) );
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_LEFT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(3), u"ParaAdjust"_ustr)) );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106174_rtlParaAlign)
{
    loadAndReload("tdf106174_rtlParaAlign.docx");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER), getProperty<sal_Int16>(getParagraph(1), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER), getProperty<sal_Int16>(getParagraph(2), u"ParaAdjust"_ustr));
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Another paragraph aligned to right"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(xPropertySet, u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(3), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(4), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(5), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(6), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(7), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(8), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(9), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(10), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(11), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(12), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(13), u"ParaAdjust"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(14), u"ParaAdjust"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf119232_startEvenPage, "tdf119232_startEvenPage.doc")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), getProperty<sal_Int16>(getParagraph(1), u"PageNumberOffset"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf104805, "tdf104805.doc")
{
    // Prefix was "." instead of empty, so the second paragraph was
    // rendered as ".1" instead of "1.".
    // Unittest modified due to Prefix/Suffix support obsolete
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf104334, "tdf104334.doc")
{
    // This failed with a container::NoSuchElementException: STYLEREF was
    // mapped to SwChapterField, and the field result was "This is a Heading 1"
    // instead of just "1".
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getRun(getParagraph(2), 4)->getString());
}

DECLARE_WW8EXPORT_TEST(testTdf108072, "tdf108072.doc")
{
    // The property IsSplitAllowed was imported from an obsolete property, sprmTFCantSplit90
    // instead of sprmTFCantSplit. sprmTFCantSplit90 is set to true for merged rows, so
    // for merged rows incorrect settings were imported, which prevented them from breaking over pages.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTableRows = xTable->getRows();
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTableRows->getByIndex(0), u"IsSplitAllowed"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf123321, "shapes-line-ellipse.doc")
{
    // These are the 3 lines in which 1st and 3rd one were disappearing before
    uno::Reference<drawing::XShape> l1 = getShape(7);
    uno::Reference<drawing::XShape> l2 = getShape(8);
    uno::Reference<drawing::XShape> l3 = getShape(9);

    // first line (smallest)
    // Fails without the fix: Expected: 423, Actual: 2
    CPPUNIT_ASSERT_EQUAL(sal_Int32(423), l1->getSize().Height);
    // Fails without the fix: Expected: 0, Actual: 2
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), l1->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7908), l1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(37), l1->getPosition().Y);

    // second line (larger)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2542), l2->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), l2->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7916), l2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(289), l2->getPosition().Y);

    // third line (largest)
    // Fails without the fix: Expected: 7027, Actual: 2
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7027), l3->getSize().Height);
    // Fails without the fix: Expected: 0, Actual: 2
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), l3->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7911), l3->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(231), l3->getPosition().Y);
}

DECLARE_WW8EXPORT_TEST(testTdf91687, "tdf91687.doc")
{
    // Exported Watermarks were resized
    uno::Reference<drawing::XShape> xWatermark = getShape(1);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(5172), xWatermark->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18105), xWatermark->getSize().Width);
}

DECLARE_WW8EXPORT_TEST(testTdf114308, "tdf114308.doc")
{
    // Watermark with no additional padding
    uno::Reference<drawing::XShape> xWatermark = getShape(1);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(8729), xWatermark->getSize().Height);
}

DECLARE_WW8EXPORT_TEST(testTdf111480, "tdf111480.doc")
{
    // Circular text was imported horizontally
    uno::Reference<drawing::XShape> xText = getShape(1);

    CPPUNIT_ASSERT(xText->getSize().Height > 11000);
    CPPUNIT_ASSERT(xText->getSize().Width  > 11000);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf70838)
{
    loadAndReload("tdf70838.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    tools::Rectangle aRect = pPage->GetObj(0)->GetSnapRect();
    CPPUNIT_ASSERT(aRect.GetHeight() > aRect.GetWidth());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf70838b_verticalRotation)
{
    loadAndReload("tdf70838b_verticalRotation.odt");
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    tools::Rectangle aGroupShape = pPage->GetObj(0)->GetSnapRect();
    tools::Rectangle aLine = pPage->GetObj(2)->GetSnapRect();

    CPPUNIT_ASSERT_MESSAGE("Smiley faces are round", aGroupShape.GetHeight() > aGroupShape.GetWidth());
    CPPUNIT_ASSERT_MESSAGE("Line is taller, not wider", aLine.GetHeight() > aLine.GetWidth());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129247)
{
    loadAndReload("tdf129247.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Without the fix in place, the checkbox wouldn't be exported
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(Test, testActiveXCheckbox)
{
    loadAndReload("checkbox_control.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // First check box anchored as a floating object
    uno::Reference<drawing::XControlShape> xControlShape;
    if(!isExported())
        xControlShape.set(getShape(1), uno::UNO_QUERY);
    else
        xControlShape.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check whether we have the right control
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService( u"com.sun.star.form.component.CheckBox"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"Floating Checkbox"_ustr, getProperty<OUString>(xPropertySet, u"Label"_ustr));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,u"AnchorType"_ustr));

    // Second check box anchored inline / as character
    if(!isExported())
        xControlShape.set(getShape(2), uno::UNO_QUERY);
    else
        xControlShape.set(getShape(1), uno::UNO_QUERY);

    // Check whether we have the right control
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService(u"com.sun.star.form.component.CheckBox"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"Inline Checkbox"_ustr, getProperty<OUString>(xPropertySet, u"Label"_ustr));

    // Check anchor type
    xPropertySet2.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,u"AnchorType"_ustr));
}

DECLARE_WW8EXPORT_TEST( testTdf115896_layoutInCell, "tdf115896_layoutInCell.doc" )
{
    // Check anchor type - was anchored to page because of unknown version of Word
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet,u"AnchorType"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf67207_MERGEFIELD, "mailmerge.doc")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), u"TextField"_ustr);
    CPPUNIT_ASSERT(xTextField.is());
    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY_THROW);
    uno::Reference<text::XDependentTextField> xDependent(xTextField, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextField.Database"_ustr));
    OUString sValue;
    xTextField->getPropertyValue(u"Content"_ustr) >>= sValue;
    CPPUNIT_ASSERT_EQUAL(u"«Name»"_ustr, sValue);

    uno::Reference<beans::XPropertySet> xFiledMaster = xDependent->getTextFieldMaster();
    uno::Reference<lang::XServiceInfo> xFiledMasterServiceInfo(xFiledMaster, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xFiledMasterServiceInfo->supportsService(u"com.sun.star.text.fieldmaster.Database"_ustr));

    // Defined properties: DataBaseName, Name, DataTableName, DataColumnName, DependentTextFields, DataCommandType, InstanceName, DataBaseURL
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"Name"_ustr) >>= sValue);
    CPPUNIT_ASSERT_EQUAL(u"Name"_ustr, sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"DataColumnName"_ustr) >>= sValue);
    CPPUNIT_ASSERT_EQUAL(u"Name"_ustr, sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"InstanceName"_ustr) >>= sValue);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.text.fieldmaster.DataBase.Name"_ustr, sValue);
}

DECLARE_OOXMLEXPORT_TEST( testTableCrossReference, "table_cross_reference.odt" )
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // tdf#42346: Cross references to tables were not saved
    // MSO uses simple bookmarks for referencing table caption, so we do the same by export
    if (!isExported())
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_number_only"_ustr));

    // Check bookmark text ranges
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table0_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table 1: Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table0_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table 1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table0_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table0_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xRange->getString());
    }

    // Check reference fields
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());

    sal_uInt16 nIndex = 0;
    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        switch (nIndex)
        {
            // Full reference to table caption
            case 0:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Table 1: Table caption"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_full"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to table number
            case 1:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_number_only"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to caption only
            case 2:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_caption_only"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to category and number
            case 3:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Table 1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_label_and_number"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to page of the table
            case 4:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_full"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Page style reference / exported as simple page reference
            case 5:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_full"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Above / below reference
            case 6:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"above"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_full"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            default:
                break;
        }
        ++nIndex;
    }
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), nIndex);
}

CPPUNIT_TEST_FIXTURE(Test, testTableCrossReferenceCustomFormat)
{
    loadAndReload("table_cross_reference_custom_format.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // tdf#42346: Cross references to tables were not saved
    // Check also captions with custom formatting
    if (!isExported())
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(16), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table1_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table1_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table1_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table1_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table2_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table2_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table2_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table2_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table3_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table3_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table3_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table3_number_only"_ustr));

    // Check bookmark text ranges
    // First table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table0_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1. Table: Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table0_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1. Table"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table0_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table0_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xRange->getString());
    }
    // Second table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table1_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"2. TableTable caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table1_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"2. Table"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table1_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table1_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, xRange->getString());
    }
    // Third table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table2_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"3) Table Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table2_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"3) Table"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table2_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table2_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"3"_ustr, xRange->getString());
    }
    // Fourth table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table3_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table 4- Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table3_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table 4"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table3_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Table3_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"4"_ustr, xRange->getString());
    }
}

DECLARE_OOXMLEXPORT_TEST( testObjectCrossReference, "object_cross_reference.odt" )
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    // tdf#42346: Cross references to objects were not saved
    // MSO uses simple bookmarks for referencing table caption, so we do the same by export
    if (!isExported())
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(15), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Drawing0_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Drawing0_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Drawing0_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Drawing0_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Drawing1_full"_ustr));

    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Illustration0_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Illustration0_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Illustration0_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Illustration0_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Illustration1_caption_only"_ustr));

    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Text0_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Text0_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Text0_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Text0_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Text1_label_and_number"_ustr));

    // Check bookmark text ranges
    // Cross references to shapes
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Drawing0_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Drawing 1: A rectangle"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Drawing0_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Drawing 1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Drawing0_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"A rectangle"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Drawing0_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Drawing1_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Drawing 2: a circle"_ustr, xRange->getString());
    }

    // Cross references to pictures
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Illustration0_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Illustration 1: A picture"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Illustration0_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Illustration 1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Illustration0_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"A picture"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Illustration0_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Illustration1_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"another image"_ustr, xRange->getString());
    }

    // Cross references to text frames
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Text0_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Text 1: A frame"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Text0_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Text 1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Text0_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"A frame"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Text0_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName(u"Ref_Text1_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Text 2"_ustr, xRange->getString());
    }
}

DECLARE_WW8EXPORT_TEST(testTdf112118_DOC, "tdf112118.doc")
{
    static const struct {
        const char* styleName;
        struct {
            const char* sideName;
            sal_Int32 nMargin;
            sal_Int32 nBorderDistance;
            sal_Int32 nBorderWidth;
        } sideParams[4];
    } styleParams[] = {                      // Margin (MS-style), border distance, border width
        {
            "Standard",
            {
                { "Top", 496, 847, 159 },    //  851 twip, 24 pt (from text), 4.5 pt
                { "Left", 2083, 706, 212 },  // 1701 twip, 20 pt (from text), 6.0 pt
                { "Bottom", 1401, 564, 35 }, // 1134 twip, 16 pt (from text), 1.0 pt
                { "Right", 3471, 423, 106 }  // 2268 twip, 12 pt (from text), 3.0 pt
            }
        },
        {
            "Convert 1",
            {
                { "Top", 847, 496, 159 },    //  851 twip, 24 pt (from edge), 4.5 pt
                { "Left", 706, 2083, 212 },  // 1701 twip, 20 pt (from edge), 6.0 pt
                { "Bottom", 564, 1401, 35 }, // 1134 twip, 16 pt (from edge), 1.0 pt
                { "Right", 423, 3471, 106 }  // 2268 twip, 12 pt (from edge), 3.0 pt
            }
        }
    };
    auto xStyles = getStyles(u"PageStyles"_ustr);

    for (const auto& style : styleParams)
    {
        const OUString sName = OUString::createFromAscii(style.styleName);
        uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName(sName), uno::UNO_QUERY_THROW);
        for (const auto& side : style.sideParams)
        {
            const OUString sSide = OUString::createFromAscii(side.sideName);
            const OString sStage = style.styleName + OString::Concat(" ") + side.sideName;

            sal_Int32 nMargin = getProperty<sal_Int32>(xStyle, sSide + "Margin");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " margin width").getStr(),
                side.nMargin, nMargin);

            sal_Int32 nBorderDistance = getProperty<sal_Int32>(xStyle, sSide + "BorderDistance");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " border distance").getStr(),
                side.nBorderDistance, nBorderDistance);

            table::BorderLine aBorder = getProperty<table::BorderLine>(xStyle, sSide + "Border");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " border width").getStr(),
                side.nBorderWidth,
                sal_Int32(aBorder.OuterLineWidth + aBorder.InnerLineWidth + aBorder.LineDistance));

            // Check that AUTO border color is imported as black
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " border color").getStr(),
                sal_Int32(COL_BLACK), aBorder.Color);
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf117503)
{
    loadAndReload("tdf117503.docx");
    // This was 3, first page + standard page styles were not merged together
    // on export.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_WW8EXPORT_TEST(testTdf117885, "tdf117885.doc")
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    /* Get the vertical position of the paragraph containing the text "Start" */
    sal_Int32 nParaA_Top = getXPath(pXmlDoc,
        "/root/page/body/column[1]/body/txt[text()='Start']/infos/bounds"_ostr, "top"_ostr
        ).toInt32();

    /* Get the vertical position of the paragraph containing the text "Top B" */
    sal_Int32 nParaB_Top = getXPath(pXmlDoc,
        "/root/page/body/column[2]/body/txt[text()='Top B']/infos/bounds"_ostr, "top"_ostr
        ).toInt32();

    /* These two paragraphs are supposed to be at the top of the left
     * and right columns respectively.  Check that they actually line up: */
    CPPUNIT_ASSERT_EQUAL(nParaA_Top, nParaB_Top);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf118133)
{
    loadAndReload("tdf118133.docx");
    // This was 0, doc import + doc export resulted in lost image due to broken
    // lazy-loading of tiff images.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(15240), getShape(1)->getSize().Width);
}

DECLARE_WW8EXPORT_TEST(testTdf118412, "tdf118412.doc")
{
    /* Check that the first page's bottom margin is 1.251cm (not 2.540cm) */
    OUString sPageStyleName = getProperty<OUString>(getParagraph(1), u"PageStyleName"_ustr);
    uno::Reference<style::XStyle> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(sPageStyleName), uno::UNO_QUERY);
    sal_Int32 nBottomMargin = getProperty<sal_Int32>(xPageStyle, u"BottomMargin"_ustr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1251), nBottomMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testContentControlExport)
{
    // Given a document with a (rich text) content control:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When saving that document to DOC and loading it back:
    saveAndReload(u"MS Word 97"_ustr);

    // Then make sure the dummy character at the end is filtered out:
    OUString aBodyText = getBodyText();
    // Without the accompanying fix in place, this test would have failed:
    // - Expected: test
    // - Actual  : test<space>
    // i.e. the CH_TXTATR_BREAKWORD at the end was written, then the import replaced that with a
    // space.
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, aBodyText);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
