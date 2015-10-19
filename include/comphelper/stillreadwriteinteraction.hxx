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

#ifndef INCLUDED_COMPHELPER_STILLREADWRITEINTERACTION_HXX
#define INCLUDED_COMPHELPER_STILLREADWRITEINTERACTION_HXX

#include <ucbhelper/interceptedinteraction.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>

#include <comphelper/comphelperdllapi.h>


namespace comphelper{
class COMPHELPER_DLLPUBLIC StillReadWriteInteraction : public ::ucbhelper::InterceptedInteraction
{
private:
    static const sal_Int32 HANDLE_INTERACTIVEIOEXCEPTION       = 0;
    static const sal_Int32 HANDLE_UNSUPPORTEDDATASINKEXCEPTION = 1;
    static const sal_Int32 HANDLE_AUTHENTICATIONREQUESTEXCEPTION = 2;

    bool m_bUsed;
    bool m_bHandledByMySelf;
    bool m_bHandledByInternalHandler;

public:
    StillReadWriteInteraction(const css::uno::Reference< css::task::XInteractionHandler >& xHandler,
                              const css::uno::Reference< css::task::XInteractionHandler >& xAuthenticationHandler);

    void resetInterceptions();
    void resetErrorStates();
    bool wasWriteError() { return (m_bUsed && m_bHandledByMySelf);}

private:
    css::uno::Reference< css::task::XInteractionHandler > m_xAuthenticationHandler;

    virtual ucbhelper::InterceptedInteraction::EInterceptionState intercepted(const ::ucbhelper::InterceptedInteraction::InterceptedRequest&                         aRequest,
        const css::uno::Reference< css::task::XInteractionRequest >& xRequest) override;

};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
