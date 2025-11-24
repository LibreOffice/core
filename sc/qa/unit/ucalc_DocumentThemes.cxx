/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <docmodel/theme/Theme.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/borderline.hxx>

#include <patattr.hxx>
#include <scitems.hxx>
#include <undomanager.hxx>
#include <ThemeColorChanger.hxx>

using namespace css;

class DocumentThemesTest : public ScUcalcTestBase
{
};

namespace
{
CPPUNIT_TEST_FIXTURE(DocumentThemesTest, testGetTheme)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);
    auto const& pTheme = pDrawLayer->getTheme();
    CPPUNIT_ASSERT(pTheme);
}

std::shared_ptr<model::ColorSet> createTestTheme()
{
    auto pColorSet = std::make_shared<model::ColorSet>("TestColorScheme");
    pColorSet->add(model::ThemeColorType::Dark1, 0x000000);
    pColorSet->add(model::ThemeColorType::Light1, 0x111111);
    pColorSet->add(model::ThemeColorType::Dark2, 0x222222);
    pColorSet->add(model::ThemeColorType::Light2, 0x333333);
    pColorSet->add(model::ThemeColorType::Accent1, 0x444444);
    pColorSet->add(model::ThemeColorType::Accent2, 0x555555);
    pColorSet->add(model::ThemeColorType::Accent3, 0x666666);
    pColorSet->add(model::ThemeColorType::Accent4, 0x777777);
    pColorSet->add(model::ThemeColorType::Accent5, 0x888888);
    pColorSet->add(model::ThemeColorType::Accent6, 0x999999);
    pColorSet->add(model::ThemeColorType::Hyperlink, 0xaaaaaa);
    pColorSet->add(model::ThemeColorType::FollowedHyperlink, 0xbbbbbb);
    return pColorSet;
}

