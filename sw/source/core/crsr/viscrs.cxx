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

#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/settings.hxx>
#include <viewopt.hxx>
#include <frmtool.hxx>
#include <viscrs.hxx>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <swtable.hxx>
#include <viewimp.hxx>
#include <dview.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <docary.hxx>
#include <extinput.hxx>
#include <ndtxt.hxx>
#include <txtfld.hxx>
#include <scriptinfo.hxx>
#include <mdiexp.hxx>
#include <wrtsh.hxx>
#include <comcore.hrc>
#include <view.hxx>

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdr/overlay/overlayselection.hxx>
#include <overlayrangesoutline.hxx>

#include <boost/scoped_ptr.hpp>

#include <touch/touch.h>

extern void SwCalcPixStatics( OutputDevice *pOut );

// Here static members are defined. They will get changed on alteration of the
// MapMode. This is done so that on ShowCrsr the same size does not have to be
// expensively determined again and again.

long SwSelPaintRects::nPixPtX = 0;
long SwSelPaintRects::nPixPtY = 0;
MapMode* SwSelPaintRects::pMapMode = 0;

// -----  Starting from here: classes / methods for the non-text-cursor -----

SwVisCrsr::SwVisCrsr( const SwCrsrShell * pCShell )
    : m_pCrsrShell( pCShell )
{
    pCShell->GetWin()->SetCursor( &m_aTxtCrsr );
    m_bIsVisible = m_aTxtCrsr.IsVisible();
    m_bIsDragCrsr = false;
    m_aTxtCrsr.SetWidth( 0 );
}

SwVisCrsr::~SwVisCrsr()
{
    if( m_bIsVisible && m_aTxtCrsr.IsVisible() )
        m_aTxtCrsr.Hide();

    m_pCrsrShell->GetWin()->SetCursor( 0 );
}

void SwVisCrsr::Show()
{
    if( !m_bIsVisible )
    {
        m_bIsVisible = true;

        // display at all?
        if( m_pCrsrShell->VisArea().IsOver( m_pCrsrShell->m_aCharRect ) )
            _SetPosAndShow();
    }
}

void SwVisCrsr::Hide()
{
    if( m_bIsVisible )
    {
        m_bIsVisible = false;

        if( m_aTxtCrsr.IsVisible() )      // Shouldn't the flags be in effect?
            m_aTxtCrsr.Hide();
    }
}

void SwVisCrsr::_SetPosAndShow()
{
    SwRect aRect;
    long nTmpY = m_pCrsrShell->m_aCrsrHeight.getY();
    if( 0 > nTmpY )
    {
        nTmpY = -nTmpY;
        m_aTxtCrsr.SetOrientation( 900 );
        aRect = SwRect( m_pCrsrShell->m_aCharRect.Pos(),
           Size( m_pCrsrShell->m_aCharRect.Height(), nTmpY ) );
        aRect.Pos().setX(aRect.Pos().getX() + m_pCrsrShell->m_aCrsrHeight.getX());
        if( m_pCrsrShell->IsOverwriteCrsr() )
            aRect.Pos().setY(aRect.Pos().getY() + aRect.Width());
    }
    else
    {
        m_aTxtCrsr.SetOrientation( 0 );
        aRect = SwRect( m_pCrsrShell->m_aCharRect.Pos(),
           Size( m_pCrsrShell->m_aCharRect.Width(), nTmpY ) );
        aRect.Pos().setY(aRect.Pos().getY() + m_pCrsrShell->m_aCrsrHeight.getX());
    }

    // check if cursor should show the current cursor bidi level
    m_aTxtCrsr.SetDirection( CURSOR_DIRECTION_NONE );
    const SwCursor* pTmpCrsr = m_pCrsrShell->_GetCrsr();

    if ( pTmpCrsr && !m_pCrsrShell->IsOverwriteCrsr() )
    {
        SwNode& rNode = pTmpCrsr->GetPoint()->nNode.GetNode();
        if( rNode.IsTxtNode() )
        {
            const SwTxtNode& rTNd = *rNode.GetTxtNode();
            const SwFrm* pFrm = rTNd.getLayoutFrm( m_pCrsrShell->GetLayout(), 0, 0, sal_False );
            if ( pFrm )
            {
                const SwScriptInfo* pSI = ((SwTxtFrm*)pFrm)->GetScriptInfo();
                 // cursor level has to be shown
                if ( pSI && pSI->CountDirChg() > 1 )
                {
                    m_aTxtCrsr.SetDirection(
                        ( pTmpCrsr->GetCrsrBidiLevel() % 2 ) ?
                          CURSOR_DIRECTION_RTL :
                          CURSOR_DIRECTION_LTR );
                }
                if ( pFrm->IsRightToLeft() )
                {
                    const OutputDevice *pOut = m_pCrsrShell->GetOut();
                    if ( pOut )
                    {
                        long nSize = pOut->GetSettings().GetStyleSettings().GetCursorSize();
                        Size aSize( nSize, nSize );
                        aSize = pOut->PixelToLogic( aSize );
                        aRect.Left( aRect.Left() - aSize.Width() );
                    }
                }
            }
        }
    }

    if( aRect.Height() )
    {
        ::SwCalcPixStatics( m_pCrsrShell->GetOut() );
        ::SwAlignRect( aRect, (SwViewShell*)m_pCrsrShell );
    }
    if( !m_pCrsrShell->IsOverwriteCrsr() || m_bIsDragCrsr ||
        m_pCrsrShell->IsSelection() )
        aRect.Width( 0 );

    m_aTxtCrsr.SetSize( aRect.SSize() );

    m_aTxtCrsr.SetPos( aRect.Pos() );
    if ( !m_pCrsrShell->IsCrsrReadonly()  || m_pCrsrShell->GetViewOptions()->IsSelectionInReadonly() )
    {
        if ( m_pCrsrShell->GetDrawView() )
            ((SwDrawView*)m_pCrsrShell->GetDrawView())->SetAnimationEnabled(
                    !m_pCrsrShell->IsSelection() );

        sal_uInt16 nStyle = m_bIsDragCrsr ? CURSOR_SHADOW : 0;
        if( nStyle != m_aTxtCrsr.GetStyle() )
        {
            m_aTxtCrsr.SetStyle( nStyle );
            m_aTxtCrsr.SetWindow( m_bIsDragCrsr ? m_pCrsrShell->GetWin() : 0 );
        }

        m_aTxtCrsr.Show();
    }
}

