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

#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <impedit.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <tools/poly.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/linguistic2/XDictionaryEntry.hpp>
#include <com/sun/star/linguistic2/DictionaryType.hpp>
#include <com/sun/star/linguistic2/DictionaryEvent.hpp>
#include <com/sun/star/linguistic2/XDictionaryEventListener.hpp>
#include <com/sun/star/linguistic2/DictionaryEventFlags.hpp>
#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <osl/mutex.hxx>
#include <editeng/flditem.hxx>
#include <svl/intitem.hxx>
#include <svtools/transfer.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/string.hxx>
#include <comphelper/lok.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

#define SCRLRANGE   20  // Scroll 1/20 of the width/height, when in QueryDrop

static inline void lcl_AllignToPixel( Point& rPoint, OutputDevice* pOutDev, short nDiffX, short nDiffY )
{
    rPoint = pOutDev->LogicToPixel( rPoint );

    if ( nDiffX )
        rPoint.X() += nDiffX;
    if ( nDiffY )
        rPoint.Y() += nDiffY;

    rPoint = pOutDev->PixelToLogic( rPoint );
}


//  class ImpEditView

ImpEditView::ImpEditView( EditView* pView, EditEngine* pEng, vcl::Window* pWindow ) :
    aOutArea( Point(), pEng->GetPaperSize() )
{
    pEditView           = pView;
    pEditEngine         = pEng;
    pOutWin             = pWindow;
    pPointer            = nullptr;
    pBackgroundColor    = nullptr;
    mpViewShell         = nullptr;
    mpOtherShell        = nullptr;
    nScrollDiffX        = 0;
    nExtraCursorFlags   = GetCursorFlags::NONE;
    nCursorBidiLevel    = CURSOR_BIDILEVEL_DONTKNOW;
    pCursor             = nullptr;
    pDragAndDropInfo    = nullptr;
    bReadOnly           = false;
    bClickedInSelection = false;
    eSelectionMode      = EE_SELMODE_TXTONLY;
    eAnchorMode         = ANCHOR_TOP_LEFT;
    nInvMore            = 1;
    nTravelXPos         = TRAVEL_X_DONTKNOW;
    nControl            = EVControlBits::AUTOSCROLL | EVControlBits::ENABLEPASTE;
    bActiveDragAndDropListener = false;

    aEditSelection.Min() = pEng->GetEditDoc().GetStartPaM();
    aEditSelection.Max() = pEng->GetEditDoc().GetEndPaM();
}

ImpEditView::~ImpEditView()
{
    RemoveDragAndDropListeners();

    if ( pOutWin && ( pOutWin->GetCursor() == pCursor ) )
        pOutWin->SetCursor( nullptr );

    delete pCursor;
    delete pBackgroundColor;
    delete pPointer;
    delete pDragAndDropInfo;
}

void ImpEditView::SetBackgroundColor( const Color& rColor )
{
    delete pBackgroundColor;
    pBackgroundColor = new Color( rColor );
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
        //EENotify aNotify( EE_NOTIFY_TEXTVIEWSELECTIONCHANGED );
        EENotify aNotify( eNotifyType );
        aNotify.pEditEngine = pEditEngine;
        aNotify.pEditView = GetEditViewPtr();
        pEditEngine->pImpEditEngine->CallNotify( aNotify );
    }
}

