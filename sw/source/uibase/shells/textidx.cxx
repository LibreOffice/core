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

#include <hintids.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/eitem.hxx>
#include <editeng/sizeitem.hxx>
#include <osl/diagnose.h>
#include <fmtfsize.hxx>
#include <fldbas.hxx>
#include <uiitems.hxx>
#include <viewopt.hxx>
#include <cmdid.h>
#include <view.hxx>
#include <wrtsh.hxx>
#include <textsh.hxx>
#include <idxmrk.hxx>
#include <toxmgr.hxx>
#include <swabstdlg.hxx>
#include <strings.hrc>
#include <svl/whiter.hxx>

#include <ndtxt.hxx>
#include <fmtfld.hxx>
#include <IDocumentFieldsAccess.hxx>

void SwTextShell::ExecIdx(SfxRequest const &rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = nullptr;
    const sal_uInt16 nSlot = rReq.GetSlot();
    if (pArgs)
        pArgs->GetItemState(nSlot, false, &pItem);

    SfxViewFrame& rVFrame = GetView().GetViewFrame();

    switch (nSlot)
    {
        case FN_EDIT_AUTH_ENTRY_DLG:
        {
            SwWrtShell& rShell = GetShell();

            const bool bWasViewLocked = rShell.IsViewLocked();
            rShell.LockView(true);

            if (const SwField* const pCurrentField = rShell.GetCurField();
                !rShell.HasReadonlySel() && pCurrentField != nullptr
                && pCurrentField->GetTyp()->Which() == SwFieldIds::TableOfAuthorities)
            {
                // Since the cursor is on a bibliography mark (e.g. "[1]"), open the edit dialog as usual
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<VclAbstractDialog> pDlg(
                    pFact->CreateSwAutoMarkDialog(GetView().GetFrameWeld(), rShell));
                pDlg->Execute();
            }
            else if (const SwTOXBase* const pCurrentTOX = rShell.GetCurTOX();
                     pCurrentTOX != nullptr && pCurrentTOX->GetType() == TOX_AUTHORITIES
                     && (rShell.GetCursor()->GetPoint()->GetNode()
                            .FindSectionNode()->GetSection().GetType()
                         == SectionType::ToxContent))
            {
                // Since the cursor is in the bibliography table, find the first mark that would match the given row
                const SwNode* const pTableRowNode = &rShell.GetCursor()->GetPoint()->GetNode();
                const OUString& rTableRowText
                    = static_cast<const SwTextNode*>(pTableRowNode)->GetText();

                const SwFieldType* pAuthField
                    = rShell.GetDoc()->getIDocumentFieldsAccess().GetFieldType(
                        SwFieldIds::TableOfAuthorities, OUString(), false);

                if (pAuthField)
                {
                    bool bMatchingMarkFound = false;
                    {
                        std::vector<SwFormatField*> vFields;
                        pAuthField->GatherFields(vFields);
                        for (auto pFormatField : vFields)
                        {
                            if (const SwField* pIteratedField = nullptr;
                                pFormatField != nullptr
                                && (pIteratedField = pFormatField->GetField()) != nullptr
                                && (pIteratedField->GetTyp()->Which()
                                    == SwFieldIds::TableOfAuthorities))
                            {
                                OUString sMarkText
                                    = static_cast<const SwAuthorityField*>(pIteratedField)
                                          ->GetAuthority(rShell.GetLayout(),
                                                         &pCurrentTOX->GetTOXForm());

                                if (sMarkText == rTableRowText)
                                {
                                    // Since the text generated from the mark would match the given row
                                    //  move cursor to it, set bMatchingMarkFound and break
                                    rShell.GotoFormatField(*pFormatField);
                                    bMatchingMarkFound = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (bMatchingMarkFound)
                    {
                        // Since matching mark has been found and cursor has been moved to it,
                        //  open the edit dialog
                        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                        ScopedVclPtr<VclAbstractDialog> pDlg(
                            pFact->CreateSwAutoMarkDialog(GetView().GetFrameWeld(), rShell));
                        pDlg->Execute();

                        // Refresh TOX
                        rShell.GetCursor_()->GetPoint()->Assign(*pTableRowNode);
                        rShell.UpdateTableOf(*pCurrentTOX);
                    }
                    else
                    {
                        // I think this ideally should be a pop-up warning, right?
                        SAL_WARN("sw", "No matching bibliography mark found. "
                                        "This feature is only guaranteed to work if the bibliography table is up to date.");
                    }
                }
            }

            if (!bWasViewLocked)
                rShell.LockView(false);
        }
        break;
        case FN_INSERT_AUTH_ENTRY_DLG:
        {
            // no BASIC support
            rVFrame.ToggleChildWindow(FN_INSERT_AUTH_ENTRY_DLG);
            Invalidate(rReq.GetSlot());
        }
        break;
        case FN_INSERT_IDX_ENTRY_DLG:
        {
            rVFrame.ToggleChildWindow(FN_INSERT_IDX_ENTRY_DLG);
            Invalidate(rReq.GetSlot());
        }
        break;
        case FN_EDIT_IDX_ENTRY_DLG:
        {
            SwTOXMgr aMgr(GetShellPtr());
            short nRet = RET_OK;
            if(aMgr.GetTOXMarkCount() > 1)
            {   // Several marks, which should it be?
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<VclAbstractDialog> pMultDlg(pFact->CreateMultiTOXMarkDlg(GetView().GetFrameWeld(), aMgr));
                nRet = pMultDlg->Execute();
            }
            if( nRet == RET_OK)
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateIndexMarkModalDlg(GetView().GetFrameWeld(), GetShell(), aMgr.GetCurTOXMark()));
                pDlg->Execute();
            }
            break;
        }
        case FN_IDX_MARK_TO_IDX:
        {
            GetShell().GotoTOXMarkBase();
            break;
        }
        case FN_INSERT_MULTI_TOX:
        {
            SfxItemSet aSet(SfxItemSet::makeFixedSfxItemSet<RES_FRM_SIZE, RES_FRM_SIZE,
                                                            RES_LR_SPACE, RES_LR_SPACE,
                                                            RES_BACKGROUND, RES_BACKGROUND,
                                                            RES_COL, RES_COL,
                                                            XATTR_FILL_FIRST, XATTR_FILL_LAST,
                                                            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                                                            FN_PARAM_TOX_TYPE, FN_PARAM_TOX_TYPE>(GetPool()));
            SwWrtShell& rSh = GetShell();
            SwRect aRect;
            rSh.CalcBoundRect(aRect, RndStdIds::FLY_AS_CHAR);

            tools::Long nWidth = aRect.Width();
            aSet.Put(SwFormatFrameSize(SwFrameSize::Variable, nWidth));
            // Height = width for a more consistent preview (analogous to edit range)
            aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
            const SwTOXBase* pCurTOX = nullptr;
            bool bGlobal = false;
            if(pItem)
            {
                pCurTOX = static_cast<const SwTOXBase*>(static_cast<const SwPtrItem*>(pItem)->GetValue());
                bGlobal = true;
            }
            else
                pCurTOX = rSh.GetCurTOX();
            if(pCurTOX)
            {
                const SfxItemSet* pSet = pCurTOX->GetAttrSet();
                if(pSet)
                    aSet.Put(*pSet);
            }
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            VclPtr<AbstractMultiTOXTabDialog> pDlg(pFact->CreateMultiTOXTabDialog(
                                                        GetView().GetFrameWeld(), aSet, rSh, const_cast<SwTOXBase*>(pCurTOX),
                                                        bGlobal));
            pDlg->StartExecuteAsync([pDlg](sal_Int32 /*nResult*/){
                pDlg->disposeOnce();
            });
        }
        break;
        case FN_REMOVE_CUR_TOX:
        {
            SwWrtShell& rSh = GetShell();
            const SwTOXBase* pBase = rSh.GetCurTOX();
            OSL_ENSURE(pBase, "no TOXBase to remove");
            if( pBase )
                rSh.DeleteTOX(*pBase, true);
        }
        break;
        default:
            OSL_ENSURE(false, "wrong dispatcher");
            return;
    }
}

void SwTextShell::GetIdxState(SfxItemSet &rSet)
{
    SwWrtShell& rSh = GetShell();
    SfxViewFrame& rVFrame = GetView().GetViewFrame();
    SwInsertIdxMarkWrapper *pIdxMrk = static_cast<SwInsertIdxMarkWrapper*>(
                        rVFrame.GetChildWindow(FN_INSERT_IDX_ENTRY_DLG));

    SfxChildWindow* pAuthMark = rVFrame.GetChildWindow(FN_INSERT_AUTH_ENTRY_DLG);

    const bool bHtmlMode = 0 != ::GetHtmlMode( GetView().GetDocShell() );
    const SwTOXBase* pBase = nullptr;
    if( bHtmlMode || nullptr != ( pBase = rSh.GetCurTOX()) )
    {
        if( pBase )
        {
            if(pBase->IsTOXBaseInReadonly())
            {
                rSet.DisableItem( FN_INSERT_MULTI_TOX );
            }
        }

        rSet.DisableItem( FN_EDIT_IDX_ENTRY_DLG );
        if(pBase == nullptr // tdf#72955: Hide the "Bibliography Entry" command if there is no TOX in the selection
            || pBase->GetType() != TOX_AUTHORITIES // or if it is not a bibliography table
            || (rSh.GetCursor()->GetPoint()->GetNode().FindSectionNode()->GetSection().GetType() != SectionType::ToxContent)) // or if it's the heading
            rSet.DisableItem(FN_EDIT_AUTH_ENTRY_DLG);

        if(!pIdxMrk)
            rSet.DisableItem( FN_INSERT_IDX_ENTRY_DLG );
        else
            rSet.Put(SfxBoolItem(FN_INSERT_IDX_ENTRY_DLG, true));

        if(!pAuthMark)
            rSet.DisableItem( FN_INSERT_AUTH_ENTRY_DLG );
        else
            rSet.Put(SfxBoolItem(FN_INSERT_AUTH_ENTRY_DLG, true));

        if (pBase)
        {
            SfxWhichIter aIter(rSet);
            if (aIter.FirstWhich() == FN_REMOVE_CUR_TOX)
            {
                const OUString sLabel = SwResId(STR_DELETEINDEX).replaceAll("%1", pBase->GetTypeName());
                rSet.Put(SfxStringItem(FN_REMOVE_CUR_TOX, sLabel));
            }
        }
    }
    else if ( rSh.CursorInsideInputField() )
    {
        rSet.DisableItem( FN_INSERT_IDX_ENTRY_DLG );
        rSet.DisableItem( FN_INSERT_AUTH_ENTRY_DLG );
        rSet.DisableItem( FN_EDIT_AUTH_ENTRY_DLG );
        rSet.DisableItem( FN_EDIT_IDX_ENTRY_DLG );
        rSet.DisableItem( FN_INSERT_MULTI_TOX );
        rSet.DisableItem( FN_REMOVE_CUR_TOX );
    }
    else
    {

        bool bEnableEdit = true;
        bool bInReadonly = rSh.HasReadonlySel();
        if(bInReadonly)
            bEnableEdit = false;
        else
        {
            SwTOXMarks aArr;
            rSh.GetCurTOXMarks( aArr );
            if( aArr.empty())
                bEnableEdit = false;
        }

        if(!bEnableEdit)
            rSet.DisableItem( FN_EDIT_IDX_ENTRY_DLG );

        if(bInReadonly)
        {
            rSet.DisableItem(FN_INSERT_IDX_ENTRY_DLG);
            rSet.DisableItem( FN_INSERT_MULTI_TOX );
        }
        else
            rSet.Put(SfxBoolItem(FN_INSERT_IDX_ENTRY_DLG,
                                    nullptr != pIdxMrk));

        SwField* pField = rSh.GetCurField();

        if(bInReadonly)
            rSet.DisableItem(FN_INSERT_AUTH_ENTRY_DLG);
        else
            rSet.Put(SfxBoolItem(FN_INSERT_AUTH_ENTRY_DLG, nullptr != pAuthMark));

        if( bInReadonly || !pField ||
            pField->GetTyp()->Which() != SwFieldIds::TableOfAuthorities)
            rSet.DisableItem(FN_EDIT_AUTH_ENTRY_DLG);
        rSet.DisableItem(FN_REMOVE_CUR_TOX);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
