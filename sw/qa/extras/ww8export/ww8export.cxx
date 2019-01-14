/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

//#include <com/sun/star/text/XDependentTextField.hpp>

#include <IDocumentSettingAccess.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
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
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/Statement.hpp>
//#include <vcl/svapp.hxx>
#include <grfatr.hxx>
#include <pagedesc.hxx>
#include <ndgrf.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>

#include <cmdid.h>
#include <envimg.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

#include <config_features.h>

#include <bordertest.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97") {}

    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".doc");
    }

    /**
     * Validation handling
     */
    bool mustValidate(const char* filename) const override
    {
        const std::vector<const char*> aBlacklist =
        {
            // the following doc exports currently don't pass binary validation
            "tdf56321_flipImage_both.doc",
            "cjklist30.doc",
            "cjklist31.doc",
            "cjklist34.doc",
            "cjklist35.doc",
            "fdo77454.doc",
            "new-page-styles.doc",
            "tdf36117_verticalAdjustment.doc",
            "bnc636128.doc",
            "tdf92281.doc",
            "fdo59530.doc",
            "fdo56513.doc",
            "tscp.doc",
            "zoom.doc",
            "comments-nested.doc",
            "commented-table.doc",
            "zoomtype.doc",
            "n325936.doc",
            "first-header-footer.doc"
        };

        // Don't bother with non-.doc files; weed out blacklisted .doc files
        return (OString(filename).endsWith(".doc") && std::find(aBlacklist.begin(), aBlacklist.end(), filename) == aBlacklist.end());
    }
protected:

    virtual void postLoad(const char* pFilename) override
    {
        if (OString(pFilename) == "tdf94386.odt")
        {
            SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
            CPPUNIT_ASSERT(pTextDoc);
            SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

            // emulate the behavior from tdf#94386 - insert an envelope to the
            // document
            SfxItemSet aSet(pWrtShell->GetView().GetCurShell()->GetPool(), svl::Items<FN_ENVELOP, FN_ENVELOP>{});
            aSet.Put(SwEnvItem());
            SfxRequest aRequest(FN_ENVELOP, SfxCallMode::SYNCHRON, aSet);
            SW_MOD()->ExecOther(aRequest);
        }
    }
};

DECLARE_WW8EXPORT_TEST(testN757910, "n757910.doc")
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

DECLARE_WW8EXPORT_TEST(testN760294, "n760294.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    table::TableBorder aTableBorder;
    xTable->getPropertyValue("TableBorder") >>= aTableBorder;
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

DECLARE_WW8EXPORT_TEST(testN757118, "n757118.doc")
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

DECLARE_WW8EXPORT_TEST(testTdf75539_relativeWidth, "tdf75539_relativeWidth.doc")
{
    //divide everything by 10 to give a margin of error for rounding etc.
    sal_Int32 pageWidth = parseDump("/root/page[1]/body/infos/bounds", "width").toInt32()/10;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page width", sal_Int32(9354/10), pageWidth);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("100% width line", pageWidth,   parseDump("/root/page[1]/body/txt[2]/Special", "nWidth").toInt32()/10);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("50% width line", pageWidth/2,  parseDump("/root/page[1]/body/txt[4]/Special", "nWidth").toInt32()/10);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("25% width line", pageWidth/4,  parseDump("/root/page[1]/body/txt[6]/Special", "nWidth").toInt32()/10);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("10% width line", pageWidth/10, parseDump("/root/page[1]/body/txt[8]/Special", "nWidth").toInt32()/10);
}

DECLARE_WW8EXPORT_TEST(testN757905, "n757905.doc")
{
    // The problem was that the paragraph had only a single fly
    // (as-character-anchored), and the height of that was smaller than the
    // paragraph height. When in Word-compat mode, we should take the max of
    // the two, not just the height of the fly.

    OUString aHeight = parseDump("/root/page/body/txt/infos/bounds", "height");
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
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(getStyles("PageStyles")->getByName("Standard"), "TopBorder");
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(convertTwipToMm100(6 * 20)), aBorder.LineWidth);
}

