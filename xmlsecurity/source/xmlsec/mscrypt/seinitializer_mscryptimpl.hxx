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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_SEINITIALIZER_MSCRYPTIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_MSCRYPT_SEINITIALIZER_MSCRYPTIMPL_HXX

#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

#include <libxml/tree.h>

class SEInitializer_MSCryptImpl : public cppu::WeakImplHelper
<
    com::sun::star::xml::crypto::XSEInitializer,
    com::sun::star::lang::XServiceInfo
>
/****** SEInitializer_MSCryptImpl.hxx/CLASS SEInitializer_MSCryptImpl ***********
 *
 *   NAME
 *  SEInitializer_MSCryptImpl -- Class to initialize a Security Context
 *  instance
 *
 *   FUNCTION
 *  Use this class to initialize a XmlSec based Security Context
 *  instance. After this instance is used up, use this class to free this
 *  instance.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
private:
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxContext;

public:
    explicit SEInitializer_MSCryptImpl(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > &rxContext);
    virtual ~SEInitializer_MSCryptImpl();

    /* XSEInitializer */
    virtual com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSecurityContext >
        SAL_CALL createSecurityContext( const OUString& certDB )
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL freeSecurityContext( const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSecurityContext >& securityContext )
        throw (com::sun::star::uno::RuntimeException);

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  )
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (com::sun::star::uno::RuntimeException);
};

OUString SEInitializer_MSCryptImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL SEInitializer_MSCryptImpl_supportsService( const OUString& ServiceName )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< OUString > SAL_CALL SEInitializer_MSCryptImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL SEInitializer_MSCryptImpl_createInstance( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
