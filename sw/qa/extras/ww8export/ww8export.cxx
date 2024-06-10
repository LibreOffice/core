/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/form/validation/XValidatableFormComponent.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>

#include <config_fonts.h>
#include <editeng/ulspitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/UnitConversion.hxx>

#include <cmdid.h>
#include <envimg.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <fmtsrnd.hxx>
#include <frameformats.hxx>
#include <grfatr.hxx>
#include <pagedesc.hxx>
#include <ndgrf.hxx>
#include <bordertest.hxx>
#include <IDocumentSettingAccess.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <o3tl/string_view.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ww8export/data/"_ustr, u"MS Word 97"_ustr) {}
};

DECLARE_WW8EXPORT_TEST(testN757910, "n757910.doc")
{
    // The internal margin was larger than 0.28cm
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue(u"LeftBorderDistance"_ustr) >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(280), nValue);

    // The border width was zero
    table::BorderLine2 aBorder;
    xPropertySet->getPropertyValue(u"LeftBorder"_ustr) >>= aBorder;
    CPPUNIT_ASSERT(aBorder.LineWidth > 0);
}

DECLARE_WW8EXPORT_TEST(testN760294, "n760294.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    table::TableBorder aTableBorder;
    xTable->getPropertyValue(u"TableBorder"_ustr) >>= aTableBorder;
    CPPUNIT_ASSERT_EQUAL(aTableBorder.TopLine.InnerLineWidth, aTableBorder.TopLine.OuterLineWidth);
    CPPUNIT_ASSERT_EQUAL(aTableBorder.TopLine.InnerLineWidth, aTableBorder.TopLine.LineDistance);
}

DECLARE_WW8EXPORT_TEST(testN750255, "n750255.doc")
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
    CPPUNIT_ASSERT_EQUAL( u"one"_ustr, text1->getString());
    CPPUNIT_ASSERT_EQUAL( u"two"_ustr, text2->getString());
    uno::Reference<beans::XPropertySet> paragraphProperties1(paragraph1, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> paragraphProperties2(paragraph2, uno::UNO_QUERY);
    OUString pageStyle1, pageStyle2;
    paragraphProperties1->getPropertyValue( u"PageStyleName"_ustr ) >>= pageStyle1;
    paragraphProperties2->getPropertyValue( u"PageStyleName"_ustr ) >>= pageStyle2;
    CPPUNIT_ASSERT_EQUAL( u"Convert 1"_ustr, pageStyle1 );
    CPPUNIT_ASSERT_EQUAL( u"Convert 2"_ustr, pageStyle2 );

}

DECLARE_WW8EXPORT_TEST(testN652364, "n652364.doc")
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
    CPPUNIT_ASSERT_EQUAL( u"text1"_ustr, text1->getString());
    CPPUNIT_ASSERT_EQUAL( u"text2"_ustr, text2->getString());
    uno::Reference<beans::XPropertySet> paragraphProperties1(paragraph1, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> paragraphProperties2(paragraph2, uno::UNO_QUERY);
    OUString pageStyle1, pageStyle2;
    paragraphProperties1->getPropertyValue( u"PageStyleName"_ustr ) >>= pageStyle1;
    paragraphProperties2->getPropertyValue( u"PageStyleName"_ustr ) >>= pageStyle2;
    // "Standard" is the style for the first page (2nd is "Convert 1").
    CPPUNIT_ASSERT_EQUAL( u"Standard"_ustr, pageStyle1 );
    CPPUNIT_ASSERT_EQUAL( u"Standard"_ustr, pageStyle2 );
}

