/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

#include <touch/touch-impl.h>

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

#include <unotools/syslocaleoptions.hxx>

using namespace sw::mark;
using namespace ::com::sun::star;

/**
 * Globals
 */
static bool bInputLanguageSwitched = false;
extern bool bNoInterrupt;       






static bool bHoldSelection      = false;

bool bFrmDrag                   = false;
bool bValidCrsrPos              = false;
bool bModePushed                = false;
bool bDDTimerStarted            = false;
bool bFlushCharBuffer           = false;
bool bDDINetAttr                = false;
SdrHdlKind eSdrMoveHdl          = HDL_USER;

QuickHelpData* SwEditWin::m_pQuickHlpData = 0;

long    SwEditWin::m_nDDStartPosY = 0;
long    SwEditWin::m_nDDStartPosX = 0;
/**
 * The initial color shown on the button is set in /core/svx/source/tbxctrls/tbxcolorupdate.cxx
 * (ToolboxButtonColorUpdater::ToolboxButtonColorUpdater()) .
 * The initial color used by the button is set in /core/svx/source/tbxcntrls/tbcontrl.cxx
 * (SvxColorExtToolBoxControl::SvxColorExtToolBoxControl())
 * and in case of writer for text(background)color also in /core/sw/source/ui/docvw/edtwin.cxx
 * (SwEditWin::aTextBackColor and SwEditWin::aTextBackColor)
 */
Color   SwEditWin::m_aTextBackColor(COL_YELLOW);
Color   SwEditWin::m_aTextColor(COL_RED);
bool SwEditWin::m_bTransparentBackColor = false; 

extern bool     bExecuteDrag;

static SfxShell* lcl_GetShellFromDispatcher( SwView& rView, TypeId nType );

DBG_NAME(edithdl)

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
    SdrHdl* GetHdl() const { return pHdl; }
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


struct QuickHelpData
{
    
    std::vector<OUString> m_aHelpStrings;
    
    sal_uInt16 nCurArrPos;
    
    sal_uInt16 nLen;

    
    bool m_bIsAutoText;
    
    bool m_bIsTip;
    
    sal_uLong nTipId;
    
    bool m_bAppendSpace;

    
    bool m_bIsDisplayed;

    QuickHelpData() { ClearCntnt(); }

    void Move( QuickHelpData& rCpy );
    void ClearCntnt();
    void Start( SwWrtShell& rSh, sal_uInt16 nWrdLen );
    void Stop( SwWrtShell& rSh );

    bool HasCntnt() const { return !m_aHelpStrings.empty() && 0 != nLen; }

    
    void Next( bool bEndLess )
    {
        if( ++nCurArrPos >= m_aHelpStrings.size() )
            nCurArrPos = (bEndLess && !m_bIsAutoText ) ? 0 : nCurArrPos-1;
    }
    
    void Previous( bool bEndLess )
    {
        if( 0 == nCurArrPos-- )
            nCurArrPos = (bEndLess && !m_bIsAutoText ) ? m_aHelpStrings.size()-1 : 0;
    }

    
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
 * hyperlinks if applicable (DownLoad/NewWindow!)
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
    default:; 
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
        PointerStyle eStyle = POINTER_FILL;
        if( rSh.IsOverReadOnlyPos( rLPt ))
        {
            delete m_pUserMarker;
            m_pUserMarker = 0L;

            eStyle = POINTER_NOTALLOWED;
        }
        else
        {
            SwRect aRect;
            SwRect* pRect = &aRect;
            const SwFrmFmt* pFmt = 0;

            bool bFrameIsValidTarget = false;
            if( m_pApplyTempl->m_pFormatClipboard )
                bFrameIsValidTarget = m_pApplyTempl->m_pFormatClipboard->HasContentForThisType( nsSelectionType::SEL_FRM );
            else if( !m_pApplyTempl->nColor )
                bFrameIsValidTarget = ( m_pApplyTempl->eType == SFX_STYLE_FAMILY_FRAME );

            if( bFrameIsValidTarget &&
                        0 !=(pFmt = rSh.GetFmtFromObj( rLPt, &pRect )) &&
                        PTR_CAST(SwFlyFrmFmt, pFmt))
            {
                
                Rectangle aTmp( pRect->SVRect() );

                if ( !m_pUserMarker )
                {
                    m_pUserMarker = new SdrDropMarkerOverlay( *rSh.GetDrawView(), aTmp );
                }
            }
            else
            {
                delete m_pUserMarker;
                m_pUserMarker = 0L;
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

            if ( !m_pUserMarker )
            {
                m_pUserMarker = new SdrDropMarkerOverlay( *rSh.GetDrawView(), aTmp );
            }
        }
        else
        {
            delete m_pUserMarker;
            m_pUserMarker = 0L;
        }

        SetPointer( eStyle );
        return;
    }

