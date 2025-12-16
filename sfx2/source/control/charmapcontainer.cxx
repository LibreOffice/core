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
#include <officecfg/Office/Common.hxx>
#include <charmapcontrol.hxx>
#include <charmappopup.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <vcl/commandevent.hxx>

using namespace css;

SfxCharmapContainer::SfxCharmapContainer(weld::Builder& rBuilder, const VclPtr<VirtualDevice>& rVirDev, bool bLockGridSizes)
    : m_aRecentCharView{SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev),
                        SvxCharView(rVirDev)}
    , m_aFavCharView{SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev),
                     SvxCharView(rVirDev)}
    , m_xRecentCharView{std::make_unique<weld::CustomWeld>(rBuilder, "viewchar1", m_aRecentCharView[0]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar2", m_aRecentCharView[1]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar3", m_aRecentCharView[2]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar4", m_aRecentCharView[3]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar5", m_aRecentCharView[4]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar6", m_aRecentCharView[5]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar7", m_aRecentCharView[6]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar8", m_aRecentCharView[7]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar9", m_aRecentCharView[8]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar10", m_aRecentCharView[9]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar11", m_aRecentCharView[10]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar12", m_aRecentCharView[11]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar13", m_aRecentCharView[12]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar14", m_aRecentCharView[13]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar15", m_aRecentCharView[14]),
                        std::make_unique<weld::CustomWeld>(rBuilder, "viewchar16", m_aRecentCharView[15])}
    , m_xFavCharView{std::make_unique<weld::CustomWeld>(rBuilder, "favchar1", m_aFavCharView[0]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar2", m_aFavCharView[1]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar3", m_aFavCharView[2]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar4", m_aFavCharView[3]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar5", m_aFavCharView[4]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar6", m_aFavCharView[5]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar7", m_aFavCharView[6]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar8", m_aFavCharView[7]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar9", m_aFavCharView[8]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar10", m_aFavCharView[9]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar11", m_aFavCharView[10]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar12", m_aFavCharView[11]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar13", m_aFavCharView[12]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar14", m_aFavCharView[13]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar15", m_aFavCharView[14]),
                     std::make_unique<weld::CustomWeld>(rBuilder, "favchar16", m_aFavCharView[15])}
    , m_xRecentGrid(rBuilder.weld_widget(u"viewgrid"_ustr))
    , m_xFavGrid(rBuilder.weld_widget(u"favgrid"_ustr))
{
    if (bLockGridSizes)
    {
        //so things don't jump around if all the children are hidden
        m_xRecentGrid->set_size_request(-1, m_aRecentCharView[0].get_preferred_size().Height());
        m_xFavGrid->set_size_request(-1, m_aFavCharView[0].get_preferred_size().Height());
    }
}

void SfxCharmapContainer::init(bool bHasInsert, const Link<SvxCharView&, void>& rMouseClickHdl,
                               const Link<void*, void>& rUpdateFavHdl,
                               const Link<void*, void>& rUpdateRecentHdl,
                               const Link<SvxCharView&, void>& rFocusInHdl)
{
    m_aUpdateFavHdl = rUpdateFavHdl;
    m_aUpdateRecentHdl = rUpdateRecentHdl;

    getRecentCharacterList();
    updateRecentCharControl();
    getFavCharacterList();
    updateFavCharControl();

    for(int i = 0; i < 16; i++)
    {
        m_aRecentCharView[i].SetHasInsert(bHasInsert);
        m_aRecentCharView[i].setFocusInHdl(rFocusInHdl);
        m_aRecentCharView[i].setMouseClickHdl(rMouseClickHdl);
        m_aRecentCharView[i].setContextMenuHdl(
            LINK(this, SfxCharmapContainer, RecentContextMenuHdl));
        m_aFavCharView[i].SetHasInsert(bHasInsert);
        m_aFavCharView[i].setFocusInHdl(rFocusInHdl);
        m_aFavCharView[i].setMouseClickHdl(rMouseClickHdl);
        m_aFavCharView[i].setContextMenuHdl(LINK(this, SfxCharmapContainer, FavContextMenuHdl));
    }
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
    updateCharControl(m_aFavCharView, m_aFavChars);

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
    m_aFavCharView[0].GrabFocus();
}

