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

#include <config_feature_desktop.h>

#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
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
#include <ndtxt.hxx>
#include <txtfld.hxx>
#include <scriptinfo.hxx>
#include <view.hxx>
#include <IDocumentLayoutAccess.hxx>

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/srchdlg.hxx>
#include <svx/sdr/overlay/overlayselection.hxx>
#include "overlayrangesoutline.hxx"

#include <memory>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <comphelper/string.hxx>
#include <osl/diagnose.h>
#include <paintfrm.hxx>
#include <PostItMgr.hxx>
#include <SwGrammarMarkUp.hxx>
#include <docsh.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <o3tl/string_view.hxx>
#include <tools/json_writer.hxx>
#include <cellfrm.hxx>
#include <wrtsh.hxx>
#include <textcontentcontrol.hxx>
#include <dropdowncontentcontrolbutton.hxx>
#include <datecontentcontrolbutton.hxx>
#include <FrameControlsManager.hxx>

// Here static members are defined. They will get changed on alteration of the
// MapMode. This is done so that on ShowCursor the same size does not have to be
// expensively determined again and again.

tools::Long SwSelPaintRects::s_nPixPtX = 0;
tools::Long SwSelPaintRects::s_nPixPtY = 0;
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
        if( m_pCursorShell->VisArea().Overlaps( m_pCursorShell->m_aCharRect ) || comphelper::LibreOfficeKit::isActive() )
            SetPosAndShow(nullptr);
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

namespace
{

// Build JSON message to be sent to Online
OString buildHyperlinkJSON(const OUString& sText, const OUString& sLink)
{
    boost::property_tree::ptree aTree;
    aTree.put("text", sText);
    aTree.put("link", sLink);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree, false);

    return OString(o3tl::trim(aStream.str()));
}

}

void SwVisibleCursor::SetPosAndShow(SfxViewShell const * pViewShell)
{
    SwRect aRect;
    tools::Long nTmpY = m_pCursorShell->m_aCursorHeight.getY();
    if( 0 > nTmpY )
    {
        nTmpY = -nTmpY;
        m_aTextCursor.SetOrientation( 900_deg10 );
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
    const SwCursor* pTmpCursor = m_pCursorShell->GetCursor_();

    if ( pTmpCursor && !m_pCursorShell->IsOverwriteCursor() )
    {
        SwNode& rNode = pTmpCursor->GetPoint()->GetNode();
        if( rNode.IsTextNode() )
        {
            const SwTextNode& rTNd = *rNode.GetTextNode();
            const SwFrame* pFrame = rTNd.getLayoutFrame(m_pCursorShell->GetLayout(), nullptr, nullptr);
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
                        tools::Long nSize = pOut->GetSettings().GetStyleSettings().GetCursorSize();
                        Size aSize( nSize, nSize );
                        aSize = pOut->PixelToLogic( aSize );
                        aRect.Left( aRect.Left() - aSize.Width() );
                    }
                }
            }
        }
    }

    if( aRect.Height())
    {
        ::SwCalcPixStatics( m_pCursorShell->GetOut() );

        // Disable pixel alignment when tiled rendering, so that twip values of
        // the cursor don't depend on statics.
        if (!comphelper::LibreOfficeKit::isActive())
            ::SwAlignRect( aRect, static_cast<SwViewShell const *>(m_pCursorShell), m_pCursorShell->GetOut() );
    }
    if( !m_pCursorShell->IsOverwriteCursor() || m_bIsDragCursor ||
        m_pCursorShell->IsSelection() )
        aRect.Width( 0 );

    m_aTextCursor.SetSize( aRect.SSize() );

    m_aTextCursor.SetPos( aRect.Pos() );

    bool bPostItActive = false;
    SwView* pView = dynamic_cast<SwView*>(m_pCursorShell->GetSfxViewShell());
    if (pView)
    {
        if (SwPostItMgr* pPostItMgr = pView->GetPostItMgr())
            bPostItActive = pPostItMgr->GetActiveSidebarWin() != nullptr;
    }

    if (comphelper::LibreOfficeKit::isActive() && !bPostItActive)
    {
        // notify about page number change (if that happened)
        sal_uInt16 nPage, nVirtPage;
        // bCalcFrame=false is important to avoid calculating the layout when
        // we're in the middle of doing that already.
        const_cast<SwCursorShell*>(m_pCursorShell)->GetPageNum(nPage, nVirtPage, /*bAtCursorPos=*/true, /*bCalcFrame=*/false);
        if (nPage != m_nPageLastTime)
        {
            m_nPageLastTime = nPage;
            OString aPayload = OString::number(nPage - 1);
            m_pCursorShell->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_SET_PART, aPayload);
        }

        // This may get called often, so instead of sending data on each update, just notify
        // that there's been an update, and the other side will pull the data using
        // getLOKPayload() when it decides to.
        m_aLastLOKRect = aRect;
        if (pViewShell)
        {
            if (pViewShell == m_pCursorShell->GetSfxViewShell())
            {
                SfxLokHelper::notifyUpdatePerViewId(pViewShell, LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR);
            }
            else
            {
                SfxLokHelper::notifyUpdatePerViewId(pViewShell, m_pCursorShell->GetSfxViewShell(), pViewShell,
                    LOK_CALLBACK_INVALIDATE_VIEW_CURSOR);
            }
        }
        else
        {
            SfxLokHelper::notifyUpdatePerViewId(m_pCursorShell->GetSfxViewShell(), SfxViewShell::Current(),
                m_pCursorShell->GetSfxViewShell(), LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR);
            SfxLokHelper::notifyOtherViewsUpdatePerViewId(m_pCursorShell->GetSfxViewShell(), LOK_CALLBACK_INVALIDATE_VIEW_CURSOR);
        }
    }

    if ( m_pCursorShell->IsCursorReadonly() && !m_pCursorShell->GetViewOptions()->IsSelectionInReadonly() )
        return;

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

