/*************************************************************************
 *
 *  $RCSfile: tabvwshh.cxx,v $
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
//#define _CONFIG_HXX
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
#define _MOREBTN_HXX
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
#define _SVTREELIST_HXX
#define _FILTER_HXX
#define _SVLBOXITM_HXX
#define _SVTREEBOX_HXX
#define _SVICNVW_HXX
#define _SVTABBX_HXX

//sfxcore.hxx
#define _SFXINIMGR_HXX
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
#define _SFXMSG_HXX
#define _SFXOBJFACE_HXX
//#define _SFXREQUEST_HXX
#define _SFXMACRO_HXX

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

//sfxdoc.hxx
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX
//#define _SFXDOCINF_HXX
//#define _SFX_OBJFAC_HXX
#define _SFX_DOCFILT_HXX
#define _SFXDOCFILE_HXX
//define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX
#define _MDIFRM_HXX
//#define _SFX_IPFRM_HXX ***
//#define _SFX_INTERNO_HXX

//sfxdlg.hxx
//#define _SFXTABDLG_HXX
#define _BASEDLGS_HXX
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
//#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
#define _SFXDOCTEMPL_HXX
#define _SFXDOCTDLG_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXNEW_HXX
#define _SFXDOCMAN_HXX
//#define _SFXDOCKWIN_HXX ***

//sfxitems.hxx
#define _SFX_WHMAP_HXX
//#define _ARGS_HXX ***
//#define _SFXPOOLITEM_HXX
//#define _SFXINTITEM_HXX
//#define _SFXENUMITEM_HXX
#define _SFXFLAGITEM_HXX
//#define _SFXSTRITEM_HXX
#define _SFXPTITEM_HXX
//#define _SFXRECTITEM_HXX ***
//#define _SFXITEMPOOL_HXX
//#define _SFXITEMSET_HXX
#define _SFXITEMITER_HXX
//#define _SFX_WHITER_HXX ***
#define _SFXPOOLCACH_HXX
//#define _AEITEM_HXX
//#define _SFXRNGITEM_HXX ***
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
#define _SIDLL_HXX

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

// svdraw.hxx
#define _SDR_NOITEMS
#define _SVDXOUT_HXX
#define _SDR_NOTRANSFORM        // Transformationen, selten verwendet
#define _SDR_NOTOUCH            // Hit-Tests, selten verwendet
#define _SDR_NOEXTDEV           // ExtOutputDevice
//#define   _SDR_NOUNDO             // Undo-Objekte
#define _SDR_NOSURROGATEOBJ     // SdrObjSurrogate
#define _SDR_NOPAGEOBJ          // SdrPageObj
#define _SDR_NOVIRTOBJ          // SdrVirtObj
#define _SDR_NOGROUPOBJ         // SdrGroupObj
#define _SDR_NOTEXTOBJ          // SdrTextObj //*
#define _SDR_NOPATHOBJ          // SdrPathObj
#define _SDR_NOEDGEOBJ          // SdrEdgeObj
#define _SDR_NORECTOBJ          // SdrRectObj
#define _SDR_NOCAPTIONOBJ       // SdrCaptionObj
#define _SDR_NOCIRCLEOBJ        // SdrCircleObj
#define _SDR_NOGRAFOBJ          // SdrGrafObj
#define _SDR_NOOLE2OBJ          // SdrOle2Obj

// INCLUDE ---------------------------------------------------------------

#include <svx/svdmark.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <svtools/sbxcore.hxx>
#include <so3/ipobj.hxx>
#include <svtools/whiter.hxx>

#include "tabvwsh.hxx"
#include "client.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "sc.hrc"

//------------------------------------------------------------------

void ScTabViewShell::ExecuteSbx( SfxRequest& rReq )
{
    //  SID_RANGE_OFFSET (Offset),
    //  SID_PIVOT_CREATE (DataPilotCreate) - removed (old Basic)
}

void ScTabViewShell::GetSbxState( SfxItemSet& rSet )
{
    //  SID_RANGE_REGION (CurrentRegion) - removed (old Basic)
}

//------------------------------------------------------------------

void ScTabViewShell::ExecuteObject( SfxRequest& rReq )
{
    USHORT nSlotId = rReq.GetSlot();
    const SfxItemSet* pReqArgs = rReq.GetArgs();

        //  Objekte aktivieren/deaktivieren immer auf der sichtbaren View

    ScTabViewShell* pVisibleSh = this;
    if ( nSlotId == SID_OLE_SELECT || nSlotId == SID_OLE_ACTIVATE || nSlotId == SID_OLE_DEACTIVATE )
    {
        DBG_ERROR("old slot SID_OLE...");
    }

    switch (nSlotId)
    {
        case SID_OLE_SELECT:
        case SID_OLE_ACTIVATE:
            {
                //  in beiden Faellen erstmal auf der sichtbaren View selektieren

                String aName;
                SdrView* pDrView = GetSdrView();
                if (pDrView)
                {
                    const SdrMarkList& rMarkList = pDrView->GetMarkList();
                    if (rMarkList.GetMarkCount() == 1)
                        aName = rMarkList.GetMark(0)->GetObj()->GetName();
                }
                pVisibleSh->SelectObject( aName );

                //  aktivieren

                if ( nSlotId == SID_OLE_ACTIVATE )
                    pVisibleSh->DoVerb( 0 );
            }
            break;
        case SID_OLE_DEACTIVATE:
            {
                ScClient* pClient = (ScClient*) pVisibleSh->GetIPClient();
                if ( pClient && pClient->IsInPlaceActive() )
                {
                    pClient->GetProtocol().Reset2Open();
                    SFX_APP()->SetViewFrame(pVisibleSh->GetViewFrame());
                }
            }
            break;

        case SID_OBJECT_LEFT:
        case SID_OBJECT_TOP:
        case SID_OBJECT_WIDTH:
        case SID_OBJECT_HEIGHT:
            {
                BOOL bDone = FALSE;
                const SfxPoolItem* pItem;
                if ( pReqArgs && pReqArgs->GetItemState( nSlotId, TRUE, &pItem ) == SFX_ITEM_SET )
                {
                    long nNewVal = ((const SfxInt32Item*)pItem)->GetValue();
                    if ( nNewVal < 0 )
                        nNewVal = 0;

                    //! von irgendwas in 1/100mm umrechnen ??????

                    SdrView* pDrView = GetSdrView();
                    if ( pDrView )
                    {
                        const SdrMarkList& rMarkList = pDrView->GetMarkList();
                        if (rMarkList.GetMarkCount() == 1)
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
                            Rectangle aRect = pObj->GetLogicRect();

                            if ( nSlotId == SID_OBJECT_LEFT )
                                pDrView->MoveMarkedObj( Size( nNewVal - aRect.Left(), 0 ) );
                            else if ( nSlotId == SID_OBJECT_TOP )
                                pDrView->MoveMarkedObj( Size( 0, nNewVal - aRect.Top() ) );
                            else if ( nSlotId == SID_OBJECT_WIDTH )
                                pDrView->ResizeMarkedObj( aRect.TopLeft(),
                                                Fraction( nNewVal, aRect.GetWidth() ),
                                                Fraction( 1, 1 ) );
                            else // if ( nSlotId == SID_OBJECT_HEIGHT )
                                pDrView->ResizeMarkedObj( aRect.TopLeft(),
                                                Fraction( 1, 1 ),
                                                Fraction( nNewVal, aRect.GetHeight() ) );
                            bDone = TRUE;
                        }
                    }
                }
                if (!bDone)
                    SbxBase::SetError( SbxERR_BAD_PARAMETER );  // Basic-Fehler
            }
            break;

    }
}

SvInPlaceObjectRef lcl_GetSelectedObj( SdrView* pDrView )       //! Member von ScDrawView?
{
    SvInPlaceObjectRef xRet;

    if (pDrView)
    {
        const SdrMarkList& rMarkList = pDrView->GetMarkList();
        if (rMarkList.GetMarkCount() == 1)
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
            if (pObj->GetObjIdentifier() == OBJ_OLE2)
            {
                SdrOle2Obj* pOle2Obj = (SdrOle2Obj*) pObj;
                xRet = pOle2Obj->GetObjRef();
            }
        }
    }

    return xRet;
}

void ScTabViewShell::GetObjectState( SfxItemSet& rSet )
{
    //  SID_OLE_OBJECT - removed (old Basic)

    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch (nWhich)
        {
            case SID_ACTIVE_OBJ_NAME:
                {
                    String aName;
                    SvInPlaceObjectRef xOLE = lcl_GetSelectedObj( GetSdrView() );
                    if (xOLE.Is())
                    {
                        SvInfoObject* pInfoObj = GetViewData()->GetSfxDocShell()->Find( xOLE );
                        if ( pInfoObj )
                            aName = pInfoObj->GetObjName();
                    }
                    rSet.Put( SfxStringItem( nWhich, aName ) );
                }
                break;
            case SID_OBJECT_LEFT:
            case SID_OBJECT_TOP:
            case SID_OBJECT_WIDTH:
            case SID_OBJECT_HEIGHT:
                {
                    SdrView* pDrView = GetSdrView();
                    if ( pDrView )
                    {
                        const SdrMarkList& rMarkList = pDrView->GetMarkList();
                        if (rMarkList.GetMarkCount() == 1)
                        {
                            SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
                            Rectangle aRect = pObj->GetLogicRect();

                            long nVal;
                            if ( nWhich == SID_OBJECT_LEFT )
                                nVal = aRect.Left();
                            else if ( nWhich == SID_OBJECT_TOP )
                                nVal = aRect.Top();
                            else if ( nWhich == SID_OBJECT_WIDTH )
                                nVal = aRect.GetWidth();
                            else // if ( nWhich == SID_OBJECT_HEIGHT )
                                nVal = aRect.GetHeight();

                            //! von 1/100mm in irgendwas umrechnen ??????

                            rSet.Put( SfxInt32Item( nWhich, nVal ) );
                        }
                    }
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}





