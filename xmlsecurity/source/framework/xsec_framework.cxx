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
