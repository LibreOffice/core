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

#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <cppuhelper/implbase.hxx>
#include <mutex>

namespace com::sun::star::task { class XInteractionContinuation; }

using stoc_javavm::InteractionRequest;

namespace {

class AbortContinuation:
    public cppu::WeakImplHelper<css::task::XInteractionAbort>
{
public:
    AbortContinuation() {}
    AbortContinuation(const AbortContinuation&) = delete;
    AbortContinuation& operator=(const AbortContinuation&)= delete;

    virtual void SAL_CALL select() override {}

private:
    virtual ~AbortContinuation() override {}
};

}

class InteractionRequest::RetryContinuation:
    public cppu::WeakImplHelper<css::task::XInteractionRetry>
{
public:
    RetryContinuation(): m_bSelected(false) {}
    RetryContinuation(const RetryContinuation&) = delete;
    RetryContinuation& operator=(const RetryContinuation&) = delete;

    virtual void SAL_CALL select() override;

    bool isSelected() const;

private:
    virtual ~RetryContinuation() override {}

    mutable std::mutex m_aMutex;
    bool m_bSelected;
};

void SAL_CALL InteractionRequest::RetryContinuation::select()
{
    std::scoped_lock aGuard(m_aMutex);
    m_bSelected = true;
}

bool InteractionRequest::RetryContinuation::isSelected() const
{
    std::scoped_lock aGuard(m_aMutex);
    return m_bSelected;
}

InteractionRequest::InteractionRequest(css::uno::Any const & rRequest):
    m_aRequest(rRequest)
{
    m_xRetryContinuation = new RetryContinuation;
    m_aContinuations = { new AbortContinuation, m_xRetryContinuation.get() };
}

css::uno::Any SAL_CALL InteractionRequest::getRequest()
{
    return m_aRequest;
}

css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > >
SAL_CALL InteractionRequest::getContinuations()
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
