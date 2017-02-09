/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_features.h>

#include <swtypes.hxx>
#include <hintids.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>

#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>

#include <vcl/help.hxx>
#include <vcl/graph.hxx>
#include <vcl/msgbox.hxx>
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
#include <sfx2/htmlmode.hxx>
#include <svx/svdview.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/protitem.hxx>
#include <unotools/charclass.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>

#include <editeng/acorrcfg.hxx>
#include <SwSmartTagMgr.hxx>
#include <edtdd.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <textboxhelper.hxx>
#include <dcontact.hxx>
#include <fldbas.hxx>
#include <swmodule.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>
#include <drawbase.hxx>
#include <dselect.hxx>
#include <textsh.hxx>
#include <shdwcrsr.hxx>
#include <txatbase.hxx>
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
#include <txtfld.hxx>
#include <fmtfld.hxx>

#include <IMark.hxx>
#include <doc.hxx>
#include <xmloff/odffields.hxx>

#include <PostItMgr.hxx>
#include <SidebarWin.hxx>
#include <FrameControlsManager.hxx>

#include <algorithm>
#include <vector>

#include "../../core/inc/rootfrm.hxx"

#include <unotools/syslocaleoptions.hxx>
#include <memory>

using namespace sw::mark;
using namespace ::com::sun::star;

/**
 * Globals
 */
static bool g_bInputLanguageSwitched = false;

// Usually in MouseButtonUp a selection is revoked when the selection is
// not currently being pulled open. Unfortunately in MouseButtonDown there
// is being selected at double/triple click. That selection is completely
// finished in the Handler and thus can't be distinguished in the Up.
// To resolve this g_bHoldSelection is set in Down at evaluated in Up.
static bool g_bHoldSelection      = false;

bool g_bFrameDrag                   = false;
static bool g_bValidCursorPos       = false;
static bool g_bModePushed         = false;
bool g_bDDTimerStarted            = false;
bool g_bFlushCharBuffer           = false;
bool g_bDDINetAttr                = false;
static SdrHdlKind g_eSdrMoveHdl   = SdrHdlKind::User;

QuickHelpData* SwEditWin::m_pQuickHlpData = nullptr;

long    SwEditWin::m_nDDStartPosY = 0;
long    SwEditWin::m_nDDStartPosX = 0;

static SfxShell* lcl_GetTextShellFromDispatcher( SwView& rView );

/// Check if the selected shape has a TextBox: if so, go into that instead.
static bool lcl_goIntoTextBox(SwEditWin& rEditWin, SwWrtShell& rSh)
{
    SdrObject* pSdrObject = rSh.GetDrawView()->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
    SwFrameFormat* pObjectFormat = ::FindFrameFormat(pSdrObject);
    if (SwFrameFormat* pTextBoxFormat = SwTextBoxHelper::getOtherTextBoxFormat(pObjectFormat, RES_DRAWFRMFMT))
    {
        SdrObject* pTextBox = pTextBoxFormat->FindRealSdrObject();
        SdrView* pSdrView = rSh.GetDrawView();
        // Unmark the shape.
        pSdrView->UnmarkAllObj();
        // Mark the textbox.
        rSh.SelectObj(Point(), SW_ALLOW_TEXTBOX, pTextBox);
        // Clear the DrawFuncPtr.
        rEditWin.StopInsFrame();
        return true;
    }
    return false;
}

class SwAnchorMarker
{
    SdrHdl* pHdl;
    Point aHdlPos;
    Point aLastPos;
    bool bTopRightHandle;
public:
    explicit SwAnchorMarker( SdrHdl* pH )
        : pHdl( pH )
        , aHdlPos( pH->GetPos() )
        , aLastPos( pH->GetPos() )
        , bTopRightHandle( pH->GetKind() == SdrHdlKind::Anchor_TR )
    {}
    const Point& GetLastPos() const { return aLastPos; }
    void SetLastPos( const Point& rNew ) { aLastPos = rNew; }
    void SetPos( const Point& rNew ) { pHdl->SetPos( rNew ); }
    const Point& GetHdlPos() { return aHdlPos; }
    SdrHdl* GetHdl() const { return pHdl; }
    void ChgHdl( SdrHdl* pNew )
    {
        pHdl = pNew;
        if ( pHdl )
        {
            bTopRightHandle = (pHdl->GetKind() == SdrHdlKind::Anchor_TR);
        }
    }
    const Point GetPosForHitTest( const OutputDevice& rOut )
    {
        Point aHitTestPos( pHdl->GetPos() );
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
    std::vector<OUString> m_aHelpStrings;
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

    QuickHelpData() { ClearContent(); }

    void Move( QuickHelpData& rCpy );
    void ClearContent();
    void Start( SwWrtShell& rSh, sal_uInt16 nWrdLen );
    void Stop( SwWrtShell& rSh );

    bool HasContent() const { return !m_aHelpStrings.empty() && 0 != nLen; }

    /// Next help string.
    void Next( bool bEndLess )
    {
        if( ++nCurArrPos >= m_aHelpStrings.size() )
            nCurArrPos = (bEndLess && !m_bIsAutoText ) ? 0 : nCurArrPos-1;
    }
    /// Previous help string.
    void Previous( bool bEndLess )
    {
        if( 0 == nCurArrPos-- )
            nCurArrPos = (bEndLess && !m_bIsAutoText ) ? m_aHelpStrings.size()-1 : 0;
    }

    // Fills internal structures with hopefully helpful information.
    void FillStrArr( SwWrtShell& rSh, const OUString& rWord );
    void SortAndFilter(const OUString &rOrigWord);
};

/**
 * Avoid minimal movement shiver
 */
#define HIT_PIX  2 /* hit tolerance in pixel  */
#define MIN_MOVE 4

inline bool IsMinMove(const Point &rStartPos, const Point &rLPt)
{
    return std::abs(rStartPos.X() - rLPt.X()) > MIN_MOVE ||
           std::abs(rStartPos.Y() - rLPt.Y()) > MIN_MOVE;
}

/**
 * For MouseButtonDown - determine whether a DrawObject
 * an NO SwgFrame was hit! Shift/Ctrl should only result
 * in selecting, with DrawObjects; at SwgFlys to trigger
 * hyperlinks if applicable (Download/NewWindow!)
 */
inline bool IsDrawObjSelectable( const SwWrtShell& rSh, const Point& rPt )
{
    bool bRet = true;
    SdrObject* pObj;
    switch( rSh.GetObjCntType( rPt, pObj ))
    {
    case OBJCNT_NONE:
    case OBJCNT_FLY:
    case OBJCNT_GRF:
    case OBJCNT_OLE:
        bRet = false;
        break;
    default:; //prevent warning
    }
    return bRet;
}

/*
 * Switch pointer
 */
void SwEditWin::UpdatePointer(const Point &rLPt, sal_uInt16 nModifier )
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    if( m_pApplyTempl )
    {
        PointerStyle eStyle = PointerStyle::Fill;
        if ( rSh.IsOverReadOnlyPos( rLPt ) )
        {
            delete m_pUserMarker;
            m_pUserMarker = nullptr;

            eStyle = PointerStyle::NotAllowed;
        }
        else
        {
            SwRect aRect;
            SwRect* pRect = &aRect;
            const SwFrameFormat* pFormat = nullptr;

            bool bFrameIsValidTarget = false;
            if( m_pApplyTempl->m_pFormatClipboard )
                bFrameIsValidTarget = m_pApplyTempl->m_pFormatClipboard->HasContentForThisType( nsSelectionType::SEL_FRM );
            else if( !m_pApplyTempl->nColor )
                bFrameIsValidTarget = ( m_pApplyTempl->eType == SfxStyleFamily::Frame );

            if( bFrameIsValidTarget &&
                        nullptr !=(pFormat = rSh.GetFormatFromObj( rLPt, &pRect )) &&
                        dynamic_cast<const SwFlyFrameFormat*>( pFormat) )
            {
                //turn on highlight for frame
                Rectangle aTmp( pRect->SVRect() );

                if ( !m_pUserMarker )
                {
                    m_pUserMarker = new SdrDropMarkerOverlay( *rSh.GetDrawView(), aTmp );
                }
            }
            else
            {
                delete m_pUserMarker;
                m_pUserMarker = nullptr;
            }

            rSh.SwCursorShell::SetVisibleCursor( rLPt );
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
        SwChainRet nChainable = rSh.Chainable( aRect, *rSh.GetFlyFrameFormat(), rLPt );
        PointerStyle eStyle = nChainable != SwChainRet::OK
                ? PointerStyle::ChainNotAllowed : PointerStyle::Chain;
        if ( nChainable == SwChainRet::OK )
        {
            Rectangle aTmp( aRect.SVRect() );

            if ( !m_pUserMarker )
            {
                m_pUserMarker = new SdrDropMarkerOverlay( *rSh.GetDrawView(), aTmp );
            }
        }
        else
        {
            delete m_pUserMarker;
            m_pUserMarker = nullptr;
        }

        SetPointer( eStyle );
        return;
    }

    bool bExecHyperlinks = m_rView.GetDocShell()->IsReadOnly();
    if ( !bExecHyperlinks )
    {
        SvtSecurityOptions aSecOpts;
        const bool bSecureOption = aSecOpts.IsOptionSet( SvtSecurityOptions::EOption::CtrlClickHyperlink );
        if ( (  bSecureOption && nModifier == KEY_MOD1 ) ||
             ( !bSecureOption && nModifier != KEY_MOD1 ) )
            bExecHyperlinks = true;
    }

    const bool bExecSmarttags  = nModifier == KEY_MOD1;

    SdrView *pSdrView = rSh.GetDrawView();
    bool bPrefSdrPointer = false;
    bool bHitHandle = false;
    bool bCntAtPos = false;
    bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly() &&
                          rSh.IsCursorReadonly();
    m_aActHitType = SdrHitKind::NONE;
    PointerStyle eStyle = PointerStyle::Text;
    if ( !pSdrView )
        bCntAtPos = true;
    else if ( (bHitHandle = (pSdrView->PickHandle(rLPt) != nullptr)) )
    {
        m_aActHitType = SdrHitKind::Object;
        bPrefSdrPointer = true;
    }
    else
    {
        const bool bNotInSelObj = !rSh.IsInsideSelectedObj( rLPt );
        if ( m_rView.GetDrawFuncPtr() && !m_bInsDraw && bNotInSelObj )
        {
            m_aActHitType = SdrHitKind::Object;
            if (IsObjectSelect())
                eStyle = PointerStyle::Arrow;
            else
                bPrefSdrPointer = true;
        }
        else
        {
            SdrPageView* pPV = nullptr;
            pSdrView->SetHitTolerancePixel( HIT_PIX );
            SdrObject* pObj  = (bNotInSelObj && bExecHyperlinks) ?
                 pSdrView->PickObj(rLPt, pSdrView->getHitTolLog(), pPV, SdrSearchOptions::PICKMACRO) :
                 nullptr;
            if (pObj)
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
                        m_aActHitType = SdrHitKind::NONE;
                        bPrefSdrPointer = true;
                    }
                    else
                    {
                        SdrViewEvent aVEvt;
                        SdrHitKind eHit = pSdrView->PickAnything(rLPt, aVEvt);

                        if (eHit == SdrHitKind::UrlField && bExecHyperlinks)
                        {
                            m_aActHitType = SdrHitKind::Object;
                            bPrefSdrPointer = true;
                        }
                        else
                        {
                            // if we're over a selected object, we show an
                            // ARROW by default. We only show a MOVE if 1) the
                            // object is selected, and 2) it may be moved
                            // (i.e., position is not protected).
                            bool bMovable =
                                (!bNotInSelObj) &&
                                (rSh.IsObjSelected() || rSh.IsFrameSelected()) &&
                                (rSh.IsSelObjProtected(FlyProtectFlags::Pos) == FlyProtectFlags::NONE);

                            SdrObject* pSelectableObj = rSh.GetObjAt(rLPt);
                            // Don't update pointer if this is a background image only.
                            if (pSelectableObj->GetLayer() != rSh.GetDoc()->getIDocumentDrawModelAccess().GetHellId())
                                eStyle = bMovable ? PointerStyle::Move : PointerStyle::Arrow;
                            m_aActHitType = SdrHitKind::Object;
                        }
                    }
                }
                else
                {
                    if ( rSh.IsFrameSelected() && !bNotInSelObj )
                    {
                        // dvo: this branch appears to be dead and should be
                        // removed in a future version. Reason: The condition
                        // !bNotInSelObj means that this branch will only be
                        // executed in the cursor points inside a selected
                        // object. However, if this is the case, the previous
                        // if( rSh.IsObjSelectable(rLPt) ) must always be true:
                        // rLPt is inside a selected object, then obviously
                        // rLPt is over a selectable object.
                        if (rSh.IsSelObjProtected(FlyProtectFlags::Size) != FlyProtectFlags::NONE)
                            eStyle = PointerStyle::NotAllowed;
                        else
                            eStyle = PointerStyle::Move;
                        m_aActHitType = SdrHitKind::Object;
                    }
                    else
                    {
                        if ( m_rView.GetDrawFuncPtr() )
                            bPrefSdrPointer = true;
                        else
                            bCntAtPos = true;
                    }
                }
            }
        }
    }
    if ( bPrefSdrPointer )
    {
        if (bIsDocReadOnly || (rSh.IsObjSelected() && rSh.IsSelObjProtected(FlyProtectFlags::Content) != FlyProtectFlags::NONE))
            SetPointer( PointerStyle::NotAllowed );
        else
        {
            if (m_rView.GetDrawFuncPtr() && m_rView.GetDrawFuncPtr()->IsInsertForm() && !bHitHandle)
                SetPointer( PointerStyle::DrawRect );
            else
                SetPointer( pSdrView->GetPreferredPointer( rLPt, rSh.GetOut() ) );
        }
    }
    else
    {
        if( !rSh.IsPageAtPos( rLPt ) || m_pAnchorMarker )
            eStyle = PointerStyle::Arrow;
        else
        {
            // Even if we already have something, prefer URLs if possible.
            SwContentAtPos aUrlPos(SwContentAtPos::SW_INETATTR);
            if (bCntAtPos || rSh.GetContentAtPos(rLPt, aUrlPos))
            {
                SwContentAtPos aSwContentAtPos(
                    SwContentAtPos::SW_FIELD |
                    SwContentAtPos::SW_CLICKFIELD |
                    SwContentAtPos::SW_INETATTR |
                    SwContentAtPos::SW_FTN |
                    SwContentAtPos::SW_SMARTTAG );
                if( rSh.GetContentAtPos( rLPt, aSwContentAtPos) )
                {
                    // Is edit inline input field
                    if (SwContentAtPos::SW_FIELD == aSwContentAtPos.eContentAtPos)
                    {
                        if ( aSwContentAtPos.pFndTextAttr != nullptr
                            && aSwContentAtPos.pFndTextAttr->Which() == RES_TXTATR_INPUTFIELD)
                        {
                            const SwField *pCursorField = rSh.CursorInsideInputField() ? rSh.GetCurField( true ) : nullptr;
                            if (!(pCursorField && pCursorField == aSwContentAtPos.pFndTextAttr->GetFormatField().GetField()))
                                eStyle = PointerStyle::RefHand;
                        }
                    }
                    else
                    {
                        const bool bClickToFollow = SwContentAtPos::SW_INETATTR == aSwContentAtPos.eContentAtPos ||
                                                    SwContentAtPos::SW_SMARTTAG == aSwContentAtPos.eContentAtPos;
                        if( !bClickToFollow ||
                            (SwContentAtPos::SW_INETATTR == aSwContentAtPos.eContentAtPos && bExecHyperlinks) ||
                            (SwContentAtPos::SW_SMARTTAG == aSwContentAtPos.eContentAtPos && bExecSmarttags) )
                            eStyle = PointerStyle::RefHand;
                    }
                }
            }
        }

        // which kind of text pointer have we to show - horz / vert - ?
        if( PointerStyle::Text == eStyle && rSh.IsInVerticalText( &rLPt ))
            eStyle = PointerStyle::TextVertical;
        else if (rSh.GetViewOptions()->CanHideWhitespace() &&
                 rSh.GetLayout()->IsBetweenPages(rLPt))
        {
            if (rSh.GetViewOptions()->IsHideWhitespaceMode())
                eStyle = PointerStyle::ShowWhitespace;
            else
                eStyle = PointerStyle::HideWhitespace;
        }

        SetPointer( eStyle );
    }
}

/**
 * Increase timer for selection
 */
IMPL_LINK_NOARG(SwEditWin, TimerHandler, Timer *, void)
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    Point aModPt( m_aMovePos );
    const SwRect aOldVis( rSh.VisArea() );
    bool bDone = false;

    if ( !rSh.VisArea().IsInside( aModPt ) )
    {
        if ( m_bInsDraw )
        {
            const int nMaxScroll = 40;
            m_rView.Scroll( Rectangle(aModPt,Size(1,1)), nMaxScroll, nMaxScroll);
            bDone = true;
        }
        else if ( g_bFrameDrag )
        {
            rSh.Drag(&aModPt, false);
            bDone = true;
        }
        if ( !bDone )
            aModPt = rSh.GetContentPos( aModPt,aModPt.Y() > rSh.VisArea().Bottom() );
    }
    if ( !bDone && !(g_bFrameDrag || m_bInsDraw) )
    {
        if ( m_pRowColumnSelectionStart )
        {
            Point aPos( aModPt );
            rSh.SelectTableRowCol( *m_pRowColumnSelectionStart, &aPos, m_bIsRowDrag );
        }
        else
            rSh.CallSetCursor( &aModPt, false );

        // It can be that a "jump" over a table cannot be accomplished like
        // that. So we jump over the table by Up/Down here.
        const SwRect& rVisArea = rSh.VisArea();
        if( aOldVis == rVisArea && !rSh.IsStartOfDoc() && !rSh.IsEndOfDoc() )
        {
            // take the center point of VisArea to
            // decide in which direction the user want.
            if( aModPt.Y() < ( rVisArea.Top() + rVisArea.Height() / 2 ) )
                rSh.Up( true );
            else
                rSh.Down( true );
        }
    }

    m_aMovePos += rSh.VisArea().Pos() - aOldVis.Pos();
    JustifyAreaTimer();
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
         nDiff = std::max(
         std::max( m_aMovePos.Y() - rVisArea.Bottom(), rVisArea.Top() - m_aMovePos.Y() ),
         std::max( m_aMovePos.X() - rVisArea.Right(),  rVisArea.Left() - m_aMovePos.X()));
    m_aTimer.SetTimeout( std::max( coMinLen, nTimeout - nDiff*2L) );
}

void SwEditWin::LeaveArea(const Point &rPos)
{
    m_aMovePos = rPos;
    JustifyAreaTimer();
    if( !m_aTimer.IsActive() )
        m_aTimer.Start();
    delete m_pShadCursor;
    m_pShadCursor = nullptr;
}

inline void SwEditWin::EnterArea()
{
    m_aTimer.Stop();
}

/**
 * Insert mode for frames
 */
void SwEditWin::InsFrame(sal_uInt16 nCols)
{
    StdDrawMode( OBJ_NONE, false );
    m_bInsFrame = true;
    m_nInsFrameColCount = nCols;
}

void SwEditWin::StdDrawMode( SdrObjKind eSdrObjectKind, bool bObjSelect )
{
    SetSdrDrawMode( eSdrObjectKind );

    if (bObjSelect)
        m_rView.SetDrawFuncPtr(new DrawSelection( &m_rView.GetWrtShell(), this, &m_rView ));
    else
        m_rView.SetDrawFuncPtr(new SwDrawBase( &m_rView.GetWrtShell(), this, &m_rView ));

    m_rView.SetSelDrawSlot();
    SetSdrDrawMode( eSdrObjectKind );
    if (bObjSelect)
        m_rView.GetDrawFuncPtr()->Activate( SID_OBJECT_SELECT );
    else
        m_rView.GetDrawFuncPtr()->Activate( sal::static_int_cast< sal_uInt16 >(eSdrObjectKind) );
    m_bInsFrame = false;
    m_nInsFrameColCount = 1;
}

void SwEditWin::StopInsFrame()
{
    if (m_rView.GetDrawFuncPtr())
    {
        m_rView.GetDrawFuncPtr()->Deactivate();
        m_rView.SetDrawFuncPtr(nullptr);
    }
    m_rView.LeaveDrawCreate();    // leave construction mode
    m_bInsFrame = false;
    m_nInsFrameColCount = 1;
}

bool SwEditWin::IsInputSequenceCheckingRequired( const OUString &rText, const SwPaM& rCursor )
{
    const SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();
    if ( !rCTLOptions.IsCTLFontEnabled() ||
         !rCTLOptions.IsCTLSequenceChecking() )
         return false;

    if ( 0 == rCursor.Start()->nContent.GetIndex() ) /* first char needs not to be checked */
        return false;

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

    return (0 <= nCTLScriptPos && nCTLScriptPos <= rText.getLength());
}

//return INVALID_HINT if language should not be explicitly overridden, the correct
//HintId to use for the eBufferLanguage otherwise
static sal_uInt16 lcl_isNonDefaultLanguage(LanguageType eBufferLanguage, SwView& rView,
    const OUString &rInBuffer)
{
    sal_uInt16 nWhich = INVALID_HINT;

    //If the option to IgnoreLanguageChange is set, short-circuit this method
    //which results in the document/paragraph language remaining the same
    //despite a change to the keyboard/input language
    SvtSysLocaleOptions aSysLocaleOptions;
    if(aSysLocaleOptions.IsIgnoreLanguageChange())
    {
        return INVALID_HINT;
    }

    bool bLang = true;
    if(eBufferLanguage != LANGUAGE_DONTKNOW)
    {
        switch( SvtLanguageOptions::GetI18NScriptTypeOfLanguage( eBufferLanguage ))
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
            if(SfxItemState::DEFAULT <= aLangSet.GetItemState(nWhich))
            {
                LanguageType eLang = static_cast<const SvxLanguageItem&>(aLangSet.Get(nWhich)).GetLanguage();
                if ( eLang == eBufferLanguage )
                {
                    // current language attribute equal to language reported from system
                    bLang = false;
                }
                else if ( !g_bInputLanguageSwitched && RES_CHRATR_LANGUAGE == nWhich )
                {
                    // special case: switching between two "LATIN" languages
                    // In case the current keyboard setting might be suitable
                    // for both languages we can't safely assume that the user
                    // wants to use the language reported from the system,
                    // except if we knew that it was explicitly switched (thus
                    // the check for "bInputLangeSwitched").

                    // The language reported by the system could be just the
                    // system default language that the user is not even aware
                    // of, because no language selection tool is installed at
                    // all. In this case the OOo language should get preference
                    // as it might have been selected by the user explicitly.

                    // Usually this case happens if the OOo language is
                    // different to the system language but the system keyboard
                    // is still suitable for the OOo language (e.g. writing
                    // English texts with a German keyboard).

                    // For non-latin keyboards overwriting the attribute is
                    // still valid. We do this for cyrillic and greek ATM.  In
                    // future versions of OOo this should be replaced by a
                    // configuration switch that allows to give the preference
                    // to the OOo setting or the system setting explicitly
                    // and/or a better handling of the script type.
                    i18n::UnicodeScript eType = !rInBuffer.isEmpty() ?
                        (i18n::UnicodeScript)GetAppCharClass().getScript( rInBuffer, 0 ) :
                        i18n::UnicodeScript_kScriptCount;

                    bool bSystemIsNonLatin = false;
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

                    bool bOOoLangIsNonLatin = MsLangId::isNonLatinWestern( eLang);

                    bLang = (bSystemIsNonLatin != bOOoLangIsNonLatin);
                }
            }
        }
    }
    return bLang ? nWhich : INVALID_HINT;
}

/**
 * Character buffer is inserted into the document
 */