SwSelPaintRects::SwSelPaintRects( const SwCrsrShell& rCSh )
    : SwRects()
    , pCShell( &rCSh )
#if HAVE_FEATURE_DESKTOP
    , mpCursorOverlay( 0 )
    , mbShowTxtInputFldOverlay( true )
    , mpTxtInputFldOverlay( NULL )
#endif
{
}

SwSelPaintRects::~SwSelPaintRects()
{
    Hide();
}

void SwSelPaintRects::swapContent(SwSelPaintRects& rSwap)
{
    SwRects::swap(rSwap);

#if HAVE_FEATURE_DESKTOP
    // #i75172# also swap mpCursorOverlay
    sdr::overlay::OverlayObject* pTempOverlay = getCursorOverlay();
    setCursorOverlay(rSwap.getCursorOverlay());
    rSwap.setCursorOverlay(pTempOverlay);

    const bool bTempShowTxtInputFldOverlay = mbShowTxtInputFldOverlay;
    mbShowTxtInputFldOverlay = rSwap.mbShowTxtInputFldOverlay;
    rSwap.mbShowTxtInputFldOverlay = bTempShowTxtInputFldOverlay;

    sw::overlay::OverlayRangesOutline* pTempTxtInputFldOverlay = mpTxtInputFldOverlay;
    mpTxtInputFldOverlay = rSwap.mpTxtInputFldOverlay;
    rSwap.mpTxtInputFldOverlay = pTempTxtInputFldOverlay;
#endif
}

void SwSelPaintRects::Hide()
{
#if HAVE_FEATURE_DESKTOP
    if(mpCursorOverlay)
    {
        delete mpCursorOverlay;
        mpCursorOverlay = 0;
    }

    if ( mpTxtInputFldOverlay != NULL )
    {
        delete mpTxtInputFldOverlay;
        mpTxtInputFldOverlay = NULL;
    }
#endif

    SwRects::clear();
}

