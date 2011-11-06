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

#include <stdio.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <decryptorimpl.hxx>
#include <encryptorimpl.hxx>
#include <signaturecreatorimpl.hxx>
#include <signatureverifierimpl.hxx>
#include <saxeventkeeperimpl.hxx>
#include <xmlencryptiontemplateimpl.hxx>
#include <xmlsignaturetemplateimpl.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    //Decryptor
    OUString implName = OUString::createFromAscii( pImplName );
    if ( pServiceManager && implName.equals(DecryptorImpl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            DecryptorImpl_createInstance, DecryptorImpl_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //Encryptor
    if ( pServiceManager && implName.equals(EncryptorImpl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            EncryptorImpl_createInstance, EncryptorImpl_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //SignatureCreator
    if ( pServiceManager && implName.equals(SignatureCreatorImpl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            SignatureCreatorImpl_createInstance, SignatureCreatorImpl_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //SignatureVerifier
    if ( pServiceManager && implName.equals(SignatureVerifierImpl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            SignatureVerifierImpl_createInstance, SignatureVerifierImpl_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //SAXEventKeeper
    if ( pServiceManager && implName.equals(SAXEventKeeperImpl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            SAXEventKeeperImpl_createInstance, SAXEventKeeperImpl_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //XMLSignatureTemplate
    if ( pServiceManager && implName.equals( XMLSignatureTemplateImpl::impl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory = XMLSignatureTemplateImpl::impl_createFactory(
                reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    //XMLEncryptionTemplate
    if ( pServiceManager && implName.equals( XMLEncryptionTemplateImpl::impl_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory = XMLEncryptionTemplateImpl::impl_createFactory(
                reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


