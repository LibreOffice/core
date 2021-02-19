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

#include <sal/config.h>

#include <string_view>

#include <vcl/image.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/settings.hxx>

#include <olinewin.hxx>
#include <olinetab.hxx>
#include <document.hxx>
#include <dbfunc.hxx>
#include <bitmaps.hlst>

const tools::Long SC_OL_BITMAPSIZE                 = 12;
const tools::Long SC_OL_POSOFFSET                  = 2;

const size_t SC_OL_NOLEVEL                  = static_cast< size_t >( -1 );
const size_t SC_OL_HEADERENTRY              = static_cast< size_t >( -1 );

ScOutlineWindow::ScOutlineWindow( vcl::Window* pParent, ScOutlineMode eMode, ScViewData* pViewData, ScSplitPos eWhich ) :
    Window( pParent ),
    mrViewData( *pViewData ),
    meWhich( eWhich ),
    mbHoriz( eMode == SC_OUTLINE_HOR ),
    mbMirrorEntries( false ),           // updated in SetHeaderSize
    mbMirrorLevels( false ),            // updated in SetHeaderSize
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
    disposeOnce();
}

void ScOutlineWindow::dispose()
{
    // remove the window from task pane list
    if( SystemWindow* pSysWin = GetSystemWindow() )
        if( TaskPaneList* pTaskPaneList = pSysWin->GetTaskPaneList() )
            pTaskPaneList->RemoveWindow( this );
    vcl::Window::dispose();
}

