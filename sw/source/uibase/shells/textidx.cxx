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
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <editeng/sizeitem.hxx>
#include <fmtfsize.hxx>
#include <fldbas.hxx>
#include <uiitems.hxx>
#include <viewopt.hxx>
#include <cmdid.h>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>
#include <textsh.hxx>
#include <idxmrk.hxx>
#include <cnttab.hxx>
#include <toxmgr.hxx>
#include <swabstdlg.hxx>
#include <globals.hrc>
#include <memory>

void SwTextShell::ExecIdx(SfxRequest const &rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = nullptr;
    const sal_uInt16 nSlot = rReq.GetSlot();
    if(pArgs)
       pArgs->GetItemState(nSlot, false, &pItem );

    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    vcl::Window *pMDI = &pVFrame->GetWindow();

    switch( nSlot )
    {
        case FN_EDIT_AUTH_ENTRY_DLG :
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSwAutoMarkDialog(GetView().GetFrameWeld(), GetShell()));
            pDlg->Execute();
        }
        break;
        case FN_INSERT_AUTH_ENTRY_DLG:
        {
            // no BASIC support
            pVFrame->ToggleChildWindow(FN_INSERT_AUTH_ENTRY_DLG);
            Invalidate(rReq.GetSlot());
        }
        break;
        case FN_INSERT_IDX_ENTRY_DLG:
        {
            pVFrame->ToggleChildWindow(FN_INSERT_IDX_ENTRY_DLG);
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
            SfxItemSet aSet(
                GetPool(),
                svl::Items<
                    RES_FRM_SIZE, RES_FRM_SIZE,
                    RES_LR_SPACE, RES_LR_SPACE,
                    RES_BACKGROUND, RES_BACKGROUND,
                    RES_COL, RES_COL,
                    XATTR_FILL_FIRST, XATTR_FILL_LAST,
                    SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                    FN_PARAM_TOX_TYPE, FN_PARAM_TOX_TYPE>{});
            SwWrtShell& rSh = GetShell();
            SwRect aRect;
            rSh.CalcBoundRect(aRect, RndStdIds::FLY_AS_CHAR);

            long nWidth = aRect.Width();
            aSet.Put(SwFormatFrameSize(ATT_VAR_SIZE, nWidth));
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
                                                        pMDI, aSet, rSh, const_cast<SwTOXBase*>(pCurTOX),
                                                        bGlobal));
            pDlg->StartExecuteAsync(nullptr);
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
    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    SwInsertIdxMarkWrapper *pIdxMrk = static_cast<SwInsertIdxMarkWrapper*>(
                        pVFrame->GetChildWindow(FN_INSERT_IDX_ENTRY_DLG));

    SfxChildWindow* pAuthMark = pVFrame->GetChildWindow(FN_INSERT_AUTH_ENTRY_DLG);

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
        rSet.DisableItem( FN_EDIT_AUTH_ENTRY_DLG );

        if(!pIdxMrk)
            rSet.DisableItem( FN_INSERT_IDX_ENTRY_DLG );
        else
            rSet.Put(SfxBoolItem(FN_INSERT_IDX_ENTRY_DLG, true));

        if(!pAuthMark)
            rSet.DisableItem( FN_INSERT_AUTH_ENTRY_DLG );
        else
            rSet.Put(SfxBoolItem(FN_INSERT_AUTH_ENTRY_DLG, true));

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
        if( rSh.HasSelection() || bInReadonly)
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
