/*************************************************************************
 *
 *  $RCSfile: drwlayer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-19 13:19:50 $
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
#include "core_pch.hxx"
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
#define _SV_MULTISEL_HXX

//SV
//#define _CLIP_HXX ***
//#define _CONFIG_HXX ***
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

#if defined  WIN
#define _MENUBTN_HXX
#endif

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
#define _SFXDOCINF_HXX
#define _SFXLINKHDL_HXX
//#define _SFX_PROGRESS_HXX

//sfxsh.hxx
//#define _SFX_SHELL_HXX
//#define _SFXAPP_HXX
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

//sfxdoc.hxx
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX
//#define _SFXDOCINF_HXX
//#define _SFX_OBJFAC_HXX
#define _SFX_DOCFILT_HXX
//#define _SFXDOCFILE_HXX ***
//define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX
//#define _MDIFRM_HXX ***
#define _SFX_IPFRM_HXX
//#define _SFX_INTERNO_HXX

//sfxdlg.hxx
//#define _SFXTABDLG_HXX
//#define _BASEDLGS_HXX ***
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
//#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
//#define _SFXDOCTEMPL_HXX ***
//#define _SFXDOCTDLG_HXX ***
//#define _SFX_TEMPLDLG_HXX ***
//#define _SFXNEW_HXX ***
#define _SFXDOCMAN_HXX
//#define _SFXDOCKWIN_HXX

//sfxitems.hxx
#define _SFX_WHMAP_HXX
//#define _ARGS_HXX ***
//#define _SFXPOOLITEM_HXX
//#define _SFXINTITEM_HXX
//#define _SFXENUMITEM_HXX
#define _SFXFLAGITEM_HXX
//#define _SFXSTRITEM_HXX
#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
//#define _SFXITEMPOOL_HXX
//#define _SFXITEMSET_HXX
#define _SFXITEMITER_HXX
#define _SFX_WHITER_HXX
#define _SFXPOOLCACH_HXX
//#define _AEITEM_HXX
#define _SFXRNGITEM_HXX
//#define _SFXSLSTITM_HXX
//#define _SFXSTYLE_HXX

//xout.hxx
//#define _XENUM_HXX
//#define _XPOLY_HXX
//#define _XATTR_HXX
//#define _XOUTX_HXX
//#define _XPOOL_HXX
//#define _XTABLE_HXX

//svdraw.hxx
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
//#define _SDR_NOOBJECTS
//#define _SDR_NOVIEWS

//#define SI_NOITEMS
//#define SI_NODRW
#define _SI_NOSBXCONTROLS
//#define _VCATTR_HXX
#define _VCONT_HXX
//#define _VCSBX_HXX
#define _SI_NOOTHERFORMS
#define _VCTRLS_HXX
//#define _VCDRWOBJ_HXX
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS
//#define _SIDLL_HXX ***

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

// neu
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
//#define _CONFIG_HXX ***
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
#define _FILDLG_HXX
//#define _PRNDLG_HXX
#define _COLDLG_HXX
//#define _TABDLG_HXX
//#define _MENU_HXX ***
//#define _GDIMTF_HXX
//#define _POLY_HXX
//#define _ACCEL_HXX
//#define _GRAPH_HXX
#define _SOUND_HXX





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
//#define _CONFIG_HXX ***
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

#if defined  WIN
#define _MENUBTN_HXX
#endif

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
#define _SFXDOCINF_HXX
#define _SFXLINKHDL_HXX
//#define _SFX_PROGRESS_HXX

//sfxsh.hxx
//#define _SFX_SHELL_HXX
//#define _SFXAPP_HXX
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


#define _SFXFILEDLG_HXX
#define _SFXIMGMGR_HXX
#define _SFXIPFRM_HXX
#define _SFX_MACRO_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXMULTISEL_HXX
#define _SFXMSG_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFX_MINFITEM_HXX
#define _SFXOBJFACE_HXX
#define _SFXOBJFAC_HXX
//#define _SFXREQUEST_HXX
#define _SFX_SAVEOPT_HXX
#define _SFXSTBITEM_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXCTRL_HXX
#define _SFXTBXMGR_HXX

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


// INCLUDE ---------------------------------------------------------------


#include <svx/objfac3d.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svditer.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/viewsh.hxx>
#include <so3/ipobj.hxx>
#include <so3/svstor.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/itempool.hxx>
#include <vcl/virdev.hxx>

#include "drwlayer.hxx"
#include "drawpage.hxx"
#include "global.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include "userdat.hxx"
#include "markdata.hxx"
#include "globstr.hrc"
#include "scmod.hxx"

#define DET_ARROW_OFFSET    1000

//  Abstand zur naechsten Zelle beim Loeschen (bShrink), damit der Anker
//  immer an der richtigen Zelle angezeigt wird
//#define SHRINK_DIST       3
//  und noch etwas mehr, damit das Objekt auch sichtbar in der Zelle liegt
#define SHRINK_DIST     25

// -----------------------------------------------------------------------
//
//  Das Anpassen der Detektiv-UserData muss zusammen mit den Draw-Undo's
//  in der SdrUndoGroup liegen, darum von SdrUndoAction abgeleitet:

class ScUndoObjData : public SdrUndoObj
{
private:
    ScTripel    aOldStt;
    ScTripel    aOldEnd;
    ScTripel    aNewStt;
    ScTripel    aNewEnd;
    BOOL        bHasNew;
public:
                ScUndoObjData( SdrObject* pObj, const ScTripel& rOS, const ScTripel& rOE,
                                                const ScTripel& rNS, const ScTripel& rNE );
                ~ScUndoObjData();

    virtual void     Undo();
    virtual void     Redo();
};

// -----------------------------------------------------------------------

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScTabDeletedHint, SfxHint);
TYPEINIT1(ScTabSizeChangedHint, SfxHint);

static ScDrawObjFactory* pFac = NULL;
static E3dObjFactory* pF3d = NULL;
static USHORT nInst = 0;

SvPersist* ScDrawLayer::pGlobalDrawPersist = NULL;

BOOL bDrawIsInUndo = FALSE;         //! Member

// -----------------------------------------------------------------------

ScUndoObjData::ScUndoObjData( SdrObject* pObj, const ScTripel& rOS, const ScTripel& rOE,
                                               const ScTripel& rNS, const ScTripel& rNE ) :
    SdrUndoObj( *pObj ),
    aOldStt( rOS ),
    aOldEnd( rOE ),
    aNewStt( rNS ),
    aNewEnd( rNE )
{
}

__EXPORT ScUndoObjData::~ScUndoObjData()
{
}

void __EXPORT ScUndoObjData::Undo()
{
    ScDrawObjData* pData = ((ScDrawLayer&)rMod).GetObjData( pObj );
    DBG_ASSERT(pData,"ScUndoObjData: Daten nicht da");
    if (pData)
    {
        pData->aStt = aOldStt;
        pData->aEnd = aOldEnd;
    }
}

void __EXPORT ScUndoObjData::Redo()
{
    ScDrawObjData* pData = ((ScDrawLayer&)rMod).GetObjData( pObj );
    DBG_ASSERT(pData,"ScUndoObjData: Daten nicht da");
    if (pData)
    {
        pData->aStt = aNewStt;
        pData->aEnd = aNewEnd;
    }
}

// -----------------------------------------------------------------------

ScTabDeletedHint::ScTabDeletedHint( USHORT nTabNo ) :
    nTab( nTabNo )
{
}

__EXPORT ScTabDeletedHint::~ScTabDeletedHint()
{
}

ScTabSizeChangedHint::ScTabSizeChangedHint( USHORT nTabNo ) :
    nTab( nTabNo )
{
}

__EXPORT ScTabSizeChangedHint::~ScTabSizeChangedHint()
{
}

// -----------------------------------------------------------------------

#define MAXMM   10000000

inline void TwipsToMM( long& nVal )
{
    nVal = (long) ( nVal * (CM_PER_TWIPS*1000.0) );
}

// -----------------------------------------------------------------------


ScDrawLayer::ScDrawLayer( ScDocument* pDocument, const String& rName ) :
    FmFormModel( SvtPathOptions().GetPalettePath(),
                 NULL,                          // SfxItemPool* Pool
                pGlobalDrawPersist ?
                    pGlobalDrawPersist :
                    pDocument->GetDocumentShell()  // SvPersist*   pPers
                                                // VCItemPool*  pVCPool=NULL
               ),
    aName( rName ),
    pDoc( pDocument ),
    pUndoGroup( NULL ),
    bRecording( FALSE ),
    bAdjustEnabled( TRUE )
{
    pGlobalDrawPersist = NULL;          // nur einmal benutzen

    SetObjectShell( pDocument->GetDocumentShell() );
    SetSwapGraphics(TRUE);
//  SetSwapAsynchron(TRUE);     // an der View

    SetScaleUnit(MAP_100TH_MM);
    SfxItemPool& rPool = GetItemPool();
    rPool.SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    rPool.FreezeIdRanges();                         // der Pool wird auch direkt verwendet

    SdrLayerAdmin& rAdmin = GetLayerAdmin();
    rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("vorne")),    SC_LAYER_FRONT);
    rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("hinten")),   SC_LAYER_BACK);
    rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("intern")),   SC_LAYER_INTERN);
    rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Controls")), SC_LAYER_CONTROLS);
    // "Controls" is new - must also be created when loading

    //  Link fuer URL-Fields setzen
    ScModule* pScMod = SC_MOD();
    Outliner& rOutliner = GetDrawOutliner();
    rOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );

    Outliner& rHitOutliner = GetHitTestOutliner();
    rHitOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );

    //  URL-Buttons haben keinen Handler mehr, machen alles selber

    if( !nInst++ )
    {
        pFac = new ScDrawObjFactory;
        pF3d = new E3dObjFactory;
    }
}

__EXPORT ScDrawLayer::~ScDrawLayer()
{
    Clear();

    delete pUndoGroup;
    if( !--nInst )
    {
        delete pFac, pFac = NULL;
        delete pF3d, pF3d = NULL;
    }
}

SdrPage* __EXPORT ScDrawLayer::AllocPage(FASTBOOL bMasterPage)
{
//  StarBASIC* pBasic = pDoc->GetDocumentShell()->GetBasicManager()->GetLib(0);

    StarBASIC* pBasic = NULL;
    SfxObjectShell* pDocSh = pDoc->GetDocumentShell();
    if ( pDocSh /* && pDocSh->HasSbxObject() ??? */ )
        pBasic = pDocSh->GetBasic();

    ScDrawPage* pPage = new ScDrawPage( *this, pBasic, bMasterPage );
    return pPage;
}

