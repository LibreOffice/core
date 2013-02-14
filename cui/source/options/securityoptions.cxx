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
#include "securityoptions.hrc"

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

    ,m_aSaveOrSendDocsFI( this, CUI_RES( FI_SAVESENDDOCS ) )
    ,m_aSaveOrSendDocsCB( this, CUI_RES( CB_SAVESENDDOCS ) )
    ,m_aSignDocsFI      ( this, CUI_RES( FI_SIGNDOCS ) )
    ,m_aSignDocsCB      ( this, CUI_RES( CB_SIGNDOCS ) )
    ,m_aPrintDocsFI     ( this, CUI_RES( FI_PRINTDOCS ) )
    ,m_aPrintDocsCB     ( this, CUI_RES( CB_PRINTDOCS ) )
    ,m_aCreatePdfFI     ( this, CUI_RES( FI_CREATEPDF ) )
    ,m_aCreatePdfCB     ( this, CUI_RES( CB_CREATEPDF ) )
    ,m_aRemovePersInfoFI( this, CUI_RES( FI_REMOVEINFO ) )
    ,m_aRemovePersInfoCB( this, CUI_RES( CB_REMOVEINFO ) )
    ,m_aRecommPasswdFI  ( this, CUI_RES( FI_RECOMMENDPWD ) )
    ,m_aRecommPasswdCB  ( this, CUI_RES( CB_RECOMMENDPWD ) )
    ,m_aCtrlHyperlinkFI ( this, CUI_RES( FI_CTRLHYPERLINK ) )
    ,m_aCtrlHyperlinkCB ( this, CUI_RES( CB_CTRLHYPERLINK ) )

{

    DBG_ASSERT( pOptions, "SecurityOptionsDialog::SecurityOptionsDialog(): invalid SvtSecurityOptions" );
    enableAndSet( *pOptions, SvtSecurityOptions::E_DOCWARN_SAVEORSEND, m_aSaveOrSendDocsCB, m_aSaveOrSendDocsFI );
    enableAndSet( *pOptions, SvtSecurityOptions::E_DOCWARN_SIGNING, m_aSignDocsCB, m_aSignDocsFI );
    enableAndSet( *pOptions, SvtSecurityOptions::E_DOCWARN_PRINT, m_aPrintDocsCB, m_aPrintDocsFI );
    enableAndSet( *pOptions, SvtSecurityOptions::E_DOCWARN_CREATEPDF, m_aCreatePdfCB, m_aCreatePdfFI );
    enableAndSet( *pOptions, SvtSecurityOptions::E_DOCWARN_REMOVEPERSONALINFO, m_aRemovePersInfoCB, m_aRemovePersInfoFI );
    enableAndSet( *pOptions, SvtSecurityOptions::E_DOCWARN_RECOMMENDPASSWORD, m_aRecommPasswdCB, m_aRecommPasswdFI );
    enableAndSet( *pOptions, SvtSecurityOptions::E_CTRLCLICK_HYPERLINK, m_aCtrlHyperlinkCB, m_aCtrlHyperlinkFI );
}

SecurityOptionsDialog::~SecurityOptionsDialog()
{
}

//........................................................................
}   // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
