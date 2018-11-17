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

#include <memory>
#include <cmdid.h>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <globals.hrc>

#include <sfx2/styfitem.hxx>

#include <uitool.hxx>
#include <ccoll.hxx>
#include <fmtcol.hxx>
#include <hintids.hxx>
#include <docsh.hxx>
#include <docstyle.hxx>
#include <hints.hxx>

#include <vcl/svapp.hxx>
#include <vcl/lstbox.hxx>

#include <swuiccoll.hxx>

const sal_uInt16 SwCondCollPage::m_aPageRg[] = {
    FN_COND_COLL, FN_COND_COLL,
    0
};

// Warning! This table is indicated directly in code (0, 1, ...)
static long nTabs[] =
    {
        0, 100
    };

SwCondCollPage::SwCondCollPage(TabPageParent pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/conditionpage.ui", "ConditionPage", &rSet)
    , m_rSh(::GetActiveView()->GetWrtShell())
    , m_pCmds(SwCondCollItem::GetCmds())
    , m_pFormat(nullptr)
    , m_bNewTemplate(false)
    , m_xConditionCB(m_xBuilder->weld_check_button("condstyle"))
    , m_xContextFT(m_xBuilder->weld_label("contextft"))
    , m_xUsedFT(m_xBuilder->weld_label("usedft"))
    , m_xTbLinks(m_xBuilder->weld_tree_view("links"))
    , m_xStyleFT(m_xBuilder->weld_label("styleft"))
    , m_xStyleLB(m_xBuilder->weld_tree_view("styles"))
    , m_xFilterLB(m_xBuilder->weld_combo_box("filter"))
    , m_xRemovePB(m_xBuilder->weld_button("remove"))
    , m_xAssignPB(m_xBuilder->weld_button("apply"))
{
    m_xStyleLB->make_sorted();
    const auto nHeightRequest = m_xStyleLB->get_height_rows(12);
    m_xStyleLB->set_size_request(-1, nHeightRequest);
    m_xTbLinks->set_size_request(-1, nHeightRequest);

    const sal_Int32 nStrCount = m_xFilterLB->get_count();
    for (sal_Int32 i = 0; i < nStrCount; ++i)
        m_aStrArr.push_back(m_xFilterLB->get_text(i));
    m_xFilterLB->clear();

    SetExchangeSupport();

    // Install handlers
    m_xConditionCB->connect_toggled(LINK(this, SwCondCollPage, OnOffHdl));
    m_xTbLinks->connect_row_activated(LINK(this, SwCondCollPage, AssignRemoveTreeListBoxHdl));
    m_xStyleLB->connect_row_activated(LINK(this, SwCondCollPage, AssignRemoveTreeListBoxHdl));
    m_xRemovePB->connect_clicked(LINK(this, SwCondCollPage, AssignRemoveClickHdl));
    m_xAssignPB->connect_clicked(LINK(this, SwCondCollPage, AssignRemoveClickHdl));
    m_xTbLinks->connect_changed(LINK(this, SwCondCollPage, SelectTreeListBoxHdl));
    m_xStyleLB->connect_changed(LINK(this, SwCondCollPage, SelectTreeListBoxHdl));
    m_xFilterLB->connect_changed(LINK(this, SwCondCollPage, SelectListBoxHdl));

#if 0
    // TODO
    m_xTbLinks->SetStyle(m_xTbLinks->GetStyle()|WB_HSCROLL|WB_CLIPCHILDREN);
    m_xTbLinks->SetTabs( SAL_N_ELEMENTS(nTabs), nTabs );
    m_xTbLinks->Resize();  // OS: Hack for the right selection
    m_xTbLinks->SetSpaceBetweenEntries( 0 );
#endif

    std::unique_ptr<SfxStyleFamilies> xFamilies(SW_MOD()->CreateStyleFamilies());
    size_t nCount = xFamilies->size();
    for (size_t j = 0; j < nCount; ++j)
    {
        const SfxStyleFamilyItem &rFamilyItem = xFamilies->at(j);
        if (SfxStyleFamily::Para == rFamilyItem.GetFamily())
        {
            const SfxStyleFilter& rFilterList = rFamilyItem.GetFilterList();
            for (size_t i = 0; i < rFilterList.size(); ++i)
                m_xFilterLB->append(OUString::number(int(rFilterList[i].nFlags)), rFilterList[i].aName);
            break;
        }
    }

    m_xFilterLB->set_active(1);
    m_xTbLinks->show();
}

SwCondCollPage::~SwCondCollPage()
{
    disposeOnce();
}

DeactivateRC SwCondCollPage::DeactivatePage(SfxItemSet * _pSet)
{
    if( _pSet )
        FillItemSet(_pSet);

    return DeactivateRC::LeavePage;
}

VclPtr<SfxTabPage> SwCondCollPage::Create(TabPageParent pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwCondCollPage>::Create(pParent, *rSet);
}

bool SwCondCollPage::FillItemSet(SfxItemSet *rSet)
{
    SwCondCollItem aCondItem;
    for (size_t i = 0; i < m_aStrArr.size(); ++i)
    {
//TODO        const OUString sEntry = m_xTbLinks->GetEntryText(i, 1);
        const OUString sEntry = m_xTbLinks->get_text(i);
        aCondItem.SetStyle( &sEntry, i);
    }
    rSet->Put(aCondItem);
    return true;
}

void SwCondCollPage::Reset(const SfxItemSet *)
{
    if (m_bNewTemplate)
        m_xConditionCB->set_sensitive(true);
    if (RES_CONDTXTFMTCOLL == m_pFormat->Which())
        m_xConditionCB->set_active(true);
    OnOffHdl(*m_xConditionCB);

    m_xTbLinks->clear();

    SfxStyleSheetBasePool* pPool = m_rSh.GetView().GetDocShell()->GetStyleSheetPool();
    pPool->SetSearchMask(SfxStyleFamily::Para);
    m_xStyleLB->clear();
    const SfxStyleSheetBase* pBase = pPool->First();
    while (pBase)
    {
        if (!m_pFormat || pBase->GetName() != m_pFormat->GetName())
            m_xStyleLB->append_text(pBase->GetName());
        pBase = pPool->Next();
    }
    m_xStyleLB->select(0);

    for (size_t n = 0; n < m_aStrArr.size(); ++n)
    {
        OUString aEntry( m_aStrArr[n] + "\t" );

        const SwCollCondition* pCond = nullptr;
        if( m_pFormat && RES_CONDTXTFMTCOLL == m_pFormat->Which() &&
            nullptr != ( pCond = static_cast<SwConditionTextFormatColl*>(m_pFormat)->
            HasCondition( SwCollCondition( nullptr, m_pCmds[n].nCnd, m_pCmds[n].nSubCond ) ) )
            && pCond->GetTextFormatColl() )
        {
            aEntry += pCond->GetTextFormatColl()->GetName();
        }

//TODO        m_xTbLinks->InsertEntryToColumn( aEntry, n );
        m_xTbLinks->append_text(aEntry);
        if (0 == n)
            m_xTbLinks->select(0);
    }

}

IMPL_LINK(SwCondCollPage, OnOffHdl, weld::ToggleButton&, rBox, void)
{
    const bool bEnable = rBox.get_active();
    m_xContextFT->set_sensitive(bEnable);
    m_xUsedFT->set_sensitive(bEnable);
    m_xTbLinks->set_sensitive(bEnable);
    m_xStyleFT->set_sensitive(bEnable);
    m_xStyleLB->set_sensitive(bEnable);
    m_xFilterLB->set_sensitive(bEnable);
    m_xRemovePB->set_sensitive(bEnable);
    m_xAssignPB->set_sensitive(bEnable);
    if (bEnable)
        SelectHdl(nullptr);
}

IMPL_LINK(SwCondCollPage, AssignRemoveClickHdl, weld::Button&, rBtn, void)
{
    AssignRemove(&rBtn);
}

IMPL_LINK(SwCondCollPage, AssignRemoveTreeListBoxHdl, weld::TreeView&, rBtn, void)
{
    AssignRemove(&rBtn);
}

void SwCondCollPage::AssignRemove(const weld::Widget* pBtn)
{
    int nPos = m_xTbLinks->get_selected_index();
    if (nPos == -1)
    {
        return;
    }

    OUString sSel = m_aStrArr[nPos] + "\t";

    const bool bAssEnabled = pBtn != m_xRemovePB.get() && m_xAssignPB->get_sensitive();
    m_xAssignPB->set_sensitive( !bAssEnabled );
    m_xRemovePB->set_sensitive(  bAssEnabled );
    if ( bAssEnabled )
        sSel += m_xStyleLB->get_selected_text();

//    pE = m_xTbLinks->InsertEntryToColumn(sSel, nPos);
    m_xTbLinks->set_text(nPos, sSel);
}

IMPL_LINK(SwCondCollPage, SelectTreeListBoxHdl, weld::TreeView&, rBox, void)
{
    SelectHdl(&rBox);
}

IMPL_LINK(SwCondCollPage, SelectListBoxHdl, weld::ComboBox&, rBox, void)
{
    SelectHdl(&rBox);
}

void SwCondCollPage::SelectHdl(const weld::Widget* pBox)
{
    if (pBox == m_xFilterLB.get())
    {
        m_xStyleLB->clear();
        const sal_Int32 nSelPos = m_xFilterLB->get_active();
        const SfxStyleSearchBits nSearchFlags = static_cast<SfxStyleSearchBits>(m_xFilterLB->get_id(nSelPos).toInt32());
        SfxStyleSheetBasePool* pPool = m_rSh.GetView().GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask(SfxStyleFamily::Para, nSearchFlags);
        const SfxStyleSheetBase* pBase = pPool->First();

        while( pBase )
        {
            if (!m_pFormat || pBase->GetName() != m_pFormat->GetName())
                m_xStyleLB->append_text(pBase->GetName());
            pBase = pPool->Next();
        }
        m_xStyleLB->select(0);
        SelectHdl(m_xStyleLB.get());

    }
    else
    {
        std::unique_ptr<weld::TreeIter> xIter = m_xTbLinks->make_iterator();
        bool bValidIter = m_xTbLinks->get_selected(xIter.get());
        const OUString sTbEntry = bValidIter
            ? m_xTbLinks->get_text(*xIter).getToken(1, '\t')
            : OUString();
        const OUString sStyle = m_xStyleLB->get_selected_text();

        m_xAssignPB->set_sensitive(sStyle != sTbEntry && m_xConditionCB->get_active());

        if (pBox != m_xStyleLB.get())
            m_xRemovePB->set_sensitive(m_xConditionCB->get_active() && !sTbEntry.isEmpty());
    }
}

void SwCondCollPage::SetCollection(SwFormat* pFormat, bool bNew)
{
    m_pFormat = pFormat;
    m_bNewTemplate = bNew;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
