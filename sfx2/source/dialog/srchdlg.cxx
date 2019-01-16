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


#include <srchdlg.hxx>
#include <comphelper/string.hxx>
#include <sfx2/sfxuno.hxx>

#include <tools/debug.hxx>
#include <unotools/viewoptions.hxx>

using namespace ::com::sun::star::uno;


namespace sfx2 {

#define MAX_SAVE_COUNT      sal_uInt16(10)


// SearchDialog


SearchDialog::SearchDialog(weld::Window* pWindow, const OUString& rConfigName)
    : GenericDialogController(pWindow, "sfx/ui/searchdialog.ui", "SearchDialog")
    , m_sConfigName(rConfigName)
    , m_xSearchEdit(m_xBuilder->weld_combo_box("searchterm"))
    , m_xWholeWordsBox(m_xBuilder->weld_check_button("wholewords"))
    , m_xMatchCaseBox(m_xBuilder->weld_check_button("matchcase"))
    , m_xWrapAroundBox(m_xBuilder->weld_check_button("wrap"))
    , m_xBackwardsBox(m_xBuilder->weld_check_button("backwards"))
    , m_xFindBtn(m_xBuilder->weld_button("ok"))
{
    // set handler
    m_xFindBtn->connect_clicked(LINK(this, SearchDialog, FindHdl));
    // load config: old search strings and the status of the check boxes
    LoadConfig();
    // the search edit should have the focus
    m_xSearchEdit->grab_focus();
}

SearchDialog::~SearchDialog()
{
    SaveConfig();
}

void SearchDialog::LoadConfig()
{
    SvtViewOptions aViewOpt( EViewType::Dialog, m_sConfigName );
    if ( aViewOpt.Exists() )
    {
        Any aUserItem = aViewOpt.GetUserItem( "UserItem" );
        OUString sUserData;
        if ( aUserItem >>= sUserData )
        {
            DBG_ASSERT( comphelper::string::getTokenCount(sUserData, ';') == 5, "invalid config data" );
            sal_Int32 nIdx = 0;
            OUString sSearchText = sUserData.getToken( 0, ';', nIdx );
            m_xWholeWordsBox->set_active( sUserData.getToken( 0, ';', nIdx ).toInt32() == 1 );
            m_xMatchCaseBox->set_active( sUserData.getToken( 0, ';', nIdx ).toInt32() == 1 );
            m_xWrapAroundBox->set_active( sUserData.getToken( 0, ';', nIdx ).toInt32() == 1 );
            m_xBackwardsBox->set_active( sUserData.getToken( 0, ';', nIdx ).toInt32() == 1 );

            nIdx = 0;
            while ( nIdx != -1 )
                m_xSearchEdit->append_text(sSearchText.getToken( 0, '\t', nIdx));
            m_xSearchEdit->set_active(0);
        }
    }
    else
        m_xWrapAroundBox->set_active(true);
}

void SearchDialog::SaveConfig()
{
    SvtViewOptions aViewOpt( EViewType::Dialog, m_sConfigName );
    OUString sUserData;
    int i = 0, nCount = std::min(m_xSearchEdit->get_count(), static_cast<int>(MAX_SAVE_COUNT));
    for ( ; i < nCount; ++i )
    {
        sUserData += m_xSearchEdit->get_text(i);
        sUserData += "\t";
    }
    sUserData = comphelper::string::stripStart(sUserData, '\t');
    sUserData += ";";
    sUserData += OUString::number( m_xWholeWordsBox->get_active() ? 1 : 0 );
    sUserData += ";";
    sUserData += OUString::number( m_xMatchCaseBox->get_active() ? 1 : 0 );
    sUserData += ";";
    sUserData += OUString::number( m_xWrapAroundBox->get_active() ? 1 : 0 );
    sUserData += ";";
    sUserData += OUString::number( m_xBackwardsBox->get_active() ? 1 : 0 );

    Any aUserItem = makeAny( sUserData );
    aViewOpt.SetUserItem( "UserItem", aUserItem );
}

IMPL_LINK_NOARG(SearchDialog, FindHdl, weld::Button&, void)
{
    OUString sSrchTxt = m_xSearchEdit->get_active_text();
    auto nPos = m_xSearchEdit->find_text(sSrchTxt);
    if (nPos != 0)
    {
        if (nPos != -1)
            m_xSearchEdit->remove(nPos);
        m_xSearchEdit->insert_text(0, sSrchTxt);
    }
    m_aFindHdl.Call( *this );
}

void SearchDialog::SetFocusOnEdit()
{
    m_xSearchEdit->select_entry_region(0, -1);
    m_xSearchEdit->grab_focus();
}

void SearchDialog::runAsync(const std::shared_ptr<SearchDialog>& rController)
{
    weld::DialogController::runAsync(rController, [=](sal_Int32 /*nResult*/){ rController->m_aCloseHdl.Call(nullptr); });
}

} // namespace sfx2


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
