/*************************************************************************
 *
 *  $RCSfile: tabvwsh9.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:10 $
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
//#define _SFXDOCINF_HXX ***
#define _SFXLINKHDL_HXX
//#define _SFX_PROGRESS_HXX

//sfxsh.hxx
//#define _SFX_SHELL_HXX
//#define _SFXAPP_HXX
//#define _SFX_BINDINGS_HXX
#define _SFXDISPATCH_HXX //???
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

//sfxitems.hxx
#define _SFX_WHMAP_HXX
#define _ARGS_HXX
//#define _SFXPOOLITEM_HXX
//#define _SFXINTITEM_HXX
//#define _SFXENUMITEM_HXX
//#define _SFXFLAGITEM_HXX
//#define _SFXSTRITEM_HXX
#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
//#define _SFXITEMPOOL_HXX ***
//#define _SFXITEMSET_HXX ***
#define _SFXITEMITER_HXX
//#define _SFX_WHITER_HXX ***
#define _SFXPOOLCACH_HXX
#define _AEITEM_HXX         //???
#define _SFXRNGITEM_HXX
#define _SFXSLSTITM_HXX     //???
#define _SFXSTYLE_HXX

//sfxdlg.hxx
//#define _SFXDLG_HXX ***
//#define _SFXTABDLG_HXX ***
//#define _BASEDLGS_HXX ***
#define _DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
#define _SFXDOCTEMPL_HXX
#define _SFXDOCTDLG_HXX
//#define _SFXDOCKWIN_HXX ***
#define _SFX_TEMPLDLG_HXX
#define _SFXNEW_HXX
#define _SFXDOCMAN_HXX

//sfxdoc.hxx
//#define _SFXDOC_HXX ***
//#define _SFX_OBJSH_HXX ***
//#define _SFX_CLIENTSH_HXX ***
#define _SFX_DOCFILT_HACK_HXX
#define _SFX_FCONTNR_HXX
#define _SFXDOCFILE_HXX
//#define _SFXFRAME_HXX ***
//#define _SFXVIEWFRM_HXX ***
//#define _SFXVIEWSH_HXX ***
#define _MDIFRM_HXX
#define _SFX_IPFRM_HXX
#define _SFX_INTERNO_HXX
#define _FSETOBJSH_HXX

//xout.hxx
//#define _XENUM_HXX ***
//#define _XPOLY_HXX ***
//#define _XATTR_HXX ***
#define _XOUTX_HXX
//#define _XPOOL_HXX ***
//#define _XTABLE_HXX ***

//si.hxx
//#define _SI_HXX ***
#define _SIDLL_HXX
#define _VCATTR_HXX
#define _VCONT_HXX
#define _VCSBX_HXX
#define _SI_NOVCSBXFORM
#define SI_NOOTHERFORMS
#define _SI_NOVCSBXBUTTON
#define _SI_NOVCSBXCHECKBOX
#define _SI_NOVCSBXRADIOBUTTON
#define _SI_NOVCSBXPUSHBUTTON
#define _SI_NOVCSBXSPINBUTTON
#define _SI_NOVCSBXFIXEDTEXT
#define _SI_NOVCSBXGROUPBOX
#define _SI_NOVCSBXLISTBOX
#define _SI_NOVCSBXCOMBOBOX
#define _SI_NOVCSBXEDIT
#define _SI_NOVCSBXSCROLLBAR
#define _SI_NOVCSBXHSCROLLBAR
#define _SI_BOVCSBXVSCROLLBAR
#define _SI_NOVCSBXPREVIEW
#define _SI_NOVCSBXURLBUTTON
#define _VCTRLS_HXX
//#define _VCDRWOBJ_HXX ***
//#define _SI_NOVCDRAWMODEL ***
#define _SI_NOVCDRAWPAGE
#define _SI_NOVCDRAWVIEW
#define _VCBRW_HXX
#define _VCFORM_HXX

//svdraw.hxx
//#define _SVDRAW_HXX ***
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
//#define _SDR_NOOBJECTS ***
//#define _SDR_NOVIEWS ***
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
//#define _SDR_NOUNDO ***
#define _SDR_NOXOUTDEV

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

//drawdlgs.hxx
//#define _SVX_DRAWDLGS_HXX ***
//#define _SVX_DLG_CTRL_HXX ***
#define _SVX_DLG_NAME_HXX
#define _SVX_TAB_AREA_HXX
#define _SVX_TAB_LINE_HXX
#define _SVX_TRANSFRM_HXX

class SbxArray;

// INCLUDE ---------------------------------------------------------------

#include <svx/svdmark.hxx>
#include <svx/svdview.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/whiter.hxx>

#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "tabview.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"

// forwards -> galwrap.cxx (wg. CLOOKs)

USHORT  GallerySGA_FORMAT_GRAPHIC();
Graphic GalleryGetGraphic       ();
BOOL    GalleryIsLinkage        ();
String  GalleryGetFullPath      ();
String  GalleryGetFilterName    ();

// forwards -> imapwrap.cxx (wg. CLOOKs)

class SvxIMapDlg;

USHORT          ScIMapChildWindowId();
SvxIMapDlg*     ScGetIMapDlg();
const void*     ScIMapDlgGetObj( SvxIMapDlg* pDlg );
const ImageMap& ScIMapDlgGetMap( SvxIMapDlg* pDlg );

//------------------------------------------------------------------


//------------------------------------------------------------------

void ScTabViewShell::ExecGallery( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();

    if ( pArgs )
    {
        const SfxPoolItem* pItem = NULL;
        SfxItemState eState = pArgs->GetItemState(SID_GALLERY_FORMATS, TRUE, &pItem);
        if ( eState == SFX_ITEM_SET )
        {
            UINT32 nFormats = ((const SfxUInt32Item*)pItem)->GetValue();

            /******************************************************************
            * Graphik einfuegen
            ******************************************************************/
            if ( nFormats & GallerySGA_FORMAT_GRAPHIC() )
            {
                MakeDrawLayer();

                Graphic aGraphic = GalleryGetGraphic();
                Point   aPos     = GetInsertPos();

                String aPath, aFilter;
                if ( GalleryIsLinkage() )           // als Link einfuegen?
                {
                    aPath = GalleryGetFullPath();
                    aFilter = GalleryGetFilterName();
                }

                PasteGraphic( aPos, aGraphic, aPath, aFilter );
            }
        }
    }
}

