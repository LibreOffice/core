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

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <officecfg/Office/Common.hxx>
#include <rtl/math.hxx>
#include <svx/svdoashp.hxx>

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
    loadFromURL(u"tdf141463_GroupTransform.pptx");

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
    loadFromURL(u"multiple-group-shapes.docx");

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
    loadFromURL(u"customshape-position.docx");

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
    loadFromURL(u"tdf125582_TextOnCircle.pptx");

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
        // Without the fix in place width was 3639, but should be 4824 for 96dpi.
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(4824), aBoundRect.GetWidth(), 5);
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
    loadFromURL(u"tdf151008_eaVertAnchor.pptx");

    struct anchorDesc
    {
        OUString sShapeName;
        drawing::TextHorizontalAdjust eAnchorHori;
        drawing::TextVerticalAdjust eAnchorVert;
    };
    anchorDesc aExpected[6] = {
        { u"Right", drawing::TextHorizontalAdjust_RIGHT, drawing::TextVerticalAdjust_TOP },
        { u"Center", drawing::TextHorizontalAdjust_CENTER, drawing::TextVerticalAdjust_TOP },
        { u"Left", drawing::TextHorizontalAdjust_LEFT, drawing::TextVerticalAdjust_TOP },
        { u"RightMiddle", drawing::TextHorizontalAdjust_RIGHT, drawing::TextVerticalAdjust_CENTER },
        { u"CenterMiddle", drawing::TextHorizontalAdjust_CENTER,
          drawing::TextVerticalAdjust_CENTER },
        { u"LeftMiddle", drawing::TextHorizontalAdjust_LEFT, drawing::TextVerticalAdjust_CENTER }
    };
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
    loadFromURL(u"tdf151518_SmartArtTextLocation.docx");

    struct TextDistance
    {
        OUString sShapeName;
        sal_Int16 nSubShapeIndex;
        sal_Int32 nLowerDistance;
        sal_Int32 nUpperDistance;
    };
    TextDistance aExpected[4] = { { u"Diagram Target List", 2, 2979, 201 },
                                  { u"Diagram Nested Target", 1, 3203, 127 },
                                  { u"Diagram Stacked Venn", 1, 3112, -302 },
                                  { u"Diagram Grouped List", 1, 4106, 196 } };
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
    loadFromURL(u"tdf54095_SmartArtThemeTextColor.docx");
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
    sal_Int32 nActualColor{ 0 };
    xPortion->getPropertyValue("CharColor") >>= nActualColor;
    // Without fix the test would have failed with:
    // - Expected:  2050429 (0x1F497D)
    // - Actual  : 16777215 (0xFFFFFF), that is text was white
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x1F497D), nActualColor);

    // clrScheme. For map between name in docx and index from CharColorTheme see
    // oox::drawingml::Color::getSchemeColorIndex()
    // Without fix the color scheme was "lt1" (1) but should be "dk2" (2).
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2),
                         xPortion->getPropertyValue("CharColorTheme").get<sal_Int16>());

    if (!bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(false, pChange);
        pChange->commit();
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