BOOL ScDrawLayer::HasObjects() const
{
    BOOL bFound = FALSE;

    USHORT nCount = GetPageCount();
    for (USHORT i=0; i<nCount && !bFound; i++)
        if (GetPage(i)->GetObjCount())
            bFound = TRUE;

    return bFound;
}

void ScDrawLayer::UpdateBasic()
{
    SfxObjectShell* pDocSh = pDoc->GetDocumentShell();
    StarBASIC* pBasic = pDocSh ? pDocSh->GetBasic() : 0;

    USHORT nCount = GetPageCount();
    for (USHORT i=0; i<nCount; i++)
        ((ScDrawPage*) GetPage(i))->SetBasic(pBasic);
}

SdrModel* __EXPORT ScDrawLayer::AllocModel() const
{
    return new ScDrawLayer( pDoc, aName );
}

Window* __EXPORT ScDrawLayer::GetCurDocViewWin()
{
    SfxViewShell* pViewSh = SfxViewShell::Current();
    SfxObjectShell* pObjSh = pDoc->GetDocumentShell();

    if (pViewSh && pViewSh->GetObjectShell() == pObjSh)
        return pViewSh->GetWindow();

    return NULL;
}

void ScDrawLayer::ScAddPage( USHORT nTab )
{
    if (bDrawIsInUndo)
        return;

    ScDrawPage* pPage = (ScDrawPage*)AllocPage( FALSE );
    InsertPage(pPage, nTab);
    if (bRecording)
        AddCalcUndo(new SdrUndoNewPage(*pPage));
}

void ScDrawLayer::ScRemovePage( USHORT nTab )
{
    if (bDrawIsInUndo)
        return;

    Broadcast( ScTabDeletedHint( nTab ) );
    if (bRecording)
    {
        SdrPage* pPage = GetPage(nTab);
        AddCalcUndo(new SdrUndoDelPage(*pPage));        // Undo-Action wird Owner der Page
        RemovePage( nTab );                         // nur austragen, nicht loeschen
    }
    else
        DeletePage( nTab );                         // einfach weg damit
}

void ScDrawLayer::ScRenamePage( USHORT nTab, const String& rNewName )
{
    ScDrawPage* pPage = (ScDrawPage*) GetPage(nTab);
    if (pPage)
        pPage->SetName(rNewName);
}

void ScDrawLayer::ScMovePage( USHORT nOldPos, USHORT nNewPos )
{
    MovePage( nOldPos, nNewPos );
}

