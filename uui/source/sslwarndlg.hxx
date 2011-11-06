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


#ifndef UUI_SSLWARN_HXX
#define UUI_SSLWARN_HXX


#include <vcl/dialog.hxx>

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/security/XCertificate.hpp>
#ifndef _COM_SUN_STAR_XML_CRYPTO_XXSECURITYENVIRONMENT_HPP_
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#endif
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace cssu = com::sun::star::uno;
namespace dcss = ::com::sun::star;


using namespace com::sun::star;

//=====================================================================
//= Https_WarnDialog
//=====================================================================
class SSLWarnDialog : public ModalDialog
{
private:
    FixedText       m_aLabel1;
    PushButton      m_aOkButton;
    CancelButton    m_aCancelButton;
    PushButton      m_aCommandButtonViewCert;
    FixedLine       m_aLine;
    FixedImage      m_aWarnImage;


    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& m_xServiceFactory;
    const cssu::Reference< dcss::security::XCertificate >& m_rXCert;
    Window* m_pParent;
    ResMgr*     pResourceMgr;

    DECL_LINK( OKHdl_Impl, PushButton * );
    DECL_LINK( ViewCertHdl_Impl, PushButton * );

    public:
    SSLWarnDialog( Window* pParent,
                       const cssu::Reference< dcss::security::XCertificate >& rXCert,
                       const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
                       ResMgr * pResMgr );

    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > getServiceFactory() { return m_xServiceFactory; };

    cssu::Reference< dcss::security::XCertificate > getCert() { return m_rXCert; };

    Window* getParent() { return m_pParent; };

    void setDescription1Text( const rtl::OUString &aText ) { m_aLabel1.SetText( aText ); };
};

#endif // UUI_SSLWARN_HXX

