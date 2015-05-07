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
#include <vcl/settings.hxx>

#include <svdata.hxx>
#include <ilstbox.hxx>
#include <controldata.hxx>

static void lcl_GetSelectedEntries( ::std::set< sal_Int32 >& rSelectedPos, const OUString& rText, sal_Unicode cTokenSep, const ImplEntryList* pEntryList )
{
    for (sal_Int32 n = comphelper::string::getTokenCount(rText, cTokenSep); n;)
    {
        OUString aToken = rText.getToken( --n, cTokenSep );
        aToken = comphelper::string::strip(aToken, ' ');
        sal_Int32 nPos = pEntryList->FindEntry( aToken );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            rSelectedPos.insert( nPos );
    }
}

ComboBox::ComboBox( vcl::Window* pParent, WinBits nStyle ) :
    Edit( WINDOW_COMBOBOX )
{
    ImplInitComboBoxData();
    ImplInit( pParent, nStyle );
    SetWidthInChars(-1);
}

ComboBox::ComboBox( vcl::Window* pParent, const ResId& rResId ) :
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

ComboBox::~ComboBox()
{
    disposeOnce();
}

void ComboBox::dispose()
{
    mpSubEdit.disposeAndClear();

    VclPtr< ImplListBox > pImplLB = mpImplLB;
    mpImplLB.clear();
    pImplLB.disposeAndClear();

    mpFloatWin.disposeAndClear();
    mpBtn.disposeAndClear();
    Edit::dispose();
}

void ComboBox::ImplInitComboBoxData()
{
    mpSubEdit.disposeAndClear();
    mpBtn               = NULL;
    mpImplLB            = NULL;
    mpFloatWin          = NULL;

    mnDDHeight          = 0;
    mbDDAutoSize        = true;
    mbSyntheticModify   = false;
    mbMatchCase         = false;
    mcMultiSep          = ';';
    m_nMaxWidthChars    = -1;
}

void ComboBox::ImplCalcEditHeight()
{
    sal_Int32 nLeft, nTop, nRight, nBottom;
    GetBorder( nLeft, nTop, nRight, nBottom );
    mnDDHeight = (sal_uInt16)(mpSubEdit->GetTextHeight() + nTop + nBottom + 4);
    if ( !IsDropDownBox() )
        mnDDHeight += 4;

    Rectangle aCtrlRegion( Point( 0, 0 ), Size( 10, 10 ) );
    Rectangle aBoundRegion, aContentRegion;
    EditBoxValue aControlValue(GetTextHeight());
    ControlType aType = IsDropDownBox() ? CTRL_COMBOBOX : CTRL_EDITBOX;
    if( GetNativeControlRegion( aType, PART_ENTIRE_CONTROL,
                                aCtrlRegion,
                                ControlState::ENABLED,
                                aControlValue, OUString(),
                                aBoundRegion, aContentRegion ) )
    {
        const long nNCHeight = aBoundRegion.GetHeight();
        if( mnDDHeight < nNCHeight )
            mnDDHeight = sal::static_int_cast<sal_uInt16>( nNCHeight );
    }
}

void ComboBox::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    ImplInitStyle( nStyle );

    bool bNoBorder = ( nStyle & WB_NOBORDER ) != 0;
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
        mpFloatWin = VclPtr<ImplListBoxFloatingWindow>::Create( this );
        mpFloatWin->SetAutoWidth( true );
        mpFloatWin->SetPopupModeEndHdl( LINK( this, ComboBox, ImplPopupModeEndHdl ) );

        mpBtn = VclPtr<ImplBtn>::Create( this, WB_NOLIGHTBORDER | WB_RECTSTYLE );
        ImplInitDropDownButton( mpBtn );
        mpBtn->buttonDownSignal.connect( boost::bind( &ComboBox::ImplClickButtonHandler, this, _1 ));
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

    mpSubEdit.set( VclPtr<Edit>::Create( this, nEditStyle ) );
    mpSubEdit->EnableRTL( false );
    SetSubEdit( mpSubEdit );
    mpSubEdit->SetPosPixel( Point() );
    EnableAutocomplete( true );
    mpSubEdit->Show();

    vcl::Window* pLBParent = this;
    if ( mpFloatWin )
        pLBParent = mpFloatWin;
    mpImplLB = VclPtr<ImplListBox>::Create( pLBParent, nListStyle|WB_SIMPLEMODE|WB_AUTOHSCROLL );
    mpImplLB->SetPosPixel( Point() );
    mpImplLB->SetSelectHdl( LINK( this, ComboBox, ImplSelectHdl ) );
    mpImplLB->SetCancelHdl( LINK( this, ComboBox, ImplCancelHdl ) );
    mpImplLB->SetDoubleClickHdl( LINK( this, ComboBox, ImplDoubleClickHdl ) );
    mpImplLB->userDrawSignal.connect( boost::bind( &ComboBox::ImplUserDrawHandler, this, _1 ) );
    mpImplLB->SetSelectionChangedHdl( LINK( this, ComboBox, ImplSelectionChangedHdl ) );
    mpImplLB->SetListItemSelectHdl( LINK( this, ComboBox, ImplListItemSelectHdl ) );
    mpImplLB->Show();

    if ( mpFloatWin )
        mpFloatWin->SetImplListBox( mpImplLB );
    else
        mpImplLB->GetMainWindow()->AllowGrabFocus( true );

    ImplCalcEditHeight();

    SetCompoundControl( true );
}

