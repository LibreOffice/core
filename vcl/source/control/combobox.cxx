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


#include <set>
#include <comphelper/string.hxx>
#include <tools/debug.hxx>
#include <tools/rc.h>
#include <vcl/decoview.hxx>
#include <vcl/lstbox.h>
#include <vcl/button.hxx>
#include <vcl/event.hxx>
#include <vcl/combobox.hxx>

#include <svdata.hxx>
#include <ilstbox.hxx>
#include <controldata.hxx>

// =======================================================================

static void lcl_GetSelectedEntries( ::std::set< sal_uInt16 >& rSelectedPos, const OUString& rText, sal_Unicode cTokenSep, const ImplEntryList* pEntryList )
{
    for (sal_Int32 n = comphelper::string::getTokenCount(rText, cTokenSep); n;)
    {
        OUString aToken = rText.getToken( --n, cTokenSep );
        aToken = comphelper::string::strip(aToken, ' ');
        sal_uInt16 nPos = pEntryList->FindEntry( aToken );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            rSelectedPos.insert( nPos );
    }
}

// =======================================================================

ComboBox::ComboBox( WindowType nType ) :
    Edit( nType )
{
    ImplInitComboBoxData();
    SetWidthInChars(-1);
}

// -----------------------------------------------------------------------

ComboBox::ComboBox( Window* pParent, WinBits nStyle ) :
    Edit( WINDOW_COMBOBOX )
{
    ImplInitComboBoxData();
    ImplInit( pParent, nStyle );
    SetWidthInChars(-1);
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

    SetWidthInChars(-1);
    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

ComboBox::~ComboBox()
{
    SetSubEdit( NULL );
    delete mpSubEdit;

    ImplListBox *pImplLB = mpImplLB;
    mpImplLB = NULL;
    delete pImplLB;

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
    mbDDAutoSize        = sal_True;
    mbSyntheticModify   = sal_False;
    mbMatchCase         = sal_False;
    mcMultiSep          = ';';
    m_nMaxWidthChars    = -1;
}

// -----------------------------------------------------------------------

void ComboBox::ImplCalcEditHeight()
{
    sal_Int32 nLeft, nTop, nRight, nBottom;
    GetBorder( nLeft, nTop, nRight, nBottom );
    mnDDHeight = (sal_uInt16)(mpSubEdit->GetTextHeight() + nTop + nBottom + 4);
    if ( !IsDropDownBox() )
        mnDDHeight += 4;

    Rectangle aCtrlRegion( Point( 0, 0 ), Size( 10, 10 ) );
    Rectangle aBoundRegion, aContentRegion;
    ImplControlValue aControlValue;
    ControlType aType = IsDropDownBox() ? CTRL_COMBOBOX : CTRL_EDITBOX;
    if( GetNativeControlRegion( aType, PART_ENTIRE_CONTROL,
                                aCtrlRegion,
                                CTRL_STATE_ENABLED,
                                aControlValue, OUString(),
                                aBoundRegion, aContentRegion ) )
    {
        const long nNCHeight = aBoundRegion.GetHeight();
        if( mnDDHeight < nNCHeight )
            mnDDHeight = sal::static_int_cast<sal_uInt16>( nNCHeight );
    }
}

// -----------------------------------------------------------------------

void ComboBox::ImplInit( Window* pParent, WinBits nStyle )
{
    ImplInitStyle( nStyle );

    bool bNoBorder = ( nStyle & WB_NOBORDER ) ? true : false;
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
        mpFloatWin->SetAutoWidth( sal_True );
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
    mpSubEdit->EnableRTL( sal_False );
    SetSubEdit( mpSubEdit );
    mpSubEdit->SetPosPixel( Point() );
    EnableAutocomplete( sal_True );
    mpSubEdit->Show();

    Window* pLBParent = this;
    if ( mpFloatWin )
        pLBParent = mpFloatWin;
    mpImplLB = new ImplListBox( pLBParent, nListStyle|WB_SIMPLEMODE|WB_AUTOHSCROLL );
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
        mpImplLB->GetMainWindow()->AllowGrabFocus( sal_True );

    ImplCalcEditHeight();

    SetCompoundControl( sal_True );
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

    sal_uLong nNumber = ReadLongRes();

    if( nNumber )
    {
        for( sal_uInt16 i = 0; i < nNumber; i++ )
        {
            InsertEntry( ReadStringRes(), LISTBOX_APPEND );
        }
    }
}

// -----------------------------------------------------------------------

void ComboBox::EnableAutocomplete( sal_Bool bEnable, sal_Bool bMatchCase )
{
    mbMatchCase = bMatchCase;

    if ( bEnable )
        mpSubEdit->SetAutocompleteHdl( LINK( this, ComboBox, ImplAutocompleteHdl ) );
    else
        mpSubEdit->SetAutocompleteHdl( Link() );
}

// -----------------------------------------------------------------------

sal_Bool ComboBox::IsAutocompleteEnabled() const
{
    return mpSubEdit->GetAutocompleteHdl().IsSet();
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ComboBox, ImplClickBtnHdl)
{
    ImplCallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
    mpSubEdit->GrabFocus();
    if ( !mpImplLB->GetEntryList()->GetMRUCount() )
        ImplUpdateFloatSelection();
    else
        mpImplLB->SelectEntry( 0 , sal_True );
    mpBtn->SetPressed( sal_True );
    SetSelection( Selection( 0, SELECTION_MAX ) );
    mpFloatWin->StartFloat( sal_True );
    ImplCallEventListeners( VCLEVENT_DROPDOWN_OPEN );

    ImplClearLayoutData();
    if( mpImplLB )
        mpImplLB->GetMainWindow()->ImplClearLayoutData();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ComboBox, ImplPopupModeEndHdl)
{
    if( mpFloatWin->IsPopupModeCanceled() )
    {
        if ( !mpImplLB->GetEntryList()->IsEntryPosSelected( mpFloatWin->GetPopupModeStartSaveSelection() ) )
        {
            mpImplLB->SelectEntry( mpFloatWin->GetPopupModeStartSaveSelection(), sal_True );
            sal_Bool bTravelSelect = mpImplLB->IsTravelSelect();
            mpImplLB->SetTravelSelect( sal_True );
            Select();
            mpImplLB->SetTravelSelect( bTravelSelect );
        }
    }

    ImplClearLayoutData();
    if( mpImplLB )
        mpImplLB->GetMainWindow()->ImplClearLayoutData();

    mpBtn->SetPressed( sal_False );
    ImplCallEventListeners( VCLEVENT_DROPDOWN_CLOSE );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ComboBox, ImplAutocompleteHdl, Edit*, pEdit )
{
    Selection           aSel = pEdit->GetSelection();
    AutocompleteAction  eAction = pEdit->GetAutocompleteAction();

    /* If there is no current selection do not auto complete on
       Tab/Shift-Tab since then we would not cycle to the next field.
    */
    if ( aSel.Len() ||
         ((eAction != AUTOCOMPLETE_TABFORWARD) && (eAction != AUTOCOMPLETE_TABBACKWARD)) )
    {
        OUString    aFullText = pEdit->GetText();
        OUString    aStartText = aFullText.copy( 0, (sal_Int32)aSel.Max() );
        sal_uInt16      nStart = mpImplLB->GetCurrentPos();

        if ( nStart == LISTBOX_ENTRY_NOTFOUND )
            nStart = 0;

        sal_Bool bForward = sal_True;
        if ( eAction == AUTOCOMPLETE_TABFORWARD )
            nStart++;
        else if ( eAction == AUTOCOMPLETE_TABBACKWARD )
        {
            bForward = sal_False;
            nStart = nStart ? nStart - 1 : mpImplLB->GetEntryList()->GetEntryCount()-1;
        }

        sal_uInt16 nPos = LISTBOX_ENTRY_NOTFOUND;
        if( ! mbMatchCase )
        {
            // Try match case insensitive from current position
            nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, nStart, bForward, sal_True );
            if ( nPos == LISTBOX_ENTRY_NOTFOUND )
                // Try match case insensitive, but from start
                nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, bForward ? 0 : (mpImplLB->GetEntryList()->GetEntryCount()-1), bForward, sal_True );
        }

        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            // Try match full from current position
            nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, nStart, bForward, sal_False );
        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            //  Match full, but from start
            nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, bForward ? 0 : (mpImplLB->GetEntryList()->GetEntryCount()-1), bForward, sal_False );

        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            OUString aText = mpImplLB->GetEntryList()->GetEntryText( nPos );
            Selection aSelection( aText.getLength(), aStartText.getLength() );
            pEdit->SetText( aText, aSelection );
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ComboBox, ImplSelectHdl)
{
    sal_Bool bPopup = IsInDropDown();
    bool bCallSelect = false;
    if ( mpImplLB->IsSelectionChanged() || bPopup )
    {
        OUString aText;
        if ( IsMultiSelectionEnabled() )
        {
            aText = mpSubEdit->GetText();

            // remove all entries to which there is an entry, but which is not selected
            sal_Int32 nIndex = 0;
            while ( nIndex >= 0 )
            {
                sal_Int32  nPrevIndex = nIndex;
                OUString   aToken = aText.getToken( 0, mcMultiSep, nIndex );
                sal_Int32  nTokenLen = aToken.getLength();
                aToken = comphelper::string::strip(aToken, ' ');
                sal_uInt16      nP = mpImplLB->GetEntryList()->FindEntry( aToken );
                if ( (nP != LISTBOX_ENTRY_NOTFOUND) && (!mpImplLB->GetEntryList()->IsEntryPosSelected( nP )) )
                {
                    aText = aText.replaceAt( nPrevIndex, nTokenLen, "" );
                    nIndex = sal::static_int_cast<xub_StrLen>(nIndex - nTokenLen);
                    sal_Int32 nSepCount=0;
                    if ( (nPrevIndex+nSepCount < aText.getLength()) && (aText[nPrevIndex+nSepCount] == mcMultiSep) )
                    {
                        nIndex--;
                        ++nSepCount;
                    }
                    aText = aText.replaceAt( nPrevIndex, nSepCount, "" );
                }
                aText = comphelper::string::strip(aText, ' ');
            }

            // attach missing entries
            ::std::set< sal_uInt16 > aSelInText;
            lcl_GetSelectedEntries( aSelInText, aText, mcMultiSep, mpImplLB->GetEntryList() );
            sal_uInt16 nSelectedEntries = mpImplLB->GetEntryList()->GetSelectEntryCount();
            for ( sal_uInt16 n = 0; n < nSelectedEntries; n++ )
            {
                sal_uInt16 nP = mpImplLB->GetEntryList()->GetSelectEntryPos( n );
                if ( !aSelInText.count( nP ) )
                {
                    if ( !aText.isEmpty() && (aText[ aText.getLength()-1 ] != mcMultiSep) )
                        aText += OUString(mcMultiSep);
                    if ( !aText.isEmpty() )
                        aText += " ";   // slightly loosen
                    aText += mpImplLB->GetEntryList()->GetEntryText( nP );
                    aText += OUString(mcMultiSep);
                }
            }
            aText = comphelper::string::stripEnd( aText, mcMultiSep );
        }
        else
        {
            aText = mpImplLB->GetEntryList()->GetSelectEntry( 0 );
        }

        mpSubEdit->SetText( aText );

        Selection aNewSelection( 0, aText.getLength() );
        if ( IsMultiSelectionEnabled() )
            aNewSelection.Min() = aText.getLength();
        mpSubEdit->SetSelection( aNewSelection );

        bCallSelect = true;
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
        mbSyntheticModify = sal_True;
        Modify();
        mbSyntheticModify = sal_False;
        Select();
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ComboBox, ImplCancelHdl)
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
        sal_uInt16 nChanged = (sal_uInt16)(sal_uLong)n;
        if ( !mpSubEdit->IsReadOnly() && mpImplLB->GetEntryList()->IsEntryPosSelected( nChanged ) )
            mpSubEdit->SetText( mpImplLB->GetEntryList()->GetEntryText( nChanged ) );
    }
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ComboBox, ImplDoubleClickHdl)
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
                mpImplLB->SelectEntry( 0 , sal_True );
            ImplCallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
            mpBtn->SetPressed( sal_True );
            SetSelection( Selection( 0, SELECTION_MAX ) );
            mpFloatWin->StartFloat( sal_True );
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

