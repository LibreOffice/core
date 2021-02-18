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
#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XCertificateCreator.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <osl/mutex.hxx>

#include <keythi.h>
#include <certt.h>

#include <string_view>
#include <vector>

#include <xmlsec-wrapper.h>

namespace com::sun::star::security { class XCertificate; }
class X509Certificate_NssImpl;

class SecurityEnvironment_NssImpl : public ::cppu::WeakImplHelper<
    css::xml::crypto::XSecurityEnvironment,
    css::xml::crypto::XCertificateCreator,
    css::lang::XServiceInfo,
    css::lang::XUnoTunnel >
{
private:

    std::vector< PK11SlotInfo* > m_Slots;
    /// The last used certificate which has the private key for signing.
    css::uno::Reference<css::security::XCertificate> m_xSigningCertificate;

    osl::Mutex m_mutex;

        CERTCertDBHandle*                   m_pHandler ;
        std::vector< PK11SymKey* >          m_tSymKeyList ;

    public:
        SecurityEnvironment_NssImpl();
        virtual ~SecurityEnvironment_NssImpl() override;

        //Methods from XSecurityEnvironment

        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override ;

        virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) override ;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override ;

        virtual ::sal_Int32 SAL_CALL verifyCertificate(
            const css::uno::Reference<
            css::security::XCertificate >& xCert,
            const css::uno::Sequence<
            css::uno::Reference< css::security::XCertificate > > &
            intermediateCerts) override ;

        virtual ::sal_Int32 SAL_CALL getCertificateCharacters( const css::uno::Reference< css::security::XCertificate >& xCert ) override ;

        virtual OUString SAL_CALL getSecurityEnvironmentInformation(  ) override;

        //Methods from XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

        static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId() ;

        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        void setCertDb( CERTCertDBHandle* aCertDb ) ;

        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        void adoptSymKey( PK11SymKey* aSymKey ) ;

        virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL getPersonalCertificates() override ;
        virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL getAllCertificates() override
        { return css::uno::Sequence< css::uno::Reference< css::security::XCertificate > >(); }

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL getCertificate( const OUString& issuerName, const css::uno::Sequence< sal_Int8 >& serialNumber ) override ;

        virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL buildCertificatePath( const css::uno::Reference< css::security::XCertificate >& beginCert ) override ;

        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL createCertificateFromRaw( const css::uno::Sequence< sal_Int8 >& rawCertificate ) override ;
        virtual css::uno::Reference< css::security::XCertificate > SAL_CALL createCertificateFromAscii( const OUString& asciiCertificate ) override ;

        // Methods of XCertificateCreator
        css::uno::Reference<css::security::XCertificate> SAL_CALL addDERCertificateToTheDatabase(
                css::uno::Sequence<sal_Int8> const & raDERCertificate,
                OUString const & raTrustString) override;

        css::uno::Reference<css::security::XCertificate> SAL_CALL createDERCertificateWithPrivateKey(
                css::uno::Sequence<sal_Int8> const & raDERCertificate,
                css::uno::Sequence<sal_Int8> const & raPrivateKey) override;

        //Native methods
        /// @throws css::uno::RuntimeException
        xmlSecKeysMngrPtr createKeysManager() ;
        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        static void destroyKeysManager(xmlSecKeysMngrPtr pKeysMngr) ;

private:

        void updateSlots();

        static rtl::Reference<X509Certificate_NssImpl> createAndAddCertificateFromPackage(
                                    const css::uno::Sequence<sal_Int8>& raDerCertificate,
                                    std::u16string_view raString);
        static SECKEYPrivateKey* insertPrivateKey(css::uno::Sequence<sal_Int8> const & raPrivateKey);

        static rtl::Reference<X509Certificate_NssImpl> createX509CertificateFromDER(const css::uno::Sequence<sal_Int8>& raDerCertificate);

          /// @throws css::uno::Exception
          /// @throws css::uno::RuntimeException
          void addCryptoSlot( PK11SlotInfo* aSlot ) ;
} ;

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_SECURITYENVIRONMENT_NSSIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
