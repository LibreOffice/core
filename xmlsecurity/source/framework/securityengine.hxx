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

#ifndef _SECURITYENGINE_HXX
#define _SECURITYENGINE_HXX

#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XMissionTaker.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeper.hpp>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>

#include <cppuhelper/implbase3.hxx>

class SecurityEngine : public cppu::WeakImplHelper3
<
    com::sun::star::xml::crypto::sax::XReferenceResolvedListener,
    com::sun::star::xml::crypto::sax::XKeyCollector,
    com::sun::star::xml::crypto::sax::XMissionTaker
>
/****** securityengine.hxx/CLASS SecurityEngine *******************************
 *
 *   NAME
 *  SecurityEngine -- Base class of SignatureEngine and EncryptionEngine
 *
 *   FUNCTION
 *  Maintains common members and methods related with security engine
 *  operation.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
protected:
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > mxMSF;

    /*
     * A SAXEventKeeper internally maintians all resources that a security
     * operation needs. The m_xSAXEventKeeper member is used to release
     * those resources when the security operation finishes.
     */
    com::sun::star::uno::Reference<
        com::sun::star::xml::crypto::sax::XSAXEventKeeper > m_xSAXEventKeeper;

    /*
     * the id of ElementCollector of the template element.
     * For a signature, the template element is the Signature element,
     * for a encryption, the EncryptedData/EncryptedKey element is.
     */
    sal_Int32 m_nIdOfTemplateEC;

    /*
     * remembers how many referenced elements have been bufferred completely,
     * including the key element, template element, and referenced element of
     * signature.
     */
    sal_Int32 m_nNumOfResolvedReferences;

    /*
     * the id of ElementCollector of the key element.
     * If a Signature element or EncryptedData/EncryptedKey element has
     * an internal key sub-element, then this member should be -1
     */
    sal_Int32 m_nIdOfKeyEC;

    /*
     * remembers whether the current opertion has finished.
     */
    bool      m_bMissionDone;

    /*
     * the Id of the security entity, a signature or encryption, which is used for
     * the result listener to identify the entity.
     */
    sal_Int32 m_nSecurityId;

    /*
     * the status of the operation
     */
    //bool      m_bOperationSucceed;
    com::sun::star::xml::crypto::SecurityOperationStatus m_nStatus;

    /*
     * the result listener, which will receives the security operation result.
     */
    com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface >
        m_xResultListener;

protected:
    explicit SecurityEngine( const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >& rxMSF = NULL );
    virtual ~SecurityEngine() {};

    /*
     * perform the security operation.
     * Any derived class will implement this method respectively.
     */
    virtual void tryToPerform( )
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException){};

    /*
     * clear up all resources used by this operation.
     * This method is called after the operation finishes, or a End-Your-Mission
     * message is received.
     * Any derived class will implement this method respectively.
     */
    virtual void clearUp( ) const {};

        /*
         * notifies any possible result listener.
         * When verify a signature or conduct a decryption, the operation result will
         * be transferred to a listener by this method.
     * Any derived class will implement this method respectively.
         */
    virtual void notifyResultListener() const
        throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
        {};

    /*
     * checks whether everything is ready.
     * Any derived class will implement this method respectively.
     */
    virtual bool checkReady() const { return true; };

public:
    /* XReferenceResolvedListener */
    virtual void SAL_CALL referenceResolved( sal_Int32 referenceId )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    /* XKeyCollector */
    virtual void SAL_CALL setKeyId( sal_Int32 id )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

        /* XMissionTaker */
        virtual sal_Bool SAL_CALL endMission(  )
            throw (com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