void ComboBox::EnableAutoSize( bool bAuto )
{
    mbDDAutoSize = bAuto;
    if ( mpFloatWin )
    {
        if ( bAuto && !mpFloatWin->GetDropDownLineCount() )
        {
            // Adapt to GetListBoxMaximumLineCount here; was on fixed number of five before
            AdaptDropDownLineCountToMaximum();
        }
        else if ( !bAuto )
        {
            mpFloatWin->SetDropDownLineCount( 0 );
        }
    }
}

// -----------------------------------------------------------------------

void ComboBox::EnableDDAutoWidth( sal_Bool b )
{
    if ( mpFloatWin )
        mpFloatWin->SetAutoWidth( b );
}

 // -----------------------------------------------------------------------

void ComboBox::SetDropDownLineCount( sal_uInt16 nLines )
{
    if ( mpFloatWin )
        mpFloatWin->SetDropDownLineCount( nLines );
}

// -----------------------------------------------------------------------

void ComboBox::AdaptDropDownLineCountToMaximum()
{
    // adapt to maximum allowed number
    SetDropDownLineCount(GetSettings().GetStyleSettings().GetListBoxMaximumLineCount());
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetDropDownLineCount() const
{
    sal_uInt16 nLines = 0;
    if ( mpFloatWin )
        nLines = mpFloatWin->GetDropDownLineCount();
    return nLines;
}

// -----------------------------------------------------------------------

void ComboBox::setPosSizePixel( long nX, long nY, long nWidth, long nHeight,
                                sal_uInt16 nFlags )
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

    Edit::setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

// -----------------------------------------------------------------------

void ComboBox::Resize()
{
    Control::Resize();

    Size aOutSz = GetOutputSizePixel();
    if( IsDropDownBox() )
    {
        ComboBoxBounds aBounds(calcComboBoxDropDownComponentBounds(aOutSz,
            GetWindow(WINDOW_BORDER)->GetOutputSizePixel()));
        mpSubEdit->SetPosSizePixel(aBounds.aSubEditPos, aBounds.aSubEditSize);
        mpBtn->SetPosSizePixel(aBounds.aButtonPos, aBounds.aButtonSize);
    }
    else
    {
        mpSubEdit->SetSizePixel( Size( aOutSz.Width(), mnDDHeight ) );
        mpImplLB->setPosSizePixel( 0, mnDDHeight, aOutSz.Width(), aOutSz.Height() - mnDDHeight );
        if ( !GetText().isEmpty() )
            ImplUpdateFloatSelection();
    }

    // adjust the size of the FloatingWindow even when invisible
    // as KEY_PGUP/DOWN is being processed...
    if ( mpFloatWin )
        mpFloatWin->SetSizePixel( mpFloatWin->CalcFloatSize() );
}

// -----------------------------------------------------------------------

void ComboBox::FillLayoutData() const
{
    mpControlData->mpLayoutData = new vcl::ControlLayoutData();
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
        mpImplLB->GetMainWindow()->EnableSort( ( GetStyle() & WB_SORT ) ? sal_True : sal_False );
    }
    else if( nType == STATE_CHANGE_MIRRORING )
    {
        if( mpBtn )
        {
            mpBtn->EnableRTL( IsRTLEnabled() );
            ImplInitDropDownButton( mpBtn );
        }
        mpSubEdit->StateChanged( STATE_CHANGE_MIRRORING );
        mpImplLB->EnableRTL( IsRTLEnabled() );
        Resize();
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
        mpImplLB->Resize(); // not called by ComboBox::Resize() if ImplLB is unchanged

        SetBackground();    // due to a hack in Window::UpdateSettings the background must be reset
                            // otherwise it will overpaint NWF drawn comboboxes
    }
}

