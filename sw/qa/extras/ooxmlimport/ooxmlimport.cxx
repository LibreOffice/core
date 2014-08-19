/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config_test.h"

#include <swmodeltestbase.hxx>

#if !defined(WNT)

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XFormLayerAccess.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <vcl/svapp.hxx>
#include <unotools/fltrcfg.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <swtypes.hxx>

#include <bordertest.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlimport/data/", "Office Open XML Text")
    {
    }

    virtual void preTest(const char* filename) SAL_OVERRIDE
    {
        if (OString(filename) == "smartart.docx" || OString(filename) == "strict-smartart.docx")
            SvtFilterOptions::Get().SetSmartArt2Shape(true);
    }

    virtual void postTest(const char* filename) SAL_OVERRIDE
    {
        if (OString(filename) == "smartart.docx" || OString(filename) == "strict-smartart.docx")
            SvtFilterOptions::Get().SetSmartArt2Shape(false);
    }
};

#define DECLARE_OOXMLIMPORT_TEST(TestName, filename) DECLARE_SW_IMPORT_TEST(TestName, filename, Test)

DECLARE_OOXMLIMPORT_TEST(testN751054, "n751054.docx")
{
    text::TextContentAnchorType eValue = getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType");
    CPPUNIT_ASSERT(eValue != text::TextContentAnchorType_AS_CHARACTER);
}

DECLARE_OOXMLIMPORT_TEST(testN751117, "n751117.docx")
{
    // First shape: the end should be an arrow, should be rotated and should be flipped.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    OUString aValue;
    xPropertySet->getPropertyValue("LineEndName") >>= aValue;
    CPPUNIT_ASSERT(aValue.indexOf("Arrow") != -1);

    // Rotating & Flipping will cause the angle to change from 90 degrees to 270 degrees
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("RotateAngle") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(270 * 100), nValue);

    uno::Reference<drawing::XShape> xShape(xPropertySet, uno::UNO_QUERY);
    awt::Size aActualSize(xShape->getSize());
    CPPUNIT_ASSERT(aActualSize.Width > 0);

    // The second shape should be a line
    uno::Reference<lang::XServiceInfo> xServiceInfo(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.drawing.LineShape"));
}

DECLARE_OOXMLIMPORT_TEST(testN751017, "n751017.docx")
{
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

DECLARE_OOXMLIMPORT_TEST(testN750935, "n750935.docx")
{
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
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    bool bValue = false;
    xPropertySet->getPropertyValue("HeaderIsShared") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(true, bValue);
    xPropertySet->getPropertyValue("FooterIsShared") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(true, bValue);
}

DECLARE_OOXMLIMPORT_TEST(testN757890, "n757890.docx")
{
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

DECLARE_OOXMLIMPORT_TEST(testRhbz988516, "rhbz988516.docx")
{
    // The problem was that the list properties of the footer leaked into body
    CPPUNIT_ASSERT_EQUAL(OUString(),
            getProperty<OUString>(getParagraph(1), "NumberingStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Enclosure 3"), getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(),
            getProperty<OUString>(getParagraph(2), "NumberingStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString(),
            getProperty<OUString>(getParagraph(3), "NumberingStyleName"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo49940, "fdo49940.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    OUString aValue;
    xPara->getPropertyValue("PageStyleName") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), aValue);
}

DECLARE_OOXMLIMPORT_TEST(testN751077, "n751077.docx")
{
/*
xray ThisComponent.DrawPage(1).getByIndex(0).String
xray ThisComponent.DrawPage(1).getByIndex(0).Anchor.PageStyleName
*/
    uno::Reference<drawing::XShapes> xShapes(getShape(2), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xShapes->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("TEXT1\n"), xShape->getString());
    // we want to test the textbox is on the first page (it was put onto another page without the fix),
    // use a small trick and instead of checking the page layout, check the page style
    uno::Reference<text::XTextContent> xTextContent(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), getProperty<OUString>(xTextContent->getAnchor(), "PageStyleName"));
}

DECLARE_OOXMLIMPORT_TEST(testN705956_1, "n705956-1.docx")
{
/*
Get the first image in the document and check it's the one image in the document.
It should be also anchored inline (as character) and be inside a groupshape.
image = ThisComponent.DrawPage.getByIndex(0)
graphic = image(0).Graphic
xray graphic.Size
xray image.AnchorType
*/
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> drawPageSupplier(textDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> drawPage = drawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1 ), drawPage->getCount());
    uno::Reference<drawing::XShapes> shapes(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> image;
    shapes->getByIndex(0) >>= image;
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

DECLARE_OOXMLIMPORT_TEST(testN705956_2, "n705956-2.docx")
{
/*
<v:shapetype> must be global, reachable even from <v:shape> inside another <w:pict>
image = ThisComponent.DrawPage.getByIndex(0)
xray image.FillColor
*/
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    sal_Int32 fillColor;
    imageProperties->getPropertyValue( "FillColor" ) >>= fillColor;
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0xc0504d ), fillColor );
}

DECLARE_OOXMLIMPORT_TEST(testN747461, "n747461.docx")
{
/*
The document contains 3 images (Red, Black, Green, in this order), with explicit
w:relativeHeight (300, 0, 225763766). Check that they are in the right ZOrder
after they are loaded.
*/
    uno::Reference<drawing::XShape> image1 = getShape(1), image2 = getShape(2), image3 = getShape(3);
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

DECLARE_OOXMLIMPORT_TEST(testN750255, "n750255.docx")
{

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

DECLARE_OOXMLIMPORT_TEST(testN652364, "n652364.docx")
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
    // get the 2nd and 4th paragraph
    uno::Reference<uno::XInterface> paragraph1(getParagraph( 2, "text1" ));
    uno::Reference<uno::XInterface> paragraph2(getParagraph( 4, "text2" ));
    OUString pageStyle1 = getProperty< OUString >( paragraph1, "PageStyleName" );
    OUString pageStyle2 = getProperty< OUString >( paragraph2, "PageStyleName" );
    // "Standard" is the style for the first page (2nd is "Converted1").
    CPPUNIT_ASSERT_EQUAL( OUString( "Standard" ), pageStyle1 );
    CPPUNIT_ASSERT_EQUAL( OUString( "Standard" ), pageStyle2 );
}

DECLARE_OOXMLIMPORT_TEST(testN760764, "n760764.docx")
{
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

DECLARE_OOXMLIMPORT_TEST(testN764005, "n764005.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);

    // The picture in the header wasn't absolutely positioned and wasn't in the background.
    text::TextContentAnchorType eValue;
    xPropertySet->getPropertyValue("AnchorType") >>= eValue;
    CPPUNIT_ASSERT(eValue != text::TextContentAnchorType_AS_CHARACTER);
    bool bValue = true;
    xPropertySet->getPropertyValue("Opaque") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(false, bool(bValue));
}

DECLARE_OOXMLIMPORT_TEST(testN764745, "n764745-alignment.docx")
{
/*
shape = ThisComponent.DrawPage.getByIndex(0)
xray shape.AnchorType
xray shape.AnchorPosition.X
xray ThisComponent.StyleFamilies.PageStyles.Default.Width
*/
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
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
    pageStyles->getByName(DEFAULT_STYLE) >>= defaultStyle;
    uno::Reference<beans::XPropertySet> styleProperties( defaultStyle, uno::UNO_QUERY );
    sal_Int32 width = 0;
    styleProperties->getPropertyValue( "Width" ) >>= width;
    CPPUNIT_ASSERT( pos.X > width / 2 );
}

DECLARE_OOXMLIMPORT_TEST(testN766477, "n766477.docx")
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

DECLARE_OOXMLIMPORT_TEST(testN758883, "n758883.docx")
{
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
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("LeftMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(794), nValue);

    // No assert for the 3rd problem: see the comment in the test doc.

    /*
     * 4th problem: Wrap type of the textwrape was not 'through'.
     *
     * xray ThisComponent.DrawPage(0).Surround ' was 2, should be 1
     */
    xPropertySet.set(getShape(1), uno::UNO_QUERY);
    text::WrapTextMode eValue;
    xPropertySet->getPropertyValue("Surround") >>= eValue;
    CPPUNIT_ASSERT_EQUAL(eValue, text::WrapTextMode_THROUGHT);

    /*
     * 5th problem: anchor type of the second textbox was wrong.
     *
     * xray ThisComponent.DrawPage(1).AnchorType ' was 1, should be 4
     */
    xPropertySet.set(getShape(2), uno::UNO_QUERY);
    text::TextContentAnchorType eAnchorType;
    xPropertySet->getPropertyValue("AnchorType") >>= eAnchorType;
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, eAnchorType);

    // 6th problem: xray ThisComponent.DrawPage(2).AnchorType ' was 2, should be 4
    xPropertySet.set(getShape(3), uno::UNO_QUERY);
    xPropertySet->getPropertyValue("AnchorType") >>= eAnchorType;
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, eAnchorType);
}

DECLARE_OOXMLIMPORT_TEST(testN766481, "n766481.docx")
{
    /*
     * The problem was that we had an additional paragraph before the pagebreak.
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * oPara = oParas.nextElement
     * xray oParas.hasMoreElements ' should be false
     */
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());
    for (int i = 0; i < 2; ++i)
        xParaEnum->nextElement();
    CPPUNIT_ASSERT_EQUAL(sal_False, xParaEnum->hasMoreElements());
}

DECLARE_OOXMLIMPORT_TEST(testN766487, "n766487.docx")
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(-360)), nValue);
}