/// Translate absolute <-> relative twips: LOK wants absolute coordinates as output and gives absolute coordinates as input.
void lcl_translateTwips(vcl::Window& rParent, vcl::Window& rChild)
{
    // Don't translate if we already have a non-zero origin.
    // This prevents multiple translate calls that negate
    // one another.
    const Point aOrigin = rChild.GetMapMode().GetOrigin();
    if (aOrigin.getX() == 0 && aOrigin.getY() == 0)
    {
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
}

void ImpEditView::DrawSelection( EditSelection aTmpSel, vcl::Region* pRegion, OutputDevice* pTargetDevice )
{
    if ( eSelectionMode == EE_SELMODE_HIDDEN )
        return;

    // It must be ensured before rendering the selection, that the contents of
    // the window is completely valid! Must be here so that in any case if
    // empty, then later on two-Paint Events! Must be done even before the
    // query from bUpdate, if after Invalidate paints still in the queue,
    // but someone switches the update mode!

    // pRegion: When not NULL, then only calculate Region.

    vcl::Region* pOldRegion = pRegion;
    vcl::Region aRegion;
    if (comphelper::LibreOfficeKit::isActive() && !pRegion)
        pRegion = &aRegion;

    OutputDevice* pTarget = pTargetDevice ? pTargetDevice : pOutWin;
    bool bClipRegion = pTarget->IsClipRegion();
    vcl::Region aOldRegion = pTarget->GetClipRegion();

    tools::PolyPolygon* pPolyPoly = nullptr;

    if ( !pRegion )
    {
        if ( !pEditEngine->pImpEditEngine->GetUpdateMode() )
            return;
        if ( pEditEngine->pImpEditEngine->IsInUndo() )
            return;

        if ( !aTmpSel.HasRange() )
            return;

        // aTmpOutArea: if OutputArea > Paper width and
        // Text > Paper width ( over large fields )
        Rectangle aTmpOutArea( aOutArea );
        if ( aTmpOutArea.GetWidth() > pEditEngine->pImpEditEngine->GetPaperSize().Width() )
            aTmpOutArea.Right() = aTmpOutArea.Left() + pEditEngine->pImpEditEngine->GetPaperSize().Width();
        pTarget->IntersectClipRegion( aTmpOutArea );

        if ( pOutWin->GetCursor() )
            pOutWin->GetCursor()->Hide();
    }
    else
    {
        pPolyPoly = new tools::PolyPolygon;
    }

    DBG_ASSERT( !pEditEngine->IsIdleFormatterActive(), "DrawSelection: Not formatted!" );
    aTmpSel.Adjust( pEditEngine->GetEditDoc() );

    ContentNode* pStartNode = aTmpSel.Min().GetNode();
    ContentNode* pEndNode = aTmpSel.Max().GetNode();
    sal_Int32 nStartPara = pEditEngine->GetEditDoc().GetPos( pStartNode );
    sal_Int32 nEndPara = pEditEngine->GetEditDoc().GetPos( pEndNode );
    for ( sal_Int32 nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        ParaPortion* pTmpPortion = pEditEngine->GetParaPortions().SafeGetObject( nPara );
        DBG_ASSERT( pTmpPortion, "Portion in Selection not found!" );
        DBG_ASSERT( !pTmpPortion->IsInvalid(), "Portion in Selection not formatted!" );

        if ( !pTmpPortion->IsVisible() || pTmpPortion->IsInvalid() )
            continue;

        long nParaStart = pEditEngine->GetParaPortions().GetYOffset( pTmpPortion );
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

            Rectangle aTmpRect( pEditEngine->pImpEditEngine->GetEditCursor( pTmpPortion, nStartIndex ) );
            Point aTopLeft( aTmpRect.TopLeft() );
            Point aBottomRight( aTmpRect.BottomRight() );

            aTopLeft.Y() += nParaStart;
            aBottomRight.Y() += nParaStart;

            // Only paint if in the visible range ...
            if ( aTopLeft.Y() > GetVisDocBottom() )
                break;

            if ( aBottomRight.Y() < GetVisDocTop() )
                continue;

            // Now that we have Bidi, the first/last index doesn't have to be the 'most outside' position
            if ( !bPartOfLine )
            {
                Range aLineXPosStartEnd = pEditEngine->GetLineXPosStartEnd(pTmpPortion, &rLine);
                aTopLeft.X() = aLineXPosStartEnd.Min();
                aBottomRight.X() = aLineXPosStartEnd.Max();
                ImplDrawHighlightRect( pTarget, aTopLeft, aBottomRight, pPolyPoly );
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

                    DBG_ASSERT( nTmpEndIndex > nTmpStartIndex, "DrawSelection, Start >= End?" );

                    long nX1 = pEditEngine->GetXPos(pTmpPortion, &rLine, nTmpStartIndex, true);
                    long nX2 = pEditEngine->GetXPos(pTmpPortion, &rLine, nTmpEndIndex);

                    Point aPt1( std::min( nX1, nX2 ), aTopLeft.Y() );
                    Point aPt2( std::max( nX1, nX2 ), aBottomRight.Y() );

                    ImplDrawHighlightRect( pTarget, aPt1, aPt2, pPolyPoly );

                    nTmpStartIndex = nTmpEndIndex;
                }
            }

        }
    }

    if ( pRegion )
    {
        *pRegion = vcl::Region( *pPolyPoly );

        if (comphelper::LibreOfficeKit::isActive() && mpViewShell && !pOldRegion)
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

            bool bMm100ToTwip = pOutWin->GetMapMode().GetMapUnit() == MapUnit::Map100thMM;

            Point aOrigin;
            if (pOutWin->GetMapMode().GetMapUnit() == MapUnit::MapTwip)
                // Writer comments: they use editeng, but are separate widgets.
                aOrigin = pOutWin->GetMapMode().GetOrigin();

            OString sRectangle;
            // If we are not in selection mode, then the exported own selection should be empty.
            if (pEditEngine->pImpEditEngine->IsInSelectionMode() || mpOtherShell)
            {
                std::vector<Rectangle> aRectangles;
                pRegion->GetRegionRectangles(aRectangles);

                if (!aRectangles.empty())
                {
                    Rectangle& rStart = aRectangles.front();
                    Rectangle aStart = Rectangle(rStart.Left(), rStart.Top(), rStart.Left() + 1, rStart.Bottom());
                    if (bMm100ToTwip)
                        aStart = OutputDevice::LogicToLogic(aStart, MapUnit::Map100thMM, MapUnit::MapTwip);
                    aStart.Move(aOrigin.getX(), aOrigin.getY());

                    mpViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_START, aStart.toString().getStr());

                    Rectangle& rEnd = aRectangles.back();
                    Rectangle aEnd = Rectangle(rEnd.Right() - 1, rEnd.Top(), rEnd.Right(), rEnd.Bottom());
                    if (bMm100ToTwip)
                        aEnd = OutputDevice::LogicToLogic(aEnd, MapUnit::Map100thMM, MapUnit::MapTwip);
                    aEnd.Move(aOrigin.getX(), aOrigin.getY());

                    mpViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION_END, aEnd.toString().getStr());
                }

                std::vector<OString> v;
                for (Rectangle & rRectangle : aRectangles)
                {
                    if (bMm100ToTwip)
                        rRectangle = OutputDevice::LogicToLogic(rRectangle, MapUnit::Map100thMM, MapUnit::MapTwip);
                    rRectangle.Move(aOrigin.getX(), aOrigin.getY());
                    v.push_back(rRectangle.toString().getStr());
                }
                sRectangle = comphelper::string::join("; ", v);
            }

            if (mpOtherShell)
            {
                // An other shell wants to know about our existing selection.
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

        delete pPolyPoly;
    }
    else
    {
        if ( pOutWin->GetCursor() )
            pOutWin->GetCursor()->Show();

        if ( bClipRegion )
            pTarget->SetClipRegion( aOldRegion );
        else
            pTarget->SetClipRegion();
    }
}

void ImpEditView::GetSelectionRectangles(std::vector<Rectangle>& rLogicRects)
{
    bool bMm100ToTwip = pOutWin->GetMapMode().GetMapUnit() == MapUnit::Map100thMM;
    vcl::Region aRegion;
    DrawSelection(aEditSelection, &aRegion);
    aRegion.GetRegionRectangles(rLogicRects);

    for (Rectangle& rRectangle : rLogicRects)
    {
        if (bMm100ToTwip)
            rRectangle = OutputDevice::LogicToLogic(rRectangle, MapUnit::Map100thMM, MapUnit::MapTwip);
    }
}

