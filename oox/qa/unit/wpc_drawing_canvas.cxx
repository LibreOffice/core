/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/color.hxx>
#include <test/unoapixml_test.hxx>

#include <docmodel/color/ComplexColor.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <editeng/unoprnms.hxx>

#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/util/XComplexColor.hpp>
using namespace ::com::sun::star;

namespace
{
/// The test suite covers tests for import of Word drawing canvas (wpc), available since LO 24.2.
/// Before its implementation the VML fallback was used. That lost properties because VML is not able
/// to describe them or the VML import of LO has deficits.
class TestWPC : public UnoApiXmlTest
{
public:
    TestWPC()
        : UnoApiXmlTest("/oox/qa/unit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_Table_inside_Textbox)
{
    // The document has a table inside a text box on a drawing canvas.
    loadFromFile(u"WPC_tdf48610_Textbox_with_table_inside.docx");

    // Make sure the table exists. Without import of drawing canvas, the table was lost.
    uno::Reference<text::XTextTablesSupplier> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextDocument->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_Text_in_ellipse)
{
    // The document has text in an ellipse on a drawing canvas.
    loadFromFile(u"WPC_Textwrap_in_ellipse.docx");

    // The VML import creates for an ellipse not a custom shape but a legacy ellipse and that has no
    // word wrap. Thus the text was in one line and overflows the shape. This overflow becomes visible
    // in the bounding box. Without fix the rectangle width was 9398 Hmm.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // getByIndex(0) gives the background shape, the ellipse is at index 1
    uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(1), uno::UNO_QUERY);
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    // The tolerance 10 is estimated and can be adjusted if required for HiDPI.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4740, aBoundRect.Width, 10);
}

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_MulticolorGradient)
{
    // The document has a shape with multi color gradient fill on a drawing canvas.
    loadFromFile(u"WPC_MulticolorGradient.docx");

    // The VML import was not able to import multicolor gradients. Thus only start and end color
    // were imported, ColorStops had only two elements.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(1), uno::UNO_QUERY);
    awt::Gradient2 aGradient;
    xShapeProps->getPropertyValue(UNO_NAME_FILLGRADIENT) >>= aGradient;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aGradient.ColorStops.getLength());
}

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_CanvasBackground)
{
    // The document has a drawing canvas with color fill.
    loadFromFile(u"WPC_CanvasBackground.docx");

    // The VML import displayed the background as if it was transparent. Thus the BoundRect
    // of the shape which represents the background was zero.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(0), uno::UNO_QUERY);
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    CPPUNIT_ASSERT(aBoundRect.Width > 0);
    CPPUNIT_ASSERT(aBoundRect.Height > 0);
}

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_Glow)
{
    // The document has a shape with glow effect.
    loadFromFile(u"WPC_Glow.docx");

    // VML does not know any glow effect. Thus it was lost on import.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(1), uno::UNO_QUERY);

    // Check glow properties
    sal_Int32 nGlowEffectRad = 0;
    xShapeProps->getPropertyValue(u"GlowEffectRadius"_ustr) >>= nGlowEffectRad;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(564), nGlowEffectRad); // 16 pt = 564.444... mm/100
    Color nGlowEffectColor;
    xShapeProps->getPropertyValue(u"GlowEffectColor"_ustr) >>= nGlowEffectColor;
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, nGlowEffectColor);
    sal_Int16 nGlowEffectTransparency = 0;
    xShapeProps->getPropertyValue(u"GlowEffectTransparency"_ustr) >>= nGlowEffectTransparency;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), nGlowEffectTransparency); // 10%
}

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_BentConnector)
{
    // The document has two shapes connected with a bentConnector on a drawing canvas.
    loadFromFile(u"WPC_BentConnector.docx");

    // VML has no information about the target shapes of the connector. The connector was imported as
    // custom shape, not as connector shape
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xInfo(xGroup->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInfo->supportsService("com.sun.star.drawing.ConnectorShape"));

    uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(2), uno::UNO_QUERY);
    com::sun::star::drawing::ConnectorType eEdgeKind;
    xShapeProps->getPropertyValue(UNO_NAME_EDGEKIND) >>= eEdgeKind;
    CPPUNIT_ASSERT_EQUAL(drawing::ConnectorType::ConnectorType_STANDARD, eEdgeKind);

    sal_Int32 nEdgeLineDelta;
    xShapeProps->getPropertyValue(UNO_NAME_EDGELINE1DELTA) >>= nEdgeLineDelta;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-635), nEdgeLineDelta);
    xShapeProps->getPropertyValue(UNO_NAME_EDGELINE2DELTA) >>= nEdgeLineDelta;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1949), nEdgeLineDelta);
    xShapeProps->getPropertyValue(UNO_NAME_EDGELINE3DELTA) >>= nEdgeLineDelta;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(887), nEdgeLineDelta);
}

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_ThemeColor)
{
    // The document has a shape with color fill used as pseudo background and a 'heart' shape with
    // color fill and colored line. All colors are theme colors.
    loadFromFile(u"WPC_ThemeColor.docx");

    // VML has no information about theme colors. Thus ThemeColorType was always 'Unknown'.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    // Check color of shape used for pseudo background
    {
        uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<util::XComplexColor> xComplexColor;
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(UNO_NAME_FILL_COMPLEX_COLOR)
                       >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Dark2, aComplexColor.getThemeColorType());
        {
            auto const& rTrans = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(2), rTrans.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans[0].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(7500), rTrans[1].mnValue);
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans[1].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2500), rTrans[0].mnValue);
        }
    }
    // Check colors of 'heart' shape
    {
        uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(1), uno::UNO_QUERY);
        uno::Reference<util::XComplexColor> xComplexColor;
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(UNO_NAME_FILL_COMPLEX_COLOR)
                       >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getThemeColorType());
    }
    {
        uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(1), uno::UNO_QUERY);
        uno::Reference<util::XComplexColor> xComplexColor;
        CPPUNIT_ASSERT(xShapeProps->getPropertyValue(UNO_NAME_LINE_COMPLEX_COLOR)
                       >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent4, aComplexColor.getThemeColorType());
    }
}

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_tdf104671_Cloud)
{
    // The document has 'cloud' shape on a drawing canvas.
    loadFromFile(u"WPC_tdf104671_Cloud.docx");

    // MS Office writes the 'cloud' shape without type to the VML fallback. Thus the VLM import uses
    // ClosedBezierShape with several closed polygons. That produces holes because of the even-odd
    // rule, and inner lines. The fix uses the mc:Choice alternative which provides the type for a
    // custom shape.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xInfo(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInfo->supportsService("com.sun.star.drawing.CustomShape"));
}

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_Shadow)
{
    // The document has a shape with blur shadow on a drawing canvas.
    loadFromFile(u"WPC_Shadow.docx");

    // The VML fallback contains a block shadow. Blur is not available in VML. The VML import does not
    // import shadow at all.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(1), uno::UNO_QUERY);
    bool bHasShadow = false;
    xShapeProps->getPropertyValue(UNO_NAME_SHADOW) >>= bHasShadow;
    CPPUNIT_ASSERT(bHasShadow);
    sal_Int32 nValue;
    xShapeProps->getPropertyValue(UNO_NAME_SHADOWBLUR) >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(282), nValue);
    xShapeProps->getPropertyValue(UNO_NAME_SHADOWXDIST) >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(224), nValue);
    xShapeProps->getPropertyValue(UNO_NAME_SHADOWYDIST) >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(224), nValue);
    Color nColor;
    xShapeProps->getPropertyValue(UNO_NAME_SHADOWCOLOR) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_GRAY, nColor);
}

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_tdf158339_shape_text_in_group)
{
    // The document has a group of two shapes with text. This group is child of a drawing canvas.
    // Without fix the text of the shapes were imported as separate text boxes.
    loadFromFile(u"WPC_tdf158339_shape_text_in_group.docx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    // Make sure there is only one object on that page. Without fix there were three objects.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());
    // Get the group which represents the drawing canvas and the group object inside.
    uno::Reference<drawing::XShapes> xCanvas(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup(xCanvas->getByIndex(1), uno::UNO_QUERY);
    // Get the properties of the second shape inside the group
    uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(1), uno::UNO_QUERY);
    // and make sure the shape has text.
    uno::Reference<css::text::XTextFrame> xTextFrame;
    xShapeProps->getPropertyValue(u"TextBoxContent"_ustr) >>= xTextFrame;
    CPPUNIT_ASSERT(xTextFrame.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Group"), xTextFrame->getText()->getString());
}

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_tdf158348_shape_text_in_table_cell)
{
    // The document has a shape with text on a drawing canvas in a table cell.
    // Without fix the text of the shape becomes part of the paragraph of the table cell.
    loadFromFile(u"WPC_tdf158348_shape_text_in_table_cell.docx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // Get the shape and make sure it has text.
    uno::Reference<drawing::XShapes> xCanvas(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xCanvas->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<css::text::XTextFrame> xTextFrame;
    xShapeProps->getPropertyValue(u"TextBoxContent"_ustr) >>= xTextFrame;
    CPPUNIT_ASSERT(xTextFrame.is());
    // The string was empty without fix.
    CPPUNIT_ASSERT_EQUAL(u"Inside shape"_ustr, xTextFrame->getText()->getString());

    // Get the table and make sure the cell has only its own text.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(
        xTablesSupplier->getTextTables()->getByName(u"Table1"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName("A1"), uno::UNO_QUERY);
    // The string had started with "Inside shape" without fix.
    CPPUNIT_ASSERT(xCellA1->getString().startsWith("Inside table"));
}

CPPUNIT_TEST_FIXTURE(TestWPC, WPC_CurvedConnector2)
{
    // The document has two shapes connected with a curvedConnector2 on a drawing canvas.
    // This connector is a single Bezier segment without handles.
    loadFromFile(u"WPC_CurvedConnector2.docx");

    // LO and OOXML differ in the position of the control points. LibreOffice uses 2/3 but OOXML
    // uses 1/2 of width or height. The path by LO looks more round.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xInfo(xGroup->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInfo->supportsService("com.sun.star.drawing.ConnectorShape"));

    uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(3), uno::UNO_QUERY);
    com::sun::star::drawing::ConnectorType eEdgeKind;
    xShapeProps->getPropertyValue(UNO_NAME_EDGEKIND) >>= eEdgeKind;
    CPPUNIT_ASSERT_EQUAL(drawing::ConnectorType::ConnectorType_CURVE, eEdgeKind);

    // Make sure the path is OOXML compatible
    drawing::PolyPolygonBezierCoords aPolyPolygonBezierCoords;
    xShapeProps->getPropertyValue("PolyPolygonBezier") >>= aPolyPolygonBezierCoords;
    drawing::PointSequence aPolygon = aPolyPolygonBezierCoords.Coordinates[0];
    // First control point. LO routing would generate point (4372|5584).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5149), aPolygon[1].Y);
    // Second control point. LO routing would generate point (5887|6458).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6645), aPolygon[2].X);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
