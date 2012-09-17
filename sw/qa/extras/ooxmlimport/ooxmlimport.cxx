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

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>


#include <vcl/svapp.hxx>

#define TWIP_TO_MM100(TWIP) ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))



using rtl::OString;
using rtl::OUString;
using rtl::OUStringBuffer;

class Test : public SwModelTestBase
{
public:
    void testN751054();
    void testN751117();
    void testN751017();
    void testN750935();
    void testN757890();
    void testFdo49940();
    void testN751077();
    void testN705956_1();
    void testN705956_2();
    void testN747461();
    void testN750255();
    void testN652364();
    void testN760764();
    void testN764005();
    void testSmartart();
    void testN764745();
    void testN766477();
    void testN758883();
    void testN766481();
    void testN766487();
    void testN693238();
    void testNumbering1();
    void testBnc773061();
    void testAllGapsWord();
    void testN775906();
    void testN775899();
    void testN777345();
    void testN777337();
    void testN778836();
    void testN778140();

    CPPUNIT_TEST_SUITE(Test);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(testN751054);
    CPPUNIT_TEST(testN751117);
    CPPUNIT_TEST(testN751017);
    CPPUNIT_TEST(testN750935);
    CPPUNIT_TEST(testN757890);
    CPPUNIT_TEST(testFdo49940);
    CPPUNIT_TEST(testN751077);
    CPPUNIT_TEST(testN705956_1);
    CPPUNIT_TEST(testN705956_2);
    CPPUNIT_TEST(testN747461);
    CPPUNIT_TEST(testN750255);
    CPPUNIT_TEST(testN652364);
    CPPUNIT_TEST(testN760764);
    CPPUNIT_TEST(testN764005);
    CPPUNIT_TEST(testSmartart);
    CPPUNIT_TEST(testN764745);
    CPPUNIT_TEST(testN766477);
    CPPUNIT_TEST(testN758883);
    CPPUNIT_TEST(testN766481);
    CPPUNIT_TEST(testN766487);
    CPPUNIT_TEST(testN693238);
    CPPUNIT_TEST(testNumbering1);
    CPPUNIT_TEST(testBnc773061);
    CPPUNIT_TEST(testAllGapsWord);
    CPPUNIT_TEST(testN775906);
    CPPUNIT_TEST(testN775899);
    CPPUNIT_TEST(testN777345);
    CPPUNIT_TEST(testN777337);
    CPPUNIT_TEST(testN778836);
    CPPUNIT_TEST(testN778140);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    /// Load an OOXML file and make the document available via mxComponent.
    void load(const OUString& rURL);
};

void Test::load(const OUString& rFilename)
{
    mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/ooxmlimport/data/") + rFilename);
}

void Test::testN751054()
{
    load("n751054.docx");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    text::TextContentAnchorType eValue;
    xPropertySet->getPropertyValue("AnchorType") >>= eValue;
    CPPUNIT_ASSERT(eValue != text::TextContentAnchorType_AS_CHARACTER);
}

void Test::testN751117()
{
    load("n751117.docx");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);

    // First shape: the end should be an arrow, should be rotated and should be flipped.
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    OUString aValue;
    xPropertySet->getPropertyValue("LineEndName") >>= aValue;
    CPPUNIT_ASSERT(aValue.indexOf("Arrow") != -1);

    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("RotateAngle") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(90 * 100), nValue);

    uno::Reference<drawing::XShape> xShape(xPropertySet, uno::UNO_QUERY);
    awt::Size aActualSize(xShape->getSize());
    CPPUNIT_ASSERT(aActualSize.Width < 0);

    // The second shape should be a line
    uno::Reference<lang::XServiceInfo> xServiceInfo(xDraws->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.drawing.LineShape"));
}

