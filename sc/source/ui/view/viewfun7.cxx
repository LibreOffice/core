/*************************************************************************
 *
 *  $RCSfile: viewfun7.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-08 16:18:37 $
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

#define _SV_NOXSOUND

#ifdef WIN
    #define _MENUBTN_HXX
#endif

#define _BASE_DLGS_HXX
#define _BIGINT_HXX
#define _CACHESTR_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _CTRLTOOL_HXX
#define _DLGCFG_HXX
#define _DYNARR_HXX
#define _EXTATTR_HXX
#define _FILDLG_HXX
#define _FONTDLG_HXX
#define _FRM3D_HXX
#define _INTRO_HXX
#define _ISETBWR_HXX
#define _NO_SVRTF_PARSER_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
#define _OUTLINER_HXX
//#define _PRNDLG_HXX
//#define _POLY_HXX
#define _PVRWIN_HXX
#define _QUEUE_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
#define _SETBRW_HXX
#define _STACK_HXX
//#define _STATUS_HXX
#define _STDCTRL_HXX
#define _STDMENU_HXX
//#define _TAB_HXX
#define _TABBAR_HXX
#define _TREELIST_HXX
#define _VALUESET_HXX
#define _VCATTR_HXX
#define _VCBRW_HXX
#define _VCTRLS_HXX
//#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX


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
#define _FILDLG_HXX
//#define _PRNDLG_HXX
#define _COLDLG_HXX
//#define _TABDLG_HXX
#ifdef WIN
#define _MENU_HXX
#endif
//#define _GDIMTF_HXX
//#define _POLY_HXX
//#define _ACCEL_HXX
//#define _GRAPH_HXX
#define _SOUND_HXX


#define _PASSWD_HXX

#define _SFX_DOCFILE_HXX
#define _SFX_DOCFILT_HXX
#define _SFX_DOCSH_HXX
#define _SFXDOCFILE_HXX
#define _SFXDOCFILT_HXX
//#define _SFXDOCINF_HXX
//#define _SFXDOCSH_HXX
#define _SFX_PRNMON_HXX
#define _SFX_RESMGR_HXX
#define _SFX_TEMPLDLG_HXX
//#define _SFXAPPWIN_HXX
#define _SFXBASIC_HXX
#define _SFXCTRLITEM
#define _SFXDLGCFG_HXX
#define _SFXDISPATCH_HXX
#define _SFXFILEDLG_HXX
//#define _SFXIMGMGR_HXX
#define _SFXIPFRM_HXX
#define _SFX_MACRO_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXMULTISEL_HXX
//#define _SFXMSG_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFX_MINFITEM_HXX
#define _SFXOBJFACE_HXX
#define _SFXOBJFAC_HXX
#define _SFX_SAVEOPT_HXX
#define _SFXSTBITEM_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXCTRL_HXX
#define _SFXTBXMGR_HXX

//sfxdoc.hxx
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX
//#define _SFXDOCINF_HXX
//#define _SFX_OBJFAC_HXX
#define _SFX_DOCFILT_HXX
//#define _SFXDOCFILE_HXX ***
//define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX ***
//#define _SFXVIEWSH_HXX ***
//#define _MDIFRM_HXX ***
#define _SFX_IPFRM_HXX
//#define _SFX_INTERNO_HXX

//sfxdlg.hxx
//#define _SFXTABDLG_HXX ***
//#define _BASEDLGS_HXX ***
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
//#define _SFXDOCTEMPL_HXX ***
//#define _SFXDOCTDLG_HXX ***
//#define _SFX_TEMPLDLG_HXX ***
//#define _SFXNEW_HXX ***
#define _SFXDOCMAN_HXX
//#define _SFXDOCKWIN_HXX

//sfxitems.hxx
#define _SFX_WHMAP_HXX
#define _ARGS_HXX
//#define _SFXPOOLITEM_HXX
//#define _SFXINTITEM_HXX
//#define _SFXENUMITEM_HXX
#define _SFXFLAGITEM_HXX
//#define _SFXSTRITEM_HXX  ***
#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
//#define _SFXITEMPOOL_HXX ***
//#define _SFXITEMSET_HXX ***
#define _SFXITEMITER_HXX
#define _SFX_WHITER_HXX
#define _SFXPOOLCACH_HXX
#define _AEITEM_HXX
#define _SFXRNGITEM_HXX
#define _SFXSLSTITM_HXX
#define _SFXSTYLE_HXX

//xout.hxx
//#define _XENUM_HXX ***
//#define _XPOLY_HXX ***
//#define _XATTR_HXX ***
#define _XOUTX_HXX
//#define _XPOOL_HXX ***
#define _XTABLE_HXX

//svdraw.hxx
//#define _SVDRAW_HXX
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
//#define _SDR_NOUNDO ***
#define _SDR_NOXOUTDEV
//#define _SDR_NOOBJECTS ***
//#define _SDR_NOVIEWS ***

//inet
#define _INETINDP_HXX
#define _INETIMP_HXX
#define _INETWRAP_HXX
#define _INETKEEP_HXX
#define _PLUGMGR_HXX
#define _URLOBJ_HXX //*

#define SFX_NOCLOOKS

#define _SI_DLL_HXX
#define _SIDLL_HXX
#define _SI_NOITEMS
#define _SI_NOOTHERFORMS
#define _SI_NOSBXCONTROLS
#define _SINOSBXCONTROLS
#define _SI_NODRW
#define _SI_NOCONTROL
#define _SVBOXITM_HXX
#define _SVCONTNR_HXX
//#define _SVDATTR_HXX ***
#define _SVDEC_HXX
#define _SVDIO_HXX
#define _SVDRAG_HXX
#define _SVINCVW_HXX
#define _SV_MULTISEL_HXX
#define _SVRTV_HXX
#define _SVTABBX_HXX
#define _SVTREEBOX_HXX
#define _SVTREELIST_HXX

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_LAYCTRL_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SELCTRL_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_STDDLG_HXX
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


// INCLUDE ---------------------------------------------------------------

#include <so3/ipobj.hxx>
#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/xbitmap.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xoutbmp.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>

#include "document.hxx"     // fuer MapMode Initialisierung in PasteDraw
#include "viewfunc.hxx"
#include "tabvwsh.hxx"
#include "drawview.hxx"
#include "scmod.hxx"
#include "drwlayer.hxx"
#include "globstr.hrc"

extern Point aDragStartDiff;

// STATIC DATA -----------------------------------------------------------

BOOL bPasteIsMove = FALSE;

//==================================================================

void lcl_AdjustInsertPos( ScViewData* pData, Point& rPos, Size& rSize )
{
//  SdrPage* pPage = pData->GetDocument()->GetDrawLayer()->GetPage( pData->GetTabNo() );
    SdrPage* pPage = pData->GetScDrawView()->GetModel()->GetPage( pData->GetTabNo() );
    DBG_ASSERT(pPage,"pPage ???");
    Size aPgSize( pPage->GetSize() );
    long x = aPgSize.Width() - rPos.X() - rSize.Width();
    long y = aPgSize.Height() - rPos.Y() - rSize.Height();
    // ggf. Ajustments (80/200) fuer Pixel-Rundungsfehler
    if( x < 0 )
        rPos.X() += x + 80;
    if( y < 0 )
        rPos.Y() += y + 200;
    rPos.X() += rSize.Width() / 2;          // Position bei Paste gibt Mittelpunkt an
    rPos.Y() += rSize.Height() / 2;
}

void ScViewFunc::PasteDraw( const Point& rLogicPos, SdrModel* pModel, BOOL bGroup )
{
    MakeDrawLayer();
    Point aPos( rLogicPos );

    //  #64184# MapMode am Outliner-RefDevice muss stimmen (wie in FuText::MakeOutliner)
    //! mit FuText::MakeOutliner zusammenfassen?
    MapMode aOldMapMode;
    OutputDevice* pRef = GetViewData()->GetDocument()->GetDrawLayer()->GetRefDevice();
    if (pRef)
    {
        aOldMapMode = pRef->GetMapMode();
        pRef->SetMapMode( MapMode(MAP_100TH_MM) );
    }

    SdrView* pDragEditView = SC_MOD()->GetDragData().pSdrView;
    if (pDragEditView)
    {
        aPos -= aDragStartDiff;
        if (aPos.X() < 0) aPos.X() = 0;
        if (aPos.Y() < 0) aPos.Y() = 0;
    }

    ScDrawView* pDrawView = GetScDrawView();
    if (bGroup)
        pDrawView->BegUndo( ScGlobal::GetRscString( STR_UNDO_PASTE ) );

    BOOL bSameDoc = ( pDragEditView && pDragEditView->GetModel() == pDrawView->GetModel() );
    if (bSameDoc)
    {
            // lokal kopieren - incl. Charts

        Point aSourceStart = pDragEditView->GetAllMarkedRect().TopLeft();
        long nDiffX = aPos.X() - aSourceStart.X();
        long nDiffY = aPos.Y() - aSourceStart.Y();

            // innerhalb einer Page verschieben?

        if ( bPasteIsMove &&
                pDrawView->GetPageViewPvNum(0)->GetPage() ==
                pDragEditView->GetPageViewPvNum(0)->GetPage() )
        {
            pDragEditView->MoveAllMarked(Size(nDiffX,nDiffY), FALSE);
        }
        else
        {
            SdrModel* pModel = pDragEditView->GetModel();
            SdrPage* pDestPage = pModel->GetPage( GetViewData()->GetTabNo() );
            DBG_ASSERT(pDestPage,"nanu, Page?");

            SdrMarkList aMark = pDragEditView->GetMarkList();
            aMark.ForceSort();
            ULONG nMarkAnz=aMark.GetMarkCount();
            for (ULONG nm=0; nm<nMarkAnz; nm++) {
                const SdrMark* pM=aMark.GetMark(nm);
                const SdrObject* pObj=pM->GetObj();
                SdrObject* pNeuObj=pObj->Clone(pDestPage,pModel);
                if (pNeuObj!=NULL)
                {
                    //  #68787# copy graphics within the same model - always needs new name
                    if ( pNeuObj->ISA(SdrGrafObj) && !bPasteIsMove )
                        pNeuObj->SetName(((ScDrawLayer*)pModel)->GetNewGraphicName());

                    if (nDiffX!=0 || nDiffY!=0)
                        pNeuObj->NbcMove(Size(nDiffX,nDiffY));
                    pDestPage->InsertObject( pNeuObj );
                    pDrawView->AddUndo(new SdrUndoInsertObj( *pNeuObj ));

                    //  Chart braucht nicht mehr getrennt behandelt zu werden,
                    //  weil es seine Daten jetzt selber hat
                }
            }

            if (bPasteIsMove)
                pDragEditView->DeleteMarked();
        }
    }
    else
    {
        bPasteIsMove = FALSE;       // kein internes Verschieben passiert

        SdrMarkView aView(pModel);
        SdrPageView* pPv = aView.ShowPagePgNum(0,Point());
        aView.MarkAll(pPv);
        Size aSize = aView.GetAllMarkedRect().GetSize();
        lcl_AdjustInsertPos( GetViewData(), aPos, aSize );

        //  #41333# Markierung nicht aendern, wenn Ole-Objekt aktiv
        //  (bei Drop aus Ole-Objekt wuerde sonst mitten im ExecuteDrag deaktiviert!)

        ULONG nOptions = 0;
        SfxInPlaceClient* pClient = GetViewData()->GetViewShell()->GetIPClient();
        if ( pClient && pClient->IsInPlaceActive() )
            nOptions |= SDRINSERT_DONTMARK;

        pDrawView->Paste( *pModel, aPos, NULL, nOptions );

        // #68991# Paste puts all objects on the active (front) layer
        // controls must be on SC_LAYER_CONTROLS

        USHORT nTab = GetViewData()->GetTabNo();
        SdrPage* pPage = pDrawView->GetModel()->GetPage(nTab);
        DBG_ASSERT(pPage,"Page?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->ISA(SdrUnoObj) && pObject->GetLayer() != SC_LAYER_CONTROLS )
                    pObject->NbcSetLayer(SC_LAYER_CONTROLS);
                pObject = aIter.Next();
            }
        }

        // #75299# all graphics objects must have names
        GetViewData()->GetDocument()->EnsureGraphicNames();
    }

    if (bGroup)
    {
        pDrawView->GroupMarked();
        pDrawView->EndUndo();
    }

    if (pRef)
        pRef->SetMapMode( aOldMapMode );

    GetViewData()->GetViewShell()->SetDrawShell( TRUE );
}

BOOL ScViewFunc::PasteObject( const Point& rPos, SvInPlaceObject* pObj,
                                SvDataObject* pDataObject )
{
    MakeDrawLayer();
    if ( pObj )
    {
        SvEmbeddedInfoObject* pInfoObj = GetViewData()->GetViewShell()->
                                            GetViewFrame()->GetObjectShell()->
                                            InsertObject(pObj, String());
        String aName = pInfoObj->GetObjName();

        MapMode aMap100( MAP_100TH_MM );
        MapMode aMapObj( pObj->GetMapUnit() );

        Size aSize;
        if (pDataObject)        // Size aus DataObject in IPObject uebernehmen
        {
            SvObjectDescriptor aDesc(pDataObject);
            Size aDescSize = aDesc.GetSize();
            if( aDescSize.Width() && aDescSize.Height() )
                pObj->SetVisAreaSize(OutputDevice::LogicToLogic( aDescSize, aMap100, aMapObj ));
        }

        aSize = pObj->GetVisArea().GetSize();
        aSize = OutputDevice::LogicToLogic( aSize, aMapObj, aMap100 );  // fuer SdrOle2Obj

        if( aSize.Height() == 0 || aSize.Width() == 0 )
        {
            DBG_ERROR("SvObjectDescriptor::GetSize == 0");
            aSize.Width() = 5000;
            aSize.Height() = 5000;
            aSize = OutputDevice::LogicToLogic( aSize, aMap100, aMapObj );
            pObj->SetVisAreaSize(aSize);
        }
        // kein AdjustInsertPos
        Rectangle aRect( rPos, aSize );

        ScDrawView* pDrView = GetScDrawView();
        SdrOle2Obj* pSdrObj = new SdrOle2Obj( pObj, aName, aRect );
        SdrPageView* pPV = pDrView->GetPageViewPvNum(0);
        pDrView->InsertObjectSafe( pSdrObj, *pPV );             // nicht markieren wenn Ole
        pSdrObj->SetOleLink(pInfoObj);
        GetViewData()->GetViewShell()->SetDrawShell( TRUE );
        return TRUE;
    }
    else
        return FALSE;
}

BOOL ScViewFunc::PasteBitmap( const Point& rPos, const Bitmap& rBmp )
{
    String aEmpty;
    Graphic aGraphic(rBmp);
    return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

BOOL ScViewFunc::PasteMetaFile( const Point& rPos, const GDIMetaFile& rMtf )
{
    String aEmpty;
    Graphic aGraphic(rMtf);
    return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

BOOL ScViewFunc::PasteGraphic( const Point& rPos, const Graphic& rGraphic,
                                const String& rFile, const String& rFilter )
{
    MakeDrawLayer();
    ScDrawView* pDrawView = GetScDrawView();

    Point aPos( rPos );
    Window* pWin = GetActiveWin();
    MapMode aSourceMap = rGraphic.GetPrefMapMode();
    MapMode aDestMap( MAP_100TH_MM );

    if (aSourceMap.GetMapUnit() == MAP_PIXEL)
    {
            //  Pixel-Korrektur beruecksichtigen, damit Bitmap auf dem Bildschirm stimmt

        Fraction aScaleX, aScaleY;
        pDrawView->CalcNormScale( aScaleX, aScaleY );
        aDestMap.SetScaleX(aScaleX);
        aDestMap.SetScaleY(aScaleY);
    }

    Size aSize = pWin->LogicToLogic( rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );
//  lcl_AdjustInsertPos( GetViewData(), aPos, aSize );
    GetViewData()->GetViewShell()->SetDrawShell( TRUE );

    Rectangle aRect(aPos, aSize);
    SdrGrafObj* pGrafObj = new SdrGrafObj(rGraphic, aRect);

    if (rFile.Len())
        pGrafObj->SetGraphicLink( rFile, rFilter );

    //  #49961# Pfad wird nicht mehr als Name der Grafik gesetzt

    ScDrawLayer* pLayer = (ScDrawLayer*) pDrawView->GetModel();
    String aName = pLayer->GetNewGraphicName();                 // "Grafik x"
    pGrafObj->SetName(aName);

    // nicht markieren wenn Ole
    pDrawView->InsertObjectSafe(pGrafObj, *pDrawView->GetPageViewPvNum(0));
    return TRUE;
}

BOOL ScViewFunc::ApplyGraphicToObject( SdrObject* pPickObj, const Graphic& rGraphic )
{
    BOOL bRet = FALSE;
    SdrGrafObj* pNewGrafObj = NULL;

    ScDrawView* pDrawView = GetScDrawView();
    if ( pDrawView && pPickObj )
    {
        /**********************************************************************
        * Objekt neu attributieren
        **********************************************************************/
        SdrPageView* pPV = pDrawView->GetPageViewPvNum(0);
        if (pPickObj->ISA(SdrGrafObj))
        {
            /******************************************************************
            * Das Graphik-Objekt bekommt eine neue Graphik
            ******************************************************************/
            pNewGrafObj = (SdrGrafObj*) pPickObj->Clone();
            pNewGrafObj->SetGraphic(rGraphic);

            pDrawView->BegUndo(ScGlobal::GetRscString(STR_UNDO_DRAGDROP));
            pDrawView->ReplaceObject(pPickObj, *pPV, pNewGrafObj);
            pDrawView->EndUndo();

            bRet = TRUE;
        }
        else if (pPickObj->IsClosedObj() && !pPickObj->ISA(SdrOle2Obj))
        {
            /******************************************************************
            * Das Objekt wird mit der Graphik gefuellt
            ******************************************************************/
            //pDrawView->BegUndo(ScGlobal::GetRscString(STR_UNDO_DRAGDROP));
            pDrawView->AddUndo(new SdrUndoAttrObj(*pPickObj));
            //pDrawView->EndUndo();

            XOBitmap aXOBitmap( rGraphic.GetBitmap() );
            SfxItemSet aSet( pDrawView->GetModel()->GetItemPool(),
                                XATTR_FILLSTYLE, XATTR_FILLBITMAP );
            aSet.Put(XFillStyleItem(XFILL_BITMAP));
            aSet.Put(XFillBitmapItem(String(), aXOBitmap));

//-/            pPickObj->SetAttributes(aSet, FALSE);
            pPickObj->SetItemSetAndBroadcast(aSet);

            bRet = TRUE;
        }
    }
    return bRet;
}


