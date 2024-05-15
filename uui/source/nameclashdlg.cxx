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
#include <unotools/resmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <strings.hrc>
#include "nameclashdlg.hxx"

// NameClashDialog ---------------------------------------------------------

IMPL_LINK(NameClashDialog, ButtonHdl_Impl, weld::Button&, rBtn, void)
{
    tools::Long nRet = tools::Long(ABORT);
    if (m_xBtnRename.get() == &rBtn)
    {
        nRet = tools::Long(RENAME);
        OUString aNewName = m_xEDNewName->get_text();
        if ( ( aNewName == m_aNewName ) || aNewName.isEmpty() )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Warning, VclButtonsType::Ok,
                                                      m_aSameName));
            xErrorBox->run();
            return;
        }
        m_aNewName = aNewName;
    }
    else if (m_xBtnOverwrite.get() == &rBtn)
        nRet = tools::Long(OVERWRITE);

    m_xDialog->response(nRet);
}


NameClashDialog::NameClashDialog( weld::Window* pParent, const std::locale& rResLocale,
                                  OUString const & rTargetFolderURL,
                                  OUString const & rClashingName,
                                  OUString const & rProposedNewName,
                                  bool bAllowOverwrite )
    : GenericDialogController(pParent, u"uui/ui/simplenameclash.ui"_ustr, u"SimpleNameClashDialog"_ustr)
    , m_aNewName(rClashingName)
    , m_xFTMessage(m_xBuilder->weld_label(u"warning"_ustr))
    , m_xEDNewName(m_xBuilder->weld_entry(u"newname"_ustr))
    , m_xBtnOverwrite(m_xBuilder->weld_button(u"replace"_ustr))
    , m_xBtnRename(m_xBuilder->weld_button(u"rename"_ustr))
    , m_xBtnCancel(m_xBuilder->weld_button(u"cancel"_ustr))
{
    Link<weld::Button&,void> aLink( LINK( this, NameClashDialog, ButtonHdl_Impl ) );
    m_xBtnOverwrite->connect_clicked( aLink );
    m_xBtnRename->connect_clicked( aLink );
    m_xBtnCancel->connect_clicked( aLink );

    OUString aInfo;
    if ( bAllowOverwrite )
    {
        aInfo = Translate::get(STR_RENAME_OR_REPLACE, rResLocale);
    }
    else
    {
        aInfo = Translate::get(STR_NAME_CLASH_RENAME_ONLY, rResLocale);
        m_xBtnOverwrite->hide();
    }

    OUString aPath;
    if ( osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL( rTargetFolderURL, aPath ) )
        aPath = rTargetFolderURL;

    m_aSameName = Translate::get(STR_SAME_NAME_USED, rResLocale);

    aInfo = aInfo.replaceFirst( "%NAME", rClashingName );
    aInfo = aInfo.replaceFirst( "%FOLDER", aPath );
    m_xFTMessage->set_label(aInfo);
    if ( !rProposedNewName.isEmpty() )
        m_xEDNewName->set_text( rProposedNewName );
    else
        m_xEDNewName->set_text( rClashingName );
}

NameClashDialog::~NameClashDialog()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