void Test::testN751017()
{
    load("n751017.docx");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xMasters(xTextFieldsSupplier->getTextFieldMasters());
    // Make sure we have a variable named foo.
    CPPUNIT_ASSERT(xMasters->hasByName("com.sun.star.text.FieldMaster.SetExpression.foo"));

    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    bool bFoundSet(false), bFoundGet(false);
    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        sal_Int16 nValue = 0;
        OUString aValue;
        if (xServiceInfo->supportsService("com.sun.star.text.TextField.SetExpression"))
        {
            bFoundSet = true;
            uno::Reference<text::XDependentTextField> xDependentTextField(xServiceInfo, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xMasterProps(xDependentTextField->getTextFieldMaster());

            // First step: did we set foo to "bar"?
            xMasterProps->getPropertyValue("Name") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("foo"), aValue);
            xPropertySet->getPropertyValue("SubType") >>= nValue;
            CPPUNIT_ASSERT_EQUAL(text::SetVariableType::STRING, nValue);
            xPropertySet->getPropertyValue("Content") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("bar"), aValue);
        }
        else if (xServiceInfo->supportsService("com.sun.star.text.TextField.GetExpression"))
        {
            // Second step: check the value of foo.
            bFoundGet = true;
            xPropertySet->getPropertyValue("Content") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("foo"), aValue);
            xPropertySet->getPropertyValue("SubType") >>= nValue;
            CPPUNIT_ASSERT_EQUAL(text::SetVariableType::STRING, nValue);
            xPropertySet->getPropertyValue("CurrentPresentation") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("bar"), aValue);
        }
    }
    CPPUNIT_ASSERT(bFoundSet);
    CPPUNIT_ASSERT(bFoundGet);
}

void Test::testN750935()
{
    load("n750935.docx");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    // Some page break types were ignores, resulting in less pages.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5), xCursor->getPage());

    /*
     * The problem was that the header and footer was not shared.
     *
     * xray ThisComponent.StyleFamilies.PageStyles.Default.FooterIsShared
     */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default"), uno::UNO_QUERY);
    sal_Bool bValue = false;
    xPropertySet->getPropertyValue("HeaderIsShared") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(sal_Bool(true), bValue);
    xPropertySet->getPropertyValue("FooterIsShared") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(sal_Bool(true), bValue);
}

void Test::testN757890()
{
    load("n757890.docx");

    // The w:pStyle token affected the text outside the textbox.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    OUString aValue;
    xPara->getPropertyValue("ParaStyleName") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"), aValue);

    // This wan't centered
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nValue;
    xFrame->getPropertyValue("HoriOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, nValue);
}

void Test::testFdo49940()
{
    load("fdo49940.docx");

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    OUString aValue;
    xPara->getPropertyValue("PageStyleName") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), aValue);
}

void Test::testN751077()
{
    load( "n751077.docx" );

/*
enum = ThisComponent.Text.createEnumeration
enum.NextElement
para = enum.NextElement
xray para.String
xray para.PageStyleName
*/
    uno::Reference<uno::XInterface> paragraph(getParagraph( 2, "TEXT1" ));
    // we want to test the paragraph is on the first page (it was put onto another page without the fix),
    // use a small trick and instead of checking the page layout, check the page style
    OUString pageStyle = getProperty< OUString >( paragraph, "PageStyleName" );
    CPPUNIT_ASSERT_EQUAL( OUString( "First Page" ), pageStyle );
}

void Test::testN705956_1()
{
    load( "n705956-1.docx" );
/*
Get the first image in the document and check it's the one image in the document.
It should be also anchored inline (as character).
image = ThisComponent.DrawPage.getByIndex(0)
graphic = image.Graphic
xray graphic.Size
xray image.AnchorType
*/
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> drawPageSupplier(textDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> drawPage = drawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1 ), drawPage->getCount());
    uno::Reference<drawing::XShape> image;
    drawPage->getByIndex(0) >>= image;
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> graphic;
    imageProperties->getPropertyValue( "Graphic" ) >>= graphic;
    uno::Reference<awt::XBitmap> bitmap(graphic, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(120), bitmap->getSize().Width );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(106), bitmap->getSize().Height );
    text::TextContentAnchorType anchorType;
    imageProperties->getPropertyValue( "AnchorType" ) >>= anchorType;
    CPPUNIT_ASSERT_EQUAL( text::TextContentAnchorType_AS_CHARACTER, anchorType );
}

