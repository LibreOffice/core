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
#ifndef INCLUDED_CPPUHELPER_IMPLEMENTATIONENTRY_HXX
#define INCLUDED_CPPUHELPER_IMPLEMENTATIONENTRY_HXX

#include "cppuhelper/factory.hxx"
#include "cppuhelper/cppuhelperdllapi.h"

namespace cppu
{
/** One struct instance represents all data necessary for registering one service implementation.

 */
struct SAL_WARN_UNUSED ImplementationEntry
{
    /** Function that creates an instance of the implementation
     */
       ComponentFactoryFunc create;

    /** Function that returns the implementation-name of the implementation
       (same as XServiceInfo.getImplementationName() ).
     */
     rtl::OUString (SAL_CALL * getImplementationName)();

    /** Function that returns all supported servicenames of the implementation
       ( same as XServiceInfo.getSupportedServiceNames() ).
    */
     css::uno::Sequence< rtl::OUString > (SAL_CALL * getSupportedServiceNames) ();

    /** Function that creates a SingleComponentFactory.

        The pModCount parameter is a backwards-compatibility remainder of a
        removed library unloading feature; always set to null.
    */
     css::uno::Reference< css::lang::XSingleComponentFactory >
     (SAL_CALL * createFactory)(
         ComponentFactoryFunc fptr,
         ::rtl::OUString const & rImplementationName,
         css::uno::Sequence< ::rtl::OUString > const & rServiceNames,
         rtl_ModuleCount * pModCount );

    /** Backwards-compatibility remainder of a removed library unloading
        feature; always set to null.
    */
     rtl_ModuleCount * moduleCounter;

    /** Must be set to 0 !
        For future extensions.
     */
    sal_Int32 nFlags;
};

/** Helper function for implementation of the component_writeInfo()-function.

    @deprecated component_writeInfo should no longer be used in new components

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
CPPUHELPER_DLLPUBLIC sal_Bool component_writeInfoHelper(
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
CPPUHELPER_DLLPUBLIC void *component_getFactoryHelper(
    const sal_Char * pImplName,
    void * pServiceManager,
    void * pRegistryKey,
    const struct ImplementationEntry entries[] );

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