WinBits ComboBox::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

void ComboBox::ImplLoadRes( const ResId& rResId )
{
    Edit::ImplLoadRes( rResId );

    sal_Int32 nNumber = ReadLongRes();

    if( nNumber )
    {
        for( sal_Int32 i = 0; i < nNumber; i++ )
        {
            InsertEntry( ReadStringRes(), LISTBOX_APPEND );
        }
    }
}

void ComboBox::EnableAutocomplete( bool bEnable, bool bMatchCase )
{
    mbMatchCase = bMatchCase;

    if ( bEnable )
    {
        if( !mAutocompleteConnection.connected())
            mAutocompleteConnection = mpSubEdit->autocompleteSignal.connect(
                boost::bind( &ComboBox::ImplAutocompleteHandler, this, _1 ) );
    }
    else
        mAutocompleteConnection.disconnect();
}

bool ComboBox::IsAutocompleteEnabled() const
{
    return mAutocompleteConnection.connected();
}

void ComboBox::ImplClickButtonHandler( ImplBtn* )
{
    CallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
    mpSubEdit->GrabFocus();
    if ( !mpImplLB->GetEntryList()->GetMRUCount() )
        ImplUpdateFloatSelection();
    else
        mpImplLB->SelectEntry( 0 , true );
    mpBtn->SetPressed( true );
    SetSelection( Selection( 0, SELECTION_MAX ) );
    mpFloatWin->StartFloat( true );
    CallEventListeners( VCLEVENT_DROPDOWN_OPEN );

    ImplClearLayoutData();
    if( mpImplLB )
        mpImplLB->GetMainWindow()->ImplClearLayoutData();
}

IMPL_LINK_NOARG(ComboBox, ImplPopupModeEndHdl)
{
    if( mpFloatWin->IsPopupModeCanceled() )
    {
        if ( !mpImplLB->GetEntryList()->IsEntryPosSelected( mpFloatWin->GetPopupModeStartSaveSelection() ) )
        {
            mpImplLB->SelectEntry( mpFloatWin->GetPopupModeStartSaveSelection(), true );
            bool bTravelSelect = mpImplLB->IsTravelSelect();
            mpImplLB->SetTravelSelect( true );
            Select();
            mpImplLB->SetTravelSelect( bTravelSelect );
        }
    }

    ImplClearLayoutData();
    if( mpImplLB )
        mpImplLB->GetMainWindow()->ImplClearLayoutData();

    mpBtn->SetPressed( false );
    CallEventListeners( VCLEVENT_DROPDOWN_CLOSE );
    return 0;
}