void SfxCharmapContainer::updateRecentCharControl()
{
    updateCharControl(m_aRecentCharView, m_aRecentChars);

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

void SfxCharmapContainer::HandleContextMenu(std::span<SvxCharView> aCharViews,
                                            const Link<SvxCharView&, void>& rClearHdl,
                                            const Link<SvxCharView&, void>& rClearAllHdl,
                                            const CommandEvent& rCmdEvent)
{
    assert(rCmdEvent.GetCommand() == CommandEventId::ContextMenu);

    for (SvxCharView& rView : aCharViews)
    {
        // the context menu is opened for the currently focused view
        if (!rView.HasFocus())
            continue;

        weld::DrawingArea* pDrawingArea = rView.GetDrawingArea();
        std::unique_ptr<weld::Builder> xBuilder(
            Application::CreateBuilder(pDrawingArea, u"sfx/ui/charviewmenu.ui"_ustr));
        std::unique_ptr<weld::Menu> xItemMenu(xBuilder->weld_menu(u"charviewmenu"_ustr));
        const OUString sMenuId = xItemMenu->popup_at_rect(
            pDrawingArea, tools::Rectangle(rCmdEvent.GetMousePosPixel(), Size(1, 1)));
        if (sMenuId == u"clearchar")
            rClearHdl.Call(rView);
        else if (sMenuId == u"clearallchar")
            rClearAllHdl.Call(rView);

        rView.Invalidate();
        return;
    }
}

void SfxCharmapContainer::updateCharControl(std::span<SvxCharView> aCharViews,
                                            const std::deque<CharAndFont>& rChars)
{
    assert(rChars.size() <= aCharViews.size());

    int i = 0;
    for (auto it = rChars.begin(); it != rChars.end(); ++it, i++)
    {
        aCharViews[i].SetText(it->sChar);
        aCharViews[i].UpdateFont(it->sFont);
        aCharViews[i].Show();
    }

    for (; i < 16; i++)
    {
        aCharViews[i].SetText(OUString());
        aCharViews[i].Hide();
    }
}

IMPL_LINK(SfxCharmapContainer, RecentContextMenuHdl, const CommandEvent&, rCmdEvent, void)
{
    HandleContextMenu(m_aRecentCharView, (LINK(this, SfxCharmapContainer, RecentClearClickHdl)),
                      LINK(this, SfxCharmapContainer, RecentClearAllClickHdl), rCmdEvent);
}

IMPL_LINK(SfxCharmapContainer, FavContextMenuHdl, const CommandEvent&, rCmdEvent, void)
{
    HandleContextMenu(m_aFavCharView, LINK(this, SfxCharmapContainer, FavClearClickHdl),
                      LINK(this, SfxCharmapContainer, FavClearAllClickHdl), rCmdEvent);
}

IMPL_LINK(SfxCharmapContainer, RecentClearClickHdl, SvxCharView&, rView, void)
{
    const OUString& sTitle = rView.GetText();
    OUString sFont = rView.GetFont().GetFamilyName();

    // if recent char to be added is already in list, remove it
    auto itChar = std::ranges::find(m_aRecentChars, CharAndFont(sTitle, sFont));
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

IMPL_LINK_NOARG(SfxCharmapContainer, RecentClearAllClickHdl, SvxCharView&, void)
{
    m_aRecentChars.clear();

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::RecentCharacters::RecentCharacterList::set({ }, batch);
    officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::set({ }, batch);
    batch->commit();

    updateRecentCharControl();
}

IMPL_LINK(SfxCharmapContainer, FavClearClickHdl, SvxCharView&, rView, void)
{
    deleteFavCharacterFromList(rView.GetText(), rView.GetFont().GetFamilyName());
    updateFavCharControl();
}

IMPL_LINK_NOARG(SfxCharmapContainer, FavClearAllClickHdl, SvxCharView&, void)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
