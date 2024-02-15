/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/unoapi_test.hxx>

#include <string_view>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <officecfg/Office/Common.hxx>
#include <rtl/math.hxx>
#include <svx/svdoashp.hxx>
#include <tools/color.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>
#include <basegfx/utils/gradienttools.hxx>

using namespace ::com::sun::star;

namespace
{
/// Gets one child of xShape, which one is specified by nIndex.
uno::Reference<drawing::XShape> getChildShape(const uno::Reference<drawing::XShape>& xShape,
                                              sal_Int32 nIndex)
{
    uno::Reference<container::XIndexAccess> xGroup(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    CPPUNIT_ASSERT(xGroup->getCount() > nIndex);

    uno::Reference<drawing::XShape> xRet(xGroup->getByIndex(nIndex), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xRet.is());

    return xRet;
}
}

/// oox shape tests.
class OoxShapeTest : public UnoApiTest
{
public:
    OoxShapeTest()
        : UnoApiTest("/oox/qa/unit/data/")
    {
    }
    uno::Reference<drawing::XShape> getShapeByName(std::u16string_view aName);
};

uno::Reference<drawing::XShape> OoxShapeTest::getShapeByName(std::u16string_view aName)
{
    uno::Reference<drawing::XShape> xRet;

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    for (sal_Int32 i = 0; i < xDrawPage->getCount(); ++i)
    {
        uno::Reference<container::XNamed> xShape(xDrawPage->getByIndex(i), uno::UNO_QUERY);
        if (xShape->getName() == aName)
        {
            xRet.set(xShape, uno::UNO_QUERY);
            break;
        }
    }

    return xRet;
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testGroupTransform)
{
    loadFromFile(u"tdf141463_GroupTransform.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(getChildShape(xGroup, 0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed in several properties.

    sal_Int32 nAngle;
    xPropSet->getPropertyValue("ShearAngle") >>= nAngle;
    // Failed with - Expected: 0
    //             - Actual  : -810
    // i.e. the shape was sheared although shearing does not exist in oox
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nAngle);

    xPropSet->getPropertyValue("RotateAngle") >>= nAngle;
    // Failed with - Expected: 26000 (is in 1/100deg)
    //             - Actual  : 26481 (is in 1/100deg)
    // 100deg in PowerPoint UI = 360deg - 100deg in LO.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(26000), nAngle);

    sal_Int32 nActual = xShape->getSize().Width;
    // The group has ext.cy=2880000 and chExt.cy=4320000 resulting in Y-scale=2/3.
    // The child has ext 2880000 x 1440000. Because of rotation angle 80deg, the Y-scale has to be
    // applied to the width, resulting in 2880000 * 2/3 = 1920000EMU = 5333Hmm
    // ToDo: Expected value currently 1 off.
    // Failed with - Expected: 5332
    //             - Actual  : 5432
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5332), nActual);
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testMultipleGroupShapes)
{
    loadFromFile(u"multiple-group-shapes.docx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the 2 group shapes from the document were imported as a single one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xDrawPage->getCount());
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testCustomshapePosition)
{
    loadFromFile(u"customshape-position.docx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    sal_Int32 nY{};
    xShape->getPropertyValue("VertOrientPosition") >>= nY;
    // <wp:posOffset>581025</wp:posOffset> in the document.
    sal_Int32 nExpected = rtl::math::round(581025.0 / 360);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1614
    // - Actual  : 0
    // i.e. the position of the shape was lost on import due to the rounded corners.
    CPPUNIT_ASSERT_EQUAL(nExpected, nY);
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testTdf125582_TextOnCircle)
{
    // The document contains a shape with a:prstTxWarp="textCircle" with two paragraphs.
    // PowerPoint aligns the bottom of the text with the path, LO had aligned the middle of the
    // text with the path, which resulted in smaller text.
    loadFromFile(u"tdf125582_TextOnCircle.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);

    // BoundRect of Fontwork shape depends on dpi.
    if (IsDefaultDPI())
    {
        SdrObjCustomShape& rSdrCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        // Without the fix in place width was 3639, but should be 4784 for 96dpi.
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(4784), aBoundRect.GetWidth(), 5);
    }

    drawing::TextVerticalAdjust eAdjust;
    xPropSet->getPropertyValue("TextVerticalAdjust") >>= eAdjust;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TextVerticalAdjust", drawing::TextVerticalAdjust_BOTTOM, eAdjust);
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testTdf151008VertAnchor)
{
    // The document contains shapes with all six kind of anchor positions in pptx. The text in the
    // shapes is larger than the shape and has no word wrap. That way anchor position is visible
    // in case you inspect the file manually.
    loadFromFile(u"tdf151008_eaVertAnchor.pptx");

    struct anchorDesc
    {
        OUString sShapeName;
        drawing::TextHorizontalAdjust eAnchorHori;
        drawing::TextVerticalAdjust eAnchorVert;
    };
    anchorDesc aExpected[6]
        = { { u"Right"_ustr, drawing::TextHorizontalAdjust_RIGHT, drawing::TextVerticalAdjust_TOP },
            { u"Center"_ustr, drawing::TextHorizontalAdjust_CENTER,
              drawing::TextVerticalAdjust_TOP },
            { u"Left"_ustr, drawing::TextHorizontalAdjust_LEFT, drawing::TextVerticalAdjust_TOP },
            { u"RightMiddle"_ustr, drawing::TextHorizontalAdjust_RIGHT,
              drawing::TextVerticalAdjust_CENTER },
            { u"CenterMiddle"_ustr, drawing::TextHorizontalAdjust_CENTER,
              drawing::TextVerticalAdjust_CENTER },
            { u"LeftMiddle"_ustr, drawing::TextHorizontalAdjust_LEFT,
              drawing::TextVerticalAdjust_CENTER } };
    // without the fix horizontal and vertical anchor positions were exchanged
    for (size_t i = 0; i < 6; ++i)
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeByName(aExpected[i].sShapeName),
                                                   uno::UNO_QUERY);
        drawing::TextHorizontalAdjust eHori;
        CPPUNIT_ASSERT(xShape->getPropertyValue("TextHorizontalAdjust") >>= eHori);
        drawing::TextVerticalAdjust eVert;
        CPPUNIT_ASSERT(xShape->getPropertyValue("TextVerticalAdjust") >>= eVert);
        CPPUNIT_ASSERT_EQUAL(aExpected[i].eAnchorHori, eHori);
        CPPUNIT_ASSERT_EQUAL(aExpected[i].eAnchorVert, eVert);
    }
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testTdf151518VertAnchor)
{
    // Make sure SmartArt is loaded as group shape
    bool bUseGroup = officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::get();
    if (!bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(true, pChange);
        pChange->commit();
    }

    // The document contains SmartArt with shapes with not default text area. Without fix the
    // text was shifted up because of wrong values in TextLowerDistance and TextUpperDistance.
    loadFromFile(u"tdf151518_SmartArtTextLocation.docx");

    struct TextDistance
    {
        OUString sShapeName;
        sal_Int16 nSubShapeIndex;
        sal_Int32 nLowerDistance;
        sal_Int32 nUpperDistance;
    };
    TextDistance aExpected[4] = { { u"Diagram Target List"_ustr, 2, 2979, 201 },
                                  { u"Diagram Nested Target"_ustr, 1, 3203, 127 },
                                  { u"Diagram Stacked Venn"_ustr, 1, 3112, -302 },
                                  { u"Diagram Grouped List"_ustr, 1, 4106, 196 } };
    // without the fix the observed distances were
    // {4434, -464}, {4674, -751}, {4620, -1399}, {6152, -744}
    for (size_t i = 0; i < 4; ++i)
    {
        uno::Reference<drawing::XShapes> xDiagramShape(getShapeByName(aExpected[i].sShapeName),
                                                       uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xShapeProps(
            xDiagramShape->getByIndex(aExpected[i].nSubShapeIndex), uno::UNO_QUERY);
        sal_Int32 nLower;
        sal_Int32 nUpper;
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue("TextLowerDistance") >>= nLower);
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue("TextUpperDistance") >>= nUpper);
        CPPUNIT_ASSERT_EQUAL(aExpected[i].nLowerDistance, nLower);
        CPPUNIT_ASSERT_EQUAL(aExpected[i].nUpperDistance, nUpper);
    }

    if (!bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(false, pChange);
        pChange->commit();
    }
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testTdf54095_SmartArtThemeTextColor)
{
    // The document contains a SmartArt where the color for the texts in the shapes is given by
    // the theme.
    // Error was, that the theme was not considered and therefore the text was white.

    // Make sure it is not loaded as metafile but with single shapes.
    bool bUseGroup = officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::get();
    if (!bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(true, pChange);
        pChange->commit();
    }

    // get SmartArt
    loadFromFile(u"tdf54095_SmartArtThemeTextColor.docx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xSmartArt(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // shape 0 is the background shape without text
    uno::Reference<text::XTextRange> xShape(xSmartArt->getByIndex(1), uno::UNO_QUERY);

    // text color
    uno::Reference<container::XEnumerationAccess> xText(xShape->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xPara(xText->createEnumeration()->nextElement(),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPortion(xPara->createEnumeration()->nextElement(),
                                                 uno::UNO_QUERY);
    Color nActualColor{ 0 };
    xPortion->getPropertyValue("CharColor") >>= nActualColor;
    // Without fix the test would have failed with:
    // - Expected: rgba[1f497dff]
    // - Actual  : rgba[ffffffff], that is text was white
    CPPUNIT_ASSERT_EQUAL(Color(0x1F497D), nActualColor);

    // clrScheme. For map between name in docx and index from CharComplexColor see
    // oox::drawingml::Color::getSchemeColorIndex()
    // Without fix the color scheme was "lt1" (1) but should be "dk2" (2).
    uno::Reference<util::XComplexColor> xComplexColor;
    xPortion->getPropertyValue("CharComplexColor") >>= xComplexColor;
    CPPUNIT_ASSERT(xComplexColor.is());
    auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Dark2, aComplexColor.getThemeColorType());

    if (!bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(false, pChange);
        pChange->commit();
    }
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testWriterFontwork)
{
    loadFromFile(u"tdf125885_WordArt.docx");
    // Without the patch WordArt in text document was imported as rectangular custom shape with
    // attached frame. So there was no artistic text at all. Now it is imported as Fontwork.
    // This test covers some basic properties.

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    // Is it a Fontwork?
    bool bTextBox = bool();
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"TextBox"_ustr) >>= bTextBox);
    CPPUNIT_ASSERT(!bTextBox);

    uno::Reference<css::text::XTextFrame> xTextFrame;
    xShapeProps->getPropertyValue(u"TextBoxContent"_ustr) >>= xTextFrame;
    CPPUNIT_ASSERT(!xTextFrame.is());

    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    CPPUNIT_ASSERT(aGeoPropSeq.getLength() > 0);
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);

    uno::Sequence<beans::PropertyValue> aTextPathSeq;
    aGeoPropMap.getValue(u"TextPath"_ustr) >>= aTextPathSeq;
    CPPUNIT_ASSERT(aTextPathSeq.getLength() > 0);

    comphelper::SequenceAsHashMap aTextPathPropMap(aTextPathSeq);
    bool bTextPathOn = bool();
    CPPUNIT_ASSERT(aTextPathPropMap.getValue(u"TextPath"_ustr) >>= bTextPathOn);
    CPPUNIT_ASSERT(bTextPathOn);

    // Is it the correct kind of Fontwork?
    uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentSeq;
    aGeoPropMap.getValue(u"AdjustmentValues"_ustr) >>= aAdjustmentSeq;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aAdjustmentSeq.getLength());

    uno::Sequence<uno::Sequence<beans::PropertyValue>> aHandleSeq;
    aGeoPropMap.getValue(u"Handles"_ustr) >>= aHandleSeq;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aHandleSeq.getLength());

    awt::Rectangle aViewBox;
    aGeoPropMap.getValue(u"ViewBox"_ustr) >>= aViewBox;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21600), aViewBox.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21600), aViewBox.Height);

    CPPUNIT_ASSERT_EQUAL(uno::Any(u"textDoubleWave1"_ustr),
                         aGeoPropMap.getValue(u"PresetTextWarp"_ustr));

    CPPUNIT_ASSERT_EQUAL(uno::Any(u"mso-spt158"_ustr), aGeoPropMap.getValue(u"Type"_ustr));

    // Are properties correctly copied to shape?
    CPPUNIT_ASSERT_EQUAL(uno::Any(COL_LIGHTBLUE), xShapeProps->getPropertyValue(u"FillColor"_ustr));

    CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::FillStyle_SOLID),
                         xShapeProps->getPropertyValue(u"FillStyle"_ustr));

    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Courier New"_ustr),
                         xShapeProps->getPropertyValue(u"CharFontName"_ustr));

    CPPUNIT_ASSERT_EQUAL(uno::Any(float(awt::FontWeight::BOLD)),
                         xShapeProps->getPropertyValue("CharWeight"));

    lang::Locale aCharLocale;
    xShapeProps->getPropertyValue(u"CharLocale"_ustr) >>= aCharLocale;
    CPPUNIT_ASSERT_EQUAL(OUString("en"), aCharLocale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), aCharLocale.Country);

    CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::TextHorizontalAdjust_RIGHT),
                         xShapeProps->getPropertyValue(u"TextHorizontalAdjust"_ustr));
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testWriterFontwork2)
{
    loadFromFile(u"tdf125885_WordArt2.docx");
    // Without the patch WordArt in text document was imported as rectangular custom shape with
    // attached frame. So there was no artistic text at all. Now it is imported as Fontwork.
    // This test covers whether theme color properties are correctly converted on import.

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    // Fill
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(4000)),
                         xShapeProps->getPropertyValue(u"FillColorLumMod"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(6000)),
                         xShapeProps->getPropertyValue(u"FillColorLumOff"_ustr));
    // ID "6" for the theme "accent3" is not yet in API, but defined in enum PredefinedClrSchemeID
    // in drawingml/clrscheme.hxx.
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(6)),
                         xShapeProps->getPropertyValue(u"FillColorTheme"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(Color(215, 228, 189)),
                         xShapeProps->getPropertyValue(u"FillColor"_ustr));

    // Stroke
    CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::LineCap_ROUND),
                         xShapeProps->getPropertyValue(u"LineCap"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::LineStyle_DASH),
                         xShapeProps->getPropertyValue(u"LineStyle"_ustr));
    // Stroke has only the resulted color, but no reference to the used theme color "accent2".
    CPPUNIT_ASSERT_EQUAL(uno::Any(Color(149, 55, 53)),
                         xShapeProps->getPropertyValue(u"LineColor"_ustr));
    drawing::LineDash aLineDash;
    xShapeProps->getPropertyValue(u"LineDash"_ustr) >>= aLineDash;
    CPPUNIT_ASSERT_EQUAL(drawing::DashStyle_ROUNDRELATIVE, aLineDash.Style);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aLineDash.Dots);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aLineDash.DotLen);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aLineDash.Dashes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aLineDash.DashLen);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(199), aLineDash.Distance);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(635)),
                         xShapeProps->getPropertyValue(u"LineWidth"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(20)),
                         xShapeProps->getPropertyValue(u"LineTransparence"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::LineJoint_BEVEL),
                         xShapeProps->getPropertyValue(u"LineJoint"_ustr));
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testWriterFontwork3)
{
    loadFromFile(u"tdf125885_WordArt3.docx");
    // Without the patch WordArt in text document was imported as rectangular custom shape with
    // attached frame. So there was no artistic text at all. Now it is imported as Fontwork.
    // This test covers some aspects of import of gradient fill.

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // linear gradient, MSO UI 21deg
    {
        uno::Reference<beans::XPropertySet> xShapeProps(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::FillStyle_GRADIENT),
                             xShapeProps->getPropertyValue(u"FillStyle"_ustr));
        awt::Gradient2 aGradient;
        xShapeProps->getPropertyValue(u"FillGradient"_ustr) >>= aGradient;

        // MCGR: Use the completely imported transparency gradient to check for correctness
        basegfx::BColorStops aColorStops;
        aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, Color(aColorStops[0].getStopColor()));
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
        CPPUNIT_ASSERT_EQUAL(Color(0xf79646), Color(aColorStops[1].getStopColor()));

        CPPUNIT_ASSERT_EQUAL(sal_Int16(690), aGradient.Angle);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aGradient.XOffset);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aGradient.YOffset);
        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
    }

    // radial gradient, centered
    {
        uno::Reference<beans::XPropertySet> xShapeProps(xDrawPage->getByIndex(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::FillStyle_GRADIENT),
                             xShapeProps->getPropertyValue(u"FillStyle"_ustr));
        awt::Gradient2 aGradient;
        xShapeProps->getPropertyValue(u"FillGradient"_ustr) >>= aGradient;

        // MCGR: Use the completely imported transparency gradient to check for correctness
        basegfx::BColorStops aColorStops;
        aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.40000000000000002));
        CPPUNIT_ASSERT_EQUAL(Color(0xf79646), Color(aColorStops[0].getStopColor()));
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, Color(aColorStops[1].getStopColor()));

        CPPUNIT_ASSERT_EQUAL(sal_Int16(900), aGradient.Angle);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(50), aGradient.XOffset);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(50), aGradient.YOffset);
        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RADIAL, aGradient.Style);
    }

    // rectangular gradient, focus right, bottom
    {
        uno::Reference<beans::XPropertySet> xShapeProps(xDrawPage->getByIndex(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::FillStyle_GRADIENT),
                             xShapeProps->getPropertyValue(u"FillStyle"_ustr));
        awt::Gradient2 aGradient;
        xShapeProps->getPropertyValue(u"FillGradient"_ustr) >>= aGradient;

        // MCGR: Use the completely imported transparency gradient to check for correctness
        basegfx::BColorStops aColorStops;
        aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
        CPPUNIT_ASSERT_EQUAL(Color(0xf79646), Color(aColorStops[0].getStopColor()));
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, Color(aColorStops[1].getStopColor()));

        CPPUNIT_ASSERT_EQUAL(sal_Int16(900), aGradient.Angle);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aGradient.XOffset);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aGradient.YOffset);
        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RECT, aGradient.Style);
    }
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testWriterFontworkNonAccentColor)
{
    loadFromFile(u"tdf152840_WordArt_non_accent_color.docx");
    // The file contains WordArt which uses the theme colors "Background 1", "Text 1", "Background 2"
    // and "Text 2".

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // The ID for the theme colors is not yet in API, but defined in enum PredefinedClrSchemeID
    // in drawingml/clrscheme.hxx. Without fix the ID was -1 meaning no theme is used, and the color
    // was Black (=0).

    // background 1 = lt1 = ID 1
    uno::Reference<beans::XPropertySet> xShape0Props(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(1)),
                         xShape0Props->getPropertyValue(u"FillColorTheme"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(Color(255, 204, 153)),
                         xShape0Props->getPropertyValue(u"FillColor"_ustr));

    // text 1 = dk1 = ID 0
    uno::Reference<beans::XPropertySet> xShape1Props(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(0)),
                         xShape1Props->getPropertyValue(u"FillColorTheme"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(COL_LIGHTRED), xShape1Props->getPropertyValue(u"FillColor"_ustr));

    // background 2 = lt2 = ID 3
    uno::Reference<beans::XPropertySet> xShape2Props(xDrawPage->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(3)),
                         xShape2Props->getPropertyValue(u"FillColorTheme"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(Color(235, 221, 195)),
                         xShape2Props->getPropertyValue(u"FillColor"_ustr));

    // text 2 = dk2 = ID 2
    uno::Reference<beans::XPropertySet> xShape3Props(xDrawPage->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(2)),
                         xShape3Props->getPropertyValue(u"FillColorTheme"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(Color(119, 95, 85)),
                         xShape3Props->getPropertyValue(u"FillColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testWriterShapeFillNonAccentColor)
{
    loadFromFile(u"tdf152840_theme_color_non_accent.docx");
    // The file contains shapes which uses the theme colors "bg2", "bg1", "tx1" and "tx2" in this
    // order as fill color.

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // The ID for the theme colors is not yet in API, but defined in enum PredefinedClrSchemeID
    // in drawingml/clrscheme.hxx. Without fix the ID was -1 meaning no theme is used.
    uno::Reference<beans::XPropertySet> xShape0Props(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(3)),
                         xShape0Props->getPropertyValue(u"FillColorTheme"_ustr));
    uno::Reference<beans::XPropertySet> xShape1Props(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(1)),
                         xShape1Props->getPropertyValue(u"FillColorTheme"_ustr));
    uno::Reference<beans::XPropertySet> xShape2Props(xDrawPage->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(0)),
                         xShape2Props->getPropertyValue(u"FillColorTheme"_ustr));
    uno::Reference<beans::XPropertySet> xShape3Props(xDrawPage->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int16(2)),
                         xShape3Props->getPropertyValue(u"FillColorTheme"_ustr));
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testWriterFontworkDarkenTransparency)
{
    loadFromFile(u"tdf152896_WordArt_color_darken.docx");
    // The file contains a WordArt shape with theme colors "Background 2", shading mode "Darken 25%"
    // and "20% Transparency". Word writes this as w:color element with additional w14:textFill
    // element. In such case the w14:textFill element supersedes the w:color element. Error was, that
    // the darkening was applied twice, once from w:color and the other time from w14:textFill.

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xShapeProps(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // Without the fix in place the test would have failed with
    // Expected: 13676402 (= 0xD0AF72 = rgb(208, 175, 114) => luminance 63.14%)
    // Actual: 11897660 (= 0xB58B3C = rgb(181, 139, 60) => luminance 47.25% )
    // The original "Background 2" is 0xEBDDC3 = rgb(235, 221, 195) => luminance 84.31%
    CPPUNIT_ASSERT_EQUAL(uno::Any(Color(208, 175, 114)),
                         xShapeProps->getPropertyValue(u"FillColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testImportWordArtGradient)
{
    loadFromFile(u"tdf139618_ImportWordArtGradient.pptx");
    // Without the patch all WordArt was imported with solid color. Now gradient is imported.
    // This test covers several aspects of import of gradient fill.

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // linear gradient, MSO UI 21deg, solid transparency on outline
    {
        uno::Reference<beans::XPropertySet> xShapeProps(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::FillStyle_GRADIENT),
                             xShapeProps->getPropertyValue(u"FillStyle"_ustr));
        awt::Gradient2 aGradient;
        xShapeProps->getPropertyValue(u"FillGradient"_ustr) >>= aGradient;

        // MCGR: Use the completely imported transparency gradient to check for correctness
        basegfx::BColorStops aColorStops;
        aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
        CPPUNIT_ASSERT_EQUAL(Color(0xffc000), Color(aColorStops[0].getStopColor()));
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
        CPPUNIT_ASSERT_EQUAL(Color(0xc00000), Color(aColorStops[1].getStopColor()));

        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(690), aGradient.Angle);

        CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::LineStyle_SOLID),
                             xShapeProps->getPropertyValue(u"LineStyle"_ustr));
        sal_Int32 nOutlineColor;
        xShapeProps->getPropertyValue(u"LineColor"_ustr) >>= nOutlineColor;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), nOutlineColor);
        sal_Int16 nLineTransparence;
        xShapeProps->getPropertyValue(u"LineTransparence"_ustr) >>= nLineTransparence;
        CPPUNIT_ASSERT_EQUAL(sal_Int16(60), nLineTransparence);
    }

    // radial gradient, direct color with transparency, focus center, dotted outline
    // The stop color transparency is imported as transparency gradient with same geometry.
    {
        uno::Reference<beans::XPropertySet> xShapeProps(xDrawPage->getByIndex(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::FillStyle_GRADIENT),
                             xShapeProps->getPropertyValue(u"FillStyle"_ustr));
        awt::Gradient2 aGradient;
        xShapeProps->getPropertyValue(u"FillGradient"_ustr) >>= aGradient;

        // MCGR: Use the completely imported transparency gradient to check for correctness
        basegfx::BColorStops aColorStops;
        aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
        CPPUNIT_ASSERT_EQUAL(Color(0x0083e0), Color(aColorStops[0].getStopColor()));
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
        CPPUNIT_ASSERT_EQUAL(Color(0xe6e600), Color(aColorStops[1].getStopColor()));

        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RADIAL, aGradient.Style);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(50), aGradient.XOffset);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(50), aGradient.YOffset);

        xShapeProps->getPropertyValue(u"FillTransparenceGradient"_ustr) >>= aGradient;
        // MCGR: Use the completely imported transparency gradient to check for correctness
        aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

        // Transparency is encoded in gray color.
        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
        CPPUNIT_ASSERT_EQUAL(COL_GRAY7, Color(aColorStops[0].getStopColor()));
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
        CPPUNIT_ASSERT_EQUAL(Color(0x4d4d4d), Color(aColorStops[1].getStopColor()));

        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RADIAL, aGradient.Style);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(50), aGradient.XOffset);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(50), aGradient.YOffset);

        CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::LineStyle_DASH),
                             xShapeProps->getPropertyValue(u"LineStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::LineCap_ROUND),
                             xShapeProps->getPropertyValue(u"LineCap"_ustr));
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(7384391)),
                             xShapeProps->getPropertyValue(u"LineColor"_ustr));
        drawing::LineDash aLineDash;
        xShapeProps->getPropertyValue(u"LineDash"_ustr) >>= aLineDash;
        CPPUNIT_ASSERT_EQUAL(drawing::DashStyle_ROUNDRELATIVE, aLineDash.Style);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aLineDash.Dots);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aLineDash.Dashes);
    }

    // solid theme color accent 1, rectangular transparency gradient, focus top-right, no outline
    // FillProperties::pushToPropMap imports this currently (Mar 2023) as color gradient.
    // Thus no theme color is tested but direct color.
    {
        uno::Reference<beans::XPropertySet> xShapeProps(xDrawPage->getByIndex(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::FillStyle_GRADIENT),
                             xShapeProps->getPropertyValue(u"FillStyle"_ustr));
        awt::Gradient2 aGradient;
        xShapeProps->getPropertyValue(u"FillGradient"_ustr) >>= aGradient;

        // MCGR: Use the completely imported transparency gradient to check for correctness
        basegfx::BColorStops aColorStops;
        aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
        CPPUNIT_ASSERT_EQUAL(Color(0x4472c4), Color(aColorStops[0].getStopColor()));
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
        CPPUNIT_ASSERT_EQUAL(Color(0x4472c4), Color(aColorStops[1].getStopColor()));

        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RECT, aGradient.Style);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aGradient.XOffset);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aGradient.YOffset);

        xShapeProps->getPropertyValue(u"FillTransparenceGradient"_ustr) >>= aGradient;
        // MCGR: Use the completely imported transparency gradient to check for correctness
        aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

        // Transparency is encoded in gray color.
        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aColorStops[0].getStopColor()));
        CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(aColorStops[1].getStopColor()));

        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RECT, aGradient.Style);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aGradient.XOffset);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aGradient.YOffset);

        CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::LineStyle_NONE),
                             xShapeProps->getPropertyValue(u"LineStyle"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testWordArtBitmapFill)
{
    // The document has a WordArt shape with bitmap fill.
    // Without fix it was imported as solid color fill.
    loadFromFile(u"tdf139618_WordArtBitmapFill.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::FillStyle_BITMAP),
                         xShapeProps->getPropertyValue(u"FillStyle"_ustr));

    // Test some bitmap properties
    CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::BitmapMode_REPEAT),
                         xShapeProps->getPropertyValue(u"FillBitmapMode"_ustr));
    CPPUNIT_ASSERT_EQUAL(uno::Any(true), xShapeProps->getPropertyValue(u"FillBitmapTile"_ustr));
    uno::Reference<awt::XBitmap> xBitmap;
    xShapeProps->getPropertyValue(u"FillBitmap"_ustr) >>= xBitmap;

    uno::Reference<graphic::XGraphic> xGraphic;
    xGraphic.set(xBitmap, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphicDescriptor(xGraphic, uno::UNO_QUERY_THROW);
    OUString sMimeType;
    CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("MimeType") >>= sMimeType);
    CPPUNIT_ASSERT_EQUAL(OUString("image/jpeg"), sMimeType);
    awt::Size aSize100thMM;
    CPPUNIT_ASSERT(xGraphicDescriptor->getPropertyValue("Size100thMM") >>= aSize100thMM);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1592), aSize100thMM.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1592), aSize100thMM.Height);
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testWordArtDefaultColor)
{
    // The document has a WordArt shape for which the text color is not explicitly set. In such cases
    // MS Office uses the scheme color 'tx1'. Without fix it was imported as 'fill none'. The shape
    // existed but was not visible on screen.
    loadFromFile(u"tdf155327_WordArtDefaultColor.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::Any(drawing::FillStyle_SOLID),
                         xShapeProps->getPropertyValue(u"FillStyle"_ustr));

    CPPUNIT_ASSERT_EQUAL(uno::Any(Color(3, 74, 144)),
                         xShapeProps->getPropertyValue(u"FillColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(OoxShapeTest, testGlowOnGroup)
{
    // The document contains a group of two shapes. A glow-effect is set on the group.
    // Without the fix, the children of the group were not imported at all.
    loadFromFile(u"tdf156902_GlowOnGroup.docx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xGroup->getCount());
}
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
