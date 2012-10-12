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

#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <impedit.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
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

// ----------------------------------------------------------------------
//  class ImpEditView
//  ----------------------------------------------------------------------
ImpEditView::ImpEditView( EditView* pView, EditEngine* pEng, Window* pWindow ) :
    aOutArea( Point(), pEng->GetPaperSize() )
{
    pEditView           = pView;
    pEditEngine         = pEng;
    pOutWin             = pWindow;
    pPointer            = NULL;
    pBackgroundColor    = NULL;
    nScrollDiffX        = 0;
    nExtraCursorFlags   = 0;
    nCursorBidiLevel    = CURSOR_BIDILEVEL_DONTKNOW;
    pCursor             = NULL;
       pDragAndDropInfo = NULL;
    bReadOnly           = sal_False;
    bClickedInSelection = sal_False;
    eSelectionMode      = EE_SELMODE_TXTONLY;
    eAnchorMode         = ANCHOR_TOP_LEFT;
    nInvMore            = 1;
    nTravelXPos         = TRAVEL_X_DONTKNOW;
    nControl            = EV_CNTRL_AUTOSCROLL | EV_CNTRL_ENABLEPASTE;
    bActiveDragAndDropListener = sal_False;

    aEditSelection.Min() = pEng->GetEditDoc().GetStartPaM();
    aEditSelection.Max() = pEng->GetEditDoc().GetEndPaM();
}

ImpEditView::~ImpEditView()
{
    RemoveDragAndDropListeners();

    if ( pOutWin && ( pOutWin->GetCursor() == pCursor ) )
        pOutWin->SetCursor( NULL );

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

void ImpEditView::SetEditSelection( const EditSelection& rEditSelection )
{
    // set state before notification
    aEditSelection = rEditSelection;

    if ( pEditEngine->pImpEditEngine->GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_TEXTVIEWSELECTIONCHANGED );
        aNotify.pEditEngine = pEditEngine;
        aNotify.pEditView = GetEditViewPtr();
        pEditEngine->pImpEditEngine->CallNotify( aNotify );
    }
}


void ImpEditView::DrawSelection( EditSelection aTmpSel, Region* pRegion )
{
    if ( GetSelectionMode() == EE_SELMODE_HIDDEN )
        return;

    // It must be ensured before rendering the selection, that the contents of
    // the window is completely valid! Must be here so that in any case if
    // empty, then later on two-Paint Events! Must be done even before the
    // query from bUpdate, if after Invalidate paints still in the queue,
    // but someone switches the update mode!

    // pRegion: When not NULL, then only calculate Region.
    PolyPolygon* pPolyPoly = NULL;
    if ( pRegion )
        pPolyPoly = new PolyPolygon;

    sal_Bool bClipRegion = pOutWin->IsClipRegion();
    Region aOldRegion = pOutWin->GetClipRegion();

    if ( !pRegion )
    {
        if ( pEditEngine->pImpEditEngine->GetUpdateMode() == sal_False )
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
        pOutWin->IntersectClipRegion( aTmpOutArea );

        if ( pOutWin->GetCursor() )
            pOutWin->GetCursor()->Hide();
    }

    DBG_ASSERT( !pEditEngine->IsIdleFormatterActive(), "DrawSelection: Not formatted!" );
    aTmpSel.Adjust( pEditEngine->GetEditDoc() );

    ContentNode* pStartNode = aTmpSel.Min().GetNode();
    ContentNode* pEndNode = aTmpSel.Max().GetNode();
    sal_uInt16 nStartPara = pEditEngine->GetEditDoc().GetPos( pStartNode );
    sal_uInt16 nEndPara = pEditEngine->GetEditDoc().GetPos( pEndNode );
    for ( sal_uInt16 nPara = nStartPara; nPara <= nEndPara; nPara++ )
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
            nStartLine = pTmpPortion->GetLines().FindLine( aTmpSel.Min().GetIndex(), sal_False );
        if ( nPara == nEndPara )
            nEndLine = pTmpPortion->GetLines().FindLine( aTmpSel.Max().GetIndex(), sal_True );

        for ( sal_uInt16 nLine = nStartLine; nLine <= nEndLine; nLine++ )
        {
            const EditLine* pLine = pTmpPortion->GetLines()[nLine];
            DBG_ASSERT( pLine, "Line not found: DrawSelection()" );

            sal_Bool bPartOfLine = sal_False;
            sal_uInt16 nStartIndex = pLine->GetStart();
            sal_uInt16 nEndIndex = pLine->GetEnd();
            if ( ( nPara == nStartPara ) && ( nLine == nStartLine ) && ( nStartIndex != aTmpSel.Min().GetIndex() ) )
            {
                nStartIndex = aTmpSel.Min().GetIndex();
                bPartOfLine = sal_True;
            }
            if ( ( nPara == nEndPara ) && ( nLine == nEndLine ) && ( nEndIndex != aTmpSel.Max().GetIndex() ) )
            {
                nEndIndex = aTmpSel.Max().GetIndex();
                bPartOfLine = sal_True;
            }

            // Can happen if at the beginning of a wrapped line.
            if ( nEndIndex < nStartIndex )
                nEndIndex = nStartIndex;

            Rectangle aTmpRec( pEditEngine->pImpEditEngine->GetEditCursor( pTmpPortion, nStartIndex ) );
            Point aTopLeft( aTmpRec.TopLeft() );
            Point aBottomRight( aTmpRec.BottomRight() );

            aTopLeft.Y() += nParaStart;
            aBottomRight.Y() += nParaStart;

            // Only paint if in the visible range ...
            if ( aTopLeft.Y() > GetVisDocBottom() )
                break;

            if ( aBottomRight.Y() < GetVisDocTop() )
                continue;

            // Now that we have Bidi, the first/last index doesn't have to be the 'most outside' postion
            if ( !bPartOfLine )
            {
                Range aLineXPosStartEnd = pEditEngine->GetLineXPosStartEnd(pTmpPortion, pLine);
                aTopLeft.X() = aLineXPosStartEnd.Min();
                aBottomRight.X() = aLineXPosStartEnd.Max();
                ImplDrawHighlightRect( pOutWin, aTopLeft, aBottomRight, pPolyPoly );
            }
            else
            {
                sal_uInt16 nTmpStartIndex = nStartIndex;
                sal_uInt16 nWritingDirStart, nTmpEndIndex;

                while ( nTmpStartIndex < nEndIndex )
                {
                    pEditEngine->pImpEditEngine->GetRightToLeft( nPara, nTmpStartIndex+1, &nWritingDirStart, &nTmpEndIndex );
                    if ( nTmpEndIndex > nEndIndex )
                        nTmpEndIndex = nEndIndex;

                    DBG_ASSERT( nTmpEndIndex > nTmpStartIndex, "DrawSelection, Start >= End?" );

                    long nX1 = pEditEngine->GetXPos(pTmpPortion, pLine, nTmpStartIndex, true);
                    long nX2 = pEditEngine->GetXPos(pTmpPortion, pLine, nTmpEndIndex);

                    Point aPt1( Min( nX1, nX2 ), aTopLeft.Y() );
                    Point aPt2( Max( nX1, nX2 ), aBottomRight.Y() );

                    ImplDrawHighlightRect( pOutWin, aPt1, aPt2, pPolyPoly );

                    nTmpStartIndex = nTmpEndIndex;
                }
            }

        }
    }

    if ( pRegion )
    {
        *pRegion = Region( *pPolyPoly );
        delete pPolyPoly;
    }
    else
    {
        if ( pOutWin->GetCursor() )
            pOutWin->GetCursor()->Show();

        if ( bClipRegion )
            pOutWin->SetClipRegion( aOldRegion );
        else
            pOutWin->SetClipRegion();
    }
}

