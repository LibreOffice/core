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
    /*
     * the Id of the encryption, which is used for the result listener to
     * identify the encryption.
     */
    sal_Int32 m_nEncryptionId;

    /*
     * the decryption result,
     * remembers whether the decryption succeeds.
     */
    bool      m_bDecryptionSucceed;

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
