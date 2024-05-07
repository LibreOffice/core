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
    : GenericDialogController(pParent, u"cui/ui/securityoptionsdialog.ui"_ustr, u"SecurityOptionsDialog"_ustr)
    , m_xSaveOrSendDocsCB(m_xBuilder->weld_check_button(u"savesenddocs"_ustr))
    , m_xSaveOrSendDocsImg(m_xBuilder->weld_widget(u"locksavesenddocs"_ustr))
    , m_xSignDocsCB(m_xBuilder->weld_check_button(u"whensigning"_ustr))
    , m_xSignDocsImg(m_xBuilder->weld_widget(u"lockwhensigning"_ustr))
    , m_xPrintDocsCB(m_xBuilder->weld_check_button(u"whenprinting"_ustr))
    , m_xPrintDocsImg(m_xBuilder->weld_widget(u"lockwhenprinting"_ustr))
    , m_xCreatePdfCB(m_xBuilder->weld_check_button(u"whenpdf"_ustr))
    , m_xCreatePdfImg(m_xBuilder->weld_widget(u"lockwhenpdf"_ustr))
    , m_xRemovePersInfoCB(m_xBuilder->weld_check_button(u"removepersonal"_ustr))
    , m_xRemovePersInfoImg(m_xBuilder->weld_widget(u"lockremovepersonal"_ustr))
    , m_xRecommPasswdCB(m_xBuilder->weld_check_button(u"password"_ustr))
    , m_xRecommPasswdImg(m_xBuilder->weld_widget(u"lockpassword"_ustr))
    , m_xCtrlHyperlinkCB(m_xBuilder->weld_check_button(u"ctrlclick"_ustr))
    , m_xCtrlHyperlinkImg(m_xBuilder->weld_widget(u"lockctrlclick"_ustr))
    , m_xBlockUntrustedRefererLinksCB(m_xBuilder->weld_check_button(u"blockuntrusted"_ustr))
    , m_xBlockUntrustedRefererLinksImg(m_xBuilder->weld_widget(u"lockblockuntrusted"_ustr))
    , m_xDisableActiveContentCB(m_xBuilder->weld_check_button(u"disableactivecontent"_ustr))
    , m_xDisableActiveContentImg(m_xBuilder->weld_widget(u"lockdisableactivecontent"_ustr))
    , m_xRedlineinfoCB(m_xBuilder->weld_check_button(u"redlineinfo"_ustr))
    , m_xRedlineinfoImg(m_xBuilder->weld_widget(u"lockredlineinfo"_ustr))
    , m_xDocPropertiesCB(m_xBuilder->weld_check_button(u"docproperties"_ustr))
    , m_xDocPropertiesImg(m_xBuilder->weld_widget(u"lockdocproperties"_ustr))
    , m_xNoteAuthorCB(m_xBuilder->weld_check_button(u"noteauthor"_ustr))
    , m_xNoteAuthorImg(m_xBuilder->weld_widget(u"locknoteauthor"_ustr))
    , m_xDocumentVersionCB(m_xBuilder->weld_check_button(u"documentversion"_ustr))
    , m_xDocumentVersionImg(m_xBuilder->weld_widget(u"lockdocumentversion"_ustr))
    , m_xPrinterSettingsCB(m_xBuilder->weld_check_button(u"printersettings"_ustr))
    , m_xPrinterSettingsImg(m_xBuilder->weld_widget(u"lockprintersettings"_ustr))
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
    enableAndSet(SvtSecurityOptions::EOption::DocKeepPrinterSettings, *m_xPrinterSettingsCB,
        *m_xPrinterSettingsImg);
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
    CheckAndSave(SvtSecurityOptions::EOption::DocKeepPrinterSettings, IsKeepPrinterSettingsChecked(), bModified);
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
    m_xPrinterSettingsCB->set_sensitive(bEnable);
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
