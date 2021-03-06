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

#include <osl/mutex.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <memory>

namespace comphelper { class OInterfaceContainerHelper2; }

namespace hierarchy_ucp {


class HierarchyDataSource : public cppu::WeakImplHelper<
                                css::lang::XServiceInfo,
                                css::lang::XComponent,
                                css::lang::XMultiServiceFactory>
{
    osl::Mutex m_aMutex;
    css::uno::Reference< css::uno::XComponentContext >     m_xContext;
    css::uno::Reference< css::lang::XMultiServiceFactory >  m_xConfigProvider;
    std::unique_ptr<comphelper::OInterfaceContainerHelper2> m_pDisposeEventListeners;

public:
    explicit HierarchyDataSource( const css::uno::Reference< css::uno::XComponentContext > & rxContext );
    virtual ~HierarchyDataSource() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener > & xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener > & aListener ) override;

    // XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString & aServiceSpecifier ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString & ServiceSpecifier,
                                 const css::uno::Sequence<
                                    css::uno::Any > & Arguments ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() override;

    // Non-Interface methods

private:
    /// @throws css::uno::Exception
    css::uno::Reference< css::uno::XInterface > createInstanceWithArguments( std::u16string_view ServiceSpecifier,
                                 const css::uno::Sequence<
                                    css::uno::Any > & Arguments,
                                 bool bCheckArgs );

    css::uno::Reference< css::lang::XMultiServiceFactory > getConfigProvider();

    static bool createConfigPath( const OUString & rInPath, OUString & rOutPath );
};

} // namespace hierarchy_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