DECLARE_WW8EXPORT_TEST(testN823651, "n823651.doc")
{
    // Character height was 10pt instead of 7.5pt in the header.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = getProperty< uno::Reference<text::XTextRange> >(xStyle, "HeaderTextFirst")->getText();
    CPPUNIT_ASSERT_EQUAL(7.5f, getProperty<float>(getParagraphOfText(1, xText), "CharHeight"));
}

DECLARE_WW8EXPORT_TEST(testFdo36868, "fdo36868.doc")
{
    OUString aText = parseDump("/root/page/body/txt[3]/Special[@nType='PortionType::Number']", "rText");
    // This was 1.1.
    CPPUNIT_ASSERT_EQUAL(OUString("2.1"), aText);
}

DECLARE_WW8EXPORT_TEST(testListNolevel, "list-nolevel.doc")
{
    // Similar to fdo#36868, numbering portions had wrong values.
    OUString aText = parseDump("/root/page/body/txt[1]/Special[@nType='PortionType::Number']", "rText");
    // PortionType::Number was completely missing.
    CPPUNIT_ASSERT_EQUAL(OUString("1."), aText);
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
    getCell(xTable, "A1", "Aan" SAL_NEWLINE_STRING "Recipient" SAL_NEWLINE_STRING "Recipient" SAL_NEWLINE_STRING);
    getCell(xTable, "A2", "Kopie aan" SAL_NEWLINE_STRING);
    getCell(xTable, "A3", "Datum" SAL_NEWLINE_STRING "31 juli 2008");
    getCell(xTable, "A4", "Locatie" SAL_NEWLINE_STRING "Locationr");
    getCell(xTable, "A5", "Van" SAL_NEWLINE_STRING "Sender  ");
    getCell(xTable, "A6", "Directie" SAL_NEWLINE_STRING "Department");
    getCell(xTable, "A7", "Telefoon" SAL_NEWLINE_STRING "Phone");
}

DECLARE_WW8EXPORT_TEST(testBnc821208, "bnc821208.doc")
{
    // WW8Num1z0 earned a Symbol font, turning numbers into rectangles.
    uno::Reference<beans::XPropertyState> xPropertyState(getStyles("CharacterStyles")->getByName("WW8Num1z0"), uno::UNO_QUERY);
    beans::PropertyState ePropertyState = xPropertyState->getPropertyState("CharFontName");
    // This was beans::PropertyState_DIRECT_VALUE.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE, ePropertyState);

    // Background of the numbering itself should have been the default, was yellow (0xffff00).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xPropertyState, "CharBackColor"));
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

    uno::Sequence<beans::PropertyValue> aGrabBag = getProperty< uno::Sequence<beans::PropertyValue> >(mxComponent, "InteropGrabBag");
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

DECLARE_WW8EXPORT_TEST(testMsoBrightnessContrast, "msobrightnesscontrast.doc")
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

DECLARE_WW8EXPORT_TEST(testTdf95321, "tdf95321.doc")
{
    // The problem was that there should be content in the second cell
    // but there wasn't.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Second Column"), uno::Reference<text::XTextRange>(xTable->getCellByName("B1"), uno::UNO_QUERY)->getString());
}

DECLARE_WW8EXPORT_TEST(testFdo77844, "fdo77844.doc")
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

DECLARE_WW8EXPORT_TEST(testFdp80333, "fdo80333.doc")
{
    // Despite there is no character border, border shadow was imported
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);
}

DECLARE_WW8EXPORT_TEST(testFdo81102, "fdo81102.doc")
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

DECLARE_WW8EXPORT_TEST(testBnc787942, "bnc787942.doc")
{
    // The frame ended up on the second page instead of first.

    // Ensure that the anchor is on the same page as the text "Zelva Mana"
    // (Note that this can actually be the second physical page, since the
    // page number is set to 0 which is even so LO will insert a blank page
    // before it to make it a left page)
    parseDump("/root/page[body/txt/text()='Zelva Mana']/body/txt[4]/anchored");
}

