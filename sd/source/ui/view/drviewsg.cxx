/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawViewShell.hxx"
#include "ViewShellImplementation.hxx"

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _IMAPDLG_HXX
#include <svx/imapdlg.hxx>
#endif
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>


#include "app.hrc"

#include "drawdoc.hxx"
#include "slideshow.hxx"
#include "imapinfo.hxx"
#include "sdmod.hxx"
#include "optsitem.hxx"
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "drawview.hxx"

#include "fupoor.hxx"

namespace sd {

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawViewShell::ExecIMap( SfxRequest& rReq )
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if(HasCurrentFunction(SID_PRESENTATION) )
        return;

    if ( rReq.GetSlot() == SID_IMAP_EXEC )
    {
        SdrObject* pSdrObj = mpDrawView->getSelectedIfSingle();

        if(pSdrObj)
        {
            SvxIMapDlg* pDlg = ViewShell::Implementation::GetImageMapDialog();

            if ( pDlg->GetEditingObject() == (void*) pSdrObj )
            {
                const ImageMap& rImageMap = pDlg->GetImageMap();
                SdIMapInfo*     pIMapInfo = GetDoc()->GetIMapInfo( pSdrObj );

                if ( !pIMapInfo )
                    pSdrObj->InsertUserData( new SdIMapInfo( rImageMap ) );
                else
                    pIMapInfo->SetImageMap( rImageMap );

                GetDoc()->SetChanged( sal_True );
            }
        }
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawViewShell::GetIMapState( SfxItemSet& rSet )
{
    bool bDisable = true;

    if( GetViewFrame()->HasChildWindow( SvxIMapDlgChildWindow::GetChildWindowId() ) )
    {
        const SdrGrafObj* pObj = dynamic_cast< const SdrGrafObj* >(mpDrawView->getSelectedIfSingle());

        if(pObj)
        {
            SvxIMapDlg* pImageMapDialog = ViewShell::Implementation::GetImageMapDialog();

            if ( pImageMapDialog && ( pImageMapDialog->GetEditingObject() == (void*) pObj ) )
            {
                bDisable = false;
            }
        }
    }

    rSet.Put( SfxBoolItem( SID_IMAP_EXEC, bDisable ) );
}

/*************************************************************************
|*
|*  Execute-Methode der Optionsleiste
|*
\************************************************************************/

void DrawViewShell::ExecOptionsBar( SfxRequest& rReq )
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if(HasCurrentFunction(SID_PRESENTATION))
        return;

    bool   bDefault = false;
    sal_uInt16 nSlot = rReq.GetSlot();
    SdOptions* pOptions = SD_MOD()->GetSdOptions(GetDoc()->GetDocumentType());

    switch( nSlot )
    {
        case SID_HANDLES_DRAFT:
            pOptions->SetSolidMarkHdl( !mpDrawView->IsSolidMarkHdl() );
        break;

        case SID_SOLID_CREATE:
            pOptions->SetSolidDragging( !mpDrawView->IsSolidDragging() );
        break;


        // Raster- / Hilfslinien-Optionen
        case SID_GRID_VISIBLE: // noch nicht hier !
        {
            pOptions->SetGridVisible( !mpDrawView->IsGridVisible() );
        }
        break;

        case SID_GRID_USE:
        {
            pOptions->SetUseGridSnap( !mpDrawView->IsGridSnap() );
        }
        break;

        case SID_HELPLINES_VISIBLE: // noch nicht hier !
        {
            pOptions->SetHelplines( !mpDrawView->IsHlplVisible() );
        }
        break;

        case SID_HELPLINES_USE:
        {
            pOptions->SetSnapHelplines( !mpDrawView->IsHelplineSnap() );
        }
        break;

        case SID_HELPLINES_MOVE:
        {
            pOptions->SetDragStripes( !mpDrawView->IsDragStripes() );
        }
        break;


        case SID_SNAP_BORDER:
        {
            pOptions->SetSnapBorder( !mpDrawView->IsBorderSnap() );
        }
        break;

        case SID_SNAP_FRAME:
        {
            pOptions->SetSnapFrame( !mpDrawView->IsOFrameSnap() );
        }
        break;

        case SID_SNAP_POINTS:
        {
            pOptions->SetSnapPoints( !mpDrawView->IsOPointSnap() );
        }
        break;


        case SID_QUICKEDIT:
        {
            pOptions->SetQuickEdit( !mpDrawView->IsQuickTextEditMode() );
        }
        break;

        case SID_PICK_THROUGH:
        {
            pOptions->SetPickThrough(
                !mpDrawView->getSdrModelFromSdrView().IsPickThroughTransparentTextFrames() );
        }
        break;

        case SID_BIG_HANDLES:
        {
            pOptions->SetBigHandles( !mpFrameView->IsBigHandles() );
        }
        break;

        case SID_DOUBLECLICK_TEXTEDIT:
        {
            pOptions->SetDoubleClickTextEdit( !mpFrameView->IsDoubleClickTextEdit() );
        }
        break;

        case SID_CLICK_CHANGE_ROTATION:
        {
            pOptions->SetClickChangeRotation( !mpFrameView->IsClickChangeRotation() );
        }
        break;

        default:
            bDefault = true;
        break;
    }

    if( !bDefault )
    {
        pOptions->StoreConfig();

        // Speichert die Konfiguration SOFORT
        // SFX_APP()->SaveConfiguration();
        WriteFrameViewData();

        mpFrameView->Update( pOptions );
        ReadFrameViewData( mpFrameView );

        Invalidate( nSlot );
        rReq.Done();
    }

}


/*************************************************************************
|*
|*  State-Methode der Optionsleiste
|*
\************************************************************************/

void DrawViewShell::GetOptionsBarState( SfxItemSet& rSet )
{
    rSet.Put( SfxBoolItem( SID_HANDLES_DRAFT, !mpDrawView->IsSolidMarkHdl() ) );
    rSet.Put( SfxBoolItem( SID_SOLID_CREATE, mpDrawView->IsSolidDragging() ) );
    rSet.Put( SfxBoolItem( SID_GRID_VISIBLE, mpDrawView->IsGridVisible() ) );
    rSet.Put( SfxBoolItem( SID_GRID_USE, mpDrawView->IsGridSnap() ) );
    rSet.Put( SfxBoolItem( SID_HELPLINES_VISIBLE, mpDrawView->IsHlplVisible() ) );
    rSet.Put( SfxBoolItem( SID_HELPLINES_USE, mpDrawView->IsHelplineSnap() ) );
    rSet.Put( SfxBoolItem( SID_HELPLINES_MOVE, mpDrawView->IsDragStripes() ) );

    rSet.Put( SfxBoolItem( SID_SNAP_BORDER, mpDrawView->IsBorderSnap() ) );
    rSet.Put( SfxBoolItem( SID_SNAP_FRAME, mpDrawView->IsOFrameSnap() ) );
    rSet.Put( SfxBoolItem( SID_SNAP_POINTS, mpDrawView->IsOPointSnap() ) );

    rSet.Put( SfxBoolItem( SID_QUICKEDIT, mpDrawView->IsQuickTextEditMode() ) );
    rSet.Put( SfxBoolItem( SID_PICK_THROUGH, (bool)
                mpDrawView->getSdrModelFromSdrView().IsPickThroughTransparentTextFrames() ) );

    rSet.Put( SfxBoolItem( SID_BIG_HANDLES, mpFrameView->IsBigHandles() ) );
    rSet.Put( SfxBoolItem( SID_DOUBLECLICK_TEXTEDIT, mpFrameView->IsDoubleClickTextEdit() ) );
    rSet.Put( SfxBoolItem( SID_CLICK_CHANGE_ROTATION, mpFrameView->IsClickChangeRotation() ) );
}

} // end of namespace sd
