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
#include <sfx2/charmappopup.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>

using namespace css;

SfxCharmapCtrl::SfxCharmapCtrl(CharmapPopup* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "sfx/ui/charmapcontrol.ui", "charmapctrl")
    , m_xControl(pControl)
    , m_xVirDev(VclPtr<VirtualDevice>::Create())
    , m_aRecentCharView{SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev)}
    , m_aFavCharView{SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev)}
    , m_xRecentLabel(m_xBuilder->weld_label("label2"))
    , m_xDlgBtn(m_xBuilder->weld_button("specialchardlg"))
    , m_xRecentCharView{std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar1", m_aRecentCharView[0]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar2", m_aRecentCharView[1]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar3", m_aRecentCharView[2]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar4", m_aRecentCharView[3]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar5", m_aRecentCharView[4]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar6", m_aRecentCharView[5]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar7", m_aRecentCharView[6]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar8", m_aRecentCharView[7]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar9", m_aRecentCharView[8]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar10", m_aRecentCharView[9]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar11", m_aRecentCharView[10]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar12", m_aRecentCharView[11]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar13", m_aRecentCharView[12]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar14", m_aRecentCharView[13]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar15", m_aRecentCharView[14]),
                        std::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar16", m_aRecentCharView[15])}
    , m_xFavCharView{std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar1", m_aFavCharView[0]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar2", m_aFavCharView[1]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar3", m_aFavCharView[2]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar4", m_aFavCharView[3]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar5", m_aFavCharView[4]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar6", m_aFavCharView[5]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar7", m_aFavCharView[6]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar8", m_aFavCharView[7]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar9", m_aFavCharView[8]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar10", m_aFavCharView[9]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar11", m_aFavCharView[10]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar12", m_aFavCharView[11]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar13", m_aFavCharView[12]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar14", m_aFavCharView[13]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar15", m_aFavCharView[14]),
                     std::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar16", m_aFavCharView[15])}
{
    for(int i = 0; i < 16; i++)
    {
        m_aRecentCharView[i].setMouseClickHdl(LINK(this,SfxCharmapCtrl, CharClickHdl));
        m_aFavCharView[i].setMouseClickHdl(LINK(this,SfxCharmapCtrl, CharClickHdl));
    }

    m_xDlgBtn->connect_clicked(LINK(this, SfxCharmapCtrl, OpenDlgHdl));

    getRecentCharacterList();
    updateRecentCharControl();
    getFavCharacterList();
    updateFavCharControl();
}

SfxCharmapCtrl::~SfxCharmapCtrl()
{
}

void SfxCharmapCtrl::getFavCharacterList()
{
    //retrieve recent character list
    css::uno::Sequence< OUString > rFavCharList( officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::get() );
    m_aFavCharList.insert( m_aFavCharList.end(), rFavCharList.begin(), rFavCharList.end() );

    //retrieve recent character font list
    css::uno::Sequence< OUString > rFavCharFontList( officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::get() );
    m_aFavCharFontList.insert( m_aFavCharFontList.end(), rFavCharFontList.begin(), rFavCharFontList.end() );
}

void SfxCharmapCtrl::updateFavCharControl()
{
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
}

void SfxCharmapCtrl::getRecentCharacterList()
{
    //retrieve recent character list
    css::uno::Sequence< OUString > rRecentCharList( officecfg::Office::Common::RecentCharacters::RecentCharacterList::get() );
    m_aRecentCharList.insert( m_aRecentCharList.end(), rRecentCharList.begin(), rRecentCharList.end() );

    //retrieve recent character font list
    css::uno::Sequence< OUString > rRecentCharFontList( officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::get() );
    m_aRecentCharFontList.insert( m_aRecentCharFontList.end(), rRecentCharFontList.begin(), rRecentCharFontList.end() );
}

void SfxCharmapCtrl::updateRecentCharControl()
{
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

    //checking if the characters are recently used or no
    m_xRecentLabel->set_label(m_aRecentCharList.size() > 0 ? SfxResId(STR_RECENT) : SfxResId(STR_NORECENT));
}

IMPL_LINK(SfxCharmapCtrl, CharClickHdl, SvxCharView*, pView, void)
{
    m_xControl->EndPopupMode();

    pView->InsertCharToDoc();
}

IMPL_LINK_NOARG(SfxCharmapCtrl, OpenDlgHdl, weld::Button&, void)
{
    m_xControl->EndPopupMode();

    uno::Reference<frame::XFrame> xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
    comphelper::dispatchCommand(".uno:InsertSymbol", xFrame, {});
}

void SfxCharmapCtrl::GrabFocus()
{
    m_aFavCharView[0].GrabFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
