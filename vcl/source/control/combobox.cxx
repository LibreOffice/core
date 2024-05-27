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

#include <vcl/toolkit/combobox.hxx>

#include <set>

#include <comphelper/string.hxx>
#include <vcl/toolkit/lstbox.hxx>
#include <vcl/builder.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <sal/log.hxx>

#include <listbox.hxx>
#include <comphelper/lok.hxx>
#include <tools/json_writer.hxx>
#include <o3tl/string_view.hxx>


struct ComboBoxBounds
{
    Point aSubEditPos;
    Size aSubEditSize;

    Point aButtonPos;
    Size aButtonSize;
};

static void lcl_GetSelectedEntries( ::std::set< sal_Int32 >& rSelectedPos, std::u16string_view rText, sal_Unicode cTokenSep, const ImplEntryList& rEntryList )
{
    if (rText.empty())
        return;

    sal_Int32 nIdx{0};
    do {
        const sal_Int32 nPos = rEntryList.FindEntry(comphelper::string::strip(o3tl::getToken(rText, 0, cTokenSep, nIdx), ' '));
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            rSelectedPos.insert( nPos );
    } while (nIdx>=0);
}

ComboBox::ComboBox(vcl::Window *const pParent, WinBits const nStyle)
    : Edit( WindowType::COMBOBOX )
    , m_nDDHeight(0)
    , m_cMultiSep(0)
    , m_isDDAutoSize(false)
    , m_isSyntheticModify(false)
    , m_isKeyBoardModify(false)
    , m_isMatchCase(false)
    , m_nMaxWidthChars(0)
    , m_nWidthInChars(-1)
{
    ImplInitComboBoxData();
    ImplInit( pParent, nStyle );
    SetWidthInChars(-1);
}

ComboBox::~ComboBox()
{
    disposeOnce();
}

void ComboBox::dispose()
{
    m_pSubEdit.disposeAndClear();

    VclPtr< ImplListBox > pImplLB = m_pImplLB;
    m_pImplLB.clear();
    pImplLB.disposeAndClear();

    m_pFloatWin.disposeAndClear();
    m_pBtn.disposeAndClear();
    Edit::dispose();
}

void ComboBox::ImplInitComboBoxData()
{
    m_pSubEdit.disposeAndClear();
    m_pBtn              = nullptr;
    m_pImplLB           = nullptr;
    m_pFloatWin         = nullptr;

    m_nDDHeight         = 0;
    m_isDDAutoSize      = true;
    m_isSyntheticModify = false;
    m_isKeyBoardModify  = false;
    m_isMatchCase       = false;
    m_cMultiSep         = ';';
    m_nMaxWidthChars    = -1;
    m_nWidthInChars     = -1;
}

void ComboBox::ImplCalcEditHeight()
{
    sal_Int32 nLeft, nTop, nRight, nBottom;
    GetBorder( nLeft, nTop, nRight, nBottom );
    m_nDDHeight = static_cast<sal_uInt16>(m_pSubEdit->GetTextHeight() + nTop + nBottom + 4);
    if ( !IsDropDownBox() )
        m_nDDHeight += 4;

    tools::Rectangle aCtrlRegion( Point( 0, 0 ), Size( 10, 10 ) );
    tools::Rectangle aBoundRegion, aContentRegion;
    ImplControlValue aControlValue;
    ControlType aType = IsDropDownBox() ? ControlType::Combobox : ControlType::Editbox;
    if( GetNativeControlRegion( aType, ControlPart::Entire,
                                aCtrlRegion,
                                ControlState::ENABLED,
                                aControlValue,
                                aBoundRegion, aContentRegion ) )
    {
        const tools::Long nNCHeight = aBoundRegion.GetHeight();
        if (m_nDDHeight < nNCHeight)
            m_nDDHeight = sal::static_int_cast<sal_uInt16>(nNCHeight);
    }
}

void ComboBox::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
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
        m_pFloatWin = VclPtr<ImplListBoxFloatingWindow>::Create( this );
        if (!IsNativeControlSupported(ControlType::Pushbutton, ControlPart::Focus))
            m_pFloatWin->RequestDoubleBuffering(true);
        m_pFloatWin->SetAutoWidth( true );
        m_pFloatWin->SetPopupModeEndHdl(LINK(this, ComboBox, ImplPopupModeEndHdl));

        m_pBtn = VclPtr<ImplBtn>::Create( this, WB_NOLIGHTBORDER | WB_RECTSTYLE );
        ImplInitDropDownButton( m_pBtn );
        m_pBtn->SetMBDownHdl(LINK(this, ComboBox, ImplClickBtnHdl));
        m_pBtn->Show();

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

    m_pSubEdit.set( VclPtr<Edit>::Create( this, nEditStyle ) );
    m_pSubEdit->EnableRTL( false );
    SetSubEdit( m_pSubEdit );
    m_pSubEdit->SetPosPixel( Point() );
    EnableAutocomplete( true );
    m_pSubEdit->Show();

    vcl::Window* pLBParent = this;
    if (m_pFloatWin)
        pLBParent = m_pFloatWin;
    m_pImplLB = VclPtr<ImplListBox>::Create( pLBParent, nListStyle|WB_SIMPLEMODE|WB_AUTOHSCROLL );
    m_pImplLB->SetPosPixel( Point() );
    m_pImplLB->SetSelectHdl(LINK(this, ComboBox, ImplSelectHdl));
    m_pImplLB->SetCancelHdl( LINK(this, ComboBox, ImplCancelHdl));
    m_pImplLB->SetDoubleClickHdl(LINK(this, ComboBox, ImplDoubleClickHdl));
    m_pImplLB->SetSelectionChangedHdl(LINK(this, ComboBox, ImplSelectionChangedHdl));
    m_pImplLB->SetListItemSelectHdl(LINK(this, ComboBox, ImplListItemSelectHdl));
    m_pImplLB->Show();

    if (m_pFloatWin)
        m_pFloatWin->SetImplListBox(m_pImplLB);
    else
        GetMainWindow()->AllowGrabFocus( true );

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

void ComboBox::EnableAutocomplete( bool bEnable, bool bMatchCase )
{
    m_isMatchCase = bMatchCase;

    if ( bEnable )
        m_pSubEdit->SetAutocompleteHdl(LINK(this, ComboBox, ImplAutocompleteHdl));
    else
        m_pSubEdit->SetAutocompleteHdl( Link<Edit&,void>() );
}

bool ComboBox::IsAutocompleteEnabled() const
{
    return m_pSubEdit->GetAutocompleteHdl().IsSet();
}

