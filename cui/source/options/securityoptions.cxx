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

#include <tools/debug.hxx>
#include <unotools/securityoptions.hxx>
#include "securityoptions.hxx"

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
    enableAndSet(SvtSecurityOptions::EOption::DocWarnRecommendPassword, *m_xRecommPasswdCB,
        *m_xRecommPasswdImg);
    enableAndSet(SvtSecurityOptions::EOption::CtrlClickHyperlink, *m_xCtrlHyperlinkCB,
        *m_xCtrlHyperlinkImg);
    enableAndSet(SvtSecurityOptions::EOption::BlockUntrustedRefererLinks, *m_xBlockUntrustedRefererLinksCB,
        *m_xBlockUntrustedRefererLinksImg);
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