DECLARE_WW8EXPORT_TEST(testLayoutHanging, "fdo68967.doc")
{
    // This must not hang in layout
}

#if HAVE_MORE_FONTS
DECLARE_WW8EXPORT_TEST(testfdo68963, "fdo68963.doc")
{
    // The problem was that the text was not displayed.
    CPPUNIT_ASSERT ( !parseDump("/root/page/body/tab/row[2]/cell[1]/txt/Special", "rText").isEmpty() );
    CPPUNIT_ASSERT_EQUAL( OUString("Topic 1"), parseDump("/root/page/body/tab/row[2]/cell[1]/txt/Special", "rText") );
    // all crossreference bookmarks should have a target.  Shouldn't be any "Reference source not found" in the xml
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), parseDump("/root/page/body/txt[24]/Special[2]","rText").indexOf("Reference source not found"));
}
#endif

DECLARE_WW8EXPORT_TEST(testTdf99100, "tdf99100.doc")
{
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    auto xField = getProperty< uno::Reference<lang::XServiceInfo> >(getRun(getParagraphOfText(1, xHeaderText), 2), "TextField");
    // This failed: the second text portion wasn't a field.
    CPPUNIT_ASSERT(xField.is());
    CPPUNIT_ASSERT(xField->supportsService("com.sun.star.text.textfield.Chapter"));
}

DECLARE_WW8EXPORT_TEST(testTdf74328, "tdf74328.doc")
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

DECLARE_WW8EXPORT_TEST(testTdf95576, "tdf95576.doc")
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

DECLARE_WW8EXPORT_TEST(testTdf59896, "tdf59896.doc")
{
    // This was awt::FontWeight::NORMAL, i.e. the first run wasn't bold, when it should be bold
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(getParagraph(1), 1), "CharWeight"));
}

DECLARE_WW8EXPORT_TEST(testTdf102334, "tdf102334.doc")
{
    // This was false, i.e. the first run wasn't hidden, when it should have been
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getRun(getParagraph(7), 1), "CharHidden"));
}

DECLARE_WW8EXPORT_TEST(testTdf38778, "tdf38778_properties_in_run_for_field.doc")
{
    CPPUNIT_ASSERT_EQUAL(10.0f, getProperty<float>(getRun(getParagraph(1), 1), "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(OUString("Courier New"), getProperty<OUString>(getRun(getParagraph(1), 1), "CharFontName"));
}

DECLARE_WW8EXPORT_TEST(testN325936, "n325936.doc")
{
    /*
     * The problem was that the transparent background of the drawing in the
     * header was exported as non-transparent.
     *
     * xray ThisComponent.DrawPage(0).BackColorTransparency
     */

    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    sal_Int32 nValue = getProperty< sal_Int32 >(getShape(1), "BackColorTransparency");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), nValue);
}

DECLARE_WW8EXPORT_TEST(testTscp, "tscp.doc")
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, "urn:bails");
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

DECLARE_WW8EXPORT_TEST(testFdo45724, "fdo45724.odt")
{
    // The text and background color of the control shape was not correct.
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<form::validation::XValidatableFormComponent> xComponent(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(getProperty<sal_uInt32>(xComponent, "BackgroundColor")));
    CPPUNIT_ASSERT_EQUAL(OUString("xxx"), xComponent->getCurrentValue().get<OUString>());
}

DECLARE_WW8EXPORT_TEST(testFdo46020, "fdo46020.odt")
{
    // The footnote in that document wasn't exported, check that it is actually exported
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xFootnotes->getCount());
}

DECLARE_WW8EXPORT_TEST(testFirstHeaderFooter, "first-header-footer.doc")
{
    // Test import and export of a section's headerf/footerf properties.

    // The document has 6 pages. Note that we don't test if 4 or just 2 page
    // styles are created, the point is that layout should be correct.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer"),  parseDump("/root/page[1]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer"),   parseDump("/root/page[2]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header"),  parseDump("/root/page[3]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer"),  parseDump("/root/page[3]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page header2"), parseDump("/root/page[4]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer 2"), parseDump("/root/page[4]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header 2"), parseDump("/root/page[5]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer 2"), parseDump("/root/page[5]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header 2"),  parseDump("/root/page[6]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer 2"),  parseDump("/root/page[6]/footer/txt/text()"));
}

