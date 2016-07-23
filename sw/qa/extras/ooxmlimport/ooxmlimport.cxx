/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config_test.h"

#ifdef MACOSX
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <premac.h>
#include <AppKit/AppKit.h>
#include <postmac.h>
#endif

#include <swmodeltestbase.hxx>

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
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
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/RubyAdjust.hpp>
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
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <vcl/bitmapaccess.hxx>
#include <vcl/svapp.hxx>
#include <unotest/assertion_traits.hxx>
#include <unotools/fltrcfg.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <swtypes.hxx>
#include <drawdoc.hxx>
#include <tools/datetimeutils.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/propertysequence.hxx>
#include <svx/svdpage.hxx>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>

#include <bordertest.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlimport/data/", "Office Open XML Text")
    {
    }

    virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        if (OString(filename) == "smartart.docx" || OString(filename) == "strict-smartart.docx" || OString(filename) == "fdo87488.docx")
        {
            std::unique_ptr<Resetter> pResetter(new Resetter(
                [] () {
                    SvtFilterOptions::Get().SetSmartArt2Shape(false);
                }));
            SvtFilterOptions::Get().SetSmartArt2Shape(true);
            return pResetter;
        }
        return nullptr;
    }

protected:
    /// Copy&paste helper.
    bool paste(const OUString& rFilename, const uno::Reference<text::XTextRange>& xTextRange)
    {
        uno::Reference<document::XFilter> xFilter(m_xSFactory->createInstance("com.sun.star.comp.Writer.WriterFilter"), uno::UNO_QUERY_THROW);
        uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
        xImporter->setTargetDocument(mxComponent);
        SvStream* pStream = utl::UcbStreamHelper::CreateStream(m_directories.getURLFromSrc("/sw/qa/extras/ooxmlimport/data/") + rFilename, StreamMode::READ);
        uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
        uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
        {
            {"InputStream", uno::makeAny(xStream)},
            {"InputMode", uno::makeAny(true)},
            {"TextInsertModeRange", uno::makeAny(xTextRange)},
        }));
        return xFilter->filter(aDescriptor);
    }
};

class FailTest : public Test
{
public:
    // UGLY: hacky manual override of MacrosTest::loadFromDesktop
    void executeImportTest(const char* filename)
    {
        header();
        preTest(filename);
        {
            if (mxComponent.is())
                mxComponent->dispose();
            std::cout << filename << ",";
            mnStartTime = osl_getGlobalTimer();
            {
                OUString aURL(m_directories.getURLFromSrc(mpTestDocumentPath) + OUString::createFromAscii(filename));
                CPPUNIT_ASSERT_MESSAGE("no desktop", mxDesktop.is());
                uno::Reference<frame::XComponentLoader> xLoader(mxDesktop, uno::UNO_QUERY);
                CPPUNIT_ASSERT_MESSAGE("no loader", xLoader.is());
                uno::Sequence<beans::PropertyValue> args(1);
                args[0].Name = "DocumentService";
                args[0].Handle = -1;
                args[0].Value <<= OUString("com.sun.star.text.TextDocument");
                args[0].State = beans::PropertyState_DIRECT_VALUE;

                uno::Reference<lang::XComponent> xComponent = xLoader->loadComponentFromURL(aURL, "_default", 0, args);
                OUString sMessage = "loading succeeded: " + aURL;
                CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sMessage, RTL_TEXTENCODING_UTF8).getStr(), !xComponent.is());
            }
        }
        postTest(filename);
        verify();
        finish();
    }
};

#if !defined(_WIN32)

DECLARE_SW_IMPORT_TEST(testMathMalformedXml, "math-malformed_xml.docx", FailTest)
{
    CPPUNIT_ASSERT(!mxComponent.is());
}

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
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
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

