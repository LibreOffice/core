/*************************************************************************
 *
 *  $RCSfile: drawsh.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:51:04 $
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


#pragma hdrstop


#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_FONTWORK_HXX //autogen
#include <svx/fontwork.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_XFTSFIT_HXX //autogen
#include <svx/xftsfit.hxx>
#endif
#ifndef _SVX_EXTRUSION_BAR_HXX
#include <svx/extrusionbar.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif

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
#include "itemdef.hxx"
#include "swslots.hxx"


SFX_IMPL_INTERFACE(SwDrawShell, SwDrawBaseShell, SW_RES(STR_SHELLNAME_DRAW))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRAW_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_DRAW_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_DRAW));
    SFX_CHILDWINDOW_REGISTRATION(SvxFontWorkChildWindow::GetChildWindowId());
}

TYPEINIT1(SwDrawShell,SwDrawBaseShell)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDrawShell::Execute(SfxRequest &rReq)
{
    SwWrtShell          &rSh = GetShell();
    SdrView             *pSdrView = rSh.GetDrawView();
    const SfxItemSet    *pArgs = rReq.GetArgs();
    SfxBindings         &rBnd  = GetView().GetViewFrame()->GetBindings();
    USHORT               nSlotId = rReq.GetSlot();
    BOOL                 bChanged = pSdrView->GetModel()->IsChanged();

    pSdrView->GetModel()->SetChanged(FALSE);

    const SfxPoolItem* pItem;
    if(pArgs)
        pArgs->GetItemState(nSlotId, FALSE, &pItem);

    BOOL bMirror = TRUE, bTopParam = TRUE, bBottomParam = TRUE;

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
            GetView().AttrChangedNotify(&rSh); // Shellwechsel...
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
                rBnd.Invalidate( SID_OBJECT_HELL, SID_OBJECT_HEAVEN, 0 );
            }
            break;

        case FN_FLIP_HORZ_GRAFIC:
            bMirror = FALSE;
            /* no break */
        case FN_FLIP_VERT_GRAFIC:
            rSh.MirrorSelection( bMirror );
            break;

        case SID_FONTWORK:
        {
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &rSh.GetView()));
            SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
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


        default:
            DBG_ASSERT(!this, "falscher Dispatcher");
            return;
    }
    if (pSdrView->GetModel()->IsChanged())
        rSh.SetModified();
    else if (bChanged)
        pSdrView->GetModel()->SetChanged(TRUE);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawShell::GetState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();
    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();
    BOOL bProtected = rSh.IsSelObjProtected(FLYPROTECT_CONTENT);

    if (!bProtected)    // Im Parent nachsehen
        bProtected |= rSh.IsSelObjProtected( (FlyProtectType)(FLYPROTECT_CONTENT|FLYPROTECT_PARENT) ) != 0;

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
                const BOOL bIsRotate = GetView().IsDrawRotate();
                if ( !bIsRotate && !pSdrView->IsRotateAllowed() || bProtected )
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
                    const USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();
                    rSet.Put(SfxBoolItem( nWhich , GetView().GetViewFrame()->HasChildWindow(nId)));
                }
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }

    svx::ExtrusionBar::getState( pSdrView, rSet );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



SwDrawShell::SwDrawShell(SwView &rView) :
    SwDrawBaseShell(rView)
{
    SetHelpId(SW_DRAWSHELL);
    SetName(String::CreateFromAscii("Draw"));
}

/*************************************************************************
|*
|* SfxRequests fuer FontWork bearbeiten
|*
\************************************************************************/



void SwDrawShell::ExecFormText(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    SdrView*    pDrView = rSh.GetDrawView();
    BOOL        bChanged = pDrView->GetModel()->IsChanged();
    pDrView->GetModel()->SetChanged(FALSE);

    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();
        const SfxPoolItem* pItem;

        if ( pDrView->IsTextEdit() )
        {
            pDrView->EndTextEdit( TRUE );
            GetView().AttrChangedNotify(&rSh);
        }

        if ( rSet.GetItemState(XATTR_FORMTXTSTDFORM, TRUE, &pItem) ==
             SFX_ITEM_SET &&
            ((const XFormTextStdFormItem*) pItem)->GetValue() != XFTFORM_NONE )
        {

            const USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();

            SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*)(GetView().GetViewFrame()->
                                        GetChildWindow(nId)->GetWindow());

            pDlg->CreateStdFormObj(*pDrView, *pDrView->GetPageViewPvNum(0),
                                    rSet, *rMarkList.GetMark(0)->GetObj(),
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
            pDrView->GetModel()->SetChanged(TRUE);
}

/*************************************************************************
|*
|* Statuswerte fuer FontWork zurueckgeben
|*
\************************************************************************/



void SwDrawShell::GetFormTextState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pDrView = rSh.GetDrawView();
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    const SdrObject* pObj = NULL;
    SvxFontWorkDialog* pDlg = NULL;

    const USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();

    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    if ( pVFrame->HasChildWindow(nId) )
        pDlg = (SvxFontWorkDialog*)(pVFrame->GetChildWindow(nId)->GetWindow());

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetObj();

    if ( pObj == NULL || !pObj->ISA(SdrTextObj) ||
        !((SdrTextObj*) pObj)->HasText() )
    {
#define XATTR_ANZ 12
        static const USHORT nXAttr[ XATTR_ANZ ] =
        {
            XATTR_FORMTXTSTYLE, XATTR_FORMTXTADJUST, XATTR_FORMTXTDISTANCE,
            XATTR_FORMTXTSTART, XATTR_FORMTXTMIRROR, XATTR_FORMTXTSTDFORM,
            XATTR_FORMTXTHIDEFORM, XATTR_FORMTXTOUTLINE, XATTR_FORMTXTSHADOW,
            XATTR_FORMTXTSHDWCOLOR, XATTR_FORMTXTSHDWXVAL, XATTR_FORMTXTSHDWYVAL
        };
        for( USHORT i = 0; i < XATTR_ANZ; )
            rSet.DisableItem( nXAttr[ i++ ] );
    }
    else
    {
        if ( pDlg )
            pDlg->SetColorTable(XColorTable::GetStdColorTable());

        pDrView->GetAttributes( rSet );
    }
}




