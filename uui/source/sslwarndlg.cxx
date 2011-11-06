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



