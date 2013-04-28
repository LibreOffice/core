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

#include <tools/shl.hxx>
#include <svx/svdview.hxx>
#include <svx/svdotext.hxx>
#include <svl/whiter.hxx>
#include <svx/fontwork.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <svl/itemiter.hxx>
#include <svl/srchitem.hxx>
#include <svx/xftsfit.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/tbxcustomshapes.hxx>
#include <uitool.hxx>
#include <wview.hxx>
#include <swmodule.hxx>
#include <swwait.hxx>
#include <docstat.hxx>
#include <IDocumentStatistics.hxx>
#include <tools/diagnose_ex.h>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>

#include <svx/xtable.hxx>

#include "swundo.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "helpid.h"
#include "popup.hrc"
#include "shells.hrc"
#include "drwbassh.hxx"
#include "drawsh.hxx"

#define SwDrawShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"
#include "swabstdlg.hxx"
#include <wordcountdialog.hxx>
#include "misc.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SFX_IMPL_INTERFACE(SwDrawShell, SwDrawBaseShell, SW_RES(STR_SHELLNAME_DRAW))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRAW_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_DRAW_TOOLBOX));
    SFX_CHILDWINDOW_REGISTRATION(SvxFontWorkChildWindow::GetChildWindowId());
}

TYPEINIT1(SwDrawShell,SwDrawBaseShell)

