/*************************************************************************
 *
 *  $RCSfile: drawvie4.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nn $ $Date: 2001-02-14 19:27:31 $
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

// -------------------------
//svtools
#define _SCRWIN_HXX
#define _RULER_HXX
#define _TABBAR_HXX //*
#define _VALUESET_HXX //*
#define _STDMENU_HXX
#define _STDCTRL_HXX //*
#define _CTRLBOX_HXX //*
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
//#define _SFXDOCINF_HXX
#define _SFXLINKHDL_HXX
//#define _SFX_PROGRESS_HXX

//-------------------------

//sfxsh.hxx
//#define _SFX_SHELL_HXX
//#define _SFXAPP_HXX
#define _SFXDISPATCH_HXX  //???
//#define _SFXMSG_HXX
//#define _SFXOBJFACE_HXX
//#define _SFXREQUEST_HXX
#define _SFXMACRO_HXX

// SFX
//#define _SFXAPPWIN_HXX
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX ***
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
//#define _SFXDOCFILE_HXX
//define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX
//#define _MDIFRM_HXX
//#define _SFX_IPFRM_HXX
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
//#define _SFXDOCKWIN_HXX //*

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
#define _SFXSTYLE_HXX //???

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


#define _SV_NOXSOUND

#ifdef WIN
#define _MENUBTN_HXX
#endif

#define _BASE_DLGS_HXX
#define _BIGINT_HXX
#define _CACHESTR_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX

#define _SVBOXITM_HXX
#define _SVDRAG_HXX
#define _SVINCVW_HXX
#define _SV_MULTISEL_HXX
#define _SVRTV_HXX
#define _SVTABBX_HXX

#define _SVX_FRMITEMS_HXX

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

#define SI_NOITEMS
//#define SI_NODRW
#define _SI_NOSBXCONTROLS
#define _VCATTR_HXX
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

//#define _SI_HXX
#define _SDR_NOTRANSFORM
#define _SDR_NOTOUCH
#define _SVDOPAGE_HXX       // SdrPageObj
#define _SVDSURO_HXX        // SdrObjSurrogate
#define _SVDOVIRT_HXX       // SdrVirtObj
#define _SVDOGRP_HXX        // SdrGroupObj
//#define _SVDOTEXT_HXX     // SdrTextObj
#define _SVDOPATH_HXX       // SdrPathObj
//#define _SVDOEDGE_HXX     // SdrEdgeObj
//#define _SVDORECT_HXX     // SdrRectObj (+SdrTextObj)
#define _SVDCAPT_HXX        // SdrCaptionObj
#define _SVDOCIRC_HXX       // SdrCircleObj
//#define _SVDOGRAF_HXX     // SdrGrafObj (+SdrRectObj)
//#define _SVDOOLE2_HXX     // SdrOle2Obj (+SdrRectObj)


//#define _SVDOATTR_HXX
//#define _SVDOTEXT_HXX

#define _SVDOUKWN_HXX
//#define _SVDORECT_HXX
#define _SVDCAPT_HXX
#define _SVDOCIRC_HXX
//#define _SVDOEDGE_HXX
//#define _SVDOGRAF_HXX
#define _SVDRAW_HXX
#define _SVDOGRP_HXX
#define _SVDOMEAS_HXX
//#define _SVDOOLE2_HXX
#define _SVDOPAGE_HXX
#define _SVDOPATH_HXX

// INCLUDE ---------------------------------------------------------------

#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>

#include "drawview.hxx"
#include "global.hxx"
#include "drwlayer.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "dataobj.hxx"
#include "drwtrans.hxx"
#include "drawutil.hxx"
#include "scmod.hxx"
#include "globstr.hrc"

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

Point aDragStartDiff;

// -----------------------------------------------------------------------

//! welche Funktionen aus drawview/drawvie4 muessen wirklich ohne Optimierung sein?

#ifdef WNT
#pragma optimize ( "", off )
#endif

// -----------------------------------------------------------------------

void lcl_InitMarks( SdrMarkView& rDest, const SdrMarkView& rSource, USHORT nTab )
{
    rDest.ShowPagePgNum( nTab, Point() );
    SdrPageView* pDestPV = rDest.GetPageViewPvNum(0);
    DBG_ASSERT(pDestPV,"PageView ??!?!");

    const SdrMarkList& rMarkList = rSource.GetMarkList();
    ULONG nCount = rMarkList.GetMarkCount();
    for (ULONG i=0; i<nCount; i++)
    {
        SdrMark* pMark = rMarkList.GetMark(i);
        SdrObject* pObj = pMark->GetObj();

        rDest.MarkObj(pObj, pDestPV);
    }
}

void lcl_CheckOle( const SdrMarkList& rMarkList, BOOL& rAnyOle, BOOL& rOneOle )
{
    rAnyOle = rOneOle = FALSE;

    ULONG nCount = rMarkList.GetMarkCount();
    if (nCount == 1)
    {
        SdrMark* pMark = rMarkList.GetMark(0);
        SdrObject* pObj = pMark->GetObj();
        UINT16 nSdrObjKind = pObj->GetObjIdentifier();
        if (nSdrObjKind == OBJ_OLE2)
            rAnyOle = rOneOle = TRUE;
    }
    else
        for (ULONG i=0; i<nCount; i++)
        {
            SdrMark* pMark = rMarkList.GetMark(i);
            SdrObject* pObj = pMark->GetObj();
            UINT16 nSdrObjKind = pObj->GetObjIdentifier();
            if (nSdrObjKind == OBJ_OLE2)
            {
                rAnyOle = TRUE;
                break;
            }
        }
}

BOOL ScDrawView::BeginDrag( Window* pWindow, const Point& rStartPos )
{
    BOOL bReturn = FALSE;

    if ( HasMarkedObj() )
    {
        ScModule* pScMod = SC_MOD();

        BrkAction();

        Rectangle aMarkedRect = GetAllMarkedRect();
        Region aRegion( aMarkedRect );

        aDragStartDiff = rStartPos - aMarkedRect.TopLeft();

        BOOL bAnyOle, bOneOle;
        const SdrMarkList& rMarkList = GetMarkList();
        lcl_CheckOle( rMarkList, bAnyOle, bOneOle );

//          PageView in MarkList nicht mehr gueltig, wenn Tabelle umgeschaltet  !!!
        SdrView aEditView( GetModel() );
        lcl_InitMarks( aEditView, *this, nTab );

        BegUndo( ScGlobal::GetRscString( STR_UNDO_DRAGDROP ) );

        //---------------------------------------------------------
        ScDocShellRef aDragShellRef;
        if (bAnyOle)
        {
            aDragShellRef = new ScDocShell;     // ohne Ref lebt die DocShell nicht !!!
            aDragShellRef->DoInitNew(NULL);
        }
        //---------------------------------------------------------

        ScDrawLayer::SetGlobalDrawPersist(aDragShellRef);
        SdrModel* pDragModel = GetAllMarkedModel();
        ScDrawLayer::SetGlobalDrawPersist(NULL);

        SvDataObjectRef pDragServer = new ScDataObject(pDragModel,
                                                        pViewData->GetDocShell(), bOneOle);
        pScMod->SetDragObject(pDragModel, &aEditView, 0);
        DropAction eDropAction = pDragServer->ExecuteDrag(pWindow,
                                        POINTER_MOVEDATA, POINTER_COPYDATA, POINTER_LINKDATA,
                                        DRAG_ALL, &aRegion);
        BOOL bIntern = pScMod->GetDragIntern();
        pScMod->ResetDragObject();
        pDragServer.Clear();            // enthaelt Referenzen auf pDragShell

        //---------------------------------------------------------

        switch (eDropAction)
        {
            case DROP_MOVE:
            case DROP_DISCARD:
                if (!bIntern)
                    aEditView.DeleteMarked();
                break;

            case DROP_NONE:
            case DROP_COPY:
            case DROP_LINK:
            case DROP_PRINT:
                break;

            default:
                break;
        }

        if (eDropAction != DROP_NONE)
            bReturn = TRUE;

        EndUndo();
    }

    return bReturn;
}

void ScDrawView::DoCopy()
{
    BOOL bAnyOle, bOneOle;
    const SdrMarkList& rMarkList = GetMarkList();
    lcl_CheckOle( rMarkList, bAnyOle, bOneOle );

    //---------------------------------------------------------
    delete ScGlobal::pDrawClipDocShellRef;
    if (bAnyOle)
    {
        ScGlobal::pDrawClipDocShellRef =
                        new ScDocShellRef(new ScDocShell);      // ohne Ref geht's nicht
        (*ScGlobal::pDrawClipDocShellRef)->DoInitNew(NULL);
        ScDrawLayer::SetGlobalDrawPersist(*ScGlobal::pDrawClipDocShellRef);
    }
    else
    {
        ScGlobal::pDrawClipDocShellRef = NULL;
        ScDrawLayer::SetGlobalDrawPersist(NULL);
    }
    //---------------------------------------------------------

    SdrModel* pModel = GetAllMarkedModel();
    ScDrawLayer::SetGlobalDrawPersist(NULL);

    ScDocShell* pDocSh = pViewData->GetDocShell();

    TransferableObjectDescriptor aObjDesc;
    pDocSh->FillTransferableObjectDescriptor( aObjDesc );
    aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
    // maSize is set in ScDrawTransferObj ctor

    ScDrawTransferObj* pTransferObj = new ScDrawTransferObj( pModel, pDocSh, aObjDesc );
    uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

    pTransferObj->CopyToClipboard();                    // system clipboard
    SC_MOD()->SetClipObject( NULL, pTransferObj );      // internal clipboard
}

//  Korrektur fuer 100% berechnen, unabhaengig von momentanen Einstellungen

void ScDrawView::CalcNormScale( Fraction& rFractX, Fraction& rFractY ) const
{
    Point aLogic = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = ScGlobal::nScreenPPTX;
    double nPPTY = ScGlobal::nScreenPPTY;

    if (pViewData)
        nPPTX /= pViewData->GetDocShell()->GetOutputFactor();

    USHORT nEndCol = 0;
    USHORT nEndRow = 0;
    pDoc->GetTableArea( nTab, nEndCol, nEndRow );
    if (nEndCol<20)
        nEndCol = 20;
    if (nEndRow<20)
        nEndRow = 20;

    Fraction aZoom(1,1);
    ScDrawUtil::CalcScale( pDoc, nTab, 0,0, nEndCol,nEndRow, pDev, aZoom,aZoom,
                            nPPTX, nPPTY, rFractX,rFractY );
}

void ScDrawView::SetMarkedOriginalSize()
{
    SdrUndoGroup* pUndoGroup = new SdrUndoGroup(*GetModel());

    const SdrMarkList& rMarkList = GetMarkList();
    long nDone = 0;
    ULONG nCount = rMarkList.GetMarkCount();
    for (ULONG i=0; i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
        USHORT nIdent = pObj->GetObjIdentifier();
        BOOL bDo = FALSE;
        Size aOriginalSize;
        if (nIdent == OBJ_OLE2)
        {
            SvInPlaceObjectRef xIPObj = ((SdrOle2Obj*)pObj)->GetObjRef();
            if (xIPObj.Is())
            {
                aOriginalSize = OutputDevice::LogicToLogic(
                                        xIPObj->GetVisArea().GetSize(),
                                        xIPObj->GetMapUnit(), MAP_100TH_MM );
                bDo = TRUE;
            }
        }
        else if (nIdent == OBJ_GRAF)
        {
            const Graphic& rGraphic = ((SdrGrafObj*)pObj)->GetGraphic();

            MapMode aSourceMap = rGraphic.GetPrefMapMode();
            MapMode aDestMap( MAP_100TH_MM );
            if (aSourceMap.GetMapUnit() == MAP_PIXEL)
            {
                //  Pixel-Korrektur beruecksichtigen, damit Bitmap auf dem Bildschirm stimmt

                Fraction aScaleX, aScaleY;
                CalcNormScale( aScaleX, aScaleY );
                aDestMap.SetScaleX(aScaleX);
                aDestMap.SetScaleY(aScaleY);
            }
            if (pViewData)
            {
                Window* pActWin = pViewData->GetActiveWin();
                if (pActWin)
                {
                    aOriginalSize = pActWin->LogicToLogic(
                                    rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );
                    bDo = TRUE;
                }
            }
        }

        if ( bDo )
        {
            Rectangle aDrawRect = pObj->GetLogicRect();

            pUndoGroup->AddAction( new SdrUndoGeoObj( *pObj ) );
            pObj->Resize( aDrawRect.TopLeft(), Fraction( aOriginalSize.Width(), aDrawRect.GetWidth() ),
                                                 Fraction( aOriginalSize.Height(), aDrawRect.GetHeight() ) );
            ++nDone;
        }
    }

    if (nDone)
    {
        pUndoGroup->SetComment(ScGlobal::GetRscString( STR_UNDO_ORIGINALSIZE ));
        pViewData->GetDocShell()->GetUndoManager()->AddUndoAction(pUndoGroup);
    }
    else
        delete pUndoGroup;
}


#ifdef WNT
#pragma optimize ( "", on )
#endif




