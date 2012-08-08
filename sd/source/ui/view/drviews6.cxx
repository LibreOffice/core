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


#include "DrawViewShell.hxx"
#include <vcl/metaact.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdograf.hxx>
#include <svx/svxids.hrc>
#include <svx/fontwork.hxx>
#include <svx/bmpmask.hxx>
#include <svx/galbrws.hxx>
#include <svx/imapdlg.hxx>
#include <svx/xftsfit.hxx>
#include <svx/colrctrl.hxx>
#include <svx/f3dchild.hxx>
#include "optsitem.hxx"
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/tbxcustomshapes.hxx>
#include <avmedia/mediaplayer.hxx>

#include "app.hrc"
#include "strings.hrc"
#include "glob.hrc"

#include "app.hxx"
#include "animobjs.hxx"
#include "AnimationChildWindow.hxx"
#include "NavigatorChildWindow.hxx"
#include "LayerDialogChildWindow.hxx"
#include "sdresid.hxx"
#include "fupoor.hxx"
#include "fusldlg.hxx"
#include "drawdoc.hxx"
#include "fuexpand.hxx"
#include "fusumry.hxx"
#include "fucushow.hxx"
#include "drawview.hxx"
#include "FrameView.hxx"
#include "Window.hxx"
#include "DrawDocShell.hxx"
#include "sdabstdlg.hxx"
#include "framework/FrameworkHelper.hxx"

namespace sd {

/*************************************************************************
|*
|* SfxRequests fuer FontWork bearbeiten
|*
\************************************************************************/

void DrawViewShell::ExecFormText(SfxRequest& rReq)
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if(HasCurrentFunction(SID_PRESENTATION))
        return;

    CheckLineTo (rReq);

    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() &&
         mpDrawView && !mpDrawView->IsPresObjSelected() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();
        const SfxPoolItem* pItem;

        if ( mpDrawView->IsTextEdit() )
            mpDrawView->SdrEndTextEdit();

        if ( rSet.GetItemState(XATTR_FORMTXTSTDFORM, sal_True, &pItem) ==
             SFX_ITEM_SET &&
            ((const XFormTextStdFormItem*) pItem)->GetValue() != XFTFORM_NONE )
        {

            sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();

            SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*)GetViewFrame()->
                                        GetChildWindow(nId)->GetWindow();

            pDlg->CreateStdFormObj(*mpDrawView, *mpDrawView->GetSdrPageView(),
                                    rSet, *rMarkList.GetMark(0)->GetMarkedSdrObj(),
                                   ((const XFormTextStdFormItem*) pItem)->
                                   GetValue());

            if(HasCurrentFunction(SID_BEZIER_EDIT))
            {   // ggf. die richtige Editfunktion aktivieren
                GetViewFrame()->GetDispatcher()->Execute(SID_SWITCH_POINTEDIT,
                                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
        }
        else
            mpDrawView->SetAttributes(rSet);
    }
}

/*************************************************************************
|*
|* Statuswerte fuer FontWork zurueckgeben
|*
\************************************************************************/

void DrawViewShell::GetFormTextState(SfxItemSet& rSet)
{
    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
    const SdrObject* pObj = NULL;
    SvxFontWorkDialog* pDlg = NULL;

    sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();

    if ( GetViewFrame()->HasChildWindow(nId) )
        pDlg = (SvxFontWorkDialog*)(GetViewFrame()->GetChildWindow(nId)->GetWindow());

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    if ( pObj == NULL || !pObj->ISA(SdrTextObj) ||
        !((SdrTextObj*) pObj)->HasText() )
    {
// automatisches Auf/Zuklappen des FontWork-Dialog; erstmal deaktiviert

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
            pDlg->SetColorList(GetDoc()->GetColorList());

        SfxItemSet aSet( GetDoc()->GetPool() );
        mpDrawView->GetAttributes( aSet );
        rSet.Set( aSet );
    }
}

/*************************************************************************
|*
|* SfxRequests fuer Animator bearbeiten
|*
\************************************************************************/

