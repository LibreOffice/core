/*************************************************************************
 *
 *  $RCSfile: drviews6.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:55:39 $
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

#include "DrawViewShell.hxx"

#ifndef _SV_METAACT_HXX //autogen
#include <vcl/metaact.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVXIDS_HXX
#include <svx/svxids.hrc>
#endif
#ifndef _FONTWORK_HXX
#include <svx/fontwork.hxx>
#endif
#ifndef _BMPMASK_HXX
#include <svx/bmpmask.hxx>
#endif
#ifndef _SVX_GALBRWS_HXX
#include <svx/galbrws.hxx>
#endif
#ifndef _IMAPDLG_HXX
#include <svx/imapdlg.hxx>
#endif
#ifndef _SVX_XFTSFIT_HXX //autogen
#include <svx/xftsfit.hxx>
#endif
#ifndef _SVX_COLRCTRL_HXX //autogen
#include <svx/colrctrl.hxx>
#endif

#ifndef _SVX_F3DCHILD_HXX //autogen
#include <svx/f3dchild.hxx>
#endif
#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif
#ifndef _SVX_EXTRUSION_BAR_HXX
#include <svx/extrusionbar.hxx>
#endif
#ifndef _SVX_FONTWORK_BAR_HXX
#include <svx/fontworkbar.hxx>
#endif

#include "app.hrc"
#include "strings.hrc"

#include "app.hxx"
#include "animobjs.hxx"
#ifndef SD_ANIMATION_CHILD_WINDOW_HXX
#include "AnimationChildWindow.hxx"
#endif
#ifndef SD_NAVIGATOR_CHILD_WINDOW_HXX
#include "NavigatorChildWindow.hxx"
#endif
#ifndef SD_PREVIEW_CHILD_WINDOW_HXX
#include "PreviewChildWindow.hxx"
#endif
#ifndef SD_EFFECT_CHILD_WINDOW_HXX
#include "EffectChildWindow.hxx"
#endif
#ifndef SD_LAYER_DIALOG_CHILD_WINDOW_HXX
#include "LayerDialogChildWindow.hxx"
#endif
#ifndef SD_SLIDE_CHANGE_CHILD_WINDOW_HXX
#include "SlideChangeChildWindow.hxx"
#endif
#include "sdresid.hxx"
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif
#ifndef SD_FU_SLIDE_SHOW_DLG_HXX
#include "fusldlg.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_FU_EXPAND_PAGE_HXX
#include "fuexpand.hxx"
#endif
#ifndef SD_FU_SUMMARY_PAGE_HXX
#include "fusumry.hxx"
#endif
#ifndef SD_FU_CUSTOM_SHOW_DLG_HXX
#include "fucushow.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef SD_PREVIEW_WINDOW_HXX
#include "PreviewWindow.hxx"
#endif
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif

namespace sd {

/*************************************************************************
|*
|* SfxRequests fuer FontWork bearbeiten
|*
\************************************************************************/

void DrawViewShell::ExecFormText(SfxRequest& rReq)
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (pFuActual && pFuActual->GetSlotID() == SID_PRESENTATION)
        return;

    CheckLineTo (rReq);

    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() &&
         !pDrView->IsPresObjSelected() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();
        const SfxPoolItem* pItem;

        if ( pDrView->IsTextEdit() )
            pDrView->EndTextEdit();

        if ( rSet.GetItemState(XATTR_FORMTXTSTDFORM, TRUE, &pItem) ==
             SFX_ITEM_SET &&
            ((const XFormTextStdFormItem*) pItem)->GetValue() != XFTFORM_NONE )
        {

            USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();

            SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*)GetViewFrame()->
                                        GetChildWindow(nId)->GetWindow();

            pDlg->CreateStdFormObj(*pDrView, *pDrView->GetPageViewPvNum(0),
                                    rSet, *rMarkList.GetMark(0)->GetObj(),
                                   ((const XFormTextStdFormItem*) pItem)->
                                   GetValue());

            if (pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT)
            {   // ggf. die richtige Editfunktion aktivieren
                GetViewFrame()->GetDispatcher()->Execute(SID_SWITCH_POINTEDIT,
                                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
        }
        else
            pDrView->SetAttributes(rSet);
    }
}

/*************************************************************************
|*
|* Statuswerte fuer FontWork zurueckgeben
|*
\************************************************************************/

