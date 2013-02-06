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


#include "srchdlg.hxx"
#include <comphelper/string.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/sfxuno.hxx>

#include "srchdlg.hrc"
#include "dialog.hrc"
#include <tools/debug.hxx>
#include <unotools/viewoptions.hxx>

using namespace ::com::sun::star::uno;

// ============================================================================

namespace sfx2 {

#define MAX_SAVE_COUNT      (sal_uInt16)10

// ============================================================================
// SearchDialog
// ============================================================================

SearchDialog::SearchDialog( Window* pWindow, const ::rtl::OUString& rConfigName ) :

    ModelessDialog( pWindow, SfxResId( RID_DLG_SEARCH ) ),

    m_aSearchLabel      ( this, SfxResId( FT_SEARCH ) ),
    m_aSearchEdit       ( this, SfxResId( ED_SEARCH ) ),
    m_aWholeWordsBox    ( this, SfxResId( CB_WHOLEWORDS ) ),
    m_aMatchCaseBox     ( this, SfxResId( CB_MATCHCASE ) ),
    m_aWrapAroundBox    ( this, SfxResId( CB_WRAPAROUND ) ),
    m_aBackwardsBox     ( this, SfxResId( CB_BACKWARDS ) ),
    m_aFindBtn          ( this, SfxResId( PB_FIND ) ),
    m_aCancelBtn        ( this, SfxResId( PB_CANCELFIND ) ),
    m_sToggleText       ( SfxResId( STR_TOGGLE ).toString() ),
    m_sConfigName       ( rConfigName ),
    m_bIsConstructed    ( false )

{
    FreeResource();

    // set handler
    m_aFindBtn.SetClickHdl( LINK( this, SearchDialog, FindHdl ) );
    m_aBackwardsBox.SetClickHdl( LINK( this, SearchDialog, ToggleHdl ) );
    // load config: old search strings and the status of the check boxes
    LoadConfig();
    // we need to change the text of the WrapAround box, depends on the status of the Backwards box
    if ( m_aBackwardsBox.IsChecked() )
        ToggleHdl( &m_aBackwardsBox );
    // the search edit should have the focus
    m_aSearchEdit.GrabFocus();
}

SearchDialog::~SearchDialog()
{
    SaveConfig();
    m_aCloseHdl.Call( NULL );
}

void SearchDialog::LoadConfig()
{
    SvtViewOptions aViewOpt( E_DIALOG, m_sConfigName );
    if ( aViewOpt.Exists() )
    {
        m_sWinState = rtl::OUStringToOString(aViewOpt.GetWindowState(), RTL_TEXTENCODING_ASCII_US);
        Any aUserItem = aViewOpt.GetUserItem( "UserItem" );
        ::rtl::OUString aTemp;
        if ( aUserItem >>= aTemp )
        {
            String sUserData( aTemp );
            DBG_ASSERT( comphelper::string::getTokenCount(sUserData, ';') == 5, "invalid config data" );
            xub_StrLen nIdx = 0;
            String sSearchText = sUserData.GetToken( 0, ';', nIdx );
            m_aWholeWordsBox.Check( sUserData.GetToken( 0, ';', nIdx ).ToInt32() == 1 );
            m_aMatchCaseBox.Check( sUserData.GetToken( 0, ';', nIdx ).ToInt32() == 1 );
            m_aWrapAroundBox.Check( sUserData.GetToken( 0, ';', nIdx ).ToInt32() == 1 );
            m_aBackwardsBox.Check( sUserData.GetToken( 0, ';', nIdx ).ToInt32() == 1 );

            nIdx = 0;
            while ( nIdx != STRING_NOTFOUND )
                m_aSearchEdit.InsertEntry( sSearchText.GetToken( 0, '\t', nIdx ) );
            m_aSearchEdit.SelectEntryPos(0);
        }
    }
    else
        m_aWrapAroundBox.Check( sal_True );
}

void SearchDialog::SaveConfig()
{
    SvtViewOptions aViewOpt( E_DIALOG, m_sConfigName );
    aViewOpt.SetWindowState(rtl::OStringToOUString(m_sWinState, RTL_TEXTENCODING_ASCII_US));
    String sUserData;
    sal_uInt16 i = 0, nCount = Min( m_aSearchEdit.GetEntryCount(), MAX_SAVE_COUNT );
    for ( ; i < nCount; ++i )
    {
        sUserData += m_aSearchEdit.GetEntry(i);
        sUserData += '\t';
    }
    sUserData = comphelper::string::stripStart(sUserData, '\t');
    sUserData += ';';
    sUserData += String::CreateFromInt32( m_aWholeWordsBox.IsChecked() ? 1 : 0 );
    sUserData += ';';
    sUserData += String::CreateFromInt32( m_aMatchCaseBox.IsChecked() ? 1 : 0 );
    sUserData += ';';
    sUserData += String::CreateFromInt32( m_aWrapAroundBox.IsChecked() ? 1 : 0 );
    sUserData += ';';
    sUserData += String::CreateFromInt32( m_aBackwardsBox.IsChecked() ? 1 : 0 );

    Any aUserItem = makeAny( ::rtl::OUString( sUserData ) );
    aViewOpt.SetUserItem( "UserItem", aUserItem );
}

IMPL_LINK_NOARG(SearchDialog, FindHdl)
{
    String sSrchTxt = m_aSearchEdit.GetText();
    sal_uInt16 nPos = m_aSearchEdit.GetEntryPos( sSrchTxt );
    if ( nPos > 0 && nPos != COMBOBOX_ENTRY_NOTFOUND )
        m_aSearchEdit.RemoveEntry( nPos );
    if ( nPos > 0 )
        m_aSearchEdit.InsertEntry( sSrchTxt, 0 );
    m_aFindHdl.Call( this );
    return 0;
}

IMPL_LINK_NOARG(SearchDialog, ToggleHdl)
{
    String sTemp = m_aWrapAroundBox.GetText();
    m_aWrapAroundBox.SetText( m_sToggleText );
    m_sToggleText = sTemp;
    return 0;
}

void SearchDialog::SetFocusOnEdit()
{
    Selection aSelection( 0, m_aSearchEdit.GetText().getLength() );
    m_aSearchEdit.SetSelection( aSelection );
    m_aSearchEdit.GrabFocus();
}

sal_Bool SearchDialog::Close()
{
    sal_Bool bRet = ModelessDialog::Close();
    m_aCloseHdl.Call( this );
    return bRet;
}

void SearchDialog::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        if (!m_sWinState.isEmpty())
            SetWindowState( m_sWinState );
        m_bIsConstructed = sal_True;
    }

    ModelessDialog::StateChanged( nStateChange );
}

void SearchDialog::Move()
{
    ModelessDialog::Move();
    if ( m_bIsConstructed && IsReallyVisible() )
        m_sWinState = GetWindowState( WINDOWSTATE_MASK_POS | WINDOWSTATE_MASK_STATE );
}

// ============================================================================

} // namespace sfx2

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
