/*************************************************************************
 *
 *  $RCSfile: lstbox.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:36 $
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

#define _SV_LSTBOX_CXX

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <scrbar.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <button.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <edit.hxx>
#endif
#ifndef _SV_SUBEDIT_HXX
#include <subedit.hxx>
#endif
#ifndef _SV_ILSTBOX_HXX
#include <ilstbox.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <lstbox.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <combobox.hxx>
#endif

#pragma hdrstop

// =======================================================================

ListBox::ListBox( WindowType nType ) : Control( nType )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

ListBox::ListBox( Window* pParent, WinBits nStyle ) : Control( WINDOW_LISTBOX )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ListBox::ListBox( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_LISTBOX )
{
    ImplInitData();
    rResId.SetRT( RSC_LISTBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

ListBox::~ListBox()
{
    delete mpImplLB;

    // Beim zerstoeren des FloatWins macht TH ein GrabFocus auf den Parent,
    // also diese ListBox => PreNotify()...
    mpImplLB = NULL;

    delete mpFloatWin;
    delete mpImplWin;
    delete mpBtn;
}

// -----------------------------------------------------------------------

void ListBox::ImplInitData()
{
    mpFloatWin      = NULL;
    mpImplWin       = NULL;
    mpBtn           = NULL;

    mnDDHeight      = 0;
    mbDDAutoSize    = TRUE;
    mnSaveValue     = LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

void ListBox::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    if ( !(nStyle & WB_NOBORDER) && ( nStyle & WB_DROPDOWN ) )
        nStyle |= WB_BORDER;

    Control::ImplInit( pParent, nStyle, NULL );
    SetBackground();

    if( nStyle & WB_DROPDOWN )
    {
        long nLeft, nTop, nRight, nBottom;
        GetBorder( nLeft, nTop, nRight, nBottom );
        mnDDHeight = (USHORT)(GetTextHeight() + nTop + nBottom + 4);

        mpFloatWin  = new ImplListBoxFloatingWindow( this );
        mpFloatWin->SetAutoWidth( TRUE );
        mpFloatWin->SetPopupModeEndHdl( LINK( this, ListBox, ImplPopupModeEndHdl ) );

        mpImplWin = new ImplWin( this, WB_NOBORDER );
        mpImplWin->SetMBDownHdl( LINK( this, ListBox, ImplClickBtnHdl ) );
        mpImplWin->SetUserDrawHdl( LINK( this, ListBox, ImplUserDrawHdl ) );
        mpImplWin->Show();

        mpBtn = new ImplBtn( this, WB_NOLIGHTBORDER | WB_RECTSTYLE );
        ImplInitDropDownButton( mpBtn );
        mpBtn->SetMBDownHdl( LINK( this, ListBox, ImplClickBtnHdl ) );
        mpBtn->Show();

    }

    Window* pLBParent = this;
    if ( mpFloatWin )
        pLBParent = mpFloatWin;
    mpImplLB = new ImplListBox( pLBParent, nStyle&(~WB_BORDER) );
    mpImplLB->SetSelectHdl( LINK( this, ListBox, ImplSelectHdl ) );
    mpImplLB->SetCancelHdl( LINK( this, ListBox, ImplCancelHdl ) );
    mpImplLB->SetDoubleClickHdl( LINK( this, ListBox, ImplDoubleClickHdl ) );
    mpImplLB->SetUserDrawHdl( LINK( this, ListBox, ImplUserDrawHdl ) );
    mpImplLB->SetPosPixel( Point() );
    mpImplLB->Show();

    if ( mpFloatWin )
    {
        mpFloatWin->SetImplListBox( mpImplLB );
        mpImplLB->SetSelectionChangedHdl( LINK( this, ListBox, ImplSelectionChangedHdl ) );
    }
    else
        mpImplLB->GetMainWindow()->AllowGrabFocus( TRUE );

    SetCompoundControl( TRUE );
}

// -----------------------------------------------------------------------

WinBits ListBox::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void ListBox::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    USHORT nSelPos = ReadShortRes();
    USHORT nNumber = ReadShortRes();

    for( USHORT i = 0; i < nNumber; i++ )
    {
        USHORT nPos = InsertEntry( ReadStringRes(), LISTBOX_APPEND );

        long nId = ReadLongRes();
        if( nId )
            SetEntryData( nPos, (void *)nId );  // ID als UserData
    }

    if( nSelPos < nNumber )
        SelectEntryPos( nSelPos );
}

// -----------------------------------------------------------------------

IMPL_LINK( ListBox, ImplSelectHdl, void*, EMPTYARG )
{
    BOOL bPopup = IsInDropDown();
    if( IsDropDownBox() )
    {
        if( !mpImplLB->IsTravelSelect() )
        {
            mpFloatWin->EndPopupMode();
            mpImplWin->GrabFocus();
        }

        mpImplWin->SetItemPos( GetSelectEntryPos() );
        mpImplWin->SetString( GetSelectEntry() );
        if( mpImplLB->GetEntryList()->HasImages() )
        {
            Image aImage = mpImplLB->GetEntryList()->GetEntryImage( GetSelectEntryPos() );
            mpImplWin->SetImage( aImage );
        }
        mpImplWin->Invalidate();
    }

    if ( mpImplLB->IsSelectionChanged() || ( bPopup && !IsMultiSelectionEnabled() ) )
        Select();

    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ListBox, ImplCancelHdl, void*, EMPTYARG )
{
    if( IsInDropDown() )
        mpFloatWin->EndPopupMode();

    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ListBox, ImplSelectionChangedHdl, void*, n )
{
    if ( !mpImplLB->IsTrackingSelect() )
    {
        USHORT nChanged = (USHORT)(ULONG)n;
        const ImplEntryList* pEntryList = mpImplLB->GetEntryList();
        if ( pEntryList->IsEntryPosSelected( nChanged ) )
        {
            // Sollte mal ein ImplPaintEntry werden...
            if ( nChanged < pEntryList->GetMRUCount() )
                nChanged = pEntryList->FindEntry( pEntryList->GetEntryText( nChanged ), MATCH_CASE, STRING_LEN, pEntryList->GetMRUCount() );
            mpImplWin->SetItemPos( nChanged );
            mpImplWin->SetString( mpImplLB->GetEntryList()->GetEntryText( nChanged ) );
            if( mpImplLB->GetEntryList()->HasImages() )
            {
                Image aImage = mpImplLB->GetEntryList()->GetEntryImage( nChanged );
                mpImplWin->SetImage( aImage );
            }
            mpImplWin->Invalidate();
        }
    }
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ListBox, ImplDoubleClickHdl, void*, p )
{
    DoubleClick();
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ListBox, ImplClickBtnHdl, void*, EMPTYARG )
{
    if( !mpFloatWin->IsInPopupMode() )
    {
        mpImplWin->GrabFocus();
        mpBtn->SetPressed( TRUE );
        mpFloatWin->StartFloat( TRUE );
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ListBox, ImplPopupModeEndHdl, void*, p )
{
    mpBtn->SetPressed( FALSE );
    return 0;
}

// -----------------------------------------------------------------------

void ListBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags )
{
    mpImplLB->GetMainWindow()->ImplInitSettings( TRUE, TRUE, TRUE );

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    Font aFont = mpImplLB->GetMainWindow()->GetDrawPixelFont( pDev );
    OutDevType eOutDevType = pDev->GetOutDevType();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    BOOL bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
    BOOL bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
    if ( bBorder || bBackground )
    {
        Rectangle aRect( aPos, aSize );
        if ( bBorder )
        {
            DecorationView aDecoView( pDev );
            aRect = aDecoView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );
        }
        if ( bBackground )
        {
            pDev->SetFillColor( GetControlBackground() );
            pDev->DrawRect( aRect );
        }
    }

    // Inhalt
    if ( ( nFlags & WINDOW_DRAW_MONO ) || ( eOutDevType == OUTDEV_PRINTER ) )
    {
        pDev->SetTextColor( Color( COL_BLACK ) );
    }
    else
    {
        if ( !(nFlags & WINDOW_DRAW_NODISABLE ) && !IsEnabled() )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            pDev->SetTextColor( rStyleSettings.GetDisableColor() );
        }
        else
        {
            pDev->SetTextColor( GetTextColor() );
        }
    }

    long nOnePixel = GetDrawPixel( pDev, 1 );
    if ( IsDropDownBox() )
    {
        XubString   aText = GetSelectEntry();
        long        nTextHeight = pDev->GetTextHeight();
        long        nTextWidth = pDev->GetTextWidth( aText );
        long        nOffX = 3*nOnePixel;
        long        nOffY = (aSize.Height()-nTextHeight) / 2;

        // Clipping?
        if ( (nOffY < 0) ||
             ((nOffY+nTextHeight) > aSize.Height()) ||
             ((nOffX+nTextWidth) > aSize.Width()) )
        {
            Rectangle aClip( aPos, aSize );
            if ( nTextHeight > aSize.Height() )
                aClip.Bottom() += nTextHeight-aSize.Height()+1;  // Damit HP-Drucker nicht 'weg-optimieren'
            pDev->IntersectClipRegion( aClip );
        }

        pDev->DrawText( Point( aPos.X()+nOffX, aPos.Y()+nOffY ), aText );
    }
    else
    {
        long        nTextHeight = pDev->GetTextHeight();
        USHORT      nLines = (USHORT)(aSize.Height() / nTextHeight);
        Rectangle   aClip( aPos, aSize );
        pDev->IntersectClipRegion( aClip );
        if ( !nLines )
            nLines = 1;
        for ( USHORT n = 0; n < nLines; n++ )
        {
            USHORT nEntry = n+mpImplLB->GetTopEntry();
            BOOL bSelected = mpImplLB->GetEntryList()->IsEntryPosSelected( nEntry );
            if ( bSelected )
            {
                pDev->SetFillColor( COL_BLACK );
                pDev->DrawRect( Rectangle(  Point( aPos.X(), aPos.Y() + n*nTextHeight ),
                                            Point( aPos.X() + aSize.Width(), aPos.Y() + (n+1)*nTextHeight + 2*nOnePixel ) ) );
                pDev->SetFillColor();
                pDev->SetTextColor( COL_WHITE );
            }
            pDev->DrawText( Point( aPos.X() + 3*nOnePixel, aPos.Y() + n*nTextHeight + nOnePixel ), mpImplLB->GetEntryList()->GetEntryText( nEntry ) );
            if ( bSelected )
                pDev->SetTextColor( COL_BLACK );
        }
    }

    pDev->Pop();
}

// -----------------------------------------------------------------------

void ListBox::GetFocus()
{
    if ( mpImplLB )
    {
        if( IsDropDownBox() )
            mpImplWin->GrabFocus();
        else
            mpImplLB->GrabFocus();
    }

    Control::GetFocus();
}

// -----------------------------------------------------------------------

void ListBox::LoseFocus()
{
    if( IsDropDownBox() )
        mpImplWin->HideFocus();
    else
        mpImplLB->HideFocus();

    Control::LoseFocus();
}

// -----------------------------------------------------------------------

void ListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        Resize();
        mpImplLB->Resize(); // Wird nicht durch ListBox::Resize() gerufen, wenn sich die ImplLB nicht aendert.

        if ( mpImplWin )
        {
            mpImplWin->SetSettings( GetSettings() );    // Falls noch nicht eingestellt...
            ImplInitFieldSettings( mpImplWin, TRUE, TRUE, TRUE );

            mpBtn->SetSettings( GetSettings() );
            ImplInitDropDownButton( mpBtn );
        }


        if ( IsDropDownBox() )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

void ListBox::EnableAutoSize( BOOL bAuto )
{
    mbDDAutoSize = bAuto;
    if ( mpFloatWin )
    {
        if ( bAuto && !mpFloatWin->GetDropDownLineCount() )
            mpFloatWin->SetDropDownLineCount( 5 );
        else if ( !bAuto )
            mpFloatWin->SetDropDownLineCount( 0 );
    }
}

// -----------------------------------------------------------------------

void ListBox::SetDropDownLineCount( USHORT nLines )
{
    if ( mpFloatWin )
        mpFloatWin->SetDropDownLineCount( nLines );
}

// -----------------------------------------------------------------------

USHORT ListBox::GetDropDownLineCount() const
{
    USHORT nLines = 0;
    if ( mpFloatWin )
        nLines = mpFloatWin->GetDropDownLineCount();
    return nLines;
}

// -----------------------------------------------------------------------

void ListBox::SetPosSizePixel( long nX, long nY, long nWidth, long nHeight, USHORT nFlags )
{
    if( IsDropDownBox() && ( nFlags & WINDOW_POSSIZE_SIZE ) )
    {
        Size aPrefSz = mpFloatWin->GetPrefSize();
        if ( ( nFlags & WINDOW_POSSIZE_HEIGHT ) && ( nHeight > mnDDHeight ) )
            aPrefSz.Height() = nHeight-mnDDHeight;
        if ( nFlags & WINDOW_POSSIZE_WIDTH )
            aPrefSz.Width() = nWidth;
        mpFloatWin->SetPrefSize( aPrefSz );

        if ( IsAutoSizeEnabled() )
            nHeight = mnDDHeight;
    }

    Control::SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

// -----------------------------------------------------------------------

void ListBox::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    if( IsDropDownBox() )
    {
        long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
        nSBWidth = CalcZoom( nSBWidth );
        mpImplWin->SetPosSizePixel( 0, 0, aOutSz.Width() - nSBWidth, aOutSz.Height() );
        mpBtn->SetPosSizePixel( aOutSz.Width() - nSBWidth, 0, nSBWidth, aOutSz.Height() );
    }
    else
    {
        mpImplLB->SetSizePixel( aOutSz );
    }

    // FloatingWindow-Groesse auch im unsichtbare Zustand auf Stand halten,
    // weil KEY_PGUP/DOWN ausgewertet wird...
    if ( mpFloatWin )
        mpFloatWin->SetSizePixel( mpFloatWin->CalcFloatSize() );
}

// -----------------------------------------------------------------------

void ListBox::StateChanged( StateChangedType nType )
{
    if( nType == STATE_CHANGE_READONLY )
    {
        if( mpImplWin )
            mpImplWin->Enable( !IsReadOnly() );
        if( mpBtn )
            mpBtn->Enable( !IsReadOnly() );
    }
    else if( nType == STATE_CHANGE_ENABLE )
    {
        mpImplLB->Enable( IsEnabled() );
        if( mpImplWin )
        {
            mpImplWin->Enable( IsEnabled() );
            mpImplWin->Invalidate();
        }
        if( mpBtn )
            mpBtn->Enable( IsEnabled() );
    }
    else if( nType == STATE_CHANGE_UPDATEMODE )
    {
        mpImplLB->SetUpdateMode( IsUpdateMode() );
    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        mpImplLB->SetZoom( GetZoom() );
        if ( mpImplWin )
        {
            mpImplWin->SetZoom( GetZoom() );
            mpImplWin->SetFont( mpImplLB->GetMainWindow()->GetFont() );
            mpImplWin->Invalidate();
        }
        Resize();
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        mpImplLB->SetControlFont( GetControlFont() );
        if ( mpImplWin )
        {
            mpImplWin->SetControlFont( GetControlFont() );
            mpImplWin->SetFont( mpImplLB->GetMainWindow()->GetFont() );
            mpImplWin->Invalidate();
        }
        Resize();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        mpImplLB->SetControlForeground( GetControlForeground() );
        if ( mpImplWin )
        {
            mpImplWin->SetControlForeground( GetControlForeground() );
            mpImplWin->SetTextColor( GetControlForeground() );
            mpImplWin->SetFont( mpImplLB->GetMainWindow()->GetFont() );
            mpImplWin->Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        mpImplLB->SetControlBackground( GetControlBackground() );
        if ( mpImplWin )
        {
            mpImplWin->SetBackground( mpImplLB->GetMainWindow()->GetControlBackground() );
            mpImplWin->SetControlBackground( mpImplLB->GetMainWindow()->GetControlBackground() );
            mpImplWin->SetFont( mpImplLB->GetMainWindow()->GetFont() );
            mpImplWin->Invalidate();
        }
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        mpImplLB->GetMainWindow()->EnableSort( ( GetStyle() & WB_SORT ) ? TRUE : FALSE );
    }

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

long ListBox::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if ( mpImplLB )
    {
        if( ( rNEvt.GetType() == EVENT_KEYINPUT ) && ( rNEvt.GetWindow() == mpImplWin ) )
        {
            KeyEvent aKeyEvt = *rNEvt.GetKeyEvent();
            switch( aKeyEvt.GetKeyCode().GetCode() )
            {
                case KEY_DOWN:
                {
                    if( mpFloatWin && !mpFloatWin->IsInPopupMode() &&
                        aKeyEvt.GetKeyCode().IsMod2() )
                    {
                        mpBtn->SetPressed( TRUE );
                        mpFloatWin->StartFloat( FALSE );
                        nDone = 1;
                    }
                    else
                    {
                        nDone = mpImplLB->ProcessKeyInput( aKeyEvt );
                    }
                }
                break;
                case KEY_UP:
                {
                    if( mpFloatWin && mpFloatWin->IsInPopupMode() &&
                        aKeyEvt.GetKeyCode().IsMod2() )
                    {
                        mpFloatWin->EndPopupMode();
                        nDone = 1;
                    }
                    else
                    {
                        nDone = mpImplLB->ProcessKeyInput( aKeyEvt );
                    }
                }
                break;
                case KEY_RETURN:
                {
                    if( IsInDropDown() )
                    {
                        mpImplLB->ProcessKeyInput( aKeyEvt );
                        nDone = 1;
                    }
                }
                break;

                default:
                    nDone = mpImplLB->ProcessKeyInput( aKeyEvt );
            }
        }
        else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
        {
            if ( IsInDropDown() && !HasChildPathFocus( TRUE ) )
                mpFloatWin->EndPopupMode();
        }
        else if ( (rNEvt.GetType() == EVENT_COMMAND) &&
                  (rNEvt.GetCommandEvent()->GetCommand() == COMMAND_WHEEL) &&
                  (rNEvt.GetWindow() == mpImplWin) )
        {
            nDone = mpImplLB->HandleWheelAsCursorTravel( *rNEvt.GetCommandEvent() );
        }
    }

    return nDone ? nDone : Control::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void ListBox::Select()
{
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void ListBox::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ListBox::Clear()
{
    mpImplLB->Clear();
    if( IsDropDownBox() )
    {
        mpImplWin->SetItemPos( LISTBOX_ENTRY_NOTFOUND );
        mpImplWin->SetString( ImplGetSVEmptyStr() );
        Image aImage;
        mpImplWin->SetImage( aImage );
        mpImplWin->Invalidate();
    }
}

// -----------------------------------------------------------------------

void ListBox::SetNoSelection()
{
    mpImplLB->SetNoSelection();
    if( IsDropDownBox() )
    {
        mpImplWin->SetItemPos( LISTBOX_ENTRY_NOTFOUND );
        mpImplWin->SetString( ImplGetSVEmptyStr() );
        Image aImage;
        mpImplWin->SetImage( aImage );
        mpImplWin->Invalidate();
    }
}

// -----------------------------------------------------------------------

USHORT ListBox::InsertEntry( const XubString& rStr, USHORT nPos )
{
    USHORT nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rStr );
    nRealPos -= mpImplLB->GetEntryList()->GetMRUCount();
    return nRealPos;
}

// -----------------------------------------------------------------------

USHORT ListBox::InsertEntry( const Image& rImage, USHORT nPos )
{
    USHORT nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rImage );
    nRealPos -= mpImplLB->GetEntryList()->GetMRUCount();
    return nRealPos;
}

// -----------------------------------------------------------------------

USHORT ListBox::InsertEntry( const XubString& rStr, const Image& rImage, USHORT nPos )
{
    USHORT nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rStr, rImage );
    nRealPos -= mpImplLB->GetEntryList()->GetMRUCount();
    return nRealPos;
}

// -----------------------------------------------------------------------

void ListBox::RemoveEntry( const XubString& rStr )
{
    RemoveEntry( GetEntryPos( rStr ) );
}

// -----------------------------------------------------------------------

void ListBox::RemoveEntry( USHORT nPos )
{
    mpImplLB->RemoveEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

USHORT ListBox::GetEntryPos( const XubString& rStr ) const
{
    USHORT nPos = mpImplLB->GetEntryList()->FindEntry( rStr, MATCH_CASE, STRING_LEN, mpImplLB->GetEntryList()->GetMRUCount() );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos -= mpImplLB->GetEntryList()->GetMRUCount();
    return nPos;
}

// -----------------------------------------------------------------------

USHORT ListBox::GetEntryPos( const void* pData ) const
{
    USHORT nPos = mpImplLB->GetEntryList()->FindEntry( pData );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos -= mpImplLB->GetEntryList()->GetMRUCount();
    return nPos;
}

// -----------------------------------------------------------------------

XubString ListBox::GetEntry( USHORT nPos ) const
{
    return mpImplLB->GetEntryList()->GetEntryText( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

USHORT ListBox::GetEntryCount() const
{
    return mpImplLB->GetEntryList()->GetEntryCount() - mpImplLB->GetEntryList()->GetMRUCount();
}

// -----------------------------------------------------------------------

XubString ListBox::GetSelectEntry( USHORT nIndex ) const
{
    return GetEntry( GetSelectEntryPos( nIndex ) );
}

// -----------------------------------------------------------------------

USHORT ListBox::GetSelectEntryCount() const
{
    return mpImplLB->GetEntryList()->GetSelectEntryCount();
}

// -----------------------------------------------------------------------

USHORT ListBox::GetSelectEntryPos( USHORT nIndex ) const
{
    USHORT nPos = mpImplLB->GetEntryList()->GetSelectEntryPos( nIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
            nPos = mpImplLB->GetEntryList()->FindEntry( mpImplLB->GetEntryList()->GetEntryText( nPos ), MATCH_CASE, STRING_LEN, mpImplLB->GetEntryList()->GetMRUCount() );
        nPos -= mpImplLB->GetEntryList()->GetMRUCount();
    }
    return nPos;
}

// -----------------------------------------------------------------------

BOOL ListBox::IsEntrySelected( const XubString& rStr ) const
{
    return IsEntryPosSelected( GetEntryPos( rStr ) );
}

// -----------------------------------------------------------------------

BOOL ListBox::IsEntryPosSelected( USHORT nPos ) const
{
    return mpImplLB->GetEntryList()->IsEntryPosSelected( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

void ListBox::SelectEntry( const XubString& rStr, BOOL bSelect )
{
    SelectEntryPos( GetEntryPos( rStr ), bSelect );
}

// -----------------------------------------------------------------------

void ListBox::SelectEntryPos( USHORT nPos, BOOL bSelect )
{
    if ( nPos < mpImplLB->GetEntryList()->GetEntryCount() )
        mpImplLB->SelectEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), bSelect );
}

// -----------------------------------------------------------------------

void ListBox::SetEntryData( USHORT nPos, void* pNewData )
{
    mpImplLB->SetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount(), pNewData );
}

// -----------------------------------------------------------------------

void* ListBox::GetEntryData( USHORT nPos ) const
{
    return mpImplLB->GetEntryList()->GetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

void ListBox::SetTopEntry( USHORT nPos )
{
    mpImplLB->SetTopEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

USHORT ListBox::GetTopEntry() const
{
    USHORT nPos = GetEntryCount() ? mpImplLB->GetTopEntry() : LISTBOX_ENTRY_NOTFOUND;
    if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
        nPos = 0;
    return nPos;
}

// -----------------------------------------------------------------------

BOOL ListBox::IsTravelSelect() const
{
    return mpImplLB->IsTravelSelect();
}

// -----------------------------------------------------------------------

BOOL ListBox::IsInDropDown() const
{
    return mpFloatWin && mpFloatWin->IsInPopupMode();
}

// -----------------------------------------------------------------------

long ListBox::CalcWindowSizePixel( USHORT nLines ) const
{
    return mpImplLB->GetEntryHeight() * nLines;
}

// -----------------------------------------------------------------------

void ListBox::EnableMultiSelection( BOOL bMulti )
{
    mpImplLB->EnableMultiSelection( bMulti );

    // WB_SIMPLEMODE:
    // Die MultiListBox verhält sich wie eine normale ListBox.
    // Die Mehrfachselektion kann nur über entsprechende Zusatztasten erfolgen.

    BOOL bSimpleMode = ( GetStyle() & WB_SIMPLEMODE ) ? TRUE : FALSE;
    mpImplLB->SetMultiSelectionSimpleMode( bSimpleMode );

    // ohne Focus ist das Traveln in einer MultiSelection nicht zu sehen:
    if ( mpFloatWin )
        mpImplLB->GetMainWindow()->AllowGrabFocus( bMulti );
}

// -----------------------------------------------------------------------

BOOL ListBox::IsMultiSelectionEnabled() const
{
    return mpImplLB->IsMultiSelectionEnabled();
}

// -----------------------------------------------------------------------

Size ListBox::CalcMinimumSize() const
{
    Size aSz;
    if ( !IsDropDownBox() )
    {
        aSz = mpImplLB->CalcSize( mpImplLB->GetEntryList()->GetEntryCount() );
    }
    else
    {
        aSz.Height() = mpImplLB->CalcSize( 1 ).Height();
        aSz.Width() = mpImplLB->GetMaxEntryWidth();
        aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
    }

    aSz = CalcWindowSize( aSz );
    return aSz;
}

// -----------------------------------------------------------------------

Size ListBox::CalcAdjustedSize( const Size& rPrefSize ) const
{
    Size aSz = rPrefSize;
    long nLeft, nTop, nRight, nBottom;
    ((Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );
    aSz.Height() -= nTop+nBottom;
    if ( !IsDropDownBox() )
    {
        long nEntryHeight = CalcSize( 1, 1 ).Height();
        long nLines = aSz.Height() / nEntryHeight;
        if ( nLines < 1 )
            nLines = 1;
        aSz.Height() = nLines * nEntryHeight;
    }
    else
    {
        aSz.Height() = mnDDHeight;
    }
    aSz.Height() += nTop+nBottom;

    aSz = CalcWindowSize( aSz );
    return aSz;
}

// -----------------------------------------------------------------------

Size ListBox::CalcSize( USHORT nColumns, USHORT nLines ) const
{
    // ggf. werden ScrollBars eingeblendet
    Size aMinSz = CalcMinimumSize();
//  aMinSz = ImplCalcOutSz( aMinSz );

    Size aSz;

    // Hoehe
    if ( nLines )
    {
        if ( !IsDropDownBox() )
            aSz.Height() = mpImplLB->CalcSize( nLines ).Height();
        else
            aSz.Height() = mnDDHeight;
    }
    else
        aSz.Height() = aMinSz.Height();

    // Breite
    if ( nColumns )
        aSz.Width() = nColumns * GetTextWidth( XubString( 'X' ) );
    else
        aSz.Width() = aMinSz.Width();

    if ( IsDropDownBox() )
        aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();

    if ( !IsDropDownBox() )
    {
        if ( aSz.Width() < aMinSz.Width() )
            aSz.Height() += GetSettings().GetStyleSettings().GetScrollBarSize();
        if ( aSz.Height() < aMinSz.Height() )
            aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
    }

    aSz = CalcWindowSize( aSz );
    return aSz;
}

// -----------------------------------------------------------------------

void ListBox::GetMaxVisColumnsAndLines( USHORT& rnCols, USHORT& rnLines ) const
{
    long nCharWidth = GetTextWidth( UniString( 'x' ) );
    if ( !IsDropDownBox() )
    {
        Size aOutSz = mpImplLB->GetMainWindow()->GetOutputSizePixel();
        rnCols = (USHORT) (aOutSz.Width()/nCharWidth);
        rnLines = (USHORT) (aOutSz.Height()/mpImplLB->GetEntryHeight());
    }
    else
    {
        Size aOutSz = mpImplWin->GetOutputSizePixel();
        rnCols = (USHORT) (aOutSz.Width()/nCharWidth);
        rnLines = 1;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( ListBox, ImplUserDrawHdl, UserDrawEvent*, pEvent )
{
    UserDraw( *pEvent );
    return 1;
}

// -----------------------------------------------------------------------

void ListBox::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

#if SUPD < 593
void ListBox::DrawEntry( const UserDrawEvent& rEvt, BOOL bDrawImage, BOOL bDrawText )
{
    if ( rEvt.GetDevice() == mpImplLB->GetMainWindow() )
        mpImplLB->GetMainWindow()->DrawEntry( rEvt.GetItemId(), bDrawImage, bDrawText );
    else if ( rEvt.GetDevice() == mpImplWin )
        mpImplWin->DrawEntry( bDrawImage, bDrawText );
}
#endif

void ListBox::DrawEntry( const UserDrawEvent& rEvt, BOOL bDrawImage, BOOL bDrawText, BOOL bDrawTextAtImagePos )
{
    if ( rEvt.GetDevice() == mpImplLB->GetMainWindow() )
        mpImplLB->GetMainWindow()->DrawEntry( rEvt.GetItemId(), bDrawImage, bDrawText, bDrawTextAtImagePos );
    else if ( rEvt.GetDevice() == mpImplWin )
        mpImplWin->DrawEntry( bDrawImage, bDrawText, bDrawTextAtImagePos );
}

// -----------------------------------------------------------------------

void ListBox::SetUserItemSize( const Size& rSz )
{
    mpImplLB->GetMainWindow()->SetUserItemSize( rSz );
    if ( mpImplWin )
        mpImplWin->SetUserItemSize( rSz );
}

// -----------------------------------------------------------------------

const Size& ListBox::GetUserItemSize() const
{
    return mpImplLB->GetMainWindow()->GetUserItemSize();
}

// -----------------------------------------------------------------------

void ListBox::EnableUserDraw( BOOL bUserDraw )
{
    mpImplLB->GetMainWindow()->EnableUserDraw( bUserDraw );
    if ( mpImplWin )
        mpImplWin->EnableUserDraw( bUserDraw );
}

// -----------------------------------------------------------------------

BOOL ListBox::IsUserDrawEnabled() const
{
    return mpImplLB->GetMainWindow()->IsUserDrawEnabled();
}

// -----------------------------------------------------------------------

void ListBox::SetReadOnly( BOOL bReadOnly )
{
    if ( mpImplLB->IsReadOnly() != bReadOnly )
    {
        mpImplLB->SetReadOnly( bReadOnly );
        StateChanged( STATE_CHANGE_READONLY );
    }
}

// -----------------------------------------------------------------------

BOOL ListBox::IsReadOnly() const
{
    return mpImplLB->IsReadOnly();
}

// -----------------------------------------------------------------------

void ListBox::SetSeparatorPos( USHORT n )
{
    mpImplLB->SetSeparatorPos( n );
}

// -----------------------------------------------------------------------

void ListBox::SetSeparatorPos()
{
    mpImplLB->SetSeparatorPos( LISTBOX_ENTRY_NOTFOUND );
}

// -----------------------------------------------------------------------

USHORT ListBox::GetSeparatorPos() const
{
    return mpImplLB->GetSeparatorPos();
}

// -----------------------------------------------------------------------

void ListBox::SetMRUEntries( const XubString& rEntries, xub_Unicode cSep )
{
    mpImplLB->SetMRUEntries( rEntries, cSep );
}

// -----------------------------------------------------------------------

XubString ListBox::GetMRUEntries( xub_Unicode cSep ) const
{
    return mpImplLB->GetMRUEntries( cSep );
}

// -----------------------------------------------------------------------

void ListBox::SetMaxMRUCount( USHORT n )
{
    mpImplLB->SetMaxMRUCount( n );
}

// -----------------------------------------------------------------------

USHORT ListBox::GetMaxMRUCount() const
{
    return mpImplLB->GetMaxMRUCount();
}

// =======================================================================

MultiListBox::MultiListBox( Window* pParent, WinBits nStyle ) :
    ListBox( WINDOW_MULTILISTBOX )
{
    ImplInit( pParent, nStyle );
    EnableMultiSelection( TRUE );
}

// -----------------------------------------------------------------------

MultiListBox::MultiListBox( Window* pParent, const ResId& rResId ) :
    ListBox( WINDOW_MULTILISTBOX )
{
    rResId.SetRT( RSC_MULTILISTBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();
    EnableMultiSelection( TRUE );
}
