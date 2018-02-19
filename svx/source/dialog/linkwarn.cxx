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
#include <vcl/svapp.hxx>
#include <svx/linkwarn.hxx>
#include <svtools/miscopt.hxx>

SvxLinkWarningDialog::SvxLinkWarningDialog(weld::Widget* pParent, const OUString& _rFileName)
    : m_xBuilder(Application::CreateBuilder(pParent, "svx/ui/linkwarndialog.ui"))
    , m_xDialog(m_xBuilder->weld_message_dialog("LinkWarnDialog"))
    , m_xWarningOnBox(m_xBuilder->weld_check_button("ask"))
{
    // replace filename
    OUString sInfoText = m_xDialog->get_primary_text();
    OUString aPath;
    if ( osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL( _rFileName, aPath ) )
        aPath = _rFileName;
    sInfoText = sInfoText.replaceAll("%FILENAME", aPath);
    m_xDialog->set_primary_text(sInfoText);

    // load state of "warning on" checkbox from misc options
    SvtMiscOptions aMiscOpt;
    m_xWarningOnBox->set_active(aMiscOpt.ShowLinkWarningDialog());
    m_xWarningOnBox->set_sensitive(!aMiscOpt.IsShowLinkWarningDialogReadOnly());
}

SvxLinkWarningDialog::~SvxLinkWarningDialog()
{
    // save value of "warning off" checkbox, if necessary
    SvtMiscOptions aMiscOpt;
    bool bChecked = m_xWarningOnBox->get_active();
    if (aMiscOpt.ShowLinkWarningDialog() != bChecked)
        aMiscOpt.SetShowLinkWarningDialog(bChecked);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