DECLARE_WW8EXPORT_TEST(testZoom, "zoom.doc")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);
}

DECLARE_WW8EXPORT_TEST(testZoomType, "zoomtype.doc")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomType") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(view::DocumentZoomType::PAGE_WIDTH), nValue);
}

DECLARE_WW8EXPORT_TEST(test56513, "fdo56513.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("This is the header of the first section"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the first page header of the second section"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("This is the non-first-page header of the second section"),  parseDump("/root/page[3]/header/txt/text()"));
}

DECLARE_WW8EXPORT_TEST(testNewPageStylesTable, "new-page-styles.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Goals and Results (Page 1)*"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Assessment (Page 2)****"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Sigma Space Performance Goals: Next Year (Page 3)*******"),  parseDump("/root/page[3]/header/txt/text()"));
}

DECLARE_WW8EXPORT_TEST(testFdo42144, "fdo42144.odt")
{
    // Footer wasn't disabled -- instead empty footer was exported.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xStyle, "FooterIsOn"));
}

DECLARE_WW8EXPORT_TEST(testCharacterBorder, "charborder.odt")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    // WW8 has just one border attribute (sprmCBrc) for text border so all side has
    // the same border
    // Border
    {
        const table::BorderLine2 aTopBorder = getProperty<table::BorderLine2>(xRun,"CharTopBorder");
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF3333,0,318,0,0,318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharRightBorder"));
    }

    // Padding (dptSpace) it is constant 0
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharTopBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xRun,"CharRightBorderDistance"));
    }

    // Shadow (fShadow)
    /* WW8 use just one bool value for shadow so the next conversions
       are made during an export-import round
       color: any -> black
       location: any -> bottom-right
       width: any -> border width */
    {
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }
}

DECLARE_WW8EXPORT_TEST(testTdf41542_imagePadding, "tdf41542_imagePadding.odt")
{
    // borderlessImage - image WITHOUT BORDERS : simulate padding with -crop
    text::GraphicCrop crop = getProperty<text::GraphicCrop>(getShape(2), "GraphicCrop");
    CPPUNIT_ASSERT( crop.Left != 0 && crop.Right != 0 );
    CPPUNIT_ASSERT( crop.Left == crop.Top && crop.Right == crop.Bottom && crop.Left == crop.Right );

    // borderedImage - image WITH BORDERS : simulate padding with -crop
    crop = getProperty<text::GraphicCrop>(getShape(3), "GraphicCrop");
    CPPUNIT_ASSERT( crop.Left != 0 && crop.Right != 0 );
    CPPUNIT_ASSERT( crop.Left == crop.Top && crop.Right == crop.Bottom && crop.Left == crop.Right );
}

