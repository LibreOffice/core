/*************************************************************************
 *
 *  $RCSfile: drviewsg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-28 18:05:00 $
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

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _IMAPDLG_HXX
#include <svx/imapdlg.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#include <sfx2/viewfrm.hxx>
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif

#pragma hdrstop

#include "app.hrc"

#include "drviewsh.hxx"
#include "drawdoc.hxx"
#include "fuslshow.hxx"
#include "imapinfo.hxx"
#include "sdmod.hxx"
#include "optsitem.hxx"
#include "frmview.hxx"
#include "drawview.hxx"


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdDrawViewShell::ExecIMap( SfxRequest& rReq )
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (pFuActual && pFuActual->GetSlotID() == SID_PRESENTATION)
        return;

    if ( rReq.GetSlot() == SID_IMAP_EXEC )
    {
        SdrMark* pMark = pDrView->GetMarkList().GetMark(0);

        if ( pMark )
        {
            SdrObject*  pSdrObj = pMark->GetObj();
            SvxIMapDlg* pDlg = SVXIMAPDLG();

            if ( pDlg->GetEditingObject() == (void*) pSdrObj )
            {
                const ImageMap& rImageMap = pDlg->GetImageMap();
                SdIMapInfo*     pIMapInfo = pDoc->GetIMapInfo( pSdrObj );

                if ( !pIMapInfo )
                    pSdrObj->InsertUserData( new SdIMapInfo( rImageMap ) );
                else
                    pIMapInfo->SetImageMap( rImageMap );
            }
        }
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdDrawViewShell::GetIMapState( SfxItemSet& rSet )
{
    BOOL bDisable = TRUE;

    if( GetViewFrame()->HasChildWindow( SvxIMapDlgChildWindow::GetChildWindowId() ) )
    {
        const SdrMarkList&  rMarkList = pDrView->GetMarkList();
        const SdrObject*    pObj = NULL;
        ULONG               nMarkCount = rMarkList.GetMarkCount();

        if ( nMarkCount == 1 )
        {
            pObj = rMarkList.GetMark( 0 )->GetObj();

            if ( ( pObj->ISA( SdrGrafObj ) || pObj->ISA( SdrOle2Obj ) ) &&
                 ( SVXIMAPDLG()->GetEditingObject() == (void*) pObj ) )
            {
                bDisable = FALSE;
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

void SdDrawViewShell::ExecOptionsBar( SfxRequest& rReq )
{
    // waehrend einer Diashow wird nichts ausgefuehrt!
    if (pFuActual && pFuActual->GetSlotID() == SID_PRESENTATION)
        return;

    BOOL   bDefault = FALSE;
    USHORT nSlot = rReq.GetSlot();
    FrameView* pFrameView = GetFrameView();

    SdOptions* pOptions = SD_MOD()->GetSdOptions(pDoc->GetDocumentType());

    switch( nSlot )
    {
        // Ersatzdarstellung-Optionen
        case SID_GRAPHIC_DRAFT:
            pOptions->SetExternGraphic( !pDrView->IsGrafDraft() );
        break;

        case SID_FILL_DRAFT:
            pOptions->SetOutlineMode( !pDrView->IsFillDraft() );
        break;

        case SID_TEXT_DRAFT:
            pOptions->SetNoText( !pDrView->IsTextDraft() );
        break;

        case SID_LINE_DRAFT:
            pOptions->SetHairlineMode( !pDrView->IsLineDraft() );
        break;

        case SID_HANDLES_DRAFT:
            pOptions->SetSolidMarkHdl( !pDrView->IsSolidMarkHdl() );
        break;

        case SID_SOLID_CREATE:
            pOptions->SetSolidDragging( !pDrView->IsSolidDragging() );
        break;


        // Raster- / Hilfslinien-Optionen
        case SID_GRID_VISIBLE: // noch nicht hier !
        {
            pOptions->SetGridVisible( !pDrView->IsGridVisible() );
        }
        break;

        case SID_GRID_USE:
        {
            pOptions->SetUseGridSnap( !pDrView->IsGridSnap() );
        }
        break;

        case SID_HELPLINES_VISIBLE: // noch nicht hier !
        {
            pOptions->SetHelplines( !pDrView->IsHlplVisible() );
        }
        break;

        case SID_HELPLINES_USE:
        {
            pOptions->SetSnapHelplines( !pDrView->IsHlplSnap() );
        }
        break;

        case SID_HELPLINES_MOVE:
        {
            pOptions->SetDragStripes( !pDrView->IsDragStripes() );
        }
        break;


        case SID_SNAP_BORDER:
        {
            pOptions->SetSnapBorder( !pDrView->IsBordSnap() );
        }
        break;

        case SID_SNAP_FRAME:
        {
            pOptions->SetSnapFrame( !pDrView->IsOFrmSnap() );
        }
        break;

        case SID_SNAP_POINTS:
        {
            pOptions->SetSnapPoints( !pDrView->IsOPntSnap() );
        }
        break;


        case SID_QUICKEDIT:
        {
            pOptions->SetQuickEdit( !pDrView->IsQuickTextEditMode() );
        }
        break;

        case SID_PICK_THROUGH:
        {
            pOptions->SetPickThrough(
                !pDrView->GetModel()->IsPickThroughTransparentTextFrames() );
        }
        break;

        case SID_BIG_HANDLES:
        {
            pOptions->SetBigHandles( !pFrameView->IsBigHandles() );
        }
        break;

        case SID_DOUBLECLICK_TEXTEDIT:
        {
            pOptions->SetDoubleClickTextEdit( !pFrameView->IsDoubleClickTextEdit() );
        }
        break;

        case SID_CLICK_CHANGE_ROTATION:
        {
            pOptions->SetClickChangeRotation( !pFrameView->IsClickChangeRotation() );
        }
        break;

        default:
            bDefault = TRUE;
        break;
    }

    if( !bDefault )
    {
        pOptions->StoreConfig();

        // Speichert die Konfiguration SOFORT
        // SFX_APP()->SaveConfiguration();
        WriteFrameViewData();

        //FrameView* pFrameView = pViewShell->GetFrameView(); schon oben
        pFrameView->Update( pOptions );
        ReadFrameViewData( pFrameView );

        Invalidate( nSlot );
        rReq.Done();
    }

}


/*************************************************************************
|*
|*  State-Methode der Optionsleiste
|*
\************************************************************************/

