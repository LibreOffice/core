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

#include <memory>
#include <boost/property_tree/json_parser.hpp>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>
#include <comphelper/string.hxx>
#include <paintfrm.hxx>

// Here static members are defined. They will get changed on alteration of the
// MapMode. This is done so that on ShowCursor the same size does not have to be
// expensively determined again and again.

long SwSelPaintRects::s_nPixPtX = 0;
long SwSelPaintRects::s_nPixPtY = 0;
MapMode* SwSelPaintRects::s_pMapMode = nullptr;

// Starting from here: classes / methods for the non-text-cursor
SwVisibleCursor::SwVisibleCursor( const SwCursorShell * pCShell )
    : m_pCursorShell( pCShell )
    , m_nPageLastTime(0)
{
    pCShell->GetWin()->SetCursor( &m_aTextCursor );
    m_bIsVisible = m_aTextCursor.IsVisible();
    m_bIsDragCursor = false;
    m_aTextCursor.SetWidth( 0 );
}

SwVisibleCursor::~SwVisibleCursor()
{
    if( m_bIsVisible && m_aTextCursor.IsVisible() )
        m_aTextCursor.Hide();

    m_pCursorShell->GetWin()->SetCursor( nullptr );
}

void SwVisibleCursor::Show()
{
    if( !m_bIsVisible )
    {
        m_bIsVisible = true;

        // display at all?
        if( m_pCursorShell->VisArea().IsOver( m_pCursorShell->m_aCharRect ) || comphelper::LibreOfficeKit::isActive() )
            _SetPosAndShow();
    }
}

void SwVisibleCursor::Hide()
{
    if( m_bIsVisible )
    {
        m_bIsVisible = false;

        if( m_aTextCursor.IsVisible() )      // Shouldn't the flags be in effect?
            m_aTextCursor.Hide();
    }
}

