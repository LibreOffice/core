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

#include "impedit.hxx"
#include <sal/log.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <tools/poly.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <comphelper/lok.hxx>
#include <editeng/flditem.hxx>
#include <svl/intitem.hxx>
#include <vcl/inputctx.hxx>
#include <vcl/transfer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/string.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/lokhelper.hxx>
#include <boost/property_tree/ptree.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

#define SCRLRANGE   20  // Scroll 1/20 of the width/height, when in QueryDrop

static void lcl_AllignToPixel( Point& rPoint, OutputDevice const * pOutDev, short nDiffX, short nDiffY )
{
    rPoint = pOutDev->LogicToPixel( rPoint );

    if ( nDiffX )
        rPoint.AdjustX(nDiffX );
    if ( nDiffY )
        rPoint.AdjustY(nDiffY );

    rPoint = pOutDev->PixelToLogic( rPoint );
}

LOKSpecialPositioning::LOKSpecialPositioning(const ImpEditView& rImpEditView, MapUnit eUnit,
                                             const tools::Rectangle& rOutputArea,
                                             const Point& rVisDocStartPos) :
                                             mrImpEditView(rImpEditView),
                                             maOutArea(rOutputArea),
                                             maVisDocStartPos(rVisDocStartPos),
                                             meUnit(eUnit)
{
}

void LOKSpecialPositioning::ReInit(MapUnit eUnit, const tools::Rectangle& rOutputArea, const Point& rVisDocStartPos)
{
    meUnit = eUnit;
    maOutArea = rOutputArea;
    maVisDocStartPos = rVisDocStartPos;
}

void LOKSpecialPositioning::SetOutputArea(const tools::Rectangle& rOutputArea)
{
    maOutArea = rOutputArea;
}

const tools::Rectangle& LOKSpecialPositioning::GetOutputArea() const
{
    return maOutArea;
}

void LOKSpecialPositioning::SetVisDocStartPos(const Point& rVisDocStartPos)
{
    maVisDocStartPos = rVisDocStartPos;
}

tools::Rectangle LOKSpecialPositioning::GetVisDocArea() const
{
    return tools::Rectangle(GetVisDocLeft(), GetVisDocTop(), GetVisDocRight(), GetVisDocBottom());
}

bool LOKSpecialPositioning::IsVertical() const
{
    return mrImpEditView.IsVertical();
}

bool LOKSpecialPositioning::IsTopToBottom() const
{
    return mrImpEditView.IsTopToBottom();
}

Point LOKSpecialPositioning::GetWindowPos(const Point& rDocPos, MapUnit eDocPosUnit) const
{
    const Point aDocPos = convertUnit(rDocPos, eDocPosUnit);
    Point aPoint;
    if ( !IsVertical() )
    {
        aPoint.setX(aDocPos.X() + maOutArea.Left() - GetVisDocLeft());
        aPoint.setY(aDocPos.Y() + maOutArea.Top() - GetVisDocTop());
    }
    else
    {
        if (IsTopToBottom())
        {
            aPoint.setX(maOutArea.Right() - aDocPos.Y() + GetVisDocTop());
            aPoint.setY(aDocPos.X() + maOutArea.Top() - GetVisDocLeft());
        }
        else
        {
            aPoint.setX(maOutArea.Left() + aDocPos.Y() - GetVisDocTop());
            aPoint.setY(maOutArea.Bottom() - aDocPos.X() + GetVisDocLeft());
        }
    }

    return aPoint;
}

tools::Rectangle LOKSpecialPositioning::GetWindowPos(const tools::Rectangle& rDocRect, MapUnit eDocRectUnit) const
{
    const tools::Rectangle aDocRect = convertUnit(rDocRect, eDocRectUnit);
    Point aPos(GetWindowPos(aDocRect.TopLeft(), meUnit));
    Size aSz = aDocRect.GetSize();
    tools::Rectangle aRect;
    if (!IsVertical())
    {
        aRect = tools::Rectangle(aPos, aSz);
    }
    else
    {
        Point aNewPos(aPos.X() - aSz.Height(), aPos.Y());
        // coverity[swapped_arguments : FALSE] - this is in the correct order
        aRect = tools::Rectangle(aNewPos, Size(aSz.Height(), aSz.Width()));
    }
    return aRect;
}

Point LOKSpecialPositioning::convertUnit(const Point& rPos, MapUnit ePosUnit) const
{
    if (ePosUnit == meUnit)
        return rPos;

    return OutputDevice::LogicToLogic(rPos, MapMode(ePosUnit), MapMode(meUnit));
}

tools::Rectangle LOKSpecialPositioning::convertUnit(const tools::Rectangle& rRect, MapUnit eRectUnit) const
{
    if (eRectUnit == meUnit)
        return rRect;

    return OutputDevice::LogicToLogic(rRect, MapMode(eRectUnit), MapMode(meUnit));
}

Point LOKSpecialPositioning::GetRefPoint() const
{
    return maOutArea.TopLeft();
}

//  class ImpEditView

ImpEditView::ImpEditView( EditView* pView, EditEngine* pEng, vcl::Window* pWindow ) :
    pEditView(pView),
    mpViewShell(nullptr),
    mpOtherShell(nullptr),
    pEditEngine(pEng),
    pOutWin(pWindow),
    nInvMore(1),
    nControl(EVControlBits::AUTOSCROLL | EVControlBits::ENABLEPASTE),
    nTravelXPos(TRAVEL_X_DONTKNOW),
    nExtraCursorFlags(GetCursorFlags::NONE),
    nCursorBidiLevel(CURSOR_BIDILEVEL_DONTKNOW),
    nScrollDiffX(0),
    bReadOnly(false),
    bClickedInSelection(false),
    bActiveDragAndDropListener(false),
    aOutArea( Point(), pEng->GetPaperSize() ),
    eSelectionMode(EESelectionMode::Std),
    eAnchorMode(EEAnchorMode::TopLeft),
    mpEditViewCallbacks(nullptr),
    mbBroadcastLOKViewCursor(comphelper::LibreOfficeKit::isActive()),
    mbSuppressLOKMessages(false)
{
    aEditSelection.Min() = pEng->GetEditDoc().GetStartPaM();
    aEditSelection.Max() = pEng->GetEditDoc().GetEndPaM();

    SelectionChanged();
}

ImpEditView::~ImpEditView()
{
    RemoveDragAndDropListeners();

    if ( pOutWin && ( pOutWin->GetCursor() == pCursor.get() ) )
        pOutWin->SetCursor( nullptr );
}

void ImpEditView::SetBackgroundColor( const Color& rColor )
{
    mxBackgroundColor = rColor;
}

void ImpEditView::RegisterViewShell(OutlinerViewShell* pViewShell)
{
    mpViewShell = pViewShell;
}

void ImpEditView::RegisterOtherShell(OutlinerViewShell* pOtherShell)
{
    mpOtherShell = pOtherShell;
}

const OutlinerViewShell* ImpEditView::GetViewShell() const
{
    return mpViewShell;
}

void ImpEditView::SetEditSelection( const EditSelection& rEditSelection )
{
    // set state before notification
    aEditSelection = rEditSelection;

    SelectionChanged();

    if (comphelper::LibreOfficeKit::isActive())
        // Tiled rendering: selections are only painted when we are in selection mode.
        pEditEngine->SetInSelectionMode(aEditSelection.HasRange());

    if ( pEditEngine->pImpEditEngine->GetNotifyHdl().IsSet() )
    {
        const EditDoc& rDoc = pEditEngine->GetEditDoc();
        const EditPaM pmEnd = rDoc.GetEndPaM();
        EENotifyType eNotifyType;
        if (rDoc.Count() > 1 &&
            pmEnd == rEditSelection.Min() &&
            pmEnd == rEditSelection.Max())//if move cursor to the last para.
        {
            eNotifyType = EE_NOTIFY_TEXTVIEWSELECTIONCHANGED_ENDD_PARA;
        }
        else
        {
            eNotifyType = EE_NOTIFY_TEXTVIEWSELECTIONCHANGED;
        }
        EENotify aNotify( eNotifyType );
        pEditEngine->pImpEditEngine->GetNotifyHdl().Call( aNotify );
    }
    if(pEditEngine->pImpEditEngine->IsFormatted())
    {
        EENotify aNotify(EE_NOTIFY_PROCESSNOTIFICATIONS);
        pEditEngine->pImpEditEngine->GetNotifyHdl().Call(aNotify);
    }
}

/// Translate absolute <-> relative twips: LOK wants absolute coordinates as output and gives absolute coordinates as input.
static void lcl_translateTwips(vcl::Window const & rParent, vcl::Window& rChild)
{
    // Don't translate if we already have a non-zero origin.
    // This prevents multiple translate calls that negate
    // one another.
    const Point aOrigin = rChild.GetMapMode().GetOrigin();
    if (aOrigin.getX() != 0 || aOrigin.getY() != 0)
        return;

    // Set map mode, so that callback payloads will contain absolute coordinates instead of relative ones.
    Point aOffset(rChild.GetOutOffXPixel() - rParent.GetOutOffXPixel(), rChild.GetOutOffYPixel() - rParent.GetOutOffYPixel());
    if (!rChild.IsMapModeEnabled())
    {
        MapMode aMapMode(rChild.GetMapMode());
        aMapMode.SetMapUnit(MapUnit::MapTwip);
        aMapMode.SetScaleX(rParent.GetMapMode().GetScaleX());
        aMapMode.SetScaleY(rParent.GetMapMode().GetScaleY());
        rChild.SetMapMode(aMapMode);
        rChild.EnableMapMode();
    }
    aOffset = rChild.PixelToLogic(aOffset);
    MapMode aMapMode(rChild.GetMapMode());
    aMapMode.SetOrigin(aOffset);
    aMapMode.SetMapUnit(rParent.GetMapMode().GetMapUnit());
    rChild.SetMapMode(aMapMode);
    rChild.EnableMapMode(false);
}

// EditView never had a central/secure place to react on SelectionChange since
// Selection was changed in many places, often by not using SetEditSelection()
// but (mis)using GetEditSelection() and manipulating this non-const return
// value. Sorted this out now to have such a place, this is needed for safely
// change/update the Selection visualization for enhanced mechanisms
void ImpEditView::SelectionChanged()
{
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
    {
        // use callback to tell about change in selection visualisation
        pCallbacks->EditViewSelectionChange();
    }
}

// This function is also called when a text's font || size is changed. Because its highlight rectangle must be updated.
void ImpEditView::lokSelectionCallback(const std::unique_ptr<tools::PolyPolygon> &pPolyPoly, bool bStartHandleVisible, bool bEndHandleVisible) {
    VclPtr<vcl::Window> pParent = pOutWin->GetParentWithLOKNotifier();
    vcl::Region aRegion( *pPolyPoly );

    if (pParent && pParent->GetLOKWindowId() != 0)
    {
        const tools::Long nX = pOutWin->GetOutOffXPixel() - pParent->GetOutOffXPixel();
        const tools::Long nY = pOutWin->GetOutOffYPixel() - pParent->GetOutOffYPixel();

        std::vector<tools::Rectangle> aRectangles;
        aRegion.GetRegionRectangles(aRectangles);

        std::vector<OString> v;
        for (tools::Rectangle & rRectangle : aRectangles)
        {
            rRectangle = pOutWin->LogicToPixel(rRectangle);
            rRectangle.Move(nX, nY);
            v.emplace_back(rRectangle.toString().getStr());
        }
        OString sRectangle = comphelper::string::join("; ", v);

        const vcl::ILibreOfficeKitNotifier* pNotifier = pParent->GetLOKNotifier();
        std::vector<vcl::LOKPayloadItem> aItems;
        aItems.emplace_back("rectangles", sRectangle);
        aItems.emplace_back("startHandleVisible", OString::boolean(bStartHandleVisible));
        aItems.emplace_back("endHandleVisible", OString::boolean(bEndHandleVisible));
        pNotifier->notifyWindow(pParent->GetLOKWindowId(), "text_selection", aItems);
    }
    else
    {
        pOutWin->Push(PushFlags::MAPMODE);
        if (pOutWin->GetMapMode().GetMapUnit() == MapUnit::MapTwip)
        {
            // Find the parent that is not right
            // on top of us to use its offset.
            vcl::Window* parent = pOutWin->GetParent();
            while (parent &&
                    parent->GetOutOffXPixel() == pOutWin->GetOutOffXPixel() &&
                    parent->GetOutOffYPixel() == pOutWin->GetOutOffYPixel())
            {
                parent = parent->GetParent();
            }

            if (parent)
            {
                lcl_translateTwips(*parent, *pOutWin);
            }
        }

        bool bMm100ToTwip = !mpLOKSpecialPositioning &&
                (pOutWin->GetMapMode().GetMapUnit() == MapUnit::Map100thMM);

        Point aOrigin;
        if (pOutWin->GetMapMode().GetMapUnit() == MapUnit::MapTwip)
            // Writer comments: they use editeng, but are separate widgets.
            aOrigin = pOutWin->GetMapMode().GetOrigin();

        OString sRectangle;
        OString sRefPoint;
        if (mpLOKSpecialPositioning)
            sRefPoint = mpLOKSpecialPositioning->GetRefPoint().toString();

        std::vector<tools::Rectangle> aRectangles;
        aRegion.GetRegionRectangles(aRectangles);

        if (!aRectangles.empty())
        {
            if (pOutWin->IsChart())
            {
                const vcl::Window* pViewShellWindow = mpViewShell->GetEditWindowForActiveOLEObj();
                if (pViewShellWindow && pViewShellWindow->IsAncestorOf(*pOutWin))
                {
                    Point aOffsetPx = pOutWin->GetOffsetPixelFrom(*pViewShellWindow);
                    Point aLogicOffset = pOutWin->PixelToLogic(aOffsetPx);
                    for (tools::Rectangle& rRect : aRectangles)
                        rRect.Move(aLogicOffset.getX(), aLogicOffset.getY());
                }
            }

            std::vector<OString> v;
            for (tools::Rectangle & rRectangle : aRectangles)
            {
                if (bMm100ToTwip)
                    rRectangle = OutputDevice::LogicToLogic(rRectangle, MapMode(MapUnit::Map100thMM), MapMode(MapUnit::MapTwip));
                rRectangle.Move(aOrigin.getX(), aOrigin.getY());
                v.emplace_back(rRectangle.toString().getStr());
            }
            sRectangle = comphelper::string::join("; ", v);

            if (mpLOKSpecialPositioning && !sRectangle.isEmpty())
                sRectangle += ":: " + sRefPoint;

            tools::Rectangle& rStart = aRectangles.front();
            tools::Rectangle aStart(rStart.Left(), rStart.Top(), rStart.Left() + 1, rStart.Bottom());

            OString aPayload = aStart.toString();
            if (mpLOKSpecialPositioning)
                aPayload += ":: " + sRefPoint;

            mpViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_START, aPayload.getStr());

            tools::Rectangle& rEnd = aRectangles.back();
            tools::Rectangle aEnd(rEnd.Right() - 1, rEnd.Top(), rEnd.Right(), rEnd.Bottom());

            aPayload = aEnd.toString();
            if (mpLOKSpecialPositioning)
                aPayload += ":: " + sRefPoint;

            mpViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_END, aPayload.getStr());
        }

        if (mpOtherShell)
        {
            // Another shell wants to know about our existing selection.
            if (mpViewShell != mpOtherShell)
                mpViewShell->NotifyOtherView(mpOtherShell, LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection", sRectangle);
        }
        else
        {
            mpViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, sRectangle.getStr());
            mpViewShell->NotifyOtherViews(LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection", sRectangle);
        }

        pOutWin->Pop();
    }
}

