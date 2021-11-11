/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <string_view>

#ifdef MACOSX
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <premac.h>
#include <AppKit/AppKit.h>
#include <postmac.h>
#endif

#include <swmodeltestbase.hxx>

#include <IDocumentSettingAccess.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/text/XNumberingRulesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>

#include <tools/UnitConversion.hxx>
#include <unotools/fltrcfg.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <xmloff/odffields.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <IDocumentMarkAccess.hxx>
#include <IMark.hxx>
#include <bordertest.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

    virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        if (filename == std::string_view("smartart.docx")
            || filename == std::string_view("strict-smartart.docx") )
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
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testN751054, "n751054.docx")
{
    text::TextContentAnchorType eValue = getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType");
    CPPUNIT_ASSERT(eValue != text::TextContentAnchorType_AS_CHARACTER);
}

DECLARE_OOXMLEXPORT_TEST(testTdf48569, "tdf48569.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // File crashing while saving in LO
    text::TextContentAnchorType eValue = getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType");
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, eValue);
}

DECLARE_OOXMLEXPORT_TEST(testN750935, "n750935.docx")
{
    // Some page break types were ignores, resulting in less pages.
    CPPUNIT_ASSERT_EQUAL(5, getPages());

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

DECLARE_OOXMLEXPORT_TEST(testN751117, "n751117.docx")
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

DECLARE_OOXMLEXPORT_TEST(testFdo74745, "fdo74745.docx")
{
    uno::Reference<text::XTextRange > paragraph = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(OUString("09/02/14"), paragraph->getString());
}

DECLARE_OOXMLEXPORT_TEST(testFdo81486, "fdo81486.docx")
{
    uno::Reference<text::XTextRange > paragraph = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("CustomTitle"), paragraph->getString());
}

DECLARE_OOXMLEXPORT_TEST(testFdo79738, "fdo79738.docx")
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

DECLARE_OOXMLEXPORT_TEST(testN705956_1, "n705956-1.docx")
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
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
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

DECLARE_OOXMLEXPORT_TEST(testN705956_2, "n705956-2.docx")
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

DECLARE_OOXMLEXPORT_TEST(testN747461, "n747461.docx")
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

DECLARE_OOXMLEXPORT_TEST(testN750255, "n750255.docx")
{

/*
Column break without columns on the page is a page break, so check those paragraphs
are on page 2 and page 3
*/
    CPPUNIT_ASSERT_EQUAL( OUString("one"), parseDump("/root/page[2]/body/txt/text()") );
    CPPUNIT_ASSERT_EQUAL( OUString("two"), parseDump("/root/page[3]/body/txt/text()") );
}

DECLARE_OOXMLEXPORT_TEST(testN652364, "n652364.docx")
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

DECLARE_OOXMLEXPORT_TEST(testN764005, "n764005.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);

    // The picture in the header wasn't absolutely positioned and wasn't in the background.
    text::TextContentAnchorType eValue;
    xPropertySet->getPropertyValue("AnchorType") >>= eValue;
    CPPUNIT_ASSERT(eValue != text::TextContentAnchorType_AS_CHARACTER);
    bool bValue = true;
    xPropertySet->getPropertyValue("Opaque") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(false, bValue);
}

DECLARE_OOXMLEXPORT_TEST(testN766481, "n766481.docx")
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

DECLARE_OOXMLEXPORT_TEST(testN766487, "n766487.docx")
{
    /*
     * The problem was that 1) the font size of the first para was too large 2) numbering had no first-line-indent.
     *
     * oParas = ThisComponent.Text.createEnumeration
     * oPara = oParas.nextElement
     * oRuns = oPara.createEnumeration
     * oRun = oRuns.nextElement
     * xray oRun.CharHeight ' 11, was larger
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
    CPPUNIT_ASSERT_EQUAL(11.f, fValue);

    xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("ParaFirstLineIndent") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(-360)), nValue);
}

DECLARE_OOXMLEXPORT_TEST(testN693238, "n693238.docx")
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

DECLARE_OOXMLEXPORT_TEST(testNumbering1, "numbering1.docx")
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
    uno::Reference< text::XTextRange > paragraph(getParagraph( 1, "Text1." ));
    OUString numberingStyleName = getProperty< OUString >( paragraph, "NumberingStyleName" );
    uno::Reference<text::XNumberingRulesSupplier> xNumberingRulesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> numberingRules = xNumberingRulesSupplier->getNumberingRules();
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

DECLARE_OOXMLEXPORT_TEST(testAllGapsWord, "all_gaps_word.docx")
{
    BorderTest borderTest;
    BorderTest::testTheBorders(mxComponent, false);
}

DECLARE_OOXMLEXPORT_TEST(testN775906, "n775906.docx")
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

DECLARE_OOXMLEXPORT_TEST(testTdf59699, "tdf59699.docx")
{
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, "Graphic");
    // This was false: the referenced graphic data wasn't imported.
    CPPUNIT_ASSERT(xGraphic.is());
}

DECLARE_OOXMLEXPORT_TEST(testN777337, "n777337.docx")
{
     // The problem was that the top and bottom margin on the first page was only 0.1cm instead of 1.7cm.
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1702), getProperty<sal_Int32>(xPropertySet, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1702), getProperty<sal_Int32>(xPropertySet, "BottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testN778836, "n778836.docx")
{
    /*
     * The problem was that the paragraph inherited margins from the numbering
     * and parent paragraph styles and the result was incorrect.
     */
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1270), getProperty<sal_Int32>(getParagraph(1), "ParaRightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3810), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-635), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
}

