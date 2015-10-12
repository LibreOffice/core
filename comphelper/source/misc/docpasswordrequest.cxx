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


#include <comphelper/docpasswordrequest.hxx>
#include <com/sun/star/task/DocumentMSPasswordRequest2.hpp>
#include <com/sun/star/task/DocumentPasswordRequest2.hpp>
#include <com/sun/star/task/PasswordRequest.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionPassword2.hpp>
#include <cppuhelper/implbase.hxx>

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



class AbortContinuation : public ::cppu::WeakImplHelper< XInteractionAbort >
{
public:
    virtual void SAL_CALL select() throw( RuntimeException, std::exception ) override {}
};



class PasswordContinuation : public ::cppu::WeakImplHelper< XInteractionPassword2 >
{
public:
    inline explicit     PasswordContinuation() : mbReadOnly( false ), mbSelected( false ) {}

    inline bool     isSelected() const { return mbSelected; }

    virtual void SAL_CALL select() throw( RuntimeException, std::exception ) override { mbSelected = true; }

    virtual void SAL_CALL setPassword( const OUString& rPass ) throw( RuntimeException, std::exception ) override { maPassword = rPass; }
    virtual OUString SAL_CALL getPassword() throw( RuntimeException, std::exception ) override { return maPassword; }

    virtual void SAL_CALL setPasswordToModify( const OUString& rPass ) throw( RuntimeException, std::exception ) override { maModifyPassword = rPass; }
    virtual OUString SAL_CALL getPasswordToModify() throw( RuntimeException, std::exception ) override { return maModifyPassword; }

    virtual void SAL_CALL setRecommendReadOnly( sal_Bool bReadOnly ) throw( RuntimeException, std::exception ) override { mbReadOnly = bReadOnly; }
    virtual sal_Bool SAL_CALL getRecommendReadOnly() throw( RuntimeException, std::exception ) override { return mbReadOnly; }

private:
    OUString            maPassword;
    OUString            maModifyPassword;
    bool            mbReadOnly;
    bool            mbSelected;
};



SimplePasswordRequest::SimplePasswordRequest( PasswordRequestMode eMode )
{
    PasswordRequest aRequest( OUString(), Reference< XInterface >(),
        InteractionClassification_QUERY, eMode );
    maRequest <<= aRequest;

    maContinuations.realloc( 2 );
    maContinuations[ 0 ].set( new AbortContinuation );
    mpPassword = new PasswordContinuation;
    maContinuations[ 1 ].set( mpPassword );
}

SimplePasswordRequest::~SimplePasswordRequest()
{
}

bool SimplePasswordRequest::isPassword() const
{
    return mpPassword->isSelected();
}

OUString SimplePasswordRequest::getPassword() const
{
    return mpPassword->getPassword();
}

Any SAL_CALL SimplePasswordRequest::getRequest() throw( RuntimeException, std::exception )
{
    return maRequest;
}

Sequence< Reference< XInteractionContinuation > > SAL_CALL SimplePasswordRequest::getContinuations() throw( RuntimeException, std::exception )
{
    return maContinuations;
}



DocPasswordRequest::DocPasswordRequest( DocPasswordRequestType eType,
        PasswordRequestMode eMode, const OUString& rDocumentName, bool bPasswordToModify )
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
    maContinuations[ 0 ].set( new AbortContinuation );
    mpPassword = new PasswordContinuation;
    maContinuations[ 1 ].set( mpPassword );
}

DocPasswordRequest::~DocPasswordRequest()
{
}

bool DocPasswordRequest::isPassword() const
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

bool DocPasswordRequest::getRecommendReadOnly() const
{
    return mpPassword->getRecommendReadOnly();
}

Any SAL_CALL DocPasswordRequest::getRequest() throw( RuntimeException, std::exception )
{
    return maRequest;
}

Sequence< Reference< XInteractionContinuation > > SAL_CALL DocPasswordRequest::getContinuations() throw( RuntimeException, std::exception )
{
    return maContinuations;
}



} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