void SwDrawShell::Execute(SfxRequest &rReq)
{
    SwWrtShell          &rSh = GetShell();
    SdrView             *pSdrView = rSh.GetDrawView();
    const SfxItemSet    *pArgs = rReq.GetArgs();
    SfxBindings         &rBnd  = GetView().GetViewFrame()->GetBindings();
    sal_uInt16               nSlotId = rReq.GetSlot();
    sal_Bool                 bChanged = pSdrView->GetModel()->IsChanged();

    pSdrView->GetModel()->SetChanged(sal_False);

    const SfxPoolItem* pItem;
    if(pArgs)
        pArgs->GetItemState(nSlotId, sal_False, &pItem);

    bool bMirror = true;

    switch (nSlotId)
    {
        case SID_OBJECT_ROTATE:
            if (rSh.IsObjSelected() && pSdrView->IsRotateAllowed())
            {
                if (GetView().IsDrawRotate())
                    rSh.SetDragMode(SDRDRAG_MOVE);
                else
                    rSh.SetDragMode(SDRDRAG_ROTATE);

                GetView().FlipDrawRotate();
            }
            break;

        case SID_BEZIER_EDIT:
            if (GetView().IsDrawRotate())
            {
                rSh.SetDragMode(SDRDRAG_MOVE);
                GetView().FlipDrawRotate();
            }
            GetView().FlipDrawSelMode();
            pSdrView->SetFrameDragSingles(GetView().IsDrawSelMode());
            GetView().AttrChangedNotify(&rSh); // Shell switch
            break;

        case SID_OBJECT_HELL:
            if (rSh.IsObjSelected())
            {
                rSh.StartUndo( UNDO_START );
                SetWrapMode(FN_FRAME_WRAPTHRU_TRANSP);
                rSh.SelectionToHell();
                rSh.EndUndo( UNDO_END );
                rBnd.Invalidate(SID_OBJECT_HEAVEN);
            }
            break;

        case SID_OBJECT_HEAVEN:
            if (rSh.IsObjSelected())
            {
                rSh.StartUndo( UNDO_START );
                SetWrapMode(FN_FRAME_WRAPTHRU);
                rSh.SelectionToHeaven();
                rSh.EndUndo( UNDO_END );
                rBnd.Invalidate(SID_OBJECT_HELL);
            }
            break;

        case FN_TOOL_HIERARCHIE:
            if (rSh.IsObjSelected())
            {
                rSh.StartUndo( UNDO_START );
                if (rSh.GetLayerId() == 0)
                {
                    SetWrapMode(FN_FRAME_WRAPTHRU);
                    rSh.SelectionToHeaven();
                }
                else
                {
                    SetWrapMode(FN_FRAME_WRAPTHRU_TRANSP);
                    rSh.SelectionToHell();
                }
                rSh.EndUndo( UNDO_END );
                rBnd.Invalidate( SID_OBJECT_HELL );
                rBnd.Invalidate( SID_OBJECT_HEAVEN );
            }
            break;

        case FN_FLIP_HORZ_GRAFIC:
            bMirror = false;
            /* no break */
        case FN_FLIP_VERT_GRAFIC:
            rSh.MirrorSelection( bMirror );
            break;

        case SID_FONTWORK:
        {
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &rSh.GetView()));
            SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            if (pArgs)
            {
                pVFrame->SetChildWindow(SvxFontWorkChildWindow::GetChildWindowId(),
                    ((const SfxBoolItem&)(pArgs->Get(SID_FONTWORK))).GetValue());
            }
            else
                pVFrame->ToggleChildWindow( SvxFontWorkChildWindow::GetChildWindowId() );
            pVFrame->GetBindings().Invalidate(SID_FONTWORK);
        }
        break;
        case FN_FORMAT_FOOTNOTE_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            VclAbstractDialog* pDlg = pFact->CreateSwFootNoteOptionDlg( GetView().GetWindow(), GetView().GetWrtShell(), DLG_DOC_FOOTNOTE );
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
            break;
        }
        case FN_NUMBERING_OUTLINE_DLG:
        {
            SfxItemSet aTmp(GetPool(), FN_PARAM_1, FN_PARAM_1);
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "Dialogdiet fail!");
            SfxAbstractTabDialog* pDlg = pFact->CreateSwTabDialog( DLG_TAB_OUTLINE,
                                                        GetView().GetWindow(), &aTmp, GetView().GetWrtShell());
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
            rReq.Done();
        }
        break;
        case SID_OPEN_XML_FILTERSETTINGS:
        {
            try
            {
                uno::Reference < ui::dialogs::XExecutableDialog > xDialog = ui::dialogs::XSLTFilterDialog::create( ::comphelper::getProcessComponentContext() );
                xDialog->execute();
            }
            catch (const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            rReq.Ignore ();
        }
        break;
        case FN_WORDCOUNT_DIALOG:
        {
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            if (pVFrame != NULL)
            {
                pVFrame->ToggleChildWindow(FN_WORDCOUNT_DIALOG);
                Invalidate(rReq.GetSlot());

                SwWordCountWrapper *pWrdCnt = (SwWordCountWrapper*)pVFrame->GetChildWindow(SwWordCountWrapper::GetChildWindowId());
                if (pWrdCnt)
                    pWrdCnt->UpdateCounts();
            }
        }
        break;
        case SID_EXTRUSION_TOOGLE:
        case SID_EXTRUSION_TILT_DOWN:
        case SID_EXTRUSION_TILT_UP:
        case SID_EXTRUSION_TILT_LEFT:
        case SID_EXTRUSION_TILT_RIGHT:
        case SID_EXTRUSION_3D_COLOR:
        case SID_EXTRUSION_DEPTH:
        case SID_EXTRUSION_DIRECTION:
        case SID_EXTRUSION_PROJECTION:
        case SID_EXTRUSION_LIGHTING_DIRECTION:
        case SID_EXTRUSION_LIGHTING_INTENSITY:
        case SID_EXTRUSION_SURFACE:
        case SID_EXTRUSION_DEPTH_FLOATER:
        case SID_EXTRUSION_DIRECTION_FLOATER:
        case SID_EXTRUSION_LIGHTING_FLOATER:
        case SID_EXTRUSION_SURFACE_FLOATER:
        case SID_EXTRUSION_DEPTH_DIALOG:
            svx::ExtrusionBar::execute( pSdrView, rReq, rBnd );
            rReq.Ignore ();
            break;

        case SID_FONTWORK_SHAPE:
        case SID_FONTWORK_SHAPE_TYPE:
        case SID_FONTWORK_ALIGNMENT:
        case SID_FONTWORK_SAME_LETTER_HEIGHTS:
        case SID_FONTWORK_CHARACTER_SPACING:
        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        case SID_FONTWORK_CHARACTER_SPACING_FLOATER:
        case SID_FONTWORK_ALIGNMENT_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_DIALOG:
            svx::FontworkBar::execute( pSdrView, rReq, rBnd );
            rReq.Ignore ();
            break;

        default:
            OSL_ENSURE(!this, "wrong dispatcher");
            return;
    }
    if (pSdrView->GetModel()->IsChanged())
        rSh.SetModified();
    else if (bChanged)
        pSdrView->GetModel()->SetChanged(sal_True);
}

