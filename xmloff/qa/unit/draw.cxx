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
#include <com/sun/star/util/Color.hpp>
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
#include <docmodel/uno/UnoThemeColor.hxx>
#include <docmodel/uno/UnoTheme.hxx>

using namespace ::com::sun::star;

/// Covers xmloff/source/draw/ fixes.
class XmloffDrawTest : public UnoApiXmlTest
{
public:
    XmloffDrawTest();
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
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
    : UnoApiXmlTest("/xmloff/qa/unit/data/")
{
}

void XmloffDrawTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    XmlTestTools::registerODFNamespaces(pXmlXpathCtx);
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
    loadFromURL(u"textbox-loss.docx");
    saveAndReload("impress8");

    // Make sure that the shape is still a textbox.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    bool bTextBox = false;
    xShape->getPropertyValue("TextBox") >>= bTextBox;

    // Without the accompanying fix in place, this test would have failed, as the shape only had
    // editeng text, losing the image part of the shape text.
    CPPUNIT_ASSERT(bTextBox);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTdf141301_Extrusion_Angle)
{
    // Load a document that has a custom shape with extrusion direction as set by LO as its default.
    loadFromURL(u"tdf141301_Extrusion_Skew.odg");

    // Prepare use of XPath
    save("draw8");
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    // Without fix draw:extrusion-skew="50 -135" was not written to file although "50 -135" is not
    // default in ODF, but only default inside LO.
    assertXPath(pXmlDoc, "//draw:enhanced-geometry", "extrusion-skew", "50 -135");
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeExport)
{
    // Create an Impress document which has a master page which has a theme associated with it.
    mxComponent = loadFromDesktop("private:factory/simpress");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPage(xDrawPage->getMasterPage(), uno::UNO_QUERY);

    model::Theme aTheme("mytheme");
    std::unique_ptr<model::ColorSet> pColorSet(new model::ColorSet("mycolorscheme"));
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
    aTheme.SetColorSet(std::move(pColorSet));

    uno::Reference<util::XTheme> xTheme = model::theme::createXTheme(aTheme);
    xMasterPage->setPropertyValue("Theme", uno::Any(xTheme));

    // Export to ODP:
    save("impress8");

    // Check if the 12 colors are written in the XML:
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 12
    // - Actual  : 0
    // - XPath '//style:master-page/loext:theme/loext:color-table/loext:color' number of nodes is incorrect
    // i.e. the theme was lost on exporting to ODF.
    assertXPath(pXmlDoc, "//style:master-page/loext:theme/loext:color-table/loext:color", 12);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testVideoSnapshot)
{
    // Execute ODP import:
    loadFromURL(u"video-snapshot.odp");
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
    save("impress8");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Check that the preview was exported:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//draw:frame[@draw:style-name='gr1']/draw:image' number of nodes is incorrect
    // i.e. the preview wasn't exported to ODP.
    assertXPath(pXmlDoc, "//draw:frame[@draw:style-name='gr1']/draw:image", "href",
                "Pictures/MediaPreview1.png");
    // Check that the crop was exported:
    assertXPath(pXmlDoc, "//style:style[@style:name='gr1']/style:graphic-properties", "clip",
                "rect(0cm, 1.356cm, 0cm, 1.356cm)");
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeImport)
{
    // Given a document that has a master page with a theme associated:
    loadFromURL(u"theme.odp");

    // Then make sure the doc model has a master page with a theme:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterpage(xDrawPage->getMasterPage(), uno::UNO_QUERY);

    uno::Reference<util::XTheme> xTheme;
    xMasterpage->getPropertyValue("Theme") >>= xTheme;

    // We expect the theme to be set on the master page
    CPPUNIT_ASSERT(xTheme.is());
    auto* pUnoTheme = dynamic_cast<UnoTheme*>(xTheme.get());
    CPPUNIT_ASSERT(pUnoTheme);
    auto const& rTheme = pUnoTheme->getTheme();

    CPPUNIT_ASSERT_EQUAL(OUString("Office Theme"), rTheme.GetName());
    CPPUNIT_ASSERT_EQUAL(OUString("Office"), rTheme.GetColorSet()->getName());

    CPPUNIT_ASSERT_EQUAL(Color(0x954F72),
                         rTheme.GetColorSet()->getColor(model::ThemeColorType::FollowedHyperlink));
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeColorExportImport)
{
    // Given a document that refers to a theme color:
    loadFromURL(u"Reference-ThemeColors-TextAndFill.pptx");
    save("impress8");

    // Make sure the export result has the theme reference:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    // Text color
    OString aStyle1(
        "//style:style[@style:name='T2']/style:text-properties/loext:char-color-theme-reference");
    assertXPath(pXmlDoc, aStyle1, "type", "accent3");
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[1]", "type", "lummod");
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[1]", "value", "2000");
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[2]", "type", "lumoff");
    assertXPath(pXmlDoc, aStyle1 + "/loext:transformation[2]", "value", "8000");

    OString aStyle2(
        "//style:style[@style:name='T3']/style:text-properties/loext:char-color-theme-reference");
    assertXPath(pXmlDoc, aStyle2, "type", "accent3");
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[1]", "type", "lummod");
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[1]", "value", "6000");
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[2]", "type", "lumoff");
    assertXPath(pXmlDoc, aStyle2 + "/loext:transformation[2]", "value", "4000");

    OString aStyle3(
        "//style:style[@style:name='T4']/style:text-properties/loext:char-color-theme-reference");
    assertXPath(pXmlDoc, aStyle3, "type", "accent3");
    assertXPath(pXmlDoc, aStyle3 + "/loext:transformation[1]", "type", "lummod");
    assertXPath(pXmlDoc, aStyle3 + "/loext:transformation[1]", "value", "5000");

    // Shapes fill color
    OString aShape1("//style:style[@style:name='gr1']/style:graphic-properties/"
                    "loext:fill-color-theme-reference");
    assertXPath(pXmlDoc, aShape1, "type", "accent2");
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[1]", "type", "lummod");
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[1]", "value", "2000");
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[2]", "type", "lumoff");
    assertXPath(pXmlDoc, aShape1 + "/loext:transformation[2]", "value", "8000");

    OString aShape2("//style:style[@style:name='gr2']/style:graphic-properties/"
                    "loext:fill-color-theme-reference");
    assertXPath(pXmlDoc, aShape2, "type", "accent2");
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[1]", "type", "lummod");
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[1]", "value", "6000");
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[2]", "type", "lumoff");
    assertXPath(pXmlDoc, aShape2 + "/loext:transformation[2]", "value", "4000");

    OString aShape3("//style:style[@style:name='gr3']/style:graphic-properties/"
                    "loext:fill-color-theme-reference");
    assertXPath(pXmlDoc, aShape3, "type", "accent2");
    assertXPath(pXmlDoc, aShape3 + "/loext:transformation[1]", "type", "lummod");
    assertXPath(pXmlDoc, aShape3 + "/loext:transformation[1]", "value", "5000");

    // reload
    load(maTempFile.GetURL());

    // check fill color theme
    {
        uno::Reference<drawing::XShape> xShape(getShape(0));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        uno::Reference<util::XThemeColor> xThemeColor;
        xShapeProperties->getPropertyValue("FillColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), rTrans1.mnValue);
        auto const& rTrans2 = aThemeColor.getTransformations()[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(8000), rTrans2.mnValue);
    }
    {
        uno::Reference<drawing::XShape> xShape(getShape(1));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        uno::Reference<util::XThemeColor> xThemeColor;
        xShapeProperties->getPropertyValue("FillColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), rTrans1.mnValue);
        auto const& rTrans2 = aThemeColor.getTransformations()[1];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTrans2.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(4000), rTrans2.mnValue);
    }
    {
        uno::Reference<drawing::XShape> xShape(getShape(2));
        CPPUNIT_ASSERT(xShape.is());
        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        uno::Reference<util::XThemeColor> xThemeColor;
        xShapeProperties->getPropertyValue("FillColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(1), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
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
        uno::Reference<util::XThemeColor> xThemeColor;
        xPortion->getPropertyValue("CharColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), rTrans1.mnValue);
        auto const& rTrans2 = aThemeColor.getTransformations()[1];
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
        uno::Reference<util::XThemeColor> xThemeColor;
        xPortion->getPropertyValue("CharColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(2), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), rTrans1.mnValue);
        auto const& rTrans2 = aThemeColor.getTransformations()[1];
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
        uno::Reference<util::XThemeColor> xThemeColor;
        xPortion->getPropertyValue("CharColorThemeReference") >>= xThemeColor;
        CPPUNIT_ASSERT(xThemeColor.is());
        model::ThemeColor aThemeColor;
        model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aThemeColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(1), aThemeColor.getTransformations().size());
        auto const& rTrans1 = aThemeColor.getTransformations()[0];
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrans1.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(5000), rTrans1.mnValue);
    }
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testThemeColor_ShapeFill)
{
    loadFromURL(u"ReferenceShapeFill.pptx");
    save("impress8");
    // reload
    load(maTempFile.GetURL());

    // check fill color theme
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    uno::Reference<util::XThemeColor> xThemeColor;
    xShapeProperties->getPropertyValue("FillColorThemeReference") >>= xThemeColor;
    CPPUNIT_ASSERT(xThemeColor.is());
    model::ThemeColor aThemeColor;
    model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6, aThemeColor.getType());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aThemeColor.getTransformations().size());
    CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod,
                         aThemeColor.getTransformations()[0].meType);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7500), aThemeColor.getTransformations()[0].mnValue);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testTableInShape)
{
    // Given a document with a shape with a "FrameX" parent style (starts with Frame, but is not
    // Frame):
    loadFromURL(u"table-in-shape.fodt");

    // Then make sure the table inside the shape is not lost:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xShape->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xEnum = xText->createEnumeration();
    uno::Reference<text::XTextTable> xTable(xEnum->nextElement(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have crashed, as xTable was an empty
    // reference, i.e. the table inside the shape was lost.
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("A1"), xCell->getString());
}

// Tests for save/load of new (LO 7.4) attribute loext:extrusion-metal-type
namespace
{
void lcl_assertMetalProperties(std::string_view sInfo, uno::Reference<drawing::XShape>& rxShape)
{
    uno::Reference<beans::XPropertySet> xShapeProps(rxShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue("CustomShapeGeometry") >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<beans::PropertyValue> aExtrusionSeq;
    aGeoPropMap.getValue("Extrusion") >>= aExtrusionSeq;
    comphelper::SequenceAsHashMap aExtrusionPropMap(aExtrusionSeq);

    bool bIsMetal(false);
    aExtrusionPropMap.getValue("Metal") >>= bIsMetal;
    OString sMsg = OString::Concat(sInfo) + " Metal";
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), bIsMetal);

    sal_Int16 nMetalType(-1);
    aExtrusionPropMap.getValue("MetalType") >>= nMetalType;
    sMsg = OString::Concat(sInfo) + " MetalType";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        sMsg.getStr(), css::drawing::EnhancedCustomShapeMetalType::MetalMSCompatible, nMetalType);
}
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionMetalTypeExtended)
{
    loadFromURL(u"tdf145700_3D_metal_type_MSCompatible.doc");
    // verify properties
    uno::Reference<drawing::XShape> xShape(getShape(0));
    lcl_assertMetalProperties("from doc", xShape);

    // Test, that new attribute is written with loext namespace. Adapt when attribute is added to ODF.
    save("writer8");

    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//draw:enhanced-geometry", "extrusion-metal", "true");
    assertXPath(pXmlDoc,
                "//draw:enhanced-geometry[@loext:extrusion-metal-type='loext:MetalMSCompatible']");

    // reload
    mxComponent = loadFromDesktop(maTempFile.GetURL(), "com.sun.star.text.TextDocument");
    // verify properties
    uno::Reference<drawing::XShape> xShapeReload(getShape(0));
    lcl_assertMetalProperties("from ODF 1.3 extended", xShapeReload);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionMetalTypeStrict)
{
    loadFromURL(u"tdf145700_3D_metal_type_MSCompatible.doc");

    // save ODF 1.3 strict and test, that new attribute is not written. Adapt when attribute is
    // added to ODF.
    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion(GetODFDefaultVersion());
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013);
    save("writer8");

    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//draw:enhanced-geometry", "extrusion-metal", "true");
    assertXPath(pXmlDoc, "//draw:enhanced-geometry[@loext:extrusion-metal-type]", 0);

    SetODFDefaultVersion(nCurrentODFVersion);
}