DECLARE_OOXMLIMPORT_TEST(testFdo74745, "fdo74745.docx")
{
    uno::Reference<text::XTextRange > paragraph = getParagraph(3);
    uno::Reference<text::XTextRange> text(paragraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text->getString(),OUString("09/02/14"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo81486, "fdo81486.docx")
{
    uno::Reference<text::XTextRange > paragraph = getParagraph(1);
    uno::Reference<text::XTextRange> text(paragraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text->getString(),OUString("CustomTitle"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo79738, "fdo79738.docx")
{
    uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier( mxComponent, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
    uno::Reference<container::XNameContainer> xStyles;
    xStyleFamilies->getByName("ParagraphStyles") >>= xStyles;
    uno::Reference<beans::XPropertySet> xPropertySetHeader( xStyles->getByName("Header"), uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL(false, xPropertySetHeader->getPropertyValue("ParaLineNumberCount").get<bool>());
    uno::Reference<beans::XPropertySet> xPropertySetFooter( xStyles->getByName("Footer"), uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL(false, xPropertySetFooter->getPropertyValue("ParaLineNumberCount").get<bool>());
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

DECLARE_OOXMLIMPORT_TEST(testfdo90720, "testfdo90720.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    uno::Reference<text::XTextFrame> textbox(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> properties(textbox, uno::UNO_QUERY);
    sal_Int32 fill_transperence;
    properties->getPropertyValue( "FillTransparence" ) >>= fill_transperence;
    CPPUNIT_ASSERT_EQUAL( sal_Int32(100), fill_transperence );
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
are on page 2 and page 3
*/
    CPPUNIT_ASSERT_EQUAL( OUString("one"), parseDump("/root/page[2]/body/txt/text()") );
    CPPUNIT_ASSERT_EQUAL( OUString("two"), parseDump("/root/page[3]/body/txt/text()") );
}

DECLARE_OOXMLIMPORT_TEST(testTdf95367_inheritFollowStyle, "tdf95367_inheritFollowStyle.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("header"),  parseDump("/root/page[2]/header/txt/text()"));
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
    pageStyles->getByName("Standard") >>= defaultStyle;
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
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("LeftMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(847), nValue);

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
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
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
note that the indexes may get off as the implementation evolves, C++ code searches in loops
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
    BorderTest::testTheBorders(mxComponent, false);
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
    CPPUNIT_ASSERT_EQUAL(BitmapChecksum(SAL_CONST_UINT64(18203404956065762943)), aGraphic.GetChecksum());
#endif
}

DECLARE_OOXMLIMPORT_TEST(testTdf59699, "tdf59699.docx")
{
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, "Graphic");
    // This was false: the referenced graphic data wasn't imported.
    CPPUNIT_ASSERT(xGraphic.is());
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(1), "ParaRightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3810), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-635), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
}

DECLARE_OOXMLIMPORT_TEST(testN778140, "n778140.docx")
{
    /*
     * The problem was that the paragraph top/bottom margins were incorrect due
     * to unhandled w:doNotUseHTMLParagraphAutoSpacing.
     */
    CPPUNIT_ASSERT_EQUAL(sal_Int32(176), getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(176), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
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

DECLARE_OOXMLIMPORT_TEST(testRHBZ1180114, "rhbz1180114.docx")
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
     * Another problem tested with this document is the unnecessary loading of the shapes
     * anchored to a discarded header or footer
     */
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> drawPageSupplier(textDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> drawPage = drawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0 ), drawPage->getCount());
}

DECLARE_OOXMLIMPORT_TEST(testN779627b, "n779627b.docx")
{
    /*
     * Another problem tested with the original n779627.docx document (before removing its unnecessary
     * shape loading) is that the roundrect is centered vertically and horizontally.
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

DECLARE_OOXMLIMPORT_TEST(testTDF91122, "tdf91122.docx")
{
    /*
     * OLE object shape: default vertical position is top in MSO, not bottom
     */
    for (int i = 1; i <= 2; ++i)
    {
        uno::Reference<beans::XPropertySet> xShapeProperties( getShape(i), uno::UNO_QUERY );
        uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShapeProperties, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), xShapeDescriptor->getShapeType());
        sal_Int16 nValue;
        xShapeProperties->getPropertyValue("VertOrient") >>= nValue;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical orientation", text::VertOrientation::TOP, nValue);
    }
}

DECLARE_OOXMLIMPORT_TEST(testTDF91260, "tdf91260.docx")
{
    /*
     * textbox can't extend beyond the page bottom
     * solution: shrinking textbox (its text frame) height, if needed
     */
    uno::Reference<text::XTextRange> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame->getString().startsWith( "Lorem ipsum" ) );
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3454), getProperty<sal_Int32>(xFrame, "Height"));
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

    xPara = getParagraph(1);
    aStyleName = getProperty<OUString>(xPara, "PageStyleName");
    CPPUNIT_ASSERT_EQUAL(OUString("First Page"), aStyleName);

    //tdf64372 this document should only have one page break (2 pages, not 3)
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());
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

DECLARE_OOXMLIMPORT_TEST(testWordArtResizing, "WordArt.docx")
{
    /* The Word-Arts and watermarks were getting resized automatically, It was as if they were
       getting glued to the fallback geometry(the sdrObj) and were getting bound to the font size.
       The test-case ensures the original height and width of the word-art is not changed while importing*/

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());

    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10105), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4755), xShape->getSize().Height);
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
    uno::Reference<container::XEnumerationAccess> xHeaderText = getProperty< uno::Reference<container::XEnumerationAccess> >(getStyles("PageStyles")->getByName("Standard"), "HeaderText");
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
    // Unhandled exception prevented import of the rest of the document.

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
        CPPUNIT_ASSERT_EQUAL(sal_Int32(oox::drawingml::convertEmuToHmm(0)), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
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
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<bool>(xTables->getByIndex(0), "IsWidthRelative")));

    uno::Reference<text::XTextFramesSupplier> xFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(xFrames->getByIndex(0), "FrameWidthPercent"));
}

DECLARE_OOXMLIMPORT_TEST(testConditionalstylesTablelook, "conditionalstyles-tbllook.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Background was -1.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x7F7F7F), getProperty<sal_Int32>(xTable->getCellByName("A1"), "BackColor"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo63685, "fdo63685.docx")
{
    // An inline image's wrapping should be always zero, even if the doc model has a non-zero value.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getShape(1), "TopMargin"));
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
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<bool>(xPropertySet, "Dropdown")));

    // Second shape: combo box
    xControlShape.set(getShape(2), uno::UNO_QUERY);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.ComboBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("dropdown default text"), getProperty<OUString>(xPropertySet, "DefaultText"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty< uno::Sequence<OUString> >(xPropertySet, "StringItemList").getLength());
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<bool>(xPropertySet, "Dropdown")));
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
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(getStyles("PageStyles")->getByName("Standard"), "ShadowFormat");
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(oox::drawingml::convertEmuToHmm(928440)), xLine->getSize().Width);
}

DECLARE_OOXMLIMPORT_TEST(testTableAutoColumnFixedSize, "table-auto-column-fixed-size.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Width was not recognized during import when table size was 'auto'
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(3996)), getProperty<sal_Int32>(xTextTable, "Width"));
}

DECLARE_OOXMLIMPORT_TEST(testTableAutoColumnFixedSize2, "table-auto-column-fixed-size2.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was 17907, i.e. the sum of the width of the 3 cells (10152 twips each), which is too wide.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16891), getProperty<sal_Int32>(xTextTable, "Width"));
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
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xPageStyle, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
    // Check for the Column Separator value.It should be FALSE as the document does not contain separator line.
    bool bValue = getProperty< bool >(xTextColumns, "SeparatorLineIsOn");
    CPPUNIT_ASSERT(!bValue) ;
}