// -----------------------------------------------------------------------

long ComboBox::PreNotify( NotifyEvent& rNEvt )
{

    return Edit::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long ComboBox::Notify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if( ( rNEvt.GetType() == EVENT_KEYINPUT ) && ( rNEvt.GetWindow() == mpSubEdit )
            && !IsReadOnly() )
    {
        KeyEvent aKeyEvt = *rNEvt.GetKeyEvent();
        sal_uInt16   nKeyCode = aKeyEvt.GetKeyCode().GetCode();
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
                    ImplCallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
                    mpBtn->SetPressed( sal_True );
                    if ( mpImplLB->GetEntryList()->GetMRUCount() )
                        mpImplLB->SelectEntry( 0 , sal_True );
                    SetSelection( Selection( 0, SELECTION_MAX ) );
                    mpFloatWin->StartFloat( sal_False );
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
        else if ( mpFloatWin->IsInPopupMode() && !HasChildPathFocus( sal_True ) )
            mpFloatWin->EndPopupMode();
    }
    else if( (rNEvt.GetType() == EVENT_COMMAND) &&
             (rNEvt.GetCommandEvent()->GetCommand() == COMMAND_WHEEL) &&
             (rNEvt.GetWindow() == mpSubEdit) )
    {
        sal_uInt16 nWheelBehavior( GetSettings().GetMouseSettings().GetWheelBehavior() );
        if  (   ( nWheelBehavior == MOUSE_WHEEL_ALWAYS )
            ||  (   ( nWheelBehavior == MOUSE_WHEEL_FOCUS_ONLY )
                &&  HasChildPathFocus()
                )
            )
        {
            nDone = mpImplLB->HandleWheelAsCursorTravel( *rNEvt.GetCommandEvent() );
        }
        else
        {
            nDone = 0;  // don't eat this event, let the default handling happen (i.e. scroll the context)
        }
    }
    else if( ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN ) && ( rNEvt.GetWindow() == mpImplLB->GetMainWindow() ) )
    {
        mpSubEdit->GrabFocus();
    }

    return nDone ? nDone : Edit::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void ComboBox::SetText( const OUString& rStr )
{
    ImplCallEventListeners( VCLEVENT_COMBOBOX_SETTEXT );

    Edit::SetText( rStr );
    ImplUpdateFloatSelection();
}