DECLARE_OOXMLIMPORT_TEST(testN693238, "n693238.docx")
{
    /*
     * The problem was that a continuous section break at the end of the doc caused the margins to be ignored.
     *
     * xray ThisComponent.StyleFamilies.PageStyles.Default.LeftMargin ' was 2000, should be 635
     */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("LeftMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), nValue);
}

DECLARE_OOXMLIMPORT_TEST(testNumbering1, "numbering1.docx")
{
/* <w:numPr> in the paragraph itself was overridden by <w:numpr> introduced by the paragraph's <w:pStyle>
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

DECLARE_OOXMLIMPORT_TEST(testBnc773061, "bnc773061.docx")
{
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

DECLARE_OOXMLIMPORT_TEST(testAllGapsWord, "all_gaps_word.docx")
{
    BorderTest borderTest;
    borderTest.testTheBorders(mxComponent, false);
}

DECLARE_OOXMLIMPORT_TEST(testN775906, "n775906.docx")
{
    /*
     * The problem was that right margin (via direct formatting) erased the left/first margin (inherited from numbering style).
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * xray oPara.ParaFirstLineIndent ' was 0
     * xray oPara.ParaLeftMargin ' was 0
     */
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-635), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1905), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

DECLARE_OOXMLIMPORT_TEST(testN775899, "n775899.docx")
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

DECLARE_OOXMLIMPORT_TEST(testN777345, "n777345.docx")
{
#if !defined(MACOSX)
    // The problem was that v:imagedata inside v:rect was ignored.
    uno::Reference<document::XEmbeddedObjectSupplier2> xSupplier(getShape(1), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic = xSupplier->getReplacementGraphic();
    Graphic aGraphic(xGraphic);
    // If this changes later, feel free to update it, but make sure it's not
    // the checksum of a white/transparent placeholder rectangle.
    CPPUNIT_ASSERT_EQUAL(sal_uLong(2529763117U), aGraphic.GetChecksum());
#endif
}

DECLARE_OOXMLIMPORT_TEST(testN777337, "n777337.docx")
{
    /*
     * The problem was that the top and bottom margin on the first page was only 0.1cm instead of 1.7cm.
     *
     * oFirst = ThisComponent.StyleFamilies.PageStyles.getByName("First Page")
     * xray oFirst.TopMargin
     * xray oFirst.BottomMargin
     */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("First Page"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1702), getProperty<sal_Int32>(xPropertySet, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1702), getProperty<sal_Int32>(xPropertySet, "BottomMargin"));
}

DECLARE_OOXMLIMPORT_TEST(testN778836, "n778836.docx")
{
    /*
     * The problem was that the paragraph inherited margins from the numbering
     * and parent paragraph styles and the result was incorrect.
     */
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(0), "ParaRightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3810), getProperty<sal_Int32>(getParagraph(0), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-635), getProperty<sal_Int32>(getParagraph(0), "ParaFirstLineIndent"));
}

DECLARE_OOXMLIMPORT_TEST(testN778140, "n778140.docx")
{
    /*
     * The problem was that the paragraph top/bottom margins were incorrect due
     * to unhandled w:doNotUseHTMLParagraphAutoSpacing.
     */
    CPPUNIT_ASSERT_EQUAL(sal_Int32(176), getProperty<sal_Int32>(getParagraph(0), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(176), getProperty<sal_Int32>(getParagraph(0), "ParaBottomMargin"));
}

DECLARE_OOXMLIMPORT_TEST(testN778828, "n778828.docx")
{
    /*
     * The problem was that a page break after a continuous section break caused
     * double page break on title page.
     */
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());
}

DECLARE_OOXMLIMPORT_TEST(testInk, "ink.docx")
{
    /*
     * The problem was that ~nothing was imported, except an empty CustomShape.
     *
     * xray ThisComponent.DrawPage(0).supportsService("com.sun.star.drawing.OpenBezierShape")
     */
    uno::Reference<lang::XServiceInfo> xServiceInfo(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.drawing.OpenBezierShape"));
}

DECLARE_OOXMLIMPORT_TEST(testN779834, "n779834.docx")
{
    // This document simply crashed the importer.
}

DECLARE_OOXMLIMPORT_TEST(testN779627, "n779627.docx")
{
    /*
     * The problem was that the table left position was based on the tableCellMar left value
     * even for nested tables, while it shouldn't.
     */
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Any aValue = xTableProperties->getPropertyValue("LeftMargin");
    sal_Int32 nLeftMargin;
    aValue >>= nLeftMargin;
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Left margin shouldn't take tableCellMar into account in nested tables",
            sal_Int32(0), nLeftMargin);

    /*
     * Another problem tested with this document is that the roundrect is
     * centered vertically and horizontally.
     */
    uno::Reference<beans::XPropertySet> xShapeProperties( getShape(1), uno::UNO_QUERY );
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShapeProperties, uno::UNO_QUERY);
    // If this goes wrong, probably the index of the shape is changed and the test should be adjusted.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.RectangleShape"), xShapeDescriptor->getShapeType());
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally", text::HoriOrientation::CENTER, nValue);
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally relatively to page", text::RelOrientation::PAGE_FRAME, nValue);
    xShapeProperties->getPropertyValue("VertOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered vertically", text::VertOrientation::CENTER, nValue);
    xShapeProperties->getPropertyValue("VertOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered vertically relatively to page", text::RelOrientation::PAGE_FRAME, nValue);
}

DECLARE_OOXMLIMPORT_TEST(testFdo74357, "fdo74357.docx")
{
    // Floating table wasn't converted to a textframe.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // This was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());

    // Bottom margin of the first paragraph was too large, causing a layout problem.
    // This was 494.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(86), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo55187, "fdo55187.docx")
{
    // 0x010d was imported as a newline.
    getParagraph(1, OUString("lup\xc4\x8dka", 7, RTL_TEXTENCODING_UTF8));
}

DECLARE_OOXMLIMPORT_TEST(testN780563, "n780563.docx")
{
    /*
     * Make sure we have the table in the fly frame created
     */
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount( ));
}

DECLARE_OOXMLIMPORT_TEST(testN780853, "n780853.docx")
{
    /*
     * The problem was that the table was not imported.
     *
     * xray ThisComponent.TextTables.Count 'was 0
     */
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

DECLARE_OOXMLIMPORT_TEST(testN780843, "n780843.docx")
{
    /*
     * The problem was that wrong footer was picked.
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * oPara = oParas.nextElement
     * oPara = oParas.nextElement
     * sStyle = oPara.PageStyleName
     * oStyle = ThisComponent.StyleFamilies.PageStyles.getByName(sStyle)
     * xray oStyle.FooterText.String ' was "hidden footer"
     */
    uno::Reference< text::XTextRange > xPara = getParagraph(3);
    OUString aStyleName = getProperty<OUString>(xPara, "PageStyleName");
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(aStyleName), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFooter = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "FooterText");
    CPPUNIT_ASSERT_EQUAL(OUString("shown footer"), xFooter->getString());
}

DECLARE_OOXMLIMPORT_TEST(testShadow, "imgshadow.docx")
{
    /*
     * The problem was that drop shadows on inline images were not being
     * imported and applied.
     */
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(2), uno::UNO_QUERY);

    bool bShadow = getProperty<bool>(xPropertySet, "Shadow");
    CPPUNIT_ASSERT(bShadow);

    sal_Int32 nShadowXDistance = getProperty<sal_Int32>(xPropertySet, "ShadowXDistance");
    CPPUNIT_ASSERT(nShadowXDistance != 0);
}

DECLARE_OOXMLIMPORT_TEST(testN782061, "n782061.docx")
{
    /*
     * The problem was that the character escapement in the second run was -58.
     */
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-9), getProperty<sal_Int32>(getRun(getParagraph(1), 2), "CharEscapement"));
}

DECLARE_OOXMLIMPORT_TEST(testN782345, "n782345.docx")
{
    /*
     * The problem was that the page break was inserted before the 3rd para, instead of before the 2nd para.
     */
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE, getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

DECLARE_OOXMLIMPORT_TEST(testN779941, "n779941.docx")
{
    /*
     * Make sure top/bottom margins of tables are set to 0 (problem was: bottom margin set to 0.35cm)
     */
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(0), uno::UNO_QUERY);
    {
        uno::Any aValue = xTableProperties->getPropertyValue("TopMargin");
        sal_Int32 nTopMargin;
        aValue >>= nTopMargin;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nTopMargin);
    }
    {
        uno::Any aValue = xTableProperties->getPropertyValue("BottomMargin");
        sal_Int32 nBottomMargin;
        aValue >>= nBottomMargin;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nBottomMargin);
    }
}

