/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

void SfxCharmapContainer::init(bool bHasInsert, const Link<SvxCharView*,void> &rMouseClickHdl,
                               const Link<void*, void>& rUpdateFavHdl,
                               const Link<void*, void>& rUpdateRecentHdl,
                               const Link<SvxCharView*,void> &rFocusInHdl)
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
        m_aRecentCharView[i].setClearClickHdl(LINK(this, SfxCharmapContainer, RecentClearClickHdl));
        m_aRecentCharView[i].setClearAllClickHdl(LINK(this, SfxCharmapContainer, RecentClearAllClickHdl));
        m_aFavCharView[i].SetHasInsert(bHasInsert);
        m_aFavCharView[i].setFocusInHdl(rFocusInHdl);
        m_aFavCharView[i].setMouseClickHdl(rMouseClickHdl);
        m_aFavCharView[i].setClearClickHdl(LINK(this, SfxCharmapContainer, FavClearClickHdl));
        m_aFavCharView[i].setClearAllClickHdl(LINK(this, SfxCharmapContainer, FavClearAllClickHdl));
    }
}

SfxCharmapCtrl::SfxCharmapCtrl(CharmapPopup* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"sfx/ui/charmapcontrol.ui"_ustr, u"charmapctrl"_ustr)
    , m_xControl(pControl)
    , m_xVirDev(VclPtr<VirtualDevice>::Create())
    , m_aCharmapContents(*m_xBuilder, m_xVirDev, false)
    , m_xRecentLabel(m_xBuilder->weld_label(u"label2"_ustr))
    , m_xDlgBtn(m_xBuilder->weld_button(u"specialchardlg"_ustr))
    , m_xCharInfoLabel(m_xBuilder->weld_label(u"charinfolabel"_ustr))
{
    m_xCharInfoLabel->set_size_request(-1, m_xCharInfoLabel->get_text_height() * 2);

    m_aCharmapContents.init(false, LINK(this, SfxCharmapCtrl, CharClickHdl),
                            Link<void*,void>(), LINK(this, SfxCharmapCtrl, UpdateRecentHdl),
                            LINK(this, SfxCharmapCtrl, CharFocusInHdl));

    m_xDlgBtn->connect_clicked(LINK(this, SfxCharmapCtrl, OpenDlgHdl));
    m_xDlgBtn->connect_focus_in(LINK(this, SfxCharmapCtrl, DlgBtnFocusInHdl));
}

SfxCharmapCtrl::~SfxCharmapCtrl()
{
}

void SfxCharmapContainer::getFavCharacterList()
{
    m_aFavCharList.clear();
    m_aFavCharFontList.clear();

    //retrieve recent character list
    const css::uno::Sequence< OUString > rFavCharList( officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::get() );
    m_aFavCharList.insert( m_aFavCharList.end(), rFavCharList.begin(), rFavCharList.end() );

    //retrieve recent character font list
    const css::uno::Sequence< OUString > rFavCharFontList( officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::get() );
    m_aFavCharFontList.insert( m_aFavCharFontList.end(), rFavCharFontList.begin(), rFavCharFontList.end() );

    // tdf#135997: make sure that the two lists are same length
    const auto nCommonLength = std::min(m_aFavCharList.size(), m_aFavCharFontList.size());
    m_aFavCharList.resize(nCommonLength);
    m_aFavCharFontList.resize(nCommonLength);
}

void SfxCharmapContainer::updateFavCharControl()
{
    assert(m_aFavCharList.size() == m_aFavCharFontList.size());

    int i = 0;
    for ( std::deque< OUString >::iterator it = m_aFavCharList.begin(), it2 = m_aFavCharFontList.begin();
        it != m_aFavCharList.end() && it2 != m_aFavCharFontList.end();
        ++it, ++it2, i++)
    {
        m_aFavCharView[i].SetText(*it);
        vcl::Font rFont = m_aFavCharView[i].GetFont();
        rFont.SetFamilyName( *it2 );
        m_aFavCharView[i].SetFont(rFont);
        m_aFavCharView[i].Show();
    }

    for(; i < 16 ; i++)
    {
        m_aFavCharView[i].SetText(OUString());
        m_aFavCharView[i].Hide();
    }

    m_aUpdateFavHdl.Call(nullptr);
}

