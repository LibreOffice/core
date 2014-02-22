/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <swmodeltestbase.hxx>

#if !defined(MACOSX) && !defined(WNT)

#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

#include <vcl/svapp.hxx>

#include <bordertest.hxx>

#define TWIP_TO_MM100(TWIP) ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ww8import/data/", "MS Word 97")
    {
    }
};

#define DECLARE_WW8IMPORT_TEST(TestName, filename) DECLARE_SW_IMPORT_TEST(TestName, filename, Test)

DECLARE_WW8IMPORT_TEST(testN757910, "n757910.doc")
{
    
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("LeftBorderDistance") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(280), nValue);

    
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
    
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    
    (void) paraEnum->nextElement();
    
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
    
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    
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
    
    CPPUNIT_ASSERT( abs( boundRect1.Width - boundRect3.Width ) < 5 );
    CPPUNIT_ASSERT( abs( boundRect2.Width - boundRect4.Width ) < 5 );
}

DECLARE_WW8IMPORT_TEST(testN757905, "n757905.doc")
{
    
    
    
    

    OUString aHeight = parseDump("/root/page/body/txt/infos/bounds", "height");
    CPPUNIT_ASSERT(sal_Int32(31) < aHeight.toInt32());
}

DECLARE_WW8IMPORT_TEST(testAllGapsWord, "all_gaps_word.doc")
{
    BorderTest borderTest;
    borderTest.testTheBorders(mxComponent);
}

DECLARE_WW8IMPORT_TEST(testI120158, "i120158.doc")
{
    
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> paraEnumAccess(textDocument->getText(), uno::UNO_QUERY);
    
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    
    uno::Reference<uno::XInterface> paragraph0(paraEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> text0(paragraph0, uno::UNO_QUERY);
    OUString sFieldResult = text0->getString();
    CPPUNIT_ASSERT(sFieldResult.endsWith("AM") || sFieldResult.endsWith("PM"));
}

DECLARE_WW8IMPORT_TEST(testN816603, "n816603.doc")
{
    
    
    
    CPPUNIT_ASSERT(getPages() > 1);
}

DECLARE_WW8IMPORT_TEST(testN816593, "n816593.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    
    
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

DECLARE_WW8IMPORT_TEST(testPageBorder, "page-border.doc")
{
    
    table::BorderLine2 aBorder = getProperty<table::BorderLine2>(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "TopBorder");
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(TWIP_TO_MM100(6 * 20)), aBorder.LineWidth);
}

DECLARE_WW8IMPORT_TEST(testN823651, "n823651.doc")
{
    
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = getProperty< uno::Reference<text::XTextRange> >(xStyle, "HeaderTextFirst")->getText();
    CPPUNIT_ASSERT_EQUAL(7.5f, getProperty<float>(getParagraphOfText(1, xText), "CharHeight"));
}

DECLARE_WW8IMPORT_TEST(testFdo36868, "fdo36868.doc")
{
    OUString aText = parseDump("/root/page/body/txt[3]/Special[@nType='POR_NUMBER']", "rText");
    
    CPPUNIT_ASSERT_EQUAL(OUString("2.1"), aText);
}

DECLARE_WW8IMPORT_TEST(testListNolevel, "list-nolevel.doc")
{
    
    OUString aText = parseDump("/root/page/body/txt[1]/Special[@nType='POR_NUMBER']", "rText");
    
    CPPUNIT_ASSERT_EQUAL(OUString("1."), aText);
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