void ScDrawLayer::ScCopyPage( USHORT nOldPos, USHORT nNewPos, BOOL bAlloc )
{
    //! remove argument bAlloc (always FALSE)

    if (bDrawIsInUndo)
        return;

    SdrPage* pOldPage = GetPage(nOldPos);
    SdrPage* pNewPage = bAlloc ? AllocPage(FALSE) : GetPage(nNewPos);

    // kopieren

    if (pOldPage && pNewPage)
    {
        SdrObjListIter aIter( *pOldPage, IM_FLAT );
        SdrObject* pOldObject = aIter.Next();
        while (pOldObject)
        {
            SdrObject* pNewObject = pOldObject->Clone( pNewPage, this );
            pNewObject->NbcMove(Size(0,0));
            pNewPage->InsertObject( pNewObject );
            if (bRecording)
                AddCalcUndo( new SdrUndoInsertObj( *pNewObject ) );

            pOldObject = aIter.Next();
        }
    }

    if (bAlloc)
        InsertPage(pNewPage, nNewPos);
}

inline BOOL IsInBlock( const ScTripel& rPos, USHORT nCol1,USHORT nRow1, USHORT nCol2,USHORT nRow2 )
{
    return rPos.nCol >= nCol1 && rPos.nCol <= nCol2 &&
           rPos.nRow >= nRow1 && rPos.nRow <= nRow2;
}

void ScDrawLayer::MoveCells( USHORT nTab, USHORT nCol1,USHORT nRow1, USHORT nCol2,USHORT nRow2,
                                short nDx,short nDy )
{
    SdrPage* pPage = GetPage(nTab);
    DBG_ASSERT(pPage,"Page nicht gefunden");
    if (!pPage)
        return;

    ULONG nCount = pPage->GetObjCount();
    for ( ULONG i = 0; i < nCount; i++ )
    {
        SdrObject* pObj = pPage->GetObj( i );
        ScDrawObjData* pData = GetObjData( pObj );
        if( pData )
        {
            ScTripel aOldStt = pData->aStt;
            ScTripel aOldEnd = pData->aEnd;
            BOOL bChange = FALSE;
            if ( pData->bValidStart && IsInBlock( pData->aStt, nCol1,nRow1, nCol2,nRow2 ) )
            {
                pData->aStt.nCol += nDx;
                pData->aStt.nRow += nDy;
                bChange = TRUE;
            }
            if ( pData->bValidEnd && IsInBlock( pData->aEnd, nCol1,nRow1, nCol2,nRow2 ) )
            {
                pData->aEnd.nCol += nDx;
                pData->aEnd.nRow += nDy;
                bChange = TRUE;
            }
            if (bChange)
            {
                if ( pObj->ISA(SdrRectObj) && pData->bValidStart && pData->bValidEnd )
                {
                    PutInOrder( pData->aStt.nCol, pData->aEnd.nCol );
                    PutInOrder( pData->aStt.nRow, pData->aEnd.nRow );
                }
                AddCalcUndo( new ScUndoObjData( pObj, aOldStt, aOldEnd, pData->aStt, pData->aEnd ) );
                RecalcPos( pObj, pData );
            }
        }
    }
}

void ScDrawLayer::SetPageSize( USHORT nPageNo, const Size& rSize )
{
    SdrPage* pPage = GetPage(nPageNo);
    if (pPage)
    {
        if ( rSize != pPage->GetSize() )
        {
            pPage->SetSize( rSize );
            Broadcast( ScTabSizeChangedHint( nPageNo ) );   // SetWorkArea() an den Views
        }

        // Detektivlinien umsetzen (an neue Hoehen/Breiten anpassen)
        //  auch wenn Groesse gleich geblieben ist
        //  (einzelne Zeilen/Spalten koennen geaendert sein)

        ULONG nCount = pPage->GetObjCount();
        for ( ULONG i = 0; i < nCount; i++ )
        {
            SdrObject* pObj = pPage->GetObj( i );
            ScDrawObjData* pData = GetObjData( pObj );
            if( pData )
                RecalcPos( pObj, pData );
        }
    }
}