DECLARE_OOXMLIMPORT_TEST(testN783638, "n783638.docx")
{
    // The problem was that the margins of inline images were not zero.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPropertySet, "LeftMargin"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo52208, "fdo52208.docx")
{
    // The problem was that the document had 2 pages instead of 1.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}

DECLARE_OOXMLIMPORT_TEST(testN785767, "n785767.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // Check the A1 and B1 cells, the width of both of them was the default value (10000 / 9, as there were 9 cells in the row).
    CPPUNIT_ASSERT_MESSAGE("A1 must not have default width", sal_Int16(10000 / 9) != getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators")[0].Position);
    CPPUNIT_ASSERT_MESSAGE("B1 must not have default width", sal_Int16(10000 / 9) != getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[0].Position);
}

DECLARE_OOXMLIMPORT_TEST(testN773061, "n773061.docx")
{
// xray ThisComponent.TextFrames(0).LeftBorderDistance
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( getProperty< sal_Int32 >( xFrame, "LeftBorderDistance" ), sal_Int32( 0 ));
    CPPUNIT_ASSERT_EQUAL( getProperty< sal_Int32 >( xFrame, "TopBorderDistance" ), sal_Int32( 0 ));
    CPPUNIT_ASSERT_EQUAL( getProperty< sal_Int32 >( xFrame, "RightBorderDistance" ), sal_Int32( 0 ));
    CPPUNIT_ASSERT_EQUAL( getProperty< sal_Int32 >( xFrame, "BottomBorderDistance" ), sal_Int32( 0 ));
}

DECLARE_OOXMLIMPORT_TEST(testN780645, "n780645.docx")
{
    // The problem was that when the number of cells didn't match the grid, we
    // didn't take care of direct cell widths.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2135), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[0].Position); // was 1999
}

DECLARE_OOXMLIMPORT_TEST(testFineTableDash, "tableborder-finedash.docx")
{
    // The problem was that finely dashed borders on tables were unsupported
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(0), uno::UNO_QUERY);
    table::TableBorder2 aBorder;
    xTableProperties->getPropertyValue("TableBorder2") >>= aBorder;
    CPPUNIT_ASSERT_EQUAL(aBorder.RightLine.LineStyle, table::BorderLineStyle::FINE_DASHED);
}

DECLARE_OOXMLIMPORT_TEST(testN792778, "n792778.docx")
{
    /*
     * The problem was that the importer didn't handle complex groupshapes with groupshapes, textboxes and graphics inside.
     *
     * xray ThisComponent.DrawPage.Count ' 1 groupshape
     * xray ThisComponent.DrawPage(0).Count ' 2 sub-groupshapes
     * xray ThisComponent.DrawPage(0).getByIndex(0).Count ' first sub-groupshape: 1 pic
     * xray ThisComponent.DrawPage(0).getByIndex(1).Count ' second sub-groupshape: 1 pic
     * xray ThisComponent.DrawPage(0).getByIndex(0).getByIndex(0).Position.Y ' 11684, the vertical position of the shapes were also wrong
     * xray ThisComponent.DrawPage(0).getByIndex(1).getByIndex(0).Position.Y ' 11684
     */
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());

    uno::Reference<drawing::XShapes> xGroupShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xGroupShape->getCount());

    uno::Reference<drawing::XShapes> xInnerGroupShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xInnerGroupShape->getCount());

    uno::Reference<drawing::XShape> xInnerShape(xInnerGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11684), xInnerShape->getPosition().Y);

    xInnerGroupShape.set(xGroupShape->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xInnerGroupShape->getCount());

    xInnerShape.set(xInnerGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11684), xInnerShape->getPosition().Y);
}

DECLARE_OOXMLIMPORT_TEST(testGroupshapeLine, "groupshape-line.docx")
{
    /*
     * Another fallout from n#792778, this time first the lines inside a
     * groupshape wasn't imported, then the fix broke the size/position of
     * non-groupshape lines. Test both here.
     *
     * xray ThisComponent.DrawPage.Count ' 2 shapes
     * xray ThisComponent.DrawPage(0).Position 'x: 2656, y: 339
     * xray ThisComponent.DrawPage(0).Size ' width: 3270, height: 1392
     * xray ThisComponent.DrawPage(1).getByIndex(0).Position 'x: 1272, y: 2286
     * xray ThisComponent.DrawPage(1).getByIndex(0).Size 'width: 10160, height: 0
     */
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDrawPage->getCount());

    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2656), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(339), xShape->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3270), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1392), xShape->getSize().Height);

    uno::Reference<drawing::XShapes> xGroupShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    xShape.set(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1272), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2286), xShape->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10160), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape->getSize().Height);
}

DECLARE_OOXMLIMPORT_TEST(testGroupshapeChildRotation, "groupshape-child-rotation.docx")
{
    // The problem was that (due to incorrect handling of rotation inside
    // groupshapes), the first child wasn't in the top left corner of an inline
    // groupshape.
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape->getPosition().Y);

#if ! TEST_FONTS_MISSING
    xShape.set(xGroupShape->getByIndex(4), uno::UNO_QUERY);
    // This was 887, i.e. border distances were included in the height.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(686), xShape->getSize().Height);
#endif

    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroupShape->getByIndex(5), uno::UNO_QUERY);
    // This was com.sun.star.drawing.RectangleShape, all shape text in a single line.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.TextShape"), xShapeDescriptor->getShapeType());
}

DECLARE_OOXMLIMPORT_TEST(testGroupshapeSmarttag, "groupshape-smarttag.docx")
{
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    // First run of shape text was missing due to the w:smartTag wrapper around it.
    CPPUNIT_ASSERT_EQUAL(OUString("Box 2"), xShape->getString());

    // Font size of the shape text was 10.
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(xShape->getText(), "CharHeight"));
}

DECLARE_OOXMLIMPORT_TEST(testN793262, "n793262.docx")
{
    uno::Reference<container::XEnumerationAccess> xHeaderText = getProperty< uno::Reference<container::XEnumerationAccess> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "HeaderText");
    uno::Reference<container::XEnumeration> xHeaderParagraphs(xHeaderText->createEnumeration());
    xHeaderParagraphs->nextElement();
    // Font size of the last empty paragraph in the header was ignored, this was 11.
    CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xHeaderParagraphs->nextElement(), "CharHeight"));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell margins as direct formatting were ignored, this was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(76), getProperty<sal_Int32>(xTable->getCellByName("A1"), "TopBorderDistance"));
}

DECLARE_OOXMLIMPORT_TEST(testN793998, "n793998.docx")
{
    sal_Int32 nTextPortion = parseDump("/root/page/body/txt/Text[1]", "nWidth").toInt32(); // Width of the first (text) portion
    sal_Int32 nTabPortion = parseDump("/root/page/body/txt/Text[2]", "nWidth").toInt32(); // Width of the second (tab) portion
    sal_Int32 nParagraph = parseDump("/root/page/body/txt/infos/bounds", "width").toInt32(); // Width of the paragraph
    sal_Int32 nRightMargin = 3000;
    // The problem was that the tab portion didn't ignore the right margin, so text + tab width wasn't larger than body (paragraph - right margin) width.
    CPPUNIT_ASSERT(nTextPortion + nTabPortion > nParagraph - nRightMargin);
}

DECLARE_OOXMLIMPORT_TEST(testN779642, "n779642.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);

    // First problem: check that we have 2 tables, nesting caused the
    // creation of outer one to fail
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong number of imported tables", sal_Int32(2), xTables->getCount());

    // Second problem: check that the outer table is in a frame, at the bottom of the page
    uno::Reference<text::XTextTable> xTextTable(xTextTablesSupplier->getTextTables()->getByName("Table2"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xAnchor(xTextTable->getAnchor(), uno::UNO_QUERY);
    uno::Any aFrame = xAnchor->getPropertyValue("TextFrame");
    uno::Reference<beans::XPropertySet> xFrame;
    aFrame >>= xFrame;
    sal_Int16 nValue;
    xFrame->getPropertyValue("VertOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical orientation", nValue, text::VertOrientation::BOTTOM);
    xFrame->getPropertyValue("VertOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical orientation relation", nValue, text::RelOrientation::PAGE_PRINT_AREA);
}

DECLARE_OOXMLIMPORT_TEST(testTbLrHeight, "tblr-height.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // btLr text direction was imported as MIN, it should be FIX to avoid incorrectly large height in case of too much content.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX, getProperty<sal_Int16>(xTableRows->getByIndex(0), "SizeType"));
}

DECLARE_OOXMLIMPORT_TEST(testBnc865381, "bnc865381.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // Second row has a vertically merged cell, make sure size type is not FIX in that case (otherwise B2 is not readable).
    CPPUNIT_ASSERT(text::SizeType::FIX != getProperty<sal_Int16>(xTableRows->getByIndex(1), "SizeType"));
    // Explicit size of 41 mm100 was set, so the vertical text in A2 was not readable.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xTableRows->getByIndex(1), "Height"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo53985, "fdo53985.docx")
{
    // Unhandled excetion prevented import of the rest of the document.

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTables->getCount()); // Only 4 tables were imported.
}

DECLARE_OOXMLIMPORT_TEST(testFdo59638, "fdo59638.docx")
{
    // The problem was that w:lvlOverride inside w:num was ignores by dmapper.

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "BulletChar")
        {
            // Was '*', should be 'o'.
            CPPUNIT_ASSERT_EQUAL(OUString("\xEF\x82\xB7", 3, RTL_TEXTENCODING_UTF8), rProp.Value.get<OUString>());
            return;
        }
    }
    CPPUNIT_FAIL("no BulletChar property");
}

