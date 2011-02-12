/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// include ---------------------------------------------------------------

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

SecurityOptionsDialog::SecurityOptionsDialog( Window* pParent, SvtSecurityOptions* pOptions ) :
     ModalDialog( pParent, CUI_RES( RID_SVXDLG_SECURITY_OPTIONS ) )
    ,m_aWarningsFL      ( this, CUI_RES( FL_WARNINGS ) )
    ,m_aWarningsFI      ( this, CUI_RES( FI_WARNINGS ) )
    ,m_aSaveOrSendDocsFI( this, CUI_RES( FI_SAVESENDDOCS ) )
    ,m_aSaveOrSendDocsCB( this, CUI_RES( CB_SAVESENDDOCS ) )
    ,m_aSignDocsFI      ( this, CUI_RES( FI_SIGNDOCS ) )
    ,m_aSignDocsCB      ( this, CUI_RES( CB_SIGNDOCS ) )
    ,m_aPrintDocsFI     ( this, CUI_RES( FI_PRINTDOCS ) )
    ,m_aPrintDocsCB     ( this, CUI_RES( CB_PRINTDOCS ) )
    ,m_aCreatePdfFI     ( this, CUI_RES( FI_CREATEPDF ) )
    ,m_aCreatePdfCB     ( this, CUI_RES( CB_CREATEPDF ) )
    ,m_aOptionsFL       ( this, CUI_RES( FL_OPTIONS ) )
    ,m_aRemovePersInfoFI( this, CUI_RES( FI_REMOVEINFO ) )
    ,m_aRemovePersInfoCB( this, CUI_RES( CB_REMOVEINFO ) )
    ,m_aRecommPasswdFI  ( this, CUI_RES( FI_RECOMMENDPWD ) )
    ,m_aRecommPasswdCB  ( this, CUI_RES( CB_RECOMMENDPWD ) )
    ,m_aCtrlHyperlinkFI ( this, CUI_RES( FI_CTRLHYPERLINK ) )
    ,m_aCtrlHyperlinkCB ( this, CUI_RES( CB_CTRLHYPERLINK ) )

    ,m_aButtonsFL       ( this, CUI_RES( FL_BUTTONS ) )
    ,m_aOKBtn           ( this, CUI_RES( PB_OK ) )
    ,m_aCancelBtn       ( this, CUI_RES( PB_CANCEL ) )
    ,m_aHelpBtn         ( this, CUI_RES( PB_HELP ) )

{
    FreeResource();

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
