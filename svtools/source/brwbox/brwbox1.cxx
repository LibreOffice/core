/*************************************************************************
 *
 *  $RCSfile: brwbox1.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2000-11-03 18:24:29 $
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

#include <brwbox.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <vcl/sound.hxx>
#include "brwhead.hxx"
#include "datwin.hxx"
#include <string>
#include <algorithm>

#pragma hdrstop

#ifndef _SV_MULTISEL_HXX
#include <tools/multisel.hxx>
#endif

DBG_NAME(BrowseBox);

extern const char* BrowseBoxCheckInvariants( const void* pVoid );

DECLARE_LIST( BrowserColumns, BrowserColumn* );

#ifdef VCL
#define SCROLL_FLAGS (SCROLL_CLIP | SCROLL_NOCHILDREN)
#else
#define SCROLL_FLAGS TRUE
#endif

//-------------------------------------------------------------------

#if DBG_MI
void DoLog_Impl( const BrowseBox *pThis, const char *pWhat, const char *pWho )
{
    SvFileStream aLog( "d:\\cursor.log", STREAM_WRITE|STREAM_NOCREATE );
    if ( aLog.IsOpen() )
    {
        aLog.Seek( STREAM_SEEK_TO_END );
        String aEntry( (long) pThis );
        aEntry += "(row=";
        aEntry += pThis->GetCurRow();
        aEntry += "): ";
        aEntry += pWhat;
        aEntry += " from ";
        aEntry += pWho;
        aEntry += " => ";
        aEntry += pThis->GetCursorHideCount();
        aLog.WriteLine( aEntry );
    }
}
#endif

//===================================================================

void BrowseBox::Construct( BrowserMode nMode )
{
    DBG_TRACE1( "BrowseBox: %p->Construct", this );
    bMultiSelection = FALSE;
    pColSel = 0;
    pDataWin = 0;
    pVScroll = 0;

    pDataWin = new BrowserDataWin( this );
    pCols = new BrowserColumns;

    aLineColor = Color( COL_LIGHTGRAY );
    InitSettings_Impl( this );
    InitSettings_Impl( pDataWin );

    bBootstrapped = FALSE;
    bHasBitmapHandle = FALSE;
    nDataRowHeight = 0;
    nTitleLines = 1;
    nFirstCol = 0;
    nTopRow = 0;
    nCurRow = BROWSER_ENDOFSELECTION;
    nCurColId = 0;
    bResizing = FALSE;
    bSelect = FALSE;
    bSelecting = FALSE;
    bScrolling = FALSE;
    bSelectionIsVisible = FALSE;
    bNotToggleSel = FALSE;
    bDrag = FALSE;
    bHit = FALSE;
    bRubber = FALSE;
    bHideSelect = FALSE;
    bHideCursor = FALSE;
    nRowCount = 0;
    m_bFocusOnlyCursor = TRUE;
    m_aCursorColor = COL_TRANSPARENT;
    m_nCurrentMode = 0;

    aHScroll.SetLineSize(1);
    aHScroll.SetScrollHdl( LINK( this, BrowseBox, ScrollHdl ) );
    aHScroll.SetEndScrollHdl( LINK( this, BrowseBox, EndScrollHdl ) );
    pDataWin->Show();

    SetMode( nMode );
    bSelectionIsVisible = bKeepHighlight;
    bHasFocus = HasChildPathFocus();
    ((BrowserDataWin*)pDataWin)->nCursorHidden =
                ( bHasFocus ? 0 : 1 ) + ( GetUpdateMode() ? 0 : 1 );
    LOG( this, "Construct", "*" );
}

//-------------------------------------------------------------------

BrowseBox::BrowseBox( Window* pParent, WinBits nBits, BrowserMode nMode ) :
    Control( pParent, nBits | WB_3DLOOK ),
    aHScroll( this, WinBits( WB_HSCROLL ) )
{
    DBG_CTOR( BrowseBox, NULL );
    Construct( nMode );
}

//-------------------------------------------------------------------

BrowseBox::BrowseBox( Window* pParent, const ResId& rId, BrowserMode nMode ):
    Control( pParent, rId ),
    aHScroll( this, WinBits(WB_HSCROLL) )
{
    DBG_CTOR( BrowseBox, NULL );
    Construct(nMode);
}

//-------------------------------------------------------------------

BrowseBox::~BrowseBox()
{
    DBG_DTOR(BrowseBox,BrowseBoxCheckInvariants);
    DBG_TRACE1( "BrowseBox: %p~", this );

    Hide();
    delete ((BrowserDataWin*)pDataWin)->pHeaderBar;
    delete ((BrowserDataWin*)pDataWin)->pCornerWin;
    delete pDataWin;
    delete pVScroll;

    // free columns-space
    for ( USHORT n = 0; n < pCols->Count(); ++n )
        delete pCols->GetObject(n);
    delete pCols;
    delete pColSel;
    if ( bMultiSelection )
        delete uRow.pSel;
}

//-------------------------------------------------------------------

short BrowseBox::GetCursorHideCount() const
{
    return ((BrowserDataWin*)pDataWin)->nCursorHidden;
}

//-------------------------------------------------------------------

void BrowseBox::DoShowCursor( const char *pWhoLogs )
{
    short nHiddenCount = --((BrowserDataWin*)pDataWin)->nCursorHidden;
    if (PaintCursorIfHiddenOnce())
    {
        if (1 == nHiddenCount)
            DrawCursor();
    }
    else
    {
        if (0 == nHiddenCount)
            DrawCursor();
    }
    LOG( this, "DoShowCursor", pWhoLogs );
}

//-------------------------------------------------------------------

void BrowseBox::DoHideCursor( const char *pWhoLogs )
{
    short nHiddenCount = ++((BrowserDataWin*)pDataWin)->nCursorHidden;
    if (PaintCursorIfHiddenOnce())
    {
        if (2 == nHiddenCount)
            DrawCursor();
    }
    else
    {
        if (1 == nHiddenCount)
            DrawCursor();
    }
    LOG( this, "DoHideCursor", pWhoLogs );
}

//-------------------------------------------------------------------

void BrowseBox::SetRealRowCount( const String &rRealRowCount )
{
    ((BrowserDataWin*)pDataWin)->aRealRowCount = rRealRowCount;
}

//-------------------------------------------------------------------

void BrowseBox::SetMapMode( const MapMode& rNewMapMode )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    pDataWin->SetMapMode( rNewMapMode );
}

//-------------------------------------------------------------------

void BrowseBox::SetFont( const Font& rNewFont )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    pDataWin->SetFont( rNewFont );
    ImpGetDataRowHeight();
}

//-------------------------------------------------------------------

void BrowseBox::InsertHandleColumn( ULONG nWidth, BOOL bBitmap )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    pCols->Insert( new BrowserColumn( 0, Image(), String(), nWidth, GetZoom(), 0 ), (ULONG) 0 );
    FreezeColumn( 0 );

    // Headerbar anpassen
    if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
    {
        ((BrowserDataWin*)pDataWin)->pHeaderBar->SetPosPixel(
                    Point(nWidth, 0));

        ((BrowserDataWin*)pDataWin)->pHeaderBar->SetSizePixel(
                    Size( GetOutputSizePixel().Width() - nWidth, GetTitleHeight() ) );
    }

    /*if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
        ((BrowserDataWin*)pDataWin)->pHeaderBar->InsertItem( USHRT_MAX - 1,
                "", nWidth, HIB_FIXEDPOS|HIB_FIXED, 0 );*/
    ColumnInserted( 0 );
    bHasBitmapHandle = bBitmap;
}

//-------------------------------------------------------------------
#if SUPD<380
void BrowseBox::InsertDataColumn( USHORT nItemId, const String &rTitle,
                    ULONG nWidth, BrowserColumnMode nFlags, USHORT nPos )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    pCols->Insert( new BrowserColumn( nItemId, Image(), rTitle, nWidth, GetZoom(), nFlags ),
                           Min( nPos, (USHORT)(pCols->Count()) ) );
    if ( nCurColId == 0 )
        nCurColId = nItemId;
    if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
        ((BrowserDataWin*)pDataWin)->pHeaderBar->InsertItem(
                nItemId, rTitle, nWidth, HIB_STDSTYLE, nPos );
    ColumnInserted( nPos );
}
#else
//-------------------------------------------------------------------

void BrowseBox::InsertDataColumn( USHORT nItemId, const Image& rImage,
        long nWidth, HeaderBarItemBits nBits, USHORT nPos )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    pCols->Insert( new BrowserColumn( nItemId, rImage, String(), nWidth, GetZoom(), nBits ),
                           Min( nPos, (USHORT)(pCols->Count()) ) );
    if ( nCurColId == 0 )
        nCurColId = nItemId;
    if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
    {
        // Handlecolumn nicht in der Headerbar
        USHORT nHeaderPos = nPos;
        if (nHeaderPos != HEADERBAR_APPEND && !GetColumnId(0))
            nHeaderPos--;
        ((BrowserDataWin*)pDataWin)->pHeaderBar->InsertItem(
                nItemId, rImage, nWidth, nBits, nHeaderPos );
    }
    ColumnInserted( nPos );
}

//-------------------------------------------------------------------

