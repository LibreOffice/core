/*************************************************************************
 *
 *  $RCSfile: fupoor.cxx,v $
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
//#define _CLIP_HXX ***
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
//#define _SELENG_HXX ***
//#define _SPLIT_HXX
#define _SPIN_HXX
//#define _FIELD_HXX
//#define _MOREBTN_HXX ***
//#define _TOOLBOX_HXX ***
//#define _STATUS_HXX ***
//#define _DIALOG_HXX ***
//#define _MSGBOX_HXX ***
//#define _SYSDLG_HXX ***
//#define _PRNDLG_HXX ***
#define _COLDLG_HXX
#define _TABDLG_HXX
//#define _GDIMTF_HXX ***
//#define _POLY_HXX ***
#ifdef WIN
#define _ACCEL_HXX
#endif
//#define _GRAPH_HXX ***
#define _SOUND_HXX

//svtools
#define _SCRWIN_HXX
#define _RULER_HXX
#define _TABBAR_HXX
#define _VALUESET_HXX
#define _STDMENU_HXX
#define _STDCTRL_HXX
#define _CTRLBOX_HXX
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
//#define _SFXCFGITEM_HXX ***
//#define _SFX_PRINTER_HXX ***
#define _SFXGENLINK_HXX
#define _SFXHINTPOST_HXX
// #define _SFXDOCINF_HXX ***
#define _SFXLINKHDL_HXX
//#define _SFX_PROGRESS_HXX ***

//sfxsh.hxx
//#define _SFX_SHELL_HXX ***
//#define _SFXAPP_HXX ***
//#define _SFX_BINDINGS_HXX ***
#define _SFXDISPATCH_HXX
//#define _SFXMSG_HXX ***
//#define _SFXOBJFACE_HXX ***
//#define _SFXREQUEST_HXX ***
#define _SFXMACRO_HXX

// SFX
//#define _SFXAPPWIN_HXX ***
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX ***
#define _SFXCTRLITEM_HXX
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

//sfxdoc.hxx
//#define _SFX_OBJSH_HXX ***
//#define _SFX_CLIENTSH_HXX ***
//#define _SFXDOCINF_HXX ***
//#define _SFX_OBJFAC_HXX ***
#define _SFX_DOCFILT_HXX
//#define _SFXDOCFILE_HXX ***
//#define _VIEWFAC_HXX ***
#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX ***
#define _MDIFRM_HXX
#define _SFX_IPFRM_HXX
#define _SFX_INTERNO_HXX


//svdraw.hxx
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
//#define _SDR_NOUNDO
#define _SDR_NOXOUTDEV
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
//#define _SDR_NOVIEWS

//------------------------------------------------------------------------

#include <svx/outliner.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>

#include "fupoor.hxx"
#include "tabvwsh.hxx"
#include "drawview.hxx"
#include "detfunc.hxx"


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuPoor::FuPoor(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
               SdrModel* pDoc, SfxRequest& rReq) :
    pViewShell(pViewSh),
    pWindow(pWin),
    pView(pView),
    pDrDoc(pDoc),
    aSfxRequest(rReq),
    pDialog(NULL),
    bIsInDragMode(FALSE)
{
    aScrollTimer.SetTimeoutHdl( LINK(this, FuPoor, ScrollHdl) );
    aScrollTimer.SetTimeout(SELENG_AUTOREPEAT_INTERVAL);

    aDragTimer.SetTimeoutHdl( LINK(this, FuPoor, DragTimerHdl) );
    aDragTimer.SetTimeout(SELENG_DRAGDROP_TIMEOUT);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuPoor::~FuPoor()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();

    if (pDialog)
        delete pDialog;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuPoor::Activate()
{
    if (pDialog)
    {
        pDialog->Show();
    }
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuPoor::Deactivate()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();

    if (pDialog)
    {
        pDialog->Hide();
    }
}

/*************************************************************************
|*
|* Scrollen bei Erreichen des Fensterrandes; wird von
|* MouseMove aufgerufen
|*
\************************************************************************/

void FuPoor::ForceScroll(const Point& aPixPos)
{
    aScrollTimer.Stop();

    Size aSize = pWindow->GetSizePixel();
    short dx = 0, dy = 0;

    if ( aPixPos.X() <= 0              ) dx = -1;
    if ( aPixPos.X() >= aSize.Width()  ) dx =  1;
    if ( aPixPos.Y() <= 0              ) dy = -1;
    if ( aPixPos.Y() >= aSize.Height() ) dy =  1;

    ScViewData* pViewData = pViewShell->GetViewData();
    ScSplitPos eWhich = pViewData->GetActivePart();
    if ( dx > 0 && pViewData->GetHSplitMode() == SC_SPLIT_FIX && WhichH(eWhich) == SC_SPLIT_LEFT )
    {
        pViewShell->ActivatePart( ( eWhich == SC_SPLIT_TOPLEFT ) ?
                        SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT );
        dx = 0;
    }
    if ( dy > 0 && pViewData->GetVSplitMode() == SC_SPLIT_FIX && WhichV(eWhich) == SC_SPLIT_TOP )
    {
        pViewShell->ActivatePart( ( eWhich == SC_SPLIT_TOPLEFT ) ?
                        SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT );
        dy = 0;
    }

    if ( dx != 0 || dy != 0 )
    {
        ScrollStart();                          // Scrollaktion in abgeleiteter Klasse
        pViewShell->ScrollLines(2*dx, 4*dy);
        ScrollEnd();
        aScrollTimer.Start();
    }
}

