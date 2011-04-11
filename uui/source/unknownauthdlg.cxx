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
#include <unknownauthdlg.hrc>
#include <unknownauthdlg.hxx>

#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>

// -----------------------------------------------------------------------

IMPL_LINK( UnknownAuthDialog, OKHdl_Impl, PushButton *, EMPTYARG )
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

IMPL_LINK( UnknownAuthDialog, ViewCertHdl_Impl, PushButton *, EMPTYARG )
{
    uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures > xDocumentDigitalSignatures;

    xDocumentDigitalSignatures = uno::Reference< ::com::sun::star::security::XDocumentDigitalSignatures >(
                    getServiceFactory().get()->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.security.DocumentDigitalSignatures" ))), uno::UNO_QUERY );

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
    m_rXCert ( rXCert ),
    pResourceMgr ( pResMgr )
{
    FreeResource();

    m_aWarnImage.SetImage( WarningBox::GetStandardImage() );
    m_pParent = pParent;
    m_aView_Certificate.SetClickHdl( LINK( this, UnknownAuthDialog, ViewCertHdl_Impl ) );
    m_aCommandButtonOK.SetClickHdl( LINK( this, UnknownAuthDialog, OKHdl_Impl ) );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
