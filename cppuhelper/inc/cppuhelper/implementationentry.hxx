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
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#define _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_

#include <cppuhelper/factory.hxx>

namespace cppu
{
/** One struct instance represents all data necessary for registering one service implementation.

 */
struct ImplementationEntry
{
    /** Function that creates an instance of the implementation
     */
       ComponentFactoryFunc create;

    /** Function that returns the implementation-name of the implementation
       (same as XServiceInfo.getImplementationName() ).
     */
     rtl::OUString ( SAL_CALL * getImplementationName )();

    /** Function that returns all supported servicenames of the implementation
       ( same as XServiceInfo.getSupportedServiceNames() ).
    */
     com::sun::star::uno::Sequence< rtl::OUString > ( SAL_CALL * getSupportedServiceNames ) ();

    /** Function that creates a SingleComponentFactory.
    */
     ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory >
     ( SAL_CALL * createFactory )(
         ComponentFactoryFunc fptr,
         ::rtl::OUString const & rImplementationName,
         ::com::sun::star::uno::Sequence< ::rtl::OUString > const & rServiceNames,
         rtl_ModuleCount * pModCount );

    /** The shared-library module-counter of the implementation. Maybe 0. The module-counter
        is used during by the createFactory()-function.
    */
     rtl_ModuleCount * moduleCounter;

    /** Must be set to 0 !
        For future extensions.
     */
    sal_Int32 nFlags;
};

/** Helper function for implementation of the component_writeInfo()-function.

    @obsolete component_writeInfo should no longer be used in new components

    @param pServiceManager The first parameter passed to component_writeInfo()-function
                           (This is an instance of the service manager, that creates the factory).
    @param pRegistryKey    The second parameter passed to the component_writeInfo()-function.
                           This is a reference to the registry key, into which the implementation
                           data shall be written to.
    @param entries         Each element of the entries-array must contains a function pointer
                           table for registering an implementation. The end of the array
                           must be marked with a 0 entry in the create-function.
    @return sal_True, if all implementations could be registered, otherwise sal_False.
 */
sal_Bool component_writeInfoHelper(
    void *pServiceManager, void *pRegistryKey , const struct ImplementationEntry entries[] );

/** Helper function for implementation of the component_getFactory()-function,
    that must be implemented by every shared library component.

    @param pImplName       The implementation-name to be instantiated ( This is the
                           first parameter passed to the component_getFactory
    @param pServiceManager The second parameter passed to component_getFactory()-function
                           (This is a of the service manager, that creates the factory).
    @param pRegistryKey    The third parameter passed to the component_getFactory()-function.
                           This is a reference to the registry key, where the implementation
                           data has been written to.
    @param entries         Each element of the entries-array must contains a function pointer
                           table for creating a factor of the implementation. The end of the array
                           must be marked with a 0 entry in the create-function.
    @return 0 if the helper failed to instantiate a factory, otherwise an acquired pointer
            to a factory.
 */
void *component_getFactoryHelper(
    const sal_Char * pImplName,
    void * pServiceManager,
    void * pRegistryKey,
    const struct ImplementationEntry entries[] );

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
