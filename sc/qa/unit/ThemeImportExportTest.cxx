/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <scitems.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/borderline.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XTheme.hpp>
#include <docmodel/uno/UnoTheme.hxx>
#include <docmodel/theme/Theme.hxx>

using namespace css;

namespace
{
class ThemeImportExportTest : public ScModelTestBase
{
public:
    ThemeImportExportTest()
        : ScModelTestBase("sc/qa/unit/data")
    {
    }
};

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testThemeExportAndImport)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    {
        uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY_THROW);

        auto pTheme = std::make_shared<model::Theme>("MyTheme");
        auto pColorSet = std::make_shared<model::ColorSet>("MyColorSet");
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

        xPropertySet->setPropertyValue("Theme", uno::Any(model::theme::createXTheme(pTheme)));
    }

    // Check the "Theme" property
    {
        uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<util::XTheme> xTheme(xPropertySet->getPropertyValue("Theme"),
                                            uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xTheme.is());
        auto* pUnoTheme = dynamic_cast<UnoTheme*>(xTheme.get());
        CPPUNIT_ASSERT(pUnoTheme);
        auto pTheme = pUnoTheme->getTheme();

        CPPUNIT_ASSERT_EQUAL(OUString("MyTheme"), pTheme->GetName());
        CPPUNIT_ASSERT_EQUAL(OUString("MyColorSet"), pTheme->getColorSet()->getName());
        CPPUNIT_ASSERT_EQUAL(OUString("Office"), pTheme->getFontScheme().getName());
        CPPUNIT_ASSERT_EQUAL(OUString(""), pTheme->getFormatScheme().getName());
    }

    saveAndReload("calc8");

    {
        xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
        static constexpr OString sThemePath = "//office:styles/loext:theme"_ostr;
        assertXPath(pXmlDoc, sThemePath, 1);
        assertXPath(pXmlDoc, sThemePath + "[@loext:name='MyTheme']");
        const OString sThemeColorsPath = sThemePath + "/loext:theme-colors";
        assertXPath(pXmlDoc, sThemeColorsPath, 1);
        assertXPath(pXmlDoc, sThemeColorsPath + "[@loext:name='MyColorSet']");
        const OString sThemeColorPath = sThemeColorsPath + "/loext:color";
        assertXPath(pXmlDoc, sThemeColorPath, 12);
        assertXPath(pXmlDoc, sThemeColorPath + "[3]", "name"_ostr, "dark2");
        assertXPath(pXmlDoc, sThemeColorPath + "[3]", "color"_ostr, "#333333");
        assertXPath(pXmlDoc, sThemeColorPath + "[9]", "name"_ostr, "accent5");
        assertXPath(pXmlDoc, sThemeColorPath + "[9]", "color"_ostr, "#999999");
        assertXPath(pXmlDoc, sThemeColorPath + "[12]", "name"_ostr, "followed-hyperlink");
        assertXPath(pXmlDoc, sThemeColorPath + "[12]", "color"_ostr, "#cccccc");
    }

    // Check the theme after import/export cycle
    {
        uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<util::XTheme> xTheme(xPropertySet->getPropertyValue("Theme"),
                                            uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xTheme.is());
        auto* pUnoTheme = dynamic_cast<UnoTheme*>(xTheme.get());
        CPPUNIT_ASSERT(pUnoTheme);
        auto pTheme = pUnoTheme->getTheme();

        CPPUNIT_ASSERT_EQUAL(OUString("MyTheme"), pTheme->GetName());
        CPPUNIT_ASSERT_EQUAL(OUString("MyColorSet"), pTheme->getColorSet()->getName());
        CPPUNIT_ASSERT_EQUAL(OUString("Office"), pTheme->getFontScheme().getName());
        CPPUNIT_ASSERT_EQUAL(OUString(""), pTheme->getFormatScheme().getName());
    }
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testThemeExportOOXML)
{
    loadFromFile(u"xlsx/CalcThemeTest.xlsx");

    save("Calc Office Open XML");

    {
        xmlDocUniquePtr pXmlDoc = parseExport("xl/theme/theme1.xml");
        OString aClrScheme = "/a:theme/a:themeElements/a:clrScheme"_ostr;
        assertXPath(pXmlDoc, aClrScheme, "name"_ostr, "Office");
        assertXPath(pXmlDoc, aClrScheme + "/a:dk1/a:srgbClr", "val"_ostr, "000000");
        assertXPath(pXmlDoc, aClrScheme + "/a:lt1/a:srgbClr", "val"_ostr, "ffffff");
        assertXPath(pXmlDoc, aClrScheme + "/a:dk2/a:srgbClr", "val"_ostr, "44546a");
        assertXPath(pXmlDoc, aClrScheme + "/a:lt2/a:srgbClr", "val"_ostr, "e7e6e6");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent1/a:srgbClr", "val"_ostr, "4472c4");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent2/a:srgbClr", "val"_ostr, "ed7d31");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent3/a:srgbClr", "val"_ostr, "a5a5a5");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent4/a:srgbClr", "val"_ostr, "ffc000");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent5/a:srgbClr", "val"_ostr, "5b9bd5");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent6/a:srgbClr", "val"_ostr, "70ad47");
        assertXPath(pXmlDoc, aClrScheme + "/a:hlink/a:srgbClr", "val"_ostr, "0563c1");
        assertXPath(pXmlDoc, aClrScheme + "/a:folHlink/a:srgbClr", "val"_ostr, "954f72");
    }

    {
        xmlDocUniquePtr pXmlDoc = parseExport("xl/styles.xml");

        assertXPath(pXmlDoc, "/x:styleSheet"_ostr, 1);

        // Fonts
        OString aFont = "/x:styleSheet/x:fonts/x:font"_ostr;
        assertXPath(pXmlDoc, aFont, 6);
        assertXPath(pXmlDoc, aFont + "[5]/x:color", "theme"_ostr, "7");
        assertXPath(pXmlDoc, aFont + "[6]/x:color", "rgb"_ostr, "FF9C5700");

        // Fills
        OString aFill = "/x:styleSheet/x:fills/x:fill"_ostr;
        assertXPath(pXmlDoc, aFill, 4);
        assertXPath(pXmlDoc, aFill + "[1]/x:patternFill", "patternType"_ostr, "none");
        assertXPath(pXmlDoc, aFill + "[2]/x:patternFill", "patternType"_ostr, "gray125");
        assertXPath(pXmlDoc, aFill + "[3]/x:patternFill", "patternType"_ostr, "solid");
        assertXPath(pXmlDoc, aFill + "[3]/x:patternFill/x:fgColor", "rgb"_ostr, "FFFFEB9C");
        assertXPath(pXmlDoc, aFill + "[4]/x:patternFill", "patternType"_ostr, "solid");
        assertXPath(pXmlDoc, aFill + "[4]/x:patternFill/x:fgColor", "theme"_ostr, "4");
    }
}

