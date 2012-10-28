/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <swtypes.hxx>
#include <hintids.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>

#include <com/sun/star/i18n/UnicodeScript.hpp>

#include <vcl/help.hxx>
#include <vcl/graph.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/cmdevt.h>
#include <sot/storage.hxx>
#include <svl/macitem.hxx>
#include <unotools/securityoptions.hxx>
#include <basic/sbxvar.hxx>
#include <svl/ctloptions.hxx>
#include <basic/sbx.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/ipclient.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/ptitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/htmlmode.hxx>
#include <svx/svdview.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editeng.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/protitem.hxx>
#include <unotools/charclass.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <editeng/acorrcfg.hxx>
#include <SwSmartTagMgr.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <fldbas.hxx>
#include <swmodule.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>
#include <drawbase.hxx>
#include <dselect.hxx>
#include <textsh.hxx>
#include <shdwcrsr.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtclds.hxx>
#include <fmthdft.hxx>
#include <frmfmt.hxx>
#include <modcfg.hxx>
#include <fmtcol.hxx>
#include <wview.hxx>
#include <listsh.hxx>
#include <gloslst.hxx>
#include <inputwin.hxx>
#include <gloshdl.hxx>
#include <swundo.hxx>
#include <drwtxtsh.hxx>
#include <fchrfmt.hxx>
#include <fmturl.hxx>
#include <romenu.hxx>
#include <initui.hxx>
#include <frmatr.hxx>
#include <extinput.hxx>
#include <acmplwrd.hxx>
#include <swcalwrp.hxx>
#include <swdtflvr.hxx>
#include <wdocsh.hxx>
#include <crsskip.hxx>
#include <breakit.hxx>
#include <checkit.hxx>
#include <pagefrm.hxx>
#include <HeaderFooterWin.hxx>

#include <helpid.h>
#include <cmdid.h>
#include <docvw.hrc>
#include <uitool.hxx>
#include <fmtfollowtextflow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <charfmt.hxx>
#include <numrule.hxx>
#include <pagedesc.hxx>
#include <svtools/ruler.hxx>
#include "formatclipboard.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <docstat.hxx>
#include <wordcountdialog.hxx>
#include <swwait.hxx>

#include <IMark.hxx>
#include <doc.hxx>
#include <xmloff/odffields.hxx>

#include <PostItMgr.hxx>

#include <algorithm>
#include <vector>

#include "../../core/inc/rootfrm.hxx"

using namespace sw::mark;
using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Description:   Globals
 --------------------------------------------------------------------*/

static bool bInputLanguageSwitched = false;
extern sal_Bool bNoInterrupt;       // in mainwn.cxx

// Usually in MouseButtonUp a selection is revoked when the selection is
// not currently being pulled open. Unfortunately in MouseButtonDown there
// is being selected at double/triple click. That selection is completely
// finished in the Handler and thus can't be distinguished in the Up.
// To resolve this bHoldSelection is set in Down at evaluated in Up.
static sal_Bool bHoldSelection      = sal_False;

sal_Bool bFrmDrag                   = sal_False;
sal_Bool bValidCrsrPos              = sal_False;
sal_Bool bModePushed                = sal_False;
sal_Bool bDDTimerStarted            = sal_False;
sal_Bool bFlushCharBuffer           = sal_False;
sal_Bool SwEditWin::bReplaceQuote   = sal_False;
sal_Bool bDDINetAttr                = sal_False;
SdrHdlKind eSdrMoveHdl          = HDL_USER;

QuickHelpData* SwEditWin::pQuickHlpData = 0;

long    SwEditWin::nDDStartPosY = 0;
long    SwEditWin::nDDStartPosX = 0;
/* Note:
   The initial color shown on the button is set in /core/svx/source/tbxctrls/tbxcolorupdate.cxx
   (ToolboxButtonColorUpdater::ToolboxButtonColorUpdater()) .
   The initial color used by the button is set in /core/svx/source/tbxcntrls/tbcontrl.cxx
   (SvxColorExtToolBoxControl::SvxColorExtToolBoxControl())
   and in case of writer for text(background)color also in /core/sw/source/ui/docvw/edtwin.cxx
   (SwEditWin::aTextBackColor and SwEditWin::aTextBackColor)
 */
Color   SwEditWin::aTextBackColor(COL_YELLOW);
Color   SwEditWin::aTextColor(COL_RED);
sal_Bool SwEditWin::bTransparentBackColor = sal_False; // background not transparent

extern sal_Bool     bExecuteDrag;

static SfxShell* lcl_GetShellFromDispatcher( SwView& rView, TypeId nType );

DBG_NAME(edithdl)

namespace
{
    static bool lcl_CheckHeaderFooterClick( SwWrtShell& rSh, const Point &rDocPos, sal_uInt16 nClicks )
    {
        bool bRet = false;

        sal_Bool bOverHdrFtr = rSh.IsOverHeaderFooterPos( rDocPos );
        if ( ( rSh.IsHeaderFooterEdit( ) && !bOverHdrFtr ) ||
             ( !rSh.IsHeaderFooterEdit() && bOverHdrFtr ) )
        {
            bRet = true;
            // Check if there we are in a FlyFrm
            Point aPt( rDocPos );
            SwPaM aPam( *rSh.GetCurrentShellCursor().GetPoint() );
            rSh.GetLayout()->GetCrsrOfst( aPam.GetPoint(), aPt );

            const SwStartNode* pStartFly = aPam.GetPoint()->nNode.GetNode().FindFlyStartNode();
            int nNbClicks = 1;
            if ( pStartFly && !rSh.IsHeaderFooterEdit() )
                nNbClicks = 2;

            if ( nClicks == nNbClicks )
            {
                rSh.SwCrsrShell::SetCrsr( rDocPos );
                bRet = false;
            }
        }
        return bRet;
    }
}

class SwAnchorMarker
{
    SdrHdl* pHdl;
    Point aHdlPos;
    Point aLastPos;
    bool bTopRightHandle;
public:
    SwAnchorMarker( SdrHdl* pH )
        : pHdl( pH )
        , aHdlPos( pH->GetPos() )
        , aLastPos( pH->GetPos() )
        , bTopRightHandle( pH->GetKind() == HDL_ANCHOR_TR )
    {}
    const Point& GetLastPos() const { return aLastPos; }
    void SetLastPos( const Point& rNew ) { aLastPos = rNew; }
    void SetPos( const Point& rNew ) { pHdl->SetPos( rNew ); }
    const Point& GetPos() { return pHdl->GetPos(); }
    const Point& GetHdlPos() { return aHdlPos; }
    void ChgHdl( SdrHdl* pNew )
    {
        pHdl = pNew;
        if ( pHdl )
        {
            bTopRightHandle = (pHdl->GetKind() == HDL_ANCHOR_TR);
        }
    }
    const Point GetPosForHitTest( const OutputDevice& rOut )
    {
        Point aHitTestPos( GetPos() );
        aHitTestPos = rOut.LogicToPixel( aHitTestPos );
        if ( bTopRightHandle )
        {
            aHitTestPos += Point( -1, 1 );
        }
        else
        {
            aHitTestPos += Point( 1, 1 );
        }
        aHitTestPos = rOut.PixelToLogic( aHitTestPos );

        return aHitTestPos;
    }
};

/// Assists with auto-completion of AutoComplete words and AutoText names.
struct QuickHelpData
{
    /// Strings that at least partially match an input word.
    std::vector<String> m_aHelpStrings;
    /// Index of the current help string.
    sal_uInt16 nCurArrPos;
    /// Length of the input word associated with the help data.
    sal_uInt16 nLen;

    /// Help data stores AutoText names rather than AutoComplete words.
    bool m_bIsAutoText;
    /// Display help string as a tip rather than inline.
    bool m_bIsTip;
    /// Tip ID when a help string is displayed as a tip.
    sal_uLong nTipId;
    /// Append a space character to the displayed help string (if appropriate).
    bool m_bAppendSpace;

    /// Help string is currently displayed.
    bool m_bIsDisplayed;

    QuickHelpData() { ClearCntnt(); }

    void Move( QuickHelpData& rCpy );
    void ClearCntnt();
    void Start( SwWrtShell& rSh, sal_uInt16 nWrdLen );
    void Stop( SwWrtShell& rSh );

    sal_Bool HasCntnt() const { return !m_aHelpStrings.empty() && 0 != nLen; }

    /// Next help string.
    void Next( sal_Bool bEndLess )
    {
        if( ++nCurArrPos >= m_aHelpStrings.size() )
            nCurArrPos = (bEndLess && !m_bIsAutoText ) ? 0 : nCurArrPos-1;
    }
    /// Previous help string.
    void Previous( sal_Bool bEndLess )
    {
        if( 0 == nCurArrPos-- )
            nCurArrPos = (bEndLess && !m_bIsAutoText ) ? m_aHelpStrings.size()-1 : 0;
    }

    // Fills internal structures with hopefully helpful information.
    void FillStrArr( SwWrtShell& rSh, const String& rWord, bool bIgnoreCurrentPos );
    void SortAndFilter();
};

/*--------------------------------------------------------------------
    Description:   avoid minimal movement shiver
 --------------------------------------------------------------------*/

#define HIT_PIX  2 /* hit tolerance in pixel  */
#define MIN_MOVE 4

inline sal_Bool IsMinMove(const Point &rStartPos, const Point &rLPt)
{
    return Abs(rStartPos.X() - rLPt.X()) > MIN_MOVE ||
           Abs(rStartPos.Y() - rLPt.Y()) > MIN_MOVE;
}

/*--------------------------------------------------------------------
                for MouseButtonDown - determine whether a DrawObject
                an NO SwgFrame was hit! Shift/Ctrl should only result
                in selecting, with DrawObjects; at SwgFlys to trigger
                hyperlinks if applicable (DownLoad/NewWindow!)
 --------------------------------------------------------------------*/
