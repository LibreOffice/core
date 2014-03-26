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

#ifndef _HIERARCHYDATASOURCE_HXX
#define _HIERARCHYDATASOURCE_HXX

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
                            public com::sun::star::lang::XServiceInfo,
                            public com::sun::star::lang::XTypeProvider,
                            public com::sun::star::lang::XComponent,
                            public com::sun::star::lang::XMultiServiceFactory
{
    osl::Mutex m_aMutex;
    com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext > m_xContext;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > m_xConfigProvider;
    cppu::OInterfaceContainerHelper * m_pDisposeEventListeners;

public:
    HierarchyDataSource( const com::sun::star::uno::Reference<
                            com::sun::star::uno::XComponentContext > &
                                rxContext );
    virtual ~HierarchyDataSource();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL acquire()
        throw() SAL_OVERRIDE;
    virtual void SAL_CALL release()
        throw() SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    static css::uno::Reference< css::lang::XSingleServiceFactory >
    createServiceFactory( const css::uno::Reference<
                          css::lang::XMultiServiceFactory >& rxServiceMgr );

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL
    dispose()
        throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XEventListener > & xListener )
        throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener > & aListener )
        throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XMultiServiceFactory
    virtual com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > SAL_CALL
    createInstance( const OUString & aServiceSpecifier )
        throw ( com::sun::star::uno::Exception,
                com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > SAL_CALL
    createInstanceWithArguments( const OUString & ServiceSpecifier,
                                 const com::sun::star::uno::Sequence<
                                    com::sun::star::uno::Any > & Arguments )
        throw ( com::sun::star::uno::Exception,
                com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
    getAvailableServiceNames()
        throw ( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // Non-Interface methods

private:
    com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > SAL_CALL
    createInstanceWithArguments( const OUString & ServiceSpecifier,
                                 const com::sun::star::uno::Sequence<
                                    com::sun::star::uno::Any > & Arguments,
                                 bool bCheckArgs )
        throw ( com::sun::star::uno::Exception,
                com::sun::star::uno::RuntimeException );

    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >
    getConfigProvider();

    bool
    createConfigPath( const OUString & rInPath, OUString & rOutPath );
};

} // namespace hierarchy_ucp

#endif /* !_HIERARCHYDATASOURCE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
