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
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeMetalType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <osl/process.h>
#include <unotools/tempfile.hxx>
#include <unotools/saveopt.hxx>
#include <svx/unopage.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdomedia.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/uno/UnoTheme.hxx>
#include <docmodel/theme/Theme.hxx>

using namespace ::com::sun::star;

/// Covers xmloff/source/draw/ fixes.
class XmloffDrawTest : public UnoApiXmlTest
{
public:
    XmloffDrawTest();
    uno::Reference<drawing::XShape> getShape(sal_uInt8 nShapeIndex);

    uno::Reference<beans::XPropertySet>
    getShapeTextPortion(sal_uInt32 nIndex, uno::Reference<drawing::XShape> const& xShape)
    {
        uno::Reference<beans::XPropertySet> xPortion;

        uno::Reference<container::XEnumerationAccess> xEnumAccess(xShape, uno::UNO_QUERY);
        if (!xEnumAccess->hasElements())
            return xPortion;
        uno::Reference<container::XEnumeration> xEnum(xEnumAccess->createEnumeration());
        uno::Reference<text::XTextContent> xTextContent;
        xEnum->nextElement() >>= xTextContent;
        if (!xTextContent.is())
            return xPortion;

        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextContent, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());
        sal_uInt32 nCurrent = 0;
        xPortion = uno::Reference<beans::XPropertySet>(xParaEnum->nextElement(), uno::UNO_QUERY);
        while (nIndex != nCurrent)
        {
            ++nCurrent;
            xPortion
                = uno::Reference<beans::XPropertySet>(xParaEnum->nextElement(), uno::UNO_QUERY);
        }
        return xPortion;
    }
};

XmloffDrawTest::XmloffDrawTest()
    : UnoApiXmlTest(u"/xmloff/qa/unit/data/"_ustr)
{
}

uno::Reference<drawing::XShape> XmloffDrawTest::getShape(sal_uInt8 nShapeIndex)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(nShapeIndex),
                                           uno::UNO_QUERY_THROW);
    return xShape;
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTextBoxLoss)
{
    // Load a document that has a shape with a textbox in it. Save it to ODF and reload.
    loadFromFile(u"textbox-loss.docx");
    saveAndReload(u"impress8"_ustr);

    // Make sure that the shape is still a textbox.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    bool bTextBox = false;
    xShape->getPropertyValue(u"TextBox"_ustr) >>= bTextBox;

    // Without the accompanying fix in place, this test would have failed, as the shape only had
    // editeng text, losing the image part of the shape text.
    CPPUNIT_ASSERT(bTextBox);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTdf141301_Extrusion_Angle)
{
    // Load a document that has a custom shape with extrusion direction as set by LO as its default.
    loadFromFile(u"tdf141301_Extrusion_Skew.odg");

    // Prepare use of XPath
    save(u"draw8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    // Without fix draw:extrusion-skew="50 -135" was not written to file although "50 -135" is not
    // default in ODF, but only default inside LO.
    assertXPath(pXmlDoc, "//draw:enhanced-geometry", "extrusion-skew", u"50 -135");
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeExport)
{
    // Create an Impress document which has a master page which has a theme associated with it.
    loadFromURL(u"private:factory/simpress"_ustr);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);

    auto pTheme = std::make_shared<model::Theme>("mytheme");
    auto pColorSet = std::make_shared<model::ColorSet>("mycolorscheme");
    pColorSet->add(model::ThemeColorType::Dark1, 0x0);
    pColorSet->add(model::ThemeColorType::Light1, 0x1);
    pColorSet->add(model::ThemeColorType::Dark2, 0x2);
    pColorSet->add(model::ThemeColorType::Light2, 0x3);
    pColorSet->add(model::ThemeColorType::Accent1, 0x4);
    pColorSet->add(model::ThemeColorType::Accent2, 0x5);
    pColorSet->add(model::ThemeColorType::Accent3, 0x6);
    pColorSet->add(model::ThemeColorType::Accent4, 0x7);
    pColorSet->add(model::ThemeColorType::Accent5, 0x8);
    pColorSet->add(model::ThemeColorType::Accent6, 0x9);
    pColorSet->add(model::ThemeColorType::Hyperlink, 0xa);
    pColorSet->add(model::ThemeColorType::FollowedHyperlink, 0xb);
    pTheme->setColorSet(pColorSet);

    uno::Reference<util::XTheme> xTheme = model::theme::createXTheme(pTheme);
    xMasterPage->setPropertyValue(u"Theme"_ustr, uno::Any(xTheme));

    // Export to ODP:
    save(u"impress8"_ustr);

    // Check if the 12 colors are written in the XML:
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 12
    // - Actual  : 0
    // - XPath '//style:master-page/loext:theme/loext:color-table/loext:color' number of nodes is incorrect
    // i.e. the theme was lost on exporting to ODF.
    assertXPath(pXmlDoc, "//style:master-page/loext:theme/loext:theme-colors/loext:color", 12);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testVideoSnapshot)
{
    // Execute ODP import:
    loadFromFile(u"video-snapshot.odp");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDrawPage.is());
    auto pUnoPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    SdrPage* pSdrPage = pUnoPage->GetSdrPage();
    auto pMedia = dynamic_cast<SdrMediaObj*>(pSdrPage->GetObj(0));

    // Check that the preview was imported:
    const avmedia::MediaItem& rItem = pMedia->getMediaProperties();
    const Graphic& rGraphic = rItem.getGraphic();
    CPPUNIT_ASSERT(!rGraphic.IsNone());

    // Check that the crop was imported:
    const text::GraphicCrop& rCrop = rItem.getCrop();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), rCrop.Top);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), rCrop.Bottom);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1356), rCrop.Left);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1356), rCrop.Right);

    // Execute ODP export:
    save(u"impress8"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Check that the preview was exported:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//draw:frame[@draw:style-name='gr1']/draw:image' number of nodes is incorrect
    // i.e. the preview wasn't exported to ODP.
    assertXPath(pXmlDoc, "//draw:frame[@draw:style-name='gr1']/draw:image", "href",
                u"Pictures/MediaPreview1.png");
    // Check that the crop was exported:
    assertXPath(pXmlDoc, "//style:style[@style:name='gr1']/style:graphic-properties", "clip",
                u"rect(0cm, 1.356cm, 0cm, 1.356cm)");
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeImport)
{
    // Given a document that has a master page with a theme associated:
    loadFromFile(u"theme.fodp");

    // Then make sure the doc model has a master page with a theme:
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
    CPPUNIT_ASSERT(pTheme);

    CPPUNIT_ASSERT_EQUAL(u"Office Theme"_ustr, pTheme->GetName());
    auto pColorSet = pTheme->getColorSet();
    CPPUNIT_ASSERT(pColorSet);
    CPPUNIT_ASSERT_EQUAL(u"Office"_ustr, pColorSet->getName());

    CPPUNIT_ASSERT_EQUAL(Color(0x954F72),
                         pColorSet->getColor(model::ThemeColorType::FollowedHyperlink));
}