namespace
{
void lcl_assertSpecularityProperty(std::string_view sInfo, uno::Reference<drawing::XShape>& rxShape)
{
    uno::Reference<beans::XPropertySet> xShapeProps(rxShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue("CustomShapeGeometry") >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<beans::PropertyValue> aExtrusionSeq;
    aGeoPropMap.getValue("Extrusion") >>= aExtrusionSeq;
    comphelper::SequenceAsHashMap aExtrusionPropMap(aExtrusionSeq);

    double fSpecularity(-1.0);
    aExtrusionPropMap.getValue("Specularity") >>= fSpecularity;
    OString sMsg = OString::Concat(sInfo) + "Specularity";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sMsg.getStr(), 122.0703125, fSpecularity);
}
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionSpecularityExtended)
{
    loadFromURL(u"tdf147580_extrusion-specularity.doc");
    // verify property
    uno::Reference<drawing::XShape> xShape(getShape(0));
    lcl_assertSpecularityProperty("from doc", xShape);

    // Test, that attribute is written in draw namespace with value 100% and in loext namespace with
    // value 122.0703125%.
    save("writer8");

    // assert XML.
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//draw:enhanced-geometry[@draw:extrusion-specularity='100%']");
    assertXPath(pXmlDoc,
                "//draw:enhanced-geometry[@loext:extrusion-specularity-loext='122.0703125%']");

    // reload and verify, that the loext value is used
    mxComponent = loadFromDesktop(maTempFile.GetURL(), "com.sun.star.text.TextDocument");
    // verify properties
    uno::Reference<drawing::XShape> xShapeReload(getShape(0));
    lcl_assertSpecularityProperty("from ODF 1.3 extended", xShapeReload);
}