void SwEditWin::FlushInBuffer()
{
    if ( !m_aInBuffer.isEmpty() )
    {
        SwWrtShell& rSh = m_rView.GetWrtShell();

        // generate new sequence input checker if not already done
        if ( !pCheckIt )
            pCheckIt = new SwCheckIt;

        uno::Reference < i18n::XExtendedInputSequenceChecker > xISC = pCheckIt->xCheck;
        if ( xISC.is() && IsInputSequenceCheckingRequired( m_aInBuffer, *rSh.GetCursor() ) )
        {

            // apply (Thai) input sequence checking/correction

            rSh.Push(); // push current cursor to stack

            // get text from the beginning (i.e left side) of current selection
            // to the start of the paragraph
            rSh.NormalizePam();     // make point be the first (left) one
            if (!rSh.GetCursor()->HasMark())
                rSh.GetCursor()->SetMark();
            rSh.GetCursor()->GetMark()->nContent = 0;

            const OUString aOldText( rSh.GetCursor()->GetText() );
            const sal_Int32 nOldLen = aOldText.getLength();

            SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();

            sal_Int32 nExpandSelection = 0;
            if (nOldLen > 0)
            {
                sal_Int32 nTmpPos = nOldLen;
                sal_Int16 nCheckMode = rCTLOptions.IsCTLSequenceCheckingRestricted() ?
                        i18n::InputSequenceCheckMode::STRICT : i18n::InputSequenceCheckMode::BASIC;

                OUString aNewText( aOldText );
                if (rCTLOptions.IsCTLSequenceCheckingTypeAndReplace())
                {
                    for( sal_Int32 k = 0;  k < m_aInBuffer.getLength();  ++k)
                    {
                        const sal_Unicode cChar = m_aInBuffer[k];
                        const sal_Int32 nPrevPos =xISC->correctInputSequence( aNewText, nTmpPos - 1, cChar, nCheckMode );

                        // valid sequence or sequence could be corrected:
                        if (nPrevPos != aNewText.getLength())
                            nTmpPos = nPrevPos + 1;
                    }

                    // find position of first character that has changed
                    sal_Int32 nNewLen = aNewText.getLength();
                    const sal_Unicode *pOldText = aOldText.getStr();
                    const sal_Unicode *pNewText = aNewText.getStr();
                    sal_Int32 nChgPos = 0;
                    while ( nChgPos < nOldLen && nChgPos < nNewLen &&
                            pOldText[nChgPos] == pNewText[nChgPos] )
                        ++nChgPos;

                    const sal_Int32 nChgLen = nNewLen - nChgPos;
                    if (nChgLen)
                    {
                        m_aInBuffer = aNewText.copy( nChgPos, nChgLen );
                        nExpandSelection = nOldLen - nChgPos;
                    }
                    else
                        m_aInBuffer.clear();
                }
                else
                {
                    for( sal_Int32 k = 0;  k < m_aInBuffer.getLength(); ++k )
                    {
                        const sal_Unicode cChar = m_aInBuffer[k];
                        if (xISC->checkInputSequence( aNewText, nTmpPos - 1, cChar, nCheckMode ))
                        {
                            // character can be inserted:
                            aNewText += OUStringLiteral1( cChar );
                            ++nTmpPos;
                        }
                    }
                    m_aInBuffer = aNewText.copy( aOldText.getLength() );  // copy new text to be inserted to buffer
                }
            }

            // at this point now we will insert the buffer text 'normally' some lines below...

            rSh.Pop( false );  // pop old cursor from stack

            if (m_aInBuffer.isEmpty())
                return;

            // if text prior to the original selection needs to be changed
            // as well, we now expand the selection accordingly.
            SwPaM &rCursor = *rSh.GetCursor();
            const sal_Int32 nCursorStartPos = rCursor.Start()->nContent.GetIndex();
            OSL_ENSURE( nCursorStartPos >= nExpandSelection, "cannot expand selection as specified!!" );
            if (nExpandSelection && nCursorStartPos >= nExpandSelection)
            {
                if (!rCursor.HasMark())
                    rCursor.SetMark();
                rCursor.Start()->nContent -= nExpandSelection;
            }
        }

        uno::Reference< frame::XDispatchRecorder > xRecorder =
                m_rView.GetViewFrame()->GetBindings().GetRecorder();
        if ( xRecorder.is() )
        {
            // determine shell
            SfxShell *pSfxShell = lcl_GetTextShellFromDispatcher( m_rView );
            // generate request and record
            if (pSfxShell)
            {
                SfxRequest aReq( m_rView.GetViewFrame(), FN_INSERT_STRING );
                aReq.AppendItem( SfxStringItem( FN_INSERT_STRING, m_aInBuffer ) );
                aReq.Done();
            }
        }

        sal_uInt16 nWhich = lcl_isNonDefaultLanguage(m_eBufferLanguage, m_rView, m_aInBuffer);
        if (nWhich != INVALID_HINT )
        {
            SvxLanguageItem aLangItem( m_eBufferLanguage, nWhich );
            rSh.SetAttrItem( aLangItem );
        }

        rSh.Insert( m_aInBuffer );
        m_eBufferLanguage = LANGUAGE_DONTKNOW;
        m_aInBuffer.clear();
        g_bFlushCharBuffer = false;
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

// #i121236# Support for shift key in writer
#define MOVE_LEFT_HUGE      8
#define MOVE_UP_HUGE        9
#define MOVE_RIGHT_HUGE     10
#define MOVE_DOWN_HUGE      11

void SwEditWin::ChangeFly( sal_uInt8 nDir, bool bWeb )
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    SwRect aTmp = rSh.GetFlyRect();
    if( aTmp.HasArea() &&
        rSh.IsSelObjProtected( FlyProtectFlags::Pos ) == FlyProtectFlags::NONE )
    {
        SfxItemSet aSet(rSh.GetAttrPool(),
                        RES_FRM_SIZE, RES_FRM_SIZE,
                        RES_VERT_ORIENT, RES_ANCHOR,
                        RES_COL, RES_COL,
                        RES_PROTECT, RES_PROTECT,
                        RES_FOLLOW_TEXT_FLOW, RES_FOLLOW_TEXT_FLOW, 0);
        rSh.GetFlyFrameAttr( aSet );
        RndStdIds eAnchorId = static_cast<const SwFormatAnchor&>(aSet.Get(RES_ANCHOR)).GetAnchorId();
        Size aSnap;
        bool bHuge(MOVE_LEFT_HUGE == nDir ||
            MOVE_UP_HUGE == nDir ||
            MOVE_RIGHT_HUGE == nDir ||
            MOVE_DOWN_HUGE == nDir);

        if(MOVE_LEFT_SMALL == nDir ||
            MOVE_UP_SMALL == nDir ||
            MOVE_RIGHT_SMALL == nDir ||
            MOVE_DOWN_SMALL == nDir )
        {
            aSnap = PixelToLogic(Size(1,1));
        }
        else
        {
            aSnap = rSh.GetViewOptions()->GetSnapSize();
            short nDiv = rSh.GetViewOptions()->GetDivisionX();
            if ( nDiv > 0 )
                aSnap.Width() = std::max( (sal_uLong)1, (sal_uLong)aSnap.Width() / nDiv );
            nDiv = rSh.GetViewOptions()->GetDivisionY();
            if ( nDiv > 0 )
                aSnap.Height() = std::max( (sal_uLong)1, (sal_uLong)aSnap.Height() / nDiv );
        }

        if(bHuge)
        {
            // #i121236# 567twips == 1cm, but just take three times the normal snap
            aSnap = Size(aSnap.Width() * 3, aSnap.Height() * 3);
        }

        SwRect aBoundRect;
        Point aRefPoint;
        // adjustment for allowing vertical position
        // aligned to page for fly frame anchored to paragraph or to character.
        {
            SwFormatVertOrient aVert( static_cast<const SwFormatVertOrient&>(aSet.Get(RES_VERT_ORIENT)) );
            const bool bFollowTextFlow =
                    static_cast<const SwFormatFollowTextFlow&>(aSet.Get(RES_FOLLOW_TEXT_FLOW)).GetValue();
            const SwPosition* pToCharContentPos = static_cast<const SwFormatAnchor&>(aSet.Get(RES_ANCHOR)).GetContentAnchor();
            rSh.CalcBoundRect( aBoundRect, eAnchorId,
                               text::RelOrientation::FRAME, aVert.GetRelationOrient(),
                               pToCharContentPos, bFollowTextFlow,
                               false, &aRefPoint );
        }
        long nLeft = std::min( aTmp.Left() - aBoundRect.Left(), aSnap.Width() );
        long nRight = std::min( aBoundRect.Right() - aTmp.Right(), aSnap.Width() );
        long nUp = std::min( aTmp.Top() - aBoundRect.Top(), aSnap.Height() );
        long nDown = std::min( aBoundRect.Bottom() - aTmp.Bottom(), aSnap.Height() );

        switch ( nDir )
        {
            case MOVE_LEFT_BIG:
            case MOVE_LEFT_HUGE:
            case MOVE_LEFT_SMALL: aTmp.Left( aTmp.Left() - nLeft );
                break;

            case MOVE_UP_BIG:
            case MOVE_UP_HUGE:
            case MOVE_UP_SMALL: aTmp.Top( aTmp.Top() - nUp );
                break;

            case MOVE_RIGHT_SMALL:
                if( aTmp.Width() < aSnap.Width() + MINFLY )
                    break;
                nRight = aSnap.Width();
                SAL_FALLTHROUGH;
            case MOVE_RIGHT_HUGE:
            case MOVE_RIGHT_BIG: aTmp.Left( aTmp.Left() + nRight );
                break;

            case MOVE_DOWN_SMALL:
                if( aTmp.Height() < aSnap.Height() + MINFLY )
                    break;
                nDown = aSnap.Height();
                SAL_FALLTHROUGH;
            case MOVE_DOWN_HUGE:
            case MOVE_DOWN_BIG: aTmp.Top( aTmp.Top() + nDown );
                break;

            default: OSL_ENSURE(true, "ChangeFly: Unknown direction." );
        }
        bool bSet = false;
        if ((FLY_AS_CHAR == eAnchorId) && ( nDir % 2 ))
        {
            long aDiff = aTmp.Top() - aRefPoint.Y();
            if( aDiff > 0 )
                aDiff = 0;
            else if ( aDiff < -aTmp.Height() )
                aDiff = -aTmp.Height();
            SwFormatVertOrient aVert( static_cast<const SwFormatVertOrient&>(aSet.Get(RES_VERT_ORIENT)) );
            sal_Int16 eNew;
            if( bWeb )
            {
                eNew = aVert.GetVertOrient();
                bool bDown = 0 != ( nDir & 0x02 );
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
            bSet = true;
        }
        if (bWeb && (FLY_AT_PARA == eAnchorId)
            && ( nDir==MOVE_LEFT_SMALL || nDir==MOVE_RIGHT_BIG ))
        {
            SwFormatHoriOrient aHori( static_cast<const SwFormatHoriOrient&>(aSet.Get(RES_HORI_ORIENT)) );
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
                bSet = true;
            }
        }
        rSh.StartAllAction();
        if( bSet )
            rSh.SetFlyFrameAttr( aSet );
        bool bSetPos = (FLY_AS_CHAR != eAnchorId);
        if(bSetPos && bWeb)
        {
            if (FLY_AT_PAGE != eAnchorId)
            {
                bSetPos = false;
            }
            else
            {
                bSetPos = true;
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
    SwWrtShell &rSh = m_rView.GetWrtShell();
    rSh.StartUndo();

    long nX = 0;
    long nY = 0;
    const bool bOnePixel(
        MOVE_LEFT_SMALL == nDir ||
        MOVE_UP_SMALL == nDir ||
        MOVE_RIGHT_SMALL == nDir ||
        MOVE_DOWN_SMALL == nDir);
    const bool bHuge(
        MOVE_LEFT_HUGE == nDir ||
        MOVE_UP_HUGE == nDir ||
        MOVE_RIGHT_HUGE == nDir ||
        MOVE_DOWN_HUGE == nDir);
    SwMove nAnchorDir = SwMove::UP;
    switch(nDir)
    {
        case MOVE_LEFT_SMALL:
        case MOVE_LEFT_HUGE:
        case MOVE_LEFT_BIG:
            nX = -1;
            nAnchorDir = SwMove::LEFT;
        break;
        case MOVE_UP_SMALL:
        case MOVE_UP_HUGE:
        case MOVE_UP_BIG:
            nY = -1;
        break;
        case MOVE_RIGHT_SMALL:
        case MOVE_RIGHT_HUGE:
        case MOVE_RIGHT_BIG:
            nX = +1;
            nAnchorDir = SwMove::RIGHT;
        break;
        case MOVE_DOWN_SMALL:
        case MOVE_DOWN_HUGE:
        case MOVE_DOWN_BIG:
            nY = +1;
            nAnchorDir = SwMove::DOWN;
        break;
    }

    if(0 != nX || 0 != nY)
    {
        FlyProtectFlags nProtect = rSh.IsSelObjProtected( FlyProtectFlags::Pos|FlyProtectFlags::Size );
        Size aSnap( rSh.GetViewOptions()->GetSnapSize() );
        short nDiv = rSh.GetViewOptions()->GetDivisionX();
        if ( nDiv > 0 )
            aSnap.Width() = std::max( (sal_uLong)1, (sal_uLong)aSnap.Width() / nDiv );
        nDiv = rSh.GetViewOptions()->GetDivisionY();
        if ( nDiv > 0 )
            aSnap.Height() = std::max( (sal_uLong)1, (sal_uLong)aSnap.Height() / nDiv );

        if(bOnePixel)
        {
            aSnap = PixelToLogic(Size(1,1));
        }
        else if(bHuge)
        {
            // #i121236# 567twips == 1cm, but just take three times the normal snap
            aSnap = Size(aSnap.Width() * 3, aSnap.Height() * 3);
        }

        nX *= aSnap.Width();
        nY *= aSnap.Height();

        SdrView *pSdrView = rSh.GetDrawView();
        const SdrHdlList& rHdlList = pSdrView->GetHdlList();
        SdrHdl* pHdl = rHdlList.GetFocusHdl();
        rSh.StartAllAction();
        if(nullptr == pHdl)
        {
            // now move the selected draw objects
            // if the object's position is not protected
            if(!(nProtect&FlyProtectFlags::Pos))
            {
                // Check if object is anchored as character and move direction
                bool bDummy1, bDummy2;
                const bool bVertAnchor = rSh.IsFrameVertical( true, bDummy1, bDummy2 );
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
                if( SdrHdlKind::Anchor == pHdl->GetKind() ||
                    SdrHdlKind::Anchor_TR == pHdl->GetKind() )
                {
                    // anchor move cannot be allowed when position is protected
                    if(!(nProtect&FlyProtectFlags::Pos))
                        rSh.MoveAnchor( nAnchorDir );
                }
                //now resize if size is protected
                else if(!(nProtect&FlyProtectFlags::Size))
                {
                    // now move the Handle (nX, nY)
                    Point aStartPoint(pHdl->GetPos());
                    Point aEndPoint(pHdl->GetPos() + Point(nX, nY));
                    const SdrDragStat& rDragStat = pSdrView->GetDragStat();

                    // start dragging
                    pSdrView->BegDragObj(aStartPoint, nullptr, pHdl, 0);

                    if(pSdrView->IsDragObj())
                    {
                        bool bWasNoSnap = rDragStat.IsNoSnap();
                        bool bWasSnapEnabled = pSdrView->IsSnapEnabled();

                        // switch snapping off
                        if(!bWasNoSnap)
                            const_cast<SdrDragStat&>(rDragStat).SetNoSnap();
                        if(bWasSnapEnabled)
                            pSdrView->SetSnapEnabled(false);

                        pSdrView->MovAction(aEndPoint);
                        pSdrView->EndDragObj();
                        rSh.SetModified();

                        // restore snap
                        if(!bWasNoSnap)
                            const_cast<SdrDragStat&>(rDragStat).SetNoSnap(bWasNoSnap);
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

/**
 * KeyEvents
 */
void SwEditWin::KeyInput(const KeyEvent &rKEvt)
{
    SwWrtShell &rSh = m_rView.GetWrtShell();

    if (comphelper::LibreOfficeKit::isActive() && m_rView.GetPostItMgr())
    {
        if (vcl::Window* pWindow = m_rView.GetPostItMgr()->GetActiveSidebarWin())
        {
            pWindow->KeyInput(rKEvt);
            return;
        }
    }

    if( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE &&
        m_pApplyTempl && m_pApplyTempl->m_pFormatClipboard )
    {
        m_pApplyTempl->m_pFormatClipboard->Erase();
        SetApplyTemplate(SwApplyTemplate());
        m_rView.GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
    }
    else if ( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE &&
            rSh.IsHeaderFooterEdit( ) )
    {
        bool bHeader = bool(FrameTypeFlags::HEADER & rSh.GetFrameType(nullptr,false));
        if ( bHeader )
            rSh.SttPg();
        else
            rSh.EndPg();
        rSh.ToggleHeaderFooterEdit();
    }

    SfxObjectShell *pObjSh = m_rView.GetViewFrame()->GetObjectShell();
    if ( m_bLockInput || (pObjSh && pObjSh->GetProgress()) )
        // When the progress bar is active or a progress is
        // running on a document, no order is being taken
        return;

    delete m_pShadCursor;
    m_pShadCursor = nullptr;
    m_aKeyInputFlushTimer.Stop();

    bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly() &&
                          rSh.IsCursorReadonly();

    //if the language changes the buffer must be flushed
    LanguageType eNewLanguage = GetInputLanguage();
    if(!bIsDocReadOnly && m_eBufferLanguage != eNewLanguage && !m_aInBuffer.isEmpty())
    {
        FlushInBuffer();
    }
    m_eBufferLanguage = eNewLanguage;

    QuickHelpData aTmpQHD;
    if( m_pQuickHlpData->m_bIsDisplayed )
    {
        aTmpQHD.Move( *m_pQuickHlpData );
        m_pQuickHlpData->Stop( rSh );
    }

    // OS:the DrawView also needs a readonly-Flag as well
    if ( !bIsDocReadOnly && rSh.GetDrawView() && rSh.GetDrawView()->KeyInput( rKEvt, this ) )
    {
        rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll( false );
        rSh.SetModified();
        return; // Event evaluated by SdrView
    }

    if ( m_rView.GetDrawFuncPtr() && m_bInsFrame )
    {
        StopInsFrame();
        rSh.Edit();
    }

    bool bFlushBuffer = false;
    bool bNormalChar = false;
    bool bAppendSpace = m_pQuickHlpData->m_bAppendSpace;
    m_pQuickHlpData->m_bAppendSpace = false;

    if ( getenv("SW_DEBUG") && rKEvt.GetKeyCode().GetCode() == KEY_F12 )
    {
        if( rKEvt.GetKeyCode().IsShift())
        {
            GetView().GetDocShell()->GetDoc()->dumpAsXml();
            return;
        }
        else
        {
            SwRootFrame* pLayout = GetView().GetDocShell()->GetWrtShell()->GetLayout();
            pLayout->dumpAsXml( );
            return;
        }
    }

    KeyEvent aKeyEvent( rKEvt );
    // look for vertical mappings
    if( !bIsDocReadOnly && !rSh.IsSelFrameMode() && !rSh.IsObjSelected() )
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
            const bool bTableCursor = rSh.GetTableCursor();
            const bool bVertTable = rSh.IsTableVertical();
            if( ( bVertText && ( !bTableCursor || bVertTable ) ) ||
                ( bTableCursor && bVertTable ) )
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
                                  vcl::KeyCode( nKey, rKEvt.GetKeyCode().GetModifier() ),
                                  rKEvt.GetRepeat() );
        }
    }

    const vcl::KeyCode& rKeyCode = aKeyEvent.GetKeyCode();
    sal_Unicode aCh = aKeyEvent.GetCharCode();

    // enable switching to notes anchor with Ctrl - Alt - Page Up/Down
    // pressing this inside a note will switch to next/previous note
    if ((rKeyCode.IsMod1() && rKeyCode.IsMod2()) && ((rKeyCode.GetCode() == KEY_PAGEUP) || (rKeyCode.GetCode() == KEY_PAGEDOWN)))
    {
        const bool bNext = rKeyCode.GetCode()==KEY_PAGEDOWN;
        const SwFieldType* pFieldType = rSh.GetFieldType( 0, RES_POSTITFLD );
        rSh.MoveFieldType( pFieldType, bNext );
        return;
    }

    const SwFrameFormat* pFlyFormat = rSh.GetFlyFrameFormat();
    if( pFlyFormat )
    {
        sal_uInt16 nEvent;

        if( 32 <= aCh &&
            0 == (( KEY_MOD1 | KEY_MOD2 ) & rKeyCode.GetModifier() ))
            nEvent = SW_EVENT_FRM_KEYINPUT_ALPHA;
        else
            nEvent = SW_EVENT_FRM_KEYINPUT_NOALPHA;

        const SvxMacro* pMacro = pFlyFormat->GetMacro().GetMacroTable().Get( nEvent );
        if( pMacro )
        {
            SbxArrayRef xArgs = new SbxArray;
            SbxVariableRef xVar = new SbxVariable;
            xVar->PutString( pFlyFormat->GetName() );
            xArgs->Put( xVar.get(), 1 );

            xVar = new SbxVariable;
            if( SW_EVENT_FRM_KEYINPUT_ALPHA == nEvent )
                xVar->PutChar( aCh );
            else
                xVar->PutUShort( rKeyCode.GetModifier() | rKeyCode.GetCode() );
            xArgs->Put( xVar.get(), 2 );

            OUString sRet;
            rSh.ExecMacro( *pMacro, &sRet, xArgs.get() );
            if( !sRet.isEmpty() && sRet.toInt32()!=0 )
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
        SdrHdl* pAnchor = rHdlList.GetHdl(SdrHdlKind::Anchor);
        if ( ! pAnchor )
            pAnchor = rHdlList.GetHdl(SdrHdlKind::Anchor_TR);
        if(pAnchor)
            rHdlList.SetFocusHdl(pAnchor);
        return;
    }

    SvxAutoCorrCfg* pACfg = nullptr;
    SvxAutoCorrect* pACorr = nullptr;

    uno::Reference< frame::XDispatchRecorder > xRecorder =
            m_rView.GetViewFrame()->GetBindings().GetRecorder();
    if ( !xRecorder.is() )
    {
        pACfg = &SvxAutoCorrCfg::Get();
        pACorr = pACfg->GetAutoCorrect();
    }

    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    TableChgWidthHeightType eTableChgMode = nsTableChgWidthHeightType::WH_COL_LEFT;    // initialization just for warning-free code
    sal_uInt16 nTableChgSize = 0;
    bool bStopKeyInputTimer = true;
    OUString sFormulaEntry;

    enum SW_KeyState { KS_CheckKey, KS_InsChar, KS_InsTab,
                       KS_NoNum, KS_NumOff, KS_NumOrNoNum, KS_NumDown, KS_NumUp,
                       KS_NumIndentInc, KS_NumIndentDec,

                       KS_OutlineLvOff,
                       KS_NextCell, KS_PrevCell, KS_OutlineUp, KS_OutlineDown,
                       KS_GlossaryExpand, KS_NextPrevGlossary,
                       KS_AutoFormatByInput,
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
                       KS_TableColCellInsDel,

                       KS_Fly_Change, KS_Draw_Change,
                       KS_SpecialInsert,
                       KS_EnterCharCell,
                       KS_GotoNextFieldMark,
                       KS_GotoPrevFieldMark,
                       KS_End };

    SW_KeyState eKeyState = bIsDocReadOnly ? KS_CheckDocReadOnlyKeys : KS_CheckKey;
    SW_KeyState eNextKeyState = KS_End;
    sal_uInt8 nDir = 0;

    if (m_nKS_NUMDOWN_Count > 0)
        m_nKS_NUMDOWN_Count--;

    if (m_nKS_NUMINDENTINC_Count > 0)
        m_nKS_NUMINDENTINC_Count--;

    while( KS_End != eKeyState )
    {
        SW_KeyState eFlyState = KS_KeyToView;

        switch( eKeyState )
        {
        case KS_CheckKey:
            eKeyState = KS_KeyToView;       // default forward to View

#if OSL_DEBUG_LEVEL > 1
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // for switching cursor behaviour in ReadOnly regions
            if( 0x7210 == rKeyCode.GetFullCode() )
                rSh.SetReadOnlyAvailable( !rSh.IsReadOnlyAvailable() );
            else
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#endif

            if( !rKeyCode.IsMod2() && '=' == aCh &&
                !rSh.IsTableMode() && rSh.GetTableFormat() &&
                rSh.IsSttPara() &&
                !rSh.HasReadonlySel() )
            {
                // at the beginning of the table's cell a '=' ->
                // call EditRow (F2-functionality)
                rSh.Push();
                if( !rSh.MoveSection( GoCurrSection, fnSectionStart) &&
                    !rSh.IsTableBoxTextFormat() )
                {
                    // is at the beginning of the box
                    eKeyState = KS_EditFormula;
                    if( rSh.HasMark() )
                        rSh.SwapPam();
                    else
                        rSh.SttSelect();
                    rSh.MoveSection( GoCurrSection, fnSectionEnd );
                    rSh.Pop();
                    rSh.EndSelect();
                    sFormulaEntry = "=";
                }
                else
                    rSh.Pop( false );
            }
            else
            {
                if( pACorr && aTmpQHD.HasContent() && !rSh.HasSelection() &&
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
                    if( rSh.IsTableMode() || !rSh.GetTableFormat() )
                    {
                        if(KS_KeyToView != eFlyState)
                        {
                            if(!pFlyFormat && KS_KeyToView != eFlyState &&
                                (rSh.GetSelectionType() & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM))  &&
                                    rSh.GetDrawView()->AreObjectsMarked())
                                eKeyState = KS_Draw_Change;
                        }

                        if( pFlyFormat )
                            eKeyState = eFlyState;
                        else if( KS_Draw_Change != eKeyState)
                            eKeyState = KS_EnterCharCell;
                    }
                    break;

                // huge object move
                case KEY_RIGHT | KEY_SHIFT:
                case KEY_LEFT | KEY_SHIFT:
                case KEY_UP | KEY_SHIFT:
                case KEY_DOWN | KEY_SHIFT:
                {
                    const int nSelectionType = rSh.GetSelectionType();
                    if ( ( pFlyFormat
                           && ( nSelectionType & (nsSelectionType::SEL_FRM|nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF) ) )
                         || ( ( nSelectionType & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM) )
                              && rSh.GetDrawView()->AreObjectsMarked() ) )
                    {
                        eKeyState = pFlyFormat ? KS_Fly_Change : KS_Draw_Change;
                        switch ( rKeyCode.GetCode() )
                        {
                            case KEY_RIGHT: nDir = MOVE_RIGHT_HUGE; break;
                            case KEY_LEFT: nDir = MOVE_LEFT_HUGE; break;
                            case KEY_UP: nDir = MOVE_UP_HUGE; break;
                            case KEY_DOWN: nDir = MOVE_DOWN_HUGE; break;
                        }
                    }
                    break;
                }

                case KEY_LEFT:
                case KEY_LEFT | KEY_MOD1:
                {
                    bool bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_LEFT_BIG;
                    }
                    eTableChgMode = nsTableChgWidthHeightType::WH_FLAG_INSDEL |
                            ( bMod1
                                ? nsTableChgWidthHeightType::WH_CELL_LEFT
                                : nsTableChgWidthHeightType::WH_COL_LEFT );
                    nTableChgSize = pModOpt->GetTableVInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_RIGHT | KEY_MOD1:
                {
                    eTableChgMode = nsTableChgWidthHeightType::WH_FLAG_INSDEL | nsTableChgWidthHeightType::WH_CELL_RIGHT;
                    nTableChgSize = pModOpt->GetTableVInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_UP:
                case KEY_UP | KEY_MOD1:
                {
                    bool bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_UP_BIG;
                    }
                    eTableChgMode = nsTableChgWidthHeightType::WH_FLAG_INSDEL |
                            ( bMod1
                                ? nsTableChgWidthHeightType::WH_CELL_TOP
                                : nsTableChgWidthHeightType::WH_ROW_TOP );
                    nTableChgSize = pModOpt->GetTableHInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;
                case KEY_DOWN:
                case KEY_DOWN | KEY_MOD1:
                {
                    bool bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
                    if(!bMod1)
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_DOWN_BIG;
                    }
                    eTableChgMode = nsTableChgWidthHeightType::WH_FLAG_INSDEL |
                            ( bMod1
                                ? nsTableChgWidthHeightType::WH_CELL_BOTTOM
                                : nsTableChgWidthHeightType::WH_ROW_BOTTOM );
                    nTableChgSize = pModOpt->GetTableHInsert();
                }
                    goto KEYINPUT_CHECKTABLE_INSDEL;

KEYINPUT_CHECKTABLE_INSDEL:
                    if( rSh.IsTableMode() || !rSh.GetTableFormat() ||
                        !m_bTableInsDelMode ||
                        false /* table protected */
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
                        if( !m_bTableIsInsMode )
                            eTableChgMode = eTableChgMode | nsTableChgWidthHeightType::WH_FLAG_BIGGER;
                        eKeyState = KS_TableColCellInsDel;
                    }
                    break;

                case KEY_DELETE:
                    if ( !rSh.HasReadonlySel() || rSh.CursorInsideInputField())
                    {
                        if (rSh.IsInFrontOfLabel() && rSh.NumOrNoNum())
                            eKeyState = KS_NumOrNoNum;
                    }
                    else
                    {
                        ScopedVclPtrInstance<MessageDialog>(this, "InfoReadonlyDialog",
                            "modules/swriter/ui/inforeadonlydialog.ui")->Execute();
                        eKeyState = KS_End;
                    }
                    break;

                case KEY_DELETE | KEY_MOD2:
                    if( !rSh.IsTableMode() && rSh.GetTableFormat() )
                    {
                        eKeyState = KS_End;
                        m_bTableInsDelMode = true;
                        m_bTableIsInsMode = false;
                        m_aKeyInputTimer.Start();
                        bStopKeyInputTimer = false;
                    }
                    break;
                case KEY_INSERT | KEY_MOD2:
                    if( !rSh.IsTableMode() && rSh.GetTableFormat() )
                    {
                        eKeyState = KS_End;
                        m_bTableInsDelMode = true;
                        m_bTableIsInsMode = true;
                        m_aKeyInputTimer.Start();
                        bStopKeyInputTimer = false;
                    }
                    break;

                case KEY_RETURN:
                {
                    if ( !rSh.HasReadonlySel()
                         && !rSh.CursorInsideInputField() )
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        if(nSelectionType & nsSelectionType::SEL_OLE)
                            eKeyState = KS_LaunchOLEObject;
                        else if(nSelectionType & nsSelectionType::SEL_FRM)
                            eKeyState = KS_GoIntoFly;
                        else if((nSelectionType & nsSelectionType::SEL_DRW) &&
                                0 == (nSelectionType & nsSelectionType::SEL_DRW_TXT) &&
                                rSh.GetDrawView()->GetMarkedObjectList().GetMarkCount() == 1)
                        {
                            eKeyState = KS_GoIntoDrawing;
                            if (lcl_goIntoTextBox(*this, rSh))
                                eKeyState = KS_GoIntoFly;
                        }
                        else if( aTmpQHD.HasContent() && !rSh.HasSelection() &&
                            aTmpQHD.m_bIsAutoText )
                            eKeyState = KS_GlossaryExpand;

                        //RETURN and empty paragraph in numbering -> end numbering
                        else if( m_aInBuffer.isEmpty() &&
                                 rSh.GetNumRuleAtCurrCursorPos() &&
                                 !rSh.GetNumRuleAtCurrCursorPos()->IsOutlineRule() &&
                                 !rSh.HasSelection() &&
                                rSh.IsSttPara() && rSh.IsEndPara() )
                        {
                            eKeyState = KS_NumOff;
                            eNextKeyState = KS_OutlineLvOff;
                        }
                        //RETURN for new paragraph with AutoFormatting
                        else if( pACfg && pACfg->IsAutoFormatByInput() &&
                                !(nSelectionType & (nsSelectionType::SEL_GRF |
                                    nsSelectionType::SEL_OLE | nsSelectionType::SEL_FRM |
                                    nsSelectionType::SEL_TBL_CELLS | nsSelectionType::SEL_DRW |
                                    nsSelectionType::SEL_DRW_TXT)) )
                        {
                            eKeyState = KS_CheckAutoCorrect;
                            eNextKeyState = KS_AutoFormatByInput;
                        }
                        else
                        {
                            eNextKeyState = eKeyState;
                            eKeyState = KS_CheckAutoCorrect;
                        }
                    }
                }
                break;
                case KEY_RETURN | KEY_MOD2:
                {
                    if ( !rSh.HasReadonlySel()
                         && !rSh.IsSttPara()
                         && rSh.GetNumRuleAtCurrCursorPos()
                         && !rSh.CursorInsideInputField() )
                    {
                        eKeyState = KS_NoNum;
                    }
                    else if( rSh.CanSpecialInsert() )
                        eKeyState = KS_SpecialInsert;
                }
                break;
                case KEY_BACKSPACE:
                case KEY_BACKSPACE | KEY_SHIFT:
                    if ( !rSh.HasReadonlySel() || rSh.CursorInsideInputField())
                    {
                        bool bDone = false;
                        // try to add comment for code snip:
                        // Remove the paragraph indent, if the cursor is at the
                        // beginning of a paragraph, there is no selection
                        // and no numbering rule found at the current paragraph
                        // Also try to remove indent, if current paragraph
                        // has numbering rule, but isn't counted and only
                        // key <backspace> is hit.
                        const bool bOnlyBackspaceKey( KEY_BACKSPACE == rKeyCode.GetFullCode() );
                        if ( rSh.IsSttPara()
                             && !rSh.HasSelection()
                             && ( rSh.GetNumRuleAtCurrCursorPos() == nullptr
                                  || ( rSh.IsNoNum() && bOnlyBackspaceKey ) ) )
                        {
                            bDone = rSh.TryRemoveIndent();
                        }

                        if (bDone)
                            eKeyState = KS_End;
                        else
                        {
                            if ( rSh.IsSttPara() && !rSh.IsNoNum() )
                            {
                                if (m_nKS_NUMDOWN_Count > 0 &&
                                    0 < rSh.GetNumLevel())
                                {
                                    eKeyState = KS_NumUp;
                                    m_nKS_NUMDOWN_Count = 2;
                                    bDone = true;
                                }
                                else if (m_nKS_NUMINDENTINC_Count > 0)
                                {
                                    eKeyState = KS_NumIndentDec;
                                    m_nKS_NUMINDENTINC_Count = 2;
                                    bDone = true;
                                }
                            }

                            // If the cursor is in an empty paragraph, which has
                            // a numbering, but not the outline numbering, and
                            // there is no selection, the numbering has to be
                            // deleted on key <Backspace>.
                            // Otherwise method <SwEditShell::NumOrNoNum(..)>
                            // should only change the <IsCounted()> state of
                            // the current paragraph depending of the key.
                            // On <backspace> it is set to <false>,
                            // on <shift-backspace> it is set to <true>.
                            // Thus, assure that method <SwEditShell::NumOrNum(..)>
                            // is only called for the intended purpose.
                            if ( !bDone && rSh.IsSttPara() )
                            {
                                bool bCallNumOrNoNum( false );
                                if ( bOnlyBackspaceKey && !rSh.IsNoNum() )
                                {
                                    bCallNumOrNoNum = true;
                                }
                                else if ( !bOnlyBackspaceKey && rSh.IsNoNum() )
                                {
                                    bCallNumOrNoNum = true;
                                }
                                else if ( bOnlyBackspaceKey
                                          && rSh.IsSttPara()
                                          && rSh.IsEndPara()
                                          && !rSh.HasSelection() )
                                {
                                    const SwNumRule* pCurrNumRule( rSh.GetNumRuleAtCurrCursorPos() );
                                    if ( pCurrNumRule != nullptr
                                         && pCurrNumRule != rSh.GetOutlineNumRule() )
                                    {
                                        bCallNumOrNoNum = true;
                                    }
                                }
                                if ( bCallNumOrNoNum
                                     && rSh.NumOrNoNum( !bOnlyBackspaceKey ) )
                                {
                                    eKeyState = KS_NumOrNoNum;
                                }
                            }
                        }
                    }
                    else
                    {
                        ScopedVclPtrInstance<MessageDialog>(this, "InfoReadonlyDialog",
                            "modules/swriter/ui/inforeadonlydialog.ui")->Execute();
                        eKeyState = KS_End;
                    }
                    break;

                case KEY_RIGHT:
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_RIGHT_BIG;
                        eTableChgMode = nsTableChgWidthHeightType::WH_FLAG_INSDEL | nsTableChgWidthHeightType::WH_COL_RIGHT;
                        nTableChgSize = pModOpt->GetTableVInsert();
                        goto KEYINPUT_CHECKTABLE_INSDEL;
                    }
                case KEY_TAB:
                {

                    if (rSh.IsFormProtected() || rSh.GetCurrentFieldmark() || rSh.GetChar(false)==CH_TXT_ATR_FORMELEMENT)
                    {
                        eKeyState=KS_GotoNextFieldMark;
                    }
                    else if ( !rSh.IsMultiSelection() && rSh.CursorInsideInputField() )
                    {
                        GetView().GetViewFrame()->GetDispatcher()->Execute( FN_GOTO_NEXT_INPUTFLD );
                        eKeyState = KS_End;
                    }
                    else if( rSh.GetNumRuleAtCurrCursorPos()
                             && rSh.IsSttOfPara()
                             && !rSh.HasReadonlySel() )
                    {
                        if ( !rSh.IsMultiSelection()
                             && rSh.IsFirstOfNumRuleAtCursorPos()
                             && numfunc::ChangeIndentOnTabAtFirstPosOfFirstListItem() )
                            eKeyState = KS_NumIndentInc;
                        else
                            eKeyState = KS_NumDown;
                    }
                    else if ( rSh.GetTableFormat() )
                    {
                        if( rSh.HasSelection() || rSh.HasReadonlySel() )
                            eKeyState = KS_NextCell;
                        else
                        {
                            eKeyState = KS_CheckAutoCorrect;
                            eNextKeyState = KS_NextCell;
                        }
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
                            SwTextFormatColl* pColl = rSh.GetCurTextFormatColl();
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
                    if (rSh.IsFormProtected() || rSh.GetCurrentFieldmark()|| rSh.GetChar(false)==CH_TXT_ATR_FORMELEMENT)
                    {
                        eKeyState=KS_GotoPrevFieldMark;
                    }
                    else if ( !rSh.IsMultiSelection() && rSh.CursorInsideInputField() )
                    {
                        GetView().GetViewFrame()->GetDispatcher()->Execute( FN_GOTO_PREV_INPUTFLD );
                        eKeyState = KS_End;
                    }
                    else if( rSh.GetNumRuleAtCurrCursorPos()
                             && rSh.IsSttOfPara()
                             && !rSh.HasReadonlySel() )
                    {
                        if ( !rSh.IsMultiSelection()
                             && rSh.IsFirstOfNumRuleAtCursorPos()
                             && numfunc::ChangeIndentOnTabAtFirstPosOfFirstListItem() )
                            eKeyState = KS_NumIndentDec;
                        else
                            eKeyState = KS_NumUp;
                    }
                    else if ( rSh.GetTableFormat() )
                    {
                        if( rSh.HasSelection() || rSh.HasReadonlySel() )
                            eKeyState = KS_PrevCell;
                        else
                        {
                            eKeyState = KS_CheckAutoCorrect;
                            eNextKeyState = KS_PrevCell;
                        }
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
                            SwTextFormatColl* pColl = rSh.GetCurTextFormatColl();
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
                        if( aTmpQHD.HasContent() && !rSh.HasSelection() )
                        {
                            // Next auto-complete suggestion
                            aTmpQHD.Next( pACorr &&
                                          pACorr->GetSwFlags().bAutoCmpltEndless );
                            eKeyState = KS_NextPrevGlossary;
                        }
                        else if( rSh.GetTableFormat() )
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
                    }
                    break;

                    case KEY_TAB | KEY_MOD1 | KEY_SHIFT:
                    {
                        if( aTmpQHD.HasContent() && !rSh.HasSelection() &&
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
                    }
                    break;
                    case KEY_F2 :
                    if( !rSh.HasReadonlySel() )
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        if(nSelectionType & nsSelectionType::SEL_FRM)
                            eKeyState = KS_GoIntoFly;
                        else if((nSelectionType & nsSelectionType::SEL_DRW))
                        {
                            eKeyState = KS_GoIntoDrawing;
                            if (lcl_goIntoTextBox(*this, rSh))
                                eKeyState = KS_GoIntoFly;
                        }
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
                        bNormalChar = false;
                        eKeyState = KS_End;
                        if ( rSh.GetSelectionType() &
                                (nsSelectionType::SEL_GRF |
                                    nsSelectionType::SEL_FRM |
                                    nsSelectionType::SEL_OLE |
                                    nsSelectionType::SEL_DRW |
                                    nsSelectionType::SEL_DRW_FORM))

                        {
                            eKeyState = (rKeyCode.GetModifier() & KEY_SHIFT) ?
                                                KS_PrevObject : KS_NextObject;
                        }
                        else if ( !rSh.IsMultiSelection() && rSh.CursorInsideInputField() )
                        {
                            GetView().GetViewFrame()->GetDispatcher()->Execute(
                                KEY_SHIFT != rKeyCode.GetModifier() ? FN_GOTO_NEXT_INPUTFLD : FN_GOTO_PREV_INPUTFLD );
                        }
                        else
                        {
                            rSh.SelectNextPrevHyperlink( KEY_SHIFT != rKeyCode.GetModifier() );
                        }
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
                            if(SfxItemState::SET == aSet.GetItemState(RES_TXTATR_INETFMT, false))
                            {
                                const SfxPoolItem& rItem = aSet.Get(RES_TXTATR_INETFMT);
                                bNormalChar = false;
                                eKeyState = KS_End;
                                rSh.ClickToINetAttr(static_cast<const SwFormatINetFormat&>(rItem));
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
                        rSh.Right( CRSR_SKIP_CHARS, false, 1, false );
                        eKeyState = KS_End;
                        FlushInBuffer();
                        break;
                    case KEY_LEFT | KEY_MOD2:
                        rSh.Left( CRSR_SKIP_CHARS, false, 1, false );
                        eKeyState = KS_End;
                        FlushInBuffer();
                        break;
                }
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
                    rSh.NumOrNoNum();
                }

                if( !m_aInBuffer.isEmpty() && ( !bNormalChar || bIsDocReadOnly ))
                    FlushInBuffer();

                if( m_rView.KeyInput( aKeyEvent ) )
                {
                    bFlushBuffer = true;
                    bNormalChar = false;
                }
                else
                {
                    // Because Sfx accelerators are only called when they were
                    // enabled at the last status update, copy has to called
                    // 'forcefully' by us if necessary.
                    if( rKeyCode.GetFunction() == KeyFuncType::COPY )
                        GetView().GetViewFrame()->GetBindings().Execute(SID_COPY);

                    if( !bIsDocReadOnly && bNormalChar )
                    {
                        const int nSelectionType = rSh.GetSelectionType();
                        const bool bDrawObject = (nSelectionType & nsSelectionType::SEL_DRW) &&
                            0 == (nSelectionType & nsSelectionType::SEL_DRW_TXT) &&
                            rSh.GetDrawView()->GetMarkedObjectList().GetMarkCount() == 1;

                        bool bTextBox = false;
                        if (bDrawObject && lcl_goIntoTextBox(*this, rSh))
                            // A draw shape was selected, but it has a TextBox,
                            // start editing that instead when the normal
                            // character is pressed.
                            bTextBox = true;

                        if (bDrawObject && !bTextBox)
                        {
                            SdrObject* pObj = rSh.GetDrawView()->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                            if(pObj)
                            {
                                EnterDrawTextMode(pObj->GetLogicRect().Center());
                                if ( dynamic_cast< const SwDrawTextShell *>(  m_rView.GetCurShell() ) != nullptr  )
                                    static_cast<SwDrawTextShell*>(m_rView.GetCurShell())->Init();
                                rSh.GetDrawView()->KeyInput( rKEvt, this );
                            }
                        }
                        else if (nSelectionType & nsSelectionType::SEL_FRM || bTextBox)
                        {
                            rSh.UnSelectFrame();
                            rSh.LeaveSelFrameMode();
                            m_rView.AttrChangedNotify(&rSh);
                            rSh.MoveSection( GoCurrSection, fnSectionEnd );
                        }
                        eKeyState = KS_InsChar;
                    }
                    else
                    {
                        bNormalChar = false;
                        Window::KeyInput( aKeyEvent );
                    }
                }
            }
            break;
        case KS_LaunchOLEObject:
        {
            rSh.LaunchOLEObj();
            eKeyState = KS_End;
        }
        break;
        case KS_GoIntoFly:
        {
            rSh.UnSelectFrame();
            rSh.LeaveSelFrameMode();
            m_rView.AttrChangedNotify(&rSh);
            rSh.MoveSection( GoCurrSection, fnSectionEnd );
            eKeyState = KS_End;
        }
        break;
        case KS_GoIntoDrawing:
        {
            SdrObject* pObj = rSh.GetDrawView()->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            if(pObj)
            {
                EnterDrawTextMode(pObj->GetLogicRect().Center());
                if (dynamic_cast< const SwDrawTextShell *>(  m_rView.GetCurShell() ) != nullptr  )
                    static_cast<SwDrawTextShell*>(m_rView.GetCurShell())->Init();
            }
            eKeyState = KS_End;
        }
        break;
        case KS_EnterDrawHandleMode:
        {
            const SdrHdlList& rHdlList = rSh.GetDrawView()->GetHdlList();
            bool bForward(!aKeyEvent.GetKeyCode().IsShift());

            const_cast<SdrHdlList&>(rHdlList).TravelFocusHdl(bForward);
            eKeyState = KS_End;
        }
        break;
        case KS_InsTab:
            if( dynamic_cast<const SwWebView*>( &m_rView) !=  nullptr)     // no Tab for WebView
            {
                // then it should be passed along
                Window::KeyInput( aKeyEvent );
                eKeyState = KS_End;
                break;
            }
            aCh = '\t';
            SAL_FALLTHROUGH;
        case KS_InsChar:
            if (rSh.GetChar(false)==CH_TXT_ATR_FORMELEMENT)
            {
                ::sw::mark::ICheckboxFieldmark* pFieldmark =
                    dynamic_cast< ::sw::mark::ICheckboxFieldmark* >
                        (rSh.GetCurrentFieldmark());
                OSL_ENSURE(pFieldmark,
                    "Where is my FieldMark??");
                if(pFieldmark)
                {
                    pFieldmark->SetChecked(!pFieldmark->IsChecked());
                    OSL_ENSURE(pFieldmark->IsExpanded(),
                        "where is the otherpos?");
                    if (pFieldmark->IsExpanded())
                    {
                        rSh.CalcLayout();
                    }
                }
                eKeyState = KS_End;
            }
            else if ( !rSh.HasReadonlySel()
                      || rSh.CursorInsideInputField() )
            {
                const bool bIsNormalChar =
                    GetAppCharClass().isLetterNumeric( OUString( aCh ), 0 );
                if( bAppendSpace && bIsNormalChar &&
                    (!m_aInBuffer.isEmpty() || !rSh.IsSttPara() || !rSh.IsEndPara() ))
                {
                    // insert a blank ahead of the character. this ends up
                    // between the expanded text and the new "non-word-separator".
                    m_aInBuffer += " ";
                }

                const bool bIsAutoCorrectChar =  SvxAutoCorrect::IsAutoCorrectChar( aCh );
                const bool bRunNext = pACorr != nullptr && pACorr->HasRunNext();
                if( !aKeyEvent.GetRepeat() && pACorr && ( bIsAutoCorrectChar || bRunNext ) &&
                        pACfg->IsAutoFormatByInput() &&
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
                        pACfg->IsAutoFormatByInput() &&
                    pACorr->IsAutoCorrFlag( CapitalStartSentence | CapitalStartWord |
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
                    OUStringBuffer aBuf(m_aInBuffer);
                    comphelper::string::padToLength(aBuf,
                        m_aInBuffer.getLength() + aKeyEvent.GetRepeat() + 1, aCh);
                    m_aInBuffer = aBuf.makeStringAndClear();
                    g_bFlushCharBuffer = Application::AnyInput( VclInputFlags::KEYBOARD );
                    bFlushBuffer = !g_bFlushCharBuffer;
                    if( g_bFlushCharBuffer )
                        m_aKeyInputFlushTimer.Start();
                }
                eKeyState = KS_End;
            }
            else
            {
                ScopedVclPtrInstance<MessageDialog>(this, "InfoReadonlyDialog",
                    "modules/swriter/ui/inforeadonlydialog.ui")->Execute();
                eKeyState = KS_End;
            }
        break;

        case KS_CheckAutoCorrect:
        {
            if( pACorr && pACfg->IsAutoFormatByInput() &&
                pACorr->IsAutoCorrFlag( CapitalStartSentence | CapitalStartWord |
                                        ChgOrdinalNumber |
                                        ChgToEnEmDash | SetINetAttr |
                                        Autocorrect ) &&
                !rSh.HasReadonlySel() )
            {
                FlushInBuffer();
                rSh.AutoCorrect( *pACorr, static_cast< sal_Unicode >('\0') );
            }
            eKeyState = eNextKeyState;
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
                break;
            case KS_OutlineLvOff: // delete autofmt outlinelevel later
                break;

            case KS_NumDown:
                rSh.NumUpDown();
                m_nKS_NUMDOWN_Count = 2;
                break;
            case KS_NumUp:
                rSh.NumUpDown( false );
                break;

            case KS_NumIndentInc:
                rSh.ChangeIndentOfAllListLevels(360);
                m_nKS_NUMINDENTINC_Count = 2;
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
                rSh.OutlineUpDown();
                break;
            case KS_OutlineUp:
                rSh.OutlineUpDown( -1 );
                break;

            case KS_NextCell:
                // always 'flush' in tables
                rSh.GoNextCell(!rSh.HasReadonlySel());
                nSlotId = FN_GOTO_NEXT_CELL;
                break;
            case KS_PrevCell:
                rSh.GoPrevCell();
                nSlotId = FN_GOTO_PREV_CELL;
                break;
            case KS_AutoFormatByInput:
                rSh.SplitNode( true );
                break;

            case KS_NextObject:
            case KS_PrevObject:
                if(rSh.GotoObj( KS_NextObject == eKeyState, GotoObjFlags::Any))
                {
                    if( rSh.IsFrameSelected() &&
                        m_rView.GetDrawFuncPtr() )
                    {
                        m_rView.GetDrawFuncPtr()->Deactivate();
                        m_rView.SetDrawFuncPtr(nullptr);
                        m_rView.LeaveDrawCreate();
                        m_rView.AttrChangedNotify( &rSh );
                    }
                    rSh.HideCursor();
                    rSh.EnterSelFrameMode();
                }
            break;
            case KS_GlossaryExpand:
            {
                // replace the word or abbreviation with the auto text
                rSh.StartUndo( UNDO_START );

                OUString sFnd( aTmpQHD.m_aHelpStrings[ aTmpQHD.nCurArrPos ] );
                if( aTmpQHD.m_bIsAutoText )
                {
                    SwGlossaryList* pList = ::GetGlossaryList();
                    OUString sShrtNm;
                    OUString sGroup;
                    if(pList->GetShortName( sFnd, sShrtNm, sGroup))
                    {
                        rSh.SttSelect();
                        rSh.ExtendSelection( false, aTmpQHD.nLen );
                        SwGlossaryHdl* pGlosHdl = GetView().GetGlosHdl();
                        pGlosHdl->SetCurGroup(sGroup, true);
                        pGlosHdl->InsertGlossary( sShrtNm);
                        m_pQuickHlpData->m_bAppendSpace = true;
                    }
                }
                else
                {
                    sFnd = sFnd.copy( aTmpQHD.nLen );
                    rSh.Insert( sFnd );
                    m_pQuickHlpData->m_bAppendSpace = !pACorr ||
                            pACorr->GetSwFlags().bAutoCmpltAppendBlanc;
                }
                rSh.EndUndo( UNDO_END );
            }
            break;

            case KS_NextPrevGlossary:
                m_pQuickHlpData->Move( aTmpQHD );
                m_pQuickHlpData->Start( rSh, USHRT_MAX );
                break;

            case KS_EditFormula:
            {
                const sal_uInt16 nId = SwInputChild::GetChildWindowId();

                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                pVFrame->ToggleChildWindow( nId );
                SwInputChild* pChildWin = static_cast<SwInputChild*>(pVFrame->
                                                    GetChildWindow( nId ));
                if( pChildWin )
                    pChildWin->SetFormula( sFormulaEntry );
            }
            break;

            case KS_ColLeftBig:         rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_COL_LEFT|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableHMove() );   break;
            case KS_ColRightBig:        rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_COL_RIGHT|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableHMove() );  break;
            case KS_ColLeftSmall:       rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_COL_LEFT, pModOpt->GetTableHMove() );   break;
            case KS_ColRightSmall:      rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_COL_RIGHT, pModOpt->GetTableHMove() );  break;
            case KS_ColBottomBig:       rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_ROW_BOTTOM|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableVMove() ); break;
            case KS_ColBottomSmall:     rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_ROW_BOTTOM, pModOpt->GetTableVMove() ); break;
            case KS_CellLeftBig:        rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_CELL_LEFT|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableHMove() );  break;
            case KS_CellRightBig:       rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_CELL_RIGHT|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableHMove() ); break;
            case KS_CellLeftSmall:      rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_CELL_LEFT, pModOpt->GetTableHMove() );  break;
            case KS_CellRightSmall:     rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_CELL_RIGHT, pModOpt->GetTableHMove() ); break;
            case KS_CellTopBig:         rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_CELL_TOP|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableVMove() );   break;
            case KS_CellBottomBig:      rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_CELL_BOTTOM|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableVMove() );    break;
            case KS_CellTopSmall:       rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_CELL_TOP, pModOpt->GetTableVMove() );   break;
            case KS_CellBottomSmall:    rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_CELL_BOTTOM, pModOpt->GetTableVMove() );    break;

            case KS_InsDel_ColLeftBig:          rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_COL_LEFT|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableHInsert() ); break;
            case KS_InsDel_ColRightBig:         rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_COL_RIGHT|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableHInsert() );    break;
            case KS_InsDel_ColLeftSmall:        rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_COL_LEFT, pModOpt->GetTableHInsert() ); break;
            case KS_InsDel_ColRightSmall:       rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_COL_RIGHT, pModOpt->GetTableHInsert() );    break;
            case KS_InsDel_ColTopBig:           rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_ROW_TOP|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableVInsert() );  break;
            case KS_InsDel_ColBottomBig:        rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_ROW_BOTTOM|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableVInsert() );   break;
            case KS_InsDel_ColTopSmall:         rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_ROW_TOP, pModOpt->GetTableVInsert() );  break;
            case KS_InsDel_ColBottomSmall:      rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_ROW_BOTTOM, pModOpt->GetTableVInsert() );   break;
            case KS_InsDel_CellLeftBig:         rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_CELL_LEFT|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableHInsert() );    break;
            case KS_InsDel_CellRightBig:        rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_CELL_RIGHT|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableHInsert() );   break;
            case KS_InsDel_CellLeftSmall:       rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_CELL_LEFT, pModOpt->GetTableHInsert() );    break;
            case KS_InsDel_CellRightSmall:      rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_CELL_RIGHT, pModOpt->GetTableHInsert() );   break;
            case KS_InsDel_CellTopBig:          rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_CELL_TOP|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableVInsert() ); break;
            case KS_InsDel_CellBottomBig:       rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_CELL_BOTTOM|nsTableChgWidthHeightType::WH_FLAG_BIGGER, pModOpt->GetTableVInsert() );  break;
            case KS_InsDel_CellTopSmall:        rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_CELL_TOP, pModOpt->GetTableVInsert() ); break;
            case KS_InsDel_CellBottomSmall:     rSh.SetColRowWidthHeight( nsTableChgWidthHeightType::WH_FLAG_INSDEL|nsTableChgWidthHeightType::WH_CELL_BOTTOM, pModOpt->GetTableVInsert() );  break;

            case KS_TableColCellInsDel:
                rSh.SetColRowWidthHeight( eTableChgMode, nTableChgSize );
                break;
            case KS_Fly_Change:
            {
                SdrView *pSdrView = rSh.GetDrawView();
                const SdrHdlList& rHdlList = pSdrView->GetHdlList();
                if(rHdlList.GetFocusHdl())
                    ChangeDrawing( nDir );
                else
                    ChangeFly( nDir, dynamic_cast<const SwWebView*>( &m_rView) !=  nullptr );
            }
            break;
            case KS_Draw_Change :
                ChangeDrawing( nDir );
                break;
            default:
                break;
            }
            if( nSlotId && m_rView.GetViewFrame()->GetBindings().GetRecorder().is() )
            {
                SfxRequest aReq(m_rView.GetViewFrame(), nSlotId );
                aReq.Done();
            }
            eKeyState = KS_End;
        }
        }
    }

    if( bStopKeyInputTimer )
    {
        m_aKeyInputTimer.Stop();
        m_bTableInsDelMode = false;
    }

    // in case the buffered characters are inserted
    if( bFlushBuffer && !m_aInBuffer.isEmpty() )
    {
        // bFlushCharBuffer was not resetted here
        // why not?
        bool bSave = g_bFlushCharBuffer;
        FlushInBuffer();
        g_bFlushCharBuffer = bSave;

        // maybe show Tip-Help
        OUString sWord;
        if( bNormalChar && pACfg && pACorr &&
            ( pACfg->IsAutoTextTip() ||
              pACorr->GetSwFlags().bAutoCompleteWords ) &&
            rSh.GetPrevAutoCorrWord( *pACorr, sWord ) )
        {
            ShowAutoTextCorrectQuickHelp(sWord, pACfg, pACorr);
        }
    }

    // get the word count dialog to update itself
    SwWordCountWrapper *pWrdCnt = static_cast<SwWordCountWrapper*>(GetView().GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId()));
    if( pWrdCnt )
        pWrdCnt->UpdateCounts();

}

