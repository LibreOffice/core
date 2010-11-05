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

#include <vcl/msgbox.hxx>

#include <ids.hrc>
#include <sslwarndlg.hrc>
#include <sslwarndlg.hxx>

#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>

// -----------------------------------------------------------------------

IMPL_LINK( SSLWarnDialog, OKHdl_Impl, PushButton *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( SSLWarnDialog, ViewCertHdl_Impl, PushButton *, EMPTYARG )
{
    uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures > xDocumentDigitalSignatures;

    xDocumentDigitalSignatures = uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures >(
                    getServiceFactory().get()->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.security.DocumentDigitalSignatures" ))), uno::UNO_QUERY );

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
    m_rXCert ( rXCert ),
    pResourceMgr    ( pResMgr )
{
    FreeResource();
    m_aWarnImage.SetImage( WarningBox::GetStandardImage() );
    m_pParent = pParent;
    m_aCommandButtonViewCert.SetClickHdl( LINK( this, SSLWarnDialog, ViewCertHdl_Impl ) );
    m_aOkButton.SetClickHdl( LINK( this, SSLWarnDialog, OKHdl_Impl ) );
};



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
