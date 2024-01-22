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

#include <securityoptions.hxx>
#include <unotools/securityoptions.hxx>

namespace
{
    bool enableAndSet(SvtSecurityOptions::EOption eOption,
                      weld::CheckButton& rCheckBox, weld::Widget& rFixedImage)
    {
        bool bEnable = !SvtSecurityOptions::IsReadOnly(eOption);
        rCheckBox.set_sensitive(bEnable);
        rFixedImage.set_visible(!bEnable);
        rCheckBox.set_active(SvtSecurityOptions::IsOptionSet(eOption));
        return bEnable;
    }

    void CheckAndSave(SvtSecurityOptions::EOption _eOpt, const bool _bIsChecked, bool& _rModified)
    {
        if (!SvtSecurityOptions::IsReadOnly(_eOpt) && SvtSecurityOptions::IsOptionSet(_eOpt) != _bIsChecked)
        {
            SvtSecurityOptions::SetOption(_eOpt, _bIsChecked);
            _rModified = true;
        }
    }
}

namespace svx
{

SecurityOptionsDialog::SecurityOptionsDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/securityoptionsdialog.ui", "SecurityOptionsDialog")
    , m_xSaveOrSendDocsCB(m_xBuilder->weld_check_button("savesenddocs"))
    , m_xSaveOrSendDocsImg(m_xBuilder->weld_widget("locksavesenddocs"))
    , m_xSignDocsCB(m_xBuilder->weld_check_button("whensigning"))
    , m_xSignDocsImg(m_xBuilder->weld_widget("lockwhensigning"))
    , m_xPrintDocsCB(m_xBuilder->weld_check_button("whenprinting"))
    , m_xPrintDocsImg(m_xBuilder->weld_widget("lockwhenprinting"))
    , m_xCreatePdfCB(m_xBuilder->weld_check_button("whenpdf"))
    , m_xCreatePdfImg(m_xBuilder->weld_widget("lockwhenpdf"))
    , m_xRemovePersInfoCB(m_xBuilder->weld_check_button("removepersonal"))
    , m_xRemovePersInfoImg(m_xBuilder->weld_widget("lockremovepersonal"))
    , m_xRecommPasswdCB(m_xBuilder->weld_check_button("password"))
    , m_xRecommPasswdImg(m_xBuilder->weld_widget("lockpassword"))
    , m_xCtrlHyperlinkCB(m_xBuilder->weld_check_button("ctrlclick"))
    , m_xCtrlHyperlinkImg(m_xBuilder->weld_widget("lockctrlclick"))
    , m_xBlockUntrustedRefererLinksCB(m_xBuilder->weld_check_button("blockuntrusted"))
    , m_xBlockUntrustedRefererLinksImg(m_xBuilder->weld_widget("lockblockuntrusted"))
    , m_xDisableActiveContentCB(m_xBuilder->weld_check_button("disableactivecontent"))
    , m_xDisableActiveContentImg(m_xBuilder->weld_widget("lockdisableactivecontent"))
    , m_xRedlineinfoCB(m_xBuilder->weld_check_button("redlineinfo"))
    , m_xRedlineinfoImg(m_xBuilder->weld_widget("lockredlineinfo"))
    , m_xDocPropertiesCB(m_xBuilder->weld_check_button("docproperties"))
    , m_xDocPropertiesImg(m_xBuilder->weld_widget("lockdocproperties"))
    , m_xNoteAuthorCB(m_xBuilder->weld_check_button("noteauthor"))
    , m_xNoteAuthorImg(m_xBuilder->weld_widget("locknoteauthor"))
    , m_xDocumentVersionCB(m_xBuilder->weld_check_button("documentversion"))
    , m_xDocumentVersionImg(m_xBuilder->weld_widget("lockdocumentversion"))
{
    m_xRemovePersInfoCB->connect_toggled(LINK(this, SecurityOptionsDialog, ShowPersonalInfosToggle));
    init();
}

IMPL_LINK_NOARG(SecurityOptionsDialog, ShowPersonalInfosToggle, weld::Toggleable&, void)
{
    changeKeepSecurityInfosEnabled();
}

void SecurityOptionsDialog::init()
{
    enableAndSet(SvtSecurityOptions::EOption::DocWarnSaveOrSend, *m_xSaveOrSendDocsCB,
        *m_xSaveOrSendDocsImg);
    enableAndSet(SvtSecurityOptions::EOption::DocWarnSigning, *m_xSignDocsCB,
        *m_xSignDocsImg);
    enableAndSet(SvtSecurityOptions::EOption::DocWarnPrint, *m_xPrintDocsCB,
        *m_xPrintDocsImg);
    enableAndSet(SvtSecurityOptions::EOption::DocWarnCreatePdf, *m_xCreatePdfCB,
        *m_xCreatePdfImg);
    enableAndSet(SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo, *m_xRemovePersInfoCB,
        *m_xRemovePersInfoImg);
    enableAndSet(SvtSecurityOptions::EOption::DocWarnKeepRedlineInfo, *m_xRedlineinfoCB,
        *m_xRedlineinfoImg);
    enableAndSet(SvtSecurityOptions::EOption::DocWarnKeepDocUserInfo, *m_xDocPropertiesCB,
        *m_xDocPropertiesImg);
    enableAndSet(SvtSecurityOptions::EOption::DocWarnKeepNoteAuthorDateInfo, *m_xNoteAuthorCB,
        *m_xNoteAuthorImg);
    enableAndSet(SvtSecurityOptions::EOption::DocWarnKeepDocVersionInfo, *m_xDocumentVersionCB,
        *m_xDocumentVersionImg);
    enableAndSet(SvtSecurityOptions::EOption::DocWarnRecommendPassword, *m_xRecommPasswdCB,
        *m_xRecommPasswdImg);
    enableAndSet(SvtSecurityOptions::EOption::CtrlClickHyperlink, *m_xCtrlHyperlinkCB,
        *m_xCtrlHyperlinkImg);
    enableAndSet(SvtSecurityOptions::EOption::BlockUntrustedRefererLinks, *m_xBlockUntrustedRefererLinksCB,
        *m_xBlockUntrustedRefererLinksImg);
    enableAndSet(SvtSecurityOptions::EOption::DisableActiveContent,
                 *m_xDisableActiveContentCB, *m_xDisableActiveContentImg);

    if (!SvtSecurityOptions::IsReadOnly(SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo))
        changeKeepSecurityInfosEnabled();
}

bool SecurityOptionsDialog::SetSecurityOptions()
{
    bool bModified = false;
    CheckAndSave(SvtSecurityOptions::EOption::DocWarnSaveOrSend, IsSaveOrSendDocsChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::DocWarnSigning, IsSignDocsChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::DocWarnPrint, IsPrintDocsChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::DocWarnCreatePdf, IsCreatePdfChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo, IsRemovePersInfoChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::DocWarnKeepRedlineInfo, IsRemoveRedlineInfoChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::DocWarnKeepDocUserInfo, IsRemoveDocUserInfoChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::DocWarnKeepNoteAuthorDateInfo, IsRemoveNoteAuthorInfoChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::DocWarnKeepDocVersionInfo, IsRemoveDocVersionInfoChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::DocWarnRecommendPassword, IsRecommPasswdChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::CtrlClickHyperlink, IsCtrlHyperlinkChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::BlockUntrustedRefererLinks, IsBlockUntrustedRefererLinksChecked(), bModified);
    CheckAndSave(SvtSecurityOptions::EOption::DisableActiveContent, IsDisableActiveContentChecked(), bModified);

    return bModified;
}

void SecurityOptionsDialog::changeKeepSecurityInfosEnabled()
{
    bool bEnable = m_xRemovePersInfoCB->get_active();
    m_xRedlineinfoCB->set_sensitive(bEnable);
    m_xDocPropertiesCB->set_sensitive(bEnable);
    m_xNoteAuthorCB->set_sensitive(bEnable);
    m_xDocumentVersionCB->set_sensitive(bEnable);
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
