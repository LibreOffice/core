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
    css::xml::crypto::XNSSInitializer,
    css::lang::XServiceInfo
>
{
protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    ONSSInitializer()
    {}

public:
    explicit ONSSInitializer(const css::uno::Reference<css::uno::XComponentContext> &rxContext);
    virtual ~ONSSInitializer() override;

    static bool initNSS( const css::uno::Reference< css::uno::XComponentContext > &rxContext );

    /* XDigestContextSupplier */
    virtual css::uno::Reference< css::xml::crypto::XDigestContext > SAL_CALL getDigestContext( ::sal_Int32 nDigestID, const css::uno::Sequence< css::beans::NamedValue >& aParams ) override;

    /* XCipherContextSupplier */
    virtual css::uno::Reference< css::xml::crypto::XCipherContext > SAL_CALL getCipherContext( ::sal_Int32 nCipherID, const css::uno::Sequence< ::sal_Int8 >& aKey, const css::uno::Sequence< ::sal_Int8 >& aInitializationVector, sal_Bool bEncryption, const css::uno::Sequence< css::beans::NamedValue >& aParams ) override;

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

/// @throws css::uno::RuntimeException
OUString ONSSInitializer_getImplementationName();

/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > SAL_CALL ONSSInitializer_getSupportedServiceNames();

/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface >
SAL_CALL ONSSInitializer_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