IMPL_LINK_NOARG(ComboBox, ImplClickBtnHdl, void*, void)
{
    CallEventListeners( VclEventId::DropdownPreOpen );
    m_pSubEdit->GrabFocus();
    if (!m_pImplLB->GetEntryList().GetMRUCount())
        ImplUpdateFloatSelection();
    else
        m_pImplLB->SelectEntry( 0 , true );
    m_pBtn->SetPressed( true );
    SetSelection( Selection( 0, SELECTION_MAX ) );
    m_pFloatWin->StartFloat( true );
    CallEventListeners( VclEventId::DropdownOpen );

    ImplClearLayoutData();
    if (m_pImplLB)
        m_pImplLB->GetMainWindow()->ImplClearLayoutData();
}

IMPL_LINK_NOARG(ComboBox, ImplPopupModeEndHdl, FloatingWindow*, void)
{
    if (m_pFloatWin->IsPopupModeCanceled())
    {
        if (!m_pImplLB->GetEntryList().IsEntryPosSelected(
                    m_pFloatWin->GetPopupModeStartSaveSelection()))
        {
            m_pImplLB->SelectEntry(m_pFloatWin->GetPopupModeStartSaveSelection(), true);
            bool bTravelSelect = m_pImplLB->IsTravelSelect();
            m_pImplLB->SetTravelSelect( true );
            Select();
            m_pImplLB->SetTravelSelect( bTravelSelect );
        }
    }

    ImplClearLayoutData();
    if (m_pImplLB)
        m_pImplLB->GetMainWindow()->ImplClearLayoutData();

    m_pBtn->SetPressed( false );
    CallEventListeners( VclEventId::DropdownClose );
}

IMPL_LINK(ComboBox, ImplAutocompleteHdl, Edit&, rEdit, void)
{
    Selection           aSel = rEdit.GetSelection();

    {
        OUString    aFullText = rEdit.GetText();
        OUString    aStartText = aFullText.copy( 0, static_cast<sal_Int32>(aSel.Max()) );
        sal_Int32   nStart = m_pImplLB->GetCurrentPos();

        if ( nStart == LISTBOX_ENTRY_NOTFOUND )
            nStart = 0;

        sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;
        if (!m_isMatchCase)
        {
            // Try match case insensitive from current position
            nPos = m_pImplLB->GetEntryList().FindMatchingEntry(aStartText, nStart, true);
            if ( nPos == LISTBOX_ENTRY_NOTFOUND )
                // Try match case insensitive, but from start
                nPos = m_pImplLB->GetEntryList().FindMatchingEntry(aStartText, 0, true);
        }

        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            // Try match full from current position
            nPos = m_pImplLB->GetEntryList().FindMatchingEntry(aStartText, nStart, false);
        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            //  Match full, but from start
            nPos = m_pImplLB->GetEntryList().FindMatchingEntry(aStartText, 0, false);

        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            OUString aText = m_pImplLB->GetEntryList().GetEntryText( nPos );
            Selection aSelection( aText.getLength(), aStartText.getLength() );
            rEdit.SetText( aText, aSelection );
        }
    }
}

IMPL_LINK_NOARG(ComboBox, ImplSelectHdl, LinkParamNone*, void)
{
    bool bPopup = IsInDropDown();
    bool bCallSelect = false;
    if (m_pImplLB->IsSelectionChanged() || bPopup)
    {
        OUString aText;
        if (IsMultiSelectionEnabled())
        {
            aText = m_pSubEdit->GetText();

            // remove all entries to which there is an entry, but which is not selected
            sal_Int32 nIndex = 0;
            while ( nIndex >= 0 )
            {
                sal_Int32  nPrevIndex = nIndex;
                std::u16string_view aToken = o3tl::getToken(aText, 0, m_cMultiSep, nIndex );
                sal_Int32  nTokenLen = aToken.size();
                aToken = comphelper::string::strip(aToken, ' ');
                sal_Int32 nP = m_pImplLB->GetEntryList().FindEntry( aToken );
                if ((nP != LISTBOX_ENTRY_NOTFOUND) && (!m_pImplLB->GetEntryList().IsEntryPosSelected(nP)))
                {
                    aText = aText.replaceAt( nPrevIndex, nTokenLen, u"" );
                    nIndex = nIndex - nTokenLen;
                    sal_Int32 nSepCount=0;
                    if ((nPrevIndex+nSepCount < aText.getLength()) && (aText[nPrevIndex+nSepCount] == m_cMultiSep))
                    {
                        nIndex--;
                        ++nSepCount;
                    }
                    aText = aText.replaceAt( nPrevIndex, nSepCount, u"" );
                }
                aText = comphelper::string::strip(aText, ' ');
            }

            // attach missing entries
            ::std::set< sal_Int32 > aSelInText;
            lcl_GetSelectedEntries( aSelInText, aText, m_cMultiSep, m_pImplLB->GetEntryList() );
            sal_Int32 nSelectedEntries = m_pImplLB->GetEntryList().GetSelectedEntryCount();
            for ( sal_Int32 n = 0; n < nSelectedEntries; n++ )
            {
                sal_Int32 nP = m_pImplLB->GetEntryList().GetSelectedEntryPos( n );
                if ( !aSelInText.count( nP ) )
                {
                    if (!aText.isEmpty() && (aText[aText.getLength()-1] != m_cMultiSep))
                        aText += OUStringChar(m_cMultiSep);
                    if ( !aText.isEmpty() )
                        aText += " ";   // slightly loosen
                    aText += m_pImplLB->GetEntryList().GetEntryText( nP ) +
                        OUStringChar(m_cMultiSep);
                }
            }
            aText = comphelper::string::stripEnd( aText, m_cMultiSep );
        }
        else
        {
            aText = m_pImplLB->GetEntryList().GetSelectedEntry( 0 );
        }

        m_pSubEdit->SetText( aText );

        switch (GetSettings().GetStyleSettings().GetComboBoxTextSelectionMode())
        {
            case ComboBoxTextSelectionMode::SelectText:
            {
                Selection aNewSelection(0, aText.getLength());
                if (IsMultiSelectionEnabled())
                    aNewSelection.Min() = aText.getLength();
                m_pSubEdit->SetSelection(aNewSelection);
                break;
            }
            case ComboBoxTextSelectionMode::CursorToEnd:
                m_pSubEdit->SetCursorAtLast();
                break;
            default:
                assert(false && "Unhandled ComboBoxTextSelectionMode case");
                break;
        }

        bCallSelect = true;
    }

    // #84652# Call GrabFocus and EndPopupMode before calling Select/Modify, but after changing the text
    bool bMenuSelect = bPopup && !m_pImplLB->IsTravelSelect() && (!IsMultiSelectionEnabled() || !m_pImplLB->GetSelectModifier());
    if (bMenuSelect)
    {
        m_pFloatWin->EndPopupMode();
        GrabFocus();
    }

    if ( bCallSelect )
    {
        m_isKeyBoardModify = !bMenuSelect;
        m_pSubEdit->SetModifyFlag();
        m_isSyntheticModify = true;
        Modify();
        m_isSyntheticModify = false;
        Select();
        m_isKeyBoardModify = false;
    }
}

