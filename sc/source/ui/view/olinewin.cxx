/*************************************************************************
 *
 *  $RCSfile: olinewin.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 15:10:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_TASKPANELIST_HXX
#include <vcl/taskpanelist.hxx>
#endif

#include "olinewin.hxx"
#include "olinetab.hxx"
#include "document.hxx"
#include "dbfunc.hxx"
#include "sc.hrc"

// ============================================================================

const sal_Int32 SC_OL_BITMAPSIZE            = 12;
const sal_Int32 SC_OL_POSOFFSET             = 2;

const sal_uInt16 SC_OL_NOLEVEL              = 0xFFFF;
const sal_uInt16 SC_OL_HEADERENTRY          = 0xFFFF;

const sal_uInt16 SC_OL_IMAGE_PLUS           = 9;
const sal_uInt16 SC_OL_IMAGE_MINUS          = SC_OL_IMAGE_PLUS + 1;
const sal_uInt16 SC_OL_IMAGE_NOTPRESSED     = SC_OL_IMAGE_MINUS + 1;
const sal_uInt16 SC_OL_IMAGE_PRESSED        = SC_OL_IMAGE_NOTPRESSED + 1;


// ============================================================================

ScOutlineWindow::ScOutlineWindow( Window* pParent, ScOutlineMode eMode, ScViewData* pViewData, ScSplitPos eWhich ) :
    Window( pParent ),
    mrViewData( *pViewData ),
    meWhich( eWhich ),
    mbHoriz( eMode == SC_OUTLINE_HOR ),
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
    EnableRTL( !mbHoriz ); // #107809# do not mirror (horizontal) column outline window
    mbMirrorHdr = mbHoriz && Application::GetSettings().GetLayoutRTL();

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

void ScOutlineWindow::SetHeaderSize( sal_Int32 nNewSize )
{
    bool bNew = (nNewSize != mnHeaderSize);
    mnHeaderSize = nNewSize;
    mnHeaderPos = mbMirrorHdr ? (GetOutputSizeEntry() - mnHeaderSize) : 0;
    mnMainFirstPos = mbMirrorHdr ? 0 : mnHeaderSize;
    mnMainLastPos = GetOutputSizeEntry() - (mbMirrorHdr ? mnHeaderSize : 0) - 1;
    if ( bNew )
        Invalidate();
}

sal_Int32 ScOutlineWindow::GetDepthSize() const
{
    sal_Int32 nSize = GetLevelCount() * SC_OL_BITMAPSIZE;
    if ( nSize > 0 )
        nSize += 2 * SC_OL_POSOFFSET + 1;
    return nSize;
}

void ScOutlineWindow::ScrollPixel( sal_Int32 nDiff )
{
    HideFocus();
    mbDontDrawFocus = true;

    sal_Int32 nStart = mnMainFirstPos;
    sal_Int32 nEnd = mnMainLastPos;

    sal_Int32 nInvStart, nInvEnd;
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

void ScOutlineWindow::ScrollRel( sal_Int32 nEntryDiff )
{
    if ( mbHoriz )
        Scroll( nEntryDiff, 0 );
    else
        Scroll( 0, nEntryDiff );
}

void ScOutlineWindow::ScrollRel( sal_Int32 nEntryDiff, sal_Int32 nEntryStart, sal_Int32 nEntryEnd )
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
    mpSymbols = ScGlobal::GetOutlineSymbols( !!GetBackground().GetColor().IsDark() );
    Invalidate();
}

const ScOutlineArray* ScOutlineWindow::GetOutlineArray() const
{
    const ScOutlineTable* pTable = GetDoc().GetOutlineTable( GetTab() );
    if ( !pTable ) return NULL;
    return mbHoriz ? pTable->GetColArray() : pTable->GetRowArray();
}

const ScOutlineEntry* ScOutlineWindow::GetOutlineEntry( sal_uInt16 nLevel, sal_uInt16 nEntry ) const
{
    const ScOutlineArray* pArray = GetOutlineArray();
    return pArray ? pArray->GetEntry( nLevel, nEntry ) : NULL;
}

bool ScOutlineWindow::IsHidden( sal_uInt16 nColRowIndex ) const
{
    sal_uInt8 nFlags = mbHoriz ?
        GetDoc().GetColFlags( nColRowIndex, GetTab() ) :
        GetDoc().GetRowFlags( nColRowIndex, GetTab() );
    return (nFlags & CR_HIDDEN) != 0;
}

bool ScOutlineWindow::IsFiltered( sal_uInt16 nColRowIndex ) const
{
    // columns cannot be filtered
    return !mbHoriz && GetDoc().IsFiltered( nColRowIndex, GetTab() );
}

bool ScOutlineWindow::IsFirstVisible( sal_uInt16 nColRowIndex ) const
{
    bool bAllHidden = true;
    for ( sal_uInt16 nPos = 0; (nPos < nColRowIndex) && bAllHidden; ++nPos )
        bAllHidden = IsHidden( nPos );
    return bAllHidden;
}

void ScOutlineWindow::GetVisibleRange( sal_uInt16& rnColRowStart, sal_uInt16& rnColRowEnd ) const
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

Point ScOutlineWindow::GetPoint( sal_Int32 nLevelPos, sal_Int32 nEntryPos ) const
{
    return mbHoriz ? Point( nEntryPos, nLevelPos ) : Point( nLevelPos, nEntryPos );
}

Rectangle ScOutlineWindow::GetRectangle(
        sal_Int32 nLevelStart, sal_Int32 nEntryStart,
        sal_Int32 nLevelEnd, sal_Int32 nEntryEnd ) const
{
    return Rectangle( GetPoint( nLevelStart, nEntryStart ), GetPoint( nLevelEnd, nEntryEnd ) );
}

sal_Int32 ScOutlineWindow::GetOutputSizeLevel() const
{
    Size aSize( GetOutputSizePixel() );
    return mbHoriz ? aSize.Height() : aSize.Width();
}

sal_Int32 ScOutlineWindow::GetOutputSizeEntry() const
{
    Size aSize( GetOutputSizePixel() );
    return mbHoriz ? aSize.Width() : aSize.Height();
}

sal_uInt16 ScOutlineWindow::GetLevelCount() const
{
    const ScOutlineArray* pArray = GetOutlineArray();
    return pArray ? (pArray->GetDepth() + 1) : 0;
}

sal_Int32 ScOutlineWindow::GetLevelPos( sal_uInt16 nLevel ) const
{
    return SC_OL_POSOFFSET + nLevel * SC_OL_BITMAPSIZE;
}

sal_uInt16 ScOutlineWindow::GetLevelFromPos( sal_Int32 nLevelPos ) const
{
    if ( nLevelPos < SC_OL_POSOFFSET ) return SC_OL_NOLEVEL;
    sal_uInt16 nLevel = static_cast< sal_uInt16 >( (nLevelPos - SC_OL_POSOFFSET) / SC_OL_BITMAPSIZE );
    return (nLevel < GetLevelCount()) ? nLevel : SC_OL_NOLEVEL;
}

sal_Int32 ScOutlineWindow::GetColRowPos( sal_uInt16 nColRowIndex ) const
{
    sal_Int32 nDocPos = mbHoriz ?
        mrViewData.GetScrPos( nColRowIndex, 0, meWhich, TRUE ).X() :
        mrViewData.GetScrPos( 0, nColRowIndex, meWhich, TRUE ).Y();
    return mnMainFirstPos + nDocPos;
}

sal_Int32 ScOutlineWindow::GetHeaderEntryPos() const
{
    return mnHeaderPos + (mnHeaderSize - SC_OL_BITMAPSIZE) / 2;
}

bool ScOutlineWindow::GetEntryPos(
        sal_uInt16 nLevel, sal_uInt16 nEntry,
        sal_Int32& rnStartPos, sal_Int32& rnEndPos, sal_Int32& rnImagePos ) const
{
    const ScOutlineEntry* pEntry = GetOutlineEntry( nLevel, nEntry );
    if ( !pEntry || !pEntry->IsVisible() )
        return false;

    sal_uInt16 nStart = pEntry->GetStart();
    sal_uInt16 nEnd = pEntry->GetEnd();

    // --- common calculation ---

    rnStartPos = GetColRowPos( nStart );
    rnEndPos = GetColRowPos( nEnd + 1 );

    bool bHidden = IsHidden( nStart );
    rnImagePos = bHidden ? (rnStartPos - SC_OL_BITMAPSIZE / 2) : rnStartPos + 1;
    rnImagePos = Min( rnImagePos, (rnStartPos + rnEndPos - SC_OL_BITMAPSIZE) / 2L );

    // --- refinements ---

    // do not cut leftmost/topmost image
    if ( bHidden && IsFirstVisible( nStart ) )
        rnImagePos = rnStartPos;

    // do not cover previous collapsed image
    if ( !bHidden && nEntry )
    {
        const ScOutlineEntry* pPrevEntry = GetOutlineEntry( nLevel, nEntry - 1 );
        sal_uInt16 nPrevEnd = pPrevEntry->GetEnd();
        if ( (nPrevEnd + 1 == nStart) && IsHidden( nPrevEnd ) )
        {
            if ( IsFirstVisible( pPrevEntry->GetStart() ) )
                rnStartPos += SC_OL_BITMAPSIZE;
            else
                rnStartPos += SC_OL_BITMAPSIZE / 2;
            rnImagePos = rnStartPos;
        }
    }

    // restrict rnStartPos...rnEndPos to valid area
    rnStartPos = Max( rnStartPos, mnMainFirstPos );
    rnEndPos = Max( rnEndPos, mnMainFirstPos );

    // --- all rows filtered? ---

    bool bVisible = true;
    if ( !mbHoriz )
    {
        bVisible = false;
        for ( sal_uInt16 nRow = nStart; (nRow <= nEnd) && !bVisible; ++nRow )
            bVisible = !IsFiltered( nRow );
    }
    return bVisible;
}

bool ScOutlineWindow::GetImagePos( sal_uInt16 nLevel, sal_uInt16 nEntry, Point& rPos ) const
{
    bool bRet = nLevel < GetLevelCount();
    if ( bRet )
    {
        sal_Int32 nLevelPos = GetLevelPos( nLevel );
        if ( nEntry == SC_OL_HEADERENTRY )
            rPos = GetPoint( nLevelPos, GetHeaderEntryPos() );
        else
        {
            sal_Int32 nStartPos, nEndPos, nImagePos;
            bRet = GetEntryPos( nLevel, nEntry, nStartPos, nEndPos, nImagePos );
            rPos = GetPoint( nLevelPos, nImagePos );
        }
    }
    return bRet;
}

bool ScOutlineWindow::IsButtonVisible( sal_uInt16 nLevel, sal_uInt16 nEntry ) const
{
    bool bRet = false;
    if ( nEntry == SC_OL_HEADERENTRY )
        bRet = (mnHeaderSize > 0) && (nLevel < GetLevelCount());
    else
    {
        const ScOutlineEntry* pEntry = GetOutlineEntry( nLevel, nEntry );
        if ( pEntry && pEntry->IsVisible() )
        {
            sal_uInt16 nStart, nEnd;
            GetVisibleRange( nStart, nEnd );
            bRet = (nStart <= pEntry->GetStart()) && (pEntry->GetStart() <= nEnd);
        }
    }
    return bRet;
}

bool ScOutlineWindow::ItemHit( const Point& rPos, sal_uInt16& rnLevel, sal_uInt16& rnEntry, bool& rbButton ) const
{
    const ScOutlineArray* pArray = GetOutlineArray();
    if ( !pArray ) return false;

    sal_uInt16 nStartIndex, nEndIndex;
    GetVisibleRange( nStartIndex, nEndIndex );

    sal_uInt16 nLevel = GetLevelFromPos( mbHoriz ? rPos.Y() : rPos.X() );
    if ( nLevel == SC_OL_NOLEVEL )
        return false;

    sal_Int32 nLevelPos = GetLevelPos( nLevel );
    sal_Int32 nEntryMousePos = mbHoriz ? rPos.X() : rPos.Y();

    // --- level buttons ---

    if ( mnHeaderSize > 0 )
    {
        sal_Int32 nImagePos = GetHeaderEntryPos();
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
    sal_uInt16 nEntry = pArray->GetCount( nLevel );
    while ( nEntry )
    {
        --nEntry;

        const ScOutlineEntry* pEntry = pArray->GetEntry( nLevel, nEntry );
        sal_uInt16 nStart = pEntry->GetStart();
        sal_uInt16 nEnd = pEntry->GetEnd();

        if ( (nEnd >= nStartIndex) && (nStart <= nEndIndex) )
        {
            sal_Int32 nStartPos, nEndPos, nImagePos;
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

bool ScOutlineWindow::ButtonHit( const Point& rPos, sal_uInt16& rnLevel, sal_uInt16& rnEntry ) const
{
    bool bButton;
    bool bRet = ItemHit( rPos, rnLevel, rnEntry, bButton );
    return bRet && bButton;
}

bool ScOutlineWindow::LineHit( const Point& rPos, sal_uInt16& rnLevel, sal_uInt16& rnEntry ) const
{
    bool bButton;
    bool bRet = ItemHit( rPos, rnLevel, rnEntry, bButton );
    return bRet && !bButton;
}

void ScOutlineWindow::DoFunction( sal_uInt16 nLevel, sal_uInt16 nEntry ) const
{
    ScDBFunc& rFunc = *mrViewData.GetView();
    if ( nEntry == SC_OL_HEADERENTRY )
        rFunc.SelectLevel( mbHoriz, nLevel );
    else
    {
        const ScOutlineEntry* pEntry = GetOutlineEntry( nLevel, nEntry );
        if ( pEntry )
        {
            if ( pEntry->IsHidden() )
                rFunc.ShowOutline( mbHoriz, nLevel, nEntry );
            else
                rFunc.HideOutline( mbHoriz, nLevel, nEntry );
        }
    }
}

void ScOutlineWindow::DoExpand( sal_uInt16 nLevel, sal_uInt16 nEntry ) const
{
    const ScOutlineEntry* pEntry = GetOutlineEntry( nLevel, nEntry );
    if ( pEntry && pEntry->IsHidden() )
        DoFunction( nLevel, nEntry );
}

void ScOutlineWindow::DoCollapse( sal_uInt16 nLevel, sal_uInt16 nEntry ) const
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
        sal_Int32 nLevelStart, sal_Int32 nEntryStart,
        sal_Int32 nLevelEnd, sal_Int32 nEntryEnd )
{
    DrawLine( GetPoint( nLevelStart, nEntryStart ), GetPoint( nLevelEnd, nEntryEnd ) );
}

void ScOutlineWindow::DrawRectRel(
        sal_Int32 nLevelStart, sal_Int32 nEntryStart,
        sal_Int32 nLevelEnd, sal_Int32 nEntryEnd )
{
    DrawRect( GetRectangle( nLevelStart, nEntryStart, nLevelEnd, nEntryEnd ) );
}

void ScOutlineWindow::DrawImageRel( sal_Int32 nLevelPos, sal_Int32 nEntryPos, sal_uInt16 nId )
{
    DBG_ASSERT( mpSymbols, "ScOutlineWindow::DrawImageRel - no images" );
    const Image& rImage = mpSymbols->GetImage( nId );
    SetLineColor();
    SetFillColor( GetBackground().GetColor() );
    Point aPos( GetPoint( nLevelPos, nEntryPos ) );
    DrawRect( Rectangle( aPos, rImage.GetSizePixel() ) );
    DrawImage( aPos, rImage );
}

void ScOutlineWindow::DrawBorderRel( sal_uInt16 nLevel, sal_uInt16 nEntry, bool bPressed )
{
    Point aPos;
    if ( GetImagePos( nLevel, nEntry, aPos ) )
    {
        DBG_ASSERT( mpSymbols, "ScOutlineWindow::DrawBorderRel - no images" );
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

void ScOutlineWindow::Paint( const Rectangle& rRect )
{
    Size aSize = GetOutputSizePixel();
    sal_Int32 nLevelEnd = (mbHoriz ? aSize.Height() : aSize.Width()) - 1;
    sal_Int32 nEntryEnd = (mbHoriz ? aSize.Width() : aSize.Height()) - 1;

    SetLineColor( maLineColor );
    DrawLineRel( nLevelEnd, 0, nLevelEnd, nEntryEnd );

    const ScOutlineArray* pArray = GetOutlineArray();
    if ( !pArray ) return;

    sal_uInt16 nLevelCount = GetLevelCount();

    // --- draw header images ---

    if ( mnHeaderSize > 0 )
    {
        sal_Int32 nEntryPos = GetHeaderEntryPos();
        for ( sal_uInt16 nLevel = 0; nLevel < nLevelCount; ++nLevel )
            DrawImageRel( GetLevelPos( nLevel ), nEntryPos, nLevel + 1 );

        SetLineColor( maLineColor );
        sal_Int32 nLinePos = mnHeaderPos + (mbMirrorHdr ? 0 : (mnHeaderSize - 1));
        DrawLineRel( 0, nLinePos, nLevelEnd, nLinePos );
    }

    // --- draw lines & collapse/expand images ---

    SetEntryAreaClipRegion();

    sal_uInt16 nStartIndex, nEndIndex;
    GetVisibleRange( nStartIndex, nEndIndex );

    for ( sal_uInt16 nLevel = 0; nLevel < nLevelCount - 1; ++nLevel )
    {
        sal_Int32 nLevelPos = GetLevelPos( nLevel );
        sal_Int32 nEntryPos1, nEntryPos2, nImagePos;

        sal_uInt16 nEntryCount = pArray->GetCount( nLevel );
        sal_uInt16 nEntry;

        // first draw all lines in the current level
        SetLineColor();
        SetFillColor( maLineColor );
        for ( nEntry = 0; nEntry < nEntryCount; ++nEntry )
        {
            const ScOutlineEntry* pEntry = pArray->GetEntry( nLevel, nEntry );
            sal_uInt16 nStart = pEntry->GetStart();
            sal_uInt16 nEnd = pEntry->GetEnd();

            // visible range?
            bool bDraw = (nEnd >= nStartIndex) && (nStart <= nEndIndex);
            // find output coordinates
            if ( bDraw )
                bDraw = GetEntryPos( nLevel, nEntry, nEntryPos1, nEntryPos2, nImagePos );
            // draw, if not collapsed
            if ( bDraw && !pEntry->IsHidden() )
            {
                if ( nStart >= nStartIndex )
                    ++nEntryPos1;
                nEntryPos2 -= 2;
                DrawRectRel( nLevelPos, nEntryPos1, nLevelPos + 1, nEntryPos2 );

                if ( nEnd <= nEndIndex )
                    DrawRectRel( nLevelPos, nEntryPos2 - 1, nLevelPos + SC_OL_BITMAPSIZE / 3, nEntryPos2 );
            }
        }

        // draw all images in the level from last to first
        nEntry = nEntryCount;
        while ( nEntry )
        {
            --nEntry;

            const ScOutlineEntry* pEntry = pArray->GetEntry( nLevel, nEntry );
            sal_uInt16 nStart = pEntry->GetStart();
            sal_uInt16 nEnd = pEntry->GetEnd();

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
bool lcl_RotateValue( sal_uInt16& rnValue, sal_uInt16 nMin, sal_uInt16 nMax, bool bForward )
{
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
    sal_uInt16 nEntryCount = pArray->GetCount( mnFocusLevel );
    sal_uInt16 nOldEntry = mnFocusEntry;

    do
    {
        if ( mnFocusEntry == SC_OL_HEADERENTRY )
        {
            // move from header to first or last entry
            if ( nEntryCount > 0 )
                mnFocusEntry = bForward ? 0 : (nEntryCount - 1);
            /*  wrapped, if forward from right header to first entry,
                or if backward from left header to last entry */
            if ( !nEntryCount || (bForward == mbMirrorHdr) )
                bWrapped = true;
        }
        else if ( lcl_RotateValue( mnFocusEntry, 0, nEntryCount - 1, bForward ) )
        {
            // lcl_RotateValue returns true -> wrapped the entry range -> move to header
            mnFocusEntry = SC_OL_HEADERENTRY;
            /*  wrapped, if forward from last entry to left header,
                or if backward from first entry to right header */
            if ( bForward != mbMirrorHdr )
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
    sal_uInt16 nLevelCount = GetLevelCount();

    if ( mnFocusEntry == SC_OL_HEADERENTRY )
    {
        if ( nLevelCount > 0 )
            bWrapped = lcl_RotateValue( mnFocusLevel, 0, nLevelCount - 1, bForward );
    }
    else
    {
        const ScOutlineEntry* pEntry = pArray->GetEntry( mnFocusLevel, mnFocusEntry );
        if ( pEntry )
        {
            sal_uInt16 nStart = pEntry->GetStart();
            sal_uInt16 nEnd = pEntry->GetEnd();
            sal_uInt16 nNewLevel = mnFocusLevel;
            sal_uInt16 nNewEntry;

            bool bFound = false;
            if ( bForward && (mnFocusLevel + 2 < nLevelCount) )
            {
                // next level -> find first child entry
                nNewLevel = mnFocusLevel + 1;
                bFound = !!pArray->GetEntryIndexInRange( nNewLevel, nStart, nEnd, nNewEntry );
            }
            else if ( !bForward && (mnFocusLevel > 0) )
            {
                // previous level -> find parent entry
                nNewLevel = mnFocusLevel - 1;
                bFound = !!pArray->GetEntryIndex( nNewLevel, nStart, nNewEntry );
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
    sal_uInt16 nOldLevel = mnFocusLevel;
    sal_uInt16 nOldEntry = mnFocusEntry;

    do
    {
        /*  one level up, if backward from left header,
            or one level down, if forward from right header */
        if ( (bForward == mbMirrorHdr) && (mnFocusEntry == SC_OL_HEADERENTRY) )
            bRet |= ImplMoveFocusByLevel( bForward );
        // move to next/previous entry
        bool bWrapInLevel = ImplMoveFocusByEntry( bForward, false );
        bRet |= bWrapInLevel;
        /*  one level up, if wrapped backward to right header,
            or one level down, if wrapped forward to right header */
        if ( (bForward != mbMirrorHdr) && bWrapInLevel )
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

void ScOutlineWindow::StartMouseTracking( sal_uInt16 nLevel, sal_uInt16 nEntry )
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
        sal_uInt16 nLevel, nEntry;
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

        sal_uInt16 nLevel, nEntry;
        if ( ButtonHit( rMEvt.GetPosPixel(), nLevel, nEntry ) )
            if ( (nLevel == mnMTLevel) && (nEntry == mnMTEntry) )
                DoFunction( nLevel, nEntry );
    }
}

void ScOutlineWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    sal_uInt16 nLevel, nEntry;
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
        MoveFocusByTabOrder( bNoMod );

    // LEFT/RIGHT/UP/DOWN keys
    else if ( bNoMod && (bUpDownKey || bLeftRightKey) )
    {
        bool bForward = (nCode == KEY_DOWN) || (nCode == KEY_RIGHT);
        if ( mbHoriz == bLeftRightKey )
            // move inside level with LEFT/RIGHT in horizontal and with UP/DOWN in vertical
            MoveFocusByEntry( bForward );
        else
            // move to next/prev level with LEFT/RIGHT in vertical and with UP/DOWN in horizontal
            MoveFocusByLevel( bForward );
    }

    // CTRL + number
    else if ( bCtrl && (nCode >= KEY_1) && (nCode <= KEY_9) )
    {
        sal_uInt16 nLevel = nCode - KEY_1;
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

