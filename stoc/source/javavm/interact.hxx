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

#ifndef INCLUDED_STOC_SOURCE_JAVAVM_INTERACT_HXX
#define INCLUDED_STOC_SOURCE_JAVAVM_INTERACT_HXX

#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

namespace com { namespace sun { namespace star { namespace task {
    class XInteractionContinuation;
} } } }

namespace stoc_javavm {

class InteractionRequest:
    public cppu::WeakImplHelper< css::task::XInteractionRequest >
{
public:
    explicit InteractionRequest(css::uno::Any const & rRequest);

    virtual css::uno::Any SAL_CALL getRequest()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Sequence< css::uno::Reference<
        css::task::XInteractionContinuation > > SAL_CALL
    getContinuations() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    bool retry() const;

private:
    class RetryContinuation;

    InteractionRequest(InteractionRequest &) = delete;
    void operator =(const InteractionRequest&) = delete;

    virtual ~InteractionRequest();

    css::uno::Any m_aRequest;
    css::uno::Sequence< css::uno::Reference<
        css::task::XInteractionContinuation > > m_aContinuations;
    rtl::Reference< RetryContinuation > m_xRetryContinuation;
};

}

#endif // INCLUDED_STOC_SOURCE_JAVAVM_INTERACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