DECLARE_WW8EXPORT_TEST(testN757118, "n757118.doc")
{
/*
Two pairs of horizontal rules (one absolute width, one relative width)
have the same width (full page width, half page width).
xray ThisComponent.DrawPage.getByIndex(0).BoundRect
*/
    uno::Reference<drawing::XShape> rule1 = getShape(1), rule2 = getShape(2), rule3 = getShape(3), rule4 = getShape(4);
    uno::Reference<beans::XPropertySet> ruleProperties1(rule1, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> ruleProperties2(rule2, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> ruleProperties3(rule3, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> ruleProperties4(rule4, uno::UNO_QUERY);
    awt::Rectangle boundRect1, boundRect2, boundRect3, boundRect4;
    ruleProperties1->getPropertyValue( u"BoundRect"_ustr ) >>= boundRect1;
    ruleProperties2->getPropertyValue( u"BoundRect"_ustr ) >>= boundRect2;
    ruleProperties3->getPropertyValue( u"BoundRect"_ustr ) >>= boundRect3;
    ruleProperties4->getPropertyValue( u"BoundRect"_ustr ) >>= boundRect4;
    // compare, allow for < 5 differences because of rounding errors
    CPPUNIT_ASSERT( abs( boundRect1.Width - boundRect3.Width ) < 5 );
    CPPUNIT_ASSERT( abs( boundRect2.Width - boundRect4.Width ) < 5 );
}

DECLARE_WW8EXPORT_TEST(testTdf75539_relativeWidth, "tdf75539_relativeWidth.doc")
{
    //divide everything by 10 to give a margin of error for rounding etc.
    sal_Int32 pageWidth = parseDump("/root/page[1]/body/infos/bounds"_ostr, "width"_ostr).toInt32()/10;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page width", sal_Int32(9354/10), pageWidth);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("100% width line", pageWidth,   parseDump("/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion"_ostr, "width"_ostr).toInt32()/10);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("50% width line", pageWidth/2,  parseDump("/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion"_ostr, "width"_ostr).toInt32()/10);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("25% width line", pageWidth/4,  parseDump("/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion"_ostr, "width"_ostr).toInt32()/10);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("10% width line", pageWidth/10, parseDump("/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion"_ostr, "width"_ostr).toInt32()/10);
}

DECLARE_WW8EXPORT_TEST(testN757905, "n757905.doc")
{
    // The problem was that the paragraph had only a single fly
    // (as-character-anchored), and the height of that was smaller than the
    // paragraph height. When in Word-compat mode, we should take the max of
    // the two, not just the height of the fly.

    OUString aHeight = parseDump("/root/page/body/txt/infos/bounds"_ostr, "height"_ostr);
    CPPUNIT_ASSERT(sal_Int32(31) < aHeight.toInt32());
}

DECLARE_WW8EXPORT_TEST(testAllGapsWord, "all_gaps_word.doc")
{
    BorderTest borderTest;
    BorderTest::testTheBorders(mxComponent, true);
}

DECLARE_WW8EXPORT_TEST(testI120158, "i120158.doc")
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

DECLARE_WW8EXPORT_TEST(testN816603, "n816603.doc")
{
    // Bugdoc was 5 pages in Word, 1 in Writer due to pointlessly wrapping the
    // table in a frame. Exact layout may depend on fonts available, etc. --
    // but at least make sure that our table spans over multiple pages now.
    CPPUNIT_ASSERT(getPages() > 1);
}

DECLARE_WW8EXPORT_TEST(testPageBorder, "page-border.doc")
{
    // Page border was missing (LineWidth was 0), due to wrong interpretation of pgbApplyTo.
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"TopBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(convertTwipToMm100(6 * 20)), aBorder.LineWidth);
}

DECLARE_WW8EXPORT_TEST(testN823651, "n823651.doc")
{
    // Character height was 10pt instead of 7.5pt in the header.
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = getProperty< uno::Reference<text::XTextRange> >(xStyle, u"HeaderTextFirst"_ustr)->getText();
    CPPUNIT_ASSERT_EQUAL(7.5f, getProperty<float>(getParagraphOfText(1, xText), u"CharHeight"_ustr));
}

DECLARE_WW8EXPORT_TEST(testFdo36868, "fdo36868.doc")
{
    OUString aText = parseDump("/root/page/body/txt[3]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr, "expand"_ostr);
    // This was 1.1.
    CPPUNIT_ASSERT_EQUAL(u"2.1"_ustr, aText);
}

DECLARE_WW8EXPORT_TEST(testListNolevel, "list-nolevel.doc")
{
    // Similar to fdo#36868, numbering portions had wrong values.
    OUString aText = parseDump("/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr, "expand"_ostr);
    // PortionType::Number was completely missing.
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, aText);
}

DECLARE_WW8EXPORT_TEST(testHeaderApoTable, "ooo92948-1.doc")
{
    // the problem was that a table anchored in the header was split across
    // 3 text frames and quite messed up

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<text::XText> xFrame(xIndexAccess->getByIndex(1), uno::UNO_QUERY);

//    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(getStyles("PageStyles")->getByName("Standard"), "HeaderTextFirst");
//    uno::Reference<text::XTextRange> xPara(getParagraphOfText(9, xHeaderText));
//TODO why does this not work
//    uno::Reference<beans::XPropertySet> xFrame(getParagraphAnchoredObject(1, xPara));

    uno::Reference<text::XTextContent> xTable(getParagraphOrTable(1, xFrame));
    getCell(xTable, u"A1"_ustr, u"Aan" SAL_NEWLINE_STRING "Recipient" SAL_NEWLINE_STRING "Recipient" SAL_NEWLINE_STRING ""_ustr);
    getCell(xTable, u"A2"_ustr, u"Kopie aan" SAL_NEWLINE_STRING ""_ustr);
    getCell(xTable, u"A3"_ustr, u"Datum" SAL_NEWLINE_STRING "31 juli 2008"_ustr);
    getCell(xTable, u"A4"_ustr, u"Locatie" SAL_NEWLINE_STRING "Locationr"_ustr);
    getCell(xTable, u"A5"_ustr, u"Van" SAL_NEWLINE_STRING "Sender  "_ustr);
    getCell(xTable, u"A6"_ustr, u"Directie" SAL_NEWLINE_STRING "Department"_ustr);
    getCell(xTable, u"A7"_ustr, u"Telefoon" SAL_NEWLINE_STRING "Phone"_ustr);
}

DECLARE_WW8EXPORT_TEST(testBnc821208, "bnc821208.doc")
{
    // WW8Num1z0 earned a Symbol font, turning numbers into rectangles.
    uno::Reference<beans::XPropertyState> xPropertyState(getStyles(u"CharacterStyles"_ustr)->getByName(u"WW8Num1z0"_ustr), uno::UNO_QUERY);
    beans::PropertyState ePropertyState = xPropertyState->getPropertyState(u"CharFontName"_ustr);
    // This was beans::PropertyState_DIRECT_VALUE.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE, ePropertyState);

    // Background of the numbering itself should have been the default, was yellow (0xffff00).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xPropertyState, u"CharBackColor"_ustr));
}

DECLARE_WW8EXPORT_TEST(testCp1000044, "cp1000044.doc")
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    // It wasn't possible to fill out this form.
    CPPUNIT_ASSERT_EQUAL(false, bool(xStorable->isReadonly()));

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT_EQUAL( true, pDoc->getIDocumentSettingAccess().get( DocumentSettingId::PROTECT_FORM ) );

    uno::Sequence<beans::PropertyValue> aGrabBag = getProperty< uno::Sequence<beans::PropertyValue> >(mxComponent, u"InteropGrabBag"_ustr);
    sal_Int32 nPasswordHash = 0;
    for ( sal_Int32 i = 0; i < aGrabBag.getLength(); ++i )
    {
        if ( aGrabBag[i].Name == "FormPasswordHash" )
            aGrabBag[i].Value >>= nPasswordHash;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Password Hash", sal_Int32(609995782), nPasswordHash);
}

DECLARE_WW8EXPORT_TEST(testBorderColours, "bordercolours.doc")
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
    uno::Reference<container::XNameAccess> bookmarks =
        bookmarksSupplier->getBookmarks();
    uno::Reference<text::XTextContent> bookmark(
        bookmarks->getByName(u"ParagraphBorder"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> anchor(bookmark->getAnchor());
    table::BorderLine2 border;
    border = getProperty<table::BorderLine2>(anchor, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(anchor, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(anchor, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);

    // Page border
    OUString pageStyleName = getProperty<OUString>(anchor, u"PageStyleName"_ustr);
    uno::Reference<style::XStyle> pageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(pageStyleName), uno::UNO_QUERY);
    border = getProperty<table::BorderLine2>(pageStyle, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(pageStyle, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(pageStyle, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(pageStyle, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);

    // Character border
    bookmark.set(bookmarks->getByName(u"CharBorder"_ustr), uno::UNO_QUERY);
    anchor = bookmark->getAnchor();
    border = getProperty<table::BorderLine2>(anchor, u"CharTopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, u"CharLeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, u"CharRightBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, u"CharBottomBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);

    // Table border
    uno::Reference<text::XTextTablesSupplier> tablesSupplier(mxComponent,
        uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> tables =
        tablesSupplier->getTextTables();
    uno::Reference<text::XTextTable> table(
        tables->getByName(u"Table1"_ustr), uno::UNO_QUERY);
    table::TableBorder2 tableBorder = getProperty<table::TableBorder2>(
        table, u"TableBorder2"_ustr);
    CPPUNIT_ASSERT_EQUAL(expectedTop.Color, tableBorder.TopLine.Color);
    CPPUNIT_ASSERT_EQUAL(expectedLeft.Color, tableBorder.LeftLine.Color);
    CPPUNIT_ASSERT_EQUAL(expectedRight.Color, tableBorder.RightLine.Color);
    CPPUNIT_ASSERT_EQUAL(expectedBottom.Color, tableBorder.BottomLine.Color);

    // Table cells
    uno::Reference<table::XCell> cell =
        table->getCellByName(u"A2"_ustr);
    border = getProperty<table::BorderLine2>(cell, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(cell, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(cell, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);

    cell = table->getCellByName(u"B2"_ustr);
    border = getProperty<table::BorderLine2>(cell, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);
    border = getProperty<table::BorderLine2>(cell, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(cell, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);

    cell = table->getCellByName(u"C2"_ustr);
    border = getProperty<table::BorderLine2>(cell, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);
    border = getProperty<table::BorderLine2>(cell, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDashedRed, border);
    border = getProperty<table::BorderLine2>(cell, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(cell, u"BottomBorder"_ustr);
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

DECLARE_WW8EXPORT_TEST(testMsoBrightnessContrast, "msobrightnesscontrast.doc")
{
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> graphic;
    imageProperties->getPropertyValue( u"Graphic"_ustr ) >>= graphic;
    Graphic vclGraphic(graphic);
    BitmapEx bitmap(vclGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL( tools::Long(58), bitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL( tools::Long(320), bitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL( Color(206,206,206), bitmap.GetPixelColor(16,27));
    CPPUNIT_ASSERT_EQUAL( Color(206,206,206), bitmap.GetPixelColor(22,48));
}

DECLARE_WW8EXPORT_TEST(testTdf95321, "tdf95321.doc")
{
    // The problem was that there should be content in the second cell
    // but there wasn't.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Second Column"_ustr, uno::Reference<text::XTextRange>(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY_THROW)->getString());
}

DECLARE_WW8EXPORT_TEST(testFdo77844, "fdo77844.doc")
{
    uno::Reference<container::XNameAccess> pageStyles = getStyles(u"PageStyles"_ustr);

    // get a page cursor
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    // check that the first page has no header
    xCursor->jumpToFirstPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
    uno::Reference<style::XStyle> pageStyle(
        pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    bool headerIsOn = getProperty<bool>(pageStyle, u"HeaderIsOn"_ustr);
    CPPUNIT_ASSERT(!headerIsOn);

    // check that the second page has a header
    xCursor->jumpToPage(2);
    pageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
    pageStyle.set(
        pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    headerIsOn = getProperty<bool>(pageStyle, u"HeaderIsOn"_ustr);
    CPPUNIT_ASSERT(headerIsOn);

    // check that the third page has a header
    xCursor->jumpToPage(3);
    pageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
    pageStyle.set(
        pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    headerIsOn = getProperty<bool>(pageStyle, u"HeaderIsOn"_ustr);
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

DECLARE_WW8EXPORT_TEST(testFdp80333, "fdo80333.doc")
{
    // Despite there is no character border, border shadow was imported
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xRun, u"CharShadowFormat"_ustr);
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);
}

DECLARE_WW8EXPORT_TEST(testFdo81102, "fdo81102.doc")
{
    // get page style at beginning of document
    uno::Reference<text::XTextDocument> textDocument(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> start =
        textDocument->getText()->getStart();
    OUString pageStyleName = getProperty<OUString>(start, u"PageStyleName"_ustr);
    uno::Reference<style::XStyle> pageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(pageStyleName), uno::UNO_QUERY);

    // check that left and right pages do not share the same header
    bool headerIsShared = getProperty<bool>(pageStyle, u"HeaderIsShared"_ustr);
    CPPUNIT_ASSERT(!headerIsShared);
}

DECLARE_WW8EXPORT_TEST(testBnc787942, "bnc787942.doc")
{
    // The frame ended up on the second page instead of first.
    // this is on page 1 in Word
    parseDump("/root/page[1]/body/txt[4]/anchored"_ostr);

    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_PARALLEL, getProperty<text::WrapTextMode>(getShape(1), u"Surround"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(getShape(1), u"HoriOrientRelation"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf133504_wrapNotBeside, "tdf133504_wrapNotBeside.doc")
{
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_NONE, getProperty<text::WrapTextMode>(getShape(1), u"Surround"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf36711_inlineFrames, "tdf36711_inlineFrames.doc")
{
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::FRAME, getProperty<sal_Int16>(getShape(1), u"VertOrientRelation"_ustr));
}

DECLARE_WW8EXPORT_TEST(testLayoutHanging, "fdo68967.doc")
{
    // This must not hang in layout
}

#if HAVE_MORE_FONTS
DECLARE_WW8EXPORT_TEST(testfdo68963, "fdo68963.doc")
{
    // The problem was that the text was not displayed.
    CPPUNIT_ASSERT ( !parseDump("/root/page/body/tab/row[2]/cell[1]/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr).isEmpty() );
    CPPUNIT_ASSERT_EQUAL( u"Topic 1"_ustr, parseDump("/root/page/body/tab/row[2]/cell[1]/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr) );
    // all crossreference bookmarks should have a target.  Shouldn't be any "Reference source not found" in the xml
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), parseDump("/root/page/body/txt[24]/SwParaPortion/SwLineLayout/SwFieldPortion[2]"_ostr,"expand"_ostr).indexOf("Reference source not found"));
}
#endif

DECLARE_WW8EXPORT_TEST(testTdf99100, "tdf99100.doc")
{
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"HeaderText"_ustr);
    auto xField = getProperty< uno::Reference<lang::XServiceInfo> >(getRun(getParagraphOfText(1, xHeaderText), 2), u"TextField"_ustr);
    // This failed: the second text portion wasn't a field.
    CPPUNIT_ASSERT(xField.is());
    CPPUNIT_ASSERT(xField->supportsService(u"com.sun.star.text.textfield.Chapter"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf74328, "tdf74328.doc")
{
/*
reading page numbers at sections > 255, in this case 256
*/
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xTextCursor = textDocument->getText()->createTextCursor( );
    uno::Reference<beans::XPropertySet> xProps(xTextCursor, uno::UNO_QUERY);
    uno::Any aOffset = xProps->getPropertyValue(u"PageNumberOffset"_ustr);
    sal_Int16 nOffset = 0;
    aOffset >>= nOffset;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(256), nOffset);
}

DECLARE_WW8EXPORT_TEST(testTdf95576, "tdf95576.doc")
{
    // The first three paragraphs in this document (which are headings)
    // should have zero indent and first line indent
    for (int nPara = 1; nPara <= 3; ++nPara) {
        std::cout << "nPara = " << nPara << "\n";
        auto xPara = getParagraph(nPara);

        // get the numbering rules effective at this paragraph
        uno::Reference<container::XIndexReplace> xNumRules =
            getProperty< uno::Reference<container::XIndexReplace> >(
                xPara, u"NumberingRules"_ustr);

        // get the numbering level of this paragraph, and the properties
        // associated with that numbering level
        int numLevel = getProperty<sal_Int32>(xPara, u"NumberingLevel"_ustr);
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

DECLARE_WW8EXPORT_TEST(testTdf59896, "tdf59896.doc")
{
    // This was awt::FontWeight::NORMAL, i.e. the first run wasn't bold, when it should be bold
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(getParagraph(1), 1), u"CharWeight"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf102334, "tdf102334.doc")
{
    // This was false, i.e. the first run wasn't hidden, when it should have been
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getRun(getParagraph(7), 1), u"CharHidden"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf128605, "tdf128605.doc")
{
    OUString aPara1PageStyleName = getProperty<OUString>(getParagraph(1), u"PageStyleName"_ustr);
    OUString aPara2PageStyleName = getProperty<OUString>(getParagraph(2), u"PageStyleName"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Standard
    // - Actual  : Convert 1
    // i.e. the continuous section break resulted in an unwanted page break.
    CPPUNIT_ASSERT_EQUAL(aPara1PageStyleName, aPara2PageStyleName);
}

DECLARE_WW8EXPORT_TEST(testTdf112535, "tdf112535.doc")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc->GetSpzFrameFormats());

    auto& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(!rFormats.empty());

    const auto pFormat = rFormats[0];
    CPPUNIT_ASSERT(pFormat);

    // Without the accompanying fix in place, this test would have failed: auto-contour was enabled
    // in Writer, but not in Word.
    CPPUNIT_ASSERT(!pFormat->GetSurround().IsContour());
}

DECLARE_WW8EXPORT_TEST(testTdf106291, "tdf106291.doc")
{
    // Table cell was merged vertically instead of horizontally -> had incorrect dimensions
    OUString cellWidth = parseDump("/root/page[1]/body/tab/row/cell[1]/infos/bounds"_ostr, "width"_ostr);
    OUString cellHeight = parseDump("/root/page[1]/body/tab/row/cell[1]/infos/bounds"_ostr, "height"_ostr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8650), cellWidth.toInt32());
    CPPUNIT_ASSERT(cellHeight.toInt32() > 200); // height might depend on font size
}

DECLARE_WW8EXPORT_TEST(testTransparentText, "transparent-text.doc")
{
    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"HeaderText"_ustr);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(3, xHeaderText);
    // Without the accompanying fix in place, this test would have failed: transparency was set to
    // 100%, so the text was not readable.
    sal_Int32 nExpected(COL_BLACK);
    sal_Int32 nActual(getProperty<sal_Int16>(xParagraph, u"CharTransparence"_ustr));
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

DECLARE_WW8EXPORT_TEST( testTdf105570, "tdf105570.doc" )
{
    /*****
      * MS-DOC specification ( https://msdn.microsoft.com/en-us/library/cc313153 )
      * ch. 2.6.3, sprmTTableHeader:
      *     A Bool8 value that specifies that the current table row is a header row.
      *     If the value is 0x01 but sprmTTableHeader is not applied with a value of 0x01
      *     for a previous row in the same table, then this property MUST be ignored.
      *
      * The document have three tables with three rows.
      * Table 1 has { 1, 0, 0 } values of the "repeat as header row" property for each row
      * Table 2 has { 1, 1, 0 }
      * Table 3 has { 0, 1, 1 }
      ****/
    SwXTextDocument* pTextDoc     = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc*           pDoc         = pTextDoc->GetDocShell()->GetDoc();
    SwWrtShell*      pWrtShell    = pDoc->GetDocShell()->GetWrtShell();
    SwShellCursor*   pShellCursor = pWrtShell->getShellCursor( false );
    SwNodeIndex      aIdx( pShellCursor->Start()->GetNode() );

    // Find first table
    SwTableNode*     pTableNd     = aIdx.GetNode().FindTableNode();

    CPPUNIT_ASSERT_EQUAL( sal_uInt16(1), pTableNd->GetTable().GetRowsToRepeat() );

    // Go to next table
    aIdx.Assign( *pTableNd->EndOfSectionNode(), 1 );
    while ( nullptr == (pTableNd = aIdx.GetNode().GetTableNode()) ) ++aIdx;

    CPPUNIT_ASSERT_EQUAL( sal_uInt16(2), pTableNd->GetTable().GetRowsToRepeat() );

    // Go to next table
    aIdx.Assign( *pTableNd->EndOfSectionNode(), 1 );
    while ( nullptr == (pTableNd = aIdx.GetNode().GetTableNode()) ) ++aIdx;

    // As first row hasn't sprmTTableHeader set, all following must be ignored, so no rows must be repeated
    CPPUNIT_ASSERT_EQUAL( sal_uInt16(0), pTableNd->GetTable().GetRowsToRepeat() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf112346)
{
    auto verify = [this]() {
        // Multi-page table was imported as a single page.
        CPPUNIT_ASSERT_EQUAL(2, getPages());
    };
    createSwDoc("tdf112346.doc");
    verify();
    saveAndReload(u"MS Word 97"_ustr);
    verify();
}

DECLARE_WW8EXPORT_TEST(testTdf79639, "tdf79639.doc")
{
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // as the floating table in the header wasn't converted to a TextFrame.
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

DECLARE_WW8EXPORT_TEST(testTdf122425_2, "tdf122425_2.doc")
{
    // This is for graphic objects in headers/footers
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwPosFlyFrames aPosFlyFrames = pDoc->GetAllFlyFormats(nullptr, false);
    // There is one fly frame in the document: the text box
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPosFlyFrames.size());
    for (const SwPosFlyFrame& rPosFlyFrame : aPosFlyFrames)
    {
        const SwFrameFormat& rFormat = rPosFlyFrame.GetFormat();
        const SfxPoolItem* pItem = nullptr;

        // Check for correct explicitly-set values of UL spacings. Previously this was "DEFAULT",
        // and resulted in inherited values (114 = 2 mm) used.
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, rFormat.GetItemState(RES_UL_SPACE, false, &pItem));
        auto pUL = static_cast<const SvxULSpaceItem*>(pItem);
        CPPUNIT_ASSERT(pUL);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pUL->GetUpper());
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pUL->GetLower());
    }
}

DECLARE_WW8EXPORT_TEST(testTdf130262, "tdf130262.doc")
{
    // We had an infinite layout loop
}

DECLARE_WW8EXPORT_TEST(testTdf38778, "tdf38778_properties_in_run_for_field.doc")
{
    CPPUNIT_ASSERT_EQUAL(10.0f, getProperty<float>(getRun(getParagraph(1), 1), u"CharHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Courier New"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1), u"CharFontName"_ustr));
}

DECLARE_WW8EXPORT_TEST(testN325936, "n325936.doc")
{
    /*
     * The problem was that the transparent background of the drawing in the
     * header was exported as non-transparent.
     *
     * xray ThisComponent.DrawPage(0).BackColorTransparency
     */

    CPPUNIT_ASSERT_EQUAL(Color(0x000064), getProperty< Color >(getShape(1), u"BackColorTransparency"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTscp, "tscp.doc")
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, u"urn:bails"_ustr);
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(mxComponent, uno::UNO_QUERY);
    uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = xDocumentMetadataAccess->getMetadataGraphsWithType(xType);
    // This failed, no graphs had the urn:bails type.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aGraphNames.getLength());
    uno::Reference<rdf::XURI> xGraphName = aGraphNames[0];
    uno::Reference<rdf::XNamedGraph> xGraph = xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);

    // No RDF statement on the first paragraph.
    uno::Reference<rdf::XResource> xParagraph(getParagraph(1), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xStatements = xGraph->getStatements(xParagraph, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
    CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(xStatements->hasMoreElements()));

    // 3 RDF statements on the second paragraph.
    xParagraph.set(getParagraph(2), uno::UNO_QUERY);
    std::map<OUString, OUString> aExpectedStatements =
    {
        {"urn:bails:ExportControl:BusinessAuthorization:Identifier", "urn:example:tscp:1"},
        {"urn:bails:ExportControl:BusinessAuthorizationCategory:Identifier", "urn:example:tscp:1:confidential"},
        {"urn:bails:ExportControl:Authorization:StartValidity", "2015-11-27"}
    };
    std::map<OUString, OUString> aActualStatements;
    xStatements = xGraph->getStatements(xParagraph, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
    while (xStatements->hasMoreElements())
    {
        rdf::Statement aStatement = xStatements->nextElement().get<rdf::Statement>();
        aActualStatements[aStatement.Predicate->getNamespace() + aStatement.Predicate->getLocalName()] = aStatement.Object->getStringValue();
    }
    CPPUNIT_ASSERT(bool(aExpectedStatements == aActualStatements));

    // No RDF statement on the third paragraph.
    xParagraph.set(getParagraph(3), uno::UNO_QUERY);
    xStatements = xGraph->getStatements(xParagraph, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
    CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(xStatements->hasMoreElements()));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo45724)
{
    loadAndReload("fdo45724.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The text and background color of the control shape was not correct.
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<form::validation::XValidatableFormComponent> xComponent(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<Color>(xComponent, u"BackgroundColor"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"xxx"_ustr, xComponent->getCurrentValue().get<OUString>());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136620)
{
    loadAndReload("tdf136620.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    uno::Reference<drawing::XShape> xShape = getShape(1);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(5636), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1826), xShape->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6630), xShape->getSize().Height);

    // Without the fix in place, this test would have failed with
    // - Expected: 5853
    // - Actual  : 850
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5853), xShape->getSize().Width);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo46020)
{
    loadAndReload("fdo46020.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The footnote in that document wasn't exported, check that it is actually exported
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xFootnotes->getCount());
}

DECLARE_WW8EXPORT_TEST(testZoom, "zoom.doc")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue(u"ZoomValue"_ustr) >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

DECLARE_WW8EXPORT_TEST(testZoomType, "zoomtype.doc")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue(u"ZoomType"_ustr) >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(view::DocumentZoomType::PAGE_WIDTH), nValue);
}

DECLARE_WW8EXPORT_TEST(test56513, "fdo56513.doc")
{
    CPPUNIT_ASSERT_EQUAL(u"This is the header of the first section"_ustr,  parseDump("/root/page[1]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"This is the first page header of the second section"_ustr,   parseDump("/root/page[2]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"This is the non-first-page header of the second section"_ustr,  parseDump("/root/page[3]/header/txt/text()"_ostr));
}

DECLARE_WW8EXPORT_TEST(testNewPageStylesTable, "new-page-styles.doc")
{
    CPPUNIT_ASSERT_EQUAL(u"Sigma Space Performance Goals and Results (Page 1)*"_ustr,  parseDump("/root/page[1]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Sigma Space Performance Assessment (Page 2)****"_ustr,   parseDump("/root/page[2]/header/txt/text()"_ostr));
    CPPUNIT_ASSERT_EQUAL(u"Sigma Space Performance Goals: Next Year (Page 3)*******"_ustr,  parseDump("/root/page[3]/header/txt/text()"_ostr));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo42144)
{
    loadAndReload("fdo42144.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Footer wasn't disabled -- instead empty footer was exported.
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xStyle, u"FooterIsOn"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testCharacterBorder)
{
    loadAndReload("charborder.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    // WW8 has just one border attribute (sprmCBrc) for text border so all side has
    // the same border
    // Border
    {
        const table::BorderLine2 aTopBorder = getProperty<table::BorderLine2>(xRun,u"CharTopBorder"_ustr);
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF3333,0,318,0,0,318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,u"CharLeftBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,u"CharBottomBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,u"CharRightBorder"_ustr));
    }

    // Padding (dptSpace) it is constant 0
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,u"CharTopBorderDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,u"CharLeftBorderDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,u"CharBottomBorderDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,u"CharRightBorderDistance"_ustr));
    }

    // Shadow (fShadow)
    /* WW8 use just one bool value for shadow so the next conversions
       are made during an export-import round
       color: any -> black
       location: any -> bottom-right
       width: any -> border width */
    {
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xRun, u"CharShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf41542_imagePadding)
{
    loadAndReload("tdf41542_imagePadding.odt");
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // borderlessImage - image WITHOUT BORDERS : simulate padding with -crop
    text::GraphicCrop crop = getProperty<text::GraphicCrop>(getShape(2), u"GraphicCrop"_ustr);
    CPPUNIT_ASSERT( crop.Left != 0 );
    CPPUNIT_ASSERT( crop.Right != 0 );
    CPPUNIT_ASSERT_EQUAL( crop.Left, crop.Top );
    CPPUNIT_ASSERT_EQUAL( crop.Right, crop.Bottom );
    CPPUNIT_ASSERT_EQUAL( crop.Left, crop.Right );

    // borderedImage - image WITH BORDERS : simulate padding with -crop
    crop = getProperty<text::GraphicCrop>(getShape(3), u"GraphicCrop"_ustr);
    CPPUNIT_ASSERT( crop.Left != 0 );
    CPPUNIT_ASSERT( crop.Right != 0 );
    CPPUNIT_ASSERT_EQUAL( crop.Left, crop.Top );
    CPPUNIT_ASSERT_EQUAL( crop.Right, crop.Bottom );
    CPPUNIT_ASSERT_EQUAL( crop.Left, crop.Right );
}

DECLARE_WW8EXPORT_TEST(testFdo77454, "fdo77454.doc")
{
    {
        // check negative crops round-trip  (with border/padding of 1)
        text::GraphicCrop const crop =
            getProperty<text::GraphicCrop>(getShape(1), u"GraphicCrop"_ustr);
        CPPUNIT_ASSERT(abs(sal_Int32( -439) - crop.Left)   <= 2);
        CPPUNIT_ASSERT(abs(sal_Int32(-7040) - crop.Right)  <= 2);
        CPPUNIT_ASSERT(abs(sal_Int32( -220) - crop.Top)    <= 2);
        CPPUNIT_ASSERT(abs(sal_Int32(-7040) - crop.Bottom) <= 2);
    }

    {
        // check positive crops round-trip (with padding of 1)
        text::GraphicCrop const crop =
            getProperty<text::GraphicCrop>(getShape(2), u"GraphicCrop"_ustr);
        CPPUNIT_ASSERT(abs(sal_Int32( 326) - crop.Left)   <= 3);
        CPPUNIT_ASSERT(abs(sal_Int32(1208) - crop.Right)  <= 3);
        CPPUNIT_ASSERT(abs(sal_Int32(1635) - crop.Top)    <= 3);
        CPPUNIT_ASSERT(abs(sal_Int32(  95) - crop.Bottom) <= 3);
    }
}

DECLARE_WW8EXPORT_TEST(testFdo59530, "fdo59530.doc")
{
    // See ooxmlexport's testFdo38244().
    // Test comment range feature.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr, getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr, getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));

    // Test initials.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"M"_ustr, getProperty<OUString>(xPropertySet, u"Initials"_ustr));

    // Test commented text range which spans over more text nodes
    // Comment starts in the second paragraph
    xRunEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr, getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));
    // Comment ends in the third paragraph
    xRunEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr, getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));
}

DECLARE_WW8EXPORT_TEST(testCommentsNested, "comments-nested.doc")
{
    uno::Reference<beans::XPropertySet> xOuter = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), u"TextField"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Outer"_ustr, getProperty<OUString>(xOuter, u"Content"_ustr));

    uno::Reference<beans::XPropertySet> xInner = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 4), u"TextField"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Inner"_ustr, getProperty<OUString>(xInner, u"Content"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testBorderColoursExport)
{
    loadAndReload("bordercolours.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // This is very close to testBorderColours in ww8import.cxx, but for export

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
    uno::Reference<container::XNameAccess> bookmarks =
        bookmarksSupplier->getBookmarks();
    uno::Reference<text::XTextContent> bookmark(
        bookmarks->getByName(u"ParagraphBorder"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> anchor(bookmark->getAnchor());
    table::BorderLine2 border;
    border = getProperty<table::BorderLine2>(anchor, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(anchor, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(anchor, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);

    // Page border
    OUString pageStyleName = getProperty<OUString>(anchor, u"PageStyleName"_ustr);
    uno::Reference<style::XStyle> pageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(pageStyleName), uno::UNO_QUERY);
    border = getProperty<table::BorderLine2>(pageStyle, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(pageStyle, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
    border = getProperty<table::BorderLine2>(pageStyle, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(pageStyle, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);

    // Character border
    bookmark.set(bookmarks->getByName(u"CharBorder"_ustr), uno::UNO_QUERY);
    anchor = bookmark->getAnchor();
    border = getProperty<table::BorderLine2>(anchor, u"CharTopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, u"CharLeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, u"CharRightBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(anchor, u"CharBottomBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);

    // Table border
    uno::Reference<text::XTextTablesSupplier> tablesSupplier(mxComponent,
        uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> tables =
        tablesSupplier->getTextTables();
    uno::Reference<text::XTextTable> table(
        tables->getByName(u"Table1"_ustr), uno::UNO_QUERY);
    table::TableBorder2 tableBorder = getProperty<table::TableBorder2>(
        table, u"TableBorder2"_ustr);
    CPPUNIT_ASSERT_EQUAL(expectedTop.Color, tableBorder.TopLine.Color);
    CPPUNIT_ASSERT_EQUAL(expectedLeft.Color, tableBorder.LeftLine.Color);
    CPPUNIT_ASSERT_EQUAL(expectedRight.Color, tableBorder.RightLine.Color);
#if 0
    // #if'd out because the "fine dashed" border line style for table borders
    // does not seem to save or load correctly in odt format at present
    CPPUNIT_ASSERT_EQUAL(expectedBottom.Color, tableBorder.BottomLine.Color);
#endif

    // Table cells
    uno::Reference<table::XCell> cell =
        table->getCellByName(u"A2"_ustr);
    border = getProperty<table::BorderLine2>(cell, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(cell, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
#if 0
    // #if'd out because the "fine dashed" border line style for table borders
    // does not seem to save or load correctly in odt format at present
    border = getProperty<table::BorderLine2>(cell, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);
#endif

    cell = table->getCellByName(u"B2"_ustr);
    border = getProperty<table::BorderLine2>(cell, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);
    border = getProperty<table::BorderLine2>(cell, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(cell, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);

    cell = table->getCellByName(u"C2"_ustr);
    border = getProperty<table::BorderLine2>(cell, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);
    border = getProperty<table::BorderLine2>(cell, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDashedRed, border);
    border = getProperty<table::BorderLine2>(cell, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedRight, border);
    border = getProperty<table::BorderLine2>(cell, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedDoubleGreen, border);

    // Picture border
    // (#if'd out as they are not yet exported with correct colours)
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

CPPUNIT_TEST_FIXTURE(Test, testRedlineExport1)
{
    loadAndReload("redline-export-1.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    //there must be no redline information on the first line before and after reloading
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(false, hasProperty(xRun, u"RedlineType"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineExport2)
{
    loadAndReload("redline-export-2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    //there must be redline information on the first portion of the third paragraph before and after reloading
    CPPUNIT_ASSERT_EQUAL(true, hasProperty(getRun(getParagraph(3), 1), u"RedlineType"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineExport3)
{
    loadAndReload("redline-export-3.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    //there must be redline information just on the para-break boundary between para one and two
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(getParagraph(1), 1), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, hasProperty(getRun(getParagraph(1), 2), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, hasProperty(getRun(getParagraph(2), 1), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(getParagraph(2), 2), u"RedlineType"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testCellBgColor)
{
    loadAndReload("cell-bg-color.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xCC0000), getProperty<Color>(xTable->getCellByName(u"A1"_ustr), u"BackColor"_ustr));
}

DECLARE_WW8EXPORT_TEST(testBnc636128, "bnc636128.doc")
{
    // Import / export of FFData.cch was missing.
    uno::Reference<text::XFormField> xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(1), 2), u"Bookmark"_ustr);
    uno::Reference<container::XNameContainer> xParameters = xFormField->getParameters();
    // This resulted in a container.NoSuchElementException.
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), xParameters->getByName(u"MaxLength"_ustr).get<sal_uInt16>());
}


DECLARE_WW8EXPORT_TEST(testWw8Cjklist30, "cjklist30.doc")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TIAN_GAN_ZH, numFormat);
}

DECLARE_WW8EXPORT_TEST(testWw8Cjklist31, "cjklist31.doc")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::DI_ZI_ZH, numFormat);
}

DECLARE_WW8EXPORT_TEST(testWw8Cjklist34, "cjklist34.doc")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH_TW, numFormat);
}

DECLARE_WW8EXPORT_TEST(testWw8Cjklist35, "cjklist35.doc")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_LOWER_ZH, numFormat);
}

DECLARE_WW8EXPORT_TEST(testTdf118564, "tdf118564.doc")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(3);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_LOWER_ZH, numFormat);
}

DECLARE_WW8EXPORT_TEST(testTdf92281, "tdf92281.doc")
{
        uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Wingdings"_ustr, getProperty<OUString>(xRun, u"CharFontName"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Wingdings"_ustr, getProperty<OUString>(xRun, u"CharFontNameAsian"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Wingdings"_ustr, getProperty<OUString>(xRun, u"CharFontNameComplex"_ustr));

        uno::Reference<text::XText> xXText = getParagraph(1)->getText();
        uno::Reference<text::XTextCursor> xCursor = xXText->createTextCursor();

        xCursor->goRight( 5 , false );
        uno::Reference< beans::XPropertySet > xPropSet(xCursor, uno::UNO_QUERY);
        static constexpr OUStringLiteral aFontname = u"\u65B0\u7D30\u660E\u9AD4;PMingLiU";
        CPPUNIT_ASSERT_EQUAL(u"Calibri"_ustr, getProperty<OUString>(xPropSet, u"CharFontName"_ustr));
        CPPUNIT_ASSERT_EQUAL(OUString(aFontname), getProperty<OUString>(xPropSet, u"CharFontNameAsian"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Times New Roman"_ustr, getProperty<OUString>(xPropSet, u"CharFontNameComplex"_ustr));
}

DECLARE_WW8EXPORT_TEST(testCommentedTable, "commented-table.doc")
{
    // Document has a non-trivial commented text range, as the range contains a table.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextContent> xField(xFields->nextElement(), uno::UNO_QUERY);
    // After first import, there was an off-by-one during import, so this was "efore.\nA1\nB1\nAfte". (Notice the additional "e" prefix.)
    // After export and import, things got worse, this was "\nA1\nB1\nAfte".
    CPPUNIT_ASSERT_EQUAL(u"fore." SAL_NEWLINE_STRING "A1" SAL_NEWLINE_STRING "B1" SAL_NEWLINE_STRING "Afte"_ustr, xField->getAnchor()->getString());
}

DECLARE_WW8EXPORT_TEST(testTextVerticalAdjustment, "tdf36117_verticalAdjustment.doc")
{
    //Preserve the page vertical alignment setting for .doc
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    SwPageDesc* pDesc = &pDoc->GetPageDesc( 0 );
    drawing::TextVerticalAdjust nVA = pDesc->GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_CENTER, nVA );

    pDesc = &pDoc->GetPageDesc( 1 );
    nVA = pDesc->GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_TOP, nVA );

    pDesc = &pDoc->GetPageDesc( 2 );
    nVA = pDesc->GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_BOTTOM, nVA );

    pDesc = &pDoc->GetPageDesc( 3 );
    nVA = pDesc->GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_BLOCK, nVA );
}

DECLARE_WW8EXPORT_TEST(testRES_MIRROR_GRAPH_BOTH, "tdf56321_flipImage_both.doc")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    for (SwNodeOffset n(0); ; n++)
    {
        SwNode* pNode = pDoc->GetNodes()[ n ];
        if (SwGrfNode *pGrfNode = pNode->GetGrfNode())
        {
            CPPUNIT_ASSERT_EQUAL(int(MirrorGraph::Both), static_cast<int>(pGrfNode->GetSwAttrSet().GetMirrorGrf().GetValue()));
            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testCommentExport)
{
    loadAndReload("comment-export.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    struct TextPortionInfo {
        OUString sKind;
        OUString sText;
        int nAnnotationID;
    };

    const TextPortionInfo aTextPortions[] = {
        {u"Annotation"_ustr, u"Comment on [A...A]"_ustr, 0},
        {u"Text"_ustr, u"[A xx "_ustr, 0},
        {u"Annotation"_ustr, u"Comment on [B...B]"_ustr, 1},
        {u"Text"_ustr, u"[B x "_ustr, 0},
        {u"Annotation"_ustr, u"Comment on [C..C]"_ustr, 2},
        {u"Text"_ustr, u"[C x B]"_ustr, 0},
        {u"AnnotationEnd"_ustr, OUString(), 1},
        {u"Text"_ustr, u" x C]"_ustr, 0},
        {u"AnnotationEnd"_ustr, OUString(), 2},
        {u"Text"_ustr, u" xx A]"_ustr, 0},
        {u"AnnotationEnd"_ustr, OUString(), 0},
        {u"Text"_ustr, u" Comment on a point"_ustr, 0},
        {u"Annotation"_ustr, u"Comment on point"_ustr, 3},
        {u"Text"_ustr, u"x "_ustr, 0},
        {u"Annotation"_ustr, u"Comment on AA...BB"_ustr, 4},
        {u"Annotation"_ustr, u"Comment on AAAAAA"_ustr, 5},
        {u"Text"_ustr, u"AAAAAA"_ustr, 0},
        {u"AnnotationEnd"_ustr, OUString(), 5},
        {u"Text"_ustr, u" BBBBBB"_ustr, 0},
        {u"AnnotationEnd"_ustr, OUString(), 4}
    };

    OUString sNames[6];

    const int nNumberOfTextPortions = SAL_N_ELEMENTS(aTextPortions);

    uno::Reference<text::XTextRange> xPara = getParagraph(1);

    for (int i = 0; i < nNumberOfTextPortions; ++i)
    {
        OUString sKind = aTextPortions[i].sKind;
        uno::Reference<text::XTextRange> xRun = getRun(xPara, i + 1);
        uno::Reference<beans::XPropertySet> xPropertySet(xRun, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sKind, getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));

        if (sKind == "Text")
        {
            // Check if textportion has the correct text
            CPPUNIT_ASSERT_EQUAL(aTextPortions[i].sText, xRun->getString());
        }
        else if (sKind == "Annotation")
        {
            // Check if the comment text is correct and save the name of the comment
            uno::Reference<beans::XPropertySet> xComment = getProperty< uno::Reference<beans::XPropertySet> >(xRun, u"TextField"_ustr);
            CPPUNIT_ASSERT_EQUAL(aTextPortions[i].sText, getProperty<OUString>(xComment, u"Content"_ustr));
            sNames[aTextPortions[i].nAnnotationID] = getProperty<OUString>(xComment, u"Name"_ustr);
        }
        else // if (sKind == OUString("AnnotationEnd"))
        {
            // Check if the correct Annotation ends here (by Name)
            uno::Reference<container::XNamed> xBookmark(getProperty< uno::Reference<beans::XPropertySet> >(xRun, u"Bookmark"_ustr), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(sNames[aTextPortions[i].nAnnotationID], xBookmark->getName());
        }
    }

    // tdf#139759 import character highlight and shade for comment text
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = getProperty<uno::Reference<text::XText>>(xField, u"TextRange"_ustr);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<Color>(getRun(xParagraph, 1), u"CharBackColor"_ustr));
}

#if HAVE_MORE_FONTS
CPPUNIT_TEST_FIXTURE(Test, testTableKeep)
{
    loadAndReload("tdf91083.odt");
    CPPUNIT_ASSERT_EQUAL(7, getPages());
    //emulate table "keep with next" -do not split table
    CPPUNIT_ASSERT_EQUAL( u"Row 1"_ustr, parseDump("/root/page[3]/body/tab[1]/row[2]/cell[1]/txt[1]"_ostr) );
    CPPUNIT_ASSERT_EQUAL( u"Row 1"_ustr, parseDump("/root/page[6]/body/tab[1]/row[2]/cell[1]/txt[1]"_ostr) );
}
#endif

CPPUNIT_TEST_FIXTURE(Test, tesTdf91083_tableKeep2)
{
    loadAndReload("tdf91083_tableKeep2.odt");
    //emulate table "keep with next" - split large row in order to keep with previous paragraph
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table doesn't split, so it starts on page 2",
                                 u"0"_ustr, parseDump("count(//page[1]//tab)"_ostr) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page 2 starts with a paragraph/title, not a table",
                                 u"KeepWithNext"_ustr, parseDump("//page[2]/body/txt[1]"_ostr) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table sticks with previous paragraph, so it starts on page 2",
                                 u"1"_ustr, parseDump("count(//page[2]//tab)"_ostr) );
    CPPUNIT_ASSERT_MESSAGE("Row itself splits, not the table at a row boundary",
                                 "Cell 2" != parseDump("//page[3]//tab//row[2]/cell[1]/txt[1]"_ostr) );
}

CPPUNIT_TEST_FIXTURE(Test, tesTdf91083_tableKeep3)
{
    loadAndReload("tdf91083_tableKeep3.odt");
    CPPUNIT_ASSERT_EQUAL(3, getPages());
    //emulate table "keep with next" - split single row table in order to keep with previous paragraph
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table doesn't split, so it starts on page 2",
                                 u"0"_ustr, parseDump("count(//page[1]//tab)"_ostr) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table sticks with previous paragraph, so it starts on page 2",
                                 u"1"_ustr, parseDump("count(//page[2]//tab)"_ostr) );
}

DECLARE_WW8EXPORT_TEST(testTdf76349_textboxMargins, "tdf76349_textboxMargins.doc")
{
    // textboxes without borders were losing their spacing items in round-tripping
    CPPUNIT_ASSERT( 0 < parseDump("/root/page/body/txt/anchored/fly/infos/prtBounds"_ostr, "left"_ostr).toInt32() );

    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox background color", Color(0xD8, 0xD8, 0xD8), getProperty<Color>(xShape, u"BackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testMoveRange)
{
    loadAndReload("fdo66304-1.odt");
    //the save must survive without asserting
}

CPPUNIT_TEST_FIXTURE(Test, testClearFramePams)
{
    loadAndReload("tdf46441-2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    //the save must survive without asserting
}

CPPUNIT_TEST_FIXTURE(Test, testTdf94386)
{
    createSwDoc("tdf94386.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    // emulate the behavior from tdf#94386 - insert an envelope to the
    // document
    {
        SfxItemSet aSet(pWrtShell->GetView().GetCurShell()->GetPool(), svl::Items<FN_ENVELOP, FN_ENVELOP>);
        aSet.Put(SwEnvItem());
        SfxRequest aRequest(FN_ENVELOP, SfxCallMode::SYNCHRON, aSet);
        SW_MOD()->ExecOther(aRequest);
    }
    saveAndReload(u"MS Word 97"_ustr);

    // check that the first and next page use different page styles
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    xCursor->jumpToFirstPage();
    OUString firstPageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
    xCursor->jumpToLastPage();
    OUString lastPageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
    CPPUNIT_ASSERT(firstPageStyleName != lastPageStyleName);

    uno::Reference<beans::XPropertySet> xFirstPropertySet(getStyles(u"PageStyles"_ustr)->getByName(firstPageStyleName), uno::UNO_QUERY);
    awt::Size fSize;
    xFirstPropertySet->getPropertyValue(u"Size"_ustr) >>= fSize;

    uno::Reference<beans::XPropertySet> xNextPropertySet(getStyles(u"PageStyles"_ustr)->getByName(lastPageStyleName), uno::UNO_QUERY);
    awt::Size lSize;
    xNextPropertySet->getPropertyValue(u"Size"_ustr) >>= lSize;

    CPPUNIT_ASSERT((fSize.Width != lSize.Width));
    CPPUNIT_ASSERT((fSize.Height != lSize.Height));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99474)
{
    loadAndReload("tdf99474.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The bullet colour of paragraph #3 should be COL_AUTO
    auto xPara = getParagraph(3);
    uno::Reference<container::XIndexReplace> xNumRules =
        getProperty< uno::Reference<container::XIndexReplace> >(
            xPara, u"NumberingRules"_ustr);

    int numLevel = getProperty<sal_Int32>(xPara, u"NumberingLevel"_ustr);
    uno::Sequence< beans::PropertyValue > aPropertyValues;
    xNumRules->getByIndex(numLevel) >>= aPropertyValues;
    OUString charStyleName;
    for(int j = 0 ; j< aPropertyValues.getLength() ; ++j)
    {
        auto aProp = aPropertyValues[j];
        if (aProp.Name == "CharStyleName") {
            charStyleName = aProp.Value.get<OUString>();
            break;
        }
    }
    CPPUNIT_ASSERT(charStyleName.getLength());
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"CharacterStyles"_ustr)->getByName(charStyleName),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xStyle, u"CharColor"_ustr));
}

DECLARE_WW8EXPORT_TEST(testContinuousSectionsNoPageBreak, "continuous-sections.doc")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    // Continuous section breaks should not add new pages
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetPageDescCnt());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
