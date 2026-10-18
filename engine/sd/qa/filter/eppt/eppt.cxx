/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/XTheme.hpp>

#include <test/xmldocptr.hxx>
#include <docmodel/uno/UnoTheme.hxx>
#include <docmodel/theme/Theme.hxx>

using namespace ::com::sun::star;
using namespace ::cpo;

namespace
{
/// Covers sd/source/filter/eppt/ fixes.
class Test : public UnoApiXmlTest
{
public:
    Test();
};

Test::Test()
    : UnoApiXmlTest(u"/sd/qa/filter/eppt/data/"_ustr)
{
}

/// A theme with the colors 0x111111 for dk1 to 0xcccccc for folHlink.
std::shared_ptr<model::Theme> createTheme()
{
    auto pTheme = std::make_shared<model::Theme>(u"mytheme"_ustr);
    auto pColorSet = std::make_shared<model::ColorSet>(u"mycolorscheme"_ustr);
    pColorSet->add(model::ThemeColorType::Dark1, 0x111111);
    pColorSet->add(model::ThemeColorType::Light1, 0x222222);
    pColorSet->add(model::ThemeColorType::Dark2, 0x333333);
    pColorSet->add(model::ThemeColorType::Light2, 0x444444);
    pColorSet->add(model::ThemeColorType::Accent1, 0x555555);
    pColorSet->add(model::ThemeColorType::Accent2, 0x666666);
    pColorSet->add(model::ThemeColorType::Accent3, 0x777777);
    pColorSet->add(model::ThemeColorType::Accent4, 0x888888);
    pColorSet->add(model::ThemeColorType::Accent5, 0x999999);
    pColorSet->add(model::ThemeColorType::Accent6, 0xaaaaaa);
    pColorSet->add(model::ThemeColorType::Hyperlink, 0xbbbbbb);
    pColorSet->add(model::ThemeColorType::FollowedHyperlink, 0xcccccc);
    pTheme->setColorSet(pColorSet);
    return pTheme;
}

void setMasterPageTheme(uno::Reference<lang::XComponent> const& xComponent,
                        std::shared_ptr<model::Theme> const& pTheme)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);
    xMasterPage->setPropertyValue(u"Theme"_ustr, cpo::uno::Any(model::theme::createXTheme(pTheme)));
}

void addSolidFillStyle(model::FormatScheme& rFormatScheme, model::ComplexColor const& rColor)
{
    auto pFill = std::make_shared<model::SolidFill>();
    pFill->maColor = rColor;
    rFormatScheme.addFillStyle()->mpFill = pFill;
}

CPPUNIT_TEST_FIXTURE(Test, testOOXMLCustomShapeBitmapFill)
{
    // Save the bugdoc to PPT.
    loadFromFile(u"custom-shape-bitmap-fill.pptx");
    saveAndReload(TestFilter::PPT);

    // Check if the bitmap shape was lost.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: com.sun.star.drawing.GraphicObjectShape
    // - Actual  : com.sun.star.drawing.CustomShape
    // i.e. the custom shape geometry was kept, but the actual bitmap was lost.
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.GraphicObjectShape"_ustr, xShape->getShapeType());
}