void ScDrawLayer::RecalcPos( SdrObject* pObj, ScDrawObjData* pData )
{
    BOOL bArrow = ( pObj->IsPolyObj() && pObj->GetPointCount()==2 );    // Pfeil ?
    BOOL bCircle = ( pObj->ISA(SdrCircObj) );                           // Kreis (Gueltigkeit)
    BOOL bCaption = ( pObj->ISA(SdrCaptionObj) );                       // Notiz

    if (bCaption)
    {
        SdrCaptionObj* pCaptObj = (SdrCaptionObj*) pObj;

        USHORT nCol = pData->aStt.nCol;
        USHORT nRow = pData->aStt.nRow;
        USHORT nTab = pData->aStt.nTab;
        Point aPos( pDoc->GetColOffset( nCol+1, nTab ),
                    pDoc->GetRowOffset( nRow, nTab ) );
        TwipsToMM( aPos.X() );
        TwipsToMM( aPos.Y() );
        aPos.X() -= 10;

        Point aOldTail = pCaptObj->GetTailPos();
        if ( aOldTail != aPos )
        {
            pCaptObj->SetTailPos(aPos);

            //  Rest in gleichem Abstand mitverschieben

            Rectangle aOldLogic = pObj->GetLogicRect();
            long nDiffX = aOldLogic.Left() - aOldTail.X();
            long nDiffY = aOldLogic.Top() - aOldTail.Y();
            Point aNewStart( aPos.X() + nDiffX, aPos.Y() + nDiffY );
            if ( aNewStart.X() < 0 ) aNewStart.X() = 0;
            if ( aNewStart.Y() < 0 ) aNewStart.Y() = 0;
            Rectangle aNewLogic( aNewStart, aOldLogic.GetSize() );

            if ( aNewLogic != aOldLogic )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                pObj->SetLogicRect(aNewLogic);
            }
        }
    }
    else if (bCircle)                   // Kreis (Gueltigkeit)
    {
        USHORT nCol = pData->aStt.nCol;
        USHORT nRow = pData->aStt.nRow;
        USHORT nTab = pData->aStt.nTab;
        Point aPos( pDoc->GetColOffset( nCol, nTab ), pDoc->GetRowOffset( nRow, nTab ) );
        TwipsToMM( aPos.X() );
        TwipsToMM( aPos.Y() );

        //  Berechnung und Werte wie in detfunc.cxx

        Size aSize( (long) ( pDoc->GetColWidth(nCol, nTab) * HMM_PER_TWIPS ),
                    (long) ( pDoc->GetRowHeight(nRow, nTab) * HMM_PER_TWIPS ) );
        Rectangle aRect( aPos, aSize );
        aRect.Left()    -= 250;
        aRect.Right()   += 250;
        aRect.Top()     -= 70;
        aRect.Bottom()  += 70;

        if ( pObj->GetLogicRect() != aRect )
        {
            if (bRecording)
                AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
            pObj->SetLogicRect(aRect);
        }
    }
    else if (bArrow)                    // Pfeil
    {
        //! nicht mehrere Undos fuer ein Objekt erzeugen (hinteres kann dann weggelassen werden)

        if( pData->bValidStart )
        {
            Point aPos(
                pDoc->GetColOffset( pData->aStt.nCol, pData->aStt.nTab ),
                pDoc->GetRowOffset( pData->aStt.nRow, pData->aStt.nTab ) );
            if( !( pDoc->GetColFlags( pData->aStt.nCol, pData->aStt.nTab )
                 & CR_HIDDEN ) )
                aPos.X() += pDoc->GetColWidth( pData->aStt.nCol, pData->aStt.nTab ) / 4;
            if( !( pDoc->GetRowFlags( pData->aStt.nRow, pData->aStt.nTab )
                 & CR_HIDDEN ) )
                aPos.Y() += pDoc->GetRowHeight( pData->aStt.nRow, pData->aStt.nTab ) / 2;
            TwipsToMM( aPos.X() );
            TwipsToMM( aPos.Y() );
            if ( pObj->GetPoint(0) != aPos )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                pObj->SetPoint( aPos, 0 );
            }

            if( !pData->bValidEnd )
            {
                Point aEndPos( aPos.X() + DET_ARROW_OFFSET, aPos.Y() - DET_ARROW_OFFSET );
                if (aEndPos.Y() < 0)
                    aEndPos.Y() += 2*DET_ARROW_OFFSET;
                if ( pObj->GetPoint(1) != aEndPos )
                {
                    if (bRecording)
                        AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                    pObj->SetPoint( aEndPos, 1 );
                }
            }
        }
        if( pData->bValidEnd )
        {
            Point aPos(
                pDoc->GetColOffset( pData->aEnd.nCol, pData->aEnd.nTab ),
                pDoc->GetRowOffset( pData->aEnd.nRow, pData->aEnd.nTab ) );
            if( !( pDoc->GetColFlags( pData->aEnd.nCol, pData->aEnd.nTab )
                 & CR_HIDDEN ) )
                aPos.X() += pDoc->GetColWidth( pData->aEnd.nCol, pData->aEnd.nTab ) / 4;
            if( !( pDoc->GetRowFlags( pData->aEnd.nRow, pData->aEnd.nTab )
                 & CR_HIDDEN ) )
                aPos.Y() += pDoc->GetRowHeight( pData->aEnd.nRow, pData->aEnd.nTab ) / 2;
            TwipsToMM( aPos.X() );
            TwipsToMM( aPos.Y() );
            if ( pObj->GetPoint(1) != aPos )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                pObj->SetPoint( aPos, 1 );
            }

            if( !pData->bValidStart )
            {
                Point aStartPos( aPos.X() - DET_ARROW_OFFSET, aPos.Y() - DET_ARROW_OFFSET );
                if (aStartPos.X() < 0)
                    aStartPos.X() += 2*DET_ARROW_OFFSET;
                if (aStartPos.Y() < 0)
                    aStartPos.Y() += 2*DET_ARROW_OFFSET;
                if ( pObj->GetPoint(0) != aStartPos )
                {
                    if (bRecording)
                        AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                    pObj->SetPoint( aStartPos, 0 );
                }
            }
        }
    }
    else                                // Referenz-Rahmen
    {
        DBG_ASSERT( pData->bValidStart, "RecalcPos: kein Start" );
        Point aPos(
            pDoc->GetColOffset( pData->aStt.nCol, pData->aStt.nTab ),
            pDoc->GetRowOffset( pData->aStt.nRow, pData->aStt.nTab ) );
        TwipsToMM( aPos.X() );
        TwipsToMM( aPos.Y() );

        if( pData->bValidEnd )
        {
            Point aEnd(
                pDoc->GetColOffset( pData->aEnd.nCol+1, pData->aEnd.nTab ),
                pDoc->GetRowOffset( pData->aEnd.nRow+1, pData->aEnd.nTab ) );
            TwipsToMM( aEnd.X() );
            TwipsToMM( aEnd.Y() );

            Rectangle aNew( aPos, aEnd );
            if ( pObj->GetLogicRect() != aNew )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                pObj->SetLogicRect(aNew);
            }
        }
        else
            if ( pObj->GetRelativePos() != aPos )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                pObj->SetRelativePos( aPos );
            }
    }

}

void ScDrawLayer::Load( SvStream& rStream )
{
    bRecording = FALSE;
    DELETEZ(pUndoGroup);

    ScReadHeader aHdr( rStream );
    while (aHdr.BytesLeft())
    {
        USHORT nID;
        rStream >> nID;
        switch (nID)
        {
            case SCID_DRAWPOOL:
                {
                    ScReadHeader aPoolHdr( rStream );
                    GetItemPool().Load( rStream );              //! in Pool-Stream ?
                }
                break;
            case SCID_DRAWMODEL:
                {
                    ScReadHeader aDrawHdr( rStream );
                    rStream >> *this;

                    //  Control-Layer ist nicht in alten Dateien
                    SdrLayerAdmin& rAdmin = GetLayerAdmin();
                    const SdrLayer* pLayer = rAdmin.GetLayerPerID(SC_LAYER_CONTROLS);
                    if (!pLayer)
                        rAdmin.NewLayer(
                            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Controls")),
                            SC_LAYER_CONTROLS);
                }
                break;
            default:
                {
                    DBG_ERROR("unbekannter Sub-Record in ScDrawLayer::Load");
                    ScReadHeader aDummyHdr( rStream );
                }
        }
    }

    GetItemPool().LoadCompleted();
}

void ScDrawLayer::Store( SvStream& rStream ) const
{
    ScWriteHeader aHdr( rStream );

    const_cast<ScDrawLayer*>(this)->PrepareStore();     // non-const

    {
        rStream << (USHORT) SCID_DRAWPOOL;
        ScWriteHeader aPoolHdr( rStream );
        GetItemPool().Store( rStream );             //! in Pool-Stream ?
    }

    {
        rStream << (USHORT) SCID_DRAWMODEL;
        ScWriteHeader aDrawHdr( rStream );
        rStream << *this;
    }
}

