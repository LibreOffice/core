/*************************************************************************
 *
 *  $RCSfile: edtwin.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-25 15:35:11 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <tools/list.hxx>

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_KEYCODES_HXX //autogen
#include <vcl/keycodes.hxx>
#endif
#ifndef _VCL_CMDEVT_HXX //autogen
#include <vcl/cmdevt.hxx>
#endif
#ifndef _VCL_CMDEVT_HXX //autogen
#include <vcl/cmdevt.hxx>
#endif
#ifndef _VCL_CMDEVT_H //autogen
#include <vcl/cmdevt.h>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _ARGS_HXX //autogen
#include <svtools/args.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVDVMARK_HXX //autogen
#include <svx/svdvmark.hxx>
#endif
#ifndef _SVDHDL_HXX //autogen
#include <svx/svdhdl.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _MySVXACORR_HXX //autogen
#include <svx/svxacorr.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_FLDITEM_HXX
#   ifndef ITEMID_FIELD
#       ifndef _EEITEM_HXX //autogen
#           include <svx/eeitem.hxx>
#       endif
#       define ITEMID_FIELD EE_FEATURE_FIELD  // wird fuer #include <flditem.hxx> benoetigt
#   endif
#   ifndef _SVX_FLDITEM_HXX //autogen
#       include <svx/flditem.hxx>
#   endif
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _OFAACCFG_HXX //autogen
#include <offmgr/ofaaccfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _EDTWIN_HXX //autogen
#include <edtwin.hxx>
#endif
#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX //autogen
#include <wrtsh.hxx>
#endif
#ifndef _FLDBAS_HXX //autogen
#include <fldbas.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _VIEWOPT_HXX //autogen
#include <viewopt.hxx>
#endif
#ifndef _SW_DRAWBASE_HXX //autogen
#include <drawbase.hxx>
#endif
#ifndef _SW_DSELECT_HXX //autogen
#include <dselect.hxx>
#endif
#ifndef _SWTEXTSH_HXX //autogen
#include <textsh.hxx>
#endif
#ifndef _SHDWCRSR_HXX
#include <shdwcrsr.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _MODOPT_HXX //autogen
#include <modcfg.hxx>
#endif
#ifndef _FMTCOL_HXX //autogen
#include <fmtcol.hxx>
#endif
#ifndef _SWWVIEW_HXX //autogen
#include <wview.hxx>
#endif
#ifndef _SWLISTSH_HXX //autogen
#include <listsh.hxx>
#endif
#ifndef _GLOSLST_HXX //autogen
#include <gloslst.hxx>
#endif
#ifndef SW_INPUTWIN_HXX //autogen
#include <inputwin.hxx>
#endif
#ifndef _GLOSHDL_HXX //autogen
#include <gloshdl.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _SWDRWTXTSH_HXX //autogen
#include <drwtxtsh.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _ROMENU_HXX
#include <romenu.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _EXTINPUT_HXX
#include <extinput.hxx>
#endif
#ifndef _ACMPLWRD_HXX
#include <acmplwrd.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _DOCVW_HRC
#include <docvw.hrc>
#endif


/*--------------------------------------------------------------------
    Beschreibung:   Globals
 --------------------------------------------------------------------*/

extern BOOL bNoInterrupt;       // in mainwn.cxx

//Normalerweise wird im MouseButtonUp eine Selektion aufgehoben wenn die
//Selektion nicht gerade aufgezogen wird. Leider wird im MouseButtonDown
//bei doppel-/dreifach-Klick Selektiert, diese Selektion wird in dem Handler
//komplett abgeschlossen und kann deshalb im Up nicht mehr unterschieden
//werden. Um dies Aufzuloese wird bHoldSelection im Down gesetzt und im
//Up ausgewertet.
static BOOL bHoldSelection      = FALSE;

BOOL bFrmDrag                   = FALSE;
BOOL bValidCrsrPos              = FALSE;
BOOL bModePushed                = FALSE;
BOOL bDDTimerStarted            = FALSE;
BOOL bFlushCharBuffer           = FALSE;
BOOL SwEditWin::bReplaceQuote   = FALSE;
BOOL bDDINetAttr                = FALSE;
SdrHdlKind eSdrMoveHdl          = HDL_USER;

QuickHelpData* SwEditWin::pQuickHlpData = 0;

long    SwEditWin::nDDStartPosY = 0;
long    SwEditWin::nDDStartPosX = 0;
Color   SwEditWin::aTextBackColor(COL_YELLOW);
Color   SwEditWin::aTextColor(COL_RED);
BOOL    SwEditWin::bTransparentBackColor = FALSE; // Hintergrund nicht transparent


extern BOOL     bExecuteDrag;

SfxShell* lcl_GetShellFromDispatcher( SwView& rView, TypeId nType );

DBG_NAME(edithdl);

class SwAnchorMarker
{
    SdrHdl* pHdl;
    Point aLastPos;
public:
    SwAnchorMarker( SdrHdl* pH ) :
        pHdl( pH ), aLastPos( pH->GetPos() ) {}
    const Point& GetLastPos() const { return aLastPos; }
    void SetLastPos( const Point& rNew ) { aLastPos = rNew; }
    void SetPos( const Point& rNew ) { pHdl->SetPos( rNew ); }
};

struct QuickHelpData
{
    SvStringsISortDtor aArr;
    USHORT* pAttrs;
    CommandExtTextInputData* pCETID;
    ULONG nTipId;
    USHORT nLen, nCurArrPos;
    BOOL bClear : 1, bChkInsBlank : 1, bIsTip : 1, bIsAutoText : 1;

    QuickHelpData() : pCETID( 0 ), pAttrs( 0 ) { ClearCntnt(); }

    void Move( QuickHelpData& rCpy );
    void ClearCntnt();
    void Start( SwWrtShell& rSh, USHORT nWrdLen );
    void Stop( SwWrtShell& rSh );

    BOOL HasCntnt() const       { return aArr.Count() && 0 != nLen; }

    void Inc( BOOL bEndLess )
        {
            if( ++nCurArrPos >= aArr.Count() )
                nCurArrPos = (bEndLess && !bIsAutoText )? 0 : nCurArrPos-1;
        }
    void Dec( BOOL bEndLess )
        {
            if( 0 == nCurArrPos-- )
                nCurArrPos = (bEndLess && !bIsAutoText ) ? aArr.Count()-1 : 0;
        }
};


/*--------------------------------------------------------------------
    Beschreibung:   Minimale Bewegung Zittern vermeiden
 --------------------------------------------------------------------*/

#define HIT_PIX  2 /* Hit-Toleranz in Pixel */
#define MIN_MOVE 4

inline BOOL IsMinMove(const Point &rStartPos, const Point &rLPt)
{
    return Abs(rStartPos.X() - rLPt.X()) > MIN_MOVE ||
           Abs(rStartPos.Y() - rLPt.Y()) > MIN_MOVE;
}

/*--------------------------------------------------------------------
    JP 30.07.98: fuer MouseButtonDown - feststellen, ob ein DrawObject
                und KEIN SwgFrame getroffen wurde! Shift/Ctrl sollen
                nur bei DrawObjecte zum Selektieren fuehren, bei SwgFlys
                ggfs zum ausloesen von Hyperlinks (DownLoad/NewWindow!)
 --------------------------------------------------------------------*/
