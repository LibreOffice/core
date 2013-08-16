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


#include "comphelper/docpasswordrequest.hxx"
#include <com/sun/star/task/DocumentMSPasswordRequest2.hpp>
#include <com/sun/star/task/DocumentPasswordRequest2.hpp>
#include <com/sun/star/task/PasswordRequest.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionPassword2.hpp>

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::task::InteractionClassification_QUERY;
using ::com::sun::star::task::DocumentMSPasswordRequest2;
using ::com::sun::star::task::DocumentPasswordRequest2;
using ::com::sun::star::task::PasswordRequest;
using ::com::sun::star::task::PasswordRequestMode;
using ::com::sun::star::task::XInteractionAbort;
using ::com::sun::star::task::XInteractionContinuation;
using ::com::sun::star::task::XInteractionPassword2;
using ::com::sun::star::task::XInteractionRequest;

namespace comphelper {

// ============================================================================

class AbortContinuation : public ::cppu::WeakImplHelper1< XInteractionAbort >
{
public:
    inline explicit     AbortContinuation() : mbSelected( false ) {}

    inline sal_Bool     isSelected() const { return mbSelected; }
    inline void         reset() { mbSelected = false; }

    virtual void SAL_CALL select() throw( RuntimeException ) { mbSelected = true; }

private:
    sal_Bool            mbSelected;
};

// ============================================================================

class PasswordContinuation : public ::cppu::WeakImplHelper1< XInteractionPassword2 >
{
public:
    inline explicit     PasswordContinuation() : mbReadOnly( sal_False ), mbSelected( sal_False ) {}

    inline sal_Bool     isSelected() const { return mbSelected; }
    inline void         reset() { mbSelected = sal_False; }

    virtual void SAL_CALL select() throw( RuntimeException ) { mbSelected = sal_True; }

    virtual void SAL_CALL setPassword( const OUString& rPass ) throw( RuntimeException ) { maPassword = rPass; }
    virtual OUString SAL_CALL getPassword() throw( RuntimeException ) { return maPassword; }

    virtual void SAL_CALL setPasswordToModify( const OUString& rPass ) throw( RuntimeException ) { maModifyPassword = rPass; }
    virtual OUString SAL_CALL getPasswordToModify() throw( RuntimeException ) { return maModifyPassword; }

    virtual void SAL_CALL setRecommendReadOnly( sal_Bool bReadOnly ) throw( RuntimeException ) { mbReadOnly = bReadOnly; }
    virtual sal_Bool SAL_CALL getRecommendReadOnly() throw( RuntimeException ) { return mbReadOnly; }

private:
    OUString            maPassword;
    OUString            maModifyPassword;
    sal_Bool            mbReadOnly;
    sal_Bool            mbSelected;
};

// ============================================================================

SimplePasswordRequest::SimplePasswordRequest( PasswordRequestMode eMode )
: mpAbort( NULL )
, mpPassword( NULL )
{
    PasswordRequest aRequest( OUString(), Reference< XInterface >(),
        InteractionClassification_QUERY, eMode );
    maRequest <<= aRequest;

    maContinuations.realloc( 2 );
    maContinuations[ 0 ].set( mpAbort = new AbortContinuation );
    maContinuations[ 1 ].set( mpPassword = new PasswordContinuation );
}

SimplePasswordRequest::~SimplePasswordRequest()
{
}

sal_Bool SimplePasswordRequest::isPassword() const
{
    return mpPassword->isSelected();
}

OUString SimplePasswordRequest::getPassword() const
{
    return mpPassword->getPassword();
}

Any SAL_CALL SimplePasswordRequest::getRequest() throw( RuntimeException )
{
    return maRequest;
}

Sequence< Reference< XInteractionContinuation > > SAL_CALL SimplePasswordRequest::getContinuations() throw( RuntimeException )
{
    return maContinuations;
}

// ============================================================================

DocPasswordRequest::DocPasswordRequest( DocPasswordRequestType eType,
        PasswordRequestMode eMode, const OUString& rDocumentName, sal_Bool bPasswordToModify )
: mpAbort( NULL )
, mpPassword( NULL )
{
    switch( eType )
    {
        case DocPasswordRequestType_STANDARD:
        {
            DocumentPasswordRequest2 aRequest( OUString(), Reference< XInterface >(),
                InteractionClassification_QUERY, eMode, rDocumentName, bPasswordToModify );
            maRequest <<= aRequest;
        }
        break;
        case DocPasswordRequestType_MS:
        {
            DocumentMSPasswordRequest2 aRequest( OUString(), Reference< XInterface >(),
                InteractionClassification_QUERY, eMode, rDocumentName, bPasswordToModify );
            maRequest <<= aRequest;
        }
        break;
        /*  no 'default', so compilers will complain about missing
            implementation of a new enum value. */
    }

    maContinuations.realloc( 2 );
    maContinuations[ 0 ].set( mpAbort = new AbortContinuation );
    maContinuations[ 1 ].set( mpPassword = new PasswordContinuation );
}

DocPasswordRequest::~DocPasswordRequest()
{
}

sal_Bool DocPasswordRequest::isPassword() const
{
    return mpPassword->isSelected();
}

OUString DocPasswordRequest::getPassword() const
{
    return mpPassword->getPassword();
}

OUString DocPasswordRequest::getPasswordToModify() const
{
    return mpPassword->getPasswordToModify();
}

sal_Bool DocPasswordRequest::getRecommendReadOnly() const
{
    return mpPassword->getRecommendReadOnly();
}

Any SAL_CALL DocPasswordRequest::getRequest() throw( RuntimeException )
{
    return maRequest;
}

Sequence< Reference< XInteractionContinuation > > SAL_CALL DocPasswordRequest::getContinuations() throw( RuntimeException )
{
    return maContinuations;
}

// ============================================================================

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
