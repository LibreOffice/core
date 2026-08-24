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
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/whiter.hxx>
#include <svl/stritem.hxx>
#include <vcl/EnumContext.hxx>

#include <sc.hrc>
#include <scmod.hxx>
#include <tableshell.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <dbdata.hxx>
#include <dbdocfun.hxx>
#include <reffact.hxx>
#include <editable.hxx>
#include <subtotalparam.hxx>
#include <tablestyle.hxx>
#include <scabstdlg.hxx>
#include <scresid.hxx>
#include <strings.hrc>

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
        rDoc, aRange.aStart.Tab(), aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
        aRange.aEnd.Row());
    return aTester.IsEditable();
}

void ScTableShell::ExecuteDatabaseSettings(const SfxRequest& rReq)
{
    const SfxItemSet* pSet = rReq.GetArgs();
    ScViewData& rViewData = m_pViewShell->GetViewData();
    SfxBindings& rBindings = rViewData.GetBindings();
    ScTabViewShell* pTabViewShell = rViewData.GetViewShell();
    ScModule* pScMod = ScModule::get();

    pTabViewShell->HideListBox();

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
            case SID_NEW_TABLE_STYLE:
            {
                // The dialog builds the style and hands it back; here we register it
                // with the document and apply it to the table at the cursor as one
                // step. The dialog runs asynchronously, as the online client requires.
                ScDocShell* pDocSh = rViewData.GetDocShell();
                ScDocument& rDoc = pDocSh->GetDocument();
                if (rDoc.GetTableStyles())
                {
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    VclPtr<AbstractScTableStyleDlg> pDlg(
                        pFact->CreateScTableStyleDlg(pTabViewShell->GetFrameWeld(), rDoc));
                    pDlg->StartExecuteAsync([pDlg, pDocSh](sal_Int32 nResult) {
                        if (nResult == RET_OK)
                        {
                            if (std::unique_ptr<ScTableStyle> pStyle = pDlg->TakeStyle())
                            {
                                ScDocument& rCbDoc = pDocSh->GetDocument();
                                if (ScTableStyles* pStyles = rCbDoc.GetTableStyles())
                                {
                                    const OUString aStyleName = pStyle->GetName();
                                    pStyles->AddTableStyle(std::move(pStyle));
                                    pDocSh->SetDocumentModified();

                                    // Re-resolve the view rather than capturing it: apply
                                    // the new style only if the same document is still
                                    // shown and a table is at the cursor.
                                    ScTabViewShell* pView = ScTabViewShell::GetActiveViewShell();
                                    if (pView && pView->GetViewData().GetDocShell() == pDocSh)
                                    {
                                        ScViewData& rCbData = pView->GetViewData();
                                        const ScAddress aPos = rCbData.GetCurPos();
                                        const ScDBData* pTableData = rCbDoc.GetTableDBAtCursor(
                                            aPos.Col(), aPos.Row(), aPos.Tab(),
                                            ScDBDataPortion::AREA);
                                        if (pTableData)
                                        {
                                            ScDBData aNewDBData(*pTableData);
                                            ScTableStyleParam aParam
                                                = pTableData->GetTableStyleInfo()
                                                      ? *pTableData->GetTableStyleInfo()
                                                      : ScTableStyleParam();
                                            aParam.maStyleID = aStyleName;
                                            aNewDBData.SetTableStyleInfo(aParam);
                                            ScDBDocFunc aFunc(*pDocSh);
                                            aFunc.ModifyDBData(aNewDBData);
                                        }
                                    }
                                }
                            }
                        }
                        pDlg->disposeOnce();
                    });
                }
                return;
            }
            case SID_DUPLICATE_TABLE_STYLE:
            {
                // Copying a style registers a new document-level style and leaves every
                // table as it is. The command is only reachable from the table context.
                // TODO: MSO offers copying a style from the Home gallery, with no
                // table at the cursor. Move this and SID_SET_DEFAULT_TABLE_STYLE to a
                // shell that is always active, as SID_INSERT_CALCTABLE is on
                // ScCellShell, once the gallery is offered outside the table context.
                if (!pSet)
                    break;
                const SfxPoolItem* pItem = nullptr;
                if (pSet->GetItemState(SID_DUPLICATE_TABLE_STYLE, true, &pItem)
                    != SfxItemState::SET)
                    break;
                const SfxStringItem* pStrItem = dynamic_cast<const SfxStringItem*>(pItem);
                if (!pStrItem)
                    break;

                ScDocShell* pDocSh = rViewData.GetDocShell();
                ScTableStyles* pStyles = pDocSh->GetDocument().GetTableStyles();
                if (!pStyles)
                    break;
                const ScTableStyle* pSource = pStyles->GetTableStyle(pStrItem->GetValue());
                if (!pSource)
                    break;

                const OUString aUIName
                    = ScResId(STR_TABLE_STYLE_COPY_OF).replaceFirst("%1", pSource->GetUIName());
                if (!pStyles->DuplicateTableStyle(pStrItem->GetValue(), aUIName).isEmpty())
                    pDocSh->SetDocumentModified();
            }
            break;
            case SID_CLEAR_TABLE_STYLE:
            {
                // Reset the table to the unstyled "None" entry, keeping the
                // other table options as they are.
                ScDBData aNewDBData(*pDBData);
                ScTableStyleParam aNewParam = pDBData->GetTableStyleInfo()
                                                  ? *pDBData->GetTableStyleInfo()
                                                  : ScTableStyleParam();
                aNewParam.maStyleID = u"None"_ustr;
                aNewDBData.SetTableStyleInfo(aNewParam);

                ScDBDocFunc aFunc(*rViewData.GetDocShell());
                aFunc.ModifyDBData(aNewDBData);
            }
            break;
            case SID_REMOVE_CALCTABLE:
                m_pViewShell->DeleteCalcTable();
                break;
            case SID_RESIZE_CALCTABLE:
            {
                sal_uInt16 nId = ScTableResizeWrapper::GetChildWindowId();
                SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = rViewFrm.GetChildWindow(nId);

                pScMod->SetRefDialog(nId, pWnd == nullptr);
            }
            break;
            case SID_RENAME_CALCTABLE:
                pTabViewShell->ExecuteRenameCalcTable();
                break;
            case SID_CONVERT_CALCTABLE_TO_RANGE:
                m_pViewShell->ConvertCalcTableToRange();
                break;
            case SID_SUMMARIZE_WITH_PIVOT:
                rViewData.GetDispatcher().Execute(SID_OPENDLG_PIVOTTABLE);
                break;
            case SID_REMOVE_DUPLICATES:
                rViewData.GetDispatcher().Execute(FID_HANDLEDUPLICATERECORDS);
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
            case SID_SET_DEFAULT_TABLE_STYLE:
            {
                if (!pSet)
                    break;
                const SfxPoolItem* pItem = nullptr;
                if (pSet->GetItemState(SID_SET_DEFAULT_TABLE_STYLE, true, &pItem)
                    != SfxItemState::SET)
                    break;
                const SfxStringItem* pStrItem = dynamic_cast<const SfxStringItem*>(pItem);
                if (!pStrItem)
                    break;

                ScDocShell* pDocSh = rViewData.GetDocShell();
                if (ScTableStyles* pStyles = pDocSh->GetDocument().GetTableStyles())
                {
                    pStyles->SetDefaultStyleName(pStrItem->GetValue());
                    pDocSh->SetDocumentModified();
                }
            }
            break;
        }
    }

    rBindings.Invalidate(SID_DATABASE_SETTINGS);
    rBindings.Invalidate(SID_TABLE_TOTALROW);
    rBindings.Invalidate(SID_SET_DEFAULT_TABLE_STYLE);
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
            case SID_RESIZE_CALCTABLE:
            case SID_RENAME_CALCTABLE:
            case SID_CONVERT_CALCTABLE_TO_RANGE:
                if (bProtected)
                    rSet.DisableItem(nWhich);
                break;
            case SID_SUMMARIZE_WITH_PIVOT:
            {
                // Keep in sync with SID_OPENDLG_PIVOTTABLE in ScCellShell::GetDBState.
                ScViewData& rViewData = m_pViewShell->GetViewData();
                if (!pDBData || bProtected || rViewData.GetDocShell()->IsReadOnly()
                    || rViewData.GetDocument().GetChangeTrack() != nullptr
                    || rViewData.IsMultiMarked())
                    rSet.DisableItem(nWhich);
            }
            break;
            case SID_REMOVE_DUPLICATES:
            {
                // Keep in sync with FID_HANDLEDUPLICATERECORDS in ScTabViewShell::GetState.
                ScViewData& rViewData = m_pViewShell->GetViewData();
                ScRange aSel;
                if (!pDBData || bProtected || rViewData.GetSimpleArea(aSel) != SC_MARK_SIMPLE)
                {
                    rSet.DisableItem(nWhich);
                    break;
                }
                for (const ScDBData* pDB : rViewData.GetDocument().GetAllNamedDBsInArea(aSel))
                {
                    ScRange aArea;
                    pDB->GetArea(aArea);
                    if (!aArea.Contains(aSel))
                    {
                        rSet.DisableItem(nWhich);
                        break;
                    }
                }
            }
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
            case SID_SET_DEFAULT_TABLE_STYLE:
            {
                ScDocument& rDoc = m_pViewShell->GetViewData().GetDocument();
                if (const ScTableStyles* pStyles = rDoc.GetTableStyles())
                    rSet.Put(SfxStringItem(nWhich, pStyles->GetDefaultStyleName()));
            }
            break;
            case SID_NEW_TABLE_STYLE:
            case SID_CLEAR_TABLE_STYLE:
            case SID_DUPLICATE_TABLE_STYLE:
            {
                // Every one of these is only offered from the table context, so all
                // three need an editable table to be at the cursor.
                ScDocument& rDoc = m_pViewShell->GetViewData().GetDocument();
                if (!pDBData || bProtected || !rDoc.GetTableStyles())
                    rSet.DisableItem(nWhich);
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