bool ComboBox::IsSyntheticModify() const
{
    return m_isSyntheticModify;
}

bool ComboBox::IsModifyByKeyboard() const
{
    return m_isKeyBoardModify;
}

IMPL_LINK_NOARG(ComboBox, ImplListItemSelectHdl, LinkParamNone*, void)
{
    CallEventListeners(VclEventId::DropdownSelect);
}

IMPL_LINK_NOARG(ComboBox, ImplCancelHdl, LinkParamNone*, void)
{
    if (IsInDropDown())
        m_pFloatWin->EndPopupMode();
}

IMPL_LINK( ComboBox, ImplSelectionChangedHdl, sal_Int32, nChanged, void)
{
    if (!m_pImplLB->IsTrackingSelect())
    {
        if (!m_pSubEdit->IsReadOnly() && m_pImplLB->GetEntryList().IsEntryPosSelected(nChanged))
            m_pSubEdit->SetText(m_pImplLB->GetEntryList().GetEntryText(nChanged));
    }
}

IMPL_LINK_NOARG(ComboBox, ImplDoubleClickHdl, ImplListBoxWindow*, void)
{
    DoubleClick();
}

void ComboBox::ToggleDropDown()
{
    if( !IsDropDownBox() )
        return;

    if (m_pFloatWin->IsInPopupMode())
        m_pFloatWin->EndPopupMode();
    else
    {
        m_pSubEdit->GrabFocus();
        if (!m_pImplLB->GetEntryList().GetMRUCount())
            ImplUpdateFloatSelection();
        else
            m_pImplLB->SelectEntry( 0 , true );
        CallEventListeners( VclEventId::DropdownPreOpen );
        m_pBtn->SetPressed( true );
        SetSelection( Selection( 0, SELECTION_MAX ) );
        m_pFloatWin->StartFloat(true);
        CallEventListeners( VclEventId::DropdownOpen );
    }
}

void ComboBox::Select()
{
    ImplCallEventListenersAndHandler( VclEventId::ComboboxSelect, [this] () { m_SelectHdl.Call(*this); } );
}

void ComboBox::DoubleClick()
{
    ImplCallEventListenersAndHandler( VclEventId::ComboboxDoubleClick, [] () {} );
}

bool ComboBox::IsAutoSizeEnabled() const { return m_isDDAutoSize; }

void ComboBox::EnableAutoSize( bool bAuto )
{
    m_isDDAutoSize = bAuto;
    if (m_pFloatWin)
    {
        if (bAuto && !m_pFloatWin->GetDropDownLineCount())
        {
            // Adapt to GetListBoxMaximumLineCount here; was on fixed number of five before
            AdaptDropDownLineCountToMaximum();
        }
        else if ( !bAuto )
        {
            m_pFloatWin->SetDropDownLineCount(0);
        }
    }
}

void ComboBox::SetDropDownLineCount( sal_uInt16 nLines )
{
    if (m_pFloatWin)
        m_pFloatWin->SetDropDownLineCount(nLines);
}

void ComboBox::AdaptDropDownLineCountToMaximum()
{
    // Adapt to maximum allowed number.
    // Limit for LOK as we can't render outside of the dialog canvas.
    if (comphelper::LibreOfficeKit::isActive())
        SetDropDownLineCount(11);
    else
        SetDropDownLineCount(GetSettings().GetStyleSettings().GetListBoxMaximumLineCount());
}

sal_uInt16 ComboBox::GetDropDownLineCount() const
{
    sal_uInt16 nLines = 0;
    if (m_pFloatWin)
        nLines = m_pFloatWin->GetDropDownLineCount();
    return nLines;
}

void ComboBox::setPosSizePixel( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                                PosSizeFlags nFlags )
{
    if( IsDropDownBox() && ( nFlags & PosSizeFlags::Size ) )
    {
        Size aPrefSz = m_pFloatWin->GetPrefSize();
        if ((nFlags & PosSizeFlags::Height) && (nHeight >= 2*m_nDDHeight))
            aPrefSz.setHeight( nHeight-m_nDDHeight );
        if ( nFlags & PosSizeFlags::Width )
            aPrefSz.setWidth( nWidth );
        m_pFloatWin->SetPrefSize(aPrefSz);

        if (IsAutoSizeEnabled())
            nHeight = m_nDDHeight;
    }

    Edit::setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

void ComboBox::Resize()
{
    Control::Resize();

    if (m_pSubEdit)
    {
        Size aOutSz = GetOutputSizePixel();
        if( IsDropDownBox() )
        {
            ComboBoxBounds aBounds(calcComboBoxDropDownComponentBounds(aOutSz,
                GetWindow(GetWindowType::Border)->GetOutputSizePixel()));
            m_pSubEdit->SetPosSizePixel(aBounds.aSubEditPos, aBounds.aSubEditSize);
            m_pBtn->SetPosSizePixel(aBounds.aButtonPos, aBounds.aButtonSize);
        }
        else
        {
            m_pSubEdit->SetSizePixel(Size(aOutSz.Width(), m_nDDHeight));
            m_pImplLB->setPosSizePixel(0, m_nDDHeight,
                    aOutSz.Width(), aOutSz.Height() - m_nDDHeight);
            if ( !GetText().isEmpty() )
                ImplUpdateFloatSelection();
        }
    }

    // adjust the size of the FloatingWindow even when invisible
    // as KEY_PGUP/DOWN is being processed...
    if (m_pFloatWin)
        m_pFloatWin->SetSizePixel(m_pFloatWin->CalcFloatSize());
}

bool ComboBox::IsDropDownBox() const { return m_pFloatWin != nullptr; }

void ComboBox::FillLayoutData() const
{
    mxLayoutData.emplace();
    AppendLayoutData( *m_pSubEdit );
    m_pSubEdit->SetLayoutDataParent( this );
    ImplListBoxWindow* rMainWindow = GetMainWindow();
    if (m_pFloatWin)
    {
        // dropdown mode
        if (m_pFloatWin->IsReallyVisible())
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
        m_pImplLB->SetReadOnly( IsReadOnly() );
        if (m_pBtn)
            m_pBtn->Enable( IsEnabled() && !IsReadOnly() );
    }
    else if ( nType == StateChangedType::Enable )
    {
        m_pSubEdit->Enable( IsEnabled() );
        m_pImplLB->Enable( IsEnabled() && !IsReadOnly() );
        if (m_pBtn)
            m_pBtn->Enable( IsEnabled() && !IsReadOnly() );
        Invalidate();
    }
    else if( nType == StateChangedType::UpdateMode )
    {
        m_pImplLB->SetUpdateMode( IsUpdateMode() );
    }
    else if ( nType == StateChangedType::Zoom )
    {
        m_pImplLB->SetZoom( GetZoom() );
        m_pSubEdit->SetZoom( GetZoom() );
        ImplCalcEditHeight();
        Resize();
    }
    else if ( nType == StateChangedType::ControlFont )
    {
        m_pImplLB->SetControlFont( GetControlFont() );
        m_pSubEdit->SetControlFont( GetControlFont() );
        ImplCalcEditHeight();
        Resize();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        m_pImplLB->SetControlForeground( GetControlForeground() );
        m_pSubEdit->SetControlForeground( GetControlForeground() );
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        m_pImplLB->SetControlBackground( GetControlBackground() );
        m_pSubEdit->SetControlBackground( GetControlBackground() );
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        GetMainWindow()->EnableSort( ( GetStyle() & WB_SORT ) != 0 );
    }
    else if( nType == StateChangedType::Mirroring )
    {
        if (m_pBtn)
        {
            m_pBtn->EnableRTL( IsRTLEnabled() );
            ImplInitDropDownButton( m_pBtn );
        }
        m_pSubEdit->CompatStateChanged( StateChangedType::Mirroring );
        m_pImplLB->EnableRTL( IsRTLEnabled() );
        Resize();
    }
}

void ComboBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( !((rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))) )
        return;

    if (m_pBtn)
    {
        m_pBtn->GetOutDev()->SetSettings( GetSettings() );
        ImplInitDropDownButton( m_pBtn );
    }
    Resize();
    m_pImplLB->Resize(); // not called by ComboBox::Resize() if ImplLB is unchanged

    SetBackground();    // due to a hack in Window::UpdateSettings the background must be reset
                        // otherwise it will overpaint NWF drawn comboboxes
}