void BrowseBox::InsertDataColumn( USHORT nItemId, const XubString& rText,
        long nWidth, HeaderBarItemBits nBits, USHORT nPos )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    pCols->Insert( new BrowserColumn( nItemId, Image(), rText, nWidth, GetZoom(), nBits ),
                           Min( nPos, (USHORT)(pCols->Count()) ) );
    if ( nCurColId == 0 )
        nCurColId = nItemId;

    if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
    {
        // Handlecolumn nicht in der Headerbar
        USHORT nHeaderPos = nPos;
        if (nHeaderPos != HEADERBAR_APPEND && !GetColumnId(0))
            nHeaderPos--;
        ((BrowserDataWin*)pDataWin)->pHeaderBar->InsertItem(
                nItemId, rText, nWidth, nBits, nHeaderPos );
    }
    ColumnInserted( nPos );
}

//-------------------------------------------------------------------

void BrowseBox::InsertDataColumn( USHORT nItemId,
        const Image& rImage, const XubString& rText,
        long nWidth, HeaderBarItemBits nBits, USHORT nPos,
        const String* pHelpText )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    pCols->Insert( new BrowserColumn( nItemId, rImage, rText, nWidth, GetZoom(), nBits ),
                           Min( nPos, (USHORT)(pCols->Count()) ) );
    if ( nCurColId == 0 )
        nCurColId = nItemId;
    if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
    {
        // Handlecolumn nicht in der Headerbar
        USHORT nHeaderPos = nPos;
        if (nHeaderPos != HEADERBAR_APPEND && !GetColumnId(0))
            nHeaderPos--;

        ((BrowserDataWin*)pDataWin)->pHeaderBar->InsertItem(
                nItemId, rImage, rText, nWidth, nBits, nHeaderPos );
        if( pHelpText && !rText.Len() )
        {
            ((BrowserDataWin*)pDataWin)->pHeaderBar->SetHelpText(
                nItemId, *pHelpText );
        }
    }
    ColumnInserted( nPos );
}
#endif
//-------------------------------------------------------------------

void BrowseBox::FreezeColumn( USHORT nItemId, BOOL bFreeze )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // never unfreeze the handle-column
    if ( nItemId == 0 && !bFreeze )
        return;

    // get the position in the current array
    USHORT nItemPos = GetColumnPos( nItemId );
    if ( nItemPos >= pCols->Count() )
        // not available!
        return;

    // doesn't the state change?
    if ( pCols->GetObject(nItemPos)->IsFrozen() == bFreeze )
        return;

    // remark the column selection
    USHORT nSelectedColId = USHRT_MAX;
    if ( pColSel && pColSel->GetSelectCount() )
    {
        DoHideCursor( "FreezeColumn" );
        ToggleSelection();
        nSelectedColId = pCols->GetObject(pColSel->FirstSelected())->GetId();
        pColSel->SelectAll(FALSE);
    }

    // freeze or unfreeze?
    if ( bFreeze )
    {
        // to be moved?
        if ( nItemPos != 0 && !pCols->GetObject(nItemPos-1)->IsFrozen() )
        {
            // move to the right of the last frozen column
            USHORT nFirstScrollable = FrozenColCount();
            BrowserColumn *pColumn = pCols->GetObject(nItemPos);
            pCols->Remove( (ULONG) nItemPos );
            nItemPos = nFirstScrollable;
            pCols->Insert( pColumn, (ULONG) nItemPos );
        }

        // adjust the number of the first scrollable and visible column
        if ( nFirstCol <= nItemPos )
            nFirstCol = nItemPos + 1;
    }
    else
    {
        // to be moved?
        if ( nItemPos != FrozenColCount()-1 )
        {
            // move to the leftmost scrollable colum
            USHORT nFirstScrollable = FrozenColCount();
            BrowserColumn *pColumn = pCols->GetObject(nItemPos);
            pCols->Remove( (ULONG) nItemPos );
            nItemPos = nFirstScrollable;
            pCols->Insert( pColumn, (ULONG) nItemPos );
        }

        // adjust the number of the first scrollable and visible column
        nFirstCol = nItemPos;
    }

    // toggle the freeze-state of the column
    pCols->GetObject(nItemPos)->Freeze( bFreeze );

    // align the scrollbar-range
    UpdateScrollbars();

    // repaint
    Control::Invalidate();
    ((BrowserDataWin*)pDataWin)->Invalidate();

    // remember the column selection
    if ( pColSel && nSelectedColId != USHRT_MAX )
    {
        pColSel->Select( GetColumnPos( nSelectedColId ) );
        ToggleSelection();
        DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );
        DoShowCursor( "FreezeColumn" );
    }
}

//-------------------------------------------------------------------

void BrowseBox::SetColumnPos( USHORT nColumnId, USHORT nPos )
{
    // never set pos of the handle-column
    if ( nColumnId == 0 )
        return;

    // do not move handle column
    if (nPos == 0 && !pCols->GetObject(0)->GetId())
        return;

    // get the position in the current array
    USHORT nOldPos = GetColumnPos( nColumnId );
    if ( nOldPos >= pCols->Count() )
        // not available!
        return;

    // does the state change?
    BrowserColumn *pCol = pCols->GetObject(nOldPos);
    if (nOldPos != nPos)
    {
        // determine old column area
        Size aDataWinSize( pDataWin->GetSizePixel() );
        Rectangle aFromRect( GetFieldRect( nColumnId) );

        // move column internally
        pCols->Insert( pCols->Remove( nOldPos ), nPos );

        // determine new column area
        Rectangle aToRect( GetFieldRect( nColumnId ) );

        // do scroll, let redraw
        Rectangle aForNewArea( Point( aToRect.Left(), 0 ),
                    Size( aDataWinSize.Width() - aToRect.Left(),
                          aDataWinSize.Height() ) );
        Rectangle aForOldArea( Point( aFromRect.Right(), 0 ),
                    Size( aDataWinSize.Width() - aFromRect.Right(),
                          aDataWinSize.Height() ) );

        if( pDataWin->GetBackground().IsScrollable() )
        {
            if ( nOldPos > nPos )
                pDataWin->Scroll( -aFromRect.GetWidth()-4, 0, aForOldArea );
            pDataWin->Scroll( aToRect.GetWidth()+4, 0, aForNewArea );
            if ( nOldPos < nPos )
                pDataWin->Scroll( -aFromRect.GetWidth()-4, 0, aForOldArea );
        }
        else
            pDataWin->Window::Invalidate( INVALIDATE_NOCHILDREN );
    }

}

//-------------------------------------------------------------------

void BrowseBox::SetColumnMode( USHORT nColumnId, BrowserColumnMode nFlags )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // never set mode of the handle-column
    if ( nColumnId == 0 )
        return;

    // get the position in the current array
    USHORT nColumnPos = GetColumnPos( nColumnId );
    if ( nColumnPos >= pCols->Count() )
        // not available!
        return;

    // does the state change?
    BrowserColumn *pCol = pCols->GetObject(nColumnPos);
    if ( pCol->Flags() != nFlags )
    {
        pCol->Flags() = nFlags;

        // redraw visible colums
        if ( GetUpdateMode() && ( pCol->IsFrozen() || nColumnPos > nFirstCol ) )
            Invalidate( Rectangle( Point(0,0),
                Size( GetOutputSizePixel().Width(), GetTitleHeight() ) ) );
    }
}

//-------------------------------------------------------------------

void BrowseBox::SetColumnTitle( USHORT nItemId, const String& rTitle )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // never set title of the handle-column
    if ( nItemId == 0 )
        return;

    // get the position in the current array
    USHORT nItemPos = GetColumnPos( nItemId );
    if ( nItemPos >= pCols->Count() )
        // not available!
        return;

    // does the state change?
    BrowserColumn *pCol = pCols->GetObject(nItemPos);
    if ( pCol->Title() != rTitle )
    {
        pCol->Title() = rTitle;

        // Headerbar-Column anpassen
        if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
            ((BrowserDataWin*)pDataWin)->pHeaderBar->SetItemText(
                    nItemId ? nItemId : USHRT_MAX - 1, rTitle );
        else
        {
            // redraw visible colums
            if ( GetUpdateMode() && ( pCol->IsFrozen() || nItemPos > nFirstCol ) )
                Invalidate( Rectangle( Point(0,0),
                    Size( GetOutputSizePixel().Width(), GetTitleHeight() ) ) );
        }
    }
}

//-------------------------------------------------------------------

