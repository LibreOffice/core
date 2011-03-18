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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include "comphelper/docpasswordrequest.hxx"
#include <com/sun/star/task/DocumentMSPasswordRequest2.hpp>
#include <com/sun/star/task/DocumentPasswordRequest2.hpp>
#include <com/sun/star/task/PasswordRequest.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionPassword2.hpp>

using ::rtl::OUString;
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

/*uno::*/Any SAL_CALL SimplePasswordRequest::queryInterface( const /*uno::*/Type& rType ) throw (RuntimeException)
{
    return ::cppu::queryInterface ( rType,
            // OWeakObject interfaces
            dynamic_cast< XInterface* > ( (XInteractionRequest *) this ),
            static_cast< XWeak* > ( this ),
            // my own interfaces
            static_cast< XInteractionRequest*  > ( this ) );
}

void SAL_CALL SimplePasswordRequest::acquire(  ) throw ()
{
    OWeakObject::acquire();
}

void SAL_CALL SimplePasswordRequest::release(  ) throw ()
{
    OWeakObject::release();
}

sal_Bool SimplePasswordRequest::isAbort() const
{
    return mpAbort->isSelected();
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

/*uno::*/Any SAL_CALL DocPasswordRequest::queryInterface( const /*uno::*/Type& rType ) throw (RuntimeException)
{
    return ::cppu::queryInterface ( rType,
            // OWeakObject interfaces
            dynamic_cast< XInterface* > ( (XInteractionRequest *) this ),
            static_cast< XWeak* > ( this ),
            // my own interfaces
            static_cast< XInteractionRequest*  > ( this ) );
}

void SAL_CALL DocPasswordRequest::acquire(  ) throw ()
{
    OWeakObject::acquire();
}

void SAL_CALL DocPasswordRequest::release(  ) throw ()
{
    OWeakObject::release();
}

sal_Bool DocPasswordRequest::isAbort() const
{
    return mpAbort->isSelected();
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
