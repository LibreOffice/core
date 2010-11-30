/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            //Decryptor
            sal_Int32 nPos = 0;
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( DecryptorImpl_getImplementationName() ) );
            xNewKey = xNewKey->createKey( OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES")) );

            const Sequence< OUString > & rSNL = DecryptorImpl_getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            //Encryptor
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( EncryptorImpl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES")) );
            const Sequence< OUString > & rSNL2 = EncryptorImpl_getSupportedServiceNames();
            pArray = rSNL2.getConstArray();
            for ( nPos = rSNL2.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            //SignatureCreator
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( SignatureCreatorImpl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES")) );
            const Sequence< OUString > & rSNL3 = SignatureCreatorImpl_getSupportedServiceNames();
            pArray = rSNL3.getConstArray();
            for ( nPos = rSNL3.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            //SignatureVerifier
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( SignatureVerifierImpl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES")) );
            const Sequence< OUString > & rSNL4 = SignatureVerifierImpl_getSupportedServiceNames();
            pArray = rSNL4.getConstArray();
            for ( nPos = rSNL4.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            //SAXEventKeeper
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( SAXEventKeeperImpl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES")) );
            const Sequence< OUString > & rSNL5 = SAXEventKeeperImpl_getSupportedServiceNames();
            pArray = rSNL5.getConstArray();
            for ( nPos = rSNL5.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            //XMLSignatureTemplateImpl
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( XMLSignatureTemplateImpl::impl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES")) );
            const Sequence< OUString > & rSNL6 = XMLSignatureTemplateImpl::impl_getSupportedServiceNames();
            pArray = rSNL6.getConstArray();
            for ( nPos = rSNL6.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            // XMLEncryptionTemplateImpl
            xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( XMLEncryptionTemplateImpl::impl_getImplementationName() );
            xNewKey = xNewKey->createKey( OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES")) );
            const Sequence< OUString > & rSNL7 = XMLEncryptionTemplateImpl::impl_getSupportedServiceNames();
            pArray = rSNL7.getConstArray();
            for ( nPos = rSNL7.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