DECLARE_OOXMLEXPORT_TEST(testN778828, "n778828.docx")
{
    /*
     * The problem was that a page break after a continuous section break caused
     * double page break on title page.
     */
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf106724, "tdf106724.docx")
{
    // This document simply crashed the importer.
}

DECLARE_OOXMLEXPORT_TEST(testN779834, "n779834.docx")
{
    // This document simply crashed the importer.
}

DECLARE_OOXMLEXPORT_TEST(testRHBZ1180114, "rhbz1180114.docx")
{
    // This document simply crashed the importer.
}

DECLARE_OOXMLEXPORT_TEST(testTdf66496, "tdf66496.docx")
{
    // This document simply crashed the importer.
}

DECLARE_OOXMLEXPORT_TEST(testTDF91122, "tdf91122.docx")
{
    /*
     * OLE object shape: default vertical position is top in MSO, not bottom
     */
    {   // Check first shape
        uno::Reference<beans::XPropertySet> xShapeProperties( getShape(1), uno::UNO_QUERY );
        uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShapeProperties, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), xShapeDescriptor->getShapeType());
        sal_Int16 nValue;
        xShapeProperties->getPropertyValue("VertOrient") >>= nValue;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical orientation", text::VertOrientation::TOP, nValue);
    }

    {   // Check second shape
        uno::Reference<beans::XPropertySet> xShapeProperties( getShape(2), uno::UNO_QUERY );
        uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShapeProperties, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), xShapeDescriptor->getShapeType());
        sal_Int16 nValue;
        xShapeProperties->getPropertyValue("VertOrient") >>= nValue;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical orientation", text::VertOrientation::TOP, nValue);
    }
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testFdo76803, "fdo76803.docx")
{
    // The ContourPolyPolygon was wrong
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);

    drawing::PointSequenceSequence rContour = getProperty<drawing::PointSequenceSequence>(xPropertySet, "ContourPolyPolygon");
    basegfx::B2DPolyPolygon aPolyPolygon(basegfx::utils::UnoPointSequenceSequenceToB2DPolyPolygon(rContour));

    // We've got exactly one polygon inside
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), aPolyPolygon.count());

    // Now check it deeply
    basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(0));

    CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), aPolygon.count());

    CPPUNIT_ASSERT_EQUAL(double(-149), aPolygon.getB2DPoint(0).getX());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -35
    // - Actual  : -67
    // i.e. the cropping did not influence the wrap polygon during export.
    CPPUNIT_ASSERT_EQUAL(double(-35), aPolygon.getB2DPoint(0).getY());

    CPPUNIT_ASSERT_EQUAL(double(-149), aPolygon.getB2DPoint(1).getX());
    CPPUNIT_ASSERT_EQUAL(double(3511), aPolygon.getB2DPoint(1).getY());

    CPPUNIT_ASSERT_EQUAL(double(16889), aPolygon.getB2DPoint(2).getX());
    CPPUNIT_ASSERT_EQUAL(double(3511), aPolygon.getB2DPoint(2).getY());

    CPPUNIT_ASSERT_EQUAL(double(16889), aPolygon.getB2DPoint(3).getX());
    CPPUNIT_ASSERT_EQUAL(double(-35), aPolygon.getB2DPoint(3).getY());
}

