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
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <editeng/sizeitem.hxx>
#include <fmtfsize.hxx>
#include <fldbas.hxx>
#include <uiitems.hxx>
#include "viewopt.hxx"
#include "cmdid.h"
#include "view.hxx"
#include "wrtsh.hxx"
#include "swundo.hxx"                   // for Undo-Ids
#include "textsh.hxx"
#include "idxmrk.hxx"
#include "cnttab.hxx"
#include "toxmgr.hxx"
#include "swabstdlg.hxx"
#include <index.hrc>
#include <globals.hrc>

void SwTextShell::ExecIdx(SfxRequest &rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    sal_uInt16 nSlot = rReq.GetSlot();
    if(pArgs)
       pArgs->GetItemState(nSlot, sal_False, &pItem );

    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    Window *pMDI = &pVFrame->GetWindow();

    switch( nSlot )
    {
        case FN_EDIT_AUTH_ENTRY_DLG :
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "Dialogdiet fail!");
            VclAbstractDialog* pDlg = pFact->CreateVclAbstractDialog( pMDI, GetShell(), DLG_EDIT_AUTHMARK);
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
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
            sal_uInt16 nRet = RET_OK;
            if(aMgr.GetTOXMarkCount() > 1)
            {   // Several marks, which should it be?
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialogdiet fail!");
                VclAbstractDialog* pMultDlg = pFact->CreateMultiTOXMarkDlg(pMDI, aMgr);
                OSL_ENSURE(pMultDlg, "Dialogdiet fail!");
                nRet = pMultDlg->Execute();
                delete pMultDlg;
            }
            if( nRet == RET_OK)
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialogdiet fail!");
                VclAbstractDialog* pDlg = pFact->CreateIndexMarkModalDlg(pMDI, GetShell(), aMgr.GetCurTOXMark());
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                pDlg->Execute();
                delete pDlg;
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
            SfxItemSet aSet(GetPool(),
                            RES_COL, RES_COL,
                            RES_BACKGROUND, RES_BACKGROUND,
                            RES_FRM_SIZE, RES_FRM_SIZE,
                            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                            RES_LR_SPACE, RES_LR_SPACE,
                            FN_PARAM_TOX_TYPE, FN_PARAM_TOX_TYPE,
                            0   );
            SwWrtShell& rSh = GetShell();
            SwRect aRect;
            rSh.CalcBoundRect(aRect, FLY_AS_CHAR);

            long nWidth = aRect.Width();
            aSet.Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth));
            // Height = width for a more consistent preview (analogous to edit range)
            aSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(nWidth, nWidth)));
            const SwTOXBase* pCurTOX = 0;
            sal_Bool bGlobal = sal_False;
            if(pItem)
            {
                pCurTOX = (const SwTOXBase* )((SwPtrItem*)pItem)->GetValue();
                bGlobal = sal_True;
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
            OSL_ENSURE(pFact, "Dialogdiet fail!");
            AbstractMultiTOXTabDialog* pDlg = pFact->CreateMultiTOXTabDialog(
                                                        pMDI, aSet, rSh, (SwTOXBase* )pCurTOX,
                                                        USHRT_MAX, bGlobal);
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
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
            OSL_ENSURE(!this, "wrong dispatcher");
            return;
    }
}

void SwTextShell::GetIdxState(SfxItemSet &rSet)
{
    SwWrtShell& rSh = GetShell();
    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    SwInsertIdxMarkWrapper *pIdxMrk = (SwInsertIdxMarkWrapper*)
                        pVFrame->GetChildWindow(FN_INSERT_IDX_ENTRY_DLG);

    SfxChildWindow* pAuthMark = pVFrame->GetChildWindow(FN_INSERT_AUTH_ENTRY_DLG);

    const bool bHtmlMode = 0 != ::GetHtmlMode( GetView().GetDocShell() );
    const SwTOXBase* pBase = 0;
    if( bHtmlMode || 0 != ( pBase = rSh.GetCurTOX()) )
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
            rSet.Put(SfxBoolItem(FN_INSERT_IDX_ENTRY_DLG, sal_True));

        if(!pAuthMark)
            rSet.DisableItem( FN_INSERT_AUTH_ENTRY_DLG );
        else
            rSet.Put(SfxBoolItem(FN_INSERT_AUTH_ENTRY_DLG, sal_True));

    }
    else
    {

        bool bEnableEdit = true;
        sal_Bool bInReadonly = rSh.HasReadonlySel();
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
                                    0 != pIdxMrk));


        SwField* pField = rSh.GetCurFld();

        if(bInReadonly)
            rSet.DisableItem(FN_INSERT_AUTH_ENTRY_DLG);
        else
            rSet.Put(SfxBoolItem(FN_INSERT_AUTH_ENTRY_DLG, 0 != pAuthMark));

        if( bInReadonly || !pField ||
            pField->GetTyp()->Which() != RES_AUTHORITY)
            rSet.DisableItem(FN_EDIT_AUTH_ENTRY_DLG);
        rSet.DisableItem(FN_REMOVE_CUR_TOX);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
