/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: combobox.cxx,v $
 * $Revision: 1.48 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/table.hxx>
#include <tools/debug.hxx>

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#include <vcl/svdata.hxx>
#include <vcl/decoview.hxx>
#include <vcl/ilstbox.hxx>
#include <vcl/lstbox.h>
#include <vcl/button.hxx>
#include <vcl/subedit.hxx>
#include <vcl/event.hxx>
#include <vcl/combobox.hxx>
#include <vcl/controllayout.hxx>



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
            rSelectedPos.Insert( ImplCreateKey( nPos ), (void*)sal_IntPtr(1L) );
    }
}

// =======================================================================

ComboBox::ComboBox( WindowType nType ) :
    Edit( nType )
{
    ImplInitComboBoxData();
}

// -----------------------------------------------------------------------

ComboBox::ComboBox( Window* pParent, WinBits nStyle ) :
    Edit( WINDOW_COMBOBOX )
{
    ImplInitComboBoxData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ComboBox::ComboBox( Window* pParent, const ResId& rResId ) :
    Edit( WINDOW_COMBOBOX )
{
    ImplInitComboBoxData();
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

void ComboBox::ImplInitComboBoxData()
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

    EnableRTL( TRUE );
}

// -----------------------------------------------------------------------

void ComboBox::ImplCalcEditHeight()
{
    sal_Int32 nLeft, nTop, nRight, nBottom;
    GetBorder( nLeft, nTop, nRight, nBottom );
    mnDDHeight = (USHORT)(mpSubEdit->GetTextHeight() + nTop + nBottom + 4);
    if ( !IsDropDownBox() )
        mnDDHeight += 4;

    // FIXME: currently only on aqua; see if we can use this on other platforms
    if( ImplGetSVData()->maNWFData.mbNoFocusRects )
    {
        Region aCtrlRegion( Rectangle( (const Point&)Point(), Size( 10, 10 ) ) );
        Region aBoundRegion, aContentRegion;
        ImplControlValue aControlValue;
        ControlType aType = IsDropDownBox() ? CTRL_COMBOBOX : CTRL_EDITBOX;
        if( GetNativeControlRegion( aType, PART_ENTIRE_CONTROL,
                                    aCtrlRegion,
                                    CTRL_STATE_ENABLED,
                                    aControlValue, rtl::OUString(),
                                    aBoundRegion, aContentRegion ) )
        {
            const long nNCHeight = aBoundRegion.GetBoundRect().GetHeight();
            if( mnDDHeight < nNCHeight )
                mnDDHeight = sal::static_int_cast<USHORT>( nNCHeight );
        }
    }
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
    WinBits nEditStyle = nStyle & ( WB_LEFT | WB_RIGHT | WB_CENTER );
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
    mpSubEdit->EnableRTL( FALSE );
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
    else
        mpImplLB->GetMainWindow()->AllowGrabFocus( TRUE );

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

    ULONG nNumber = ReadLongRes();

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
    SetSelection( Selection( 0, SELECTION_MAX ) );
    mpFloatWin->StartFloat( TRUE );
    ImplCallEventListeners( VCLEVENT_DROPDOWN_OPEN );

    ImplClearLayoutData();
    if( mpImplLB )
        mpImplLB->GetMainWindow()->ImplClearLayoutData();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplPopupModeEndHdl, void*, EMPTYARG )
{
    if( mpFloatWin->IsPopupModeCanceled() )
    {
        if ( !mpImplLB->GetEntryList()->IsEntryPosSelected( mpFloatWin->GetPopupModeStartSaveSelection() ) )
        {
            mpImplLB->SelectEntry( mpFloatWin->GetPopupModeStartSaveSelection(), TRUE );
            BOOL bTravelSelect = mpImplLB->IsTravelSelect();
            mpImplLB->SetTravelSelect( TRUE );
            Select();
            mpImplLB->SetTravelSelect( bTravelSelect );
        }
    }

    ImplClearLayoutData();
    if( mpImplLB )
        mpImplLB->GetMainWindow()->ImplClearLayoutData();

    mpBtn->SetPressed( FALSE );
    ImplCallEventListeners( VCLEVENT_DROPDOWN_CLOSE );
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
            nStart = nStart ? nStart - 1 : mpImplLB->GetEntryList()->GetEntryCount()-1;
        }
        BOOL bLazy = mbMatchCase ? FALSE : TRUE;
        // 1. Try match full from current position
        USHORT nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, nStart, bForward, FALSE );
        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            // 2. Match full, but from start
            nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, bForward ? 0 : (mpImplLB->GetEntryList()->GetEntryCount()-1), bForward, FALSE );
        if ( ( nPos == LISTBOX_ENTRY_NOTFOUND ) && bLazy )
            // 3. Try match lazy from current position
            nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, nStart, bForward, TRUE );
        if ( ( nPos == LISTBOX_ENTRY_NOTFOUND ) && bLazy )
            // 4. Try match lazy, but from start
            nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, bForward ? 0 : (mpImplLB->GetEntryList()->GetEntryCount()-1), bForward, bLazy );

        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            XubString aText = mpImplLB->GetEntryList()->GetEntryText( nPos );
            Selection aSelection( aText.Len(), aStartText.Len() );
            pEdit->SetText( aText, aSelection );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplSelectHdl, void*, EMPTYARG )
{
    BOOL bPopup = IsInDropDown();
    BOOL bCallSelect = FALSE;
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
                    nIndex = sal::static_int_cast<xub_StrLen>(nIndex - nTokenLen);
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

        bCallSelect = TRUE;
    }

    // #84652# Call GrabFocus and EndPopupMode before calling Select/Modify, but after changing the text

    if ( bPopup && !mpImplLB->IsTravelSelect() &&
        ( !IsMultiSelectionEnabled() || !mpImplLB->GetSelectModifier() ) )
    {
        mpFloatWin->EndPopupMode();
        GrabFocus();
    }

    if ( bCallSelect )
    {
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

void ComboBox::ToggleDropDown()
{
    if( IsDropDownBox() )
    {
        if( mpFloatWin->IsInPopupMode() )
            mpFloatWin->EndPopupMode();
        else
        {
            mpSubEdit->GrabFocus();
            if ( !mpImplLB->GetEntryList()->GetMRUCount() )
                ImplUpdateFloatSelection();
            else
                mpImplLB->SelectEntry( 0 , TRUE );
            mpBtn->SetPressed( TRUE );
            SetSelection( Selection( 0, SELECTION_MAX ) );
            mpFloatWin->StartFloat( TRUE );
            ImplCallEventListeners( VCLEVENT_DROPDOWN_OPEN );
        }
    }
}

// -----------------------------------------------------------------------

void ComboBox::Select()
{
    ImplCallEventListenersAndHandler( VCLEVENT_COMBOBOX_SELECT, maSelectHdl, this );
}

// -----------------------------------------------------------------------

void ComboBox::DoubleClick()
{
    ImplCallEventListenersAndHandler( VCLEVENT_COMBOBOX_DOUBLECLICK, maDoubleClickHdl, this );
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

void ComboBox::EnableDDAutoWidth( BOOL b )
{
    if ( mpFloatWin )
        mpFloatWin->SetAutoWidth( b );
}

 // -----------------------------------------------------------------------

BOOL ComboBox::IsDDAutoWidthEnabled() const
{
    return mpFloatWin ? mpFloatWin->IsAutoWidth() : FALSE;
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
        if ( ( nFlags & WINDOW_POSSIZE_HEIGHT ) && ( nHeight >= 2*mnDDHeight ) )
            aPrefSz.Height() = nHeight-mnDDHeight;
        if ( nFlags & WINDOW_POSSIZE_WIDTH )
            aPrefSz.Width() = nWidth;
        mpFloatWin->SetPrefSize( aPrefSz );

        if ( IsAutoSizeEnabled() && ! (nFlags & WINDOW_POSSIZE_DROPDOWN) )
            nHeight = mnDDHeight;
    }

    Edit::SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

// -----------------------------------------------------------------------

void ComboBox::Resize()
{
    Control::Resize();

    Size aOutSz = GetOutputSizePixel();
    if( IsDropDownBox() )
    {
        long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
        long    nTop = 0;
        long    nBottom = aOutSz.Height();

        Window *pBorder = GetWindow( WINDOW_BORDER );
        ImplControlValue aControlValue;
        Point aPoint;
        Region aContent, aBound;

        // use the full extent of the control
        Region aArea( Rectangle(aPoint, pBorder->GetOutputSizePixel()) );

        if ( GetNativeControlRegion(CTRL_COMBOBOX, PART_BUTTON_DOWN,
                aArea, 0, aControlValue, rtl::OUString(), aBound, aContent) )
        {
            // convert back from border space to local coordinates
            aPoint = pBorder->ScreenToOutputPixel( OutputToScreenPixel( aPoint ) );
            aContent.Move(-aPoint.X(), -aPoint.Y());

            mpBtn->SetPosSizePixel( aContent.GetBoundRect().Left(), nTop, aContent.GetBoundRect().getWidth(), (nBottom-nTop) );

            // adjust the size of the edit field
            if ( GetNativeControlRegion(CTRL_COMBOBOX, PART_SUB_EDIT,
                        aArea, 0, aControlValue, rtl::OUString(), aBound, aContent) )
            {
                // convert back from border space to local coordinates
                aContent.Move(-aPoint.X(), -aPoint.Y());

                // use the themes drop down size
                Rectangle aContentRect = aContent.GetBoundRect();
                mpSubEdit->SetPosSizePixel( aContentRect.TopLeft(), aContentRect.GetSize() );
            }
            else
            {
                // use the themes drop down size for the button
                aOutSz.Width() -= aContent.GetBoundRect().getWidth();
                mpSubEdit->SetSizePixel( aOutSz );
            }
        }
        else
        {
            nSBWidth = CalcZoom( nSBWidth );
            mpSubEdit->SetSizePixel( Size( aOutSz.Width() - nSBWidth, aOutSz.Height() ) );
            mpBtn->SetPosSizePixel( aOutSz.Width() - nSBWidth, nTop, nSBWidth, (nBottom-nTop) );
        }
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

void ComboBox::FillLayoutData() const
{
    mpLayoutData = new vcl::ControlLayoutData();
    AppendLayoutData( *mpSubEdit );
    mpSubEdit->SetLayoutDataParent( this );
    Control* pMainWindow = mpImplLB->GetMainWindow();
    if( mpFloatWin )
    {
        // dropdown mode
        if( mpFloatWin->IsReallyVisible() )
        {
            AppendLayoutData( *pMainWindow );
            pMainWindow->SetLayoutDataParent( this );
        }
    }
    else
    {
        AppendLayoutData( *pMainWindow );
        pMainWindow->SetLayoutDataParent( this );
    }
}

// -----------------------------------------------------------------------

void ComboBox::StateChanged( StateChangedType nType )
{
    Edit::StateChanged( nType );

    if ( nType == STATE_CHANGE_READONLY )
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
        SetBackground();    // due to a hack in Window::UpdateSettings the background must be reset
                            // otherwise it will overpaint NWF drawn comboboxes
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
                    SetSelection( Selection( 0, SELECTION_MAX ) );
                    mpFloatWin->StartFloat( FALSE );
                    ImplCallEventListeners( VCLEVENT_DROPDOWN_OPEN );
                    nDone = 1;
                }
                else if( ( nKeyCode == KEY_UP ) && mpFloatWin && mpFloatWin->IsInPopupMode() && aKeyEvt.GetKeyCode().IsMod2() )
                {
                    mpFloatWin->EndPopupMode();
                    nDone = 1;
                }
                else
                {
                    if( mpFloatWin )
                        mpImplLB->GetMainWindow()->CalcMaxVisibleEntries( mpFloatWin->CalcFloatSize() );
                    nDone = mpImplLB->ProcessKeyInput( aKeyEvt );
                }
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
        if( ! GetSettings().GetMouseSettings().GetNoWheelActionWithoutFocus() || HasChildPathFocus() )
            nDone = mpImplLB->HandleWheelAsCursorTravel( *rNEvt.GetCommandEvent() );
        else
            nDone = 0;  // don't eat this event, let the default handling happen (i.e. scroll the context)
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
            nSelect = mpImplLB->GetEntryList()->FindEntry( aSearchStr );
        if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
        {
            nSelect = mpImplLB->GetEntryList()->FindMatchingEntry( aSearchStr );
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
            // mpImplLB->SetTopEntry( 0 ); #92555# Ugly....
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
    nRealPos = sal::static_int_cast<USHORT>(nRealPos - mpImplLB->GetEntryList()->GetMRUCount());
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMADDED, (void*) sal_IntPtr(nRealPos) );
    return nRealPos;
}

// -----------------------------------------------------------------------

USHORT ComboBox::InsertEntry( const XubString& rStr, const Image& rImage, USHORT nPos )
{
    USHORT nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rStr, rImage );
    nRealPos = sal::static_int_cast<USHORT>(nRealPos - mpImplLB->GetEntryList()->GetMRUCount());
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMADDED, (void*) sal_IntPtr(nRealPos) );
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
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMREMOVED, (void*) sal_IntPtr(nPos) );
}

