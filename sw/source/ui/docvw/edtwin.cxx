/*************************************************************************
 *
 *  $RCSfile: edtwin.cxx,v $
 *
 *  $Revision: 1.72 $
 *
 *  last change: $Author: hjs $ $Date: 2003-09-25 10:50:38 $
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


#pragma hdrstop

#include <tools/list.hxx>

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifdef ACCESSIBLE_LAYOUT
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
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
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
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
#ifndef _SFXPTITEM_HXX
#include <svtools/ptitem.hxx>
#endif
#define ITEMID_SIZE SID_ATTR_SIZE
#ifndef _SVX_SIZEITEM_HXX
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
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
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
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
#ifndef _SWCALWRP_HXX
#include <swcalwrp.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
#ifndef _SWWDOCSH_HXX //autogen
#include <wdocsh.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
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
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif

#include <charfmt.hxx>
#include <numrule.hxx>
#include <pagedesc.hxx>

//JP 11.10.2001: enable test code for bug fix 91313
#if !defined( PRODUCT ) && (OSL_DEBUG_LEVEL > 1)
//#define TEST_FOR_BUG91313
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
    Point aHdlPos;
    Point aLastPos;
public:
    SwAnchorMarker( SdrHdl* pH ) :
        pHdl( pH ), aHdlPos( pH->GetPos() ), aLastPos( pH->GetPos() ) {}
    const Point& GetLastPos() const { return aLastPos; }
    void SetLastPos( const Point& rNew ) { aLastPos = rNew; }
    void SetPos( const Point& rNew ) { pHdl->SetPos( rNew ); }
    const Point& GetPos() { return pHdl->GetPos(); }
    const Point& GetHdlPos() { return aHdlPos; }
    void ChgHdl( SdrHdl* pNew ) { pHdl = pNew; }
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
    void FillStrArr( SwWrtShell& rSh, const String& rWord );
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
                // dvo: IsObjSelectable() eventually calls SdrView::PickObj, so
                // apparently this is used to determine whether this is a
                // drawling layer object or not.
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
                            // if we're over a selected object, we show an
                            // ARROW by default. We only show a MOVE if 1) the
                            // object is selected, and 2) it may be moved
                            // (i.e., position is not protected).
                            bool bMovable =
                                (!bNotInSelObj) &&
                                (rSh.IsObjSelected() || rSh.IsFrmSelected()) &&
                                (!rSh.IsSelObjProtected(FLYPROTECT_POS));

                            eStyle = bMovable ? POINTER_MOVE : POINTER_ARROW;
                            aActHitType = SDRHIT_OBJECT;
                        }
                    }
                }
                else
                {
                    if ( rSh.IsFrmSelected() && !bNotInSelObj )
                    {
                        // dvo: this branch appears to be dead and should be
                        // removed in a future version. Reason: The condition
                        // !bNotInSelObj means that this branch will only be
                        // executed in the cursor points inside a selected
                        // object. However, if this is the case, the previous
                        // if( rSh.IsObjSelectable(rLPt) ) must always be true:
                        // rLPt is inside a selected object, then obviously
                        // rLPt is over a selectable object.
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
        if( !rSh.IsPageAtPos( rLPt ) || pAnchorMarker )
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

        // which kind of text pointer have we to show - horz / vert - ?
        if( POINTER_TEXT == eStyle && rSh.IsInVerticalText( &rLPt ))
            eStyle = POINTER_TEXT_VERTICAL;

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
        const SwRect& rVisArea = rSh.VisArea();
        if( aOldVis == rVisArea && !rSh.IsStartOfDoc() && !rSh.IsEndOfDoc() )
        {
            //JP 11.10.2001 Bug 72294 - take the center point of VisArea to
            //              decide in which direction the user want.
            if( aModPt.Y() < ( rVisArea.Top() + rVisArea.Height() / 2 ) )
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
#ifdef UNX
    const long coMinLen = 100;
#else
    const long coMinLen = 50;
#endif
    long nTimeout = 800,
         nDiff = Max(
         Max( aMovePos.Y() - rVisArea.Bottom(), rVisArea.Top() - aMovePos.Y() ),
         Max( aMovePos.X() - rVisArea.Right(),  rVisArea.Left() - aMovePos.X()));
#ifdef TEST_FOR_BUG91313
    aTimer.SetTimeout( Max( coMinLen, nTimeout - nDiff) );
#else
    aTimer.SetTimeout( Max( coMinLen, nTimeout - nDiff*2L) );
#endif
}

void SwEditWin::LeaveArea(const Point &rPos)
{
    aMovePos = rPos;
    JustifyAreaTimer();
    if( !aTimer.IsActive() )
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
        com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder =
                rView.GetViewFrame()->GetBindings().GetRecorder();
        if ( xRecorder.is() )
        {
            //Shell ermitteln
            SfxShell *pSfxShell = lcl_GetShellFromDispatcher( rView, TYPE(SwTextShell) );
            // Request generieren und recorden
            if (pSfxShell)
            {
                SfxRequest aReq( rView.GetViewFrame(), FN_INSERT_STRING );
                aReq.AppendItem( SfxStringItem( FN_INSERT_STRING, aInBuffer ) );
                aReq.Done();
            }
        }

        pSh->Insert( aInBuffer );
        aInBuffer.Erase();
        bFlushCharBuffer = FALSE;
    }
}

#define MOVE_LEFT_SMALL     0
#define MOVE_UP_SMALL       1
#define MOVE_RIGHT_BIG      2
#define MOVE_DOWN_BIG       3
#define MOVE_LEFT_BIG       4
#define MOVE_UP_BIG         5
#define MOVE_RIGHT_SMALL    6
#define MOVE_DOWN_SMALL     7

void SwEditWin::ChangeFly( BYTE nDir, BOOL bWeb )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SwRect aTmp = rSh.GetFlyRect();
    if( aTmp.HasArea() )
    {
        SfxItemSet aSet(rSh.GetAttrPool(),
                        RES_FRM_SIZE, RES_FRM_SIZE,
                        RES_VERT_ORIENT, RES_ANCHOR,
                        RES_COL, RES_COL,
                        RES_PROTECT, RES_PROTECT, 0);
        rSh.GetFlyFrmAttr( aSet );
        const SvxProtectItem& rProtect = ((SvxProtectItem&)aSet.Get(RES_PROTECT));
        if( rProtect.IsSizeProtected() ||
            rProtect.IsPosProtected() )
        {
            return;
        }
        RndStdIds eAnchorId = ((SwFmtAnchor&)aSet.Get(RES_ANCHOR)).GetAnchorId();
        Size aSnap;
        if(MOVE_LEFT_SMALL == nDir ||
            MOVE_UP_SMALL == nDir ||
            MOVE_RIGHT_SMALL == nDir ||
            MOVE_DOWN_SMALL == nDir )
            aSnap = PixelToLogic(Size(1,1));
        else
        {
            aSnap = rSh.GetViewOptions()->GetSnapSize();
            short nDiv = rSh.GetViewOptions()->GetDivisionX();
            if ( nDiv > 0 )
                aSnap.Width() = Max( (ULONG)1, (ULONG)aSnap.Width() / nDiv );
            nDiv = rSh.GetViewOptions()->GetDivisionY();
            if ( nDiv > 0 )
                aSnap.Height() = Max( (ULONG)1, (ULONG)aSnap.Height() / nDiv );
        }

        SwRect aBoundRect;
        Point aRefPoint;
        rSh.CalcBoundRect( aBoundRect, eAnchorId, FRAME, FALSE, &aRefPoint );
        long nLeft = Min( aTmp.Left() - aBoundRect.Left(), aSnap.Width() );
        long nRight = Min( aBoundRect.Right() - aTmp.Right(), aSnap.Width() );
        long nUp = Min( aTmp.Top() - aBoundRect.Top(), aSnap.Height() );
        long nDown = Min( aBoundRect.Bottom() - aTmp.Bottom(), aSnap.Height() );

        switch ( nDir )
        {
            case MOVE_LEFT_BIG:
            case MOVE_LEFT_SMALL: aTmp.Left( aTmp.Left() - nLeft ); break;
            case MOVE_UP_BIG:
            case MOVE_UP_SMALL: aTmp.Top( aTmp.Top() - nUp ); break;
            case MOVE_RIGHT_SMALL: if( aTmp.Width() < aSnap.Width() + MINFLY ) break;
                    nRight = aSnap.Width(); // kein break
            case MOVE_RIGHT_BIG: aTmp.Left( aTmp.Left() + nRight ); break;
            case MOVE_DOWN_SMALL: if( aTmp.Height() < aSnap.Height() + MINFLY ) break;
                    nDown = aSnap.Height(); // kein break
            case MOVE_DOWN_BIG: aTmp.Top( aTmp.Top() + nDown ); break;
            default: ASSERT( TRUE, "ChangeFly: Unknown direction." );
        }
        BOOL bSet = FALSE;
        if( FLY_IN_CNTNT == eAnchorId && ( nDir % 2 ) )
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
        if( bWeb && FLY_AT_CNTNT == eAnchorId && ( nDir==MOVE_LEFT_SMALL || nDir==MOVE_RIGHT_BIG ) )
        {
            SwFmtHoriOrient aHori( (SwFmtHoriOrient&)aSet.Get(RES_HORI_ORIENT) );
            SwHoriOrient eNew;
            eNew = aHori.GetHoriOrient();
            switch( eNew )
            {
                case HORI_RIGHT:
                    if( nDir==MOVE_LEFT_SMALL )
                        eNew = HORI_LEFT;
                break;
                case HORI_LEFT:
                    if( nDir==MOVE_RIGHT_BIG )
                        eNew = HORI_RIGHT;
                break;
            }
            if( eNew != aHori.GetHoriOrient() )
            {
                aHori.SetHoriOrient( eNew );
                aSet.Put( aHori );
                bSet = TRUE;
            }
        }
        rSh.StartAllAction();
        if( bSet )
            rSh.SetFlyFrmAttr( aSet );
        BOOL bSetPos = FLY_IN_CNTNT != eAnchorId;
        if(bSetPos && bWeb)
        {
            if(FLY_PAGE != eAnchorId)
                bSetPos = FALSE;
            else
            {
                bSetPos = (::GetHtmlMode(rView.GetDocShell()) & HTMLMODE_SOME_ABS_POS) ?
                    TRUE : FALSE;
            }
        }
        if( bSetPos )
            rSh.SetFlyPos( aTmp.Pos() );
        rSh.EndAllAction();
    }
}
/* -----------------------------23.05.2002 11:35------------------------------

 ---------------------------------------------------------------------------*/