DECLARE_WW8EXPORT_TEST(testFdo77454, "fdo77454.doc")
{
    {
        // check negative crops round-trip  (with border/padding of 1)
        text::GraphicCrop const crop =
            getProperty<text::GraphicCrop>(getShape(1), "GraphicCrop");
        CPPUNIT_ASSERT(abs(sal_Int32( -439) - crop.Left)   <= 2);
        CPPUNIT_ASSERT(abs(sal_Int32(-7040) - crop.Right)  <= 2);
        CPPUNIT_ASSERT(abs(sal_Int32( -220) - crop.Top)    <= 2);
        CPPUNIT_ASSERT(abs(sal_Int32(-7040) - crop.Bottom) <= 2);
    }

    {
        // check positive crops round-trip (with padding of 1)
        text::GraphicCrop const crop =
            getProperty<text::GraphicCrop>(getShape(2), "GraphicCrop");
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
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"), getProperty<OUString>(xPropertySet, "TextPortionType"));
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"), getProperty<OUString>(xPropertySet, "TextPortionType"));

    // Test initials.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("M"), getProperty<OUString>(xPropertySet, "Initials"));

    // Test commented text range which spans over more text nodes
    // Comment starts in the second paragraph
    xRunEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"), getProperty<OUString>(xPropertySet, "TextPortionType"));
    // Comment ends in the third paragraph
    xRunEnumAccess.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"), getProperty<OUString>(xPropertySet, "TextPortionType"));
}

DECLARE_WW8EXPORT_TEST(testCommentsNested, "comments-nested.doc")
{
    uno::Reference<beans::XPropertySet> xOuter(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Outer"), getProperty<OUString>(xOuter, "Content"));

    uno::Reference<beans::XPropertySet> xInner(getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 4), "TextField"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Inner"), getProperty<OUString>(xInner, "Content"));
}

DECLARE_WW8EXPORT_TEST(testBorderColoursExport, "bordercolours.odt")
{
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
#if 0
    // #if'd out because the "fine dashed" border line style for table borders
    // does not seem to save or load correctly in odt format at present
    CPPUNIT_ASSERT_EQUAL(expectedBottom.Color, tableBorder.BottomLine.Color);
#endif

    // Table cells
    uno::Reference<table::XCell> cell(
        table->getCellByName("A2"), uno::UNO_QUERY);
    border = getProperty<table::BorderLine2>(cell, "TopBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedTop, border);
    border = getProperty<table::BorderLine2>(cell, "LeftBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedLeft, border);
#if 0
    // #if'd out because the "fine dashed" border line style for table borders
    // does not seem to save or load correctly in odt format at present
    border = getProperty<table::BorderLine2>(cell, "BottomBorder");
    CPPUNIT_ASSERT_BORDER_EQUAL(expectedBottom, border);
#endif

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

DECLARE_WW8EXPORT_TEST(testRedlineExport1, "redline-export-1.odt")
{
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    //there must be no redline information on the first line before and after reloading
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(false, hasProperty(xRun, "RedlineType"));
    }
}

DECLARE_WW8EXPORT_TEST(testRedlineExport2, "redline-export-2.odt")
{
    //there must be redline information on the first portion of the third paragraph before and after reloading
    CPPUNIT_ASSERT_EQUAL(true, hasProperty(getRun(getParagraph(3), 1), "RedlineType"));
}

DECLARE_WW8EXPORT_TEST(testRedlineExport3, "redline-export-3.odt")
{
    //there must be redline information just on the para-break boundary between para one and two
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(getParagraph(1), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true, hasProperty(getRun(getParagraph(1), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true, hasProperty(getRun(getParagraph(2), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(getParagraph(2), 2), "RedlineType"));
}

DECLARE_WW8EXPORT_TEST(testCellBgColor, "cell-bg-color.odt")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xCC0000), getProperty<sal_Int32>(xTable->getCellByName("A1"), "BackColor"));
}

DECLARE_WW8EXPORT_TEST(testBnc636128, "bnc636128.doc")
{
    // Import / export of FFData.cch was missing.
    uno::Reference<text::XFormField> xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(1), 2), "Bookmark");
    uno::Reference<container::XNameContainer> xParameters = xFormField->getParameters();
    // This resulted in a container.NoSuchElementException.
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), xParameters->getByName("MaxLength").get<sal_uInt16>());
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
        CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontName"));
        CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontNameAsian"));
        CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontNameComplex"));

        uno::Reference<text::XText> xXText(getParagraph(1)->getText(), uno::UNO_QUERY);
        uno::Reference<text::XTextCursor> xCursor( xXText->createTextCursor() , uno::UNO_QUERY );

        xCursor->goRight( 5 , false );
        uno::Reference< beans::XPropertySet > xPropSet(xCursor, uno::UNO_QUERY);
        const sal_Unicode aFontname[14] = { 26032, 32048, 26126, 39636, ';' , 'P' , 'M', 'i', 'n', 'g', 'L', 'i', 'U' , 0 };
        CPPUNIT_ASSERT_EQUAL(OUString("Calibri"), getProperty<OUString>(xPropSet, "CharFontName"));
        CPPUNIT_ASSERT_EQUAL(OUString(aFontname), getProperty<OUString>(xPropSet, "CharFontNameAsian"));
        CPPUNIT_ASSERT_EQUAL(OUString("Times New Roman"), getProperty<OUString>(xPropSet, "CharFontNameComplex"));
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
    CPPUNIT_ASSERT_EQUAL(OUString("fore." SAL_NEWLINE_STRING "A1" SAL_NEWLINE_STRING "B1" SAL_NEWLINE_STRING "Afte"), xField->getAnchor()->getString());
}

