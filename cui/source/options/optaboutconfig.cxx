/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "optaboutconfig.hxx"
#include "optHeaderTabListbox.hxx"

using namespace svx;

CuiAboutConfigTabPage::CuiAboutConfigTabPage( Window* pParent, const SfxItemSet& rItemSet )
    :SfxTabPage( pParent, "AboutConfig", "cui/ui/aboutconfigdialog.ui", rItemSet)
{
    get(m_pDefaultBtn,"default");
    get(m_pEditBtn, "edit");

    m_pPrefCtrl = get<SvxSimpleTableContainer>("preferences");

    Size aControlSize(200,200);
    m_pPrefCtrl->set_width_request(aControlSize.Width());
    m_pPrefCtrl->set_height_request(aControlSize.Height());

    WinBits nBits = WB_SCROLL | WB_SORT;
    pPrefBox = new svx::OptHeaderTabListBox( *m_pPrefCtrl, nBits );
    //FIXME use define's for ItemID's, InsertItem(itemID, text, nsize, winbits)
    HeaderBar &rBar = pPrefBox->GetTheHeaderBar();
    rBar.InsertItem( 1, get<FixedText>("preference")->GetText(), 0, HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( 2, get<FixedText>("status")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( 3, get<FixedText>("type")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( 4, get<FixedText>("value")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );
    //FIXME this numbers are just for testing purposes implement this better and more dynamic way.
    long aTabs[] = {12,12,12,12};

    pPrefBox->SetTabs(aTabs, MAP_PIXEL);

}

CuiAboutConfigTabPage::~CuiAboutConfigTabPage()
{
}

SfxTabPage* CuiAboutConfigTabPage::Create( Window* pParent, const SfxItemSet& rItemSet )
{
    return ( new CuiAboutConfigTabPage( pParent, rItemSet) );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
