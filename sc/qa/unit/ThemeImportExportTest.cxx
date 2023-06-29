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

    xmlDocUniquePtr pXmlDoc = parseExport("xl/styles.xml");

    assertXPath(pXmlDoc, "/x:styleSheet", 1);

    // Fonts
    assertXPath(pXmlDoc, "/x:styleSheet/x:fonts/x:font", 6);

    assertXPath(pXmlDoc, "/x:styleSheet/x:fonts/x:font[5]/x:color", "rgb",
                "FFFFC000"); // need to be theme probably

    assertXPath(pXmlDoc, "/x:styleSheet/x:fonts/x:font[6]/x:color", "rgb",
                "FF9C5700"); // need to be theme probably

    // Fills
    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill", 4);

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[1]/x:patternFill", "patternType", "none");

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[2]/x:patternFill", "patternType", "gray125");

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[3]/x:patternFill", "patternType", "solid");
    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[3]/x:patternFill/x:fgColor", "rgb",
                "FFFFEB9C");

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[4]/x:patternFill", "patternType", "solid");
    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[4]/x:patternFill/x:fgColor", "theme", "4");
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
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), rTransformations[0].mnValue);
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(7999), rTransformations[1].mnValue);
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
        CPPUNIT_ASSERT_EQUAL(sal_Int16(5000), rTransformations[0].mnValue);
    }
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellBackgroundThemeColor)
{
    loadFromURL(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
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
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6000), rTransformations[0].mnValue);
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(3999), rTransformations[1].mnValue);
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
        CPPUNIT_ASSERT_EQUAL(sal_Int16(5000), rTransformations[0].mnValue);
    }
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellTextThemeColor)
{
    loadFromURL(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
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
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), rTransformations[0].mnValue);
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(7999), rTransformations[1].mnValue);
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
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), rTransformations[0].mnValue);
            CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(7999), rTransformations[1].mnValue);
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
            CPPUNIT_ASSERT_EQUAL(sal_Int16(5000), rTransformations[0].mnValue);
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
            CPPUNIT_ASSERT_EQUAL(sal_Int16(5000), rTransformations[0].mnValue);
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
            CPPUNIT_ASSERT_EQUAL(sal_Int16(5000), rTransformations[0].mnValue);
        }
    }
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellBorderThemeColor)
{
    loadFromURL(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");
    checkCellBorderThemeColor(getScDoc());
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