DECLARE_WW8EXPORT_TEST(testTextVerticalAdjustment, "tdf36117_verticalAdjustment.doc")
{
    //Preserve the page vertical alignment setting for .doc
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    SwPageDesc &Desc = pDoc->GetPageDesc( 0 );
    drawing::TextVerticalAdjust nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_CENTER, nVA );

    Desc = pDoc->GetPageDesc( 1 );
    nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_TOP, nVA );

    Desc = pDoc->GetPageDesc( 2 );
    nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_BOTTOM, nVA );

    Desc = pTextDoc->GetDocShell()->GetDoc()->GetPageDesc( 3 );
    nVA = Desc.GetVerticalAdjustment();
    CPPUNIT_ASSERT_EQUAL( drawing::TextVerticalAdjust_BLOCK, nVA );
}

DECLARE_WW8EXPORT_TEST(testRES_MIRROR_GRAPH_BOTH, "tdf56321_flipImage_both.doc")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    for (int n = 0; ; n++)
    {
        SwNode* pNode = pDoc->GetNodes()[ n ];
        if (SwGrfNode *pGrfNode = pNode->GetGrfNode())
        {
            CPPUNIT_ASSERT_EQUAL(int(MirrorGraph::Both), static_cast<int>(pGrfNode->GetSwAttrSet().GetMirrorGrf().GetValue()));
            break;
        }
    }
}