void BrowseBox::SetColumnWidth( USHORT nItemId, ULONG nWidth )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // get the position in the current array
    USHORT nItemPos = GetColumnPos( nItemId );
    if ( nItemPos >= pCols->Count() )
        return;

    // does the state change?
    nWidth = QueryColumnResize( nItemId, nWidth );
    if ( nWidth >= LONG_MAX || pCols->GetObject(nItemPos)->Width() != nWidth )
    {
        long nOldWidth = pCols->GetObject(nItemPos)->Width();

        // ggf. letzte Spalte anpassen
        if ( IsVisible() && nItemPos == pCols->Count() - 1 )
        {
            long nMaxWidth = pDataWin->GetSizePixel().Width();
            nMaxWidth -= ((BrowserDataWin*)pDataWin)->bAutoSizeLastCol
                    ? GetFieldRect(nItemId).Left()
                    : GetFrozenWidth();
            if ( ( (BrowserDataWin*)pDataWin )->bAutoSizeLastCol || nWidth > (ULONG)nMaxWidth )
            {
                nWidth = nMaxWidth > 16 ? nMaxWidth : nOldWidth;
                nWidth = QueryColumnResize( nItemId, nWidth );
            }
        }

        // OV
        // In AutoSizeLastColumn() wird SetColumnWidth mit nWidth==0xffff
        // gerufen. Deshalb muss hier nochmal geprueft werden, ob sich die
        // Breite tatsaechlich geaendert hat.
        if( (ULONG)nOldWidth == nWidth )
            return;

        // soll die Aenderung sofort dargestellt werden?
        BOOL bUpdate = GetUpdateMode() &&
                       ( pCols->GetObject(nItemPos)->IsFrozen() || nItemPos >= nFirstCol );

        if ( bUpdate )
        {
            // Selection hiden
            DoHideCursor( "SetColumnWidth" );
            ToggleSelection();
            //!((BrowserDataWin*)pDataWin)->Update();
            //!Control::Update();
        }

        // Breite setzen
        pCols->GetObject(nItemPos)->SetWidth(nWidth, GetZoom());
#if 0
        if ( nItemPos != pCols->Count() - 1 )
        {
            long nLastColMaxWidth = pDataWin->GetSizePixel().Width() -
                        GetFieldRect(GetColumnId(pCols->Count()-1)).Left();
            pCols->GetObject(pCols->Count()-1)->Width() = nLastColMaxWidth;
        }
#endif

        // scroll and invalidate
        if ( bUpdate )
        {
            // X-Pos der veraenderten Spalte ermitteln
            long nX = 0;
            for ( USHORT nCol = 0; nCol < nItemPos; ++nCol )
            {
                BrowserColumn *pCol = pCols->GetObject(nCol);
                if ( pCol->IsFrozen() || nCol >= nFirstCol )
                    nX += pCol->Width();
            }

            // eigentliches scroll+invalidate
            pDataWin->SetClipRegion();
            BOOL bSelVis = bSelectionIsVisible;
            bSelectionIsVisible = FALSE;
            if( GetBackground().IsScrollable() )
            {

                Rectangle aScrRect( nX + std::min( (ULONG)nOldWidth, nWidth ), 0,
                                    GetSizePixel().Width() , // the header is longer than the datawin
                                    pDataWin->GetPosPixel().Y() - 1 );
                Control::Scroll( nWidth-nOldWidth, 0, aScrRect, SCROLL_FLAGS );
                aScrRect.Bottom() = pDataWin->GetSizePixel().Height();
                ((BrowserDataWin*)pDataWin)->Scroll( nWidth-nOldWidth, 0, aScrRect, SCROLL_FLAGS );
                Rectangle aInvRect( nX, 0, nX + std::max( nWidth, (ULONG)nOldWidth ), USHRT_MAX );
                Control::Invalidate( aInvRect );
                ( (BrowserDataWin*)pDataWin )->Invalidate( aInvRect );
            }
            else
            {
                Control::Invalidate( INVALIDATE_NOCHILDREN );
                ((BrowserDataWin*)pDataWin)->Window::Invalidate( INVALIDATE_NOCHILDREN );
            }


            //!((BrowserDataWin*)pDataWin)->Update();
            //!Control::Update();
            bSelectionIsVisible = bSelVis;
            ToggleSelection();
            DoShowCursor( "SetColumnWidth" );
        }
        UpdateScrollbars();

        // Headerbar-Column anpassen
        if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
            ((BrowserDataWin*)pDataWin)->pHeaderBar->SetItemSize(
                    nItemId ? nItemId : USHRT_MAX - 1, nWidth );

        // adjust last column
        if ( nItemPos != pCols->Count() - 1 )
            AutoSizeLastColumn();
    }
}

//-------------------------------------------------------------------

void BrowseBox::AutoSizeLastColumn()
{
    if ( ((BrowserDataWin*)pDataWin)->bAutoSizeLastCol &&
         ((BrowserDataWin*)pDataWin)->GetUpdateMode() )
    {
        USHORT nId = GetColumnId( (USHORT)pCols->Count() - 1 );
        SetColumnWidth( nId, LONG_MAX );
        ColumnResized( nId );
    }
}

//-------------------------------------------------------------------

void BrowseBox::RemoveColumn( USHORT nItemId )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // Spaltenposition ermitteln
    USHORT nPos = GetColumnPos(nItemId);
    if ( nPos >= ColCount() )
        // nicht vorhanden
        return;

    // Spaltenselektion korrigieren
    if ( pColSel )
        pColSel->Remove( nPos );

    // Spaltencursor korrigieren
    if ( nCurColId == nItemId )
        nCurColId = 0;

    if ( nFirstCol >= nPos && nFirstCol > 0 )
        --nFirstCol;

    // Spalte entfernen
    delete( pCols->Remove( (ULONG) nPos ));

    // Handlecolumn nicht in der Headerbar
    if (nItemId)
    {
        if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
            ((BrowserDataWin*)pDataWin)->pHeaderBar->RemoveItem( nItemId );
    }
    else
    {
        // Headerbar anpassen
        if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
        {
            ((BrowserDataWin*)pDataWin)->pHeaderBar->SetPosPixel(
                        Point(0, 0));

            ((BrowserDataWin*)pDataWin)->pHeaderBar->SetSizePixel(
                        Size( GetOutputSizePixel().Width(), GetTitleHeight() ) );
        }
    }

    // vertikalen Scrollbar korrigieren
    UpdateScrollbars();

    // ggf. Repaint ausl"osen
    if ( GetUpdateMode() )
    {
        ((BrowserDataWin*)pDataWin)->Invalidate();
        Control::Invalidate();
        if ( ((BrowserDataWin*)pDataWin)->bAutoSizeLastCol && nPos ==ColCount() )
            SetColumnWidth( GetColumnId( nPos - 1 ), LONG_MAX );
    }
}

//-------------------------------------------------------------------

void BrowseBox::RemoveColumns()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // alle Spalten entfernen
    while ( pCols->Count() )
        delete ( pCols->Remove( (ULONG) 0 ));

    // Spaltenselektion korrigieren
    if ( pColSel )
    {
        pColSel->SelectAll(FALSE);
        pColSel->SetTotalRange( Range( 0, 0 ) );
    }

    // Spaltencursor korrigieren
    nCurColId = 0;
    nFirstCol = 0;

    if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
        ((BrowserDataWin*)pDataWin)->pHeaderBar->Clear( );

    // vertikalen Scrollbar korrigieren
    UpdateScrollbars();

    // ggf. Repaint ausl"osen
    if ( GetUpdateMode() )
    {
        ((BrowserDataWin*)pDataWin)->Invalidate();
        Control::Invalidate();
    }
}

//-------------------------------------------------------------------

String BrowseBox::GetColumnTitle( USHORT nId ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    USHORT nItemPos = GetColumnPos( nId );
    if ( nItemPos >= pCols->Count() )
        return String();
    return pCols->GetObject(nItemPos)->Title();
}

//-------------------------------------------------------------------

long BrowseBox::GetRowCount() const
{
    return nRowCount;
}

//-------------------------------------------------------------------

USHORT BrowseBox::ColCount() const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return (USHORT) pCols->Count();
}

//-------------------------------------------------------------------

long BrowseBox::ImpGetDataRowHeight() const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    BrowseBox *pThis = (BrowseBox*)this;
    pThis->nDataRowHeight = pThis->CalcReverseZoom(pDataWin->GetTextHeight() + 2);
    pThis->Resize();
    ((BrowserDataWin*)pDataWin)->Invalidate();
    return nDataRowHeight;
}

//-------------------------------------------------------------------

void BrowseBox::SetDataRowHeight( long nPixel )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    nDataRowHeight = CalcReverseZoom(nPixel);
    Resize();
    ((BrowserDataWin*)pDataWin)->Invalidate();
}

//-------------------------------------------------------------------

void BrowseBox::SetTitleLines( USHORT nLines )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    nTitleLines = nLines;
}

//-------------------------------------------------------------------

void BrowseBox::ToTop()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}

//-------------------------------------------------------------------

void BrowseBox::ToBottom()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}

//-------------------------------------------------------------------