/**
 * MouseEvents
 */
void SwEditWin::RstMBDownFlags()
{
    // Not on all systems a MouseButtonUp is used ahead
    // of the modal dialog (like on WINDOWS).
    // So reset the statuses here and release the mouse
    // for the dialog.
    m_bMBPressed = false;
    g_bNoInterrupt = false;
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

    return rSh.GetContentAtPos(rDocPos, aSwContentAtPos) && pSelectableObj->GetLayer() == rSh.GetDoc()->getIDocumentDrawModelAccess().GetHellId();
}

void SwEditWin::MoveCursor( SwWrtShell &rSh, const Point& rDocPos,
                            const bool bOnlyText, bool bLockView )
{
    const bool bTmpNoInterrupt = g_bNoInterrupt;
    g_bNoInterrupt = false;

    int nTmpSetCursor = 0;

    if( !rSh.IsViewLocked() && bLockView )
        rSh.LockView( true );
    else
        bLockView = false;

    {
        // only temporary generate move context because otherwise
        // the query to the content form doesn't work!!!
        SwMvContext aMvContext( &rSh );
        nTmpSetCursor = rSh.CallSetCursor(&rDocPos, bOnlyText);
        g_bValidCursorPos = !(CRSR_POSCHG & nTmpSetCursor);
    }

    // notify the edit window that from now on we do not use the input language
    if ( !(CRSR_POSOLD & nTmpSetCursor) )
        SetUseInputLanguage( false );

    if( bLockView )
        rSh.LockView( false );

    g_bNoInterrupt = bTmpNoInterrupt;
}