void SdDrawViewShell::GetOptionsBarState( SfxItemSet& rSet )
{
    FrameView* pFrameView = GetFrameView();

    rSet.Put( SfxBoolItem( SID_GRAPHIC_DRAFT, pDrView->IsGrafDraft() ) );
    rSet.Put( SfxBoolItem( SID_FILL_DRAFT, pDrView->IsFillDraft() ) );
    rSet.Put( SfxBoolItem( SID_TEXT_DRAFT, pDrView->IsTextDraft() ) );
    rSet.Put( SfxBoolItem( SID_LINE_DRAFT, pDrView->IsLineDraft() ) );
    rSet.Put( SfxBoolItem( SID_HANDLES_DRAFT, !pDrView->IsSolidMarkHdl() ) );
    rSet.Put( SfxBoolItem( SID_SOLID_CREATE, pDrView->IsSolidDragging() ) );

    rSet.Put( SfxBoolItem( SID_GRID_VISIBLE, pDrView->IsGridVisible() ) );
    rSet.Put( SfxBoolItem( SID_GRID_USE, pDrView->IsGridSnap() ) );
    rSet.Put( SfxBoolItem( SID_HELPLINES_VISIBLE, pDrView->IsHlplVisible() ) );
    rSet.Put( SfxBoolItem( SID_HELPLINES_USE, pDrView->IsHlplSnap() ) );
    rSet.Put( SfxBoolItem( SID_HELPLINES_MOVE, pDrView->IsDragStripes() ) );

    rSet.Put( SfxBoolItem( SID_SNAP_BORDER, pDrView->IsBordSnap() ) );
    rSet.Put( SfxBoolItem( SID_SNAP_FRAME, pDrView->IsOFrmSnap() ) );
    rSet.Put( SfxBoolItem( SID_SNAP_POINTS, pDrView->IsOPntSnap() ) );

    rSet.Put( SfxBoolItem( SID_QUICKEDIT, pDrView->IsQuickTextEditMode() ) );
    rSet.Put( SfxBoolItem( SID_PICK_THROUGH,
                pDrView->GetModel()->IsPickThroughTransparentTextFrames() ) );

    rSet.Put( SfxBoolItem( SID_BIG_HANDLES, pFrameView->IsBigHandles() ) );
    rSet.Put( SfxBoolItem( SID_DOUBLECLICK_TEXTEDIT, pFrameView->IsDoubleClickTextEdit() ) );
    rSet.Put( SfxBoolItem( SID_CLICK_CHANGE_ROTATION, pFrameView->IsClickChangeRotation() ) );
}