bool ComboBox::EventNotify( NotifyEvent& rNEvt )
{
    bool bDone = false;
    if ((rNEvt.GetType() == NotifyEventType::KEYINPUT)
        && (rNEvt.GetWindow() == m_pSubEdit)
        && !IsReadOnly())
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
                if ((nKeyCode == KEY_DOWN) && m_pFloatWin
                    && !m_pFloatWin->IsInPopupMode()
                    && aKeyEvt.GetKeyCode().IsMod2())
                {
                    CallEventListeners( VclEventId::DropdownPreOpen );
                    m_pBtn->SetPressed( true );
                    if (m_pImplLB->GetEntryList().GetMRUCount())
                        m_pImplLB->SelectEntry( 0 , true );
                    SetSelection( Selection( 0, SELECTION_MAX ) );
                    m_pFloatWin->StartFloat(false);
                    CallEventListeners( VclEventId::DropdownOpen );
                    bDone = true;
                }
                else if ((nKeyCode == KEY_UP) && m_pFloatWin
                        && m_pFloatWin->IsInPopupMode()
                        && aKeyEvt.GetKeyCode().IsMod2())
                {
                    m_pFloatWin->EndPopupMode();
                    bDone = true;
                }
                else
                {
                    bDone = m_pImplLB->ProcessKeyInput( aKeyEvt );
                }
            }
            break;

            case KEY_RETURN:
            {
                if ((rNEvt.GetWindow() == m_pSubEdit) && IsInDropDown())
                {
                    m_pImplLB->ProcessKeyInput( aKeyEvt );
                    bDone = true;
                }
            }
            break;
        }
    }
    else if ((rNEvt.GetType() == NotifyEventType::LOSEFOCUS) && m_pFloatWin)
    {
        if (m_pFloatWin->HasChildPathFocus())
            m_pSubEdit->GrabFocus();
        else if (m_pFloatWin->IsInPopupMode() && !HasChildPathFocus(true))
            m_pFloatWin->EndPopupMode();
    }
    else if( (rNEvt.GetType() == NotifyEventType::COMMAND) &&
             (rNEvt.GetCommandEvent()->GetCommand() == CommandEventId::Wheel) &&
             (rNEvt.GetWindow() == m_pSubEdit) )
    {
        MouseWheelBehaviour nWheelBehavior( GetSettings().GetMouseSettings().GetWheelBehavior() );
        if  (   ( nWheelBehavior == MouseWheelBehaviour::ALWAYS )
            ||  (   ( nWheelBehavior == MouseWheelBehaviour::FocusOnly )
                &&  HasChildPathFocus()
                )
            )
        {
            bDone = m_pImplLB->HandleWheelAsCursorTravel(*rNEvt.GetCommandEvent(), *this);
        }
        else
        {
            bDone = false;  // don't eat this event, let the default handling happen (i.e. scroll the context)
        }
    }
    else if ((rNEvt.GetType() == NotifyEventType::MOUSEBUTTONDOWN)
            && (rNEvt.GetWindow() == GetMainWindow()))
    {
        m_pSubEdit->GrabFocus();
    }

    return bDone || Edit::EventNotify( rNEvt );
}

void ComboBox::SetText( const OUString& rStr )
{
    CallEventListeners( VclEventId::ComboboxSetText );

    Edit::SetText( rStr );
    ImplUpdateFloatSelection();
}

void ComboBox::SetText( const OUString& rStr, const Selection& rNewSelection )
{
    CallEventListeners( VclEventId::ComboboxSetText );

    Edit::SetText( rStr, rNewSelection );
    ImplUpdateFloatSelection();
}

void ComboBox::Modify()
{
    if (!m_isSyntheticModify)
        ImplUpdateFloatSelection();

    Edit::Modify();
}