std::optional<OString> SwVisibleCursor::getLOKPayload(int nType, int nViewId) const
{
    assert(nType == LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR || nType == LOK_CALLBACK_INVALIDATE_VIEW_CURSOR);
    if (comphelper::LibreOfficeKit::isActive())
    {
        SwRect aRect = m_aLastLOKRect;

        // notify about the cursor position & size
        tools::Rectangle aSVRect(aRect.Pos().getX(), aRect.Pos().getY(), aRect.Pos().getX() + aRect.SSize().Width(), aRect.Pos().getY() + aRect.SSize().Height());
        OString sRect = aSVRect.toString();

        if(nType == LOK_CALLBACK_INVALIDATE_VIEW_CURSOR)
            return SfxLokHelper::makePayloadJSON(m_pCursorShell->GetSfxViewShell(), nViewId, "rectangle", sRect);

        // is cursor at a misspelled word ?
        bool bIsWrong = false;
        SwView* pView = dynamic_cast<SwView*>(m_pCursorShell->GetSfxViewShell());
        if (pView && pView->GetWrtShellPtr())
        {
            const SwViewOption* pVOpt = pView->GetWrtShell().GetViewOptions();
            if(pVOpt && pVOpt->IsOnlineSpell())
            {
                SwPaM* pCursor = m_pCursorShell->GetCursor();
                SwPosition aPos(*pCursor->GetPoint());
                Point aPt = aRect.Pos();
                SwCursorMoveState eTmpState(CursorMoveState::SetOnlyText);
                SwTextNode *pNode = nullptr;
                if (m_pCursorShell->GetLayout()->GetModelPositionForViewPoint(&aPos, aPt, &eTmpState))
                    pNode = aPos.GetNode().GetTextNode();
                if (pNode && !pNode->IsInProtectSect())
                {
                    sal_Int32 nBegin = aPos.GetContentIndex();
                    sal_Int32 nLen = 1;

                    SwWrongList *pWrong = pNode->GetWrong();
                    if (!pWrong)
                        pWrong = pNode->GetGrammarCheck();
                    if (pWrong)
                        bIsWrong = pWrong->InWrongWord(nBegin,nLen) && !pNode->IsSymbolAt(nBegin);
                }
            }
        }

        OString sHyperlink;
        SwContentAtPos aContentAtPos(IsAttrAtPos::InetAttr);
        bool bIsSelection = m_pCursorShell->IsSelection();

        if (const_cast<SwCursorShell*>(m_pCursorShell)->GetContentAtPos(aRect.Pos(), aContentAtPos))
        {
            const SwFormatINetFormat* pItem = static_cast<const SwFormatINetFormat*>(aContentAtPos.aFnd.pAttr);
            sHyperlink = buildHyperlinkJSON(aContentAtPos.sStr, pItem->GetValue());
        }
        else if (bIsSelection)
        {
            SwWrtShell* pShell = m_pCursorShell->GetDoc()->GetDocShell()->GetWrtShell();

            if (pShell)
            {
                SfxItemSetFixed<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>
                     aSet(m_pCursorShell->GetSfxViewShell()->GetPool());
                pShell->GetCurAttr(aSet);
                if(SfxItemState::SET <= aSet.GetItemState( RES_TXTATR_INETFMT ))
                {
                    sHyperlink = buildHyperlinkJSON(m_pCursorShell->GetSelText(),
                                                    aSet.GetItem(RES_TXTATR_INETFMT)->GetValue());
                }
            }
        }

        return SfxLokHelper::makeVisCursorInvalidation(nViewId, sRect, bIsWrong, sHyperlink);
    }
    else
        abort();
}

