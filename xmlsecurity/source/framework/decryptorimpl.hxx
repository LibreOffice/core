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

#ifndef _DECRYPTORIMPL_HXX
#define _DECRYPTORIMPL_HXX

#include <com/sun/star/xml/crypto/sax/XDecryptionResultBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XDecryptionResultListener.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase3.hxx>

#include "encryptionengine.hxx"

class DecryptorImpl : public cppu::ImplInheritanceHelper3
<
    EncryptionEngine,
    com::sun::star::xml::crypto::sax::XDecryptionResultBroadcaster,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
/****** DecryptorImpl.hxx/CLASS DecryptorImpl *********************************
 *
 *   NAME
 *  DecryptorImpl -- decrypts an encryption
 *
 *   FUNCTION
 *  Collects all resources for decrypting an encryption, then decrypts the
 *  encryption by invoking a xmlsec-based encryption bridge component.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
private:
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSecurityContext > m_xXMLSecurityContext;

    virtual void notifyResultListener() const
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
    virtual bool checkReady() const;
    virtual void startEngine( const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLEncryptionTemplate >&
        xEncryptionTemplate)
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

public:
    explicit DecryptorImpl( const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >& rxMSF);
    virtual ~DecryptorImpl();

    /* XDecryptionResultBroadcaster */
    virtual void SAL_CALL addDecryptionResultListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XDecryptionResultListener >&
            listener )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeDecryptionResultListener(
            const com::sun::star::uno::Reference<
                com::sun::star::xml::crypto::sax::XDecryptionResultListener >&
                listener )
            throw (com::sun::star::uno::RuntimeException);

    /* XInitialization */
    virtual void SAL_CALL initialize(
        const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    /* XServiceInfo */
    virtual rtl::OUString SAL_CALL getImplementationName(  )
        throw (com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName )
        throw (com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (com::sun::star::uno::RuntimeException);
};

rtl::OUString DecryptorImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL DecryptorImpl_supportsService( const rtl::OUString& ServiceName )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL DecryptorImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL DecryptorImpl_createInstance(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >&
        rSMgr)
    throw ( com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
