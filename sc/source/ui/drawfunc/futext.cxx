/*************************************************************************
 *
 *  $RCSfile: futext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:49:37 $
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
#define _STATUS_HXX
#define _SVTCTRL3_HXX
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
//#define _SFXDOCKWIN_HXX **

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
#define _VCATTR_HXX
#define _VCONT_HXX
//#define _VCSBX_HXX ***
#define _SI_NOOTHERFORMS
#define _VCTRLS_HXX
//#define _VCDRWOBJ_HXX ***
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS
#define _SIDLL_HXX

//------------------------------------------------------------------

#include <svx/svddef.hxx>
#include <svx/svdoutl.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdview.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>

#include "futext.hxx"
#include "drwlayer.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"

//  Maximal erlaubte Mausbewegung um noch Drag&Drop zu starten
//! fusel,fuconstr,futext - zusammenfassen!
#define SC_MAXDRAGMOVE  3

//------------------------------------------------------------------

void lcl_InvalidateAttribs( SfxBindings& rBindings )
{
    rBindings.Invalidate( SID_ATTR_CHAR_WEIGHT );
    rBindings.Invalidate( SID_ATTR_CHAR_POSTURE );
    rBindings.Invalidate( SID_ATTR_CHAR_UNDERLINE );
    rBindings.Invalidate( SID_ULINE_VAL_NONE );
    rBindings.Invalidate( SID_ULINE_VAL_SINGLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOUBLE );
    rBindings.Invalidate( SID_ULINE_VAL_DOTTED );
    rBindings.Invalidate( SID_ATTR_CHAR_COLOR );
    rBindings.Invalidate( SID_ATTR_CHAR_FONT );
    rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_LEFT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_CENTER );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_BLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_10 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_15 );
    rBindings.Invalidate( SID_ATTR_PARA_LINESPACE_20 );
    rBindings.Invalidate( SID_SET_SUPER_SCRIPT );
    rBindings.Invalidate( SID_SET_SUB_SCRIPT );
    rBindings.Invalidate( SID_HYPERLINK_GETLINK );
}

/*************************************************************************
|*
|* Basisklasse fuer Textfunktionen
|*
\************************************************************************/

FuText::FuText(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
                   SdrModel* pDoc, SfxRequest& rReq) :
    FuConstruct(pViewSh, pWin, pView, pDoc, rReq),
    pTextObj(NULL)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuText::~FuText()
{
//  StopEditMode();                 // in Deactivate !
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL __EXPORT FuText::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    if ( pView->MouseButtonDown(rMEvt, pWindow) )
        return (TRUE);                 // Event von der SdrView ausgewertet

    if ( pView->IsTextEdit() )
    {
        StopEditMode();                    // Danebengeklickt, Ende mit Edit
        pView->SetCreateMode();
    }

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        SdrHdl* pHdl = pView->HitHandle(aMDPos, *pWindow);

        ULONG nHdlNum = pView->GetHdlNum(pHdl);

        if (pHdl != NULL)
        {
            if (pView->HasMarkablePoints() && pView->IsPointMarkable(*pHdl))
            {
                BOOL bPointMarked=pView->IsPointMarked(*pHdl);

                if ( rMEvt.IsShift() )
                {
                    if (!bPointMarked)
                    {
                        pView->MarkPoint(*pHdl);
                    }
                    else
                    {
                        pView->UnmarkPoint(*pHdl);
                    }
                }
                else
                {
                    if (!bPointMarked)
                    {
                        pView->UnmarkAllPoints();
                        pView->MarkPoint(*pHdl);
                    }
                }
                pHdl=pView->GetHdl(nHdlNum);
            }
        }

        SdrObject* pObj;
        SdrPageView* pPV;

        if ( pHdl != NULL || pView->IsMarkedHit(aMDPos) )
        {
            if (pHdl == NULL &&
//              pView->TakeTextEditObject(aMDPos, pObj, pPV) )
                pView->PickObj(aMDPos, pObj, pPV, SDRSEARCH_PICKTEXTEDIT) )
            {
                SdrOutliner* pO = MakeOutliner();
                //!??   ohne uebergebenen Outliner stimmen die Defaults nicht ???!?
                if ( pView->BegTextEdit(pObj, pPV, pWindow, (FASTBOOL)TRUE, pO) )
                {
                    //  EditEngine-UndoManager anmelden
                    pViewShell->SetDrawTextUndo( &pO->GetUndoManager() );

                    OutlinerView* pOLV = pView->GetTextEditOutlinerView();
                    if ( pOLV->MouseButtonDown(rMEvt) )
                        return (TRUE); // Event an den Outliner
                }
            }
            else
            {
                aDragTimer.Start();
                pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl);
            }
        }
        else
        {
            BOOL bMacro = FALSE;

//          if (bMacro && pView->TakeMacroObject(aMDPos,pObj,pPV))
            if (bMacro && pView->PickObj(aMDPos, pObj, pPV, SDRSEARCH_PICKMACRO) )

            {
                pView->BegMacroObj(aMDPos,pObj,pPV,pWindow);
            }
            else
            {
                if (pView->IsEditMode())
                {
                    BOOL bPointMode=pView->HasMarkablePoints();

                    if (!rMEvt.IsShift())
                    {
                        if (bPointMode)
                        {
                            pView->UnmarkAllPoints();
                        }
                        else
                        {
                            pView->UnmarkAll();
                        }

                        pView->SetDragMode(SDRDRAG_MOVE);
                        SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
                        rBindings.Invalidate( SID_OBJECT_ROTATE );
                        rBindings.Invalidate( SID_OBJECT_MIRROR );
                        pHdl=pView->GetHdl(nHdlNum);
                    }

                    if ( pView->MarkObj(aMDPos, -2, FALSE, rMEvt.IsMod1()) )
                    {
                        aDragTimer.Start();

                        pHdl=pView->HitHandle(aMDPos,*pWindow);

                        if (pHdl!=NULL)
                        {
                            pView->MarkPoint(*pHdl);
                            pHdl=pView->GetHdl(nHdlNum);
                        }

                        pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl);
                    }
                    else
                    {
                        if (bPointMode)
                        {
                            pView->BegMarkPoints(aMDPos, (OutputDevice*) NULL);
                        }
                        else
                        {
                            pView->BegMarkObj(aMDPos, (OutputDevice*) NULL);
                        }
                    }
                }
                else if (aSfxRequest.GetSlot() == SID_DRAW_NOTEEDIT )
                {
                    //  Notizen editieren -> keine neuen Textobjekte erzeugen,
                    //  stattdessen Textmodus verlassen

                    pViewShell->GetViewData()->GetDispatcher().
                        Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
                }
                else
                {
                    /**********************************************************
                    * Objekt erzeugen
                    **********************************************************/
                    pView->BegCreateObj(aMDPos, (OutputDevice*) NULL);
                }
            }
        }
    }

    if (!bIsInDragMode)
    {
        pWindow->CaptureMouse();
//      ForcePointer(&rMEvt);
        lcl_InvalidateAttribs( pViewShell->GetViewFrame()->GetBindings() );
    }

    pViewShell->SetActivePointer(pView->GetPreferedPointer(
                    pWindow->PixelToLogic(rMEvt.GetPosPixel()), pWindow ));