void SfxCharmapContainer::getRecentCharacterList()
{
    m_aRecentCharList.clear();
    m_aRecentCharFontList.clear();

    //retrieve recent character list
    const css::uno::Sequence< OUString > rRecentCharList( officecfg::Office::Common::RecentCharacters::RecentCharacterList::get() );
    m_aRecentCharList.insert( m_aRecentCharList.end(), rRecentCharList.begin(), rRecentCharList.end() );

    //retrieve recent character font list
    const css::uno::Sequence< OUString > rRecentCharFontList( officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::get() );
    m_aRecentCharFontList.insert( m_aRecentCharFontList.end(), rRecentCharFontList.begin(), rRecentCharFontList.end() );

    // tdf#135997: make sure that the two lists are same length
    const auto nCommonLength = std::min(m_aRecentCharList.size(), m_aRecentCharFontList.size());
    m_aRecentCharList.resize(nCommonLength);
    m_aRecentCharFontList.resize(nCommonLength);
}

IMPL_LINK(SfxCharmapCtrl, CharFocusInHdl, SvxCharView*, pView, void)
{
    m_xCharInfoLabel->set_label(pView->GetCharInfoText());
}

IMPL_LINK(SfxCharmapCtrl, CharClickHdl, SvxCharView*, pView, void)
{
    m_xControl->EndPopupMode();

    pView->InsertCharToDoc();
}

IMPL_LINK_NOARG(SfxCharmapCtrl, OpenDlgHdl, weld::Button&, void)
{
    m_xControl->EndPopupMode();

    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        uno::Reference<frame::XFrame> xFrame = pViewFrm->GetFrame().GetFrameInterface();
        comphelper::dispatchCommand(u".uno:InsertSymbol"_ustr, xFrame, {});
    }
}

IMPL_LINK_NOARG(SfxCharmapCtrl, DlgBtnFocusInHdl, weld::Widget&, void)
{
    m_xCharInfoLabel->set_label(u""_ustr);
}

void SfxCharmapCtrl::GrabFocus()
{
    m_aCharmapContents.GrabFocusToFirstFavorite();
}

void SfxCharmapContainer::GrabFocusToFirstFavorite()
{
    m_aFavCharView[0].GrabFocus();
}

static std::pair<std::deque<OUString>::const_iterator, std::deque<OUString>::const_iterator>
findInPair(std::u16string_view str1, const std::deque<OUString>& rContainer1,
           std::u16string_view str2, const std::deque<OUString>& rContainer2)
{
    assert(rContainer1.size() == rContainer2.size());

    for (auto it1 = std::find(rContainer1.begin(), rContainer1.end(), str1);
         it1 != rContainer1.end(); it1 = std::find(std::next(it1), rContainer1.end(), str1))
    {
        auto it2 = rContainer2.begin() + (it1 - rContainer1.begin());
        if (*it2 == str2)
            return { it1, it2 };
    }
    return { rContainer1.end(), rContainer2.end() };
}

std::pair<std::deque<OUString>::const_iterator, std::deque<OUString>::const_iterator>
SfxCharmapContainer::getRecentChar(std::u16string_view sTitle, std::u16string_view rFont) const
{
    return findInPair(sTitle, m_aRecentCharList, rFont, m_aRecentCharFontList);
}

std::pair<std::deque<OUString>::const_iterator, std::deque<OUString>::const_iterator>
SfxCharmapContainer::getFavChar(std::u16string_view sTitle, std::u16string_view rFont) const
{
    return findInPair(sTitle, m_aFavCharList, rFont, m_aFavCharFontList);
}

