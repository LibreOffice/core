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
#ifndef UUI_SSLWARN_HXX
#define UUI_SSLWARN_HXX


#include <vcl/dialog.hxx>

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
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

    DECL_LINK(OKHdl_Impl, void *);
    DECL_LINK(ViewCertHdl_Impl, void *);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
