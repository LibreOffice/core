/*************************************************************************
 *
 *  $RCSfile: undodraw.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:07 $
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

//------------------------------------------------------------------

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
#define _FILDLG_HXX
//#define _PRNDLG_HXX
#define _COLDLG_HXX
//#define _TABDLG_HXX
//#define _MENU_HXX
//#define _GDIMTF_HXX
//#define _POLY_HXX
//#define _ACCEL_HXX
//#define _GRAPH_HXX
#define _SOUND_HXX


#define SI_NOITEMS
//#define SI_NODRW
#define _SI_NOSBXCONTROLS
#define _SI_NOOTHERFORMS
#define _SI_NOCONTROL
#define _SI_NOSBXCONTROLS
#define _SIDLL_HXX

// SFX
#define _SFXAPPWIN_HXX
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

// INCLUDE ---------------------------------------------------------------

#include <svx/svdundo.hxx>

#include "undodraw.hxx"
#include "docsh.hxx"


// -----------------------------------------------------------------------

TYPEINIT1(ScUndoDraw, SfxUndoAction);

// -----------------------------------------------------------------------

ScUndoDraw::ScUndoDraw( SfxUndoAction* pUndo, ScDocShell* pDocSh ) :
    pDrawUndo( pUndo ),
    pDocShell( pDocSh )
{
}

__EXPORT ScUndoDraw::~ScUndoDraw()
{
    delete pDrawUndo;
}

void ScUndoDraw::ForgetDrawUndo()
{
    pDrawUndo = NULL;   // nicht loeschen (Draw-Undo muss dann von aussen gemerkt werden)
}

String __EXPORT ScUndoDraw::GetComment() const
{
    if (pDrawUndo)
        return pDrawUndo->GetComment();
    else
        return String();
}

String __EXPORT ScUndoDraw::GetRepeatComment(SfxRepeatTarget& rTarget) const
{
    if (pDrawUndo)
        return pDrawUndo->GetRepeatComment(rTarget);
    else
        return String();
}

USHORT __EXPORT ScUndoDraw::GetId() const
{
    if (pDrawUndo)
        return pDrawUndo->GetId();
    else
        return 0;
}

BOOL __EXPORT ScUndoDraw::IsLinked()
{
    if (pDrawUndo)
        return pDrawUndo->IsLinked();
    else
        return FALSE;
}

void __EXPORT ScUndoDraw::SetLinked( BOOL bIsLinked )
{
    if (pDrawUndo)
        pDrawUndo->SetLinked(bIsLinked);
}

BOOL  __EXPORT ScUndoDraw::Merge( SfxUndoAction* pNextAction )
{
    if (pDrawUndo)
        return pDrawUndo->Merge(pNextAction);
    else
        return FALSE;
}

void __EXPORT ScUndoDraw::Undo()
{
    if (pDrawUndo)
    {
        pDrawUndo->Undo();
        pDocShell->SetDrawModified();
    }
}

void __EXPORT ScUndoDraw::Redo()
{
    if (pDrawUndo)
    {
        pDrawUndo->Redo();
        pDocShell->SetDrawModified();
    }
}

void __EXPORT ScUndoDraw::Repeat(SfxRepeatTarget& rTarget)
{
    if (pDrawUndo)
        pDrawUndo->Repeat(rTarget);
}

BOOL __EXPORT ScUndoDraw::CanRepeat(SfxRepeatTarget& rTarget) const
{
    if (pDrawUndo)
        return pDrawUndo->CanRepeat(rTarget);
    else
        return FALSE;
}