BOOL ScDrawLayer::GetPrintArea( ScRange& rRange, BOOL bSetHor, BOOL bSetVer ) const
{
    USHORT nTab = rRange.aStart.Tab();
    DBG_ASSERT( rRange.aEnd.Tab() == nTab, "GetPrintArea: Tab unterschiedlich" );

    BOOL bAny = FALSE;
    long nEndX = 0;
    long nEndY = 0;
    long nStartX = LONG_MAX;
    long nStartY = LONG_MAX;
    USHORT i;

    // Grenzen ausrechnen

    if (!bSetHor)
    {
        nStartX = 0;
        USHORT nStartCol = rRange.aStart.Col();
        for (i=0; i<nStartCol; i++)
            nStartX +=pDoc->GetColWidth(i,nTab);
        nEndX = nStartX;
        USHORT nEndCol = rRange.aEnd.Col();
        for (i=nStartCol; i<=nEndCol; i++)
            nEndX += pDoc->GetColWidth(i,nTab);
        nStartX = (long)(nStartX * HMM_PER_TWIPS);
        nEndX   = (long)(nEndX   * HMM_PER_TWIPS);
    }
    if (!bSetVer)
    {
        nStartY = 0;
        USHORT nStartRow = rRange.aStart.Row();
        for (i=0; i<nStartRow; i++)
            nStartY +=pDoc->FastGetRowHeight(i,nTab);
        nEndY = nStartY;
        USHORT nEndRow = rRange.aEnd.Row();
        for (i=nStartRow; i<=nEndRow; i++)
            nEndY += pDoc->FastGetRowHeight(i,nTab);
        nStartY = (long)(nStartY * HMM_PER_TWIPS);
        nEndY   = (long)(nEndY   * HMM_PER_TWIPS);
    }

    const SdrPage* pPage = GetPage(nTab);
    DBG_ASSERT(pPage,"Page nicht gefunden");
    if (pPage)
    {
        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
                            //! Flags (ausgeblendet?) testen

            Rectangle aObjRect = pObject->GetBoundRect();
            BOOL bFit = TRUE;
            if ( !bSetHor && ( aObjRect.Right() < nStartX || aObjRect.Left() > nEndX ) )
                bFit = FALSE;
            if ( !bSetVer && ( aObjRect.Bottom() < nStartY || aObjRect.Top() > nEndY ) )
                bFit = FALSE;
            if ( bFit )
            {
                if (bSetHor)
                {
                    if (aObjRect.Left() < nStartX) nStartX = aObjRect.Left();
                    if (aObjRect.Right()  > nEndX) nEndX = aObjRect.Right();
                }
                if (bSetVer)
                {
                    if (aObjRect.Top()  < nStartY) nStartY = aObjRect.Top();
                    if (aObjRect.Bottom() > nEndY) nEndY = aObjRect.Bottom();
                }
                bAny = TRUE;
            }

            pObject = aIter.Next();
        }
    }

    if (bAny)
    {
        DBG_ASSERT( nStartX<=nEndX && nStartY<=nEndY, "Start/End falsch in ScDrawLayer::GetPrintArea" );

        if (bSetHor)
        {
            nStartX = (long) (nStartX / HMM_PER_TWIPS);
            nEndX = (long) (nEndX / HMM_PER_TWIPS);
            long nWidth;

            nWidth = 0;
            for (i=0; i<MAXCOL && nWidth<=nStartX; i++)
                nWidth += pDoc->GetColWidth(i,nTab);
            rRange.aStart.SetCol( i ? (i-1) : 0 );

            nWidth = 0;
            for (i=0; i<MAXCOL && nWidth<=nEndX; i++)           //! bei Start anfangen
                nWidth += pDoc->GetColWidth(i,nTab);
            rRange.aEnd.SetCol( i ? (i-1) : 0 );
        }

        if (bSetVer)
        {
            nStartY = (long) (nStartY / HMM_PER_TWIPS);
            nEndY = (long) (nEndY / HMM_PER_TWIPS);
            long nHeight;

            nHeight = 0;
            for (i=0; i<MAXROW && nHeight<=nStartY; i++)
                nHeight += pDoc->FastGetRowHeight(i,nTab);
            rRange.aStart.SetRow( i ? (i-1) : 0 );

            nHeight = 0;
            for (i=0; i<MAXROW && nHeight<=nEndY; i++)
                nHeight += pDoc->FastGetRowHeight(i,nTab);
            rRange.aEnd.SetRow( i ? (i-1) : 0 );
        }
    }
    else
    {
        if (bSetHor)
        {
            rRange.aStart.SetCol(0);
            rRange.aEnd.SetCol(0);
        }
        if (bSetVer)
        {
            rRange.aStart.SetRow(0);
            rRange.aEnd.SetRow(0);
        }
    }
    return bAny;
}

void ScDrawLayer::AddCalcUndo( SdrUndoAction* pUndo )
{
    if (bRecording)
    {
        if (!pUndoGroup)
            pUndoGroup = new SdrUndoGroup(*this);

        pUndoGroup->AddAction( pUndo );
    }
    else
        delete pUndo;
}

void ScDrawLayer::BeginCalcUndo()
{
//! DBG_ASSERT( !bRecording, "BeginCalcUndo ohne GetCalcUndo" );

    DELETEZ(pUndoGroup);
    bRecording = TRUE;
}

SdrUndoGroup* ScDrawLayer::GetCalcUndo()
{
//! DBG_ASSERT( bRecording, "GetCalcUndo ohne BeginCalcUndo" );

    SdrUndoGroup* pRet = pUndoGroup;
    pUndoGroup = NULL;
    bRecording = FALSE;
    return pRet;
}