void ImpEditView::ImplDrawHighlightRect( Window* _pOutWin, const Point& rDocPosTopLeft, const Point& rDocPosBottomRight, PolyPolygon* pPolyPoly )
{
    if ( rDocPosTopLeft.X() != rDocPosBottomRight.X() )
    {
        sal_Bool bPixelMode = _pOutWin->GetMapMode() == MAP_PIXEL;

        Point aPnt1( GetWindowPos( rDocPosTopLeft ) );
        Point aPnt2( GetWindowPos( rDocPosBottomRight ) );

        if ( !IsVertical() )
        {
            lcl_AllignToPixel( aPnt1, _pOutWin, +1, 0 );
            lcl_AllignToPixel( aPnt2, _pOutWin, 0, ( bPixelMode ? 0 : -1 ) );
        }
        else
        {
            lcl_AllignToPixel( aPnt1, _pOutWin, 0, +1 );
            lcl_AllignToPixel( aPnt2, _pOutWin, ( bPixelMode ? 0 : +1 ), 0 );
        }

        Rectangle aRect( aPnt1, aPnt2 );
        if ( pPolyPoly )
        {
            Polygon aTmpPoly( 4 );
            aTmpPoly[0] = aRect.TopLeft();
            aTmpPoly[1] = aRect.TopRight();
            aTmpPoly[2] = aRect.BottomRight();
            aTmpPoly[3] = aRect.BottomLeft();
            pPolyPoly->Insert( aTmpPoly );
        }
        else
        {
            _pOutWin->Invert( aRect );
        }
    }
}


sal_Bool ImpEditView::IsVertical() const
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

void ImpEditView::SetOutputArea( const Rectangle& rRec )
{
    // should be better be aligned on pixels!
    Rectangle aNewRec( pOutWin->LogicToPixel( rRec ) );
    aNewRec = pOutWin->PixelToLogic( aNewRec );
    aOutArea = aNewRec;
    if ( aOutArea.Right() < aOutArea.Left() )
        aOutArea.Right() = aOutArea.Left();
    if ( aOutArea.Bottom() < aOutArea.Top() )
        aOutArea.Bottom() = aOutArea.Top();

    if ( DoBigScroll() )
        SetScrollDiffX( (sal_uInt16)aOutArea.GetWidth() * 3 / 10 );
    else
        SetScrollDiffX( (sal_uInt16)aOutArea.GetWidth() * 2 / 10 );
}

