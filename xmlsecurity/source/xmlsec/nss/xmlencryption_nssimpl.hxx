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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_XMLENCRYPTION_NSSIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_XMLENCRYPTION_NSSIMPL_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Exception.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/XXMLEncryption.hpp>
#include <com/sun/star/xml/crypto/XXMLEncryptionTemplate.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>

class XMLEncryption_NssImpl : public ::cppu::WeakImplHelper<
    css::xml::crypto::XXMLEncryption ,
    css::lang::XServiceInfo >
{
    public:
        explicit XMLEncryption_NssImpl();
        virtual ~XMLEncryption_NssImpl() ;

        //Methods from XXMLEncryption
        virtual css::uno::Reference< css::xml::crypto::XXMLEncryptionTemplate > SAL_CALL encrypt(
            const css::uno::Reference< css::xml::crypto::XXMLEncryptionTemplate >& aTemplate ,
            const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& aEnvironment)
            // ) throw( css::uno::Exception , css::uno::RuntimeException ) ;
            throw (css::xml::crypto::XMLEncryptionException ,
                   css::uno::SecurityException,
                   css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Reference< css::xml::crypto::XXMLEncryptionTemplate > SAL_CALL decrypt(
            const css::uno::Reference< css::xml::crypto::XXMLEncryptionTemplate >& aTemplate ,
            const css::uno::Reference< css::xml::crypto::XXMLSecurityContext >& aContext
        ) throw (css::xml::crypto::XMLEncryptionException ,
                 css::uno::SecurityException,
                 css::uno::RuntimeException, std::exception) override;

        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override ;

        virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) throw( css::uno::RuntimeException, std::exception ) override ;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override ;

        //Helper for XServiceInfo
        static css::uno::Sequence< OUString > impl_getSupportedServiceNames() ;

        static OUString impl_getImplementationName() throw( css::uno::RuntimeException ) ;

        //Helper for registry
        static css::uno::Reference< css::uno::XInterface > SAL_CALL impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& aServiceManager ) throw( css::uno::RuntimeException ) ;

        static css::uno::Reference< css::lang::XSingleServiceFactory > impl_createFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& aServiceManager ) ;
} ;

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_XMLENCRYPTION_NSSIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
