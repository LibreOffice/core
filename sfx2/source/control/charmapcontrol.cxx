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
#include <sfx2/charmapcontrol.hxx>
#include <vcl/event.hxx>

using namespace css;

SfxCharmapCtrl::SfxCharmapCtrl(sal_uInt16 nId, vcl::Window* pParent, const css::uno::Reference< css::frame::XFrame >& rFrame)
    : SfxPopupWindow(nId, pParent, "charmapctrl", "sfx/ui/charmapcontrol.ui", rFrame)
{
    get( m_pRecentCharView[0], "viewchar1" );
    get( m_pRecentCharView[1], "viewchar2" );
    get( m_pRecentCharView[2], "viewchar3" );
    get( m_pRecentCharView[3], "viewchar4" );
    get( m_pRecentCharView[4], "viewchar5" );
    get( m_pRecentCharView[5], "viewchar6" );
    get( m_pRecentCharView[6], "viewchar7" );
    get( m_pRecentCharView[7], "viewchar8" );
    get( m_pRecentCharView[8], "viewchar9" );
    get( m_pRecentCharView[9], "viewchar10" );
    get( m_pRecentCharView[10], "viewchar11" );
    get( m_pRecentCharView[11], "viewchar12" );
    get( m_pRecentCharView[12], "viewchar13" );
    get( m_pRecentCharView[13], "viewchar14" );
    get( m_pRecentCharView[14], "viewchar15" );
    get( m_pRecentCharView[15], "viewchar16" );

    get( m_pFavCharView[0], "favchar1" );
    get( m_pFavCharView[1], "favchar2" );
    get( m_pFavCharView[2], "favchar3" );
    get( m_pFavCharView[3], "favchar4" );
    get( m_pFavCharView[4], "favchar5" );
    get( m_pFavCharView[5], "favchar6" );
    get( m_pFavCharView[6], "favchar7" );
    get( m_pFavCharView[7], "favchar8" );
    get( m_pFavCharView[8], "favchar9" );
    get( m_pFavCharView[9], "favchar10" );
    get( m_pFavCharView[10], "favchar11" );
    get( m_pFavCharView[11], "favchar12" );
    get( m_pFavCharView[12], "favchar13" );
    get( m_pFavCharView[13], "favchar14" );
    get( m_pFavCharView[14], "favchar15" );
    get( m_pFavCharView[15], "favchar16" );

    get( maDlgBtn, "specialchardlg");

    for(int i = 0; i < 16; i++)
    {
        m_pRecentCharView[i]->SetStyle(m_pRecentCharView[i]->GetStyle() | WB_GROUP);
        m_pRecentCharView[i]->setMouseClickHdl(LINK(this,SfxCharmapCtrl, CharClickHdl));
        m_pRecentCharView[i]->SetGetFocusHdl(LINK(this,SfxCharmapCtrl, FocusHdl));
        m_pRecentCharView[i]->SetLoseFocusHdl(LINK(this,SfxCharmapCtrl, FocusHdl));
        m_pFavCharView[i]->SetStyle(m_pFavCharView[i]->GetStyle() | WB_GROUP);
        m_pFavCharView[i]->setMouseClickHdl(LINK(this,SfxCharmapCtrl, CharClickHdl));
        m_pFavCharView[i]->SetGetFocusHdl(LINK(this,SfxCharmapCtrl, FocusHdl));
        m_pFavCharView[i]->SetLoseFocusHdl(LINK(this,SfxCharmapCtrl, FocusHdl));
    }

    maDlgBtn->SetClickHdl(LINK(this, SfxCharmapCtrl, OpenDlgHdl));

    getRecentCharacterList();
    updateRecentCharControl();
    getFavCharacterList();
    updateFavCharControl();
}

SfxCharmapCtrl::~SfxCharmapCtrl()
{
    disposeOnce();
}


void SfxCharmapCtrl::dispose()
{
    for(int i = 0; i < 16; i++)
        m_pRecentCharView[i].clear();

    maRecentCharList.clear();
    maRecentCharFontList.clear();
    maDlgBtn.clear();

    SfxPopupWindow::dispose();
}