void Test::testN705956_2()
{
    load( "n705956-2.docx" );
/*
<v:shapetype> must be global, reachable even from <v:shape> inside another <w:pict>
image = ThisComponent.DrawPage.getByIndex(0)
xray image.FillColor
*/
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> drawPageSupplier(textDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> drawPage = drawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShape> image;
    drawPage->getByIndex(0) >>= image;
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    sal_Int32 fillColor;
    imageProperties->getPropertyValue( "FillColor" ) >>= fillColor;
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0xc0504d ), fillColor );
}

void Test::testN747461()
{
    load( "n747461.docx" );
/*
The document contains 3 images (Red, Black, Green, in this order), with explicit
w:relativeHeight (300, 0, 225763766). Check that they are in the right ZOrder
after they are loaded.
*/
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> drawPageSupplier(textDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> drawPage = drawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShape> image1, image2, image3;
    drawPage->getByIndex( 0 ) >>= image1;
    drawPage->getByIndex( 1 ) >>= image2;
    drawPage->getByIndex( 2 ) >>= image3;
    sal_Int32 zOrder1, zOrder2, zOrder3;
    OUString descr1, descr2, descr3;
    uno::Reference<beans::XPropertySet> imageProperties1(image1, uno::UNO_QUERY);
    imageProperties1->getPropertyValue( "ZOrder" ) >>= zOrder1;
    imageProperties1->getPropertyValue( "Description" ) >>= descr1;
    uno::Reference<beans::XPropertySet> imageProperties2(image2, uno::UNO_QUERY);
    imageProperties2->getPropertyValue( "ZOrder" ) >>= zOrder2;
    imageProperties2->getPropertyValue( "Description" ) >>= descr2;
    uno::Reference<beans::XPropertySet> imageProperties3(image3, uno::UNO_QUERY);
    imageProperties3->getPropertyValue( "ZOrder" ) >>= zOrder3;
    imageProperties3->getPropertyValue( "Description" ) >>= descr3;
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), zOrder1 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1 ), zOrder2 );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 2 ), zOrder3 );
    CPPUNIT_ASSERT_EQUAL( OUString( "Black" ), descr1 );
    CPPUNIT_ASSERT_EQUAL( OUString( "Red" ), descr2 );
    CPPUNIT_ASSERT_EQUAL( OUString( "Green" ), descr3 );
}

void Test::testN750255()
{
    load( "n750255.docx" );

/*
Column break without columns on the page is a page break, so check those paragraphs
are on page 2 (page style 'Converted1') and page 3 (page style 'Converted2')
enum = ThisComponent.Text.createEnumeration
enum.nextElement
para1 = enum.nextElement
xray para1.String
xray para1.PageStyleName
para2 = enum.nextElement
xray para2.String
xray para2.PageStyleName
*/
    // get the 2nd and 3rd paragraph
    uno::Reference<uno::XInterface> paragraph1(getParagraph( 2, "one" ));
    uno::Reference<uno::XInterface> paragraph2(getParagraph( 3, "two" ));
    OUString pageStyle1 = getProperty< OUString >( paragraph1, "PageStyleName" );
    OUString pageStyle2 = getProperty< OUString >( paragraph2, "PageStyleName" );
    CPPUNIT_ASSERT_EQUAL( OUString( "Converted1" ), pageStyle1 );
    CPPUNIT_ASSERT_EQUAL( OUString( "Converted2" ), pageStyle2 );

}

void Test::testN652364()
{
    load( "n652364.docx" );

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
    // get the 2nd and 4th paragraph
    uno::Reference<uno::XInterface> paragraph1(getParagraph( 2, "text1" ));
    uno::Reference<uno::XInterface> paragraph2(getParagraph( 4, "text2" ));
    OUString pageStyle1 = getProperty< OUString >( paragraph1, "PageStyleName" );
    OUString pageStyle2 = getProperty< OUString >( paragraph2, "PageStyleName" );
    // "Standard" is the style for the first page (2nd is "Converted1").
    CPPUNIT_ASSERT_EQUAL( OUString( "Standard" ), pageStyle1 );
    CPPUNIT_ASSERT_EQUAL( OUString( "Standard" ), pageStyle2 );
}

void Test::testN760764()
{
    load("n760764.docx");

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum(xRunEnumAccess->createEnumeration());

    // Access the second run, which is a textfield
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
    float fValue;
    xRun->getPropertyValue("CharHeight") >>= fValue;
    // This used to be 11, as character properties were ignored.
    CPPUNIT_ASSERT_EQUAL(8.f, fValue);
}

