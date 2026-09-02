/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <wincrypt.h>
#include <sal/config.h>

#include <xmlsec/keysmngr.h>

#include <rtl/ustring.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cpo/uno/Exception.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/security/CertificateCharacters.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>

#include <vector>

#include <sal/types.h>


class SecurityEnvironment_MSCryptImpl : public ::cppu::WeakImplHelper<
    css::xml::crypto::XSecurityEnvironment ,
    css::lang::XServiceInfo >
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
        bool                                m_bEnableDefault ;

        //Service manager
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xServiceManager ;

    public:
        explicit SecurityEnvironment_MSCryptImpl( const css::uno::Reference< cpo::uno::XComponentContext >& xContext ) ;
        virtual ~SecurityEnvironment_MSCryptImpl() override;

        //Methods from XSecurityEnvironment
        virtual cpo::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL getPersonalCertificates() override;
        virtual cpo::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL getAllCertificates() override
        { return cpo::uno::Sequence< css::uno::Reference< css::security::XCertificate > >(); }

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL getCertificate(
            const OUString& issuerName,
            const cpo::uno::Sequence< sal_Int8 >& serialNumber ) override;

        /// @throws cpo::uno::SecurityException
        /// @throws cpo::uno::RuntimeException
        virtual css::uno::Reference< css::security::XCertificate > getCertificate(
            const OUString& issuerName,
            const OUString& serialNumber ) ;

        virtual cpo::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL buildCertificatePath(
            const css::uno::Reference< css::security::XCertificate >& beginCert ) override;

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL createCertificateFromRaw(
            const cpo::uno::Sequence< sal_Int8 >& rawCertificate ) override;

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL createCertificateFromAscii(
            const OUString& asciiCertificate ) override;

        virtual ::sal_Int32 SAL_CALL verifyCertificate(
            const css::uno::Reference< css::security::XCertificate >& xCert,
            const cpo::uno::Sequence< css::uno::Reference<
            css::security::XCertificate > >& intermediateCertificates) override;

        virtual ::sal_Int32 SAL_CALL getCertificateCharacters(
            const css::uno::Reference< css::security::XCertificate >& xCert ) override;

        virtual OUString SAL_CALL getSecurityEnvironmentInformation(  ) override;


        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;

        virtual bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) override;

        virtual cpo::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        HCRYPTPROV getCryptoProvider() ;
        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        void setCryptoProvider( HCRYPTPROV aProv ) ;

        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        LPCTSTR getKeyContainer() ;
        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        void setKeyContainer( LPCTSTR aKeyContainer ) ;

        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        HCERTSTORE getCryptoSlot() ;
        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        void setCryptoSlot( HCERTSTORE aKeyStore ) ;

        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        HCERTSTORE getCertDb() ;
        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        void setCertDb( HCERTSTORE aCertDb ) ;

        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        void enableDefaultCrypt( bool enable ) ;
        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        bool defaultEnabled() ;

        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        xmlSecKeysMngrPtr createKeysManager() ;

        /// @throws cpo::uno::Exception
        /// @throws cpo::uno::RuntimeException
        static void destroyKeysManager(xmlSecKeysMngrPtr pKeysMngr) ;
} ;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
