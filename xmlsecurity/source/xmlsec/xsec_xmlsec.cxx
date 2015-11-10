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

#include <cppuhelper/factory.hxx>

#include "serialnumberadapter.hxx"
#include "xmlelementwrapper_xmlsecimpl.hxx"
#include "xmldocumentwrapper_xmlsecimpl.hxx"
#include "xsec_xmlsec.hxx"

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

extern "C"
{

SAL_DLLPUBLIC_EXPORT void* SAL_CALL xsec_xmlsec_component_getFactory( const sal_Char* pImplName , void* pServiceManager , void* pRegistryKey )
{
    void* pRet = nullptr;
    Reference< XInterface > xFactory ;

    if( pImplName != nullptr ) {
        if( XMLElementWrapper_XmlSecImpl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = cppu::createSingleComponentFactory(
                XMLElementWrapper_XmlSecImpl_createInstance,
                OUString::createFromAscii( pImplName ),
                XMLElementWrapper_XmlSecImpl_getSupportedServiceNames() );
        }
        else if( XMLDocumentWrapper_XmlSecImpl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = cppu::createSingleComponentFactory(
                XMLDocumentWrapper_XmlSecImpl_createInstance,
                OUString::createFromAscii( pImplName ),
                XMLDocumentWrapper_XmlSecImpl_getSupportedServiceNames() );
        }
        else if( xml_security::serial_number_adapter::implementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = ::cppu::createSingleComponentFactory(
              xml_security::serial_number_adapter::create,
              OUString::createFromAscii( pImplName ),
              xml_security::serial_number_adapter::serviceNames() );
        }
    }

    if( xFactory.is() ) {
        xFactory->acquire() ;
        pRet = xFactory.get() ;
    } else {
        pRet = nss_component_getFactory( pImplName, pServiceManager, pRegistryKey ) ;
        if( pRet != nullptr )
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
