/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

const ScDBData* ScTableShell::GetDBDataAtCursor()
{
    ScViewData& rViewData = m_pViewShell->GetViewData();
    const ScAddress aAddr = rViewData.GetCurPos();
    return rViewData.GetDocument().GetDBAtCursor(aAddr.Col(), aAddr.Row(), aAddr.Tab(),
                                                 ScDBDataPortion::AREA);
}

void ScTableShell::ExecuteDatabaseSettings(SfxRequest& rReq)
{
    const SfxItemSet* pSet = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    ScViewData& rViewData = m_pViewShell->GetViewData();
    SfxBindings& rBindings = rViewData.GetBindings();

    switch (nSlot)
    {
        case SID_DATABASE_SETTINGS:
        {
            const SfxPoolItem* pItem;
            SfxItemState eItemState = pSet->GetItemState(SCITEM_DATABASE_SETTING, true, &pItem);
            if (eItemState == SfxItemState::SET
                && dynamic_cast<const ScDatabaseSettingItem*>(pItem))
            {
                const ScDatabaseSettingItem* pDBItem
                    = static_cast<const ScDatabaseSettingItem*>(pItem);
                const ScDBData* pDBData = GetDBDataAtCursor();
                if (pDBData)
                {
                    ScDBData aNewDBData(*pDBData);
                    aNewDBData.SetAutoFilter(pDBItem->HasShowFilters());
                    aNewDBData.SetHeader(pDBItem->HasHeaderRow());
                    aNewDBData.SetTotals(pDBItem->HasTotalRow());

                    ScTableStyleParam aNewParam(*pDBData->GetTableStyleInfo());
                    aNewParam.mbRowStripes = pDBItem->HasStripedRows();
                    aNewParam.mbColumnStripes = pDBItem->HasStripedCols();
                    aNewParam.mbFirstColumn = pDBItem->HasFirstCol();
                    aNewParam.mbLastColumn = pDBItem->HasLastCol();
                    aNewDBData.SetTableStyleInfo(aNewParam);

                    ScDBDocFunc aFunc(*rViewData.GetDocShell());
                    // Set new area if size changed
                    if (pDBData->HasTotals() != aNewDBData.HasTotals())
                    {
                        // Subtotals
                        ScSubTotalParam aSubTotalParam;
                        aNewDBData.GetSubTotalParam(aSubTotalParam);
                        aSubTotalParam.bHasHeader = aNewDBData.HasHeader();

                        if (!aNewDBData.HasTotals())
                        {
                            // store current subtotal settings
                            pDBData->CreateSubTotalParam(aSubTotalParam);
                            aNewDBData.SetSubTotalParam(aSubTotalParam);
                            // remove total row
                            aSubTotalParam.bRemoveOnly = true;
                            aSubTotalParam.bReplace = true;
                            aFunc.DoTableSubTotals(aNewDBData.GetTab(), aNewDBData, aSubTotalParam,
                                                   true, true);
                        }
                        else
                        {
                            // add/replace total row
                            aSubTotalParam.bRemoveOnly = false;
                            aSubTotalParam.bReplace = false;
                            aFunc.DoTableSubTotals(aNewDBData.GetTab(), aNewDBData, aSubTotalParam,
                                                   true, true);
                        }
                    }
                    else
                    {
                        aFunc.ModifyDBData(aNewDBData);
                    }
                }
            }
            break;
        }
    }

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
                const ScDBData* pDBData = GetDBDataAtCursor();
                if (pDBData)
                {
                    const ScTableStyleParam* pParam = pDBData->GetTableStyleInfo();
                    if (pParam)
                    {
                        rSet.Put(ScDatabaseSettingItem(
                            pDBData->HasHeader(), pDBData->HasTotals(), pParam->mbFirstColumn,
                            pParam->mbLastColumn, pParam->mbRowStripes, pParam->mbColumnStripes,
                            pDBData->HasAutoFilter()));
                    }
                    else
                    {
                        rSet.Put(ScDatabaseSettingItem(pDBData->HasHeader(), pDBData->HasTotals(),
                                                       false, false, false, false,
                                                       pDBData->HasAutoFilter()));
                    }
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