// renamed from DrawSelection to DrawSelectionXOR to better reflect what this
// method was used for: Paint Selection in XOR, change it and again paint it in XOR.
// This can be safely assumed due to the EditView only being capable of painting the
// selection in XOR until today.
// This also means that all places calling DrawSelectionXOR are thoroughly weighted
// and chosen to make this fragile XOR-paint water-proof and thus contain some
// information in this sense.
// Someone thankfully expanded it to collect the SelectionRectangles when called with
// the Region*, see GetSelectionRectangles below.
void ImpEditView::DrawSelectionXOR( EditSelection aTmpSel, vcl::Region* pRegion, OutputDevice* pTargetDevice )
{
    if (getEditViewCallbacks() && !pRegion && !comphelper::LibreOfficeKit::isActive())
    {
        // we are done, do *not* visualize self
        // CAUTION: do not use when comphelper::LibreOfficeKit::isActive()
        // due to event stuff triggered below. That *should* probably be moved
        // to SelectionChanged() which exists now, but I do not know enough about
        // that stuff to do it
        return;
    }

    if ( eSelectionMode == EESelectionMode::Hidden )
        return;

    // It must be ensured before rendering the selection, that the contents of
    // the window is completely valid! Must be here so that in any case if
    // empty, then later on two-Paint Events! Must be done even before the
    // query from bUpdate, if after Invalidate paints still in the queue,
    // but someone switches the update mode!

    // pRegion: When not NULL, then only calculate Region.

    OutputDevice* pTarget;
    if (pTargetDevice)
        pTarget = pTargetDevice;
    else
        pTarget = getEditViewCallbacks() ? &getEditViewCallbacks()->EditViewOutputDevice() : pOutWin;
    bool bClipRegion = pTarget->IsClipRegion();
    vcl::Region aOldRegion = pTarget->GetClipRegion();

    std::unique_ptr<tools::PolyPolygon> pPolyPoly;

    if ( !pRegion && !comphelper::LibreOfficeKit::isActive())
    {
        if ( !pEditEngine->pImpEditEngine->GetUpdateMode() )
            return;
        if ( pEditEngine->pImpEditEngine->IsInUndo() )
            return;

        if ( !aTmpSel.HasRange() )
            return;

        // aTmpOutArea: if OutputArea > Paper width and
        // Text > Paper width ( over large fields )
        tools::Rectangle aTmpOutArea( aOutArea );
        if ( aTmpOutArea.GetWidth() > pEditEngine->pImpEditEngine->GetPaperSize().Width() )
            aTmpOutArea.SetRight( aTmpOutArea.Left() + pEditEngine->pImpEditEngine->GetPaperSize().Width() );
        pTarget->IntersectClipRegion( aTmpOutArea );

        if (pOutWin && pOutWin->GetCursor())
            pOutWin->GetCursor()->Hide();
    }

    if (comphelper::LibreOfficeKit::isActive() || pRegion)
        pPolyPoly.reset(new tools::PolyPolygon);

    DBG_ASSERT( !pEditEngine->IsIdleFormatterActive(), "DrawSelectionXOR: Not formatted!" );
    aTmpSel.Adjust( pEditEngine->GetEditDoc() );

    ContentNode* pStartNode = aTmpSel.Min().GetNode();
    ContentNode* pEndNode = aTmpSel.Max().GetNode();
    const sal_Int32 nStartPara = pEditEngine->GetEditDoc().GetPos(pStartNode);
    const sal_Int32 nEndPara = pEditEngine->GetEditDoc().GetPos(pEndNode);
    if (nStartPara == EE_PARA_NOT_FOUND || nEndPara == EE_PARA_NOT_FOUND)
        return;

    bool bStartHandleVisible = false;
    bool bEndHandleVisible = false;

    for ( sal_Int32 nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        ParaPortion* pTmpPortion = pEditEngine->GetParaPortions().SafeGetObject( nPara );
        if (!pTmpPortion)
        {
            SAL_WARN( "editeng", "Portion in Selection not found!" );
            continue;
        }

        DBG_ASSERT( !pTmpPortion->IsInvalid(), "Portion in Selection not formatted!" );

        if ( !pTmpPortion->IsVisible() || pTmpPortion->IsInvalid() )
            continue;

        tools::Long nParaStart = pEditEngine->GetParaPortions().GetYOffset( pTmpPortion );
        if ( ( nParaStart + pTmpPortion->GetHeight() ) < GetVisDocTop() )
            continue;
        if ( nParaStart > GetVisDocBottom() )
            break;

        sal_uInt16 nStartLine = 0;
        sal_uInt16 nEndLine = pTmpPortion->GetLines().Count() -1;
        if ( nPara == nStartPara )
            nStartLine = pTmpPortion->GetLines().FindLine( aTmpSel.Min().GetIndex(), false );
        if ( nPara == nEndPara )
            nEndLine = pTmpPortion->GetLines().FindLine( aTmpSel.Max().GetIndex(), true );

        for ( sal_uInt16 nLine = nStartLine; nLine <= nEndLine; nLine++ )
        {
            const EditLine& rLine = pTmpPortion->GetLines()[nLine];

            bool bPartOfLine = false;
            sal_Int32 nStartIndex = rLine.GetStart();
            sal_Int32 nEndIndex = rLine.GetEnd();
            if ( ( nPara == nStartPara ) && ( nLine == nStartLine ) && ( nStartIndex != aTmpSel.Min().GetIndex() ) )
            {
                nStartIndex = aTmpSel.Min().GetIndex();
                bPartOfLine = true;
            }
            if ( ( nPara == nEndPara ) && ( nLine == nEndLine ) && ( nEndIndex != aTmpSel.Max().GetIndex() ) )
            {
                nEndIndex = aTmpSel.Max().GetIndex();
                bPartOfLine = true;
            }

            // Can happen if at the beginning of a wrapped line.
            if ( nEndIndex < nStartIndex )
                nEndIndex = nStartIndex;

            tools::Rectangle aTmpRect( pEditEngine->pImpEditEngine->GetEditCursor( pTmpPortion, nStartIndex ) );
            Point aTopLeft( aTmpRect.TopLeft() );
            Point aBottomRight( aTmpRect.BottomRight() );

            aTopLeft.AdjustY(nParaStart );
            aBottomRight.AdjustY(nParaStart );

            // Only paint if in the visible range ...
            if ( aTopLeft.Y() > GetVisDocBottom() )
                break;

            if ( aBottomRight.Y() < GetVisDocTop() )
                continue;

            if ( ( nPara == nStartPara ) && ( nLine == nStartLine ) )
                bStartHandleVisible = true;
            if ( ( nPara == nEndPara ) && ( nLine == nEndLine ) )
                bEndHandleVisible = true;

            // Now that we have Bidi, the first/last index doesn't have to be the 'most outside' position
            if ( !bPartOfLine )
            {
                Range aLineXPosStartEnd = pEditEngine->GetLineXPosStartEnd(pTmpPortion, &rLine);
                aTopLeft.setX( aLineXPosStartEnd.Min() );
                aBottomRight.setX( aLineXPosStartEnd.Max() );
                ImplDrawHighlightRect( pTarget, aTopLeft, aBottomRight, pPolyPoly.get() );
            }
            else
            {
                sal_Int32 nTmpStartIndex = nStartIndex;
                sal_Int32 nWritingDirStart, nTmpEndIndex;

                while ( nTmpStartIndex < nEndIndex )
                {
                    pEditEngine->pImpEditEngine->GetRightToLeft( nPara, nTmpStartIndex+1, &nWritingDirStart, &nTmpEndIndex );
                    if ( nTmpEndIndex > nEndIndex )
                        nTmpEndIndex = nEndIndex;

                    DBG_ASSERT( nTmpEndIndex > nTmpStartIndex, "DrawSelectionXOR, Start >= End?" );

                    tools::Long nX1 = pEditEngine->GetXPos(pTmpPortion, &rLine, nTmpStartIndex, true);
                    tools::Long nX2 = pEditEngine->GetXPos(pTmpPortion, &rLine, nTmpEndIndex);

                    Point aPt1( std::min( nX1, nX2 ), aTopLeft.Y() );
                    Point aPt2( std::max( nX1, nX2 ), aBottomRight.Y() );

                    ImplDrawHighlightRect( pTarget, aPt1, aPt2, pPolyPoly.get() );
                    nTmpStartIndex = nTmpEndIndex;
                }
            }
        }
    }

    if (comphelper::LibreOfficeKit::isActive() && mpViewShell && pOutWin)
        lokSelectionCallback(pPolyPoly, bStartHandleVisible, bEndHandleVisible);

    if (pRegion || comphelper::LibreOfficeKit::isActive())
    {
        if (pRegion)
            *pRegion = vcl::Region( *pPolyPoly );
        pPolyPoly.reset();
    }
    else
    {
        if (pOutWin && pOutWin->GetCursor())
            pOutWin->GetCursor()->Show();

        if ( bClipRegion )
            pTarget->SetClipRegion( aOldRegion );
        else
            pTarget->SetClipRegion();
    }
}

void ImpEditView::GetSelectionRectangles(EditSelection aTmpSel, std::vector<tools::Rectangle>& rLogicRects)
{
    vcl::Region aRegion;
    DrawSelectionXOR(aTmpSel, &aRegion);
    aRegion.GetRegionRectangles(rLogicRects);
}

void ImpEditView::ImplDrawHighlightRect( OutputDevice* _pTarget, const Point& rDocPosTopLeft, const Point& rDocPosBottomRight, tools::PolyPolygon* pPolyPoly )
{
    if ( rDocPosTopLeft.X() == rDocPosBottomRight.X() )
        return;

    if (mpLOKSpecialPositioning && pPolyPoly)
    {
        MapUnit eDevUnit = _pTarget->GetMapMode().GetMapUnit();
        tools::Rectangle aSelRect(rDocPosTopLeft, rDocPosBottomRight);
        aSelRect = mpLOKSpecialPositioning->GetWindowPos(aSelRect, eDevUnit);
        const Point aRefPoint = mpLOKSpecialPositioning->GetRefPoint();
        aSelRect.Move(-aRefPoint.X(), -aRefPoint.Y());

        tools::Polygon aTmpPoly(4);
        aTmpPoly[0] = aSelRect.TopLeft();
        aTmpPoly[1] = aSelRect.TopRight();
        aTmpPoly[2] = aSelRect.BottomRight();
        aTmpPoly[3] = aSelRect.BottomLeft();
        pPolyPoly->Insert(aTmpPoly);
        return;
    }

    bool bPixelMode = _pTarget->GetMapMode().GetMapUnit() == MapUnit::MapPixel;

    Point aPnt1( GetWindowPos( rDocPosTopLeft ) );
    Point aPnt2( GetWindowPos( rDocPosBottomRight ) );

    if ( !IsVertical() )
    {
        lcl_AllignToPixel( aPnt1, _pTarget, +1, 0 );
        lcl_AllignToPixel( aPnt2, _pTarget, 0, ( bPixelMode ? 0 : -1 ) );
    }
    else
    {
        lcl_AllignToPixel( aPnt1, _pTarget, 0, +1 );
        lcl_AllignToPixel( aPnt2, _pTarget, ( bPixelMode ? 0 : +1 ), 0 );
    }

    tools::Rectangle aRect( aPnt1, aPnt2 );
    if ( pPolyPoly )
    {
        tools::Polygon aTmpPoly( 4 );
        aTmpPoly[0] = aRect.TopLeft();
        aTmpPoly[1] = aRect.TopRight();
        aTmpPoly[2] = aRect.BottomRight();
        aTmpPoly[3] = aRect.BottomLeft();
        pPolyPoly->Insert( aTmpPoly );
    }
    else
    {
        vcl::Window* pWindow = dynamic_cast< vcl::Window* >(_pTarget);

        if(pWindow)
        {
            pWindow->Invert( aRect );
        }
        else
        {
            _pTarget->Push(PushFlags::LINECOLOR|PushFlags::FILLCOLOR|PushFlags::RASTEROP);
            _pTarget->SetLineColor();
            _pTarget->SetFillColor(COL_BLACK);
            _pTarget->SetRasterOp(RasterOp::Invert);
            _pTarget->DrawRect(aRect);
            _pTarget->Pop();
        }
    }
}


