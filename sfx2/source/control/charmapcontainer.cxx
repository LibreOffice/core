/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertyvalue.hxx>
#include <officecfg/Office/Common.hxx>
#include <charmapcontrol.hxx>
#include <charmappopup.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/temporary.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/weld/Menu.hxx>
#include <vcl/weld/weldutils.hxx>

#include <unicode/uchar.h>
#include <unicode/utypes.h>

using namespace css;

SfxCharmapContainer::SfxCharmapContainer(weld::Builder& rBuilder)
    : m_xRecentIconView(rBuilder.weld_icon_view(u"recentchars_iconview"_ustr))
    , m_xFavIconView(rBuilder.weld_icon_view(u"favchars_iconview"_ustr))
{
}

void SfxCharmapContainer::init(const Link<const CharAndFont&, void>& rActivateHdl,
                               const Link<void*, void>& rUpdateFavHdl,
                               const Link<void*, void>& rUpdateRecentHdl,
                               const Link<const CharAndFont&, void>& rCharSelectedHdl)
{
    m_aCharActivateHdl = rActivateHdl;
    m_aCharSelectedHdl = rCharSelectedHdl;
    m_aUpdateFavHdl = rUpdateFavHdl;
    m_aUpdateRecentHdl = rUpdateRecentHdl;

    getRecentCharacterList();
    updateRecentCharControl();
    getFavCharacterList();
    updateFavCharControl();

    m_xRecentIconView->connect_selection_changed(LINK(this, SfxCharmapContainer, IconViewSelectionChangedHdl));
    m_xRecentIconView->connect_item_activated(LINK(this, SfxCharmapContainer, ItemActivatedHdl));
    m_xRecentIconView->connect_focus_in(LINK(this, SfxCharmapContainer, ItemViewFocusInHdl));
    m_xRecentIconView->connect_focus_out(LINK(this, SfxCharmapContainer, ItemViewFocusOutHdl));
    m_xRecentIconView->connect_command(LINK(this, SfxCharmapContainer, RecentContextMenuHdl));

    m_xFavIconView->connect_selection_changed(LINK(this, SfxCharmapContainer, IconViewSelectionChangedHdl));
    m_xFavIconView->connect_item_activated(LINK(this, SfxCharmapContainer, ItemActivatedHdl));
    m_xFavIconView->connect_focus_in(LINK(this, SfxCharmapContainer, ItemViewFocusInHdl));
    m_xFavIconView->connect_focus_out(LINK(this, SfxCharmapContainer, ItemViewFocusOutHdl));
    m_xFavIconView->connect_command(LINK(this, SfxCharmapContainer, FavContextMenuHdl));
}

