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



#ifndef EXTENSIONS_RESOURCE_SERVICES_HXX
#define EXTENSIONS_RESOURCE_SERVICES_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
/** === end UNO includes === **/
#include <cppuhelper/factory.hxx>

//........................................................................
namespace res
{
//........................................................................

    struct ComponentInfo
    {
        /// services supported by the component
        ::com::sun::star::uno::Sequence< ::rtl::OUString >  aSupportedServices;
        /// implementation name of the component
        ::rtl::OUString                                     sImplementationName;
        /** name of the singleton instance of the component, if it is a singleton, empty otherwise
            If the component is a singleton, aSupportedServices must contain exactly one element.
        */
        ::rtl::OUString                                     sSingletonName;
        /// factory for creating the component
        ::cppu::ComponentFactoryFunc                        pFactory;
    };

    ComponentInfo   getComponentInfo_VclStringResourceLoader();
    ComponentInfo   getComponentInfo_OpenOfficeResourceLoader();

//........................................................................
}   // namespace res
//........................................................................

#endif // EXTENSIONS_RESOURCE_SERVICES_HXX