void ComboBox::ImplAutocompleteHandler( Edit* pEdit )
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
        sal_Int32   nStart = mpImplLB->GetCurrentPos();

        if ( nStart == LISTBOX_ENTRY_NOTFOUND )
            nStart = 0;

        bool bForward = true;
        if ( eAction == AUTOCOMPLETE_TABFORWARD )
            nStart++;
        else if ( eAction == AUTOCOMPLETE_TABBACKWARD )
        {
            bForward = false;
            if (nStart)
                nStart = nStart - 1;
            else if (mpImplLB->GetEntryList()->GetEntryCount())
                nStart = mpImplLB->GetEntryList()->GetEntryCount()-1;
        }

        sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;
        if( ! mbMatchCase )
        {
            // Try match case insensitive from current position
            nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, nStart, bForward, true );
            if ( nPos == LISTBOX_ENTRY_NOTFOUND )
                // Try match case insensitive, but from start
                nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, bForward ? 0 : (mpImplLB->GetEntryList()->GetEntryCount()-1), bForward, true );
        }

        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            // Try match full from current position
            nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, nStart, bForward, false );
        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            //  Match full, but from start
            nPos = mpImplLB->GetEntryList()->FindMatchingEntry( aStartText, bForward ? 0 : (mpImplLB->GetEntryList()->GetEntryCount()-1), bForward, false );

        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            OUString aText = mpImplLB->GetEntryList()->GetEntryText( nPos );
            Selection aSelection( aText.getLength(), aStartText.getLength() );
            pEdit->SetText( aText, aSelection );
        }
    }
}

IMPL_LINK_NOARG(ComboBox, ImplSelectHdl)
{
    bool bPopup = IsInDropDown();
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
                sal_Int32      nP = mpImplLB->GetEntryList()->FindEntry( aToken );
                if ( (nP != LISTBOX_ENTRY_NOTFOUND) && (!mpImplLB->GetEntryList()->IsEntryPosSelected( nP )) )
                {
                    aText = aText.replaceAt( nPrevIndex, nTokenLen, "" );
                    nIndex = nIndex - nTokenLen;
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
            ::std::set< sal_Int32 > aSelInText;
            lcl_GetSelectedEntries( aSelInText, aText, mcMultiSep, mpImplLB->GetEntryList() );
            sal_Int32 nSelectedEntries = mpImplLB->GetEntryList()->GetSelectEntryCount();
            for ( sal_Int32 n = 0; n < nSelectedEntries; n++ )
            {
                sal_Int32 nP = mpImplLB->GetEntryList()->GetSelectEntryPos( n );
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
        mbSyntheticModify = true;
        Modify();
        mbSyntheticModify = false;
        Select();
    }

    return 0;
}

IMPL_LINK_NOARG( ComboBox, ImplListItemSelectHdl )
{
    CallEventListeners( VCLEVENT_DROPDOWN_SELECT );
    return 1;
}

IMPL_LINK_NOARG(ComboBox, ImplCancelHdl)
{
    if( IsInDropDown() )
        mpFloatWin->EndPopupMode();

    return 1;
}

IMPL_LINK( ComboBox, ImplSelectionChangedHdl, void*, n )
{
    if ( !mpImplLB->IsTrackingSelect() )
    {
        sal_Int32 nChanged = (sal_Int32)reinterpret_cast<sal_uLong>(n);
        if ( !mpSubEdit->IsReadOnly() && mpImplLB->GetEntryList()->IsEntryPosSelected( nChanged ) )
            mpSubEdit->SetText( mpImplLB->GetEntryList()->GetEntryText( nChanged ) );
    }
    return 1;
}

IMPL_LINK_NOARG(ComboBox, ImplDoubleClickHdl)
{
    DoubleClick();
    return 0;
}

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
                mpImplLB->SelectEntry( 0 , true );
            CallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
            mpBtn->SetPressed( true );
            SetSelection( Selection( 0, SELECTION_MAX ) );
            mpFloatWin->StartFloat( true );
            CallEventListeners( VCLEVENT_DROPDOWN_OPEN );
        }
    }
}

void ComboBox::Select()
{
    ImplCallEventListenersAndHandler( VCLEVENT_COMBOBOX_SELECT, maSelectHdl, this );
}

void ComboBox::DoubleClick()
{
    ImplCallEventListenersAndHandler( VCLEVENT_COMBOBOX_DOUBLECLICK, maDoubleClickHdl, this );
}

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

void ComboBox::EnableDDAutoWidth( bool b )
{
    if ( mpFloatWin )
        mpFloatWin->SetAutoWidth( b );
}

void ComboBox::SetDropDownLineCount( sal_uInt16 nLines )
{
    if ( mpFloatWin )
        mpFloatWin->SetDropDownLineCount( nLines );
}

void ComboBox::AdaptDropDownLineCountToMaximum()
{
    // adapt to maximum allowed number
    SetDropDownLineCount(GetSettings().GetStyleSettings().GetListBoxMaximumLineCount());
}