DECLARE_OOXMLIMPORT_TEST(testFdo61343, "fdo61343.docx")
{
    // The problem was that there were a groupshape in the doc, followed by an
    // OLE object, and this lead to a crash.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
}

DECLARE_OOXMLIMPORT_TEST(testToolsLineNumbering, "tools-line-numbering.docx")
{
    /*
     * Test the line numbering feature import (Tools->Line Numbering ...)
     *
     * xray ThisComponent.getLineNumberingProperties().IsOn == True
     * xray ThisComponent.getLineNumberingProperties().CountEmptyLines == True
     * xray ThisComponent.getLineNumberingProperties().NumberPosition == 0
     * xray ThisComponent.getLineNumberingProperties().NumberingType == 4
     * xray ThisComponent.getLineNumberingProperties().SeparatorInterval == 3
     */

    bool bValue = false;
    sal_Int32 nValue = -1;

    uno::Reference< text::XTextDocument > xtextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference< text::XLineNumberingProperties > xLineProperties( xtextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPropertySet = xLineProperties->getLineNumberingProperties();

    xPropertySet->getPropertyValue("IsOn") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(true, bValue);

    xPropertySet->getPropertyValue("CountEmptyLines") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(true, bValue);

    xPropertySet->getPropertyValue("NumberPosition") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nValue);

    xPropertySet->getPropertyValue("NumberingType") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), nValue);

    xPropertySet->getPropertyValue("SeparatorInterval") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), nValue);
}

DECLARE_OOXMLIMPORT_TEST(testfdo78904, "fdo78904.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    if (xIndexAccess->getCount())
    {
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(EMU_TO_MM100(0)), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
    }
}

DECLARE_OOXMLIMPORT_TEST(testFdo60922, "fdo60922.docx")
{
    // This was 0, not 100, due to wrong import of w:position w:val="0"
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(getRun(getParagraph(1), 1), "CharEscapementHeight"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo59273, "fdo59273.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Was 115596 (i.e. 10 times wider than necessary), as w:tblW was missing and the importer didn't set it.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12961), getProperty<sal_Int32>(xTextTable, "Width"));

    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // Was 9997, so the 4th column had ~zero width
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7498), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators")[2].Position);
}

DECLARE_OOXMLIMPORT_TEST(testTableWidth, "table_width.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // Relative width wasn't recognized during import.
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(xTables->getByIndex(0), "IsWidthRelative")));

    uno::Reference<text::XTextFramesSupplier> xFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(xFrames->getByIndex(0), "FrameWidthPercent"));
}

DECLARE_OOXMLIMPORT_TEST(testConditionalstylesTbllook, "conditionalstyles-tbllook.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Background was -1.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x7F7F7F), getProperty<sal_Int32>(xTable->getCellByName("A1"), "BackColor"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo63685, "fdo63685.docx")
{
    // Was 85697, i.e. original 114120 was converted to mm100 from twips, not from EMUs.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(318), getProperty<sal_Int32>(getShape(1), "TopMargin"));
}

DECLARE_OOXMLIMPORT_TEST(testN592908_Frame, "n592908-frame.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    text::WrapTextMode eValue;
    xPropertySet->getPropertyValue("Surround") >>= eValue;
    CPPUNIT_ASSERT_EQUAL(eValue, text::WrapTextMode_PARALLEL);
}

DECLARE_OOXMLIMPORT_TEST(testN592908_Picture, "n592908-picture.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    text::WrapTextMode eValue;
    xPropertySet->getPropertyValue("Surround") >>= eValue;
    CPPUNIT_ASSERT_EQUAL(eValue, text::WrapTextMode_PARALLEL);
}

DECLARE_OOXMLIMPORT_TEST(testN779630, "n779630.docx")
{
    // First shape: date picker
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.DateField")));
    CPPUNIT_ASSERT_EQUAL(OUString("date default text"), getProperty<OUString>(xPropertySet, "HelpText"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(8), getProperty<sal_Int16>(xPropertySet, "DateFormat"));
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(xPropertySet, "Dropdown")));

    // Second shape: combo box
    xControlShape.set(getShape(2), uno::UNO_QUERY);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.ComboBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("dropdown default text"), getProperty<OUString>(xPropertySet, "DefaultText"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty< uno::Sequence<OUString> >(xPropertySet, "StringItemList").getLength());
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(xPropertySet, "Dropdown")));
}

DECLARE_OOXMLIMPORT_TEST(testIndentation, "indentation.docx")
{
    uno::Reference<uno::XInterface> xParaLTRTitle(getParagraph( 1, "Title aligned"));
    uno::Reference<uno::XInterface> xParaLTRNormal(getParagraph( 2, ""));

    // this will test the text direction for paragraphs
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, getProperty<sal_Int16>( xParaLTRTitle, "WritingMode" ));
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, getProperty<sal_Int16>( xParaLTRNormal, "WritingMode" ));
}

DECLARE_OOXMLIMPORT_TEST(testPageBorderShadow, "page-border-shadow.docx")
{
    // The problem was that in w:pgBorders, child elements had a w:shadow attribute, but that was ignored.
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "ShadowFormat");
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, sal_uInt32(aShadow.Color));
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
    // w:sz="48" is in eights of a point, 1 pt is 20 twips.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(convertTwipToMm100(48/8*20)), aShadow.ShadowWidth);
}

DECLARE_OOXMLIMPORT_TEST(testN816593, "n816593.docx")
{
    // Two consecutive <w:tbl> without any paragraph in between, but with different tblpPr. In this
    // case we need to have 2 different tables instead of 1
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
}

DECLARE_OOXMLIMPORT_TEST(testN820509, "n820509.docx")
{
    // Design mode was enabled.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XFormLayerAccess> xFormLayerAccess(xModel->getCurrentController(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, bool(xFormLayerAccess->isFormDesignMode()));

    // M.d.yyyy date format was unhandled.
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(8), getProperty<sal_Int16>(xPropertySet, "DateFormat"));
}

DECLARE_OOXMLIMPORT_TEST(testN820788, "n820788.docx")
{
    // The problem was that AutoSize was not enabled for the text frame.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    // This was text::SizeType::FIX.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::MIN, getProperty<sal_Int16>(xFrame, "SizeType"));
}

DECLARE_OOXMLIMPORT_TEST(testN820504, "n820504.docx")
{
    uno::Reference<style::XStyleFamiliesSupplier> xFamiliesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xFamiliesAccess(xFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStylesAccess(xFamiliesAccess->getByName("ParagraphStyles"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStylesAccess->getByName("Default Style"), uno::UNO_QUERY);
    // The problem was that the CharColor was set to AUTO (-1) even if we have some default char color set
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4040635), getProperty<sal_Int32>(xStyle, "CharColor"));

    // Also, the groupshape was anchored at-page instead of at-character
    // (that's incorrect as Word only supports at-character and as-character).
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_OOXMLIMPORT_TEST(testN830205, "n830205.docx")
{
    // Previously import just crashed (due to infinite recursion).
    getParagraph(1, "XXX");
}

DECLARE_OOXMLIMPORT_TEST(testFdo43641, "fdo43641.docx")
{
    uno::Reference<container::XIndexAccess> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xLine(xGroupShape->getByIndex(1), uno::UNO_QUERY);
    // This was 2200, not 2579 in mm100, i.e. the size of the line shape was incorrect.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(EMU_TO_MM100(928694)), xLine->getSize().Width);
}

DECLARE_OOXMLIMPORT_TEST(testTableAutoColumnFixedSize, "table-auto-column-fixed-size.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Width was not recognized during import when table size was 'auto'
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(3996)), getProperty<sal_Int32>(xTextTable, "Width"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo46361, "fdo46361.docx")
{
    uno::Reference<container::XIndexAccess> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    // This was CENTER.
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP, getProperty<drawing::TextVerticalAdjust>(xShape, "TextVerticalAdjust"));
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    // This was LEFT.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xParagraph, "ParaAdjust")));
    // This was black, not green.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x008000), getProperty<sal_Int32>(getRun(xParagraph, 1), "CharColor"));
    // \n char was missing due to unhandled w:br.
    CPPUNIT_ASSERT_EQUAL(OUString("text\ntext"), uno::Reference<text::XTextRange>(xGroupShape->getByIndex(1), uno::UNO_QUERY)->getString());
    // \n chars were missing, due to unhandled multiple w:p tags.
    CPPUNIT_ASSERT_EQUAL(OUString("text\ntext\n"), uno::Reference<text::XTextRange>(xGroupShape->getByIndex(2), uno::UNO_QUERY)->getString());
}

