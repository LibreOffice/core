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
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
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
    assertXPath(pXmlDoc, "//draw:enhanced-geometry"_ostr, "extrusion-skew"_ostr, u"50 -135"_ustr);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeExport)
{
    // Create an Impress document which has a master page which has a theme associated with it.
    mxComponent = loadFromDesktop(u"private:factory/simpress"_ustr);
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
    assertXPath(pXmlDoc, "//style:master-page/loext:theme/loext:theme-colors/loext:color"_ostr, 12);
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
    assertXPath(pXmlDoc, "//draw:frame[@draw:style-name='gr1']/draw:image"_ostr, "href"_ostr,
                u"Pictures/MediaPreview1.png"_ustr);
    // Check that the crop was exported:
    assertXPath(pXmlDoc, "//style:style[@style:name='gr1']/style:graphic-properties"_ostr,
                "clip"_ostr, u"rect(0cm, 1.356cm, 0cm, 1.356cm)"_ustr);
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

    load(maTempFile.GetURL());

    checkFillAndLineComplexColors(getShape(0));
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTextAndFillThemeColorExportImport)
{
    // Given a document that refers to a theme color:
    loadFromFile(u"Reference-ThemeColors-TextAndFill.pptx");
    save(u"impress8"_ustr);

    // Make sure the export result has the theme reference:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    // Text color
    OString aStyle1(
        "//style:style[@style:name='T2']/style:text-properties/loext:char-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type"_ostr, u"theme"_ustr);
    assertXPath(pXmlDoc, aStyle1, "theme-type"_ostr, u"accent3"_ustr);
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[1]", "type"_ostr, u"lummod"_ustr);
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[1]", "value"_ostr, u"2000"_ustr);
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[2]", "type"_ostr, u"lumoff"_ustr);
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[2]", "value"_ostr, u"8000"_ustr);

    OString aStyle2(
        "//style:style[@style:name='T3']/style:text-properties/loext:char-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type"_ostr, u"theme"_ustr);
    assertXPath(pXmlDoc, aStyle2, "theme-type"_ostr, u"accent3"_ustr);
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[1]", "type"_ostr, u"lummod"_ustr);
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[1]", "value"_ostr, u"6000"_ustr);
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[2]", "type"_ostr, u"lumoff"_ustr);
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[2]", "value"_ostr, u"4000"_ustr);

    OString aStyle3(
        "//style:style[@style:name='T4']/style:text-properties/loext:char-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type"_ostr, u"theme"_ustr);
    assertXPath(pXmlDoc, aStyle3, "theme-type"_ostr, u"accent3"_ustr);
    assertXPath(pXmlDoc, aStyle3 + "/loext:transformation[1]", "type"_ostr, u"lummod"_ustr);
    assertXPath(pXmlDoc, aStyle3 + "/loext:transformation[1]", "value"_ostr, u"5000"_ustr);

    // Shapes fill color
    OString aShape1("//style:style[@style:name='gr1']/style:graphic-properties/"
                    "loext:fill-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type"_ostr, u"theme"_ustr);
    assertXPath(pXmlDoc, aShape1, "theme-type"_ostr, u"accent2"_ustr);
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[1]", "type"_ostr, u"lummod"_ustr);
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[1]", "value"_ostr, u"2000"_ustr);
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[2]", "type"_ostr, u"lumoff"_ustr);
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[2]", "value"_ostr, u"8000"_ustr);

    OString aShape2("//style:style[@style:name='gr2']/style:graphic-properties/"
                    "loext:fill-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type"_ostr, u"theme"_ustr);
    assertXPath(pXmlDoc, aShape2, "theme-type"_ostr, u"accent2"_ustr);
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[1]", "type"_ostr, u"lummod"_ustr);
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[1]", "value"_ostr, u"6000"_ustr);
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[2]", "type"_ostr, u"lumoff"_ustr);
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[2]", "value"_ostr, u"4000"_ustr);

    OString aShape3("//style:style[@style:name='gr3']/style:graphic-properties/"
                    "loext:fill-complex-color"_ostr);
    assertXPath(pXmlDoc, aStyle1, "color-type"_ostr, u"theme"_ustr);
    assertXPath(pXmlDoc, aShape3, "theme-type"_ostr, u"accent2"_ustr);
    assertXPath(pXmlDoc, aShape3 + "/loext:transformation[1]", "type"_ostr, u"lummod"_ustr);
    assertXPath(pXmlDoc, aShape3 + "/loext:transformation[1]", "value"_ostr, u"5000"_ustr);

    // reload
    load(maTempFile.GetURL());

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
    save(u"impress8"_ustr);
    // reload
    load(maTempFile.GetURL());

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
namespace
{
void lcl_assertMetalProperties(std::string_view sInfo, uno::Reference<drawing::XShape>& rxShape)
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        sMsg.getStr(), css::drawing::EnhancedCustomShapeMetalType::MetalMSCompatible, nMetalType);
}
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionMetalTypeExtended)
{
    loadFromFile(u"tdf145700_3D_metal_type_MSCompatible.doc");
    // verify properties
    uno::Reference<drawing::XShape> xShape(getShape(0));
    lcl_assertMetalProperties("from doc", xShape);

    // Test, that new attribute is written with loext namespace. Adapt when attribute is added to ODF.
    save(u"writer8"_ustr);

    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry"_ostr, "extrusion-metal"_ostr, u"true"_ustr);
    assertXPath(
        pXmlDoc,
        "//draw:enhanced-geometry[@loext:extrusion-metal-type='loext:MetalMSCompatible']"_ostr);

    // reload
    mxComponent = loadFromDesktop(maTempFile.GetURL(), u"com.sun.star.text.TextDocument"_ustr);
    // verify properties
    uno::Reference<drawing::XShape> xShapeReload(getShape(0));
    lcl_assertMetalProperties("from ODF 1.3 extended", xShapeReload);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionMetalTypeStrict)
{
    loadFromFile(u"tdf145700_3D_metal_type_MSCompatible.doc");

    // save ODF 1.3 strict and test, that new attribute is not written. Adapt when attribute is
    // added to ODF.
    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion(GetODFDefaultVersion());
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013);
    save(u"writer8"_ustr);

    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry"_ostr, "extrusion-metal"_ostr, u"true"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry[@loext:extrusion-metal-type]"_ostr, 0);

    SetODFDefaultVersion(nCurrentODFVersion);
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
    loadFromFile(u"tdf147580_extrusion-specularity.doc");
    // verify property
    uno::Reference<drawing::XShape> xShape(getShape(0));
    lcl_assertSpecularityProperty("from doc", xShape);

    // Test, that attribute is written in draw namespace with value 100% and in loext namespace with
    // value 122.0703125%.
    save(u"writer8"_ustr);

    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//draw:enhanced-geometry[@draw:extrusion-specularity='100%']"_ostr);
    assertXPath(pXmlDoc,
                "//draw:enhanced-geometry[@loext:extrusion-specularity-loext='122.0703125%']"_ostr);

    // reload and verify, that the loext value is used
    mxComponent = loadFromDesktop(maTempFile.GetURL(), u"com.sun.star.text.TextDocument"_ustr);
    // verify properties
    uno::Reference<drawing::XShape> xShapeReload(getShape(0));
    lcl_assertSpecularityProperty("from ODF 1.3 extended", xShapeReload);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionSpecularity)
{
    loadFromFile(u"tdf147580_extrusion-specularity.doc");

    // The file has c3DSpecularAmt="80000" which results internally in specularity=122%.
    // Save to ODF 1.3 strict and make sure it does not produce a validation error.
    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion(GetODFDefaultVersion());
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013);
    save(u"writer8"_ustr);

    SetODFDefaultVersion(nCurrentODFVersion);
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

    mxComponent = loadFromDesktop(u"private:factory/sdraw"_ustr);
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
    assertXPath(pXmlDoc, sThemeColorPath + "[3]", "name"_ostr, u"dark2"_ustr);
    assertXPath(pXmlDoc, sThemeColorPath + "[3]", "color"_ostr, u"#002200"_ustr);
    assertXPath(pXmlDoc, sThemeColorPath + "[9]", "name"_ostr, u"accent5"_ustr);
    assertXPath(pXmlDoc, sThemeColorPath + "[9]", "color"_ostr, u"#880088"_ustr);
    assertXPath(pXmlDoc, sThemeColorPath + "[12]", "name"_ostr, u"followed-hyperlink"_ustr);
    assertXPath(pXmlDoc, sThemeColorPath + "[12]", "color"_ostr, u"#b0b0b0"_ustr);
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
