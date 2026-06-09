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
#include <editable.hxx>
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
    SetName(u"Table"_ustr);
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

bool ScTableShell::IsTableEditable(const ScDBData& rDBData) const
{
    ScDocument& rDoc = m_pViewShell->GetViewData().GetDocument();
    ScRange aRange;
    rDBData.GetArea(aRange);
    ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(
        rDoc, aRange.aStart.Tab(), aRange.aStart.Col(), aRange.aStart.Row(),
        aRange.aEnd.Col(), aRange.aEnd.Row());
    return aTester.IsEditable();
}

void ScTableShell::ExecuteDatabaseSettings(const SfxRequest& rReq)
{
    const SfxItemSet* pSet = rReq.GetArgs();
    ScViewData& rViewData = m_pViewShell->GetViewData();
    SfxBindings& rBindings = rViewData.GetBindings();

    const ScDBData* pDBData = GetTableDBDataAtCursor();
    if (pDBData && IsTableEditable(*pDBData))
    {
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
                const ScDatabaseSettingItem* pDBItem
                    = dynamic_cast<const ScDatabaseSettingItem*>(pItem);
                if (!pDBItem)
                    break;

                ScDBData aNewDBData(*pDBData);
                aNewDBData.SetAutoFilter(pDBItem->HasShowFilters());
                aNewDBData.SetHeader(pDBItem->HasHeaderRow());

                ScTableStyleParam aNewParam(*pDBData->GetTableStyleInfo());
                aNewParam.mbRowStripes = pDBItem->HasStripedRows();
                aNewParam.mbColumnStripes = pDBItem->HasStripedCols();
                aNewParam.mbFirstColumn = pDBItem->HasFirstCol();
                aNewParam.mbLastColumn = pDBItem->HasLastCol();
                aNewParam.maStyleID = pDBItem->GetStyleID();
                aNewDBData.SetTableStyleInfo(aNewParam);

                ScDBDocFunc aFunc(*rViewData.GetDocShell());
                aFunc.ModifyDBData(aNewDBData);
            }
            break;
            case SID_REMOVE_CALCTABLE:
                m_pViewShell->DeleteCalcTable();
                break;
            case SID_TABLE_TOTALROW:
            {
                // Desired value: SfxBoolItem in args if present, else toggle.
                bool bNewTotal = !pDBData->HasTotals();
                if (pSet)
                {
                    const SfxPoolItem* pItem = nullptr;
                    if (pSet->GetItemState(SID_TABLE_TOTALROW, true, &pItem) == SfxItemState::SET)
                    {
                        if (const SfxBoolItem* pBoolItem = dynamic_cast<const SfxBoolItem*>(pItem))
                            bNewTotal = pBoolItem->GetValue();
                    }
                }
                if (bNewTotal == pDBData->HasTotals())
                    break;

                ScDBData aNewDBData(*pDBData);
                aNewDBData.SetTotals(bNewTotal);

                ScSubTotalParam aSubTotalParam;
                aNewDBData.GetSubTotalParam(aSubTotalParam);
                const bool bRemove = !bNewTotal;
                if (bRemove)
                {
                    // store current subtotal settings before removing total row
                    pDBData->CreateTotalRowParam(aSubTotalParam);
                    aNewDBData.SetSubTotalParam(aSubTotalParam);
                }
                aSubTotalParam.bRemoveOnly = bRemove;
                aSubTotalParam.bReplace = bRemove;

                ScDBDocFunc aFunc(*rViewData.GetDocShell());
                aFunc.DoTableSubTotals(aNewDBData.GetTab(), aNewDBData, aSubTotalParam, true,
                                       false);
            }
            break;
        }
    }

    rBindings.Invalidate(SID_DATABASE_SETTINGS);
    rBindings.Invalidate(SID_TABLE_TOTALROW);
}

void ScTableShell::GetDatabaseSettings(SfxItemSet& rSet)
{
    const ScDBData* pDBData = GetTableDBDataAtCursor();
    const bool bProtected = pDBData && !IsTableEditable(*pDBData);

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SCITEM_DATABASE_SETTING:
            {
                if (pDBData && !bProtected)
                {
                    const ScTableStyleParam* pParam = pDBData->GetTableStyleInfo();
                    rSet.Put(ScDatabaseSettingItem(pDBData->HasHeader(), pParam->mbFirstColumn,
                                                   pParam->mbLastColumn, pParam->mbRowStripes,
                                                   pParam->mbColumnStripes,
                                                   pDBData->HasAutoFilter(), pParam->maStyleID));
                }
                else
                    rSet.DisableItem(nWhich);
            }
            break;
            case SID_REMOVE_CALCTABLE:
                if (bProtected)
                    rSet.DisableItem(nWhich);
                break;
            case SID_TABLE_TOTALROW:
            {
                if (!pDBData || bProtected)
                {
                    rSet.DisableItem(nWhich);
                    break;
                }
                // Disable if the next click (toggle direction) would be refused.
                if (pDBData->WouldTableTotalsBeRefused(!pDBData->HasTotals()))
                    rSet.DisableItem(nWhich);
                else
                    rSet.Put(SfxBoolItem(nWhich, pDBData->HasTotals()));
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