DECLARE_OOXMLIMPORT_TEST(testFdo69636, "fdo69636.docx")
{
    // The problem was that the mso-layout-flow-alt:bottom-to-top VML shape property wasn't handled for sw text frames.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aCustomShapeGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-270), aCustomShapeGeometry["TextPreRotateAngle"].get<sal_Int32>());
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

void lcl_countTextFrames(css::uno::Reference< lang::XComponent >& xComponent,
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

DECLARE_OOXMLIMPORT_TEST(testTdf75573, "tdf75573_page1frame.docx")
{
    // the problem was that the frame was discarded
    // when an unrelated, unused, odd-header was flagged as discardable
    lcl_countTextFrames( mxComponent, 1 );

    // the frame should be on page 1
    CPPUNIT_ASSERT_EQUAL( OUString("lorem ipsum"), parseDump("/root/page[1]/body/section/txt/anchored/fly/txt[1]/text()") );

    // the "Proprietary" style should set the vertical and horizontal anchors to the page
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xPropertySet, "VertOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xPropertySet, "HoriOrientRelation"));

    // the frame should be located near the bottom[23186]/center[2955] of the page
    CPPUNIT_ASSERT(sal_Int32(20000) < getProperty<sal_Int32>(xPropertySet, "VertOrientPosition"));
    CPPUNIT_ASSERT(sal_Int32(2500) < getProperty<sal_Int32>(xPropertySet, "HoriOrientPosition"));
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(oox::drawingml::convertEmuToHmm(671830)), xShape->getPosition().X);

    // Left margin was 0, instead of 114300 EMU's.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(318), getProperty<sal_Int32>(xShape, "LeftMargin"));
    // Wrap type was PARALLEL.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT, getProperty<text::WrapTextMode>(xShape, "Surround"));

    // This should be in front of text.
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<bool>(xShape, "Opaque")));
    // And this should be behind the document.
    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<bool>(getShape(2), "Opaque")));
}

DECLARE_OOXMLIMPORT_TEST(lineWpsOnly, "line-wps-only.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Check position, it was -7223 as it was set after the CustomShapeGeometry property.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(210), xShape->getPosition().X);
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
    uno::Reference<text::XTextRange> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello world!"), xFrame->getString());
    // Position wasn't horizontally centered.
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(xFrame, "HoriOrient"));

    // Position was the default (hori center, vert top) for the textbox.
    xFrame.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2173), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
#ifdef MACOSX
    // FIXME: The assert below fails wildly on a Retina display
    NSScreen* nsScreen = [ NSScreen mainScreen ];
    CGFloat scaleFactor = [ nsScreen backingScaleFactor ];   // for instance on the 5K Retina iMac,
                                                             // [NSScreen mainScreen].frame.size is 2560x1440,
                                                             // while real display size is 5120x2880
    if ( nsScreen.frame.size.width * scaleFactor > 4000 )
        return;
#endif
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2805), getProperty<sal_Int32>(xFrame, "VertOrientPosition"));
}

DECLARE_OOXMLIMPORT_TEST(testWpgOnly, "wpg-only.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Check position, it was nearly 0. This is a shape, so use getPosition(), not a property.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(oox::drawingml::convertEmuToHmm(548005)), xShape->getPosition().X);
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
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<bool>(xShape, "Opaque")));

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
    // Vertical position of the shape was incorrect due to incorrect nested mce handling.
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // positionV's posOffset from the bugdoc, was 0.
    CPPUNIT_ASSERT(6985 <= getProperty<sal_Int32>(xShape, "VertOrientPosition"));
    // This was -1 (default), make sure the background color is set.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4f81bd), getProperty<sal_Int32>(xShape, "FillColor"));

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
    OUString aTocString(xTextCursor->getString());
    aTocString = aTocString.copy(256);
    CPPUNIT_ASSERT(aTocString.startsWithIgnoreAsciiCase( "Heading 15.1:\t15" ) );
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(oox::drawingml::convertEmuToHmm(9142730)), getShape(1)->getSize().Height);
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
    Graphic aVclGraphic(xGraphic);
    Bitmap aBitmap(aVclGraphic.GetBitmap());
    BitmapReadAccess* pAccess = aBitmap.AcquireReadAccess();
    CPPUNIT_ASSERT(pAccess);
    CPPUNIT_ASSERT_EQUAL(341L, pAccess->Width());
    CPPUNIT_ASSERT_EQUAL(181L, pAccess->Height());
    Color aColor(pAccess->GetPixel(30, 120));
    CPPUNIT_ASSERT_EQUAL(aColor.GetColor(), RGB_COLORDATA( 0xb1, 0xc8, 0xdd ));
    aColor = pAccess->GetPixel(130, 260);
    CPPUNIT_ASSERT_EQUAL(aColor.GetColor(), RGB_COLORDATA( 0xb1, 0xc8, 0xdd ));
    Bitmap::ReleaseAccess(pAccess);
}

DECLARE_OOXMLIMPORT_TEST(testTdf99135, "tdf99135.docx")
{
    // This was 0, crop was ignored on VML import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1825), getProperty<text::GraphicCrop>(getShape(1), "GraphicCrop").Bottom);
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

DECLARE_OOXMLIMPORT_TEST(testTdf85523, "tdf85523.docx")
{
    auto xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 6), "TextField");
    auto xText = getProperty< uno::Reference<text::XText> >(xTextField, "TextRange");
    // This was "commentX": an unexpected extra char was added at the comment end.
    getParagraphOfText(1, xText, "comment");
}

DECLARE_OOXMLIMPORT_TEST(testDMLGroupShapeRunFonts, "dml-groupshape-runfonts.docx")
{
    // Fonts defined by w:rFonts was not imported and so the font specified by a:fontRef was used.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText    = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText),1);
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Arial Unicode MS"), getProperty<OUString>(xRun, "CharFontNameComplex"));
    CPPUNIT_ASSERT_EQUAL(OUString("MS Mincho"), getProperty<OUString>(xRun, "CharFontNameAsian"));
}

DECLARE_OOXMLIMPORT_TEST(testStrict, "strict.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
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
    // Slightly related to cp#1000043, the twips value was negative, which wrapped around somewhere,
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
    Graphic aVclGraphic(graphic);
    Bitmap aBitmap(aVclGraphic.GetBitmap());
    BitmapReadAccess* pAccess = aBitmap.AcquireReadAccess();
    CPPUNIT_ASSERT(pAccess);
    CPPUNIT_ASSERT_EQUAL(58L, pAccess->Width());
    CPPUNIT_ASSERT_EQUAL(320L, pAccess->Height());
    Color aColor(pAccess->GetPixel(30, 20));
    CPPUNIT_ASSERT_EQUAL(aColor.GetColor(), RGB_COLORDATA( 0xce, 0xce, 0xce ));
    Bitmap::ReleaseAccess(pAccess);
}

DECLARE_OOXMLIMPORT_TEST(testChartSize, "chart-size.docx")
{
    // When chart was in a TextFrame, its size was too large.
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This was 10954.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6008), getProperty<sal_Int32>(xEmbeddedObjects->getByIndex(0), "Width"));

    // Layout modified the document when it had this chart.
    uno::Reference<util::XModifiable> xModifiable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, bool(xModifiable->isModified()));
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