void ImpEditView::ImplDrawHighlightRect( OutputDevice* _pTarget, const Point& rDocPosTopLeft, const Point& rDocPosBottomRight, tools::PolyPolygon* pPolyPoly )
{
    if ( rDocPosTopLeft.X() != rDocPosBottomRight.X() )
    {
        bool bPixelMode = _pTarget->GetMapMode() == MapUnit::MapPixel;

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

        Rectangle aRect( aPnt1, aPnt2 );
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
}


bool ImpEditView::IsVertical() const
{
    return pEditEngine->pImpEditEngine->IsVertical();
}

Rectangle ImpEditView::GetVisDocArea() const
{
    return Rectangle( GetVisDocLeft(), GetVisDocTop(), GetVisDocRight(), GetVisDocBottom() );
}

Point ImpEditView::GetDocPos( const Point& rWindowPos ) const
{
    // Window Position => Position Document
    Point aPoint;

    if ( !pEditEngine->pImpEditEngine->IsVertical() )
    {
        aPoint.X() = rWindowPos.X() - aOutArea.Left() + GetVisDocLeft();
        aPoint.Y() = rWindowPos.Y() - aOutArea.Top() + GetVisDocTop();
    }
    else
    {
        aPoint.X() = rWindowPos.Y() - aOutArea.Top() + GetVisDocLeft();
        aPoint.Y() = aOutArea.Right() - rWindowPos.X() + GetVisDocTop();
    }

    return aPoint;
}

Point ImpEditView::GetWindowPos( const Point& rDocPos ) const
{
    // Document position => window position
    Point aPoint;

    if ( !pEditEngine->pImpEditEngine->IsVertical() )
    {
        aPoint.X() = rDocPos.X() + aOutArea.Left() - GetVisDocLeft();
        aPoint.Y() = rDocPos.Y() + aOutArea.Top() - GetVisDocTop();
    }
    else
    {
        aPoint.X() = aOutArea.Right() - rDocPos.Y() + GetVisDocTop();
        aPoint.Y() = rDocPos.X() + aOutArea.Top() - GetVisDocLeft();
    }

    return aPoint;
}

Rectangle ImpEditView::GetWindowPos( const Rectangle& rDocRect ) const
{
    // Document position => window position
    Point aPos( GetWindowPos( rDocRect.TopLeft() ) );
    Size aSz = rDocRect.GetSize();
    Rectangle aRect;
    if ( !pEditEngine->pImpEditEngine->IsVertical() )
    {
        aRect = Rectangle( aPos, aSz );
    }
    else
    {
        Point aNewPos( aPos.X()-aSz.Height(), aPos.Y() );
        aRect = Rectangle( aNewPos, Size( aSz.Height(), aSz.Width() ) );
    }
    return aRect;
}

void ImpEditView::SetSelectionMode( EESelectionMode eNewMode )
{
    if ( eSelectionMode != eNewMode )
    {
        DrawSelection();
        eSelectionMode = eNewMode;
        DrawSelection();    // redraw
    }
}

void ImpEditView::SetOutputArea( const Rectangle& rRect )
{
    // should be better be aligned on pixels!
    Rectangle aNewRect( pOutWin->LogicToPixel( rRect ) );
    aNewRect = pOutWin->PixelToLogic( aNewRect );
    aOutArea = aNewRect;
    if ( aOutArea.Right() < aOutArea.Left() )
        aOutArea.Right() = aOutArea.Left();
    if ( aOutArea.Bottom() < aOutArea.Top() )
        aOutArea.Bottom() = aOutArea.Top();

    if ( DoBigScroll() )
        SetScrollDiffX( (sal_uInt16)aOutArea.GetWidth() * 3 / 10 );
    else
        SetScrollDiffX( (sal_uInt16)aOutArea.GetWidth() * 2 / 10 );
}

void ImpEditView::ResetOutputArea( const Rectangle& rRect )
{
    // remember old out area
    const Rectangle aOldArea(aOutArea);

    // apply new one
    SetOutputArea(rRect);

    // invalidate surrounding areas if update is true
    if(!aOldArea.IsEmpty() && pEditEngine->pImpEditEngine->GetUpdateMode())
    {
        // #i119885# use grown area if needed; do when getting bigger OR smaller
        const sal_Int32 nMore(DoInvalidateMore() ? GetWindow()->PixelToLogic(Size(nInvMore, 0)).Width() : 0);

        if(aOldArea.Left() > aOutArea.Left())
        {
            GetWindow()->Invalidate(Rectangle(aOutArea.Left() - nMore, aOldArea.Top() - nMore, aOldArea.Left(), aOldArea.Bottom() + nMore));
        }
        else if(aOldArea.Left() < aOutArea.Left())
        {
            GetWindow()->Invalidate(Rectangle(aOldArea.Left() - nMore, aOldArea.Top() - nMore, aOutArea.Left(), aOldArea.Bottom() + nMore));
        }

        if(aOldArea.Right() > aOutArea.Right())
        {
            GetWindow()->Invalidate(Rectangle(aOutArea.Right(), aOldArea.Top() - nMore, aOldArea.Right() + nMore, aOldArea.Bottom() + nMore));
        }
        else if(aOldArea.Right() < aOutArea.Right())
        {
            GetWindow()->Invalidate(Rectangle(aOldArea.Right(), aOldArea.Top() - nMore, aOutArea.Right() + nMore, aOldArea.Bottom() + nMore));
        }

        if(aOldArea.Top() > aOutArea.Top())
        {
            GetWindow()->Invalidate(Rectangle(aOldArea.Left() - nMore, aOutArea.Top() - nMore, aOldArea.Right() + nMore, aOldArea.Top()));
        }
        else if(aOldArea.Top() < aOutArea.Top())
        {
            GetWindow()->Invalidate(Rectangle(aOldArea.Left() - nMore, aOldArea.Top() - nMore, aOldArea.Right() + nMore, aOutArea.Top()));
        }

        if(aOldArea.Bottom() > aOutArea.Bottom())
        {
            GetWindow()->Invalidate(Rectangle(aOldArea.Left() - nMore, aOutArea.Bottom(), aOldArea.Right() + nMore, aOldArea.Bottom() + nMore));
        }
        else if(aOldArea.Bottom() < aOutArea.Bottom())
        {
            GetWindow()->Invalidate(Rectangle(aOldArea.Left() - nMore, aOldArea.Bottom(), aOldArea.Right() + nMore, aOutArea.Bottom() + nMore));
        }
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
            aNewSz.Width() = pEditEngine->pImpEditEngine->GetPaperSize().Width();
        switch ( eAnchorMode )
        {
            case ANCHOR_TOP_LEFT:
            case ANCHOR_VCENTER_LEFT:
            case ANCHOR_BOTTOM_LEFT:
            {
                aNewTopLeft.X() = aAnchorPoint.X();
            }
            break;
            case ANCHOR_TOP_HCENTER:
            case ANCHOR_VCENTER_HCENTER:
            case ANCHOR_BOTTOM_HCENTER:
            {
                aNewTopLeft.X() = aAnchorPoint.X() - aNewSz.Width() / 2;
            }
            break;
            case ANCHOR_TOP_RIGHT:
            case ANCHOR_VCENTER_RIGHT:
            case ANCHOR_BOTTOM_RIGHT:
            {
                aNewTopLeft.X() = aAnchorPoint.X() - aNewSz.Width() - 1;
            }
            break;
        }
    }

    // Y:
    if ( DoAutoHeight() )
    {
        if ( pEditEngine->pImpEditEngine->GetStatus().AutoPageHeight() )
            aNewSz.Height() = pEditEngine->pImpEditEngine->GetPaperSize().Height();
        switch ( eAnchorMode )
        {
            case ANCHOR_TOP_LEFT:
            case ANCHOR_TOP_HCENTER:
            case ANCHOR_TOP_RIGHT:
            {
                aNewTopLeft.Y() = aAnchorPoint.Y();
            }
            break;
            case ANCHOR_VCENTER_LEFT:
            case ANCHOR_VCENTER_HCENTER:
            case ANCHOR_VCENTER_RIGHT:
            {
                aNewTopLeft.Y() = aAnchorPoint.Y() - aNewSz.Height() / 2;
            }
            break;
            case ANCHOR_BOTTOM_LEFT:
            case ANCHOR_BOTTOM_HCENTER:
            case ANCHOR_BOTTOM_RIGHT:
            {
                aNewTopLeft.Y() = aAnchorPoint.Y() - aNewSz.Height() - 1;
            }
            break;
        }
    }
    ResetOutputArea( Rectangle( aNewTopLeft, aNewSz ) );
}

void ImpEditView::SetAnchorMode( EVAnchorMode eMode )
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
        case ANCHOR_TOP_LEFT:
        case ANCHOR_VCENTER_LEFT:
        case ANCHOR_BOTTOM_LEFT:
        {
            aAnchorPoint.X() = aOutArea.Left();
        }
        break;
        case ANCHOR_TOP_HCENTER:
        case ANCHOR_VCENTER_HCENTER:
        case ANCHOR_BOTTOM_HCENTER:
        {
            aAnchorPoint.X() = aOutArea.Left() + (aOutArea.GetWidth()-1) / 2;
        }
        break;
        case ANCHOR_TOP_RIGHT:
        case ANCHOR_VCENTER_RIGHT:
        case ANCHOR_BOTTOM_RIGHT:
        {
            aAnchorPoint.X() = aOutArea.Right();
        }
        break;
    }

    // Y:
    switch ( eAnchorMode )
    {
        case ANCHOR_TOP_LEFT:
        case ANCHOR_TOP_HCENTER:
        case ANCHOR_TOP_RIGHT:
        {
            aAnchorPoint.Y() = aOutArea.Top();
        }
        break;
        case ANCHOR_VCENTER_LEFT:
        case ANCHOR_VCENTER_HCENTER:
        case ANCHOR_VCENTER_RIGHT:
        {
            aAnchorPoint.Y() = aOutArea.Top() + (aOutArea.GetHeight()-1) / 2;
        }
        break;
        case ANCHOR_BOTTOM_LEFT:
        case ANCHOR_BOTTOM_HCENTER:
        case ANCHOR_BOTTOM_RIGHT:
        {
            aAnchorPoint.Y() = aOutArea.Bottom() - 1;
        }
        break;
    }
}

