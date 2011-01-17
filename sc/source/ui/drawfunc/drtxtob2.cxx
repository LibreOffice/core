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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "scitems.hxx"
#include <editeng/adjitem.hxx>
#include <svx/drawitem.hxx>
#include <svx/fontwork.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdocapt.hxx>
#include <svx/xtextit.hxx>
#include <editeng/writingmodeitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sot/formats.hxx>
#include <svl/whiter.hxx>

#include "sc.hrc"
#include "drtxtob.hxx"
#include "viewdata.hxx"
#include "drawview.hxx"
#include "tabvwsh.hxx"
#include "impex.hxx"
#include "docsh.hxx"
#include "transobj.hxx"
#include "drwtrans.hxx"
#include "drwlayer.hxx"

//------------------------------------------------------------------------

sal_uInt16 ScGetFontWorkId()
{
    return SvxFontWorkChildWindow::GetChildWindowId();
}

sal_Bool ScDrawTextObjectBar::IsNoteEdit()
{
    return ScDrawLayer::IsNoteCaption( pViewData->GetView()->GetSdrView()->GetTextEditObject() );
}

//  wenn kein Text editiert wird, Funktionen wie in drawsh

void __EXPORT ScDrawTextObjectBar::ExecuteGlobal( SfxRequest &rReq )
{
    ScTabView*   pTabView  = pViewData->GetView();
    ScDrawView*  pView     = pTabView->GetScDrawView();

    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_COPY:
            pView->DoCopy();
            break;

        case SID_CUT:
            pView->DoCut();
            if (!pTabView->IsDrawSelMode())
                pViewData->GetViewShell()->SetDrawShell( sal_False );
            break;

        case SID_PASTE:
        case SID_PASTE_SPECIAL:
        case SID_CLIPBOARD_FORMAT_ITEMS:
        case SID_HYPERLINK_SETLINK:
            {
                //  cell methods are at cell shell, which is not available if
                //  ScDrawTextObjectBar is active
                //! move paste etc. to view shell?
            }
            break;

        case SID_SELECTALL:
            pView->MarkAll();
            break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            {
                SfxItemSet aAttr( pView->GetModel()->GetItemPool(), SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION, 0 );
                aAttr.Put( SvxWritingModeItem(
                    nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ?
                        com::sun::star::text::WritingMode_LR_TB : com::sun::star::text::WritingMode_TB_RL,
                        SDRATTR_TEXTDIRECTION ) );
                pView->SetAttributes( aAttr );
                pViewData->GetScDrawView()->InvalidateDrawTextAttrs();  // Bidi slots may be disabled
                rReq.Done( aAttr );
            }
            break;

        case SID_ENABLE_HYPHENATION:
            {
                SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, SID_ENABLE_HYPHENATION, sal_False);
                if( pItem )
                {
                    SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
                    sal_Bool bValue = ( (const SfxBoolItem*) pItem)->GetValue();
                    aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                    pView->SetAttributes( aSet );
                }
                rReq.Done();
            }
            break;
    }
}

void ScDrawTextObjectBar::GetGlobalClipState( SfxItemSet& rSet )
{
    //  cell methods are at cell shell, which is not available if
    //  ScDrawTextObjectBar is active -> disable everything
    //! move paste etc. to view shell?

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

void __EXPORT ScDrawTextObjectBar::ExecuteExtra( SfxRequest &rReq )
{
    ScTabView*   pTabView  = pViewData->GetView();
    ScDrawView*  pView     = pTabView->GetScDrawView();

    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_FONTWORK:
            {
                sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();
                SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();

                if ( rReq.GetArgs() )
                    pViewFrm->SetChildWindow( nId,
                                               ((const SfxBoolItem&)
                                                (rReq.GetArgs()->Get(SID_FONTWORK))).
                                                    GetValue() );
                else
                    pViewFrm->ToggleChildWindow( nId );

                pViewFrm->GetBindings().Invalidate( SID_FONTWORK );
                rReq.Done();
            }
            break;

        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
            {
                SfxItemSet aAttr( pView->GetModel()->GetItemPool(),
                                    EE_PARA_WRITINGDIR, EE_PARA_WRITINGDIR,
                                    EE_PARA_JUST, EE_PARA_JUST,
                                    0 );
                sal_Bool bLeft = ( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT );
                aAttr.Put( SvxFrameDirectionItem(
                                bLeft ? FRMDIR_HORI_LEFT_TOP : FRMDIR_HORI_RIGHT_TOP,
                                EE_PARA_WRITINGDIR ) );
                aAttr.Put( SvxAdjustItem(
                                bLeft ? SVX_ADJUST_LEFT : SVX_ADJUST_RIGHT,
                                EE_PARA_JUST ) );
                pView->SetAttributes( aAttr );
                pViewData->GetScDrawView()->InvalidateDrawTextAttrs();
                rReq.Done();        //! Done(aAttr) ?

            }
            break;
    }
}

