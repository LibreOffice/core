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

#include "xsec_xmlsec.hxx"

#include <config_gpgme.h>
#if HAVE_FEATURE_GPGME
# include <gpg/xmlsignature_gpgimpl.hxx>
# include <gpg/SEInitializer.hxx>
#endif

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

extern "C"
{

SAL_DLLPUBLIC_EXPORT void* xsec_xmlsec_component_getFactory( const sal_Char* pImplName , void* pServiceManager , void* pRegistryKey )
{
    void* pRet = nullptr;
    Reference< XInterface > xFactory ;

    if( pImplName != nullptr ) {
#if HAVE_FEATURE_GPGME
        if( XMLSignature_GpgImpl::impl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = XMLSignature_GpgImpl::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( SEInitializerGpg::impl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = SEInitializerGpg::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
#endif
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
        if( pRet != nullptr )
            return pRet ;
#endif
    }

    return pRet ;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