void ImpEditView::ShowCursor( bool bGotoCursor, bool bForceVisCursor )
{
    // No ShowCursor in an empty View ...
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

    if ( pOutWin->GetCursor() != GetCursor() )
        pOutWin->SetCursor( GetCursor() );

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

    Rectangle aEditCursor = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM, nShowCursorFlags );
    if ( !IsInsertMode() && !aEditSelection.HasRange() )
    {
        if ( aPaM.GetNode()->Len() && ( aPaM.GetIndex() < aPaM.GetNode()->Len() ) )
        {
            // If we are behind a portion, and the next portion has other direction, we must change position...
            aEditCursor.Left() = aEditCursor.Right() = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM, GetCursorFlags::TextOnly|GetCursorFlags::PreferPortionStart ).Left();

            sal_Int32 nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, true );
            const TextPortion& rTextPortion = pParaPortion->GetTextPortions()[nTextPortion];
            if ( rTextPortion.GetKind() == PortionKind::TAB )
            {
                aEditCursor.Right() += rTextPortion.GetSize().Width();
            }
            else
            {
                EditPaM aNext = pEditEngine->CursorRight( aPaM );
                Rectangle aTmpRect = pEditEngine->pImpEditEngine->PaMtoEditCursor( aNext, GetCursorFlags::TextOnly );
                if ( aTmpRect.Top() != aEditCursor.Top() )
                    aTmpRect = pEditEngine->pImpEditEngine->PaMtoEditCursor( aNext, GetCursorFlags::TextOnly|GetCursorFlags::EndOfLine );
                aEditCursor.Right() = aTmpRect.Left();
            }
        }
    }
    long nMaxHeight = !IsVertical() ? aOutArea.GetHeight() : aOutArea.GetWidth();
    if ( aEditCursor.GetHeight() > nMaxHeight )
    {
        aEditCursor.Bottom() = aEditCursor.Top() + nMaxHeight - 1;
    }
    if ( bGotoCursor  ) // && (!pEditEngine->pImpEditEngine->GetStatus().AutoPageSize() ) )
    {
        // check if scrolling is necessary...
        // if scrolling, then update () and Scroll ()!
        long nDocDiffX = 0;
        long nDocDiffY = 0;

        Rectangle aTmpVisArea( GetVisDocArea() );
        // aTmpOutArea: if OutputArea > Paper width and
        // Text > Paper width ( over large fields )
        long nMaxTextWidth = !IsVertical() ? pEditEngine->pImpEditEngine->GetPaperSize().Width() : pEditEngine->pImpEditEngine->GetPaperSize().Height();
        if ( aTmpVisArea.GetWidth() > nMaxTextWidth )
            aTmpVisArea.Right() = aTmpVisArea.Left() + nMaxTextWidth;

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
            // Scroll left, positiv
            nDocDiffX = aEditCursor.Right() - aTmpVisArea.Right();
            // Can it be a little more?
            if ( aEditCursor.Right() < ( nMaxTextWidth - GetScrollDiffX() ) )
                nDocDiffX += GetScrollDiffX();
            else
            {
                long n = nMaxTextWidth - aEditCursor.Right();
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
            if ( aEditCursor.Left() > ( - (long)GetScrollDiffX() ) )
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
            long nDiffX = !IsVertical() ? nDocDiffX : -nDocDiffY;
            long nDiffY = !IsVertical() ? nDocDiffY : nDocDiffX;

            // Negative: Back to the top or left edge
            if ( ( std::abs( nDiffY ) > pEditEngine->GetOnePixelInRef() ) && DoBigScroll() )
            {
                long nH = aOutArea.GetHeight() / 4;
                if ( ( nH > aEditCursor.GetHeight() ) && ( std::abs( nDiffY ) < nH ) )
                {
                    if ( nDiffY < 0 )
                        nDiffY -= nH;
                    else
                        nDiffY += nH;
                }
            }

            if ( ( std::abs( nDiffX ) > pEditEngine->GetOnePixelInRef() ) && DoBigScroll() )
            {
                long nW = aOutArea.GetWidth() / 4;
                if ( std::abs( nDiffX ) < nW )
                {
                    if ( nDiffY < 0 )
                        nDiffY -= nW;
                    else
                        nDiffY += nW;
                }
            }

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
            aEditCursor.Bottom() = GetVisDocBottom();
        if ( aEditCursor.Top() < GetVisDocTop() )
            aEditCursor.Top() = GetVisDocTop();
    }

    long nOnePixel = pOutWin->PixelToLogic( Size( 1, 0 ) ).Width();

    if ( /* pEditEngine->pImpEditEngine->GetStatus().AutoPageSize() || */
         ( ( aEditCursor.Top() + nOnePixel >= GetVisDocTop() ) &&
         ( aEditCursor.Bottom() - nOnePixel <= GetVisDocBottom() ) &&
         ( aEditCursor.Left() + nOnePixel >= GetVisDocLeft() ) &&
         ( aEditCursor.Right() - nOnePixel <= GetVisDocRight() ) ) )
    {
        Rectangle aCursorRect = GetWindowPos( aEditCursor );
        GetCursor()->SetPos( aCursorRect.TopLeft() );
        Size aCursorSz( aCursorRect.GetSize() );
        // Rectangle is inclusive
        aCursorSz.Width()--;
        aCursorSz.Height()--;
        if ( !aCursorSz.Width() || !aCursorSz.Height() )
        {
            long nCursorSz = pOutWin->GetSettings().GetStyleSettings().GetCursorSize();
            nCursorSz = pOutWin->PixelToLogic( Size( nCursorSz, 0 ) ).Width();
            if ( !aCursorSz.Width() )
                aCursorSz.Width() = nCursorSz;
            if ( !aCursorSz.Height() )
                aCursorSz.Height() = nCursorSz;
        }
        // #111036# Let VCL do orientation for cursor, otherwise problem when cursor has direction flag
        if ( IsVertical() )
        {
            Size aOldSz( aCursorSz );
            aCursorSz.Width() = aOldSz.Height();
            aCursorSz.Height() = aOldSz.Width();
            GetCursor()->SetPos( aCursorRect.TopRight() );
            GetCursor()->SetOrientation( 2700 );
        }
        else
            // #i32593# Reset correct orientation in horizontal layout
            GetCursor()->SetOrientation();

        GetCursor()->SetSize( aCursorSz );

        if (comphelper::LibreOfficeKit::isActive() && mpViewShell)
        {
            const Point& rPos = GetCursor()->GetPos();
            Rectangle aRect(rPos.getX(), rPos.getY(), rPos.getX() + GetCursor()->GetWidth(), rPos.getY() + GetCursor()->GetHeight());

            // LOK output is always in twips, convert from mm100 if necessary.
            if (pOutWin->GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
                aRect = OutputDevice::LogicToLogic(aRect, MapUnit::Map100thMM, MapUnit::MapTwip);
            else if (pOutWin->GetMapMode().GetMapUnit() == MapUnit::MapTwip)
            {
                // Writer comments: they use editeng, but are separate widgets.
                Point aOrigin = pOutWin->GetMapMode().GetOrigin();
                // Move the rectangle, so that we output absolute twips.
                aRect.Move(aOrigin.getX(), aOrigin.getY());
            }
            // Let the LOK client decide the cursor width.
            aRect.setWidth(0);

            OString sRect = aRect.toString();
            if (mpOtherShell)
            {
                // An other shell wants to know about our existing cursor.
                if (mpViewShell != mpOtherShell)
                    mpViewShell->NotifyOtherView(mpOtherShell, LOK_CALLBACK_INVALIDATE_VIEW_CURSOR, "rectangle", sRect);
            }
            else
            {
                mpViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, sRect.getStr());
                mpViewShell->NotifyOtherViews(LOK_CALLBACK_INVALIDATE_VIEW_CURSOR, "rectangle", sRect);
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
            InputContextFlags nContextFlags = InputContextFlags::Text|InputContextFlags::ExtText;
            GetWindow()->SetInputContext( InputContext( aFont, nContextFlags ) );
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

Pair ImpEditView::Scroll( long ndX, long ndY, ScrollRangeCheck nRangeCheck )
{
    DBG_ASSERT( pEditEngine->pImpEditEngine->IsFormatted(), "Scroll: Not formatted!" );
    if ( !ndX && !ndY )
        return Pair( 0, 0 );

#ifdef DBG_UTIL
    Rectangle aR( aOutArea );
    aR = pOutWin->LogicToPixel( aR );
    aR = pOutWin->PixelToLogic( aR );
    SAL_WARN_IF( aR != aOutArea, "editeng", "OutArea before Scroll not aligned" );
#endif

    Rectangle aNewVisArea( GetVisDocArea() );

    // Vertical:
    if ( !IsVertical() )
    {
        aNewVisArea.Top() -= ndY;
        aNewVisArea.Bottom() -= ndY;
    }
    else
    {
        aNewVisArea.Top() += ndX;
        aNewVisArea.Bottom() += ndX;
    }
    if ( ( nRangeCheck == ScrollRangeCheck::PaperWidthTextSize ) && ( aNewVisArea.Bottom() > (long)pEditEngine->pImpEditEngine->GetTextHeight() ) )
    {
        // GetTextHeight still optimizing!
        long nDiff = pEditEngine->pImpEditEngine->GetTextHeight() - aNewVisArea.Bottom(); // negative
        aNewVisArea.Move( 0, nDiff );   // could end up in the negative area...
    }
    if ( ( aNewVisArea.Top() < 0 ) && ( nRangeCheck != ScrollRangeCheck::NONE ) )
        aNewVisArea.Move( 0, -aNewVisArea.Top() );

    // Horizontal:
    if ( !IsVertical() )
    {
        aNewVisArea.Left() -= ndX;
        aNewVisArea.Right() -= ndX;
    }
    else
    {
        aNewVisArea.Left() -= ndY;
        aNewVisArea.Right() -= ndY;
    }
    if ( ( nRangeCheck == ScrollRangeCheck::PaperWidthTextSize ) && ( aNewVisArea.Right() > (long)pEditEngine->pImpEditEngine->CalcTextWidth( false ) ) )
    {
        long nDiff = pEditEngine->pImpEditEngine->CalcTextWidth( false ) - aNewVisArea.Right();     // negative
        aNewVisArea.Move( nDiff, 0 );   // could end up in the negative area...
    }
    if ( ( aNewVisArea.Left() < 0 ) && ( nRangeCheck != ScrollRangeCheck::NONE ) )
        aNewVisArea.Move( -aNewVisArea.Left(), 0 );

    // The difference must be alignt on pixel (due to scroll!)
    long nDiffX = !IsVertical() ? ( GetVisDocLeft() - aNewVisArea.Left() ) : -( GetVisDocTop() - aNewVisArea.Top() );
    long nDiffY = !IsVertical() ? ( GetVisDocTop() - aNewVisArea.Top() ) : ( GetVisDocLeft() - aNewVisArea.Left() );

    Size aDiffs( nDiffX, nDiffY );
    aDiffs = pOutWin->LogicToPixel( aDiffs );
    aDiffs = pOutWin->PixelToLogic( aDiffs );

    long nRealDiffX = aDiffs.Width();
    long nRealDiffY = aDiffs.Height();


    if ( nRealDiffX || nRealDiffY )
    {
        vcl::Cursor* pCrsr = GetCursor();
        bool bVisCursor = pCrsr->IsVisible();
        pCrsr->Hide();
        pOutWin->Update();
        if ( !IsVertical() )
            aVisDocStartPos.Move( -nRealDiffX, -nRealDiffY );
        else
            aVisDocStartPos.Move( -nRealDiffY, nRealDiffX );
        // Das Move um den allignten Wert ergibt nicht unbedingt ein
        // alligntes Rechteck...
        // Aligned value of the move does not necessarily result in aligned
        // rectangle ...
        aVisDocStartPos = pOutWin->LogicToPixel( aVisDocStartPos );
        aVisDocStartPos = pOutWin->PixelToLogic( aVisDocStartPos );
        Rectangle aRect( aOutArea );
        pOutWin->Scroll( nRealDiffX, nRealDiffY, aRect, ScrollFlags::Clip );

        if (comphelper::LibreOfficeKit::isActive())
        {
            // Need to invalidate the window, otherwise no tile will be re-painted.
            pEditView->Invalidate();
        }

        pOutWin->Update();
        pCrsr->SetPos( pCrsr->GetPos() + Point( nRealDiffX, nRealDiffY ) );
        if ( bVisCursor )
        {
            Rectangle aCursorRect( pCrsr->GetPos(), pCrsr->GetSize() );
            if ( aOutArea.IsInside( aCursorRect ) )
                pCrsr->Show();
        }

        if ( pEditEngine->pImpEditEngine->GetNotifyHdl().IsSet() )
        {
            EENotify aNotify( EE_NOTIFY_TEXTVIEWSCROLLED );
            aNotify.pEditEngine = pEditEngine;
            aNotify.pEditView = GetEditViewPtr();
            pEditEngine->pImpEditEngine->CallNotify( aNotify );
        }
    }

    return Pair( nRealDiffX, nRealDiffY );
}

bool ImpEditView::PostKeyEvent( const KeyEvent& rKeyEvent, vcl::Window* pFrameWin )
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
                    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
                    CutCopy( aClipBoard, true );
                    bDone = true;
                }
            }
            break;
            case KeyFuncType::COPY:
            {
                Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
                CutCopy( aClipBoard, false );
                bDone = true;
            }
            break;
            case KeyFuncType::PASTE:
            {
                if ( !bReadOnly && IsPasteEnabled() )
                {
                    pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_PASTE );
                    Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
                    Paste( aClipBoard, pEditEngine->pImpEditEngine->GetStatus().AllowPasteSpecial() );
                    pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_PASTE );
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
    if ( pEditEngine->GetInternalEditStatus().NotifyCursorMovements() )
    {
        if ( pEditEngine->GetInternalEditStatus().GetPrevParagraph() != pEditEngine->GetEditDoc().GetPos( GetEditSelection().Max().GetNode() ) )
        {
            pEditEngine->GetInternalEditStatus().GetStatusWord() = pEditEngine->GetInternalEditStatus().GetStatusWord() | EditStatusFlags::CRSRLEFTPARA;
            pEditEngine->pImpEditEngine->CallStatusHdl();
        }
    }
    nTravelXPos = TRAVEL_X_DONTKNOW;
    nCursorBidiLevel = CURSOR_BIDILEVEL_DONTKNOW;
    nExtraCursorFlags = GetCursorFlags::NONE;
    bClickedInSelection = false;

    if ( rMouseEvent.IsLeft() && GetEditSelection().HasRange() )
    {
        Reference<css::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetPrimarySelection());
        CutCopy( aClipBoard, false );
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
    if ( pEditEngine->GetInternalEditStatus().NotifyCursorMovements() )
        pEditEngine->GetInternalEditStatus().GetPrevParagraph() = pEditEngine->GetEditDoc().GetPos( GetEditSelection().Max().GetNode() );
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

void ImpEditView::Command( const CommandEvent& rCEvt )
{
    pEditEngine->CheckIdleFormatter();  // If fast typing and mouse button down
    pEditEngine->pImpEditEngine->Command( rCEvt, GetEditViewPtr() );
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
            DrawSelection();
            SetEditSelection( aSel );
            DrawSelection();
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
            DrawSelection();
            SetEditSelection( EditSelection( aPaM, aPaM ) );
            DrawSelection();
        }

        if ( !aWord.isEmpty() )
        {
            Reference< XDictionary >  xDic( LinguMgr::GetIgnoreAllList(), UNO_QUERY );
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
    DrawSelection();

    pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_DELETE );

    EditPaM aPaM = pEditEngine->pImpEditEngine->DeleteSelected( GetEditSelection() );

    pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_DELETE );

    SetEditSelection( EditSelection( aPaM, aPaM ) );
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
        const EditCharAttrib& rAttr = *rAttrs[--nAttr].get();
        if (rAttr.GetStart() == nXPos)
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
        Rectangle aBulletArea = pEditEngine->GetBulletArea( nPara );
        long nY = pEditEngine->GetDocPosTopLeft( nPara ).Y();
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

void ImpEditView::CutCopy( css::uno::Reference< css::datatransfer::clipboard::XClipboard >& rxClipboard, bool bCut )
{
    if ( rxClipboard.is() && GetEditSelection().HasRange() )
    {
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
            pEditEngine->pImpEditEngine->EnterBlockNotifications();
            pEditEngine->pImpEditEngine->UndoActionStart(EDITUNDO_CUT);
            DeleteSelected();
            pEditEngine->pImpEditEngine->UndoActionEnd(EDITUNDO_CUT);
            pEditEngine->pImpEditEngine->LeaveBlockNotifications();
        }
    }
}