void ImpEditView::ResetOutputArea( const Rectangle& rRec )
{
    Rectangle aCurArea( aOutArea );
    SetOutputArea( rRec );
    // Invalidate surrounding areas if in update mode of the engine on sal_True
    if ( !aCurArea.IsEmpty() && pEditEngine->pImpEditEngine->GetUpdateMode() )
    {
        long nMore = 0;
        if ( DoInvalidateMore() )
            nMore = GetWindow()->PixelToLogic( Size( nInvMore, 0 ) ).Width();
        if ( aCurArea.Left() < aOutArea.Left() )
        {
            Rectangle aRect( aCurArea.TopLeft(),
                Size( aOutArea.Left()-aCurArea.Left(), aCurArea.GetHeight() ) );
            if ( nMore )
            {
                aRect.Left() -= nMore;
                aRect.Top() -= nMore;
                aRect.Bottom() += nMore;
            }
            GetWindow()->Invalidate( aRect );
        }
        if ( aCurArea.Right() > aOutArea.Right() )
        {
            long nW = aCurArea.Right() - aOutArea.Right();
            Point aPos( aCurArea.TopRight() );
            aPos.X() -= nW;
            Rectangle aRect( aPos, Size( nW, aCurArea.GetHeight() ) );
            if ( nMore )
            {
                aRect.Right() += nMore;
                aRect.Top() -= nMore;
                aRect.Bottom() += nMore;
            }
            GetWindow()->Invalidate( aRect );
        }
        if ( aCurArea.Top() < aOutArea.Top() )
        {
            Rectangle aRect( aCurArea.TopLeft(), Size( aCurArea.GetWidth(), aOutArea.Top() - aCurArea.Top() ) );
            if ( nMore )
            {
                aRect.Top() -= nMore;
                aRect.Left() -= nMore;
                aRect.Right() += nMore;
            }
            GetWindow()->Invalidate( aRect );
        }
        if ( aCurArea.Bottom() > aOutArea.Bottom() )
        {
            long nH = aCurArea.Bottom() - aOutArea.Bottom();
            Point aPos( aCurArea.BottomLeft() );
            aPos.Y() -= nH;
            Rectangle aRect( aPos, Size( aCurArea.GetWidth(), nH ) );
            if ( nMore )
            {
                aRect.Bottom() += nMore;
                aRect.Left() -= nMore;
                aRect.Right() += nMore;
            }

            GetWindow()->Invalidate( aRect );
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
    // GetHeight() and GetWidth() -1, because rectangle calculation not prefered.

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

void ImpEditView::ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor, sal_uInt16 nShowCursorFlags )
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
    if ( pEditEngine->pImpEditEngine->GetUpdateMode() == sal_False )
        return;
    if ( pEditEngine->pImpEditEngine->IsInUndo() )
        return;

    if ( pOutWin->GetCursor() != GetCursor() )
        pOutWin->SetCursor( GetCursor() );

    EditPaM aPaM( aEditSelection.Max() );

    sal_uInt16 nTextPortionStart = 0;
    sal_uInt16 nPara = pEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
    if (nPara == USHRT_MAX) // #i94322
        return;

    const ParaPortion* pParaPortion = pEditEngine->GetParaPortions()[nPara];

    nShowCursorFlags |= nExtraCursorFlags;

    nShowCursorFlags |= GETCRSR_TXTONLY;

    // Use CursorBidiLevel 0/1 in meaning of
    // 0: prefer portion end, normal mode
    // 1: prefer portion start

    if ( ( GetCursorBidiLevel() != CURSOR_BIDILEVEL_DONTKNOW ) && GetCursorBidiLevel() )
    {
        nShowCursorFlags |= GETCRSR_PREFERPORTIONSTART;
    }

    Rectangle aEditCursor = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM, nShowCursorFlags );
    if ( !IsInsertMode() && !aEditSelection.HasRange() )
    {
        if ( aPaM.GetNode()->Len() && ( aPaM.GetIndex() < aPaM.GetNode()->Len() ) )
        {
            // If we are behind a portion, and the next portion has other direction, we must change position...
            aEditCursor.Left() = aEditCursor.Right() = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM, GETCRSR_TXTONLY|GETCRSR_PREFERPORTIONSTART ).Left();

            sal_uInt16 nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, sal_True );
            const TextPortion* pTextPortion = pParaPortion->GetTextPortions()[nTextPortion];
            if ( pTextPortion->GetKind() == PORTIONKIND_TAB )
            {
                aEditCursor.Right() += pTextPortion->GetSize().Width();
            }
            else
            {
                EditPaM aNext = pEditEngine->CursorRight( aPaM, (sal_uInt16)i18n::CharacterIteratorMode::SKIPCELL );
                Rectangle aTmpRect = pEditEngine->pImpEditEngine->PaMtoEditCursor( aNext, GETCRSR_TXTONLY );
                if ( aTmpRect.Top() != aEditCursor.Top() )
                    aTmpRect = pEditEngine->pImpEditEngine->PaMtoEditCursor( aNext, GETCRSR_TXTONLY|GETCRSR_ENDOFLINE );
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
            if ( ( Abs( nDiffY ) > pEditEngine->GetOnePixelInRef() ) && DoBigScroll() )
            {
                long nH = aOutArea.GetHeight() / 4;
                if ( ( nH > aEditCursor.GetHeight() ) && ( Abs( nDiffY ) < nH ) )
                {
                    if ( nDiffY < 0 )
                        nDiffY -= nH;
                    else
                        nDiffY += nH;
                }
            }

            if ( ( Abs( nDiffX ) > pEditEngine->GetOnePixelInRef() ) && DoBigScroll() )
            {
                long nW = aOutArea.GetWidth() / 4;
                if ( Abs( nDiffX ) < nW )
                {
                    if ( nDiffY < 0 )
                        nDiffY -= nW;
                    else
                        nDiffY += nW;
                }
            }

            if ( nDiffX )
                pEditEngine->GetInternalEditStatus().GetStatusWord() = pEditEngine->GetInternalEditStatus().GetStatusWord() | EE_STAT_HSCROLL;
            if ( nDiffY )
                pEditEngine->GetInternalEditStatus().GetStatusWord() = pEditEngine->GetInternalEditStatus().GetStatusWord() | EE_STAT_VSCROLL;
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
            GetCursor()->SetOrientation( 0 );

        GetCursor()->SetSize( aCursorSz );

        unsigned char nCursorDir = CURSOR_DIRECTION_NONE;
        if ( IsInsertMode() && !aEditSelection.HasRange() && ( pEditEngine->pImpEditEngine->HasDifferentRTLLevels( aPaM.GetNode() ) ) )
        {
            sal_uInt16 nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, nShowCursorFlags & GETCRSR_PREFERPORTIONSTART ? sal_True : sal_False );
            const TextPortion* pTextPortion = pParaPortion->GetTextPortions()[nTextPortion];
            sal_uInt16 nRTLLevel = pTextPortion->GetRightToLeft();
            if ( nRTLLevel%2 )
                nCursorDir = CURSOR_DIRECTION_RTL;
            else
                nCursorDir = CURSOR_DIRECTION_LTR;

        }
        GetCursor()->SetDirection( nCursorDir );

        if ( bForceVisCursor )
            GetCursor()->Show();
        {
            SvxFont aFont;
            pEditEngine->SeekCursor( aPaM.GetNode(), aPaM.GetIndex()+1, aFont );
            sal_uLong nContextFlags = INPUTCONTEXT_TEXT|INPUTCONTEXT_EXTTEXTINPUT;
            GetWindow()->SetInputContext( InputContext( aFont, nContextFlags ) );
        }
    }
    else
    {
        pEditEngine->pImpEditEngine->GetStatus().GetStatusWord() = pEditEngine->pImpEditEngine->GetStatus().GetStatusWord() | EE_STAT_CURSOROUT;
        GetCursor()->Hide();
        GetCursor()->SetPos( Point( -1, -1 ) );
        GetCursor()->SetSize( Size( 0, 0 ) );
    }
}

