/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>

#include <vcl/svapp.hxx>

#include <bordertest.hxx>

#define convertTwipToMm100(TWIP) ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ww8import/data/", "MS Word 97")
    {
    }
};

#define DECLARE_WW8IMPORT_TEST(TestName, filename) DECLARE_SW_IMPORT_TEST(TestName, filename, Test)

DECLARE_WW8IMPORT_TEST(testFloatingTableSectionMargins, "floating-table-section-margins.doc")
{
    sal_Int32 pageLeft = parseDump("/root/page[2]/infos/bounds", "left").toInt32();
    sal_Int32 pageWidth = parseDump("/root/page[2]/infos/bounds", "width").toInt32();
    sal_Int32 tableLeft = parseDump("/root/page[2]/body/column/body/section/column/body/txt[2]/anchored/fly/tab/infos/bounds", "left").toInt32();
    sal_Int32 tableWidth = parseDump("/root/page[2]/body/column/body/section/column/body/txt[2]/anchored/fly/tab/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT( pageWidth > 0 );
    CPPUNIT_ASSERT( tableWidth > 0 );
    // The table's resulting position should be roughly centered.
    CPPUNIT_ASSERT( abs(( pageLeft + pageWidth / 2 ) - ( tableLeft + tableWidth / 2 )) < 20 );
}

DECLARE_WW8IMPORT_TEST(testN757910, "n757910.doc")
{
    // The internal margin was larger than 0.28cm
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("LeftBorderDistance") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(280), nValue);

    // The border width was zero
    table::BorderLine2 aBorder;
    xPropertySet->getPropertyValue("LeftBorder") >>= aBorder;
    CPPUNIT_ASSERT(aBorder.LineWidth > 0);
}

DECLARE_WW8IMPORT_TEST(testN760294, "n760294.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    table::TableBorder aTableBorder;
    xTable->getPropertyValue("TableBorder") >>= aTableBorder;
    CPPUNIT_ASSERT_EQUAL(aTableBorder.TopLine.InnerLineWidth, aTableBorder.TopLine.OuterLineWidth);
    CPPUNIT_ASSERT_EQUAL(aTableBorder.TopLine.InnerLineWidth, aTableBorder.TopLine.LineDistance);
}

