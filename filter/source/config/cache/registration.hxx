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



#ifndef __COMPHELPER_REGISTRATION_HXX_
#define __COMPHELPER_REGISTRATION_HXX_

//_______________________________________________
// includes

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/factory.hxx>

//_______________________________________________
// namespace

namespace comphelper{

//_______________________________________________
// declaration

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_COMPONENT_GETIMPLEMENTATIONENVIRONMENT                                                          \
    extern "C" void SAL_CALL component_getImplementationEnvironment(const sal_Char**        ppEnvironmentTypeName,  \
                                                                          uno_Environment** /* ppEnvironment */ )   \
    {                                                                                                               \
        *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;                                                \
    }

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_MULTIINSTANCEFACTORY(IMPLEMENTATIONNAME, SERVICENAMES, FACTORYMETHOD)   \
    if (IMPLEMENTATIONNAME == sImplName)                                                    \
        xFactory = ::cppu::createSingleFactory(xSMGR             ,                          \
                                               IMPLEMENTATIONNAME,                          \
                                               FACTORYMETHOD     ,                          \
                                               SERVICENAMES      );

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_ONEINSTANCEFACTORY(IMPLEMENTATIONNAME, SERVICENAMES, FACTORYMETHOD)     \
    if (IMPLEMENTATIONNAME == sImplName)                                                    \
        xFactory = ::cppu::createOneInstanceFactory(xSMGR             ,                     \
                                                    IMPLEMENTATIONNAME,                     \
                                                    FACTORYMETHOD     ,                     \
                                                    SERVICENAMES      );

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_COMPONENT_GETFACTORY(STATIC_INIT,FACTORYLIST)                                                                                               \
    extern "C" void* SAL_CALL component_getFactory(const sal_Char* pImplementationName,                                                             \
                                                         void*     pServiceManager    ,                                                             \
                                                         void*     /* pRegistryKey */ )                                                             \
    {                                                                                                                                               \
        if (                                                                                                                                        \
            (!pImplementationName) ||                                                                                                               \
            (!pServiceManager    )                                                                                                                  \
           )                                                                                                                                        \
        return NULL;                                                                                                                                \
                                                                                                                                                    \
        STATIC_INIT                                                                                                                                 \
                                                                                                                                                    \
        css::uno::Reference< css::lang::XMultiServiceFactory >  xSMGR     = reinterpret_cast< css::lang::XMultiServiceFactory* >(pServiceManager);  \
        css::uno::Reference< css::lang::XSingleServiceFactory > xFactory  ;                                                                         \
        rtl::OUString                                           sImplName = ::rtl::OUString::createFromAscii(pImplementationName);                  \
                                                                                                                                                    \
        /* This parameter will expand to: */                                                                                                        \
        /*  _COMPHELPER_xxxFACTORY(1)     */                                                                                                        \
        /*  else                          */                                                                                                        \
        /*  ...                           */                                                                                                        \
        /*  else                          */                                                                                                        \
        /*  _COMPHELPER_xxxFACTORY(n)     */                                                                                                        \
        FACTORYLIST                                                                                                                                 \
                                                                                                                                                    \
        /* And if one of these checks was successfully => xFactory was set! */                                                                      \
        if (xFactory.is())                                                                                                                          \
        {                                                                                                                                           \
            xFactory->acquire();                                                                                                                    \
            return xFactory.get();                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
        return NULL;                                                                                                                                \
    }

} // namespace comphelper

#endif  //  #ifndef __COMPHELPER_REGISTRATION_HXX_
