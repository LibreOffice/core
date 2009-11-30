/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: securityoptions.cxx,v $
 * $Revision: 1.3 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#ifndef INCLUDED_SVTOOLS_SECURITIYOPTIONS_HXX
#include <svtools/securityoptions.hxx>
#endif
#include <svtools/stdctrl.hxx>
#include <svx/dialmgr.hxx>
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif

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
     ModalDialog( pParent, SVX_RES( RID_SVXDLG_SECURITY_OPTIONS ) )
    ,m_aWarningsFL      ( this, SVX_RES( FL_WARNINGS ) )
    ,m_aWarningsFI      ( this, SVX_RES( FI_WARNINGS ) )
    ,m_aSaveOrSendDocsFI( this, SVX_RES( FI_SAVESENDDOCS ) )
    ,m_aSaveOrSendDocsCB( this, SVX_RES( CB_SAVESENDDOCS ) )
    ,m_aSignDocsFI      ( this, SVX_RES( FI_SIGNDOCS ) )
    ,m_aSignDocsCB      ( this, SVX_RES( CB_SIGNDOCS ) )
    ,m_aPrintDocsFI     ( this, SVX_RES( FI_PRINTDOCS ) )
    ,m_aPrintDocsCB     ( this, SVX_RES( CB_PRINTDOCS ) )
    ,m_aCreatePdfFI     ( this, SVX_RES( FI_CREATEPDF ) )
    ,m_aCreatePdfCB     ( this, SVX_RES( CB_CREATEPDF ) )
    ,m_aOptionsFL       ( this, SVX_RES( FL_OPTIONS ) )
    ,m_aRemovePersInfoFI( this, SVX_RES( FI_REMOVEINFO ) )
    ,m_aRemovePersInfoCB( this, SVX_RES( CB_REMOVEINFO ) )
    ,m_aRecommPasswdFI  ( this, SVX_RES( FI_RECOMMENDPWD ) )
    ,m_aRecommPasswdCB  ( this, SVX_RES( CB_RECOMMENDPWD ) )
    ,m_aCtrlHyperlinkFI ( this, SVX_RES( FI_CTRLHYPERLINK ) )
    ,m_aCtrlHyperlinkCB ( this, SVX_RES( CB_CTRLHYPERLINK ) )

    ,m_aButtonsFL       ( this, SVX_RES( FL_BUTTONS ) )
    ,m_aOKBtn           ( this, SVX_RES( PB_OK ) )
    ,m_aCancelBtn       ( this, SVX_RES( PB_CANCEL ) )
    ,m_aHelpBtn         ( this, SVX_RES( PB_HELP ) )

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