void Test::testN764005()
{
    load("n764005.docx");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);

    // The picture in the header wasn't absolutely positioned and wasn't in the background.
    text::TextContentAnchorType eValue;
    xPropertySet->getPropertyValue("AnchorType") >>= eValue;
    CPPUNIT_ASSERT(eValue != text::TextContentAnchorType_AS_CHARACTER);
    sal_Bool bValue = sal_True;
    xPropertySet->getPropertyValue("Opaque") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(sal_False, bValue);
}

void Test::testSmartart()
{
    load("smartart.docx");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount()); // One groupshape in the doc

    uno::Reference<container::XIndexAccess> xGroup(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xGroup->getCount()); // 3 rectangles and an arrow in the group

    uno::Reference<beans::XPropertySet> xPropertySet(xGroup->getByIndex(1), uno::UNO_QUERY);
    sal_Int32 nValue(0);
    xPropertySet->getPropertyValue("FillColor") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4f81bd), nValue); // If fill color is right, theme import is OK

    uno::Reference<text::XTextRange> xTextRange(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Sample"), xTextRange->getString()); // Shape has text

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextRange->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPropertySet->getPropertyValue("ParaAdjust") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_CENTER), nValue); // Paragraph properties are imported
}

void Test::testN764745()
{
    load( "n764745-alignment.docx" );
/*
shape = ThisComponent.DrawPage.getByIndex(0)
xray shape.AnchorType
xray shape.AnchorPosition.X
xray ThisComponent.StyleFamilies.PageStyles.Default.Width
*/
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    // The paragraph is right-aligned and the picture does not explicitly specify position,
    // so check it's anchored as character and in the right side of the document.
    text::TextContentAnchorType anchorType;
    xPropertySet->getPropertyValue("AnchorType") >>= anchorType;
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, anchorType);
    awt::Point pos;
    xPropertySet->getPropertyValue("AnchorPosition") >>= pos;
    uno::Reference<style::XStyleFamiliesSupplier> styleFamiliesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> styleFamilies = styleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> pageStyles;
    styleFamilies->getByName("PageStyles") >>= pageStyles;
    uno::Reference<uno::XInterface> defaultStyle;
    pageStyles->getByName("Default") >>= defaultStyle;
    uno::Reference<beans::XPropertySet> styleProperties( defaultStyle, uno::UNO_QUERY );
    sal_Int32 width = 0;
    styleProperties->getPropertyValue( "Width" ) >>= width;
    CPPUNIT_ASSERT( pos.X > width / 2 );
}

void Test::testN766477()
{
    /*
     * The problem was that the checkbox was not checked.
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * oRuns = oPara.createEnumeration
     * oRun = oRuns.nextElement
     * xray oRun.Bookmark.Parameters.ElementNames(0) 'Checkbox_Checked
     */
    load("n766477.docx");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum(xRunEnumAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<text::XFormField> xFormField(xRun->getPropertyValue("Bookmark"), uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xParameters(xFormField->getParameters());
    uno::Sequence<OUString> aElementNames(xParameters->getElementNames());
    CPPUNIT_ASSERT_EQUAL(OUString("Checkbox_Checked"), aElementNames[0]);
}

void Test::testN758883()
{
    load("n758883.docx");

    /*
     * The problem was that direct formatting of the paragraph was not applied
     * to the numbering. This is easier to test using a layout dump.
     */
    OUString aHeight = parseDump("/root/page/body/txt/Special", "nHeight");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(220), aHeight.toInt32()); // It was 280

    /*
     * Next problem was that the page margin contained the width of the page border as well.
     *
     * xray ThisComponent.StyleFamilies.PageStyles.Default.LeftMargin
     */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default"), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("LeftMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(794), nValue);

    // No assert for the 3rd problem: see the comment in the test doc.

    /*
     * 4th problem: Wrap type of the textwrape was not 'through'.
     *
     * xray ThisComponent.DrawPage(0).Surround ' was 2, should be 1
     */
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    xPropertySet.set(xDraws->getByIndex(0), uno::UNO_QUERY);
    text::WrapTextMode eValue;
    xPropertySet->getPropertyValue("Surround") >>= eValue;
    CPPUNIT_ASSERT_EQUAL(eValue, text::WrapTextMode_THROUGHT);

    /*
     * 5th problem: anchor type of the second textbox was wrong.
     *
     * xray ThisComponent.DrawPage(1).AnchorType ' was 1, should be 4
     */
    xPropertySet.set(xDraws->getByIndex(1), uno::UNO_QUERY);
    text::TextContentAnchorType eAnchorType;
    xPropertySet->getPropertyValue("AnchorType") >>= eAnchorType;
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, eAnchorType);

    // 6th problem: xray ThisComponent.DrawPage(2).AnchorType ' was 2, should be 4
    xPropertySet.set(xDraws->getByIndex(2), uno::UNO_QUERY);
    xPropertySet->getPropertyValue("AnchorType") >>= eAnchorType;
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, eAnchorType);
}