inline sal_Bool IsDrawObjSelectable( const SwWrtShell& rSh, const Point& rPt )
{
    sal_Bool bRet = sal_True;
    SdrObject* pObj;
    switch( rSh.GetObjCntType( rPt, pObj ))
    {
    case OBJCNT_NONE:
    case OBJCNT_FLY:
    case OBJCNT_GRF:
    case OBJCNT_OLE:
        bRet = sal_False;
        break;
    default:; //prevent warning
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Description:   switch pointer
 --------------------------------------------------------------------*/

void SwEditWin::UpdatePointer(const Point &rLPt, sal_uInt16 nModifier )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    if( pApplyTempl )
    {
        PointerStyle eStyle = POINTER_FILL;
        if( rSh.IsOverReadOnlyPos( rLPt ))
        {
            delete pUserMarker;
            pUserMarker = 0L;

            eStyle = POINTER_NOTALLOWED;
        }
        else
        {
            SwRect aRect;
            SwRect* pRect = &aRect;
            const SwFrmFmt* pFmt = 0;

            bool bFrameIsValidTarget = false;
            if( pApplyTempl->pFormatClipboard )
                bFrameIsValidTarget = pApplyTempl->pFormatClipboard->HasContentForThisType( nsSelectionType::SEL_FRM );
            else if( !pApplyTempl->nColor )
                bFrameIsValidTarget = ( pApplyTempl->eType == SFX_STYLE_FAMILY_FRAME );

            if( bFrameIsValidTarget &&
                        0 !=(pFmt = rSh.GetFmtFromObj( rLPt, &pRect )) &&
                        PTR_CAST(SwFlyFrmFmt, pFmt))
            {
                //turn on highlight for frame
                Rectangle aTmp( pRect->SVRect() );

                if ( !pUserMarker )
                {
                    pUserMarker = new SdrDropMarkerOverlay( *rSh.GetDrawView(), aTmp );
                }
            }
            else
            {
                delete pUserMarker;
                pUserMarker = 0L;
            }

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
            {
                pUserMarker = new SdrDropMarkerOverlay( *rSh.GetDrawView(), aTmp );
            }
        }
        else
        {
            delete pUserMarker;
            pUserMarker = 0L;
        }

        SetPointer( eStyle );
        return;
    }

    sal_Bool bExecHyperlinks = rView.GetDocShell()->IsReadOnly();
    if ( !bExecHyperlinks )
    {
        SvtSecurityOptions aSecOpts;
        const sal_Bool bSecureOption = aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );
        if ( (  bSecureOption && nModifier == KEY_MOD1 ) ||
             ( !bSecureOption && nModifier != KEY_MOD1 ) )
            bExecHyperlinks = sal_True;
    }

    const sal_Bool bExecSmarttags  = nModifier == KEY_MOD1;

    SdrView *pSdrView = rSh.GetDrawView();
    sal_Bool bPrefSdrPointer = sal_False;
    sal_Bool bHitHandle = sal_False;
    sal_Bool bCntAtPos = sal_False;
    sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                          rSh.IsCrsrReadonly();
    aActHitType = SDRHIT_NONE;
    PointerStyle eStyle = POINTER_TEXT;
    if ( !pSdrView )
        bCntAtPos = sal_True;
    else if ( sal_True == (bHitHandle = pSdrView->PickHandle( rLPt ) != 0) )
    {
        aActHitType = SDRHIT_OBJECT;
        bPrefSdrPointer = sal_True;
    }
    else
    {
        const sal_Bool bNotInSelObj = !rSh.IsInsideSelectedObj( rLPt );
        if ( rView.GetDrawFuncPtr() && !bInsDraw && bNotInSelObj )
        {
            aActHitType = SDRHIT_OBJECT;
            if (IsObjectSelect())
                eStyle = POINTER_ARROW;
            else
                bPrefSdrPointer = sal_True;
        }
        else
        {
            SdrObject* pObj; SdrPageView* pPV;
            pSdrView->SetHitTolerancePixel( HIT_PIX );
            if ( bNotInSelObj && bExecHyperlinks &&
                 pSdrView->PickObj( rLPt, pSdrView->getHitTolLog(), pObj, pPV, SDRSEARCH_PICKMACRO ))
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
                        bPrefSdrPointer = sal_True;
                    }
                    else
                    {
                        SdrViewEvent aVEvt;
                        SdrHitKind eHit = pSdrView->PickAnything(rLPt, aVEvt);

                        if (eHit == SDRHIT_URLFIELD && bExecHyperlinks)
                        {
                            aActHitType = SDRHIT_OBJECT;
                            bPrefSdrPointer = sal_True;
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

                            SdrObject* pSelectableObj = rSh.GetObjAt(rLPt);
                            // Don't update pointer if this is a background image only.
                            if (pSelectableObj->GetLayer() != rSh.GetDoc()->GetHellId())
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
                            bPrefSdrPointer = sal_True;
                        else
                            bCntAtPos = sal_True;
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
        else
        {
            // Even if we already have something, prefer URLs if possible.
            SwContentAtPos aUrlPos(SwContentAtPos::SW_INETATTR);
            if (bCntAtPos || rSh.GetContentAtPos(rLPt, aUrlPos))
            {
                SwContentAtPos aSwContentAtPos(
                    SwContentAtPos::SW_CLICKFIELD|
                    SwContentAtPos::SW_INETATTR|
                    SwContentAtPos::SW_FTN |
                    SwContentAtPos::SW_SMARTTAG );
                if( rSh.GetContentAtPos( rLPt, aSwContentAtPos) )
                {
                    const bool bClickToFollow = SwContentAtPos::SW_INETATTR == aSwContentAtPos.eCntntAtPos ||
                                                SwContentAtPos::SW_SMARTTAG == aSwContentAtPos.eCntntAtPos;

                     if( !bClickToFollow ||
                         (SwContentAtPos::SW_INETATTR == aSwContentAtPos.eCntntAtPos && bExecHyperlinks) ||
                         (SwContentAtPos::SW_SMARTTAG == aSwContentAtPos.eCntntAtPos && bExecSmarttags) )
                        eStyle = POINTER_REFHAND;
                }
            }
        }

        // which kind of text pointer have we to show - horz / vert - ?
        if( POINTER_TEXT == eStyle && rSh.IsInVerticalText( &rLPt ))
            eStyle = POINTER_TEXT_VERTICAL;

        SetPointer( eStyle );
    }
}

/*--------------------------------------------------------------------
    Description: increase timer for selection
 --------------------------------------------------------------------*/

IMPL_LINK_NOARG(SwEditWin, TimerHandler)
{
    DBG_PROFSTART(edithdl);

    SwWrtShell &rSh = rView.GetWrtShell();
    Point aModPt( aMovePos );
    const SwRect aOldVis( rSh.VisArea() );
    sal_Bool bDone = sal_False;

    if ( !rSh.VisArea().IsInside( aModPt ) )
    {
        if ( bInsDraw )
        {
            const int nMaxScroll = 40;
            rView.Scroll( Rectangle(aModPt,Size(1,1)), nMaxScroll, nMaxScroll);
            bDone = sal_True;
        }
        else if ( bFrmDrag )
        {
            rSh.Drag(&aModPt, false);
            bDone = sal_True;
        }
        if ( !bDone )
            aModPt = rSh.GetCntntPos( aModPt,aModPt.Y() > rSh.VisArea().Bottom() );
    }
    if ( !bDone && !(bFrmDrag || bInsDraw) )
    {
        if ( pRowColumnSelectionStart )
        {
            Point aPos( aModPt );
            rSh.SelectTableRowCol( *pRowColumnSelectionStart, &aPos, bIsRowDrag );
        }
        else
            rSh.SetCursor( &aModPt, false );

        // It can be that a "jump" over a table cannot be accomplished like
        // that. So we jump over the table by Up/Down here.
        const SwRect& rVisArea = rSh.VisArea();
        if( aOldVis == rVisArea && !rSh.IsStartOfDoc() && !rSh.IsEndOfDoc() )
        {
            // take the center point of VisArea to
            // decide in which direction the user want.
            if( aModPt.Y() < ( rVisArea.Top() + rVisArea.Height() / 2 ) )
                rSh.Up( sal_True, 1 );
            else
                rSh.Down( sal_True, 1 );
        }
    }

    aMovePos += rSh.VisArea().Pos() - aOldVis.Pos();
    JustifyAreaTimer();
    DBG_PROFSTOP(edithdl);
    return 0;
}

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
    aTimer.SetTimeout( Max( coMinLen, nTimeout - nDiff*2L) );
}

void SwEditWin::LeaveArea(const Point &rPos)
{
    aMovePos = rPos;
    JustifyAreaTimer();
    if( !aTimer.IsActive() )
        aTimer.Start();
    delete pShadCrsr, pShadCrsr = 0;
}

inline void SwEditWin::EnterArea()
{
    aTimer.Stop();
}

/*------------------------------------------------------------------------
 Description:  insert mode for frames
------------------------------------------------------------------------*/

void SwEditWin::InsFrm(sal_uInt16 nCols)
{
    StdDrawMode( OBJ_NONE, sal_False );
    bInsFrm = sal_True;
    nInsFrmColCount = nCols;
}

void SwEditWin::StdDrawMode( SdrObjKind eSdrObjectKind, sal_Bool bObjSelect )
{
    SetSdrDrawMode( eSdrObjectKind );

    if (bObjSelect)
        rView.SetDrawFuncPtr(new DrawSelection( &rView.GetWrtShell(), this, &rView ));
    else
        rView.SetDrawFuncPtr(new SwDrawBase( &rView.GetWrtShell(), this, &rView ));

    rView.SetSelDrawSlot();
    SetSdrDrawMode( eSdrObjectKind );
    if (bObjSelect)
        rView.GetDrawFuncPtr()->Activate( SID_OBJECT_SELECT );
    else
        rView.GetDrawFuncPtr()->Activate( sal::static_int_cast< sal_uInt16 >(eSdrObjectKind) );
    bInsFrm = sal_False;
    nInsFrmColCount = 1;
}

void SwEditWin::StopInsFrm()
{
    if (rView.GetDrawFuncPtr())
    {
        rView.GetDrawFuncPtr()->Deactivate();
        rView.SetDrawFuncPtr(NULL);
    }
    rView.LeaveDrawCreate();    // leave construction mode
    bInsFrm = sal_False;
    nInsFrmColCount = 1;
}



sal_Bool SwEditWin::IsInputSequenceCheckingRequired( const String &rText, const SwPaM& rCrsr ) const
{
    const SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();
    if ( !rCTLOptions.IsCTLFontEnabled() ||
         !rCTLOptions.IsCTLSequenceChecking() )
         return sal_False;

    const xub_StrLen nFirstPos = rCrsr.Start()->nContent.GetIndex();
    if ( 0 == nFirstPos ) /* first char needs not to be checked */
        return sal_False;

    SwBreakIt *pBreakIter = SwBreakIt::Get();
    uno::Reference < i18n::XBreakIterator > xBI = pBreakIter->GetBreakIter();
    long nCTLScriptPos = -1;

    if (xBI.is())
    {
        if (xBI->getScriptType( rText, 0 ) == i18n::ScriptType::COMPLEX)
            nCTLScriptPos = 0;
        else
            nCTLScriptPos = xBI->nextScript( rText, 0, i18n::ScriptType::COMPLEX );
    }

    return (0 <= nCTLScriptPos && nCTLScriptPos <= rText.Len());
}

//return INVALID_HINT if language should not be explictly overridden, the correct
//HintId to use for the eBufferLanguage otherwise
static sal_uInt16 lcl_isNonDefaultLanguage(LanguageType eBufferLanguage, SwView& rView,
    const String &rInBuffer)
{
    sal_uInt16 nWhich = INVALID_HINT;
    bool bLang = true;
    if(eBufferLanguage != LANGUAGE_DONTKNOW)
    {
        switch( GetI18NScriptTypeOfLanguage( eBufferLanguage ))
        {
            case  i18n::ScriptType::ASIAN:     nWhich = RES_CHRATR_CJK_LANGUAGE; break;
            case  i18n::ScriptType::COMPLEX:   nWhich = RES_CHRATR_CTL_LANGUAGE; break;
            case  i18n::ScriptType::LATIN:     nWhich = RES_CHRATR_LANGUAGE; break;
            default: bLang = false;
        }
        if(bLang)
        {
            SfxItemSet aLangSet(rView.GetPool(), nWhich, nWhich);
            SwWrtShell& rSh = rView.GetWrtShell();
            rSh.GetCurAttr(aLangSet);
            if(SFX_ITEM_DEFAULT <= aLangSet.GetItemState(nWhich, sal_True))
            {
                LanguageType eLang = static_cast<const SvxLanguageItem&>(aLangSet.Get(nWhich)).GetLanguage();
                if ( eLang == eBufferLanguage )
                {
                    // current language attribute equal to language reported from system
                    bLang = false;
                }
                else if ( !bInputLanguageSwitched && RES_CHRATR_LANGUAGE == nWhich )
                {
                    // special case: switching between two "LATIN" languages
                    // In case the current keyboard setting might be suitable
                    // for both languages we can't safely assume that the user
                    // wants to use the language reported from the system,
                    // except if we knew that it was explicitly switched (thus
                    // the check for "bInputLangeSwitched").
                    //
                    // The language reported by the system could be just the
                    // system default language that the user is not even aware
                    // of, because no language selection tool is installed at
                    // all. In this case the OOo language should get preference
                    // as it might have been selected by the user explicitly.
                    //
                    // Usually this case happens if the OOo language is
                    // different to the system language but the system keyboard
                    // is still suitable for the OOo language (e.g. writing
                    // English texts with a German keyboard).
                    //
                    // For non-latin keyboards overwriting the attribute is
                    // still valid. We do this for kyrillic and greek ATM.  In
                    // future versions of OOo this should be replaced by a
                    // configuration switch that allows to give the preference
                    // to the OOo setting or the system setting explicitly
                    // and/or a better handling of the script type.
                    i18n::UnicodeScript eType = rInBuffer.Len() ?
                        (i18n::UnicodeScript)GetAppCharClass().getScript( rInBuffer, 0 ) :
                        i18n::UnicodeScript_kScriptCount;

                    bool bSystemIsNonLatin = false, bOOoLangIsNonLatin = false;
                    switch ( eType )
                    {
                        case i18n::UnicodeScript_kGreek:
                        case i18n::UnicodeScript_kCyrillic:
                            // in case other UnicodeScripts require special
                            // keyboards they can be added here
                            bSystemIsNonLatin = true;
                            break;
                        default:
                            break;
                    }

                    switch ( eLang )
                    {
                        case LANGUAGE_AZERI_CYRILLIC:
                        case LANGUAGE_BOSNIAN_CYRILLIC_BOSNIA_HERZEGOVINA:
                        case LANGUAGE_BULGARIAN:
                        case LANGUAGE_GREEK:
                        case LANGUAGE_RUSSIAN:
                        case LANGUAGE_RUSSIAN_MOLDOVA:
                        case LANGUAGE_SERBIAN_CYRILLIC:
                        case LANGUAGE_SERBIAN_CYRILLIC_BOSNIA_HERZEGOVINA:
                        case LANGUAGE_UZBEK_CYRILLIC:
                        case LANGUAGE_UKRAINIAN:
                        case LANGUAGE_BELARUSIAN:
                            bOOoLangIsNonLatin = true;
                            break;
                        default:
                            break;
                    }

                    bLang = (bSystemIsNonLatin != bOOoLangIsNonLatin);
                }
            }
        }
    }
    return bLang ? nWhich : INVALID_HINT;
}

/*--------------------------------------------------------------------
     Description:  character buffer is inserted into the document
 --------------------------------------------------------------------*/

void SwEditWin::FlushInBuffer()
{
    if ( aInBuffer.Len() )
    {
        SwWrtShell& rSh = rView.GetWrtShell();

        // generate new sequence input checker if not already done
        if ( !pCheckIt )
            pCheckIt = new SwCheckIt;

        uno::Reference < i18n::XExtendedInputSequenceChecker > xISC = pCheckIt->xCheck;
        if ( xISC.is() && IsInputSequenceCheckingRequired( aInBuffer, *rSh.GetCrsr() ) )
        {
            //
            // apply (Thai) input sequence checking/correction
            //

            rSh.Push(); // push current cursor to stack

            // get text from the beginning (i.e left side) of current selection
            // to the start of the paragraph
            rSh.NormalizePam();     // make point be the first (left) one
            if (!rSh.GetCrsr()->HasMark())
                rSh.GetCrsr()->SetMark();
            rSh.GetCrsr()->GetMark()->nContent = 0;
            String aLeftText( rSh.GetCrsr()->GetTxt() );

            SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();

            xub_StrLen nExpandSelection = 0;
            if (aLeftText.Len() > 0)
            {
                sal_Unicode cChar = '\0';

                xub_StrLen nTmpPos = aLeftText.Len();
                sal_Int16 nCheckMode = rCTLOptions.IsCTLSequenceCheckingRestricted() ?
                        i18n::InputSequenceCheckMode::STRICT : i18n::InputSequenceCheckMode::BASIC;

                rtl::OUString aOldText( aLeftText );
                rtl::OUString aNewText( aOldText );
                if (rCTLOptions.IsCTLSequenceCheckingTypeAndReplace())
                {
                    for (xub_StrLen k = 0;  k < aInBuffer.Len();  ++k)
                    {
                        cChar = aInBuffer.GetChar(k);
                        const xub_StrLen nPrevPos = static_cast<xub_StrLen>(xISC->correctInputSequence( aNewText, nTmpPos - 1, cChar, nCheckMode ));

                        // valid sequence or sequence could be corrected:
                        if (nPrevPos != aNewText.getLength())
                            nTmpPos = nPrevPos + 1;
                    }

                    // find position of first character that has changed
                    sal_Int32 nOldLen = aOldText.getLength();
                    sal_Int32 nNewLen = aNewText.getLength();
                    const sal_Unicode *pOldTxt = aOldText.getStr();
                    const sal_Unicode *pNewTxt = aNewText.getStr();
                    sal_Int32 nChgPos = 0;
                    while ( nChgPos < nOldLen && nChgPos < nNewLen &&
                            pOldTxt[nChgPos] == pNewTxt[nChgPos] )
                        ++nChgPos;

                    xub_StrLen nChgLen = static_cast< xub_StrLen >(nNewLen - nChgPos);
                    String aChgText( aNewText.copy( static_cast< xub_StrLen >(nChgPos), nChgLen ) );

                    if (aChgText.Len())
                    {
                        aInBuffer = aChgText;
                        nExpandSelection = aLeftText.Len() - static_cast< xub_StrLen >(nChgPos);
                    }
                    else
                        aInBuffer.Erase();
                }
                else
                {
                    for (xub_StrLen k = 0;  k < aInBuffer.Len();  ++k)
                    {
                        cChar = aInBuffer.GetChar(k);
                        if (xISC->checkInputSequence( aNewText, nTmpPos - 1, cChar, nCheckMode ))
                        {
                            // character can be inserted:
                            aNewText += rtl::OUString( (sal_Unicode) cChar );
                            ++nTmpPos;
                        }
                    }
                    aInBuffer = aNewText.copy( aOldText.getLength() );  // copy new text to be inserted to buffer
                }
            }

            // at this point now we will insert the buffer text 'normally' some lines below...

            rSh.Pop( sal_False );  // pop old cursor from stack

            if (!aInBuffer.Len())
                return;

            // if text prior to the original selection needs to be changed
            // as well, we now expand the selection accordingly.
            SwPaM &rCrsr = *rSh.GetCrsr();
            xub_StrLen nCrsrStartPos = rCrsr.Start()->nContent.GetIndex();
            OSL_ENSURE( nCrsrStartPos >= nExpandSelection, "cannot expand selection as specified!!" );
            if (nExpandSelection && nCrsrStartPos >= nExpandSelection)
            {
                if (!rCrsr.HasMark())
                    rCrsr.SetMark();
                rCrsr.Start()->nContent -= nExpandSelection;
            }
        }

        uno::Reference< frame::XDispatchRecorder > xRecorder =
                rView.GetViewFrame()->GetBindings().GetRecorder();
        if ( xRecorder.is() )
        {
            // determine shell
            SfxShell *pSfxShell = lcl_GetShellFromDispatcher( rView, TYPE(SwTextShell) );
            // generate request and record
            if (pSfxShell)
            {
                SfxRequest aReq( rView.GetViewFrame(), FN_INSERT_STRING );
                aReq.AppendItem( SfxStringItem( FN_INSERT_STRING, aInBuffer ) );
                aReq.Done();
            }
        }

        sal_uInt16 nWhich = lcl_isNonDefaultLanguage(eBufferLanguage, rView, aInBuffer);
        if (nWhich != INVALID_HINT )
        {
            SvxLanguageItem aLangItem( eBufferLanguage, nWhich );
            rSh.SetAttr( aLangItem );
        }

        rSh.Insert( aInBuffer );
        eBufferLanguage = LANGUAGE_DONTKNOW;
        aInBuffer.Erase();
        bFlushCharBuffer = sal_False;
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

void SwEditWin::ChangeFly( sal_uInt8 nDir, sal_Bool bWeb )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SwRect aTmp = rSh.GetFlyRect();
    if( aTmp.HasArea() &&
        // IsSelObjProtected() seems to be the correct condition, not
        // !HasReadonlySel(), otherwise frame is not moveable if content is
        // protected.
        !rSh.IsSelObjProtected( FLYPROTECT_POS|FLYPROTECT_SIZE ) )
    {
        SfxItemSet aSet(rSh.GetAttrPool(),
                        RES_FRM_SIZE, RES_FRM_SIZE,
                        RES_VERT_ORIENT, RES_ANCHOR,
                        RES_COL, RES_COL,
                        RES_PROTECT, RES_PROTECT,
                        RES_FOLLOW_TEXT_FLOW, RES_FOLLOW_TEXT_FLOW, 0);
        rSh.GetFlyFrmAttr( aSet );
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
                aSnap.Width() = Max( (sal_uLong)1, (sal_uLong)aSnap.Width() / nDiv );
            nDiv = rSh.GetViewOptions()->GetDivisionY();
            if ( nDiv > 0 )
                aSnap.Height() = Max( (sal_uLong)1, (sal_uLong)aSnap.Height() / nDiv );
        }

        SwRect aBoundRect;
        Point aRefPoint;
        // adjustment for allowing vertical position
        // aligned to page for fly frame anchored to paragraph or to character.
        {
            SwFmtVertOrient aVert( (SwFmtVertOrient&)aSet.Get(RES_VERT_ORIENT) );
            const bool bFollowTextFlow =
                    static_cast<const SwFmtFollowTextFlow&>(aSet.Get(RES_FOLLOW_TEXT_FLOW)).GetValue();
            const SwPosition* pToCharCntntPos = ((SwFmtAnchor&)aSet.Get(RES_ANCHOR)).GetCntntAnchor();
            rSh.CalcBoundRect( aBoundRect, eAnchorId,
                               text::RelOrientation::FRAME, aVert.GetRelationOrient(),
                               pToCharCntntPos, bFollowTextFlow,
                               false, &aRefPoint );
        }
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
                    nRight = aSnap.Width(); // no break
            case MOVE_RIGHT_BIG: aTmp.Left( aTmp.Left() + nRight ); break;
            case MOVE_DOWN_SMALL: if( aTmp.Height() < aSnap.Height() + MINFLY ) break;
                    nDown = aSnap.Height(); // no break
            case MOVE_DOWN_BIG: aTmp.Top( aTmp.Top() + nDown ); break;
            default: OSL_ENSURE(true, "ChangeFly: Unknown direction." );
        }
        sal_Bool bSet = sal_False;
        if ((FLY_AS_CHAR == eAnchorId) && ( nDir % 2 ))
        {
            long aDiff = aTmp.Top() - aRefPoint.Y();
            if( aDiff > 0 )
                aDiff = 0;
            else if ( aDiff < -aTmp.Height() )
                aDiff = -aTmp.Height();
            SwFmtVertOrient aVert( (SwFmtVertOrient&)aSet.Get(RES_VERT_ORIENT) );
            sal_Int16 eNew;
            if( bWeb )
            {
                eNew = aVert.GetVertOrient();
                sal_Bool bDown = 0 != ( nDir & 0x02 );
                switch( eNew )
                {
                    case text::VertOrientation::CHAR_TOP:
                        if( bDown ) eNew = text::VertOrientation::CENTER;
                    break;
                    case text::VertOrientation::CENTER:
                        eNew = bDown ? text::VertOrientation::TOP : text::VertOrientation::CHAR_TOP;
                    break;
                    case text::VertOrientation::TOP:
                        if( !bDown ) eNew = text::VertOrientation::CENTER;
                    break;
                    case text::VertOrientation::LINE_TOP:
                        if( bDown ) eNew = text::VertOrientation::LINE_CENTER;
                    break;
                    case text::VertOrientation::LINE_CENTER:
                        eNew = bDown ? text::VertOrientation::LINE_BOTTOM : text::VertOrientation::LINE_TOP;
                    break;
                    case text::VertOrientation::LINE_BOTTOM:
                        if( !bDown ) eNew = text::VertOrientation::LINE_CENTER;
                    break;
                    default:; //prevent warning
                }
            }
            else
            {
                aVert.SetPos( aDiff );
                eNew = text::VertOrientation::NONE;
            }
            aVert.SetVertOrient( eNew );
            aSet.Put( aVert );
            bSet = sal_True;
        }
        if (bWeb && (FLY_AT_PARA == eAnchorId)
            && ( nDir==MOVE_LEFT_SMALL || nDir==MOVE_RIGHT_BIG ))
        {
            SwFmtHoriOrient aHori( (SwFmtHoriOrient&)aSet.Get(RES_HORI_ORIENT) );
            sal_Int16 eNew;
            eNew = aHori.GetHoriOrient();
            switch( eNew )
            {
                case text::HoriOrientation::RIGHT:
                    if( nDir==MOVE_LEFT_SMALL )
                        eNew = text::HoriOrientation::LEFT;
                break;
                case text::HoriOrientation::LEFT:
                    if( nDir==MOVE_RIGHT_BIG )
                        eNew = text::HoriOrientation::RIGHT;
                break;
                default:; //prevent warning
            }
            if( eNew != aHori.GetHoriOrient() )
            {
                aHori.SetHoriOrient( eNew );
                aSet.Put( aHori );
                bSet = sal_True;
            }
        }
        rSh.StartAllAction();
        if( bSet )
            rSh.SetFlyFrmAttr( aSet );
        sal_Bool bSetPos = (FLY_AS_CHAR != eAnchorId);
        if(bSetPos && bWeb)
        {
            if (FLY_AT_PAGE != eAnchorId)
            {
                bSetPos = sal_False;
            }
            else
            {
                bSetPos = sal_True;
            }
        }
        if( bSetPos )
            rSh.SetFlyPos( aTmp.Pos() );
        rSh.EndAllAction();
    }
}

void SwEditWin::ChangeDrawing( sal_uInt8 nDir )
{
    // start undo action in order to get only one
    // undo action for this change.
    SwWrtShell &rSh = rView.GetWrtShell();
    rSh.StartUndo();

    long nX = 0;
    long nY = 0;
    sal_Bool bOnePixel = sal_False;
    sal_uInt16 nAnchorDir = SW_MOVE_UP;
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
        sal_uInt8 nProtect = rSh.IsSelObjProtected( FLYPROTECT_POS|FLYPROTECT_SIZE );
        Size aSnap( rSh.GetViewOptions()->GetSnapSize() );
        short nDiv = rSh.GetViewOptions()->GetDivisionX();
        if ( nDiv > 0 )
            aSnap.Width() = Max( (sal_uLong)1, (sal_uLong)aSnap.Width() / nDiv );
        nDiv = rSh.GetViewOptions()->GetDivisionY();
        if ( nDiv > 0 )
            aSnap.Height() = Max( (sal_uLong)1, (sal_uLong)aSnap.Height() / nDiv );

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
                // Check if object is anchored as character and move direction
                sal_Bool bDummy1, bDummy2;
                const bool bVertAnchor = rSh.IsFrmVertical( sal_True, bDummy1, bDummy2 );
                bool bHoriMove = !bVertAnchor == !( nDir % 2 );
                bool bMoveAllowed =
                    !bHoriMove || (rSh.GetAnchorId() != FLY_AS_CHAR);
                if ( bMoveAllowed )
                {
                    pSdrView->MoveAllMarked(Size(nX, nY));
                    rSh.SetModified();
                }
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
                        sal_Bool bWasNoSnap = static_cast< sal_Bool >(rDragStat.IsNoSnap());
                        sal_Bool bWasSnapEnabled = pSdrView->IsSnapEnabled();

                        // switch snapping off
                        if(!bWasNoSnap)
                            ((SdrDragStat&)rDragStat).SetNoSnap(sal_True);
                        if(bWasSnapEnabled)
                            pSdrView->SetSnapEnabled(sal_False);

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

    rSh.EndUndo();
}

/*--------------------------------------------------------------------
    Description:   KeyEvents
 --------------------------------------------------------------------*/