bool ImpEditView::IsVertical() const
{
    return pEditEngine->pImpEditEngine->IsVertical();
}

bool ImpEditView::IsTopToBottom() const
{
    return pEditEngine->pImpEditEngine->IsTopToBottom();
}

tools::Rectangle ImpEditView::GetVisDocArea() const
{
    return tools::Rectangle( GetVisDocLeft(), GetVisDocTop(), GetVisDocRight(), GetVisDocBottom() );
}

Point ImpEditView::GetDocPos( const Point& rWindowPos ) const
{
    // Window Position => Position Document
    Point aPoint;

    if ( !pEditEngine->pImpEditEngine->IsVertical() )
    {
        aPoint.setX( rWindowPos.X() - aOutArea.Left() + GetVisDocLeft() );
        aPoint.setY( rWindowPos.Y() - aOutArea.Top() + GetVisDocTop() );
    }
    else
    {
        if (pEditEngine->pImpEditEngine->IsTopToBottom())
        {
            aPoint.setX( rWindowPos.Y() - aOutArea.Top() + GetVisDocLeft() );
            aPoint.setY( aOutArea.Right() - rWindowPos.X() + GetVisDocTop() );
        }
        else
        {
            aPoint.setX( aOutArea.Bottom() - rWindowPos.Y() + GetVisDocLeft() );
            aPoint.setY( rWindowPos.X() - aOutArea.Left() + GetVisDocTop() );
        }
    }

    return aPoint;
}

Point ImpEditView::GetWindowPos( const Point& rDocPos ) const
{
    // Document position => window position
    Point aPoint;

    if ( !pEditEngine->pImpEditEngine->IsVertical() )
    {
        aPoint.setX( rDocPos.X() + aOutArea.Left() - GetVisDocLeft() );
        aPoint.setY( rDocPos.Y() + aOutArea.Top() - GetVisDocTop() );
    }
    else
    {
        if (pEditEngine->pImpEditEngine->IsTopToBottom())
        {
            aPoint.setX( aOutArea.Right() - rDocPos.Y() + GetVisDocTop() );
            aPoint.setY( rDocPos.X() + aOutArea.Top() - GetVisDocLeft() );
        }
        else
        {
            aPoint.setX( aOutArea.Left() + rDocPos.Y() - GetVisDocTop() );
            aPoint.setY( aOutArea.Bottom() - rDocPos.X() + GetVisDocLeft() );
        }
    }

    return aPoint;
}

tools::Rectangle ImpEditView::GetWindowPos( const tools::Rectangle& rDocRect ) const
{
    // Document position => window position
    Point aPos( GetWindowPos( rDocRect.TopLeft() ) );
    Size aSz = rDocRect.GetSize();
    tools::Rectangle aRect;
    if ( !pEditEngine->pImpEditEngine->IsVertical() )
    {
        aRect = tools::Rectangle( aPos, aSz );
    }
    else
    {
        Point aNewPos( aPos.X()-aSz.Height(), aPos.Y() );
        // coverity[swapped_arguments : FALSE] - this is in the correct order
        aRect = tools::Rectangle( aNewPos, Size( aSz.Height(), aSz.Width() ) );
    }
    return aRect;
}

void ImpEditView::SetSelectionMode( EESelectionMode eNewMode )
{
    if ( eSelectionMode != eNewMode )
    {
        DrawSelectionXOR();
        eSelectionMode = eNewMode;
        DrawSelectionXOR();    // redraw
    }
}

OutputDevice& ImpEditView::GetOutputDevice() const
{
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
        return pCallbacks->EditViewOutputDevice();
    return *pOutWin;
}

weld::Widget* ImpEditView::GetPopupParent(tools::Rectangle& rRect) const
{
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
    {
        weld::Widget* pParent = pCallbacks->EditViewPopupParent();
        if (pParent)
            return pParent;
    }
    return weld::GetPopupParent(*pOutWin, rRect);
}

void ImpEditView::SetOutputArea( const tools::Rectangle& rRect )
{
    const OutputDevice& rOutDev = GetOutputDevice();
    // should be better be aligned on pixels!
    tools::Rectangle aNewRect(rOutDev.LogicToPixel(rRect));
    aNewRect = rOutDev.PixelToLogic(aNewRect);
    aOutArea = aNewRect;
    if ( !aOutArea.IsWidthEmpty() && aOutArea.Right() < aOutArea.Left() )
        aOutArea.SetRight( aOutArea.Left() );
    if ( !aOutArea.IsHeightEmpty() && aOutArea.Bottom() < aOutArea.Top() )
        aOutArea.SetBottom( aOutArea.Top() );

    SetScrollDiffX( static_cast<sal_uInt16>(aOutArea.GetWidth()) * 2 / 10 );
}

void ImpEditView::InvalidateAtWindow(const tools::Rectangle& rRect)
{
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
    {
        // do not invalidate and trigger a global repaint, but forward
        // the need for change to the applied EditViewCallback, can e.g.
        // be used to visualize the active edit text in an OverlayObject
        pCallbacks->EditViewInvalidate(rRect);
    }
    else
    {
        // classic mode: invalidate and trigger full repaint
        // of the changed area
        GetWindow()->Invalidate(rRect);
    }
}

