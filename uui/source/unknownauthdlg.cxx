/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: unknownauthdlg.cxx,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: rt $ $Date: 2007-11-07 10:11:44 $
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
#include <unknownauthdlg.hrc>
#endif
#ifndef UUI_UNKNOWNAUTHDLG_HXX
#include <unknownauthdlg.hxx>
#endif

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
                    getServiceFactory().get()->createInstance( rtl::OUString::createFromAscii( "com.sun.star.security.DocumentDigitalSignatures" )), uno::UNO_QUERY );

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
    //SetMapMode( MapMode( MAP_APPFONT ) );
    FreeResource();

    m_aWarnImage.SetImage( WarningBox::GetStandardImage() );
    m_pParent = pParent;
    m_aView_Certificate.SetClickHdl( LINK( this, UnknownAuthDialog, ViewCertHdl_Impl ) );
    m_aCommandButtonOK.SetClickHdl( LINK( this, UnknownAuthDialog, OKHdl_Impl ) );
};

