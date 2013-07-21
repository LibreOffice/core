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
#include <svtools/accessibilityoption.hxx>

using namespace svx;

#define ITEMID_PREF     1
#define ITEMID_TYPE     2
#define ITEMID_STATUS   3
#define ITEMID_VALUE    4

struct CuiAboutConfigTabPage_Impl
{
    SvtAccessibilityOptions     m_aAccConfig;
    CuiAccessibilityOptionsTabPage_Impl()
        : m_aAccConfig(){}
    //FIXME use this struct for all options possible.
}

CuiAboutConfigTabPage::CuiAboutConfigTabPage( Window* pParent, const SfxItemSet& rItemSet )
    :SfxTabPage( pParent, "AboutConfig", "cui/ui/aboutconfigdialog.ui", rItemSet),
    m_pImpl(new CuiAboutConfigTabPage_Impl)
{
    get(m_pDefaultBtn,"default");
    get(m_pEditBtn, "edit");

    m_pPrefCtrl = get<SvxSimpleTableContainer>("preferences");

    Size aControlSize(200,200);
    m_pPrefCtrl->set_width_request(aControlSize.Width());
    m_pPrefCtrl->set_height_request(aControlSize.Height());

    WinBits nBits = WB_SCROLL | WB_SORT | WB_HSCROLL | WB_VSCROLL;
    pPrefBox = new svx::OptHeaderTabListBox( *m_pPrefCtrl, nBits );

    HeaderBar &rBar = pPrefBox->GetTheHeaderBar();
    rBar.InsertItem( ITEMID_PREF, get<FixedText>("preference")->GetText(), 0, HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_TYPE, get<FixedText>("status")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_STATUS, get<FixedText>("type")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );
    rBar.InsertItem( ITEMID_VALUE, get<FixedText>("value")->GetText(), 0,  HIB_LEFT | HIB_VCENTER );

    long aTabs[] = {4,0,12,12,12};

    aTabs[2] += aTabs[1] + rBar.GetTextWidth(rBar.GetItemText(1));
    aTabs[3] += aTabs[2] + rBar.GetTextWidth(rBar.GetItemText(2));
    aTabs[4] += aTabs[3] + rBar.GetTextWidth(rBar.GetItemText(3));

    pPrefBox->SetTabs(aTabs, MAP_PIXEL);

}

CuiAboutConfigTabPage::~CuiAboutConfigTabPage()
{
    delete pPrefBox;
}

SfxTabPage* CuiAboutConfigTabPage::Create( Window* pParent, const SfxItemSet& rItemSet )
{
    return ( new CuiAboutConfigTabPage( pParent, rItemSet) );
}

sal_Bool CuiAboutConfigTabPage::FillItemSet()
{
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
