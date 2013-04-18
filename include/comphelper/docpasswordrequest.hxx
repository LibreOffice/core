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

#ifndef COMPHELPER_DOCPASSWORDREQUEST_HXX
#define COMPHELPER_DOCPASSWORDREQUEST_HXX

#include "comphelper/comphelperdllapi.h"
#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weak.hxx>


namespace comphelper {

class AbortContinuation;
class PasswordContinuation;

// ============================================================================

/** Selects which UNO document password request type to use. */
enum DocPasswordRequestType
{
    DocPasswordRequestType_STANDARD,    /// Uses the standard com.sun.star.task.DocumentPasswordRequest request.
    DocPasswordRequestType_MS           /// Uses the com.sun.star.task.DocumentMSPasswordRequest request.
};

// ============================================================================

class COMPHELPER_DLLPUBLIC SimplePasswordRequest :
        public ::com::sun::star::task::XInteractionRequest,
        public ::cppu::OWeakObject
{
public:
    explicit    SimplePasswordRequest( com::sun::star::task::PasswordRequestMode eMode );
    virtual     ~SimplePasswordRequest();

    // XInterface / OWeakObject
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();

    sal_Bool            isPassword() const;

    OUString     getPassword() const;

private:
    // XInteractionRequest
    virtual ::com::sun::star::uno::Any SAL_CALL getRequest() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( ::com::sun::star::uno::RuntimeException );

private:
    ::com::sun::star::uno::Any      maRequest;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > maContinuations;
    AbortContinuation *             mpAbort;
    PasswordContinuation *          mpPassword;
};

// ============================================================================

/** Implements the task.XInteractionRequest interface for requesting a password
    string for a document.
 */
class COMPHELPER_DLLPUBLIC DocPasswordRequest :
        public ::com::sun::star::task::XInteractionRequest,
        public ::cppu::OWeakObject
{
public:
    explicit            DocPasswordRequest(
                            DocPasswordRequestType eType,
                            ::com::sun::star::task::PasswordRequestMode eMode,
                            const OUString& rDocumentName,
                            sal_Bool bPasswordToModify = sal_False );
    virtual             ~DocPasswordRequest();

    // XInterface / OWeakObject
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();

    sal_Bool            isPassword() const;

    OUString     getPassword() const;

    OUString     getPasswordToModify() const;
    sal_Bool            getRecommendReadOnly() const;

private:
    // XInteractionRequest
    virtual ::com::sun::star::uno::Any SAL_CALL getRequest() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( ::com::sun::star::uno::RuntimeException );

private:
    ::com::sun::star::uno::Any      maRequest;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > maContinuations;
    AbortContinuation *             mpAbort;
    PasswordContinuation *          mpPassword;
};

// ============================================================================

} // namespace comphelper

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
