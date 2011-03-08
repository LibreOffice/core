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
#ifndef UUI_UNKNOWNAUTH_HXX
#define UUI_UNKNOWNAUTH_HXX


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
//= Https_UADialog
//=====================================================================
class UnknownAuthDialog : public ModalDialog
{
private:
    PushButton  m_aCommandButtonOK;
    CancelButton  m_aCommandButtonCancel;
    HelpButton  m_aCommandButtonHelp;
    PushButton  m_aView_Certificate;
    RadioButton m_aOptionButtonAccept;
    RadioButton m_aOptionButtonDontAccept;
    FixedLine   m_aLine;
    FixedText   m_aLabel1;
    FixedImage  m_aWarnImage;

    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& m_xServiceFactory;
    const cssu::Reference< dcss::security::XCertificate >& m_rXCert;
    Window* m_pParent;
    ResMgr* pResourceMgr;

    DECL_LINK( OKHdl_Impl, PushButton * );
    DECL_LINK( ViewCertHdl_Impl, PushButton * );

    public:
    UnknownAuthDialog( Window* pParent,
                       const cssu::Reference< dcss::security::XCertificate >& rXCert,
                       const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
                       ResMgr * pResMgr );

    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > getServiceFactory() { return m_xServiceFactory; };

    cssu::Reference< dcss::security::XCertificate > getCert() { return m_rXCert; };

    Window* getParent() { return m_pParent; };

    void setDescriptionText( const rtl::OUString &aText ) { m_aLabel1.SetText( aText ); };

};

#endif // UUI_UNKNOWNAUTH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