void SfxCharmapCtrl::getFavCharacterList()
{
    //retrieve recent character list
    css::uno::Sequence< OUString > rFavCharList( officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::get() );
    std::copy(rFavCharList.begin(), rFavCharList.end(), std::back_inserter(maFavCharList));

    //retrieve recent character font list
    css::uno::Sequence< OUString > rFavCharFontList( officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::get() );
    std::copy(rFavCharFontList.begin(), rFavCharFontList.end(), std::back_inserter(maFavCharFontList));
}


void SfxCharmapCtrl::updateFavCharControl()
{
    int i = 0;
    for ( std::deque< OUString >::iterator it = maFavCharList.begin(), it2 = maFavCharFontList.begin();
        it != maFavCharList.end() || it2 != maFavCharFontList.end();
        ++it, ++it2, i++)
    {
        m_pFavCharView[i]->SetText(*it);
        vcl::Font rFont = m_pFavCharView[i]->GetControlFont();
        rFont.SetFamilyName( *it2 );
        m_pFavCharView[i]->SetFont(rFont);
        m_pFavCharView[i]->Show();
    }

    for(; i < 16 ; i++)
    {
        m_pFavCharView[i]->SetText(OUString());
        m_pFavCharView[i]->Hide();
    }
}


void SfxCharmapCtrl::getRecentCharacterList()
{
    //retrieve recent character list
    css::uno::Sequence< OUString > rRecentCharList( officecfg::Office::Common::RecentCharacters::RecentCharacterList::get() );
    std::copy(rRecentCharList.begin(), rRecentCharList.end(), std::back_inserter(maRecentCharList));

    //retrieve recent character font list
    css::uno::Sequence< OUString > rRecentCharFontList( officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::get() );
    std::copy(rRecentCharFontList.begin(), rRecentCharFontList.end(), std::back_inserter(maRecentCharFontList));
}


void SfxCharmapCtrl::updateRecentCharControl()
{
    int i = 0;
    for ( std::deque< OUString >::iterator it = maRecentCharList.begin(), it2 = maRecentCharFontList.begin();
        it != maRecentCharList.end() || it2 != maRecentCharFontList.end();
        ++it, ++it2, i++)
    {
        m_pRecentCharView[i]->SetText(*it);
        vcl::Font rFont = m_pRecentCharView[i]->GetControlFont();
        rFont.SetFamilyName( *it2 );
        m_pRecentCharView[i]->SetFont(rFont);
        m_pRecentCharView[i]->Show();
    }

    for(; i < 16 ; i++)
    {
        m_pRecentCharView[i]->SetText(OUString());
        m_pRecentCharView[i]->Hide();
    }
}


bool SfxCharmapCtrl::EventNotify( NotifyEvent& rNEvt )
{
    if ( maDlgBtn->HasFocus() && rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const vcl::KeyCode& rKey = rNEvt.GetKeyEvent()->GetKeyCode();
        const sal_uInt16 nCode = rKey.GetCode();
        if ( nCode != KEY_TAB && nCode != KEY_RETURN && nCode != KEY_SPACE && nCode != KEY_ESCAPE )
        {
            return true;
        }
        if ( mbNeedsInit && nCode == KEY_TAB )
        {
            for(int i = 0; i < 16; i++)
            {
                m_pRecentCharView[i]->set_property( "can-focus", "true" );
                m_pFavCharView[i]->set_property( "can-focus", "true" );
            }
            mbNeedsInit = false;
        }
    }
    return SfxPopupWindow::EventNotify( rNEvt );
}


IMPL_STATIC_LINK(SfxCharmapCtrl, FocusHdl, Control&, pItem, void)
{
    pItem.Invalidate();
}


IMPL_LINK(SfxCharmapCtrl, CharClickHdl, SvxCharViewControl*, rView, void)
{
    rView->InsertCharToDoc();
    Close();
}

IMPL_LINK_NOARG(SfxCharmapCtrl, OpenDlgHdl, Button*, void)
{
    Close();

    comphelper::dispatchCommand(".uno:InsertSymbol", GetFrame(), {});
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