void SwEditWin::KeyInput(const KeyEvent &rKEvt)
{
    SwWrtShell &rSh = rView.GetWrtShell();

    if( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE &&
        pApplyTempl && pApplyTempl->pFormatClipboard )
    {
        pApplyTempl->pFormatClipboard->Erase();
        SetApplyTemplate(SwApplyTemplate());
        rView.GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
    }
    else if ( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE &&
            rSh.IsHeaderFooterEdit( ) )
    {
        bool bHeader = FRMTYPE_HEADER & rSh.GetFrmType(0,sal_False);
        if ( bHeader )
            rSh.SttPg();
        else
            rSh.EndPg();
        rSh.ToggleHeaderFooterEdit();
    }

    SfxObjectShell *pObjSh = (SfxObjectShell*)rView.GetViewFrame()->GetObjectShell();
    if ( bLockInput || (pObjSh && pObjSh->GetProgress()) )
        // When the progress bar is active or a progress is
        // running on a document, no order is being taken
        return;

    delete pShadCrsr, pShadCrsr = 0;
    aKeyInputFlushTimer.Stop();

    sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                          rSh.IsCrsrReadonly();

    //if the language changes the buffer must be flushed
    LanguageType eNewLanguage = GetInputLanguage();
    if(!bIsDocReadOnly && eBufferLanguage != eNewLanguage && aInBuffer.Len())
    {
        FlushInBuffer();
    }
    eBufferLanguage = eNewLanguage;

    QuickHelpData aTmpQHD;
    if( pQuickHlpData->m_bIsDisplayed )
    {
        aTmpQHD.Move( *pQuickHlpData );
        pQuickHlpData->Stop( rSh );
    }

    // OS:the DrawView also needs a readonly-Flag as well
    if ( !bIsDocReadOnly && rSh.GetDrawView() && rSh.GetDrawView()->KeyInput( rKEvt, this ) )
    {
        rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll( sal_False );
        rSh.SetModified();
        return; // Event evaluated by SdrView
    }

    if ( rView.GetDrawFuncPtr() && bInsFrm )
    {
        StopInsFrm();
        rSh.Edit();
    }

    sal_Bool bFlushBuffer = sal_False;
    sal_Bool bNormalChar = sal_False;
    bool bAppendSpace = pQuickHlpData->m_bAppendSpace;
    pQuickHlpData->m_bAppendSpace = false;

    if ( getenv("SW_DEBUG") && rKEvt.GetKeyCode().GetCode() == KEY_F12 )
    {
        if( rKEvt.GetKeyCode().IsShift())
        {
            GetView().GetDocShell()->GetDoc()->dumpAsXml();
            return;
        }
        else
        {
            SwRootFrm* pLayout = GetView().GetDocShell()->GetWrtShell()->GetLayout();
            pLayout->dumpAsXml( );
            return;
        }
    }

    KeyEvent aKeyEvent( rKEvt );
    // look for vertical mappings
    if( !bIsDocReadOnly && !rSh.IsSelFrmMode() && !rSh.IsObjSelected() )
    {
        // must changed from switch to if, because the Linux
        // compiler has problem with the code. Has to remove if the new general
        // handler exist.
        sal_uInt16 nKey = rKEvt.GetKeyCode().GetCode();

        if( KEY_UP == nKey || KEY_DOWN == nKey ||
            KEY_LEFT == nKey || KEY_RIGHT == nKey )
        {
            // In general, we want to map the direction keys if we are inside
            // some vertical formatted text.
            // 1. Exception: For a table cursor in a horizontal table, the
            //               directions should never be mapped.
            // 2. Exception: For a table cursor in a vertical table, the
            //               directions should always be mapped.
            const bool bVertText = rSh.IsInVerticalText();
            const bool bTblCrsr = rSh.GetTableCrsr();
            const bool bVertTable = rSh.IsTableVertical();
            if( ( bVertText && ( !bTblCrsr || bVertTable ) ) ||
                ( bTblCrsr && bVertTable ) )
            {
                // Attempt to integrate cursor travelling for mongolian layout does not work.
                // Thus, back to previous mapping of cursor keys to direction keys.
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
    }

    const KeyCode& rKeyCode = aKeyEvent.GetKeyCode();
    sal_Unicode aCh = aKeyEvent.GetCharCode();

    // enable switching to notes ankor with Ctrl - Alt - Page Up/Down
    // pressing this inside a note will switch to next/previous note
    if ((rKeyCode.IsMod1() && rKeyCode.IsMod2()) && ((rKeyCode.GetCode() == KEY_PAGEUP) || (rKeyCode.GetCode() == KEY_PAGEDOWN)))
    {
        bool bNext = rKeyCode.GetCode()==KEY_PAGEDOWN ? true : false;
        SwFieldType* pFldType = rSh.GetFldType(0, RES_POSTITFLD);
        rSh.MoveFldType( pFldType, bNext );
        return;
    }

    const SwFrmFmt* pFlyFmt = rSh.GetFlyFrmFmt();
    if( pFlyFmt )
    {
        sal_uInt16 nEvent;

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
        ((nLclSelectionType & (nsSelectionType::SEL_FRM|nsSelectionType::SEL_GRF)) ||
        ((nLclSelectionType & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM)) &&
                rSh.GetDrawView()->GetMarkedObjectList().GetMarkCount() == 1))))
    {
        SdrHdlList& rHdlList = (SdrHdlList&)rSh.GetDrawView()->GetHdlList();
        SdrHdl* pAnchor = rHdlList.GetHdl(HDL_ANCHOR);
        if ( ! pAnchor )
            pAnchor = rHdlList.GetHdl(HDL_ANCHOR_TR);
        if(pAnchor)
            rHdlList.SetFocusHdl(pAnchor);
        return;
    }

    SvxAutoCorrCfg* pACfg = 0;
    SvxAutoCorrect* pACorr = 0;

    uno::Reference< frame::XDispatchRecorder > xRecorder =
            rView.GetViewFrame()->GetBindings().GetRecorder();
    if ( !xRecorder.is() )
    {
        pACfg = &SvxAutoCorrCfg::Get();
        pACorr = pACfg->GetAutoCorrect();
    }

    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    TblChgWidthHeightType eTblChgMode = nsTblChgWidthHeightType::WH_COL_LEFT;    // initialization just for warning-free code
    sal_uInt16 nTblChgSize = 0;
    sal_Bool bStopKeyInputTimer = sal_True;
    String sFmlEntry;

    enum SW_AutoCompleteAction { ACA_NoOp,            // No maintenance operation required for AutoComplete tree
                                 ACA_ReturnToRoot,    // Input of a char marking the end of a word, like '.', ' ', etc.
                                 ACA_SingleCharInput,
                                 ACA_SingleBackspace,
                                 ACA_Refresh };       // Refresh AutoComplete information completely.

    // Do refresh by default to gracefully handle all unimplemented special cases.
    SW_AutoCompleteAction eAutoCompleteAction = ACA_Refresh;
    // TODO: Make sure eAutoCompleteAction is set to something other than ACA_Refresh to increase performance.

    // Stores input char in case ACA_SingleCharInput is used. Defaults to aCh to avoid compiler warning.
    sal_Unicode aSingleCharInput = aCh;

    enum SW_KeyState { KS_Start,
                       KS_CheckKey, KS_InsChar, KS_InsTab,
                       KS_NoNum, KS_NumOff, KS_NumOrNoNum, KS_NumDown, KS_NumUp,
                       KS_NumIndentInc, KS_NumIndentDec,

                       KS_OutlineLvOff,
                       KS_NextCell, KS_PrevCell, KS_OutlineUp, KS_OutlineDown,
                       KS_GlossaryExpand, KS_NextPrevGlossary,
                       KS_AutoFmtByInput,
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
                       KS_GotoNextFieldMark,
                       KS_GotoPrevFieldMark,
                       KS_End };

    SW_KeyState eKeyState = bIsDocReadOnly ? KS_CheckDocReadOnlyKeys
                                           : KS_CheckKey,
                eNextKeyState = KS_End;
    sal_uInt8 nDir = 0;

    if (nKS_NUMDOWN_Count > 0)
        nKS_NUMDOWN_Count--;

    if (nKS_NUMINDENTINC_Count > 0)
        nKS_NUMINDENTINC_Count--;

    while( KS_End != eKeyState )
    {
        SW_KeyState eFlyState = KS_KeyToView;

        switch( eKeyState )
        {
        case KS_CheckKey:
            eKeyState = KS_KeyToView;       // default forward to View

#if OSL_DEBUG_LEVEL > 1
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // for switching curor behaviour in ReadOnly regions
            if( 0x7210 == rKeyCode.GetFullCode() )
                rSh.SetReadOnlyAvailable( !rSh.IsReadOnlyAvailable() );
            else
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#endif

            if( !rKeyCode.IsMod2() && '=' == aCh &&
                !rSh.IsTableMode() && rSh.GetTableFmt() &&
                rSh.IsSttPara() &&
                !rSh.HasReadonlySel() )
            {
                // at the beginning of the table's cell a '=' ->
                // call EditRow (F2-functionality)
                rSh.Push();
                if( !rSh.MoveSection( fnSectionCurr, fnSectionStart) &&
                    !rSh.IsTableBoxTextFormat() )
                {
                    // is at the beginning of the box
                    eKeyState = KS_EditFormula;
                    if( rSh.HasMark() )
                        rSh.SwapPam();
                    else
                        rSh.SttSelect();
                    rSh.MoveSection( fnSectionCurr, fnSectionEnd );
                    rSh.Pop( sal_True );
                    rSh.EndSelect();
                    sFmlEntry = '=';
                }
                else
                    rSh.Pop( sal_False );
            }
            else
            {
                if( pACorr && aTmpQHD.HasCntnt() && !rSh.HasSelection() &&
                    !rSh.HasReadonlySel() && !aTmpQHD.m_bIsAutoText &&
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
                                (rSh.GetSelectionType() & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM))  &&
                                    rSh.GetDrawView()->AreObjectsMarked())
                                eKeyState = KS_Draw_Change;
                        }

                        if( pFlyFmt )
                            eKeyState = eFlyState;
                        else if( KS_Draw_Change != eKeyState)
                            eKeyState = KS_EnterCharCell;
                    }
                    break;

                case KEY_LEFT:
                {
                    eAutoCompleteAction = ACA_ReturnToRoot;
                }
                // No break;
                case KEY_LEFT | KEY_MOD1:
                {
                    sal_Bool bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_LEFT_BIG;
                    }
                    eTblChgMode = nsTblChgWidthHeightType::WH_FLAG_INSDEL |
                            ( bMod1
                                ? nsTblChgWidthHeightType::WH_CELL_LEFT
                                : nsTblChgWidthHeightType::WH_COL_LEFT );
                    nTblChgSize = pModOpt->GetTblVInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_RIGHT | KEY_MOD1:
                {
                    eTblChgMode = nsTblChgWidthHeightType::WH_FLAG_INSDEL | nsTblChgWidthHeightType::WH_CELL_RIGHT;
                    nTblChgSize = pModOpt->GetTblVInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_UP:
                {
                    eAutoCompleteAction = ACA_ReturnToRoot;
                }
                // No break;
                case KEY_UP | KEY_MOD1:
                {
                    sal_Bool bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_UP_BIG;
                    }
                    eTblChgMode = nsTblChgWidthHeightType::WH_FLAG_INSDEL |
                            ( bMod1
                                ? nsTblChgWidthHeightType::WH_CELL_TOP
                                : nsTblChgWidthHeightType::WH_ROW_TOP );
                    nTblChgSize = pModOpt->GetTblHInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_DOWN:
                {
                    eAutoCompleteAction = ACA_ReturnToRoot;
                }
                // No break;
                case KEY_DOWN | KEY_MOD1:
                {
                    sal_Bool bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_DOWN_BIG;
                    }
                    eTblChgMode = nsTblChgWidthHeightType::WH_FLAG_INSDEL |
                            ( bMod1
                                ? nsTblChgWidthHeightType::WH_CELL_BOTTOM
                                : nsTblChgWidthHeightType::WH_ROW_BOTTOM );
                    nTblChgSize = pModOpt->GetTblHInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;

KEYINPUT_CHECKTABLE_INSDEL:
                    if( rSh.IsTableMode() || !rSh.GetTableFmt() ||
                        !bTblInsDelMode ||
                        sal_False /* table protected */
                            )
                    {
                        const int nSelectionType = rSh.GetSelectionType();

                        eKeyState = KS_KeyToView;
                        if(KS_KeyToView != eFlyState)
                        {
                            if((nSelectionType & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM))  &&
                                    rSh.GetDrawView()->AreObjectsMarked())
                                eKeyState = KS_Draw_Change;
                            else if(nSelectionType & (nsSelectionType::SEL_FRM|nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF))
                                eKeyState = KS_Fly_Change;
                        }
                    }
                    else
                    {
                        if( !bTblIsInsMode )
                            eTblChgMode = eTblChgMode | nsTblChgWidthHeightType::WH_FLAG_BIGGER;
                        eKeyState = KS_TblColCellInsDel;
                    }
                    break;

                case KEY_DELETE:
                    if ( !rSh.HasReadonlySel() )
                    {
                        if (rSh.IsInFrontOfLabel() && rSh.NumOrNoNum(sal_False))
                            eKeyState = KS_NumOrNoNum;
                        eAutoCompleteAction = ACA_NoOp;
                    }
                    else
                    {
                        InfoBox( this, SW_RES( MSG_READONLY_CONTENT )).Execute();
                        eKeyState = KS_End;
                    }
                    break;

                case KEY_DELETE | KEY_MOD2:
                    if( !rSh.IsTableMode() && rSh.GetTableFmt() )
                    {
                        eKeyState = KS_End;
                        bTblInsDelMode = sal_True;
                        bTblIsInsMode = sal_False;
                        bTblIsColMode = sal_True;
                        aKeyInputTimer.Start();
                        bStopKeyInputTimer = sal_False;
                        eAutoCompleteAction = ACA_NoOp;
                    }
                    break;
                case KEY_INSERT | KEY_MOD2:
                    if( !rSh.IsTableMode() && rSh.GetTableFmt() )
                    {
                        eKeyState = KS_End;
                        bTblInsDelMode = sal_True;
                        bTblIsInsMode = sal_True;
                        bTblIsColMode = sal_True;
                        aKeyInputTimer.Start();
                        bStopKeyInputTimer = sal_False;
                        eAutoCompleteAction = ACA_NoOp;
                    }
                    break;

                case KEY_RETURN:
                {
                    if( !rSh.HasReadonlySel() )
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        if(nSelectionType & nsSelectionType::SEL_OLE)
                            eKeyState = KS_LaunchOLEObject;
                        else if(nSelectionType & nsSelectionType::SEL_FRM)
                            eKeyState = KS_GoIntoFly;
                        else if((nSelectionType & nsSelectionType::SEL_DRW) &&
                                0 == (nSelectionType & nsSelectionType::SEL_DRW_TXT) &&
                                rSh.GetDrawView()->GetMarkedObjectList().GetMarkCount() == 1)
                            eKeyState = KS_GoIntoDrawing;
                        else if( aTmpQHD.HasCntnt() && !rSh.HasSelection() &&
                            aTmpQHD.m_bIsAutoText )
                            eKeyState = KS_GlossaryExpand;

                        //RETURN and empty paragraph in numbering -> end numbering
                        else if( !aInBuffer.Len() &&
                                 rSh.GetCurNumRule() &&
                                 !rSh.GetCurNumRule()->IsOutlineRule() &&
                                 !rSh.HasSelection() &&
                                rSh.IsSttPara() && rSh.IsEndPara() )
                            eKeyState = KS_NumOff, eNextKeyState = KS_OutlineLvOff;

                        //RETURN for new paragraph with AutoFormating
                        else if( pACfg && pACfg->IsAutoFmtByInput() &&
                                !(nSelectionType & (nsSelectionType::SEL_GRF |
                                    nsSelectionType::SEL_OLE | nsSelectionType::SEL_FRM |
                                    nsSelectionType::SEL_TBL_CELLS | nsSelectionType::SEL_DRW |
                                    nsSelectionType::SEL_DRW_TXT)) )
                            eKeyState = KS_CheckAutoCorrect, eNextKeyState = KS_AutoFmtByInput;
                        else
                            eNextKeyState = eKeyState, eKeyState = KS_CheckAutoCorrect;
                    }
                    eAutoCompleteAction = ACA_ReturnToRoot;
                }
                break;
                case KEY_RETURN | KEY_MOD2:     // ALT-Return
                {
                    if( !rSh.HasReadonlySel() && !rSh.IsSttPara() && rSh.GetCurNumRule() )
                        eKeyState = KS_NoNum;
                    else if( rSh.CanSpecialInsert() )
                        eKeyState = KS_SpecialInsert;
                    eAutoCompleteAction = ACA_ReturnToRoot;
                }
                break;
                case KEY_BACKSPACE:
                case KEY_BACKSPACE | KEY_SHIFT:
                    if( !rSh.HasReadonlySel() )
                    {
                        sal_Bool bDone = sal_False;
                        // try to add comment for code snip:
                        // Remove the paragraph indent, if the cursor is at the
                        // beginning of a paragraph, there is no selection
                        // and no numbering rule found at the current paragraph
                        // Also try to remove indent, if current paragraph
                        // has numbering rule, but isn't counted and only
                        // key <backspace> is hit.
                        const bool bOnlyBackspaceKey(
                                    KEY_BACKSPACE == rKeyCode.GetFullCode() );
                        if ( rSh.IsSttPara() &&
                             !rSh.HasSelection() &&
                             ( NULL == rSh.GetCurNumRule() ||
                               ( rSh.IsNoNum() && bOnlyBackspaceKey ) ) )
                        {
                            bDone = rSh.TryRemoveIndent();
                        }

                        if ( bOnlyBackspaceKey )
                        {
                            eAutoCompleteAction = ACA_SingleBackspace;
                        }

                        if (bDone)
                            eKeyState = KS_End;
                        else
                        {
                            if (rSh.IsSttPara() &&
                                ! rSh.IsNoNum())
                            {
                                if (nKS_NUMDOWN_Count > 0 &&
                                    0 < rSh.GetNumLevel())
                                {
                                    eKeyState = KS_NumUp;
                                    nKS_NUMDOWN_Count = 2;
                                    bDone = sal_True;
                                }
                                else if (nKS_NUMINDENTINC_Count > 0)
                                {
                                    eKeyState = KS_NumIndentDec;
                                    nKS_NUMINDENTINC_Count = 2;
                                    bDone = sal_True;
                                }
                            }

                            // If the cursor is in an empty paragraph, which has
                            // a numbering, but not the oultine numbering, and
                            // there is no selection, the numbering has to be
                            // deleted on key <Backspace>.
                            // Otherwise method <SwEditShell::NumOrNoNum(..)>
                            // should only change the <IsCounted()> state of
                            // the current paragraph depending of the key.
                            // On <backspace> it is set to <false>,
                            // on <shift-backspace> it is set to <true>.
                            // Thus, assure that method <SwEditShell::NumOrNum(..)>
                            // is only called for the intended purpose.
                            bool bCallNumOrNoNum( false );
                            {
                                if ( !bDone )
                                {
                                    if ( bOnlyBackspaceKey && !rSh.IsNoNum() )
                                    {
                                        bCallNumOrNoNum = true;
                                    }
                                    else if ( !bOnlyBackspaceKey && rSh.IsNoNum() )
                                    {
                                        bCallNumOrNoNum = true;
                                    }
                                    else if ( bOnlyBackspaceKey &&
                                              rSh.IsSttPara() && rSh.IsEndPara() &&
                                              !rSh.HasSelection() )
                                    {
                                        const SwNumRule* pCurrNumRule( rSh.GetCurNumRule() );
                                        if ( pCurrNumRule &&
                                             pCurrNumRule != rSh.GetOutlineNumRule() )
                                        {
                                            bCallNumOrNoNum = true;
                                        }
                                    }
                                }
                            }
                            if ( bCallNumOrNoNum &&
                                 rSh.NumOrNoNum( !bOnlyBackspaceKey, sal_True ) )
                            {
                                eKeyState = KS_NumOrNoNum;
                            }
                        }
                    }
                    else
                    {
                        InfoBox( this, SW_RES( MSG_READONLY_CONTENT )).Execute();
                        eKeyState = KS_End;
                    }
                    break;

                case KEY_RIGHT:
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_RIGHT_BIG;
                        eTblChgMode = nsTblChgWidthHeightType::WH_FLAG_INSDEL | nsTblChgWidthHeightType::WH_COL_RIGHT;
                        nTblChgSize = pModOpt->GetTblVInsert();
                        eAutoCompleteAction = ACA_ReturnToRoot;
                        goto KEYINPUT_CHECKTABLE_INSDEL;
                    }
                case KEY_TAB:
                {

                    if (rSh.IsFormProtected() || rSh.GetCurrentFieldmark() || rSh.GetChar(sal_False)==CH_TXT_ATR_FORMELEMENT)
                    {
                        eKeyState=KS_GotoNextFieldMark;
                    }
                    else
                    if( rSh.GetCurNumRule() && rSh.IsSttOfPara() &&
                        !rSh.HasReadonlySel() )
                    {
                        if ( rSh.IsFirstOfNumRule() &&
                             numfunc::ChangeIndentOnTabAtFirstPosOfFirstListItem() )
                            eKeyState = KS_NumIndentInc;
                        else
                            eKeyState = KS_NumDown;
                    }
                    else if ( rSh.GetTableFmt() )
                    {
                        if( rSh.HasSelection() || rSh.HasReadonlySel() )
                            eKeyState = KS_NextCell;
                        else
                            eKeyState = KS_CheckAutoCorrect, eNextKeyState = KS_NextCell;
                    }
                    else if ( rSh.GetSelectionType() &
                                (nsSelectionType::SEL_GRF |
                                    nsSelectionType::SEL_FRM |
                                    nsSelectionType::SEL_OLE |
                                    nsSelectionType::SEL_DRW |
                                    nsSelectionType::SEL_DRW_FORM))

                            eKeyState = KS_NextObject;
                    else
                    {
                        eKeyState = KS_InsTab;
                        if( rSh.IsSttOfPara() && !rSh.HasReadonlySel() )
                        {
                            SwTxtFmtColl* pColl = rSh.GetCurTxtFmtColl();
                            if( pColl &&

                                pColl->IsAssignedToListLevelOfOutlineStyle()
                                && MAXLEVEL-1 > pColl->GetAssignedOutlineStyleLevel() )
                                eKeyState = KS_OutlineDown;
                        }
                    }
                }
                break;
                case KEY_TAB | KEY_SHIFT:
                {
                    if (rSh.IsFormProtected() || rSh.GetCurrentFieldmark()|| rSh.GetChar(sal_False)==CH_TXT_ATR_FORMELEMENT) {
                        eKeyState=KS_GotoPrevFieldMark;
                    }
                    else if( rSh.GetCurNumRule() && rSh.IsSttOfPara() &&
                         !rSh.HasReadonlySel() )
                    {

                        if ( rSh.IsFirstOfNumRule() &&
                             numfunc::ChangeIndentOnTabAtFirstPosOfFirstListItem() )
                            eKeyState = KS_NumIndentDec;
                        else
                            eKeyState = KS_NumUp;
                    }
                    else if ( rSh.GetTableFmt() )
                    {
                        if( rSh.HasSelection() || rSh.HasReadonlySel() )
                            eKeyState = KS_PrevCell;
                        else
                            eKeyState = KS_CheckAutoCorrect, eNextKeyState = KS_PrevCell;
                    }
                    else if ( rSh.GetSelectionType() &
                                (nsSelectionType::SEL_GRF |
                                    nsSelectionType::SEL_FRM |
                                    nsSelectionType::SEL_OLE |
                                    nsSelectionType::SEL_DRW |
                                    nsSelectionType::SEL_DRW_FORM))

                            eKeyState = KS_PrevObject;
                    else
                    {
                        eKeyState = KS_End;
                        if( rSh.IsSttOfPara() && !rSh.HasReadonlySel() )
                        {
                            SwTxtFmtColl* pColl = rSh.GetCurTxtFmtColl();
                            if( pColl &&
                                pColl->IsAssignedToListLevelOfOutlineStyle() &&
                                0 < pColl->GetAssignedOutlineStyleLevel())
                                eKeyState = KS_OutlineUp;
                        }
                    }
                }
                break;
                case KEY_TAB | KEY_MOD1:
                case KEY_TAB | KEY_MOD2:
                    if( !rSh.HasReadonlySel() )
                    {
                        if( aTmpQHD.HasCntnt() && !rSh.HasSelection() )
                        {
                            // Next auto-complete suggestion
                            aTmpQHD.Next( pACorr &&
                                          pACorr->GetSwFlags().bAutoCmpltEndless );
                            eKeyState = KS_NextPrevGlossary;
                        }
                        else if( rSh.GetTableFmt() )
                            eKeyState = KS_InsTab;
                        else if((rSh.GetSelectionType() &
                                    (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM|
                                        nsSelectionType::SEL_FRM|nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF))  &&
                                rSh.GetDrawView()->AreObjectsMarked())
                            eKeyState = KS_EnterDrawHandleMode;
                        else
                        {
                            eKeyState = KS_InsTab;
                        }
                        eAutoCompleteAction = ACA_NoOp;
                    }
                    break;

                    case KEY_TAB | KEY_MOD1 | KEY_SHIFT:
                    {
                        if( aTmpQHD.HasCntnt() && !rSh.HasSelection() &&
                            !rSh.HasReadonlySel() )
                        {
                            // Previous auto-complete suggestion.
                            aTmpQHD.Previous( pACorr &&
                                              pACorr->GetSwFlags().bAutoCmpltEndless );
                            eKeyState = KS_NextPrevGlossary;
                        }
                        else if((rSh.GetSelectionType() & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM|
                                        nsSelectionType::SEL_FRM|nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF)) &&
                                rSh.GetDrawView()->AreObjectsMarked())
                        {
                            eKeyState = KS_EnterDrawHandleMode;
                        }
                        eAutoCompleteAction = ACA_NoOp;
                    }
                    break;
                    case KEY_F2 :
                    if( !rSh.HasReadonlySel() )
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        if(nSelectionType & nsSelectionType::SEL_FRM)
                            eKeyState = KS_GoIntoFly;
                        else if((nSelectionType & nsSelectionType::SEL_DRW))
                            eKeyState = KS_GoIntoDrawing;
                        eAutoCompleteAction = ACA_NoOp;
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
                        bNormalChar = sal_False;
                        eKeyState = KS_End;
                        if ( rSh.GetSelectionType() &
                                (nsSelectionType::SEL_GRF |
                                    nsSelectionType::SEL_FRM |
                                    nsSelectionType::SEL_OLE |
                                    nsSelectionType::SEL_DRW |
                                    nsSelectionType::SEL_DRW_FORM))

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
                        if(nSelectionType & nsSelectionType::SEL_FRM)
                            eKeyState = KS_GoIntoFly;
                        else
                        {
                            SfxItemSet aSet(rSh.GetAttrPool(), RES_TXTATR_INETFMT, RES_TXTATR_INETFMT);
                            rSh.GetCurAttr(aSet);
                            if(SFX_ITEM_SET == aSet.GetItemState(RES_TXTATR_INETFMT, sal_False))
                            {
                                const SfxPoolItem& rItem = aSet.Get(RES_TXTATR_INETFMT, sal_True);
                                bNormalChar = sal_False;
                                eKeyState = KS_End;
                                rSh.ClickToINetAttr((const SwFmtINetFmt&)rItem, URLLOAD_NOFILTER);
                            }
                        }
                    }
                    break;
                }
                eAutoCompleteAction = ACA_NoOp;
            }
            break;

        case KS_EnterCharCell:
            {
                eKeyState = KS_KeyToView;
                switch ( rKeyCode.GetModifier() | rKeyCode.GetCode() )
                {
                    case KEY_RIGHT | KEY_MOD2:
                        rSh.Right( CRSR_SKIP_CHARS, sal_False, 1, sal_False );
                        eKeyState = KS_End;
                        FlushInBuffer();
                        break;
                    case KEY_LEFT | KEY_MOD2:
                        rSh.Left( CRSR_SKIP_CHARS, sal_False, 1, sal_False );
                        eKeyState = KS_End;
                        FlushInBuffer();
                        break;
                }
                eAutoCompleteAction = ACA_NoOp;
            }
            break;

        case KS_KeyToView:
            {
                eKeyState = KS_End;
                bNormalChar =
                    !rKeyCode.IsMod2() &&
                    rKeyCode.GetModifier() != (KEY_MOD1) &&
                    rKeyCode.GetModifier() != (KEY_MOD1|KEY_SHIFT) &&
                    SW_ISPRINTABLE( aCh );

                if( bNormalChar && rSh.IsInFrontOfLabel() )
                {
                    rSh.NumOrNoNum(sal_False);
                }

                if( aInBuffer.Len() && ( !bNormalChar || bIsDocReadOnly ))
                    FlushInBuffer();

                if( rView.KeyInput( aKeyEvent ) )
                    bFlushBuffer = sal_True, bNormalChar = sal_False;
                else
                {
                    // Because Sfx accelerators are only called when they were
                    // enabled at the last status update, copy has to called
                    // 'forcefully' by us if necessary.
                    if( rKeyCode.GetFunction() == KEYFUNC_COPY )
                        GetView().GetViewFrame()->GetBindings().Execute(SID_COPY);

                    if( !bIsDocReadOnly && bNormalChar )
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        if((nSelectionType & nsSelectionType::SEL_DRW) &&
                            0 == (nSelectionType & nsSelectionType::SEL_DRW_TXT) &&
                            rSh.GetDrawView()->GetMarkedObjectList().GetMarkCount() == 1)
                        {
                            SdrObject* pObj = rSh.GetDrawView()->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                            if(pObj)
                            {
                                EnterDrawTextMode(pObj->GetLogicRect().Center());
                                if ( rView.GetCurShell()->ISA(SwDrawTextShell) )
                                    ((SwDrawTextShell*)rView.GetCurShell())->Init();
                                rSh.GetDrawView()->KeyInput( rKEvt, this );
                            }
                        }
                        else if(nSelectionType & nsSelectionType::SEL_FRM)
                        {
                            rSh.UnSelectFrm();
                            rSh.LeaveSelFrmMode();
                            rView.AttrChangedNotify(&rSh);
                            rSh.MoveSection( fnSectionCurr, fnSectionEnd );
                        }
                        eKeyState = KS_InsChar;
                    }
                    else
                    {
                        bNormalChar = sal_False;
                        Window::KeyInput( aKeyEvent );
                    }
                }
                if( bNormalChar )
                {
                    if ( aCh == ' ' )
                    {
                        eAutoCompleteAction = ACA_ReturnToRoot;
                    }
                    else
                    {
                        eAutoCompleteAction = ACA_SingleCharInput;
                        aSingleCharInput = aCh;
                    }
                }
            }
            break;
        case KS_LaunchOLEObject:
        {
            rSh.LaunchOLEObj();
            eKeyState = KS_End;
            eAutoCompleteAction = ACA_NoOp;
        }
        break;
        case KS_GoIntoFly:
        {
            rSh.UnSelectFrm();
            rSh.LeaveSelFrmMode();
            rView.AttrChangedNotify(&rSh);
            rSh.MoveSection( fnSectionCurr, fnSectionEnd );
            eKeyState = KS_End;
            eAutoCompleteAction = ACA_NoOp;
        }
        break;
        case KS_GoIntoDrawing:
        {
            SdrObject* pObj = rSh.GetDrawView()->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            if(pObj)
            {
                EnterDrawTextMode(pObj->GetLogicRect().Center());
                if ( rView.GetCurShell()->ISA(SwDrawTextShell) )
                    ((SwDrawTextShell*)rView.GetCurShell())->Init();
            }
            eKeyState = KS_End;
            eAutoCompleteAction = ACA_NoOp;
        }
        break;
        case KS_EnterDrawHandleMode:
        {
            const SdrHdlList& rHdlList = rSh.GetDrawView()->GetHdlList();
            sal_Bool bForward(!aKeyEvent.GetKeyCode().IsShift());

            ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);
            eKeyState = KS_End;
            eAutoCompleteAction = ACA_NoOp;
        }
        break;
        case KS_InsTab:
            if( rView.ISA( SwWebView ))     // no Tab for WebView
            {
                // then it should be passed along
                Window::KeyInput( aKeyEvent );
                eKeyState = KS_End;
                break;
            }
            aCh = '\t';
            eAutoCompleteAction = ACA_ReturnToRoot;
            // no break!
        case KS_InsChar:
            if (rSh.GetChar(sal_False)==CH_TXT_ATR_FORMELEMENT)
            {
                ::sw::mark::ICheckboxFieldmark* pFieldmark =
                    dynamic_cast< ::sw::mark::ICheckboxFieldmark* >
                        (rSh.GetCurrentFieldmark());
                OSL_ENSURE(pFieldmark,
                    "Where is my FieldMark??");
                if(pFieldmark)
                {
                    pFieldmark->SetChecked(!pFieldmark->IsChecked());
                    SwDocShell* pDocSh = rView.GetDocShell();
                    SwDoc *pDoc=pDocSh->GetDoc();
                    OSL_ENSURE(pFieldmark->IsExpanded(),
                        "where is the otherpos?");
                    if (pFieldmark->IsExpanded())
                    {
                        SwPaM aPaM(pFieldmark->GetMarkPos(), pFieldmark->GetOtherMarkPos());
                        if(0)
                        {
                            rSh.StartAllAction();  //$flr TODO: understand why this not works
                            pDoc->SetModified(aPaM);
                            rSh.EndAllAction();
                        }
                        else
                        {
                            rSh.CalcLayout(); // workaround
                        }
                    }
                }
                eKeyState = KS_End;
            }
            else if(!rSh.HasReadonlySel())
            {
                sal_Bool bIsNormalChar = GetAppCharClass().isLetterNumeric(
                                                            rtl::OUString( aCh ), 0 );
                if( bAppendSpace && bIsNormalChar &&
                    (aInBuffer.Len() || !rSh.IsSttPara() || !rSh.IsEndPara() ))
                {
                    // insert a blank ahead of the character. this ends up
                    // between the expanded text and the new "non-word-seperator".
                    aInBuffer += ' ';
                }

                sal_Bool bIsAutoCorrectChar =  SvxAutoCorrect::IsAutoCorrectChar( aCh );
                sal_Bool bRunNext = pACorr && pACorr->HasRunNext();
                if( !aKeyEvent.GetRepeat() && pACorr && ( bIsAutoCorrectChar || bRunNext ) &&
                        pACfg->IsAutoFmtByInput() &&
                    (( pACorr->IsAutoCorrFlag( ChgWeightUnderl ) &&
                        ( '*' == aCh || '_' == aCh ) ) ||
                     ( pACorr->IsAutoCorrFlag( ChgQuotes ) && ('\"' == aCh ))||
                     ( pACorr->IsAutoCorrFlag( ChgSglQuotes ) && ( '\'' == aCh))))
                {
                    FlushInBuffer();
                    rSh.AutoCorrect( *pACorr, aCh );
                    if( '\"' != aCh && '\'' != aCh )        // only call when "*_"!
                        rSh.UpdateAttr();
                }
                else if( !aKeyEvent.GetRepeat() && pACorr && ( bIsAutoCorrectChar || bRunNext ) &&
                        pACfg->IsAutoFmtByInput() &&
                    pACorr->IsAutoCorrFlag( CptlSttSntnc | CptlSttWrd |
                                            ChgOrdinalNumber | AddNonBrkSpace |
                                            ChgToEnEmDash | SetINetAttr |
                                            Autocorrect ) &&
                    '\"' != aCh && '\'' != aCh && '*' != aCh && '_' != aCh
                    )
                {
                    FlushInBuffer();
                    rSh.AutoCorrect( *pACorr, aCh );
                }
                else
                {
                    rtl::OUStringBuffer aBuf(aInBuffer);
                    comphelper::string::padToLength(aBuf,
                        aInBuffer.Len() + aKeyEvent.GetRepeat() + 1, aCh);
                    aInBuffer = aBuf.makeStringAndClear();
                    bFlushCharBuffer = Application::AnyInput( VCL_INPUT_KEYBOARD );
                    bFlushBuffer = !bFlushCharBuffer;
                    if( bFlushCharBuffer )
                        aKeyInputFlushTimer.Start();
                }
                eKeyState = KS_End;
            }
            else
            {
                InfoBox( this, SW_RES( MSG_READONLY_CONTENT )).Execute();
                eKeyState = KS_End;
            }
        break;

        case KS_CheckAutoCorrect:
        {
            if( pACorr && pACfg->IsAutoFmtByInput() &&
                pACorr->IsAutoCorrFlag( CptlSttSntnc | CptlSttWrd |
                                        ChgOrdinalNumber |
                                        ChgToEnEmDash | SetINetAttr |
                                        Autocorrect ) &&
                !rSh.HasReadonlySel() )
            {
                FlushInBuffer();
                rSh.AutoCorrect( *pACorr, static_cast< sal_Unicode >('\0') );
            }
            eKeyState = eNextKeyState;
            if ( eAutoCompleteAction == ACA_Refresh )
                eAutoCompleteAction = ACA_NoOp;
        }
        break;

        default:
        {
            sal_uInt16 nSlotId = 0;
            FlushInBuffer();
            switch( eKeyState )
            {
            case KS_SpecialInsert:
                rSh.DoSpecialInsert();
                break;

            case KS_NoNum:
                rSh.NoNum();
                break;

            case KS_NumOff:
                // shell change - so record in advance
                rSh.DelNumRules();
                eKeyState = eNextKeyState;
                break;
            case KS_OutlineLvOff: // delete autofmt outlinelevel later
                break;

            case KS_NumDown:
                rSh.NumUpDown( sal_True );
                nKS_NUMDOWN_Count = 2;
                break;
            case KS_NumUp:
                rSh.NumUpDown( sal_False );
                break;

            case KS_NumIndentInc:
                rSh.ChangeIndentOfAllListLevels(360);
                nKS_NUMINDENTINC_Count = 2;
                break;

            case KS_GotoNextFieldMark:
                {
                    ::sw::mark::IFieldmark const * const pFieldmark = rSh.GetFieldmarkAfter();
                    if(pFieldmark) rSh.GotoFieldmark(pFieldmark);
                }
                break;

            case KS_GotoPrevFieldMark:
                {
                    ::sw::mark::IFieldmark const * const pFieldmark = rSh.GetFieldmarkBefore();
                    if( pFieldmark )
                        rSh.GotoFieldmark(pFieldmark);
                }
                break;

            case KS_NumIndentDec:
                rSh.ChangeIndentOfAllListLevels(-360);
                break;

            case KS_OutlineDown:
                rSh.OutlineUpDown( 1 );
                break;
            case KS_OutlineUp:
                rSh.OutlineUpDown( -1 );
                break;

            case KS_NextCell:
                // always 'flush' in tables
                rSh.GoNextCell();
                nSlotId = FN_GOTO_NEXT_CELL;
                break;
            case KS_PrevCell:
                rSh.GoPrevCell();
                nSlotId = FN_GOTO_PREV_CELL;
                break;
            case KS_AutoFmtByInput:
                rSh.SplitNode( sal_True );
                break;

            case KS_NextObject:
            case KS_PrevObject:
                if(rSh.GotoObj( KS_NextObject == eKeyState, GOTOOBJ_GOTO_ANY))
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
                // replace the word or abbreviation with the auto text
                rSh.StartUndo( UNDO_START );

                String sFnd( aTmpQHD.m_aHelpStrings[ aTmpQHD.nCurArrPos ] );
                if( aTmpQHD.m_bIsAutoText )
                {
                    SwGlossaryList* pList = ::GetGlossaryList();
                    String sShrtNm;
                    String sGroup;
                    if(pList->GetShortName( sFnd, sShrtNm, sGroup))
                    {
                        rSh.SttSelect();
                        rSh.ExtendSelection( sal_False, aTmpQHD.nLen );
                        SwGlossaryHdl* pGlosHdl = GetView().GetGlosHdl();
                        pGlosHdl->SetCurGroup(sGroup, sal_True);
                        pGlosHdl->InsertGlossary( sShrtNm);
                        pQuickHlpData->m_bAppendSpace = true;
                    }
                }
                else
                {
                    rSh.Insert( sFnd.Erase( 0, aTmpQHD.nLen ));
                    pQuickHlpData->m_bAppendSpace = !pACorr ||
                            pACorr->GetSwFlags().bAutoCmpltAppendBlanc;
                }
                rSh.EndUndo( UNDO_END );
                eAutoCompleteAction = ACA_ReturnToRoot;
            }
            break;

            case KS_NextPrevGlossary:
                pQuickHlpData->Move( aTmpQHD );
                pQuickHlpData->Start( rSh, USHRT_MAX );
                break;

            case KS_EditFormula:
            {
                const sal_uInt16 nId = SwInputChild::GetChildWindowId();

                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                pVFrame->ToggleChildWindow( nId );
                SwInputChild* pChildWin = (SwInputChild*)pVFrame->
                                                    GetChildWindow( nId );
                if( pChildWin )
                    pChildWin->SetFormula( sFmlEntry );
            }
            break;

            case KS_ColLeftBig:         rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_COL_LEFT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHMove() );   break;
            case KS_ColRightBig:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_COL_RIGHT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHMove() );  break;
            case KS_ColLeftSmall:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_COL_LEFT, pModOpt->GetTblHMove() );   break;
            case KS_ColRightSmall:      rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_COL_RIGHT, pModOpt->GetTblHMove() );  break;
            case KS_ColBottomBig:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_ROW_BOTTOM|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVMove() ); break;
            case KS_ColBottomSmall:     rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_ROW_BOTTOM, pModOpt->GetTblVMove() ); break;
            case KS_CellLeftBig:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_LEFT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHMove() );  break;
            case KS_CellRightBig:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_RIGHT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHMove() ); break;
            case KS_CellLeftSmall:      rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_LEFT, pModOpt->GetTblHMove() );  break;
            case KS_CellRightSmall:     rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_RIGHT, pModOpt->GetTblHMove() ); break;
            case KS_CellTopBig:         rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_TOP|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVMove() );   break;
            case KS_CellBottomBig:      rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_BOTTOM|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVMove() );    break;
            case KS_CellTopSmall:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_TOP, pModOpt->GetTblVMove() );   break;
            case KS_CellBottomSmall:    rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_CELL_BOTTOM, pModOpt->GetTblVMove() );    break;