namespace
{
void checkFillAndLineComplexColors(uno::Reference<drawing::XShape> const& xShape)
{
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    {
        uno::Reference<util::XComplexColor> xComplexColor;
        xShapeProperties->getPropertyValue(u"FillComplexColor"_ustr) >>= xComplexColor;
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aComplexColor.getTransformations().size());
        auto const& rTrans1 = aComplexColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), rTrans1.mnValue);
        auto const& rTrans2 = aComplexColor.getTransformations()[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), rTrans2.mnValue);
    }
    {
        uno::Reference<util::XComplexColor> xComplexColor;
        xShapeProperties->getPropertyValue(u"LineComplexColor"_ustr) >>= xComplexColor;
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aComplexColor.getTransformations().size());
        auto const& rTrans1 = aComplexColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), rTrans1.mnValue);
        auto const& rTrans2 = aComplexColor.getTransformations()[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), rTrans2.mnValue);
    }
}

} // end anonymous ns

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testFillAndLineThemeColorExportImport)
{
    loadFromFile(u"FillAndStrokeThemeColorTest.fodp");

    checkFillAndLineComplexColors(getShape(0));

    save(u"impress8"_ustr);

    loadFromURL(maTempFile.GetURL());

    checkFillAndLineComplexColors(getShape(0));
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTextAndFillThemeColorExportImport)
{
    // Given a document that refers to a theme color:
    loadFromFile(u"Reference-ThemeColors-TextAndFill.pptx");
    saveAndReload(u"impress8"_ustr);

    // Make sure the export result has the theme reference:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    // Text color
    OString aStyle1(
        "//style:style[@style:name='T2']/style:text-properties/loext:char-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type", u"theme");
    assertXPath(pXmlDoc, aStyle1, "theme-type", u"accent3");
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[1]", "type", u"lummod");
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[1]", "value", u"2000");
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[2]", "type", u"lumoff");
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[2]", "value", u"8000");

    OString aStyle2(
        "//style:style[@style:name='T3']/style:text-properties/loext:char-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type", u"theme");
    assertXPath(pXmlDoc, aStyle2, "theme-type", u"accent3");
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[1]", "type", u"lummod");
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[1]", "value", u"6000");
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[2]", "type", u"lumoff");
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[2]", "value", u"4000");

    OString aStyle3(
        "//style:style[@style:name='T4']/style:text-properties/loext:char-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type", u"theme");
    assertXPath(pXmlDoc, aStyle3, "theme-type", u"accent3");
    assertXPath(pXmlDoc, aStyle3 + "/loext:transformation[1]", "type", u"lummod");
    assertXPath(pXmlDoc, aStyle3 + "/loext:transformation[1]", "value", u"5000");

    // Shapes fill color
    OString aShape1("//style:style[@style:name='gr1']/style:graphic-properties/"
                    "loext:fill-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type", u"theme");
    assertXPath(pXmlDoc, aShape1, "theme-type", u"accent2");
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[1]", "type", u"lummod");
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[1]", "value", u"2000");
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[2]", "type", u"lumoff");
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[2]", "value", u"8000");

    OString aShape2("//style:style[@style:name='gr2']/style:graphic-properties/"
                    "loext:fill-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type", u"theme");
    assertXPath(pXmlDoc, aShape2, "theme-type", u"accent2");
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[1]", "type", u"lummod");
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[1]", "value", u"6000");
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[2]", "type", u"lumoff");
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[2]", "value", u"4000");

    OString aShape3("//style:style[@style:name='gr3']/style:graphic-properties/"
                    "loext:fill-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type", u"theme");
    assertXPath(pXmlDoc, aShape3, "theme-type", u"accent2");
    assertXPath(pXmlDoc, aShape3 + "/loext:transformation[1]", "type", u"lummod");
    assertXPath(pXmlDoc, aShape3 + "/loext:transformation[1]", "value", u"5000");

    // check fill color theme
    {
        uno::Reference<drawing::XShape> xShape(getShape(0));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        uno::Reference<util::XComplexColor> xComplexColor;
        xShapeProperties->getPropertyValue(u"FillComplexColor"_ustr) >>= xComplexColor;
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aComplexColor.getTransformations().size());
        auto const& rTrans1 = aComplexColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), rTrans1.mnValue);
        auto const& rTrans2 = aComplexColor.getTransformations()[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(8000), rTrans2.mnValue);
    }
    {
        uno::Reference<drawing::XShape> xShape(getShape(1));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        uno::Reference<util::XComplexColor> xComplexColor;
        xShapeProperties->getPropertyValue(u"FillComplexColor"_ustr) >>= xComplexColor;
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aComplexColor.getTransformations().size());
        auto const& rTrans1 = aComplexColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), rTrans1.mnValue);
        auto const& rTrans2 = aComplexColor.getTransformations()[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), rTrans2.mnValue);
    }
    {
        uno::Reference<drawing::XShape> xShape(getShape(2));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        uno::Reference<util::XComplexColor> xComplexColor;
        xShapeProperties->getPropertyValue(u"FillComplexColor"_ustr) >>= xComplexColor;
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT_EQUAL(size_t(1), aComplexColor.getTransformations().size());
        auto const& rTrans1 = aComplexColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(5000), rTrans1.mnValue);
    }

    // Char color theme
    // Shape 4
    {
        // Check the first text portion properties
        uno::Reference<drawing::XShape> xShape(getShape(3));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xPortion = getShapeTextPortion(0, xShape);
        CPPUNIT_ASSERT(xPortion.is());
        uno::Reference<util::XComplexColor> xComplexColor;
        xPortion->getPropertyValue(u"CharComplexColor"_ustr) >>= xComplexColor;
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getThemeColorType());
        auto const& rTransforms = aComplexColor.getTransformations();
        CPPUNIT_ASSERT_EQUAL(size_t(2), rTransforms.size());
        auto const& rTrans1 = rTransforms[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), rTrans1.mnValue);
        auto const& rTrans2 = rTransforms[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(8000), rTrans2.mnValue);
    }
    // Shape 5
    {
        // Check the first text portion properties
        uno::Reference<drawing::XShape> xShape(getShape(4));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xPortion = getShapeTextPortion(0, xShape);
        CPPUNIT_ASSERT(xPortion.is());
        uno::Reference<util::XComplexColor> xComplexColor;
        xPortion->getPropertyValue(u"CharComplexColor"_ustr) >>= xComplexColor;
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getThemeColorType());
        auto const& rTransforms = aComplexColor.getTransformations();
        CPPUNIT_ASSERT_EQUAL(size_t(2), rTransforms.size());
        auto const& rTrans1 = rTransforms[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), rTrans1.mnValue);
        auto const& rTrans2 = rTransforms[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), rTrans2.mnValue);
    }
    // Shape 6
    {
        // Check the first text portion properties
        uno::Reference<drawing::XShape> xShape(getShape(5));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xPortion = getShapeTextPortion(0, xShape);
        CPPUNIT_ASSERT(xPortion.is());
        uno::Reference<util::XComplexColor> xComplexColor;
        xPortion->getPropertyValue(u"CharComplexColor"_ustr) >>= xComplexColor;
        CPPUNIT_ASSERT(xComplexColor.is());
        auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT_EQUAL(size_t(1), aComplexColor.getTransformations().size());
        auto const& rTrans1 = aComplexColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(5000), rTrans1.mnValue);
    }
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeColor_ShapeFill)
{
    loadFromFile(u"ReferenceShapeFill.pptx");
    saveAndReload(u"impress8"_ustr);

    // check fill color theme
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    uno::Reference<util::XComplexColor> xComplexColor;
    xShapeProperties->getPropertyValue(u"FillComplexColor"_ustr) >>= xComplexColor;
    CPPUNIT_ASSERT(xComplexColor.is());
    auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6, aComplexColor.getThemeColorType());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aComplexColor.getTransformations().size());
    CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod,
                         aComplexColor.getTransformations()[0].meType);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7500), aComplexColor.getTransformations()[0].mnValue);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTableInShape)
{
    // Given a document with a shape with a "FrameX" parent style (starts with Frame, but is not
    // Frame):
    loadFromFile(u"table-in-shape.fodt");

    // Then make sure the table inside the shape is not lost:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xShape->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xEnum = xText->createEnumeration();
    uno::Reference<text::XTextTable> xTable(xEnum->nextElement(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have crashed, as xTable was an empty
    // reference, i.e. the table inside the shape was lost.
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"A1"_ustr, xCell->getString());
}

// Tests for save/load of new (LO 7.4) attribute loext:extrusion-metal-type
// Since ODF 1.4 it is available as draw:extrusion-metal-type in the standard.
namespace
{
void lcl_assertMetalProperties(std::string_view sInfo, uno::Reference<drawing::XShape>& rxShape,
                               sal_Int16 nExpectedMetalType)
{
    uno::Reference<beans::XPropertySet> xShapeProps(rxShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<beans::PropertyValue> aExtrusionSeq;
    aGeoPropMap.getValue(u"Extrusion"_ustr) >>= aExtrusionSeq;
    comphelper::SequenceAsHashMap aExtrusionPropMap(aExtrusionSeq);

    bool bIsMetal(false);
    aExtrusionPropMap.getValue(u"Metal"_ustr) >>= bIsMetal;
    OString sMsg = OString::Concat(sInfo) + " Metal";
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), bIsMetal);

    sal_Int16 nMetalType(-1);
    aExtrusionPropMap.getValue(u"MetalType"_ustr) >>= nMetalType;
    sMsg = OString::Concat(sInfo) + " MetalType";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sMsg.getStr(), nExpectedMetalType, nMetalType);
}
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionMetalTypeExtended)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });
    loadFromFile(u"tdf145700_3D_metal_type_MSCompatible.doc");
    // verify properties
    uno::Reference<drawing::XShape> xShape(getShape(0));
    lcl_assertMetalProperties("from doc", xShape,
                              css::drawing::EnhancedCustomShapeMetalType::MetalMSCompatible);

    // Test, that attribute is written with 'draw' namespace in ODF version LATEST
    saveAndReload(u"writer8"_ustr);

    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry", "extrusion-metal", u"true");
    assertXPath(pXmlDoc,
                "//draw:enhanced-geometry[@draw:extrusion-metal-type='loext:MetalMSCompatible']");

    // verify properties
    uno::Reference<drawing::XShape> xShapeReload(getShape(0));
    lcl_assertMetalProperties("from LATEST", xShapeReload,
                              css::drawing::EnhancedCustomShapeMetalType::MetalMSCompatible);

    // Test, that attribute is written with 'loext' namespace in extended version before ODF 1.4
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013_EXTENDED);
    // As of Nov 2024, validating against a version other than LATEST is not implemented.
    skipValidation();
    saveAndReload(u"writer8"_ustr);

    // assert XML.
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry", "extrusion-metal", u"true");
    assertXPath(pXmlDoc,
                "//draw:enhanced-geometry[@loext:extrusion-metal-type='loext:MetalMSCompatible']");

    // verify properties
    uno::Reference<drawing::XShape> xShapeReload2(getShape(0));
    lcl_assertMetalProperties("from ODF 1.3 extended", xShapeReload2,
                              css::drawing::EnhancedCustomShapeMetalType::MetalMSCompatible);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionMetalTypeStrict)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });
    loadFromFile(u"tdf145700_3D_metal_type_MSCompatible.doc");

    // save in ODF 1.4 strict and test that new attribute is written.
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_014);
    save(u"writer8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry", "extrusion-metal", u"true");
    assertXPath(pXmlDoc,
                "//draw:enhanced-geometry[@draw:extrusion-metal-type='loext:MetalMSCompatible']");

    // save in ODF 1.3 strict and test that new attribute is not written.
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013);
    // As of Nov 2024, validating against a version other than LATEST is not implemented.
    skipValidation();
    save(u"writer8"_ustr);
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry", "extrusion-metal", u"true");
    assertXPathNoAttribute(pXmlDoc, "//draw:enhanced-geometry", "extrusion-metal-type");
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionMetalTypeODF)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });
    loadFromFile(u"tdf162686_3D_metal_type_ODF.fods");
    // verify properties
    uno::Reference<drawing::XShape> xShape(getShape(0));
    lcl_assertMetalProperties("from doc", xShape,
                              css::drawing::EnhancedCustomShapeMetalType::MetalODF);

    // Test, that attribute is written with 'draw' namespace in ODF version LATEST
    saveAndReload(u"calc8"_ustr);

    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry", "extrusion-metal", u"true");
    assertXPath(pXmlDoc, "//draw:enhanced-geometry[@draw:extrusion-metal-type='draw:MetalODF']");

    // verify properties
    uno::Reference<drawing::XShape> xShapeReload(getShape(0));
    lcl_assertMetalProperties("from LATEST", xShapeReload,
                              css::drawing::EnhancedCustomShapeMetalType::MetalODF);

    // Test, that export in ODFVER_014 is valid. Needs adaption, when ODF 1.5 comes out.
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_014);
    saveAndReload(u"calc8"_ustr);

    // Test, that attribute is written with 'loext' namespace in extended version before ODF 1.4
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013_EXTENDED);
    // As of Nov 2024, validating against a version other than LATEST is not implemented.
    skipValidation();
    saveAndReload(u"calc8"_ustr);

    // assert XML.
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry[@loext:extrusion-metal-type='draw:MetalODF']");

    // verify properties
    uno::Reference<drawing::XShape> xShapeReload2(getShape(0));
    lcl_assertMetalProperties("from ODF 1.3 extended", xShapeReload2,
                              css::drawing::EnhancedCustomShapeMetalType::MetalODF);

    // Test, that attribute is not written at all in strict version before ODF 1.4
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013);
    save(u"calc8"_ustr);
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPathNoAttribute(pXmlDoc, "//draw:enhanced-geometry", "extrusion-metal-type");
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testHandlePosition)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });
    loadFromFile(u"tdf162691_handle_position.fodt");

    save(u"writer8"_ustr);
    static constexpr OString sXPath("/office:document-content/office:body/office:text/text:p/"
                                    "draw:custom-shape/draw:enhanced-geometry/draw:handle"_ostr);
    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    assertXPath(pXmlDoc, sXPath, "handle-position", u"$0 10800");
    assertXPath(pXmlDoc, sXPath, "handle-position-x", u"$0");
    assertXPath(pXmlDoc, sXPath, "handle-position-y", u"10800");

    // assert that new attributes are not written in older ODF versions and old one is written.
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013_EXTENDED);
    // As of Nov 2024, validating against a version other than LATEST is not implemented.
    skipValidation();
    save(u"writer8"_ustr);
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, sXPath, "handle-position", u"$0 10800");
    assertXPathNoAttribute(pXmlDoc, sXPath, "handle-position-x");
    assertXPathNoAttribute(pXmlDoc, sXPath, "handle-position-y");
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testHandlePolar)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });
    loadFromFile(u"tdf162691_handle_polar.fodt");

    save(u"writer8"_ustr);

    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    static constexpr OString sXPath("/office:document-content/office:body/office:text/text:p/"
                                    "draw:custom-shape/draw:enhanced-geometry/draw:handle"_ostr);
    assertXPath(pXmlDoc, sXPath, "handle-position", u"9000 $0");
    assertXPath(pXmlDoc, sXPath, "handle-polar", u"10800 1000");
    assertXPath(pXmlDoc, sXPath, "handle-polar-angle", u"$0");
    assertXPath(pXmlDoc, sXPath, "handle-polar-radius", u"9000");
    assertXPath(pXmlDoc, sXPath, "handle-polar-pole-x", u"10800");
    assertXPath(pXmlDoc, sXPath, "handle-polar-pole-y", u"1000");

    // assert that new attributes are not written in older ODF versions and old ones are written.
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013_EXTENDED);
    // As of Nov 2024, validating against a version other than LATEST is not implemented.
    skipValidation();
    save(u"writer8"_ustr);
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, sXPath, "handle-position", u"9000 $0");
    assertXPath(pXmlDoc, sXPath, "handle-polar", u"10800 1000");
    assertXPathNoAttribute(pXmlDoc, sXPath, "handle-polar-pole-x");
    assertXPathNoAttribute(pXmlDoc, sXPath, "handle-polar-pole-y");
    assertXPathNoAttribute(pXmlDoc, sXPath, "handle-polar-angle");
    assertXPathNoAttribute(pXmlDoc, sXPath, "handle-polar-radius");
}

