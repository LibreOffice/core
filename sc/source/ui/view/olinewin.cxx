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

#include <vcl/svapp.hxx>
#include <vcl/taskpanelist.hxx>

#include "olinewin.hxx"
#include "olinetab.hxx"
#include "document.hxx"
#include "dbfunc.hxx"
#include "sc.hrc"

// ============================================================================

const long SC_OL_BITMAPSIZE                 = 12;
const long SC_OL_POSOFFSET                  = 2;

const size_t SC_OL_NOLEVEL                  = static_cast< size_t >( -1 );
const size_t SC_OL_HEADERENTRY              = static_cast< size_t >( -1 );

const sal_uInt16 SC_OL_IMAGE_PLUS               = 9;
const sal_uInt16 SC_OL_IMAGE_MINUS              = SC_OL_IMAGE_PLUS + 1;
const sal_uInt16 SC_OL_IMAGE_NOTPRESSED         = SC_OL_IMAGE_MINUS + 1;
const sal_uInt16 SC_OL_IMAGE_PRESSED            = SC_OL_IMAGE_NOTPRESSED + 1;

// ============================================================================

ScOutlineWindow::ScOutlineWindow( Window* pParent, ScOutlineMode eMode, ScViewData* pViewData, ScSplitPos eWhich ) :
    Window( pParent ),
    mrViewData( *pViewData ),
    meWhich( eWhich ),
    mbHoriz( eMode == SC_OUTLINE_HOR ),
    mbMirrorEntries( false ),           // updated in SetHeaderSize
    mbMirrorLevels( false ),            // updated in SetHeaderSize
    mpSymbols( NULL ),
    maLineColor( COL_BLACK ),
    mnHeaderSize( 0 ),
    mnHeaderPos( 0 ),
    mnMainFirstPos( 0 ),
    mnMainLastPos( 0 ),
    mbMTActive( false ),
    mbMTPressed( false ),
    mnFocusLevel( 0 ),
    mnFocusEntry( SC_OL_HEADERENTRY ),
    mbDontDrawFocus( false )
{
    EnableRTL( false );                 // mirroring is done manually

    InitSettings();
    maFocusRect.SetEmpty();
    SetHeaderSize( 0 );

    // insert the window into task pane list for "F6 cycling"
    if( SystemWindow* pSysWin = GetSystemWindow() )
        if( TaskPaneList* pTaskPaneList = pSysWin->GetTaskPaneList() )
            pTaskPaneList->AddWindow( this );
}

ScOutlineWindow::~ScOutlineWindow()
{
    // remove the window from task pane list
    if( SystemWindow* pSysWin = GetSystemWindow() )
        if( TaskPaneList* pTaskPaneList = pSysWin->GetTaskPaneList() )
            pTaskPaneList->RemoveWindow( this );
}

void ScOutlineWindow::SetHeaderSize( long nNewSize )
{
    sal_Bool bLayoutRTL = GetDoc().IsLayoutRTL( GetTab() );
    mbMirrorEntries = bLayoutRTL && mbHoriz;
    mbMirrorLevels = bLayoutRTL && !mbHoriz;

    bool bNew = (nNewSize != mnHeaderSize);
    mnHeaderSize = nNewSize;
    mnHeaderPos = mbMirrorEntries ? (GetOutputSizeEntry() - mnHeaderSize) : 0;
    mnMainFirstPos = mbMirrorEntries ? 0 : mnHeaderSize;
    mnMainLastPos = GetOutputSizeEntry() - (mbMirrorEntries ? mnHeaderSize : 0) - 1;
    if ( bNew )
        Invalidate();
}

long ScOutlineWindow::GetDepthSize() const
{
    long nSize = GetLevelCount() * SC_OL_BITMAPSIZE;
    if ( nSize > 0 )
        nSize += 2 * SC_OL_POSOFFSET + 1;
    return nSize;
}

void ScOutlineWindow::ScrollPixel( long nDiff )
{
    HideFocus();
    mbDontDrawFocus = true;

    long nStart = mnMainFirstPos;
    long nEnd = mnMainLastPos;

    long nInvStart, nInvEnd;
    if (nDiff < 0)
    {
        nStart -= nDiff;
        nInvStart = nEnd + nDiff;
        nInvEnd = nEnd;
    }
    else
    {
        nEnd -= nDiff;
        nInvStart = nStart;
        nInvEnd = nStart + nDiff;
    }

    ScrollRel( nDiff, nStart, nEnd );
    Invalidate( GetRectangle( 0, nInvStart, GetOutputSizeLevel() - 1, nInvEnd ) );
    Update();

    // if focus becomes invisible, move it to next visible button
    ImplMoveFocusToVisible( nDiff < 0 );

    mbDontDrawFocus = false;
    ShowFocus();
}

