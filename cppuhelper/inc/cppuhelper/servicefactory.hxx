/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#define _CPPUHELPER_SERVICEFACTORY_HXX_

#include <rtl/ustring.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include "cppuhelper/cppuhelperdllapi.h"

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
CPPUHELPER_DLLPUBLIC
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

