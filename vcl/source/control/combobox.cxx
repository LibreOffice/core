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

#include <vcl/combobox.hxx>

#include <set>

#include <comphelper/string.hxx>
#include <tools/debug.hxx>
#include <tools/rc.h>
#include <vcl/decoview.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/uitest/uiobject.hxx>

#include <svdata.hxx>
#include "listbox.hxx"
#include <controldata.hxx>


struct ComboBoxBounds
{
    Point aSubEditPos;
    Size aSubEditSize;

    Point aButtonPos;
    Size aButtonSize;
};

struct ComboBox::Impl
{
    ComboBox &          m_rThis;
    VclPtr<Edit>        m_pSubEdit;
    VclPtr<ImplListBox> m_pImplLB;
    VclPtr<ImplBtn>     m_pBtn;
    VclPtr<ImplListBoxFloatingWindow>  m_pFloatWin;
    sal_uInt16          m_nDDHeight;
    sal_Unicode         m_cMultiSep;
    bool                m_isDDAutoSize        : 1;
    bool                m_isSyntheticModify   : 1;
    bool                m_isMatchCase         : 1;
    sal_Int32           m_nMaxWidthChars;
    Link<ComboBox&,void>               m_SelectHdl;
    Link<ComboBox&,void>               m_DoubleClickHdl;

    explicit Impl(ComboBox & rThis)
        : m_rThis(rThis)
        , m_nDDHeight(0)
        , m_cMultiSep(0)
        , m_isDDAutoSize(false)
        , m_isSyntheticModify(false)
        , m_isMatchCase(false)
        , m_nMaxWidthChars(0)
    {
    }

    void ImplInitComboBoxData();
    void ImplUpdateFloatSelection();
    ComboBoxBounds calcComboBoxDropDownComponentBounds(
        const Size &rOutSize, const Size &rBorderOutSize) const;

    DECL_DLLPRIVATE_LINK_TYPED( ImplSelectHdl, LinkParamNone*, void );
    DECL_DLLPRIVATE_LINK_TYPED( ImplCancelHdl, LinkParamNone*, void );
    DECL_DLLPRIVATE_LINK_TYPED( ImplDoubleClickHdl, ImplListBoxWindow*, void );
    DECL_DLLPRIVATE_LINK_TYPED( ImplClickBtnHdl, void*, void );
    DECL_DLLPRIVATE_LINK_TYPED( ImplPopupModeEndHdl, FloatingWindow*, void );
    DECL_DLLPRIVATE_LINK_TYPED( ImplSelectionChangedHdl, sal_Int32, void );
    DECL_DLLPRIVATE_LINK_TYPED( ImplUserDrawHdl, UserDrawEvent*, void );
    DECL_DLLPRIVATE_LINK_TYPED( ImplAutocompleteHdl, Edit&, void );
    DECL_DLLPRIVATE_LINK_TYPED( ImplListItemSelectHdl , LinkParamNone*, void );
};


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

ComboBox::ComboBox(vcl::Window *const pParent, WinBits const nStyle)
    : Edit( WINDOW_COMBOBOX )
    , m_pImpl(new Impl(*this))
{
    m_pImpl->ImplInitComboBoxData();
    ImplInit( pParent, nStyle );
    SetWidthInChars(-1);
}

ComboBox::ComboBox(vcl::Window *const pParent, const ResId& rResId)
    : Edit( WINDOW_COMBOBOX )
    , m_pImpl(new Impl(*this))
{
    m_pImpl->ImplInitComboBoxData();
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
    m_pImpl->m_pSubEdit.disposeAndClear();

    VclPtr< ImplListBox > pImplLB = m_pImpl->m_pImplLB;
    m_pImpl->m_pImplLB.clear();
    pImplLB.disposeAndClear();

    m_pImpl->m_pFloatWin.disposeAndClear();
    m_pImpl->m_pBtn.disposeAndClear();
    Edit::dispose();
}

void ComboBox::Impl::ImplInitComboBoxData()
{
    m_pSubEdit.disposeAndClear();
    m_pBtn              = nullptr;
    m_pImplLB           = nullptr;
    m_pFloatWin         = nullptr;

    m_nDDHeight         = 0;
    m_isDDAutoSize      = true;
    m_isSyntheticModify = false;
    m_isMatchCase       = false;
    m_cMultiSep         = ';';
    m_nMaxWidthChars    = -1;
}

void ComboBox::ImplCalcEditHeight()
{
    sal_Int32 nLeft, nTop, nRight, nBottom;
    GetBorder( nLeft, nTop, nRight, nBottom );
    m_pImpl->m_nDDHeight = (sal_uInt16)(m_pImpl->m_pSubEdit->GetTextHeight() + nTop + nBottom + 4);
    if ( !IsDropDownBox() )
        m_pImpl->m_nDDHeight += 4;

    Rectangle aCtrlRegion( Point( 0, 0 ), Size( 10, 10 ) );
    Rectangle aBoundRegion, aContentRegion;
    ImplControlValue aControlValue;
    ControlType aType = IsDropDownBox() ? ControlType::Combobox : ControlType::Editbox;
    if( GetNativeControlRegion( aType, ControlPart::Entire,
                                aCtrlRegion,
                                ControlState::ENABLED,
                                aControlValue, OUString(),
                                aBoundRegion, aContentRegion ) )
    {
        const long nNCHeight = aBoundRegion.GetHeight();
        if (m_pImpl->m_nDDHeight < nNCHeight)
            m_pImpl->m_nDDHeight = sal::static_int_cast<sal_uInt16>(nNCHeight);
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
        m_pImpl->m_pFloatWin = VclPtr<ImplListBoxFloatingWindow>::Create( this );
        m_pImpl->m_pFloatWin->SetAutoWidth( true );
        m_pImpl->m_pFloatWin->SetPopupModeEndHdl( LINK(m_pImpl.get(), ComboBox::Impl, ImplPopupModeEndHdl) );

        m_pImpl->m_pBtn = VclPtr<ImplBtn>::Create( this, WB_NOLIGHTBORDER | WB_RECTSTYLE );
        ImplInitDropDownButton( m_pImpl->m_pBtn );
        m_pImpl->m_pBtn->SetMBDownHdl( LINK( m_pImpl.get(), ComboBox::Impl, ImplClickBtnHdl ) );
        m_pImpl->m_pBtn->Show();

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

    m_pImpl->m_pSubEdit.set( VclPtr<Edit>::Create( this, nEditStyle ) );
    m_pImpl->m_pSubEdit->EnableRTL( false );
    SetSubEdit( m_pImpl->m_pSubEdit );
    m_pImpl->m_pSubEdit->SetPosPixel( Point() );
    EnableAutocomplete( true );
    m_pImpl->m_pSubEdit->Show();

    vcl::Window* pLBParent = this;
    if (m_pImpl->m_pFloatWin)
        pLBParent = m_pImpl->m_pFloatWin;
    m_pImpl->m_pImplLB = VclPtr<ImplListBox>::Create( pLBParent, nListStyle|WB_SIMPLEMODE|WB_AUTOHSCROLL );
    m_pImpl->m_pImplLB->SetPosPixel( Point() );
    m_pImpl->m_pImplLB->SetSelectHdl( LINK(m_pImpl.get(), ComboBox::Impl, ImplSelectHdl) );
    m_pImpl->m_pImplLB->SetCancelHdl( LINK(m_pImpl.get(), ComboBox::Impl, ImplCancelHdl) );
    m_pImpl->m_pImplLB->SetDoubleClickHdl( LINK(m_pImpl.get(), ComboBox::Impl, ImplDoubleClickHdl) );
    m_pImpl->m_pImplLB->SetUserDrawHdl( LINK(m_pImpl.get(), ComboBox::Impl, ImplUserDrawHdl) );
    m_pImpl->m_pImplLB->SetSelectionChangedHdl( LINK(m_pImpl.get(), ComboBox::Impl, ImplSelectionChangedHdl) );
    m_pImpl->m_pImplLB->SetListItemSelectHdl( LINK(m_pImpl.get(), ComboBox::Impl, ImplListItemSelectHdl) );
    m_pImpl->m_pImplLB->Show();

    if (m_pImpl->m_pFloatWin)
        m_pImpl->m_pFloatWin->SetImplListBox( m_pImpl->m_pImplLB );
    else
        m_pImpl->m_pImplLB->GetMainWindow()->AllowGrabFocus( true );

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
            InsertEntry( ReadStringRes() );
        }
    }
}