Pair ImpEditView::Scroll( long ndX, long ndY, sal_uInt8 nRangeCheck )
{
    DBG_ASSERT( pEditEngine->pImpEditEngine->IsFormatted(), "Scroll: Not formatted!" );
    if ( !ndX && !ndY )
        return Range( 0, 0 );

#ifdef DBG_UTIL
    Rectangle aR( aOutArea );
    aR = pOutWin->LogicToPixel( aR );
    aR = pOutWin->PixelToLogic( aR );
    DBG_ASSERTWARNING( aR == aOutArea, "OutArea before Scroll not aligned" );
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
    if ( ( nRangeCheck == RGCHK_PAPERSZ1 ) && ( aNewVisArea.Bottom() > (long)pEditEngine->pImpEditEngine->GetTextHeight() ) )
    {
        // GetTextHeight still optimizing!
        long nDiff = pEditEngine->pImpEditEngine->GetTextHeight() - aNewVisArea.Bottom(); // negative
        aNewVisArea.Move( 0, nDiff );   // could end up in the negative area...
    }
    if ( ( aNewVisArea.Top() < 0 ) && ( nRangeCheck != RGCHK_NONE ) )
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
    if ( ( nRangeCheck == RGCHK_PAPERSZ1 ) && ( aNewVisArea.Right() > (long)pEditEngine->pImpEditEngine->CalcTextWidth( sal_False ) ) )
    {
        long nDiff = pEditEngine->pImpEditEngine->CalcTextWidth( sal_False ) - aNewVisArea.Right();     // negative
        aNewVisArea.Move( nDiff, 0 );   // could end up in the negative area...
    }
    if ( ( aNewVisArea.Left() < 0 ) && ( nRangeCheck != RGCHK_NONE ) )
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
        Cursor* pCrsr = GetCursor();
        sal_Bool bVisCursor = pCrsr->IsVisible();
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
        Rectangle aRec( aOutArea );
        pOutWin->Scroll( nRealDiffX, nRealDiffY, aRec, sal_True );
        pOutWin->Update();
        pCrsr->SetPos( pCrsr->GetPos() + Point( nRealDiffX, nRealDiffY ) );
        if ( bVisCursor )
        {
            Rectangle aCursorRec( pCrsr->GetPos(), pCrsr->GetSize() );
            if ( aOutArea.IsInside( aCursorRec ) )
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

sal_Bool ImpEditView::PostKeyEvent( const KeyEvent& rKeyEvent, Window* pFrameWin )
{
    sal_Bool bDone = sal_False;

    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_CUT:
            {
                if ( !bReadOnly )
                {
                    Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
                    CutCopy( aClipBoard, sal_True );
                    bDone = sal_True;
                }
            }
            break;
            case KEYFUNC_COPY:
            {
                Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
                CutCopy( aClipBoard, sal_False );
                bDone = sal_True;
            }
            break;
            case KEYFUNC_PASTE:
            {
                if ( !bReadOnly && IsPasteEnabled() )
                {
                    pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_PASTE );
                    Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
                    Paste( aClipBoard, pEditEngine->pImpEditEngine->GetStatus().AllowPasteSpecial() );
                    pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_PASTE );
                    bDone = sal_True;
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

sal_Bool ImpEditView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    if ( pEditEngine->GetInternalEditStatus().NotifyCursorMovements() )
    {
        if ( pEditEngine->GetInternalEditStatus().GetPrevParagraph() != pEditEngine->GetEditDoc().GetPos( GetEditSelection().Max().GetNode() ) )
        {
            pEditEngine->GetInternalEditStatus().GetStatusWord() = pEditEngine->GetInternalEditStatus().GetStatusWord() | EE_STAT_CRSRLEFTPARA;
            pEditEngine->pImpEditEngine->CallStatusHdl();
        }
    }
    nTravelXPos = TRAVEL_X_DONTKNOW;
    nCursorBidiLevel = CURSOR_BIDILEVEL_DONTKNOW;
    nExtraCursorFlags = 0;
    bClickedInSelection = sal_False;

    if ( rMouseEvent.IsMiddle() && !bReadOnly &&
         ( GetWindow()->GetSettings().GetMouseSettings().GetMiddleButtonAction() == MOUSE_MIDDLE_PASTESELECTION ) )
    {
        Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetPrimarySelection());
        Paste( aClipBoard );
    }
    else if ( rMouseEvent.IsLeft() && GetEditSelection().HasRange() )
    {
        Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetPrimarySelection());
        CutCopy( aClipBoard, sal_False );
    }

    return pEditEngine->pImpEditEngine->MouseButtonUp( rMouseEvent, GetEditViewPtr() );
}

