/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: sslwarndlg.cxx,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: rt $ $Date: 2007-11-07 10:10:43 $
*
*  The Contents of this file are made available subject to
*  the terms of GNU Lesser General Public License Version 2.1.
*
*
*    GNU Lesser General Public License Version 2.1
*    =============================================
*    Copyright 2005 by Sun Microsystems, Inc.
*    901 San Antonio Road, Palo Alto, CA 94303, USA
*
*    This library is free software; you can redistribute it and/or
*    modify it under the terms of the GNU Lesser General Public
*    License version 2.1, as published by the Free Software Foundation.
*
*    This library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    Lesser General Public License for more details.
*
*    You should have received a copy of the GNU Lesser General Public
*    License along with this library; if not, write to the Free Software
*    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*    MA  02111-1307  USA
*
************************************************************************/

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef UUI_IDS_HRC
#include <ids.hrc>
#endif
#ifndef UUI_UNKNOWNAUTHDLG_HRC
#include <sslwarndlg.hrc>
#endif
#ifndef UUI_UNKNOWNAUTHDLG_HXX
#include <sslwarndlg.hxx>
#endif

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
                    getServiceFactory().get()->createInstance( rtl::OUString::createFromAscii( "com.sun.star.security.DocumentDigitalSignatures" )), uno::UNO_QUERY );

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