void ComboBox::EnableAutocomplete( bool bEnable, bool bMatchCase )
{
    m_pImpl->m_isMatchCase = bMatchCase;

    if ( bEnable )
        m_pImpl->m_pSubEdit->SetAutocompleteHdl( LINK(m_pImpl.get(), ComboBox::Impl, ImplAutocompleteHdl) );
    else
        m_pImpl->m_pSubEdit->SetAutocompleteHdl( Link<Edit&,void>() );
}

bool ComboBox::IsAutocompleteEnabled() const
{
    return m_pImpl->m_pSubEdit->GetAutocompleteHdl().IsSet();
}

IMPL_LINK_NOARG_TYPED(ComboBox::Impl, ImplClickBtnHdl, void*, void)
{
    m_rThis.CallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
    m_pSubEdit->GrabFocus();
    if (!m_pImplLB->GetEntryList()->GetMRUCount())
        ImplUpdateFloatSelection();
    else
        m_pImplLB->SelectEntry( 0 , true );
    m_pBtn->SetPressed( true );
    m_rThis.SetSelection( Selection( 0, SELECTION_MAX ) );
    m_pFloatWin->StartFloat( true );
    m_rThis.CallEventListeners( VCLEVENT_DROPDOWN_OPEN );

    m_rThis.ImplClearLayoutData();
    if (m_pImplLB)
        m_pImplLB->GetMainWindow()->ImplClearLayoutData();
}

IMPL_LINK_NOARG_TYPED(ComboBox::Impl, ImplPopupModeEndHdl, FloatingWindow*, void)
{
    if (m_pFloatWin->IsPopupModeCanceled())
    {
        if (!m_pImplLB->GetEntryList()->IsEntryPosSelected(
                    m_pFloatWin->GetPopupModeStartSaveSelection()))
        {
            m_pImplLB->SelectEntry(m_pFloatWin->GetPopupModeStartSaveSelection(), true);
            bool bTravelSelect = m_pImplLB->IsTravelSelect();
            m_pImplLB->SetTravelSelect( true );
            m_rThis.Select();
            m_pImplLB->SetTravelSelect( bTravelSelect );
        }
    }

    m_rThis.ImplClearLayoutData();
    if (m_pImplLB)
        m_pImplLB->GetMainWindow()->ImplClearLayoutData();

    m_pBtn->SetPressed( false );
    m_rThis.CallEventListeners( VCLEVENT_DROPDOWN_CLOSE );
}

IMPL_LINK_TYPED(ComboBox::Impl, ImplAutocompleteHdl, Edit&, rEdit, void)
{
    Selection           aSel = rEdit.GetSelection();

    {
        OUString    aFullText = rEdit.GetText();
        OUString    aStartText = aFullText.copy( 0, (sal_Int32)aSel.Max() );
        sal_Int32   nStart = m_pImplLB->GetCurrentPos();

        if ( nStart == LISTBOX_ENTRY_NOTFOUND )
            nStart = 0;

        bool bForward = true;

        sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;
        if (!m_isMatchCase)
        {
            // Try match case insensitive from current position
            nPos = m_pImplLB->GetEntryList()->FindMatchingEntry( aStartText, nStart, bForward );
            if ( nPos == LISTBOX_ENTRY_NOTFOUND )
                // Try match case insensitive, but from start
                nPos = m_pImplLB->GetEntryList()->FindMatchingEntry( aStartText,
                    bForward ? 0 : (m_pImplLB->GetEntryList()->GetEntryCount()-1),
                    bForward );
        }

        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            // Try match full from current position
            nPos = m_pImplLB->GetEntryList()->FindMatchingEntry( aStartText, nStart, bForward, false );
        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            //  Match full, but from start
            nPos = m_pImplLB->GetEntryList()->FindMatchingEntry( aStartText,
                bForward ? 0 : (m_pImplLB->GetEntryList()->GetEntryCount()-1),
                bForward, false );

        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            OUString aText = m_pImplLB->GetEntryList()->GetEntryText( nPos );
            Selection aSelection( aText.getLength(), aStartText.getLength() );
            rEdit.SetText( aText, aSelection );
        }
    }
}

