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


#include <sal/macros.h>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxuno.hxx>
#include <unotools/saveopt.hxx>
#include <vcl/svapp.hxx>
#include <alienwarn.hxx>

SfxAlienWarningDialog::SfxAlienWarningDialog(weld::Window* pParent, const OUString& _rFormatName,
                                             const OUString& _rDefaultExtension, bool rDefaultIsAlien)
    : m_xBuilder(Application::CreateBuilder(pParent, "sfx/ui/alienwarndialog.ui"))
    , m_xDialog(m_xBuilder->weld_message_dialog("AlienWarnDialog"))
    , m_xKeepCurrentBtn(m_xBuilder->weld_button("save"))
    , m_xUseDefaultFormatBtn(m_xBuilder->weld_button("cancel"))
    , m_xWarningOnBox(m_xBuilder->weld_check_button("ask"))
    , m_xOrigParent(m_xWarningOnBox->weld_parent())
    , m_xContentArea(m_xDialog->weld_message_area())
{
    //fdo#75121, a bit tricky because the widgets we want to align with
    //don't actually exist in the ui description, they're implied
    m_xOrigParent->remove(m_xWarningOnBox.get());
    m_xContentArea->add(m_xWarningOnBox.get());

    OUString aExtension = "ODF";

    // replace formatname (text)
    OUString sInfoText = m_xDialog->get_primary_text();
    sInfoText = sInfoText.replaceAll( "%FORMATNAME", _rFormatName );
    m_xDialog->set_primary_text(sInfoText);

    // replace formatname (button)
    sInfoText = m_xKeepCurrentBtn->get_label();
    sInfoText = sInfoText.replaceAll( "%FORMATNAME", _rFormatName );
    m_xKeepCurrentBtn->set_label(sInfoText);

    // hide ODF explanation if default format is alien
    // and set the proper extension in the button
    if( rDefaultIsAlien )
    {
        m_xDialog->set_secondary_text(OUString());
        aExtension = _rDefaultExtension.toAsciiUpperCase();
    }

    // replace defaultextension (button)
    sInfoText = m_xUseDefaultFormatBtn->get_label();
    sInfoText = sInfoText.replaceAll( "%DEFAULTEXTENSION", aExtension );
    m_xUseDefaultFormatBtn->set_label(sInfoText);

    // load value of "warning on" checkbox from save options
    m_xWarningOnBox->set_active(SvtSaveOptions().IsWarnAlienFormat());
}

SfxAlienWarningDialog::~SfxAlienWarningDialog()
{
    m_xContentArea->remove(m_xWarningOnBox.get());
    m_xOrigParent->add(m_xWarningOnBox.get());
    // save value of "warning off" checkbox, if necessary
    SvtSaveOptions aSaveOpt;
    bool bChecked = m_xWarningOnBox->get_active();
    if (aSaveOpt.IsWarnAlienFormat() != bChecked)
        aSaveOpt.SetWarnAlienFormat(bChecked);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