inline BOOL IsDrawObjSelectable( const SwWrtShell& rSh, const Point& rPt )
{
    BOOL bRet = TRUE;
    SdrObject* pObj;
    switch( rSh.GetObjCntType( rPt, pObj ))
    {
    case OBJCNT_NONE:
    case OBJCNT_FLY:
    case OBJCNT_GRF:
    case OBJCNT_OLE:
        bRet = FALSE;
        break;
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Pointer umschalten
 --------------------------------------------------------------------*/


void SwEditWin::UpdatePointer(const Point &rLPt, USHORT nModifier )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    if( pApplyTempl )
    {
        PointerStyle eStyle = POINTER_FILL;
        if( rSh.IsOverReadOnlyPos( rLPt ))
        {
            if( pUserMarker )
                DELETEZ( pUserMarker );
//          rSh.SwCrsrShell::UnSetVisCrsr( rLPt );
            eStyle = POINTER_NOTALLOWED;
        }
        else
        {
            SwRect aRect;
            SwRect* pRect = &aRect;
            const SwFrmFmt* pFmt = 0;
            if(!pApplyTempl->nColor &&
                    pApplyTempl->eType == SFX_STYLE_FAMILY_FRAME &&
                        0 !=(pFmt = rSh.GetFmtFromObj( rLPt, &pRect )) &&
                        PTR_CAST(SwFlyFrmFmt, pFmt))
            {
                //Highlight fuer Rahmen anwerfen
                Rectangle aTmp( pRect->SVRect() );
                if ( !pUserMarker )
                    pUserMarker = new SdrViewUserMarker( rSh.GetDrawView() );
                pUserMarker->SetRectangle( aTmp );
                pUserMarker->Show();
            }
            else if(pUserMarker)
                DELETEZ(pUserMarker);
            rSh.SwCrsrShell::SetVisCrsr( rLPt );
        }
        SetPointer( eStyle );
        return;
    }

    if( !rSh.VisArea().Width() )
        return;

    SET_CURR_SHELL(&rSh);

    if ( IsChainMode() )
    {
        SwRect aRect;
        int nChainable = rSh.Chainable( aRect, *rSh.GetFlyFrmFmt(), rLPt );
        PointerStyle eStyle = nChainable
                ? POINTER_CHAIN_NOTALLOWED : POINTER_CHAIN;
        if ( !nChainable )
        {
            Rectangle aTmp( aRect.SVRect() );
            if ( !pUserMarker )
                pUserMarker = new SdrViewUserMarker( rSh.GetDrawView() );
            pUserMarker->SetRectangle( aTmp );
            pUserMarker->Show();
        }
        else
        {
            DELETEZ( pUserMarker );
        }
        rView.GetViewFrame()->ShowStatusText(
                                        SW_RESSTR(STR_CHAIN_OK+nChainable));
        SetPointer( eStyle );
        return;
    }

    BOOL bExecHyperlinks = rSh.GetViewOptions()->IsExecHyperlinks() ^
                           (nModifier == KEY_MOD2 ? TRUE : FALSE);

    SdrView *pSdrView = rSh.GetDrawView();
    BOOL bPrefSdrPointer = FALSE;
    BOOL bHitHandle = FALSE;
    BOOL bCntAtPos = FALSE;
    BOOL bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                          rSh.IsCrsrReadonly();
    aActHitType = SDRHIT_NONE;
    PointerStyle eStyle = POINTER_TEXT;
    if ( !pSdrView )
        bCntAtPos = TRUE;
    else if ( bHitHandle = pSdrView->HitHandle( rLPt, *(rSh.GetOut())) != 0 )
    {
        aActHitType = SDRHIT_OBJECT;
        bPrefSdrPointer = TRUE;
    }
    else
    {
        const BOOL bNotInSelObj = !rSh.IsInsideSelectedObj( rLPt );
        if ( rView.GetDrawFuncPtr() && !bInsDraw && bNotInSelObj )
        {
            aActHitType = SDRHIT_OBJECT;
            if (GetDrawMode() == SID_OBJECT_SELECT)
                eStyle = POINTER_ARROW;
            else
                bPrefSdrPointer = TRUE;
        }
        else
        {
            SdrObject* pObj; SdrPageView* pPV;
            pSdrView->SetHitTolerancePixel( HIT_PIX );
            if ( bNotInSelObj && bExecHyperlinks &&
                 pSdrView->PickObj( rLPt, pObj, pPV, SDRSEARCH_PICKMACRO ))
            {
                SdrObjMacroHitRec aTmp;
                aTmp.aPos = rLPt;
                aTmp.pPageView = pPV;
                SetPointer( pObj->GetMacroPointer( aTmp ) );
                return;
            }
            else
            {
                if ( rSh.IsObjSelectable( rLPt ) )
                {
                    if (pSdrView->IsTextEdit())
                    {
                        aActHitType = SDRHIT_NONE;
                        bPrefSdrPointer = TRUE;
                    }
                    else
                    {
                        SdrViewEvent aVEvt;
                        SdrHitKind eHit = pSdrView->PickAnything(rLPt, aVEvt);

                        if (eHit == SDRHIT_URLFIELD && bExecHyperlinks)
                        {
                            aActHitType = SDRHIT_OBJECT;
                            bPrefSdrPointer = TRUE;
                        }
                        else
                        {
                            eStyle = POINTER_MOVE;

                            if (!bNotInSelObj)
                            {
                                if (rSh.IsObjSelected() || rSh.IsFrmSelected())
                                {
                                    if (rSh.IsSelObjProtected(FLYPROTECT_POS))
                                        eStyle = POINTER_NOTALLOWED;
                                }
                            }
                            aActHitType = SDRHIT_OBJECT;
                        }
                    }
                }
                else
                {
                    if ( rSh.IsFrmSelected() && !bNotInSelObj )
                    {
                        if (rSh.IsSelObjProtected(FLYPROTECT_SIZE))
                            eStyle = POINTER_NOTALLOWED;
                        else
                            eStyle = POINTER_MOVE;
                        aActHitType = SDRHIT_OBJECT;
                    }
                    else
                    {
                        if ( rView.GetDrawFuncPtr() )
                            bPrefSdrPointer = TRUE;
                        else
                            bCntAtPos = TRUE;
                    }
                }
            }
        }
    }
    if ( bPrefSdrPointer )
    {
        if (bIsDocReadOnly || (rSh.IsObjSelected() && rSh.IsSelObjProtected(FLYPROTECT_CONTENT)))
            SetPointer( POINTER_NOTALLOWED );
        else
        {
            if (rView.GetDrawFuncPtr() && rView.GetDrawFuncPtr()->IsInsertForm() && !bHitHandle)
                SetPointer( POINTER_DRAW_RECT );
            else
                SetPointer( pSdrView->GetPreferedPointer( rLPt, rSh.GetOut() ) );
        }
    }
    else
    {
        if( pAnchorMarker )
            eStyle = POINTER_ARROW;
        else if( bCntAtPos && bExecHyperlinks )
        {
            // sollten wir ueber einem InternetAttr/ClickFeld/Fussnote stehen?
            SwContentAtPos aSwContentAtPos(
                SwContentAtPos::SW_CLICKFIELD|
                SwContentAtPos::SW_INETATTR|
                SwContentAtPos::SW_FTN );
            if( rSh.GetContentAtPos( rLPt, aSwContentAtPos) )
                eStyle = POINTER_REFHAND;
        }

        SetPointer( eStyle );
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Timer fuer Selektion vergroessern
 --------------------------------------------------------------------*/


IMPL_LINK( SwEditWin, TimerHandler, Timer *, EMPTYARG )
{
    DBG_PROFSTART(edithdl);

    SwWrtShell &rSh = rView.GetWrtShell();
    Point aModPt( aMovePos );
    const SwRect aOldVis( rSh.VisArea() );
    FASTBOOL bDone = FALSE;

    if ( !rSh.VisArea().IsInside( aModPt ) )
    {
        if ( bInsDraw )
        {
            const int nMaxScroll = 40;
            rView.Scroll( Rectangle(aModPt,Size(1,1)), nMaxScroll, nMaxScroll);
            bDone = TRUE;
        }
        else if ( bFrmDrag )
        {
            (rSh.*rSh.fnDrag)(&aModPt,FALSE);
            bDone = TRUE;
        }
        if ( !bDone )
            aModPt = rSh.GetCntntPos( aModPt,aModPt.Y() > rSh.VisArea().Bottom() );
    }
    if ( !bDone && !(bFrmDrag || bInsDraw) )
    {
        (rSh.*rSh.fnSetCrsr)( &aModPt, FALSE );

        //fix(24138): Es kann sein, dass der "Sprung" ueber eine Tabelle so
        //nicht geschafft wird. Deshalb wir hier eben per Up/Down ueber die
        //Tabelle gesprungen.
        if ( aOldVis == rSh.VisArea() &&
            !rSh.IsStartOfDoc() && !rSh.IsEndOfDoc() && !rSh.IsTableMode())
        {
            if ( aModPt.Y() < rSh.VisArea().Top() )
                rSh.Up( TRUE );
            else
                rSh.Down( TRUE );
        }
    }

    aMovePos += rSh.VisArea().Pos() - aOldVis.Pos();
    JustifyAreaTimer();
    DBG_PROFSTOP(edithdl);
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwEditWin::JustifyAreaTimer()
{
    const Rectangle &rVisArea = GetView().GetVisArea();
    long nTimeout = 800,
         nDiff = Max(
         Max( aMovePos.Y() - rVisArea.Bottom(), rVisArea.Top() - aMovePos.Y() ),
         Max( aMovePos.X() - rVisArea.Right(),  rVisArea.Left() - aMovePos.X()));
    aTimer.SetTimeout( Max( 0L, nTimeout - nDiff*2L) );
}

void SwEditWin::LeaveArea(const Point &rPos)
{
    aMovePos = rPos;
    JustifyAreaTimer();
    if ( !aTimer.IsActive() )
        aTimer.Start();
    if( pShadCrsr )
        delete pShadCrsr, pShadCrsr = 0;
}

inline void SwEditWin::EnterArea()
{
    aTimer.Stop();
}

/*------------------------------------------------------------------------
 Beschreibung:  Modus fuer Rahmen einfuegen
------------------------------------------------------------------------*/


void SwEditWin::InsFrm(USHORT nCols)
{
    StdDrawMode(OBJ_NONE);
    bInsFrm = TRUE;
    nInsFrmColCount = nCols;
}



void SwEditWin::StdDrawMode(USHORT nSlotId)
{
    SetDrawMode( nSlotId );

/*  if (GetDrawFuncPtr())
        GetDrawFuncPtr()->Deactivate();*/

    if (nSlotId == OBJ_NONE)
        rView.SetDrawFuncPtr(new SwDrawBase( &rView.GetWrtShell(), this, &rView ));
    else
        rView.SetDrawFuncPtr(new DrawSelection( &rView.GetWrtShell(), this, &rView ));

    rView.SetSelDrawSlot();
    SetDrawMode(nSlotId);
    rView.GetDrawFuncPtr()->Activate( nSlotId );
    bInsFrm = FALSE;
    nInsFrmColCount = 1;
}



void SwEditWin::StopInsFrm()
{
    if (rView.GetDrawFuncPtr())
    {
        rView.GetDrawFuncPtr()->Deactivate();
        rView.SetDrawFuncPtr(NULL);
    }
    rView.LeaveDrawCreate();    // Konstruktionsmode verlassen
    bInsFrm = FALSE;
    nInsFrmColCount = 1;
}

/*--------------------------------------------------------------------
     Beschreibung:  Der Character Buffer wird in das Dokument eingefuegt
 --------------------------------------------------------------------*/


void SwEditWin::FlushInBuffer( SwWrtShell *pSh )
{
    if ( aInBuffer.Len() )
    {
        pSh->Insert( aInBuffer );
        aInBuffer.Erase();
        bFlushCharBuffer = FALSE;
    }
}



void SwEditWin::ChangeFly( BYTE nDir, BOOL bWeb )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SwRect aTmp = rSh.GetFlyRect();
    if( aTmp.HasArea() )
    {
        SfxItemSet aSet(rSh.GetAttrPool(),
                        RES_FRM_SIZE, RES_FRM_SIZE,
                        RES_VERT_ORIENT, RES_ANCHOR,
                        RES_COL, RES_COL, 0);
        rSh.GetFlyFrmAttr( aSet );
        RndStdIds eAnchorId = ((SwFmtAnchor&)aSet.Get(RES_ANCHOR)).GetAnchorId();
        Size aSnap( rSh.GetViewOptions()->GetSnapSize() );
        short nDiv = rSh.GetViewOptions()->GetDivisionX();
        if ( nDiv > 0 )
            aSnap.Width() = Max( (ULONG)1, (ULONG)aSnap.Width() / nDiv );
        nDiv = rSh.GetViewOptions()->GetDivisionY();
        if ( nDiv > 0 )
            aSnap.Height() = Max( (ULONG)1, (ULONG)aSnap.Height() / nDiv );
        SwRect aBoundRect;
        Point aRefPoint;
        rSh.CalcBoundRect( aBoundRect, eAnchorId, FRAME, FALSE, &aRefPoint );
        long nLeft = Min( aTmp.Left() - aBoundRect.Left(), aSnap.Width() );
        long nRight = Min( aBoundRect.Right() - aTmp.Right(), aSnap.Width() );
        long nUp = Min( aTmp.Top() - aBoundRect.Top(), aSnap.Height() );
        long nDown = Min( aBoundRect.Bottom() - aTmp.Bottom(), aSnap.Height() );

        switch ( nDir )
        {
            case 4:
            case 0: aTmp.Left( aTmp.Left() - nLeft ); break;
            case 5:
            case 1: aTmp.Top( aTmp.Top() - nUp ); break;
            case 6: if( aTmp.Width() < aSnap.Width() + MINFLY ) break;
                    nRight = aSnap.Width(); // kein break
            case 2: aTmp.Left( aTmp.Left() + nRight ); break;
            case 7: if( aTmp.Height() < aSnap.Height() + MINFLY ) break;
                    nDown = aSnap.Height(); // kein break
            case 3: aTmp.Top( aTmp.Top() + nDown ); break;
            case 8: if( aTmp.Width() >= aSnap.Width() + MINFLY )
                        aTmp.Right( aTmp.Right() - aSnap.Width() );
                    break;
            case 9: if( aTmp.Height() >= aSnap.Height() + MINFLY )
                        aTmp.Height( aTmp.Height() - aSnap.Height() );
                    break;
            case 10: aTmp.Width( aTmp.Width() + nRight ); break;
            case 11: aTmp.Height( aTmp.Height() + nDown ); break;
            default: ASSERT( TRUE, "ChangeFly: Unknown direction." );
        }
        BOOL bSet = FALSE;
        if( nDir < 8 && FLY_IN_CNTNT == eAnchorId && ( nDir % 2 ) )
        {
            long aDiff = aTmp.Top() - aRefPoint.Y();
            if( aDiff > 0 )
                aDiff = 0;
            else if ( aDiff < -aTmp.Height() )
                aDiff = -aTmp.Height();
            SwFmtVertOrient aVert( (SwFmtVertOrient&)aSet.Get(RES_VERT_ORIENT) );
            SwVertOrient eNew;
            if( bWeb )
            {
                eNew = aVert.GetVertOrient();
                BOOL bDown = 0 != ( nDir & 0x02 );
                switch( eNew )
                {
                    case VERT_CHAR_TOP:
                        if( bDown ) eNew = VERT_CENTER; break;
                    case VERT_CENTER:
                        eNew = bDown ? VERT_TOP : VERT_CHAR_TOP; break;
                    case VERT_TOP:
                        if( !bDown ) eNew = VERT_CENTER; break;
                    case VERT_LINE_TOP:
                        if( bDown ) eNew = VERT_LINE_CENTER; break;
                    case VERT_LINE_CENTER:
                        eNew = bDown ? VERT_LINE_BOTTOM : VERT_LINE_TOP; break;
                    case VERT_LINE_BOTTOM:
                        if( !bDown ) eNew = VERT_LINE_CENTER; break;
                }
            }
            else
            {
                aVert.SetPos( aDiff );
                eNew = VERT_NONE;
            }
            aVert.SetVertOrient( eNew );
            aSet.Put( aVert );
            bSet = TRUE;
        }
        if( bWeb && FLY_AT_CNTNT == eAnchorId && ( nDir==0 || nDir==2 ) )
        {
            SwFmtHoriOrient aHori( (SwFmtHoriOrient&)aSet.Get(RES_HORI_ORIENT) );
            SwHoriOrient eNew;
            eNew = aHori.GetHoriOrient();
            switch( eNew )
            {
                case HORI_RIGHT:
                    if( !nDir ) eNew = HORI_LEFT; break;
                case HORI_LEFT:
                    if( nDir ) eNew = HORI_RIGHT; break;
            }
            if( eNew != aHori.GetHoriOrient() )
            {
                aHori.SetHoriOrient( eNew );
                aSet.Put( aHori );
                bSet = TRUE;
            }
        }
        if( nDir > 3 )
        {
            SwFmtFrmSize aSize( (SwFmtFrmSize&)aSet.Get(RES_FRM_SIZE) );

            long nMinWidth = MINFLY;
            // alle Spalten muessen passen
            if (SFX_ITEM_DEFAULT <= aSet.GetItemState(RES_COL))
            {
                const SwFmtCol& rCol = (const SwFmtCol&)aSet.Get(RES_COL);
                if ( rCol.GetColumns().Count() > 1 )
                {
                    for ( USHORT i = 0; i < rCol.GetColumns().Count(); ++i )
                    {
                        nMinWidth += rCol.GetColumns()[i]->GetLeft() +
                                          rCol.GetColumns()[i]->GetRight() +
                                          MINFLY;
                    }
                    nMinWidth -= MINFLY;//einen hatten wir schon mit drin!
                }
            }

            aSize.SetSize(Size(Max(aTmp.Width(), nMinWidth), Max(aTmp.Height(), long(MINFLY))));
            aSet.Put( aSize );
            bSet = TRUE;
        }
        rSh.StartAllAction();
        if( bSet )
            rSh.SetFlyFrmAttr( aSet );
        if( !bWeb && nDir < 8 && FLY_IN_CNTNT != eAnchorId )
            rSh.SetFlyPos( aTmp.Pos() );
        rSh.EndAllAction();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   KeyEvents
 --------------------------------------------------------------------*/


void SwEditWin::KeyInput(const KeyEvent &rKEvt)
{
    SfxObjectShell *pObjSh = (SfxObjectShell*)rView.GetViewFrame()->GetObjectShell();
    if ( bLockInput || pObjSh && pObjSh->GetProgress() )
        // Wenn die Rechenleiste aktiv ist oder
        // auf dem Document ein Progress laeuft wird keine
        // Bestellungen angenommen.
        return;

    if( pShadCrsr )
        delete pShadCrsr, pShadCrsr = 0;
    aKeyInputFlushTimer.Stop();

    SwWrtShell &rSh = rView.GetWrtShell();
    BOOL bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                          rSh.IsCrsrReadonly();

    QuickHelpData aTmpQHD;
    if( pQuickHlpData->bClear )
    {
        aTmpQHD.Move( *pQuickHlpData );
        pQuickHlpData->Stop( rSh );
    }

    // OS:auch die DrawView braucht noch ein readonly-Flag
    if ( !bIsDocReadOnly && rSh.GetDrawView() && rSh.GetDrawView()->KeyInput( rKEvt, this ) )
    {
        rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll( FALSE );
        rSh.SetModified();
        return; // Event von der SdrView ausgewertet
    }

    if ( rView.GetDrawFuncPtr() && bInsFrm )
    {
        StopInsFrm();
        rSh.Edit();
    }

    BOOL bFlushBuffer = FALSE;
    BOOL bNormalChar = FALSE;
    BOOL bChkInsBlank = pQuickHlpData->bChkInsBlank;
    pQuickHlpData->bChkInsBlank = FALSE;

    const KeyCode&  rKeyCode = rKEvt.GetKeyCode();
    sal_Unicode aCh = rKEvt.GetCharCode();

    const SwFrmFmt* pFlyFmt = rSh.GetFlyFrmFmt();
    if( pFlyFmt )
    {
        USHORT nEvent;

        if( 32 <= aCh &&
            0 == (( KEY_MOD1 | KEY_MOD2 ) & rKeyCode.GetModifier() ))
            nEvent = SW_EVENT_FRM_KEYINPUT_ALPHA;
        else
            nEvent = SW_EVENT_FRM_KEYINPUT_NOALPHA;

        const SvxMacro* pMacro = pFlyFmt->GetMacro().GetMacroTable().Get( nEvent );
        if( pMacro )
        {
            String sRet;
            SbxArrayRef xArgs = new SbxArray;
            SbxVariableRef xVar = new SbxVariable;
            xVar->PutString( pFlyFmt->GetName() );
            xArgs->Put( &xVar, 1 );

            xVar = new SbxVariable;
            if( SW_EVENT_FRM_KEYINPUT_ALPHA == nEvent )
                xVar->PutChar( aCh );
            else
                xVar->PutUShort( rKeyCode.GetModifier() | rKeyCode.GetCode() );
            xArgs->Put( &xVar, 2 );

            rSh.ExecMacro( *pMacro, &sRet, &xArgs );
            if( sRet.Len() && 0 != sRet.ToInt32() )
                return ;
        }
    }

    OfaAutoCorrCfg* pACfg = OFF_APP()->GetAutoCorrConfig();
    SvxAutoCorrect* pACorr = pACfg->GetAutoCorrect();

    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    /*TblChgWidthHeightType*/int eTblChgMode;
    USHORT nTblChgSize = 0;
    BOOL bStopKeyInputTimer = TRUE;
    String sFmlEntry;

    enum SW_KeyState { KS_Start,
                    KS_CheckKey, KS_InsChar, KS_InsTab,
                    KS_NoNum, KS_NumOff, KS_NumOrNoNum, KS_NumDown, KS_NumUp,
                    KS_NextCell, KS_PrevCell, KS_OutlineUp, KS_OutlineDown,
                    KS_GlossaryExpand, KS_NextPrevGlossary,
                    KS_AutoFmtByInput, KS_DontExpand,
                    KS_KeyToView,
                    KS_CheckAutoCorrect, KS_EditFormula,
                    KS_ColLeftBig, KS_ColRightBig,
                    KS_ColLeftSmall, KS_ColRightSmall,
                    KS_ColTopBig, KS_ColBottomBig,
                    KS_ColTopSmall, KS_ColBottomSmall,
                    KS_CellLeftBig, KS_CellRightBig,
                    KS_CellLeftSmall, KS_CellRightSmall,
                    KS_CellTopBig, KS_CellBottomBig,
                    KS_CellTopSmall, KS_CellBottomSmall,
//-----
                    KS_InsDel_ColLeftBig, KS_InsDel_ColRightBig,
                    KS_InsDel_ColLeftSmall, KS_InsDel_ColRightSmall,
                    KS_InsDel_ColTopBig, KS_InsDel_ColBottomBig,
                    KS_InsDel_ColTopSmall, KS_InsDel_ColBottomSmall,
                    KS_InsDel_CellLeftBig, KS_InsDel_CellRightBig,
                    KS_InsDel_CellLeftSmall, KS_InsDel_CellRightSmall,
                    KS_InsDel_CellTopBig, KS_InsDel_CellBottomBig,
                    KS_InsDel_CellTopSmall, KS_InsDel_CellBottomSmall,
                    KS_TblColCellInsDel,
//-----
                    KS_Fly_Change,
                    KS_AppendNodeInSection,
                    KS_Ende };

    SW_KeyState eKeyState = bIsDocReadOnly ? KS_KeyToView : KS_CheckKey,
                eNextKeyState = KS_Ende;

    while( KS_Ende != eKeyState )
    {
        SW_KeyState eFlyState = KS_KeyToView;
        BYTE nDir;

        switch( eKeyState )
        {
        case KS_CheckKey:
            eKeyState = KS_KeyToView;       // default weiter zur View

#ifndef PRODUCT
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // JP 19.01.99: zum Umschalten des Cursor Verhaltens in ReadOnly
            //              Bereichen
            if( 0x7210 == rKeyCode.GetFullCode() )
                rSh.SetReadOnlyAvailable( !rSh.IsReadOnlyAvailable() );
            else
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#endif

            if( !rKeyCode.IsControlMod() && '=' == aCh &&
                !rSh.IsTableMode() && rSh.GetTableFmt() &&
                rSh.IsSttPara() /*&& rSh.IsEndPara()*/ &&
                !rSh.HasReadonlySel() )
            {
                // in der Tabelle am Anfang der Zelle ein '=' ->
                //  EditZeile aufrufen (F2-Funktionalitaet)
                rSh.Push();
                if( !rSh.MoveSection( fnSectionCurr, fnSectionStart) &&
                    !rSh.IsTableBoxTextFormat() )
                {
                    // steht also am Anfang der Box
                    eKeyState = KS_EditFormula;
                    if( rSh.HasMark() )
                        rSh.SwapPam();
                    else
                        rSh.SttSelect();
                    rSh.MoveSection( fnSectionCurr, fnSectionEnd );
                    rSh.Pop( TRUE );
                    rSh.EndSelect();
                    sFmlEntry = '=';
                }
                else
                    rSh.Pop( FALSE );
            }
            else
            {
                if( pACorr && aTmpQHD.HasCntnt() && !rSh.HasSelection() &&
                    !rSh.HasReadonlySel() && !aTmpQHD.bIsAutoText &&
                    pACorr->GetSwFlags().nAutoCmpltExpandKey ==
                    (rKeyCode.GetModifier() | rKeyCode.GetCode()) )
                {
                    eKeyState = KS_GlossaryExpand;
                    break;
                }

                switch( rKeyCode.GetModifier() | rKeyCode.GetCode() )
                {
                case KEY_RIGHT | KEY_MOD2:
                    eKeyState = KS_ColRightBig;
                    eFlyState = KS_Fly_Change;
                    nDir = 2;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_LEFT | KEY_MOD2:
                    eKeyState = KS_ColRightSmall;
                    eFlyState = KS_Fly_Change;
                    nDir = 0;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_RIGHT | KEY_MOD2 | KEY_SHIFT:
                    eKeyState = KS_ColLeftSmall;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_LEFT | KEY_MOD2 | KEY_SHIFT:
                    eKeyState = KS_ColLeftBig;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_RIGHT | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellRightBig;
                    eFlyState = KS_Fly_Change;
                    nDir = 10;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_LEFT | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellRightSmall;
                    eFlyState = KS_Fly_Change;
                    nDir = 8;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_RIGHT | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellLeftSmall;
                    eFlyState = KS_Fly_Change;
                    nDir = 6;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_LEFT | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellLeftBig;
                    eFlyState = KS_Fly_Change;
                    nDir = 4;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_UP | KEY_MOD2:
                    eKeyState = KS_ColBottomSmall;
                    eFlyState = KS_Fly_Change;
                    nDir = 1;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_DOWN | KEY_MOD2:
                    eKeyState = KS_ColBottomBig;
                    eFlyState = KS_Fly_Change;
                    nDir = 3;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_UP | KEY_MOD2 | KEY_SHIFT:
                    eKeyState = KS_ColTopBig;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_DOWN | KEY_MOD2 | KEY_SHIFT:
                    eKeyState = KS_ColTopSmall;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_UP | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellBottomSmall;
                    eFlyState = KS_Fly_Change;
                    nDir = 9;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_DOWN | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellBottomBig;
                    eFlyState = KS_Fly_Change;
                    nDir = 11;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_UP | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellTopBig;
                    eFlyState = KS_Fly_Change;
                    nDir = 5;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_DOWN | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellTopSmall;
                    eFlyState = KS_Fly_Change;
                    nDir = 7;
                    goto KEYINPUT_CHECKTABLE;

KEYINPUT_CHECKTABLE:
                    if( rSh.IsTableMode() || !rSh.GetTableFmt() )
                    {

                        if( pFlyFmt && KS_KeyToView != eFlyState )
                        {
                            const SfxPoolItem* pItem;
                            if( SFX_ITEM_SET == pFlyFmt->GetItemState(
                                RES_PROTECT, TRUE, &pItem ) && (
                                ((SvxProtectItem*)pItem)->IsSizeProtected() ||
                                ((SvxProtectItem*)pItem)->IsPosProtected() ))
                            {
                                //JP 18.01.99: man koennte es noch feiner
                                // ermitteln (nDir auswerten)
                                pFlyFmt =  0;
                            }
                        }

                        if( pFlyFmt )
                            eKeyState = eFlyState;
                        else
                            eKeyState = KS_KeyToView;
                    }
                    break;

//-------
// Insert/Delete
                case KEY_LEFT:
                case KEY_LEFT | KEY_MOD1:
                    eTblChgMode = WH_FLAG_INSDEL |
                            ( rKeyCode.GetModifier() & KEY_MOD1
                                ? WH_CELL_LEFT
                                : WH_COL_LEFT );
                    nTblChgSize = pModOpt->GetTblVInsert();
                    goto KEYINPUT_CHECKTABLE_INSDEL;
//              case KEY_RIGHT:
                case KEY_RIGHT | KEY_MOD1:
                    eTblChgMode = WH_FLAG_INSDEL | WH_CELL_RIGHT;
                    nTblChgSize = pModOpt->GetTblVInsert();
                    goto KEYINPUT_CHECKTABLE_INSDEL;

                case KEY_UP:
                case KEY_UP | KEY_MOD1:
                    eTblChgMode = WH_FLAG_INSDEL |
                            ( rKeyCode.GetModifier() & KEY_MOD1
                                ? WH_CELL_TOP
                                : WH_ROW_TOP );
                    nTblChgSize = pModOpt->GetTblHInsert();
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_DOWN:
                case KEY_DOWN | KEY_MOD1:
                    eTblChgMode = WH_FLAG_INSDEL |
                            ( rKeyCode.GetModifier() & KEY_MOD1
                                ? WH_CELL_BOTTOM
                                : WH_ROW_BOTTOM );
                    nTblChgSize = pModOpt->GetTblHInsert();
                    goto KEYINPUT_CHECKTABLE_INSDEL;

KEYINPUT_CHECKTABLE_INSDEL:
                    if( rSh.IsTableMode() || !rSh.GetTableFmt() ||
                        !bTblInsDelMode ||
                        FALSE /* Tabelle geschuetzt */
                            )
                        eKeyState = KS_KeyToView;
                    else
                    {
                        if( !bTblIsInsMode )
                            eTblChgMode = eTblChgMode | WH_FLAG_BIGGER;
                        eKeyState = KS_TblColCellInsDel;
                    }
                    break;

                case KEY_DELETE | KEY_MOD2:
                    if( !rSh.IsTableMode() && rSh.GetTableFmt() )
                    {
                        eKeyState = KS_Ende;
                        bTblInsDelMode = TRUE;
                        bTblIsInsMode = FALSE;
                        bTblIsColMode = TRUE;
                        aKeyInputTimer.Start();
                        bStopKeyInputTimer = FALSE;
                    }
                    break;
                case KEY_INSERT | KEY_MOD2:
                    if( !rSh.IsTableMode() && rSh.GetTableFmt() )
                    {
                        eKeyState = KS_Ende;
                        bTblInsDelMode = TRUE;
                        bTblIsInsMode = TRUE;
                        bTblIsColMode = TRUE;
                        aKeyInputTimer.Start();
                        bStopKeyInputTimer = FALSE;
                    }
                    break;

                case KEY_RETURN:                // Return
                    if( !rSh.HasReadonlySel() )
                    {
                        if( aTmpQHD.HasCntnt() && !rSh.HasSelection() &&
                            aTmpQHD.bIsAutoText )
                            eKeyState = KS_GlossaryExpand;

                        //RETURN und leerer Absatz in Numerierung -> Num. beenden
                        else if( rSh.GetCurNumRule() && !rSh.HasSelection() &&
                                rSh.IsSttPara() && rSh.IsEndPara() )
                            eKeyState = KS_NumOff;

                        //RETURN fuer neuen Absatz mit AutoFormatierung
                        else if( pACfg && pACfg->IsAutoFmtByInput() &&
                                !(rSh.GetSelectionType() & (SwWrtShell::SEL_GRF |
                                    SwWrtShell::SEL_OLE | SwWrtShell::SEL_FRM |
                                    SwWrtShell::SEL_TBL_CELLS | SwWrtShell::SEL_DRW |
                                    SwWrtShell::SEL_DRW_TXT)) )
                            eKeyState = KS_CheckAutoCorrect, eNextKeyState = KS_AutoFmtByInput;
                        else
                            eNextKeyState = eKeyState, eKeyState = KS_CheckAutoCorrect;
                    }
                    break;

                case KEY_RETURN | KEY_MOD2:     // ALT-Return
                    if( !rSh.HasReadonlySel() && rSh.GetCurNumRule() )
                        eKeyState = KS_NoNum;
                    else if( rSh.CanInsertNodeAtEndOfSection() )
                        eKeyState = KS_AppendNodeInSection;
                    break;

                case KEY_BACKSPACE:
                case KEY_BACKSPACE | KEY_SHIFT:
                    if( !rSh.HasReadonlySel() )
                    {
                        BOOL bOutline = FALSE;
#ifdef TASK_59308
                        const SwTxtFmtColl* pColl;
                        if( !rSh.SwCrsrShell::HasSelection() &&
                            0 != ( pColl = rSh.GetCurTxtFmtColl() ) &&
                            NO_NUMBERING != pColl->GetOutlineLevel() &&
                            NO_NUMBERING == rSh.GetNumLevel( FALSE ) )
                            bOutline = TRUE;
#endif

                        if( rSh.NumOrNoNum(
                                    KEY_BACKSPACE != rKeyCode.GetFullCode(),
                                    TRUE, bOutline ))
                            eKeyState = KS_NumOrNoNum;
                    }
                    break;

                case KEY_END:
                    if( !rSh.HasReadonlySel() && rSh.IsEndPara() &&
                        rSh.DontExpandFmt() )
                        eKeyState = KS_DontExpand;
                    break;

                case KEY_RIGHT:
                    if( !rSh.HasReadonlySel() )
                    {
                        if( rSh.IsEndPara() && rSh.DontExpandFmt() )
                            eKeyState = KS_DontExpand;
                        else
                        {
                            eTblChgMode = WH_FLAG_INSDEL | WH_COL_RIGHT;
                            nTblChgSize = pModOpt->GetTblVInsert();
                            goto KEYINPUT_CHECKTABLE_INSDEL;
                        }
                    }
                    break;

                case KEY_TAB:
                {
#ifdef SW_CRSR_TIMER
                    BOOL bOld = rSh.ChgCrsrTimerFlag( FALSE );
#endif
                    if( rSh.GetCurNumRule() && rSh.IsSttOfPara() &&
                        !rSh.HasReadonlySel() )
                        eKeyState = KS_NumDown;
                    else if ( rSh.GetTableFmt() )
                    {
                        if( rSh.HasSelection() || rSh.HasReadonlySel() )
                            eKeyState = KS_NextCell;
                        else
                            eKeyState = KS_CheckAutoCorrect, eNextKeyState = KS_NextCell;
                    }
                    else
                    {
                        eKeyState = KS_InsTab;
                        if( rSh.IsSttOfPara() && !rSh.HasReadonlySel() )
                        {
                            SwTxtFmtColl* pColl = rSh.GetCurTxtFmtColl();
                            if( pColl && 0 <= pColl->GetOutlineLevel() &&
                                MAXLEVEL - 1 > pColl->GetOutlineLevel() )
                                eKeyState = KS_OutlineDown;
                        }
                    }
#ifdef SW_CRSR_TIMER
                    rSh.ChgCrsrTimerFlag( bOld );
#endif
                }
                break;
                case KEY_TAB | KEY_SHIFT:
                {
#ifdef SW_CRSR_TIMER
                    BOOL bOld = rSh.ChgCrsrTimerFlag( FALSE );
#endif
                    if( rSh.GetCurNumRule() && rSh.IsSttOfPara() &&
                        !rSh.HasReadonlySel() )
                        eKeyState = KS_NumUp;
                    else if ( rSh.GetTableFmt() )
                    {
                        if( rSh.HasSelection() || rSh.HasReadonlySel() )
                            eKeyState = KS_PrevCell;
                        else
                            eKeyState = KS_CheckAutoCorrect, eNextKeyState = KS_PrevCell;
                    }
                    else
                    {
                        eKeyState = KS_Ende;
                        if( rSh.IsSttOfPara() && !rSh.HasReadonlySel() )
                        {
                            SwTxtFmtColl* pColl = rSh.GetCurTxtFmtColl();
                            if( pColl && 0 < pColl->GetOutlineLevel() &&
                                MAXLEVEL - 1 >= pColl->GetOutlineLevel() )
                                eKeyState = KS_OutlineUp;
                        }
                    }
#ifdef SW_CRSR_TIMER
                    rSh.ChgCrsrTimerFlag( bOld );
#endif
                }
                break;
                case KEY_TAB | KEY_MOD1:
                    if( !rSh.HasReadonlySel() )
                    {
                        if( aTmpQHD.HasCntnt() && !rSh.HasSelection() )
                        {
                            // zum naechsten Tip
                            aTmpQHD.Inc( pACorr && pACorr->GetSwFlags().
                                                   bAutoCmpltEndless );
                            eKeyState = KS_NextPrevGlossary;
                        }
                        else if( rSh.GetTableFmt() )
                            eKeyState = KS_InsTab;
                        else if( rSh.IsSttOfPara() )
                        {
                            if( rSh.GetCurNumRule() )
                                eKeyState = KS_InsTab;
                            else
                            {
                                SwTxtFmtColl* pColl = rSh.GetCurTxtFmtColl();
                                if( pColl && 0 <= pColl->GetOutlineLevel() &&
                                    MAXLEVEL - 1 > pColl->GetOutlineLevel() )
                                    eKeyState = KS_InsTab;
                            }
                        }
                    }
                    break;

                case KEY_TAB | KEY_MOD1 | KEY_SHIFT:
                    if( aTmpQHD.HasCntnt() && !rSh.HasSelection() &&
                        !rSh.HasReadonlySel() )
                    {
                        // zum vorherigen Tip
                        aTmpQHD.Dec( pACorr && pACorr->GetSwFlags().
                                                    bAutoCmpltEndless );
                        eKeyState = KS_NextPrevGlossary;
                    }
                    break;
                }
            }
            break;

        case KS_KeyToView:
            {
                eKeyState = KS_Ende;
                bNormalChar = !rKeyCode.IsControlMod() &&
                                SW_ISPRINTABLE( aCh );
                if( aInBuffer.Len() && ( !bNormalChar || bIsDocReadOnly ))
                    FlushInBuffer( &rSh );

                if( rView.KeyInput( rKEvt ) )
                    bFlushBuffer = TRUE, bNormalChar = FALSE;
                else
                {
// OS 22.09.95: Da der Sfx Acceleratoren nur aufruft, wenn sie beim letzten
//              Statusupdate enabled wurden, muss copy ggf. von uns
//              'gewaltsam' gerufen werden.
                    if( rKeyCode.GetFunction() == KEYFUNC_COPY )
                        GetView().GetViewFrame()->GetDispatcher()->Execute(SID_COPY);


                    if( !bIsDocReadOnly && bNormalChar )
                        eKeyState = KS_InsChar;
                    else
                    {
                        bNormalChar = FALSE;
                        Window::KeyInput( rKEvt );
                    }
                }
            }
            break;

        case KS_InsTab:
            if( rView.ISA( SwWebView ))     //Kein Tabulator fuer Web!
            {
                // Bug 56196 - dann sollte der weiter gereicht werden.
                Window::KeyInput( rKEvt );
                eKeyState = KS_Ende;
                break;
            }
            aCh = '\t';
            // kein break!
        case KS_InsChar:
        if( !rSh.HasReadonlySel() )
        {
            BOOL bIsNormalChar = GetAppCharClass().isLetterNumeric(
                                                        String( aCh ), 0 );
            if( bChkInsBlank && bIsNormalChar &&
                (aInBuffer.Len() || !rSh.IsSttPara() || !rSh.IsEndPara() ))
            {
                // vor dem Zeichen noch ein Blank einfuegen. Dieses
                // kommt zwischen den Expandierten Text und dem neuen
                // "nicht Worttrenner".
                aInBuffer.Expand( aInBuffer.Len() + 1, ' ' );
            }


            if( !rKEvt.GetRepeat() && pACorr &&
                (( pACorr->IsAutoCorrFlag( ChgWeightUnderl ) &&
                    ( '*' == aCh || '_' == aCh ) ) ||
                 ( pACorr->IsAutoCorrFlag( ChgQuotes ) && ('\"' == aCh ))||
                 ( pACorr->IsAutoCorrFlag( ChgSglQuotes ) && ( '\'' == aCh))))
            {
                FlushInBuffer( &rSh );
                rSh.AutoCorrect( *pACorr, aCh );
                if( '\"' != aCh && '\'' != aCh )        // nur bei "*_" rufen!
                    rSh.UpdateAttr();
            }
            else if( !rKEvt.GetRepeat() && pACorr &&
                pACorr->IsAutoCorrFlag( CptlSttSntnc | CptlSttWrd |
                                        ChgFractionSymbol | ChgOrdinalNumber |
                                        ChgToEnEmDash | SetINetAttr |
                                        Autocorrect ) &&
                '\"' != aCh && '\'' != aCh && '*' != aCh && '_' != aCh &&
                !bIsNormalChar
                )
            {
                FlushInBuffer( &rSh );
                rSh.AutoCorrect( *pACorr, aCh );
            }
            else
            {
                aInBuffer.Expand( aInBuffer.Len() + rKEvt.GetRepeat() + 1,aCh );
                bFlushCharBuffer = Application::AnyInput( INPUT_KEYBOARD );
                bFlushBuffer = !bFlushCharBuffer;
                if( bFlushCharBuffer )
                    aKeyInputFlushTimer.Start();
            }
            eKeyState = KS_Ende;
        }
        else
        {
            InfoBox( this, SW_RES( MSG_READONLY_CONTENT )).Execute();
// ???          Window::KeyInput( rKEvt );
            eKeyState = KS_Ende;
        }
        break;

        case KS_CheckAutoCorrect:
        {
            if( pACorr &&
                pACorr->IsAutoCorrFlag( CptlSttSntnc | CptlSttWrd |
                                        ChgFractionSymbol | ChgOrdinalNumber |
                                        ChgToEnEmDash | SetINetAttr |
                                        Autocorrect ) &&
                !rSh.HasReadonlySel() )
            {
                FlushInBuffer( &rSh );
                rSh.AutoCorrect( *pACorr, 0 );
            }
            eKeyState = eNextKeyState;
        }
        break;

        default:
        {
            TypeId pTypeId = 0;

            FlushInBuffer( &rSh );
//???           if( bFlushCharBuffer )
//???               FlushInBuffer( &rSh );
            switch( eKeyState )
            {
            case KS_AppendNodeInSection:
            {
                rSh.AppendNodeInSection();
            }
            break;

            case KS_NoNum:
            {
                rSh.NoNum();
                pTypeId = TYPE(SwListShell);
            }
            break;
            case KS_NumOff:
            {
                // Shellwechsel - also vorher aufzeichnen
                rSh.DelNumRules();
            }
            break;

            case KS_NumDown:
            {
                rSh.NumUpDown( TRUE );
            }
            break;
            case KS_NumUp:
            {
                rSh.NumUpDown( FALSE );
            }
            break;
            case KS_NumOrNoNum:
            {
                //Backspace in Numerierung und Nummer -> Nummer auschalten
                //SHIFT-Backspace in Numerierung und NoNum -> Nummer wieder einschalten
                pTypeId = TYPE(SwListShell);
            }
            break;

            case KS_OutlineDown:
                rSh.OutlineUpDown( 1 );
                break;
            case KS_OutlineUp:
                rSh.OutlineUpDown( -1 );
                break;

            case KS_NextCell:
            {
                //In Tabelle immer 'flushen'
                rSh.GoNextCell();
            }
            break;
            case KS_PrevCell:
            {
                rSh.GoPrevCell();
            }
            break;
            case KS_AutoFmtByInput:
            {
                rSh.SplitNode( TRUE );
                pTypeId = TYPE(SwTextShell);
            }
            break;

            case KS_DontExpand:
                break;

            case KS_GlossaryExpand:
            {
                // ersetze das Wort oder Kuerzel durch den den Textbaustein
                rSh.StartUndo( UNDO_START );

                String sFnd( *aTmpQHD.aArr[ aTmpQHD.nCurArrPos ] );
                if( aTmpQHD.bIsAutoText )
                {
                    SwGlossaryList* pList = ::GetGlossaryList();
                    String sShrtNm;
                    String sGroup;
                    if(pList->GetShortName( sFnd, sShrtNm, sGroup))
                    {
                        rSh.SttSelect();
                        rSh.ExtendSelection( FALSE, aTmpQHD.nLen );
                        SwGlossaryHdl* pGlosHdl = GetView().GetGlosHdl();
                        pGlosHdl->SetCurGroup(sGroup, TRUE);
                        pGlosHdl->InsertGlossary( sShrtNm);
                        pQuickHlpData->bChkInsBlank = TRUE;
                    }
                }
                else
                {
                    rSh.Insert( sFnd.Erase( 0, aTmpQHD.nLen ));
                    pQuickHlpData->bChkInsBlank = !pACorr ||
                            pACorr->GetSwFlags().bAutoCmpltAppendBlanc;
                }
                rSh.EndUndo( UNDO_END );
            }
            break;

            case KS_NextPrevGlossary:
                pQuickHlpData->Move( aTmpQHD );
                pQuickHlpData->Start( rSh, USHRT_MAX );
                break;

            case KS_EditFormula:
            {
                pTypeId = TYPE(SwTextShell);
                const USHORT nId = SwInputChild::GetChildWindowId();

                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                pVFrame->ToggleChildWindow( nId );
                SwInputChild* pChildWin = (SwInputChild*)pVFrame->
                                                    GetChildWindow( nId );
                if( pChildWin )
                    pChildWin->SetFormula( sFmlEntry );
            }
            break;

            case KS_ColLeftBig:         rSh.SetColRowWidthHeight( WH_COL_LEFT|WH_FLAG_BIGGER, pModOpt->GetTblHMove() ); break;
            case KS_ColRightBig:        rSh.SetColRowWidthHeight( WH_COL_RIGHT|WH_FLAG_BIGGER, pModOpt->GetTblHMove() );    break;
            case KS_ColLeftSmall:       rSh.SetColRowWidthHeight( WH_COL_LEFT, pModOpt->GetTblHMove() );    break;
            case KS_ColRightSmall:      rSh.SetColRowWidthHeight( WH_COL_RIGHT, pModOpt->GetTblHMove() );   break;
            case KS_ColTopBig:          rSh.SetColRowWidthHeight( WH_ROW_TOP|WH_FLAG_BIGGER, pModOpt->GetTblVMove() );  break;
            case KS_ColBottomBig:       rSh.SetColRowWidthHeight( WH_ROW_BOTTOM|WH_FLAG_BIGGER, pModOpt->GetTblVMove() );   break;
            case KS_ColTopSmall:        rSh.SetColRowWidthHeight( WH_ROW_TOP, pModOpt->GetTblVMove() ); break;
            case KS_ColBottomSmall:     rSh.SetColRowWidthHeight( WH_ROW_BOTTOM, pModOpt->GetTblVMove() );  break;
            case KS_CellLeftBig:        rSh.SetColRowWidthHeight( WH_CELL_LEFT|WH_FLAG_BIGGER, pModOpt->GetTblHMove() );    break;
            case KS_CellRightBig:       rSh.SetColRowWidthHeight( WH_CELL_RIGHT|WH_FLAG_BIGGER, pModOpt->GetTblHMove() );   break;
            case KS_CellLeftSmall:      rSh.SetColRowWidthHeight( WH_CELL_LEFT, pModOpt->GetTblHMove() );   break;
            case KS_CellRightSmall:     rSh.SetColRowWidthHeight( WH_CELL_RIGHT, pModOpt->GetTblHMove() );  break;
            case KS_CellTopBig:         rSh.SetColRowWidthHeight( WH_CELL_TOP|WH_FLAG_BIGGER, pModOpt->GetTblVMove() ); break;
            case KS_CellBottomBig:      rSh.SetColRowWidthHeight( WH_CELL_BOTTOM|WH_FLAG_BIGGER, pModOpt->GetTblVMove() );  break;
            case KS_CellTopSmall:       rSh.SetColRowWidthHeight( WH_CELL_TOP, pModOpt->GetTblVMove() );    break;
            case KS_CellBottomSmall:    rSh.SetColRowWidthHeight( WH_CELL_BOTTOM, pModOpt->GetTblVMove() ); break;

//---------------
            case KS_InsDel_ColLeftBig:          rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_COL_LEFT|WH_FLAG_BIGGER, pModOpt->GetTblHInsert() );    break;
            case KS_InsDel_ColRightBig:         rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_COL_RIGHT|WH_FLAG_BIGGER, pModOpt->GetTblHInsert() );   break;
            case KS_InsDel_ColLeftSmall:        rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_COL_LEFT, pModOpt->GetTblHInsert() );   break;
            case KS_InsDel_ColRightSmall:       rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_COL_RIGHT, pModOpt->GetTblHInsert() );  break;
            case KS_InsDel_ColTopBig:           rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_ROW_TOP|WH_FLAG_BIGGER, pModOpt->GetTblVInsert() ); break;
            case KS_InsDel_ColBottomBig:        rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_ROW_BOTTOM|WH_FLAG_BIGGER, pModOpt->GetTblVInsert() );  break;
            case KS_InsDel_ColTopSmall:         rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_ROW_TOP, pModOpt->GetTblVInsert() );    break;
            case KS_InsDel_ColBottomSmall:      rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_ROW_BOTTOM, pModOpt->GetTblVInsert() ); break;
            case KS_InsDel_CellLeftBig:         rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_CELL_LEFT|WH_FLAG_BIGGER, pModOpt->GetTblHInsert() );   break;
            case KS_InsDel_CellRightBig:        rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_CELL_RIGHT|WH_FLAG_BIGGER, pModOpt->GetTblHInsert() );  break;
            case KS_InsDel_CellLeftSmall:       rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_CELL_LEFT, pModOpt->GetTblHInsert() );  break;
            case KS_InsDel_CellRightSmall:      rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_CELL_RIGHT, pModOpt->GetTblHInsert() ); break;
            case KS_InsDel_CellTopBig:          rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_CELL_TOP|WH_FLAG_BIGGER, pModOpt->GetTblVInsert() );    break;
            case KS_InsDel_CellBottomBig:       rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_CELL_BOTTOM|WH_FLAG_BIGGER, pModOpt->GetTblVInsert() ); break;
            case KS_InsDel_CellTopSmall:        rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_CELL_TOP, pModOpt->GetTblVInsert() );   break;
            case KS_InsDel_CellBottomSmall:     rSh.SetColRowWidthHeight( WH_FLAG_INSDEL|WH_CELL_BOTTOM, pModOpt->GetTblVInsert() );    break;