long BrowseBox::ScrollColumns( long nCols )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( nFirstCol + nCols < 0 ||
         nFirstCol + nCols >= (long)pCols->Count() )
         //?MI: pCols->GetObject( nFirstCol + nCols )->IsFrozen() )
        return 0;

    // implicitly hides cursor while scrolling
    StartScroll();
    bScrolling = TRUE;
    BOOL bScrollable = pDataWin->GetBackground().IsScrollable();
    BOOL bInvalidateView = FALSE;

    // eine Spalte nach links scrollen?
    if ( nCols == 1 )
    {
        // update internal value and scrollbar
        ++nFirstCol;
        aHScroll.SetThumbPos( nFirstCol - FrozenColCount() );

        long nDelta = pCols->GetObject(nFirstCol-1)->Width();
        long nFrozenWidth = GetFrozenWidth();

        // scroll the title-line
        Rectangle aScrollRect(
                Point( nFrozenWidth + nDelta, 0 ),
                Size( GetOutputSizePixel().Width() - nFrozenWidth - nDelta,
                      GetTitleHeight() - 1 ) );

        // ggf. Headerbar mitscrollen
        if ( !((BrowserDataWin*)pDataWin)->pHeaderBar && nTitleLines )
        {
            if( bScrollable )
                Scroll( -nDelta, 0, aScrollRect );
            else
                bInvalidateView = TRUE;
        }


        long nSkippedWidth = GetOutputSizePixel().Width() -
            2 * aScrollRect.GetWidth() - nFrozenWidth;
        if ( nSkippedWidth > 0 )
        {
            aScrollRect.Right() = aScrollRect.Left()-1;
            aScrollRect.Left() -= nSkippedWidth;
            Invalidate( aScrollRect );
        }

        // scroll the data-area
        aScrollRect = Rectangle(
                Point( nFrozenWidth + nDelta, 0 ),
                Size( pDataWin->GetOutputSizePixel().Width() - nFrozenWidth -
                      nDelta, pDataWin->GetSizePixel().Height() ) );
        if( bScrollable )
            pDataWin->Scroll( -nDelta, 0, aScrollRect );
        else
            bInvalidateView = TRUE;
        nSkippedWidth = pDataWin->GetOutputSizePixel().Width() -
            2 * aScrollRect.GetWidth() - nFrozenWidth;
        if ( nSkippedWidth > 0 )
        {
            aScrollRect.Right() = aScrollRect.Left()-1;
            aScrollRect.Left() -= nSkippedWidth;
            ((BrowserDataWin*)pDataWin)->Invalidate( aScrollRect );
        }
    }

    // eine Spalte nach rechts scrollen?
    else if ( nCols == -1 )
    {
        --nFirstCol;
        aHScroll.SetThumbPos( nFirstCol - FrozenColCount() );

        long nDelta = pCols->GetObject(nFirstCol)->Width();
        long nFrozenWidth = GetFrozenWidth();

        // ggf. Headerbar mitscrollen
        if ( !((BrowserDataWin*)pDataWin)->pHeaderBar && nTitleLines )
        {
            if( bScrollable )
            {
                Scroll( nDelta, 0, Rectangle(
                    Point( nFrozenWidth, 0 ),
                    Size( GetOutputSizePixel().Width() - nFrozenWidth,
                        GetTitleHeight() - 1 ) ) );
            }
            else
                bInvalidateView = TRUE;
        }
        if( bScrollable )
        {
            pDataWin->Scroll( nDelta, 0, Rectangle(
                Point( nFrozenWidth, 0 ),
                Size( pDataWin->GetSizePixel().Width() - nFrozenWidth,
                    pDataWin->GetSizePixel().Height() ) ) );
        }
        else
            bInvalidateView = TRUE;

    }
    else
    {
        if ( GetUpdateMode() )
        {
            Invalidate( Rectangle(
                Point( GetFrozenWidth(), 0 ),
                Size( GetOutputSizePixel().Width(), GetTitleHeight() ) ) );
            ((BrowserDataWin*)pDataWin)->Invalidate( Rectangle(
                Point( GetFrozenWidth(), 0 ),
                pDataWin->GetSizePixel() ) );
        }

        nFirstCol += (USHORT)nCols;
        aHScroll.SetThumbPos( nFirstCol - FrozenColCount() );
    }

    // ggf. externe Headerbar anpassen
    if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
    {
        long nWidth = 0;
        for ( USHORT nCol = 0;
              nCol < pCols->Count() && nCol < nFirstCol;
              ++nCol )
        {
            // HandleColumn nicht
            if ( pCols->GetObject(nCol)->GetId() )
                nWidth += pCols->GetObject(nCol)->Width();
        }

        ((BrowserDataWin*)pDataWin)->pHeaderBar->SetOffset( nWidth );
    }

    if( bInvalidateView )
    {
        Control::Invalidate( INVALIDATE_NOCHILDREN );
        pDataWin->Window::Invalidate( INVALIDATE_NOCHILDREN );
    }

    // implicitly show cursor after scrolling
    if ( nCols )
    {
        ((BrowserDataWin*)pDataWin)->Update();
        Update();
    }
    bScrolling = FALSE;
    EndScroll();

    return nCols;
}

//-------------------------------------------------------------------

long BrowseBox::ScrollRows( long nRows )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // out of range?
    if ( ((BrowserDataWin*)pDataWin)->bNoScrollBack && nRows < 0 )
        return 0;

    // compute new top row
    long nTmpMin = Min( (long)(nTopRow + nRows), (long)(nRowCount - 1) );

    long nNewTopRow = Max( (long)nTmpMin, (long)0 );

    if ( nNewTopRow == nTopRow )
        return 0;

    USHORT nVisibleRows =
        (USHORT)(pDataWin->GetOutputSizePixel().Height() / GetDataRowHeight() + 1);

    VisibleRowsChanged(nNewTopRow, nVisibleRows);

    // compute new top row again (nTopRow might have changed!)
    nTmpMin = Min( (long)(nTopRow + nRows), (long)(nRowCount - 1) );

    nNewTopRow = Max( (long)nTmpMin, (long)0 );

    StartScroll();

    // scroll area on screen and/or repaint
    long nDeltaY = GetDataRowHeight() * ( nNewTopRow - nTopRow );
    long nOldTopRow = nTopRow;
    nTopRow = nNewTopRow;

    if ( GetUpdateMode() )
    {
        pVScroll->SetRange( Range( 0L, nRowCount ) );
        pVScroll->SetThumbPos( nTopRow );

        if( pDataWin->GetBackground().IsScrollable() &&
            Abs( nDeltaY ) > 0 &&
            Abs( nDeltaY ) < pDataWin->GetSizePixel().Height() )
        {
            pDataWin->Scroll( 0, (short)-nDeltaY );
        }
        else
            ((BrowserDataWin*)pDataWin)->Invalidate();

        if ( nTopRow - nOldTopRow )
            ((BrowserDataWin*)pDataWin)->Update();
    }

    EndScroll();

    return nTopRow - nOldTopRow;
}

//-------------------------------------------------------------------

long BrowseBox::ScrollPages( long nPagesY )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return ScrollRows( pDataWin->GetSizePixel().Height() / GetDataRowHeight() );
}

//-------------------------------------------------------------------

void BrowseBox::RowModified( long nRow, USHORT nColId )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( !GetUpdateMode() )
        return;

    Rectangle aRect;
    if ( nColId == USHRT_MAX )
        // invalidate the whole row
        aRect = Rectangle( Point( 0, (nRow-nTopRow) * GetDataRowHeight() ),
                    Size( pDataWin->GetSizePixel().Width(), GetDataRowHeight() ) );
    else
    {
        // invalidate the specific field
        aRect = GetFieldRectPixel( nRow, nColId, FALSE );
    }
    ((BrowserDataWin*)pDataWin)->Invalidate( aRect );
}

//-------------------------------------------------------------------

void BrowseBox::Clear()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // adjust the total number of rows
    DoHideCursor( "Clear" );
    nRowCount = 0;
    nCurRow = BROWSER_ENDOFSELECTION;
    nTopRow = 0;
    nCurColId = 0;

    // nFirstCol darf nicht zurueckgesetzt werden, da ansonsten das Scrollen
    // total durcheinander kommt
    // nFirstCol darf nur beim Hinzufuegen oder Loeschen von Spalten geaendert werden
    // nFirstCol = 0; ->Falsch!!!!
    aHScroll.SetThumbPos( 0 );
    pVScroll->SetThumbPos( 0 );

    Invalidate();
    UpdateScrollbars();
    SetNoSelection();
    DoShowCursor( "Clear" );
    CursorMoved();
}

//-------------------------------------------------------------------
#if SUPD > 511

#else

void BrowseBox::RowInserted( long nRow, long nNumRows, BOOL bDoPaint )
{
    RowInserted( nRow, nNumRows, bDoPaint, FALSE );
}

#endif

void BrowseBox::RowInserted( long nRow, long nNumRows, BOOL bDoPaint, BOOL bKeepSelection )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if (nRow < 0)
        nRow = 0;
    else if (nRow > nRowCount) // maximal = nRowCount
        nRow = nRowCount;

    if ( nNumRows <= 0 )
        return;

#if 0
    // Zerlegung in einzelne RowInserted-Aufrufe:
    if (nNumRows > 1)
    {
        for (long i = 0; i < nNumRows; i++)
            RowInserted(nRow + i,1,bDoPaint);
        return;
    }
#endif

    // adjust total row count
    BOOL bLastRow = nRow >= nRowCount;
    nRowCount += nNumRows;

    DoHideCursor( "RowInserted" );

    // must we paint the new rows?
    long nOldCurRow = nCurRow;
    Size aSz = pDataWin->GetOutputSizePixel();
    if ( bDoPaint && nRow >= nTopRow &&
         nRow <= nTopRow + aSz.Height() / GetDataRowHeight() )
    {
        long nY = (nRow-nTopRow) * GetDataRowHeight();
        if ( !bLastRow )
        {
            // scroll down the rows behind the new row
            pDataWin->SetClipRegion();
            if( pDataWin->GetBackground().IsScrollable() )
            {
                pDataWin->Scroll( 0, GetDataRowHeight() * nNumRows,
                                Rectangle( Point( 0, nY ),
                                        Size( aSz.Width(), aSz.Height() - nY ) ),
                                SCROLL_CLIP );
            }
            else
                pDataWin->Window::Invalidate( INVALIDATE_NOCHILDREN );
        }
        else
            // scroll would cause a repaint, so we must explicitly invalidate
            pDataWin->Invalidate( Rectangle( Point( 0, nY ),
                         Size( aSz.Width(), nNumRows * GetDataRowHeight() ) ) );
    }

    // ggf. Top-Row korrigieren
    if ( nRow < nTopRow )
        nTopRow += nNumRows;

    // adjust the selection
    if ( bMultiSelection )
        uRow.pSel->Insert( nRow, nNumRows );
    else if ( uRow.nSel != BROWSER_ENDOFSELECTION && nRow <= uRow.nSel )
        uRow.nSel += nNumRows;

    // adjust the cursor
    if ( nCurRow == BROWSER_ENDOFSELECTION )
        GoToRow( 0, FALSE, bKeepSelection );
    else if ( nRow <= nCurRow )
        GoToRow( nCurRow += nNumRows, FALSE, bKeepSelection );

    // adjust the vertical scrollbar
    if ( bDoPaint )
    {
        UpdateScrollbars();
        AutoSizeLastColumn();
    }

    DoShowCursor( "RowInserted" );
    if ( nCurRow != nOldCurRow )
        CursorMoved();

    DBG_ASSERT(nRowCount > 0,"BrowseBox: nRowCount <= 0");
    DBG_ASSERT(nCurRow >= 0,"BrowseBox: nCurRow < 0");
    DBG_ASSERT(nCurRow < nRowCount,"nCurRow >= nRowCount");
}