void SwSelPaintRects::Show()
{
    SdrView* pView = (SdrView*)pCShell->GetDrawView();

    if(pView && pView->PaintWindowCount())
    {
        // reset rects
        SwRects::clear();
        FillRects();

#if HAVE_FEATURE_DESKTOP
        // get new rects
        std::vector< basegfx::B2DRange > aNewRanges;

        for(sal_uInt16 a(0); a < size(); a++)
        {
            const SwRect aNextRect((*this)[a]);
            const Rectangle aPntRect(aNextRect.SVRect());

            aNewRanges.push_back(basegfx::B2DRange(
                aPntRect.Left(), aPntRect.Top(),
                aPntRect.Right() + 1, aPntRect.Bottom() + 1));
        }

        if(mpCursorOverlay)
        {
            if(!aNewRanges.empty())
            {
                static_cast< sdr::overlay::OverlaySelection* >(mpCursorOverlay)->setRanges(aNewRanges);
            }
            else
            {
                delete mpCursorOverlay;
                mpCursorOverlay = 0;
            }
        }
        else if(!empty())
        {
            SdrPaintWindow* pCandidate = pView->GetPaintWindow(0);
            rtl::Reference< ::sdr::overlay::OverlayManager > xTargetOverlay = pCandidate->GetOverlayManager();

            if (xTargetOverlay.is())
            {
                // get the system's hilight color
                const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                const Color aHighlight(aSvtOptionsDrawinglayer.getHilightColor());

                // create correct selection
                mpCursorOverlay = new sdr::overlay::OverlaySelection(
                    sdr::overlay::OVERLAY_TRANSPARENT,
                    aHighlight,
                    aNewRanges,
                    true);

                xTargetOverlay->add(*mpCursorOverlay);
            }
        }

        HighlightInputFld();
#else
        const OutputDevice* pOut = GetShell()->GetWin();
        if ( ! pOut )
            pOut = GetShell()->GetOut();
        SwWrtShell *pWrtShell = dynamic_cast<SwWrtShell*>(const_cast<SwCrsrShell*>(GetShell()));
        if (!empty())
        {
            if (pWrtShell)
            {
                // Buffer will be deallocated in the UI layer
                MLORect *rects = (MLORect *) malloc((sizeof(MLORect))*size());
                for (size_t i = 0; i < size(); ++i)
                {
                    Point origin = pOut->LogicToPixel((*this)[i].Pos());
                    Size ssize = pOut->LogicToPixel((*this)[i].SSize());
#ifdef IOS
                    rects[i] = CGRectMake(origin.X(), origin.Y(),
                                          ssize.Width(), ssize.Height());
#else
                    // Not yet implemented
                    (void) origin;
                    (void) ssize;
#endif
                }
                // GetShell returns a SwCrsrShell which actually is a SwWrtShell
                touch_ui_selection_start(MLOSelectionText, pWrtShell, rects, size(), NULL);
            }
        }
        else
        {
            touch_ui_selection_none();
        }
#endif
    }
}

#if !HAVE_FEATURE_DESKTOP

extern "C" void touch_lo_selection_attempt_resize(const void * /* documentHandle */,
                                                  MLORect * /* selectedRectangles */,
                                                  int /* numberOfRectangles */)
{
}

#endif

void SwSelPaintRects::HighlightInputFld()
{
    std::vector< basegfx::B2DRange > aInputFldRanges;

    if ( mbShowTxtInputFldOverlay )
    {
        SwTxtInputFld* pCurTxtInputFldAtCrsr =
            dynamic_cast<SwTxtInputFld*>(GetShell()->GetTxtFldAtPos( GetShell()->GetCrsr()->Start(), false ));
        if ( pCurTxtInputFldAtCrsr != NULL )
        {
            SwTxtNode* pTxtNode = pCurTxtInputFldAtCrsr->GetpTxtNode();
            ::boost::scoped_ptr<SwShellCrsr> pCrsrForInputTxtFld(
                new SwShellCrsr( *GetShell(), SwPosition( *pTxtNode, *(pCurTxtInputFldAtCrsr->GetStart()) ) ) );
            pCrsrForInputTxtFld->SetMark();
            pCrsrForInputTxtFld->GetMark()->nNode = *pTxtNode;
            pCrsrForInputTxtFld->GetMark()->nContent.Assign( pTxtNode, *(pCurTxtInputFldAtCrsr->End()) );

            pCrsrForInputTxtFld->FillRects();

            for (size_t a(0); a < pCrsrForInputTxtFld->size(); ++a)
            {
                const SwRect aNextRect((*pCrsrForInputTxtFld)[a]);
                const Rectangle aPntRect(aNextRect.SVRect());

                aInputFldRanges.push_back(basegfx::B2DRange(
                    aPntRect.Left(), aPntRect.Top(),
                    aPntRect.Right() + 1, aPntRect.Bottom() + 1));
            }
        }
    }

    if ( aInputFldRanges.size() > 0 )
    {
        if ( mpTxtInputFldOverlay != NULL )
        {
            mpTxtInputFldOverlay->setRanges( aInputFldRanges );
        }
        else
        {
            SdrView* pView = (SdrView*)GetShell()->GetDrawView();
            SdrPaintWindow* pCandidate = pView->GetPaintWindow(0);
            rtl::Reference<sdr::overlay::OverlayManager> xTargetOverlay = pCandidate->GetOverlayManager();

            if (xTargetOverlay.is())
            {
                // use system's hilight color with decreased luminance as highlight color
                const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                Color aHighlight(aSvtOptionsDrawinglayer.getHilightColor());
                aHighlight.DecreaseLuminance( 128 );

                mpTxtInputFldOverlay = new sw::overlay::OverlayRangesOutline( aHighlight, aInputFldRanges );
                xTargetOverlay->add( *mpTxtInputFldOverlay );
            }
        }
    }
    else
    {
        if ( mpTxtInputFldOverlay != NULL )
        {
            delete mpTxtInputFldOverlay;
            mpTxtInputFldOverlay = NULL;
        }
    }
}