//---------------
            case KS_TblColCellInsDel:
                rSh.SetColRowWidthHeight( eTblChgMode, nTblChgSize );
                break;
            case KS_Fly_Change:
                ChangeFly( nDir, rView.ISA( SwWebView ) );
                break;
            }

            eKeyState = KS_Ende;
        }
        }
    }

    if( bStopKeyInputTimer )
    {
        aKeyInputTimer.Stop();
        bTblInsDelMode = FALSE;
    }

    // falls die gepufferten Zeichen eingefuegt werden sollen
    if( bFlushBuffer && aInBuffer.Len() )
    {
        //OS 16.02.96 11.04: bFlushCharBuffer wurde hier nicht zurueckgesetzt
        // warum nicht?
        BOOL bSave = bFlushCharBuffer;
        FlushInBuffer(&rSh);
        bFlushCharBuffer = bSave;

        // evt. Tip-Hilfe anzeigen
        String sWord;
        if( bNormalChar && pACfg && pACorr &&
            ( pACfg->IsAutoTextTip() ||
              pACorr->GetSwFlags().bAutoCompleteWords ) &&
            rSh.GetPrevAutoCorrWord( *pACorr, sWord ) )
        {
            pQuickHlpData->ClearCntnt();
            if( pACfg->IsAutoTextTip() )
            {
                SwGlossaryList* pList = ::GetGlossaryList();
                pList->HasLongName( sWord, &pQuickHlpData->aArr );
            }

            if( pQuickHlpData->aArr.Count() )
            {
                pQuickHlpData->bIsTip = TRUE;
                pQuickHlpData->bIsAutoText = TRUE;
            }
            else if( pACorr->GetSwFlags().bAutoCompleteWords )
            {
                pQuickHlpData->bIsAutoText = FALSE;
                pQuickHlpData->bIsTip = !pACorr ||
                            pACorr->GetSwFlags().bAutoCmpltShowAsTip;
                const International* pIntl = &Application::GetAppInternational();
                {
                    // besorge mal die akt. Sprache:
                    SfxItemSet aSet( pObjSh->GetPool(), RES_CHRATR_LANGUAGE,
                                                        RES_CHRATR_LANGUAGE );
                    rSh.GetAttr( aSet );

                    LanguageType eLang = ((const SvxLanguageItem&)aSet.Get(
                                        RES_CHRATR_LANGUAGE )).GetLanguage();

                    if( eLang != pIntl->GetLanguage() &&
                        LANGUAGE_SYSTEM != eLang && LANGUAGE_DONTKNOW != eLang )
                        pIntl = new International( eLang );
                }

                for( int n = MONDAY; n <= SUNDAY; ++n )
                {
                    const String& rDay = pIntl->GetDayText( (DayOfWeek)n );
                    if( sWord.Len() + 1 < rDay.Len() &&
                        pIntl->CompareEqual(sWord, rDay.Copy(0,sWord.Len()), INTN_COMPARE_IGNORECASE ) )
                    {
                        String* pNew = new String( rDay );
                        if( !pQuickHlpData->aArr.Insert( pNew ) )
                            delete pNew;
                    }
                }

                for( n = 1; n <= 12; ++n )
                {
                    const String& rMon = pIntl->GetMonthText( n );
                    if( sWord.Len() + 1 < rMon.Len() &&
                        pIntl->CompareEqual(sWord, rMon.Copy(0,sWord.Len()), INTN_COMPARE_IGNORECASE ))
                    {
                        String* pNew = new String( rMon );
                        if( !pQuickHlpData->aArr.Insert( pNew ) )
                            delete pNew;
                    }
                }

                // wurde die International - Klasse von uns angelegt?
                if( pIntl != &Application::GetAppInternational() )
                    delete (International*)pIntl;

                USHORT nStt, nEnd;
                const SwAutoCompleteWord& rACLst = rSh.GetAutoCompleteWords();
                if( rACLst.GetRange( sWord, nStt, nEnd ) )
                {
                    while( nStt < nEnd )
                    {
                        const String& rS = rACLst[ nStt ];
                        //JP 16.06.99: Bug 66927 - only if the count of chars
                        //              from the suggest greater as the
                        //              actual word
                        if( rS.Len() > sWord.Len() )
                        {
                            String* pNew = new String( rS );
                            if( !pQuickHlpData->aArr.Insert( pNew ) )
                                delete pNew;
                        }
                        ++nStt;
                    }
                }
            }

            if( pQuickHlpData->aArr.Count() )
                pQuickHlpData->Start( rSh, sWord.Len() );
        }
    }
}