void DrawViewShell::GetFormTextState(SfxItemSet& rSet)
{
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    const SdrObject* pObj = NULL;
    SvxFontWorkDialog* pDlg = NULL;

    USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();

    if ( GetViewFrame()->HasChildWindow(nId) )
        pDlg = (SvxFontWorkDialog*)(GetViewFrame()->GetChildWindow(nId)->GetWindow());

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetObj();

    if ( pObj == NULL || !pObj->ISA(SdrTextObj) ||
        !((SdrTextObj*) pObj)->HasText() )
    {
// automatisches Auf/Zuklappen des FontWork-Dialog; erstmal deaktiviert
//      if ( pDlg )
//          pDlg->SetActive(FALSE);

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
//          pDlg->SetActive();
            pDlg->SetColorTable(GetDoc()->GetColorTable());
        }

        SfxItemSet aSet( GetDoc()->GetPool() );
        pDrView->GetAttributes( aSet );
        rSet.Set( aSet );
    }
}

/*************************************************************************
|*
|* SfxRequests fuer Objektpalette bearbeiten
|*
\************************************************************************/

void DrawViewShell::ExecObjPalette( SfxRequest& rReq )
{
    // Diese Methode muss erhalten bleiben, bis
    // der/die Slots entfernt wurden;
    // ==> Voll-Update
}

/*************************************************************************
|*
|* Statuswerte fuer Objektpalette zurueckgeben
|*
\************************************************************************/

void DrawViewShell::GetObjPaletteState(SfxItemSet& rSet)
{
    // Diese Methode muss erhalten bleiben, bis
    // der/die Slots entfernt wurden;
    // ==> Voll-Update
}

/*************************************************************************
|*
|* SfxRequests fuer Animator bearbeiten
|*
\************************************************************************/

void DrawViewShell::ExecAnimationWin( SfxRequest& rReq )
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (pFuActual && pFuActual->GetSlotID() == SID_PRESENTATION)
        return;

    CheckLineTo (rReq);

    USHORT nSId = rReq.GetSlot();

    switch( nSId )
    {
        case SID_ANIMATOR_INIT:
        case SID_ANIMATOR_ADD:
        case SID_ANIMATOR_CREATE:
        {
            AnimationWindow* pAnimWin;
            USHORT nId = AnimationChildWindow::GetChildWindowId();
            //((const SfxUInt16Item&)(rReq.GetArgs()->Get(nSId))).GetValue();

            pAnimWin = static_cast<AnimationWindow*>(
                GetViewFrame()->GetChildWindow(nId)->GetWindow());

            if ( pAnimWin )
            {
                if( nSId == SID_ANIMATOR_ADD )
                    pAnimWin->AddObj( *pDrView );
                else if( nSId == SID_ANIMATOR_CREATE )
                    pAnimWin->CreateAnimObj( *pDrView );
            }
        }
        break;

        default:
        break;
    }
}

/*************************************************************************
|*
|* Statuswerte fuer Animator zurueckgeben
|*
|* nValue == 0 -> Kein Button
|* nValue == 1 -> Button 'uebernehmen'
|* nValue == 2 -> Button 'einzeln uebernehmen'
|* nValue == 3 -> Buttons 'uebernehmen' und 'einzeln uebernehmen'
|*
\************************************************************************/

void DrawViewShell::GetAnimationWinState( SfxItemSet& rSet )
{
    // Hier koennten Buttons etc. disabled werden
    UINT16 nValue;

    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    ULONG nMarkCount = rMarkList.GetMarkCount();

    if( nMarkCount == 0 )
        nValue = 0;
    else if( nMarkCount > 1 )
        nValue = 3;
    else // 1 Objekt
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();
        UINT32 nInv = pObj->GetObjInventor();
        UINT16 nId  = pObj->GetObjIdentifier();
        // 1 selektiertes Gruppenobjekt
        if( nInv == SdrInventor && nId == OBJ_GRUP )
            nValue = 3;
        else if( nInv == SdrInventor && nId == OBJ_GRAF ) // Anim. GIF ?
        {
            USHORT nCount = 0;

            if( ( (SdrGrafObj*) pObj )->IsAnimated() )
                nCount = ( (SdrGrafObj*) pObj )->GetGraphic().GetAnimation().Count();
            if( nCount > 0 )
                nValue = 2;
            else
                nValue = 1;
        }
        else
            nValue = 1;
    }
    rSet.Put( SfxUInt16Item( SID_ANIMATOR_STATE, nValue ) );
}

