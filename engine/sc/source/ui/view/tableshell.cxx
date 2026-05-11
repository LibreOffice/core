/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <scitems.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <vcl/EnumContext.hxx>

#include <sc.hrc>
#include <tableshell.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <dbdata.hxx>
#include <dbdocfun.hxx>
#include <subtotalparam.hxx>

#define ShellClass_ScTableShell
#include <scslots.hxx>

SFX_IMPL_INTERFACE(ScTableShell, SfxShell)

void ScTableShell::InitInterface_Impl() {}

ScTableShell::ScTableShell(ScTabViewShell* pViewShell)
    : SfxShell(pViewShell)
    , m_pViewShell(pViewShell)
{
    SetPool(&m_pViewShell->GetPool());
    ScViewData& rViewData = m_pViewShell->GetViewData();
    SfxUndoManager* pUndoManager = rViewData.GetSfxDocShell()->GetUndoManager();
    SetUndoManager(pUndoManager);
    if (!rViewData.GetDocument().IsUndoEnabled())
    {
        pUndoManager->SetMaxUndoActionCount(0);
    }
    SetName("Table");
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Table));
}

ScTableShell::~ScTableShell() = default;

const ScDBData* ScTableShell::GetTableDBDataAtCursor()
{
    ScViewData& rViewData = m_pViewShell->GetViewData();
    const ScAddress aAddr = rViewData.GetCurPos();
    return rViewData.GetDocument().GetTableDBAtCursor(aAddr.Col(), aAddr.Row(), aAddr.Tab(),
                                                      ScDBDataPortion::AREA);
}

void ScTableShell::ExecuteDatabaseSettings(const SfxRequest& rReq)
{
    const SfxItemSet* pSet = rReq.GetArgs();
    ScViewData& rViewData = m_pViewShell->GetViewData();
    SfxBindings& rBindings = rViewData.GetBindings();

    switch (rReq.GetSlot())
    {
        case SID_DATABASE_SETTINGS:
        {
            if (!pSet)
            {
                SAL_WARN("sc", "No arguments provided for ExecuteDatabaseSettings");
                break;
            }
            const SfxPoolItem* pItem = nullptr;
            if (pSet->GetItemState(SCITEM_DATABASE_SETTING, true, &pItem) != SfxItemState::SET)
                break;
            const auto* pDBItem = dynamic_cast<const ScDatabaseSettingItem*>(pItem);
            if (!pDBItem)
                break;
            const ScDBData* pDBData = GetTableDBDataAtCursor();
            if (!pDBData)
                break;

            ScDBData aNewDBData(*pDBData);
            aNewDBData.SetAutoFilter(pDBItem->HasShowFilters());
            aNewDBData.SetHeader(pDBItem->HasHeaderRow());
            aNewDBData.SetTotals(pDBItem->HasTotalRow());

            ScTableStyleParam aNewParam(*pDBData->GetTableStyleInfo());
            aNewParam.mbRowStripes = pDBItem->HasStripedRows();
            aNewParam.mbColumnStripes = pDBItem->HasStripedCols();
            aNewParam.mbFirstColumn = pDBItem->HasFirstCol();
            aNewParam.mbLastColumn = pDBItem->HasLastCol();
            aNewParam.maStyleID = pDBItem->GetStyleID();
            aNewDBData.SetTableStyleInfo(aNewParam);

            ScDBDocFunc aFunc(*rViewData.GetDocShell());
            if (pDBData->HasTotals() == aNewDBData.HasTotals())
            {
                aFunc.ModifyDBData(aNewDBData);
                break;
            }

            // Total Row toggled — add or remove.
            ScSubTotalParam aSubTotalParam;
            aNewDBData.GetSubTotalParam(aSubTotalParam);
            const bool bRemove = !aNewDBData.HasTotals();
            if (bRemove)
            {
                // store current subtotal settings before removing total row
                pDBData->CreateTotalRowParam(aSubTotalParam);
                aNewDBData.SetSubTotalParam(aSubTotalParam);
            }
            aSubTotalParam.bRemoveOnly = bRemove;
            aSubTotalParam.bReplace = bRemove;
            aFunc.DoTableSubTotals(aNewDBData.GetTab(), aNewDBData, aSubTotalParam, true, false);
        }
        break;
        case SID_REMOVE_CALCTABLE:
            m_pViewShell->DeleteCalcTable();
            break;
    }

    // TODO: when DoTableSubTotals refuses the toggle (tear-risk + blocked band), the
    // underlying ScDBData state is unchanged, so this Invalidate produces an
    // ScDatabaseSettingItem equal to the cached one. SfxBindings suppresses the
    // resulting NotifyItemUpdate, no commandstatechanged echo is sent, and the Total
    // Row checkbox in both the sidebar and the notebookbar stays visually flipped.
    rBindings.Invalidate(SID_DATABASE_SETTINGS);
}

void ScTableShell::GetDatabaseSettings(SfxItemSet& rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SCITEM_DATABASE_SETTING:
            {
                if (const ScDBData* pDBData = GetTableDBDataAtCursor())
                {
                    const ScTableStyleParam* pParam = pDBData->GetTableStyleInfo();
                    rSet.Put(ScDatabaseSettingItem(pDBData->HasHeader(), pDBData->HasTotals(),
                                                   pParam->mbFirstColumn, pParam->mbLastColumn,
                                                   pParam->mbRowStripes, pParam->mbColumnStripes,
                                                   pDBData->HasAutoFilter(), pParam->maStyleID));
                }
            }
            break;
            case SID_REMOVE_CALCTABLE:
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
