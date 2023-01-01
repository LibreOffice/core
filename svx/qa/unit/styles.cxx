/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <docmodel/uno/UnoThemeColor.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/styles/ code.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("svx/qa/unit/data/")
    {
    }
};

/// Get the character color of the first text portion in xShape.
sal_Int32 GetShapeTextColor(const uno::Reference<text::XTextRange>& xShape)
{
    uno::Reference<container::XEnumerationAccess> xText(xShape->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xPara(xText->createEnumeration()->nextElement(),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPortion(xPara->createEnumeration()->nextElement(),
                                                 uno::UNO_QUERY);
    sal_Int32 nColor{};
    xPortion->getPropertyValue("CharColor") >>= nColor;
    return nColor;
}

/// Get the solid fill color of xShape.
sal_Int32 GetShapeFillColor(const uno::Reference<beans::XPropertySet>& xShape)
{
    sal_Int32 nColor{};
    xShape->getPropertyValue("FillColor") >>= nColor;
    return nColor;
}

CPPUNIT_TEST_FIXTURE(Test, testThemeChange)
{
    // Given a document, with a first slide and blue shape text from theme:
    loadFromURL(u"theme.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    // The draw page also contains a group shape to make sure we don't crash on group shapes.
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xDrawPageShapes(xDrawPage, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xDrawPageShapes->getByIndex(0), uno::UNO_QUERY);
    // Blue.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x4472c4), GetShapeTextColor(xShape));
    uno::Reference<text::XTextRange> xShape2(xDrawPageShapes->getByIndex(1), uno::UNO_QUERY);
    // Blue, lighter.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xb4c7e7), GetShapeTextColor(xShape2));
    uno::Reference<text::XTextRange> xShape3(xDrawPageShapes->getByIndex(2), uno::UNO_QUERY);
    // Blue, darker.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x2f5597), GetShapeTextColor(xShape3));
    // Shape fill:
    uno::Reference<beans::XPropertySet> xShape4(xDrawPageShapes->getByIndex(4), uno::UNO_QUERY);
    // Blue.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x4472c4), GetShapeFillColor(xShape4));

    // The theme color of this filled shape is set by the PPTX import:
    {
        uno::Reference<util::XThemeColor> xThemeColor;
        CPPUNIT_ASSERT(xShape4->getPropertyValue("FillColorThemeReference") >>= xThemeColor);
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aThemeColor.getType());
    }
    uno::Reference<beans::XPropertySet> xShape5(xDrawPageShapes->getByIndex(5), uno::UNO_QUERY);
    // Blue, lighter.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xb4c7e7), GetShapeFillColor(xShape5));
    // The theme index, and effects (lum mod, lum off) are set by the PPTX import:
    {
        uno::Reference<util::XThemeColor> xThemeColor;
        CPPUNIT_ASSERT(xShape5->getPropertyValue("FillColorThemeReference") >>= xThemeColor);
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod,
                             aThemeColor.getTransformations()[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), aThemeColor.getTransformations()[0].mnValue);
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff,
                             aThemeColor.getTransformations()[1].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), aThemeColor.getTransformations()[1].mnValue);
    }
    // When changing the master slide of slide 1 to use the theme of the second master slide:
    uno::Reference<drawing::XMasterPageTarget> xDrawPage2(
        xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPage2(xDrawPage2->getMasterPage(), uno::UNO_QUERY);
    uno::Any aTheme = xMasterPage2->getPropertyValue("Theme");
    uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);
    xMasterPage->setPropertyValue("Theme", aTheme);

    // Then make sure the shape text color is now green:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 9486886 (#90c226, green)
    // - Actual  : 4485828 (#4472c4, blue)
    // i.e. shape text was not updated on theme change.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x90c226), GetShapeTextColor(xShape));
    // Green, lighter:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14020002 (#d5eda2, light green)
    // - Actual  : 9486886 (#90c226, stock green)
    // i.e. the "light" effect on green was not applied.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xd5eda2), GetShapeTextColor(xShape2));
    // Green, darker.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x6c911d), GetShapeTextColor(xShape3));
    // Shape fill:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 9486886 (#90c226, green)
    // - Actual  : 4485828 (#4472c4, blue)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x90c226), GetShapeFillColor(xShape4));
    // Green, lighter:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14020002 (#d5eda2, light green)
    // - Actual  : 9486886 (#90c226, green)
    // i.e. the "light" effect on green was not applied.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xd5eda2), GetShapeFillColor(xShape5));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