void ScDrawTextObjectBar::ExecFormText(SfxRequest& rReq)
{
    ScTabView*          pTabView    = pViewData->GetView();
    ScDrawView*         pDrView     = pTabView->GetScDrawView();
    const SdrMarkList&  rMarkList   = pDrView->GetMarkedObjectList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();
        const SfxPoolItem* pItem;

        if ( pDrView->IsTextEdit() )
            pDrView->ScEndTextEdit();

        if (    SFX_ITEM_SET ==
                rSet.GetItemState(XATTR_FORMTXTSTDFORM, sal_True, &pItem)
             && XFTFORM_NONE !=
                ((const XFormTextStdFormItem*) pItem)->GetValue() )
        {

            sal_uInt16 nId              = SvxFontWorkChildWindow::GetChildWindowId();
            SfxViewFrame* pViewFrm  = pViewData->GetViewShell()->GetViewFrame();
            SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*)
                                       (pViewFrm->
                                            GetChildWindow(nId)->GetWindow());

            pDlg->CreateStdFormObj(*pDrView, *pDrView->GetSdrPageView(),
                                    rSet, *rMarkList.GetMark(0)->GetMarkedSdrObj(),
                                   ((const XFormTextStdFormItem*) pItem)->
                                   GetValue());
        }
        else
            pDrView->SetAttributes(rSet);
    }
}

void ScDrawTextObjectBar::GetFormTextState(SfxItemSet& rSet)
{
    const SdrObject*    pObj        = NULL;
    SvxFontWorkDialog*  pDlg        = NULL;
    ScDrawView*         pDrView     = pViewData->GetView()->GetScDrawView();
    const SdrMarkList&  rMarkList   = pDrView->GetMarkedObjectList();
    sal_uInt16              nId = SvxFontWorkChildWindow::GetChildWindowId();

    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    if ( pViewFrm->HasChildWindow(nId) )
        pDlg = (SvxFontWorkDialog*)(pViewFrm->GetChildWindow(nId)->GetWindow());

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    if ( pObj == NULL || !pObj->ISA(SdrTextObj) ||
        !((SdrTextObj*) pObj)->HasText() )
    {
        if ( pDlg )
            pDlg->SetActive(sal_False);

        rSet.DisableItem(XATTR_FORMTXTSTYLE);
        rSet.DisableItem(XATTR_FORMTXTADJUST);
        rSet.DisableItem(XATTR_FORMTXTDISTANCE);
        rSet.DisableItem(XATTR_FORMTXTSTART);
        rSet.DisableItem(XATTR_FORMTXTMIRROR);
        rSet.DisableItem(XATTR_FORMTXTSTDFORM);
        rSet.DisableItem(XATTR_FORMTXTHIDEFORM);
        rSet.DisableItem(XATTR_FORMTXTOUTLINE);
        rSet.DisableItem(XATTR_FORMTXTSHADOW);
        rSet.DisableItem(XATTR_FORMTXTSHDWCOLOR);
        rSet.DisableItem(XATTR_FORMTXTSHDWXVAL);
        rSet.DisableItem(XATTR_FORMTXTSHDWYVAL);
    }
    else
    {
        if ( pDlg )
        {
            SfxObjectShell* pDocSh = SfxObjectShell::Current();

            if ( pDocSh )
            {
                const SfxPoolItem*  pItem = pDocSh->GetItem( SID_COLOR_TABLE );
                XColorTable*        pColorTable = NULL;

                if ( pItem )
                    pColorTable = ((SvxColorTableItem*)pItem)->GetColorTable();

                pDlg->SetActive();

                if ( pColorTable )
                    pDlg->SetColorTable( pColorTable );
                else
                    { DBG_ERROR( "ColorList not found :-/" ); }
            }
        }
        SfxItemSet aViewAttr(pDrView->GetModel()->GetItemPool());
        pDrView->GetAttributes(aViewAttr);
        rSet.Set(aViewAttr);
    }
}




