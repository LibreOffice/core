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
