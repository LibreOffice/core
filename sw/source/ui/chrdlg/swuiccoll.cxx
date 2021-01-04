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

#include <sfx2/styfitem.hxx>

#include <ccoll.hxx>
#include <fmtcol.hxx>
#include <hintids.hxx>
#include <docsh.hxx>

#include <swuiccoll.hxx>

const sal_uInt16 SwCondCollPage::m_aPageRg[] = {
    FN_COND_COLL, FN_COND_COLL,
    0
};

SwCondCollPage::SwCondCollPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/conditionpage.ui", "ConditionPage", &rSet)
    , m_rSh(::GetActiveView()->GetWrtShell())
    , m_pCmds(SwCondCollItem::GetCmds())
    , m_pFormat(nullptr)
    , m_xTbLinks(m_xBuilder->weld_tree_view("links"))
    , m_xStyleLB(m_xBuilder->weld_tree_view("styles"))
    , m_xFilterLB(m_xBuilder->weld_combo_box("filter"))
    , m_xRemovePB(m_xBuilder->weld_button("remove"))
    , m_xAssignPB(m_xBuilder->weld_button("apply"))
{
    m_xStyleLB->make_sorted();
    const auto nHeightRequest = m_xStyleLB->get_height_rows(12);
    m_xStyleLB->set_size_request(-1, nHeightRequest);
    m_xTbLinks->set_size_request(-1, nHeightRequest);
    std::vector<int> aWidths;
    aWidths.push_back(m_xTbLinks->get_approximate_digit_width() * 40);
    m_xTbLinks->set_column_fixed_widths(aWidths);

    const sal_Int32 nStrCount = m_xFilterLB->get_count();
    for (sal_Int32 i = 0; i < nStrCount; ++i)
        m_aStrArr.push_back(m_xFilterLB->get_text(i));
    m_xFilterLB->clear();

    SetExchangeSupport();

    // Install handlers
    m_xTbLinks->connect_row_activated(LINK(this, SwCondCollPage, AssignRemoveTreeListBoxHdl));
    m_xStyleLB->connect_row_activated(LINK(this, SwCondCollPage, AssignRemoveTreeListBoxHdl));
    m_xRemovePB->connect_clicked(LINK(this, SwCondCollPage, AssignRemoveClickHdl));
    m_xAssignPB->connect_clicked(LINK(this, SwCondCollPage, AssignRemoveClickHdl));
    m_xTbLinks->connect_changed(LINK(this, SwCondCollPage, SelectTreeListBoxHdl));
    m_xStyleLB->connect_changed(LINK(this, SwCondCollPage, SelectTreeListBoxHdl));
    m_xFilterLB->connect_changed(LINK(this, SwCondCollPage, SelectListBoxHdl));

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
}

DeactivateRC SwCondCollPage::DeactivatePage(SfxItemSet * _pSet)
{
    if( _pSet )
        FillItemSet(_pSet);

    return DeactivateRC::LeavePage;
}

std::unique_ptr<SfxTabPage> SwCondCollPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet)
{
    return std::make_unique<SwCondCollPage>(pPage, pController, *rSet);
}

bool SwCondCollPage::FillItemSet(SfxItemSet *rSet)
{
    SwCondCollItem aCondItem;
    for (size_t i = 0; i < m_aStrArr.size(); ++i)
    {
        const OUString sEntry = m_xTbLinks->get_text(i, 1);
        aCondItem.SetStyle( &sEntry, i);
    }
    rSet->Put(aCondItem);
    return true;
}

void SwCondCollPage::Reset(const SfxItemSet *)
{
    m_xTbLinks->clear();

    SfxStyleSheetBasePool* pPool = m_rSh.GetView().GetDocShell()->GetStyleSheetPool();
    m_xStyleLB->clear();
    const SfxStyleSheetBase* pBase = pPool->First(SfxStyleFamily::Para);
    while (pBase)
    {
        if (!m_pFormat || pBase->GetName() != m_pFormat->GetName())
            m_xStyleLB->append_text(pBase->GetName());
        pBase = pPool->Next();
    }
    m_xStyleLB->select(0);

    for (size_t n = 0; n < m_aStrArr.size(); ++n)
    {
        m_xTbLinks->append_text(m_aStrArr[n]);

        const SwCollCondition* pCond = nullptr;
        if( m_pFormat && RES_CONDTXTFMTCOLL == m_pFormat->Which() &&
            nullptr != ( pCond = static_cast<SwConditionTextFormatColl*>(m_pFormat)->
            HasCondition( SwCollCondition( nullptr, m_pCmds[n].nCnd, m_pCmds[n].nSubCond ) ) )
            && pCond->GetTextFormatColl() )
        {
            m_xTbLinks->set_text(n, pCond->GetTextFormatColl()->GetName(), 1);
        }

        if (0 == n)
        {
            m_xTbLinks->select(0);
            SelectTreeListBoxHdl(*m_xTbLinks);
        }
    }
}

IMPL_LINK(SwCondCollPage, AssignRemoveClickHdl, weld::Button&, rBtn, void)
{
    AssignRemove(&rBtn);
}

IMPL_LINK(SwCondCollPage, AssignRemoveTreeListBoxHdl, weld::TreeView&, rBtn, bool)
{
    AssignRemove(&rBtn);
    return true;
}

void SwCondCollPage::AssignRemove(const weld::Widget* pBtn)
{
    int nPos = m_xTbLinks->get_selected_index();
    if (nPos == -1)
    {
        return;
    }

    const bool bAssEnabled = pBtn != m_xRemovePB.get() && m_xAssignPB->get_sensitive();
    m_xAssignPB->set_sensitive(!bAssEnabled);
    m_xRemovePB->set_sensitive(bAssEnabled);
    if (bAssEnabled)
        m_xTbLinks->set_text(nPos, m_xStyleLB->get_selected_text(), 1);
    else
        m_xTbLinks->set_text(nPos, "", 1);
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
        const SfxStyleSheetBase* pBase = pPool->First(SfxStyleFamily::Para, nSearchFlags);

        bool bEmpty = true;
        while (pBase)
        {
            if (!m_pFormat || pBase->GetName() != m_pFormat->GetName())
            {
                m_xStyleLB->append_text(pBase->GetName());
                bEmpty = false;
            }
            pBase = pPool->Next();
        }
        m_xStyleLB->select(bEmpty ? -1 : 0);
        SelectHdl(m_xStyleLB.get());
    }
    else
    {
        int nSelected = m_xTbLinks->get_selected_index();
        const OUString sTbEntry = nSelected != -1
            ? m_xTbLinks->get_text(nSelected, 1)
            : OUString();
        const OUString sStyle = m_xStyleLB->get_selected_text();

        m_xAssignPB->set_sensitive(sStyle != sTbEntry);

        if (pBox != m_xStyleLB.get())
            m_xRemovePB->set_sensitive(!sTbEntry.isEmpty());
    }
}

void SwCondCollPage::SetCollection(SwFormat* pFormat)
{
    m_pFormat = pFormat;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
