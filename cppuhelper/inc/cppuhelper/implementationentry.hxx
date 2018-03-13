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


#ifndef _CPPUHELPER_IMPLEMENATIONENTRY_HXX_
#define _CPPUHELPER_IMPLEMENATIONENTRY_HXX_

#include <cppuhelper/factory.hxx>

namespace cppu
{
/** One struct instance represents all data necessary for registering one service implementation.

 */
struct ImplementationEntry
{
    /** Function, that creates an instance of the implemenation
     */
       ComponentFactoryFunc create;

    /** Function, that returns the implemenation-name of the implemenation
       (same as XServiceInfo.getImplementationName() ).
     */
     rtl::OUString ( SAL_CALL * getImplementationName )();

    /** Function, that returns all supported servicenames of the implemenation
       ( same as XServiceInfo.getSupportedServiceNames() ).
    */
     com::sun::star::uno::Sequence< rtl::OUString > ( SAL_CALL * getSupportedServiceNames ) ();

    /** Function, that creates a SingleComponentFactory.
    */
     ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory >
     ( SAL_CALL * createFactory )(
         ComponentFactoryFunc fptr,
         ::rtl::OUString const & rImplementationName,
         ::com::sun::star::uno::Sequence< ::rtl::OUString > const & rServiceNames,
         rtl_ModuleCount * pModCount );

    /** The shared-library module-counter of the implemenation. Maybe 0. The module-counter
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
                           table for registering an implemenation. The end of the array
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
