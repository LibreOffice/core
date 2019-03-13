/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

#include <ftninfo.hxx>
#include <pagedesc.hxx>
#include <svx/svdpage.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <editeng/unoprnms.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97") {}

    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".doc");
    }

};
DECLARE_WW8EXPORT_TEST(testTdf99120, "tdf99120.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Section 1, odd."),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Section 1, even."),  parseDump("/root/page[2]/header/txt/text()"));
    // This failed: the header was empty on the 3rd page, as the first page header was shown.
    CPPUNIT_ASSERT_EQUAL(OUString("Section 2, odd."),  parseDump("/root/page[3]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Section 2, even."),  parseDump("/root/page[4]/header/txt/text()"));
}

DECLARE_WW8EXPORT_TEST(testTdf41542_borderlessPadding, "tdf41542_borderlessPadding.odt")
{
    // the page style's borderless padding should force this to 3 pages, not 1
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );
}

DECLARE_WW8EXPORT_TEST(testTdf55528_relativeTableWidth, "tdf55528_relativeTableWidth.doc")
{

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table relative width percent", sal_Int16(98), getProperty<sal_Int16>(xTable, "RelativeWidth"));
 }

DECLARE_WW8EXPORT_TEST(testTdf116436_tableBackground, "tdf116436_tableBackground.odt")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xF8DF7C), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell.set(xTable->getCellByName("A6"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x81D41A), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell.set(xTable->getCellByName("B6"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFBCC), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_WW8EXPORT_TEST(testTdf37153, "tdf37153_considerWrapOnObjPos.doc")
{
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH, getProperty<text::WrapTextMode>(getShape(1), "Surround"));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::BOTTOM, getProperty<sal_Int16>(xTable->getCellByName("A1"), "VertOrient"));

    //For MSO compatibility, the image should be at the top of the cell, not at the bottom - despite VertOrientation::BOTTOM
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFlyTop  = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt/anchored/fly/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_MESSAGE("FlyTop should be 3820, not 6623", nFlyTop < 4000);
    sal_Int32 nTextTop  = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt[1]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_MESSAGE("TextTop should be 5388", nTextTop > 4000);
}

DECLARE_WW8EXPORT_TEST(testTdf49102_mergedCellNumbering, "tdf49102_mergedCellNumbering.doc")
{
    CPPUNIT_ASSERT_EQUAL( OUString("2."), parseDump("/root/page/body/tab/row[4]/cell/txt/Special[@nType='PortionType::Number']", "rText") );
}

DECLARE_WW8EXPORT_TEST(testTdf55427_footnote2endnote, "tdf55427_footnote2endnote.odt")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("ParagraphStyles")->getByName("Footnote"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Footnote style is rose color", sal_Int32(0xFF007F), getProperty< sal_Int32 >(xPageStyle, "CharColor") );
    xPageStyle.set(getStyles("ParagraphStyles")->getByName("Endnote"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Endnote style is cyan3 color", sal_Int32(0x2BD0D2), getProperty< sal_Int32 >(xPageStyle, "CharColor") );

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    // The footnote numbering type of ARABIC will not transfer over when those footnotes are converted to endnotes.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Footnote numbering type", SVX_NUM_ARABIC, pDoc->GetFootnoteInfo().aFormat.GetNumberingType() );
    // The original document has a real endnote using ROMAN_LOWER numbering, so that setting MUST remain unchanged.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Endnote numbering type", SVX_NUM_ROMAN_LOWER, pDoc->GetEndNoteInfo().aFormat.GetNumberingType() );

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);

    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes(xEndnotesSupplier->getEndnotes(), uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xEndnote;
    xEndnotes->getByIndex(0) >>= xEndnote;
    uno::Reference<text::XText> xEndnoteText;
    xEndnotes->getByIndex(0) >>= xEndnoteText;

    // ODT footnote-at-document-end's closest DOC match is an endnote, so the two imports will not exactly match by design.
    if (!mbExported)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote count", sal_Int32(5), xFootnotes->getCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote count", sal_Int32(1), xEndnotes->getCount() );

        uno::Reference<text::XFootnote> xFootnote;
        xFootnotes->getByIndex(0) >>= xFootnote;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote's number", OUString("1"), xFootnote->getAnchor()->getString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote's number", OUString("i"), xEndnote->getAnchor()->getString() );

        uno::Reference<text::XText> xFootnoteText;
        xFootnotes->getByIndex(0) >>= xFootnoteText;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote style", OUString("Footnote"), getProperty<OUString>(getParagraphOfText(1, xFootnoteText), "ParaStyleName") );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote style", OUString("Endnote"), getProperty<OUString>(getParagraphOfText(1, xEndnoteText), "ParaStyleName") );
    }
    else
    {
        // These asserted items are major differences in the conversion from footnote to endnote, NOT necessary conditions for a proper functioning document.
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "At-Document-End footnotes were converted into endnotes", sal_Int32(0), xFootnotes->getCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "At-Document-End footnotes became endnotes", sal_Int32(6), xEndnotes->getCount() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "converted footnote's number", OUString("i"), xEndnote->getAnchor()->getString() );
        xEndnotes->getByIndex(4) >>= xEndnote;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote's new number", OUString("v"), xEndnote->getAnchor()->getString() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "retained footnote style", OUString("Footnote"), getProperty<OUString>(getParagraphOfText(1, xEndnoteText), "ParaStyleName") );
        xEndnotes->getByIndex(4) >>= xEndnoteText;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote style", OUString("Endnote"), getProperty<OUString>(getParagraphOfText(1, xEndnoteText), "ParaStyleName") );
    }
}