DECLARE_OOXMLIMPORT_TEST(testFdo65632, "fdo65632.docx")
{
    // The problem was that the footnote text had fake redline: only the body
    // text has redline in fact.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    //uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), getProperty<OUString>(getRun(getParagraphOfText(1, xText), 1), "TextPortionType"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo66474, "fdo66474.docx")
{
    // The table width was too small, so the text in the second cell was unreadable: this was 1397.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10492), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_OOXMLIMPORT_TEST(testGroupshapeRotation, "groupshape-rotation.docx")
{
    // Rotation on groupshapes wasn't handled at all by the VML importer.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(315 * 100), getProperty<sal_Int32>(getShape(1), "RotateAngle"));
}

DECLARE_OOXMLIMPORT_TEST(testBnc780044Spacing, "bnc780044_spacing.docx")
{
    // The document has global w:spacing in styles.xml , and local w:spacing in w:pPr, which however
    // only applied to text runs, not to as-character pictures. So the picture made the line higher.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}

DECLARE_OOXMLIMPORT_TEST(testTableAutoNested, "table-auto-nested.docx")
{
    // This was 176, when compat option is not enabled, the auto paragraph bottom margin value was incorrect.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(494), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // This was 115596, i.e. the width of the outer table was too large.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(23051), getProperty<sal_Int32>(xTables->getByIndex(1), "Width"));
}

DECLARE_OOXMLIMPORT_TEST(testTableStyleParprop, "table-style-parprop.docx")
{
    // The problem was that w:spacing's w:after=0 (a paragraph property) wasn't imported from table style.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 353, the document default, i.e. paragraph property from table style had no effect.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaBottomMargin"));
}

DECLARE_OOXMLIMPORT_TEST(testTablePagebreak, "table-pagebreak.docx")
{
    // Page break inside table: should be ignored (was style::BreakType_PAGE_BEFORE before).
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE, getProperty<style::BreakType>(getParagraphOrTable(2), "BreakType"));

    // This one is outside the table: should not be ignored.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE, getProperty<style::BreakType>(getParagraph(3), "BreakType"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo68607, "fdo68607.docx")
{
    // Bugdoc was 8 pages in Word, 1 in Writer due to pointlessly wrapping the
    // table in a frame. Exact layout may depend on fonts available, etc. --
    // but at least make sure that our table spans over multiple pages now.
    CPPUNIT_ASSERT(getPages() > 1);
}

DECLARE_OOXMLIMPORT_TEST(testVmlTextVerticalAdjust, "vml-text-vertical-adjust.docx")
{
    uno::Reference<drawing::XShapes> xOuterGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xInnerGroupShape(xOuterGroupShape->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xInnerGroupShape->getByIndex(0), uno::UNO_QUERY);
    // Was CENTER.
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP, getProperty<drawing::TextVerticalAdjust>(xShape, "TextVerticalAdjust"));
}

DECLARE_OOXMLIMPORT_TEST(testGroupshapeSdt, "groupshape-sdt.docx")
{
    // All problems here are due to the groupshape: we have a drawinglayer rectangle, not a writer textframe.
    uno::Reference<drawing::XShapes> xOuterGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xInnerGroupShape(xOuterGroupShape->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xInnerGroupShape->getByIndex(0), uno::UNO_QUERY);
    // Border distances were not implemented, this was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1905), getProperty<sal_Int32>(xShape, "TextUpperDistance"));
    // Sdt field result wasn't imported, this was "".
    CPPUNIT_ASSERT_EQUAL(OUString("placeholder text"), xShape->getString());
    // w:spacing was ignored in oox, this was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), getProperty<sal_Int32>(getRun(getParagraphOfText(1, xShape->getText()), 1), "CharKerning"));
}

DECLARE_OOXMLIMPORT_TEST(testDefaultSectBreakCols, "default-sect-break-cols.docx")
{
    // First problem: the first two paragraphs did not have their own text section, so the whole document had two columns.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(1, "First."), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    // Second problem: the page style had two columns, while it shouldn't have any.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xPageStyle, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
    // Check for the Column Separator value.It should be FALSE as the document does not contain separator line.
    bool bValue = getProperty< bool >(xTextColumns, "SeparatorLineIsOn");
    CPPUNIT_ASSERT(!bValue) ;
}

DECLARE_OOXMLIMPORT_TEST(testFdo69636, "fdo69636.docx")
{
    // The problem was that the mso-layout-flow-alt:bottom-to-top VML shape property wasn't handled for sw text frames.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(900), getProperty<sal_Int32>(getRun(getParagraphOfText(1, xFrame->getText()), 1), "CharRotation"));
}

DECLARE_OOXMLIMPORT_TEST(testChartProp, "chart-prop.docx")
{
    // The problem was that chart was not getting parsed in writer module.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());

    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15240), getProperty<sal_Int32>(xPropertySet, "Width"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8890), getProperty<sal_Int32>(xPropertySet, "Height"));
}

void lcl_countTextFrames(com::sun::star::uno::Reference< lang::XComponent >& xComponent,
   sal_Int32 nExpected )
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(xComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( nExpected, xIndexAccess->getCount());
}

DECLARE_OOXMLIMPORT_TEST(testBnc779620, "bnc779620.docx")
{
    // The problem was that the floating table was imported as a non-floating one.
    lcl_countTextFrames( mxComponent, 1 );
}

DECLARE_OOXMLIMPORT_TEST(testfdo76583, "fdo76583.docx")
{
    // The problem was that the floating table was imported as a non-floating one.
    // floating tables are imported as text frames, therefore the document should
    // exactly 1 text frame.
    lcl_countTextFrames( mxComponent, 1 );
}

DECLARE_OOXMLIMPORT_TEST(testFdo43093, "fdo43093.docx")
{
    // The problem was that the direction and alignment are not correct for RTL paragraphs.
    uno::Reference<uno::XInterface> xParaRtlRight(getParagraph( 1, "Right and RTL in M$"));
    sal_Int32 nRtlRight = getProperty< sal_Int32 >( xParaRtlRight, "ParaAdjust" );
    sal_Int16 nRRDir  = getProperty< sal_Int32 >( xParaRtlRight, "WritingMode" );

    uno::Reference<uno::XInterface> xParaRtlLeft(getParagraph( 2, "Left and RTL in M$"));
    sal_Int32 nRtlLeft = getProperty< sal_Int32 >( xParaRtlLeft, "ParaAdjust" );
    sal_Int16 nRLDir  = getProperty< sal_Int32 >( xParaRtlLeft, "WritingMode" );

    uno::Reference<uno::XInterface> xParaLtrRight(getParagraph( 3, "Right and LTR in M$"));
    sal_Int32 nLtrRight = getProperty< sal_Int32 >( xParaLtrRight, "ParaAdjust" );
    sal_Int16 nLRDir  = getProperty< sal_Int32 >( xParaLtrRight, "WritingMode" );

    uno::Reference<uno::XInterface> xParaLtrLeft(getParagraph( 4, "Left and LTR in M$"));
    sal_Int32 nLtrLeft = getProperty< sal_Int32 >( xParaLtrLeft, "ParaAdjust" );
    sal_Int16 nLLDir  = getProperty< sal_Int32 >( xParaLtrLeft, "WritingMode" );

    // this will test the both the text direction and alignment for each paragraph
    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_RIGHT), nRtlRight);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, nRRDir);

    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_LEFT), nRtlLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, nRLDir);

    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_RIGHT), nLtrRight);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLRDir);

    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_LEFT), nLtrLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLLDir);
}

DECLARE_OOXMLIMPORT_TEST(testSmartart, "smartart.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount()); // One groupshape in the doc

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
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

DECLARE_OOXMLIMPORT_TEST(testMultiColumnSeparator, "multi-column-separator-with-line.docx")
{
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(1, "First data."), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());
    // Check for the Column Separator value.It should be TRUE as the document contains separator line.
    bool  bValue = getProperty< bool >(xTextColumns, "SeparatorLineIsOn");
    CPPUNIT_ASSERT(bValue);
}