void SwSelPaintRects::Invalidate( const SwRect& rRect )
{
    sal_uInt16 nSz = size();
    if( !nSz )
        return;

    SwRegionRects aReg( GetShell()->VisArea() );
    aReg.assign( begin(), end() );
    aReg -= rRect;
    SwRects::erase( begin(), begin() + nSz );
    SwRects::insert( begin(), aReg.begin(), aReg.end() );

    // If the selection is to the right or at the bottom, outside the
    // visible area, it is never aligned on one pixel at the right/bottom.
    // This has to be determined here and if that is the case the
    // rectangle has to be expanded.
    if( GetShell()->m_bVisPortChgd && 0 != ( nSz = size()) )
    {
        SwSelPaintRects::Get1PixelInLogic( *GetShell() );
        iterator it = begin();
        for( ; nSz--; ++it )
        {
            SwRect& rRectIt = *it;
            if( rRectIt.Right() == GetShell()->m_aOldRBPos.X() )
                rRectIt.Right( rRectIt.Right() + nPixPtX );
            if( rRectIt.Bottom() == GetShell()->m_aOldRBPos.Y() )
                rRectIt.Bottom( rRectIt.Bottom() + nPixPtY );
        }
    }
}

void SwSelPaintRects::Paint( const Rectangle& /*rRect*/ )
{
    // nothing to do with overlays
}

// check current MapMode of the shell and set possibly the static members.
// Optional set the parameters pX, pY
void SwSelPaintRects::Get1PixelInLogic( const SwViewShell& rSh,
                                        long* pX, long* pY )
{
    const OutputDevice* pOut = rSh.GetWin();
    if ( ! pOut )
        pOut = rSh.GetOut();

    const MapMode& rMM = pOut->GetMapMode();
    if( pMapMode->GetMapUnit() != rMM.GetMapUnit() ||
        pMapMode->GetScaleX() != rMM.GetScaleX() ||
        pMapMode->GetScaleY() != rMM.GetScaleY() )
    {
        *pMapMode = rMM;
        Size aTmp( 1, 1 );
        aTmp = pOut->PixelToLogic( aTmp );
        nPixPtX = aTmp.Width();
        nPixPtY = aTmp.Height();
    }
    if( pX )
        *pX = nPixPtX;
    if( pY )
        *pY = nPixPtY;
}

SwShellCrsr::SwShellCrsr(
    const SwCrsrShell& rCShell,
    const SwPosition &rPos )
    : SwCursor(rPos,0,false)
    , SwSelPaintRects(rCShell)
    , pPt(SwPaM::GetPoint())
{}

SwShellCrsr::SwShellCrsr(
    const SwCrsrShell& rCShell,
    const SwPosition &rPos,
    const Point& rPtPos,
    SwPaM* pRing )
    : SwCursor(rPos, pRing, false)
    , SwSelPaintRects(rCShell)
    , aMkPt(rPtPos)
    , aPtPt(rPtPos)
    , pPt(SwPaM::GetPoint())
{}

SwShellCrsr::SwShellCrsr( SwShellCrsr& rICrsr )
    : SwCursor(rICrsr)
    , SwSelPaintRects(*rICrsr.GetShell())
    , aMkPt(rICrsr.GetMkPos())
    , aPtPt(rICrsr.GetPtPos())
    , pPt(SwPaM::GetPoint())
{}

SwShellCrsr::~SwShellCrsr()
{}