/*************************************************************************
|*
|* Statuswerte fuer SfxChildWindows setzen
|*
\************************************************************************/

void DrawViewShell::SetChildWindowState( SfxItemSet& rSet )
{
    // Stati der SfxChild-Windows (Animator, Fontwork etc.)
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_FONTWORK ) )
    {
        USHORT nId = SvxFontWorkChildWindow::GetChildWindowId();
        rSet.Put(SfxBoolItem(SID_FONTWORK, GetViewFrame()->HasChildWindow(nId)));
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_COLOR_CONTROL ) )
    {
        USHORT nId = SvxColorChildWindow::GetChildWindowId();
        rSet.Put(SfxBoolItem(SID_COLOR_CONTROL, GetViewFrame()->HasChildWindow(nId)));
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ANIMATION_OBJECTS ) )
    {
        USHORT nId = AnimationChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_ANIMATION_OBJECTS, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_NAVIGATOR ) )
    {
        USHORT nId = SID_NAVIGATOR;
        rSet.Put( SfxBoolItem( SID_NAVIGATOR, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_BMPMASK ) )
    {
        USHORT nId = SvxBmpMaskChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_BMPMASK, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GALLERY ) )
    {
        USHORT nId = GalleryChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_GALLERY, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_IMAP ) )
    {
        USHORT nId = SvxIMapDlgChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_IMAP, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_WIN ) )
    {
        USHORT nId = PreviewChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_PREVIEW_WIN, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_EFFECT_WIN ) )
    {
        USHORT nId = EffectChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_EFFECT_WIN, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_LAYER_DIALOG_WIN ) )
    {
        USHORT nId = LayerDialogChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_LAYER_DIALOG_WIN, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_SLIDE_CHANGE_WIN ) )
    {
        USHORT nId = SlideChangeChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_SLIDE_CHANGE_WIN, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_3D_WIN ) )
    {
        USHORT nId = Svx3DChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_3D_WIN, GetViewFrame()->HasChildWindow( nId ) ) );
    }
}


/*************************************************************************
|*
|* SfxRequests fuer Pipette bearbeiten
|*
\************************************************************************/

