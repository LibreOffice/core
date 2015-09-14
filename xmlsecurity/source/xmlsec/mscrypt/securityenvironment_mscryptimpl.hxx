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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_SECURITYENVIRONMENT_MSCRYPTIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_SECURITYENVIRONMENT_MSCRYPTIMPL_HXX

#ifdef _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#include <wincrypt.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
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

#include <list>
#include "xmlsecurity/xmlsec-wrapper.h"

#include <sal/types.h>


class SecurityEnvironment_MSCryptImpl : public ::cppu::WeakImplHelper<
    ::com::sun::star::xml::crypto::XSecurityEnvironment ,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel >
{
    private:
        //crypto provider and key container
        HCRYPTPROV                          m_hProv ;
        LPCTSTR                             m_pszContainer ;

        //Key store
        HCERTSTORE                          m_hKeyStore ;

        //Certificate store
        HCERTSTORE                          m_hCertStore ;

        // i120675, save the store handles
        HCERTSTORE                          m_hMySystemStore;
        HCERTSTORE                          m_hRootSystemStore;
        HCERTSTORE                          m_hTrustSystemStore;
        HCERTSTORE                          m_hCaSystemStore;

        //Enable default system cryptography setting
        sal_Bool                            m_bEnableDefault ;

        //External keys
        std::list< HCRYPTKEY >              m_tSymKeyList ;
        std::list< HCRYPTKEY >              m_tPubKeyList ;
        std::list< HCRYPTKEY >              m_tPriKeyList ;

        //Service manager
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager ;

    public:
        explicit SecurityEnvironment_MSCryptImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aFactory ) ;
        virtual ~SecurityEnvironment_MSCryptImpl() ;

        //Methods from XSecurityEnvironment
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > > SAL_CALL getPersonalCertificates()
            throw(  ::com::sun::star::uno::SecurityException , ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > SAL_CALL getCertificate(
            const OUString& issuerName,
            const ::com::sun::star::uno::Sequence< sal_Int8 >& serialNumber )
            throw( ::com::sun::star::uno::SecurityException , ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > SAL_CALL getCertificate(
            const OUString& issuerName,
            const OUString& serialNumber )
            throw( ::com::sun::star::uno::SecurityException , ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > > SAL_CALL buildCertificatePath(
            const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& beginCert )
            throw(  ::com::sun::star::uno::SecurityException , ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > SAL_CALL createCertificateFromRaw(
            const ::com::sun::star::uno::Sequence< sal_Int8 >& rawCertificate )
            throw( ::com::sun::star::uno::SecurityException , ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate > SAL_CALL createCertificateFromAscii(
            const OUString& asciiCertificate )
            throw( ::com::sun::star::uno::SecurityException , ::com::sun::star::uno::RuntimeException ) ;

        virtual ::sal_Int32 SAL_CALL verifyCertificate(
            const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& xCert,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
            ::com::sun::star::security::XCertificate > >& intermediateCertificates)
            throw (::com::sun::star::uno::SecurityException, ::com::sun::star::uno::RuntimeException) ;

        virtual ::sal_Int32 SAL_CALL getCertificateCharacters(
            const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& xCert )
            throw (::com::sun::star::uno::SecurityException, ::com::sun::star::uno::RuntimeException) ;

        virtual OUString SAL_CALL getSecurityEnvironmentInformation(  )
            throw (::com::sun::star::uno::RuntimeException);


        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException ) ;

        virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) throw( ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException ) ;

        //Helper for XServiceInfo
        static ::com::sun::star::uno::Sequence< OUString > impl_getSupportedServiceNames() ;

        static OUString impl_getImplementationName() throw( ::com::sun::star::uno::RuntimeException ) ;

        //Helper for registry
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_createInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aServiceManager )
            throw( ::com::sun::star::uno::RuntimeException ) ;

        static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > impl_createFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aServiceManager ) ;

        //Methods from XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
            throw (::com::sun::star::uno::RuntimeException);

        static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId() ;
        static SecurityEnvironment_MSCryptImpl* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rObj ) ;

        //Native methods
        virtual HCRYPTPROV getCryptoProvider() throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;
        virtual void setCryptoProvider( HCRYPTPROV aProv ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        virtual LPCTSTR getKeyContainer() throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;
        virtual void setKeyContainer( LPCTSTR aKeyContainer ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        virtual HCERTSTORE getCryptoSlot() throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;
        virtual void setCryptoSlot( HCERTSTORE aKeyStore ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        virtual HCERTSTORE getCertDb() throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;
        virtual void setCertDb( HCERTSTORE aCertDb ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        virtual void adoptSymKey( HCRYPTKEY aSymKey ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;
        virtual HCRYPTKEY getSymKey( unsigned int position ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        virtual HCRYPTKEY getPubKey( unsigned int position ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        virtual HCRYPTKEY getPriKey( unsigned int position ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        virtual void enableDefaultCrypt( sal_Bool enable ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;
        virtual sal_Bool defaultEnabled() throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        //Native methods
        virtual xmlSecKeysMngrPtr createKeysManager() throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        virtual void destroyKeysManager(xmlSecKeysMngrPtr pKeysMngr) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;
} ;

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_SECURITYENVIRONMENT_MSCRYPTIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