    sal_Bool bExecHyperlinks = m_rView.GetDocShell()->IsReadOnly();
    if ( !bExecHyperlinks )
    {
        SvtSecurityOptions aSecOpts;
        const sal_Bool bSecureOption = aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );
        if ( (  bSecureOption && nModifier == KEY_MOD1 ) ||
             ( !bSecureOption && nModifier != KEY_MOD1 ) )
            bExecHyperlinks = sal_True;
    }

    const bool bExecSmarttags  = nModifier == KEY_MOD1;

    SdrView *pSdrView = rSh.GetDrawView();
    bool bPrefSdrPointer = false;
    bool bHitHandle = false;
    bool bCntAtPos = false;
    bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly() &&
                          rSh.IsCrsrReadonly();
    m_aActHitType = SDRHIT_NONE;
    PointerStyle eStyle = POINTER_TEXT;
    if ( !pSdrView )
        bCntAtPos = true;
    else if ( (bHitHandle = pSdrView->PickHandle( rLPt ) != 0) )
    {
        m_aActHitType = SDRHIT_OBJECT;
        bPrefSdrPointer = true;
    }
    else
    {
        const bool bNotInSelObj = !rSh.IsInsideSelectedObj( rLPt );
        if ( m_rView.GetDrawFuncPtr() && !m_bInsDraw && bNotInSelObj )
        {
            m_aActHitType = SDRHIT_OBJECT;
            if (IsObjectSelect())
                eStyle = POINTER_ARROW;
            else
                bPrefSdrPointer = true;
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
                
                
                
                if ( rSh.IsObjSelectable( rLPt ) )
                {
                    if (pSdrView->IsTextEdit())
                    {
                        m_aActHitType = SDRHIT_NONE;
                        bPrefSdrPointer = true;
                    }
                    else
                    {
                        SdrViewEvent aVEvt;
                        SdrHitKind eHit = pSdrView->PickAnything(rLPt, aVEvt);

                        if (eHit == SDRHIT_URLFIELD && bExecHyperlinks)
                        {
                            m_aActHitType = SDRHIT_OBJECT;
                            bPrefSdrPointer = true;
                        }
                        else
                        {
                            
                            
                            
                            
                            bool bMovable =
                                (!bNotInSelObj) &&
                                (rSh.IsObjSelected() || rSh.IsFrmSelected()) &&
                                (!rSh.IsSelObjProtected(FLYPROTECT_POS));

                            SdrObject* pSelectableObj = rSh.GetObjAt(rLPt);
                            
                            if (pSelectableObj->GetLayer() != rSh.GetDoc()->GetHellId())
                                eStyle = bMovable ? POINTER_MOVE : POINTER_ARROW;
                            m_aActHitType = SDRHIT_OBJECT;
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
                        m_aActHitType = SDRHIT_OBJECT;
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
        if (bIsDocReadOnly || (rSh.IsObjSelected() && rSh.IsSelObjProtected(FLYPROTECT_CONTENT)))
            SetPointer( POINTER_NOTALLOWED );
        else
        {
            if (m_rView.GetDrawFuncPtr() && m_rView.GetDrawFuncPtr()->IsInsertForm() && !bHitHandle)
                SetPointer( POINTER_DRAW_RECT );
            else
                SetPointer( pSdrView->GetPreferredPointer( rLPt, rSh.GetOut() ) );
        }
    }
    else
    {
        if( !rSh.IsPageAtPos( rLPt ) || m_pAnchorMarker )
            eStyle = POINTER_ARROW;
        else
        {
            
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

        
        if( POINTER_TEXT == eStyle && rSh.IsInVerticalText( &rLPt ))
            eStyle = POINTER_TEXT_VERTICAL;

        SetPointer( eStyle );
    }
}

/**
 * Increase timer for selection
 */
IMPL_LINK_NOARG(SwEditWin, TimerHandler)
{
    DBG_PROFSTART(edithdl);

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
        else if ( bFrmDrag )
        {
            rSh.Drag(&aModPt, false);
            bDone = true;
        }
        if ( !bDone )
            aModPt = rSh.GetCntntPos( aModPt,aModPt.Y() > rSh.VisArea().Bottom() );
    }
    if ( !bDone && !(bFrmDrag || m_bInsDraw) )
    {
        if ( m_pRowColumnSelectionStart )
        {
            Point aPos( aModPt );
            rSh.SelectTableRowCol( *m_pRowColumnSelectionStart, &aPos, m_bIsRowDrag );
        }
        else
            rSh.SetCursor( &aModPt, false );

        
        
        const SwRect& rVisArea = rSh.VisArea();
        if( aOldVis == rVisArea && !rSh.IsStartOfDoc() && !rSh.IsEndOfDoc() )
        {
            
            
            if( aModPt.Y() < ( rVisArea.Top() + rVisArea.Height() / 2 ) )
                rSh.Up( sal_True, 1 );
            else
                rSh.Down( sal_True, 1 );
        }
    }

    m_aMovePos += rSh.VisArea().Pos() - aOldVis.Pos();
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
    delete m_pShadCrsr, m_pShadCrsr = 0;
}

inline void SwEditWin::EnterArea()
{
    m_aTimer.Stop();
}

/**
 * Insert mode for frames
 */
void SwEditWin::InsFrm(sal_uInt16 nCols)
{
    StdDrawMode( OBJ_NONE, sal_False );
    m_bInsFrm = sal_True;
    m_nInsFrmColCount = nCols;
}

void SwEditWin::StdDrawMode( SdrObjKind eSdrObjectKind, sal_Bool bObjSelect )
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
    m_bInsFrm = sal_False;
    m_nInsFrmColCount = 1;
}

void SwEditWin::StopInsFrm()
{
    if (m_rView.GetDrawFuncPtr())
    {
        m_rView.GetDrawFuncPtr()->Deactivate();
        m_rView.SetDrawFuncPtr(NULL);
    }
    m_rView.LeaveDrawCreate();    
    m_bInsFrm = sal_False;
    m_nInsFrmColCount = 1;
}



sal_Bool SwEditWin::IsInputSequenceCheckingRequired( const OUString &rText, const SwPaM& rCrsr ) const
{
    const SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();
    if ( !rCTLOptions.IsCTLFontEnabled() ||
         !rCTLOptions.IsCTLSequenceChecking() )
         return sal_False;

    if ( 0 == rCrsr.Start()->nContent.GetIndex() ) /* first char needs not to be checked */
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

    return (0 <= nCTLScriptPos && nCTLScriptPos <= rText.getLength());
}



static sal_uInt16 lcl_isNonDefaultLanguage(LanguageType eBufferLanguage, SwView& rView,
    const OUString &rInBuffer)
{
    sal_uInt16 nWhich = INVALID_HINT;

    
    
    
    SvtSysLocaleOptions aSysLocaleOptions;
    if(aSysLocaleOptions.IsIgnoreLanguageChange())
    {
        return INVALID_HINT;
    }

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
            if(SFX_ITEM_DEFAULT <= aLangSet.GetItemState(nWhich, true))
            {
                LanguageType eLang = static_cast<const SvxLanguageItem&>(aLangSet.Get(nWhich)).GetLanguage();
                if ( eLang == eBufferLanguage )
                {
                    
                    bLang = false;
                }
                else if ( !bInputLanguageSwitched && RES_CHRATR_LANGUAGE == nWhich )
                {
                    
                    
                    
                    
                    
                    
                    //
                    
                    
                    
                    
                    
                    //
                    
                    
                    
                    
                    //
                    
                    
                    
                    
                    
                    
                    i18n::UnicodeScript eType = !rInBuffer.isEmpty() ?
                        (i18n::UnicodeScript)GetAppCharClass().getScript( rInBuffer, 0 ) :
                        i18n::UnicodeScript_kScriptCount;

                    bool bSystemIsNonLatin = false;
                    switch ( eType )
                    {
                        case i18n::UnicodeScript_kGreek:
                        case i18n::UnicodeScript_kCyrillic:
                            
                            
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

        
        if ( !pCheckIt )
            pCheckIt = new SwCheckIt;

        uno::Reference < i18n::XExtendedInputSequenceChecker > xISC = pCheckIt->xCheck;
        if ( xISC.is() && IsInputSequenceCheckingRequired( m_aInBuffer, *rSh.GetCrsr() ) )
        {
            //
            
            //

            rSh.Push(); 

            
            
            rSh.NormalizePam();     
            if (!rSh.GetCrsr()->HasMark())
                rSh.GetCrsr()->SetMark();
            rSh.GetCrsr()->GetMark()->nContent = 0;

            const OUString aOldText( rSh.GetCrsr()->GetTxt() );
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

                        
                        if (nPrevPos != aNewText.getLength())
                            nTmpPos = nPrevPos + 1;
                    }

                    
                    sal_Int32 nNewLen = aNewText.getLength();
                    const sal_Unicode *pOldTxt = aOldText.getStr();
                    const sal_Unicode *pNewTxt = aNewText.getStr();
                    sal_Int32 nChgPos = 0;
                    while ( nChgPos < nOldLen && nChgPos < nNewLen &&
                            pOldTxt[nChgPos] == pNewTxt[nChgPos] )
                        ++nChgPos;

                    const sal_Int32 nChgLen = nNewLen - nChgPos;
                    if (nChgLen)
                    {
                        m_aInBuffer = aNewText.copy( nChgPos, nChgLen );
                        nExpandSelection = nOldLen - nChgPos;
                    }
                    else
                        m_aInBuffer = "";
                }
                else
                {
                    for( sal_Int32 k = 0;  k < m_aInBuffer.getLength(); ++k )
                    {
                        const sal_Unicode cChar = m_aInBuffer[k];
                        if (xISC->checkInputSequence( aNewText, nTmpPos - 1, cChar, nCheckMode ))
                        {
                            
                            aNewText += OUString( (sal_Unicode) cChar );
                            ++nTmpPos;
                        }
                    }
                    m_aInBuffer = aNewText.copy( aOldText.getLength() );  
                }
            }

            

            rSh.Pop( sal_False );  

            if (m_aInBuffer.isEmpty())
                return;

            
            
            SwPaM &rCrsr = *rSh.GetCrsr();
            const sal_Int32 nCrsrStartPos = rCrsr.Start()->nContent.GetIndex();
            OSL_ENSURE( nCrsrStartPos >= nExpandSelection, "cannot expand selection as specified!!" );
            if (nExpandSelection && nCrsrStartPos >= nExpandSelection)
            {
                if (!rCrsr.HasMark())
                    rCrsr.SetMark();
                rCrsr.Start()->nContent -= nExpandSelection;
            }
        }

        uno::Reference< frame::XDispatchRecorder > xRecorder =
                m_rView.GetViewFrame()->GetBindings().GetRecorder();
        if ( xRecorder.is() )
        {
            
            SfxShell *pSfxShell = lcl_GetShellFromDispatcher( m_rView, TYPE(SwTextShell) );
            
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
        m_aInBuffer = "";
        bFlushCharBuffer = false;
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


#define MOVE_LEFT_HUGE      8
#define MOVE_UP_HUGE        9
#define MOVE_RIGHT_HUGE     10
#define MOVE_DOWN_HUGE      11

void SwEditWin::ChangeFly( sal_uInt8 nDir, sal_Bool bWeb )
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    SwRect aTmp = rSh.GetFlyRect();
    if( aTmp.HasArea() &&
        !rSh.IsSelObjProtected( FLYPROTECT_POS ) )
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
            
            aSnap = Size(aSnap.Width() * 3, aSnap.Height() * 3);
        }

        SwRect aBoundRect;
        Point aRefPoint;
        
        
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
            case MOVE_RIGHT_HUGE:
            case MOVE_RIGHT_BIG: aTmp.Left( aTmp.Left() + nRight );
                break;

            case MOVE_DOWN_SMALL:
                if( aTmp.Height() < aSnap.Height() + MINFLY )
                    break;
                nDown = aSnap.Height(); 
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
            SwFmtVertOrient aVert( (SwFmtVertOrient&)aSet.Get(RES_VERT_ORIENT) );
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
                    default:; 
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
                default:; 
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
            rSh.SetFlyFrmAttr( aSet );
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
    sal_uInt16 nAnchorDir = SW_MOVE_UP;
    switch(nDir)
    {
        case MOVE_LEFT_SMALL:
        case MOVE_LEFT_HUGE:
        case MOVE_LEFT_BIG:
            nX = -1;
            nAnchorDir = SW_MOVE_LEFT;
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
            nAnchorDir = SW_MOVE_RIGHT;
        break;
        case MOVE_DOWN_SMALL:
        case MOVE_DOWN_HUGE:
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
            
            aSnap = Size(aSnap.Width() * 3, aSnap.Height() * 3);
        }

        nX *= aSnap.Width();
        nY *= aSnap.Height();

        SdrView *pSdrView = rSh.GetDrawView();
        const SdrHdlList& rHdlList = pSdrView->GetHdlList();
        SdrHdl* pHdl = rHdlList.GetFocusHdl();
        rSh.StartAllAction();
        if(0L == pHdl)
        {
            
            
            if(0 == (nProtect&FLYPROTECT_POS))
            {
                
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
            
            if(pHdl && (nX || nY))
            {
                if( HDL_ANCHOR == pHdl->GetKind() ||
                    HDL_ANCHOR_TR == pHdl->GetKind() )
                {
                    
                    if(0 == (nProtect&FLYPROTECT_POS))
                        rSh.MoveAnchor( nAnchorDir );
                }
                
                else if(0 == (nProtect&FLYPROTECT_SIZE))
                {
                    
                    Point aStartPoint(pHdl->GetPos());
                    Point aEndPoint(pHdl->GetPos() + Point(nX, nY));
                    const SdrDragStat& rDragStat = pSdrView->GetDragStat();

                    
                    pSdrView->BegDragObj(aStartPoint, 0, pHdl, 0);

                    if(pSdrView->IsDragObj())
                    {
                        sal_Bool bWasNoSnap = static_cast< sal_Bool >(rDragStat.IsNoSnap());
                        sal_Bool bWasSnapEnabled = pSdrView->IsSnapEnabled();

                        
                        if(!bWasNoSnap)
                            ((SdrDragStat&)rDragStat).SetNoSnap(true);
                        if(bWasSnapEnabled)
                            pSdrView->SetSnapEnabled(sal_False);

                        pSdrView->MovAction(aEndPoint);
                        pSdrView->EndDragObj();
                        rSh.SetModified();

                        
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

/**
 * KeyEvents
 */
void SwEditWin::KeyInput(const KeyEvent &rKEvt)
{
    SwWrtShell &rSh = m_rView.GetWrtShell();

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
        bool bHeader = FRMTYPE_HEADER & rSh.GetFrmType(0,sal_False);
        if ( bHeader )
            rSh.SttPg();
        else
            rSh.EndPg();
        rSh.ToggleHeaderFooterEdit();
    }

    SfxObjectShell *pObjSh = (SfxObjectShell*)m_rView.GetViewFrame()->GetObjectShell();
    if ( m_bLockInput || (pObjSh && pObjSh->GetProgress()) )
        
        
        return;

    delete m_pShadCrsr, m_pShadCrsr = 0;
    m_aKeyInputFlushTimer.Stop();

    bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly() &&
                          rSh.IsCrsrReadonly();

    
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

    
    if ( !bIsDocReadOnly && rSh.GetDrawView() && rSh.GetDrawView()->KeyInput( rKEvt, this ) )
    {
        rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll( sal_False );
        rSh.SetModified();
        return; 
    }

    if ( m_rView.GetDrawFuncPtr() && m_bInsFrm )
    {
        StopInsFrm();
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
            SwRootFrm* pLayout = GetView().GetDocShell()->GetWrtShell()->GetLayout();
            pLayout->dumpAsXml( );
            return;
        }
    }

    KeyEvent aKeyEvent( rKEvt );
    
    if( !bIsDocReadOnly && !rSh.IsSelFrmMode() && !rSh.IsObjSelected() )
    {
        
        
        
        sal_uInt16 nKey = rKEvt.GetKeyCode().GetCode();

        if( KEY_UP == nKey || KEY_DOWN == nKey ||
            KEY_LEFT == nKey || KEY_RIGHT == nKey )
        {
            
            
            
            
            
            
            const bool bVertText = rSh.IsInVerticalText();
            const bool bTblCrsr = rSh.GetTableCrsr();
            const bool bVertTable = rSh.IsTableVertical();
            if( ( bVertText && ( !bTblCrsr || bVertTable ) ) ||
                ( bTblCrsr && bVertTable ) )
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
    }

    const KeyCode& rKeyCode = aKeyEvent.GetKeyCode();
    sal_Unicode aCh = aKeyEvent.GetCharCode();

    
    
    if ((rKeyCode.IsMod1() && rKeyCode.IsMod2()) && ((rKeyCode.GetCode() == KEY_PAGEUP) || (rKeyCode.GetCode() == KEY_PAGEDOWN)))
    {
        const bool bNext = rKeyCode.GetCode()==KEY_PAGEDOWN ? true : false;
        const SwFieldType* pFldType = rSh.GetFldType( 0, RES_POSTITFLD );
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

            OUString sRet;
            rSh.ExecMacro( *pMacro, &sRet, &xArgs );
            if( !sRet.isEmpty() && sRet.toInt32()!=0 )
                return ;
        }
    }
    int nLclSelectionType;
    
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
            m_rView.GetViewFrame()->GetBindings().GetRecorder();
    if ( !xRecorder.is() )
    {
        pACfg = &SvxAutoCorrCfg::Get();
        pACorr = pACfg->GetAutoCorrect();
    }

    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    TblChgWidthHeightType eTblChgMode = nsTblChgWidthHeightType::WH_COL_LEFT;    
    sal_uInt16 nTblChgSize = 0;
    bool bStopKeyInputTimer = true;
    OUString sFmlEntry;

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
            eKeyState = KS_KeyToView;       

#if OSL_DEBUG_LEVEL > 1

            
            if( 0x7210 == rKeyCode.GetFullCode() )
                rSh.SetReadOnlyAvailable( !rSh.IsReadOnlyAvailable() );
            else

#endif

            if( !rKeyCode.IsMod2() && '=' == aCh &&
                !rSh.IsTableMode() && rSh.GetTableFmt() &&
                rSh.IsSttPara() &&
                !rSh.HasReadonlySel() )
            {
                
                
                rSh.Push();
                if( !rSh.MoveSection( fnSectionCurr, fnSectionStart) &&
                    !rSh.IsTableBoxTextFormat() )
                {
                    
                    eKeyState = KS_EditFormula;
                    if( rSh.HasMark() )
                        rSh.SwapPam();
                    else
                        rSh.SttSelect();
                    rSh.MoveSection( fnSectionCurr, fnSectionEnd );
                    rSh.Pop( sal_True );
                    rSh.EndSelect();
                    sFmlEntry = "=";
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

                
                case KEY_RIGHT | KEY_SHIFT:
                case KEY_LEFT | KEY_SHIFT:
                case KEY_UP | KEY_SHIFT:
                case KEY_DOWN | KEY_SHIFT:
                {
                    const int nSelectionType = rSh.GetSelectionType();
                    if ( ( pFlyFmt
                           && ( nSelectionType & (nsSelectionType::SEL_FRM|nsSelectionType::SEL_OLE|nsSelectionType::SEL_GRF) ) )
                         || ( ( nSelectionType & (nsSelectionType::SEL_DRW|nsSelectionType::SEL_DRW_FORM) )
                              && rSh.GetDrawView()->AreObjectsMarked() ) )
                    {
                        eKeyState = pFlyFmt ? KS_Fly_Change : KS_Draw_Change;
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
                case KEY_UP | KEY_MOD1:
                {
                    bool bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
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
                case KEY_DOWN | KEY_MOD1:
                {
                    bool bMod1 = 0 != (rKeyCode.GetModifier() & KEY_MOD1);
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
                        !m_bTblInsDelMode ||
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
                        if( !m_bTblIsInsMode )
                            eTblChgMode = eTblChgMode | nsTblChgWidthHeightType::WH_FLAG_BIGGER;
                        eKeyState = KS_TblColCellInsDel;
                    }
                    break;

                case KEY_DELETE:
                    if ( !rSh.HasReadonlySel() )
                    {
                        if (rSh.IsInFrontOfLabel() && rSh.NumOrNoNum(sal_False))
                            eKeyState = KS_NumOrNoNum;
                    }
                    else
                    {
                        MessageDialog(this, "InfoReadonlyDialog",
                            "modules/swriter/ui/inforeadonlydialog.ui").Execute();
                        eKeyState = KS_End;
                    }
                    break;

                case KEY_DELETE | KEY_MOD2:
                    if( !rSh.IsTableMode() && rSh.GetTableFmt() )
                    {
                        eKeyState = KS_End;
                        m_bTblInsDelMode = sal_True;
                        m_bTblIsInsMode = sal_False;
                        m_aKeyInputTimer.Start();
                        bStopKeyInputTimer = false;
                    }
                    break;
                case KEY_INSERT | KEY_MOD2:
                    if( !rSh.IsTableMode() && rSh.GetTableFmt() )
                    {
                        eKeyState = KS_End;
                        m_bTblInsDelMode = sal_True;
                        m_bTblIsInsMode = sal_True;
                        m_aKeyInputTimer.Start();
                        bStopKeyInputTimer = false;
                    }
                    break;

                case KEY_RETURN:
                {
                    if ( !rSh.HasReadonlySel()
                         && !rSh.CrsrInsideInputFld() )
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

                        
                        else if( m_aInBuffer.isEmpty() &&
                                 rSh.GetCurNumRule() &&
                                 !rSh.GetCurNumRule()->IsOutlineRule() &&
                                 !rSh.HasSelection() &&
                                rSh.IsSttPara() && rSh.IsEndPara() )
                            eKeyState = KS_NumOff, eNextKeyState = KS_OutlineLvOff;

                        
                        else if( pACfg && pACfg->IsAutoFmtByInput() &&
                                !(nSelectionType & (nsSelectionType::SEL_GRF |
                                    nsSelectionType::SEL_OLE | nsSelectionType::SEL_FRM |
                                    nsSelectionType::SEL_TBL_CELLS | nsSelectionType::SEL_DRW |
                                    nsSelectionType::SEL_DRW_TXT)) )
                            eKeyState = KS_CheckAutoCorrect, eNextKeyState = KS_AutoFmtByInput;
                        else
                            eNextKeyState = eKeyState, eKeyState = KS_CheckAutoCorrect;
                    }
                }
                break;
                case KEY_RETURN | KEY_MOD2:
                {
                    if ( !rSh.HasReadonlySel()
                         && !rSh.IsSttPara()
                         && rSh.GetCurNumRule()
                         && !rSh.CrsrInsideInputFld() )
                    {
                        eKeyState = KS_NoNum;
                    }
                    else if( rSh.CanSpecialInsert() )
                        eKeyState = KS_SpecialInsert;
                }
                break;
                case KEY_BACKSPACE:
                case KEY_BACKSPACE | KEY_SHIFT:
                    if( !rSh.HasReadonlySel() )
                    {
                        bool bDone = false;
                        
                        
                        
                        
                        
                        
                        
                        const bool bOnlyBackspaceKey(
                                    KEY_BACKSPACE == rKeyCode.GetFullCode() );
                        if ( rSh.IsSttPara() &&
                             !rSh.HasSelection() &&
                             ( NULL == rSh.GetCurNumRule() ||
                               ( rSh.IsNoNum() && bOnlyBackspaceKey ) ) )
                        {
                            bDone = rSh.TryRemoveIndent();
                        }

                        if (bDone)
                            eKeyState = KS_End;
                        else
                        {
                            if (rSh.IsSttPara() &&
                                ! rSh.IsNoNum())
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
                                 rSh.NumOrNoNum( !bOnlyBackspaceKey, true ) )
                            {
                                eKeyState = KS_NumOrNoNum;
                            }
                        }
                    }
                    else
                    {
                        MessageDialog(this, "InfoReadonlyDialog",
                            "modules/swriter/ui/inforeadonlydialog.ui").Execute();
                        eKeyState = KS_End;
                    }
                    break;

                case KEY_RIGHT:
                    {
                        eFlyState = KS_Fly_Change;
                        nDir = MOVE_RIGHT_BIG;
                        eTblChgMode = nsTblChgWidthHeightType::WH_FLAG_INSDEL | nsTblChgWidthHeightType::WH_COL_RIGHT;
                        nTblChgSize = pModOpt->GetTblVInsert();
                        goto KEYINPUT_CHECKTABLE_INSDEL;
                    }
                case KEY_TAB:
                {

                    if (rSh.IsFormProtected() || rSh.GetCurrentFieldmark() || rSh.GetChar(sal_False)==CH_TXT_ATR_FORMELEMENT)
                    {
                        eKeyState=KS_GotoNextFieldMark;
                    }
                    else if ( !rSh.IsMultiSelection() && rSh.CrsrInsideInputFld() )
                    {
                        GetView().GetViewFrame()->GetDispatcher()->Execute( FN_GOTO_NEXT_INPUTFLD );
                        eKeyState = KS_End;
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
                    if (rSh.IsFormProtected() || rSh.GetCurrentFieldmark()|| rSh.GetChar(sal_False)==CH_TXT_ATR_FORMELEMENT)
                    {
                        eKeyState=KS_GotoPrevFieldMark;
                    }
                    else if ( !rSh.IsMultiSelection() && rSh.CrsrInsideInputFld() )
                    {
                        GetView().GetViewFrame()->GetDispatcher()->Execute( FN_GOTO_PREV_INPUTFLD );
                        eKeyState = KS_End;
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
                    }
                    break;

                    case KEY_TAB | KEY_MOD1 | KEY_SHIFT:
                    {
                        if( aTmpQHD.HasCntnt() && !rSh.HasSelection() &&
                            !rSh.HasReadonlySel() )
                        {
                            
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
                        bNormalChar = false;
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
                        else if ( !rSh.IsMultiSelection() && rSh.CrsrInsideInputFld() )
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
                            if(SFX_ITEM_SET == aSet.GetItemState(RES_TXTATR_INETFMT, false))
                            {
                                const SfxPoolItem& rItem = aSet.Get(RES_TXTATR_INETFMT, sal_True);
                                bNormalChar = false;
                                eKeyState = KS_End;
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

                if( !m_aInBuffer.isEmpty() && ( !bNormalChar || bIsDocReadOnly ))
                    FlushInBuffer();

                if( m_rView.KeyInput( aKeyEvent ) )
                    bFlushBuffer = true, bNormalChar = false;
                else
                {
                    
                    
                    
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
                                if ( m_rView.GetCurShell()->ISA(SwDrawTextShell) )
                                    ((SwDrawTextShell*)m_rView.GetCurShell())->Init();
                                rSh.GetDrawView()->KeyInput( rKEvt, this );
                            }
                        }
                        else if(nSelectionType & nsSelectionType::SEL_FRM)
                        {
                            rSh.UnSelectFrm();
                            rSh.LeaveSelFrmMode();
                            m_rView.AttrChangedNotify(&rSh);
                            rSh.MoveSection( fnSectionCurr, fnSectionEnd );
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
            rSh.UnSelectFrm();
            rSh.LeaveSelFrmMode();
            m_rView.AttrChangedNotify(&rSh);
            rSh.MoveSection( fnSectionCurr, fnSectionEnd );
            eKeyState = KS_End;
        }
        break;
        case KS_GoIntoDrawing:
        {
            SdrObject* pObj = rSh.GetDrawView()->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            if(pObj)
            {
                EnterDrawTextMode(pObj->GetLogicRect().Center());
                if ( m_rView.GetCurShell()->ISA(SwDrawTextShell) )
                    ((SwDrawTextShell*)m_rView.GetCurShell())->Init();
            }
            eKeyState = KS_End;
        }
        break;
        case KS_EnterDrawHandleMode:
        {
            const SdrHdlList& rHdlList = rSh.GetDrawView()->GetHdlList();
            sal_Bool bForward(!aKeyEvent.GetKeyCode().IsShift());

            ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);
            eKeyState = KS_End;
        }
        break;
        case KS_InsTab:
            if( m_rView.ISA( SwWebView ))     
            {
                
                Window::KeyInput( aKeyEvent );
                eKeyState = KS_End;
                break;
            }
            aCh = '\t';
            
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
                    OSL_ENSURE(pFieldmark->IsExpanded(),
                        "where is the otherpos?");
                    if (pFieldmark->IsExpanded())
                    {
                        rSh.CalcLayout();
                    }
                }
                eKeyState = KS_End;
            }
            else if(!rSh.HasReadonlySel())
            {
                sal_Bool bIsNormalChar = GetAppCharClass().isLetterNumeric(
                                                            OUString( aCh ), 0 );
                if( bAppendSpace && bIsNormalChar &&
                    (!m_aInBuffer.isEmpty() || !rSh.IsSttPara() || !rSh.IsEndPara() ))
                {
                    
                    
                    m_aInBuffer += " ";
                }

                sal_Bool bIsAutoCorrectChar =  SvxAutoCorrect::IsAutoCorrectChar( aCh );
                bool bRunNext = pACorr && pACorr->HasRunNext();
                if( !aKeyEvent.GetRepeat() && pACorr && ( bIsAutoCorrectChar || bRunNext ) &&
                        pACfg->IsAutoFmtByInput() &&
                    (( pACorr->IsAutoCorrFlag( ChgWeightUnderl ) &&
                        ( '*' == aCh || '_' == aCh ) ) ||
                     ( pACorr->IsAutoCorrFlag( ChgQuotes ) && ('\"' == aCh ))||
                     ( pACorr->IsAutoCorrFlag( ChgSglQuotes ) && ( '\'' == aCh))))
                {
                    FlushInBuffer();
                    rSh.AutoCorrect( *pACorr, aCh );
                    if( '\"' != aCh && '\'' != aCh )        
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
                    OUStringBuffer aBuf(m_aInBuffer);
                    comphelper::string::padToLength(aBuf,
                        m_aInBuffer.getLength() + aKeyEvent.GetRepeat() + 1, aCh);
                    m_aInBuffer = aBuf.makeStringAndClear();
                    bFlushCharBuffer = Application::AnyInput( VCL_INPUT_KEYBOARD );
                    bFlushBuffer = !bFlushCharBuffer;
                    if( bFlushCharBuffer )
                        m_aKeyInputFlushTimer.Start();
                }
                eKeyState = KS_End;
            }
            else
            {
                MessageDialog(this, "InfoReadonlyDialog",
                    "modules/swriter/ui/inforeadonlydialog.ui").Execute();
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
                
                rSh.DelNumRules();
                eKeyState = eNextKeyState;
                break;
            case KS_OutlineLvOff: 
                break;

            case KS_NumDown:
                rSh.NumUpDown( true );
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
                rSh.OutlineUpDown( 1 );
                break;
            case KS_OutlineUp:
                rSh.OutlineUpDown( -1 );
                break;

            case KS_NextCell:
                
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
                        m_rView.GetDrawFuncPtr() )
                    {
                        m_rView.GetDrawFuncPtr()->Deactivate();
                        m_rView.SetDrawFuncPtr(NULL);
                        m_rView.LeaveDrawCreate();
                        m_rView.AttrChangedNotify( &rSh );
                    }
                    rSh.HideCrsr();
                    rSh.EnterSelFrmMode();
                }
            break;
            case KS_GlossaryExpand:
            {
                
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
                        rSh.ExtendSelection( sal_False, aTmpQHD.nLen );
                        SwGlossaryHdl* pGlosHdl = GetView().GetGlosHdl();
                        pGlosHdl->SetCurGroup(sGroup, sal_True);
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
                    ChangeFly( nDir, m_rView.ISA( SwWebView ) );
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
        m_bTblInsDelMode = sal_False;
    }

    
    if( bFlushBuffer && !m_aInBuffer.isEmpty() )
    {
        
        
        bool bSave = bFlushCharBuffer;
        FlushInBuffer();
        bFlushCharBuffer = bSave;

        
        OUString sWord;
        if( bNormalChar && pACfg && pACorr &&
            ( pACfg->IsAutoTextTip() ||
              pACorr->GetSwFlags().bAutoCompleteWords ) &&
            rSh.GetPrevAutoCorrWord( *pACorr, sWord ) )
        {
            ShowAutoTextCorrectQuickHelp(sWord, pACfg, pACorr);
        }
    }

    
    SwWordCountWrapper *pWrdCnt = (SwWordCountWrapper*)GetView().GetViewFrame()->GetChildWindow(SwWordCountWrapper::GetChildWindowId());
    if( pWrdCnt )
        pWrdCnt->UpdateCounts();

}

/**
 * MouseEvents
 */
void SwEditWin::RstMBDownFlags()
{
    
    
    
    
    m_bMBPressed = sal_False;
    bNoInterrupt = false;
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

#if !HAVE_FEATURE_DESKTOP






void touch_lo_selection_start_move_impl(const void *documentHandle,
                                        int x,
                                        int y)
{
    SwWrtShell *pWrtShell = reinterpret_cast<SwWrtShell*>(const_cast<void*>(documentHandle));

    if (!pWrtShell)
        return;

    const OutputDevice *pOut = pWrtShell->GetWin();
    if (!pOut)
        pOut = pWrtShell->GetOut();

    const Point aDocPos( pOut->PixelToLogic( Point(x, y) ) );

    
    

    SwRect startCharRect;
    pWrtShell->GetCharRectAt(startCharRect, pWrtShell->GetCrsr()->GetPoint());
    const Point startCharPos = startCharRect.Center();

    if (startCharPos.Y() < aDocPos.Y() ||
        (startCharPos.Y() == aDocPos.Y() && startCharPos.X() - startCharRect.Width() <= aDocPos.X()))
        return;

    pWrtShell->ChgCurrPam( aDocPos );

    
    
    pWrtShell->GetCrsr()->Exchange();
    {
        SwMvContext aMvContext( pWrtShell );
        pWrtShell->SwCrsrShell::SetCrsr( aDocPos );
    }
    pWrtShell->GetCrsr()->Exchange();
}

void touch_lo_selection_end_move_impl(const void *documentHandle,
                                      int x,
                                      int y)
{
    SwWrtShell *pWrtShell = reinterpret_cast<SwWrtShell*>(const_cast<void*>(documentHandle));

    if (!pWrtShell)
        return;

    const OutputDevice *pOut = pWrtShell->GetWin();
    if (!pOut)
        pOut = pWrtShell->GetOut();

    const Point aDocPos( pOut->PixelToLogic( Point(x, y) ) );

    
    

    SwRect endCharRect;
    pWrtShell->GetCharRectAt(endCharRect, pWrtShell->GetCrsr()->GetMark());
    const Point endCharPos = endCharRect.Center();

    if (endCharPos.Y() > aDocPos.Y() ||
        (endCharPos.Y() == aDocPos.Y() && endCharPos.X() + endCharRect.Width() >= aDocPos.X()))
        return;

    pWrtShell->ChgCurrPam( aDocPos );

    {
        SwMvContext aMvContext( pWrtShell );
        pWrtShell->SwCrsrShell::SetCrsr( aDocPos );
    }
}

#endif

void SwEditWin::MouseButtonDown(const MouseEvent& _rMEvt)
{
    SwWrtShell &rSh = m_rView.GetWrtShell();

    
    
    
    
    
    
    SfxInPlaceClient* pIPClient = rSh.GetSfxViewShell()->GetIPClient();
    bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );

    if ( bIsOleActive && PopupMenu::IsInExecute() )
        return;

    MouseEvent rMEvt(_rMEvt);

    if (m_rView.GetPostItMgr()->IsHit(rMEvt.GetPosPixel()))
        return;

    m_rView.GetPostItMgr()->SetActiveSidebarWin(0);

    GrabFocus();

    
    {
        bool bExecFormatPaintbrush = m_pApplyTempl && m_pApplyTempl->m_pFormatClipboard
                                &&  m_pApplyTempl->m_pFormatClipboard->HasContent();
        if( bExecFormatPaintbrush )
            rMEvt = MouseEvent( _rMEvt.GetPosPixel(), _rMEvt.GetClicks(),
                                    _rMEvt.GetMode(), _rMEvt.GetButtons() );
    }

    m_bWasShdwCrsr = 0 != m_pShadCrsr;
    delete m_pShadCrsr, m_pShadCrsr = 0;

    const Point aDocPos( PixelToLogic( rMEvt.GetPosPixel() ) );

    
    FrameControlType eControl;
    bool bOverFly = false;
    bool bPageAnchored = false;
    bool bOverHeaderFooterFly = IsOverHeaderFooterFly( aDocPos, eControl, bOverFly, bPageAnchored );

    sal_Bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly();
    if (bOverHeaderFooterFly && (!bIsDocReadOnly && rSh.GetCurFld()))
        
        bOverHeaderFooterFly = false;

    int nNbFlyClicks = 1;
    
    if ( !bPageAnchored && ( ( rSh.IsHeaderFooterEdit( ) && !bOverHeaderFooterFly && bOverFly ) ||
         ( !rSh.IsHeaderFooterEdit( ) && bOverHeaderFooterFly ) ) )
    {
        nNbFlyClicks = 2;
        if ( _rMEvt.GetClicks( ) < nNbFlyClicks )
            return;
    }

    
    if ( IsInHeaderFooter( aDocPos, eControl ) || bOverHeaderFooterFly )
    {
        const SwPageFrm* pPageFrm = rSh.GetLayout()->GetPageAtPos( aDocPos );

        
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

            
            
            
            //
            
            if ( !( bWasInHeader && eControl == Header ) &&
                 !( bWasInFooter && eControl == Footer ) )
            {
                rSh.SetShowHeaderFooterSeparator( eControl, !rSh.IsShowHeaderFooterSeparator( eControl ) );
            }

            
            Invalidate();
        }
        else
        {
            
            
            rSh.SetShowHeaderFooterSeparator( Header, eControl == Header );
            rSh.SetShowHeaderFooterSeparator( Footer, eControl == Footer );

            if ( !rSh.IsHeaderFooterEdit() )
                rSh.ToggleHeaderFooterEdit();

            
            rSh.GetWin()->Invalidate();
        }
    }
    else
    {
        if ( rSh.IsHeaderFooterEdit( ) )
            rSh.ToggleHeaderFooterEdit( );
        else
        {
            
            rSh.SetShowHeaderFooterSeparator( Header, false );
            rSh.SetShowHeaderFooterSeparator( Footer, false );

            
            
            

        }
    }


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

    
    
    m_rView.SelectShellForDrop();

    bool bCallBase = true;

    if( m_pQuickHlpData->m_bIsDisplayed )
        m_pQuickHlpData->Stop( rSh );
    m_pQuickHlpData->m_bAppendSpace = false;

    if( rSh.FinishOLEObj() )
        return; 

    SET_CURR_SHELL( &rSh );

    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        if (pSdrView->MouseButtonDown( rMEvt, this ) )
        {
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);
            return; 
        }
    }

    m_bIsInMove = sal_False;
    m_aStartPos = rMEvt.GetPosPixel();
    m_aRszMvHdlPt.X() = 0, m_aRszMvHdlPt.Y() = 0;

    sal_uInt8 nMouseTabCol = 0;
    const bool bTmp = !rSh.IsDrawCreate() && !m_pApplyTempl && !rSh.IsInSelect() &&
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
                m_pRowColumnSelectionStart = new Point( aDocPos );
                m_bIsRowDrag = SW_TABROWSEL_HORI == nMouseTabCol||
                            SW_TABROWSEL_HORI_RTL == nMouseTabCol ||
                            SW_TABCOLSEL_VERT == nMouseTabCol;
                m_bMBPressed = sal_True;
                CaptureMouse();
            }
            return;
        }

        if ( !rSh.IsTableMode() )
        {
            
            if(SW_TABCOL_VERT == nMouseTabCol || SW_TABCOL_HORI == nMouseTabCol)
                m_rView.SetTabColFromDoc( sal_True );
            else
                m_rView.SetTabRowFromDoc( sal_True );

            m_rView.SetTabColFromDocPos( aDocPos );
            m_rView.InvalidateRulerPos();
            SfxBindings& rBind = m_rView.GetViewFrame()->GetBindings();
            rBind.Update();
            if ( RulerColumnDrag( rMEvt,
                    (SW_TABCOL_VERT == nMouseTabCol || SW_TABROW_HORI == nMouseTabCol)) )
            {
                m_rView.SetTabColFromDoc( sal_False );
                m_rView.SetTabRowFromDoc( sal_False );
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
        SwTxtNode* pNodeAtPos = rSh.GetNumRuleNodeAtPos( aDocPos );
        m_rView.SetNumRuleNodeFromDoc( pNodeAtPos );
        m_rView.InvalidateRulerPos();
        SfxBindings& rBind = m_rView.GetViewFrame()->GetBindings();
        rBind.Update();

        if ( RulerMarginDrag( rMEvt,
                        rSh.IsVerticalModeAtNdAndPos( *pNodeAtPos, aDocPos ) ) )
        {
            m_rView.SetNumRuleNodeFromDoc( NULL );
            m_rView.InvalidateRulerPos();
            rBind.Update();
            bCallBase = false;
        }
        else
        {
            
            
            m_rView.SetNumRuleNodeFromDoc( NULL );
            return;
        }
    }

    if ( rSh.IsInSelect() )
        rSh.EndSelect();

    
    
    if ( MOUSE_LEFT == rMEvt.GetButtons() )
    {
        sal_Bool bOnlyText = sal_False;
        m_bMBPressed = sal_True;
        bNoInterrupt = true;
        m_nKS_NUMDOWN_Count = 0;

        CaptureMouse();

        
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
                        0 != ( pHdl = pSdrView->PickHandle(aDocPos) ) &&
                            ( pHdl->GetKind() == HDL_ANCHOR ||
                              pHdl->GetKind() == HDL_ANCHOR_TR ) )
                    {
                        
                        pHdl->SetSelected(true);
                        m_pAnchorMarker = new SwAnchorMarker( pHdl );
                        UpdatePointer( aDocPos, rMEvt.GetModifier() );
                        return;
                    }
                }
                if ( EnterDrawMode( rMEvt, aDocPos ) )
                {
                    bNoInterrupt = false;
                    return;
                }
                else  if ( m_rView.GetDrawFuncPtr() && m_bInsFrm )
                {
                    StopInsFrm();
                    rSh.Edit();
                }

                
                if (rMEvt.GetClicks() == 1)
                {
                    if ( rSh.IsSelFrmMode())
                    {
                        SdrHdl* pHdl = rSh.GetDrawView()->PickHandle(aDocPos);
                        bool bHitHandle = pHdl && pHdl->GetKind() != HDL_ANCHOR &&
                                                  pHdl->GetKind() != HDL_ANCHOR_TR;

                        if ((rSh.IsInsideSelectedObj(aDocPos) || bHitHandle) &&
                            !(rMEvt.GetModifier() == KEY_SHIFT && !bHitHandle))
                        {
                            rSh.EnterSelFrmMode( &aDocPos );
                            if ( !m_pApplyTempl )
                            {
                                
                                if (!bHitHandle)
                                {
                                    SdrObject* pObj;
                                    SdrPageView* pPV;
                                    if (pSdrView->PickObj(aDocPos, pSdrView->getHitTolLog(), pObj, pPV, SDRSEARCH_ALSOONMASTER | SDRSEARCH_BEFOREMARK))
                                    {
                                        pSdrView->UnmarkAllObj();
                                        pSdrView->MarkObj(pObj,pPV,false,false);
                                        return;
                                    }
                                    StartDDTimer();
                                    SwEditWin::m_nDDStartPosY = aDocPos.Y();
                                    SwEditWin::m_nDDStartPosX = aDocPos.X();
                                }
                                bFrmDrag = true;
                            }
                            bNoInterrupt = false;
                            return;
                        }
                    }
                }
        }

        sal_Bool bExecHyperlinks = m_rView.GetDocShell()->IsReadOnly();
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

        bool bExecDrawTextLink = false;

        switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
        {
            case MOUSE_LEFT:
            case MOUSE_LEFT + KEY_MOD1:
            case MOUSE_LEFT + KEY_MOD2:
            {

                bool bHandledFlyClick = false;
                if ( nNumberOfClicks == nNbFlyClicks )
                {
                    bHandledFlyClick = true;
                    
                    
                    if ( m_aActHitType != SDRHIT_NONE && !rSh.IsSelFrmMode() &&
                        !GetView().GetViewFrame()->GetDispatcher()->IsLocked() &&
                        !bExecDrawTextLink)
                    {
                        
                        sal_Bool bShould = rSh.ShouldObjectBeSelected(aDocPos);

                        if(bShould)
                        {
                            m_rView.NoRotate();
                            rSh.HideCrsr();

                            bool bUnLockView = !rSh.IsViewLocked();
                            rSh.LockView( sal_True );
                            sal_Bool bSelObj = rSh.SelectObj( aDocPos,
                                           rMEvt.IsMod1() ? SW_ENTER_GROUP : 0);
                            if( bUnLockView )
                                rSh.LockView( sal_False );

                            if( bSelObj )
                            {
                                
                                
                                if( FRMTYPE_NONE == rSh.GetSelFrmType() )
                                    rSh.ShowCrsr();
                                else
                                {
                                    if (rSh.IsFrmSelected() && m_rView.GetDrawFuncPtr())
                                    {
                                        m_rView.GetDrawFuncPtr()->Deactivate();
                                        m_rView.SetDrawFuncPtr(NULL);
                                        m_rView.LeaveDrawCreate();
                                        m_rView.AttrChangedNotify( &rSh );
                                    }

                                    rSh.EnterSelFrmMode( &aDocPos );
                                    bFrmDrag = true;
                                    UpdatePointer( aDocPos, rMEvt.GetModifier() );
                                }
                                return;
                            }
                            else
                                bOnlyText = static_cast< sal_Bool >(rSh.IsObjSelectable( aDocPos ));

                            if (!m_rView.GetDrawFuncPtr())
                                rSh.ShowCrsr();
                        }
                        else
                            bOnlyText = KEY_MOD1 != rMEvt.GetModifier();
                    }
                    else if ( rSh.IsSelFrmMode() &&
                              (m_aActHitType == SDRHIT_NONE ||
                               !rSh.IsInsideSelectedObj( aDocPos )))
                    {
                        m_rView.NoRotate();
                        SdrHdl *pHdl;
                        if( !bIsDocReadOnly && !m_pAnchorMarker && 0 !=
                            ( pHdl = pSdrView->PickHandle(aDocPos) ) &&
                                ( pHdl->GetKind() == HDL_ANCHOR ||
                                  pHdl->GetKind() == HDL_ANCHOR_TR ) )
                        {
                            m_pAnchorMarker = new SwAnchorMarker( pHdl );
                            UpdatePointer( aDocPos, rMEvt.GetModifier() );
                            return;
                        }
                        else
                        {
                            bool bUnLockView = !rSh.IsViewLocked();
                            rSh.LockView( sal_True );
                            sal_uInt8 nFlag = rMEvt.IsShift() ? SW_ADD_SELECT :0;
                            if( rMEvt.IsMod1() )
                                nFlag = nFlag | SW_ENTER_GROUP;

                            if ( rSh.IsSelFrmMode() )
                            {
                                rSh.UnSelectFrm();
                                rSh.LeaveSelFrmMode();
                                m_rView.AttrChangedNotify(&rSh);
                            }

                            sal_Bool bSelObj = rSh.SelectObj( aDocPos, nFlag );
                            if( bUnLockView )
                                rSh.LockView( sal_False );

                            if( !bSelObj )
                            {
                                
                                
                                bValidCrsrPos = !(CRSR_POSCHG & rSh.SetCursor(&aDocPos, false));
                                rSh.LeaveSelFrmMode();
                                m_rView.AttrChangedNotify( &rSh );
                                bCallBase = false;
                            }
                            else
                            {
                                rSh.HideCrsr();
                                rSh.EnterSelFrmMode( &aDocPos );
                                rSh.SelFlyGrabCrsr();
                                rSh.MakeSelVisible();
                                bFrmDrag = true;
                                if( rSh.IsFrmSelected() &&
                                    m_rView.GetDrawFuncPtr() )
                                {
                                    m_rView.GetDrawFuncPtr()->Deactivate();
                                    m_rView.SetDrawFuncPtr(NULL);
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
                    {
                        UpdatePointer( aDocPos, rMEvt.GetModifier() );
                        SwEditWin::m_nDDStartPosY = aDocPos.Y();
                        SwEditWin::m_nDDStartPosX = aDocPos.X();

                        
                        if (bExecHyperlinks && pSdrView)
                        {
                            SdrViewEvent aVEvt;
                            pSdrView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                            if (aVEvt.eEvent == SDREVENT_EXECUTEURL)
                                bExecDrawTextLink = true;
                        }
                        break;
                    }
                    case 2:
                    {
                        bFrmDrag = false;
                        if ( !bHandledFlyClick && !bIsDocReadOnly && rSh.IsInsideSelectedObj(aDocPos) &&
                             0 == rSh.IsSelObjProtected( FLYPROTECT_CONTENT|FLYPROTECT_PARENT ) )
                        {
                        /* This is no good: on the one hand GetSelectionType is used as flag field
                         * (take a look into the GetSelectionType method) and on the other hand the
                         * return value is used in a switch without proper masking (very nice), this must lead to trouble
                         */
                            switch ( rSh.GetSelectionType() &~ ( nsSelectionType::SEL_FONTWORK | nsSelectionType::SEL_EXTRUDED_CUSTOMSHAPE ) )
                            {
                            case nsSelectionType::SEL_GRF:
                                RstMBDownFlags();
                                GetView().GetViewFrame()->GetBindings().Execute(
                                    FN_FORMAT_GRAFIC_DLG, 0, 0,
                                    SFX_CALLMODE_RECORD|SFX_CALLMODE_SLOT);
                                return;

                            
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
                                if ( m_rView.GetCurShell()->ISA(SwDrawTextShell) )
                                    ((SwDrawTextShell*)m_rView.GetCurShell())->Init();
                                return;
                            }
                        }

                        
                        
                        if ( !bValidCrsrPos || rSh.IsFrmSelected() )
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
                        
                        
                        if ( rSh.IsExtMode() || rSh.IsBlockMode() )
                            return;

                        
                        if ( KEY_MOD1 == rMEvt.GetModifier() && !rSh.IsAddMode() )
                        {
                            rSh.EnterAddMode();
                            rSh.SelWrd( &aDocPos );
                            rSh.LeaveAddMode();
                        }
                        else
                            rSh.SelWrd( &aDocPos );
                        bHoldSelection = true;
                        return;
                    }
                    case 3:
                    case 4:
                    {
                        bFrmDrag = false;
                        
                        
                        if ( rSh.IsExtMode() )
                            return;

                        
                        
                        if ( !bValidCrsrPos || rSh.IsFrmSelected() )
                            return;

                        
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

                        bHoldSelection = true;
                        return;
                    }

                    default:
                        return;
                }
            }
            /* no break */
            case MOUSE_LEFT + KEY_SHIFT:
            case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            {
                sal_Bool bLockView = m_bWasShdwCrsr;

                switch ( rMEvt.GetModifier() )
                {
                    case KEY_MOD1 + KEY_SHIFT:
                    {
                        if ( !m_bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) )
                        {
                            m_rView.NoRotate();
                            rSh.HideCrsr();
                            if ( rSh.IsSelFrmMode() )
                                rSh.SelectObj(aDocPos, SW_ADD_SELECT | SW_ENTER_GROUP);
                            else
                            {   if ( rSh.SelectObj( aDocPos, SW_ADD_SELECT | SW_ENTER_GROUP ) )
                                {
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    SwEditWin::m_nDDStartPosY = aDocPos.Y();
                                    SwEditWin::m_nDDStartPosX = aDocPos.X();
                                    bFrmDrag = true;
                                    return;
                                }
                            }
                        }
                        else if( rSh.IsSelFrmMode() &&
                                 rSh.GetDrawView()->PickHandle( aDocPos ))
                        {
                            bFrmDrag = true;
                            bNoInterrupt = false;
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
                            rSh.HideCrsr();
                            if ( rSh.IsSelFrmMode() )
                                rSh.SelectObj(aDocPos, SW_ENTER_GROUP);
                            else
                            {   if ( rSh.SelectObj( aDocPos, SW_ENTER_GROUP ) )
                                {
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    SwEditWin::m_nDDStartPosY = aDocPos.Y();
                                    SwEditWin::m_nDDStartPosX = aDocPos.X();
                                    bFrmDrag = true;
                                    return;
                                }
                            }
                        }
                        else if( rSh.IsSelFrmMode() &&
                                 rSh.GetDrawView()->PickHandle( aDocPos ))
                        {
                            bFrmDrag = true;
                            bNoInterrupt = false;
                            return;
                        }
                        else
                        {
                            if ( !rSh.IsAddMode() && !rSh.IsExtMode() && !rSh.IsBlockMode() )
                            {
                                rSh.PushMode();
                                bModePushed = true;

                                bool bUnLockView = !rSh.IsViewLocked();
                                rSh.LockView( sal_True );
                                rSh.EnterAddMode();
                                if( bUnLockView )
                                    rSh.LockView( sal_False );
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
                            bModePushed = true;
                            bool bUnLockView = !rSh.IsViewLocked();
                            rSh.LockView( sal_True );
                            rSh.EnterBlockMode();
                            if( bUnLockView )
                                rSh.LockView( sal_False );
                        }
                        bCallBase = false;
                    }
                    break;
                    case KEY_SHIFT:
                    {
                        if ( !m_bInsDraw && IsDrawObjSelectable( rSh, aDocPos ) )
                        {
                            m_rView.NoRotate();
                            rSh.HideCrsr();
                            if ( rSh.IsSelFrmMode() )
                            {
                                rSh.SelectObj(aDocPos, SW_ADD_SELECT);

                                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                                if (rMarkList.GetMark(0) == NULL)
                                {
                                    rSh.LeaveSelFrmMode();
                                    m_rView.AttrChangedNotify(&rSh);
                                    bFrmDrag = false;
                                }
                            }
                            else
                            {   if ( rSh.SelectObj( aDocPos ) )
                                {
                                    rSh.EnterSelFrmMode( &aDocPos );
                                    SwEditWin::m_nDDStartPosY = aDocPos.Y();
                                    SwEditWin::m_nDDStartPosX = aDocPos.X();
                                    bFrmDrag = true;
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
                                SwEditWin::m_nDDStartPosY = aDocPos.Y();
                                SwEditWin::m_nDDStartPosX = aDocPos.X();
                                bFrmDrag = true;
                                return;
                            }
                            if ( rSh.IsSelFrmMode() )
                            {
                                rSh.UnSelectFrm();
                                rSh.LeaveSelFrmMode();
                                m_rView.AttrChangedNotify(&rSh);
                                bFrmDrag = false;
                            }
                            if ( !rSh.IsExtMode() )
                            {
                                
                                
                                bool bSttSelect = rSh.HasSelection() ||
                                                Pointer(POINTER_REFHAND) != GetPointer();

                                if( !bSttSelect )
                                {
                                    bSttSelect = true;
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

                SwContentAtPos aFieldAtPos(SwContentAtPos::SW_FIELD);

                
                if (rSh.GetContentAtPos(aDocPos, aFieldAtPos))
                {
                    bool bEditableField = (aFieldAtPos.pFndTxtAttr != NULL
                        && aFieldAtPos.pFndTxtAttr->Which() == RES_TXTATR_INPUTFIELD);

                    if (!bEditableField)
                    {
                        rSh.SetCursor(&aDocPos, bOnlyText);
                        
                        
                        
                        SwTxtAttr const*const pTxtFld(aFieldAtPos.pFndTxtAttr);
                        if (rSh.GetCurrentShellCursor().GetPoint()->nContent
                                .GetIndex() != *pTxtFld->GetStart())
                        {
                            assert(rSh.GetCurrentShellCursor().GetPoint()->nContent
                                    .GetIndex() == (*pTxtFld->GetStart() + 1));
                            rSh.Left( CRSR_SKIP_CHARS, false, 1, false );
                        }
                        
                        
                        break;
                    }
                }

                sal_Bool bOverSelect = rSh.ChgCurrPam( aDocPos ), bOverURLGrf = sal_False;
                if( !bOverSelect )
                    bOverURLGrf = bOverSelect = 0 != rSh.IsURLGrfAtPos( aDocPos );

                if ( !bOverSelect )
                {
                    const bool bTmpNoInterrupt = bNoInterrupt;
                    bNoInterrupt = false;

                    if( !rSh.IsViewLocked() && bLockView )
                        rSh.LockView( sal_True );
                    else
                        bLockView = sal_False;

                    int nTmpSetCrsr = 0;

                    {   
                        
                        SwMvContext aMvContext( &rSh );
                        nTmpSetCrsr = rSh.SetCursor(&aDocPos, bOnlyText);
                        bValidCrsrPos = !(CRSR_POSCHG & nTmpSetCrsr);
                        bCallBase = false;
                    }

                    
                    if ( !(CRSR_POSOLD & nTmpSetCrsr) )
                        SetUseInputLanguage( sal_False );

                    if( bLockView )
                        rSh.LockView( sal_False );

                    bNoInterrupt = bTmpNoInterrupt;
                }
                if ( !bOverURLGrf && !bOnlyText )
                {
                    const int nSelType = rSh.GetSelectionType();
                    
                    
                    if ( ( nSelType & nsSelectionType::SEL_OLE ||
                         nSelType & nsSelectionType::SEL_GRF ||
                         rSh.IsObjSelectable( aDocPos ) ) && !lcl_urlOverBackground( rSh, aDocPos ) )
                    {
                        SwMvContext aMvContext( &rSh );
                        if( !rSh.IsFrmSelected() )
                            rSh.GotoNextFly();
                        rSh.EnterSelFrmMode();
                        bCallBase = false;
                    }
                }
                
                
                
                break;
            }
        }
    }
    if (bCallBase)
        Window::MouseButtonDown(rMEvt);
}

/**
 * MouseMove
 */
void SwEditWin::MouseMove(const MouseEvent& _rMEvt)
{
    MouseEvent rMEvt(_rMEvt);

    
    {
        bool bExecFormatPaintbrush = m_pApplyTempl && m_pApplyTempl->m_pFormatClipboard
                                &&  m_pApplyTempl->m_pFormatClipboard->HasContent();
        if( bExecFormatPaintbrush )
            rMEvt = MouseEvent( _rMEvt.GetPosPixel(), _rMEvt.GetClicks(),
                                    _rMEvt.GetMode(), _rMEvt.GetButtons() );
    }

    
    
    SwWrtShell &rSh = m_rView.GetWrtShell();
    if( rSh.ActionPend() )
        return ;

    if( m_pShadCrsr && 0 != (rMEvt.GetModifier() + rMEvt.GetButtons() ) )
        delete m_pShadCrsr, m_pShadCrsr = 0;

    sal_Bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly();

    SET_CURR_SHELL( &rSh );

    
    
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
        SetPointer( POINTER_TEXT );
        return; 
    }

    const Point aOldPt( rSh.VisArea().Pos() );
    const sal_Bool bInsWin = rSh.VisArea().IsInside( aDocPt );

    if( m_pShadCrsr && !bInsWin )
        delete m_pShadCrsr, m_pShadCrsr = 0;

    if( bInsWin && m_pRowColumnSelectionStart )
    {
        EnterArea();
        Point aPos( aDocPt );
        if( rSh.SelectTableRowCol( *m_pRowColumnSelectionStart, &aPos, m_bIsRowDrag ))
            return;
    }

    
    
    if( bDDTimerStarted )
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
            const SfxStringItem aCell( SID_TABLE_CELL, OUString() );
            rBnd.SetState( aCell );
        }
    }

    sal_uInt8 nMouseTabCol;
    if( !bIsDocReadOnly && bInsWin && !m_pApplyTempl && !rSh.IsInSelect() )
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
                
                ( !bChkTblSel || !rSh.IsTableMode() ) )
            {
                SetPointer( nPointer );
            }

            return;
        }
        else if (rSh.IsNumLabel(aDocPt, RULER_MOUSE_MARGINWIDTH))
        {
            
            SwTxtNode* pNodeAtPos = rSh.GetNumRuleNodeAtPos( aDocPt );
            const sal_uInt16 nPointer =
                    rSh.IsVerticalModeAtNdAndPos( *pNodeAtPos, aDocPt )
                    ? POINTER_VSIZEBAR
                    : POINTER_HSIZEBAR;
            SetPointer( nPointer );

            return;
        }
    }

    bool bDelShadCrsr = true;

    switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
    {
        case MOUSE_LEFT:
            if( m_pAnchorMarker )
            {
                
                
                
                
                
                
                
                
                

                const Point aOld = m_pAnchorMarker->GetPosForHitTest( *(rSh.GetOut()) );
                Point aNew = rSh.FindAnchorPos( aDocPt );
                SdrHdl* pHdl;
                if( (0!=( pHdl = pSdrView->PickHandle( aOld ) )||
                    0 !=(pHdl = pSdrView->PickHandle( m_pAnchorMarker->GetHdlPos()) ) ) &&
                        ( pHdl->GetKind() == HDL_ANCHOR ||
                          pHdl->GetKind() == HDL_ANCHOR_TR ) )
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
                    m_pAnchorMarker = NULL;
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
                    m_bIsInMove = sal_True;
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
            if ( !m_bMBPressed )
                break;
        case MOUSE_LEFT + KEY_MOD1:
            if ( bFrmDrag && rSh.IsSelFrmMode() )
            {
                if( !m_bMBPressed )
                    break;

                if ( m_bIsInMove || IsMinMove( m_aStartPos, aPixPt ) )
                {
                    
                    if( pSdrView->AreObjectsMarked() )
                    {
                        const SwFrmFmt* pFlyFmt;
                        const SvxMacro* pMacro;

                        const Point aSttPt( PixelToLogic( m_aStartPos ) );

                        
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
                        
                            m_aRszMvHdlPt != aDocPt )
                        {
                            m_aRszMvHdlPt = aDocPt;
                            sal_uInt16 nPos = 0;
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

                            OUString sRet;

                            ReleaseMouse();

                            rSh.ExecMacro( *pMacro, &sRet, &xArgs );

                            CaptureMouse();

                            if( !sRet.isEmpty() && sRet.toInt32()!=0 )
                                return ;
                        }
                    }
                    

                    if( bIsDocReadOnly )
                        break;

                    bool bIsSelectionGfx = rSh.GetSelectionType() & nsSelectionType::SEL_GRF;
                    bool bisResize = eSdrMoveHdl != HDL_MOVE;

                    if ( rMEvt.IsShift() )
                    {
                        pSdrView->SetAngleSnapEnabled(!bIsSelectionGfx);
                        if (bisResize)
                            pSdrView->SetOrtho(!bIsSelectionGfx);
                        else
                            pSdrView->SetOrtho(true);
                    }
                    else
                    {
                        pSdrView->SetAngleSnapEnabled(bIsSelectionGfx);
                        if (bisResize)
                            pSdrView->SetOrtho(bIsSelectionGfx);
                        else
                            pSdrView->SetOrtho(false);
                    }

                    rSh.Drag( &aDocPt, rMEvt.IsShift() );
                    m_bIsInMove = sal_True;
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
            if ( !rSh.IsSelFrmMode() && !bDDINetAttr &&
                (IsMinMove( m_aStartPos,aPixPt ) || m_bIsInMove) &&
                (rSh.IsInSelect() || !rSh.ChgCurrPam( aDocPt )) )
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

                        bValidCrsrPos = !(CRSR_POSCHG & rSh.SetCursor(&aDocPt, false));
                        EnterArea();
                    }
                }
            }
            bDDINetAttr = false;
            break;
        case 0:
        {
            if ( m_pApplyTempl )
            {
                UpdatePointer(aDocPt, 0); 
                break;
            }
            
            
            SwRect aFldRect;
            SwContentAtPos aCntntAtPos( SwContentAtPos::SW_FIELD);
            if( rSh.GetContentAtPos( aDocPt, aCntntAtPos, sal_False, &aFldRect ) )
            {
                const SwField* pFld = aCntntAtPos.aFnd.pFld;
                if (pFld->Which()== RES_POSTITFLD)
                {
                    m_rView.GetPostItMgr()->SetShadowState(reinterpret_cast<const SwPostItField*>(pFld),false);
                }
                else
                    m_rView.GetPostItMgr()->SetShadowState(0,false);
            }
            else
                m_rView.GetPostItMgr()->SetShadowState(0,false);
        }
        
        case KEY_SHIFT:
        case KEY_MOD2:
        case KEY_MOD1:
            if ( !m_bInsDraw )
            {
                bool bTstShdwCrsr = true;

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
                    bTstShdwCrsr = false;
                    if( pTmp == pINet )
                        m_aSaveCallEvent.Set( pINet );
                    else
                    {
                        IMapObject* pIMapObj = pFmt->GetIMapObject( aDocPt );
                        if( pIMapObj )
                            m_aSaveCallEvent.Set( pFmt, pIMapObj );
                        else
                            m_aSaveCallEvent.Set( EVENT_OBJECT_URLITEM, pFmt );
                    }

                    
                    
                    if( m_aSaveCallEvent != aLastCallEvent )
                    {
                        if( aLastCallEvent.HasEvent() )
                            rSh.CallEvent( SFX_EVENT_MOUSEOUT_OBJECT,
                                            aLastCallEvent, true );
                        
                        if( !rSh.CallEvent( SFX_EVENT_MOUSEOVER_OBJECT,
                                        m_aSaveCallEvent ))
                            m_aSaveCallEvent.Clear();
                    }
                }
                else if( aLastCallEvent.HasEvent() )
                {
                    
                    rSh.CallEvent( SFX_EVENT_MOUSEOUT_OBJECT,
                                    aLastCallEvent, true );
                }

                if( bTstShdwCrsr && bInsWin && !bIsDocReadOnly &&
                    !m_bInsFrm &&
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
                        if( !m_pShadCrsr )
                            m_pShadCrsr = new SwShadowCursor( *this,
                                SwViewOption::GetDirectCursorColor() );
                        if( text::HoriOrientation::RIGHT != eOrient && text::HoriOrientation::CENTER != eOrient )
                            eOrient = text::HoriOrientation::LEFT;
                        m_pShadCrsr->SetPos( aRect.Pos(), aRect.Height(), static_cast< sal_uInt16 >(eOrient) );
                        bDelShadCrsr = false;
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

    if( bDelShadCrsr && m_pShadCrsr )
        delete m_pShadCrsr, m_pShadCrsr = 0;
    m_bWasShdwCrsr = sal_False;
}

/**
 * Button Up
 */
void SwEditWin::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bCallBase = true;

    sal_Bool bCallShadowCrsr = m_bWasShdwCrsr;
    m_bWasShdwCrsr = sal_False;
    if( m_pShadCrsr )
        delete m_pShadCrsr, m_pShadCrsr = 0;

    if( m_pRowColumnSelectionStart )
        DELETEZ( m_pRowColumnSelectionStart );

    SdrHdlKind eOldSdrMoveHdl = eSdrMoveHdl;
    eSdrMoveHdl = HDL_USER;     

    
    m_rView.SetTabColFromDoc( sal_False );
    m_rView.SetNumRuleNodeFromDoc(NULL);

    SwWrtShell &rSh = m_rView.GetWrtShell();
    SET_CURR_SHELL( &rSh );
    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        pSdrView->SetOrtho(false);

        if ( pSdrView->MouseButtonUp( rMEvt,this ) )
        {
            rSh.GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);
            return; 
        }
    }
    
    if ( !m_bMBPressed )
    {
    
    

        return;
    }

    Point aDocPt( PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( bDDTimerStarted )
    {
        StopDDTimer( &rSh, aDocPt );
        m_bMBPressed = sal_False;
        if ( rSh.IsSelFrmMode() )
        {
            rSh.EndDrag( &aDocPt, false );
            bFrmDrag = false;
        }
        bNoInterrupt = false;
        ReleaseMouse();
        return;
    }

    if( m_pAnchorMarker )
    {
        if(m_pAnchorMarker->GetHdl())
        {
            
            m_pAnchorMarker->GetHdl()->SetSelected(false);
        }

        Point aPnt( m_pAnchorMarker->GetLastPos() );
        DELETEZ( m_pAnchorMarker );
        if( aPnt.X() || aPnt.Y() )
            rSh.FindAnchorPos( aPnt, sal_True );
    }
    if ( m_bInsDraw && m_rView.GetDrawFuncPtr() )
    {
        if ( m_rView.GetDrawFuncPtr()->MouseButtonUp( rMEvt ) )
        {
            if (m_rView.GetDrawFuncPtr()) 
            {
                m_rView.GetDrawFuncPtr()->Deactivate();

                if (!m_rView.IsDrawMode())
                {
                    m_rView.SetDrawFuncPtr(NULL);
                    SfxBindings& rBind = m_rView.GetViewFrame()->GetBindings();
                    rBind.Invalidate( SID_ATTR_SIZE );
                    rBind.Invalidate( SID_TABLE_CELL );
                }
            }

            if ( rSh.IsObjSelected() )
            {
                rSh.EnterSelFrmMode();
                if (!m_rView.GetDrawFuncPtr())
                    StdDrawMode( OBJ_NONE, sal_True );
            }
            else if ( rSh.IsFrmSelected() )
            {
                rSh.EnterSelFrmMode();
                StopInsFrm();
            }
            else
            {
                const Point aDocPos( PixelToLogic( m_aStartPos ) );
                bValidCrsrPos = !(CRSR_POSCHG & rSh.SetCursor(&aDocPos, false));
                rSh.Edit();
            }

            m_rView.AttrChangedNotify( &rSh );
        }
        else if (rMEvt.GetButtons() == MOUSE_RIGHT && rSh.IsDrawCreate())
            m_rView.GetDrawFuncPtr()->BreakCreate();   

        bNoInterrupt = false;
        ReleaseMouse();
        return;
    }
    bool bPopMode = false;
    switch ( rMEvt.GetModifier() + rMEvt.GetButtons() )
    {
        case MOUSE_LEFT:
            if ( m_bInsDraw && rSh.IsDrawCreate() )
            {
                if ( m_rView.GetDrawFuncPtr() && m_rView.GetDrawFuncPtr()->MouseButtonUp(rMEvt) == sal_True )
                {
                    m_rView.GetDrawFuncPtr()->Deactivate();
                    m_rView.AttrChangedNotify( &rSh );
                    if ( rSh.IsObjSelected() )
                        rSh.EnterSelFrmMode();
                    if ( m_rView.GetDrawFuncPtr() && m_bInsFrm )
                        StopInsFrm();
                }
                bCallBase = false;
                break;
            }
        case MOUSE_LEFT + KEY_MOD1:
        case MOUSE_LEFT + KEY_MOD2:
        case MOUSE_LEFT + KEY_SHIFT + KEY_MOD1:
            if ( bFrmDrag && rSh.IsSelFrmMode() )
            {
                if ( rMEvt.IsMod1() ) 
                {
                    
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
                    else {
                        rSh.EndDrag( &aDocPt, false );
                    }
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
                            const Point aSttPt( PixelToLogic( m_aStartPos ) );
                            m_aRszMvHdlPt = aDocPt;
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
                bFrmDrag = false;
                bCallBase = false;
                break;
            }
            bPopMode = true;
            
        case MOUSE_LEFT + KEY_SHIFT:
            if (rSh.IsSelFrmMode())
            {

                rSh.EndDrag( &aDocPt, false );
                bFrmDrag = false;
                bCallBase = false;
                break;
            }

            if( bHoldSelection )
            {
                
                bHoldSelection = false;
                rSh.EndDrag( &aDocPt, false );
            }
            else
            {
                SwContentAtPos aFieldAtPos ( SwContentAtPos::SW_FIELD );
                if ( !rSh.IsInSelect() && rSh.ChgCurrPam( aDocPt ) && !rSh.GetContentAtPos( aDocPt, aFieldAtPos ) )
                {
                    const bool bTmpNoInterrupt = bNoInterrupt;
                    bNoInterrupt = false;
                    {   
                        
                        SwMvContext aMvContext( &rSh );
                        const Point aDocPos( PixelToLogic( m_aStartPos ) );
                        bValidCrsrPos = !(CRSR_POSCHG & rSh.SetCursor(&aDocPos, false));
                    }
                    bNoInterrupt = bTmpNoInterrupt;

                }
                else
                {
                    sal_Bool bInSel = rSh.IsInSelect();
                    rSh.EndDrag( &aDocPt, false );

                    
                    if( !bInSel )
                    {
                        sal_uInt16 nFilter = URLLOAD_NOFILTER;
                        if( KEY_MOD1 == rMEvt.GetModifier() )
                            nFilter |= URLLOAD_NEWVIEW;

                        sal_Bool bExecHyperlinks = m_rView.GetDocShell()->IsReadOnly();
                        if ( !bExecHyperlinks )
                        {
                            SvtSecurityOptions aSecOpts;
                            const sal_Bool bSecureOption = aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );
                            if ( (  bSecureOption && rMEvt.GetModifier() == KEY_MOD1 ) ||
                                 ( !bSecureOption && rMEvt.GetModifier() != KEY_MOD1 ) )
                                bExecHyperlinks = sal_True;
                        }

                        const bool bExecSmarttags = rMEvt.GetModifier() == KEY_MOD1;

                        if(m_pApplyTempl)
                            bExecHyperlinks = sal_False;

                        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_FIELD |
                                                    SwContentAtPos::SW_INETATTR |
                                                    SwContentAtPos::SW_SMARTTAG  | SwContentAtPos::SW_FORMCTRL);

                        if( rSh.GetContentAtPos( aDocPt, aCntntAtPos, sal_False ) )
                        {
                            
                            if ( SwContentAtPos::SW_FIELD != aCntntAtPos.eCntntAtPos
                                 && SwContentAtPos::SW_INETATTR != aCntntAtPos.eCntntAtPos )
                                rSh.GetContentAtPos( aDocPt, aCntntAtPos, sal_True );

                            sal_Bool bViewLocked = rSh.IsViewLocked();
                            if( !bViewLocked && !rSh.IsReadOnlyAvailable() &&
                                aCntntAtPos.IsInProtectSect() )
                                rSh.LockView( sal_True );

                            ReleaseMouse();

                            if( SwContentAtPos::SW_FIELD == aCntntAtPos.eCntntAtPos )
                            {
                                bool bAddMode(false);
                                
                                if (KEY_MOD1 == rMEvt.GetModifier()
                                    && !rSh.IsAddMode())
                                {
                                    bAddMode = true;
                                    rSh.EnterAddMode();
                                }
                                if ( aCntntAtPos.pFndTxtAttr != NULL
                                     && aCntntAtPos.pFndTxtAttr->Which() == RES_TXTATR_INPUTFIELD )
                                {
                                    
                                    
                                    rSh.SttSelect();
                                    rSh.SelectTxt( *(aCntntAtPos.pFndTxtAttr->GetStart()) + 1,
                                                   *(aCntntAtPos.pFndTxtAttr->End()) - 1 );
                                }
                                else
                                {
                                    rSh.ClickToField( *aCntntAtPos.aFnd.pFld );
                                }
                                
                                
                                
                                bValidCrsrPos = true;
                                if (bAddMode)
                                {
                                    rSh.LeaveAddMode();
                                }
                            }
                            else if ( SwContentAtPos::SW_SMARTTAG == aCntntAtPos.eCntntAtPos )
                            {
                                    
                                    if ( bExecSmarttags && SwSmartTagMgr::Get().IsSmartTagsEnabled() )
                                        m_rView.ExecSmartTagPopup( aDocPt );
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
                                        rSh.InvalidateWindows( m_rView.GetVisArea() );
                                    } else if ( fieldBM->GetFieldname() == ODF_FORMDROPDOWN ) {
                                        m_rView.ExecFieldPopup( aDocPt, fieldBM );
                                        fieldBM->Invalidate();
                                        rSh.InvalidateWindows( m_rView.GetVisArea() );
                                    } else {
                                        
                                    }
                                }
                            }
                            else if ( SwContentAtPos::SW_INETATTR == aCntntAtPos.eCntntAtPos )
                            {
                                if ( bExecHyperlinks && aCntntAtPos.aFnd.pAttr )
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
                                    
                                    const SvxURLField *pField = aVEvt.pURLField;
                                    if (pField)
                                    {
                                        OUString sURL(pField->GetURL());
                                        OUString sTarget(pField->GetTargetFrame());
                                        ::LoadURL(rSh, sURL, nFilter, sTarget);
                                    }
                                    bCallShadowCrsr = sal_False;
                                }
                                else
                                {
                                    
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
                bCallBase = false;

            }

            
            if ( bPopMode && bModePushed )
            {
                rSh.PopMode();
                bModePushed = false;
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
        if( pFormatClipboard )
        {
            
            SwWrtShell& rWrtShell = m_rView.GetWrtShell();
            SfxStyleSheetBasePool* pPool=0;
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
            
            pFormatClipboard->Paste( rWrtShell, pPool, bNoCharacterFormats, bNoParagraphFormats );

            
            if(!pFormatClipboard->HasContent())
                SetApplyTemplate(SwApplyTemplate());
        }
        else if( m_pApplyTempl->nColor )
        {
            sal_uInt16 nId = 0;
            switch( m_pApplyTempl->nColor )
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
                        if( !SwEditWin::m_bTransparentBackColor )
                            aColor = SwEditWin::m_aTextBackColor;
                        rSh.SetAttrItem( SvxBrushItem( aColor, nId ) );
                    }
                    else
                        rSh.SetAttrItem( SvxColorItem(SwEditWin::m_aTextColor, nId) );
                    rSh.UnSetVisCrsr();
                    rSh.EnterStdMode();
                    rSh.SetVisCrsr(aDocPt);

                    m_pApplyTempl->bUndo = true;
                    bCallBase = false;
                    m_aTemplateTimer.Stop();
                }
                else if(rMEvt.GetClicks() == 1)
                {
                    
                    m_aTemplateTimer.Start();
                }
            }
        }
        else
        {
            OUString aStyleName;
            switch ( m_pApplyTempl->eType )
            {
                case SFX_STYLE_FAMILY_PARA:
                    if( (( nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL )
                         & eSelection ) && !rSh.HasReadonlySel() )
                    {
                        rSh.SetTxtFmtColl( m_pApplyTempl->aColl.pTxtColl );
                        m_pApplyTempl->bUndo = true;
                        bCallBase = false;
                        if ( m_pApplyTempl->aColl.pTxtColl )
                            aStyleName = m_pApplyTempl->aColl.pTxtColl->GetName();
                    }
                    break;
                case SFX_STYLE_FAMILY_CHAR:
                    if( (( nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL )
                         & eSelection ) && !rSh.HasReadonlySel() )
                    {
                        rSh.SetAttrItem( SwFmtCharFmt(m_pApplyTempl->aColl.pCharFmt) );
                        rSh.UnSetVisCrsr();
                        rSh.EnterStdMode();
                        rSh.SetVisCrsr(aDocPt);
                        m_pApplyTempl->bUndo = true;
                        bCallBase = false;
                        if ( m_pApplyTempl->aColl.pCharFmt )
                            aStyleName = m_pApplyTempl->aColl.pCharFmt->GetName();
                    }
                    break;
                case SFX_STYLE_FAMILY_FRAME :
                {
                    const SwFrmFmt* pFmt = rSh.GetFmtFromObj( aDocPt );
                    if(PTR_CAST(SwFlyFrmFmt, pFmt))
                    {
                        rSh.SetFrmFmt( m_pApplyTempl->aColl.pFrmFmt, false, &aDocPt );
                        m_pApplyTempl->bUndo = true;
                        bCallBase = false;
                        if( m_pApplyTempl->aColl.pFrmFmt )
                            aStyleName = m_pApplyTempl->aColl.pFrmFmt->GetName();
                    }
                    break;
                }
                case SFX_STYLE_FAMILY_PAGE:
                    
                    rSh.ChgCurPageDesc( *m_pApplyTempl->aColl.pPageDesc );
                    if ( m_pApplyTempl->aColl.pPageDesc )
                        aStyleName = m_pApplyTempl->aColl.pPageDesc->GetName();
                    bCallBase = false;
                    break;
                case SFX_STYLE_FAMILY_PSEUDO:
                    if( !rSh.HasReadonlySel() )
                    {
                        rSh.SetCurNumRule( *m_pApplyTempl->aColl.pNumRule,
                                           false,
                                           m_pApplyTempl->aColl.pNumRule->GetDefaultListId() );
                        bCallBase = false;
                        m_pApplyTempl->bUndo = true;
                        if( m_pApplyTempl->aColl.pNumRule )
                            aStyleName = m_pApplyTempl->aColl.pNumRule->GetName();
                    }
                    break;
            }

            uno::Reference< frame::XDispatchRecorder > xRecorder =
                    m_rView.GetViewFrame()->GetBindings().GetRecorder();
            if ( !aStyleName.isEmpty() && xRecorder.is() )
            {
                SfxShell *pSfxShell = lcl_GetShellFromDispatcher( m_rView, TYPE(SwTextShell) );
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
    
    
    m_bMBPressed = sal_False;

    
    
    EnterArea();
    bNoInterrupt = false;

    if (bCallBase)
        Window::MouseButtonUp(rMEvt);
}

/**
 * Apply template
 */
void SwEditWin::SetApplyTemplate(const SwApplyTemplate &rTempl)
{
    static sal_Bool bIdle = sal_False;
    DELETEZ(m_pApplyTempl);
    SwWrtShell &rSh = m_rView.GetWrtShell();

    if(rTempl.m_pFormatClipboard)
    {
        m_pApplyTempl = new SwApplyTemplate( rTempl );
              SetPointer( POINTER_FILL );
              rSh.NoEdit( false );
              bIdle = rSh.GetViewOptions()->IsIdle();
              ((SwViewOption *)rSh.GetViewOptions())->SetIdle( sal_False );
    }
    else if(rTempl.nColor)
    {
        m_pApplyTempl = new SwApplyTemplate( rTempl );
        SetPointer( POINTER_FILL );
        rSh.NoEdit( false );
        bIdle = rSh.GetViewOptions()->IsIdle();
        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( sal_False );
    }
    else if( rTempl.eType )
    {
        m_pApplyTempl = new SwApplyTemplate( rTempl );
        SetPointer( POINTER_FILL  );
        rSh.NoEdit( false );
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
    m_rView.GetViewFrame()->GetBindings().Invalidate(aInva);
}

/**
 * Ctor
 */
SwEditWin::SwEditWin(Window *pParent, SwView &rMyView):
    Window(pParent, WinBits(WB_CLIPCHILDREN | WB_DIALOGCONTROL)),
    DropTargetHelper( this ),
    DragSourceHelper( this ),

    m_eBufferLanguage(LANGUAGE_DONTKNOW),
    m_pApplyTempl(0),
    m_pAnchorMarker( 0 ),
    m_pUserMarker( 0 ),
    m_pUserMarkerObj( 0 ),
    m_pShadCrsr( 0 ),
    m_pRowColumnSelectionStart( 0 ),

    m_rView( rMyView ),

    m_aActHitType(SDRHIT_NONE),
    m_nDropFormat( 0 ),
    m_nDropAction( 0 ),
    m_nDropDestination( 0 ),

    m_eBezierMode(SID_BEZIER_INSERT),
    m_nInsFrmColCount( 1 ),
    m_eDrawMode(OBJ_NONE),

    m_bMBPressed(false),
    m_bInsDraw(false),
    m_bInsFrm(false),
    m_bIsInMove(false),
    m_bIsInDrag(false),
    m_bOldIdle(false),
    m_bOldIdleSet(false),
    m_bTblInsDelMode(false),
    m_bTblIsInsMode(false),
    m_bChainMode(false),
    m_bWasShdwCrsr(false),
    m_bLockInput(false),
    m_bIsRowDrag(false),
    m_bUseInputLanguage(false),
    m_bObjectSelect(false),
    m_nKS_NUMDOWN_Count(0),
    m_nKS_NUMINDENTINC_Count(0),
    m_aFrameControlsManager( this )
{
    SetHelpId(HID_EDIT_WIN);
    EnableChildTransparentMode();
    SetDialogControlFlags( WINDOW_DLGCTRL_RETURN | WINDOW_DLGCTRL_WANTFOCUS );

    m_bMBPressed = m_bInsDraw = m_bInsFrm =
    m_bIsInDrag = m_bOldIdle = m_bOldIdleSet = m_bChainMode = m_bWasShdwCrsr = sal_False;
    
    m_bUseInputLanguage = sal_True;

    SetMapMode(MapMode(MAP_TWIP));

    SetPointer( POINTER_TEXT );
    m_aTimer.SetTimeoutHdl(LINK(this, SwEditWin, TimerHandler));

    m_bTblInsDelMode = sal_False;
    m_aKeyInputTimer.SetTimeout( 3000 );
    m_aKeyInputTimer.SetTimeoutHdl(LINK(this, SwEditWin, KeyInputTimerHandler));

    m_aKeyInputFlushTimer.SetTimeout( 200 );
    m_aKeyInputFlushTimer.SetTimeoutHdl(LINK(this, SwEditWin, KeyInputFlushHandler));

    
    
    m_aTemplateTimer.SetTimeout(400);
    m_aTemplateTimer.SetTimeoutHdl(LINK(this, SwEditWin, TemplateTimerHdl));

    
    
    if( !rMyView.GetDocShell()->IsReadOnly() )
    {
        Font aFont;
        SetInputContext( InputContext( aFont, INPUTCONTEXT_TEXT |
                                            INPUTCONTEXT_EXTTEXTINPUT ) );
    }
}

SwEditWin::~SwEditWin()
{
    m_aKeyInputTimer.Stop();
    delete m_pShadCrsr;
    delete m_pRowColumnSelectionStart;
    if( m_pQuickHlpData->m_bIsDisplayed && m_rView.GetWrtShellPtr() )
        m_pQuickHlpData->Stop( m_rView.GetWrtShell() );
    bExecuteDrag = false;
    delete m_pApplyTempl;
    m_rView.SetDrawFuncPtr(NULL);

    delete m_pUserMarker;
    delete m_pAnchorMarker;
}

/**
 * Turn on DrawTextEditMode
 */
void SwEditWin::EnterDrawTextMode( const Point& aDocPos )
{
    if ( m_rView.EnterDrawTextMode(aDocPos) == sal_True )
    {
        if (m_rView.GetDrawFuncPtr())
        {
            m_rView.GetDrawFuncPtr()->Deactivate();
            m_rView.SetDrawFuncPtr(NULL);
            m_rView.LeaveDrawCreate();
        }
        m_rView.NoRotate();
        m_rView.AttrChangedNotify( &m_rView.GetWrtShell() );
    }
}

/**
 * Turn on DrawMode
 */
sal_Bool SwEditWin::EnterDrawMode(const MouseEvent& rMEvt, const Point& aDocPos)
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    SdrView *pSdrView = rSh.GetDrawView();

    if ( m_rView.GetDrawFuncPtr() )
    {
        if (rSh.IsDrawCreate())
            return sal_True;

        sal_Bool bRet = m_rView.GetDrawFuncPtr()->MouseButtonDown( rMEvt );
        m_rView.AttrChangedNotify( &rSh );
        return bRet;
    }

    if ( pSdrView && pSdrView->IsTextEdit() )
    {
        bool bUnLockView = !rSh.IsViewLocked();
        rSh.LockView( sal_True );

        rSh.EndTextEdit(); 
        rSh.SelectObj( aDocPos );
        if ( !rSh.IsObjSelected() && !rSh.IsFrmSelected() )
            rSh.LeaveSelFrmMode();
        else
        {
            SwEditWin::m_nDDStartPosY = aDocPos.Y();
            SwEditWin::m_nDDStartPosX = aDocPos.X();
            bFrmDrag = true;
        }
        if( bUnLockView )
            rSh.LockView( sal_False );
        m_rView.AttrChangedNotify( &rSh );
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
    m_rView.GetWrtShell().InvalidateAccessibleFocus();
    Window::LoseFocus();
    if( m_pQuickHlpData->m_bIsDisplayed )
        m_pQuickHlpData->Stop( m_rView.GetWrtShell() );
    m_rView.LostFocus();
}

void SwEditWin::Command( const CommandEvent& rCEvt )
{
    SwWrtShell &rSh = m_rView.GetWrtShell();

    if ( !m_rView.GetViewFrame() )
    {
        
        Window::Command(rCEvt);
        return;
    }

    
    
    
    
    SfxInPlaceClient* pIPClient = rSh.GetSfxViewShell()->GetIPClient();
    bool bIsOleActive = ( pIPClient && pIPClient->IsObjectInPlaceActive() );
    if ( bIsOleActive && ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU ))
    {
        rSh.FinishOLEObj();
        return;
    }

    bool bCallBase      = true;

    switch ( rCEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            const sal_uInt16 nId = SwInputChild::GetChildWindowId();
            SwInputChild* pChildWin = (SwInputChild*)GetView().GetViewFrame()->
                                                GetChildWindow( nId );

            if (m_rView.GetPostItMgr()->IsHit(rCEvt.GetMousePosPixel()))
                return;

            Point aDocPos( PixelToLogic( rCEvt.GetMousePosPixel() ) );
            if ( !rCEvt.IsMouseEvent() )
                aDocPos = rSh.GetCharRect().Center();

            
            FrameControlType eControl;
            bool bOverFly = false;
            bool bPageAnchored = false;
            bool bOverHeaderFooterFly = IsOverHeaderFooterFly( aDocPos, eControl, bOverFly, bPageAnchored );
            
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
                    if (bNoInterrupt)
                    {
                        ReleaseMouse();
                        bNoInterrupt = false;
                        m_bMBPressed = sal_False;
                    }
                    if ( rCEvt.IsMouseEvent() )
                    {
                        SelectMenuPosition(rSh, rCEvt.GetMousePosPixel());
                        m_rView.StopShellTimer();
                    }
                    const Point aPixPos = LogicToPixel( aDocPos );

                    if ( m_rView.GetDocShell()->IsReadOnly() )
                    {
                        SwReadOnlyPopup* pROPopup = new SwReadOnlyPopup( aDocPos, m_rView );

                        ui::ContextMenuExecuteEvent aEvent;
                        aEvent.SourceWindow = VCLUnoHelper::GetInterface( this );
                        aEvent.ExecutePosition.X = aPixPos.X();
                        aEvent.ExecutePosition.Y = aPixPos.Y();
                        Menu* pMenu = 0;
                        OUString sMenuName("private:resource/ReadonlyContextMenu");
                        if( GetView().TryContextMenuInterception( *pROPopup, sMenuName, pMenu, aEvent ) )
                        {
                            if ( pMenu )
                            {
                                sal_uInt16 nExecId = ((PopupMenu*)pMenu)->Execute(this, aPixPos);
                                if( !::ExecuteMenuCommand( *static_cast<PopupMenu*>(pMenu), *m_rView.GetViewFrame(), nExecId ))
                                    pROPopup->Execute(this, nExecId);
                            }
                            else
                                pROPopup->Execute(this, aPixPos);
                        }
                        delete pROPopup;
                    }
                    else if ( !m_rView.ExecSpellPopup( aDocPos ) )
                        GetView().GetViewFrame()->GetDispatcher()->ExecutePopup( 0, this, &aPixPos);
                }
                else if (m_pApplyTempl->bUndo)
                    rSh.Do(SwWrtShell::UNDO);
                bCallBase = false;
            }
        }
        break;

        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
            if( m_pShadCrsr )
                delete m_pShadCrsr, m_pShadCrsr = 0;
            bCallBase = !m_rView.HandleWheelCommands( rCEvt );
            break;

    case COMMAND_STARTEXTTEXTINPUT:
    {
        bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
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
    case COMMAND_ENDEXTTEXTINPUT:
    {
        bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
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
                        
                        SfxShell *pSfxShell = lcl_GetShellFromDispatcher( m_rView, TYPE(SwTextShell) );
                        
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
    case COMMAND_EXTTEXTINPUT:
    {
        bool bIsDocReadOnly = m_rView.GetDocShell()->IsReadOnly() &&
                              rSh.IsCrsrReadonly();
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
                        
                        ( rACfg.IsAutoTextTip() ||
                          pACorr->GetSwFlags().bAutoCompleteWords ) &&
                        
                        rSh.GetPrevAutoCorrWord( *pACorr, sWord ) )
                    {
                        
                        ShowAutoTextCorrectQuickHelp(sWord, &rACfg, pACorr, true);
                    }
                }
        }
    }
    break;
    case COMMAND_CURSORPOS:
        
        break;

    case COMMAND_PASTESELECTION:
        if( !m_rView.GetDocShell()->IsReadOnly() )
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

                
                while( rSh.GetCrsr()->GetNext() != rSh.GetCrsr() )
                delete rSh.GetCrsr()->GetNext();
            }

            if( pCrsr )
            {
                sal_uLong nPosNodeIdx = pCrsr->GetPoint()->nNode.GetIndex();
                const sal_Int32 nPosIdx = pCrsr->GetPoint()->nContent.GetIndex();
                sal_uLong nMarkNodeIdx = pCrsr->GetMark()->nNode.GetIndex();
                const sal_Int32 nMarkIdx = pCrsr->GetMark()->nContent.GetIndex();

                if( !rSh.GetCrsr()->HasMark() )
                    rSh.GetCrsr()->SetMark();

                rSh.SttCrsrMove();

                if( nPosNodeIdx < nMarkNodeIdx )
                {
                rSh.GetCrsr()->GetPoint()->nNode = nPosNodeIdx;
                rSh.GetCrsr()->GetPoint()->nContent = nPosIdx;
                rSh.GetCrsr()->GetMark()->nNode = nPosNodeIdx;
                rSh.GetCrsr()->GetMark()->nContent =
                    rSh.GetCrsr()->GetCntntNode( true )->Len();
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
                    rSh.GetCrsr()->GetCntntNode( false )->Len();
                }

                rSh.EndCrsrMove( sal_True );
            }
        }
        break;
        case COMMAND_QUERYCHARPOSITION:
        {
            sal_Bool bVertical = rSh.IsInVerticalText();
            const SwPosition& rPos = *rSh.GetCrsr()->GetPoint();
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
                Window& rWin = rSh.GetView().GetEditWin();
                if ( nSize == 0 )
                {
                    
                    SwRect aCaretRect ( rSh.GetCharRect() );
                    Rectangle aRect( aCaretRect.Left(), aCaretRect.Top(), aCaretRect.Right(), aCaretRect.Bottom() );
                    rWin.SetCompositionCharRect( &aRect, 1, bVertical );
                }
                else
                {
                    Rectangle* aRects = new Rectangle[ nSize ];
                    int nRectIndex = 0;
                    for ( SwIndex nIndex = rStart.nContent; nIndex < rEnd.nContent; ++nIndex )
                    {
                        const SwPosition aPos( rStart.nNode, nIndex );
                        SwRect aRect ( rSh.GetCharRect() );
                        rSh.GetCharRectAt( aRect, &aPos );
                        aRects[ nRectIndex ] = Rectangle( aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom() );
                        ++nRectIndex;
                    }
                    rWin.SetCompositionCharRect( aRects, nSize, bVertical );
                    delete[] aRects;
                }
            }
            bCallBase = false;
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
    
    MouseEvent aMEvt(rMousePos);
    SdrView *pSdrView = rSh.GetDrawView();
    if ( pSdrView )
    {
        
        
        if ( !bIsInsideSelectedObj && m_rView.GetDrawFuncPtr() )
        {

            m_rView.GetDrawFuncPtr()->Deactivate();
            m_rView.SetDrawFuncPtr(NULL);
            m_rView.LeaveDrawCreate();
            SfxBindings& rBind = m_rView.GetViewFrame()->GetBindings();
            rBind.Invalidate( SID_ATTR_SIZE );
            rBind.Invalidate( SID_TABLE_CELL );
        }

        
        
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
                
                if(bVertical)
                {
                    aEEPos -= rOutputArea.TopRight();
                    
                    long nTemp = -aEEPos.X();
                    aEEPos.X() = aEEPos.Y();
                    aEEPos.Y() = nTemp;
                }
                else
                    aEEPos -= rOutputArea.TopLeft();

                EPosition aDocPosition = rEditEng.FindDocPosition(aEEPos);
                ESelection aCompare(aDocPosition.nPara, aDocPosition.nIndex);
                
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
    if ( m_rView.GetDrawFuncPtr() && m_bInsFrm )
    {
        StopInsFrm();
        rSh.Edit();
    }

    UpdatePointer( aDocPos, 0 );

    if( !rSh.IsSelFrmMode() &&
        !GetView().GetViewFrame()->GetDispatcher()->IsLocked() )
    {
        
        sal_Bool bShould = rSh.ShouldObjectBeSelected(aDocPos);

        if(bShould)
        {
            m_rView.NoRotate();
            rSh.HideCrsr();

            bool bUnLockView = !rSh.IsViewLocked();
            rSh.LockView( sal_True );
            sal_Bool bSelObj = rSh.SelectObj( aDocPos, 0);
            if( bUnLockView )
                rSh.LockView( sal_False );

            if( bSelObj )
            {
                bRet = sal_True;
                
                
                if( FRMTYPE_NONE == rSh.GetSelFrmType() )
                    rSh.ShowCrsr();
                else
                {
                    if (rSh.IsFrmSelected() && m_rView.GetDrawFuncPtr())
                    {
                        m_rView.GetDrawFuncPtr()->Deactivate();
                        m_rView.SetDrawFuncPtr(NULL);
                        m_rView.LeaveDrawCreate();
                        m_rView.AttrChangedNotify( &rSh );
                    }

                    rSh.EnterSelFrmMode( &aDocPos );
                    bFrmDrag = true;
                    UpdatePointer( aDocPos, 0 );
                    return bRet;
                }
            }

            if (!m_rView.GetDrawFuncPtr())
                rSh.ShowCrsr();
        }
    }
    else if ( rSh.IsSelFrmMode() &&
              (m_aActHitType == SDRHIT_NONE ||
               !bIsInsideSelectedObj))
    {
        m_rView.NoRotate();
        bool bUnLockView = !rSh.IsViewLocked();
        rSh.LockView( sal_True );
        sal_uInt8 nFlag = 0;

        if ( rSh.IsSelFrmMode() )
        {
            rSh.UnSelectFrm();
            rSh.LeaveSelFrmMode();
            m_rView.AttrChangedNotify(&rSh);
            bRet = sal_True;
        }

        sal_Bool bSelObj = rSh.SelectObj( aDocPos, nFlag );
        if( bUnLockView )
            rSh.LockView( sal_False );

        if( !bSelObj )
        {
            
            
            bValidCrsrPos = !(CRSR_POSCHG & rSh.SetCursor(&aDocPos, false));
            rSh.LeaveSelFrmMode();
            m_rView.LeaveDrawCreate();
            m_rView.AttrChangedNotify( &rSh );
            bRet = sal_True;
        }
        else
        {
            rSh.HideCrsr();
            rSh.EnterSelFrmMode( &aDocPos );
            rSh.SelFlyGrabCrsr();
            rSh.MakeSelVisible();
            bFrmDrag = true;
            if( rSh.IsFrmSelected() &&
                m_rView.GetDrawFuncPtr() )
            {
                m_rView.GetDrawFuncPtr()->Deactivate();
                m_rView.SetDrawFuncPtr(NULL);
                m_rView.LeaveDrawCreate();
                m_rView.AttrChangedNotify( &rSh );
            }
            UpdatePointer( aDocPos, 0 );
            bRet = sal_True;
        }
    }
    else if ( rSh.IsSelFrmMode() && bIsInsideSelectedObj )
    {
        
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
        {   
            
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
    
    SfxShell* pShell;
    SfxDispatcher* pDispatcher = rView.GetViewFrame()->GetDispatcher();
    for(sal_uInt16  i = 0; true; ++i )
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
    m_bTblInsDelMode = sal_False;
    return 0;
}

void SwEditWin::_InitStaticData()
{
    m_pQuickHlpData = new QuickHelpData();
}

void SwEditWin::_FinitStaticData()
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

IMPL_LINK_NOARG(SwEditWin, TemplateTimerHdl)
{
    SetApplyTemplate(SwApplyTemplate());
    return 0;
}

void SwEditWin::SetChainMode( sal_Bool bOn )
{
    if ( !m_bChainMode )
        StopInsFrm();

    if ( m_pUserMarker )
    {
        delete m_pUserMarker;
        m_pUserMarker = 0L;
    }

    m_bChainMode = bOn;

    static sal_uInt16 aInva[] =
    {
        FN_FRAME_CHAIN, FN_FRAME_UNCHAIN, 0
    };
    m_rView.GetViewFrame()->GetBindings().Invalidate(aInva);
}

uno::Reference< ::com::sun::star::accessibility::XAccessible > SwEditWin::CreateAccessible()
{
    SolarMutexGuard aGuard;   
    SwWrtShell *pSh = m_rView.GetWrtShellPtr();
    OSL_ENSURE( pSh, "no writer shell, no accessible object" );
    uno::Reference<
        ::com::sun::star::accessibility::XAccessible > xAcc;
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
        OUString sStr( m_aHelpStrings[ nCurArrPos ] );
        sStr = sStr.copy( nLen );
        sal_uInt16 nL = sStr.getLength();
        const sal_uInt16 nVal = EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE |
                                EXTTEXTINPUT_ATTR_HIGHLIGHT;
        const std::vector<sal_uInt16> aAttrs( nL, nVal );
        CommandExtTextInputData aCETID( sStr, &aAttrs[0], nL,
                                        0, false );

        
        
        
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

void QuickHelpData::FillStrArr( SwWrtShell& rSh, const OUString& rWord )
{
    enum Capitalization { CASE_LOWER, CASE_UPPER, CASE_SENTENCE, CASE_OTHER };

    
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

    
    uno::Sequence< i18n::CalendarItem2 > aNames( (*pCalendar)->getMonths() );
    for ( sal_uInt16 i = 0; i < 2; ++i )
    {
        for ( long n = 0; n < aNames.getLength(); ++n )
        {
            const OUString& rStr( aNames[n].FullName );
            
            if( rStr.getLength() > rWord.getLength() &&
                rCC.lowercase( rStr, 0, rWord.getLength() ) == sWordLower )
            {
                
                
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
                else 
                    m_aHelpStrings.push_back( rStr );
            }
        }
        
        if ( i == 0 )
            aNames = (*pCalendar)->getDays();
    }

    
    const SwAutoCompleteWord& rACList = rSh.GetAutoCompleteWords();
    std::vector<OUString> strings;

    if ( rACList.GetWordsMatching( rWord, strings ) )
    {
        for (unsigned int i= 0; i<strings.size(); i++)
        {
            OUString aCompletedString = strings[i];
            
            
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
            else 
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
    CompareIgnoreCaseAsciiFavorExact(const OUString& rOrigWord)
        : m_rOrigWord(rOrigWord)
    {
    }

    bool operator()(const OUString& s1, const OUString& s2) const
    {
        int nRet = s1.compareToIgnoreAsciiCase(s2);
        if (nRet == 0)
        {
            
            
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

} 


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
    m_pQuickHlpData->ClearCntnt();
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

bool SwEditWin::IsOverHeaderFooterFly( const Point& rDocPos, FrameControlType& rControl, bool& bOverFly, bool& bPageAnchored ) const
{
    bool bRet = false;
    Point aPt( rDocPos );
    SwWrtShell &rSh = m_rView.GetWrtShell();
    SwPaM aPam( *rSh.GetCurrentShellCursor().GetPoint() );
    rSh.GetLayout()->GetCrsrOfst( aPam.GetPoint(), aPt, NULL, true );

    const SwStartNode* pStartFly = aPam.GetPoint()->nNode.GetNode().FindFlyStartNode();
    if ( pStartFly )
    {
        bOverFly = true;
        SwFrmFmt* pFlyFmt = pStartFly->GetFlyFmt( );
        if ( pFlyFmt )
        {
            const SwPosition* pAnchor = pFlyFmt->GetAnchor( ).GetCntntAnchor( );
            if ( pAnchor )
            {
                bool bInHeader = pAnchor->nNode.GetNode( ).FindHeaderStartNode( ) != NULL;
                bool bInFooter = pAnchor->nNode.GetNode( ).FindFooterStartNode( ) != NULL;

                bRet = bInHeader || bInFooter;
                if ( bInHeader )
                    rControl = Header;
                else if ( bInFooter )
                    rControl = Footer;
            }
            else
                bPageAnchored = pFlyFmt->GetAnchor( ).GetAnchorId( ) == FLY_AT_PAGE;
        }
    }
    else
        bOverFly = false;
    return bRet;
}

void SwEditWin::SetUseInputLanguage( sal_Bool bNew )
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
        SwPosition *pPos = rSh.GetCrsr()->GetPoint();
        const sal_Int32 nPos = pPos->nContent.GetIndex();

        
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
    SwWrtShell& rSh = m_rView.GetWrtShell();
    if( rSh.HasSelection() )
    {
        OUString sReturn;
        rSh.GetSelectedText( sReturn, GETSELTXT_PARABRK_TO_ONLYCR  );
        return Selection( 0, sReturn.getLength() );
    }
    else
    {
        
        
        SwPosition *pPos = rSh.GetCrsr()->GetPoint();
        const sal_Int32 nPos = pPos->nContent.GetIndex();

        rSh.HideCrsr();
        rSh.GoStartSentence();
        const sal_Int32 nStartPos = rSh.GetCrsr()->GetPoint()->nContent.GetIndex();

        pPos->nContent = nPos;
        rSh.ClearMark();
        rSh.ShowCrsr();

        return Selection( nPos - nStartPos, nPos - nStartPos );
    }
}



void SwEditWin::SwitchView()
{
#ifdef ACCESSIBLE_LAYOUT
    if (!Application::IsAccessibilityEnabled())
    {
        return ;
    }
    rView.GetWrtShell().InvalidateAccessibleFocus();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