/*--------------------------------------------------------------------
     Beschreibung:  MouseEvents
 --------------------------------------------------------------------*/


void SwEditWin::RstMBDownFlags()
{
    //Nicht auf allen Systemen kommt vor dem modalen
    //Dialog noch ein MouseButton Up (wie unter WINDOWS).
    //Daher hier die Stati zuruecksetzen und die Maus
    //fuer den Dialog freigeben.
    bMBPressed = bNoInterrupt = FALSE;
    EnterArea();
    ReleaseMouse();
}





void SwEditWin::MouseButtonDown(const MouseEvent& rMEvt)
{
    GrabFocus();

    bWasShdwCrsr = 0 != pShadCrsr;
    if( bWasShdwCrsr )
        delete pShadCrsr, pShadCrsr = 0;

    SwWrtShell &rSh = rView.GetWrtShell();
    const Point aDocPos( PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( IsChainMode() )
    {
        SetChainMode( FALSE );
        SwRect aDummy;
        SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)rSh.GetFlyFrmFmt();
        if ( !rSh.Chainable( aDummy, *pFmt, aDocPos ) )
            rSh.Chain( *pFmt, aDocPos );
        UpdatePointer( aDocPos, rMEvt.GetModifier() );
        return;
    }

    //Nach GrabFocus sollte eine Shell gepusht sein. Das muss eigentlich
    //klappen aber in der Praxis ...
    lcl_SelectShellForDrop( rView );

    BOOL bIsDocReadOnly = rView.GetDocShell()->IsReadOnly();
    BOOL bCallBase = TRUE;

    if( pQuickHlpData->bClear )
        pQuickHlpData->Stop( rSh );
    pQuickHlpData->bChkInsBlank = FALSE;

    if( rSh.FinishOLEObj() )
        return; //InPlace beenden und der Klick zaehlt nicht mehr

    SET_CURR_SHELL( &rSh );

    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        if (pSdrView->MouseButtonDown( rMEvt, this ) )
        {
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(FALSE);
            return; // Event von der SdrView ausgewertet
        }
    }

#ifdef MAC
    if (rMEvt.IsMod1() && rMEvt.IsMod2())
        return;
