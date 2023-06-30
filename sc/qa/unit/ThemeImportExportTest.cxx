/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <scitems.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/borderline.hxx>

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

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testThemeExport)
{
    loadFromURL(u"xlsx/CalcThemeTest.xlsx");

    save("Calc Office Open XML");

    {
        xmlDocUniquePtr pXmlDoc = parseExport("xl/theme/theme1.xml");
        OString aClrScheme = "/a:theme/a:themeElements/a:clrScheme";
        assertXPath(pXmlDoc, aClrScheme, "name", "Office");
        assertXPath(pXmlDoc, aClrScheme + "/a:dk1/a:srgbClr", "val", "000000");
        assertXPath(pXmlDoc, aClrScheme + "/a:lt1/a:srgbClr", "val", "ffffff");
        assertXPath(pXmlDoc, aClrScheme + "/a:dk2/a:srgbClr", "val", "44546a");
        assertXPath(pXmlDoc, aClrScheme + "/a:lt2/a:srgbClr", "val", "e7e6e6");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent1/a:srgbClr", "val", "4472c4");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent2/a:srgbClr", "val", "ed7d31");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent3/a:srgbClr", "val", "a5a5a5");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent4/a:srgbClr", "val", "ffc000");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent5/a:srgbClr", "val", "5b9bd5");
        assertXPath(pXmlDoc, aClrScheme + "/a:accent6/a:srgbClr", "val", "70ad47");
        assertXPath(pXmlDoc, aClrScheme + "/a:hlink/a:srgbClr", "val", "0563c1");
        assertXPath(pXmlDoc, aClrScheme + "/a:folHlink/a:srgbClr", "val", "954f72");
    }

    {
        xmlDocUniquePtr pXmlDoc = parseExport("xl/styles.xml");

        assertXPath(pXmlDoc, "/x:styleSheet", 1);

        // Fonts
        OString aFont = "/x:styleSheet/x:fonts/x:font";
        assertXPath(pXmlDoc, aFont, 6);
        assertXPath(pXmlDoc, aFont + "[5]/x:color", "theme", "7");
        assertXPath(pXmlDoc, aFont + "[6]/x:color", "rgb", "FF9C5700");

        // Fills
        OString aFill = "/x:styleSheet/x:fills/x:fill";
        assertXPath(pXmlDoc, aFill, 4);
        assertXPath(pXmlDoc, aFill + "[1]/x:patternFill", "patternType", "none");
        assertXPath(pXmlDoc, aFill + "[2]/x:patternFill", "patternType", "gray125");
        assertXPath(pXmlDoc, aFill + "[3]/x:patternFill", "patternType", "solid");
        assertXPath(pXmlDoc, aFill + "[3]/x:patternFill/x:fgColor", "rgb", "FFFFEB9C");
        assertXPath(pXmlDoc, aFill + "[4]/x:patternFill", "patternType", "solid");
        assertXPath(pXmlDoc, aFill + "[4]/x:patternFill/x:fgColor", "theme", "4");
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
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getSchemeType());
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
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getSchemeType());
        auto& rTransformations = aComplexColor.getTransformations();
        CPPUNIT_ASSERT_EQUAL(size_t(2), rTransformations.size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
        CPPUNIT_ASSERT_EQUAL(20, roundToPercent(rTransformations[0].mnValue));
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
        CPPUNIT_ASSERT_EQUAL(80, roundToPercent(rTransformations[1].mnValue));
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
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getSchemeType());
        auto& rTransformations = aComplexColor.getTransformations();
        CPPUNIT_ASSERT_EQUAL(size_t(1), rTransformations.size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
        CPPUNIT_ASSERT_EQUAL(50, roundToPercent(rTransformations[0].mnValue));
    }
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellBackgroundThemeColor)
{
    loadFromURL(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
    checkCellBackgroundThemeColor(getScDoc());
    saveAndReload("Calc Office Open XML");
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
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6, aComplexColor.getSchemeType());
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
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6, aComplexColor.getSchemeType());
        auto& rTransformations = aComplexColor.getTransformations();
        CPPUNIT_ASSERT_EQUAL(size_t(2), rTransformations.size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
        CPPUNIT_ASSERT_EQUAL(60, roundToPercent(rTransformations[0].mnValue));
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
        CPPUNIT_ASSERT_EQUAL(40, roundToPercent(rTransformations[1].mnValue));
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
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6, aComplexColor.getSchemeType());
        auto& rTransformations = aComplexColor.getTransformations();
        CPPUNIT_ASSERT_EQUAL(size_t(1), rTransformations.size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
        CPPUNIT_ASSERT_EQUAL(50, roundToPercent(rTransformations[0].mnValue));
    }
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellTextThemeColor)
{
    loadFromURL(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
    checkCellTextThemeColor(getScDoc());
    saveAndReload("Calc Office Open XML");
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
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getSchemeType());
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
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getSchemeType());
            auto& rTransformations = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(2), rTransformations.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
            CPPUNIT_ASSERT_EQUAL(20, roundToPercent(rTransformations[0].mnValue));
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
            CPPUNIT_ASSERT_EQUAL(80, roundToPercent(rTransformations[1].mnValue));
        }
        {
            auto* pTop = pBoxItem->GetRight();
            CPPUNIT_ASSERT(pTop);
            CPPUNIT_ASSERT_EQUAL(Color(0xd4ebdb), pTop->GetColor());
            auto aComplexColor = pTop->getComplexColor();
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getSchemeType());
            auto& rTransformations = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(2), rTransformations.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
            CPPUNIT_ASSERT_EQUAL(20, roundToPercent(rTransformations[0].mnValue));
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
            CPPUNIT_ASSERT_EQUAL(80, roundToPercent(rTransformations[1].mnValue));
        }
        {
            auto* pTop = pBoxItem->GetBottom();
            CPPUNIT_ASSERT(pTop);
            CPPUNIT_ASSERT_EQUAL(Color(0x1f442a), pTop->GetColor());
            auto aComplexColor = pTop->getComplexColor();
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getSchemeType());
            auto& rTransformations = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(1), rTransformations.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
            CPPUNIT_ASSERT_EQUAL(50, roundToPercent(rTransformations[0].mnValue));
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
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getSchemeType());
            auto& rTransformations = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(1), rTransformations.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
            CPPUNIT_ASSERT_EQUAL(50, roundToPercent(rTransformations[0].mnValue));
        }

        {
            auto* pBottom = pBoxItem->GetBottom();
            CPPUNIT_ASSERT(pBottom);
            CPPUNIT_ASSERT_EQUAL(Color(0x1f442a), pBottom->GetColor());
            auto aComplexColor = pBottom->getComplexColor();
            CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5, aComplexColor.getSchemeType());
            auto& rTransformations = aComplexColor.getTransformations();
            CPPUNIT_ASSERT_EQUAL(size_t(1), rTransformations.size());
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
            CPPUNIT_ASSERT_EQUAL(50, roundToPercent(rTransformations[0].mnValue));
        }
    }
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellBorderThemeColor)
{
    loadFromURL(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
    checkCellBorderThemeColor(getScDoc());
    saveAndReload("Calc Office Open XML");
    checkCellBorderThemeColor(getScDoc());
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
