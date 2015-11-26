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

#ifndef INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_DECRYPTORIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_DECRYPTORIMPL_HXX

#include <com/sun/star/xml/crypto/sax/XDecryptionResultBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XDecryptionResultListener.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>

#include "encryptionengine.hxx"

typedef cppu::ImplInheritanceHelper
<
    EncryptionEngine,
    css::xml::crypto::sax::XDecryptionResultBroadcaster,
    css::lang::XInitialization,
    css::lang::XServiceInfo
> DecryptorImpl_Base;
class DecryptorImpl : public DecryptorImpl_Base
/****** DecryptorImpl.hxx/CLASS DecryptorImpl *********************************
 *
 *   NAME
 *  DecryptorImpl -- decrypts an encryption
 *
 *   FUNCTION
 *  Collects all resources for decrypting an encryption, then decrypts the
 *  encryption by invoking a xmlsec-based encryption bridge component.
 ******************************************************************************/
{
private:
    css::uno::Reference< css::xml::crypto::XXMLSecurityContext > m_xXMLSecurityContext;

    virtual void notifyResultListener() const
        throw (css::uno::Exception, css::uno::RuntimeException) override;
    virtual bool checkReady() const override;
    virtual void startEngine( const css::uno::Reference<
        css::xml::crypto::XXMLEncryptionTemplate >&
        xEncryptionTemplate)
        throw (css::uno::Exception, css::uno::RuntimeException) override;

public:
    explicit DecryptorImpl(const css::uno::Reference< css::uno::XComponentContext > & xContext);
    virtual ~DecryptorImpl();

    /* XDecryptionResultBroadcaster */
    virtual void SAL_CALL addDecryptionResultListener(
        const css::uno::Reference< css::xml::crypto::sax::XDecryptionResultListener >& listener )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeDecryptionResultListener(
            const css::uno::Reference< css::xml::crypto::sax::XDecryptionResultListener >& listener )
            throw (css::uno::RuntimeException, std::exception) override;

    /* XInitialization */
    virtual void SAL_CALL initialize(
        const css::uno::Sequence< css::uno::Any >& aArguments )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (css::uno::RuntimeException, std::exception) override;
};

OUString DecryptorImpl_getImplementationName()
    throw ( css::uno::RuntimeException );

css::uno::Sequence< OUString > SAL_CALL DecryptorImpl_getSupportedServiceNames(  )
    throw ( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface >
SAL_CALL DecryptorImpl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr)
    throw ( css::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