void ScOutlineWindow::ScrollRel( long nEntryDiff, long nEntryStart, long nEntryEnd )
{
    Rectangle aRect( GetRectangle( 0, nEntryStart, GetOutputSizeLevel() - 1, nEntryEnd ) );
    if ( mbHoriz )
        Scroll( nEntryDiff, 0, aRect );
    else
        Scroll( 0, nEntryDiff, aRect );
}

// internal -------------------------------------------------------------------

void ScOutlineWindow::InitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( rStyleSettings.GetFaceColor() );
    maLineColor = rStyleSettings.GetButtonTextColor();
    mpSymbols = ScGlobal::GetOutlineSymbols();
    Invalidate();
}

const ScOutlineArray* ScOutlineWindow::GetOutlineArray() const
{
    const ScOutlineTable* pTable = GetDoc().GetOutlineTable( GetTab() );
    if ( !pTable ) return NULL;
    return mbHoriz ? pTable->GetColArray() : pTable->GetRowArray();
}

const ScOutlineEntry* ScOutlineWindow::GetOutlineEntry( size_t nLevel, size_t nEntry ) const
{
    const ScOutlineArray* pArray = GetOutlineArray();
    return pArray ? pArray->GetEntry( sal::static_int_cast<sal_uInt16>(nLevel), sal::static_int_cast<sal_uInt16>(nEntry) ) : NULL;
}

bool ScOutlineWindow::IsHidden( SCCOLROW nColRowIndex ) const
{
    return mbHoriz ?
        GetDoc().ColHidden(static_cast<SCCOL>(nColRowIndex), GetTab()) :
        GetDoc().RowHidden(static_cast<SCROW>(nColRowIndex), GetTab());
}

bool ScOutlineWindow::IsFiltered( SCCOLROW nColRowIndex ) const
{
    // columns cannot be filtered
    return !mbHoriz && GetDoc().RowFiltered( static_cast<SCROW>(nColRowIndex), GetTab() );
}

bool ScOutlineWindow::IsFirstVisible( SCCOLROW nColRowIndex ) const
{
    bool bAllHidden = true;
    for ( SCCOLROW nPos = 0; (nPos < nColRowIndex) && bAllHidden; ++nPos )
        bAllHidden = IsHidden( nPos );
    return bAllHidden;
}

void ScOutlineWindow::GetVisibleRange( SCCOLROW& rnColRowStart, SCCOLROW& rnColRowEnd ) const
{
    if ( mbHoriz )
    {
        rnColRowStart = mrViewData.GetPosX( WhichH( meWhich ) );
        rnColRowEnd = rnColRowStart + mrViewData.VisibleCellsX( WhichH( meWhich ) );
    }
    else
    {
        rnColRowStart = mrViewData.GetPosY( WhichV( meWhich ) );
        rnColRowEnd = rnColRowStart + mrViewData.VisibleCellsY( WhichV( meWhich ) );
    }

    // include collapsed columns/rows in front of visible range
    while ( (rnColRowStart > 0) && IsHidden( rnColRowStart - 1 ) )
        --rnColRowStart;
}

Point ScOutlineWindow::GetPoint( long nLevelPos, long nEntryPos ) const
{
    return mbHoriz ? Point( nEntryPos, nLevelPos ) : Point( nLevelPos, nEntryPos );
}

Rectangle ScOutlineWindow::GetRectangle(
        long nLevelStart, long nEntryStart, long nLevelEnd, long nEntryEnd ) const
{
    return Rectangle( GetPoint( nLevelStart, nEntryStart ), GetPoint( nLevelEnd, nEntryEnd ) );
}

long ScOutlineWindow::GetOutputSizeLevel() const
{
    Size aSize( GetOutputSizePixel() );
    return mbHoriz ? aSize.Height() : aSize.Width();
}

long ScOutlineWindow::GetOutputSizeEntry() const
{
    Size aSize( GetOutputSizePixel() );
    return mbHoriz ? aSize.Width() : aSize.Height();
}

size_t ScOutlineWindow::GetLevelCount() const
{
    const ScOutlineArray* pArray = GetOutlineArray();
    size_t nLevelCount = pArray ? pArray->GetDepth() : 0;
    return nLevelCount ? (nLevelCount + 1) : 0;
}

long ScOutlineWindow::GetLevelPos( size_t nLevel ) const
{
    // #i51970# must always return the *left* edge of the area used by a level
    long nPos = static_cast< long >( SC_OL_POSOFFSET + nLevel * SC_OL_BITMAPSIZE );
    return mbMirrorLevels ? (GetOutputSizeLevel() - nPos - SC_OL_BITMAPSIZE) : nPos;
}

