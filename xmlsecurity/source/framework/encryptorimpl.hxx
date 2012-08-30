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

#ifndef _ENCRYPTORIMPL_HXX
#define _ENCRYPTORIMPL_HXX

#include <com/sun/star/xml/crypto/sax/XEncryptionResultBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XEncryptionResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase4.hxx>

#include "encryptionengine.hxx"

class EncryptorImpl : public cppu::ImplInheritanceHelper4
<
    EncryptionEngine,
    com::sun::star::xml::crypto::sax::XEncryptionResultBroadcaster,
    com::sun::star::xml::crypto::sax::XReferenceCollector,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
/****** EncryptorImpl.hxx/CLASS EncryptorImpl *********************************
 *
 *   NAME
 *  EncryptorImpl -- generates an encryption
 *
 *   FUNCTION
 *  Collects all resources for an encryption generation, then generates the
 *  encryption by invoking a xmlsec-based encryption bridge component.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
private:
    /*
     * the Id of the element to be encrypted.
     */
    sal_Int32 m_nReferenceId;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XSecurityEnvironment > m_xSecurityEnvironment;

    virtual void notifyResultListener() const
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
    virtual bool checkReady() const;
    virtual void startEngine( const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLEncryptionTemplate >&
        xEncryptionTemplate)
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

public:
    explicit EncryptorImpl( const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >& rxMSF);
    virtual ~EncryptorImpl();

    /* XEncryptionResultBroadcaster */
    virtual void SAL_CALL addEncryptionResultListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XEncryptionResultListener >&
            listener )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEncryptionResultListener(
            const com::sun::star::uno::Reference<
                com::sun::star::xml::crypto::sax::XEncryptionResultListener >&
                listener )
            throw (com::sun::star::uno::RuntimeException);

    /* XReferenceCollector */
    virtual void SAL_CALL setReferenceCount( sal_Int32 count )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setReferenceId( sal_Int32 id )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

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

rtl::OUString EncryptorImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL EncryptorImpl_supportsService( const rtl::OUString& ServiceName )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL EncryptorImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL EncryptorImpl_createInstance(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMgr)
    throw ( com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