/*************************************************************************
|*
|* Timer-Handler fuer Fensterscrolling
|*
\************************************************************************/

IMPL_LINK_INLINE_START( FuPoor, ScrollHdl, Timer *, pTimer )
{
#ifdef VCL
    Point aPosPixel = pWindow->GetPointerPosPixel();
#else
    Point aPosPixel = pWindow->ScreenToOutputPixel( Pointer::GetPosPixel() );
#endif
    MouseMove(MouseEvent(aPosPixel));
    return 0;
}
IMPL_LINK_INLINE_END( FuPoor, ScrollHdl, Timer *, pTimer )

/*************************************************************************
|*
|* String in Applikations-Statuszeile ausgeben
|*
\************************************************************************/

//  WriteStatus gibt's nicht mehr

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuPoor::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

    return(bReturn);
}

BYTE FuPoor::Command(const CommandEvent& rCEvt)
{
    if ( COMMAND_STARTDRAG == rCEvt.GetCommand() )
    {
        //!!! sollte Joe eigentlich machen:
        // nur, wenn im Outliner was selektiert ist, darf
        // Command TRUE zurueckliefern:

        OutlinerView* pOutView = pView->GetTextEditOutlinerView();

        if ( pOutView )
            return pOutView->HasSelection() ? pView->Command(rCEvt,pWindow) : SC_CMD_NONE;
        else
            return pView->Command(rCEvt,pWindow);
    }
    else
        return pView->Command(rCEvt,pWindow);
}

/*************************************************************************
|*
|* Cut object to clipboard
|*
\************************************************************************/

void FuPoor::DoCut()
{
    if (pView)
    {
//!     pView->DoCut(pWindow);
    }
}

/*************************************************************************
|*
|* Copy object to clipboard
|*
\************************************************************************/

void FuPoor::DoCopy()
{
    if (pView)
    {
//!     pView->DoCopy(pWindow);
    }
}

/*************************************************************************
|*
|* Paste object from clipboard
|*
\************************************************************************/

void FuPoor::DoPaste()
{
    if (pView)
    {
//!     pView->DoPaste(pWindow);
    }
}

/*************************************************************************
|*
|* Timer-Handler fuer Drag&Drop
|*
\************************************************************************/

IMPL_LINK( FuPoor, DragTimerHdl, Timer *, EMPTYARG )
{
    //  ExecuteDrag (und das damit verbundene Reschedule) direkt aus dem Timer
    //  aufzurufen, bringt die VCL-Timer-Verwaltung durcheinander, wenn dabei
    //  (z.B. im Drop) wieder ein Timer gestartet wird (z.B. ComeBack-Timer der
    //  DrawView fuer Solid Handles / ModelHasChanged) - der neue Timer laeuft
    //  dann um die Dauer des Drag&Drop zu spaet ab.
    //  Darum Drag&Drop aus eigenem Event:

    Application::PostUserEvent( LINK( this, FuPoor, DragHdl ) );
    return 0;
}

IMPL_LINK( FuPoor, DragHdl, void *, EMPTYARG )
{
    SdrHdl* pHdl = pView->HitHandle(aMDPos, *pWindow);

    if ( pHdl==NULL && pView->IsMarkedHit(aMDPos) )
    {
        pWindow->ReleaseMouse();
        bIsInDragMode = TRUE;

//      pView->BeginDrag(pWindow, aMDPos);
        pViewShell->GetScDrawView()->BeginDrag(pWindow, aMDPos);
    }
    return 0;
}


//  Detektiv-Linie

BOOL FuPoor::IsDetectiveHit( const Point& rLogicPos )
{
    SdrPageView* pPV = pView->GetPageViewPvNum(0);
    if (!pPV)
        return FALSE;

    BOOL bFound = FALSE;
    SdrObjListIter aIter( *pPV->GetObjList(), IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if (ScDetectiveFunc::IsNonAlienArrow( pObject ))
        {
            USHORT nHitLog = (USHORT) pWindow->PixelToLogic(
                                Size(pView->GetHitTolerancePixel(),0)).Width();
            if ( pObject->IsHit( rLogicPos, nHitLog ) )
                bFound = TRUE;
        }

        pObject = aIter.Next();
    }
    return bFound;
}

void FuPoor::StopDragTimer()
{
    if (aDragTimer.IsActive() )
        aDragTimer.Stop();
}