sal_Bool ImpEditView::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    pEditEngine->CheckIdleFormatter();  // If fast typing and mouse button downs
    if ( pEditEngine->GetInternalEditStatus().NotifyCursorMovements() )
        pEditEngine->GetInternalEditStatus().GetPrevParagraph() = pEditEngine->GetEditDoc().GetPos( GetEditSelection().Max().GetNode() );
    nTravelXPos = TRAVEL_X_DONTKNOW;
    nExtraCursorFlags = 0;
    nCursorBidiLevel    = CURSOR_BIDILEVEL_DONTKNOW;
    bClickedInSelection = IsSelectionAtPoint( rMouseEvent.GetPosPixel() );
    return pEditEngine->pImpEditEngine->MouseButtonDown( rMouseEvent, GetEditViewPtr() );
}

sal_Bool ImpEditView::MouseMove( const MouseEvent& rMouseEvent )
{
    return pEditEngine->pImpEditEngine->MouseMove( rMouseEvent, GetEditViewPtr() );
}

void ImpEditView::Command( const CommandEvent& rCEvt )
{
    pEditEngine->CheckIdleFormatter();  // If fast typing and mouse button down
    pEditEngine->pImpEditEngine->Command( rCEvt, GetEditViewPtr() );
}


void ImpEditView::SetInsertMode( sal_Bool bInsert )
{
    if ( bInsert != IsInsertMode() )
    {
        SetFlags( nControl, EV_CNTRL_OVERWRITE, !bInsert );
        ShowCursor( DoAutoScroll(), sal_False );
    }
}