#endif

    bIsInMove = FALSE;
    aStartPos = rMEvt.GetPosPixel();
    aRszMvHdlPt.X() = 0, aRszMvHdlPt.Y() = 0;

    if ( !rSh.IsDrawCreate() && !pApplyTempl && !rSh.IsInSelect() &&
         rMEvt.GetClicks() == 1 && MOUSE_LEFT == rMEvt.GetButtons() &&
         !rSh.IsTableMode() &&
         rSh.IsMouseTabCol( aDocPos ) )
    {
        //Zuppeln von Tabellenspalten aus dem Dokument heraus.
        rView.SetTabColFromDoc( TRUE );
        rView.SetTabColFromDocPos( aDocPos );
        rView.InvalidateRulerPos();
        SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
        rBind.Update();
        if ( RulerClook( rView , rMEvt ) )
        {
            rView.SetTabColFromDoc( FALSE );
            rView.InvalidateRulerPos();
            rBind.Update();
            bCallBase = FALSE;
        }
        else
        {
            return;
        }
    }

    //Man kann sich in einem Selektionszustand befinden, wenn zuletzt
    //mit dem Keyboard selektiert wurde, aber noch kein CURSOR_KEY
    //anschliessend bewegt worden ist. In diesem Fall muss die vorher-
    //gehende Selektion zuerst beendet werden.
    //MA 07. Oct. 95: Und zwar nicht nur bei Linker Maustaste sondern immer.
    //siehe auch Bug: 19263
    if ( rSh.IsInSelect() )
        rSh.EndSelect();

    //Abfrage auf LEFT, da sonst auch bei einem Click mit der rechten Taste
    //beispielsweise die Selektion aufgehoben wird.
    if ( MOUSE_LEFT == rMEvt.GetButtons() )
    {
        BOOL bOnlyText = FALSE;
        bMBPressed = bNoInterrupt = TRUE;
        CaptureMouse();

        //ggf. Cursorpositionen zuruecksetzen
        rSh.ResetCursorStack();

        switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
        {
            case MOUSE_LEFT:
            case MOUSE_LEFT + KEY_SHIFT:
            case MOUSE_LEFT + KEY_MOD2:
                if( rSh.IsObjSelected() )
                {
                    SdrHdl* pHdl;
                    if( !bIsDocReadOnly && !pAnchorMarker && 0 !=
                        ( pHdl = pSdrView->HitHandle(aDocPos, *(rSh.GetOut())) )
                        && pHdl->GetKind() == HDL_ANCHOR )
                    {
                        pAnchorMarker = new SwAnchorMarker( pHdl );
                        UpdatePointer( aDocPos, rMEvt.GetModifier() );
                        return;
                    }
                }
                if ( EnterDrawMode( rMEvt, aDocPos ) )
                {
                    bNoInterrupt = FALSE;
                    return;
                }
                else  if ( rView.GetDrawFuncPtr() && bInsFrm )
                {
                    StopInsFrm();
                    rSh.Edit();
                }

                // Ohne SHIFT, da sonst Toggle bei Selektion nicht funktioniert
                if (rMEvt.GetClicks() == 1)
                {
                    if ( rSh.IsSelFrmMode())
                    {
                        SdrHdl* pHdl = rSh.GetDrawView()->HitHandle
                                                    (aDocPos, *(rSh.GetOut()));
                        BOOL bHitHandle = pHdl && pHdl->GetKind() != HDL_ANCHOR;

                        if ((rSh.IsInsideSelectedObj(aDocPos) || bHitHandle) &&
                            !(rMEvt.GetModifier() == KEY_SHIFT && !bHitHandle))
                        {
                            rSh.EnterSelFrmMode( &aDocPos );
                            if ( !pApplyTempl )
                            {
                                //nur, wenn keine Position zum Sizen getroffen ist.
                                if (!bHitHandle)
                                {
                                    StartDDTimer();
                                    SwEditWin::nDDStartPosY = aDocPos.Y();
                                    SwEditWin::nDDStartPosX = aDocPos.X();
                                }
                                bFrmDrag = TRUE;
                            }
                            bNoInterrupt = FALSE;
                            return;
                        }
                    }
                }
        }

        BOOL bExecHyperlinks = rSh.GetViewOptions()->IsExecHyperlinks()^
                            (rMEvt.GetModifier() == KEY_MOD2 ? TRUE : FALSE);

        switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
        {
            case MOUSE_LEFT:
            case MOUSE_LEFT + KEY_MOD1:
            case MOUSE_LEFT + KEY_MOD2:
                switch ( rMEvt.GetClicks() )
                {
                    case 1:
                    {
                        UpdatePointer( aDocPos, rMEvt.GetModifier() );
                        SwEditWin::nDDStartPosY = aDocPos.Y();
                        SwEditWin::nDDStartPosX = aDocPos.X();

                        // URL in DrawText-Objekt getroffen?
                        BOOL bExecDrawTextLink = FALSE;

                        if (bExecHyperlinks && pSdrView)
                        {
                            SdrViewEvent aVEvt;
                            SdrHitKind eHit = pSdrView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                            if (aVEvt.eEvent == SDREVENT_EXECUTEURL)
                                bExecDrawTextLink = TRUE;
                        }

                        //Rahmen nur zu selektieren versuchen, wenn
                        //der Pointer bereits entsprechend geschaltet wurde
                        if ( aActHitType != SDRHIT_NONE && !rSh.IsSelFrmMode() &&
                            !SFX_APP()->IsDispatcherLocked() &&
                            !bExecDrawTextLink)
                        {
                            rView.NoRotate();
                            rSh.HideCrsr();
                            if( rSh.SelectObj( aDocPos, FALSE, rMEvt.IsMod1() ) )
                            {
                                // falls im Macro der Rahmen deselektiert
                                // wurde, muss nur noch der Cursor
                                // wieder angezeigt werden.
                                if( FRMTYPE_NONE == rSh.GetSelFrmType() )
                                    rSh.ShowCrsr();
                                else
                                {
                                    if (rSh.IsFrmSelected() && rView.GetDrawFuncPtr())
                                    {
                                        rView.GetDrawFuncPtr()->Deactivate();
                                        rView.SetDrawFuncPtr(NULL);
                                        rView.LeaveDrawCreate();
                                        rView.AttrChangedNotify( &rSh );
                                    }

                                    rSh.EnterSelFrmMode( &aDocPos );
                                    bFrmDrag = TRUE;
                                    UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                }
                                return;
                            }
                            else
                                bOnlyText = rSh.IsObjSelectable( aDocPos );

                            if (!rView.GetDrawFuncPtr())
                                rSh.ShowCrsr();
                        }
                        else if ( rSh.IsSelFrmMode() &&
                                  (aActHitType == SDRHIT_NONE ||
                                   !rSh.IsInsideSelectedObj( aDocPos )))
                        {
                            rView.NoRotate();
                            SdrHdl *pHdl;
                            if( !bIsDocReadOnly && !pAnchorMarker && 0 !=
                                ( pHdl = pSdrView->HitHandle(aDocPos, *(rSh.GetOut())) )
                                && pHdl->GetKind() == HDL_ANCHOR )
                            {
                                pAnchorMarker = new SwAnchorMarker( pHdl );
                                UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                return;
                            }
                            else
                            {
                                BOOL bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( TRUE );
                                BOOL bSelObj = rSh.SelectObj( aDocPos,
                                        rMEvt.IsShift(), rMEvt.IsMod1() );
                                if( bUnLockView )
                                    rSh.LockView( FALSE );

                                if( !bSelObj )
                                {
                                    // Cursor hier umsetzen, damit er nicht zuerst
                                    // im Rahmen gezeichnet wird; ShowCrsr() geschieht
                                    // in LeaveSelFrmMode()
                                    bValidCrsrPos = !(CRSR_POSCHG & (rSh.*rSh.fnSetCrsr)(&aDocPos,FALSE));
                                    rSh.LeaveSelFrmMode();
                                    rView.AttrChangedNotify( &rSh );
                                    bCallBase = FALSE;
                                }
                                else
                                {
                                    rSh.HideCrsr();
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    rSh.SelFlyGrabCrsr();
                                    rSh.MakeSelVisible();
                                    bFrmDrag = TRUE;
                                    if( rSh.IsFrmSelected() &&
                                        rView.GetDrawFuncPtr() )
                                    {
                                        rView.GetDrawFuncPtr()->Deactivate();
                                        rView.SetDrawFuncPtr(NULL);
                                        rView.LeaveDrawCreate();
                                        rView.AttrChangedNotify( &rSh );
                                    }
                                    UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                    return;
                                }
                            }
                        }
                        break;
                    }
                    case 2:
                    {
                        bFrmDrag = FALSE;
                        if ( !bIsDocReadOnly && rSh.IsInsideSelectedObj(aDocPos) &&
                             0 == rSh.IsSelObjProtected( (FlyProtectType)(FLYPROTECT_CONTENT|FLYPROTECT_PARENT) ) )
                        switch ( rSh.GetSelectionType() )
                        {
                            case SwWrtShell::SEL_GRF:
                                RstMBDownFlags();
                                GetView().GetViewFrame()->GetDispatcher()->Execute(
                                    FN_FORMAT_GRAFIC_DLG, SFX_CALLMODE_RECORD|SFX_CALLMODE_SLOT);
                                return;

                                // Doppelklick auf OLE-Objekt --> OLE-InPlace
                            case SwWrtShell::SEL_OLE:
                                if (!rSh.IsSelObjProtected(FLYPROTECT_CONTENT))
                                {
                                    RstMBDownFlags();
                                    rSh.LaunchOLEObj();
                                }
                                return;

                            case SwWrtShell::SEL_FRM:
                                RstMBDownFlags();
                                GetView().GetViewFrame()->GetDispatcher()->Execute(
                                    FN_FORMAT_FRAME_DLG, SFX_CALLMODE_RECORD|SFX_CALLMODE_SLOT);
                                return;

                            case SwWrtShell::SEL_DRW:
                                RstMBDownFlags();
                                EnterDrawTextMode(aDocPos);
                                if ( rView.GetCurShell()->ISA(SwDrawTextShell) )
                                    ((SwDrawTextShell*)rView.GetCurShell())->Init();
                                return;
                        }

                        //falls die Cursorposition korrigiert wurde oder
                        // ein Fly im ReadOnlyModus selektiert ist,
                        //keine Wortselektion.
                        if ( !bValidCrsrPos ||
                            (rSh.IsFrmSelected() && rSh.IsFrmSelected() ))
                            return;

                        SwField *pFld;
                        BOOL bFtn = FALSE;

                        if( !bIsDocReadOnly &&
                            ( 0 != ( pFld = rSh.GetCurFld() ) ||
                              0 != ( bFtn = rSh.GetCurFtn() )) )
                        {
                            RstMBDownFlags();
                            if( bFtn )
                                GetView().GetViewFrame()->GetDispatcher()->Execute( FN_EDIT_FOOTNOTE );
                            else
                            {
                                USHORT nTypeId = pFld->GetTypeId();
                                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                                switch( nTypeId )
                                {
                                case TYP_POSTITFLD:
                                case TYP_SCRIPTFLD:
                                {
                                    //falls es ein Readonly-Bereich ist, dann muss der Status
                                    //enabled werden
                                    USHORT nSlot = TYP_POSTITFLD == nTypeId ? FN_POSTIT : FN_JAVAEDIT;
                                    SfxBoolItem aItem(nSlot, TRUE);
                                    pVFrame->GetBindings().SetState(aItem);
                                    pVFrame->GetDispatcher()->Execute(nSlot);
                                    break;
                                }
                                case TYP_AUTHORITY :
                                    pVFrame->GetDispatcher()->Execute(FN_EDIT_AUTH_ENTRY_DLG);
                                break;
                                default:
                                    pVFrame->GetBindings().Execute(FN_EDIT_FIELD);
                                }
                            }
                            return;
                        }
                        //im Extended Mode hat Doppel- und
                        //Dreifachklick keine Auswirkungen.
                        if ( rSh.IsExtMode() )
                            return;

                        //Wort selektieren, gfs. Additional Mode
                        if ( KEY_MOD1 == rMEvt.GetModifier() && !rSh.IsAddMode() )
                        {
                            rSh.EnterAddMode();
                            rSh.SelWrd( &aDocPos );
                            rSh.LeaveAddMode();
                        }
                        else
                            rSh.SelWrd( &aDocPos );
                        bHoldSelection = TRUE;
                        return;
                    }
                    case 3:
                        bFrmDrag = FALSE;
                        //im Extended Mode hat Doppel- und
                        //Dreifachklick keine Auswirkungen.
                        if ( rSh.IsExtMode() )
                            return;

                        //falls die Cursorposition korrigiert wurde oder
                        // ein Fly im ReadOnlyModus selektiert ist,
                        //keine Wortselektion.
                        if ( !bValidCrsrPos || rSh.IsFrmSelected() )
                            return;

                        //Zeile selektieren, gfs. Additional Mode
                        if ( KEY_MOD1 == rMEvt.GetModifier() && !rSh.IsAddMode())
                        {
                            rSh.EnterAddMode();
                            rSh.SelLine( &aDocPos );
                            rSh.LeaveAddMode();
                        }
                        else
                            rSh.SelLine( &aDocPos );
                        bHoldSelection = TRUE;
                        return;

                    default:
                        return;
                }
                /* no break */
            case MOUSE_LEFT + KEY_SHIFT:
            case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            {
                BOOL bLockView = bWasShdwCrsr;

                switch ( rMEvt.GetModifier() )
                {
                    case KEY_MOD1 + KEY_SHIFT:
                    {
                        if ( !bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) )
                        {
                            rView.NoRotate();
                            rSh.HideCrsr();
                            if ( rSh.IsSelFrmMode() )
                                rSh.SelectObj(aDocPos, TRUE, TRUE);
                            else
                            {   if ( rSh.SelectObj( aDocPos, TRUE, TRUE ) )
                                {
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    SwEditWin::nDDStartPosY = aDocPos.Y();
                                    SwEditWin::nDDStartPosX = aDocPos.X();
                                    bFrmDrag = TRUE;
                                    return;
                                }
                            }
                        }
                        else if( rSh.IsSelFrmMode() &&
                                 rSh.GetDrawView()->HitHandle( aDocPos,
                                                            *rSh.GetOut() ))
                        {
                            bFrmDrag = TRUE;
                            bNoInterrupt = FALSE;
                            return;
                        }
                    }
                    break;
                    case KEY_MOD1:
                    {
                        if ( !bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) )
                        {
                            rView.NoRotate();
                            rSh.HideCrsr();
                            if ( rSh.IsSelFrmMode() )
                                rSh.SelectObj(aDocPos, FALSE, TRUE);
                            else
                            {   if ( rSh.SelectObj( aDocPos, FALSE, TRUE ) )
                                {
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    SwEditWin::nDDStartPosY = aDocPos.Y();
                                    SwEditWin::nDDStartPosX = aDocPos.X();
                                    bFrmDrag = TRUE;
                                    return;
                                }
                            }
                        }
                        else if( rSh.IsSelFrmMode() &&
                                 rSh.GetDrawView()->HitHandle( aDocPos,
                                                            *rSh.GetOut() ))
                        {
                            bFrmDrag = TRUE;
                            bNoInterrupt = FALSE;
                            return;
                        }
                        else
                        {
                            if ( !rSh.IsAddMode() && !rSh.IsExtMode())
                            {
                                rSh.PushMode();
                                bModePushed = TRUE;
                                rSh.EnterAddMode();
                            }
                            bCallBase = FALSE;
                        }
                    }
                    break;
                    case KEY_SHIFT:
                    {
                        if ( !bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) )
                        {
                            rView.NoRotate();
                            rSh.HideCrsr();
                            if ( rSh.IsSelFrmMode() )
                            {
                                rSh.SelectObj(aDocPos, TRUE);

                                const SdrMarkList& rMarkList = pSdrView->GetMarkList();
                                if (rMarkList.GetMark(0) == NULL)
                                {
                                    rSh.LeaveSelFrmMode();
                                    rView.AttrChangedNotify(&rSh);
                                    bFrmDrag = FALSE;
                                }
                            }
                            else
                            {   if ( rSh.SelectObj( aDocPos ) )
                                {
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    SwEditWin::nDDStartPosY = aDocPos.Y();
                                    SwEditWin::nDDStartPosX = aDocPos.X();
                                    bFrmDrag = TRUE;
                                    return;
                                }
                            }
                        }
                        else
                        {
                            if ( rSh.IsSelFrmMode() &&
                                 rSh.IsInsideSelectedObj( aDocPos ) )
                            {
                                rSh.EnterSelFrmMode( &aDocPos );
                                SwEditWin::nDDStartPosY = aDocPos.Y();
                                SwEditWin::nDDStartPosX = aDocPos.X();
                                bFrmDrag = TRUE;
                                return;
                            }
                            if ( rSh.IsSelFrmMode() )
                            {
                                rSh.UnSelectFrm();
                                rSh.LeaveSelFrmMode();
                                rView.AttrChangedNotify(&rSh);
                                bFrmDrag = FALSE;
                            }
                            if ( !rSh.IsExtMode() )
                            {
                                // keine Selection anfangen, wenn in ein URL-
                                // Feld oder eine -Grafik geklickt wird
                                BOOL bSttSelect = rSh.HasSelection() ||
                                                Pointer(POINTER_REFHAND) != GetPointer();

                                if( !bSttSelect )
                                {
                                    bSttSelect = TRUE;
                                    if( bExecHyperlinks )
                                    {
                                        SwContentAtPos aCntntAtPos(
                                            SwContentAtPos::SW_FTN |
                                            SwContentAtPos::SW_INETATTR );

                                        if( rSh.GetContentAtPos( aDocPos, aCntntAtPos ) )
                                        {
                                            if( !rSh.IsViewLocked() &&
                                                !rSh.IsReadOnlyAvailable() &&
                                                aCntntAtPos.IsInProtectSect() )
                                                    bLockView = TRUE;

                                            bSttSelect = FALSE;
                                        }
                                        else if( rSh.IsURLGrfAtPos( aDocPos ))
                                            bSttSelect = FALSE;
                                    }
                                }

                                if( bSttSelect )
                                    rSh.SttSelect();
                            }
                        }
                        bCallBase = FALSE;
                        break;
                    }
                    default:
                        if( !rSh.IsViewLocked() )
                        {
                            SwContentAtPos aCntntAtPos( SwContentAtPos::SW_CLICKFIELD |
                                                        SwContentAtPos::SW_INETATTR );
                            if( rSh.GetContentAtPos( aDocPos, aCntntAtPos, FALSE ) &&
                                !rSh.IsReadOnlyAvailable() &&
                                aCntntAtPos.IsInProtectSect() )
                                bLockView = TRUE;
                        }
                }

                if ( rSh.IsGCAttr() )
                {
                    rSh.GCAttr();
                    rSh.ClearGCAttr();
                }

                BOOL bOverSelect = rSh.ChgCurrPam( aDocPos ), bOverURLGrf = FALSE;
                if( !bOverSelect )
                    bOverURLGrf = bOverSelect = 0 != rSh.IsURLGrfAtPos( aDocPos );

                if ( !bOverSelect )
                {
                    const BOOL bTmp = bNoInterrupt;
                    bNoInterrupt = FALSE;

                    if( !rSh.IsViewLocked() && bLockView )
                        rSh.LockView( TRUE );
                    else
                        bLockView = FALSE;

                    {   // nur temp. Move-Kontext aufspannen, da sonst die
                        // Abfrage auf die Inhaltsform nicht funktioniert!!!
                        MV_KONTEXT( &rSh );
                        bValidCrsrPos = !(CRSR_POSCHG & (rSh.*rSh.fnSetCrsr)(&aDocPos,bOnlyText));
                        bCallBase = FALSE;
                    }

                    if( bLockView )
                        rSh.LockView( FALSE );

                    bNoInterrupt = bTmp;
                }
                if( !bOverURLGrf )
                {
                    const int nSelType = rSh.GetSelectionType();
                    if( nSelType == SwWrtShell::SEL_OLE ||
                        nSelType == SwWrtShell::SEL_GRF )
                    {
                        MV_KONTEXT( &rSh );
                        if( !rSh.IsFrmSelected() )
                            rSh.GotoNextFly();
                        rSh.EnterSelFrmMode();
                        bCallBase = FALSE;
                    }
                }
                // nicht mehr hier zuruecksetzen, damit -- falls durch MouseMove
                // bei gedrueckter Ctrl-Taste eine Mehrfachselektion erfolgen soll,
                // im Drag nicht die vorherige Selektion aufgehoben wird.
//              if(bModePushed)
//                  rSh.PopMode(FALSE);
                break;
            }
        }
    }
    if (bCallBase)
        Window::MouseButtonDown(rMEvt);
}

#pragma optimize("",off)

/*--------------------------------------------------------------------
    Beschreibung:   MouseMove
 --------------------------------------------------------------------*/


