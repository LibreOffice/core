/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/theme/Theme.hxx>

#include <svx/unoapi.hxx>

#include <DrawDocShell.hxx>
#include <unomodel.hxx>
#include <sdpage.hxx>
#include <ViewShell.hxx>
#include <theme/ThemeColorChanger.hxx>

using namespace css;

class ThemeTest : public SdModelTestBase
{
public:
    ThemeTest()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

namespace
{
/// Get the character color of the first text portion in xShape.
Color GetShapeTextColor(const uno::Reference<text::XTextRange>& xShape)
{
    uno::Reference<container::XEnumerationAccess> xText(xShape->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xPara(xText->createEnumeration()->nextElement(),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPortion(xPara->createEnumeration()->nextElement(),
                                                 uno::UNO_QUERY);
    Color nColor{};
    xPortion->getPropertyValue(u"CharColor"_ustr) >>= nColor;
    return nColor;
}

/// Get the solid fill color of xShape.
Color GetShapeFillColor(const uno::Reference<beans::XPropertySet>& xShape)
{
    Color nColor{};
    xShape->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    return nColor;
}

} // end anonymous namespace

CPPUNIT_TEST_FIXTURE(ThemeTest, testThemeChange)
{
    // Given a document, with a first slide and blue shape text from theme:
    loadFromFile(u"theme.pptx");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    auto* pDocShell = pXImpressDocument->GetDocShell();

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    // The draw page also contains a group shape to make sure we don't crash on group shapes.
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes> xDrawPageShapes(xDrawPage, uno::UNO_QUERY);

    uno::Reference<text::XTextRange> xShape(xDrawPageShapes->getByIndex(0), uno::UNO_QUERY);
    // Blue.
    CPPUNIT_ASSERT_EQUAL(Color(0x4472c4), GetShapeTextColor(xShape));

    uno::Reference<text::XTextRange> xShape2(xDrawPageShapes->getByIndex(1), uno::UNO_QUERY);
    // Blue, lighter.
    CPPUNIT_ASSERT_EQUAL(Color(0xb4c7e7), GetShapeTextColor(xShape2));

    uno::Reference<text::XTextRange> xShape3(xDrawPageShapes->getByIndex(2), uno::UNO_QUERY);
    // Blue, darker.
    CPPUNIT_ASSERT_EQUAL(Color(0x2f5597), GetShapeTextColor(xShape3));

    // Shape fill:
    uno::Reference<beans::XPropertySet> xShape4(xDrawPageShapes->getByIndex(4), uno::UNO_QUERY);
    // Blue.
    CPPUNIT_ASSERT_EQUAL(Color(0x4472c4), GetShapeFillColor(xShape4));

    // The theme color of this filled shape is set by the PPTX import:
    {
        uno::Reference<util::XComplexColor> xComplexColor;
        CPPUNIT_ASSERT(xShape4->getPropertyValue(u"FillComplexColor"_ustr) >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aComplexColor.getThemeColorType());
    }

    uno::Reference<beans::XPropertySet> xShape5(xDrawPageShapes->getByIndex(5), uno::UNO_QUERY);
    // Blue, lighter.
    CPPUNIT_ASSERT_EQUAL(Color(0xb4c7e7), GetShapeFillColor(xShape5));
    // The theme index, and effects (lum mod, lum off) are set by the PPTX import:
    {
        uno::Reference<util::XComplexColor> xComplexColor;
        CPPUNIT_ASSERT(xShape5->getPropertyValue(u"FillComplexColor"_ustr) >>= xComplexColor);
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod,
                             aComplexColor.getTransformations()[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), aComplexColor.getTransformations()[0].mnValue);
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff,
                             aComplexColor.getTransformations()[1].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), aComplexColor.getTransformations()[1].mnValue);
    }

    // When changing the master slide of slide 1 to use the theme of the second master slide:
    uno::Reference<drawing::XMasterPageTarget> xDrawPage2(
        xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPage2(xDrawPage2->getMasterPage(), uno::UNO_QUERY);
    uno::Any aTheme = xMasterPage2->getPropertyValue(u"Theme"_ustr);

    uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);
    xMasterPage->setPropertyValue(u"Theme"_ustr, aTheme);

    css::uno::Reference<css::drawing::XDrawPage> xDrawPageMaster(xMasterPage, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPageMaster.is());
    auto* pMasterPage = GetSdrPageFromXDrawPage(xDrawPageMaster);
    auto pTheme = pMasterPage->getSdrPageProperties().getTheme();

    sd::ThemeColorChanger aChanger(pMasterPage, pDocShell);
    aChanger.apply(pTheme->getColorSet());

    // Then make sure the shape text color is now green:
    CPPUNIT_ASSERT_EQUAL(Color(0x90c226), GetShapeTextColor(xShape));
    // Green, lighter:
    CPPUNIT_ASSERT_EQUAL(Color(0xd5eda2), GetShapeTextColor(xShape2));
    // Green, darker.
    CPPUNIT_ASSERT_EQUAL(Color(0x6c911d), GetShapeTextColor(xShape3));
    // Shape fill:
    CPPUNIT_ASSERT_EQUAL(Color(0x90c226), GetShapeFillColor(xShape4));
    // Green, lighter:
    CPPUNIT_ASSERT_EQUAL(Color(0xd5eda2), GetShapeFillColor(xShape5));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