sal_Bool ImpEditView::IsWrongSpelledWord( const EditPaM& rPaM, sal_Bool bMarkIfWrong )
{
    bool bIsWrong = false;
    if ( rPaM.GetNode()->GetWrongList() )
    {
        EditSelection aSel = pEditEngine->SelectWord( rPaM, ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
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

String ImpEditView::SpellIgnoreOrAddWord( sal_Bool bAdd )
{
    String aWord;
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

        if ( aWord.Len() )
        {
            if ( bAdd )
            {
                OSL_FAIL( "Sorry, AddWord not implemented" );
            }
            else // Ignore
            {
                Reference< XDictionary >  xDic( SvxGetIgnoreAllList(), UNO_QUERY );
                if (xDic.is())
                    xDic->add( aWord, sal_False, String() );
            }
            EditDoc& rDoc = pEditEngine->GetEditDoc();
            sal_uInt16 nNodes = rDoc.Count();
            for ( sal_uInt16 n = 0; n < nNodes; n++ )
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
    ShowCursor( DoAutoScroll(), sal_True );
}

const SvxFieldItem* ImpEditView::GetField( const Point& rPos, sal_uInt16* pPara, sal_uInt16* pPos ) const
{
    if( !GetOutputArea().IsInside( rPos ) )
        return 0;

    Point aDocPos( GetDocPos( rPos ) );
    EditPaM aPaM = pEditEngine->GetPaM(aDocPos, false);

    if ( aPaM.GetIndex() == aPaM.GetNode()->Len() )
    {
        // Otherwise, whenever the Field at the very end and mouse under the text
        return 0;
    }

    const CharAttribList::AttribsType& rAttrs = aPaM.GetNode()->GetCharAttribs().GetAttribs();
    sal_uInt16 nXPos = aPaM.GetIndex();
    for (size_t nAttr = rAttrs.size(); nAttr; )
    {
        const EditCharAttrib& rAttr = rAttrs[--nAttr];
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
    return NULL;
}

sal_Bool ImpEditView::IsBulletArea( const Point& rPos, sal_uInt16* pPara )
{
    if ( pPara )
        *pPara = 0xFFFF;

    if( !GetOutputArea().IsInside( rPos ) )
        return sal_False;

    Point aDocPos( GetDocPos( rPos ) );
    EditPaM aPaM = pEditEngine->GetPaM(aDocPos, false);

    if ( aPaM.GetIndex() == 0 )
    {
        sal_uInt16 nPara = pEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
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
            return sal_True;
        }
    }

    return sal_False;
}

void ImpEditView::CutCopy( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard, sal_Bool bCut )
{
    if ( rxClipboard.is() && GetEditSelection().HasRange() )
    {
        uno::Reference<datatransfer::XTransferable> xData = pEditEngine->CreateTransferable( GetEditSelection() );

        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

        try
        {
            rxClipboard->setContents( xData, NULL );

            // #87756# FlushClipboard, but it would be better to become a TerminateListener to the Desktop and flush on demand...
            uno::Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( rxClipboard, uno::UNO_QUERY );
            if( xFlushableClipboard.is() )
                xFlushableClipboard->flushClipboard();
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        Application::AcquireSolarMutex( nRef );

        if ( bCut )
        {
            pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_CUT );
            DeleteSelected();
            pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_CUT );

        }
    }
}

void ImpEditView::Paste( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard, sal_Bool bUseSpecial )
{
    if ( rxClipboard.is() )
    {
        uno::Reference< datatransfer::XTransferable > xDataObj;

        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

        try
        {
            xDataObj = rxClipboard->getContents();
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        Application::AcquireSolarMutex( nRef );

        if ( xDataObj.is() && EditEngine::HasValidData( xDataObj ) )
        {
            pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_PASTE );

            EditSelection aSel( GetEditSelection() );
            if ( aSel.HasRange() )
            {
                DrawSelection();
                aSel = pEditEngine->DeleteSelection(aSel);
            }

            PasteOrDropInfos aPasteOrDropInfos;
            aPasteOrDropInfos.nAction = EE_ACTION_PASTE;
            aPasteOrDropInfos.nStartPara = pEditEngine->GetEditDoc().GetPos( aSel.Min().GetNode() );
            pEditEngine->HandleBeginPasteOrDrop(aPasteOrDropInfos);

            if ( DoSingleLinePaste() )
            {
                datatransfer::DataFlavor aFlavor;
                SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
                if ( xDataObj->isDataFlavorSupported( aFlavor ) )
                {
                    try
                    {
                        uno::Any aData = xDataObj->getTransferData( aFlavor );
                        ::rtl::OUString aTmpText;
                        aData >>= aTmpText;
                        String aText(convertLineEnd(aTmpText, LINEEND_LF));
                        aText.SearchAndReplaceAll( LINE_SEP, ' ' );
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
                aSel = pEditEngine->InsertText(
                    xDataObj, String(), aSel.Min(),
                    bUseSpecial && pEditEngine->GetInternalEditStatus().AllowPasteSpecial());
            }

            aPasteOrDropInfos.nEndPara = pEditEngine->GetEditDoc().GetPos( aSel.Max().GetNode() );
            pEditEngine->HandleEndPasteOrDrop(aPasteOrDropInfos);

            pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_PASTE );
            SetEditSelection( aSel );
            pEditEngine->pImpEditEngine->UpdateSelections();
            pEditEngine->pImpEditEngine->FormatAndUpdate( GetEditViewPtr() );
            ShowCursor( DoAutoScroll(), sal_True );
        }
    }
}


sal_Bool ImpEditView::IsInSelection( const EditPaM& rPaM )
{
    EditSelection aSel = GetEditSelection();
    if ( !aSel.HasRange() )
        return sal_False;

    aSel.Adjust( pEditEngine->GetEditDoc() );

    sal_uInt16 nStartNode = pEditEngine->GetEditDoc().GetPos( aSel.Min().GetNode() );
    sal_uInt16 nEndNode = pEditEngine->GetEditDoc().GetPos( aSel.Max().GetNode() );
    sal_uInt16 nCurNode = pEditEngine->GetEditDoc().GetPos( rPaM.GetNode() );

    if ( ( nCurNode > nStartNode ) && ( nCurNode < nEndNode ) )
        return sal_True;

    if ( nStartNode == nEndNode )
    {
        if ( nCurNode == nStartNode )
            if ( ( rPaM.GetIndex() >= aSel.Min().GetIndex() ) && ( rPaM.GetIndex() < aSel.Max().GetIndex() ) )
                return sal_True;
    }
    else if ( ( nCurNode == nStartNode ) && ( rPaM.GetIndex() >= aSel.Min().GetIndex() ) )
        return sal_True;
    else if ( ( nCurNode == nEndNode ) && ( rPaM.GetIndex() < aSel.Max().GetIndex() ) )
        return sal_True;

    return sal_False;
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

sal_Bool ImpEditView::IsSelectionAtPoint( const Point& rPosPixel )
{
    if ( pDragAndDropInfo && pDragAndDropInfo->pField )
        return sal_True;

    Point aMousePos( rPosPixel );

    // Logical units ...
    aMousePos = GetWindow()->PixelToLogic( aMousePos );

    if ( ( !GetOutputArea().IsInside( aMousePos ) ) && !pEditEngine->pImpEditEngine->IsInSelectionMode() )
    {
        return sal_False;
    }

    Point aDocPos( GetDocPos( aMousePos ) );
    EditPaM aPaM = pEditEngine->GetPaM(aDocPos, false);
    return IsInSelection( aPaM );
}

sal_Bool ImpEditView::SetCursorAtPoint( const Point& rPointPixel )
{
    pEditEngine->CheckIdleFormatter();

    Point aMousePos( rPointPixel );

    // Logical units ...
    aMousePos = GetWindow()->PixelToLogic( aMousePos );

    if ( ( !GetOutputArea().IsInside( aMousePos ) ) && !pEditEngine->pImpEditEngine->IsInSelectionMode() )
    {
        return sal_False;
    }

    Point aDocPos( GetDocPos( aMousePos ) );

    // Can be optimized: first go through the lines within a paragraph for PAM,
    // then again wiht the PaM for the Rect, even though the line is already
    // known .... This must not be, though!
    EditPaM aPaM = pEditEngine->GetPaM(aDocPos);
    sal_Bool bGotoCursor = DoAutoScroll();

    // aTmpNewSel: Diff between old and new, not the new selection
    EditSelection aTmpNewSel( GetEditSelection().Max(), aPaM );

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

    sal_Bool bForceCursor = ( pDragAndDropInfo ? sal_False : sal_True ) && !pEditEngine->pImpEditEngine->IsInSelectionMode();
    ShowCursor( bGotoCursor, bForceCursor );
    return sal_True;
}


void ImpEditView::HideDDCursor()
{
    if ( pDragAndDropInfo && pDragAndDropInfo->bVisCursor )
    {
        GetWindow()->DrawOutDev( pDragAndDropInfo->aCurSavedCursor.TopLeft(), pDragAndDropInfo->aCurSavedCursor.GetSize(),
                            Point(0,0), pDragAndDropInfo->aCurSavedCursor.GetSize(),*pDragAndDropInfo->pBackground );
        pDragAndDropInfo->bVisCursor = sal_False;
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
        Rectangle aSaveRec( GetWindow()->LogicToPixel( rRect ) );
        // prefer to save some more ...
        aSaveRec.Right() += 1;
        aSaveRec.Bottom() += 1;

#ifdef DBG_UTIL
        Size aNewSzPx( aSaveRec.GetSize() );
#endif
        if ( !pDragAndDropInfo->pBackground )
        {
            pDragAndDropInfo->pBackground = new VirtualDevice( *GetWindow() );
            MapMode aMapMode( GetWindow()->GetMapMode() );
            aMapMode.SetOrigin( Point( 0, 0 ) );
            pDragAndDropInfo->pBackground->SetMapMode( aMapMode );

        }

#ifdef DBG_UTIL
        Size aCurSzPx( pDragAndDropInfo->pBackground->GetOutputSizePixel() );
        if ( ( aCurSzPx.Width() < aNewSzPx.Width() ) ||( aCurSzPx.Height() < aNewSzPx.Height() ) )
        {
            sal_Bool bDone = pDragAndDropInfo->pBackground->SetOutputSizePixel( aNewSzPx );
            DBG_ASSERT( bDone, "Virtual Device broken?" );
        }
#endif

        aSaveRec = GetWindow()->PixelToLogic( aSaveRec );

        pDragAndDropInfo->pBackground->DrawOutDev( Point(0,0), aSaveRec.GetSize(),
                                    aSaveRec.TopLeft(), aSaveRec.GetSize(), *GetWindow() );
        pDragAndDropInfo->aCurSavedCursor = aSaveRec;

        // Draw Cursor...
        GetWindow()->DrawRect( rRect );

        pDragAndDropInfo->bVisCursor = sal_True;
        pDragAndDropInfo->aCurCursor = rRect;

        GetWindow()->SetFillColor( aOldFillColor );
    }
}

void ImpEditView::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& rDGE ) throw (::com::sun::star::uno::RuntimeException)
{
    DBG_ASSERT( !pDragAndDropInfo, "dragGestureRecognized - DragAndDropInfo exist!" );

    SolarMutexGuard aVclGuard;

    pDragAndDropInfo = NULL;

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
        sal_uInt16 nPara, nPos;
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
            sal_Bool bGotoCursor = DoAutoScroll();
            sal_Bool bForceCursor = ( pDragAndDropInfo ? sal_False : sal_True ) && !pEditEngine->pImpEditEngine->IsInSelectionMode();
            ShowCursor( bGotoCursor, bForceCursor );
        }
        else if ( IsBulletArea( aMousePos, &nPara ) )
        {
            pDragAndDropInfo = new DragAndDropInfo();
            pDragAndDropInfo->bOutlinerMode = sal_True;
            EditPaM aStartPaM( pEditEngine->GetEditDoc().GetObject( nPara ), 0 );
            EditPaM aEndPaM( aStartPaM );
            const SfxInt16Item& rLevel = (const SfxInt16Item&) pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
            for ( sal_uInt16 n = nPara +1; n < pEditEngine->GetEditDoc().Count(); n++ )
            {
                const SfxInt16Item& rL = (const SfxInt16Item&) pEditEngine->GetParaAttrib( n, EE_PARA_OUTLLEVEL );
                if ( rL.GetValue() > rLevel.GetValue() )
                {
                    aEndPaM.SetNode( pEditEngine->GetEditDoc().GetObject( n ) );
                }
                else
                {
                    break;
                }
            }
            aEndPaM.GetIndex() = aEndPaM.GetNode()->Len();
            SetEditSelection( EditSelection( aStartPaM, aEndPaM ) );
        }
    }

    if ( pDragAndDropInfo )
    {

        pDragAndDropInfo->bStarterOfDD = sal_True;

        // Sensitive area to be scrolled.
        Size aSz( 5, 0 );
        aSz = GetWindow()->PixelToLogic( aSz );
        pDragAndDropInfo->nSensibleRange = (sal_uInt16) aSz.Width();
        pDragAndDropInfo->nCursorWidth = (sal_uInt16) aSz.Width() / 2;
        pDragAndDropInfo->aBeginDragSel = pEditEngine->pImpEditEngine->CreateESel( aCopySel );

        uno::Reference<datatransfer::XTransferable> xData = pEditEngine->CreateTransferable(aCopySel);

        sal_Int8 nActions = bReadOnly ? datatransfer::dnd::DNDConstants::ACTION_COPY : datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE;

        rDGE.DragSource->startDrag( rDGE, nActions, 0 /*cursor*/, 0 /*image*/, xData, mxDnDListener );
        // If Drag&Move in an Engine, thenCopy&Del has to be optional!
        GetCursor()->Hide();

    }
}

void ImpEditView::dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& rDSDE ) throw (::com::sun::star::uno::RuntimeException)
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
                sal_Bool bBeforeSelection = aDropPos.IsLess( pDragAndDropInfo->aBeginDragSel );
                sal_uInt16 nParaDiff = pDragAndDropInfo->aBeginDragSel.nEndPara - pDragAndDropInfo->aBeginDragSel.nStartPara;
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
                if (pEditEngine->HasText())   // #88630# SC ist removing the content when switching the task
                    DeleteSelected();
            }
        }

        if ( pDragAndDropInfo->bUndoAction )
            pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_DRAGANDDROP );

        HideDDCursor();
        ShowCursor( DoAutoScroll(), sal_True );
        delete pDragAndDropInfo;
        pDragAndDropInfo = NULL;
        pEditEngine->GetEndDropHdl().Call(GetEditViewPtr());
    }
}

