/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/util/XTheme.hpp>

#include <docmodel/uno/UnoGradientTools.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/uno/UnoTheme.hxx>
#include <docmodel/theme/Theme.hxx>

#include <comphelper/sequenceashashmap.hxx>
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

/// oox drawingml tests.
class OoxDrawingmlTest : public UnoApiTest
{
public:
    OoxDrawingmlTest()
        : UnoApiTest(u"/oox/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTransparentText)
{
    loadFromFile(u"transparent-text.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraph(
        xShape->createEnumeration()->nextElement(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPortion(xParagraph->createEnumeration()->nextElement(),
                                                 uno::UNO_QUERY);

    sal_Int16 nTransparency = 0;
    xPortion->getPropertyValue(u"CharTransparence"_ustr) >>= nTransparency;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 75
    // - Actual  : 0
    // i.e. the transparency of the character color was lost on import/export.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(75), nTransparency);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTdf131082)
{
    loadFromFile(u"tdf131082.pptx");
    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> XPropSet(getChildShape(getChildShape(xShape, 0), 0),
                                                 uno::UNO_QUERY);

    drawing::FillStyle eFillStyle = drawing::FillStyle_NONE;
    XPropSet->getPropertyValue(u"FillStyle"_ustr) >>= eFillStyle;

    // Without the accompanying fix in place, this test would have failed with:
    // with drawing::FillStyle_NONE - 0
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, eFillStyle);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testPresetAdjustValue)
{
    loadFromFile(u"preset-adjust-value.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentSeq;
    aGeoPropMap.getValue(u"AdjustmentValues"_ustr) >>= aAdjustmentSeq;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aAdjustmentSeq.getLength());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 11587
    // - Actual  : 10813
    // i.e. the adjust value was set from the placeholder, not from the shape.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11587), aAdjustmentSeq[0].Value.get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testChartDataLabelCharColor)
{
    loadFromFile(u"chart-data-label-char-color.docx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<chart2::XChartDocument> xModel(xShape->getPropertyValue(u"Model"_ustr),
                                                  uno::UNO_QUERY);
    uno::Reference<chart2::XCoordinateSystemContainer> xDiagram(xModel->getFirstDiagram(),
                                                                uno::UNO_QUERY);

    uno::Reference<chart2::XChartTypeContainer> xCoordinateSystem(
        xDiagram->getCoordinateSystems()[0], uno::UNO_QUERY);

    uno::Reference<chart2::XDataSeriesContainer> xChartType(xCoordinateSystem->getChartTypes()[0],
                                                            uno::UNO_QUERY);

    uno::Reference<chart2::XDataSeries> xDataSeries = xChartType->getDataSeries()[0];

    uno::Reference<beans::XPropertySet> xDataPoint = xDataSeries->getDataPointByIndex(0);

    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aLabels;
    xDataPoint->getPropertyValue(u"CustomLabelFields"_ustr) >>= aLabels;
    uno::Reference<beans::XPropertySet> xLabel = aLabels[0];

    Color nCharColor;
    xLabel->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 16777215
    // - Actual  : -1
    // i.e. the data label had no explicit (white) color.
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nCharColor);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testGradientMultiStepTransparency)
{
    // Load a document with a multi-step gradient.
    loadFromFile(u"gradient-multistep-transparency.pptx");

    // Check the end transparency of the gradient.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XNamed> xShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Rectangle 4"_ustr, xShape->getName());
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    awt::Gradient2 aTransparence;
    xShapeProps->getPropertyValue(u"FillTransparenceGradient"_ustr) >>= aTransparence;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 16777215 (COL_WHITE)
    // - Actual  : 3487029 (0x353535)
    // i.e. the end transparency was not 100%, but was 21%, leading to an unexpected visible line on
    // the right of this shape.
    // MCGR: Use the completely imported transparency gradient to check for correctness
    const basegfx::BColorStops aColorStops
        = model::gradient::getColorStopsFromUno(aTransparence.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(5), aColorStops.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, aColorStops[0].getStopOffset(), 1E-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.19, aColorStops[1].getStopOffset(), 1E-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.35, aColorStops[2].getStopOffset(), 1E-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.58, aColorStops[3].getStopOffset(), 1E-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.00, aColorStops[4].getStopOffset(), 1E-3);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT_EQUAL(Color(0x9e9e9e), Color(aColorStops[1].getStopColor()));
    CPPUNIT_ASSERT_EQUAL(Color(0x363636), Color(aColorStops[2].getStopColor()));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aColorStops[3].getStopColor()));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aColorStops[4].getStopColor()));
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testShapeTextAlignment)
{
    loadFromFile(u"shape-text-alignment.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nParaAdjust = -1;
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"ParaAdjust"_ustr) >>= nParaAdjust);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 3
    // i.e. text which is meant to be left-aligned was centered at a paragraph level.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testShapeTextAdjustLeft)
{
    loadFromFile(u"shape-text-adjust-left.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    drawing::TextHorizontalAdjust eAdjust;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3 (center)
    // - Actual  : 1 (block)
    // i.e. text was center-adjusted, not default-adjusted (~left).
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"TextHorizontalAdjust"_ustr) >>= eAdjust);
    CPPUNIT_ASSERT_EQUAL(drawing::TextHorizontalAdjust_BLOCK, eAdjust);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testCameraRotationRevolution)
{
    loadFromFile(u"camera-rotation-revolution.docx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XNamed> xShape0(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xShape1(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps0(xShape0, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps1(xShape1, uno::UNO_QUERY);
    sal_Int32 nRotateAngle0;
    sal_Int32 nRotateAngle1;
    xShapeProps0->getPropertyValue(u"RotateAngle"_ustr) >>= nRotateAngle0;
    xShapeProps1->getPropertyValue(u"RotateAngle"_ustr) >>= nRotateAngle1;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 8000
    // - Actual  : 0
    // so the camera rotation would not have been factored into how the shape is displayed
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8000), nRotateAngle0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(27000), nRotateAngle1);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTdf146534_CameraRotationRevolutionNonWpsShapes)
{
    loadFromFile(u"camera-rotation-revolution-nonwps.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XNamed> xShape0(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xShape1(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps0(xShape0, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps1(xShape1, uno::UNO_QUERY);
    sal_Int32 nRotateAngle0;
    sal_Int32 nRotateAngle1;
    xShapeProps0->getPropertyValue(u"RotateAngle"_ustr) >>= nRotateAngle0;
    xShapeProps1->getPropertyValue(u"RotateAngle"_ustr) >>= nRotateAngle1;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 9000
    // - Actual  : 0
    // so the camera rotation would not have been factored into how the shape is displayed
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(9000), nRotateAngle0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(30500), nRotateAngle1);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTableShadow)
{
    auto verify = [](const uno::Reference<lang::XComponent>& xComponent) {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xComponent, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage(
            xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        bool bShadow = false;
        CPPUNIT_ASSERT(xShape->getPropertyValue(u"Shadow"_ustr) >>= bShadow);

        CPPUNIT_ASSERT(bShadow);
        Color nColor;
        CPPUNIT_ASSERT(xShape->getPropertyValue(u"ShadowColor"_ustr) >>= nColor);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nColor);
    };
    loadFromFile(u"table-shadow.pptx");
    // Without the accompanying fix in place, this test would have failed, because shadow on a table
    // was lost on import.
    verify(mxComponent);

    saveAndReload(u"Impress Office Open XML"_ustr);

    // Without the accompanying fix in place, this test would have failed, because shadow on a table
    // was lost on export.
    verify(mxComponent);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testGroupShapeSmartArt)
{
    // Given a file with a smartart inside a group shape:
    loadFromFile(u"smartart-groupshape.pptx");

    // Then make sure that the smartart is not just an empty group shape:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xSmartArt(xGroup->getByIndex(0), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed, because we lost all
    // children of the group shape representing the smartart.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), xSmartArt->getCount());
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTdf142605_CurveSize)
{
    // The document contains a Bezier curve, where the control points are outside the bounding
    // rectangle of the shape. Error was, that the export uses a path size which included the
    // control points.
    loadFromFile(u"tdf142605_CurveSize.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    auto xPage = xDrawPagesSupplier->getDrawPages()->getByIndex(0);
    uno::Reference<drawing::XDrawPage> xDrawPage(xPage, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapeProps.is());
    uno::Reference<container::XNamed> xShapeNamed(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapeNamed.is());
    CPPUNIT_ASSERT_EQUAL(u"Bézier curve 1"_ustr, xShapeNamed->getName());

    css::awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(u"BoundRect"_ustr) >>= aBoundRect;
    // Without fix, size was 6262 x 3509, and position was 10037|6790.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(8601), aBoundRect.Width, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(4601), aBoundRect.Height, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(7699), aBoundRect.X, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(5699), aBoundRect.Y, 1);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testChartThemeOverride)
{
    // Given a document with 2 slides, slide1 has a chart with a theme override and slide2 has a
    // shape:
    loadFromFile(u"chart-theme-override.pptx");

    // Then make sure that the slide 2 shape's text color is blue, not red:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(1),
                                                 uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xShape->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xPara(xText->createEnumeration()->nextElement(),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPortion(xPara->createEnumeration()->nextElement(),
                                                 uno::UNO_QUERY);
    Color nActual{ 0 };
    xPortion->getPropertyValue(u"CharColor"_ustr) >>= nActual;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4485828 (0x4472c4)
    // - Actual  : 16711680 (0xff0000)
    // i.e. the text color was red, not blue.
    CPPUNIT_ASSERT_EQUAL(Color(0x4472C4), nActual);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testPptxTheme)
{
    // Given a PPTX file with a slide -> master slide -> theme:
    loadFromFile(u"theme.pptx");

    // Then make sure the theme + referring to that theme is imported:
    // Check the imported theme of the master page:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterpage(xDrawPage->getMasterPage(), uno::UNO_QUERY);

    uno::Reference<util::XTheme> xTheme;
    xMasterpage->getPropertyValue(u"Theme"_ustr) >>= xTheme;

    // We expect the theme to be set on the master page
    CPPUNIT_ASSERT(xTheme.is());
    auto* pUnoTheme = dynamic_cast<UnoTheme*>(xTheme.get());
    CPPUNIT_ASSERT(pUnoTheme);
    auto pTheme = pUnoTheme->getTheme();

    CPPUNIT_ASSERT_EQUAL(u"Office Theme"_ustr, pTheme->GetName());
    CPPUNIT_ASSERT_EQUAL(u"Office"_ustr, pTheme->getColorSet()->getName());

    CPPUNIT_ASSERT_EQUAL(Color(0x954F72),
                         pTheme->getColorSet()->getColor(model::ThemeColorType::FollowedHyperlink));

    // Check the reference to that theme:
    uno::Reference<drawing::XShapes> xDrawPageShapes(xDrawPage, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xDrawPageShapes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xShape->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xPara(xText->createEnumeration()->nextElement(),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPortion(xPara->createEnumeration()->nextElement(),
                                                 uno::UNO_QUERY);

    // Check the theme colors are as expected
    {
        uno::Reference<util::XComplexColor> xComplexColor;
        CPPUNIT_ASSERT(xPortion->getPropertyValue(u"CharComplexColor"_ustr) >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod,
                             aComplexColor.getTransformations()[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), aComplexColor.getTransformations()[0].mnValue);
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff,
                             aComplexColor.getTransformations()[1].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), aComplexColor.getTransformations()[1].mnValue);
    }
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTdf132557_footerCustomShapes)
{
    // slide with date, footer, slide number with custom shapes
    loadFromFile(u"testTdf132557_footerCustomShapes.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // Test if we were able to import the footer shapes with CustomShape service.
    uno::Reference<drawing::XShape> xShapeDateTime(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.CustomShape"_ustr, xShapeDateTime->getShapeType());
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.lang.IndexOutOfBoundsException
    // i.e. the shape wasn't on the slide there since it was imported as a property, not a shape.

    uno::Reference<drawing::XShape> xShapeFooter(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.CustomShape"_ustr, xShapeFooter->getShapeType());

    uno::Reference<drawing::XShape> xShapeSlideNum(xDrawPage->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.CustomShape"_ustr, xShapeSlideNum->getShapeType());
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testThemeColorTint_Table)
{
    // Given a document with a table style, using theme color with tinting in the A2 cell:
    loadFromFile(u"theme-tint.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCellRange> xTable;
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"Model"_ustr) >>= xTable);
    uno::Reference<beans::XPropertySet> xA1(xTable->getCellByPosition(0, 0), uno::UNO_QUERY);

    // check theme color
    {
        uno::Reference<util::XComplexColor> xComplexColor;
        CPPUNIT_ASSERT(xA1->getPropertyValue(u"FillComplexColor"_ustr) >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aComplexColor.getThemeColorType());
        {
            auto const& rTrans = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(0), rTrans.size());
        }
    }

    {
        uno::Reference<beans::XPropertySet> xA2(xTable->getCellByPosition(0, 1), uno::UNO_QUERY);
        uno::Reference<util::XComplexColor> xComplexColor;
        CPPUNIT_ASSERT(xA2->getPropertyValue(u"FillComplexColor"_ustr) >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aComplexColor.getThemeColorType());
        {
            auto const& rTrans = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(1), rTrans.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::Tint, rTrans[0].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), rTrans[0].mnValue);
        }
    }
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testThemeColor_Shape)
{
    // Given a document with a table style, using theme color with tinting in the A2 cell:
    loadFromFile(u"ThemeShapesReference.pptx");

    // Then make sure that we only import theming info to the doc model if the effects are limited
    // to lum mod / off that we can handle (i.e. no tint/shade):
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // check line and fill theme color of shape1
    {
        uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<util::XComplexColor> xComplexColor;
        CPPUNIT_ASSERT(xShape->getPropertyValue(u"FillComplexColor"_ustr) >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        {
            auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6, aComplexColor.getThemeColorType());

            auto const& rTrans = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(2), rTrans.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans[0].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), rTrans[0].mnValue);
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans[1].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), rTrans[1].mnValue);
        }

        CPPUNIT_ASSERT(xShape->getPropertyValue(u"LineComplexColor"_ustr) >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        {
            auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6, aComplexColor.getThemeColorType());

            auto const& rTrans = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(1), rTrans.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans[0].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(5000), rTrans[0].mnValue);
        }
    }
    // check line and fill theme color of shape2
    {
        uno::Reference<util::XComplexColor> xComplexColor;
        uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShape->getPropertyValue(u"FillComplexColor"_ustr) >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        {
            auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aComplexColor.getThemeColorType());

            auto const& rTrans = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(0), rTrans.size());
        }

        CPPUNIT_ASSERT(xShape->getPropertyValue(u"LineComplexColor"_ustr) >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        {
            auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aComplexColor.getThemeColorType());

            auto const& rTrans = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(1), rTrans.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans[0].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(7500), rTrans[0].mnValue);
        }
    }
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testVert270AndTextRot)
{
    // tdf##149551. The document contains a shape with attributes 'rot="720000"' and 'vert="vert270"'
    // of the <bodyPr> element. Without the fix the simulation of vert270 had overwritten the text
    // rotation angle and thus 'rot'="720000" was lost.
    loadFromFile(u"tdf149551_vert270AndTextRot.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);

    // Without the fix the property "TextRotateAngle" does not exist.
    comphelper::SequenceAsHashMap::iterator it = aGeoPropMap.find(u"TextRotateAngle"_ustr);
    CPPUNIT_ASSERT(it != aGeoPropMap.end());
    sal_Int32 nAngle;
    // MS 720000 clockwise -> ODF -12deg counter-clockwise
    it->second >>= nAngle;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-12), nAngle);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTextRot)
{
    // tdf#149551 The document contains a shape with attribute 'rot="720000"' of the <bodyPr> element.
    // Without fix, the text rotation angle was saved in "TextPreRotateAngle" instead of
    // "TextRotateAngle". That resulted in unrotated but sheared text.
    loadFromFile(u"tdf149551_TextRot.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);

    // Without the fix the property "TextRotateAngle" does not exist.
    comphelper::SequenceAsHashMap::iterator it = aGeoPropMap.find(u"TextRotateAngle"_ustr);
    CPPUNIT_ASSERT(it != aGeoPropMap.end());
    sal_Int32 nAngle;
    // MS 720000 clockwise -> ODF -12deg counter-clockwise
    it->second >>= nAngle;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-12), nAngle);

    // Because writing mode is LR_TB, the property "TextPreRotateAngle" may missing, or in case it
    // exists, its value must be 0. Without fix it had value -12.
    it = aGeoPropMap.find(u"TextPreRotateAngle"_ustr);
    if (it != aGeoPropMap.end())
    {
        it->second >>= nAngle;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nAngle);
    }
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTdf113187ConstantArcTo)
{
    loadFromFile(u"tdf113187_arcTo_withoutReferences.pptx");

    // Get ViewBox of shape
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);

    // Without the fix width and height of the ViewBox were 0 and thus the shape was not shown.
    auto aViewBox = aGeoPropMap[u"ViewBox"_ustr].get<css::awt::Rectangle>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3600000), aViewBox.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3600000), aViewBox.Height);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTdf125085WordArtFontTheme)
{
    // The font info for the shape is in the theme, the text run has no font settings.
    loadFromFile(u"tdf125085_WordArtFontTheme.pptx");

    // Get shape and its properties
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);

    // Make sure shape has correct font and local.
    // Without the fix some application defaults were used.
    OUString sFontName;
    xShapeProps->getPropertyValue(u"CharFontNameComplex"_ustr) >>= sFontName;
    CPPUNIT_ASSERT_EQUAL(u"Noto Serif Hebrew"_ustr, sFontName);
    css::lang::Locale aLocal;
    xShapeProps->getPropertyValue(u"CharLocaleComplex"_ustr) >>= aLocal;
    CPPUNIT_ASSERT_EQUAL(u"IL"_ustr, aLocal.Country);
    CPPUNIT_ASSERT_EQUAL(u"he"_ustr, aLocal.Language);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTdf125085WordArtFontText)
{
    // The font info for the shape is in the text run inside the shape.
    loadFromFile(u"tdf125085_WordArtFontText.pptx");

    // Get shape and its properties
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);

    // Make sure shape has correct font and local.
    // Without the fix some application defaults were used.
    OUString sFontName;
    xShapeProps->getPropertyValue(u"CharFontNameComplex"_ustr) >>= sFontName;
    CPPUNIT_ASSERT_EQUAL(u"Noto Serif Hebrew"_ustr, sFontName);
    css::lang::Locale aLocal;
    xShapeProps->getPropertyValue(u"CharLocaleComplex"_ustr) >>= aLocal;
    CPPUNIT_ASSERT_EQUAL(u"IL"_ustr, aLocal.Country);
    CPPUNIT_ASSERT_EQUAL(u"he"_ustr, aLocal.Language);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