//-------------------------------------------------------------------

void BrowseBox::RowRemoved( long nRow, long nNumRows, BOOL bDoPaint )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( nRow < 0 )
        nRow = 0;
    else if ( nRow >= nRowCount )
        nRow = nRowCount - 1;

    if ( nNumRows <= 0 )
        return;

    if ( nRowCount <= 0 )
        return;

    if ( bDoPaint )
    {
        // hide cursor and selection
        DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
        ToggleSelection();
        DoHideCursor( "RowRemoved" );
    }

    // adjust total row count
    nRowCount -= nNumRows;
    if (nRowCount < 0) nRowCount = 0;
    long nOldCurRow = nCurRow;

    // adjust the selection
    if ( bMultiSelection )
        // uRow.pSel->Remove( nRow, nNumRows );
        for ( long i = 0; i < nNumRows; i++ )
            uRow.pSel->Remove( nRow );
    else if ( nRow < uRow.nSel && uRow.nSel >= nNumRows )
        uRow.nSel -= nNumRows;
    else if ( nRow <= uRow.nSel )
        uRow.nSel = BROWSER_ENDOFSELECTION;

    // adjust the cursor
    if ( nRowCount == 0 )   // don't compare nRowCount with nNumRows as nNumRows already was subtracted from nRowCount
        nCurRow = BROWSER_ENDOFSELECTION;
    else if ( nRow < nCurRow )
    {
        nCurRow -= Min( nCurRow - nRow, nNumRows );
        // with the above nCurRow points a) to the first row after the removed block or b) to the same line
        // as before, but moved up nNumRows
        // case a) needs an additional correction if the last n lines were deleted, as 'the first row after the
        // removed block' is an invalid position then
        // FS - 09/28/99 - 68429
        if (nCurRow == nRowCount)
            --nCurRow;
    }
    else if( nRow == nCurRow && nCurRow == nRowCount )
        nCurRow = nRowCount-1;

    // is the deleted row visible?
    Size aSz = pDataWin->GetOutputSizePixel();
    if ( nRow >= nTopRow &&
         nRow <= nTopRow + aSz.Height() / GetDataRowHeight() )
    {
        if ( bDoPaint )
        {
            // scroll up the rows behind the deleted row
            // if there are Rows behind
            if (nRow < nRowCount)
            {
                long nY = (nRow-nTopRow) * GetDataRowHeight();
                pDataWin->SetClipRegion();
                if( pDataWin->GetBackground().IsScrollable() )
                {
                    pDataWin->Scroll( 0, - (short) GetDataRowHeight() * nNumRows,
                        Rectangle( Point( 0, nY ), Size( aSz.Width(),
                            aSz.Height() - nY + nNumRows*GetDataRowHeight() ) ),
                            SCROLL_CLIP );
                }
                else
                    pDataWin->Window::Invalidate( INVALIDATE_NOCHILDREN );
            }
            else
            {
                // Repaint the Rect of the deleted row
                Rectangle aRect(
                        Point( 0, (nRow-nTopRow)*GetDataRowHeight() ),
                        Size( pDataWin->GetSizePixel().Width(),
                              nNumRows * GetDataRowHeight() ) );
                pDataWin->Invalidate( aRect );
            }
        }
    }
    // is the deleted row above of the visible area?
    else if ( nRow < nTopRow )
        nTopRow = nTopRow >= nNumRows ? nTopRow-nNumRows : 0;

    if ( bDoPaint )
    {
        // reshow cursor and selection
        ToggleSelection();
        DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );
        DoShowCursor( "RowRemoved" );

        // adjust the vertical scrollbar
        UpdateScrollbars();
        AutoSizeLastColumn();
    }

    if ( nOldCurRow != nCurRow )
        CursorMoved();

    DBG_ASSERT(nRowCount >= 0,"BrowseBox: nRowCount < 0");
    DBG_ASSERT(nCurRow >= 0 || nRowCount == 0,"BrowseBox: nCurRow < 0 && nRowCount != 0");
    DBG_ASSERT(nCurRow < nRowCount,"nCurRow >= nRowCount");
}

//-------------------------------------------------------------------

BOOL BrowseBox::GoToRow( long nRow)
{
    return GoToRow(nRow, FALSE, FALSE);
}

//-------------------------------------------------------------------

BOOL BrowseBox::GoToRowAndDoNotModifySelection( long nRow )
{
    return GoToRow( nRow, FALSE, TRUE );
}

//-------------------------------------------------------------------

#if SUPD > 511

#else
BOOL BrowseBox::GoToRow( long nRow, BOOL bRowColMove  )
{
    return GoToRow( nRow, bRowColMove, FALSE );
}
#endif

BOOL BrowseBox::GoToRow( long nRow, BOOL bRowColMove, BOOL bKeepSelection )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    long nOldCurRow = nCurRow;

    // nothing to do?
    if ( nRow == nCurRow && ( bMultiSelection || uRow.nSel == nRow ) )
        return TRUE;

    // out of range?
    if ( nRow < 0 || nRow >= nRowCount )
        return FALSE;

    // nicht erlaubt?
    if ( ( !bRowColMove && !IsCursorMoveAllowed( nRow, nCurColId ) ) )
        return FALSE;

    if ( ((BrowserDataWin*)pDataWin)->bNoScrollBack && nRow < nTopRow )
        nRow = nTopRow;

    // compute the last visible row
    Size aSz( pDataWin->GetSizePixel() );
    USHORT nVisibleRows = USHORT( aSz.Height() ) / GetDataRowHeight() - 1;
    long nLastRow = nTopRow + nVisibleRows;

    // suspend Updates
    ((BrowserDataWin*)pDataWin)->EnterUpdateLock();

    // ggf. altes Highlight weg
    if ( !bMultiSelection && !bKeepSelection )
        ToggleSelection();
    DoHideCursor( "GoToRow" );

    // must we scroll?
    BOOL bWasVisible = bSelectionIsVisible;
    if (! bMultiSelection)
    {
        if( !bKeepSelection )
            bSelectionIsVisible = FALSE;
    }
    if ( nRow < nTopRow )
        ScrollRows( nRow - nTopRow );
    else if ( nRow > nLastRow )
        ScrollRows( nRow - nLastRow );
    bSelectionIsVisible = bWasVisible;

    // adjust cursor (selection) and thumb
    if ( GetUpdateMode() )
        pVScroll->SetThumbPos( nTopRow );

    // relative positioning (because nCurRow might have changed in the meantime)!
    if (nCurRow != BROWSER_ENDOFSELECTION )
        nCurRow = nCurRow + (nRow - nOldCurRow);

    // make sure that the current position is valid
    if (nCurRow == BROWSER_ENDOFSELECTION && nRowCount > 0)
        nCurRow = 0;
    else if ( nCurRow >= nRowCount )
        nCurRow = nRowCount - 1;
    aSelRange = Range( nCurRow, nCurRow );

    // ggf. neues Highlight anzeigen
    if ( !bMultiSelection && !bKeepSelection )
        uRow.nSel = nRow;

    // resume Updates
    ((BrowserDataWin*)pDataWin)->LeaveUpdateLock();

    // Cursor+Highlight
    if ( !bMultiSelection && !bKeepSelection)
        ToggleSelection();
    DoShowCursor( "GoToRow" );
    if ( !bRowColMove  && nOldCurRow != nCurRow )
        CursorMoved();

    if ( !bMultiSelection && !bKeepSelection )
        if ( !bSelecting )
            Select();
        else
            bSelect = TRUE;
    return TRUE;
}

//-------------------------------------------------------------------

BOOL BrowseBox::GoToColumnId( USHORT nColId)
{
    return GoToColumnId(nColId,TRUE,FALSE);
}


BOOL BrowseBox::GoToColumnId( USHORT nColId, BOOL bMakeVisible, BOOL bRowColMove)
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if (!bColumnCursor)
        return FALSE;

    // erlaubt?
    if (!bRowColMove && !IsCursorMoveAllowed( nCurRow, nColId ) )
        return FALSE;

    if ( nColId != nCurColId || bMakeVisible && !IsFieldVisible(nCurRow, nColId, TRUE))
    {
        DoHideCursor( "GoToColumnId" );
        nCurColId = nColId;

        USHORT nNewPos = GetColumnPos(nColId);
        DBG_ASSERT( nNewPos != USHRT_MAX, "unknown column-id" );
        USHORT nFirstPos = nFirstCol;
        USHORT nWidth = (USHORT)pCols->GetObject( nNewPos )->Width();
        USHORT nLastPos = GetColumnAtXPosPixel(
                            pDataWin->GetSizePixel().Width()-nWidth, FALSE );
        USHORT nFrozen = FrozenColCount();
        if ( bMakeVisible && nLastPos &&
             nNewPos >= nFrozen && ( nNewPos < nFirstPos || nNewPos > nLastPos ) )
            if ( nNewPos < nFirstPos )
                ScrollColumns( nNewPos-nFirstPos );
            else if ( nNewPos > nLastPos )
                ScrollColumns( nNewPos-nLastPos );

        DoShowCursor( "GoToColumnId" );
        if (!bRowColMove)
            CursorMoved();
        return TRUE;
    }
    return TRUE;
}

//-------------------------------------------------------------------