size_t ScOutlineWindow::GetLevelFromPos( long nLevelPos ) const
{
    if( mbMirrorLevels ) nLevelPos = GetOutputSizeLevel() - nLevelPos - 1;
    long nStart = SC_OL_POSOFFSET;
    if ( nLevelPos < nStart ) return SC_OL_NOLEVEL;
    size_t nLevel = static_cast< size_t >( (nLevelPos - nStart) / SC_OL_BITMAPSIZE );
    return (nLevel < GetLevelCount()) ? nLevel : SC_OL_NOLEVEL;
}

long ScOutlineWindow::GetColRowPos( SCCOLROW nColRowIndex ) const
{
    long nDocPos = mbHoriz ?
        mrViewData.GetScrPos( static_cast<SCCOL>(nColRowIndex), 0, meWhich, sal_True ).X() :
        mrViewData.GetScrPos( 0, static_cast<SCROW>(nColRowIndex), meWhich, sal_True ).Y();
    return mnMainFirstPos + nDocPos;
}

long ScOutlineWindow::GetHeaderEntryPos() const
{
    return mnHeaderPos + (mnHeaderSize - SC_OL_BITMAPSIZE) / 2;
}

bool ScOutlineWindow::GetEntryPos(
        size_t nLevel, size_t nEntry,
        long& rnStartPos, long& rnEndPos, long& rnImagePos ) const
{
    const ScOutlineEntry* pEntry = GetOutlineEntry( nLevel, nEntry );
    if ( !pEntry || !pEntry->IsVisible() )
        return false;

    SCCOLROW nStart = pEntry->GetStart();
    SCCOLROW nEnd = pEntry->GetEnd();

    long nEntriesSign = mbMirrorEntries ? -1 : 1;

    // --- common calculation ---

    rnStartPos = GetColRowPos( nStart );
    rnEndPos = GetColRowPos( nEnd + 1 );

    bool bHidden = IsHidden( nStart );
    rnImagePos = bHidden ?
                (rnStartPos - ( SC_OL_BITMAPSIZE / 2 ) * nEntriesSign) :
                rnStartPos + nEntriesSign;
    long nCenter = (rnStartPos + rnEndPos - SC_OL_BITMAPSIZE * nEntriesSign +
                        ( mbMirrorEntries ? 1 : 0 )) / 2L;
    rnImagePos = mbMirrorEntries ? Max( rnImagePos, nCenter ) : Min( rnImagePos, nCenter );

    // --- refinements ---

    // do not cut leftmost/topmost image
    if ( bHidden && IsFirstVisible( nStart ) )
        rnImagePos = rnStartPos;

    // do not cover previous collapsed image
    if ( !bHidden && nEntry )
    {
        const ScOutlineEntry* pPrevEntry = GetOutlineEntry( nLevel, nEntry - 1 );
        SCCOLROW nPrevEnd = pPrevEntry->GetEnd();
        if ( (nPrevEnd + 1 == nStart) && IsHidden( nPrevEnd ) )
        {
            if ( IsFirstVisible( pPrevEntry->GetStart() ) )
                rnStartPos += SC_OL_BITMAPSIZE * nEntriesSign;
            else
                rnStartPos += ( SC_OL_BITMAPSIZE / 2 ) * nEntriesSign;
            rnImagePos = rnStartPos;
        }
    }

    // restrict rnStartPos...rnEndPos to valid area
    rnStartPos = std::max( rnStartPos, mnMainFirstPos );
    rnEndPos = std::max( rnEndPos, mnMainFirstPos );

    if ( mbMirrorEntries )
        rnImagePos -= SC_OL_BITMAPSIZE - 1;     // start pos aligns with right edge of bitmap

    // --- all rows filtered? ---

    bool bVisible = true;
    if ( !mbHoriz )
    {
        bVisible = false;
        for ( SCCOLROW nRow = nStart; (nRow <= nEnd) && !bVisible; ++nRow )
            bVisible = !IsFiltered( nRow );
    }
    return bVisible;
}

bool ScOutlineWindow::GetImagePos( size_t nLevel, size_t nEntry, Point& rPos ) const
{
    bool bRet = nLevel < GetLevelCount();
    if ( bRet )
    {
        long nLevelPos = GetLevelPos( nLevel );
        if ( nEntry == SC_OL_HEADERENTRY )
            rPos = GetPoint( nLevelPos, GetHeaderEntryPos() );
        else
        {
            long nStartPos, nEndPos, nImagePos;
            bRet = GetEntryPos( nLevel, nEntry, nStartPos, nEndPos, nImagePos );
            rPos = GetPoint( nLevelPos, nImagePos );
        }
    }
    return bRet;
}

bool ScOutlineWindow::IsButtonVisible( size_t nLevel, size_t nEntry ) const
{
    bool bRet = false;
    if ( nEntry == SC_OL_HEADERENTRY )
        bRet = (mnHeaderSize > 0) && (nLevel < GetLevelCount());
    else
    {
        const ScOutlineEntry* pEntry = GetOutlineEntry( nLevel, nEntry );
        if ( pEntry && pEntry->IsVisible() )
        {
            SCCOLROW nStart, nEnd;
            GetVisibleRange( nStart, nEnd );
            bRet = (nStart <= pEntry->GetStart()) && (pEntry->GetStart() <= nEnd);
        }
    }
    return bRet;
}

