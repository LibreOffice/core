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

#ifndef INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_SIGNATUREVERIFIERIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_SIGNATUREVERIFIERIMPL_HXX

#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultBroadcaster.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

#include "signatureengine.hxx"

typedef cppu::ImplInheritanceHelper
<
    SignatureEngine,
    css::xml::crypto::sax::XSignatureVerifyResultBroadcaster,
    css::lang::XInitialization,
    css::lang::XServiceInfo
> SignatureVerifierImpl_Base;

class SignatureVerifierImpl : public SignatureVerifierImpl_Base
/****** SignatureVerifier.hxx/CLASS SignatureVerifierImpl *********************
 *
 *   NAME
 *  SignatureVerifierImpl -- verifies a signature
 *
 *   FUNCTION
 *  Collects all resources for a signature verification, then verifies the
 *  signature by invoking a xmlsec-based signature bridge component.
 ******************************************************************************/
{
private:
    css::uno::Reference< css::xml::crypto::XXMLSecurityContext > m_xXMLSecurityContext;

    virtual void notifyResultListener() const
        throw (css::uno::Exception, css::uno::RuntimeException) override;
    virtual bool checkReady() const override;
    virtual void startEngine( const css::uno::Reference< css::xml::crypto::XXMLSignatureTemplate >& xSignatureTemplate)
        throw (css::uno::Exception, css::uno::RuntimeException) override;

public:
    explicit SignatureVerifierImpl( const css::uno::Reference<
        css::uno::XComponentContext >& rxContext);
    virtual ~SignatureVerifierImpl();

    /* XSignatureVerifyResultBroadcaster */
    virtual void SAL_CALL addSignatureVerifyResultListener(
        const css::uno::Reference< css::xml::crypto::sax::XSignatureVerifyResultListener >& listener )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSignatureVerifyResultListener(
        const css::uno::Reference< css::xml::crypto::sax::XSignatureVerifyResultListener >& listener )
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

OUString SignatureVerifierImpl_getImplementationName()
    throw ( css::uno::RuntimeException );

css::uno::Sequence< OUString > SAL_CALL SignatureVerifierImpl_getSupportedServiceNames(  )
    throw ( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface >
SAL_CALL SignatureVerifierImpl_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)
    throw ( css::uno::Exception );

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
