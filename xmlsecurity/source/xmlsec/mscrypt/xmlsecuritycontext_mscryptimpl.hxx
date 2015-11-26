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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_XMLSECURITYCONTEXT_MSCRYPTIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_XMLSECURITYCONTEXT_MSCRYPTIMPL_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Exception.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>


class XMLSecurityContext_MSCryptImpl : public ::cppu::WeakImplHelper<
    css::xml::crypto::XXMLSecurityContext ,
    css::lang::XServiceInfo >
{
    private:
        //xmlSecKeysMngrPtr m_pKeysMngr ;
        css::uno::Reference< css::xml::crypto::XSecurityEnvironment > m_xSecurityEnvironment ;

    public:
        XMLSecurityContext_MSCryptImpl();
        virtual ~XMLSecurityContext_MSCryptImpl();

        //Methods from XXMLSecurityContext
        virtual sal_Int32 SAL_CALL addSecurityEnvironment(
            const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& aSecurityEnvironment
            ) throw (css::security::SecurityInfrastructureException, css::uno::RuntimeException);

        virtual ::sal_Int32 SAL_CALL getSecurityEnvironmentNumber(  )
            throw (css::uno::RuntimeException);

        virtual css::uno::Reference<
            css::xml::crypto::XSecurityEnvironment > SAL_CALL
            getSecurityEnvironmentByIndex( ::sal_Int32 index )
            throw (css::uno::RuntimeException);

        virtual css::uno::Reference<
            css::xml::crypto::XSecurityEnvironment > SAL_CALL
            getSecurityEnvironment(  )
            throw (css::uno::RuntimeException);

        virtual ::sal_Int32 SAL_CALL getDefaultSecurityEnvironmentIndex(  )
            throw (css::uno::RuntimeException);

        virtual void SAL_CALL setDefaultSecurityEnvironmentIndex( sal_Int32 nDefaultEnvIndex )
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
        static css::uno::Reference< css::uno::XInterface > SAL_CALL impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& aServiceManager ) throw( css::uno::RuntimeException ) ;

        static css::uno::Reference< css::lang::XSingleServiceFactory > impl_createFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& aServiceManager ) ;

} ;

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_XMLSECURITYCONTEXT_MSCRYPTIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