sal_uInt16 ComboBox::GetDropDownLineCount() const
{
    sal_uInt16 nLines = 0;
    if ( mpFloatWin )
        nLines = mpFloatWin->GetDropDownLineCount();
    return nLines;
}

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

void ComboBox::Resize()
{
    Control::Resize();

    if (mpSubEdit)
    {
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
    }

    // adjust the size of the FloatingWindow even when invisible
    // as KEY_PGUP/DOWN is being processed...
    if ( mpFloatWin )
        mpFloatWin->SetSizePixel( mpFloatWin->CalcFloatSize() );
}

void ComboBox::FillLayoutData() const
{
    mpControlData->mpLayoutData = new vcl::ControlLayoutData();
    AppendLayoutData( *mpSubEdit );
    mpSubEdit->SetLayoutDataParent( this );
    ImplListBoxWindow* rMainWindow = mpImplLB->GetMainWindow();
    if( mpFloatWin )
    {
        // dropdown mode
        if( mpFloatWin->IsReallyVisible() )
        {
            AppendLayoutData( *rMainWindow );
            rMainWindow->SetLayoutDataParent( this );
        }
    }
    else
    {
        AppendLayoutData( *rMainWindow );
        rMainWindow->SetLayoutDataParent( this );
    }
}

void ComboBox::StateChanged( StateChangedType nType )
{
    Edit::StateChanged( nType );

    if ( nType == StateChangedType::ReadOnly )
    {
        mpImplLB->SetReadOnly( IsReadOnly() );
        if ( mpBtn )
            mpBtn->Enable( IsEnabled() && !IsReadOnly() );
    }
    else if ( nType == StateChangedType::Enable )
    {
        mpSubEdit->Enable( IsEnabled() );
        mpImplLB->Enable( IsEnabled() && !IsReadOnly() );
        if ( mpBtn )
            mpBtn->Enable( IsEnabled() && !IsReadOnly() );
        Invalidate();
    }
    else if( nType == StateChangedType::UpdateMode )
    {
        mpImplLB->SetUpdateMode( IsUpdateMode() );
    }
    else if ( nType == StateChangedType::Zoom )
    {
        mpImplLB->SetZoom( GetZoom() );
        mpSubEdit->SetZoom( GetZoom() );
        ImplCalcEditHeight();
        Resize();
    }
    else if ( nType == StateChangedType::ControlFont )
    {
        mpImplLB->SetControlFont( GetControlFont() );
        mpSubEdit->SetControlFont( GetControlFont() );
        ImplCalcEditHeight();
        Resize();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        mpImplLB->SetControlForeground( GetControlForeground() );
        mpSubEdit->SetControlForeground( GetControlForeground() );
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        mpImplLB->SetControlBackground( GetControlBackground() );
        mpSubEdit->SetControlBackground( GetControlBackground() );
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        mpImplLB->GetMainWindow()->EnableSort( ( GetStyle() & WB_SORT ) != 0 );
    }
    else if( nType == StateChangedType::Mirroring )
    {
        if( mpBtn )
        {
            mpBtn->EnableRTL( IsRTLEnabled() );
            ImplInitDropDownButton( mpBtn );
        }
        mpSubEdit->StateChanged( StateChangedType::Mirroring );
        mpImplLB->EnableRTL( IsRTLEnabled() );
        Resize();
    }
}

void ComboBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
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

bool ComboBox::PreNotify( NotifyEvent& rNEvt )
{

    return Edit::PreNotify( rNEvt );
}

