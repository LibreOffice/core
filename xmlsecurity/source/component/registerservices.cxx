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

#include <tools/debug.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <cppuhelper/factory.hxx>


#include <documentdigitalsignatures.hxx>
#include <certificatecontainer.hxx>

using namespace ::com::sun::star;

extern "C"
{
void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void* SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void* pRet = 0;
    uno::Reference< XInterface > xFactory;

    //Decryptor
    rtl::OUString implName = rtl::OUString::createFromAscii( pImplName );

    if ( pServiceManager && implName.equals( DocumentDigitalSignatures::GetImplementationName() ) )
    {
        // DocumentDigitalSignatures
        xFactory = cppu::createSingleComponentFactory(
            DocumentDigitalSignatures_CreateInstance,
            rtl::OUString::createFromAscii( pImplName ),
            DocumentDigitalSignatures::GetSupportedServiceNames() );
    }
    else if ( pServiceManager && implName.equals( CertificateContainer::impl_getStaticImplementationName() ))
    {
        // CertificateContainer
        xFactory = cppu::createOneInstanceFactory(
            reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
            rtl::OUString::createFromAscii( pImplName ),
            CertificateContainer::impl_createInstance,
            CertificateContainer::impl_getStaticSupportedServiceNames() );
    }

     if (xFactory.is())
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

}   // extern "C"