bool SwShellCrsr::IsReadOnlyAvailable() const
{
    return GetShell()->IsReadOnlyAvailable();
}

void SwShellCrsr::SetMark()
{
    if( SwPaM::GetPoint() == pPt )
        aMkPt = aPtPt;
    else
        aPtPt = aMkPt;
    SwPaM::SetMark();
}

void SwShellCrsr::FillRects()
{
    // calculate the new rectangles
    if( HasMark() &&
        GetPoint()->nNode.GetNode().IsCntntNode() &&
        GetPoint()->nNode.GetNode().GetCntntNode()->getLayoutFrm( GetShell()->GetLayout() ) &&
        (GetMark()->nNode == GetPoint()->nNode ||
        (GetMark()->nNode.GetNode().IsCntntNode() &&
         GetMark()->nNode.GetNode().GetCntntNode()->getLayoutFrm( GetShell()->GetLayout() ) )   ))
        GetShell()->GetLayout()->CalcFrmRects( *this, GetShell()->IsTableMode() );
}

void SwShellCrsr::Show()
{
    SwShellCrsr * pTmp = this;
    do {
        pTmp->SwSelPaintRects::Show();
    } while( this != ( pTmp = dynamic_cast<SwShellCrsr*>(pTmp->GetNext()) ) );
}

// This rectangle gets painted anew, therefore the SSelection in this
// area is invalid.
void SwShellCrsr::Invalidate( const SwRect& rRect )
{
    SwShellCrsr * pTmp = this;

    do
    {
        pTmp->SwSelPaintRects::Invalidate( rRect );

        // skip any non SwShellCrsr objects in the ring
        // see also: SwAutoFormat::DeleteSel()
        Ring* pTmpRing = pTmp;
        pTmp = 0;
        do
        {
            pTmpRing = pTmpRing->GetNext();
            pTmp = dynamic_cast<SwShellCrsr*>(pTmpRing);
        }
        while ( !pTmp );
    }
    while( this != pTmp );
}

void SwShellCrsr::Hide()
{
    SwShellCrsr * pTmp = this;
    do {
        pTmp->SwSelPaintRects::Hide();
    } while( this != ( pTmp = dynamic_cast<SwShellCrsr*>(pTmp->GetNext()) ) );
}

SwCursor* SwShellCrsr::Create( SwPaM* pRing ) const
{
    return new SwShellCrsr( *GetShell(), *GetPoint(), GetPtPos(), pRing );
}

short SwShellCrsr::MaxReplaceArived()
{
    short nRet = RET_YES;
    Window* pDlg = (Window*) SwView::GetSearchDialog();
    if( pDlg )
    {
        // Terminate old actions. The table-frames get constructed and
        // a SSelection can be created.
        std::vector<sal_uInt16> aArr;
        sal_uInt16 nActCnt;
        SwViewShell *pShell = const_cast< SwCrsrShell* >( GetShell() ),
                  *pSh = pShell;
        do {
            for( nActCnt = 0; pSh->ActionPend(); ++nActCnt )
                pSh->EndAction();
            aArr.push_back( nActCnt );
        } while( pShell != ( pSh = (SwViewShell*)pSh->GetNext() ) );

        {
            nRet = QueryBox( pDlg, SW_RES( MSG_COMCORE_ASKSEARCH )).Execute();
        }

        for( sal_uInt16 n = 0; n < aArr.size(); ++n )
        {
            for( nActCnt = aArr[n]; nActCnt--; )
                pSh->StartAction();
            pSh = (SwViewShell*)pSh->GetNext();
        }
    }
    else
        // otherwise from the Basic, and than switch to RET_YES
        nRet = RET_YES;

    return nRet;
}

void SwShellCrsr::SaveTblBoxCntnt( const SwPosition* pPos )
{
    ((SwCrsrShell*)GetShell())->SaveTblBoxCntnt( pPos );
}

bool SwShellCrsr::UpDown( bool bUp, sal_uInt16 nCnt )
{
    return SwCursor::UpDown( bUp, nCnt,
                            &GetPtPos(), GetShell()->GetUpDownX() );
}

// if <true> than the cursor can be set to the position.
sal_Bool SwShellCrsr::IsAtValidPos( sal_Bool bPoint ) const
{
    if( GetShell() && ( GetShell()->IsAllProtect() ||
        GetShell()->GetViewOptions()->IsReadonly() ||
        ( GetShell()->Imp()->GetDrawView() &&
          GetShell()->Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() )))
        return sal_True;

    return SwCursor::IsAtValidPos( bPoint );
}

