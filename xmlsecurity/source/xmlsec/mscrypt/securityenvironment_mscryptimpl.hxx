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

#pragma once

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <wincrypt.h>
#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <comphelper/servicehelper.hxx>
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

#include <vector>
#include <xmlsec-wrapper.h>

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
        bool                                m_bEnableDefault ;

        //Service manager
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xServiceManager ;

    public:
        explicit SecurityEnvironment_MSCryptImpl( const css::uno::Reference< css::uno::XComponentContext >& xContext ) ;
        virtual ~SecurityEnvironment_MSCryptImpl() override;

        //Methods from XSecurityEnvironment
        virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL getPersonalCertificates() override;
        virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL getAllCertificates() override
        { return css::uno::Sequence< css::uno::Reference< css::security::XCertificate > >(); }

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL getCertificate(
            const OUString& issuerName,
            const css::uno::Sequence< sal_Int8 >& serialNumber ) override;

        /// @throws css::uno::SecurityException
        /// @throws css::uno::RuntimeException
        virtual css::uno::Reference< css::security::XCertificate > getCertificate(
            const OUString& issuerName,
            const OUString& serialNumber ) ;

        virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL buildCertificatePath(
            const css::uno::Reference< css::security::XCertificate >& beginCert ) override;

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL createCertificateFromRaw(
            const css::uno::Sequence< sal_Int8 >& rawCertificate ) override;

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL createCertificateFromAscii(
            const OUString& asciiCertificate ) override;

        virtual ::sal_Int32 SAL_CALL verifyCertificate(
            const css::uno::Reference< css::security::XCertificate >& xCert,
            const css::uno::Sequence< css::uno::Reference<
            css::security::XCertificate > >& intermediateCertificates) override;

        virtual ::sal_Int32 SAL_CALL getCertificateCharacters(
            const css::uno::Reference< css::security::XCertificate >& xCert ) override;

        virtual OUString SAL_CALL getSecurityEnvironmentInformation(  ) override;


        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;

        virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        //Methods from XUnoTunnel
        UNO3_GETIMPLEMENTATION_DECL(SecurityEnvironment_MSCryptImpl)

        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        HCRYPTPROV getCryptoProvider() ;
        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        void setCryptoProvider( HCRYPTPROV aProv ) ;

        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        LPCTSTR getKeyContainer() ;
        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        void setKeyContainer( LPCTSTR aKeyContainer ) ;

        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        HCERTSTORE getCryptoSlot() ;
        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        void setCryptoSlot( HCERTSTORE aKeyStore ) ;

        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        HCERTSTORE getCertDb() ;
        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        void setCertDb( HCERTSTORE aCertDb ) ;

        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        void enableDefaultCrypt( bool enable ) ;
        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        bool defaultEnabled() ;

        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        xmlSecKeysMngrPtr createKeysManager() ;

        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        static void destroyKeysManager(xmlSecKeysMngrPtr pKeysMngr) ;
} ;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
