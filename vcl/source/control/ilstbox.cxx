/*************************************************************************
 *
 *  $RCSfile: ilstbox.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:35 $
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

#define _SV_ILSTBOX_CXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <scrbar.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif

#ifndef _SV_LSTBOX_H
#include <lstbox.h>
#endif
#ifndef _SV_ILSTBOX_HXX
#include <ilstbox.hxx>
#endif

#pragma hdrstop

// =======================================================================

void ImplInitFieldSettings( Window* pWin, BOOL bFont, BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = pWin->GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetFieldFont();
        if ( pWin->IsControlFont() )
            aFont.Merge( pWin->GetControlFont() );
        pWin->SetZoomedPointFont( aFont );
    }

    if ( bFont || bForeground )
    {
        Color aTextColor = rStyleSettings.GetFieldTextColor();
        if ( pWin->IsControlForeground() )
            aTextColor = pWin->GetControlForeground();
        pWin->SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if( pWin->IsControlBackground() )
            pWin->SetBackground( pWin->GetControlBackground() );
        else
            pWin->SetBackground( rStyleSettings.GetFieldColor() );
    }
}

// -----------------------------------------------------------------------

void ImplInitDropDownButton( PushButton* pButton )
{
    if ( pButton->GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_SPINUPDOWN )
        pButton->SetSymbol( SYMBOL_SPIN_UPDOWN );
    else
        pButton->SetSymbol( SYMBOL_SPIN_DOWN );
}

// =======================================================================

ImplEntryList::ImplEntryList()
{
    mnLastSelected = LISTBOX_ENTRY_NOTFOUND;
    mnSelectionAnchor = LISTBOX_ENTRY_NOTFOUND;
    mnImages = 0;
    mbCallSelectionChangedHdl = TRUE;

    mnMRUCount = 0;
    mnMaxMRUCount = 0;
}

// -----------------------------------------------------------------------

ImplEntryList::~ImplEntryList()
{
    Clear();
}

// -----------------------------------------------------------------------

void ImplEntryList::Clear()
{
    mnImages = 0;
    for ( USHORT n = GetEntryCount(); n; )
    {
        ImplEntryType* pImplEntry = GetEntry( --n );
        delete pImplEntry;
    }
    List::Clear();
}

// -----------------------------------------------------------------------

void ImplEntryList::SelectEntry( USHORT nPos, BOOL bSelect )
{
    ImplEntryType* pImplEntry = GetEntry( nPos );
    if ( pImplEntry && ( pImplEntry->mbIsSelected != bSelect ) )
    {
        pImplEntry->mbIsSelected = bSelect;
        if ( mbCallSelectionChangedHdl )
            maSelectionChangedHdl.Call( (void*)nPos );
    }
}

// -----------------------------------------------------------------------

USHORT ImplEntryList::InsertEntry( USHORT nPos, ImplEntryType* pNewEntry, BOOL bSort )
{
    if ( !!pNewEntry->maImage )
        mnImages++;

    if ( !bSort || !Count() )
    {
        Insert( pNewEntry, nPos );
    }
    else
    {
        const XubString& rStr = pNewEntry->maStr;
        ULONG nLow, nHigh, nMid;

        nHigh = Count();

        const International& rIntl = Application::GetSettings().GetInternational();
        ImplEntryType* pTemp = GetEntry( (USHORT)(nHigh-1) );
        StringCompare eComp = rIntl.Compare( rStr, pTemp->maStr );

        // Schnelles Einfuegen bei sortierten Daten
        if ( eComp != COMPARE_LESS )
        {
            Insert( pNewEntry, LIST_APPEND );
        }
        else
        {
            nLow  = mnMRUCount;
            pTemp = (ImplEntryType*)GetEntry( (USHORT)nLow );
            eComp = rIntl.Compare( rStr, pTemp->maStr );
            if ( eComp != COMPARE_GREATER )
            {
                Insert( pNewEntry, (ULONG)0 );
            }
            else
            {
                // Binaeres Suchen
                nHigh--;
                do
                {
                    nMid = (nLow + nHigh) / 2;
                    pTemp = (ImplEntryType*)GetObject( nMid );
                    eComp = rIntl.Compare( rStr, pTemp->maStr );

                    if ( eComp == COMPARE_LESS )
                        nHigh = nMid-1;
                    else
                    {
                        if ( eComp == COMPARE_GREATER )
                            nLow = nMid + 1;
                        else
                            break;
                    }
                }
                while ( nLow <= nHigh );

                if ( eComp != COMPARE_LESS )
                    nMid++;

                Insert( pNewEntry, nMid );
            }
        }
    }

    return (USHORT)GetPos( pNewEntry );
}

// -----------------------------------------------------------------------

void ImplEntryList::RemoveEntry( USHORT nPos )
{
    ImplEntryType* pImplEntry = (ImplEntryType*)List::Remove( nPos );
    if ( pImplEntry )
    {
        if ( !!pImplEntry->maImage )
            mnImages--;

        delete pImplEntry;
    }
}

// -----------------------------------------------------------------------

USHORT ImplEntryList::FindEntry( const XubString& rStr, MatchMode nMatchMode, USHORT nMatchLen, USHORT nStart, BOOL bForward ) const
{
    BOOL    bEqual;
    USHORT  nPos = LISTBOX_ENTRY_NOTFOUND;
    USHORT  nEntryCount = GetEntryCount();
    if ( !bForward )
        nStart++;   // wird sofort dekrementiert
    for ( USHORT n = nStart; bForward ? ( n < nEntryCount ) : n; )
    {
        if ( !bForward )
            n--;

        ImplEntryType* pImplEntry = GetEntry( n );
        if ( nMatchMode == MATCH_CASE )
            bEqual = pImplEntry->maStr.Equals( rStr, 0, nMatchLen );
        else
            bEqual = pImplEntry->maStr.EqualsIgnoreCaseAscii( rStr, 0, nMatchLen );
        if (  bEqual )
        {
            // Wenn Case-Insensitiv matching, dann einen Eintrag bevorzugen,
            // der trotzdem case-sensitiv matched.
            if ( (nMatchMode == MATCH_CASE) ||
                 ((nMatchMode == MATCH_BEST) && (pImplEntry->maStr.Equals( rStr, 0, nMatchLen ))) )
            {
                nPos = n;
                break;
            }
            else if ( nPos == LISTBOX_ENTRY_NOTFOUND )
                nPos = n;   // Bei Case-Insensitiv gewinnt der erste, wenn keiner 100%
        }

        if ( bForward )
            n++;
    }
    return nPos;
}

// -----------------------------------------------------------------------

USHORT ImplEntryList::FindEntry( const void* pData ) const
{
    USHORT nPos = LISTBOX_ENTRY_NOTFOUND;
    for ( USHORT n = GetEntryCount(); n; )
    {
        ImplEntryType* pImplEntry = GetEntry( --n );
        if ( pImplEntry->mpUserData == pData )
        {
            nPos = n;
            break;
        }
    }
    return nPos;
}

// -----------------------------------------------------------------------

XubString ImplEntryList::GetEntryText( USHORT nPos ) const
{
    XubString aEntryText;
    ImplEntryType* pImplEntry = GetEntry( nPos );
    if ( pImplEntry )
        aEntryText = pImplEntry->maStr;
    return aEntryText;
}

// -----------------------------------------------------------------------

BOOL ImplEntryList::HasEntryImage( USHORT nPos ) const
{
    BOOL bImage = FALSE;
    ImplEntryType* pImplEntry = (ImplEntryType*)List::GetObject( nPos );
    if ( pImplEntry )
        bImage = !!pImplEntry->maImage;
    return bImage;
}

// -----------------------------------------------------------------------

Image ImplEntryList::GetEntryImage( USHORT nPos ) const
{
    Image aImage;
    ImplEntryType* pImplEntry = (ImplEntryType*)List::GetObject( nPos );
    if ( pImplEntry )
        aImage = pImplEntry->maImage;
    return aImage;
}

// -----------------------------------------------------------------------

void ImplEntryList::SetEntryData( USHORT nPos, void* pNewData )
{
    ImplEntryType* pImplEntry = (ImplEntryType*)List::GetObject( nPos );
    if ( pImplEntry )
        pImplEntry->mpUserData = pNewData;
}

// -----------------------------------------------------------------------

void* ImplEntryList::GetEntryData( USHORT nPos ) const
{
    ImplEntryType* pImplEntry = (ImplEntryType*)List::GetObject( nPos );
    return pImplEntry ? pImplEntry->mpUserData : NULL;
}

// -----------------------------------------------------------------------

USHORT ImplEntryList::GetSelectEntryCount() const
{
    USHORT nSelCount = 0;
    for ( USHORT n = GetEntryCount(); n; )
    {
        ImplEntryType* pImplEntry = GetEntry( --n );
        if ( pImplEntry->mbIsSelected )
            nSelCount++;
    }
    return nSelCount;
}

// -----------------------------------------------------------------------

XubString ImplEntryList::GetSelectEntry( USHORT nIndex ) const
{
    return GetEntryText( GetSelectEntryPos( nIndex ) );
}

// -----------------------------------------------------------------------

USHORT ImplEntryList::GetSelectEntryPos( USHORT nIndex ) const
{
    USHORT nSelEntryPos = LISTBOX_ENTRY_NOTFOUND;
    USHORT nSel = 0;
    USHORT nEntryCount = GetEntryCount();

    for ( USHORT n = 0; n < nEntryCount; n++ )
    {
        ImplEntryType* pImplEntry = GetEntry( n );
        if ( pImplEntry->mbIsSelected )
        {
            if ( nSel == nIndex )
            {
                nSelEntryPos = n;
                break;
            }
            nSel++;
        }
    }

    return nSelEntryPos;
}

// -----------------------------------------------------------------------

BOOL ImplEntryList::IsEntrySelected( const XubString& rStr ) const
{
    return IsEntryPosSelected( FindEntry( rStr ) );
}

// -----------------------------------------------------------------------

BOOL ImplEntryList::IsEntryPosSelected( USHORT nIndex ) const
{
    ImplEntryType* pImplEntry = GetEntry( nIndex );
    return pImplEntry ? pImplEntry->mbIsSelected : FALSE;
}

// -----------------------------------------------------------------------

void ImplEntryList::SetNoSelection()
{
    USHORT nEntryCount = GetEntryCount();
    for ( USHORT n = 0; n < nEntryCount; n++ )
        SelectEntry( n, FALSE );
}

// =======================================================================

ImplListBoxWindow::ImplListBoxWindow( Window* pParent, WinBits nWinStyle ) :
    Control( pParent, 0 )
{
    mpEntryList         = new ImplEntryList;

    mnTop               = 0;
    mnLeft              = 0;
    mnBorder            = 1;
    mnSelectModifier    = 0;
    mnUserDrawEntry     = LISTBOX_ENTRY_NOTFOUND;
    mbTrack             = FALSE;
    mbImgsDiffSz        = FALSE;
    mbTravelSelect      = FALSE;
    mbTrackingSelect    = FALSE;
    mbSelectionChanged  = FALSE;
    mbMouseMoveSelect   = FALSE;
    mbMulti             = FALSE;
    mbGrabFocus         = FALSE;
    mbUserDrawEnabled   = FALSE;
    mbInUserDraw        = FALSE;
    mbReadOnly          = FALSE;
    mbSimpleMode        = ( nWinStyle & WB_SIMPLEMODE ) ? TRUE : FALSE;
    mbSort              = ( nWinStyle & WB_SORT ) ? TRUE : FALSE;

    mnCurrentPos            = LISTBOX_ENTRY_NOTFOUND;
    mnTrackingSaveSelection = LISTBOX_ENTRY_NOTFOUND;
    mnSeparatorPos      = LISTBOX_ENTRY_NOTFOUND;

    SetLineColor();
    SetTextFillColor();
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );

    maSearchTimeout.SetTimeout( 500 );
    maSearchTimeout.SetTimeoutHdl( LINK( this, ImplListBoxWindow, SearchStringTimeout ) );

    ImplInitSettings( TRUE, TRUE, TRUE );
    ImplCalcMetrics();
}

// -----------------------------------------------------------------------

ImplListBoxWindow::~ImplListBoxWindow()
{
    maSearchTimeout.Stop();
    delete mpEntryList;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground )
{
    ImplInitFieldSettings( this, bFont, bForeground, bBackground );
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplCalcMetrics()
{
    mnMaxWidth      = 0;
    mnMaxTxtWidth   = 0;
    mnMaxImgWidth   = 0;
    mnMaxImgTxtWidth= 0;
    mnMaxImgHeight  = 0;

    mnTextHeight = (USHORT)GetTextHeight();
    mnMaxTxtHeight = mnTextHeight + mnBorder;
    mnMaxHeight = mnMaxTxtHeight;

    if ( maUserItemSize.Height() > mnMaxHeight )
        mnMaxHeight = (USHORT) maUserItemSize.Height();
    if ( maUserItemSize.Width() > mnMaxWidth )
        mnMaxWidth= (USHORT) maUserItemSize.Width();

    for ( USHORT n = mpEntryList->GetEntryCount(); n; )
    {
        const ImplEntryType* pEntry = mpEntryList->GetEntryPtr( --n );
        ImplCalcEntryMetrics( *pEntry, TRUE );
    }

    Size aSz( GetOutputSizePixel().Width(), mnMaxHeight );
    maFocusRect.SetSize( aSz );
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplListBoxWindow, SearchStringTimeout, Timer*, EMPTYARG )
{
    maSearchStr.Erase();
    return 1;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::Clear()
{
    mpEntryList->Clear();

    mnMaxHeight     = mnMaxTxtHeight;
    mnMaxWidth      = 0;
    mnMaxTxtWidth   = 0;
    mnMaxImgTxtWidth= 0;
    mnMaxImgWidth   = 0;
    mnMaxImgHeight  = 0;
    mnTop           = 0;
    mnLeft          = 0;
    mbImgsDiffSz    = FALSE;

    Invalidate();
}

void ImplListBoxWindow::SetUserItemSize( const Size& rSz )
{
    maUserItemSize = rSz;
    ImplCalcMetrics();
}

// -----------------------------------------------------------------------

struct ImplEntryMetrics
{
    BOOL    bText;
    BOOL    bImage;
    USHORT  nEntryWidth;
    USHORT  nEntryHeight;
    USHORT  nTextWidth;
    USHORT  nImgWidth;
    USHORT  nImgHeight;
};

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplCalcEntryMetrics( const ImplEntryType& rEntry, BOOL bUpdateMetrics )
{
    // bUpdateMetrics: Wenn Entry groesser als Max-Wert, dann uebernehmen

    // Diese Methode war mal auch fuer RemoveRentry gedacht, deshalb die
    // ImplEntryMetrics-Struktur, damit die Werte zurueckgegeben werden.
    // Jetzt werden die aktuellen Metriken aber doch immer angepasst, weil
    // bei RemoveEntry einmal komplett durchgegangen wird.

    ImplEntryMetrics aMetrics;
    aMetrics.bText = rEntry.maStr.Len() ? TRUE : FALSE;
    aMetrics.bImage = !!rEntry.maImage;
    aMetrics.nEntryWidth = 0;
    aMetrics.nEntryHeight = 0;
    aMetrics.nTextWidth = 0;
    aMetrics.nImgWidth = 0;
    aMetrics.nImgHeight = 0;

    if ( aMetrics.bText )
    {
        aMetrics.nTextWidth = (USHORT)GetTextWidth( rEntry.maStr );
        if( bUpdateMetrics && ( aMetrics.nTextWidth > mnMaxTxtWidth ) )
            mnMaxTxtWidth = aMetrics.nTextWidth;
        aMetrics.nEntryWidth = mnMaxTxtWidth;
    }
    if ( aMetrics.bImage )
    {
        Size aImgSz = rEntry.maImage.GetSizePixel();
        aMetrics.nImgWidth  = (USHORT) CalcZoom( aImgSz.Width() );
        aMetrics.nImgHeight = (USHORT) CalcZoom( aImgSz.Height() );

        if ( bUpdateMetrics )
        {
            if( mnMaxImgWidth && ( aMetrics.nImgWidth != mnMaxImgWidth ) )
                mbImgsDiffSz = TRUE;
            else if ( mnMaxImgHeight && ( aMetrics.nImgHeight != mnMaxImgHeight ) )
                mbImgsDiffSz = TRUE;

            if( aMetrics.nImgWidth > mnMaxImgWidth )
                mnMaxImgWidth = aMetrics.nImgWidth;
            if( aMetrics.nImgHeight > mnMaxImgHeight )
                mnMaxImgHeight = aMetrics.nImgHeight;

            mnMaxImgTxtWidth = Max( mnMaxImgTxtWidth, aMetrics.nTextWidth );
        }
    }
    if ( IsUserDrawEnabled() || aMetrics.bImage )
    {
        aMetrics.nEntryWidth = Max( aMetrics.nImgWidth, (USHORT)maUserItemSize.Width() );
        if ( aMetrics.bText )
            aMetrics.nEntryWidth += aMetrics.nTextWidth + IMG_TXT_DISTANCE;
        aMetrics.nEntryHeight = Max( mnMaxImgHeight, (USHORT)maUserItemSize.Height() ) + 2;
    }

    if ( bUpdateMetrics )
    {
        if ( aMetrics.nEntryWidth > mnMaxWidth )
            mnMaxWidth = aMetrics.nEntryWidth;
        if ( aMetrics.nEntryHeight > mnMaxHeight )
            mnMaxHeight = aMetrics.nEntryHeight;
    }
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplCallSelect()
{
    if ( !IsTravelSelect() && GetEntryList()->GetMaxMRUCount() )
    {
        // Insert the selected entry as MRU, if not allready first MRU
        USHORT nSelected = GetEntryList()->GetSelectEntryPos( 0 );
        USHORT nMRUCount = GetEntryList()->GetMRUCount();
        String aSelected = GetEntryList()->GetEntryText( nSelected );
        USHORT nFirstMatchingEntryPos = GetEntryList()->FindEntry( aSelected );
        if ( nFirstMatchingEntryPos || !nMRUCount )
        {
            BOOL bSelectNewEntry = FALSE;
            if ( nFirstMatchingEntryPos < nMRUCount )
            {
                RemoveEntry( nFirstMatchingEntryPos );
                nMRUCount--;
                if ( nFirstMatchingEntryPos == nSelected )
                    bSelectNewEntry = TRUE;
            }
            else if ( nMRUCount == GetEntryList()->GetMaxMRUCount() )
            {
                RemoveEntry( nMRUCount - 1 );
                nMRUCount--;
            }

            ImplEntryType* pNewEntry = new ImplEntryType( aSelected );
            pNewEntry->mbIsSelected = bSelectNewEntry;
            GetEntryList()->InsertEntry( 0, pNewEntry, FALSE );
            GetEntryList()->SetMRUCount( ++nMRUCount );
            SetSeparatorPos( nMRUCount ? nMRUCount-1 : 0 );
            maMRUChangedHdl.Call( NULL );
        }
    }

    maSelectHdl.Call( NULL );
    mbSelectionChanged = FALSE;
}

// -----------------------------------------------------------------------

USHORT ImplListBoxWindow::InsertEntry( USHORT nPos, ImplEntryType* pNewEntry )
{
    USHORT nNewPos = mpEntryList->InsertEntry( nPos, pNewEntry, mbSort );

    ImplCalcEntryMetrics( *pNewEntry, TRUE );
    return nNewPos;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::RemoveEntry( USHORT nPos )
{
    mpEntryList->RemoveEntry( nPos );
    ImplCalcMetrics();
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    mbMouseMoveSelect = FALSE;  // Nur bis zum ersten MouseButtonDown

    if ( !IsReadOnly() )
    {
        if( rMEvt.GetClicks() == 1 )
        {
            USHORT nSelect = (USHORT) ( ( rMEvt.GetPosPixel().Y() + mnBorder ) / mnMaxHeight ) + (USHORT) mnTop;
            if( nSelect < mpEntryList->GetEntryCount() )
            {
                if ( !mbMulti && GetEntryList()->GetSelectEntryCount() )
                    mnTrackingSaveSelection = GetEntryList()->GetSelectEntryPos( 0 );
                else
                    mnTrackingSaveSelection = LISTBOX_ENTRY_NOTFOUND;

                mnCurrentPos = nSelect;
                mbTrackingSelect = TRUE;
                SelectEntries( nSelect, LET_MBDOWN, rMEvt.IsShift(), rMEvt.IsMod1() );
                mbTrackingSelect = FALSE;
                if ( mbGrabFocus )
                    GrabFocus();

                StartTracking( STARTTRACK_SCROLLREPEAT );
            }
        }
        if( rMEvt.GetClicks() == 2 )
        {
            maDoubleClickHdl.Call( this );
        }
    }
    else // if ( mbGrabFocus )
    {
        GrabFocus();
    }
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( !mbMulti && mbMouseMoveSelect && mpEntryList->GetEntryCount() && !rMEvt.IsLeaveWindow() )
    {
        Point aPoint;
        Rectangle aRect( aPoint, GetOutputSizePixel() );
        if( aRect.IsInside( rMEvt.GetPosPixel() ) )
        {
            USHORT nSelect = (USHORT) ( ( rMEvt.GetPosPixel().Y() + mnBorder ) / mnMaxHeight ) + (USHORT) mnTop;
            nSelect = Min( nSelect, (USHORT) ( mnTop + mnMaxVisibleEntries ) );
            nSelect = Min( nSelect, (USHORT) ( mpEntryList->GetEntryCount() - 1 ) );
            if ( ( nSelect != mnCurrentPos ) || !GetEntryList()->GetSelectEntryCount() || ( nSelect != GetEntryList()->GetSelectEntryPos( 0 ) ) )
            {
                mbTrackingSelect = TRUE;
                SelectEntries( nSelect, LET_TRACKING, FALSE, FALSE );
                mbTrackingSelect = FALSE;
            }

            // Falls der DD-Button gedrueckt wurde und jemand mit gedrueckter
            // Maustaste in die ListBox faehrt...
            if ( rMEvt.IsLeft() && !rMEvt.IsSynthetic() )
            {
                if ( !mbMulti && GetEntryList()->GetSelectEntryCount() )
                    mnTrackingSaveSelection = GetEntryList()->GetSelectEntryPos( 0 );
                else
                    mnTrackingSaveSelection = LISTBOX_ENTRY_NOTFOUND;
                StartTracking( STARTTRACK_SCROLLREPEAT );
            }
        }
    }
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::SelectEntry( USHORT nPos, BOOL bSelect )
{
    if ( mpEntryList->IsEntryPosSelected( nPos ) != bSelect )
    {
        HideFocus();
        if( bSelect )
        {
            if( !mbMulti )
            {
                // Selektierten Eintrag deselektieren
                USHORT nDeselect = GetEntryList()->GetSelectEntryPos( 0 );
                if( nDeselect != LISTBOX_ENTRY_NOTFOUND )
                {
                    //SelectEntryPos( nDeselect, FALSE );
                    GetEntryList()->SelectEntry( nDeselect, FALSE );
                    if ( IsUpdateMode() && IsReallyVisible() )
                        ImplPaint( nDeselect, TRUE );
                }
            }
            mpEntryList->SelectEntry( nPos, TRUE );
            mnCurrentPos = nPos;
            if ( ( nPos != LISTBOX_ENTRY_NOTFOUND ) && IsUpdateMode() )
            {
                ImplPaint( nPos );
                if ( !IsVisible( nPos ) )
                    SetTopEntry( nPos );
            }
        }
        else
        {
            mpEntryList->SelectEntry( nPos, FALSE );
            ImplPaint( nPos, TRUE );
        }
    }
}

// -----------------------------------------------------------------------

BOOL ImplListBoxWindow::SelectEntries( USHORT nSelect, LB_EVENT_TYPE eLET, BOOL bShift, BOOL bCtrl )
{
    BOOL bFocusChanged = FALSE;
    BOOL bSelectionChanged = FALSE;

    if( IsEnabled() )
    {
        // Hier (Single-ListBox) kann nur ein Eintrag deselektiert werden
        if( !mbMulti )
        {
            USHORT nDeselect = mpEntryList->GetSelectEntryPos( 0 );
            if( nSelect != nDeselect )
            {
                SelectEntry( nSelect, TRUE );
                mpEntryList->SetLastSelected( nSelect );
                bFocusChanged = TRUE;
                bSelectionChanged = TRUE;
            }
        }
        // MultiListBox ohne Modifier
        else if( mbSimpleMode && !bCtrl && !bShift )
        {
            USHORT nEntryCount = mpEntryList->GetEntryCount();
            for ( USHORT nPos = 0; nPos < nEntryCount; nPos++ )
            {
                BOOL bSelect = nPos == nSelect;
                if ( mpEntryList->IsEntryPosSelected( nPos ) != bSelect )
                {
                    SelectEntry( nPos, bSelect );
                    bFocusChanged = TRUE;
                    bSelectionChanged = TRUE;
                }
            }
            mpEntryList->SetLastSelected( nSelect );
            mpEntryList->SetSelectionAnchor( nSelect );
        }
        // MultiListBox nur mit CTRL/SHIFT oder nicht im SimpleMode
        else if( ( !mbSimpleMode /* && !bShift */ ) || ( mbSimpleMode && ( bCtrl || bShift ) ) )
        {
            // Space fuer Selektionswechsel
            if( !bShift && ( ( eLET == LET_KEYSPACE ) || ( eLET == LET_MBDOWN ) ) )
            {
                SelectEntry( nSelect, !mpEntryList->IsEntryPosSelected( nSelect ) );
                mpEntryList->SetLastSelected( nSelect );
                mpEntryList->SetSelectionAnchor( nSelect );
                if ( !mpEntryList->IsEntryPosSelected( nSelect ) )
                    mpEntryList->SetSelectionAnchor( LISTBOX_ENTRY_NOTFOUND );
                bFocusChanged = TRUE;
                bSelectionChanged = TRUE;
            }
            else if( ( ( eLET == LET_TRACKING ) && ( nSelect != mnCurrentPos ) ) ||
                     ( bShift && ( ( eLET == LET_KEYMOVE ) || ( eLET == LET_MBDOWN ) ) ) )
            {
                mnCurrentPos = nSelect;
                bFocusChanged = TRUE;

                USHORT nAnchor = mpEntryList->GetSelectionAnchor();
                if( ( nAnchor == LISTBOX_ENTRY_NOTFOUND ) && ( mpEntryList->GetSelectEntryCount() ) )
                {
                    nAnchor = mpEntryList->GetSelectEntryPos( mpEntryList->GetSelectEntryCount() - 1 );
                }
                if( nAnchor != LISTBOX_ENTRY_NOTFOUND )
                {
                    // Alle Eintraege vom Anchor bis nSelect muessen selektiert sein
                    USHORT nStart = Min( nSelect, nAnchor );
                    USHORT nEnd = Max( nSelect, nAnchor );
                    for ( USHORT n = nStart; n <= nEnd; n++ )
                    {
                        if ( !mpEntryList->IsEntryPosSelected( n ) )
                        {
                            SelectEntry( n, TRUE );
                            bSelectionChanged = TRUE;
                        }
                    }

                    // Ggf. muss noch was deselektiert werden...
                    USHORT nLast = mpEntryList->GetLastSelected();
                    if ( nLast != LISTBOX_ENTRY_NOTFOUND )
                    {
                        if ( ( nLast > nSelect ) && ( nLast > nAnchor ) )
                        {
                            for ( USHORT n = nSelect+1; n <= nLast; n++ )
                            {
                                if ( mpEntryList->IsEntryPosSelected( n ) )
                                {
                                    SelectEntry( n, FALSE );
                                    bSelectionChanged = TRUE;
                                }
                            }
                        }
                        else if ( ( nLast < nSelect ) && ( nLast < nAnchor ) )
                        {
                            for ( USHORT n = nLast; n < nSelect; n++ )
                            {
                                if ( mpEntryList->IsEntryPosSelected( n ) )
                                {
                                    SelectEntry( n, FALSE );
                                    bSelectionChanged = TRUE;
                                }
                            }
                        }
                    }
                    mpEntryList->SetLastSelected( nSelect );
                }
            }
            else if( eLET != LET_TRACKING )
            {
                HideFocus();
                ImplPaint( nSelect, TRUE );
                bFocusChanged = TRUE;
            }
        }
        else if( bShift )
        {
            bFocusChanged = TRUE;
        }

        if( bSelectionChanged )
            mbSelectionChanged = TRUE;

        if( bFocusChanged )
        {
            maFocusRect.SetPos( Point( 0, ( nSelect - mnTop ) * mnMaxHeight ) );
            if( HasFocus() )
                ShowFocus( maFocusRect );
        }
    }
    return bSelectionChanged;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::Tracking( const TrackingEvent& rTEvt )
{
    Point aPoint;
    Rectangle aRect( aPoint, GetOutputSizePixel() );
    BOOL bInside = aRect.IsInside( rTEvt.GetMouseEvent().GetPosPixel() );

    if( rTEvt.IsTrackingCanceled() || rTEvt.IsTrackingEnded() ) // MouseButtonUp
    {
        if ( bInside && !rTEvt.IsTrackingCanceled() )
        {
            mnSelectModifier = rTEvt.GetMouseEvent().GetModifier();
            ImplCallSelect();
        }
        else
        {
            maCancelHdl.Call( NULL );
            if ( !mbMulti )
            {
                mbTrackingSelect = TRUE;
                SelectEntry( mnTrackingSaveSelection, TRUE );
                mbTrackingSelect = FALSE;
                if ( mnTrackingSaveSelection != LISTBOX_ENTRY_NOTFOUND )
                {
                    maFocusRect.SetPos( Point( 0, ( mnCurrentPos - mnTop ) * mnMaxHeight ) );
                    ShowFocus( maFocusRect );
                }
            }
        }

        mbTrack = FALSE;
    }
    else
    {
        BOOL bTrackOrQuickClick = mbTrack;
        if( !mbTrack )
        {
            if ( bInside )
            {
                mbTrack = TRUE;
            }

            // Folgender Fall tritt nur auf, wenn man ganz kurz die Maustaste drueckt
            if( rTEvt.IsTrackingEnded() && mbTrack )
            {
                bTrackOrQuickClick = TRUE;
                mbTrack = FALSE;
            }
        }

        if( bTrackOrQuickClick )
        {
            MouseEvent aMEvt = rTEvt.GetMouseEvent();
            Point aPt( aMEvt.GetPosPixel() );
            BOOL bShift = aMEvt.IsShift();
            BOOL bCtrl  = aMEvt.IsMod1();

            USHORT nSelect = LISTBOX_ENTRY_NOTFOUND;
            if( aPt.Y() < 0 )
            {
                nSelect = mnCurrentPos ? ( mnCurrentPos - 1 ) : 0;
                if( nSelect < mnTop )
                    SetTopEntry( mnTop-1 );
            }
            else if( aPt.Y() > GetOutputSizePixel().Height() )
            {
                nSelect = Min(  (USHORT)(mnCurrentPos+1), (USHORT)(mpEntryList->GetEntryCount()-1) );
                if( nSelect >= mnTop + mnMaxVisibleEntries )
                    SetTopEntry( mnTop+1 );
            }
            else
            {
                nSelect = (USHORT) ( ( aPt.Y() + mnBorder ) / mnMaxHeight ) + (USHORT) mnTop;
                nSelect = Min( nSelect, (USHORT) ( mnTop + mnMaxVisibleEntries ) );
                nSelect = Min( nSelect, (USHORT) ( mpEntryList->GetEntryCount() - 1 ) );
            }

            if ( bInside )
            {
                if ( ( nSelect != mnCurrentPos ) || !GetEntryList()->GetSelectEntryCount() )
                {
                    mbTrackingSelect = TRUE;
                    SelectEntries( nSelect, LET_TRACKING, bShift, bCtrl );
                    mbTrackingSelect = FALSE;
                }
            }
            else
            {
                if ( !mbMulti && GetEntryList()->GetSelectEntryCount() )
                {
                    mbTrackingSelect = TRUE;
                    SelectEntry( GetEntryList()->GetSelectEntryPos( 0 ), FALSE );
                    mbTrackingSelect = FALSE;
                }
            }
            mnCurrentPos = nSelect;
        }
    }
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::KeyInput( const KeyEvent& rKEvt )
{
    if( !ProcessKeyInput( rKEvt ) )
        Control::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

BOOL ImplListBoxWindow::ProcessKeyInput( const KeyEvent& rKEvt )
{
    // zu selektierender Eintrag
    USHORT nSelect = LISTBOX_ENTRY_NOTFOUND;
    LB_EVENT_TYPE eLET = LET_KEYMOVE;

    KeyCode aKeyCode = rKEvt.GetKeyCode();

    BOOL bShift = aKeyCode.IsShift();
    BOOL bCtrl  = aKeyCode.IsMod1();
    BOOL bMod2 = aKeyCode.IsMod2();
    BOOL bDone = FALSE;

    switch( aKeyCode.GetCode() )
    {
        case KEY_UP:
        {
            if ( IsReadOnly() )
            {
                if ( GetTopEntry() )
                    SetTopEntry( GetTopEntry()-1 );
            }
            else if ( !bMod2 )
            {
                if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
                    nSelect = mpEntryList->GetEntryCount() ? 0 : LISTBOX_ENTRY_NOTFOUND;
                else if ( mnCurrentPos )
                    nSelect = mnCurrentPos - 1;

                if( ( nSelect != LISTBOX_ENTRY_NOTFOUND ) && ( nSelect < mnTop ) )
                    SetTopEntry( mnTop-1 );

                bDone = TRUE;
            }
        }
        break;

        case KEY_DOWN:
        {
            if ( IsReadOnly() )
            {
                SetTopEntry( GetTopEntry()+1 );
            }
            else if ( !bMod2 )
            {
                if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
                    nSelect = mpEntryList->GetEntryCount() ? 0 : LISTBOX_ENTRY_NOTFOUND;
                else if ( (mnCurrentPos+1) < mpEntryList->GetEntryCount() )
                    nSelect = mnCurrentPos+1;

                if( ( nSelect != LISTBOX_ENTRY_NOTFOUND ) && ( nSelect >= ( mnTop + mnMaxVisibleEntries ) ) )
                    SetTopEntry( mnTop+1 );

                bDone = TRUE;
            }
        }
        break;

        case KEY_PAGEUP:
        {
            if ( IsReadOnly() )
            {
                SetTopEntry( ( mnTop > mnMaxVisibleEntries ) ?
                                (mnTop-mnMaxVisibleEntries) : 0 );
            }
            else if ( !bCtrl && !bMod2 )
            {
                if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
                    nSelect = mpEntryList->GetEntryCount() ? 0 : LISTBOX_ENTRY_NOTFOUND;
                else if ( mnCurrentPos )
                {
                    if( mnCurrentPos == mnTop )
                        SetTopEntry( ( mnTop > mnMaxVisibleEntries ) ?
                                     ( mnTop-mnMaxVisibleEntries+1 ) : 0 );
                    nSelect = mnTop;
                }
                bDone = TRUE;
            }
        }
        break;

        case KEY_PAGEDOWN:
        {
            if ( IsReadOnly() )
            {
                SetTopEntry( mnTop + mnMaxVisibleEntries );
            }
            else if ( !bCtrl && !bMod2 )
            {
                if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
                    nSelect = mpEntryList->GetEntryCount() ? 0 : LISTBOX_ENTRY_NOTFOUND;
                else if ( (mnCurrentPos+1) < mpEntryList->GetEntryCount() )
                {
                    USHORT nCount = mpEntryList->GetEntryCount();
                    USHORT nTmp = Min( mnMaxVisibleEntries, nCount );
                    nTmp += mnTop - 1;
                    if( mnCurrentPos == nTmp && mnCurrentPos != nCount - 1 )
                    {
                        long nTmp2 = Min( (long)(nCount-mnMaxVisibleEntries), (long)((long)mnTop+(long)mnMaxVisibleEntries-1) );
                        nTmp2 = Max( (long)0 , nTmp2 );
                        nTmp = (USHORT)(nTmp2+(mnMaxVisibleEntries-1) );
                        SetTopEntry( (USHORT)nTmp2 );
                    }
                    nSelect = nTmp;
                }
                bDone = TRUE;
            }
        }
        break;

        case KEY_HOME:
        {
            if ( IsReadOnly() )
            {
                SetTopEntry( 0 );
            }
            else if ( !bCtrl && !bMod2 )
            {
                if ( mnCurrentPos )
                {
                    nSelect = mpEntryList->GetEntryCount() ? 0 : LISTBOX_ENTRY_NOTFOUND;
                    if( mnTop != 0 )
                        SetTopEntry( 0 );

                    bDone = TRUE;
                }
            }
        }
        break;

        case KEY_END:
        {
            if ( IsReadOnly() )
            {
                SetTopEntry( 0xFFFF );
            }
            else if ( !bCtrl && !bMod2 )
            {
                if( mnCurrentPos == LISTBOX_ENTRY_NOTFOUND )
                    nSelect = mpEntryList->GetEntryCount() ? 0 : LISTBOX_ENTRY_NOTFOUND;
                else if ( (mnCurrentPos+1) < mpEntryList->GetEntryCount() )
                {
                    USHORT nCount = mpEntryList->GetEntryCount();
                    nSelect = nCount - 1;
                    if( nCount > mnMaxVisibleEntries )
                        SetTopEntry( nCount - mnMaxVisibleEntries );
                }
                bDone = TRUE;
            }
        }
        break;

        case KEY_LEFT:
        {
            if ( !bCtrl && !bMod2 )
            {
                ScrollHorz( -HORZ_SCROLL );
                bDone = TRUE;
            }
        }
        break;

        case KEY_RIGHT:
        {
            if ( !bCtrl && !bMod2 )
            {
                ScrollHorz( HORZ_SCROLL );
                bDone = TRUE;
            }
        }
        break;

        case KEY_RETURN:
        {
            if ( !bMod2 && !IsReadOnly() )
            {
                mnSelectModifier = rKEvt.GetKeyCode().GetModifier();
                ImplCallSelect();
                bDone = FALSE;  // RETURN nicht abfangen.
            }
        }
        break;

        case KEY_SPACE:
        {
            if ( !bMod2 && !IsReadOnly() )
            {
                if( mbMulti && ( !mbSimpleMode || ( mbSimpleMode && bCtrl && !bShift ) ) )
                {
                    nSelect = mnCurrentPos;
                    eLET = LET_KEYSPACE;
                }
                bDone = TRUE;
            }
        }
        break;

        default:
        {
            xub_Unicode c = rKEvt.GetCharCode();

            if ( !IsReadOnly() && (c >= 32) && (c != 127) &&
                 !rKEvt.GetKeyCode().IsControlMod() )
            {
                maSearchStr += c;
                XubString aTmpSearch( maSearchStr );

                nSelect = mpEntryList->FindEntry( aTmpSearch, MATCH_IGNORECASE, aTmpSearch.Len(), mnCurrentPos );
                if ( (nSelect == LISTBOX_ENTRY_NOTFOUND) && (aTmpSearch.Len() > 1) )
                {
                    // Wenn alles die gleichen Buchstaben, dann anderer Such-Modus
                    BOOL bAllEqual = TRUE;
                    for ( USHORT n = aTmpSearch.Len(); n && bAllEqual; )
                        bAllEqual = aTmpSearch.GetChar( --n ) == c;
                    if ( bAllEqual )
                    {
                        aTmpSearch = c;
                        nSelect = mpEntryList->FindEntry( aTmpSearch, MATCH_IGNORECASE, aTmpSearch.Len(), mnCurrentPos+1 );
                    }
                }
                if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
                    nSelect = mpEntryList->FindEntry( aTmpSearch, MATCH_IGNORECASE, aTmpSearch.Len(), 0 );

                if ( nSelect != LISTBOX_ENTRY_NOTFOUND )
                {
                    if( nSelect < mnTop )
                        SetTopEntry( nSelect );
                    else if( nSelect >= (mnTop + mnMaxVisibleEntries) )
                        SetTopEntry( nSelect - mnMaxVisibleEntries + 1 );

                    if ( nSelect == mnCurrentPos )
                        nSelect = LISTBOX_ENTRY_NOTFOUND;

                    maSearchTimeout.Start();
                }
                else
                    maSearchStr.Erase();
                bDone = TRUE;
            }
        }
    }

    if ( (nSelect != LISTBOX_ENTRY_NOTFOUND) &&
         ((nSelect != mnCurrentPos ) || ( eLET == LET_KEYSPACE)) )
    {
        DBG_ASSERT( (nSelect != mnCurrentPos) || mbMulti, "ImplListBox: Selecting same Entry" );
        mnCurrentPos = nSelect;
        if ( SelectEntries( nSelect, eLET, bShift, bCtrl ) )
        {
            mbTravelSelect = TRUE;
            mnSelectModifier = rKEvt.GetKeyCode().GetModifier();
            ImplCallSelect();
            mbTravelSelect = FALSE;
        }
    }

    return bDone;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ImplPaint( USHORT nPos, BOOL bErase )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    long nWidth  = GetOutputSizePixel().Width();
    long nY = ( nPos - mnTop ) * mnMaxHeight;
    Rectangle aRect( Point( 0, nY ), Size( nWidth, mnMaxHeight ) );

    if( IsEnabled() )
    {
        if( mpEntryList->IsEntryPosSelected( nPos ) )
        {
            SetTextColor( rStyleSettings.GetHighlightTextColor() );
            SetFillColor( rStyleSettings.GetHighlightColor() );
            DrawRect( aRect );
        }
        else
        {
            ImplInitSettings( FALSE, TRUE, FALSE );
            if( bErase )
                Erase( aRect );
        }
    }
    else // Disabled
    {
        SetTextColor( rStyleSettings.GetDisableColor() );
        //SetFillColor( rStyleSettings.Get???Color() );
        //DrawRect( aRect );
        if( bErase )
            Erase( aRect );
    }

    if ( IsUserDrawEnabled() )
    {
        mbInUserDraw = TRUE;
        mnUserDrawEntry = nPos;
        aRect.Left() -= mnLeft;
        if ( nPos < GetEntryList()->GetMRUCount() )
            nPos = GetEntryList()->FindEntry( GetEntryList()->GetEntryText( nPos ), MATCH_CASE, STRING_LEN, GetEntryList()->GetMRUCount() );
        nPos -= GetEntryList()->GetMRUCount();
        UserDrawEvent aUDEvt( this, aRect, nPos, 0 );
        maUserDrawHdl.Call( &aUDEvt );
        mbInUserDraw = FALSE;
    }
    else
    {
        DrawEntry( nPos, TRUE, TRUE );
    }
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::DrawEntry( USHORT nPos, BOOL bDrawImage, BOOL bDrawText, BOOL bDrawTextAtImagePos )
{
    // Bei Aenderungen in dieser Methode ggf. auch ImplWin::DrawEntry() anpassen.

    if ( mbInUserDraw )
        nPos = mnUserDrawEntry; // real entry, not the matching entry from MRU

    long nY = ( nPos - mnTop ) * mnMaxHeight;

    if( bDrawImage && mpEntryList->HasImages() )
    {
        Image aImage = mpEntryList->GetEntryImage( nPos );
        if( !!aImage )
        {
            Size aImgSz = aImage.GetSizePixel();
            Point aPtImg( mnBorder - mnLeft, nY + ( ( mnMaxHeight - aImgSz.Height() ) / 2 ) );

            if ( !IsZoom() )
            {
                DrawImage( aPtImg, aImage );
            }
            else
            {
                aImgSz.Width() = CalcZoom( aImgSz.Width() );
                aImgSz.Height() = CalcZoom( aImgSz.Height() );
                DrawImage( aPtImg, aImgSz, aImage );
            }
        }
    }

    if( bDrawText )
    {
        XubString aStr( mpEntryList->GetEntryText( nPos ) );
        if ( aStr.Len() )
        {
            Point aPtTxt( mnBorder - mnLeft, nY + ( ( mnMaxHeight - mnMaxTxtHeight ) / 2 ) );
            if( !bDrawTextAtImagePos && ( mpEntryList->HasEntryImage(nPos) || IsUserDrawEnabled() ) )
            {
                USHORT nMaxWidth = Max( mnMaxImgWidth, (USHORT)maUserItemSize.Width() );
                aPtTxt.X() += nMaxWidth + IMG_TXT_DISTANCE;
            }
            DrawText( aPtTxt, aStr );
        }
    }

    if ( ( mnSeparatorPos != LISTBOX_ENTRY_NOTFOUND ) &&
         ( ( nPos == mnSeparatorPos ) || ( nPos == mnSeparatorPos+1 ) ) )
    {
        Color aOldLineColor( GetLineColor() );
        SetLineColor( ( GetBackground().GetColor() != COL_LIGHTGRAY ) ? COL_LIGHTGRAY : COL_GRAY );
        Point aStartPos( 0, nY );
        if ( nPos == mnSeparatorPos )
            aStartPos.Y() += mnMaxHeight-1;
        Point aEndPos( aStartPos );
        aEndPos.X() = GetOutputSizePixel().Width();
        DrawLine( aStartPos, aEndPos );
        SetLineColor( aOldLineColor );
    }
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::Paint( const Rectangle& rRect )
{
    USHORT nCount = mpEntryList->GetEntryCount();

    if( HasFocus() )
        HideFocus();

    long nY = 0; // + mnBorder;
    long nHeight = GetOutputSizePixel().Height();// - mnMaxHeight + mnBorder;

    for( USHORT i = (USHORT)mnTop; i < nCount && nY < nHeight + mnMaxHeight; i++ )
    {
        if( nY + mnMaxHeight >= rRect.Top() &&
            nY <= rRect.Bottom() + mnMaxHeight )
        {
            ImplPaint( i );
        }
        nY += mnMaxHeight;
    }

    maFocusRect.SetPos( Point( 0, ( mnCurrentPos - mnTop ) * mnMaxHeight ) );
    if( HasFocus() )
        ShowFocus( maFocusRect );
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::Resize()
{
    Size aSz( GetOutputSizePixel().Width(), mnMaxHeight );
    maFocusRect.SetSize( aSz );
    mnMaxVisibleEntries = (USHORT) ( GetOutputSizePixel().Height() / mnMaxHeight );
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::GetFocus()
{
    USHORT nPos = mnCurrentPos;
    if ( nPos == LISTBOX_ENTRY_NOTFOUND )
        nPos = 0;
    maFocusRect.SetPos( Point( 0, ( nPos - mnTop ) * mnMaxHeight ) );
    ShowFocus( maFocusRect );
    Control::GetFocus();
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::LoseFocus()
{
    HideFocus();
    Control::LoseFocus();
}

// -----------------------------------------------------------------------

/*
void ImplListBoxWindow::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & HELPMODE_BALLOON )
        Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), String() );

    Window::RequestHelp( rHEvt );
}
*/

// -----------------------------------------------------------------------

void ImplListBoxWindow::SetTopEntry( USHORT nTop )
{
    USHORT nMaxTop = 0;
    if ( GetEntryList()->GetEntryCount() > mnMaxVisibleEntries )
        nMaxTop = GetEntryList()->GetEntryCount() - mnMaxVisibleEntries;
    nTop = Min( nTop, nMaxTop );
    if ( nTop != mnTop )
    {
        HideFocus();
        long nDiff = ( mnTop - nTop ) * mnMaxHeight;
        mnTop = nTop;
        Scroll( 0, nDiff );
        maFocusRect.Top() += nDiff;
        maFocusRect.Bottom() += nDiff;
        if( HasFocus() )
            ShowFocus( maFocusRect );
        maScrollHdl.Call( this );
    }
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::SetLeftIndent( USHORT n )
{
    ScrollHorz( n - mnLeft );
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::ScrollHorz( short n )
{
    short nDiff = 0;
    if ( n > 0 )
    {
        long nWidth = GetOutputSizePixel().Width();
        if( ( mnMaxWidth - mnLeft + n ) > nWidth )
            nDiff = n;
    }
    else if ( n < 0 )
    {
        if( mnLeft )
        {
            USHORT nAbs = -n;
            nDiff = - ( ( mnLeft > nAbs ) ? nAbs : mnLeft );
        }
    }

    if ( nDiff )
    {
        HideFocus();
        mnLeft += nDiff;
        Scroll( -nDiff, 0 );
        if( HasFocus() )
            ShowFocus( maFocusRect );
        maScrollHdl.Call( this );
    }
}

// -----------------------------------------------------------------------

Size ImplListBoxWindow::CalcSize( USHORT nMaxLines ) const
{
    Size aSz;
//  USHORT nL = Min( nMaxLines, mpEntryList->GetEntryCount() );
    aSz.Height() =  nMaxLines * mnMaxHeight;
    aSz.Width() = mnMaxWidth + 2*mnBorder;
    return aSz;
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == STATE_CHANGE_ZOOM )
    {
        ImplInitSettings( TRUE, FALSE, FALSE );
        ImplCalcMetrics();
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsUpdateMode() && IsReallyVisible() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        ImplInitSettings( TRUE, FALSE, FALSE );
        ImplCalcMetrics();
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( FALSE, TRUE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( FALSE, FALSE, TRUE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void ImplListBoxWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( TRUE, TRUE, TRUE );
        ImplCalcMetrics();
        Invalidate();
    }
}

// =======================================================================

ImplListBox::ImplListBox( Window* pParent, WinBits nWinStyle ) :
    Control( pParent, nWinStyle ),
    maLBWindow( this, nWinStyle&(~WB_BORDER) )
{
    mpVScrollBar    = new ScrollBar( this, WB_VSCROLL | WB_DRAG );
    mpHScrollBar    = new ScrollBar( this, WB_HSCROLL | WB_DRAG );
    mpScrollBarBox  = new ScrollBarBox( this );

    Link aLink( LINK( this, ImplListBox, ScrollBarHdl ) );
    mpVScrollBar->SetScrollHdl( aLink );
    mpHScrollBar->SetScrollHdl( aLink );

    mbVScroll       = FALSE;
    mbHScroll       = FALSE;
    mbAutoHScroll   = ( nWinStyle & WB_AUTOHSCROLL ) ? TRUE : FALSE;

    maLBWindow.SetScrollHdl( LINK( this, ImplListBox, LBWindowScrolled ) );
    maLBWindow.SetMRUChangedHdl( LINK( this, ImplListBox, MRUChanged ) );
    maLBWindow.Show();
}

// -----------------------------------------------------------------------

ImplListBox::~ImplListBox()
{
    delete mpHScrollBar;
    delete mpVScrollBar;
    delete mpScrollBarBox;
}

// -----------------------------------------------------------------------

void ImplListBox::Clear()
{
    maLBWindow.Clear();
    if ( GetEntryList()->GetMRUCount() )
    {
        maLBWindow.GetEntryList()->SetMRUCount( 0 );
        maLBWindow.SetSeparatorPos( LISTBOX_ENTRY_NOTFOUND );
    }
    mpVScrollBar->SetThumbPos( 0 );
    mpHScrollBar->SetThumbPos( 0 );
    StateChanged( STATE_CHANGE_DATA );
}

// -----------------------------------------------------------------------

USHORT ImplListBox::InsertEntry( USHORT nPos, const XubString& rStr )
{
    ImplEntryType* pNewEntry = new ImplEntryType( rStr );
    USHORT nNewPos = maLBWindow.InsertEntry( nPos, pNewEntry );
    StateChanged( STATE_CHANGE_DATA );
    return nNewPos;
}

// -----------------------------------------------------------------------

USHORT ImplListBox::InsertEntry( USHORT nPos, const Image& rImage )
{
    ImplEntryType* pNewEntry = new ImplEntryType( rImage );
    USHORT nNewPos = maLBWindow.InsertEntry( nPos, pNewEntry );
    StateChanged( STATE_CHANGE_DATA );
    return nNewPos;
}

// -----------------------------------------------------------------------

USHORT ImplListBox::InsertEntry( USHORT nPos, const XubString& rStr, const Image& rImage )
{
    ImplEntryType* pNewEntry = new ImplEntryType( rStr, rImage );
    USHORT nNewPos = maLBWindow.InsertEntry( nPos, pNewEntry );
    StateChanged( STATE_CHANGE_DATA );
    return nNewPos;
}

// -----------------------------------------------------------------------

void ImplListBox::RemoveEntry( USHORT nPos )
{
    maLBWindow.RemoveEntry( nPos );
    StateChanged( STATE_CHANGE_DATA );
}

// -----------------------------------------------------------------------

void ImplListBox::SelectEntry( USHORT nPos, BOOL bSelect )
{
    maLBWindow.SelectEntry( nPos, bSelect );
}

// -----------------------------------------------------------------------

void ImplListBox::SetNoSelection()
{
    USHORT nSelected = GetEntryList()->GetSelectEntryCount();
    for ( USHORT n = 0; n < nSelected; n++ )
    {
        USHORT nS = GetEntryList()->GetSelectEntryPos( n );
        SelectEntry( nS, FALSE );
    }
    maLBWindow.GetEntryList()->SetNoSelection();
}

// -----------------------------------------------------------------------

void ImplListBox::GetFocus()
{
    maLBWindow.GrabFocus();
}

// -----------------------------------------------------------------------

void ImplListBox::Resize()
{
    ImplResizeControls();
    ImplCheckScrollBars();
}


// -----------------------------------------------------------------------

IMPL_LINK( ImplListBox, MRUChanged, void*, EMPTYARG )
{
    StateChanged( STATE_CHANGE_DATA );
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplListBox, LBWindowScrolled, void*, EMPTYARG )
{
    mpVScrollBar->SetThumbPos( GetTopEntry() );
    mpHScrollBar->SetThumbPos( GetLeftIndent() );

    maScrollHdl.Call( this );

    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplListBox, ScrollBarHdl, ScrollBar*, pSB )
{
    USHORT nPos = (USHORT) pSB->GetThumbPos();
    if( pSB == mpVScrollBar )
        SetTopEntry( nPos );
    else if( pSB == mpHScrollBar )
        SetLeftIndent( nPos );

    return 1;
}

// -----------------------------------------------------------------------

void ImplListBox::ImplCheckScrollBars()
{
    BOOL bArrange = FALSE;

    Size aOutSz = GetOutputSizePixel();
    USHORT nEntries = GetEntryList()->GetEntryCount();
    USHORT nMaxVisEntries = (USHORT) (aOutSz.Height() / GetEntryHeight());

    // vert. ScrollBar
    if( nEntries > nMaxVisEntries )
    {
        if( !mbVScroll )
            bArrange = TRUE;
        mbVScroll = TRUE;

        // Ueberpruefung des rausgescrollten Bereichs
        SetTopEntry( GetTopEntry() );   // MaxTop wird geprueft...
    }
    else
    {
        if( mbVScroll )
            bArrange = TRUE;
        mbVScroll = FALSE;
        SetTopEntry( 0 );
    }

    // horz. ScrollBar
    if( mbAutoHScroll )
    {
        long nWidth = (USHORT) aOutSz.Width();
        if ( mbVScroll )
            nWidth -= mpVScrollBar->GetSizePixel().Width();

        long nMaxWidth = GetMaxEntryWidth();
        if( nWidth < nMaxWidth )
        {
            if( !mbHScroll )
                bArrange = TRUE;
            mbHScroll = TRUE;

            if ( !mbVScroll )   // ggf. brauchen wir jetzt doch einen
            {
                nMaxVisEntries = (USHORT) ( ( aOutSz.Height() - mpHScrollBar->GetSizePixel().Height() ) / GetEntryHeight() );
                if( nEntries > nMaxVisEntries )
                {
                    bArrange = TRUE;
                    mbVScroll = TRUE;

                    // Ueberpruefung des rausgescrollten Bereichs
                    SetTopEntry( GetTopEntry() );   // MaxTop wird geprueft...
                }
            }

            // Ueberpruefung des rausgescrollten Bereichs
            USHORT nMaxLI = (USHORT) (nMaxWidth - nWidth);
            if ( nMaxLI < GetLeftIndent() )
                SetLeftIndent( nMaxLI );
        }
        else
        {
            if( mbHScroll )
                bArrange = TRUE;
            mbHScroll = FALSE;
            SetLeftIndent( 0 );
        }
    }

    if( bArrange )
        ImplResizeControls();

    ImplInitScrollBars();
}

// -----------------------------------------------------------------------

void ImplListBox::ImplInitScrollBars()
{
    Size aOutSz = maLBWindow.GetOutputSizePixel();

    if ( mbVScroll )
    {
        USHORT nEntries = GetEntryList()->GetEntryCount();
        USHORT nVisEntries = (USHORT) (aOutSz.Height() / GetEntryHeight());
        mpVScrollBar->SetRangeMax( nEntries );
        mpVScrollBar->SetVisibleSize( nVisEntries );
        mpVScrollBar->SetPageSize( nVisEntries - 1 );
    }

    if ( mbHScroll )
    {
        mpHScrollBar->SetRangeMax( GetMaxEntryWidth() + HORZ_SCROLL );
        mpHScrollBar->SetVisibleSize( (USHORT)aOutSz.Width() );
        mpHScrollBar->SetLineSize( HORZ_SCROLL );
        mpHScrollBar->SetPageSize( aOutSz.Width() - HORZ_SCROLL );
    }
}

// -----------------------------------------------------------------------

void ImplListBox::ImplResizeControls()
{
    // Hier werden die Controls nur angeordnet, ob die Scrollbars
    // sichtbar sein sollen wird bereits in ImplCheckScrollBars ermittelt.

    Size aOutSz = GetOutputSizePixel();
    long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
    nSBWidth = CalcZoom( nSBWidth );

    Size aInnerSz( aOutSz );
    if ( mbVScroll )
        aInnerSz.Width() -= nSBWidth;
    if ( mbHScroll )
        aInnerSz.Height() -= nSBWidth;

    maLBWindow.SetPosSizePixel( Point(), aInnerSz );

    // ScrollBarBox
    if( mbVScroll && mbHScroll )
    {
        mpScrollBarBox->SetPosSizePixel( Point( aInnerSz.Width(), aInnerSz.Height() ),
                                         Size( nSBWidth, nSBWidth ) );
        mpScrollBarBox->Show();
    }
    else
    {
        mpScrollBarBox->Hide();
    }

    // vert. ScrollBar
    if( mbVScroll )
    {
        mpVScrollBar->SetPosSizePixel( Point( aOutSz.Width()-nSBWidth, 0 ),
                                       Size( nSBWidth, aInnerSz.Height() ) );
        mpVScrollBar->Show();
    }
    else
    {
        mpVScrollBar->Hide();
        SetTopEntry( 0 );
    }

    // horz. ScrollBar
    if( mbHScroll )
    {
        mpHScrollBar->SetPosSizePixel( Point( 0, aOutSz.Height()-nSBWidth ),
                                       Size( aInnerSz.Width(), nSBWidth ) );
        mpHScrollBar->Show();
    }
    else
    {
        mpHScrollBar->Hide();
        SetLeftIndent( 0 );
    }
}

// -----------------------------------------------------------------------

void ImplListBox::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_INITSHOW )
    {
        ImplCheckScrollBars();
    }
    else if ( ( nType == STATE_CHANGE_UPDATEMODE ) || ( nType == STATE_CHANGE_DATA ) )
    {
        BOOL bUpdate = IsUpdateMode();
        maLBWindow.SetUpdateMode( bUpdate );
//      mpHScrollBar->SetUpdateMode( bUpdate );
//      mpVScrollBar->SetUpdateMode( bUpdate );
        if ( bUpdate && IsReallyVisible() )
            ImplCheckScrollBars();
    }
    else if( nType == STATE_CHANGE_ENABLE )
    {
        mpHScrollBar->Enable( IsEnabled() );
        mpVScrollBar->Enable( IsEnabled() );
        mpScrollBarBox->Enable( IsEnabled() );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        maLBWindow.SetZoom( GetZoom() );
        Resize();
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        maLBWindow.SetControlFont( GetControlFont() );
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        maLBWindow.SetControlForeground( GetControlForeground() );
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        maLBWindow.SetControlBackground( GetControlBackground() );
    }

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void ImplListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
//  if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
//       (rDCEvt.GetFlags() & SETTINGS_STYLE) )
//  {
//      maLBWindow.SetSettings( GetSettings() );
//      Resize();
//  }
//  else
        Control::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

long ImplListBox::Notify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if ( rNEvt.GetType() == EVENT_COMMAND )
    {
        const CommandEvent& rCEvt = *rNEvt.GetCommandEvent();
        if ( rCEvt.GetCommand() == COMMAND_WHEEL )
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();
            if( !pData->GetModifier() && ( pData->GetMode() == COMMAND_WHEEL_SCROLL ) )
            {
                nDone = HandleScrollCommand( rCEvt, mpHScrollBar, mpVScrollBar );
            }
        }
    }

    return nDone ? nDone : Window::Notify( rNEvt );
}

// -----------------------------------------------------------------------

BOOL ImplListBox::HandleWheelAsCursorTravel( const CommandEvent& rCEvt )
{
    BOOL bDone = FALSE;
    if ( rCEvt.GetCommand() == COMMAND_WHEEL )
    {
        const CommandWheelData* pData = rCEvt.GetWheelData();
        if( !pData->GetModifier() && ( pData->GetMode() == COMMAND_WHEEL_SCROLL ) )
        {
            USHORT nKey = ( pData->GetDelta() < 0 ) ? KEY_DOWN : KEY_UP;
            KeyEvent aKeyEvent( 0, KeyCode( nKey ) );
            bDone = ProcessKeyInput( aKeyEvent );
        }
    }
    return bDone;
}

// -----------------------------------------------------------------------

void ImplListBox::SetMRUEntries( const XubString& rEntries, xub_Unicode cSep )
{
    BOOL bChanges = GetEntryList()->GetMRUCount() ? TRUE : FALSE;

    // Remove old MRU entries
    for ( USHORT n = GetEntryList()->GetMRUCount();n; )
        maLBWindow.RemoveEntry( --n );

    USHORT nMRUCount = 0;
    USHORT nEntries = rEntries.GetTokenCount( cSep );
    for ( USHORT nEntry = 0; nEntry < nEntries; nEntry++ )
    {
        XubString aEntry = rEntries.GetToken( nEntry, cSep );
        // Accept only existing entries
        if ( GetEntryList()->FindEntry( aEntry ) != LISTBOX_ENTRY_NOTFOUND )
        {
            ImplEntryType* pNewEntry = new ImplEntryType( aEntry );
            maLBWindow.GetEntryList()->InsertEntry( nMRUCount++, pNewEntry, FALSE );
            bChanges = TRUE;
        }
    }

    if ( bChanges )
    {
        maLBWindow.GetEntryList()->SetMRUCount( nMRUCount );
        SetSeparatorPos( nMRUCount ? nMRUCount-1 : 0 );
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

XubString ImplListBox::GetMRUEntries( xub_Unicode cSep ) const
{
    String aEntries;
    for ( USHORT n = 0; n < GetEntryList()->GetMRUCount(); n++ )
    {
        aEntries += GetEntryList()->GetEntryText( n );
        if( n < ( GetEntryList()->GetMRUCount() - 1 ) )
            aEntries += cSep;
    }
    return aEntries;
}

// =======================================================================

ImplWin::ImplWin( Window* pParent, WinBits nWinStyle ) :
    Control ( pParent, nWinStyle )
{
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFieldColor() ) );
    mbInUserDraw = FALSE;
    mbUserDrawEnabled = FALSE;
    mnItemPos = LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

void ImplWin::MBDown()
{
    if( IsEnabled() )
        maMBDownHdl.Call( this );
}

// -----------------------------------------------------------------------

void ImplWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( IsEnabled() )
    {
//      Control::MouseButtonDown( rMEvt );
        MBDown();
    }
}

// -----------------------------------------------------------------------

void ImplWin::Paint( const Rectangle& rRect )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if( IsEnabled() )
    {
        if( HasFocus() )
        {
            SetTextColor( rStyleSettings.GetHighlightTextColor() );
            SetFillColor( rStyleSettings.GetHighlightColor() );
            DrawRect( maFocusRect );
        }
        else
        {
            Color aColor = rStyleSettings.GetFieldTextColor();
            if( IsControlForeground() )
                aColor = GetControlForeground();
            SetTextColor( aColor );
            Erase( maFocusRect );
        }
    }
    else // Disabled
    {
        SetTextColor( rStyleSettings.GetDisableColor() );
        Erase( maFocusRect );
    }

    if ( IsUserDrawEnabled() )
    {
        mbInUserDraw = TRUE;
        UserDrawEvent aUDEvt( this, maFocusRect, mnItemPos, 0 );
        maUserDrawHdl.Call( &aUDEvt );
        mbInUserDraw = FALSE;
    }
    else
    {
        DrawEntry( TRUE, TRUE );
    }
}

// -----------------------------------------------------------------------

void ImplWin::DrawEntry( BOOL bDrawImage, BOOL bDrawText, BOOL bDrawTextAtImagePos )
{
    long nBorder = 1;
    Size aOutSz = GetOutputSizePixel();

    BOOL bImage = !!maImage;
    if( bDrawImage && bImage )
    {
        Size aImgSz = maImage.GetSizePixel();
        Point aPtImg( nBorder, ( ( aOutSz.Height() - aImgSz.Height() ) / 2 ) );

        if ( !IsZoom() )
        {
            DrawImage( aPtImg, maImage );
        }
        else
        {
            aImgSz.Width() = CalcZoom( aImgSz.Width() );
            aImgSz.Height() = CalcZoom( aImgSz.Height() );
            DrawImage( aPtImg, aImgSz, maImage );
        }
    }

    if( bDrawText && maString.Len() )
    {
        long nTextHeight = GetTextHeight();
        Point aPtTxt( nBorder, (aOutSz.Height()-nTextHeight)/2 );
        if ( !bDrawTextAtImagePos && ( bImage || IsUserDrawEnabled() ) )
        {
            long nMaxWidth = Max( maImage.GetSizePixel().Width(), maUserItemSize.Width() );
            aPtTxt.X() += nMaxWidth + IMG_TXT_DISTANCE;
        }
        DrawText( aPtTxt, maString );
    }

    if( HasFocus() )
        ShowFocus( maFocusRect );
}

// -----------------------------------------------------------------------

void ImplWin::Resize()
{
    maFocusRect.SetSize( GetOutputSizePixel() );
    Invalidate();
}

// -----------------------------------------------------------------------

void ImplWin::GetFocus()
{
    ShowFocus( maFocusRect );
    Invalidate();
    Control::GetFocus();
}

// -----------------------------------------------------------------------

void ImplWin::LoseFocus()
{
    HideFocus();
    Invalidate();
    Control::LoseFocus();
}

// =======================================================================

ImplBtn::ImplBtn( Window* pParent, WinBits nWinStyle ) :
    PushButton(  pParent, nWinStyle ),
    mbDown  ( FALSE )
{
}

// -----------------------------------------------------------------------

void ImplBtn::MBDown()
{
    if( IsEnabled() )
       maMBDownHdl.Call( this );
}

// -----------------------------------------------------------------------

void ImplBtn::MouseButtonDown( const MouseEvent& rMEvt )
{
    //PushButton::MouseButtonDown( rMEvt );
    if( IsEnabled() )
    {
        MBDown();
        mbDown = TRUE;
    }
}

// =======================================================================

ImplListBoxFloatingWindow::ImplListBoxFloatingWindow( Window* pParent ) :
    FloatingWindow( pParent, WB_NOBORDER )
{
    mpImplLB = NULL;
    mnDDLineCount = 0;
    mbAutoWidth = FALSE;

    EnableSaveBackground();
}

// -----------------------------------------------------------------------

long ImplListBoxFloatingWindow::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if( !GetParent()->HasChildPathFocus( TRUE ) )
            EndPopupMode();
    }

    return FloatingWindow::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void ImplListBoxFloatingWindow::SetPosSizePixel( long nX, long nY, long nWidth, long nHeight, USHORT nFlags )
{
    FloatingWindow::SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );

    // Fix #60890# ( MBA ): um auch im aufgeklappten Zustand der Listbox die Gr"o\se einfach zu einen
    // Aufruf von Resize() "andern zu k"onnen, wird die Position hier ggf. angepa\t
    if ( IsReallyVisible() && ( nFlags & WINDOW_POSSIZE_HEIGHT ) )
    {
        Point aPos = GetParent()->GetPosPixel();
        aPos = GetParent()->GetParent()->OutputToScreenPixel( aPos );

        if ( nFlags & WINDOW_POSSIZE_X )
            aPos.X() = nX;

        if ( nFlags & WINDOW_POSSIZE_Y )
            aPos.Y() = nY;

        USHORT nIndex;
        SetPosPixel( ImplCalcPos( this, Rectangle( aPos, GetParent()->GetSizePixel() ), FLOATWIN_POPUPMODE_DOWN, nIndex ) );
    }

//  if( !IsReallyVisible() )
    {
        // Die ImplListBox erhaelt kein Resize, weil nicht sichtbar.
        // Die Fenster muessen aber ein Resize() erhalten, damit die
        // Anzahl der sichtbaren Eintraege fuer PgUp/PgDown stimmt.
        // Die Anzahl kann auch nicht von List/Combobox berechnet werden,
        // weil hierfuer auch die ggf. vorhandene vertikale Scrollbar
        // beruecksichtigt werden muss.
        mpImplLB->SetSizePixel( GetOutputSizePixel() );
        ((Window*)mpImplLB)->Resize();
        ((Window*)mpImplLB->GetMainWindow())->Resize();
    }
}

// -----------------------------------------------------------------------

Size ImplListBoxFloatingWindow::CalcFloatSize()
{
    Size aFloatSz( maPrefSz );

    long nLeft, nTop, nRight, nBottom;
    GetBorder( nLeft, nTop, nRight, nBottom );

    USHORT nLines = mpImplLB->GetEntryList()->GetEntryCount();
    if ( mnDDLineCount && ( nLines > mnDDLineCount ) )
        nLines = mnDDLineCount;

    Size aSz = mpImplLB->CalcSize( nLines );
    long nMaxHeight = aSz.Height() + nTop + nBottom;

    if ( mnDDLineCount )
        aFloatSz.Height() = nMaxHeight;

    if( mbAutoWidth )
    {
        // AutoSize erstmal nur fuer die Breite...

        aFloatSz.Width() = aSz.Width() + nLeft + nRight;
        aFloatSz.Width() += nRight; // etwas mehr Platz sieht besser aus...

        if ( aFloatSz.Height() < nMaxHeight )
        {
            // dann wird noch der vertikale Scrollbar benoetigt
            long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
            aFloatSz.Width() += nSBWidth;
        }
    }

    if ( aFloatSz.Height() > nMaxHeight )
        aFloatSz.Height() = nMaxHeight;

    // Minimale Hoehe, falls Hoehe nicht auf Float-Hoehe eingestellt wurde.
    // Der Parent vom FloatWin muss die DropDown-Combo/Listbox sein.
    Size aParentSz = GetParent()->GetSizePixel();
    if( aFloatSz.Height() < aParentSz.Height() )
        aFloatSz.Height() = aParentSz.Height();

    // Nicht schmaler als der Parent werden...
    if( aFloatSz.Width() < aParentSz.Width() )
        aFloatSz.Width() = aParentSz.Width();

    // Hoehe auf Entries alignen...
    long nInnerHeight = aFloatSz.Height() - nTop - nBottom;
    long nEntryHeight = mpImplLB->GetEntryHeight();
    if ( nInnerHeight % nEntryHeight )
    {
        nInnerHeight /= nEntryHeight;
        nInnerHeight++;
        nInnerHeight *= nEntryHeight;
        aFloatSz.Height() = nInnerHeight + nTop + nBottom;
    }

    return aFloatSz;
}

// -----------------------------------------------------------------------

void ImplListBoxFloatingWindow::StartFloat( BOOL bStartTracking )
{
    if( !IsInPopupMode() )
    {
        Size aFloatSz = CalcFloatSize();

        SetSizePixel( aFloatSz );
        mpImplLB->SetSizePixel( GetOutputSizePixel() );

        Size aSz = GetParent()->GetSizePixel();
        Point aPos = GetParent()->GetPosPixel();
        aPos = GetParent()->GetParent()->OutputToScreenPixel( aPos );
        Rectangle aRect( aPos, aSz );
        StartPopupMode( aRect, FLOATWIN_POPUPMODE_DOWN );

        USHORT nPos = mpImplLB->GetEntryList()->GetSelectEntryPos( 0 );
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            mpImplLB->SetTopEntry( nPos );

        if( bStartTracking )
            mpImplLB->GetMainWindow()->EnableMouseMoveSelect( TRUE );

        if ( mpImplLB->GetMainWindow()->IsGrabFocusAllowed() )
            mpImplLB->GetMainWindow()->GrabFocus();
    }
}

