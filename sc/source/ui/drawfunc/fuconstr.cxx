/*************************************************************************
 *
 *  $RCSfile: fuconstr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:56 $
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

//------------------------------------------------------------------------

// TOOLS
#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
#define _STACK_HXX
#define _QUEUE_HXX
#define _DYNARR_HXX
#define _TREELIST_HXX
#define _CACHESTR_HXX
#define _NEW_HXX
//#define _SHL_HXX
//#define _LINK_HXX
//#define _ERRCODE_HXX
//#define _GEN_HXX
//#define _FRACT_HXX
//#define _STRING_HXX
//#define _MTF_HXX
//#define _CONTNR_HXX
//#define _LIST_HXX
//#define _TABLE_HXX
#define _DYNARY_HXX
//#define _UNQIDX_HXX
#define _SVMEMPOOL_HXX
//#define _UNQID_HXX
//#define _DEBUG_HXX
//#define _DATE_HXX
//#define _TIME_HXX
//#define _DATETIME_HXX
//#define _INTN_HXX
//#define _WLDCRD_HXX
//#define _FSYS_HXX
//#define _STREAM_HXX
#define _CACHESTR_HXX
#define _SV_MULTISEL_HXX


//SV
//#define _CLIP_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX
//#define _COLOR_HXX
//#define _PAL_HXX
//#define _BITMAP_HXX
//#define _GDIOBJ_HXX
//#define _POINTR_HXX
//#define _ICON_HXX
//#define _IMAGE_HXX
//#define _KEYCOD_HXX
//#define _EVENT_HXX
#define _HELP_HXX
//#define _APP_HXX
//#define _MDIAPP_HXX
//#define _TIMER_HXX
//#define _METRIC_HXX
//#define _REGION_HXX
//#define _OUTDEV_HXX
//#define _SYSTEM_HXX
//#define _VIRDEV_HXX
//#define _JOBSET_HXX
//#define _PRINT_HXX
//#define _WINDOW_HXX
//#define _SYSWIN_HXX
//#define _WRKWIN_HXX
#define _MDIWIN_HXX
//#define _FLOATWIN_HXX
//#define _DOCKWIN_HXX
//#define _CTRL_HXX
//#define _SCRBAR_HXX
//#define _BUTTON_HXX
//#define _IMAGEBTN_HXX
//#define _FIXED_HXX
//#define _GROUP_HXX
//#define _EDIT_HXX
//#define _COMBOBOX_HXX
//#define _LSTBOX_HXX
//#define _SELENG_HXX
//#define _SPLIT_HXX
#define _SPIN_HXX
//#define _FIELD_HXX
//#define _MOREBTN_HXX
//#define _TOOLBOX_HXX
//#define _STATUS_HXX
//#define _DIALOG_HXX
//#define _MSGBOX_HXX
//#define _SYSDLG_HXX
//#define _PRNDLG_HXX
#define _COLDLG_HXX
//#define _TABDLG_HXX
//#define _GDIMTF_HXX
//#define _POLY_HXX
//#define _ACCEL_HXX
//#define _GRAPH_HXX
#define _SOUND_HXX

//svtools
#define _SCRWIN_HXX
#define _RULER_HXX
//#define _TABBAR_HXX
//#define _VALUESET_HXX
#define _STDMENU_HXX
//#define _STDCTRL_HXX
//#define _CTRLBOX_HXX
#define _CTRLTOOL_HXX
#define _EXTATTR_HXX
#define _FRM3D_HXX
#define _EXTATTR_HXX

//SVTOOLS
//#define _SVTREELIST_HXX ***
#define _FILTER_HXX
//#define _SVLBOXITM_HXX ***
//#define _SVTREEBOX_HXX ***
#define _SVICNVW_HXX
#define _SVTABBX_HXX

//sfxcore.hxx
//#define _SFXINIMGR_HXX ***
//#define _SFXCFGITEM_HXX
//#define _SFX_PRINTER_HXX
#define _SFXGENLINK_HXX
#define _SFXHINTPOST_HXX
//#define _SFXDOCINF_HXX ***
#define _SFXLINKHDL_HXX
//#define _SFX_PROGRESS_HXX

//sfxsh.hxx
//#define _SFX_SHELL_HXX
//#define _SFXAPP_HXX
//#define _SFX_BINDINGS_HXX
//#define _SFXDISPATCH_HXX
//#define _SFXMSG_HXX ***
//#define _SFXOBJFACE_HXX ***
//#define _SFXREQUEST_HXX
#define _SFXMACRO_HXX

// SFX
//#define _SFXAPPWIN_HXX ***
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX
//#define _SFXCTRLITEM_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

//------------------------------------------------------------------------

#include <svx/outliner.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdview.hxx>
#include <sfx2/dispatch.hxx>

#include "fuconstr.hxx"
#include "fudraw.hxx"
#include "tabvwsh.hxx"
#include "futext.hxx"
#include "sc.hrc"

//  Maximal erlaubte Mausbewegung um noch Drag&Drop zu starten
//! fusel,fuconstr,futext - zusammenfassen!
#define SC_MAXDRAGMOVE  3

//------------------------------------------------------------------------

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstruct::FuConstruct(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
                   SdrModel* pDoc, SfxRequest& rReq) :
    FuDraw(pViewSh, pWin, pView, pDoc, rReq)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstruct::~FuConstruct()
{
}

BYTE FuConstruct::Command(const CommandEvent& rCEvt)
{
    //  special code for non-VCL OS2/UNX removed

    return FuDraw::Command( rCEvt );
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL __EXPORT FuConstruct::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuDraw::MouseButtonDown(rMEvt);

    if ( pView->IsAction() )
    {
        if ( rMEvt.IsRight() )
            pView->BckAction();
        return TRUE;
    }

    aDragTimer.Start();

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        pWindow->CaptureMouse();

        SdrHdl* pHdl = pView->HitHandle(aMDPos, *pWindow);

        if ( pHdl != NULL || pView->IsMarkedHit(aMDPos) )
        {
            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, 1);
            bReturn = TRUE;
        }
        else if ( pView->HasMarkedObj() )
        {
            pView->UnmarkAll();
            bReturn = TRUE;
        }
    }

    bIsInDragMode = FALSE;

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL __EXPORT FuConstruct::MouseMove(const MouseEvent& rMEvt)
{
    FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        Point aOldPixel = pWindow->LogicToPixel( aMDPos );
        Point aNewPixel = rMEvt.GetPosPixel();
        if ( Abs( aOldPixel.X() - aNewPixel.X() ) > SC_MAXDRAGMOVE ||
             Abs( aOldPixel.Y() - aNewPixel.Y() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    Point aPix(rMEvt.GetPosPixel());
    Point aPnt( pWindow->PixelToLogic(aPix) );

    if ( pView->IsAction() )
    {
        ForceScroll(aPix);
        pView->MovAction(aPnt);
    }
    else
    {
        SdrHdl* pHdl=pView->HitHandle(aPnt, *pWindow);

        if ( pHdl != NULL )
        {
            pViewShell->SetActivePointer(pHdl->GetPointer());
        }
        else if ( pView->IsMarkedHit(aPnt) )
        {
            pViewShell->SetActivePointer(Pointer(POINTER_MOVE));
        }
        else
        {
            pViewShell->SetActivePointer( aNewPointer );
        }
    }
    return TRUE;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL __EXPORT FuConstruct::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = SimpleMouseButtonUp( rMEvt );

    //      Doppelklick auf Textobjekt? (->fusel)

    USHORT nClicks = rMEvt.GetClicks();
    if ( nClicks == 2 && rMEvt.IsLeft() )
    {
        if ( pView->HasMarkedObj() )
        {
            const SdrMarkList& rMarkList = pView->GetMarkList();
            if (rMarkList.GetMarkCount() == 1)
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                SdrObject* pObj = pMark->GetObj();

                //  #49458# bei Uno-Controls nicht in Textmodus
                if ( pObj->ISA(SdrTextObj) && !pObj->ISA(SdrUnoObj) )
                {
                    SFX_DISPATCHER().Execute(SID_DRAW_TEXT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);

                    // jetzt den erzeugten FuText holen und in den EditModus setzen
                    FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
                    if ( pPoor && pPoor->GetSlotID() == SID_DRAW_TEXT )  // hat keine RTTI
                    {
                        FuText* pText = (FuText*)pPoor;
                        Point aMousePixel = rMEvt.GetPosPixel();
                        pText->SetInEditMode( pObj, &aMousePixel );
                    }
                    bReturn = TRUE;
                }
            }
        }
    }

    FuDraw::MouseButtonUp(rMEvt);

    return bReturn;
}

//      SimpleMouseButtonUp - ohne Test auf Doppelklick

BOOL FuConstruct::SimpleMouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL    bReturn = TRUE;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    Point   aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( pView->IsDragObj() )
         pView->EndDragObj( rMEvt.IsMod1() );

    else if ( pView->IsMarkObj() )
        pView->EndMarkObj();

    else bReturn = FALSE;

    if ( !pView->IsAction() )
    {
        pWindow->ReleaseMouse();

        if ( !pView->HasMarkedObj() && rMEvt.GetClicks() < 2 )
        {
            pView->MarkObj(aPnt, -2, FALSE, rMEvt.IsMod1());

            if ( pView->HasMarkedObj() )
                SFX_DISPATCHER().Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            else
                SFX_DISPATCHER().Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL __EXPORT FuConstruct::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
            if ( pView->IsAction() )
            {
                pView->BrkAction();
                pWindow->ReleaseMouse();
                bReturn = TRUE;
            }
            else                            // Zeichenmodus beenden
            {
                SFX_DISPATCHER().Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            }
            break;

        case KEY_DELETE:
            pView->DeleteMarked();
            bReturn = TRUE;
            break;
    }

    if ( !bReturn )
    {
        bReturn = FuDraw::KeyInput(rKEvt);
    }

    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstruct::Activate()
{
    FuDraw::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstruct::Deactivate()
{
    FuDraw::Deactivate();
}