void ImpEditView::ResetOutputArea( const tools::Rectangle& rRect )
{
    // remember old out area
    const tools::Rectangle aOldArea(aOutArea);

    // apply new one
    SetOutputArea(rRect);

    // invalidate surrounding areas if update is true
    if(aOldArea.IsEmpty() || !pEditEngine->pImpEditEngine->GetUpdateMode())
        return;

    // #i119885# use grown area if needed; do when getting bigger OR smaller
    const sal_Int32 nMore(DoInvalidateMore() ? GetOutputDevice().PixelToLogic(Size(nInvMore, 0)).Width() : 0);

    if(aOldArea.Left() > aOutArea.Left())
    {
        const tools::Rectangle aRect(aOutArea.Left() - nMore, aOldArea.Top() - nMore, aOldArea.Left(), aOldArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }
    else if(aOldArea.Left() < aOutArea.Left())
    {
        const tools::Rectangle aRect(aOldArea.Left() - nMore, aOldArea.Top() - nMore, aOutArea.Left(), aOldArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }

    if(aOldArea.Right() > aOutArea.Right())
    {
        const tools::Rectangle aRect(aOutArea.Right(), aOldArea.Top() - nMore, aOldArea.Right() + nMore, aOldArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }
    else if(aOldArea.Right() < aOutArea.Right())
    {
        const tools::Rectangle aRect(aOldArea.Right(), aOldArea.Top() - nMore, aOutArea.Right() + nMore, aOldArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }

    if(aOldArea.Top() > aOutArea.Top())
    {
        const tools::Rectangle aRect(aOldArea.Left() - nMore, aOutArea.Top() - nMore, aOldArea.Right() + nMore, aOldArea.Top());
        InvalidateAtWindow(aRect);
    }
    else if(aOldArea.Top() < aOutArea.Top())
    {
        const tools::Rectangle aRect(aOldArea.Left() - nMore, aOldArea.Top() - nMore, aOldArea.Right() + nMore, aOutArea.Top());
        InvalidateAtWindow(aRect);
    }

    if(aOldArea.Bottom() > aOutArea.Bottom())
    {
        const tools::Rectangle aRect(aOldArea.Left() - nMore, aOutArea.Bottom(), aOldArea.Right() + nMore, aOldArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }
    else if(aOldArea.Bottom() < aOutArea.Bottom())
    {
        const tools::Rectangle aRect(aOldArea.Left() - nMore, aOldArea.Bottom(), aOldArea.Right() + nMore, aOutArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }
}

void ImpEditView::RecalcOutputArea()
{
    Point aNewTopLeft( aOutArea.TopLeft() );
    Size aNewSz( aOutArea.GetSize() );

    // X:
    if ( DoAutoWidth() )
    {
        if ( pEditEngine->pImpEditEngine->GetStatus().AutoPageWidth() )
            aNewSz.setWidth( pEditEngine->pImpEditEngine->GetPaperSize().Width() );
        switch ( eAnchorMode )
        {
            case EEAnchorMode::TopLeft:
            case EEAnchorMode::VCenterLeft:
            case EEAnchorMode::BottomLeft:
            {
                aNewTopLeft.setX( aAnchorPoint.X() );
            }
            break;
            case EEAnchorMode::TopHCenter:
            case EEAnchorMode::VCenterHCenter:
            case EEAnchorMode::BottomHCenter:
            {
                aNewTopLeft.setX( aAnchorPoint.X() - aNewSz.Width() / 2 );
            }
            break;
            case EEAnchorMode::TopRight:
            case EEAnchorMode::VCenterRight:
            case EEAnchorMode::BottomRight:
            {
                aNewTopLeft.setX( aAnchorPoint.X() - aNewSz.Width() - 1 );
            }
            break;
        }
    }

    // Y:
    if ( DoAutoHeight() )
    {
        if ( pEditEngine->pImpEditEngine->GetStatus().AutoPageHeight() )
            aNewSz.setHeight( pEditEngine->pImpEditEngine->GetPaperSize().Height() );
        switch ( eAnchorMode )
        {
            case EEAnchorMode::TopLeft:
            case EEAnchorMode::TopHCenter:
            case EEAnchorMode::TopRight:
            {
                aNewTopLeft.setY( aAnchorPoint.Y() );
            }
            break;
            case EEAnchorMode::VCenterLeft:
            case EEAnchorMode::VCenterHCenter:
            case EEAnchorMode::VCenterRight:
            {
                aNewTopLeft.setY( aAnchorPoint.Y() - aNewSz.Height() / 2 );
            }
            break;
            case EEAnchorMode::BottomLeft:
            case EEAnchorMode::BottomHCenter:
            case EEAnchorMode::BottomRight:
            {
                aNewTopLeft.setY( aAnchorPoint.Y() - aNewSz.Height() - 1 );
            }
            break;
        }
    }
    ResetOutputArea( tools::Rectangle( aNewTopLeft, aNewSz ) );
}

void ImpEditView::SetAnchorMode( EEAnchorMode eMode )
{
    eAnchorMode = eMode;
    CalcAnchorPoint();
}

void ImpEditView::CalcAnchorPoint()
{
    // GetHeight() and GetWidth() -1, because rectangle calculation not preferred.

    // X:
    switch ( eAnchorMode )
    {
        case EEAnchorMode::TopLeft:
        case EEAnchorMode::VCenterLeft:
        case EEAnchorMode::BottomLeft:
        {
            aAnchorPoint.setX( aOutArea.Left() );
        }
        break;
        case EEAnchorMode::TopHCenter:
        case EEAnchorMode::VCenterHCenter:
        case EEAnchorMode::BottomHCenter:
        {
            aAnchorPoint.setX( aOutArea.Left() + (aOutArea.GetWidth()-1) / 2 );
        }
        break;
        case EEAnchorMode::TopRight:
        case EEAnchorMode::VCenterRight:
        case EEAnchorMode::BottomRight:
        {
            aAnchorPoint.setX( aOutArea.Right() );
        }
        break;
    }

    // Y:
    switch ( eAnchorMode )
    {
        case EEAnchorMode::TopLeft:
        case EEAnchorMode::TopHCenter:
        case EEAnchorMode::TopRight:
        {
            aAnchorPoint.setY( aOutArea.Top() );
        }
        break;
        case EEAnchorMode::VCenterLeft:
        case EEAnchorMode::VCenterHCenter:
        case EEAnchorMode::VCenterRight:
        {
            aAnchorPoint.setY( aOutArea.Top() + (aOutArea.GetHeight()-1) / 2 );
        }
        break;
        case EEAnchorMode::BottomLeft:
        case EEAnchorMode::BottomHCenter:
        case EEAnchorMode::BottomRight:
        {
            aAnchorPoint.setY( aOutArea.Bottom() - 1 );
        }
        break;
    }
}

namespace
{

// For building JSON message to be sent to Online
boost::property_tree::ptree getHyperlinkPropTree(const OUString& sText, const OUString& sLink)
{
    boost::property_tree::ptree aTree;
    aTree.put("text", sText);
    aTree.put("link", sLink);
    return aTree;
}

} // End of anon namespace

tools::Rectangle ImpEditView::ImplGetEditCursor(EditPaM& aPaM, GetCursorFlags nShowCursorFlags, sal_Int32& nTextPortionStart,
        const ParaPortion* pParaPortion) const
{
    tools::Rectangle aEditCursor = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM, nShowCursorFlags );
    if ( !IsInsertMode() && !aEditSelection.HasRange() )
    {
        if ( aPaM.GetNode()->Len() && ( aPaM.GetIndex() < aPaM.GetNode()->Len() ) )
        {
            // If we are behind a portion, and the next portion has other direction, we must change position...
            aEditCursor.SetLeft( pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM, GetCursorFlags::TextOnly|GetCursorFlags::PreferPortionStart ).Left() );
            aEditCursor.SetRight( aEditCursor.Left() );

            sal_Int32 nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, true );
            const TextPortion& rTextPortion = pParaPortion->GetTextPortions()[nTextPortion];
            if ( rTextPortion.GetKind() == PortionKind::TAB )
            {
                aEditCursor.AdjustRight(rTextPortion.GetSize().Width() );
            }
            else
            {
                EditPaM aNext = pEditEngine->CursorRight( aPaM );
                tools::Rectangle aTmpRect = pEditEngine->pImpEditEngine->PaMtoEditCursor( aNext, GetCursorFlags::TextOnly );
                if ( aTmpRect.Top() != aEditCursor.Top() )
                    aTmpRect = pEditEngine->pImpEditEngine->PaMtoEditCursor( aNext, GetCursorFlags::TextOnly|GetCursorFlags::EndOfLine );
                aEditCursor.SetRight( aTmpRect.Left() );
            }
        }
    }

    tools::Long nMaxHeight = !IsVertical() ? aOutArea.GetHeight() : aOutArea.GetWidth();
    if ( aEditCursor.GetHeight() > nMaxHeight )
    {
        aEditCursor.SetBottom( aEditCursor.Top() + nMaxHeight - 1 );
    }

    return aEditCursor;
}

tools::Rectangle ImpEditView::GetEditCursor() const
{
    EditPaM aPaM( aEditSelection.Max() );

    sal_Int32 nTextPortionStart = 0;
    sal_Int32 nPara = pEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
    if (nPara == EE_PARA_NOT_FOUND) // #i94322
        return tools::Rectangle();

    const ParaPortion* pParaPortion = pEditEngine->GetParaPortions()[nPara];

    GetCursorFlags nShowCursorFlags = nExtraCursorFlags | GetCursorFlags::TextOnly;

    // Use CursorBidiLevel 0/1 in meaning of
    // 0: prefer portion end, normal mode
    // 1: prefer portion start

    if ( ( GetCursorBidiLevel() != CURSOR_BIDILEVEL_DONTKNOW ) && GetCursorBidiLevel() )
    {
        nShowCursorFlags |= GetCursorFlags::PreferPortionStart;
    }

    return ImplGetEditCursor(aPaM, nShowCursorFlags, nTextPortionStart, pParaPortion);
}

void ImpEditView::ShowCursor( bool bGotoCursor, bool bForceVisCursor )
{
    // No ShowCursor in an empty View ...
    if (aOutArea.IsEmpty())
        return;
    if ( ( aOutArea.Left() >= aOutArea.Right() ) && ( aOutArea.Top() >= aOutArea.Bottom() ) )
        return;

    pEditEngine->CheckIdleFormatter();
    if (!pEditEngine->IsFormatted())
        pEditEngine->pImpEditEngine->FormatDoc();

    // For some reasons I end up here during the formatting, if the Outliner
    // is initialized in Paint, because no SetPool();
    if ( pEditEngine->pImpEditEngine->IsFormatting() )
        return;
    if ( !pEditEngine->pImpEditEngine->GetUpdateMode() )
        return;
    if ( pEditEngine->pImpEditEngine->IsInUndo() )
        return;

    if (pOutWin && pOutWin->GetCursor() != GetCursor())
        pOutWin->SetCursor(GetCursor());

    EditPaM aPaM( aEditSelection.Max() );

    sal_Int32 nTextPortionStart = 0;
    sal_Int32 nPara = pEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
    if (nPara == EE_PARA_NOT_FOUND) // #i94322
        return;

    const ParaPortion* pParaPortion = pEditEngine->GetParaPortions()[nPara];

    GetCursorFlags nShowCursorFlags = nExtraCursorFlags | GetCursorFlags::TextOnly;

    // Use CursorBidiLevel 0/1 in meaning of
    // 0: prefer portion end, normal mode
    // 1: prefer portion start

    if ( ( GetCursorBidiLevel() != CURSOR_BIDILEVEL_DONTKNOW ) && GetCursorBidiLevel() )
    {
        nShowCursorFlags |= GetCursorFlags::PreferPortionStart;
    }

    tools::Rectangle aEditCursor = ImplGetEditCursor(aPaM, nShowCursorFlags, nTextPortionStart, pParaPortion);

    if ( bGotoCursor  ) // && (!pEditEngine->pImpEditEngine->GetStatus().AutoPageSize() ) )
    {
        // check if scrolling is necessary...
        // if scrolling, then update () and Scroll ()!
        tools::Long nDocDiffX = 0;
        tools::Long nDocDiffY = 0;

        tools::Rectangle aTmpVisArea( GetVisDocArea() );
        // aTmpOutArea: if OutputArea > Paper width and
        // Text > Paper width ( over large fields )
        tools::Long nMaxTextWidth = !IsVertical() ? pEditEngine->pImpEditEngine->GetPaperSize().Width() : pEditEngine->pImpEditEngine->GetPaperSize().Height();
        if ( aTmpVisArea.GetWidth() > nMaxTextWidth )
            aTmpVisArea.SetRight( aTmpVisArea.Left() + nMaxTextWidth );

        if ( aEditCursor.Bottom() > aTmpVisArea.Bottom() )
        {   // Scroll up, here positive
            nDocDiffY = aEditCursor.Bottom() - aTmpVisArea.Bottom();
        }
        else if ( aEditCursor.Top() < aTmpVisArea.Top() )
        {   // Scroll down, here negative
            nDocDiffY = aEditCursor.Top() - aTmpVisArea.Top();
        }

        if ( aEditCursor.Right() > aTmpVisArea.Right() )
        {
            // Scroll left, positive
            nDocDiffX = aEditCursor.Right() - aTmpVisArea.Right();
            // Can it be a little more?
            if ( aEditCursor.Right() < ( nMaxTextWidth - GetScrollDiffX() ) )
                nDocDiffX += GetScrollDiffX();
            else
            {
                tools::Long n = nMaxTextWidth - aEditCursor.Right();
                // If MapMode != RefMapMode then the EditCursor can go beyond
                // the paper width!
                nDocDiffX += ( n > 0 ? n : -n );
            }
        }
        else if ( aEditCursor.Left() < aTmpVisArea.Left() )
        {
            // Scroll right, negative:
            nDocDiffX = aEditCursor.Left() - aTmpVisArea.Left();
            // Can it be a little more?
            if ( aEditCursor.Left() > ( - static_cast<tools::Long>(GetScrollDiffX()) ) )
                nDocDiffX -= GetScrollDiffX();
            else
                nDocDiffX -= aEditCursor.Left();
        }
        if ( aPaM.GetIndex() == 0 )     // Olli needed for the Outliner
        {
            // But make sure that the cursor is not leaving visible area
            // because of this!
            if ( aEditCursor.Left() < aTmpVisArea.GetWidth() )
            {
                nDocDiffX = -aTmpVisArea.Left();
            }
        }

        if ( nDocDiffX | nDocDiffY )
        {
            tools::Long nDiffX = !IsVertical() ? nDocDiffX : (IsTopToBottom() ? -nDocDiffY : nDocDiffY);
            tools::Long nDiffY = !IsVertical() ? nDocDiffY : (IsTopToBottom() ? nDocDiffX : -nDocDiffX);

            if ( nDiffX )
                pEditEngine->GetInternalEditStatus().GetStatusWord() = pEditEngine->GetInternalEditStatus().GetStatusWord() | EditStatusFlags::HSCROLL;
            if ( nDiffY )
                pEditEngine->GetInternalEditStatus().GetStatusWord() = pEditEngine->GetInternalEditStatus().GetStatusWord() | EditStatusFlags::VSCROLL;
            Scroll( -nDiffX, -nDiffY );
            pEditEngine->pImpEditEngine->DelayedCallStatusHdl();
        }
    }

    // Cursor may trim a little ...
    if ( ( aEditCursor.Bottom() > GetVisDocTop() ) &&
         ( aEditCursor.Top() < GetVisDocBottom() ) )
    {
        if ( aEditCursor.Bottom() > GetVisDocBottom() )
            aEditCursor.SetBottom( GetVisDocBottom() );
        if ( aEditCursor.Top() < GetVisDocTop() )
            aEditCursor.SetTop( GetVisDocTop() );
    }

    const OutputDevice& rOutDev = GetOutputDevice();

    tools::Long nOnePixel = rOutDev.PixelToLogic( Size( 1, 0 ) ).Width();

    if ( ( aEditCursor.Top() + nOnePixel >= GetVisDocTop() ) &&
         ( aEditCursor.Bottom() - nOnePixel <= GetVisDocBottom() ) &&
         ( aEditCursor.Left() + nOnePixel >= GetVisDocLeft() ) &&
         ( aEditCursor.Right() - nOnePixel <= GetVisDocRight() ) )
    {
        tools::Rectangle aCursorRect = GetWindowPos( aEditCursor );
        GetCursor()->SetPos( aCursorRect.TopLeft() );
        Size aCursorSz( aCursorRect.GetSize() );
        // Rectangle is inclusive
        aCursorSz.AdjustWidth( -1 );
        aCursorSz.AdjustHeight( -1 );
        if ( !aCursorSz.Width() || !aCursorSz.Height() )
        {
            tools::Long nCursorSz = rOutDev.GetSettings().GetStyleSettings().GetCursorSize();
            nCursorSz = rOutDev.PixelToLogic( Size( nCursorSz, 0 ) ).Width();
            if ( !aCursorSz.Width() )
                aCursorSz.setWidth( nCursorSz );
            if ( !aCursorSz.Height() )
                aCursorSz.setHeight( nCursorSz );
        }
        // #111036# Let VCL do orientation for cursor, otherwise problem when cursor has direction flag
        if ( IsVertical() )
        {
            Size aOldSz( aCursorSz );
            aCursorSz.setWidth( aOldSz.Height() );
            aCursorSz.setHeight( aOldSz.Width() );
            GetCursor()->SetPos( aCursorRect.TopRight() );
            GetCursor()->SetOrientation( Degree10(IsTopToBottom() ? 2700 : 900) );
        }
        else
            // #i32593# Reset correct orientation in horizontal layout
            GetCursor()->SetOrientation();

        GetCursor()->SetSize( aCursorSz );

        if (comphelper::LibreOfficeKit::isActive() && mpViewShell && !mbSuppressLOKMessages)
        {
            Point aPos = GetCursor()->GetPos();
            boost::property_tree::ptree aMessageParams;
            if (mpLOKSpecialPositioning)
            {
                // Sending the absolute (pure) logical coordinates of the cursor to the client is not
                // enough for it to accurately reconstruct the corresponding tile-twips coordinates of the cursor.
                // This is because the editeng(doc) positioning is not pixel aligned for each cell involved in the output-area
                // (it better not be!). A simple solution is to send the coordinates of a point ('refpoint') in the output-area
                // along with the relative position of the cursor w.r.t this chosen 'refpoint'.

                MapUnit eDevUnit = rOutDev.GetMapMode().GetMapUnit();
                tools::Rectangle aCursorRectPureLogical(aEditCursor.TopLeft(), GetCursor()->GetSize());
                // Get rectangle in window-coordinates from editeng(doc) coordinates.
                aCursorRectPureLogical = mpLOKSpecialPositioning->GetWindowPos(aCursorRectPureLogical, eDevUnit);
                const Point aRefPoint = mpLOKSpecialPositioning->GetRefPoint();
                // Get the relative coordinates w.r.t rRefPoint.
                aCursorRectPureLogical.Move(-aRefPoint.X(), -aRefPoint.Y());
                aMessageParams.put("relrect", aCursorRectPureLogical.toString());
                aMessageParams.put("refpoint", aRefPoint.toString());
            }

            if (pOutWin && pOutWin->IsChart())
            {
                const vcl::Window* pViewShellWindow = mpViewShell->GetEditWindowForActiveOLEObj();
                if (pViewShellWindow && pViewShellWindow->IsAncestorOf(*pOutWin))
                {
                    Point aOffsetPx = pOutWin->GetOffsetPixelFrom(*pViewShellWindow);
                    Point aLogicOffset = pOutWin->PixelToLogic(aOffsetPx);
                    aPos.Move(aLogicOffset.getX(), aLogicOffset.getY());
                }
            }

            tools::Rectangle aRect(aPos.getX(), aPos.getY(), aPos.getX() + GetCursor()->GetWidth(), aPos.getY() + GetCursor()->GetHeight());

            // LOK output is always in twips, convert from mm100 if necessary.
            if (rOutDev.GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
                aRect = OutputDevice::LogicToLogic(aRect, MapMode(MapUnit::Map100thMM), MapMode(MapUnit::MapTwip));
            else if (rOutDev.GetMapMode().GetMapUnit() == MapUnit::MapTwip)
            {
                // Writer comments: they use editeng, but are separate widgets.
                Point aOrigin = rOutDev.GetMapMode().GetOrigin();
                // Move the rectangle, so that we output absolute twips.
                aRect.Move(aOrigin.getX(), aOrigin.getY());
            }
            // Let the LOK client decide the cursor width.
            aRect.setWidth(0);

            OString sRect = aRect.toString();
            aMessageParams.put("rectangle", sRect);

            SfxViewShell* pThisShell = dynamic_cast<SfxViewShell*>(mpViewShell);
            SfxViewShell* pOtherShell = dynamic_cast<SfxViewShell*>(mpOtherShell);
            assert(pThisShell);

            if (pOtherShell && pThisShell != pOtherShell)
            {
                // Another shell wants to know about our existing cursor.
                SfxLokHelper::notifyOtherView(pThisShell, pOtherShell,
                        LOK_CALLBACK_INVALIDATE_VIEW_CURSOR, aMessageParams);
            }
            else
            {
                // is cursor at a misspelled word ?
                Reference< linguistic2::XSpellChecker1 >  xSpeller( pEditEngine->pImpEditEngine->GetSpeller() );
                bool bIsWrong = xSpeller.is() && IsWrongSpelledWord(aPaM, /*bMarkIfWrong*/ false);

                boost::property_tree::ptree aHyperlinkTree;
                if (const SvxFieldItem* pFld = GetField(aPos, nullptr, nullptr))
                {
                    if (auto pUrlField = dynamic_cast<const SvxURLField*>(pFld->GetField()))
                    {
                        aHyperlinkTree = getHyperlinkPropTree(pUrlField->GetRepresentation(), pUrlField->GetURL());
                    }
                }
                else if (GetEditSelection().HasRange())
                {
                    EditView* pActiveView = GetEditViewPtr();

                    if (pActiveView)
                    {
                        const SvxFieldItem* pFieldItem = pActiveView->GetFieldAtSelection();
                        if (pFieldItem)
                        {
                            const SvxFieldData* pField = pFieldItem->GetField();
                            if ( auto pUrlField = dynamic_cast<const SvxURLField*>( pField) )
                            {
                                aHyperlinkTree = getHyperlinkPropTree(pUrlField->GetRepresentation(), pUrlField->GetURL());
                            }
                        }
                    }
                }

                if (mbBroadcastLOKViewCursor)
                    SfxLokHelper::notifyOtherViews(pThisShell,
                            LOK_CALLBACK_INVALIDATE_VIEW_CURSOR, aMessageParams);

                aMessageParams.put("mispelledWord", bIsWrong ? 1 : 0);
                aMessageParams.add_child("hyperlink", aHyperlinkTree);

                if (comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
                    SfxLokHelper::notifyOtherView(pThisShell, pThisShell,
                            LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, aMessageParams);
                else
                    pThisShell->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR,
                            aMessageParams.get<std::string>("rectangle").c_str());
            }
        }

        CursorDirection nCursorDir = CursorDirection::NONE;
        if ( IsInsertMode() && !aEditSelection.HasRange() && ( pEditEngine->pImpEditEngine->HasDifferentRTLLevels( aPaM.GetNode() ) ) )
        {
            sal_uInt16 nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, bool(nShowCursorFlags & GetCursorFlags::PreferPortionStart) );
            const TextPortion& rTextPortion = pParaPortion->GetTextPortions()[nTextPortion];
            if (rTextPortion.IsRightToLeft())
                nCursorDir = CursorDirection::RTL;
            else
                nCursorDir = CursorDirection::LTR;

        }
        GetCursor()->SetDirection( nCursorDir );

        if ( bForceVisCursor )
            GetCursor()->Show();
        {
            SvxFont aFont;
            pEditEngine->SeekCursor( aPaM.GetNode(), aPaM.GetIndex()+1, aFont );

            InputContext aInputContext(aFont, InputContextFlags::Text | InputContextFlags::ExtText);
            if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
                pCallbacks->EditViewInputContext(aInputContext);
            else if (auto xWindow = GetWindow())
                xWindow->SetInputContext(aInputContext);
        }
    }
    else
    {
        pEditEngine->pImpEditEngine->GetStatus().GetStatusWord() = pEditEngine->pImpEditEngine->GetStatus().GetStatusWord() | EditStatusFlags::CURSOROUT;
        GetCursor()->Hide();
        GetCursor()->SetPos( Point( -1, -1 ) );
        GetCursor()->SetSize( Size( 0, 0 ) );
    }
}

// call this so users of EditViewCallbacks can update their scrollbar state
// so called when we have either scrolled to a new location
// or the size of document has changed
void ImpEditView::ScrollStateChange()
{
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
        pCallbacks->EditViewScrollStateChange();
}

Pair ImpEditView::Scroll( tools::Long ndX, tools::Long ndY, ScrollRangeCheck nRangeCheck )
{
    DBG_ASSERT( pEditEngine->pImpEditEngine->IsFormatted(), "Scroll: Not formatted!" );
    if ( !ndX && !ndY )
        return Pair( 0, 0 );

    const OutputDevice& rOutDev = GetOutputDevice();

#ifdef DBG_UTIL
    tools::Rectangle aR( aOutArea );
    aR = rOutDev.LogicToPixel( aR );
    aR = rOutDev.PixelToLogic( aR );
    SAL_WARN_IF( aR != aOutArea, "editeng", "OutArea before Scroll not aligned" );
#endif

    tools::Rectangle aNewVisArea( GetVisDocArea() );

    // Vertical:
    if ( !IsVertical() )
    {
        aNewVisArea.AdjustTop( -ndY );
        aNewVisArea.AdjustBottom( -ndY );
    }
    else
    {
        if( IsTopToBottom() )
        {
            aNewVisArea.AdjustTop(ndX );
            aNewVisArea.AdjustBottom(ndX );
        }
        else
        {
            aNewVisArea.AdjustTop( -ndX );
            aNewVisArea.AdjustBottom( -ndX );
        }
    }
    if ( ( nRangeCheck == ScrollRangeCheck::PaperWidthTextSize ) && ( aNewVisArea.Bottom() > static_cast<tools::Long>(pEditEngine->pImpEditEngine->GetTextHeight()) ) )
    {
        // GetTextHeight still optimizing!
        tools::Long nDiff = pEditEngine->pImpEditEngine->GetTextHeight() - aNewVisArea.Bottom(); // negative
        aNewVisArea.Move( 0, nDiff );   // could end up in the negative area...
    }
    if ( aNewVisArea.Top() < 0 )
        aNewVisArea.Move( 0, -aNewVisArea.Top() );

    // Horizontal:
    if ( !IsVertical() )
    {
        aNewVisArea.AdjustLeft( -ndX );
        aNewVisArea.AdjustRight( -ndX );
    }
    else
    {
        if (IsTopToBottom())
        {
            aNewVisArea.AdjustLeft( -ndY );
            aNewVisArea.AdjustRight( -ndY );
        }
        else
        {
            aNewVisArea.AdjustLeft(ndY );
            aNewVisArea.AdjustRight(ndY );
        }
    }
    if ( ( nRangeCheck == ScrollRangeCheck::PaperWidthTextSize ) && ( aNewVisArea.Right() > static_cast<tools::Long>(pEditEngine->pImpEditEngine->CalcTextWidth( false )) ) )
    {
        tools::Long nDiff = pEditEngine->pImpEditEngine->CalcTextWidth( false ) - aNewVisArea.Right();     // negative
        aNewVisArea.Move( nDiff, 0 );   // could end up in the negative area...
    }
    if ( aNewVisArea.Left() < 0 )
        aNewVisArea.Move( -aNewVisArea.Left(), 0 );

    // The difference must be alignt on pixel (due to scroll!)
    tools::Long nDiffX = !IsVertical() ? ( GetVisDocLeft() - aNewVisArea.Left() ) : (IsTopToBottom() ? -( GetVisDocTop() - aNewVisArea.Top() ) : (GetVisDocTop() - aNewVisArea.Top()));
    tools::Long nDiffY = !IsVertical() ? ( GetVisDocTop() - aNewVisArea.Top() ) : (IsTopToBottom() ? (GetVisDocLeft() - aNewVisArea.Left()) : -(GetVisDocTop() - aNewVisArea.Top()));

    Size aDiffs( nDiffX, nDiffY );
    aDiffs = rOutDev.LogicToPixel( aDiffs );
    aDiffs = rOutDev.PixelToLogic( aDiffs );

    tools::Long nRealDiffX = aDiffs.Width();
    tools::Long nRealDiffY = aDiffs.Height();


    if ( nRealDiffX || nRealDiffY )
    {
        vcl::Cursor* pCrsr = GetCursor();
        bool bVisCursor = pCrsr->IsVisible();
        pCrsr->Hide();
        if (pOutWin)
            pOutWin->PaintImmediately();
        if ( !IsVertical() )
            aVisDocStartPos.Move( -nRealDiffX, -nRealDiffY );
        else
        {
            if (IsTopToBottom())
                aVisDocStartPos.Move(-nRealDiffY, nRealDiffX);
            else
                aVisDocStartPos.Move(nRealDiffY, -nRealDiffX);
        }
        // Move by aligned value does not necessarily result in aligned
        // rectangle ...
        aVisDocStartPos = rOutDev.LogicToPixel( aVisDocStartPos );
        aVisDocStartPos = rOutDev.PixelToLogic( aVisDocStartPos );
        tools::Rectangle aRect( aOutArea );

        if (pOutWin)
        {
            pOutWin->Scroll( nRealDiffX, nRealDiffY, aRect, ScrollFlags::Clip );
        }

        if (comphelper::LibreOfficeKit::isActive() || getEditViewCallbacks())
        {
            // Need to invalidate the window, otherwise no tile will be re-painted.
            pEditView->Invalidate();
        }

        if (pOutWin)
            pOutWin->PaintImmediately();
        pCrsr->SetPos( pCrsr->GetPos() + Point( nRealDiffX, nRealDiffY ) );
        if ( bVisCursor )
        {
            tools::Rectangle aCursorRect( pCrsr->GetPos(), pCrsr->GetSize() );
            if ( aOutArea.IsInside( aCursorRect ) )
                pCrsr->Show();
        }

        if ( pEditEngine->pImpEditEngine->GetNotifyHdl().IsSet() )
        {
            EENotify aNotify( EE_NOTIFY_TEXTVIEWSCROLLED );
            pEditEngine->pImpEditEngine->GetNotifyHdl().Call( aNotify );
        }

        if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
            pCallbacks->EditViewScrollStateChange();

        if (comphelper::LibreOfficeKit::isActive())
        {
            DrawSelectionXOR();
        }
    }

    return Pair( nRealDiffX, nRealDiffY );
}

Reference<css::datatransfer::clipboard::XClipboard> ImpEditView::GetClipboard() const
{
    if (vcl::Window* pWindow = GetWindow())
        return pWindow->GetClipboard();
    return GetSystemClipboard();
}

Reference<css::datatransfer::clipboard::XClipboard> ImpEditView::GetSelection() const
{
    if (vcl::Window* pWindow = GetWindow())
        return pWindow->GetPrimarySelection();
    return GetSystemPrimarySelection();
}

bool ImpEditView::PostKeyEvent( const KeyEvent& rKeyEvent, vcl::Window const * pFrameWin )
{
    bool bDone = false;

    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KeyFuncType::DONTKNOW )
    {
        switch ( eFunc )
        {
            case KeyFuncType::CUT:
            {
                if ( !bReadOnly )
                {
                    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetClipboard());
                    CutCopy( aClipBoard, true );
                    bDone = true;
                }
            }
            break;
            case KeyFuncType::COPY:
            {
                Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetClipboard());
                CutCopy( aClipBoard, false );
                bDone = true;
            }
            break;
            case KeyFuncType::PASTE:
            {
                if ( !bReadOnly && IsPasteEnabled() )
                {
                    pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_PASTE );
                    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetClipboard());
                    Paste( aClipBoard, pEditEngine->pImpEditEngine->GetStatus().AllowPasteSpecial() );
                    pEditEngine->pImpEditEngine->UndoActionEnd();
                    bDone = true;
                }
            }
            break;
            default:
                break;
        }
    }

    if( !bDone )
        bDone = pEditEngine->PostKeyEvent( rKeyEvent, GetEditViewPtr(), pFrameWin );

    return bDone;
}