CPPUNIT_TEST_FIXTURE(DocumentThemesTest, testChangeTheme)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Get the theme and some constants
    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);
    auto const& pTheme = pDrawLayer->getTheme();
    CPPUNIT_ASSERT(pTheme);

    const Color aBackgroundThemeColor(0xc99c00);
    const auto eBackgroundThemeType = model::ThemeColorType::Accent5;

    const Color aCellTextThemeColor(0x0369a3);
    const auto eCellTextThemeType = model::ThemeColorType::Accent2;

    const Color aCellBorderLeftThemeColor(0x18a303);
    const Color aCellBorderRightThemeColor(0xa33e03);
    const Color aCellBorderTopThemeColor(0x8e03a3);
    const Color aCellBorderBottomThemeColor(0xc9211e);
    const auto eCellBorderLeftThemeType = model::ThemeColorType::Accent1;
    const auto eCellBorderRightThemeType = model::ThemeColorType::Accent3;
    const auto eCellBorderTopThemeType = model::ThemeColorType::Accent4;
    const auto eCellBorderBottomThemeType = model::ThemeColorType::Accent6;

    // Create a new pattern with font, background and borders set to theme colors
    ScPatternAttr aNewPattern(m_pDoc->getCellAttributeHelper());
    {
        model::ComplexColor aComplexColor;
        aComplexColor.setThemeColor(eBackgroundThemeType);
        Color aColor = pTheme->getColorSet()->resolveColor(aComplexColor);
        aNewPattern.GetItemSet().Put(SvxBrushItem(aColor, aComplexColor, ATTR_BACKGROUND));
    }
    {
        model::ComplexColor aComplexColor;
        aComplexColor.setThemeColor(eCellTextThemeType);
        Color aColor = pTheme->getColorSet()->resolveColor(aComplexColor);
        aNewPattern.GetItemSet().Put(SvxColorItem(aColor, aComplexColor, ATTR_FONT_COLOR));
    }
    {
        SvxBoxItem aBoxItem(ATTR_BORDER);
        SvxBorderLineStyle eStyle = SvxBorderLineStyle::SOLID;

        {
            model::ComplexColor aComplexColor;
            aComplexColor.setThemeColor(eCellBorderLeftThemeType);
            Color aColor = pTheme->getColorSet()->resolveColor(aComplexColor);
            editeng::SvxBorderLine aLine(&aColor, 10, eStyle);
            aLine.setComplexColor(aComplexColor);
            aBoxItem.SetLine(&aLine, SvxBoxItemLine::LEFT);
        }

        {
            model::ComplexColor aComplexColor;
            aComplexColor.setThemeColor(eCellBorderRightThemeType);
            Color aColor = pTheme->getColorSet()->resolveColor(aComplexColor);
            editeng::SvxBorderLine aLine(&aColor, 20, eStyle);
            aLine.setComplexColor(aComplexColor);
            aBoxItem.SetLine(&aLine, SvxBoxItemLine::RIGHT);
        }

        {
            model::ComplexColor aComplexColor;
            aComplexColor.setThemeColor(eCellBorderTopThemeType);
            Color aColor = pTheme->getColorSet()->resolveColor(aComplexColor);
            editeng::SvxBorderLine aLine(&aColor, 30, eStyle);
            aLine.setComplexColor(aComplexColor);
            aBoxItem.SetLine(&aLine, SvxBoxItemLine::TOP);
        }

        {
            model::ComplexColor aComplexColor;
            aComplexColor.setThemeColor(eCellBorderBottomThemeType);
            Color aColor = pTheme->getColorSet()->resolveColor(aComplexColor);
            editeng::SvxBorderLine aLine(&aColor, 40, eStyle);
            aLine.setComplexColor(aComplexColor);
            aBoxItem.SetLine(&aLine, SvxBoxItemLine::BOTTOM);
        }
        aNewPattern.GetItemSet().Put(aBoxItem);
    }

    // Apply the pattern to cells C3:E5 (2,2 - 4,4) on the first sheet
    m_pDoc->ApplyPatternAreaTab(2, 2, 4, 4, 0, aNewPattern);

    // Check the colors
    {
        const SfxPoolItem* pItem = nullptr;
        auto* pPattern = m_pDoc->GetPattern(ScAddress(3, 3, 0));
        CPPUNIT_ASSERT(pPattern);

        pPattern->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
        auto pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(aBackgroundThemeColor, pBrushItem->GetColor());

        pPattern->GetItemSet().HasItem(ATTR_FONT_COLOR, &pItem);
        auto pColorItem = static_cast<const SvxColorItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(aCellTextThemeColor, pColorItem->getColor());

        pPattern->GetItemSet().HasItem(ATTR_BORDER, &pItem);
        auto pBorderItem = static_cast<const SvxBoxItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(aCellBorderLeftThemeColor, pBorderItem->GetLeft()->GetColor());
        CPPUNIT_ASSERT_EQUAL(aCellBorderRightThemeColor, pBorderItem->GetRight()->GetColor());
        CPPUNIT_ASSERT_EQUAL(aCellBorderTopThemeColor, pBorderItem->GetTop()->GetColor());
        CPPUNIT_ASSERT_EQUAL(aCellBorderBottomThemeColor, pBorderItem->GetBottom()->GetColor());
    }

    // Create a new theme
    auto pColorSet = createTestTheme();

    // Change the colors in the document to those of the new theme
    sc::ThemeColorChanger aChanger(*m_xDocShell);
    aChanger.apply(pColorSet);

    // Check the values again
    {
        const SfxPoolItem* pItem = nullptr;
        auto* pPattern = m_pDoc->GetPattern(ScAddress(3, 3, 0));
        CPPUNIT_ASSERT(pPattern);

        pPattern->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
        auto pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eBackgroundThemeType), pBrushItem->GetColor());

        pPattern->GetItemSet().HasItem(ATTR_FONT_COLOR, &pItem);
        auto pColorItem = static_cast<const SvxColorItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eCellTextThemeType), pColorItem->getColor());

        pPattern->GetItemSet().HasItem(ATTR_BORDER, &pItem);
        auto pBorderItem = static_cast<const SvxBoxItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eCellBorderLeftThemeType),
                             pBorderItem->GetLeft()->GetColor());
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eCellBorderRightThemeType),
                             pBorderItem->GetRight()->GetColor());
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eCellBorderTopThemeType),
                             pBorderItem->GetTop()->GetColor());
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eCellBorderBottomThemeType),
                             pBorderItem->GetBottom()->GetColor());
    }

    // Undo / Redo

    m_pDoc->GetUndoManager()->Undo();

    {
        const SfxPoolItem* pItem = nullptr;
        auto* pPattern = m_pDoc->GetPattern(ScAddress(3, 3, 0));
        CPPUNIT_ASSERT(pPattern);

        pPattern->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
        auto pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(aBackgroundThemeColor, pBrushItem->GetColor());

        pPattern->GetItemSet().HasItem(ATTR_FONT_COLOR, &pItem);
        auto pColorItem = static_cast<const SvxColorItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(aCellTextThemeColor, pColorItem->getColor());

        pPattern->GetItemSet().HasItem(ATTR_BORDER, &pItem);
        auto pBorderItem = static_cast<const SvxBoxItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(aCellBorderLeftThemeColor, pBorderItem->GetLeft()->GetColor());
        CPPUNIT_ASSERT_EQUAL(aCellBorderRightThemeColor, pBorderItem->GetRight()->GetColor());
        CPPUNIT_ASSERT_EQUAL(aCellBorderTopThemeColor, pBorderItem->GetTop()->GetColor());
        CPPUNIT_ASSERT_EQUAL(aCellBorderBottomThemeColor, pBorderItem->GetBottom()->GetColor());
    }

    m_pDoc->GetUndoManager()->Redo();

    {
        const SfxPoolItem* pItem = nullptr;
        auto* pPattern = m_pDoc->GetPattern(ScAddress(3, 3, 0));
        CPPUNIT_ASSERT(pPattern);

        pPattern->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
        auto pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eBackgroundThemeType), pBrushItem->GetColor());

        pPattern->GetItemSet().HasItem(ATTR_FONT_COLOR, &pItem);
        auto pColorItem = static_cast<const SvxColorItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eCellTextThemeType), pColorItem->getColor());

        pPattern->GetItemSet().HasItem(ATTR_BORDER, &pItem);
        auto pBorderItem = static_cast<const SvxBoxItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eCellBorderLeftThemeType),
                             pBorderItem->GetLeft()->GetColor());
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eCellBorderRightThemeType),
                             pBorderItem->GetRight()->GetColor());
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eCellBorderTopThemeType),
                             pBorderItem->GetTop()->GetColor());
        CPPUNIT_ASSERT_EQUAL(pColorSet->getColor(eCellBorderBottomThemeType),
                             pBorderItem->GetBottom()->GetColor());
    }
}