bool ScOutlineWindow::ItemHit( const Point& rPos, size_t& rnLevel, size_t& rnEntry, bool& rbButton ) const
{
    const ScOutlineArray* pArray = GetOutlineArray();
    if ( !pArray ) return false;

    SCCOLROW nStartIndex, nEndIndex;
    GetVisibleRange( nStartIndex, nEndIndex );

    size_t nLevel = GetLevelFromPos( mbHoriz ? rPos.Y() : rPos.X() );
    if ( nLevel == SC_OL_NOLEVEL )
        return false;

    long nEntryMousePos = mbHoriz ? rPos.X() : rPos.Y();

    // --- level buttons ---

    if ( mnHeaderSize > 0 )
    {
        long nImagePos = GetHeaderEntryPos();
        if ( (nImagePos <= nEntryMousePos) && (nEntryMousePos < nImagePos + SC_OL_BITMAPSIZE) )
        {
            rnLevel = nLevel;
            rnEntry = SC_OL_HEADERENTRY;
            rbButton = true;
            return true;
        }
    }

    // --- expand/collapse buttons and expanded lines ---

    // search outline entries backwards
    size_t nEntry = pArray->GetCount( sal::static_int_cast<sal_uInt16>(nLevel) );
    while ( nEntry )
    {
        --nEntry;

        const ScOutlineEntry* pEntry = pArray->GetEntry( sal::static_int_cast<sal_uInt16>(nLevel),
                                                         sal::static_int_cast<sal_uInt16>(nEntry) );
        SCCOLROW nStart = pEntry->GetStart();
        SCCOLROW nEnd = pEntry->GetEnd();

        if ( (nEnd >= nStartIndex) && (nStart <= nEndIndex) )
        {
            long nStartPos, nEndPos, nImagePos;
            if ( GetEntryPos( nLevel, nEntry, nStartPos, nEndPos, nImagePos ) )
            {
                rnLevel = nLevel;
                rnEntry = nEntry;

                // button?
                if ( (nStart >= nStartIndex) && (nImagePos <= nEntryMousePos) && (nEntryMousePos < nImagePos + SC_OL_BITMAPSIZE) )
                {
                    rbButton = true;
                    return true;
                }

                // line?
                if ( mbMirrorEntries )
                    ::std::swap( nStartPos, nEndPos );      // in RTL mode, nStartPos is the larger value
                if ( (nStartPos <= nEntryMousePos) && (nEntryMousePos <= nEndPos) )
                {
                    rbButton = false;
                    return true;
                }
            }
        }
    }

    return false;
}

bool ScOutlineWindow::ButtonHit( const Point& rPos, size_t& rnLevel, size_t& rnEntry ) const
{
    bool bButton;
    bool bRet = ItemHit( rPos, rnLevel, rnEntry, bButton );
    return bRet && bButton;
}

bool ScOutlineWindow::LineHit( const Point& rPos, size_t& rnLevel, size_t& rnEntry ) const
{
    bool bButton;
    bool bRet = ItemHit( rPos, rnLevel, rnEntry, bButton );
    return bRet && !bButton;
}

void ScOutlineWindow::DoFunction( size_t nLevel, size_t nEntry ) const
{
    ScDBFunc& rFunc = *mrViewData.GetView();
    if ( nEntry == SC_OL_HEADERENTRY )
        rFunc.SelectLevel( mbHoriz, sal::static_int_cast<sal_uInt16>(nLevel) );
    else
    {
        const ScOutlineEntry* pEntry = GetOutlineEntry( nLevel, nEntry );
        if ( pEntry )
        {
            if ( pEntry->IsHidden() )
                rFunc.ShowOutline( mbHoriz, sal::static_int_cast<sal_uInt16>(nLevel), sal::static_int_cast<sal_uInt16>(nEntry) );
            else
                rFunc.HideOutline( mbHoriz, sal::static_int_cast<sal_uInt16>(nLevel), sal::static_int_cast<sal_uInt16>(nEntry) );
        }
    }
}

void ScOutlineWindow::DoExpand( size_t nLevel, size_t nEntry ) const
{
    const ScOutlineEntry* pEntry = GetOutlineEntry( nLevel, nEntry );
    if ( pEntry && pEntry->IsHidden() )
        DoFunction( nLevel, nEntry );
}

void ScOutlineWindow::DoCollapse( size_t nLevel, size_t nEntry ) const
{
    const ScOutlineEntry* pEntry = GetOutlineEntry( nLevel, nEntry );
    if ( pEntry && !pEntry->IsHidden() )
        DoFunction( nLevel, nEntry );
}