IMPL_LINK_NOARG_TYPED(ComboBox::Impl, ImplSelectHdl, LinkParamNone*, void)
{
    bool bPopup = m_rThis.IsInDropDown();
    bool bCallSelect = false;
    if (m_pImplLB->IsSelectionChanged() || bPopup)
    {
        OUString aText;
        if (m_rThis.IsMultiSelectionEnabled())
        {
            aText = m_pSubEdit->GetText();

            // remove all entries to which there is an entry, but which is not selected
            sal_Int32 nIndex = 0;
            while ( nIndex >= 0 )
            {
                sal_Int32  nPrevIndex = nIndex;
                OUString   aToken = aText.getToken( 0, m_cMultiSep, nIndex );
                sal_Int32  nTokenLen = aToken.getLength();
                aToken = comphelper::string::strip(aToken, ' ');
                sal_Int32 nP = m_pImplLB->GetEntryList()->FindEntry( aToken );
                if ((nP != LISTBOX_ENTRY_NOTFOUND) && (!m_pImplLB->GetEntryList()->IsEntryPosSelected(nP)))
                {
                    aText = aText.replaceAt( nPrevIndex, nTokenLen, "" );
                    nIndex = nIndex - nTokenLen;
                    sal_Int32 nSepCount=0;
                    if ((nPrevIndex+nSepCount < aText.getLength()) && (aText[nPrevIndex+nSepCount] == m_cMultiSep))
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
            lcl_GetSelectedEntries( aSelInText, aText, m_cMultiSep, m_pImplLB->GetEntryList() );
            sal_Int32 nSelectedEntries = m_pImplLB->GetEntryList()->GetSelectEntryCount();
            for ( sal_Int32 n = 0; n < nSelectedEntries; n++ )
            {
                sal_Int32 nP = m_pImplLB->GetEntryList()->GetSelectEntryPos( n );
                if ( !aSelInText.count( nP ) )
                {
                    if (!aText.isEmpty() && (aText[aText.getLength()-1] != m_cMultiSep))
                        aText += OUString(m_cMultiSep);
                    if ( !aText.isEmpty() )
                        aText += " ";   // slightly loosen
                    aText += m_pImplLB->GetEntryList()->GetEntryText( nP );
                    aText += OUString(m_cMultiSep);
                }
            }
            aText = comphelper::string::stripEnd( aText, m_cMultiSep );
        }
        else
        {
            aText = m_pImplLB->GetEntryList()->GetSelectEntry( 0 );
        }

        m_pSubEdit->SetText( aText );

        Selection aNewSelection( 0, aText.getLength() );
        if (m_rThis.IsMultiSelectionEnabled())
            aNewSelection.Min() = aText.getLength();
        m_pSubEdit->SetSelection( aNewSelection );

        bCallSelect = true;
    }

    // #84652# Call GrabFocus and EndPopupMode before calling Select/Modify, but after changing the text

    if (bPopup && !m_pImplLB->IsTravelSelect() &&
        (!m_rThis.IsMultiSelectionEnabled() || !m_pImplLB->GetSelectModifier()))
    {
        m_pFloatWin->EndPopupMode();
        m_rThis.GrabFocus();
    }

    if ( bCallSelect )
    {
        m_pSubEdit->SetModifyFlag();
        m_isSyntheticModify = true;
        m_rThis.Modify();
        m_isSyntheticModify = false;
        m_rThis.Select();
    }
}

IMPL_LINK_NOARG_TYPED( ComboBox::Impl, ImplListItemSelectHdl, LinkParamNone*, void )
{
    m_rThis.CallEventListeners( VCLEVENT_DROPDOWN_SELECT );
}

IMPL_LINK_NOARG_TYPED(ComboBox::Impl, ImplCancelHdl, LinkParamNone*, void)
{
    if (m_rThis.IsInDropDown())
        m_pFloatWin->EndPopupMode();
}

IMPL_LINK_TYPED( ComboBox::Impl, ImplSelectionChangedHdl, sal_Int32, nChanged, void )
{
    if (!m_pImplLB->IsTrackingSelect())
    {
        if (!m_pSubEdit->IsReadOnly() && m_pImplLB->GetEntryList()->IsEntryPosSelected(nChanged))
            m_pSubEdit->SetText(m_pImplLB->GetEntryList()->GetEntryText(nChanged));
    }
}

IMPL_LINK_NOARG_TYPED(ComboBox::Impl, ImplDoubleClickHdl, ImplListBoxWindow*, void)
{
    m_rThis.DoubleClick();
}

void ComboBox::ToggleDropDown()
{
    if( IsDropDownBox() )
    {
        if (m_pImpl->m_pFloatWin->IsInPopupMode())
            m_pImpl->m_pFloatWin->EndPopupMode();
        else
        {
            m_pImpl->m_pSubEdit->GrabFocus();
            if (!m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount())
                m_pImpl->ImplUpdateFloatSelection();
            else
                m_pImpl->m_pImplLB->SelectEntry( 0 , true );
            CallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
            m_pImpl->m_pBtn->SetPressed( true );
            SetSelection( Selection( 0, SELECTION_MAX ) );
            m_pImpl->m_pFloatWin->StartFloat( true );
            CallEventListeners( VCLEVENT_DROPDOWN_OPEN );
        }
    }
}

void ComboBox::Select()
{
    ImplCallEventListenersAndHandler( VCLEVENT_COMBOBOX_SELECT, [this] () { m_pImpl->m_SelectHdl.Call(*this); } );
}

void ComboBox::DoubleClick()
{
    ImplCallEventListenersAndHandler( VCLEVENT_COMBOBOX_DOUBLECLICK, [this] () { m_pImpl->m_DoubleClickHdl.Call(*this); } );
}

bool ComboBox::IsAutoSizeEnabled() const { return m_pImpl->m_isDDAutoSize; }

void ComboBox::EnableAutoSize( bool bAuto )
{
    m_pImpl->m_isDDAutoSize = bAuto;
    if (m_pImpl->m_pFloatWin)
    {
        if (bAuto && !m_pImpl->m_pFloatWin->GetDropDownLineCount())
        {
            // Adapt to GetListBoxMaximumLineCount here; was on fixed number of five before
            AdaptDropDownLineCountToMaximum();
        }
        else if ( !bAuto )
        {
            m_pImpl->m_pFloatWin->SetDropDownLineCount( 0 );
        }
    }
}

void ComboBox::EnableDDAutoWidth( bool b )
{
    if (m_pImpl->m_pFloatWin)
        m_pImpl->m_pFloatWin->SetAutoWidth( b );
}

void ComboBox::SetDropDownLineCount( sal_uInt16 nLines )
{
    if (m_pImpl->m_pFloatWin)
        m_pImpl->m_pFloatWin->SetDropDownLineCount( nLines );
}

void ComboBox::AdaptDropDownLineCountToMaximum()
{
    // adapt to maximum allowed number
    SetDropDownLineCount(GetSettings().GetStyleSettings().GetListBoxMaximumLineCount());
}

sal_uInt16 ComboBox::GetDropDownLineCount() const
{
    sal_uInt16 nLines = 0;
    if (m_pImpl->m_pFloatWin)
        nLines = m_pImpl->m_pFloatWin->GetDropDownLineCount();
    return nLines;
}

void ComboBox::setPosSizePixel( long nX, long nY, long nWidth, long nHeight,
                                PosSizeFlags nFlags )
{
    if( IsDropDownBox() && ( nFlags & PosSizeFlags::Size ) )
    {
        Size aPrefSz = m_pImpl->m_pFloatWin->GetPrefSize();
        if ((nFlags & PosSizeFlags::Height) && (nHeight >= 2*m_pImpl->m_nDDHeight))
            aPrefSz.Height() = nHeight-m_pImpl->m_nDDHeight;
        if ( nFlags & PosSizeFlags::Width )
            aPrefSz.Width() = nWidth;
        m_pImpl->m_pFloatWin->SetPrefSize( aPrefSz );

        if (IsAutoSizeEnabled())
            nHeight = m_pImpl->m_nDDHeight;
    }

    Edit::setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

void ComboBox::Resize()
{
    Control::Resize();

    if (m_pImpl->m_pSubEdit)
    {
        Size aOutSz = GetOutputSizePixel();
        if( IsDropDownBox() )
        {
            ComboBoxBounds aBounds(m_pImpl->calcComboBoxDropDownComponentBounds(aOutSz,
                GetWindow(GetWindowType::Border)->GetOutputSizePixel()));
            m_pImpl->m_pSubEdit->SetPosSizePixel(aBounds.aSubEditPos, aBounds.aSubEditSize);
            m_pImpl->m_pBtn->SetPosSizePixel(aBounds.aButtonPos, aBounds.aButtonSize);
        }
        else
        {
            m_pImpl->m_pSubEdit->SetSizePixel(Size(aOutSz.Width(), m_pImpl->m_nDDHeight));
            m_pImpl->m_pImplLB->setPosSizePixel(0, m_pImpl->m_nDDHeight,
                    aOutSz.Width(), aOutSz.Height() - m_pImpl->m_nDDHeight);
            if ( !GetText().isEmpty() )
                m_pImpl->ImplUpdateFloatSelection();
        }
    }

    // adjust the size of the FloatingWindow even when invisible
    // as KEY_PGUP/DOWN is being processed...
    if (m_pImpl->m_pFloatWin)
        m_pImpl->m_pFloatWin->SetSizePixel(m_pImpl->m_pFloatWin->CalcFloatSize());
}

bool ComboBox::IsDropDownBox() const { return m_pImpl->m_pFloatWin != nullptr; }

void ComboBox::FillLayoutData() const
{
    mpControlData->mpLayoutData = new vcl::ControlLayoutData();
    AppendLayoutData( *m_pImpl->m_pSubEdit );
    m_pImpl->m_pSubEdit->SetLayoutDataParent( this );
    ImplListBoxWindow* rMainWindow = m_pImpl->m_pImplLB->GetMainWindow();
    if (m_pImpl->m_pFloatWin)
    {
        // dropdown mode
        if (m_pImpl->m_pFloatWin->IsReallyVisible())
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
        m_pImpl->m_pImplLB->SetReadOnly( IsReadOnly() );
        if (m_pImpl->m_pBtn)
            m_pImpl->m_pBtn->Enable( IsEnabled() && !IsReadOnly() );
    }
    else if ( nType == StateChangedType::Enable )
    {
        m_pImpl->m_pSubEdit->Enable( IsEnabled() );
        m_pImpl->m_pImplLB->Enable( IsEnabled() && !IsReadOnly() );
        if (m_pImpl->m_pBtn)
            m_pImpl->m_pBtn->Enable( IsEnabled() && !IsReadOnly() );
        Invalidate();
    }
    else if( nType == StateChangedType::UpdateMode )
    {
        m_pImpl->m_pImplLB->SetUpdateMode( IsUpdateMode() );
    }
    else if ( nType == StateChangedType::Zoom )
    {
        m_pImpl->m_pImplLB->SetZoom( GetZoom() );
        m_pImpl->m_pSubEdit->SetZoom( GetZoom() );
        ImplCalcEditHeight();
        Resize();
    }
    else if ( nType == StateChangedType::ControlFont )
    {
        m_pImpl->m_pImplLB->SetControlFont( GetControlFont() );
        m_pImpl->m_pSubEdit->SetControlFont( GetControlFont() );
        ImplCalcEditHeight();
        Resize();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        m_pImpl->m_pImplLB->SetControlForeground( GetControlForeground() );
        m_pImpl->m_pSubEdit->SetControlForeground( GetControlForeground() );
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        m_pImpl->m_pImplLB->SetControlBackground( GetControlBackground() );
        m_pImpl->m_pSubEdit->SetControlBackground( GetControlBackground() );
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        m_pImpl->m_pImplLB->GetMainWindow()->EnableSort( ( GetStyle() & WB_SORT ) != 0 );
    }
    else if( nType == StateChangedType::Mirroring )
    {
        if (m_pImpl->m_pBtn)
        {
            m_pImpl->m_pBtn->EnableRTL( IsRTLEnabled() );
            ImplInitDropDownButton( m_pImpl->m_pBtn );
        }
        m_pImpl->m_pSubEdit->CompatStateChanged( StateChangedType::Mirroring );
        m_pImpl->m_pImplLB->EnableRTL( IsRTLEnabled() );
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
        if (m_pImpl->m_pBtn)
        {
            m_pImpl->m_pBtn->SetSettings( GetSettings() );
            ImplInitDropDownButton( m_pImpl->m_pBtn );
        }
        Resize();
        m_pImpl->m_pImplLB->Resize(); // not called by ComboBox::Resize() if ImplLB is unchanged

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
    bool bDone = false;
    if ((rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
        && (rNEvt.GetWindow() == m_pImpl->m_pSubEdit)
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
                m_pImpl->ImplUpdateFloatSelection();
                if ((nKeyCode == KEY_DOWN) && m_pImpl->m_pFloatWin
                    && !m_pImpl->m_pFloatWin->IsInPopupMode()
                    && aKeyEvt.GetKeyCode().IsMod2())
                {
                    CallEventListeners( VCLEVENT_DROPDOWN_PRE_OPEN );
                    m_pImpl->m_pBtn->SetPressed( true );
                    if (m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount())
                        m_pImpl->m_pImplLB->SelectEntry( 0 , true );
                    SetSelection( Selection( 0, SELECTION_MAX ) );
                    m_pImpl->m_pFloatWin->StartFloat( false );
                    CallEventListeners( VCLEVENT_DROPDOWN_OPEN );
                    bDone = true;
                }
                else if ((nKeyCode == KEY_UP) && m_pImpl->m_pFloatWin
                        && m_pImpl->m_pFloatWin->IsInPopupMode()
                        && aKeyEvt.GetKeyCode().IsMod2())
                {
                    m_pImpl->m_pFloatWin->EndPopupMode();
                    bDone = true;
                }
                else
                {
                    bDone = m_pImpl->m_pImplLB->ProcessKeyInput( aKeyEvt );
                }
            }
            break;

            case KEY_RETURN:
            {
                if ((rNEvt.GetWindow() == m_pImpl->m_pSubEdit) && IsInDropDown())
                {
                    m_pImpl->m_pImplLB->ProcessKeyInput( aKeyEvt );
                    bDone = true;
                }
            }
            break;
        }
    }
    else if ((rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS) && m_pImpl->m_pFloatWin)
    {
        if (m_pImpl->m_pFloatWin->HasChildPathFocus())
            m_pImpl->m_pSubEdit->GrabFocus();
        else if (m_pImpl->m_pFloatWin->IsInPopupMode() && !HasChildPathFocus(true))
            m_pImpl->m_pFloatWin->EndPopupMode();
    }
    else if( (rNEvt.GetType() == MouseNotifyEvent::COMMAND) &&
             (rNEvt.GetCommandEvent()->GetCommand() == CommandEventId::Wheel) &&
             (rNEvt.GetWindow() == m_pImpl->m_pSubEdit) )
    {
        MouseWheelBehaviour nWheelBehavior( GetSettings().GetMouseSettings().GetWheelBehavior() );
        if  (   ( nWheelBehavior == MouseWheelBehaviour::ALWAYS )
            ||  (   ( nWheelBehavior == MouseWheelBehaviour::FocusOnly )
                &&  HasChildPathFocus()
                )
            )
        {
            bDone = m_pImpl->m_pImplLB->HandleWheelAsCursorTravel( *rNEvt.GetCommandEvent() );
        }
        else
        {
            bDone = false;  // don't eat this event, let the default handling happen (i.e. scroll the context)
        }
    }
    else if ((rNEvt.GetType() == MouseNotifyEvent::MOUSEBUTTONDOWN)
            && (rNEvt.GetWindow() == m_pImpl->m_pImplLB->GetMainWindow()))
    {
        m_pImpl->m_pSubEdit->GrabFocus();
    }

    return bDone || Edit::Notify( rNEvt );
}

void ComboBox::SetText( const OUString& rStr )
{
    CallEventListeners( VCLEVENT_COMBOBOX_SETTEXT );

    Edit::SetText( rStr );
    m_pImpl->ImplUpdateFloatSelection();
}

void ComboBox::SetText( const OUString& rStr, const Selection& rNewSelection )
{
    CallEventListeners( VCLEVENT_COMBOBOX_SETTEXT );

    Edit::SetText( rStr, rNewSelection );
    m_pImpl->ImplUpdateFloatSelection();
}

void ComboBox::Modify()
{
    if (!m_pImpl->m_isSyntheticModify)
        m_pImpl->ImplUpdateFloatSelection();

    Edit::Modify();
}

void ComboBox::Impl::ImplUpdateFloatSelection()
{
    if (!m_pImplLB || !m_pSubEdit)
        return;

    // move text in the ListBox into the visible region
    m_pImplLB->SetCallSelectionChangedHdl( false );
    if (!m_rThis.IsMultiSelectionEnabled())
    {
        OUString        aSearchStr( m_pSubEdit->GetText() );
        sal_Int32       nSelect = LISTBOX_ENTRY_NOTFOUND;
        bool        bSelect = true;

        if (m_pImplLB->GetCurrentPos() != LISTBOX_ENTRY_NOTFOUND)
        {
            OUString aCurrent = m_pImplLB->GetEntryList()->GetEntryText(
                                    m_pImplLB->GetCurrentPos());
            if ( aCurrent == aSearchStr )
                nSelect = m_pImplLB->GetCurrentPos();
        }

        if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
            nSelect = m_pImplLB->GetEntryList()->FindEntry( aSearchStr );
        if ( nSelect == LISTBOX_ENTRY_NOTFOUND )
        {
            nSelect = m_pImplLB->GetEntryList()->FindMatchingEntry( aSearchStr );
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
            nSelect = m_pImplLB->GetEntryList()->GetSelectEntryPos( 0 );
            if( nSelect != LISTBOX_ENTRY_NOTFOUND )
                m_pImplLB->SelectEntry( nSelect, false );
            m_pImplLB->ResetCurrentPos();
        }
    }
    else
    {
        ::std::set< sal_Int32 > aSelInText;
        lcl_GetSelectedEntries(aSelInText, m_pSubEdit->GetText(), m_cMultiSep, m_pImplLB->GetEntryList());
        for (sal_Int32 n = 0; n < m_pImplLB->GetEntryList()->GetEntryCount(); n++)
            m_pImplLB->SelectEntry( n, aSelInText.count( n ) );
    }
    m_pImplLB->SetCallSelectionChangedHdl( true );
}

sal_Int32 ComboBox::InsertEntry(const OUString& rStr, sal_Int32 const nPos)
{
    assert(nPos >= 0 && COMBOBOX_MAX_ENTRIES > m_pImpl->m_pImplLB->GetEntryList()->GetEntryCount());

    sal_Int32 nRealPos;
    if (nPos == COMBOBOX_APPEND)
        nRealPos = nPos;
    else
    {
        const sal_Int32 nMRUCount = m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount();
        assert(nPos <= COMBOBOX_MAX_ENTRIES - nMRUCount);
        nRealPos = nPos + nMRUCount;
    }

    nRealPos = m_pImpl->m_pImplLB->InsertEntry( nRealPos, rStr );
    nRealPos -= m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount();
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMADDED, reinterpret_cast<void*>(nRealPos) );
    return nRealPos;
}

sal_Int32 ComboBox::InsertEntryWithImage(
        const OUString& rStr, const Image& rImage, sal_Int32 const nPos)
{
    assert(nPos >= 0 && COMBOBOX_MAX_ENTRIES > m_pImpl->m_pImplLB->GetEntryList()->GetEntryCount());

    sal_Int32 nRealPos;
    if (nPos == COMBOBOX_APPEND)
        nRealPos = nPos;
    else
    {
        const sal_Int32 nMRUCount = m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount();
        assert(nPos <= COMBOBOX_MAX_ENTRIES - nMRUCount);
        nRealPos = nPos + nMRUCount;
    }

    nRealPos = m_pImpl->m_pImplLB->InsertEntry( nRealPos, rStr, rImage );
    nRealPos -= m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount();
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMADDED, reinterpret_cast<void*>(nRealPos) );
    return nRealPos;
}

void ComboBox::RemoveEntry( const OUString& rStr )
{
    RemoveEntryAt(GetEntryPos(rStr));
}

void ComboBox::RemoveEntryAt(sal_Int32 const nPos)
{
    const sal_Int32 nMRUCount = m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount();
    if (nPos < 0 || nPos > COMBOBOX_MAX_ENTRIES - nMRUCount)
        return;

    m_pImpl->m_pImplLB->RemoveEntry( nPos + nMRUCount );
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMREMOVED, reinterpret_cast<void*>(nPos) );
}