void ScTabViewShell::GetGalleryState( SfxItemSet& rSet )
{
}

//------------------------------------------------------------------

ScInputHandler* ScTabViewShell::GetInputHandler() const
{
    return pInputHandler;
}

//------------------------------------------------------------------

String __EXPORT ScTabViewShell::GetDescription() const
{
    return String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(" ** Test ** "));
}

void ScTabViewShell::ExecImageMap( SfxRequest& rReq )
{
    USHORT nSlot = rReq.GetSlot();
    switch(nSlot)
    {
        case SID_IMAP:
        {
            SfxViewFrame* pThisFrame = GetViewFrame();
            USHORT nId = ScIMapChildWindowId();
            pThisFrame->ToggleChildWindow( nId );
            SFX_BINDINGS().Invalidate( SID_IMAP );

            if ( pThisFrame->HasChildWindow( nId ) )
            {
                SvxIMapDlg* pDlg = ScGetIMapDlg();
                if ( pDlg )
                {
                    SdrView* pDrView = GetSdrView();
                    if ( pDrView )
                    {
                        const SdrMarkList& rMarkList = pDrView->GetMarkList();
                        if ( rMarkList.GetMarkCount() == 1 )
                            UpdateIMap( rMarkList.GetMark( 0 )->GetObj() );
                    }
                }
            }

            rReq.Ignore();
        }
        break;

        case SID_IMAP_EXEC:
        {
            SdrView* pDrView = GetSdrView();
            SdrMark* pMark = pDrView ? pDrView->GetMarkList().GetMark(0) : 0;

            if ( pMark )
            {
                SdrObject*  pSdrObj = pMark->GetObj();
                SvxIMapDlg* pDlg = ScGetIMapDlg();

                if ( ScIMapDlgGetObj(pDlg) == (void*) pSdrObj )
                {
                    const ImageMap& rImageMap = ScIMapDlgGetMap(pDlg);
                    ScIMapInfo*     pIMapInfo = ScDrawLayer::GetIMapInfo( pSdrObj );

                    if ( !pIMapInfo )
                        pSdrObj->InsertUserData( new ScIMapInfo( rImageMap ) );
                    else
                        pIMapInfo->SetImageMap( rImageMap );
                }
            }
        }
        break;
    }
}

void ScTabViewShell::GetImageMapState( SfxItemSet& rSet )
{
    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_IMAP:
                {
                    //  Disabled wird nicht mehr...

                    BOOL bThere = FALSE;
                    SfxViewFrame* pThisFrame = GetViewFrame();
                    USHORT nId = ScIMapChildWindowId();
                    if ( pThisFrame->KnowsChildWindow(nId) )
                        if ( pThisFrame->HasChildWindow(nId) )
                            bThere = TRUE;

                    ObjectSelectionType eType=GetCurObjectSelectionType();
                    BOOL bEnable=(eType==OST_OleObject) ||(eType==OST_Graphic);
                    if(!bThere && !bEnable)
                    {
                       rSet.DisableItem( nWhich );
                    }
                    else
                    {
                        rSet.Put( SfxBoolItem( nWhich, bThere ) );
                    }
                }
                break;

            case SID_IMAP_EXEC:
                {
                    BOOL bDisable = TRUE;

                    SdrView* pDrView = GetSdrView();
                    if ( pDrView )
                    {
                        const SdrMarkList& rMarkList = pDrView->GetMarkList();
                        if ( rMarkList.GetMarkCount() == 1 )
                            if ( ScIMapDlgGetObj(ScGetIMapDlg()) ==
                                        (void*) rMarkList.GetMark(0)->GetObj() )
                                bDisable = FALSE;
                    }

                    rSet.Put( SfxBoolItem( SID_IMAP_EXEC, bDisable ) );
                }
                break;
        }

        nWhich = aIter.NextWhich();
    }
}




