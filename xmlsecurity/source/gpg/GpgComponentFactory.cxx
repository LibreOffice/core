/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SecurityEnvironment.hxx"
#include "SEInitializer.hxx"
#include "XMLSignature.hxx"
#include "XMLEncryption.hxx"
#include "XMLSecurityContext.hxx"

#include <cppuhelper/factory.hxx>

using namespace cppu;
using namespace css::uno;
using namespace css::lang;
using namespace css::registry;

extern "C"
{

SAL_DLLPUBLIC_EXPORT void* SAL_CALL xsec_gpg_component_getFactory( const sal_Char* pImplName , void* pServiceManager , void* /*pRegistryKey*/ )
{
    void* pRet = nullptr;
    Reference< XSingleServiceFactory > xFactory ;

    if( pImplName != nullptr && pServiceManager != nullptr )
    {
#ifdef XMLSEC_CRYPTO_GPG
        if( SEInitializer_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory.set( createSingleFactory(
                static_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                SEInitializer_createInstance, SEInitializer_getSupportedServiceNames() ) );
        }
        else if( XMLSignatureGpg::impl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = XMLSignatureGpg::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( XMLSecurityContextGpg::impl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = XMLSecurityContextGpg::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( SecurityEnvironmentGpg::impl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = SecurityEnvironmentGpg::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( XMLEncryptionGpg::impl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = XMLEncryptionGpg::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
