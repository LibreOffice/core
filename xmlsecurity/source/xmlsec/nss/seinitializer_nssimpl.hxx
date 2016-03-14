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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_SEINITIALIZER_NSSIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_SEINITIALIZER_NSSIMPL_HXX

#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>

#include <cppuhelper/implbase.hxx>

#include <libxml/tree.h>

#include "nssinitializer.hxx"

class SEInitializer_NssImpl : public cppu::ImplInheritanceHelper
<
    ONSSInitializer,
    css::xml::crypto::XSEInitializer
>
{
public:
    explicit SEInitializer_NssImpl(const css::uno::Reference<css::uno::XComponentContext > &rxContext);
    virtual ~SEInitializer_NssImpl();

    /* XSEInitializer */
    virtual css::uno::Reference< css::xml::crypto::XXMLSecurityContext >
        SAL_CALL createSecurityContext( const OUString& )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL freeSecurityContext( const css::uno::Reference<
        css::xml::crypto::XXMLSecurityContext >& securityContext )
        throw (css::uno::RuntimeException, std::exception) override;

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (css::uno::RuntimeException, std::exception) override;
};

OUString SEInitializer_NssImpl_getImplementationName()
    throw ( css::uno::RuntimeException );

css::uno::Sequence< OUString > SAL_CALL SEInitializer_NssImpl_getSupportedServiceNames(  )
    throw ( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface >
SAL_CALL SEInitializer_NssImpl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & rxMSF)
    throw ( css::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