void SfxCharmapContainer::updateRecentCharControl()
{
    assert(m_aRecentCharList.size() == m_aRecentCharFontList.size());

    int i = 0;
    for ( std::deque< OUString >::iterator it = m_aRecentCharList.begin(), it2 = m_aRecentCharFontList.begin();
        it != m_aRecentCharList.end() && it2 != m_aRecentCharFontList.end();
        ++it, ++it2, i++)
    {
        m_aRecentCharView[i].SetText(*it);
        vcl::Font rFont = m_aRecentCharView[i].GetFont();
        rFont.SetFamilyName( *it2 );
        m_aRecentCharView[i].SetFont(rFont);
        m_aRecentCharView[i].Show();
    }

    for(; i < 16 ; i++)
    {
        m_aRecentCharView[i].SetText(OUString());
        m_aRecentCharView[i].Hide();
    }

    m_aUpdateRecentHdl.Call(nullptr);
}

void SfxCharmapContainer::updateRecentCharacterList(const OUString& sTitle, const OUString& rFont)
{
    // if recent char to be added is already in list, remove it
    if( const auto& [itChar, itChar2] = getRecentChar(sTitle, rFont);
        itChar != m_aRecentCharList.end() &&  itChar2 != m_aRecentCharFontList.end() )
    {
        m_aRecentCharList.erase( itChar );
        m_aRecentCharFontList.erase( itChar2);
    }

    if (m_aRecentCharList.size() == 16)
    {
        m_aRecentCharList.pop_back();
        m_aRecentCharFontList.pop_back();
    }

    m_aRecentCharList.push_front(sTitle);
    m_aRecentCharFontList.push_front(rFont);

    css::uno::Sequence< OUString > aRecentCharList(m_aRecentCharList.size());
    auto aRecentCharListRange = asNonConstRange(aRecentCharList);
    css::uno::Sequence< OUString > aRecentCharFontList(m_aRecentCharFontList.size());
    auto aRecentCharFontListRange = asNonConstRange(aRecentCharFontList);

    for (size_t i = 0; i < m_aRecentCharList.size(); ++i)
    {
        aRecentCharListRange[i] = m_aRecentCharList[i];
        aRecentCharFontListRange[i] = m_aRecentCharFontList[i];
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::RecentCharacters::RecentCharacterList::set(aRecentCharList, batch);
    officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::set(aRecentCharFontList, batch);
    batch->commit();

    updateRecentCharControl();
}

void SfxCharmapContainer::updateFavCharacterList(const OUString& sTitle, const OUString& rFont)
{
    // if Fav char to be added is already in list, remove it
    if( const auto& [itChar, itChar2] = getFavChar(sTitle, rFont);
        itChar != m_aFavCharList.end() &&  itChar2 != m_aFavCharFontList.end() )
    {
        m_aFavCharList.erase( itChar );
        m_aFavCharFontList.erase( itChar2);
    }

    if (m_aFavCharList.size() == 16)
    {
        m_aFavCharList.pop_back();
        m_aFavCharFontList.pop_back();
    }

    m_aFavCharList.push_back(sTitle);
    m_aFavCharFontList.push_back(rFont);

    css::uno::Sequence< OUString > aFavCharList(m_aFavCharList.size());
    auto aFavCharListRange = asNonConstRange(aFavCharList);
    css::uno::Sequence< OUString > aFavCharFontList(m_aFavCharFontList.size());
    auto aFavCharFontListRange = asNonConstRange(aFavCharFontList);

    for (size_t i = 0; i < m_aFavCharList.size(); ++i)
    {
        aFavCharListRange[i] = m_aFavCharList[i];
        aFavCharFontListRange[i] = m_aFavCharFontList[i];
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::set(aFavCharList, batch);
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::set(aFavCharFontList, batch);
    batch->commit();
}

void SfxCharmapContainer::deleteFavCharacterFromList(std::u16string_view sTitle, std::u16string_view rFont)
{
    // if Fav char is found, remove it
    if( const auto& [itChar, itChar2] = getFavChar(sTitle, rFont);
        itChar != m_aFavCharList.end() &&  itChar2 != m_aFavCharFontList.end() )
    {
        m_aFavCharList.erase( itChar );
        m_aFavCharFontList.erase( itChar2);
    }

    css::uno::Sequence< OUString > aFavCharList(m_aFavCharList.size());
    auto aFavCharListRange = asNonConstRange(aFavCharList);
    css::uno::Sequence< OUString > aFavCharFontList(m_aFavCharFontList.size());
    auto aFavCharFontListRange = asNonConstRange(aFavCharFontList);

    for (size_t i = 0; i < m_aFavCharList.size(); ++i)
    {
        aFavCharListRange[i] = m_aFavCharList[i];
        aFavCharFontListRange[i] = m_aFavCharFontList[i];
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::set(aFavCharList, batch);
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::set(aFavCharFontList, batch);
    batch->commit();
}

bool SfxCharmapContainer::isFavChar(std::u16string_view sTitle, std::u16string_view rFont)
{
    const auto& [itChar, itFont] = getFavChar(sTitle, rFont);
    return itChar != m_aFavCharList.end() && itFont != m_aFavCharFontList.end();
}

IMPL_LINK(SfxCharmapContainer, RecentClearClickHdl, SvxCharView*, rView, void)
{
    const OUString& sTitle = rView->GetText();
    OUString sFont = rView->GetFont().GetFamilyName();

    // if recent char to be added is already in list, remove it
    if( const auto& [itChar, itChar2] = getRecentChar(sTitle, sFont);
        itChar != m_aRecentCharList.end() &&  itChar2 != m_aRecentCharFontList.end() )
    {
        m_aRecentCharList.erase( itChar );
        m_aRecentCharFontList.erase( itChar2);
    }

    css::uno::Sequence< OUString > aRecentCharList(m_aRecentCharList.size());
    auto aRecentCharListRange = asNonConstRange(aRecentCharList);
    css::uno::Sequence< OUString > aRecentCharFontList(m_aRecentCharFontList.size());
    auto aRecentCharFontListRange = asNonConstRange(aRecentCharFontList);

    for (size_t i = 0; i < m_aRecentCharList.size(); ++i)
    {
        aRecentCharListRange[i] = m_aRecentCharList[i];
        aRecentCharFontListRange[i] = m_aRecentCharFontList[i];
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::RecentCharacters::RecentCharacterList::set(aRecentCharList, batch);
    officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::set(aRecentCharFontList, batch);
    batch->commit();

    updateRecentCharControl();
}

IMPL_LINK_NOARG(SfxCharmapContainer, RecentClearAllClickHdl, SvxCharView*, void)
{
    m_aRecentCharList.clear();
    m_aRecentCharFontList.clear();

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::RecentCharacters::RecentCharacterList::set({ }, batch);
    officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::set({ }, batch);
    batch->commit();

    updateRecentCharControl();
}

IMPL_LINK(SfxCharmapContainer, FavClearClickHdl, SvxCharView*, rView, void)
{
    deleteFavCharacterFromList(rView->GetText(), rView->GetFont().GetFamilyName());
    updateFavCharControl();
}

IMPL_LINK_NOARG(SfxCharmapContainer, FavClearAllClickHdl, SvxCharView*, void)
{
    m_aFavCharList.clear();
    m_aFavCharFontList.clear();

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::set({ }, batch);
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::set({ }, batch);
    batch->commit();

    updateFavCharControl();
}

bool SfxCharmapContainer::FavCharListIsFull() const
{
    return m_aFavCharList.size() == 16;
}

bool SfxCharmapContainer::hasRecentChars() const
{
    return !m_aRecentCharList.empty();
}

IMPL_LINK_NOARG(SfxCharmapCtrl, UpdateRecentHdl, void*, void)
{
    //checking if the characters are recently used or no
    m_xRecentLabel->set_label(m_aCharmapContents.hasRecentChars() ? SfxResId(STR_RECENT) : SfxResId(STR_NORECENT));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