DECLARE_OOXMLEXPORT_TEST(testTDF91260, "tdf91260.docx")
{
    uno::Reference<text::XTextRange> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame->getString().startsWith( "Lorem ipsum" ) );

    //Check the textbox doesn't get shrunk
    CPPUNIT_ASSERT_GREATER(sal_Int32(10900), getProperty<sal_Int32>(xFrame, "Height"));

    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5450), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21946), xShape->getPosition().Y);
}

DECLARE_OOXMLEXPORT_TEST(testFdo74357, "fdo74357.docx")
{
    // Floating table wasn't converted to a textframe.
    // This was 0.
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    // Bottom margin of the first paragraph was too large, causing a layout problem.
    // This was 494.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(86), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo55187, "fdo55187.docx")
{
    // 0x010d was imported as a newline.
    getParagraph(1, OUString(u"lup\u010Dka"));
}

DECLARE_OOXMLEXPORT_TEST(testN780563, "n780563.docx")
{
    /*
     * Make sure we have the table in the fly frame created
     */
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount( ));
}

DECLARE_OOXMLEXPORT_TEST(testN780853, "n780853.docx")
{
    /*
     * The problem was that the table was not imported.
     *
     * xray ThisComponent.TextTables.Count 'was 0
     */
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    //tdf#102619 - I would have expected this to be "Standard", but MSO 2013/2010/2003 all give FollowStyle==Date
    uno::Reference< beans::XPropertySet > properties(getStyles("ParagraphStyles")->getByName("Date"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Date"), getProperty<OUString>(properties, "FollowStyle"));
}

DECLARE_OOXMLEXPORT_TEST(testN780843, "n780843.docx")
{
    CPPUNIT_ASSERT_EQUAL(OUString("shown footer"), parseDump("/root/page[2]/footer/txt/text()"));

    //tdf64372 this document should only have one page break (2 pages, not 3)
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testN780843b, "n780843b.docx")
{
    // Same document as testN780843 except there is more text before the continuous break. Now the opposite footer results should happen.
    uno::Reference< text::XTextRange > xPara = getParagraph(3);
    OUString aStyleName = getProperty<OUString>(xPara, "PageStyleName");
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(aStyleName), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFooterText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "FooterText");
    CPPUNIT_ASSERT_EQUAL( OUString("hidden footer"), xFooterText->getString() );

    CPPUNIT_ASSERT_EQUAL( 7, getParagraphs() );
}

DECLARE_OOXMLEXPORT_TEST(testShadow, "imgshadow.docx")
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

DECLARE_OOXMLEXPORT_TEST(testN782345, "n782345.docx")
{
    /*
     * The problem was that the page break was inserted before the 3rd para, instead of before the 2nd para.
     */
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE, getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

DECLARE_OOXMLEXPORT_TEST(testN779941, "n779941.docx")
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

DECLARE_OOXMLEXPORT_TEST(testN783638, "n783638.docx")
{
    // The problem was that the margins of inline images were not zero.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPropertySet, "LeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo52208, "fdo52208.docx")
{
    // The problem was that the document had 2 pages instead of 1.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testN785767, "n785767.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    // Check the A1 and B1 cells, the width of both of them was the default value (10000 / 9, as there were 9 cells in the row).
    CPPUNIT_ASSERT_MESSAGE("A1 must not have default width", sal_Int16(10000 / 9) != getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators")[0].Position);
    CPPUNIT_ASSERT_MESSAGE("B1 must not have default width", sal_Int16(10000 / 9) != getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[0].Position);
}

DECLARE_OOXMLEXPORT_TEST(testFineTableDash, "tableborder-finedash.docx")
{
    // The problem was that finely dashed borders on tables were unsupported
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(0), uno::UNO_QUERY);
    table::TableBorder2 aBorder;
    xTableProperties->getPropertyValue("TableBorder2") >>= aBorder;
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::FINE_DASHED, aBorder.RightLine.LineStyle);
}

DECLARE_OOXMLEXPORT_TEST(testN792778, "n792778.docx")
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
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
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
//FIXME:
//DECLARE_OOXMLEXPORT_TEST(testGroupshapeSmarttag, "groupshape-smarttag.docx")
//{
//    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
//    uno::Reference<text::XTextRange> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);
//    // First run of shape text was missing due to the w:smartTag wrapper around it.
//    CPPUNIT_ASSERT_EQUAL(OUString("Box 2"), xShape->getString());
//
//    // Font size of the shape text was 10.
//    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(getParagraphOfText(1, xShape->getText()), "CharHeight"));
//}

DECLARE_OOXMLEXPORT_TEST(testN793262, "n793262.docx")
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

DECLARE_OOXMLEXPORT_TEST(testN793998, "n793998.docx")
{
    sal_Int32 nTextPortion = parseDump("/root/page/body/txt/Text[1]", "nWidth").toInt32(); // Width of the first (text) portion
    sal_Int32 nTabPortion = parseDump("/root/page/body/txt/Text[2]", "nWidth").toInt32(); // Width of the second (tab) portion
    sal_Int32 nParagraph = parseDump("/root/page/body/txt/infos/bounds", "width").toInt32(); // Width of the paragraph
    sal_Int32 const nRightMargin = 3000;
    // The problem was that the tab portion didn't ignore the right margin, so text + tab width wasn't larger than body (paragraph - right margin) width.
    CPPUNIT_ASSERT(nTextPortion + nTabPortion > nParagraph - nRightMargin);
}

DECLARE_OOXMLEXPORT_TEST(testN779642, "n779642.docx")
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical orientation", text::VertOrientation::BOTTOM, nValue);
    xFrame->getPropertyValue("VertOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical orientation relation", text::RelOrientation::PAGE_PRINT_AREA, nValue);

    // tdf#106572 - perhaps not the best test to hijack since this file
    // produces an error in Word, but it nicely matches danger points,
    // and has a different first footer, so nice visual confirmation.
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // There is no footer text on the first page.
    assertXPath(pXmlDoc, "/root/page[1]/footer/txt", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTbLrHeight, "tblr-height.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("B1");
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, getProperty<sal_Int16>(xCell, "WritingMode"));
}

DECLARE_OOXMLEXPORT_TEST(testBnc865381, "bnc865381.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTextTable->getCellByName("A2");
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, getProperty<sal_Int16>(xCell, "WritingMode"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo53985, "fdo53985.docx")
{
    // Unhandled exception prevented import of the rest of the document.

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTables->getCount()); // Only 4 tables were imported.

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT_MESSAGE("Compatibility: Protect form", pDoc->getIDocumentSettingAccess().get( DocumentSettingId::PROTECT_FORM ) );

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xSections->getCount()); // The first paragraph wasn't counted as a section.

    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TextSection is protected", true, getProperty<bool>(xSect, "IsProtected"));
    xSect.set(xSections->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section3 is protected", false, getProperty<bool>(xSect, "IsProtected"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo59638, "fdo59638.docx")
{
    // The problem was that w:lvlOverride inside w:num was ignores by dmapper.

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (beans::PropertyValue const & rProp : std::as_const(aProps))
    {
        if (rProp.Name == "BulletChar")
        {
            // Was '*', should be 'o'.
            CPPUNIT_ASSERT_EQUAL(OUString(u"\uF0B7"), rProp.Value.get<OUString>());
            return;
        }
    }
    CPPUNIT_FAIL("no BulletChar property");
}

DECLARE_OOXMLEXPORT_TEST(testFdo61343, "fdo61343.docx")
{
    // The problem was that there were a groupshape in the doc, followed by an
    // OLE object, and this lead to a crash.
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

DECLARE_OOXMLEXPORT_TEST(testToolsLineNumbering, "tools-line-numbering.docx")
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

DECLARE_OOXMLEXPORT_TEST(testfdo78904, "fdo78904.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    if (xIndexAccess->getCount())
    {
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(0), getProperty<sal_Int32>(xFrame, "HoriOrientPosition"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testFdo60922, "fdo60922.docx")
{
    // This was 0, not 100, due to wrong import of w:position w:val="0"
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(getRun(getParagraph(1), 1), "CharEscapementHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo59273, "fdo59273.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Was 115596 (i.e. 10 times wider than necessary), as w:tblW was missing and the importer didn't set it.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12963), getProperty<sal_Int32>(xTextTable, "Width"));

    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    // Was 9997, so the 4th column had ~zero width
    CPPUNIT_ASSERT_LESSEQUAL(2, (sal_Int16(7500)
                                 - getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                       xTableRows->getByIndex(0), "TableColumnSeparators")[2]
                                       .Position));
}

DECLARE_OOXMLEXPORT_TEST(testConditionalstylesTablelook, "conditionalstyles-tbllook.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Background was -1.
    CPPUNIT_ASSERT_EQUAL(Color(0x7F7F7F), Color(ColorTransparency, getProperty<sal_Int32>(xTable->getCellByName("A1"), "BackColor")));
}

DECLARE_OOXMLEXPORT_TEST(testFdo63685, "fdo63685.docx")
{
    // An inline image's wrapping should be always zero, even if the doc model has a non-zero value.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getShape(1), "TopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testN592908_Frame, "n592908-frame.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    text::WrapTextMode eValue;
    xPropertySet->getPropertyValue("Surround") >>= eValue;
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_PARALLEL, eValue);
}

DECLARE_OOXMLEXPORT_TEST(testN592908_Picture, "n592908-picture.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    text::WrapTextMode eValue;
    xPropertySet->getPropertyValue("Surround") >>= eValue;
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_PARALLEL, eValue);
}

DECLARE_OOXMLEXPORT_TEST(testN779630, "n779630.docx")
{
    // A combo box is imported
    if (getShapes() > 0)
    {
        uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
        uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.ComboBox")));
        CPPUNIT_ASSERT_EQUAL(OUString("dropdown default text"), getProperty<OUString>(xPropertySet, "DefaultText"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty< uno::Sequence<OUString> >(xPropertySet, "StringItemList").getLength());
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPropertySet, "Dropdown"));
    }
    else
    {
        // ComboBox was imported as DropDown text field
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        CPPUNIT_ASSERT(xFields->hasMoreElements());
        uno::Any aField = xFields->nextElement();
        uno::Reference<lang::XServiceInfo> xServiceInfo(aField, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.textfield.DropDown"));

        uno::Sequence<OUString> aItems = getProperty< uno::Sequence<OUString> >(aField, "Items");
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aItems.getLength());
        CPPUNIT_ASSERT_EQUAL(OUString("Yes"), aItems[0]);
        CPPUNIT_ASSERT_EQUAL(OUString("No"), aItems[1]);
        CPPUNIT_ASSERT_EQUAL(OUString("dropdown default text"), aItems[2]);
    }
}

DECLARE_OOXMLEXPORT_TEST(testIndentation, "indentation.docx")
{
    uno::Reference<uno::XInterface> xParaLTRTitle(getParagraph( 1, "Title aligned"));
    uno::Reference<uno::XInterface> xParaLTRNormal(getParagraph( 2, ""));

    // this will test the text direction for paragraphs
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, getProperty<sal_Int16>( xParaLTRTitle, "WritingMode" ));
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, getProperty<sal_Int16>( xParaLTRNormal, "WritingMode" ));
}

