/*************************************************************************
 *
 *  $RCSfile: drawsh.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:55 $
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
//#define _FILDLG_HXX ***
//#define _PRNDLG_HXX
#define _COLDLG_HXX
//#define _TABDLG_HXX
//#define _MENU_HXX ***
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
//#define _SVTREELIST_HXX
#define _FILTER_HXX
//#define _SVLBOXITM_HXX
//#define _SVTREEBOX_HXX
#define _SVICNVW_HXX
#define _SVTABBX_HXX

//sfxcore.hxx
//#define _SFXINIMGR_HXX
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
//#define _SFX_BINDINGS_HXX
//#define _SFXDISPATCH_HXX
//#define _SFXMSG_HXX
//#define _SFXOBJFACE_HXX
//#define _SFXREQUEST_HXX
#define _SFXMACRO_HXX

// SFX
//#define _SFXAPPWIN_HXX
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX
//#define _SFXCTRLITEM_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
//#define _SFXTBXCTRL_HXX ***
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
//#define _SFXDOCFILE_HXX
//define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX
//#define _MDIFRM_HXX
#define _SFX_IPFRM_HXX
//#define _SFX_INTERNO_HXX

//sfxdlg.hxx
//#define _SFXTABDLG_HXX
//#define _BASEDLGS_HXX
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
//#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
//#define _SFXDOCTEMPL_HXX
//#define _SFXDOCTDLG_HXX
//#define _SFX_TEMPLDLG_HXX
//#define _SFXNEW_HXX
#define _SFXDOCMAN_HXX
//#define _SFXDOCKWIN_HXX ***

//sfxitems.hxx
#define _SFX_WHMAP_HXX
#define _ARGS_HXX
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
//#define _SFX_WHITER_HXX ***
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
//#define _SIDLL_HXX

//------------------------------------------------------------------

#include <svx/eeitem.hxx>
#include <svx/fontwork.hxx>
#include <svx/labdlg.hxx>
#include <svx/srchitem.hxx>
#include <svx/tabarea.hxx>
#include <svx/tabline.hxx>
#include <svx/textanim.hxx>
#include <svx/transfrm.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <svtools/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <segmentc.hxx>

#include "drawsh.hxx"
#include "drwlayer.hxx"
#include "sc.hrc"
#include "viewdata.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "drawview.hxx"
#include "scresid.hxx"

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif

#define ScDrawShell
#include "scslots.hxx"

//------------------------------------------------------------------

SEG_EOFGLOBALS()

#pragma SEG_SEGCLASS(SFXMACROS_SEG,STARTWORK_CODE)

TYPEINIT1( ScDrawShell, SfxShell );

SFX_IMPL_INTERFACE(ScDrawShell, SfxShell, ScResId(SCSTR_DRAWSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                ScResId(RID_DRAW_OBJECTBAR) );
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_DRAW) );
    SFX_OBJECTMENU_REGISTRATION( SID_OBJECTMENU0, ScResId(RID_OBJECTMENU_DRAW) );
    SFX_CHILDWINDOW_REGISTRATION( SvxFontWorkChildWindow::GetChildWindowId() );
}


// abschalten der nicht erwuenschten Acceleratoren:

#pragma SEG_FUNCDEF(drawsh_09)

void ScDrawShell::StateDisableItems( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}


#pragma SEG_FUNCDEF(drawsh_05)

void ScDrawShell::ExecDrawAttr( SfxRequest& rReq )
{
    USHORT              nSlot       = rReq.GetSlot();
    Window*             pWin        = pViewData->GetActiveWin();
    Window*             pDlgParent  = pViewData->GetDialogParent();
//  SfxViewFrame*       pViewFrame  = SfxViewShell::Current()->GetViewFrame(); //!!! koennte knallen
    ScDrawView*         pView       = pViewData->GetScDrawView();
    SdrModel*           pDoc        = pViewData->GetDocument()->GetDrawLayer();

    switch ( nSlot )
    {
        case SID_TEXT_STANDARD: // Harte Textattributierung loeschen
            {
                SfxItemSet aEmptyAttr(GetPool(), EE_ITEMS_START, EE_ITEMS_END);
                pView->SetAttributes(aEmptyAttr, TRUE);
            }
            break;

        case SID_ATTR_LINE_STYLE:
        case SID_ATTR_LINEEND_STYLE:
        case SID_ATTR_LINE_DASH:
        case SID_ATTR_LINE_WIDTH:
        case SID_ATTR_LINE_COLOR:
        case SID_ATTR_FILL_STYLE:
        case SID_ATTR_FILL_COLOR:
        case SID_ATTR_FILL_GRADIENT:
        case SID_ATTR_FILL_HATCH:
        case SID_ATTR_FILL_BITMAP:
            {
                // Wenn ToolBar vertikal :
                if ( !rReq.GetArgs() )
                {
                    switch ( nSlot )
                    {
                        case SID_ATTR_LINE_STYLE:
                        case SID_ATTR_LINE_DASH:
                        case SID_ATTR_LINE_WIDTH:
                        case SID_ATTR_LINE_COLOR:
                            ExecuteLineDlg( rReq );
                            break;

                        case SID_ATTR_FILL_STYLE:
                        case SID_ATTR_FILL_COLOR:
                        case SID_ATTR_FILL_GRADIENT:
                        case SID_ATTR_FILL_HATCH:
                        case SID_ATTR_FILL_BITMAP:
                            ExecuteAreaDlg( rReq );
                            break;

                        default:
                            break;
                    }

                    //=====
                    return;
                    //=====
                }

                if( pView->HasMarkedObj() )
                    pView->SetAttrToMarked( *rReq.GetArgs(), FALSE );
                else
                    pView->SetDefaultAttr( *rReq.GetArgs(), FALSE);
                pView->InvalidateAttribs();
            }
            break;

        case SID_ATTRIBUTES_LINE:
            ExecuteLineDlg( rReq );
            break;

        case SID_ATTRIBUTES_AREA:
            ExecuteAreaDlg( rReq );
            break;

        case SID_DRAWTEXT_ATTR_DLG:
            ExecuteTextAttrDlg( rReq );
            break;

        case SID_ATTR_TRANSFORM:
            {
                if ( pView->HasMarkedObj() )
                {
                    const SfxItemSet* pArgs = rReq.GetArgs();

                    if( !pArgs )
                    {
                        const SdrMarkList& rMarkList = pView->GetMarkList();
                        if( rMarkList.GetMark(0) != 0 )
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
                            if( pObj->GetObjIdentifier() == OBJ_CAPTION )
                            {
                                // --------- Itemset fuer Caption --------
                                SfxItemSet aNewAttr(pDoc->GetItemPool());
                                pView->GetAttributes(aNewAttr);
                                // --------- Itemset fuer Groesse und Position --------
                                SfxItemSet aNewGeoAttr(pView->GetGeoAttrFromMarked());

                                SvxCaptionTabDialog* pDlg = new SvxCaptionTabDialog(pWin, pView);

                                const USHORT* pRange = pDlg->GetInputRanges( *aNewAttr.GetPool() );
                                SfxItemSet aCombSet( *aNewAttr.GetPool(), pRange );
                                aCombSet.Put( aNewAttr );
                                aCombSet.Put( aNewGeoAttr );
                                pDlg->SetInputSet( &aCombSet );

                                if (pDlg->Execute() == RET_OK)
                                {
                                    rReq.Done(*(pDlg->GetOutputItemSet()));
                                    pView->SetAttributes(*pDlg->GetOutputItemSet());
                                    pView->SetGeoAttrToMarked(*pDlg->GetOutputItemSet());
                                }

                                delete pDlg;
                            }
                            else
                            {
                                SfxItemSet aNewAttr(pView->GetGeoAttrFromMarked());
                                SvxTransformTabDialog* pDlg = new SvxTransformTabDialog(pWin, &aNewAttr, pView);

                                if (pDlg->Execute() == RET_OK)
                                {
                                    rReq.Done(*(pDlg->GetOutputItemSet()));
                                    pView->SetGeoAttrToMarked(*pDlg->GetOutputItemSet());
                                }
                                delete pDlg;
                            }
                        }


                    }
                    else
                        pView->SetGeoAttrToMarked( *pArgs );
                }
            }
            break;

        default:
            break;
    }
}

#pragma SEG_FUNCDEF(drawsh_06)

void ScDrawShell::ExecuteLineDlg( SfxRequest& rReq, USHORT nTabPage )
{
    ScDrawView*         pView       = pViewData->GetScDrawView();
    BOOL                bHasMarked  = pView->HasMarkedObj();
    const SdrObject*    pObj        = NULL;
    const SdrMarkList&  rMarkList   = pView->GetMarkList();

    if( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetObj();

    SfxItemSet  aNewAttr( pView->GetDefaultAttr() );
    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, FALSE );

    SvxLineTabDialog* pDlg
        = new SvxLineTabDialog( pViewData->GetDialogParent(),
                                &aNewAttr,
                                pViewData->GetDocument()->GetDrawLayer(),
                                pObj,
                                bHasMarked );

    if ( nTabPage != 0xffff )
        pDlg->SetCurPageId( nTabPage );

    if ( pDlg->Execute() == RET_OK )
    {
        if( bHasMarked )
            pView->SetAttrToMarked( *pDlg->GetOutputItemSet(), FALSE );
        else
            pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), FALSE );

        pView->InvalidateAttribs();
        rReq.Done();
    }

    delete pDlg;
}

#pragma SEG_FUNCDEF(drawsh_07)

void ScDrawShell::ExecuteAreaDlg( SfxRequest& rReq, USHORT nTabPage )
{
    ScDrawView* pView       = pViewData->GetScDrawView();
    BOOL        bHasMarked  = pView->HasMarkedObj();

    SfxItemSet  aNewAttr( pView->GetDefaultAttr() );
    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, FALSE );

    SvxAreaTabDialog* pDlg
        = new SvxAreaTabDialog( pViewData->GetDialogParent(),
                                &aNewAttr,
                                pViewData->GetDocument()->GetDrawLayer(),
                                pView );

    if ( nTabPage != 0xffff )
        pDlg->SetCurPageId( nTabPage );

    if ( pDlg->Execute() == RET_OK )
    {
        if( bHasMarked )
            pView->SetAttrToMarked( *pDlg->GetOutputItemSet(), FALSE );
        else
            pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), FALSE );

        pView->InvalidateAttribs();
        rReq.Done();
    }

    delete pDlg;
}

#pragma SEG_FUNCDEF(drawsh_08)

void ScDrawShell::ExecuteTextAttrDlg( SfxRequest& rReq, USHORT nTabPage )
{
    ScDrawView* pView       = pViewData->GetScDrawView();
    BOOL        bHasMarked  = pView->HasMarkedObj();
    SfxItemSet  aNewAttr    ( pView->GetDefaultAttr() );

    if( bHasMarked )
        pView->MergeAttrFromMarked( aNewAttr, FALSE );

    SvxTextTabDialog* pDlg =
        new SvxTextTabDialog( pViewData->GetDialogParent(), &aNewAttr , pView);

    USHORT nResult = pDlg->Execute();

    if ( RET_OK == nResult )
    {
        if ( bHasMarked )
            pView->SetAttributes( *pDlg->GetOutputItemSet() );
        else
            pView->SetDefaultAttr( *pDlg->GetOutputItemSet(), FALSE );

        rReq.Done();
    }
    delete( pDlg );
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.70  2000/09/17 14:09:01  willem.vandorp
    OpenOffice header added.

    Revision 1.69  2000/08/31 16:38:23  willem.vandorp
    Header and footer replaced

    Revision 1.68  2000/05/09 18:30:02  nn
    use IMPL_INTERFACE macro without IDL

    Revision 1.67  2000/02/11 12:25:38  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.66  1999/06/22 08:55:54  NN
    pass ItemSet to GetAttributes


      Rev 1.65   22 Jun 1999 10:55:54   NN
   pass ItemSet to GetAttributes

      Rev 1.64   22 Feb 1999 20:49:50   ANK
   #47158# Erweiterungen fuer neue DrawForm-Shell

      Rev 1.63   12 Jan 1998 14:06:46   TJ
   include

      Rev 1.62   05 Dec 1997 19:31:10   NN
   Includes

      Rev 1.61   04 Sep 1997 14:30:38   RG
   change header

      Rev 1.60   27 Aug 1997 13:41:54   TRI
   VCL includes

      Rev 1.59   05 Aug 1997 15:21:44   TJ
   include svx/srchitem.hxx

      Rev 1.58   14 Apr 1997 13:05:16   HJS
   includes

      Rev 1.57   04 Apr 1997 16:27:02   TRI
   includes

      Rev 1.56   23 Jan 1997 15:37:02   NN
   beim SvxAreaTabDialog die View uebergeben

      Rev 1.55   08 Jan 1997 20:50:12   HJS
   forward

      Rev 1.54   15 Nov 1996 17:55:46   NN
   SvxTextTabDialog aufrufen

      Rev 1.53   08 Nov 1996 08:43:00   NF
   includes...

      Rev 1.52   29 Oct 1996 14:05:14   NN
   ueberall ScResId statt ResId

      Rev 1.51   21 Oct 1996 15:36:18   NN
   include tbxalign war ueberfluessig

      Rev 1.50   22 Aug 1996 15:39:42   NF
   clooks

      Rev 1.49   09 Aug 1996 20:37:12   NN
   Svx-Includes aus scitems.hxx raus

      Rev 1.48   22 May 1996 14:52:46   NN
   GetDrawView -> GetScDrawView

      Rev 1.47   10 Apr 1996 09:31:42   NF
   sv.hxx als ertes includen

      Rev 1.46   22 Mar 1996 14:03:20   MH
   add: include sfxiiter.hxx

      Rev 1.45   07 Mar 1996 12:27:38   NF
   Zuviele defines

      Rev 1.44   13 Feb 1996 08:26:50   TRI
   CLOOKS

      Rev 1.43   23 Jan 1996 23:43:50   HJS
   anpassung fuer die nicht-b

      Rev 1.42   18 Dec 1995 14:18:42   MO
   Visibility-Flags auf Server gesetzt

      Rev 1.41   12 Dec 1995 18:58:44   MO
   Disable-Methode fuer div. Acceleratoren

      Rev 1.40   12 Dec 1995 17:41:40   MO
   Standard-Textattribute setzen

      Rev 1.39   11 Dec 1995 18:42:58   MO
   ExecuteTextAttrDlg

      Rev 1.37   10 Dec 1995 15:51:54   TRI
   define entfernt

      Rev 1.36   30 Nov 1995 20:09:40   MO
   Linienenden-Control Execute

      Rev 1.35   28 Nov 1995 21:43:52   MO
   include-Reihenfolge geandert

      Rev 1.34   10 Nov 1995 18:36:30   NN
   static -> lcl

      Rev 1.33   26 Oct 1995 14:24:20   JN
   richtigen Dialog bei Legende aufrufen

      Rev 1.32   06 Oct 1995 08:05:46   NN
   300: Parameter bOnlyHardAttr bei MergeAttrFromMarked

      Rev 1.31   10 Aug 1995 00:36:10   HJS
   svundo muss da sein

      Rev 1.30   07 Aug 1995 13:04:22   MO
   253b: Transform-Dialog Aufrufparameter

      Rev 1.29   03 Aug 1995 13:22:26   MO
   FontWork-ChildWindow Register

      Rev 1.28   21 Jul 1995 10:06:14   NN
   InvalidateAttribs beim direkten Setzen von Attributen

      Rev 1.27   20 Jul 1995 18:37:38   MO
   BugFix: vertikale Anordnung der ObjBar (BugId: 16227)

      Rev 1.26   08 Jul 1995 19:14:00   HJS
   ein define zuviel

      Rev 1.25   06 Jul 1995 22:11:12   HJS
   ein #define zuviel

      Rev 1.24   04 Jul 1995 11:57:40   MO
   Parameterkorrektur fuer LineDlg

      Rev 1.23   20 Jun 1995 16:23:46   NN
   nochmal Segmentierung

      Rev 1.22   13 Jun 1995 18:30:40   MO
   seg-pragmas korrigiert

      Rev 1.21   13 Jun 1995 14:43:30   MO
   spezielles Segmentierungs-pragma

      Rev 1.20   07 Jun 1995 10:09:00   MO
   SFX_OBJECTBAR_OBJECT

      Rev 1.19   07 Jun 1995 08:43:22   MO
   SvxIds fuer DrawUI

      Rev 1.18   23 May 1995 14:46:04   MO
   Aufruf Transform-Dialog angepasst

      Rev 1.17   10 May 1995 14:44:36   TRI

      Rev 1.16   04 May 1995 09:28:36   TRI
   SvxAreaTabPage: geanderte Paramteranzahl

      Rev 1.15   05 Apr 1995 18:16:34   NN
   tabvwsh-include raus (Funktionen nach viewdata)

      Rev 1.14   04 Apr 1995 18:14:20   TRI
   Out of Keys

      Rev 1.13   30 Mar 1995 18:31:12   TRI
   Out of Keys

      Rev 1.12   27 Mar 1995 17:44:54   TRI
   Out of Keys - Aufteilung

      Rev 1.11   21 Mar 1995 11:12:18   MO
   242-Aenderungen

      Rev 1.10   15 Mar 1995 12:59:28   NN
   MarkAll

      Rev 1.9   03 Mar 1995 22:59:16   NN
   2.41 Anpassung

      Rev 1.8   27 Feb 1995 13:25:36   SC
   * syntax-Fehler korrigiert


      Rev 1.7   27 Feb 1995 08:53:52   NN
   Anordnung

      Rev 1.6   25 Feb 1995 20:18:46   NN
   Alignment-Funktionen

      Rev 1.5   17 Feb 1995 18:59:50   NN
   Paste mit Fenster

      Rev 1.4   15 Feb 1995 02:00:44   NN
   Cut/Copy/Paste

      Rev 1.3   14 Feb 1995 19:55:38   NN
   ObjectMenue (disabled)

      Rev 1.2   13 Feb 1995 20:59:58   NN
   Popup Menue

      Rev 1.1   12 Feb 1995 21:42:34   NN
   Drawing Objektleiste

      Rev 1.0   01 Feb 1995 18:21:42   NN
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


