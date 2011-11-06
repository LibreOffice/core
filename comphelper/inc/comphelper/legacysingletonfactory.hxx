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



#ifndef COMPHELPER_LEGACYSINGLETONFACTORY_HXX
#define COMPHELPER_LEGACYSINGLETONFACTORY_HXX

#include "comphelper/comphelperdllapi.h"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <cppuhelper/factory.hxx>

//........................................................................
namespace comphelper
{
//........................................................................

/** creates a factory which can be used when implementing old-style singletons

    Before UNO having the component context concept, singletons were implemented by providing
    a factory which return the very same component instance for every creation request.

    Nowadays, singletons are implemented by making them available at the component context,
    as dedicated property.

    To bridge between both worlds - sometimes necessary to support legacy code, which instantiates
    singletons at the global service manager, instead of obtaining the property at the
    component context -, you can use the function below. I creates a single-component
    factory, which behaves like the old factories did: Upon multiple creation requests,
    it will always return the same instance. Additionally, the signature of the function
    is the same as the signature of <code>::cppu::createSingleComponentFactory</code>
    (which creates a "normal" component factory), so you can use both factory functions
    in the same context.
*/
::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleComponentFactory >
    COMPHELPER_DLLPUBLIC createLegacySingletonFactory(
        ::cppu::ComponentFactoryFunc _componentFactoryFunc,
        const ::rtl::OUString& _rImplementationName,
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames,
        rtl_ModuleCount* _pModCount = NULL
    );

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_LEGACYSINGLETONFACTORY_HXX