void SwEditWin::MouseButtonDown(const MouseEvent& _rMEvt)
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    const SwField *pCursorField = rSh.CursorInsideInputField() ? rSh.GetCurField( true ) : nullptr;

    // We have to check if a context menu is shown and we have an UI
    // active inplace client. In that case we have to ignore the mouse
    // button down event. Otherwise we would crash (context menu has been
    // opened by inplace client and we would deactivate the inplace client,
    // the contex menu is closed by VCL asynchronously which in the end
    // would work on deleted objects or the context menu has no parent anymore)
    SfxInPlaceClient* pIPClient = rSh.GetSfxViewShell()->GetIPClient();
    bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );

    if ( bIsOleActive && PopupMenu::IsInExecute() )
        return;

    MouseEvent rMEvt(_rMEvt);

    if (m_rView.GetPostItMgr()->IsHit(rMEvt.GetPosPixel()))
        return;

    if (comphelper::LibreOfficeKit::isActive())
    {
        if (vcl::Window* pWindow = m_rView.GetPostItMgr()->IsHitSidebarWindow(rMEvt.GetPosPixel()))
        {
            pWindow->MouseButtonDown(rMEvt);
            return;
        }
    }

    m_rView.GetPostItMgr()->SetActiveSidebarWin(nullptr);

    GrabFocus();

    //ignore key modifiers for format paintbrush
    {
        bool bExecFormatPaintbrush = m_pApplyTempl && m_pApplyTempl->m_pFormatClipboard
                                &&  m_pApplyTempl->m_pFormatClipboard->HasContent();
        if( bExecFormatPaintbrush )
            rMEvt = MouseEvent( _rMEvt.GetPosPixel(), _rMEvt.GetClicks(),
                                    _rMEvt.GetMode(), _rMEvt.GetButtons() );
    }

    m_bWasShdwCursor = nullptr != m_pShadCursor;
    delete m_pShadCursor;
    m_pShadCursor = nullptr;

    const Point aDocPos( PixelToLogic( rMEvt.GetPosPixel() ) );

    FrameControlType eControl;
    bool bOverFly = false;
    bool bPageAnchored = false;
    bool bOverHeaderFooterFly = IsOverHeaderFooterFly( aDocPos, eControl, bOverFly, bPageAnchored );

    bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly();
    if (bOverHeaderFooterFly && (!bIsDocReadOnly && rSh.GetCurField()))
        // We have a field here, that should have priority over header/footer fly.
        bOverHeaderFooterFly = false;

    // Are we clicking on a blank header/footer area?
    if ( IsInHeaderFooter( aDocPos, eControl ) || bOverHeaderFooterFly )
    {
        const SwPageFrame* pPageFrame = rSh.GetLayout()->GetPageAtPos( aDocPos );

        // Is it active?
        bool bActive = true;
        const SwPageDesc* pDesc = pPageFrame->GetPageDesc();

        const SwFrameFormat* pFormat = pDesc->GetLeftFormat();
        if ( pPageFrame->OnRightPage() )
             pFormat = pDesc->GetRightFormat();

        if ( pFormat )
        {
            if ( eControl == Header )
                bActive = pFormat->GetHeader().IsActive();
            else
                bActive = pFormat->GetFooter().IsActive();
        }

        if ( !bActive )
        {
            // When in Hide-Whitespace mode, we don't want header
            // and footer controls.
            if (!rSh.GetViewOptions()->IsHideWhitespaceMode())
            {
                SwPaM aPam(*rSh.GetCurrentShellCursor().GetPoint());
                const bool bWasInHeader = aPam.GetPoint()->nNode.GetNode().FindHeaderStartNode() != nullptr;
                const bool bWasInFooter = aPam.GetPoint()->nNode.GetNode().FindFooterStartNode() != nullptr;

                // Is the cursor in a part like similar to the one we clicked on? For example,
                // if the cursor is in a header and we click on an empty header... don't change anything to
                // keep consistent behaviour due to header edit mode (and the same for the footer as well).

                // Otherwise, we hide the header/footer control if a separator is shown, and vice versa.
                if (!(bWasInHeader && eControl == Header) &&
                    !(bWasInFooter && eControl == Footer))
                {
                    rSh.SetShowHeaderFooterSeparator(eControl, !rSh.IsShowHeaderFooterSeparator(eControl));

                    // Repaint everything
                    Invalidate();
                }
            }
        }
        else
        {
            // Make sure we have the proper Header/Footer separators shown
            // as these may be changed if clicking on an empty Header/Footer
            rSh.SetShowHeaderFooterSeparator( Header, eControl == Header );
            rSh.SetShowHeaderFooterSeparator( Footer, eControl == Footer );

            if ( !rSh.IsHeaderFooterEdit() )
            {
                rSh.ToggleHeaderFooterEdit();

                // Repaint everything
                rSh.GetWin()->Invalidate();
            }
        }
    }
    else
    {
        if ( rSh.IsHeaderFooterEdit( ) )
            rSh.ToggleHeaderFooterEdit( );
        else
        {
            // Make sure that the separators are hidden
            rSh.SetShowHeaderFooterSeparator( Header, false );
            rSh.SetShowHeaderFooterSeparator( Footer, false );

            // Repaint everything
            // FIXME fdo#67358 for unknown reasons this causes painting
            // problems when resizing table columns, so disable it
//            rSh.GetWin()->Invalidate();
        }

        // Toggle Hide-Whitespace if between pages.
        if (rSh.GetViewOptions()->CanHideWhitespace() &&
            rSh.GetLayout()->IsBetweenPages(aDocPos))
        {
            if (_rMEvt.GetClicks() >= 2)
            {
                SwViewOption aOpt(*rSh.GetViewOptions());
                aOpt.SetHideWhitespaceMode(!aOpt.IsHideWhitespaceMode());
                rSh.ApplyViewOptions(aOpt);
            }

            return;
        }
    }

    if ( IsChainMode() )
    {
        SetChainMode( false );
        SwRect aDummy;
        SwFlyFrameFormat *pFormat = static_cast<SwFlyFrameFormat*>(rSh.GetFlyFrameFormat());
        if ( rSh.Chainable( aDummy, *pFormat, aDocPos ) == SwChainRet::OK )
            rSh.Chain( *pFormat, aDocPos );
        UpdatePointer( aDocPos, rMEvt.GetModifier() );
        return;
    }

    // After GrabFocus a shell should be pushed. That should actually
    // work but in practice ...
    m_rView.SelectShellForDrop();

    bool bCallBase = true;

    if( m_pQuickHlpData->m_bIsDisplayed )
        m_pQuickHlpData->Stop( rSh );
    m_pQuickHlpData->m_bAppendSpace = false;

    if( rSh.FinishOLEObj() )
        return; // end InPlace and the click doesn't count anymore

    SET_CURR_SHELL( &rSh );

    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        if (pSdrView->MouseButtonDown( rMEvt, this ) )
        {
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(false);
            return; // SdrView's event evaluated
        }
    }

    m_bIsInMove = false;
    m_aStartPos = rMEvt.GetPosPixel();
    m_aRszMvHdlPt.X() = 0;
    m_aRszMvHdlPt.Y() = 0;

    SwTab nMouseTabCol = SwTab::COL_NONE;
    const bool bTmp = !rSh.IsDrawCreate() && !m_pApplyTempl && !rSh.IsInSelect() &&
         rMEvt.GetClicks() == 1 && MOUSE_LEFT == rMEvt.GetButtons();
    if (  bTmp &&
         SwTab::COL_NONE != (nMouseTabCol = rSh.WhichMouseTabCol( aDocPos ) ) &&
         !rSh.IsObjSelectable( aDocPos ) )
    {
        // Enhanced table selection
        if ( SwTab::SEL_HORI <= nMouseTabCol && SwTab::COLSEL_VERT >= nMouseTabCol )
        {
            rSh.EnterStdMode();
            rSh.SelectTableRowCol( aDocPos );
            if( SwTab::SEL_HORI  != nMouseTabCol && SwTab::SEL_HORI_RTL  != nMouseTabCol)
            {
                m_pRowColumnSelectionStart = new Point( aDocPos );
                m_bIsRowDrag = SwTab::ROWSEL_HORI == nMouseTabCol||
                            SwTab::ROWSEL_HORI_RTL == nMouseTabCol ||
                            SwTab::COLSEL_VERT == nMouseTabCol;
                m_bMBPressed = true;
                CaptureMouse();
            }
            return;
        }

        if ( !rSh.IsTableMode() )
        {
            // comes from table columns out of the document.
            if(SwTab::COL_VERT == nMouseTabCol || SwTab::COL_HORI == nMouseTabCol)
                m_rView.SetTabColFromDoc( true );
            else
                m_rView.SetTabRowFromDoc( true );

            m_rView.SetTabColFromDocPos( aDocPos );
            m_rView.InvalidateRulerPos();
            SfxBindings& rBind = m_rView.GetViewFrame()->GetBindings();
            rBind.Update();
            if ( RulerColumnDrag( rMEvt,
                    (SwTab::COL_VERT == nMouseTabCol || SwTab::ROW_HORI == nMouseTabCol)) )
            {
                m_rView.SetTabColFromDoc( false );
                m_rView.SetTabRowFromDoc( false );
                m_rView.InvalidateRulerPos();
                rBind.Update();
                bCallBase = false;
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
        SwTextNode* pNodeAtPos = rSh.GetNumRuleNodeAtPos( aDocPos );
        m_rView.SetNumRuleNodeFromDoc( pNodeAtPos );
        m_rView.InvalidateRulerPos();
        SfxBindings& rBind = m_rView.GetViewFrame()->GetBindings();
        rBind.Update();

        if ( RulerMarginDrag( rMEvt,
                        SwFEShell::IsVerticalModeAtNdAndPos( *pNodeAtPos, aDocPos ) ) )
        {
            m_rView.SetNumRuleNodeFromDoc( nullptr );
            m_rView.InvalidateRulerPos();
            rBind.Update();
            bCallBase = false;
        }
        else
        {
            // Make sure the pointer is set to 0, otherwise it may point to
            // nowhere after deleting the corresponding text node.
            m_rView.SetNumRuleNodeFromDoc( nullptr );
            return;
        }
    }

    if ( rSh.IsInSelect() )
        rSh.EndSelect();

    // query against LEFT because otherwise for example also a right
    // click releases the selection.
    if ( MOUSE_LEFT == rMEvt.GetButtons() )
    {
        bool bOnlyText = false;
        m_bMBPressed = true;
        g_bNoInterrupt = true;
        m_nKS_NUMDOWN_Count = 0;

        CaptureMouse();

        // reset cursor position if applicable
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
                        !m_pAnchorMarker &&
                        pSdrView &&
                        nullptr != ( pHdl = pSdrView->PickHandle(aDocPos) ) &&
                            ( pHdl->GetKind() == SdrHdlKind::Anchor ||
                              pHdl->GetKind() == SdrHdlKind::Anchor_TR ) )
                    {
                        // #i121463# Set selected during drag
                        pHdl->SetSelected();
                        m_pAnchorMarker = new SwAnchorMarker( pHdl );
                        UpdatePointer( aDocPos, rMEvt.GetModifier() );
                        return;
                    }
                }
                if ( EnterDrawMode( rMEvt, aDocPos ) )
                {
                    g_bNoInterrupt = false;
                    return;
                }
                else  if ( m_rView.GetDrawFuncPtr() && m_bInsFrame )
                {
                    StopInsFrame();
                    rSh.Edit();
                }

                // Without SHIFT because otherwise Toggle doesn't work at selection
                if (rMEvt.GetClicks() == 1)
                {
                    if ( rSh.IsSelFrameMode())
                    {
                        SdrHdl* pHdl = rSh.GetDrawView()->PickHandle(aDocPos);
                        bool bHitHandle = pHdl && pHdl->GetKind() != SdrHdlKind::Anchor &&
                                                  pHdl->GetKind() != SdrHdlKind::Anchor_TR;

                        if ((rSh.IsInsideSelectedObj(aDocPos) || bHitHandle) &&
                            !(rMEvt.GetModifier() == KEY_SHIFT && !bHitHandle))
                        {
                            rSh.EnterSelFrameMode( &aDocPos );
                            if ( !m_pApplyTempl )
                            {
                                // only if no position to size was hit.
                                if (!bHitHandle)
                                {
                                    StartDDTimer();
                                    SwEditWin::m_nDDStartPosY = aDocPos.Y();
                                    SwEditWin::m_nDDStartPosX = aDocPos.X();
                                }
                                g_bFrameDrag = true;
                            }
                            g_bNoInterrupt = false;
                            return;
                        }
                    }
                }
        }

        bool bExecHyperlinks = m_rView.GetDocShell()->IsReadOnly();
        if ( !bExecHyperlinks )
        {
            SvtSecurityOptions aSecOpts;
            const bool bSecureOption = aSecOpts.IsOptionSet( SvtSecurityOptions::EOption::CtrlClickHyperlink );
            if ( (  bSecureOption && rMEvt.GetModifier() == KEY_MOD1 ) ||
                 ( !bSecureOption && rMEvt.GetModifier() != KEY_MOD1 ) )
                bExecHyperlinks = true;
        }

        // Enhanced selection
        sal_uInt8 nNumberOfClicks = static_cast< sal_uInt8 >(rMEvt.GetClicks() % 4);
        if ( 0 == nNumberOfClicks && 0 < rMEvt.GetClicks() )
            nNumberOfClicks = 4;

        bool bExecDrawTextLink = false;

        switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
        {
            case MOUSE_LEFT:
            case MOUSE_LEFT + KEY_MOD1:
            case MOUSE_LEFT + KEY_MOD2:
            {

                // fdo#79604: first, check if a link has been clicked - do not
                // select fly in this case!
                if (1 == nNumberOfClicks)
                {
                    UpdatePointer( aDocPos, rMEvt.GetModifier() );
                    SwEditWin::m_nDDStartPosY = aDocPos.Y();
                    SwEditWin::m_nDDStartPosX = aDocPos.X();

                    // hit an URL in DrawText object?
                    if (bExecHyperlinks && pSdrView)
                    {
                        SdrViewEvent aVEvt;
                        pSdrView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

                        if (aVEvt.eEvent == SdrEventKind::ExecuteUrl)
                            bExecDrawTextLink = true;
                    }
                }

                if (1 == nNumberOfClicks && !bExecDrawTextLink)
                {
                    // only try to select frame, if pointer already was
                    // switched accordingly
                    if ( m_aActHitType != SdrHitKind::NONE && !rSh.IsSelFrameMode() &&
                        !GetView().GetViewFrame()->GetDispatcher()->IsLocked() &&
                        !bExecDrawTextLink)
                    {
                        // Test if there is a draw object at that position and if it should be selected.
                        bool bShould = rSh.ShouldObjectBeSelected(aDocPos);

                        if(bShould)
                        {
                            m_rView.NoRotate();
                            rSh.HideCursor();

                            bool bUnLockView = !rSh.IsViewLocked();
                            rSh.LockView( true );
                            bool bSelObj = rSh.SelectObj( aDocPos,
                                           rMEvt.IsMod1() ? SW_ENTER_GROUP : 0);
                            if( bUnLockView )
                                rSh.LockView( false );

                            if( bSelObj )
                            {
                                // if the frame was deselected in the macro
                                // the cursor just has to be displayed again
                                if( FrameTypeFlags::NONE == rSh.GetSelFrameType() )
                                    rSh.ShowCursor();
                                else
                                {
                                    if (rSh.IsFrameSelected() && m_rView.GetDrawFuncPtr())
                                    {
                                        m_rView.GetDrawFuncPtr()->Deactivate();
                                        m_rView.SetDrawFuncPtr(nullptr);
                                        m_rView.LeaveDrawCreate();
                                        m_rView.AttrChangedNotify( &rSh );
                                    }

                                    rSh.EnterSelFrameMode( &aDocPos );
                                    g_bFrameDrag = true;
                                    UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                }
                                return;
                            }
                            else
                                bOnlyText = rSh.IsObjSelectable( aDocPos );

                            if (!m_rView.GetDrawFuncPtr())
                                rSh.ShowCursor();
                        }
                        else
                            bOnlyText = KEY_MOD1 != rMEvt.GetModifier();
                    }
                    else if ( rSh.IsSelFrameMode() &&
                              (m_aActHitType == SdrHitKind::NONE ||
                               !rSh.IsInsideSelectedObj( aDocPos )))
                    {
                        m_rView.NoRotate();
                        SdrHdl *pHdl;
                        if( !bIsDocReadOnly && !m_pAnchorMarker && nullptr !=
                            ( pHdl = pSdrView->PickHandle(aDocPos) ) &&
                                ( pHdl->GetKind() == SdrHdlKind::Anchor ||
                                  pHdl->GetKind() == SdrHdlKind::Anchor_TR ) )
                        {
                            m_pAnchorMarker = new SwAnchorMarker( pHdl );
                            UpdatePointer( aDocPos, rMEvt.GetModifier() );
                            return;
                        }
                        else
                        {
                            bool bUnLockView = !rSh.IsViewLocked();
                            rSh.LockView( true );
                            sal_uInt8 nFlag = rMEvt.IsShift() ? SW_ADD_SELECT :0;
                            if( rMEvt.IsMod1() )
                                nFlag = nFlag | SW_ENTER_GROUP;

                            if ( rSh.IsSelFrameMode() )
                            {
                                rSh.UnSelectFrame();
                                rSh.LeaveSelFrameMode();
                                m_rView.AttrChangedNotify(&rSh);
                            }

                            bool bSelObj = rSh.SelectObj( aDocPos, nFlag );
                            if( bUnLockView )
                                rSh.LockView( false );

                            if( !bSelObj )
                            {
                                // move cursor here so that it is not drawn in the
                                // frame first; ShowCursor() happens in LeaveSelFrameMode()
                                g_bValidCursorPos = !(CRSR_POSCHG & rSh.CallSetCursor(&aDocPos, false));
                                rSh.LeaveSelFrameMode();
                                m_rView.AttrChangedNotify( &rSh );
                                bCallBase = false;
                            }
                            else
                            {
                                rSh.HideCursor();
                                rSh.EnterSelFrameMode( &aDocPos );
                                rSh.SelFlyGrabCursor();
                                rSh.MakeSelVisible();
                                g_bFrameDrag = true;
                                if( rSh.IsFrameSelected() &&
                                    m_rView.GetDrawFuncPtr() )
                                {
                                    m_rView.GetDrawFuncPtr()->Deactivate();
                                    m_rView.SetDrawFuncPtr(nullptr);
                                    m_rView.LeaveDrawCreate();
                                    m_rView.AttrChangedNotify( &rSh );
                                }
                                UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                return;
                            }
                        }
                    }
                }

                switch ( nNumberOfClicks )
                {
                    case 1:
                        break;
                    case 2:
                    {
                        g_bFrameDrag = false;
                        if ( !bIsDocReadOnly && rSh.IsInsideSelectedObj(aDocPos) &&
                             FlyProtectFlags::NONE == rSh.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent ) )
                        {
                        /* This is no good: on the one hand GetSelectionType is used as flag field
                         * (take a look into the GetSelectionType method) and on the other hand the
                         * return value is used in a switch without proper masking (very nice), this must lead to trouble
                         */
                            switch ( rSh.GetSelectionType() &~ ( nsSelectionType::SEL_FONTWORK | nsSelectionType::SEL_EXTRUDED_CUSTOMSHAPE ) )
                            {
                            case nsSelectionType::SEL_GRF:
                                RstMBDownFlags();
                                if (!comphelper::LibreOfficeKit::isActive())
                                {
                                    GetView().GetViewFrame()->GetBindings().Execute(
                                        FN_FORMAT_GRAFIC_DLG, nullptr,
                                        SfxCallMode::RECORD|SfxCallMode::SLOT);
                                }
                                return;

                            // double click on OLE object --> OLE-InPlace
                            case nsSelectionType::SEL_OLE:
                                if (rSh.IsSelObjProtected(FlyProtectFlags::Content) == FlyProtectFlags::NONE)
                                {
                                    RstMBDownFlags();
                                    rSh.LaunchOLEObj();
                                }
                                return;

                            case nsSelectionType::SEL_FRM:
                                RstMBDownFlags();
                                if (!comphelper::LibreOfficeKit::isActive())
                                {
                                    GetView().GetViewFrame()->GetBindings().Execute(
                                        FN_FORMAT_FRAME_DLG, nullptr,
                                        SfxCallMode::RECORD|SfxCallMode::SLOT);
                                }
                                return;

                            case nsSelectionType::SEL_DRW:
                                RstMBDownFlags();
                                EnterDrawTextMode(aDocPos);
                                if ( dynamic_cast< const SwDrawTextShell *>(  m_rView.GetCurShell() ) != nullptr  )
                                    static_cast<SwDrawTextShell*>(m_rView.GetCurShell())->Init();
                                return;
                            }
                        }

                        // if the cursor position was corrected or if a Fly
                        // was selected in ReadOnlyMode, no word selection, except when tiled rendering.
                        if ((!g_bValidCursorPos || rSh.IsFrameSelected()) && !comphelper::LibreOfficeKit::isActive())
                            return;

                        SwField *pField;
                        bool bFootnote = false;

                        if( !bIsDocReadOnly &&
                            ( nullptr != ( pField = rSh.GetCurField() ) ||
                              ( bFootnote = rSh.GetCurFootnote() )        ) )
                        {
                            RstMBDownFlags();
                            if( bFootnote )
                                GetView().GetViewFrame()->GetBindings().Execute( FN_EDIT_FOOTNOTE );
                            else
                            {
                                sal_uInt16 nTypeId = pField->GetTypeId();
                                SfxViewFrame* pVFrame = GetView().GetViewFrame();
                                switch( nTypeId )
                                {
                                case TYP_POSTITFLD:
                                case TYP_SCRIPTFLD:
                                {
                                    // if it's a Readonly region, status has to be enabled
                                    sal_uInt16 nSlot = TYP_POSTITFLD == nTypeId ? FN_POSTIT : FN_JAVAEDIT;
                                    SfxBoolItem aItem(nSlot, true);
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

                        // select word, AdditionalMode if applicable
                        if ( KEY_MOD1 == rMEvt.GetModifier() && !rSh.IsAddMode() )
                        {
                            rSh.EnterAddMode();
                            rSh.SelWrd( &aDocPos );
                            rSh.LeaveAddMode();
                        }
                        else
                        {
                            if (!rSh.SelWrd(&aDocPos) && comphelper::LibreOfficeKit::isActive())
                                // Double click did not select any word: try to
                                // select the current cell in case we are in a
                                // table.
                                rSh.SelTableBox();
                        }
                        g_bHoldSelection = true;
                        return;
                    }
                    case 3:
                    case 4:
                    {
                        g_bFrameDrag = false;
                        // in extended mode double and triple
                        // click has no effect.
                        if ( rSh.IsExtMode() )
                            return;

                        // if the cursor position was corrected or if a Fly
                        // was selected in ReadOnlyMode, no word selection.
                        if ( !g_bValidCursorPos || rSh.IsFrameSelected() )
                            return;

                        // select line, AdditionalMode if applicable
                        const bool bMod = KEY_MOD1 == rMEvt.GetModifier() &&
                                         !rSh.IsAddMode();

                        if ( bMod )
                            rSh.EnterAddMode();

                        // Enhanced selection
                        if ( 3 == nNumberOfClicks )
                            rSh.SelSentence( &aDocPos );
                        else
                            rSh.SelPara( &aDocPos );

                        if ( bMod )
                            rSh.LeaveAddMode();

                        g_bHoldSelection = true;
                        return;
                    }

                    default:
                        return;
                }

                SAL_FALLTHROUGH;
            }
            case MOUSE_LEFT + KEY_SHIFT:
            case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            {
                bool bLockView = m_bWasShdwCursor;

                switch ( rMEvt.GetModifier() )
                {
                    case KEY_MOD1 + KEY_SHIFT:
                    {
                        if ( !m_bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) )
                        {
                            m_rView.NoRotate();
                            rSh.HideCursor();
                            if ( rSh.IsSelFrameMode() )
                                rSh.SelectObj(aDocPos, SW_ADD_SELECT | SW_ENTER_GROUP);
                            else
                            {   if ( rSh.SelectObj( aDocPos, SW_ADD_SELECT | SW_ENTER_GROUP ) )
                                {
                                    rSh.EnterSelFrameMode( &aDocPos );
                                    SwEditWin::m_nDDStartPosY = aDocPos.Y();
                                    SwEditWin::m_nDDStartPosX = aDocPos.X();
                                    g_bFrameDrag = true;
                                    return;
                                }
                            }
                        }
                        else if( rSh.IsSelFrameMode() &&
                                 rSh.GetDrawView()->PickHandle( aDocPos ))
                        {
                            g_bFrameDrag = true;
                            g_bNoInterrupt = false;
                            return;
                        }
                    }
                    break;
                    case KEY_MOD1:
                    if ( !bExecDrawTextLink )
                    {
                        if ( !m_bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) && !lcl_urlOverBackground( rSh, aDocPos ) )
                        {
                            m_rView.NoRotate();
                            rSh.HideCursor();
                            if ( rSh.IsSelFrameMode() )
                                rSh.SelectObj(aDocPos, SW_ENTER_GROUP);
                            else
                            {   if ( rSh.SelectObj( aDocPos, SW_ENTER_GROUP ) )
                                {
                                    rSh.EnterSelFrameMode( &aDocPos );
                                    SwEditWin::m_nDDStartPosY = aDocPos.Y();
                                    SwEditWin::m_nDDStartPosX = aDocPos.X();
                                    g_bFrameDrag = true;
                                    return;
                                }
                            }
                        }
                        else if( rSh.IsSelFrameMode() &&
                                 rSh.GetDrawView()->PickHandle( aDocPos ))
                        {
                            g_bFrameDrag = true;
                            g_bNoInterrupt = false;
                            return;
                        }
                        else
                        {
                            if ( !rSh.IsAddMode() && !rSh.IsExtMode() && !rSh.IsBlockMode() )
                            {
                                rSh.PushMode();
                                g_bModePushed = true;

                                bool bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( true );
                                rSh.EnterAddMode();
                                if( bUnLockView )
                                    rSh.LockView( false );
                            }
                            bCallBase = false;
                        }
                    }
                    break;
                    case KEY_MOD2:
                    {
                        if ( !rSh.IsAddMode() && !rSh.IsExtMode() && !rSh.IsBlockMode() )
                        {
                            rSh.PushMode();
                            g_bModePushed = true;
                            bool bUnLockView = !rSh.IsViewLocked();
                            rSh.LockView( true );
                            rSh.EnterBlockMode();
                            if( bUnLockView )
                                rSh.LockView( false );
                        }
                        bCallBase = false;
                    }
                    break;
                    case KEY_SHIFT:
                    {
                        if ( !m_bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) )
                        {
                            m_rView.NoRotate();
                            rSh.HideCursor();
                            if ( rSh.IsSelFrameMode() )
                            {
                                rSh.SelectObj(aDocPos, SW_ADD_SELECT);

                                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                                if (rMarkList.GetMark(0) == nullptr)
                                {
                                    rSh.LeaveSelFrameMode();
                                    m_rView.AttrChangedNotify(&rSh);
                                    g_bFrameDrag = false;
                                }
                            }
                            else
                            {   if ( rSh.SelectObj( aDocPos ) )
                                {
                                    rSh.EnterSelFrameMode( &aDocPos );
                                    SwEditWin::m_nDDStartPosY = aDocPos.Y();
                                    SwEditWin::m_nDDStartPosX = aDocPos.X();
                                    g_bFrameDrag = true;
                                    return;
                                }
                            }
                        }
                        else
                        {
                            if ( rSh.IsSelFrameMode() &&
                                 rSh.IsInsideSelectedObj( aDocPos ) )
                            {
                                rSh.EnterSelFrameMode( &aDocPos );
                                SwEditWin::m_nDDStartPosY = aDocPos.Y();
                                SwEditWin::m_nDDStartPosX = aDocPos.X();
                                g_bFrameDrag = true;
                                return;
                            }
                            if ( rSh.IsSelFrameMode() )
                            {
                                rSh.UnSelectFrame();
                                rSh.LeaveSelFrameMode();
                                m_rView.AttrChangedNotify(&rSh);
                                g_bFrameDrag = false;
                            }
                            if ( !rSh.IsExtMode() )
                            {
                                // don't start a selection when an
                                // URL field or a graphic is clicked
                                bool bSttSelect = rSh.HasSelection() ||
                                                Pointer(PointerStyle::RefHand) != GetPointer();

                                if( !bSttSelect )
                                {
                                    bSttSelect = true;
                                    if( bExecHyperlinks )
                                    {
                                        SwContentAtPos aContentAtPos(
                                            SwContentAtPos::SW_FTN |
                                            SwContentAtPos::SW_INETATTR );

                                        if( rSh.GetContentAtPos( aDocPos, aContentAtPos ) )
                                        {
                                            if( !rSh.IsViewLocked() &&
                                                !rSh.IsReadOnlyAvailable() &&
                                                aContentAtPos.IsInProtectSect() )
                                                    bLockView = true;

                                            bSttSelect = false;
                                        }
                                        else if( rSh.IsURLGrfAtPos( aDocPos ))
                                            bSttSelect = false;
                                    }
                                }

                                if( bSttSelect )
                                    rSh.SttSelect();
                            }
                        }
                        bCallBase = false;
                        break;
                    }
                    default:
                        if( !rSh.IsViewLocked() )
                        {
                            SwContentAtPos aContentAtPos( SwContentAtPos::SW_CLICKFIELD |
                                                        SwContentAtPos::SW_INETATTR );
                            if( rSh.GetContentAtPos( aDocPos, aContentAtPos ) &&
                                !rSh.IsReadOnlyAvailable() &&
                                aContentAtPos.IsInProtectSect() )
                                bLockView = true;
                        }
                }

                if ( rSh.IsGCAttr() )
                {
                    rSh.GCAttr();
                    rSh.ClearGCAttr();
                }

                SwContentAtPos aFieldAtPos(SwContentAtPos::SW_FIELD);
                bool bEditableFieldClicked = false;

                // Are we clicking on a field?
                if (rSh.GetContentAtPos(aDocPos, aFieldAtPos))
                {
                    bool bEditableField = (aFieldAtPos.pFndTextAttr != nullptr
                        && aFieldAtPos.pFndTextAttr->Which() == RES_TXTATR_INPUTFIELD);

                    if (!bEditableField)
                    {
                        rSh.CallSetCursor(&aDocPos, bOnlyText);
                        // Unfortunately the cursor may be on field
                        // position or on position after field depending on which
                        // half of the field was clicked on.
                        SwTextAttr const*const pTextField(aFieldAtPos.pFndTextAttr);
                        if (rSh.GetCurrentShellCursor().GetPoint()->nContent
                                .GetIndex() != pTextField->GetStart())
                        {
                            assert(rSh.GetCurrentShellCursor().GetPoint()->nContent
                                    .GetIndex() == (pTextField->GetStart() + 1));
                            rSh.Left( CRSR_SKIP_CHARS, false, 1, false );
                        }
                        // don't go into the !bOverSelect block below - it moves
                        // the cursor
                        break;
                    }
                    else
                    {
                        bEditableFieldClicked = true;
                    }
                }

                bool bOverSelect = rSh.TestCurrPam( aDocPos );
                bool bOverURLGrf = false;
                if( !bOverSelect )
                    bOverURLGrf = bOverSelect = nullptr != rSh.IsURLGrfAtPos( aDocPos );

                if ( !bOverSelect )
                {
                    MoveCursor( rSh, aDocPos, bOnlyText, bLockView );
                    bCallBase = false;
                }
                if (!bOverURLGrf && !bExecDrawTextLink && !bOnlyText)
                {
                    const int nSelType = rSh.GetSelectionType();
                    // Check in general, if an object is selectable at given position.
                    // Thus, also text fly frames in background become selectable via Ctrl-Click.
                    if ( ( nSelType & nsSelectionType::SEL_OLE ||
                         nSelType & nsSelectionType::SEL_GRF ||
                         rSh.IsObjSelectable( aDocPos ) ) && !lcl_urlOverBackground( rSh, aDocPos ) )
                    {
                        SwMvContext aMvContext( &rSh );
                        rSh.EnterSelFrameMode();
                        bCallBase = false;
                    }
                }
                if ( !bOverSelect && bEditableFieldClicked && (!pCursorField ||
                     pCursorField != aFieldAtPos.pFndTextAttr->GetFormatField().GetField()))
                {
                    // select content of Input Field, but exclude CH_TXT_ATR_INPUTFIELDSTART
                    // and CH_TXT_ATR_INPUTFIELDEND
                    rSh.SttSelect();
                    rSh.SelectText( aFieldAtPos.pFndTextAttr->GetStart() + 1,
                                 *(aFieldAtPos.pFndTextAttr->End()) - 1 );
                }
                // don't reset here any longer so that, in case through MouseMove
                // with pressed Ctrl key a multiple-selection should happen,
                // the previous selection is not released in Drag.
                break;
            }
        }
    }
    else if ( MOUSE_RIGHT == rMEvt.GetButtons() && !rMEvt.GetModifier()
        && static_cast< sal_uInt8 >(rMEvt.GetClicks() % 4) == 1
        && !rSh.TestCurrPam( aDocPos ) )
    {
        SwContentAtPos aFieldAtPos(SwContentAtPos::SW_FIELD);

        // Are we clicking on a field?
        if (g_bValidCursorPos
            && rSh.GetContentAtPos(aDocPos, aFieldAtPos)
            && aFieldAtPos.pFndTextAttr != nullptr
            && aFieldAtPos.pFndTextAttr->Which() == RES_TXTATR_INPUTFIELD
            && (!pCursorField || pCursorField != aFieldAtPos.pFndTextAttr->GetFormatField().GetField()))
        {
            // Move the cursor
            MoveCursor( rSh, aDocPos, rSh.IsObjSelectable( aDocPos ), m_bWasShdwCursor );
            bCallBase = false;

            // select content of Input Field, but exclude CH_TXT_ATR_INPUTFIELDSTART
            // and CH_TXT_ATR_INPUTFIELDEND
            rSh.SttSelect();
            rSh.SelectText( aFieldAtPos.pFndTextAttr->GetStart() + 1,
                         *(aFieldAtPos.pFndTextAttr->End()) - 1 );
        }
    }

    if (bCallBase)
        Window::MouseButtonDown(rMEvt);
}