DECLARE_OOXMLEXPORT_TEST(testPageBorderShadow, "page-border-shadow.docx")
{
    // The problem was that in w:pgBorders, child elements had a w:shadow attribute, but that was ignored.
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(getStyles("PageStyles")->getByName("Standard"), "ShadowFormat");
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aShadow.Color));
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
    // w:sz="48" is in eights of a point, 1 pt is 20 twips.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(convertTwipToMm100(48/8*20)), aShadow.ShadowWidth);
}

DECLARE_OOXMLEXPORT_TEST(testN816593, "n816593.docx")
{
    // Two consecutive <w:tbl> without any paragraph in between, but with different tblpPr. In this
    // case we need to have 2 different tables instead of 1
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testN820509, "n820509.docx")
{
    // M.d.yyyy date format was unhandled.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMarkAccess->getAllMarksCount());

    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*pMarkAccess->getAllMarksBegin());

    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    const sw::mark::IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
    OUString sDateFormat;
    auto pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sDateFormat;
    }
    CPPUNIT_ASSERT_EQUAL(OUString("M.d.yyyy"), sDateFormat);
}

DECLARE_OOXMLEXPORT_TEST(testN830205, "n830205.docx")
{
    // Previously import just crashed (due to infinite recursion).
    getParagraph(1, "XXX");
}