DECLARE_WW8EXPORT_TEST(testTdf107931_KERN_DocEnabled_disabledDefStyle, "testTdf107931_KERN_DocEnabled_disabledDefStyle.doc")
{
    // Paragraph 3: the default style has kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(3), 1), "CharAutoKerning"));
    // Paragraph 4: style with kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(4), 1), "CharAutoKerning"));
    // Paragraph 5: style with kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(5), 1), "CharAutoKerning"));
    // Paragraph 6: directly applied character properties: kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(6), 1), "CharAutoKerning"));
    // Paragraph 7: directly applied character properties: kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(7), 1), "CharAutoKerning"));
}

DECLARE_WW8EXPORT_TEST(testTdf107931_KERN_enabledDefStyle, "testTdf107931_KERN_enabledDefStyle.doc")
{
    // Paragraph 3: the default style has kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(3), 1), "CharAutoKerning"));
    // Paragraph 4: style with kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(4), 1), "CharAutoKerning"));
    // Paragraph 5: style with kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(5), 1), "CharAutoKerning"));
    // Paragraph 6: directly applied character properties: kerning disabled
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(6), 1), "CharAutoKerning"));
    // Paragraph 7: directly applied character properties: kerning enabled
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(7), 1), "CharAutoKerning"));
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

DECLARE_WW8EXPORT_TEST(testTdf108448_endNote, "tdf108448_endNote.odt")
{
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes(xEndnotesSupplier->getEndnotes(), uno::UNO_QUERY);
    uno::Reference<text::XText> xEndnote;
    xEndnotes->getByIndex(0) >>= xEndnote;

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of paragraphs in Endnote i", 1, getParagraphs(xEndnote) );
}

DECLARE_WW8EXPORT_TEST(testTdf106062_nonHangingFootnote, "tdf106062_nonHangingFootnote.odt")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This failed, tab between the footnote number and the footnote content was lost on import.
    CPPUNIT_ASSERT_MESSAGE( "Footnote starts with a tab", xTextRange->getString().startsWith("\t") );
}

DECLARE_WW8EXPORT_TEST(testTdf116570_exportFootnote, "tdf116570_exportFootnote.odt")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of paragraphs in first footnote", 2, getParagraphs(xFootnoteText) );
}

DECLARE_WW8EXPORT_TEST(testTdf112074_RTLtableJustification, "tdf112074_RTLtableJustification.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Right To Left writing mode", text::WritingMode2::RL_TB, getProperty<sal_Int16>(xTable, "WritingMode"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Horizontal Orientation", text::HoriOrientation::LEFT_AND_WIDTH, getProperty<sal_Int16>(xTable, "HoriOrient"));
    CPPUNIT_ASSERT_MESSAGE("Table Indent", getProperty<long>(xTable, "LeftMargin") > 3000);
    CPPUNIT_ASSERT_MESSAGE("Table Indent is 3750", getProperty<long>(xTable, "LeftMargin") < 4000 );
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_RIGHT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraphOrTable(2), "ParaAdjust")) );
}

DECLARE_WW8EXPORT_TEST(testTdf98620_rtlJustify, "tdf98620_rtlJustify.doc")
{
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_RIGHT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")) );
}

DECLARE_WW8EXPORT_TEST(testTdf121110_absJustify, "tdf121110_absJustify.doc")
{
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_RIGHT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")) );
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_LEFT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(3), "ParaAdjust")) );
}