//  return (bReturn);
    return TRUE;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL __EXPORT FuText::MouseMove(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

//  pViewShell->SetActivePointer(aNewPointer);

    pViewShell->SetActivePointer(pView->GetPreferedPointer(
                    pWindow->PixelToLogic(rMEvt.GetPosPixel()), pWindow ));

    if (aDragTimer.IsActive() )
    {
        Point aOldPixel = pWindow->LogicToPixel( aMDPos );
        Point aNewPixel = rMEvt.GetPosPixel();
        if ( Abs( aOldPixel.X() - aNewPixel.X() ) > SC_MAXDRAGMOVE ||
             Abs( aOldPixel.Y() - aNewPixel.Y() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    if ( pView->MouseMove(rMEvt, pWindow) )
        return (TRUE); // Event von der SdrView ausgewertet

    if ( pView->IsAction() )
    {
/*      aNewPointer = Pointer(POINTER_TEXT);
        pViewShell->SetActivePointer(aNewPointer);
*/
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt(pWindow->PixelToLogic(aPix));

        ForceScroll(aPix);
        pView->MovAction(aPnt);
    }

//  ForcePointer(&rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL __EXPORT FuText::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    lcl_InvalidateAttribs( pViewShell->GetViewFrame()->GetBindings() );

    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( pView->MouseButtonUp(rMEvt, pWindow) )
        return (TRUE); // Event von der SdrView ausgewertet

    if ( pView->IsDragObj() )
    {
        pView->EndDragObj( rMEvt.IsShift() );
        pView->ForceMarkedToAnotherPage();
    }
    else if ( pView->IsCreateObj() )
    {
        if (rMEvt.IsLeft())
        {
            pView->EndCreateObj(SDRCREATE_FORCEEND);
            if (aSfxRequest.GetSlot() == SID_DRAW_TEXT_MARQUEE)
            {
                //  Lauftext-Objekt erzeugen?

                const SdrMarkList& rMarkList = pView->GetMarkList();
                if (rMarkList.GetMark(0))
                {
                    SdrObject* pObj = rMarkList.GetMark(0)->GetObj();

                    // die fuer das Scrollen benoetigten Attribute setzen
                    SfxItemSet aItemSet( pDrDoc->GetItemPool(),
                                            SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);

                    aItemSet.Put( SdrTextAutoGrowWidthItem( FALSE ) );
                    aItemSet.Put( SdrTextAutoGrowHeightItem( FALSE ) );
                    aItemSet.Put( SdrTextAniKindItem( SDRTEXTANI_SLIDE ) );
                    aItemSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                    aItemSet.Put( SdrTextAniCountItem( 1 ) );
                    aItemSet.Put( SdrTextAniAmountItem(
                                    (INT16)pWindow->PixelToLogic(Size(2,1)).Width()) );
                    pObj->SetAttributes(aItemSet, FALSE);
                }
            }
            SetInEditMode();

                //  Modus verlassen bei einzelnem Klick
                //  (-> fuconstr)

            if ( !pView->HasMarkedObj() )
            {
                pView->MarkObj(aPnt, -2, FALSE, rMEvt.IsMod1());

                SfxDispatcher& rDisp = pViewShell->GetViewData()->GetDispatcher();
                if ( pView->HasMarkedObj() )
                    rDisp.Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
                else
                    rDisp.Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            }
        }
    }
    else if ( pView->IsAction() )
    {
        pView->EndAction();
    }

/*  aNewPointer = Pointer(POINTER_TEXT);
    pViewShell->SetActivePointer(aNewPointer);
*/
    ForcePointer(&rMEvt);
    pWindow->ReleaseMouse();

    if ( !pView->HasMarkedObj() )
    {
        SdrObject* pObj;
        SdrPageView* pPV;

        if ( pView->PickObj(aPnt, pObj, pPV) )
        {
            if ( pView->MarkObj(aPnt, -2, FALSE, rMEvt.IsMod1()) )
            {
                UINT16 nSdrObjKind = pObj->GetObjIdentifier();

                if (nSdrObjKind != OBJ_TEXT &&
                    nSdrObjKind != OBJ_TITLETEXT &&
                    nSdrObjKind != OBJ_OUTLINETEXT &&
                    ! pObj->ISA(SdrTextObj) )
                {
//                  pViewShell->GetViewData()->GetDispatcher().Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                }
            }
        }
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Maus-Pointer umschalten
|*
\************************************************************************/

void FuText::ForcePointer(const MouseEvent* pMEvt)
{
    pViewShell->SetActivePointer( aNewPointer );

/*
    if ( !pView->IsAction() )
    {
        Point aPnt(pWindow->PixelToLogic( pWindow->ScreenToOutputPixel(
                   Pointer::GetPosPixel() ) ) );
        SdrHdl* pHdl=pView->HitHandle(aPnt, *pWindow);

        if (pHdl!=NULL)
        {
            pViewShell->SetActivePointer(pHdl->GetPointer() );
        }
        else
        {
            SdrObject* pObj;
            SdrPageView* pPV;

            if ( pView->IsMarkedHit(aPnt) )
            {
                if ( pView->TakeTextEditObject(aPnt, pObj, pPV) )
                {
                    pViewShell->SetActivePointer(Pointer(POINTER_TEXT));
                }
                else
                {
                    pViewShell->SetActivePointer(Pointer(POINTER_MOVE));
                }
            }
            else
            {
//              if ( pView->TakeMacroObject(aPnt, pObj, pPV) )
                if ( pView->PickObj(aPnt, pObj, pPV, SDRSEARCH_PICKMACRO) )
                {
                    pViewShell->SetActivePointer( pObj->GetMacroPointer() );
                }
                else
                {
                    pViewShell->SetActivePointer( aNewPointer );
                }
            }
        }
    }
*/
}



/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL __EXPORT FuText::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

    if ( pView->KeyInput(rKEvt, pWindow) )
    {
        bReturn = TRUE;
        lcl_InvalidateAttribs( pViewShell->GetViewFrame()->GetBindings() );
    }
    else
    {
        bReturn = FuDraw::KeyInput(rKEvt);
    }

    return (bReturn);
}



/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuText::Activate()
{
    pView->SetDragMode(SDRDRAG_MOVE);
    SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_OBJECT_ROTATE );
    rBindings.Invalidate( SID_OBJECT_MIRROR );

//  Sofort in den Edit Mode setzen
//  SetInEditMode();

//  if (!pTextObj)
    {
        /**********************************************************************
        * Kein Textobjekt im EditMode, daher CreateMode setzen
        **********************************************************************/
        USHORT nObj = OBJ_TEXT;

/*      UINT16 nIdent;
        UINT32 nInvent;
        pView->TakeCurrentObj(nIdent, nInvent);
*/
        pView->SetCurrentObj(nObj);

        pView->SetCreateMode();
    }

    aNewPointer = Pointer(POINTER_TEXT);
//  aNewPointer = Pointer(POINTER_CROSS);               //! ???

    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer( aNewPointer );

    FuConstruct::Activate();
}


/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuText::Deactivate()
{
    FuConstruct::Deactivate();
    pViewShell->SetActivePointer( aOldPointer );
    StopEditMode();
}


/*************************************************************************
|*
|* Selektion hat sich geaendert
|*
\************************************************************************/

void FuText::SelectionHasChanged()
{
    pView->SetDragMode(SDRDRAG_MOVE);
    SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_OBJECT_ROTATE );
    rBindings.Invalidate( SID_OBJECT_MIRROR );

    pTextObj = NULL;

    if ( pView->HasMarkedObj() )
    {
        const SdrMarkList& rMarkList = pView->GetMarkList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetObj();

            UINT16 nSdrObjKind = pObj->GetObjIdentifier();

            if (nSdrObjKind == OBJ_TEXT ||
                nSdrObjKind == OBJ_TITLETEXT ||
                nSdrObjKind == OBJ_OUTLINETEXT /* ||
                pObj->ISA(SdrTextObj) */ )
            {
                pTextObj = (SdrTextObj*) pObj;
            }
        }
    }

    if (!pTextObj)
    {
        /**********************************************************************
        * Kein Textobjekt im EditMode, daher CreateMode setzen
        **********************************************************************/
        USHORT nObj = OBJ_TEXT;
        UINT16 nIdent;
        UINT32 nInvent;
        pView->TakeCurrentObj(nIdent, nInvent);

//        if (! pView->IsEditMode() )
//        {
//            if (nIdent == OBJ_TEXT)
//            {
//                nObj = OBJ_TEXT;
//            }
//            else if (nIdent == OBJ_OUTLINETEXT)
//            {
//                nObj = OBJ_OUTLINETEXT;
//            }
//            else if (nIdent == OBJ_TITLETEXT)
//            {
//                nObj = OBJ_TITLETEXT;
//            }
//        }

        pView->SetCurrentObj(nObj);

        pView->SetCreateMode();
    }
}

/*************************************************************************
|*
|* Objekt in Edit-Mode setzen
|*
\************************************************************************/

void FuText::SetInEditMode(SdrObject* pObj, const Point* pMousePixel)
{
    //  pObj != NULL, wenn ein spezielles (nicht markiertes) Objekt editiert werden soll
    //  (-> Legendenobjekt von Notizen)
    //  wenn pObj == NULL, markiertes Objekt nehmen

    SdrLayer* pLockLayer = NULL;
    if ( pObj && pObj->GetLayer() == SC_LAYER_INTERN )
    {
        //  auf gelocktem Layer kann nicht editiert werden, darum den Layer
        //  temporaer auf nicht gelockt setzen

        pLockLayer = pDrDoc->GetLayerAdmin().GetLayerPerID(SC_LAYER_INTERN);
        if (pLockLayer)
            pView->SetLayerLocked( pLockLayer->GetName(), FALSE );
    }

    if ( !pObj && pView->HasMarkedObj() )
    {
        const SdrMarkList& rMarkList = pView->GetMarkList();
        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            pObj = pMark->GetObj();
        }
    }

    pTextObj = NULL;

    if ( pObj )
    {
        UINT16 nSdrObjKind = pObj->GetObjIdentifier();

        if (nSdrObjKind == OBJ_TEXT ||
            nSdrObjKind == OBJ_TITLETEXT ||
            nSdrObjKind == OBJ_OUTLINETEXT ||
            pObj->ISA(SdrTextObj))
        {
            SdrPageView* pPV = pView->GetPageViewPvNum(0);
            Rectangle aRect = pObj->GetLogicRect();
            Point aPnt = aRect.Center();

            if ( pObj->HasTextEdit() )
            {
                SdrOutliner* pO = MakeOutliner();
                //!??   ohne uebergebenen Outliner stimmen die Defaults nicht ???!?
                if ( pView->BegTextEdit(pObj, pPV, pWindow, (FASTBOOL)TRUE, pO) )
                {
                    //  EditEngine-UndoManager anmelden
                    pViewShell->SetDrawTextUndo( &pO->GetUndoManager() );

                    pTextObj = (SdrTextObj*) pObj;
                    pView->SetEditMode();

                    //  ggf. Text-Cursor an Klickposition setzen
                    if (pMousePixel)
                    {
                        OutlinerView* pOLV = pView->GetTextEditOutlinerView();
                        if (pOLV)
                        {
                            MouseEvent aEditEvt( *pMousePixel, 1, MOUSE_SYNTHETIC, MOUSE_LEFT, 0 );
                            pOLV->MouseButtonDown(aEditEvt);
                            pOLV->MouseButtonUp(aEditEvt);
                        }
                    }
                }
            }
        }
    }

    if (pLockLayer)
        pView->SetLayerLocked( pLockLayer->GetName(), TRUE );
}