SwShellTableCrsr::SwShellTableCrsr( const SwCrsrShell& rCrsrSh,
                                    const SwPosition& rPos )
    : SwCursor(rPos,0,false), SwShellCrsr(rCrsrSh, rPos), SwTableCursor(rPos)
{
}

SwShellTableCrsr::SwShellTableCrsr( const SwCrsrShell& rCrsrSh,
                    const SwPosition& rMkPos, const Point& rMkPt,
                    const SwPosition& rPtPos, const Point& rPtPt )
    : SwCursor(rPtPos,0,false), SwShellCrsr(rCrsrSh, rPtPos), SwTableCursor(rPtPos)
{
    SetMark();
    *GetMark() = rMkPos;
    GetMkPos() = rMkPt;
    GetPtPos() = rPtPt;
}

SwShellTableCrsr::~SwShellTableCrsr() {}

void SwShellTableCrsr::SetMark()                { SwShellCrsr::SetMark(); }

SwCursor* SwShellTableCrsr::Create( SwPaM* pRing ) const
{
    return SwShellCrsr::Create( pRing );
}

short SwShellTableCrsr::MaxReplaceArived()
{
    return SwShellCrsr::MaxReplaceArived();
}

void SwShellTableCrsr::SaveTblBoxCntnt( const SwPosition* pPos )
{
    SwShellCrsr::SaveTblBoxCntnt( pPos );
}

void SwShellTableCrsr::FillRects()
{
    // Calculate the new rectangles. If the cursor is still "parked" do nothing
    if (m_SelectedBoxes.empty() || bParked || !GetPoint()->nNode.GetIndex())
        return;

    SwRegionRects aReg( GetShell()->VisArea() );
    SwNodes& rNds = GetDoc()->GetNodes();
    for (size_t n = 0; n < m_SelectedBoxes.size(); ++n)
    {
        const SwStartNode* pSttNd = m_SelectedBoxes[n]->GetSttNd();
        const SwTableNode* pSelTblNd = pSttNd->FindTableNode();

        SwNodeIndex aIdx( *pSttNd );
           SwCntntNode* pCNd = rNds.GoNextSection( &aIdx, true, false );

        // table in table
        // (see also lcl_FindTopLevelTable in unoobj2.cxx for a different
        // version to do this)
        const SwTableNode* pCurTblNd = pCNd->FindTableNode();
        while ( pSelTblNd != pCurTblNd && pCurTblNd )
        {
            aIdx = pCurTblNd->EndOfSectionIndex();
            pCNd = rNds.GoNextSection( &aIdx, true, false );
            pCurTblNd = pCNd->FindTableNode();
        }

        if( !pCNd )
            continue;

        SwFrm* pFrm = pCNd->getLayoutFrm( GetShell()->GetLayout(), &GetSttPos() );
        while( pFrm && !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();

        OSL_ENSURE( pFrm, "Node not in a table" );

        while ( pFrm )
        {
            if( aReg.GetOrigin().IsOver( pFrm->Frm() ) )
                aReg -= pFrm->Frm();

            pFrm = pFrm->GetNextCellLeaf( MAKEPAGE_NONE );
        }
    }
    aReg.Invert();
    insert( begin(), aReg.begin(), aReg.end() );
}

// Check if the SPoint is within the Table-SSelection.
sal_Bool SwShellTableCrsr::IsInside( const Point& rPt ) const
{
    // Calculate the new rectangles. If the cursor is still "parked" do nothing
    if (m_SelectedBoxes.empty() || bParked || !GetPoint()->nNode.GetIndex())
        return sal_False;

    SwNodes& rNds = GetDoc()->GetNodes();
    for (size_t n = 0; n < m_SelectedBoxes.size(); ++n)
    {
        SwNodeIndex aIdx( *m_SelectedBoxes[n]->GetSttNd() );
        SwCntntNode* pCNd = rNds.GoNextSection( &aIdx, true, false );
        if( !pCNd )
            continue;

        SwFrm* pFrm = pCNd->getLayoutFrm( GetShell()->GetLayout(), &GetPtPos() );
        while( pFrm && !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();
        OSL_ENSURE( pFrm, "Node not in a table" );
        if( pFrm && pFrm->Frm().IsInside( rPt ) )
            return sal_True;
    }
    return sal_False;
}

sal_Bool SwShellTableCrsr::IsAtValidPos( sal_Bool bPoint ) const
{
    return SwShellCrsr::IsAtValidPos( bPoint );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
