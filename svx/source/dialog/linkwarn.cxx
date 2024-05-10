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
#include <svx/linkwarn.hxx>
#include <officecfg/Office/Common.hxx>

SvxLinkWarningDialog::SvxLinkWarningDialog(weld::Widget* pParent, const OUString& _rFileName)
    : MessageDialogController(pParent, u"svx/ui/linkwarndialog.ui"_ustr, u"LinkWarnDialog"_ustr,
                              u"ask"_ustr)
    , m_xWarningOnBox(m_xBuilder->weld_check_button(u"ask"_ustr))
{
    // replace filename
    OUString sInfoText = m_xDialog->get_primary_text();
    OUString aPath;
    if (osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL(_rFileName, aPath))
        aPath = _rFileName;
    sInfoText = sInfoText.replaceAll("%FILENAME", aPath);
    m_xDialog->set_primary_text(sInfoText);

    // load state of "warning on" checkbox from misc options
    m_xWarningOnBox->set_active(officecfg::Office::Common::Misc::ShowLinkWarningDialog::get());
    m_xWarningOnBox->set_sensitive(
        !officecfg::Office::Common::Misc::ShowLinkWarningDialog::isReadOnly());
}

SvxLinkWarningDialog::~SvxLinkWarningDialog()
{
    try
    {
        // save value of "warning off" checkbox, if necessary
        bool bChecked = m_xWarningOnBox->get_active();
        if (officecfg::Office::Common::Misc::ShowLinkWarningDialog::get() != bChecked)
        {
            std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
                comphelper::ConfigurationChanges::create());
            officecfg::Office::Common::Misc::ShowLinkWarningDialog::set(bChecked, xChanges);
            xChanges->commit();
        }
    }
    catch (...)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