const vcl::Cursor& SwVisibleCursor::GetTextCursor() const
{
    return m_aTextCursor;
}

SwSelPaintRects::SwSelPaintRects( const SwCursorShell& rCSh )
    : m_pCursorShell( &rCSh )
#if HAVE_FEATURE_DESKTOP
    , m_bShowTextInputFieldOverlay(true)
    , m_bShowContentControlOverlay(true)
#endif
{
}

SwSelPaintRects::~SwSelPaintRects()
{
    Hide();
    m_pContentControlButton.disposeAndClear();
}

void SwSelPaintRects::swapContent(SwSelPaintRects& rSwap)
{
    SwRects::swap(rSwap);

#if HAVE_FEATURE_DESKTOP
    // #i75172# also swap m_pCursorOverlay
    std::swap(m_pCursorOverlay, rSwap.m_pCursorOverlay);
    std::swap(m_bShowTextInputFieldOverlay, rSwap.m_bShowTextInputFieldOverlay);
    std::swap(m_pTextInputFieldOverlay, rSwap.m_pTextInputFieldOverlay);
    std::swap(m_bShowContentControlOverlay, rSwap.m_bShowContentControlOverlay);
    std::swap(m_pContentControlOverlay, rSwap.m_pContentControlOverlay);
#endif
}

void SwSelPaintRects::Hide()
{
#if HAVE_FEATURE_DESKTOP
    m_pCursorOverlay.reset();
    m_pTextInputFieldOverlay.reset();
    m_pContentControlOverlay.reset();
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
    const SwContentNode* pNode = rPosition.GetNode().GetContentNode();
    std::pair<Point, bool> const tmp(rPoint, true);
    const SwContentFrame* pFrame = pNode->getLayoutFrame(
            pNode->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(),
            &rPosition, &tmp);
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

    if(!(pView && pView->PaintWindowCount()))
        return;

    // reset rects
    SwRects::clear();
    FillRects();

#if HAVE_FEATURE_DESKTOP
    // get new rects
    std::vector< basegfx::B2DRange > aNewRanges;
    aNewRanges.reserve(size());
    for(size_type a = 0; a < size(); ++a)
    {
        const SwRect aNextRect((*this)[a]);
        const tools::Rectangle aPntRect(aNextRect.SVRect());

        aNewRanges.emplace_back(
            aPntRect.Left(), aPntRect.Top(),
            aPntRect.Right() + 1, aPntRect.Bottom() + 1);
    }

    if (m_pCursorOverlay)
    {
        if(!aNewRanges.empty())
        {
            static_cast<sdr::overlay::OverlaySelection*>(m_pCursorOverlay.get())->setRanges(std::move(aNewRanges));
        }
        else
        {
            m_pCursorOverlay.reset();
        }
    }
    else if(!empty())
    {
        SdrPaintWindow* pCandidate = pView->GetPaintWindow(0);
        const rtl::Reference< sdr::overlay::OverlayManager >& xTargetOverlay = pCandidate->GetOverlayManager();

        if (xTargetOverlay.is())
        {
            // get the system's highlight color
            const Color aHighlight(SvtOptionsDrawinglayer::getHilightColor());

            // create correct selection
            m_pCursorOverlay.reset( new sdr::overlay::OverlaySelection(
                sdr::overlay::OverlayType::Transparent,
                aHighlight,
                std::move(aNewRanges),
                true) );

            xTargetOverlay->add(*m_pCursorOverlay);
        }
    }

    HighlightInputField();
    HighlightContentControl();
#endif

    // Tiled editing does not expose the draw and writer cursor, it just
    // talks about "the" cursor at the moment. As long as that's true,
    // don't say anything about the Writer cursor till a draw object is
    // being edited.
    if (!comphelper::LibreOfficeKit::isActive() || pView->GetTextEditObject())
        return;

    // If pSelectionRectangles is set, we're just collecting the text selections -> don't emit start/end.
    if (!empty() && !pSelectionRectangles)
    {
        SwRect aStartRect;
        SwRect aEndRect;
        FillStartEnd(aStartRect, aEndRect);

        if (aStartRect.HasArea())
            SfxLokHelper::notifyUpdate(GetShell()->GetSfxViewShell(), LOK_CALLBACK_TEXT_SELECTION_START);
        if (aEndRect.HasArea())
            SfxLokHelper::notifyUpdate(GetShell()->GetSfxViewShell(), LOK_CALLBACK_TEXT_SELECTION_END);
    }

    std::vector<OString> aRect;
    aRect.reserve(size());
    for (size_type i = 0; i < size(); ++i)
    {
        const SwRect& rRect = (*this)[i];
        aRect.push_back(rRect.SVRect().toString());
    }
    OString sRect = comphelper::string::join("; ", aRect);
    if (!pSelectionRectangles)
    {
        SfxLokHelper::notifyUpdate(GetShell()->GetSfxViewShell(),LOK_CALLBACK_TEXT_SELECTION);
        SfxLokHelper::notifyOtherViewsUpdatePerViewId(GetShell()->GetSfxViewShell(), LOK_CALLBACK_TEXT_VIEW_SELECTION);
    }
    else
        pSelectionRectangles->push_back(sRect);
}