bool ComboBox::Notify( NotifyEvent& rNEvt )
{
    bool nDone = false;
    if( ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT ) && ( rNEvt.GetWindow() == mpSubEdit )
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
                    CallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
                    mpBtn->SetPressed( true );
                    if ( mpImplLB->GetEntryList()->GetMRUCount() )
                        mpImplLB->SelectEntry( 0 , true );
                    SetSelection( Selection( 0, SELECTION_MAX ) );
                    mpFloatWin->StartFloat( false );
                    CallEventListeners( VCLEVENT_DROPDOWN_OPEN );
                    nDone = true;
                }
                else if( ( nKeyCode == KEY_UP ) && mpFloatWin && mpFloatWin->IsInPopupMode() && aKeyEvt.GetKeyCode().IsMod2() )
                {
                    mpFloatWin->EndPopupMode();
                    nDone = true;
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
                    nDone = true;
                }
            }
            break;
        }
    }
    else if ( (rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS) && mpFloatWin )
    {
        if( mpFloatWin->HasChildPathFocus() )
            mpSubEdit->GrabFocus();
        else if ( mpFloatWin->IsInPopupMode() && !HasChildPathFocus( true ) )
            mpFloatWin->EndPopupMode();
    }
    else if( (rNEvt.GetType() == MouseNotifyEvent::COMMAND) &&
             (rNEvt.GetCommandEvent()->GetCommand() == CommandEventId::Wheel) &&
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
            nDone = false;  // don't eat this event, let the default handling happen (i.e. scroll the context)
        }
    }
    else if( ( rNEvt.GetType() == MouseNotifyEvent::MOUSEBUTTONDOWN ) && ( rNEvt.GetWindow() == mpImplLB->GetMainWindow() ) )
    {
        mpSubEdit->GrabFocus();
    }

    return nDone || Edit::Notify( rNEvt );
}

void ComboBox::SetText( const OUString& rStr )
{
    CallEventListeners( VCLEVENT_COMBOBOX_SETTEXT );

    Edit::SetText( rStr );
    ImplUpdateFloatSelection();
}

void ComboBox::SetText( const OUString& rStr, const Selection& rNewSelection )
{
    CallEventListeners( VCLEVENT_COMBOBOX_SETTEXT );

    Edit::SetText( rStr, rNewSelection );
    ImplUpdateFloatSelection();
}

void ComboBox::Modify()
{
    if ( !mbSyntheticModify )
        ImplUpdateFloatSelection();

    Edit::Modify();
}

void ComboBox::ImplUpdateFloatSelection()
{
    if (!mpImplLB)
        return;

    // move text in the ListBox into the visible region
    mpImplLB->SetCallSelectionChangedHdl( false );
    if ( !IsMultiSelectionEnabled() )
    {
        OUString        aSearchStr( mpSubEdit->GetText() );
        sal_Int32       nSelect = LISTBOX_ENTRY_NOTFOUND;
        bool        bSelect = true;

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
            bSelect = false;
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
                mpImplLB->SelectEntry( nSelect, false );
            mpImplLB->ResetCurrentPos();
        }
    }
    else
    {
        ::std::set< sal_Int32 > aSelInText;
        lcl_GetSelectedEntries( aSelInText, mpSubEdit->GetText(), mcMultiSep, mpImplLB->GetEntryList() );
        for ( sal_Int32 n = 0; n < mpImplLB->GetEntryList()->GetEntryCount(); n++ )
            mpImplLB->SelectEntry( n, aSelInText.count( n ) );
    }
    mpImplLB->SetCallSelectionChangedHdl( true );
}

sal_Int32 ComboBox::InsertEntry(const OUString& rStr, sal_Int32 const nPos)
{
    assert(nPos >= 0 && COMBOBOX_MAX_ENTRIES > mpImplLB->GetEntryList()->GetEntryCount());

    sal_Int32 nRealPos;
    if (nPos == COMBOBOX_APPEND)
        nRealPos = nPos;
    else
    {
        const sal_Int32 nMRUCount = mpImplLB->GetEntryList()->GetMRUCount();
        assert(nPos <= COMBOBOX_MAX_ENTRIES - nMRUCount);
        nRealPos = nPos + nMRUCount;
    }

    nRealPos = mpImplLB->InsertEntry( nRealPos, rStr );
    nRealPos -= mpImplLB->GetEntryList()->GetMRUCount();
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMADDED, reinterpret_cast<void*>(nRealPos) );
    return nRealPos;
}

sal_Int32 ComboBox::InsertEntryWithImage(
        const OUString& rStr, const Image& rImage, sal_Int32 const nPos)
{
    assert(nPos >= 0 && COMBOBOX_MAX_ENTRIES > mpImplLB->GetEntryList()->GetEntryCount());

    sal_Int32 nRealPos;
    if (nPos == COMBOBOX_APPEND)
        nRealPos = nPos;
    else
    {
        const sal_Int32 nMRUCount = mpImplLB->GetEntryList()->GetMRUCount();
        assert(nPos <= COMBOBOX_MAX_ENTRIES - nMRUCount);
        nRealPos = nPos + nMRUCount;
    }

    nRealPos = mpImplLB->InsertEntry( nRealPos, rStr, rImage );
    nRealPos -= mpImplLB->GetEntryList()->GetMRUCount();
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMADDED, reinterpret_cast<void*>(nRealPos) );
    return nRealPos;
}