void SwVisibleCursor::_SetPosAndShow()
{
    SwRect aRect;
    long nTmpY = m_pCursorShell->m_aCursorHeight.getY();
    if( 0 > nTmpY )
    {
        nTmpY = -nTmpY;
        m_aTextCursor.SetOrientation( 900 );
        aRect = SwRect( m_pCursorShell->m_aCharRect.Pos(),
           Size( m_pCursorShell->m_aCharRect.Height(), nTmpY ) );
        aRect.Pos().setX(aRect.Pos().getX() + m_pCursorShell->m_aCursorHeight.getX());
        if( m_pCursorShell->IsOverwriteCursor() )
            aRect.Pos().setY(aRect.Pos().getY() + aRect.Width());
    }
    else
    {
        m_aTextCursor.SetOrientation();
        aRect = SwRect( m_pCursorShell->m_aCharRect.Pos(),
           Size( m_pCursorShell->m_aCharRect.Width(), nTmpY ) );
        aRect.Pos().setY(aRect.Pos().getY() + m_pCursorShell->m_aCursorHeight.getX());
    }

    // check if cursor should show the current cursor bidi level
    m_aTextCursor.SetDirection();
    const SwCursor* pTmpCursor = m_pCursorShell->_GetCursor();

    if ( pTmpCursor && !m_pCursorShell->IsOverwriteCursor() )
    {
        SwNode& rNode = pTmpCursor->GetPoint()->nNode.GetNode();
        if( rNode.IsTextNode() )
        {
            const SwTextNode& rTNd = *rNode.GetTextNode();
            const SwFrame* pFrame = rTNd.getLayoutFrame( m_pCursorShell->GetLayout(), nullptr, nullptr, false );
            if ( pFrame )
            {
                const SwScriptInfo* pSI = static_cast<const SwTextFrame*>(pFrame)->GetScriptInfo();
                 // cursor level has to be shown
                if ( pSI && pSI->CountDirChg() > 1 )
                {
                    m_aTextCursor.SetDirection(
                        ( pTmpCursor->GetCursorBidiLevel() % 2 ) ?
                          CursorDirection::RTL :
                          CursorDirection::LTR );
                }
                if ( pFrame->IsRightToLeft() )
                {
                    const OutputDevice *pOut = m_pCursorShell->GetOut();
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
        ::SwCalcPixStatics( m_pCursorShell->GetOut() );
        ::SwAlignRect( aRect, static_cast<SwViewShell const *>(m_pCursorShell), m_pCursorShell->GetOut() );
    }
    if( !m_pCursorShell->IsOverwriteCursor() || m_bIsDragCursor ||
        m_pCursorShell->IsSelection() )
        aRect.Width( 0 );

    m_aTextCursor.SetSize( aRect.SSize() );

    m_aTextCursor.SetPos( aRect.Pos() );

    if (comphelper::LibreOfficeKit::isActive())
    {
        // notify about page number change (if that happened)
        sal_uInt16 nPage, nVirtPage;
        const_cast<SwCursorShell*>(m_pCursorShell)->GetPageNum(nPage, nVirtPage);
        if (nPage != m_nPageLastTime)
        {
            m_nPageLastTime = nPage;
            OString aPayload = OString::number(nPage - 1);
            m_pCursorShell->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_SET_PART, aPayload.getStr());
        }

        // notify about the cursor position & size
        Rectangle aSVRect(aRect.Pos().getX(), aRect.Pos().getY(), aRect.Pos().getX() + aRect.SSize().Width(), aRect.Pos().getY() + aRect.SSize().Height());
        OString sRect = aSVRect.toString();
        m_pCursorShell->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, sRect.getStr());
        SfxLokHelper::notifyOtherViews(m_pCursorShell->GetSfxViewShell(), LOK_CALLBACK_INVALIDATE_VIEW_CURSOR, "rectangle", sRect);
    }

    if ( !m_pCursorShell->IsCursorReadonly()  || m_pCursorShell->GetViewOptions()->IsSelectionInReadonly() )
    {
        if ( m_pCursorShell->GetDrawView() )
            const_cast<SwDrawView*>(static_cast<const SwDrawView*>(m_pCursorShell->GetDrawView()))->SetAnimationEnabled(
                    !m_pCursorShell->IsSelection() );

        sal_uInt16 nStyle = m_bIsDragCursor ? CURSOR_SHADOW : 0;
        if( nStyle != m_aTextCursor.GetStyle() )
        {
            m_aTextCursor.SetStyle( nStyle );
            m_aTextCursor.SetWindow( m_bIsDragCursor ? m_pCursorShell->GetWin() : nullptr );
        }

        m_aTextCursor.Show();
    }
}

SwSelPaintRects::SwSelPaintRects( const SwCursorShell& rCSh )
    : SwRects()
    , m_pCursorShell( &rCSh )
#if HAVE_FEATURE_DESKTOP
    , m_pCursorOverlay(nullptr)
    , m_bShowTextInputFieldOverlay(true)
    , m_pTextInputFieldOverlay(nullptr)
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

    const bool bTempShowTextInputFieldOverlay = m_bShowTextInputFieldOverlay;
    m_bShowTextInputFieldOverlay = rSwap.m_bShowTextInputFieldOverlay;
    rSwap.m_bShowTextInputFieldOverlay = bTempShowTextInputFieldOverlay;

    sw::overlay::OverlayRangesOutline* pTempTextInputFieldOverlay = m_pTextInputFieldOverlay;
    m_pTextInputFieldOverlay = rSwap.m_pTextInputFieldOverlay;
    rSwap.m_pTextInputFieldOverlay = pTempTextInputFieldOverlay;
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

    if (m_pTextInputFieldOverlay != nullptr)
    {
        delete m_pTextInputFieldOverlay;
        m_pTextInputFieldOverlay = nullptr;
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
    const SwContentNode* pNode = rPosition.nNode.GetNode().GetContentNode();
    const SwContentFrame* pFrame = pNode->getLayoutFrame(pNode->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &rPoint, &rPosition);
    SwRect aRect;
    pFrame->GetCharRect(aRect, rPosition);
    return aRect;
}

void SwShellCursor::FillStartEnd(SwRect& rStart, SwRect& rEnd) const
{
    const SwShellCursor* pCursor = GetShell()->getShellCursor(false);
    rStart = lcl_getLayoutRect(pCursor->GetSttPos(), *pCursor->Start());
    rEnd = lcl_getLayoutRect(pCursor->GetEndPos(), *pCursor->End());
}

void SwSelPaintRects::Show(std::vector<OString>* pSelectionRectangles)
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

        HighlightInputField();
#endif

        // Tiled editing does not expose the draw and writer cursor, it just
        // talks about "the" cursor at the moment. As long as that's true,
        // don't say anything about the Writer cursor till a draw object is
        // being edited.
        if (comphelper::LibreOfficeKit::isActive() && !pView->GetTextEditObject())
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
                    GetShell()->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_START, sRect.getStr());
                }
                if (aEndRect.HasArea())
                {
                    OString sRect = aEndRect.SVRect().toString();
                    GetShell()->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_END, sRect.getStr());
                }
            }

            std::vector<OString> aRect;
            for (size_type i = 0; i < size(); ++i)
            {
                const SwRect& rRect = (*this)[i];
                aRect.push_back(rRect.SVRect().toString());
            }
            OString sRect = comphelper::string::join("; ", aRect);
            if (!pSelectionRectangles)
            {
                GetShell()->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, sRect.getStr());
                SfxLokHelper::notifyOtherViews(GetShell()->GetSfxViewShell(), LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection", sRect);
            }
            else
                pSelectionRectangles->push_back(sRect);
        }
    }
}