DECLARE_OOXMLIMPORT_TEST(testFdo69548, "fdo69548.docx")
{
    // The problem was that the last space in target URL was removed
    CPPUNIT_ASSERT_EQUAL(OUString("#this is a bookmark"), getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkURL"));
}

DECLARE_OOXMLIMPORT_TEST(testWpsOnly, "wps-only.docx")
{
    // Document has wp:anchor, not wp:inline, so handle it accordingly.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    text::TextContentAnchorType eValue = getProperty<text::TextContentAnchorType>(xShape, "AnchorType");
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_PARAGRAPH, eValue);

    // Check position, it was 0. This is a shape, so use getPosition(), not a property.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(EMU_TO_MM100(671830)), xShape->getPosition().X);

    // Left margin was 0, instead of 114300 EMU's.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xShape, "LeftMargin"));
    // Wrap type was PARALLEL.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT, getProperty<text::WrapTextMode>(xShape, "Surround"));

    // This should be in front of text.
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(xShape, "Opaque")));
    // And this should be behind the document.
    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<sal_Bool>(getShape(2), "Opaque")));
}

DECLARE_OOXMLIMPORT_TEST(lineWpsOnly, "line-wps-only.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Check position, it was -7223 as it was set after the CustomShapeGeometry property.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(208), xShape->getPosition().X);
}

DECLARE_OOXMLIMPORT_TEST(lineRotation, "line-rotation.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(3);
    // This was 5096: the line was shifted towards the bottom, so the end of
    // the 3 different lines wasn't at the same point.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4808), xShape->getPosition().Y);
}

DECLARE_OOXMLIMPORT_TEST(textboxWpsOnly, "textbox-wps-only.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello world!"), xFrame->getString());
    // Position wasn't horizontally centered.
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(xFrame, "HoriOrient"));

    // Position was the default (hori center, vert top) for the textbox.
    xFrame.set(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2173), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2805), getProperty<sal_Int32>(xFrame, "VertOrientPosition"));
}

DECLARE_OOXMLIMPORT_TEST(testWpgOnly, "wpg-only.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Check position, it was nearly 0. This is a shape, so use getPosition(), not a property.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(EMU_TO_MM100(548005)), xShape->getPosition().X);
}

DECLARE_OOXMLIMPORT_TEST(testWpgNested, "wpg-nested.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was a com.sun.star.drawing.CustomShape, due to lack of handling of groupshapes inside groupshapes.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GroupShape"), xShapeDescriptor->getShapeType());
    // This was text::RelOrientation::PAGE_FRAME, effectively placing the group shape on the left side of the page instead of the right one.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_RIGHT, getProperty<sal_Int16>(xGroup, "HoriOrientRelation"));
}

DECLARE_OOXMLIMPORT_TEST(textboxWpgOnly, "textbox-wpg-only.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // The relativeFrom attribute was ignored for groupshapes, i.e. these were text::RelOrientation::FRAME.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, "VertOrientRelation"));
    // Make sure the shape is not in the background, as we have behindDoc="0" in the doc.
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(xShape, "Opaque")));

    // The 3 paragraphs on the rectangles inside the groupshape ended up in the
    // body text, make sure we don't have multiple paragraphs there anymore.
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs()); // was 4

    // Character escapement was enabled by default, this was 58.
    uno::Reference<container::XIndexAccess> xGroup(xShape, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY)->getText();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(getRun(getParagraphOfText(1, xText), 1), "CharEscapementHeight"));
}

DECLARE_OOXMLIMPORT_TEST(testMceWpg, "mce-wpg.docx")
{
    // Make sure that we read the primary branch, if wpg is requested as a feature.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY)->getText();
    // This was VML1.
    getParagraphOfText(1, xText, "DML1");
}

DECLARE_OOXMLIMPORT_TEST(testMceNested, "mce-nested.docx")
{
    // Vertical position of the textbox was incorrect due to incorrect nested mce handling.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    // positionV's posOffset from the bugdoc, was 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(EMU_TO_MM100(2514600)), getProperty<sal_Int32>(xFrame, "VertOrientPosition"));
    // This was -1 (default), make sure the background color is set.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4f81bd), getProperty<sal_Int32>(xFrame, "BackColor"));

    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(getShape(2), uno::UNO_QUERY);
    // This was a com.sun.star.drawing.CustomShape, due to incorrect handling of wpg elements after a wps textbox.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GroupShape"), xShapeDescriptor->getShapeType());

    // Now check the top right textbox.
    uno::Reference<container::XIndexAccess> xGroup(getShape(2), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText, "[Year]");
    CPPUNIT_ASSERT_EQUAL(48.f, getProperty<float>(getRun(xParagraph, 1), "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xffffff), getProperty<sal_Int32>(getRun(xParagraph, 1), "CharColor"));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(xParagraph, 1), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM, getProperty<drawing::TextVerticalAdjust>(xGroup->getByIndex(1), "TextVerticalAdjust"));
}

DECLARE_OOXMLIMPORT_TEST(testMissingPath, "missing-path.docx")
{
    comphelper::SequenceAsHashMap aCustomShapeGeometry(getProperty<beans::PropertyValues>(getShape(1), "CustomShapeGeometry"));
    comphelper::SequenceAsHashMap aPath(aCustomShapeGeometry["Path"].get<beans::PropertyValues>());
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates = aPath["Coordinates"].get< uno::Sequence<drawing::EnhancedCustomShapeParameterPair> >();
    // This was 0, the coordinate list was empty.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(19), aCoordinates.getLength());
}

DECLARE_OOXMLIMPORT_TEST(testFdo70457, "fdo70457.docx")
{
    // The document contains a rotated bitmap
    // It must be imported as a XShape object with the proper rotation value

    // Check: there is one shape in the doc
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());

    // Check: the angle of the shape is 45º
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4500), getProperty<sal_Int32>(getShape(1), "RotateAngle"));
}

DECLARE_OOXMLIMPORT_TEST(testLOCrash,"file_crash.docx")
{
    //The problem was libreoffice crash while opening the file.
    getParagraph(1,"Contents");
}

DECLARE_OOXMLIMPORT_TEST(testFdo72560, "fdo72560.docx")
{
    // The problem was libreoffice confuse when there RTL default style for paragraph
    uno::Reference<uno::XInterface> xParaLeftRTL(getParagraph( 1, "RTL LEFT"));
    uno::Reference<uno::XInterface> xParaRightLTR(getParagraph( 2, "LTR RIGHT"));

    // this will test the text direction and alignment for paragraphs
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>( xParaLeftRTL, "WritingMode" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_LEFT), getProperty< sal_Int32 >( xParaLeftRTL, "ParaAdjust" ));

    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, getProperty<sal_Int16>( xParaRightLTR, "WritingMode" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_RIGHT), getProperty< sal_Int32 >( xParaRightLTR, "ParaAdjust" ));
}

DECLARE_OOXMLIMPORT_TEST(testRPrChangeClosed, "rprchange_closed.docx")
{
    // Redline defined by rPrChanged wasn't removed.
    // First paragraph has an rPrChange element, make sure it doesn't appear in the second paragraph.
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(getParagraph(2), 1), "RedlineType"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo65090, "fdo65090.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // The first row had two cells, instead of a single horizontally merged one.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators").getLength());
}

DECLARE_OOXMLIMPORT_TEST(testFdo69649, "fdo69649.docx")
{
    // The DOCX containing the Table of Contents was not imported with correct page nos
    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes(xIndexSupplier->getDocumentIndexes( ), uno::UNO_QUERY);
    uno::Reference<text::XDocumentIndex> xTOCIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xTOCIndex->getAnchor(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xTextRange->getText( ), uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xTextCursor(xText->createTextCursor( ), uno::UNO_QUERY);
    xTextCursor->gotoRange(xTextRange->getStart(),false);
    xTextCursor->gotoRange(xTextRange->getEnd(),true);
    OUString xTocString(xTextCursor->getString());
    xTocString = xTocString.copy(256);
    CPPUNIT_ASSERT(xTocString.startsWithIgnoreAsciiCase( "Heading 15.1:\t15" ) );
}

DECLARE_OOXMLIMPORT_TEST(testFdo73389,"fdo73389.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // This was 9340, i.e. the width of the inner table was too large.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2842), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_OOXMLIMPORT_TEST(testDMLGroupshapeSdt, "dml-groupshape-sdt.docx")
{
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    // The text in the groupshape was missing due to the w:sdt and w:sdtContent wrapper around it.
    CPPUNIT_ASSERT_EQUAL(OUString("sdt and sdtContent inside groupshape"), uno::Reference<text::XTextRange>(xGroupShape->getByIndex(1), uno::UNO_QUERY)->getString());
}

DECLARE_OOXMLIMPORT_TEST(testDmlCharheightDefault, "dml-charheight-default.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was 16: the first run of the second para incorrectly inherited the char height of the first para.
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(getRun(getParagraphOfText(2, xShape->getText()), 1), "CharHeight"));
}

DECLARE_OOXMLIMPORT_TEST(testGroupshapeRelsize, "groupshape-relsize.docx")
{
    // This was 43760, i.e. the height of the groupshape was larger than the page height, which is obviously incorrect.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(EMU_TO_MM100(9142730)), getShape(1)->getSize().Height);
}

DECLARE_OOXMLIMPORT_TEST(testOleAnchor, "ole-anchor.docx")
{
    // This was AS_CHARACTER, even if the VML style explicitly contains "position:absolute".
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
    // This was DYNAMIC, even if the default is THROUGHT and there is no w10:wrap element in the bugdoc.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT, getProperty<text::WrapTextMode>(getShape(1), "Surround"));
}

DECLARE_OOXMLIMPORT_TEST(testDMLGroupShapeCapitalization, "dml-groupshape-capitalization.docx")
{
    // Capitalization inside a group shape was not imported
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();
    // 2nd line is written with uppercase letters
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::UPPERCASE, getProperty<sal_Int16>(getRun(getParagraphOfText(2, xText), 1), "CharCaseMap"));
    // 3rd line has no capitalization
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::NONE, getProperty<sal_Int16>(getRun(getParagraphOfText(3, xText), 1), "CharCaseMap"));
    // 4th line has written with small capitals
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::SMALLCAPS, getProperty<sal_Int16>(getRun(getParagraphOfText(4, xText), 1), "CharCaseMap"));
    // 5th line has no capitalization
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::NONE, getProperty<sal_Int16>(getRun(getParagraphOfText(5, xText), 1), "CharCaseMap"));
}

