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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_SECURITYENVIRONMENT_NSSIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_SECURITYENVIRONMENT_NSSIMPL_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Exception.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/security/CertificateCharacters.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <osl/mutex.hxx>

#include "pk11func.h"
#include "keyhi.h"
#include "certdb.h"
#include "list"

#include "xmlsecurity/xmlsec-wrapper.h"

class SecurityEnvironment_NssImpl : public ::cppu::WeakImplHelper<
    ::com::sun::star::xml::crypto::XSecurityEnvironment ,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel >
{
private:

    std::list< PK11SlotInfo* > m_Slots;
    typedef std::list< PK11SlotInfo* >::const_iterator CIT_SLOTS;

    osl::Mutex m_mutex;

        CERTCertDBHandle*                   m_pHandler ;
        std::list< PK11SymKey* >            m_tSymKeyList ;
        std::list< SECKEYPublicKey* >       m_tPubKeyList ;
        std::list< SECKEYPrivateKey* >      m_tPriKeyList ;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager ;

    public:
        SecurityEnvironment_NssImpl();
        virtual ~SecurityEnvironment_NssImpl();

        //Methods from XSecurityEnvironment

        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE ;

        virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE ;

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE ;

        //Helper for XServiceInfo
        static ::com::sun::star::uno::Sequence< OUString > impl_getSupportedServiceNames() ;

        static OUString impl_getImplementationName() throw( ::com::sun::star::uno::RuntimeException ) ;

        //Helper for registry
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aServiceManager ) throw( ::com::sun::star::uno::RuntimeException ) ;

        static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > impl_createFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aServiceManager ) ;

        virtual ::sal_Int32 SAL_CALL verifyCertificate(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::security::XCertificate >& xCert,
            const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > > &
            intermediateCerts)
            throw (::com::sun::star::uno::SecurityException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;

        virtual ::sal_Int32 SAL_CALL getCertificateCharacters( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& xCert ) throw (::com::sun::star::uno::SecurityException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;

        virtual OUString SAL_CALL getSecurityEnvironmentInformation(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        //Methods from XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId() ;

        //Native methods
        CERTCertDBHandle* getCertDb() throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        void setCertDb( CERTCertDBHandle* aCertDb ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        void adoptSymKey( PK11SymKey* aSymKey ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;
        PK11SymKey* getSymKey( unsigned int position ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        SECKEYPublicKey* getPubKey( unsigned int position ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        SECKEYPrivateKey* getPriKey( unsigned int position ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > > SAL_CALL getPersonalCertificates() throw(  ::com::sun::star::uno::SecurityException , ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE ;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > SAL_CALL getCertificate( const OUString& issuerName, const ::com::sun::star::uno::Sequence< sal_Int8 >& serialNumber ) throw( ::com::sun::star::uno::SecurityException , ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE ;

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > > SAL_CALL buildCertificatePath( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& beginCert ) throw(  ::com::sun::star::uno::SecurityException , ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE ;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > SAL_CALL createCertificateFromRaw( const ::com::sun::star::uno::Sequence< sal_Int8 >& rawCertificate ) throw( ::com::sun::star::uno::SecurityException , ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE ;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > SAL_CALL createCertificateFromAscii( const OUString& asciiCertificate ) throw( ::com::sun::star::uno::SecurityException , ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE ;


        //Native methods
        xmlSecKeysMngrPtr createKeysManager() throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;
        static void destroyKeysManager(xmlSecKeysMngrPtr pKeysMngr) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

private:
        void updateSlots();

          void addCryptoSlot( PK11SlotInfo* aSlot ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;
} ;

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_SECURITYENVIRONMENT_NSSIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
