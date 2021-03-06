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

#pragma once

#include <com/sun/star/xml/crypto/sax/XReferenceResolvedListener.hpp>
#include <com/sun/star/xml/crypto/sax/XReferenceResolvedBroadcaster.hpp>
#include <com/sun/star/xml/crypto/sax/XKeyCollector.hpp>
#include <com/sun/star/xml/crypto/sax/XMissionTaker.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeper.hpp>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>

#include <cppuhelper/implbase.hxx>

#include <xmlsecuritydllapi.h>

class SAL_DLLPUBLIC_RTTI SecurityEngine : public cppu::WeakImplHelper
<
    css::xml::crypto::sax::XReferenceResolvedListener,
    css::xml::crypto::sax::XKeyCollector,
    css::xml::crypto::sax::XMissionTaker
>
/****** securityengine.hxx/CLASS SecurityEngine *******************************
 *
 *   NAME
 *  SecurityEngine -- Base class of SignatureEngine and EncryptionEngine
 *
 *   FUNCTION
 *  Maintains common members and methods related with security engine
 *  operation.
 ******************************************************************************/
{
protected:

    /*
     * A SAXEventKeeper internally maintains all resources that a security
     * operation needs. The m_xSAXEventKeeper member is used to release
     * those resources when the security operation finishes.
     */
    css::uno::Reference< css::xml::crypto::sax::XSAXEventKeeper > m_xSAXEventKeeper;

    /*
     * the id of ElementCollector of the template element.
     * For a signature, the template element is the Signature element,
     * for an encryption, the EncryptedData/EncryptedKey element is.
     */
    sal_Int32 m_nIdOfTemplateEC;

    /*
     * remembers how many referenced elements have been buffered completely,
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
     * remembers whether the current operation has finished.
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
    css::xml::crypto::SecurityOperationStatus m_nStatus;

    /*
     * the result listener, which will receives the security operation result.
     */
    css::uno::Reference< css::uno::XInterface > m_xResultListener;

protected:
    explicit SecurityEngine();
    virtual ~SecurityEngine() override {};

    /*
     * perform the security operation.
     * Any derived class will implement this method respectively.
     */
    /// @throws css::uno::Exception
    /// @throws css::uno::RuntimeException
    virtual void tryToPerform( ){};

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
    /// @throws css::uno::Exception
    /// @throws css::uno::RuntimeException
    virtual void notifyResultListener() const
        {};

public:
    /* XReferenceResolvedListener */
    virtual void SAL_CALL referenceResolved( sal_Int32 referenceId ) override;

    /* XKeyCollector */
    virtual void SAL_CALL setKeyId( sal_Int32 id ) override;

        /* XMissionTaker */
        virtual sal_Bool SAL_CALL endMission(  ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