DECLARE_OOXMLIMPORT_TEST(testDMLGroupShapeParaAdjust, "dml-groupshape-paraadjust.docx")
{
    // Paragraph adjustment inside a group shape was not imported
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();
    // 2nd line is adjusted to the right
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_RIGHT), getProperty<sal_Int16>(getRun(getParagraphOfText(2, xText), 1), "ParaAdjust"));
    // 3rd line has no adjustment
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT), getProperty<sal_Int16>(getRun(getParagraphOfText(3, xText), 1), "ParaAdjust"));
    // 4th line is adjusted to center
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER), getProperty<sal_Int16>(getRun(getParagraphOfText(4, xText), 1), "ParaAdjust"));
    // 5th line has no adjustment
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT), getProperty<sal_Int16>(getRun(getParagraphOfText(5, xText), 1), "ParaAdjust"));
    // 6th line is justified
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_BLOCK), getProperty<sal_Int16>(getRun(getParagraphOfText(6, xText), 1), "ParaAdjust"));
    // 7th line has no adjustment
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_LEFT), getProperty<sal_Int16>(getRun(getParagraphOfText(7, xText), 1), "ParaAdjust"));
}

DECLARE_OOXMLIMPORT_TEST(testPictureWithSchemeColor, "picture-with-schemecolor.docx")
{
    // At the start of the document, a picture which has a color specified with a color scheme, lost
    // it's color during import.
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, "Graphic");
    uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(341), xBitmap->getSize().Width );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(181), xBitmap->getSize().Height );

    // Check some bits of the bitmap which change when color is imported.
    const uno::Sequence< sal_Int8 > aDIB = xBitmap->getDIB();
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int8>(-91), aDIB[54] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int8>(110), aDIB[55] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int8>(49), aDIB[56] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int8>(-36), aDIB[96] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int8>(-57), aDIB[97] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int8>(-80), aDIB[98] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int8>(-91), aDIB[135] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int8>(110), aDIB[136] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int8>(49), aDIB[137] );
}

DECLARE_OOXMLIMPORT_TEST(testFdo69656, "Table_cell_auto_width_fdo69656.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8154), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_OOXMLIMPORT_TEST(testFloatingTablesAnchor, "floating-tables-anchor.docx")
{
    // Problem was one of the two text frames was anchored to the other text frame
    // Both frames should be anchored to the paragraph with the text "Anchor point"
    uno::Reference<text::XTextContent> xTextContent(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());

    xTextContent.set(getShape(2), uno::UNO_QUERY);
    xRange.set(xTextContent->getAnchor(), uno::UNO_QUERY);
    xText.set(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());
}

DECLARE_OOXMLIMPORT_TEST(testAnnotationFormatting, "annotation-formatting.docx")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(2), 2), "TextField");
    uno::Reference<text::XText> xText = getProperty< uno::Reference<text::XText> >(xTextField, "TextRange");
    // Make sure we test the right annotation.
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText, "days");
    // Formatting was lost: the second text portion was NONE, not SINGLE.
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::SINGLE, getProperty<sal_Int16>(getRun(xParagraph, 1), "CharUnderline"));
}

DECLARE_OOXMLIMPORT_TEST(testDMLGroupShapeRunFonts, "dml-groupshape-runfonts.docx")
{
    // Fonts defined by w:rFonts was not imported and so the font specified by a:fontRef was used.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText),1);
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Arial Unicode MS"), getProperty<OUString>(xRun, "CharFontNameComplex"));
    CPPUNIT_ASSERT_EQUAL(OUString("MS Mincho"), getProperty<OUString>(xRun, "CharFontNameAsian"));
}

DECLARE_OOXMLIMPORT_TEST(testStrict, "strict.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    // This was only 127, pt suffix was ignored, so this got parsed as twips instead of points.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(72 * 20)), getProperty<sal_Int32>(xPageStyle, "BottomMargin"));
    // This was only 1397, same issue
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(792 * 20)), getProperty<sal_Int32>(xPageStyle, "Height"));
    // Text was missing, due to not handling the strict namespaces.
    getParagraph(1, "Hello world!");

    // Header in the document caused a crash on import.
    uno::Reference<text::XText> xHeaderText(xPageStyle->getPropertyValue("HeaderText"), uno::UNO_QUERY);
    getParagraphOfText(1, xHeaderText, "This is a header.");

    // Picture was missing.
    uno::Reference<lang::XServiceInfo> xServiceInfo(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextGraphicObject"));

    // SmartArt was missing.
    xServiceInfo.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"));

    // Chart was missing.
    xServiceInfo.set(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextEmbeddedObject"));

    // Math was missing.
    xServiceInfo.set(getShape(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextEmbeddedObject"));
}

DECLARE_OOXMLIMPORT_TEST(testStrictLockedcanvas, "strict-lockedcanvas.docx")
{
    // locked canvas shape was missing.
    getShape(1);
}

DECLARE_OOXMLIMPORT_TEST(testSmartartStrict, "strict-smartart.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    // This was 0, SmartArt was visually missing.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xGroup->getCount()); // 3 ellipses + 3 arrows
}

DECLARE_OOXMLIMPORT_TEST(testLibreOfficeHang, "frame-wrap-auto.docx")
{
    // fdo#72775
    // This was text::WrapTextMode_NONE.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_DYNAMIC, getProperty<text::WrapTextMode>(getShape(1), "Surround"));
}

DECLARE_OOXMLIMPORT_TEST(testI124106, "i124106.docx")
{
    // This was 2.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLIMPORT_TEST(testLargeTwips, "large-twips.docx" )
{
    // cp#1000043: MSO seems to ignore large twips values, we didn't, which resulted in different
    // layout of broken documents (text not visible in this specific document).
    OUString width = parseDump( "/root/page/body/tab/row[1]/cell[1]/txt/infos/bounds", "width" );
    CPPUNIT_ASSERT( width.toInt32() > 0 );
}

DECLARE_OOXMLIMPORT_TEST(testNegativeCellMarginTwips, "negative-cell-margin-twips.docx" )
{
    // Sligtly related to cp#1000043, the twips value was negative, which wrapped around somewhere,
    // while MSO seems to ignore that as well.
    OUString width = parseDump( "/root/page/body/tab/row[1]/cell[1]/txt/infos/bounds", "width" );
    CPPUNIT_ASSERT( width.toInt32() > 0 );
}

DECLARE_OOXMLIMPORT_TEST(testFdo38414, "fdo38414.docx" )
{
    // The cells in the last (4th) column were merged properly and so the result didn't have the same height.
    // (Since w:gridBefore is worked around by faking another cell in the row, so column count is thus 5
    // instead of 4, therefore compare height of cells 4 and 5 rather than 3 and 4.)
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableColumns> xTableColumns(xTextTable->getColumns(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 5 ), xTableColumns->getCount());
    OUString height3 = parseDump("/root/page/body/tab/row[1]/cell[4]/infos/bounds", "height" );
    OUString height4 = parseDump("/root/page/body/tab/row[1]/cell[5]/infos/bounds", "height" );
    CPPUNIT_ASSERT_EQUAL( height3, height4 );
}

DECLARE_OOXMLIMPORT_TEST(test_extra_image, "test_extra_image.docx" )
{
    // fdo#74652 Check there is no shape added to the doc during import
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xDraws->getCount());
}

DECLARE_OOXMLIMPORT_TEST(testFdo74401, "fdo74401.docx")
{
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShape(xGroupShape->getByIndex(1), uno::UNO_QUERY);
    // The triangle (second child) was a TextShape before, so it was shown as a rectangle.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.CustomShape"), xShape->getShapeType());
}

DECLARE_OOXMLIMPORT_TEST(testFdo75722vml, "fdo75722-vml.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    awt::Point aPos = xShape->getPosition();
    awt::Size aSize = xShape->getSize();
    sal_Int64 nRot = getProperty<sal_Int64>(xShape, "RotateAngle");

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3720), aPos.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-392), aPos.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5457), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3447), aSize.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3100), nRot);
}

