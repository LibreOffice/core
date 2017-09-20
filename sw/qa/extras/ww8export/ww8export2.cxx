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
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>

#include <ftninfo.hxx>
#include <pagedesc.hxx>

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
    CPPUNIT_ASSERT_EQUAL( OUString("2."), parseDump("/root/page/body/tab/row[4]/cell/txt/Special[@nType='POR_NUMBER']", "rText") );
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

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xEndnote->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    int nRet = 0;
    while (xParaEnum->hasMoreElements())
    {
        xParaEnum->nextElement();
        nRet++;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of paragraphs in Endnote i", 1, nRet );
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
    uno::Reference<beans::XPropertySet> xWatermarkProperties(xWatermark, uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL((sal_Int32)5172, xWatermark->getSize().Height);
    CPPUNIT_ASSERT_EQUAL((sal_Int32)18105, xWatermark->getSize().Width);
}

DECLARE_WW8EXPORT_TEST(testTdf111480, "tdf111480.doc")
{
    // Circular text was imported horizontally
    uno::Reference<drawing::XShape> xText = getShape(1);

    CPPUNIT_ASSERT(xText->getSize().Height > 11000);
    CPPUNIT_ASSERT(xText->getSize().Width  > 11000);
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