void ComboBox::RemoveEntry( const OUString& rStr )
{
    RemoveEntryAt(GetEntryPos(rStr));
}

void ComboBox::RemoveEntryAt(sal_Int32 const nPos)
{
    const sal_Int32 nMRUCount = mpImplLB->GetEntryList()->GetMRUCount();
    if (nPos < 0 || nPos > COMBOBOX_MAX_ENTRIES - nMRUCount)
        return;

    mpImplLB->RemoveEntry( nPos + nMRUCount );
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMREMOVED, reinterpret_cast<void*>(nPos) );
}

void ComboBox::Clear()
{
    mpImplLB->Clear();
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMREMOVED, reinterpret_cast<void*>(-1) );
}

Image ComboBox::GetEntryImage( sal_Int32 nPos ) const
{
    if ( mpImplLB->GetEntryList()->HasEntryImage( nPos ) )
        return mpImplLB->GetEntryList()->GetEntryImage( nPos );
    return Image();
}

sal_Int32 ComboBox::GetEntryPos( const OUString& rStr ) const
{
    sal_Int32 nPos = mpImplLB->GetEntryList()->FindEntry( rStr );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos -= mpImplLB->GetEntryList()->GetMRUCount();
    return nPos;
}

sal_Int32 ComboBox::GetEntryPos( const void* pData ) const
{
    sal_Int32 nPos = mpImplLB->GetEntryList()->FindEntry( pData );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos = nPos - mpImplLB->GetEntryList()->GetMRUCount();
    return nPos;
}

OUString ComboBox::GetEntry( sal_Int32 nPos ) const
{
    const sal_Int32 nMRUCount = mpImplLB->GetEntryList()->GetMRUCount();
    if (nPos < 0 || nPos > COMBOBOX_MAX_ENTRIES - nMRUCount)
        return OUString();

    return mpImplLB->GetEntryList()->GetEntryText( nPos + nMRUCount );
}

sal_Int32 ComboBox::GetEntryCount() const
{
    return mpImplLB->GetEntryList()->GetEntryCount() - mpImplLB->GetEntryList()->GetMRUCount();
}

bool ComboBox::IsTravelSelect() const
{
    return mpImplLB->IsTravelSelect();
}

bool ComboBox::IsInDropDown() const
{
    return mpFloatWin && mpFloatWin->IsInPopupMode();
}

void ComboBox::EnableMultiSelection( bool bMulti )
{
    mpImplLB->EnableMultiSelection( bMulti, false );
    mpImplLB->SetMultiSelectionSimpleMode( true );
}

bool ComboBox::IsMultiSelectionEnabled() const
{
    return mpImplLB->IsMultiSelectionEnabled();
}

long ComboBox::CalcWindowSizePixel( sal_uInt16 nLines ) const
{
    return mpImplLB->GetEntryHeight() * nLines;
}

Size ComboBox::GetOptimalSize() const
{
    return CalcMinimumSize();
}