void SwEditWin::MouseMove(const MouseEvent& _rMEvt)
{
    MouseEvent rMEvt(_rMEvt);

    if (comphelper::LibreOfficeKit::isActive())
    {
        if (vcl::Window* pWindow = m_rView.GetPostItMgr()->IsHitSidebarWindow(rMEvt.GetPosPixel()))
        {
            pWindow->MouseMove(rMEvt);
            return;
        }
    }

    //ignore key modifiers for format paintbrush
    {
        bool bExecFormatPaintbrush = m_pApplyTempl && m_pApplyTempl->m_pFormatClipboard
                                &&  m_pApplyTempl->m_pFormatClipboard->HasContent();
        if( bExecFormatPaintbrush )
            rMEvt = MouseEvent( _rMEvt.GetPosPixel(), _rMEvt.GetClicks(),
                                    _rMEvt.GetMode(), _rMEvt.GetButtons() );
    }

    // as long as an action is running the MouseMove should be disconnected
    // otherwise bug 40102 occurs
    SwWrtShell &rSh = m_rView.GetWrtShell();
    if( rSh.ActionPend() )
        return ;

    if( m_pShadCursor && 0 != (rMEvt.GetModifier() + rMEvt.GetButtons() ) )
    {
        delete m_pShadCursor;
        m_pShadCursor = nullptr;
    }

    bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly();

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

    const SwCallMouseEvent aLastCallEvent( m_aSaveCallEvent );
    m_aSaveCallEvent.Clear();

    if ( !bIsDocReadOnly && pSdrView && pSdrView->MouseMove(rMEvt,this) )
    {
        SetPointer( PointerStyle::Text );
        return; // evaluate SdrView's event
    }

    const Point aOldPt( rSh.VisArea().Pos() );
    const bool bInsWin = rSh.VisArea().IsInside( aDocPt ) || comphelper::LibreOfficeKit::isActive();

    if( m_pShadCursor && !bInsWin )
    {
        delete m_pShadCursor;
        m_pShadCursor = nullptr;
    }

    if( bInsWin && m_pRowColumnSelectionStart )
    {
        EnterArea();
        Point aPos( aDocPt );
        if( rSh.SelectTableRowCol( *m_pRowColumnSelectionStart, &aPos, m_bIsRowDrag ))
            return;
    }

    // position is necessary for OS/2 because obviously after a MB-Down
    // a MB-Move is called immediately.
    if( g_bDDTimerStarted )
    {
        Point aDD( SwEditWin::m_nDDStartPosX, SwEditWin::m_nDDStartPosY );
        aDD = LogicToPixel( aDD );
        Rectangle aRect( aDD.X()-3, aDD.Y()-3, aDD.X()+3, aDD.Y()+3 );
        if ( !aRect.IsInside( aPixPt ) )
            StopDDTimer( &rSh, aDocPt );
    }

    if(m_rView.GetDrawFuncPtr())
    {
        if( m_bInsDraw  )
        {
            m_rView.GetDrawFuncPtr()->MouseMove( rMEvt );
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
        else if(!rSh.IsFrameSelected() && !rSh.IsObjSelected())
        {
            SfxBindings &rBnd = rSh.GetView().GetViewFrame()->GetBindings();
            Point aRelPos = rSh.GetRelativePagePosition(aDocPt);
            if(aRelPos.X() >= 0)
            {
                FieldUnit eMetric = ::GetDfltMetric(dynamic_cast<SwWebView*>( &GetView())  != nullptr );
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
                const SfxPointItem aTmp1( SID_ATTR_POSITION, aRelPos );
                rBnd.SetState( aTmp1 );
            }
            else
            {
                rBnd.Invalidate(SID_ATTR_POSITION);
            }
            rBnd.Invalidate(SID_ATTR_SIZE);
            const SfxStringItem aCell( SID_TABLE_CELL, OUString() );
            rBnd.SetState( aCell );
        }
    }

    SwTab nMouseTabCol;
    if( !bIsDocReadOnly && bInsWin && !m_pApplyTempl && !rSh.IsInSelect() )
    {
        if ( SwTab::COL_NONE != (nMouseTabCol = rSh.WhichMouseTabCol( aDocPt ) ) &&
             !rSh.IsObjSelectable( aDocPt ) )
        {
            PointerStyle nPointer = PointerStyle::Null;
            bool bChkTableSel = false;

            switch ( nMouseTabCol )
            {
                case SwTab::COL_VERT :
                case SwTab::ROW_HORI :
                    nPointer = PointerStyle::VSizeBar;
                    bChkTableSel = true;
                    break;
                case SwTab::ROW_VERT :
                case SwTab::COL_HORI :
                    nPointer = PointerStyle::HSizeBar;
                    bChkTableSel = true;
                    break;
                // Enhanced table selection
                case SwTab::SEL_HORI :
                    nPointer = PointerStyle::TabSelectSE;
                    break;
                case SwTab::SEL_HORI_RTL :
                case SwTab::SEL_VERT :
                    nPointer = PointerStyle::TabSelectSW;
                    break;
                case SwTab::COLSEL_HORI :
                case SwTab::ROWSEL_VERT :
                    nPointer = PointerStyle::TabSelectS;
                    break;
                case SwTab::ROWSEL_HORI :
                    nPointer = PointerStyle::TabSelectE;
                    break;
                case SwTab::ROWSEL_HORI_RTL :
                case SwTab::COLSEL_VERT :
                    nPointer = PointerStyle::TabSelectW;
                    break;
                default: break; // prevent compiler warning
            }

            if ( PointerStyle::Null != nPointer &&
                // i#35543 - Enhanced table selection is explicitly allowed in table mode
                ( !bChkTableSel || !rSh.IsTableMode() ) )
            {
                SetPointer( nPointer );
            }

            return;
        }
        else if (rSh.IsNumLabel(aDocPt, RULER_MOUSE_MARGINWIDTH))
        {
            // i#42921 - consider vertical mode
            SwTextNode* pNodeAtPos = rSh.GetNumRuleNodeAtPos( aDocPt );
            const PointerStyle nPointer =
                    SwFEShell::IsVerticalModeAtNdAndPos( *pNodeAtPos, aDocPt )
                    ? PointerStyle::VSizeBar
                    : PointerStyle::HSizeBar;
            SetPointer( nPointer );

            return;
        }
    }

    bool bDelShadCursor = true;

    switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
    {
        case MOUSE_LEFT:
            if( m_pAnchorMarker )
            {
                // Now we need to refresh the SdrHdl pointer of m_pAnchorMarker.
                // This looks a little bit tricky, but it solves the following
                // problem: the m_pAnchorMarker contains a pointer to an SdrHdl,
                // if the FindAnchorPos-call cause a scrolling of the visible
                // area, it's possible that the SdrHdl will be destroyed and a
                // new one will initialized at the original position(GetHdlPos).
                // So the m_pAnchorMarker has to find the right SdrHdl, if it's
                // the old one, it will find it with position aOld, if this one
                // is destroyed, it will find a new one at position GetHdlPos().

                const Point aOld = m_pAnchorMarker->GetPosForHitTest( *(rSh.GetOut()) );
                Point aNew = rSh.FindAnchorPos( aDocPt );
                SdrHdl* pHdl;
                if( pSdrView && (nullptr!=( pHdl = pSdrView->PickHandle( aOld ) )||
                    nullptr !=(pHdl = pSdrView->PickHandle( m_pAnchorMarker->GetHdlPos()) ) ) &&
                        ( pHdl->GetKind() == SdrHdlKind::Anchor ||
                          pHdl->GetKind() == SdrHdlKind::Anchor_TR ) )
                {
                    m_pAnchorMarker->ChgHdl( pHdl );
                    if( aNew.X() || aNew.Y() )
                    {
                        m_pAnchorMarker->SetPos( aNew );
                        m_pAnchorMarker->SetLastPos( aDocPt );
                    }
                }
                else
                {
                    delete m_pAnchorMarker;
                    m_pAnchorMarker = nullptr;
                }
            }
            if ( m_bInsDraw )
            {
                if ( !m_bMBPressed )
                    break;
                if ( m_bIsInMove || IsMinMove( m_aStartPos, aPixPt ) )
                {
                    if ( !bInsWin )
                        LeaveArea( aDocPt );
                    else
                        EnterArea();
                    if ( m_rView.GetDrawFuncPtr() )
                    {
                        pSdrView->SetOrtho(false);
                        m_rView.GetDrawFuncPtr()->MouseMove( rMEvt );
                    }
                    m_bIsInMove = true;
                }
                return;
            }

            {
            SwWordCountWrapper *pWrdCnt = static_cast<SwWordCountWrapper*>(GetView().GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId()));
            if (pWrdCnt)
                pWrdCnt->UpdateCounts();
            }
            SAL_FALLTHROUGH;

        case MOUSE_LEFT + KEY_SHIFT:
        case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            if ( !m_bMBPressed )
                break;
            SAL_FALLTHROUGH;
        case MOUSE_LEFT + KEY_MOD1:
            if ( g_bFrameDrag && rSh.IsSelFrameMode() )
            {
                if( !m_bMBPressed )
                    break;

                if ( m_bIsInMove || IsMinMove( m_aStartPos, aPixPt ) )
                {
                    // event processing for resizing
                    if (pSdrView && pSdrView->AreObjectsMarked())
                    {
                        const Point aSttPt( PixelToLogic( m_aStartPos ) );

                        // can we start?
                        if( SdrHdlKind::User == g_eSdrMoveHdl )
                        {
                            SdrHdl* pHdl = pSdrView->PickHandle( aSttPt );
                            g_eSdrMoveHdl = pHdl ? pHdl->GetKind() : SdrHdlKind::Move;
                        }

                        const SwFrameFormat *const pFlyFormat(rSh.GetFlyFrameFormat());
                        const SvxMacro* pMacro = nullptr;

                        sal_uInt16 nEvent = SdrHdlKind::Move == g_eSdrMoveHdl
                                            ? SW_EVENT_FRM_MOVE
                                            : SW_EVENT_FRM_RESIZE;

                        if (nullptr != pFlyFormat)
                            pMacro = pFlyFormat->GetMacro().GetMacroTable().Get(nEvent);
                        if (nullptr != pMacro &&
                        // or notify only e.g. every 20 Twip?
                            m_aRszMvHdlPt != aDocPt )
                        {
                            m_aRszMvHdlPt = aDocPt;
                            sal_uInt16 nPos = 0;
                            SbxArrayRef xArgs = new SbxArray;
                            SbxVariableRef xVar = new SbxVariable;
                            xVar->PutString( pFlyFormat->GetName() );
                            xArgs->Put( xVar.get(), ++nPos );

                            if( SW_EVENT_FRM_RESIZE == nEvent )
                            {
                                xVar = new SbxVariable;
                                xVar->PutUShort( static_cast< sal_uInt16 >(g_eSdrMoveHdl) );
                                xArgs->Put( xVar.get(), ++nPos );
                            }

                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.X() - aSttPt.X() );
                            xArgs->Put( xVar.get(), ++nPos );
                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.Y() - aSttPt.Y() );
                            xArgs->Put( xVar.get(), ++nPos );

                            OUString sRet;

                            ReleaseMouse();

                            rSh.ExecMacro( *pMacro, &sRet, xArgs.get() );

                            CaptureMouse();

                            if( !sRet.isEmpty() && sRet.toInt32()!=0 )
                                return ;
                        }
                    }
                    // event processing for resizing

                    if( bIsDocReadOnly )
                        break;

                    bool bResizeKeepRatio = rSh.GetSelectionType() & nsSelectionType::SEL_GRF ||
                                            rSh.GetSelectionType() & nsSelectionType::SEL_MEDIA ||
                                            rSh.GetSelectionType() & nsSelectionType::SEL_OLE;
                    bool bisResize = g_eSdrMoveHdl != SdrHdlKind::Move;

                    if (pSdrView)
                    {
                        // Resize proportionally when media is selected and the user drags on a corner
                        const Point aSttPt(PixelToLogic(m_aStartPos));
                        SdrHdl* pHdl = pSdrView->PickHandle(aSttPt);
                        if (pHdl)
                            bResizeKeepRatio = bResizeKeepRatio && pHdl->IsCornerHdl();

                        if (pSdrView->GetDragMode() == SdrDragMode::Crop)
                            bisResize = false;
                        if (rMEvt.IsShift())
                        {
                            pSdrView->SetAngleSnapEnabled(!bResizeKeepRatio);
                            if (bisResize)
                                pSdrView->SetOrtho(!bResizeKeepRatio);
                            else
                                pSdrView->SetOrtho(true);
                        }
                        else
                        {
                            pSdrView->SetAngleSnapEnabled(bResizeKeepRatio);
                            if (bisResize)
                                pSdrView->SetOrtho(bResizeKeepRatio);
                            else
                                pSdrView->SetOrtho(false);
                        }
                    }

                    rSh.Drag( &aDocPt, rMEvt.IsShift() );
                    m_bIsInMove = true;
                }
                else if( bIsDocReadOnly )
                    break;

                if ( !bInsWin )
                {
                    Point aTmp( aDocPt );
                    aTmp += rSh.VisArea().Pos() - aOldPt;
                    LeaveArea( aTmp );
                }
                else if(m_bIsInMove)
                    EnterArea();
                return;
            }
            if ( !rSh.IsSelFrameMode() && !g_bDDINetAttr &&
                (IsMinMove( m_aStartPos,aPixPt ) || m_bIsInMove) &&
                (rSh.IsInSelect() || !rSh.TestCurrPam( aDocPt )) )
            {
                if ( pSdrView )
                {
                    if ( rMEvt.IsShift() )
                        pSdrView->SetOrtho(true);
                    else
                        pSdrView->SetOrtho(false);
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

                        g_bValidCursorPos = !(CRSR_POSCHG & rSh.CallSetCursor(&aDocPt, false));
                        EnterArea();
                    }
                }
            }
            g_bDDINetAttr = false;
            break;
        case 0:
        {
            if ( m_pApplyTempl )
            {
                UpdatePointer(aDocPt); // maybe a frame has to be marked here
                break;
            }
            // change ui if mouse is over SwPostItField
            // TODO: do the same thing for redlines SW_REDLINE
            SwRect aFieldRect;
            SwContentAtPos aContentAtPos( SwContentAtPos::SW_FIELD);
            if( rSh.GetContentAtPos( aDocPt, aContentAtPos, false, &aFieldRect ) )
            {
                const SwField* pField = aContentAtPos.aFnd.pField;
                if (pField->Which()== RES_POSTITFLD)
                {
                    m_rView.GetPostItMgr()->SetShadowState(reinterpret_cast<const SwPostItField*>(pField),false);
                }
                else
                    m_rView.GetPostItMgr()->SetShadowState(nullptr,false);
            }
            else
                m_rView.GetPostItMgr()->SetShadowState(nullptr,false);
            SAL_FALLTHROUGH;
        }
        case KEY_SHIFT:
        case KEY_MOD2:
        case KEY_MOD1:
            if ( !m_bInsDraw )
            {
                bool bTstShdwCursor = true;

                UpdatePointer( aDocPt, rMEvt.GetModifier() );

                const SwFrameFormat* pFormat = nullptr;
                const SwFormatINetFormat* pINet = nullptr;
                SwContentAtPos aContentAtPos( SwContentAtPos::SW_INETATTR );
                if( rSh.GetContentAtPos( aDocPt, aContentAtPos ) )
                    pINet = static_cast<const SwFormatINetFormat*>(aContentAtPos.aFnd.pAttr);

                const void* pTmp = pINet;

                if( pINet ||
                    nullptr != ( pTmp = pFormat = rSh.GetFormatFromAnyObj( aDocPt )))
                {
                    bTstShdwCursor = false;
                    if( pTmp == pINet )
                        m_aSaveCallEvent.Set( pINet );
                    else
                    {
                        IMapObject* pIMapObj = pFormat->GetIMapObject( aDocPt );
                        if( pIMapObj )
                            m_aSaveCallEvent.Set( pFormat, pIMapObj );
                        else
                            m_aSaveCallEvent.Set( EVENT_OBJECT_URLITEM, pFormat );
                    }

                    // should be over a InternetField with an
                    // embedded macro?
                    if( m_aSaveCallEvent != aLastCallEvent )
                    {
                        if( aLastCallEvent.HasEvent() )
                            rSh.CallEvent( SFX_EVENT_MOUSEOUT_OBJECT,
                                            aLastCallEvent, true );
                        // 0 says that the object doesn't have any table
                        if( !rSh.CallEvent( SFX_EVENT_MOUSEOVER_OBJECT,
                                        m_aSaveCallEvent ))
                            m_aSaveCallEvent.Clear();
                    }
                }
                else if( aLastCallEvent.HasEvent() )
                {
                    // cursor was on an object
                    rSh.CallEvent( SFX_EVENT_MOUSEOUT_OBJECT,
                                    aLastCallEvent, true );
                }

                if( bTstShdwCursor && bInsWin && !bIsDocReadOnly &&
                    !m_bInsFrame &&
                    !rSh.GetViewOptions()->getBrowseMode() &&
                    rSh.GetViewOptions()->IsShadowCursor() &&
                    !(rMEvt.GetModifier() + rMEvt.GetButtons()) &&
                    !rSh.HasSelection() && !GetConnectMetaFile() )
                {
                    SwRect aRect;
                    sal_Int16 eOrient;
                    SwFillMode eMode = (SwFillMode)rSh.GetViewOptions()->GetShdwCursorFillMode();
                    if( rSh.GetShadowCursorPos( aDocPt, eMode, aRect, eOrient ))
                    {
                        if( !m_pShadCursor )
                            m_pShadCursor = new SwShadowCursor( *this,
                                SwViewOption::GetDirectCursorColor() );
                        if( text::HoriOrientation::RIGHT != eOrient && text::HoriOrientation::CENTER != eOrient )
                            eOrient = text::HoriOrientation::LEFT;
                        m_pShadCursor->SetPos( aRect.Pos(), aRect.Height(), static_cast< sal_uInt16 >(eOrient) );
                        bDelShadCursor = false;
                    }
                }
            }
            break;
        case MOUSE_LEFT + KEY_MOD2:
            if( rSh.IsBlockMode() && !rMEvt.IsSynthetic() )
            {
                rSh.Drag( &aDocPt, false );
                g_bValidCursorPos = !(CRSR_POSCHG & rSh.CallSetCursor(&aDocPt, false));
                EnterArea();
            }
        break;
    }

    if( bDelShadCursor && m_pShadCursor )
    {
        delete m_pShadCursor;
        m_pShadCursor = nullptr;
    }
    m_bWasShdwCursor = false;
}

