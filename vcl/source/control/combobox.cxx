/*************************************************************************
 *
 *  $RCSfile: combobox.cxx,v $
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

#define _SV_COMBOBOX_CXX

#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_ILSTBOX_HXX
#include <ilstbox.hxx>
#endif
#ifndef _SV_LSTBOX_H
#include <lstbox.h>
#endif
#ifndef _SV_BUTTON_HXX
#include <button.hxx>
#endif
#ifndef _SV_SUBEDIT_HXX
#include <subedit.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <combobox.hxx>
#endif

#pragma hdrstop

// =======================================================================

inline ULONG ImplCreateKey( USHORT nPos )
{
    // Key = Pos+1, wegen Pos 0
    return nPos+1;
}

// -----------------------------------------------------------------------

static void lcl_GetSelectedEntries( Table& rSelectedPos, const XubString& rText, xub_Unicode cTokenSep, const ImplEntryList* pEntryList )
{
    for( xub_StrLen n = rText.GetTokenCount( cTokenSep ); n; )
    {
        XubString aToken = rText.GetToken( --n, cTokenSep );
        aToken.EraseLeadingAndTrailingChars( ' ' );
        USHORT nPos = pEntryList->FindEntry( aToken );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            rSelectedPos.Insert( ImplCreateKey( nPos ), (void*)1L );
    }
}

// =======================================================================

ComboBox::ComboBox( WindowType nType ) :
    Edit( nType )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

ComboBox::ComboBox( Window* pParent, WinBits nStyle ) :
    Edit( WINDOW_COMBOBOX )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ComboBox::ComboBox( Window* pParent, const ResId& rResId ) :
    Edit( WINDOW_COMBOBOX )
{
    ImplInitData();
    rResId.SetRT( RSC_COMBOBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

ComboBox::~ComboBox()
{
    SetSubEdit( NULL );
    delete mpSubEdit;

    delete mpImplLB;
    mpImplLB = NULL;

    delete mpFloatWin;
    delete mpBtn;
}

// -----------------------------------------------------------------------

void ComboBox::ImplInitData()
{
    mpSubEdit           = NULL;
    mpBtn               = NULL;
    mpImplLB            = NULL;
    mpFloatWin          = NULL;

    mnDDHeight          = 0;
    mbDDAutoSize        = TRUE;
    mbSyntheticModify   = FALSE;
    mbMatchCase         = FALSE;
    mcMultiSep          = ';';
}

// -----------------------------------------------------------------------

void ComboBox::ImplCalcEditHeight()
{
    long nLeft, nTop, nRight, nBottom;
    GetBorder( nLeft, nTop, nRight, nBottom );
    mnDDHeight = (USHORT)(mpSubEdit->GetTextHeight() + nTop + nBottom + 4);
    if ( !IsDropDownBox() )
        mnDDHeight += 4;
}

// -----------------------------------------------------------------------

void ComboBox::ImplInit( Window* pParent, WinBits nStyle )
{
    ImplInitStyle( nStyle );

    BOOL bNoBorder = ( nStyle & WB_NOBORDER ) ? TRUE : FALSE;
    if ( !(nStyle & WB_DROPDOWN) )
    {
        nStyle &= ~WB_BORDER;
        nStyle |= WB_NOBORDER;
    }
    else
    {
        if ( !bNoBorder )
            nStyle |= WB_BORDER;
    }

    Edit::ImplInit( pParent, nStyle );
    SetBackground();

    // DropDown ?
    WinBits nEditStyle = 0;
    WinBits nListStyle = nStyle;
    if( nStyle & WB_DROPDOWN )
    {
        mpFloatWin = new ImplListBoxFloatingWindow( this );
        mpFloatWin->SetAutoWidth( TRUE );
        mpFloatWin->SetPopupModeEndHdl( LINK( this, ComboBox, ImplPopupModeEndHdl ) );

        mpBtn = new ImplBtn( this, WB_NOLIGHTBORDER | WB_RECTSTYLE );
        ImplInitDropDownButton( mpBtn );
        mpBtn->SetMBDownHdl( LINK( this, ComboBox, ImplClickBtnHdl ) );
        mpBtn->Show();

        nEditStyle |= WB_NOBORDER;
        nListStyle &= ~WB_BORDER;
        nListStyle |= WB_NOBORDER;
    }
    else
    {
        if ( !bNoBorder )
        {
            nEditStyle |= WB_BORDER;
            nListStyle &= ~WB_NOBORDER;
            nListStyle |= WB_BORDER;
        }
    }

    mpSubEdit = new Edit( this, nEditStyle );
    SetSubEdit( mpSubEdit );
    mpSubEdit->SetPosPixel( Point() );
    EnableAutocomplete( TRUE );
    mpSubEdit->Show();

    Window* pLBParent = this;
    if ( mpFloatWin )
        pLBParent = mpFloatWin;
    mpImplLB = new ImplListBox( pLBParent, nListStyle|WB_SIMPLEMODE );
    mpImplLB->SetPosPixel( Point() );
    mpImplLB->SetSelectHdl( LINK( this, ComboBox, ImplSelectHdl ) );
    mpImplLB->SetCancelHdl( LINK( this, ComboBox, ImplCancelHdl ) );
    mpImplLB->SetDoubleClickHdl( LINK( this, ComboBox, ImplDoubleClickHdl ) );
    mpImplLB->SetUserDrawHdl( LINK( this, ComboBox, ImplUserDrawHdl ) );
    mpImplLB->SetSelectionChangedHdl( LINK( this, ComboBox, ImplSelectionChangedHdl ) );
    mpImplLB->Show();

    if ( mpFloatWin )
        mpFloatWin->SetImplListBox( mpImplLB );
//  else
//      mpImplLB->GetMainWindow()->AllowGrabFocus( TRUE );

    ImplCalcEditHeight();

    SetCompoundControl( TRUE );
}

// -----------------------------------------------------------------------

WinBits ComboBox::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void ComboBox::ImplLoadRes( const ResId& rResId )
{
    Edit::ImplLoadRes( rResId );

    USHORT nNumber = ReadShortRes();

    if( nNumber )
    {
        for( USHORT i = 0; i < nNumber; i++ )
        {
            InsertEntry( ReadStringRes(), LISTBOX_APPEND );
        }
    }
}

// -----------------------------------------------------------------------

void ComboBox::EnableAutocomplete( BOOL bEnable, BOOL bMatchCase )
{
    mbMatchCase = bMatchCase;

    if ( bEnable )
        mpSubEdit->SetAutocompleteHdl( LINK( this, ComboBox, ImplAutocompleteHdl ) );
    else
        mpSubEdit->SetAutocompleteHdl( Link() );
}

// -----------------------------------------------------------------------

BOOL ComboBox::IsAutocompleteEnabled() const
{
    return mpSubEdit->GetAutocompleteHdl().IsSet();
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplClickBtnHdl, void*, EMPTYARG )
{
    mpSubEdit->GrabFocus();
    if ( !mpImplLB->GetEntryList()->GetMRUCount() )
        ImplUpdateFloatSelection();
    else
        mpImplLB->SelectEntry( 0 , TRUE );
    mpBtn->SetPressed( TRUE );
    mpFloatWin->StartFloat( TRUE );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplPopupModeEndHdl, void*, p )
{
    mpBtn->SetPressed( FALSE );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplAutocompleteHdl, Edit*, pEdit )
{
    Selection           aSel = pEdit->GetSelection();
    AutocompleteAction  eAction = pEdit->GetAutocompleteAction();

    // Wenn keine Selection vorhanden ist, wird bei Tab/Shift+Tab auch
    // keine AutoCompletion durchgefuehrt, da man ansonsten nicht in
    // das naechste Feld kommt und der Text wieder mit etwas AutoExpandiert
    // wird, was man nicht haben moechte.
    if ( aSel.Len() ||
         ((eAction != AUTOCOMPLETE_TABFORWARD) && (eAction != AUTOCOMPLETE_TABBACKWARD)) )
    {
        XubString   aFullText = pEdit->GetText();
        XubString   aStartText = aFullText.Copy( 0, (xub_StrLen)aSel.Max() );
        USHORT      nStart = mpImplLB->GetCurrentPos();

        if ( nStart == LISTBOX_ENTRY_NOTFOUND )
            nStart = 0;

        BOOL bForward = TRUE;
        if ( eAction == AUTOCOMPLETE_TABFORWARD )
            nStart++;
        else if ( eAction == AUTOCOMPLETE_TABBACKWARD )
        {
            bForward = FALSE;
            nStart = nStart ? (nStart--) : mpImplLB->GetEntryList()->GetEntryCount()-1;
        }
        MatchMode eMatchMode = mbMatchCase ? MATCH_CASE : MATCH_BEST;
        USHORT nPos = mpImplLB->GetEntryList()->FindEntry( aStartText, eMatchMode, aStartText.Len(), nStart, bForward );
        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            nPos = mpImplLB->GetEntryList()->FindEntry( aStartText, eMatchMode, aStartText.Len(), bForward ? 0 : (mpImplLB->GetEntryList()->GetEntryCount()-1), bForward );

        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            XubString aText = mpImplLB->GetEntryList()->GetEntryText( nPos );
            Selection aSel( aText.Len(), aStartText.Len() );
            pEdit->SetText( aText, aSel );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplSelectHdl, void*, EMPTYARG )
{
    BOOL bPopup = IsInDropDown();
    if ( bPopup && !mpImplLB->IsTravelSelect() &&
            ( !IsMultiSelectionEnabled() || !mpImplLB->GetSelectModifier() ) )
    {
        mpFloatWin->EndPopupMode();
        GrabFocus();
    }

    if ( mpImplLB->IsSelectionChanged() || bPopup )
    {
        XubString aText;
        if ( IsMultiSelectionEnabled() )
        {
            aText = mpSubEdit->GetText();

            // Alle Eintraege entfernen, zu denen es einen Entry gibt, der aber nicht selektiert ist.
            xub_StrLen nIndex = 0;
            while ( nIndex != STRING_NOTFOUND )
            {
                xub_StrLen  nPrevIndex = nIndex;
                XubString   aToken = aText.GetToken( 0, mcMultiSep, nIndex );
                xub_StrLen  nTokenLen = aToken.Len();
                aToken.EraseLeadingAndTrailingChars( ' ' );
                USHORT      nP = mpImplLB->GetEntryList()->FindEntry( aToken );
                if ( (nP != LISTBOX_ENTRY_NOTFOUND) && (!mpImplLB->GetEntryList()->IsEntryPosSelected( nP )) )
                {
                    aText.Erase( nPrevIndex, nTokenLen );
                    nIndex -= nTokenLen;
                    if ( (nPrevIndex < aText.Len()) && (aText.GetChar( nPrevIndex ) == mcMultiSep) )
                    {
                        aText.Erase( nPrevIndex, 1 );
                        nIndex--;
                    }
                }
                aText.EraseLeadingAndTrailingChars( ' ' );
            }

            // Fehlende Eintraege anhaengen...
            Table aSelInText;
            lcl_GetSelectedEntries( aSelInText, aText, mcMultiSep, mpImplLB->GetEntryList() );
            USHORT nSelectedEntries = mpImplLB->GetEntryList()->GetSelectEntryCount();
            for ( USHORT n = 0; n < nSelectedEntries; n++ )
            {
                USHORT nP = mpImplLB->GetEntryList()->GetSelectEntryPos( n );
                if ( !aSelInText.IsKeyValid( ImplCreateKey( nP ) ) )
                {
                    if ( aText.Len() && (aText.GetChar( aText.Len()-1 ) != mcMultiSep) )
                        aText += mcMultiSep;
                    if ( aText.Len() )
                        aText += ' ';   // etwas auflockern
                    aText += mpImplLB->GetEntryList()->GetEntryText( nP );
                    aText += mcMultiSep;
                }
            }
            if ( aText.Len() && (aText.GetChar( aText.Len()-1 ) == mcMultiSep) )
                aText.Erase( aText.Len()-1, 1 );
        }
        else
        {
            aText = mpImplLB->GetEntryList()->GetSelectEntry( 0 );
        }

        mpSubEdit->SetText( aText );

        Selection aNewSelection( 0, aText.Len() );
        if ( IsMultiSelectionEnabled() )
            aNewSelection.Min() = aText.Len();
        mpSubEdit->SetSelection( aNewSelection );

        mpSubEdit->SetModifyFlag();
        mbSyntheticModify = TRUE;
        Modify();
        mbSyntheticModify = FALSE;
        Select();
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplCancelHdl, void*, EMPTYARG )
{
    if( IsInDropDown() )
        mpFloatWin->EndPopupMode();

    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplSelectionChangedHdl, void*, n )
{
    if ( !mpImplLB->IsTrackingSelect() )
    {
        USHORT nChanged = (USHORT)(ULONG)n;
        if ( !mpSubEdit->IsReadOnly() && mpImplLB->GetEntryList()->IsEntryPosSelected( nChanged ) )
            mpSubEdit->SetText( mpImplLB->GetEntryList()->GetEntryText( nChanged ) );
    }
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplDoubleClickHdl, void*, EMPTYARG )
{
    DoubleClick();
    return 0;
}

// -----------------------------------------------------------------------

void ComboBox::Select()
{
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void ComboBox::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ComboBox::EnableAutoSize( BOOL bAuto )
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

void ComboBox::SetDropDownLineCount( USHORT nLines )
{
    if ( mpFloatWin )
        mpFloatWin->SetDropDownLineCount( nLines );
}

// -----------------------------------------------------------------------

USHORT ComboBox::GetDropDownLineCount() const
{
    USHORT nLines = 0;
    if ( mpFloatWin )
        nLines = mpFloatWin->GetDropDownLineCount();
    return nLines;
}

// -----------------------------------------------------------------------

void ComboBox::SetPosSizePixel( long nX, long nY, long nWidth, long nHeight,
                                USHORT nFlags )
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

    Edit::SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

// -----------------------------------------------------------------------

void ComboBox::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    if( IsDropDownBox() )
    {
        long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
        nSBWidth = CalcZoom( nSBWidth );
        mpSubEdit->SetSizePixel( Size( aOutSz.Width() - nSBWidth, aOutSz.Height() ) );
        mpBtn->SetPosSizePixel( aOutSz.Width() - nSBWidth, 0, nSBWidth, aOutSz.Height() );
    }
    else
    {
        mpSubEdit->SetSizePixel( Size( aOutSz.Width(), mnDDHeight ) );
        mpImplLB->SetPosSizePixel( 0, mnDDHeight, aOutSz.Width(), aOutSz.Height() - mnDDHeight );
        if ( GetText().Len() )
            ImplUpdateFloatSelection();
    }

    // FloatingWindow-Groesse auch im unsichtbare Zustand auf Stand halten,
    // weil KEY_PGUP/DOWN ausgewertet wird...
    if ( mpFloatWin )
        mpFloatWin->SetSizePixel( mpFloatWin->CalcFloatSize() );
}

// -----------------------------------------------------------------------

void ComboBox::StateChanged( StateChangedType nType )
{
    Edit::StateChanged( nType );

    if ( ( nType == STATE_CHANGE_ENABLE ) || ( nType == STATE_CHANGE_READONLY ) )
    {
        mpImplLB->SetReadOnly( IsReadOnly() );
        if ( mpBtn )
            mpBtn->Enable( IsEnabled() && !IsReadOnly() );
    }
    else if ( nType == STATE_CHANGE_ENABLE )
    {
        mpSubEdit->Enable( IsEnabled() );
        mpImplLB->Enable( IsEnabled() && !IsReadOnly() );
        if ( mpBtn )
            mpBtn->Enable( IsEnabled() && !IsReadOnly() );
        Invalidate();
    }
    else if( nType == STATE_CHANGE_UPDATEMODE )
    {
        mpImplLB->SetUpdateMode( IsUpdateMode() );
    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        mpImplLB->SetZoom( GetZoom() );
        mpSubEdit->SetZoom( GetZoom() );
        ImplCalcEditHeight();
        Resize();
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        mpImplLB->SetControlFont( GetControlFont() );
        mpSubEdit->SetControlFont( GetControlFont() );
        ImplCalcEditHeight();
        Resize();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        mpImplLB->SetControlForeground( GetControlForeground() );
        mpSubEdit->SetControlForeground( GetControlForeground() );
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        mpImplLB->SetControlBackground( GetControlBackground() );
        mpSubEdit->SetControlBackground( GetControlBackground() );
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        mpImplLB->GetMainWindow()->EnableSort( ( GetStyle() & WB_SORT ) ? TRUE : FALSE );
    }
}

// -----------------------------------------------------------------------

void ComboBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        if ( mpBtn )
        {
            mpBtn->SetSettings( GetSettings() );
            ImplInitDropDownButton( mpBtn );
        }
        Resize();
        mpImplLB->Resize(); // Wird nicht durch ComboBox::Resize() gerufen, wenn sich die ImplLB nicht aendert.
    }
}

// -----------------------------------------------------------------------

long ComboBox::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;

    if( ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN ) && ( rNEvt.GetWindow() == mpImplLB->GetMainWindow() ) )
    {
        mpSubEdit->GrabFocus();
    }

    return nDone ? nDone : Edit::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long ComboBox::Notify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if( ( rNEvt.GetType() == EVENT_KEYINPUT ) && ( rNEvt.GetWindow() == mpSubEdit )
            && !IsReadOnly() )
    {
        KeyEvent aKeyEvt = *rNEvt.GetKeyEvent();
        USHORT   nKeyCode = aKeyEvt.GetKeyCode().GetCode();
        switch( nKeyCode )
        {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
            {
                ImplUpdateFloatSelection();
                if( ( nKeyCode == KEY_DOWN ) && mpFloatWin && !mpFloatWin->IsInPopupMode() && aKeyEvt.GetKeyCode().IsMod2() )
                {
                    mpBtn->SetPressed( TRUE );
                    if ( mpImplLB->GetEntryList()->GetMRUCount() )
                        mpImplLB->SelectEntry( 0 , TRUE );
                    mpFloatWin->StartFloat( FALSE );
                    nDone = 1;
                }
                else if( ( nKeyCode == KEY_UP ) && mpFloatWin && mpFloatWin->IsInPopupMode() && aKeyEvt.GetKeyCode().IsMod2() )
                {
                    mpFloatWin->EndPopupMode();
                    nDone = 1;
                }
                else
                    nDone = mpImplLB->ProcessKeyInput( aKeyEvt );
            }
            break;

            case KEY_RETURN:
            {
                if( ( rNEvt.GetWindow() == mpSubEdit ) && IsInDropDown() )
                {
                    mpImplLB->ProcessKeyInput( aKeyEvt );
                    nDone = 1;
                }
            }
            break;
        }
    }
    else if ( (rNEvt.GetType() == EVENT_LOSEFOCUS) && mpFloatWin )
    {
        if( mpFloatWin->HasChildPathFocus() )
            mpSubEdit->GrabFocus();
        else if ( mpFloatWin->IsInPopupMode() && !HasChildPathFocus( TRUE ) )
            mpFloatWin->EndPopupMode();
    }
    else if( (rNEvt.GetType() == EVENT_COMMAND) &&
             (rNEvt.GetCommandEvent()->GetCommand() == COMMAND_WHEEL) &&
             (rNEvt.GetWindow() == mpSubEdit) )
    {
        nDone = mpImplLB->HandleWheelAsCursorTravel( *rNEvt.GetCommandEvent() );
    }

    return nDone ? nDone : Edit::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void ComboBox::SetText( const XubString& rStr )
{
    Edit::SetText( rStr );
    ImplUpdateFloatSelection();
}

// -----------------------------------------------------------------------

void ComboBox::SetText( const XubString& rStr, const Selection& rNewSelection )
{
    Edit::SetText( rStr, rNewSelection );
    ImplUpdateFloatSelection();
}

// -----------------------------------------------------------------------

void ComboBox::Modify()
{
    if ( !mbSyntheticModify )
        ImplUpdateFloatSelection();

    Edit::Modify();
}

// -----------------------------------------------------------------------

void ComboBox::ImplUpdateFloatSelection()
{
    // Text in der ListBox in den sichtbaren Bereich bringen
    mpImplLB->SetCallSelectionChangedHdl( FALSE );
    if ( !IsMultiSelectionEnabled() )
    {
        XubString   aSearchStr( mpSubEdit->GetText() );
        USHORT      nSelect = LISTBOX_ENTRY_NOTFOUND;
        BOOL        bSelect = TRUE;

        if ( mpImplLB->GetCurrentPos() != LISTBOX_ENTRY_NOTFOUND )
        {
            XubString aCurrent = mpImplLB->GetEntryList()->GetEntryText( mpImplLB->GetCurrentPos() );
            if ( aCurrent == aSearchStr )
                nSelect = mpImplLB->GetCurrentPos();
        }

        if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
            nSelect = mpImplLB->GetEntryList()->FindEntry( aSearchStr, MATCH_CASE, STRING_LEN );
        if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
        {
            nSelect = mpImplLB->GetEntryList()->FindEntry( aSearchStr, MATCH_BEST, aSearchStr.Len() );
            bSelect = FALSE;
        }

        if( nSelect != LISTBOX_ENTRY_NOTFOUND )
        {
            if ( !mpImplLB->IsVisible( nSelect ) )
                mpImplLB->SetTopEntry( nSelect );
            mpImplLB->SelectEntry( nSelect, bSelect );
        }
        else
        {
            nSelect = mpImplLB->GetEntryList()->GetSelectEntryPos( 0 );
            if( nSelect != LISTBOX_ENTRY_NOTFOUND )
                mpImplLB->SelectEntry( nSelect, FALSE );
            mpImplLB->SetTopEntry( 0 );
            mpImplLB->ResetCurrentPos();
        }
    }
    else
    {
        Table aSelInText;
        lcl_GetSelectedEntries( aSelInText, mpSubEdit->GetText(), mcMultiSep, mpImplLB->GetEntryList() );
        for ( USHORT n = 0; n < mpImplLB->GetEntryList()->GetEntryCount(); n++ )
            mpImplLB->SelectEntry( n, aSelInText.IsKeyValid( ImplCreateKey( n ) ) );
    }
    mpImplLB->SetCallSelectionChangedHdl( TRUE );
}

// -----------------------------------------------------------------------

USHORT ComboBox::InsertEntry( const XubString& rStr, USHORT nPos )
{
    USHORT nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rStr );
    nRealPos -= mpImplLB->GetEntryList()->GetMRUCount();
    return nRealPos;
}

// -----------------------------------------------------------------------

USHORT ComboBox::InsertEntry( const XubString& rStr, const Image& rImage, USHORT nPos )
{
    USHORT nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rStr, rImage );
    nRealPos -= mpImplLB->GetEntryList()->GetMRUCount();
    return nRealPos;
}

// -----------------------------------------------------------------------

void ComboBox::RemoveEntry( const XubString& rStr )
{
    RemoveEntry( GetEntryPos( rStr ) );
}

// -----------------------------------------------------------------------

void ComboBox::RemoveEntry( USHORT nPos )
{
    mpImplLB->RemoveEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

void ComboBox::Clear()
{
    mpImplLB->Clear();
}

// -----------------------------------------------------------------------

USHORT ComboBox::GetEntryPos( const XubString& rStr ) const
{
    USHORT nPos = mpImplLB->GetEntryList()->FindEntry( rStr, MATCH_CASE, STRING_LEN, mpImplLB->GetEntryList()->GetMRUCount() );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos -= mpImplLB->GetEntryList()->GetMRUCount();
    return nPos;
}

// -----------------------------------------------------------------------

USHORT ComboBox::GetEntryPos( const void* pData ) const
{
    USHORT nPos = mpImplLB->GetEntryList()->FindEntry( pData );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos -= mpImplLB->GetEntryList()->GetMRUCount();
    return nPos;
}

// -----------------------------------------------------------------------

XubString ComboBox::GetEntry( USHORT nPos ) const
{
    return mpImplLB->GetEntryList()->GetEntryText( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

USHORT ComboBox::GetEntryCount() const
{
    return mpImplLB->GetEntryList()->GetEntryCount() - mpImplLB->GetEntryList()->GetMRUCount();
}

// -----------------------------------------------------------------------

BOOL ComboBox::IsTravelSelect() const
{
    return mpImplLB->IsTravelSelect();
}

// -----------------------------------------------------------------------

BOOL ComboBox::IsInDropDown() const
{
    return mpFloatWin && mpFloatWin->IsInPopupMode();
}

// -----------------------------------------------------------------------

void ComboBox::EnableMultiSelection( BOOL bMulti )
{
    mpImplLB->EnableMultiSelection( bMulti );
    mpImplLB->SetMultiSelectionSimpleMode( TRUE );
}

// -----------------------------------------------------------------------

BOOL ComboBox::IsMultiSelectionEnabled() const
{
    return mpImplLB->IsMultiSelectionEnabled();
}

// -----------------------------------------------------------------------

long ComboBox::CalcWindowSizePixel( USHORT nLines ) const
{
    return mpImplLB->GetEntryHeight() * nLines;
}

// -----------------------------------------------------------------------

Size ComboBox::CalcMinimumSize() const
{
    Size aSz;
    if ( !IsDropDownBox() )
    {
        aSz = mpImplLB->CalcSize( mpImplLB->GetEntryList()->GetEntryCount() );
        aSz.Height() += mnDDHeight;
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

Size ComboBox::CalcAdjustedSize( const Size& rPrefSize ) const
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
        aSz.Height() += mnDDHeight;
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

Size ComboBox::CalcSize( USHORT nColumns, USHORT nLines ) const
{
    // ggf. werden ScrollBars eingeblendet
    Size aMinSz = CalcMinimumSize();
    Size aSz;

    // Hoehe
    if ( nLines )
    {
        if ( !IsDropDownBox() )
            aSz.Height() = mpImplLB->CalcSize( nLines ).Height() + mnDDHeight;
        else
            aSz.Height() = mnDDHeight;
    }
    else
        aSz.Height() = aMinSz.Height();

    // Breite
    if ( nColumns )
        aSz.Width() = nColumns * GetTextWidth( UniString( 'X' ) );
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

void ComboBox::GetMaxVisColumnsAndLines( USHORT& rnCols, USHORT& rnLines ) const
{
    long nCharWidth = GetTextWidth( UniString( 'x' ) );
    if ( !IsDropDownBox() )
    {
        Size aOutSz = mpImplLB->GetMainWindow()->GetOutputSizePixel();
        rnCols = (USHORT)(aOutSz.Width()/nCharWidth);
        rnLines = (USHORT)(aOutSz.Height()/mpImplLB->GetEntryHeight());
    }
    else
    {
        Size aOutSz = mpSubEdit->GetOutputSizePixel();
        rnCols = (USHORT)(aOutSz.Width()/nCharWidth);
        rnLines = 1;
    }
}

// -----------------------------------------------------------------------

void ComboBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags )
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
        // aRect.Top() += nEditHeight;
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
    if ( IsDropDownBox() )
    {
        mpSubEdit->Draw( pDev, aPos, aSize, nFlags );
        // DD-Button ?
    }
    else
    {
        long nOnePixel = GetDrawPixel( pDev, 1 );
        long nTextHeight = pDev->GetTextHeight();
        long nEditHeight = nTextHeight + 6*nOnePixel;

        mpSubEdit->Draw( pDev, aPos, Size( aSize.Width(), nEditHeight ), nFlags );

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

        Rectangle aClip( aPos, aSize );
        pDev->IntersectClipRegion( aClip );
        USHORT nLines = (USHORT) ( (aSize.Height()-nEditHeight) / nTextHeight );
        if ( !nLines )
            nLines = 1;
        USHORT nTEntry = IsReallyVisible() ? mpImplLB->GetTopEntry() : 0;
        for ( USHORT n = 0; n < nLines; n++ )
            pDev->DrawText( Point( aPos.X() + 3*nOnePixel, aPos.Y() + n*nTextHeight + nEditHeight + nOnePixel ), mpImplLB->GetEntryList()->GetEntryText( n+nTEntry ) );
    }

    pDev->Pop();
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplUserDrawHdl, UserDrawEvent*, pEvent )
{
    UserDraw( *pEvent );
    return 1;
}

// -----------------------------------------------------------------------

void ComboBox::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void ComboBox::SetUserItemSize( const Size& rSz )
{
    mpImplLB->GetMainWindow()->SetUserItemSize( rSz );
}

// -----------------------------------------------------------------------

const Size& ComboBox::GetUserItemSize() const
{
    return mpImplLB->GetMainWindow()->GetUserItemSize();
}

// -----------------------------------------------------------------------

void ComboBox::EnableUserDraw( BOOL bUserDraw )
{
    mpImplLB->GetMainWindow()->EnableUserDraw( bUserDraw );
}

// -----------------------------------------------------------------------

BOOL ComboBox::IsUserDrawEnabled() const
{
    return mpImplLB->GetMainWindow()->IsUserDrawEnabled();
}

// -----------------------------------------------------------------------

#if SUPD < 593
void ComboBox::DrawEntry( const UserDrawEvent& rEvt, BOOL bDrawImage, BOOL bDrawText )
{
    DBG_ASSERT( rEvt.GetDevice() == mpImplLB->GetMainWindow(), "DrawEntry?!" );
    mpImplLB->GetMainWindow()->DrawEntry( rEvt.GetItemId(), bDrawImage, bDrawText );
}
#endif

void ComboBox::DrawEntry( const UserDrawEvent& rEvt, BOOL bDrawImage, BOOL bDrawText, BOOL bDrawTextAtImagePos )
{
    DBG_ASSERT( rEvt.GetDevice() == mpImplLB->GetMainWindow(), "DrawEntry?!" );
    mpImplLB->GetMainWindow()->DrawEntry( rEvt.GetItemId(), bDrawImage, bDrawText, bDrawTextAtImagePos );
}

// -----------------------------------------------------------------------

void ComboBox::SetSeparatorPos( USHORT n )
{
    mpImplLB->SetSeparatorPos( n );
}

// -----------------------------------------------------------------------

void ComboBox::SetSeparatorPos()
{
    mpImplLB->SetSeparatorPos( LISTBOX_ENTRY_NOTFOUND );
}

// -----------------------------------------------------------------------

USHORT ComboBox::GetSeparatorPos() const
{
    return mpImplLB->GetSeparatorPos();
}

// -----------------------------------------------------------------------

void ComboBox::SetMRUEntries( const XubString& rEntries, xub_Unicode cSep )
{
    mpImplLB->SetMRUEntries( rEntries, cSep );
}

// -----------------------------------------------------------------------

XubString ComboBox::GetMRUEntries( xub_Unicode cSep ) const
{
    return mpImplLB->GetMRUEntries( cSep );
}

// -----------------------------------------------------------------------

void ComboBox::SetMaxMRUCount( USHORT n )
{
    mpImplLB->SetMaxMRUCount( n );
}

// -----------------------------------------------------------------------

USHORT ComboBox::GetMaxMRUCount() const
{
    return mpImplLB->GetMaxMRUCount();
}

// -----------------------------------------------------------------------

void ComboBox::SetEntryData( USHORT nPos, void* pNewData )
{
    mpImplLB->SetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount(), pNewData );
}

// -----------------------------------------------------------------------

void* ComboBox::GetEntryData( USHORT nPos ) const
{
    return mpImplLB->GetEntryList()->GetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}