void SwEditWin::MouseMove(const MouseEvent& rMEvt)
{
    // solange eine Action laeuft sollte das MouseMove abgeklemmt sein
    // Ansonsten gibt es den Bug 40102
    SwWrtShell &rSh = rView.GetWrtShell();
    if( rSh.ActionPend() )
        return ;

    if( pShadCrsr && 0 != (rMEvt.GetModifier() + rMEvt.GetButtons() ) )
        delete pShadCrsr, pShadCrsr = 0;

    BOOL bIsDocReadOnly = rView.GetDocShell()->IsReadOnly();

    SET_CURR_SHELL( &rSh );

    //aPixPt == Point in Pixel, rel. zu ChildWin
    //aDocPt == Point in Twips, Dokumentkoordinaten
    const Point aPixPt( rMEvt.GetPosPixel() );
    const Point aDocPt( PixelToLogic( aPixPt ) );

    if ( IsChainMode() )
    {
        UpdatePointer( aDocPt, rMEvt.GetModifier() );
        if ( rMEvt.IsLeaveWindow() )
            rView.GetViewFrame()->HideStatusText();
        return;
    }

    SdrView *pSdrView = rSh.GetDrawView();

    const SwCallMouseEvent aLastCallEvent( aSaveCallEvent );
    aSaveCallEvent.Clear();

    if ( !bIsDocReadOnly && pSdrView && pSdrView->MouseMove(rMEvt,this) )
    {
        SetPointer( POINTER_TEXT );
        return; // Event von der SdrView ausgewertet
    }

    const Point aOldPt( rSh.VisArea().Pos() );
    const BOOL bInsWin = rSh.VisArea().IsInside( aDocPt );

    if( pShadCrsr && !bInsWin )
        delete pShadCrsr, pShadCrsr = 0;

    // Position ist noetig fuer OS/2, da dort nach einem MB-Down
    // offensichtlich sofort ein MB-Move gerufen wird.
    if( bDDTimerStarted )
    {
        Point aDD( SwEditWin::nDDStartPosX, SwEditWin::nDDStartPosY );
        aDD = LogicToPixel( aDD );
        Rectangle aRect( aDD.X()-3, aDD.Y()-3, aDD.X()+3, aDD.Y()+3 );
        if ( !aRect.IsInside( aPixPt ) )    // MA 23. May. 95: Tatterschutz.
            StopDDTimer( &rSh, aDocPt );
    }

    if ( bInsDraw && rView.GetDrawFuncPtr() )
    {
        rView.GetDrawFuncPtr()->MouseMove( rMEvt );
        if ( !bInsWin )
        {
            Point aTmp( aDocPt );
            aTmp += rSh.VisArea().Pos() - aOldPt;
            LeaveArea( aTmp );
        }
        else
            EnterArea();
        return;
    }

    if( !bIsDocReadOnly && bInsWin && !pApplyTempl && !rSh.IsInSelect() &&
         rSh.IsMouseTabCol( aDocPt ) && !rSh.IsTableMode())
    {
        //Zuppeln von Tabellenspalten aus dem Dokument heraus.
        SetPointer( POINTER_HSIZEBAR );
        return;
    }

    BOOL bDelShadCrsr = TRUE;

    switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
    {
        case MOUSE_LEFT:
            if( pAnchorMarker )
            {
                Point aNew = rSh.FindAnchorPos( aDocPt );
                if( aNew.X() || aNew.Y() )
                {
                    pAnchorMarker->SetPos( aNew );
                    pAnchorMarker->SetLastPos( aDocPt );
                    pSdrView->RefreshAllIAOManagers();
                }
            }
            if ( bInsDraw )
            {
                if ( !bMBPressed )
                    break;
                if ( bIsInMove || IsMinMove( aStartPos, aPixPt ) )
                {
                    if ( !bInsWin )
                        LeaveArea( aDocPt );
                    else
                        EnterArea();
                    if ( rView.GetDrawFuncPtr() )
                    {
                        pSdrView->SetOrtho(FALSE);
                        rView.GetDrawFuncPtr()->MouseMove( rMEvt );
                    }
                    bIsInMove = TRUE;
                }
                return;
            }
        case MOUSE_LEFT + KEY_SHIFT:
        case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            if ( !bMBPressed )
                break;
        case MOUSE_LEFT + KEY_MOD1:
            if ( bFrmDrag && rSh.IsSelFrmMode() )
            {
                if( !bMBPressed )
                    break;

                if ( bIsInMove || IsMinMove( aStartPos, aPixPt ) )
                {
                    // Event-Verarbeitung fuers Resizen
                    if( pSdrView->HasMarkedObj() )
                    {
                        const SwFrmFmt* pFlyFmt;
                        const SvxMacro* pMacro;

                        const Point aSttPt( PixelToLogic( aStartPos ) );

                        // geht es los?
                        if( HDL_USER == eSdrMoveHdl )
                        {
                            SdrHdl* pHdl = pSdrView->HitHandle( aSttPt, *this );
                            eSdrMoveHdl = pHdl ? pHdl->GetKind() : HDL_MOVE;
                        }

                        USHORT nEvent = HDL_MOVE == eSdrMoveHdl
                                            ? SW_EVENT_FRM_MOVE
                                            : SW_EVENT_FRM_RESIZE;

                        if( 0 != ( pFlyFmt = rSh.GetFlyFrmFmt() ) &&
                            0 != ( pMacro = pFlyFmt->GetMacro().GetMacroTable().
                            Get( nEvent )) &&
// oder nur z.B. alle 20 Twip bescheid sagen?
//                          ( 20 > Abs( aRszMvHdlPt.X() - aDocPt.X() ) ||
//                            20 > Abs( aRszMvHdlPt.Y() - aDocPt.Y() ) )
                            aRszMvHdlPt != aDocPt )
                        {
                            aRszMvHdlPt = aDocPt;
                            USHORT nPos = 0;
                            String sRet;
                            SbxArrayRef xArgs = new SbxArray;
                            SbxVariableRef xVar = new SbxVariable;
                            xVar->PutString( pFlyFmt->GetName() );
                            xArgs->Put( &xVar, ++nPos );

                            if( SW_EVENT_FRM_RESIZE == nEvent )
                            {
                                xVar = new SbxVariable;
                                xVar->PutUShort( eSdrMoveHdl );
                                xArgs->Put( &xVar, ++nPos );
                            }

                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.X() - aSttPt.X() );
                            xArgs->Put( &xVar, ++nPos );
                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.Y() - aSttPt.Y() );
                            xArgs->Put( &xVar, ++nPos );

                            ReleaseMouse();

                            rSh.ExecMacro( *pMacro, &sRet, &xArgs );

                            CaptureMouse();

                            if( sRet.Len() && 0 != sRet.ToInt32() )
                                return ;
                        }
                    }
                    // Event-Verarbeitung fuers Resizen

                    if( bIsDocReadOnly )
                        break;

                    if ( rMEvt.IsShift() )
                    {
                        pSdrView->SetOrtho(TRUE);
                        pSdrView->SetAngleSnapEnabled(TRUE);
                    }
                    else
                    {
                        pSdrView->SetOrtho(FALSE);
                        pSdrView->SetAngleSnapEnabled(FALSE);
                    }

                    (rSh.*rSh.fnDrag)( &aDocPt, rMEvt.IsShift() );
                    bIsInMove = TRUE;
                }
                else if( bIsDocReadOnly )
                    break;

                if ( !bInsWin )
                {
                    Point aTmp( aDocPt );
                    aTmp += rSh.VisArea().Pos() - aOldPt;
                    LeaveArea( aTmp );
                }
                else if(bIsInMove)
                    EnterArea();
                return;
            }
            if ( !rSh.IsSelFrmMode() && !bDDINetAttr &&
                (IsMinMove( aStartPos,aPixPt ) || bIsInMove) &&
                (rSh.IsInSelect() || !rSh.ChgCurrPam( aDocPt )) )
            {
                if ( pSdrView )
                {
                    if ( rMEvt.IsShift() )
                        pSdrView->SetOrtho(TRUE);
                    else
                        pSdrView->SetOrtho(FALSE);
                }
                if ( !bInsWin )
                {
                    Point aTmp( aDocPt );
                    aTmp += rSh.VisArea().Pos() - aOldPt;
                    LeaveArea( aTmp );
                }
                else
                {
                    //JP 24.09.98: Fix fuer die Bugs 55592 / 55931
                    //JP 23.04.99: Fix fuer den Bugs 65289
                    //JP 06.07.99: Fix fuer den Bugs 67360
                    if( !rMEvt.IsSynthetic() &&
                            !(( MOUSE_LEFT + KEY_MOD1 ==
                            rMEvt.GetModifier() + rMEvt.GetButtons() ) &&
                            rSh.Is_FnDragEQBeginDrag() && !rSh.IsAddMode() ))
                    {
                        (rSh.*rSh.fnDrag)( &aDocPt,FALSE );

                        bValidCrsrPos = !(CRSR_POSCHG & (rSh.*rSh.fnSetCrsr)(&aDocPt,FALSE));
                        EnterArea();
                    }
                }
            }
            bDDINetAttr = FALSE;
            break;
        case 0:
            if ( pApplyTempl )
            {
                UpdatePointer(aDocPt, 0); // evtl. muss hier ein Rahmen markiert werden
                break;
            }
        case KEY_SHIFT:
        case KEY_MOD2:
        case KEY_MOD1:
            if ( !bInsDraw )
            {
                BOOL bTstShdwCrsr = TRUE;

                UpdatePointer( aDocPt, rMEvt.GetModifier() );

                const SwFrmFmt* pFmt;
                const SwFmtINetFmt* pINet = 0;
                SwContentAtPos aCntntAtPos( SwContentAtPos::SW_INETATTR );
                if( rSh.GetContentAtPos( aDocPt, aCntntAtPos ) )
                    pINet = (SwFmtINetFmt*)aCntntAtPos.aFnd.pAttr;

                const void* pTmp = pINet;

                if( pINet ||
                    0 != ( pTmp = pFmt = rSh.GetFmtFromAnyObj( aDocPt )))
                {
                    bTstShdwCrsr = FALSE;
                    if( pTmp == pINet )
                        aSaveCallEvent.Set( pINet );
                    else
                    {
                        IMapObject* pIMapObj = pFmt->GetIMapObject( aDocPt );
                        if( pIMapObj )
                            aSaveCallEvent.Set( pFmt, pIMapObj );
                        else
                            aSaveCallEvent.Set( EVENT_OBJECT_URLITEM, pFmt );
                    }

                    // sollte wir ueber einem InternetFeld mit einem
                    // gebundenen Macro stehen?
                    if( aSaveCallEvent != aLastCallEvent )
                    {
                        if( aLastCallEvent.HasEvent() )
                            rSh.CallEvent( SFX_EVENT_MOUSEOUT_OBJECT,
                                            aLastCallEvent, TRUE );
                        // 0 besagt, das das Object gar keine Tabelle hat
                        if( !rSh.CallEvent( SFX_EVENT_MOUSEOVER_OBJECT,
                                        aSaveCallEvent ))
                            aSaveCallEvent.Clear();
                    }
                }
                else if( aLastCallEvent.HasEvent() )
                {
                    // Cursor stand auf einem Object
                    rSh.CallEvent( SFX_EVENT_MOUSEOUT_OBJECT,
                                    aLastCallEvent, TRUE );
                }

                if( bTstShdwCrsr && bInsWin && !bIsDocReadOnly &&
                    !bInsFrm && !rSh.IsBrowseMode() &&
                    rSh.GetViewOptions()->IsShadowCursor() &&
                    !(rMEvt.GetModifier() + rMEvt.GetButtons()) &&
                    !rSh.HasSelection() && !GetConnectMetaFile() )
                {
                    SwRect aRect;
                    SwHoriOrient eOrient;
                    SwFillMode eMode = (SwFillMode)rSh.GetViewOptions()->GetShdwCrsrFillMode();
                    if( rSh.GetShadowCrsrPos( aDocPt, eMode, aRect, eOrient ))
                    {
                        if( !pShadCrsr )
                            pShadCrsr = new SwShadowCursor( *this,
                                rSh.GetViewOptions()->GetShdwCrsrColor() );
                        if( HORI_RIGHT != eOrient && HORI_CENTER != eOrient )
                            eOrient = HORI_LEFT;
                        pShadCrsr->SetPos( aRect.Pos(), aRect.Height(), eOrient );
                        bDelShadCrsr = FALSE;
                    }
                }
            }
            break;
    }

    if( bDelShadCrsr && pShadCrsr )
        delete pShadCrsr, pShadCrsr = 0;
    bWasShdwCrsr = FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:   Button Up
 --------------------------------------------------------------------*/


void SwEditWin::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bCallBase = TRUE;

    BOOL bCallShadowCrsr = bWasShdwCrsr;
    bWasShdwCrsr = FALSE;
    if( pShadCrsr )
        delete pShadCrsr, pShadCrsr = 0;

    SdrHdlKind eOldSdrMoveHdl = eSdrMoveHdl;
    eSdrMoveHdl = HDL_USER;     // fuer die MoveEvents - wieder zuruecksetzen

    // sicherheitshalber zuruecksetzen Bug 27900
    rView.SetTabColFromDoc( FALSE );
    SwWrtShell &rSh = rView.GetWrtShell();
    SET_CURR_SHELL( &rSh );
    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        pSdrView->SetOrtho(FALSE);

        if ( pSdrView->MouseButtonUp( rMEvt,this ) )
        {
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(FALSE);
            return; // Event von der SdrView ausgewertet
        }
    }
    //MouseButtonUp nur bearbeiten, wenn auch das Down an dieses Fenster ging.
    if ( !bMBPressed )
    {
// OS 25.02.97 Undo fuer die Giesskann ist bereits im CommandHdl
//JP 29.09.95: so sollte es sein!!!     if(pApplyTempl->bUndo)
//      if( pApplyTempl && MOUSE_RIGHT == rMEvt.GetModifier() + rMEvt.GetButtons() )
//          rSh.Do( SwWrtShell::UNDO );
        return;
    }

    Point aDocPt( PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( bDDTimerStarted )
    {
        StopDDTimer( &rSh, aDocPt );
        bMBPressed = FALSE;
        if ( rSh.IsSelFrmMode() )
        {
            (rSh.*rSh.fnEndDrag)( &aDocPt, FALSE );
            bFrmDrag = FALSE;
        }
        bNoInterrupt = FALSE;
        ReleaseMouse();
        return;
    }

    if( pAnchorMarker )
    {
        Point aPnt( pAnchorMarker->GetLastPos() );
        pSdrView->RefreshAllIAOManagers();
        DELETEZ( pAnchorMarker );
        if( aPnt.X() || aPnt.Y() )
            rSh.FindAnchorPos( aPnt, TRUE );
    }
    if ( bInsDraw && rView.GetDrawFuncPtr() )
    {
        if ( rView.GetDrawFuncPtr()->MouseButtonUp( rMEvt ) )
        {
            if (rView.GetDrawFuncPtr()) // Koennte im MouseButtonUp zerstoert worden sein
            {
                rView.GetDrawFuncPtr()->Deactivate();

                if (!rView.IsDrawMode())
                    rView.SetDrawFuncPtr(NULL);
            }

            if ( rSh.IsObjSelected() )
            {
                rSh.EnterSelFrmMode();
                if (!rView.GetDrawFuncPtr())
                    StdDrawMode(SID_OBJECT_SELECT);
            }
            else if ( rSh.IsFrmSelected() )
            {
                rSh.EnterSelFrmMode();
                StopInsFrm();
            }
            else
            {
                const Point aDocPos( PixelToLogic( aStartPos ) );
                bValidCrsrPos = !(CRSR_POSCHG & (rSh.*rSh.fnSetCrsr)(&aDocPos,FALSE));
                rSh.Edit();
            }

            rView.AttrChangedNotify( &rSh );
        }
        else if (rMEvt.GetButtons() == MOUSE_RIGHT && rSh.IsDrawCreate())
            rView.GetDrawFuncPtr()->BreakCreate();   // Zeichnen abbrechen

        bNoInterrupt = FALSE;
        ReleaseMouse();
        return;
    }
    BOOL bPopMode = FALSE;
    switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
    {
        case MOUSE_LEFT:
            if ( bInsDraw && rSh.IsDrawCreate() )
            {
                if ( rView.GetDrawFuncPtr() && rView.GetDrawFuncPtr()->MouseButtonUp(rMEvt) == TRUE )
                {
                    rView.GetDrawFuncPtr()->Deactivate();
                    rView.AttrChangedNotify( &rSh );
                    if ( rSh.IsObjSelected() )
                        rSh.EnterSelFrmMode();
                    if ( rView.GetDrawFuncPtr() && bInsFrm )
                        StopInsFrm();
                }
                bCallBase = FALSE;
                break;
            }
        case MOUSE_LEFT + KEY_MOD1:
        case MOUSE_LEFT + KEY_MOD2:
        case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            if ( bFrmDrag && rSh.IsSelFrmMode() )
            {
                if ( rMEvt.IsMod1() ) //Kopieren und nicht moven.
                {
                    //Drag abbrechen, statt dessen internes Copy verwenden
                    Rectangle aRect;
                    rSh.GetDrawView()->TakeActionRect( aRect );
                    if (!aRect.IsEmpty())
                    {
                        rSh.BreakDrag();
                        Point aEndPt, aSttPt;
                        if ( rSh.GetSelFrmType() & FRMTYPE_FLY_ATCNT )
                        {
                            aEndPt = aRect.TopLeft();
                            aSttPt = rSh.GetDrawView()->GetAllMarkedRect().TopLeft();
                        }
                        else
                        {
                            aEndPt = aRect.Center();
                            aSttPt = rSh.GetDrawView()->GetAllMarkedRect().Center();
                        }
                        if ( aSttPt != aEndPt )
                        {
                            rSh.StartUndo( UIUNDO_DRAG_AND_COPY );
                            rSh.Copy(&rSh, aSttPt, aEndPt, FALSE);
                            rSh.EndUndo( UIUNDO_DRAG_AND_COPY );
                        }
                    }
                    else
                        (rSh.*rSh.fnEndDrag)( &aDocPt,FALSE );
                }
                else
                {
                    {
                        const SwFrmFmt* pFlyFmt;
                        const SvxMacro* pMacro;

                        USHORT nEvent = HDL_MOVE == eOldSdrMoveHdl
                                            ? SW_EVENT_FRM_MOVE
                                            : SW_EVENT_FRM_RESIZE;

                        if( 0 != ( pFlyFmt = rSh.GetFlyFrmFmt() ) &&
                            0 != ( pMacro = pFlyFmt->GetMacro().GetMacroTable().
                            Get( nEvent )) )
                        {
                            const Point aSttPt( PixelToLogic( aStartPos ) );
                            aRszMvHdlPt = aDocPt;
                            USHORT nPos = 0;
                            SbxArrayRef xArgs = new SbxArray;
                            SbxVariableRef xVar = new SbxVariable;
                            xVar->PutString( pFlyFmt->GetName() );
                            xArgs->Put( &xVar, ++nPos );

                            if( SW_EVENT_FRM_RESIZE == nEvent )
                            {
                                xVar = new SbxVariable;
                                xVar->PutUShort( eOldSdrMoveHdl );
                                xArgs->Put( &xVar, ++nPos );
                            }

                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.X() - aSttPt.X() );
                            xArgs->Put( &xVar, ++nPos );
                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.Y() - aSttPt.Y() );
                            xArgs->Put( &xVar, ++nPos );

                            xVar = new SbxVariable;
                            xVar->PutUShort( 1 );
                            xArgs->Put( &xVar, ++nPos );

                            ReleaseMouse();

                            rSh.ExecMacro( *pMacro, 0, &xArgs );

                            CaptureMouse();
                        }
                    }
                    (rSh.*rSh.fnEndDrag)( &aDocPt,FALSE );
                }
                bFrmDrag = FALSE;
                bCallBase = FALSE;
                break;
            }
            bPopMode = TRUE;
            // no break
        case MOUSE_LEFT + KEY_SHIFT:
            if (rSh.IsSelFrmMode())
            {

                (rSh.*rSh.fnEndDrag)( &aDocPt, FALSE );
                bFrmDrag = FALSE;
                bCallBase = FALSE;
                break;
            }

            if( bHoldSelection )
            {
                //JP 27.04.99: Bug 65389 - das EndDrag sollte auf jedenfall
                //              gerufen werden.
                bHoldSelection = FALSE;
                (rSh.*rSh.fnEndDrag)( &aDocPt, FALSE );
            }
            else
            {
                if ( !rSh.IsInSelect() && rSh.ChgCurrPam( aDocPt ) )
                {
                    const BOOL bTmp = bNoInterrupt;
                    bNoInterrupt = FALSE;
                    {   // nur temp. Move-Kontext aufspannen, da sonst die
                        // Abfrage auf die Inhaltsform nicht funktioniert!!!
                        MV_KONTEXT( &rSh );
                        const Point aDocPos( PixelToLogic( aStartPos ) );
                        bValidCrsrPos = !(CRSR_POSCHG & (rSh.*rSh.fnSetCrsr)(&aDocPos,FALSE));
                    }
                    bNoInterrupt = bTmp;

                }
                else
                {
                    BOOL bInSel = rSh.IsInSelect();
                    (rSh.*rSh.fnEndDrag)( &aDocPt, FALSE );

                    // Internetfield? --> Link-Callen (DocLaden!!)
//JP 18.10.96: Bug 32437 -
//                  if( !rSh.HasSelection() )
                    if( !bInSel )
                    {
                        USHORT nFilter = KEY_SHIFT == rMEvt.GetModifier()
                                        ? URLLOAD_DOWNLOADFILTER
                                        : URLLOAD_NOFILTER;
                        if( KEY_MOD1 == rMEvt.GetModifier() )
                            nFilter |= URLLOAD_NEWVIEW;

                        BOOL bExecHyperlinks = rSh.GetViewOptions()->IsExecHyperlinks()^
                                               (rMEvt.GetModifier() == KEY_MOD2 ? TRUE : FALSE);
                        if(pApplyTempl)
                            bExecHyperlinks = FALSE;
                        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_CLICKFIELD |
                                                    SwContentAtPos::SW_INETATTR );

                        if( rSh.GetContentAtPos( aDocPt, aCntntAtPos, TRUE ) )
                        {
                            BOOL bViewLocked = rSh.IsViewLocked();
                            if( !bViewLocked && !rSh.IsReadOnlyAvailable() &&
                                aCntntAtPos.IsInProtectSect() )
                                rSh.LockView( TRUE );

                            ReleaseMouse();
                            if( SwContentAtPos::SW_FIELD == aCntntAtPos.eCntntAtPos )
                            {
                                if( bExecHyperlinks )
                                    rSh.ClickToField( *aCntntAtPos.aFnd.pFld, nFilter );
                            }
                            else if( bExecHyperlinks )
                            {
                                rSh.ClickToINetAttr( *(SwFmtINetFmt*)
                                        aCntntAtPos.aFnd.pAttr, nFilter );
                            }
                            rSh.LockView( bViewLocked );
                            bCallShadowCrsr = FALSE;
                        }
                        else if( bExecHyperlinks )
                        {
                            aCntntAtPos = SwContentAtPos( SwContentAtPos::SW_FTN );
                            if( !rSh.GetContentAtPos( aDocPt, aCntntAtPos, TRUE ) )
                            {
                                SdrViewEvent aVEvt;

                                if (pSdrView)
                                    pSdrView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                                if (pSdrView && aVEvt.eEvent == SDREVENT_EXECUTEURL)
                                {
                                    // URL-Feld getroffen
                                    const SvxURLField *pField = aVEvt.pURLField;
                                    if (pField)
                                    {
                                        String sURL(pField->GetURL());
                                        String sTarget(pField->GetTargetFrame());
                                        ::LoadURL( sURL, &rSh, nFilter, &sTarget);
                                    }
                                    bCallShadowCrsr = FALSE;
                                }
                                else
                                {
                                    // Grafik getroffen
                                    ReleaseMouse();
                                    if( rSh.ClickToINetGrf( aDocPt, nFilter ))
                                        bCallShadowCrsr = FALSE;
                                }
                            }
                        }

                        if( bCallShadowCrsr &&
                            rSh.GetViewOptions()->IsShadowCursor() &&
                            MOUSE_LEFT == (rMEvt.GetModifier() + rMEvt.GetButtons()) &&
                            !rSh.HasSelection() &&
                            !GetConnectMetaFile() &&
                            rSh.VisArea().IsInside( aDocPt ))
                        {
                            if( UNDO_INS_FROM_SHADOWCRSR == rSh.GetUndoIds() )
                                rSh.Undo();
                            SwFillMode eMode = (SwFillMode)rSh.GetViewOptions()->GetShdwCrsrFillMode();
                            rSh.SetShadowCrsrPos( aDocPt, eMode );
                        }
                    }
                }
                bCallBase = FALSE;

            }

            // gfs. im Down gepushten Mode wieder zuruecksetzen
            if ( bPopMode && bModePushed )
            {
                rSh.PopMode();
                bModePushed = FALSE;
                bCallBase = FALSE;
            }
            break;

        default:
            ReleaseMouse();
            return;
    }

    if( pApplyTempl )
    {
        int eSelection = rSh.GetSelectionType();
        if( pApplyTempl->nColor )
        {
            USHORT nId = 0;
            switch( pApplyTempl->nColor )
            {
                case SID_ATTR_CHAR_COLOR_EXT:
                    nId = RES_CHRATR_COLOR;
                break;
                case SID_ATTR_CHAR_COLOR_BACKGROUND_EXT:
                    nId = RES_CHRATR_BACKGROUND;
                break;
            }
            if( nId && (SwWrtShell::SEL_TXT|SwWrtShell::SEL_TBL) & eSelection)
            {
                if( rSh.IsSelection() && !rSh.HasReadonlySel() )
                {
                    if(nId == RES_CHRATR_BACKGROUND)
                    {
                        Color aColor( COL_TRANSPARENT  );
                        if( !SwEditWin::bTransparentBackColor )
                            aColor = SwEditWin::aTextBackColor;
                        rSh.SetAttr( SvxBrushItem( aColor, nId ) );
                    }
                    else
                        rSh.SetAttr( SvxColorItem(SwEditWin::aTextColor, nId) );
                    rSh.UnSetVisCrsr();
                    rSh.EnterStdMode();
                    rSh.SetVisCrsr(aDocPt);

                    pApplyTempl->bUndo = TRUE;
                    bCallBase = FALSE;
                    aTemplateTimer.Stop();
                }
                else if(rMEvt.GetClicks() == 1)
                {
                    // keine Selektion -> also Giesskanne abschalten
                    aTemplateTimer.Start();
                }
            }
        }
        else
        {
            switch ( pApplyTempl->eType )
            {
                case SFX_STYLE_FAMILY_PARA:
                    if( (( SwWrtShell::SEL_TXT | SwWrtShell::SEL_TBL )
                         & eSelection ) && !rSh.HasReadonlySel() )
                    {
                        rSh.SetTxtFmtColl( pApplyTempl->aColl.pTxtColl );
                        pApplyTempl->bUndo = TRUE;
                        bCallBase = FALSE;
                    }
                    break;
                case SFX_STYLE_FAMILY_CHAR:
                    if( (( SwWrtShell::SEL_TXT | SwWrtShell::SEL_TBL )
                         & eSelection ) && !rSh.HasReadonlySel() )
                    {
                        rSh.SetAttr( SwFmtCharFmt(pApplyTempl->aColl.pCharFmt) );
                        rSh.UnSetVisCrsr();
                        rSh.EnterStdMode();
                        rSh.SetVisCrsr(aDocPt);
                        pApplyTempl->bUndo = TRUE;
                        bCallBase = FALSE;
                    }
                    break;
                case SFX_STYLE_FAMILY_FRAME :
                {
                    const SwFrmFmt* pFmt = rSh.GetFmtFromObj( aDocPt );
                    if(PTR_CAST(SwFlyFrmFmt, pFmt))
                    {
                        rSh.SetFrmFmt( pApplyTempl->aColl.pFrmFmt, FALSE, &aDocPt );
                        pApplyTempl->bUndo = TRUE;
                        bCallBase = FALSE;
                    }
                    break;
                }
                case SFX_STYLE_FAMILY_PAGE:
                            // Kein Undo bei Seitenvorlagen
                    rSh.ChgCurPageDesc( *pApplyTempl->aColl.pPageDesc );
                    bCallBase = FALSE;
                    break;
                case SFX_STYLE_FAMILY_PSEUDO:
                    if( !rSh.HasReadonlySel() )
                    {
                        rSh.SetCurNumRule( *pApplyTempl->aColl.pNumRule );
                        bCallBase = FALSE;
                        pApplyTempl->bUndo = TRUE;
                    }
                    break;
            }
        }

    }
    ReleaseMouse();
    // Hier kommen nur verarbeitete MouseEvents an; nur bei diesen duerfen
    // die Modi zurueckgesetzt werden.
    bMBPressed = FALSE;

    //sicherheitshalber aufrufen, da jetzt das Selektieren bestimmt zu Ende ist.
    //Andernfalls koennte der Timeout des Timers Kummer machen.
    EnterArea();
    bNoInterrupt = FALSE;

    if (bCallBase)
        Window::MouseButtonUp(rMEvt);
}


