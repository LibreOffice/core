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

#include <unotools/saveopt.hxx>
#include <alienwarn.hxx>

SfxAlienWarningDialog::SfxAlienWarningDialog(weld::Window* pParent,
                                             std::u16string_view _rFormatName,
                                             const OUString& _rDefaultExtension,
                                             bool rDefaultIsAlien)
    : MessageDialogController(pParent, "sfx/ui/alienwarndialog.ui", "AlienWarnDialog", "ask")
    , m_xKeepCurrentBtn(m_xBuilder->weld_button("save"))
    , m_xUseDefaultFormatBtn(m_xBuilder->weld_button("cancel"))
    , m_xWarningOnBox(m_xBuilder->weld_check_button("ask"))
{
    OUString aExtension = "ODF";

    // replace formatname (text)
    OUString sInfoText = m_xDialog->get_primary_text();
    sInfoText = sInfoText.replaceAll("%FORMATNAME", _rFormatName);
    m_xDialog->set_primary_text(sInfoText);

    // replace formatname (button)
    sInfoText = m_xKeepCurrentBtn->get_label();
    sInfoText = sInfoText.replaceAll("%FORMATNAME", _rFormatName);
    m_xKeepCurrentBtn->set_label(sInfoText);

    // hide ODF explanation if default format is alien
    // and set the proper extension in the button
    if (rDefaultIsAlien)
    {
        m_xDialog->set_secondary_text(OUString());
        aExtension = _rDefaultExtension.toAsciiUpperCase();
    }

    // replace defaultextension (button)
    sInfoText = m_xUseDefaultFormatBtn->get_label();
    sInfoText = sInfoText.replaceAll("%DEFAULTEXTENSION", aExtension);
    m_xUseDefaultFormatBtn->set_label(sInfoText);

    // load value of "warning on" checkbox from save options
    m_xWarningOnBox->set_active(SvtSaveOptions().IsWarnAlienFormat());
}

SfxAlienWarningDialog::~SfxAlienWarningDialog()
{
    try
    {
        // save value of "warning off" checkbox, if necessary
        SvtSaveOptions aSaveOpt;
        bool bChecked = m_xWarningOnBox->get_active();
        if (aSaveOpt.IsWarnAlienFormat() != bChecked)
            aSaveOpt.SetWarnAlienFormat(bChecked);
    }
    catch (...)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
