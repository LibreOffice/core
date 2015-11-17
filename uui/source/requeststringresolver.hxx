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

#ifndef INCLUDED_UUI_SOURCE_REQUESTSTRINGRESOLVER_HXX
#define INCLUDED_UUI_SOURCE_REQUESTSTRINGRESOLVER_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XInteractionRequestStringResolver.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <memory>

class UUIInteractionHelper;

class UUIInteractionRequestStringResolver:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::task::XInteractionRequestStringResolver >
{
public:
    static char const m_aImplementationName[];

    static css::uno::Sequence< OUString >
    getSupportedServiceNames_static();

    static css::uno::Reference< css::uno::XInterface >
    SAL_CALL
    createInstance(
        css::uno::Reference< css::lang::XMultiServiceFactory > const &
    rServiceFactory);

private:
    std::unique_ptr<UUIInteractionHelper> m_pImpl;

    UUIInteractionRequestStringResolver(UUIInteractionRequestStringResolver &) = delete;
    void operator =(UUIInteractionRequestStringResolver&) = delete;

    explicit UUIInteractionRequestStringResolver(
        css::uno::Reference< css::uno::XComponentContext > const & rxContext);

    virtual ~UUIInteractionRequestStringResolver();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService(OUString const &
                          rServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::beans::Optional< OUString > SAL_CALL
    getStringFromInformationalRequest(
        const css::uno::Reference< css::task::XInteractionRequest >& Request )
        throw (css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_UUI_SOURCE_REQUESTSTRINGRESOLVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