void ComboBox::Clear()
{
    m_pImpl->m_pImplLB->Clear();
    CallEventListeners( VCLEVENT_COMBOBOX_ITEMREMOVED, reinterpret_cast<void*>(-1) );
}

Image ComboBox::GetEntryImage( sal_Int32 nPos ) const
{
    if (m_pImpl->m_pImplLB->GetEntryList()->HasEntryImage(nPos))
        return m_pImpl->m_pImplLB->GetEntryList()->GetEntryImage( nPos );
    return Image();
}

sal_Int32 ComboBox::GetEntryPos( const OUString& rStr ) const
{
    sal_Int32 nPos = m_pImpl->m_pImplLB->GetEntryList()->FindEntry( rStr );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos -= m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount();
    return nPos;
}

sal_Int32 ComboBox::GetEntryPos( const void* pData ) const
{
    sal_Int32 nPos = m_pImpl->m_pImplLB->GetEntryList()->FindEntry( pData );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        nPos = nPos - m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount();
    return nPos;
}

OUString ComboBox::GetEntry( sal_Int32 nPos ) const
{
    const sal_Int32 nMRUCount = m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount();
    if (nPos < 0 || nPos > COMBOBOX_MAX_ENTRIES - nMRUCount)
        return OUString();

    return m_pImpl->m_pImplLB->GetEntryList()->GetEntryText( nPos + nMRUCount );
}