DECLARE_OOXMLIMPORT_TEST(testFdo79535, "fdo79535.docx")
{
    // fdo#79535 : LO was crashing while opening document
    // Checking there is a single page after loading a doc successfully in LO.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
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
    CPPUNIT_ASSERT_EQUAL( OUString( "Text" ), text->getString());
}

DECLARE_OOXMLIMPORT_TEST(testCaption, "caption.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Caption"), uno::UNO_QUERY);
    // This was awt::FontSlant_ITALIC: Writer default was used instead of what is in the document.
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xStyle, "CharPosture"));
}

DECLARE_OOXMLIMPORT_TEST(testGroupshapeTrackedchanges, "groupshape-trackedchanges.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // Shape text was completely missing, ensure inserted text is available.
    CPPUNIT_ASSERT_EQUAL(OUString(" Inserted"), xShape->getString());
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

DECLARE_OOXMLIMPORT_TEST(testFootnote, "footnote.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFootnote(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    OUString aFootnote = xFootnote->getString();
    // Ensure there are no additional newlines after "bar".
    CPPUNIT_ASSERT(aFootnote.endsWith("bar"));
}

DECLARE_OOXMLIMPORT_TEST(testTableBtlrCenter, "table-btlr-center.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell vertical alignment was NONE, should be CENTER.
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER, getProperty<sal_Int16>(xTable->getCellByName("A2"), "VertOrient"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo80555, "fdo80555.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Shape was wrongly placed at X=0, Y=0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3318), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(247), xShape->getPosition().Y);
}

DECLARE_OOXMLIMPORT_TEST(testFdo76803, "fdo76803.docx")
{
    // The ContourPolyPolygon was wrong
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);

    drawing::PointSequenceSequence rContour = getProperty<drawing::PointSequenceSequence>(xPropertySet, "ContourPolyPolygon");
    basegfx::B2DPolyPolygon aPolyPolygon(basegfx::tools::UnoPointSequenceSequenceToB2DPolyPolygon(rContour));

    // We've got exactly one polygon inside
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), aPolyPolygon.count());

    // Now check it deeply
    basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(0));

    CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), aPolygon.count());

    CPPUNIT_ASSERT_EQUAL(double(-163), aPolygon.getB2DPoint(0).getX());
    CPPUNIT_ASSERT_EQUAL(double(0), aPolygon.getB2DPoint(0).getY());

    CPPUNIT_ASSERT_EQUAL(double(-163), aPolygon.getB2DPoint(1).getX());
    CPPUNIT_ASSERT_EQUAL(double(3661), aPolygon.getB2DPoint(1).getY());

    CPPUNIT_ASSERT_EQUAL(double(16988), aPolygon.getB2DPoint(2).getX());
    CPPUNIT_ASSERT_EQUAL(double(3661), aPolygon.getB2DPoint(2).getY());

    CPPUNIT_ASSERT_EQUAL(double(16988), aPolygon.getB2DPoint(3).getX());
    CPPUNIT_ASSERT_EQUAL(double(0), aPolygon.getB2DPoint(3).getY());
}

DECLARE_OOXMLIMPORT_TEST(testUnbalancedColumns, "unbalanced-columns.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // This was false, last section was balanced, but it's unbalanced in Word.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextSections->getByIndex(2), "DontBalanceTextColumns"));
}

DECLARE_OOXMLIMPORT_TEST(testUnbalancedColumnsCompat, "unbalanced-columns-compat.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // This was false, we ignored the relevant compat setting to make this non-last section unbalanced.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
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
    uno::Reference<beans::XPropertySet> defaultStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_ALL), defaultStyle->getPropertyValue("PageStyleLayout"));
    uno::Reference<beans::XPropertySet> firstPage( getStyles("PageStyles")->getByName("First Page"), uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_ALL), firstPage->getPropertyValue("PageStyleLayout"));

    OUString page1StyleName = getProperty<OUString>( getParagraph( 1, "This is the first page." ), "PageDescName");
    uno::Reference<beans::XPropertySet> page1Style(getStyles("PageStyles")->getByName(page1StyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_RIGHT), page1Style->getPropertyValue("PageStyleLayout"));
    getParagraphOfText( 1, getProperty< uno::Reference<text::XText> >(page1Style, "HeaderText"), "This is the header for odd pages");

    // Page2 comes from follow of style for page 1 and should be a normal page. Also check the two page style have the same properties,
    // since page style for page1 was created from page style for page 2.
    uno::Any page2StyleAny = uno::Reference<beans::XPropertySet>(
        getParagraph(3, "This is page 2, which is obviously an even page."),
        uno::UNO_QUERY_THROW)->getPropertyValue("PageDescName");
    CPPUNIT_ASSERT_EQUAL(uno::Any(), page2StyleAny);
    OUString page2StyleName = getProperty<OUString>( page1Style, "FollowStyle" );
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