DECLARE_WW8EXPORT_TEST(testCommentExport, "comment-export.odt")
{
    struct TextPortionInfo {
        OUString sKind;
        OUString sText;
        int nAnnotationID;
    };

    const TextPortionInfo aTextPortions[] = {
        {OUString("Annotation"), OUString("Comment on [A...A]"), 0},
        {OUString("Text"), OUString("[A xx "), 0},
        {OUString("Annotation"), OUString("Comment on [B...B]"), 1},
        {OUString("Text"), OUString("[B x "), 0},
        {OUString("Annotation"), OUString("Comment on [C..C]"), 2},
        {OUString("Text"), OUString("[C x B]"), 0},
        {OUString("AnnotationEnd"), OUString(), 1},
        {OUString("Text"), OUString(" x C]"), 0},
        {OUString("AnnotationEnd"), OUString(), 2},
        {OUString("Text"), OUString(" xx A]"), 0},
        {OUString("AnnotationEnd"), OUString(), 0},
        {OUString("Text"), OUString(" Comment on a point"), 0},
        {OUString("Annotation"), OUString("Comment on point"), 3},
        {OUString("Text"), OUString("x "), 0},
        {OUString("Annotation"), OUString("Comment on AA...BB"), 4},
        {OUString("Annotation"), OUString("Comment on AAAAAA"), 5},
        {OUString("Text"), OUString("AAAAAA"), 0},
        {OUString("AnnotationEnd"), OUString(), 5},
        {OUString("Text"), OUString(" BBBBBB"), 0},
        {OUString("AnnotationEnd"), OUString(), 4}
    };

    OUString sNames[6];

    const int nNumberOfTextPortions = SAL_N_ELEMENTS(aTextPortions);

    uno::Reference<text::XTextRange> xPara = getParagraph(1);

    for (int i = 0; i < nNumberOfTextPortions; ++i)
    {
        OUString sKind = aTextPortions[i].sKind;
        uno::Reference<text::XTextRange> xRun(getRun(xPara, i + 1), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xRun, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sKind, getProperty<OUString>(xPropertySet, "TextPortionType"));

        if (sKind == "Text")
        {
            // Check if textportion has the correct text
            CPPUNIT_ASSERT_EQUAL(aTextPortions[i].sText, xRun->getString());
        }
        else if (sKind == "Annotation")
        {
            // Check if the comment text is correct and save the name of the comment
            uno::Reference<beans::XPropertySet> xComment(getProperty< uno::Reference<beans::XPropertySet> >(xRun, "TextField"), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(aTextPortions[i].sText, getProperty<OUString>(xComment, "Content"));
            sNames[aTextPortions[i].nAnnotationID] = getProperty<OUString>(xComment, "Name");
        }
        else // if (sKind == OUString("AnnotationEnd"))
        {
            // Check if the correct Annotation ends here (by Name)
            uno::Reference<container::XNamed> xBookmark(getProperty< uno::Reference<beans::XPropertySet> >(xRun, "Bookmark"), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(sNames[aTextPortions[i].nAnnotationID], xBookmark->getName());
        }
    }
}

#if !defined(MACOSX) && !defined(_WIN32)
#if HAVE_MORE_FONTS
DECLARE_WW8EXPORT_TEST(testTableKeep, "tdf91083.odt")
{
    //emulate table "keep with next" -do not split table
    CPPUNIT_ASSERT_EQUAL( OUString("Row 1"), parseDump("/root/page[3]/body/tab[1]/row[2]/cell[1]/txt[1]") );
    CPPUNIT_ASSERT_EQUAL( OUString("Row 1"), parseDump("/root/page[6]/body/tab[1]/row[2]/cell[1]/txt[1]") );
}
#endif
#endif

DECLARE_WW8EXPORT_TEST(testTdf76349_textboxMargins, "tdf76349_textboxMargins.doc")
{
    // textboxes without borders were losing their spacing items in round-tripping
    CPPUNIT_ASSERT( 0 < parseDump("/root/page/body/txt/anchored/fly/infos/prtBounds", "left").toInt32() );

    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox background color", Color(0xD8, 0xD8, 0xD8), getProperty<Color>(xShape, "BackColor"));
}

DECLARE_WW8EXPORT_TEST(testMoveRange, "fdo66304-1.odt")
{
    //the save must survive without asserting
}

DECLARE_WW8EXPORT_TEST(testClearFramePams, "tdf46441-2.odt")
{
    //the save must survive without asserting
}

DECLARE_WW8EXPORT_TEST(testTdf94386, "tdf94386.odt")
{
    // check that the first and next page use different page styles
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    xCursor->jumpToFirstPage();
    OUString firstPageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xCursor->jumpToLastPage();
    OUString lastPageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    CPPUNIT_ASSERT(firstPageStyleName != lastPageStyleName);

    uno::Reference<beans::XPropertySet> xFirstPropertySet(getStyles("PageStyles")->getByName(firstPageStyleName), uno::UNO_QUERY);
    awt::Size fSize;
    xFirstPropertySet->getPropertyValue("Size") >>= fSize;

    uno::Reference<beans::XPropertySet> xNextPropertySet(getStyles("PageStyles")->getByName(lastPageStyleName), uno::UNO_QUERY);
    awt::Size lSize;
    xNextPropertySet->getPropertyValue("Size") >>= lSize;

    CPPUNIT_ASSERT((fSize.Width != lSize.Width) && (fSize.Height != lSize.Height));
}

DECLARE_WW8EXPORT_TEST(testTdf99474, "tdf99474.odt")
{
    // The bullet colour of paragraph #3 should be COL_AUTO
    auto xPara = getParagraph(3);
    uno::Reference<container::XIndexReplace> xNumRules(
        getProperty< uno::Reference<container::XIndexReplace> >(
            xPara, "NumberingRules"),
        uno::UNO_QUERY);
    int numLevel = getProperty<sal_Int32>(xPara, "NumberingLevel");
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
        getStyles("CharacterStyles")->getByName(charStyleName),
        uno::UNO_QUERY);
    Color charColor(getProperty<util::Color>(xStyle, "CharColor"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(COL_AUTO), sal_uInt32(charColor));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
