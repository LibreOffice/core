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

    sal_Bool            isAbort() const;
    sal_Bool            isPassword() const;

    ::rtl::OUString     getPassword() const;

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
                            const ::rtl::OUString& rDocumentName,
                            sal_Bool bPasswordToModify = sal_False );
    virtual             ~DocPasswordRequest();

    // XInterface / OWeakObject
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();

    sal_Bool            isAbort() const;
    sal_Bool            isPassword() const;

    ::rtl::OUString     getPassword() const;

    ::rtl::OUString     getPasswordToModify() const;
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
