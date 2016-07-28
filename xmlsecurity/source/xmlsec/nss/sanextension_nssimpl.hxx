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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_SANEXTENSION_NSSIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_SANEXTENSION_NSSIMPL_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/SecurityException.hpp>
#include <com/sun/star/security/XCertificateExtension.hpp>
#include <com/sun/star/security/XSanExtension.hpp>
#include <com/sun/star/security/CertAltNameEntry.hpp>

class SanExtensionImpl : public ::cppu::WeakImplHelper<
    css::security::XSanExtension >
{
    private:
        bool m_critical ;
        css::uno::Sequence< sal_Int8 > m_xExtnId ;
        css::uno::Sequence< sal_Int8 > m_xExtnValue ;
        css::uno::Sequence< css::security::CertAltNameEntry > m_Entries;

        static OString removeOIDFromString( const OString &oid);

    public:
        SanExtensionImpl() ;
        virtual ~SanExtensionImpl() ;

        //Methods from XCertificateExtension
        virtual sal_Bool SAL_CALL isCritical() throw( css::uno::RuntimeException, std::exception ) override ;

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getExtensionId() throw( css::uno::RuntimeException, std::exception ) override ;

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getExtensionValue() throw( css::uno::RuntimeException, std::exception ) override ;

        void setCertExtn( unsigned char* value, unsigned int vlen, unsigned char* id, unsigned int idlen, bool critical ) ;

        //Methods from XSanExtension

        virtual css::uno::Sequence< css::security::CertAltNameEntry > SAL_CALL getAlternativeNames() throw( css::uno::RuntimeException, std::exception ) override ;
} ;

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_SANEXTENSION_NSSIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
