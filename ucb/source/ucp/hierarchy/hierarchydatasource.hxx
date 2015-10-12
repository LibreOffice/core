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

#ifndef INCLUDED_UCB_SOURCE_UCP_HIERARCHY_HIERARCHYDATASOURCE_HXX
#define INCLUDED_UCB_SOURCE_UCP_HIERARCHY_HIERARCHYDATASOURCE_HXX

#include <osl/mutex.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>

namespace cppu { class OInterfaceContainerHelper; }

namespace hierarchy_ucp {



class HierarchyDataSource : public cppu::OWeakObject,
                            public css::lang::XServiceInfo,
                            public css::lang::XTypeProvider,
                            public css::lang::XComponent,
                            public css::lang::XMultiServiceFactory
{
    osl::Mutex m_aMutex;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xConfigProvider;
    cppu::OInterfaceContainerHelper * m_pDisposeEventListeners;

public:
    explicit HierarchyDataSource( const css::uno::Reference< css::uno::XComponentContext > & rxContext );
    virtual ~HierarchyDataSource();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    static css::uno::Reference< css::lang::XSingleServiceFactory > createServiceFactory( const css::uno::Reference<
                          css::lang::XMultiServiceFactory >& rxServiceMgr );

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XComponent
    virtual void SAL_CALL dispose()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener > & xListener )
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener > & aListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString & aServiceSpecifier )
        throw ( css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString & ServiceSpecifier,
                                 const css::uno::Sequence<
                                    css::uno::Any > & Arguments )
        throw ( css::uno::Exception,
                css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // Non-Interface methods

private:
    css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString & ServiceSpecifier,
                                 const css::uno::Sequence<
                                    css::uno::Any > & Arguments,
                                 bool bCheckArgs )
        throw ( css::uno::Exception, css::uno::RuntimeException );

    css::uno::Reference< css::lang::XMultiServiceFactory > getConfigProvider();

    static bool createConfigPath( const OUString & rInPath, OUString & rOutPath );
};

} // namespace hierarchy_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_HIERARCHY_HIERARCHYDATASOURCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
