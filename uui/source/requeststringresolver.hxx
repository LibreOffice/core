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

class UUIInteractionHelper;

class UUIInteractionRequestStringResolver:
    public cppu::WeakImplHelper<
        com::sun::star::lang::XServiceInfo,
        com::sun::star::task::XInteractionRequestStringResolver >
{
public:
    static char const m_aImplementationName[];

    static com::sun::star::uno::Sequence< OUString >
    getSupportedServiceNames_static();

    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL
    createInstance(
        com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > const &
    rServiceFactory);

private:
    UUIInteractionHelper * m_pImpl;

    UUIInteractionRequestStringResolver(UUIInteractionRequestStringResolver &) SAL_DELETED_FUNCTION;
    void operator =(UUIInteractionRequestStringResolver&) SAL_DELETED_FUNCTION;

    explicit UUIInteractionRequestStringResolver(
        com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext >
                const & rxContext);

    virtual ~UUIInteractionRequestStringResolver();

    virtual OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService(OUString const &
                          rServiceName)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::beans::Optional< OUString > SAL_CALL
    getStringFromInformationalRequest(
        const com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest >& Request )
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif // INCLUDED_UUI_SOURCE_REQUESTSTRINGRESOLVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
