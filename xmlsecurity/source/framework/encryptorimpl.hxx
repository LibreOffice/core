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
    css::xml::crypto::sax::XEncryptionResultBroadcaster,
    css::xml::crypto::sax::XReferenceCollector,
    css::lang::XInitialization,
    css::lang::XServiceInfo
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
 ******************************************************************************/
{
private:
    /*
     * the Id of the element to be encrypted.
     */
    sal_Int32 m_nReferenceId;

    css::uno::Reference<
        css::xml::crypto::XSecurityEnvironment > m_xSecurityEnvironment;

    virtual void notifyResultListener() const
        throw (css::uno::Exception, css::uno::RuntimeException) override;
    virtual bool checkReady() const override;
    virtual void startEngine( const css::uno::Reference<
        css::xml::crypto::XXMLEncryptionTemplate >&
        xEncryptionTemplate)
        throw (css::uno::Exception, css::uno::RuntimeException) override;

public:
    explicit EncryptorImpl(const css::uno::Reference< css::uno::XComponentContext > & xContext);
    virtual ~EncryptorImpl();

    /* XEncryptionResultBroadcaster */
    virtual void SAL_CALL addEncryptionResultListener(
        const css::uno::Reference<
            css::xml::crypto::sax::XEncryptionResultListener >&
            listener )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEncryptionResultListener(
            const css::uno::Reference<
                css::xml::crypto::sax::XEncryptionResultListener >&
                listener )
            throw (css::uno::RuntimeException, std::exception) override;

    /* XReferenceCollector */
    virtual void SAL_CALL setReferenceCount( sal_Int32 count )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setReferenceId( sal_Int32 id )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

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

OUString EncryptorImpl_getImplementationName()
    throw ( css::uno::RuntimeException );

css::uno::Sequence< OUString > SAL_CALL EncryptorImpl_getSupportedServiceNames(  )
    throw ( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface >
SAL_CALL EncryptorImpl_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr)
    throw ( css::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