DECLARE_WW8EXPORT_TEST(testTdf106174_rtlParaAlign, "tdf106174_rtlParaAlign.docx")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER), getProperty<sal_Int16>(getParagraph(1), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER), getProperty<sal_Int16>(getParagraph(2), "ParaAdjust"));
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("ParagraphStyles")->getByName("Another paragraph aligned to right"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(xPropertySet, "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(3), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(4), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(5), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(6), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(7), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(8), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(9), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(10), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(11), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(12), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT),  getProperty<sal_Int16>(getParagraph(13), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getParagraph(14), "ParaAdjust"));
}

DECLARE_WW8EXPORT_TEST(testTdf119232_startEvenPage, "tdf119232_startEvenPage.doc")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), getProperty<sal_Int16>(getParagraph(1), "PageNumberOffset"));
}

DECLARE_WW8EXPORT_TEST(testTdf104805, "tdf104805.doc")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WW8Num1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aNumberingRule;
    xLevels->getByIndex(1) >>= aNumberingRule; // 2nd level
    for (const auto& rPair : aNumberingRule)
    {
        if (rPair.Name == "Prefix")
            // This was "." instead of empty, so the second paragraph was
            // rendered as ".1" instead of "1.".
            CPPUNIT_ASSERT_EQUAL(OUString(), rPair.Value.get<OUString>());
        else if (rPair.Name == "Suffix")
            CPPUNIT_ASSERT_EQUAL(OUString("."), rPair.Value.get<OUString>());
    }
}

DECLARE_WW8EXPORT_TEST(testTdf104334, "tdf104334.doc")
{
    // This failed with a container::NoSuchElementException: STYLEREF was
    // mapped to SwChapterField, and the field result was "This is a Heading 1"
    // instead of just "1".
    CPPUNIT_ASSERT_EQUAL(OUString("1"), getRun(getParagraph(2), 3)->getString());
}

DECLARE_WW8EXPORT_TEST(testTdf108072, "tdf108072.doc")
{
    // The property IsSplitAllowed was imported from an obsolete property, sprmTFCantSplit90
    // instead of sprmTFCantSplit. sprmTFCantSplit90 is set to true for merged rows, so
    // for merged rows incorrect settings were imported, which prevented them from breaking over pages.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTableRows->getByIndex(0), "IsSplitAllowed"));
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

DECLARE_WW8EXPORT_TEST(testTdf70838, "tdf70838.odt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    tools::Rectangle aRect = pPage->GetObj(0)->GetSnapRect();
    CPPUNIT_ASSERT(aRect.GetHeight() > aRect.GetWidth());
}

DECLARE_WW8EXPORT_TEST(testTdf70838b_verticalRotation, "tdf70838b_verticalRotation.odt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    tools::Rectangle aGroupShape = pPage->GetObj(0)->GetSnapRect();
    tools::Rectangle aLine = pPage->GetObj(2)->GetSnapRect();

    CPPUNIT_ASSERT_MESSAGE("Smiley faces are round", aGroupShape.GetHeight() > aGroupShape.GetWidth());
    CPPUNIT_ASSERT_MESSAGE("Line is taller, not wider", aLine.GetHeight() > aLine.GetWidth());
}

DECLARE_WW8EXPORT_TEST( testActiveXCheckbox, "checkbox_control.odt" )
{
    // First check box anchored as a floating object
    uno::Reference<drawing::XControlShape> xControlShape;
    if(!mbExported)
        xControlShape.set(getShape(1), uno::UNO_QUERY);
    else
        xControlShape.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check whether we have the right control
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService( "com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Floating Checkbox"), getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,"AnchorType"));

    // Second check box anchored inline / as character
    if(!mbExported)
        xControlShape.set(getShape(2), uno::UNO_QUERY);
    else
        xControlShape.set(getShape(1), uno::UNO_QUERY);

    // Check whether we have the right control
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Inline Checkbox"), getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    xPropertySet2.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,"AnchorType"));
}

DECLARE_WW8EXPORT_TEST(testTdf67207_MERGEFIELD, "mailmerge.doc")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), "TextField");
    CPPUNIT_ASSERT(xTextField.is());
    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY_THROW);
    uno::Reference<text::XDependentTextField> xDependent(xTextField, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.Database"));
    OUString sValue;
    xTextField->getPropertyValue("Content") >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8(u8"«Name»"), sValue);

    uno::Reference<beans::XPropertySet> xFiledMaster = xDependent->getTextFieldMaster();
    uno::Reference<lang::XServiceInfo> xFiledMasterServiceInfo(xFiledMaster, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xFiledMasterServiceInfo->supportsService("com.sun.star.text.fieldmaster.Database"));

    // Defined properties: DataBaseName, Name, DataTableName, DataColumnName, DependentTextFields, DataCommandType, InstanceName, DataBaseURL
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("Name") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("DataColumnName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("InstanceName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.text.fieldmaster.DataBase.Name"), sValue);
}

