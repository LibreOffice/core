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

#include <vcl/msgbox.hxx>

#include <ids.hrc>
#include <unknownauthdlg.hrc>
#include <unknownauthdlg.hxx>
#include <comphelper/componentcontext.hxx>

#include <com/sun/star/security/DocumentDigitalSignatures.hpp>

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(UnknownAuthDialog, OKHdl_Impl)
{
    if ( m_aOptionButtonAccept.IsChecked() )
    {
        EndDialog( RET_OK );
    } else
    {
        EndDialog( RET_CANCEL );
    }

    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(UnknownAuthDialog, ViewCertHdl_Impl)
{
    uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures > xDocumentDigitalSignatures;

    xDocumentDigitalSignatures = uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures >(
                    ::com::sun::star::security::DocumentDigitalSignatures::createDefault(comphelper::ComponentContext(getServiceFactory()).getUNOContext()) );

    xDocumentDigitalSignatures.get()->showCertificate(getCert());

    return 0;
}

// -----------------------------------------------------------------------

UnknownAuthDialog::UnknownAuthDialog
(
    Window*                                     pParent,
    const cssu::Reference< dcss::security::XCertificate >& rXCert,
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    ResMgr*                                     pResMgr
) :
    ModalDialog( pParent, ResId( DLG_UUI_UNKNOWNAUTH, *pResMgr ) ),

    m_aCommandButtonOK ( this, ResId( PB_OK, *pResMgr ) ),
    m_aCommandButtonCancel ( this, ResId( PB_CANCEL, *pResMgr ) ),
    m_aCommandButtonHelp ( this, ResId( PB_HELP, *pResMgr ) ),
    m_aView_Certificate ( this, ResId( PB_VIEW__CERTIFICATE, *pResMgr ) ),
    m_aOptionButtonAccept ( this, ResId( RB_ACCEPT_1, *pResMgr ) ),
    m_aOptionButtonDontAccept ( this, ResId( RB_DONTACCEPT_2, *pResMgr ) ),
    m_aLine ( this, ResId( FL_LINE, *pResMgr ) ),
    m_aLabel1 ( this, ResId( FT_LABEL_1, *pResMgr ) ),
    m_aWarnImage ( this, ResId( IMG_WARN, *pResMgr ) ),
    m_xServiceFactory ( xServiceFactory ),
    m_rXCert ( rXCert )
{
    FreeResource();

    m_aWarnImage.SetImage( WarningBox::GetStandardImage() );
    m_pParent = pParent;
    m_aView_Certificate.SetClickHdl( LINK( this, UnknownAuthDialog, ViewCertHdl_Impl ) );
    m_aCommandButtonOK.SetClickHdl( LINK( this, UnknownAuthDialog, OKHdl_Impl ) );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