long ComboBox::getMaxWidthScrollBarAndDownButton() const
{
    long nButtonDownWidth = 0;

    vcl::Window *pBorder = GetWindow( WINDOW_BORDER );
    EditBoxValue aControlValue(GetTextHeight());
    Point aPoint;
    Rectangle aContent, aBound;

    // use the full extent of the control
    Rectangle aArea( aPoint, pBorder->GetOutputSizePixel() );

    if ( GetNativeControlRegion(CTRL_COMBOBOX, PART_BUTTON_DOWN,
        aArea, ControlState::NONE, aControlValue, OUString(), aBound, aContent) )
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

Size ComboBox::CalcAdjustedSize( const Size& rPrefSize ) const
{
    Size aSz = rPrefSize;
    sal_Int32 nLeft, nTop, nRight, nBottom;
    ((vcl::Window*)this)->GetBorder( nLeft, nTop, nRight, nBottom );
    aSz.Height() -= nTop+nBottom;
    if ( !IsDropDownBox() )
    {
        long nEntryHeight = CalcBlockSize( 1, 1 ).Height();
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

Size ComboBox::CalcBlockSize( sal_uInt16 nColumns, sal_uInt16 nLines ) const
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

void ComboBox::GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const
{
    long nCharWidth = GetTextWidth(OUString(static_cast<sal_Unicode>('x')));
    if ( !IsDropDownBox() )
    {
        Size aOutSz = mpImplLB->GetMainWindow()->GetOutputSizePixel();
        rnCols = (nCharWidth > 0) ? (sal_uInt16)(aOutSz.Width()/nCharWidth) : 1;
        rnLines = (sal_uInt16)(aOutSz.Height()/mpImplLB->GetEntryHeight());
    }
    else
    {
        Size aOutSz = mpSubEdit->GetOutputSizePixel();
        rnCols = (nCharWidth > 0) ? (sal_uInt16)(aOutSz.Width()/nCharWidth) : 1;
        rnLines = 1;
    }
}

void ComboBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    mpImplLB->GetMainWindow()->ImplInitSettings( true, true, true );

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    vcl::Font aFont = mpImplLB->GetMainWindow()->GetDrawPixelFont( pDev );
    OutDevType eOutDevType = pDev->GetOutDevType();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    bool bBorder = !(nFlags & WINDOW_DRAW_NOBORDER ) && (GetStyle() & WB_BORDER);
    bool bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && IsControlBackground();
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
        sal_uInt16 nLines = (sal_uInt16) ( ( nTextHeight > 0 ) ? ( (aSize.Height()-nEditHeight) / nTextHeight ) : 1 );
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

void::ComboBox::ImplUserDrawHandler( UserDrawEvent* pEvent )
{
    UserDraw( *pEvent );
}

void ComboBox::UserDraw( const UserDrawEvent& )
{
}

void ComboBox::SetUserItemSize( const Size& rSz )
{
    mpImplLB->GetMainWindow()->SetUserItemSize( rSz );
}

void ComboBox::EnableUserDraw( bool bUserDraw )
{
    mpImplLB->GetMainWindow()->EnableUserDraw( bUserDraw );
}

void ComboBox::DrawEntry(const UserDrawEvent& rEvt, bool bDrawImage, bool bDrawText, bool bDrawTextAtImagePos)
{
    DBG_ASSERT(rEvt.GetDevice() == mpImplLB->GetMainWindow(), "DrawEntry?!");
    mpImplLB->GetMainWindow()->DrawEntry(*rEvt.GetDevice(), rEvt.GetItemId(), bDrawImage, bDrawText, bDrawTextAtImagePos);
}

void ComboBox::SetSeparatorPos( sal_Int32 n )
{
    mpImplLB->SetSeparatorPos( n );
}

void ComboBox::SetMRUEntries( const OUString& rEntries, sal_Unicode cSep )
{
    mpImplLB->SetMRUEntries( rEntries, cSep );
}

OUString ComboBox::GetMRUEntries( sal_Unicode cSep ) const
{
    return mpImplLB ? mpImplLB->GetMRUEntries( cSep ) : OUString();
}

void ComboBox::SetMaxMRUCount( sal_Int32 n )
{
    mpImplLB->SetMaxMRUCount( n );
}

sal_Int32 ComboBox::GetMaxMRUCount() const
{
    return mpImplLB ? mpImplLB->GetMaxMRUCount() : 0;
}

sal_uInt16 ComboBox::GetDisplayLineCount() const
{
    return mpImplLB ? mpImplLB->GetDisplayLineCount() : 0;
}

void ComboBox::SetEntryData( sal_Int32 nPos, void* pNewData )
{
    mpImplLB->SetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount(), pNewData );
}

void* ComboBox::GetEntryData( sal_Int32 nPos ) const
{
    return mpImplLB->GetEntryList()->GetEntryData( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

sal_Int32 ComboBox::GetTopEntry() const
{
    sal_Int32 nPos = GetEntryCount() ? mpImplLB->GetTopEntry() : LISTBOX_ENTRY_NOTFOUND;
    if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
        nPos = 0;
    return nPos;
}

void ComboBox::SetProminentEntryType( ProminentEntry eType )
{
    mpImplLB->SetProminentEntryType( eType );
}

Rectangle ComboBox::GetDropDownPosSizePixel() const
{
    return mpFloatWin ? mpFloatWin->GetWindowExtentsRelative( const_cast<ComboBox*>(this) ) : Rectangle();
}

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

sal_Int32 ComboBox::GetSelectEntryCount() const
{
    return mpImplLB->GetEntryList()->GetSelectEntryCount();
}

sal_Int32 ComboBox::GetSelectEntryPos( sal_Int32 nIndex ) const
{
    sal_Int32 nPos = mpImplLB->GetEntryList()->GetSelectEntryPos( nIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if ( nPos < mpImplLB->GetEntryList()->GetMRUCount() )
            nPos = mpImplLB->GetEntryList()->FindEntry( mpImplLB->GetEntryList()->GetEntryText( nPos ) );
        nPos = sal::static_int_cast<sal_Int32>(nPos - mpImplLB->GetEntryList()->GetMRUCount());
    }
    return nPos;
}

bool ComboBox::IsEntryPosSelected( sal_Int32 nPos ) const
{
    return mpImplLB->GetEntryList()->IsEntryPosSelected( nPos + mpImplLB->GetEntryList()->GetMRUCount() );
}

void ComboBox::SelectEntryPos( sal_Int32 nPos, bool bSelect)
{
    if ( nPos < mpImplLB->GetEntryList()->GetEntryCount() )
        mpImplLB->SelectEntry( nPos + mpImplLB->GetEntryList()->GetMRUCount(), bSelect );
}

void ComboBox::SetNoSelection()
{
    mpImplLB->SetNoSelection();
    mpSubEdit->SetText( OUString() );
}

Rectangle ComboBox::GetBoundingRectangle( sal_Int32 nItem ) const
{
    Rectangle aRect = mpImplLB->GetMainWindow()->GetBoundingRectangle( nItem );
    Rectangle aOffset = mpImplLB->GetMainWindow()->GetWindowExtentsRelative( (vcl::Window*)this );
    aRect.Move( aOffset.TopLeft().X(), aOffset.TopLeft().Y() );
    return aRect;
}

void ComboBox::SetBorderStyle( WindowBorderStyle nBorderStyle )
{
    Window::SetBorderStyle( nBorderStyle );
    if ( !IsDropDownBox() )
    {
        mpSubEdit->SetBorderStyle( nBorderStyle );
        mpImplLB->SetBorderStyle( nBorderStyle );
    }
}

long ComboBox::GetIndexForPoint( const Point& rPoint, sal_Int32& rPos ) const
{
    if( !HasLayoutData() )
        FillLayoutData();

    // check whether rPoint fits at all
    long nIndex = Control::GetIndexForPoint( rPoint );
    if( nIndex != -1 )
    {
        // point must be either in main list window
        // or in impl window (dropdown case)
        ImplListBoxWindow* rMain = mpImplLB->GetMainWindow();

        // convert coordinates to ImplListBoxWindow pixel coordinate space
        Point aConvPoint = LogicToPixel( rPoint );
        aConvPoint = OutputToAbsoluteScreenPixel( aConvPoint );
        aConvPoint = rMain->AbsoluteScreenToOutputPixel( aConvPoint );
        aConvPoint = rMain->PixelToLogic( aConvPoint );

        // try to find entry
        sal_Int32 nEntry = rMain->GetEntryPosForPoint( aConvPoint );
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

    vcl::Window *pBorder = GetWindow( WINDOW_BORDER );
    EditBoxValue aControlValue(GetTextHeight());
    Point aPoint;
    Rectangle aContent, aBound;

    // use the full extent of the control
    Rectangle aArea( aPoint, rBorderOutSz );

    if ( GetNativeControlRegion(CTRL_COMBOBOX, PART_BUTTON_DOWN,
            aArea, ControlState::NONE, aControlValue, OUString(), aBound, aContent) )
    {
        // convert back from border space to local coordinates
        aPoint = pBorder->ScreenToOutputPixel( OutputToScreenPixel( aPoint ) );
        aContent.Move(-aPoint.X(), -aPoint.Y());

        aBounds.aButtonPos = Point(aContent.Left(), nTop);
        aBounds.aButtonSize = Size(aContent.getWidth(), (nBottom-nTop));

        // adjust the size of the edit field
        if ( GetNativeControlRegion(CTRL_COMBOBOX, PART_SUB_EDIT,
                    aArea, ControlState::NONE, aControlValue, OUString(), aBound, aContent) )
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