DECLARE_OOXMLEXPORT_TEST( testTableCrossReference, "table_cross_reference.odt" )
{
    // tdf#42346: Cross references to tables were not saved
    // MSO uses simple bookmarks for referencing table caption, so we do the same by export
    if (!mbExported)
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_number_only"));

    // Check bookmark text ranges
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table 1: Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
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
                CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Table 1: Table caption"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to table number
            case 1:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_number_only"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to caption only
            case 2:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_caption_only"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to category and number
            case 3:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Table 1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_label_and_number"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to page of the table
            case 4:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Page style reference / exported as simple page reference
            case 5:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Above / below reference
            case 6:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("above"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
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

DECLARE_OOXMLEXPORT_TEST( testTableCrossReferenceCustomFormat, "table_cross_reference_custom_format.odt" )
{
    // tdf#42346: Cross references to tables were not saved
    // Check also captions with custom formatting
    if (!mbExported)
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(16), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_number_only"));

    // Check bookmark text ranges
    // First table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1. Table: Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1. Table"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    // Second table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table1_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2. TableTable caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table1_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2. Table"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table1_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table1_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("2"), xRange->getString());
    }
    // Third table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table2_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("3) Table Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table2_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("3) Table"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table2_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table2_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("3"), xRange->getString());
    }
    // Fourth table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table3_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table 4- Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table3_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table 4"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table3_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table3_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("4"), xRange->getString());
    }
}

DECLARE_OOXMLEXPORT_TEST( testObjectCrossReference, "object_cross_reference.odt" )
{
    // tdf#42346: Cross references to objects were not saved
    // MSO uses simple bookmarks for referencing table caption, so we do the same by export
    if (!mbExported)
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(15), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing1_full"));

    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration1_caption_only"));

    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text1_label_and_number"));

    // Check bookmark text ranges
    // Cross references to shapes
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Drawing0_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Drawing 1: A rectangle"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Drawing0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Drawing 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Drawing0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("A rectangle"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Drawing0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Drawing1_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Drawing 2: a circle"), xRange->getString());
    }

    // Cross references to pictures
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Illustration0_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Illustration 1: A picture"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Illustration0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Illustration 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Illustration0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("A picture"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Illustration0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Illustration1_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("an other image"), xRange->getString());
    }

    // Cross references to text frames
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Text0_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Text 1: A frame"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Text0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Text 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Text0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("A frame"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Text0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Text1_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange(xContent->getAnchor(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("Text 2"), xRange->getString());
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
    auto xStyles = getStyles("PageStyles");

    for (const auto& style : styleParams)
    {
        const OUString sName = OUString::createFromAscii(style.styleName);
        uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName(sName), uno::UNO_QUERY_THROW);
        for (const auto& side : style.sideParams)
        {
            const OUString sSide = OUString::createFromAscii(side.sideName);
            const OString sStage = OString(style.styleName) + " " + side.sideName;

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

DECLARE_WW8EXPORT_TEST(testTdf117503, "tdf117503.docx")
{
    // This was 3, first page + standard page styles were not merged together
    // on export.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_WW8EXPORT_TEST(testTdf117885, "tdf117885.doc")
{
    xmlDocPtr pXmlDoc = parseLayoutDump();

    /* Get the vertical position of the paragraph containing the text "Start" */
    sal_Int32 nParaA_Top = getXPath(pXmlDoc,
        "/root/page/body/column[1]/body/txt[text()='Start']/infos/bounds", "top"
        ).toInt32();

    /* Get the vertical position of the paragraph containing the text "Top B" */
    sal_Int32 nParaB_Top = getXPath(pXmlDoc,
        "/root/page/body/column[2]/body/txt[text()='Top B']/infos/bounds", "top"
        ).toInt32();

    /* These two paragraphs are supposed to be at the top of the left
     * and right columns respectively.  Check that they actually line up: */
    CPPUNIT_ASSERT_EQUAL(nParaA_Top, nParaB_Top);
}

DECLARE_WW8EXPORT_TEST(testTdf118133, "tdf118133.docx")
{
    // This was 0, doc import + doc export resulted in lost image due to broken
    // lazy-loading of tiff images.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(15240), getShape(1)->getSize().Width);
}

DECLARE_WW8EXPORT_TEST(testTdf118412, "tdf118412.doc")
{
    /* Check that the first page's bottom margin is 1.251cm (not 2.540cm) */
    OUString sPageStyleName = getProperty<OUString>(getParagraph(1), "PageStyleName");
    uno::Reference<style::XStyle> xPageStyle(
        getStyles("PageStyles")->getByName(sPageStyleName), uno::UNO_QUERY);
    sal_Int32 nBottomMargin = getProperty<sal_Int32>(xPageStyle, "BottomMargin");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1251), nBottomMargin);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
