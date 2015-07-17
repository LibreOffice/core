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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_XMLSECURITYCONTEXT_NSSIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_XMLSECURITYCONTEXT_NSSIMPL_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Exception.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>

#include <vector>

class XMLSecurityContext_NssImpl : public ::cppu::WeakImplHelper2<
    ::com::sun::star::xml::crypto::XXMLSecurityContext ,
    ::com::sun::star::lang::XServiceInfo >
{
    private:
        //xmlSecKeysMngrPtr m_pKeysMngr ;
        //::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > m_xSecurityEnvironment ;
        std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > > m_vSecurityEnvironments;

        sal_Int32 m_nDefaultEnvIndex;

    public:
        XMLSecurityContext_NssImpl() ;
        virtual ~XMLSecurityContext_NssImpl() ;

        //Methods from XXMLSecurityContext
        virtual sal_Int32 SAL_CALL addSecurityEnvironment(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment >& aSecurityEnvironment
            ) throw (::com::sun::star::security::SecurityInfrastructureException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual ::sal_Int32 SAL_CALL getSecurityEnvironmentNumber(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::crypto::XSecurityEnvironment > SAL_CALL
            getSecurityEnvironmentByIndex( ::sal_Int32 index )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::crypto::XSecurityEnvironment > SAL_CALL
            getSecurityEnvironment(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual ::sal_Int32 SAL_CALL getDefaultSecurityEnvironmentIndex(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual void SAL_CALL setDefaultSecurityEnvironmentIndex( sal_Int32 nDefaultEnvIndex )
            throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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

        /*
         * Because of the issue of multi-securityenvironment, so the keyManager method is not useful any longer.
         *

        //Methods from XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw (com::sun::star::uno::RuntimeException);

        static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId() ;
        static XMLSecurityContext_NssImpl* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xObj ) ;

        //Native methods
        virtual xmlSecKeysMngrPtr keysManager() throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        */
} ;

#endif // INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_XMLSECURITYCONTEXT_NSSIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