//---------------
            case KS_InsDel_ColLeftBig:          rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_COL_LEFT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHInsert() ); break;
            case KS_InsDel_ColRightBig:         rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_COL_RIGHT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHInsert() );    break;
            case KS_InsDel_ColLeftSmall:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_COL_LEFT, pModOpt->GetTblHInsert() ); break;
            case KS_InsDel_ColRightSmall:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_COL_RIGHT, pModOpt->GetTblHInsert() );    break;
            case KS_InsDel_ColTopBig:           rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_ROW_TOP|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVInsert() );  break;
            case KS_InsDel_ColBottomBig:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_ROW_BOTTOM|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVInsert() );   break;
            case KS_InsDel_ColTopSmall:         rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_ROW_TOP, pModOpt->GetTblVInsert() );  break;
            case KS_InsDel_ColBottomSmall:      rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_ROW_BOTTOM, pModOpt->GetTblVInsert() );   break;
            case KS_InsDel_CellLeftBig:         rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_LEFT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHInsert() );    break;
            case KS_InsDel_CellRightBig:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_RIGHT|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblHInsert() );   break;
            case KS_InsDel_CellLeftSmall:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_LEFT, pModOpt->GetTblHInsert() );    break;
            case KS_InsDel_CellRightSmall:      rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_RIGHT, pModOpt->GetTblHInsert() );   break;
            case KS_InsDel_CellTopBig:          rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_TOP|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVInsert() ); break;
            case KS_InsDel_CellBottomBig:       rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_BOTTOM|nsTblChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTblVInsert() );  break;
            case KS_InsDel_CellTopSmall:        rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_TOP, pModOpt->GetTblVInsert() ); break;
            case KS_InsDel_CellBottomSmall:     rSh.SetColRowWidthHeight( nsTblChgWidthHeightType::WH_FLAG_INSDEL|nsTblChgWidthHeightType::WH_CELL_BOTTOM, pModOpt->GetTblVInsert() );  break;
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
            default:
                break;
            }
            if( nSlotId && rView.GetViewFrame()->GetBindings().GetRecorder().is() )
            {
                SfxRequest aReq(rView.GetViewFrame(), nSlotId );
                aReq.Done();
            }
            eKeyState = KS_End;
        }
        }
    }

    if( bStopKeyInputTimer )
    {
        aKeyInputTimer.Stop();
        bTblInsDelMode = sal_False;
    }

    bool bInsertBufferedChars = bFlushBuffer && aInBuffer.Len();
    bool bAutoCompleteEnabled = pACfg && pACorr && ( pACfg->IsAutoTextTip() || pACorr->GetSwFlags().bAutoCompleteWords );
    bool bGotWord = false;
    String sPrefix;

    // in case the buffered characters are inserted
    if( bInsertBufferedChars )
    {
        // bFlushCharBuffer was not resetted here
        // why not?
        sal_Bool bSave = bFlushCharBuffer;
        FlushInBuffer();
        bFlushCharBuffer = bSave;
    }

    // maintain AutoComplete tree
    if( bAutoCompleteEnabled )
    {
        // avoid unnecessary calls to GetPrevAutoCorrWord
        if( (bInsertBufferedChars && bNormalChar) || eAutoCompleteAction == ACA_Refresh )
        {
            bGotWord = rSh.GetPrevAutoCorrWord( *pACorr, sPrefix );
        }

        SwAutoCompleteWord& rACList = rSh.GetAutoCompleteWords();

        switch( eAutoCompleteAction )
        {
            case ACA_NoOp:
                // do nothing
            break;

            case ACA_ReturnToRoot:
                rACList.returnToRoot();
            break;

            case ACA_SingleCharInput:
                rACList.advance( aSingleCharInput );
            break;

            case ACA_SingleBackspace:
                rACList.goBack();
            break;

            case ACA_Refresh:
            {
                if( sPrefix.Len() > 0 )
                    rACList.gotoNode( sPrefix );
                else
                    rACList.returnToRoot();
            }
            break;
        }
    }

    if( bInsertBufferedChars && bNormalChar && bAutoCompleteEnabled && bGotWord )
    {
        ShowAutoTextCorrectQuickHelp( sPrefix, pACfg, pACorr );
    }

    // get the word count dialog to update itself
    SwWordCountWrapper *pWrdCnt = (SwWordCountWrapper*)GetView().GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId());
    if( pWrdCnt )
        pWrdCnt->UpdateCounts();

}

/*--------------------------------------------------------------------
     Description:  MouseEvents
 --------------------------------------------------------------------*/

void SwEditWin::RstMBDownFlags()
{
    // Not on all systems a MouseButtonUp is used ahead
    // of the modal dialog (like on WINDOWS).
    // So reset the statuses here and release the mouse
    // for the dialog.
    bMBPressed = bNoInterrupt = sal_False;
    EnterArea();
    ReleaseMouse();
}

/**
 * Determines if the current position has a clickable url over a background
 * frame. In that case, ctrl-click should select the url, not the frame.
 */
static bool lcl_urlOverBackground(SwWrtShell& rSh, const Point& rDocPos)
{
    SwContentAtPos aSwContentAtPos(SwContentAtPos::SW_INETATTR);
    SdrObject* pSelectableObj = rSh.GetObjAt(rDocPos);

    return rSh.GetContentAtPos(rDocPos, aSwContentAtPos) && pSelectableObj->GetLayer() == rSh.GetDoc()->GetHellId();
}