DECLARE_WW8IMPORT_TEST(testN750255, "n750255.doc")
{
/*
Column break without columns on the page is a page break, so check those paragraphs
are on page 2 (page style 'Convert 1') and page 3 (page style 'Convert 2')
enum = ThisComponent.Text.createEnumeration
enum.nextElement
para1 = enum.nextElement
xray para1.String
xray para1.PageStyleName
para2 = enum.nextElement
xray para2.String
xray para2.PageStyleName
*/
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> paraEnumAccess(textDocument->getText(), uno::UNO_QUERY);
    // list of paragraphs
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    // go to 1st paragraph
    (void) paraEnum->nextElement();
    // get the 2nd and 3rd paragraph
    uno::Reference<uno::XInterface> paragraph1(paraEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<uno::XInterface> paragraph2(paraEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> text1(paragraph1, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> text2(paragraph2, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( OUString( "one" ), text1->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "two" ), text2->getString());
    uno::Reference<beans::XPropertySet> paragraphProperties1(paragraph1, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> paragraphProperties2(paragraph2, uno::UNO_QUERY);
    OUString pageStyle1, pageStyle2;
    paragraphProperties1->getPropertyValue( "PageStyleName" ) >>= pageStyle1;
    paragraphProperties2->getPropertyValue( "PageStyleName" ) >>= pageStyle2;
    CPPUNIT_ASSERT_EQUAL( OUString( "Convert 1" ), pageStyle1 );
    CPPUNIT_ASSERT_EQUAL( OUString( "Convert 2" ), pageStyle2 );

}

DECLARE_WW8IMPORT_TEST(testN652364, "n652364.doc")
{
/*
Related to 750255 above, column break with columns on the page however should be a column break.
enum = ThisComponent.Text.createEnumeration
enum.nextElement
para1 = enum.nextElement
xray para1.String
xray para1.PageStyleName
enum.nextElement
para2 = enum.nextElement
xray para2.String
xray para2.PageStyleName
*/
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> paraEnumAccess(textDocument->getText(), uno::UNO_QUERY);
    // list of paragraphs
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    // get the 2nd and 4th paragraph
    (void) paraEnum->nextElement();
    uno::Reference<uno::XInterface> paragraph1(paraEnum->nextElement(), uno::UNO_QUERY);
    (void) paraEnum->nextElement();
    uno::Reference<uno::XInterface> paragraph2(paraEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> text1(paragraph1, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> text2(paragraph2, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( OUString( "text1" ), text1->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "text2" ), text2->getString());
    uno::Reference<beans::XPropertySet> paragraphProperties1(paragraph1, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> paragraphProperties2(paragraph2, uno::UNO_QUERY);
    OUString pageStyle1, pageStyle2;
    paragraphProperties1->getPropertyValue( "PageStyleName" ) >>= pageStyle1;
    paragraphProperties2->getPropertyValue( "PageStyleName" ) >>= pageStyle2;
    // "Standard" is the style for the first page (2nd is "Convert 1").
    CPPUNIT_ASSERT_EQUAL( OUString( "Standard" ), pageStyle1 );
    CPPUNIT_ASSERT_EQUAL( OUString( "Standard" ), pageStyle2 );
}

DECLARE_WW8IMPORT_TEST(testN757118, "n757118.doc")
{
/*
Two pairs of horizontal rules (one absolute width, one relative width)
have the same width (full page width, half page width).
xray ThisComponent.DrawPage.getByIndex(0).BoundRect
*/
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> rule1 = getShape(1), rule2 = getShape(2), rule3 = getShape(3), rule4 = getShape(4);
    uno::Reference<beans::XPropertySet> ruleProperties1(rule1, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> ruleProperties2(rule2, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> ruleProperties3(rule3, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> ruleProperties4(rule4, uno::UNO_QUERY);
    awt::Rectangle boundRect1, boundRect2, boundRect3, boundRect4;
    ruleProperties1->getPropertyValue( "BoundRect" ) >>= boundRect1;
    ruleProperties2->getPropertyValue( "BoundRect" ) >>= boundRect2;
    ruleProperties3->getPropertyValue( "BoundRect" ) >>= boundRect3;
    ruleProperties4->getPropertyValue( "BoundRect" ) >>= boundRect4;
    // compare, allow for < 5 differences because of rounding errors
    CPPUNIT_ASSERT( abs( boundRect1.Width - boundRect3.Width ) < 5 );
    CPPUNIT_ASSERT( abs( boundRect2.Width - boundRect4.Width ) < 5 );
}

DECLARE_WW8IMPORT_TEST(testN757905, "n757905.doc")
{
    // The problem was that the paragraph had only a single fly
    // (as-character-anchored), and the height of that was smaller than the
    // paragraph height. When in Word-compat mode, we should take the max of
    // the two, not just the height of the fly.

    OUString aHeight = parseDump("/root/page/body/txt/infos/bounds", "height");
    CPPUNIT_ASSERT(sal_Int32(31) < aHeight.toInt32());
}

DECLARE_WW8IMPORT_TEST(testAllGapsWord, "all_gaps_word.doc")
{
    BorderTest borderTest;
    BorderTest::testTheBorders(mxComponent, true);
}

DECLARE_WW8IMPORT_TEST(testI120158, "i120158.doc")
{
    // See https://bz.apache.org/ooo/show_bug.cgi?id=120158
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> paraEnumAccess(textDocument->getText(), uno::UNO_QUERY);
    // list of paragraphs
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    // get contents of 1st paragraph as text
    uno::Reference<uno::XInterface> paragraph0(paraEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> text0(paragraph0, uno::UNO_QUERY);
    OUString sFieldResult = text0->getString();
    CPPUNIT_ASSERT(sFieldResult.endsWith("AM") || sFieldResult.endsWith("PM"));
}

DECLARE_WW8IMPORT_TEST(testN816603, "n816603.doc")
{
    // Bugdoc was 5 pages in Word, 1 in Writer due to pointlessly wrapping the
    // table in a frame. Exact layout may depend on fonts available, etc. --
    // but at least make sure that our table spans over multiple pages now.
    CPPUNIT_ASSERT(getPages() > 1);
}

DECLARE_WW8IMPORT_TEST(testN816593, "n816593.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // Make sure that even if we import the two tables as non-floating, we
    // still consider them different, and not merge them.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

DECLARE_WW8IMPORT_TEST(testPageBorder, "page-border.doc")
{
    // Page border was missing (LineWidth was 0), due to wrong interpretation of pgbApplyTo.
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(getStyles("PageStyles")->getByName("Standard"), "TopBorder");
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(convertTwipToMm100(6 * 20)), aBorder.LineWidth);
}

DECLARE_WW8IMPORT_TEST(testN823651, "n823651.doc")
{
    // Character height was 10pt instead of 7.5pt in the header.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = getProperty< uno::Reference<text::XTextRange> >(xStyle, "HeaderTextFirst")->getText();
    CPPUNIT_ASSERT_EQUAL(7.5f, getProperty<float>(getParagraphOfText(1, xText), "CharHeight"));
}

DECLARE_WW8IMPORT_TEST(testFdo36868, "fdo36868.doc")
{
    OUString aText = parseDump("/root/page/body/txt[3]/Special[@nType='POR_NUMBER']", "rText");
    // This was 1.1.
    CPPUNIT_ASSERT_EQUAL(OUString("2.1"), aText);
}

DECLARE_WW8IMPORT_TEST(testListNolevel, "list-nolevel.doc")
{
    // Similar to fdo#36868, numbering portions had wrong values.
    OUString aText = parseDump("/root/page/body/txt[1]/Special[@nType='POR_NUMBER']", "rText");
    // POR_NUMBER was completely missing.
    CPPUNIT_ASSERT_EQUAL(OUString("1."), aText);
}

DECLARE_WW8IMPORT_TEST(testBnc821208, "bnc821208.doc")
{
    // WW8Num1z0 earned a Symbol font, turning numbers into rectangles.
    uno::Reference<beans::XPropertyState> xPropertyState(getStyles("CharacterStyles")->getByName("WW8Num1z0"), uno::UNO_QUERY);
    beans::PropertyState ePropertyState = xPropertyState->getPropertyState("CharFontName");
    // This was beans::PropertyState_DIRECT_VALUE.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE, ePropertyState);

    // Background of the numbering itself should have been the default, was yellow (0xffff00).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xPropertyState, "CharBackColor"));
}

DECLARE_WW8IMPORT_TEST(testCp1000044, "cp1000044.doc")
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    // It wasn't possible to fill out this form.
    CPPUNIT_ASSERT_EQUAL(false, bool(xStorable->isReadonly()));
}

DECLARE_WW8IMPORT_TEST(testCp1000039, "cp1000039.doc")
{
    // This was RTL_TEXTENCODING_SYMBOL, causing "1" rendered as a placeholder rectangle.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(RTL_TEXTENCODING_DONTKNOW), getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharFontCharSet"));
}

DECLARE_WW8IMPORT_TEST(testBorderColours, "bordercolours.doc")
{
    // The following 6 colours can only be represented with WW9 (Word 2000)
    // BRC (BoRder Control) structures.  We can tell that they have been
    // exported/imported using a WW8 (Word '97) BRC if they instead come
    // through as one of the 16 colours listed at this link:
    // http://msdn.microsoft.com/en-us/library/dd773060.aspx
    table::BorderLine2 expectedTop(0xFA670C, 0, 53, 0, 1, 53);
    table::BorderLine2 expectedLeft(0xD99594, 0, 79, 0, 0, 79);
    table::BorderLine2 expectedRight(0xB2A1C7, 53, 53, 53, 3, 159);
    table::BorderLine2 expectedBottom(0xB6DDE8, 0, 106, 0, 14, 106);
    table::BorderLine2 expectedDashedRed(0xFA670C, 0, 53, 0, 2, 53);
    table::BorderLine2 expectedDoubleGreen(0xC2D69B, 26, 106, 26, 4, 159);

    // Paragraph border
    uno::Reference<text::XBookmarksSupplier> bookmarksSupplier(mxComponent,
        uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> bookmarks(
        bookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<text::XTextContent> bookmark(
        bookmarks->getByName("ParagraphBorder"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> anchor(bookmark->getAnchor());
    table::BorderLine2 border;
    border = getProperty<table::BorderLine2>(anchor, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(anchor, "RightBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(anchor, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);

    // Page border
    OUString pageStyleName = getProperty<OUString>(anchor, "PageStyleName");
    uno::Reference<style::XStyle> pageStyle(
        getStyles("PageStyles")->getByName(pageStyleName), uno::UNO_QUERY);
    border = getProperty<table::BorderLine2>(pageStyle, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(pageStyle, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(pageStyle, "RightBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(pageStyle, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);

    // Character border
    bookmark.set(bookmarks->getByName("CharBorder"), uno::UNO_QUERY);
    anchor = bookmark->getAnchor();
    border = getProperty<table::BorderLine2>(anchor, "CharTopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, "CharLeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, "CharRightBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, "CharBottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);

    // Table border
    uno::Reference<text::XTextTablesSupplier> tablesSupplier(mxComponent,
        uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> tables(
        tablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> table(
        tables->getByName("Table1"), uno::UNO_QUERY);
    table::TableBorder2 tableBorder = getProperty<table::TableBorder2>(
        table, "TableBorder2");
    CPPUNIT_ASSERT_EQUAL(expectedTop.Color, tableBorder.TopLine.Color);
    CPPUNIT_ASSERT_EQUAL(expectedLeft.Color, tableBorder.LeftLine.Color);
    CPPUNIT_ASSERT_EQUAL(expectedRight.Color, tableBorder.RightLine.Color);
    CPPUNIT_ASSERT_EQUAL(expectedBottom.Color, tableBorder.BottomLine.Color);

    // Table cells
    uno::Reference<table::XCell> cell(
        table->getCellByName("A2"), uno::UNO_QUERY);
    border = getProperty<table::BorderLine2>(cell, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(cell, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(cell, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);

    cell.set(table->getCellByName("B2"), uno::UNO_QUERY);
    border = getProperty<table::BorderLine2>(cell, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);
    border = getProperty<table::BorderLine2>(cell, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(cell, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);

    cell.set(table->getCellByName("C2"), uno::UNO_QUERY);
    border = getProperty<table::BorderLine2>(cell, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);
    border = getProperty<table::BorderLine2>(cell, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDashedRed, border);
    border = getProperty<table::BorderLine2>(cell, "RightBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(cell, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);

    // Picture border
    // (#if'd out as they are not yet imported with correct colours)
#if 0
    bookmark.set(bookmarks->getByName("PictureBorder"),uno::UNO_QUERY);
    anchor = bookmark->getAnchor();
    border = getProperty<table::BorderLine2>(anchor, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(anchor, "RightBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(anchor, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);
#endif
}

DECLARE_WW8IMPORT_TEST(testMsoBrightnessContrast, "msobrightnesscontrast.doc")
{
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> image(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> graphic;
    imageProperties->getPropertyValue( "Graphic" ) >>= graphic;
    uno::Reference<awt::XBitmap> bitmap(graphic, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(58), bitmap->getSize().Width );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(320), bitmap->getSize().Height );
    const uno::Sequence< sal_Int8 > data = bitmap->getDIB(); // as .bmp data
    CPPUNIT_ASSERT_EQUAL( sal_Int32(20278), data.getLength());
    CPPUNIT_ASSERT_EQUAL( -50, int(data[0x6b0])); // -50 = 206 pixel value
    CPPUNIT_ASSERT_EQUAL( -50, int(data[0x6b1]));
    CPPUNIT_ASSERT_EQUAL( -50, int(data[0x6b2]));
    CPPUNIT_ASSERT_EQUAL( -50, int(data[0x6b3]));
    CPPUNIT_ASSERT_EQUAL( -50, int(data[0x6b4]));
}

DECLARE_WW8IMPORT_TEST(testBnc863018, "bnc863018.doc")
{
    // The problem was that there should be some whitespace above the table,
    // but there wasn't.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    // This was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5002), getProperty<sal_Int32>(xTable, "TopMargin"));
}

DECLARE_WW8IMPORT_TEST(testTdf95321, "tdf95321.doc")
{
    // The problem was that there should be content in the second cell
    // but there wasn't.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Second Column"), uno::Reference<text::XTextRange>(xTable->getCellByName("B1"), uno::UNO_QUERY)->getString());
}

DECLARE_WW8IMPORT_TEST(testBnc875715, "bnc875715.doc")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // Was incorrectly set as -1270.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xSections->getByIndex(0), "SectionLeftMargin"));
}

DECLARE_WW8IMPORT_TEST(testFdo77844, "fdo77844.doc")
{
    uno::Reference<container::XNameAccess> pageStyles = getStyles("PageStyles");

    // get a page cursor
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    // check that the first page has no header
    xCursor->jumpToFirstPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    uno::Reference<style::XStyle> pageStyle(
        pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    bool headerIsOn = getProperty<bool>(pageStyle, "HeaderIsOn");
    CPPUNIT_ASSERT(!headerIsOn);

    // check that the second page has a header
    xCursor->jumpToPage(2);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    pageStyle.set(
        pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    headerIsOn = getProperty<bool>(pageStyle, "HeaderIsOn");
    CPPUNIT_ASSERT(headerIsOn);

    // check that the third page has a header
    xCursor->jumpToPage(3);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    pageStyle.set(
        pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    headerIsOn = getProperty<bool>(pageStyle, "HeaderIsOn");
    CPPUNIT_ASSERT(headerIsOn);

    // check that the fourth page has no header
    // (#if'd out as this is not yet imported correctly)
#if 0
    xCursor->jumpToPage(4);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    pageStyle.set(
        pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    headerIsOn = getProperty<bool>(pageStyle, "HeaderIsOn");
    CPPUNIT_ASSERT(!headerIsOn);
#endif
}

DECLARE_WW8IMPORT_TEST(testFdp80333, "fdo80333.doc")
{
    // Despite there is no character border, border shadow was imported
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);
}

DECLARE_WW8IMPORT_TEST(testFdo81102, "fdo81102.doc")
{
    // get page style at beginning of document
    uno::Reference<text::XTextDocument> textDocument(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> start(
        textDocument->getText()->getStart(), uno::UNO_QUERY);
    OUString pageStyleName = getProperty<OUString>(start, "PageStyleName");
    uno::Reference<style::XStyle> pageStyle(
        getStyles("PageStyles")->getByName(pageStyleName), uno::UNO_QUERY);

    // check that left and right pages do not share the same header
    bool headerIsShared = getProperty<bool>(pageStyle, "HeaderIsShared");
    CPPUNIT_ASSERT(!headerIsShared);
}

DECLARE_WW8IMPORT_TEST(testFloatingTableSectionColumns, "floating-table-section-columns.doc")
{
    OUString tableWidth = parseDump("/root/page[1]/body/section/column[2]/body/txt/anchored/fly/tab/infos/bounds", "width");
    // table width was restricted by a column
    CPPUNIT_ASSERT( tableWidth.toInt32() > 10000 );
}

DECLARE_WW8IMPORT_TEST(testBnc787942, "bnc787942.doc")
{
    // The frame ended up on the second page instead of first.
    parseDump("/root/page[1]/body/txt[4]/anchored");
}

DECLARE_WW8IMPORT_TEST(testLayoutHanging, "fdo68967.doc")
{
    // This must not hang in layout
}

DECLARE_WW8IMPORT_TEST(testfdo68963, "fdo68963.doc")
{
    // The problem was that the text was not displayed.
    CPPUNIT_ASSERT ( !parseDump("/root/page/body/tab/row[2]/cell[1]/txt/Special", "rText").isEmpty() );
    CPPUNIT_ASSERT_EQUAL( OUString("Topic 1"), parseDump("/root/page/body/tab/row[2]/cell[1]/txt/Special", "rText") );
    // all crossreference bookmarks should have a target.  Shouldn't be any "Reference source not found" in the xml
    CPPUNIT_ASSERT ( -1 == parseDump("/root/page/body/txt[24]/Special[2]","rText").indexOf("Reference source not found"));
}

DECLARE_WW8IMPORT_TEST(testTdf99100, "tdf99100.doc")
{
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    auto xField = getProperty< uno::Reference<lang::XServiceInfo> >(getRun(getParagraphOfText(1, xHeaderText), 2), "TextField");
    // This failed: the second text portion wasn't a field.
    CPPUNIT_ASSERT(xField.is());
    CPPUNIT_ASSERT(xField->supportsService("com.sun.star.text.textfield.Chapter"));
}

DECLARE_WW8IMPORT_TEST(testTdf99120, "tdf99120.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Section 1, odd."),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Section 1, even."),  parseDump("/root/page[2]/header/txt/text()"));
    // This failed: the header was empty on the 3rd page, as the first page header was shown.
    CPPUNIT_ASSERT_EQUAL(OUString("Section 2, odd."),  parseDump("/root/page[3]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Section 2, even."),  parseDump("/root/page[4]/header/txt/text()"));
}

DECLARE_WW8IMPORT_TEST(testTdf74328, "tdf74328.doc")
{
/*
reading page numbers at sections > 255, in this case 256
*/
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xTextCursor(textDocument->getText()->createTextCursor( ), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xProps(xTextCursor, uno::UNO_QUERY);
    uno::Any aOffset = xProps->getPropertyValue("PageNumberOffset");
    sal_Int16 nOffset = 0;
    aOffset >>= nOffset;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(256), nOffset);
}

DECLARE_WW8IMPORT_TEST(testTdf95576, "tdf95576.doc")
{
    // The first three paragraphs in this document (which are headings)
    // should have zero indent and first line indent
    for (int nPara = 1; nPara <= 3; ++nPara) {
        std::cout << "nPara = " << nPara << "\n";
        auto xPara = getParagraph(nPara);

        // get the numbering rules effective at this paragraph
        uno::Reference<container::XIndexReplace> xNumRules(
            getProperty< uno::Reference<container::XIndexReplace> >(
                xPara, "NumberingRules"),
            uno::UNO_QUERY);

        // get the numbering level of this paragraph, and the properties
        // associated with that numbering level
        int numLevel = getProperty<sal_Int32>(xPara, "NumberingLevel");
        uno::Sequence< beans::PropertyValue > aPropertyValues;
        xNumRules->getByIndex(numLevel) >>= aPropertyValues;

        // Now look through these properties for the indent and
        // first line indent settings
        sal_Int32 nIndentAt = -1;
        sal_Int32 nFirstLineIndent = -1;
        for(int j = 0 ; j< aPropertyValues.getLength() ; ++j)
        {
            auto aProp = aPropertyValues[j];
            std::cout << "Prop.Name: " << aProp.Name << "\n";
            if (aProp.Name == "FirstLineIndent") {
                nFirstLineIndent = aProp.Value.get<sal_Int32>();
            } else if (aProp.Name == "IndentAt") {
                nIndentAt = aProp.Value.get<sal_Int32>();
            }
        }

        // The indent and first line indent should be zero
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nIndentAt);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nFirstLineIndent);
    }
}

DECLARE_WW8IMPORT_TEST(testTdf59896, "tdf59896.doc")
{
    // This was awt::FontWeight::NORMAL, i.e. the first run wasn't bold, when it should be bold
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(getParagraph(1), 1), "CharWeight"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