CPPUNIT_TEST_FIXTURE(XmloffDrawTest, testExtrusionSpecularity)
{
    loadFromURL(u"tdf147580_extrusion-specularity.doc");

    // The file has c3DSpecularAmt="80000" which results internally in specularity=122%.
    // Save to ODF 1.3 strict and make sure it does not produce a validation error.
    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion(GetODFDefaultVersion());
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_013);
    save("writer8");

    SetODFDefaultVersion(nCurrentODFVersion);
}

namespace
{
bool lcl_getShapeSegments(uno::Sequence<drawing::EnhancedCustomShapeSegment>& rSegments,
                          const uno::Reference<drawing::XShape>& xShape)
{
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY_THROW);
    uno::Any anotherAny = xShapeProps->getPropertyValue("CustomShapeGeometry");
    uno::Sequence<beans::PropertyValue> aCustomShapeGeometry;
    if (!(anotherAny >>= aCustomShapeGeometry))
        return false;
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : std::as_const(aCustomShapeGeometry))
    {
        if (rProp.Name == "Path")
        {
            rProp.Value >>= aPathProps;
            break;
        }
    }

    for (beans::PropertyValue const& rProp : std::as_const(aPathProps))
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
    loadFromURL(u"tdf148714_CurvedArrowsOld.odp");

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
    loadFromURL(u"tdf149551_verticalText.pptx");
    // The file has a shape with attribute vert="vert" in <bodyPr> element. That generates a
    // TextPreRotateAngle attribute in CustomShapeGeometry.

    // Add a TextRotateAngle attribute.
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeomSeq;
    xShapeProps->getPropertyValue("CustomShapeGeometry") >>= aGeomSeq;
    auto aGeomVec(comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(aGeomSeq));
    aGeomVec.push_back(comphelper::makePropertyValue("TextRotateAngle", sal_Int32(45)));
    aGeomSeq = comphelper::containerToSequence(aGeomVec);
    xShapeProps->setPropertyValue("CustomShapeGeometry", uno::Any(aGeomSeq));

    // Save to ODF. Without the fix, a file format error was produced, because attribute
    // draw:text-rotate-angle was written twice, one from TextPreRotateAngle and the other from
    // TextRotateAngle.
    // This should already catch the format error, but does not, see tdf#149567
    // But reload catches it.
    saveAndReload("writer8");
}
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