/*--------------------------------------------------------------------
    Beschreibung:   Vorlage anwenden
 --------------------------------------------------------------------*/


void SwEditWin::SetApplyTemplate(const SwApplyTemplate &rTempl)
{
    static BOOL bIdle = FALSE;
    DELETEZ(pApplyTempl);
    SwWrtShell &rSh = rView.GetWrtShell();

    if(rTempl.nColor)
    {
        pApplyTempl = new SwApplyTemplate( rTempl );
        SetPointer( POINTER_FILL );
        rSh.NoEdit( FALSE );
        bIdle = rSh.GetViewOptions()->IsIdle();
        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( FALSE );
    }
    else if( rTempl.eType )
    {
        pApplyTempl = new SwApplyTemplate( rTempl );
        SetPointer( POINTER_FILL  );
        rSh.NoEdit( FALSE );
        bIdle = rSh.GetViewOptions()->IsIdle();
        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( FALSE );
    }
    else
    {
        SetPointer( POINTER_TEXT );
        rSh.UnSetVisCrsr();

        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( bIdle );
        if ( !rSh.IsSelFrmMode() )
            rSh.Edit();
    }

    static USHORT __READONLY_DATA aInva[] =
    {
        SID_STYLE_WATERCAN,
        SID_ATTR_CHAR_COLOR_EXT,
        SID_ATTR_CHAR_COLOR_BACKGROUND_EXT,
        0
    };
    rView.GetViewFrame()->GetBindings().Invalidate(aInva);
}

/*--------------------------------------------------------------------
    Beschreibung:   ctor
 --------------------------------------------------------------------*/


SwEditWin::SwEditWin(Window *pParent, SwView &rMyView):
    Window(pParent, WinBits(WB_CLIPCHILDREN | WB_DIALOGCONTROL)),
    aActHitType(SDRHIT_NONE),
    eDrawMode(OBJ_NONE),
    pApplyTempl(0),
    rView( rMyView ),
    pAnchorMarker( 0 ),
    pUserMarker( 0 ),
    pUserMarkerObj( 0 ),
    pShadCrsr( 0 ),
    nDropAction( 0 ),
    nDropFormat( 0 ),
    nDropDestination( 0 ),
    nInsFrmColCount( 1 ),
    bLockInput(FALSE)
{
    SetHelpId(HID_EDIT_WIN);
    EnableChildTransparentMode();
    SetDialogControlFlags( WINDOW_DLGCTRL_RETURN | WINDOW_DLGCTRL_WANTFOCUS );

    bLinkRemoved = bMBPressed = bInsDraw = bInsFrm =
    bIsInDrag = bOldIdle = bOldIdleSet = bChainMode = bWasShdwCrsr = FALSE;

    SetMapMode(MapMode(MAP_TWIP));

    SetPointer( POINTER_TEXT );
    aTimer.SetTimeoutHdl(LINK(this, SwEditWin, TimerHandler));
    EnableDrop();

    bTblInsDelMode = FALSE;
    aKeyInputTimer.SetTimeout( 3000 );
    aKeyInputTimer.SetTimeoutHdl(LINK(this, SwEditWin, KeyInputTimerHandler));

    aKeyInputFlushTimer.SetTimeout( 200 );
    aKeyInputFlushTimer.SetTimeoutHdl(LINK(this, SwEditWin, KeyInputFlushHandler));

    // TemplatePointer fuer Farben soll nach Einfachclick
    // ohne Selektion zurueckgesetzt werden
    aTemplateTimer.SetTimeout(400);
    aTemplateTimer.SetTimeoutHdl(LINK(this, SwEditWin, TemplateTimerHdl));

    //JP 16.12.98: temporaere Loesung!!! Sollte bei jeder Cursorbewegung
    //          den Font von der akt. einfuege Position setzen!
    if( !rMyView.GetDocShell()->IsReadOnly() )
    {
        Font aFont;
        SetInputContext( InputContext( aFont, INPUTCONTEXT_TEXT |
                                            INPUTCONTEXT_EXTTEXTINPUT ) );
    }
}



SwEditWin::~SwEditWin()
{
    aKeyInputTimer.Stop();
    delete pShadCrsr;
    if( pQuickHlpData->bClear && rView.GetWrtShellPtr() )
        pQuickHlpData->Stop( rView.GetWrtShell() );
    bExecuteDrag = FALSE;
    delete pApplyTempl;
    rView.SetDrawFuncPtr(NULL);
    delete pUserMarker;
    delete pAnchorMarker;
}


/******************************************************************************
 *  Beschreibung: DrawTextEditMode einschalten
 ******************************************************************************/


void SwEditWin::EnterDrawTextMode( const Point& aDocPos )
{
    if ( rView.EnterDrawTextMode(aDocPos) == TRUE )
    {
        if (rView.GetDrawFuncPtr())
        {
            rView.GetDrawFuncPtr()->Deactivate();
            rView.SetDrawFuncPtr(NULL);
            rView.LeaveDrawCreate();
        }
        rView.NoRotate();
        rView.AttrChangedNotify( &rView.GetWrtShell() );
    }
}

/******************************************************************************
 *  Beschreibung: DrawMode einschalten
 ******************************************************************************/



BOOL SwEditWin::EnterDrawMode(const MouseEvent& rMEvt, const Point& aDocPos)
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SdrView *pSdrView = rSh.GetDrawView();

//  if ( GetDrawFuncPtr() && (aActHitType == SDRHIT_NONE || rSh.IsDrawCreate()) )
    if ( rView.GetDrawFuncPtr() )
    {
        if (rSh.IsDrawCreate())
            return TRUE;

        BOOL bRet = rView.GetDrawFuncPtr()->MouseButtonDown( rMEvt );
        rView.AttrChangedNotify( &rSh );
        return bRet;
    }

    if ( pSdrView && pSdrView->IsTextEdit() )
    {
        rSh.EndTextEdit(); // Danebengeklickt, Ende mit Edit
        rSh.SelectObj( aDocPos );
        if ( !rSh.IsObjSelected() && !rSh.IsFrmSelected() )
            rSh.LeaveSelFrmMode();
        else
        {
            SwEditWin::nDDStartPosY = aDocPos.Y();
            SwEditWin::nDDStartPosX = aDocPos.X();
            bFrmDrag = TRUE;
        }
        rView.AttrChangedNotify( &rSh );
        return TRUE;
    }
    return FALSE;
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/



