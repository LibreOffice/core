/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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
#include "swundo.hxx"                   // fuer Undo-Ids
#include "textsh.hxx"
#include "idxmrk.hxx"
#include "cnttab.hxx"
#include "toxmgr.hxx"
#include "swabstdlg.hxx"
#include <index.hrc>
#include <globals.hrc>

// STATIC DATA -----------------------------------------------------------
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
            {   // Mehrere Marken, welche solls denn sein ?
                //
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "Dialogdiet fail!");
                VclAbstractDialog* pMultDlg = pFact->CreateMultiTOXMarkDlg( DLG_MULTMRK,
                                                        pMDI, aMgr);
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
            // Hoehe=Breite fuer konsistentere Vorschau (analog zu Bereich bearbeiten)
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
            AbstractMultiTOXTabDialog* pDlg = pFact->CreateMultiTOXTabDialog( DLG_MULTI_TOX,
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
                rSh.DeleteTOX(*pBase, sal_True);
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

    const sal_Bool bHtmlMode = 0 != ::GetHtmlMode( GetView().GetDocShell() );
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

        sal_Bool bEnableEdit = sal_True;
        sal_Bool bInReadonly = rSh.HasReadonlySel();
        if( rSh.HasSelection() || bInReadonly)
            bEnableEdit = sal_False;
        else
        {
            SwTOXMarks aArr;
            rSh.GetCurTOXMarks( aArr );
            if( aArr.empty())
                bEnableEdit = sal_False;
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