void SwEditWin::ChangeDrawing( BYTE nDir )
{
    long nX = 0;
    long nY = 0;
    sal_Bool bOnePixel = sal_False;
    USHORT nAnchorDir = SW_MOVE_UP;
    switch(nDir)
    {
        case MOVE_LEFT_SMALL:
            bOnePixel = sal_True;
            //no break;
        case MOVE_LEFT_BIG:
            nX = -1;
            nAnchorDir = SW_MOVE_LEFT;
        break;
        case MOVE_UP_SMALL:
            bOnePixel = sal_True;
            //no break;
        case MOVE_UP_BIG:
            nY = -1;
        break;
        case MOVE_RIGHT_SMALL:
            bOnePixel = sal_True;
            //no break;
        case MOVE_RIGHT_BIG:
            nX = +1;
            nAnchorDir = SW_MOVE_RIGHT;
        break;
        case MOVE_DOWN_SMALL:
            bOnePixel = sal_True;
            //no break;
        case MOVE_DOWN_BIG:
            nY = +1;
            nAnchorDir = SW_MOVE_DOWN;
        break;
    }

    if(0 != nX || 0 != nY)
    {
        SwWrtShell &rSh = rView.GetWrtShell();
        BYTE nProtect = rSh.IsSelObjProtected( FlyProtectType(FLYPROTECT_POS|FLYPROTECT_SIZE) );
        Size aSnap( rSh.GetViewOptions()->GetSnapSize() );
        short nDiv = rSh.GetViewOptions()->GetDivisionX();
        if ( nDiv > 0 )
            aSnap.Width() = Max( (ULONG)1, (ULONG)aSnap.Width() / nDiv );
        nDiv = rSh.GetViewOptions()->GetDivisionY();
        if ( nDiv > 0 )
            aSnap.Height() = Max( (ULONG)1, (ULONG)aSnap.Height() / nDiv );

        if(bOnePixel)
            aSnap = PixelToLogic(Size(1,1));

        nX *= aSnap.Width();
        nY *= aSnap.Height();

        SdrView *pSdrView = rSh.GetDrawView();
        const SdrHdlList& rHdlList = pSdrView->GetHdlList();
        SdrHdl* pHdl = rHdlList.GetFocusHdl();
        rSh.StartAllAction();
        if(0L == pHdl)
        {
            // now move the selected draw objects
            // if the object's position is not protected
            if(0 == (nProtect&FLYPROTECT_POS))
            {
                pSdrView->MoveAllMarked(Size(nX, nY));
                rSh.SetModified();
            }
        }
        else
        {
            // move handle with index nHandleIndex
            if(pHdl && (nX || nY))
            {
                if( HDL_ANCHOR == pHdl->GetKind() ||
                    HDL_ANCHOR_TR == pHdl->GetKind() )
                {
                    // anchor move cannot be allowed when position is protected
                    if(0 == (nProtect&FLYPROTECT_POS))
                        rSh.MoveAnchor( nAnchorDir );
                }
                //now resize if size is protected
                else if(0 == (nProtect&FLYPROTECT_SIZE))
                {
                    // now move the Handle (nX, nY)
                    Point aStartPoint(pHdl->GetPos());
                    Point aEndPoint(pHdl->GetPos() + Point(nX, nY));
                    const SdrDragStat& rDragStat = pSdrView->GetDragStat();

                    // start dragging
                    pSdrView->BegDragObj(aStartPoint, 0, pHdl, 0);

                    if(pSdrView->IsDragObj())
                    {
                        FASTBOOL bWasNoSnap = rDragStat.IsNoSnap();
                        BOOL bWasSnapEnabled = pSdrView->IsSnapEnabled();

                        // switch snapping off
                        if(!bWasNoSnap)
                            ((SdrDragStat&)rDragStat).SetNoSnap(TRUE);
                        if(bWasSnapEnabled)
                            pSdrView->SetSnapEnabled(FALSE);

                        pSdrView->MovAction(aEndPoint);
                        pSdrView->EndDragObj();
                        rSh.SetModified();

                        // restore snap
                        if(!bWasNoSnap)
                            ((SdrDragStat&)rDragStat).SetNoSnap(bWasNoSnap);
                        if(bWasSnapEnabled)
                            pSdrView->SetSnapEnabled(bWasSnapEnabled);
                    }
                }
            }
        }
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

    KeyEvent aKeyEvent( rKEvt );
    // look for vertical mappings
    if( !bIsDocReadOnly && !rSh.IsSelFrmMode() && !rSh.IsObjSelected() )
    {
        //JP 21.2.2002: must changed from switch to if, because the Linux
        // compiler has problem with the code. Has to remove if the new general
        // handler exist.
        USHORT nKey = rKEvt.GetKeyCode().GetCode();

#ifdef BIDI
        if( KEY_UP == nKey || KEY_DOWN == nKey ||
            KEY_LEFT == nKey || KEY_RIGHT == nKey )
        {
            if( rSh.IsInVerticalText() )
            {
                if( KEY_UP == nKey ) nKey = KEY_LEFT;
                else if( KEY_DOWN == nKey ) nKey = KEY_RIGHT;
                else if( KEY_LEFT == nKey ) nKey = KEY_DOWN;
                else if( KEY_RIGHT == nKey ) nKey = KEY_UP;
            }
            if ( rSh.IsInRightToLeftText() )
            {
                if( KEY_LEFT == nKey ) nKey = KEY_RIGHT;
                else if( KEY_RIGHT == nKey ) nKey = KEY_LEFT;
            }

            aKeyEvent = KeyEvent( rKEvt.GetCharCode(),
                            KeyCode( nKey, rKEvt.GetKeyCode().GetModifier() ),
                            rKEvt.GetRepeat() );
        }
#else
        if( KEY_UP == nKey ) nKey = KEY_LEFT;
        else if( KEY_DOWN == nKey ) nKey = KEY_RIGHT;
        else if( KEY_LEFT == nKey ) nKey = KEY_DOWN;
        else if( KEY_RIGHT == nKey ) nKey = KEY_UP;
        else nKey = 0;

        if( nKey && rSh.IsInVerticalText() )
        {
            aKeyEvent = KeyEvent( rKEvt.GetCharCode(),
                            KeyCode( nKey, rKEvt.GetKeyCode().GetModifier() ),
                            rKEvt.GetRepeat() );
        }
#endif
    }

    const KeyCode& rKeyCode = aKeyEvent.GetKeyCode();
    sal_Unicode aCh = aKeyEvent.GetCharCode();

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
    int nLclSelectionType;
    //A is converted to 1
    if( rKeyCode.GetFullCode() == (KEY_A | KEY_MOD1 |KEY_SHIFT)
        && rSh.HasDrawView() &&
        (0 != (nLclSelectionType = rSh.GetSelectionType()) &&
        ((nLclSelectionType & SwWrtShell::SEL_FRM) ||
        ((nLclSelectionType & SwWrtShell::SEL_DRW|SwWrtShell::SEL_DRW_FORM) &&
                rSh.GetDrawView()->GetMarkList().GetMarkCount() == 1))))
    {
        SdrHdlList& rHdlList = (SdrHdlList&)rSh.GetDrawView()->GetHdlList();
        SdrHdl* pAnchor = rHdlList.GetHdl(HDL_ANCHOR);
        if ( ! pAnchor )
            pAnchor = rHdlList.GetHdl(HDL_ANCHOR_TR);
        if(pAnchor)
            rHdlList.SetFocusHdl(pAnchor);
        return;
    }

    OfaAutoCorrCfg* pACfg = 0;
    SvxAutoCorrect* pACorr = 0;

    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder =
            rView.GetViewFrame()->GetBindings().GetRecorder();
    if ( !xRecorder.is() )
    {
        pACfg = OFF_APP()->GetAutoCorrConfig();
        pACorr = pACfg->GetAutoCorrect();
    }

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
                    KS_NextObject, KS_PrevObject,
                    KS_KeyToView,
                    KS_LaunchOLEObject, KS_GoIntoFly, KS_GoIntoDrawing,
                    KS_EnterDrawHandleMode,
                    KS_CheckDocReadOnlyKeys,
                    KS_CheckAutoCorrect, KS_EditFormula,
                    KS_ColLeftBig, KS_ColRightBig,
                    KS_ColLeftSmall, KS_ColRightSmall,
                    KS_ColTopBig, KS_ColBottomBig,
                    KS_ColTopSmall, KS_ColBottomSmall,
                    KS_CellLeftBig, KS_CellRightBig,
                    KS_CellLeftSmall, KS_CellRightSmall,
                    KS_CellTopBig, KS_CellBottomBig,
                    KS_CellTopSmall, KS_CellBottomSmall,

                    KS_InsDel_ColLeftBig, KS_InsDel_ColRightBig,
                    KS_InsDel_ColLeftSmall, KS_InsDel_ColRightSmall,
                    KS_InsDel_ColTopBig, KS_InsDel_ColBottomBig,
                    KS_InsDel_ColTopSmall, KS_InsDel_ColBottomSmall,
                    KS_InsDel_CellLeftBig, KS_InsDel_CellRightBig,
                    KS_InsDel_CellLeftSmall, KS_InsDel_CellRightSmall,
                    KS_InsDel_CellTopBig, KS_InsDel_CellBottomBig,
                    KS_InsDel_CellTopSmall, KS_InsDel_CellBottomSmall,
                    KS_TblColCellInsDel,

                    KS_Fly_Change, KS_Draw_Change,
                    KS_SpecialInsert,
                    KS_EnterCharCell,
                    KS_Ende };

    SW_KeyState eKeyState = bIsDocReadOnly ? KS_CheckDocReadOnlyKeys
                                           : KS_CheckKey,
                eNextKeyState = KS_Ende;
    BYTE nDir;

    while( KS_Ende != eKeyState )
    {
        SW_KeyState eFlyState = KS_KeyToView;

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
                    nDir = MOVE_RIGHT_SMALL;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_LEFT | KEY_MOD2:
                    eKeyState = KS_ColRightSmall;
                    eFlyState = KS_Fly_Change;
                    nDir = MOVE_LEFT_SMALL;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_RIGHT | KEY_MOD2 | KEY_SHIFT:
                    eKeyState = KS_ColLeftSmall;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_LEFT | KEY_MOD2 | KEY_SHIFT:
                    eKeyState = KS_ColLeftBig;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_RIGHT | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellRightBig;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_LEFT | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellRightSmall;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_RIGHT | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellLeftSmall;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_LEFT | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellLeftBig;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_UP | KEY_MOD2:
                    eKeyState = KS_ColBottomSmall;
                    eFlyState = KS_Fly_Change;
                    nDir = MOVE_UP_SMALL;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_DOWN | KEY_MOD2:
                    eKeyState = KS_ColBottomBig;
                    eFlyState = KS_Fly_Change;
                    nDir = MOVE_DOWN_SMALL;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_UP | KEY_MOD2 | KEY_SHIFT:
                    eKeyState = KS_ColTopBig;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_DOWN | KEY_MOD2 | KEY_SHIFT:
                    eKeyState = KS_ColTopSmall;
                    goto KEYINPUT_CHECKTABLE;

                case KEY_UP | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellBottomSmall;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_DOWN | KEY_MOD2 | KEY_MOD1:
                    eKeyState = KS_CellBottomBig;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_UP | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellTopBig;
                    goto KEYINPUT_CHECKTABLE;
                case KEY_DOWN | KEY_MOD2 | KEY_SHIFT | KEY_MOD1:
                    eKeyState = KS_CellTopSmall;
                    goto KEYINPUT_CHECKTABLE;

KEYINPUT_CHECKTABLE:
                    if( rSh.IsTableMode() || !rSh.GetTableFmt() )
                    {
                        if(KS_KeyToView != eFlyState)
                        {
                            if(!pFlyFmt && KS_KeyToView != eFlyState &&
                                (rSh.GetSelectionType() & (SwWrtShell::SEL_DRW|SwWrtShell::SEL_DRW_FORM))  &&
                                    rSh.GetDrawView()->HasMarkedObj())
                                eKeyState = KS_Draw_Change;
                        }

                        if( pFlyFmt )
                            eKeyState = eFlyState;
                        else if( KS_Draw_Change != eKeyState)
                            eKeyState = KS_EnterCharCell;
                    }
                    break;

//-------
// Insert/Delete
                case KEY_LEFT:
                case KEY_LEFT | KEY_MOD1:
                {
                    BOOL bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_LEFT_BIG;
                    }
                    eTblChgMode = WH_FLAG_INSDEL |
                            ( bMod1
                                ? WH_CELL_LEFT
                                : WH_COL_LEFT );
                    nTblChgSize = pModOpt->GetTblVInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
//                case KEY_RIGHT:
                case KEY_RIGHT | KEY_MOD1:
                {
                    eTblChgMode = WH_FLAG_INSDEL | WH_CELL_RIGHT;
                    nTblChgSize = pModOpt->GetTblVInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_UP:
                case KEY_UP | KEY_MOD1:
                {
                    BOOL bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_UP_BIG;
                    }
                    eTblChgMode = WH_FLAG_INSDEL |
                            ( bMod1
                                ? WH_CELL_TOP
                                : WH_ROW_TOP );
                    nTblChgSize = pModOpt->GetTblHInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_DOWN:
                case KEY_DOWN | KEY_MOD1:
                {
                    BOOL bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_DOWN_BIG;
                    }
                    eTblChgMode = WH_FLAG_INSDEL |
                            ( bMod1
                                ? WH_CELL_BOTTOM
                                : WH_ROW_BOTTOM );
                    nTblChgSize = pModOpt->GetTblHInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;

KEYINPUT_CHECKTABLE_INSDEL:
                    if( rSh.IsTableMode() || !rSh.GetTableFmt() ||
                        !bTblInsDelMode ||
                        FALSE /* Tabelle geschuetzt */
                            )
                    {
                        const int nSelectionType = rSh.GetSelectionType();

                        eKeyState = KS_KeyToView;
                        if(KS_KeyToView != eFlyState)
                        {
                            if((nSelectionType & (SwWrtShell::SEL_DRW|SwWrtShell::SEL_DRW_FORM))  &&
                                    rSh.GetDrawView()->HasMarkedObj())
                                eKeyState = KS_Draw_Change;
                            else if(nSelectionType & (SwWrtShell::SEL_FRM|SwWrtShell::SEL_OLE|SwWrtShell::SEL_GRF))
                                eKeyState = KS_Fly_Change;
                        }
                    }
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
                        const int nSelectionType = rSh.GetSelectionType();
                        if(nSelectionType & SwWrtShell::SEL_OLE)
                            eKeyState = KS_LaunchOLEObject;
                        else if(nSelectionType & SwWrtShell::SEL_FRM)
                            eKeyState = KS_GoIntoFly;
                        else if((nSelectionType & SwWrtShell::SEL_DRW) &&
                                0 == (nSelectionType & SwWrtShell::SEL_DRW_TXT) &&
                                rSh.GetDrawView()->GetMarkList().GetMarkCount() == 1)
                            eKeyState = KS_GoIntoDrawing;
                        else if( aTmpQHD.HasCntnt() && !rSh.HasSelection() &&
                            aTmpQHD.bIsAutoText )
                            eKeyState = KS_GlossaryExpand;

                        //RETURN und leerer Absatz in Numerierung -> Num. beenden
                        else if( rSh.GetCurNumRule() && !rSh.HasSelection() &&
                                rSh.IsSttPara() && rSh.IsEndPara() )
                            eKeyState = KS_NumOff;

                        //RETURN fuer neuen Absatz mit AutoFormatierung
                        else if( pACfg && pACfg->IsAutoFmtByInput() &&
                                !(nSelectionType & (SwWrtShell::SEL_GRF |
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
                    else if( rSh.CanSpecialInsert() )
                        eKeyState = KS_SpecialInsert;
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
                    {
                        if( rSh.IsEndPara() && rSh.DontExpandFmt() && !rSh.HasReadonlySel())
                            eKeyState = KS_DontExpand;
                        else
                        {
                            BOOL bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                            eFlyState = KS_Fly_Change;
                            nDir = MOVE_RIGHT_BIG;
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
                    else if ( rSh.GetSelectionType() &
                                (SwWrtShell::SEL_GRF |
                                    SwWrtShell::SEL_FRM |
                                    SwWrtShell::SEL_OLE |
                                    SwWrtShell::SEL_DRW |
                                    SwWrtShell::SEL_DRW_FORM))

                            eKeyState = KS_NextObject;
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
                    else if ( rSh.GetSelectionType() &
                                (SwWrtShell::SEL_GRF |
                                    SwWrtShell::SEL_FRM |
                                    SwWrtShell::SEL_OLE |
                                    SwWrtShell::SEL_DRW |
                                    SwWrtShell::SEL_DRW_FORM))

                            eKeyState = KS_PrevObject;
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
                case KEY_TAB | KEY_MOD2:
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
                        else if((rSh.GetSelectionType() &
                                    (SwWrtShell::SEL_DRW|SwWrtShell::SEL_DRW_FORM|
                                        SwWrtShell::SEL_FRM|SwWrtShell::SEL_OLE|SwWrtShell::SEL_GRF))  &&
                                rSh.GetDrawView()->HasMarkedObj())
                            eKeyState = KS_EnterDrawHandleMode;
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
                        else if((rSh.GetSelectionType() & (SwWrtShell::SEL_DRW|SwWrtShell::SEL_DRW_FORM|
                                        SwWrtShell::SEL_FRM|SwWrtShell::SEL_OLE|SwWrtShell::SEL_GRF)) &&
                                rSh.GetDrawView()->HasMarkedObj())
                            eKeyState = KS_EnterDrawHandleMode;
                    break;
                    case KEY_F2 :
                    if( !rSh.HasReadonlySel() )
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        if(nSelectionType & SwWrtShell::SEL_FRM)
                            eKeyState = KS_GoIntoFly;
                        else if((nSelectionType & SwWrtShell::SEL_DRW))
                            eKeyState = KS_GoIntoDrawing;
                    }
                    break;
                }
            }
            break;
        case KS_CheckDocReadOnlyKeys:
            {
                eKeyState = KS_KeyToView;
                switch( rKeyCode.GetModifier() | rKeyCode.GetCode() )
                {
                    case KEY_TAB:
                    case KEY_TAB | KEY_SHIFT:
                        bNormalChar = FALSE;
                        eKeyState = KS_Ende;
                        if ( rSh.GetSelectionType() &
                                (SwWrtShell::SEL_GRF |
                                    SwWrtShell::SEL_FRM |
                                    SwWrtShell::SEL_OLE |
                                    SwWrtShell::SEL_DRW |
                                    SwWrtShell::SEL_DRW_FORM))

                        {
                            eKeyState = rKeyCode.GetModifier() & KEY_SHIFT ?
                                                KS_PrevObject : KS_NextObject;
                        }
                        else
                            rSh.SelectNextPrevHyperlink(
                                            KEY_SHIFT != rKeyCode.GetModifier() );
                    break;
                    case KEY_RETURN:
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        if(nSelectionType & SwWrtShell::SEL_FRM)
                            eKeyState = KS_GoIntoFly;
                        else
                        {
                            SfxItemSet aSet(rSh.GetAttrPool(), RES_TXTATR_INETFMT, RES_TXTATR_INETFMT);
                            rSh.GetAttr(aSet);
                            if(SFX_ITEM_SET == aSet.GetItemState(RES_TXTATR_INETFMT, FALSE))
                            {
                                const SfxPoolItem& rItem = aSet.Get(RES_TXTATR_INETFMT, TRUE);
                                bNormalChar = FALSE;
                                eKeyState = KS_Ende;
                                rSh.ClickToINetAttr((const SwFmtINetFmt&)rItem, URLLOAD_NOFILTER);
                            }
                        }
                    }
                    break;
                }
            }
            break;

        case KS_EnterCharCell:
            {
                eKeyState = KS_KeyToView;
                switch ( rKeyCode.GetModifier() | rKeyCode.GetCode() )
                {
                    case KEY_RIGHT | KEY_MOD2:
                        rSh.Right( CRSR_SKIP_CHARS, FALSE, 1, FALSE );
                        eKeyState = KS_Ende;
                        FlushInBuffer( &rSh );
                        break;
                    case KEY_LEFT | KEY_MOD2:
                        rSh.Left( CRSR_SKIP_CHARS, FALSE, 1, FALSE );
                        eKeyState = KS_Ende;
                        FlushInBuffer( &rSh );
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

                if( rView.KeyInput( aKeyEvent ) )
                    bFlushBuffer = TRUE, bNormalChar = FALSE;
                else
                {
// OS 22.09.95: Da der Sfx Acceleratoren nur aufruft, wenn sie beim letzten
//              Statusupdate enabled wurden, muss copy ggf. von uns
//              'gewaltsam' gerufen werden.
                    if( rKeyCode.GetFunction() == KEYFUNC_COPY )
                        GetView().GetViewFrame()->GetBindings().Execute(SID_COPY);


                    if( !bIsDocReadOnly && bNormalChar )
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        if((nSelectionType & SwWrtShell::SEL_DRW) &&
                            0 == (nSelectionType & SwWrtShell::SEL_DRW_TXT) &&
                            rSh.GetDrawView()->GetMarkList().GetMarkCount() == 1)
                                eKeyState = KS_GoIntoDrawing;
                        else
                            eKeyState = KS_InsChar;
                    }
                    else
                    {
                        bNormalChar = FALSE;
                        Window::KeyInput( aKeyEvent );
                    }
                }
            }
            break;
        case KS_LaunchOLEObject:
            rSh.LaunchOLEObj();
            eKeyState = KS_Ende;
        break;
        case KS_GoIntoFly :
            rSh.UnSelectFrm();
            rSh.LeaveSelFrmMode();
            rView.AttrChangedNotify(&rSh);
            eKeyState = KS_Ende;
        break;
        case KS_GoIntoDrawing :
        {
            SdrObject* pObj = rSh.GetDrawView()->GetMarkList().GetMark(0)->GetObj();
            if(pObj)
            {
                EnterDrawTextMode(pObj->GetLogicRect().Center());
                if ( rView.GetCurShell()->ISA(SwDrawTextShell) )
                    ((SwDrawTextShell*)rView.GetCurShell())->Init();
            }
            eKeyState = KS_Ende;
        }
        break;
        case KS_EnterDrawHandleMode:
        {
            const SdrHdlList& rHdlList = rSh.GetDrawView()->GetHdlList();
            sal_Bool bForward(!aKeyEvent.GetKeyCode().IsShift());

            ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);
            eKeyState = KS_Ende;
        }
        break;
        case KS_InsTab:
            if( rView.ISA( SwWebView ))     //Kein Tabulator fuer Web!
            {
                // Bug 56196 - dann sollte der weiter gereicht werden.
                Window::KeyInput( aKeyEvent );
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


            if( !aKeyEvent.GetRepeat() && pACorr &&
                    pACfg->IsAutoFmtByInput() &&
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
            else if( !aKeyEvent.GetRepeat() && pACorr &&
                    pACfg->IsAutoFmtByInput() &&
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
                aInBuffer.Expand( aInBuffer.Len() + aKeyEvent.GetRepeat() + 1,aCh );
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
// ???          Window::KeyInput( aKeyEvent );
            eKeyState = KS_Ende;
        }
        break;

        case KS_CheckAutoCorrect:
        {
            if( pACorr && pACfg->IsAutoFmtByInput() &&
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
            USHORT nSlotId = 0;
            FlushInBuffer( &rSh );
//???           if( bFlushCharBuffer )
//???               FlushInBuffer( &rSh );
            switch( eKeyState )
            {
            case KS_SpecialInsert:
                rSh.DoSpecialInsert();
                break;

            case KS_NoNum:
                rSh.NoNum();
                break;

            case KS_NumOff:
                // Shellwechsel - also vorher aufzeichnen
                rSh.DelNumRules();
                break;

            case KS_NumDown:
                rSh.NumUpDown( TRUE );
                break;
            case KS_NumUp:
                rSh.NumUpDown( FALSE );
                break;

            case KS_OutlineDown:
                rSh.OutlineUpDown( 1 );
                break;
            case KS_OutlineUp:
                rSh.OutlineUpDown( -1 );
                break;

            case KS_NextCell:
                //In Tabelle immer 'flushen'
                rSh.GoNextCell();
                nSlotId = FN_GOTO_NEXT_CELL;
                break;
            case KS_PrevCell:
                rSh.GoPrevCell();
                nSlotId = FN_GOTO_PREV_CELL;
                break;
            case KS_AutoFmtByInput:
                rSh.SplitNode( TRUE );
                break;

//          case KS_NumOrNoNum:
//          case KS_DontExpand:
//              break;
            case KS_NextObject:
            case KS_PrevObject:
                if(rSh.GotoObj( KS_NextObject == eKeyState, GOTO_ANY))
                {
                    if( rSh.IsFrmSelected() &&
                        rView.GetDrawFuncPtr() )
                    {
                        rView.GetDrawFuncPtr()->Deactivate();
                        rView.SetDrawFuncPtr(NULL);
                        rView.LeaveDrawCreate();
                        rView.AttrChangedNotify( &rSh );
                    }
                    rSh.HideCrsr();
                    rSh.EnterSelFrmMode();
                }
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
            {
                SdrView *pSdrView = rSh.GetDrawView();
                const SdrHdlList& rHdlList = pSdrView->GetHdlList();
                if(rHdlList.GetFocusHdl())
                    ChangeDrawing( nDir );
                else
                    ChangeFly( nDir, rView.ISA( SwWebView ) );
            }
            break;
            case KS_Draw_Change :
                ChangeDrawing( nDir );
                break;
            }
            if( nSlotId && rView.GetViewFrame()->GetBindings().GetRecorder().is() )
            {
                SfxRequest aReq(rView.GetViewFrame(), nSlotId );
                aReq.Done();
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
            ShowAutoTextCorrectQuickHelp(sWord, pACfg, pACorr);
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

    BYTE nMouseTabCol = 0;
    if ( !rSh.IsDrawCreate() && !pApplyTempl && !rSh.IsInSelect() &&
         rMEvt.GetClicks() == 1 && MOUSE_LEFT == rMEvt.GetButtons() &&
         !rSh.IsTableMode() &&
         0 != (nMouseTabCol = rSh.WhichMouseTabCol( aDocPos ) ))
    {
        //Zuppeln von Tabellenspalten aus dem Dokument heraus.
        rView.SetTabColFromDoc( TRUE );
        rView.SetTabColFromDocPos( aDocPos );
        rView.InvalidateRulerPos();
        SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
        rBind.Update();
        if ( RulerColumnDrag( rView , rMEvt, SW_TABCOL_VERT == nMouseTabCol ) )
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
                    if( !bIsDocReadOnly &&
                        !pAnchorMarker &&
                        0 != ( pHdl = pSdrView->HitHandle(aDocPos, *(rSh.GetOut())) ) &&
                            ( pHdl->GetKind() == HDL_ANCHOR ||
                              pHdl->GetKind() == HDL_ANCHOR_TR ) )
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
                        BOOL bHitHandle = pHdl && pHdl->GetKind() != HDL_ANCHOR &&
                                                  pHdl->GetKind() != HDL_ANCHOR_TR;

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
                            // #107513#
                            // Test if there is a draw object at that position and if it should be selected.
                            sal_Bool bShould = rSh.ShouldObjectBeSelected(aDocPos);

                            if(bShould)
                            {
                                rView.NoRotate();
                                rSh.HideCrsr();

                                BOOL bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( TRUE );
                                BOOL bSelObj = rSh.SelectObj( aDocPos,
                                               rMEvt.IsMod1() ? SW_ENTER_GROUP : 0);
                                if( bUnLockView )
                                    rSh.LockView( FALSE );

                                if( bSelObj )
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
                        }
                        else if ( rSh.IsSelFrmMode() &&
                                  (aActHitType == SDRHIT_NONE ||
                                   !rSh.IsInsideSelectedObj( aDocPos )))
                        {
                            rView.NoRotate();
                            SdrHdl *pHdl;
                            if( !bIsDocReadOnly && !pAnchorMarker && 0 !=
                                ( pHdl = pSdrView->HitHandle(aDocPos, *(rSh.GetOut())) ) &&
                                    ( pHdl->GetKind() == HDL_ANCHOR ||
                                      pHdl->GetKind() == HDL_ANCHOR_TR ) )
                            {
                                pAnchorMarker = new SwAnchorMarker( pHdl );
                                UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                return;
                            }
                            else
                            {
                                BOOL bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( TRUE );
                                BYTE nFlag = rMEvt.IsShift() ? SW_ADD_SELECT :0;
                                if( rMEvt.IsMod1() )
                                    nFlag = nFlag | SW_ENTER_GROUP;

                                if ( rSh.IsSelFrmMode() )
                                {
                                    rSh.UnSelectFrm();
                                    rSh.LeaveSelFrmMode();
                                    rView.AttrChangedNotify(&rSh);
                                }

                                BOOL bSelObj = rSh.SelectObj( aDocPos, nFlag );
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
                                GetView().GetViewFrame()->GetBindings().Execute(
                                    FN_FORMAT_GRAFIC_DLG, 0, 0,
                                    SFX_CALLMODE_RECORD|SFX_CALLMODE_SLOT);
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
                                GetView().GetViewFrame()->GetBindings().Execute(
                                    FN_FORMAT_FRAME_DLG, 0, 0, SFX_CALLMODE_RECORD|SFX_CALLMODE_SLOT);
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
                                GetView().GetViewFrame()->GetBindings().Execute( FN_EDIT_FOOTNOTE );
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
                                    pVFrame->GetBindings().Execute(nSlot);
                                    break;
                                }
                                case TYP_AUTHORITY :
                                    pVFrame->GetBindings().Execute(FN_EDIT_AUTH_ENTRY_DLG);
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
                                rSh.SelectObj(aDocPos, SW_ADD_SELECT | SW_ENTER_GROUP);
                            else
                            {   if ( rSh.SelectObj( aDocPos, SW_ADD_SELECT | SW_ENTER_GROUP ) )
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
                                rSh.SelectObj(aDocPos, SW_ENTER_GROUP);
                            else
                            {   if ( rSh.SelectObj( aDocPos, SW_ENTER_GROUP ) )
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

                                BOOL bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( TRUE );
                                rSh.EnterAddMode();
                                if( bUnLockView )
                                    rSh.LockView( FALSE );
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
                                rSh.SelectObj(aDocPos, SW_ADD_SELECT);

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
#ifdef TEST_FOR_BUG91313
    // n Pixel as FUZZY border
    SwRect aVis( rSh.VisArea() );
    Size aFuzzySz( 2, 2 );
    aFuzzySz = PixelToLogic( aFuzzySz );

    aVis.Top(    aVis.Top()    + aFuzzySz.Height() );
    aVis.Bottom( aVis.Bottom() - aFuzzySz.Height() );
    aVis.Left(   aVis.Left()   + aFuzzySz.Width() );
    aVis.Right(  aVis.Right()  - aFuzzySz.Width() );
    const BOOL bInsWin = aVis.IsInside( aDocPt );
#else
    const BOOL bInsWin = rSh.VisArea().IsInside( aDocPt );
#endif

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

    if(rView.GetDrawFuncPtr())
    {
        if( bInsDraw  )
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
        else if(!rSh.IsFrmSelected() && !rSh.IsObjSelected())
        {
            SfxBindings &rBnd = rSh.GetView().GetViewFrame()->GetBindings();
            Point aRelPos = rSh.GetRelativePagePosition(aDocPt);
            if(aRelPos.X() >= 0)
            {
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &GetView()));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
                const SfxPointItem aTmp1( SID_ATTR_POSITION, aRelPos );
                rBnd.SetState( aTmp1 );
            }
            else
            {
                rBnd.Invalidate(SID_ATTR_POSITION);
            }
            rBnd.Invalidate(SID_ATTR_SIZE);
            const SfxStringItem aCell( SID_TABLE_CELL, aEmptyStr );
            rBnd.SetState( aCell );
        }
    }

    BYTE nMouseTabCol;
    if( !bIsDocReadOnly && bInsWin && !pApplyTempl && !rSh.IsInSelect() &&
         0 != (nMouseTabCol = rSh.WhichMouseTabCol( aDocPt )) && !rSh.IsTableMode())
    {
        //Zuppeln von Tabellenspalten aus dem Dokument heraus.

        SetPointer( SW_TABCOL_VERT == nMouseTabCol ? POINTER_VSIZEBAR : POINTER_HSIZEBAR );
        return;
    }

    BOOL bDelShadCrsr = TRUE;

    switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
    {
        case MOUSE_LEFT:
            if( pAnchorMarker )
            {
                // Now we need to refresh the SdrHdl pointer of pAnchorMarker.
                // This looks a little bit tricky, but it solves the following
                // problem: the pAnchorMarker contains a pointer to an SdrHdl,
                // if the FindAnchorPos-call cause a scrolling of the visible
                // area, it's possible that the SdrHdl will be destroyed and a
                // new one will initialized at the original position(GetHdlPos).
                // So the pAnchorMarker has to find the right SdrHdl, if it's
                // the old one, it will find it with position aOld, if this one
                // is destroyed, it will find a new one at position GetHdlPos().
                Point aOld = pAnchorMarker->GetPos();
                Point aNew = rSh.FindAnchorPos( aDocPt );
                SdrHdl* pHdl;
                if( (0!=( pHdl = pSdrView->HitHandle( aOld, *(rSh.GetOut())) )||
                    0 !=(pHdl = pSdrView->HitHandle( pAnchorMarker->GetHdlPos(),
                    *(rSh.GetOut())) ) ) &&
                        ( pHdl->GetKind() == HDL_ANCHOR ||
                          pHdl->GetKind() == HDL_ANCHOR_TR ) )
                {
                    pAnchorMarker->ChgHdl( pHdl );
                    if( aNew.X() || aNew.Y() )
                    {
                         pAnchorMarker->SetPos( aNew );
                         pAnchorMarker->SetLastPos( aDocPt );
                         pSdrView->RefreshAllIAOManagers();
                     }
                }
                else
                {
                    delete pAnchorMarker;
                    pAnchorMarker = NULL;
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
                                SwViewOption::GetDirectCursorColor() );
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
                {
                    rView.SetDrawFuncPtr(NULL);
                    SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
                    rBind.Invalidate( SID_ATTR_SIZE );
                    rBind.Invalidate( SID_TABLE_CELL );
                }
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
                        USHORT nFilter = URLLOAD_NOFILTER;
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
            String aStyleName;
            switch ( pApplyTempl->eType )
            {
                case SFX_STYLE_FAMILY_PARA:
                    if( (( SwWrtShell::SEL_TXT | SwWrtShell::SEL_TBL )
                         & eSelection ) && !rSh.HasReadonlySel() )
                    {
                        rSh.SetTxtFmtColl( pApplyTempl->aColl.pTxtColl );
                        pApplyTempl->bUndo = TRUE;
                        bCallBase = FALSE;
                        if ( pApplyTempl->aColl.pTxtColl )
                            aStyleName = pApplyTempl->aColl.pTxtColl->GetName();
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
                        if ( pApplyTempl->aColl.pCharFmt )
                            aStyleName = pApplyTempl->aColl.pCharFmt->GetName();
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
                        if( pApplyTempl->aColl.pFrmFmt )
                            aStyleName = pApplyTempl->aColl.pFrmFmt->GetName();
                    }
                    break;
                }
                case SFX_STYLE_FAMILY_PAGE:
                            // Kein Undo bei Seitenvorlagen
                    rSh.ChgCurPageDesc( *pApplyTempl->aColl.pPageDesc );
                    if ( pApplyTempl->aColl.pPageDesc )
                        aStyleName = pApplyTempl->aColl.pPageDesc->GetName();
                    bCallBase = FALSE;
                    break;
                case SFX_STYLE_FAMILY_PSEUDO:
                    if( !rSh.HasReadonlySel() )
                    {
                        rSh.SetCurNumRule( *pApplyTempl->aColl.pNumRule );
                        bCallBase = FALSE;
                        pApplyTempl->bUndo = TRUE;
                        if( pApplyTempl->aColl.pNumRule )
                            aStyleName = pApplyTempl->aColl.pNumRule->GetName();
                    }
                    break;
            }

            com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder =
                    rView.GetViewFrame()->GetBindings().GetRecorder();
            if ( aStyleName.Len() && xRecorder.is() )
            {
                SfxShell *pSfxShell = lcl_GetShellFromDispatcher( rView, TYPE(SwTextShell) );
                if ( pSfxShell )
                {
                    SfxRequest aReq( rView.GetViewFrame(), SID_STYLE_APPLY );
                    aReq.AppendItem( SfxStringItem( SID_STYLE_APPLY, aStyleName ) );
                    aReq.AppendItem( SfxUInt16Item( SID_STYLE_FAMILY, (USHORT) pApplyTempl->eType ) );
                    aReq.Done();
                }
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
    DropTargetHelper( this ),
    DragSourceHelper( this ),
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
        BOOL bUnLockView = !rSh.IsViewLocked();
        rSh.LockView( TRUE );

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
        if( bUnLockView )
            rSh.LockView( FALSE );
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
#ifdef ACCESSIBLE_LAYOUT
    rView.GetWrtShell().InvalidateAccessibleFocus();
#endif
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/



void SwEditWin::LoseFocus()
{
#ifdef ACCESSIBLE_LAYOUT
    rView.GetWrtShell().InvalidateAccessibleFocus();
#endif
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
                        const SfxPoolItem* aArgs[2];
                        aArgs[0] = pItem;
                        aArgs[1] = 0;
                        GetView().GetViewFrame()->GetBindings().Execute(
                                    nSlotId, aArgs, 0, SFX_CALLMODE_STANDARD );
                        delete pItem;
                    }
                    else
                        GetView().GetViewFrame()->GetBindings().Execute( nSlotId );
                }
            }
            break;

    case COMMAND_STARTEXTTEXTINPUT:
    {
        SwWrtShell &rSh = rView.GetWrtShell();
        BOOL bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
        if(!bIsDocReadOnly)
        {
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
        }
        break;
    }
    case COMMAND_ENDEXTTEXTINPUT:
    {
        SwWrtShell &rSh = rView.GetWrtShell();
        BOOL bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
        if(!bIsDocReadOnly)
        {
            if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
            {
                bCallBase = FALSE;
                rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
            }
            else
            {
                bCallBase = FALSE;
                String sRecord = rSh.DeleteExtTextInput();
                com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder =
                        rView.GetViewFrame()->GetBindings().GetRecorder();

                if ( sRecord.Len() )
                {
                    // #102812# convert quotes in IME text
                    // works on the last input character, this is escpecially in Korean text often done
                    // quotes that are inside of the string are not replaced!
                    const sal_Unicode aCh = sRecord.GetChar(sRecord.Len() - 1);
                    OfaAutoCorrCfg* pACfg = OFF_APP()->GetAutoCorrConfig();
                    SvxAutoCorrect* pACorr = pACfg->GetAutoCorrect();
                    if(pACorr &&
                        ( pACorr->IsAutoCorrFlag( ChgQuotes ) && ('\"' == aCh ))||
                        ( pACorr->IsAutoCorrFlag( ChgSglQuotes ) && ( '\'' == aCh)))
                    {
                        rSh.DelLeft();
                        rSh.AutoCorrect( *pACorr, aCh );
                    }

                    if ( xRecorder.is() )
                    {
                        //Shell ermitteln
                        SfxShell *pSfxShell = lcl_GetShellFromDispatcher( rView, TYPE(SwTextShell) );
                        // Request generieren und recorden
                        if (pSfxShell)
                        {
                            SfxRequest aReq( rView.GetViewFrame(), FN_INSERT_STRING );
                            aReq.AppendItem( SfxStringItem( FN_INSERT_STRING, sRecord ) );
                            aReq.Done();
                        }
                    }
                }
            }
        }
    }
    break;
    case COMMAND_EXTTEXTINPUT:
    {
        SwWrtShell &rSh = rView.GetWrtShell();
        BOOL bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
        if(!bIsDocReadOnly)
        {
            QuickHelpData aTmpQHD;
            if( pQuickHlpData->bClear )
            {
                aTmpQHD.Move( *pQuickHlpData );
                pQuickHlpData->Stop( rSh );
            }
            String sWord;
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
                    sWord = pData->GetText();
                    bCallBase = FALSE;
                    rSh.SetExtTextInputData( *pData );
                }
            }
                com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder =
                        rView.GetViewFrame()->GetBindings().GetRecorder();
                if(!xRecorder.is())
                {
                    OfaAutoCorrCfg* pACfg = OFF_APP()->GetAutoCorrConfig();
                    SvxAutoCorrect* pACorr = pACfg->GetAutoCorrect();
                    if( pACfg && pACorr &&
                        ( pACfg->IsAutoTextTip() ||
                          pACorr->GetSwFlags().bAutoCompleteWords ) &&
                        rSh.GetPrevAutoCorrWord( *pACorr, sWord ) )
                    {
                        ShowAutoTextCorrectQuickHelp(sWord, pACfg, pACorr);
                    }
                }
        }
    }
    break;
    case COMMAND_CURSORPOS:
        // will be handled by the base class
        break;

    case COMMAND_PASTESELECTION:
        if( !rView.GetDocShell()->IsReadOnly() )
        {
            TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSelection( this ));
            if( !aDataHelper.GetXTransferable().is() )
                break;

            ULONG nDropFormat;
            USHORT nEventAction, nDropAction, nDropDestination;
            nDropDestination = GetDropDestination( rCEvt.GetMousePosPixel() );
            if( !nDropDestination )
                break;

            nDropAction = SotExchange::GetExchangeAction(
                                aDataHelper.GetDataFlavorExVector(),
                                nDropDestination, EXCHG_IN_ACTION_COPY,
                                EXCHG_IN_ACTION_COPY, nDropFormat,
                                nEventAction );
            if( EXCHG_INOUT_ACTION_NONE != nDropAction )
            {
                const Point aDocPt( PixelToLogic( rCEvt.GetMousePosPixel() ) );
                SwTransferable::PasteData( aDataHelper, rSh, nDropAction,
                                    nDropFormat, nDropDestination, FALSE,
                                    FALSE, &aDocPt, EXCHG_IN_ACTION_COPY,
                                    TRUE );
            }
        }
        break;
        case COMMAND_MODKEYCHANGE :
        {
            const CommandModKeyData* pCommandData = (const CommandModKeyData*)rCEvt.GetData();
            if(pCommandData->IsMod1() && !pCommandData->IsMod2())
            {
                USHORT nSlot = 0;
                if(pCommandData->IsLeftShift() && !pCommandData->IsRightShift())
                    nSlot = SID_ATTR_PARA_LEFT_TO_RIGHT;
                else if(!pCommandData->IsLeftShift() && pCommandData->IsRightShift())
                    nSlot = SID_ATTR_PARA_RIGHT_TO_LEFT;
                if(nSlot && SW_MOD()->GetCTLOptions().IsCTLFontEnabled())
                    GetView().GetViewFrame()->GetDispatcher()->Execute(nSlot);
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
/* -----------------23.01.2003 12:15-----------------
 * #i3370# remove quick help to prevent saving
 * of autocorrection suggestions
 * --------------------------------------------------*/
void SwEditWin::StopQuickHelp()
{
    if( HasFocus() && pQuickHlpData && pQuickHlpData->bClear  )
        pQuickHlpData->Stop( rView.GetWrtShell() );
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

#ifdef ACCESSIBLE_LAYOUT
::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SwEditWin::CreateAccessible()
{
    vos::OGuard aGuard(Application::GetSolarMutex());   // this should have
                                                        // happend already!!!
    SwWrtShell *pSh = rView.GetWrtShellPtr();
    ASSERT( pSh, "no writer shell, no accessible object" );
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > xAcc;
    if( pSh )
        xAcc = pSh->CreateAccessible();

    return xAcc;
}
#endif

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

    if( pAttrs )
        delete[] pAttrs;
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
    delete[] pAttrs, pAttrs = 0;
}


void QuickHelpData::Start( SwWrtShell& rSh, USHORT nWrdLen )
{
    if( pCETID ) delete pCETID, pCETID = 0;
    if( pAttrs ) delete[] pAttrs, pAttrs = 0;

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
                                                0, 0, 0, FALSE );
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



void QuickHelpData::FillStrArr( SwWrtShell& rSh, const String& rWord )
{
    pCalendarWrapper->LoadDefaultCalendar( rSh.GetCurLang() );

    using namespace ::com::sun::star;
    {
        uno::Sequence< i18n::CalendarItem > aNames(
                                            pCalendarWrapper->getMonths() );
        for( int n = 0; n < 2; ++n )
        {
            for( long nPos = 0, nEnd = aNames.getLength(); nPos < nEnd; ++nPos )
            {
                String sStr( aNames[ nPos ].FullName );
                if( rWord.Len() + 1 < sStr.Len() &&

//!!! UNICODE: fehlendes interface
//                  pIntl->CompareEqual( rWord, sStr.Copy( 0, rWord.Len() ),
//                              INTN_COMPARE_IGNORECASE ) )
                    COMPARE_EQUAL == rWord.CompareIgnoreCaseToAscii(
                                        sStr, rWord.Len() ))
                {
                    String* pNew = new String( sStr );
                    if( !aArr.Insert( pNew ) )
                        delete pNew;
                }
            }
            if( !n )                    // get data for the second loop
                aNames = pCalendarWrapper->getDays();
        }
    }

    // and than add all words from the AutoCompleteWord-List
    const SwAutoCompleteWord& rACLst = rSh.GetAutoCompleteWords();
    USHORT nStt, nEnd;
    if( rACLst.GetRange( rWord, nStt, nEnd ) )
    {
        while( nStt < nEnd )
        {
            const String& rS = rACLst[ nStt ];
            //JP 16.06.99: Bug 66927 - only if the count of chars
            //              from the suggest greater as the
            //              actual word
            if( rS.Len() > rWord.Len() )
            {
                String* pNew = new String( rS );
                if( !aArr.Insert( pNew ) )
                    delete pNew;
            }
            ++nStt;
        }
    }
}
/* -----------------06.11.2002 12:01-----------------
 *
 * --------------------------------------------------*/
void SwEditWin::ShowAutoTextCorrectQuickHelp(
        const String& rWord, OfaAutoCorrCfg* pACfg, SvxAutoCorrect* pACorr )
{
    SwWrtShell& rSh = rView.GetWrtShell();
    pQuickHlpData->ClearCntnt();
    if( pACfg->IsAutoTextTip() )
    {
        SwGlossaryList* pList = ::GetGlossaryList();
        pList->HasLongName( rWord, &pQuickHlpData->aArr );
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

        pQuickHlpData->FillStrArr( rSh, rWord );
    }

    if( pQuickHlpData->aArr.Count() )
        pQuickHlpData->Start( rSh, rWord.Len() );
}

