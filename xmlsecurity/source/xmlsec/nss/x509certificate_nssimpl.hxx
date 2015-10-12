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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_X509CERTIFICATE_NSSIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_X509CERTIFICATE_NSSIMPL_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/SecurityException.hpp>
#include <com/sun/star/security/XCertificate.hpp>

#include "cert.h"

class X509Certificate_NssImpl : public ::cppu::WeakImplHelper<
    ::com::sun::star::security::XCertificate ,
    ::com::sun::star::lang::XUnoTunnel >
{
    private:
        CERTCertificate* m_pCert ;

    public:
        X509Certificate_NssImpl() ;
        virtual ~X509Certificate_NssImpl() ;

        //Methods from XCertificate
        virtual sal_Int16 SAL_CALL getVersion(  ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSerialNumber(  ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual OUString SAL_CALL getIssuerName(  ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual OUString SAL_CALL getSubjectName(  ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual ::com::sun::star::util::DateTime SAL_CALL getNotValidBefore(  ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual ::com::sun::star::util::DateTime SAL_CALL getNotValidAfter(  ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getIssuerUniqueID(  ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSubjectUniqueID(  ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > > SAL_CALL getExtensions(  ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > SAL_CALL findCertificateExtension( const ::com::sun::star::uno::Sequence< sal_Int8 >& oid ) throw (::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getEncoded(  ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual OUString SAL_CALL getSubjectPublicKeyAlgorithm()
            throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSubjectPublicKeyValue()
            throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual OUString SAL_CALL getSignatureAlgorithm()
            throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSHA1Thumbprint()
            throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getMD5Thumbprint()
            throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        virtual sal_Int32 SAL_CALL getCertificateUsage( ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) override ;

        //Methods from XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (com::sun::star::uno::RuntimeException, std::exception) override;

        static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId() ;

        //Helper methods
        void setCert( CERTCertificate* cert ) ;
        const CERTCertificate* getNssCert() const ;
        void setRawCert( const ::com::sun::star::uno::Sequence< sal_Int8 >& rawCert ) throw ( ::com::sun::star::uno::RuntimeException) ;
} ;

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_X509CERTIFICATE_NSSIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