std::optional<OString> SwSelPaintRects::getLOKPayload(int nType, int nViewId) const
{
    switch( nType )
    {
        case LOK_CALLBACK_TEXT_SELECTION_START:
        case LOK_CALLBACK_TEXT_SELECTION_END:
        {
            // The selection may be a complex polygon, emit the logical
            // start/end cursor rectangle of the selection as separate
            // events, if there is a real selection.
            // This can be used to easily show selection handles on the
            // client side.
            SwRect aStartRect;
            SwRect aEndRect;
            FillStartEnd(aStartRect, aEndRect);

            // no selection rect
            if (!size())
                return {};

            if( nType == LOK_CALLBACK_TEXT_SELECTION_START )
            {
                if (aStartRect.HasArea())
                    return aStartRect.SVRect().toString();
                return {};
            }
            else // LOK_CALLBACK_TEXT_SELECTION_END
            {
                if (aEndRect.HasArea())
                    return aEndRect.SVRect().toString();
                return {};
            }
        }
        break;
        case LOK_CALLBACK_TEXT_SELECTION:
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
        {
            std::vector<OString> aRect;
            aRect.reserve(size());
            for (size_type i = 0; i < size(); ++i)
            {
                const SwRect& rRect = (*this)[i];
                aRect.push_back(rRect.SVRect().toString());
            }
            OString sRect = comphelper::string::join("; ", aRect);
            if( nType == LOK_CALLBACK_TEXT_SELECTION )
                return sRect;
            else // LOK_CALLBACK_TEXT_VIEW_SELECTION
                return SfxLokHelper::makePayloadJSON(GetShell()->GetSfxViewShell(), nViewId, "selection", sRect);
        }
        break;
    }
    abort();
}