DECLARE_OOXMLIMPORT_TEST(testBnc891663, "bnc891663.docx")
{
    // The image should be inside a cell, so the text in the following cell should be below it.
    int imageTop = parseDump("/root/page/body/tab/row[1]/cell[2]/txt[1]/anchored/fly/infos/bounds", "top").toInt32();
    int imageHeight = parseDump("/root/page/body/tab/row[1]/cell[2]/txt[1]/anchored/fly/infos/bounds", "height").toInt32();
    int textNextRowTop = parseDump("/root/page/body/tab/row[2]/cell[1]/txt[1]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT( textNextRowTop >= imageTop + imageHeight );
}

static OString dateTimeToString( const util::DateTime& dt )
{
    return DateTimeToOString( DateTime( Date( dt.Day, dt.Month, dt.Year ), tools::Time( dt.Hours, dt.Minutes, dt.Seconds )));
}

DECLARE_OOXMLIMPORT_TEST(testTcwRounding, "tcw-rounding.docx")
{
    // Width of the A1 cell in twips was 3200, due to a rounding error.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3201),  parseDump("/root/page/body/tab/row/cell[1]/infos/bounds", "width").toInt32());
}

DECLARE_OOXMLIMPORT_TEST(testBnc821804, "bnc821804.docx")
{
    CPPUNIT_ASSERT_EQUAL( OUString( "TITLE" ), getRun( getParagraph( 1 ), 1 )->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(1), 1), "RedlineType"));
    // Redline information (SwXRedlinePortion) are separate "runs" apparently.
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(1), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(1), 2), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown1"),getProperty<OUString>(getRun(getParagraph(1), 2), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OString("2006-08-29T09:46:00Z"),dateTimeToString(getProperty<util::DateTime>(getRun(getParagraph(1), 2), "RedlineDateTime")));
    // So only the 3rd run is actual text (and the two runs have been merged into one, not sure why, but that shouldn't be a problem).
    CPPUNIT_ASSERT_EQUAL(OUString(" (1st run of an insert) (2nd run of an insert)"), getRun(getParagraph(1),3)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(1), 3), "RedlineType"));
    // And the end SwXRedlinePortion of the redline.
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(1), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown1"),getProperty<OUString>(getRun(getParagraph(1), 4), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OString("2006-08-29T09:46:00Z"),dateTimeToString(getProperty<util::DateTime>(getRun(getParagraph(1), 4), "RedlineDateTime")));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(1), 4), "IsStart"));

    CPPUNIT_ASSERT_EQUAL(OUString("Normal text"), getRun(getParagraph(2),1)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(2), 1), "RedlineType"));

    CPPUNIT_ASSERT_EQUAL(OUString("Delete"),getProperty<OUString>(getRun(getParagraph(3), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown2"),getProperty<OUString>(getRun(getParagraph(3), 1), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OString("2006-08-29T09:47:00Z"),dateTimeToString(getProperty<util::DateTime>(getRun(getParagraph(3), 1), "RedlineDateTime")));
    CPPUNIT_ASSERT_EQUAL(OUString("Deleted"), getRun(getParagraph(3),2)->getString());

    // This is both inserted and formatted, so there are two SwXRedlinePortion "runs". Given that the redlines overlap and Writer core
    // doesn't officially expect that (even though it copes, the redline info will be split depending on how Writer deal with it).
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(4), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(4), 1), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(4), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(4), 2), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("Inserted and formatted"), getRun(getParagraph(4),3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(4), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(4), 4), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString(" and this is only formatted"), getRun(getParagraph(4),5)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(4), 6), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(4), 6), "IsStart"));

    CPPUNIT_ASSERT_EQUAL(OUString("Normal text"), getRun(getParagraph(5),1)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(5), 1), "RedlineType"));

    CPPUNIT_ASSERT_EQUAL(OUString("Format"),getProperty<OUString>(getRun(getParagraph(6), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(6), 1), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown5"),getProperty<OUString>(getRun(getParagraph(6), 1), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OString("2006-08-29T10:02:00Z"),dateTimeToString(getProperty<util::DateTime>(getRun(getParagraph(6), 1), "RedlineDateTime")));
    CPPUNIT_ASSERT_EQUAL(OUString("Formatted run"), getRun(getParagraph(6),2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Format"),getProperty<OUString>(getRun(getParagraph(6), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(6), 3), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString(" and normal text here "), getRun(getParagraph(6),4)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(6), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(6), 5), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(6), 5), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown6"),getProperty<OUString>(getRun(getParagraph(6), 5), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OString("2006-08-29T09:48:00Z"),dateTimeToString(getProperty<util::DateTime>(getRun(getParagraph(6), 5), "RedlineDateTime")));
    CPPUNIT_ASSERT_EQUAL(OUString("and inserted again"), getRun(getParagraph(6),6)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(6), 7), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(6), 7), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString(" and normal text again "), getRun(getParagraph(6),8)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(6), 8), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Format"),getProperty<OUString>(getRun(getParagraph(6), 9), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(6), 9), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown7"),getProperty<OUString>(getRun(getParagraph(6), 9), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OUString("and formatted"), getRun(getParagraph(6),10)->getString());
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(6), 11), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString(" and normal."), getRun(getParagraph(6),12)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(6), 12), "RedlineType"));

    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(7), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(7), 1), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown8"),getProperty<OUString>(getRun(getParagraph(7), 1), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OUString("One insert."), getRun(getParagraph(7),2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(7), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(7), 3), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(7), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(7), 4), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("unknown9"),getProperty<OUString>(getRun(getParagraph(7), 4), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OUString("Second insert."), getRun(getParagraph(7),5)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(7), 6), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(7), 6), "IsStart"));

    // Overlapping again.
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"),getProperty<OUString>(getRun(getParagraph(8), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(8), 1), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(8), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(8), 2), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("Deleted and formatted"), getRun(getParagraph(8),3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"),getProperty<OUString>(getRun(getParagraph(8), 4), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(8), 4), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString(" and this is only formatted"), getRun(getParagraph(8),5)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(8), 6), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(8), 6), "IsStart"));

    CPPUNIT_ASSERT_EQUAL(OUString("Normal text"), getRun(getParagraph(9),1)->getString());
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(9), 1), "RedlineType"));

    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(10), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true,getProperty<bool>(getRun(getParagraph(10), 1), "IsStart"));
    CPPUNIT_ASSERT_EQUAL(OUString("This is only formatted."), getRun(getParagraph(10),2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("ParagraphFormat"),getProperty<OUString>(getRun(getParagraph(10), 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(false,getProperty<bool>(getRun(getParagraph(10), 3), "IsStart"));
}

DECLARE_OOXMLIMPORT_TEST(testFdo85542, "fdo85542.docx")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xBookmarksByIdx->getCount(), static_cast<sal_Int32>(3));
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("B1"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("B2"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("B3"));
    // B1
    uno::Reference<text::XTextContent> xContent1(xBookmarksByName->getByName("B1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange1(xContent1->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xRange1->getString(), OUString("ABB"));
    // B2
    uno::Reference<text::XTextContent> xContent2(xBookmarksByName->getByName("B2"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange2(xContent2->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xRange2->getString(), OUString("BBC"));
    // B3 -- testing a collapsed bookmark
    uno::Reference<text::XTextContent> xContent3(xBookmarksByName->getByName("B3"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange3(xContent3->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xRange3->getString(), OUString(""));
    uno::Reference<text::XText> xText(xRange3->getText( ), uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xNeighborhoodCursor(xText->createTextCursor( ), uno::UNO_QUERY);
    xNeighborhoodCursor->gotoRange(xRange3, false);
    xNeighborhoodCursor->goLeft(1, false);
    xNeighborhoodCursor->goRight(2, true);
    uno::Reference<text::XTextRange> xTextNeighborhood(xNeighborhoodCursor, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xTextNeighborhood->getString(), OUString("AB"));
}

DECLARE_OOXMLIMPORT_TEST(testChtOutlineNumberingOoxml, "chtoutline.docx")
{
    const sal_Unicode aExpectedPrefix[2] = { 0x7b2c, 0x0020 };
    const sal_Unicode aExpectedSuffix[2] = { 0x0020, 0x7ae0 };
    uno::Reference< text::XChapterNumberingSupplier > xChapterNumberingSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference< container::XIndexAccess> xLevels(xChapterNumberingSupplier->getChapterNumberingRules());
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix,aPrefix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
        if (rProp.Name == "Prefix")
            aPrefix = rProp.Value.get<OUString>();
    }
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedPrefix,SAL_N_ELEMENTS(aExpectedPrefix)), aPrefix);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedSuffix,SAL_N_ELEMENTS(aExpectedSuffix)), aSuffix);
}

DECLARE_OOXMLIMPORT_TEST(testFdo87488, "fdo87488.docx")
{
    // The shape on the right (index 0, CustomShape within a
    // GroupShape) is rotated 90 degrees clockwise and contains text
    // rotated 90 degrees anticlockwise.  Must be read with SmartArt
    // enabled in preTest above, otherwise it gets converted to a
    // StarView MetaFile.
    uno::Reference<container::XIndexAccess> group(getShape(1), uno::UNO_QUERY);
    {
        uno::Reference<text::XTextRange> text(group->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("text2"), text->getString());
    }
    {
        uno::Reference<beans::XPropertySet> props(group->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(props->getPropertyValue("RotateAngle"),
                             uno::makeAny<sal_Int32>(270 * 100));
        comphelper::SequenceAsHashMap geom(props->getPropertyValue("CustomShapeGeometry"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(90), geom["TextPreRotateAngle"].get<sal_Int32>());
    }
}

DECLARE_OOXMLIMPORT_TEST(mathtype, "mathtype.docx")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This failed as the Model property was empty.
    auto xModel = getProperty< uno::Reference<lang::XServiceInfo> >(xEmbeddedObjects->getByIndex(0), "Model");
    CPPUNIT_ASSERT(xModel->supportsService("com.sun.star.formula.FormulaProperties"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf49073, "tdf49073.docx")
{
    // test case for Asisan phontic guide ( ruby text.)
    sal_Unicode aRuby[3] = {0x304D,0x3082,0x3093};
    OUString sRuby = OUString(aRuby, SAL_N_ELEMENTS(aRuby));
    CPPUNIT_ASSERT_EQUAL(sRuby,getProperty<OUString>(getParagraph(1)->getStart(), "RubyText"));
    OUString sStyle = getProperty<OUString>( getParagraph(1)->getStart(), "RubyCharStyleName");
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("CharacterStyles")->getByName(sStyle), uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL(5.f, getProperty<float>(xPropertySet, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_CENTER) ,getProperty<sal_Int16>(getParagraph(2)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_BLOCK)  ,getProperty<sal_Int16>(getParagraph(3)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_INDENT_BLOCK),getProperty<sal_Int16>(getParagraph(4)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_LEFT)   ,getProperty<sal_Int16>(getParagraph(5)->getStart(),"RubyAdjust"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::RubyAdjust_RIGHT)  ,getProperty<sal_Int16>(getParagraph(6)->getStart(),"RubyAdjust"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf8255, "tdf8255.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xTextDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1: a full-page-wide multi-page floating table was imported as a TextFrame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

DECLARE_OOXMLIMPORT_TEST(testTdf87460, "tdf87460.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(xTextDocument, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes = xEndnotesSupplier->getEndnotes();
    // This was 0: endnote was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xEndnotes->getCount());
}

DECLARE_OOXMLIMPORT_TEST(testTdf90611, "tdf90611.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;
    // This was 11.
    CPPUNIT_ASSERT_EQUAL(10.f, getProperty<float>(getParagraphOfText(1, xFootnoteText), "CharHeight"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf89702, "tdf89702.docx")
{
    // Get the first paragraph's numbering style's 2nd level's character style name.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, "NumberingRules");
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(1) >>= aLevel; // 2nd level
    OUString aCharStyleName = std::find_if(aLevel.begin(), aLevel.end(), [](const beans::PropertyValue& rValue) { return rValue.Name == "CharStyleName"; })->Value.get<OUString>();

    // Make sure that the font name is Arial, this was Verdana.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName(aCharStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xStyle, "CharFontName"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf86374, "tdf86374.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // btLr text direction was imported as FIX, it should be MIN to have enough space for the additionally entered paragraphs.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::MIN, getProperty<sal_Int16>(xTableRows->getByIndex(0), "SizeType"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf87924, "tdf87924.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    // This was -270, the text rotation angle was set when it should not be rotated.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aGeometry["TextPreRotateAngle"].get<sal_Int32>());
}

#endif

DECLARE_OOXMLIMPORT_TEST(testIndents, "indents.docx")
{
    //expected left margin and first line indent values
    static const sal_Int32 indents[] =
    {
            0,     0,
        -2000,     0,
        -2000,  1000,
        -1000, -1000,
         2000, -1000
    };
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(), uno::UNO_QUERY);
    // list of paragraphs
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    size_t paraIndex = 0;
    do
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo;
        if (xParaEnum->nextElement() >>= xServiceInfo)
        {
            uno::Reference<beans::XPropertySet> const xPropertySet(xServiceInfo, uno::UNO_QUERY_THROW);
            sal_Int32 nIndent = 0;
            sal_Int32 nFirstLine = 0;
            xPropertySet->getPropertyValue("ParaLeftMargin") >>= nIndent;
            xPropertySet->getPropertyValue("ParaFirstLineIndent") >>= nFirstLine;
            CPPUNIT_ASSERT_EQUAL(indents[paraIndex * 2], nIndent);
            CPPUNIT_ASSERT_EQUAL(indents[paraIndex * 2 + 1], nFirstLine);
            ++paraIndex;
        }
    } while (xParaEnum->hasMoreElements());
}

DECLARE_OOXMLIMPORT_TEST(testTdf92454, "tdf92454.docx")
{
    // The first paragraph had a large indentation / left margin as inheritance
    // in Word and Writer works differently, and no direct value was set to be
    // explicit.
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(1), uno::UNO_QUERY);
    // This was beans::PropertyState_DEFAULT_VALUE.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, xParagraph->getPropertyState("ParaFirstLineIndent"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf95376, "tdf95376.docx")
{
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(2), uno::UNO_QUERY);
    // This was beans::PropertyState_DIRECT_VALUE: indentation-from-numbering
    // did not have priority over indentation-from-paragraph-style, due to a
    // filter workaround that's not correct here.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE, xParagraph->getPropertyState("ParaFirstLineIndent"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf92124, "tdf92124.docx")
{
    // Get the second paragraph's numbering style's 1st level's suffix.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(2);
    auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, "NumberingRules");
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(0) >>= aLevel; // 1st level
    OUString aSuffix = std::find_if(aLevel.begin(), aLevel.end(), [](const beans::PropertyValue& rValue) { return rValue.Name == "Suffix"; })->Value.get<OUString>();
    // Make sure it's empty as the source document contains <w:suff w:val="nothing"/>.
    CPPUNIT_ASSERT(aSuffix.isEmpty());
}

DECLARE_OOXMLIMPORT_TEST(testTdf90153, "tdf90153.docx")
{
    // This was at-para, so the line-level VertOrientRelation was lost, resulting in an incorrect vertical position.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf93919, "tdf93919.docx")
{
    // This was 0, left margin was not inherited from the list style.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}
DECLARE_OOXMLIMPORT_TEST(testTdf91417, "tdf91417.docx")
{
    // The first paragraph should contain a link to "http://www.google.com/"
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xTextCursor(xTextDocument->getText()->createTextCursor( ), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCursorProps(xTextCursor, uno::UNO_QUERY);
    OUString aValue;
    xCursorProps->getPropertyValue("HyperLinkURL") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("http://www.google.com/"), aValue);
}

DECLARE_OOXMLIMPORT_TEST(testTdf90810, "tdf90810short.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XFootnotesSupplier> xFootnoteSupp(xTextDocument, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnoteIdxAcc(xFootnoteSupp->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xFootnote(xFootnoteIdxAcc->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xFootnoteText(xFootnote, uno::UNO_QUERY);
    rtl::OUString sFootnoteText = xFootnoteText->getString();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(90), static_cast<sal_Int32>(sFootnoteText.getLength()));
}

DECLARE_OOXMLIMPORT_TEST(testTdf89165, "tdf89165.docx")
{
    // This must not hang in layout
}

DECLARE_OOXMLIMPORT_TEST(testTdf95777, "tdf95777.docx")
{
    // This must not fail on open
}

DECLARE_OOXMLIMPORT_TEST(testTdf94374, "hello.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    // This failed: it wasn't possible to insert a DOCX document into an existing Writer one.
    CPPUNIT_ASSERT(paste("tdf94374.docx", xEnd));
}

DECLARE_OOXMLIMPORT_TEST(testTdf83300, "tdf83300.docx")
{
    // This was 'Contents Heading', which (in the original document) implied 'keep with next' on unexpected paragraphs.
    CPPUNIT_ASSERT_EQUAL(OUString("TOC Heading"), getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf85232, "tdf85232.docx")
{
    uno::Reference<drawing::XShapes> xShapes(getShapeByName("Group 219"), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xShapes->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShape, uno::UNO_QUERY);
    // Make sure we're not testing the ellipse child.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.LineShape"), xShapeDescriptor->getShapeType());
    // This was 2900: horizontal position of the line was incorrect, the 3 children were not connected visually.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2267), xShape->getPosition().X);
}

DECLARE_OOXMLIMPORT_TEST(testTdf78902, "tdf78902.docx")
{
    // This hung in layout.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLIMPORT_TEST(testTdf95755, "tdf95755.docx")
{
    /*
    * The problem was that the width of a second table with single cell was discarded
    * and resulted in too wide table
    */
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Any aValue = xTableProperties->getPropertyValue("Width");
    sal_Int32 nWidth;
    aValue >>= nWidth;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10659), nWidth);
}

DECLARE_OOXMLIMPORT_TEST(testTdf95775, "tdf95775.docx")
{
    // This must not fail in layout
}

DECLARE_OOXMLIMPORT_TEST(testTdf92157, "tdf92157.docx")
{
    // A graphic with dimensions 0,0 should not fail on load
}

DECLARE_OOXMLIMPORT_TEST(testTdf60351, "tdf60351.docx")
{
    // Get the first image in the document and check its contour polygon.
    // It should contain 6 points. Check their coordinates.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    css::drawing::PointSequenceSequence aPolyPolygon;
    xPropertySet->getPropertyValue("ContourPolyPolygon") >>= aPolyPolygon;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aPolyPolygon.getLength());
    const css::drawing::PointSequence& aPolygon = aPolyPolygon[0];
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6),   aPolygon.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[0].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[0].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(316), aPolygon[1].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[1].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(316), aPolygon[2].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(316), aPolygon[2].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(158), aPolygon[3].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(298), aPolygon[3].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[4].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(316), aPolygon[4].Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[5].X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),   aPolygon[5].Y);
}

DECLARE_OOXMLIMPORT_TEST(testTdf97417, "section_break_numbering.docx")
{
    // paragraph with numbering and section break was removed by writerfilter
    // but its numbering was copied to all following paragraphs
    CPPUNIT_ASSERT_MESSAGE("first paragraph missing numbering",
        getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules").is());
    uno::Reference<beans::XPropertySet> const xProps(getParagraph(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("second paragraph erroneous numbering",
        !xProps->getPropertyValue("NumberingRules").hasValue());

}

DECLARE_OOXMLIMPORT_TEST(testTdf95970, "tdf95970.docx")
{
    // First shape: the rotation should be -12.94 deg, it should be mirrored.
    // Proper color order of image on test doc (left->right):
    // top row: green->red
    // bottom row: yellow->blue
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY_THROW);
    sal_Int32 aRotate = 0;
    xPropertySet->getPropertyValue("RotateAngle") >>= aRotate;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(34706), aRotate);
    bool bIsMirrored = false;
    xPropertySet->getPropertyValue("IsMirrored") >>= bIsMirrored;
    CPPUNIT_ASSERT(bIsMirrored);
    drawing::HomogenMatrix3 aTransform;
    xPropertySet->getPropertyValue("Transformation") >>= aTransform;
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line1.Column1,  4767.0507250872988));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line1.Column2, -1269.0985325236848));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line1.Column3,  696.73611111111109));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line2.Column1,  1095.3035265135941));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line2.Column2,  5523.4525711162969));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line2.Column3,  672.04166666666663));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line3.Column1,  0.0));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line3.Column2,  0.0));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aTransform.Line3.Column3,  1.0));
}

