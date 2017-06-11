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
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxuno.hxx>

#include "sfx2/strings.hrc"
#include <tools/debug.hxx>
#include <unotools/viewoptions.hxx>

using namespace ::com::sun::star::uno;


namespace sfx2 {

#define MAX_SAVE_COUNT      (sal_uInt16)10


// SearchDialog


SearchDialog::SearchDialog(vcl::Window* pWindow, const OUString& rConfigName)
    : ModelessDialog(pWindow, "SearchDialog", "sfx/ui/searchdialog.ui")
    , m_sConfigName(rConfigName)
    , m_bIsConstructed(false)

{
    get(m_pSearchEdit, "searchterm");
    get(m_pWholeWordsBox, "wholewords");
    get(m_pMatchCaseBox, "matchcase");
    get(m_pWrapAroundBox, "wrap");
    get(m_pBackwardsBox, "backwards");
    get(m_pFindBtn, "search");

    // set handler
    m_pFindBtn->SetClickHdl( LINK( this, SearchDialog, FindHdl ) );
    // load config: old search strings and the status of the check boxes
    LoadConfig();
    // the search edit should have the focus
    m_pSearchEdit->GrabFocus();
}

SearchDialog::~SearchDialog()
{
    disposeOnce();
}

void SearchDialog::dispose()
{
    SaveConfig();
    m_pSearchEdit.clear();
    m_pWholeWordsBox.clear();
    m_pMatchCaseBox.clear();
    m_pWrapAroundBox.clear();
    m_pBackwardsBox.clear();
    m_pFindBtn.clear();
    ModelessDialog::dispose();
}

void SearchDialog::LoadConfig()
{
    SvtViewOptions aViewOpt( EViewType::Dialog, m_sConfigName );
    if ( aViewOpt.Exists() )
    {
        m_sWinState = OUStringToOString(aViewOpt.GetWindowState(), RTL_TEXTENCODING_ASCII_US);
        Any aUserItem = aViewOpt.GetUserItem( "UserItem" );
        OUString sUserData;
        if ( aUserItem >>= sUserData )
        {
            DBG_ASSERT( comphelper::string::getTokenCount(sUserData, ';') == 5, "invalid config data" );
            sal_Int32 nIdx = 0;
            OUString sSearchText = sUserData.getToken( 0, ';', nIdx );
            m_pWholeWordsBox->Check( sUserData.getToken( 0, ';', nIdx ).toInt32() == 1 );
            m_pMatchCaseBox->Check( sUserData.getToken( 0, ';', nIdx ).toInt32() == 1 );
            m_pWrapAroundBox->Check( sUserData.getToken( 0, ';', nIdx ).toInt32() == 1 );
            m_pBackwardsBox->Check( sUserData.getToken( 0, ';', nIdx ).toInt32() == 1 );

            nIdx = 0;
            while ( nIdx != -1 )
                m_pSearchEdit->InsertEntry( sSearchText.getToken( 0, '\t', nIdx ) );
            m_pSearchEdit->SelectEntryPos(0);
        }
    }
    else
        m_pWrapAroundBox->Check();
}

void SearchDialog::SaveConfig()
{
    SvtViewOptions aViewOpt( EViewType::Dialog, m_sConfigName );
    aViewOpt.SetWindowState(OStringToOUString(m_sWinState, RTL_TEXTENCODING_ASCII_US));
    OUString sUserData;
    sal_Int32 i = 0, nCount = std::min( m_pSearchEdit->GetEntryCount(), static_cast<sal_Int32>(MAX_SAVE_COUNT) );
    for ( ; i < nCount; ++i )
    {
        sUserData += m_pSearchEdit->GetEntry(i);
        sUserData += "\t";
    }
    sUserData = comphelper::string::stripStart(sUserData, '\t');
    sUserData += ";";
    sUserData += OUString::number( m_pWholeWordsBox->IsChecked() ? 1 : 0 );
    sUserData += ";";
    sUserData += OUString::number( m_pMatchCaseBox->IsChecked() ? 1 : 0 );
    sUserData += ";";
    sUserData += OUString::number( m_pWrapAroundBox->IsChecked() ? 1 : 0 );
    sUserData += ";";
    sUserData += OUString::number( m_pBackwardsBox->IsChecked() ? 1 : 0 );

    Any aUserItem = makeAny( sUserData );
    aViewOpt.SetUserItem( "UserItem", aUserItem );
}

IMPL_LINK_NOARG(SearchDialog, FindHdl, Button*, void)
{
    OUString sSrchTxt = m_pSearchEdit->GetText();
    sal_Int32 nPos = m_pSearchEdit->GetEntryPos( sSrchTxt );
    if ( nPos > 0 && nPos != COMBOBOX_ENTRY_NOTFOUND )
        m_pSearchEdit->RemoveEntryAt(nPos);
    if ( nPos > 0 )
        m_pSearchEdit->InsertEntry( sSrchTxt, 0 );
    m_aFindHdl.Call( *this );
}

void SearchDialog::SetFocusOnEdit()
{
    Selection aSelection( 0, m_pSearchEdit->GetText().getLength() );
    m_pSearchEdit->SetSelection( aSelection );
    m_pSearchEdit->GrabFocus();
}

bool SearchDialog::Close()
{
    bool bRet = ModelessDialog::Close();
    m_aCloseHdl.Call( nullptr );
    return bRet;
}

void SearchDialog::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::InitShow )
    {
        if (!m_sWinState.isEmpty())
            SetWindowState( m_sWinState );
        m_bIsConstructed = true;
    }

    ModelessDialog::StateChanged( nStateChange );
}

void SearchDialog::Move()
{
    ModelessDialog::Move();
    if ( m_bIsConstructed && IsReallyVisible() )
        m_sWinState = GetWindowState( WindowStateMask::Pos | WindowStateMask::State );
}


} // namespace sfx2


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
