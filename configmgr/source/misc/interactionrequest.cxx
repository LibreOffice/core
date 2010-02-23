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
#include "precompiled_configmgr.hxx"

#include "interactionrequest.hxx"

namespace configmgr {
namespace apihelper {
    namespace uno  = com::sun::star::uno;
    namespace task = com::sun::star::task;
//=========================================================================
//=========================================================================
//
// InteractionRequest Implementation.
//
//=========================================================================
//=========================================================================

struct InteractionRequest::Impl
{
    uno::Reference< task::XInteractionContinuation > m_xSelection;
    uno::Any m_aRequest;
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > m_aContinuations;

    Impl() {}
    Impl( const uno::Any & rRequest )
    : m_aRequest( rRequest )
    {}
};

//=========================================================================
InteractionRequest::InteractionRequest( const uno::Any & rRequest )
: m_pImpl( new Impl( rRequest ) )
{
}

//=========================================================================
// virtual
InteractionRequest::~InteractionRequest()
{
    delete m_pImpl;
}

//=========================================================================
void InteractionRequest::setContinuations(
                const uno::Sequence< uno::Reference<
                    task::XInteractionContinuation > > & rContinuations )
{
    m_pImpl->m_aContinuations = rContinuations;
}

//=========================================================================
uno::Reference< task::XInteractionContinuation > InteractionRequest::getSelection() const
{
    return m_pImpl->m_xSelection;
}

//=========================================================================
void InteractionRequest::setSelection( const uno::Reference< task::XInteractionContinuation > & rxSelection )
{
    m_pImpl->m_xSelection = rxSelection;
}

//=========================================================================
//
// XInteractionRequest methods.
//
//=========================================================================

// virtual
uno::Any SAL_CALL InteractionRequest::getRequest()
    throw( uno::RuntimeException )
{
    return m_pImpl->m_aRequest;
}

//=========================================================================
// virtual
uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL
    InteractionRequest::getContinuations()
    throw( uno::RuntimeException )
{
    return m_pImpl->m_aContinuations;
}

//=========================================================================

} // namespace apihelper
} // namespace configmgr
