/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _HIERARCHYDATASOURCE_HXX
#define _HIERARCHYDATASOURCE_HXX

#include <osl/mutex.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>

namespace cppu { class OInterfaceContainerHelper; }

namespace hierarchy_ucp {

//=========================================================================

class HierarchyDataSource : public cppu::OWeakObject,
                            public com::sun::star::lang::XServiceInfo,
                            public com::sun::star::lang::XTypeProvider,
                            public com::sun::star::lang::XComponent,
                            public com::sun::star::lang::XMultiServiceFactory
{
    osl::Mutex m_aMutex;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > m_xConfigProvider;
    cppu::OInterfaceContainerHelper * m_pDisposeEventListeners;

public:
    HierarchyDataSource( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XMultiServiceFactory > &
                                rxServiceMgr );
    virtual ~HierarchyDataSource();

    // XInterface
    XINTERFACE_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XComponent
    virtual void SAL_CALL
    dispose()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XEventListener > & xListener )
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener > & aListener )
        throw ( com::sun::star::uno::RuntimeException );

    // XMultiServiceFactory
    virtual com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > SAL_CALL
    createInstance( const rtl::OUString & aServiceSpecifier )
        throw ( com::sun::star::uno::Exception,
                com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > SAL_CALL
    createInstanceWithArguments( const rtl::OUString & ServiceSpecifier,
                                 const com::sun::star::uno::Sequence<
                                    com::sun::star::uno::Any > & Arguments )
        throw ( com::sun::star::uno::Exception,
                com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getAvailableServiceNames()
        throw ( com::sun::star::uno::RuntimeException );

    // Non-Interface methods

private:
    com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > SAL_CALL
    createInstanceWithArguments( const rtl::OUString & ServiceSpecifier,
                                 const com::sun::star::uno::Sequence<
                                    com::sun::star::uno::Any > & Arguments,
                                 bool bCheckArgs )
        throw ( com::sun::star::uno::Exception,
                com::sun::star::uno::RuntimeException );

    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >
    getConfigProvider();

    bool
    createConfigPath( const rtl::OUString & rInPath, rtl::OUString & rOutPath );
};

} // namespace hierarchy_ucp

#endif /* !_HIERARCHYDATASOURCE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