void ComboBox::ImplUpdateFloatSelection()
{
    if (!m_pImplLB || !m_pSubEdit)
        return;

    // move text in the ListBox into the visible region
    m_pImplLB->SetCallSelectionChangedHdl( false );
    if (!IsMultiSelectionEnabled())
    {
        OUString        aSearchStr( m_pSubEdit->GetText() );
        sal_Int32       nSelect = LISTBOX_ENTRY_NOTFOUND;
        bool        bSelect = true;

        if (m_pImplLB->GetCurrentPos() != LISTBOX_ENTRY_NOTFOUND)
        {
            OUString aCurrent = m_pImplLB->GetEntryList().GetEntryText(
                                    m_pImplLB->GetCurrentPos());
            if ( aCurrent == aSearchStr )
                nSelect = m_pImplLB->GetCurrentPos();
        }

        if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
            nSelect = m_pImplLB->GetEntryList().FindEntry( aSearchStr );
        if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
        {
            nSelect = m_pImplLB->GetEntryList().FindMatchingEntry( aSearchStr, 0, true );
            bSelect = false;
        }

        if( nSelect != LISTBOX_ENTRY_NOTFOUND )
        {
            if (!m_pImplLB->IsVisible(nSelect))
                m_pImplLB->ShowProminentEntry( nSelect );
            m_pImplLB->SelectEntry( nSelect, bSelect );
        }
        else
        {
            nSelect = m_pImplLB->GetEntryList().GetSelectedEntryPos( 0 );
            if( nSelect != LISTBOX_ENTRY_NOTFOUND )
                m_pImplLB->SelectEntry( nSelect, false );
            m_pImplLB->ResetCurrentPos();
        }
    }
    else
    {
        ::std::set< sal_Int32 > aSelInText;
        lcl_GetSelectedEntries(aSelInText, m_pSubEdit->GetText(), m_cMultiSep, m_pImplLB->GetEntryList());
        for (sal_Int32 n = 0; n < m_pImplLB->GetEntryList().GetEntryCount(); n++)
            m_pImplLB->SelectEntry( n, aSelInText.count( n ) != 0 );
    }
    m_pImplLB->SetCallSelectionChangedHdl( true );
}

sal_Int32 ComboBox::InsertEntry(const OUString& rStr, sal_Int32 const nPos)
{
    assert(nPos >= 0 && COMBOBOX_MAX_ENTRIES > m_pImplLB->GetEntryList().GetEntryCount());

    sal_Int32 nRealPos;
    if (nPos == COMBOBOX_APPEND)
        nRealPos = nPos;
    else
    {
        const sal_Int32 nMRUCount = m_pImplLB->GetEntryList().GetMRUCount();
        assert(nPos <= COMBOBOX_MAX_ENTRIES - nMRUCount);
        nRealPos = nPos + nMRUCount;
    }

    nRealPos = m_pImplLB->InsertEntry( nRealPos, rStr );
    nRealPos -= m_pImplLB->GetEntryList().GetMRUCount();
    CallEventListeners( VclEventId::ComboboxItemAdded, reinterpret_cast<void*>(static_cast<sal_IntPtr>(nRealPos)) );
    return nRealPos;
}

sal_Int32 ComboBox::InsertEntryWithImage(
        const OUString& rStr, const Image& rImage, sal_Int32 const nPos)
{
    assert(nPos >= 0 && COMBOBOX_MAX_ENTRIES > m_pImplLB->GetEntryList().GetEntryCount());

    sal_Int32 nRealPos;
    if (nPos == COMBOBOX_APPEND)
        nRealPos = nPos;
    else
    {
        const sal_Int32 nMRUCount = m_pImplLB->GetEntryList().GetMRUCount();
        assert(nPos <= COMBOBOX_MAX_ENTRIES - nMRUCount);
        nRealPos = nPos + nMRUCount;
    }

    nRealPos = m_pImplLB->InsertEntry( nRealPos, rStr, rImage );
    nRealPos -= m_pImplLB->GetEntryList().GetMRUCount();
    CallEventListeners( VclEventId::ComboboxItemAdded, reinterpret_cast<void*>(static_cast<sal_IntPtr>(nRealPos)) );
    return nRealPos;
}

void ComboBox::RemoveEntryAt(sal_Int32 const nPos)
{
    const sal_Int32 nMRUCount = m_pImplLB->GetEntryList().GetMRUCount();
    assert(nPos >= 0 && nPos <= COMBOBOX_MAX_ENTRIES - nMRUCount);
    m_pImplLB->RemoveEntry( nPos + nMRUCount );
    CallEventListeners( VclEventId::ComboboxItemRemoved, reinterpret_cast<void*>(static_cast<sal_IntPtr>(nPos)) );
}

void ComboBox::Clear()
{
    if (!m_pImplLB)
        return;
    m_pImplLB->Clear();
    CallEventListeners( VclEventId::ComboboxItemRemoved, reinterpret_cast<void*>(sal_IntPtr(-1)) );
}

Image ComboBox::GetEntryImage( sal_Int32 nPos ) const
{
    if (m_pImplLB->GetEntryList().HasEntryImage(nPos))
        return m_pImplLB->GetEntryList().GetEntryImage( nPos );
    return Image();
}

sal_Int32 ComboBox::GetEntryPos( std::u16string_view rStr ) const
{
    sal_Int32 nPos = m_pImplLB->GetEntryList().FindEntry( rStr );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos -= m_pImplLB->GetEntryList().GetMRUCount();
    return nPos;
}

OUString ComboBox::GetEntry( sal_Int32 nPos ) const
{
    const sal_Int32 nMRUCount = m_pImplLB->GetEntryList().GetMRUCount();
    if (nPos < 0 || nPos > COMBOBOX_MAX_ENTRIES - nMRUCount)
        return OUString();

    return m_pImplLB->GetEntryList().GetEntryText( nPos + nMRUCount );
}

sal_Int32 ComboBox::GetEntryCount() const
{
    if (!m_pImplLB)
        return 0;
    return m_pImplLB->GetEntryList().GetEntryCount() - m_pImplLB->GetEntryList().GetMRUCount();
}

bool ComboBox::IsTravelSelect() const
{
    return m_pImplLB->IsTravelSelect();
}

bool ComboBox::IsInDropDown() const
{
    // when the dropdown is dismissed, first mbInPopupMode is set to false, and on the next event iteration then
    // mbPopupMode is set to false
    return m_pFloatWin && m_pFloatWin->IsInPopupMode() && m_pFloatWin->ImplIsInPrivatePopupMode();
}

bool ComboBox::IsMultiSelectionEnabled() const
{
    return m_pImplLB->IsMultiSelectionEnabled();
}

void ComboBox::SetSelectHdl(const Link<ComboBox&,void>& rLink) { m_SelectHdl = rLink; }

void ComboBox::SetEntryActivateHdl(const Link<Edit&,bool>& rLink)
{
    if (!m_pSubEdit)
        return;
    m_pSubEdit->SetActivateHdl(rLink);
}

Size ComboBox::GetOptimalSize() const
{
    return CalcMinimumSize();
}

