/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: interactionrequest.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:09:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
InteractionRequest::InteractionRequest()
: m_pImpl( new Impl )
{
}

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
void InteractionRequest::setRequest( const uno::Any & rRequest )
{
    m_pImpl->m_aRequest = rRequest;
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