// -----------------------------------------------------------------------

void ComboBox::SetText( const OUString& rStr, const Selection& rNewSelection )
{
    ImplCallEventListeners( VCLEVENT_COMBOBOX_SETTEXT );

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
    // move text in the ListBox into the visible region
    mpImplLB->SetCallSelectionChangedHdl( sal_False );
    if ( !IsMultiSelectionEnabled() )
    {
        OUString   aSearchStr( mpSubEdit->GetText() );
        sal_uInt16 nSelect = LISTBOX_ENTRY_NOTFOUND;
        sal_Bool   bSelect = sal_True;

        if ( mpImplLB->GetCurrentPos() != LISTBOX_ENTRY_NOTFOUND )
        {
            OUString aCurrent = mpImplLB->GetEntryList()->GetEntryText( mpImplLB->GetCurrentPos() );
            if ( aCurrent == aSearchStr )
                nSelect = mpImplLB->GetCurrentPos();
        }

        if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
            nSelect = mpImplLB->GetEntryList()->FindEntry( aSearchStr );
        if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
        {
            nSelect = mpImplLB->GetEntryList()->FindMatchingEntry( aSearchStr );
            bSelect = sal_False;
        }

        if( nSelect != LISTBOX_ENTRY_NOTFOUND )
        {
            if ( !mpImplLB->IsVisible( nSelect ) )
                mpImplLB->ShowProminentEntry( nSelect );
            mpImplLB->SelectEntry( nSelect, bSelect );
        }
        else
        {
            nSelect = mpImplLB->GetEntryList()->GetSelectEntryPos( 0 );
            if( nSelect != LISTBOX_ENTRY_NOTFOUND )
                mpImplLB->SelectEntry( nSelect, sal_False );
            mpImplLB->ResetCurrentPos();
        }
    }
    else
    {
        ::std::set< sal_uInt16 > aSelInText;
        lcl_GetSelectedEntries( aSelInText, mpSubEdit->GetText(), mcMultiSep, mpImplLB->GetEntryList() );
        for ( sal_uInt16 n = 0; n < mpImplLB->GetEntryList()->GetEntryCount(); n++ )
            mpImplLB->SelectEntry( n, aSelInText.count( n ) );
    }
    mpImplLB->SetCallSelectionChangedHdl( sal_True );
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::InsertEntry(const OUString& rStr, sal_uInt16 const nPos)
{
    sal_uInt16 nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rStr );
    nRealPos = sal::static_int_cast<sal_uInt16>(nRealPos - mpImplLB->GetEntryList()->GetMRUCount());
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMADDED, (void*) sal_IntPtr(nRealPos) );
    return nRealPos;
}