void SwSelPaintRects::HighlightInputField()
{
    std::vector< basegfx::B2DRange > aInputFieldRanges;

    if (m_bShowTextInputFieldOverlay)
    {
        SwTextInputField* pCurTextInputFieldAtCursor =
            dynamic_cast<SwTextInputField*>(SwCursorShell::GetTextFieldAtPos( GetShell()->GetCursor()->Start(), ::sw::GetTextAttrMode::Expand));
        if ( pCurTextInputFieldAtCursor != nullptr )
        {
            SwTextNode* pTextNode = pCurTextInputFieldAtCursor->GetpTextNode();
            std::unique_ptr<SwShellCursor> pCursorForInputTextField(
                new SwShellCursor( *GetShell(), SwPosition( *pTextNode, pCurTextInputFieldAtCursor->GetStart() ) ) );
            pCursorForInputTextField->SetMark();
            pCursorForInputTextField->GetMark()->Assign(*pTextNode, *(pCurTextInputFieldAtCursor->End()) );

            pCursorForInputTextField->FillRects();
            SwRects* pRects = pCursorForInputTextField.get();
            for (const SwRect & rNextRect : *pRects)
            {
                const tools::Rectangle aPntRect(rNextRect.SVRect());

                aInputFieldRanges.emplace_back(
                    aPntRect.Left(), aPntRect.Top(),
                    aPntRect.Right() + 1, aPntRect.Bottom() + 1);
            }
        }
    }

    if ( !aInputFieldRanges.empty() )
    {
        if (m_pTextInputFieldOverlay != nullptr)
        {
            m_pTextInputFieldOverlay->setRanges( std::move(aInputFieldRanges) );
        }
        else
        {
            SdrView* pView = const_cast<SdrView*>(GetShell()->GetDrawView());
            SdrPaintWindow* pCandidate = pView->GetPaintWindow(0);
            const rtl::Reference<sdr::overlay::OverlayManager>& xTargetOverlay = pCandidate->GetOverlayManager();

            if (xTargetOverlay.is())
            {
                // use system's highlight color with decreased luminance as highlight color
                Color aHighlight(SvtOptionsDrawinglayer::getHilightColor());
                aHighlight.DecreaseLuminance( 128 );

                m_pTextInputFieldOverlay.reset( new sw::overlay::OverlayRangesOutline(
                        aHighlight, std::move(aInputFieldRanges) ) );
                xTargetOverlay->add( *m_pTextInputFieldOverlay );
            }
        }
    }
    else
    {
        m_pTextInputFieldOverlay.reset();
    }
}