void DrawViewShell::ExecAnimationWin( SfxRequest& rReq )
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (HasCurrentFunction(SID_PRESENTATION))
        return;

    CheckLineTo (rReq);

    sal_uInt16 nSId = rReq.GetSlot();

    switch( nSId )
    {
        case SID_ANIMATOR_INIT:
        case SID_ANIMATOR_ADD:
        case SID_ANIMATOR_CREATE:
        {
            AnimationWindow* pAnimWin;
            sal_uInt16 nId = AnimationChildWindow::GetChildWindowId();

            pAnimWin = static_cast<AnimationWindow*>(
                GetViewFrame()->GetChildWindow(nId)->GetWindow());

            if ( pAnimWin )
            {
                if( nSId == SID_ANIMATOR_ADD )
                    pAnimWin->AddObj( *mpDrawView );
                else if( nSId == SID_ANIMATOR_CREATE )
                    pAnimWin->CreateAnimObj( *mpDrawView );
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
    sal_uInt16 nValue;

    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
    sal_uLong nMarkCount = rMarkList.GetMarkCount();

    if( nMarkCount == 0 )
        nValue = 0;
    else if( nMarkCount > 1 )
        nValue = 3;
    else // 1 Objekt
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
        sal_uInt32 nInv = pObj->GetObjInventor();
        sal_uInt16 nId  = pObj->GetObjIdentifier();
        // 1 selektiertes Gruppenobjekt
        if( nInv == SdrInventor && nId == OBJ_GRUP )
            nValue = 3;
        else if( nInv == SdrInventor && nId == OBJ_GRAF ) // Anim. GIF ?
        {
            sal_uInt16 nCount = 0;

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
        sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();
        rSet.Put(SfxBoolItem(SID_FONTWORK, GetViewFrame()->HasChildWindow(nId)));
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_COLOR_CONTROL ) )
    {
        sal_uInt16 nId = SvxColorChildWindow::GetChildWindowId();
        rSet.Put(SfxBoolItem(SID_COLOR_CONTROL, GetViewFrame()->HasChildWindow(nId)));
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ANIMATION_OBJECTS ) )
    {
        sal_uInt16 nId = AnimationChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_ANIMATION_OBJECTS, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_NAVIGATOR ) )
    {
        sal_uInt16 nId = SID_NAVIGATOR;
        rSet.Put( SfxBoolItem( SID_NAVIGATOR, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_BMPMASK ) )
    {
        sal_uInt16 nId = SvxBmpMaskChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_BMPMASK, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_GALLERY ) )
    {
        sal_uInt16 nId = GalleryChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_GALLERY, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_IMAP ) )
    {
        sal_uInt16 nId = SvxIMapDlgChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_IMAP, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_LAYER_DIALOG_WIN ) )
    {
        sal_uInt16 nId = LayerDialogChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_LAYER_DIALOG_WIN, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_3D_WIN ) )
    {
        sal_uInt16 nId = Svx3DChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_3D_WIN, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_AVMEDIA_PLAYER ) )
    {
        sal_uInt16 nId = ::avmedia::MediaPlayer::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_AVMEDIA_PLAYER, GetViewFrame()->HasChildWindow( nId ) ) );
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
    if (HasCurrentFunction(SID_PRESENTATION))
        return;

    switch ( rReq.GetSlot() )
    {
        case ( SID_BMPMASK_PIPETTE ) :
        {
            mbPipette = ( (const SfxBoolItem&) ( rReq.GetArgs()->
                       Get( SID_BMPMASK_PIPETTE ) ) ).GetValue();
        }
        break;

        case ( SID_BMPMASK_EXEC ) :
        {
            SdrGrafObj* pObj = 0;
            if( mpDrawView && mpDrawView->GetMarkedObjectList().GetMarkCount() )
                pObj = dynamic_cast< SdrGrafObj* >( mpDrawView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj() );

            if ( pObj && !mpDrawView->IsTextEdit() )
            {
                SdrGrafObj* pNewObj = (SdrGrafObj*) pObj->Clone();
                sal_Bool        bCont = sal_True;

                if( pNewObj->IsLinkedGraphic() )
                {
                    QueryBox aQBox( (Window*) GetActiveWindow(), WB_YES_NO | WB_DEF_YES,
                                    String( SdResId( STR_RELEASE_GRAPHICLINK ) ) );

                    if( RET_YES == aQBox.Execute() )
                        pNewObj->ReleaseGraphicLink();
                    else
                    {
                        delete pNewObj;
                        bCont = sal_False;
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
                        SdrPageView* pPV = mpDrawView->GetSdrPageView();

                        pNewObj->SetEmptyPresObj( sal_False );
                        pNewObj->SetGraphic( ( (SvxBmpMask*) GetViewFrame()->GetChildWindow(
                                             SvxBmpMaskChildWindow::GetChildWindowId() )->GetWindow() )->
                                             Mask( pNewObj->GetGraphic() ) );

                        String aStr( mpDrawView->GetDescriptionOfMarkedObjects() );
                        aStr += (sal_Unicode)( ' ' ), aStr += String( SdResId( STR_EYEDROPPER ) );

                        mpDrawView->BegUndo( aStr );
                        mpDrawView->ReplaceObjectAtView( pObj, *pPV, pNewObj );
                        mpDrawView->EndUndo();
                    }
                }
            }
        }
        break;

        default:
        break;
    }
}