void SwEditWin::MouseButtonDown(const MouseEvent& _rMEvt)
{
    SwWrtShell &rSh = rView.GetWrtShell();

    // We have to check if a context menu is shown and we have an UI
    // active inplace client. In that case we have to ignore the mouse
    // button down event. Otherwise we would crash (context menu has been
    // opened by inplace client and we would deactivate the inplace client,
    // the contex menu is closed by VCL asynchronously which in the end
    // would work on deleted objects or the context menu has no parent anymore)
    SfxInPlaceClient* pIPClient = rSh.GetSfxViewShell()->GetIPClient();
    sal_Bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );

    if ( bIsOleActive && PopupMenu::IsInExecute() )
        return;

    MouseEvent rMEvt(_rMEvt);

    if (rView.GetPostItMgr()->IsHit(rMEvt.GetPosPixel()))
        return;

    rView.GetPostItMgr()->SetActiveSidebarWin(0);

    GrabFocus();

    //ignore key modifiers for format paintbrush
    {
        sal_Bool bExecFormatPaintbrush = pApplyTempl && pApplyTempl->pFormatClipboard
                                &&  pApplyTempl->pFormatClipboard->HasContent();
        if( bExecFormatPaintbrush )
            rMEvt = MouseEvent( _rMEvt.GetPosPixel(), _rMEvt.GetClicks(),
                                    _rMEvt.GetMode(), _rMEvt.GetButtons() );
    }

    bWasShdwCrsr = 0 != pShadCrsr;
    delete pShadCrsr, pShadCrsr = 0;

    const Point aDocPos( PixelToLogic( rMEvt.GetPosPixel() ) );

    // Are we clicking on a blank header/footer area?
    FrameControlType eControl;
    if ( IsInHeaderFooter( aDocPos, eControl ) )
    {
        const SwPageFrm* pPageFrm = rSh.GetLayout()->GetPageAtPos( aDocPos );

        // Is it active?
        bool bActive = true;
        const SwPageDesc* pDesc = pPageFrm->GetPageDesc();

        const SwFrmFmt* pFmt = pDesc->GetLeftFmt();
        if ( pPageFrm->OnRightPage() )
             pFmt = pDesc->GetRightFmt();

        if ( pFmt )
        {
            if ( eControl == Header )
                bActive = pFmt->GetHeader().IsActive();
            else
                bActive = pFmt->GetFooter().IsActive();
        }

        if ( !bActive )
        {
            SwPaM aPam( *rSh.GetCurrentShellCursor().GetPoint() );
            bool bWasInHeader = aPam.GetPoint( )->nNode.GetNode( ).FindHeaderStartNode( ) != NULL;
            bool bWasInFooter = aPam.GetPoint( )->nNode.GetNode( ).FindFooterStartNode( ) != NULL;

            // Is the cursor in a part like similar to the one we clicked on? For example,
            // if the cursor is in a header and we click on an empty header... don't change anything to
            // keep consistent behaviour due to header edit mode (and the same for the footer as well).
            //
            // Otherwise, we hide the header/footer control if a separator is shown, and vice versa.
            if ( !( bWasInHeader && eControl == Header ) &&
                 !( bWasInFooter && eControl == Footer ) )
            {
                rSh.SetShowHeaderFooterSeparator( eControl, !rSh.IsShowHeaderFooterSeparator( eControl ) );
            }

            // Repaint everything
            Invalidate();

            return;
        }
    }

    if ( lcl_CheckHeaderFooterClick( rSh, aDocPos, rMEvt.GetClicks() ) )
        return;


    if ( IsChainMode() )
    {
        SetChainMode( sal_False );
        SwRect aDummy;
        SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)rSh.GetFlyFrmFmt();
        if ( !rSh.Chainable( aDummy, *pFmt, aDocPos ) )
            rSh.Chain( *pFmt, aDocPos );
        UpdatePointer( aDocPos, rMEvt.GetModifier() );
        return;
    }

    // After GrabFocus a shell should be pushed. That should actually
    // work but in practice ...
    rView.SelectShellForDrop();

    sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly();
    sal_Bool bCallBase = sal_True;

    if( pQuickHlpData->m_bIsDisplayed )
        pQuickHlpData->Stop( rSh );
    pQuickHlpData->m_bAppendSpace = false;

    if( rSh.FinishOLEObj() )
        return; // end InPlace and the click doesn't count anymore

    SET_CURR_SHELL( &rSh );

    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        if (pSdrView->MouseButtonDown( rMEvt, this ) )
        {
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);
            return; // SdrView's event evaluated
        }
    }

    bIsInMove = sal_False;
    aStartPos = rMEvt.GetPosPixel();
    aRszMvHdlPt.X() = 0, aRszMvHdlPt.Y() = 0;

    sal_uInt8 nMouseTabCol = 0;
    const sal_Bool bTmp = !rSh.IsDrawCreate() && !pApplyTempl && !rSh.IsInSelect() &&
         rMEvt.GetClicks() == 1 && MOUSE_LEFT == rMEvt.GetButtons();
    if (  bTmp &&
         0 != (nMouseTabCol = rSh.WhichMouseTabCol( aDocPos ) ) &&
         !rSh.IsObjSelectable( aDocPos ) )
    {
        if ( SW_TABSEL_HORI <= nMouseTabCol && SW_TABCOLSEL_VERT >= nMouseTabCol )
        {
            rSh.EnterStdMode();
            rSh.SelectTableRowCol( aDocPos );
            if( SW_TABSEL_HORI  != nMouseTabCol && SW_TABSEL_HORI_RTL  != nMouseTabCol)
            {
                pRowColumnSelectionStart = new Point( aDocPos );
                bIsRowDrag = SW_TABROWSEL_HORI == nMouseTabCol||
                            SW_TABROWSEL_HORI_RTL == nMouseTabCol ||
                            SW_TABCOLSEL_VERT == nMouseTabCol;
                bMBPressed = sal_True;
                CaptureMouse();
            }
            return;
        }

        if ( !rSh.IsTableMode() )
        {
            // comes from table columns out of the document.
            if(SW_TABCOL_VERT == nMouseTabCol || SW_TABCOL_HORI == nMouseTabCol)
                rView.SetTabColFromDoc( sal_True );
            else
                rView.SetTabRowFromDoc( sal_True );

            rView.SetTabColFromDocPos( aDocPos );
            rView.InvalidateRulerPos();
            SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
            rBind.Update();
            if ( RulerColumnDrag( rMEvt,
                    (SW_TABCOL_VERT == nMouseTabCol || SW_TABROW_HORI == nMouseTabCol)) )
            {
                rView.SetTabColFromDoc( sal_False );
                rView.SetTabRowFromDoc( sal_False );
                rView.InvalidateRulerPos();
                rBind.Update();
                bCallBase = sal_False;
            }
            else
            {
                return;
            }
        }
    }
    else if (bTmp &&
             rSh.IsNumLabel(aDocPos))
    {
        SwTxtNode* pNodeAtPos = rSh.GetNumRuleNodeAtPos( aDocPos );
        rView.SetNumRuleNodeFromDoc( pNodeAtPos );
        rView.InvalidateRulerPos();
        SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
        rBind.Update();

        if ( RulerMarginDrag( rMEvt,
                        rSh.IsVerticalModeAtNdAndPos( *pNodeAtPos, aDocPos ) ) )
        {
            rView.SetNumRuleNodeFromDoc( NULL );
            rView.InvalidateRulerPos();
            rBind.Update();
            bCallBase = sal_False;
        }
        else
        {
            // Make sure the pointer is set to 0,
            // otherwise it may point to nowhere after deleting the corresponding
            // text node.
            rView.SetNumRuleNodeFromDoc( NULL );
            return;
        }
    }

    // One can be in a selection state when recently the keyboard was
    // used to select but no CURSOR_KEY was moved yet after that. In
    // that case the previous selction has to be finished first.
    // MA 07. Oct. 95: Not only with left mouse button but always.
    // also see Bug: 19263
    if ( rSh.IsInSelect() )
        rSh.EndSelect();

    // query against LEFT because otherwise for example also a right
    // click releases the selection.
    if ( MOUSE_LEFT == rMEvt.GetButtons() )
    {
        sal_Bool bOnlyText = sal_False;
        bMBPressed = bNoInterrupt = sal_True;
        nKS_NUMDOWN_Count = 0;

        CaptureMouse();

        // reset curor position if applicable
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
                        0 != ( pHdl = pSdrView->PickHandle(aDocPos) ) &&
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
                    bNoInterrupt = sal_False;
                    return;
                }
                else  if ( rView.GetDrawFuncPtr() && bInsFrm )
                {
                    StopInsFrm();
                    rSh.Edit();
                }

                // Without SHIFT because otherwise Toggle doesn't work at selection
                if (rMEvt.GetClicks() == 1)
                {
                    if ( rSh.IsSelFrmMode())
                    {
                        SdrHdl* pHdl = rSh.GetDrawView()->PickHandle(aDocPos);
                        sal_Bool bHitHandle = pHdl && pHdl->GetKind() != HDL_ANCHOR &&
                                                  pHdl->GetKind() != HDL_ANCHOR_TR;

                        if ((rSh.IsInsideSelectedObj(aDocPos) || bHitHandle) &&
                            !(rMEvt.GetModifier() == KEY_SHIFT && !bHitHandle))
                        {
                            rSh.EnterSelFrmMode( &aDocPos );
                            if ( !pApplyTempl )
                            {
                                // only if no position to size was hit.
                                if (!bHitHandle)
                                {
                                    StartDDTimer();
                                    SwEditWin::nDDStartPosY = aDocPos.Y();
                                    SwEditWin::nDDStartPosX = aDocPos.X();
                                }
                                bFrmDrag = sal_True;
                            }
                            bNoInterrupt = sal_False;
                            return;
                        }
                    }
                }
        }

        sal_Bool bExecHyperlinks = rView.GetDocShell()->IsReadOnly();
        if ( !bExecHyperlinks )
        {
            SvtSecurityOptions aSecOpts;
            const sal_Bool bSecureOption = aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );
            if ( (  bSecureOption && rMEvt.GetModifier() == KEY_MOD1 ) ||
                 ( !bSecureOption && rMEvt.GetModifier() != KEY_MOD1 ) )
                bExecHyperlinks = sal_True;
        }

        sal_uInt8 nNumberOfClicks = static_cast< sal_uInt8 >(rMEvt.GetClicks() % 4);
        if ( 0 == nNumberOfClicks && 0 < rMEvt.GetClicks() )
            nNumberOfClicks = 4;

        sal_Bool bExecDrawTextLink = sal_False;

        switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
        {
            case MOUSE_LEFT:
            case MOUSE_LEFT + KEY_MOD1:
            case MOUSE_LEFT + KEY_MOD2:
                switch ( nNumberOfClicks )
                {
                    case 1:
                    {
                        UpdatePointer( aDocPos, rMEvt.GetModifier() );
                        SwEditWin::nDDStartPosY = aDocPos.Y();
                        SwEditWin::nDDStartPosX = aDocPos.X();

                        // hit an URL in DrawText object?
                        if (bExecHyperlinks && pSdrView)
                        {
                            SdrViewEvent aVEvt;
                            pSdrView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                            if (aVEvt.eEvent == SDREVENT_EXECUTEURL)
                                bExecDrawTextLink = sal_True;
                        }

                        // only try to select frame, if pointer already was
                        // switched accordingly
                        if ( aActHitType != SDRHIT_NONE && !rSh.IsSelFrmMode() &&
                            !GetView().GetViewFrame()->GetDispatcher()->IsLocked() &&
                            !bExecDrawTextLink)
                        {
                            // Test if there is a draw object at that position and if it should be selected.
                            sal_Bool bShould = rSh.ShouldObjectBeSelected(aDocPos);

                            if(bShould)
                            {
                                rView.NoRotate();
                                rSh.HideCrsr();

                                sal_Bool bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( sal_True );
                                sal_Bool bSelObj = rSh.SelectObj( aDocPos,
                                               rMEvt.IsMod1() ? SW_ENTER_GROUP : 0);
                                if( bUnLockView )
                                    rSh.LockView( sal_False );

                                if( bSelObj )
                                {
                                    // if the frame was deselected in the macro
                                    // the cursor just has to be displayed again
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
                                        bFrmDrag = sal_True;
                                        UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                    }
                                    return;
                                }
                                else
                                    bOnlyText = static_cast< sal_Bool >(rSh.IsObjSelectable( aDocPos ));

                                if (!rView.GetDrawFuncPtr())
                                    rSh.ShowCrsr();
                            }
                            else
                                bOnlyText = KEY_MOD1 != rMEvt.GetModifier();
                        }
                        else if ( rSh.IsSelFrmMode() &&
                                  (aActHitType == SDRHIT_NONE ||
                                   !rSh.IsInsideSelectedObj( aDocPos )))
                        {
                            rView.NoRotate();
                            SdrHdl *pHdl;
                            if( !bIsDocReadOnly && !pAnchorMarker && 0 !=
                                ( pHdl = pSdrView->PickHandle(aDocPos) ) &&
                                    ( pHdl->GetKind() == HDL_ANCHOR ||
                                      pHdl->GetKind() == HDL_ANCHOR_TR ) )
                            {
                                pAnchorMarker = new SwAnchorMarker( pHdl );
                                UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                return;
                            }
                            else
                            {
                                sal_Bool bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( sal_True );
                                sal_uInt8 nFlag = rMEvt.IsShift() ? SW_ADD_SELECT :0;
                                if( rMEvt.IsMod1() )
                                    nFlag = nFlag | SW_ENTER_GROUP;

                                if ( rSh.IsSelFrmMode() )
                                {
                                    rSh.UnSelectFrm();
                                    rSh.LeaveSelFrmMode();
                                    rView.AttrChangedNotify(&rSh);
                                }

                                sal_Bool bSelObj = rSh.SelectObj( aDocPos, nFlag );
                                if( bUnLockView )
                                    rSh.LockView( sal_False );

                                if( !bSelObj )
                                {
                                    // move cursor here so that it is not drawn in the
                                    // frame first; ShowCrsr() happens in LeaveSelFrmMode()
                                    bValidCrsrPos = !(CRSR_POSCHG & rSh.SetCursor(&aDocPos, false));
                                    rSh.LeaveSelFrmMode();
                                    rView.AttrChangedNotify( &rSh );
                                    bCallBase = sal_False;
                                }
                                else
                                {
                                    rSh.HideCrsr();
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    rSh.SelFlyGrabCrsr();
                                    rSh.MakeSelVisible();
                                    bFrmDrag = sal_True;
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
                        bFrmDrag = sal_False;
                        if ( !bIsDocReadOnly && rSh.IsInsideSelectedObj(aDocPos) &&
                             0 == rSh.IsSelObjProtected( FLYPROTECT_CONTENT|FLYPROTECT_PARENT ) )

/* this is no good, on the one hand GetSelectionType is used as flag field (take a look into the GetSelectionType method)
   on the other hand the return value is used in a switch without proper masking (very nice), this must lead to trouble
*/
                        switch ( rSh.GetSelectionType() &~ ( nsSelectionType::SEL_FONTWORK | nsSelectionType::SEL_EXTRUDED_CUSTOMSHAPE ) )
                        {
                            case nsSelectionType::SEL_GRF:
                                RstMBDownFlags();
                                GetView().GetViewFrame()->GetBindings().Execute(
                                    FN_FORMAT_GRAFIC_DLG, 0, 0,
                                    SFX_CALLMODE_RECORD|SFX_CALLMODE_SLOT);
                                return;

                                // double click on OLE object --> OLE-InPlace
                            case nsSelectionType::SEL_OLE:
                                if (!rSh.IsSelObjProtected(FLYPROTECT_CONTENT))
                                {
                                    RstMBDownFlags();
                                    rSh.LaunchOLEObj();
                                }
                                return;

                            case nsSelectionType::SEL_FRM:
                                RstMBDownFlags();
                                GetView().GetViewFrame()->GetBindings().Execute(
                                    FN_FORMAT_FRAME_DLG, 0, 0, SFX_CALLMODE_RECORD|SFX_CALLMODE_SLOT);
                                return;

                            case nsSelectionType::SEL_DRW:
                                RstMBDownFlags();
                                EnterDrawTextMode(aDocPos);
                                if ( rView.GetCurShell()->ISA(SwDrawTextShell) )
                                    ((SwDrawTextShell*)rView.GetCurShell())->Init();
                                return;
                        }

                        // if the cursor position was corrected or if a Fly
                        // was selected in ReadOnlyMode, no word selection.
                        if ( !bValidCrsrPos ||
                            (rSh.IsFrmSelected() && rSh.IsFrmSelected() ))
                            return;

                        SwField *pFld;
                        sal_Bool bFtn = sal_False;

                        if( !bIsDocReadOnly &&
                            ( 0 != ( pFld = rSh.GetCurFld() ) ||
                              0 != ( bFtn = rSh.GetCurFtn() )) )
                        {
                            RstMBDownFlags();
                            if( bFtn )
                                GetView().GetViewFrame()->GetBindings().Execute( FN_EDIT_FOOTNOTE );
                            else
                            {
                                sal_uInt16 nTypeId = pFld->GetTypeId();
                                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                                switch( nTypeId )
                                {
                                case TYP_POSTITFLD:
                                case TYP_SCRIPTFLD:
                                {
                                    // if it's a Readonly region, status has to be enabled
                                    sal_uInt16 nSlot = TYP_POSTITFLD == nTypeId ? FN_POSTIT : FN_JAVAEDIT;
                                    SfxBoolItem aItem(nSlot, sal_True);
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
                        // in extended mode double and triple
                        // click has no effect.
                        if ( rSh.IsExtMode() || rSh.IsBlockMode() )
                            return;

                        // select work, AdditionalMode if applicable
                        if ( KEY_MOD1 == rMEvt.GetModifier() && !rSh.IsAddMode() )
                        {
                            rSh.EnterAddMode();
                            rSh.SelWrd( &aDocPos );
                            rSh.LeaveAddMode();
                        }
                        else
                            rSh.SelWrd( &aDocPos );
                        bHoldSelection = sal_True;
                        return;
                    }
                    case 3:
                    case 4:
                    {
                        bFrmDrag = sal_False;
                        // in extended mode double and triple
                        // click has no effect.
                        if ( rSh.IsExtMode() )
                            return;

                        // if the cursor position was corrected or if a Fly
                        // was selected in ReadOnlyMode, no word selection.
                        if ( !bValidCrsrPos || rSh.IsFrmSelected() )
                            return;

                        // select line, AdditionalMode if applicable
                        const bool bMod = KEY_MOD1 == rMEvt.GetModifier() &&
                                         !rSh.IsAddMode();

                        if ( bMod )
                            rSh.EnterAddMode();

                        if ( 3 == nNumberOfClicks )
                            rSh.SelSentence( &aDocPos );
                        else
                            rSh.SelPara( &aDocPos );

                        if ( bMod )
                            rSh.LeaveAddMode();

                        bHoldSelection = sal_True;
                        return;
                    }

                    default:
                        return;
                }
                /* no break */
            case MOUSE_LEFT + KEY_SHIFT:
            case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            {
                sal_Bool bLockView = bWasShdwCrsr;

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
                                    bFrmDrag = sal_True;
                                    return;
                                }
                            }
                        }
                        else if( rSh.IsSelFrmMode() &&
                                 rSh.GetDrawView()->PickHandle( aDocPos ))
                        {
                            bFrmDrag = sal_True;
                            bNoInterrupt = sal_False;
                            return;
                        }
                    }
                    break;
                    case KEY_MOD1:
                    if ( !bExecDrawTextLink )
                    {
                        if ( !bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) && !lcl_urlOverBackground( rSh, aDocPos ) )
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
                                    bFrmDrag = sal_True;
                                    return;
                                }
                            }
                        }
                        else if( rSh.IsSelFrmMode() &&
                                 rSh.GetDrawView()->PickHandle( aDocPos ))
                        {
                            bFrmDrag = sal_True;
                            bNoInterrupt = sal_False;
                            return;
                        }
                        else
                        {
                            if ( !rSh.IsAddMode() && !rSh.IsExtMode() && !rSh.IsBlockMode() )
                            {
                                rSh.PushMode();
                                bModePushed = sal_True;

                                sal_Bool bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( sal_True );
                                rSh.EnterAddMode();
                                if( bUnLockView )
                                    rSh.LockView( sal_False );
                            }
                            bCallBase = sal_False;
                        }
                    }
                    break;
                    case KEY_MOD2:
                    {
                        if ( !rSh.IsAddMode() && !rSh.IsExtMode() && !rSh.IsBlockMode() )
                        {
                            rSh.PushMode();
                            bModePushed = sal_True;
                            sal_Bool bUnLockView = !rSh.IsViewLocked();
                            rSh.LockView( sal_True );
                            rSh.EnterBlockMode();
                            if( bUnLockView )
                                rSh.LockView( sal_False );
                        }
                        bCallBase = sal_False;
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

                                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                                if (rMarkList.GetMark(0) == NULL)
                                {
                                    rSh.LeaveSelFrmMode();
                                    rView.AttrChangedNotify(&rSh);
                                    bFrmDrag = sal_False;
                                }
                            }
                            else
                            {   if ( rSh.SelectObj( aDocPos ) )
                                {
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    SwEditWin::nDDStartPosY = aDocPos.Y();
                                    SwEditWin::nDDStartPosX = aDocPos.X();
                                    bFrmDrag = sal_True;
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
                                bFrmDrag = sal_True;
                                return;
                            }
                            if ( rSh.IsSelFrmMode() )
                            {
                                rSh.UnSelectFrm();
                                rSh.LeaveSelFrmMode();
                                rView.AttrChangedNotify(&rSh);
                                bFrmDrag = sal_False;
                            }
                            if ( !rSh.IsExtMode() )
                            {
                                // don't start a selection when an
                                // URL field or a graphic is clicked
                                sal_Bool bSttSelect = rSh.HasSelection() ||
                                                Pointer(POINTER_REFHAND) != GetPointer();

                                if( !bSttSelect )
                                {
                                    bSttSelect = sal_True;
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
                                                    bLockView = sal_True;

                                            bSttSelect = sal_False;
                                        }
                                        else if( rSh.IsURLGrfAtPos( aDocPos ))
                                            bSttSelect = sal_False;
                                    }
                                }

                                if( bSttSelect )
                                    rSh.SttSelect();
                            }
                        }
                        bCallBase = sal_False;
                        break;
                    }
                    default:
                        if( !rSh.IsViewLocked() )
                        {
                            SwContentAtPos aCntntAtPos( SwContentAtPos::SW_CLICKFIELD |
                                                        SwContentAtPos::SW_INETATTR );
                            if( rSh.GetContentAtPos( aDocPos, aCntntAtPos, sal_False ) &&
                                !rSh.IsReadOnlyAvailable() &&
                                aCntntAtPos.IsInProtectSect() )
                                bLockView = sal_True;
                        }
                }

                if ( rSh.IsGCAttr() )
                {
                    rSh.GCAttr();
                    rSh.ClearGCAttr();
                }

                sal_Bool bOverSelect = rSh.ChgCurrPam( aDocPos ), bOverURLGrf = sal_False;
                if( !bOverSelect )
                    bOverURLGrf = bOverSelect = 0 != rSh.IsURLGrfAtPos( aDocPos );

                if ( !bOverSelect )
                {
                    const sal_Bool bTmpNoInterrupt = bNoInterrupt;
                    bNoInterrupt = sal_False;

                    if( !rSh.IsViewLocked() && bLockView )
                        rSh.LockView( sal_True );
                    else
                        bLockView = sal_False;

                    int nTmpSetCrsr = 0;

                    {   // only temporary generate Move-Kontext because otherwise
                        // the query to the content form doesn't work!!!
                        SwMvContext aMvContext( &rSh );
                        nTmpSetCrsr = rSh.SetCursor(&aDocPos, bOnlyText);
                        bValidCrsrPos = !(CRSR_POSCHG & nTmpSetCrsr);
                        bCallBase = sal_False;
                    }

                    // notify the edit window that from now on we do not use the input language
                    if ( !(CRSR_POSOLD & nTmpSetCrsr) )
                        SetUseInputLanguage( sal_False );

                    if( bLockView )
                        rSh.LockView( sal_False );

                    bNoInterrupt = bTmpNoInterrupt;
                }
                if ( !bOverURLGrf && !bOnlyText )
                {
                    const int nSelType = rSh.GetSelectionType();
                    // Check in general, if an object is selectable at given position.
                    // Thus, also text fly frames in background become selectable via Ctrl-Click.
                    if ( ( nSelType & nsSelectionType::SEL_OLE ||
                         nSelType & nsSelectionType::SEL_GRF ||
                         rSh.IsObjSelectable( aDocPos ) ) && !lcl_urlOverBackground( rSh, aDocPos ) )
                    {
                        SwMvContext aMvContext( &rSh );
                        if( !rSh.IsFrmSelected() )
                            rSh.GotoNextFly();
                        rSh.EnterSelFrmMode();
                        bCallBase = sal_False;
                    }
                }
                // don't reset here any longer so that, in case through MouseMove
                // with pressed Ctrl key a multiple-selection should happen,
                // the previous selection is not released in Drag.
                break;
            }
        }
    }
    if (bCallBase)
        Window::MouseButtonDown(rMEvt);
}

/*--------------------------------------------------------------------
    Description:   MouseMove
 --------------------------------------------------------------------*/