void Test::testN766481()
{
    /*
     * The problem was that we had an additional paragraph before the pagebreak.
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * oPara = oParas.nextElement
     * xray oParas.hasMoreElements ' should be false
     */
    load("n766481.docx");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());
    for (int i = 0; i < 2; ++i)
        xParaEnum->nextElement();
    CPPUNIT_ASSERT_EQUAL(sal_False, xParaEnum->hasMoreElements());
}

void Test::testN766487()
{
    /*
     * The problem was that 1) the font size of the first para was too large 2) numbering had no first-line-indent.
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * oRuns = oPara.createEnumeration
     * oRun = oRuns.nextElement
     * xray oRun.CharHeight ' 12, was larger
     * oPara = oParas.nextElement
     * xray oPara.ParaFirstLineIndent ' -635, was 0
     */
    load("n766487.docx");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());

    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum(xRunEnumAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
    float fValue = 0;
    xPropertySet->getPropertyValue("CharHeight") >>= fValue;
    CPPUNIT_ASSERT_EQUAL(12.f, fValue);

    xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("ParaFirstLineIndent") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(-360)), nValue);
}

void Test::testN693238()
{
    /*
     * The problem was that a continous section break at the end of the doc caused the margins to be ignored.
     *
     * xray ThisComponent.StyleFamilies.PageStyles.Default.LeftMargin ' was 2000, should be 635
     */
    load("n693238.docx");
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default"), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("LeftMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), nValue);
}

void Test::testNumbering1()
{
    load( "numbering1.docx" );
/* <w:numPr> in the paragraph itself was overriden by <w:numpr> introduced by the paragraph's <w:pStyle>
enum = ThisComponent.Text.createEnumeration
para = enum.NextElement
xray para.NumberingStyleName
numberingstyle = ThisComponent.NumberingRules.getByIndex(6)
xray numberingstyle.name   - should match name above
numbering = numberingstyle.getByIndex(0)
xray numbering(11)  - should be 4, arabic
note that the indexes may get off as the implementation evolves, C++ code seaches in loops
*/
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference< text::XTextRange > paragraph(getParagraph( 1, "Text1." ));
    OUString numberingStyleName = getProperty< OUString >( paragraph, "NumberingStyleName" );
    uno::Reference<text::XNumberingRulesSupplier> xNumberingRulesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> numberingRules(xNumberingRulesSupplier->getNumberingRules(), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> numberingRule;
    for( int i = 0;
         i < numberingRules->getCount();
         ++i )
    {
        OUString name = getProperty< OUString >( numberingRules->getByIndex( i ), "Name" );
        if( name == numberingStyleName )
        {
            numberingRule.set( numberingRules->getByIndex( i ), uno::UNO_QUERY );
            break;
        }
    }
    CPPUNIT_ASSERT( numberingRule.is());
    uno::Sequence< beans::PropertyValue > numbering;
    numberingRule->getByIndex( 0 ) >>= numbering;
    sal_Int16 numberingType = style::NumberingType::NUMBER_NONE;
    for( int i = 0;
         i < numbering.getLength();
         ++i )
        {
        if( numbering[ i ].Name == "NumberingType" )
            {
            numbering[ i ].Value >>= numberingType;
            break;
            }
        }
    CPPUNIT_ASSERT_EQUAL( style::NumberingType::ARABIC, numberingType );
}

void Test::testBnc773061()
{
    load( "bnc773061.docx" );
    uno::Reference< text::XTextRange > paragraph = getParagraph( 1 );
    uno::Reference< text::XTextRange > normal = getRun( paragraph, 1, "Normal " );
    uno::Reference< text::XTextRange > raised = getRun( paragraph, 2, "Raised" );
    uno::Reference< text::XTextRange > lowered = getRun( paragraph, 4, "Lowered" );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), getProperty< sal_Int32 >( normal, "CharEscapement" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 50 ), getProperty< sal_Int32 >( raised, "CharEscapement" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( -25 ), getProperty< sal_Int32 >( lowered, "CharEscapement" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 100 ), getProperty< sal_Int32 >( normal, "CharEscapementHeight" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 100 ), getProperty< sal_Int32 >( raised, "CharEscapementHeight" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 100 ), getProperty< sal_Int32 >( lowered, "CharEscapementHeight" ));
}