namespace
{
void lcl_assertSpecularityProperty(std::string_view sInfo, uno::Reference<drawing::XShape>& rxShape)
{
    uno::Reference<beans::XPropertySet> xShapeProps(rxShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<beans::PropertyValue> aExtrusionSeq;
    aGeoPropMap.getValue(u"Extrusion"_ustr) >>= aExtrusionSeq;
    comphelper::SequenceAsHashMap aExtrusionPropMap(aExtrusionSeq);

    double fSpecularity(-1.0);
    aExtrusionPropMap.getValue(u"Specularity"_ustr) >>= fSpecularity;
    OString sMsg = OString::Concat(sInfo) + "Specularity";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sMsg.getStr(), 122.0703125, fSpecularity);
}
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionSpecularityExtended)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });
    loadFromFile(u"tdf147580_extrusion-specularity.doc");
    // verify property
    uno::Reference<drawing::XShape> xShape(getShape(0));
    lcl_assertSpecularityProperty("from doc", xShape);

    // Test, that attribute is written in draw namespace with value 100% and in loext namespace with
    // value 122.0703125%.
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013_EXTENDED);
    // As of Nov 2024, validating against a version other than LATEST is not implemented.
    skipValidation();
    saveAndReload(u"writer8"_ustr);

    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry[@draw:extrusion-specularity='100%']");
    assertXPath(pXmlDoc,
                "//draw:enhanced-geometry[@loext:extrusion-specularity-loext='122.0703125%']");

    // verify properties
    uno::Reference<drawing::XShape> xShapeReload(getShape(0));
    lcl_assertSpecularityProperty("from ODF 1.3 extended", xShapeReload);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionSpecularity)
{
    loadFromFile(u"tdf147580_extrusion-specularity.doc");
    // verify property
    uno::Reference<drawing::XShape> xShape(getShape(0));
    lcl_assertSpecularityProperty("from doc", xShape);

    saveAndReload(u"writer8"_ustr);

    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry[@draw:extrusion-specularity='122.0703125%']");

    // verify properties
    uno::Reference<drawing::XShape> xShapeReload(getShape(0));
    lcl_assertSpecularityProperty("from ODF 1.4", xShapeReload);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionSpecularityStrict)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });
    loadFromFile(u"tdf147580_extrusion-specularity.doc");

    // The file has c3DSpecularAmt="80000" which results internally in specularity=122%.
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_014);
    save(u"writer8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry[@draw:extrusion-specularity='122.0703125%']");

    // Save to ODF 1.3 strict and make sure draw:extrusion-specularity="100%" is written and
    // loext:extrusion-specularity is not written.
    // As of Nov 2024, validating against a version other than LATEST is not implemented.
    skipValidation();
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013);
    save(u"writer8"_ustr);
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry[@loext:extrusion-specularity]", 0);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry[@draw:extrusion-specularity='100%']");
}