DECLARE_OOXMLIMPORT_TEST(testFdo78939, "fdo78939.docx")
{
    // fdo#78939 : LO hanged while opening issue document

    // Whenever a para-style was applied to a Numbering format level,
    // LO incorrectly also changed the para-style..

    // check that file opens and does not hang while opening and also
    // check that an incorrect numbering style is not applied ...
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(1), "NumberingStyleName"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo75722dml, "fdo75722-dml.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    awt::Point aPos = xShape->getPosition();
    awt::Size aSize = xShape->getSize();
    sal_Int64 nRot = getProperty<sal_Int64>(xShape, "RotateAngle");

    // a slight difference regarding vml file is tolerated due to rounding errors
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3720), aPos.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-397), aPos.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5457), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3447), aSize.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3128), nRot);
}

DECLARE_OOXMLIMPORT_TEST(testFdo55381, "fdo55381.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(4), xCursor->getPage());
}

DECLARE_OOXMLIMPORT_TEST(testGridBefore, "gridbefore.docx")
{
    // w:gridBefore is faked by inserting two cells without border (because Writer can't do non-rectangular tables).
    // So check the first cell in the first row is in fact 3rd and that it's more to the right than the second
    // cell on the second row.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableColumns> xTableColumns(xTextTable->getColumns(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 3 ), xTableColumns->getCount());
    OUString textA3 = parseDump("/root/page/body/tab/row[1]/cell[3]/txt/text()" );
    OUString leftA3 = parseDump("/root/page/body/tab/row[1]/cell[3]/infos/bounds", "left" );
    OUString leftB2 = parseDump("/root/page/body/tab/row[2]/cell[2]/infos/bounds", "left" );
    CPPUNIT_ASSERT_EQUAL( OUString( "A3" ), textA3 );
    CPPUNIT_ASSERT( leftA3.toInt32() > leftB2.toInt32());
}

DECLARE_OOXMLIMPORT_TEST(testMsoBrightnessContrast, "msobrightnesscontrast.docx")
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
    CPPUNIT_ASSERT_EQUAL( sal_Int32(56374), data.getLength());
    CPPUNIT_ASSERT_EQUAL( -50, int(data[0x6e0])); // -50 = 206 pixel value
    CPPUNIT_ASSERT_EQUAL( -50, int(data[0x6e1]));
    CPPUNIT_ASSERT_EQUAL( -50, int(data[0x6e2]));
    CPPUNIT_ASSERT_EQUAL( -50, int(data[0x6e3]));
    CPPUNIT_ASSERT_EQUAL( -50, int(data[0x6e4]));
}

DECLARE_OOXMLIMPORT_TEST(testChartSize, "chart-size.docx")
{
    // When chart was in a TextFrame, its size was too large.
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This was 10954.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6008), getProperty<sal_Int32>(xEmbeddedObjects->getByIndex(0), "Width"));
}

DECLARE_OOXMLIMPORT_TEST(testInlineGroupshape, "inline-groupshape.docx")
{
    // Inline groupshape was in the background, so it was hidden sometimes by other shapes.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo78883, "fdo78883.docx")
{
    // fdo#78883 : LO was getting hang while opening document
    // Checking there is a single page after loading a doc in LO.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();

    // Check to make sure the document loaded.  Note that the page number may
    // be 1 or 2 depending on the environment.
    CPPUNIT_ASSERT(xCursor->getPage() > sal_Int16(0));
}

DECLARE_OOXMLIMPORT_TEST(testBnc875718, "bnc875718.docx")
{
    // The frame in the footer must not accidentally end up in the document body.
    // The easiest way for this to test I've found is checking that
    // xray ThisComponent.TextFrames.GetByIndex( index ).Anchor.Text.ImplementationName
    // is not SwXBodyText but rather SwXHeadFootText
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    for( int i = 0;
         i < xIndexAccess->getCount();
         ++i )
    {
        uno::Reference<text::XTextFrame> frame(xIndexAccess->getByIndex( i ), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> range(frame->getAnchor(), uno::UNO_QUERY);
        uno::Reference<lang::XServiceInfo> text(range->getText(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL( OUString( "SwXHeadFootText" ), text->getImplementationName());
    }
    // Also check that the footer contents are not in the body text.
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> text(textDocument->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( OUString( "Text\n" ), text->getString());
}

DECLARE_OOXMLIMPORT_TEST(testFootnote, "footnote.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFootnote(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    OUString aFootnote = xFootnote->getString();
    // Ensure there are no additional newlines after "bar".
    CPPUNIT_ASSERT(aFootnote.endsWith("bar"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo80555, "fdo80555.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Shape was wrongly placed at X=0, Y=0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3318), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(245), xShape->getPosition().Y);
}

DECLARE_OOXMLIMPORT_TEST(testTableBtlrCenter, "table-btlr-center.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell vertical alignment was NONE, should be CENTER.
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER, getProperty<sal_Int16>(xTable->getCellByName("A2"), "VertOrient"));
}

DECLARE_OOXMLIMPORT_TEST(testFloatingTableSectionColumns, "floating-table-section-columns.docx")
{
    OUString tableWidth = parseDump("/root/page[1]/body/section/column[2]/body/txt/anchored/fly/tab/infos/bounds", "width");
    // table width was restricted by a column
    CPPUNIT_ASSERT( tableWidth.toInt32() > 10000 );
}

DECLARE_OOXMLIMPORT_TEST(testHidemark, "hidemark.docx")
{
    // Problem was that <w:hideMark> cell property was ignored.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // Height should be minimal
    CPPUNIT_ASSERT_EQUAL(convertTwipToMm100(MINLAY), getProperty<sal_Int64>(xTableRows->getByIndex(1), "Height"));
    // Size type was MIN, should be FIX to avoid considering the end of paragraph marker.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX, getProperty<sal_Int16>(xTableRows->getByIndex(1), "SizeType"));
}

DECLARE_OOXMLIMPORT_TEST(testBnc519228OddBreaks, "bnc519228_odd-breaks.docx")
{
    // Check that all the normal styles are not set as right-only, those should be only those used after odd page breaks.
    uno::Reference<beans::XPropertySet> defaultStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_ALL), defaultStyle->getPropertyValue("PageStyleLayout"));
    uno::Reference<beans::XPropertySet> firstPage( getStyles("PageStyles")->getByName("First Page"), uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_ALL), firstPage->getPropertyValue("PageStyleLayout"));

    OUString page1StyleName = getProperty<OUString>( getParagraph( 1, "This is the first page." ), "PageDescName");
    uno::Reference<beans::XPropertySet> page1Style(getStyles("PageStyles")->getByName(page1StyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_RIGHT), page1Style->getPropertyValue("PageStyleLayout"));
    getParagraphOfText( 1, getProperty< uno::Reference<text::XText> >(page1Style, "HeaderText"), "This is the header for odd pages");

    // Page2 comes from follow of style for page 1 and should be a normal page. Also check the two page style have the same properties,
    // since page style for page1 was created from page style for page 2.
    OUString page2StyleName = getProperty<OUString>( getParagraph( 3, "This is page 2, which is obviously an even page." ), "PageDescName");
    CPPUNIT_ASSERT_EQUAL(OUString(), page2StyleName);
    page2StyleName = getProperty<OUString>( page1Style, "FollowStyle" );
    uno::Reference<beans::XPropertySet> page2Style(getStyles("PageStyles")->getByName(page2StyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_ALL), page2Style->getPropertyValue("PageStyleLayout"));
    getParagraphOfText( 1, getProperty< uno::Reference<text::XText> >(page2Style, "HeaderTextLeft"), "This is the even header");
    getParagraphOfText( 1, getProperty< uno::Reference<text::XText> >(page2Style, "HeaderTextRight"), "This is the header for odd pages");
    CPPUNIT_ASSERT_EQUAL(getProperty<sal_Int32>(page1Style, "TopMargin"), getProperty<sal_Int32>(page2Style, "TopMargin"));

    OUString page5StyleName = getProperty<OUString>( getParagraph( 5, "Then an odd break after an odd page, should lead us to page #5." ), "PageDescName");
    uno::Reference<beans::XPropertySet> page5Style(getStyles("PageStyles")->getByName(page5StyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_RIGHT), page5Style->getPropertyValue("PageStyleLayout"));
    getParagraphOfText( 1, getProperty< uno::Reference<text::XText> >(page5Style, "HeaderText"), "This is the header for odd pages");
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
