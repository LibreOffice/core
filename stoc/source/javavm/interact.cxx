/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

#include "interact.hxx"

#include "com/sun/star/java/JavaDisabledException.hpp"
#include "com/sun/star/java/JavaVMCreationFailureException.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionRetry.hpp"
#include "com/sun/star/task/XInteractionContinuation.hpp"
#include "cppuhelper/implbase1.hxx"
#include "osl/mutex.hxx"

namespace css = com::sun::star;

using stoc_javavm::InteractionRequest;

namespace {

class AbortContinuation:
    public cppu::WeakImplHelper1< css::task::XInteractionAbort >
{
public:
    inline AbortContinuation() {}

    virtual inline void SAL_CALL select() throw (css::uno::RuntimeException) {}

private:
    AbortContinuation(AbortContinuation &); // not implemented
    void operator =(AbortContinuation); // not implemented

    virtual inline ~AbortContinuation() {}
};

}

class InteractionRequest::RetryContinuation:
    public cppu::WeakImplHelper1< css::task::XInteractionRetry >
{
public:
    inline RetryContinuation(): m_bSelected(false) {}

    virtual void SAL_CALL select() throw (css::uno::RuntimeException);

    bool isSelected() const;

private:
    RetryContinuation(RetryContinuation &); // not implemented
    void operator =(RetryContinuation); // not implemented

    virtual inline ~RetryContinuation() {}

    mutable osl::Mutex m_aMutex;
    bool m_bSelected;
};

void SAL_CALL InteractionRequest::RetryContinuation::select()
    throw (css::uno::RuntimeException)
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
    throw (css::uno::RuntimeException)
{
    return m_aRequest;
}

css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > >
SAL_CALL InteractionRequest::getContinuations()
    throw (css::uno::RuntimeException)
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