// -----------------------------------------------------------------------

void ComboBox::Clear()
{
    mpImplLB->Clear();
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMREMOVED, (void*) sal_IntPtr(-1) );
}

// -----------------------------------------------------------------------

USHORT ComboBox::GetEntryPos( const XubString& rStr ) const
{
    USHORT nPos = mpImplLB->GetEntryList()->FindEntry( rStr );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos = sal::static_int_cast<USHORT>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
    return nPos;
}

// -----------------------------------------------------------------------

USHORT ComboBox::GetEntryPos( const void* pData ) const
{
    USHORT nPos = mpImplLB->GetEntryList()->FindEntry( pData );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos = sal::static_int_cast<USHORT>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
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
    mpImplLB->EnableMultiSelection( bMulti, FALSE );
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

Size ComboBox::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return CalcMinimumSize();
    default:
        return Edit::GetOptimalSize( eType );
    }
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
    sal_Int32 nLeft, nTop, nRight, nBottom;
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
            ImplDrawFrame( pDev, aRect );
        }
        if ( bBackground )
        {
            pDev->SetFillColor( GetControlBackground() );
            pDev->DrawRect( aRect );
        }
    }

    // Inhalt
    if ( !IsDropDownBox() )
    {
        long        nOnePixel = GetDrawPixel( pDev, 1 );
        long        nTextHeight = pDev->GetTextHeight();
        long        nEditHeight = nTextHeight + 6*nOnePixel;
        USHORT      nTextStyle = TEXT_DRAW_VCENTER;

        // First, draw the edit part
        mpSubEdit->Draw( pDev, aPos, Size( aSize.Width(), nEditHeight ), nFlags );

        // Second, draw the listbox
        if ( GetStyle() & WB_CENTER )
            nTextStyle |= TEXT_DRAW_CENTER;
        else if ( GetStyle() & WB_RIGHT )
            nTextStyle |= TEXT_DRAW_RIGHT;
        else
            nTextStyle |= TEXT_DRAW_LEFT;

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

        Rectangle aTextRect( aPos, aSize );

        aTextRect.Left() += 3*nOnePixel;
        aTextRect.Right() -= 3*nOnePixel;
        aTextRect.Top() += nEditHeight + nOnePixel;
        aTextRect.Bottom() = aTextRect.Top() + nTextHeight;

        // the drawing starts here
        for ( USHORT n = 0; n < nLines; n++ )
        {
            pDev->DrawText( aTextRect, mpImplLB->GetEntryList()->GetEntryText( n+nTEntry ), nTextStyle );
            aTextRect.Top() += nTextHeight;
            aTextRect.Bottom() += nTextHeight;
        }
    }

    pDev->Pop();

    // Call Edit::Draw after restoring the MapMode...
    if ( IsDropDownBox() )
    {
        mpSubEdit->Draw( pDev, rPos, rSize, nFlags );
        // DD-Button ?
    }

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

