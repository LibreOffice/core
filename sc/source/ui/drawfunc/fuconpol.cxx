/*************************************************************************
 *
 *  $RCSfile: fuconpol.cxx,v $
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
//#define _TOOLBOX_HXX
//#define _STATUS_HXX ***
//#define _DIALOG_HXX
//#define _MSGBOX_HXX
//#define _SYSDLG_HXX
//#define _FILDLG_HXX
//#define _PRNDLG_HXX
#define _COLDLG_HXX
//#define _TABDLG_HXX
//#define _MENU_HXX
//#define _GDIMTF_HXX
//#define _POLY_HXX
//#define _ACCEL_HXX
//#define _GRAPH_HXX
#define _SOUND_HXX

//------------------------------------------------------------------------

#include <svx/svdview.hxx>
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

#include "fuconpol.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"

//  Pixelabstand zum Schliessen von Freihand-Zeichnungen

#ifndef CLOSE_PIXDIST
#define CLOSE_PIXDIST 5
#endif

//------------------------------------------------------------------------

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstPolygon::FuConstPolygon(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
                   SdrModel* pDoc, SfxRequest& rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstPolygon::~FuConstPolygon()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL __EXPORT FuConstPolygon::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuConstruct::MouseButtonDown(rMEvt);

    SdrViewEvent aVEvt;
    SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
    if (aVEvt.eEvent == SDREVENT_BEGTEXTEDIT)
    {
        // Texteingabe hier nicht zulassen
        aVEvt.eEvent = SDREVENT_BEGDRAGOBJ;
        pView->EnableExtendedMouseEventDispatcher(FALSE);
    }
    else
    {
        pView->EnableExtendedMouseEventDispatcher(TRUE);
    }

    if ( pView->MouseButtonDown(rMEvt, pWindow) )
        bReturn = TRUE;

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL __EXPORT FuConstPolygon::MouseMove(const MouseEvent& rMEvt)
{
    pView->MouseMove(rMEvt, pWindow);
    BOOL bReturn = FuConstruct::MouseMove(rMEvt);
    return bReturn;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL __EXPORT FuConstPolygon::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;
    BOOL bSimple = FALSE;

    SdrViewEvent aVEvt;
    SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONUP, aVEvt);

    pView->MouseButtonUp(rMEvt, pWindow);

    if (aVEvt.eEvent == SDREVENT_ENDCREATE)
    {
        bReturn = TRUE;
        bSimple = TRUE;         // Doppelklick nicht weiterreichen
    }

    BOOL bParent;
    if (bSimple)
        bParent = FuConstruct::SimpleMouseButtonUp(rMEvt);
    else
        bParent = FuConstruct::MouseButtonUp(rMEvt);

    return (bParent || bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL __EXPORT FuConstPolygon::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FuConstruct::KeyInput(rKEvt);

    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstPolygon::Activate()
{
    pView->EnableExtendedMouseEventDispatcher(TRUE);

    SdrObjKind eKind;

    switch (GetSlotID())
    {
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON_NOFILL:
        {
            eKind = OBJ_PLIN;
        }
        break;

        case SID_DRAW_POLYGON:
        case SID_DRAW_XPOLYGON:
        {
            eKind = OBJ_POLY;
        }
        break;

        case SID_DRAW_BEZIER_NOFILL:
        {
            eKind = OBJ_PATHLINE;
        }
        break;

        case SID_DRAW_BEZIER_FILL:
        {
            eKind = OBJ_PATHFILL;
        }
        break;

        case SID_DRAW_FREELINE_NOFILL:
        {
            eKind = OBJ_FREELINE;
        }
        break;

        case SID_DRAW_FREELINE:
        {
            eKind = OBJ_FREEFILL;
        }
        break;

        default:
        {
            eKind = OBJ_PATHLINE;
        }
        break;
    }

    pView->SetCurrentObj(eKind);

    pView->SetEditMode(SDREDITMODE_CREATE);

    FuConstruct::Activate();

    aNewPointer = Pointer( POINTER_DRAW_POLYGON );
    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer( aNewPointer );
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstPolygon::Deactivate()
{
    pView->SetEditMode(SDREDITMODE_EDIT);

    pView->EnableExtendedMouseEventDispatcher(FALSE);

    FuConstruct::Deactivate();

    pViewShell->SetActivePointer( aOldPointer );
}