bool ImpEditView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    nTravelXPos = TRAVEL_X_DONTKNOW;
    nCursorBidiLevel = CURSOR_BIDILEVEL_DONTKNOW;
    nExtraCursorFlags = GetCursorFlags::NONE;
    bClickedInSelection = false;

    if (vcl::Window* pWindow = GetWindow())
    {
        if ( rMouseEvent.IsMiddle() && !bReadOnly &&
             ( pWindow->GetSettings().GetMouseSettings().GetMiddleButtonAction() == MouseMiddleButtonAction::PasteSelection ) )
        {
            Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetSelection());
            Paste( aClipBoard );
        }
        else if ( rMouseEvent.IsLeft() && GetEditSelection().HasRange() )
        {
            Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetSelection());
            CutCopy( aClipBoard, false );
        }
    }

    return pEditEngine->pImpEditEngine->MouseButtonUp( rMouseEvent, GetEditViewPtr() );
}

void ImpEditView::ReleaseMouse()
{
    pEditEngine->pImpEditEngine->ReleaseMouse();
}

bool ImpEditView::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    pEditEngine->CheckIdleFormatter();  // If fast typing and mouse button downs
    nTravelXPos         = TRAVEL_X_DONTKNOW;
    nExtraCursorFlags   = GetCursorFlags::NONE;
    nCursorBidiLevel    = CURSOR_BIDILEVEL_DONTKNOW;
    bClickedInSelection = IsSelectionAtPoint( rMouseEvent.GetPosPixel() );
    return pEditEngine->pImpEditEngine->MouseButtonDown( rMouseEvent, GetEditViewPtr() );
}

bool ImpEditView::MouseMove( const MouseEvent& rMouseEvent )
{
    return pEditEngine->pImpEditEngine->MouseMove( rMouseEvent, GetEditViewPtr() );
}

bool ImpEditView::Command(const CommandEvent& rCEvt)
{
    pEditEngine->CheckIdleFormatter();  // If fast typing and mouse button down
    return pEditEngine->pImpEditEngine->Command(rCEvt, GetEditViewPtr());
}


void ImpEditView::SetInsertMode( bool bInsert )
{
    if ( bInsert != IsInsertMode() )
    {
        SetFlags( nControl, EVControlBits::OVERWRITE, !bInsert );
        ShowCursor( DoAutoScroll(), false );
    }
}