void ImpEditView::drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aVclGuard;

    DBG_ASSERT( pDragAndDropInfo, "Drop - No Drag&Drop info?!" );

    if ( pDragAndDropInfo && pDragAndDropInfo->bDragAccepted )
    {
        pEditEngine->GetBeginDropHdl().Call(GetEditViewPtr());
        sal_Bool bChanges = sal_False;

        HideDDCursor();

        if ( pDragAndDropInfo->bStarterOfDD )
        {
            pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_DRAGANDDROP );
            pDragAndDropInfo->bUndoAction = sal_True;
        }

        if ( pDragAndDropInfo->bOutlinerMode )
        {
            bChanges = sal_True;
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
                aPasteOrDropInfos.nAction = EE_ACTION_DROP;
                aPasteOrDropInfos.nStartPara = pEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
                pEditEngine->HandleBeginPasteOrDrop(aPasteOrDropInfos);

                EditSelection aNewSel = pEditEngine->InsertText(
                    xDataObj, String(), aPaM, pEditEngine->GetInternalEditStatus().AllowPasteSpecial());

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
                    pDragAndDropInfo->bDroppedInMe = sal_True;
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
            pDragAndDropInfo = NULL;
        }

        rDTDE.Context->dropComplete( bChanges );
    }
}

void ImpEditView::dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& rDTDEE ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aVclGuard;

    if ( !pDragAndDropInfo )
        pDragAndDropInfo = new DragAndDropInfo( );

    pDragAndDropInfo->bHasValidData = sal_False;

    // Check for supported format...
    // Only check for text, will also be there if bin or rtf
    datatransfer::DataFlavor aTextFlavor;
    SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aTextFlavor );
    const ::com::sun::star::datatransfer::DataFlavor* pFlavors = rDTDEE.SupportedDataFlavors.getConstArray();
    int nFlavors = rDTDEE.SupportedDataFlavors.getLength();
    for ( int n = 0; n < nFlavors; n++ )
    {
        if( TransferableDataHelper::IsEqual( pFlavors[n], aTextFlavor ) )
        {
            pDragAndDropInfo->bHasValidData = sal_True;
            break;
        }
    }

    dragOver( rDTDEE );
}