void ImpEditView::Paste( css::uno::Reference< css::datatransfer::clipboard::XClipboard >& rxClipboard, bool bUseSpecial )
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
        DrawSelection();
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
                aText = aText.replaceAll( OUStringLiteral1(LINE_SEP), " " );
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
        pEditEngine->pImpEditEngine->EnterBlockNotifications();
        aSel = pEditEngine->InsertText(
            xDataObj, OUString(), aSel.Min(),
            bUseSpecial && pEditEngine->GetInternalEditStatus().AllowPasteSpecial());
        pEditEngine->pImpEditEngine->LeaveBlockNotifications();
    }

    aPasteOrDropInfos.nEndPara = pEditEngine->GetEditDoc().GetPos( aSel.Max().GetNode() );
    pEditEngine->HandleEndPasteOrDrop(aPasteOrDropInfos);

    pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_PASTE );
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
    GetEditSelection().Min() = GetEditSelection().Max();
}

void ImpEditView::DeselectAll()
{
    pEditEngine->SetInSelectionMode(false);
    DrawSelection();
    GetEditSelection().Min() = GetEditSelection().Max();
}

bool ImpEditView::IsSelectionAtPoint( const Point& rPosPixel )
{
    if ( pDragAndDropInfo && pDragAndDropInfo->pField )
        return true;

    Point aMousePos( rPosPixel );

    // Logical units ...
    aMousePos = GetWindow()->PixelToLogic( aMousePos );

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
    aMousePos = GetWindow()->PixelToLogic( aMousePos );

    if ( ( !GetOutputArea().IsInside( aMousePos ) ) && !pEditEngine->pImpEditEngine->IsInSelectionMode() )
    {
        return false;
    }

    Point aDocPos( GetDocPos( aMousePos ) );

    // Can be optimized: first go through the lines within a paragraph for PAM,
    // then again with the PaM for the Rect, even though the line is already
    // known .... This must not be, though!
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
        DrawSelection( aTmpNewSel );
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
        GetWindow()->DrawOutDev( pDragAndDropInfo->aCurSavedCursor.TopLeft(), pDragAndDropInfo->aCurSavedCursor.GetSize(),
                            Point(0,0), pDragAndDropInfo->aCurSavedCursor.GetSize(),*pDragAndDropInfo->pBackground );
        pDragAndDropInfo->bVisCursor = false;
    }
}