void SwSelPaintRects::HighlightContentControl()
{
    std::vector<basegfx::B2DRange> aContentControlRanges;
    std::vector<OString> aLOKRectangles;
    SwRect aFirstPortionPaintArea;
    SwRect aLastPortionPaintArea;
    bool bRTL = false;
    std::shared_ptr<SwContentControl> pContentControl;

    if (m_bShowContentControlOverlay)
    {
        const SwPosition* pStart = GetShell()->GetCursor()->Start();
        SwTextNode* pTextNode = pStart->GetNode().GetTextNode();
        SwTextContentControl* pCurContentControlAtCursor = nullptr;
        if (pTextNode)
        {
            // GetTextAttrMode::Parent because this way we highlight when the user will type inside the
            // content control, not outside of it.
            SwTextAttr* pAttr = pTextNode->GetTextAttrAt(
                pStart->GetContentIndex(), RES_TXTATR_CONTENTCONTROL, ::sw::GetTextAttrMode::Parent);
            if (pAttr)
            {
                pCurContentControlAtCursor = static_txtattr_cast<SwTextContentControl*>(pAttr);
            }
        }
        if (pCurContentControlAtCursor)
        {
            auto pCursorForContentControl = std::make_unique<SwShellCursor>(
                *GetShell(), SwPosition(*pTextNode, pCurContentControlAtCursor->GetStart()));
            pCursorForContentControl->SetMark();
            pCursorForContentControl->GetMark()->Assign(
                *pTextNode, *(pCurContentControlAtCursor->End()));

            pCursorForContentControl->FillRects();
            SwRects* pRects = pCursorForContentControl.get();
            for (const auto& rRect : *pRects)
            {
                tools::Rectangle aRect(rRect.SVRect());

                aContentControlRanges.emplace_back(aRect.Left(), aRect.Top(), aRect.Right() + 1,
                                                   aRect.Bottom() + 1);
                if (comphelper::LibreOfficeKit::isActive())
                {
                    aLOKRectangles.push_back(aRect.toString());
                }
            }

            if (!pRects->empty())
            {
                aFirstPortionPaintArea = (*pRects)[0];
                aLastPortionPaintArea = (*pRects)[pRects->size() - 1];
            }
            pContentControl = pCurContentControlAtCursor->GetContentControl().GetContentControl();

            // The layout knows if the text node is RTL (either set directly, or inherited from the
            // environment).
            SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aFrames(*pTextNode);
            SwTextFrame* pFrame = aFrames.First();
            if (pFrame)
            {
                bRTL = pFrame->IsRightToLeft();
            }
        }
    }

    auto pWrtShell = dynamic_cast<const SwWrtShell*>(GetShell());
    if (!aContentControlRanges.empty())
    {
        if (comphelper::LibreOfficeKit::isActive())
        {
            OString aPayload = comphelper::string::join("; ", aLOKRectangles);
            tools::JsonWriter aJson;
            aJson.put("action", "show");
            aJson.put("rectangles", aPayload);

            if (pContentControl && (pContentControl->GetComboBox() || pContentControl->GetDropDown()))
            {
                tools::ScopedJsonWriterArray aItems = aJson.startArray("items");
                for (const auto& rItem : pContentControl->GetListItems())
                {
                    aJson.putSimpleValue(rItem.ToString());
                }
            }

            if (pContentControl && pContentControl->GetDate())
            {
                aJson.put("date", "true");
            }

            if (pContentControl && !pContentControl->GetAlias().isEmpty())
            {
                aJson.put("alias", pContentControl->GetAlias());
            }

            OString pJson(aJson.finishAndGetAsOString());
            GetShell()->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_CONTENT_CONTROL, pJson);
        }
        if (m_pContentControlOverlay)
        {
            m_pContentControlOverlay->setRanges(std::move(aContentControlRanges));
        }
        else
        {
            SdrView* pView = const_cast<SdrView*>(GetShell()->GetDrawView());
            SdrPaintWindow* pCandidate = pView->GetPaintWindow(0);
            const rtl::Reference<sdr::overlay::OverlayManager>& xTargetOverlay
                = pCandidate->GetOverlayManager();

            if (xTargetOverlay.is())
            {
                // Use the system's highlight color with decreased luminance as highlight color.
                Color aHighlight(SvtOptionsDrawinglayer::getHilightColor());
                aHighlight.DecreaseLuminance(128);

                m_pContentControlOverlay.reset(new sw::overlay::OverlayRangesOutline(
                    aHighlight, std::move(aContentControlRanges)));
                xTargetOverlay->add(*m_pContentControlOverlay);
            }
        }

        if (pContentControl && (pContentControl->GetComboBox() || pContentControl->GetDropDown()))
        {
            if (pWrtShell)
            {
                auto& rEditWin = const_cast<SwEditWin&>(pWrtShell->GetView().GetEditWin());
                if (m_pContentControlButton
                    && m_pContentControlButton->GetContentControl() != pContentControl)
                {
                    m_pContentControlButton.disposeAndClear();
                }
                if (!m_pContentControlButton)
                {
                    m_pContentControlButton = VclPtr<SwDropDownContentControlButton>::Create(
                        &rEditWin, pContentControl);
                }
                m_pContentControlButton->SetRTL(bRTL);
                if (bRTL)
                {
                    m_pContentControlButton->CalcPosAndSize(aFirstPortionPaintArea);
                }
                else
                {
                    m_pContentControlButton->CalcPosAndSize(aLastPortionPaintArea);
                }
                m_pContentControlButton->Show();
            }
        }
        if (pContentControl && pContentControl->GetDate())
        {
            if (pWrtShell)
            {
                auto& rEditWin = const_cast<SwEditWin&>(pWrtShell->GetView().GetEditWin());
                if (m_pContentControlButton
                    && m_pContentControlButton->GetContentControl() != pContentControl)
                {
                    m_pContentControlButton.disposeAndClear();
                }
                if (!m_pContentControlButton)
                {
                    m_pContentControlButton = VclPtr<SwDateContentControlButton>::Create(
                        &rEditWin, pContentControl, pWrtShell->GetDoc()->GetNumberFormatter());
                }
                m_pContentControlButton->CalcPosAndSize(aLastPortionPaintArea);
                m_pContentControlButton->Show();
            }
        }

        if (pWrtShell)
        {
            auto& rEditWin = const_cast<SwEditWin&>(pWrtShell->GetView().GetEditWin());
            SwFrameControlsManager& rMngr = rEditWin.GetFrameControlsManager();
            if (pContentControl && !pContentControl->GetAlias().isEmpty())
            {
                Point aTopLeftPixel = rEditWin.LogicToPixel(aFirstPortionPaintArea.TopLeft());
                rMngr.SetContentControlAliasButton(pContentControl.get(), aTopLeftPixel);
            }
            else
            {
                rMngr.HideControls(FrameControlType::ContentControl);
            }
        }
    }
    else
    {
        if (comphelper::LibreOfficeKit::isActive() && m_pContentControlOverlay)
        {
            tools::JsonWriter aJson;
            aJson.put("action", "hide");
            OString pJson(aJson.finishAndGetAsOString());
            GetShell()->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_CONTENT_CONTROL, pJson);
        }
        m_pContentControlOverlay.reset();

        if (m_pContentControlButton)
        {
            m_pContentControlButton.disposeAndClear();
        }

        if (pWrtShell)
        {
            auto& rEditWin = const_cast<SwEditWin&>(pWrtShell->GetView().GetEditWin());
            SwFrameControlsManager& rMngr = rEditWin.GetFrameControlsManager();
            rMngr.HideControls(FrameControlType::ContentControl);
        }
    }
}