sal_Int32 ComboBox::GetEntryCount() const
{
    return m_pImpl->m_pImplLB->GetEntryList()->GetEntryCount() - m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount();
}

bool ComboBox::IsTravelSelect() const
{
    return m_pImpl->m_pImplLB->IsTravelSelect();
}

bool ComboBox::IsInDropDown() const
{
    return m_pImpl->m_pFloatWin && m_pImpl->m_pFloatWin->IsInPopupMode();
}

void ComboBox::EnableMultiSelection( bool bMulti )
{
    m_pImpl->m_pImplLB->EnableMultiSelection( bMulti, false );
    m_pImpl->m_pImplLB->SetMultiSelectionSimpleMode( true );
}

bool ComboBox::IsMultiSelectionEnabled() const
{
    return m_pImpl->m_pImplLB->IsMultiSelectionEnabled();
}

void ComboBox::SetSelectHdl(const Link<ComboBox&,void>& rLink) { m_pImpl->m_SelectHdl = rLink; }

const Link<ComboBox&,void>& ComboBox::GetSelectHdl() const { return m_pImpl->m_SelectHdl; }

void ComboBox::SetDoubleClickHdl(const Link<ComboBox&,void>& rLink) { m_pImpl->m_DoubleClickHdl = rLink; }

const Link<ComboBox&,void>& ComboBox::GetDoubleClickHdl() const { return m_pImpl->m_DoubleClickHdl; }

