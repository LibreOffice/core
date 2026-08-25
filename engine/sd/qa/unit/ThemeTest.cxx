/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <tools/XPath.hxx>

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

Color GetShapeColor(const uno::Reference<beans::XPropertySet>& xShape,
                    const OUString& rPropertyName)
{
    Color aColor{};
    xShape->getPropertyValue(rPropertyName) >>= aColor;
    return aColor;
}

/// Get the solid fill color of xShape.
Color GetShapeFillColor(const uno::Reference<beans::XPropertySet>& xShape)
{
    return GetShapeColor(xShape, u"FillColor"_ustr);
}

model::ComplexColor GetComplexColor(const uno::Reference<beans::XPropertySet>& xShape,
                                    const OUString& rPropertyName)
{
    uno::Reference<util::XComplexColor> xComplexColor;
    CPPUNIT_ASSERT(xShape->getPropertyValue(rPropertyName) >>= xComplexColor);
    CPPUNIT_ASSERT(xComplexColor.is());
    return model::color::getFromXComplexColor(xComplexColor);
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
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4000),
                             aComplexColor.getTransformations()[0].mnValue);
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff,
                             aComplexColor.getTransformations()[1].meType);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6000),
                             aComplexColor.getTransformations()[1].mnValue);
    }

    // When changing the master slide of slide 1 to use the theme of the second master slide:
    uno::Reference<drawing::XMasterPageTarget> xDrawPage2(
        xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPage2(xDrawPage2->getMasterPage(), uno::UNO_QUERY);
    cpo::uno::Any aTheme = xMasterPage2->getPropertyValue(u"Theme"_ustr);

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

CPPUNIT_TEST_FIXTURE(ThemeTest, testExplicitShapeColorsRoundtrip)
{
    // A plain shape colour survives a round trip, and so does one that names a theme colour.
    createSdImpressDoc("ShapeExplicitColors.fodp");

    // The shapes inherit from a default drawing style that names a theme colour, and the export
    // repeats each shape's fill on a paragraph style of its own that is applied after it. Only an
    // exported document has those paragraph styles.
    saveAndReload(TestFilter::ODP);

    uno::Reference<beans::XPropertySet> xPlain(getShapeFromPage(0, 0));
    CPPUNIT_ASSERT_EQUAL(Color(0x7f59ae), GetShapeFillColor(xPlain));
    CPPUNIT_ASSERT_EQUAL(Color(0xe54b89), GetShapeColor(xPlain, u"LineColor"_ustr));
    for (OUString const& rPropertyName : { u"FillComplexColor"_ustr, u"LineComplexColor"_ustr })
    {
        auto aComplexColor = GetComplexColor(xPlain, rPropertyName);
        CPPUNIT_ASSERT(!aComplexColor.isUsed());
        CPPUNIT_ASSERT(!aComplexColor.isValidThemeType());
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Unknown, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT(aComplexColor.getTransformations().empty());
    }

    uno::Reference<beans::XPropertySet> xThemed(getShapeFromPage(1, 0));
    CPPUNIT_ASSERT_EQUAL(Color(0x008000), GetShapeFillColor(xThemed));
    {
        auto aComplexColor = GetComplexColor(xThemed, u"FillComplexColor"_ustr);
        CPPUNIT_ASSERT(aComplexColor.isValidThemeType());
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT(aComplexColor.getTransformations().empty());
    }

    // Accent 3 is grey, so half its luminance is half of each channel.
    uno::Reference<beans::XPropertySet> xShaded(getShapeFromPage(2, 0));
    CPPUNIT_ASSERT_EQUAL(Color(0x404040), GetShapeFillColor(xShaded));
    {
        auto aComplexColor = GetComplexColor(xShaded, u"FillComplexColor"_ustr);
        CPPUNIT_ASSERT(aComplexColor.isValidThemeType());
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT_EQUAL(size_t(1), aComplexColor.getTransformations().size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod,
                             aComplexColor.getTransformations()[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5000), aComplexColor.getTransformations()[0].mnValue);
    }

    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    tools::XPath aPath(pXmlDoc.get(), [](xmlXPathContextPtr pContext) {
        XmlTestTools::registerOOXMLNamespaces(pContext);
    });
    auto pFill = aPath.create("/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:solidFill/a:srgbClr");
    CPPUNIT_ASSERT_EQUAL(1, pFill->count());
    CPPUNIT_ASSERT_EQUAL(u"7F59AE"_ustr, pFill->attribute("val"));

    auto pLine = aPath.create("/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:ln/a:solidFill/a:srgbClr");
    CPPUNIT_ASSERT_EQUAL(1, pLine->count());
    CPPUNIT_ASSERT_EQUAL(u"E54B89"_ustr, pLine->attribute("val"));

    auto pThemed = aPath.create("/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:solidFill/a:schemeClr");
    CPPUNIT_ASSERT_EQUAL(1, pThemed->count());
    CPPUNIT_ASSERT_EQUAL(u"accent2"_ustr, pThemed->attribute("val"));
    CPPUNIT_ASSERT_EQUAL(0, pThemed->at(0)->countChildren());

    auto pShaded = aPath.create("/p:sld/p:cSld/p:spTree/p:sp[3]/p:spPr/a:solidFill/a:schemeClr");
    CPPUNIT_ASSERT_EQUAL(1, pShaded->count());
    CPPUNIT_ASSERT_EQUAL(u"accent3"_ustr, pShaded->attribute("val"));
    // A luminance modulation is a percentage on a 0..100000 scale here.
    auto pLumMod = aPath.create(pShaded, "/a:lumMod");
    CPPUNIT_ASSERT_EQUAL(1, pLumMod->count());
    CPPUNIT_ASSERT_EQUAL(u"50000"_ustr, pLumMod->attribute("val"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
