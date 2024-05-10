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

#include <alienwarn.hxx>
#include <officecfg/Office/Common.hxx>

SfxAlienWarningDialog::SfxAlienWarningDialog(weld::Window* pParent,
                                             std::u16string_view _rFormatName,
                                             const OUString& _rDefaultExtension,
                                             bool rDefaultIsAlien)
    : MessageDialogController(pParent, u"sfx/ui/alienwarndialog.ui"_ustr, u"AlienWarnDialog"_ustr,
                              u"ask"_ustr)
    , m_xKeepCurrentBtn(m_xBuilder->weld_button(u"save"_ustr))
    , m_xUseDefaultFormatBtn(m_xBuilder->weld_button(u"cancel"_ustr))
    , m_xWarningOnBox(m_xBuilder->weld_check_button(u"ask"_ustr))
{
    OUString aExtension = u"ODF"_ustr;

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
    m_xWarningOnBox->set_active(officecfg::Office::Common::Save::Document::WarnAlienFormat::get());
}

SfxAlienWarningDialog::~SfxAlienWarningDialog()
{
    try
    {
        // save value of "warning off" checkbox, if necessary
        bool bChecked = m_xWarningOnBox->get_active();
        if (officecfg::Office::Common::Save::Document::WarnAlienFormat::get() != bChecked)
        {
            auto xChanges = comphelper::ConfigurationChanges::create();
            officecfg::Office::Common::Save::Document::WarnAlienFormat::set(bChecked, xChanges);
            xChanges->commit();
        }
    }
    catch (...)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