namespace
{
bool lcl_getShapeSegments(uno::Sequence<drawing::EnhancedCustomShapeSegment>& rSegments,
                          const uno::Reference<drawing::XShape>& xShape)
{
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY_THROW);
    uno::Any anotherAny = xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr);
    uno::Sequence<beans::PropertyValue> aCustomShapeGeometry;
    if (!(anotherAny >>= aCustomShapeGeometry))
        return false;
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : aCustomShapeGeometry)
    {
        if (rProp.Name == "Path")
        {
            rProp.Value >>= aPathProps;
            break;
        }
    }

    for (beans::PropertyValue const& rProp : aPathProps)
    {
        if (rProp.Name == "Segments")
        {
            rProp.Value >>= rSegments;
            break;
        }
    }
    if (rSegments.getLength() > 2)
        return true;
    else
        return false;
}
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTdf148714_CurvedArrowsOld)
{
    // Load a document with CurveArrow shapes with faulty path as written by older LO versions.
    loadFromFile(u"tdf148714_CurvedArrowsOld.odp");

    //  Make sure, that the error has been corrected on opening.
    for (sal_Int32 nShapeIndex = 0; nShapeIndex < 4; nShapeIndex++)
    {
        uno::Reference<drawing::XShape> xShape(getShape(nShapeIndex));
        uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
        CPPUNIT_ASSERT(lcl_getShapeSegments(aSegments, xShape));

        if (nShapeIndex == 0 || nShapeIndex == 3)
        {
            // curvedDownArrow or curvedLeftArrow. Segments should start with VW. Without fix it was
            // V with count 2, which means VV.
            CPPUNIT_ASSERT_EQUAL(
                sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARC),
                aSegments[0].Command);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aSegments[0].Count);
            CPPUNIT_ASSERT_EQUAL(
                sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO),
                aSegments[1].Command);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aSegments[1].Count);
        }
        else
        {
            // curvedUpArrow or curvedRightArrow. Segments should start with BA. Without fix is was
            // B with count 2, which means BB.
            CPPUNIT_ASSERT_EQUAL(sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::ARC),
                                 aSegments[0].Command);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aSegments[0].Count);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::ARCTO),
                                 aSegments[1].Command);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aSegments[1].Count);
        }
    }
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTextRotationPlusPre)
{
    loadFromFile(u"tdf149551_verticalText.pptx");
    // The file has a shape with attribute vert="vert" in <bodyPr> element. That generates a
    // TextPreRotateAngle attribute in CustomShapeGeometry.

    // Add a TextRotateAngle attribute.
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeomSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeomSeq;
    auto aGeomVec(comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(aGeomSeq));
    aGeomVec.push_back(comphelper::makePropertyValue(u"TextRotateAngle"_ustr, sal_Int32(45)));
    aGeomSeq = comphelper::containerToSequence(aGeomVec);
    xShapeProps->setPropertyValue(u"CustomShapeGeometry"_ustr, uno::Any(aGeomSeq));

    // Save to ODF. Without the fix, a file format error was produced, because attribute
    // draw:text-rotate-angle was written twice, one from TextPreRotateAngle and the other from
    // TextRotateAngle.
    // This should already catch the format error, but does not, see tdf#149567
    // But reload catches it.
    saveAndReload(u"writer8"_ustr);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTdf156975_ThemeExport)
{
    // It tests, that a theme is written to master page in Draw documents.
    // Without fix for tdf#156975 it was not written at all.
    // The test needs to be adapted, when themes are available in ODF.

    loadFromURL(u"private:factory/sdraw"_ustr);
    // generate a theme to be sure we have got one and know the values
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPageProps(xDrawPage->getMasterPage(),
                                                         uno::UNO_QUERY);

    auto pTheme = std::make_shared<model::Theme>("Custom");
    auto pColorSet = std::make_shared<model::ColorSet>("My Colors");
    pColorSet->add(model::ThemeColorType::Dark1, 0x000000);
    pColorSet->add(model::ThemeColorType::Light1, 0xffff11);
    pColorSet->add(model::ThemeColorType::Dark2, 0x002200);
    pColorSet->add(model::ThemeColorType::Light2, 0xff33ff);
    pColorSet->add(model::ThemeColorType::Accent1, 0x440000);
    pColorSet->add(model::ThemeColorType::Accent2, 0x005500);
    pColorSet->add(model::ThemeColorType::Accent3, 0x000066);
    pColorSet->add(model::ThemeColorType::Accent4, 0x777700);
    pColorSet->add(model::ThemeColorType::Accent5, 0x880088);
    pColorSet->add(model::ThemeColorType::Accent6, 0x009999);
    pColorSet->add(model::ThemeColorType::Hyperlink, 0x0a0a0a);
    pColorSet->add(model::ThemeColorType::FollowedHyperlink, 0xb0b0b0);
    pTheme->setColorSet(pColorSet);

    uno::Reference<util::XTheme> xTheme = model::theme::createXTheme(pTheme);
    xMasterPageProps->setPropertyValue(u"Theme"_ustr, uno::Any(xTheme));

    // save as odg
    save(u"draw8"_ustr);

    // and check the markup.
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    static constexpr OString sThemePath
        = "//office:master-styles/style:master-page/loext:theme"_ostr;
    assertXPath(pXmlDoc, sThemePath, 1);
    assertXPath(pXmlDoc, sThemePath + "[@loext:name='Custom']");

    const OString sThemeColorsPath = sThemePath + "/loext:theme-colors";
    assertXPath(pXmlDoc, sThemeColorsPath, 1);
    assertXPath(pXmlDoc, sThemeColorsPath + "[@loext:name='My Colors']");

    const OString sThemeColorPath = sThemeColorsPath + "/loext:color";
    assertXPath(pXmlDoc, sThemeColorPath, 12);
    assertXPath(pXmlDoc, sThemeColorPath + "[3]", "name", u"dark2");
    assertXPath(pXmlDoc, sThemeColorPath + "[3]", "color", u"#002200");
    assertXPath(pXmlDoc, sThemeColorPath + "[9]", "name", u"accent5");
    assertXPath(pXmlDoc, sThemeColorPath + "[9]", "color", u"#880088");
    assertXPath(pXmlDoc, sThemeColorPath + "[12]", "name", u"followed-hyperlink");
    assertXPath(pXmlDoc, sThemeColorPath + "[12]", "color", u"#b0b0b0");
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTdf157018_ThemeImportDraw)
{
    // Similar to testThemeImport but for Draw.
    // Load document with custom color theme
    loadFromFile(u"tdf157018_CustomTheme.fodg");

    // First make sure the doc model has a master page with a theme:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterpage(xDrawPage->getMasterPage(), uno::UNO_QUERY);

    uno::Reference<util::XTheme> xTheme;
    xMasterpage->getPropertyValue(u"Theme"_ustr) >>= xTheme;
    CPPUNIT_ASSERT(xTheme.is());

    // Then make sure it is the custom color theme
    auto* pUnoTheme = dynamic_cast<UnoTheme*>(xTheme.get());
    CPPUNIT_ASSERT(pUnoTheme);
    auto pTheme = pUnoTheme->getTheme();
    CPPUNIT_ASSERT(pTheme);

    CPPUNIT_ASSERT_EQUAL(u"Custom"_ustr, pTheme->GetName());
    auto pColorSet = pTheme->getColorSet();
    CPPUNIT_ASSERT(pColorSet);
    CPPUNIT_ASSERT_EQUAL(u"My Colors"_ustr, pColorSet->getName());

    // and test some colors
    CPPUNIT_ASSERT_EQUAL(Color(0xFFFF11), pColorSet->getColor(model::ThemeColorType::Light1));
    CPPUNIT_ASSERT_EQUAL(Color(0x0A0A0A), pColorSet->getColor(model::ThemeColorType::Hyperlink));
    CPPUNIT_ASSERT_EQUAL(Color(0x440000), pColorSet->getColor(model::ThemeColorType::Accent1));
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, test_scene3d_ooxml_light)
{
    // The document has a shape in 3D mode. The import of ooxml light rigs can produce light
    // levels outside the 0..100 range allowed in ODF. Such high levels are needed for rendering
    // similar to MS Office.
    loadFromFile(u"Scene3d_LightRig_threePt.pptx");

    // Without fix this would have failed with validation error.
    save(u"impress8"_ustr);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTdf161327_LatheEndAngle)
{
    // Load document with 3D-Scene with 4 rotation objects
    loadFromFile(u"tdf161327_LatheEndAngle.fodg");

    // get scene object
    uno::Reference<drawing::XShape> xSceneShape(getShape(0));
    static constexpr OUString sExpected(u"com.sun.star.drawing.Shape3DSceneObject"_ustr);
    CPPUNIT_ASSERT_EQUAL(sExpected, xSceneShape->getShapeType());

    // Examine child objects
    // [0] dr3d:end-angle="1512"
    // [1] dr3d:end-angle="151.2deg"
    // [2] dr3d:end-angle="168.0grad"
    // [3] dr3d:end-angle="2.638937829015430rad"
    // All cases should result in D3DEndAngle = 1512. Without fix, cases [1], [2] and [3]
    // could not be read and default 3600 was used, although the values are valid in ODF.
    for (size_t i = 0; i < 4; ++i)
    {
        uno::Reference<container::XIndexAccess> xGroup(xSceneShape, uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xShape(xGroup->getByIndex(i), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        sal_Int16 nEndAngle;
        xShapeProps->getPropertyValue(u"D3DEndAngle"_ustr) >>= nEndAngle;
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1512), nEndAngle);
    }
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTdf161327_HatchAngle)
{
    // Load document with four rectangles with linear hatch background fill
    loadFromFile(u"tdf161327_HatchAngle.fodg");

    // The hatch angle is given in file as
    // [0] 585 unit less
    // [1] 58.5deg,
    // [2] 65grad,
    // [3] 1.01201761241668rad
    // The resulting angle should be 585 (meaning 1/10 of a degree) in all cases.
    // Cases [1], [2] and [3] had angle 0 without fix.
    for (size_t i = 0; i < 4; ++i)
    {
        uno::Reference<drawing::XShape> xShape(getShape(i));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        constexpr css::drawing::FillStyle eExpectedStyle = css::drawing::FillStyle_HATCH;
        css::drawing::FillStyle aActualStyle;
        xShapeProps->getPropertyValue(u"FillStyle"_ustr) >>= aActualStyle;
        CPPUNIT_ASSERT_EQUAL(eExpectedStyle, aActualStyle);
        sal_Int32 nExpectedAngle = 585; // FillHatch.Angle has data type 'long'
        css::drawing::Hatch aActualHatch;
        xShapeProps->getPropertyValue(u"FillHatch"_ustr) >>= aActualHatch;
        CPPUNIT_ASSERT_EQUAL(nExpectedAngle, aActualHatch.Angle);
    }
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTdf161483_ShadowSlant)
{
    // Load document with four 3D-scenes, that differ in the draw:shadow-slant value
    loadFromFile(u"tdf161483_ShadowSlant.fodg");

    // The shadow-slant angle is given in file as
    // [0] 36 unitless
    // [1] 36deg,
    // [2] 40grad,
    // [3] 1.628318530717959rad
    // The resulting angle should be 36 in all cases.
    // Cases [1], [2] and [3] had angle 0 without fix.

    constexpr sal_Int16 nExpectedAngle = 36; // D3DSceneShadowSlant has data type 'short'
    for (size_t i = 0; i < 4; ++i)
    {
        uno::Reference<drawing::XShape> xShape(getShape(i));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        sal_Int16 nActualAngle;
        xShapeProps->getPropertyValue(u"D3DSceneShadowSlant"_ustr) >>= nActualAngle;
        CPPUNIT_ASSERT_EQUAL(nExpectedAngle, nActualAngle);
    }
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTdf161483_CircleStartEndAngle)
{
    // Load document with four 'Arc' shapes, which differ in the type of start and end angles
    loadFromFile(u"tdf161483_CircleStartEndAngle.fodg");

    // The start and end angles are given in file as
    // [0] unitless: start 337.5   end 306
    // [1] deg: start 337.5deg   end 306deg
    // [2] grad: start 375grad   end 340grad
    // [3] rad: start 5.89048622548086rad   end 5.34070751110265rad
    // The resulting angle should be 33750 and 30600 in all cases.

    // CircleStartAngle and CircleEndAngle have data type 'long', meaning Degree100
    constexpr sal_Int32 nExpectedStartAngle = 33750;
    constexpr sal_Int32 nExpectedEndAngle = 30600;
    for (size_t i = 0; i < 4; ++i)
    {
        uno::Reference<drawing::XShape> xShape(getShape(i));
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        sal_Int32 nActualStartAngle;
        xShapeProps->getPropertyValue(u"CircleStartAngle"_ustr) >>= nActualStartAngle;
        CPPUNIT_ASSERT_EQUAL(nExpectedStartAngle, nActualStartAngle);
        sal_Int32 nActualEndAngle;
        xShapeProps->getPropertyValue(u"CircleEndAngle"_ustr) >>= nActualEndAngle;
        CPPUNIT_ASSERT_EQUAL(nExpectedEndAngle, nActualEndAngle);
    }
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testPdfExportAsOdg)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    // We need to enable PDFium import (and make sure to disable after the test)
    bool bResetEnvVar = false;
    if (getenv("LO_IMPORT_USE_PDFIUM") == nullptr)
    {
        bResetEnvVar = true;
        osl_setEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData, OUString("1").pData);
    }
    comphelper::ScopeGuard aPDFiumEnvVarGuard([&]() {
        if (bResetEnvVar)
            osl_clearEnvironment(OUString("LO_IMPORT_USE_PDFIUM").pData);
    });

    loadFromFile(u"two-pages.pdf");
    // save and reload as odg
    saveAndReload("draw8");

    // Check that the graphic in the second page of the document is the second page of the pdf
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xImage(xDrawPage->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xImage.is());
    uno::Reference<beans::XPropertySet> xShapeProps(xImage, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic;
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue("Graphic") >>= xGraphic);

    Graphic aGraphic(xGraphic);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : -1
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aGraphic.getPageNumber());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