void ScDrawLayer::MoveAreaMM( USHORT nTab, const Rectangle& rArea,
        const Point& rMove, const Point& rTopLeft )
{
    if (!rMove.X() && !rMove.Y())
        return;                                     // nix

    SdrPage* pPage = GetPage(nTab);
    DBG_ASSERT(pPage,"Page nicht gefunden");
    if (!pPage)
        return;

    Point aNewPos;
    Rectangle aObjRect;

    // fuer Shrinking!
    Rectangle aNew( rArea );
    BOOL bShrink = FALSE;
    if ( rMove.X() < 0 || rMove.Y() < 0 )       // verkleinern
    {
        if ( rTopLeft != rArea.TopLeft() )      // sind gleich beim Verschieben von Zellen
        {
            bShrink = TRUE;
            aNew.Left() = rTopLeft.X();
            aNew.Top() = rTopLeft.Y();
        }
    }
    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject)
    {
        if( GetAnchor( pObject ) == SCA_CELL )
        {
            if ( GetObjData( pObject ) )                    // Detektiv-Pfeil ?
            {
                // hier nichts
            }
            else if ( pObject->ISA( SdrEdgeObj ) )          // Verbinder?
            {
                //  hier auch nichts
                //! nicht verbundene Enden wie bei Linien (s.u.) behandeln?
            }
            else if ( pObject->IsPolyObj() && pObject->GetPointCount()==2 )
            {
                for (USHORT i=0; i<2; i++)
                {
                    BOOL bMoved = FALSE;
                    Point aPoint = pObject->GetPoint(i);
                    if (rArea.IsInside(aPoint))
                    {
                        aPoint += rMove; bMoved = TRUE;
                    }
                    else if (bShrink && aNew.IsInside(aPoint))
                    {
                        //  Punkt ist in betroffener Zelle - Test auf geloeschten Bereich
                        if ( rMove.X() && aPoint.X() >= rArea.Left() + rMove.X() )
                        {
                            aPoint.X() = rArea.Left() + rMove.X() - SHRINK_DIST;
                            if ( aPoint.X() < 0 ) aPoint.X() = 0;
                            bMoved = TRUE;
                        }
                        if ( rMove.Y() && aPoint.Y() >= rArea.Top() + rMove.Y() )
                        {
                            aPoint.Y() = rArea.Top() + rMove.Y() - SHRINK_DIST;
                            if ( aPoint.Y() < 0 ) aPoint.Y() = 0;
                            bMoved = TRUE;
                        }
                    }
                    if( bMoved )
                    {
                        AddCalcUndo( new SdrUndoGeoObj( *pObject ) );
                        pObject->SetPoint( aPoint, i );
                    }
                }
            }
            else
            {
                aObjRect = pObject->GetLogicRect();
                Point aTopLeft = aObjRect.TopLeft();
                Size aMoveSize;
                BOOL bDoMove = FALSE;
                if (rArea.IsInside(aTopLeft))
                {
                    aMoveSize = Size(rMove.X(),rMove.Y());
                    bDoMove = TRUE;
                }
                else if (bShrink && aNew.IsInside(aTopLeft))
                {
                    //  Position ist in betroffener Zelle - Test auf geloeschten Bereich
                    if ( rMove.X() && aTopLeft.X() >= rArea.Left() + rMove.X() )
                    {
                        aMoveSize.Width() = rArea.Left() + rMove.X() - SHRINK_DIST - aTopLeft.X();
                        bDoMove = TRUE;
                    }
                    if ( rMove.Y() && aTopLeft.Y() >= rArea.Top() + rMove.Y() )
                    {
                        aMoveSize.Height() = rArea.Top() + rMove.Y() - SHRINK_DIST - aTopLeft.Y();
                        bDoMove = TRUE;
                    }
                }
                if ( bDoMove )
                {
                    if ( aTopLeft.X() + aMoveSize.Width() < 0 )
                        aMoveSize.Width() = -aTopLeft.X();
                    if ( aTopLeft.Y() + aMoveSize.Height() < 0 )
                        aMoveSize.Height() = -aTopLeft.Y();
                    AddCalcUndo( new SdrUndoMoveObj( *pObject, aMoveSize ) );
                    pObject->Move( aMoveSize );
                }
                else if ( rArea.IsInside(aObjRect.BottomRight()) &&
                            !pObject->IsResizeProtect() )
                {
                    //  geschuetzte Groessen werden nicht veraendert
                    //  (Positionen schon, weil sie ja an der Zelle "verankert" sind)
                    AddCalcUndo( new SdrUndoGeoObj( *pObject ) );
                    long nOldSizeX = aObjRect.Right() - aObjRect.Left() + 1;
                    long nOldSizeY = aObjRect.Bottom() - aObjRect.Top() + 1;
                    pObject->Resize( aObjRect.TopLeft(), Fraction( nOldSizeX+rMove.X(), nOldSizeX ),
                                                         Fraction( nOldSizeY+rMove.Y(), nOldSizeY ) );
                }
            }
        }
        pObject = aIter.Next();
    }
}

void ScDrawLayer::MoveArea( USHORT nTab, USHORT nCol1,USHORT nRow1, USHORT nCol2,USHORT nRow2,
                            short nDx,short nDy, BOOL bInsDel )
{
    if (!bAdjustEnabled)
        return;

    Rectangle aRect = pDoc->GetMMRect( nCol1, nRow1, nCol2, nRow2, nTab );

    short s;
    Point aMove;

    if (nDx > 0)
        for (s=0; s<nDx; s++)
            aMove.X() += pDoc->GetColWidth(s+(short)nCol1,nTab);
    else
        for (s=-1; s>=nDx; s--)
            aMove.X() -= pDoc->GetColWidth(s+(short)nCol1,nTab);
    if (nDy > 0)
        for (s=0; s<nDy; s++)
            aMove.Y() += pDoc->FastGetRowHeight(s+(short)nRow1,nTab);
    else
        for (s=-1; s>=nDy; s--)
            aMove.Y() -= pDoc->FastGetRowHeight(s+(short)nRow1,nTab);

    TwipsToMM( aMove.X() );
    TwipsToMM( aMove.Y() );

    Point aTopLeft = aRect.TopLeft();       // Anfang beim Verkleinern
    if (bInsDel)
    {
        if ( aMove.X() < 0 )
            aTopLeft.X() += aMove.X();
        if ( aMove.Y() < 0 )
            aTopLeft.Y() += aMove.Y();
    }

    MoveAreaMM( nTab, aRect, aMove, aTopLeft );

        //
        //      Detektiv-Pfeile: Zellpositionen anpassen
        //

    MoveCells( nTab, nCol1,nRow1, nCol2,nRow2, nDx,nDy );
}

void ScDrawLayer::WidthChanged( USHORT nTab, USHORT nCol, long nDifTwips )
{
    if (!bAdjustEnabled)
        return;

    USHORT i;
    Rectangle aRect;
    Point aTopLeft;

    for (i=0; i<nCol; i++)
        aRect.Left() += pDoc->GetColWidth(i,nTab);
    aTopLeft.X() = aRect.Left();
    aRect.Left() += pDoc->GetColWidth(nCol,nTab);

    TwipsToMM( aRect.Left() );
    TwipsToMM( aTopLeft.X() );

    aRect.Right() = MAXMM;
    aRect.Top() = 0;
    aRect.Bottom() = MAXMM;

    TwipsToMM( nDifTwips );

    //! aTopLeft ist falsch, wenn mehrere Spalten auf einmal ausgeblendet werden

    MoveAreaMM( nTab, aRect, Point( nDifTwips,0 ), aTopLeft );
}

void ScDrawLayer::HeightChanged( USHORT nTab, USHORT nRow, long nDifTwips )
{
    if (!bAdjustEnabled)
        return;

    USHORT i;
    Rectangle aRect;
    Point aTopLeft;

    for (i=0; i<nRow; i++)
        aRect.Top() += pDoc->FastGetRowHeight(i,nTab);
    aTopLeft.Y() = aRect.Top();
    aRect.Top() += pDoc->FastGetRowHeight(nRow,nTab);

    TwipsToMM( aRect.Top() );
    TwipsToMM( aTopLeft.Y() );

    aRect.Bottom() = MAXMM;
    aRect.Left() = 0;
    aRect.Right() = MAXMM;

    TwipsToMM( nDifTwips );

    //! aTopLeft ist falsch, wenn mehrere Zeilen auf einmal ausgeblendet werden

    MoveAreaMM( nTab, aRect, Point( 0,nDifTwips ), aTopLeft );
}

