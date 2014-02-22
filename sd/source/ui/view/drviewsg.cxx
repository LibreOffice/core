/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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
#include "FrameView.hxx"
#include "drawview.hxx"
#include "fupoor.hxx"

namespace sd {

void DrawViewShell::ExecIMap( SfxRequest& rReq )
{
    
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
                    pSdrObj->AppendUserData( new SdIMapInfo( rImageMap ) );
                else
                    pIMapInfo->SetImageMap( rImageMap );

                GetDoc()->SetChanged( true );
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
        sal_uLong               nMarkCount = rMarkList.GetMarkCount();

        if ( nMarkCount == 1 )
        {
            const SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

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


void DrawViewShell::ExecOptionsBar( SfxRequest& rReq )
{
    
    if(HasCurrentFunction(SID_PRESENTATION))
        return;

    sal_Bool   bDefault = sal_False;
    sal_uInt16 nSlot = rReq.GetSlot();

    SdOptions* pOptions = SD_MOD()->GetSdOptions(GetDoc()->GetDocumentType());

    switch( nSlot )
    {
        case SID_SOLID_CREATE:
            pOptions->SetSolidDragging( !mpDrawView->IsSolidDragging() );
        break;


        
        case SID_GRID_VISIBLE: 
        {
            pOptions->SetGridVisible( !mpDrawView->IsGridVisible() );
        }
        break;

        case SID_GRID_USE:
        {
            pOptions->SetUseGridSnap( !mpDrawView->IsGridSnap() );
        }
        break;

        case SID_HELPLINES_VISIBLE: 
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

        
        
        WriteFrameViewData();

        mpFrameView->Update( pOptions );
        ReadFrameViewData( mpFrameView );

        Invalidate( nSlot );
        rReq.Done();
    }

}



void DrawViewShell::GetOptionsBarState( SfxItemSet& rSet )
{
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
    rSet.Put( SfxBoolItem( SID_PICK_THROUGH,
                mpDrawView->GetModel()->IsPickThroughTransparentTextFrames() ) );

    rSet.Put( SfxBoolItem( SID_DOUBLECLICK_TEXTEDIT, mpFrameView->IsDoubleClickTextEdit() ) );
    rSet.Put( SfxBoolItem( SID_CLICK_CHANGE_ROTATION, mpFrameView->IsClickChangeRotation() ) );
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