long ComboBox::CalcWindowSizePixel( sal_uInt16 nLines ) const
{
    return m_pImpl->m_pImplLB->GetEntryHeight() * nLines;
}

Size ComboBox::GetOptimalSize() const
{
    return CalcMinimumSize();
}

long ComboBox::getMaxWidthScrollBarAndDownButton() const
{
    long nButtonDownWidth = 0;

    vcl::Window *pBorder = GetWindow( GetWindowType::Border );
    ImplControlValue aControlValue;
    Point aPoint;
    Rectangle aContent, aBound;

    // use the full extent of the control
    Rectangle aArea( aPoint, pBorder->GetOutputSizePixel() );

    if ( GetNativeControlRegion(ControlType::Combobox, ControlPart::ButtonDown,
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

    if (!m_pImpl->m_pImplLB)
        return aSz;

    if (!IsDropDownBox())
    {
        aSz = m_pImpl->m_pImplLB->CalcSize( m_pImpl->m_pImplLB->GetEntryList()->GetEntryCount() );
        aSz.Height() += m_pImpl->m_nDDHeight;
    }
    else
    {
        aSz.Height() = Edit::CalcMinimumSizeForText(GetText()).Height();
        aSz.Width() = m_pImpl->m_pImplLB->GetMaxEntryWidth();
    }

    if (m_pImpl->m_nMaxWidthChars != -1)
    {
        long nMaxWidth = m_pImpl->m_nMaxWidthChars * approximate_char_width();
        aSz.Width() = std::min(aSz.Width(), nMaxWidth);
    }

    if (IsDropDownBox())
        aSz.Width() += getMaxWidthScrollBarAndDownButton();

    ComboBoxBounds aBounds(m_pImpl->calcComboBoxDropDownComponentBounds(
        Size(0xFFFF, 0xFFFF), Size(0xFFFF, 0xFFFF)));
    aSz.Width() += aBounds.aSubEditPos.X()*2;

    aSz.Width() += ImplGetExtraXOffset() * 2;

    aSz = CalcWindowSize( aSz );
    return aSz;
}

Size ComboBox::CalcAdjustedSize( const Size& rPrefSize ) const
{
    Size aSz = rPrefSize;
    sal_Int32 nLeft, nTop, nRight, nBottom;
    static_cast<vcl::Window*>(const_cast<ComboBox *>(this))->GetBorder( nLeft, nTop, nRight, nBottom );
    aSz.Height() -= nTop+nBottom;
    if ( !IsDropDownBox() )
    {
        long nEntryHeight = CalcBlockSize( 1, 1 ).Height();
        long nLines = aSz.Height() / nEntryHeight;
        if ( nLines < 1 )
            nLines = 1;
        aSz.Height() = nLines * nEntryHeight;
        aSz.Height() += m_pImpl->m_nDDHeight;
    }
    else
    {
        aSz.Height() = m_pImpl->m_nDDHeight;
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
            aSz.Height() = m_pImpl->m_pImplLB->CalcSize( nLines ).Height() + m_pImpl->m_nDDHeight;
        else
            aSz.Height() = m_pImpl->m_nDDHeight;
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

    aSz.Width() += ImplGetExtraXOffset() * 2;

    aSz = CalcWindowSize( aSz );
    return aSz;
}

void ComboBox::GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const
{
    long nCharWidth = GetTextWidth(OUString(static_cast<sal_Unicode>('x')));
    if ( !IsDropDownBox() )
    {
        Size aOutSz = m_pImpl->m_pImplLB->GetMainWindow()->GetOutputSizePixel();
        rnCols = (nCharWidth > 0) ? (sal_uInt16)(aOutSz.Width()/nCharWidth) : 1;
        rnLines = (sal_uInt16)(aOutSz.Height()/m_pImpl->m_pImplLB->GetEntryHeight());
    }
    else
    {
        Size aOutSz = m_pImpl->m_pSubEdit->GetOutputSizePixel();
        rnCols = (nCharWidth > 0) ? (sal_uInt16)(aOutSz.Width()/nCharWidth) : 1;
        rnLines = 1;
    }
}

void ComboBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags )
{
    m_pImpl->m_pImplLB->GetMainWindow()->ApplySettings(*pDev);

    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    vcl::Font aFont = m_pImpl->m_pImplLB->GetMainWindow()->GetDrawPixelFont( pDev );
    OutDevType eOutDevType = pDev->GetOutDevType();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    pDev->SetTextFillColor();

    // Border/Background
    pDev->SetLineColor();
    pDev->SetFillColor();
    bool bBorder = !(nFlags & DrawFlags::NoBorder ) && (GetStyle() & WB_BORDER);
    bool bBackground = !(nFlags & DrawFlags::NoBackground) && IsControlBackground();
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
        DrawTextFlags nTextStyle = DrawTextFlags::VCenter;

        // First, draw the edit part
        m_pImpl->m_pSubEdit->Draw( pDev, aPos, Size( aSize.Width(), nEditHeight ), nFlags );

        // Second, draw the listbox
        if ( GetStyle() & WB_CENTER )
            nTextStyle |= DrawTextFlags::Center;
        else if ( GetStyle() & WB_RIGHT )
            nTextStyle |= DrawTextFlags::Right;
        else
            nTextStyle |= DrawTextFlags::Left;

        if ( ( nFlags & DrawFlags::Mono ) || ( eOutDevType == OUTDEV_PRINTER ) )
        {
            pDev->SetTextColor( Color( COL_BLACK ) );
        }
        else
        {
            if ( !(nFlags & DrawFlags::NoDisable ) && !IsEnabled() )
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
        sal_Int32 nLines = static_cast<sal_Int32>( nTextHeight > 0 ? (aSize.Height()-nEditHeight)/nTextHeight : 1 );
        if ( !nLines )
            nLines = 1;
        const sal_Int32 nTEntry = IsReallyVisible() ? m_pImpl->m_pImplLB->GetTopEntry() : 0;

        Rectangle aTextRect( aPos, aSize );

        aTextRect.Left() += 3*nOnePixel;
        aTextRect.Right() -= 3*nOnePixel;
        aTextRect.Top() += nEditHeight + nOnePixel;
        aTextRect.Bottom() = aTextRect.Top() + nTextHeight;

        // the drawing starts here
        for ( sal_Int32 n = 0; n < nLines; ++n )
        {
            pDev->DrawText( aTextRect, m_pImpl->m_pImplLB->GetEntryList()->GetEntryText( n+nTEntry ), nTextStyle );
            aTextRect.Top() += nTextHeight;
            aTextRect.Bottom() += nTextHeight;
        }
    }

    pDev->Pop();

    // Call Edit::Draw after restoring the MapMode...
    if ( IsDropDownBox() )
    {
        m_pImpl->m_pSubEdit->Draw( pDev, rPos, rSize, nFlags );
        // DD-Button ?
    }

}

IMPL_LINK_TYPED(ComboBox::Impl, ImplUserDrawHdl, UserDrawEvent*, pEvent, void)
{
    m_rThis.UserDraw(*pEvent);
}

void ComboBox::UserDraw( const UserDrawEvent& )
{
}

void ComboBox::SetUserItemSize( const Size& rSz )
{
    m_pImpl->m_pImplLB->GetMainWindow()->SetUserItemSize( rSz );
}

void ComboBox::EnableUserDraw( bool bUserDraw )
{
    m_pImpl->m_pImplLB->GetMainWindow()->EnableUserDraw( bUserDraw );
}

void ComboBox::DrawEntry(const UserDrawEvent& rEvt, bool bDrawImage, bool bDrawText, bool bDrawTextAtImagePos)
{
    SAL_WARN_IF(rEvt.GetWindow() != m_pImpl->m_pImplLB->GetMainWindow(), "vcl", "DrawEntry?!");
    m_pImpl->m_pImplLB->GetMainWindow()->DrawEntry(*rEvt.GetRenderContext(), rEvt.GetItemId(), bDrawImage, bDrawText, bDrawTextAtImagePos);
}

void ComboBox::SetSeparatorPos( sal_Int32 n )
{
    m_pImpl->m_pImplLB->SetSeparatorPos( n );
}

void ComboBox::SetMRUEntries( const OUString& rEntries )
{
    m_pImpl->m_pImplLB->SetMRUEntries( rEntries, ';' );
}

OUString ComboBox::GetMRUEntries() const
{
    return m_pImpl->m_pImplLB ? m_pImpl->m_pImplLB->GetMRUEntries( ';' ) : OUString();
}

void ComboBox::SetMaxMRUCount( sal_Int32 n )
{
    m_pImpl->m_pImplLB->SetMaxMRUCount( n );
}

sal_Int32 ComboBox::GetMaxMRUCount() const
{
    return m_pImpl->m_pImplLB ? m_pImpl->m_pImplLB->GetMaxMRUCount() : 0;
}

sal_uInt16 ComboBox::GetDisplayLineCount() const
{
    return m_pImpl->m_pImplLB ? m_pImpl->m_pImplLB->GetDisplayLineCount() : 0;
}

void ComboBox::SetEntryData( sal_Int32 nPos, void* pNewData )
{
    m_pImpl->m_pImplLB->SetEntryData( nPos + m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount(), pNewData );
}

void* ComboBox::GetEntryData( sal_Int32 nPos ) const
{
    return m_pImpl->m_pImplLB->GetEntryList()->GetEntryData(
            nPos + m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount() );
}

sal_Int32 ComboBox::GetTopEntry() const
{
    sal_Int32 nPos = GetEntryCount() ? m_pImpl->m_pImplLB->GetTopEntry() : LISTBOX_ENTRY_NOTFOUND;
    if (nPos < m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount())
        nPos = 0;
    return nPos;
}

void ComboBox::SetProminentEntryType( ProminentEntry eType )
{
    m_pImpl->m_pImplLB->SetProminentEntryType( eType );
}

Rectangle ComboBox::GetDropDownPosSizePixel() const
{
    return m_pImpl->m_pFloatWin
        ? m_pImpl->m_pFloatWin->GetWindowExtentsRelative(const_cast<ComboBox*>(this))
        : Rectangle();
}

const Wallpaper& ComboBox::GetDisplayBackground() const
{
    if (!m_pImpl->m_pSubEdit->IsBackground())
        return Control::GetDisplayBackground();

    const Wallpaper& rBack = m_pImpl->m_pSubEdit->GetBackground();
    if( ! rBack.IsBitmap() &&
        ! rBack.IsGradient() &&
        rBack.GetColor().GetColor() == COL_TRANSPARENT
        )
        return Control::GetDisplayBackground();
    return rBack;
}

sal_Int32 ComboBox::GetSelectEntryCount() const
{
    return m_pImpl->m_pImplLB->GetEntryList()->GetSelectEntryCount();
}

sal_Int32 ComboBox::GetSelectEntryPos( sal_Int32 nIndex ) const
{
    sal_Int32 nPos = m_pImpl->m_pImplLB->GetEntryList()->GetSelectEntryPos( nIndex );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if (nPos < m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount())
            nPos = m_pImpl->m_pImplLB->GetEntryList()->FindEntry(m_pImpl->m_pImplLB->GetEntryList()->GetEntryText(nPos));
        nPos = sal::static_int_cast<sal_Int32>(nPos - m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount());
    }
    return nPos;
}

bool ComboBox::IsEntryPosSelected( sal_Int32 nPos ) const
{
    return m_pImpl->m_pImplLB->GetEntryList()->IsEntryPosSelected(
            nPos + m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount() );
}

void ComboBox::SelectEntryPos( sal_Int32 nPos, bool bSelect)
{
    if (nPos < m_pImpl->m_pImplLB->GetEntryList()->GetEntryCount())
        m_pImpl->m_pImplLB->SelectEntry(
            nPos + m_pImpl->m_pImplLB->GetEntryList()->GetMRUCount(), bSelect);
}

void ComboBox::SetNoSelection()
{
    m_pImpl->m_pImplLB->SetNoSelection();
    m_pImpl->m_pSubEdit->SetText( OUString() );
}

Rectangle ComboBox::GetBoundingRectangle( sal_Int32 nItem ) const
{
    Rectangle aRect = m_pImpl->m_pImplLB->GetMainWindow()->GetBoundingRectangle( nItem );
    Rectangle aOffset = m_pImpl->m_pImplLB->GetMainWindow()->GetWindowExtentsRelative( static_cast<vcl::Window*>(const_cast<ComboBox *>(this)) );
    aRect.Move( aOffset.TopLeft().X(), aOffset.TopLeft().Y() );
    return aRect;
}

void ComboBox::SetBorderStyle( WindowBorderStyle nBorderStyle )
{
    Window::SetBorderStyle( nBorderStyle );
    if ( !IsDropDownBox() )
    {
        m_pImpl->m_pSubEdit->SetBorderStyle( nBorderStyle );
        m_pImpl->m_pImplLB->SetBorderStyle( nBorderStyle );
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
        ImplListBoxWindow* rMain = m_pImpl->m_pImplLB->GetMainWindow();

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

ComboBoxBounds ComboBox::Impl::calcComboBoxDropDownComponentBounds(
    const Size &rOutSz, const Size &rBorderOutSz) const
{
    ComboBoxBounds aBounds;

    long    nTop = 0;
    long    nBottom = rOutSz.Height();

    vcl::Window *pBorder = m_rThis.GetWindow( GetWindowType::Border );
    ImplControlValue aControlValue;
    Point aPoint;
    Rectangle aContent, aBound;

    // use the full extent of the control
    Rectangle aArea( aPoint, rBorderOutSz );

    if (m_rThis.GetNativeControlRegion(ControlType::Combobox, ControlPart::ButtonDown,
            aArea, ControlState::NONE, aControlValue, OUString(), aBound, aContent) )
    {
        // convert back from border space to local coordinates
        aPoint = pBorder->ScreenToOutputPixel(m_rThis.OutputToScreenPixel(aPoint));
        aContent.Move(-aPoint.X(), -aPoint.Y());

        aBounds.aButtonPos = Point(aContent.Left(), nTop);
        aBounds.aButtonSize = Size(aContent.getWidth(), (nBottom-nTop));

        // adjust the size of the edit field
        if (m_rThis.GetNativeControlRegion(ControlType::Combobox, ControlPart::SubEdit,
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
        long nSBWidth = m_rThis.GetSettings().GetStyleSettings().GetScrollBarSize();
        nSBWidth = m_rThis.CalcZoom( nSBWidth );
        aBounds.aSubEditSize = Size(rOutSz.Width() - nSBWidth, rOutSz.Height());
        aBounds.aButtonPos = Point(rOutSz.Width() - nSBWidth, nTop);
        aBounds.aButtonSize = Size(nSBWidth, (nBottom-nTop));
    }
    return aBounds;
}

void ComboBox::setMaxWidthChars(sal_Int32 nWidth)
{
    if (nWidth != m_pImpl->m_nMaxWidthChars)
    {
        m_pImpl->m_nMaxWidthChars = nWidth;
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

FactoryFunction ComboBox::GetUITestFactory() const
{
    return ComboBoxUIObject::create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
