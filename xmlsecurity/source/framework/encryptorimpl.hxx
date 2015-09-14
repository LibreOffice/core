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

#ifndef INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_ENCRYPTORIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_ENCRYPTORIMPL_HXX

#include <com/sun/star/xml/crypto/sax/XEncryptionResultBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XEncryptionResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceCollector.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>

#include "encryptionengine.hxx"

typedef cppu::ImplInheritanceHelper
<
    EncryptionEngine,
    com::sun::star::xml::crypto::sax::XEncryptionResultBroadcaster,
    com::sun::star::xml::crypto::sax::XReferenceCollector,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
> EncryptorImpl_Base;

class EncryptorImpl : public EncryptorImpl_Base
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
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual bool checkReady() const SAL_OVERRIDE;
    virtual void startEngine( const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLEncryptionTemplate >&
        xEncryptionTemplate)
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

public:
    explicit EncryptorImpl(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > & xContext);
    virtual ~EncryptorImpl();

    /* XEncryptionResultBroadcaster */
    virtual void SAL_CALL addEncryptionResultListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XEncryptionResultListener >&
            listener )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeEncryptionResultListener(
            const com::sun::star::uno::Reference<
                com::sun::star::xml::crypto::sax::XEncryptionResultListener >&
                listener )
            throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /* XReferenceCollector */
    virtual void SAL_CALL setReferenceCount( sal_Int32 count )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setReferenceId( sal_Int32 id )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /* XInitialization */
    virtual void SAL_CALL initialize(
        const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  )
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

OUString EncryptorImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< OUString > SAL_CALL EncryptorImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL EncryptorImpl_createInstance(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMgr)
    throw ( com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
