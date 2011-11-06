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
#include "precompiled_xmlsecurity.hxx"

#include <sal/config.h>
#include <stdio.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "seinitializer_nssimpl.hxx"
#include "xmlsignature_nssimpl.hxx"
#include "xmlencryption_nssimpl.hxx"
#include "xmlsecuritycontext_nssimpl.hxx"
#include "securityenvironment_nssimpl.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{

void* SAL_CALL nss_component_getFactory( const sal_Char* pImplName , void* pServiceManager , void* /*pRegistryKey*/ )
{
    void* pRet = 0;
    Reference< XSingleServiceFactory > xFactory ;

    if( pImplName != NULL && pServiceManager != NULL )
    {
#ifdef XMLSEC_CRYPTO_NSS
        if( SEInitializer_NssImpl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = Reference< XSingleServiceFactory >( createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                SEInitializer_NssImpl_createInstance, SEInitializer_NssImpl_getSupportedServiceNames() ) );
        }
        else if( XMLSignature_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = XMLSignature_NssImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( XMLSecurityContext_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = XMLSecurityContext_NssImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( SecurityEnvironment_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = SecurityEnvironment_NssImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( XMLEncryption_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = XMLEncryption_NssImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
#else
        if( ONSSInitializer_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = Reference< XSingleServiceFactory >( createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                ONSSInitializer_createInstance, ONSSInitializer_getSupportedServiceNames() ) );
        }
#endif
    }

    if( xFactory.is() ) {
        xFactory->acquire() ;
        pRet = xFactory.get() ;
    }

    return pRet ;
}

}