const VclPtr<SwContentControlButton>& SwSelPaintRects::GetContentControlButton() const
{
    return m_pContentControlButton;
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
    if( !(GetShell()->m_bVisPortChgd && 0 != ( nSz = size())) )
        return;

    SwSelPaintRects::Get1PixelInLogic( *GetShell() );
    iterator it = begin();
    for( ; nSz--; ++it )
    {
        SwRect& rRectIt = *it;
        if( rRectIt.Right() == GetShell()->m_aOldRBPos.X() )
            rRectIt.AddRight( s_nPixPtX );
        if( rRectIt.Bottom() == GetShell()->m_aOldRBPos.Y() )
            rRectIt.AddBottom( s_nPixPtY );
    }
}

// check current MapMode of the shell and set possibly the static members.
// Optional set the parameters pX, pY
void SwSelPaintRects::Get1PixelInLogic( const SwViewShell& rSh,
                                        tools::Long* pX, tools::Long* pY )
{
    const OutputDevice* pOut = rSh.GetWin()->GetOutDev();
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
    : SwCursor(rPos,nullptr)
    , SwSelPaintRects(rCShell)
    , m_pInitialPoint(SwPaM::GetPoint())
{}

SwShellCursor::SwShellCursor(
    const SwCursorShell& rCShell,
    const SwPosition &rPos,
    const Point& rPtPos,
    SwPaM* pRing )
    : SwCursor(rPos, pRing)
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
        GetPoint()->GetNode().IsContentNode() &&
        GetPoint()->GetNode().GetContentNode()->getLayoutFrame( GetShell()->GetLayout() ) &&
        (GetMark()->GetNode() == GetPoint()->GetNode() ||
        (GetMark()->GetNode().IsContentNode() &&
         GetMark()->GetNode().GetContentNode()->getLayoutFrame( GetShell()->GetLayout() ) )   ))
    {
        GetShell()->GetLayout()->CalcFrameRects(*this, *this);
    }
}

void SwShellCursor::Show(SfxViewShell const * pViewShell)
{
    std::vector<OString> aSelectionRectangles;
    for(SwPaM& rPaM : GetRingContainer())
    {
        SwShellCursor* pShCursor = dynamic_cast<SwShellCursor*>(&rPaM);
        if(pShCursor)
            pShCursor->SwSelPaintRects::Show(&aSelectionRectangles);
    }

    if (!comphelper::LibreOfficeKit::isActive())
        return;

    std::vector<OString> aRect;
    for (const OString & rSelectionRectangle : aSelectionRectangles)
    {
        if (rSelectionRectangle.isEmpty())
            continue;
        aRect.push_back(rSelectionRectangle);
    }
    OString sRect = comphelper::string::join("; ", aRect);
    if (pViewShell)
    {
        // Just notify pViewShell about our existing selection.
        if (pViewShell != GetShell()->GetSfxViewShell())
            SfxLokHelper::notifyOtherView(GetShell()->GetSfxViewShell(), pViewShell, LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection", sRect);
    }
    else
    {
        GetShell()->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, sRect);
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
    SvxSearchDialog* pDlg = SwView::GetSearchDialog();
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
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pDlg->getDialog(), "modules/swriter/ui/asksearchdialog.ui"));
        std::unique_ptr<weld::MessageDialog> xDialog(xBuilder->weld_message_dialog("AskSearchDialog"));
        nRet = xDialog->run();
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
        // otherwise from the Basic, and then switch to RET_YES
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
                            &GetPtPos(), GetShell()->GetUpDownX(),
                            *GetShell()->GetLayout());
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
    : SwCursor(rPos,nullptr), SwShellCursor(rCursorSh, rPos), SwTableCursor(rPos)
{
}

