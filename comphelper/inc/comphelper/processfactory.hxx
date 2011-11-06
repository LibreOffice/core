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



#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#define _COMPHELPER_PROCESSFACTORY_HXX_

#if ! defined(_COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HXX_)
#include "com/sun/star/uno/XComponentContext.hpp"
#endif
#include <com/sun/star/uno/Sequence.hxx>
#include "comphelper/comphelperdllapi.h"

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

namespace comphelper
{

/**
 * This function set the process service factory.
 *
 * @author Juergen Schmidt
 */
COMPHELPER_DLLPUBLIC void setProcessServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMgr);

/**
 * This function get the process service factory. If no service factory is set the function returns
 * a null interface.
 *
 * @author Juergen Schmidt
 */
COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getProcessServiceFactory();

/** creates a component, using the process factory if set
    @see getProcessServiceFactory
    @see setProcessServiceFactory
*/
COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    createProcessComponent(
        const ::rtl::OUString& _rServiceSpecifier
    ) SAL_THROW( ( ::com::sun::star::uno::RuntimeException ) );

/** creates a component with arguments, using the process factory if set

    @see getProcessServiceFactory
    @see setProcessServiceFactory
*/
COMPHELPER_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    createProcessComponentWithArguments(
        const ::rtl::OUString& _rServiceSpecifier,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArgs
    ) SAL_THROW( ( ::com::sun::star::uno::RuntimeException ) );

/**
 * This function gets the process service factory's default component context.
 * If no service factory is set the function returns a null interface.
 */
COMPHELPER_DLLPUBLIC
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
getProcessComponentContext();

}


extern "C" {
/// @internal ATTENTION returns ACQUIRED pointer! release it explicitly!
COMPHELPER_DLLPUBLIC
::com::sun::star::uno::XComponentContext *
comphelper_getProcessComponentContext();
} // extern "C"

#endif // _COMPHELPER_PROCESSFACTORY_HXX_