BOOL ScDrawLayer::HasObjectsInRows( USHORT nTab, USHORT nStartRow, USHORT nEndRow )
{
    Rectangle aTestRect;

    USHORT i;
    for (i=0; i<nStartRow; i++)
        aTestRect.Top() += pDoc->FastGetRowHeight(i,nTab);

    if (nEndRow==MAXROW)
        aTestRect.Bottom() = MAXMM;
    else
    {
        aTestRect.Bottom() = aTestRect.Top();
        for (i=nStartRow; i<=nEndRow; i++)
        {
            aTestRect.Bottom() += pDoc->FastGetRowHeight(i,nTab);
        }
        TwipsToMM( aTestRect.Bottom() );
    }

    TwipsToMM( aTestRect.Top() );

    aTestRect.Left()  = 0;
    aTestRect.Right() = MAXMM;

    SdrPage* pPage = GetPage(nTab);
    DBG_ASSERT(pPage,"Page nicht gefunden");
    if (!pPage)
        return FALSE;

    BOOL bFound = FALSE;

    Rectangle aObjRect;
    SdrObjListIter aIter( *pPage );
    SdrObject* pObject = aIter.Next();
    while ( pObject && !bFound )
    {
        aObjRect = pObject->GetSnapRect();  //! GetLogicRect ?
        if (aTestRect.IsInside(aObjRect.TopLeft()) || aTestRect.IsInside(aObjRect.BottomLeft()))
            bFound = TRUE;

        pObject = aIter.Next();
    }

    return bFound;
}

void ScDrawLayer::DeleteObjects( USHORT nTab )
{
    SdrPage* pPage = GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return;

    pPage->RecalcObjOrdNums();

    long    nDelCount = 0;
    ULONG   nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        SdrObject** ppObj = new SdrObject*[nObjCount];

        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            //  alle loeschen
            ppObj[nDelCount++] = pObject;
            pObject = aIter.Next();
        }

        long i;
        if (bRecording)
            for (i=1; i<=nDelCount; i++)
                AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );

        for (i=1; i<=nDelCount; i++)
            pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );

        delete[] ppObj;
    }
}

void ScDrawLayer::DeleteObjectsInArea( USHORT nTab, USHORT nCol1,USHORT nRow1,
                                            USHORT nCol2,USHORT nRow2 )
{
    SdrPage* pPage = GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return;

    pPage->RecalcObjOrdNums();

    long    nDelCount = 0;
    ULONG   nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        Rectangle aDelRect = pDoc->GetMMRect( nCol1, nRow1, nCol2, nRow2, nTab );

        SdrObject** ppObj = new SdrObject*[nObjCount];

        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            Rectangle aObjRect = pObject->GetBoundRect();
            if ( aDelRect.IsInside( aObjRect ) )
                ppObj[nDelCount++] = pObject;

            pObject = aIter.Next();
        }

        long i;
        if (bRecording)
            for (i=1; i<=nDelCount; i++)
                AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );

        for (i=1; i<=nDelCount; i++)
            pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );

        delete[] ppObj;
    }
}

void ScDrawLayer::DeleteObjectsInSelection( const ScMarkData& rMark )
{
    if ( !rMark.IsMultiMarked() )
        return;

    ScRange aMarkRange;
    rMark.GetMultiMarkArea( aMarkRange );

    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<=nTabCount; nTab++)
        if ( rMark.GetTableSelect( nTab ) )
        {
            SdrPage* pPage = GetPage(nTab);
            if (pPage)
            {
                pPage->RecalcObjOrdNums();
                long    nDelCount = 0;
                ULONG   nObjCount = pPage->GetObjCount();
                if (nObjCount)
                {
                    //  Rechteck um die ganze Selektion
                    Rectangle aMarkBound = pDoc->GetMMRect(
                                aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                                aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(), nTab );

                    SdrObject** ppObj = new SdrObject*[nObjCount];

                    SdrObjListIter aIter( *pPage, IM_FLAT );
                    SdrObject* pObject = aIter.Next();
                    while (pObject)
                    {
                        Rectangle aObjRect = pObject->GetBoundRect();
                        if ( aMarkBound.IsInside( aObjRect ) )
                        {
                            ScRange aRange = pDoc->GetRange( nTab, aObjRect );
                            if (rMark.IsAllMarked(aRange))
                                ppObj[nDelCount++] = pObject;
                        }

                        pObject = aIter.Next();
                    }

                    //  Objekte loeschen (rueckwaerts)

                    long i;
                    if (bRecording)
                        for (i=1; i<=nDelCount; i++)
                            AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );

                    for (i=1; i<=nDelCount; i++)
                        pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );

                    delete[] ppObj;
                }
            }
            else
                DBG_ERROR("pPage?");
        }
}

SdrObject* ScDrawLayer::GetNamedObject( const String& rName, USHORT nId, USHORT& rFoundTab ) const
{
    USHORT nTabCount = GetPageCount();
    for (USHORT nTab=0; nTab<nTabCount; nTab++)
    {
        const SdrPage* pPage = GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( nId == 0 || pObject->GetObjIdentifier() == nId )
                    if ( pObject->GetName() == rName )
                    {
                        rFoundTab = nTab;
                        return pObject;
                    }

                pObject = aIter.Next();
            }
        }
    }

    return NULL;
}

String ScDrawLayer::GetNewGraphicName() const
{
    String aBase = ScGlobal::GetRscString(STR_GRAPHICNAME);
    aBase += ' ';

    BOOL bThere = TRUE;
    String aName;
    USHORT nDummy;
    long nId = 0;
    while (bThere)
    {
        ++nId;
        aName = aBase;
        aName += String::CreateFromInt32( nId );
        bThere = ( GetNamedObject( aName, 0, nDummy ) != NULL );
    }

    return aName;
}

void ScDrawLayer::EnsureGraphicNames()
{
    //  make sure all graphic objects have names (after Excel import etc.)

    USHORT nTabCount = GetPageCount();
    for (USHORT nTab=0; nTab<nTabCount; nTab++)
    {
        SdrPage* pPage = GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_GRAF && pObject->GetName().Len() == 0 )
                    pObject->SetName( GetNewGraphicName() );

                pObject = aIter.Next();
            }
        }
    }
}

ULONG ScDrawLayer::GetDefTextHeight() const
{
    return nDefTextHgt;     // protected in SdrModel
}

void ScDrawLayer::SetAnchor( SdrObject* pObj, ScAnchorType eType )
{
    // Ein an der Seite verankertes Objekt zeichnet sich durch eine Anker-Pos
    // von (0,1) aus. Das ist ein shabby Trick, der aber funktioniert!
    Point aAnchor( 0, eType == SCA_PAGE ? 1 : 0 );
    pObj->SetAnchorPos( aAnchor );
}

ScAnchorType ScDrawLayer::GetAnchor( const SdrObject* pObj )
{
    Point aAnchor( pObj->GetAnchorPos() );
    return ( aAnchor.Y() != 0 ) ? SCA_PAGE : SCA_CELL;
}