SwShellTableCursor::SwShellTableCursor( const SwCursorShell& rCursorSh,
                    const SwPosition& rMkPos, const Point& rMkPt,
                    const SwPosition& rPtPos, const Point& rPtPt )
    : SwCursor(rPtPos,nullptr), SwShellCursor(rCursorSh, rPtPos), SwTableCursor(rPtPos)
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
    if (m_SelectedBoxes.empty() || m_bParked || !GetPoint()->GetNodeIndex())
        return;

    bool bStart = true;
    SwRegionRects aReg( comphelper::LibreOfficeKit::isActive()
        ? GetShell()->getIDocumentLayoutAccess().GetCurrentLayout()->getFrameArea()
        : GetShell()->VisArea() );
    SwNodes& rNds = GetDoc().GetNodes();
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

        std::pair<Point, bool> const tmp(GetSttPos(), false);
        SwFrame* pFrame = pCNd->getLayoutFrame(GetShell()->GetLayout(), nullptr, &tmp);
        while( pFrame && !pFrame->IsCellFrame() )
            pFrame = pFrame->GetUpper();

        OSL_ENSURE( pFrame, "Node not in a table" );

        while ( pFrame )
        {
            if( aReg.GetOrigin().Overlaps( pFrame->getFrameArea() ) )
            {
                aReg -= pFrame->getFrameArea();
                if (bStart)
                {
                    bStart = false;
                    m_aStart = SwRect(pFrame->getFrameArea().Left(), pFrame->getFrameArea().Top(), 1, pFrame->getFrameArea().Height());
                }
            }

            pEndFrame = pFrame;
            pFrame = pFrame->GetNextCellLeaf();
        }
    }
    if (pEndFrame)
        m_aEnd = SwRect(pEndFrame->getFrameArea().Right(), pEndFrame->getFrameArea().Top(), 1, pEndFrame->getFrameArea().Height());
    aReg.Invert();
    insert( begin(), aReg.begin(), aReg.end() );
}

void SwShellTableCursor::FillStartEnd(SwRect& rStart, SwRect& rEnd) const
{
    rStart = m_aStart;
    rEnd = m_aEnd;
}

// Check if the SPoint is within the Table-SSelection.
bool SwShellTableCursor::Contains( const Point& rPt ) const
{
    // Calculate the new rectangles. If the cursor is still "parked" do nothing
    if (m_SelectedBoxes.empty() || m_bParked || !GetPoint()->GetNodeIndex())
        return false;

    SwNodes& rNds = GetDoc().GetNodes();
    for (size_t n = 0; n < m_SelectedBoxes.size(); ++n)
    {
        SwNodeIndex aIdx( *m_SelectedBoxes[n]->GetSttNd() );
        SwContentNode* pCNd = rNds.GoNextSection( &aIdx, true, false );
        if( !pCNd )
            continue;

        std::pair<Point, bool> const tmp(GetPtPos(), true);
        SwFrame* pFrame = pCNd->getLayoutFrame(GetShell()->GetLayout(), nullptr, &tmp);
        while( pFrame && !pFrame->IsCellFrame() )
            pFrame = pFrame->GetUpper();
        OSL_ENSURE( pFrame, "Node not in a table" );
        if( pFrame && pFrame->getFrameArea().Contains( rPt ) )
            return true;

        for ( SwCellFrame* pCellFrame = static_cast<SwCellFrame*>(pFrame); pCellFrame; pCellFrame = pCellFrame->GetFollowCell() )
        {
            if( pCellFrame->getFrameArea().Contains( rPt ) )
                return true;
        }
    }
    return false;
}

bool SwShellTableCursor::IsAtValidPos( bool bPoint ) const
{
    return SwShellCursor::IsAtValidPos( bPoint );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
