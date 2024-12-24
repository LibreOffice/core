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

#ifndef INCLUDED_COMPHELPER_DOCPASSWORDREQUEST_HXX
#define INCLUDED_COMPHELPER_DOCPASSWORDREQUEST_HXX

#include <comphelper/comphelperdllapi.h>
#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

namespace comphelper {

class PasswordContinuation;
class AbortContinuation;

/** Selects which UNO document password request type to use. */
enum class DocPasswordRequestType
{
    Standard,    /// Uses the standard com.sun.star.task.DocumentPasswordRequest request.
    MS           /// Uses the com.sun.star.task.DocumentMSPasswordRequest request.
};


class COMPHELPER_DLLPUBLIC SimplePasswordRequest final :
        public cppu::WeakImplHelper<css::task::XInteractionRequest>
{
public:
    explicit    SimplePasswordRequest();
    virtual     ~SimplePasswordRequest() override;

    bool         isPassword() const;

    OUString     getPassword() const;

private:
    SimplePasswordRequest(SimplePasswordRequest const&) = delete;
    SimplePasswordRequest& operator=(SimplePasswordRequest const&) = delete;

    // XInteractionRequest
    virtual css::uno::Any SAL_CALL getRequest() override;
    virtual css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL getContinuations() override;

private:
    css::uno::Any                         maRequest;
    rtl::Reference<AbortContinuation>     mxAbort;
    rtl::Reference<PasswordContinuation>  mxPassword;
};


/** Implements the task.XInteractionRequest interface for requesting a password
    string for a document.
 */
class COMPHELPER_DLLPUBLIC DocPasswordRequest final :
        public cppu::WeakImplHelper<css::task::XInteractionRequest>
{
public:
    explicit            DocPasswordRequest(
                            DocPasswordRequestType eType,
                            css::task::PasswordRequestMode eMode,
                            const OUString& rDocumentUrl,
                            bool bPasswordToModify = false );
    virtual             ~DocPasswordRequest() override;

    bool            isPassword() const;

    OUString     getPassword() const;

    OUString     getPasswordToModify() const;
    bool            getRecommendReadOnly() const;

private:
    DocPasswordRequest(DocPasswordRequest const&) = delete;
    DocPasswordRequest& operator=(DocPasswordRequest const&) = delete;

    // XInteractionRequest
    virtual css::uno::Any SAL_CALL getRequest() override;
    virtual css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL getContinuations() override;

private:
    css::uno::Any                         maRequest;
    rtl::Reference<AbortContinuation>     mxAbort;
    rtl::Reference<PasswordContinuation>  mxPassword;
};


} // namespace comphelper

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