void SwEditWin::MouseMove(const MouseEvent& _rMEvt)
{
    MouseEvent rMEvt(_rMEvt);

    //ignore key modifiers for format paintbrush
    {
        sal_Bool bExecFormatPaintbrush = pApplyTempl && pApplyTempl->pFormatClipboard
                                &&  pApplyTempl->pFormatClipboard->HasContent();
        if( bExecFormatPaintbrush )
            rMEvt = MouseEvent( _rMEvt.GetPosPixel(), _rMEvt.GetClicks(),
                                    _rMEvt.GetMode(), _rMEvt.GetButtons() );
    }

    // as long as an action is running the MouseMove should be disconnected
    // otherwise bug 40102 occurs
    SwWrtShell &rSh = rView.GetWrtShell();
    if( rSh.ActionPend() )
        return ;

    if( pShadCrsr && 0 != (rMEvt.GetModifier() + rMEvt.GetButtons() ) )
        delete pShadCrsr, pShadCrsr = 0;

    sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly();

    SET_CURR_SHELL( &rSh );

    //aPixPt == Point in Pixel, relative to ChildWin
    //aDocPt == Point in Twips, document coordinates
    const Point aPixPt( rMEvt.GetPosPixel() );
    const Point aDocPt( PixelToLogic( aPixPt ) );

    if ( IsChainMode() )
    {
        UpdatePointer( aDocPt, rMEvt.GetModifier() );
        return;
    }

    SdrView *pSdrView = rSh.GetDrawView();

    const SwCallMouseEvent aLastCallEvent( aSaveCallEvent );
    aSaveCallEvent.Clear();

    if ( !bIsDocReadOnly && pSdrView && pSdrView->MouseMove(rMEvt,this) )
    {
        SetPointer( POINTER_TEXT );
        return; // evaluate SdrView's event
    }

    const Point aOldPt( rSh.VisArea().Pos() );
    const sal_Bool bInsWin = rSh.VisArea().IsInside( aDocPt );

    if( pShadCrsr && !bInsWin )
        delete pShadCrsr, pShadCrsr = 0;

    if( bInsWin && pRowColumnSelectionStart )
    {
        EnterArea();
        Point aPos( aDocPt );
        if( rSh.SelectTableRowCol( *pRowColumnSelectionStart, &aPos, bIsRowDrag ))
            return;
    }

    // position is necessary for OS/2 because obviously after a MB-Down
    // a MB-Move is called immediately.
    if( bDDTimerStarted )
    {
        Point aDD( SwEditWin::nDDStartPosX, SwEditWin::nDDStartPosY );
        aDD = LogicToPixel( aDD );
        Rectangle aRect( aDD.X()-3, aDD.Y()-3, aDD.X()+3, aDD.Y()+3 );
        if ( !aRect.IsInside( aPixPt ) )
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
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
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

    sal_uInt8 nMouseTabCol;
    if( !bIsDocReadOnly && bInsWin && !pApplyTempl && !rSh.IsInSelect() )
    {
        if ( SW_TABCOL_NONE != (nMouseTabCol = rSh.WhichMouseTabCol( aDocPt ) ) &&
             !rSh.IsObjSelectable( aDocPt ) )
        {
            sal_uInt16 nPointer = USHRT_MAX;
            bool bChkTblSel = false;

            switch ( nMouseTabCol )
            {
                case SW_TABCOL_VERT :
                case SW_TABROW_HORI :
                    nPointer = POINTER_VSIZEBAR;
                    bChkTblSel = true;
                    break;
                case SW_TABROW_VERT :
                case SW_TABCOL_HORI :
                    nPointer = POINTER_HSIZEBAR;
                    bChkTblSel = true;
                    break;
                case SW_TABSEL_HORI :
                    nPointer = POINTER_TAB_SELECT_SE;
                    break;
                case SW_TABSEL_HORI_RTL :
                case SW_TABSEL_VERT :
                    nPointer = POINTER_TAB_SELECT_SW;
                    break;
                case SW_TABCOLSEL_HORI :
                case SW_TABROWSEL_VERT :
                    nPointer = POINTER_TAB_SELECT_S;
                    break;
                case SW_TABROWSEL_HORI :
                    nPointer = POINTER_TAB_SELECT_E;
                    break;
                case SW_TABROWSEL_HORI_RTL :
                case SW_TABCOLSEL_VERT :
                    nPointer = POINTER_TAB_SELECT_W;
                    break;
            }

            if ( USHRT_MAX != nPointer &&
                // i#35543 - Enhanced table selection is explicitly allowed in table mode
                ( !bChkTblSel || !rSh.IsTableMode() ) )
            {
                SetPointer( nPointer );
            }

            return;
        }
        else if (rSh.IsNumLabel(aDocPt, RULER_MOUSE_MARGINWIDTH))
        {
            // i#42921 - consider vertical mode
            SwTxtNode* pNodeAtPos = rSh.GetNumRuleNodeAtPos( aDocPt );
            const sal_uInt16 nPointer =
                    rSh.IsVerticalModeAtNdAndPos( *pNodeAtPos, aDocPt )
                    ? POINTER_VSIZEBAR
                    : POINTER_HSIZEBAR;
            SetPointer( nPointer );

            return;
        }
    }

    sal_Bool bDelShadCrsr = sal_True;

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

                const Point aOld = pAnchorMarker->GetPosForHitTest( *(rSh.GetOut()) );
                Point aNew = rSh.FindAnchorPos( aDocPt );
                SdrHdl* pHdl;
                if( (0!=( pHdl = pSdrView->PickHandle( aOld ) )||
                    0 !=(pHdl = pSdrView->PickHandle( pAnchorMarker->GetHdlPos()) ) ) &&
                        ( pHdl->GetKind() == HDL_ANCHOR ||
                          pHdl->GetKind() == HDL_ANCHOR_TR ) )
                {
                    pAnchorMarker->ChgHdl( pHdl );
                    if( aNew.X() || aNew.Y() )
                    {
                        pAnchorMarker->SetPos( aNew );
                        pAnchorMarker->SetLastPos( aDocPt );
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
                        pSdrView->SetOrtho(sal_False);
                        rView.GetDrawFuncPtr()->MouseMove( rMEvt );
                    }
                    bIsInMove = sal_True;
                }
                return;
            }

            {
            SwWordCountWrapper *pWrdCnt = (SwWordCountWrapper*)GetView().GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId());
            if (pWrdCnt)
                pWrdCnt->UpdateCounts();
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
                    // event processing for resizing
                    if( pSdrView->AreObjectsMarked() )
                    {
                        const SwFrmFmt* pFlyFmt;
                        const SvxMacro* pMacro;

                        const Point aSttPt( PixelToLogic( aStartPos ) );

                        // can we start?
                        if( HDL_USER == eSdrMoveHdl )
                        {
                            SdrHdl* pHdl = pSdrView->PickHandle( aSttPt );
                            eSdrMoveHdl = pHdl ? pHdl->GetKind() : HDL_MOVE;
                        }

                        sal_uInt16 nEvent = HDL_MOVE == eSdrMoveHdl
                                            ? SW_EVENT_FRM_MOVE
                                            : SW_EVENT_FRM_RESIZE;

                        if( 0 != ( pFlyFmt = rSh.GetFlyFrmFmt() ) &&
                            0 != ( pMacro = pFlyFmt->GetMacro().GetMacroTable().
                            Get( nEvent )) &&
// or notify only e.g. every 20 Twip?
                            aRszMvHdlPt != aDocPt )
                        {
                            aRszMvHdlPt = aDocPt;
                            sal_uInt16 nPos = 0;
                            String sRet;
                            SbxArrayRef xArgs = new SbxArray;
                            SbxVariableRef xVar = new SbxVariable;
                            xVar->PutString( pFlyFmt->GetName() );
                            xArgs->Put( &xVar, ++nPos );

                            if( SW_EVENT_FRM_RESIZE == nEvent )
                            {
                                xVar = new SbxVariable;
                                xVar->PutUShort( static_cast< sal_uInt16 >(eSdrMoveHdl) );
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
                    // event processing for resizing

                    if( bIsDocReadOnly )
                        break;

                    if ( rMEvt.IsShift() )
                    {
                        pSdrView->SetOrtho(sal_True);
                        pSdrView->SetAngleSnapEnabled(sal_True);
                    }
                    else
                    {
                        pSdrView->SetOrtho(sal_False);
                        pSdrView->SetAngleSnapEnabled(sal_False);
                    }

                    rSh.Drag( &aDocPt, rMEvt.IsShift() );
                    bIsInMove = sal_True;
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
                        pSdrView->SetOrtho(sal_True);
                    else
                        pSdrView->SetOrtho(sal_False);
                }
                if ( !bInsWin )
                {
                    Point aTmp( aDocPt );
                    aTmp += rSh.VisArea().Pos() - aOldPt;
                    LeaveArea( aTmp );
                }
                else
                {
                    if( !rMEvt.IsSynthetic() &&
                            !(( MOUSE_LEFT + KEY_MOD1 ==
                            rMEvt.GetModifier() + rMEvt.GetButtons() ) &&
                            rSh.Is_FnDragEQBeginDrag() && !rSh.IsAddMode() ))
                    {
                        rSh.Drag( &aDocPt, false );

                        bValidCrsrPos = !(CRSR_POSCHG & rSh.SetCursor(&aDocPt, false));
                        EnterArea();
                    }
                }
            }
            bDDINetAttr = sal_False;
            break;
        case 0:
        {
            if ( pApplyTempl )
            {
                UpdatePointer(aDocPt, 0); // maybe a frame has to be marked here
                break;
            }
            // change ui if mouse is over SwPostItField
            // TODO: do the same thing for redlines SW_REDLINE
            SwRect aFldRect;
            SwContentAtPos aCntntAtPos( SwContentAtPos::SW_FIELD);
            if( rSh.GetContentAtPos( aDocPt, aCntntAtPos, sal_False, &aFldRect ) )
            {
                const SwField* pFld = aCntntAtPos.aFnd.pFld;
                if (pFld->Which()== RES_POSTITFLD)
                {
                    rView.GetPostItMgr()->SetShadowState(reinterpret_cast<const SwPostItField*>(pFld),false);
                }
                else
                    rView.GetPostItMgr()->SetShadowState(0,false);
            }
            else
                rView.GetPostItMgr()->SetShadowState(0,false);
        }
        // no break;
        case KEY_SHIFT:
        case KEY_MOD2:
        case KEY_MOD1:
            if ( !bInsDraw )
            {
                sal_Bool bTstShdwCrsr = sal_True;

                UpdatePointer( aDocPt, rMEvt.GetModifier() );

                const SwFrmFmt* pFmt = 0;
                const SwFmtINetFmt* pINet = 0;
                SwContentAtPos aCntntAtPos( SwContentAtPos::SW_INETATTR );
                if( rSh.GetContentAtPos( aDocPt, aCntntAtPos ) )
                    pINet = (SwFmtINetFmt*)aCntntAtPos.aFnd.pAttr;

                const void* pTmp = pINet;

                if( pINet ||
                    0 != ( pTmp = pFmt = rSh.GetFmtFromAnyObj( aDocPt )))
                {
                    bTstShdwCrsr = sal_False;
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

                    // should be be over a InternetField with an
                    // embedded macro?
                    if( aSaveCallEvent != aLastCallEvent )
                    {
                        if( aLastCallEvent.HasEvent() )
                            rSh.CallEvent( SFX_EVENT_MOUSEOUT_OBJECT,
                                            aLastCallEvent, true );
                        // 0 says that the object doesn't have any table
                        if( !rSh.CallEvent( SFX_EVENT_MOUSEOVER_OBJECT,
                                        aSaveCallEvent ))
                            aSaveCallEvent.Clear();
                    }
                }
                else if( aLastCallEvent.HasEvent() )
                {
                    // cursor was on an object
                    rSh.CallEvent( SFX_EVENT_MOUSEOUT_OBJECT,
                                    aLastCallEvent, true );
                }

                if( bTstShdwCrsr && bInsWin && !bIsDocReadOnly &&
                    !bInsFrm &&
                    !rSh.GetViewOptions()->getBrowseMode() &&
                    rSh.GetViewOptions()->IsShadowCursor() &&
                    !(rMEvt.GetModifier() + rMEvt.GetButtons()) &&
                    !rSh.HasSelection() && !GetConnectMetaFile() )
                {
                    SwRect aRect;
                    sal_Int16 eOrient;
                    SwFillMode eMode = (SwFillMode)rSh.GetViewOptions()->GetShdwCrsrFillMode();
                    if( rSh.GetShadowCrsrPos( aDocPt, eMode, aRect, eOrient ))
                    {
                        if( !pShadCrsr )
                            pShadCrsr = new SwShadowCursor( *this,
                                SwViewOption::GetDirectCursorColor() );
                        if( text::HoriOrientation::RIGHT != eOrient && text::HoriOrientation::CENTER != eOrient )
                            eOrient = text::HoriOrientation::LEFT;
                        pShadCrsr->SetPos( aRect.Pos(), aRect.Height(), static_cast< sal_uInt16 >(eOrient) );
                        bDelShadCrsr = sal_False;
                    }
                }
            }
            break;
        case MOUSE_LEFT + KEY_MOD2:
            if( rSh.IsBlockMode() && !rMEvt.IsSynthetic() )
            {
                rSh.Drag( &aDocPt, false );
                bValidCrsrPos = !(CRSR_POSCHG & rSh.SetCursor(&aDocPt, false));
                EnterArea();
            }
        break;
    }

    if( bDelShadCrsr && pShadCrsr )
        delete pShadCrsr, pShadCrsr = 0;
    bWasShdwCrsr = sal_False;
}

/*--------------------------------------------------------------------
    Description:   Button Up
 --------------------------------------------------------------------*/

void SwEditWin::MouseButtonUp(const MouseEvent& rMEvt)
{
    sal_Bool bCallBase = sal_True;

    sal_Bool bCallShadowCrsr = bWasShdwCrsr;
    bWasShdwCrsr = sal_False;
    if( pShadCrsr )
        delete pShadCrsr, pShadCrsr = 0;

    if( pRowColumnSelectionStart )
        DELETEZ( pRowColumnSelectionStart );

    SdrHdlKind eOldSdrMoveHdl = eSdrMoveHdl;
    eSdrMoveHdl = HDL_USER;     // for MoveEvents - reset again

    // preventively reset
    rView.SetTabColFromDoc( sal_False );
    rView.SetNumRuleNodeFromDoc(NULL);

    SwWrtShell &rSh = rView.GetWrtShell();
    SET_CURR_SHELL( &rSh );
    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        pSdrView->SetOrtho(sal_False);

        if ( pSdrView->MouseButtonUp( rMEvt,this ) )
        {
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);
            return; // SdrView's event evaluated
        }
    }
    // only process MouseButtonUp when the Down went to that windows as well.
    if ( !bMBPressed )
    {
// Undo for the watering can is already in CommandHdl
// that's the way it should be!

        return;
    }

    Point aDocPt( PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( bDDTimerStarted )
    {
        StopDDTimer( &rSh, aDocPt );
        bMBPressed = sal_False;
        if ( rSh.IsSelFrmMode() )
        {
            rSh.EndDrag( &aDocPt, false );
            bFrmDrag = sal_False;
        }
        bNoInterrupt = sal_False;
        ReleaseMouse();
        return;
    }

    if( pAnchorMarker )
    {
        Point aPnt( pAnchorMarker->GetLastPos() );
        DELETEZ( pAnchorMarker );
        if( aPnt.X() || aPnt.Y() )
            rSh.FindAnchorPos( aPnt, sal_True );
    }
    if ( bInsDraw && rView.GetDrawFuncPtr() )
    {
        if ( rView.GetDrawFuncPtr()->MouseButtonUp( rMEvt ) )
        {
            if (rView.GetDrawFuncPtr()) // could have been destroyed in MouseButtonUp
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
                    StdDrawMode( OBJ_NONE, sal_True );
            }
            else if ( rSh.IsFrmSelected() )
            {
                rSh.EnterSelFrmMode();
                StopInsFrm();
            }
            else
            {
                const Point aDocPos( PixelToLogic( aStartPos ) );
                bValidCrsrPos = !(CRSR_POSCHG & rSh.SetCursor(&aDocPos, false));
                rSh.Edit();
            }

            rView.AttrChangedNotify( &rSh );
        }
        else if (rMEvt.GetButtons() == MOUSE_RIGHT && rSh.IsDrawCreate())
            rView.GetDrawFuncPtr()->BreakCreate();   // abort drawing

        bNoInterrupt = sal_False;
        ReleaseMouse();
        return;
    }
    sal_Bool bPopMode = sal_False;
    switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
    {
        case MOUSE_LEFT:
            if ( bInsDraw && rSh.IsDrawCreate() )
            {
                if ( rView.GetDrawFuncPtr() && rView.GetDrawFuncPtr()->MouseButtonUp(rMEvt) == sal_True )
                {
                    rView.GetDrawFuncPtr()->Deactivate();
                    rView.AttrChangedNotify( &rSh );
                    if ( rSh.IsObjSelected() )
                        rSh.EnterSelFrmMode();
                    if ( rView.GetDrawFuncPtr() && bInsFrm )
                        StopInsFrm();
                }
                bCallBase = sal_False;
                break;
            }
        case MOUSE_LEFT + KEY_MOD1:
        case MOUSE_LEFT + KEY_MOD2:
        case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            if ( bFrmDrag && rSh.IsSelFrmMode() )
            {
                if ( rMEvt.IsMod1() ) // copy and don't move.
                {
                    // abort drag, use internal Copy instead
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
                            rSh.StartUndo( UNDO_UI_DRAG_AND_COPY );
                            rSh.Copy(&rSh, aSttPt, aEndPt, sal_False);
                            rSh.EndUndo( UNDO_UI_DRAG_AND_COPY );
                        }
                    }
                    else
                        rSh.EndDrag( &aDocPt, false );
                }
                else
                {
                    {
                        const SwFrmFmt* pFlyFmt;
                        const SvxMacro* pMacro;

                        sal_uInt16 nEvent = HDL_MOVE == eOldSdrMoveHdl
                                            ? SW_EVENT_FRM_MOVE
                                            : SW_EVENT_FRM_RESIZE;

                        if( 0 != ( pFlyFmt = rSh.GetFlyFrmFmt() ) &&
                            0 != ( pMacro = pFlyFmt->GetMacro().GetMacroTable().
                            Get( nEvent )) )
                        {
                            const Point aSttPt( PixelToLogic( aStartPos ) );
                            aRszMvHdlPt = aDocPt;
                            sal_uInt16 nPos = 0;
                            SbxArrayRef xArgs = new SbxArray;
                            SbxVariableRef xVar = new SbxVariable;
                            xVar->PutString( pFlyFmt->GetName() );
                            xArgs->Put( &xVar, ++nPos );

                            if( SW_EVENT_FRM_RESIZE == nEvent )
                            {
                                xVar = new SbxVariable;
                                xVar->PutUShort( static_cast< sal_uInt16 >(eOldSdrMoveHdl) );
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
                    rSh.EndDrag( &aDocPt, false );
                }
                bFrmDrag = sal_False;
                bCallBase = sal_False;
                break;
            }
            bPopMode = sal_True;
            // no break
        case MOUSE_LEFT + KEY_SHIFT:
            if (rSh.IsSelFrmMode())
            {

                rSh.EndDrag( &aDocPt, false );
                bFrmDrag = sal_False;
                bCallBase = sal_False;
                break;
            }

            if( bHoldSelection )
            {
                // the EndDrag should be called in any case
                bHoldSelection = sal_False;
                rSh.EndDrag( &aDocPt, false );
            }
            else
            {
                if ( !rSh.IsInSelect() && rSh.ChgCurrPam( aDocPt ) )
                {
                    const sal_Bool bTmpNoInterrupt = bNoInterrupt;
                    bNoInterrupt = sal_False;
                    {   // create only temporary move context because otherwise
                        // the query to the content form doesn't work!!!
                        SwMvContext aMvContext( &rSh );
                        const Point aDocPos( PixelToLogic( aStartPos ) );
                        bValidCrsrPos = !(CRSR_POSCHG & rSh.SetCursor(&aDocPos, false));
                    }
                    bNoInterrupt = bTmpNoInterrupt;

                }
                else
                {
                    sal_Bool bInSel = rSh.IsInSelect();
                    rSh.EndDrag( &aDocPt, false );

                    // Internetfield? --> call link (load doc!!)
                    if( !bInSel )
                    {
                        sal_uInt16 nFilter = URLLOAD_NOFILTER;
                        if( KEY_MOD1 == rMEvt.GetModifier() )
                            nFilter |= URLLOAD_NEWVIEW;

                        sal_Bool bExecHyperlinks = rView.GetDocShell()->IsReadOnly();
                        if ( !bExecHyperlinks )
                        {
                            SvtSecurityOptions aSecOpts;
                            const sal_Bool bSecureOption = aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );
                            if ( (  bSecureOption && rMEvt.GetModifier() == KEY_MOD1 ) ||
                                 ( !bSecureOption && rMEvt.GetModifier() != KEY_MOD1 ) )
                                bExecHyperlinks = sal_True;
                        }

                        const sal_Bool bExecSmarttags = rMEvt.GetModifier() == KEY_MOD1;

                        if(pApplyTempl)
                            bExecHyperlinks = sal_False;

                        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_CLICKFIELD |
                                                    SwContentAtPos::SW_INETATTR |
                                                    SwContentAtPos::SW_SMARTTAG  | SwContentAtPos::SW_FORMCTRL);

                        if( rSh.GetContentAtPos( aDocPt, aCntntAtPos, sal_True ) )
                        {
                            sal_Bool bViewLocked = rSh.IsViewLocked();
                            if( !bViewLocked && !rSh.IsReadOnlyAvailable() &&
                                aCntntAtPos.IsInProtectSect() )
                                rSh.LockView( sal_True );

                            ReleaseMouse();

                            if( SwContentAtPos::SW_FIELD == aCntntAtPos.eCntntAtPos )
                            {
                                rSh.ClickToField( *aCntntAtPos.aFnd.pFld );
                            }
                            else if ( SwContentAtPos::SW_SMARTTAG == aCntntAtPos.eCntntAtPos )
                            {
                                    // execute smarttag menu
                                    if ( bExecSmarttags && SwSmartTagMgr::Get().IsSmartTagsEnabled() )
                                        rView.ExecSmartTagPopup( aDocPt );
                            }
                            else if ( SwContentAtPos::SW_FORMCTRL == aCntntAtPos.eCntntAtPos )
                            {
                                OSL_ENSURE( aCntntAtPos.aFnd.pFldmark != NULL, "where is my field ptr???");
                                if ( aCntntAtPos.aFnd.pFldmark != NULL)
                                {
                                    IFieldmark *fieldBM = const_cast< IFieldmark* > ( aCntntAtPos.aFnd.pFldmark );
                                    if ( fieldBM->GetFieldname( ) == ODF_FORMCHECKBOX )
                                    {
                                        ICheckboxFieldmark* pCheckboxFm = dynamic_cast<ICheckboxFieldmark*>(fieldBM);
                                        pCheckboxFm->SetChecked(!pCheckboxFm->IsChecked());
                                        pCheckboxFm->Invalidate();
                                        rSh.InvalidateWindows( rView.GetVisArea() );
                                    } else if ( fieldBM->GetFieldname() == ODF_FORMDROPDOWN ) {
                                        rView.ExecFieldPopup( aDocPt, fieldBM );
                                        fieldBM->Invalidate();
                                        rSh.InvalidateWindows( rView.GetVisArea() );
                                    } else {
                                        // unknown type..
                                    }
                                }
                            }
                            else
                            {
                                if ( bExecHyperlinks )
                                    rSh.ClickToINetAttr( *(SwFmtINetFmt*)aCntntAtPos.aFnd.pAttr, nFilter );
                            }

                            rSh.LockView( bViewLocked );
                            bCallShadowCrsr = sal_False;
                        }
                        else
                        {
                            aCntntAtPos = SwContentAtPos( SwContentAtPos::SW_FTN );
                            if( !rSh.GetContentAtPos( aDocPt, aCntntAtPos, sal_True ) && bExecHyperlinks )
                            {
                                SdrViewEvent aVEvt;

                                if (pSdrView)
                                    pSdrView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                                if (pSdrView && aVEvt.eEvent == SDREVENT_EXECUTEURL)
                                {
                                    // hit URL field
                                    const SvxURLField *pField = aVEvt.pURLField;
                                    if (pField)
                                    {
                                        String sURL(pField->GetURL());
                                        String sTarget(pField->GetTargetFrame());
                                        ::LoadURL(rSh, sURL, nFilter, sTarget);
                                    }
                                    bCallShadowCrsr = sal_False;
                                }
                                else
                                {
                                    // hit graphic
                                    ReleaseMouse();
                                    if( rSh.ClickToINetGrf( aDocPt, nFilter ))
                                        bCallShadowCrsr = sal_False;
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
                            SwUndoId nLastUndoId(UNDO_EMPTY);
                            if (rSh.GetLastUndoInfo(0, & nLastUndoId))
                            {
                                if (UNDO_INS_FROM_SHADOWCRSR == nLastUndoId)
                                {
                                    rSh.Undo();
                                }
                            }
                            SwFillMode eMode = (SwFillMode)rSh.GetViewOptions()->GetShdwCrsrFillMode();
                            rSh.SetShadowCrsrPos( aDocPt, eMode );
                        }
                    }
                }
                bCallBase = sal_False;

            }

            // reset pushed mode in Down again if applicable
            if ( bPopMode && bModePushed )
            {
                rSh.PopMode();
                bModePushed = sal_False;
                bCallBase = sal_False;
            }
            break;

        default:
            ReleaseMouse();
            return;
    }

    if( pApplyTempl )
    {
        int eSelection = rSh.GetSelectionType();
        SwFormatClipboard* pFormatClipboard = pApplyTempl->pFormatClipboard;
        if( pFormatClipboard )//apply format paintbrush
        {
            //get some parameters
            SwWrtShell& rWrtShell = rView.GetWrtShell();
            SfxStyleSheetBasePool* pPool=0;
            bool bNoCharacterFormats = false;
            bool bNoParagraphFormats = true;
            {
                SwDocShell* pDocSh = rView.GetDocShell();
                if(pDocSh)
                    pPool = pDocSh->GetStyleSheetPool();
                if( (rMEvt.GetModifier()&KEY_MOD1) && (rMEvt.GetModifier()&KEY_SHIFT) )
                {
                    bNoCharacterFormats = true;
                    bNoParagraphFormats = false;
                }
                else if( rMEvt.GetModifier() & KEY_MOD1 )
                    bNoParagraphFormats = false;
            }
            //execute paste
            pFormatClipboard->Paste( rWrtShell, pPool, bNoCharacterFormats, bNoParagraphFormats );

            //if the clipboard is empty after paste remove the ApplyTemplate
            if(!pFormatClipboard->HasContent())
                SetApplyTemplate(SwApplyTemplate());
        }
        else if( pApplyTempl->nColor )
        {
            sal_uInt16 nId = 0;
            switch( pApplyTempl->nColor )
            {
                case SID_ATTR_CHAR_COLOR_EXT:
                case SID_ATTR_CHAR_COLOR2:
                    nId = RES_CHRATR_COLOR;
                    break;
                case SID_ATTR_CHAR_COLOR_BACKGROUND_EXT:
                case SID_ATTR_CHAR_COLOR_BACKGROUND:
                    nId = RES_CHRATR_BACKGROUND;
                    break;
            }
            if( nId && (nsSelectionType::SEL_TXT|nsSelectionType::SEL_TBL) & eSelection)
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

                    pApplyTempl->bUndo = sal_True;
                    bCallBase = sal_False;
                    aTemplateTimer.Stop();
                }
                else if(rMEvt.GetClicks() == 1)
                {
                    // no selection -> so turn off watering can
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
                    if( (( nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL )
                         & eSelection ) && !rSh.HasReadonlySel() )
                    {
                        rSh.SetTxtFmtColl( pApplyTempl->aColl.pTxtColl );
                        pApplyTempl->bUndo = sal_True;
                        bCallBase = sal_False;
                        if ( pApplyTempl->aColl.pTxtColl )
                            aStyleName = pApplyTempl->aColl.pTxtColl->GetName();
                    }
                    break;
                case SFX_STYLE_FAMILY_CHAR:
                    if( (( nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL )
                         & eSelection ) && !rSh.HasReadonlySel() )
                    {
                        rSh.SetAttr( SwFmtCharFmt(pApplyTempl->aColl.pCharFmt) );
                        rSh.UnSetVisCrsr();
                        rSh.EnterStdMode();
                        rSh.SetVisCrsr(aDocPt);
                        pApplyTempl->bUndo = sal_True;
                        bCallBase = sal_False;
                        if ( pApplyTempl->aColl.pCharFmt )
                            aStyleName = pApplyTempl->aColl.pCharFmt->GetName();
                    }
                    break;
                case SFX_STYLE_FAMILY_FRAME :
                {
                    const SwFrmFmt* pFmt = rSh.GetFmtFromObj( aDocPt );
                    if(PTR_CAST(SwFlyFrmFmt, pFmt))
                    {
                        rSh.SetFrmFmt( pApplyTempl->aColl.pFrmFmt, false, &aDocPt );
                        pApplyTempl->bUndo = sal_True;
                        bCallBase = sal_False;
                        if( pApplyTempl->aColl.pFrmFmt )
                            aStyleName = pApplyTempl->aColl.pFrmFmt->GetName();
                    }
                    break;
                }
                case SFX_STYLE_FAMILY_PAGE:
                            // no Undo with page templates
                    rSh.ChgCurPageDesc( *pApplyTempl->aColl.pPageDesc );
                    if ( pApplyTempl->aColl.pPageDesc )
                        aStyleName = pApplyTempl->aColl.pPageDesc->GetName();
                    bCallBase = sal_False;
                    break;
                case SFX_STYLE_FAMILY_PSEUDO:
                    if( !rSh.HasReadonlySel() )
                    {
                        rSh.SetCurNumRule( *pApplyTempl->aColl.pNumRule,
                                           false,
                                           pApplyTempl->aColl.pNumRule->GetDefaultListId() );
                        bCallBase = sal_False;
                        pApplyTempl->bUndo = sal_True;
                        if( pApplyTempl->aColl.pNumRule )
                            aStyleName = pApplyTempl->aColl.pNumRule->GetName();
                    }
                    break;
            }

            uno::Reference< frame::XDispatchRecorder > xRecorder =
                    rView.GetViewFrame()->GetBindings().GetRecorder();
            if ( aStyleName.Len() && xRecorder.is() )
            {
                SfxShell *pSfxShell = lcl_GetShellFromDispatcher( rView, TYPE(SwTextShell) );
                if ( pSfxShell )
                {
                    SfxRequest aReq( rView.GetViewFrame(), SID_STYLE_APPLY );
                    aReq.AppendItem( SfxStringItem( SID_STYLE_APPLY, aStyleName ) );
                    aReq.AppendItem( SfxUInt16Item( SID_STYLE_FAMILY, (sal_uInt16) pApplyTempl->eType ) );
                    aReq.Done();
                }
            }
        }

    }
    ReleaseMouse();
    // Only processed MouseEvents arrive here; only at these the moduses can
    // be resetted.
    bMBPressed = sal_False;

    //sicherheitshalber aufrufen, da jetzt das Selektieren bestimmt zu Ende ist.
    //Andernfalls koennte der Timeout des Timers Kummer machen.
    EnterArea();
    bNoInterrupt = sal_False;

    if (bCallBase)
        Window::MouseButtonUp(rMEvt);
}

