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

#if !defined INCLUDED_STOC_JAVAVM_INTERACT_HXX
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
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionContinuation > > SAL_CALL
    getContinuations() throw (com::sun::star::uno::RuntimeException);

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