void Test::testAllGapsWord()
{
    load("all_gaps_word.docx");
    BorderTest borderTest;
    borderTest.testTheBorders(mxComponent);
}

void Test::testN775906()
{
    /*
     * The problem was that right margin (via direct formatting) erased the left/first margin (inherited from numbering style).
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * xray oPara.ParaFirstLineIndent ' was 0
     * xray oPara.ParaLeftMargin ' was 0
     */
    load("n775906.docx");

    CPPUNIT_ASSERT_EQUAL(sal_Int32(-635), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1905), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

void Test::testN775899()
{
    /*
     * The problem was that a floating table wasn't imported as a frame, then it contained fake paragraphs.
     *
     * ThisComponent.TextFrames.Count ' was 0
     * oParas = ThisComponent.TextFrames(0).Text.createEnumeration
     * oPara = oParas.nextElement
     * oPara.supportsService("com.sun.star.text.TextTable") 'was a fake paragraph
     * oParas.hasMoreElements 'was true
     */
    load("n775899.docx");

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    uno::Reference<text::XTextFrame> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xFrame->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<lang::XServiceInfo> xServiceInfo(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_True, xServiceInfo->supportsService("com.sun.star.text.TextTable"));

    CPPUNIT_ASSERT_EQUAL(sal_False, xParaEnum->hasMoreElements());
}

void Test::testN777345()
{
    // The problem was that v:imagedata inside v:rect was ignored.
    load("n777345.docx");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<document::XEmbeddedObjectSupplier2> xSupplier(xDraws->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic = xSupplier->getReplacementGraphic();
    Graphic aGraphic(xGraphic);
    // If this changes later, feel free to update it, but make sure it's not
    // the checksum of a white/transparent placeholder rectangle.
    CPPUNIT_ASSERT_EQUAL(sal_uLong(2404338915U), aGraphic.GetChecksum());
}

void Test::testN777337()
{
    /*
     * The problem was that the top and bottom margin on the first page was only 0.1cm instead of 1.7cm.
     *
     * oFirst = ThisComponent.StyleFamilies.PageStyles.getByName("First Page")
     * xray oFirst.TopMargin
     * xray oFirst.BottomMargin
     */
    load("n777337.docx");

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("First Page"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1702), getProperty<sal_Int32>(xPropertySet, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1702), getProperty<sal_Int32>(xPropertySet, "BottomMargin"));
}

void Test::testN778836()
{
    /*
     * The problem was that the paragraph inherited margins from the numbering
     * and parent paragraph styles and the result was incorrect.
     */
    load("n778836.docx");

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(0), "ParaRightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3810), getProperty<sal_Int32>(getParagraph(0), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-635), getProperty<sal_Int32>(getParagraph(0), "ParaFirstLineIndent"));
}

void Test::testN778140()
{
    /*
     * The problem was that the paragraph top/bottom margins were incorrect due
     * to unhandled w:doNotUseHTMLParagraphAutoSpacing.
     */
    load("n778140.docx");

    CPPUNIT_ASSERT_EQUAL(sal_Int32(176), getProperty<sal_Int32>(getParagraph(0), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(353), getProperty<sal_Int32>(getParagraph(0), "ParaBottomMargin"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