void DrawViewShell::ExecBmpMask( SfxRequest& rReq )
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (pFuActual && pFuActual->GetSlotID() == SID_PRESENTATION)
        return;

    switch ( rReq.GetSlot() )
    {
        case ( SID_BMPMASK_PIPETTE ) :
        {
            bPipette = ( (const SfxBoolItem&) ( rReq.GetArgs()->
                       Get( SID_BMPMASK_PIPETTE ) ) ).GetValue();
        }
        break;

        case ( SID_BMPMASK_EXEC ) :
        {
            SdrGrafObj* pObj = (SdrGrafObj*) pDrView->GetMarkedObjectList().GetMark(0)->GetObj();

            if ( pObj && !pDrView->IsTextEdit() )
            {
                SdrGrafObj* pNewObj = (SdrGrafObj*) pObj->Clone();
                BOOL        bCont = TRUE;

                if( pNewObj->IsLinkedGraphic() )
                {
                    QueryBox aQBox( (Window*) GetActiveWindow(), WB_YES_NO | WB_DEF_YES,
                                    String( SdResId( STR_RELEASE_GRAPHICLINK ) ) );

                    if( RET_YES == aQBox.Execute() )
                        pNewObj->ReleaseGraphicLink();
                    else
                    {
                        delete pNewObj;
                        bCont = FALSE;
                    }
                }

                if( bCont )
                {
                    const Graphic&  rOldGraphic = pNewObj->GetGraphic();
                    const Graphic   aNewGraphic( ( (SvxBmpMask*) GetViewFrame()->GetChildWindow(
                                                 SvxBmpMaskChildWindow::GetChildWindowId() )->GetWindow() )->
                                                 Mask( rOldGraphic ) );

                    if( aNewGraphic != rOldGraphic )
                    {
                        SdrPageView* pPV = pDrView->GetPageViewPvNum( 0 );

                        pNewObj->SetEmptyPresObj( FALSE );
                        pNewObj->SetGraphic( ( (SvxBmpMask*) GetViewFrame()->GetChildWindow(
                                             SvxBmpMaskChildWindow::GetChildWindowId() )->GetWindow() )->
                                             Mask( pNewObj->GetGraphic() ) );

                        String aStr( pDrView->GetDescriptionOfMarkedObjects() );
                        aStr += (sal_Unicode)( ' ' ), aStr += String( SdResId( STR_EYEDROPPER ) );

                        pDrView->BegUndo( aStr );
                        pDrView->ReplaceObject( pObj, *pPV, pNewObj );
                        pDrView->EndUndo();
                    }
                }
            }
        }
        break;

        default:
        break;
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawViewShell::GetBmpMaskState( SfxItemSet& rSet )
{
    const SdrMarkList&  rMarkList = pDrView->GetMarkedObjectList();
    const SdrObject*    pObj = NULL;
    USHORT              nId = SvxBmpMaskChildWindow::GetChildWindowId();
    SvxBmpMask*         pDlg = NULL;
    BOOL                bEnable = FALSE;

    if ( GetViewFrame()->HasChildWindow( nId ) )
    {
        pDlg = (SvxBmpMask*) ( GetViewFrame()->GetChildWindow( nId )->GetWindow() );

        if ( pDlg->NeedsColorTable() )
            pDlg->SetColorTable( GetDoc()->GetColorTable() );
    }

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetObj();

    // valid graphic object?
    if( pObj && pObj->ISA( SdrGrafObj ) && !( (SdrGrafObj*) pObj )->IsEPS() && !pDrView->IsTextEdit() )
        bEnable = TRUE;

    // put value
    rSet.Put( SfxBoolItem( SID_BMPMASK_EXEC, bEnable ) );
}

/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void DrawViewShell::FuTemp04(SfxRequest& rReq)
{
    USHORT nSId = rReq.GetSlot();
    switch( nSId )
    {
        case SID_FONTWORK:
        {
            if ( rReq.GetArgs() )
            {
                GetViewFrame()->SetChildWindow(SvxFontWorkChildWindow::GetChildWindowId(),
                                        ((const SfxBoolItem&) (rReq.GetArgs()->
                                        Get(SID_FONTWORK))).GetValue());
            }
            else
            {
                GetViewFrame()->ToggleChildWindow( SvxFontWorkChildWindow::GetChildWindowId() );
            }

            GetViewFrame()->GetBindings().Invalidate(SID_FONTWORK);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_COLOR_CONTROL:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(SvxColorChildWindow::GetChildWindowId(),
                                        ((const SfxBoolItem&) (rReq.GetArgs()->
                                        Get(SID_COLOR_CONTROL))).GetValue());
            else
                GetViewFrame()->ToggleChildWindow(SvxColorChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate(SID_COLOR_CONTROL);
            Cancel();
            rReq.Ignore ();
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
            svx::ExtrusionBar::execute( pDrView, rReq, GetViewFrame()->GetBindings() );
            Cancel();
            rReq.Ignore ();
            break;

        case SID_FONTWORK_SHAPE:
        case SID_FONTWORK_ALIGNMENT:
        case SID_FONTWORK_CHARACTER_SPACING:
        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        case SID_FONTWORK_GALLERY_FLOATER:
        case SID_FONTWORK_SHAPE_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_FLOATER:
        case SID_FONTWORK_ALIGNMENT_FLOATER:
            svx::FontworkBar::execute( pDrView, rReq );
            Cancel();
            rReq.Ignore ();
            break;

        case SID_BMPMASK:
        {
            GetViewFrame()->ToggleChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() );
            GetViewFrame()->GetBindings().Invalidate( SID_BMPMASK );

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_GALLERY:
        {
            GetViewFrame()->ToggleChildWindow( GalleryChildWindow::GetChildWindowId() );
            GetViewFrame()->GetBindings().Invalidate( SID_GALLERY );

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_NAVIGATOR:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(SID_NAVIGATOR,
                                        ((const SfxBoolItem&) (rReq.GetArgs()->
                                        Get(SID_NAVIGATOR))).GetValue());
            else
                GetViewFrame()->ToggleChildWindow( SID_NAVIGATOR );

            GetViewFrame()->GetBindings().Invalidate(SID_NAVIGATOR);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_ANIMATION_OBJECTS:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(
                    AnimationChildWindow::GetChildWindowId(),
                    ((const SfxBoolItem&) (rReq.GetArgs()->
                        Get(SID_ANIMATION_OBJECTS))).GetValue());
            else
                GetViewFrame()->ToggleChildWindow(
                    AnimationChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate(SID_ANIMATION_OBJECTS);
            Cancel();
            rReq.Ignore ();
        }
        break;


        case SID_PREVIEW_WIN:
        {
            bool bPreview = FALSE;

            if ( rReq.GetArgs() )
            {
                bPreview = ((const SfxBoolItem&) (rReq.GetArgs()->Get(SID_PREVIEW_WIN))).GetValue();
            }
            else
            {
                USHORT nId = PreviewChildWindow::GetChildWindowId();
                bPreview = !SfxBoolItem(SID_PREVIEW_WIN, GetViewFrame()->HasChildWindow(nId)).GetValue();
            }

            SetPreview( bPreview );

            if (eEditMode == EM_PAGE)
                pFrameView->SetShowPreviewInPageMode(bPreview);
            else
                pFrameView->SetShowPreviewInMasterPageMode(bPreview);

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_EFFECT_WIN:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(
                    EffectChildWindow::GetChildWindowId(),
                    ((const SfxBoolItem&) (rReq.GetArgs()->
                        Get(SID_EFFECT_WIN))).GetValue());
            else
                GetViewFrame()->ToggleChildWindow(
                    EffectChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate(SID_EFFECT_WIN);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_SLIDE_CHANGE_WIN:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(
                    SlideChangeChildWindow::GetChildWindowId(),
                    ((const SfxBoolItem&) (rReq.GetArgs()->
                        Get(SID_SLIDE_CHANGE_WIN))).GetValue());
            else
                GetViewFrame()->ToggleChildWindow(
                    SlideChangeChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate(SID_SLIDE_CHANGE_WIN);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_3D_WIN:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow( Svx3DChildWindow::GetChildWindowId(),
                                        ((const SfxBoolItem&) (rReq.GetArgs()->
                                        Get( SID_3D_WIN ))).GetValue());
            else
                GetViewFrame()->ToggleChildWindow( Svx3DChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate( SID_3D_WIN );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CONVERT_TO_3D_LATHE_FAST:
        {
            // Der Aufruf ist ausreichend. Die Initialisierung per Start3DCreation und CreateMirrorPolygons
            // ist nicht mehr noetig, falls der Parameter TRUE uebergeben wird. Dann wird sofort und
            // ohne Benutzereingriff ein gekippter Rotationskoerper mit einer Achse links neben dem
            // Umschliessenden Rechteck der slektierten Objekte gezeichnet.
            pDrView->EndTextEdit();
            if(GetActiveWindow())
                GetActiveWindow()->EnterWait();
            pDrView->End3DCreation(TRUE);
            Cancel();
            rReq.Ignore();
            if(GetActiveWindow())
                GetActiveWindow()->LeaveWait();
        }
        break;

        case SID_PRESENTATION_DLG:
        {
            pFuActual = new FuSlideShowDlg( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_CUSTOMSHOW_DLG:
        {
            pFuActual = new FuCustomShowDlg( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_EXPAND_PAGE:
        {
            pFuActual = new FuExpandPage( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_SUMMARY_PAGE:
        {
            pDrView->EndTextEdit();
            pFuActual = new FuSummaryPage( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            Cancel();
        }
        break;

        case SID_LAYER_DIALOG_WIN:
        {
            if ( rReq.GetArgs() )
            {
                GetViewFrame()->SetChildWindow(
                    LayerDialogChildWindow::GetChildWindowId(),
                    ((const SfxBoolItem&) (rReq.GetArgs()->
                        Get(SID_LAYER_DIALOG_WIN))).GetValue());
            }
            else
            {
                GetViewFrame()->ToggleChildWindow(
                    LayerDialogChildWindow::GetChildWindowId());
            }

            GetViewFrame()->GetBindings().Invalidate(SID_LAYER_DIALOG_WIN);
            Cancel();
            rReq.Ignore ();
        }
        break;

        default:
        {
            DBG_ASSERT( 0, "Slot ohne Funktion" );
            Cancel();
            rReq.Ignore ();
        }
        break;
    };
};

} // end of namespace sd
