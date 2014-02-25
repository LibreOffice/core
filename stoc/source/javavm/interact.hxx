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

#ifndef INCLUDED_STOC_JAVAVM_INTERACT_HXX
#define INCLUDED_STOC_JAVAVM_INTERACT_HXX

#include "com/sun/star/task/XInteractionRequest.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "cppuhelper/implbase1.hxx"
#include "rtl/ref.hxx"

namespace com { namespace sun { namespace star { namespace task {
    class XInteractionContinuation;
} } } }

namespace stoc_javavm {

class InteractionRequest:
    public cppu::WeakImplHelper1< com::sun::star::task::XInteractionRequest >
{
public:
    explicit InteractionRequest(com::sun::star::uno::Any const & rRequest);

    virtual com::sun::star::uno::Any SAL_CALL getRequest()
        throw (com::sun::star::uno::RuntimeException, std::exception);

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionContinuation > > SAL_CALL
    getContinuations() throw (com::sun::star::uno::RuntimeException, std::exception);

    bool retry() const;

private:
    class RetryContinuation;

    InteractionRequest(InteractionRequest &); // not implemented
    void operator =(InteractionRequest); // not implemented

    virtual ~InteractionRequest();

    com::sun::star::uno::Any m_aRequest;
    com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionContinuation > > m_aContinuations;
    rtl::Reference< RetryContinuation > m_xRetryContinuation;
};

}

#endif // INCLUDED_STOC_JAVAVM_INTERACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