USHORT ComboBox::GetDisplayLineCount() const
{
    return mpImplLB->GetDisplayLineCount();
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

// -----------------------------------------------------------------------

void ComboBox::SetTopEntry( USHORT nPos )
{
    mpImplLB->SetTopEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

USHORT ComboBox::GetTopEntry() const
{
    USHORT nPos = GetEntryCount() ? mpImplLB->GetTopEntry() : LISTBOX_ENTRY_NOTFOUND;
    if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
        nPos = 0;
    return nPos;
}

// -----------------------------------------------------------------------

Rectangle ComboBox::GetDropDownPosSizePixel() const
{
    return mpFloatWin ? mpFloatWin->GetWindowExtentsRelative( const_cast<ComboBox*>(this) ) : Rectangle();
}

// -----------------------------------------------------------------------

Rectangle ComboBox::GetListPosSizePixel() const
{
    return mpFloatWin ? Rectangle() : mpImplLB->GetMainWindow()->GetWindowExtentsRelative( const_cast<ComboBox*>(this) );
}

// -----------------------------------------------------------------------

const Wallpaper& ComboBox::GetDisplayBackground() const
{
    if( ! mpSubEdit->IsBackground() )
        return Control::GetDisplayBackground();

    const Wallpaper& rBack = mpSubEdit->GetBackground();
    if( ! rBack.IsBitmap() &&
        ! rBack.IsGradient() &&
        rBack.GetColor().GetColor() == COL_TRANSPARENT
        )
        return Control::GetDisplayBackground();
    return rBack;
}
// -----------------------------------------------------------------------------
USHORT ComboBox::GetSelectEntryCount() const
{
    return mpImplLB->GetEntryList()->GetSelectEntryCount();
}
// -----------------------------------------------------------------------------
USHORT ComboBox::GetSelectEntryPos( USHORT nIndex ) const
{
    USHORT nPos = mpImplLB->GetEntryList()->GetSelectEntryPos( nIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
            nPos = mpImplLB->GetEntryList()->FindEntry( mpImplLB->GetEntryList()->GetEntryText( nPos ) );
        nPos = sal::static_int_cast<USHORT>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
    }
    return nPos;
}
// -----------------------------------------------------------------------------
BOOL ComboBox::IsEntryPosSelected( USHORT nPos ) const
{
    return mpImplLB->GetEntryList()->IsEntryPosSelected( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}
// -----------------------------------------------------------------------------
void ComboBox::SelectEntryPos( USHORT nPos, BOOL bSelect)
{
    if ( nPos < mpImplLB->GetEntryList()->GetEntryCount() )
        mpImplLB->SelectEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), bSelect );
}
// -----------------------------------------------------------------------------
void ComboBox::SetNoSelection()
{
    mpImplLB->SetNoSelection();
    mpSubEdit->SetText( String() );
}
// -----------------------------------------------------------------------------
Rectangle ComboBox::GetBoundingRectangle( USHORT nItem ) const
{
    Rectangle aRect = mpImplLB->GetMainWindow()->GetBoundingRectangle( nItem );
    Rectangle aOffset = mpImplLB->GetMainWindow()->GetWindowExtentsRelative( (Window*)this );
    aRect.Move( aOffset.TopLeft().X(), aOffset.TopLeft().Y() );
    return aRect;
}
// -----------------------------------------------------------------------------

void ComboBox::SetBorderStyle( USHORT nBorderStyle )
{
    Window::SetBorderStyle( nBorderStyle );
    if ( !IsDropDownBox() )
    {
        mpSubEdit->SetBorderStyle( nBorderStyle );
        mpImplLB->SetBorderStyle( nBorderStyle );
    }
}
// -----------------------------------------------------------------------------

long ComboBox::GetIndexForPoint( const Point& rPoint, USHORT& rPos ) const
{
    if( ! mpLayoutData )
        FillLayoutData();

    // check whether rPoint fits at all
    long nIndex = Control::GetIndexForPoint( rPoint );
    if( nIndex != -1 )
    {
        // point must be either in main list window
        // or in impl window (dropdown case)
        ImplListBoxWindow* pMain = mpImplLB->GetMainWindow();

        // convert coordinates to ImplListBoxWindow pixel coordinate space
        Point aConvPoint = LogicToPixel( rPoint );
        aConvPoint = OutputToAbsoluteScreenPixel( aConvPoint );
        aConvPoint = pMain->AbsoluteScreenToOutputPixel( aConvPoint );
        aConvPoint = pMain->PixelToLogic( aConvPoint );

        // try to find entry
        USHORT nEntry = pMain->GetEntryPosForPoint( aConvPoint );
        if( nEntry == LISTBOX_ENTRY_NOTFOUND )
            nIndex = -1;
        else
            rPos = nEntry;
    }

    // get line relative index
    if( nIndex != -1 )
        nIndex = ToRelativeLineIndex( nIndex );

    return nIndex;
}