/**
 * Button Up
 */
void SwEditWin::MouseButtonUp(const MouseEvent& rMEvt)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        if (vcl::Window* pWindow = m_rView.GetPostItMgr()->IsHitSidebarWindow(rMEvt.GetPosPixel()))
        {
            pWindow->MouseButtonUp(rMEvt);
            return;
        }
    }

    bool bCallBase = true;

    bool bCallShadowCursor = m_bWasShdwCursor;
    m_bWasShdwCursor = false;
    if( m_pShadCursor )
    {
        delete m_pShadCursor;
        m_pShadCursor = nullptr;
    }

    if( m_pRowColumnSelectionStart )
        DELETEZ( m_pRowColumnSelectionStart );

    SdrHdlKind eOldSdrMoveHdl = g_eSdrMoveHdl;
    g_eSdrMoveHdl = SdrHdlKind::User;     // for MoveEvents - reset again

    // preventively reset
    m_rView.SetTabColFromDoc( false );
    m_rView.SetNumRuleNodeFromDoc(nullptr);

    SwWrtShell &rSh = m_rView.GetWrtShell();
    SET_CURR_SHELL( &rSh );
    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        // tdf34555: ortho was always reset before being used in EndSdrDrag
        // Now, it is reset only if not in Crop mode.
        if (pSdrView->GetDragMode() != SdrDragMode::Crop && !rMEvt.IsShift())
            pSdrView->SetOrtho(false);

        if ( pSdrView->MouseButtonUp( rMEvt,this ) )
        {
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(false);
            return; // SdrView's event evaluated
        }
    }
    // only process MouseButtonUp when the Down went to that windows as well.
    if ( !m_bMBPressed )
    {
    // Undo for the watering can is already in CommandHdl
    // that's the way it should be!

        return;
    }

    Point aDocPt( PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( g_bDDTimerStarted )
    {
        StopDDTimer( &rSh, aDocPt );
        m_bMBPressed = false;
        if ( rSh.IsSelFrameMode() )
        {
            rSh.EndDrag( &aDocPt, false );
            g_bFrameDrag = false;
        }
        g_bNoInterrupt = false;
        const Point aDocPos( PixelToLogic( rMEvt.GetPosPixel() ) );
        if ((PixelToLogic(m_aStartPos).Y() == (aDocPos.Y())) && (PixelToLogic(m_aStartPos).X() == (aDocPos.X())))//To make sure it was not moved
        {
            SdrPageView* pPV = nullptr;
            SdrObject* pObj = pSdrView ? pSdrView->PickObj(aDocPos, pSdrView->getHitTolLog(), pPV, SdrSearchOptions::ALSOONMASTER) : nullptr;
            if (pObj)
            {
                SwFrameFormat* pFormat = GetUserCall(pObj)->GetFormat();
                SwFrameFormat* pShapeFormat = SwTextBoxHelper::getOtherTextBoxFormat(pFormat, RES_FLYFRMFMT);
                if (!pShapeFormat)
                {
                    pSdrView->UnmarkAllObj();
                    pSdrView->MarkObj(pObj,pPV);
                }
                else
                {
                    // If the fly frame is a textbox of a shape, then select the shape instead.
                    SdrObject* pShape = pShapeFormat->FindSdrObject();
                    pSdrView->UnmarkAllObj();
                    pSdrView->MarkObj(pShape, pPV);
                }
            }
        }
        ReleaseMouse();
        return;
    }

    if( m_pAnchorMarker )
    {
        if(m_pAnchorMarker->GetHdl())
        {
            // #i121463# delete selected after drag
            m_pAnchorMarker->GetHdl()->SetSelected(false);
        }

        Point aPnt( m_pAnchorMarker->GetLastPos() );
        DELETEZ( m_pAnchorMarker );
        if( aPnt.X() || aPnt.Y() )
            rSh.FindAnchorPos( aPnt, true );
    }
    if ( m_bInsDraw && m_rView.GetDrawFuncPtr() )
    {
        if ( m_rView.GetDrawFuncPtr()->MouseButtonUp( rMEvt ) )
        {
            if (m_rView.GetDrawFuncPtr()) // could have been destroyed in MouseButtonUp
            {
                m_rView.GetDrawFuncPtr()->Deactivate();

                if (!m_rView.IsDrawMode())
                {
                    m_rView.SetDrawFuncPtr(nullptr);
                    SfxBindings& rBind = m_rView.GetViewFrame()->GetBindings();
                    rBind.Invalidate( SID_ATTR_SIZE );
                    rBind.Invalidate( SID_TABLE_CELL );
                }
            }

            if ( rSh.IsObjSelected() )
            {
                rSh.EnterSelFrameMode();
                if (!m_rView.GetDrawFuncPtr())
                    StdDrawMode( OBJ_NONE, true );
            }
            else if ( rSh.IsFrameSelected() )
            {
                rSh.EnterSelFrameMode();
                StopInsFrame();
            }
            else
            {
                const Point aDocPos( PixelToLogic( m_aStartPos ) );
                g_bValidCursorPos = !(CRSR_POSCHG & rSh.CallSetCursor(&aDocPos, false));
                rSh.Edit();
            }

            m_rView.AttrChangedNotify( &rSh );
        }
        else if (rMEvt.GetButtons() == MOUSE_RIGHT && rSh.IsDrawCreate())
            m_rView.GetDrawFuncPtr()->BreakCreate();   // abort drawing

        g_bNoInterrupt = false;
        if (IsMouseCaptured())
            ReleaseMouse();
        return;
    }
    bool bPopMode = false;
    switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
    {
        case MOUSE_LEFT:
            if ( m_bInsDraw && rSh.IsDrawCreate() )
            {
                if ( m_rView.GetDrawFuncPtr() && m_rView.GetDrawFuncPtr()->MouseButtonUp(rMEvt) )
                {
                    m_rView.GetDrawFuncPtr()->Deactivate();
                    m_rView.AttrChangedNotify( &rSh );
                    if ( rSh.IsObjSelected() )
                        rSh.EnterSelFrameMode();
                    if ( m_rView.GetDrawFuncPtr() && m_bInsFrame )
                        StopInsFrame();
                }
                bCallBase = false;
                break;
            }
            SAL_FALLTHROUGH;
        case MOUSE_LEFT + KEY_MOD1:
        case MOUSE_LEFT + KEY_MOD2:
        case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            if ( g_bFrameDrag && rSh.IsSelFrameMode() )
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
                        if ( rSh.GetSelFrameType() & FrameTypeFlags::FLY_ATCNT )
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
                            rSh.Copy(&rSh, aSttPt, aEndPt);
                            rSh.EndUndo( UNDO_UI_DRAG_AND_COPY );
                        }
                    }
                    else {
                        rSh.EndDrag( &aDocPt, false );
                    }
                }
                else
                {
                    {
                        const SwFrameFormat *const pFlyFormat(rSh.GetFlyFrameFormat());
                        const SvxMacro* pMacro = nullptr;

                        sal_uInt16 nEvent = SdrHdlKind::Move == eOldSdrMoveHdl
                                            ? SW_EVENT_FRM_MOVE
                                            : SW_EVENT_FRM_RESIZE;

                        if (nullptr != pFlyFormat)
                            pMacro = pFlyFormat->GetMacro().GetMacroTable().Get(nEvent);
                        if (nullptr != pMacro)
                        {
                            const Point aSttPt( PixelToLogic( m_aStartPos ) );
                            m_aRszMvHdlPt = aDocPt;
                            sal_uInt16 nPos = 0;
                            SbxArrayRef xArgs = new SbxArray;
                            SbxVariableRef xVar = new SbxVariable;
                            xVar->PutString( pFlyFormat->GetName() );
                            xArgs->Put( xVar.get(), ++nPos );

                            if( SW_EVENT_FRM_RESIZE == nEvent )
                            {
                                xVar = new SbxVariable;
                                xVar->PutUShort( static_cast< sal_uInt16 >(eOldSdrMoveHdl) );
                                xArgs->Put( xVar.get(), ++nPos );
                            }

                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.X() - aSttPt.X() );
                            xArgs->Put( xVar.get(), ++nPos );
                            xVar = new SbxVariable;
                            xVar->PutLong( aDocPt.Y() - aSttPt.Y() );
                            xArgs->Put( xVar.get(), ++nPos );

                            xVar = new SbxVariable;
                            xVar->PutUShort( 1 );
                            xArgs->Put( xVar.get(), ++nPos );

                            ReleaseMouse();

                            rSh.ExecMacro( *pMacro, nullptr, xArgs.get() );

                            CaptureMouse();
                        }
                    }
                    rSh.EndDrag( &aDocPt, false );
                }
                g_bFrameDrag = false;
                bCallBase = false;
                break;
            }
            bPopMode = true;
            SAL_FALLTHROUGH;
        case MOUSE_LEFT + KEY_SHIFT:
            if (rSh.IsSelFrameMode())
            {

                rSh.EndDrag( &aDocPt, false );
                g_bFrameDrag = false;
                bCallBase = false;
                break;
            }

            if( g_bHoldSelection )
            {
                // the EndDrag should be called in any case
                g_bHoldSelection = false;
                rSh.EndDrag( &aDocPt, false );
            }
            else
            {
                SwContentAtPos aFieldAtPos ( SwContentAtPos::SW_FIELD );
                if ( !rSh.IsInSelect() && rSh.TestCurrPam( aDocPt ) &&
                     !rSh.GetContentAtPos( aDocPt, aFieldAtPos ) )
                {
                    const bool bTmpNoInterrupt = g_bNoInterrupt;
                    g_bNoInterrupt = false;
                    {   // create only temporary move context because otherwise
                        // the query to the content form doesn't work!!!
                        SwMvContext aMvContext( &rSh );
                        const Point aDocPos( PixelToLogic( m_aStartPos ) );
                        g_bValidCursorPos = !(CRSR_POSCHG & rSh.CallSetCursor(&aDocPos, false));
                    }
                    g_bNoInterrupt = bTmpNoInterrupt;

                }
                else
                {
                    bool bInSel = rSh.IsInSelect();
                    rSh.EndDrag( &aDocPt, false );

                    // Internetfield? --> call link (load doc!!)
                    if( !bInSel )
                    {
                        LoadUrlFlags nFilter = LoadUrlFlags::NONE;
                        if( KEY_MOD1 == rMEvt.GetModifier() )
                            nFilter |= LoadUrlFlags::NewView;

                        bool bExecHyperlinks = m_rView.GetDocShell()->IsReadOnly();
                        if ( !bExecHyperlinks )
                        {
                            SvtSecurityOptions aSecOpts;
                            const bool bSecureOption = aSecOpts.IsOptionSet( SvtSecurityOptions::EOption::CtrlClickHyperlink );
                            if ( (  bSecureOption && rMEvt.GetModifier() == KEY_MOD1 ) ||
                                 ( !bSecureOption && rMEvt.GetModifier() != KEY_MOD1 ) )
                                bExecHyperlinks = true;
                        }

                        const bool bExecSmarttags = rMEvt.GetModifier() == KEY_MOD1;

                        if(m_pApplyTempl)
                            bExecHyperlinks = false;

                        SwContentAtPos aContentAtPos( SwContentAtPos::SW_FIELD |
                                                    SwContentAtPos::SW_INETATTR |
                                                    SwContentAtPos::SW_SMARTTAG  | SwContentAtPos::SW_FORMCTRL);

                        if( rSh.GetContentAtPos( aDocPt, aContentAtPos ) )
                        {
                            // Do it again if we're not on a field/hyperlink to update the cursor accordingly
                            if ( SwContentAtPos::SW_FIELD != aContentAtPos.eContentAtPos
                                 && SwContentAtPos::SW_INETATTR != aContentAtPos.eContentAtPos )
                                rSh.GetContentAtPos( aDocPt, aContentAtPos, true );

                            bool bViewLocked = rSh.IsViewLocked();
                            if( !bViewLocked && !rSh.IsReadOnlyAvailable() &&
                                aContentAtPos.IsInProtectSect() )
                                rSh.LockView( true );

                            ReleaseMouse();

                            if( SwContentAtPos::SW_FIELD == aContentAtPos.eContentAtPos )
                            {
                                bool bAddMode(false);
                                // AdditionalMode if applicable
                                if (KEY_MOD1 == rMEvt.GetModifier()
                                    && !rSh.IsAddMode())
                                {
                                    bAddMode = true;
                                    rSh.EnterAddMode();
                                }
                                if ( aContentAtPos.pFndTextAttr != nullptr
                                     && aContentAtPos.pFndTextAttr->Which() == RES_TXTATR_INPUTFIELD )
                                {
                                    if (!rSh.IsInSelect())
                                    {
                                        // create only temporary move context because otherwise
                                        // the query to the content form doesn't work!!!
                                        SwMvContext aMvContext( &rSh );
                                        const Point aDocPos( PixelToLogic( m_aStartPos ) );
                                        g_bValidCursorPos = !(CRSR_POSCHG & rSh.CallSetCursor(&aDocPos, false));
                                    }
                                    else
                                    {
                                        g_bValidCursorPos = true;
                                    }
                                }
                                else
                                {
                                    rSh.ClickToField( *aContentAtPos.aFnd.pField );
                                    // a bit of a mystery what this is good for?
                                    // in this case we assume it's valid since we
                                    // just selected a field
                                    g_bValidCursorPos = true;
                                }
                                if (bAddMode)
                                {
                                    rSh.LeaveAddMode();
                                }
                            }
                            else if ( SwContentAtPos::SW_SMARTTAG == aContentAtPos.eContentAtPos )
                            {
                                    // execute smarttag menu
                                    if ( bExecSmarttags && SwSmartTagMgr::Get().IsSmartTagsEnabled() )
                                        m_rView.ExecSmartTagPopup( aDocPt );
                            }
                            else if ( SwContentAtPos::SW_FORMCTRL == aContentAtPos.eContentAtPos )
                            {
                                OSL_ENSURE( aContentAtPos.aFnd.pFieldmark != nullptr, "where is my field ptr???");
                                if ( aContentAtPos.aFnd.pFieldmark != nullptr)
                                {
                                    IFieldmark *fieldBM = const_cast< IFieldmark* > ( aContentAtPos.aFnd.pFieldmark );
                                    if ( fieldBM->GetFieldname( ) == ODF_FORMCHECKBOX )
                                    {
                                        ICheckboxFieldmark& rCheckboxFm = dynamic_cast<ICheckboxFieldmark&>(*fieldBM);
                                        rCheckboxFm.SetChecked(!rCheckboxFm.IsChecked());
                                        rCheckboxFm.Invalidate();
                                        rSh.InvalidateWindows( m_rView.GetVisArea() );
                                    } else if ( fieldBM->GetFieldname() == ODF_FORMDROPDOWN ) {
                                        m_rView.ExecFieldPopup( aDocPt, fieldBM );
                                        fieldBM->Invalidate();
                                        rSh.InvalidateWindows( m_rView.GetVisArea() );
                                    } else {
                                        // unknown type..
                                    }
                                }
                            }
                            else if ( SwContentAtPos::SW_INETATTR == aContentAtPos.eContentAtPos )
                            {
                                if ( bExecHyperlinks && aContentAtPos.aFnd.pAttr )
                                    rSh.ClickToINetAttr( *static_cast<const SwFormatINetFormat*>(aContentAtPos.aFnd.pAttr), nFilter );
                            }

                            rSh.LockView( bViewLocked );
                            bCallShadowCursor = false;
                        }
                        else
                        {
                            aContentAtPos = SwContentAtPos( SwContentAtPos::SW_FTN );
                            if( !rSh.GetContentAtPos( aDocPt, aContentAtPos, true ) && bExecHyperlinks )
                            {
                                SdrViewEvent aVEvt;

                                if (pSdrView)
                                    pSdrView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

                                if (pSdrView && aVEvt.eEvent == SdrEventKind::ExecuteUrl)
                                {
                                    // hit URL field
                                    const SvxURLField *pField = aVEvt.pURLField;
                                    if (pField)
                                    {
                                        OUString sURL(pField->GetURL());
                                        OUString sTarget(pField->GetTargetFrame());
                                        ::LoadURL(rSh, sURL, nFilter, sTarget);
                                    }
                                    bCallShadowCursor = false;
                                }
                                else
                                {
                                    // hit graphic
                                    ReleaseMouse();
                                    if( rSh.ClickToINetGrf( aDocPt, nFilter ))
                                        bCallShadowCursor = false;
                                }
                            }
                        }

                        if( bCallShadowCursor &&
                            rSh.GetViewOptions()->IsShadowCursor() &&
                            MOUSE_LEFT == (rMEvt.GetModifier() + rMEvt.GetButtons()) &&
                            !rSh.HasSelection() &&
                            !GetConnectMetaFile() &&
                            rSh.VisArea().IsInside( aDocPt ))
                        {
                            SwUndoId nLastUndoId(UNDO_EMPTY);
                            if (rSh.GetLastUndoInfo(nullptr, & nLastUndoId))
                            {
                                if (UNDO_INS_FROM_SHADOWCRSR == nLastUndoId)
                                {
                                    rSh.Undo();
                                }
                            }
                            SwFillMode eMode = (SwFillMode)rSh.GetViewOptions()->GetShdwCursorFillMode();
                            rSh.SetShadowCursorPos( aDocPt, eMode );
                        }
                    }
                }
                bCallBase = false;

            }

            // reset pushed mode in Down again if applicable
            if ( bPopMode && g_bModePushed )
            {
                rSh.PopMode();
                g_bModePushed = false;
                bCallBase = false;
            }
            break;

        default:
            ReleaseMouse();
            return;
    }

    if( m_pApplyTempl )
    {
        int eSelection = rSh.GetSelectionType();
        SwFormatClipboard* pFormatClipboard = m_pApplyTempl->m_pFormatClipboard;
        if( pFormatClipboard )//apply format paintbrush
        {
            //get some parameters
            SwWrtShell& rWrtShell = m_rView.GetWrtShell();
            SfxStyleSheetBasePool* pPool=nullptr;
            bool bNoCharacterFormats = false;
            bool bNoParagraphFormats = true;
            {
                SwDocShell* pDocSh = m_rView.GetDocShell();
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
        else if( m_pApplyTempl->nColor )
        {
            sal_uInt16 nId = 0;
            switch( m_pApplyTempl->nColor )
            {
                case SID_ATTR_CHAR_COLOR_EXT:
                    nId = RES_CHRATR_COLOR;
                    break;
                case SID_ATTR_CHAR_COLOR_BACKGROUND_EXT:
                    nId = RES_CHRATR_BACKGROUND;
                    break;
            }
            if( nId && (nsSelectionType::SEL_TXT|nsSelectionType::SEL_TBL) & eSelection)
            {
                if( rSh.IsSelection() && !rSh.HasReadonlySel() )
                {
                    m_pApplyTempl->nUndo =
                        std::min(m_pApplyTempl->nUndo, rSh.GetDoc()->GetIDocumentUndoRedo().GetUndoActionCount());
                    if (nId == RES_CHRATR_BACKGROUND)
                        rSh.SetAttrItem( SvxBrushItem( m_aWaterCanTextBackColor, nId ) );
                    else
                        rSh.SetAttrItem( SvxColorItem( m_aWaterCanTextColor, nId ) );
                    rSh.UnSetVisibleCursor();
                    rSh.EnterStdMode();
                    rSh.SetVisibleCursor(aDocPt);
                    bCallBase = false;
                    m_aTemplateIdle.Stop();
                }
                else if(rMEvt.GetClicks() == 1)
                {
                    // no selection -> so turn off watering can
                    m_aTemplateIdle.Start();
                }
            }
        }
        else
        {
            OUString aStyleName;
            switch ( m_pApplyTempl->eType )
            {
                case SfxStyleFamily::Para:
                    if( (( nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL )
                         & eSelection ) && !rSh.HasReadonlySel() )
                    {
                        rSh.SetTextFormatColl( m_pApplyTempl->aColl.pTextColl );
                        m_pApplyTempl->nUndo =
                            std::min(m_pApplyTempl->nUndo, rSh.GetDoc()->GetIDocumentUndoRedo().GetUndoActionCount());
                        bCallBase = false;
                        if ( m_pApplyTempl->aColl.pTextColl )
                            aStyleName = m_pApplyTempl->aColl.pTextColl->GetName();
                    }
                    break;
                case SfxStyleFamily::Char:
                    if( (( nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL )
                         & eSelection ) && !rSh.HasReadonlySel() )
                    {
                        rSh.SetAttrItem( SwFormatCharFormat(m_pApplyTempl->aColl.pCharFormat) );
                        rSh.UnSetVisibleCursor();
                        rSh.EnterStdMode();
                        rSh.SetVisibleCursor(aDocPt);
                        m_pApplyTempl->nUndo =
                            std::min(m_pApplyTempl->nUndo, rSh.GetDoc()->GetIDocumentUndoRedo().GetUndoActionCount());
                        bCallBase = false;
                        if ( m_pApplyTempl->aColl.pCharFormat )
                            aStyleName = m_pApplyTempl->aColl.pCharFormat->GetName();
                    }
                    break;
                case SfxStyleFamily::Frame :
                {
                    const SwFrameFormat* pFormat = rSh.GetFormatFromObj( aDocPt );
                    if(dynamic_cast<const SwFlyFrameFormat*>( pFormat) )
                    {
                        rSh.SetFrameFormat( m_pApplyTempl->aColl.pFrameFormat, false, &aDocPt );
                        m_pApplyTempl->nUndo =
                            std::min(m_pApplyTempl->nUndo, rSh.GetDoc()->GetIDocumentUndoRedo().GetUndoActionCount());
                        bCallBase = false;
                        if( m_pApplyTempl->aColl.pFrameFormat )
                            aStyleName = m_pApplyTempl->aColl.pFrameFormat->GetName();
                    }
                    break;
                }
                case SfxStyleFamily::Page:
                    // no Undo with page templates
                    rSh.ChgCurPageDesc( *m_pApplyTempl->aColl.pPageDesc );
                    if ( m_pApplyTempl->aColl.pPageDesc )
                        aStyleName = m_pApplyTempl->aColl.pPageDesc->GetName();
                    m_pApplyTempl->nUndo =
                        std::min(m_pApplyTempl->nUndo, rSh.GetDoc()->GetIDocumentUndoRedo().GetUndoActionCount());
                    bCallBase = false;
                    break;
                case SfxStyleFamily::Pseudo:
                    if( !rSh.HasReadonlySel() )
                    {
                        rSh.SetCurNumRule( *m_pApplyTempl->aColl.pNumRule,
                                           false,
                                           m_pApplyTempl->aColl.pNumRule->GetDefaultListId() );
                        bCallBase = false;
                        m_pApplyTempl->nUndo =
                            std::min(m_pApplyTempl->nUndo, rSh.GetDoc()->GetIDocumentUndoRedo().GetUndoActionCount());
                        if( m_pApplyTempl->aColl.pNumRule )
                            aStyleName = m_pApplyTempl->aColl.pNumRule->GetName();
                    }
                    break;
                default: break;
            }

            uno::Reference< frame::XDispatchRecorder > xRecorder =
                    m_rView.GetViewFrame()->GetBindings().GetRecorder();
            if ( !aStyleName.isEmpty() && xRecorder.is() )
            {
                SfxShell *pSfxShell = lcl_GetTextShellFromDispatcher( m_rView );
                if ( pSfxShell )
                {
                    SfxRequest aReq( m_rView.GetViewFrame(), SID_STYLE_APPLY );
                    aReq.AppendItem( SfxStringItem( SID_STYLE_APPLY, aStyleName ) );
                    aReq.AppendItem( SfxUInt16Item( SID_STYLE_FAMILY, (sal_uInt16) m_pApplyTempl->eType ) );
                    aReq.Done();
                }
            }
        }

    }
    ReleaseMouse();
    // Only processed MouseEvents arrive here; only at these the moduses can
    // be resetted.
    m_bMBPressed = false;

    // Make this call just to be sure. Selecting has finished surely by now.
    // Otherwise the timeout's timer could give problems.
    EnterArea();
    g_bNoInterrupt = false;

    if (bCallBase)
        Window::MouseButtonUp(rMEvt);

    if (pSdrView && rMEvt.GetClicks() == 1 && comphelper::LibreOfficeKit::isActive())
    {
        // When tiled rendering, single click on a shape text starts editing already.
        SdrViewEvent aViewEvent;
        SdrHitKind eHit = pSdrView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONUP, aViewEvent);
        const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
        if (eHit == SdrHitKind::TextEditObj && rMarkList.GetMarkCount() == 1)
        {
            if (SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj())
            {
                EnterDrawTextMode(pObj->GetLogicRect().Center());
                if ( dynamic_cast< const SwDrawTextShell *>( m_rView.GetCurShell() ) != nullptr )
                    static_cast<SwDrawTextShell*>(m_rView.GetCurShell())->Init();
            }
        }
    }
}

