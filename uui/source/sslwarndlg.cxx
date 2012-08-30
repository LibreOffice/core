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
#include <sslwarndlg.hrc>
#include <sslwarndlg.hxx>

#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SSLWarnDialog, OKHdl_Impl)
{
    EndDialog( RET_OK );
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SSLWarnDialog, ViewCertHdl_Impl)
{
    uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures > xDocumentDigitalSignatures;

    xDocumentDigitalSignatures = uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures >(
                    getServiceFactory().get()->createInstance( rtl::OUString( "com.sun.star.security.DocumentDigitalSignatures" )), uno::UNO_QUERY );

    xDocumentDigitalSignatures.get()->showCertificate(getCert());

    return 0;
}

// -----------------------------------------------------------------------

SSLWarnDialog::SSLWarnDialog
(
    Window*                                     pParent,
    const cssu::Reference< dcss::security::XCertificate >& rXCert,
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    ResMgr*                                     pResMgr
) :

    ModalDialog( pParent, ResId( DLG_UUI_SSLWARN, *pResMgr ) ),
    m_aLabel1 ( this, ResId( FT_LABEL_1, *pResMgr ) ),
    m_aOkButton ( this, ResId( PB_OK, *pResMgr ) ),
    m_aCancelButton ( this, ResId( PB_CANCEL, *pResMgr ) ),
    m_aCommandButtonViewCert ( this, ResId( PB_VIEW__CERTIFICATE, *pResMgr ) ),
    m_aLine ( this, ResId( FL_LINE, *pResMgr ) ),
    m_aWarnImage ( this, ResId( IMG_WARN, *pResMgr ) ),
    m_xServiceFactory ( xServiceFactory ),
    m_rXCert ( rXCert )
{
    FreeResource();
    m_aWarnImage.SetImage( WarningBox::GetStandardImage() );
    m_pParent = pParent;
    m_aCommandButtonViewCert.SetClickHdl( LINK( this, SSLWarnDialog, ViewCertHdl_Impl ) );
    m_aOkButton.SetClickHdl( LINK( this, SSLWarnDialog, OKHdl_Impl ) );
};



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
