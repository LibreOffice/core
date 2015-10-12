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


#include "interact.hxx"

#include <boost/noncopyable.hpp>
#include <com/sun/star/java/JavaDisabledException.hpp>
#include <com/sun/star/java/JavaVMCreationFailureException.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>

using stoc_javavm::InteractionRequest;

namespace {

class AbortContinuation:
    public cppu::WeakImplHelper<css::task::XInteractionAbort>,
    private boost::noncopyable
{
public:
    inline AbortContinuation() {}

    virtual void SAL_CALL select() throw (css::uno::RuntimeException, std::exception) override {}

private:
    virtual inline ~AbortContinuation() {}
};

}

class InteractionRequest::RetryContinuation:
    public cppu::WeakImplHelper<css::task::XInteractionRetry>,
    private boost::noncopyable
{
public:
    inline RetryContinuation(): m_bSelected(false) {}

    virtual void SAL_CALL select() throw (css::uno::RuntimeException, std::exception) override;

    bool isSelected() const;

private:
    virtual inline ~RetryContinuation() {}

    mutable osl::Mutex m_aMutex;
    bool m_bSelected;
};

void SAL_CALL InteractionRequest::RetryContinuation::select()
    throw (css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard aGuard(m_aMutex);
    m_bSelected = true;
}

bool InteractionRequest::RetryContinuation::isSelected() const
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_bSelected;
}

InteractionRequest::InteractionRequest(css::uno::Any const & rRequest):
    m_aRequest(rRequest)
{
    m_aContinuations.realloc(2);
    m_xRetryContinuation = new RetryContinuation;
    m_aContinuations[0] = new AbortContinuation;
    m_aContinuations[1] = m_xRetryContinuation.get();
}

css::uno::Any SAL_CALL InteractionRequest::getRequest()
    throw (css::uno::RuntimeException, std::exception)
{
    return m_aRequest;
}

css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > >
SAL_CALL InteractionRequest::getContinuations()
    throw (css::uno::RuntimeException, std::exception)
{
    return m_aContinuations;
}

bool InteractionRequest::retry() const
{
    return m_xRetryContinuation.is() && m_xRetryContinuation->isSelected();
}

InteractionRequest::~InteractionRequest()
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
