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


#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/task/DocumentPasswordRequest.hpp>
#include <com/sun/star/task/XInteractionPassword.hpp>

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <ucbhelper/interactionrequest.hxx>

#include "tdoc_passwordrequest.hxx"

#include <mutex>

using namespace com::sun::star;
using namespace tdoc_ucp;

namespace tdoc_ucp
{
    namespace {

    using InteractionSupplyPassword_BASE = cppu::ImplInheritanceHelper<ucbhelper::InteractionContinuation,
                                                                       task::XInteractionPassword>;
    class InteractionSupplyPassword : public InteractionSupplyPassword_BASE
    {
    public:
        explicit InteractionSupplyPassword( ucbhelper::InteractionRequest * pRequest )
            : InteractionSupplyPassword_BASE( pRequest ) {}

        // XInteractionContinuation
        virtual void SAL_CALL select() override;

        // XInteractionPassword
        virtual void SAL_CALL setPassword( const OUString & aPasswd ) override;
        virtual OUString SAL_CALL getPassword() override;

    private:
        std::mutex m_aMutex;
        OUString m_aPassword;
    };

    }
} // namespace tdoc_ucp


// InteractionSupplyPassword Implementation.


// XInteractionContinuation methods.


// virtual
void SAL_CALL InteractionSupplyPassword::select()
{
    recordSelection();
}


// XInteractionPassword methods.


// virtual
void SAL_CALL
InteractionSupplyPassword::setPassword( const OUString& aPasswd )
{
    std::scoped_lock aGuard( m_aMutex );
    m_aPassword = aPasswd;
}

// virtual
OUString SAL_CALL InteractionSupplyPassword::getPassword()
{
    std::scoped_lock aGuard( m_aMutex );
    return m_aPassword;
}


// DocumentPasswordRequest Implementation.


DocumentPasswordRequest::DocumentPasswordRequest(
    task::PasswordRequestMode eMode,
    const OUString & rDocumentName )
{
    // Fill request...
    task::DocumentPasswordRequest aRequest;
//    aRequest.Message        = // OUString
//    aRequest.Context        = // XInterface
    aRequest.Classification = task::InteractionClassification_ERROR;
    aRequest.Mode           = eMode;
    aRequest.Name           = rDocumentName;

    setRequest( uno::Any( aRequest ) );

    // Fill continuations...
    uno::Sequence<
        uno::Reference< task::XInteractionContinuation > > aContinuations{
            new ucbhelper::InteractionAbort( this ),
            new ucbhelper::InteractionRetry( this ),
            new InteractionSupplyPassword( this )
    };

    setContinuations( aContinuations );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