BOOL BrowseBox::GoToRowColumnId( long nRow, USHORT nColId )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    long nOldCurRow = nCurRow;

    // out of range?
    if ( nRow < 0 || nRow >= nRowCount )
        return FALSE;

    if (!bColumnCursor)
        return FALSE;

    // nothing to do ?
    if ( nRow == nCurRow && ( bMultiSelection || uRow.nSel == nRow ) &&
         nColId == nCurColId && IsFieldVisible(nCurRow, nColId, TRUE))
        return TRUE;

    // erlaubt?
    if (!IsCursorMoveAllowed(nRow, nColId))
        return FALSE;

    DoHideCursor( "GoToRowColumnId" );
    BOOL bMoved = GoToRow(nRow, TRUE) && GoToColumnId(nColId, TRUE, TRUE);
    DoShowCursor( "GoToRowColumnId" );

    if (bMoved)
        CursorMoved();

    return bMoved;
}

//-------------------------------------------------------------------

void BrowseBox::SetNoSelection()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // is there no selection
    if ( ( !pColSel || !pColSel->GetSelectCount() ) &&
         ( ( !bMultiSelection && uRow.nSel == BROWSER_ENDOFSELECTION ) ||
           ( bMultiSelection && !uRow.pSel->GetSelectCount() ) ) )
        // nothing to do
        return;

    DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
    ToggleSelection();

    // unselect all
    if ( bMultiSelection )
        uRow.pSel->SelectAll(FALSE);
    else
        uRow.nSel = BROWSER_ENDOFSELECTION;
    if ( pColSel )
        pColSel->SelectAll(FALSE);
    if ( !bSelecting )
        Select();
    else
        bSelect = TRUE;

    // restore screen
    DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );
}

//-------------------------------------------------------------------

void BrowseBox::SetSelection( const MultiSelection &rSel )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    DBG_ASSERT( bMultiSelection, "SetSelection only allowed with Multi-Selection-Mode" );

    // prepare inverted areas
    DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
    ToggleSelection();

    // assign Selection
    *uRow.pSel = rSel;

    // only highlight painted areas
    pDataWin->Update();

    // notify derived class
    if ( !bSelecting )
        Select();
    else
        bSelect = TRUE;

    // restore screen
    ToggleSelection();
    DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );
}

//-------------------------------------------------------------------

void BrowseBox::SelectAll()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( !bMultiSelection )
        return;

    DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
    ToggleSelection();

    // select all rows
    if ( pColSel )
        pColSel->SelectAll(FALSE);
    uRow.pSel->SelectAll(TRUE);

    // Handle-Column nicht highlighten
    BrowserColumn *pFirstCol = pCols->GetObject(0);
    long nOfsX = pFirstCol->GetId() ? 0 : pFirstCol->Width();

    // highlight the row selection
    if ( !bHideSelect )
    {
        Rectangle aHighlightRect;
        USHORT nVisibleRows =
            (USHORT)(pDataWin->GetOutputSizePixel().Height() / GetDataRowHeight() + 1);
        for ( long nRow = Max( nTopRow, uRow.pSel->FirstSelected() );
              nRow != BROWSER_ENDOFSELECTION && nRow < nTopRow + nVisibleRows;
              nRow = uRow.pSel->NextSelected() )
            aHighlightRect.Union( Rectangle(
                Point( nOfsX, (nRow-nTopRow)*GetDataRowHeight() ),
                Size( pDataWin->GetSizePixel().Width(), GetDataRowHeight() ) ) );
        pDataWin->Invalidate( aHighlightRect );
    }

    if ( !bSelecting )
        Select();
    else
        bSelect = TRUE;

    // restore screen
    DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );
}

//-------------------------------------------------------------------

void BrowseBox::SelectRow( long nRow, BOOL bSelect, BOOL bExpand )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( !bMultiSelection )
    {
        // deselecting is impossible, selecting via cursor
        if ( bSelect )
            GoToRow(nRow, FALSE);
        return;
    }

    DBG_TRACE1( "BrowseBox: %p->HideCursor", this );

    // remove old selection?
    if ( !bExpand || !bMultiSelection )
    {
        ToggleSelection();
        if ( bMultiSelection )
            uRow.pSel->SelectAll(FALSE);
        else
            uRow.nSel = BROWSER_ENDOFSELECTION;
        if ( pColSel )
            pColSel->SelectAll(FALSE);
    }

    // set new selection
    if ( !bHideSelect &&
         ( (bMultiSelection && uRow.pSel->GetTotalRange().Max() >= nRow && uRow.pSel->Select(nRow,bSelect)) ||
           (!bMultiSelection && ( uRow.nSel = nRow ) != BROWSER_ENDOFSELECTION ) ) )
    {
        // Handle-Column nicht highlighten
        BrowserColumn *pFirstCol = pCols->GetObject(0);
        long nOfsX = pFirstCol->GetId() ? 0 : pFirstCol->Width();

        // highlight only newly selected part
        Rectangle aRect(
            Point( nOfsX, (nRow-nTopRow)*GetDataRowHeight() ),
            Size( pDataWin->GetSizePixel().Width(), GetDataRowHeight() ) );
        pDataWin->Invalidate( aRect );
    }

    if ( !bSelecting )
        Select();
    else
        bSelect = TRUE;

    // restore screen
    DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );
}

//-------------------------------------------------------------------

long BrowseBox::GetSelectRowCount() const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return bMultiSelection ? uRow.pSel->GetSelectCount() :
           uRow.nSel == BROWSER_ENDOFSELECTION ? 0 : 1;
}

//-------------------------------------------------------------------

void BrowseBox::SelectColumnPos( USHORT nNewColPos, BOOL bSelect, BOOL bMakeVisible )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( !bColumnCursor )
        return;

    if ( !bMultiSelection )
    {
        if ( bSelect )
            GoToColumnId( pCols->GetObject(nNewColPos)->GetId(), bMakeVisible );
        return;
    }
    else if (!GoToColumnId( pCols->GetObject(nNewColPos)->GetId(), bMakeVisible ))
        return;

    DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
    ToggleSelection();
    if ( bMultiSelection )
        uRow.pSel->SelectAll(FALSE);
    else
        uRow.nSel = BROWSER_ENDOFSELECTION;
    pColSel->SelectAll(FALSE);

    if ( pColSel->Select( nNewColPos ) )
    {
        // GoToColumnId( pCols->GetObject(nNewColPos)->GetId(), bMakeVisible );

        // only highlight painted areas
        pDataWin->Update();
        Rectangle aFieldRectPix( GetFieldRectPixel( nCurRow, nCurColId, FALSE ) );
        Rectangle aRect(
            Point( aFieldRectPix.Left() - MIN_COLUMNWIDTH, 0 ),
            Size( pCols->GetObject(nNewColPos)->Width(),
                  pDataWin->GetOutputSizePixel().Height() ) );
        pDataWin->Invalidate( aRect );
        if ( !bSelecting )
            Select();
        else
            bSelect = TRUE;
    }

    // restore screen
    DBG_TRACE1( "BrowseBox: %p->ShowCursor", this );
}

//-------------------------------------------------------------------

USHORT BrowseBox::GetSelectColumnCount() const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // while bAutoSelect (==!pColSel), 1 if any rows (yes rows!) else none
    return pColSel ? (USHORT) pColSel->GetSelectCount() :
           nCurRow >= 0 ? 1 : 0;
}

//-------------------------------------------------------------------

long BrowseBox::FirstSelectedRow( BOOL bInverse )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return bMultiSelection ? uRow.pSel->FirstSelected(bInverse) : uRow.nSel;
}

//-------------------------------------------------------------------

long BrowseBox::NextSelectedRow()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return bMultiSelection ? uRow.pSel->NextSelected() : BROWSER_ENDOFSELECTION;
}

//-------------------------------------------------------------------

long BrowseBox::PrevSelectedRow()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return bMultiSelection ? uRow.pSel->PrevSelected() : BROWSER_ENDOFSELECTION;
}

//-------------------------------------------------------------------

long BrowseBox::LastSelectedRow( BOOL bInverse )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return bMultiSelection ? uRow.pSel->LastSelected(bInverse) : uRow.nSel;
}

//-------------------------------------------------------------------

BOOL BrowseBox::IsRowSelected( long nRow ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return bMultiSelection ? uRow.pSel->IsSelected(nRow) : nRow == uRow.nSel;
}

//-------------------------------------------------------------------

BOOL BrowseBox::IsColumnSelected( USHORT nColumnId ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return pColSel ? pColSel->IsSelected( GetColumnPos(nColumnId) ) :
                     nCurColId == nColumnId;
}

//-------------------------------------------------------------------

BOOL BrowseBox::MakeFieldVisible
(
    long    nRow,       // Zeilen-Nr des Feldes (beginnend mit 0)
    USHORT  nColId,     // Spalten-Id des Feldes
    BOOL    bComplete   // (== FALSE), TRUE => vollst"andig sichtbar machen
)

/*  [Beschreibung]

    Macht das durch 'nRow' und 'nColId' beschriebene Feld durch
    entsprechendes scrollen sichtbar. Ist 'bComplete' gesetzt, dann wird
    gefordert, da\s das Feld ganz sichtbar wird.

    [R"uckgabewert]

    BOOL            TRUE
                    Das angegebene Feld wurde sichtbar gemacht, bzw. war
                    bereits sichtbar.

                    FALSE
                    Das angegebene Feld konnte nicht sichtbar bzw. bei
                    'bComplete' nicht vollst"andig sichtbar gemacht werden.
*/

