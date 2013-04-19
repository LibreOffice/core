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
#ifndef _CPPUHELPER_SHLIB_HXX_
#define _CPPUHELPER_SHLIB_HXX_

#include <osl/module.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/registry/CannotRegisterImplementationException.hpp>
#include "cppuhelperdllapi.h"


namespace cppu
{

// Note the pointless redundancy
// "::com::sun::star::lang::XSingleComponentFactory or
// ::com::sun::star::lang::XSingleComponentFactory" in the doc
// comments below. Whether the documentation is supposed to mean only
// XSingleComponentFactory, or whether the other one should be
// something else, I don't know.

/** Loads a shared library component and gets the factory out of it.  You can give either a
    fully qualified libname or single lib name.  The libname need not be pre/postfixed
    (e.g. xxx.dll).  You can give parameter rPath to force lookup of the library in a specific
    directory.  The resulting path of the library will be checked against environment variable
    CPLD_ACCESSPATH if set.

    @param rLibName name of the library
    @param rPath optional path
    @param rImplName implementation to be retrieved from the library
    @param xMgr service manager to be provided to the component
    @param xKey registry key to be provided to the component
    @return
    factory instance (com::sun::star::lang::XSingleComponentFactory or
    com::sun::star::lang::XSingleComponentFactory)
*/
CPPUHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL loadSharedLibComponentFactory(
    ::rtl::OUString const & rLibName, ::rtl::OUString const & rPath,
    ::rtl::OUString const & rImplName,
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > const & xKey )
    SAL_THROW( (::com::sun::star::loader::CannotActivateFactoryException) );

/** Loads a shared library component and gets the factory out of it.  You can give either a
    fully qualified libname or single lib name.  The libname need not be pre/postfixed
    (e.g. xxx.dll).  You can give parameter rPath to force lookup of the library in a specific
    directory.  The resulting path of the library will be checked against environment variable
    CPLD_ACCESSPATH if set. An optional 'prefix' parameter is used to determine the symbol
    name of the entry point in the library.

    @param rLibName name of the library
    @param rPath optional path
    @param rImplName implementation to be retrieved from the library
    @param xMgr service manager to be provided to the component
    @param xKey registry key to be provided to the component
    @param rPrefix optional component prefix
    @return
    factory instance (com::sun::star::lang::XSingleComponentFactory or
    com::sun::star::lang::XSingleComponentFactory)
*/
CPPUHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL loadSharedLibComponentFactory(
    ::rtl::OUString const & rLibName, ::rtl::OUString const & rPath,
    ::rtl::OUString const & rImplName,
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > const & xKey,
    ::rtl::OUString const & rPrefix )
    SAL_THROW( (::com::sun::star::loader::CannotActivateFactoryException) );

/** Gets the factory out of an already loaded (for instance statically linked) component.

    @param pGetter the component's component_getFactory function
    @param rImplName implementation to be retrieved from the library
    @param xMgr service manager to be provided to the component
    @param xKey registry key to be provided to the component
    @param rPrefix optional component prefix
    @return
    factory instance (com::sun::star::lang::XSingleComponentFactory or
    com::sun::star::lang::XSingleComponentFactory)
*/
CPPUHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL invokeStaticComponentFactory(
    oslGenericFunction pGetter,
    ::rtl::OUString const & rImplName,
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > const & xKey,
    ::rtl::OUString const & rPrefix )
    SAL_THROW( (::com::sun::star::loader::CannotActivateFactoryException) );

/** Invokes component_writeInfo() function of specified component library.  You can give either
    a fully qualified libname or single lib name. The libname need not be pre/postfixed
    (e.g. xxx.dll).  You can give parameter rPath to force lookup of the library in a specific
    directory.  The resulting path of the library will be checked against environment variable
    CPLD_ACCESSPATH if set.

    @deprecated component_writeInfo should no longer be used in new components

    @param rLibName name of the library
    @param rPath optional path
    @param xMgr service manager to be provided to the component
    @param xKey registry key to be provided to the component
*/
CPPUHELPER_DLLPUBLIC void
SAL_CALL writeSharedLibComponentInfo(
    ::rtl::OUString const & rLibName, ::rtl::OUString const & rPath,
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > const & xKey )
    SAL_THROW( (::com::sun::star::registry::CannotRegisterImplementationException) );

} // end namespace cppu

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
