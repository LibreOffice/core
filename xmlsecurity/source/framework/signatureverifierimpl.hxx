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

#ifndef _SIGNATUREVERIFIERIMPL_HXX
#define _SIGNATUREVERIFIERIMPL_HXX

#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultBroadcaster.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase3.hxx>

#include "signatureengine.hxx"

class SignatureVerifierImpl : public cppu::ImplInheritanceHelper3
<
    SignatureEngine,
    com::sun::star::xml::crypto::sax::XSignatureVerifyResultBroadcaster,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
/****** SignatureVerifier.hxx/CLASS SignatureVerifierImpl *********************
 *
 *   NAME
 *  SignatureVerifierImpl -- verifies a signature
 *
 *   FUNCTION
 *  Collects all resources for a signature verification, then verifies the
 *  signature by invoking a xmlsec-based signature bridge component.
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
        com::sun::star::xml::crypto::XXMLSignatureTemplate >&
        xSignatureTemplate)
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

public:
    explicit SignatureVerifierImpl( const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >& rxMSF);
    virtual ~SignatureVerifierImpl();

    /* XSignatureVerifyResultBroadcaster */
    virtual void SAL_CALL addSignatureVerifyResultListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSignatureVerifyResultListener >&
            listener )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSignatureVerifyResultListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSignatureVerifyResultListener >&
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

rtl::OUString SignatureVerifierImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL SignatureVerifierImpl_supportsService( const rtl::OUString& ServiceName )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL SignatureVerifierImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL SignatureVerifierImpl_createInstance(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( com::sun::star::uno::Exception );

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