VclPtr<VirtualDevice> SfxCharmapContainer::CreateIcon(weld::IconView& rIconView,
                                                      const OUString& rFont, const OUString& rText)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    vcl::Font aFont = rStyleSettings.GetLabelFont();
    Size aFontSize = aFont.GetFontSize();
    aFont.SetFontSize(Size(aFontSize.Width() * 2, aFontSize.Height() * 2));

    VclPtr<VirtualDevice> pDev = rIconView.create_virtual_device();

    Size aSize;
    {
        auto popIt = pDev->ScopedPush(PUSH_ALLFONT);
        weld::SetPointFont(*pDev, aFont);
        aSize = Size(pDev->approximate_digit_width() * 2, pDev->GetTextHeight());
    }

    tools::Long nWinHeight = aSize.Height();
    aFont.SetFamilyName(rFont);
    aFont.SetWeight(WEIGHT_NORMAL);
    aFont.SetAlignment(ALIGN_TOP);
    aFont.SetFontSize(pDev->PixelToLogic(Size(0, nWinHeight / 2)));
    aFont.SetTransparent(true);

    pDev->SetOutputSize(aSize);
    pDev->SetFont(aFont);

    const Color aWindowTextColor(rStyleSettings.GetFieldTextColor());
    Color aFillColor(rStyleSettings.GetWindowColor());
    Color aTextColor(rStyleSettings.GetWindowTextColor());
    Color aShadowColor(rStyleSettings.GetShadowColor());

    bool bGotBoundary = true;
    tools::Rectangle aBoundRect;

    tools::Long nY = (nWinHeight - pDev->GetTextHeight()) / 2;

    for (tools::Long nFontHeight = aFontSize.Height(); nFontHeight > 0; nFontHeight -= 1)
    {
        if (!pDev->GetTextBoundRect(aBoundRect, rText) || aBoundRect.IsEmpty())
        {
            bGotBoundary = false;
            break;
        }

        tools::Long nTextWidth = aBoundRect.GetWidth();
        if (aSize.Width() > nTextWidth)
            break;
        vcl::Font aFont2(aFont);
        aFontSize.setHeight(nFontHeight);
        aFont2.SetFontSize(aFontSize);
        pDev->SetFont(aFont2);
        nY = (nWinHeight - pDev->GetTextHeight()) / 2;
    }

    Point aPoint(2, nY);

    if (!bGotBoundary)
        aPoint.setX((aSize.Width() - pDev->GetTextWidth(rText)) / 2);
    else
    {
        // adjust position
        aBoundRect += aPoint;

        // vertical adjustment
        int nYLDelta = aBoundRect.Top();
        int nYHDelta = aSize.Height() - aBoundRect.Bottom();
        if (nYLDelta <= 0)
            aPoint.AdjustY(-(nYLDelta - 1));
        else if (nYHDelta <= 0)
            aPoint.AdjustY(nYHDelta - 1);

        // centrally align glyph
        aPoint.setX(-aBoundRect.Left() + (aSize.Width() - aBoundRect.GetWidth()) / 2);
    }

    pDev->SetFillColor(aFillColor);
    pDev->SetLineColor(aShadowColor);
    pDev->DrawRect(tools::Rectangle(Point(0, 0), aSize));

    pDev->SetTextColor(aWindowTextColor);
    pDev->DrawText(aPoint, rText);

    pDev->SetFillColor(aFillColor);
    pDev->SetTextColor(aTextColor);

    return pDev;
}

void SfxCharmapContainer::getFavCharacterList()
{
    m_aFavChars.clear();

    //retrieve recent character list
    const css::uno::Sequence<OUString> aFavCharList(officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::get());

    //retrieve recent character font list
    const css::uno::Sequence<OUString> aFavCharFontList(officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::get());

    const auto nCommonLength = std::min(aFavCharList.size(), aFavCharFontList.size());
    for (sal_uInt32 i = 0; i < nCommonLength; i++)
        m_aFavChars.emplace_back(aFavCharList[i], aFavCharFontList[i]);
}

void SfxCharmapContainer::updateFavCharControl()
{
    updateCharControl(*m_xFavIconView, m_aFavChars);

    m_aUpdateFavHdl.Call(nullptr);
}

void SfxCharmapContainer::getRecentCharacterList()
{
    m_aRecentChars.clear();

    //retrieve recent character list
    const css::uno::Sequence<OUString> aRecentCharList(officecfg::Office::Common::RecentCharacters::RecentCharacterList::get());

    //retrieve recent character font list
    const css::uno::Sequence<OUString> aRecentCharFontList(officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::get());

    const sal_uInt32 nCommonLength = std::min(aRecentCharList.size(), aRecentCharFontList.size());
    for (sal_uInt32 i = 0; i < nCommonLength; i++)
        m_aRecentChars.emplace_back(aRecentCharList[i], aRecentCharFontList[i]);
}

void SfxCharmapContainer::GrabFocusToFirstFavorite()
{
    std::unique_ptr<weld::TreeIter> pIter = m_xFavIconView->make_iterator();
    if (m_xFavIconView->get_iter_first(*pIter))
    {
        m_xFavIconView->select(*pIter);
        m_xFavIconView->grab_focus();
    }
}

void SfxCharmapContainer::updateRecentCharControl()
{
    updateCharControl(*m_xRecentIconView, m_aRecentChars);

    m_aUpdateRecentHdl.Call(nullptr);
}