CPPUNIT_TEST_FIXTURE(DocumentThemesTest, testChangeTheme_TopBottomBorderOnly)
{
    // Check that applying only the top and bottom borders will
    // change when the theme changes.

    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, u"Test"_ustr);

    // Get the theme and define constants
    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);
    auto const& pTheme = pDrawLayer->getTheme();
    CPPUNIT_ASSERT(pTheme);

    const auto eCellBorderTopThemeType = model::ThemeColorType::Accent1;
    const auto eCellBorderBottomThemeType = model::ThemeColorType::Accent2;

    // Create a new pattern with font, background and borders set to theme colors
    ScPatternAttr aNewPattern(m_pDoc->getCellAttributeHelper());
    {
        SvxBoxItem aBoxItem(ATTR_BORDER);
        SvxBorderLineStyle eStyle = SvxBorderLineStyle::SOLID;

        // Create the top line
        {
            model::ComplexColor aComplexColor;
            aComplexColor.setThemeColor(eCellBorderTopThemeType);
            Color aColor = pTheme->getColorSet()->resolveColor(aComplexColor);
            editeng::SvxBorderLine aLine(&aColor, 30, eStyle);
            aLine.setComplexColor(aComplexColor);
            aBoxItem.SetLine(&aLine, SvxBoxItemLine::TOP);
        }

        // Create the bottom line
        {
            model::ComplexColor aComplexColor;
            aComplexColor.setThemeColor(eCellBorderBottomThemeType);
            Color aColor = pTheme->getColorSet()->resolveColor(aComplexColor);
            editeng::SvxBorderLine aLine(&aColor, 40, eStyle);
            aLine.setComplexColor(aComplexColor);
            aBoxItem.SetLine(&aLine, SvxBoxItemLine::BOTTOM);
        }
        aNewPattern.GetItemSet().Put(aBoxItem);
    }

    // Apply the pattern to cells C3:E5 (2,2 - 4,4) on the first sheet
    m_pDoc->ApplyPatternAreaTab(2, 2, 4, 4, 0, aNewPattern);

    // Check the theme colors of the top and bottom borders
    {
        const SfxPoolItem* pItem = nullptr;
        auto* pPattern = m_pDoc->GetPattern(ScAddress(3, 3, 0));
        CPPUNIT_ASSERT(pPattern);

        pPattern->GetItemSet().HasItem(ATTR_BORDER, &pItem);
        auto pBorderItem = static_cast<const SvxBoxItem*>(pItem);

        // Colors should be from the default theme
        CPPUNIT_ASSERT_EQUAL(Color(0x18a303), pBorderItem->GetTop()->GetColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x0369a3), pBorderItem->GetBottom()->GetColor());
    }

    // Create a new theme
    auto pColorSet = createTestTheme();

    // Change the colors in the document to those of the new theme
    sc::ThemeColorChanger aChanger(*m_xDocShell);
    aChanger.apply(pColorSet);

    // Check the border top and bottom colors again
    {
        const SfxPoolItem* pItem = nullptr;
        auto* pPattern = m_pDoc->GetPattern(ScAddress(3, 3, 0));
        CPPUNIT_ASSERT(pPattern);

        pPattern->GetItemSet().HasItem(ATTR_BORDER, &pItem);
        auto pBorderItem = static_cast<const SvxBoxItem*>(pItem);

        // Colors should be from the new theme
        CPPUNIT_ASSERT_EQUAL(Color(0x444444), pBorderItem->GetTop()->GetColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x555555), pBorderItem->GetBottom()->GetColor());
    }
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
