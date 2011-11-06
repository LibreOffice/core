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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

//-----------------------------------------------------------------------
//  includes of other projects
//-----------------------------------------------------------------------
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include "shellexec.hxx"

//-----------------------------------------------------------------------
// namespace directives
//-----------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::cppu;
using com::sun::star::system::XSystemShellExecute;

//-----------------------------------------------------------------------
// defines
//-----------------------------------------------------------------------

#define SHELLEXEC_SERVICE_NAME  "com.sun.star.system.SystemShellExecute"
#define SHELLEXEC_IMPL_NAME     "com.sun.star.comp.system.SystemShellExecute"
#define SHELLEXEC_REGKEY_NAME   "/com.sun.star.comp.system.SystemShellExecute/UNO/SERVICES/com.sun.star.system.SystemShellExecute"

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------

namespace
{
    Reference< XInterface > SAL_CALL createInstance(const Reference< XComponentContext >& xContext)
    {
        return Reference< XInterface >( static_cast< XSystemShellExecute* >( new ShellExec(xContext) ) );
    }
}

//-----------------------------------------------------------------------
// the 3 important functions which will be exported
//-----------------------------------------------------------------------

extern "C"
{

//----------------------------------------------------------------------
// component_getImplementationEnvironment
//----------------------------------------------------------------------

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//----------------------------------------------------------------------
// component_getFactory
//----------------------------------------------------------------------

void* SAL_CALL component_getFactory( const sal_Char* pImplName, uno_Interface* /*pSrvManager*/, uno_Interface* /*pRegistryKey*/ )
{
    Reference< XSingleComponentFactory > xFactory;

    if (0 == ::rtl_str_compare( pImplName, SHELLEXEC_IMPL_NAME ))
    {
        OUString serviceName( RTL_CONSTASCII_USTRINGPARAM(SHELLEXEC_SERVICE_NAME) );

        xFactory = ::cppu::createSingleComponentFactory(
            createInstance,
            OUString( RTL_CONSTASCII_USTRINGPARAM(SHELLEXEC_IMPL_NAME) ),
            Sequence< OUString >( &serviceName, 1 ) );

    }

    if (xFactory.is())
    xFactory->acquire();

    return xFactory.get();
}

} // extern "C"
