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

#include <DrawViewShell.hxx>
#include <ViewShellImplementation.hxx>

#include <svx/svxids.hrc>
#include <svx/imapdlg.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdograf.hxx>
#include <svx/ImageMapInfo.hxx>
#include <officecfg/Office/Draw.hxx>
#include <officecfg/Office/Impress.hxx>

#include <app.hrc>

#include <drawdoc.hxx>
#include <sdmod.hxx>
#include <optsitem.hxx>
#include <FrameView.hxx>
#include <drawview.hxx>

namespace sd {

void DrawViewShell::ExecIMap( SfxRequest const & rReq )
{
    // during a slide show, nothing is executed!
    if(HasCurrentFunction(SID_PRESENTATION) )
        return;

    if ( rReq.GetSlot() != SID_IMAP_EXEC )
        return;

    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
    SdrMark* pMark = rMarkList.GetMark(0);

    if ( !pMark )
        return;

    SdrObject*  pSdrObj = pMark->GetMarkedSdrObj();
    SvxIMapDlg* pDlg = ViewShell::Implementation::GetImageMapDialog();

    if ( pDlg->GetEditingObject() == static_cast<void*>(pSdrObj) )
    {
        const ImageMap& rImageMap = pDlg->GetImageMap();
        SvxIMapInfo*     pIMapInfo = SvxIMapInfo::GetIMapInfo( pSdrObj );

        if ( !pIMapInfo )
            pSdrObj->AppendUserData( std::unique_ptr<SdrObjUserData>(new SvxIMapInfo( rImageMap )) );
        else
            pIMapInfo->SetImageMap( rImageMap );

        GetDoc()->SetChanged();
    }
}

void DrawViewShell::GetIMapState( SfxItemSet& rSet )
{
    bool bDisable = true;

    if( GetViewFrame()->HasChildWindow( SvxIMapDlgChildWindow::GetChildWindowId() ) )
    {
        const SdrMarkList&  rMarkList = mpDrawView->GetMarkedObjectList();

        if ( rMarkList.GetMarkCount() == 1 )
        {
            const SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

            SvxIMapDlg* pImageMapDialog = ViewShell::Implementation::GetImageMapDialog();
            if ( ( dynamic_cast< const SdrGrafObj *>( pObj ) !=  nullptr /*|| pObj->ISA( SdrOle2Obj )*/ )
                && pImageMapDialog!=nullptr
                && ( pImageMapDialog->GetEditingObject() == static_cast<void const *>(pObj) ) )
            {
                bDisable = false;
            }
        }
    }

    rSet.Put( SfxBoolItem( SID_IMAP_EXEC, bDisable ) );
}

void DrawViewShell::ExecOptionsBar( SfxRequest& rReq )
{
    // during a slide show, nothing is executed!
    if(HasCurrentFunction(SID_PRESENTATION))
        return;

    bool   bDefault = false;
    sal_uInt16 nSlot = rReq.GetSlot();

    SdOptions* pOptions = SD_MOD()->GetSdOptions(GetDoc()->GetDocumentType());
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());

    switch( nSlot )
    {
        case SID_SOLID_CREATE:
            pOptions->SetSolidDragging( !mpDrawView->IsSolidDragging() );
        break;

        // Grid- / Help lines option
        case SID_GRID_VISIBLE: // not here yet!
        {
            pOptions->SetGridVisible( !mpDrawView->IsGridVisible() );
        }
        break;

        case SID_GRID_USE:
        {
            pOptions->SetUseGridSnap( !mpDrawView->IsGridSnap() );
        }
        break;

        case SID_HELPLINES_VISIBLE: // not here yet!
        {
            if ( GetDoc()->GetDocumentType() == DocumentType::Impress )
                officecfg::Office::Impress::Layout::Display::Guide::set(!mpDrawView->IsHlplVisible(), batch);
            else
                officecfg::Office::Draw::Layout::Display::Guide::set(!mpDrawView->IsHlplVisible(), batch);
        }
        break;

        case SID_HELPLINES_USE:
        {
            if (GetDoc()->GetDocumentType() == DocumentType::Impress)
                officecfg::Office::Impress::Snap::Object::SnapLine::set(!mpDrawView->IsHlplSnap(), batch);
            else
                officecfg::Office::Draw::Snap::Object::SnapLine::set(!mpDrawView->IsHlplSnap(), batch);
        }
        break;

        case SID_HELPLINES_MOVE:
        {
            if ( GetDoc()->GetDocumentType() == DocumentType::Impress )
                officecfg::Office::Impress::Layout::Display::Guide::set(!mpDrawView->IsDragStripes(), batch);
            else
                officecfg::Office::Draw::Layout::Display::Guide::set(!mpDrawView->IsDragStripes(), batch);
        }
        break;

        case SID_SNAP_BORDER:
        {
            if ( GetDoc()->GetDocumentType() == DocumentType::Impress )
                officecfg::Office::Impress::Snap::Object::PageMargin::set( !mpDrawView->IsBordSnap(), batch );
            else
                officecfg::Office::Draw::Snap::Object::PageMargin::set( !mpDrawView->IsBordSnap(), batch );
        }
        break;

        case SID_SNAP_FRAME:
        {
            if ( GetDoc()->GetDocumentType() == DocumentType::Impress )
                officecfg::Office::Impress::Snap::Object::ObjectFrame::set(!mpDrawView->IsOFrmSnap(), batch);
            else
                officecfg::Office::Draw::Snap::Object::ObjectFrame::set(!mpDrawView->IsOFrmSnap(), batch);
        }
        break;

        case SID_SNAP_POINTS:
        {
            if ( GetDoc()->GetDocumentType() == DocumentType::Impress )
                officecfg::Office::Impress::Snap::Object::ObjectPoint::set( !mpDrawView->IsOPntSnap(), batch );
            else
                officecfg::Office::Draw::Snap::Object::ObjectPoint::set( !mpDrawView->IsOPntSnap(), batch );
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
                !mpDrawView->GetModel().IsPickThroughTransparentTextFrames() );
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

    if( bDefault )
        return;

    batch->commit();
    pOptions->StoreConfig();

    // Saves the configuration IMMEDIATELY
    // SfxGetpApp()->SaveConfiguration();
    WriteFrameViewData();

    mpFrameView->Update( pOptions );
    ReadFrameViewData( mpFrameView );

    Invalidate( nSlot );
    rReq.Done();

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
                mpDrawView->GetModel().IsPickThroughTransparentTextFrames() ) );

    rSet.Put( SfxBoolItem( SID_DOUBLECLICK_TEXTEDIT, mpFrameView->IsDoubleClickTextEdit() ) );
    rSet.Put( SfxBoolItem( SID_CLICK_CHANGE_ROTATION, mpFrameView->IsClickChangeRotation() ) );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