tools::Long ComboBox::getMaxWidthScrollBarAndDownButton() const
{
    tools::Long nButtonDownWidth = 0;

    vcl::Window *pBorder = GetWindow( GetWindowType::Border );
    ImplControlValue aControlValue;
    tools::Rectangle aContent, aBound;

    // use the full extent of the control
    tools::Rectangle aArea( Point(), pBorder->GetOutputSizePixel() );

    if ( GetNativeControlRegion(ControlType::Combobox, ControlPart::ButtonDown,
        aArea, ControlState::NONE, aControlValue, aBound, aContent) )
    {
        nButtonDownWidth = aContent.getOpenWidth();
    }

    tools::Long nScrollBarWidth = GetSettings().GetStyleSettings().GetScrollBarSize();

    return std::max(nScrollBarWidth, nButtonDownWidth);
}

Size ComboBox::CalcMinimumSize() const
{
    Size aSz;

    if (!m_pImplLB)
        return aSz;

    if (!IsDropDownBox())
    {
        aSz = m_pImplLB->CalcSize( m_pImplLB->GetEntryList().GetEntryCount() );
        aSz.AdjustHeight(m_nDDHeight );
    }
    else
    {
        aSz.setHeight( Edit::CalcMinimumSizeForText(GetText()).Height() );

        if (m_nWidthInChars!= -1)
            aSz.setWidth(m_nWidthInChars * approximate_digit_width());
        else
            aSz.setWidth(m_pImplLB->GetMaxEntryWidth());
    }

    if (m_nMaxWidthChars != -1)
    {
        tools::Long nMaxWidth = m_nMaxWidthChars * approximate_char_width();
        aSz.setWidth( std::min(aSz.Width(), nMaxWidth) );
    }

    if (IsDropDownBox())
        aSz.AdjustWidth(getMaxWidthScrollBarAndDownButton() );

    ComboBoxBounds aBounds(calcComboBoxDropDownComponentBounds(
        Size(0xFFFF, 0xFFFF), Size(0xFFFF, 0xFFFF)));
    aSz.AdjustWidth(aBounds.aSubEditPos.X()*2 );

    aSz.AdjustWidth(ImplGetExtraXOffset() * 2 );

    aSz = CalcWindowSize( aSz );
    return aSz;
}

Size ComboBox::CalcAdjustedSize( const Size& rPrefSize ) const
{
    Size aSz = rPrefSize;
    sal_Int32 nLeft, nTop, nRight, nBottom;
    static_cast<vcl::Window*>(const_cast<ComboBox *>(this))->GetBorder( nLeft, nTop, nRight, nBottom );
    aSz.AdjustHeight( -(nTop+nBottom) );
    if ( !IsDropDownBox() )
    {
        tools::Long nEntryHeight = CalcBlockSize( 1, 1 ).Height();
        tools::Long nLines = aSz.Height() / nEntryHeight;
        if ( nLines < 1 )
            nLines = 1;
        aSz.setHeight( nLines * nEntryHeight );
        aSz.AdjustHeight(m_nDDHeight );
    }
    else
    {
        aSz.setHeight( m_nDDHeight );
    }
    aSz.AdjustHeight(nTop+nBottom );

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
            aSz.setHeight( m_pImplLB->CalcSize( nLines ).Height() + m_nDDHeight );
        else
            aSz.setHeight( m_nDDHeight );
    }
    else
        aSz.setHeight( aMinSz.Height() );

    // width
    if ( nColumns )
        aSz.setWidth( nColumns * approximate_char_width() );
    else
        aSz.setWidth( aMinSz.Width() );

    if ( IsDropDownBox() )
        aSz.AdjustWidth(getMaxWidthScrollBarAndDownButton() );

    if ( !IsDropDownBox() )
    {
        if ( aSz.Width() < aMinSz.Width() )
            aSz.AdjustHeight(GetSettings().GetStyleSettings().GetScrollBarSize() );
        if ( aSz.Height() < aMinSz.Height() )
            aSz.AdjustWidth(GetSettings().GetStyleSettings().GetScrollBarSize() );
    }

    aSz.AdjustWidth(ImplGetExtraXOffset() * 2 );

    aSz = CalcWindowSize( aSz );
    return aSz;
}

tools::Long ComboBox::GetDropDownEntryHeight() const
{
    return m_pImplLB->GetEntryHeight();
}

void ComboBox::GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const
{
    tools::Long nCharWidth = GetTextWidth(OUString(u'x'));
    if ( !IsDropDownBox() )
    {
        Size aOutSz = GetMainWindow()->GetOutputSizePixel();
        rnCols = (nCharWidth > 0) ? static_cast<sal_uInt16>(aOutSz.Width()/nCharWidth) : 1;
        rnLines = static_cast<sal_uInt16>(aOutSz.Height()/GetDropDownEntryHeight());
    }
    else
    {
        Size aOutSz = m_pSubEdit->GetOutputSizePixel();
        rnCols = (nCharWidth > 0) ? static_cast<sal_uInt16>(aOutSz.Width()/nCharWidth) : 1;
        rnLines = 1;
    }
}

void ComboBox::Draw( OutputDevice* pDev, const Point& rPos, SystemTextColorFlags nFlags )
{
    GetMainWindow()->ApplySettings(*pDev);

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = GetSizePixel();
    vcl::Font aFont = GetMainWindow()->GetDrawPixelFont( pDev );

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    bool bBorder = (GetStyle() & WB_BORDER);
    bool bBackground = IsControlBackground();
    if ( bBorder || bBackground )
    {
        tools::Rectangle aRect( aPos, aSize );
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
        tools::Long        nOnePixel = GetDrawPixel( pDev, 1 );
        tools::Long        nTextHeight = pDev->GetTextHeight();
        tools::Long        nEditHeight = nTextHeight + 6*nOnePixel;
        DrawTextFlags nTextStyle = DrawTextFlags::VCenter;

        // First, draw the edit part
        Size aOrigSize(m_pSubEdit->GetSizePixel());
        m_pSubEdit->SetSizePixel(Size(aSize.Width(), nEditHeight));
        m_pSubEdit->Draw( pDev, aPos, nFlags );
        m_pSubEdit->SetSizePixel(aOrigSize);

        // Second, draw the listbox
        if ( GetStyle() & WB_CENTER )
            nTextStyle |= DrawTextFlags::Center;
        else if ( GetStyle() & WB_RIGHT )
            nTextStyle |= DrawTextFlags::Right;
        else
            nTextStyle |= DrawTextFlags::Left;

        if ( nFlags & SystemTextColorFlags::Mono )
        {
            pDev->SetTextColor( COL_BLACK );
        }
        else
        {
            if ( !IsEnabled() )
            {
                const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
                pDev->SetTextColor( rStyleSettings.GetDisableColor() );
            }
            else
            {
                pDev->SetTextColor( GetTextColor() );
            }
        }

        tools::Rectangle aClip( aPos, aSize );
        pDev->IntersectClipRegion( aClip );
        sal_Int32 nLines = static_cast<sal_Int32>( nTextHeight > 0 ? (aSize.Height()-nEditHeight)/nTextHeight : 1 );
        if ( !nLines )
            nLines = 1;
        const sal_Int32 nTEntry = IsReallyVisible() ? m_pImplLB->GetTopEntry() : 0;

        tools::Rectangle aTextRect( aPos, aSize );

        aTextRect.AdjustLeft(3*nOnePixel );
        aTextRect.AdjustRight( -(3*nOnePixel) );
        aTextRect.AdjustTop(nEditHeight + nOnePixel );
        aTextRect.SetBottom( aTextRect.Top() + nTextHeight );

        // the drawing starts here
        for ( sal_Int32 n = 0; n < nLines; ++n )
        {
            pDev->DrawText( aTextRect, m_pImplLB->GetEntryList().GetEntryText( n+nTEntry ), nTextStyle );
            aTextRect.AdjustTop(nTextHeight );
            aTextRect.AdjustBottom(nTextHeight );
        }
    }

    pDev->Pop();

    // Call Edit::Draw after restoring the MapMode...
    if ( IsDropDownBox() )
    {
        Size aOrigSize(m_pSubEdit->GetSizePixel());
        m_pSubEdit->SetSizePixel(GetSizePixel());
        m_pSubEdit->Draw( pDev, rPos, nFlags );
        m_pSubEdit->SetSizePixel(aOrigSize);
        // DD-Button ?
    }
}