CPPUNIT_TEST_FIXTURE(Test, testThemeExport)
{
    // Given a document with a master slide and a theme, lt1 is set to 0x000002:
    loadFromURL(u"private:factory/simpress"_ustr);
    setMasterPageTheme(mxComponent, createTheme());

    // Export to PPTX and load again:
    saveAndReload(TestFilter::PPTX);

    // Verify that this color is not lost:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/theme/theme1.xml"_ustr);
    assertXPath(pXmlDoc, "//a:clrScheme/a:lt1/a:srgbClr", "val",
                u"222222"); // expected color 22-22-22

    // Check the theme after loading again
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<drawing::XMasterPageTarget> xDrawPage(
            xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);
        uno::Reference<util::XTheme> xTheme(xMasterPage->getPropertyValue(u"Theme"_ustr),
                                            uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(true, xTheme.is());

        auto* pUnoTheme = dynamic_cast<UnoTheme*>(xTheme.get());
        CPPUNIT_ASSERT(pUnoTheme);
        auto pTheme = pUnoTheme->getTheme();

        CPPUNIT_ASSERT_EQUAL(u"mytheme"_ustr, pTheme->GetName());
        CPPUNIT_ASSERT_EQUAL(u"mycolorscheme"_ustr, pTheme->getColorSet()->getName());
        CPPUNIT_ASSERT_EQUAL(u"Office"_ustr, pTheme->getFontScheme().getName());
        CPPUNIT_ASSERT_EQUAL(u""_ustr, pTheme->getFormatScheme().getName());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testThemeExportKeepsTheTransformationsOfColors)
{
    // Given a theme with fill styles of an RGB color with an alpha and a hue offset, a system color
    // with a luminance modulation, and an RGB color with an inversion, which has no value:
    loadFromURL(u"private:factory/simpress"_ustr);
    auto pTheme = createTheme();
    model::FormatScheme& rFormatScheme = pTheme->getFormatScheme();

    model::ComplexColor aRGB = model::ComplexColor::createRGB(Color(0x12, 0x34, 0x56));
    aRGB.addTransformation({ model::TransformationType::Alpha, 5000 });
    aRGB.addTransformation({ model::TransformationType::HueOff, 1000 });
    addSolidFillStyle(rFormatScheme, aRGB);

    model::ComplexColor aSystem;
    aSystem.setSystemColor(model::SystemColorType::Window, 0xffffff);
    aSystem.addTransformation({ model::TransformationType::LumMod, 7500 });
    addSolidFillStyle(rFormatScheme, aSystem);

    model::ComplexColor aInverted = model::ComplexColor::createRGB(Color(0x65, 0x43, 0x21));
    aInverted.addTransformation({ model::TransformationType::Inv, 0 });
    addSolidFillStyle(rFormatScheme, aInverted);

    setMasterPageTheme(mxComponent, pTheme);

    // When exporting to PPTX:
    save(TestFilter::PPTX);

    // Then every color keeps its transformations, and the system color is a complete sysClr:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/theme/theme1.xml"_ustr);
    const OString sFill1 = "//a:fillStyleLst/a:solidFill[1]/a:srgbClr"_ostr;
    assertXPath(pXmlDoc, sFill1, "val", u"123456");
    assertXPath(pXmlDoc, sFill1 + "/a:alpha", "val", u"50000");
    assertXPath(pXmlDoc, sFill1 + "/a:hueOff", "val", u"10000");

    const OString sFill2 = "//a:fillStyleLst/a:solidFill[2]/a:sysClr"_ostr;
    assertXPath(pXmlDoc, sFill2, "val", u"window");
    assertXPath(pXmlDoc, sFill2 + "/a:lumMod", "val", u"75000");

    const OString sFill3 = "//a:fillStyleLst/a:solidFill[3]/a:srgbClr"_ostr;
    assertXPath(pXmlDoc, sFill3, "val", u"654321");
    assertXPath(pXmlDoc, sFill3 + "/a:inv", 1);
    assertXPathNoAttribute(pXmlDoc, sFill3 + "/a:inv", "val");
}

CPPUNIT_TEST_FIXTURE(Test, testLoopingFromAnimation)
{
    // Given a media shape that has an animation that specifies looping for the video:
    loadFromFile(u"video-loop.pptx");

    // When exporting that to PPTX:
    save(TestFilter::PPTX);

    // Then make sure that the "infinite" repeat count is written:
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '//p:cMediaNode/p:cTn' number of nodes is incorrect
    // i.e. the media node was lost on export, the video no longer looped.
    assertXPath(pXmlDoc, "//p:cMediaNode/p:cTn", "repeatCount", u"indefinite");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