BOOL SwEditWin::IsDrawSelMode()
{
//  return (IsFrmAction() == FALSE && GetDrawMode() == OBJ_NONE);
    return (GetDrawMode() == SID_OBJECT_SELECT);
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/


void SwEditWin::GetFocus()
{
    rView.GotFocus();
    Window::GetFocus();
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/



void SwEditWin::LoseFocus()
{
    Window::LoseFocus();
    if( pQuickHlpData->bClear )
        pQuickHlpData->Stop( rView.GetWrtShell() );
    rView.LostFocus();
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/



void SwEditWin::Command( const CommandEvent& rCEvt )
{
    if ( !rView.GetViewFrame() || !rView.GetViewFrame()->GetFrame() ||
          rView.GetViewFrame()->GetFrame()->TransferForReplaceInProgress())
    {
        //Wenn der ViewFrame in Kuerze stirbt kein Popup mehr!
        Window::Command(rCEvt);
        return;
    }

    BOOL bCallBase      = TRUE;

    SwWrtShell &rSh = rView.GetWrtShell();

    switch ( rCEvt.GetCommand() )
    {
        case COMMAND_STARTDRAG:
        {
            if (rSh.GetDrawView() && rSh.GetDrawView()->Command(rCEvt, this))
            {
                rView.GetViewFrame()->GetBindings().InvalidateAll(FALSE);
                return; // Event von der SdrView ausgewertet
            }
            if ( !pApplyTempl && !rSh.IsDrawCreate() && !IsDrawAction())
            {
                BOOL bStart = FALSE, bDelSelect = FALSE;
                SdrObject *pObj = NULL;
                Point aDocPos( PixelToLogic( rCEvt.GetMousePosPixel() ) );
                if ( !rSh.IsInSelect() && rSh.ChgCurrPam( aDocPos, TRUE, TRUE))
                    //Wir sind nicht beim Selektieren und stehen auf einer
                    //Selektion
                    bStart = TRUE;
                else if ( !bFrmDrag && rSh.IsSelFrmMode() &&
                          rSh.IsInsideSelectedObj( aDocPos ) )
                    //Wir sind nicht am internen Draggen und stehen auf
                    //einem Objekt (Rahmen, Zeichenobjekt)
                    bStart = TRUE;
                else if( !bFrmDrag && rView.GetDocShell()->IsReadOnly() &&
                        OBJCNT_NONE != rSh.GetObjCntType( aDocPos, pObj ))
                {
                    rSh.LockPaint();
                    if( rSh.SelectObj( aDocPos, FALSE, FALSE, pObj ))
                        bStart = bDelSelect = TRUE;
                    else
                        rSh.UnlockPaint();
                }
                else
                {
                    SwContentAtPos aSwContentAtPos( SwContentAtPos::SW_INETATTR );
                    bStart = rSh.GetContentAtPos( aDocPos,
                                aSwContentAtPos,
                                FALSE );
                }

                if ( bStart && !bIsInDrag )
                {
                    bMBPressed = FALSE;
                    ReleaseMouse();
                    bFrmDrag = FALSE;
                    bExecuteDrag = TRUE;
                    SwEditWin::nDDStartPosY = aDocPos.Y();
                    SwEditWin::nDDStartPosX = aDocPos.X();
                    aMovePos = aDocPos;
                    StartExecuteDrag();
                    bCallBase = FALSE;
                    if( bDelSelect )
                    {
                        rSh.UnSelectFrm();
                        rSh.UnlockPaint();
                    }
                }
            }
        }
        break;

        case COMMAND_CONTEXTMENU:
        {
            const USHORT nId = SwInputChild::GetChildWindowId();
            SwInputChild* pChildWin = (SwInputChild*)GetView().GetViewFrame()->
                                                GetChildWindow( nId );

            if((!pChildWin || pChildWin->GetView() != &rView) &&
                !rSh.IsDrawCreate() && !IsDrawAction())
            {
                SET_CURR_SHELL( &rSh );
                if (!pApplyTempl)
                {
                    if (bNoInterrupt == TRUE)
                    {
                        ReleaseMouse();
                        bNoInterrupt = FALSE;
                        bMBPressed = FALSE;
                    }
                    Point aDocPos( PixelToLogic( rCEvt.GetMousePosPixel() ) );
                    if ( !rCEvt.IsMouseEvent() )
                        aDocPos = rSh.GetCharRect().Center();
                    const Point aPixPos = LogicToPixel( aDocPos );

                    if ( rView.GetDocShell()->IsReadOnly() )
                        SwReadOnlyPopup( aDocPos, rView ).Execute( this, aPixPos );
                    else if ( !rView.ExecSpellPopup( aDocPos ) )
                        GetView().GetViewFrame()->GetDispatcher()->ExecutePopup( 0, this, &aPixPos);
                }
                else if (pApplyTempl->bUndo)
                    rSh.Do(SwWrtShell::UNDO);
                bCallBase = FALSE;
            }
        }
        break;

        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
            if( pShadCrsr )
                delete pShadCrsr, pShadCrsr = 0;
            bCallBase = !rView.HandleWheelCommands( rCEvt );
            break;

        case COMMAND_VOICE:
            {
                //ggf. an Outliner weiterleiten
                if ( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
                {
                    bCallBase = FALSE;
                    rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
                    break;
                }

                const CommandVoiceData *pCData = rCEvt.GetVoiceData();
                if ( VOICECOMMANDTYPE_CONTROL == pCData->GetType() )
                    break;


                USHORT nSlotId = 0;
                SfxPoolItem *pItem = 0;

                switch ( pCData->GetCommand() )
                {
                    case DICTATIONCOMMAND_NEWPARAGRAPH: nSlotId = FN_INSERT_BREAK; break;
                    case DICTATIONCOMMAND_NEWLINE:      nSlotId = FN_INSERT_LINEBREAK; break;
                    case DICTATIONCOMMAND_LEFT:         nSlotId = FN_PREV_WORD; break;
                    case DICTATIONCOMMAND_RIGHT:        nSlotId = FN_NEXT_WORD; break;
                    case DICTATIONCOMMAND_UP:           nSlotId = FN_LINE_UP; break;
                    case DICTATIONCOMMAND_DOWN:         nSlotId = FN_LINE_DOWN; break;
                    case DICTATIONCOMMAND_UNDO:         nSlotId = SID_UNDO; break;
                    case DICTATIONCOMMAND_REPEAT:       nSlotId = SID_REPEAT; break;
                    case DICTATIONCOMMAND_DEL:          nSlotId = FN_DELETE_BACK_WORD; break;

                    case DICTATIONCOMMAND_BOLD_ON:      nSlotId = SID_ATTR_CHAR_WEIGHT;
                                                        pItem = new SvxWeightItem( WEIGHT_BOLD );
                                                        break;
                    case DICTATIONCOMMAND_BOLD_OFF:     nSlotId = SID_ATTR_CHAR_WEIGHT;
                                                        pItem = new SvxWeightItem( WEIGHT_NORMAL );
                                                        break;
                    case DICTATIONCOMMAND_UNDERLINE_ON: nSlotId = SID_ATTR_CHAR_UNDERLINE;
                                                        pItem = new SvxUnderlineItem( UNDERLINE_SINGLE );
                                                        break;
                    case DICTATIONCOMMAND_UNDERLINE_OFF:nSlotId = SID_ATTR_CHAR_UNDERLINE;
                                                        pItem = new SvxUnderlineItem( UNDERLINE_NONE );
                                                        break;
                    case DICTATIONCOMMAND_ITALIC_ON:    nSlotId = SID_ATTR_CHAR_POSTURE;
                                                        pItem = new SvxPostureItem( ITALIC_NORMAL );
                                                        break;
                    case DICTATIONCOMMAND_ITALIC_OFF:   nSlotId = SID_ATTR_CHAR_POSTURE;
                                                        pItem = new SvxPostureItem( ITALIC_NONE );
                                                        break;
                    case DICTATIONCOMMAND_NUMBERING_ON:
                                    if ( !rSh.GetCurNumRule() )
                                        nSlotId = FN_NUM_NUMBERING_ON;
                                    break;
                    case DICTATIONCOMMAND_NUMBERING_OFF:
                                    if ( rSh.GetCurNumRule() )
                                       nSlotId = FN_NUM_NUMBERING_ON;
                                    break;
                    case DICTATIONCOMMAND_TAB:
                                    {
                                       rSh.Insert( '\t' );
                                    }
                                    break;
                    case DICTATIONCOMMAND_UNKNOWN:
                                    {
                                        rView.GetWrtShell().Insert( pCData->GetText() );
                                    }
                                    break;

#ifdef DBG_UTIL
                    default:
                        ASSERT( !this, "unknown speech command." );
#endif
                }
                if ( nSlotId )
                {
                    bCallBase = FALSE;
                    if ( pItem )
                    {
                        //Eigentumsuebergang des Items
                        GetView().GetViewFrame()->GetDispatcher()->Execute( nSlotId, SFX_CALLMODE_STANDARD,
                                                  pItem, 0 );
                    }
                    else
                        GetView().GetViewFrame()->GetDispatcher()->Execute( nSlotId );
                }
            }
            break;

    case COMMAND_STARTEXTTEXTINPUT:
        if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
        {
            bCallBase = FALSE;
            rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
        }
        else
        {
            if( rSh.HasSelection() )
                rSh.DelRight();

            bCallBase = FALSE;
            rSh.CreateExtTextInput();
        }
        break;

    case COMMAND_ENDEXTTEXTINPUT:
        if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
        {
            bCallBase = FALSE;
            rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
        }
        else
        {
            bCallBase = FALSE;
            rSh.DeleteExtTextInput();
        }
        break;

    case COMMAND_EXTTEXTINPUT:
        if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
        {
            bCallBase = FALSE;
            rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
        }
        else
        {
            const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();
            if( pData )
            {
                bCallBase = FALSE;
                rSh.SetExtTextInputData( *pData );
            }
        }
        break;

    case COMMAND_CURSORPOS:
        // will be handled by the base class
        break;

    case COMMAND_EXTTEXTINPUTPOS:
        if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
        {
            bCallBase = FALSE;
            rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
        }
        else
        {
            SwExtTextInput* pInput = rSh.GetExtTextInput();
            const CommandExtTextInputPosData* pData = rCEvt.GetExtTextInputPosData();
            if( pInput && pData )
            {
                const Point aPt( rSh.GetCharRect().Pos() );
                Rectangle* pRects = pInput->GetPosInputData( *pData, &aPt );
                SetExtTextInputPos( pData->GetFirstPos(),
                                    pData->GetChars(), pRects );
                delete pRects;
                bCallBase = FALSE;
            }
        }
        break;

#ifdef DBG_UTIL
        default:
            ASSERT( !this, "unknown command." );
#endif
    }
    if (bCallBase)
        Window::Command(rCEvt);
}



SfxShell* lcl_GetShellFromDispatcher( SwView& rView, TypeId nType )
{
    //Shell ermitteln
    SfxShell* pShell;
    SfxDispatcher* pDispatcher = rView.GetViewFrame()->GetDispatcher();
    for(int  i = 0; TRUE; ++i )
    {
        pShell = pDispatcher->GetShell( i );
        if( !pShell || pShell->IsA( nType ) )
            break;
    }
    return pShell;
}



void SwEditWin::ClearTip()
{
}

IMPL_LINK( SwEditWin, KeyInputFlushHandler, Timer *, EMPTYARG )
{
    FlushInBuffer( &rView.GetWrtShell() );
    return 0;
}


IMPL_LINK( SwEditWin, KeyInputTimerHandler, Timer *, EMPTYARG )
{
    bTblInsDelMode = FALSE;
    return 0;
}

void SwEditWin::_InitStaticData()
{
    pQuickHlpData = new QuickHelpData();
}

void SwEditWin::_FinitStaticData()
{
    delete pQuickHlpData;
}

/*-----------------23.02.97 18:39-------------------

--------------------------------------------------*/


IMPL_LINK(SwEditWin, TemplateTimerHdl, Timer*, EMPTYARG)
{
    SetApplyTemplate(SwApplyTemplate());
    return 0;
}


void SwEditWin::SetChainMode( BOOL bOn )
{
    if ( !bChainMode )
        StopInsFrm();
    if ( pUserMarker )
    {
        DELETEZ( pUserMarker );
    }
    bChainMode = bOn;
    if ( !bChainMode )
        rView.GetViewFrame()->HideStatusText();

    static USHORT __READONLY_DATA aInva[] =
    {
        FN_FRAME_CHAIN, FN_FRAME_UNCHAIN, 0
    };
    rView.GetViewFrame()->GetBindings().Invalidate(aInva);
}

//-------------------------------------------------------------

void QuickHelpData::Move( QuickHelpData& rCpy )
{
    // Pointer verschieben
    aArr.Insert( &rCpy.aArr );
    rCpy.aArr.Remove( (USHORT)0, rCpy.aArr.Count() );
//  nTipId = rCpy.nTipId;
    bClear = rCpy.bClear;
    nLen = rCpy.nLen;
    nCurArrPos = rCpy.nCurArrPos;
    bChkInsBlank = rCpy.bChkInsBlank;
    bIsTip = rCpy.bIsTip;
    bIsAutoText = rCpy.bIsAutoText;

    if( pCETID ) delete pCETID;
    pCETID = rCpy.pCETID;
    rCpy.pCETID = 0;

    if( pAttrs ) delete pAttrs;
    pAttrs = rCpy.pAttrs;
    rCpy.pAttrs = 0;
}

void QuickHelpData::ClearCntnt()
{
    nLen = nCurArrPos = 0;
    bClear = bChkInsBlank = FALSE;
    nTipId = 0;
    aArr.DeleteAndDestroy( 0 , aArr.Count() );
    bIsTip = TRUE;
    bIsAutoText = TRUE;
    delete pCETID, pCETID = 0;
    delete pAttrs, pAttrs = 0;
}


void QuickHelpData::Start( SwWrtShell& rSh, USHORT nWrdLen )
{
    if( pCETID ) delete pCETID, pCETID = 0;
    if( pAttrs ) delete pAttrs, pAttrs = 0;

    if( USHRT_MAX != nWrdLen )
    {
        nLen = nWrdLen;
        nCurArrPos = 0;
    }
    bClear = TRUE;

    if( bIsTip )
    {
        Window& rWin = rSh.GetView().GetEditWin();
        Point aPt( rWin.OutputToScreenPixel( rWin.LogicToPixel(
                    rSh.GetCharRect().Pos() )));
        aPt.Y() -= 3;
        nTipId = Help::ShowTip( &rWin, Rectangle( aPt, Size( 1, 1 )),
                        *aArr[ nCurArrPos ],
                        QUICKHELP_LEFT | QUICKHELP_BOTTOM );
    }
    else
    {
        String sStr( *aArr[ nCurArrPos ] );
        sStr.Erase( 0, nLen );
        USHORT nL = sStr.Len();
        pAttrs = new USHORT[ nL ];
        for( USHORT n = nL; n;  )
            *(pAttrs + --n ) = EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE |
                                EXTTEXTINPUT_ATTR_HIGHLIGHT;
        pCETID = new CommandExtTextInputData( sStr, pAttrs, nL,
                                                TRUE, 0, 0, FALSE );
        rSh.CreateExtTextInput();
        rSh.SetExtTextInputData( *pCETID );
    }
}

void QuickHelpData::Stop( SwWrtShell& rSh )
{
    if( !bIsTip )
        rSh.DeleteExtTextInput( 0, FALSE );
    else if( nTipId )
        Help::HideTip( nTipId );
    ClearCntnt();
}


/***********************************************************************

        $Log: not supported by cvs2svn $
        Revision 1.2  2000/10/05 12:13:21  jp
        should change: remove image

        Revision 1.1.1.1  2000/09/18 17:14:35  hr
        initial import

        Revision 1.776  2000/09/18 16:05:23  willem.vandorp
        OpenOffice header added.

        Revision 1.775  2000/09/08 08:12:50  os
        Change: Set/Toggle/Has/Knows/Show/GetChildWindow

        Revision 1.774  2000/09/07 16:06:14  os
        change: SFX_DISPATCHER/SFX_BINDINGS removed

        Revision 1.773  2000/09/07 15:59:21  os
        change: SFX_DISPATCHER/SFX_BINDINGS removed

        Revision 1.772  2000/07/18 18:30:46  jp
        KeyInput: start a timer if AnyInput don't call the KeyInput again, for flush the data

        Revision 1.771  2000/06/28 09:40:29  os
        #76471# call FN_EDIT_FIELD via SfxBindings instead of SfxDispatcher

        Revision 1.770  2000/06/13 14:56:20  os
        #76167# character background color assignment corrected

        Revision 1.769  2000/06/13 10:00:15  os
        using UCB

        Revision 1.768  2000/06/09 13:35:02  ama
        Fix: No assert for COMMAND_CURSORPOS

        Revision 1.767  2000/05/26 07:21:29  os
        old SW Basic API Slots removed

        Revision 1.766  2000/05/24 10:39:15  jp
        Fix for solaris compiler

        Revision 1.765  2000/05/19 13:12:42  jp
        Changes for Unicode

        Revision 1.764  2000/05/19 11:04:59  jp
        Changes for Unicode

        Revision 1.763  2000/05/10 11:54:57  os
        Basic API removed

        Revision 1.762  2000/05/09 14:46:33  os
        BASIC interface partially removed

        Revision 1.761  2000/05/03 09:04:35  os
        #75368# prevent hyperlink execution if watercan mode is activated

        Revision 1.760  2000/04/20 12:55:18  os
        GetName() returns String&

        Revision 1.759  2000/04/18 15:18:16  os
        UNICODE

        Revision 1.758  2000/03/03 15:16:59  os
        StarView remainders removed

        Revision 1.757  2000/02/11 14:44:59  hr
        #70473# changes for unicode ( patched by automated patchtool )

        Revision 1.756  2000/02/09 12:02:37  jp
        Task #72684#: AutoText expand only with return-Key

        Revision 1.755  2000/01/13 21:28:12  jp
        Task #71894#: new Options for SW-AutoComplete

        Revision 1.754  1999/12/06 18:19:55  jp
        Bug #70535#: Wheelcommands - hide ShadowCursor

        Revision 1.753  1999/11/10 09:38:34  ama
        Fix #69089#: HitHandle and hit anchor

        Revision 1.752  1999/10/19 12:24:44  os
        call authority-entry dialog on double click

        Revision 1.751  1999/10/12 14:33:19  jp
        Bug #69139#: DTOR - stop QuickHelpData only if the WrtShell exist

        Revision 1.750  1999/10/12 13:39:17  ama
        Fix #69089#: Use sdr-anchor-handle

        Revision 1.749  1999/10/11 20:10:29  jp
        Bug #68496#: append paragraph behind a section

        Revision 1.748  1999/10/08 09:42:58  os
        #69059# last cast from GetpApp to SfxApplication* removed

        Revision 1.747  1999/10/06 08:41:11  jp
        Bug #68761#: MouseMove - use new function GetFmtFromAnyObj

        Revision 1.746  1999/08/27 09:34:38  JP
        no cast from GetpApp to SfxApp


      Rev 1.745   27 Aug 1999 11:34:38   JP
   no cast from GetpApp to SfxApp

      Rev 1.744   06 Aug 1999 16:48:18   JP
   Bug #68113#: MouseButtonDown - lockview before call SelectObject

      Rev 1.743   06 Jul 1999 20:06:50   JP
   Bug #67360#: MouseMove - check for aditional mode

      Rev 1.742   17 Jun 1999 09:36:30   JP
   Bug #66927#: KeyInput - fill AutoCompleteList only with words which has lower count of characters

      Rev 1.741   27 May 1999 13:00:40   OS
   Einf?gen/Bearbeiten von Spalten ueberarbeitet

      Rev 1.740   11 May 1999 20:09:22   HJS
   includes

      Rev 1.739   27 Apr 1999 14:50:20   JP
   Bug #65389#: MouseButtonUp - im HoldSelection auf jedenfall das EndDrag rufen

      Rev 1.738   26 Apr 1999 12:20:14   KZ
   #include <svtools/args.hxx> eingefuegt

      Rev 1.737   23 Apr 1999 17:02:32   JP
   Bug #65289#: MouseMove: synthetische Moves/ButtonDowns ignorieren

      Rev 1.736   22 Apr 1999 12:33:40   JP
   Bug #65171#: UpdatePointer - bei ApplyStyle den richtigen MousePointer setzen

      Rev 1.735   19 Apr 1999 13:38:44   OS
   #64780# Notizen und Scripts in geschuetzten Bereichen

      Rev 1.734   01 Apr 1999 15:26:38   JP
   Bug #64154#: KeyInput - beim AutoComplete die schon eingegebenen Buchstaben nicht entfernen

      Rev 1.733   17 Mar 1999 11:31:40   JP
   Task #63576#: KeyInput - das AutoComplete-Flag von der OffApp erfragen

      Rev 1.732   10 Mar 1999 09:40:30   JP
   Task #61405#: AutoCompletion von Woertern

      Rev 1.731   09 Mar 1999 19:38:14   JP
   Task #61405#: AutoCompletion von Woertern

      Rev 1.730   08 Mar 1999 11:52:14   JP
   Bug #62920#: aus readonly Docs Grafiken einfacher herausdraggen

      Rev 1.729   01 Mar 1999 23:01:24   JP
   Bug #62582#: InInProtectCntnt nur noch auswerten, wenn Cursor nicht im Readonly zugelassen ist

      Rev 1.728   03 Feb 1999 13:59:58   JP
   Bug #61399#: MouseButtonDown - CTRL auf Handels behandeln

      Rev 1.727   28 Jan 1999 20:46:08   JP
   Bug #61264#: MouseButtonDown - SHIFT-CTRL auf Handels behandeln

      Rev 1.726   26 Jan 1999 11:01:24   AWO
   Syntax

      Rev 1.725   22 Jan 1999 16:56:44   JP
   Bug Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.724   20 Jan 1999 14:17:30   JP
   Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.723   19 Jan 1999 22:58:12   JP
   Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.722   17 Dec 1998 19:19:54   JP
   Task #59490# ExtTextInput fuer japan./chine. Version

      Rev 1.721   11 Dec 1998 12:47:42   MT
   #59490# IME

      Rev 1.720   11 Nov 1998 15:04:26   JP
   Task #59308#: NoNum auch bei Outlines setzen

      Rev 1.719   06 Nov 1998 14:43:04   OS
   #57903# NumOff kann weg

      Rev 1.718   03 Nov 1998 11:51:40   JP
   Task #57916#: Vorbereitungen fuer den Selektionshandler vom Chart

      Rev 1.717   25 Sep 1998 13:21:00   JP
   Bug #56196#: KeyInput - Tab in der WebView ans Window weiterleiten, damit der SFX ggfs. was tun kann

      Rev 1.716   24 Sep 1998 13:33:22   JP
   Bug #55592#/#55931#: synthetische MouseMoves mit Ctrl-Taste sollten keine Selektion anfangen

      Rev 1.715   10 Sep 1998 10:39:28   OS
   #56290# Rahmenvorlage per Giesskanne auch ohne Selektion

**********************************************************************/

