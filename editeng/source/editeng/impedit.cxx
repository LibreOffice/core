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
#include <editeng/urlfieldhelper.hxx>
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
#include <vcl/window.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/string.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/lokhelper.hxx>
#include <boost/property_tree/ptree.hpp>

using namespace css;

#define SCRLRANGE   20  // Scroll 1/20 of the width/height, when in QueryDrop

static void lcl_AlignToPixel(Point& rPoint, const OutputDevice& rOutDev, short nDiffX, short nDiffY)
{
    rPoint = rOutDev.LogicToPixel( rPoint );

    if ( nDiffX )
        rPoint.AdjustX(nDiffX );
    if ( nDiffY )
        rPoint.AdjustY(nDiffY );

    rPoint = rOutDev.PixelToLogic( rPoint );
}

LOKSpecialPositioning::LOKSpecialPositioning(const ImpEditView& rImpEditView, MapUnit eUnit,
                                             const tools::Rectangle& rOutputArea,
                                             const Point& rVisDocStartPos) :
                                             mrImpEditView(rImpEditView),
                                             maOutArea(rOutputArea),
                                             maVisDocStartPos(rVisDocStartPos),
                                             meUnit(eUnit),
                                             meFlags(LOKSpecialFlags::NONE)
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

ImpEditView::ImpEditView(EditView* pView, EditEngine* pEditEngine, vcl::Window* pWindow)
    : mpEditView(pView)
    , mpViewShell(nullptr)
    , mpOtherShell(nullptr)
    , mpEditEngine(pEditEngine)
    , mpOutputWindow(pWindow)
    , mnInvalidateMore(1)
    , mnControl(EVControlBits::AUTOSCROLL | EVControlBits::ENABLEPASTE)
    , mnTravelXPos(TRAVEL_X_DONTKNOW)
    , mnCursorBidiLevel(CURSOR_BIDILEVEL_DONTKNOW)
    , mnScrollDiffX(0)
    , mbReadOnly(false)
    , mbClickedInSelection(false)
    , mbActiveDragAndDropListener(false)
    , maOutputArea(Point(), mpEditEngine->GetPaperSize())
    , meSelectionMode(EESelectionMode::Std)
    , meAnchorMode(EEAnchorMode::TopLeft)
    , mpEditViewCallbacks(nullptr)
    , mbBroadcastLOKViewCursor(comphelper::LibreOfficeKit::isActive())
    , mbSuppressLOKMessages(false)
    , mbNegativeX(false)
{
    maEditSelection.Min() = mpEditEngine->GetEditDoc().GetStartPaM();
    maEditSelection.Max() = mpEditEngine->GetEditDoc().GetEndPaM();

    SelectionChanged();
}

ImpEditView::~ImpEditView()
{
    RemoveDragAndDropListeners();

    if (mpOutputWindow && (mpOutputWindow->GetCursor() == mpCursor.get()))
        mpOutputWindow->SetCursor( nullptr );
}

void ImpEditView::SetBackgroundColor( const Color& rColor )
{
    mxBackgroundColor = rColor;
}

const Color& ImpEditView::GetBackgroundColor() const
{
    return mxBackgroundColor ? *mxBackgroundColor : GetOutputDevice().GetBackground().GetColor();
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
    maEditSelection = rEditSelection;

    SelectionChanged();

    if (comphelper::LibreOfficeKit::isActive())
    {
        // Tiled rendering: selections are only painted when we are in selection mode.
        getEditEngine().SetInSelectionMode(maEditSelection.HasRange());
    }

    if (getImpEditEngine().GetNotifyHdl().IsSet() )
    {
        const EditDoc& rDoc = getEditEngine().GetEditDoc();
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
        getImpEditEngine().GetNotifyHdl().Call( aNotify );
    }

    if (getImpEditEngine().IsFormatted())
    {
        EENotify aNotify(EE_NOTIFY_PROCESSNOTIFICATIONS);
        getImpEditEngine().GetNotifyHdl().Call(aNotify);
    }
}

