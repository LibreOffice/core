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

#include <osl/file.hxx>
#include <tools/simplerm.hxx>
#include <vcl/layout.hxx>

#include "strings.hrc"
#include "nameclashdlg.hxx"

// NameClashDialog ---------------------------------------------------------

IMPL_LINK( NameClashDialog, ButtonHdl_Impl, Button *, pBtn, void )
{
    long nRet = (long) ABORT;
    if ( m_pBtnRename == pBtn )
    {
        nRet = (long) RENAME;
        OUString aNewName = m_pEDNewName->GetText();
        if ( ( aNewName == maNewName ) || aNewName.isEmpty() )
        {
            ScopedVclPtrInstance< MessageDialog > aError(nullptr, maSameName);
            aError->Execute();
            return;
        }
        maNewName = aNewName;
    }
    else if ( m_pBtnOverwrite == pBtn )
        nRet = (long) OVERWRITE;

    EndDialog( nRet );
}


NameClashDialog::NameClashDialog( vcl::Window* pParent, const std::locale& rResLocale,
                                  OUString const & rTargetFolderURL,
                                  OUString const & rClashingName,
                                  OUString const & rProposedNewName,
                                  bool bAllowOverwrite )
    : ModalDialog( pParent, "SimpleNameClashDialog", "uui/ui/simplenameclash.ui" ),
    maNewName              ( rClashingName )
{
    get(m_pFTMessage, "warning");
    get(m_pEDNewName, "newname");
    get(m_pBtnOverwrite, "replace");
    get(m_pBtnRename, "rename");
    get(m_pBtnCancel, "cancel");

    Link<Button*,void> aLink( LINK( this, NameClashDialog, ButtonHdl_Impl ) );
    m_pBtnOverwrite->SetClickHdl( aLink );
    m_pBtnRename->SetClickHdl( aLink );
    m_pBtnCancel->SetClickHdl( aLink );

    OUString aInfo;
    if ( bAllowOverwrite )
    {
        aInfo = Translate::get(STR_RENAME_OR_REPLACE, rResLocale);
    }
    else
    {
        aInfo = Translate::get(STR_NAME_CLASH_RENAME_ONLY, rResLocale);
        m_pBtnOverwrite->Hide();
    }

    OUString aPath;
    if ( osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL( rTargetFolderURL, aPath ) )
        aPath = rTargetFolderURL;

    maSameName = Translate::get(STR_SAME_NAME_USED, rResLocale);

    aInfo = aInfo.replaceFirst( "%NAME", rClashingName );
    aInfo = aInfo.replaceFirst( "%FOLDER", aPath );
    m_pFTMessage->SetText( aInfo );
    if ( !rProposedNewName.isEmpty() )
        m_pEDNewName->SetText( rProposedNewName );
    else
        m_pEDNewName->SetText( rClashingName );
}

NameClashDialog::~NameClashDialog()
{
    disposeOnce();
}

void NameClashDialog::dispose()
{
    m_pFTMessage.clear();
    m_pEDNewName.clear();
    m_pBtnOverwrite.clear();
    m_pBtnRename.clear();
    m_pBtnCancel.clear();
    ModalDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
