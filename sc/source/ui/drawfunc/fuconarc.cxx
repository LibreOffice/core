/*************************************************************************
 *
 *  $RCSfile: fuconarc.cxx,v $
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
//SVTOOLS
#define _SVTREELIST_HXX
#define _FILTER_HXX
#define _SVLBOXITM_HXX
#define _SVTREEBOX_HXX
#define _SVICNVW_HXX
#define _SVTABBX_HXX


#define _BASE_DLGS_HXX
#define _BIGINT_HXX
#define _CACHESTR_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _CTRLBOX_HXX
#define _CTRLTOOL_HXX
#define _DLGCFG_HXX
#define _DYNARR_HXX
#define _EXTATTR_HXX
//#define _FILDLG_HXX
#define _FILTER_HXX
#define _FONTDLG_HXX

//xout.hxx
//#define _XENUM_HXX
//#define _XPOLY_HXX
//#define _XATTR_HXH
#define _XOUTX_HXX
//#define _XPOOL_HXX
#define _XTABLE_HXX


//#define _SVDLAYER_HXX
//#define _SVDATTR_HXX
#define _SVDIO_HXX
#define _SVBOXITM_HXX
#define _SVDEC_HXX
#define _SVDXOUT_HXX
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
#define _SDR_NOUNDO
#define _SDR_NOXOUTDEV
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
//#define _SDR_NOVIEWS

#define _SVDRAG_HXX
#define _SVINCVW_HXX
#define _SV_MULTISEL_HXX
#define _SVRTV_HXX
#define _SVTABBX_HXX

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX

//------------------------------------------------------------------------

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

#include <svx/svdview.hxx>

#include "fuconarc.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"


//------------------------------------------------------------------------


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstArc::FuConstArc( ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
                   SdrModel* pDoc, SfxRequest& rReq )
    : FuConstruct( pViewSh, pWin, pView, pDoc, rReq )
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstArc::~FuConstArc()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL __EXPORT FuConstArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    BOOL bReturn = FuConstruct::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pWindow->CaptureMouse();
        pView->BegCreateObj( aPnt );
        bReturn = TRUE;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL __EXPORT FuConstArc::MouseMove( const MouseEvent& rMEvt )
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL __EXPORT FuConstArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    BOOL bReturn = FALSE;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        // Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pView->EndCreateObj( SDRCREATE_NEXTPOINT );
        bReturn = TRUE;
    }
/*
    else if ( pView->IsCreateObj() && rMEvt.IsRight() )
    {
        // Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pView->EndCreateObj( SDRCREATE_FORCEEND );
        bReturn = TRUE;
    }
*/
    return (FuConstruct::MouseButtonUp(rMEvt) || bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL __EXPORT FuConstArc::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstArc::Activate()
{
    SdrObjKind aObjKind;

    switch (aSfxRequest.GetSlot() )
    {
        case SID_DRAW_ARC:
            aNewPointer = Pointer( POINTER_DRAW_ARC );
            aObjKind = OBJ_CARC;
            break;

        case SID_DRAW_PIE:
            aNewPointer = Pointer( POINTER_DRAW_PIE );
            aObjKind = OBJ_SECT;
            break;

        case SID_DRAW_CIRCLECUT:
            aNewPointer = Pointer( POINTER_DRAW_CIRCLECUT );
            aObjKind = OBJ_CCUT;
            break;

        default:
            aNewPointer = Pointer( POINTER_CROSS );
            aObjKind = OBJ_CARC;
            break;
    }

    pView->SetCurrentObj( aObjKind );

    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer( aNewPointer );

    FuDraw::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstArc::Deactivate()
{
    FuDraw::Deactivate();
    pViewShell->SetActivePointer( aOldPointer );
}