void ImpEditView::ShowDDCursor( const Rectangle& rRect )
{
    if ( pDragAndDropInfo && !pDragAndDropInfo->bVisCursor )
    {
        if ( pOutWin->GetCursor() )
            pOutWin->GetCursor()->Hide();

        Color aOldFillColor = GetWindow()->GetFillColor();
        GetWindow()->SetFillColor( Color(4210752) );    // GRAY BRUSH_50, OLDSV, change to DDCursor!

        // Save background ...
        Rectangle aSaveRect( GetWindow()->LogicToPixel( rRect ) );
        // prefer to save some more ...
        aSaveRect.Right() += 1;
        aSaveRect.Bottom() += 1;

#ifdef DBG_UTIL
        Size aNewSzPx( aSaveRect.GetSize() );
#endif
        if ( !pDragAndDropInfo->pBackground )
        {
            pDragAndDropInfo->pBackground = VclPtr<VirtualDevice>::Create( *GetWindow() );
            MapMode aMapMode( GetWindow()->GetMapMode() );
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

        aSaveRect = GetWindow()->PixelToLogic( aSaveRect );

        pDragAndDropInfo->pBackground->DrawOutDev( Point(0,0), aSaveRect.GetSize(),
                                    aSaveRect.TopLeft(), aSaveRect.GetSize(), *GetWindow() );
        pDragAndDropInfo->aCurSavedCursor = aSaveRect;

        // Draw Cursor...
        GetWindow()->DrawRect( rRect );

        pDragAndDropInfo->bVisCursor = true;
        pDragAndDropInfo->aCurCursor = rRect;

        GetWindow()->SetFillColor( aOldFillColor );
    }
}

void ImpEditView::dragGestureRecognized(const css::datatransfer::dnd::DragGestureEvent& rDGE)
{
    DBG_ASSERT( !pDragAndDropInfo, "dragGestureRecognized - DragAndDropInfo exist!" );

    SolarMutexGuard aVclGuard;

    pDragAndDropInfo = nullptr;

    Point aMousePosPixel( rDGE.DragOriginX, rDGE.DragOriginY );

    EditSelection aCopySel( GetEditSelection() );
    aCopySel.Adjust( pEditEngine->GetEditDoc() );

    if ( GetEditSelection().HasRange() && bClickedInSelection )
    {
        pDragAndDropInfo = new DragAndDropInfo();
    }
    else
    {
        // Field?!
        sal_Int32 nPara;
        sal_Int32 nPos;
        Point aMousePos = GetWindow()->PixelToLogic( aMousePosPixel );
        const SvxFieldItem* pField = GetField( aMousePos, &nPara, &nPos );
        if ( pField )
        {
            pDragAndDropInfo = new DragAndDropInfo();
            pDragAndDropInfo->pField = pField;
            ContentNode* pNode = pEditEngine->GetEditDoc().GetObject( nPara );
            aCopySel = EditSelection( EditPaM( pNode, nPos ), EditPaM( pNode, nPos+1 ) );
            GetEditSelection() = aCopySel;
            DrawSelection();
            bool bGotoCursor = DoAutoScroll();
            ShowCursor( bGotoCursor, /*bForceCursor=*/false );
        }
        else if ( IsBulletArea( aMousePos, &nPara ) )
        {
            pDragAndDropInfo = new DragAndDropInfo();
            pDragAndDropInfo->bOutlinerMode = true;
            EditPaM aStartPaM( pEditEngine->GetEditDoc().GetObject( nPara ), 0 );
            EditPaM aEndPaM( aStartPaM );
            const SfxInt16Item& rLevel = static_cast<const SfxInt16Item&>(pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL ));
            for ( sal_Int32 n = nPara +1; n < pEditEngine->GetEditDoc().Count(); n++ )
            {
                const SfxInt16Item& rL = static_cast<const SfxInt16Item&>( pEditEngine->GetParaAttrib( n, EE_PARA_OUTLLEVEL ) );
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

    if ( pDragAndDropInfo )
    {

        pDragAndDropInfo->bStarterOfDD = true;

        // Sensitive area to be scrolled.
        Size aSz( 5, 0 );
        aSz = GetWindow()->PixelToLogic( aSz );
        pDragAndDropInfo->nSensibleRange = (sal_uInt16) aSz.Width();
        pDragAndDropInfo->nCursorWidth = (sal_uInt16) aSz.Width() / 2;
        pDragAndDropInfo->aBeginDragSel = pEditEngine->pImpEditEngine->CreateESel( aCopySel );

        uno::Reference<datatransfer::XTransferable> xData = pEditEngine->CreateTransferable(aCopySel);

        sal_Int8 nActions = bReadOnly ? datatransfer::dnd::DNDConstants::ACTION_COPY : datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE;

        rDGE.DragSource->startDrag( rDGE, nActions, 0 /*cursor*/, 0 /*image*/, xData, mxDnDListener );
        // If Drag&Move in an Engine, then Copy&Del has to be optional!
        GetCursor()->Hide();

    }
}

void ImpEditView::dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& rDSDE )
{
    SolarMutexGuard aVclGuard;

    DBG_ASSERT( pDragAndDropInfo, "ImpEditView::dragDropEnd: pDragAndDropInfo is NULL!" );

    // #123688# Shouldn't happen, but seems to happen...
    if ( pDragAndDropInfo )
    {
        if ( !bReadOnly && rDSDE.DropSuccess && !pDragAndDropInfo->bOutlinerMode && ( rDSDE.DropAction & datatransfer::dnd::DNDConstants::ACTION_MOVE ) )
        {
            if ( pDragAndDropInfo->bStarterOfDD && pDragAndDropInfo->bDroppedInMe )
            {
                // DropPos: Where was it dropped, irrespective of length.
                ESelection aDropPos( pDragAndDropInfo->aDropSel.nStartPara, pDragAndDropInfo->aDropSel.nStartPos, pDragAndDropInfo->aDropSel.nStartPara, pDragAndDropInfo->aDropSel.nStartPos );
                ESelection aToBeDelSel = pDragAndDropInfo->aBeginDragSel;
                ESelection aNewSel( pDragAndDropInfo->aDropSel.nEndPara, pDragAndDropInfo->aDropSel.nEndPos,
                                    pDragAndDropInfo->aDropSel.nEndPara, pDragAndDropInfo->aDropSel.nEndPos );
                bool bBeforeSelection = aDropPos.IsLess( pDragAndDropInfo->aBeginDragSel );
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

                DrawSelection();
                EditSelection aDelSel( pEditEngine->pImpEditEngine->CreateSel( aToBeDelSel ) );
                DBG_ASSERT( !aDelSel.DbgIsBuggy( pEditEngine->GetEditDoc() ), "ToBeDel is buggy!" );
                pEditEngine->DeleteSelection(aDelSel);
                if ( !bBeforeSelection )
                {
                    DBG_ASSERT( !pEditEngine->pImpEditEngine->CreateSel( aNewSel ).DbgIsBuggy(pEditEngine->GetEditDoc()), "Bad" );
                    SetEditSelection( pEditEngine->pImpEditEngine->CreateSel( aNewSel ) );
                }
                pEditEngine->pImpEditEngine->FormatAndUpdate( pEditEngine->pImpEditEngine->GetActiveView() );
                DrawSelection();
            }
            else
            {
                // other EditEngine ...
                if (pEditEngine->HasText())   // #88630# SC is removing the content when switching the task
                    DeleteSelected();
            }
        }

        if ( pDragAndDropInfo->bUndoAction )
            pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_DRAGANDDROP );

        HideDDCursor();
        ShowCursor( DoAutoScroll(), true );
        delete pDragAndDropInfo;
        pDragAndDropInfo = nullptr;
        pEditEngine->GetEndDropHdl().Call(GetEditViewPtr());
    }
}

void ImpEditView::drop( const css::datatransfer::dnd::DropTargetDropEvent& rDTDE )
{
    SolarMutexGuard aVclGuard;

    DBG_ASSERT( pDragAndDropInfo, "Drop - No Drag&Drop info?!" );

    if ( pDragAndDropInfo && pDragAndDropInfo->bDragAccepted )
    {
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
                DrawSelection();
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
            delete pDragAndDropInfo;
            pDragAndDropInfo = nullptr;
        }

        rDTDE.Context->dropComplete( bChanges );
    }
}