/**
 * Apply template
 */
void SwEditWin::SetApplyTemplate(const SwApplyTemplate &rTempl)
{
    static bool bIdle = false;
    DELETEZ(m_pApplyTempl);
    SwWrtShell &rSh = m_rView.GetWrtShell();

    if(rTempl.m_pFormatClipboard)
    {
        m_pApplyTempl = new SwApplyTemplate( rTempl );
        m_pApplyTempl->nUndo = rSh.GetDoc()->GetIDocumentUndoRedo().GetUndoActionCount();
        SetPointer( PointerStyle::Fill );//@todo #i20119# maybe better a new brush pointer here in future
        rSh.NoEdit( false );
        bIdle = rSh.GetViewOptions()->IsIdle();
        rSh.GetViewOptions()->SetIdle( false );
    }
    else if(rTempl.nColor)
    {
        m_pApplyTempl = new SwApplyTemplate( rTempl );
        m_pApplyTempl->nUndo = rSh.GetDoc()->GetIDocumentUndoRedo().GetUndoActionCount();
        SetPointer( PointerStyle::Fill );
        rSh.NoEdit( false );
        bIdle = rSh.GetViewOptions()->IsIdle();
        rSh.GetViewOptions()->SetIdle( false );
    }
    else if( rTempl.eType != SfxStyleFamily::None )
    {
        m_pApplyTempl = new SwApplyTemplate( rTempl );
        m_pApplyTempl->nUndo = rSh.GetDoc()->GetIDocumentUndoRedo().GetUndoActionCount();
        SetPointer( PointerStyle::Fill  );
        rSh.NoEdit( false );
        bIdle = rSh.GetViewOptions()->IsIdle();
        rSh.GetViewOptions()->SetIdle( false );
    }
    else
    {
        SetPointer( PointerStyle::Text );
        rSh.UnSetVisibleCursor();

        rSh.GetViewOptions()->SetIdle( bIdle );
        if ( !rSh.IsSelFrameMode() )
            rSh.Edit();
    }

    static sal_uInt16 aInva[] =
    {
        SID_STYLE_WATERCAN,
        SID_ATTR_CHAR_COLOR_EXT,
        SID_ATTR_CHAR_COLOR_BACKGROUND_EXT,
        0
    };
    m_rView.GetViewFrame()->GetBindings().Invalidate(aInva);
}

/**
 * Ctor
 */
SwEditWin::SwEditWin(vcl::Window *pParent, SwView &rMyView):
    Window(pParent, WinBits(WB_CLIPCHILDREN | WB_DIALOGCONTROL)),
    DropTargetHelper( this ),
    DragSourceHelper( this ),

    m_eBufferLanguage(LANGUAGE_DONTKNOW),
    m_pApplyTempl(nullptr),
    m_pAnchorMarker( nullptr ),
    m_pUserMarker( nullptr ),
    m_pUserMarkerObj( nullptr ),
    m_pShadCursor( nullptr ),
    m_pRowColumnSelectionStart( nullptr ),

    m_rView( rMyView ),

    m_aActHitType(SdrHitKind::NONE),
    m_nDropFormat( SotClipboardFormatId::NONE ),
    m_nDropAction( 0 ),
    m_nDropDestination( SotExchangeDest::NONE ),

    m_eBezierMode(SID_BEZIER_INSERT),
    m_nInsFrameColCount( 1 ),
    m_eDrawMode(OBJ_NONE),

    m_bMBPressed(false),
    m_bInsDraw(false),
    m_bInsFrame(false),
    m_bIsInMove(false),
    m_bIsInDrag(false),
    m_bOldIdle(false),
    m_bOldIdleSet(false),
    m_bTableInsDelMode(false),
    m_bTableIsInsMode(false),
    m_bChainMode(false),
    m_bWasShdwCursor(false),
    m_bLockInput(false),
    m_bIsRowDrag(false),
    m_bUseInputLanguage(false),
    m_bObjectSelect(false),
    m_nKS_NUMDOWN_Count(0),
    m_nKS_NUMINDENTINC_Count(0),
    m_pFrameControlsManager(new SwFrameControlsManager(this))
{
    set_id("writer_edit");
    SetHelpId(HID_EDIT_WIN);
    EnableChildTransparentMode();
    SetDialogControlFlags( DialogControlFlags::Return | DialogControlFlags::WantFocus );

    m_bMBPressed = m_bInsDraw = m_bInsFrame =
    m_bIsInDrag = m_bOldIdle = m_bOldIdleSet = m_bChainMode = m_bWasShdwCursor = false;
    // initially use the input language
    m_bUseInputLanguage = true;

    SetMapMode(MapMode(MapUnit::MapTwip));

    SetPointer( PointerStyle::Text );
    m_aTimer.SetInvokeHandler(LINK(this, SwEditWin, TimerHandler));

    m_bTableInsDelMode = false;
    m_aKeyInputTimer.SetTimeout( 3000 );
    m_aKeyInputTimer.SetInvokeHandler(LINK(this, SwEditWin, KeyInputTimerHandler));

    m_aKeyInputFlushTimer.SetTimeout( 200 );
    m_aKeyInputFlushTimer.SetInvokeHandler(LINK(this, SwEditWin, KeyInputFlushHandler));

    // TemplatePointer for colors should be resetted without
    // selection after single click
    m_aTemplateIdle.SetPriority(TaskPriority::LOWEST);
    m_aTemplateIdle.SetInvokeHandler(LINK(this, SwEditWin, TemplateTimerHdl));

    // temporary solution!!! Should set the font of the current
    // insert position at every cursor movement!
    if( !rMyView.GetDocShell()->IsReadOnly() )
    {
        vcl::Font aFont;
        SetInputContext( InputContext( aFont, InputContextFlags::Text |
                                            InputContextFlags::ExtText ) );
    }
}

SwEditWin::~SwEditWin()
{
    disposeOnce();
}

void SwEditWin::dispose()
{
    m_aKeyInputTimer.Stop();

    delete m_pShadCursor;
    m_pShadCursor = nullptr;

    delete m_pRowColumnSelectionStart;
    m_pRowColumnSelectionStart = nullptr;

    if( m_pQuickHlpData->m_bIsDisplayed && m_rView.GetWrtShellPtr() )
        m_pQuickHlpData->Stop( m_rView.GetWrtShell() );
    g_bExecuteDrag = false;
    delete m_pApplyTempl;
    m_pApplyTempl = nullptr;

    m_rView.SetDrawFuncPtr(nullptr);

    delete m_pUserMarker;
    m_pUserMarker = nullptr;

    delete m_pAnchorMarker;
    m_pAnchorMarker = nullptr;

    m_pFrameControlsManager->dispose();
    delete m_pFrameControlsManager;
    m_pFrameControlsManager = nullptr;

    DragSourceHelper::dispose();
    DropTargetHelper::dispose();
    vcl::Window::dispose();
}

/**
 * Turn on DrawTextEditMode
 */
void SwEditWin::EnterDrawTextMode( const Point& aDocPos )
{
    if ( m_rView.EnterDrawTextMode(aDocPos) )
    {
        if (m_rView.GetDrawFuncPtr())
        {
            m_rView.GetDrawFuncPtr()->Deactivate();
            m_rView.SetDrawFuncPtr(nullptr);
            m_rView.LeaveDrawCreate();
        }
        m_rView.NoRotate();
        m_rView.AttrChangedNotify( &m_rView.GetWrtShell() );
    }
}

/**
 * Turn on DrawMode
 */
bool SwEditWin::EnterDrawMode(const MouseEvent& rMEvt, const Point& aDocPos)
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    SdrView *pSdrView = rSh.GetDrawView();

    if ( m_rView.GetDrawFuncPtr() )
    {
        if (rSh.IsDrawCreate())
            return true;

        bool bRet = m_rView.GetDrawFuncPtr()->MouseButtonDown( rMEvt );
        m_rView.AttrChangedNotify( &rSh );
        return bRet;
    }

    if ( pSdrView && pSdrView->IsTextEdit() )
    {
        bool bUnLockView = !rSh.IsViewLocked();
        rSh.LockView( true );

        rSh.EndTextEdit(); // clicked aside, end Edit
        rSh.SelectObj( aDocPos );
        if ( !rSh.IsObjSelected() && !rSh.IsFrameSelected() )
            rSh.LeaveSelFrameMode();
        else
        {
            SwEditWin::m_nDDStartPosY = aDocPos.Y();
            SwEditWin::m_nDDStartPosX = aDocPos.X();
            g_bFrameDrag = true;
        }
        if( bUnLockView )
            rSh.LockView( false );
        m_rView.AttrChangedNotify( &rSh );
        return true;
    }
    return false;
}

bool SwEditWin::IsDrawSelMode()
{
    return IsObjectSelect();
}

void SwEditWin::GetFocus()
{
    if ( m_rView.GetPostItMgr()->HasActiveSidebarWin() )
    {
        m_rView.GetPostItMgr()->GrabFocusOnActiveSidebarWin();
    }
    else
    {
        m_rView.GotFocus();
        Window::GetFocus();
        m_rView.GetWrtShell().InvalidateAccessibleFocus();
    }
}

void SwEditWin::LoseFocus()
{
    if (m_rView.GetWrtShellPtr())
        m_rView.GetWrtShell().InvalidateAccessibleFocus();
    Window::LoseFocus();
    if( m_pQuickHlpData && m_pQuickHlpData->m_bIsDisplayed )
        m_pQuickHlpData->Stop( m_rView.GetWrtShell() );
}

void SwEditWin::Command( const CommandEvent& rCEvt )
{
    SwWrtShell &rSh = m_rView.GetWrtShell();

    if ( !m_rView.GetViewFrame() )
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
    bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );
    if ( bIsOleActive && ( rCEvt.GetCommand() == CommandEventId::ContextMenu ))
    {
        rSh.FinishOLEObj();
        return;
    }

    bool bCallBase      = true;

    switch ( rCEvt.GetCommand() )
    {
    case CommandEventId::ContextMenu:
    {
            const sal_uInt16 nId = SwInputChild::GetChildWindowId();
            SwInputChild* pChildWin = static_cast<SwInputChild*>(GetView().GetViewFrame()->
                                                GetChildWindow( nId ));

            if (m_rView.GetPostItMgr()->IsHit(rCEvt.GetMousePosPixel()))
                return;

            Point aDocPos( PixelToLogic( rCEvt.GetMousePosPixel() ) );
            if ( !rCEvt.IsMouseEvent() )
                aDocPos = rSh.GetCharRect().Center();

            // Don't trigger the command on a frame anchored to header/footer is not editing it
            FrameControlType eControl;
            bool bOverFly = false;
            bool bPageAnchored = false;
            bool bOverHeaderFooterFly = IsOverHeaderFooterFly( aDocPos, eControl, bOverFly, bPageAnchored );
            // !bOverHeaderFooterFly doesn't mean we have a frame to select
            if ( !bPageAnchored && rCEvt.IsMouseEvent( ) &&
                 ( ( rSh.IsHeaderFooterEdit( ) && !bOverHeaderFooterFly && bOverFly ) ||
                   ( !rSh.IsHeaderFooterEdit( ) && bOverHeaderFooterFly ) ) )
            {
                return;
            }

            if((!pChildWin || pChildWin->GetView() != &m_rView) &&
                !rSh.IsDrawCreate() && !IsDrawAction())
            {
                SET_CURR_SHELL( &rSh );
                if (!m_pApplyTempl)
                {
                    if (g_bNoInterrupt)
                    {
                        ReleaseMouse();
                        g_bNoInterrupt = false;
                        m_bMBPressed = false;
                    }
                    if ( rCEvt.IsMouseEvent() )
                    {
                        SelectMenuPosition(rSh, rCEvt.GetMousePosPixel());
                        m_rView.StopShellTimer();
                    }
                    const Point aPixPos = LogicToPixel( aDocPos );

                    if ( m_rView.GetDocShell()->IsReadOnly() )
                    {
                        ScopedVclPtrInstance<SwReadOnlyPopup> pROPopup( aDocPos, m_rView );

                        ui::ContextMenuExecuteEvent aEvent;
                        aEvent.SourceWindow = VCLUnoHelper::GetInterface( this );
                        aEvent.ExecutePosition.X = aPixPos.X();
                        aEvent.ExecutePosition.Y = aPixPos.Y();
                        VclPtr<Menu> pMenu;
                        OUString sMenuName("private:resource/ReadonlyContextMenu");
                        if( GetView().TryContextMenuInterception( *pROPopup, sMenuName, pMenu, aEvent ) )
                        {
                            if ( pMenu )
                            {
                                sal_uInt16 nExecId = static_cast<PopupMenu*>(pMenu.get())->Execute(this, aPixPos);
                                if( !::ExecuteMenuCommand( *static_cast<PopupMenu*>(pMenu.get()), *m_rView.GetViewFrame(), nExecId ))
                                    pROPopup->Execute(this, nExecId);
                            }
                            else
                                pROPopup->Execute(this, aPixPos);
                        }
                    }
                    else if ( !m_rView.ExecSpellPopup( aDocPos ) )
                        SfxDispatcher::ExecutePopup(this, &aPixPos);
                }
                else if (m_pApplyTempl->nUndo < rSh.GetDoc()->GetIDocumentUndoRedo().GetUndoActionCount())
                {
                    // Undo until we reach the point when we entered this context.
                    rSh.Do(SwWrtShell::UNDO);
                }
                bCallBase = false;
            }
    }
    break;

    case CommandEventId::Wheel:
    case CommandEventId::StartAutoScroll:
    case CommandEventId::AutoScroll:
            if( m_pShadCursor )
            {
                delete m_pShadCursor;
                m_pShadCursor = nullptr;
            }
            bCallBase = !m_rView.HandleWheelCommands( rCEvt );
            break;

    case CommandEventId::LongPress:
    case CommandEventId::Swipe: //nothing yet
            break;

    case CommandEventId::StartExtTextInput:
    {
        bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCursorReadonly();
        if(!bIsDocReadOnly)
        {
            if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
            {
                bCallBase = false;
                rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
            }
            else
            {
                if( rSh.HasSelection() )
                    rSh.DelRight();

                bCallBase = false;
                LanguageType eInputLanguage = GetInputLanguage();
                rSh.CreateExtTextInput(eInputLanguage);
            }
        }
        break;
    }
    case CommandEventId::EndExtTextInput:
    {
        bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCursorReadonly();
        if(!bIsDocReadOnly)
        {
            if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
            {
                bCallBase = false;
                rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
            }
            else
            {
                bCallBase = false;
                OUString sRecord = rSh.DeleteExtTextInput();
                uno::Reference< frame::XDispatchRecorder > xRecorder =
                        m_rView.GetViewFrame()->GetBindings().GetRecorder();

                if ( !sRecord.isEmpty() )
                {
                    // convert quotes in IME text
                    // works on the last input character, this is escpecially in Korean text often done
                    // quotes that are inside of the string are not replaced!
                    const sal_Unicode aCh = sRecord[sRecord.getLength() - 1];
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
                        SfxShell *pSfxShell = lcl_GetTextShellFromDispatcher( m_rView );
                        // generate request and record
                        if (pSfxShell)
                        {
                            SfxRequest aReq( m_rView.GetViewFrame(), FN_INSERT_STRING );
                            aReq.AppendItem( SfxStringItem( FN_INSERT_STRING, sRecord ) );
                            aReq.Done();
                        }
                    }
                }
            }
        }
    }
    break;
    case CommandEventId::ExtTextInput:
    {
        bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCursorReadonly();
        if(!bIsDocReadOnly)
        {
            if( m_pQuickHlpData->m_bIsDisplayed )
                m_pQuickHlpData->Stop( rSh );

            OUString sWord;
            if( rSh.HasDrawView() && rSh.GetDrawView()->IsTextEdit() )
            {
                bCallBase = false;
                rSh.GetDrawView()->GetTextEditOutlinerView()->Command( rCEvt );
            }
            else
            {
                const CommandExtTextInputData* pData = rCEvt.GetExtTextInputData();
                if( pData )
                {
                    sWord = pData->GetText();
                    bCallBase = false;
                    rSh.SetExtTextInputData( *pData );
                }
            }
                uno::Reference< frame::XDispatchRecorder > xRecorder =
                        m_rView.GetViewFrame()->GetBindings().GetRecorder();
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
    case CommandEventId::CursorPos:
        // will be handled by the base class
        break;

    case CommandEventId::PasteSelection:
        if( !m_rView.GetDocShell()->IsReadOnly() )
        {
            TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSelection( this ));
            if( !aDataHelper.GetXTransferable().is() )
                break;

            SotExchangeDest nDropDestination = GetDropDestination( rCEvt.GetMousePosPixel() );
            if( !bool(nDropDestination) )
                break;
            SotClipboardFormatId nDropFormat;
            sal_uInt8 nEventAction, nDropAction;
            SotExchangeActionFlags nActionFlags;
            nDropAction = SotExchange::GetExchangeAction(
                                aDataHelper.GetDataFlavorExVector(),
                                nDropDestination, EXCHG_IN_ACTION_COPY,
                                EXCHG_IN_ACTION_COPY, nDropFormat,
                                nEventAction,
                                SotClipboardFormatId::NONE, nullptr,
                                &nActionFlags );
            if( EXCHG_INOUT_ACTION_NONE != nDropAction )
            {
                const Point aDocPt( PixelToLogic( rCEvt.GetMousePosPixel() ) );
                SwTransferable::PasteData( aDataHelper, rSh, nDropAction, nActionFlags,
                                    nDropFormat, nDropDestination, false,
                                    false, &aDocPt, EXCHG_IN_ACTION_COPY,
                                    true );
            }
        }
        break;
        case CommandEventId::ModKeyChange :
        {
            const CommandModKeyData* pCommandData = rCEvt.GetModKeyData();
            if (pCommandData->IsMod1() && !pCommandData->IsMod2())
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
        case CommandEventId::InputLanguageChange :
            // i#42732 - update state of fontname if input language changes
            g_bInputLanguageSwitched = true;
            SetUseInputLanguage( true );
        break;
        case CommandEventId::SelectionChange:
        {
            const CommandSelectionChangeData *pData = rCEvt.GetSelectionChangeData();
            rSh.SttCursorMove();
            rSh.GoStartSentence();
            rSh.GetCursor()->GetPoint()->nContent += sal::static_int_cast<sal_uInt16, sal_uLong>(pData->GetStart());
            rSh.SetMark();
            rSh.GetCursor()->GetMark()->nContent += sal::static_int_cast<sal_uInt16, sal_uLong>(pData->GetEnd() - pData->GetStart());
            rSh.EndCursorMove( true );
        }
        break;
        case CommandEventId::PrepareReconversion:
        if( rSh.HasSelection() )
        {
            SwPaM *pCursor = rSh.GetCursor();

            if( rSh.IsMultiSelection() )
            {
                if (pCursor && !pCursor->HasMark() &&
                    pCursor->GetPoint() == pCursor->GetMark())
                {
                    rSh.GoPrevCursor();
                    pCursor = rSh.GetCursor();
                }

                // Cancel all selections other than the last selected one.
                while( rSh.GetCursor()->GetNext() != rSh.GetCursor() )
                    delete rSh.GetCursor()->GetNext();
            }

            if( pCursor )
            {
                sal_uLong nPosNodeIdx = pCursor->GetPoint()->nNode.GetIndex();
                const sal_Int32 nPosIdx = pCursor->GetPoint()->nContent.GetIndex();
                sal_uLong nMarkNodeIdx = pCursor->GetMark()->nNode.GetIndex();
                const sal_Int32 nMarkIdx = pCursor->GetMark()->nContent.GetIndex();

                if( !rSh.GetCursor()->HasMark() )
                    rSh.GetCursor()->SetMark();

                rSh.SttCursorMove();

                if( nPosNodeIdx < nMarkNodeIdx )
                {
                rSh.GetCursor()->GetPoint()->nNode = nPosNodeIdx;
                rSh.GetCursor()->GetPoint()->nContent = nPosIdx;
                rSh.GetCursor()->GetMark()->nNode = nPosNodeIdx;
                rSh.GetCursor()->GetMark()->nContent =
                    rSh.GetCursor()->GetContentNode()->Len();
                }
                else if( nPosNodeIdx == nMarkNodeIdx )
                {
                rSh.GetCursor()->GetPoint()->nNode = nPosNodeIdx;
                rSh.GetCursor()->GetPoint()->nContent = nPosIdx;
                rSh.GetCursor()->GetMark()->nNode = nMarkNodeIdx;
                rSh.GetCursor()->GetMark()->nContent = nMarkIdx;
                }
                else
                {
                rSh.GetCursor()->GetMark()->nNode = nMarkNodeIdx;
                rSh.GetCursor()->GetMark()->nContent = nMarkIdx;
                rSh.GetCursor()->GetPoint()->nNode = nMarkNodeIdx;
                rSh.GetCursor()->GetPoint()->nContent =
                    rSh.GetCursor()->GetContentNode( false )->Len();
                }

                rSh.EndCursorMove( true );
            }
        }
        break;
        case CommandEventId::QueryCharPosition:
        {
            bool bVertical = rSh.IsInVerticalText();
            const SwPosition& rPos = *rSh.GetCursor()->GetPoint();
            SwDocShell* pDocSh = m_rView.GetDocShell();
            SwDoc *pDoc = pDocSh->GetDoc();
            SwExtTextInput* pInput = pDoc->GetExtTextInput( rPos.nNode.GetNode(), rPos.nContent.GetIndex() );
            if ( pInput )
            {
                const SwPosition& rStart = *pInput->Start();
                const SwPosition& rEnd = *pInput->End();
                int nSize = 0;
                for ( SwIndex nIndex = rStart.nContent; nIndex < rEnd.nContent; ++nIndex )
                {
                    ++nSize;
                }
                vcl::Window& rWin = rSh.GetView().GetEditWin();
                if ( nSize == 0 )
                {
                    // When the composition does not exist, use Caret rect instead.
                    SwRect aCaretRect ( rSh.GetCharRect() );
                    Rectangle aRect( aCaretRect.Left(), aCaretRect.Top(), aCaretRect.Right(), aCaretRect.Bottom() );
                    rWin.SetCompositionCharRect( &aRect, 1, bVertical );
                }
                else
                {
                    std::unique_ptr<Rectangle[]> aRects(new Rectangle[ nSize ]);
                    int nRectIndex = 0;
                    for ( SwIndex nIndex = rStart.nContent; nIndex < rEnd.nContent; ++nIndex )
                    {
                        const SwPosition aPos( rStart.nNode, nIndex );
                        SwRect aRect ( rSh.GetCharRect() );
                        rSh.GetCharRectAt( aRect, &aPos );
                        aRects[ nRectIndex ] = Rectangle( aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom() );
                        ++nRectIndex;
                    }
                    rWin.SetCompositionCharRect( aRects.get(), nSize, bVertical );
                }
            }
            bCallBase = false;
        }
        break;
        default:
#if OSL_DEBUG_LEVEL > 0
            OSL_ENSURE( false, "unknown command." );
#endif
        break;
    }
    if (bCallBase)
        Window::Command(rCEvt);
}

