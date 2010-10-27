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

#ifndef _X509CERTIFICATE_NSSIMPL_HXX_
#define _X509CERTIFICATE_NSSIMPL_HXX_

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include "com/sun/star/uno/SecurityException.hpp"
#include <com/sun/star/security/XCertificate.hpp>

#include "cert.h"

class X509Certificate_NssImpl : public ::cppu::WeakImplHelper2<
    ::com::sun::star::security::XCertificate ,
    ::com::sun::star::lang::XUnoTunnel >
{
    private :
        CERTCertificate* m_pCert ;

    public :
        X509Certificate_NssImpl() ;
        virtual ~X509Certificate_NssImpl() ;

        //Methods from XCertificate
        virtual sal_Int16 SAL_CALL getVersion(  ) throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSerialNumber(  ) throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::rtl::OUString SAL_CALL getIssuerName(  ) throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::rtl::OUString SAL_CALL getSubjectName(  ) throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::util::DateTime SAL_CALL getNotValidBefore(  ) throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::util::DateTime SAL_CALL getNotValidAfter(  ) throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getIssuerUniqueID(  ) throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSubjectUniqueID(  ) throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > > SAL_CALL getExtensions(  ) throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > SAL_CALL findCertificateExtension( const ::com::sun::star::uno::Sequence< sal_Int8 >& oid ) throw (::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getEncoded(  ) throw ( ::com::sun::star::uno::RuntimeException) ;

        // MM : added by MM
        virtual ::rtl::OUString SAL_CALL getSubjectPublicKeyAlgorithm()
            throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSubjectPublicKeyValue()
            throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::rtl::OUString SAL_CALL getSignatureAlgorithm()
            throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSHA1Thumbprint()
            throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getMD5Thumbprint()
            throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual sal_Int32 SAL_CALL getCertificateUsage( ) throw ( ::com::sun::star::uno::RuntimeException) ;
        // MM : end

        //Methods from XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (com::sun::star::uno::RuntimeException);

        static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId() ;
        static X509Certificate_NssImpl* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xObj ) ;

        //Helper methods
        void setCert( CERTCertificate* cert ) ;
        const CERTCertificate* getNssCert() const ;
        void setRawCert( ::com::sun::star::uno::Sequence< sal_Int8 > rawCert ) throw ( ::com::sun::star::uno::RuntimeException) ;
} ;

#endif  // _X509CERTIFICATE_NSSIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