void ImpEditView::dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& rDTDEE )
{
    SolarMutexGuard aVclGuard;

    if ( !pDragAndDropInfo )
        pDragAndDropInfo = new DragAndDropInfo( );

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
        delete pDragAndDropInfo;
        pDragAndDropInfo = nullptr;
    }
}

void ImpEditView::dragOver(const css::datatransfer::dnd::DropTargetDragEvent& rDTDE)
{
    SolarMutexGuard aVclGuard;

    Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );
    aMousePos = GetWindow()->PixelToLogic( aMousePos );

    bool bAccept = false;

    if ( GetOutputArea().IsInside( aMousePos ) && !bReadOnly )
    {
        if ( pDragAndDropInfo && pDragAndDropInfo->bHasValidData )
        {
            bAccept = true;

            bool bAllowScroll = DoAutoScroll();
            if ( bAllowScroll )
            {
                long nScrollX = 0;
                long nScrollY = 0;
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
                long nDestParaStartY = pEditEngine->GetParaPortions().GetYOffset( pPPortion );
                long nRel = aDocPos.Y() - nDestParaStartY;
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
            else if ( HasSelection() )
            {
                // it must not be dropped into a selection
                EPaM aP = pEditEngine->pImpEditEngine->CreateEPaM( aPaM );
                ESelection aDestSel( aP.nPara, aP.nIndex, aP.nPara, aP.nIndex);
                ESelection aCurSel = pEditEngine->pImpEditEngine->CreateESel( GetEditSelection() );
                aCurSel.Adjust();
                if ( !aDestSel.IsLess( aCurSel ) && !aDestSel.IsGreater( aCurSel ) )
                {
                    bAccept = false;
                }
            }
            if ( bAccept )
            {
                Rectangle aEditCursor;
                if ( pDragAndDropInfo->bOutlinerMode )
                {
                    long nDDYPos;
                    if ( pDragAndDropInfo->nOutlinerDropDest < pEditEngine->GetEditDoc().Count() )
                    {
                        ParaPortion* pPPortion = pEditEngine->GetParaPortions().SafeGetObject( pDragAndDropInfo->nOutlinerDropDest );
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
                    aEditCursor = GetWindow()->LogicToPixel( Rectangle( aStartPos, aEndPos ) );
                    if ( !pEditEngine->IsVertical() )
                    {
                        aEditCursor.Top()--;
                        aEditCursor.Bottom()++;
                    }
                    else
                    {
                        aEditCursor.Left()--;
                        aEditCursor.Right()++;
                    }
                    aEditCursor = GetWindow()->PixelToLogic( aEditCursor );
                }
                else
                {
                    aEditCursor = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM );
                    Point aTopLeft( GetWindowPos( aEditCursor.TopLeft() ) );
                    aEditCursor.SetPos( aTopLeft );
                    aEditCursor.Right() = aEditCursor.Left() + pDragAndDropInfo->nCursorWidth;
                    aEditCursor = GetWindow()->LogicToPixel( aEditCursor );
                    aEditCursor = GetWindow()->PixelToLogic( aEditCursor );
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
    vcl::Window* pWindow = GetWindow();
    if ( !bActiveDragAndDropListener && pWindow && pWindow->GetDragGestureRecognizer().is() )
    {
        vcl::unohelper::DragAndDropWrapper* pDnDWrapper = new vcl::unohelper::DragAndDropWrapper( this );
        mxDnDListener = pDnDWrapper;

        uno::Reference< datatransfer::dnd::XDragGestureListener> xDGL( mxDnDListener, uno::UNO_QUERY );
        pWindow->GetDragGestureRecognizer()->addDragGestureListener( xDGL );
        uno::Reference< datatransfer::dnd::XDropTargetListener> xDTL( xDGL, uno::UNO_QUERY );
        pWindow->GetDropTarget()->addDropTargetListener( xDTL );
        pWindow->GetDropTarget()->setActive( true );
        pWindow->GetDropTarget()->setDefaultActions( datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE );

        bActiveDragAndDropListener = true;
    }
}

void ImpEditView::RemoveDragAndDropListeners()
{
    if ( bActiveDragAndDropListener && GetWindow() && GetWindow()->GetDragGestureRecognizer().is() )
    {
        uno::Reference< datatransfer::dnd::XDragGestureListener> xDGL( mxDnDListener, uno::UNO_QUERY );
        GetWindow()->GetDragGestureRecognizer()->removeDragGestureListener( xDGL );
        uno::Reference< datatransfer::dnd::XDropTargetListener> xDTL( xDGL, uno::UNO_QUERY );
        GetWindow()->GetDropTarget()->removeDropTargetListener( xDTL );

        if ( mxDnDListener.is() )
        {
            uno::Reference< lang::XEventListener> xEL( mxDnDListener, uno::UNO_QUERY );
            xEL->disposing( lang::EventObject() );  // #95154# Empty Source means it's the Client
            mxDnDListener.clear();
        }

        bActiveDragAndDropListener = false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