void ComboBox::SetUserDrawHdl(const Link<UserDrawEvent*, void>& rLink)
{
    m_pImplLB->SetUserDrawHdl(rLink);
}

void ComboBox::SetUserItemSize( const Size& rSz )
{
    GetMainWindow()->SetUserItemSize( rSz );
}

void ComboBox::EnableUserDraw( bool bUserDraw )
{
    GetMainWindow()->EnableUserDraw( bUserDraw );
}

bool ComboBox::IsUserDrawEnabled() const
{
    return GetMainWindow()->IsUserDrawEnabled();
}

void ComboBox::DrawEntry(const UserDrawEvent& rEvt)
{
    GetMainWindow()->DrawEntry(*rEvt.GetRenderContext(), rEvt.GetItemId(), /*bDrawImage*/false, /*bDrawText*/false);
}

void ComboBox::AddSeparator( sal_Int32 n )
{
    m_pImplLB->AddSeparator( n );
}

void ComboBox::SetMRUEntries( std::u16string_view rEntries )
{
    m_pImplLB->SetMRUEntries( rEntries, ';' );
}

OUString ComboBox::GetMRUEntries() const
{
    return m_pImplLB ? m_pImplLB->GetMRUEntries( ';' ) : OUString();
}

void ComboBox::SetMaxMRUCount( sal_Int32 n )
{
    m_pImplLB->SetMaxMRUCount( n );
}

sal_Int32 ComboBox::GetMaxMRUCount() const
{
    return m_pImplLB ? m_pImplLB->GetMaxMRUCount() : 0;
}

sal_uInt16 ComboBox::GetDisplayLineCount() const
{
    return m_pImplLB ? m_pImplLB->GetDisplayLineCount() : 0;
}

void ComboBox::SetEntryData( sal_Int32 nPos, void* pNewData )
{
    m_pImplLB->SetEntryData( nPos + m_pImplLB->GetEntryList().GetMRUCount(), pNewData );
}

void* ComboBox::GetEntryData( sal_Int32 nPos ) const
{
    return m_pImplLB->GetEntryList().GetEntryData(
            nPos + m_pImplLB->GetEntryList().GetMRUCount() );
}

sal_Int32 ComboBox::GetTopEntry() const
{
    sal_Int32 nPos = GetEntryCount() ? m_pImplLB->GetTopEntry() : LISTBOX_ENTRY_NOTFOUND;
    if (nPos < m_pImplLB->GetEntryList().GetMRUCount())
        nPos = 0;
    return nPos;
}

tools::Rectangle ComboBox::GetDropDownPosSizePixel() const
{
    return m_pFloatWin
        ? m_pFloatWin->GetWindowExtentsRelative(*this)
        : tools::Rectangle();
}

const Wallpaper& ComboBox::GetDisplayBackground() const
{
    if (!m_pSubEdit->IsBackground())
        return Control::GetDisplayBackground();

    const Wallpaper& rBack = m_pSubEdit->GetBackground();
    if( ! rBack.IsBitmap() &&
        ! rBack.IsGradient() &&
        rBack == Wallpaper(COL_TRANSPARENT)
        )
        return Control::GetDisplayBackground();
    return rBack;
}

sal_Int32 ComboBox::GetSelectedEntryCount() const
{
    return m_pImplLB->GetEntryList().GetSelectedEntryCount();
}

sal_Int32 ComboBox::GetSelectedEntryPos( sal_Int32 nIndex ) const
{
    sal_Int32 nPos = m_pImplLB->GetEntryList().GetSelectedEntryPos( nIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if (nPos < m_pImplLB->GetEntryList().GetMRUCount())
            nPos = m_pImplLB->GetEntryList().FindEntry(m_pImplLB->GetEntryList().GetEntryText(nPos));
        nPos = sal::static_int_cast<sal_Int32>(nPos - m_pImplLB->GetEntryList().GetMRUCount());
    }
    return nPos;
}

bool ComboBox::IsEntryPosSelected( sal_Int32 nPos ) const
{
    return m_pImplLB->GetEntryList().IsEntryPosSelected(
            nPos + m_pImplLB->GetEntryList().GetMRUCount() );
}

void ComboBox::SelectEntryPos( sal_Int32 nPos, bool bSelect)
{
    if (nPos < m_pImplLB->GetEntryList().GetEntryCount())
        m_pImplLB->SelectEntry(
            nPos + m_pImplLB->GetEntryList().GetMRUCount(), bSelect);
}

void ComboBox::SetNoSelection()
{
    m_pImplLB->SetNoSelection();
    m_pSubEdit->SetText(OUString());
}

tools::Rectangle ComboBox::GetBoundingRectangle( sal_Int32 nItem ) const
{
    tools::Rectangle aRect = GetMainWindow()->GetBoundingRectangle( nItem );
    tools::Rectangle aOffset = GetMainWindow()->GetWindowExtentsRelative( *static_cast<vcl::Window*>(const_cast<ComboBox *>(this)) );
    aRect.Move( aOffset.Left(), aOffset.Top() );
    return aRect;
}

void ComboBox::SetBorderStyle( WindowBorderStyle nBorderStyle )
{
    Window::SetBorderStyle( nBorderStyle );
    if ( !IsDropDownBox() )
    {
        m_pSubEdit->SetBorderStyle(nBorderStyle);
        m_pImplLB->SetBorderStyle( nBorderStyle );
    }
}