DECLARE_OOXMLEXPORT_TEST(tdf123705, "tdf123705.docx")
{
}

DECLARE_OOXMLEXPORT_TEST(testTableAutoColumnFixedSize, "table-auto-column-fixed-size.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Width was not recognized during import when table size was 'auto'
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(3996)), getProperty<sal_Int32>(xTextTable, "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testTableAutoColumnFixedSize2, "table-auto-column-fixed-size2.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was 17907, i.e. the sum of the width of the 3 cells (10152 twips each), which is too wide.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16891), getProperty<sal_Int32>(xTextTable, "Width"));
}
#if 0
// FIXME:
DECLARE_OOXMLEXPORT_TEST(testFdo46361, "fdo46361.docx")
{
    uno::Reference<container::XIndexAccess> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroupShape->getByIndex(0), uno::UNO_QUERY);
    // This was CENTER.
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP, getProperty<drawing::TextVerticalAdjust>(xShape, "TextVerticalAdjust"));
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    // This was LEFT.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xParagraph, "ParaAdjust")));
    // This was black, not green.
    CPPUNIT_ASSERT_EQUAL(Color(0x008000), Color(ColorTransparency, getProperty<sal_Int32>(getRun(xParagraph, 1), "CharColor")));
    // \n char was missing due to unhandled w:br.
    uno::Reference<text::XText> xShapeText(xGroupShape->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("text\ntext"), xShapeText->getString());
    // \n chars were missing, due to unhandled multiple w:p tags.
    xShapeText.set(xGroupShape->getByIndex(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("text\ntext\n"), xShapeText->getString());

    // tdf#128153 The first and second lines are directly specified as centered. Make sure that doesn't change.
    xParagraph.set(getParagraphOfText(2, xShapeText->getText(), "text"));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xParagraph, "ParaAdjust")));
    // The last paragraph should be left aligned.
    xParagraph.set(getParagraphOfText(3, xShapeText->getText(), ""));
    CPPUNIT_ASSERT_MESSAGE("You FIXED me!", style::ParagraphAdjust_LEFT != static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xParagraph, "ParaAdjust")));
}
#endif
DECLARE_OOXMLEXPORT_TEST(testFdo65632, "fdo65632.docx")
{
    // The problem was that the footnote text had fake redline: only the body
    // text has redline in fact.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XText> xText(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    //uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), getProperty<OUString>(getRun(getParagraphOfText(1, xText), 1), "TextPortionType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66474, "fdo66474.docx")
{
    // The table width was too small, so the text in the second cell was unreadable: this was 1397.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10492), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeRotation, "groupshape-rotation.docx")
{
    // Rotation on groupshapes wasn't handled at all by the VML importer.
    // Note: the shapes are still shifting on the page, so the rotation drifts after multiple round-trips.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(31500.0, getProperty<double>(getShape(1), "RotateAngle"), 100);
}

DECLARE_OOXMLEXPORT_TEST(testBnc780044Spacing, "bnc780044_spacing.docx")
{
    // The document has global w:spacing in styles.xml , and local w:spacing in w:pPr, which however
    // only applied to text runs, not to as-character pictures. So the picture made the line higher.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTableAutoNested, "table-auto-nested.docx")
{
    // This was 176, when compat option is not enabled, the auto paragraph bottom margin value was incorrect.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(494), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // This was 115596, i.e. the width of the outer table was too large.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(23051), getProperty<sal_Int32>(xTables->getByIndex(1), "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testTableStyleParprop, "table-style-parprop.docx")
{
    // The problem was that w:spacing's w:after=0 (a paragraph property) wasn't imported from table style.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 353, the document default, i.e. paragraph property from table style had no effect.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTablePagebreak, "table-pagebreak.docx")
{
    // Page break inside table: should be ignored (was style::BreakType_PAGE_BEFORE before).
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE, getProperty<style::BreakType>(getParagraphOrTable(2), "BreakType"));

    // This one is outside the table: should not be ignored.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE, getProperty<style::BreakType>(getParagraph(3), "BreakType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo68607, "fdo68607.docx")
{
    // Bugdoc was 8 pages in Word, 1 in Writer due to pointlessly wrapping the
    // table in a frame. Exact layout may depend on fonts available, etc. --
    // but at least make sure that our table spans over multiple pages now.
    CPPUNIT_ASSERT(getPages() > 1);
}

DECLARE_OOXMLEXPORT_TEST(testVmlTextVerticalAdjust, "vml-text-vertical-adjust.docx")
{
    uno::Reference<drawing::XShapes> xOuterGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xInnerGroupShape(xOuterGroupShape->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xInnerGroupShape->getByIndex(0), uno::UNO_QUERY);
    // Was CENTER.
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP, getProperty<drawing::TextVerticalAdjust>(xShape, "TextVerticalAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo69636, "fdo69636.docx")
{
    // The problem was that the mso-layout-flow-alt:bottom-to-top VML shape property wasn't handled for sw text frames.
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aCustomShapeGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-270), aCustomShapeGeometry["TextPreRotateAngle"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testChartProp, "chart-prop.docx")
{
    // The problem was that chart was not getting parsed in writer module.
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15240), getProperty<sal_Int32>(xPropertySet, "Width"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8890), getProperty<sal_Int32>(xPropertySet, "Height"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo43093, "fdo43093b.docx")
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