/*--------------------------------------------------------------------
    Description:   apply template
 --------------------------------------------------------------------*/

void SwEditWin::SetApplyTemplate(const SwApplyTemplate &rTempl)
{
    static sal_Bool bIdle = sal_False;
    DELETEZ(pApplyTempl);
    SwWrtShell &rSh = rView.GetWrtShell();

    if(rTempl.pFormatClipboard)
    {
        pApplyTempl = new SwApplyTemplate( rTempl );
              SetPointer( POINTER_FILL );//@todo #i20119# maybe better a new brush pointer here in future
              rSh.NoEdit( sal_False );
              bIdle = rSh.GetViewOptions()->IsIdle();
              ((SwViewOption *)rSh.GetViewOptions())->SetIdle( sal_False );
    }
    else if(rTempl.nColor)
    {
        pApplyTempl = new SwApplyTemplate( rTempl );
        SetPointer( POINTER_FILL );
        rSh.NoEdit( sal_False );
        bIdle = rSh.GetViewOptions()->IsIdle();
        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( sal_False );
    }
    else if( rTempl.eType )
    {
        pApplyTempl = new SwApplyTemplate( rTempl );
        SetPointer( POINTER_FILL  );
        rSh.NoEdit( sal_False );
        bIdle = rSh.GetViewOptions()->IsIdle();
        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( sal_False );
    }
    else
    {
        SetPointer( POINTER_TEXT );
        rSh.UnSetVisCrsr();

        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( bIdle );
        if ( !rSh.IsSelFrmMode() )
            rSh.Edit();
    }

    static sal_uInt16 aInva[] =
    {
        SID_STYLE_WATERCAN,
        SID_ATTR_CHAR_COLOR_EXT,
        SID_ATTR_CHAR_COLOR_BACKGROUND_EXT,
        0
    };
    rView.GetViewFrame()->GetBindings().Invalidate(aInva);
}

/*--------------------------------------------------------------------
    Description:   ctor
 --------------------------------------------------------------------*/

SwEditWin::SwEditWin(Window *pParent, SwView &rMyView):
    Window(pParent, WinBits(WB_CLIPCHILDREN | WB_DIALOGCONTROL)),
    DropTargetHelper( this ),
    DragSourceHelper( this ),

    eBufferLanguage(LANGUAGE_DONTKNOW),
    pApplyTempl(0),
    pAnchorMarker( 0 ),
    pUserMarker( 0 ),
    pUserMarkerObj( 0 ),
    pShadCrsr( 0 ),
    pRowColumnSelectionStart( 0 ),

    rView( rMyView ),

    aActHitType(SDRHIT_NONE),
    m_nDropFormat( 0 ),
    m_nDropAction( 0 ),
    m_nDropDestination( 0 ),

    nInsFrmColCount( 1 ),
    eDrawMode(OBJ_NONE),

    bLockInput(sal_False),
    bObjectSelect( sal_False ),
    nKS_NUMDOWN_Count(0),
    nKS_NUMINDENTINC_Count(0),
    m_aFrameControlsManager( this )
{
    SetHelpId(HID_EDIT_WIN);
    EnableChildTransparentMode();
    SetDialogControlFlags( WINDOW_DLGCTRL_RETURN | WINDOW_DLGCTRL_WANTFOCUS );

    bLinkRemoved = bMBPressed = bInsDraw = bInsFrm =
    bIsInDrag = bOldIdle = bOldIdleSet = bChainMode = bWasShdwCrsr = sal_False;
    // initially use the input language
    bUseInputLanguage = sal_True;

    SetMapMode(MapMode(MAP_TWIP));

    SetPointer( POINTER_TEXT );
    aTimer.SetTimeoutHdl(LINK(this, SwEditWin, TimerHandler));

    bTblInsDelMode = sal_False;
    aKeyInputTimer.SetTimeout( 3000 );
    aKeyInputTimer.SetTimeoutHdl(LINK(this, SwEditWin, KeyInputTimerHandler));

    aKeyInputFlushTimer.SetTimeout( 200 );
    aKeyInputFlushTimer.SetTimeoutHdl(LINK(this, SwEditWin, KeyInputFlushHandler));

    // TemplatePointer for colors should be resetted without
    // selection after single click
    aTemplateTimer.SetTimeout(400);
    aTemplateTimer.SetTimeoutHdl(LINK(this, SwEditWin, TemplateTimerHdl));

    // temporary solution!!! Should set the font of the current
    //          insert position at every curor movement!
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
    delete pRowColumnSelectionStart;
    if( pQuickHlpData->m_bIsDisplayed && rView.GetWrtShellPtr() )
        pQuickHlpData->Stop( rView.GetWrtShell() );
    bExecuteDrag = sal_False;
    delete pApplyTempl;
    rView.SetDrawFuncPtr(NULL);

    delete pUserMarker;
    delete pAnchorMarker;
}

/******************************************************************************
 *  Description: turn on DrawTextEditMode
 ******************************************************************************/

void SwEditWin::EnterDrawTextMode( const Point& aDocPos )
{
    if ( rView.EnterDrawTextMode(aDocPos) == sal_True )
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
 *  Description: turn on DrawMode
 ******************************************************************************/

sal_Bool SwEditWin::EnterDrawMode(const MouseEvent& rMEvt, const Point& aDocPos)
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SdrView *pSdrView = rSh.GetDrawView();

    if ( rView.GetDrawFuncPtr() )
    {
        if (rSh.IsDrawCreate())
            return sal_True;

        sal_Bool bRet = rView.GetDrawFuncPtr()->MouseButtonDown( rMEvt );
        rView.AttrChangedNotify( &rSh );
        return bRet;
    }

    if ( pSdrView && pSdrView->IsTextEdit() )
    {
        sal_Bool bUnLockView = !rSh.IsViewLocked();
        rSh.LockView( sal_True );

        rSh.EndTextEdit(); // clicked aside, end Edit
        rSh.SelectObj( aDocPos );
        if ( !rSh.IsObjSelected() && !rSh.IsFrmSelected() )
            rSh.LeaveSelFrmMode();
        else
        {
            SwEditWin::nDDStartPosY = aDocPos.Y();
            SwEditWin::nDDStartPosX = aDocPos.X();
            bFrmDrag = sal_True;
        }
        if( bUnLockView )
            rSh.LockView( sal_False );
        rView.AttrChangedNotify( &rSh );
        return sal_True;
    }
    return sal_False;
}

sal_Bool SwEditWin::IsDrawSelMode()
{
    return IsObjectSelect();
}

void SwEditWin::GetFocus()
{
    if ( rView.GetPostItMgr()->HasActiveSidebarWin() )
    {
        rView.GetPostItMgr()->GrabFocusOnActiveSidebarWin();
    }
    else
    {
        rView.GotFocus();
        Window::GetFocus();
        rView.GetWrtShell().InvalidateAccessibleFocus();
    }
}

void SwEditWin::LoseFocus()
{
    rView.GetWrtShell().InvalidateAccessibleFocus();
    Window::LoseFocus();
    if( pQuickHlpData->m_bIsDisplayed )
        pQuickHlpData->Stop( rView.GetWrtShell() );
    rView.LostFocus();
}

void SwEditWin::Command( const CommandEvent& rCEvt )
{
    SwWrtShell &rSh = rView.GetWrtShell();

    if ( !rView.GetViewFrame() )
    {
        // If ViewFrame dies shortly, no popup anymore!
        Window::Command(rCEvt);
        return;
    }

    // The command event is send to the window after a possible context
    // menu from an inplace client has been closed. Now we have the chance
    // to deactivate the inplace client without any problem regarding parent
    // windows and code on the stack.
    SfxInPlaceClient* pIPClient = rSh.GetSfxViewShell()->GetIPClient();
    sal_Bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );
    if ( bIsOleActive && ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU ))
    {
        rSh.FinishOLEObj();
        return;
    }

    sal_Bool bCallBase      = sal_True;

    switch ( rCEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            const sal_uInt16 nId = SwInputChild::GetChildWindowId();
            SwInputChild* pChildWin = (SwInputChild*)GetView().GetViewFrame()->
                                                GetChildWindow( nId );

            if (rView.GetPostItMgr()->IsHit(rCEvt.GetMousePosPixel()))
                return;

            Point aDocPos( PixelToLogic( rCEvt.GetMousePosPixel() ) );
            if ( !rCEvt.IsMouseEvent() )
                aDocPos = rSh.GetCharRect().Center();

            if (rCEvt.IsMouseEvent() && lcl_CheckHeaderFooterClick( rSh, aDocPos, 1 ) )
                return;


            if((!pChildWin || pChildWin->GetView() != &rView) &&
                !rSh.IsDrawCreate() && !IsDrawAction())
            {
                SET_CURR_SHELL( &rSh );
                if (!pApplyTempl)
                {
                    if (bNoInterrupt == sal_True)
                    {
                        ReleaseMouse();
                        bNoInterrupt = sal_False;
                        bMBPressed = sal_False;
                    }
                    if ( rCEvt.IsMouseEvent() )
                    {
                        SelectMenuPosition(rSh, rCEvt.GetMousePosPixel());
                        rView.StopShellTimer();
                    }
                    const Point aPixPos = LogicToPixel( aDocPos );

                    if ( rView.GetDocShell()->IsReadOnly() )
                    {
                        SwReadOnlyPopup* pROPopup = new SwReadOnlyPopup( aDocPos, rView );

                        ui::ContextMenuExecuteEvent aEvent;
                        aEvent.SourceWindow = VCLUnoHelper::GetInterface( this );
                        aEvent.ExecutePosition.X = aPixPos.X();
                        aEvent.ExecutePosition.Y = aPixPos.Y();
                        Menu* pMenu = 0;
                        ::rtl::OUString sMenuName(RTL_CONSTASCII_USTRINGPARAM("private:resource/ReadonlyContextMenu"));
                        if( GetView().TryContextMenuInterception( *pROPopup, sMenuName, pMenu, aEvent ) )
                        {
                            if ( pMenu )
                            {
                                sal_uInt16 nExecId = ((PopupMenu*)pMenu)->Execute(this, aPixPos);
                                if( !::ExecuteMenuCommand( *static_cast<PopupMenu*>(pMenu), *rView.GetViewFrame(), nExecId ))
                                    pROPopup->Execute(this, nExecId);
                            }
                            else
                                pROPopup->Execute(this, aPixPos);
                        }
                        delete pROPopup;
                    }
                    else if ( !rView.ExecSpellPopup( aDocPos ) )
                        GetView().GetViewFrame()->GetDispatcher()->ExecutePopup( 0, this, &aPixPos);
                }
                else if (pApplyTempl->bUndo)
                    rSh.Do(SwWrtShell::UNDO);
                bCallBase = sal_False;
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
                // forward to Outliner if applicable
                if ( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
                {
                    bCallBase = sal_False;
                    rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
                    break;
                }

                const CommandVoiceData *pCData = rCEvt.GetVoiceData();
                if ( VOICECOMMANDTYPE_CONTROL == pCData->GetType() )
                    break;

                sal_uInt16 nSlotId = 0;
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
                                                        pItem = new SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT );
                                                        break;
                    case DICTATIONCOMMAND_BOLD_OFF:     nSlotId = SID_ATTR_CHAR_WEIGHT;
                                                        pItem = new SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_WEIGHT );
                                                        break;
                    case DICTATIONCOMMAND_UNDERLINE_ON: nSlotId = SID_ATTR_CHAR_UNDERLINE;
                                                        pItem = new SvxUnderlineItem( UNDERLINE_SINGLE, RES_CHRATR_WEIGHT );
                                                        break;
                    case DICTATIONCOMMAND_UNDERLINE_OFF:nSlotId = SID_ATTR_CHAR_UNDERLINE;
                                                        pItem = new SvxUnderlineItem( UNDERLINE_NONE, RES_CHRATR_UNDERLINE );
                                                        break;
                    case DICTATIONCOMMAND_ITALIC_ON:    nSlotId = SID_ATTR_CHAR_POSTURE;
                                                        pItem = new SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE );
                                                        break;
                    case DICTATIONCOMMAND_ITALIC_OFF:   nSlotId = SID_ATTR_CHAR_POSTURE;
                                                        pItem = new SvxPostureItem( ITALIC_NONE, RES_CHRATR_POSTURE );
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

#if OSL_DEBUG_LEVEL > 0
                    default:
                        OSL_ENSURE( !this, "unknown speech command." );