void ScOutlineWindow::Resize()
{
    Window::Resize();
    SetHeaderSize( mnHeaderSize );  // recalculates header/group positions
    if ( !IsFocusButtonVisible() )
    {
        HideFocus();
        ShowFocus();    // calculates valid position
    }
}

void ScOutlineWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        InitSettings();
        Invalidate();
    }
    Window::DataChanged( rDCEvt );
}

// drawing --------------------------------------------------------------------

void ScOutlineWindow::SetEntryAreaClipRegion()
{
    SetClipRegion( Rectangle(
        GetPoint( 0, mnMainFirstPos ),
        GetPoint( GetOutputSizeLevel() - 1, mnMainLastPos ) ) );
}

void ScOutlineWindow::DrawLineRel(
        long nLevelStart, long nEntryStart, long nLevelEnd, long nEntryEnd )
{
    DrawLine( GetPoint( nLevelStart, nEntryStart ), GetPoint( nLevelEnd, nEntryEnd ) );
}

void ScOutlineWindow::DrawRectRel(
        long nLevelStart, long nEntryStart, long nLevelEnd, long nEntryEnd )
{
    DrawRect( GetRectangle( nLevelStart, nEntryStart, nLevelEnd, nEntryEnd ) );
}

void ScOutlineWindow::DrawImageRel( long nLevelPos, long nEntryPos, sal_uInt16 nId )
{
    OSL_ENSURE( mpSymbols, "ScOutlineWindow::DrawImageRel - no images" );
    const Image& rImage = mpSymbols->GetImage( nId );
    SetLineColor();
    SetFillColor( GetBackground().GetColor() );
    Point aPos( GetPoint( nLevelPos, nEntryPos ) );
    DrawRect( Rectangle( aPos, rImage.GetSizePixel() ) );
    DrawImage( aPos, rImage );
}

void ScOutlineWindow::DrawBorderRel( size_t nLevel, size_t nEntry, bool bPressed )
{
    Point aPos;
    if ( GetImagePos( nLevel, nEntry, aPos ) )
    {
        OSL_ENSURE( mpSymbols, "ScOutlineWindow::DrawBorderRel - no images" );
        sal_uInt16 nId = bPressed ? SC_OL_IMAGE_PRESSED : SC_OL_IMAGE_NOTPRESSED;
        bool bClip = (nEntry != SC_OL_HEADERENTRY);
        if ( bClip )
            SetEntryAreaClipRegion();
        DrawImage( aPos, mpSymbols->GetImage( nId ) );
        if ( bClip )
            SetClipRegion();
    }
    mbMTPressed = bPressed;
}

