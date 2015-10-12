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

#ifndef INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_SIGNATURECREATORIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_SIGNATURECREATORIMPL_HXX

#include <com/sun/star/xml/crypto/sax/XBlockerMonitor.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultBroadcaster.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>

#include "signatureengine.hxx"

typedef cppu::ImplInheritanceHelper
<
    SignatureEngine,
    com::sun::star::xml::crypto::sax::XBlockerMonitor,
    com::sun::star::xml::crypto::sax::XSignatureCreationResultBroadcaster,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
> SignatureCreatorImpl_Base;

class SignatureCreatorImpl : public SignatureCreatorImpl_Base
/****** SignatureCreatorImpl.hxx/CLASS SignatureCreatorImpl *******************
 *
 *   NAME
 *  SignatureCreatorImpl -- generates a signature
 *
 *   FUNCTION
 *  Collects all resources for a signature generation, then generates the
 *  signature by invoking a xmlsec-based signature bridge component.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
private:
    /*
     * the Id of template blocker.
     */
    sal_Int32 m_nIdOfBlocker;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XSecurityEnvironment > m_xSecurityEnvironment;

    virtual void notifyResultListener() const
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException) override;
    virtual void clearUp( ) const override;
    virtual bool checkReady() const override;
    virtual void startEngine( const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSignatureTemplate >&
        xSignatureTemplate)
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException) override;

public:
    explicit SignatureCreatorImpl( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > & xContext );
    virtual ~SignatureCreatorImpl();

    /* XBlockerMonitor */
    virtual void SAL_CALL setBlockerId( sal_Int32 id )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception) override;

    /* XSignatureCreationResultBroadcaster */
    void SAL_CALL addSignatureCreationResultListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSignatureCreationResultListener >& listener )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception) override;

    void SAL_CALL removeSignatureCreationResultListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSignatureCreationResultListener >& listener )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    /* XInitialization */
    virtual void SAL_CALL initialize(
        const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception) override;

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;
};

OUString SignatureCreatorImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< OUString > SAL_CALL SignatureCreatorImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL SignatureCreatorImpl_createInstance(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( com::sun::star::uno::Exception );

#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