#endif
                }
                if ( nSlotId )
                {
                    bCallBase = sal_False;
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
        sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
        if(!bIsDocReadOnly)
        {
            if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
            {
                bCallBase = sal_False;
                rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
            }
            else
            {
                if( rSh.HasSelection() )
                    rSh.DelRight();

                bCallBase = sal_False;
                LanguageType eInputLanguage = GetInputLanguage();
                rSh.CreateExtTextInput(eInputLanguage);
            }
        }
        break;
    }
    case COMMAND_ENDEXTTEXTINPUT:
    {
        sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
        if(!bIsDocReadOnly)
        {
            if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
            {
                bCallBase = sal_False;
                rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
            }
            else
            {
                bCallBase = sal_False;
                String sRecord = rSh.DeleteExtTextInput();
                uno::Reference< frame::XDispatchRecorder > xRecorder =
                        rView.GetViewFrame()->GetBindings().GetRecorder();

                if ( sRecord.Len() )
                {
                    // convert quotes in IME text
                    // works on the last input character, this is escpecially in Korean text often done
                    // quotes that are inside of the string are not replaced!
                    const sal_Unicode aCh = sRecord.GetChar(sRecord.Len() - 1);
                    SvxAutoCorrCfg& rACfg = SvxAutoCorrCfg::Get();
                    SvxAutoCorrect* pACorr = rACfg.GetAutoCorrect();
                    if(pACorr &&
                        (( pACorr->IsAutoCorrFlag( ChgQuotes ) && ('\"' == aCh ))||
                        ( pACorr->IsAutoCorrFlag( ChgSglQuotes ) && ( '\'' == aCh))))
                    {
                        rSh.DelLeft();
                        rSh.AutoCorrect( *pACorr, aCh );
                    }

                    if ( xRecorder.is() )
                    {
                        // determine Shell
                        SfxShell *pSfxShell = lcl_GetShellFromDispatcher( rView, TYPE(SwTextShell) );
                        // generate request and record
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
        sal_Bool bIsDocReadOnly = rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
        if(!bIsDocReadOnly)
        {
            if( pQuickHlpData->m_bIsDisplayed )
                pQuickHlpData->Stop( rSh );

            String sWord;
            if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
            {
                bCallBase = sal_False;
                rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
            }
            else
            {
                const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();
                if( pData )
                {
                    sWord = pData->GetText();
                    bCallBase = sal_False;
                    rSh.SetExtTextInputData( *pData );
                }
            }
                uno::Reference< frame::XDispatchRecorder > xRecorder =
                        rView.GetViewFrame()->GetBindings().GetRecorder();
                if(!xRecorder.is())
                {
                    SvxAutoCorrCfg& rACfg = SvxAutoCorrCfg::Get();
                    SvxAutoCorrect* pACorr = rACfg.GetAutoCorrect();
                    if( pACorr &&
                        // If autocompletion required...
                        ( rACfg.IsAutoTextTip() ||
                          pACorr->GetSwFlags().bAutoCompleteWords ) &&
                        // ... and extraction of last word from text input was successful...
                        rSh.GetPrevAutoCorrWord( *pACorr, sWord ) )
                    {
                        // ... request for auto completion help to be shown.
                        ShowAutoTextCorrectQuickHelp(sWord, &rACfg, pACorr, true);
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

            sal_uLong nDropFormat;
            sal_uInt16 nEventAction, nDropAction, nDropDestination;
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
                                    nDropFormat, nDropDestination, sal_False,
                                    sal_False, &aDocPt, EXCHG_IN_ACTION_COPY,
                                    sal_True );
            }
        }
        break;
        case COMMAND_MODKEYCHANGE :
        {
            const CommandModKeyData* pCommandData = (const CommandModKeyData*)rCEvt.GetData();
            if(pCommandData->IsMod1() && !pCommandData->IsMod2())
            {
                sal_uInt16 nSlot = 0;
                if(pCommandData->IsLeftShift() && !pCommandData->IsRightShift())
                    nSlot = SID_ATTR_PARA_LEFT_TO_RIGHT;
                else if(!pCommandData->IsLeftShift() && pCommandData->IsRightShift())
                    nSlot = SID_ATTR_PARA_RIGHT_TO_LEFT;
                if(nSlot && SW_MOD()->GetCTLOptions().IsCTLFontEnabled())
                    GetView().GetViewFrame()->GetDispatcher()->Execute(nSlot);
            }
        }
        break;
        case COMMAND_HANGUL_HANJA_CONVERSION :
            GetView().GetViewFrame()->GetDispatcher()->Execute(SID_HANGUL_HANJA_CONVERSION);
        break;
        case COMMAND_INPUTLANGUAGECHANGE :
            // i#42732 - update state of fontname if input language changes
            bInputLanguageSwitched = true;
            SetUseInputLanguage( sal_True );
        break;
        case COMMAND_SELECTIONCHANGE:
        {
            const CommandSelectionChangeData *pData = rCEvt.GetSelectionChangeData();
            rSh.SttCrsrMove();
            rSh.GoStartSentence();
            rSh.GetCrsr()->GetPoint()->nContent += sal::static_int_cast<sal_uInt16, sal_uLong>(pData->GetStart());
            rSh.SetMark();
            rSh.GetCrsr()->GetMark()->nContent += sal::static_int_cast<sal_uInt16, sal_uLong>(pData->GetEnd() - pData->GetStart());
            rSh.EndCrsrMove( sal_True );
        }
        break;
        case COMMAND_PREPARERECONVERSION:
        if( rSh.HasSelection() )
        {
            SwPaM *pCrsr = (SwPaM*)rSh.GetCrsr();

            if( rSh.IsMultiSelection() )
            {
                if( pCrsr && !pCrsr->HasMark() &&
                pCrsr->GetPoint() == pCrsr->GetMark() )
                {
                rSh.GoPrevCrsr();
                pCrsr = (SwPaM*)rSh.GetCrsr();
                }

                // Cancel all selections other than the last selected one.
                while( rSh.GetCrsr()->GetNext() != rSh.GetCrsr() )
                delete rSh.GetCrsr()->GetNext();
            }

            if( pCrsr )
            {
                sal_uLong nPosNodeIdx = pCrsr->GetPoint()->nNode.GetIndex();
                xub_StrLen nPosIdx = pCrsr->GetPoint()->nContent.GetIndex();
                sal_uLong nMarkNodeIdx = pCrsr->GetMark()->nNode.GetIndex();
                xub_StrLen nMarkIdx = pCrsr->GetMark()->nContent.GetIndex();

                if( !rSh.GetCrsr()->HasMark() )
                rSh.GetCrsr()->SetMark();

                rSh.SttCrsrMove();

                if( nPosNodeIdx < nMarkNodeIdx )
                {
                rSh.GetCrsr()->GetPoint()->nNode = nPosNodeIdx;
                rSh.GetCrsr()->GetPoint()->nContent = nPosIdx;
                rSh.GetCrsr()->GetMark()->nNode = nPosNodeIdx;
                rSh.GetCrsr()->GetMark()->nContent =
                    rSh.GetCrsr()->GetCntntNode( sal_True )->Len();
                }
                else if( nPosNodeIdx == nMarkNodeIdx )
                {
                rSh.GetCrsr()->GetPoint()->nNode = nPosNodeIdx;
                rSh.GetCrsr()->GetPoint()->nContent = nPosIdx;
                rSh.GetCrsr()->GetMark()->nNode = nMarkNodeIdx;
                rSh.GetCrsr()->GetMark()->nContent = nMarkIdx;
                }
                else
                {
                rSh.GetCrsr()->GetMark()->nNode = nMarkNodeIdx;
                rSh.GetCrsr()->GetMark()->nContent = nMarkIdx;
                rSh.GetCrsr()->GetPoint()->nNode = nMarkNodeIdx;
                rSh.GetCrsr()->GetPoint()->nContent =
                    rSh.GetCrsr()->GetCntntNode( sal_False )->Len();
                }

                rSh.EndCrsrMove( sal_True );
            }
        }
        break;
#if OSL_DEBUG_LEVEL > 0
        default:
            OSL_ENSURE( !this, "unknown command." );
#endif
    }
    if (bCallBase)
        Window::Command(rCEvt);
}

/*  i#18686 select the object/cursor at the mouse
    position of the context menu request */
sal_Bool SwEditWin::SelectMenuPosition(SwWrtShell& rSh, const Point& rMousePos )
{
    sal_Bool bRet = sal_False;
    const Point aDocPos( PixelToLogic( rMousePos ) );
    const bool bIsInsideSelectedObj( rSh.IsInsideSelectedObj( aDocPos ) );
    //create a synthetic mouse event out of the coordinates
    MouseEvent aMEvt(rMousePos);
    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        // no close of insert_draw and reset of
        // draw mode, if context menu position is inside a selected object.
        if ( !bIsInsideSelectedObj && rView.GetDrawFuncPtr() )
        {

            rView.GetDrawFuncPtr()->Deactivate();
            rView.SetDrawFuncPtr(NULL);
            rView.LeaveDrawCreate();
            SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
            rBind.Invalidate( SID_ATTR_SIZE );
            rBind.Invalidate( SID_TABLE_CELL );
        }

        // if draw text is active and there's a text selection
        // at the mouse position then do nothing
        if(rSh.GetSelectionType() & nsSelectionType::SEL_DRW_TXT)
        {
            OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
            ESelection aSelection = pOLV->GetSelection();
            if(!aSelection.IsZero())
            {
                SdrOutliner* pOutliner = pSdrView->GetTextEditOutliner();
                sal_Bool bVertical = pOutliner->IsVertical();
                const EditEngine& rEditEng = pOutliner->GetEditEngine();
                Point aEEPos(aDocPos);
                const Rectangle& rOutputArea = pOLV->GetOutputArea();
                // regard vertical mode
                if(bVertical)
                {
                    aEEPos -= rOutputArea.TopRight();
                    //invert the horizontal direction and exchange X and Y
                    long nTemp = -aEEPos.X();
                    aEEPos.X() = aEEPos.Y();
                    aEEPos.Y() = nTemp;
                }
                else
                    aEEPos -= rOutputArea.TopLeft();

                EPosition aDocPosition = rEditEng.FindDocPosition(aEEPos);
                ESelection aCompare(aDocPosition.nPara, aDocPosition.nIndex);
                // make it a forward selection - otherwise the IsLess/IsGreater do not work :-(
                aSelection.Adjust();
                if(!aCompare.IsLess(aSelection)  && !aCompare.IsGreater(aSelection))
                {
                    return sal_False;
                }
            }

        }

        if (pSdrView->MouseButtonDown( aMEvt, this ) )
        {
            pSdrView->MouseButtonUp( aMEvt, this );
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);
            return sal_True;
        }
    }
    rSh.ResetCursorStack();

    if ( EnterDrawMode( aMEvt, aDocPos ) )
    {
        return sal_True;
    }
    if ( rView.GetDrawFuncPtr() && bInsFrm )
    {
        StopInsFrm();
        rSh.Edit();
    }

    UpdatePointer( aDocPos, 0 );

    if( !rSh.IsSelFrmMode() &&
        !GetView().GetViewFrame()->GetDispatcher()->IsLocked() )
    {
        // Test if there is a draw object at that position and if it should be selected.
        sal_Bool bShould = rSh.ShouldObjectBeSelected(aDocPos);

        if(bShould)
        {
            rView.NoRotate();
            rSh.HideCrsr();

            sal_Bool bUnLockView = !rSh.IsViewLocked();
            rSh.LockView( sal_True );
            sal_Bool bSelObj = rSh.SelectObj( aDocPos, 0);
            if( bUnLockView )
                rSh.LockView( sal_False );

            if( bSelObj )
            {
                bRet = sal_True;
                // in case the frame was deselected in the macro
                // just the cursor has to be displayed again.
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
                    bFrmDrag = sal_True;
                    UpdatePointer( aDocPos, 0 );
                    return bRet;
                }
            }

            if (!rView.GetDrawFuncPtr())
                rSh.ShowCrsr();
        }
    }
    else if ( rSh.IsSelFrmMode() &&
              (aActHitType == SDRHIT_NONE ||
               !bIsInsideSelectedObj))
    {
        rView.NoRotate();
        sal_Bool bUnLockView = !rSh.IsViewLocked();
        rSh.LockView( sal_True );
        sal_uInt8 nFlag = 0;

        if ( rSh.IsSelFrmMode() )
        {
            rSh.UnSelectFrm();
            rSh.LeaveSelFrmMode();
            rView.AttrChangedNotify(&rSh);
            bRet = sal_True;
        }

        sal_Bool bSelObj = rSh.SelectObj( aDocPos, nFlag );
        if( bUnLockView )
            rSh.LockView( sal_False );

        if( !bSelObj )
        {
            // move cursor here so that it is not drawn in the
            // frame at first; ShowCrsr() happens in LeaveSelFrmMode()
            bValidCrsrPos = !(CRSR_POSCHG & rSh.SetCursor(&aDocPos, false));
            rSh.LeaveSelFrmMode();
            rView.LeaveDrawCreate();
            rView.AttrChangedNotify( &rSh );
            bRet = sal_True;
        }
        else
        {
            rSh.HideCrsr();
            rSh.EnterSelFrmMode( &aDocPos );
            rSh.SelFlyGrabCrsr();
            rSh.MakeSelVisible();
            bFrmDrag = sal_True;
            if( rSh.IsFrmSelected() &&
                rView.GetDrawFuncPtr() )
            {
                rView.GetDrawFuncPtr()->Deactivate();
                rView.SetDrawFuncPtr(NULL);
                rView.LeaveDrawCreate();
                rView.AttrChangedNotify( &rSh );
            }
            UpdatePointer( aDocPos, 0 );
            bRet = sal_True;
        }
    }
    else if ( rSh.IsSelFrmMode() && bIsInsideSelectedObj )
    {
        // ## object at the mouse cursor is already selected - do nothing
        return sal_False;
    }

    if ( rSh.IsGCAttr() )
    {
        rSh.GCAttr();
        rSh.ClearGCAttr();
    }

    sal_Bool bOverSelect = rSh.ChgCurrPam( aDocPos ), bOverURLGrf = sal_False;
    if( !bOverSelect )
        bOverURLGrf = bOverSelect = 0 != rSh.IsURLGrfAtPos( aDocPos );

    if ( !bOverSelect )
    {
        {   // create only temporary move context because otherwise
            // the query against the content form doesn't work!!!
            SwMvContext aMvContext( &rSh );
            rSh.SetCursor(&aDocPos, false);
            bRet = sal_True;
        }
    }
    if( !bOverURLGrf )
    {
        const int nSelType = rSh.GetSelectionType();
        if( nSelType == nsSelectionType::SEL_OLE ||
            nSelType == nsSelectionType::SEL_GRF )
        {
            SwMvContext aMvContext( &rSh );
            if( !rSh.IsFrmSelected() )
                rSh.GotoNextFly();
            rSh.EnterSelFrmMode();
            bRet = sal_True;
        }
    }
    return bRet;
}

static SfxShell* lcl_GetShellFromDispatcher( SwView& rView, TypeId nType )
{
    // determine Shell
    SfxShell* pShell;
    SfxDispatcher* pDispatcher = rView.GetViewFrame()->GetDispatcher();
    for(sal_uInt16  i = 0; sal_True; ++i )
    {
        pShell = pDispatcher->GetShell( i );
        if( !pShell || pShell->IsA( nType ) )
            break;
    }
    return pShell;
}

IMPL_LINK_NOARG(SwEditWin, KeyInputFlushHandler)
{
    FlushInBuffer();
    return 0;
}

IMPL_LINK_NOARG(SwEditWin, KeyInputTimerHandler)
{
    bTblInsDelMode = sal_False;
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
/* i#3370 - remove quick help to prevent saving
 * of autocorrection suggestions */
void SwEditWin::StopQuickHelp()
{
    if( HasFocus() && pQuickHlpData && pQuickHlpData->m_bIsDisplayed  )
        pQuickHlpData->Stop( rView.GetWrtShell() );
}

IMPL_LINK_NOARG(SwEditWin, TemplateTimerHdl)
{
    SetApplyTemplate(SwApplyTemplate());
    return 0;
}

void SwEditWin::SetChainMode( sal_Bool bOn )
{
    if ( !bChainMode )
        StopInsFrm();

    if ( pUserMarker )
    {
        delete pUserMarker;
        pUserMarker = 0L;
    }

    bChainMode = bOn;

    static sal_uInt16 aInva[] =
    {
        FN_FRAME_CHAIN, FN_FRAME_UNCHAIN, 0
    };
    rView.GetViewFrame()->GetBindings().Invalidate(aInva);
}

uno::Reference< ::com::sun::star::accessibility::XAccessible > SwEditWin::CreateAccessible()
{
    SolarMutexGuard aGuard;   // this should have happend already!!!
    SwWrtShell *pSh = rView.GetWrtShellPtr();
    OSL_ENSURE( pSh, "no writer shell, no accessible object" );
    uno::Reference<
        ::com::sun::star::accessibility::XAccessible > xAcc;
    if( pSh )
        xAcc = pSh->CreateAccessible();

    return xAcc;
}

//-------------------------------------------------------------

void QuickHelpData::Move( QuickHelpData& rCpy )
{
    m_aHelpStrings.clear();
    m_aHelpStrings.swap( rCpy.m_aHelpStrings );

    m_bIsDisplayed = rCpy.m_bIsDisplayed;
    nLen = rCpy.nLen;
    nCurArrPos = rCpy.nCurArrPos;
    m_bAppendSpace = rCpy.m_bAppendSpace;
    m_bIsTip = rCpy.m_bIsTip;
    m_bIsAutoText = rCpy.m_bIsAutoText;
}

void QuickHelpData::ClearCntnt()
{
    nLen = nCurArrPos = 0;
    m_bIsDisplayed = m_bAppendSpace = false;
    nTipId = 0;
    m_aHelpStrings.clear();
    m_bIsTip = true;
    m_bIsAutoText = true;
}

void QuickHelpData::Start( SwWrtShell& rSh, sal_uInt16 nWrdLen )
{
    if( USHRT_MAX != nWrdLen )
    {
        nLen = nWrdLen;
        nCurArrPos = 0;
    }
    m_bIsDisplayed = true;

    Window& rWin = rSh.GetView().GetEditWin();
    if( m_bIsTip )
    {
        Point aPt( rWin.OutputToScreenPixel( rWin.LogicToPixel(
                    rSh.GetCharRect().Pos() )));
        aPt.Y() -= 3;
        nTipId = Help::ShowTip( &rWin, Rectangle( aPt, Size( 1, 1 )),
                        m_aHelpStrings[ nCurArrPos ],
                        QUICKHELP_LEFT | QUICKHELP_BOTTOM );
    }
    else
    {
        String sStr( m_aHelpStrings[ nCurArrPos ] );
        sStr.Erase( 0, nLen );
        sal_uInt16 nL = sStr.Len();
        const sal_uInt16 nVal = EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE |
                                EXTTEXTINPUT_ATTR_HIGHLIGHT;
        const std::vector<sal_uInt16> aAttrs( nL, nVal );
        CommandExtTextInputData aCETID( sStr, &aAttrs[0], nL,
                                        0, 0, 0, sal_False );

        //fdo#33092. If the current input language is the default
        //language that text would appear in if typed, then don't
        //force a language on for the ExtTextInput.
        LanguageType eInputLanguage = rWin.GetInputLanguage();
        if (lcl_isNonDefaultLanguage(eInputLanguage,
            rSh.GetView(), sStr) == INVALID_HINT)
        {
            eInputLanguage = LANGUAGE_DONTKNOW;
        }

        rSh.CreateExtTextInput(eInputLanguage);
        rSh.SetExtTextInputData( aCETID );
    }
}

void QuickHelpData::Stop( SwWrtShell& rSh )
{
    if( !m_bIsTip )
        rSh.DeleteExtTextInput( 0, sal_False );
    else if( nTipId )
        Help::HideTip( nTipId );
    ClearCntnt();
}

void QuickHelpData::FillStrArr( SwWrtShell& rSh, const String& rWord, bool bIgnoreCurrentPos )
{
    enum Capitalization { CASE_LOWER, CASE_UPPER, CASE_SENTENCE, CASE_OTHER };

    // Determine word capitalization
    const CharClass& rCC = GetAppCharClass();
    const String sWordLower = rCC.lowercase( rWord );
    Capitalization aWordCase = CASE_OTHER;
    if ( rWord.Len() > 0 )
    {
        if ( rWord.GetChar(0) == sWordLower.GetChar(0) )
        {
            if ( rWord == sWordLower )
                aWordCase = CASE_LOWER;
        }
        else
        {
            // First character is not lower case i.e. assume upper or title case
            String sWordSentence = sWordLower;
            sWordSentence.SetChar( 0, rWord.GetChar(0) );
            if ( rWord == sWordSentence )
                aWordCase = CASE_SENTENCE;
            else
            {
                if ( rWord == static_cast<String>( rCC.uppercase( rWord ) ) )
                    aWordCase = CASE_UPPER;
            }
        }
    }

    salhelper::SingletonRef<SwCalendarWrapper>* pCalendar = s_getCalendarWrapper();
    (*pCalendar)->LoadDefaultCalendar( rSh.GetCurLang() );

    // Add matching calendar month and day names
    uno::Sequence< i18n::CalendarItem2 > aNames( (*pCalendar)->getMonths() );
    for ( sal_uInt16 i = 0; i < 2; ++i )
    {
        for ( long n = 0; n < aNames.getLength(); ++n )
        {
            const String& rStr( aNames[n].FullName );
            // Check string longer than word and case insensitive match
            if( rStr.Len() > rWord.Len() &&
                static_cast<String>( rCC.lowercase( rStr, 0, rWord.Len() ) )
                == sWordLower )
            {
                if ( aWordCase == CASE_LOWER )
                    m_aHelpStrings.push_back( rCC.lowercase( rStr ) );
                else if ( aWordCase == CASE_SENTENCE )
                {
                    String sTmp = rCC.lowercase( rStr );
                    sTmp.SetChar( 0, rStr.GetChar(0) );
                    m_aHelpStrings.push_back( sTmp );
                }
                else if ( aWordCase == CASE_UPPER )
                    m_aHelpStrings.push_back( rCC.uppercase( rStr ) );
                else // CASE_OTHER - use retrieved capitalization
                    m_aHelpStrings.push_back( rStr );
            }
        }
        // Data for second loop iteration
        if ( i == 0 )
            aNames = (*pCalendar)->getDays();
    }

    // Add matching words from AutoCompleteWord list
    const SwAutoCompleteWord& rACList = rSh.GetAutoCompleteWords();
    std::vector<String> strings;

    if ( rACList.GetWordsMatching( rWord, strings, bIgnoreCurrentPos ) )
    {
        for (unsigned int i= 0; i<strings.size(); i++)
        {
            String aCompletedString = strings[i];
            if ( aWordCase == CASE_LOWER )
                m_aHelpStrings.push_back( rCC.lowercase( aCompletedString ) );
            else if ( aWordCase == CASE_SENTENCE )
            {
                String sTmp = rCC.lowercase( aCompletedString );
                sTmp.SetChar( 0, aCompletedString.GetChar(0) );
                m_aHelpStrings.push_back( sTmp );
            }
            else if ( aWordCase == CASE_UPPER )
                m_aHelpStrings.push_back( rCC.uppercase( aCompletedString ) );
            else // CASE_OTHER - use retrieved capitalization
                m_aHelpStrings.push_back( aCompletedString );
        }
    }

}

namespace {

struct CompareIgnoreCaseAscii
{
    bool operator()(const String& s1, const String& s2) const
    {
        return s1.CompareIgnoreCaseToAscii(s2) == COMPARE_LESS;
    }
};

struct EqualIgnoreCaseAscii
{
    bool operator()(const String& s1, const String& s2) const
    {
        return s1.CompareIgnoreCaseToAscii(s2) == COMPARE_EQUAL;
    }
};

} // anonymous namespace

// TODO - implement an i18n aware sort
void QuickHelpData::SortAndFilter()
{
    std::sort( m_aHelpStrings.begin(),
               m_aHelpStrings.end(),
               CompareIgnoreCaseAscii() );

    std::vector<String>::iterator it = std::unique( m_aHelpStrings.begin(),
                                                    m_aHelpStrings.end(),
                                                    EqualIgnoreCaseAscii() );
    m_aHelpStrings.erase( it, m_aHelpStrings.end() );

    nCurArrPos = 0;
}

void SwEditWin::ShowAutoTextCorrectQuickHelp(
        const String& rWord, SvxAutoCorrCfg* pACfg, SvxAutoCorrect* pACorr,
        bool bFromIME )
{
    SwWrtShell& rSh = rView.GetWrtShell();
    pQuickHlpData->ClearCntnt();
    if( pACfg->IsAutoTextTip() )
    {
        SwGlossaryList* pList = ::GetGlossaryList();
        pList->HasLongName( rWord, &pQuickHlpData->m_aHelpStrings );
    }

    if( pQuickHlpData->m_aHelpStrings.empty() &&
        pACorr->GetSwFlags().bAutoCompleteWords )
    {
        pQuickHlpData->m_bIsAutoText = false;
        pQuickHlpData->m_bIsTip = bFromIME ||
                    !pACorr ||
                    pACorr->GetSwFlags().bAutoCmpltShowAsTip;

        // Get the neccessary data to show help text.
        pQuickHlpData->FillStrArr( rSh, rWord, bFromIME );
    }


    if( !pQuickHlpData->m_aHelpStrings.empty() )
    {
        pQuickHlpData->SortAndFilter();
        pQuickHlpData->Start( rSh, rWord.Len() );
    }
}

bool SwEditWin::IsInHeaderFooter( const Point &rDocPt, FrameControlType &rControl ) const
{
    SwWrtShell &rSh = rView.GetWrtShell();
    const SwPageFrm* pPageFrm = rSh.GetLayout()->GetPageAtPos( rDocPt );

    if ( pPageFrm && pPageFrm->IsOverHeaderFooterArea( rDocPt, rControl ) )
        return true;

    if ( rSh.IsShowHeaderFooterSeparator( Header ) || rSh.IsShowHeaderFooterSeparator( Footer ) )
    {
        SwFrameControlsManager &rMgr = rSh.GetView().GetEditWin().GetFrameControlsManager();
        Point aPoint( LogicToPixel( rDocPt ) );

        if ( rSh.IsShowHeaderFooterSeparator( Header ) )
        {
            SwFrameControlPtr pControl = rMgr.GetControl( Header, pPageFrm );
            if ( pControl.get() && pControl->Contains( aPoint ) )
            {
                rControl = Header;
                return true;
            }
        }

        if ( rSh.IsShowHeaderFooterSeparator( Footer ) )
        {
            SwFrameControlPtr pControl = rMgr.GetControl( Footer, pPageFrm );
            if ( pControl.get() && pControl->Contains( aPoint ) )
            {
                rControl = Footer;
                return true;
            }
        }
    }

    return false;
}

void SwEditWin::SetUseInputLanguage( sal_Bool bNew )
{
    if ( bNew || bUseInputLanguage )
    {
        SfxBindings& rBind = GetView().GetViewFrame()->GetBindings();
        rBind.Invalidate( SID_ATTR_CHAR_FONT );
        rBind.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    }
    bUseInputLanguage = bNew;
}

rtl::OUString SwEditWin::GetSurroundingText() const
{
    String sReturn;
    SwWrtShell& rSh = rView.GetWrtShell();
    if( rSh.HasSelection() && !rSh.IsMultiSelection() && rSh.IsSelOnePara() )
        rSh.GetSelectedText( sReturn, GETSELTXT_PARABRK_TO_ONLYCR  );
    else if( !rSh.HasSelection() )
    {
        SwPosition *pPos = rSh.GetCrsr()->GetPoint();
        xub_StrLen nPos = pPos->nContent.GetIndex();

        // get the sentence around the cursor
        rSh.HideCrsr();
        rSh.GoStartSentence();
        rSh.SetMark();
        rSh.GoEndSentence();
        rSh.GetSelectedText( sReturn, GETSELTXT_PARABRK_TO_ONLYCR  );

        pPos->nContent = nPos;
        rSh.ClearMark();
        rSh.HideCrsr();
    }

    return sReturn;
}

Selection SwEditWin::GetSurroundingTextSelection() const
{
    SwWrtShell& rSh = rView.GetWrtShell();
    if( rSh.HasSelection() )
    {
        String sReturn;
        rSh.GetSelectedText( sReturn, GETSELTXT_PARABRK_TO_ONLYCR  );
        return Selection( 0, sReturn.Len() );
    }
    else
    {
        // Return the position of the visible cursor in the sentence
        // around the visible cursor.
        SwPosition *pPos = rSh.GetCrsr()->GetPoint();
        xub_StrLen nPos = pPos->nContent.GetIndex();

        rSh.HideCrsr();
        rSh.GoStartSentence();
        xub_StrLen nStartPos = rSh.GetCrsr()->GetPoint()->nContent.GetIndex();

        pPos->nContent = nPos;
        rSh.ClearMark();
        rSh.ShowCrsr();

        return Selection( nPos - nStartPos, nPos - nStartPos );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