bool ImpEditView::IsWrongSpelledWord( const EditPaM& rPaM, bool bMarkIfWrong )
{
    bool bIsWrong = false;
    if ( rPaM.GetNode()->GetWrongList() )
    {
        EditSelection aSel = pEditEngine->SelectWord( rPaM, css::i18n::WordType::DICTIONARY_WORD );
        bIsWrong = rPaM.GetNode()->GetWrongList()->HasWrong( aSel.Min().GetIndex(), aSel.Max().GetIndex() );
        if ( bIsWrong && bMarkIfWrong )
        {
            DrawSelectionXOR();
            SetEditSelection( aSel );
            DrawSelectionXOR();
        }
    }
    return bIsWrong;
}

OUString ImpEditView::SpellIgnoreWord()
{
    OUString aWord;
    if ( pEditEngine->pImpEditEngine->GetSpeller().is() )
    {
        EditPaM aPaM = GetEditSelection().Max();
        if ( !HasSelection() )
        {
            EditSelection aSel = pEditEngine->SelectWord(aPaM);
            aWord = pEditEngine->pImpEditEngine->GetSelected( aSel );
        }
        else
        {
            aWord = pEditEngine->pImpEditEngine->GetSelected( GetEditSelection() );
            // And deselect
            DrawSelectionXOR();
            SetEditSelection( EditSelection( aPaM, aPaM ) );
            DrawSelectionXOR();
        }

        if ( !aWord.isEmpty() )
        {
            Reference< XDictionary >  xDic = LinguMgr::GetIgnoreAllList();
            if (xDic.is())
                xDic->add( aWord, false, OUString() );
            EditDoc& rDoc = pEditEngine->GetEditDoc();
            sal_Int32 nNodes = rDoc.Count();
            for ( sal_Int32 n = 0; n < nNodes; n++ )
            {
                ContentNode* pNode = rDoc.GetObject( n );
                pNode->GetWrongList()->MarkWrongsInvalid();
            }
            pEditEngine->pImpEditEngine->DoOnlineSpelling( aPaM.GetNode() );
            pEditEngine->pImpEditEngine->StartOnlineSpellTimer();
        }
    }
    return aWord;
}

void ImpEditView::DeleteSelected()
{
    DrawSelectionXOR();

    pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_DELETE );

    EditPaM aPaM = pEditEngine->pImpEditEngine->DeleteSelected( GetEditSelection() );

    pEditEngine->pImpEditEngine->UndoActionEnd();

    SetEditSelection( EditSelection( aPaM, aPaM ) );

    DrawSelectionXOR();

    pEditEngine->pImpEditEngine->FormatAndUpdate( GetEditViewPtr() );
    ShowCursor( DoAutoScroll(), true );
}

const SvxFieldItem* ImpEditView::GetField( const Point& rPos, sal_Int32* pPara, sal_Int32* pPos ) const
{
    if( !GetOutputArea().IsInside( rPos ) )
        return nullptr;

    Point aDocPos( GetDocPos( rPos ) );
    EditPaM aPaM = pEditEngine->GetPaM(aDocPos, false);

    if ( aPaM.GetIndex() == aPaM.GetNode()->Len() )
    {
        // Otherwise, whenever the Field at the very end and mouse under the text
        return nullptr;
    }

    const CharAttribList::AttribsType& rAttrs = aPaM.GetNode()->GetCharAttribs().GetAttribs();
    const sal_Int32 nXPos = aPaM.GetIndex();
    for (size_t nAttr = rAttrs.size(); nAttr; )
    {
        const EditCharAttrib& rAttr = *rAttrs[--nAttr];
        if (rAttr.GetStart() == nXPos || rAttr.GetEnd() == nXPos)
        {
            if (rAttr.Which() == EE_FEATURE_FIELD)
            {
                DBG_ASSERT(dynamic_cast<const SvxFieldItem*>(rAttr.GetItem()), "No FieldItem...");
                if ( pPara )
                    *pPara = pEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
                if ( pPos )
                    *pPos = rAttr.GetStart();
                return static_cast<const SvxFieldItem*>(rAttr.GetItem());
            }
        }
    }
    return nullptr;
}

bool ImpEditView::IsBulletArea( const Point& rPos, sal_Int32* pPara )
{
    if ( pPara )
        *pPara = EE_PARA_NOT_FOUND;

    if( !GetOutputArea().IsInside( rPos ) )
        return false;

    Point aDocPos( GetDocPos( rPos ) );
    EditPaM aPaM = pEditEngine->GetPaM(aDocPos, false);

    if ( aPaM.GetIndex() == 0 )
    {
        sal_Int32 nPara = pEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
        tools::Rectangle aBulletArea = pEditEngine->GetBulletArea( nPara );
        tools::Long nY = pEditEngine->GetDocPosTopLeft( nPara ).Y();
        const ParaPortion* pParaPortion = pEditEngine->GetParaPortions()[nPara];
        nY += pParaPortion->GetFirstLineOffset();
        if ( ( aDocPos.Y() > ( nY + aBulletArea.Top() ) ) &&
             ( aDocPos.Y() < ( nY + aBulletArea.Bottom() ) ) &&
             ( aDocPos.X() > ( aBulletArea.Left() ) ) &&
             ( aDocPos.X() < ( aBulletArea.Right() ) ) )
        {
            if ( pPara )
                *pPara = nPara;
            return true;
        }
    }

    return false;
}

void ImpEditView::CutCopy( css::uno::Reference< css::datatransfer::clipboard::XClipboard > const & rxClipboard, bool bCut )
{
    if ( !(rxClipboard.is() && HasSelection()) )
        return;

    uno::Reference<datatransfer::XTransferable> xData = pEditEngine->CreateTransferable( GetEditSelection() );

    {
        SolarMutexReleaser aReleaser;

        try
            {
                rxClipboard->setContents( xData, nullptr );

                // #87756# FlushClipboard, but it would be better to become a TerminateListener to the Desktop and flush on demand...
                uno::Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( rxClipboard, uno::UNO_QUERY );
                if( xFlushableClipboard.is() )
                    xFlushableClipboard->flushClipboard();
            }
        catch( const css::uno::Exception& )
            {
            }

    }

    if (bCut)
    {
        pEditEngine->pImpEditEngine->UndoActionStart(EDITUNDO_CUT);
        DeleteSelected();
        pEditEngine->pImpEditEngine->UndoActionEnd();
    }
}

void ImpEditView::Paste( css::uno::Reference< css::datatransfer::clipboard::XClipboard > const & rxClipboard, bool bUseSpecial )
{
    if ( !rxClipboard.is() )
        return;

    uno::Reference< datatransfer::XTransferable > xDataObj;

    try
    {
        SolarMutexReleaser aReleaser;
        xDataObj = rxClipboard->getContents();
    }
    catch( const css::uno::Exception& )
    {
    }

    if ( !xDataObj.is() || !EditEngine::HasValidData( xDataObj ) )
        return;

    pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_PASTE );

    EditSelection aSel( GetEditSelection() );
    if ( aSel.HasRange() )
    {
        DrawSelectionXOR();
        aSel = pEditEngine->DeleteSelection(aSel);
    }

    PasteOrDropInfos aPasteOrDropInfos;
    aPasteOrDropInfos.nStartPara = pEditEngine->GetEditDoc().GetPos( aSel.Min().GetNode() );
    pEditEngine->HandleBeginPasteOrDrop(aPasteOrDropInfos);

    if ( DoSingleLinePaste() )
    {
        datatransfer::DataFlavor aFlavor;
        SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aFlavor );
        if ( xDataObj->isDataFlavorSupported( aFlavor ) )
        {
            try
            {
                uno::Any aData = xDataObj->getTransferData( aFlavor );
                OUString aTmpText;
                aData >>= aTmpText;
                OUString aText(convertLineEnd(aTmpText, LINEEND_LF));
                aText = aText.replaceAll( OUStringChar(LINE_SEP), " " );
                aSel = pEditEngine->InsertText(aSel, aText);
            }
            catch( ... )
            {
                ; // #i9286# can happen, even if isDataFlavorSupported returns true...
            }
        }
    }
    else
    {
        // Prevent notifications of paragraph inserts et al that would trigger
        // a11y to format content in a half-ready state when obtaining
        // paragraphs. Collect and broadcast when done instead.
        aSel = pEditEngine->InsertText(
            xDataObj, OUString(), aSel.Min(),
            bUseSpecial && pEditEngine->GetInternalEditStatus().AllowPasteSpecial());
    }

    aPasteOrDropInfos.nEndPara = pEditEngine->GetEditDoc().GetPos( aSel.Max().GetNode() );
    pEditEngine->HandleEndPasteOrDrop(aPasteOrDropInfos);

    pEditEngine->pImpEditEngine->UndoActionEnd();
    SetEditSelection( aSel );
    pEditEngine->pImpEditEngine->UpdateSelections();
    pEditEngine->pImpEditEngine->FormatAndUpdate( GetEditViewPtr() );
    ShowCursor( DoAutoScroll(), true );
}


bool ImpEditView::IsInSelection( const EditPaM& rPaM )
{
    EditSelection aSel = GetEditSelection();
    if ( !aSel.HasRange() )
        return false;

    aSel.Adjust( pEditEngine->GetEditDoc() );

    sal_Int32 nStartNode = pEditEngine->GetEditDoc().GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = pEditEngine->GetEditDoc().GetPos( aSel.Max().GetNode() );
    sal_Int32 nCurNode = pEditEngine->GetEditDoc().GetPos( rPaM.GetNode() );

    if ( ( nCurNode > nStartNode ) && ( nCurNode < nEndNode ) )
        return true;

    if ( nStartNode == nEndNode )
    {
        if ( nCurNode == nStartNode )
            if ( ( rPaM.GetIndex() >= aSel.Min().GetIndex() ) && ( rPaM.GetIndex() < aSel.Max().GetIndex() ) )
                return true;
    }
    else if ( ( nCurNode == nStartNode ) && ( rPaM.GetIndex() >= aSel.Min().GetIndex() ) )
        return true;
    else if ( ( nCurNode == nEndNode ) && ( rPaM.GetIndex() < aSel.Max().GetIndex() ) )
        return true;

    return false;
}

void ImpEditView::CreateAnchor()
{
    pEditEngine->SetInSelectionMode(true);
    EditSelection aNewSelection(GetEditSelection());
    aNewSelection.Min() = aNewSelection.Max();
    SetEditSelection(aNewSelection);
    // const_cast<EditPaM&>(GetEditSelection().Min()) = GetEditSelection().Max();
}

void ImpEditView::DeselectAll()
{
    pEditEngine->SetInSelectionMode(false);
    DrawSelectionXOR();
    EditSelection aNewSelection(GetEditSelection());
    aNewSelection.Min() = aNewSelection.Max();
    SetEditSelection(aNewSelection);
    // const_cast<EditPaM&>(GetEditSelection().Min()) = GetEditSelection().Max();

    if (comphelper::LibreOfficeKit::isActive() && mpViewShell)
    {
        VclPtr<vcl::Window> pParent = pOutWin->GetParentWithLOKNotifier();
        if (pParent && pParent->GetLOKWindowId())
        {
            const vcl::ILibreOfficeKitNotifier* pNotifier = pParent->GetLOKNotifier();
            std::vector<vcl::LOKPayloadItem> aItems;
            aItems.emplace_back("rectangles", "");
            pNotifier->notifyWindow(pParent->GetLOKWindowId(), "text_selection", aItems);
        }
    }
}

bool ImpEditView::IsSelectionAtPoint( const Point& rPosPixel )
{
    if ( pDragAndDropInfo && pDragAndDropInfo->pField )
        return true;

    // Logical units ...
    const OutputDevice& rOutDev = GetOutputDevice();
    Point aMousePos = rOutDev.PixelToLogic(rPosPixel);

    if ( ( !GetOutputArea().IsInside( aMousePos ) ) && !pEditEngine->pImpEditEngine->IsInSelectionMode() )
    {
        return false;
    }

    Point aDocPos( GetDocPos( aMousePos ) );
    EditPaM aPaM = pEditEngine->GetPaM(aDocPos, false);
    return IsInSelection( aPaM );
}

bool ImpEditView::SetCursorAtPoint( const Point& rPointPixel )
{
    pEditEngine->CheckIdleFormatter();

    Point aMousePos( rPointPixel );

    // Logical units ...
    const OutputDevice& rOutDev = GetOutputDevice();
    aMousePos = rOutDev.PixelToLogic( aMousePos );

    if ( ( !GetOutputArea().IsInside( aMousePos ) ) && !pEditEngine->pImpEditEngine->IsInSelectionMode() )
    {
        return false;
    }

    Point aDocPos( GetDocPos( aMousePos ) );

    // Can be optimized: first go through the lines within a paragraph for PAM,
    // then again with the PaM for the Rect, even though the line is already
    // known... This must not be, though!
    EditPaM aPaM = pEditEngine->GetPaM(aDocPos);
    bool bGotoCursor = DoAutoScroll();

    // aTmpNewSel: Diff between old and new, not the new selection, unless for tiled rendering
    EditSelection aTmpNewSel( comphelper::LibreOfficeKit::isActive() ? GetEditSelection().Min() : GetEditSelection().Max(), aPaM );

    // #i27299#
    // work on copy of current selection and set new selection, if it has changed.
    EditSelection aNewEditSelection( GetEditSelection() );

    aNewEditSelection.Max() = aPaM;
    if (!pEditEngine->GetSelectionEngine().HasAnchor())
    {
        if ( aNewEditSelection.Min() != aPaM )
            pEditEngine->CursorMoved(aNewEditSelection.Min().GetNode());
        aNewEditSelection.Min() = aPaM;
    }
    else
    {
        DrawSelectionXOR( aTmpNewSel );
    }

    // set changed text selection
    if ( GetEditSelection() != aNewEditSelection )
    {
        SetEditSelection( aNewEditSelection );
    }

    bool bForceCursor = pDragAndDropInfo == nullptr && !pEditEngine->pImpEditEngine->IsInSelectionMode();
    ShowCursor( bGotoCursor, bForceCursor );
    return true;
}

