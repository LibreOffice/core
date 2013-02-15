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
#include <vcl/msgbox.hxx>

#include <ids.hrc>
#include <masterpasscrtdlg.hxx>

// MasterPasswordCreateDialog---------------------------------------------------

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(MasterPasswordCreateDialog, EditHdl_Impl)
{
    m_pOKBtn->Enable( m_pEDMasterPasswordCrt->GetText().getLength() >= nMinLen );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(MasterPasswordCreateDialog, OKHdl_Impl)
{
    // compare both passwords and show message box if there are not equal!!
    if( m_pEDMasterPasswordCrt->GetText() == m_pEDMasterPasswordRepeat->GetText() )
        EndDialog( RET_OK );
    else
    {
        String aErrorMsg( ResId( STR_ERROR_PASSWORDS_NOT_IDENTICAL, *pResourceMgr ));
        ErrorBox aErrorBox( this, WB_OK, aErrorMsg );
        aErrorBox.Execute();
        m_pEDMasterPasswordCrt->SetText( String() );
        m_pEDMasterPasswordRepeat->SetText( String() );
        m_pEDMasterPasswordCrt->GrabFocus();
    }
    return 1;
}

// -----------------------------------------------------------------------

MasterPasswordCreateDialog::MasterPasswordCreateDialog(Window* pParent, ResMgr* pResMgr)
    : ModalDialog(pParent, "SetMasterPasswordDialog", "uui/ui/setmasterpassworddlg.ui")
    , pResourceMgr(pResMgr)
    , nMinLen(1)
{
    get(m_pEDMasterPasswordCrt, "password1");
    get(m_pEDMasterPasswordRepeat, "password2");
    get(m_pOKBtn, "ok");
    m_pOKBtn->Enable( sal_False );
    m_pOKBtn->SetClickHdl( LINK( this, MasterPasswordCreateDialog, OKHdl_Impl ) );
    m_pEDMasterPasswordCrt->SetModifyHdl( LINK( this, MasterPasswordCreateDialog, EditHdl_Impl ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