ScDrawObjData* ScDrawLayer::GetObjData( SdrObject* pObj, BOOL bCreate )     // static
{
    USHORT nCount = pObj->GetUserDataCount();
    for( USHORT i = 0; i < nCount; i++ )
    {
        SdrObjUserData* pData = pObj->GetUserData( i );
        if( pData && pData->GetInventor() == SC_DRAWLAYER
                    && pData->GetId() == SC_UD_OBJDATA )
            return (ScDrawObjData*) pData;
    }
    if( bCreate )
    {
        ScDrawObjData* pData = new ScDrawObjData;
        pObj->InsertUserData( pData, 0 );
        return pData;
    }
    return NULL;
}

ScIMapInfo* ScDrawLayer::GetIMapInfo( SdrObject* pObj )             // static
{
    USHORT nCount = pObj->GetUserDataCount();
    for( USHORT i = 0; i < nCount; i++ )
    {
        SdrObjUserData* pData = pObj->GetUserData( i );
        if( pData && pData->GetInventor() == SC_DRAWLAYER
                    && pData->GetId() == SC_UD_IMAPDATA )
            return (ScIMapInfo*) pData;
    }
    return NULL;
}

Graphic ScDrawLayer::GetGraphicFromOle2Obj( const SdrOle2Obj* pOle2Obj )    // static
{
    SvInPlaceObjectRef  aIPObjRef = pOle2Obj->GetObjRef();
    Graphic             aGraphic;

    if ( aIPObjRef.Is() )
    {
        VirtualDevice   aVDev;
        GDIMetaFile     aGDIMtf;
        const MapMode   aMap100( MAP_100TH_MM );
        const Size&     rSize = aIPObjRef->GetVisArea().GetSize();

        aVDev.SetMapMode( aMap100 );
        aGDIMtf.Record( &aVDev );

        aIPObjRef->DoDraw( &aVDev, Point(), rSize, JobSetup() );

        aGDIMtf.Stop();
        aGDIMtf.WindStart();
        aGDIMtf.SetPrefMapMode( aMap100 );
        aGDIMtf.SetPrefSize( rSize );
        aGraphic = Graphic( aGDIMtf );
    }

    return aGraphic;
}

// static:
IMapObject* ScDrawLayer::GetHitIMapObject( SdrObject* pObj,
                                          const Point& rWinPoint, const Window& rCmpWnd )
{
    const MapMode       aMap100( MAP_100TH_MM );
    MapMode             aWndMode = rCmpWnd.GetMapMode();
    Point               aRelPoint( rCmpWnd.LogicToLogic( rWinPoint, &aWndMode, &aMap100 ) );
    Rectangle           aLogRect = rCmpWnd.LogicToLogic( pObj->GetLogicRect(), &aWndMode, &aMap100 );
    ScIMapInfo*         pIMapInfo = GetIMapInfo( pObj );
    IMapObject*         pIMapObj = NULL;

    if ( pIMapInfo )
    {
        Size        aGraphSize;
        ImageMap&   rImageMap = (ImageMap&) pIMapInfo->GetImageMap();
        Graphic     aGraphic;
        BOOL        bObjSupported = FALSE;

        if ( pObj->ISA( SdrGrafObj )  ) // einfaches Grafik-Objekt
        {
            const SdrGrafObj*   pGrafObj = (const SdrGrafObj*) pObj;
            const GeoStat&      rGeo = pGrafObj->GetGeoStat();
            const Graphic&      rGraphic = pGrafObj->GetGraphic();

            // Drehung rueckgaengig
            if ( rGeo.nDrehWink )
                RotatePoint( aRelPoint, aLogRect.TopLeft(), -rGeo.nSin, rGeo.nCos );

            // Spiegelung rueckgaengig
            if ( ( (const SdrGrafObjGeoData*) pGrafObj->GetGeoData() )->bMirrored )
                aRelPoint.X() = aLogRect.Right() + aLogRect.Left() - aRelPoint.X();

            // ggf. Unshear:
            if ( rGeo.nShearWink )
                ShearPoint( aRelPoint, aLogRect.TopLeft(), -rGeo.nTan );


            if ( rGraphic.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
                aGraphSize = rCmpWnd.PixelToLogic( rGraphic.GetPrefSize(),
                                                         aMap100 );
            else
                aGraphSize = OutputDevice::LogicToLogic( rGraphic.GetPrefSize(),
                                                         rGraphic.GetPrefMapMode(),
                                                         aMap100 );

            bObjSupported = TRUE;
        }
        else if ( pObj->ISA( SdrOle2Obj ) ) // OLE-Objekt
        {
            SvInPlaceObjectRef aIPObjRef = ( (SdrOle2Obj*) pObj )->GetObjRef();

            if ( aIPObjRef.Is() )
            {
                aGraphSize = aIPObjRef->GetVisArea().GetSize();
                bObjSupported = TRUE;
            }
        }

        // hat alles geklappt, dann HitTest ausfuehren
        if ( bObjSupported )
        {
            // relativen Mauspunkt berechnen
            aRelPoint -= aLogRect.TopLeft();
            pIMapObj = rImageMap.GetHitIMapObject( aGraphSize, aLogRect.GetSize(), aRelPoint );
        }
    }

    return pIMapObj;
}

void ScDrawLayer::SetGlobalDrawPersist(SvPersist* pPersist)         // static
{
    DBG_ASSERT(!pGlobalDrawPersist,"SetGlobalDrawPersist mehrfach");
    pGlobalDrawPersist = pPersist;
}

void __EXPORT ScDrawLayer::SetChanged( FASTBOOL bFlg /* =TRUE */ )
{
    if ( bFlg )
        pDoc->SetChartListenerCollectionNeedsUpdate( TRUE );
    FmFormModel::SetChanged( bFlg );
}

SvStream* __EXPORT ScDrawLayer::GetDocumentStream(FASTBOOL& rbDeleteAfterUse) const
{
    rbDeleteAfterUse = FALSE;
    SvStream* pStream = NULL;

    SfxObjectShell* pDocSh = pDoc->GetDocumentShell();
    if (pDocSh)
    {
        SvStorage* pStor = pDocSh->GetStorage();

        if (pStor)
        {
            pStream = pStor->OpenStream(
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_SCSTREAM)),
                        STREAM_READ | STREAM_WRITE | STREAM_TRUNC);
            pStream->SetVersion(pStor->GetVersion());
            pStream->SetKey( pStor->GetKey() ); // Passwort setzen
        }
    }

    return pStream;
}

SdrLayerID __EXPORT ScDrawLayer::GetControlExportLayerId( const SdrObject & ) const
{
    //  Layer fuer Export von Form-Controls in Versionen vor 5.0 - immer SC_LAYER_FRONT
    return SC_LAYER_FRONT;
}