void DrawViewShell::GetBmpMaskState( SfxItemSet& rSet )
{
    const SdrMarkList&  rMarkList = mpDrawView->GetMarkedObjectList();
    const SdrObject*    pObj = NULL;
    sal_uInt16              nId = SvxBmpMaskChildWindow::GetChildWindowId();
    SvxBmpMask*         pDlg = NULL;
    sal_Bool                bEnable = sal_False;

    if ( GetViewFrame()->HasChildWindow( nId ) )
    {
        pDlg = (SvxBmpMask*) ( GetViewFrame()->GetChildWindow( nId )->GetWindow() );

        if ( pDlg->NeedsColorList() )
            pDlg->SetColorList( GetDoc()->GetColorList() );
    }

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    // valid graphic object?
    if( pObj && pObj->ISA( SdrGrafObj ) &&
        !( ((SdrGrafObj*) pObj)->IsEPS() || ((SdrGrafObj*) pObj)->IsRenderGraphic() ) &&
        !mpDrawView->IsTextEdit() )
    {
        bEnable = sal_True;
    }

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
    sal_uInt16 nSId = rReq.GetSlot();
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
            svx::ExtrusionBar::execute( mpDrawView, rReq, GetViewFrame()->GetBindings() );
            Cancel();
            rReq.Ignore ();
            break;

        case SID_FONTWORK_SHAPE:
        case SID_FONTWORK_SHAPE_TYPE:
        case SID_FONTWORK_ALIGNMENT:
        case SID_FONTWORK_SAME_LETTER_HEIGHTS:
        case SID_FONTWORK_CHARACTER_SPACING:
        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        case SID_FONTWORK_GALLERY_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_FLOATER:
        case SID_FONTWORK_ALIGNMENT_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_DIALOG:
            svx::FontworkBar::execute( mpDrawView, rReq, GetViewFrame()->GetBindings() );
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

        case SID_CUSTOM_ANIMATION_PANEL:
        {
            // Make the slide transition panel visible (expand it) in the
            // tool pane.
            framework::FrameworkHelper::Instance(GetViewShellBase())->RequestTaskPanel(
                framework::FrameworkHelper::msCustomAnimationTaskPanelURL);

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SLIDE_TRANSITIONS_PANEL:
        {
            // Make the slide transition panel visible (expand it) in the
            // tool pane.
            framework::FrameworkHelper::Instance(GetViewShellBase())->RequestTaskPanel(
                framework::FrameworkHelper::msSlideTransitionTaskPanelURL);

            Cancel();
            rReq.Done ();
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
            // ist nicht mehr noetig, falls der Parameter sal_True uebergeben wird. Dann wird sofort und
            // ohne Benutzereingriff ein gekippter Rotationskoerper mit einer Achse links neben dem
            // Umschliessenden Rechteck der slektierten Objekte gezeichnet.
            mpDrawView->SdrEndTextEdit();
            if(GetActiveWindow())
                GetActiveWindow()->EnterWait();
            mpDrawView->End3DCreation(sal_True);
            Cancel();
            rReq.Ignore();
            if(GetActiveWindow())
                GetActiveWindow()->LeaveWait();
        }
        break;

        case SID_PRESENTATION_DLG:
        {
            SetCurrentFunction( FuSlideShowDlg::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_REMOTE_DLG:
        {
             SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
             VclAbstractDialog* pDlg = pFact ? pFact->CreateRemoteDialog(GetActiveWindow()) : 0;
             if (pDlg)
                 pDlg->Execute();
        }
        break;

        case SID_CUSTOMSHOW_DLG:
        {
            SetCurrentFunction( FuCustomShowDlg::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_EXPAND_PAGE:
        {
            SetCurrentFunction( FuExpandPage::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_SUMMARY_PAGE:
        {
            mpDrawView->SdrEndTextEdit();
            SetCurrentFunction( FuSummaryPage::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_AVMEDIA_PLAYER:
        {
            GetViewFrame()->ToggleChildWindow( ::avmedia::MediaPlayer::GetChildWindowId() );
            GetViewFrame()->GetBindings().Invalidate( SID_AVMEDIA_PLAYER );
            Cancel();
            rReq.Ignore ();
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

        case SID_DISPLAY_MASTER_BACKGROUND:
        case SID_DISPLAY_MASTER_OBJECTS:
        {
            // Determine current page and toggle visibility of layers
            // associated with master page background or master page shapes.
            SdPage* pPage = GetActualPage();
            if (pPage != NULL
                && GetDoc() != NULL)
            {
                SetOfByte aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
                sal_uInt8 aLayerId;
                if (nSId == SID_DISPLAY_MASTER_BACKGROUND)
                    aLayerId = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), sal_False);
                else
                    aLayerId = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), sal_False);
                aVisibleLayers.Set(aLayerId, !aVisibleLayers.IsSet(aLayerId));
                pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
            }
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
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
