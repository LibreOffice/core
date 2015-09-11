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

#include "lokinteractionhandler.hxx"

#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/task/XInteractionApprove.hpp>

using namespace com::sun::star;

LOKInteractionHandler::LOKInteractionHandler(uno::Reference<uno::XComponentContext> const & /*rxContext*/)
{
}

LOKInteractionHandler::~LOKInteractionHandler()
{
}

OUString SAL_CALL LOKInteractionHandler::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.uui.LOKInteractionHandler");
}

sal_Bool SAL_CALL LOKInteractionHandler::supportsService(OUString const & rServiceName) throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL LOKInteractionHandler::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aNames(3);
    aNames[0] = "com.sun.star.task.InteractionHandler";
    // added to indicate support for configuration.backend.MergeRecoveryRequest
    aNames[1] = "com.sun.star.configuration.backend.InteractionHandler";
    aNames[2] = "com.sun.star.uui.InteractionHandler";
    // for backwards compatibility
    return aNames;
}

void SAL_CALL LOKInteractionHandler::initialize(uno::Sequence<uno::Any> const & /*rArguments*/) throw (uno::Exception, std::exception)
{
}

void SAL_CALL LOKInteractionHandler::handle(uno::Reference<task::XInteractionRequest> const & rRequest) throw (uno::RuntimeException, std::exception)
{
    // just do the same thing in both cases
    handleInteractionRequest(rRequest);
}

sal_Bool SAL_CALL LOKInteractionHandler::handleInteractionRequest(const uno::Reference<task::XInteractionRequest >& rRequest) throw ( uno::RuntimeException, std::exception )
{
    uno::Sequence<uno::Reference<task::XInteractionContinuation>> const &rContinuations = rRequest->getContinuations();

    // TODO: add LOK api that allows handling this for real, for the moment we
    // just set the interaction as 'Approved'
    for (sal_Int32 i = 0; i < rContinuations.getLength(); ++i)
    {
        uno::Reference<task::XInteractionApprove> xApprove(rContinuations[i], uno::UNO_QUERY);
        if (xApprove.is())
            xApprove->select();
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
