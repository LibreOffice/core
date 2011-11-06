/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <vcl/msgbox.hxx>

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