void SwSelPaintRects::HighlightInputField()
{
    std::vector< basegfx::B2DRange > aInputFieldRanges;

    if (m_bShowTextInputFieldOverlay)
    {
        SwTextInputField* pCurTextInputFieldAtCursor =
            dynamic_cast<SwTextInputField*>(SwCursorShell::GetTextFieldAtPos( GetShell()->GetCursor()->Start(), false ));
        if ( pCurTextInputFieldAtCursor != nullptr )
        {
            SwTextNode* pTextNode = pCurTextInputFieldAtCursor->GetpTextNode();
            std::unique_ptr<SwShellCursor> pCursorForInputTextField(
                new SwShellCursor( *GetShell(), SwPosition( *pTextNode, pCurTextInputFieldAtCursor->GetStart() ) ) );
            pCursorForInputTextField->SetMark();
            pCursorForInputTextField->GetMark()->nNode = *pTextNode;
            pCursorForInputTextField->GetMark()->nContent.Assign( pTextNode, *(pCurTextInputFieldAtCursor->End()) );

            pCursorForInputTextField->FillRects();
            SwRects* pRects = static_cast<SwRects*>(pCursorForInputTextField.get());
            for (size_t a(0); a < pRects->size(); ++a)
            {
                const SwRect aNextRect((*pRects)[a]);
                const Rectangle aPntRect(aNextRect.SVRect());

                aInputFieldRanges.push_back(basegfx::B2DRange(
                    aPntRect.Left(), aPntRect.Top(),
                    aPntRect.Right() + 1, aPntRect.Bottom() + 1));
            }
        }
    }

    if ( aInputFieldRanges.size() > 0 )
    {
        if (m_pTextInputFieldOverlay != nullptr)
        {
            m_pTextInputFieldOverlay->setRanges( aInputFieldRanges );
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

                m_pTextInputFieldOverlay = new sw::overlay::OverlayRangesOutline(
                        aHighlight, aInputFieldRanges );
                xTargetOverlay->add( *m_pTextInputFieldOverlay );
            }
        }
    }
    else
    {
        if (m_pTextInputFieldOverlay != nullptr)
        {
            delete m_pTextInputFieldOverlay;
            m_pTextInputFieldOverlay = nullptr;
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

SwShellCursor::SwShellCursor(
    const SwCursorShell& rCShell,
    const SwPosition &rPos )
    : SwCursor(rPos,nullptr,false)
    , SwSelPaintRects(rCShell)
    , m_pInitialPoint(SwPaM::GetPoint())
{}

SwShellCursor::SwShellCursor(
    const SwCursorShell& rCShell,
    const SwPosition &rPos,
    const Point& rPtPos,
    SwPaM* pRing )
    : SwCursor(rPos, pRing, false)
    , SwSelPaintRects(rCShell)
    , m_MarkPt(rPtPos)
    , m_PointPt(rPtPos)
    , m_pInitialPoint(SwPaM::GetPoint())
{}

SwShellCursor::SwShellCursor( SwShellCursor& rICursor )
    : SwCursor(rICursor, &rICursor)
    , SwSelPaintRects(*rICursor.GetShell())
    , m_MarkPt(rICursor.GetMkPos())
    , m_PointPt(rICursor.GetPtPos())
    , m_pInitialPoint(SwPaM::GetPoint())
{}

SwShellCursor::~SwShellCursor()
{}

bool SwShellCursor::IsReadOnlyAvailable() const
{
    return GetShell()->IsReadOnlyAvailable();
}

void SwShellCursor::SetMark()
{
    if (SwPaM::GetPoint() == m_pInitialPoint)
        m_MarkPt = m_PointPt;
    else
        m_PointPt = m_MarkPt;
    SwPaM::SetMark();
}

void SwShellCursor::FillRects()
{
    // calculate the new rectangles
    if( HasMark() &&
        GetPoint()->nNode.GetNode().IsContentNode() &&
        GetPoint()->nNode.GetNode().GetContentNode()->getLayoutFrame( GetShell()->GetLayout() ) &&
        (GetMark()->nNode == GetPoint()->nNode ||
        (GetMark()->nNode.GetNode().IsContentNode() &&
         GetMark()->nNode.GetNode().GetContentNode()->getLayoutFrame( GetShell()->GetLayout() ) )   ))
        GetShell()->GetLayout()->CalcFrameRects( *this );
}

void SwShellCursor::Show()
{
    std::vector<OString> aSelectionRectangles;
    for(SwPaM& rPaM : GetRingContainer())
    {
        SwShellCursor* pShCursor = dynamic_cast<SwShellCursor*>(&rPaM);
        if(pShCursor)
            pShCursor->SwSelPaintRects::Show(&aSelectionRectangles);
    }

    if (comphelper::LibreOfficeKit::isActive())
    {
        std::vector<OString> aRect;
        for (size_t i = 0; i < aSelectionRectangles.size(); ++i)
        {
            const OString& rSelectionRectangle = aSelectionRectangles[i];
            if (rSelectionRectangle.isEmpty())
                continue;
            aRect.push_back(rSelectionRectangle);
        }
        OString sRect = comphelper::string::join("; ", aRect);
        GetShell()->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, sRect.getStr());
        SfxLokHelper::notifyOtherViews(GetShell()->GetSfxViewShell(), LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection", sRect);
    }
}

// This rectangle gets painted anew, therefore the SSelection in this
// area is invalid.
void SwShellCursor::Invalidate( const SwRect& rRect )
{
    for(SwPaM& rPaM : GetRingContainer())
    {
        SwShellCursor* pShCursor = dynamic_cast<SwShellCursor*>(&rPaM);
        // skip any non SwShellCursor objects in the ring
        // see also: SwAutoFormat::DeleteSel()
        if(pShCursor)
            pShCursor->SwSelPaintRects::Invalidate(rRect);
    }
}

void SwShellCursor::Hide()
{
    for(SwPaM& rPaM : GetRingContainer())
    {
        SwShellCursor* pShCursor = dynamic_cast<SwShellCursor*>(&rPaM);
        if(pShCursor)
            pShCursor->SwSelPaintRects::Hide();
    }
}

SwCursor* SwShellCursor::Create( SwPaM* pRing ) const
{
    return new SwShellCursor( *GetShell(), *GetPoint(), GetPtPos(), pRing );
}

short SwShellCursor::MaxReplaceArived()
{
    short nRet = RET_YES;
    vcl::Window* pDlg = SwView::GetSearchDialog();
    if( pDlg )
    {
        // Terminate old actions. The table-frames get constructed and
        // a SSelection can be created.
        std::vector<sal_uInt16> vActionCounts;
        for(SwViewShell& rShell : const_cast< SwCursorShell* >( GetShell() )->GetRingContainer())
        {
            sal_uInt16 nActCnt = 0;
            while(rShell.ActionPend())
            {
                rShell.EndAction();
                ++nActCnt;
            }
            vActionCounts.push_back(nActCnt);
        }
        nRet = ScopedVclPtrInstance<MessageDialog>::Create(pDlg, "AskSearchDialog",
                "modules/swriter/ui/asksearchdialog.ui")->Execute();
        auto pActionCount = vActionCounts.begin();
        for(SwViewShell& rShell : const_cast< SwCursorShell* >( GetShell() )->GetRingContainer())
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

void SwShellCursor::SaveTableBoxContent( const SwPosition* pPos )
{
    const_cast<SwCursorShell*>(GetShell())->SaveTableBoxContent( pPos );
}

bool SwShellCursor::UpDown( bool bUp, sal_uInt16 nCnt )
{
    return SwCursor::UpDown( bUp, nCnt,
                            &GetPtPos(), GetShell()->GetUpDownX() );
}

// if <true> than the cursor can be set to the position.
bool SwShellCursor::IsAtValidPos( bool bPoint ) const
{
    if( GetShell() && ( GetShell()->IsAllProtect() ||
        GetShell()->GetViewOptions()->IsReadonly() ||
        ( GetShell()->Imp()->GetDrawView() &&
          GetShell()->Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() )))
        return true;

    return SwCursor::IsAtValidPos( bPoint );
}

SwShellTableCursor::SwShellTableCursor( const SwCursorShell& rCursorSh,
                                    const SwPosition& rPos )
    : SwCursor(rPos,nullptr,false), SwShellCursor(rCursorSh, rPos), SwTableCursor(rPos)
{
}

SwShellTableCursor::SwShellTableCursor( const SwCursorShell& rCursorSh,
                    const SwPosition& rMkPos, const Point& rMkPt,
                    const SwPosition& rPtPos, const Point& rPtPt )
    : SwCursor(rPtPos,nullptr,false), SwShellCursor(rCursorSh, rPtPos), SwTableCursor(rPtPos)
{
    SetMark();
    *GetMark() = rMkPos;
    GetMkPos() = rMkPt;
    GetPtPos() = rPtPt;
}

SwShellTableCursor::~SwShellTableCursor() {}

void SwShellTableCursor::SetMark()                { SwShellCursor::SetMark(); }

SwCursor* SwShellTableCursor::Create( SwPaM* pRing ) const
{
    return SwShellCursor::Create( pRing );
}

short SwShellTableCursor::MaxReplaceArived()
{
    return SwShellCursor::MaxReplaceArived();
}

void SwShellTableCursor::SaveTableBoxContent( const SwPosition* pPos )
{
    SwShellCursor::SaveTableBoxContent( pPos );
}

void SwShellTableCursor::FillRects()
{
    // Calculate the new rectangles. If the cursor is still "parked" do nothing
    if (m_SelectedBoxes.empty() || m_bParked || !GetPoint()->nNode.GetIndex())
        return;

    bool bStart = true;
    SwRegionRects aReg( GetShell()->VisArea() );
    if (comphelper::LibreOfficeKit::isActive())
        aReg = GetShell()->getIDocumentLayoutAccess().GetCurrentLayout()->Frame();
    SwNodes& rNds = GetDoc()->GetNodes();
    SwFrame* pEndFrame = nullptr;
    for (size_t n = 0; n < m_SelectedBoxes.size(); ++n)
    {
        const SwStartNode* pSttNd = m_SelectedBoxes[n]->GetSttNd();
        const SwTableNode* pSelTableNd = pSttNd->FindTableNode();

        SwNodeIndex aIdx( *pSttNd );
        SwContentNode* pCNd = rNds.GoNextSection( &aIdx, true, false );

        // table in table
        // (see also lcl_FindTopLevelTable in unoobj2.cxx for a different
        // version to do this)
        const SwTableNode* pCurTableNd = pCNd ? pCNd->FindTableNode() : nullptr;
        while ( pSelTableNd != pCurTableNd && pCurTableNd )
        {
            aIdx = pCurTableNd->EndOfSectionIndex();
            pCNd = rNds.GoNextSection( &aIdx, true, false );
            pCurTableNd = pCNd->FindTableNode();
        }

        if( !pCNd )
            continue;

        SwFrame* pFrame = pCNd->getLayoutFrame( GetShell()->GetLayout(), &GetSttPos() );
        while( pFrame && !pFrame->IsCellFrame() )
            pFrame = pFrame->GetUpper();

        OSL_ENSURE( pFrame, "Node not in a table" );

        while ( pFrame )
        {
            if( aReg.GetOrigin().IsOver( pFrame->Frame() ) )
            {
                aReg -= pFrame->Frame();
                if (bStart)
                {
                    bStart = false;
                    m_aStart = SwRect(pFrame->Frame().Left(), pFrame->Frame().Top(), 1, pFrame->Frame().Height());
                }
            }

            pEndFrame = pFrame;
            pFrame = pFrame->GetNextCellLeaf( MAKEPAGE_NONE );
        }
    }
    if (pEndFrame)
        m_aEnd = SwRect(pEndFrame->Frame().Right(), pEndFrame->Frame().Top(), 1, pEndFrame->Frame().Height());
    aReg.Invert();
    insert( begin(), aReg.begin(), aReg.end() );
}

void SwShellTableCursor::FillStartEnd(SwRect& rStart, SwRect& rEnd) const
{
    rStart = m_aStart;
    rEnd = m_aEnd;
}

// Check if the SPoint is within the Table-SSelection.
bool SwShellTableCursor::IsInside( const Point& rPt ) const
{
    // Calculate the new rectangles. If the cursor is still "parked" do nothing
    if (m_SelectedBoxes.empty() || m_bParked || !GetPoint()->nNode.GetIndex())
        return false;

    SwNodes& rNds = GetDoc()->GetNodes();
    for (size_t n = 0; n < m_SelectedBoxes.size(); ++n)
    {
        SwNodeIndex aIdx( *m_SelectedBoxes[n]->GetSttNd() );
        SwContentNode* pCNd = rNds.GoNextSection( &aIdx, true, false );
        if( !pCNd )
            continue;

        SwFrame* pFrame = pCNd->getLayoutFrame( GetShell()->GetLayout(), &GetPtPos() );
        while( pFrame && !pFrame->IsCellFrame() )
            pFrame = pFrame->GetUpper();
        OSL_ENSURE( pFrame, "Node not in a table" );
        if( pFrame && pFrame->Frame().IsInside( rPt ) )
            return true;
    }
    return false;
}

bool SwShellTableCursor::IsAtValidPos( bool bPoint ) const
{
    return SwShellCursor::IsAtValidPos( bPoint );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