/*  i#18686 select the object/cursor at the mouse
    position of the context menu request */
void SwEditWin::SelectMenuPosition(SwWrtShell& rSh, const Point& rMousePos )
{
    const Point aDocPos( PixelToLogic( rMousePos ) );
    const bool bIsInsideSelectedObj( rSh.IsInsideSelectedObj( aDocPos ) );
    //create a synthetic mouse event out of the coordinates
    MouseEvent aMEvt(rMousePos);
    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        // no close of insert_draw and reset of
        // draw mode, if context menu position is inside a selected object.
        if ( !bIsInsideSelectedObj && m_rView.GetDrawFuncPtr() )
        {

            m_rView.GetDrawFuncPtr()->Deactivate();
            m_rView.SetDrawFuncPtr(nullptr);
            m_rView.LeaveDrawCreate();
            SfxBindings& rBind = m_rView.GetViewFrame()->GetBindings();
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
                bool bVertical = pOutliner->IsVertical();
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
                    return;
                }
            }

        }

        if (pSdrView->MouseButtonDown( aMEvt, this ) )
        {
            pSdrView->MouseButtonUp( aMEvt, this );
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(false);
            return;
        }
    }
    rSh.ResetCursorStack();

    if ( EnterDrawMode( aMEvt, aDocPos ) )
    {
        return;
    }
    if ( m_rView.GetDrawFuncPtr() && m_bInsFrame )
    {
        StopInsFrame();
        rSh.Edit();
    }

    UpdatePointer( aDocPos );

    if( !rSh.IsSelFrameMode() &&
        !GetView().GetViewFrame()->GetDispatcher()->IsLocked() )
    {
        // Test if there is a draw object at that position and if it should be selected.
        bool bShould = rSh.ShouldObjectBeSelected(aDocPos);

        if(bShould)
        {
            m_rView.NoRotate();
            rSh.HideCursor();

            bool bUnLockView = !rSh.IsViewLocked();
            rSh.LockView( true );
            bool bSelObj = rSh.SelectObj( aDocPos );
            if( bUnLockView )
                rSh.LockView( false );

            if( bSelObj )
            {
                // in case the frame was deselected in the macro
                // just the cursor has to be displayed again.
                if( FrameTypeFlags::NONE == rSh.GetSelFrameType() )
                    rSh.ShowCursor();
                else
                {
                    if (rSh.IsFrameSelected() && m_rView.GetDrawFuncPtr())
                    {
                        m_rView.GetDrawFuncPtr()->Deactivate();
                        m_rView.SetDrawFuncPtr(nullptr);
                        m_rView.LeaveDrawCreate();
                        m_rView.AttrChangedNotify( &rSh );
                    }

                    rSh.EnterSelFrameMode( &aDocPos );
                    g_bFrameDrag = true;
                    UpdatePointer( aDocPos );
                    return;
                }
            }

            if (!m_rView.GetDrawFuncPtr())
                rSh.ShowCursor();
        }
    }
    else if ( rSh.IsSelFrameMode() &&
              (m_aActHitType == SdrHitKind::NONE ||
               !bIsInsideSelectedObj))
    {
        m_rView.NoRotate();
        bool bUnLockView = !rSh.IsViewLocked();
        rSh.LockView( true );
        sal_uInt8 nFlag = 0;

        if ( rSh.IsSelFrameMode() )
        {
            rSh.UnSelectFrame();
            rSh.LeaveSelFrameMode();
            m_rView.AttrChangedNotify(&rSh);
        }

        bool bSelObj = rSh.SelectObj( aDocPos, nFlag );
        if( bUnLockView )
            rSh.LockView( false );

        if( !bSelObj )
        {
            // move cursor here so that it is not drawn in the
            // frame at first; ShowCursor() happens in LeaveSelFrameMode()
            g_bValidCursorPos = !(CRSR_POSCHG & rSh.CallSetCursor(&aDocPos, false));
            rSh.LeaveSelFrameMode();
            m_rView.LeaveDrawCreate();
            m_rView.AttrChangedNotify( &rSh );
        }
        else
        {
            rSh.HideCursor();
            rSh.EnterSelFrameMode( &aDocPos );
            rSh.SelFlyGrabCursor();
            rSh.MakeSelVisible();
            g_bFrameDrag = true;
            if( rSh.IsFrameSelected() &&
                m_rView.GetDrawFuncPtr() )
            {
                m_rView.GetDrawFuncPtr()->Deactivate();
                m_rView.SetDrawFuncPtr(nullptr);
                m_rView.LeaveDrawCreate();
                m_rView.AttrChangedNotify( &rSh );
            }
            UpdatePointer( aDocPos );
        }
    }
    else if ( rSh.IsSelFrameMode() && bIsInsideSelectedObj )
    {
        // Object at the mouse cursor is already selected - do nothing
        return;
    }

    if ( rSh.IsGCAttr() )
    {
        rSh.GCAttr();
        rSh.ClearGCAttr();
    }

    bool bOverSelect = rSh.TestCurrPam( aDocPos );
    bool bOverURLGrf = false;
    if( !bOverSelect )
        bOverURLGrf = bOverSelect = nullptr != rSh.IsURLGrfAtPos( aDocPos );

    if ( !bOverSelect )
    {
        {   // create only temporary move context because otherwise
            // the query against the content form doesn't work!!!
            SwMvContext aMvContext( &rSh );
            rSh.CallSetCursor(&aDocPos, false);
        }
    }
    if( !bOverURLGrf )
    {
        const int nSelType = rSh.GetSelectionType();
        if( nSelType == nsSelectionType::SEL_OLE ||
            nSelType == nsSelectionType::SEL_GRF )
        {
            SwMvContext aMvContext( &rSh );
            if( !rSh.IsFrameSelected() )
                rSh.GotoNextFly();
            rSh.EnterSelFrameMode();
        }
    }
}

static SfxShell* lcl_GetTextShellFromDispatcher( SwView& rView )
{
    // determine Shell
    SfxShell* pShell;
    SfxDispatcher* pDispatcher = rView.GetViewFrame()->GetDispatcher();
    for(sal_uInt16  i = 0; true; ++i )
    {
        pShell = pDispatcher->GetShell( i );
        if( !pShell || dynamic_cast< const SwTextShell *>( pShell ) !=  nullptr )
            break;
    }
    return pShell;
}

IMPL_LINK_NOARG(SwEditWin, KeyInputFlushHandler, Timer *, void)
{
    FlushInBuffer();
}

IMPL_LINK_NOARG(SwEditWin, KeyInputTimerHandler, Timer *, void)
{
    m_bTableInsDelMode = false;
}

void SwEditWin::InitStaticData()
{
    m_pQuickHlpData = new QuickHelpData();
}

void SwEditWin::FinitStaticData()
{
    delete m_pQuickHlpData;
}
/* i#3370 - remove quick help to prevent saving
 * of autocorrection suggestions */
void SwEditWin::StopQuickHelp()
{
    if( HasFocus() && m_pQuickHlpData && m_pQuickHlpData->m_bIsDisplayed  )
        m_pQuickHlpData->Stop( m_rView.GetWrtShell() );
}

IMPL_LINK_NOARG(SwEditWin, TemplateTimerHdl, Timer *, void)
{
    SetApplyTemplate(SwApplyTemplate());
}

void SwEditWin::SetChainMode( bool bOn )
{
    if ( !m_bChainMode )
        StopInsFrame();

    if ( m_pUserMarker )
    {
        delete m_pUserMarker;
        m_pUserMarker = nullptr;
    }

    m_bChainMode = bOn;

    static sal_uInt16 aInva[] =
    {
        FN_FRAME_CHAIN, FN_FRAME_UNCHAIN, 0
    };
    m_rView.GetViewFrame()->GetBindings().Invalidate(aInva);
}

uno::Reference< css::accessibility::XAccessible > SwEditWin::CreateAccessible()
{
    SolarMutexGuard aGuard;   // this should have happened already!!!
    SwWrtShell *pSh = m_rView.GetWrtShellPtr();
    OSL_ENSURE( pSh, "no writer shell, no accessible object" );
    uno::Reference<
        css::accessibility::XAccessible > xAcc;
    if( pSh )
        xAcc = pSh->CreateAccessible();

    return xAcc;
}

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

void QuickHelpData::ClearContent()
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

    vcl::Window& rWin = rSh.GetView().GetEditWin();
    if( m_bIsTip )
    {
        Point aPt( rWin.OutputToScreenPixel( rWin.LogicToPixel(
                    rSh.GetCharRect().Pos() )));
        aPt.Y() -= 3;
        nTipId = Help::ShowPopover(&rWin, Rectangle( aPt, Size( 1, 1 )),
                        m_aHelpStrings[ nCurArrPos ],
                        QuickHelpFlags::Left | QuickHelpFlags::Bottom);
    }
    else
    {
        OUString sStr( m_aHelpStrings[ nCurArrPos ] );
        sStr = sStr.copy( nLen );
        sal_uInt16 nL = sStr.getLength();
        const ExtTextInputAttr nVal = ExtTextInputAttr::DottedUnderline |
                                ExtTextInputAttr::Highlight;
        const std::vector<ExtTextInputAttr> aAttrs( nL, nVal );
        CommandExtTextInputData aCETID( sStr, &aAttrs[0], nL,
                                        0, false );

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
        rSh.DeleteExtTextInput( false );
    else if( nTipId )
    {
        vcl::Window& rWin = rSh.GetView().GetEditWin();
        Help::HidePopover(&rWin, nTipId);
    }
    ClearContent();
}

void QuickHelpData::FillStrArr( SwWrtShell& rSh, const OUString& rWord )
{
    enum Capitalization { CASE_LOWER, CASE_UPPER, CASE_SENTENCE, CASE_OTHER };

    // Determine word capitalization
    const CharClass& rCC = GetAppCharClass();
    const OUString sWordLower = rCC.lowercase( rWord );
    Capitalization aWordCase = CASE_OTHER;
    if ( !rWord.isEmpty() )
    {
        if ( rWord[0] == sWordLower[0] )
        {
            if ( rWord == sWordLower )
                aWordCase = CASE_LOWER;
        }
        else
        {
            // First character is not lower case i.e. assume upper or title case
            OUString sWordSentence = sWordLower;
            sWordSentence = sWordSentence.replaceAt( 0, 1, OUString(rWord[0]) );
            if ( rWord == sWordSentence )
                aWordCase = CASE_SENTENCE;
            else
            {
                if ( rWord == rCC.uppercase( rWord ) )
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
            const OUString& rStr( aNames[n].FullName );
            // Check string longer than word and case insensitive match
            if( rStr.getLength() > rWord.getLength() &&
                rCC.lowercase( rStr, 0, rWord.getLength() ) == sWordLower )
            {
                //fdo#61251 if it's an exact match, ensure unchanged replacement
                //exists as a candidate
                if (rStr.startsWith(rWord))
                    m_aHelpStrings.push_back(rStr);

                if ( aWordCase == CASE_LOWER )
                    m_aHelpStrings.push_back( rCC.lowercase( rStr ) );
                else if ( aWordCase == CASE_SENTENCE )
                {
                    OUString sTmp = rCC.lowercase( rStr );
                    sTmp = sTmp.replaceAt( 0, 1, OUString(rStr[0]) );
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

    // Add matching current date in ISO 8601 format, for example 2016-01-30
    OUString rStrToday;

    if (rWord[0] == '2')
    {
        OUStringBuffer rStr("");
        rStr.append(sal::static_int_cast< sal_Int32 >((*pCalendar)->getValue(i18n::CalendarFieldIndex::YEAR))).append("-");
        sal_Int32 nMonth = sal::static_int_cast< sal_Int32 >((*pCalendar)->getValue(i18n::CalendarFieldIndex::MONTH)+1);
        sal_Int32 nDay = sal::static_int_cast< sal_Int32 > ((*pCalendar)->getValue(i18n::CalendarFieldIndex::DAY_OF_MONTH));
        if (nMonth < 10)
            rStr.append("0");
        rStr.append(nMonth).append("-");
        if (nDay < 10)
            rStr.append("0");
        rStrToday = rStr.append(nDay).toString();

        // do not suggest for single years, for example for "2016",
        // only for "201" or "2016-..." (to avoid unintentional text
        // insertion at line ending, for example typing "30 January 2016")
        if (rWord.getLength() != 4 && rStrToday.startsWith(rWord))
            m_aHelpStrings.push_back(rStrToday);
    }

    // Add matching words from AutoCompleteWord list
    const SwAutoCompleteWord& rACList = SwEditShell::GetAutoCompleteWords();
    std::vector<OUString> strings;

    if ( rACList.GetWordsMatching( rWord, strings ) )
    {
        for (const OUString & aCompletedString : strings)
        {
            // when we have a matching current date, avoid to suggest
            // other words with the same matching starting characters,
            // for example 2016-01-3 instead of 2016-01-30
            if (!rStrToday.isEmpty() && aCompletedString.startsWith(rWord))
                continue;

            //fdo#61251 if it's an exact match, ensure unchanged replacement
            //exists as a candidate
            if (aCompletedString.startsWith(rWord))
                m_aHelpStrings.push_back(aCompletedString);
            if ( aWordCase == CASE_LOWER )
                m_aHelpStrings.push_back( rCC.lowercase( aCompletedString ) );
            else if ( aWordCase == CASE_SENTENCE )
            {
                OUString sTmp = rCC.lowercase( aCompletedString );
                sTmp = sTmp.replaceAt( 0, 1, OUString(aCompletedString[0]) );
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

class CompareIgnoreCaseAsciiFavorExact
    : public std::binary_function<const OUString&, const OUString&, bool>
{
    const OUString &m_rOrigWord;
public:
    explicit CompareIgnoreCaseAsciiFavorExact(const OUString& rOrigWord)
        : m_rOrigWord(rOrigWord)
    {
    }

    bool operator()(const OUString& s1, const OUString& s2) const
    {
        int nRet = s1.compareToIgnoreAsciiCase(s2);
        if (nRet == 0)
        {
            //fdo#61251 sort stuff that starts with the exact rOrigWord before
            //another ignore-case candidate
            int n1StartsWithOrig = s1.startsWith(m_rOrigWord) ? 0 : 1;
            int n2StartsWithOrig = s2.startsWith(m_rOrigWord) ? 0 : 1;
            return n1StartsWithOrig < n2StartsWithOrig;
        }
        return nRet < 0;
    }
};

struct EqualIgnoreCaseAscii
{
    bool operator()(const OUString& s1, const OUString& s2) const
    {
        return s1.equalsIgnoreAsciiCase(s2);
    }
};

} // anonymous namespace

// TODO Implement an i18n aware sort
void QuickHelpData::SortAndFilter(const OUString &rOrigWord)
{
    std::sort( m_aHelpStrings.begin(),
               m_aHelpStrings.end(),
               CompareIgnoreCaseAsciiFavorExact(rOrigWord) );

    std::vector<OUString>::iterator it = std::unique( m_aHelpStrings.begin(),
                                                    m_aHelpStrings.end(),
                                                    EqualIgnoreCaseAscii() );
    m_aHelpStrings.erase( it, m_aHelpStrings.end() );

    nCurArrPos = 0;
}

void SwEditWin::ShowAutoTextCorrectQuickHelp(
        const OUString& rWord, SvxAutoCorrCfg* pACfg, SvxAutoCorrect* pACorr,
        bool bFromIME )
{
    SwWrtShell& rSh = m_rView.GetWrtShell();
    m_pQuickHlpData->ClearContent();
    if( pACfg->IsAutoTextTip() )
    {
        SwGlossaryList* pList = ::GetGlossaryList();
        pList->HasLongName( rWord, &m_pQuickHlpData->m_aHelpStrings );
    }

    if( m_pQuickHlpData->m_aHelpStrings.empty() &&
        pACorr->GetSwFlags().bAutoCompleteWords )
    {
        m_pQuickHlpData->m_bIsAutoText = false;
        m_pQuickHlpData->m_bIsTip = bFromIME ||
                    !pACorr ||
                    pACorr->GetSwFlags().bAutoCmpltShowAsTip;

        // Get the necessary data to show help text.
        m_pQuickHlpData->FillStrArr( rSh, rWord );
    }

    if( !m_pQuickHlpData->m_aHelpStrings.empty() )
    {
        m_pQuickHlpData->SortAndFilter(rWord);
        m_pQuickHlpData->Start( rSh, rWord.getLength() );
    }
}

bool SwEditWin::IsInHeaderFooter( const Point &rDocPt, FrameControlType &rControl ) const
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    const SwPageFrame* pPageFrame = rSh.GetLayout()->GetPageAtPos( rDocPt );

    if ( pPageFrame && pPageFrame->IsOverHeaderFooterArea( rDocPt, rControl ) )
        return true;

    if ( rSh.IsShowHeaderFooterSeparator( Header ) || rSh.IsShowHeaderFooterSeparator( Footer ) )
    {
        SwFrameControlsManager &rMgr = rSh.GetView().GetEditWin().GetFrameControlsManager();
        Point aPoint( LogicToPixel( rDocPt ) );

        if ( rSh.IsShowHeaderFooterSeparator( Header ) )
        {
            SwFrameControlPtr pControl = rMgr.GetControl( Header, pPageFrame );
            if ( pControl.get() && pControl->Contains( aPoint ) )
            {
                rControl = Header;
                return true;
            }
        }

        if ( rSh.IsShowHeaderFooterSeparator( Footer ) )
        {
            SwFrameControlPtr pControl = rMgr.GetControl( Footer, pPageFrame );
            if ( pControl.get() && pControl->Contains( aPoint ) )
            {
                rControl = Footer;
                return true;
            }
        }
    }

    return false;
}

bool SwEditWin::IsOverHeaderFooterFly( const Point& rDocPos, FrameControlType& rControl, bool& bOverFly, bool& bPageAnchored ) const
{
    bool bRet = false;
    Point aPt( rDocPos );
    SwWrtShell &rSh = m_rView.GetWrtShell();
    SwPaM aPam( *rSh.GetCurrentShellCursor().GetPoint() );
    rSh.GetLayout()->GetCursorOfst( aPam.GetPoint(), aPt, nullptr, true );

    const SwStartNode* pStartFly = aPam.GetPoint()->nNode.GetNode().FindFlyStartNode();
    if ( pStartFly )
    {
        bOverFly = true;
        SwFrameFormat* pFlyFormat = pStartFly->GetFlyFormat( );
        if ( pFlyFormat )
        {
            const SwPosition* pAnchor = pFlyFormat->GetAnchor( ).GetContentAnchor( );
            if ( pAnchor )
            {
                bool bInHeader = pAnchor->nNode.GetNode( ).FindHeaderStartNode( ) != nullptr;
                bool bInFooter = pAnchor->nNode.GetNode( ).FindFooterStartNode( ) != nullptr;

                bRet = bInHeader || bInFooter;
                if ( bInHeader )
                    rControl = Header;
                else if ( bInFooter )
                    rControl = Footer;
            }
            else
                bPageAnchored = pFlyFormat->GetAnchor( ).GetAnchorId( ) == FLY_AT_PAGE;
        }
    }
    else
        bOverFly = false;
    return bRet;
}

void SwEditWin::SetUseInputLanguage( bool bNew )
{
    if ( bNew || m_bUseInputLanguage )
    {
        SfxBindings& rBind = GetView().GetViewFrame()->GetBindings();
        rBind.Invalidate( SID_ATTR_CHAR_FONT );
        rBind.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
    }
    m_bUseInputLanguage = bNew;
}

OUString SwEditWin::GetSurroundingText() const
{
    OUString sReturn;
    SwWrtShell& rSh = m_rView.GetWrtShell();
    if( rSh.HasSelection() && !rSh.IsMultiSelection() && rSh.IsSelOnePara() )
        rSh.GetSelectedText( sReturn, GETSELTXT_PARABRK_TO_ONLYCR  );
    else if( !rSh.HasSelection() )
    {
        SwPosition *pPos = rSh.GetCursor()->GetPoint();
        const sal_Int32 nPos = pPos->nContent.GetIndex();

        // get the sentence around the cursor
        rSh.HideCursor();
        rSh.GoStartSentence();
        rSh.SetMark();
        rSh.GoEndSentence();
        rSh.GetSelectedText( sReturn, GETSELTXT_PARABRK_TO_ONLYCR  );

        pPos->nContent = nPos;
        rSh.ClearMark();
        rSh.HideCursor();
    }

    return sReturn;
}

Selection SwEditWin::GetSurroundingTextSelection() const
{
    SwWrtShell& rSh = m_rView.GetWrtShell();
    if( rSh.HasSelection() )
    {
        OUString sReturn;
        rSh.GetSelectedText( sReturn, GETSELTXT_PARABRK_TO_ONLYCR  );
        return Selection( 0, sReturn.getLength() );
    }
    else
    {
        // Return the position of the visible cursor in the sentence
        // around the visible cursor.
        SwPosition *pPos = rSh.GetCursor()->GetPoint();
        const sal_Int32 nPos = pPos->nContent.GetIndex();

        rSh.HideCursor();
        rSh.GoStartSentence();
        const sal_Int32 nStartPos = rSh.GetCursor()->GetPoint()->nContent.GetIndex();

        pPos->nContent = nPos;
        rSh.ClearMark();
        rSh.ShowCursor();

        return Selection( nPos - nStartPos, nPos - nStartPos );
    }
}

void SwEditWin::LogicInvalidate(const Rectangle* pRectangle)
{
    OString sRectangle;
    if (!pRectangle)
        sRectangle = "EMPTY";
    else
        sRectangle = pRectangle->toString();

    SfxLokHelper::notifyInvalidation(&m_rView, sRectangle);
}

void SwEditWin::LogicMouseButtonDown(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    Point aPoint = GetPointerPosPixel();
    SetLastMousePos(rMouseEvent.GetPosPixel());

    MouseButtonDown(rMouseEvent);

    SetPointerPosPixel(aPoint);
}

void SwEditWin::LogicMouseButtonUp(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    Point aPoint = GetPointerPosPixel();
    SetLastMousePos(rMouseEvent.GetPosPixel());

    MouseButtonUp(rMouseEvent);

    SetPointerPosPixel(aPoint);
}

void SwEditWin::LogicMouseMove(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    Point aPoint = GetPointerPosPixel();
    SetLastMousePos(rMouseEvent.GetPosPixel());

    MouseMove(rMouseEvent);

    SetPointerPosPixel(aPoint);
}

void SwEditWin::SetCursorTwipPosition(const Point& rPosition, bool bPoint, bool bClearMark)
{
    if (SdrView* pSdrView = m_rView.GetWrtShell().GetDrawView())
    {
        // Editing shape text, then route the call to editeng.
        if (pSdrView->GetTextEditObject())
        {
            EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
            rEditView.SetCursorLogicPosition(rPosition, bPoint, bClearMark);
            return;
        }
    }

    if (m_rView.GetPostItMgr())
    {
        if (sw::sidebarwindows::SwSidebarWin* pWin = m_rView.GetPostItMgr()->GetActiveSidebarWin())
        {
            // Editing postit text.
            pWin->SetCursorLogicPosition(rPosition, bPoint, bClearMark);
            return;
        }
    }

    // Not an SwWrtShell, as that would make SwCursorShell::GetCursor() inaccessible.
    SwEditShell& rShell = m_rView.GetWrtShell();

    bool bCreateSelection = false;
    {
        SwMvContext aMvContext(&rShell);
        if (bClearMark)
            rShell.ClearMark();
        else
            bCreateSelection = !rShell.HasMark();

        if (bCreateSelection)
            m_rView.GetWrtShell().SttSelect();

        // If the mark is to be updated, then exchange the point and mark before
        // and after, as we can't easily set the mark.
        if (!bPoint)
            rShell.getShellCursor(/*bBlock=*/false)->Exchange();
        rShell.SetCursor(rPosition);
        if (!bPoint)
            rShell.getShellCursor(/*bBlock=*/false)->Exchange();
    }

    if (bCreateSelection)
        m_rView.GetWrtShell().EndSelect();
}

void SwEditWin::SetGraphicTwipPosition(bool bStart, const Point& rPosition)
{
    if (bStart)
    {
        MouseEvent aClickEvent(rPosition, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
        MouseButtonDown(aClickEvent);
        MouseEvent aMoveEvent(Point(rPosition.getX() + MIN_MOVE + 1, rPosition.getY()), 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
        MouseMove(aMoveEvent);
    }
    else
    {
        MouseEvent aMoveEvent(Point(rPosition.getX() - MIN_MOVE - 1, rPosition.getY()), 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
        MouseMove(aMoveEvent);
        MouseEvent aClickEvent(rPosition, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
        MouseButtonUp(aClickEvent);
    }
}

SwFrameControlsManager& SwEditWin::GetFrameControlsManager()
{
    return *m_pFrameControlsManager;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