void SwDrawShell::GetState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    sal_Bool bProtected = rSh.IsSelObjProtected(FLYPROTECT_CONTENT);

    if (!bProtected)    // Check the parent
        bProtected |= rSh.IsSelObjProtected( FLYPROTECT_CONTENT|FLYPROTECT_PARENT ) != 0;

    while( nWhich )
    {
        switch( nWhich )
        {
            case SID_OBJECT_HELL:
                if ( !rSh.IsObjSelected() || rSh.GetLayerId() == 0 || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case SID_OBJECT_HEAVEN:
                if ( !rSh.IsObjSelected() || rSh.GetLayerId() == 1 || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case FN_TOOL_HIERARCHIE:
                if ( !rSh.IsObjSelected() || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case SID_OBJECT_ROTATE:
            {
                const sal_Bool bIsRotate = GetView().IsDrawRotate();
                if ( (!bIsRotate && !pSdrView->IsRotateAllowed()) || bProtected )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( nWhich, bIsRotate ) );
            }
            break;

            case SID_BEZIER_EDIT:
                if (!Disable(rSet, nWhich))
                    rSet.Put( SfxBoolItem( nWhich, !GetView().IsDrawSelMode()));
            break;

            case FN_FLIP_HORZ_GRAFIC:
                if ( !pSdrView->IsMirrorAllowed() || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case FN_FLIP_VERT_GRAFIC:
                if ( !pSdrView->IsMirrorAllowed() || bProtected )
                    rSet.DisableItem( nWhich );
                break;

            case SID_FONTWORK:
            {
                if (bProtected)
                    rSet.DisableItem( nWhich );
                else
                {
                    const sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();
                    rSet.Put(SfxBoolItem( nWhich , GetView().GetViewFrame()->HasChildWindow(nId)));
                }
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
    svx::ExtrusionBar::getState( pSdrView, rSet );
    svx::FontworkBar::getState( pSdrView, rSet );
}

SwDrawShell::SwDrawShell(SwView &_rView) :
    SwDrawBaseShell(_rView)
{
    SetHelpId(SW_DRAWSHELL);
    SetName(OUString("Draw"));
}

// Edit SfxRequests for FontWork

void SwDrawShell::ExecFormText(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    SdrView*    pDrView = rSh.GetDrawView();
    sal_Bool        bChanged = pDrView->GetModel()->IsChanged();
    pDrView->GetModel()->SetChanged(sal_False);

    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();
        const SfxPoolItem* pItem;

        if ( pDrView->IsTextEdit() )
        {
            pDrView->SdrEndTextEdit( sal_True );
            GetView().AttrChangedNotify(&rSh);
        }

        if ( rSet.GetItemState(XATTR_FORMTXTSTDFORM, sal_True, &pItem) ==
             SFX_ITEM_SET &&
            ((const XFormTextStdFormItem*) pItem)->GetValue() != XFTFORM_NONE )
        {

            const sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();

            SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*)(GetView().GetViewFrame()->
                                        GetChildWindow(nId)->GetWindow());

            pDlg->CreateStdFormObj(*pDrView, *pDrView->GetSdrPageView(),
                                    rSet, *rMarkList.GetMark(0)->GetMarkedSdrObj(),
                                   ((const XFormTextStdFormItem*) pItem)->
                                   GetValue());

        }
        else
            pDrView->SetAttributes(rSet);
    }
    if (pDrView->GetModel()->IsChanged())
        rSh.SetModified();
    else
        if (bChanged)
            pDrView->GetModel()->SetChanged(sal_True);
}

//Return status values for FontWork

void SwDrawShell::GetFormTextState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pDrView = rSh.GetDrawView();
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    const SdrObject* pObj = NULL;
    SvxFontWorkDialog* pDlg = NULL;

    const sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();

    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    if ( pVFrame->HasChildWindow(nId) )
        pDlg = (SvxFontWorkDialog*)(pVFrame->GetChildWindow(nId)->GetWindow());

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    if ( pObj == NULL || !pObj->ISA(SdrTextObj) ||
        !((SdrTextObj*) pObj)->HasText() )
    {
#define XATTR_ANZ 12
        static const sal_uInt16 nXAttr[ XATTR_ANZ ] =
        {
            XATTR_FORMTXTSTYLE, XATTR_FORMTXTADJUST, XATTR_FORMTXTDISTANCE,
            XATTR_FORMTXTSTART, XATTR_FORMTXTMIRROR, XATTR_FORMTXTSTDFORM,
            XATTR_FORMTXTHIDEFORM, XATTR_FORMTXTOUTLINE, XATTR_FORMTXTSHADOW,
            XATTR_FORMTXTSHDWCOLOR, XATTR_FORMTXTSHDWXVAL, XATTR_FORMTXTSHDWYVAL
        };
        for( sal_uInt16 i = 0; i < XATTR_ANZ; )
            rSet.DisableItem( nXAttr[ i++ ] );
    }
    else
    {
        if ( pDlg )
            pDlg->SetColorList(XColorList::GetStdColorList());

        pDrView->GetAttributes( rSet );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
