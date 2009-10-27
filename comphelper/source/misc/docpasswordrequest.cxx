/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docpasswordrequest.cxx,v $
 * $Revision: 1.1 $
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
#include <com/sun/star/task/DocumentMSPasswordRequest.hpp>
#include <com/sun/star/task/DocumentPasswordRequest.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionPassword.hpp>

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::task::InteractionClassification_QUERY;
using ::com::sun::star::task::DocumentMSPasswordRequest;
using ::com::sun::star::task::DocumentPasswordRequest;
using ::com::sun::star::task::PasswordRequestMode;
using ::com::sun::star::task::XInteractionAbort;
using ::com::sun::star::task::XInteractionContinuation;
using ::com::sun::star::task::XInteractionPassword;

namespace comphelper {

// ============================================================================

class AbortContinuation : public ::cppu::WeakImplHelper1< XInteractionAbort >
{
public:
    inline explicit     AbortContinuation() : mbSelected( false ) {}

    inline bool         isSelected() const { return mbSelected; }
    inline void         reset() { mbSelected = false; }

    virtual void SAL_CALL select() throw( RuntimeException ) { mbSelected = true; }

private:
    bool                mbSelected;
};

// ============================================================================

class PasswordContinuation : public ::cppu::WeakImplHelper1< XInteractionPassword >
{
public:
    inline explicit     PasswordContinuation() : mbSelected( false ) {}

    inline bool         isSelected() const { return mbSelected; }
    inline void         reset() { mbSelected = false; }

    virtual void SAL_CALL select() throw( RuntimeException ) { mbSelected = true; }
    virtual void SAL_CALL setPassword( const OUString& rPass ) throw( RuntimeException ) { maPassword = rPass; }
    virtual OUString SAL_CALL getPassword() throw( RuntimeException ) { return maPassword; }

private:
    OUString            maPassword;
    bool                mbSelected;
};

// ============================================================================

DocPasswordRequest::DocPasswordRequest( DocPasswordRequestType eType,
        PasswordRequestMode eMode, const OUString& rDocumentName )
{
    switch( eType )
    {
        case DocPasswordRequestType_STANDARD:
        {
            DocumentPasswordRequest aRequest( OUString(), Reference< XInterface >(),
                InteractionClassification_QUERY, eMode, rDocumentName );
            maRequest <<= aRequest;
        }
        break;
        case DocPasswordRequestType_MS:
        {
            DocumentMSPasswordRequest aRequest( OUString(), Reference< XInterface >(),
                InteractionClassification_QUERY, eMode, rDocumentName );
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

bool DocPasswordRequest::isAbort() const
{
    return mpAbort->isSelected();
}

bool DocPasswordRequest::isPassword() const
{
    return mpPassword->isSelected();
}

OUString DocPasswordRequest::getPassword() const
{
    return mpPassword->getPassword();
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

