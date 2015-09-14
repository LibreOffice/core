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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_NSSINITIALIZER_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_NSSINITIALIZER_HXX

#include <com/sun/star/xml/crypto/XNSSInitializer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <cppuhelper/implbase.hxx>

#define NSS_SERVICE_NAME "com.sun.star.xml.crypto.NSSInitializer"

class ONSSInitializer : public cppu::WeakImplHelper
<
    ::com::sun::star::xml::crypto::XNSSInitializer,
    ::com::sun::star::lang::XServiceInfo
>
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

    ONSSInitializer()
    {}

public:
    explicit ONSSInitializer(const css::uno::Reference<css::uno::XComponentContext> &rxContext);
    virtual ~ONSSInitializer();

    static bool initNSS( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext );

    /* XDigestContextSupplier */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XDigestContext > SAL_CALL getDigestContext( ::sal_Int32 nDigestID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aParams ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /* XCipherContextSupplier */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext > SAL_CALL getCipherContext( ::sal_Int32 nCipherID, const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aKey, const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aInitializationVector, sal_Bool bEncryption, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aParams ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

OUString ONSSInitializer_getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< OUString > SAL_CALL ONSSInitializer_getSupportedServiceNames()
    throw ( ::com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL ONSSInitializer_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr )
    throw ( ::com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
