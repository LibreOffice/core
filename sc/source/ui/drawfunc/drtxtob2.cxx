/*************************************************************************
 *
 *  $RCSfile: drtxtob2.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:28:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//-------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/adjitem.hxx>
#include <svx/drawitem.hxx>
#include <svx/fontwork.hxx>
#include <svx/frmdiritem.hxx>
#include <svx/outlobj.hxx>
#include <svx/svdocapt.hxx>
#include <svx/xtextit.hxx>
#ifndef _SVX_WRITINGMODEITEM_HXX
#include <svx/writingmodeitem.hxx>
#endif
#include <sfx2/bindings.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sot/formats.hxx>
#include <svtools/whiter.hxx>

#include "sc.hrc"
#include "drtxtob.hxx"
#include "viewdata.hxx"
#include "drawview.hxx"
#include "tabvwsh.hxx"
#include "impex.hxx"
#include "docsh.hxx"
#include "transobj.hxx"
#include "drwtrans.hxx"

//------------------------------------------------------------------------

USHORT ScGetFontWorkId()
{
    return SvxFontWorkChildWindow::GetChildWindowId();
}

BOOL ScDrawTextObjectBar::IsNoteEdit()
{
    ScTabView* pTabView = pViewData->GetView();
    SdrView*   pSdrView = pTabView->GetSdrView();
    SdrObject* pObject  = pSdrView->GetTextEditObject();
    return ( pObject && pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA( SdrCaptionObj ) );
}

//  wenn kein Text editiert wird, Funktionen wie in drawsh

void __EXPORT ScDrawTextObjectBar::ExecuteGlobal( SfxRequest &rReq )
{
    ScTabView*   pTabView  = pViewData->GetView();
    ScDrawView*  pView     = pTabView->GetScDrawView();

    USHORT nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_COPY:
            pView->DoCopy();
            break;

        case SID_CUT:
            pView->DoCut();
            if (!pTabView->IsDrawSelMode())
                pViewData->GetViewShell()->SetDrawShell( FALSE );
            break;

        case SID_PASTE:
        case FID_PASTE_CONTENTS:
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
                aAttr.Put( SvxWritingModeItem( nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ? com::sun::star::text::WritingMode_LR_TB : com::sun::star::text::WritingMode_TB_RL ) );
                pView->SetAttributes( aAttr );
                pViewData->GetScDrawView()->InvalidateDrawTextAttrs();  // Bidi slots may be disabled
                rReq.Done( aAttr );
            }
            break;

        case SID_ENABLE_HYPHENATION:
            {
                SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, SID_ENABLE_HYPHENATION, FALSE);
                if( pItem )
                {
                    SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
                    BOOL bValue = ( (const SfxBoolItem*) pItem)->GetValue();
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
    USHORT nWhich = aIter.FirstWhich();
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

    USHORT nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_FONTWORK:
            {
                USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();
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
                BOOL bLeft = ( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT );
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
                rSet.GetItemState(XATTR_FORMTXTSTDFORM, TRUE, &pItem)
             && XFTFORM_NONE !=
                ((const XFormTextStdFormItem*) pItem)->GetValue() )
        {

            USHORT nId              = SvxFontWorkChildWindow::GetChildWindowId();
            SfxViewFrame* pViewFrm  = pViewData->GetViewShell()->GetViewFrame();
            SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*)
                                       (pViewFrm->
                                            GetChildWindow(nId)->GetWindow());

            pDlg->CreateStdFormObj(*pDrView, *pDrView->GetPageViewPvNum(0),
                                    rSet, *rMarkList.GetMark(0)->GetObj(),
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
    USHORT              nId = SvxFontWorkChildWindow::GetChildWindowId();

    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    if ( pViewFrm->HasChildWindow(nId) )
        pDlg = (SvxFontWorkDialog*)(pViewFrm->GetChildWindow(nId)->GetWindow());

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetObj();

    if ( pObj == NULL || !pObj->ISA(SdrTextObj) ||
        !((SdrTextObj*) pObj)->HasText() )
    {
        if ( pDlg )
            pDlg->SetActive(FALSE);

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
                const SfxPoolItem*  pItem = pDocSh->GetItem( ITEMID_COLOR_TABLE );
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




