/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <sal/config.h>
#include <stdio.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/security/XSerialNumberAdapter.hpp>

#include "xmlelementwrapper_xmlsecimpl.hxx"
#include "xmldocumentwrapper_xmlsecimpl.hxx"
#include "xmlsecurity/biginteger.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

namespace
{
class SerialNumberAdapterImpl : public WeakImplHelper1<
        ::com::sun::star::security::XSerialNumberAdapter >
{
    virtual OUString SAL_CALL toString( const Sequence< sal_Int8 >& rSerialNumber )
        throw (RuntimeException)
    {
        return bigIntegerToNumericString(rSerialNumber);
    }
    virtual Sequence< sal_Int8 > SAL_CALL toSequence( const OUString& rSerialNumber )
        throw (RuntimeException)
    {
        return numericStringToBigInteger(rSerialNumber);
    }
};

OUString SerialNumberAdapterImpl_getImplementationName()
    throw (RuntimeException)
{
    return OUString( "com.sun.star.security.SerialNumberAdapter");
}

Sequence< OUString > SerialNumberAdapterImpl_getSupportedServiceNames()
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( "com.sun.star.security.SerialNumberAdapter"  );
    return aRet;
}

Reference< XInterface > SerialNumberAdapterImpl_createInstance(
    const Reference< XComponentContext > &) throw( Exception )
{
    return Reference< XInterface >( *new SerialNumberAdapterImpl() );
}

}

extern "C"
{

extern void* nss_component_getFactory( const sal_Char*, void*, void* );

#if defined( XMLSEC_CRYPTO_MSCRYPTO )
extern void* mscrypt_component_getFactory( const sal_Char*, void*, void* );
#endif

SAL_DLLPUBLIC_EXPORT void* SAL_CALL xsec_xmlsec_component_getFactory( const sal_Char* pImplName , void* pServiceManager , void* pRegistryKey )
{
    void* pRet = 0;
    Reference< XInterface > xFactory ;

    if( pImplName != NULL && pServiceManager != NULL ) {
        if( XMLElementWrapper_XmlSecImpl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = Reference< XSingleServiceFactory >( createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                XMLElementWrapper_XmlSecImpl_createInstance, XMLElementWrapper_XmlSecImpl_getSupportedServiceNames() ) );
        }
        else if( XMLDocumentWrapper_XmlSecImpl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = Reference< XSingleServiceFactory >( createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                XMLDocumentWrapper_XmlSecImpl_createInstance, XMLDocumentWrapper_XmlSecImpl_getSupportedServiceNames() ) );
        }
        else if( SerialNumberAdapterImpl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = ::cppu::createSingleComponentFactory(
              SerialNumberAdapterImpl_createInstance,
              OUString::createFromAscii( pImplName ),
              SerialNumberAdapterImpl_getSupportedServiceNames() );
        }
    }

    if( xFactory.is() ) {
        xFactory->acquire() ;
        pRet = xFactory.get() ;
    } else {
        pRet = nss_component_getFactory( pImplName, pServiceManager, pRegistryKey ) ;
        if( pRet != NULL )
            return pRet ;

#if defined( XMLSEC_CRYPTO_MSCRYPTO )
        pRet = mscrypt_component_getFactory( pImplName, pServiceManager, pRegistryKey ) ;
        if( pRet != NULL )
            return pRet ;
#endif
    }

    return pRet ;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
