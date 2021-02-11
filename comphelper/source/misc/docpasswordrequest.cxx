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
using ::com::sun::star::uno::Reference;
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

namespace comphelper {

namespace {

class AbortContinuation : public ::cppu::WeakImplHelper< XInteractionAbort >
{
public:
    virtual void SAL_CALL select() override {}
};

}

class PasswordContinuation : public ::cppu::WeakImplHelper< XInteractionPassword2 >
{
public:
    explicit     PasswordContinuation() : mbReadOnly( false ), mbSelected( false ) {}

    bool     isSelected() const { return mbSelected; }

    virtual void SAL_CALL select() override { mbSelected = true; }

    virtual void SAL_CALL setPassword( const OUString& rPass ) override { maPassword = rPass; }
    virtual OUString SAL_CALL getPassword() override { return maPassword; }

    virtual void SAL_CALL setPasswordToModify( const OUString& rPass ) override { maModifyPassword = rPass; }
    virtual OUString SAL_CALL getPasswordToModify() override { return maModifyPassword; }

    virtual void SAL_CALL setRecommendReadOnly( sal_Bool bReadOnly ) override { mbReadOnly = bReadOnly; }
    virtual sal_Bool SAL_CALL getRecommendReadOnly() override { return mbReadOnly; }

private:
    OUString            maPassword;
    OUString            maModifyPassword;
    bool            mbReadOnly;
    bool            mbSelected;
};


SimplePasswordRequest::SimplePasswordRequest()
{
    PasswordRequest aRequest( OUString(), Reference< XInterface >(),
        InteractionClassification_QUERY, css::task::PasswordRequestMode_PASSWORD_CREATE );
    maRequest <<= aRequest;

    mxAbort = new AbortContinuation;
    mxPassword = new PasswordContinuation;
}

SimplePasswordRequest::~SimplePasswordRequest()
{
}

bool SimplePasswordRequest::isPassword() const
{
    return mxPassword->isSelected();
}

OUString SimplePasswordRequest::getPassword() const
{
    return mxPassword->getPassword();
}

Any SAL_CALL SimplePasswordRequest::getRequest()
{
    return maRequest;
}

Sequence< Reference< XInteractionContinuation > > SAL_CALL SimplePasswordRequest::getContinuations()
{
    return { mxAbort, mxPassword };
}


DocPasswordRequest::DocPasswordRequest( DocPasswordRequestType eType,
        PasswordRequestMode eMode, const OUString& rDocumentUrl, bool bPasswordToModify )
{
    switch( eType )
    {
        case DocPasswordRequestType::Standard:
        {
            DocumentPasswordRequest2 aRequest( OUString(), Reference< XInterface >(),
                InteractionClassification_QUERY, eMode, rDocumentUrl, bPasswordToModify );
            maRequest <<= aRequest;
        }
        break;
        case DocPasswordRequestType::MS:
        {
            DocumentMSPasswordRequest2 aRequest( OUString(), Reference< XInterface >(),
                InteractionClassification_QUERY, eMode, rDocumentUrl, bPasswordToModify );
            maRequest <<= aRequest;
        }
        break;
        /*  no 'default', so compilers will complain about missing
            implementation of a new enum value. */
    }

    mxAbort = new AbortContinuation;
    mxPassword = new PasswordContinuation;
}

DocPasswordRequest::~DocPasswordRequest()
{
}

bool DocPasswordRequest::isPassword() const
{
    return mxPassword->isSelected();
}

OUString DocPasswordRequest::getPassword() const
{
    return mxPassword->getPassword();
}

OUString DocPasswordRequest::getPasswordToModify() const
{
    return mxPassword->getPasswordToModify();
}

bool DocPasswordRequest::getRecommendReadOnly() const
{
    return mxPassword->getRecommendReadOnly();
}

Any SAL_CALL DocPasswordRequest::getRequest()
{
    return maRequest;
}

Sequence< Reference< XInteractionContinuation > > SAL_CALL DocPasswordRequest::getContinuations()
{
    return { mxAbort, mxPassword };
}


} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
