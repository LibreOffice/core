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
#ifndef _CPPUHELPER_SHLIB_HXX_
#define _CPPUHELPER_SHLIB_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/registry/CannotRegisterImplementationException.hpp>


namespace cppu
{

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
    factory instance (::com::sun::star::lang::XSingleComponentFactory or
    ::com::sun::star::lang::XSingleComponentFactory)
*/
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL loadSharedLibComponentFactory(
    ::rtl::OUString const & rLibName, ::rtl::OUString const & rPath,
    ::rtl::OUString const & rImplName,
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > const & xKey )
    SAL_THROW( (::com::sun::star::loader::CannotActivateFactoryException) );

/** Invokes component_writeInfo() function of specified component library.  You can give either
    a fully qualified libname or single lib name. The libname need not be pre/postfixed
    (e.g. xxx.dll).  You can give parameter rPath to force lookup of the library in a specific
    directory.  The resulting path of the library will be checked against environment variable
    CPLD_ACCESSPATH if set.

    @obsolete component_writeInfo should no longer be used in new components

    @param rLibName name of the library
    @param rPath optional path
    @param xMgr service manager to be provided to the component
    @param xKey registry key to be provided to the component
*/
void
SAL_CALL writeSharedLibComponentInfo(
    ::rtl::OUString const & rLibName, ::rtl::OUString const & rPath,
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & xMgr,
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > const & xKey )
    SAL_THROW( (::com::sun::star::registry::CannotRegisterImplementationException) );

} // end namespace cppu

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