void ScOutlineWindow::ShowFocus()
{
    if ( HasFocus() )
    {
        // first move to a visible position
        ImplMoveFocusToVisible( true );

        if ( IsFocusButtonVisible() )
        {
            Point aPos;
            if ( GetImagePos( mnFocusLevel, mnFocusEntry, aPos ) )
            {
                aPos += Point( 1, 1 );
                maFocusRect = Rectangle( aPos, Size( SC_OL_BITMAPSIZE - 2, SC_OL_BITMAPSIZE - 2 ) );
                bool bClip = (mnFocusEntry != SC_OL_HEADERENTRY);
                if ( bClip )
                    SetEntryAreaClipRegion();
                InvertTracking( maFocusRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
                if ( bClip )
                    SetClipRegion();
            }
        }
    }
}

void ScOutlineWindow::HideFocus()
{
    if ( !maFocusRect.IsEmpty() )
    {
        bool bClip = (mnFocusEntry != SC_OL_HEADERENTRY);
        if ( bClip )
            SetEntryAreaClipRegion();
        InvertTracking( maFocusRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
        if ( bClip )
            SetClipRegion();
        maFocusRect.SetEmpty();
    }
}

void ScOutlineWindow::Paint( const Rectangle& /* rRect */ )
{
    long nEntriesSign = mbMirrorEntries ? -1 : 1;
    long nLevelsSign  = mbMirrorLevels  ? -1 : 1;

    Size aSize = GetOutputSizePixel();
    long nLevelEnd = (mbHoriz ? aSize.Height() : aSize.Width()) - 1;
    long nEntryEnd = (mbHoriz ? aSize.Width() : aSize.Height()) - 1;

    SetLineColor( maLineColor );
    long nBorderPos = mbMirrorLevels ? 0 : nLevelEnd;
    DrawLineRel( nBorderPos, 0, nBorderPos, nEntryEnd );

    const ScOutlineArray* pArray = GetOutlineArray();
    if ( !pArray ) return;

    size_t nLevelCount = GetLevelCount();

    // --- draw header images ---

    if ( mnHeaderSize > 0 )
    {
        long nEntryPos = GetHeaderEntryPos();
        for ( size_t nLevel = 0; nLevel < nLevelCount; ++nLevel )
            DrawImageRel( GetLevelPos( nLevel ), nEntryPos, static_cast< sal_uInt16 >( nLevel + 1 ) );

        SetLineColor( maLineColor );
        long nLinePos = mnHeaderPos + (mbMirrorEntries ? 0 : (mnHeaderSize - 1));
        DrawLineRel( 0, nLinePos, nLevelEnd, nLinePos );
    }

    // --- draw lines & collapse/expand images ---

    SetEntryAreaClipRegion();

    SCCOLROW nStartIndex, nEndIndex;
    GetVisibleRange( nStartIndex, nEndIndex );

    for ( size_t nLevel = 0; nLevel + 1 < nLevelCount; ++nLevel )
    {
        long nLevelPos = GetLevelPos( nLevel );
        long nEntryPos1 = 0, nEntryPos2 = 0, nImagePos = 0;

        size_t nEntryCount = pArray->GetCount( sal::static_int_cast<sal_uInt16>(nLevel) );
        size_t nEntry;

        // first draw all lines in the current level
        SetLineColor();
        SetFillColor( maLineColor );
        for ( nEntry = 0; nEntry < nEntryCount; ++nEntry )
        {
            const ScOutlineEntry* pEntry = pArray->GetEntry( sal::static_int_cast<sal_uInt16>(nLevel),
                                                             sal::static_int_cast<sal_uInt16>(nEntry) );
            SCCOLROW nStart = pEntry->GetStart();
            SCCOLROW nEnd = pEntry->GetEnd();

            // visible range?
            bool bDraw = (nEnd >= nStartIndex) && (nStart <= nEndIndex);
            // find output coordinates
            if ( bDraw )
                bDraw = GetEntryPos( nLevel, nEntry, nEntryPos1, nEntryPos2, nImagePos );
            // draw, if not collapsed
            if ( bDraw && !pEntry->IsHidden() )
            {
                if ( nStart >= nStartIndex )
                    nEntryPos1 += nEntriesSign;
                nEntryPos2 -= 2 * nEntriesSign;
                long nLinePos = nLevelPos;
                if ( mbMirrorLevels )
                    nLinePos += SC_OL_BITMAPSIZE - 1;   // align with right edge of bitmap
                DrawRectRel( nLinePos, nEntryPos1, nLinePos + nLevelsSign, nEntryPos2 );

                if ( nEnd <= nEndIndex )
                    DrawRectRel( nLinePos, nEntryPos2 - nEntriesSign,
                                 nLinePos + ( SC_OL_BITMAPSIZE / 3 ) * nLevelsSign, nEntryPos2 );
            }
        }

        // draw all images in the level from last to first
        nEntry = nEntryCount;
        while ( nEntry )
        {
            --nEntry;

            const ScOutlineEntry* pEntry = pArray->GetEntry( sal::static_int_cast<sal_uInt16>(nLevel),
                                                             sal::static_int_cast<sal_uInt16>(nEntry) );
            SCCOLROW nStart = pEntry->GetStart();

            // visible range?
            bool bDraw = (nStartIndex <= nStart) && (nStart <= nEndIndex + 1);
            // find output coordinates
            if ( bDraw )
                bDraw = GetEntryPos( nLevel, nEntry, nEntryPos1, nEntryPos2, nImagePos );
            // draw, if not hidden by higher levels
            if ( bDraw )
            {
                sal_uInt16 nImageId = pEntry->IsHidden() ? SC_OL_IMAGE_PLUS : SC_OL_IMAGE_MINUS;
                DrawImageRel( nLevelPos, nImagePos, nImageId );
            }
        }
    }

    SetClipRegion();

    if ( !mbDontDrawFocus )
        ShowFocus();
}

// focus ----------------------------------------------------------------------

/** Increments or decrements a value and wraps at the specified limits.
    @return  true = value wrapped. */
static bool lcl_RotateValue( size_t& rnValue, size_t nMin, size_t nMax, bool bForward )
{
    OSL_ENSURE( nMin <= nMax, "lcl_RotateValue - invalid range" );
    OSL_ENSURE( nMax < static_cast< size_t >( -1 ), "lcl_RotateValue - range overflow" );
    bool bWrap = false;
    if ( bForward )
    {
        if ( rnValue < nMax )
            ++rnValue;
        else
        {
            rnValue = nMin;
            bWrap = true;
        }
    }
    else
    {
        if ( rnValue > nMin )
            --rnValue;
        else
        {
            rnValue = nMax;
            bWrap = true;
        }
    }
    return bWrap;
}

bool ScOutlineWindow::IsFocusButtonVisible() const
{
    return IsButtonVisible( mnFocusLevel, mnFocusEntry );
}

bool ScOutlineWindow::ImplMoveFocusByEntry( bool bForward, bool bFindVisible )
{
    const ScOutlineArray* pArray = GetOutlineArray();
    if ( !pArray )
        return false;

    bool bWrapped = false;
    size_t nEntryCount = pArray->GetCount( sal::static_int_cast<sal_uInt16>(mnFocusLevel) );
    // #i29530# entry count may be decreased after changing active sheet
    if( mnFocusEntry >= nEntryCount )
        mnFocusEntry = SC_OL_HEADERENTRY;
    size_t nOldEntry = mnFocusEntry;

    do
    {
        if ( mnFocusEntry == SC_OL_HEADERENTRY )
        {
            // move from header to first or last entry
            if ( nEntryCount > 0 )
                mnFocusEntry = bForward ? 0 : (nEntryCount - 1);
            /*  wrapped, if forward from right header to first entry,
                or if backward from left header to last entry */
            // Header and entries are now always in consistent order,
            // so there's no need to check for mirroring here.
            if ( !nEntryCount || !bForward )
                bWrapped = true;
        }
        else if ( lcl_RotateValue( mnFocusEntry, 0, nEntryCount - 1, bForward ) )
        {
            // lcl_RotateValue returns true -> wrapped the entry range -> move to header
            mnFocusEntry = SC_OL_HEADERENTRY;
            /*  wrapped, if forward from last entry to left header,
                or if backward from first entry to right header */
            if ( bForward )
                bWrapped = true;
        }
    }
    while ( bFindVisible && !IsFocusButtonVisible() && (nOldEntry != mnFocusEntry) );

    return bWrapped;
}

bool ScOutlineWindow::ImplMoveFocusByLevel( bool bForward )
{
    const ScOutlineArray* pArray = GetOutlineArray();
    if ( !pArray )
        return false;

    bool bWrapped = false;
    size_t nLevelCount = GetLevelCount();

    if ( mnFocusEntry == SC_OL_HEADERENTRY )
    {
        if ( nLevelCount > 0 )
            bWrapped = lcl_RotateValue( mnFocusLevel, 0, nLevelCount - 1, bForward );
    }
    else
    {
        const ScOutlineEntry* pEntry = pArray->GetEntry(
            mnFocusLevel, mnFocusEntry);

        if ( pEntry )
        {
            SCCOLROW nStart = pEntry->GetStart();
            SCCOLROW nEnd = pEntry->GetEnd();
            size_t nNewLevel = mnFocusLevel;
            size_t nNewEntry = 0;

            bool bFound = false;
            if ( bForward && (mnFocusLevel + 2 < nLevelCount) )
            {
                // next level -> find first child entry
                nNewLevel = mnFocusLevel + 1;
                bFound = pArray->GetEntryIndexInRange(nNewLevel, nStart, nEnd, nNewEntry);
            }
            else if ( !bForward && (mnFocusLevel > 0) )
            {
                // previous level -> find parent entry
                nNewLevel = mnFocusLevel - 1;
                bFound = pArray->GetEntryIndex(nNewLevel, nStart, nNewEntry);
            }

            if ( bFound && IsButtonVisible( nNewLevel, nNewEntry ) )
            {
                mnFocusLevel = nNewLevel;
                mnFocusEntry = nNewEntry;
            }
        }
    }

    return bWrapped;
}

bool ScOutlineWindow::ImplMoveFocusByTabOrder( bool bForward, bool bFindVisible )
{
    bool bRet = false;
    size_t nOldLevel = mnFocusLevel;
    size_t nOldEntry = mnFocusEntry;

    do
    {
        /*  one level up, if backward from left header,
            or one level down, if forward from right header */
        if ( (!bForward) && (mnFocusEntry == SC_OL_HEADERENTRY) )
            bRet |= ImplMoveFocusByLevel( bForward );
        // move to next/previous entry
        bool bWrapInLevel = ImplMoveFocusByEntry( bForward, false );
        bRet |= bWrapInLevel;
        /*  one level up, if wrapped backward to right header,
            or one level down, if wrapped forward to right header */
        if ( bForward && bWrapInLevel )
            bRet |= ImplMoveFocusByLevel( bForward );
    }
    while ( bFindVisible && !IsFocusButtonVisible() && ((nOldLevel != mnFocusLevel) || (nOldEntry != mnFocusEntry)) );

    return bRet;
}

void ScOutlineWindow::ImplMoveFocusToVisible( bool bForward )
{
    // first try to find an entry in the same level
    if ( !IsFocusButtonVisible() )
        ImplMoveFocusByEntry( bForward, true );
    // then try to find any other entry
    if ( !IsFocusButtonVisible() )
        ImplMoveFocusByTabOrder( bForward, true );
}

void ScOutlineWindow::MoveFocusByEntry( bool bForward )
{
    HideFocus();
    ImplMoveFocusByEntry( bForward, true );
    ShowFocus();
}

void ScOutlineWindow::MoveFocusByLevel( bool bForward )
{
    HideFocus();
    ImplMoveFocusByLevel( bForward );
    ShowFocus();
}

void ScOutlineWindow::MoveFocusByTabOrder( bool bForward )
{
    HideFocus();
    ImplMoveFocusByTabOrder( bForward, true );
    ShowFocus();
}

void ScOutlineWindow::GetFocus()
{
    Window::GetFocus();
    ShowFocus();
}

void ScOutlineWindow::LoseFocus()
{
    HideFocus();
    Window::LoseFocus();
}


// mouse ----------------------------------------------------------------------

void ScOutlineWindow::StartMouseTracking( size_t nLevel, size_t nEntry )
{
    mbMTActive = true;
    mnMTLevel = nLevel;
    mnMTEntry = nEntry;
    DrawBorderRel( nLevel, nEntry, true );
}

void ScOutlineWindow::EndMouseTracking()
{
    if ( mbMTPressed )
        DrawBorderRel( mnMTLevel, mnMTEntry, false );
    mbMTActive = false;
}

void ScOutlineWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( IsMouseTracking() )
    {
        size_t nLevel, nEntry;
        bool bHit = false;

        if ( ButtonHit( rMEvt.GetPosPixel(), nLevel, nEntry ) )
            bHit = (nLevel == mnMTLevel) && (nEntry == mnMTEntry);

        if ( bHit != mbMTPressed )
            DrawBorderRel( mnMTLevel, mnMTEntry, bHit );
    }
}

void ScOutlineWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( IsMouseTracking() )
    {
        EndMouseTracking();

        size_t nLevel, nEntry;
        if ( ButtonHit( rMEvt.GetPosPixel(), nLevel, nEntry ) )
            if ( (nLevel == mnMTLevel) && (nEntry == mnMTEntry) )
                DoFunction( nLevel, nEntry );
    }
}

void ScOutlineWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    size_t nLevel, nEntry;
    bool bHit = ButtonHit( rMEvt.GetPosPixel(), nLevel, nEntry );
    if ( bHit )
        StartMouseTracking( nLevel, nEntry );
    else if ( rMEvt.GetClicks() == 2 )
    {
        bHit = LineHit( rMEvt.GetPosPixel(), nLevel, nEntry );
        if ( bHit )
            DoFunction( nLevel, nEntry );
    }

    // if an item has been hit and window is focused, move focus to this item
    if ( bHit && HasFocus() )
    {
        HideFocus();
        mnFocusLevel = nLevel;
        mnFocusEntry = nEntry;
        ShowFocus();
    }
}


// keyboard -------------------------------------------------------------------

void ScOutlineWindow::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKCode = rKEvt.GetKeyCode();
    bool bNoMod = !rKCode.GetModifier();
    bool bShift = (rKCode.GetModifier() == KEY_SHIFT);
    bool bCtrl = (rKCode.GetModifier() == KEY_MOD1);

    sal_uInt16 nCode = rKCode.GetCode();
    bool bUpDownKey = (nCode == KEY_UP) || (nCode == KEY_DOWN);
    bool bLeftRightKey = (nCode == KEY_LEFT) || (nCode == KEY_RIGHT);

    // TAB key
    if ( (nCode == KEY_TAB) && (bNoMod || bShift) )
        // move forward without SHIFT key
        MoveFocusByTabOrder( bNoMod );      // TAB uses logical order, regardless of mirroring

    // LEFT/RIGHT/UP/DOWN keys
    else if ( bNoMod && (bUpDownKey || bLeftRightKey) )
    {
        bool bForward = (nCode == KEY_DOWN) || (nCode == KEY_RIGHT);
        if ( mbHoriz == bLeftRightKey )
            // move inside level with LEFT/RIGHT in horizontal and with UP/DOWN in vertical
            MoveFocusByEntry( bForward != mbMirrorEntries );
        else
            // move to next/prev level with LEFT/RIGHT in vertical and with UP/DOWN in horizontal
            MoveFocusByLevel( bForward != mbMirrorLevels );
    }

    // CTRL + number
    else if ( bCtrl && (nCode >= KEY_1) && (nCode <= KEY_9) )
    {
        size_t nLevel = static_cast< size_t >( nCode - KEY_1 );
        if ( nLevel < GetLevelCount() )
            DoFunction( nLevel, SC_OL_HEADERENTRY );
    }

    // other key codes
    else switch ( rKCode.GetFullCode() )
    {
        case KEY_ADD:       DoExpand( mnFocusLevel, mnFocusEntry );     break;
        case KEY_SUBTRACT:  DoCollapse( mnFocusLevel, mnFocusEntry );   break;
        case KEY_SPACE:
        case KEY_RETURN:    DoFunction( mnFocusLevel, mnFocusEntry );   break;
        default:            Window::KeyInput( rKEvt );
    }
}


// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