DECLARE_OOXMLIMPORT_TEST(testTdf94043, "tdf94043.docx")
{
    auto xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(2), "TextSection");
    auto xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    // This was 0, the separator line was not visible due to 0 width.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), getProperty<sal_Int32>(xTextColumns, "SeparatorLineWidth"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf96674, "tdf96674.docx")
{
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    awt::Size aActualSize(xShape->getSize());
    // This was 3493: the vertical line was horizontal.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aActualSize.Width);
    CPPUNIT_ASSERT(aActualSize.Height > 0);
}

DECLARE_OOXMLIMPORT_TEST(testTdf92045, "tdf92045.docx")
{
    // This was true, <w:effect w:val="none"/> resulted in setting the blinking font effect.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getRun(getParagraph(1), 1), "CharFlash"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf95213, "tdf95213.docx")
{
    // Get the second paragraph's numbering style's 2nd level's character style name.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(2);
    auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, "NumberingRules");
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(1) >>= aLevel; // 2nd level
    OUString aName = std::find_if(aLevel.begin(), aLevel.end(), [](const beans::PropertyValue& rValue) { return rValue.Name == "CharStyleName"; })->Value.get<OUString>();

    uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName(aName), uno::UNO_QUERY);
    // This was awt::FontWeight::BOLD.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xStyle, "CharWeight"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf97371, "tdf97371.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pShape = pPage->GetObj(0);
    SdrObject* pTextBox = pPage->GetObj(1);
    long nDiff = std::abs(pShape->GetSnapRect().Top() - pTextBox->GetSnapRect().Top());
    // The top of the two shapes were 410 and 3951, now it should be 3950 and 3951.
    CPPUNIT_ASSERT(nDiff < 10);
}

