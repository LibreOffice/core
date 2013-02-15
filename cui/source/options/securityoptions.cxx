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
#include <svtools/stdctrl.hxx>
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

//........................................................................
namespace svx
{
//........................................................................

SecurityOptionsDialog::SecurityOptionsDialog(Window* pParent, SvtSecurityOptions* pOptions)
    : ModalDialog(pParent, "SecurityOptionsDialog", "cui/ui/securityoptionsdialog.ui")
{
    DBG_ASSERT( pOptions, "SecurityOptionsDialog::SecurityOptionsDialog(): invalid SvtSecurityOptions" );
    get(m_pSaveOrSendDocsCB, "savesenddocs");
    enableAndSet(*pOptions, SvtSecurityOptions::E_DOCWARN_SAVEORSEND, *m_pSaveOrSendDocsCB,
        *get<FixedImage>("locksavesenddocs"));
    get(m_pSignDocsCB, "whensigning");
    enableAndSet(*pOptions, SvtSecurityOptions::E_DOCWARN_SIGNING, *m_pSignDocsCB,
        *get<FixedImage>("lockwhensigning"));
    get(m_pPrintDocsCB, "whenprinting");
    enableAndSet(*pOptions, SvtSecurityOptions::E_DOCWARN_PRINT, *m_pPrintDocsCB,
        *get<FixedImage>("lockwhenprinting"));
    get(m_pCreatePdfCB, "whenpdf");
    enableAndSet(*pOptions, SvtSecurityOptions::E_DOCWARN_CREATEPDF, *m_pCreatePdfCB,
        *get<FixedImage>("lockwhenpdf"));
    get(m_pRemovePersInfoCB, "removepersonal");
    enableAndSet(*pOptions, SvtSecurityOptions::E_DOCWARN_REMOVEPERSONALINFO, *m_pRemovePersInfoCB,
        *get<FixedImage>("lockremovepersonal"));
    get(m_pRecommPasswdCB, "password");
    enableAndSet(*pOptions, SvtSecurityOptions::E_DOCWARN_RECOMMENDPASSWORD, *m_pRecommPasswdCB,
        *get<FixedImage>("lockpassword"));
    get(m_pCtrlHyperlinkCB, "ctrlclick");
    enableAndSet(*pOptions, SvtSecurityOptions::E_CTRLCLICK_HYPERLINK, *m_pCtrlHyperlinkCB,
        *get<FixedImage>("lockctrlclick"));
}

SecurityOptionsDialog::~SecurityOptionsDialog()
{
}

//........................................................................
}   // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