void ComboBox::SetHighlightColor( const Color& rColor )
{
    AllSettings aSettings(GetSettings());
    StyleSettings aStyle(aSettings.GetStyleSettings());
    aStyle.SetHighlightColor(rColor);
    aSettings.SetStyleSettings(aStyle);
    SetSettings(aSettings);

    AllSettings aSettingsSubEdit(m_pSubEdit->GetSettings());
    StyleSettings aStyleSubEdit(aSettingsSubEdit.GetStyleSettings());
    aStyleSubEdit.SetHighlightColor(rColor);
    aSettingsSubEdit.SetStyleSettings(aStyleSubEdit);
    m_pSubEdit->SetSettings(aSettings);

    m_pImplLB->SetHighlightColor(rColor);
}

void ComboBox::SetHighlightTextColor( const Color& rColor )
{
    AllSettings aSettings(GetSettings());
    StyleSettings aStyle(aSettings.GetStyleSettings());
    aStyle.SetHighlightTextColor(rColor);
    aSettings.SetStyleSettings(aStyle);
    SetSettings(aSettings);

    AllSettings aSettingsSubEdit(m_pSubEdit->GetSettings());
    StyleSettings aStyleSubEdit(aSettingsSubEdit.GetStyleSettings());
    aStyleSubEdit.SetHighlightTextColor(rColor);
    aSettingsSubEdit.SetStyleSettings(aStyleSubEdit);
    m_pSubEdit->SetSettings(aSettings);

    m_pImplLB->SetHighlightTextColor(rColor);
}

ImplListBoxWindow* ComboBox::GetMainWindow() const
{
    return m_pImplLB->GetMainWindow();
}

tools::Long ComboBox::GetIndexForPoint( const Point& rPoint, sal_Int32& rPos ) const
{
    if( !HasLayoutData() )
        FillLayoutData();

    // check whether rPoint fits at all
    tools::Long nIndex = Control::GetIndexForPoint( rPoint );
    if( nIndex != -1 )
    {
        // point must be either in main list window
        // or in impl window (dropdown case)
        ImplListBoxWindow* rMain = GetMainWindow();

        // convert coordinates to ImplListBoxWindow pixel coordinate space
        Point aConvPoint = LogicToPixel( rPoint );
        AbsoluteScreenPixelPoint aConvPointAbs = OutputToAbsoluteScreenPixel( aConvPoint );
        aConvPoint = rMain->AbsoluteScreenToOutputPixel( aConvPointAbs );
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

ComboBoxBounds ComboBox::calcComboBoxDropDownComponentBounds(
    const Size &rOutSz, const Size &rBorderOutSz) const
{
    ComboBoxBounds aBounds;

    tools::Long    nTop = 0;
    tools::Long    nBottom = rOutSz.Height();

    vcl::Window *pBorder = GetWindow(GetWindowType::Border);
    ImplControlValue aControlValue;
    Point aPoint;
    tools::Rectangle aContent, aBound;

    // use the full extent of the control
    tools::Rectangle aArea( aPoint, rBorderOutSz );

    if (GetNativeControlRegion(ControlType::Combobox, ControlPart::ButtonDown,
            aArea, ControlState::NONE, aControlValue, aBound, aContent) )
    {
        // convert back from border space to local coordinates
        aPoint = pBorder->ScreenToOutputPixel(OutputToScreenPixel(aPoint));
        aContent.Move(-aPoint.X(), -aPoint.Y());

        aBounds.aButtonPos = Point(aContent.Left(), nTop);
        aBounds.aButtonSize = Size(aContent.getOpenWidth(), (nBottom-nTop));

        // adjust the size of the edit field
        if (GetNativeControlRegion(ControlType::Combobox, ControlPart::SubEdit,
                    aArea, ControlState::NONE, aControlValue, aBound, aContent) )
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
            aBounds.aSubEditSize = Size(rOutSz.Width() - aContent.getOpenWidth(), rOutSz.Height());
        }
    }
    else
    {
        tools::Long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
        nSBWidth = CalcZoom( nSBWidth );
        aBounds.aSubEditSize = Size(rOutSz.Width() - nSBWidth, rOutSz.Height());
        aBounds.aButtonPos = Point(rOutSz.Width() - nSBWidth, nTop);
        aBounds.aButtonSize = Size(nSBWidth, (nBottom-nTop));
    }
    return aBounds;
}

void ComboBox::SetWidthInChars(sal_Int32 nWidthInChars)
{
    if (nWidthInChars != m_nWidthInChars)
    {
        m_nWidthInChars = nWidthInChars;
        queue_resize();
    }
}

void ComboBox::setMaxWidthChars(sal_Int32 nWidth)
{
    if (nWidth != m_nMaxWidthChars)
    {
        m_nMaxWidthChars = nWidth;
        queue_resize();
    }
}

bool ComboBox::set_property(const OUString &rKey, const OUString &rValue)
{
    if (rKey == "width-chars")
        SetWidthInChars(rValue.toInt32());
    else if (rKey == "max-width-chars")
        setMaxWidthChars(rValue.toInt32());
    else if (rKey == "can-focus")
    {
        // as far as I can see in Gtk, setting a ComboBox as can.focus means
        // the focus gets stuck in it, so try here to behave like gtk does
        // with the settings that work, i.e. can.focus of false doesn't
        // set the hard WB_NOTABSTOP
        WinBits nBits = GetStyle();
        nBits &= ~(WB_TABSTOP|WB_NOTABSTOP);
        if (toBool(rValue))
            nBits |= WB_TABSTOP;
        SetStyle(nBits);
    }
    else if (rKey == "placeholder-text")
        SetPlaceholderText(rValue);
    else
        return Control::set_property(rKey, rValue);
    return true;
}

FactoryFunction ComboBox::GetUITestFactory() const
{
    return ComboBoxUIObject::create;
}

void ComboBox::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    Control::DumpAsPropertyTree(rJsonWriter);

    {
        auto entriesNode = rJsonWriter.startArray("entries");
        for (int i = 0; i < GetEntryCount(); ++i)
        {
            rJsonWriter.putSimpleValue(GetEntry(i));
        }
    }

    {
        auto selectedNode = rJsonWriter.startArray("selectedEntries");
        for (int i = 0; i < GetSelectedEntryCount(); ++i)
        {
            rJsonWriter.putSimpleValue(OUString::number(GetSelectedEntryPos(i)));
        }
    }

    rJsonWriter.put("selectedCount", GetSelectedEntryCount());

    if (IsUserDrawEnabled())
        rJsonWriter.put("customEntryRenderer", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
