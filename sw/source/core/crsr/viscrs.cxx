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
#include <vcl/layout.hxx>
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
#include <IDocumentLayoutAccess.hxx>

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/srchdlg.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdr/overlay/overlayselection.hxx>
#include <overlayrangesoutline.hxx>

#include <boost/scoped_ptr.hpp>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <paintfrm.hxx>

// Here static members are defined. They will get changed on alteration of the
// MapMode. This is done so that on ShowCrsr the same size does not have to be
// expensively determined again and again.

long SwSelPaintRects::s_nPixPtX = 0;
long SwSelPaintRects::s_nPixPtY = 0;
MapMode* SwSelPaintRects::s_pMapMode = 0;

// Starting from here: classes / methods for the non-text-cursor
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
        if( m_pCrsrShell->VisArea().IsOver( m_pCrsrShell->m_aCharRect ) || m_pCrsrShell->isTiledRendering() )
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
    m_aTxtCrsr.SetDirection( CursorDirection::NONE );
    const SwCursor* pTmpCrsr = m_pCrsrShell->_GetCrsr();

    if ( pTmpCrsr && !m_pCrsrShell->IsOverwriteCrsr() )
    {
        SwNode& rNode = pTmpCrsr->GetPoint()->nNode.GetNode();
        if( rNode.IsTxtNode() )
        {
            const SwTxtNode& rTNd = *rNode.GetTxtNode();
            const SwFrm* pFrm = rTNd.getLayoutFrm( m_pCrsrShell->GetLayout(), 0, 0, false );
            if ( pFrm )
            {
                const SwScriptInfo* pSI = static_cast<const SwTxtFrm*>(pFrm)->GetScriptInfo();
                 // cursor level has to be shown
                if ( pSI && pSI->CountDirChg() > 1 )
                {
                    m_aTxtCrsr.SetDirection(
                        ( pTmpCrsr->GetCrsrBidiLevel() % 2 ) ?
                          CursorDirection::RTL :
                          CursorDirection::LTR );
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

    if (m_pCrsrShell->isTiledRendering())
    {
        Rectangle aSVRect(aRect.Pos().getX(), aRect.Pos().getY(), aRect.Pos().getX() + aRect.SSize().Width(), aRect.Pos().getY() + aRect.SSize().Height());
        OString sRect = aSVRect.toString();
        m_pCrsrShell->libreOfficeKitCallback(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, sRect.getStr());
    }

    if ( !m_pCrsrShell->IsCrsrReadonly()  || m_pCrsrShell->GetViewOptions()->IsSelectionInReadonly() )
    {
        if ( m_pCrsrShell->GetDrawView() )
            const_cast<SwDrawView*>(static_cast<const SwDrawView*>(m_pCrsrShell->GetDrawView()))->SetAnimationEnabled(
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
    , m_pCursorShell( &rCSh )
#if HAVE_FEATURE_DESKTOP
    , m_pCursorOverlay(nullptr)
    , m_bShowTxtInputFldOverlay(true)
    , m_pTxtInputFldOverlay(nullptr)
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
    // #i75172# also swap m_pCursorOverlay
    sdr::overlay::OverlayObject* pTempOverlay = getCursorOverlay();
    setCursorOverlay(rSwap.getCursorOverlay());
    rSwap.setCursorOverlay(pTempOverlay);

    const bool bTempShowTxtInputFldOverlay = m_bShowTxtInputFldOverlay;
    m_bShowTxtInputFldOverlay = rSwap.m_bShowTxtInputFldOverlay;
    rSwap.m_bShowTxtInputFldOverlay = bTempShowTxtInputFldOverlay;

    sw::overlay::OverlayRangesOutline* pTempTxtInputFldOverlay = m_pTxtInputFldOverlay;
    m_pTxtInputFldOverlay = rSwap.m_pTxtInputFldOverlay;
    rSwap.m_pTxtInputFldOverlay = pTempTxtInputFldOverlay;
#endif
}

void SwSelPaintRects::Hide()
{
#if HAVE_FEATURE_DESKTOP
    if (m_pCursorOverlay)
    {
        delete m_pCursorOverlay;
        m_pCursorOverlay = nullptr;
    }

    if (m_pTxtInputFldOverlay != nullptr)
    {
        delete m_pTxtInputFldOverlay;
        m_pTxtInputFldOverlay = nullptr;
    }
#endif

    SwRects::clear();
}

/**
 * Return a layout rectangle (typically with minimal width) that represents a
 * cursor at rPosition.
 *
 * @param rPoint layout position as a hint about what layout frame contains
 * rPosition (there might be multiple frames for a single node)
 * @param rPosition the doc model position (paragraph / character index)
 */
static SwRect lcl_getLayoutRect(const Point& rPoint, const SwPosition& rPosition)
{
    const SwCntntNode* pNode = rPosition.nNode.GetNode().GetCntntNode();
    const SwCntntFrm* pFrm = pNode->getLayoutFrm(pNode->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &rPoint, &rPosition);
    SwRect aRect;
    pFrm->GetCharRect(aRect, rPosition);
    return aRect;
}

void SwShellCrsr::FillStartEnd(SwRect& rStart, SwRect& rEnd) const
{
    const SwShellCrsr* pCursor = GetShell()->getShellCrsr(false);
    rStart = lcl_getLayoutRect(pCursor->GetSttPos(), *pCursor->Start());
    rEnd = lcl_getLayoutRect(pCursor->GetEndPos(), *pCursor->End());
}

void SwSelPaintRects::Show()
{
    SdrView *const pView = const_cast<SdrView*>(m_pCursorShell->GetDrawView());

    if(pView && pView->PaintWindowCount())
    {
        // reset rects
        SwRects::clear();
        FillRects();

#if HAVE_FEATURE_DESKTOP
        // get new rects
        std::vector< basegfx::B2DRange > aNewRanges;

        for(size_type a = 0; a < size(); ++a)
        {
            const SwRect aNextRect((*this)[a]);
            const Rectangle aPntRect(aNextRect.SVRect());

            aNewRanges.push_back(basegfx::B2DRange(
                aPntRect.Left(), aPntRect.Top(),
                aPntRect.Right() + 1, aPntRect.Bottom() + 1));
        }

        if (m_pCursorOverlay)
        {
            if(!aNewRanges.empty())
            {
                static_cast<sdr::overlay::OverlaySelection*>(m_pCursorOverlay)->setRanges(aNewRanges);
            }
            else
            {
                delete m_pCursorOverlay;
                m_pCursorOverlay = nullptr;
            }
        }
        else if(!empty())
        {
            SdrPaintWindow* pCandidate = pView->GetPaintWindow(0);
            rtl::Reference< sdr::overlay::OverlayManager > xTargetOverlay = pCandidate->GetOverlayManager();

            if (xTargetOverlay.is())
            {
                // get the system's highlight color
                const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                const Color aHighlight(aSvtOptionsDrawinglayer.getHilightColor());

                // create correct selection
                m_pCursorOverlay = new sdr::overlay::OverlaySelection(
                    sdr::overlay::OVERLAY_TRANSPARENT,
                    aHighlight,
                    aNewRanges,
                    true);

                xTargetOverlay->add(*m_pCursorOverlay);
            }
        }

        HighlightInputFld();
#endif

        // Tiled editing does not expose the draw and writer cursor, it just
        // talks about "the" cursor at the moment. As long as that's true,
        // don't say anything about the Writer cursor till a draw object is
        // being edited.
        if (GetShell()->isTiledRendering() && !pView->GetTextEditObject())
        {
            if (!empty())
            {
                // The selection may be a complex polygon, emit the logical
                // start/end cursor rectangle of the selection as separate
                // events, if there is a real selection.
                // This can be used to easily show selection handles on the
                // client side.
                SwRect aStartRect;
                SwRect aEndRect;
                FillStartEnd(aStartRect, aEndRect);

                if (aStartRect.HasArea())
                {
                    OString sRect = aStartRect.SVRect().toString();
                    GetShell()->libreOfficeKitCallback(LOK_CALLBACK_TEXT_SELECTION_START, sRect.getStr());
                }
                if (aEndRect.HasArea())
                {
                    OString sRect = aEndRect.SVRect().toString();
                    GetShell()->libreOfficeKitCallback(LOK_CALLBACK_TEXT_SELECTION_END, sRect.getStr());
                }
            }

            std::stringstream ss;
            for (size_type i = 0; i < size(); ++i)
            {
                const SwRect& rRect = (*this)[i];
                if (i)
                    ss << "; ";
                ss << rRect.SVRect().toString().getStr();
            }
            OString sRect = ss.str().c_str();
            GetShell()->libreOfficeKitCallback(LOK_CALLBACK_TEXT_SELECTION, sRect.getStr());
        }
    }
}

void SwSelPaintRects::HighlightInputFld()
{
    std::vector< basegfx::B2DRange > aInputFldRanges;

    if (m_bShowTxtInputFldOverlay)
    {
        SwTxtInputFld* pCurTxtInputFldAtCrsr =
            dynamic_cast<SwTxtInputFld*>(SwCrsrShell::GetTxtFldAtPos( GetShell()->GetCrsr()->Start(), false ));
        if ( pCurTxtInputFldAtCrsr != NULL )
        {
            SwTxtNode* pTxtNode = pCurTxtInputFldAtCrsr->GetpTxtNode();
            ::boost::scoped_ptr<SwShellCrsr> pCrsrForInputTxtFld(
                new SwShellCrsr( *GetShell(), SwPosition( *pTxtNode, pCurTxtInputFldAtCrsr->GetStart() ) ) );
            pCrsrForInputTxtFld->SetMark();
            pCrsrForInputTxtFld->GetMark()->nNode = *pTxtNode;
            pCrsrForInputTxtFld->GetMark()->nContent.Assign( pTxtNode, *(pCurTxtInputFldAtCrsr->End()) );

            pCrsrForInputTxtFld->FillRects();
            SwRects* pRects = static_cast<SwRects*>(pCrsrForInputTxtFld.get());
            for (size_t a(0); a < pRects->size(); ++a)
            {
                const SwRect aNextRect((*pRects)[a]);
                const Rectangle aPntRect(aNextRect.SVRect());

                aInputFldRanges.push_back(basegfx::B2DRange(
                    aPntRect.Left(), aPntRect.Top(),
                    aPntRect.Right() + 1, aPntRect.Bottom() + 1));
            }
        }
    }

    if ( aInputFldRanges.size() > 0 )
    {
        if (m_pTxtInputFldOverlay != nullptr)
        {
            m_pTxtInputFldOverlay->setRanges( aInputFldRanges );
        }
        else
        {
            SdrView* pView = const_cast<SdrView*>(GetShell()->GetDrawView());
            SdrPaintWindow* pCandidate = pView->GetPaintWindow(0);
            rtl::Reference<sdr::overlay::OverlayManager> xTargetOverlay = pCandidate->GetOverlayManager();

            if (xTargetOverlay.is())
            {
                // use system's highlight color with decreased luminance as highlight color
                const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                Color aHighlight(aSvtOptionsDrawinglayer.getHilightColor());
                aHighlight.DecreaseLuminance( 128 );

                m_pTxtInputFldOverlay = new sw::overlay::OverlayRangesOutline(
                        aHighlight, aInputFldRanges );
                xTargetOverlay->add( *m_pTxtInputFldOverlay );
            }
        }
    }
    else
    {
        if (m_pTxtInputFldOverlay != nullptr)
        {
            delete m_pTxtInputFldOverlay;
            m_pTxtInputFldOverlay = nullptr;
        }
    }
}

void SwSelPaintRects::Invalidate( const SwRect& rRect )
{
    size_type nSz = size();
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
                rRectIt.Right( rRectIt.Right() + s_nPixPtX );
            if( rRectIt.Bottom() == GetShell()->m_aOldRBPos.Y() )
                rRectIt.Bottom( rRectIt.Bottom() + s_nPixPtY );
        }
    }
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
    if (s_pMapMode->GetMapUnit() != rMM.GetMapUnit() ||
        s_pMapMode->GetScaleX() != rMM.GetScaleX() ||
        s_pMapMode->GetScaleY() != rMM.GetScaleY())
    {
        *s_pMapMode = rMM;
        Size aTmp( 1, 1 );
        aTmp = pOut->PixelToLogic( aTmp );
        s_nPixPtX = aTmp.Width();
        s_nPixPtY = aTmp.Height();
    }
    if( pX )
        *pX = s_nPixPtX;
    if( pY )
        *pY = s_nPixPtY;
}

SwShellCrsr::SwShellCrsr(
    const SwCrsrShell& rCShell,
    const SwPosition &rPos )
    : SwCursor(rPos,0,false)
    , SwSelPaintRects(rCShell)
    , m_pInitialPoint(SwPaM::GetPoint())
{}

SwShellCrsr::SwShellCrsr(
    const SwCrsrShell& rCShell,
    const SwPosition &rPos,
    const Point& rPtPos,
    SwPaM* pRing )
    : SwCursor(rPos, pRing, false)
    , SwSelPaintRects(rCShell)
    , m_MarkPt(rPtPos)
    , m_PointPt(rPtPos)
    , m_pInitialPoint(SwPaM::GetPoint())
{}

SwShellCrsr::SwShellCrsr( SwShellCrsr& rICrsr )
    : SwCursor(rICrsr, &rICrsr)
    , SwSelPaintRects(*rICrsr.GetShell())
    , m_MarkPt(rICrsr.GetMkPos())
    , m_PointPt(rICrsr.GetPtPos())
    , m_pInitialPoint(SwPaM::GetPoint())
{}

SwShellCrsr::~SwShellCrsr()
{}

bool SwShellCrsr::IsReadOnlyAvailable() const
{
    return GetShell()->IsReadOnlyAvailable();
}

void SwShellCrsr::SetMark()
{
    if (SwPaM::GetPoint() == m_pInitialPoint)
        m_MarkPt = m_PointPt;
    else
        m_PointPt = m_MarkPt;
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
        GetShell()->GetLayout()->CalcFrmRects( *this );
}

void SwShellCrsr::Show()
{
    for(SwPaM& rPaM : GetRingContainer())
    {
        SwShellCrsr* pShCrsr = dynamic_cast<SwShellCrsr*>(&rPaM);
        if(pShCrsr)
            pShCrsr->SwSelPaintRects::Show();
    }
}

// This rectangle gets painted anew, therefore the SSelection in this
// area is invalid.
void SwShellCrsr::Invalidate( const SwRect& rRect )
{
    for(SwPaM& rPaM : GetRingContainer())
    {
        SwShellCrsr* pShCrsr = dynamic_cast<SwShellCrsr*>(&rPaM);
        // skip any non SwShellCrsr objects in the ring
        // see also: SwAutoFormat::DeleteSel()
        if(pShCrsr)
            pShCrsr->SwSelPaintRects::Invalidate(rRect);
    }
}

void SwShellCrsr::Hide()
{
    for(SwPaM& rPaM : GetRingContainer())
    {
        SwShellCrsr* pShCrsr = dynamic_cast<SwShellCrsr*>(&rPaM);
        if(pShCrsr)
            pShCrsr->SwSelPaintRects::Hide();
    }
}

SwCursor* SwShellCrsr::Create( SwPaM* pRing ) const
{
    return new SwShellCrsr( *GetShell(), *GetPoint(), GetPtPos(), pRing );
}

short SwShellCrsr::MaxReplaceArived()
{
    short nRet = RET_YES;
    vcl::Window* pDlg = SwView::GetSearchDialog();
    if( pDlg )
    {
        // Terminate old actions. The table-frames get constructed and
        // a SSelection can be created.
        std::vector<sal_uInt16> vActionCounts;
        for(SwViewShell& rShell : const_cast< SwCrsrShell* >( GetShell() )->GetRingContainer())
        {
            sal_uInt16 nActCnt = 0;
            while(rShell.ActionPend())
            {
                rShell.EndAction();
                ++nActCnt;
            }
            vActionCounts.push_back(nActCnt);
        }
        nRet = MessageDialog(pDlg, "AskSearchDialog",
                "modules/swriter/ui/asksearchdialog.ui").Execute();
        auto pActionCount = vActionCounts.begin();
        for(SwViewShell& rShell : const_cast< SwCrsrShell* >( GetShell() )->GetRingContainer())
        {
            while(*pActionCount)
            {
                rShell.StartAction();
                --(*pActionCount);
            }
            ++pActionCount;
        }
    }
    else
        // otherwise from the Basic, and than switch to RET_YES
        nRet = RET_YES;

    return nRet;
}

void SwShellCrsr::SaveTblBoxCntnt( const SwPosition* pPos )
{
    const_cast<SwCrsrShell*>(GetShell())->SaveTblBoxCntnt( pPos );
}

bool SwShellCrsr::UpDown( bool bUp, sal_uInt16 nCnt )
{
    return SwCursor::UpDown( bUp, nCnt,
                            &GetPtPos(), GetShell()->GetUpDownX() );
}

// if <true> than the cursor can be set to the position.
bool SwShellCrsr::IsAtValidPos( bool bPoint ) const
{
    if( GetShell() && ( GetShell()->IsAllProtect() ||
        GetShell()->GetViewOptions()->IsReadonly() ||
        ( GetShell()->Imp()->GetDrawView() &&
          GetShell()->Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() )))
        return true;

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
    if (m_SelectedBoxes.empty() || m_bParked || !GetPoint()->nNode.GetIndex())
        return;

    bool bStart = true;
    SwRegionRects aReg( GetShell()->VisArea() );
    if (GetShell()->isTiledRendering())
        aReg = GetShell()->getIDocumentLayoutAccess()->GetCurrentLayout()->Frm();
    SwNodes& rNds = GetDoc()->GetNodes();
    SwFrm* pEndFrm = 0;
    for (size_t n = 0; n < m_SelectedBoxes.size(); ++n)
    {
        const SwStartNode* pSttNd = m_SelectedBoxes[n]->GetSttNd();
        const SwTableNode* pSelTblNd = pSttNd->FindTableNode();

        SwNodeIndex aIdx( *pSttNd );
        SwCntntNode* pCNd = rNds.GoNextSection( &aIdx, true, false );

        // table in table
        // (see also lcl_FindTopLevelTable in unoobj2.cxx for a different
        // version to do this)
        const SwTableNode* pCurTblNd = pCNd ? pCNd->FindTableNode() : NULL;
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
            {
                aReg -= pFrm->Frm();
                if (bStart)
                {
                    bStart = false;
                    m_aStart = SwRect(pFrm->Frm().Left(), pFrm->Frm().Top(), 1, pFrm->Frm().Height());
                }
            }

            pEndFrm = pFrm;
            pFrm = pFrm->GetNextCellLeaf( MAKEPAGE_NONE );
        }
    }
    if (pEndFrm)
        m_aEnd = SwRect(pEndFrm->Frm().Right(), pEndFrm->Frm().Top(), 1, pEndFrm->Frm().Height());
    aReg.Invert();
    insert( begin(), aReg.begin(), aReg.end() );
}

void SwShellTableCrsr::FillStartEnd(SwRect& rStart, SwRect& rEnd) const
{
    rStart = m_aStart;
    rEnd = m_aEnd;
}

// Check if the SPoint is within the Table-SSelection.
bool SwShellTableCrsr::IsInside( const Point& rPt ) const
{
    // Calculate the new rectangles. If the cursor is still "parked" do nothing
    if (m_SelectedBoxes.empty() || m_bParked || !GetPoint()->nNode.GetIndex())
        return false;

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
            return true;
    }
    return false;
}

bool SwShellTableCrsr::IsAtValidPos( bool bPoint ) const
{
    return SwShellCrsr::IsAtValidPos( bPoint );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
