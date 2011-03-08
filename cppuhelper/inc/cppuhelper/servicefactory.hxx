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
#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#define _CPPUHELPER_SERVICEFACTORY_HXX_

#include <rtl/ustring.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>


namespace cppu
{

/** Deprecated.  Use cppuhelper/bootstrap.hxx functions instead.

    This bootstraps an initial service factory working on a registry. If the first or both
    parameters contain a value then the service factory is initialized with a simple registry
    or a nested registry. Otherwise the service factory must be initialized later with a valid
    registry.

    @param rWriteRegistryFile
    file name of the simple registry or the first registry file of
    the nested registry which will be opened with read/write rights. This
    file will be created if necessary.
    @param rReadRegistryFile
    file name of the second registry file of the nested registry
    which will be opened with readonly rights.
    @param bReadOnly
    flag which specify that the first registry file will be opened with
    readonly rights. Default is FALSE. If this flag is used the registry
    will not be created if not exist.
    @param rBootstrapPath
    specifies a path where the bootstrap components could be find. If this
    parameter is an empty string the compoents will be searched in the system
    path.
    @deprecated
*/
::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > SAL_CALL
createRegistryServiceFactory(
    const ::rtl::OUString & rWriteRegistryFile,
    const ::rtl::OUString & rReadRegistryFile,
    sal_Bool bReadOnly = sal_False,
    const ::rtl::OUString & rBootstrapPath = ::rtl::OUString() )
    SAL_THROW( (::com::sun::star::uno::Exception) );


/** Deprecated.  Use cppuhelper/bootstrap.hxx functions instead.

    This bootstraps an initial service factory working on a registry file.

    @param rRegistryFile
    file name of the registry to use/ create; if this is an empty
    string, the default registry is used instead
    @param bReadOnly
    flag which specify that the registry file will be opened with
    readonly rights. Default is FALSE. If this flag is used the registry
    will not be created if not exist.
    @param rBootstrapPath
    specifies a path where the bootstrap components could be find. If this
    parameter is an empty string the compoents will be searched in the system
    path.
    @deprecated
*/
inline ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > SAL_CALL
createRegistryServiceFactory(
    const ::rtl::OUString & rRegistryFile,
    sal_Bool bReadOnly = sal_False,
    const ::rtl::OUString & rBootstrapPath = ::rtl::OUString() )
    SAL_THROW( (::com::sun::star::uno::Exception) )
{
    return ::cppu::createRegistryServiceFactory(
        rRegistryFile, ::rtl::OUString(), bReadOnly, rBootstrapPath );
}

/** Deprecated.  Use cppuhelper/bootstrap.hxx functions instead.

    This bootstraps a service factory without initialize a registry.

    @param rBootstrapPath
    specifies a path where the bootstrap components could be find. If this
    parameter is an empty string the compoents will be searched in the system
    path.
    @deprecated
*/
inline ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > SAL_CALL
createServiceFactory(
    const ::rtl::OUString & rBootstrapPath = ::rtl::OUString() )
    SAL_THROW( (::com::sun::star::uno::Exception) )
{
    return ::cppu::createRegistryServiceFactory(
        ::rtl::OUString(), ::rtl::OUString(), sal_False, rBootstrapPath );
}

} // end namespace cppu

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
