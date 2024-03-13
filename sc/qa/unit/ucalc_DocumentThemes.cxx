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
    m_pDoc->InsertTab(0, "Test");

    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);
    auto const& pTheme = pDrawLayer->getTheme();
    CPPUNIT_ASSERT(pTheme);
}

CPPUNIT_TEST_FIXTURE(DocumentThemesTest, testChangeTheme)
{
    m_pDoc->InitDrawLayer();
    m_pDoc->InsertTab(0, "Test");

    ScDrawLayer* pDrawLayer = m_pDoc->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);
    auto const& pTheme = pDrawLayer->getTheme();
    CPPUNIT_ASSERT(pTheme);

    Color aBackgroundThemeColor(0xc99c00);
    Color aCellTextThemeColor(0x0369a3);

    auto eBackgroundThemeType = model::ThemeColorType::Accent5;
    auto eCellTextThemeType = model::ThemeColorType::Accent2;

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

    // Apply the pattern to cells C3:E5 (2,2 - 4,4) on the first sheet
    m_pDoc->ApplyPatternAreaTab(2, 2, 4, 4, 0, aNewPattern);

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
    }

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

    sc::ThemeColorChanger aChanger(*m_xDocShell);
    aChanger.apply(pColorSet);

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
    }
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