/// Translate absolute <-> relative twips: LOK wants absolute coordinates as output and gives absolute coordinates as input.
static void lcl_translateTwips(const OutputDevice& rParent, OutputDevice& rChild)
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
void ImpEditView::lokSelectionCallback(const std::optional<tools::PolyPolygon> &pPolyPoly, bool bStartHandleVisible, bool bEndHandleVisible) {
    VclPtr<vcl::Window> pParent = mpOutputWindow->GetParentWithLOKNotifier();
    vcl::Region aRegion( *pPolyPoly );

    if (pParent && pParent->GetLOKWindowId() != 0)
    {
        const tools::Long nX = mpOutputWindow->GetOutOffXPixel() - pParent->GetOutOffXPixel();
        const tools::Long nY = mpOutputWindow->GetOutOffYPixel() - pParent->GetOutOffYPixel();

        std::vector<tools::Rectangle> aRectangles;
        aRegion.GetRegionRectangles(aRectangles);

        std::vector<OString> v;
        for (tools::Rectangle & rRectangle : aRectangles)
        {
            rRectangle = mpOutputWindow->LogicToPixel(rRectangle);
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
    else if (mpViewShell)
    {
        mpOutputWindow->GetOutDev()->Push(vcl::PushFlags::MAPMODE);
        if (mpOutputWindow->GetMapMode().GetMapUnit() == MapUnit::MapTwip)
        {
            // Find the parent that is not right
            // on top of us to use its offset.
            vcl::Window* parent = mpOutputWindow->GetParent();
            while (parent &&
                    parent->GetOutOffXPixel() == mpOutputWindow->GetOutOffXPixel() &&
                    parent->GetOutOffYPixel() == mpOutputWindow->GetOutOffYPixel())
            {
                parent = parent->GetParent();
            }

            if (parent)
            {
                lcl_translateTwips(*parent->GetOutDev(), *mpOutputWindow->GetOutDev());
            }
        }

        bool bMm100ToTwip = !mpLOKSpecialPositioning && (mpOutputWindow->GetMapMode().GetMapUnit() == MapUnit::Map100thMM);

        Point aOrigin;
        if (mpOutputWindow->GetMapMode().GetMapUnit() == MapUnit::MapTwip)
            // Writer comments: they use editeng, but are separate widgets.
            aOrigin = mpOutputWindow->GetMapMode().GetOrigin();

        OString sRectangle;
        OString sRefPoint;
        if (mpLOKSpecialPositioning)
            sRefPoint = mpLOKSpecialPositioning->GetRefPoint().toString();

        std::vector<tools::Rectangle> aRectangles;
        aRegion.GetRegionRectangles(aRectangles);

        if (!aRectangles.empty())
        {
            if (mpOutputWindow->IsChart())
            {
                const vcl::Window* pViewShellWindow = mpViewShell->GetEditWindowForActiveOLEObj();
                if (pViewShellWindow && pViewShellWindow->IsAncestorOf(*mpOutputWindow))
                {
                    Point aOffsetPx = mpOutputWindow->GetOffsetPixelFrom(*pViewShellWindow);
                    Point aLogicOffset = mpOutputWindow->PixelToLogic(aOffsetPx);
                    for (tools::Rectangle& rRect : aRectangles)
                        rRect.Move(aLogicOffset.getX(), aLogicOffset.getY());
                }
            }

            std::vector<OString> v;
            for (tools::Rectangle & rRectangle : aRectangles)
            {
                if (bMm100ToTwip)
                {
                    rRectangle = o3tl::convert(rRectangle, o3tl::Length::mm100, o3tl::Length::twip);
                }
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

            mpViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_START, aPayload);

            tools::Rectangle& rEnd = aRectangles.back();
            tools::Rectangle aEnd(rEnd.Right() - 1, rEnd.Top(), rEnd.Right(), rEnd.Bottom());

            aPayload = aEnd.toString();
            if (mpLOKSpecialPositioning)
                aPayload += ":: " + sRefPoint;

            mpViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_END, aPayload);
        }

        if (mpOtherShell)
        {
            // Another shell wants to know about our existing selection.
            if (mpViewShell != mpOtherShell)
                mpViewShell->NotifyOtherView(mpOtherShell, LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection"_ostr, sRectangle);
        }
        else
        {
            mpViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, sRectangle);
            mpViewShell->NotifyOtherViews(LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection"_ostr, sRectangle);
        }

        mpOutputWindow->GetOutDev()->Pop();
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

    if (meSelectionMode == EESelectionMode::Hidden)
        return;

    // It must be ensured before rendering the selection, that the contents of
    // the window is completely valid! Must be here so that in any case if
    // empty, then later on two-Paint Events! Must be done even before the
    // query from bUpdate, if after Invalidate paints still in the queue,
    // but someone switches the update mode!

    // pRegion: When not NULL, then only calculate Region.

    OutputDevice& rTarget = pTargetDevice ? *pTargetDevice : GetOutputDevice();
    bool bClipRegion = rTarget.IsClipRegion();
    vcl::Region aOldRegion = rTarget.GetClipRegion();

    std::optional<tools::PolyPolygon> pPolyPoly;

    if ( !pRegion && !comphelper::LibreOfficeKit::isActive())
    {
        if (!getImpEditEngine().IsUpdateLayout())
            return;
        if (getImpEditEngine().IsInUndo())
            return;

        if ( !aTmpSel.HasRange() )
            return;

        // aTmpOutArea: if OutputArea > Paper width and
        // Text > Paper width ( over large fields )
        tools::Rectangle aTmpOutArea(maOutputArea);
        if ( aTmpOutArea.GetWidth() > getImpEditEngine().GetPaperSize().Width() )
            aTmpOutArea.SetRight( aTmpOutArea.Left() + getImpEditEngine().GetPaperSize().Width() );
        rTarget.IntersectClipRegion( aTmpOutArea );

        if (mpOutputWindow && mpOutputWindow->GetCursor())
            mpOutputWindow->GetCursor()->Hide();
    }

    if (comphelper::LibreOfficeKit::isActive() || pRegion)
        pPolyPoly = tools::PolyPolygon();

    DBG_ASSERT(!getEditEngine().IsIdleFormatterActive(), "DrawSelectionXOR: Not formatted!");
    aTmpSel.Adjust(getEditEngine().GetEditDoc());

    ContentNode* pStartNode = aTmpSel.Min().GetNode();
    ContentNode* pEndNode = aTmpSel.Max().GetNode();
    const sal_Int32 nStartPara = getEditEngine().GetEditDoc().GetPos(pStartNode);
    const sal_Int32 nEndPara = getEditEngine().GetEditDoc().GetPos(pEndNode);
    if (nStartPara == EE_PARA_NOT_FOUND || nEndPara == EE_PARA_NOT_FOUND)
        return;

    bool bStartHandleVisible = false;
    bool bEndHandleVisible = false;
    bool bLOKCalcRTL = mpLOKSpecialPositioning &&
        (mpLOKSpecialPositioning->IsLayoutRTL() || getEditEngine().IsRightToLeft(nStartPara));

    auto DrawHighlight = [&, nStartLine = sal_Int32(0), nEndLine = sal_Int32(0)](
                             const ImpEditEngine::LineAreaInfo& rInfo) mutable {
        if (!rInfo.pLine) // Begin of ParaPortion
        {
            if (rInfo.nPortion < nStartPara)
                return ImpEditEngine::CallbackResult::SkipThisPortion;
            if (rInfo.nPortion > nEndPara)
                return ImpEditEngine::CallbackResult::Stop;
            DBG_ASSERT(!rInfo.rPortion.IsInvalid(), "Portion in Selection not formatted!");
            if (rInfo.rPortion.IsInvalid())
                return ImpEditEngine::CallbackResult::SkipThisPortion;

            if (rInfo.nPortion == nStartPara)
                nStartLine = rInfo.rPortion.GetLines().FindLine(aTmpSel.Min().GetIndex(), false);
            else
                nStartLine = 0;

            if (rInfo.nPortion == nEndPara)
                nEndLine = rInfo.rPortion.GetLines().FindLine(aTmpSel.Max().GetIndex(), true);
            else
                nEndLine = rInfo.rPortion.GetLines().Count() - 1;
        }
        else // This is a correct ParaPortion
        {
            if (rInfo.nLine < nStartLine)
                return ImpEditEngine::CallbackResult::Continue;
            if (rInfo.nLine > nEndLine)
                return ImpEditEngine::CallbackResult::SkipThisPortion;

            bool bPartOfLine = false;
            sal_Int32 nStartIndex = rInfo.pLine->GetStart();
            sal_Int32 nEndIndex = rInfo.pLine->GetEnd();
            if ((rInfo.nPortion == nStartPara) && (rInfo.nLine == nStartLine)
                && (nStartIndex != aTmpSel.Min().GetIndex()))
            {
                nStartIndex = aTmpSel.Min().GetIndex();
                bPartOfLine = true;
            }
            if ((rInfo.nPortion == nEndPara) && (rInfo.nLine == nEndLine)
                && (nEndIndex != aTmpSel.Max().GetIndex()))
            {
                nEndIndex = aTmpSel.Max().GetIndex();
                bPartOfLine = true;
            }

            // Can happen if at the beginning of a wrapped line.
            if (nEndIndex < nStartIndex)
                nEndIndex = nStartIndex;

            tools::Rectangle aTmpRect(getImpEditEngine().GetEditCursor(rInfo.rPortion, *rInfo.pLine, nStartIndex, CursorFlags()));
            const Size aLineOffset = getImpEditEngine().getTopLeftDocOffset(rInfo.aArea);
            aTmpRect.Move(0, aLineOffset.Height());

            // Only paint if in the visible range ...
            if (aTmpRect.Top() > GetVisDocBottom())
                return ImpEditEngine::CallbackResult::Continue;

            if (aTmpRect.Bottom() < GetVisDocTop())
                return ImpEditEngine::CallbackResult::Continue;

            if ((rInfo.nPortion == nStartPara) && (rInfo.nLine == nStartLine))
                bStartHandleVisible = true;
            if ((rInfo.nPortion == nEndPara) && (rInfo.nLine == nEndLine))
                bEndHandleVisible = true;

            // Now that we have Bidi, the first/last index doesn't have to be the 'most outside' position
            if (!bPartOfLine)
            {
                Range aLineXPosStartEnd = getEditEngine().GetLineXPosStartEnd(rInfo.rPortion, *rInfo.pLine);
                aTmpRect.SetLeft(aLineXPosStartEnd.Min());
                aTmpRect.SetRight(aLineXPosStartEnd.Max());
                aTmpRect.Move(aLineOffset.Width(), 0);
                ImplDrawHighlightRect(rTarget, aTmpRect.TopLeft(), aTmpRect.BottomRight(),
                                      pPolyPoly ? &*pPolyPoly : nullptr, bLOKCalcRTL);
            }
            else
            {
                sal_Int32 nTmpStartIndex = nStartIndex;
                sal_Int32 nWritingDirStart, nTmpEndIndex;

                while (nTmpStartIndex < nEndIndex)
                {
                    getImpEditEngine().GetRightToLeft(rInfo.nPortion, nTmpStartIndex + 1,
                                                                &nWritingDirStart, &nTmpEndIndex);
                    if (nTmpEndIndex > nEndIndex)
                        nTmpEndIndex = nEndIndex;

                    DBG_ASSERT(nTmpEndIndex > nTmpStartIndex, "DrawSelectionXOR, Start >= End?");

                    tools::Long nX1 = getEditEngine().GetXPos(rInfo.rPortion, *rInfo.pLine, nTmpStartIndex, true);
                    tools::Long nX2 = getEditEngine().GetXPos(rInfo.rPortion, *rInfo.pLine, nTmpEndIndex);

                    aTmpRect.SetLeft(std::min(nX1, nX2));
                    aTmpRect.SetRight(std::max(nX1, nX2));
                    aTmpRect.Move(aLineOffset.Width(), 0);

                    ImplDrawHighlightRect(rTarget, aTmpRect.TopLeft(), aTmpRect.BottomRight(),
                                          pPolyPoly ? &*pPolyPoly : nullptr, bLOKCalcRTL);
                    nTmpStartIndex = nTmpEndIndex;
                }
            }
        }
        return ImpEditEngine::CallbackResult::Continue;
    };
    getImpEditEngine().IterateLineAreas(DrawHighlight, ImpEditEngine::IterFlag::none);

    if (comphelper::LibreOfficeKit::isActive() && mpViewShell && mpOutputWindow)
        lokSelectionCallback(pPolyPoly, bStartHandleVisible, bEndHandleVisible);

    if (pRegion || comphelper::LibreOfficeKit::isActive())
    {
        if (pRegion)
            *pRegion = vcl::Region( *pPolyPoly );
        pPolyPoly.reset();
    }
    else
    {
        if (mpOutputWindow && mpOutputWindow->GetCursor())
            mpOutputWindow->GetCursor()->Show();

        if (bClipRegion)
            rTarget.SetClipRegion(aOldRegion);
        else
            rTarget.SetClipRegion();
    }
}

void ImpEditView::GetSelectionRectangles(EditSelection aTmpSel, std::vector<tools::Rectangle>& rLogicRects)
{
    vcl::Region aRegion;
    DrawSelectionXOR(aTmpSel, &aRegion);
    aRegion.GetRegionRectangles(rLogicRects);
}

void ImpEditView::ImplDrawHighlightRect( OutputDevice& rTarget, const Point& rDocPosTopLeft, const Point& rDocPosBottomRight, tools::PolyPolygon* pPolyPoly, bool bLOKCalcRTL )
{
    if ( rDocPosTopLeft.X() == rDocPosBottomRight.X() )
        return;

    if (mpLOKSpecialPositioning && pPolyPoly)
    {
        MapUnit eDevUnit = rTarget.GetMapMode().GetMapUnit();
        tools::Rectangle aSelRect(rDocPosTopLeft, rDocPosBottomRight);
        aSelRect = GetWindowPos(aSelRect);
        Point aRefPointLogical = GetOutputArea().TopLeft();
        // Get the relative coordinates w.r.t refpoint in display units.
        aSelRect.Move(-aRefPointLogical.X(), -aRefPointLogical.Y());
        if (bLOKCalcRTL)
        {
            tools::Long nMirrorW = GetOutputArea().GetWidth();
            tools::Long nLeft = aSelRect.Left(), nRight = aSelRect.Right();
            aSelRect.SetLeft(nMirrorW - nRight);
            aSelRect.SetRight(nMirrorW - nLeft);
        }
        // Convert from display unit to twips.
        aSelRect = OutputDevice::LogicToLogic(aSelRect, MapMode(eDevUnit), MapMode(MapUnit::MapTwip));

        tools::Polygon aTmpPoly(4);
        aTmpPoly[0] = aSelRect.TopLeft();
        aTmpPoly[1] = aSelRect.TopRight();
        aTmpPoly[2] = aSelRect.BottomRight();
        aTmpPoly[3] = aSelRect.BottomLeft();
        pPolyPoly->Insert(aTmpPoly);
        return;
    }

    bool bPixelMode = rTarget.GetMapMode().GetMapUnit() == MapUnit::MapPixel;

    Point aPnt1( GetWindowPos( rDocPosTopLeft ) );
    Point aPnt2( GetWindowPos( rDocPosBottomRight ) );

    if ( !IsVertical() )
    {
        lcl_AlignToPixel(aPnt1, rTarget, +1, 0);
        lcl_AlignToPixel(aPnt2, rTarget, 0, (bPixelMode ? 0 : -1));
    }
    else
    {
        lcl_AlignToPixel(aPnt1, rTarget, 0, +1 );
        lcl_AlignToPixel(aPnt2, rTarget, (bPixelMode ? 0 : +1), 0);
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
        vcl::Window* pWindow = rTarget.GetOwnerWindow();

        if (pWindow)
        {
            pWindow->GetOutDev()->Invert( aRect );
        }
        else
        {
            rTarget.Push(vcl::PushFlags::LINECOLOR|vcl::PushFlags::FILLCOLOR|vcl::PushFlags::RASTEROP);
            rTarget.SetLineColor();
            rTarget.SetFillColor(COL_BLACK);
            rTarget.SetRasterOp(RasterOp::Invert);
            rTarget.DrawRect(aRect);
            rTarget.Pop();
        }
    }
}


bool ImpEditView::IsVertical() const
{
    return getImpEditEngine().IsEffectivelyVertical();
}

bool ImpEditView::IsTopToBottom() const
{
    return getImpEditEngine().IsTopToBottom();
}

tools::Rectangle ImpEditView::GetVisDocArea() const
{
    return tools::Rectangle( GetVisDocLeft(), GetVisDocTop(), GetVisDocRight(), GetVisDocBottom() );
}

Point ImpEditView::GetDocPos( const Point& rWindowPos ) const
{
    // Window Position => Position Document
    Point aPoint;

    if (!getImpEditEngine().IsEffectivelyVertical())
    {
        aPoint.setX( rWindowPos.X() - maOutputArea.Left() + GetVisDocLeft() );
        aPoint.setY( rWindowPos.Y() - maOutputArea.Top() + GetVisDocTop() );
    }
    else
    {
        if (getImpEditEngine().IsTopToBottom())
        {
            aPoint.setX( rWindowPos.Y() - maOutputArea.Top() + GetVisDocLeft() );
            aPoint.setY( maOutputArea.Right() - rWindowPos.X() + GetVisDocTop() );
        }
        else
        {
            aPoint.setX( maOutputArea.Bottom() - rWindowPos.Y() + GetVisDocLeft() );
            aPoint.setY( rWindowPos.X() - maOutputArea.Left() + GetVisDocTop() );
        }
    }

    return aPoint;
}

Point ImpEditView::GetWindowPos( const Point& rDocPos ) const
{
    // Document position => window position
    Point aPoint;

    if (!getImpEditEngine().IsEffectivelyVertical())
    {
        aPoint.setX( rDocPos.X() + maOutputArea.Left() - GetVisDocLeft() );
        aPoint.setY( rDocPos.Y() + maOutputArea.Top() - GetVisDocTop() );
    }
    else
    {
        if (getImpEditEngine().IsTopToBottom())
        {
            aPoint.setX( maOutputArea.Right() - rDocPos.Y() + GetVisDocTop() );
            aPoint.setY( rDocPos.X() + maOutputArea.Top() - GetVisDocLeft() );
        }
        else
        {
            aPoint.setX( maOutputArea.Left() + rDocPos.Y() - GetVisDocTop() );
            aPoint.setY( maOutputArea.Bottom() - rDocPos.X() + GetVisDocLeft() );
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
    if (!getImpEditEngine().IsEffectivelyVertical())
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
    if (meSelectionMode != eNewMode)
    {
        DrawSelectionXOR();
        meSelectionMode = eNewMode;
        DrawSelectionXOR();    // redraw
    }
}

OutputDevice& ImpEditView::GetOutputDevice() const
{
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
        return pCallbacks->EditViewOutputDevice();
    return *mpOutputWindow->GetOutDev();
}

weld::Widget* ImpEditView::GetPopupParent(tools::Rectangle& rRect) const
{
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
    {
        weld::Widget* pParent = pCallbacks->EditViewPopupParent();
        if (pParent)
            return pParent;
    }
    return weld::GetPopupParent(*mpOutputWindow, rRect);
}

void ImpEditView::SetOutputArea( const tools::Rectangle& rRect )
{
    const OutputDevice& rOutDev = GetOutputDevice();
    // should be better be aligned on pixels!
    tools::Rectangle aNewRect(rOutDev.LogicToPixel(rRect));
    aNewRect = rOutDev.PixelToLogic(aNewRect);
    maOutputArea = aNewRect;
    if (!maOutputArea.IsWidthEmpty() && maOutputArea.Right() < maOutputArea.Left())
        maOutputArea.SetRight(maOutputArea.Left());
    if (!maOutputArea.IsHeightEmpty() && maOutputArea.Bottom() < maOutputArea.Top())
        maOutputArea.SetBottom(maOutputArea.Top());

    SetScrollDiffX( static_cast<sal_uInt16>(maOutputArea.GetWidth()) * 2 / 10 );
}

namespace {

tools::Rectangle lcl_negateRectX(const tools::Rectangle& rRect)
{
    return tools::Rectangle(-rRect.Right(), rRect.Top(), -rRect.Left(), rRect.Bottom());
}

}

void ImpEditView::InvalidateAtWindow(const tools::Rectangle& rRect)
{
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
    {
        // do not invalidate and trigger a global repaint, but forward
        // the need for change to the applied EditViewCallback, can e.g.
        // be used to visualize the active edit text in an OverlayObject
        pCallbacks->EditViewInvalidate(mbNegativeX ? lcl_negateRectX(rRect) : rRect);
    }
    else
    {
        // classic mode: invalidate and trigger full repaint
        // of the changed area
        GetWindow()->Invalidate(mbNegativeX ? lcl_negateRectX(rRect) : rRect);
    }
}

void ImpEditView::ResetOutputArea( const tools::Rectangle& rRect )
{
    // remember old out area
    const tools::Rectangle aOldArea(maOutputArea);

    // apply new one
    SetOutputArea(rRect);

    // invalidate surrounding areas if update is true
    if(aOldArea.IsEmpty() || !getImpEditEngine().IsUpdateLayout())
        return;

    // #i119885# use grown area if needed; do when getting bigger OR smaller
    const sal_Int32 nMore(DoInvalidateMore() ? GetOutputDevice().PixelToLogic(Size(mnInvalidateMore, 0)).Width() : 0);

    if (aOldArea.Left() > maOutputArea.Left())
    {
        const tools::Rectangle aRect(maOutputArea.Left() - nMore, aOldArea.Top() - nMore, aOldArea.Left(), aOldArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }
    else if (aOldArea.Left() < maOutputArea.Left())
    {
        const tools::Rectangle aRect(aOldArea.Left() - nMore, aOldArea.Top() - nMore, maOutputArea.Left(), aOldArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }

    if (aOldArea.Right() > maOutputArea.Right())
    {
        const tools::Rectangle aRect(maOutputArea.Right(), aOldArea.Top() - nMore, aOldArea.Right() + nMore, aOldArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }
    else if (aOldArea.Right() < maOutputArea.Right())
    {
        const tools::Rectangle aRect(aOldArea.Right(), aOldArea.Top() - nMore, maOutputArea.Right() + nMore, aOldArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }

    if (aOldArea.Top() > maOutputArea.Top())
    {
        const tools::Rectangle aRect(aOldArea.Left() - nMore, maOutputArea.Top() - nMore, aOldArea.Right() + nMore, aOldArea.Top());
        InvalidateAtWindow(aRect);
    }
    else if (aOldArea.Top() < maOutputArea.Top())
    {
        const tools::Rectangle aRect(aOldArea.Left() - nMore, aOldArea.Top() - nMore, aOldArea.Right() + nMore, maOutputArea.Top());
        InvalidateAtWindow(aRect);
    }

    if (aOldArea.Bottom() > maOutputArea.Bottom())
    {
        const tools::Rectangle aRect(aOldArea.Left() - nMore, maOutputArea.Bottom(), aOldArea.Right() + nMore, aOldArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }
    else if (aOldArea.Bottom() < maOutputArea.Bottom())
    {
        const tools::Rectangle aRect(aOldArea.Left() - nMore, aOldArea.Bottom(), aOldArea.Right() + nMore, maOutputArea.Bottom() + nMore);
        InvalidateAtWindow(aRect);
    }
}

void ImpEditView::RecalcOutputArea()
{
    Point aNewTopLeft(maOutputArea.TopLeft());
    Size aNewSz(maOutputArea.GetSize());

    // X:
    if ( DoAutoWidth() )
    {
        if (getImpEditEngine().GetStatus().AutoPageWidth())
            aNewSz.setWidth(getImpEditEngine().GetPaperSize().Width());
        switch (meAnchorMode)
        {
            case EEAnchorMode::TopLeft:
            case EEAnchorMode::VCenterLeft:
            case EEAnchorMode::BottomLeft:
            {
                aNewTopLeft.setX(maAnchorPoint.X());
            }
            break;
            case EEAnchorMode::TopHCenter:
            case EEAnchorMode::VCenterHCenter:
            case EEAnchorMode::BottomHCenter:
            {
                aNewTopLeft.setX(maAnchorPoint.X() - aNewSz.Width() / 2);
            }
            break;
            case EEAnchorMode::TopRight:
            case EEAnchorMode::VCenterRight:
            case EEAnchorMode::BottomRight:
            {
                aNewTopLeft.setX(maAnchorPoint.X() - aNewSz.Width() - 1);
            }
            break;
        }
    }

    // Y:
    if ( DoAutoHeight() )
    {
        if (getImpEditEngine().GetStatus().AutoPageHeight())
            aNewSz.setHeight(getImpEditEngine().GetPaperSize().Height());
        switch (meAnchorMode)
        {
            case EEAnchorMode::TopLeft:
            case EEAnchorMode::TopHCenter:
            case EEAnchorMode::TopRight:
            {
                aNewTopLeft.setY(maAnchorPoint.Y());
            }
            break;
            case EEAnchorMode::VCenterLeft:
            case EEAnchorMode::VCenterHCenter:
            case EEAnchorMode::VCenterRight:
            {
                aNewTopLeft.setY(maAnchorPoint.Y() - aNewSz.Height() / 2);
            }
            break;
            case EEAnchorMode::BottomLeft:
            case EEAnchorMode::BottomHCenter:
            case EEAnchorMode::BottomRight:
            {
                aNewTopLeft.setY(maAnchorPoint.Y() - aNewSz.Height() - 1);
            }
            break;
        }
    }
    ResetOutputArea( tools::Rectangle( aNewTopLeft, aNewSz ) );
}

void ImpEditView::SetAnchorMode(EEAnchorMode eMode)
{
    meAnchorMode = eMode;
    CalcAnchorPoint();
}

void ImpEditView::CalcAnchorPoint()
{
    // GetHeight() and GetWidth() -1, because rectangle calculation not preferred.

    // X:
    switch (meAnchorMode)
    {
        case EEAnchorMode::TopLeft:
        case EEAnchorMode::VCenterLeft:
        case EEAnchorMode::BottomLeft:
        {
            maAnchorPoint.setX(maOutputArea.Left());
        }
        break;
        case EEAnchorMode::TopHCenter:
        case EEAnchorMode::VCenterHCenter:
        case EEAnchorMode::BottomHCenter:
        {
            maAnchorPoint.setX(maOutputArea.Left() + (maOutputArea.GetWidth() - 1) / 2);
        }
        break;
        case EEAnchorMode::TopRight:
        case EEAnchorMode::VCenterRight:
        case EEAnchorMode::BottomRight:
        {
            maAnchorPoint.setX(maOutputArea.Right());
        }
        break;
    }

    // Y:
    switch (meAnchorMode)
    {
        case EEAnchorMode::TopLeft:
        case EEAnchorMode::TopHCenter:
        case EEAnchorMode::TopRight:
        {
            maAnchorPoint.setY(maOutputArea.Top());
        }
        break;
        case EEAnchorMode::VCenterLeft:
        case EEAnchorMode::VCenterHCenter:
        case EEAnchorMode::VCenterRight:
        {
            maAnchorPoint.setY(maOutputArea.Top() + (maOutputArea.GetHeight() - 1) / 2);
        }
        break;
        case EEAnchorMode::BottomLeft:
        case EEAnchorMode::BottomHCenter:
        case EEAnchorMode::BottomRight:
        {
            maAnchorPoint.setY(maOutputArea.Bottom() - 1);
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

tools::Rectangle ImpEditView::ImplGetEditCursor(EditPaM& aPaM, CursorFlags aShowCursorFlags, sal_Int32& nTextPortionStart, ParaPortion const& rParaPortion) const
{
    tools::Rectangle aEditCursor = getImpEditEngine().PaMtoEditCursor(aPaM, aShowCursorFlags);
    if (!IsInsertMode() && !maEditSelection.HasRange())
    {
        if ( aPaM.GetNode()->Len() && ( aPaM.GetIndex() < aPaM.GetNode()->Len() ) )
        {
            // If we are behind a portion, and the next portion has other direction, we must change position...
            aEditCursor.SetLeft(getImpEditEngine().PaMtoEditCursor(aPaM, CursorFlags{.bTextOnly = true, .bPreferPortionStart = true}).Left());
            aEditCursor.SetRight( aEditCursor.Left() );

            sal_Int32 nTextPortion = rParaPortion.GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, true );
            const TextPortion& rTextPortion = rParaPortion.GetTextPortions()[nTextPortion];
            if ( rTextPortion.GetKind() == PortionKind::TAB )
            {
                aEditCursor.AdjustRight(rTextPortion.GetSize().Width() );
            }
            else
            {
                EditPaM aNext = getEditEngine().CursorRight( aPaM );
                tools::Rectangle aTmpRect = getImpEditEngine().PaMtoEditCursor(aNext, CursorFlags{ .bTextOnly = true });
                if ( aTmpRect.Top() != aEditCursor.Top() )
                    aTmpRect = getImpEditEngine().PaMtoEditCursor(aNext, CursorFlags{ .bTextOnly = true, .bEndOfLine = true });
                aEditCursor.SetRight( aTmpRect.Left() );
            }
        }
    }

    tools::Long nMaxHeight = !IsVertical() ? maOutputArea.GetHeight() : maOutputArea.GetWidth();
    if ( aEditCursor.GetHeight() > nMaxHeight )
    {
        aEditCursor.SetBottom( aEditCursor.Top() + nMaxHeight - 1 );
    }

    return aEditCursor;
}

tools::Rectangle ImpEditView::GetEditCursor() const
{
    EditPaM aPaM(maEditSelection.Max());

    sal_Int32 nTextPortionStart = 0;
    sal_Int32 nPara = getEditEngine().GetEditDoc().GetPos( aPaM.GetNode() );
    if (nPara == EE_PARA_NOT_FOUND) // #i94322
        return tools::Rectangle();

    ParaPortion const& rParaPortion = getEditEngine().GetParaPortions().getRef(nPara);

    CursorFlags aShowCursorFlags = maExtraCursorFlags;
    aShowCursorFlags.bTextOnly = true;

    // Use CursorBidiLevel 0/1 in meaning of
    // 0: prefer portion end, normal mode
    // 1: prefer portion start

    if ( ( GetCursorBidiLevel() != CURSOR_BIDILEVEL_DONTKNOW ) && GetCursorBidiLevel() )
    {
        aShowCursorFlags.bPreferPortionStart = true;
    }

    return ImplGetEditCursor(aPaM, aShowCursorFlags, nTextPortionStart, rParaPortion);
}

void ImpEditView::ShowCursor( bool bGotoCursor, bool bForceVisCursor )
{
    // No ShowCursor in an empty View ...
    if (maOutputArea.IsEmpty())
        return;
    if ( (maOutputArea.Left() >= maOutputArea.Right() ) && ( maOutputArea.Top() >= maOutputArea.Bottom() ) )
        return;

    getEditEngine().CheckIdleFormatter();
    if (!getEditEngine().IsFormatted())
        getImpEditEngine().FormatDoc();

    // For some reasons I end up here during the formatting, if the Outliner
    // is initialized in Paint, because no SetPool();
    if (getImpEditEngine().IsFormatting())
        return;
    if (!getImpEditEngine().IsUpdateLayout())
        return;
    if (getImpEditEngine().IsInUndo())
        return;

    if (mpOutputWindow && mpOutputWindow->GetCursor() != GetCursor())
        mpOutputWindow->SetCursor(GetCursor());

    EditPaM aPaM(maEditSelection.Max());

    sal_Int32 nTextPortionStart = 0;
    sal_Int32 nPara = getEditEngine().GetEditDoc().GetPos( aPaM.GetNode() );
    if (nPara == EE_PARA_NOT_FOUND) // #i94322
        return;

    ParaPortion const& rParaPortion = getEditEngine().GetParaPortions().getRef(nPara);

    CursorFlags aShowCursorFlags = maExtraCursorFlags;
    aShowCursorFlags.bTextOnly = true;

    // Use CursorBidiLevel 0/1 in meaning of
    // 0: prefer portion end, normal mode
    // 1: prefer portion start

    if ( ( GetCursorBidiLevel() != CURSOR_BIDILEVEL_DONTKNOW ) && GetCursorBidiLevel() )
    {
        aShowCursorFlags.bPreferPortionStart = true;
    }

    tools::Rectangle aEditCursor = ImplGetEditCursor(aPaM, aShowCursorFlags, nTextPortionStart, rParaPortion);

    if ( bGotoCursor  ) // && (!getImpEditEngine().GetStatus().AutoPageSize() ) )
    {
        // check if scrolling is necessary...
        // if scrolling, then update () and Scroll ()!
        tools::Long nDocDiffX = 0;
        tools::Long nDocDiffY = 0;

        tools::Rectangle aTmpVisArea( GetVisDocArea() );
        // aTmpOutArea: if OutputArea > Paper width and
        // Text > Paper width ( over large fields )
        tools::Long nMaxTextWidth = !IsVertical() ? getImpEditEngine().GetPaperSize().Width() : getImpEditEngine().GetPaperSize().Height();
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
                getEditEngine().GetInternalEditStatus().GetStatusWord() = getEditEngine().GetInternalEditStatus().GetStatusWord() | EditStatusFlags::HSCROLL;
            if ( nDiffY )
                getEditEngine().GetInternalEditStatus().GetStatusWord() = getEditEngine().GetInternalEditStatus().GetStatusWord() | EditStatusFlags::VSCROLL;
            Scroll( -nDiffX, -nDiffY );
            getImpEditEngine().DelayedCallStatusHdl();
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
                // Get rectangle in window-coordinates from editeng(doc) coordinates in hmm.
                aCursorRectPureLogical = GetWindowPos(aCursorRectPureLogical);
                Point aRefPointLogical = GetOutputArea().TopLeft();
                // Get the relative coordinates w.r.t refpoint in display hmm.
                aCursorRectPureLogical.Move(-aRefPointLogical.X(), -aRefPointLogical.Y());
                if (getEditEngine().IsRightToLeft(nPara) || mpLOKSpecialPositioning->IsLayoutRTL())
                {
                    tools::Long nMirrorW = GetOutputArea().GetWidth();
                    tools::Long nLeft = aCursorRectPureLogical.Left(), nRight = aCursorRectPureLogical.Right();
                    aCursorRectPureLogical.SetLeft(nMirrorW - nRight);
                    aCursorRectPureLogical.SetRight(nMirrorW - nLeft);
                }
                // Convert to twips.
                aCursorRectPureLogical = OutputDevice::LogicToLogic(aCursorRectPureLogical, MapMode(eDevUnit), MapMode(MapUnit::MapTwip));
                // "refpoint" in print twips.
                const Point aRefPoint = mpLOKSpecialPositioning->GetRefPoint();
                aMessageParams.put("relrect", aCursorRectPureLogical.toString());
                aMessageParams.put("refpoint", aRefPoint.toString());
            }

            if (mpOutputWindow && mpOutputWindow->IsChart())
            {
                const vcl::Window* pViewShellWindow = mpViewShell->GetEditWindowForActiveOLEObj();
                if (pViewShellWindow && pViewShellWindow->IsAncestorOf(*mpOutputWindow))
                {
                    Point aOffsetPx = mpOutputWindow->GetOffsetPixelFrom(*pViewShellWindow);
                    Point aLogicOffset = mpOutputWindow->PixelToLogic(aOffsetPx);
                    aPos.Move(aLogicOffset.getX(), aLogicOffset.getY());
                }
            }

            tools::Rectangle aRect(aPos.getX(), aPos.getY(), aPos.getX() + GetCursor()->GetWidth(), aPos.getY() + GetCursor()->GetHeight());

            // LOK output is always in twips, convert from mm100 if necessary.
            if (rOutDev.GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
            {
                aRect = o3tl::convert(aRect, o3tl::Length::mm100, o3tl::Length::twip);
            }
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
                uno::Reference<linguistic2::XSpellChecker1>  xSpeller(getImpEditEngine().GetSpeller());
                bool bIsWrong = xSpeller.is() && IsWrongSpelledWord(aPaM, /*bMarkIfWrong*/ false);
                EditView* pActiveView = GetEditViewPtr();

                boost::property_tree::ptree aHyperlinkTree;
                if (pActiveView && URLFieldHelper::IsCursorAtURLField(*pActiveView))
                {
                    if (const SvxFieldItem* pFld = GetField(aPos, nullptr, nullptr))
                        if (auto pUrlField = dynamic_cast<const SvxURLField*>(pFld->GetField()))
                            aHyperlinkTree = getHyperlinkPropTree(pUrlField->GetRepresentation(), pUrlField->GetURL());
                }
                else if (GetEditSelection().HasRange())
                {
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
                            OString(aMessageParams.get<std::string>("rectangle")));
            }
        }

        CursorDirection nCursorDir = CursorDirection::NONE;
        if ( IsInsertMode() && !maEditSelection.HasRange() && (getImpEditEngine().HasDifferentRTLLevels(aPaM.GetNode()) ) )
        {
            sal_uInt16 nTextPortion = rParaPortion.GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, aShowCursorFlags.bPreferPortionStart);
            const TextPortion& rTextPortion = rParaPortion.GetTextPortions()[nTextPortion];
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
            getEditEngine().SeekCursor( aPaM.GetNode(), aPaM.GetIndex()+1, aFont );

            InputContext aInputContext(std::move(aFont), InputContextFlags::Text | InputContextFlags::ExtText);
            if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
                pCallbacks->EditViewInputContext(aInputContext);
            else if (auto xWindow = GetWindow())
                xWindow->SetInputContext(aInputContext);
        }
    }
    else
    {
        getImpEditEngine().GetStatus().GetStatusWord() = getImpEditEngine().GetStatus().GetStatusWord() | EditStatusFlags::CURSOROUT;
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
    DBG_ASSERT(getImpEditEngine().IsFormatted(), "Scroll: Not formatted!");
    if ( !ndX && !ndY )
        return Pair( 0, 0 );

    const OutputDevice& rOutDev = GetOutputDevice();

#ifdef DBG_UTIL
    tools::Rectangle aR(maOutputArea);
    aR = rOutDev.LogicToPixel( aR );
    aR = rOutDev.PixelToLogic( aR );
    SAL_WARN_IF(aR != maOutputArea, "editeng", "OutArea before Scroll not aligned");
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
    if ( ( nRangeCheck == ScrollRangeCheck::PaperWidthTextSize ) && ( aNewVisArea.Bottom() > static_cast<tools::Long>(getImpEditEngine().GetTextHeight()) ) )
    {
        // GetTextHeight still optimizing!
        tools::Long nDiff = getImpEditEngine().GetTextHeight() - aNewVisArea.Bottom(); // negative
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
    if ( ( nRangeCheck == ScrollRangeCheck::PaperWidthTextSize ) && ( aNewVisArea.Right() > static_cast<tools::Long>(getImpEditEngine().CalcTextWidth( false )) ) )
    {
        tools::Long nDiff = getImpEditEngine().CalcTextWidth( false ) - aNewVisArea.Right();     // negative
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
        if (mpOutputWindow)
            mpOutputWindow->PaintImmediately();
        if (!IsVertical())
            maVisDocStartPos.Move(-nRealDiffX, -nRealDiffY);
        else
        {
            if (IsTopToBottom())
                maVisDocStartPos.Move(-nRealDiffY, nRealDiffX);
            else
                maVisDocStartPos.Move(nRealDiffY, -nRealDiffX);
        }
        // Move by aligned value does not necessarily result in aligned
        // rectangle ...
        maVisDocStartPos = rOutDev.LogicToPixel(maVisDocStartPos);
        maVisDocStartPos = rOutDev.PixelToLogic(maVisDocStartPos);
        tools::Rectangle aRect(maOutputArea);

        if (mpOutputWindow)
        {
            mpOutputWindow->Scroll( nRealDiffX, nRealDiffY, aRect, ScrollFlags::Clip );
        }

        if (comphelper::LibreOfficeKit::isActive() || getEditViewCallbacks())
        {
            // Need to invalidate the window, otherwise no tile will be re-painted.
            GetEditViewPtr()->Invalidate();
        }

        if (mpOutputWindow)
            mpOutputWindow->PaintImmediately();
        pCrsr->SetPos( pCrsr->GetPos() + Point( nRealDiffX, nRealDiffY ) );
        if ( bVisCursor )
        {
            tools::Rectangle aCursorRect( pCrsr->GetPos(), pCrsr->GetSize() );
            if (maOutputArea.Contains(aCursorRect))
                pCrsr->Show();
        }

        if (getImpEditEngine().GetNotifyHdl().IsSet())
        {
            EENotify aNotify( EE_NOTIFY_TEXTVIEWSCROLLED );
            getImpEditEngine().GetNotifyHdl().Call( aNotify );
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

uno::Reference<datatransfer::clipboard::XClipboard> ImpEditView::GetClipboard() const
{
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
        return pCallbacks->GetClipboard();
    if (vcl::Window* pWindow = GetWindow())
        return pWindow->GetClipboard();
    SAL_WARN("editeng", "falling back to using GetSystemClipboard");
    return GetSystemClipboard();
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
                if (!mbReadOnly)
                {
                    uno::Reference<datatransfer::clipboard::XClipboard> aClipBoard(GetClipboard());
                    CutCopy( aClipBoard, true );
                    bDone = true;
                }
            }
            break;
            case KeyFuncType::COPY:
            {
                uno::Reference<datatransfer::clipboard::XClipboard> aClipBoard(GetClipboard());
                CutCopy( aClipBoard, false );
                bDone = true;
            }
            break;
            case KeyFuncType::PASTE:
            {
                if (!mbReadOnly && IsPasteEnabled())
                {
                    getImpEditEngine().UndoActionStart( EDITUNDO_PASTE );
                    uno::Reference<datatransfer::clipboard::XClipboard> aClipBoard(GetClipboard());
                    Paste( aClipBoard, getImpEditEngine().GetStatus().AllowPasteSpecial() );
                    getImpEditEngine().UndoActionEnd();
                    bDone = true;
                }
            }
            break;
            default:
                break;
        }
    }

    if( !bDone )
        bDone = getEditEngine().PostKeyEvent( rKeyEvent, GetEditViewPtr(), pFrameWin );

    return bDone;
}

bool ImpEditView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    mnTravelXPos = TRAVEL_X_DONTKNOW;
    mnCursorBidiLevel = CURSOR_BIDILEVEL_DONTKNOW;
    maExtraCursorFlags = CursorFlags();
    mbClickedInSelection = false;

    if ( rMouseEvent.IsMiddle() && !mbReadOnly &&
         Application::GetSettings().GetMouseSettings().GetMiddleButtonAction() == MouseMiddleButtonAction::PasteSelection )
    {
        uno::Reference<datatransfer::clipboard::XClipboard> aClipBoard(GetSystemPrimarySelection());
        Paste( aClipBoard );
    }
    else if ( rMouseEvent.IsLeft() && GetEditSelection().HasRange() )
    {
        uno::Reference<datatransfer::clipboard::XClipboard> aClipBoard(GetSystemPrimarySelection());
        CutCopy( aClipBoard, false );
    }

    return getImpEditEngine().MouseButtonUp( rMouseEvent, GetEditViewPtr() );
}

void ImpEditView::ReleaseMouse()
{
    getImpEditEngine().ReleaseMouse();
}

bool ImpEditView::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    getEditEngine().CheckIdleFormatter();  // If fast typing and mouse button downs
    mnTravelXPos = TRAVEL_X_DONTKNOW;
    maExtraCursorFlags = CursorFlags();
    mnCursorBidiLevel = CURSOR_BIDILEVEL_DONTKNOW;
    bool bPrevUpdateLayout = getImpEditEngine().SetUpdateLayout(true);
    mbClickedInSelection = IsSelectionAtPoint( rMouseEvent.GetPosPixel() );
    bool bRet = getImpEditEngine().MouseButtonDown( rMouseEvent, GetEditViewPtr() );
    getImpEditEngine().SetUpdateLayout(bPrevUpdateLayout);
    return bRet;
}

bool ImpEditView::MouseMove( const MouseEvent& rMouseEvent )
{
    return getImpEditEngine().MouseMove( rMouseEvent, GetEditViewPtr() );
}

bool ImpEditView::Command(const CommandEvent& rCEvt)
{
    getEditEngine().CheckIdleFormatter();  // If fast typing and mouse button down
    return getImpEditEngine().Command(rCEvt, GetEditViewPtr());
}


void ImpEditView::SetInsertMode( bool bInsert )
{
    if ( bInsert != IsInsertMode() )
    {
        SetFlags(mnControl, EVControlBits::OVERWRITE, !bInsert);
        ShowCursor( DoAutoScroll(), false );
    }
}

bool ImpEditView::IsWrongSpelledWord( const EditPaM& rPaM, bool bMarkIfWrong )
{
    bool bIsWrong = false;
    if ( rPaM.GetNode()->GetWrongList() )
    {
        EditSelection aSel = getEditEngine().SelectWord( rPaM, css::i18n::WordType::DICTIONARY_WORD );
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
    if (getImpEditEngine().GetSpeller().is())
    {
        EditPaM aPaM = GetEditSelection().Max();
        if ( !HasSelection() )
        {
            EditSelection aSel = getEditEngine().SelectWord(aPaM);
            aWord = getImpEditEngine().GetSelected( aSel );
        }
        else
        {
            aWord = getImpEditEngine().GetSelected( GetEditSelection() );
            // And deselect
            DrawSelectionXOR();
            SetEditSelection( EditSelection( aPaM, aPaM ) );
            DrawSelectionXOR();
        }

        if ( !aWord.isEmpty() )
        {
            uno::Reference<linguistic2::XDictionary> xDic = LinguMgr::GetIgnoreAllList();
            if (xDic.is())
                xDic->add( aWord, false, OUString() );
            EditDoc& rDoc = getEditEngine().GetEditDoc();
            sal_Int32 nNodes = rDoc.Count();
            for ( sal_Int32 n = 0; n < nNodes; n++ )
            {
                ContentNode* pNode = rDoc.GetObject( n );
                pNode->GetWrongList()->MarkWrongsInvalid();
            }
            getImpEditEngine().DoOnlineSpelling( aPaM.GetNode() );
            getImpEditEngine().StartOnlineSpellTimer();
        }
    }
    return aWord;
}

void ImpEditView::DeleteSelected()
{
    DrawSelectionXOR();

    getImpEditEngine().UndoActionStart( EDITUNDO_DELETE );

    EditPaM aPaM = getImpEditEngine().DeleteSelected( GetEditSelection() );

    getImpEditEngine().UndoActionEnd();

    SetEditSelection( EditSelection( aPaM, aPaM ) );

    DrawSelectionXOR();

    getImpEditEngine().FormatAndLayout( GetEditViewPtr() );
    ShowCursor( DoAutoScroll(), true );
}

const SvxFieldItem* ImpEditView::GetField( const Point& rPos, sal_Int32* pPara, sal_Int32* pPos ) const
{
    if( !GetOutputArea().Contains( rPos ) )
        return nullptr;

    Point aDocPos( GetDocPos( rPos ) );
    EditPaM aPaM = getEditEngine().GetPaM(aDocPos, false);
    if (!aPaM)
        return nullptr;

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
                    *pPara = getEditEngine().GetEditDoc().GetPos( aPaM.GetNode() );
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

    if( !GetOutputArea().Contains( rPos ) )
        return false;

    Point aDocPos( GetDocPos( rPos ) );
    EditPaM aPaM = getEditEngine().GetPaM(aDocPos, false);
    if (!aPaM)
        return false;

    if ( aPaM.GetIndex() == 0 )
    {
        sal_Int32 nPara = getEditEngine().GetEditDoc().GetPos( aPaM.GetNode() );
        tools::Rectangle aBulletArea = getEditEngine().GetBulletArea( nPara );
        tools::Long nY = getEditEngine().GetDocPosTopLeft( nPara ).Y();
        ParaPortion const& rParaPortion = getEditEngine().GetParaPortions().getRef(nPara);
        nY += rParaPortion.GetFirstLineOffset();
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

void ImpEditView::CutCopy(uno::Reference<datatransfer::clipboard::XClipboard> const & rxClipboard, bool bCut )
{
    if ( !(rxClipboard.is() && HasSelection()) )
        return;

    uno::Reference<datatransfer::XTransferable> xData = getEditEngine().CreateTransferable( GetEditSelection() );

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
        getImpEditEngine().UndoActionStart(EDITUNDO_CUT);
        DeleteSelected();
        getImpEditEngine().UndoActionEnd();
    }
}

void ImpEditView::Paste(uno::Reference<datatransfer::clipboard::XClipboard> const & rxClipboard, bool bUseSpecial, SotClipboardFormatId format)
{
    if ( !rxClipboard.is() )
        return;

    uno::Reference<datatransfer::XTransferable> xDataObj;

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

    getImpEditEngine().UndoActionStart( EDITUNDO_PASTE );

    EditSelection aSel( GetEditSelection() );
    if ( aSel.HasRange() )
    {
        DrawSelectionXOR();
        aSel = getEditEngine().DeleteSelection(aSel);
    }

    PasteOrDropInfos aPasteOrDropInfos;
    aPasteOrDropInfos.nStartPara = getEditEngine().GetEditDoc().GetPos( aSel.Min().GetNode() );
    getEditEngine().HandleBeginPasteOrDrop(aPasteOrDropInfos);

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
                aSel = getEditEngine().InsertText(aSel, aText);
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
        aSel = getEditEngine().InsertText(
            xDataObj, OUString(), aSel.Min(),
            bUseSpecial && getEditEngine().GetInternalEditStatus().AllowPasteSpecial(), format);
    }

    aPasteOrDropInfos.nEndPara = getEditEngine().GetEditDoc().GetPos( aSel.Max().GetNode() );
    getEditEngine().HandleEndPasteOrDrop(aPasteOrDropInfos);

    getImpEditEngine().UndoActionEnd();
    SetEditSelection( aSel );
    getImpEditEngine().UpdateSelections();
    getImpEditEngine().FormatAndLayout( GetEditViewPtr() );
    ShowCursor( DoAutoScroll(), true );
}


bool ImpEditView::IsInSelection( const EditPaM& rPaM )
{
    EditSelection aSel = GetEditSelection();
    if ( !aSel.HasRange() )
        return false;

    aSel.Adjust(getEditEngine().GetEditDoc());

    sal_Int32 nStartNode = getEditEngine().GetEditDoc().GetPos( aSel.Min().GetNode() );
    sal_Int32 nEndNode = getEditEngine().GetEditDoc().GetPos( aSel.Max().GetNode() );
    sal_Int32 nCurNode = getEditEngine().GetEditDoc().GetPos( rPaM.GetNode() );

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

bool ImpEditView::IsSelectionFullPara() const
{
    if (!IsSelectionInSinglePara())
        return false;

    sal_Int32 nSelectionStartPos = GetEditSelection().Min().GetIndex();
    sal_Int32 nSelectionEndPos = GetEditSelection().Max().GetIndex();

    if (nSelectionStartPos > nSelectionEndPos)
        std::swap(nSelectionStartPos, nSelectionEndPos);

    if (nSelectionStartPos != 0)
        return false;

    const ContentNode* pNode = GetEditSelection().Min().GetNode();
    return pNode->Len() == nSelectionEndPos;
}

bool ImpEditView::IsSelectionInSinglePara() const
{
    return GetEditSelection().Min().GetNode() == GetEditSelection().Max().GetNode();
}

void ImpEditView::CreateAnchor()
{
    getEditEngine().SetInSelectionMode(true);
    EditSelection aNewSelection(GetEditSelection());
    aNewSelection.Min() = aNewSelection.Max();
    SetEditSelection(aNewSelection);
    // const_cast<EditPaM&>(GetEditSelection().Min()) = GetEditSelection().Max();
}

void ImpEditView::DeselectAll()
{
    getEditEngine().SetInSelectionMode(false);
    DrawSelectionXOR();
    EditSelection aNewSelection(GetEditSelection());
    aNewSelection.Min() = aNewSelection.Max();
    SetEditSelection(aNewSelection);
    // const_cast<EditPaM&>(GetEditSelection().Min()) = GetEditSelection().Max();

    if (comphelper::LibreOfficeKit::isActive() && mpViewShell && mpOutputWindow)
    {
        VclPtr<vcl::Window> pParent = mpOutputWindow->GetParentWithLOKNotifier();
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
    if (mpDragAndDropInfo && mpDragAndDropInfo->pField)
        return true;

    // Logical units ...
    const OutputDevice& rOutDev = GetOutputDevice();
    Point aMousePos = rOutDev.PixelToLogic(rPosPixel);

    if ( ( !GetOutputArea().Contains( aMousePos ) ) && !getImpEditEngine().IsInSelectionMode() )
    {
        return false;
    }

    Point aDocPos( GetDocPos( aMousePos ) );
    EditPaM aPaM = getEditEngine().GetPaM(aDocPos, false);
    return IsInSelection( aPaM );
}

bool ImpEditView::SetCursorAtPoint( const Point& rPointPixel )
{
    getEditEngine().CheckIdleFormatter();

    Point aMousePos( rPointPixel );

    // Logical units ...
    const OutputDevice& rOutDev = GetOutputDevice();
    aMousePos = rOutDev.PixelToLogic( aMousePos );

    if ( ( !GetOutputArea().Contains( aMousePos ) ) && !getImpEditEngine().IsInSelectionMode() )
    {
        return false;
    }

    Point aDocPos( GetDocPos( aMousePos ) );

    // Can be optimized: first go through the lines within a paragraph for PAM,
    // then again with the PaM for the Rect, even though the line is already
    // known... This must not be, though!
    EditPaM aPaM = getEditEngine().GetPaM(aDocPos);
    bool bGotoCursor = DoAutoScroll();

    // aTmpNewSel: Diff between old and new, not the new selection, unless for tiled rendering
    EditSelection aTmpNewSel( comphelper::LibreOfficeKit::isActive() ? GetEditSelection().Min() : GetEditSelection().Max(), aPaM );

    // #i27299#
    // work on copy of current selection and set new selection, if it has changed.
    EditSelection aNewEditSelection( GetEditSelection() );

    aNewEditSelection.Max() = aPaM;
    if (!getEditEngine().GetSelectionEngine().HasAnchor())
    {
        if ( aNewEditSelection.Min() != aPaM )
        {
            const ContentNode* pNode(aNewEditSelection.Min().GetNode());
            if (nullptr != pNode)
                pNode->checkAndDeleteEmptyAttribs();
        }
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

    bool bForceCursor = mpDragAndDropInfo == nullptr && !getImpEditEngine().IsInSelectionMode();
    ShowCursor( bGotoCursor, bForceCursor );
    return true;
}

void ImpEditView::HideDDCursor()
{
    if (mpDragAndDropInfo && mpDragAndDropInfo->bVisCursor)
    {
        OutputDevice& rOutDev = GetOutputDevice();
        rOutDev.DrawOutDev(mpDragAndDropInfo->aCurSavedCursor.TopLeft(), mpDragAndDropInfo->aCurSavedCursor.GetSize(),
                            Point(0,0), mpDragAndDropInfo->aCurSavedCursor.GetSize(),*mpDragAndDropInfo->pBackground);
        mpDragAndDropInfo->bVisCursor = false;
    }
}

void ImpEditView::ShowDDCursor( const tools::Rectangle& rRect )
{
    if (!mpDragAndDropInfo || mpDragAndDropInfo->bVisCursor)
        return;

    if (mpOutputWindow && mpOutputWindow->GetCursor())
        mpOutputWindow->GetCursor()->Hide();

    OutputDevice& rOutDev = GetOutputDevice();
    Color aOldFillColor = rOutDev.GetFillColor();
    rOutDev.SetFillColor( Color(4210752) );    // GRAY BRUSH_50, OLDSV, change to DDCursor!

    // Save background ...
    tools::Rectangle aSaveRect( rOutDev.LogicToPixel( rRect ) );
    // prefer to save some more ...
    aSaveRect.AdjustRight(1 );
    aSaveRect.AdjustBottom(1 );

    if (!mpDragAndDropInfo->pBackground)
    {
        mpDragAndDropInfo->pBackground = VclPtr<VirtualDevice>::Create(rOutDev);
        MapMode aMapMode( rOutDev.GetMapMode() );
        aMapMode.SetOrigin( Point( 0, 0 ) );
        mpDragAndDropInfo->pBackground->SetMapMode( aMapMode );

    }

    Size aNewSzPx( aSaveRect.GetSize() );
    Size aCurSzPx(mpDragAndDropInfo->pBackground->GetOutputSizePixel());
    if ( ( aCurSzPx.Width() < aNewSzPx.Width() ) ||( aCurSzPx.Height() < aNewSzPx.Height() ) )
    {
        bool bDone = mpDragAndDropInfo->pBackground->SetOutputSizePixel( aNewSzPx );
        DBG_ASSERT( bDone, "Virtual Device broken?" );
    }

    aSaveRect = rOutDev.PixelToLogic( aSaveRect );

    mpDragAndDropInfo->pBackground->DrawOutDev( Point(0,0), aSaveRect.GetSize(),
                                aSaveRect.TopLeft(), aSaveRect.GetSize(), rOutDev );
    mpDragAndDropInfo->aCurSavedCursor = aSaveRect;

    // Draw Cursor...
    rOutDev.DrawRect( rRect );

    mpDragAndDropInfo->bVisCursor = true;
    mpDragAndDropInfo->aCurCursor = rRect;

    rOutDev.SetFillColor( aOldFillColor );
}

void ImpEditView::dragGestureRecognized(const css::datatransfer::dnd::DragGestureEvent& rDGE)
{
    DBG_ASSERT(!mpDragAndDropInfo, "dragGestureRecognized - DragAndDropInfo exist!");

    SolarMutexGuard aVclGuard;

    mpDragAndDropInfo.reset();

    Point aMousePosPixel( rDGE.DragOriginX, rDGE.DragOriginY );

    EditSelection aCopySel( GetEditSelection() );
    aCopySel.Adjust(getEditEngine().GetEditDoc());

    if (HasSelection() && mbClickedInSelection)
    {
        mpDragAndDropInfo.reset(new DragAndDropInfo);
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
            mpDragAndDropInfo.reset(new DragAndDropInfo);
            mpDragAndDropInfo->pField = pField;
            ContentNode* pNode = getEditEngine().GetEditDoc().GetObject( nPara );
            aCopySel = EditSelection( EditPaM( pNode, nPos ), EditPaM( pNode, nPos+1 ) );
            SetEditSelection(aCopySel);
            DrawSelectionXOR();
            bool bGotoCursor = DoAutoScroll();
            ShowCursor( bGotoCursor, /*bForceCursor=*/false );
        }
        else if ( IsBulletArea( aMousePos, &nPara ) )
        {
            mpDragAndDropInfo.reset(new DragAndDropInfo);
            mpDragAndDropInfo->bOutlinerMode = true;
            EditPaM aStartPaM(getEditEngine().GetEditDoc().GetObject(nPara), 0);
            EditPaM aEndPaM( aStartPaM );
            const SfxInt16Item& rLevel = getEditEngine().GetParaAttrib(nPara, EE_PARA_OUTLLEVEL);
            for ( sal_Int32 n = nPara +1; n < getEditEngine().GetEditDoc().Count(); n++ )
            {
                const SfxInt16Item& rL = getEditEngine().GetParaAttrib( n, EE_PARA_OUTLLEVEL );
                if ( rL.GetValue() > rLevel.GetValue() )
                {
                    aEndPaM.SetNode( getEditEngine().GetEditDoc().GetObject( n ) );
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

    if (!mpDragAndDropInfo)
        return;


    mpDragAndDropInfo->bStarterOfDD = true;

    // Sensitive area to be scrolled.
    Size aSz( 5, 0 );
    aSz = GetOutputDevice().PixelToLogic( aSz );
    mpDragAndDropInfo->nSensibleRange = static_cast<sal_uInt16>(aSz.Width());
    mpDragAndDropInfo->nCursorWidth = static_cast<sal_uInt16>(aSz.Width()) / 2;
    mpDragAndDropInfo->aBeginDragSel = getImpEditEngine().CreateESel( aCopySel );

    uno::Reference<datatransfer::XTransferable> xData = getEditEngine().CreateTransferable(aCopySel);

    sal_Int8 nActions = mbReadOnly ? datatransfer::dnd::DNDConstants::ACTION_COPY : datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE;

    rDGE.DragSource->startDrag( rDGE, nActions, 0 /*cursor*/, 0 /*image*/, xData, mxDnDListener );
    // If Drag&Move in an Engine, then Copy&Del has to be optional!
    GetCursor()->Hide();
}

void ImpEditView::dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& rDSDE )
{
    SolarMutexGuard aVclGuard;

    DBG_ASSERT(mpDragAndDropInfo, "ImpEditView::dragDropEnd: mpDragAndDropInfo is NULL!");

    // #123688# Shouldn't happen, but seems to happen...
    if (!mpDragAndDropInfo)
        return;

    if (!mbReadOnly && rDSDE.DropSuccess && !mpDragAndDropInfo->bOutlinerMode && ( rDSDE.DropAction & datatransfer::dnd::DNDConstants::ACTION_MOVE ) )
    {
        if (mpDragAndDropInfo->bStarterOfDD && mpDragAndDropInfo->bDroppedInMe )
        {
            // DropPos: Where was it dropped, irrespective of length.
            ESelection aDropPos(mpDragAndDropInfo->aDropSel.nStartPara, mpDragAndDropInfo->aDropSel.nStartPos, mpDragAndDropInfo->aDropSel.nStartPara, mpDragAndDropInfo->aDropSel.nStartPos );
            ESelection aToBeDelSel = mpDragAndDropInfo->aBeginDragSel;
            ESelection aNewSel( mpDragAndDropInfo->aDropSel.nEndPara, mpDragAndDropInfo->aDropSel.nEndPos,
                                mpDragAndDropInfo->aDropSel.nEndPara, mpDragAndDropInfo->aDropSel.nEndPos );
            bool bBeforeSelection = aDropPos < mpDragAndDropInfo->aBeginDragSel;
            sal_Int32 nParaDiff = mpDragAndDropInfo->aBeginDragSel.nEndPara - mpDragAndDropInfo->aBeginDragSel.nStartPara;
            if ( bBeforeSelection )
            {
                // Adjust aToBeDelSel.
                DBG_ASSERT(mpDragAndDropInfo->aBeginDragSel.nStartPara >= mpDragAndDropInfo->aDropSel.nStartPara, "But not before? ");
                aToBeDelSel.nStartPara = aToBeDelSel.nStartPara + nParaDiff;
                aToBeDelSel.nEndPara = aToBeDelSel.nEndPara + nParaDiff;
                // To correct the character?
                if ( aToBeDelSel.nStartPara == mpDragAndDropInfo->aDropSel.nEndPara )
                {
                    sal_uInt16 nMoreChars;
                    if (mpDragAndDropInfo->aDropSel.nStartPara == mpDragAndDropInfo->aDropSel.nEndPara )
                        nMoreChars = mpDragAndDropInfo->aDropSel.nEndPos - mpDragAndDropInfo->aDropSel.nStartPos;
                    else
                        nMoreChars = mpDragAndDropInfo->aDropSel.nEndPos;
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
                DBG_ASSERT(mpDragAndDropInfo->aBeginDragSel.nStartPara <= mpDragAndDropInfo->aDropSel.nStartPara, "But not before? ");
                aNewSel.nStartPara = aNewSel.nStartPara - nParaDiff;
                aNewSel.nEndPara = aNewSel.nEndPara - nParaDiff;
                // To correct the character?
                if (mpDragAndDropInfo->aBeginDragSel.nEndPara == mpDragAndDropInfo->aDropSel.nStartPara )
                {
                    sal_uInt16 nLessChars;
                    if (mpDragAndDropInfo->aBeginDragSel.nStartPara == mpDragAndDropInfo->aBeginDragSel.nEndPara )
                        nLessChars = mpDragAndDropInfo->aBeginDragSel.nEndPos - mpDragAndDropInfo->aBeginDragSel.nStartPos;
                    else
                        nLessChars = mpDragAndDropInfo->aBeginDragSel.nEndPos;
                    aNewSel.nStartPos = aNewSel.nStartPos - nLessChars;
                    if ( aNewSel.nStartPara == aNewSel.nEndPara )
                        aNewSel.nEndPos = aNewSel.nEndPos - nLessChars;
                }
            }

            DrawSelectionXOR();
            EditSelection aDelSel(getImpEditEngine().CreateSel(aToBeDelSel));
            DBG_ASSERT( !aDelSel.DbgIsBuggy(getEditEngine().GetEditDoc()), "ToBeDel is buggy!");
            getEditEngine().DeleteSelection(aDelSel);
            if ( !bBeforeSelection )
            {
                DBG_ASSERT(!getImpEditEngine().CreateSel(aNewSel).DbgIsBuggy(getEditEngine().GetEditDoc()), "Bad");
                SetEditSelection(getImpEditEngine().CreateSel(aNewSel));
            }
            getImpEditEngine().FormatAndLayout(getImpEditEngine().GetActiveView());
            DrawSelectionXOR();
        }
        else
        {
            // other EditEngine ...
            if (getEditEngine().HasText())   // #88630# SC is removing the content when switching the task
                DeleteSelected();
        }
    }

    if (mpDragAndDropInfo->bUndoAction)
        getImpEditEngine().UndoActionEnd();

    HideDDCursor();
    ShowCursor( DoAutoScroll(), true );
    mpDragAndDropInfo.reset();
    getEditEngine().GetEndDropHdl().Call(GetEditViewPtr());
}

void ImpEditView::drop( const css::datatransfer::dnd::DropTargetDropEvent& rDTDE )
{
    SolarMutexGuard aVclGuard;

    DBG_ASSERT(mpDragAndDropInfo, "Drop - No Drag&Drop info?!");

    if (!(mpDragAndDropInfo && mpDragAndDropInfo->bDragAccepted))
        return;

    getEditEngine().GetBeginDropHdl().Call(GetEditViewPtr());
    bool bChanges = false;

    HideDDCursor();

    if (mpDragAndDropInfo->bStarterOfDD)
    {
        getImpEditEngine().UndoActionStart( EDITUNDO_DRAGANDDROP );
        mpDragAndDropInfo->bUndoAction = true;
    }

    if (mpDragAndDropInfo->bOutlinerMode)
    {
        bChanges = true;
        GetEditViewPtr()->MoveParagraphs(Range(mpDragAndDropInfo->aBeginDragSel.nStartPara, mpDragAndDropInfo->aBeginDragSel.nEndPara ), mpDragAndDropInfo->nOutlinerDropDest);
    }
    else
    {
        uno::Reference<datatransfer::XTransferable> xDataObj = rDTDE.Transferable;
        if ( xDataObj.is() )
        {
            bChanges = true;
            // remove Selection ...
            DrawSelectionXOR();
            EditPaM aPaM(mpDragAndDropInfo->aDropDest);

            PasteOrDropInfos aPasteOrDropInfos;
            aPasteOrDropInfos.nStartPara = getEditEngine().GetEditDoc().GetPos( aPaM.GetNode() );
            getEditEngine().HandleBeginPasteOrDrop(aPasteOrDropInfos);

            EditSelection aNewSel = getEditEngine().InsertText(
                xDataObj, OUString(), aPaM, getEditEngine().GetInternalEditStatus().AllowPasteSpecial());

            aPasteOrDropInfos.nEndPara = getEditEngine().GetEditDoc().GetPos( aNewSel.Max().GetNode() );
            getEditEngine().HandleEndPasteOrDrop(aPasteOrDropInfos);

            SetEditSelection( aNewSel );
            getImpEditEngine().FormatAndLayout(getImpEditEngine().GetActiveView());
            if (mpDragAndDropInfo->bStarterOfDD)
            {
                // Only set if the same engine!
                mpDragAndDropInfo->aDropSel.nStartPara = getEditEngine().GetEditDoc().GetPos( aPaM.GetNode() );
                mpDragAndDropInfo->aDropSel.nStartPos = aPaM.GetIndex();
                mpDragAndDropInfo->aDropSel.nEndPara = getEditEngine().GetEditDoc().GetPos( aNewSel.Max().GetNode() );
                mpDragAndDropInfo->aDropSel.nEndPos = aNewSel.Max().GetIndex();
                mpDragAndDropInfo->bDroppedInMe = true;
            }
        }
    }

    if ( bChanges )
    {
        rDTDE.Context->acceptDrop( rDTDE.DropAction );
    }

    if (!mpDragAndDropInfo->bStarterOfDD)
    {
        mpDragAndDropInfo.reset();
    }

    rDTDE.Context->dropComplete( bChanges );
}

void ImpEditView::dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& rDTDEE )
{
    SolarMutexGuard aVclGuard;

    if (!mpDragAndDropInfo)
        mpDragAndDropInfo.reset(new DragAndDropInfo);

    mpDragAndDropInfo->bHasValidData = false;

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
            mpDragAndDropInfo->bHasValidData = true;
            break;
        }
    }

    dragOver( rDTDEE );
}

void ImpEditView::dragExit( const css::datatransfer::dnd::DropTargetEvent& )
{
    SolarMutexGuard aVclGuard;

    HideDDCursor();

    if (mpDragAndDropInfo && !mpDragAndDropInfo->bStarterOfDD)
    {
        mpDragAndDropInfo.reset();
    }
}

void ImpEditView::dragOver(const css::datatransfer::dnd::DropTargetDragEvent& rDTDE)
{
    SolarMutexGuard aVclGuard;

    const OutputDevice& rOutDev = GetOutputDevice();

    Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );
    aMousePos = rOutDev.PixelToLogic( aMousePos );

    bool bAccept = false;

    if (GetOutputArea().Contains( aMousePos ) && !mbReadOnly)
    {
        if (mpDragAndDropInfo && mpDragAndDropInfo->bHasValidData)
        {
            bAccept = true;

            bool bAllowScroll = DoAutoScroll();
            if ( bAllowScroll )
            {
                tools::Long nScrollX = 0;
                tools::Long nScrollY = 0;
                // Check if in the sensitive area
                if ( ( (aMousePos.X() - mpDragAndDropInfo->nSensibleRange) < GetOutputArea().Left() ) && ( ( aMousePos.X() + mpDragAndDropInfo->nSensibleRange ) > GetOutputArea().Left() ) )
                        nScrollX = GetOutputArea().GetWidth() / SCRLRANGE;
                else if ( ( (aMousePos.X() + mpDragAndDropInfo->nSensibleRange) > GetOutputArea().Right() ) && ( ( aMousePos.X() - mpDragAndDropInfo->nSensibleRange ) < GetOutputArea().Right() ) )
                        nScrollX = -( GetOutputArea().GetWidth() / SCRLRANGE );

                if ( ( (aMousePos.Y() - mpDragAndDropInfo->nSensibleRange) < GetOutputArea().Top() ) && ( ( aMousePos.Y() + mpDragAndDropInfo->nSensibleRange ) > GetOutputArea().Top() ) )
                        nScrollY = GetOutputArea().GetHeight() / SCRLRANGE;
                else if ( ( (aMousePos.Y() + mpDragAndDropInfo->nSensibleRange) > GetOutputArea().Bottom() ) && ( ( aMousePos.Y() - mpDragAndDropInfo->nSensibleRange ) < GetOutputArea().Bottom() ) )
                        nScrollY = -( GetOutputArea().GetHeight() / SCRLRANGE );

                if ( nScrollX || nScrollY )
                {
                    HideDDCursor();
                    Scroll( nScrollX, nScrollY, ScrollRangeCheck::PaperWidthTextSize );
                }
            }

            Point aDocPos( GetDocPos( aMousePos ) );
            EditPaM aPaM = getEditEngine().GetPaM( aDocPos );
            mpDragAndDropInfo->aDropDest = aPaM;
            if (mpDragAndDropInfo->bOutlinerMode)
            {
                sal_Int32 nPara = getEditEngine().GetEditDoc().GetPos( aPaM.GetNode() );
                ParaPortion* pPPortion = getEditEngine().GetParaPortions().SafeGetObject( nPara );
                if (pPPortion)
                {
                    tools::Long nDestParaStartY = getEditEngine().GetParaPortions().GetYOffset( pPPortion );
                    tools::Long nRel = aDocPos.Y() - nDestParaStartY;
                    if ( nRel < ( pPPortion->GetHeight() / 2 ) )
                    {
                        mpDragAndDropInfo->nOutlinerDropDest = nPara;
                    }
                    else
                    {
                        mpDragAndDropInfo->nOutlinerDropDest = nPara+1;
                    }

                    if ((mpDragAndDropInfo->nOutlinerDropDest >= mpDragAndDropInfo->aBeginDragSel.nStartPara) &&
                            (mpDragAndDropInfo->nOutlinerDropDest <= (mpDragAndDropInfo->aBeginDragSel.nEndPara + 1)))
                    {
                        bAccept = false;
                    }
                }
            }
            else if ( HasSelection() )
            {
                // it must not be dropped into a selection
                EPaM aP = getImpEditEngine().CreateEPaM( aPaM );
                ESelection aDestSel( aP.nPara, aP.nIndex, aP.nPara, aP.nIndex);
                ESelection aCurSel = getImpEditEngine().CreateESel( GetEditSelection() );
                aCurSel.Adjust();
                if ( !(aDestSel < aCurSel) && !(aDestSel > aCurSel) )
                {
                    bAccept = false;
                }
            }
            if ( bAccept )
            {
                tools::Rectangle aEditCursor;
                if (mpDragAndDropInfo->bOutlinerMode)
                {
                    tools::Long nDDYPos(0);
                    if (mpDragAndDropInfo->nOutlinerDropDest < getEditEngine().GetEditDoc().Count())
                    {
                        ParaPortion* pPPortion = getEditEngine().GetParaPortions().SafeGetObject(mpDragAndDropInfo->nOutlinerDropDest);
                        if (pPPortion)
                            nDDYPos = getEditEngine().GetParaPortions().GetYOffset( pPPortion );
                    }
                    else
                    {
                        nDDYPos = getImpEditEngine().GetTextHeight();
                    }
                    Point aStartPos( 0, nDDYPos );
                    aStartPos = GetWindowPos( aStartPos );
                    Point aEndPos( GetOutputArea().GetWidth(), nDDYPos );
                    aEndPos = GetWindowPos( aEndPos );
                    aEditCursor = rOutDev.LogicToPixel( tools::Rectangle( aStartPos, aEndPos ) );
                    if (!getEditEngine().IsEffectivelyVertical())
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
                    aEditCursor = getImpEditEngine().PaMtoEditCursor( aPaM );
                    Point aTopLeft( GetWindowPos( aEditCursor.TopLeft() ) );
                    aEditCursor.SetPos( aTopLeft );
                    aEditCursor.SetRight(aEditCursor.Left() + mpDragAndDropInfo->nCursorWidth);
                    aEditCursor = rOutDev.LogicToPixel( aEditCursor );
                    aEditCursor = rOutDev.PixelToLogic( aEditCursor );
                }

                bool bCursorChanged = !mpDragAndDropInfo->bVisCursor || (mpDragAndDropInfo->aCurCursor != aEditCursor);
                if ( bCursorChanged )
                {
                    HideDDCursor();
                    ShowDDCursor(aEditCursor );
                }
                mpDragAndDropInfo->bDragAccepted = true;
                rDTDE.Context->acceptDrag( rDTDE.DropAction );
            }
        }
    }

    if ( !bAccept )
    {
        HideDDCursor();
        if (mpDragAndDropInfo)
            mpDragAndDropInfo->bDragAccepted = false;
        rDTDE.Context->rejectDrag();
    }
}

void ImpEditView::AddDragAndDropListeners()
{
    if (mbActiveDragAndDropListener)
        return;

    uno::Reference<datatransfer::dnd::XDropTarget> xDropTarget;
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
        xDropTarget = pCallbacks->GetDropTarget();
    else if (auto xWindow = GetWindow())
        xDropTarget = xWindow->GetDropTarget();

    if (!xDropTarget.is())
        return;

    mxDnDListener = new vcl::unohelper::DragAndDropWrapper(this);

    uno::Reference<datatransfer::dnd::XDragGestureRecognizer> xDragGestureRecognizer(xDropTarget, uno::UNO_QUERY);
    if (xDragGestureRecognizer.is())
    {
        uno::Reference<datatransfer::dnd::XDragGestureListener> xDGL(mxDnDListener, uno::UNO_QUERY);
        xDragGestureRecognizer->addDragGestureListener(xDGL);
    }

    uno::Reference<datatransfer::dnd::XDropTargetListener> xDTL(mxDnDListener, uno::UNO_QUERY);
    xDropTarget->addDropTargetListener(xDTL);
    xDropTarget->setActive(true);
    xDropTarget->setDefaultActions(datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE);

    mbActiveDragAndDropListener = true;
}

void ImpEditView::RemoveDragAndDropListeners()
{
    if (!mbActiveDragAndDropListener)
        return;

    uno::Reference<datatransfer::dnd::XDropTarget> xDropTarget;
    if (EditViewCallbacks* pCallbacks = getEditViewCallbacks())
        xDropTarget = pCallbacks->GetDropTarget();
    else if (auto xWindow = GetWindow())
        xDropTarget = xWindow->GetDropTarget();

    if (xDropTarget.is())
    {
        uno::Reference<datatransfer::dnd::XDragGestureRecognizer> xDragGestureRecognizer(xDropTarget, uno::UNO_QUERY);
        if (xDragGestureRecognizer.is())
        {
            uno::Reference<datatransfer::dnd::XDragGestureListener> xDGL(mxDnDListener, uno::UNO_QUERY);
            xDragGestureRecognizer->removeDragGestureListener(xDGL);
        }

        uno::Reference<datatransfer::dnd::XDropTargetListener> xDTL(mxDnDListener, uno::UNO_QUERY);
        xDropTarget->removeDropTargetListener(xDTL);
    }

    if ( mxDnDListener.is() )
    {
        mxDnDListener->disposing( lang::EventObject() );  // #95154# Empty Source means it's the Client
        mxDnDListener.clear();
    }

    mbActiveDragAndDropListener = false;
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

void ImpEditView::SetLOKSpecialFlags(LOKSpecialFlags eFlags)
{
    assert(mpLOKSpecialPositioning);
    mpLOKSpecialPositioning->SetFlags(eFlags);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