void SfxCharmapContainer::updateRecentCharacterList(const OUString& rTitle, const OUString& rFont)
{
    // if recent char to be added is already in list, remove it
    auto itChar = std::ranges::find(m_aRecentChars, CharAndFont(rTitle, rFont));
    if (itChar != m_aRecentChars.end())
        m_aRecentChars.erase(itChar);

    if (m_aRecentChars.size() == 16)
        m_aRecentChars.pop_back();

    m_aRecentChars.emplace_front(rTitle, rFont);

    css::uno::Sequence<OUString> aRecentCharList(m_aRecentChars.size());
    auto aRecentCharListRange = asNonConstRange(aRecentCharList);
    css::uno::Sequence<OUString> aRecentCharFontList(m_aRecentChars.size());
    auto aRecentCharFontListRange = asNonConstRange(aRecentCharFontList);

    for (size_t i = 0; i < m_aRecentChars.size(); ++i)
    {
        aRecentCharListRange[i] = m_aRecentChars[i].sChar;
        aRecentCharFontListRange[i] = m_aRecentChars[i].sFont;
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::RecentCharacters::RecentCharacterList::set(aRecentCharList, batch);
    officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::set(aRecentCharFontList, batch);
    batch->commit();

    updateRecentCharControl();
}

void SfxCharmapContainer::updateFavCharacterList(const OUString& rTitle, const OUString& rFont)
{
    // if Fav char to be added is already in list, remove it
    auto itChar = std::ranges::find(m_aFavChars, CharAndFont(rTitle, rFont));
    if (itChar != m_aFavChars.end())
        m_aFavChars.erase(itChar);

    if (m_aFavChars.size() == 16)
        m_aFavChars.pop_back();

    m_aFavChars.emplace_back(rTitle, rFont);

    css::uno::Sequence<OUString> aFavCharList(m_aFavChars.size());
    auto aFavCharListRange = asNonConstRange(aFavCharList);
    css::uno::Sequence<OUString> aFavCharFontList(m_aFavChars.size());
    auto aFavCharFontListRange = asNonConstRange(aFavCharFontList);

    for (size_t i = 0; i < m_aFavChars.size(); ++i)
    {
        aFavCharListRange[i] = m_aFavChars[i].sChar;
        aFavCharFontListRange[i] = m_aFavChars[i].sFont;
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::set(aFavCharList, batch);
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::set(aFavCharFontList, batch);
    batch->commit();
}

void SfxCharmapContainer::deleteFavCharacterFromList(const OUString& rTitle, const OUString& rFont)
{
    // if Fav char is found, remove it
    auto itChar = std::ranges::find(m_aFavChars, CharAndFont(rTitle, rFont));
    if (itChar != m_aFavChars.end())
        m_aFavChars.erase(itChar);

    css::uno::Sequence<OUString> aFavCharList(m_aFavChars.size());
    auto aFavCharListRange = asNonConstRange(aFavCharList);
    css::uno::Sequence<OUString> aFavCharFontList(m_aFavChars.size());
    auto aFavCharFontListRange = asNonConstRange(aFavCharFontList);

    for (size_t i = 0; i < m_aFavChars.size(); ++i)
    {
        aFavCharListRange[i] = m_aFavChars[i].sChar;
        aFavCharFontListRange[i] = m_aFavChars[i].sFont;
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::set(aFavCharList, batch);
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::set(aFavCharFontList, batch);
    batch->commit();
}

bool SfxCharmapContainer::isFavChar(const OUString& rTitle, const OUString& rFont)
{
    return std::ranges::find(m_aFavChars, CharAndFont(rTitle, rFont)) != m_aFavChars.end();
}

bool SfxCharmapContainer::HandleContextMenu(weld::IconView& rIconView,
                                            std::deque<CharAndFont>& rChars,
                                            const Link<const CharAndFont&, void>& rClearHdl,
                                            const Link<weld::IconView&, void>& rClearAllHdl,
                                            const CommandEvent& rCmdEvent)
{
    if (rCmdEvent.GetCommand() != CommandEventId::ContextMenu)
        return false;

    Point aPos;
    std::unique_ptr<weld::TreeIter> pItem;
    if (rCmdEvent.IsMouseEvent())
    {
        aPos = rCmdEvent.GetMousePosPixel();
        pItem = rIconView.get_item_at_pos(aPos);
        if (!pItem)
            return false;
        rIconView.select(*pItem);
    }
    else
    {
        pItem = rIconView.get_selected();
        if (!pItem)
            return false;
        aPos = rIconView.get_rect(*pItem).Center();
    }

    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(&rIconView, u"sfx/ui/charviewmenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xItemMenu(xBuilder->weld_menu(u"charviewmenu"_ustr));
    const OUString sMenuId
        = xItemMenu->popup_at_rect(&rIconView, tools::Rectangle(aPos, Size(1, 1)));
    if (sMenuId == u"clearchar")
        rClearHdl.Call(rChars.at(rIconView.get_iter_index_in_parent(*pItem)));
    else if (sMenuId == u"clearallchar")
        rClearAllHdl.Call(rIconView);

    return true;
}

void SfxCharmapContainer::updateCharControl(weld::IconView& rIconView,
                                            const std::deque<CharAndFont>& rChars)
{
    rIconView.clear();

    for (auto it = rChars.begin(); it != rChars.end(); ++it)
    {
        const int nIndex = rIconView.n_children();
        const OUString* pNullIconName = nullptr;
        rIconView.insert(nIndex, nullptr, nullptr, pNullIconName, nullptr);
        ScopedVclPtr<VirtualDevice> pDev = CreateIcon(rIconView, it->sFont, it->sChar);
        rIconView.set_image(nIndex, *pDev);
        OUString sAccName;
        if (GetDecimalValueAndCharName(it->sChar, o3tl::temporary(sal_UCS4()), sAccName))
            rIconView.set_item_accessible_name(nIndex, sAccName);
        rIconView.set_item_tooltip_text(nIndex, GetCharInfoText(it->sChar));
    }
}

IMPL_LINK(SfxCharmapContainer, IconViewSelectionChangedHdl, weld::IconView&, rIconView, void)
{
    std::unique_ptr<weld::TreeIter> pIter = rIconView.get_selected();
    if (!pIter)
        return;

    const int nIndex = rIconView.get_iter_index_in_parent(*pIter);
    if (&rIconView == m_xRecentIconView.get())
        m_aCharSelectedHdl.Call(m_aRecentChars.at(nIndex));
    else if (&rIconView == m_xFavIconView.get())
        m_aCharSelectedHdl.Call(m_aFavChars.at(nIndex));
}

IMPL_LINK(SfxCharmapContainer, ItemViewFocusInHdl, weld::Widget&, rWidget, void)
{
    weld::IconView& rIconView = dynamic_cast<weld::IconView&>(rWidget);
    std::unique_ptr<weld::TreeIter> pIter = rIconView.make_iterator();
    std::unique_ptr<weld::TreeIter> pCursor = rIconView.get_cursor();
    if (pCursor)
        rIconView.select(*pCursor);
    else if (rIconView.get_iter_first(*pIter))
        rIconView.select(*pIter);
    else
        return;

    IconViewSelectionChangedHdl(rIconView);
}

IMPL_STATIC_LINK(SfxCharmapContainer, ItemViewFocusOutHdl,  weld::Widget&, rWidget, void)
{
    weld::IconView& rIconView = dynamic_cast<weld::IconView&>(rWidget);
    rIconView.unselect_all();
}

IMPL_LINK(SfxCharmapContainer, ItemActivatedHdl, weld::IconView&, rIconView, bool)
{
    std::unique_ptr<weld::TreeIter> pIter = rIconView.get_selected();
    if (!pIter)
        return false;

    const int nIndex = rIconView.get_iter_index_in_parent(*pIter);
    if (&rIconView == m_xRecentIconView.get())
        m_aCharActivateHdl.Call(m_aRecentChars.at(nIndex));
    else if (&rIconView == m_xFavIconView.get())
        m_aCharActivateHdl.Call(m_aFavChars.at(nIndex));

    return true;
}

IMPL_LINK(SfxCharmapContainer, RecentContextMenuHdl, const CommandEvent&, rCmdEvent, bool)
{
    return HandleContextMenu(*m_xRecentIconView, m_aRecentChars,
                             LINK(this, SfxCharmapContainer, RecentClearClickHdl),
                             LINK(this, SfxCharmapContainer, RecentClearAllClickHdl), rCmdEvent);
}

IMPL_LINK(SfxCharmapContainer, FavContextMenuHdl, const CommandEvent&, rCmdEvent, bool)
{
    return HandleContextMenu(*m_xFavIconView, m_aFavChars,
                             LINK(this, SfxCharmapContainer, FavClearClickHdl),
                             LINK(this, SfxCharmapContainer, FavClearAllClickHdl), rCmdEvent);
}

IMPL_LINK(SfxCharmapContainer, RecentClearClickHdl, const CharAndFont&, rCharAndFont, void)
{
    // if recent char to be added is already in list, remove it
    auto itChar = std::ranges::find(m_aRecentChars, rCharAndFont);
    if (itChar != m_aRecentChars.end())
        m_aRecentChars.erase(itChar);

    css::uno::Sequence<OUString> aRecentCharList(m_aRecentChars.size());
    auto aRecentCharListRange = asNonConstRange(aRecentCharList);
    css::uno::Sequence<OUString> aRecentCharFontList(m_aRecentChars.size());
    auto aRecentCharFontListRange = asNonConstRange(aRecentCharFontList);

    for (size_t i = 0; i < m_aRecentChars.size(); ++i)
    {
        aRecentCharListRange[i] = m_aRecentChars[i].sChar;
        aRecentCharFontListRange[i] = m_aRecentChars[i].sFont;
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::RecentCharacters::RecentCharacterList::set(aRecentCharList, batch);
    officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::set(aRecentCharFontList, batch);
    batch->commit();

    updateRecentCharControl();
}

IMPL_LINK_NOARG(SfxCharmapContainer, RecentClearAllClickHdl, weld::IconView&, void)
{
    m_aRecentChars.clear();

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::RecentCharacters::RecentCharacterList::set({ }, batch);
    officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::set({ }, batch);
    batch->commit();

    updateRecentCharControl();
}

IMPL_LINK(SfxCharmapContainer, FavClearClickHdl, const CharAndFont&, rChar, void)
{
    deleteFavCharacterFromList(rChar.sChar, rChar.sFont);
    updateFavCharControl();
}

IMPL_LINK_NOARG(SfxCharmapContainer, FavClearAllClickHdl, weld::IconView&, void)
{
    m_aFavChars.clear();

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::set({ }, batch);
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::set({ }, batch);
    batch->commit();

    updateFavCharControl();
}

bool SfxCharmapContainer::FavCharListIsFull() const
{
    return m_aFavChars.size() == 16;
}

bool SfxCharmapContainer::hasRecentChars() const
{
    return !m_aRecentChars.empty();
}

bool SfxCharmapContainer::GetDecimalValueAndCharName(std::u16string_view sCharText,
                                                     sal_UCS4& rDecimalValue, OUString& rCharName)
{
    if (sCharText.empty())
        return false;

    sal_UCS4 nDecimalValue = o3tl::iterateCodePoints(sCharText, &o3tl::temporary(sal_Int32(1)), -1);
    /* get the character name */
    UErrorCode errorCode = U_ZERO_ERROR;
    // icu has a private uprv_getMaxCharNameLength function which returns the max possible
    // length of this property. Unicode 3.2 max char name length was 83
    char buffer[100];
    u_charName(nDecimalValue, U_UNICODE_CHAR_NAME, buffer, sizeof(buffer), &errorCode);
    if (U_SUCCESS(errorCode))
    {
        rDecimalValue = nDecimalValue;
        rCharName = OUString::createFromAscii(buffer);
        return true;
    }
    return false;
}

OUString SfxCharmapContainer::GetCharInfoText(std::u16string_view sCharText)
{
    sal_UCS4 nDecimalValue = 0;
    OUString sCharName;
    const bool bSuccess = GetDecimalValueAndCharName(sCharText, nDecimalValue, sCharName);
    if (bSuccess)
    {
        auto aHexText = OUString::number(nDecimalValue, 16).toAsciiUpperCase();
        return sCharText + u" "_ustr + sCharName + u" U+" + aHexText;
    }
    return OUString();
}

void SfxCharmapContainer::InsertCharToDoc(const CharAndFont& rChar)
{
    if (rChar.sChar.isEmpty())
        return;

    uno::Sequence<beans::PropertyValue> aArgs{
        comphelper::makePropertyValue(u"Symbols"_ustr, rChar.sChar),
        comphelper::makePropertyValue(u"FontName"_ustr, rChar.sFont)
    };

    comphelper::dispatchCommand(u".uno:InsertSymbol"_ustr, aArgs);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