void ImpEditView::dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aVclGuard;

    HideDDCursor();

    if ( pDragAndDropInfo && !pDragAndDropInfo->bStarterOfDD )
    {
        delete pDragAndDropInfo;
        pDragAndDropInfo = NULL;
    }
}

void ImpEditView::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aVclGuard;

    Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );
    aMousePos = GetWindow()->PixelToLogic( aMousePos );

    sal_Bool bAccept = sal_False;

    if ( GetOutputArea().IsInside( aMousePos ) && !bReadOnly )
    {
        if ( pDragAndDropInfo && pDragAndDropInfo->bHasValidData )
        {
            bAccept = sal_True;

            sal_Bool bAllowScroll = DoAutoScroll();
            if ( bAllowScroll )
            {
                long nScrollX = 0;
                long nScrollY = 0;
                // Chech if in the sensitive area
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
                    Scroll( nScrollX, nScrollY, RGCHK_PAPERSZ1 );
                }
            }

            Point aDocPos( GetDocPos( aMousePos ) );
            EditPaM aPaM = pEditEngine->GetPaM( aDocPos );
            pDragAndDropInfo->aDropDest = aPaM;
            if ( pDragAndDropInfo->bOutlinerMode )
            {
                sal_uInt16 nPara = pEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
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
                    bAccept = sal_False;
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
                    bAccept = sal_False;
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

                sal_Bool bCursorChanged = !pDragAndDropInfo->bVisCursor || ( pDragAndDropInfo->aCurCursor != aEditCursor );
                if ( bCursorChanged )
                {
                    HideDDCursor();
                    ShowDDCursor(aEditCursor );
                }
                pDragAndDropInfo->bDragAccepted = sal_True;
                rDTDE.Context->acceptDrag( rDTDE.DropAction );
            }
        }
    }

    if ( !bAccept )
    {
        HideDDCursor();
        if (pDragAndDropInfo)
            pDragAndDropInfo->bDragAccepted = sal_False;
        rDTDE.Context->rejectDrag();
    }
}

void ImpEditView::AddDragAndDropListeners()
{
    Window* pWindow = GetWindow();
    if ( !bActiveDragAndDropListener && pWindow && pWindow->GetDragGestureRecognizer().is() )
    {
        vcl::unohelper::DragAndDropWrapper* pDnDWrapper = new vcl::unohelper::DragAndDropWrapper( this );
        mxDnDListener = pDnDWrapper;

        uno::Reference< datatransfer::dnd::XDragGestureListener> xDGL( mxDnDListener, uno::UNO_QUERY );
        pWindow->GetDragGestureRecognizer()->addDragGestureListener( xDGL );
        uno::Reference< datatransfer::dnd::XDropTargetListener> xDTL( xDGL, uno::UNO_QUERY );
        pWindow->GetDropTarget()->addDropTargetListener( xDTL );
        pWindow->GetDropTarget()->setActive( sal_True );
        pWindow->GetDropTarget()->setDefaultActions( datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE );

        bActiveDragAndDropListener = sal_True;
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

        bActiveDragAndDropListener = sal_False;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
