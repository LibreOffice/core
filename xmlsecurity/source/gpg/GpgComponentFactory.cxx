/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <sal/config.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "SEInitializer.hxx"
#include "XMLSignature.hxx"
#include "XMLEncryption.hxx"
#include "XMLSecurityContext.hxx"
#include "SecurityEnvironment.hxx"

using namespace cppu;
using namespace css::uno;
using namespace css::lang;
using namespace css::registry;

extern "C"
{

void* SAL_CALL gpg_component_getFactory( const sal_Char* pImplName , void* pServiceManager , void* /*pRegistryKey*/ )
{
    void* pRet = nullptr;
    Reference< XSingleServiceFactory > xFactory ;

    if( pImplName != nullptr && pServiceManager != nullptr )
    {
#ifdef XMLSEC_CRYPTO_GPG
        if( SEInitializer_Gpg_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory.set( createSingleFactory(
                static_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                SEInitializer_Gpg_createInstance, SEInitializer_Gpg_getSupportedServiceNames() ) );
        }
        else if( XMLSignature_Gpg::impl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = XMLSignature_Gpg::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( XMLSecurityContext_Gpg::impl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = XMLSecurityContext_Gpg::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( SecurityEnvironment_Gpg::impl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = SecurityEnvironment_Gpg::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( XMLEncryption_Gpg::impl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = XMLEncryption_Gpg::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
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
