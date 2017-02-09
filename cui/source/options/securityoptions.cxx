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

#include <unotools/securityoptions.hxx>
#include <dialmgr.hxx>
#include <cuires.hrc>
#include "securityoptions.hxx"

namespace
{
    bool enableAndSet( const SvtSecurityOptions& rOptions,
                       SvtSecurityOptions::EOption eOption,
                       CheckBox& rCheckBox, FixedImage& rFixedImage )
    {
        bool bEnable = rOptions.IsOptionEnabled( eOption );
        rCheckBox.Enable( bEnable );
        rFixedImage.Show( !bEnable );
        rCheckBox.Check( rOptions.IsOptionSet( eOption ) );
        return bEnable;
    }
}


namespace svx
{


SecurityOptionsDialog::SecurityOptionsDialog(vcl::Window* pParent, SvtSecurityOptions* pOptions)
    : ModalDialog(pParent, "SecurityOptionsDialog", "cui/ui/securityoptionsdialog.ui")
{
    DBG_ASSERT( pOptions, "SecurityOptionsDialog::SecurityOptionsDialog(): invalid SvtSecurityOptions" );
    get(m_pSaveOrSendDocsCB, "savesenddocs");
    enableAndSet(*pOptions, SvtSecurityOptions::EOption::DocWarnSaveOrSend, *m_pSaveOrSendDocsCB,
        *get<FixedImage>("locksavesenddocs"));
    get(m_pSignDocsCB, "whensigning");
    enableAndSet(*pOptions, SvtSecurityOptions::EOption::DocWarnSigning, *m_pSignDocsCB,
        *get<FixedImage>("lockwhensigning"));
    get(m_pPrintDocsCB, "whenprinting");
    enableAndSet(*pOptions, SvtSecurityOptions::EOption::DocWarnPrint, *m_pPrintDocsCB,
        *get<FixedImage>("lockwhenprinting"));
    get(m_pCreatePdfCB, "whenpdf");
    enableAndSet(*pOptions, SvtSecurityOptions::EOption::DocWarnCreatePdf, *m_pCreatePdfCB,
        *get<FixedImage>("lockwhenpdf"));
    get(m_pRemovePersInfoCB, "removepersonal");
    enableAndSet(*pOptions, SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo, *m_pRemovePersInfoCB,
        *get<FixedImage>("lockremovepersonal"));
    get(m_pRecommPasswdCB, "password");
    enableAndSet(*pOptions, SvtSecurityOptions::EOption::DocWarnRecommendPassword, *m_pRecommPasswdCB,
        *get<FixedImage>("lockpassword"));
    get(m_pCtrlHyperlinkCB, "ctrlclick");
    enableAndSet(*pOptions, SvtSecurityOptions::EOption::CtrlClickHyperlink, *m_pCtrlHyperlinkCB,
        *get<FixedImage>("lockctrlclick"));
    get(m_pBlockUntrustedRefererLinksCB, "blockuntrusted");
    enableAndSet(*pOptions, SvtSecurityOptions::EOption::BlockUntrustedRefererLinks, *m_pBlockUntrustedRefererLinksCB,
        *get<FixedImage>("lockblockuntrusted"));
}

SecurityOptionsDialog::~SecurityOptionsDialog()
{
    disposeOnce();
}

void SecurityOptionsDialog::dispose()
{
    m_pSaveOrSendDocsCB.clear();
    m_pSignDocsCB.clear();
    m_pPrintDocsCB.clear();
    m_pCreatePdfCB.clear();
    m_pRemovePersInfoCB.clear();
    m_pRecommPasswdCB.clear();
    m_pCtrlHyperlinkCB.clear();
    m_pBlockUntrustedRefererLinksCB.clear();
    ModalDialog::dispose();
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