// base class to supply a helper method for testHFLinkToPrev
class testHFBase : public Test
{
protected:
    OUString
    getHFText(const uno::Reference<style::XStyle>& xPageStyle,
              const OUString &sPropName)
    {
        auto xTextRange = getProperty< uno::Reference<text::XTextRange> >(
            xPageStyle, sPropName);
        return xTextRange->getString();
    }
};

DECLARE_SW_IMPORT_TEST(testHFLinkToPrev, "headerfooter-link-to-prev.docx",
    testHFBase)
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");

    // get a page cursor
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    // get LO page style for page 1, corresponding to docx section 1 first page
    xCursor->jumpToFirstPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    uno::Reference<style::XStyle> xPageStyle(
        xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    // check page 1 header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("First page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("First page footer for section 1 only"));

    // get LO page style for page 2, corresponding to docx section 1
    xCursor->jumpToPage(2);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set( xPageStyles->getByName(pageStyleName), uno::UNO_QUERY );
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderTextLeft"),
        OUString("Even page header for section 1 only"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterTextLeft"),
        OUString("Even page footer for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("Odd page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("Odd page footer for section 1 only"));

    // get LO page style for page 4, corresponding to docx section 2 first page
    xCursor->jumpToPage(4);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set( xPageStyles->getByName(pageStyleName), uno::UNO_QUERY );
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("First page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("First page footer for sections 2 and 3 only"));

    // get LO page style for page 5, corresponding to docx section 2
    xCursor->jumpToPage(5);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set( xPageStyles->getByName(pageStyleName), uno::UNO_QUERY );
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderTextLeft"),
        OUString("Even page header for sections 2 and 3 only"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterTextLeft"),
        OUString("Even page footer for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("Odd page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("Odd page footer for sections 2 and 3 only"));

    // get LO page style for page 7, corresponding to docx section 3 first page
    xCursor->jumpToPage(7);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set( xPageStyles->getByName(pageStyleName), uno::UNO_QUERY );
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("First page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("First page footer for sections 2 and 3 only"));

    // get LO page style for page 8, corresponding to docx section 3
    xCursor->jumpToPage(8);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set( xPageStyles->getByName(pageStyleName), uno::UNO_QUERY );
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderTextLeft"),
        OUString("Even page header for sections 2 and 3 only"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterTextLeft"),
        OUString("Even page footer for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("Odd page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("Odd page footer for sections 2 and 3 only"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf98882, "tdf98882.docx")
{
    sal_Int32 nFlyHeight = parseDump("//fly/infos/bounds", "height").toInt32();
    sal_Int32 nContentHeight = parseDump("//notxt/infos/bounds", "height").toInt32();
    // The content height was 600, not 360, so the frame and the content height did not match.
    CPPUNIT_ASSERT_EQUAL(nFlyHeight, nContentHeight);
}

DECLARE_OOXMLIMPORT_TEST(testTdf99074, "tdf99074.docx")
{
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<uno::XInterface> xSettings = xFactory->createInstance("com.sun.star.document.Settings");
    // This was false, Web Layout was ignored on import.
    CPPUNIT_ASSERT(getProperty<bool>(xSettings, "InBrowseMode"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf99140, "tdf99140.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xTextDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1: a multi-page floating table was imported as a TextFrame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(1), uno::UNO_QUERY);
    // This was text::HoriOrientation::NONE, the second table was too wide due to this.
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::LEFT_AND_WIDTH, getProperty<sal_Int16>(xTableProperties, "HoriOrient"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
