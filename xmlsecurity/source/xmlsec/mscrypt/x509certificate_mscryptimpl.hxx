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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_X509CERTIFICATE_MSCRYPTIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_X509CERTIFICATE_MSCRYPTIMPL_HXX

#ifdef _MSC_VER
#pragma warning(push,1)
#endif
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <WinCrypt.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/SecurityException.hpp>
#include <com/sun/star/security/CertificateKind.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <certificate.hxx>

class X509Certificate_MSCryptImpl : public ::cppu::WeakImplHelper<
    css::security::XCertificate ,
    css::lang::XUnoTunnel > , public xmlsecurity::Certificate
{
    private:
        const CERT_CONTEXT* m_pCertContext ;

    public:
        X509Certificate_MSCryptImpl() ;
        virtual ~X509Certificate_MSCryptImpl() override;

        //Methods from XCertificate
        virtual sal_Int16 SAL_CALL getVersion() override;

        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getSerialNumber() override;
        virtual OUString SAL_CALL getIssuerName() override;
        virtual OUString SAL_CALL getSubjectName() override;
        virtual css::util::DateTime SAL_CALL getNotValidBefore() override;
        virtual css::util::DateTime SAL_CALL getNotValidAfter() override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getIssuerUniqueID() override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getSubjectUniqueID() override;
        virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificateExtension > > SAL_CALL getExtensions() override;
        virtual css::uno::Reference< css::security::XCertificateExtension > SAL_CALL findCertificateExtension( const css::uno::Sequence< sal_Int8 >& oid ) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getEncoded() override;
        virtual OUString SAL_CALL getSubjectPublicKeyAlgorithm() override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getSubjectPublicKeyValue() override;
        virtual OUString SAL_CALL getSignatureAlgorithm() override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getSHA1Thumbprint() override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getMD5Thumbprint() override;
        virtual css::security::CertificateKind SAL_CALL getCertificateKind() override;


        virtual sal_Int32 SAL_CALL getCertificateUsage( ) override;

        //Methods from XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

        /// @see xmlsecurity::Certificate::getSHA256Thumbprint().
        virtual css::uno::Sequence<sal_Int8> getSHA256Thumbprint() override;

        static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId() ;
        static X509Certificate_MSCryptImpl* getImplementation( const css::uno::Reference< css::uno::XInterface >& rObj ) ;

        //Helper methods
        void setMswcryCert( const CERT_CONTEXT* cert ) ;
        const CERT_CONTEXT* getMswcryCert() const ;
        /// @throws css::uno::RuntimeException
        void setRawCert( css::uno::Sequence< sal_Int8 > const & rawCert ) ;
} ;

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_X509CERTIFICATE_MSCRYPTIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
