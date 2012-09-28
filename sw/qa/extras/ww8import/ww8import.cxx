/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "../swmodeltestbase.hxx"
#include "bordertest.hxx"

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

#include <vcl/svapp.hxx>

class Test : public SwModelTestBase
{
public:
    void testN757910();
    void testN760294();
    void testN750255();
    void testN652364();
    void testN757118();
    void testN757905();
    void testAllGapsWord();

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(testN757910);
    CPPUNIT_TEST(testN760294);
    CPPUNIT_TEST(testN750255);
    CPPUNIT_TEST(testN652364);
    CPPUNIT_TEST(testN757118);
    CPPUNIT_TEST(testN757905);
    CPPUNIT_TEST(testAllGapsWord);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    /// Load a WW8 file and make the document available via mxComponent.
    void load(const OUString& rURL);
};

void Test::load(const OUString& rFilename)
{
    mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/ww8import/data/") + rFilename);
}

void Test::testN757910()
{
    load("n757910.doc");

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

void Test::testN760294()
{
    load("n760294.doc");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    table::TableBorder aTableBorder;
    xTable->getPropertyValue("TableBorder") >>= aTableBorder;
    CPPUNIT_ASSERT_EQUAL(aTableBorder.TopLine.InnerLineWidth, aTableBorder.TopLine.OuterLineWidth);
    CPPUNIT_ASSERT_EQUAL(aTableBorder.TopLine.InnerLineWidth, aTableBorder.TopLine.LineDistance);
}

void Test::testN750255()
{
    load( "n750255.doc" );

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

void Test::testN652364()
{
    load( "n652364.doc" );

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

void Test::testN757118()
{
    load( "n757118.doc" );
/*
Two pairs of horizontal rules (one absolute width, one relative width)
have the same width (full page width, half page width).
xray ThisComponent.DrawPage.getByIndex(0).BoundRect
*/
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> drawPageSupplier(textDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> drawPage = drawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShape> rule1, rule2, rule3, rule4;
    drawPage->getByIndex(0) >>= rule1;
    drawPage->getByIndex(1) >>= rule2;
    drawPage->getByIndex(2) >>= rule3;
    drawPage->getByIndex(3) >>= rule4;
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

void Test::testN757905()
{
    // The problem was that the paragraph had only a single fly
    // (as-character-anchored), and the height of that was smallar than the
    // paragraph height. When in Word-compat mode, we should take the max of
    // the two, not just the height of the fly.

    load("n757905.doc");

    OUString aHeight = parseDump("/root/page/body/txt/infos/bounds", "height");
    CPPUNIT_ASSERT(sal_Int32(31) < aHeight.toInt32());
}

void Test::testAllGapsWord()
{
    load("all_gaps_word.doc");
    BorderTest borderTest;
    borderTest.testTheBorders(mxComponent);
}


CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