// -----------------------------------------------------------------------

void ComboBox::InsertEntryWithImage(
        const OUString& rStr, const Image& rImage, sal_uInt16 const nPos)
{
    sal_uInt16 nRealPos = mpImplLB->InsertEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), rStr, rImage );
    nRealPos = sal::static_int_cast<sal_uInt16>(nRealPos - mpImplLB->GetEntryList()->GetMRUCount());
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMADDED, (void*) sal_IntPtr(nRealPos) );
}

// -----------------------------------------------------------------------

void ComboBox::RemoveEntry( const OUString& rStr )
{
    RemoveEntryAt(GetEntryPos(rStr));
}

// -----------------------------------------------------------------------

void ComboBox::RemoveEntryAt(sal_uInt16 const nPos)
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

Image ComboBox::GetEntryImage( sal_uInt16 nPos ) const
{
    if ( mpImplLB->GetEntryList()->HasEntryImage( nPos ) )
        return mpImplLB->GetEntryList()->GetEntryImage( nPos );
    return Image();
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetEntryPos( const OUString& rStr ) const
{
    sal_uInt16 nPos = mpImplLB->GetEntryList()->FindEntry( rStr );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos = sal::static_int_cast<sal_uInt16>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
    return nPos;
}

// -----------------------------------------------------------------------

OUString ComboBox::GetEntry( sal_uInt16 nPos ) const
{
    return mpImplLB->GetEntryList()->GetEntryText( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetEntryCount() const
{
    return mpImplLB->GetEntryList()->GetEntryCount() - mpImplLB->GetEntryList()->GetMRUCount();
}

// -----------------------------------------------------------------------

sal_Bool ComboBox::IsTravelSelect() const
{
    return mpImplLB->IsTravelSelect();
}

// -----------------------------------------------------------------------

sal_Bool ComboBox::IsInDropDown() const
{
    return mpFloatWin && mpFloatWin->IsInPopupMode();
}

// -----------------------------------------------------------------------

void ComboBox::EnableMultiSelection( sal_Bool bMulti )
{
    mpImplLB->EnableMultiSelection( bMulti, sal_False );
    mpImplLB->SetMultiSelectionSimpleMode( sal_True );
}

// -----------------------------------------------------------------------

sal_Bool ComboBox::IsMultiSelectionEnabled() const
{
    return mpImplLB->IsMultiSelectionEnabled();
}

// -----------------------------------------------------------------------

long ComboBox::CalcWindowSizePixel( sal_uInt16 nLines ) const
{
    return mpImplLB->GetEntryHeight() * nLines;
}

// -----------------------------------------------------------------------

Size ComboBox::GetOptimalSize() const
{
    return CalcMinimumSize();
}

// -----------------------------------------------------------------------

long ComboBox::getMaxWidthScrollBarAndDownButton() const
{
    long nButtonDownWidth = 0;

    Window *pBorder = GetWindow( WINDOW_BORDER );
    ImplControlValue aControlValue;
    Point aPoint;
    Rectangle aContent, aBound;

    // use the full extent of the control
    Rectangle aArea( aPoint, pBorder->GetOutputSizePixel() );

    if ( GetNativeControlRegion(CTRL_COMBOBOX, PART_BUTTON_DOWN,
        aArea, 0, aControlValue, OUString(), aBound, aContent) )
    {
        nButtonDownWidth = aContent.getWidth();
    }

    long nScrollBarWidth = GetSettings().GetStyleSettings().GetScrollBarSize();

    return std::max(nScrollBarWidth, nButtonDownWidth);
}

Size ComboBox::CalcMinimumSize() const
{
    Size aSz;

    if (!mpImplLB)
        return aSz;

    if (!IsDropDownBox())
    {
        aSz = mpImplLB->CalcSize( mpImplLB->GetEntryList()->GetEntryCount() );
        aSz.Height() += mnDDHeight;
    }
    else
    {
        aSz.Height() = Edit::CalcMinimumSizeForText(GetText()).Height();
        aSz.Width() = mpImplLB->GetMaxEntryWidth();
    }

    if (m_nMaxWidthChars != -1)
    {
        long nMaxWidth = m_nMaxWidthChars * approximate_char_width();
        aSz.Width() = std::min(aSz.Width(), nMaxWidth);
    }

    if (IsDropDownBox())
        aSz.Width() += getMaxWidthScrollBarAndDownButton();

    ComboBoxBounds aBounds(calcComboBoxDropDownComponentBounds(
        Size(0xFFFF, 0xFFFF), Size(0xFFFF, 0xFFFF)));
    aSz.Width() += aBounds.aSubEditPos.X()*2;

    aSz.Width() += ImplGetExtraOffset() * 2;

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

Size ComboBox::CalcSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const
{
    // show ScrollBars where appropriate
    Size aMinSz = CalcMinimumSize();
    Size aSz;

    // height
    if ( nLines )
    {
        if ( !IsDropDownBox() )
            aSz.Height() = mpImplLB->CalcSize( nLines ).Height() + mnDDHeight;
        else
            aSz.Height() = mnDDHeight;
    }
    else
        aSz.Height() = aMinSz.Height();

    // width
    if ( nColumns )
        aSz.Width() = nColumns * approximate_char_width();
    else
        aSz.Width() = aMinSz.Width();

    if ( IsDropDownBox() )
        aSz.Width() += getMaxWidthScrollBarAndDownButton();

    if ( !IsDropDownBox() )
    {
        if ( aSz.Width() < aMinSz.Width() )
            aSz.Height() += GetSettings().GetStyleSettings().GetScrollBarSize();
        if ( aSz.Height() < aMinSz.Height() )
            aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
    }

    aSz.Width() += ImplGetExtraOffset() * 2;

    aSz = CalcWindowSize( aSz );
    return aSz;
}

// -----------------------------------------------------------------------

void ComboBox::GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const
{
    long nCharWidth = GetTextWidth(OUString(static_cast<sal_Unicode>('x')));
    if ( !IsDropDownBox() )
    {
        Size aOutSz = mpImplLB->GetMainWindow()->GetOutputSizePixel();
        rnCols = (sal_uInt16)(aOutSz.Width()/nCharWidth);
        rnLines = (sal_uInt16)(aOutSz.Height()/mpImplLB->GetEntryHeight());
    }
    else
    {
        Size aOutSz = mpSubEdit->GetOutputSizePixel();
        rnCols = (sal_uInt16)(aOutSz.Width()/nCharWidth);
        rnLines = 1;
    }
}

// -----------------------------------------------------------------------

void ComboBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    mpImplLB->GetMainWindow()->ImplInitSettings( sal_True, sal_True, sal_True );

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
    sal_Bool bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
    sal_Bool bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
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

    // contents
    if ( !IsDropDownBox() )
    {
        long        nOnePixel = GetDrawPixel( pDev, 1 );
        long        nTextHeight = pDev->GetTextHeight();
        long        nEditHeight = nTextHeight + 6*nOnePixel;
        sal_uInt16      nTextStyle = TEXT_DRAW_VCENTER;

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
        sal_uInt16 nLines = (sal_uInt16) ( (aSize.Height()-nEditHeight) / nTextHeight );
        if ( !nLines )
            nLines = 1;
        sal_uInt16 nTEntry = IsReallyVisible() ? mpImplLB->GetTopEntry() : 0;

        Rectangle aTextRect( aPos, aSize );

        aTextRect.Left() += 3*nOnePixel;
        aTextRect.Right() -= 3*nOnePixel;
        aTextRect.Top() += nEditHeight + nOnePixel;
        aTextRect.Bottom() = aTextRect.Top() + nTextHeight;

        // the drawing starts here
        for ( sal_uInt16 n = 0; n < nLines; n++ )
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

void ComboBox::EnableUserDraw( sal_Bool bUserDraw )
{
    mpImplLB->GetMainWindow()->EnableUserDraw( bUserDraw );
}

// -----------------------------------------------------------------------

void ComboBox::DrawEntry( const UserDrawEvent& rEvt, sal_Bool bDrawImage, sal_Bool bDrawText, sal_Bool bDrawTextAtImagePos )
{
    DBG_ASSERT( rEvt.GetDevice() == mpImplLB->GetMainWindow(), "DrawEntry?!" );
    mpImplLB->GetMainWindow()->DrawEntry( rEvt.GetItemId(), bDrawImage, bDrawText, bDrawTextAtImagePos );
}

// -----------------------------------------------------------------------

void ComboBox::SetSeparatorPos( sal_uInt16 n )
{
    mpImplLB->SetSeparatorPos( n );
}

// -----------------------------------------------------------------------

void ComboBox::SetMRUEntries( const OUString& rEntries, sal_Unicode cSep )
{
    mpImplLB->SetMRUEntries( rEntries, cSep );
}

// -----------------------------------------------------------------------

OUString ComboBox::GetMRUEntries( sal_Unicode cSep ) const
{
    return mpImplLB->GetMRUEntries( cSep );
}

// -----------------------------------------------------------------------

void ComboBox::SetMaxMRUCount( sal_uInt16 n )
{
    mpImplLB->SetMaxMRUCount( n );
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetMaxMRUCount() const
{
    return mpImplLB->GetMaxMRUCount();
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetDisplayLineCount() const
{
    return mpImplLB->GetDisplayLineCount();
}

// -----------------------------------------------------------------------

void ComboBox::SetEntryData( sal_uInt16 nPos, void* pNewData )
{
    mpImplLB->SetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount(), pNewData );
}

// -----------------------------------------------------------------------

void* ComboBox::GetEntryData( sal_uInt16 nPos ) const
{
    return mpImplLB->GetEntryList()->GetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

// -----------------------------------------------------------------------

sal_uInt16 ComboBox::GetTopEntry() const
{
    sal_uInt16 nPos = GetEntryCount() ? mpImplLB->GetTopEntry() : LISTBOX_ENTRY_NOTFOUND;
    if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
        nPos = 0;
    return nPos;
}

// -----------------------------------------------------------------------

void ComboBox::SetProminentEntryType( ProminentEntry eType )
{
    mpImplLB->SetProminentEntryType( eType );
}

// -----------------------------------------------------------------------

Rectangle ComboBox::GetDropDownPosSizePixel() const
{
    return mpFloatWin ? mpFloatWin->GetWindowExtentsRelative( const_cast<ComboBox*>(this) ) : Rectangle();
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
sal_uInt16 ComboBox::GetSelectEntryCount() const
{
    return mpImplLB->GetEntryList()->GetSelectEntryCount();
}
// -----------------------------------------------------------------------------
sal_uInt16 ComboBox::GetSelectEntryPos( sal_uInt16 nIndex ) const
{
    sal_uInt16 nPos = mpImplLB->GetEntryList()->GetSelectEntryPos( nIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
            nPos = mpImplLB->GetEntryList()->FindEntry( mpImplLB->GetEntryList()->GetEntryText( nPos ) );
        nPos = sal::static_int_cast<sal_uInt16>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
    }
    return nPos;
}
// -----------------------------------------------------------------------------
sal_Bool ComboBox::IsEntryPosSelected( sal_uInt16 nPos ) const
{
    return mpImplLB->GetEntryList()->IsEntryPosSelected( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}
// -----------------------------------------------------------------------------
void ComboBox::SelectEntryPos( sal_uInt16 nPos, sal_Bool bSelect)
{
    if ( nPos < mpImplLB->GetEntryList()->GetEntryCount() )
        mpImplLB->SelectEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), bSelect );
}
// -----------------------------------------------------------------------------
void ComboBox::SetNoSelection()
{
    mpImplLB->SetNoSelection();
    mpSubEdit->SetText( OUString() );
}
// -----------------------------------------------------------------------------
Rectangle ComboBox::GetBoundingRectangle( sal_uInt16 nItem ) const
{
    Rectangle aRect = mpImplLB->GetMainWindow()->GetBoundingRectangle( nItem );
    Rectangle aOffset = mpImplLB->GetMainWindow()->GetWindowExtentsRelative( (Window*)this );
    aRect.Move( aOffset.TopLeft().X(), aOffset.TopLeft().Y() );
    return aRect;
}
// -----------------------------------------------------------------------------

void ComboBox::SetBorderStyle( sal_uInt16 nBorderStyle )
{
    Window::SetBorderStyle( nBorderStyle );
    if ( !IsDropDownBox() )
    {
        mpSubEdit->SetBorderStyle( nBorderStyle );
        mpImplLB->SetBorderStyle( nBorderStyle );
    }
}
// -----------------------------------------------------------------------------

long ComboBox::GetIndexForPoint( const Point& rPoint, sal_uInt16& rPos ) const
{
    if( !HasLayoutData() )
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
        sal_uInt16 nEntry = pMain->GetEntryPosForPoint( aConvPoint );
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

ComboBox::ComboBoxBounds ComboBox::calcComboBoxDropDownComponentBounds(const Size &rOutSz,
    const Size &rBorderOutSz) const
{
    ComboBoxBounds aBounds;

    long    nTop = 0;
    long    nBottom = rOutSz.Height();

    Window *pBorder = GetWindow( WINDOW_BORDER );
    ImplControlValue aControlValue;
    Point aPoint;
    Rectangle aContent, aBound;

    // use the full extent of the control
    Rectangle aArea( aPoint, rBorderOutSz );

    if ( GetNativeControlRegion(CTRL_COMBOBOX, PART_BUTTON_DOWN,
            aArea, 0, aControlValue, OUString(), aBound, aContent) )
    {
        // convert back from border space to local coordinates
        aPoint = pBorder->ScreenToOutputPixel( OutputToScreenPixel( aPoint ) );
        aContent.Move(-aPoint.X(), -aPoint.Y());

        aBounds.aButtonPos = Point(aContent.Left(), nTop);
        aBounds.aButtonSize = Size(aContent.getWidth(), (nBottom-nTop));

        // adjust the size of the edit field
        if ( GetNativeControlRegion(CTRL_COMBOBOX, PART_SUB_EDIT,
                    aArea, 0, aControlValue, OUString(), aBound, aContent) )
        {
            // convert back from border space to local coordinates
            aContent.Move(-aPoint.X(), -aPoint.Y());

            // use the themes drop down size
            aBounds.aSubEditPos = aContent.TopLeft();
            aBounds.aSubEditSize = aContent.GetSize();
        }
        else
        {
            // use the themes drop down size for the button
            aBounds.aSubEditSize = Size(rOutSz.Width() - aContent.getWidth(), rOutSz.Height());
        }
    }
    else
    {
        long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
        nSBWidth = CalcZoom( nSBWidth );
        aBounds.aSubEditSize = Size(rOutSz.Width() - nSBWidth, rOutSz.Height());
        aBounds.aButtonPos = Point(rOutSz.Width() - nSBWidth, nTop);
        aBounds.aButtonSize = Size(nSBWidth, (nBottom-nTop));
    }
    return aBounds;
}

void ComboBox::setMaxWidthChars(sal_Int32 nWidth)
{
    if (nWidth != m_nMaxWidthChars)
    {
        m_nMaxWidthChars = nWidth;
        queue_resize();
    }
}

bool ComboBox::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "max-width-chars")
        setMaxWidthChars(rValue.toInt32());
    else
        return Control::set_property(rKey, rValue);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
