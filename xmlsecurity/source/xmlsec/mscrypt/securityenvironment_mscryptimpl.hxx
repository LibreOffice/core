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
    css::xml::crypto::XSecurityEnvironment ,
    css::lang::XServiceInfo,
    css::lang::XUnoTunnel >
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
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xServiceManager ;

    public:
        explicit SecurityEnvironment_MSCryptImpl( const css::uno::Reference< css::lang::XMultiServiceFactory >& aFactory ) ;
        virtual ~SecurityEnvironment_MSCryptImpl() ;

        //Methods from XSecurityEnvironment
        virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL getPersonalCertificates()
            throw(  css::uno::SecurityException , css::uno::RuntimeException ) ;

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL getCertificate(
            const OUString& issuerName,
            const css::uno::Sequence< sal_Int8 >& serialNumber )
            throw( css::uno::SecurityException , css::uno::RuntimeException ) ;

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL getCertificate(
            const OUString& issuerName,
            const OUString& serialNumber )
            throw( css::uno::SecurityException , css::uno::RuntimeException ) ;

        virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL buildCertificatePath(
            const css::uno::Reference< css::security::XCertificate >& beginCert )
            throw(  css::uno::SecurityException , css::uno::RuntimeException ) ;

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL createCertificateFromRaw(
            const css::uno::Sequence< sal_Int8 >& rawCertificate )
            throw( css::uno::SecurityException , css::uno::RuntimeException ) ;

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL createCertificateFromAscii(
            const OUString& asciiCertificate )
            throw( css::uno::SecurityException , css::uno::RuntimeException ) ;

        virtual ::sal_Int32 SAL_CALL verifyCertificate(
            const css::uno::Reference< css::security::XCertificate >& xCert,
            const css::uno::Sequence< css::uno::Reference<
            css::security::XCertificate > >& intermediateCertificates)
            throw (css::uno::SecurityException, css::uno::RuntimeException) ;

        virtual ::sal_Int32 SAL_CALL getCertificateCharacters(
            const css::uno::Reference< css::security::XCertificate >& xCert )
            throw (css::uno::SecurityException, css::uno::RuntimeException) ;

        virtual OUString SAL_CALL getSecurityEnvironmentInformation(  )
            throw (css::uno::RuntimeException);


        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException ) ;

        virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) throw( css::uno::RuntimeException ) ;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException ) ;

        //Helper for XServiceInfo
        static css::uno::Sequence< OUString > impl_getSupportedServiceNames() ;

        static OUString impl_getImplementationName() throw( css::uno::RuntimeException ) ;

        //Helper for registry
        static css::uno::Reference< css::uno::XInterface > SAL_CALL impl_createInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& aServiceManager )
            throw( css::uno::RuntimeException ) ;

        static css::uno::Reference< css::lang::XSingleServiceFactory > impl_createFactory(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& aServiceManager ) ;

        //Methods from XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
            throw (css::uno::RuntimeException);

        static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId() ;
        static SecurityEnvironment_MSCryptImpl* getImplementation( const css::uno::Reference< css::uno::XInterface >& rObj ) ;

        //Native methods
        virtual HCRYPTPROV getCryptoProvider() throw( css::uno::Exception , css::uno::RuntimeException ) ;
        virtual void setCryptoProvider( HCRYPTPROV aProv ) throw( css::uno::Exception , css::uno::RuntimeException ) ;

        virtual LPCTSTR getKeyContainer() throw( css::uno::Exception , css::uno::RuntimeException ) ;
        virtual void setKeyContainer( LPCTSTR aKeyContainer ) throw( css::uno::Exception , css::uno::RuntimeException ) ;

        virtual HCERTSTORE getCryptoSlot() throw( css::uno::Exception , css::uno::RuntimeException ) ;
        virtual void setCryptoSlot( HCERTSTORE aKeyStore ) throw( css::uno::Exception , css::uno::RuntimeException ) ;

        virtual HCERTSTORE getCertDb() throw( css::uno::Exception , css::uno::RuntimeException ) ;
        virtual void setCertDb( HCERTSTORE aCertDb ) throw( css::uno::Exception , css::uno::RuntimeException ) ;

        virtual void adoptSymKey( HCRYPTKEY aSymKey ) throw( css::uno::Exception , css::uno::RuntimeException ) ;
        virtual HCRYPTKEY getSymKey( unsigned int position ) throw( css::uno::Exception , css::uno::RuntimeException ) ;

        virtual HCRYPTKEY getPubKey( unsigned int position ) throw( css::uno::Exception , css::uno::RuntimeException ) ;

        virtual HCRYPTKEY getPriKey( unsigned int position ) throw( css::uno::Exception , css::uno::RuntimeException ) ;

        virtual void enableDefaultCrypt( sal_Bool enable ) throw( css::uno::Exception , css::uno::RuntimeException ) ;
        virtual sal_Bool defaultEnabled() throw( css::uno::Exception , css::uno::RuntimeException ) ;

        //Native methods
        virtual xmlSecKeysMngrPtr createKeysManager() throw( css::uno::Exception , css::uno::RuntimeException ) ;

        virtual void destroyKeysManager(xmlSecKeysMngrPtr pKeysMngr) throw( css::uno::Exception , css::uno::RuntimeException ) ;
} ;

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_SECURITYENVIRONMENT_MSCRYPTIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
