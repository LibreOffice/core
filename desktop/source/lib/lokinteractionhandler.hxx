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

#include <osl/conditn.hxx>
#include <cppuhelper/implbase.hxx>
#include <vcl/errcode.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>

namespace desktop {
    struct LibLibreOffice_Impl;
    struct LibLODocument_Impl;
}

/** InteractionHandler is an interface that provides the user with various dialogs / error messages.

We need an own implementation for the LibreOfficeKit so that we can route the
information easily via callbacks.

TODO: the callbacks are not implemented yet, we just approve any interaction
that we get.
*/
class LOKInteractionHandler: public cppu::WeakImplHelper<com::sun::star::lang::XServiceInfo,
                                                         com::sun::star::lang::XInitialization,
                                                         com::sun::star::task::XInteractionHandler2>
{
private:
    desktop::LibLibreOffice_Impl * m_pLOKit;
    desktop::LibLODocument_Impl * m_pLOKDocument;

    /// Command for which we use this interaction handler (like "load", "save", "saveas", ...)
    OString m_command;

    OUString m_Password;
    bool m_usePassword;
    osl::Condition m_havePassword;

    LOKInteractionHandler(const LOKInteractionHandler&) = delete;
    LOKInteractionHandler& operator=(const LOKInteractionHandler&) = delete;

    /** Call the LOK_CALLBACK_ERROR on the LOK document (if available) or LOK lib.

        The error itself is a JSON message, like:
        {
            "classification": "error" | "warning" | "info"
            "kind": "network" etc.
            "code": 403 | 404 | ...
            "message": freeform description
        }
    */
    void postError(css::task::InteractionClassification classif, const char* kind, ErrCode code, const OUString &message);

    bool handleIOException(const css::uno::Sequence<css::uno::Reference<css::task::XInteractionContinuation>> &rContinuations, const css::uno::Any& rRequest);
    bool handleNetworkException(const css::uno::Sequence<css::uno::Reference<css::task::XInteractionContinuation>> &rContinuations, const css::uno::Any& rRequest);
    bool handlePasswordRequest(const css::uno::Sequence<css::uno::Reference<css::task::XInteractionContinuation>> &rContinuations, const css::uno::Any& rRequest);

public:
    void SetPassword(char const* pPassword);

    explicit LOKInteractionHandler(
            const OString& rCommand,
            desktop::LibLibreOffice_Impl *,
            desktop::LibLODocument_Impl *pLOKDocumt = nullptr);

    virtual ~LOKInteractionHandler() override;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & rServiceName) override;

    virtual com::sun::star::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    virtual void SAL_CALL initialize(com::sun::star::uno::Sequence<com::sun::star::uno::Any > const & rArguments) override;

    virtual void SAL_CALL handle(com::sun::star::uno::Reference<com::sun::star::task::XInteractionRequest> const & rRequest) override;

    virtual sal_Bool SAL_CALL handleInteractionRequest(const ::com::sun::star::uno::Reference<::com::sun::star::task::XInteractionRequest>& Request) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