// Round 100th percent to percent value - so that small differences don't fail the test
sal_Int32 roundToPercent(sal_Int16 n100Percent)
{
    return sal_Int32(std::round(double(n100Percent) / 100.0));
}

void checkCellBackgroundThemeColor(ScDocument* pDoc)
{
    // A1
    {
        const ScPatternAttr* pPattern = pDoc->GetPattern(0, 0, 0);
        const SfxPoolItem* pItem = nullptr;
        pPattern->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
        CPPUNIT_ASSERT(pItem);

        auto* pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(Color(0x27ced7), pBrushItem->GetColor());
        auto aComplexColor = pBrushItem->getComplexColor();
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT_EQUAL(size_t(0), aComplexColor.getTransformations().size());
    }

    // A2
    {
        const ScPatternAttr* pPattern = pDoc->GetPattern(0, 1, 0);
        const SfxPoolItem* pItem = nullptr;
        pPattern->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
        CPPUNIT_ASSERT(pItem);

        auto* pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(Color(0xd4f5f7), pBrushItem->GetColor());
        auto aComplexColor = pBrushItem->getComplexColor();
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getThemeColorType());
        auto& rTransformations = aComplexColor.getTransformations();
        CPPUNIT_ASSERT_EQUAL(size_t(2), rTransformations.size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(20), roundToPercent(rTransformations[0].mnValue));
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(80), roundToPercent(rTransformations[1].mnValue));
    }

    // A3
    {
        const ScPatternAttr* pPattern = pDoc->GetPattern(0, 2, 0);
        const SfxPoolItem* pItem = nullptr;
        pPattern->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
        CPPUNIT_ASSERT(pItem);

        auto* pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(Color(0x14676b), pBrushItem->GetColor());
        auto aComplexColor = pBrushItem->getComplexColor();
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getThemeColorType());
        auto& rTransformations = aComplexColor.getTransformations();
        CPPUNIT_ASSERT_EQUAL(size_t(1), rTransformations.size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(50), roundToPercent(rTransformations[0].mnValue));
    }
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellBackgroundThemeColorOOXML)
{
    loadFromFile(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
    checkCellBackgroundThemeColor(getScDoc());
    saveAndReload("Calc Office Open XML");
    checkCellBackgroundThemeColor(getScDoc());
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellBackgroundThemeColorODF)
{
    // Open the OOXML source
    loadFromFile(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
    // Save as ODF and load again - checks import / export cycle
    saveAndReload("calc8");
    // Check the values and show that the document is unchanged and all the data preserved
    checkCellBackgroundThemeColor(getScDoc());
}

void checkCellTextThemeColor(ScDocument* pDoc)
{
    // B1
    {
        const ScPatternAttr* pPattern = pDoc->GetPattern(1, 0, 0);
        const SfxPoolItem* pItem = nullptr;
        pPattern->GetItemSet().HasItem(ATTR_FONT_COLOR, &pItem);
        CPPUNIT_ASSERT(pItem);

        auto* pColorItem = static_cast<const SvxColorItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(Color(0x62a39f), pColorItem->getColor());
        auto aComplexColor = pColorItem->getComplexColor();
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6, aComplexColor.getThemeColorType());
        CPPUNIT_ASSERT_EQUAL(size_t(0), aComplexColor.getTransformations().size());
    }

    // B2
    {
        const ScPatternAttr* pPattern = pDoc->GetPattern(1, 1, 0);
        const SfxPoolItem* pItem = nullptr;
        pPattern->GetItemSet().HasItem(ATTR_FONT_COLOR, &pItem);
        CPPUNIT_ASSERT(pItem);

        auto* pColorItem = static_cast<const SvxColorItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(Color(0xa1c8c5), pColorItem->getColor());
        auto aComplexColor = pColorItem->getComplexColor();
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6, aComplexColor.getThemeColorType());
        auto& rTransformations = aComplexColor.getTransformations();
        CPPUNIT_ASSERT_EQUAL(size_t(2), rTransformations.size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(60), roundToPercent(rTransformations[0].mnValue));
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(40), roundToPercent(rTransformations[1].mnValue));
    }

    // B3
    {
        const ScPatternAttr* pPattern = pDoc->GetPattern(1, 2, 0);
        const SfxPoolItem* pItem = nullptr;
        pPattern->GetItemSet().HasItem(ATTR_FONT_COLOR, &pItem);
        CPPUNIT_ASSERT(pItem);

        auto* pColorItem = static_cast<const SvxColorItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(Color(0x305250), pColorItem->getColor());
        auto aComplexColor = pColorItem->getComplexColor();
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6, aComplexColor.getThemeColorType());
        auto& rTransformations = aComplexColor.getTransformations();
        CPPUNIT_ASSERT_EQUAL(size_t(1), rTransformations.size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(50), roundToPercent(rTransformations[0].mnValue));
    }
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellTextThemeColor)
{
    loadFromFile(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
    checkCellTextThemeColor(getScDoc());
    saveAndReload("Calc Office Open XML");
    checkCellTextThemeColor(getScDoc());
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellTextThemeColorODF)
{
    // Open the OOXML source
    loadFromFile(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
    // Save as ODF and load again - checks import / export cycle
    saveAndReload("calc8");
    // Check the values and show that the document is unchanged and all the data preserved
    checkCellTextThemeColor(getScDoc());
}

void checkCellBorderThemeColor(ScDocument* pDoc)
{
    // C1
    {
        const ScPatternAttr* pPattern = pDoc->GetPattern(2, 0, 0);
        const SfxPoolItem* pItem = nullptr;
        pPattern->GetItemSet().HasItem(ATTR_BORDER, &pItem);
        CPPUNIT_ASSERT(pItem);

        auto* pBoxItem = static_cast<const SvxBoxItem*>(pItem);
        CPPUNIT_ASSERT(!pBoxItem->GetLeft());
        CPPUNIT_ASSERT(!pBoxItem->GetRight());
        CPPUNIT_ASSERT(!pBoxItem->GetBottom());

        {
            auto* pTop = pBoxItem->GetTop();
            CPPUNIT_ASSERT(pTop);
            CPPUNIT_ASSERT_EQUAL(Color(0x3e8853), pTop->GetColor());
            auto aComplexColor = pTop->getComplexColor();
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getThemeColorType());
            auto& rTransformations = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(0), rTransformations.size());
        }
    }

    // C2
    {
        const ScPatternAttr* pPattern = pDoc->GetPattern(2, 1, 0);
        const SfxPoolItem* pItem = nullptr;
        pPattern->GetItemSet().HasItem(ATTR_BORDER, &pItem);
        CPPUNIT_ASSERT(pItem);

        auto* pBoxItem = static_cast<const SvxBoxItem*>(pItem);
        CPPUNIT_ASSERT(!pBoxItem->GetTop());

        {
            auto* pTop = pBoxItem->GetLeft();
            CPPUNIT_ASSERT(pTop);
            CPPUNIT_ASSERT_EQUAL(Color(0xd4ebdb), pTop->GetColor());
            auto aComplexColor = pTop->getComplexColor();
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getThemeColorType());
            auto& rTransformations = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(2), rTransformations.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(20), roundToPercent(rTransformations[0].mnValue));
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(80), roundToPercent(rTransformations[1].mnValue));
        }
        {
            auto* pTop = pBoxItem->GetRight();
            CPPUNIT_ASSERT(pTop);
            CPPUNIT_ASSERT_EQUAL(Color(0xd4ebdb), pTop->GetColor());
            auto aComplexColor = pTop->getComplexColor();
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getThemeColorType());
            auto& rTransformations = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(2), rTransformations.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(20), roundToPercent(rTransformations[0].mnValue));
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(80), roundToPercent(rTransformations[1].mnValue));
        }
        {
            auto* pTop = pBoxItem->GetBottom();
            CPPUNIT_ASSERT(pTop);
            CPPUNIT_ASSERT_EQUAL(Color(0x1f442a), pTop->GetColor());
            auto aComplexColor = pTop->getComplexColor();
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getThemeColorType());
            auto& rTransformations = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(1), rTransformations.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(50), roundToPercent(rTransformations[0].mnValue));
        }
    }

    // C3
    {
        const ScPatternAttr* pPattern = pDoc->GetPattern(2, 2, 0);
        const SfxPoolItem* pItem = nullptr;
        pPattern->GetItemSet().HasItem(ATTR_BORDER, &pItem);
        CPPUNIT_ASSERT(pItem);

        auto* pBoxItem = static_cast<const SvxBoxItem*>(pItem);
        CPPUNIT_ASSERT(!pBoxItem->GetLeft());
        CPPUNIT_ASSERT(!pBoxItem->GetRight());

        {
            auto* pTop = pBoxItem->GetTop();
            CPPUNIT_ASSERT(pTop);
            CPPUNIT_ASSERT_EQUAL(Color(0x1f442a), pTop->GetColor());
            auto aComplexColor = pTop->getComplexColor();
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getThemeColorType());
            auto& rTransformations = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(1), rTransformations.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(50), roundToPercent(rTransformations[0].mnValue));
        }

        {
            auto* pBottom = pBoxItem->GetBottom();
            CPPUNIT_ASSERT(pBottom);
            CPPUNIT_ASSERT_EQUAL(Color(0x1f442a), pBottom->GetColor());
            auto aComplexColor = pBottom->getComplexColor();
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getThemeColorType());
            auto& rTransformations = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(1), rTransformations.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(50), roundToPercent(rTransformations[0].mnValue));
        }
    }
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellBorderThemeColor)
{
    loadFromFile(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
    checkCellBorderThemeColor(getScDoc());
    saveAndReload("Calc Office Open XML");
    checkCellBorderThemeColor(getScDoc());
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellBorderThemeColorODF)
{
    // Open the OOXML source
    loadFromFile(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
    // Save as ODF and load again - checks import / export cycle
    saveAndReload("calc8");
    // Check the values and show that the document is unchanged and all the data preserved
    checkCellBorderThemeColor(getScDoc());
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
