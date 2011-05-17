/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SIGNATURECREATORIMPL_HXX
#define _SIGNATURECREATORIMPL_HXX

#include <com/sun/star/xml/crypto/sax/XBlockerMonitor.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultBroadcaster.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase4.hxx>

#include "signatureengine.hxx"

class SignatureCreatorImpl : public cppu::ImplInheritanceHelper4
<
    SignatureEngine,
    com::sun::star::xml::crypto::sax::XBlockerMonitor,
    com::sun::star::xml::crypto::sax::XSignatureCreationResultBroadcaster,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
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
     * the Id of the signature, which is used for the result listener to
     * identify the signature.
     */
    sal_Int32 m_nSignatureId;

    /*
     * the Id of template blocker.
     */
    sal_Int32 m_nIdOfBlocker;

    /*
     * the signature creation result
     */
    bool      m_bCreationSucceed;

    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XSecurityEnvironment > m_xSecurityEnvironment;

    virtual void notifyResultListener() const
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
    virtual void clearUp( ) const;
    virtual bool checkReady() const;
    virtual void startEngine( const com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::XXMLSignatureTemplate >&
        xSignatureTemplate)
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

public:
    explicit SignatureCreatorImpl( const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >& rxMSF);
    virtual ~SignatureCreatorImpl();

    /* XBlockerMonitor */
    virtual void SAL_CALL setBlockerId( sal_Int32 id )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    /* XSignatureCreationResultBroadcaster */
    void SAL_CALL addSignatureCreationResultListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSignatureCreationResultListener >& listener )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    void SAL_CALL removeSignatureCreationResultListener(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::crypto::sax::XSignatureCreationResultListener >& listener )
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

rtl::OUString SignatureCreatorImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL SignatureCreatorImpl_supportsService( const rtl::OUString& ServiceName )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL SignatureCreatorImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL SignatureCreatorImpl_createInstance(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( com::sun::star::uno::Exception );

#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