void ScOutlineWindow::SetHeaderSize( tools::Long nNewSize )
{
    bool bLayoutRTL = GetDoc().IsLayoutRTL( GetTab() );
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

tools::Long ScOutlineWindow::GetDepthSize() const
{
    tools::Long nSize = GetLevelCount() * SC_OL_BITMAPSIZE;
    if ( nSize > 0 )
        nSize += 2 * SC_OL_POSOFFSET + 1;
    return nSize;
}

void ScOutlineWindow::ScrollPixel( tools::Long nDiff )
{
    HideFocus();
    mbDontDrawFocus = true;

    tools::Long nStart = mnMainFirstPos;
    tools::Long nEnd = mnMainLastPos;

    tools::Long nInvStart, nInvEnd;
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

    // if focus becomes invisible, move it to next visible button
    ImplMoveFocusToVisible( nDiff < 0 );

    mbDontDrawFocus = false;
    ShowFocus();
}

void ScOutlineWindow::ScrollRel( tools::Long nEntryDiff, tools::Long nEntryStart, tools::Long nEntryEnd )
{
    tools::Rectangle aRect( GetRectangle( 0, nEntryStart, GetOutputSizeLevel() - 1, nEntryEnd ) );
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
    Invalidate();
}

const ScOutlineArray* ScOutlineWindow::GetOutlineArray() const
{
    const ScOutlineTable* pTable = GetDoc().GetOutlineTable( GetTab() );
    if ( !pTable ) return nullptr;
    return mbHoriz ? &pTable->GetColArray() : &pTable->GetRowArray();
}

const ScOutlineEntry* ScOutlineWindow::GetOutlineEntry( size_t nLevel, size_t nEntry ) const
{
    const ScOutlineArray* pArray = GetOutlineArray();
    return pArray ? pArray->GetEntry( sal::static_int_cast<sal_uInt16>(nLevel), sal::static_int_cast<sal_uInt16>(nEntry) ) : nullptr;
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

Point ScOutlineWindow::GetPoint( tools::Long nLevelPos, tools::Long nEntryPos ) const
{
    return mbHoriz ? Point( nEntryPos, nLevelPos ) : Point( nLevelPos, nEntryPos );
}

tools::Rectangle ScOutlineWindow::GetRectangle(
        tools::Long nLevelStart, tools::Long nEntryStart, tools::Long nLevelEnd, tools::Long nEntryEnd ) const
{
    return tools::Rectangle( GetPoint( nLevelStart, nEntryStart ), GetPoint( nLevelEnd, nEntryEnd ) );
}

tools::Long ScOutlineWindow::GetOutputSizeLevel() const
{
    Size aSize( GetOutputSizePixel() );
    return mbHoriz ? aSize.Height() : aSize.Width();
}

tools::Long ScOutlineWindow::GetOutputSizeEntry() const
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

tools::Long ScOutlineWindow::GetLevelPos( size_t nLevel ) const
{
    // #i51970# must always return the *left* edge of the area used by a level
    tools::Long nPos = static_cast< tools::Long >( SC_OL_POSOFFSET + nLevel * SC_OL_BITMAPSIZE );
    return mbMirrorLevels ? (GetOutputSizeLevel() - nPos - SC_OL_BITMAPSIZE) : nPos;
}

size_t ScOutlineWindow::GetLevelFromPos( tools::Long nLevelPos ) const
{
    if( mbMirrorLevels ) nLevelPos = GetOutputSizeLevel() - nLevelPos - 1;
    tools::Long nStart = SC_OL_POSOFFSET;
    if ( nLevelPos < nStart ) return SC_OL_NOLEVEL;
    size_t nLevel = static_cast< size_t >( (nLevelPos - nStart) / SC_OL_BITMAPSIZE );
    return (nLevel < GetLevelCount()) ? nLevel : SC_OL_NOLEVEL;
}

tools::Long ScOutlineWindow::GetColRowPos( SCCOLROW nColRowIndex ) const
{
    tools::Long nDocPos = mbHoriz ?
        mrViewData.GetScrPos( static_cast<SCCOL>(nColRowIndex), 0, meWhich, true ).X() :
        mrViewData.GetScrPos( 0, static_cast<SCROW>(nColRowIndex), meWhich, true ).Y();
    return mnMainFirstPos + nDocPos;
}

tools::Long ScOutlineWindow::GetHeaderEntryPos() const
{
    return mnHeaderPos + (mnHeaderSize - SC_OL_BITMAPSIZE) / 2;
}

bool ScOutlineWindow::GetEntryPos(
        size_t nLevel, size_t nEntry,
        tools::Long& rnStartPos, tools::Long& rnEndPos, tools::Long& rnImagePos ) const
{
    const ScOutlineEntry* pEntry = GetOutlineEntry( nLevel, nEntry );
    if ( !pEntry || !pEntry->IsVisible() )
        return false;

    SCCOLROW nStart = pEntry->GetStart();
    SCCOLROW nEnd = pEntry->GetEnd();

    tools::Long nEntriesSign = mbMirrorEntries ? -1 : 1;

    // --- common calculation ---

    rnStartPos = GetColRowPos( nStart );
    rnEndPos = GetColRowPos( nEnd + 1 );

    bool bHidden = IsHidden( nStart );
    rnImagePos = bHidden ?
                (rnStartPos - ( SC_OL_BITMAPSIZE / 2 ) * nEntriesSign) :
                rnStartPos + nEntriesSign;
    tools::Long nCenter = (rnStartPos + rnEndPos - SC_OL_BITMAPSIZE * nEntriesSign +
                        ( mbMirrorEntries ? 1 : 0 )) / 2;
    rnImagePos = mbMirrorEntries ? std::max( rnImagePos, nCenter ) : std::min( rnImagePos, nCenter );

    // --- refinements ---

    // do not cut leftmost/topmost image
    if ( bHidden && IsFirstVisible( nStart ) )
        rnImagePos = rnStartPos;

    // do not cover previous collapsed image
    bool bDoNoCover = !bHidden && nEntry;
    const ScOutlineEntry* pPrevEntry = bDoNoCover ? GetOutlineEntry(nLevel, nEntry - 1) : nullptr;
    if (pPrevEntry)
    {
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
        tools::Long nLevelPos = GetLevelPos( nLevel );
        if ( nEntry == SC_OL_HEADERENTRY )
            rPos = GetPoint( nLevelPos, GetHeaderEntryPos() );
        else
        {
            tools::Long nStartPos, nEndPos, nImagePos;
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

    tools::Long nEntryMousePos = mbHoriz ? rPos.X() : rPos.Y();

    // --- level buttons ---

    if ( mnHeaderSize > 0 )
    {
        tools::Long nImagePos = GetHeaderEntryPos();
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
            tools::Long nStartPos, nEndPos, nImagePos;
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
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        InitSettings();
        Invalidate();
    }
    Window::DataChanged( rDCEvt );
}

// drawing --------------------------------------------------------------------

void ScOutlineWindow::SetEntryAreaClipRegion()
{
    SetClipRegion( vcl::Region(tools::Rectangle(
        GetPoint( 0, mnMainFirstPos ),
        GetPoint( GetOutputSizeLevel() - 1, mnMainLastPos ))));
}

void ScOutlineWindow::DrawLineRel(
        tools::Long nLevelStart, tools::Long nEntryStart, tools::Long nLevelEnd, tools::Long nEntryEnd )
{
    DrawLine( GetPoint( nLevelStart, nEntryStart ), GetPoint( nLevelEnd, nEntryEnd ) );
}

void ScOutlineWindow::DrawRectRel(
        tools::Long nLevelStart, tools::Long nEntryStart, tools::Long nLevelEnd, tools::Long nEntryEnd )
{
    DrawRect( GetRectangle( nLevelStart, nEntryStart, nLevelEnd, nEntryEnd ) );
}

namespace
{
    Image GetImage(const OUString& rId)
    {
        return Image(StockImage::Yes, rId);
    }
}

void ScOutlineWindow::DrawImageRel(tools::Long nLevelPos, tools::Long nEntryPos, const OUString& rId)
{
    const Image& rImage = GetImage(rId);
    SetLineColor();
    SetFillColor( GetBackground().GetColor() );
    Point aPos( GetPoint( nLevelPos, nEntryPos ) );
    DrawRect( tools::Rectangle( aPos, rImage.GetSizePixel() ) );
    DrawImage( aPos, rImage );
}

void ScOutlineWindow::DrawBorderRel( size_t nLevel, size_t nEntry, bool bPressed )
{
    Point aPos;
    if ( GetImagePos( nLevel, nEntry, aPos ) )
    {
        OUString sId = bPressed ? OUString(RID_BMP_PRESSED) : OUString(RID_BMP_NOTPRESSED);
        bool bClip = (nEntry != SC_OL_HEADERENTRY);
        if ( bClip )
            SetEntryAreaClipRegion();
        DrawImage(aPos, GetImage(sId));
        if ( bClip )
            SetClipRegion();
    }
    mbMTPressed = bPressed;
}

void ScOutlineWindow::ShowFocus()
{
    if ( !HasFocus() )
        return;

    // first move to a visible position
    ImplMoveFocusToVisible( true );

    if ( !IsFocusButtonVisible() )
        return;

    Point aPos;
    if ( GetImagePos( mnFocusLevel, mnFocusEntry, aPos ) )
    {
        aPos += Point( 1, 1 );
        maFocusRect = tools::Rectangle( aPos, Size( SC_OL_BITMAPSIZE - 2, SC_OL_BITMAPSIZE - 2 ) );
        bool bClip = (mnFocusEntry != SC_OL_HEADERENTRY);
        if ( bClip )
            SetEntryAreaClipRegion();
        InvertTracking( maFocusRect, ShowTrackFlags::Small | ShowTrackFlags::TrackWindow );
        if ( bClip )
            SetClipRegion();
    }
}

void ScOutlineWindow::HideFocus()
{
    if ( !maFocusRect.IsEmpty() )
    {
        bool bClip = (mnFocusEntry != SC_OL_HEADERENTRY);
        if ( bClip )
            SetEntryAreaClipRegion();
        InvertTracking( maFocusRect, ShowTrackFlags::Small | ShowTrackFlags::TrackWindow );
        if ( bClip )
            SetClipRegion();
        maFocusRect.SetEmpty();
    }
}

const std::u16string_view aLevelBmps[]=
{
    u"" RID_BMP_LEVEL1,
    u"" RID_BMP_LEVEL2,
    u"" RID_BMP_LEVEL3,
    u"" RID_BMP_LEVEL4,
    u"" RID_BMP_LEVEL5,
    u"" RID_BMP_LEVEL6,
    u"" RID_BMP_LEVEL7,
    u"" RID_BMP_LEVEL8
};

void ScOutlineWindow::Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& /* rRect */ )
{
    tools::Long nEntriesSign = mbMirrorEntries ? -1 : 1;
    tools::Long nLevelsSign  = mbMirrorLevels  ? -1 : 1;

    Size aSize = GetOutputSizePixel();
    tools::Long nLevelEnd = (mbHoriz ? aSize.Height() : aSize.Width()) - 1;
    tools::Long nEntryEnd = (mbHoriz ? aSize.Width() : aSize.Height()) - 1;

    SetLineColor( maLineColor );
    tools::Long nBorderPos = mbMirrorLevels ? 0 : nLevelEnd;
    DrawLineRel( nBorderPos, 0, nBorderPos, nEntryEnd );

    const ScOutlineArray* pArray = GetOutlineArray();
    if ( !pArray ) return;

    size_t nLevelCount = GetLevelCount();

    // --- draw header images ---

    if ( mnHeaderSize > 0 )
    {
        tools::Long nEntryPos = GetHeaderEntryPos();
        for ( size_t nLevel = 0; nLevel < nLevelCount; ++nLevel )
            DrawImageRel(GetLevelPos(nLevel), nEntryPos, OUString(aLevelBmps[nLevel]));

        SetLineColor( maLineColor );
        tools::Long nLinePos = mnHeaderPos + (mbMirrorEntries ? 0 : (mnHeaderSize - 1));
        DrawLineRel( 0, nLinePos, nLevelEnd, nLinePos );
    }

    // --- draw lines & collapse/expand images ---

    SetEntryAreaClipRegion();

    SCCOLROW nStartIndex, nEndIndex;
    GetVisibleRange( nStartIndex, nEndIndex );

    for ( size_t nLevel = 0; nLevel + 1 < nLevelCount; ++nLevel )
    {
        tools::Long nLevelPos = GetLevelPos( nLevel );
        tools::Long nEntryPos1 = 0, nEntryPos2 = 0, nImagePos = 0;

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
                tools::Long nLinePos = nLevelPos;
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
                OUString sImageId = pEntry->IsHidden() ? OUString(RID_BMP_PLUS) : OUString(RID_BMP_MINUS);
                DrawImageRel(nLevelPos, nImagePos, sImageId);
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

bool ScOutlineWindow::ImplMoveFocusByTabOrder( bool bForward )
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
    while ( !IsFocusButtonVisible() && ((nOldLevel != mnFocusLevel) || (nOldEntry != mnFocusEntry)) );

    return bRet;
}

void ScOutlineWindow::ImplMoveFocusToVisible( bool bForward )
{
    // first try to find an entry in the same level
    if ( !IsFocusButtonVisible() )
        ImplMoveFocusByEntry( bForward, true );
    // then try to find any other entry
    if ( !IsFocusButtonVisible() )
        ImplMoveFocusByTabOrder( bForward );
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
    ImplMoveFocusByTabOrder( bForward );
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
    const vcl::KeyCode& rKCode = rKEvt.GetKeyCode();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