{
    Size aTestSize = pDataWin->GetSizePixel();

    if ( !bBootstrapped ||
         ( aTestSize.Width() == 0 && aTestSize.Height() == 0 ) )
        return FALSE;

    // ist es schon sichtbar?
    BOOL bVisible = IsFieldVisible( nRow, nColId, bComplete );
    if ( bVisible )
        return TRUE;

    // Spaltenposition und Feld-Rechteck und Ausgabebereich berechnen
    USHORT nColPos = GetColumnPos( nColId );
    Rectangle aFieldRect = GetFieldRectPixel( nRow, nColId, FALSE );
    Rectangle aDataRect = Rectangle( Point(0, 0), pDataWin->GetSizePixel() );

    // links au\serhalb?
    if ( nColPos >= FrozenColCount() && nColPos < nFirstCol )
        // => nach rechts scrollen
        ScrollColumns( nColPos - nFirstCol );

    // solange rechts au\serhalb
    while ( aDataRect.Right() < ( bComplete
                ? aFieldRect.Right()
                : aFieldRect.Left()+aFieldRect.GetWidth()/2 ) )
    {
        // => nach links scrollen
        if ( ScrollColumns( 1 ) != 1 )
            // nichts mehr zu scrollen
            break;
        aFieldRect = GetFieldRectPixel( nRow, nColId, FALSE );
    }

    // oben au\serhalb?
    if ( nRow < nTopRow )
        // nach unten scrollen
        ScrollRows( nRow - nTopRow );

    // unten au\serhalb?
    long nBottomRow = nTopRow + GetVisibleRows();
    // OV: damit nBottomRow die Nummer der letzten sichtbaren Zeile ist
    // (Zaehlung ab Null!), muss sie dekrementiert werden.
    // Beispiel: BrowseBox enthaelt genau einen Eintrag. nBottomRow := 0 + 1 - 1
    if( nBottomRow )
        nBottomRow--;

    if ( nRow > nBottomRow )
        // nach oben scrollen
        ScrollRows( nRow - nBottomRow );

    // jetzt kann es immer noch nicht passen, z.B. weil Window zu klein
    return IsFieldVisible( nRow, nColId, bComplete );
}

//-------------------------------------------------------------------

BOOL BrowseBox::IsFieldVisible( long nRow, USHORT nColumnId,
                                BOOL bCompletely ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // durch frozen-Column verdeckt?
    USHORT nColPos = GetColumnPos( nColumnId );
    if ( nColPos >= FrozenColCount() && nColPos < nFirstCol )
        return FALSE;

    Rectangle aRect( ImplFieldRectPixel( nRow, nColumnId ) );
    if ( aRect.IsEmpty() )
        return FALSE;

    // get the visible area
    Rectangle aOutRect( Point(0, 0), pDataWin->GetOutputSizePixel() );

    if ( bCompletely )
        // test if the field is completely visible
        return aOutRect.IsInside( aRect );
    else
        // test if the field is partly of completely visible
        return !aOutRect.Intersection( aRect ).IsEmpty();
}

//-------------------------------------------------------------------

Rectangle BrowseBox::GetFieldRectPixel( long nRow, USHORT nColumnId,
                                        BOOL bRelToBrowser ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // get the rectangle relative to DataWin
    Rectangle aRect( ImplFieldRectPixel( nRow, nColumnId ) );
    if ( aRect.IsEmpty() )
        return aRect;

    // adjust relative to BrowseBox's output area
    Point aTopLeft( aRect.TopLeft() );
    if ( bRelToBrowser )
    {
        aTopLeft = pDataWin->OutputToScreenPixel( aTopLeft );
        aTopLeft = ScreenToOutputPixel( aTopLeft );
    }

    return Rectangle( aTopLeft, aRect.GetSize() );
}

//-------------------------------------------------------------------

Rectangle BrowseBox::GetRowRectPixel( long nRow, BOOL bRelToBrowser  ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // get the rectangle relative to DataWin
    Rectangle aRect;
    if ( nTopRow > nRow )
        // row is above visible area
        return aRect;
    aRect = Rectangle(
        Point( 0, GetDataRowHeight() * (nRow-nTopRow) ),
        Size( pDataWin->GetOutputSizePixel().Width(), GetDataRowHeight() ) );
    if ( aRect.TopLeft().Y() > pDataWin->GetOutputSizePixel().Height() )
        // row is below visible area
        return aRect;

    // adjust relative to BrowseBox's output area
    Point aTopLeft( aRect.TopLeft() );
    if ( bRelToBrowser )
    {
        aTopLeft = pDataWin->OutputToScreenPixel( aTopLeft );
        aTopLeft = ScreenToOutputPixel( aTopLeft );
    }

    return Rectangle( aTopLeft, aRect.GetSize() );
}

//-------------------------------------------------------------------

Rectangle BrowseBox::ImplFieldRectPixel( long nRow, USHORT nColumnId ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // compute the X-coordinte realtiv to DataWin by accumulation
    long nColX = 0;
    USHORT nFrozenCols = FrozenColCount();
    USHORT nCol;
    for ( nCol = 0;
          nCol < pCols->Count() && pCols->GetObject(nCol)->GetId() != nColumnId;
          ++nCol )
        if ( pCols->GetObject(nCol)->IsFrozen() || nCol >= nFirstCol )
            nColX += pCols->GetObject(nCol)->Width();

    if ( nCol >= pCols->Count() || ( nCol >= nFrozenCols && nCol < nFirstCol ) )
        return Rectangle();

    // compute the Y-coordinate relative to DataWin
    long nRowY = ( nRow - nTopRow ) * GetDataRowHeight();

    // assemble the Rectangle relative to DataWin
    return Rectangle(
        Point( nColX + MIN_COLUMNWIDTH, nRowY ),
        Size( pCols->GetObject(nCol)->Width() - 2*MIN_COLUMNWIDTH,
              GetDataRowHeight() - 1 ) );
}

//-------------------------------------------------------------------

long BrowseBox::GetRowAtYPosPixel( long nY, BOOL bRelToBrowser ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // compute the Y-coord
    if ( bRelToBrowser )
    {
        Point aDataTopLeft = pDataWin->OutputToScreenPixel( Point(0, 0) );
        Point aTopLeft = OutputToScreenPixel( Point(0, 0) );
        nY -= aDataTopLeft.Y() - aTopLeft.Y();
    }

    // no row there (e.g. in the header)
    if ( nY < 0 || nY >= pDataWin->GetOutputSizePixel().Height() )
        return -1;

    return nY / GetDataRowHeight() + nTopRow;
}

//-------------------------------------------------------------------

Rectangle BrowseBox::GetFieldRect( USHORT nColumnId ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return GetFieldRectPixel( nCurRow, nColumnId );
}

//-------------------------------------------------------------------

USHORT BrowseBox::GetColumnAtXPosPixel( long nX, BOOL ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // accumulate the withds of the visible columns
    long nColX = 0;
    USHORT nCol;
    for ( nCol = 0; nCol < USHORT(pCols->Count()); ++nCol )
    {
        BrowserColumn *pCol = pCols->GetObject(nCol);
        if ( pCol->IsFrozen() || nCol >= nFirstCol )
            nColX += pCol->Width();

        if ( nColX > nX )
            return nCol;
    }

    return BROWSER_INVALIDID;
}

//-------------------------------------------------------------------

void BrowseBox::ReserveControlArea( USHORT nWidth )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( nWidth != nControlAreaWidth )
    {
        nControlAreaWidth = nWidth;
        UpdateScrollbars();
    }
}

//-------------------------------------------------------------------

Rectangle BrowseBox::GetControlArea() const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return Rectangle(
        Point( 0, GetOutputSizePixel().Height() - aHScroll.GetSizePixel().Height() ),
        Size( GetOutputSizePixel().Width() - aHScroll.GetSizePixel().Width(),
             aHScroll.GetSizePixel().Height() ) );
}

#if SUPD<558
//-------------------------------------------------------------------

BrowserMode BrowseBox::GetMode( ) const
{
    return m_nCurrentMode;
}
#endif

//-------------------------------------------------------------------

void BrowseBox::SetMode( BrowserMode nMode )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

#ifdef DBG_MIx
    Sound::Beep();
    nMode =
//          BROWSER_COLUMNSELECTION |
//          BROWSER_MULTISELECTION |
            BROWSER_THUMBDRAGGING |
            BROWSER_KEEPHIGHLIGHT |
            BROWSER_HLINES |
            BROWSER_VLINES |
//          BROWSER_HIDECURSOR |
//          BROWSER_NO_HSCROLL |
//          BROWSER_NO_SCROLLBACK |
            BROWSER_AUTO_VSCROLL |
            BROWSER_AUTO_HSCROLL |
            BROWSER_TRACKING_TIPS |
//          BROWSER_HIGHLIGHT_NONE |
            BROWSER_HIGHLIGHT_AUTO |
//          BROWSER_HIGHLIGHT_MANU |
            BROWSER_HEADERBAR_NEW |
//          BROWSER_AUTOSIZE_LASTCOL |
            0;