void ImpEditView::HideDDCursor()
{
    if ( pDragAndDropInfo && pDragAndDropInfo->bVisCursor )
    {
        OutputDevice& rOutDev = GetOutputDevice();
        rOutDev.DrawOutDev( pDragAndDropInfo->aCurSavedCursor.TopLeft(), pDragAndDropInfo->aCurSavedCursor.GetSize(),
                            Point(0,0), pDragAndDropInfo->aCurSavedCursor.GetSize(),*pDragAndDropInfo->pBackground );
        pDragAndDropInfo->bVisCursor = false;
    }
}

void ImpEditView::ShowDDCursor( const tools::Rectangle& rRect )
{
    if ( !pDragAndDropInfo || pDragAndDropInfo->bVisCursor )
        return;

    if (pOutWin && pOutWin->GetCursor())
        pOutWin->GetCursor()->Hide();

    OutputDevice& rOutDev = GetOutputDevice();
    Color aOldFillColor = rOutDev.GetFillColor();
    rOutDev.SetFillColor( Color(4210752) );    // GRAY BRUSH_50, OLDSV, change to DDCursor!

    // Save background ...
    tools::Rectangle aSaveRect( rOutDev.LogicToPixel( rRect ) );
    // prefer to save some more ...
    aSaveRect.AdjustRight(1 );
    aSaveRect.AdjustBottom(1 );

#ifdef DBG_UTIL
    Size aNewSzPx( aSaveRect.GetSize() );
#endif
    if ( !pDragAndDropInfo->pBackground )
    {
        pDragAndDropInfo->pBackground = VclPtr<VirtualDevice>::Create(rOutDev);
        MapMode aMapMode( rOutDev.GetMapMode() );
        aMapMode.SetOrigin( Point( 0, 0 ) );
        pDragAndDropInfo->pBackground->SetMapMode( aMapMode );

    }

#ifdef DBG_UTIL
    Size aCurSzPx( pDragAndDropInfo->pBackground->GetOutputSizePixel() );
    if ( ( aCurSzPx.Width() < aNewSzPx.Width() ) ||( aCurSzPx.Height() < aNewSzPx.Height() ) )
    {
        bool bDone = pDragAndDropInfo->pBackground->SetOutputSizePixel( aNewSzPx );
        DBG_ASSERT( bDone, "Virtual Device broken?" );
    }
#endif

    aSaveRect = rOutDev.PixelToLogic( aSaveRect );

    pDragAndDropInfo->pBackground->DrawOutDev( Point(0,0), aSaveRect.GetSize(),
                                aSaveRect.TopLeft(), aSaveRect.GetSize(), rOutDev );
    pDragAndDropInfo->aCurSavedCursor = aSaveRect;

    // Draw Cursor...
    rOutDev.DrawRect( rRect );

    pDragAndDropInfo->bVisCursor = true;
    pDragAndDropInfo->aCurCursor = rRect;

    rOutDev.SetFillColor( aOldFillColor );
}

void ImpEditView::dragGestureRecognized(const css::datatransfer::dnd::DragGestureEvent& rDGE)
{
    DBG_ASSERT( !pDragAndDropInfo, "dragGestureRecognized - DragAndDropInfo exist!" );

    SolarMutexGuard aVclGuard;

    pDragAndDropInfo.reset();

    Point aMousePosPixel( rDGE.DragOriginX, rDGE.DragOriginY );

    EditSelection aCopySel( GetEditSelection() );
    aCopySel.Adjust( pEditEngine->GetEditDoc() );

    if ( HasSelection() && bClickedInSelection )
    {
        pDragAndDropInfo.reset(new DragAndDropInfo());
    }
    else
    {
        // Field?!
        sal_Int32 nPara;
        sal_Int32 nPos;
        Point aMousePos = GetOutputDevice().PixelToLogic( aMousePosPixel );
        const SvxFieldItem* pField = GetField( aMousePos, &nPara, &nPos );
        if ( pField )
        {
            pDragAndDropInfo.reset(new DragAndDropInfo());
            pDragAndDropInfo->pField = pField;
            ContentNode* pNode = pEditEngine->GetEditDoc().GetObject( nPara );
            aCopySel = EditSelection( EditPaM( pNode, nPos ), EditPaM( pNode, nPos+1 ) );
            SetEditSelection(aCopySel);
            DrawSelectionXOR();
            bool bGotoCursor = DoAutoScroll();
            ShowCursor( bGotoCursor, /*bForceCursor=*/false );
        }
        else if ( IsBulletArea( aMousePos, &nPara ) )
        {
            pDragAndDropInfo.reset(new DragAndDropInfo());
            pDragAndDropInfo->bOutlinerMode = true;
            EditPaM aStartPaM( pEditEngine->GetEditDoc().GetObject( nPara ), 0 );
            EditPaM aEndPaM( aStartPaM );
            const SfxInt16Item& rLevel = pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
            for ( sal_Int32 n = nPara +1; n < pEditEngine->GetEditDoc().Count(); n++ )
            {
                const SfxInt16Item& rL = pEditEngine->GetParaAttrib( n, EE_PARA_OUTLLEVEL );
                if ( rL.GetValue() > rLevel.GetValue() )
                {
                    aEndPaM.SetNode( pEditEngine->GetEditDoc().GetObject( n ) );
                }
                else
                {
                    break;
                }
            }
            aEndPaM.SetIndex( aEndPaM.GetNode()->Len() );
            SetEditSelection( EditSelection( aStartPaM, aEndPaM ) );
        }
    }

    if ( !pDragAndDropInfo )
        return;


    pDragAndDropInfo->bStarterOfDD = true;

    // Sensitive area to be scrolled.
    Size aSz( 5, 0 );
    aSz = GetOutputDevice().PixelToLogic( aSz );
    pDragAndDropInfo->nSensibleRange = static_cast<sal_uInt16>(aSz.Width());
    pDragAndDropInfo->nCursorWidth = static_cast<sal_uInt16>(aSz.Width()) / 2;
    pDragAndDropInfo->aBeginDragSel = pEditEngine->pImpEditEngine->CreateESel( aCopySel );

    uno::Reference<datatransfer::XTransferable> xData = pEditEngine->CreateTransferable(aCopySel);

    sal_Int8 nActions = bReadOnly ? datatransfer::dnd::DNDConstants::ACTION_COPY : datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE;

    rDGE.DragSource->startDrag( rDGE, nActions, 0 /*cursor*/, 0 /*image*/, xData, mxDnDListener );
    // If Drag&Move in an Engine, then Copy&Del has to be optional!
    GetCursor()->Hide();
}

void ImpEditView::dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& rDSDE )
{
    SolarMutexGuard aVclGuard;

    DBG_ASSERT( pDragAndDropInfo, "ImpEditView::dragDropEnd: pDragAndDropInfo is NULL!" );

    // #123688# Shouldn't happen, but seems to happen...
    if ( !pDragAndDropInfo )
        return;

    if ( !bReadOnly && rDSDE.DropSuccess && !pDragAndDropInfo->bOutlinerMode && ( rDSDE.DropAction & datatransfer::dnd::DNDConstants::ACTION_MOVE ) )
    {
        if ( pDragAndDropInfo->bStarterOfDD && pDragAndDropInfo->bDroppedInMe )
        {
            // DropPos: Where was it dropped, irrespective of length.
            ESelection aDropPos( pDragAndDropInfo->aDropSel.nStartPara, pDragAndDropInfo->aDropSel.nStartPos, pDragAndDropInfo->aDropSel.nStartPara, pDragAndDropInfo->aDropSel.nStartPos );
            ESelection aToBeDelSel = pDragAndDropInfo->aBeginDragSel;
            ESelection aNewSel( pDragAndDropInfo->aDropSel.nEndPara, pDragAndDropInfo->aDropSel.nEndPos,
                                pDragAndDropInfo->aDropSel.nEndPara, pDragAndDropInfo->aDropSel.nEndPos );
            bool bBeforeSelection = aDropPos < pDragAndDropInfo->aBeginDragSel;
            sal_Int32 nParaDiff = pDragAndDropInfo->aBeginDragSel.nEndPara - pDragAndDropInfo->aBeginDragSel.nStartPara;
            if ( bBeforeSelection )
            {
                // Adjust aToBeDelSel.
                DBG_ASSERT( pDragAndDropInfo->aBeginDragSel.nStartPara >= pDragAndDropInfo->aDropSel.nStartPara, "But not before? ");
                aToBeDelSel.nStartPara = aToBeDelSel.nStartPara + nParaDiff;
                aToBeDelSel.nEndPara = aToBeDelSel.nEndPara + nParaDiff;
                // To correct the character?
                if ( aToBeDelSel.nStartPara == pDragAndDropInfo->aDropSel.nEndPara )
                {
                    sal_uInt16 nMoreChars;
                    if ( pDragAndDropInfo->aDropSel.nStartPara == pDragAndDropInfo->aDropSel.nEndPara )
                        nMoreChars = pDragAndDropInfo->aDropSel.nEndPos - pDragAndDropInfo->aDropSel.nStartPos;
                    else
                        nMoreChars = pDragAndDropInfo->aDropSel.nEndPos;
                    aToBeDelSel.nStartPos =
                        aToBeDelSel.nStartPos + nMoreChars;
                    if ( aToBeDelSel.nStartPara == aToBeDelSel.nEndPara )
                        aToBeDelSel.nEndPos =
                            aToBeDelSel.nEndPos + nMoreChars;
                }
            }
            else
            {
                // aToBeDelSel is ok, but the selection of the  View
                // has to be adapted, if it was deleted before!
                DBG_ASSERT( pDragAndDropInfo->aBeginDragSel.nStartPara <= pDragAndDropInfo->aDropSel.nStartPara, "But not before? ");
                aNewSel.nStartPara = aNewSel.nStartPara - nParaDiff;
                aNewSel.nEndPara = aNewSel.nEndPara - nParaDiff;
                // To correct the character?
                if ( pDragAndDropInfo->aBeginDragSel.nEndPara == pDragAndDropInfo->aDropSel.nStartPara )
                {
                    sal_uInt16 nLessChars;
                    if ( pDragAndDropInfo->aBeginDragSel.nStartPara == pDragAndDropInfo->aBeginDragSel.nEndPara )
                        nLessChars = pDragAndDropInfo->aBeginDragSel.nEndPos - pDragAndDropInfo->aBeginDragSel.nStartPos;
                    else
                        nLessChars = pDragAndDropInfo->aBeginDragSel.nEndPos;
                    aNewSel.nStartPos = aNewSel.nStartPos - nLessChars;
                    if ( aNewSel.nStartPara == aNewSel.nEndPara )
                        aNewSel.nEndPos = aNewSel.nEndPos - nLessChars;
                }
            }

            DrawSelectionXOR();
            EditSelection aDelSel( pEditEngine->pImpEditEngine->CreateSel( aToBeDelSel ) );
            DBG_ASSERT( !aDelSel.DbgIsBuggy( pEditEngine->GetEditDoc() ), "ToBeDel is buggy!" );
            pEditEngine->DeleteSelection(aDelSel);
            if ( !bBeforeSelection )
            {
                DBG_ASSERT( !pEditEngine->pImpEditEngine->CreateSel( aNewSel ).DbgIsBuggy(pEditEngine->GetEditDoc()), "Bad" );
                SetEditSelection( pEditEngine->pImpEditEngine->CreateSel( aNewSel ) );
            }
            pEditEngine->pImpEditEngine->FormatAndUpdate( pEditEngine->pImpEditEngine->GetActiveView() );
            DrawSelectionXOR();
        }
        else
        {
            // other EditEngine ...
            if (pEditEngine->HasText())   // #88630# SC is removing the content when switching the task
                DeleteSelected();
        }
    }

    if ( pDragAndDropInfo->bUndoAction )
        pEditEngine->pImpEditEngine->UndoActionEnd();

    HideDDCursor();
    ShowCursor( DoAutoScroll(), true );
    pDragAndDropInfo.reset();
    pEditEngine->GetEndDropHdl().Call(GetEditViewPtr());
}

