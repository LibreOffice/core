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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawViewShell.hxx"
#include "ViewShellImplementation.hxx"

#include <svx/svxids.hrc>
#include <svx/imapdlg.hxx>
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

void DrawViewShell::ExecIMap( SfxRequest& rReq )
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if(HasCurrentFunction(SID_PRESENTATION) )
        return;

    if ( rReq.GetSlot() == SID_IMAP_EXEC )
    {
        SdrMark* pMark = mpDrawView->GetMarkedObjectList().GetMark(0);

        if ( pMark )
        {
            SdrObject*  pSdrObj = pMark->GetMarkedSdrObj();
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

void DrawViewShell::GetIMapState( SfxItemSet& rSet )
{
    sal_Bool bDisable = sal_True;

    if( GetViewFrame()->HasChildWindow( SvxIMapDlgChildWindow::GetChildWindowId() ) )
    {
        const SdrMarkList&  rMarkList = mpDrawView->GetMarkedObjectList();
        const SdrObject*    pObj = NULL;
        sal_uLong               nMarkCount = rMarkList.GetMarkCount();

        if ( nMarkCount == 1 )
        {
            pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

            SvxIMapDlg* pImageMapDialog = ViewShell::Implementation::GetImageMapDialog();
            if ( ( pObj->ISA( SdrGrafObj ) /*|| pObj->ISA( SdrOle2Obj )*/ )
                && pImageMapDialog!=NULL
                && ( pImageMapDialog->GetEditingObject() == (void*) pObj ) )
            {
                bDisable = sal_False;
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

    sal_Bool   bDefault = sal_False;
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
            pOptions->SetSnapHelplines( !mpDrawView->IsHlplSnap() );
        }
        break;

        case SID_HELPLINES_MOVE:
        {
            pOptions->SetDragStripes( !mpDrawView->IsDragStripes() );
        }
        break;


        case SID_SNAP_BORDER:
        {
            pOptions->SetSnapBorder( !mpDrawView->IsBordSnap() );
        }
        break;

        case SID_SNAP_FRAME:
        {
            pOptions->SetSnapFrame( !mpDrawView->IsOFrmSnap() );
        }
        break;

        case SID_SNAP_POINTS:
        {
            pOptions->SetSnapPoints( !mpDrawView->IsOPntSnap() );
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
                !mpDrawView->GetModel()->IsPickThroughTransparentTextFrames() );
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
            bDefault = sal_True;
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
    rSet.Put( SfxBoolItem( SID_HELPLINES_USE, mpDrawView->IsHlplSnap() ) );
    rSet.Put( SfxBoolItem( SID_HELPLINES_MOVE, mpDrawView->IsDragStripes() ) );

    rSet.Put( SfxBoolItem( SID_SNAP_BORDER, mpDrawView->IsBordSnap() ) );
    rSet.Put( SfxBoolItem( SID_SNAP_FRAME, mpDrawView->IsOFrmSnap() ) );
    rSet.Put( SfxBoolItem( SID_SNAP_POINTS, mpDrawView->IsOPntSnap() ) );

    rSet.Put( SfxBoolItem( SID_QUICKEDIT, mpDrawView->IsQuickTextEditMode() ) );
    rSet.Put( SfxBoolItem( SID_PICK_THROUGH, (sal_Bool)
                mpDrawView->GetModel()->IsPickThroughTransparentTextFrames() ) );

    rSet.Put( SfxBoolItem( SID_BIG_HANDLES, mpFrameView->IsBigHandles() ) );
    rSet.Put( SfxBoolItem( SID_DOUBLECLICK_TEXTEDIT, mpFrameView->IsDoubleClickTextEdit() ) );
    rSet.Put( SfxBoolItem( SID_CLICK_CHANGE_ROTATION, mpFrameView->IsClickChangeRotation() ) );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