#endif

    ((BrowserDataWin*)pDataWin)->bAutoHScroll =
            BROWSER_AUTO_HSCROLL == ( nMode & BROWSER_AUTO_HSCROLL);
    ((BrowserDataWin*)pDataWin)->bAutoVScroll =
            BROWSER_AUTO_VSCROLL == ( nMode & BROWSER_AUTO_VSCROLL);

    ((BrowserDataWin*)pDataWin)->bNoHScroll =
            BROWSER_NO_HSCROLL == ( nMode & BROWSER_NO_HSCROLL);
    if ( ((BrowserDataWin*)pDataWin)->bNoHScroll )
    {
        aHScroll.Hide();
        nControlAreaWidth = 0;
    }
    else
        nControlAreaWidth = USHRT_MAX;

    ((BrowserDataWin*)pDataWin)->bNoScrollBack =
            BROWSER_NO_SCROLLBACK == ( nMode & BROWSER_NO_SCROLLBACK);

    long nOldRowSel = bMultiSelection ? uRow.pSel->FirstSelected() : uRow.nSel;
    MultiSelection *pOldRowSel = bMultiSelection ? uRow.pSel : 0;
    MultiSelection *pOldColSel = pColSel;

    delete pVScroll;

    bThumbDragging = ( nMode & BROWSER_THUMBDRAGGING ) == BROWSER_THUMBDRAGGING;
    bMultiSelection = ( nMode & BROWSER_MULTISELECTION ) == BROWSER_MULTISELECTION;
    bColumnCursor = ( nMode & BROWSER_COLUMNSELECTION ) == BROWSER_COLUMNSELECTION;
    bKeepHighlight = ( nMode & BROWSER_KEEPSELECTION ) == BROWSER_KEEPSELECTION;

    bHideSelect = ((nMode & BROWSER_HIDESELECT) == BROWSER_HIDESELECT);
    bHideCursor = ((nMode & BROWSER_HIDECURSOR) == BROWSER_HIDECURSOR);
    m_bFocusOnlyCursor = ((nMode & BROWSER_CURSOR_WO_FOCUS) == 0);

    bHLines = ( nMode & BROWSER_HLINESFULL ) == BROWSER_HLINESFULL;
    bVLines = ( nMode & BROWSER_VLINESFULL ) == BROWSER_VLINESFULL;
    bHDots  = ( nMode & BROWSER_HLINESDOTS ) == BROWSER_HLINESDOTS;
    bVDots  = ( nMode & BROWSER_VLINESDOTS ) == BROWSER_VLINESDOTS;

    WinBits nVScrollWinBits =
        WB_VSCROLL | ( ( nMode & BROWSER_THUMBDRAGGING ) ? WB_DRAG : 0 );
    pVScroll = ( nMode & BROWSER_TRACKING_TIPS ) == BROWSER_TRACKING_TIPS
                ? new BrowserScrollBar( this, nVScrollWinBits,
                                        (BrowserDataWin*) pDataWin )
                : new ScrollBar( this, nVScrollWinBits );
    pVScroll->SetLineSize( 1 );
    pVScroll->SetPageSize(1);
    pVScroll->SetScrollHdl( LINK( this, BrowseBox, ScrollHdl ) );
    pVScroll->SetEndScrollHdl( LINK( this, BrowseBox, EndScrollHdl ) );

    ((BrowserDataWin*)pDataWin)->bHighlightAuto =
            BROWSER_HIGHLIGHT_AUTO == ( nMode & BROWSER_HIGHLIGHT_AUTO ) ||
            BROWSER_HIGHLIGHT_MANU != ( nMode & BROWSER_HIGHLIGHT_MANU );
    ((BrowserDataWin*)pDataWin)->bAutoSizeLastCol =
            BROWSER_AUTOSIZE_LASTCOL == ( nMode & BROWSER_AUTOSIZE_LASTCOL );
    ((BrowserDataWin*)pDataWin)->bOwnDataChangedHdl =
            BROWSER_OWN_DATACHANGED == ( nMode & BROWSER_OWN_DATACHANGED );

    // Headerbar erzeugen, was passiert, wenn eine erzeugt werden muß und schon Spalten bestehen ?
    if ( BROWSER_HEADERBAR_NEW == ( nMode & BROWSER_HEADERBAR_NEW ) )
    {
        if (!((BrowserDataWin*)pDataWin)->pHeaderBar)
            ((BrowserDataWin*)pDataWin)->pHeaderBar = CreateHeaderBar( this );
    }
    else
    {
        DELETEZ(((BrowserDataWin*)pDataWin)->pHeaderBar);
    }



    if ( bColumnCursor )
    {
        pColSel = pOldColSel ? pOldColSel : new MultiSelection;
        pColSel->SetTotalRange( Range( 0, pCols->Count()-1 ) );
    }
    else
    {
        pColSel = 0;
        delete pColSel;
    }

    if ( bMultiSelection )
    {
        if ( pOldRowSel )
            uRow.pSel = pOldRowSel;
        else
            uRow.pSel = new MultiSelection;
    }
    else
    {
        uRow.nSel = nOldRowSel;
        delete pOldRowSel;
    }

    if ( bBootstrapped )
    {
        StateChanged( STATE_CHANGE_INITSHOW );
        if ( bMultiSelection && !pOldRowSel &&
             nOldRowSel != BROWSER_ENDOFSELECTION )
            uRow.pSel->Select( nOldRowSel );
    }

    if ( pDataWin )
        pDataWin->Invalidate();

    // kein Cursor auf Handle-Column
    if ( nCurColId == 0 )
        nCurColId = GetColumnId( 1 );

    m_nCurrentMode = nMode;
}

//-------------------------------------------------------------------

void BrowseBox::VisibleRowsChanged( long nNewTopRow, USHORT nNumRows)
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // Das alte Verhalten: NumRows automatisch korrigieren:
    if ( nRowCount < GetRowCount() )
    {
        RowInserted(nRowCount,GetRowCount() - nRowCount,FALSE);
    }
    else if ( nRowCount > GetRowCount() )
    {
        RowRemoved(nRowCount-(nRowCount - GetRowCount()),nRowCount - GetRowCount(),FALSE);
    }
}

//-------------------------------------------------------------------

BOOL BrowseBox::IsCursorMoveAllowed( long nNewRow, USHORT nNewColId ) const

/*  [Beschreibung]

    Diese virtuelle Methode wird immer gerufen bevor der Cursor direkt
    bewegt werden soll. Durch 'return FALSE' kann verhindert werden, da\s
    dies geschieht, wenn z.B. ein Datensatz irgendwelchen Rules widerspricht.

    Diese Methode wird nicht gerufen, wenn die Cursorbewegung durch
    ein L"oschen oder Einf"ugen (einer Zeile/Spalte) ausgel"ost wird, also
    genaugenommen nur eine Cursor-Korrektur vorliegt.

    Die Basisimplementierung liefert derzeit immer TRUE.
*/

{
    return TRUE;
}

//-------------------------------------------------------------------

long BrowseBox::GetDataRowHeight() const
{
    return CalcZoom(nDataRowHeight ? nDataRowHeight : ImpGetDataRowHeight());
}

//-------------------------------------------------------------------

Window& BrowseBox::GetEventWindow() const
{
    return *((BrowserDataWin*)pDataWin)->pEventWin;
}

//-------------------------------------------------------------------

#if SUPD >= 376

BrowserHeader* BrowseBox::CreateHeaderBar( BrowseBox* pParent )
{
    BrowserHeader* pNewBar = new BrowserHeader( pParent );
    pNewBar->SetStartDragHdl( LINK( this, BrowseBox, StartDragHdl ) );
    return pNewBar;
}

void BrowseBox::SetHeaderBar( BrowserHeader* pHeaderBar )
{
    delete ( (BrowserDataWin*)pDataWin )->pHeaderBar;
    ( (BrowserDataWin*)pDataWin )->pHeaderBar = pHeaderBar;
    ( (BrowserDataWin*)pDataWin )->pHeaderBar->SetStartDragHdl( LINK( this, BrowseBox, StartDragHdl ) );
}

#endif

//-------------------------------------------------------------------

#ifdef DBG_UTIL
const char* BrowseBoxCheckInvariants( const void * pVoid )
{
    const BrowseBox * p = (const BrowseBox *)pVoid;

    if (p->nRowCount < 0) return "BrowseBox: nRowCount < 0";
    if (p->nTopRow < 0) return "BrowseBox: nTopRow < 0";
    if (p->nTopRow >= p->nRowCount && p->nRowCount != 0) return "BrowseBox: nTopRow >= nRowCount && nRowCount != 0";
    if (p->nCurRow < -1) return "BrowseBox: nCurRow < -1";
    if (p->nCurRow > p->nRowCount) return "BrowseBox: nCurRow > nRowCount";

    // Leider waehrend der Bearbeitung nicht immer der Fall:
    //if (p->nCurRow < 0 && p->nRowCount != 0) return "nCurRow < 0 && nRowCount != 0";
    //if (p->nCurRow >= p->nRowCount && p->nRowCount != 0) return "nCurRow >= nRowCount && nRowCount != 0";

    return NULL;
}
#endif

//-------------------------------------------------------------------
long BrowseBox::GetTitleHeight() const
{
    long nHeight;
    // ask the header bar for the text height (if possible), as the header bar's font is adjusted with
    // our (and the header's) zoom factor
    HeaderBar* pHeaderBar = ( (BrowserDataWin*)pDataWin )->pHeaderBar;
    if ( pHeaderBar )
        nHeight = pHeaderBar->GetTextHeight();
    else
        nHeight = GetTextHeight();

    return nTitleLines ? nTitleLines * nHeight + 4 : 0;
}

//-------------------------------------------------------------------
long BrowseBox::CalcReverseZoom(long nVal)
{
    if (IsZoom())
    {
        const Fraction& rZoom = GetZoom();
        double n = (double)nVal;
        n *= (double)rZoom.GetDenominator();
        n /= (double)rZoom.GetNumerator();
        nVal = n>0 ? (long)(n + 0.5) : -(long)(-n + 0.5);
    }

    return nVal;
}

//-------------------------------------------------------------------
HeaderBar* BrowseBox::GetHeaderBar() const
{
    return ((BrowserDataWin*)pDataWin)->pHeaderBar;
}