void ImpEditView::drop( const css::datatransfer::dnd::DropTargetDropEvent& rDTDE )
{
    SolarMutexGuard aVclGuard;

    DBG_ASSERT( pDragAndDropInfo, "Drop - No Drag&Drop info?!" );

    if ( !(pDragAndDropInfo && pDragAndDropInfo->bDragAccepted) )
        return;

    pEditEngine->GetBeginDropHdl().Call(GetEditViewPtr());
    bool bChanges = false;

    HideDDCursor();

    if ( pDragAndDropInfo->bStarterOfDD )
    {
        pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_DRAGANDDROP );
        pDragAndDropInfo->bUndoAction = true;
    }

    if ( pDragAndDropInfo->bOutlinerMode )
    {
        bChanges = true;
        GetEditViewPtr()->MoveParagraphs( Range( pDragAndDropInfo->aBeginDragSel.nStartPara, pDragAndDropInfo->aBeginDragSel.nEndPara ), pDragAndDropInfo->nOutlinerDropDest );
    }
    else
    {
        uno::Reference< datatransfer::XTransferable > xDataObj = rDTDE.Transferable;
        if ( xDataObj.is() )
        {
            bChanges = true;
            // remove Selection ...
            DrawSelectionXOR();
            EditPaM aPaM( pDragAndDropInfo->aDropDest );

            PasteOrDropInfos aPasteOrDropInfos;
            aPasteOrDropInfos.nStartPara = pEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
            pEditEngine->HandleBeginPasteOrDrop(aPasteOrDropInfos);

            EditSelection aNewSel = pEditEngine->InsertText(
                xDataObj, OUString(), aPaM, pEditEngine->GetInternalEditStatus().AllowPasteSpecial());

            aPasteOrDropInfos.nEndPara = pEditEngine->GetEditDoc().GetPos( aNewSel.Max().GetNode() );
            pEditEngine->HandleEndPasteOrDrop(aPasteOrDropInfos);

            SetEditSelection( aNewSel );
            pEditEngine->pImpEditEngine->FormatAndUpdate( pEditEngine->pImpEditEngine->GetActiveView() );
            if ( pDragAndDropInfo->bStarterOfDD )
            {
                // Only set if the same engine!
                pDragAndDropInfo->aDropSel.nStartPara = pEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
                pDragAndDropInfo->aDropSel.nStartPos = aPaM.GetIndex();
                pDragAndDropInfo->aDropSel.nEndPara = pEditEngine->GetEditDoc().GetPos( aNewSel.Max().GetNode() );
                pDragAndDropInfo->aDropSel.nEndPos = aNewSel.Max().GetIndex();
                pDragAndDropInfo->bDroppedInMe = true;
            }
        }
    }

    if ( bChanges )
    {
        rDTDE.Context->acceptDrop( rDTDE.DropAction );
    }

    if ( !pDragAndDropInfo->bStarterOfDD )
    {
        pDragAndDropInfo.reset();
    }

    rDTDE.Context->dropComplete( bChanges );
}

void ImpEditView::dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& rDTDEE )
{
    SolarMutexGuard aVclGuard;

    if ( !pDragAndDropInfo )
        pDragAndDropInfo.reset(new DragAndDropInfo());

    pDragAndDropInfo->bHasValidData = false;

    // Check for supported format...
    // Only check for text, will also be there if bin or rtf
    datatransfer::DataFlavor aTextFlavor;
    SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aTextFlavor );
    const css::datatransfer::DataFlavor* pFlavors = rDTDEE.SupportedDataFlavors.getConstArray();
    int nFlavors = rDTDEE.SupportedDataFlavors.getLength();
    for ( int n = 0; n < nFlavors; n++ )
    {
        if( TransferableDataHelper::IsEqual( pFlavors[n], aTextFlavor ) )
        {
            pDragAndDropInfo->bHasValidData = true;
            break;
        }
    }

    dragOver( rDTDEE );
}

void ImpEditView::dragExit( const css::datatransfer::dnd::DropTargetEvent& )
{
    SolarMutexGuard aVclGuard;

    HideDDCursor();

    if ( pDragAndDropInfo && !pDragAndDropInfo->bStarterOfDD )
    {
        pDragAndDropInfo.reset();
    }
}

void ImpEditView::dragOver(const css::datatransfer::dnd::DropTargetDragEvent& rDTDE)
{
    SolarMutexGuard aVclGuard;

    const OutputDevice& rOutDev = GetOutputDevice();

    Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );
    aMousePos = rOutDev.PixelToLogic( aMousePos );

    bool bAccept = false;

    if ( GetOutputArea().IsInside( aMousePos ) && !bReadOnly )
    {
        if ( pDragAndDropInfo && pDragAndDropInfo->bHasValidData )
        {
            bAccept = true;

            bool bAllowScroll = DoAutoScroll();
            if ( bAllowScroll )
            {
                tools::Long nScrollX = 0;
                tools::Long nScrollY = 0;
                // Check if in the sensitive area
                if ( ( (aMousePos.X()-pDragAndDropInfo->nSensibleRange) < GetOutputArea().Left() ) && ( ( aMousePos.X() + pDragAndDropInfo->nSensibleRange ) > GetOutputArea().Left() ) )
                        nScrollX = GetOutputArea().GetWidth() / SCRLRANGE;
                else if ( ( (aMousePos.X()+pDragAndDropInfo->nSensibleRange) > GetOutputArea().Right() ) && ( ( aMousePos.X() - pDragAndDropInfo->nSensibleRange ) < GetOutputArea().Right() ) )
                        nScrollX = -( GetOutputArea().GetWidth() / SCRLRANGE );

                if ( ( (aMousePos.Y()-pDragAndDropInfo->nSensibleRange) < GetOutputArea().Top() ) && ( ( aMousePos.Y() + pDragAndDropInfo->nSensibleRange ) > GetOutputArea().Top() ) )
                        nScrollY = GetOutputArea().GetHeight() / SCRLRANGE;
                else if ( ( (aMousePos.Y()+pDragAndDropInfo->nSensibleRange) > GetOutputArea().Bottom() ) && ( ( aMousePos.Y() - pDragAndDropInfo->nSensibleRange ) < GetOutputArea().Bottom() ) )
                        nScrollY = -( GetOutputArea().GetHeight() / SCRLRANGE );

                if ( nScrollX || nScrollY )
                {
                    HideDDCursor();
                    Scroll( nScrollX, nScrollY, ScrollRangeCheck::PaperWidthTextSize );
                }
            }

            Point aDocPos( GetDocPos( aMousePos ) );
            EditPaM aPaM = pEditEngine->GetPaM( aDocPos );
            pDragAndDropInfo->aDropDest = aPaM;
            if ( pDragAndDropInfo->bOutlinerMode )
            {
                sal_Int32 nPara = pEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
                ParaPortion* pPPortion = pEditEngine->GetParaPortions().SafeGetObject( nPara );
                if (pPPortion)
                {
                    tools::Long nDestParaStartY = pEditEngine->GetParaPortions().GetYOffset( pPPortion );
                    tools::Long nRel = aDocPos.Y() - nDestParaStartY;
                    if ( nRel < ( pPPortion->GetHeight() / 2 ) )
                    {
                        pDragAndDropInfo->nOutlinerDropDest = nPara;
                    }
                    else
                    {
                        pDragAndDropInfo->nOutlinerDropDest = nPara+1;
                    }

                    if( ( pDragAndDropInfo->nOutlinerDropDest >= pDragAndDropInfo->aBeginDragSel.nStartPara ) &&
                            ( pDragAndDropInfo->nOutlinerDropDest <= (pDragAndDropInfo->aBeginDragSel.nEndPara+1) ) )
                    {
                        bAccept = false;
                    }
                }
            }
            else if ( HasSelection() )
            {
                // it must not be dropped into a selection
                EPaM aP = pEditEngine->pImpEditEngine->CreateEPaM( aPaM );
                ESelection aDestSel( aP.nPara, aP.nIndex, aP.nPara, aP.nIndex);
                ESelection aCurSel = pEditEngine->pImpEditEngine->CreateESel( GetEditSelection() );
                aCurSel.Adjust();
                if ( !(aDestSel < aCurSel) && !(aDestSel > aCurSel) )
                {
                    bAccept = false;
                }
            }
            if ( bAccept )
            {
                tools::Rectangle aEditCursor;
                if ( pDragAndDropInfo->bOutlinerMode )
                {
                    tools::Long nDDYPos(0);
                    if ( pDragAndDropInfo->nOutlinerDropDest < pEditEngine->GetEditDoc().Count() )
                    {
                        ParaPortion* pPPortion = pEditEngine->GetParaPortions().SafeGetObject( pDragAndDropInfo->nOutlinerDropDest );
                        if (pPPortion)
                            nDDYPos = pEditEngine->GetParaPortions().GetYOffset( pPPortion );
                    }
                    else
                    {
                        nDDYPos = pEditEngine->pImpEditEngine->GetTextHeight();
                    }
                    Point aStartPos( 0, nDDYPos );
                    aStartPos = GetWindowPos( aStartPos );
                    Point aEndPos( GetOutputArea().GetWidth(), nDDYPos );
                    aEndPos = GetWindowPos( aEndPos );
                    aEditCursor = rOutDev.LogicToPixel( tools::Rectangle( aStartPos, aEndPos ) );
                    if ( !pEditEngine->IsVertical() )
                    {
                        aEditCursor.AdjustTop( -1 );
                        aEditCursor.AdjustBottom( 1 );
                    }
                    else
                    {
                        if( IsTopToBottom() )
                        {
                            aEditCursor.AdjustLeft( -1 );
                            aEditCursor.AdjustRight( 1 );
                        }
                        else
                        {
                            aEditCursor.AdjustLeft( 1 );
                            aEditCursor.AdjustRight( -1 );
                        }
                    }
                    aEditCursor = rOutDev.PixelToLogic( aEditCursor );
                }
                else
                {
                    aEditCursor = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM );
                    Point aTopLeft( GetWindowPos( aEditCursor.TopLeft() ) );
                    aEditCursor.SetPos( aTopLeft );
                    aEditCursor.SetRight( aEditCursor.Left() + pDragAndDropInfo->nCursorWidth );
                    aEditCursor = rOutDev.LogicToPixel( aEditCursor );
                    aEditCursor = rOutDev.PixelToLogic( aEditCursor );
                }

                bool bCursorChanged = !pDragAndDropInfo->bVisCursor || ( pDragAndDropInfo->aCurCursor != aEditCursor );
                if ( bCursorChanged )
                {
                    HideDDCursor();
                    ShowDDCursor(aEditCursor );
                }
                pDragAndDropInfo->bDragAccepted = true;
                rDTDE.Context->acceptDrag( rDTDE.DropAction );
            }
        }
    }

    if ( !bAccept )
    {
        HideDDCursor();
        if (pDragAndDropInfo)
            pDragAndDropInfo->bDragAccepted = false;
        rDTDE.Context->rejectDrag();
    }
}

void ImpEditView::AddDragAndDropListeners()
{
    if (bActiveDragAndDropListener)
        return;

    css::uno::Reference<css::datatransfer::dnd::XDropTarget> xDropTarget;
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
        xDropTarget = pCallbacks->GetDropTarget();
    else if (GetWindow())
        xDropTarget = GetWindow()->GetDropTarget();

    if (!xDropTarget.is())
        return;

    vcl::unohelper::DragAndDropWrapper* pDnDWrapper = new vcl::unohelper::DragAndDropWrapper(this);
    mxDnDListener = pDnDWrapper;

    css::uno::Reference<css::datatransfer::dnd::XDragGestureRecognizer> xDragGestureRecognizer(xDropTarget, uno::UNO_QUERY);
    if (xDragGestureRecognizer.is())
    {
        uno::Reference<datatransfer::dnd::XDragGestureListener> xDGL(mxDnDListener, uno::UNO_QUERY);
        xDragGestureRecognizer->addDragGestureListener(xDGL);
    }

    uno::Reference<datatransfer::dnd::XDropTargetListener> xDTL(mxDnDListener, uno::UNO_QUERY);
    xDropTarget->addDropTargetListener(xDTL);
    xDropTarget->setActive(true);
    xDropTarget->setDefaultActions(datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE);

    bActiveDragAndDropListener = true;
}

void ImpEditView::RemoveDragAndDropListeners()
{
    if (!bActiveDragAndDropListener)
        return;

    css::uno::Reference<css::datatransfer::dnd::XDropTarget> xDropTarget;
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
        xDropTarget = pCallbacks->GetDropTarget();
    else if (GetWindow())
        xDropTarget = GetWindow()->GetDropTarget();

    if (!xDropTarget.is())
        return;

    css::uno::Reference<css::datatransfer::dnd::XDragGestureRecognizer> xDragGestureRecognizer(xDropTarget, uno::UNO_QUERY);
    if (xDragGestureRecognizer.is())
    {
        uno::Reference<datatransfer::dnd::XDragGestureListener> xDGL(mxDnDListener, uno::UNO_QUERY);
        xDragGestureRecognizer->removeDragGestureListener(xDGL);
    }

    uno::Reference<datatransfer::dnd::XDropTargetListener> xDTL(mxDnDListener, uno::UNO_QUERY);
    xDropTarget->removeDropTargetListener(xDTL);

    if ( mxDnDListener.is() )
    {
        mxDnDListener->disposing( lang::EventObject() );  // #95154# Empty Source means it's the Client
        mxDnDListener.clear();
    }

    bActiveDragAndDropListener = false;
}

void ImpEditView::InitLOKSpecialPositioning(MapUnit eUnit,
                                           const tools::Rectangle& rOutputArea,
                                           const Point& rVisDocStartPos)
{
    if (!mpLOKSpecialPositioning)
        mpLOKSpecialPositioning.reset(new LOKSpecialPositioning(*this, eUnit, rOutputArea, rVisDocStartPos));
    else
        mpLOKSpecialPositioning->ReInit(eUnit, rOutputArea, rVisDocStartPos);
}

void ImpEditView::SetLOKSpecialOutputArea(const tools::Rectangle& rOutputArea)
{
    assert(mpLOKSpecialPositioning);
    mpLOKSpecialPositioning->SetOutputArea(rOutputArea);
}

const tools::Rectangle & ImpEditView::GetLOKSpecialOutputArea() const
{
    assert(mpLOKSpecialPositioning);
    return mpLOKSpecialPositioning->GetOutputArea();
}

void ImpEditView::SetLOKSpecialVisArea(const tools::Rectangle& rVisArea)
{
    assert(mpLOKSpecialPositioning);
    mpLOKSpecialPositioning->SetVisDocStartPos(rVisArea.TopLeft());
}

tools::Rectangle ImpEditView::GetLOKSpecialVisArea() const
{
    assert(mpLOKSpecialPositioning);
    return mpLOKSpecialPositioning->GetVisDocArea();
}

bool ImpEditView::HasLOKSpecialPositioning() const
{
    return bool(mpLOKSpecialPositioning);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
