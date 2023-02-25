/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/util/Color.hpp>

#include <test/xmldocptr.hxx>
#include <docmodel/uno/UnoTheme.hxx>
#include <docmodel/theme/Theme.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers sd/source/filter/eppt/ fixes.
class Test : public UnoApiXmlTest
{
public:
    Test();
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
};

void Test::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    XmlTestTools::registerOOXMLNamespaces(pXmlXpathCtx);
}

Test::Test()
    : UnoApiXmlTest("/sd/qa/filter/eppt/data/")
{
}

CPPUNIT_TEST_FIXTURE(Test, testOOXMLCustomShapeBitmapFill)
{
    // Save the bugdoc to PPT.
    loadFromURL(u"custom-shape-bitmap-fill.pptx");
    saveAndReload("MS PowerPoint 97");

    // Check if the bitmap shape was lost.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: com.sun.star.drawing.GraphicObjectShape
    // - Actual  : com.sun.star.drawing.CustomShape
    // i.e. the custom shape geometry was kept, but the actual bitmap was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GraphicObjectShape"),
                         xShape->getShapeType());
}

CPPUNIT_TEST_FIXTURE(Test, testThemeExport)
{
    // Given a document with a master slide and a theme, lt1 is set to 0x000002:
    mxComponent = loadFromDesktop("private:factory/simpress");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);

    auto pTheme = std::make_shared<model::Theme>("mytheme");
    std::unique_ptr<model::ColorSet> pColorSet(new model::ColorSet("mycolorscheme"));
    pColorSet->add(model::ThemeColorType::Dark1, 0x1);
    pColorSet->add(model::ThemeColorType::Light1, 0x2);
    pColorSet->add(model::ThemeColorType::Dark2, 0x3);
    pColorSet->add(model::ThemeColorType::Light2, 0x4);
    pColorSet->add(model::ThemeColorType::Accent1, 0x5);
    pColorSet->add(model::ThemeColorType::Accent2, 0x6);
    pColorSet->add(model::ThemeColorType::Accent3, 0x7);
    pColorSet->add(model::ThemeColorType::Accent4, 0x8);
    pColorSet->add(model::ThemeColorType::Accent5, 0x9);
    pColorSet->add(model::ThemeColorType::Accent6, 0xa);
    pColorSet->add(model::ThemeColorType::Hyperlink, 0xb);
    pColorSet->add(model::ThemeColorType::FollowedHyperlink, 0xc);
    pTheme->SetColorSet(std::move(pColorSet));

    xMasterPage->setPropertyValue("Theme", uno::Any(model::theme::createXTheme(pTheme)));

    // When exporting to PPTX:
    save("Impress Office Open XML");

    // Then verify that this color is not lost:
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/theme/theme1.xml");
    assertXPath(pXmlDoc, "//a:clrScheme/a:lt1/a:srgbClr", "val", "000002");
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//a:clrScheme/a:lt1/a:srgbClr' number of nodes is incorrect
    // i.e. the RGB color was lost on export.
}

CPPUNIT_TEST_FIXTURE(Test, testLoopingFromAnimation)
{
    // Given a media shape that has an animation that specifies looping for the video:
    loadFromURL(u"video-loop.pptx");

    // When exporting that to PPTX:
    save("Impress Office Open XML");

    // Then make sure that the "infinite" repeat count is written:
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '//p:cMediaNode/p:cTn' number of nodes is incorrect
    // i.e. the media node was lost on export, the video no longer looped.
    assertXPath(pXmlDoc, "//p:cMediaNode/p:cTn", "repeatCount", "indefinite");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
